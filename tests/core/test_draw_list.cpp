#include "esia/core/draw_list.hpp"
#include "esia_test.hpp"
#include <cmath>
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

// A shape's fields packed into the quad's two rows (QuadInstance::SetShape): kind, smoothing in 1/1023 and stroke align
// in 1/64 in one negative number, the radius as it is, the stroke width in 1/64 beside the alpha - every one a whole
// number below 2^24, exact in a float; a glyph's packed uv is never negative.
ESIA_TEST(DrawList, ShapeQuadsPackTheirFields)
{
    QuadInstance q;
    q.SetShape(QuadShape::SegmentStroke, Rect(Vec2(5, 6), Vec2(-7, 8)), 2.75f, 0.6f, 1.5f, 0.5f, 0x80402010u);
    ESIA_CHECK(q.IsShape() && q.Shape() == QuadShape::SegmentStroke && q.Radius() == 2.75f);
    ESIA_CHECK(std::fabs(q.Smoothing() - 0.6f) < 0.5f / 1023.0f && q.StrokeWidth() == 1.5f && q.StrokeAlign() == 0.5f && q.Alpha() == 0x80u);
    ESIA_CHECK(q.packed[2] == (float)0x402010 && q.rect[2] == -7.0f);
    q.SetShape(QuadShape::Stroke, Rect(0, 0, 10, 10), 4.0f, 1.0f, 1023.0f, 1.0f, 0xFFFFFFFFu);   // the largest fields
    ESIA_CHECK(q.Shape() == QuadShape::Stroke && q.Smoothing() == 1.0f && q.StrokeAlign() == 1.0f && q.StrokeWidth() == 1023.0f && q.Alpha() == 255u);
    ESIA_CHECK(-q.packed[0] - 1.0f < 16777216.0f && q.packed[3] < 16777216.0f);
    ESIA_CHECK(q.Bounds() == Rect(-1023, -1023, 1033, 1033));   // the stroke all outside
    QuadInstance g;
    g.Set(Rect(0, 0, 4, 4), Vec2(0, 0), Vec2(0.5f, 0.5f), 0xFFFFFFFFu);
    ESIA_CHECK(!g.IsShape() && g.Bounds() == Rect(0, 0, 4, 4));
}

// A polyline's piece and end (QuadInstance::SetLinePiece / SetLineCap): the joints' angles, the reach, the end's sides
// and the alpha come back whole from their largest values (the code just below 2^24); a piece's bounds go as far as its
// reach past its end points, an end's are its half disc's.
ESIA_TEST(DrawList, LineQuadsPackTheirFields)
{
    QuadInstance q;
    q.SetLinePiece(Vec2(5, 6), Vec2(-7, 8), 2.0f, 65535u, 65535u, 31u, 0xFFFFFFFFu);
    ESIA_CHECK(q.IsShape() && q.Shape() == QuadShape::LinePiece && q.Radius() == 2.0f && q.Alpha() == 255u);
    ESIA_CHECK(q.LineJoint() == 65535u && q.LineReach() == 31u && q.LineOther() == 65535u);
    ESIA_CHECK(-q.packed[0] - 1.0f < 16777216.0f && q.packed[3] < 16777216.0f);
    q.SetLinePiece(Vec2(5, 6), Vec2(-7, 8), 2.0f, 16384u, 0u, 8u, 0x80FFFFFFu);
    ESIA_CHECK(q.LineJoint() == 16384u && q.LineReach() == 8u && q.LineOther() == 0u && q.Alpha() == 0x80u);
    ESIA_CHECK(q.Bounds() == Rect(-10, 3, 8, 11));   // 1.5 half thicknesses past the end points
    q.SetLineCap(Vec2(5, 6), Vec2(-7, 8), 2.0f, 32768u, 3u, 12u, 0xFF0000FFu);
    ESIA_CHECK(q.Shape() == QuadShape::LineCap && q.LineJoint() == 32768u && q.LineReach() == 3u && q.LineOther() == 12u);
    ESIA_CHECK(q.Bounds() == Rect(3, 4, 7, 8));
}

// A plain shape is a quad that reads no texture (QuadInstance::SetShape): it joins the quads before it whatever their
// texture, a command of shapes alone takes the texture of the first textured quad, and the command's bounds hold what
// the shapes reach - the part of a stroke outside the edge, a segment's thickness round its end points.
ESIA_TEST(DrawList, ShapeQuadsJoinQuadsOnAnyTexture)
{
    DrawList dl;
    dl.Reset(Rect(0, 0, 400, 300));
    QuadInstance fill, line, seg, clear;
    fill.SetShape(QuadShape::Fill, Rect(10, 10, 50, 30), 4.0f, 0.6f, 0.0f, 0.0f, 0xFF0000FFu);
    line.SetShape(QuadShape::Stroke, Rect(10, 10, 50, 30), 4.0f, 0.6f, 2.0f, 1.0f, 0xFFFFFFFFu);    // 2 wide, outside
    seg.SetShape(QuadShape::SegmentFill, Rect(Vec2(100, 80), Vec2(60, 40)), 3.0f, 0.0f, 0.0f, 0.0f, 0xFF00FF00u);   // end points
    clear.SetShape(QuadShape::Fill, Rect(0, 0, 5, 5), 0.0f, 0.0f, 0.0f, 0.0f, 0x00FFFFFFu);
    dl.AddShapeQuad(fill);
    dl.AddShapeQuad(line);
    ESIA_CHECK(dl.Commands().size() == 1 && dl.Commands()[0].anyTexture && dl.Commands()[0].texture == 0);
    dl.AddQuad(42, Rect(12, 12, 18, 22), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);   // a glyph: its page for the command
    dl.AddShapeQuad(seg);
    dl.AddQuad(42, Rect(20, 12, 26, 22), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    dl.AddShapeQuad(clear);   // transparent: nothing
    ESIA_CHECK(dl.Commands().size() == 1 && dl.Quads().size() == 5);
    const DrawCmd& c = dl.Commands()[0];
    ESIA_CHECK(c.kind == DrawCmdKind::Quads && c.count == 5 && c.texture == 42 && !c.anyTexture && c.shapes);
    ESIA_CHECK(c.vtxBounds == Rect(8, 8, 103, 83));
    ESIA_CHECK(dl.Quads()[1].IsShape() && dl.Quads()[1].Shape() == QuadShape::Stroke && !dl.Quads()[2].IsShape());
    ESIA_CHECK(dl.Quads()[3].rect[0] == 100.0f && dl.Quads()[3].rect[3] == 40.0f);   // the end points as they came
    // quads on another page: a command of their own; a shape after them joins it
    dl.AddQuad(43, Rect(30, 12, 36, 22), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    dl.AddShapeQuad(fill);
    ESIA_CHECK(dl.Commands().size() == 2 && dl.Commands()[1].texture == 43 && dl.Commands()[1].count == 2 && dl.Commands()[1].shapes);
    // another clip: another command, of shapes alone until a glyph comes
    dl.PushClipRect(Rect(0, 0, 100, 100));
    dl.AddShapeQuad(fill);
    ESIA_CHECK(dl.Commands().size() == 3 && dl.Commands()[2].anyTexture);
    dl.PopClipRect();
}

// Plain coverage (QuadInstance::SetCoverage: an image's alpha in a glyph page, drawn as the image): a glyph's quad with
// 256 more beside its alpha - exact, the alpha as it was - in the command of the glyphs on its page, which then draws
// with the programs that tell the two apart (DrawCmd::shapes).
ESIA_TEST(DrawList, CoverageQuadsJoinTheGlyphsOfTheirPage)
{
    QuadInstance q;
    q.SetCoverage(Rect(1.5f, 2, 3, 4.25f), Vec2(10.0f / 2048.0f, 2047.0f / 2048.0f), Vec2(16.0f / 2048.0f, 1.0f), 0x80FF2010u);
    ESIA_CHECK(q.packed[0] == 20.0f + 4096.0f * 4094.0f && q.packed[1] == 12.0f + 4096.0f * 2.0f);
    ESIA_CHECK(q.packed[2] == (float)(0x10 + 256 * 0x20 + 65536 * 0xFF) && q.packed[3] == 256.0f + 128.0f);
    ESIA_CHECK(q.IsCoverage() && !q.IsShape() && q.Alpha() == 128 && q.Bounds() == Rect(1.5f, 2, 3, 4.25f));
    QuadInstance glyph;
    glyph.Set(Rect(0, 0, 1, 1), Vec2(0, 0), Vec2(0, 0), 0xFFFFFFFFu);   // the largest alpha: still a glyph
    ESIA_CHECK(!glyph.IsCoverage());

    DrawList dl;
    dl.Reset(Rect(0, 0, 400, 300));
    dl.AddQuad(42, Rect(12, 12, 18, 22), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    ESIA_CHECK(dl.Commands().size() == 1 && !dl.Commands()[0].shapes);
    dl.AddCoverageQuad(42, Rect(20, 10, 44, 34), Vec2(0, 0), Vec2(0.5f, 0.5f), 0xFF00FF00u);
    dl.AddQuad(42, Rect(50, 12, 56, 22), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    dl.AddCoverageQuad(42, Rect(60, 10, 60, 34), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);   // empty: nothing
    dl.AddCoverageQuad(42, Rect(60, 10, 70, 34), Vec2(0, 0), Vec2(1, 1), 0x00FFFFFFu);   // transparent: nothing
    ESIA_CHECK(dl.Commands().size() == 1 && dl.Quads().size() == 3);
    const DrawCmd& c = dl.Commands()[0];
    ESIA_CHECK(c.kind == DrawCmdKind::Quads && c.texture == 42 && c.count == 3 && c.shapes && c.vtxBounds == Rect(12, 10, 56, 34));
    ESIA_CHECK(!dl.Quads()[0].IsCoverage() && dl.Quads()[1].IsCoverage() && dl.Quads()[1].Alpha() == 255 && !dl.Quads()[2].IsCoverage());
    // on another page: a command of its own; after shapes alone: it gives them its page
    dl.AddCoverageQuad(43, Rect(20, 40, 44, 64), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    ESIA_CHECK(dl.Commands().size() == 2 && dl.Commands()[1].texture == 43 && dl.Commands()[1].shapes);
    dl.PushClipRect(Rect(0, 0, 100, 100));
    QuadInstance fill;
    fill.SetShape(QuadShape::Fill, Rect(10, 10, 50, 30), 4.0f, 0.6f, 0.0f, 0.0f, 0xFF0000FFu);
    dl.AddShapeQuad(fill);
    dl.AddCoverageQuad(44, Rect(20, 40, 44, 64), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    ESIA_CHECK(dl.Commands().size() == 3 && dl.Commands()[2].texture == 44 && !dl.Commands()[2].anyTexture && dl.Commands()[2].count == 2);
    dl.PopClipRect();
}

// ---------------------------------------------------------------- recording (DrawRecord)
namespace
{
    // the same lists: every array byte for byte, every command field for field
    bool SameList(const DrawList& a, const DrawList& b)
    {
        const auto bytes = [](const auto& x, const auto& y) {
            return x.size() == y.size() && (x.empty() || std::memcmp(x.data(), y.data(), x.size() * sizeof(x[0])) == 0);
        };
        if (!bytes(a.Vertices(), b.Vertices()) || !bytes(a.Indices(), b.Indices()) || !bytes(a.Quads(), b.Quads()) ||
            !bytes(a.FxInstances(), b.FxInstances()) || a.Commands().size() != b.Commands().size() || a.Layers().size() != b.Layers().size())
            return false;
        for (std::size_t i = 0; i < a.Commands().size(); ++i)
        {
            const DrawCmd &x = a.Commands()[i], &y = b.Commands()[i];
            if (x.kind != y.kind || !(x.clip == y.clip) || x.texture != y.texture || x.anyTexture != y.anyTexture || x.shapes != y.shapes ||
                x.first != y.first || x.count != y.count || x.effect != y.effect || x.payload != y.payload || x.callback != y.callback ||
                x.userData != y.userData || x.vtxFirst != y.vtxFirst || x.vtxEnd != y.vtxEnd || !(x.vtxBounds == y.vtxBounds))
                return false;
        }
        return true;
    }

    // what other widgets drew before: a label's glyph on page 7 at `x`, `rects` filled rectangles before it
    void Before(DrawList& dl, float x, int rects)
    {
        dl.Reset(Rect(0, 0, 400, 300));
        for (int i = 0; i < rects; ++i)
            dl.AddRectFilled(Rect(x + (float)i, 50, x + (float)i + 0.5f, 60), 0xFF102030u);
        DrawList::QuadWriter w = dl.BeginQuads(7, 1);
        w.Add(Rect(x, 40, x + 5, 48), Vec2(0, 0), Vec2(0.25f, 0.25f), 0xFFFFFFFFu);
        dl.EndQuads(w);
    }

    // a widget's drawing at x: its label's glyphs on page 7 (they join the glyph before them), a shape, a rectangle in
    // a clip of its own, an FX shape
    void Widget(DrawList& dl, float x)
    {
        DrawList::QuadWriter w = dl.BeginQuads(7, 3);
        w.Add(Rect(x, 0, x + 5, 8), Vec2(0, 0), Vec2(0.5f, 0.5f), 0xFF00FF00u);
        w.Add(Rect(x + 6, 0, x + 11, 8), Vec2(0.5f, 0), Vec2(1, 0.5f), 0xFF00FF00u);
        dl.EndQuads(w);
        QuadInstance s;
        s.SetShape(QuadShape::Fill, Rect(x, 10, x + 20, 20), 4.0f, 0.6f, 0.0f, 0.0f, 0xFF336699u);
        dl.AddShapeQuad(s);
        dl.PushClipRect(Rect(x, 0, x + 30, 30));
        dl.AddRectFilled(Rect(x, 22, x + 9, 28), 0x80FFFFFFu);
        dl.PopClipRect();
        fx::Instance inst;
        std::memset(&inst, 0, sizeof(inst));
        inst.rect[0] = x;
        inst.rect[1] = 30;
        inst.rect[2] = x + 40;
        inst.rect[3] = 50;
        dl.AddFx(inst, 3, 0);
        dl.AddFx(inst, 3, 0);
    }
}

// A widget's drawing recorded last frame and appended this frame after other content - other glyphs before it on its
// label's page (it joins their command), more geometry (its indices move) - is what drawing it again makes; and a
// recording leaves the list as drawing without one does.
ESIA_TEST(DrawList, ReplayAppendsWhatDrawingWould)
{
    DrawList recorded, plain;
    DrawRecord rec;
    Before(recorded, 3.0f, 1);
    recorded.BeginRecord();
    ESIA_CHECK(recorded.Recording());
    Widget(recorded, 50.0f);
    ESIA_CHECK(recorded.EndRecord(rec) && !recorded.Recording());
    ESIA_CHECK(rec.took && rec.addCount == 3 && rec.cmds.size() == 2);   // the glyphs and the shape joined; geometry, FX
    Before(plain, 3.0f, 1);
    Widget(plain, 50.0f);
    ESIA_CHECK(SameList(recorded, plain));

    DrawList drawn, replayed;
    Before(drawn, 7.25f, 3);
    Widget(drawn, 50.0f);
    Before(replayed, 7.25f, 3);
    ESIA_CHECK(replayed.CanReplay(rec));
    replayed.Replay(rec);
    ESIA_CHECK(SameList(drawn, replayed));
    // twice in a row: the second after the first, as drawing it twice
    ESIA_CHECK(!replayed.CanReplay(rec));   // (its last command is now an FX shape, not the glyphs it began after)
    // a record kept from the second drawing replays after the first
    DrawRecord second;
    drawn.BeginRecord();
    Widget(drawn, 50.0f);
    ESIA_CHECK(drawn.EndRecord(second));
    ESIA_CHECK(replayed.CanReplay(second));
    replayed.Replay(second);
    ESIA_CHECK(SameList(drawn, replayed));
    // the same stretch recorded again: the same record (the widget layer's check)
    DrawRecord again;
    DrawList other;
    Before(other, 9.0f, 2);
    other.BeginRecord();
    Widget(other, 50.0f);
    ESIA_CHECK(other.EndRecord(again) && again == rec);
    DrawRecord moved;
    Before(other, 9.0f, 2);
    other.BeginRecord();
    Widget(other, 51.0f);
    ESIA_CHECK(other.EndRecord(moved) && !(moved == rec));
}

// A record is appended only where the list is in the state its stretch began in.
ESIA_TEST(DrawList, ReplayNeedsTheStateItBeganIn)
{
    DrawList dl;
    DrawRecord rec;
    Before(dl, 3.0f, 0);
    dl.BeginRecord();
    Widget(dl, 50.0f);
    ESIA_CHECK(dl.EndRecord(rec));

    DrawList other;
    Before(other, 3.0f, 0);
    ESIA_CHECK(other.CanReplay(rec));
    other.AddQuad(8, Rect(0, 0, 4, 4), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);   // its last command on another page
    ESIA_CHECK(!other.CanReplay(rec));
    Before(other, 3.0f, 0);
    other.Mark();   // nothing joins the glyphs before
    ESIA_CHECK(!other.CanReplay(rec));
    Before(other, 3.0f, 0);
    other.PushClipRect(Rect(0, 0, 200, 200));
    ESIA_CHECK(!other.CanReplay(rec));
    other.PopClipRect();
    other.PushTexture(5);
    ESIA_CHECK(!other.CanReplay(rec));
    other.PopTexture();
    ESIA_CHECK(other.CanReplay(rec));
    other.Reset(Rect(0, 0, 400, 300));   // nothing before it
    ESIA_CHECK(!other.CanReplay(rec));
}

// A recording that reaches into what lay before it is not kept, and the list is what it is unrecorded.
ESIA_TEST(DrawList, RecordingsThatReachBackAreNotKept)
{
    const auto check = [](const auto& draw) {
        DrawList recorded, plain;
        DrawRecord rec;
        Before(recorded, 3.0f, 2);
        recorded.PushClipRect(Rect(0, 0, 300, 300));
        recorded.BeginRecord();
        draw(recorded);
        ESIA_CHECK(!recorded.EndRecord(rec));
        ESIA_CHECK(rec.cmds.empty() && rec.quads.empty());
        Before(plain, 3.0f, 2);
        plain.PushClipRect(Rect(0, 0, 300, 300));
        draw(plain);
        ESIA_CHECK(SameList(recorded, plain));
    };
    check([](DrawList& dl) { Widget(dl, 50.0f); dl.Mark(); Widget(dl, 60.0f); });
    check([](DrawList& dl) { Widget(dl, 50.0f); dl.PopClipRect(); dl.PushClipRect(Rect(0, 0, 300, 300)); });   // a clip from before
    check([](DrawList& dl) { dl.BeginLayer(fx::LayerParams{}); Widget(dl, 50.0f); dl.EndLayer(); });
    check([](DrawList& dl) { const std::size_t from = dl.Commands().size(); Widget(dl, 50.0f); dl.MoveCommands(from, 0); });
    check([](DrawList& dl) {
        // a scale that rewrites the glyphs before it too: their bounds refreshed
        Widget(dl, 50.0f);
        for (QuadInstance& q : dl.Quads())
            q.rect[2] += 1.0f;
        dl.RefreshBounds(0, 0);
    });
    check([](DrawList& dl) { Widget(dl, 50.0f); dl.PushClipRect(Rect(0, 0, 10, 10)); });   // left pushed
    check([](DrawList& dl) { Widget(dl, 50.0f); dl.BreakRecording(); });                    // drawn from live state
}
