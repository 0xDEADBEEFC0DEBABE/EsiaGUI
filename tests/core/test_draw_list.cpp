#include "esia/core/draw_list.hpp"
#include "esia_test.hpp"
#include <cstring>

using namespace esia;

ESIA_TEST(DrawList, GeometryMergesUntilStateChanges)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 100, 100));
    dl.AddRectFilled(Rect(0, 0, 10, 10), 0xFFFFFFFFu);
    dl.AddRectFilled(Rect(10, 0, 20, 10), 0xFFFFFFFFu);
    ESIA_CHECK(dl.Commands().size() == 1);
    ESIA_CHECK(dl.Commands()[0].count == 12);
    ESIA_CHECK(dl.Vertices().size() == 8);
    // indices are local to the list
    ESIA_CHECK(dl.Indices()[6] == 4);

    dl.PushClipRect(Rect(0, 0, 50, 50));
    dl.AddRectFilled(Rect(0, 0, 10, 10), 0xFFFFFFFFu);
    dl.PopClipRect();
    ESIA_CHECK(dl.Commands().size() == 2);
    ESIA_CHECK(dl.Commands()[1].clip == Rect(0, 0, 50, 50));

    dl.AddImage(42, Rect(0, 0, 4, 4), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    ESIA_CHECK(dl.Commands().size() == 3);
    ESIA_CHECK(dl.Commands()[2].texture == 42);
    ESIA_CHECK(dl.Commands()[2].first == 18);
}

// A quad run (a text's glyphs) leaves exactly what AddQuad one by one does: the same instances and commands, a
// command per texture continued while nothing else comes between, with fewer quads written than reserved. Quads are
// commands of their own beside geometry of the same texture.
ESIA_TEST(DrawList, QuadRunsMatchSingleQuads)
{
    DrawList a, b;
    a.Reset(Rect(0, 0, 100, 100));
    b.Reset(Rect(0, 0, 100, 100));
    for (DrawList* dl : {&a, &b})
    {
        dl->AddRectFilled(Rect(0, 0, 10, 10), 0xFFFFFFFFu);
        dl->AddQuad(7, Rect(1, 1, 2, 2), Vec2(0, 0), Vec2(0.5f, 0.5f), 0xFF0000FFu);
    }
    const Rect glyphs[3] = {Rect(10, 10, 14, 18), Rect(15, 10, 19, 18), Rect(20, 9, 24, 18)};
    for (const Rect& r : glyphs)
        a.AddQuad(7, r, Vec2(0.25f, 0.5f), Vec2(0.75f, 1.0f), 0xFF00FF00u);
    a.AddQuad(9, Rect(30, 10, 40, 20), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    DrawList::QuadWriter w = b.BeginQuads(7, 5);   // two more reserved than written
    for (const Rect& r : glyphs)
        w.Add(r, Vec2(0.25f, 0.5f), Vec2(0.75f, 1.0f), 0xFF00FF00u);
    b.EndQuads(w);
    w = b.BeginQuads(9, 1);
    w.Add(Rect(30, 10, 40, 20), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    b.EndQuads(w);
    ESIA_CHECK(a.Quads().size() == 5 && b.Quads().size() == 5);
    ESIA_CHECK(std::memcmp(a.Quads().data(), b.Quads().data(), a.Quads().size() * sizeof(QuadInstance)) == 0);
    ESIA_CHECK(a.Commands().size() == 3 && b.Commands().size() == 3);   // white geometry, quads of page 7 (continued), page 9
    for (std::size_t i = 0; i < a.Commands().size() && i < b.Commands().size(); ++i)
        ESIA_CHECK(a.Commands()[i].kind == b.Commands()[i].kind && a.Commands()[i].texture == b.Commands()[i].texture &&
                   a.Commands()[i].first == b.Commands()[i].first && a.Commands()[i].count == b.Commands()[i].count);
    ESIA_CHECK(a.Commands()[0].kind == DrawCmdKind::Geometry && a.Commands()[1].kind == DrawCmdKind::Quads && a.Commands()[1].count == 4);
    // nothing for an empty rect or a transparent color
    a.AddQuad(9, Rect(5, 5, 5, 9), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    a.AddQuad(9, Rect(5, 5, 9, 9), Vec2(0, 0), Vec2(1, 1), 0x00FFFFFFu);
    ESIA_CHECK(a.Quads().size() == 5);
}

// A quad's uv and color as the shaders read them: whole numbers, uv in 1/4096 (whole texels of a 2048 page: even
// numbers), the color's R + 256 G + 65536 B and its A.
ESIA_TEST(DrawList, QuadsPackTheirUvAndColor)
{
    QuadInstance q;
    q.Set(Rect(1.5f, 2, 3, 4.25f), Vec2(10.0f / 2048.0f, 2047.0f / 2048.0f), Vec2(16.0f / 2048.0f, 1.0f), 0x80FF2010u);
    ESIA_CHECK(q.rect[0] == 1.5f && q.rect[1] == 2.0f && q.rect[2] == 3.0f && q.rect[3] == 4.25f);
    ESIA_CHECK(q.packed[0] == 20.0f + 4096.0f * 4094.0f);   // below 2^24: exact
    ESIA_CHECK(q.packed[1] == 12.0f + 4096.0f * 2.0f);
    ESIA_CHECK(q.packed[2] == (float)(0x10 + 256 * 0x20 + 65536 * 0xFF) && q.packed[3] == 128.0f);
}

// Every geometry command keeps the bounds of its vertices as they are written (the planner's), through each way of
// writing them, and again after positions were rewritten (Painter::PopScale).
ESIA_TEST(DrawList, CommandsKeepTheirVertexBounds)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 100, 100));
    dl.AddRectFilled(Rect(10, 20, 30, 40), 0xFFFFFFFFu);
    dl.AddTriangleFilled(Vec2(50, 5), Vec2(60, 50), Vec2(40, 45), 0xFFFFFFFFu);   // WriteVertex, the same command
    ESIA_CHECK(dl.Commands().size() == 1);
    ESIA_CHECK(dl.Commands()[0].vtxFirst == 0 && dl.Commands()[0].vtxEnd == 7 && dl.Commands()[0].vtxBounds == Rect(10, 5, 60, 50));
    dl.PushTexture(5);
    const DrawList::PrimWriter pw = dl.PrimReserve(6, 4);   // written in place
    for (int i = 0; i < 4; ++i)
        pw.vtx[i] = {Vec2(i & 1 ? 80.0f : 70.0f, i & 2 ? 90.0f : 70.0f), Vec2(0, 0), 0xFFFFFFFFu};
    for (int i = 0; i < 6; ++i)
        pw.idx[i] = pw.base + (std::uint32_t)(i < 3 ? i : i - 2);
    dl.PrimCommit(6, Rect(70, 70, 80, 90));
    dl.PopTexture();
    ESIA_CHECK(dl.Commands().size() == 2 && dl.Commands()[1].texture == 5);
    ESIA_CHECK(dl.Commands()[1].vtxFirst == 7 && dl.Commands()[1].vtxEnd == 11 && dl.Commands()[1].vtxBounds == Rect(70, 70, 80, 90));
    DrawList::QuadWriter w = dl.BeginQuads(5, 4);   // quads: the bounds of their rects (more reserved than written)
    w.Add(Rect(60, 10, 64, 18), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    w.Add(Rect(66, 12, 70, 20), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    dl.EndQuads(w);
    ESIA_CHECK(dl.Commands().size() == 3 && dl.Commands()[2].kind == DrawCmdKind::Quads && dl.Commands()[2].vtxBounds == Rect(60, 10, 70, 20));
    for (std::size_t i = 7; i < 11; ++i)
        dl.Vertices()[i].pos = Vec2(dl.Vertices()[i].pos.x * 0.5f, dl.Vertices()[i].pos.y * 0.5f);
    dl.RefreshBounds(7);
    ESIA_CHECK(dl.Commands()[1].vtxBounds == Rect(35, 35, 40, 45) && dl.Commands()[0].vtxBounds == Rect(10, 5, 60, 50));
    ESIA_CHECK(dl.Commands()[2].vtxBounds == Rect(60, 10, 70, 20));   // quads only when asked
    dl.Quads()[1].rect[2] = 90.0f;
    dl.RefreshBounds(11, 1);
    ESIA_CHECK(dl.Commands()[2].vtxBounds == Rect(60, 10, 90, 20));
}

ESIA_TEST(DrawList, TransparentGeometryIsSkipped)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 100, 100));
    dl.AddRectFilled(Rect(0, 0, 10, 10), 0x00FFFFFFu);
    dl.AddTriangleFilled(Vec2(0, 0), Vec2(1, 0), Vec2(0, 1), 0x00000000u);
    ESIA_CHECK(dl.Vertices().empty());
}

ESIA_TEST(DrawList, ClipIntersectsAndNeverInverts)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 100, 100));
    dl.PushClipRect(Rect(50, 50, 200, 200));
    ESIA_CHECK(dl.ClipRect() == Rect(50, 50, 100, 100));
    dl.PushClipRect(Rect(0, 0, 10, 10));
    ESIA_CHECK(dl.ClipRect().Empty());
    dl.PopClipRect();
    dl.PushClipRect(Rect(0, 0, 10, 10), false);
    ESIA_CHECK(dl.ClipRect() == Rect(0, 0, 10, 10));
}

ESIA_TEST(DrawList, FxCommandsBatchAndInterleave)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 100, 100));
    fx::Instance a;
    std::memset(&a, 0, sizeof(a));
    a.flags[0] = fx::kFill;
    dl.AddFx(a);
    dl.AddFx(a);
    dl.AddFx(a, 3);   // other effect: new command
    dl.AddRectFilled(Rect(0, 0, 1, 1), 0xFFFFFFFFu);
    dl.AddFx(a);      // after geometry: new command, order kept
    const auto& c = dl.Commands();
    ESIA_CHECK(c.size() == 4);
    ESIA_CHECK(c[0].kind == DrawCmdKind::Fx && c[0].count == 2 && c[0].first == 0);
    ESIA_CHECK(c[1].kind == DrawCmdKind::Fx && c[1].effect == 3 && c[1].first == 2);
    ESIA_CHECK(c[2].kind == DrawCmdKind::Geometry);
    ESIA_CHECK(c[3].kind == DrawCmdKind::Fx && c[3].first == 3);
    ESIA_CHECK(dl.FxInstances().size() == 4);
}

ESIA_TEST(DrawList, LayersFadesCallbacks)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 100, 100));
    fx::LayerParams lp{};
    lp.radius = 12.0f;
    dl.BeginLayer(lp);
    dl.BeginFade({0, 100, 8, 8});
    dl.EndFade();
    dl.EndLayer();
    static int calls = 0;
    dl.AddCallback([](const DrawList&, const DrawCmd&, void*) { ++calls; }, nullptr);
    const auto& c = dl.Commands();
    ESIA_CHECK(c.size() == 5);
    ESIA_CHECK(c[0].kind == DrawCmdKind::LayerBegin && dl.Layers()[c[0].payload].radius == 12.0f);
    ESIA_CHECK(c[1].kind == DrawCmdKind::FadeBegin && dl.Fades()[c[1].payload].top == 8.0f);
    ESIA_CHECK(c[4].kind == DrawCmdKind::Callback);
    c[4].callback(dl, c[4], nullptr);
    ESIA_CHECK(calls == 1);
}

ESIA_TEST(DrawList, ConvexPolyFan)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 100, 100));
    const Vec2 pts[5] = {{0, 0}, {10, 0}, {12, 5}, {10, 10}, {0, 10}};
    dl.AddConvexPolyFilled(pts, 5, 0xFF0000FFu);
    ESIA_CHECK(dl.Indices().size() == 9);
    ESIA_CHECK(dl.Vertices().size() == 5);
}

// A card draws its background after its content (the height is known at its end) and moves it under the content.
ESIA_TEST(DrawList, MoveCommandsUnderEarlierContent)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 100, 100));
    dl.AddRectFilled(Rect(0, 0, 5, 5), 0xFF0000FFu);    // before the card
    const std::size_t under = dl.Mark();
    dl.AddRectFilled(Rect(10, 10, 20, 20), 0xFF00FF00u);   // content: does not merge into the command before the mark
    fx::Instance shape{};
    dl.AddFx(shape);
    ESIA_CHECK(dl.Commands().size() == 3);
    const std::size_t bg = dl.Mark();
    dl.AddRectFilled(Rect(8, 8, 30, 30), 0xFFFFFFFFu);     // the background, drawn last ...
    dl.AddFx(shape);
    ESIA_CHECK(dl.Commands().size() == 5);                 // ... in commands of its own (the mark stops merging)
    dl.MoveCommands(bg, under);                             // ... and moved under the content
    const auto& c = dl.Commands();
    ESIA_CHECK(c.size() == 5);
    ESIA_CHECK(c[0].kind == DrawCmdKind::Geometry && c[0].first == 0 && c[0].count == 6);
    ESIA_CHECK(c[1].kind == DrawCmdKind::Geometry && c[1].first == 12 && c[1].count == 6);   // the background
    ESIA_CHECK(c[2].kind == DrawCmdKind::Fx && c[2].first == 1 && c[2].count == 1);
    ESIA_CHECK(c[3].kind == DrawCmdKind::Geometry && c[3].first == 6 && c[3].count == 6);     // the content
    ESIA_CHECK(c[4].kind == DrawCmdKind::Fx && c[4].first == 0 && c[4].count == 1);
    // what comes next never merges into a moved command
    dl.AddRectFilled(Rect(0, 0, 1, 1), 0xFFFFFFFFu);
    dl.AddFx(shape);
    ESIA_CHECK(c.size() == 7 && c[5].first == 18 && c[6].first == 2);
    dl.AddFx(shape);
    ESIA_CHECK(c.size() == 7 && c[6].count == 2);          // ... but new commands merge again
}
