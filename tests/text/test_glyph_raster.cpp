// The analytic rasterizer: exact area coverage (edges, sub-pixel offsets, area of arbitrary outlines), non-zero
// winding and curve flattening within its tolerance.
#include "esia/text/glyph_raster.hpp"
#include "esia_test.hpp"
#include <algorithm>
#include <cmath>

using namespace esia;
using namespace esia::text;

namespace
{
    void AddRect(Outline& o, float x0, float y0, float x1, float y1, bool reverse = false)
    {
        o.MoveTo(Vec2(x0, y0));
        if (reverse)
        {
            o.LineTo(Vec2(x0, y1));
            o.LineTo(Vec2(x1, y1));
            o.LineTo(Vec2(x1, y0));
        }
        else
        {
            o.LineTo(Vec2(x1, y0));
            o.LineTo(Vec2(x1, y1));
            o.LineTo(Vec2(x0, y1));
        }
    }

    void AddCircle(Outline& o, Vec2 c, float r)
    {
        const float k = 0.5522847f * r;   // cubic quarter-circle handles
        o.MoveTo(c + Vec2(r, 0));
        o.CubicTo(c + Vec2(r, k), c + Vec2(k, r), c + Vec2(0, r));
        o.CubicTo(c + Vec2(-k, r), c + Vec2(-r, k), c + Vec2(-r, 0));
        o.CubicTo(c + Vec2(-r, -k), c + Vec2(-k, -r), c + Vec2(0, -r));
        o.CubicTo(c + Vec2(k, -r), c + Vec2(r, -k), c + Vec2(r, 0));
    }

    // coverage (0..255) at absolute pixel (x, y); 0 outside the bitmap
    int At(const GlyphBitmap& b, int x, int y)
    {
        const int bx = x - b.left, by = y - b.top;
        if (bx < 0 || by < 0 || bx >= b.width || by >= b.height)
            return 0;
        return b.pixels[(std::size_t)by * b.width + bx];
    }

    double CoverageSum(const GlyphBitmap& b)
    {
        double sum = 0.0;
        for (std::uint8_t v : b.pixels)
            sum += v;
        return sum / 255.0;
    }

    // area enclosed by the flattened outline (shoelace, signed per contour)
    double PolygonArea(const Outline& o)
    {
        double area = 0.0;
        for (std::size_t c = 0; c < o.ContourCount(); ++c)
        {
            const auto pts = o.Contour(c);
            double a = 0.0;
            for (std::size_t i = 0; i < pts.size(); ++i)
            {
                const Vec2 p = pts[i], q = pts[(i + 1) % pts.size()];
                a += (double)p.x * q.y - (double)q.x * p.y;
            }
            area += a * 0.5;
        }
        return std::fabs(area);
    }

    float DistanceToPolyline(std::span<const Vec2> pts, Vec2 p)
    {
        float best = 1e9f;
        for (std::size_t i = 0; i + 1 < pts.size(); ++i)
        {
            const Vec2 a = pts[i], ab = pts[i + 1] - a;
            const float t = Clamp(Dot(p - a, ab) / std::max(LengthSq(ab), 1e-12f), 0.0f, 1.0f);
            best = std::min(best, Length(p - (a + ab * t)));
        }
        return best;
    }
}

ESIA_TEST(Raster, PixelAlignedSquareIsExactWithEmptyBorders)
{
    Outline o;
    AddRect(o, 2, 3, 7, 9);
    GlyphBitmap b;
    ESIA_CHECK(RasterizeGray(o, 0.0f, b));
    ESIA_CHECK(b.left == 1 && b.top == 2 && b.width == 8 && b.height == 8);
    ESIA_CHECK(b.pixels.size() == 64);
    for (int y = b.top; y < b.top + b.height; ++y)
        for (int x = b.left; x < b.left + b.width; ++x)
            ESIA_CHECK(At(b, x, y) == ((x >= 2 && x < 7 && y >= 3 && y < 9) ? 255 : 0));
    ESIA_CHECK_NEAR(CoverageSum(b), 30.0, 1e-9);
}

ESIA_TEST(Raster, GrownCoverageReachesTheRadiusRoundAtTheCorners)
{
    Outline o;
    AddRect(o, 10, 10, 20, 20);
    GlyphBitmap g, b;
    ESIA_CHECK(RasterizeGray(o, 0.0f, g));
    ESIA_CHECK(RasterizeGrown(o, 0.0f, 2.0f, b));
    // larger by the growth (ceil(2) + 1) on every side
    ESIA_CHECK(b.left == g.left - 3 && b.top == g.top - 3 && b.width == g.width + 6 && b.height == g.height + 6);
    ESIA_CHECK(b.pixels.size() == (std::size_t)b.width * b.height);
    ESIA_CHECK(At(b, 15, 15) == 255);   // inside
    ESIA_CHECK(At(b, 8, 15) == 255);    // a center 1.5 px out: within 2 + 1/2
    ESIA_CHECK(At(b, 7, 15) == 0);      // 2.5 px out: none
    // the corner is round: (8.5, 8.5) lies 2.12 px from (10, 10), where a square growth would cover it
    ESIA_CHECK(At(b, 8, 8) > 60 && At(b, 8, 8) < 140);
    // never less than the glyph, and empty borders as RasterizeGray's
    bool covers = true, emptyBorders = true;
    for (int y = g.top; y < g.top + g.height; ++y)
        for (int x = g.left; x < g.left + g.width; ++x)
            covers = covers && At(b, x, y) >= At(g, x, y);
    for (int x = b.left; x < b.left + b.width; ++x)
        emptyBorders = emptyBorders && At(b, x, b.top) == 0 && At(b, x, b.top + b.height - 1) == 0;
    for (int y = b.top; y < b.top + b.height; ++y)
        emptyBorders = emptyBorders && At(b, b.left, y) == 0 && At(b, b.left + b.width - 1, y) == 0;
    ESIA_CHECK(covers && emptyBorders);

    // half a pixel of coverage where a center lies at radius + 0 (1.5 px from the edge, radius 1.5)
    GlyphBitmap h;
    ESIA_CHECK(RasterizeGrown(o, 0.0f, 1.5f, h));
    ESIA_CHECK(std::abs(At(h, 8, 15) - 128) <= 1);
    // no growth: the glyph's own coverage
    GlyphBitmap z;
    ESIA_CHECK(RasterizeGrown(o, 0.0f, 0.0f, z));
    ESIA_CHECK_NEAR(CoverageSum(z), CoverageSum(g), 1e-9);
    // no ink: an empty, valid bitmap
    GlyphBitmap e;
    ESIA_CHECK(RasterizeGrown(Outline(), 0.0f, 2.0f, e) && e.width == 0 && e.pixels.empty());
}

ESIA_TEST(Raster, PartialPixelsGetTheirAreaFraction)
{
    // half-pixel edges
    Outline half;
    AddRect(half, 2.5f, 3.0f, 6.5f, 5.0f);
    GlyphBitmap b;
    ESIA_CHECK(RasterizeGray(half, 0.0f, b));
    ESIA_CHECK(At(b, 2, 3) == 128 && At(b, 3, 3) == 255 && At(b, 5, 4) == 255 && At(b, 6, 4) == 128 && At(b, 7, 4) == 0);

    // the pen's sub-pixel phase moves coverage, not the shape
    Outline sq;
    AddRect(sq, 2, 3, 7, 9);
    ESIA_CHECK(RasterizeGray(sq, 0.25f, b));
    ESIA_CHECK(At(b, 2, 5) == 191 && At(b, 3, 5) == 255 && At(b, 6, 5) == 255 && At(b, 7, 5) == 64);
    ESIA_CHECK_NEAR(CoverageSum(b), 30.0, 0.02);

    // a diagonal edge: the triangle's area to quantization
    Outline tri;
    tri.MoveTo(Vec2(0.3f, 0.6f));
    tri.LineTo(Vec2(10.3f, 0.6f));
    tri.LineTo(Vec2(0.3f, 10.6f));
    ESIA_CHECK(RasterizeGray(tri, 0.0f, b));
    ESIA_CHECK_NEAR(CoverageSum(b), 50.0, 0.25);
    ESIA_CHECK(At(b, 1, 1) == 255 && At(b, 9, 9) == 0);
}

ESIA_TEST(Raster, CurvesFlattenWithinToleranceAndCoverTheirArea)
{
    Outline circle;
    AddCircle(circle, Vec2(10.4f, 10.7f), 8.0f);
    ESIA_CHECK(circle.ContourCount() == 1);
    const auto pts = circle.Contour(0);
    // every flattened point is on the circle, and the circle never strays more than 1/12 px from the polygon
    for (const Vec2& p : pts)
        ESIA_CHECK_NEAR(Length(p - Vec2(10.4f, 10.7f)), 8.0f, 0.01f);
    for (int i = 0; i < 360; ++i)
    {
        const float a = Radians((float)i + 0.5f);
        ESIA_CHECK(DistanceToPolyline(pts, Vec2(10.4f, 10.7f) + Vec2(std::cos(a), std::sin(a)) * 8.0f) <= 1.0f / 12.0f + 0.01f);
    }
    const Rect bounds = circle.Bounds();
    ESIA_CHECK_NEAR(bounds.min.x, 2.4f, 1e-4f);
    ESIA_CHECK_NEAR(bounds.max.y, 18.7f, 1e-4f);

    // coverage adds up to the area of what was flattened: exact area coverage, whatever the edge slopes
    GlyphBitmap b;
    ESIA_CHECK(RasterizeGray(circle, 0.5f, b));
    ESIA_CHECK_NEAR(CoverageSum(b), PolygonArea(circle), 0.5);

    // quadratic (TrueType) curves take the same path: an arch stays within tolerance of the true parabola
    Outline arch;
    arch.MoveTo(Vec2(0, 20));
    arch.QuadTo(Vec2(15, -10), Vec2(30, 20));
    const auto ap = arch.Contour(0);
    ESIA_CHECK(ap.size() > 4);
    for (int i = 0; i <= 100; ++i)
    {
        const float t = (float)i / 100.0f, u = 1.0f - t;
        const Vec2 q = Vec2(0, 20) * (u * u) + Vec2(15, -10) * (2.0f * u * t) + Vec2(30, 20) * (t * t);
        ESIA_CHECK(DistanceToPolyline(ap, q) <= 1.0f / 12.0f + 0.01f);
    }
}

ESIA_TEST(Raster, NonZeroWinding)
{
    // a counter-wound inner contour is a hole; a co-wound one is not
    Outline hole;
    AddRect(hole, 0, 0, 10, 10);
    AddRect(hole, 3, 3, 7, 7, true);
    GlyphBitmap b;
    ESIA_CHECK(RasterizeGray(hole, 0.0f, b));
    ESIA_CHECK(At(b, 1, 1) == 255 && At(b, 5, 5) == 0 && At(b, 8, 5) == 255);
    ESIA_CHECK_NEAR(CoverageSum(b), 100.0 - 16.0, 1e-9);

    Outline overlap;
    AddRect(overlap, 0, 0, 10, 10);
    AddRect(overlap, 3, 3, 7, 7);
    ESIA_CHECK(RasterizeGray(overlap, 0.0f, b));
    ESIA_CHECK(At(b, 5, 5) == 255);
    ESIA_CHECK_NEAR(CoverageSum(b), 100.0, 1e-9);
}

ESIA_TEST(Raster, EmptyAndOversizedOutlines)
{
    Outline empty;
    GlyphBitmap b;
    b.width = 3;
    ESIA_CHECK(RasterizeGray(empty, 0.0f, b));   // a space: valid, nothing to draw
    ESIA_CHECK(b.width == 0 && b.height == 0 && b.pixels.empty());
    ESIA_CHECK(empty.Bounds().Empty());

    Outline huge;
    AddRect(huge, 0, 0, 5000, 20);
    ESIA_CHECK(!RasterizeGray(huge, 0.0f, b));

    // a contour that skips MoveTo starts at its first point
    Outline implicit;
    implicit.LineTo(Vec2(1, 1));
    implicit.LineTo(Vec2(5, 1));
    implicit.LineTo(Vec2(5, 5));
    ESIA_CHECK(implicit.ContourCount() == 1 && implicit.Contour(0).size() == 3);
    implicit.Clear();
    ESIA_CHECK(implicit.ContourCount() == 0);
}
