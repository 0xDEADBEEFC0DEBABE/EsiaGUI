// Esia - SVG rendering (svg_internal.hpp): the drawing's operations run row by row - each shape's coverage from its
// sweep, its paint composited premultiplied in sRGB (as browsers composite SVG), layers for group opacity and clips -
// then the pixels straight-alpha for a texture, transparent texels colored like their neighbours.
//
// Row by row: a row of float pixels per layer depth is all the memory a drawing of any size needs, and stays in cache.
#include "svg_internal.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#include <emmintrin.h>
#define ESIA_SVG_SSE2 1
#elif defined(__aarch64__) || defined(_M_ARM64)
#include <arm_neon.h>
#define ESIA_SVG_NEON 1
#endif

namespace esia::svg::detail
{
    namespace
    {
        struct Px
        {
            float r, g, b, a;   // premultiplied
        };

        Px Premul(Color c, float alpha)
        {
            const float a = Saturate(c.a * alpha);
            return {Saturate(c.r) * a, Saturate(c.g) * a, Saturate(c.b) * a, a};
        }

        // A row of premultiplied pixels as straight-alpha RGBA8 (white with the alpha for a mask): each channel divided by
        // the alpha, clamped to 0 .. 1, times 255, plus 1/2, truncated - on SSE2 and NEON a pixel's four channels at once,
        // the same IEEE operations as the plain loop. Opaque pixels (most of a drawing) skip the division; transparent ones
        // stay as they are. Returns how many pixels have ink.
        int Straighten(const Px* src, std::uint8_t* dst, int n, bool mask)
        {
            int ink = 0;
#if defined(ESIA_SVG_SSE2)
            const __m128 zero = _mm_setzero_ps(), one = _mm_set1_ps(1.0f), k255 = _mm_set1_ps(255.0f), half = _mm_set1_ps(0.5f);
            const __m128 rgb = _mm_castsi128_ps(_mm_set_epi32(0, -1, -1, -1));
            for (int x = 0; x < n; ++x, dst += 4)
            {
                const float af = src[x].a;
                if (!(af > 0.0f))
                    continue;   // transparent (a mask's texels are white already)
                __m128 c;
                if (af >= 1.0f)
                    c = mask ? one : _mm_loadu_ps(&src[x].r);   // opaque: premultiplied is straight
                else
                {
                    const __m128 a = _mm_set1_ps(af);
                    c = _mm_or_ps(_mm_and_ps(rgb, mask ? one : _mm_div_ps(_mm_loadu_ps(&src[x].r), a)), _mm_andnot_ps(rgb, a));
                }
                c = _mm_add_ps(_mm_mul_ps(_mm_min_ps(_mm_max_ps(c, zero), one), k255), half);
                const __m128i i = _mm_cvttps_epi32(c);
                const __m128i b16 = _mm_packs_epi32(i, i);
                const int px = _mm_cvtsi128_si32(_mm_packus_epi16(b16, b16));
                if ((px >> 24) == 0)
                    continue;
                std::memcpy(dst, &px, 4);
                ++ink;
            }
#elif defined(ESIA_SVG_NEON)
            const float32x4_t zero = vdupq_n_f32(0.0f), one = vdupq_n_f32(1.0f), k255 = vdupq_n_f32(255.0f), half = vdupq_n_f32(0.5f);
            static const std::uint32_t kRgb[4] = {~0u, ~0u, ~0u, 0u};
            const uint32x4_t rgb = vld1q_u32(kRgb);
            for (int x = 0; x < n; ++x, dst += 4)
            {
                const float af = src[x].a;
                if (!(af > 0.0f))
                    continue;
                float32x4_t c;
                if (af >= 1.0f)
                    c = mask ? one : vld1q_f32(&src[x].r);
                else
                {
                    const float32x4_t a = vdupq_n_f32(af);
                    c = vbslq_f32(rgb, mask ? one : vdivq_f32(vld1q_f32(&src[x].r), a), a);
                }
                c = vaddq_f32(vmulq_f32(vminq_f32(vmaxq_f32(c, zero), one), k255), half);
                const uint16x4_t h = vmovn_u32(vcvtq_u32_f32(c));
                std::uint8_t px[8];
                vst1_u8(px, vmovn_u16(vcombine_u16(h, h)));
                if (px[3] == 0)
                    continue;
                std::memcpy(dst, px, 4);
                ++ink;
            }
#else
            const auto q = [](float v) { return (std::uint8_t)(Saturate(v) * 255.0f + 0.5f); };
            for (int x = 0; x < n; ++x, dst += 4)
            {
                const Px& p = src[x];
                if (!(p.a > 0.0f))
                    continue;
                const bool opaque = p.a >= 1.0f;
                const float a = opaque ? 1.0f : p.a;
                const std::uint8_t a8 = q(a);
                if (a8 == 0)
                    continue;
                if (mask)
                    dst[0] = dst[1] = dst[2] = 255;
                else
                {
                    dst[0] = q(opaque ? p.r : p.r / a);
                    dst[1] = q(opaque ? p.g : p.g / a);
                    dst[2] = q(opaque ? p.b : p.b / a);
                }
                dst[3] = a8;
                ++ink;
            }
#endif
            return ink;
        }

        // source over: d = s k + d (1 - s.a k), the four channels at once where the processor has vectors
        inline void Blend(Px& d, const Px& s, float k)
        {
            const float ia = 1.0f - s.a * k;
#if defined(ESIA_SVG_SSE2)
            _mm_storeu_ps(&d.r, _mm_add_ps(_mm_mul_ps(_mm_loadu_ps(&s.r), _mm_set1_ps(k)), _mm_mul_ps(_mm_loadu_ps(&d.r), _mm_set1_ps(ia))));
#elif defined(ESIA_SVG_NEON)
            vst1q_f32(&d.r, vaddq_f32(vmulq_n_f32(vld1q_f32(&s.r), k), vmulq_n_f32(vld1q_f32(&d.r), ia)));
#else
            d.r = s.r * k + d.r * ia;
            d.g = s.g * k + d.g * ia;
            d.b = s.b * k + d.b * ia;
            d.a = s.a * k + d.a * ia;
#endif
        }

        struct GradientJob
        {
            Affine inv;   // pixel -> gradient units
            Spread spread = Spread::Pad;
            bool radial = false, centered = false;
            // linear: t = (p - p1) . dir
            float x1 = 0.0f, y1 = 0.0f, dx = 0.0f, dy = 0.0f;
            // radial (two-point conical: the focal circle c0 r0 to the end circle c1 r1)
            float c0x = 0.0f, c0y = 0.0f, r0 = 0.0f, cdx = 0.0f, cdy = 0.0f, dr = 0.0f, A = 0.0f, invR = 0.0f;
            std::vector<float> offsets;
            std::vector<float> scale;   // 1 / each segment's span (0: a hard stop)
            std::vector<Px> colors;
            float invA = 0.0f;
            bool opaque = false;   // every pixel it paints opaque: a covered pixel takes its color, nothing blends
        };

        struct PaintJob
        {
            enum class Kind : std::uint8_t { None, Solid, Gradient };
            Kind kind = Kind::None;
            Px solid{0.0f, 0.0f, 0.0f, 0.0f};
            GradientJob grad;
        };

        struct ShapeJob
        {
            Sweep fill, stroke;
            PaintJob fillPaint, strokePaint;
            bool hasFill = false, hasStroke = false, strokeFirst = false, crisp = false;
            int clip = -1;
            int top = 0, bottom = 0, left = 0, right = 0;
        };

        struct ClipJob
        {
            Sweep sweep;
            int parent = -1;
            bool prepared = false;
            int top = 0, bottom = 0, left = 0, right = 0;
            int rowY = -1;
            std::vector<float> row;
        };

        struct LayerJob
        {
            int top = 0, bottom = 0, left = 0, right = 0;   // the pixels it composites
            int zl = 0, zr = 0;                             // the pixels its contents may touch
            bool empty = true;
            float opacity = 1.0f;
            int clip = -1;
        };

        void Union(int& t, int& b, int& l, int& r, bool& empty, int t2, int b2, int l2, int r2)
        {
            if (t2 >= b2 || l2 >= r2)
                return;
            if (empty)
            {
                t = t2, b = b2, l = l2, r = r2;
                empty = false;
                return;
            }
            t = std::min(t, t2), b = std::max(b, b2), l = std::min(l, l2), r = std::max(r, r2);
        }

        float Spreaded(float t, Spread s)
        {
            switch (s)
            {
            case Spread::Pad:
                return Saturate(t);
            case Spread::Repeat:
                return t - std::floor(t);
            case Spread::Reflect:
            {
                const float f = std::fmod(std::fabs(t), 2.0f);
                return f > 1.0f ? 2.0f - f : f;
            }
            }
            return Saturate(t);
        }

        // the stops' color at t, `seg` the segment the last pixel was in (neighbours' t are close: it rarely moves)
        Px StopColor(const GradientJob& g, float t, std::size_t& seg)
        {
            const std::size_t n = g.offsets.size();
            if (t <= g.offsets[0])
                return g.colors[0];
            if (t >= g.offsets[n - 1])
                return g.colors[n - 1];
            while (seg > 0 && t < g.offsets[seg])
                --seg;
            while (seg + 2 < n && t >= g.offsets[seg + 1])
                ++seg;
            const float k = g.scale[seg] > 0.0f ? (t - g.offsets[seg]) * g.scale[seg] : 1.0f;
            const Px& a = g.colors[seg];
            const Px& b = g.colors[seg + 1];
            return {a.r + (b.r - a.r) * k, a.g + (b.g - a.g) * k, a.b + (b.b - a.b) * k, a.a + (b.a - a.a) * k};
        }

        // the gradient's colors at pixels x0 .. x1 of row y
        void Shade(const GradientJob& g, int y, int x0, int x1, Px* out)
        {
            const Vec2 p0 = g.inv.Apply(Vec2((float)x0 + 0.5f, (float)y + 0.5f));
            const float ddx = g.inv.a, ddy = g.inv.b;
            std::size_t seg = 0;
            for (int x = x0; x < x1; ++x)
            {
                const float k = (float)(x - x0);
                const float px = p0.x + ddx * k, py = p0.y + ddy * k;
                float t;
                if (!g.radial)
                    t = (px - g.x1) * g.dx + (py - g.y1) * g.dy;
                else if (g.centered)
                    t = std::sqrt((px - g.c0x) * (px - g.c0x) + (py - g.c0y) * (py - g.c0y)) * g.invR;
                else
                {
                    // the largest t with |p - c(t)| = r(t), r(t) >= 0 (none: the pixel is not painted)
                    const float pdx = px - g.c0x, pdy = py - g.c0y;
                    const float B = pdx * g.cdx + pdy * g.cdy + g.r0 * g.dr;
                    const float C = pdx * pdx + pdy * pdy - g.r0 * g.r0;
                    if (std::fabs(g.A) < 1e-9f)
                    {
                        if (B == 0.0f)
                        {
                            out[x] = {0.0f, 0.0f, 0.0f, 0.0f};
                            continue;
                        }
                        t = C / (2.0f * B);
                        if (g.r0 + t * g.dr < 0.0f)
                        {
                            out[x] = {0.0f, 0.0f, 0.0f, 0.0f};
                            continue;
                        }
                    }
                    else
                    {
                        const float disc = B * B - g.A * C;
                        if (disc < 0.0f)
                        {
                            out[x] = {0.0f, 0.0f, 0.0f, 0.0f};
                            continue;
                        }
                        const float sq = std::sqrt(disc);
                        const float ta = (B + sq) * g.invA, tb = (B - sq) * g.invA;
                        const float hi = std::max(ta, tb), lo = std::min(ta, tb);
                        if (g.r0 + hi * g.dr >= 0.0f)
                            t = hi;
                        else if (g.r0 + lo * g.dr >= 0.0f)
                            t = lo;
                        else
                        {
                            out[x] = {0.0f, 0.0f, 0.0f, 0.0f};
                            continue;
                        }
                    }
                }
                out[x] = StopColor(g, Spreaded(t, g.spread), seg);
            }
        }

        void Composite(Px* row, const std::vector<Cells::Span>& spans, const float* cov, const PaintJob& p, int y, const float* clip, Px* scratch)
        {
            for (const Cells::Span& s : spans)
            {
                if (p.kind == PaintJob::Kind::Solid)
                {
                    const Px c = p.solid;
                    if (s.value >= 0.0f)
                    {
                        if (!clip && s.value >= 1.0f && c.a >= 1.0f)
                            std::fill(row + s.x0, row + s.x1, c);
                        else if (!clip)
                            for (int x = s.x0; x < s.x1; ++x)
                                Blend(row[x], c, s.value);
                        else
                            for (int x = s.x0; x < s.x1; ++x)
                                Blend(row[x], c, s.value * clip[x]);
                    }
                    else if (!clip)
                        for (int x = s.x0; x < s.x1; ++x)
                            Blend(row[x], c, cov[x]);
                    else
                        for (int x = s.x0; x < s.x1; ++x)
                            Blend(row[x], c, cov[x] * clip[x]);
                    continue;
                }
                if (!clip && s.value >= 1.0f && p.grad.opaque)
                {
                    Shade(p.grad, y, s.x0, s.x1, row);   // covered by an opaque gradient: its colors, nothing to blend
                    continue;
                }
                Shade(p.grad, y, s.x0, s.x1, scratch);
                for (int x = s.x0; x < s.x1; ++x)
                {
                    const float k = (s.value >= 0.0f ? s.value : cov[x]) * (clip ? clip[x] : 1.0f);
                    Blend(row[x], scratch[x], k);
                }
            }
        }

        // The part of a closed contour on one side of the line x = v (axis 0) or y = v (axis 1) - the side `keep` points to
        // (1: greater, -1: less) - by Sutherland-Hodgman: what lies beyond becomes a run along the line, so every point
        // on the kept side keeps its winding, and the coverage there is the contour's.
        void ClipSide(const std::vector<Vec2>& in, int axis, float v, float keep, std::vector<Vec2>& out)
        {
            out.clear();
            const std::size_t n = in.size();
            for (std::size_t i = 0; i < n; ++i)
            {
                const Vec2 p = in[i], q = in[i + 1 < n ? i + 1 : 0];
                const float dp = ((axis ? p.y : p.x) - v) * keep, dq = ((axis ? q.y : q.x) - v) * keep;
                if (dp >= 0.0f)
                    out.push_back(p);
                if ((dp >= 0.0f) != (dq >= 0.0f))
                {
                    const float t = dp / (dp - dq);
                    Vec2 c(p.x + (q.x - p.x) * t, p.y + (q.y - p.y) * t);
                    (axis ? c.y : c.x) = v;
                    out.push_back(c);
                }
            }
        }

        class Renderer
        {
        public:
            Renderer(const Tree& tree, int width, int height, const RasterOptions& o) : t_(tree), w_(width), h_(height), o_(o)
            {
                const Rect& vp = o.viewport;
                if (vp.Empty())
                    view_ = ViewTransform(tree, (float)width, (float)height);
                else
                {
                    // fitted into the viewport; what reaches beyond the sides of it that are inside the image is cut off
                    // there (the image's own edges cut the rest)
                    view_ = Affine::Translate(vp.min.x, vp.min.y) * ViewTransform(tree, vp.Width(), vp.Height());
                    box_ = vp;
                    sides_ = (vp.min.x > 0.0f ? 1u : 0u) | (vp.max.x < (float)width ? 2u : 0u) | (vp.min.y > 0.0f ? 4u : 0u) |
                             (vp.max.y < (float)height ? 8u : 0u);
                }
                current_ = o.mask ? Color(1.0f, 1.0f, 1.0f, o.currentColor.a) : o.currentColor;
            }

            void Run(Image& out);

        private:
            PaintJob PreparePaint(const Paint& p, float opacity, const Shape& sh) const;
            void Prepare(std::uint32_t shape);
            void CutToViewport(Contours& c);
            ClipJob& PrepareClip(int index);
            const float* ClipRow(int index, int y);
            void DrawShape(ShapeJob& job, Px* row, int y);

            const Tree& t_;
            int w_, h_;
            const RasterOptions& o_;
            Affine view_;
            Rect box_;                 // the viewport
            std::uint32_t sides_ = 0;  // its sides inside the image: 1 left, 2 right, 4 top, 8 bottom
            Color current_;
            std::vector<ShapeJob> shapes_;
            std::vector<ClipJob> clips_;
            Contours contours_, clipped_;
            std::vector<Vec2> sideA_, sideB_;
            Cells cells_;
            std::vector<Cells::Span> spans_;
            std::vector<float> cov_;
            std::vector<Px> scratch_;
        };

        // `c` (pixels) within the viewport's sides inside the image: contours inside them as they are, those beyond one
        // left out, the rest cut at them
        void Renderer::CutToViewport(Contours& c)
        {
            clipped_.Clear();
            const std::size_t count = c.starts.size();
            for (std::size_t k = 0; k < count; ++k)
            {
                const std::size_t s = c.starts[k], e = k + 1 < count ? c.starts[k + 1] : c.pts.size();
                float x0 = std::numeric_limits<float>::max(), y0 = x0, x1 = -x0, y1 = -x0;
                for (std::size_t i = s; i < e; ++i)
                {
                    const Vec2 p = c.pts[i];
                    x0 = std::min(x0, p.x), y0 = std::min(y0, p.y), x1 = std::max(x1, p.x), y1 = std::max(y1, p.y);
                }
                if (((sides_ & 1) && x1 <= box_.min.x) || ((sides_ & 2) && x0 >= box_.max.x) || ((sides_ & 4) && y1 <= box_.min.y) ||
                    ((sides_ & 8) && y0 >= box_.max.y))
                    continue;
                const bool within = (!(sides_ & 1) || x0 >= box_.min.x) && (!(sides_ & 2) || x1 <= box_.max.x) &&
                                    (!(sides_ & 4) || y0 >= box_.min.y) && (!(sides_ & 8) || y1 <= box_.max.y);
                if (within)
                {
                    clipped_.Begin(c.groups[k]);
                    clipped_.pts.insert(clipped_.pts.end(), c.pts.begin() + (std::ptrdiff_t)s, c.pts.begin() + (std::ptrdiff_t)e);
                    continue;
                }
                sideA_.assign(c.pts.begin() + (std::ptrdiff_t)s, c.pts.begin() + (std::ptrdiff_t)e);
                const struct
                {
                    std::uint32_t side;
                    int axis;
                    float v, keep;
                } cuts[4] = {{1u, 0, box_.min.x, 1.0f}, {2u, 0, box_.max.x, -1.0f}, {4u, 1, box_.min.y, 1.0f}, {8u, 1, box_.max.y, -1.0f}};
                for (const auto& cut : cuts)
                    if ((sides_ & cut.side) && sideA_.size() >= 3)
                    {
                        ClipSide(sideA_, cut.axis, cut.v, cut.keep, sideB_);
                        sideA_.swap(sideB_);
                    }
                if (sideA_.size() >= 3)
                {
                    clipped_.Begin(c.groups[k]);
                    clipped_.pts.insert(clipped_.pts.end(), sideA_.begin(), sideA_.end());
                }
            }
            std::swap(c, clipped_);
        }

        PaintJob Renderer::PreparePaint(const Paint& p, float opacity, const Shape& sh) const
        {
            PaintJob job;
            const auto color = [&](Color c) { return o_.mask ? Color(1.0f, 1.0f, 1.0f, c.a) : c; };
            switch (p.kind)
            {
            case PaintKind::None:
                return job;
            case PaintKind::Solid:
                job.kind = PaintJob::Kind::Solid;
                job.solid = Premul(color(p.color), opacity);
                return job;
            case PaintKind::Current:
                job.kind = PaintJob::Kind::Solid;
                job.solid = Premul(current_, opacity * p.color.a);
                return job;
            case PaintKind::Gradient:
                break;
            }
            if (p.gradient < 0 || (std::size_t)p.gradient >= t_.gradients.size())
                return job;
            const Gradient& g = t_.gradients[(std::size_t)p.gradient];
            if (g.stops.empty())
                return job;
            Affine units = g.transform;
            if (!g.userSpace)
            {
                const Rect bb = t_.paths[sh.path].bounds;
                if (!(bb.Width() > 0.0f) || !(bb.Height() > 0.0f))
                    return job;   // a box without area has no fractions: the gradient paints nothing
                units = Affine{bb.Width(), 0.0f, 0.0f, bb.Height(), bb.min.x, bb.min.y} * g.transform;
            }
            const Affine toPixels = view_ * sh.transform * units;
            GradientJob& gj = job.grad;
            gj.offsets.reserve(g.stops.size());
            gj.colors.reserve(g.stops.size());
            for (const Stop& s : g.stops)
            {
                gj.offsets.push_back(s.offset);
                gj.colors.push_back(s.current ? Premul(current_, opacity * s.color.a) : Premul(color(s.color), opacity));
            }
            const auto solid = [&] {
                job.kind = PaintJob::Kind::Solid;
                job.solid = gj.colors.back();   // a gradient without extent: its last stop
                return job;
            };
            if (g.stops.size() == 1)
                return solid();
            if (!toPixels.Invert(gj.inv))
                return PaintJob();
            gj.spread = g.spread;
            if (!g.radial)
            {
                const float vx = g.x2 - g.x1, vy = g.y2 - g.y1, l2 = vx * vx + vy * vy;
                if (!(l2 > 0.0f))
                    return solid();
                gj.x1 = g.x1;
                gj.y1 = g.y1;
                gj.dx = vx / l2;
                gj.dy = vy / l2;
            }
            else
            {
                if (!(g.r > 0.0f))
                    return solid();
                gj.radial = true;
                gj.c0x = g.fx;
                gj.c0y = g.fy;
                gj.r0 = std::max(g.fr, 0.0f);
                gj.cdx = g.cx - g.fx;
                gj.cdy = g.cy - g.fy;
                gj.dr = g.r - gj.r0;
                gj.A = gj.cdx * gj.cdx + gj.cdy * gj.cdy - gj.dr * gj.dr;
                gj.centered = gj.cdx == 0.0f && gj.cdy == 0.0f && gj.r0 == 0.0f;
                gj.invR = 1.0f / g.r;
            }
            gj.scale.resize(gj.offsets.size(), 0.0f);
            for (std::size_t i = 0; i + 1 < gj.offsets.size(); ++i)
                gj.scale[i] = gj.offsets[i + 1] > gj.offsets[i] ? 1.0f / (gj.offsets[i + 1] - gj.offsets[i]) : 0.0f;
            gj.invA = gj.A != 0.0f ? 1.0f / gj.A : 0.0f;
            // opaque wherever it paints: opaque stops, and no pixel the two circles leave out
            gj.opaque = !gj.radial || gj.centered || gj.A < 0.0f;
            for (const Px& c : gj.colors)
                gj.opaque = gj.opaque && c.a >= 1.0f;
            job.kind = PaintJob::Kind::Gradient;
            return job;
        }

        ClipJob& Renderer::PrepareClip(int index)
        {
            ClipJob& c = clips_[(std::size_t)index];
            if (c.prepared)
                return c;
            c.prepared = true;
            const Clip& clip = t_.clips[(std::size_t)index];
            contours_.Clear();
            std::vector<std::uint8_t> evenOdd(std::max<std::size_t>(clip.items.size(), 1), 0);
            for (std::size_t k = 0; k < clip.items.size() && k < 65535; ++k)
            {
                const ClipItem& item = clip.items[k];
                FlattenFill(t_.paths[item.path], view_ * item.transform, kTolerance, (std::uint16_t)k, contours_);
                evenOdd[k] = item.evenOdd ? 1 : 0;
            }
            c.sweep.Build(contours_, evenOdd.data(), (int)evenOdd.size(), w_, h_);
            c.top = c.sweep.Top(), c.bottom = c.sweep.Bottom(), c.left = c.sweep.Left(), c.right = c.sweep.Right();
            c.row.assign((std::size_t)w_, 0.0f);
            // a clip path's own clip path: the intersection
            c.parent = clip.clip >= 0 && clip.clip != index && (std::size_t)clip.clip < t_.clips.size() ? clip.clip : -1;
            if (c.parent >= 0)
            {
                const int parent = c.parent;
                c.parent = -1;   // no cycles while it prepares
                const ClipJob& p = PrepareClip(parent);
                ClipJob& self = clips_[(std::size_t)index];
                self.parent = parent;
                self.top = std::max(self.top, p.top), self.bottom = std::min(self.bottom, p.bottom);
                self.left = std::max(self.left, p.left), self.right = std::min(self.right, p.right);
                return self;
            }
            return c;
        }

        const float* Renderer::ClipRow(int index, int y)
        {
            ClipJob& c = clips_[(std::size_t)index];
            if (c.rowY == y)
                return c.row.data();
            c.rowY = y;
            std::fill(c.row.begin(), c.row.end(), 0.0f);
            if (y >= c.sweep.Top() && y < c.sweep.Bottom())
            {
                c.sweep.Row(y, cells_);
                cells_.Resolve(spans_, cov_.data(), false);
                for (const Cells::Span& s : spans_)
                    for (int x = s.x0; x < s.x1; ++x)
                        c.row[(std::size_t)x] = s.value >= 0.0f ? s.value : cov_[(std::size_t)x];
            }
            if (c.parent >= 0)
            {
                const float* p = ClipRow(c.parent, y);
                for (int x = 0; x < w_; ++x)
                    c.row[(std::size_t)x] *= p[x];
            }
            return c.row.data();
        }

        void Renderer::Prepare(std::uint32_t index)
        {
            const Shape& sh = t_.shapes[index];
            ShapeJob& job = shapes_[index];
            const Affine m = view_ * sh.transform;
            const PathData& path = t_.paths[sh.path];
            job.fillPaint = PreparePaint(sh.fill, sh.fillOpacity, sh);
            if (sh.strokeStyle >= 0)
                job.strokePaint = PreparePaint(sh.stroke, sh.strokeOpacity, sh);
            job.strokeFirst = sh.strokeFirst;
            job.crisp = sh.crisp;
            bool empty = true;
            if (job.fillPaint.kind != PaintJob::Kind::None)
            {
                contours_.Clear();
                FlattenFill(path, m, kTolerance, 0, contours_);
                if (sides_)
                    CutToViewport(contours_);
                const std::uint8_t eo = sh.evenOdd ? 1 : 0;
                job.fill.Build(contours_, &eo, 1, w_, h_);
                job.hasFill = !job.fill.Empty();
                if (job.hasFill)
                    Union(job.top, job.bottom, job.left, job.right, empty, job.fill.Top(), job.fill.Bottom(), job.fill.Left(), job.fill.Right());
            }
            if (job.strokePaint.kind != PaintJob::Kind::None)
            {
                contours_.Clear();
                StrokeOutline(path, t_.strokes[(std::size_t)sh.strokeStyle], m, kTolerance, 0, contours_);
                if (sides_)
                    CutToViewport(contours_);
                const std::uint8_t nz = 0;
                job.stroke.Build(contours_, &nz, 1, w_, h_);
                job.hasStroke = !job.stroke.Empty();
                if (job.hasStroke)
                    Union(job.top, job.bottom, job.left, job.right, empty, job.stroke.Top(), job.stroke.Bottom(), job.stroke.Left(), job.stroke.Right());
            }
            if (empty)
            {
                job.hasFill = job.hasStroke = false;
                return;
            }
            if (sh.clip >= 0 && (std::size_t)sh.clip < t_.clips.size())
            {
                const ClipJob& c = PrepareClip(sh.clip);
                job.clip = sh.clip;
                job.top = std::max(job.top, c.top), job.bottom = std::min(job.bottom, c.bottom);
                job.left = std::max(job.left, c.left), job.right = std::min(job.right, c.right);
                if (job.top >= job.bottom || job.left >= job.right)
                    job.hasFill = job.hasStroke = false;
            }
        }

        void Renderer::DrawShape(ShapeJob& job, Px* row, int y)
        {
            const float* clip = job.clip >= 0 ? ClipRow(job.clip, y) : nullptr;
            for (int pass = 0; pass < 2; ++pass)
            {
                const bool stroke = (pass == 0) == job.strokeFirst;
                Sweep& sweep = stroke ? job.stroke : job.fill;
                if (!(stroke ? job.hasStroke : job.hasFill) || y < sweep.Top() || y >= sweep.Bottom())
                    continue;
                sweep.Row(y, cells_);
                cells_.Resolve(spans_, cov_.data(), job.crisp);
                if (!spans_.empty())
                    Composite(row, spans_, cov_.data(), stroke ? job.strokePaint : job.fillPaint, y, clip, scratch_.data());
            }
        }

        void Renderer::Run(Image& out)
        {
            const int b = std::max(0, o_.border);
            const int ow = w_ + 2 * b, oh = h_ + 2 * b;
            out.width = ow;
            out.height = oh;
            out.pixels.assign((std::size_t)ow * (std::size_t)oh * 4, 0);
            if (o_.mask)
                for (std::size_t i = 0; i < out.pixels.size(); i += 4)
                    out.pixels[i] = out.pixels[i + 1] = out.pixels[i + 2] = 255;   // white, so filtering keeps it white

            shapes_.resize(t_.shapes.size());
            clips_.resize(t_.clips.size());
            cells_.Reset(w_);
            cov_.assign((std::size_t)w_ + 1, 0.0f);
            scratch_.assign((std::size_t)w_ + 1, Px{0.0f, 0.0f, 0.0f, 0.0f});

            // shapes, the layers' extents from what is inside them, and the drawing's: the rows and columns anything
            // touches (the rest of the image is never visited)
            std::vector<LayerJob> layers(t_.layers.size());
            std::vector<std::uint32_t> stack;
            int maxDepth = 0;
            int top = 0, bottom = 0, left = 0, right = 0;
            bool none = true;
            for (const Op& op : t_.ops)
            {
                if (op.kind == OpKind::Shape)
                {
                    Prepare(op.index);
                    const ShapeJob& job = shapes_[op.index];
                    if (job.hasFill || job.hasStroke)
                        Union(top, bottom, left, right, none, job.top, job.bottom, job.left, job.right);
                    if ((job.hasFill || job.hasStroke) && !stack.empty())
                    {
                        LayerJob& l = layers[stack.back()];
                        Union(l.top, l.bottom, l.zl, l.zr, l.empty, job.top, job.bottom, job.left, job.right);
                    }
                }
                else if (op.kind == OpKind::Begin)
                {
                    stack.push_back(op.index);
                    maxDepth = std::max(maxDepth, (int)stack.size());
                }
                else if (!stack.empty())
                {
                    LayerJob& l = layers[stack.back()];
                    stack.pop_back();
                    const Layer& src = t_.layers[op.index];
                    l.opacity = src.opacity;
                    l.left = l.zl, l.right = l.zr;
                    if (!l.empty && src.clip >= 0 && (std::size_t)src.clip < t_.clips.size())
                    {
                        const ClipJob& c = PrepareClip(src.clip);
                        l.clip = src.clip;
                        l.top = std::max(l.top, c.top), l.bottom = std::min(l.bottom, c.bottom);
                        l.left = std::max(l.left, c.left), l.right = std::min(l.right, c.right);
                        if (l.top >= l.bottom || l.left >= l.right)
                            l.empty = true;
                    }
                    if (!l.empty && !stack.empty())
                    {
                        LayerJob& p = layers[stack.back()];
                        Union(p.top, p.bottom, p.zl, p.zr, p.empty, l.top, l.bottom, l.left, l.right);
                    }
                }
            }

            if (none)
                return;   // nothing drawn: transparent
            std::vector<std::vector<Px>> rows((std::size_t)maxDepth + 1, std::vector<Px>((std::size_t)w_ + 1, Px{0.0f, 0.0f, 0.0f, 0.0f}));
            std::vector<std::uint8_t> full((std::size_t)oh, 0);   // rows inked across all the drawing's columns
            for (int y = top; y < bottom; ++y)
            {
                std::fill(rows[0].begin() + left, rows[0].begin() + right, Px{0.0f, 0.0f, 0.0f, 0.0f});
                std::size_t depth = 0;
                for (std::size_t i = 0; i < t_.ops.size(); ++i)
                {
                    const Op& op = t_.ops[i];
                    if (op.kind == OpKind::Shape)
                    {
                        ShapeJob& job = shapes_[op.index];
                        if ((job.hasFill || job.hasStroke) && y >= job.top && y < job.bottom)
                            DrawShape(job, rows[depth].data(), y);
                    }
                    else if (op.kind == OpKind::Begin)
                    {
                        const LayerJob& l = layers[op.index];
                        if (l.empty || y < l.top || y >= l.bottom)
                        {
                            i = op.match;   // past its End
                            continue;
                        }
                        ++depth;
                        std::fill(rows[depth].begin() + l.zl, rows[depth].begin() + l.zr, Px{0.0f, 0.0f, 0.0f, 0.0f});
                    }
                    else if (depth > 0)
                    {
                        const LayerJob& l = layers[op.index];
                        const float* clip = l.clip >= 0 ? ClipRow(l.clip, y) : nullptr;
                        const Px* src = rows[depth].data();
                        Px* dst = rows[depth - 1].data();
                        for (int x = l.left; x < l.right; ++x)
                        {
                            const float k = l.opacity * (clip ? clip[x] : 1.0f);
                            if (k > 0.0f && src[x].a > 0.0f)
                                Blend(dst[x], src[x], k);
                        }
                        --depth;
                    }
                }

                // straight alpha, 8 bits
                std::uint8_t* dst = out.pixels.data() + ((std::size_t)(y + b) * (std::size_t)ow + (std::size_t)(b + left)) * 4;
                const int ink = Straighten(rows[0].data() + left, dst, right - left, o_.mask);
                full[(std::size_t)(y + b)] = ink == right - left ? 1 : 0;
            }

            if (o_.mask)
                return;
            // transparent texels take the color of the texels next to them (alpha-weighted): a texel sampled between the
            // drawing and the transparent area around it keeps the drawing's color. Only around the drawing, and only the
            // texels a 3 x 3 window of alpha marks (rows of "alpha here or beside" ORed with the rows above and below).
            std::uint8_t* px = out.pixels.data();
            const auto at = [&](int x, int y) { return px + ((std::size_t)y * (std::size_t)ow + (std::size_t)x) * 4; };
            const int x0 = std::max(0, b + left - 1), x1 = std::min(ow, b + right + 1);
            const int y0 = std::max(0, b + top - 1), y1 = std::min(oh, b + bottom + 1);
            // "alpha here or beside", for three rows at a time
            struct Marks
            {
                int y = -2;
                std::vector<std::uint8_t> m;
            } marks[3];
            const auto window = [&](int y) -> const std::vector<std::uint8_t>& {
                Marks& s = marks[(std::size_t)((y % 3 + 3) % 3)];
                if (s.y == y)
                    return s.m;
                s.y = y;
                s.m.assign((std::size_t)ow, 0);
                if (y < 0 || y >= oh)
                    return s.m;
                const std::uint8_t* row = at(0, y);
                for (int x = x0; x < x1; ++x)
                {
                    if (!row[(std::size_t)x * 4 + 3])
                        continue;
                    s.m[(std::size_t)x] = 1;
                    if (x > 0)
                        s.m[(std::size_t)x - 1] = 1;
                    if (x + 1 < ow)
                        s.m[(std::size_t)x + 1] = 1;
                }
                return s.m;
            };
            for (int y = y0; y < y1; ++y)
            {
                // a row inked all across: only the columns beside the drawing can need a color
                const bool across = full[(std::size_t)y] != 0;
                const std::vector<std::uint8_t>* m[3] = {nullptr, nullptr, nullptr};
                if (!across)
                {
                    m[0] = &window(y - 1);
                    m[1] = &window(y);
                    m[2] = &window(y + 1);
                }
                for (int x = x0; x < x1; ++x)
                {
                    if (across && x != x0 && x != x1 - 1)
                        x = x1 - 1;
                    std::uint8_t* p = at(x, y);
                    if (p[3] != 0)
                        continue;
                    if (!across && !((*m[0])[(std::size_t)x] | (*m[1])[(std::size_t)x] | (*m[2])[(std::size_t)x]))
                        continue;
                    unsigned r = 0, g = 0, bl = 0, wsum = 0;
                    for (int dy = -1; dy <= 1; ++dy)
                        for (int dx = -1; dx <= 1; ++dx)
                        {
                            const int nx = x + dx, ny = y + dy;
                            if ((dx == 0 && dy == 0) || nx < 0 || ny < 0 || nx >= ow || ny >= oh)
                                continue;
                            const std::uint8_t* n = at(nx, ny);
                            if (n[3] == 0)
                                continue;
                            r += (unsigned)n[0] * n[3];
                            g += (unsigned)n[1] * n[3];
                            bl += (unsigned)n[2] * n[3];
                            wsum += n[3];
                        }
                    if (wsum)
                    {
                        p[0] = (std::uint8_t)((r + wsum / 2) / wsum);
                        p[1] = (std::uint8_t)((g + wsum / 2) / wsum);
                        p[2] = (std::uint8_t)((bl + wsum / 2) / wsum);
                    }
                }
            }
        }
    }

    bool Render(const Tree& tree, int width, int height, const RasterOptions& options, Image& out)
    {
        Renderer r(tree, width, height, options);
        r.Run(out);
        return true;
    }
}
