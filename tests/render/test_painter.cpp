// Painter: the fx::Instance encoding (it must stay WGT's: the shaders read it), culling, masks, scale, glow
// containment, geometry primitives, layers and fades.
#include "esia/core/cpu.hpp"
#include "esia/render/painter.hpp"
#include "esia/text/glyph_atlas.hpp"
#include "esia_test.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

using namespace esia;

namespace
{
    // Tests of the FX instance encoding and of what is drawn from it: their plain shapes stay FX instances
    // (PainterEnv::quadShapes; the quads have tests of their own)
    PainterEnv Fx(PainterEnv env = {})
    {
        env.quadShapes = false;
        return env;
    }

    // Text stand-in: every character is a 6 x 10 quad on texture 42 (an instanced quad, as glyphs are).
    struct FakeText final : text::TextSystem
    {
        float lastScale = 0.0f;
        Vec2 lastPos;
        // the outlines asked for, and what was drawn after them (DrawOutline / DrawGlyphOutline, then Draw / DrawGlyph)
        std::string calls;
        text::TextOutline lastOutline;
        float lastOutlineScale = 0.0f;
        void DrawOutline(DrawList&, text::FontRef, Vec2, const text::TextOutline& o, std::string_view, float, std::uint32_t, float scale) override
        {
            calls += 'o';
            lastOutline = o;
            lastOutlineScale = scale;
        }
        void DrawGlyphOutline(DrawList&, text::FontRef, char32_t, Vec2, const text::TextOutline& o) override
        {
            calls += 'O';
            lastOutline = o;
        }
        // the material and weight of the last DrawShapes
        fx::Instance lastMaterial{};
        float lastWeight = 0.0f;
        EffectId lastEffect = 0;
        Vec2 DrawShapes(DrawList&, text::FontRef f, Vec2 pos, const fx::Instance& m, EffectId effect, float weight, std::string_view s, float, std::uint32_t,
                        float scale) override
        {
            calls += 's';
            lastMaterial = m;
            lastWeight = weight;
            lastEffect = effect;
            lastScale = scale;
            lastPos = pos;
            return Measure(f, s, 0, 0).size;
        }
        text::FontId AddFontFile(const char*, int) override { return 1; }
        text::FontId AddFontMemory(const void*, std::size_t, int) override { return 1; }
        void AddFallback(text::FontId) override {}
        void NewFrame(const text::RasterParams&) override {}
        text::TextMetrics Measure(text::FontRef f, std::string_view s, float, std::uint32_t) override
        {
            return {Vec2(6.0f * (float)s.size() * f.size / 10.0f, f.size), f.size * 0.8f, 1};
        }
        Vec2 Draw(DrawList& dl, text::FontRef f, Vec2 pos, Color c, std::string_view s, float, std::uint32_t, float scale) override
        {
            calls += 't';
            lastScale = scale;
            lastPos = pos;
            for (std::size_t i = 0; i < s.size(); ++i)
                dl.AddQuad(42, Rect::FromSize(pos + Vec2(6.0f * scale * (float)i, 0), Vec2(6, 10) * scale), Vec2(0, 0), Vec2(1, 1), c.ToRgba8());
            return Measure(f, s, 0, 0).size;
        }
        void DrawGlyph(DrawList& dl, text::FontRef f, char32_t, Vec2 center, Color c) override
        {
            calls += 'g';
            lastScale = f.size;
            dl.AddRectFilled(Rect::FromCenter(center, Vec2(f.size, f.size)), c.ToRgba8());
        }
    };

    struct FakeGlow final : GlowContainment
    {
        Rect reported;
        float reach = 0.0f;
        void ReportGlowReach(const DrawList&, const Rect& restShape, float r) override
        {
            reported = restShape;
            reach = r;
        }
        bool GlowBounds(const DrawList&, const Rect& shape, float, Rect& bounds, float& fade) override
        {
            bounds = shape.Expanded(5.0f);
            fade = 3.0f;
            return true;
        }
    };

    DrawList MakeList()
    {
        DrawList dl;
        dl.Reset(Rect(0, 0, 400, 300));
        return dl;
    }
}

ESIA_TEST(Painter, RoundRectEncoding)
{
    DrawList dl = MakeList();
    Painter p(dl, Fx());
    p.Rect(Rect(10, 20, 110, 70), Style().Fill(Paint::Linear(Color::Hex(0xFF0000), Color::Hex(0x0000FF), 45)).Radius(4, 5, 6, 7)
                                        .Stroke(2, Color::White(0.5f), 0.5f).Shadow(Color::Black(0.3f), 12, Vec2(0, 4)).Opacity(0.8f));
    ESIA_CHECK(dl.FxInstances().size() == 1 && dl.Commands().size() == 1 && dl.Commands()[0].kind == DrawCmdKind::Fx);
    const fx::Instance& i = dl.FxInstances()[0];
    ESIA_CHECK(i.rect[0] == 10 && i.rect[1] == 20 && i.rect[2] == 110 && i.rect[3] == 70);
    ESIA_CHECK(i.radii[0] == 4 && i.radii[3] == 7);
    ESIA_CHECK(i.flags[0] == (fx::kFill | fx::kStroke | fx::kShadow));
    ESIA_CHECK(i.flags[1] == (std::uint32_t)fx::PaintKind::Linear && i.flags[2] == (std::uint32_t)fx::ShapeKind::RoundRect);
    ESIA_CHECK_NEAR(i.fillParams[0], Radians(45), 1e-6f);
    ESIA_CHECK(i.strokeParams[0] == 2 && i.strokeParams[1] == 0.5f);
    ESIA_CHECK(i.shadowParams[0] == 12 && i.shadowParams[3] == 4);
    ESIA_CHECK_NEAR(i.misc[0], 0.8f, 1e-6f);
    ESIA_CHECK(i.shape[1] == 0.6f);   // PainterEnv::cornerSmoothing
}

ESIA_TEST(Painter, ShapeKinds)
{
    DrawList dl = MakeList();
    Painter p(dl, Fx());
    const Style s = Style().Fill(Color::White());
    p.Arc(Vec2(100, 100), 20, 4, 0.5f, -1.0f, s);   // a negative sweep is flipped
    p.Line(Vec2(10, 10), Vec2(50, 30), 6, s);
    p.Circle(Vec2(200, 200), 10, s);
    p.Merge(Rect(10, 10, 50, 50), Rect(40, 40, 90, 90), 12, 8, s);
    const auto& in = dl.FxInstances();
    ESIA_CHECK(in.size() == 4);
    ESIA_CHECK(in[0].flags[2] == (std::uint32_t)fx::ShapeKind::Arc && in[0].radii[0] == 20 && in[0].radii[1] == 2);
    ESIA_CHECK_NEAR(in[0].shape[2], -0.5f, 1e-6f);
    ESIA_CHECK_NEAR(in[0].shape[3], 1.0f, 1e-6f);
    ESIA_CHECK(in[1].flags[2] == (std::uint32_t)fx::ShapeKind::Segment && in[1].shape2[2] == 50 && in[1].radii[0] == 3);
    ESIA_CHECK(in[2].radii[0] == 10 && in[2].shape[1] == 0.0f);   // circles never get continuous corners
    ESIA_CHECK((in[3].flags[0] & fx::kMerge) && in[3].shape2[0] == 40 && in[3].shape2Params[0] == 12 && in[3].shape2Params[1] == 8);
}

ESIA_TEST(Painter, GlassUsesTheMetricsScaleAndGetsARim)
{
    DrawList dl = MakeList();
    PainterEnv env;
    env.metricsScale = 1.5f;
    Painter p(dl, Fx(env));
    GlassMaterial g;
    g.blur = 10;
    g.refraction = 8;
    g.bezel = 20;
    g.legibility = 0.4f;
    g.magnify = 0.25f;
    p.Capsule(Rect(10, 10, 110, 50), Style().Glass(g));
    const fx::Instance& i = dl.FxInstances()[0];
    ESIA_CHECK((i.flags[0] & fx::kGlass) && (i.flags[0] & fx::kStroke) && (i.flags[0] & fx::kNoise));
    ESIA_CHECK(i.glass[0] == 15 && i.glass[1] == 12 && i.glass[2] == 30);
    ESIA_CHECK(i.strokeParams[0] == 1.5f);            // the hairline rim follows the scale
    ESIA_CHECK(i.radii[0] == 20 && i.radii[2] == 20);  // capsule: half the height
    ESIA_CHECK(i.shape[0] == 0.4f && i.shape2Params[2] == 0.25f);
}

ESIA_TEST(Painter, InvisibleAndClippedShapesAreDropped)
{
    DrawList dl = MakeList();
    Painter p(dl, Fx());
    p.Rect(Rect(10, 10, 20, 20), Style());                                 // nothing to draw
    p.Rect(Rect(10, 10, 20, 20), Style().Fill(Color::White()).Opacity(0));   // transparent
    p.Rect(Rect(500, 10, 520, 20), Style().Fill(Color::White()));             // outside the clip
    p.Rect(Rect(-40, 10, -20, 20), Style().Fill(Color::White()).Glow(Color::White(), 20));   // its glow reaches in
    ESIA_CHECK(dl.FxInstances().size() == 1);
}

ESIA_TEST(Painter, MasksAndScale)
{
    DrawList dl = MakeList();
    Painter p(dl, Fx());
    p.PushMask(Rect(0, 0, 100, 100), 12);
    p.PushScale(Vec2(50, 50), 0.5f);
    p.Rect(Rect(0, 0, 100, 100), Style().Fill(Color::White()).Radius(20).Glow(Color::White(), 10));
    dl.AddRectFilled(Rect(0, 0, 100, 100), 0xFFFFFFFFu);
    dl.AddQuad(7, Rect(0, 0, 100, 100), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    p.PopScale();
    p.PopMask();
    p.Rect(Rect(0, 0, 10, 10), Style().Fill(Color::White()));
    const fx::Instance& i = dl.FxInstances()[0];
    ESIA_CHECK((i.flags[0] & fx::kMask) && i.mask[2] == 100 && i.maskParams[0] == 12);
    ESIA_CHECK(i.rect[0] == 25 && i.rect[2] == 75 && i.radii[0] == 10 && i.glowParams[0] == 5);
    ESIA_CHECK(!(dl.FxInstances()[1].flags[0] & fx::kMask));
    // geometry and quads drawn under the scale are scaled at PopScale
    ESIA_CHECK(dl.Vertices()[0].pos == Vec2(25, 25) && dl.Vertices()[2].pos == Vec2(75, 75));
    const QuadInstance& q = dl.Quads()[0];
    ESIA_CHECK(q.rect[0] == 25 && q.rect[1] == 25 && q.rect[2] == 75 && q.rect[3] == 75);
    ESIA_CHECK(dl.Commands()[2].kind == DrawCmdKind::Quads && dl.Commands()[2].vtxBounds == Rect(25, 25, 75, 75));
}

ESIA_TEST(Painter, TextUnderScaleIsRasterizedScaledNotStretched)
{
    DrawList dl = MakeList();
    FakeText text;
    PainterEnv env;
    env.text = &text;
    Painter p(dl, Fx(env));
    p.PushScale(Vec2(0, 0), 2.0f);
    const Vec2 size = p.Text(Vec2(10, 10), text::FontRef{1, 10}, Color::White(), "ab");
    p.PopScale();
    ESIA_CHECK(text.lastScale == 2.0f && text.lastPos == Vec2(20, 20));
    ESIA_CHECK(size == Vec2(12, 10));   // the layout size, unscaled
    // the glyph quads keep their final geometry: PopScale skips them
    const QuadInstance& q = dl.Quads()[0];
    ESIA_CHECK(q.rect[0] == 20 && q.rect[1] == 20 && q.rect[2] == 32 && q.rect[3] == 40);
    ESIA_CHECK(dl.Commands().back().texture == 42);

    const Vec2 box = p.TextBox(Rect(0, 100, 100, 140), Vec2(0.5f, 0.5f), text::FontRef{1, 10}, Color::White(), "abcd");
    ESIA_CHECK(box == Vec2(24, 10) && text.lastPos == Vec2(38, 115));
    p.Icon(Vec2(50, 50), text::FontRef{2, 16}, U'\xE700', Color::White());
    ESIA_CHECK(text.lastScale == 16.0f);
}

ESIA_TEST(Painter, UnscaledImagesStayWhereTheScalePutThem)
{
    DrawList dl = MakeList();
    Painter p(dl, Fx());
    ESIA_CHECK(p.Scaled(Vec2(30, 30)) == Vec2(30, 30) && p.ScaleFactor() == 1.0f);
    p.PushScale(Vec2(10, 10), 2.0f);
    p.PushScale(Vec2(20, 20), 0.5f);
    // the inner scale first, then the outer: 30 -> 25 -> 40
    ESIA_CHECK(p.Scaled(Vec2(30, 30)) == Vec2(40, 40) && p.ScaleFactor() == 1.0f);
    p.PopScale();
    ESIA_CHECK(p.Scaled(Vec2(30, 30)) == Vec2(50, 50) && p.ScaleFactor() == 2.0f);
    // a hundred images at their final place, a rectangle of geometry after every 25: PopScale scales the rectangles
    // and leaves the images (more of them than the ranges it can skip one by one: images in a row are one range)
    for (int i = 0; i < 100; ++i)
    {
        p.UnscaledImage(7, Rect((float)i * 4.0f, 0.0f, (float)i * 4.0f + 2.0f, 2.0f));
        if (i % 25 == 24)
            dl.AddRectFilled(Rect(1, 1, 2, 2), 0xFFFFFFFFu);
    }
    p.PopScale();
    const auto& vtx = dl.Vertices();
    ESIA_CHECK(vtx.size() == 4 * 100 + 4 * 4);
    bool placed = vtx.size() == 4 * 100 + 4 * 4;
    std::size_t v = 0;
    for (int i = 0; i < 100 && placed; ++i)
    {
        float x0 = 1e9f, y0 = 1e9f, x1 = -1e9f, y1 = -1e9f;
        for (int k = 0; k < 4; ++k, ++v)
            x0 = std::min(x0, vtx[v].pos.x), y0 = std::min(y0, vtx[v].pos.y), x1 = std::max(x1, vtx[v].pos.x), y1 = std::max(y1, vtx[v].pos.y);
        placed = x0 == (float)i * 4.0f && x1 == (float)i * 4.0f + 2.0f && y0 == 0.0f && y1 == 2.0f;
        if (i % 25 == 24)
        {
            // (1, 1) - (2, 2) twice as far from (10, 10): (-8, -8) - (-6, -6)
            placed = placed && vtx[v].pos == Vec2(-8, -8) && vtx[v + 2].pos == Vec2(-6, -6);
            v += 4;
        }
    }
    ESIA_CHECK(placed);
    // under a mask: an FX instance at `r`, not scaled
    p.PushMask(Rect(0, 0, 100, 100), 4);
    p.PushScale(Vec2(0, 0), 2.0f);
    p.UnscaledImage(7, Rect(10, 10, 20, 20));
    p.PopScale();
    p.PopMask();
    const fx::Instance& i = dl.FxInstances().back();
    ESIA_CHECK((i.flags[0] & fx::kMask) && (i.flags[0] & fx::kImage) && i.rect[0] == 10 && i.rect[1] == 10 && i.rect[2] == 20 && i.rect[3] == 20);
}

// A coverage tile drawn as an image (svg::Draw's icons in the text's pages): a plain-coverage quad on the tile's page at
// `r` with the painter's opacity, left where it is by PopScale; none under a mask (the image is an FX shape there).
ESIA_TEST(Painter, UnscaledCoverageIsAQuadWithTheText)
{
    DrawList dl = MakeList();
    PainterEnv env;
    env.alpha = 0.5f;
    Painter p(dl, Fx(env));
    text::GlyphSlot tile;
    tile.page = 42;
    tile.uv0 = Vec2(0.25f, 0.5f);
    tile.uv1 = Vec2(0.375f, 0.625f);
    dl.AddQuad(42, Rect(0, 0, 6, 10), Vec2(0, 0), Vec2(0.01f, 0.01f), 0xFFFFFFFFu);   // a glyph before it
    p.PushScale(Vec2(0, 0), 2.0f);
    ESIA_CHECK(p.UnscaledCoverage(tile, Rect(10, 20, 34, 44), Color(1.0f, 0.0f, 0.0f, 0.8f)));
    p.PopScale();
    ESIA_CHECK(dl.Commands().size() == 1 && dl.Commands()[0].shapes && dl.Quads().size() == 2);
    const QuadInstance& q = dl.Quads()[1];
    QuadInstance want;
    want.SetCoverage(Rect(10, 20, 34, 44), tile.uv0, tile.uv1, Color(1.0f, 0.0f, 0.0f, 0.4f).ToRgba8());
    ESIA_CHECK(std::memcmp(&q, &want, sizeof(q)) == 0);
    // under a mask: refused, nothing drawn
    ESIA_CHECK(!p.Masked());
    p.PushMask(Rect(0, 0, 200, 200), 16.0f);
    ESIA_CHECK(p.Masked() && !p.UnscaledCoverage(tile, Rect(40, 40, 64, 64), Color::White()) && dl.Quads().size() == 2);
    p.PopMask();
    ESIA_CHECK(!p.Masked() && p.UnscaledCoverage(tile, Rect(40, 40, 64, 64), Color::White()) && dl.Quads().size() == 3);
    // nothing for a transparent color or an empty tile
    ESIA_CHECK(p.UnscaledCoverage(tile, Rect(40, 40, 64, 64), Color::White(0.0f)) && dl.Quads().size() == 3);
    tile.page = 0;
    ESIA_CHECK(p.UnscaledCoverage(tile, Rect(40, 40, 64, 64), Color::White()) && dl.Quads().size() == 3);
}

ESIA_TEST(Painter, TextOutlinesGoUnderTheirTextAndIcons)
{
    DrawList dl = MakeList();
    FakeText text;
    PainterEnv env;
    env.text = &text;
    env.alpha = 0.5f;
    Painter p(dl, Fx(env));
    // none by default
    p.Text(Vec2(10, 10), text::FontRef{1, 10}, Color::White(), "ab");
    p.Icon(Vec2(50, 50), text::FontRef{2, 16}, U'\xE700', Color::White());
    ESIA_CHECK(text.calls == "tg");
    // set: before the text, the icon and a TextBox's text, with the Painter's opacity
    text.calls.clear();
    p.SetTextOutline({2.0f, Color(1.0f, 0.0f, 0.0f, 0.8f)});
    p.Text(Vec2(10, 10), text::FontRef{1, 10}, Color::White(), "ab");
    ESIA_CHECK(text.lastOutline.width == 2.0f && text.lastOutline.color.a == 0.4f);
    p.Icon(Vec2(50, 50), text::FontRef{2, 16}, U'\xE700', Color::White());
    p.TextBox(Rect(0, 100, 100, 140), Vec2(0.5f, 0.5f), text::FontRef{1, 10}, Color::White(), "abcd");
    ESIA_CHECK(text.calls == "otOgot");
    // under a scale: the text's scale, an icon's outline grown with it
    text.calls.clear();
    p.PushScale(Vec2(0, 0), 2.0f);
    p.Text(Vec2(10, 10), text::FontRef{1, 10}, Color::White(), "ab");
    ESIA_CHECK(text.lastOutlineScale == 2.0f);
    p.Icon(Vec2(50, 50), text::FontRef{2, 16}, U'\xE700', Color::White());
    ESIA_CHECK(text.lastOutline.width == 4.0f);
    p.PopScale();
    ESIA_CHECK(text.calls == "otOg");
    // invisible text: no outline either
    text.calls.clear();
    p.Text(Vec2(10, 10), text::FontRef{1, 10}, Color::Clear(), "ab");
    ESIA_CHECK(text.calls.empty());
    ESIA_CHECK(p.GetTextOutline().width == 2.0f);
}

ESIA_TEST(Painter, GlowContainmentReportsRestGeometryAndHalo)
{
    DrawList dl = MakeList();
    FakeGlow glow;
    PainterEnv env;
    env.glow = &glow;
    Painter p(dl, Fx(env));
    p.PushScale(Vec2(0, 0), 1.1f);   // a press animation must not change what the layout reserves
    p.Rect(Rect(100, 100, 200, 140), Style().Fill(Color::White()).Glow(Color::White(), 20));
    p.PopScale();
    ESIA_CHECK(glow.reported == Rect(100, 100, 200, 140) && glow.reach == 15.0f);
    const fx::Instance& i = dl.FxInstances()[0];
    ESIA_CHECK(i.flags[0] & fx::kHalo);
    ESIA_CHECK_NEAR(i.halo[0], 105.0f, 1e-4f);
    ESIA_CHECK(i.maskParams[2] == 3.0f);
    p.Rect(Rect(10, 10, 20, 20), Style().Fill(Color::White()).Glow(Color::White(), 20).ContainGlow(false));
    ESIA_CHECK(!(dl.FxInstances()[1].flags[0] & fx::kHalo));
}

ESIA_TEST(Painter, CheapPrimitives)
{
    DrawList dl = MakeList();
    PainterEnv env;
    env.pixelScale = 2.0f;
    Painter p(dl, Fx(env));
    p.FillRect(Rect(0, 0, 10, 10), Color::White());
    p.FillRect(Rect(0, 0, 10, 10), Color::White(), 4);   // rounded: an SDF instance
    p.HLine(0, 100, 10.3f, Color::White(), 0.5f);
    ESIA_CHECK(dl.FxInstances().size() == 1);
    ESIA_CHECK(dl.Vertices().size() == 8);
    ESIA_CHECK(dl.Vertices()[4].pos.y == 10.0f && dl.Vertices()[6].pos.y == 10.5f);   // exactly one physical pixel row
    ESIA_CHECK(p.SnapToPixel(10.3f) == 10.5f);
}

// Masks and scales reach SDF shapes only: a square FillRect under one becomes one (a checkerboard in a capsule, a
// number field's fill in its rounded well kept their square corners), and stays indexed geometry otherwise.
ESIA_TEST(Painter, FillRectFollowsMasksAndScales)
{
    DrawList dl = MakeList();
    PainterEnv env;
    Painter p(dl, Fx(env));
    p.PushMask(Rect(0, 0, 40, 20), 10.0f);
    p.FillRect(Rect(0, 0, 10, 20), Color::White());
    p.PopMask();
    p.PushScale(Vec2(20, 10), 0.5f);
    p.FillRect(Rect(0, 0, 10, 20), Color::White());
    p.PopScale();
    p.FillRect(Rect(0, 0, 10, 20), Color::White());
    ESIA_CHECK(dl.FxInstances().size() == 2);
    ESIA_CHECK(dl.Vertices().size() == 4);
    if (dl.FxInstances().size() == 2)
        ESIA_CHECK(dl.FxInstances()[0].maskParams[0] > 0.0f && dl.FxInstances()[1].maskParams[0] == 0.0f);
}

ESIA_TEST(Painter, SquareImagesOnThePixelGridAreQuads)
{
    DrawList dl = MakeList();
    PainterEnv env;
    env.pixelScale = 2.0f;
    Painter p(dl, Fx(env));
    p.SetAlpha(0.5f);
    p.Image(7, Rect(0, 0, 100, 50.5f), 0, Color::White(), Vec2(0.25f, 0), Vec2(0.75f, 1));   // whole pixels at 2x: a quad
    ESIA_CHECK(dl.FxInstances().empty() && dl.Vertices().size() == 4);
    ESIA_CHECK(dl.Commands().back().texture == 7 && dl.Vertices()[0].uv == Vec2(0.25f, 0) && dl.Vertices()[2].uv == Vec2(0.75f, 1));
    ESIA_CHECK((dl.Vertices()[0].color >> 24) == 128);   // the painter's alpha
    // rounded, between pixels, masked or scaled: the SDF shape (anti-aliased edges)
    p.Image(7, Rect(0, 0, 100, 50), 4);
    p.Image(7, Rect(0, 0, 100, 50.25f), 0);
    p.PushMask(Rect(0, 0, 60, 60), 8);
    p.Image(7, Rect(0, 0, 100, 50), 0);
    p.PopMask();
    p.PushScale(Vec2(50, 25), 0.9f);
    p.Image(7, Rect(0, 0, 100, 50), 0);
    p.PopScale();
    ESIA_CHECK(dl.FxInstances().size() == 4 && dl.Vertices().size() == 4);
    ESIA_CHECK(dl.FxInstances()[0].flags[0] & fx::kImage);
}

ESIA_TEST(Painter, PolylineAndArea)
{
    DrawList dl = MakeList();
    Painter p(dl, Fx());
    const Vec2 pts[4] = {Vec2(0, 50), Vec2(40, 10), Vec2(80, 30), Vec2(120, 5)};
    p.Polyline(pts, 4, 3.0f, Style().Fill(Color::Hex(0x30D158)));
    const std::size_t n = dl.Vertices().size();
    ESIA_CHECK(n > 16 && dl.Indices().size() % 3 == 0);
    for (std::uint32_t idx : dl.Indices())
        ESIA_CHECK(idx < n);
    p.Polyline(pts, 4, 3.0f, Style().Fill(Color::Hex(0x30D158)).Glow(Color::Hex(0x30D158), 8), PolylineFlags_Smooth);
    int layers = 0;
    for (const DrawCmd& c : dl.Commands())
        layers += c.kind == DrawCmdKind::LayerBegin || c.kind == DrawCmdKind::LayerEnd;
    ESIA_CHECK(layers == 2);   // one glow for the whole line
    ESIA_CHECK(dl.Layers()[0].color[3] == 0.0f);   // a glow in the line's own color keeps its colors
    const std::size_t before = dl.Vertices().size();
    p.Area(pts, 4, 60.0f, Paint::Linear(Color::White(), Color::White(0), 90));
    ESIA_CHECK(dl.Vertices().size() == before + 8);
    ESIA_CHECK(dl.Vertices()[before].color != dl.Vertices()[before + 1].color);   // top and baseline differ
}

ESIA_TEST(Painter, LayersFadesAndStreaks)
{
    DrawList dl = MakeList();
    PainterEnv env;
    env.metricsScale = 2.0f;
    env.alpha = 0.5f;
    Painter p(dl, Fx(env));
    p.BeginGlowLayer(Color::Hex(0xFF00FF, 0.4f), 6.0f, 2.0f, 0.9f);
    p.EndGlowLayer();
    p.BeginEdgeFade(Rect(0, 40, 100, 240), 12, -3);
    p.EndEdgeFade();
    p.LightStreak(Rect(0, 0, 200, 60), 1.0f, 2.0f);
    ESIA_CHECK(dl.Layers().size() == 1 && dl.Layers()[0].radius == 12.0f && dl.Layers()[0].intensity == 1.0f && dl.Layers()[0].opacity == 0.9f);
    ESIA_CHECK(dl.Fades().size() == 1 && dl.Fades()[0].y0 == 40 && dl.Fades()[0].y1 == 240 && dl.Fades()[0].top == 12 && dl.Fades()[0].bottom == 0);
    const fx::Instance& s = dl.FxInstances()[0];
    ESIA_CHECK(s.flags[0] == fx::kCaustic && s.fill0[3] == 1.0f && s.fill1[0] == -1.0f && s.misc[0] == 0.5f);
}

namespace
{
    // FakeText with the coverage tiles of flat drawing: they sit on the text's texture (42), as in the FreeType system
    struct TileText final : text::TextSystem
    {
        FakeText base;
        std::unordered_map<std::uint64_t, text::GlyphSlot> tiles;
        std::unordered_map<std::uint64_t, text::GlyphBitmap> bitmaps;
        int added = 0;
        std::uint64_t generation = NextGeneration();   // every text system its own (the tile memo keys on it)
        static std::uint64_t NextGeneration()
        {
            static std::uint64_t n = 0;
            return (++n) << 32;
        }
        text::FontId AddFontFile(const char* p, int i) override { return base.AddFontFile(p, i); }
        text::FontId AddFontMemory(const void* d, std::size_t s, int i) override { return base.AddFontMemory(d, s, i); }
        void AddFallback(text::FontId f) override { base.AddFallback(f); }
        void NewFrame(const text::RasterParams& r) override { base.NewFrame(r); }
        text::TextMetrics Measure(text::FontRef f, std::string_view s, float w, std::uint32_t fl) override { return base.Measure(f, s, w, fl); }
        Vec2 Draw(DrawList& dl, text::FontRef f, Vec2 pos, Color c, std::string_view s, float w, std::uint32_t fl, float k) override
        {
            return base.Draw(dl, f, pos, c, s, w, fl, k);
        }
        void DrawGlyph(DrawList& dl, text::FontRef f, char32_t cp, Vec2 center, Color c) override { base.DrawGlyph(dl, f, cp, center, c); }
        const text::GlyphSlot* FindTile(std::uint64_t key) override
        {
            const auto it = tiles.find(key);
            return it != tiles.end() ? &it->second : nullptr;
        }
        const text::GlyphSlot* AddTile(std::uint64_t key, const text::GlyphBitmap& b) override
        {
            ++added;
            bitmaps[key] = b;
            text::GlyphSlot s;
            s.page = 42;
            s.uv0 = Vec2(0.0f, 0.0f);
            s.uv1 = Vec2((float)b.width / 1024.0f, (float)b.height / 1024.0f);
            s.width = b.width;
            s.height = b.height;
            return &(tiles[key] = s);
        }
        std::uint64_t TileGeneration() override { return generation; }
    };

    int QuadCommands(const DrawList& dl, TextureId texture)
    {
        int n = 0;
        for (const DrawCmd& c : dl.Commands())
            n += c.kind == DrawCmdKind::Quads && c.count > 0 && c.texture == texture ? 1 : 0;
        return n;
    }

    std::uint32_t QuadColor(const QuadInstance& q) { return (std::uint32_t)q.packed[2] | ((std::uint32_t)q.packed[3] << 24); }
}

// Flat drawing: rounded rectangles, capsules and circles are quads on the text's texture - one command with the text
// around them - from coverage tiles made once per kind of shape; no shadows; glass a solid surface
ESIA_TEST(Painter, FlatShapesAreGeometryThatBatchesWithText)
{
    TileText tt;
    DrawList dl = MakeList();
    PainterEnv env;
    env.text = &tt;
    env.flat = true;
    env.flatSurface = Color(0.1f, 0.1f, 0.1f, 1.0f);
    Painter p(dl, Fx(env));
    const Style button = Style().Radius(18).Fill(Color::Hex(0x0A84FF)).Shadow(Color::Black(0.3f), 12, Vec2(0, 4));
    p.Text(Vec2(10, 10), text::FontRef{1, 10.0f}, Color::White(), "Label");
    p.Rect(Rect(80, 4, 160, 40), button);
    p.Text(Vec2(100, 15), text::FontRef{1, 10.0f}, Color::White(), "OK");
    p.Capsule(Rect(80, 60, 160, 96), Style().Fill(Color::Hex(0x0A84FF)));   // the same shape elsewhere: the same tile
    p.Circle(Vec2(200, 20), 10, Style().Fill(Color::White()));
    ESIA_CHECK(dl.FxInstances().empty());
    ESIA_CHECK(QuadCommands(dl, 42) == 1 && dl.Vertices().empty());   // the text and the shapes: one command
    ESIA_CHECK(tt.added == 2);                                       // the button's tile and the circle's
    // a capsule 80 x 36: stretched along x (3 cells), whole along y; a circle: one quad
    ESIA_CHECK(dl.Quads().size() == (std::size_t)(5 + 3 + 2 + 3 + 1));

    // a large fill: its middle from the white texture, the rim from the tile; a stroke's ring around it
    DrawList d2 = MakeList();
    Painter q(d2, Fx(env));
    q.Rect(Rect(10, 10, 310, 210), Style().Radius(20).Fill(Color(0.2f, 0.2f, 0.2f, 1)).Stroke(1.0f, Color::White(0.3f)));
    ESIA_CHECK(d2.FxInstances().empty() && QuadCommands(d2, 0) == 1 && QuadCommands(d2, 42) >= 1);
    ESIA_CHECK(tt.added == 4);

    // what geometry cannot do stays an FX shape, without the shadow and glow flat drawing leaves out
    DrawList d3 = MakeList();
    Painter r(d3, Fx(env));
    r.Rect(Rect(10, 10, 60, 60), Style().Radius(8).Fill(Paint::Linear(Color::White(), Color::Black())).Shadow(Color::Black(0.3f), 8).Glow(Color::White(), 6));
    ESIA_CHECK(d3.FxInstances().size() == 1 && !(d3.FxInstances()[0].flags[0] & (fx::kShadow | fx::kGlow)));

    // glass: a solid surface, flatSurface under its tint
    DrawList d4 = MakeList();
    Painter g(d4, Fx(env));
    GlassMaterial m;
    m.tint = Color(1, 1, 1, 0.5f);
    g.Rect(Rect(10, 10, 60, 60), Style().Radius(8).Glass(m));
    ESIA_CHECK(d4.FxInstances().empty() && !d4.Quads().empty());
    if (!d4.Quads().empty())
        ESIA_CHECK(QuadColor(d4.Quads()[0]) == Color(0.55f, 0.55f, 0.55f, 1.0f).ToRgba8());

    // the tiles are gone when the text system starts its pages over: made again
    ++tt.generation;
    tt.tiles.clear();
    const int before = tt.added;
    DrawList d5 = MakeList();
    Painter h(d5, Fx(env));
    h.Rect(Rect(80, 4, 160, 40), button);
    ESIA_CHECK(tt.added == before + 1 && d5.FxInstances().empty());
}

// A tile covers pixels as the FX shader does: saturate(0.5 - distance), the middle full, the margin empty
ESIA_TEST(Painter, FlatTilesCoverAsTheShaderDoes)
{
    TileText tt;
    DrawList dl = MakeList();
    PainterEnv env;
    env.text = &tt;
    env.flat = true;
    Painter p(dl, Fx(env));
    p.Circle(Vec2(50, 50), 10, Style().Fill(Color::White()));
    ESIA_CHECK(tt.bitmaps.size() == 1);
    if (tt.bitmaps.size() != 1)
        return;
    const text::GlyphBitmap& b = tt.bitmaps.begin()->second;
    ESIA_CHECK(b.width == 22 && b.height == 22);   // 20 pixels and one of nothing around
    auto at = [&](int x, int y) { return (int)b.pixels[(std::size_t)y * b.width + x]; };
    ESIA_CHECK(at(11, 11) == 255 && at(0, 0) == 0 && at(0, 11) == 0);
    // the circle of radius 10 around (11, 11): its left end on pixel 1's left edge (that pixel nearly full), and
    // pixel (3, 4) a little past half covered (its middle 0.08 inside the edge)
    ESIA_CHECK(at(1, 11) >= 250);
    ESIA_CHECK(at(3, 3) == 0 && at(3, 4) > 120 && at(3, 4) < 180);
}

// Text with a material: the style's rows and features go to the text system's glyph shapes - no image (the field
// takes its uv), no radii -, the weight with them, under a scale at the final geometry; flat drawing drops shadows and
// glows as for shapes.
ESIA_TEST(Painter, TextWithAStyleHandsItsMaterialToTheTextSystem)
{
    DrawList dl = MakeList();
    FakeText text;
    PainterEnv env;
    env.text = &text;
    Painter p(dl, Fx(env));
    const Style s = Style().Fill(Paint::Linear(Color::Hex(0xFF0000), Color::Hex(0x0000FF), 0)).Stroke(2, Color::Hex(0x00FF00), 1.0f).Shadow(Color::Black(0.5f), 4, Vec2(0, 2))
                        .Glow(Color::White(), 6).Image(9).Radius(5).Weight(0.75f).Opacity(0.5f);
    const Vec2 size = p.Text(Vec2(10, 20), text::FontRef{1, 10}, s, "abc");
    ESIA_CHECK(text.calls == "s" && size == Vec2(18, 10) && text.lastPos == Vec2(10, 20) && text.lastScale == 1.0f);
    const fx::Instance& m = text.lastMaterial;
    ESIA_CHECK(m.flags[0] == (fx::kFill | fx::kStroke | fx::kShadow | fx::kGlow));   // no kImage
    ESIA_CHECK(m.flags[1] == (std::uint32_t)fx::PaintKind::Linear && m.fill0[0] == 1.0f && m.fill1[2] == 1.0f);
    ESIA_CHECK(m.stroke[1] == 1.0f && m.strokeParams[0] == 2.0f && m.strokeParams[1] == 1.0f);
    ESIA_CHECK(m.shadowParams[0] == 4.0f && m.shadowParams[3] == 2.0f && m.glowParams[0] == 6.0f && m.misc[0] == 0.5f);
    ESIA_CHECK(text.lastWeight == 0.75f && m.radii[0] == 0.0f && m.uvRect[2] == 0.0f);
    ESIA_CHECK(dl.FxInstances().empty());   // the text system draws it

    p.PushScale(Vec2(0, 0), 2.0f);
    p.Text(Vec2(10, 20), text::FontRef{1, 10}, Style().Fill(Color::White()), "abc");
    p.PopScale();
    ESIA_CHECK(text.lastScale == 2.0f && text.lastPos == Vec2(20, 40));
    const Vec2 box = p.TextBox(Rect(0, 100, 100, 140), Vec2(0.5f, 0.5f), text::FontRef{1, 10}, Style().Fill(Color::White()), "abcd");
    ESIA_CHECK(box == Vec2(24, 10) && text.lastPos == Vec2(38, 115));
    // nothing to draw: measured, not drawn
    text.calls.clear();
    ESIA_CHECK(p.Text(Vec2(0, 0), text::FontRef{1, 10}, Style(), "ab") == Vec2(12, 10) && text.calls.empty());

    env.flat = true;
    Painter flat(dl, Fx(env));
    flat.Text(Vec2(0, 0), text::FontRef{1, 10}, s, "ab");
    ESIA_CHECK(text.lastMaterial.flags[0] == (fx::kFill | fx::kStroke));   // no shadow, no glow
}

// Legible text: kLegible with its strength (at most 1) and radius in radii.z / .w - for a fill only, and not drawn flat
// (nothing behind it is captured there). Under a scale the halo grows with the glyphs, as their stroke and glow do.
ESIA_TEST(Painter, LegibleTextAsksForItsHalo)
{
    DrawList dl = MakeList();
    FakeText text;
    PainterEnv env;
    env.text = &text;
    Painter p(dl, Fx(env));
    const fx::Instance& m = text.lastMaterial;
    p.Text(Vec2(0, 0), text::FontRef{1, 10}, Style().Fill(Color::White()).Legible(0.75f, 4.0f), "ab");
    ESIA_CHECK(m.flags[0] == (fx::kFill | fx::kLegible) && m.radii[2] == 0.75f && m.radii[3] == 4.0f);
    p.Text(Vec2(0, 0), text::FontRef{1, 10}, Style().Fill(Color::White()).Legible(3.0f), "ab");
    ESIA_CHECK(m.radii[2] == 1.0f && m.radii[3] == 0.0f);   // 0: the text system's radius
    p.Text(Vec2(0, 0), text::FontRef{1, 10}, Style().Stroke(1, Color::White()).Legible(), "ab");
    ESIA_CHECK((m.flags[0] & fx::kStroke) && !(m.flags[0] & fx::kLegible));
    p.PushScale(Vec2(0, 0), 2.0f);
    p.Text(Vec2(0, 0), text::FontRef{1, 10}, Style().Fill(Color::White()).Stroke(1, Color::Black()).Glow(Color::White(), 6).Legible(1.0f, 4.0f), "ab");
    p.PopScale();
    ESIA_CHECK(m.radii[2] == 1.0f && m.radii[3] == 8.0f && m.strokeParams[0] == 2.0f && m.glowParams[0] == 12.0f);
    env.flat = true;
    Painter flat(dl, Fx(env));
    flat.Text(Vec2(0, 0), text::FontRef{1, 10}, Style().Fill(Color::White()).Legible(), "ab");
    ESIA_CHECK(m.flags[0] == fx::kFill);
}

// Motion blur: the displacement in the custom row with kMotion, scaled under PushScale; not with a user effect (its
// row) nor when drawing flat.
ESIA_TEST(Painter, MotionBlurIsTheShapesDisplacement)
{
    DrawList dl = MakeList();
    Painter p(dl, Fx());
    p.Rect(Rect(10, 10, 50, 30), Style().Fill(Color::White()).MotionBlur(Vec2(12, -4)));
    const fx::Instance& a = dl.FxInstances()[0];
    ESIA_CHECK((a.flags[0] & fx::kMotion) && a.custom[0] == 12.0f && a.custom[1] == -4.0f);
    p.PushScale(Vec2(0, 0), 2.0f);
    p.Rect(Rect(10, 10, 50, 30), Style().Fill(Color::White()).MotionBlur(Vec2(12, -4)));
    p.PopScale();
    ESIA_CHECK(dl.FxInstances()[1].custom[0] == 24.0f && dl.FxInstances()[1].custom[1] == -8.0f);
    p.Rect(Rect(10, 10, 50, 30), Style().Fill(Color::White()).Effect(3, 0.5f).MotionBlur(Vec2(12, 0)));
    ESIA_CHECK(!(dl.FxInstances()[2].flags[0] & fx::kMotion) && dl.FxInstances()[2].custom[0] == 0.5f);
    // text with a style: every glyph shape moves
    FakeText text;
    PainterEnv env;
    env.text = &text;
    Painter t(dl, Fx(env));
    t.Text(Vec2(0, 0), text::FontRef{1, 10}, Style().Fill(Color::White()).MotionBlur(Vec2(0, 6)), "ab");
    ESIA_CHECK((text.lastMaterial.flags[0] & fx::kMotion) && text.lastMaterial.custom[1] == 6.0f);
    env.flat = true;
    Painter flat(dl, Fx(env));
    flat.Rect(Rect(10, 10, 50, 30), Style().Radius(4).Fill(Paint::Linear(Color::White(), Color::Black())).MotionBlur(Vec2(12, 0)));
    ESIA_CHECK(!(dl.FxInstances().back().flags[0] & fx::kMotion));
}

// A text system without distance fields draws the fill's first color, its opacity applied.
ESIA_TEST(Painter, TextWithAStyleWithoutFieldsDrawsTheFillColor)
{
    struct PlainText final : text::TextSystem
    {
        Color drawn;
        text::FontId AddFontFile(const char*, int) override { return 1; }
        text::FontId AddFontMemory(const void*, std::size_t, int) override { return 1; }
        void AddFallback(text::FontId) override {}
        void NewFrame(const text::RasterParams&) override {}
        text::TextMetrics Measure(text::FontRef f, std::string_view s, float, std::uint32_t) override { return {Vec2(6.0f * (float)s.size(), f.size), 8.0f, 1}; }
        Vec2 Draw(DrawList&, text::FontRef f, Vec2, Color c, std::string_view s, float, std::uint32_t, float) override
        {
            drawn = c;
            return Measure(f, s, 0, 0).size;
        }
        void DrawGlyph(DrawList&, text::FontRef, char32_t, Vec2, Color) override {}
    } text;
    DrawList dl = MakeList();
    PainterEnv env;
    env.text = &text;
    Painter p(dl, Fx(env));
    p.Text(Vec2(0, 0), text::FontRef{1, 10}, Style().Fill(Color(1, 0.5f, 0, 0.8f)).Opacity(0.5f), "ab");
    ESIA_CHECK(text.drawn.r == 1.0f && text.drawn.g == 0.5f && std::fabs(text.drawn.a - 0.4f) < 1e-6f);
}

namespace
{
    // The points a path's data instances carry (fx::ShapeKind::PathData): two to a row, kPathPointsPerData each
    std::vector<Vec2> PathPoints(const DrawList& dl, std::size_t path)
    {
        const fx::Instance& in = dl.FxInstances()[path];
        const int n = (int)in.radii[2];
        std::vector<Vec2> out;
        for (int first = 0; first < n; first += (int)fx::kPathPointsPerData)
        {
            float row[fx::kPathPointsPerData * 2];
            std::memcpy(row, &dl.FxInstances()[path + 1 + (std::size_t)first / fx::kPathPointsPerData], sizeof(row));
            for (int i = 0; i < std::min(n - first, (int)fx::kPathPointsPerData); ++i)
                out.push_back(Vec2(row[i * 2], row[i * 2 + 1]));
        }
        return out;
    }
}

// A polygon is a path: one instance with the feature kPath, its points in the data instances right after it - in the
// same command, so they are drawn and planned with it.
ESIA_TEST(Painter, PolygonsArePathsWithTheirPointsAfterThem)
{
    DrawList dl = MakeList();
    Painter p(dl, Fx());
    const Vec2 tri[3] = {Vec2(50, 10), Vec2(90, 80), Vec2(10, 80)};
    p.Polygon(tri, 3, Style().Fill(Color::White()).Shadow(Color::Black(0.3f), 8, Vec2(0, 2)));
    const auto& in = dl.FxInstances();
    ESIA_CHECK(in.size() == 2 && dl.Commands().size() == 1 && dl.Commands()[0].count == 2);
    ESIA_CHECK(in[0].flags[2] == (std::uint32_t)fx::ShapeKind::Path && (in[0].flags[0] & fx::kPath) && (in[0].flags[0] & fx::kShadow));
    ESIA_CHECK(in[0].rect[0] == 10 && in[0].rect[1] == 10 && in[0].rect[2] == 90 && in[0].rect[3] == 80);
    ESIA_CHECK(in[0].radii[0] == 0 && in[0].radii[2] == 3 && in[0].radii[3] == 0);   // no rounding, three points, closed
    ESIA_CHECK(in[1].flags[0] == 0 && in[1].flags[2] == (std::uint32_t)fx::ShapeKind::PathData);
    const std::vector<Vec2> pts = PathPoints(dl, 0);
    ESIA_CHECK(pts.size() == 3 && pts[0] == tri[0] && pts[1] == tri[1] && pts[2] == tri[2]);
    // the other winding: turned around, the inside on the left of every edge as the rounding expects
    const Vec2 back[3] = {tri[2], tri[1], tri[0]};
    p.Polygon(back, 3, Style().Fill(Color::White()));
    const std::vector<Vec2> turned = PathPoints(dl, 2);
    ESIA_CHECK(turned.size() == 3 && turned[0] == tri[0] && turned[2] == tri[2]);
    // more points than a data instance holds: one more instance per kPathPointsPerData, still one command
    std::vector<Vec2> ring;
    for (int i = 0; i < 40; ++i)
        ring.push_back(Vec2(200, 150) + Vec2(std::cos(kTau * (float)i / 40.0f), std::sin(kTau * (float)i / 40.0f)) * 50.0f);
    p.Polygon(ring.data(), 40, Style().Fill(Color::White()));
    ESIA_CHECK(in.size() == 7 && dl.Commands().size() == 1 && in[4].radii[2] == 40);
    ESIA_CHECK(in[5].flags[2] == (std::uint32_t)fx::ShapeKind::PathData && in[6].flags[2] == (std::uint32_t)fx::ShapeKind::PathData);
    ESIA_CHECK(PathPoints(dl, 4).size() == 40 && PathPoints(dl, 4)[39] == ring[39]);
    // nothing to draw, or no area: nothing
    const Vec2 line[3] = {Vec2(0, 0), Vec2(10, 10), Vec2(20, 20)};
    p.Polygon(line, 3, Style().Fill(Color::White()));
    p.Polygon(tri, 3, Style());
    ESIA_CHECK(in.size() == 7);
}

// Rounded corners: the outline is inset by the radius and the shader takes the radius off its distance again - convex
// corners become exact arcs, the bounds stay the polygon's. A concave corner gets a fillet of points; a corner takes
// its radius as far as its edges allow.
ESIA_TEST(Painter, PolygonCornersAreRounded)
{
    DrawList dl = MakeList();
    Painter p(dl, Fx());
    const Vec2 box[4] = {Vec2(100, 100), Vec2(200, 100), Vec2(200, 150), Vec2(100, 150)};
    p.Polygon(box, 4, Style().Fill(Color::White()).Radius(10));
    const fx::Instance& r = dl.FxInstances()[0];
    ESIA_CHECK(r.radii[0] == 10 && r.radii[2] == 4);
    ESIA_CHECK_NEAR(r.rect[0], 100, 1e-4f);
    ESIA_CHECK_NEAR(r.rect[3], 150, 1e-4f);
    const std::vector<Vec2> inset = PathPoints(dl, 0);
    ESIA_CHECK_NEAR(inset[0].x, 110, 1e-4f);
    ESIA_CHECK_NEAR(inset[0].y, 110, 1e-4f);
    ESIA_CHECK_NEAR(inset[2].x, 190, 1e-4f);
    ESIA_CHECK_NEAR(inset[2].y, 140, 1e-4f);
    // a radius larger than the shape: as round as its shorter side allows (a capsule's ends)
    p.Polygon(box, 4, Style().Fill(Color::White()).Radius(500));
    ESIA_CHECK_NEAR(dl.FxInstances()[2].radii[0], 25, 1e-3f);
    // an arrow: its two concave corners rounded with fillets of points
    const Vec2 arrow[7] = {Vec2(10, 20), Vec2(40, 20), Vec2(40, 5), Vec2(70, 30), Vec2(40, 55), Vec2(40, 40), Vec2(10, 40)};
    p.Polygon(arrow, 7, Style().Fill(Color::White()));
    p.Polygon(arrow, 7, Style().Fill(Color::White()).Radius(4));
    const auto& in = dl.FxInstances();
    ESIA_CHECK(in[4].radii[2] == 7 && in[6].radii[2] > 9 && in[6].radii[0] > 0.0f);
}

// A slanted rectangle (CSS skewX): the top edge moves left by tan(skew) x half the height, the bottom edge right; the
// style's four radii are its corners.
ESIA_TEST(Painter, ParallelogramsAreSlantedLikeCss)
{
    DrawList dl = MakeList();
    Painter p(dl, Fx());
    p.Parallelogram(Rect(100, 100, 200, 140), -20.0f, Style().Fill(Color::White()));
    const float shift = std::tan(Radians(20.0f)) * 20.0f;
    const fx::Instance& r = dl.FxInstances()[0];
    ESIA_CHECK_NEAR(r.rect[0], 100 - shift, 1e-3f);
    ESIA_CHECK_NEAR(r.rect[2], 200 + shift, 1e-3f);
    const std::vector<Vec2> pts = PathPoints(dl, 0);
    ESIA_CHECK(pts.size() == 4);
    ESIA_CHECK_NEAR(pts[0].x, 100 + shift, 1e-3f);   // top left: a negative angle leans right
    ESIA_CHECK_NEAR(pts[3].x, 100 - shift, 1e-3f);   // bottom left
    // per-corner radii: the rounded corners get fillets, the inset is the smallest convex radius (0)
    p.Parallelogram(Rect(100, 160, 200, 200), -20.0f, Style().Fill(Color::White()).Radius(0, 8, 0, 8));
    const fx::Instance& c = dl.FxInstances()[2];
    ESIA_CHECK(c.radii[0] == 0 && c.radii[2] > 6);
    p.RegularPolygon(Vec2(300, 100), 20, 6, 0.0f, Style().Fill(Color::White()));
    p.Star(Vec2(300, 200), 30, 12, 5, 0.0f, Style().Fill(Color::White()));
    ESIA_CHECK(dl.FxInstances()[4].radii[2] == 6 && dl.FxInstances()[6].radii[2] == 10);
    bool top = false;   // the first corner at the top
    for (const Vec2& q : PathPoints(dl, 4))
        top = top || std::fabs(q.y - 80.0f) < 1e-3f;
    ESIA_CHECK(top);
}

// Glass on a thin shape - a ring, a line, a path - is one rounded rod: its bevel no deeper than the shape goes.
ESIA_TEST(Painter, GlassBevelsComeNoDeeperThanTheShape)
{
    DrawList dl = MakeList();
    Painter p(dl, Fx());
    GlassMaterial g;
    g.bezel = 40;
    p.Ring(Vec2(100, 100), 30, 8, Style().Glass(g));
    p.Line(Vec2(10, 200), Vec2(100, 200), 10, Style().Glass(g));
    p.Rect(Rect(200, 200, 300, 260), Style().Glass(g));   // the shader clamps a rectangle's: left as it is
    const Vec2 bar[4] = {Vec2(200, 10), Vec2(380, 10), Vec2(380, 22), Vec2(200, 22)};
    p.Polygon(bar, 4, Style().Glass(g));
    const auto& in = dl.FxInstances();
    ESIA_CHECK(in[0].glass[2] == 4 && in[1].glass[2] == 5 && in[2].glass[2] == 40);
    ESIA_CHECK((in[3].flags[0] & fx::kGlass) && in[3].glass[2] > 5.5f && in[3].glass[2] <= 6.0f);
}

// Lines, areas and ring sectors with a material geometry cannot draw become paths; a plain one stays geometry.
ESIA_TEST(Painter, LinesAreasAndSectorsWithAMaterialArePaths)
{
    DrawList dl = MakeList();
    Painter p(dl, Fx());
    const Vec2 pts[4] = {Vec2(0, 50), Vec2(40, 10), Vec2(80, 30), Vec2(120, 5)};
    GlassMaterial g;
    p.Polyline(pts, 4, 6.0f, Style().Glass(g));
    const auto& in = dl.FxInstances();
    ESIA_CHECK(in.size() == 2 && in[0].flags[2] == (std::uint32_t)fx::ShapeKind::Path && in[0].radii[3] == 1.0f && in[0].radii[0] == 3);
    ESIA_CHECK(!(in[0].flags[0] & fx::kFill));   // glass alone stays clear
    ESIA_CHECK_NEAR(in[0].rect[0], -3, 1e-4f);   // the bounds grown by half the thickness
    p.Polyline(pts, 4, 6.0f, Style().Stroke(1, Color::White()).Shadow(Color::Black(0.5f), 6));   // a stroke's color is the line's
    ESIA_CHECK(in.size() == 4 && (in[2].flags[0] & fx::kFill) && !(in[2].flags[0] & fx::kStroke) && in[2].fill0[0] == 1.0f);
    p.Area(pts, 4, 60.0f, Style().Fill(Color::White()));   // plain: geometry
    ESIA_CHECK(in.size() == 4 && !dl.Vertices().empty());
    p.Area(pts, 4, 60.0f, Style().Fill(Color::White()).Radius(4).Glass(g));
    ESIA_CHECK(in.size() == 6 && in[4].radii[3] == 0.0f);
    ESIA_CHECK_NEAR(in[4].rect[3], 60.0f, 1e-4f);
    p.Sector(Vec2(200, 200), 20, 40, 0.0f, kTau, Style().Glass(g));   // a whole ring: the FX ring
    ESIA_CHECK(in.size() == 7 && in[6].flags[2] == (std::uint32_t)fx::ShapeKind::Arc && in[6].radii[0] == 30 && in[6].radii[1] == 10);
    p.Sector(Vec2(200, 200), 0, 40, 0.0f, kPi * 0.5f, Style().Fill(Color::White()));   // a pie's slice: from the center
    ESIA_CHECK(in.size() == 9 && in[7].flags[2] == (std::uint32_t)fx::ShapeKind::Path);
    bool center = false;
    for (const Vec2& q : PathPoints(dl, 7))
        center = center || LengthSq(q - Vec2(200, 200)) < 1e-6f;
    ESIA_CHECK(center);
}

// Paths follow the scale stack (their points where they are drawn, the material's lengths with them) and are culled
// as a whole, their points with them.
ESIA_TEST(Painter, PathsFollowTheScaleAndAreCulledWhole)
{
    DrawList dl = MakeList();
    Painter p(dl, Fx());
    const Vec2 tri[3] = {Vec2(0, 0), Vec2(100, 0), Vec2(0, 100)};
    p.PushScale(Vec2(0, 0), 0.5f);
    p.Polygon(tri, 3, Style().Fill(Color::White()).Radius(10).Shadow(Color::Black(0.3f), 8));
    p.PopScale();
    const fx::Instance& r = dl.FxInstances()[0];
    ESIA_CHECK(r.radii[0] > 0.0f && r.shadowParams[0] == 4.0f);
    ESIA_CHECK(r.rect[2] <= 50.0f + 1e-4f && r.rect[3] <= 50.0f + 1e-4f);
    const Vec2 far[3] = {Vec2(1000, 1000), Vec2(1100, 1000), Vec2(1000, 1100)};
    p.Polygon(far, 3, Style().Fill(Color::White()));
    ESIA_CHECK(dl.FxInstances().size() == 2);
}

// Plain shapes - a solid fill and / or a solid stroke on a rectangle with one radius on every corner, a circle, a
// capsule, a segment - are quads (PainterEnv::quadShapes): the fill, then the stroke, each with what the FX shader
// would read for it.
ESIA_TEST(Painter, PlainShapesBecomeQuads)
{
    DrawList dl = MakeList();
    Painter p(dl);
    p.Rect(Rect(10, 20, 110, 70), Style().Fill(Color::Hex(0xFF0000)).Radius(6).Stroke(2, Color::White(), 0.5f));
    p.Circle(Vec2(200, 200), 10, Style().Fill(Color::White(0.5f)));
    p.Capsule(Rect(10, 100, 60, 120), Style().Stroke(1, Color::White()));
    p.Line(Vec2(10, 10), Vec2(50, 30), 6, Style().Fill(Color::White()).Stroke(1, Color::Black(), 1.0f));
    p.Rect(Rect(150, 20, 190, 60), Style().Fill(Color::White()).Opacity(0.5f));   // its opacity in the color
    ESIA_CHECK(dl.FxInstances().empty() && dl.Commands().size() == 1 && dl.Quads().size() == 7);
    if (dl.Quads().size() != 7)
        return;
    const QuadInstance* q = dl.Quads().data();
    const std::uint32_t half = Color::White(0.5f).ToRgba8() >> 24;
    ESIA_CHECK(q[0].Shape() == QuadShape::Fill && q[0].rect[0] == 10 && q[0].rect[3] == 70 && q[0].Radius() == 6);
    ESIA_CHECK(std::fabs(q[0].Smoothing() - 0.6f) < 1e-3f);   // PainterEnv::cornerSmoothing
    ESIA_CHECK(q[0].packed[2] == (float)(Color::Hex(0xFF0000).ToRgba8() & 0xFFFFFFu) && q[0].Alpha() == 255);
    ESIA_CHECK(q[1].Shape() == QuadShape::Stroke && q[1].StrokeWidth() == 2 && q[1].StrokeAlign() == 0.5f && q[1].Radius() == 6);
    ESIA_CHECK(q[2].Shape() == QuadShape::Fill && q[2].Radius() == 10 && q[2].Smoothing() == 0.0f && q[2].Alpha() == half);   // circles: circular corners
    ESIA_CHECK(q[3].Shape() == QuadShape::Stroke && q[3].Radius() == 10 && q[3].StrokeWidth() == 1);   // a capsule: half its height
    ESIA_CHECK(q[4].Shape() == QuadShape::SegmentFill && q[4].rect[0] == 10 && q[4].rect[2] == 50 && q[4].rect[3] == 30 && q[4].Radius() == 3);
    ESIA_CHECK(q[5].Shape() == QuadShape::SegmentStroke && q[5].StrokeWidth() == 1 && q[5].StrokeAlign() == 1);
    ESIA_CHECK(q[6].Alpha() == half);
}

// What only an FX instance draws keeps a shape one: another paint, a shadow, a glow, corners of different radii, a
// stroke that fades, a fill and a stroke fading together, an arc, a mask, a scale - or quadShapes off. Invisible
// shapes and shapes outside the clip are dropped as quads too.
ESIA_TEST(Painter, RichShapesStayFxInstances)
{
    DrawList dl = MakeList();
    Painter p(dl);
    const Color c = Color::Hex(0x0A84FF);
    p.Rect(Rect(10, 10, 50, 50), Style().Fill(Paint::Linear(c, Color::White())));
    p.Rect(Rect(10, 10, 50, 50), Style().Fill(c).Shadow(Color::Black(0.3f), 8));
    p.Rect(Rect(10, 10, 50, 50), Style().Fill(c).Glow(c, 8));
    p.Rect(Rect(10, 10, 50, 50), Style().Fill(c).Radius(2, 4, 6, 8));
    p.Rect(Rect(10, 10, 50, 50), Style().Stroke(2, c).StrokeFade(0.2f));
    p.Rect(Rect(10, 10, 50, 50), Style().Fill(c).Stroke(2, Color::White()).Opacity(0.5f));
    p.Ring(Vec2(100, 100), 20, 3, Style().Fill(c));
    p.PushMask(Rect(0, 0, 100, 100), 8);
    p.Rect(Rect(10, 10, 50, 50), Style().Fill(c));
    p.PopMask();
    p.PushScale(Vec2(30, 30), 0.9f);
    p.Rect(Rect(10, 10, 50, 50), Style().Fill(c));
    p.PopScale();
    Painter(dl, Fx()).Rect(Rect(10, 10, 50, 50), Style().Fill(c));
    ESIA_CHECK(dl.FxInstances().size() == 10 && dl.Quads().empty());
    DrawList dropped = MakeList();
    Painter d(dropped);
    d.Rect(Rect(10, 10, 20, 20), Style());                                   // nothing to draw
    d.Rect(Rect(10, 10, 20, 20), Style().Fill(Color::White()).Opacity(0));   // transparent
    d.Rect(Rect(500, 10, 520, 20), Style().Fill(Color::White()));             // outside the clip
    d.Line(Vec2(-50, -50), Vec2(-20, -20), 2, Style().Fill(Color::White()));
    ESIA_CHECK(dropped.Empty());
}

// FillRect and HLine without antialiasing are Solid quads over the pixels their geometry covered (geometry without
// quadShapes); a rounded FillRect is a Fill shape
ESIA_TEST(Painter, SolidRectanglesAreQuads)
{
    DrawList dl = MakeList();
    Painter p(dl);
    p.FillRect(Rect(10, 10, 30.5f, 20.25f), Color::White());
    p.HLine(10, 50, 40.6f, Color::White(), 1.0f);
    p.FillRect(Rect(10, 50, 30, 60), Color::White(), 3.0f);
    ESIA_CHECK(dl.Vertices().empty() && dl.Quads().size() == 3 && dl.Commands().size() == 1);
    if (dl.Quads().size() != 3)
        return;
    const QuadInstance* q = dl.Quads().data();
    ESIA_CHECK(q[0].Shape() == QuadShape::Solid && q[0].rect[2] == 30.5f && q[0].rect[3] == 20.25f);
    ESIA_CHECK(q[1].Shape() == QuadShape::Solid && q[1].rect[1] == 40.0f && q[1].rect[3] == 41.0f);   // on a pixel row
    ESIA_CHECK(q[2].Shape() == QuadShape::Fill && q[2].Radius() == 3.0f && q[2].Smoothing() == 0.0f);
    DrawList geo = MakeList();
    Painter(geo, Fx()).FillRect(Rect(10, 10, 30, 20), Color::White());
    ESIA_CHECK(geo.Quads().empty() && geo.Vertices().size() == 4);
}

// Polylines as quads (PainterEnv::quadShapes), translucent too: a piece per segment and an end per end, the joints'
// lines shared - a piece's second joint is the next one's first, an end's its piece's - and reached along as far as the
// sharpest miter goes; one glow layer round them. Geometry where quads cannot give its pixels: a turn sharper than 120
// degrees (a clipped miter), a segment too short for the turns at its ends; under a scale; with quadShapes off.
ESIA_TEST(Painter, PolylinesBecomeQuads)
{
    DrawList dl = MakeList();
    Painter p(dl);
    const Vec2 zig[4] = {Vec2(10, 40), Vec2(30, 20), Vec2(50, 40), Vec2(70, 20)};   // quarter turns
    const Color half(1, 1, 1, 0.5f);
    p.Polyline(zig, 4, 4.0f, Style().Fill(half));
    ESIA_CHECK(dl.Vertices().empty() && dl.Quads().size() == 5 && dl.Commands().size() == 1);
    if (dl.Quads().size() != 5)
        return;
    const QuadInstance* q = dl.Quads().data();
    for (int i = 0; i < 5; ++i)
    {
        ESIA_CHECK(q[i].Shape() == (i < 3 ? QuadShape::LinePiece : QuadShape::LineCap) && q[i].Radius() == 2.0f);
        ESIA_CHECK(q[i].Alpha() == half.ToRgba8() >> 24 && q[i].LineReach() == 7u);   // a quarter turn's miter: 1.41 half thicknesses
    }
    ESIA_CHECK(q[0].rect[0] == 10.0f && q[0].rect[3] == 20.0f && q[2].rect[2] == 70.0f);
    // the ends' lines along the segments' normals (45 degrees), the miters' straight down (90)
    ESIA_CHECK(q[0].LineJoint() == 16384u && q[0].LineOther() == 32768u && q[1].LineJoint() == 32768u && q[1].LineOther() == 32768u);
    ESIA_CHECK(q[2].LineJoint() == 32768u && q[2].LineOther() == 16384u);
    ESIA_CHECK(q[3].LineJoint() == q[0].LineJoint() && q[3].rect[0] == 10.0f && q[3].rect[2] == 30.0f && q[3].LineOther() == 3u);   // 3 sides
    ESIA_CHECK(q[4].LineJoint() == q[2].LineOther() && q[4].rect[0] == 70.0f && q[4].rect[2] == 50.0f);
    // many short pieces of a smooth line; one glow for the whole line
    DrawList smooth = MakeList();
    const Vec2 chart[5] = {Vec2(10, 100), Vec2(40, 80), Vec2(70, 110), Vec2(100, 70), Vec2(130, 95)};
    Painter(smooth).Polyline(chart, 5, 2.0f, Style().Fill(Color::White()).Glow(Color::White(), 8), PolylineFlags_Smooth);
    ESIA_CHECK(smooth.Vertices().empty() && smooth.Quads().size() > 20);
    ESIA_CHECK(smooth.Commands().size() == 3 && smooth.Commands()[0].kind == DrawCmdKind::LayerBegin && smooth.Commands()[2].kind == DrawCmdKind::LayerEnd);
    // geometry
    auto geometry = [](const Vec2* pts, int n, float thickness, const PainterEnv& env, bool scaled) {
        DrawList list = MakeList();
        Painter g(list, env);
        if (scaled)
            g.PushScale(Vec2(0, 0), 2.0f);
        g.Polyline(pts, n, thickness, Style().Fill(Color::White()));
        if (scaled)
            g.PopScale();
        return list.Quads().empty() && !list.Vertices().empty();
    };
    const Vec2 back[3] = {Vec2(10, 10), Vec2(60, 10), Vec2(15, 30)};                   // turns 156 degrees
    const Vec2 kink[4] = {Vec2(10, 60), Vec2(40, 60), Vec2(42, 63), Vec2(42, 90)};     // 3.6 long between turns the same way, 10 thick
    ESIA_CHECK(geometry(back, 3, 4.0f, PainterEnv(), false) && geometry(kink, 4, 10.0f, PainterEnv(), false));
    ESIA_CHECK(geometry(zig, 4, 4.0f, PainterEnv(), true) && geometry(zig, 4, 4.0f, Fx(), false));
}

// A solid line or polyline drawn with its Color (no Style built) is the one its Style().Fill(color) draws, quad for quad
// and vertex for vertex: under an alpha, at a fractional pixel scale, outside the clip (nothing), thinner than a pixel,
// with a repeated point and a turn sharper than 120 degrees (through the Style: BuildPath, the geometry), with
// quadShapes off.
ESIA_TEST(Painter, SolidLinesAreTheirStyledLines)
{
    const Color c(0.2f, 0.6f, 0.9f, 0.8f);
    const Vec2 zig[5] = {Vec2(10, 40), Vec2(30, 20), Vec2(30, 20), Vec2(50, 40), Vec2(70, 20)};
    const Vec2 wave[4] = {Vec2(10, 90), Vec2(40, 70), Vec2(70, 95), Vec2(100, 75)};
    const Vec2 back[3] = {Vec2(10, 10), Vec2(60, 10), Vec2(15, 30)};
    for (int k = 0; k < 4; ++k)
    {
        PainterEnv env;
        env.alpha = k == 1 ? 0.5f : 1.0f;
        env.quadShapes = k != 2;
        env.pixelScale = k == 3 ? 1.5f : 1.0f;
        DrawList a = MakeList(), b = MakeList();
        Painter pa(a, env), pb(b, env);
        pa.Line(Vec2(5, 5), Vec2(90, 60), 1.5f, c);
        pb.Line(Vec2(5, 5), Vec2(90, 60), 1.5f, Style().Fill(c));
        pa.Line(Vec2(500, 5), Vec2(600, 60), 1.5f, c);   // outside the clip
        pb.Line(Vec2(500, 5), Vec2(600, 60), 1.5f, Style().Fill(c));
        pa.Polyline(wave, 4, 3.0f, c);
        pb.Polyline(wave, 4, 3.0f, Style().Fill(c));
        pa.Polyline(wave, 4, 0.5f, c);
        pb.Polyline(wave, 4, 0.5f, Style().Fill(c));
        pa.Polyline(zig, 5, 3.0f, c);
        pb.Polyline(zig, 5, 3.0f, Style().Fill(c));
        pa.Polyline(back, 3, 4.0f, c);
        pb.Polyline(back, 3, 4.0f, Style().Fill(c));
        ESIA_CHECK(a.Quads().size() == b.Quads().size() && a.Vertices().size() == b.Vertices().size() && a.Indices().size() == b.Indices().size());
        ESIA_CHECK(a.FxInstances().size() == b.FxInstances().size() && a.Commands().size() == b.Commands().size());
        if (a.Quads().size() != b.Quads().size() || a.Vertices().size() != b.Vertices().size() || a.FxInstances().size() != b.FxInstances().size())
            continue;
        ESIA_CHECK(std::memcmp(a.Quads().data(), b.Quads().data(), a.Quads().size() * sizeof(QuadInstance)) == 0);
        ESIA_CHECK(std::memcmp(a.Vertices().data(), b.Vertices().data(), a.Vertices().size() * sizeof(Vertex)) == 0);
        ESIA_CHECK(std::memcmp(a.FxInstances().data(), b.FxInstances().data(), a.FxInstances().size() * sizeof(fx::Instance)) == 0);
        ESIA_CHECK(k == 2 ? a.Quads().empty() : a.Quads().size() == 1 + 5 + 5 + 5);   // the line, the waves, the zigzag less its repeat; the turn geometry
    }
}

// Every vector level this processor runs (esia::cpu: SSE2, AVX2, AVX-512, NEON) draws polylines bit for bit as plain C++
// does - quads, the geometry of the ones left to it, the bounds: lines of 2 to 71 points (every length of the vector
// loops' tails) turning either way, hairlines to thick ones, translucent ones, at whole and fractional scales, with
// pieces short enough to be measured one by one, and long dense series (a pixel or two a point: every piece measured).
ESIA_TEST(Painter, PolylineKernelsAgree)
{
    std::vector<std::vector<Vec2>> lines;
    std::uint32_t seed = 12345u;
    auto rnd = [&seed]() {
        seed = seed * 1664525u + 1013904223u;
        return (float)(seed >> 8) / 16777216.0f;
    };
    for (int count = 2; count < 72; ++count)
    {
        std::vector<Vec2> pts;
        float x = 5.0f + rnd() * 20.0f, y = 40.0f + rnd() * 200.0f;
        for (int i = 0; i < count; ++i)
        {
            pts.push_back(Vec2(x, y));
            x += 3.0f + rnd() * 9.0f;
            y += (rnd() - 0.5f) * 10.0f;
        }
        lines.push_back(pts);
    }
    for (int k = 0; k < 4; ++k)
    {
        std::vector<Vec2> pts;
        for (int i = 0; i < 300 + 7 * k; ++i)
            pts.push_back(Vec2(4.0f + (float)i * (1.0f + 0.25f * (float)k), 150.0f + 30.0f * std::sin((float)i * 0.05f + (float)k) + 2.0f * rnd()));
        lines.push_back(pts);
    }
    auto draw = [&](cpu::Simd level, float scale) {
        cpu::Force(level);
        DrawList dl = MakeList();
        PainterEnv env;
        env.pixelScale = scale;
        Painter p(dl, env);
        int k = 0;
        for (const std::vector<Vec2>& pts : lines)
        {
            const Color c(0.2f, 0.7f, 1.0f, k % 3 == 0 ? 0.5f : 1.0f);
            p.Polyline(pts.data(), (int)pts.size(), 0.6f + (float)(k % 5) * 1.7f, c);
            ++k;
        }
        cpu::Unforce();
        return dl;
    };
    for (const float scale : {1.0f, 1.25f})
    {
        const DrawList ref = draw(cpu::Simd::Scalar, scale);
        ESIA_CHECK(ref.Quads().size() > 1000 && !ref.Vertices().empty());   // quads mostly, a few lines left to geometry
        for (const cpu::Simd level : {cpu::Simd::Sse2, cpu::Simd::Avx2, cpu::Simd::Avx512, cpu::Simd::Neon})
        {
            if (!cpu::Runs(level))
                continue;
            const DrawList got = draw(level, scale);
            ESIA_CHECK(got.Quads().size() == ref.Quads().size() && got.Vertices().size() == ref.Vertices().size());
            ESIA_CHECK(got.Commands().size() == ref.Commands().size());
            if (got.Quads().size() != ref.Quads().size() || got.Vertices().size() != ref.Vertices().size() || got.Commands().size() != ref.Commands().size())
                continue;
            ESIA_CHECK(std::memcmp(got.Quads().data(), ref.Quads().data(), ref.Quads().size() * sizeof(QuadInstance)) == 0);
            ESIA_CHECK(std::memcmp(got.Vertices().data(), ref.Vertices().data(), ref.Vertices().size() * sizeof(Vertex)) == 0);
            for (std::size_t i = 0; i < ref.Commands().size(); ++i)
                ESIA_CHECK(got.Commands()[i].vtxBounds == ref.Commands()[i].vtxBounds && got.Commands()[i].count == ref.Commands()[i].count);
        }
    }
}
