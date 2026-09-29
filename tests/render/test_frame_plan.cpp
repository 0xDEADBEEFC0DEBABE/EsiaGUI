// Frame planner: batching, merged buffers, capture planning, layers, fades.
#include "esia/render/frame_plan.hpp"
#include "esia/render/painter.hpp"
#include "esia_test.hpp"

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
    ESIA_CHECK(p.ops.size() == 2);
    ESIA_CHECK(p.vertices.size() == 8 && p.indices.size() == 12);
    ESIA_CHECK(p.ops[1].type == RenderOp::Draw && p.ops[1].idxOffset == 6 && p.ops[1].idxCount == 6);
    ESIA_CHECK(p.indices[6] == 4);   // b's first vertex follows a's four
    ESIA_CHECK(p.vertices[4].color == 0xFF0000FFu);
}

ESIA_TEST(FramePlan, FxInstancesBatchUntilTheStateChanges)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 400, 300));
    Painter p(dl);
    p.Rect(Rect(10, 10, 50, 50), Style().Fill(Color::Hex(0xFF0000)).Radius(8));
    p.Circle(Vec2(100, 100), 20, Style().Fill(Color::Hex(0x00FF00)));
    p.Image(7, Rect(200, 10, 260, 70), 6);                       // another texture: a new batch
    p.PushClip(Rect(0, 0, 200, 200));
    p.Rect(Rect(10, 120, 50, 160), Style().Fill(Color::White()));   // another clip: a new batch
    p.PopClip();
    FramePlan plan;
    plan.Build(Data({&dl}), {});
    ESIA_CHECK(plan.fxCount == 4 && plan.instances.size() == 4);
    ESIA_CHECK(Count(plan, RenderOp::FxBatch) == 3);
    ESIA_CHECK(plan.ops[0].instStart == 0 && plan.ops[0].instCount == 2);
    ESIA_CHECK(plan.ops[1].texture == 7 && plan.ops[1].instStart == 2);
    ESIA_CHECK(plan.ops[2].clip == (PxRect{0, 0, 200, 200}));
    ESIA_CHECK(!plan.anyGlass && plan.plannedCaptures == 0);
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
