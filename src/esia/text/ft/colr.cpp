// Esia - COLR color glyphs (colr.hpp).
//
// The paint graph is walked twice: once for the bounds (the union of the outlines it fills, under their transforms),
// then to paint into a premultiplied float canvas of that size. A PaintGlyph narrows the clip - a coverage mask, the
// product of the glyph's coverage and the enclosing clip's - and its child paints through it: every fill is blended
// source-over, scaled by the clip's coverage (what a canvas with an anti-aliased clip path does, Skia's COLR renderer
// among them). A PaintComposite paints its backdrop and its source into canvases of their own, combines them by its
// mode, and blends the result over the target. Gradients are evaluated per pixel center in the paint's own space
// (font units, y up), colors interpolated premultiplied.
#include "colr.hpp"

#include FT_COLOR_H
#include FT_OUTLINE_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H

#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

namespace esia::text::detail
{
    namespace
    {
        constexpr int kMaxDepth = 64;          // nested paints (PaintColrGlyph can recurse)
        constexpr int kMaxPixels = 4096;       // the gray rasterizer's limit too
        constexpr std::uint16_t kForeground = 0xFFFF;   // palette index of the text color

        float Fixed(FT_Fixed v) { return (float)v / 65536.0f; }

        // x' = x xx + y xy + dx, y' = x yx + y yy + dy (FT_Affine23's layout)
        struct Affine
        {
            float xx = 1.0f, xy = 0.0f, dx = 0.0f;
            float yx = 0.0f, yy = 1.0f, dy = 0.0f;

            Vec2 Apply(Vec2 p) const { return Vec2(p.x * xx + p.y * xy + dx, p.x * yx + p.y * yy + dy); }

            // this after b
            Affine operator*(const Affine& b) const
            {
                return {xx * b.xx + xy * b.yx, xx * b.xy + xy * b.yy, xx * b.dx + xy * b.dy + dx,
                        yx * b.xx + yy * b.yx, yx * b.xy + yy * b.yy, yx * b.dx + yy * b.dy + dy};
            }

            bool Invert(Affine& out) const
            {
                const float det = xx * yy - xy * yx;
                if (!(std::fabs(det) > 1e-12f))
                    return false;
                const float k = 1.0f / det;
                out = {yy * k, -xy * k, (xy * dy - yy * dx) * k, -yx * k, xx * k, (yx * dx - xx * dy) * k};
                return true;
            }

            static Affine Translate(float x, float y) { return {1.0f, 0.0f, x, 0.0f, 1.0f, y}; }
            // `m` about the point (cx, cy)
            static Affine About(const Affine& m, float cx, float cy) { return Translate(cx, cy) * m * Translate(-cx, -cy); }
        };

        struct Rgba   // premultiplied
        {
            float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
        };

        struct Stop
        {
            float offset = 0.0f;
            Rgba color;
        };

        // A premultiplied RGBA canvas, w x h.
        struct Canvas
        {
            int w = 0, h = 0;
            std::vector<Rgba> px;
            Canvas(int width, int height) : w(width), h(height), px((std::size_t)width * height) {}
        };

        // The region paints may touch: a rectangle, and inside it a coverage mask (none: full coverage).
        struct Clip
        {
            int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
            std::vector<float> mask;   // (x1 - x0) x (y1 - y0), or empty
            bool Empty() const { return x1 <= x0 || y1 <= y0; }
            float Cov(int x, int y) const { return mask.empty() ? 1.0f : mask[(std::size_t)(y - y0) * (std::size_t)(x1 - x0) + (std::size_t)(x - x0)]; }
        };

        struct OutlineSink
        {
            Outline* out;
            Affine m;
            Vec2 Map(const FT_Vector* v) const { return m.Apply(Vec2((float)v->x, (float)v->y)); }
        };

        int SinkMoveTo(const FT_Vector* to, void* user)
        {
            auto* s = static_cast<OutlineSink*>(user);
            s->out->MoveTo(s->Map(to));
            return 0;
        }
        int SinkLineTo(const FT_Vector* to, void* user)
        {
            auto* s = static_cast<OutlineSink*>(user);
            s->out->LineTo(s->Map(to));
            return 0;
        }
        int SinkConicTo(const FT_Vector* c, const FT_Vector* to, void* user)
        {
            auto* s = static_cast<OutlineSink*>(user);
            s->out->QuadTo(s->Map(c), s->Map(to));
            return 0;
        }
        int SinkCubicTo(const FT_Vector* c1, const FT_Vector* c2, const FT_Vector* to, void* user)
        {
            auto* s = static_cast<OutlineSink*>(user);
            s->out->CubicTo(s->Map(c1), s->Map(c2), s->Map(to));
            return 0;
        }

        float Clamp01(float v) { return std::min(std::max(v, 0.0f), 1.0f); }

        // ---- composite modes (W3C Compositing and Blending), on premultiplied colors
        float Lum(float r, float g, float b) { return 0.3f * r + 0.59f * g + 0.11f * b; }
        float Sat(float r, float g, float b) { return std::max({r, g, b}) - std::min({r, g, b}); }

        void ClipColor(float& r, float& g, float& b)
        {
            const float l = Lum(r, g, b), n = std::min({r, g, b}), x = std::max({r, g, b});
            if (n < 0.0f && l - n > 1e-6f)
            {
                r = l + (r - l) * l / (l - n);
                g = l + (g - l) * l / (l - n);
                b = l + (b - l) * l / (l - n);
            }
            if (x > 1.0f && x - l > 1e-6f)
            {
                r = l + (r - l) * (1.0f - l) / (x - l);
                g = l + (g - l) * (1.0f - l) / (x - l);
                b = l + (b - l) * (1.0f - l) / (x - l);
            }
        }

        void SetLum(float& r, float& g, float& b, float l)
        {
            const float d = l - Lum(r, g, b);
            r += d;
            g += d;
            b += d;
            ClipColor(r, g, b);
        }

        void SetSat(float& r, float& g, float& b, float s)
        {
            float* c[3] = {&r, &g, &b};
            std::sort(c, c + 3, [](const float* p, const float* q) { return *p < *q; });
            if (*c[2] > *c[0])
            {
                *c[1] = (*c[1] - *c[0]) * s / (*c[2] - *c[0]);
                *c[2] = s;
            }
            else
                *c[1] = *c[2] = 0.0f;
            *c[0] = 0.0f;
        }

        float Blend(int mode, float cs, float cb)   // separable modes, straight colors
        {
            switch (mode)
            {
            case FT_COLR_COMPOSITE_SCREEN: return cs + cb - cs * cb;
            case FT_COLR_COMPOSITE_OVERLAY: return Blend(FT_COLR_COMPOSITE_HARD_LIGHT, cb, cs);
            case FT_COLR_COMPOSITE_DARKEN: return std::min(cs, cb);
            case FT_COLR_COMPOSITE_LIGHTEN: return std::max(cs, cb);
            case FT_COLR_COMPOSITE_COLOR_DODGE: return cb <= 0.0f ? 0.0f : cs >= 1.0f ? 1.0f : std::min(1.0f, cb / (1.0f - cs));
            case FT_COLR_COMPOSITE_COLOR_BURN: return cb >= 1.0f ? 1.0f : cs <= 0.0f ? 0.0f : 1.0f - std::min(1.0f, (1.0f - cb) / cs);
            case FT_COLR_COMPOSITE_HARD_LIGHT: return cs <= 0.5f ? cb * 2.0f * cs : Blend(FT_COLR_COMPOSITE_SCREEN, 2.0f * cs - 1.0f, cb);
            case FT_COLR_COMPOSITE_SOFT_LIGHT:
            {
                if (cs <= 0.5f)
                    return cb - (1.0f - 2.0f * cs) * cb * (1.0f - cb);
                const float d = cb <= 0.25f ? ((16.0f * cb - 12.0f) * cb + 4.0f) * cb : std::sqrt(cb);
                return cb + (2.0f * cs - 1.0f) * (d - cb);
            }
            case FT_COLR_COMPOSITE_DIFFERENCE: return std::fabs(cs - cb);
            case FT_COLR_COMPOSITE_EXCLUSION: return cs + cb - 2.0f * cs * cb;
            default: return cs * cb;   // MULTIPLY
            }
        }

        Rgba Composite(int mode, const Rgba& s, const Rgba& d)
        {
            const auto pd = [&](float fs, float fd) {
                return Rgba{s.r * fs + d.r * fd, s.g * fs + d.g * fd, s.b * fs + d.b * fd, s.a * fs + d.a * fd};
            };
            switch (mode)
            {
            case FT_COLR_COMPOSITE_CLEAR: return Rgba{};
            case FT_COLR_COMPOSITE_SRC: return s;
            case FT_COLR_COMPOSITE_DEST: return d;
            case FT_COLR_COMPOSITE_SRC_OVER: return pd(1.0f, 1.0f - s.a);
            case FT_COLR_COMPOSITE_DEST_OVER: return pd(1.0f - d.a, 1.0f);
            case FT_COLR_COMPOSITE_SRC_IN: return pd(d.a, 0.0f);
            case FT_COLR_COMPOSITE_DEST_IN: return pd(0.0f, s.a);
            case FT_COLR_COMPOSITE_SRC_OUT: return pd(1.0f - d.a, 0.0f);
            case FT_COLR_COMPOSITE_DEST_OUT: return pd(0.0f, 1.0f - s.a);
            case FT_COLR_COMPOSITE_SRC_ATOP: return pd(d.a, 1.0f - s.a);
            case FT_COLR_COMPOSITE_DEST_ATOP: return pd(1.0f - d.a, s.a);
            case FT_COLR_COMPOSITE_XOR: return pd(1.0f - d.a, 1.0f - s.a);
            case FT_COLR_COMPOSITE_PLUS:
            {
                const Rgba p = pd(1.0f, 1.0f);
                return Rgba{std::min(p.r, 1.0f), std::min(p.g, 1.0f), std::min(p.b, 1.0f), std::min(p.a, 1.0f)};
            }
            default: break;
            }
            // blend modes: co = (1 - ab) Cs + (1 - as) Cb + as ab B(cs, cb), on straight cs, cb
            const float as = s.a, ab = d.a;
            const float ks = as > 0.0f ? 1.0f / as : 0.0f, kb = ab > 0.0f ? 1.0f / ab : 0.0f;
            const float sr = s.r * ks, sg = s.g * ks, sb = s.b * ks;
            const float br = d.r * kb, bg = d.g * kb, bb = d.b * kb;
            float r, g, b;
            switch (mode)
            {
            case FT_COLR_COMPOSITE_HSL_HUE:
                r = sr, g = sg, b = sb;
                SetSat(r, g, b, Sat(br, bg, bb));
                SetLum(r, g, b, Lum(br, bg, bb));
                break;
            case FT_COLR_COMPOSITE_HSL_SATURATION:
                r = br, g = bg, b = bb;
                SetSat(r, g, b, Sat(sr, sg, sb));
                SetLum(r, g, b, Lum(br, bg, bb));
                break;
            case FT_COLR_COMPOSITE_HSL_COLOR:
                r = sr, g = sg, b = sb;
                SetLum(r, g, b, Lum(br, bg, bb));
                break;
            case FT_COLR_COMPOSITE_HSL_LUMINOSITY:
                r = br, g = bg, b = bb;
                SetLum(r, g, b, Lum(sr, sg, sb));
                break;
            default:
                r = Blend(mode, sr, br);
                g = Blend(mode, sg, bg);
                b = Blend(mode, sb, bb);
                break;
            }
            const float both = as * ab;
            return Rgba{(1.0f - ab) * s.r + (1.0f - as) * d.r + both * r, (1.0f - ab) * s.g + (1.0f - as) * d.g + both * g,
                        (1.0f - ab) * s.b + (1.0f - as) * d.b + both * b, as + ab - both};
        }

        // ---- gradients
        enum class Shape
        {
            Linear,
            Radial,
            Sweep
        };

        struct Gradient
        {
            Shape shape = Shape::Linear;
            FT_PaintExtend extend = FT_COLR_PAINT_EXTEND_PAD;
            std::vector<Stop> stops;   // sorted by offset, at least one
            Vec2 p0, d;                // linear: t = (p - p0) . d / |d|^2
            float invLen2 = 0.0f;
            Vec2 c0, cd;               // radial: circles c0 + t cd, radius r0 + t dr
            float r0 = 0.0f, dr = 0.0f, a = 0.0f;
            float start = 0.0f, invSpan = 0.0f;   // sweep: t = (angle - start) / span, degrees counter-clockwise

            // t at paint-space point p; false where the gradient paints nothing (outside a radial gradient's cone)
            bool T(Vec2 p, float& t) const
            {
                switch (shape)
                {
                case Shape::Linear: t = ((p.x - p0.x) * d.x + (p.y - p0.y) * d.y) * invLen2; return true;
                case Shape::Radial:
                {
                    const Vec2 pd(p.x - c0.x, p.y - c0.y);
                    const float b = pd.x * cd.x + pd.y * cd.y + r0 * dr, c = pd.x * pd.x + pd.y * pd.y - r0 * r0;
                    if (std::fabs(a) < 1e-6f)
                    {
                        if (std::fabs(b) < 1e-9f)
                            return false;
                        t = c / (2.0f * b);
                        return r0 + t * dr >= 0.0f;
                    }
                    const float disc = b * b - a * c;
                    if (disc < 0.0f)
                        return false;
                    const float s = std::sqrt(disc), t1 = (b + s) / a, t2 = (b - s) / a;
                    const float hi = std::max(t1, t2), lo = std::min(t1, t2);
                    if (r0 + hi * dr >= 0.0f)
                        t = hi;
                    else if (r0 + lo * dr >= 0.0f)
                        t = lo;
                    else
                        return false;
                    return true;
                }
                case Shape::Sweep:
                {
                    float angle = std::atan2(p.y - c0.y, p.x - c0.x) * (180.0f / std::numbers::pi_v<float>);
                    if (angle < 0.0f)
                        angle += 360.0f;
                    t = (angle - start) * invSpan;
                    return true;
                }
                }
                return false;
            }

            Rgba At(float t) const
            {
                const float lo = stops.front().offset, hi = stops.back().offset;
                if (hi > lo && extend != FT_COLR_PAINT_EXTEND_PAD)
                {
                    float u = (t - lo) / (hi - lo);
                    if (extend == FT_COLR_PAINT_EXTEND_REPEAT)
                        u -= std::floor(u);
                    else
                    {
                        u = std::fmod(std::fabs(u), 2.0f);
                        if (u > 1.0f)
                            u = 2.0f - u;
                    }
                    t = lo + u * (hi - lo);
                }
                if (t <= lo)
                    return stops.front().color;
                if (t >= hi)
                    return stops.back().color;
                std::size_t i = 1;
                while (i + 1 < stops.size() && stops[i].offset < t)
                    ++i;
                const Stop& s0 = stops[i - 1];
                const Stop& s1 = stops[i];
                const float span = s1.offset - s0.offset;
                const float k = span > 0.0f ? (t - s0.offset) / span : 1.0f;
                return Rgba{s0.color.r + (s1.color.r - s0.color.r) * k, s0.color.g + (s1.color.g - s0.color.g) * k,
                            s0.color.b + (s1.color.b - s0.color.b) * k, s0.color.a + (s1.color.a - s0.color.a) * k};
            }
        };

        class ColrPainter
        {
        public:
            explicit ColrPainter(FT_Face face) : face_(face)
            {
                FT_Color* palette = nullptr;
                FT_Palette_Data data{};
                if (FT_Palette_Data_Get(face, &data) == 0 && FT_Palette_Select(face, 0, &palette) == 0)
                {
                    palette_ = palette;
                    paletteSize_ = data.num_palette_entries;
                }
            }

            // ---- pass 1: the bounds of the outlines the paint fills, in canvas pixels before the canvas offset
            void Measure(const FT_OpaquePaint& op, const Affine& m, int depth)
            {
                FT_COLR_Paint p{};
                if (depth > kMaxDepth || !FT_Get_Paint(face_, op, &p))
                    return;
                switch (p.format)
                {
                case FT_COLR_PAINTFORMAT_COLR_LAYERS:
                {
                    FT_LayerIterator it = p.u.colr_layers.layer_iterator;
                    FT_OpaquePaint layer{};
                    while (FT_Get_Paint_Layers(face_, &it, &layer))
                        Measure(layer, m, depth + 1);
                    break;
                }
                case FT_COLR_PAINTFORMAT_GLYPH: AddBounds(p.u.glyph.glyphID, m); break;
                case FT_COLR_PAINTFORMAT_COLR_GLYPH:
                {
                    FT_OpaquePaint root{};
                    if (FT_Get_Color_Glyph_Paint(face_, p.u.colr_glyph.glyphID, FT_COLOR_NO_ROOT_TRANSFORM, &root))
                        Measure(root, m, depth + 1);
                    break;
                }
                case FT_COLR_PAINTFORMAT_COMPOSITE:
                    Measure(p.u.composite.backdrop_paint, m, depth + 1);
                    Measure(p.u.composite.source_paint, m, depth + 1);
                    break;
                default:
                {
                    FT_OpaquePaint child{};
                    Affine t;
                    if (Transform(p, child, t))
                        Measure(child, m * t, depth + 1);
                    break;   // fills: bounded by the glyphs they are clipped to
                }
                }
            }

            void MeasureLayers(std::uint16_t glyph, const Affine& m)
            {
                FT_LayerIterator it{};
                FT_UInt layerGlyph = 0, color = 0;
                while (FT_Get_Color_Glyph_Layer(face_, glyph, &layerGlyph, &color, &it))
                    AddBounds(layerGlyph, m);
            }

            bool HaveBounds() const { return haveBounds_; }
            Rect Bounds() const { return bounds_; }

            // ---- pass 2: painting
            void Paint(const FT_OpaquePaint& op, const Affine& m, Canvas& dst, const Clip& clip, int depth)
            {
                FT_COLR_Paint p{};
                if (depth > kMaxDepth || clip.Empty() || !FT_Get_Paint(face_, op, &p))
                    return;
                switch (p.format)
                {
                case FT_COLR_PAINTFORMAT_COLR_LAYERS:
                {
                    FT_LayerIterator it = p.u.colr_layers.layer_iterator;
                    FT_OpaquePaint layer{};
                    while (FT_Get_Paint_Layers(face_, &it, &layer))
                        Paint(layer, m, dst, clip, depth + 1);
                    break;
                }
                case FT_COLR_PAINTFORMAT_GLYPH:
                {
                    Clip inner;
                    if (GlyphClip(p.u.glyph.glyphID, m, dst, clip, inner))
                        Paint(p.u.glyph.paint, m, dst, inner, depth + 1);
                    break;
                }
                case FT_COLR_PAINTFORMAT_SOLID: FillSolid(dst, clip, PaletteColor(p.u.solid.color)); break;
                case FT_COLR_PAINTFORMAT_LINEAR_GRADIENT:
                case FT_COLR_PAINTFORMAT_RADIAL_GRADIENT:
                case FT_COLR_PAINTFORMAT_SWEEP_GRADIENT:
                {
                    Gradient g;
                    Affine inverse;
                    if (MakeGradient(p, g) && m.Invert(inverse))
                        FillGradient(dst, clip, g, inverse);
                    break;
                }
                case FT_COLR_PAINTFORMAT_COLR_GLYPH:
                {
                    FT_OpaquePaint root{};
                    if (FT_Get_Color_Glyph_Paint(face_, p.u.colr_glyph.glyphID, FT_COLOR_NO_ROOT_TRANSFORM, &root))
                        Paint(root, m, dst, clip, depth + 1);
                    break;
                }
                case FT_COLR_PAINTFORMAT_COMPOSITE:
                {
                    Canvas backdrop(dst.w, dst.h), source(dst.w, dst.h);
                    Paint(p.u.composite.backdrop_paint, m, backdrop, clip, depth + 1);
                    Paint(p.u.composite.source_paint, m, source, clip, depth + 1);
                    const int mode = (int)p.u.composite.composite_mode;
                    for (int y = clip.y0; y < clip.y1; ++y)
                        for (int x = clip.x0; x < clip.x1; ++x)
                        {
                            const std::size_t i = (std::size_t)y * dst.w + x;
                            Over(dst.px[i], Composite(mode, source.px[i], backdrop.px[i]), 1.0f);
                        }
                    break;
                }
                default:
                {
                    FT_OpaquePaint child{};
                    Affine t;
                    if (Transform(p, child, t))
                        Paint(child, m * t, dst, clip, depth + 1);
                    break;
                }
                }
            }

            // version 0: layers of glyphs, each filled with a palette color
            void PaintLayers(std::uint16_t glyph, const Affine& m, Canvas& dst, const Clip& clip)
            {
                FT_LayerIterator it{};
                FT_UInt layerGlyph = 0, color = 0;
                while (FT_Get_Color_Glyph_Layer(face_, glyph, &layerGlyph, &color, &it))
                {
                    Clip inner;
                    if (GlyphClip(layerGlyph, m, dst, clip, inner))
                        FillSolid(dst, inner, PaletteColor(FT_ColorIndex{(FT_UInt16)color, 1 << 14}));
                }
            }

        private:
            void AddBounds(FT_UInt glyph, const Affine& m)
            {
                if (!LoadOutline(glyph, m))
                    return;
                const Rect b = outline_.Bounds();
                bounds_ = haveBounds_ ? Rect(std::min(bounds_.min.x, b.min.x), std::min(bounds_.min.y, b.min.y), std::max(bounds_.max.x, b.max.x),
                                             std::max(bounds_.max.y, b.max.y))
                                      : b;
                haveBounds_ = true;
            }

            // A transform paint's child and its transform (child space to the paint's space); false for other paints.
            bool Transform(const FT_COLR_Paint& p, FT_OpaquePaint& child, Affine& t) const
            {
                switch (p.format)
                {
                case FT_COLR_PAINTFORMAT_TRANSFORM:
                {
                    const FT_Affine23& a = p.u.transform.affine;
                    child = p.u.transform.paint;
                    t = {Fixed(a.xx), Fixed(a.xy), Fixed(a.dx), Fixed(a.yx), Fixed(a.yy), Fixed(a.dy)};
                    return true;
                }
                case FT_COLR_PAINTFORMAT_TRANSLATE:
                    child = p.u.translate.paint;
                    t = Affine::Translate(Fixed(p.u.translate.dx), Fixed(p.u.translate.dy));
                    return true;
                case FT_COLR_PAINTFORMAT_SCALE:
                    child = p.u.scale.paint;
                    t = Affine::About({Fixed(p.u.scale.scale_x), 0.0f, 0.0f, 0.0f, Fixed(p.u.scale.scale_y), 0.0f}, Fixed(p.u.scale.center_x),
                                      Fixed(p.u.scale.center_y));
                    return true;
                case FT_COLR_PAINTFORMAT_ROTATE:   // counter-clockwise, half turns
                {
                    const float r = Fixed(p.u.rotate.angle) * std::numbers::pi_v<float>, c = std::cos(r), s = std::sin(r);
                    child = p.u.rotate.paint;
                    t = Affine::About({c, -s, 0.0f, s, c, 0.0f}, Fixed(p.u.rotate.center_x), Fixed(p.u.rotate.center_y));
                    return true;
                }
                case FT_COLR_PAINTFORMAT_SKEW:   // the y axis turned counter-clockwise by the x angle, the x axis by the y one
                {
                    const float tx = std::tan(Fixed(p.u.skew.x_skew_angle) * std::numbers::pi_v<float>),
                                ty = std::tan(Fixed(p.u.skew.y_skew_angle) * std::numbers::pi_v<float>);
                    child = p.u.skew.paint;
                    t = Affine::About({1.0f, -tx, 0.0f, ty, 1.0f, 0.0f}, Fixed(p.u.skew.center_x), Fixed(p.u.skew.center_y));
                    return true;
                }
                default: return false;
                }
            }

            // `glyph`'s outline in canvas space (design units under `m`); false when it has none
            bool LoadOutline(FT_UInt glyph, const Affine& m)
            {
                outline_.Clear();
                if (FT_Load_Glyph(face_, glyph, FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP) != 0 || face_->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
                    return false;
                FT_Outline_Funcs funcs{};
                funcs.move_to = &SinkMoveTo;
                funcs.line_to = &SinkLineTo;
                funcs.conic_to = &SinkConicTo;
                funcs.cubic_to = &SinkCubicTo;
                OutlineSink sink{&outline_, m};
                FT_Outline_Decompose(&face_->glyph->outline, &funcs, &sink);
                return outline_.ContourCount() > 0;
            }

            // `outer` narrowed to `glyph`'s outline: the glyph's coverage times the outer clip's
            bool GlyphClip(FT_UInt glyph, const Affine& m, const Canvas& dst, const Clip& outer, Clip& inner)
            {
                if (!LoadOutline(glyph, m) || !RasterizeGray(outline_, 0.0f, coverage_) || coverage_.width == 0)
                    return false;
                const GlyphBitmap& c = coverage_;
                inner.x0 = std::max({outer.x0, c.left, 0});
                inner.y0 = std::max({outer.y0, c.top, 0});
                inner.x1 = std::min({outer.x1, c.left + c.width, dst.w});
                inner.y1 = std::min({outer.y1, c.top + c.height, dst.h});
                if (inner.Empty())
                    return false;
                const int w = inner.x1 - inner.x0;
                inner.mask.assign((std::size_t)w * (inner.y1 - inner.y0), 0.0f);
                for (int y = inner.y0; y < inner.y1; ++y)
                    for (int x = inner.x0; x < inner.x1; ++x)
                        inner.mask[(std::size_t)(y - inner.y0) * w + (x - inner.x0)] =
                            (float)c.pixels[(std::size_t)(y - c.top) * c.width + (x - c.left)] * (1.0f / 255.0f) * outer.Cov(x, y);
                return true;
            }

            Rgba PaletteColor(const FT_ColorIndex& index) const
            {
                float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;   // the text color: black
                if (index.palette_index != kForeground)
                {
                    if (!palette_ || index.palette_index >= paletteSize_)
                        return Rgba{};
                    const FT_Color& c = palette_[index.palette_index];
                    r = c.red / 255.0f, g = c.green / 255.0f, b = c.blue / 255.0f, a = c.alpha / 255.0f;
                }
                a *= Clamp01((float)index.alpha / 16384.0f);
                return Rgba{r * a, g * a, b * a, a};
            }

            bool Stops(FT_ColorLine line, Gradient& g) const
            {
                g.extend = line.extend;
                FT_ColorStopIterator it = line.color_stop_iterator;
                FT_ColorStop stop{};
                while (FT_Get_Colorline_Stops(face_, &stop, &it))
                    g.stops.push_back(Stop{Fixed(stop.stop_offset), PaletteColor(stop.color)});
                std::stable_sort(g.stops.begin(), g.stops.end(), [](const Stop& a, const Stop& b) { return a.offset < b.offset; });
                return !g.stops.empty();
            }

            bool MakeGradient(const FT_COLR_Paint& p, Gradient& g) const
            {
                const auto vec = [](const FT_Vector& v) { return Vec2(Fixed(v.x), Fixed(v.y)); };
                switch (p.format)
                {
                case FT_COLR_PAINTFORMAT_LINEAR_GRADIENT:
                {
                    const FT_PaintLinearGradient& l = p.u.linear_gradient;
                    if (!Stops(l.colorline, g))
                        return false;
                    // the color line runs from p0 to p1 projected onto the perpendicular of p0 p2 through p0
                    const Vec2 p0 = vec(l.p0), p1 = vec(l.p1), p2 = vec(l.p2);
                    const Vec2 v01(p1.x - p0.x, p1.y - p0.y), n(-(p2.y - p0.y), p2.x - p0.x);
                    const float nn = n.x * n.x + n.y * n.y;
                    Vec2 d = v01;
                    if (nn > 1e-9f)
                    {
                        const float k = (v01.x * n.x + v01.y * n.y) / nn;
                        d = Vec2(n.x * k, n.y * k);
                    }
                    const float len2 = d.x * d.x + d.y * d.y;
                    if (!(len2 > 1e-9f))
                        return false;
                    g.shape = Shape::Linear;
                    g.p0 = p0;
                    g.d = d;
                    g.invLen2 = 1.0f / len2;
                    return true;
                }
                case FT_COLR_PAINTFORMAT_RADIAL_GRADIENT:
                {
                    const FT_PaintRadialGradient& r = p.u.radial_gradient;
                    if (!Stops(r.colorline, g))
                        return false;
                    g.shape = Shape::Radial;
                    g.c0 = vec(r.c0);
                    const Vec2 c1 = vec(r.c1);
                    g.cd = Vec2(c1.x - g.c0.x, c1.y - g.c0.y);
                    g.r0 = Fixed(r.r0);
                    g.dr = Fixed(r.r1) - g.r0;
                    g.a = g.cd.x * g.cd.x + g.cd.y * g.cd.y - g.dr * g.dr;
                    return true;
                }
                case FT_COLR_PAINTFORMAT_SWEEP_GRADIENT:
                {
                    const FT_PaintSweepGradient& s = p.u.sweep_gradient;
                    if (!Stops(s.colorline, g))
                        return false;
                    const float start = Fixed(s.start_angle) * 180.0f, end = Fixed(s.end_angle) * 180.0f;
                    if (!(std::fabs(end - start) > 1e-6f))
                        return false;
                    g.shape = Shape::Sweep;
                    g.c0 = vec(s.center);
                    g.start = start;
                    g.invSpan = 1.0f / (end - start);
                    return true;
                }
                default: return false;
                }
            }

            static void Over(Rgba& d, const Rgba& s, float cov)
            {
                const float k = 1.0f - s.a * cov;
                d.r = s.r * cov + d.r * k;
                d.g = s.g * cov + d.g * k;
                d.b = s.b * cov + d.b * k;
                d.a = s.a * cov + d.a * k;
            }

            static void FillSolid(Canvas& dst, const Clip& clip, const Rgba& c)
            {
                if (c.a <= 0.0f)
                    return;
                for (int y = clip.y0; y < clip.y1; ++y)
                    for (int x = clip.x0; x < clip.x1; ++x)
                        if (const float cov = clip.Cov(x, y); cov > 0.0f)
                            Over(dst.px[(std::size_t)y * dst.w + x], c, cov);
            }

            // `inverse`: canvas pixels to the gradient's space
            static void FillGradient(Canvas& dst, const Clip& clip, const Gradient& g, const Affine& inverse)
            {
                for (int y = clip.y0; y < clip.y1; ++y)
                    for (int x = clip.x0; x < clip.x1; ++x)
                    {
                        const float cov = clip.Cov(x, y);
                        float t = 0.0f;
                        if (cov > 0.0f && g.T(inverse.Apply(Vec2((float)x + 0.5f, (float)y + 0.5f)), t))
                            Over(dst.px[(std::size_t)y * dst.w + x], g.At(t), cov);
                    }
            }

            FT_Face face_;
            const FT_Color* palette_ = nullptr;
            unsigned paletteSize_ = 0;
            Outline outline_;
            GlyphBitmap coverage_;
            Rect bounds_;
            bool haveBounds_ = false;
        };

        bool HasPaint(FT_Face face, std::uint16_t glyph, FT_OpaquePaint& root)
        {
            root = FT_OpaquePaint{};
            return FT_Get_Color_Glyph_Paint(face, glyph, FT_COLOR_NO_ROOT_TRANSFORM, &root) != 0;
        }

        bool HasLayers(FT_Face face, std::uint16_t glyph)
        {
            FT_LayerIterator it{};
            FT_UInt layerGlyph = 0, color = 0;
            return FT_Get_Color_Glyph_Layer(face, glyph, &layerGlyph, &color, &it) != 0;
        }
    }

    bool HasColrGlyphs(FT_Face face)
    {
        FT_ULong length = 0;
        FT_Palette_Data data{};
        return face && FT_IS_SFNT(face) && FT_Load_Sfnt_Table(face, TTAG_COLR, 0, nullptr, &length) == 0 && length > 0 &&
               FT_Palette_Data_Get(face, &data) == 0 && data.num_palettes > 0;
    }

    bool IsColrGlyph(FT_Face face, std::uint16_t glyph)
    {
        FT_OpaquePaint root{};
        return HasPaint(face, glyph, root) || HasLayers(face, glyph);
    }

    bool DrawColrGlyph(FT_Face face, std::uint16_t glyph, float emPixels, float offsetX, GlyphBitmap& out, Rect& ink)
    {
        out = GlyphBitmap{};
        out.channels = 4;
        ink = Rect();
        FT_OpaquePaint root{};
        const bool v1 = HasPaint(face, glyph, root);
        if (!v1 && !HasLayers(face, glyph))
            return false;
        const float s = emPixels / (float)std::max<FT_UShort>(face->units_per_EM, 1);
        ColrPainter painter(face);
        // design units (y up) to pixels from the pen (y down), shifted by the pen's phase
        const Affine toPen{s, 0.0f, offsetX, 0.0f, -s, 0.0f};
        if (v1)
            painter.Measure(root, toPen, 0);
        else
            painter.MeasureLayers(glyph, toPen);
        if (!painter.HaveBounds())
            return true;   // no ink
        const Rect b = painter.Bounds();
        const int left = (int)std::floor(b.min.x) - 1, top = (int)std::floor(b.min.y) - 1;
        const int width = (int)std::ceil(b.max.x) + 1 - left, height = (int)std::ceil(b.max.y) + 1 - top;
        if (width > kMaxPixels || height > kMaxPixels || width <= 0 || height <= 0)
            return false;
        Canvas canvas(width, height);
        Clip all;
        all.x1 = width;
        all.y1 = height;
        const Affine toCanvas = Affine::Translate(-(float)left, -(float)top) * toPen;
        if (v1)
            painter.Paint(root, toCanvas, canvas, all, 0);
        else
            painter.PaintLayers(glyph, toCanvas, canvas, all);

        out.left = left;
        out.top = top;
        out.width = width;
        out.height = height;
        out.pixels.resize((std::size_t)width * height * 4);
        for (std::size_t i = 0; i < canvas.px.size(); ++i)
        {
            const Rgba& p = canvas.px[i];
            const float a = Clamp01(p.a), k = a > 0.0f ? 1.0f / a : 0.0f;
            out.pixels[i * 4 + 0] = (std::uint8_t)std::lround(Clamp01(p.r * k) * 255.0f);
            out.pixels[i * 4 + 1] = (std::uint8_t)std::lround(Clamp01(p.g * k) * 255.0f);
            out.pixels[i * 4 + 2] = (std::uint8_t)std::lround(Clamp01(p.b * k) * 255.0f);
            out.pixels[i * 4 + 3] = (std::uint8_t)std::lround(a * 255.0f);
        }
        ink = Rect(b.min.x - offsetX, b.min.y, b.max.x - offsetX, b.max.y);
        return true;
    }
}
