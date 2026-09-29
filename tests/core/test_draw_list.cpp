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
