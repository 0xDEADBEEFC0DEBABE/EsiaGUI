// Esia - analytic glyph rasterizer (include/esia/text/glyph_raster.hpp): WGT's src/text/glyph_raster.cpp with the
// DirectWrite outline sink replaced by Outline.
//
// Coverage: signed-area accumulation per scanline - each edge deposits the exact area it covers in the cells it
// crosses, a running sum across the row gives the covered fraction of every pixel; non-zero winding by clamping
// |sum| to 1.
#include "esia/text/glyph_raster.hpp"
#include <algorithm>
#include <cmath>

namespace esia::text
{
    // ------------------------------------------------------------------ outline
    void Outline::MoveTo(Vec2 p)
    {
        starts_.push_back(points_.size());
        points_.push_back(p);
    }

    void Outline::LineTo(Vec2 p)
    {
        if (starts_.empty())
        {
            MoveTo(p);   // a contour has to start somewhere: one that skips MoveTo starts here
            return;
        }
        points_.push_back(p);
    }

    void Outline::QuadTo(Vec2 control, Vec2 p)
    {
        if (starts_.empty())
        {
            MoveTo(p);
            return;
        }
        // degree elevation is exact: one flattening path for both curve kinds
        const Vec2 p0 = points_.back();
        CubicTo(p0 + (control - p0) * (2.0f / 3.0f), p + (control - p) * (2.0f / 3.0f), p);
    }

    void Outline::CubicTo(Vec2 p1, Vec2 p2, Vec2 p3)
    {
        if (starts_.empty())
        {
            MoveTo(p3);
            return;
        }
        const Vec2 p0 = points_.back();
        // subdivisions so the flattening error stays below `tol` (second-difference bound)
        const float ax = p0.x - 2.0f * p1.x + p2.x, ay = p0.y - 2.0f * p1.y + p2.y;
        const float bx = p1.x - 2.0f * p2.x + p3.x, by = p1.y - 2.0f * p2.y + p3.y;
        const float dd = std::sqrt(std::max(ax * ax + ay * ay, bx * bx + by * by));
        constexpr float tol = 1.0f / 12.0f;
        const int n = std::clamp((int)std::ceil(std::sqrt(0.75f * dd / tol)), 1, 64);
        for (int k = 1; k <= n; ++k)
        {
            const float t = (float)k / (float)n, u = 1.0f - t;
            const float w0 = u * u * u, w1 = 3.0f * u * u * t, w2 = 3.0f * u * t * t, w3 = t * t * t;
            points_.push_back({w0 * p0.x + w1 * p1.x + w2 * p2.x + w3 * p3.x, w0 * p0.y + w1 * p1.y + w2 * p2.y + w3 * p3.y});
        }
    }

    void Outline::Clear()
    {
        points_.clear();
        starts_.clear();
    }

    std::span<const Vec2> Outline::Contour(std::size_t i) const
    {
        const std::size_t end = i + 1 < starts_.size() ? starts_[i + 1] : points_.size();
        return {points_.data() + starts_[i], end - starts_[i]};
    }

    Rect Outline::Bounds() const
    {
        if (points_.empty())
            return {};
        Rect r(points_[0], points_[0]);
        for (const Vec2& p : points_)
        {
            r.min.x = std::min(r.min.x, p.x);
            r.min.y = std::min(r.min.y, p.y);
            r.max.x = std::max(r.max.x, p.x);
            r.max.y = std::max(r.max.y, p.y);
        }
        return r;
    }

    // ------------------------------------------------------------------ rasterizer
    namespace
    {
        constexpr int kMaxGlyphPixels = 4096;

        // Signed-area accumulation of one edge (bitmap coordinates, y down).
        class Accumulator
        {
        public:
            Accumulator(int w, int h) : w_(w), h_(h), a_((std::size_t)w * h + 4, 0.0f) {}

            void Line(Vec2 p0, Vec2 p1)
            {
                if (std::fabs(p0.y - p1.y) <= 1e-6f)
                    return;
                float dir = 1.0f;
                if (p0.y > p1.y)
                {
                    std::swap(p0, p1);
                    dir = -1.0f;
                }
                const float dxdy = (p1.x - p0.x) / (p1.y - p0.y);
                float x = p0.x;
                if (p0.y < 0.0f)
                    x -= p0.y * dxdy;
                const int yEnd = std::min(h_, (int)std::ceil(p1.y));
                for (int y = std::max(0, (int)p0.y); y < yEnd; ++y)
                {
                    float* row = a_.data() + (std::size_t)y * w_;
                    const float dy = std::min((float)(y + 1), p1.y) - std::max((float)y, p0.y);
                    const float xnext = x + dxdy * dy;
                    const float d = dy * dir;
                    const float x0 = std::min(x, xnext), x1 = std::max(x, xnext);
                    const float x0floor = std::floor(x0);
                    const int x0i = (int)x0floor;
                    const float x1ceil = std::ceil(x1);
                    const int x1i = (int)x1ceil;
                    if (x0i < 0 || x1i + 1 >= w_)
                    {
                        x = xnext;   // outside the padded box: cannot happen for a correct bounding box
                        continue;
                    }
                    if (x1i <= x0i + 1)
                    {
                        const float xmf = 0.5f * (x + xnext) - x0floor;
                        row[x0i] += d - d * xmf;
                        row[x0i + 1] += d * xmf;
                    }
                    else
                    {
                        const float s = 1.0f / (x1 - x0);
                        const float x0f = x0 - x0floor;
                        const float a0 = 0.5f * s * (1.0f - x0f) * (1.0f - x0f);
                        const float x1f = x1 - x1ceil + 1.0f;
                        const float am = 0.5f * s * x1f * x1f;
                        row[x0i] += d * a0;
                        if (x1i == x0i + 2)
                            row[x0i + 1] += d * (1.0f - a0 - am);
                        else
                        {
                            const float a1 = s * (1.5f - x0f);
                            row[x0i + 1] += d * (a1 - a0);
                            for (int xi = x0i + 2; xi < x1i - 1; ++xi)
                                row[xi] += d * s;
                            const float a2 = a1 + (float)(x1i - x0i - 3) * s;
                            row[x1i - 1] += d * (1.0f - a2 - am);
                        }
                        row[x1i] += d * am;
                    }
                    x = xnext;
                }
            }

            void Resolve(std::vector<std::uint8_t>& out) const
            {
                out.resize((std::size_t)w_ * h_);
                for (int y = 0; y < h_; ++y)
                {
                    float acc = 0.0f;
                    const float* row = a_.data() + (std::size_t)y * w_;
                    std::uint8_t* dst = out.data() + (std::size_t)y * w_;
                    for (int x = 0; x < w_; ++x)
                    {
                        acc += row[x];
                        dst[x] = (std::uint8_t)(std::min(std::fabs(acc), 1.0f) * 255.0f + 0.5f);
                    }
                }
            }

        private:
            int w_, h_;
            std::vector<float> a_;
        };

        struct PixelBox
        {
            int left = 0, top = 0, width = 0, height = 0;
        };

        // Pixel box of the outline shifted by `offsetX`: an empty column on the left, two on the right (the
        // accumulator writes one cell past an edge's last pixel), an empty row above and below. False when the
        // outline has no ink.
        bool InkBox(const Outline& outline, float offsetX, PixelBox& box)
        {
            std::size_t points = 0;
            for (std::size_t c = 0; c < outline.ContourCount(); ++c)
                points += outline.Contour(c).size();
            if (points < 2)
                return false;
            const Rect b = outline.Bounds();
            box.left = (int)std::floor(b.min.x + offsetX) - 1;
            box.top = (int)std::floor(b.min.y) - 1;
            box.width = (int)std::ceil(b.max.x + offsetX) + 2 - box.left;
            box.height = (int)std::ceil(b.max.y) + 1 - box.top;
            return true;
        }

        // Every edge of every contour, the closing edge included, in bitmap coordinates.
        void Accumulate(const Outline& outline, float offsetX, const PixelBox& box, Accumulator& acc)
        {
            const auto map = [&](Vec2 p) { return Vec2(p.x + offsetX - (float)box.left, p.y - (float)box.top); };
            for (std::size_t c = 0; c < outline.ContourCount(); ++c)
            {
                const std::span<const Vec2> pts = outline.Contour(c);
                for (std::size_t i = 0; i < pts.size(); ++i)
                    acc.Line(map(pts[i]), map(pts[(i + 1) % pts.size()]));
            }
        }
    }

    bool RasterizeGrown(const Outline& outline, float offsetX, float radius, GlyphBitmap& out)
    {
        out = {};
        PixelBox box;
        if (!InkBox(outline, offsetX, box))
            return true;
        radius = std::max(radius, 0.0f);
        const int grow = (int)std::ceil(radius) + 1;
        box.left -= grow;
        box.top -= grow;
        box.width += 2 * grow;
        box.height += 2 * grow;
        if (box.width > kMaxGlyphPixels || box.height > kMaxGlyphPixels)
            return false;
        Accumulator acc(box.width, box.height);
        Accumulate(outline, offsetX, box, acc);
        std::vector<std::uint8_t> coverage;
        acc.Resolve(coverage);

        // the squared distance from each pixel's center to the nearest edge, where it can matter (within radius + 1):
        // every edge visits the pixels around it only, so the cost follows the glyph's perimeter, not its area
        const float reach = radius + 1.0f, reach2 = reach * reach;
        std::vector<float> d2((std::size_t)box.width * (std::size_t)box.height, reach2);
        const auto map = [&](Vec2 p) { return Vec2(p.x + offsetX - (float)box.left, p.y - (float)box.top); };
        for (std::size_t c = 0; c < outline.ContourCount(); ++c)
        {
            const std::span<const Vec2> pts = outline.Contour(c);
            for (std::size_t i = 0; i < pts.size(); ++i)
            {
                const Vec2 a = map(pts[i]), b = map(pts[(i + 1) % pts.size()]);
                const Vec2 ab = b - a;
                const float len2 = Dot(ab, ab);
                const int x0 = std::max(0, (int)std::floor(std::min(a.x, b.x) - reach)), x1 = std::min(box.width - 1, (int)std::ceil(std::max(a.x, b.x) + reach));
                const int y0 = std::max(0, (int)std::floor(std::min(a.y, b.y) - reach)), y1 = std::min(box.height - 1, (int)std::ceil(std::max(a.y, b.y) + reach));
                for (int y = y0; y <= y1; ++y)
                {
                    float* row = d2.data() + (std::size_t)y * (std::size_t)box.width;
                    for (int x = x0; x <= x1; ++x)
                    {
                        const Vec2 ap = Vec2((float)x + 0.5f, (float)y + 0.5f) - a;
                        const float t = len2 > 0.0f ? Clamp(Dot(ap, ab) / len2, 0.0f, 1.0f) : 0.0f;
                        const Vec2 d = ap - ab * t;
                        row[x] = std::min(row[x], Dot(d, d));
                    }
                }
            }
        }
        out.pixels.resize(coverage.size());
        for (std::size_t i = 0; i < coverage.size(); ++i)
        {
            const float grown = Saturate(radius + 0.5f - std::sqrt(d2[i]));
            out.pixels[i] = std::max(coverage[i], (std::uint8_t)(grown * 255.0f + 0.5f));
        }
        out.left = box.left;
        out.top = box.top;
        out.width = box.width;
        out.height = box.height;
        return true;
    }

    bool RasterizeGray(const Outline& outline, float offsetX, GlyphBitmap& out)
    {
        out = {};
        PixelBox box;
        if (!InkBox(outline, offsetX, box))
            return true;   // no ink (space): valid, nothing to draw
        if (box.width > kMaxGlyphPixels || box.height > kMaxGlyphPixels)
            return false;
        Accumulator acc(box.width, box.height);
        Accumulate(outline, offsetX, box, acc);
        acc.Resolve(out.pixels);
        out.left = box.left;
        out.top = box.top;
        out.width = box.width;
        out.height = box.height;
        return true;
    }
}
