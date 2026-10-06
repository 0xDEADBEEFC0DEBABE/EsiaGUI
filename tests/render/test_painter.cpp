// Painter: the fx::Instance encoding (it must stay WGT's: the shaders read it), culling, masks, scale, glow
// containment, geometry primitives, layers and fades.
#include "esia/render/painter.hpp"
#include "esia/text/glyph_atlas.hpp"
#include "esia_test.hpp"
#include <string>
#include <unordered_map>

using namespace esia;

namespace
{
    // Text stand-in: every character is a 6 x 10 quad on texture 42.
    struct FakeText final : text::TextSystem
    {
        float lastScale = 0.0f;
        Vec2 lastPos;
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
            lastScale = scale;
            lastPos = pos;
            dl.PushTexture(42);
            for (std::size_t i = 0; i < s.size(); ++i)
                dl.AddRectFilled(Rect::FromSize(pos + Vec2(6.0f * scale * (float)i, 0), Vec2(6, 10) * scale), c.ToRgba8());
            dl.PopTexture();
            return Measure(f, s, 0, 0).size;
        }
        void DrawGlyph(DrawList& dl, text::FontRef f, char32_t, Vec2 center, Color c) override
        {
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
    Painter p(dl);
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
    Painter p(dl);
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
    Painter p(dl, env);
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
    Painter p(dl);
    p.Rect(Rect(10, 10, 20, 20), Style());                                 // nothing to draw
    p.Rect(Rect(10, 10, 20, 20), Style().Fill(Color::White()).Opacity(0));   // transparent
    p.Rect(Rect(500, 10, 520, 20), Style().Fill(Color::White()));             // outside the clip
    p.Rect(Rect(-40, 10, -20, 20), Style().Fill(Color::White()).Glow(Color::White(), 20));   // its glow reaches in
    ESIA_CHECK(dl.FxInstances().size() == 1);
}

ESIA_TEST(Painter, MasksAndScale)
{
    DrawList dl = MakeList();
    Painter p(dl);
    p.PushMask(Rect(0, 0, 100, 100), 12);
    p.PushScale(Vec2(50, 50), 0.5f);
    p.Rect(Rect(0, 0, 100, 100), Style().Fill(Color::White()).Radius(20).Glow(Color::White(), 10));
    dl.AddRectFilled(Rect(0, 0, 100, 100), 0xFFFFFFFFu);
    p.PopScale();
    p.PopMask();
    p.Rect(Rect(0, 0, 10, 10), Style().Fill(Color::White()));
    const fx::Instance& i = dl.FxInstances()[0];
    ESIA_CHECK((i.flags[0] & fx::kMask) && i.mask[2] == 100 && i.maskParams[0] == 12);
    ESIA_CHECK(i.rect[0] == 25 && i.rect[2] == 75 && i.radii[0] == 10 && i.glowParams[0] == 5);
    ESIA_CHECK(!(dl.FxInstances()[1].flags[0] & fx::kMask));
    // geometry drawn under the scale is scaled at PopScale
    ESIA_CHECK(dl.Vertices()[0].pos == Vec2(25, 25) && dl.Vertices()[2].pos == Vec2(75, 75));
}

ESIA_TEST(Painter, TextUnderScaleIsRasterizedScaledNotStretched)
{
    DrawList dl = MakeList();
    FakeText text;
    PainterEnv env;
    env.text = &text;
    Painter p(dl, env);
    p.PushScale(Vec2(0, 0), 2.0f);
    const Vec2 size = p.Text(Vec2(10, 10), text::FontRef{1, 10}, Color::White(), "ab");
    p.PopScale();
    ESIA_CHECK(text.lastScale == 2.0f && text.lastPos == Vec2(20, 20));
    ESIA_CHECK(size == Vec2(12, 10));   // the layout size, unscaled
    // the glyph quads keep their final geometry: PopScale skips them
    ESIA_CHECK(dl.Vertices()[0].pos == Vec2(20, 20) && dl.Vertices()[2].pos == Vec2(32, 40));
    ESIA_CHECK(dl.Commands().back().texture == 42);

    const Vec2 box = p.TextBox(Rect(0, 100, 100, 140), Vec2(0.5f, 0.5f), text::FontRef{1, 10}, Color::White(), "abcd");
    ESIA_CHECK(box == Vec2(24, 10) && text.lastPos == Vec2(38, 115));
    p.Icon(Vec2(50, 50), text::FontRef{2, 16}, U'\xE700', Color::White());
    ESIA_CHECK(text.lastScale == 16.0f);
}

ESIA_TEST(Painter, GlowContainmentReportsRestGeometryAndHalo)
{
    DrawList dl = MakeList();
    FakeGlow glow;
    PainterEnv env;
    env.glow = &glow;
    Painter p(dl, env);
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
    Painter p(dl, env);
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
    Painter p(dl, env);
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
    Painter p(dl, env);
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
    Painter p(dl);
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
    Painter p(dl, env);
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

    int GeometryCommands(const DrawList& dl, TextureId texture)
    {
        int n = 0;
        for (const DrawCmd& c : dl.Commands())
            n += c.kind == DrawCmdKind::Geometry && c.count > 0 && c.texture == texture ? 1 : 0;
        return n;
    }
}

// Flat drawing: rounded rectangles, capsules and circles are geometry on the text's texture - one command with the
// text around them - from coverage tiles made once per kind of shape; no shadows; glass a solid surface
ESIA_TEST(Painter, FlatShapesAreGeometryThatBatchesWithText)
{
    TileText tt;
    DrawList dl = MakeList();
    PainterEnv env;
    env.text = &tt;
    env.flat = true;
    env.flatSurface = Color(0.1f, 0.1f, 0.1f, 1.0f);
    Painter p(dl, env);
    const Style button = Style().Radius(18).Fill(Color::Hex(0x0A84FF)).Shadow(Color::Black(0.3f), 12, Vec2(0, 4));
    p.Text(Vec2(10, 10), text::FontRef{1, 10.0f}, Color::White(), "Label");
    p.Rect(Rect(80, 4, 160, 40), button);
    p.Text(Vec2(100, 15), text::FontRef{1, 10.0f}, Color::White(), "OK");
    p.Capsule(Rect(80, 60, 160, 96), Style().Fill(Color::Hex(0x0A84FF)));   // the same shape elsewhere: the same tile
    p.Circle(Vec2(200, 20), 10, Style().Fill(Color::White()));
    ESIA_CHECK(dl.FxInstances().empty());
    ESIA_CHECK(GeometryCommands(dl, 42) == 1);   // the text and the shapes: one command
    ESIA_CHECK(tt.added == 2);                  // the button's tile and the circle's
    // a capsule 80 x 36: stretched along x (3 cells), whole along y; a circle: one quad
    ESIA_CHECK(dl.Vertices().size() == (std::size_t)(5 * 4 + 8 + 2 * 4 + 8 + 4));

    // a large fill: its middle from the white texture, the rim from the tile; a stroke's ring around it
    DrawList d2 = MakeList();
    Painter q(d2, env);
    q.Rect(Rect(10, 10, 310, 210), Style().Radius(20).Fill(Color(0.2f, 0.2f, 0.2f, 1)).Stroke(1.0f, Color::White(0.3f)));
    ESIA_CHECK(d2.FxInstances().empty() && GeometryCommands(d2, 0) == 1 && GeometryCommands(d2, 42) >= 1);
    ESIA_CHECK(tt.added == 4);

    // what geometry cannot do stays an FX shape, without the shadow and glow flat drawing leaves out
    DrawList d3 = MakeList();
    Painter r(d3, env);
    r.Rect(Rect(10, 10, 60, 60), Style().Radius(8).Fill(Paint::Linear(Color::White(), Color::Black())).Shadow(Color::Black(0.3f), 8).Glow(Color::White(), 6));
    ESIA_CHECK(d3.FxInstances().size() == 1 && !(d3.FxInstances()[0].flags[0] & (fx::kShadow | fx::kGlow)));

    // glass: a solid surface, flatSurface under its tint
    DrawList d4 = MakeList();
    Painter g(d4, env);
    GlassMaterial m;
    m.tint = Color(1, 1, 1, 0.5f);
    g.Rect(Rect(10, 10, 60, 60), Style().Radius(8).Glass(m));
    ESIA_CHECK(d4.FxInstances().empty() && !d4.Vertices().empty());
    if (!d4.Vertices().empty())
        ESIA_CHECK(d4.Vertices()[0].color == Color(0.55f, 0.55f, 0.55f, 1.0f).ToRgba8());

    // the tiles are gone when the text system starts its pages over: made again
    ++tt.generation;
    tt.tiles.clear();
    const int before = tt.added;
    DrawList d5 = MakeList();
    Painter h(d5, env);
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
    Painter p(dl, env);
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
