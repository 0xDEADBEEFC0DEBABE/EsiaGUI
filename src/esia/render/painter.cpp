// Esia - Painter (see esia/render/painter.hpp): encodes SDF shapes as fx::Instances in the draw list's FX stream.
// Ported from WGT's src/render/painter.cpp; the instance encoding must stay identical (the shaders read it).
#include "esia/render/painter.hpp"
#include "esia/text/glyph_atlas.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <optional>

namespace esia
{
    namespace
    {
        inline void Set4(float* d, float a, float b, float c, float e) { d[0] = a; d[1] = b; d[2] = c; d[3] = e; }
        inline void SetColor(float* d, const Color& c) { Set4(d, c.r, c.g, c.b, c.a); }
        inline void SetRect(float* d, const Rect& r) { Set4(d, r.min.x, r.min.y, r.max.x, r.max.y); }

        inline Vec2 ScalePoint(Vec2 p, Vec2 o, float s) { return Vec2(o.x + (p.x - o.x) * s, o.y + (p.y - o.y) * s); }

        // The path Polyline / Area follow: the points (minus repeats), or a smooth curve through them sampled every
        // few pixels. Monotone cubic (Fritsch-Carlson) when x keeps increasing, so a chart never overshoots its
        // data; Catmull-Rom otherwise.
        void BuildPath(const Vec2* pts, int n, std::uint32_t flags, float pixelScale, std::vector<Vec2>& out)
        {
            out.clear();
            for (int i = 0; i < n; ++i)
                if (out.empty() || LengthSq(pts[i] - out.back()) > 1e-8f)
                    out.push_back(pts[i]);
            if (!(flags & PolylineFlags_Smooth) || out.size() < 3)
                return;
            std::vector<Vec2> src;
            src.swap(out);
            const int m = (int)src.size();
            bool monotone = true;
            for (int i = 1; i < m && monotone; ++i)
                monotone = src[(std::size_t)i].x > src[(std::size_t)i - 1].x;
            std::vector<float> tan;
            if (monotone)
            {
                std::vector<float> d((std::size_t)m - 1);
                for (int i = 0; i < m - 1; ++i)
                    d[(std::size_t)i] = (src[(std::size_t)i + 1].y - src[(std::size_t)i].y) / (src[(std::size_t)i + 1].x - src[(std::size_t)i].x);
                tan.resize((std::size_t)m);
                tan[0] = d[0];
                tan[(std::size_t)m - 1] = d[(std::size_t)m - 2];
                for (int i = 1; i < m - 1; ++i)
                    tan[(std::size_t)i] = d[(std::size_t)i - 1] * d[(std::size_t)i] <= 0.0f ? 0.0f : (d[(std::size_t)i - 1] + d[(std::size_t)i]) * 0.5f;
                for (int i = 0; i < m - 1; ++i)
                {
                    const std::size_t k = (std::size_t)i;
                    if (d[k] == 0.0f)
                    {
                        tan[k] = tan[k + 1] = 0.0f;
                        continue;
                    }
                    const float a = tan[k] / d[k], b = tan[k + 1] / d[k];
                    const float h = a * a + b * b;
                    if (h > 9.0f)
                    {
                        const float s = 3.0f / std::sqrt(h);
                        tan[k] = s * a * d[k];
                        tan[k + 1] = s * b * d[k];
                    }
                }
            }
            out.push_back(src[0]);
            for (int i = 0; i < m - 1 && out.size() < 16000; ++i)
            {
                const Vec2 p0 = src[(std::size_t)(i > 0 ? i - 1 : 0)], p1 = src[(std::size_t)i], p2 = src[(std::size_t)i + 1];
                const Vec2 p3 = src[(std::size_t)(i + 2 < m ? i + 2 : m - 1)];
                const float len = Length(p2 - p1) * pixelScale;
                const int steps = (int)Clamp(std::ceil(len / 3.0f), 1.0f, 24.0f);
                for (int k = 1; k <= steps; ++k)
                {
                    const float t = (float)k / (float)steps, t2 = t * t, t3 = t2 * t;
                    if (monotone)
                    {
                        const float h = p2.x - p1.x;
                        const float y = (2 * t3 - 3 * t2 + 1) * p1.y + (t3 - 2 * t2 + t) * h * tan[(std::size_t)i] + (-2 * t3 + 3 * t2) * p2.y +
                                        (t3 - t2) * h * tan[(std::size_t)i + 1];
                        out.push_back(Vec2(p1.x + h * t, y));
                    }
                    else
                    {
                        auto cr = [&](float a, float b, float c, float e) {
                            return 0.5f * (2 * b + (-a + c) * t + (2 * a - 5 * b + 4 * c - e) * t2 + (-a + 3 * b - 3 * c + e) * t3);
                        };
                        out.push_back(Vec2(cr(p0.x, p1.x, p2.x, p3.x), cr(p0.y, p1.y, p2.y, p3.y)));
                    }
                }
            }
        }

        thread_local std::vector<Vec2> tPath;

        // A flat shape's corners as SdRoundRect draws them, per corner: the radius grown by the smoothing (R) and the
        // superellipse's exponent (n), for a box of half size `maxR` at the smallest
        void CornerOf(float r, float maxR, float smoothing, float& R, float& n)
        {
            r = std::min(std::max(r, 0.0f), maxR);
            float s = smoothing;
            R = r * (1.0f + 0.6f * s);
            if (R > maxR)
            {
                s *= Saturate((maxR - r) / std::max(0.6f * r, 1e-4f));
                R = r * (1.0f + 0.6f * s);
            }
            n = s > 0.001f && R > 1e-4f ? 2.0f + 2.0f * s : 2.0f;
        }

        // SdRoundRect's distance at `p` (pixels) from a box [b0, b1] whose corners (tl, tr, br, bl) are R / n
        float BoxDistance(Vec2 p, Vec2 b0, Vec2 b1, const float* R, const float* n)
        {
            const Vec2 c = (b0 + b1) * 0.5f, hs = (b1 - b0) * 0.5f, d = p - c;
            const int k = d.x > 0.0f ? (d.y > 0.0f ? 2 : 1) : (d.y > 0.0f ? 3 : 0);
            const float r = R[k];
            const float qx = std::fabs(d.x) - hs.x + r, qy = std::fabs(d.y) - hs.y + r;
            const float mx = std::max(qx, 0.0f), my = std::max(qy, 0.0f);
            float corner;
            if (mx > 0.0f && my > 0.0f)
                corner = n[k] > 2.0f ? std::pow(std::pow(mx / r, n[k]) + std::pow(my / r, n[k]), 1.0f / n[k]) * r : std::sqrt(mx * mx + my * my);
            else
                corner = std::max(mx, my);
            return std::min(std::max(qx, qy), 0.0f) + corner - r;
        }

        // A tile's key: what it holds, quantized (an eighth of a pixel, the exponent in 1/64)
        std::uint64_t TileKey(const std::uint32_t* v, int count)
        {
            std::uint64_t h = 0x9E3779B97F4A7C15ull;
            for (int i = 0; i < count; ++i)
                h = (h ^ v[i]) * 0x100000001B3ull + (h >> 29);
            return h & ~(1ull << 63);
        }
        // rounding without the C runtime's calls (MSVC does not inline roundf / lroundf / ceilf for SSE2)
        std::int32_t RoundI(float v) { return (std::int32_t)(v + (v >= 0.0f ? 0.5f : -0.5f)); }
        std::uint32_t Q(float v, float steps) { return (std::uint32_t)RoundI(v * steps); }
        int CeilI(float v)
        {
            const int i = (int)v;
            return i + ((float)i < v ? 1 : 0);
        }

        thread_local text::GlyphBitmap tTile;

        // A flat shape's tile and grid, for its size, corners and stroke: rows of widgets repeat a few of them, which
        // then cost a look-up and the vertices
        // (no member initializers: a trivial type keeps the thread-local table zero-initialized without the guarded
        // initialization MSVC checks on every access of one with a constructor)
        struct FlatTile
        {
            std::int32_t in[10];                  // what it is for (pixels, in 1/64: shapes of a size at other places
                                                  // differ in the last bits of their widths)
            const void* text;                     // null: an empty slot
            std::uint64_t generation;             // TextSystem::TileGeneration when made
            TextureId page;
            float offX[4], offY[4];               // grid lines from the shape's start (0, 1) or end (2, 3) when stretched
            float u[4], v[4];
            int nx, ny;
            bool plainMiddle, skipMiddle;
            std::uint32_t indexCount;
            std::uint32_t indices[54];            // the cells' triangles, from the grid's first vertex
        };
        thread_local FlatTile tFlatTiles[128];
        bool SameInputs(const std::int32_t* a, const std::int32_t* b)
        {
            return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3] && a[4] == b[4] && a[5] == b[5] && a[6] == b[6] && a[7] == b[7] &&
                   a[8] == b[8] && a[9] == b[9];
        }
    }

    Painter::Painter(DrawList& drawList, const PainterEnv& env) : dl_(&drawList), env_(env), alpha_(env.alpha) {}

    // ------------------------------------------------------------ shapes
    void Painter::Rect(const esia::Rect& r, const Style& s) { EmitShape(fx::ShapeKind::RoundRect, r, s, nullptr); }

    void Painter::FillRound(const esia::Rect& r, float radius, Color color, float smoothing)
    {
        if (env_.flat && env_.text)
        {
            if (r.Empty())
                return;
            const float radii[4] = {radius, radius, radius, radius};
            Color c = color;
            c.a *= Saturate(alpha_);
            if (FlatShape(r, c, Color::Clear(), 0.0f, 0.0f, radii, smoothing >= 0.0f ? smoothing : env_.cornerSmoothing))
                return;
        }
        Style s;
        s.Radius(radius).Fill(color);
        if (smoothing >= 0.0f)
            s.smoothing = smoothing;
        EmitShape(fx::ShapeKind::RoundRect, r, s, nullptr);
    }

    void Painter::Capsule(const esia::Rect& r, const Style& s)
    {
        EmitShape(fx::ShapeKind::RoundRect, r, s, nullptr, std::min(r.Width(), r.Height()) * 0.5f);
    }

    void Painter::Circle(Vec2 center, float radius, const Style& s)
    {
        EmitShape(fx::ShapeKind::RoundRect, esia::Rect(center.x - radius, center.y - radius, center.x + radius, center.y + radius), s, nullptr,
                  radius, 0.0f);
    }

    void Painter::Arc(Vec2 center, float radius, float thickness, float startRad, float sweepRad, const Style& s)
    {
        if (std::fabs(sweepRad) < 1e-4f)
            return;
        if (sweepRad < 0.0f)
        {
            startRad += sweepRad;
            sweepRad = -sweepRad;
        }
        const float ext = radius + thickness * 0.5f;
        const float extra[4] = {radius, thickness * 0.5f, startRad, std::min(sweepRad, kTau)};
        EmitShape(fx::ShapeKind::Arc, esia::Rect(center.x - ext, center.y - ext, center.x + ext, center.y + ext), s, extra);
    }

    void Painter::Ring(Vec2 center, float radius, float thickness, const Style& s) { Arc(center, radius, thickness, 0.0f, kTau, s); }

    void Painter::Line(Vec2 a, Vec2 b, float thickness, const Style& s)
    {
        const float h = thickness * 0.5f;
        const esia::Rect bounds(std::min(a.x, b.x) - h, std::min(a.y, b.y) - h, std::max(a.x, b.x) + h, std::max(a.y, b.y) + h);
        const float extra[4] = {a.x, a.y, b.x, b.y};
        Style c = s;
        c.radii[0] = h;
        EmitShape(fx::ShapeKind::Segment, bounds, c, extra);
    }

    void Painter::Merge(const esia::Rect& a, const esia::Rect& b, float radius, float smoothness, const Style& s)
    {
        Style c = s;
        c.Radius(radius);
        const float extra[4] = {b.min.x, b.min.y, b.max.x, b.max.y};
        // the merge parameters travel outside Style (they are not a material property)
        mergeRadius_ = radius;
        mergeSmooth_ = smoothness;
        EmitShape(fx::ShapeKind::RoundRect, a, c, extra);
        mergeSmooth_ = -1.0f;
    }

    void Painter::Image(TextureId tex, const esia::Rect& r, float radius, Color tint, Vec2 uv0, Vec2 uv1)
    {
        // Square corners on whole physical pixels, no mask, no scale: a textured quad covers the same pixels as the SDF
        // shape (its edge pixels are fully in or out) without running the FX shader over the area. A full-screen
        // wallpaper is most of a frame's FX cost otherwise (Metal, M3 Pro: 1 ms of 5 at 3024 x 1890).
        const float px = Pixel();
        const auto onGrid = [px](float v) { return std::fabs(v * px - std::round(v * px)) < 1e-3f; };
        if (radius <= 0.0f && maskDepth_ == 0 && scaleDepth_ == 0 && onGrid(r.min.x) && onGrid(r.min.y) && onGrid(r.max.x) && onGrid(r.max.y))
        {
            tint.a *= alpha_;
            if (tint.a > 0.0f && !r.Empty())
                dl_->AddImage(tex, r, uv0, uv1, tint.ToRgba8());
            return;
        }
        Style s;
        s.Radius(radius).Fill(tint).Image(tex, uv0, uv1);
        Rect(r, s);
    }

    void Painter::EmitShape(fx::ShapeKind kind, const esia::Rect& bounds, const Style& s, const float* extra, float radius, float smoothingOverride)
    {
        if (bounds.Empty() && kind == fx::ShapeKind::RoundRect)
            return;
        const float radii[4] = {radius >= 0.0f ? radius : s.radii[0], radius >= 0.0f ? radius : s.radii[1], radius >= 0.0f ? radius : s.radii[2],
                                radius >= 0.0f ? radius : s.radii[3]};
        const float smoothing = smoothingOverride >= 0.0f ? smoothingOverride : s.smoothing >= 0.0f ? s.smoothing : env_.cornerSmoothing;
        // flat drawing: geometry when it can be; else the FX shape without its shadow, glow, glass (its surface instead),
        // shimmer and grain
        const bool flat = env_.flat;
        bool flatGlass = false;   // the glass's surface replaces the fill
        Color flatFill;
        if (flat)
        {
            // glass becomes its surface: flatSurface under the tint, a solid fill over it
            const bool solidPaint = !s.hasFill || s.fill.kind == fx::PaintKind::Solid;
            flatFill = s.hasFill ? s.fill.a : Color::Clear();
            if (s.hasGlass && solidPaint)
            {
                const Color tint = s.glass.tint, c = flatFill;
                const Color base = Lerp(env_.flatSurface, tint.WithAlpha(env_.flatSurface.a), tint.a);
                flatFill = Color(c.r * c.a + base.r * (1.0f - c.a), c.g * c.a + base.g * (1.0f - c.a), c.b * c.a + base.b * (1.0f - c.a),
                                 c.a + base.a * (1.0f - c.a));
                flatGlass = true;
            }
            if (kind == fx::ShapeKind::RoundRect && solidPaint && !(extra && mergeSmooth_ >= 0.0f) && FlatGeometry(bounds, s, flatFill, radii, smoothing))
                return;
        }
        const float scale = env_.metricsScale;

        fx::Instance inst;
        std::memset(&inst, 0, sizeof(inst));
        std::uint32_t feat = 0;

        SetRect(inst.rect, bounds);
        if (kind == fx::ShapeKind::Arc)
        {
            Set4(inst.radii, extra[0], extra[1], 0, 0);
            Set4(inst.shape, 0, 0, extra[2], extra[3]);
        }
        else if (kind == fx::ShapeKind::Segment)
        {
            Set4(inst.radii, radii[0], 0, 0, 0);
            Set4(inst.shape2, extra[0], extra[1], extra[2], extra[3]);
        }
        else
        {
            Set4(inst.radii, radii[0], radii[1], radii[2], radii[3]);
            inst.shape[1] = smoothing;
            if (extra && mergeSmooth_ >= 0.0f)
            {
                feat |= fx::kMerge;
                Set4(inst.shape2, extra[0], extra[1], extra[2], extra[3]);
                Set4(inst.shape2Params, mergeRadius_, std::max(mergeSmooth_, 0.01f), 0, 0);
            }
        }

        // paint
        if (flatGlass)
        {
            if (flatFill.a > 0.0f)
            {
                feat |= fx::kFill;
                SetColor(inst.fill0, flatFill);
                SetColor(inst.fill1, flatFill);
                inst.flags[1] = (std::uint32_t)fx::PaintKind::Solid;
            }
        }
        else if (s.hasFill && (s.fill.a.a > 0.0f || s.fill.b.a > 0.0f || s.image != 0))
        {
            feat |= fx::kFill;
            SetColor(inst.fill0, s.fill.a);
            SetColor(inst.fill1, s.fill.b);
            inst.flags[1] = (std::uint32_t)s.fill.kind;
            switch (s.fill.kind)
            {
            case fx::PaintKind::Linear: Set4(inst.fillParams, s.fill.angle, 0, 0, 0); break;
            case fx::PaintKind::Radial: Set4(inst.fillParams, s.fill.center.x, s.fill.center.y, s.fill.radius, 0); break;
            case fx::PaintKind::Conic: Set4(inst.fillParams, s.fill.center.x, s.fill.center.y, s.fill.angle, s.fill.loop ? 1.0f : 0.0f); break;
            case fx::PaintKind::Spectrum: Set4(inst.fillParams, s.fill.angle, s.fill.center.x, s.fill.center.y, 0); break;
            case fx::PaintKind::Solid: break;
            }
        }
        if (s.image != 0)
        {
            feat |= fx::kImage | fx::kFill;
            Set4(inst.uvRect, s.uv0.x, s.uv0.y, s.uv1.x, s.uv1.y);
        }

        // stroke (glass gets an automatic specular hairline rim)
        float strokeWidth = s.strokeWidth;
        Color strokeColor = s.strokeColor;
        float strokeAlign = s.strokeAlign, fadeTo = s.strokeFadeTo, fadeAngle = s.strokeFadeAngle;
        if (s.hasGlass && !flat && strokeWidth <= 0.0f && s.glass.rim.a > 0.0f)
        {
            strokeWidth = std::max(1.0f, 1.0f * scale);
            strokeColor = s.glass.rim;
            strokeAlign = 0.0f;
            // a light rim is a highlight (strongest facing the light); a dark one is the edge's shade, strongest
            // away from it and never gone (the outline that defines light glass on a light background)
            const bool shade = s.glass.rim.r + s.glass.rim.g + s.glass.rim.b < 1.5f;
            fadeTo = shade ? 0.55f : 0.25f;
            fadeAngle = shade ? s.glass.lightAngle : s.glass.lightAngle + kPi;
        }
        if (strokeWidth > 0.0f && strokeColor.a > 0.0f)
        {
            feat |= fx::kStroke;
            SetColor(inst.stroke, strokeColor);
            Set4(inst.strokeParams, strokeWidth, strokeAlign, fadeTo, fadeAngle);
        }

        // shadow
        if (!flat && s.shadowColor.a > 0.0f && (s.shadowBlur > 0.0f || s.shadowSpread != 0.0f || s.shadowOffset.x != 0.0f || s.shadowOffset.y != 0.0f))
        {
            feat |= fx::kShadow;
            if (s.shadowInset)
                feat |= fx::kInnerShadow;
            SetColor(inst.shadow, s.shadowColor);
            Set4(inst.shadowParams, s.shadowBlur, s.shadowSpread, s.shadowOffset.x, s.shadowOffset.y);
        }

        // glow
        if (!flat && s.glowColor.a > 0.0f)
        {
            SetColor(inst.glow, s.glowColor);
            if (s.glowRadius > 0.0f && s.glowIntensity > 0.0f)
                feat |= fx::kGlow;
            if (s.innerGlowRadius > 0.0f && s.innerGlowIntensity > 0.0f)
                feat |= fx::kInnerGlow;
            Set4(inst.glowParams, s.glowRadius, s.glowIntensity, s.innerGlowRadius, s.innerGlowIntensity);
        }

        // glass
        if (s.hasGlass && !flat)
        {
            feat |= fx::kGlass;
            const GlassMaterial& g = s.glass;
            Set4(inst.glass, g.blur * scale, g.refraction * scale, g.bezel * scale, g.dispersion);
            SetColor(inst.glassTint, g.tint);
            Set4(inst.glassParams, g.saturation, g.brightness, g.specular, g.lightAngle);
            inst.shape[0] = g.legibility;
            inst.shape2Params[2] = std::max(g.magnify, 0.0f);
        }
        const float noise = flat ? 0.0f : s.noise >= 0.0f ? s.noise : (s.hasGlass ? s.glass.noise : 0.0f);
        if (noise > 0.0f)
            feat |= fx::kNoise;

        const float shimmer = flat ? 0.0f : s.shimmer;
        if (shimmer > 0.0f)
            feat |= fx::kShimmer;
        Set4(inst.misc, Saturate(s.opacity * alpha_), noise, shimmer, s.shimmerSpeed);

        if (s.effect != 0)
        {
            feat |= fx::kCustom;
            Set4(inst.custom, s.custom[0], s.custom[1], s.custom[2], s.custom[3]);
        }

        if (maskDepth_ > 0)
        {
            feat |= fx::kMask;
            SetRect(inst.mask, masks_[maskDepth_ - 1]);
            Set4(inst.maskParams, maskRadius_[maskDepth_ - 1], env_.cornerSmoothing, 0, 0);
        }

        if ((feat & (fx::kFill | fx::kStroke | fx::kShadow | fx::kGlow | fx::kInnerGlow | fx::kGlass | fx::kCustom)) == 0)
            return;
        if (inst.misc[0] <= 0.0f)
            return;

        inst.flags[0] = feat;
        inst.flags[2] = (std::uint32_t)kind;
        // layouts reserve glow room from the resting geometry: press / pop scale animations must not re-flow
        esia::Rect restShape(inst.rect[0], inst.rect[1], inst.rect[2], inst.rect[3]);
        if (feat & fx::kMerge)
            restShape = restShape.Union(esia::Rect(inst.shape2[0], inst.shape2[1], inst.shape2[2], inst.shape2[3]));
        const float restGlow = inst.glowParams[0];
        ApplyScale(inst);

        // glows stay off neighbouring items: layouts reserve room for them, and whatever still reaches a
        // neighbour fades out before its edge (halo bounds from the layout layer)
        if ((feat & fx::kGlow) && s.containGlow && env_.glow)
        {
            esia::Rect shape(inst.rect[0], inst.rect[1], inst.rect[2], inst.rect[3]);
            if (feat & fx::kMerge)
                shape = shape.Union(esia::Rect(inst.shape2[0], inst.shape2[1], inst.shape2[2], inst.shape2[3]));
            env_.glow->ReportGlowReach(*dl_, restShape, restGlow * 0.75f);
            esia::Rect bound;
            float fade = 0.0f;
            if (env_.glow->GlowBounds(*dl_, shape, inst.glowParams[0] * 1.8f, bound, fade))
            {
                feat |= fx::kHalo;
                inst.flags[0] = feat;
                SetRect(inst.halo, bound);
                inst.maskParams[2] = fade;
            }
        }

        // coarse CPU culling against the current clip rect
        float pad = 2.0f;
        if (feat & fx::kShadow)
            pad = std::max(pad, inst.shadowParams[0] * 1.6f + std::max(inst.shadowParams[1], 0.0f) + std::max(std::fabs(inst.shadowParams[2]), std::fabs(inst.shadowParams[3])));
        if (feat & fx::kGlow)
            pad = std::max(pad, inst.glowParams[0] * 1.8f);
        esia::Rect ext(inst.rect[0] - pad, inst.rect[1] - pad, inst.rect[2] + pad, inst.rect[3] + pad);
        if (feat & fx::kMerge)
            ext = ext.Union(esia::Rect(inst.shape2[0] - pad, inst.shape2[1] - pad, inst.shape2[2] + pad, inst.shape2[3] + pad));
        const esia::Rect& clip = dl_->ClipRect();
        if (ext.max.x < clip.min.x || ext.max.y < clip.min.y || ext.min.x > clip.max.x || ext.min.y > clip.max.y)
            return;

        Emit(inst, s.effect, s.image);
    }

    bool Painter::FlatGeometry(const esia::Rect& bounds, const Style& s, Color fill, const float* shapeRadii, float smoothing)
    {
        if (s.effect != 0 || s.image != 0 || !env_.text)
            return false;
        const float opacity = Saturate(s.opacity * alpha_);
        Color stroke = s.strokeWidth > 0.0f ? s.strokeColor : Color::Clear();
        fill.a *= opacity;
        stroke.a *= opacity;
        return FlatShape(bounds, fill, stroke, s.strokeWidth, s.strokeAlign, shapeRadii, smoothing);
    }

    bool Painter::FlatShape(const esia::Rect& bounds, Color fill, Color stroke, float strokeWidthUi, float strokeAlign, const float* shapeRadii,
                            float smoothing)
    {
        if (fill.a <= 0.0f && stroke.a <= 0.0f)
            return true;

        // the scale stack (press / pop animations)
        esia::Rect r = bounds;
        float radii[4] = {shapeRadii[0], shapeRadii[1], shapeRadii[2], shapeRadii[3]};
        float strokeWidth = strokeWidthUi;
        for (int i = scaleDepth_ - 1; i >= 0; --i)
        {
            const float k = scaleValue_[i];
            if (k == 1.0f)
                continue;
            r = esia::Rect(ScalePoint(r.min, scaleOrigin_[i], k), ScalePoint(r.max, scaleOrigin_[i], k));
            for (float& v : radii)
                v *= k;
            strokeWidth *= k;
        }
        if (r.Empty())
            return true;
        const float outsetUi = stroke.a > 0.0f ? strokeWidth * strokeAlign : 0.0f;
        const esia::Rect& clip = dl_->ClipRect();
        if (r.max.x + outsetUi + 2.0f <= clip.min.x || r.min.x - outsetUi - 2.0f >= clip.max.x || r.max.y + outsetUi + 2.0f <= clip.min.y ||
            r.min.y - outsetUi - 2.0f >= clip.max.y)
            return true;
        if (maskDepth_ > 0)
        {
            // the innermost mask (the one the FX shader applies) changes nothing when the shape lies inside it, a pixel
            // in from its edges and clear of its corners (a window's rounded content mask around its widgets); else the
            // FX shape
            const esia::Rect& m = masks_[maskDepth_ - 1];
            const float edge = 1.0f / Pixel(), reach = outsetUi + edge;
            const esia::Rect b(r.min.x - reach, r.min.y - reach, r.max.x + reach, r.max.y + reach);
            const float R = maskRadius_[maskDepth_ - 1] * (1.0f + 0.6f * env_.cornerSmoothing) + edge;
            if (b.min.x < m.min.x + edge || b.min.y < m.min.y + edge || b.max.x > m.max.x - edge || b.max.y > m.max.y - edge)
                return false;
            const bool left = b.min.x < m.min.x + R, right = b.max.x > m.max.x - R, top = b.min.y < m.min.y + R, bottom = b.max.y > m.max.y - R;
            if ((left || right) && (top || bottom))
                return false;
        }

        const float px = Pixel(), inv = 1.0f / px;
        const Vec2 p0 = r.min * px, p1 = r.max * px;
        if (tileGeneration_ == 0)
            tileGeneration_ = env_.text->TileGeneration();   // once a Painter (a widget's shapes)
        const std::uint64_t generation = tileGeneration_;

        // the tile of a fill (ring = false) or of a stroke's ring, made or found again
        auto tileFor = [&](bool ring, float width, float align) -> const FlatTile* {
            // all of these are >= 0: rounding is adding a half and truncating
            const float k64 = px * 64.0f;
            std::int32_t in[10] = {(std::int32_t)((p1.x - p0.x) * 64.0f + 0.5f), (std::int32_t)((p1.y - p0.y) * 64.0f + 0.5f),
                                   (std::int32_t)(radii[0] * k64 + 0.5f), (std::int32_t)(radii[1] * k64 + 0.5f), (std::int32_t)(radii[2] * k64 + 0.5f),
                                   (std::int32_t)(radii[3] * k64 + 0.5f), (std::int32_t)(smoothing * 1024.0f + 0.5f),
                                   ring ? (std::int32_t)(width * 64.0f + 0.5f) : -1, (std::int32_t)(align * 64.0f + 0.5f), (std::int32_t)(px * 1024.0f + 0.5f)};
            // A capsule's length does not change its tile: every corner at least half the short side caps them at
            // that half (CornerOf), so the short axis is never stretched and the long one always is when it is 4 px or
            // more longer - one entry for every length (a slider's fill, list rows), -1 in its place
            const std::int32_t minR2 = 2 * std::min(std::min(in[2], in[3]), std::min(in[4], in[5]));
            if (minR2 >= in[1] && in[0] >= in[1] + 4 * 64)
                in[0] = -1;
            else if (minR2 >= in[0] && in[1] >= in[0] + 4 * 64)
                in[1] = -1;
            // the slot from the size and a corner (independent multiplies); everything is compared
            const std::uint32_t hsh = (std::uint32_t)in[0] * 0x9E3779B1u ^ (std::uint32_t)in[1] * 0x85EBCA77u ^ (std::uint32_t)in[2] * 0xC2B2AE3Du ^
                                      (std::uint32_t)in[7] * 0x27D4EB2Fu;
            FlatTile& t = tFlatTiles[(hsh ^ (hsh >> 16)) & 127u];
            if (t.text == env_.text && t.generation == generation && SameInputs(t.in, in))
                return &t;

            // in pixels: the corners as SdRoundRect draws them (a capsule's long axis at its length: the tile is the same
            // at any)
            const float w = in[0] >= 0 ? (float)in[0] * (1.0f / 64.0f) : p1.x - p0.x, h = in[1] >= 0 ? (float)in[1] * (1.0f / 64.0f) : p1.y - p0.y;
            const float maxR = 0.5f * std::min(w, h);
            float R[4], n[4], bigR = 0.0f;
            for (int c = 0; c < 4; ++c)
            {
                CornerOf((float)in[2 + c] * (1.0f / 64.0f), maxR, smoothing, R[c], n[c]);
                R[c] = (float)RoundI(R[c] * 8.0f) * 0.125f;
                n[c] = (float)RoundI(n[c] * 64.0f) * (1.0f / 64.0f);
                bigR = std::max(bigR, R[c]);
            }
            if (bigR > 96.0f)
                return nullptr;   // a tile too large for the atlas to hold many: the FX shape
            const int corner = CeilI(bigR);
            // an axis is stretched when there is a pixel or more between its corners; else its tile is its whole length
            const bool stretchX = w > (float)(2 * corner + 1), stretchY = h > (float)(2 * corner + 1);
            const float wq = stretchX ? 0.0f : (float)RoundI(w * 4.0f) * 0.25f, hq = stretchY ? 0.0f : (float)RoundI(h * 4.0f) * 0.25f;
            // corner cells of C texels and one texel between them that the edges and the middle stretch (a nine-patch)
            // along a stretched axis, else the whole length (a knob, a dot); P texels of nothing around the shape, so
            // bilinear sampling fades its edges out over a pixel wherever it lies
            const float outset = ring ? width * align : 0.0f;
            const int P = 1 + CeilI(outset);
            const int C = P + corner;
            const int tw = stretchX ? 2 * C + 1 : CeilI(wq) + 2 * P, th = stretchY ? 2 * C + 1 : CeilI(hq) + 2 * P;
            std::uint32_t kv[16] = {ring ? 2u : 1u, (std::uint32_t)P, (std::uint32_t)C, Q(wq, 4.0f), Q(hq, 4.0f), Q(width, 8.0f), Q(align, 8.0f),
                                    stretchX ? 1u : 0u, stretchY ? 1u : 0u};
            for (int c = 0; c < 4; ++c)
            {
                kv[9 + c] = Q(R[c], 8.0f);
                kv[13 + (c & 1)] ^= Q(n[c], 64.0f) << (c * 8);
            }
            kv[15] = (std::uint32_t)tw << 16 | (std::uint32_t)th;
            const std::uint64_t key = TileKey(kv, 16);
            const text::GlyphSlot* slot = env_.text->FindTile(key);
            if (!slot)
            {
                // rasterized as the FX shader covers pixels: saturate(0.5 - distance)
                text::GlyphBitmap& b = tTile;
                b.width = tw;
                b.height = th;
                b.left = b.top = 0;
                b.channels = 1;
                b.pixels.assign((std::size_t)tw * th, 0);
                const Vec2 b0((float)P, (float)P);
                const Vec2 b1(stretchX ? (float)(tw - P) : (float)P + wq, stretchY ? (float)(th - P) : (float)P + hq);
                for (int y = 0; y < th; ++y)
                    for (int x = 0; x < tw; ++x)
                    {
                        const float d = BoxDistance(Vec2((float)x + 0.5f, (float)y + 0.5f), b0, b1, R, n);
                        float cov = Saturate(0.5f - d);
                        if (ring)
                            cov = Saturate(0.5f - (d - width * align)) - Saturate(0.5f - (d + width * (1.0f - align)));
                        b.pixels[(std::size_t)y * tw + x] = (std::uint8_t)RoundI(Saturate(cov) * 255.0f);
                    }
                slot = env_.text->AddTile(key, b);
                if (!slot || !slot->page)
                    return nullptr;
            }
            std::memcpy(t.in, in, sizeof(in));
            t.text = env_.text;
            t.generation = tileGeneration_ = env_.text->TileGeneration();   // after AddTile: a new page does not start the atlas over
            t.page = slot->page;
            const Vec2 duv((slot->uv1.x - slot->uv0.x) / (float)tw, (slot->uv1.y - slot->uv0.y) / (float)th);
            t.offX[0] = -(float)P;
            t.u[0] = slot->uv0.x;
            if (stretchX)
            {
                t.offX[1] = (float)(C - P), t.offX[2] = (float)(P - C), t.offX[3] = (float)P;
                t.u[1] = slot->uv0.x + (float)C * duv.x, t.u[2] = slot->uv0.x + (float)(C + 1) * duv.x, t.u[3] = slot->uv1.x;
                t.nx = 4;
            }
            else
                t.offX[1] = (float)(tw - P), t.u[1] = slot->uv1.x, t.nx = 2;
            t.offY[0] = -(float)P;
            t.v[0] = slot->uv0.y;
            if (stretchY)
            {
                t.offY[1] = (float)(C - P), t.offY[2] = (float)(P - C), t.offY[3] = (float)P;
                t.v[1] = slot->uv0.y + (float)C * duv.y, t.v[2] = slot->uv0.y + (float)(C + 1) * duv.y, t.v[3] = slot->uv1.y;
                t.ny = 4;
            }
            else
                t.offY[1] = (float)(th - P), t.v[1] = slot->uv1.y, t.ny = 2;
            // a fill's middle larger than 48 x 48 pixels: a quad of the white texture (the UI geometry program: the
            // cheapest pixels; the coverage program also composes text), the rim around it from the tile
            const bool middle = t.nx == 4 && t.ny == 4;
            t.plainMiddle = middle && !ring && (w - 2.0f * (float)(C - P)) * (h - 2.0f * (float)(C - P)) > 48.0f * 48.0f;
            t.skipMiddle = middle && (ring || t.plainMiddle);
            t.indexCount = 0;
            for (int yi = 0; yi + 1 < t.ny; ++yi)
                for (int xi = 0; xi + 1 < t.nx; ++xi)
                {
                    if (t.skipMiddle && xi == 1 && yi == 1)
                        continue;
                    const int a = yi * t.nx + xi;
                    const int q[6] = {a, a + 1, a + 1 + t.nx, a, a + 1 + t.nx, a + t.nx};
                    for (const int k : q)
                        t.indices[t.indexCount++] = (std::uint32_t)k;
                }
            return &t;
        };

        auto emit = [&](const FlatTile& t, Color color) {
            float gx[4], gy[4];
            // a stretched axis: two lines from each end; a whole one: both from the start
            for (int k = 0; k < t.nx; ++k)
                gx[k] = ((t.nx == 4 && k >= 2 ? p1.x : p0.x) + t.offX[k]) * inv;
            for (int k = 0; k < t.ny; ++k)
                gy[k] = ((t.ny == 4 && k >= 2 ? p1.y : p0.y) + t.offY[k]) * inv;
            const std::uint32_t col = color.ToRgba8();
            if (t.plainMiddle)
            {
                DrawList::PrimWriter pw = dl_->PrimReserve(0, 6, 4);
                const Vec2 wuv(0.5f, 0.5f);
                pw.vtx[0] = {Vec2(gx[1], gy[1]), wuv, col};
                pw.vtx[1] = {Vec2(gx[2], gy[1]), wuv, col};
                pw.vtx[2] = {Vec2(gx[2], gy[2]), wuv, col};
                pw.vtx[3] = {Vec2(gx[1], gy[2]), wuv, col};
                const std::uint32_t b = pw.base;
                pw.idx[0] = b, pw.idx[1] = b + 1, pw.idx[2] = b + 2, pw.idx[3] = b, pw.idx[4] = b + 2, pw.idx[5] = b + 3;
                dl_->PrimCommit(pw, 6, esia::Rect(gx[1], gy[1], gx[2], gy[2]));
            }
            DrawList::PrimWriter pw = dl_->PrimReserve(t.page, t.indexCount, (std::uint32_t)(t.nx * t.ny));
            Vertex* v = pw.vtx;
            for (int yi = 0; yi < t.ny; ++yi)
            {
                const float y = gy[yi], vv = t.v[yi];
                for (int xi = 0; xi < t.nx; ++xi, ++v)
                {
                    v->pos.x = gx[xi];
                    v->pos.y = y;
                    v->uv.x = t.u[xi];
                    v->uv.y = vv;
                    v->color = col;
                }
            }
            const std::uint32_t base = pw.base, n = t.indexCount;
            std::uint32_t* x = pw.idx;
            for (std::uint32_t k = 0; k < n; ++k)
                x[k] = base + t.indices[k];
            dl_->PrimCommit(pw, n, esia::Rect(gx[0], gy[0], gx[t.nx - 1], gy[t.ny - 1]));
        };

        const bool wantFill = fill.a > 0.0f, wantStroke = stroke.a > 0.0f && strokeWidth > 0.0f;
        const FlatTile* ft = wantFill ? tileFor(false, 0.0f, 0.0f) : nullptr;
        if (wantFill && !ft)
            return false;   // nothing drawn yet: the FX shape draws it all
        if (!wantStroke)
        {
            emit(*ft, fill);
            return true;
        }
        std::optional<FlatTile> fillTile;   // a copy: the stroke's tile may take the same slot
        if (ft)
            ft = &fillTile.emplace(*ft);
        const FlatTile* st = tileFor(true, strokeWidth * px, strokeAlign);
        if (!st)
            return false;
        if (ft)
            emit(*ft, fill);
        emit(*st, stroke);
        return true;
    }

    void Painter::ApplyScale(fx::Instance& inst) const
    {
        for (int i = scaleDepth_ - 1; i >= 0; --i)
        {
            const Vec2 o = scaleOrigin_[i];
            const float k = scaleValue_[i];
            if (k == 1.0f)
                continue;
            const Vec2 a = ScalePoint(Vec2(inst.rect[0], inst.rect[1]), o, k), b = ScalePoint(Vec2(inst.rect[2], inst.rect[3]), o, k);
            Set4(inst.rect, a.x, a.y, b.x, b.y);
            const std::uint32_t kind = inst.flags[2];
            if (kind == (std::uint32_t)fx::ShapeKind::Segment || (inst.flags[0] & fx::kMerge))
            {
                const Vec2 c = ScalePoint(Vec2(inst.shape2[0], inst.shape2[1]), o, k), d = ScalePoint(Vec2(inst.shape2[2], inst.shape2[3]), o, k);
                Set4(inst.shape2, c.x, c.y, d.x, d.y);
                inst.shape2Params[0] *= k;
                inst.shape2Params[1] *= k;
            }
            for (float& r : inst.radii)
                r *= k;
            inst.strokeParams[0] *= k;
            inst.shadowParams[0] *= k;
            inst.shadowParams[1] *= k;
            inst.shadowParams[2] *= k;
            inst.shadowParams[3] *= k;
            inst.glowParams[0] *= k;
            inst.glowParams[2] *= k;
            inst.glass[1] *= k;
            inst.glass[2] *= k;
        }
    }

    void Painter::LightStreak(const esia::Rect& r, float intensity, float thickness, float speed, float smile, float open, float time)
    {
        if (r.Empty() || intensity <= 0.0f || alpha_ <= 0.0f)
            return;
        fx::Instance inst;
        std::memset(&inst, 0, sizeof(inst));
        SetRect(inst.rect, r);
        inst.fill0[3] = intensity;
        inst.fill1[0] = time;
        Set4(inst.custom, smile, open, speed, thickness);
        std::uint32_t feat = fx::kCaustic;
        Set4(inst.misc, Saturate(alpha_), 0, 0, 0);
        if (maskDepth_ > 0)
        {
            feat |= fx::kMask;
            SetRect(inst.mask, masks_[maskDepth_ - 1]);
            Set4(inst.maskParams, maskRadius_[maskDepth_ - 1], env_.cornerSmoothing, 0, 0);
        }
        inst.flags[0] = feat;
        inst.flags[2] = (std::uint32_t)fx::ShapeKind::RoundRect;
        ApplyScale(inst);
        Emit(inst);
    }

    void Painter::Emit(const fx::Instance& inst, EffectId effect, TextureId texture) { dl_->AddFx(inst, effect, texture); }

    // -------------------------------------------------------------- text
    float Painter::SnapToPixel(float v) const
    {
        const float rs = Pixel();
        return std::floor(v * rs + 0.5f) / rs;
    }

    Vec2 Painter::MeasureText(text::FontRef font, std::string_view text, float wrapWidth) const
    {
        return env_.text ? env_.text->Measure(font, text, wrapWidth, 0).size : Vec2(0, 0);
    }

    Vec2 Painter::Text(Vec2 pos, text::FontRef font, Color color, std::string_view text, float wrapWidth, std::uint32_t flags)
    {
        if (!env_.text)
            return Vec2(0, 0);
        color.a *= alpha_;
        if (color.a <= 0.0f)
            return env_.text->Measure(font, text, wrapWidth, flags).size;
        // the outline first: the glyphs go over it (one command when they share their atlas page)
        text::TextOutline outline = env_.textOutline;
        outline.color.a *= alpha_;
        const bool outlined = outline.Visible();
        if (scaleDepth_ > 0)
        {
            // under PushScale: drawn at the final (scaled) geometry, rasterized at the scaled size; PopScale skips it
            float k = 1.0f;
            Vec2 at = pos;
            for (int i = scaleDepth_ - 1; i >= 0; --i)
            {
                at = ScalePoint(at, scaleOrigin_[i], scaleValue_[i]);
                k *= scaleValue_[i];
            }
            const std::size_t start = dl_->Vertices().size();
            if (outlined)
                env_.text->DrawOutline(*dl_, font, at, outline, text, wrapWidth, flags, k);
            const Vec2 size = env_.text->Draw(*dl_, font, at, color, text, wrapWidth, flags, k);
            ExcludeFromScale(start);
            return size;
        }
        if (outlined)
            env_.text->DrawOutline(*dl_, font, pos, outline, text, wrapWidth, flags, 1.0f);
        return env_.text->Draw(*dl_, font, pos, color, text, wrapWidth, flags, 1.0f);
    }

    Vec2 Painter::TextBox(const esia::Rect& r, Vec2 align, text::FontRef font, Color color, std::string_view text, std::uint32_t flags)
    {
        if (!env_.text)
            return Vec2(0, 0);
        const bool trim = (flags & text::TextFlags_Ellipsis) != 0;
        const float wrap = trim ? std::max(r.Width(), 1.0f) : 0.0f;
        const text::TextMetrics m = env_.text->Measure(font, text, wrap, flags);
        const Vec2 pos(r.min.x + (r.Width() - m.size.x) * align.x, r.min.y + (r.Height() - m.size.y) * align.y);
        Text(pos, font, color, text, wrap, flags);
        return m.size;
    }

    void Painter::Icon(Vec2 center, text::FontRef font, char32_t icon, Color color)
    {
        color.a *= alpha_;
        if (icon == 0 || color.a <= 0.0f || !env_.text)
            return;
        text::TextOutline outline = env_.textOutline;
        outline.color.a *= alpha_;
        const bool outlined = outline.Visible();
        if (scaleDepth_ > 0)
        {
            float k = 1.0f;
            Vec2 at = center;
            for (int i = scaleDepth_ - 1; i >= 0; --i)
            {
                at = ScalePoint(at, scaleOrigin_[i], scaleValue_[i]);
                k *= scaleValue_[i];
            }
            const std::size_t start = dl_->Vertices().size();
            if (outlined)
            {
                outline.width *= k;
                env_.text->DrawGlyphOutline(*dl_, text::FontRef{font.id, font.size * k}, icon, at, outline);
            }
            env_.text->DrawGlyph(*dl_, text::FontRef{font.id, font.size * k}, icon, at, color);
            ExcludeFromScale(start);
            return;
        }
        if (outlined)
            env_.text->DrawGlyphOutline(*dl_, font, icon, center, outline);
        env_.text->DrawGlyph(*dl_, font, icon, center, color);
    }

    void Painter::ExcludeFromScale(std::size_t vtxStart)
    {
        if (dl_->Vertices().size() > vtxStart && excludedCount_ < kMaxExcluded)
        {
            excludedStart_[excludedCount_] = vtxStart;
            excludedEnd_[excludedCount_] = dl_->Vertices().size();
            ++excludedCount_;
        }
    }

    // ------------------------------------------------------------ cheap primitives
    void Painter::FillRect(const esia::Rect& r, Color c, float rounding)
    {
        if (rounding > 0.0f || maskDepth_ > 0 || scaleDepth_ > 0)
        {
            // rounded: one SDF instance (anti-aliased corners) instead of a tessellated outline; under a mask or a
            // scale too, which only SDF shapes follow (a checkerboard in a capsule kept its square corners)
            Rect(r, Style().Fill(c).Radius(rounding).Smoothing(0.0f));
            return;
        }
        c.a *= alpha_;
        if (c.a > 0.0f)
            dl_->AddRectFilled(r, c.ToRgba8());
    }

    void Painter::HLine(float x0, float x1, float y, Color c, float thickness)
    {
        c.a *= alpha_;
        if (c.a <= 0.0f)
            return;
        // hairlines land exactly on physical pixel rows (crisp at any DPI / render scale)
        const float rs = Pixel();
        const float y0 = std::floor(y * rs) / rs;
        const float h = std::max(1.0f, std::floor(thickness * rs + 0.5f)) / rs;
        dl_->AddRectFilled(esia::Rect(x0, y0, x1, y0 + h), c.ToRgba8());
    }

    void Painter::Polyline(const Vec2* points, int count, float thickness, const Style& s, std::uint32_t flags)
    {
        if (!points || count < 2 || thickness <= 0.0f)
            return;
        Color col = s.hasFill ? s.fill.a : (s.strokeColor.a > 0.0f ? s.strokeColor : Color::White());
        col.a *= Saturate(s.opacity) * alpha_;
        if (col.a <= 0.0f)
            return;
        const float rs = Pixel();
        std::vector<Vec2>& path = tPath;
        BuildPath(points, count, flags, rs, path);
        const int n = (int)path.size();
        if (n < 2)
            return;

        // one glow for the whole line: the stroke goes through a glow layer. The layer blurs the line's light over
        // the glow radius, so a thin line gets proportionally more of it; a glow in the line's own color keeps the
        // line's colors instead of re-tinting them
        const bool glow = s.glowRadius > 0.0f && s.glowIntensity > 0.0f && s.glowColor.a > 0.0f;
        if (glow)
        {
            const float scale = env_.metricsScale;
            const float thin = Clamp(s.glowRadius / std::max(thickness, 1e-3f), 1.0f, 4.0f);
            const float dc = std::fabs(s.glowColor.r - col.r) + std::fabs(s.glowColor.g - col.g) + std::fabs(s.glowColor.b - col.b);
            BeginGlowLayer(Color(s.glowColor.r, s.glowColor.g, s.glowColor.b, dc < 0.05f ? 0.0f : 1.0f), s.glowRadius / std::max(scale, 1e-3f),
                           s.glowIntensity * s.glowColor.a * thin);
        }

        // cross-sections: outer fringe, solid core, solid core, outer fringe (1 physical pixel of anti-aliasing)
        const float aa = 1.0f / rs;
        float hw = thickness * 0.5f;
        if (thickness < aa)
        {
            col.a *= thickness / aa;   // hairline thinner than a pixel: fainter, not thinner
            hw = aa * 0.5f;
        }
        const float inner = std::max(hw - aa * 0.5f, 0.0f), outer = hw + aa * 0.5f;
        const std::uint32_t cIn = col.ToRgba8(), cOut = Color(col.r, col.g, col.b, 0.0f).ToRgba8();
        const Vec2 uv(0, 0);
        const int capSeg = (int)Clamp(std::ceil(hw * rs * 1.2f), 3.0f, 12.0f);
        const std::uint32_t vtxCount = (std::uint32_t)(n * 4 + 2 * (1 + 2 * (capSeg + 1)));
        const std::uint32_t idxCount = (std::uint32_t)((n - 1) * 18 + 2 * (capSeg * 9));
        const std::uint32_t base = dl_->PrimBegin(idxCount, vtxCount);
        auto segNormal = [&](int i) {
            const Vec2 d = path[(std::size_t)i + 1] - path[(std::size_t)i];
            const float l = Length(d);
            return l > 0.0f ? Vec2(-d.y / l, d.x / l) : Vec2(0, 1);
        };
        Vec2 nFirst(0, 1), nLast(0, 1);
        for (int i = 0; i < n; ++i)
        {
            Vec2 m;
            float k = 1.0f;
            if (i == 0)
                m = nFirst = segNormal(0);
            else if (i == n - 1)
                m = nLast = segNormal(n - 2);
            else
            {
                const Vec2 a = segNormal(i - 1), b = segNormal(i);
                m = a + b;
                const float l = Length(m);
                m = l > 1e-4f ? m / l : a;
                k = 1.0f / std::max(Dot(m, b), 0.5f);   // miter, clipped at 2x on sharp turns
            }
            const Vec2 p = path[(std::size_t)i];
            dl_->WriteVertex(p + m * (outer * k), uv, cOut);
            dl_->WriteVertex(p + m * (inner * k), uv, cIn);
            dl_->WriteVertex(p - m * (inner * k), uv, cIn);
            dl_->WriteVertex(p - m * (outer * k), uv, cOut);
            if (i > 0)
            {
                const std::uint32_t a = base + (std::uint32_t)(i - 1) * 4, b = base + (std::uint32_t)i * 4;
                for (std::uint32_t j = 0; j < 3; ++j)
                {
                    dl_->WriteTriangle(a + j, a + j + 1, b + j + 1);
                    dl_->WriteTriangle(a + j, b + j + 1, b + j);
                }
            }
        }
        // round caps: a half disc beyond each end, its rim anti-aliased
        std::uint32_t next = base + (std::uint32_t)n * 4;
        for (int e = 0; e < 2; ++e)
        {
            const Vec2 p = e == 0 ? path[0] : path[(std::size_t)n - 1];
            const Vec2 nm = e == 0 ? nFirst : nLast;
            const Vec2 away = e == 0 ? Vec2(nm.y, -nm.x) : Vec2(-nm.y, nm.x);
            const std::uint32_t c0 = next;
            dl_->WriteVertex(p, uv, cIn);
            for (int k = 0; k <= capSeg; ++k)
            {
                const float a = kPi * (float)k / (float)capSeg;
                const Vec2 dir = nm * std::cos(a) + away * std::sin(a);
                dl_->WriteVertex(p + dir * inner, uv, cIn);
                dl_->WriteVertex(p + dir * outer, uv, cOut);
            }
            for (int k = 0; k < capSeg; ++k)
            {
                const std::uint32_t i0 = c0 + 1 + (std::uint32_t)k * 2, i1 = i0 + 2;
                dl_->WriteTriangle(c0, i0, i1);
                dl_->WriteTriangle(i0, i0 + 1, i1 + 1);
                dl_->WriteTriangle(i0, i1 + 1, i1);
            }
            next = c0 + 1 + 2 * (std::uint32_t)(capSeg + 1);
        }

        if (glow)
            EndGlowLayer();
    }

    void Painter::Area(const Vec2* points, int count, float baseline, const Paint& paint, std::uint32_t flags)
    {
        if (!points || count < 2)
            return;
        std::vector<Vec2>& path = tPath;
        BuildPath(points, count, flags, Pixel(), path);
        const int n = (int)path.size();
        if (n < 2)
            return;
        // linear paint across the area's bounds (solid: both ends the same color)
        float x0 = path[0].x, x1 = x0, y0 = std::min(path[0].y, baseline), y1 = std::max(path[0].y, baseline);
        for (const Vec2& p : path)
        {
            x0 = std::min(x0, p.x);
            x1 = std::max(x1, p.x);
            y0 = std::min(y0, p.y);
            y1 = std::max(y1, p.y);
        }
        const bool linear = paint.kind == fx::PaintKind::Linear;
        const Vec2 dir(std::cos(paint.angle), std::sin(paint.angle));
        float lo = 1e30f, hi = -1e30f;
        const Vec2 corners[4] = {Vec2(x0, y0), Vec2(x1, y0), Vec2(x0, y1), Vec2(x1, y1)};
        for (const Vec2& c : corners)
        {
            const float d = Dot(c, dir);
            lo = std::min(lo, d);
            hi = std::max(hi, d);
        }
        auto colorAt = [&](Vec2 p) {
            Color c = paint.a;
            if (linear)
                c = Lerp(paint.a, paint.b, Saturate((Dot(p, dir) - lo) / std::max(hi - lo, 1e-4f)));
            c.a *= alpha_;
            return c.ToRgba8();
        };
        const std::uint32_t base = dl_->PrimBegin((std::uint32_t)(n - 1) * 6, (std::uint32_t)n * 2);
        for (int i = 0; i < n; ++i)
        {
            const Vec2 top = path[(std::size_t)i], bot(path[(std::size_t)i].x, baseline);
            dl_->WriteVertex(top, Vec2(0, 0), colorAt(top));
            dl_->WriteVertex(bot, Vec2(0, 0), colorAt(bot));
            if (i > 0)
            {
                const std::uint32_t a = base + (std::uint32_t)(i - 1) * 2, b = base + (std::uint32_t)i * 2;
                dl_->WriteTriangle(a, b, b + 1);
                dl_->WriteTriangle(a, b + 1, a + 1);
            }
        }
    }

    void Painter::Sector(Vec2 center, float innerRadius, float outerRadius, float startRad, float sweepRad, Color color, float inset)
    {
        if (sweepRad < 0.0f)
        {
            startRad += sweepRad;
            sweepRad = -sweepRad;
        }
        sweepRad = std::min(sweepRad, kTau);
        innerRadius = std::max(innerRadius, 0.0f);
        color.a *= alpha_;
        if (sweepRad < 1e-4f || outerRadius <= innerRadius || color.a <= 0.0f)
            return;
        // half a pixel in from the edges full color, half a pixel out transparent: the anti-aliasing
        const float h = 0.5f / Pixel();
        const bool ring = sweepRad >= kTau - 1e-4f && inset <= 0.0f;
        // the ends: a line parallel to the radius at `inset` (+ h inside), so each radius has its own angles
        auto ends = [&](float r, float pad, float& a0, float& a1) {
            const float off = r > 1e-4f ? std::asin(std::min((inset + pad) / r, 1.0f)) : 0.0f;
            a0 = startRad + off;
            a1 = startRad + sweepRad - off;
        };
        float ia0, ia1, oa0, oa1, ifa0, ifa1, ofa0, ofa1;   // inner / outer, core / fringe
        const float rIn = innerRadius + h, rOut = outerRadius - h, rInF = std::max(innerRadius - h, 0.0f), rOutF = outerRadius + h;
        ends(rIn, ring ? -inset : h, ia0, ia1);
        ends(rOut, ring ? -inset : h, oa0, oa1);
        ends(rInF, ring ? -inset : -h, ifa0, ifa1);
        ends(rOutF, ring ? -inset : -h, ofa0, ofa1);
        if (oa1 <= oa0)
            return;   // narrower than its gap
        ia1 = std::max(ia1, ia0);
        ifa1 = std::max(ifa1, ifa0);
        // segments: the chord within a quarter pixel of the rim
        const float px = 1.0f / Pixel();
        const float step = 2.0f * std::acos(std::max(1.0f - 0.25f * px / std::max(rOutF, px), -1.0f));
        const int n = std::clamp((int)std::ceil((ofa1 - ofa0) / std::max(step, 1e-3f)), 2, 512);
        const std::uint32_t core = color.ToRgba8(), clear = color.WithAlpha(0.0f).ToRgba8();
        // per step: inner fringe, inner, outer, outer fringe
        const std::uint32_t base = dl_->PrimBegin((std::uint32_t)n * 18 + (ring ? 0u : 12u), (std::uint32_t)(n + 1) * 4);
        for (int i = 0; i <= n; ++i)
        {
            const float t = (float)i / (float)n;
            auto at = [&](float a0, float a1, float r) { const float a = a0 + (a1 - a0) * t; return center + Vec2(std::cos(a), std::sin(a)) * r; };
            dl_->WriteVertex(at(ifa0, ifa1, rInF), Vec2(0, 0), clear);
            dl_->WriteVertex(at(ia0, ia1, rIn), Vec2(0, 0), core);
            dl_->WriteVertex(at(oa0, oa1, rOut), Vec2(0, 0), core);
            dl_->WriteVertex(at(ofa0, ofa1, rOutF), Vec2(0, 0), clear);
            if (i == 0)
                continue;
            const std::uint32_t a = base + (std::uint32_t)(i - 1) * 4, b = a + 4;
            for (std::uint32_t k = 0; k < 3; ++k)   // the inner fringe, the body, the outer fringe
            {
                dl_->WriteTriangle(a + k, b + k, b + k + 1);
                dl_->WriteTriangle(a + k, b + k + 1, a + k + 1);
            }
        }
        if (!ring)
        {
            // the ends' fringes: from the first and last core edges out to the fringe's
            const std::uint32_t first = base, last = base + (std::uint32_t)n * 4;
            for (const std::uint32_t e : {first, last})
            {
                dl_->WriteTriangle(e + 0, e + 1, e + 2);
                dl_->WriteTriangle(e + 0, e + 2, e + 3);
            }
        }
    }

    // ------------------------------------------------------------- state
    void Painter::PushMask(const esia::Rect& r, float radius)
    {
        ESIA_ASSERT(maskDepth_ < 8);
        if (maskDepth_ >= 8)
            return;
        masks_[maskDepth_] = r;
        maskRadius_[maskDepth_] = radius;
        ++maskDepth_;
    }

    void Painter::PopMask()
    {
        ESIA_ASSERT(maskDepth_ > 0);
        if (maskDepth_ > 0)
            --maskDepth_;
    }

    void Painter::PushClip(const esia::Rect& r, bool intersect) { dl_->PushClipRect(r, intersect); }
    void Painter::PopClip() { dl_->PopClipRect(); }

    void Painter::PushScale(Vec2 origin, float scale)
    {
        ESIA_ASSERT(scaleDepth_ < 8);
        if (scaleDepth_ >= 8)
            return;
        scaleOrigin_[scaleDepth_] = origin;
        scaleValue_[scaleDepth_] = scale;
        scaleVtxStart_[scaleDepth_] = dl_->Vertices().size();
        ++scaleDepth_;
    }

    void Painter::PopScale()
    {
        ESIA_ASSERT(scaleDepth_ > 0);
        if (scaleDepth_ <= 0)
            return;
        --scaleDepth_;
        const float k = scaleValue_[scaleDepth_];
        const Vec2 o = scaleOrigin_[scaleDepth_];
        if (k != 1.0f)
        {
            VertexVector& vtx = dl_->Vertices();
            int ex = 0;
            for (std::size_t i = scaleVtxStart_[scaleDepth_]; i < vtx.size(); ++i)
            {
                // text / icons were drawn at their final geometry (re-rasterized): skip their vertices
                while (ex < excludedCount_ && i >= excludedEnd_[ex])
                    ++ex;
                if (ex < excludedCount_ && i >= excludedStart_[ex])
                {
                    i = excludedEnd_[ex] - 1;
                    continue;
                }
                vtx[i].pos = ScalePoint(vtx[i].pos, o, k);
            }
            dl_->RefreshBounds(scaleVtxStart_[scaleDepth_]);
        }
        if (scaleDepth_ == 0)
            excludedCount_ = 0;
    }

    void Painter::BeginGlowLayer(Color tint, float radius, float intensity, float contentOpacity)
    {
        fx::LayerParams p{};
        SetColor(p.color, tint);
        p.intensity = intensity * alpha_;
        p.radius = radius * env_.metricsScale;
        p.opacity = contentOpacity;
        dl_->BeginLayer(p);
    }

    void Painter::EndGlowLayer() { dl_->EndLayer(); }

    void Painter::BeginEdgeFade(const esia::Rect& region, float top, float bottom)
    {
        dl_->BeginFade(fx::FadeParams{region.min.y, region.max.y, std::max(top, 0.0f), std::max(bottom, 0.0f)});
    }

    void Painter::EndEdgeFade() { dl_->EndFade(); }
}
