// Esia conformance suite - scene definitions (see scenes.hpp). Every scene is deterministic: fixed clock, no
// randomness; what moves (shimmer, light streak) is sampled at the same time on every run.
#include "scenes.hpp"
#include "esia/render/painter.hpp"
#include <cmath>
#include <cstring>

namespace esia::conformance
{
    namespace
    {
        constexpr double kTime = 1.25;

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

        // An Alpha8 page of the eight glyphs.
        TextureId MakeAtlas(TextureRegistry& reg)
        {
            std::vector<std::uint8_t> px((std::size_t)kAtlasW * kAtlasH);
            for (int y = 0; y < kAtlasH; ++y)
                for (int x = 0; x < kAtlasW; ++x)
                {
                    const float lx = (float)(x % kCell) + 0.5f, ly = (float)y + 0.5f;
                    px[(std::size_t)y * kAtlasW + (std::size_t)x] = (std::uint8_t)std::lround(Coverage(x / kCell, lx, ly) * 255.0f);
                }
            TextureInfo info;
            info.format = TextureFormat::Alpha8;
            info.width = kAtlasW;
            info.height = kAtlasH;
            return reg.Create(info, px.data());
        }

        // "Text": one quad per glyph ('a'..'h', space), `h` UI units high, pixel-snapped like a text engine does.
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
                    dl.AddImage(atlas, Rect(sx, sy, sx + w, sy + h), Vec2((float)g / kCells, 0), Vec2((float)(g + 1) / kCells, 1), color.ToRgba8());
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
            Painter p(dl);
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
            Painter p(dl);
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
            Painter p(dl);
            p.Rect(Rect(0, 0, 320, 240), Style().Fill(Color::Hex(0xF2F2F7)));
            p.Rect(Rect(16, 16, 96, 76), Style().Fill(Color::White()).Radius(14).Shadow(Color::Black(0.35f), 16, Vec2(0, 6)));
            p.Rect(Rect(120, 16, 200, 76), Style().Fill(Color::White()).Radius(14).Shadow(kBlue.WithAlpha(0.6f), 6, Vec2(4, 4), 3));
            p.Rect(Rect(224, 16, 304, 76), Style().Fill(Color::Hex(0xE5E5EA)).Radius(14).InnerShadow(Color::Black(0.4f), 10, Vec2(0, 4)));
            p.Rect(Rect(0, 96, 320, 240), Style().Fill(Color::Hex(0x101014)));
            p.Circle(Vec2(56, 150), 18, Style().Fill(kPink).Glow(kPink, 18, 0.9f));
            p.Rect(Rect(120, 132, 200, 168), Style().Fill(Color::Hex(0x1C1C1E)).Radius(12).InnerGlow(14, 0.8f).Glow(kTeal, 4, 0.5f));
            Neighbours n;
            PainterEnv env;
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
            Painter p(dl);
            Wallpaper(p, dl, Rect(0, 0, 320, 240));
            GlassCards(p);
        }

        void CountCallback(const DrawList&, const DrawCmd& cmd, void*) { ++*static_cast<int*>(cmd.userData); }

        // The glass scene with a host callback between the backdrop's content and the first glass: the pass the host
        // code ran in ends for the capture and resumes with everything bound again. Host code is API specific, so the
        // callbacks draw nothing and the image is the glass scene's. A second callback has an empty clip: it must
        // not run (it would draw unclipped).
        void CallbackCapture(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl);
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
            Painter p(dl);
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
            Painter p(dl);
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

        void EdgeFade(SceneFrame& f)
        {
            DrawList& dl = f.NewList();
            Painter p(dl);
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
            Painter p(dl);
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
                Painter p(dl);
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
            PainterEnv env;
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
            Painter p(dl);
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
            Painter p(dl);
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
            return s;
        }

        void Build(const Scene& scene, SceneFrame& frame, void (*build)(SceneFrame&))
        {
            frame.data = DrawData();
            frame.data.displaySize = Vec2((float)scene.width / scene.scale, (float)scene.height / scene.scale);
            frame.data.framebufferScale = Vec2(scene.scale, scene.scale);
            build(frame);
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
        return params;
    }
}
