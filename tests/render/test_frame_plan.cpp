// Frame planner: batching, merged buffers, capture planning, layers, fades.
#include "esia/render/frame_plan.hpp"
#include "esia/render/painter.hpp"
#include "esia_test.hpp"
#include <algorithm>
#include <vector>

using namespace esia;
using namespace esia::render;

namespace
{
    DrawData Data(std::initializer_list<const DrawList*> lists, Vec2 size = {400, 300}, Vec2 scale = {1, 1})
    {
        DrawData dd;
        dd.lists = lists;
        dd.displaySize = size;
        dd.framebufferScale = scale;
        return dd;
    }

    GlassMaterial Frosted(float blur = 10.0f)
    {
        GlassMaterial g;
        g.blur = blur;
        return g;
    }

    int Count(const FramePlan& p, RenderOp::Type t)
    {
        int n = 0;
        for (const RenderOp& o : p.ops)
            n += o.type == t;
        return n;
    }

    int Captures(const FramePlan& p)
    {
        int n = 0;
        for (const RenderOp& o : p.ops)
            n += o.type == RenderOp::FxBatch && !o.captureRegion.Empty();
        return n;
    }
}

ESIA_TEST(FramePlan, GeometryOfSeveralListsIsMergedAndRebased)
{
    DrawList a, b;
    a.Reset(Rect(0, 0, 400, 300));
    b.Reset(Rect(0, 0, 400, 300));
    a.AddRectFilled(Rect(0, 0, 10, 10), 0xFFFFFFFFu);
    b.AddRectFilled(Rect(20, 20, 30, 30), 0xFF0000FFu);
    FramePlan p;
    p.Build(Data({&a, &b}), {});
    ESIA_CHECK(p.ops.size() == 1);   // same clip and texture, nothing between: one draw
    ESIA_CHECK(p.vertices.size() == 8 && p.indices.size() == 12);
    ESIA_CHECK(p.ops[0].type == RenderOp::Draw && p.ops[0].idxOffset == 0 && p.ops[0].idxCount == 12);
    ESIA_CHECK(p.indices[6] == 4);   // b's first vertex follows a's four
    ESIA_CHECK(p.vertices[4].color == 0xFF0000FFu);
}

// A direct plan (a device with Caps::baseVertex) leaves the lists' vertices and indices as they are: each list at its
// offsets, every draw a run of a list's indices with the list's first vertex as base - the same indices as the
// rebased plan's, and the same instances, as runs.
ESIA_TEST(FramePlan, DirectPlansDrawTheListsAsTheyAre)
{
    DrawList a, b;
    a.Reset(Rect(0, 0, 400, 300));
    b.Reset(Rect(0, 0, 400, 300));
    Painter pa(a), pb(b);
    a.AddRectFilled(Rect(0, 0, 10, 10), 0xFFFFFFFFu);
    pa.Rect(Rect(100, 100, 150, 150), Style().Fill(Color::White()).Radius(4));
    a.AddRectFilled(Rect(110, 120, 140, 130), 0xFF0000FFu);   // a label on the shape
    b.AddRectFilled(Rect(20, 20, 30, 30), 0xFF00FF00u);
    pb.Rect(Rect(200, 100, 250, 150), Style().Fill(Color::White()).Radius(4));
    FramePlan rebased, direct;
    rebased.Build(Data({&a, &b}), {});
    direct.Build(Data({&a, &b}), {}, true);
    ESIA_CHECK(direct.direct && direct.vertices.empty() && direct.indices.empty() && direct.instances.empty());
    ESIA_CHECK(direct.totalVertices == 12 && direct.totalIndices == 18 && direct.lists.size() == 2);
    ESIA_CHECK(direct.lists[1].list == &b && direct.lists[1].firstVertex == 8 && direct.lists[1].firstIndex == 12);
    ESIA_CHECK(direct.ops.size() == rebased.ops.size());
    bool same = direct.ops.size() == rebased.ops.size();
    for (std::size_t k = 0; same && k < direct.ops.size(); ++k)
    {
        const RenderOp& d = direct.ops[k];
        const RenderOp& r = rebased.ops[k];
        same = d.type == r.type;
        if (d.type == RenderOp::FxBatch)
            same = same && d.instStart == r.instStart && d.instCount == r.instCount;
        if (d.type != RenderOp::Draw)
            continue;
        std::vector<std::uint32_t> got;
        for (std::uint32_t g = d.geometryFirst; g < d.geometryFirst + d.geometryCount; ++g)
        {
            const FramePlan::GeometryDraw& gd = direct.geometry[g];
            for (const FramePlan::ListGeometry& l : direct.lists)
                if (gd.firstIndex >= l.firstIndex && gd.firstIndex < l.firstIndex + l.list->Indices().size())
                    for (std::uint32_t i = 0; i < gd.count; ++i)
                        got.push_back(l.list->Indices()[gd.firstIndex - l.firstIndex + i] + gd.baseVertex);
        }
        same = same && got.size() == r.idxCount && std::equal(got.begin(), got.end(), rebased.indices.begin() + r.idxOffset);
    }
    ESIA_CHECK(same);
    std::vector<float> left;
    for (const FramePlan::InstanceRun& run : direct.instanceRuns)
        for (std::uint32_t i = 0; i < run.count; ++i)
            left.push_back(run.first[i].rect[0]);
    ESIA_CHECK(left.size() == rebased.instances.size() && left.size() == 2 && left[0] == rebased.instances[0].rect[0] &&
               left[1] == rebased.instances[1].rect[0]);
}

ESIA_TEST(FramePlan, FxInstancesBatchUntilTheStateChanges)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 400, 300));
    Painter p(dl);
    p.Rect(Rect(10, 10, 50, 50), Style().Fill(Color::Hex(0xFF0000)).Radius(8));
    p.Circle(Vec2(100, 100), 20, Style().Fill(Color::Hex(0x00FF00)));
    p.Image(7, Rect(200, 10, 260, 70), 6);                       // another texture: a new batch
    p.PushClip(Rect(0, 100, 200, 140));
    p.Rect(Rect(10, 120, 50, 160), Style().Fill(Color::White()));   // cut by a clip the batch is not in: a new batch
    p.PopClip();
    FramePlan plan;
    plan.Build(Data({&dl}), {});
    ESIA_CHECK(plan.fxCount == 4 && plan.instances.size() == 4);
    ESIA_CHECK(Count(plan, RenderOp::FxBatch) == 3);
    ESIA_CHECK(plan.ops.size() == 3);
    if (!(plan.ops.size() == 3))
        return;
    ESIA_CHECK(plan.ops[0].instStart == 0 && plan.ops[0].instCount == 2);
    ESIA_CHECK(plan.ops[1].texture == 7 && plan.ops[1].instStart == 2);
    ESIA_CHECK(plan.ops[2].clip == (PxRect{0, 100, 200, 140}) && !plan.ops[2].clipFree);
    ESIA_CHECK(!plan.anyGlass && plan.plannedCaptures == 0);
}

// A clip that cuts nothing of what it holds (a control's own clip around its shadow) does not split a batch: the
// shape joins the batch of the window's clip, whose scissor holds it whole - the same pixels.
ESIA_TEST(FramePlan, ClipsThatCutNothingDoNotSplitBatches)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 400, 300));
    Painter p(dl);
    p.PushClip(Rect(0, 0, 300, 200));   // the window's body
    p.Rect(Rect(10, 10, 50, 50), Style().Fill(Color::Hex(0xFF0000)).Radius(8));
    p.PushClip(Rect(60, 0, 140, 80), false);   // a control's own clip around it and its shadow
    p.Rect(Rect(80, 20, 120, 60), Style().Fill(Color::Hex(0x00FF00)).Radius(8).Shadow(Color::Black(0.3f), 6.0f));
    p.PopClip();
    dl.AddRectFilled(Rect(20, 100, 60, 110), 0xFFFFFFFFu);   // a label clipped by the body, under its own text
    p.PushClip(Rect(0, 90, 400, 130), false);
    dl.AddRectFilled(Rect(80, 100, 120, 110), 0xFFFFFFFFu);
    p.PopClip();
    p.Rect(Rect(150, 150, 250, 250), Style().Fill(Color::Hex(0x0000FF)));   // cut by the body: fixes the batch's scissor
    p.PopClip();
    FramePlan plan;
    plan.Build(Data({&dl}), {});
    ESIA_CHECK(plan.ops.size() == 2);
    if (!(plan.ops.size() == 2))
        return;
    ESIA_CHECK(plan.ops[0].type == RenderOp::FxBatch && plan.ops[0].instCount == 3);
    ESIA_CHECK(plan.ops[0].clip == (PxRect{0, 0, 300, 200}) && !plan.ops[0].clipFree);
    ESIA_CHECK(plan.ops[1].type == RenderOp::Draw && plan.ops[1].idxCount == 12);
    // the labels keep the scissor before them, which holds them (no scissor change)
    ESIA_CHECK(plan.ops[1].clipFree && plan.ops[1].clip == (PxRect{0, 0, 300, 200}));
}

ESIA_TEST(FramePlan, ShapesAndTextBatchAcrossEachOtherWhereTheyDoNotOverlap)
{
    // two buttons, each a shape with a label on it (a white rectangle stands in for the text)
    DrawList dl;
    dl.Reset(Rect(0, 0, 400, 300));
    Painter p(dl);
    p.Rect(Rect(10, 10, 110, 50), Style().Fill(Color::Hex(0x3060C0)).Radius(8));
    dl.AddRectFilled(Rect(30, 25, 90, 35), 0xFFFFFFFFu);
    p.Rect(Rect(150, 10, 250, 50), Style().Fill(Color::Hex(0x3060C0)).Radius(8));
    dl.AddRectFilled(Rect(170, 25, 230, 35), 0xFFFFFFFFu);
    FramePlan plan;
    plan.Build(Data({&dl}), {});
    ESIA_CHECK(plan.ops.size() == 2);   // both shapes, then both labels
    ESIA_CHECK(plan.ops[0].type == RenderOp::FxBatch && plan.ops[0].instStart == 0 && plan.ops[0].instCount == 2);
    ESIA_CHECK(plan.ops[1].type == RenderOp::Draw && plan.ops[1].idxOffset == 0 && plan.ops[1].idxCount == 12);
    ESIA_CHECK(plan.instances[0].rect[0] == 10.0f && plan.instances[1].rect[0] == 150.0f);   // each in drawing order
    ESIA_CHECK(plan.vertices[plan.indices[0]].pos.x == 30.0f && plan.vertices[plan.indices[6]].pos.x == 170.0f);

    // a shape over the first label stays after it
    DrawList d2;
    d2.Reset(Rect(0, 0, 400, 300));
    Painter q(d2);
    q.Rect(Rect(10, 10, 110, 50), Style().Fill(Color::Hex(0x3060C0)).Radius(8));
    d2.AddRectFilled(Rect(30, 25, 90, 35), 0xFFFFFFFFu);
    q.Rect(Rect(80, 20, 120, 40), Style().Fill(Color::Hex(0xC03060)));
    FramePlan plan2;
    plan2.Build(Data({&d2}), {});
    ESIA_CHECK(plan2.ops.size() == 3);
    ESIA_CHECK(plan2.ops[1].type == RenderOp::Draw && plan2.ops[2].type == RenderOp::FxBatch);

    // nothing joins a batch across a callback
    DrawList d3;
    d3.Reset(Rect(0, 0, 400, 300));
    Painter r(d3);
    r.Rect(Rect(10, 10, 110, 50), Style().Fill(Color::Hex(0x3060C0)));
    d3.AddCallback([](const DrawList&, const DrawCmd&, void*) {}, nullptr);
    r.Rect(Rect(150, 10, 250, 50), Style().Fill(Color::Hex(0x3060C0)));
    FramePlan plan3;
    plan3.Build(Data({&d3}), {});
    ESIA_CHECK(plan3.ops.size() == 3 && plan3.ops[2].type == RenderOp::FxBatch && plan3.ops[2].instStart == 1);
}

// A form: rows of a label, a button with a shadow beside it, the button's title and a switch. The shadows' faint tails
// reach the labels, what they show does not: the shapes batch together and the text after them; a batch's text keeps
// its pieces apart (a title and the next row's label do not claim the gap between them, where the next button lies).
ESIA_TEST(FramePlan, ShadowsBatchByWhatTheyShow)
{
    auto form = [](DrawList& dl, float buttonX) {
        dl.Reset(Rect(0, 0, 400, 300));
        Painter p(dl);
        for (int row = 0; row < 2; ++row)
        {
            const float y = 20.0f + 60.0f * (float)row;
            dl.AddRectFilled(Rect(10, y, 60, y + 14), 0xFFFFFFFFu);   // the label
            p.Rect(Rect(buttonX, y - 6, buttonX + 80, y + 22), Style().Fill(Color::Hex(0x3060C0)).Radius(14).Shadow(Color::Black(0.3f), 12.0f, Vec2(0, 4)));
            dl.AddRectFilled(Rect(buttonX + 10, y, buttonX + 70, y + 14), 0xFFFFFFFFu);   // its title
            p.Rect(Rect(300, y - 4, 350, y + 18), Style().Fill(Color::Hex(0x30C060)).Radius(11));   // a switch
        }
    };
    DrawList dl;
    form(dl, 70.0f);   // the shadow's reach (25 px) passes the label's end, its visible part (6) does not
    FramePlan plan;
    plan.Build(Data({&dl}), {});
    ESIA_CHECK(plan.ops.size() == 3);   // the first label, the shapes, the rest of the text
    if (plan.ops.size() == 3)
    {
        ESIA_CHECK(plan.ops[0].type == RenderOp::Draw && plan.ops[0].idxCount == 6);
        ESIA_CHECK(plan.ops[1].type == RenderOp::FxBatch && plan.ops[1].instCount == 4);
        ESIA_CHECK(plan.ops[2].type == RenderOp::Draw && plan.ops[2].idxCount == 18);
    }

    // a button right beside its label: what its shadow shows covers the label's end, so each row keeps its order
    DrawList d2;
    form(d2, 62.0f);
    FramePlan plan2;
    plan2.Build(Data({&d2}), {});
    ESIA_CHECK(plan2.ops.size() == 5);
}

ESIA_TEST(FramePlan, GlassKeepsItsPlace)
{
    // a shape, a label, a glass panel, another label - all apart. The second label joins the first unless the glass
    // lies between them: nothing moves across glass (its backdrop capture stays where it was)
    for (const bool glass : {true, false})
    {
        DrawList dl;
        dl.Reset(Rect(0, 0, 400, 300));
        Painter p(dl);
        p.Rect(Rect(0, 0, 60, 60), Style().Fill(Color::Hex(0x336699)));
        dl.AddRectFilled(Rect(100, 10, 140, 20), 0xFFFFFFFFu);
        const Style panel = glass ? Style().Radius(12).Glass(Frosted()) : Style().Radius(12).Fill(Color::Hex(0x808080));
        p.Rect(Rect(200, 100, 290, 160), panel);
        dl.AddRectFilled(Rect(100, 250, 140, 260), 0xFFFFFFFFu);
        FramePlan plan;
        plan.Build(Data({&dl}), {});
        if (glass)
        {
            ESIA_CHECK(plan.ops.size() == 4 && plan.ops[2].glass && plan.ops[3].type == RenderOp::Draw);
            ESIA_CHECK(plan.ops[0].instCount == 1);   // the glass did not join the first shape either
        }
        else
        {
            ESIA_CHECK(plan.ops.size() == 2 && plan.ops[0].instCount == 2 && plan.ops[1].idxCount == 12);
        }
    }
}

ESIA_TEST(FramePlan, GlassOnGlassRecapturesButNeighboursShare)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 400, 300));
    Painter p(dl);
    p.Rect(Rect(0, 0, 400, 300), Style().Fill(Paint::Linear(Color::Hex(0x203050), Color::Hex(0x805030))));   // wallpaper
    p.Rect(Rect(40, 40, 360, 260), Style().Radius(28).Glass(Frosted()));                                         // glass card
    p.Capsule(Rect(60, 200, 160, 236), Style().Glass(Frosted(2)));                                                // two buttons on it
    p.Capsule(Rect(240, 200, 340, 236), Style().Glass(Frosted(2)));
    FramePlan plan;
    plan.Build(Data({&dl}), {});
    ESIA_CHECK(plan.anyGlass);
    ESIA_CHECK(Count(plan, RenderOp::FxBatch) == 3);   // wallpaper | card | both buttons (they do not overlap)
    ESIA_CHECK(plan.ops[2].instCount == 2);
    // the card sees the wallpaper; the buttons must see the card: a second capture, shared by both buttons
    ESIA_CHECK(Captures(plan) == 2 && plan.plannedCaptures == 2);
    ESIA_CHECK(!plan.ops[1].captureRegion.Empty() && !plan.ops[2].captureRegion.Empty());
    ESIA_CHECK(plan.ops[2].captureRegion.Contains(plan.ops[2].glassRegion));
}

ESIA_TEST(FramePlan, OneCaptureServesGlassThatNothingCovers)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 400, 300));
    Painter p(dl);
    p.Rect(Rect(0, 0, 400, 300), Style().Fill(Color::Hex(0x336699)));
    p.Rect(Rect(20, 20, 180, 120), Style().Radius(20).Glass(Frosted()));
    p.Rect(Rect(360, 260, 390, 290), Style().Fill(Color::Hex(0xFF0000)));   // drawn far from the second glass
    p.Rect(Rect(200, 20, 360, 120), Style().Radius(20).Glass(Frosted()));
    FramePlan plan;
    plan.Build(Data({&dl}), {});
    ESIA_CHECK(Count(plan, RenderOp::FxBatch) == 3);                       // wallpaper | first glass + red square | second glass
    ESIA_CHECK(Captures(plan) == 1);                                       // look-ahead: the first capture covers both
    ESIA_CHECK(plan.ops[1].captureRegion.Contains(plan.ops[2].glassRegion));

    // the same, but the red square now lies under the second glass: it must be captured again
    DrawList dl2;
    dl2.Reset(Rect(0, 0, 400, 300));
    Painter q(dl2);
    q.Rect(Rect(0, 0, 400, 300), Style().Fill(Color::Hex(0x336699)));
    q.Rect(Rect(20, 20, 180, 120), Style().Radius(20).Glass(Frosted()));
    q.Rect(Rect(250, 50, 300, 90), Style().Fill(Color::Hex(0xFF0000)));
    q.Rect(Rect(200, 20, 360, 120), Style().Radius(20).Glass(Frosted()));
    FramePlan plan2;
    plan2.Build(Data({&dl2}), {});
    ESIA_CHECK(Captures(plan2) == 2);
}

ESIA_TEST(FramePlan, PyramidLevelsAndLevelZeroFollowTheGlass)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 400, 300));
    Painter p(dl);
    p.Rect(Rect(0, 0, 400, 300), Style().Fill(Color::Hex(0x336699)));
    p.Rect(Rect(20, 20, 180, 120), Style().Radius(20).Glass(Frosted(10)));
    FramePlan frosted;
    frosted.Build(Data({&dl}), {});
    const RenderOp& f = frosted.ops[1];
    ESIA_CHECK(!f.readsLevel0 && !f.captureLevel0);   // frost only reads blurred levels: the target can be read directly
    ESIA_CHECK(f.captureLevels == 4);                 // env blur 16 px: log2(16) - 1 = 3 -> levels 1..4
    ESIA_CHECK(f.blurPx == 16.0f);

    DrawList dl2;
    dl2.Reset(Rect(0, 0, 400, 300));
    Painter q(dl2);
    q.Rect(Rect(0, 0, 400, 300), Style().Fill(Color::Hex(0x336699)));
    q.Rect(Rect(20, 20, 180, 120), Style().Radius(20).Glass(Frosted(0)));   // clear glass reads level 0
    FramePlan clear;
    clear.Build(Data({&dl2}, {400, 300}, {2, 2}), {});
    ESIA_CHECK(clear.ops[1].readsLevel0 && clear.ops[1].captureLevel0);
    ESIA_CHECK(clear.ops[1].blurPx == 32.0f);          // 16 UI units at render scale 2
    ESIA_CHECK(clear.ops[1].glassShape == (PxRect{40, 40, 360, 240}));
}

ESIA_TEST(FramePlan, UserEffectsBuildTheWholePyramid)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 400, 300));
    Painter p(dl);
    p.Rect(Rect(10, 10, 100, 100), Style().Fill(Color::White()).Effect(3, 1.0f));
    FramePlan plan;
    plan.Build(Data({&dl}), {});
    ESIA_CHECK(plan.anyGlass);
    ESIA_CHECK(plan.ops[0].effect == 3 && plan.ops[0].blurPx < 0.0f && plan.ops[0].captureLevels == 5 && plan.ops[0].readsLevel0);
}

ESIA_TEST(FramePlan, GlowLayersAndTheirBounds)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 400, 300));
    Painter p(dl, PainterEnv{2.0f, 0.6f, 1.0f, 1.0f, nullptr, nullptr});   // metrics scale 2
    p.BeginGlowLayer(Color::Hex(0x00FFFF, 0.5f), 10.0f, 1.5f);
    p.Rect(Rect(100, 100, 140, 120), Style().Fill(Color::White()));
    p.EndGlowLayer();
    p.BeginGlowLayer(Color::White(), 4.0f);   // left open: closed by the planner
    p.Rect(Rect(10, 10, 20, 20), Style().Fill(Color::White()));
    FramePlan plan;
    plan.Build(Data({&dl}), {});
    ESIA_CHECK(plan.anyLayer);
    ESIA_CHECK(Count(plan, RenderOp::LayerBegin) == 2 && Count(plan, RenderOp::LayerEnd) == 2);
    const RenderOp& begin = plan.ops[0];
    const RenderOp& end = plan.ops[2];
    ESIA_CHECK(begin.type == RenderOp::LayerBegin && end.type == RenderOp::LayerEnd);
    ESIA_CHECK(end.layer.radius == 20.0f && end.blurPx == 20.0f);       // 10 UI units at metrics scale 2
    ESIA_CHECK_NEAR(end.layer.intensity, 1.5f, 1e-6f);
    ESIA_CHECK(end.bounds == begin.bounds);
    ESIA_CHECK(end.bounds.Contains(plan.ops[1].bounds.Expand(20.0f * 2.5f)));
    ESIA_CHECK(plan.ops.back().type == RenderOp::LayerEnd);
}

ESIA_TEST(FramePlan, EdgeFadesInPixelsAndNeverMixed)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 400, 300));
    Painter p(dl);
    p.BeginEdgeFade(Rect(0, 50, 400, 250), 10, 20);
    p.Rect(Rect(10, 60, 50, 100), Style().Fill(Color::White()));
    dl.AddRectFilled(Rect(60, 60, 90, 90), 0xFFFFFFFFu);
    p.EndEdgeFade();
    p.Rect(Rect(10, 110, 50, 150), Style().Fill(Color::White()));
    FramePlan plan;
    plan.Build(Data({&dl}, {400, 300}, {2, 2}), {});
    ESIA_CHECK(plan.ops.size() == 3);
    const float want[4] = {100, 500, 20, 40};
    for (int k = 0; k < 4; ++k)
    {
        ESIA_CHECK(plan.ops[0].fade[k] == want[k]);
        ESIA_CHECK(plan.ops[1].fade[k] == want[k]);
        ESIA_CHECK(plan.ops[2].fade[k] == 0.0f);
    }
}

ESIA_TEST(FramePlan, CoveragePagesPickTheTextProgram)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 400, 300));
    dl.AddImage(5, Rect(0, 0, 10, 10), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    dl.AddImage(6, Rect(0, 20, 10, 30), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    dl.AddImage(9, Rect(0, 40, 10, 50), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    FramePlan plan;
    plan.Build(Data({&dl}), [](TextureId id, TextureInfo& out) {
        if (id == 5)
            out = {TextureFormat::Alpha8, 256, 256};
        else if (id == 6)
            out = {TextureFormat::RGBA8, 256, 256};
        else
            return false;
        return true;
    });
    ESIA_CHECK(plan.ops.size() == 3);
    ESIA_CHECK(plan.ops[0].coverage);                            // an Alpha8 glyph page
    ESIA_CHECK(!plan.ops[1].coverage && !plan.ops[2].coverage);   // an image, an unknown texture
}

ESIA_TEST(FramePlan, CallbacksAndDisplayMapping)
{
    DrawList dl;
    dl.Reset(Rect(100, 50, 500, 350));
    dl.PushClipRect(Rect(110, 60, 210, 160));
    dl.AddCallback([](const DrawList&, const DrawCmd&, void*) {}, nullptr);
    dl.AddRectFilled(Rect(120, 70, 130, 80), 0xFFFFFFFFu);
    DrawData dd = Data({&dl}, {400, 300}, {1.5f, 1.5f});
    dd.displayPos = Vec2(100, 50);
    FramePlan plan;
    plan.Build(dd, {});
    ESIA_CHECK(plan.ops.size() == 2 && plan.ops[0].type == RenderOp::Callback && plan.ops[0].list == &dl);
    ESIA_CHECK(plan.ops[0].clip == (PxRect{15, 15, 165, 165}));
    ESIA_CHECK(plan.ops[1].clip == (PxRect{15, 15, 165, 165}));
}
