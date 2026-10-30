// Esia conformance suite - scene definitions (see scenes.hpp). Every scene is deterministic: fixed clock, no
// randomness; what moves (shimmer, light streak) is sampled at the same time on every run.
#include "scenes.hpp"
#include "esia/render/painter.hpp"
#include "esia/text/distance_field.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace esia::conformance
{
    namespace
    {
        constexpr double kTime = 1.25;

        // What the scenes' painters start from: plain shapes as quads, or as FX instances while a scene with
        // Scene::fxShapes is built (Build)
        bool gFxShapes = false;

        PainterEnv Env()
        {
            PainterEnv env;
            env.quadShapes = !gFxShapes;
            return env;
        }

        // iOS dark-mode system colors
        const Color kBlue = Color::Hex(0x0A84FF), kGreen = Color::Hex(0x30D158), kIndigo = Color::Hex(0x5E5CE6), kOrange = Color::Hex(0xFF9F0A),
                    kPink = Color::Hex(0xFF375F), kPurple = Color::Hex(0xBF5AF2), kRed = Color::Hex(0xFF453A), kTeal = Color::Hex(0x40C8E0),
                    kYellow = Color::Hex(0xFFD60A), kGray = Color::Hex(0x8E8E93);

        GlassMaterial Glass(float blur, float refraction = 8.0f, float bezel = 40.0f)
        {
            GlassMaterial g;
            g.blur = blur;
            g.refraction = refraction;
            g.bezel = bezel;
            return g;
        }

        // ------------------------------------------------------------------ test atlas
        // Eight "glyphs" in 16 x 32 cells, rasterized analytically: the stand-in for a font engine's glyph pages.
        constexpr int kCell = 16, kCells = 8, kAtlasW = kCell * kCells, kAtlasH = 32;

        float Box(float px, float py, float cx, float cy, float hx, float hy)
        {
            const float qx = std::fabs(px - cx) - hx, qy = std::fabs(py - cy) - hy;
            const float ox = std::max(qx, 0.0f), oy = std::max(qy, 0.0f);
            return std::sqrt(ox * ox + oy * oy) + std::min(std::max(qx, qy), 0.0f);
        }

        float Segment(float px, float py, float ax, float ay, float bx, float by, float half)
        {
            const float pax = px - ax, pay = py - ay, bax = bx - ax, bay = by - ay;
            const float h = std::clamp((pax * bax + pay * bay) / (bax * bax + bay * bay), 0.0f, 1.0f);
            const float dx = pax - bax * h, dy = pay - bay * h;
            return std::sqrt(dx * dx + dy * dy) - half;
        }

        float GlyphDistance(int glyph, float x, float y)
        {
            const float cx = 8.0f, cy = 17.0f;
            switch (glyph)
            {
            case 0: return Box(x, y, cx, cy, 1.6f, 9.0f);                                                     // bar
            case 1: return std::fabs(std::hypot(x - cx, y - 20.0f) - 5.0f) - 1.3f;                              // ring
            case 2: return std::hypot(x - cx, y - 22.0f) - 4.2f;                                                 // dot
            case 3: return std::max({y - 26.0f, (x - cx) * 1.6f - (y - 9.0f) * 0.55f, (cx - x) * 1.6f - (y - 9.0f) * 0.55f});   // triangle
            case 4: return Segment(x, y, 3.0f, 26.0f, 13.0f, 8.0f, 1.4f);                                       // diagonal
            case 5: return std::min({Box(x, y, 5.5f, cy, 1.4f, 9.0f), Box(x, y, 9.0f, 8.8f, 4.5f, 1.2f), Box(x, y, 9.0f, cy, 4.0f, 1.2f),
                                     Box(x, y, 9.0f, 25.2f, 4.5f, 1.2f)});                                      // E
            case 6: return Box(x, y, cx, 21.0f, 4.5f, 4.5f);                                                   // square
            default: return std::min(Box(x, y, cx, 19.0f, 5.5f, 1.3f), Box(x, y, cx, 19.0f, 1.3f, 5.5f));    // plus
            }
        }

        float Coverage(int glyph, float x, float y) { return std::clamp(0.5f - GlyphDistance(glyph, x, y), 0.0f, 1.0f); }

        // An Alpha8 page of the eight glyphs, or an RGBA8 sub-pixel page (R / G / B coverage a third of a pixel apart,
        // grayscale in A).
        TextureId MakeAtlas(TextureRegistry& reg, bool lcd = false)
        {
            std::vector<std::uint8_t> px((std::size_t)kAtlasW * kAtlasH * (lcd ? 4 : 1));
            for (int y = 0; y < kAtlasH; ++y)
                for (int x = 0; x < kAtlasW; ++x)
                {
                    const int g = x / kCell;
                    const float lx = (float)(x % kCell) + 0.5f, ly = (float)y + 0.5f;
                    const std::size_t i = (std::size_t)y * kAtlasW + (std::size_t)x;
                    if (!lcd)
                    {
                        px[i] = (std::uint8_t)std::lround(Coverage(g, lx, ly) * 255.0f);
                        continue;
                    }
                    const float r = Coverage(g, lx - 1.0f / 3.0f, ly), gg = Coverage(g, lx, ly), b = Coverage(g, lx + 1.0f / 3.0f, ly);
                    px[i * 4 + 0] = (std::uint8_t)std::lround(r * 255.0f);
                    px[i * 4 + 1] = (std::uint8_t)std::lround(gg * 255.0f);
                    px[i * 4 + 2] = (std::uint8_t)std::lround(b * 255.0f);
                    px[i * 4 + 3] = (std::uint8_t)std::lround((r + gg + b) / 3.0f * 255.0f);
                }
            TextureInfo info;
            info.format = lcd ? TextureFormat::RGBA8 : TextureFormat::Alpha8;
            info.width = kAtlasW;
            info.height = kAtlasH;
            info.flags = lcd ? TextureFlags_LcdCoverage : TextureFlags_None;
            return reg.Create(info, px.data());
        }

        // "Text": one quad per glyph ('a'..'h', space), `h` UI units high, pixel-snapped like a text engine does; drawn
        // instanced, as glyphs are (QuadInstance).
        float Glyphs(DrawList& dl, TextureId atlas, Vec2 pos, const char* s, float h, Color color, float pixelScale = 1.0f)
        {
            const float w = h * 0.5f;
            float x = pos.x;
            for (; *s; ++s)
            {
                if (*s >= 'a' && *s <= 'h')
                {
                    const int g = *s - 'a';
                    const float sx = std::round(x * pixelScale) / pixelScale, sy = std::round(pos.y * pixelScale) / pixelScale;
                    dl.AddQuad(atlas, Rect(sx, sy, sx + w, sy + h), Vec2((float)g / kCells, 0), Vec2((float)(g + 1) / kCells, 1), color.ToRgba8());
                }
                x += h * 0.56f;
            }
            return x - pos.x;
        }

        // A 64 x 64 test image: checker, a radial glow and a diagonal stripe (orientation is visible).
        TextureId MakeImage(TextureRegistry& reg)
        {
            std::vector<std::uint8_t> px(64 * 64 * 4);
            for (int y = 0; y < 64; ++y)
                for (int x = 0; x < 64; ++x)
                {
                    std::uint8_t* p = px.data() + ((std::size_t)y * 64 + (std::size_t)x) * 4;
                    const bool checker = ((x / 8) + (y / 8)) % 2 == 0;
                    const float d = std::hypot((float)x - 20.0f, (float)y - 20.0f) / 40.0f;
                    const float glow = std::clamp(1.0f - d, 0.0f, 1.0f);
                    const bool stripe = std::abs(x - y) < 4;
                    p[0] = (std::uint8_t)std::clamp((checker ? 40 : 90) + (int)(glow * 160) + (stripe ? 90 : 0), 0, 255);
                    p[1] = (std::uint8_t)std::clamp((checker ? 60 : 110) + (int)(glow * 120), 0, 255);
                    p[2] = (std::uint8_t)std::clamp((checker ? 120 : 170) - (stripe ? 100 : 0), 0, 255);
                    p[3] = 255;
                }
            TextureInfo info;
            info.width = info.height = 64;
            return reg.Create(info, px.data());
        }

        // A colorful backdrop for glass: gradient, disks and hard-edged stripes (refraction and blur are visible).
        void Wallpaper(Painter& p, DrawList& dl, Rect r)
        {
            p.Rect(r, Style().Fill(Paint::Linear(Color::Hex(0x1E3A8A), Color::Hex(0x9D174D), 35)));
            p.Circle(Vec2(r.min.x + r.Width() * 0.25f, r.min.y + r.Height() * 0.35f), r.Height() * 0.22f, Style().Fill(kOrange));
            p.Circle(Vec2(r.min.x + r.Width() * 0.72f, r.min.y + r.Height() * 0.62f), r.Height() * 0.26f, Style().Fill(kTeal));
            for (int i = 0; i < 8; ++i)
            {
                const float x = r.min.x + 12.0f + (float)i * r.Width() / 8.0f;
                dl.AddRectFilled(Rect(x, r.min.y, x + 6.0f, r.max.y), Color(1, 1, 1, 0.55f).ToRgba8());
            }
        }

        // ------------------------------------------------------------------ scenes
        void Shapes(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            p.Rect(Rect(0, 0, 320, 240), Style().Fill(Paint::Linear(Color::Hex(0x1C1C1E), Color::Hex(0x3A3A3C))));
            // corners: square, circular, continuous, per corner, capsule
            p.Rect(Rect(12, 14, 60, 62), Style().Fill(kBlue));
            p.Rect(Rect(72, 14, 120, 62), Style().Fill(kGreen).Radius(14).Smoothing(0));
            p.Rect(Rect(132, 14, 180, 62), Style().Fill(kGreen).Radius(14).Smoothing(1));
            p.Rect(Rect(192, 14, 240, 62), Style().Fill(kOrange).Radius(0, 8, 16, 24));
            p.Capsule(Rect(252, 24, 310, 52), Style().Fill(kPink));
            // circle, arc, ring, lines, liquid merge
            p.Circle(Vec2(36, 100), 22, Style().Fill(kIndigo));
            p.Arc(Vec2(96, 100), 18, 8, -kPi * 0.5f, kPi * 1.5f, Style().Fill(kYellow));
            p.Ring(Vec2(150, 100), 20, 3, Style().Fill(kTeal));
            p.Line(Vec2(186, 80), Vec2(226, 120), 2, Style().Fill(Color::White()));
            p.Line(Vec2(196, 120), Vec2(236, 84), 8, Style().Fill(kRed));
            p.Merge(Rect(250, 80, 284, 114), Rect(272, 96, 306, 124), 12, 10, Style().Fill(kPurple));
            // strokes: inside, centered, outside, stroke only, faded
            p.Rect(Rect(14, 150, 64, 200), Style().Fill(kGray).Radius(10).Stroke(3, Color::White(), 0.0f));
            p.Rect(Rect(80, 150, 130, 200), Style().Fill(kGray).Radius(10).Stroke(4, kYellow, 0.5f));
            p.Rect(Rect(146, 150, 196, 200), Style().Fill(kGray).Radius(10).Stroke(3, kGreen, 1.0f));
            p.Rect(Rect(212, 150, 262, 200), Style().Radius(25).Stroke(2, kBlue));
            p.Capsule(Rect(270, 150, 312, 226), Style().Fill(Color::Hex(0x2C2C2E)).Stroke(2, Color::White(), 0.0f).StrokeFade(0.0f, 90));
            p.Rect(Rect(14, 212, 262, 228), Style().Fill(kBlue).Radius(8).Opacity(0.5f));
        }

        void Gradients(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            p.Rect(Rect(0, 0, 320, 240), Style().Fill(Color::Hex(0x000000)));
            p.Rect(Rect(10, 10, 100, 60), Style().Radius(10).Fill(Paint::Linear(kBlue, kPink, 0)));
            p.Rect(Rect(115, 10, 205, 60), Style().Radius(10).Fill(Paint::Linear(kGreen, kIndigo, 45)));
            p.Rect(Rect(220, 10, 310, 60), Style().Radius(10).Fill(Paint::Linear(kYellow, Color::Hex(0xFF9F0A, 0.0f), 90)));
            p.Rect(Rect(10, 70, 100, 150), Style().Radius(16).Fill(Paint::Radial(Color::White(), kPurple, Vec2(0.35f, 0.35f), 0.6f)));
            p.Circle(Vec2(160, 110), 38, Style().Fill(Paint::Conic(kTeal, kBlue, -90)));
            p.Ring(Vec2(262, 110), 32, 10, Style().Fill(Paint::ConicLoop(kOrange, kPink, -90)));
            p.Capsule(Rect(10, 160, 150, 180), Style().Fill(Paint::Spectrum(1.0f, 0.05f, 0.95f, 0)));
            // chart: area with a vertical fade under a smooth line
            const Vec2 pts[7] = {Vec2(165, 225), Vec2(185, 190), Vec2(205, 205), Vec2(230, 170), Vec2(255, 185), Vec2(280, 160), Vec2(305, 175)};
            p.Area(pts, 7, 232.0f, Paint::Linear(kGreen.WithAlpha(0.6f), kGreen.WithAlpha(0.0f), 90), PolylineFlags_Smooth);
            p.Polyline(pts, 7, 2.5f, Style().Fill(kGreen), PolylineFlags_Smooth);
            const Vec2 zig[5] = {Vec2(12, 225), Vec2(45, 195), Vec2(78, 225), Vec2(111, 195), Vec2(144, 225)};
            p.Polyline(zig, 5, 6.0f, Style().Fill(kOrange.WithAlpha(0.8f)));
        }

        // Neighbours a glow must fade out before (what the layout layer computes from the item map).
        struct Neighbours final : GlowContainment
        {
            void ReportGlowReach(const DrawList&, const Rect&, float) override {}
            bool GlowBounds(const DrawList&, const Rect& shape, float, Rect& bounds, float& fade) override
            {
                bounds = Rect(shape.min.x - 40.0f, shape.min.y - 14.0f, shape.max.x + 40.0f, shape.max.y + 14.0f);
                fade = 6.0f;
                return true;
            }
        };

        void Shadows(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            p.Rect(Rect(0, 0, 320, 240), Style().Fill(Color::Hex(0xF2F2F7)));
            p.Rect(Rect(16, 16, 96, 76), Style().Fill(Color::White()).Radius(14).Shadow(Color::Black(0.35f), 16, Vec2(0, 6)));
            p.Rect(Rect(120, 16, 200, 76), Style().Fill(Color::White()).Radius(14).Shadow(kBlue.WithAlpha(0.6f), 6, Vec2(4, 4), 3));
            p.Rect(Rect(224, 16, 304, 76), Style().Fill(Color::Hex(0xE5E5EA)).Radius(14).InnerShadow(Color::Black(0.4f), 10, Vec2(0, 4)));
            p.Rect(Rect(0, 96, 320, 240), Style().Fill(Color::Hex(0x101014)));
            p.Circle(Vec2(56, 150), 18, Style().Fill(kPink).Glow(kPink, 18, 0.9f));
            p.Rect(Rect(120, 132, 200, 168), Style().Fill(Color::Hex(0x1C1C1E)).Radius(12).InnerGlow(14, 0.8f).Glow(kTeal, 4, 0.5f));
            Neighbours n;
            PainterEnv env = Env();
            env.glow = &n;
            Painter contained(dl, env);
            contained.Capsule(Rect(226, 136, 294, 164), Style().Fill(kGreen).Glow(kGreen, 22, 1.0f));
            p.Rect(Rect(16, 196, 196, 208), Style().Fill(Color::Hex(0x2C2C2E)).Radius(6).Shimmer(0.5f, 0.6f));
            p.Rect(Rect(16, 216, 136, 228), Style().Fill(Color::Hex(0x2C2C2E)).Radius(6).Shimmer(0.5f, 0.6f));
            p.Rect(Rect(212, 192, 304, 230), Style().Fill(Color::Hex(0x3A3A3C)).Radius(10).Noise(0.08f));
        }

        // The glass scene's shapes, on its wallpaper.
        void GlassCards(Painter& p)
        {
            // frosted card with legibility, and a nearly clear capsule on it (reads the full-resolution level)
            GlassMaterial card = Glass(12, 10, 24);
            card.legibility = 0.4f;
            card.tint = Color(1, 1, 1, 0.16f);
            p.Rect(Rect(16, 20, 170, 150), Style().Radius(24).Glass(card).Shadow(Color::Black(0.3f), 16, Vec2(0, 8)));
            p.Capsule(Rect(32, 100, 154, 134), Style().Glass(Glass(1, 12, 17)));
            // a clear lens: strong refraction, dispersion and magnification
            GlassMaterial lens = Glass(0, 16, 40);
            lens.dispersion = 0.9f;
            lens.magnify = 0.25f;
            lens.tint = Color(1, 1, 1, 0.0f);
            p.Circle(Vec2(248, 70), 48, Style().Glass(lens));
            // dark tinted glass bar
            GlassMaterial dark = Glass(6, 6, 20);
            dark.tint = Color(0, 0, 0, 0.35f);
            dark.rim = Color(0, 0, 0, 0.3f);
            p.Capsule(Rect(186, 170, 308, 214), Style().Glass(dark));
            p.Capsule(Rect(16, 176, 170, 212), Style().Glass(Glass(4, 8, 18)).Fill(kBlue.WithAlpha(0.35f)));
        }

        void GlassScene(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            Wallpaper(p, dl, Rect(0, 0, 320, 240));
            GlassCards(p);
        }

        // Paths (fx::ShapeKind::Path): any outline with every material, on the glass scene's wallpaper - slanted tabs of
        // frosted and tinted glass and an outlined one with per-corner radii, a rounded triangle with a gradient and a
        // shadow, a star of clear dispersive glass, a hexagon of frosted glass with an inner glow, an arrow whose
        // concave corners are rounded too, a glass ring and a thick glass segment (their bevel no deeper than they
        // are), a glass polyline (round joins), a glass area chart and a glass ring sector.
        void Paths(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            Wallpaper(p, dl, Rect(0, 0, 320, 240));
            GlassMaterial tab = Glass(6, 8, 14);
            tab.tint = Color(1, 1, 1, 0.12f);
            p.Parallelogram(Rect(14, 12, 84, 40), -18.0f, Style().Radius(6).Glass(tab));
            GlassMaterial selected = tab;
            selected.tint = Color(kBlue.r, kBlue.g, kBlue.b, 0.45f);
            p.Parallelogram(Rect(88, 12, 158, 40), -18.0f, Style().Radius(6).Glass(selected).Shadow(Color::Black(0.3f), 8, Vec2(0, 3)));
            p.Parallelogram(Rect(162, 12, 226, 40), -18.0f, Style().Radius(0, 10, 0, 10).Fill(Color::Black(0.3f)).Stroke(1.5f, Color::White(0.85f)));
            const Vec2 tri[3] = {Vec2(272, 12), Vec2(308, 66), Vec2(236, 66)};
            p.Polygon(tri, 3, Style().Radius(8).Fill(Paint::Linear(kOrange, kPink, 90)).Shadow(Color::Black(0.4f), 10, Vec2(0, 4)));
            GlassMaterial clear = Glass(0, 10, 9);
            clear.dispersion = 0.6f;
            clear.tint = Color(1, 1, 1, 0.05f);
            p.Star(Vec2(48, 98), 36, 16, 5, 0.0f, Style().Radius(4).Glass(clear));
            p.RegularPolygon(Vec2(130, 98), 34, 6, 30.0f, Style().Radius(10).Glass(Glass(10, 10, 20)).InnerGlow(10, 0.4f));
            const Vec2 arrow[7] = {Vec2(178, 86), Vec2(212, 86), Vec2(212, 70), Vec2(244, 98), Vec2(212, 126), Vec2(212, 110), Vec2(178, 110)};
            p.Polygon(arrow, 7, Style().Radius(5).Fill(kGreen.WithAlpha(0.9f)).Stroke(2, Color::White(0.9f)));
            p.Ring(Vec2(282, 104), 22, 12, Style().Glass(Glass(2, 10, 30)));
            p.Line(Vec2(252, 144), Vec2(310, 144), 12, Style().Glass(Glass(2, 10, 30)));
            const Vec2 zig[5] = {Vec2(16, 214), Vec2(40, 160), Vec2(66, 214), Vec2(92, 160), Vec2(116, 214)};
            p.Polyline(zig, 5, 12.0f, Style().Fill(Color::White(0.1f)).Glass(Glass(3, 8, 20)));
            const Vec2 chart[6] = {Vec2(132, 200), Vec2(152, 174), Vec2(172, 188), Vec2(192, 158), Vec2(212, 172), Vec2(232, 150)};
            GlassMaterial area = Glass(8, 10, 24);
            area.tint = Color(kGreen.r, kGreen.g, kGreen.b, 0.3f);
            p.Area(chart, 6, 228.0f, Style().Radius(6).Glass(area), PolylineFlags_Smooth);
            GlassMaterial sector = Glass(4, 10, 20);
            sector.tint = Color(kPurple.r, kPurple.g, kPurple.b, 0.35f);
            p.Sector(Vec2(282, 200), 16, 38, kPi, kPi * 0.75f, Style().Radius(4).Glass(sector), 2.0f);
        }

        void CountCallback(const DrawList&, const DrawCmd& cmd, void*) { ++*static_cast<int*>(cmd.userData); }

        // The glass scene with a host callback between the backdrop's content and the first glass: the pass the host
        // code ran in ends for the capture and resumes with everything bound again. Host code is API specific, so the
        // callbacks draw nothing and the image is the glass scene's. A second callback has an empty clip: it must
        // not run (it would draw unclipped).
        void CallbackCapture(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            Wallpaper(p, dl, Rect(0, 0, 320, 240));
            dl.AddCallback(&CountCallback, &f.callbacks);
            dl.PushClipRect(Rect(100, 100, 100, 140));
            dl.AddCallback(&CountCallback, &f.callbacks);
            dl.PopClipRect();
            GlassCards(p);
        }

        void GlowLayers(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            p.Rect(Rect(0, 0, 320, 240), Style().Fill(Color::Hex(0x05050A)));
            const TextureId atlas = MakeAtlas(f.textures);
            p.BeginGlowLayer(Color::Hex(0x40C8E0, 0.8f), 10.0f, 1.2f);
            p.Rect(Rect(20, 20, 140, 90), Style().Radius(16).Stroke(3, kTeal));
            Glyphs(dl, atlas, Vec2(34, 40), "abcd", 28, Color::White());
            p.EndGlowLayer();
            p.BeginGlowLayer(Color::White(0.0f), 14.0f, 1.0f, 0.9f);   // untinted: the content's own colors bloom
            p.Circle(Vec2(230, 60), 26, Style().Fill(kPink));
            p.Line(Vec2(180, 110), Vec2(300, 110), 4, Style().Fill(kYellow));
            p.EndGlowLayer();
            const Vec2 pts[6] = {Vec2(20, 200), Vec2(70, 150), Vec2(120, 190), Vec2(170, 140), Vec2(220, 185), Vec2(300, 145)};
            p.Polyline(pts, 6, 3.0f, Style().Fill(kGreen).Glow(kGreen, 10, 0.9f), PolylineFlags_Smooth);
        }

        void Text(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            const TextureId atlas = MakeAtlas(f.textures);
            p.Rect(Rect(0, 0, 320, 120), Style().Fill(Color::Hex(0x1C1C1E)));
            p.Rect(Rect(0, 120, 320, 240), Style().Fill(Color::Hex(0xF2F2F7)));
            Glyphs(dl, atlas, Vec2(12, 10), "abcdefgh", 32, Color::White());
            Glyphs(dl, atlas, Vec2(12, 50), "hgfe dcba", 20, kYellow);
            Glyphs(dl, atlas, Vec2(12, 80), "abc def gha bcd efg", 12, kGray);
            Glyphs(dl, atlas, Vec2(12, 130), "abcdefgh", 32, Color::Black());
            Glyphs(dl, atlas, Vec2(12, 170), "hgfe dcba", 20, kBlue);
            Glyphs(dl, atlas, Vec2(12.4f, 200.3f), "abc def gha bcd efg", 12, Color::Black(0.6f));
        }

        void TextLcd(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            const TextureId lcd = MakeAtlas(f.textures, true);
            p.Rect(Rect(0, 0, 320, 120), Style().Fill(Color::Hex(0x1C1C1E)));
            p.Rect(Rect(0, 120, 320, 240), Style().Fill(Color::Hex(0xF2F2F7)));
            Glyphs(dl, lcd, Vec2(12, 10), "abcdefgh", 32, Color::White());
            Glyphs(dl, lcd, Vec2(12, 60), "hgfe dcba", 16, kOrange);
            Glyphs(dl, lcd, Vec2(12, 130), "abcdefgh", 32, Color::Black());
            Glyphs(dl, lcd, Vec2(12, 180), "hgfe dcba", 16, kBlue);
            // inside a glow layer (an alpha target) sub-pixel pages fall back to their grayscale coverage
            p.BeginGlowLayer(Color::Hex(0xFF375F, 0.6f), 6.0f);
            Glyphs(dl, lcd, Vec2(200, 60), "abc", 24, Color::White());
            p.EndGlowLayer();
        }

        // ------------------------------------------------------------------ glyph fields
        // Three glyphs as distance fields (MTSDF, made here as a text system makes them: 48 field pixels per em,
        // ranges 8 and 24) on one page: an E (right angles), an O (curves, a hole), a star (acute corners).
        constexpr float kFieldEm = 48.0f;
        struct FieldGlyph
        {
            int x = 0, w = 0, h = 0;      // in the page
            int left = 0, top = 0;        // field pixels from the pen
            float advance = 0.0f;         // field pixels
        };
        struct FieldPage
        {
            TextureId page = 0;
            int width = 0, height = 0;
            FieldGlyph glyphs[3];
        };

        FieldPage MakeFieldPage(TextureRegistry& reg)
        {
            text::GlyphShape shapes[3];
            // E: 30 x 36 above the baseline
            text::GlyphShape& e = shapes[0];
            e.MoveTo(Vec2(4, -36));
            for (const Vec2 v : {Vec2(30, -36), Vec2(30, -30), Vec2(11, -30), Vec2(11, -21), Vec2(27, -21), Vec2(27, -15), Vec2(11, -15),
                                 Vec2(11, -6), Vec2(30, -6), Vec2(30, 0), Vec2(4, 0)})
                e.LineTo(v);
            // O: a ring of cubics
            const auto circle = [](text::GlyphShape& s, Vec2 c, float rx, float ry, bool reverse) {
                const float kx = 0.5522847f * rx, ky = 0.5522847f * ry;
                s.MoveTo(c + Vec2(rx, 0));
                if (!reverse)
                {
                    s.CubicTo(c + Vec2(rx, ky), c + Vec2(kx, ry), c + Vec2(0, ry));
                    s.CubicTo(c + Vec2(-kx, ry), c + Vec2(-rx, ky), c + Vec2(-rx, 0));
                    s.CubicTo(c + Vec2(-rx, -ky), c + Vec2(-kx, -ry), c + Vec2(0, -ry));
                    s.CubicTo(c + Vec2(kx, -ry), c + Vec2(rx, -ky), c + Vec2(rx, 0));
                }
                else
                {
                    s.CubicTo(c + Vec2(rx, -ky), c + Vec2(kx, -ry), c + Vec2(0, -ry));
                    s.CubicTo(c + Vec2(-kx, -ry), c + Vec2(-rx, -ky), c + Vec2(-rx, 0));
                    s.CubicTo(c + Vec2(-rx, ky), c + Vec2(-kx, ry), c + Vec2(0, ry));
                    s.CubicTo(c + Vec2(kx, ry), c + Vec2(rx, ky), c + Vec2(rx, 0));
                }
            };
            circle(shapes[1], Vec2(19, -18), 16, 18.5f, false);
            circle(shapes[1], Vec2(19, -18), 9, 11.5f, true);
            // a five-pointed star, its points acute
            text::GlyphShape& star = shapes[2];
            for (int i = 0; i < 10; ++i)
            {
                const float a = -1.5707963f + (float)i * 0.62831853f, r = (i % 2) ? 7.5f : 19.0f;
                const Vec2 v(20.0f + std::cos(a) * r, -18.0f + std::sin(a) * r);
                if (i == 0)
                    star.MoveTo(v);
                else
                    star.LineTo(v);
            }
            FieldPage page;
            text::GlyphBitmap fields[3];
            int x = 0, h = 0;
            for (int i = 0; i < 3; ++i)
            {
                text::GenerateMtsdf(shapes[i], {8.0f, 24.0f}, fields[i]);
                FieldGlyph& g = page.glyphs[i];
                g.x = x;
                g.w = fields[i].width;
                g.h = fields[i].height;
                g.left = fields[i].left;
                g.top = fields[i].top;
                g.advance = i == 0 ? 34.0f : 40.0f;
                x += g.w + 1;
                h = std::max(h, g.h);
            }
            page.width = x;
            page.height = h;
            std::vector<std::uint8_t> px((std::size_t)page.width * (std::size_t)page.height * 4, 0);
            for (int i = 0; i < 3; ++i)
                for (int y = 0; y < fields[i].height; ++y)
                    std::memcpy(px.data() + ((std::size_t)y * (std::size_t)page.width + (std::size_t)page.glyphs[i].x) * 4,
                                fields[i].pixels.data() + (std::size_t)y * (std::size_t)fields[i].width * 4, (std::size_t)fields[i].width * 4);
            TextureInfo info;
            info.format = TextureFormat::RGBA8;
            info.width = page.width;
            info.height = page.height;
            info.flags = TextureFlags_DistanceField;
            page.page = reg.Create(info, px.data());
            return page;
        }

        // A run of field glyphs (0 E, 1 O, 2 star) as TextSystem::DrawShapes makes them: the outer layers of all of
        // them, then the faces, the run's box in rect (paints span it), each field's extent in shape2.
        void FieldText(Painter& p, const FieldPage& page, const std::vector<int>& run, Vec2 pen, float em, fx::Instance m, float weight = 0.0f)
        {
            const float k = em / kFieldEm;
            float width = 0.0f;
            for (int g : run)
                width += page.glyphs[g].advance * k;
            const std::uint32_t feat = m.flags[0];
            std::uint32_t under = 0;
            if ((feat & fx::kShadow) && !(feat & fx::kInnerShadow))
                under |= fx::kShadow;
            if (feat & fx::kGlow)
                under |= fx::kGlow;
            if ((feat & fx::kStroke) && m.strokeParams[1] >= 1.0f)
                under |= fx::kStroke;
            if (feat & fx::kLegible)
                under |= fx::kLegible;
            const std::uint32_t faces = feat & ~under;
            if (under != 0)
                under |= feat & fx::kMotion;   // blurred as the faces are
            for (const std::uint32_t pass : {under, faces})
            {
                if (pass == 0)
                    continue;
                float x = pen.x;
                for (int gi : run)
                {
                    const FieldGlyph& g = page.glyphs[gi];
                    fx::Instance in = m;
                    in.rect[0] = pen.x;
                    in.rect[1] = pen.y - 36.0f * k;
                    in.rect[2] = pen.x + width;
                    in.rect[3] = pen.y;
                    in.shape2[0] = x + (float)g.left * k;
                    in.shape2[1] = pen.y + (float)g.top * k;
                    in.shape2[2] = in.shape2[0] + (float)g.w * k;
                    in.shape2[3] = in.shape2[1] + (float)g.h * k;
                    in.uvRect[0] = (float)g.x / (float)page.width;
                    in.uvRect[1] = 0.0f;
                    in.uvRect[2] = (float)(g.x + g.w) / (float)page.width;
                    in.uvRect[3] = (float)g.h / (float)page.height;
                    in.radii[0] = 8.0f;
                    in.radii[1] = 24.0f;
                    in.shape[2] = 1.0f / k;
                    in.shape[3] = weight;
                    in.flags[0] = pass | fx::kGlyph;
                    in.flags[2] = (std::uint32_t)fx::ShapeKind::Glyph;
                    p.Emit(in, 0, page.page);
                    x += g.advance * k;
                }
            }
        }

        fx::Instance FieldMaterial(Color fill)
        {
            fx::Instance m;
            std::memset(&m, 0, sizeof(m));
            for (int c = 0; c < 4; ++c)
                m.fill0[c] = m.fill1[c] = (&fill.r)[c];
            m.misc[0] = 1.0f;
            m.flags[0] = fx::kFill;
            return m;
        }

        void TextFields(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            p.Rect(Rect(0, 0, 320, 240), Style().Fill(Color::Hex(0x1C1C1E)));
            p.Rect(Rect(0, 150, 320, 240), Style().Fill(Color::Hex(0xF2F2F7)));
            const FieldPage page = MakeFieldPage(f.textures);
            // plain, at three sizes: the same fields
            FieldText(p, page, {0, 1, 2}, Vec2(10, 30), 20, FieldMaterial(Color::White()));
            FieldText(p, page, {0, 1, 2}, Vec2(80, 30), 12, FieldMaterial(kYellow));
            // magnified: corners and curves stay sharp
            FieldText(p, page, {0, 2}, Vec2(130, 110), 80, FieldMaterial(Color::White()));
            // an outline outside and a glow under every face
            fx::Instance glow = FieldMaterial(Color::White());
            glow.flags[0] |= fx::kStroke | fx::kGlow;
            std::memcpy(glow.stroke, &kBlue.r, sizeof(glow.stroke));
            glow.strokeParams[0] = 2.0f;
            glow.strokeParams[1] = 1.0f;
            glow.strokeParams[2] = 1.0f;
            std::memcpy(glow.glow, &kPink.r, sizeof(glow.glow));
            glow.glowParams[0] = 10.0f;
            glow.glowParams[1] = 1.0f;
            FieldText(p, page, {0, 1, 2}, Vec2(10, 80), 28, glow);
            // a gradient across the run, a drop shadow, on the light half
            fx::Instance shade = FieldMaterial(kOrange);
            shade.flags[1] = (std::uint32_t)fx::PaintKind::Linear;
            std::memcpy(shade.fill1, &kPurple.r, sizeof(shade.fill1));
            shade.flags[0] |= fx::kShadow;
            const Color shadow = Color::Black(0.45f);
            std::memcpy(shade.shadow, &shadow.r, sizeof(shade.shadow));
            shade.shadowParams[0] = 4.0f;
            shade.shadowParams[2] = 2.0f;
            shade.shadowParams[3] = 3.0f;
            FieldText(p, page, {0, 1, 2, 0}, Vec2(10, 200), 30, shade);
            // weight: thinner and bolder from the same fields
            FieldText(p, page, {0, 1}, Vec2(200, 190), 24, FieldMaterial(Color::Black()), -0.8f);
            FieldText(p, page, {0, 1}, Vec2(200, 225), 24, FieldMaterial(Color::Black()), 1.2f);
        }

        // Motion blur (fx::kMotion): shapes that move while the frame is shown beside the same shapes at rest - a
        // capsule moving right, a circle on the diagonal, a stroked card down, glyph shapes left with their outline.
        void MotionBlurScene(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            p.Rect(Rect(0, 0, 320, 240), Style().Fill(Color::Hex(0x1C1C1E)));
            const Style capsule = Style().Fill(Color::Hex(0x0A84FF));
            p.Capsule(Rect(16, 14, 116, 42), capsule);
            p.Capsule(Rect(170, 14, 270, 42), Style(capsule).MotionBlur(Vec2(32, 0)));
            const Style dot = Style().Fill(Color::Hex(0xFF9F0A));
            p.Circle(Vec2(52, 86), 22, dot);
            p.Circle(Vec2(206, 86), 22, Style(dot).MotionBlur(Vec2(16, 16)));
            const Style card = Style().Radius(10).Fill(Color::Hex(0x30D158)).Stroke(3, Color::White(), 1.0f);
            p.Rect(Rect(110, 64, 150, 120), card);
            p.Rect(Rect(264, 64, 304, 120), Style(card).MotionBlur(Vec2(0, 20)));
            const FieldPage page = MakeFieldPage(f.textures);
            fx::Instance m = FieldMaterial(Color::White());
            m.flags[0] |= fx::kStroke;
            m.stroke[0] = 0.04f, m.stroke[1] = 0.52f, m.stroke[2] = 1.0f, m.stroke[3] = 1.0f;   // a blue outline outside
            m.strokeParams[0] = 2.0f, m.strokeParams[1] = 1.0f, m.strokeParams[2] = 1.0f, m.strokeParams[3] = 0.0f;
            FieldText(p, page, {0, 1, 2}, Vec2(16, 200), 44, m);
            m.flags[0] |= fx::kMotion;
            m.custom[0] = -28.0f;
            FieldText(p, page, {0, 1, 2}, Vec2(170, 200), 44, m);
        }

        // One UI light (RenderParams::light, a point above the middle): the glass rims facing it catch it, and the
        // cards' drop shadows fall away from it on every side.
        void UiLightScene(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            p.Rect(Rect(0, 0, 320, 240), Style().Fill(Paint::Linear(Color::Hex(0x203050), Color::Hex(0x805030))));
            const Style card = Style().Radius(12).Fill(Color::Hex(0xF2F2F7)).Shadow(Color::Black(0.45f), 8, Vec2(0, 10));
            p.Rect(Rect(20, 20, 90, 80), card);
            p.Rect(Rect(230, 20, 300, 80), card);
            p.Rect(Rect(20, 160, 90, 220), card);
            p.Rect(Rect(230, 160, 300, 220), card);
            GlassMaterial g;
            g.blur = 6.0f;
            g.specular = 1.0f;
            p.Rect(Rect(110, 30, 210, 90), Style().Radius(24).Glass(g));
            p.Rect(Rect(110, 150, 210, 210), Style().Radius(24).Glass(g));
            p.Circle(Vec2(160, 120), 6, Style().Fill(Color::White()));   // where the light is
        }

        // Legible text: white and black runs across a backdrop from black to white - each gets its halo where the
        // backdrop comes close to its own lightness, none where the contrast is ample.
        void TextLegible(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            p.Rect(Rect(0, 0, 320, 240), Style().Fill(Paint::Linear(Color::Black(), Color::White(), 0)));
            p.Rect(Rect(0, 160, 320, 240), Style().Fill(Color::Hex(0x808080)));
            const FieldPage page = MakeFieldPage(f.textures);
            for (int row = 0; row < 2; ++row)
            {
                fx::Instance m = FieldMaterial(row == 0 ? Color::White() : Color::Black());
                m.flags[0] |= fx::kLegible;
                m.radii[2] = 1.0f;
                m.radii[3] = 6.0f;
                FieldText(p, page, {0, 1, 2, 0, 1, 2, 0, 1}, Vec2(12, 60.0f + 70.0f * (float)row), 36, m);
            }
            // half strength on mid gray: both tones too close to it, a fainter halo
            fx::Instance m = FieldMaterial(Color::Hex(0xA0A0A0));
            m.flags[0] |= fx::kLegible;
            m.radii[2] = 0.5f;
            m.radii[3] = 5.0f;
            FieldText(p, page, {2, 1, 0}, Vec2(12, 220), 30, m);
        }

        void EdgeFade(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            const TextureId atlas = MakeAtlas(f.textures);
            p.Rect(Rect(0, 0, 320, 240), Style().Fill(Color::Hex(0x2C2C2E)));
            for (int col = 0; col < 2; ++col)
            {
                const Rect view(10.0f + (float)col * 155.0f, 20, 155.0f + (float)col * 155.0f, 220);
                p.PushClip(view);
                if (col == 1)
                    p.BeginEdgeFade(view, 28, 44);
                for (int i = 0; i < 7; ++i)
                {
                    const float y = view.min.y - 14.0f + (float)i * 34.0f;   // scrolled: rows cut at both edges
                    p.Rect(Rect(view.min.x + 6, y, view.max.x - 6, y + 28), Style().Fill(i % 2 ? kIndigo : kBlue).Radius(8));
                    Glyphs(dl, atlas, Vec2(view.min.x + 14, y + 6), "abcd", 16, Color::White());
                    dl.AddRectFilled(Rect(view.min.x + 6, y + 30, view.max.x - 6, y + 31), Color::White(0.4f).ToRgba8());
                }
                if (col == 1)
                    p.EndEdgeFade();
                p.PopClip();
            }
        }

        void Clipping(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            const TextureId image = MakeImage(f.textures);
            p.Rect(Rect(0, 0, 320, 240), Style().Fill(Color::Hex(0x1C1C1E)));
            // nested clips: the second intersects the first; a non-intersecting one replaces it
            p.PushClip(Rect(10, 10, 150, 110));
            p.Rect(Rect(0, 0, 320, 240), Style().Fill(Paint::Linear(kBlue, kGreen, 0)));
            p.PushClip(Rect(60, 40, 220, 200));
            p.Circle(Vec2(110, 80), 40, Style().Fill(kOrange));
            dl.AddRectFilled(Rect(40, 70, 300, 80), Color::White().ToRgba8());
            p.PopClip();
            p.PushClip(Rect(170, 10, 310, 110), false);
            p.Rect(Rect(160, 0, 320, 120), Style().Fill(kPurple).Radius(30));
            p.PopClip();
            p.PopClip();
            // rounded masks and images: SDF-masked image fill, a uv sub-rect, and plain geometry
            p.PushMask(Rect(10, 124, 110, 224), 26);
            p.Rect(Rect(0, 110, 130, 240), Style().Fill(Paint::Radial(kYellow, kRed)));
            p.Circle(Vec2(10, 124), 30, Style().Fill(kTeal));
            p.PopMask();
            p.Image(image, Rect(122, 124, 222, 224), 20);
            p.Image(image, Rect(234, 124, 284, 174), 8, Color::White(), Vec2(0.25f, 0.25f), Vec2(0.75f, 0.75f));
            dl.AddImage(image, Rect(234, 180, 278, 224), Vec2(0, 0), Vec2(1, 1), Color::White().ToRgba8());
            dl.AddImage(image, Rect(282, 180, 314, 224), Vec2(1, 0), Vec2(0, 1), Color::White(0.7f).ToRgba8());   // mirrored
        }

        // Through the UI core: the background list, two overlapping windows (z-order, clip rects, layout cursor)
        // and the foreground list.
        void Windows(SceneFrame& f)
        {
            f.ui = std::make_unique<Context>();
            Context& ui = *f.ui;
            const TextureId atlas = MakeAtlas(f.textures);
            ui.NewFrame({Vec2(320, 240), Vec2(1, 1), kTime});
            Painter bg(ui.BackgroundDrawList());
            Wallpaper(bg, ui.BackgroundDrawList(), Rect(0, 0, 320, 240));
            const char* names[2] = {"Settings", "Library"};
            const Vec2 pos[2] = {Vec2(16, 16), Vec2(128, 84)};
            for (int w = 0; w < 2; ++w)
            {
                ui.SetNextWindowPos(pos[w], Cond::FirstUse);
                ui.SetNextWindowSize(Vec2(176, 136), Cond::FirstUse);
                ui.Begin(names[w]);
                Window* win = ui.CurrentWindow();
                DrawList& dl = win->GetDrawList();
                Painter p(dl, Env());
                // the window surface with its shadow (drawn outside the window's clip on purpose)
                dl.PushClipRect(Rect(0, 0, 320, 240), false);
                GlassMaterial g = Glass(10, 8, 28);
                g.tint = Color(0.1f, 0.1f, 0.12f, 0.35f);
                p.Rect(win->GetRect(), Style().Radius(22).Glass(g).Shadow(Color::Black(0.35f), 18, Vec2(0, 8)));
                dl.PopClipRect();
                for (int row = 0; row < 5; ++row)
                {
                    const Rect bb = Rect::FromSize(ui.CursorPos(), Vec2(row == 4 ? 220.0f : 150.0f, 22));   // the last row overflows
                    ui.ItemSize(bb.Size());
                    if (ui.ItemAdd(ui.GetId(row), bb))
                    {
                        p.Rect(bb, Style().Fill(row % 2 ? kIndigo : kBlue).Radius(7));
                        Glyphs(dl, atlas, bb.min + Vec2(8, 4), row % 2 ? "abc" : "efgh", 14, Color::White());
                    }
                }
                ui.End();
            }
            Painter fg(ui.ForegroundDrawList());
            fg.Capsule(Rect(270, 12, 308, 30), Style().Fill(kRed));
            ui.EndFrame();
            f.data = ui.GetDrawData();
        }

        void HiDpi(SceneFrame& f)
        {
            // 160 x 120 UI units on a 320 x 240 target: blur radii, glass reach, hairlines and glyphs in pixels
            DrawList& dl = f.NewList();
            PainterEnv env = Env();
            env.pixelScale = 2.0f;
            Painter p(dl, env);
            const TextureId atlas = MakeAtlas(f.textures);
            Wallpaper(p, dl, Rect(0, 0, 160, 120));
            p.Rect(Rect(10, 10, 100, 70), Style().Radius(14).Glass(Glass(6, 6, 14)));
            Glyphs(dl, atlas, Vec2(20, 22), "abcd", 14, Color::White(), 2.0f);
            p.HLine(14, 96, 44.3f, Color::White(0.8f), 0.5f);
            p.HLine(14, 96, 50.0f, Color::White(0.8f), 1.0f);
            const Vec2 pts[4] = {Vec2(110, 100), Vec2(125, 80), Vec2(138, 92), Vec2(152, 70)};
            p.Polyline(pts, 4, 1.0f, Style().Fill(kYellow));
            p.Circle(Vec2(130, 36), 14, Style().Fill(kGreen).Glow(kGreen, 8, 0.8f));
        }

        void LightStreak(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            p.Rect(Rect(0, 0, 320, 240), Style().Fill(Color::Black()));
            const Rect island(40, 40, 280, 200);
            p.Rect(island, Style().Fill(Color::Hex(0x0B0B10)).Radius(48));
            p.PushMask(island, 48);
            p.LightStreak(Rect(56, 110, 264, 170), 1.2f, 3.0f, 1.0f, 0.15f, 0.30f, 0.9f);
            p.PopMask();
            GlassMaterial dome = Glass(0, 18, 60);
            dome.magnify = 0.12f;
            dome.dispersion = 0.8f;
            dome.tint = Color(1, 1, 1, 0.0f);
            p.Rect(Rect(60, 120, 260, 196), Style().Radius(38).Glass(dome));
        }

        // BuildPoisonFrame: loud colors everywhere, glass over the whole target (every pyramid level; the clear one
        // copies level 0), a glow layer over it all, 160 FX instances (more than any scene: the later rows of the
        // instance data stay dirty) and a few hundred vertices.
        void Poison(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            const Rect all(Vec2(0, 0), f.data.displaySize);
            const Color magenta = Color::Hex(0xFF00FF), green = Color::Hex(0x00FF00);
            p.Rect(all, Style().Fill(Paint::Linear(magenta, green, 90)));
            for (int i = 0; i < 160; ++i)
            {
                const float x = std::fmod((float)i * 37.0f, all.Width()), y = std::fmod((float)i * 23.0f, all.Height());
                p.Rect(Rect(x - 6, y - 6, x + 6, y + 6), Style().Fill(i % 2 ? Color::Hex(0xFFFF00) : Color::Hex(0x00FFFF)).Radius(4).Shadow(magenta, 6, Vec2(2, 2)));
            }
            for (int i = 0; i < 48; ++i)
            {
                const float x = std::fmod((float)i * 13.0f, all.Width());
                dl.AddRectFilled(Rect(x, all.min.y, x + 2, all.max.y), green.ToRgba8());
            }
            p.Rect(all, Style().Glass(Glass(24, 24, 60)));
            GlassMaterial clear = Glass(0, 24, 60);
            clear.tint = Color(1, 1, 1, 0.0f);
            p.Rect(all, Style().Glass(clear));
            p.BeginGlowLayer(magenta, 16.0f, 1.0f);
            p.Rect(all, Style().Fill(green.WithAlpha(0.8f)));
            p.EndGlowLayer();
        }

        // Much content at once, at a fractional scale (1.25: the shapes' antialiasing from the derivatives there):
        // items - a frame (a stroke inside the edge), a bar, a dot, a label and a line to the bottom each - overlapping
        // one another, plain shapes and text interleaved (drawn as quads, in few draws); a clip that cuts the last row;
        // strokes centered and outside on continuous corners, a stroked capsule, a stroked segment, a transparent fill
        // under a stroke, and a fill and a stroke fading together (FX instances, between the quads).
        void Dense(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl, Env());
            const TextureId atlas = MakeAtlas(f.textures);
            const Vec2 size = f.data.displaySize;   // 256 x 192 UI units
            p.Rect(Rect(Vec2(0, 0), size), Style().Fill(Color::Hex(0x101014)));
            const Color colors[4] = {kRed, kTeal, kYellow, kGreen};
            for (int i = 0; i < 24; ++i)
            {
                const int row = i / 8, col = i % 8;
                if (row == 2 && col == 0)
                    p.PushClip(Rect(0, 0, size.x, 128.0f));   // cuts the last row's labels and lines
                const float x = 9.0f + (float)col * 29.5f + (float)row * 4.25f, y = 12.0f + (float)row * 42.3f + (float)(i % 3) * 3.7f;
                const float w = 22.0f + (float)(i % 4) * 3.0f, h = 26.0f;
                const Color c = colors[i % 4];
                p.Line(Vec2(size.x * 0.5f, size.y), Vec2(x + w * 0.5f, y + h), 1.0f, Style().Fill(c.WithAlpha(0.45f)));
                p.Rect(Rect(x, y, x + w, y + h), Style().Stroke(1.5f, c));
                p.Rect(Rect(x - 5.0f, y + h * (float)(i % 5) / 4.0f * 0.75f, x - 2.5f, y + h), Style().Fill(kGreen).Radius(1.0f));
                p.Circle(Vec2(x + w * 0.5f, y - 4.5f), 2.5f, Style().Fill(c));
                Glyphs(dl, atlas, Vec2(x, y + h + 2.0f), "abc", 8.0f, Color::White(), f.data.framebufferScale.x);
            }
            p.PopClip();
            p.Rect(Rect(8, 150, 58, 180), Style().Radius(8).Stroke(2, kBlue, 0.5f));
            p.Rect(Rect(68, 150, 118, 180), Style().Fill(kIndigo.WithAlpha(0.6f)).Radius(10).Stroke(2.5f, Color::White(0.8f), 1.0f));
            p.Capsule(Rect(128, 152, 178, 172), Style().Stroke(1.5f, kOrange));
            p.Line(Vec2(190, 158), Vec2(240, 178), 6.0f, Style().Fill(kPurple).Stroke(1.5f, Color::White(), 1.0f));
            p.Rect(Rect(188, 136, 248, 146), Style().Fill(kGray).Radius(3).Stroke(1.5f, kYellow).Opacity(0.6f));
        }

        // A path from `start` heading `angle` (degrees): segments `length` long, turning by `turns` (degrees) between them
        int Turning(Vec2* out, Vec2 start, float angle, float length, const float* turns, int count)
        {
            out[0] = start;
            float a = Radians(angle);
            for (int i = 0; i <= count; ++i)
            {
                out[i + 1] = out[i] + Vec2(std::cos(a), std::sin(a)) * length;
                if (i < count)
                    a += Radians(turns[i]);
            }
            return count + 2;
        }

        // Polylines as quads (PainterEnv::quadShapes: a piece per segment between its joints' lines, a cap per end) or
        // as geometry (polylines_mesh): the same pixels. On a checker, where a pixel blended twice would show: opaque
        // and translucent lines turning by up to 115 degrees, steps on whole and half units (joints' lines through
        // pixel centers), ends on pixel centers, a hairline and a thick line, smooth curves, a line crossing itself, a
        // short segment between turns, and lines left to geometry either way (sharper turns, a segment too short for the
        // turns at its ends).
        void Polylines(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            PainterEnv env = Env();
            env.pixelScale = f.data.framebufferScale.x;
            Painter p(dl, env);
            const Vec2 size = f.data.displaySize;   // 320 x 240 UI units
            p.FillRect(Rect(Vec2(0, 0), size), Color::Hex(0x16161C));
            for (int y = 0; y < (int)size.y; y += 10)
                for (int x = (y / 10 % 2) * 10; x < (int)size.x; x += 20)
                    p.FillRect(Rect((float)x, (float)y, (float)x + 10.0f, (float)y + 10.0f), Color::Hex(0x2A2A33));
            Vec2 pts[16];
            // opaque, turning a quarter each way; turning 110 degrees
            const float quarter[3] = {90, -90, 90}, wide[2] = {110, -110};
            p.Polyline(pts, Turning(pts, Vec2(10, 44), -45, 24, quarter, 3), 4.0f, Style().Fill(kBlue));
            p.Polyline(pts, Turning(pts, Vec2(10, 76), -35, 20, wide, 2), 3.0f, Style().Fill(kTeal));
            // translucent, turning 30 to 115 degrees
            const float mixed[4] = {30, -60, 90, -115};
            p.Polyline(pts, Turning(pts, Vec2(116, 60), -20, 22, mixed, 4), 7.0f, Style().Fill(kOrange.WithAlpha(0.55f)));
            // steps on whole units (the joints' lines through pixel centers), translucent; a square wave on half units
            const Vec2 steps[10] = {Vec2(218, 14), Vec2(236, 14), Vec2(236, 28), Vec2(254, 28), Vec2(254, 42),
                                    Vec2(272, 42), Vec2(272, 56), Vec2(290, 56), Vec2(290, 70), Vec2(308, 70)};
            p.Polyline(steps, 10, 3.0f, Style().Fill(kGreen.WithAlpha(0.6f)));
            const Vec2 wave[8] = {Vec2(218.5f, 86.5f), Vec2(232.5f, 86.5f), Vec2(232.5f, 76.5f), Vec2(246.5f, 76.5f),
                                  Vec2(246.5f, 86.5f), Vec2(260.5f, 86.5f), Vec2(260.5f, 76.5f), Vec2(274.5f, 76.5f)};
            p.Polyline(wave, 8, 2.0f, Style().Fill(kYellow));
            // a hairline (fainter, not thinner) and a pixel-wide line
            const float bends[3] = {45, -90, 100}, zig[3] = {60, -60, 60};
            p.Polyline(pts, Turning(pts, Vec2(10, 130), -30, 26, bends, 3), 0.6f, Style().Fill(Color::White()));
            p.Polyline(pts, Turning(pts, Vec2(10, 160), -30, 22, zig, 3), 1.0f, Style().Fill(kYellow));
            // thick and translucent: ten-sided ends
            const float thick[2] = {40, -70};
            p.Polyline(pts, Turning(pts, Vec2(120, 150), -40, 30, thick, 2), 16.0f, Style().Fill(kPurple.WithAlpha(0.6f)));
            // smooth: a monotone curve (opaque) and a Catmull-Rom one (translucent)
            const Vec2 chart[6] = {Vec2(218, 160), Vec2(238, 120), Vec2(258, 150), Vec2(278, 110), Vec2(298, 140), Vec2(310, 100)};
            p.Polyline(chart, 6, 3.0f, Style().Fill(kPink), PolylineFlags_Smooth);
            const Vec2 curl[5] = {Vec2(220, 100), Vec2(250, 95), Vec2(240, 125), Vec2(280, 120), Vec2(300, 100)};
            p.Polyline(curl, 5, 5.0f, Style().Fill(kGreen.WithAlpha(0.5f)), PolylineFlags_Smooth);
            // crossing itself: blended twice where it crosses, either way
            const float loop[4] = {100, 100, 100, 100};
            p.Polyline(pts, Turning(pts, Vec2(20, 200), -10, 34, loop, 4), 5.0f, Style().Fill(kIndigo.WithAlpha(0.5f)));
            // a short segment between turns either way: its joints' lines do not cross inside it, quads
            const Vec2 zag[4] = {Vec2(70, 234), Vec2(86, 234), Vec2(88, 231), Vec2(102, 220)};
            p.Polyline(zag, 4, 6.0f, Style().Fill(kBlue.WithAlpha(0.7f)));
            // geometry either way: turns sharper than 120 degrees (clipped miters), a segment too short for its turns the
            // same way
            const float sharp[1] = {150}, sharper[1] = {160};
            p.Polyline(pts, Turning(pts, Vec2(118, 190), 0, 30, sharp, 1), 5.0f, Style().Fill(kRed));
            p.Polyline(pts, Turning(pts, Vec2(160, 182), 10, 30, sharper, 1), 6.0f, Style().Fill(Color::White(0.5f)));
            const Vec2 kink[4] = {Vec2(118, 214), Vec2(146, 214), Vec2(148, 217), Vec2(148, 234)};
            p.Polyline(kink, 4, 10.0f, Style().Fill(kOrange));
            // ends on pixel centers: across, down, diagonal, and a corner
            const Vec2 across[2] = {Vec2(220.5f, 184), Vec2(300.5f, 184)}, down[2] = {Vec2(226, 196.5f), Vec2(226, 232.5f)};
            const Vec2 diagonal[2] = {Vec2(240.5f, 196.5f), Vec2(276.5f, 232.5f)};
            const Vec2 corner[3] = {Vec2(288.5f, 196.5f), Vec2(310.5f, 196.5f), Vec2(310.5f, 230.5f)};
            p.Polyline(across, 2, 4.0f, Style().Fill(Color::White(0.7f)));
            p.Polyline(down, 2, 3.0f, Style().Fill(kTeal.WithAlpha(0.6f)));
            p.Polyline(diagonal, 2, 5.0f, Style().Fill(kYellow.WithAlpha(0.6f)));
            p.Polyline(corner, 3, 2.5f, Style().Fill(kPink.WithAlpha(0.6f)));
        }

        std::vector<Scene> MakeScenes()
        {
            std::vector<Scene> s;
            auto add = [&](const char* name, const char* covers, void (*build)(SceneFrame&)) -> Scene& {
                Scene sc;
                sc.name = name;
                sc.covers = covers;
                sc.build = build;
                s.push_back(sc);
                return s.back();
            };
            add("shapes", "SDF rounded rects (circular / continuous / per-corner), capsules, circles, arcs, rings, segments, liquid merge, strokes (inside / centered / outside / faded), opacity",
                &Shapes);
            add("gradients", "linear / radial / conic / conic-loop / spectrum paints, area chart gradient, smooth and mitered polylines (indexed geometry)", &Gradients);
            add("shadows", "drop shadows (blur, offset, spread), inner shadow, outer and inner glow, glow halo containment, shimmer, noise", &Shadows);
            // a value, not a reference into `s`: the scenes added later reallocate it (srgb_target and msaa_target
            // read this after them)
            testkit::Tolerance glassTolerance;
            glassTolerance.channel = 10;
            glassTolerance.fraction = 0.01;
            add("glass", "liquid glass: backdrop captures (copy and direct read), blur pyramid, refraction, dispersion, magnify, legibility, tint, glass on glass",
                &GlassScene)
                .tolerance = glassTolerance;
            Scene& glow = add("glow_layer", "glow layers: region clears, bloom pyramid, tinted / untinted composite, text and polylines in layers", &GlowLayers);
            glow.tolerance.channel = 10;
            glow.tolerance.fraction = 0.01;
            add("text", "text quads from an Alpha8 test atlas (grayscale text program, gamma / contrast composition) on dark and light", &Text);
            Scene& lcd = add("text_lcd", "sub-pixel text pages (dual-source blending) and their grayscale fallback inside glow layers", &TextLcd);
            lcd.needsDualSource = true;
            add("text_fields", "distance-field glyph shapes (FX): plain at three sizes, magnified, an outline and a glow under the faces, a gradient across a run, a drop shadow, weight",
                &TextFields);
            Scene& motion = add("motion_blur", "motion blur (fx::kMotion): a capsule, a circle, a stroked card and outlined glyph shapes moving while the "
                                               "frame is shown, beside the same at rest",
                                &MotionBlurScene);
            motion.tolerance.channel = 10;
            Scene& lit = add("ui_light", "one UI light (RenderParams::light): glass rims facing a point light, drop shadows falling away from it", &UiLightScene);
            lit.tolerance = glassTolerance;
            lit.light = render::UiLight::Point(Vec2(160, 120), 60.0f);
            // HDR output: the glass scene through the float working target, encoded as scRGB with paper white at 80 nits
            // (the target then holds linear sRGB) and as PQ with BT.2020 primaries
            Scene& scrgb = add("hdr_scrgb", "HDR output (RenderParams::hdr): the glass scene drawn in the float working target, encoded as scRGB "
                                             "into an RGBA16_FLOAT target",
                               &GlassScene);
            scrgb.format = rhi::Format::RGBA16_FLOAT;
            scrgb.hdr.encoding = render::HdrEncoding::ScRgb;
            scrgb.hdr.paperWhiteNits = 80.0f;
            scrgb.tolerance = glassTolerance;
            Scene& pq = add("hdr_pq", "HDR output: the glass scene encoded as PQ (SMPTE ST 2084, BT.2020 primaries) into an RGB10A2_UNORM target",
                            &GlassScene);
            pq.format = rhi::Format::RGB10A2_UNORM;
            pq.hdr.encoding = render::HdrEncoding::Pq2020;
            pq.tolerance = glassTolerance;
            Scene& legible = add("text_legible", "legible glyph shapes: a halo of the other tone where the captured backdrop comes close to the text's lightness", &TextLegible);
            legible.tolerance.channel = 10;   // the backdrop through its pyramid (as glass)
            legible.tolerance.fraction = 0.01;
            add("edge_fade", "edge fades (per-draw constants) on SDF shapes, text and geometry inside clip rects", &EdgeFade);
            add("clipping", "nested / replacing clip rects (scissors), rounded masks, image fills with uv sub-rects, textured geometry", &Clipping);
            Scene& win = add("windows", "the UI core end to end: background list, overlapping windows (z-order, clip, layout cursor), foreground list", &Windows);
            win.tolerance.channel = 10;
            win.tolerance.fraction = 0.01;
            Scene& hidpi = add("hidpi", "render scale 2: glass blur and reach in pixels, hairlines on pixel rows, snapped glyphs, polylines", &HiDpi);
            hidpi.scale = 2.0f;
            hidpi.tolerance.channel = 10;
            hidpi.tolerance.fraction = 0.01;
            Scene& streak = add("light_streak", "the Siri light streak (caustic) under a clear dispersive glass dome, rounded mask", &LightStreak);
            streak.tolerance.channel = 12;
            streak.tolerance.fraction = 0.02;
            // its own golden: an sRGB target blends in linear light, so translucent content differs from "glass"
            Scene& srgb = add("srgb_target", "the glass scene into an RGBA8_SRGB target: shaders encode their output, blending in linear light, captures copy raw values",
                              &GlassScene);
            srgb.format = rhi::Format::RGBA8_SRGB;
            srgb.tolerance = glassTolerance;
            srgb.checkCopy = true;
            Scene& msaa = add("msaa_target", "the glass scene into a 4x multisampled target: captures resolve, pipelines match the sample count; a resolve to an offset",
                              &GlassScene);
            msaa.samples = 4;
            msaa.golden = "glass";
            msaa.tolerance = glassTolerance;
            msaa.checkCopy = true;
            // the scenes below share the goldens of those above
            Scene& copy = add("glass_copy", "the glass scene into a target that cannot be sampled: every capture copies, none reads the target; a copy to an offset",
                              &GlassScene);
            copy.sampleable = false;
            copy.golden = "glass";
            copy.tolerance = glassTolerance;
            copy.checkCopy = true;
            Scene& srgbMsaa = add("srgb_msaa", "the glass scene into a 4x multisampled RGBA8_SRGB target: captures resolve sRGB samples into the raw format",
                                  &GlassScene);
            srgbMsaa.format = rhi::Format::RGBA8_SRGB;
            srgbMsaa.samples = 4;
            srgbMsaa.golden = "srgb_target";
            srgbMsaa.tolerance = glassTolerance;
            srgbMsaa.checkCopy = true;
            Scene& callback = add("callback_capture", "a host callback before a backdrop capture (the pass ends and resumes, state bound again); one with an empty clip does not run",
                                  &CallbackCapture);
            callback.golden = "glass";
            callback.tolerance = glassTolerance;
            callback.callbacks = 1;
            Scene& rows = add("fx_rows", "the shapes scene with 2 FX instances per row of the instance texture (FxStorage::Texture: row math across rows and batches)",
                              &Shapes);
            rows.golden = "shapes";
            rows.fxInstancesPerRow = 2;
            Scene& quadRows = add("quad_rows", "the text scene with 16 quads per row of the instance texture (FxStorage::Texture: the quads' rows after the FX "
                                               "instances')",
                                  &Text);
            quadRows.golden = "text";
            quadRows.fxInstancesPerRow = 2;
            Scene& paths = add("paths", "paths (fx::ShapeKind::Path): slanted tabs, polygons, a star, a hexagon, an arrow with concave corners, a glass "
                                        "polyline, area and ring sector - fills, gradients, strokes, shadows, glows and glass on any outline; glass on a ring and "
                                        "a segment",
                               &Paths);
            paths.tolerance = glassTolerance;
            Scene& pathRows = add("path_rows", "the paths scene with 2 FX instances per row of the instance texture (FxStorage::Texture: a path's points in the "
                                               "data instances after it, across rows)",
                                  &Paths);
            pathRows.golden = "paths";
            pathRows.tolerance = glassTolerance;
            pathRows.fxInstancesPerRow = 2;
            Scene& reused = add("glass_reused", "the glass scene three times with RenderParams::targetStatic: the third frame's captures come from the "
                                                "pyramid or from the copies kept aside (temporal reuse), with the same pixels",
                                &GlassScene);
            reused.golden = "glass";
            reused.tolerance = glassTolerance;
            reused.repeat = 3;
            Scene& shapesFx = add("shapes_fx", "the shapes scene with its plain shapes as FX instances (PainterEnv::quadShapes off): the pixels of the quads",
                                  &Shapes);
            shapesFx.golden = "shapes";
            shapesFx.fxShapes = true;
            shapesFx.sameAs = "shapes";
            Scene& dense = add("dense", "much content at once at scale 1.25: overlapping items (frames, bars, dots, labels, lines) - plain shapes as "
                                        "quads among text quads, in few draws; a cut clip; centered and outside strokes on continuous corners, a stroked "
                                        "capsule and segment, a fill and a stroke fading together",
                               &Dense);
            dense.scale = 1.25f;
            Scene& denseFx = add("dense_fx", "the dense scene with its plain shapes as FX instances: the pixels of the quads", &Dense);
            denseFx.scale = 1.25f;
            denseFx.golden = "dense";
            denseFx.fxShapes = true;
            denseFx.sameAs = "dense";
            // where the 24 translucent lines meet at the bottom a pixel is blended a dozen times over: the two ways' last bits
            // add up to 3 levels there on llvmpipe and lavapipe (2 everywhere else)
            denseFx.sameTolerance = {3, 0.0, 3, 0.05};
            add("polylines", "polylines as quads drawn with the text (a piece per segment between its joints' lines, a cap per end) on a checker: "
                             "opaque and translucent lines turning by up to 115 degrees, steps on whole and half units, ends on pixel centers, a "
                             "hairline, a thick line, smooth curves, a line crossing itself, a short segment between turns; lines left to geometry "
                             "(sharper turns, a segment too short for its turns)",
                &Polylines);
            Scene& linesMesh = add("polylines_mesh", "the polylines scene as geometry (PainterEnv::quadShapes off): the pixels of the quads", &Polylines);
            linesMesh.golden = "polylines";
            linesMesh.fxShapes = true;
            linesMesh.sameAs = "polylines";
            Scene& linesScaled = add("polylines_scaled", "the polylines scene at scale 1.5", &Polylines);
            linesScaled.width = 480;
            linesScaled.height = 360;
            linesScaled.scale = 1.5f;
            Scene& linesScaledMesh = add("polylines_scaled_mesh", "the polylines scene at scale 1.5 as geometry: the pixels of the quads", &Polylines);
            linesScaledMesh.width = 480;
            linesScaledMesh.height = 360;
            linesScaledMesh.scale = 1.5f;
            linesScaledMesh.golden = "polylines_scaled";
            linesScaledMesh.fxShapes = true;
            linesScaledMesh.sameAs = "polylines_scaled";
            return s;
        }

        void Build(const Scene& scene, SceneFrame& frame, void (*build)(SceneFrame&))
        {
            frame.data = DrawData();
            frame.data.displaySize = Vec2((float)scene.width / scene.scale, (float)scene.height / scene.scale);
            frame.data.framebufferScale = Vec2(scene.scale, scene.scale);
            gFxShapes = scene.fxShapes;
            build(frame);
            gFxShapes = false;
            frame.data.time = kTime;
            frame.data.deltaTime = 1.0f / 60.0f;
        }
    }

    DrawList& SceneFrame::NewList()
    {
        lists.push_back(std::make_unique<DrawList>());
        DrawList& dl = *lists.back();
        dl.Reset(Rect(Vec2(0, 0), data.displaySize));
        data.lists.push_back(&dl);
        return dl;
    }

    const std::vector<Scene>& Scenes()
    {
        static const std::vector<Scene> scenes = MakeScenes();
        return scenes;
    }

    const Scene* FindScene(const char* name)
    {
        for (const Scene& s : Scenes())
            if (std::strcmp(s.name, name) == 0)
                return &s;
        return nullptr;
    }

    void BuildScene(const Scene& scene, SceneFrame& frame) { Build(scene, frame, scene.build); }

    void BuildPoisonFrame(const Scene& scene, SceneFrame& frame) { Build(scene, frame, &Poison); }

    rhi::HeadlessDesc HeadlessDescOf(const Scene& scene)
    {
        rhi::HeadlessDesc hd;
        hd.width = scene.width;
        hd.height = scene.height;
        hd.format = scene.format;
        hd.sampleable = scene.sampleable;
        hd.samples = scene.samples;
        return hd;
    }

    render::RenderParams RenderParamsOf(const Scene& scene)
    {
        render::RenderParams params;
        params.maxFxInstancesPerRow = scene.fxInstancesPerRow;
        params.targetStatic = scene.repeat > 1;
        params.light = scene.light;
        params.hdr = scene.hdr;
        return params;
    }
}
