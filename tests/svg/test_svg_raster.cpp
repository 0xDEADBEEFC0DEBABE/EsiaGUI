// SVG rasterizing: exact area coverage - a pixel's coverage is the area of the filled region in it, under either rule and
// where contours overlap or cross (checked against dense point sampling); strokes as the union of their pieces (a round
// stroke is every point within half its width of the path); paints, layers, clips, masks and the texels around the
// drawing; images of parts of a drawing; the atlas's cache, its icons in the text's pages, what the clip rect shows.
#include "esia/core/draw_list.hpp"
#include "esia/core/texture.hpp"
#include "esia/render/painter.hpp"
#include "esia/svg/svg.hpp"
#include "esia/text/glyph_atlas.hpp"
#include "esia_test.hpp"
#include "svg_internal.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace esia;
using namespace esia::svg;
using namespace esia::svg::detail;

namespace
{
    struct Coverage
    {
        int w = 0, h = 0;
        std::vector<float> v;
        float At(int x, int y) const { return v[(std::size_t)y * (std::size_t)w + (std::size_t)x]; }
    };

    // the sweep's coverage of `c` in w x h pixels
    Coverage Cover(const Contours& c, std::vector<std::uint8_t> evenOdd, int w, int h)
    {
        detail::Sweep s;
        s.Build(c, evenOdd.data(), (int)evenOdd.size(), w, h);
        Cells cells;
        cells.Reset(w);
        Coverage out{w, h, std::vector<float>((std::size_t)w * (std::size_t)h, 0.0f)};
        std::vector<Cells::Span> spans;
        std::vector<float> cov((std::size_t)w + 1);
        for (int y = s.Top(); y < s.Bottom(); ++y)
        {
            s.Row(y, cells);
            cells.Resolve(spans, cov.data(), false);
            for (const Cells::Span& sp : spans)
                for (int x = sp.x0; x < sp.x1; ++x)
                    out.v[(std::size_t)y * (std::size_t)w + (std::size_t)x] = sp.value >= 0.0f ? sp.value : cov[(std::size_t)x];
        }
        return out;
    }

    int Winding(const Contours& c, std::size_t k, Vec2 p)
    {
        const std::size_t s = c.starts[k], e = k + 1 < c.starts.size() ? c.starts[k + 1] : c.pts.size();
        int w = 0;
        for (std::size_t i = s; i < e; ++i)
        {
            const Vec2 a = c.pts[i], b = c.pts[i + 1 < e ? i + 1 : s];
            const float side = (b.x - a.x) * (p.y - a.y) - (p.x - a.x) * (b.y - a.y);
            if (a.y <= p.y && p.y < b.y && side > 0.0f)
                ++w;
            else if (b.y <= p.y && p.y < a.y && side < 0.0f)
                --w;
        }
        return w;
    }

    // the same by n x n points a pixel: inside the union of the groups, each by its rule
    Coverage Sampled(const Contours& c, const std::vector<std::uint8_t>& evenOdd, int w, int h, int n)
    {
        Coverage out{w, h, std::vector<float>((std::size_t)w * (std::size_t)h, 0.0f)};
        std::vector<int> wind(evenOdd.size());
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            {
                int inside = 0;
                for (int sy = 0; sy < n; ++sy)
                    for (int sx = 0; sx < n; ++sx)
                    {
                        const Vec2 p((float)x + ((float)sx + 0.5f) / (float)n, (float)y + ((float)sy + 0.5f) / (float)n);
                        std::fill(wind.begin(), wind.end(), 0);
                        for (std::size_t k = 0; k < c.starts.size(); ++k)
                            wind[c.groups[k]] += Winding(c, k, p);
                        bool in = false;
                        for (std::size_t g = 0; g < wind.size(); ++g)
                            in = in || (evenOdd[g] ? (wind[g] & 1) != 0 : wind[g] != 0);
                        inside += in ? 1 : 0;
                    }
                out.v[(std::size_t)y * (std::size_t)w + (std::size_t)x] = (float)inside / (float)(n * n);
            }
        return out;
    }

    float MaxDiff(const Coverage& a, const Coverage& b)
    {
        float m = 0.0f;
        for (std::size_t i = 0; i < a.v.size(); ++i)
            m = std::max(m, std::fabs(a.v[i] - b.v[i]));
        return m;
    }

    void Polygon(Contours& c, std::initializer_list<Vec2> pts, std::uint16_t group = 0)
    {
        c.Begin(group);
        c.pts.insert(c.pts.end(), pts.begin(), pts.end());
    }

    // the area of [x0, x1] x [y0, y1] inside pixel (px, py)
    float BoxIn(float x0, float y0, float x1, float y1, int px, int py)
    {
        const float w = std::max(0.0f, std::min(x1, (float)px + 1.0f) - std::max(x0, (float)px));
        const float h = std::max(0.0f, std::min(y1, (float)py + 1.0f) - std::max(y0, (float)py));
        return w * h;
    }

    Image Raster(const std::string& svg, int w, int h, RasterOptions o = {})
    {
        std::string error;
        const Document doc = Document::Parse(svg, &error);
        ESIA_CHECK(!doc.Empty());
        if (doc.Empty())
            std::printf("    %s\n", error.c_str());
        Image img;
        ESIA_CHECK(Rasterize(doc, w, h, o, img));
        return img;
    }

    const std::uint8_t* Px(const Image& img, int x, int y) { return img.pixels.data() + ((std::size_t)y * (std::size_t)img.width + (std::size_t)x) * 4; }
}

ESIA_TEST(SvgCoverage, RectanglesAtFractionalEdges)
{
    Contours c;
    Polygon(c, {Vec2(1.25f, 2.5f), Vec2(5.75f, 2.5f), Vec2(5.75f, 4.0f), Vec2(1.25f, 4.0f)});
    const Coverage cv = Cover(c, {0}, 8, 8);
    float worst = 0.0f;
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            worst = std::max(worst, std::fabs(cv.At(x, y) - BoxIn(1.25f, 2.5f, 5.75f, 4.0f, x, y)));
    ESIA_CHECK(worst < 1e-5f);
}

ESIA_TEST(SvgCoverage, OverlapsAreTheirUnion)
{
    // two squares wound the same way, overlapping, edges inside the same pixels: the union's area, never more
    Contours c;
    Polygon(c, {Vec2(1.3f, 1.3f), Vec2(6.4f, 1.3f), Vec2(6.4f, 6.4f), Vec2(1.3f, 6.4f)});
    Polygon(c, {Vec2(3.7f, 3.6f), Vec2(8.2f, 3.6f), Vec2(8.2f, 8.1f), Vec2(3.7f, 8.1f)});
    const Coverage cv = Cover(c, {0}, 10, 10);
    float worst = 0.0f;
    for (int y = 0; y < 10; ++y)
        for (int x = 0; x < 10; ++x)
        {
            const float a = BoxIn(1.3f, 1.3f, 6.4f, 6.4f, x, y), b = BoxIn(3.7f, 3.6f, 8.2f, 8.1f, x, y);
            const float both = BoxIn(3.7f, 3.6f, 6.4f, 6.4f, x, y);
            worst = std::max(worst, std::fabs(cv.At(x, y) - (a + b - both)));
        }
    ESIA_CHECK(worst < 1e-5f);
    // even-odd: the overlap is a hole
    const Coverage eo = Cover(c, {1}, 10, 10);
    worst = 0.0f;
    for (int y = 0; y < 10; ++y)
        for (int x = 0; x < 10; ++x)
        {
            const float a = BoxIn(1.3f, 1.3f, 6.4f, 6.4f, x, y), b = BoxIn(3.7f, 3.6f, 8.2f, 8.1f, x, y);
            const float both = BoxIn(3.7f, 3.6f, 6.4f, 6.4f, x, y);
            worst = std::max(worst, std::fabs(eo.At(x, y) - (a + b - 2.0f * both)));
        }
    ESIA_CHECK(worst < 1e-5f);
}

ESIA_TEST(SvgCoverage, CrossingsAndBothRules)
{
    // a pentagram (its edges cross four times a row), a bow tie and overlapping triangles, under both rules and as the
    // union of groups with rules of their own: within dense sampling's error of the exact area everywhere
    Contours c;
    const Vec2 o(8.3f, 8.1f);
    std::vector<Vec2> star;
    for (int k = 0; k < 5; ++k)
    {
        const float a = -kPi * 0.5f + (float)(k * 2 % 5) * kTau / 5.0f;
        star.push_back(o + Vec2(std::cos(a), std::sin(a)) * 7.2f);
    }
    c.Begin(0);
    c.pts.insert(c.pts.end(), star.begin(), star.end());
    Polygon(c, {Vec2(0.4f, 12.2f), Vec2(15.6f, 15.7f), Vec2(15.3f, 11.9f), Vec2(0.7f, 15.5f)});   // a bow tie
    for (const std::uint8_t rule : {std::uint8_t(0), std::uint8_t(1)})
    {
        const Coverage cv = Cover(c, {rule}, 17, 17);
        const Coverage ref = Sampled(c, {rule}, 17, 17, 64);
        ESIA_CHECK(MaxDiff(cv, ref) < 0.02f);
    }
    Contours u;
    Polygon(u, {Vec2(1.2f, 1.1f), Vec2(14.7f, 3.3f), Vec2(3.4f, 14.6f)}, 0);
    Polygon(u, {Vec2(1.2f, 1.1f), Vec2(14.7f, 3.3f), Vec2(3.4f, 14.6f)}, 0);   // twice: even-odd cancels it
    Polygon(u, {Vec2(14.6f, 14.8f), Vec2(2.1f, 9.3f), Vec2(12.7f, 0.9f)}, 1);
    const Coverage cv = Cover(u, {1, 0}, 16, 16);
    const Coverage ref = Sampled(u, {1, 0}, 16, 16, 64);
    ESIA_CHECK(MaxDiff(cv, ref) < 0.02f);
}

ESIA_TEST(SvgCoverage, OutsideTheCanvas)
{
    // contours past every side: what is inside is covered as if nothing were cut
    Contours c;
    Polygon(c, {Vec2(-30.0f, -4.5f), Vec2(6.5f, -9.0f), Vec2(40.0f, 7.25f), Vec2(-12.0f, 9.5f)});
    const Coverage cv = Cover(c, {0}, 12, 6);
    const Coverage ref = Sampled(c, {0}, 12, 6, 64);
    ESIA_CHECK(MaxDiff(cv, ref) < 0.02f);
}

ESIA_TEST(SvgCoverage, CircleArea)
{
    PathData p;
    AddEllipse(p, 10.37f, 9.81f, 7.3f, 7.3f);
    Contours c;
    FlattenFill(p, Affine(), kTolerance, 0, c);
    const Coverage cv = Cover(c, {0}, 21, 21);
    double sum = 0.0;
    for (const float v : cv.v)
        sum += v;
    // within the flattening's tolerance: the chords stay within 1/32 pixel of the circle
    ESIA_CHECK(std::fabs(sum - kPi * 7.3 * 7.3) < 2.0 * kPi * 7.3 * kTolerance);
}

ESIA_TEST(SvgStroke, RoundStrokeIsEveryPointNearThePath)
{
    // a round-joined, round-capped stroke is the set of points within half its width of the path: sharp and shallow
    // turns, a segment shorter than the width, a crossing, a cusp
    PathData p;
    ParsePathData("M2.3 3.1 L14.6 3.4 L4.2 9.7 L4.9 10.1 L16.2 15.3 L6.1 16.4 L12.4 1.6 L12.5 17.8", p);
    StrokeStyle st;
    st.width = 2.6f;
    st.cap = Cap::Round;
    st.join = Join::Round;
    Contours c;
    StrokeOutline(p, st, Affine(), kTolerance, 0, c);
    const Coverage cv = Cover(c, {0}, 20, 20);
    std::vector<Vec2> poly = {Vec2(2.3f, 3.1f), Vec2(14.6f, 3.4f), Vec2(4.2f, 9.7f), Vec2(4.9f, 10.1f),
                              Vec2(16.2f, 15.3f), Vec2(6.1f, 16.4f), Vec2(12.4f, 1.6f), Vec2(12.5f, 17.8f)};
    const float hw = 1.3f;
    Coverage ref{20, 20, std::vector<float>(400, 0.0f)};
    const int n = 48;
    for (int y = 0; y < 20; ++y)
        for (int x = 0; x < 20; ++x)
        {
            int in = 0;
            for (int sy = 0; sy < n; ++sy)
                for (int sx = 0; sx < n; ++sx)
                {
                    const Vec2 q((float)x + ((float)sx + 0.5f) / (float)n, (float)y + ((float)sy + 0.5f) / (float)n);
                    float best = 1e9f;
                    for (std::size_t k = 0; k + 1 < poly.size(); ++k)
                    {
                        const Vec2 ab = poly[k + 1] - poly[k], aq = q - poly[k];
                        const float t = Clamp(Dot(aq, ab) / Dot(ab, ab), 0.0f, 1.0f);
                        best = std::min(best, Length(aq - ab * t));
                    }
                    in += best <= hw ? 1 : 0;
                }
            ref.v[(std::size_t)y * 20 + (std::size_t)x] = (float)in / (float)(n * n);
        }
    ESIA_CHECK(MaxDiff(cv, ref) < 0.025f);
}

ESIA_TEST(SvgStroke, ThickStrokesOnTightOutlines)
{
    // strokes wider than the outline's curvature allows (the inner side folds): every point within half the width of the
    // outline, the folded inside filled - a circle's stroke becomes a disc, a rounded rectangle's corners fill in
    const auto check = [](const char* d, float hw, auto distance, int size) {
        PathData p;
        ParsePathData(d, p);
        StrokeStyle st;
        st.width = 2.0f * hw;
        Contours c;
        StrokeOutline(p, st, Affine(), kTolerance, 0, c);
        const Coverage cv = Cover(c, {0}, size, size);
        Coverage ref{size, size, std::vector<float>((std::size_t)size * (std::size_t)size, 0.0f)};
        const int n = 32;
        for (int y = 0; y < size; ++y)
            for (int x = 0; x < size; ++x)
            {
                int in = 0;
                for (int sy = 0; sy < n; ++sy)
                    for (int sx = 0; sx < n; ++sx)
                        in += distance(Vec2((float)x + ((float)sx + 0.5f) / (float)n, (float)y + ((float)sy + 0.5f) / (float)n)) <= hw ? 1 : 0;
                ref.v[(std::size_t)y * (std::size_t)size + (std::size_t)x] = (float)in / (float)(n * n);
            }
        ESIA_CHECK(MaxDiff(cv, ref) < 0.04f);
    };
    const auto circle = [](Vec2 c, float r) { return [c, r](Vec2 p) { return std::fabs(Length(p - c) - r); }; };
    check("M18.3 12.2 A6 6 0 1 1 6.3 12.2 A6 6 0 1 1 18.3 12.2 Z", 7.5f, circle(Vec2(12.3f, 12.2f), 6.0f), 28);   // a disc
    check("M18.3 12.2 A6 6 0 1 1 6.3 12.2 A6 6 0 1 1 18.3 12.2 Z", 3.5f, circle(Vec2(12.3f, 12.2f), 6.0f), 24);   // a ring
    // a rounded rectangle 16 x 10, corners of radius 2, its stroke 6 wide: the corners fold
    const auto box = [](Vec2 p) {
        const Vec2 q(std::fabs(p.x - 12.5f) - 8.0f + 2.0f, std::fabs(p.y - 11.5f) - 5.0f + 2.0f);
        const float outside = Length(Vec2(std::max(q.x, 0.0f), std::max(q.y, 0.0f))) + std::min(std::max(q.x, q.y), 0.0f) - 2.0f;
        return std::fabs(outside);
    };
    check("M6.5 6.5 H18.5 A2 2 0 0 1 20.5 8.5 V14.5 A2 2 0 0 1 18.5 16.5 H6.5 A2 2 0 0 1 4.5 14.5 V8.5 A2 2 0 0 1 6.5 6.5 Z", 3.0f, box, 25);
}

ESIA_TEST(SvgStroke, ButtSquareAndMiter)
{
    // a horizontal stroke 2 wide at y = 5: rows 4 and 5 covered from x = 2 to 10 (butt), 1 to 11 (square)
    for (const Cap cap : {Cap::Butt, Cap::Square})
    {
        PathData p;
        ParsePathData("M2 5 H10", p);
        StrokeStyle st;
        st.width = 2.0f;
        st.cap = cap;
        Contours c;
        StrokeOutline(p, st, Affine(), kTolerance, 0, c);
        const Coverage cv = Cover(c, {0}, 12, 8);
        const float x0 = cap == Cap::Butt ? 2.0f : 1.0f, x1 = cap == Cap::Butt ? 10.0f : 11.0f;
        float worst = 0.0f;
        for (int y = 0; y < 8; ++y)
            for (int x = 0; x < 12; ++x)
                worst = std::max(worst, std::fabs(cv.At(x, y) - BoxIn(x0, 4.0f, x1, 6.0f, x, y)));
        ESIA_CHECK(worst < 1e-5f);
    }
    // a closed square's mitered stroke: the outer square minus the inner one, exactly
    PathData p;
    ParsePathData("M3 3 H11 V11 H3 Z", p);
    StrokeStyle st;
    st.width = 2.0f;
    Contours c;
    StrokeOutline(p, st, Affine(), kTolerance, 0, c);
    const Coverage cv = Cover(c, {0}, 14, 14);
    float worst = 0.0f;
    for (int y = 0; y < 14; ++y)
        for (int x = 0; x < 14; ++x)
            worst = std::max(worst, std::fabs(cv.At(x, y) - (BoxIn(2.0f, 2.0f, 12.0f, 12.0f, x, y) - BoxIn(4.0f, 4.0f, 10.0f, 10.0f, x, y))));
    ESIA_CHECK(worst < 1e-5f);
}

ESIA_TEST(SvgStroke, DashesAndDots)
{
    // dashes 2 on, 2 off along x from 0, butt caps: pixel columns alternate
    const Image img = Raster("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 12 4'><path d='M0 2 H12' stroke='#000' stroke-width='2' "
                             "stroke-dasharray='2 2'/></svg>",
                             12, 4);
    for (int x = 0; x < 12; ++x)
        ESIA_CHECK(Px(img, x, 1)[3] == ((x / 2) % 2 == 0 ? 255 : 0));
    // dots every 4 along a line 8 long: at 0 and 4, none where the line ends (dashes start before it, SVG 2)
    const Image dots = Raster("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 12 4'><path d='M2 2 H10' stroke='#000' stroke-width='2' "
                              "stroke-dasharray='0 4' stroke-linecap='round'/></svg>",
                              12, 4);
    ESIA_CHECK(Px(dots, 1, 1)[3] > 100 && Px(dots, 5, 1)[3] > 100 && Px(dots, 9, 1)[3] == 0 && Px(dots, 10, 1)[3] == 0);
    // a zero-length subpath with round caps: a dot of the stroke's width
    const Image dot = Raster("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 20 20'><path d='M10 10 Z' stroke='#000' stroke-width='12' "
                             "stroke-linecap='round'/></svg>",
                             20, 20);
    double area = 0.0;
    for (int y = 0; y < 20; ++y)
        for (int x = 0; x < 20; ++x)
            area += Px(dot, x, y)[3] / 255.0;
    ESIA_CHECK(std::fabs(area - kPi * 36.0) < 1.0);
}

ESIA_TEST(SvgPaint, LinearAndRadialGradients)
{
    const Image img = Raster("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 8'><defs><linearGradient id='g'>"
                             "<stop offset='0' stop-color='#ff0000'/><stop offset='1' stop-color='#0000ff'/></linearGradient></defs>"
                             "<rect width='64' height='8' fill='url(#g)'/></svg>",
                             64, 8);
    for (const int x : {0, 13, 31, 50, 63})
    {
        const float t = ((float)x + 0.5f) / 64.0f;
        ESIA_CHECK(std::abs((int)Px(img, x, 4)[0] - (int)std::lround((1.0f - t) * 255.0f)) <= 1);
        ESIA_CHECK(std::abs((int)Px(img, x, 4)[2] - (int)std::lround(t * 255.0f)) <= 1);
        ESIA_CHECK(Px(img, x, 4)[3] == 255);
    }
    // radial: the center's color at the center, the edge's beyond r (pad), user space units
    const Image rad = Raster("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 32 32'><radialGradient id='r' gradientUnits='userSpaceOnUse' "
                             "cx='16' cy='16' r='8'><stop offset='0' stop-color='white'/><stop offset='1' stop-color='black'/></radialGradient>"
                             "<rect width='32' height='32' fill='url(#r)'/></svg>",
                             32, 32);
    ESIA_CHECK(std::abs((int)Px(rad, 16, 16)[0] - (int)std::lround((1.0f - std::sqrt(0.5f) / 8.0f) * 255.0f)) <= 1);
    ESIA_CHECK(Px(rad, 2, 2)[0] == 0);
    const float d = std::sqrt(2.0f * 4.5f * 4.5f);   // pixel (20, 20)'s center from (16, 16)
    ESIA_CHECK(std::abs((int)Px(rad, 20, 20)[0] - (int)std::lround((1.0f - d / 8.0f) * 255.0f)) <= 1);
    // reflect and repeat
    const Image rep = Raster("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 40 4'><linearGradient id='g' gradientUnits='userSpaceOnUse' x1='0' x2='10' "
                             "spreadMethod='reflect'><stop offset='0' stop-color='#000'/><stop offset='1' stop-color='#fff'/></linearGradient>"
                             "<rect width='40' height='4' fill='url(#g)'/></svg>",
                             40, 4);
    ESIA_CHECK(std::abs((int)Px(rep, 9, 1)[0] - (int)Px(rep, 10, 1)[0]) <= 1);   // mirrored about x = 10
    ESIA_CHECK(Px(rep, 19, 1)[0] < 20);
}

ESIA_TEST(SvgPaint, GroupOpacityIsALayer)
{
    // two opaque squares overlapping in a group at 50%: the overlap is 50% too, not 75%
    const Image img = Raster("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 12 12'><g opacity='0.5'>"
                             "<rect width='8' height='8' fill='#f00'/><rect x='4' y='4' width='8' height='8' fill='#00f'/></g></svg>",
                             12, 12);
    ESIA_CHECK(std::abs((int)Px(img, 6, 6)[3] - 128) <= 1);
    ESIA_CHECK(Px(img, 6, 6)[2] == 255 && Px(img, 6, 6)[0] == 0);
    ESIA_CHECK(std::abs((int)Px(img, 1, 1)[3] - 128) <= 1);
    // fill-opacity on its own is not a layer: overlapping shapes add up
    const Image two = Raster("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 12 12'><rect width='8' height='8' fill-opacity='0.5'/>"
                             "<rect x='4' y='4' width='8' height='8' fill-opacity='0.5'/></svg>",
                             12, 12);
    ESIA_CHECK(std::abs((int)Px(two, 6, 6)[3] - 191) <= 1);
}

ESIA_TEST(SvgPaint, ClipPaths)
{
    const Image img = Raster("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 20 20'><clipPath id='c'><circle cx='10' cy='10' r='6'/></clipPath>"
                             "<rect width='20' height='20' fill='#0a0' clip-path='url(#c)'/></svg>",
                             20, 20);
    ESIA_CHECK(Px(img, 10, 10)[3] == 255);
    ESIA_CHECK(Px(img, 1, 1)[3] == 0);
    ESIA_CHECK(Px(img, 18, 10)[3] == 0);
    double area = 0.0;
    for (int y = 0; y < 20; ++y)
        for (int x = 0; x < 20; ++x)
            area += Px(img, x, y)[3] / 255.0;
    ESIA_CHECK(std::fabs(area - kPi * 36.0) < 0.6);
    // in bounding-box units, on a group
    const Image box = Raster("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 20 20'><clipPath id='c' clipPathUnits='objectBoundingBox'>"
                             "<rect width='0.5' height='1'/></clipPath><g clip-path='url(#c)'><rect x='4' y='4' width='12' height='12'/></g></svg>",
                             20, 20);
    ESIA_CHECK(Px(box, 5, 10)[3] == 255);
    ESIA_CHECK(Px(box, 9, 10)[3] == 255);
    ESIA_CHECK(Px(box, 10, 10)[3] == 0);
}

ESIA_TEST(SvgPaint, MaskModeAndTexelColors)
{
    const std::string svg = "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 8 8'><rect x='2' y='2' width='4' height='4' fill='#c03020' "
                            "fill-opacity='0.5'/></svg>";
    RasterOptions o;
    o.mask = true;
    const Image mask = Raster(svg, 8, 8, o);
    ESIA_CHECK(Px(mask, 3, 3)[0] == 255 && Px(mask, 3, 3)[1] == 255 && std::abs((int)Px(mask, 3, 3)[3] - 128) <= 1);
    ESIA_CHECK(Px(mask, 0, 0)[0] == 255 && Px(mask, 0, 0)[3] == 0);
    // transparent texels next to the drawing carry its color (filtering does not darken the edge)
    const Image img = Raster(svg, 8, 8);
    ESIA_CHECK(Px(img, 1, 3)[3] == 0 && Px(img, 1, 3)[0] == 0xC0 && Px(img, 1, 3)[2] == 0x20);
    ESIA_CHECK(Px(img, 7, 7)[0] == 0);   // far from it: untouched
    o = RasterOptions();
    o.border = 2;
    const Image bordered = Raster(svg, 8, 8, o);
    ESIA_CHECK(bordered.width == 12 && bordered.height == 12);
    ESIA_CHECK(Px(bordered, 5, 5)[3] == Px(img, 3, 3)[3]);
}

ESIA_TEST(SvgAtlas, CachesBySizeAndColor)
{
    TextureRegistry textures;
    Atlas atlas(textures);
    const Document mono = Document::Parse("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'><path d='M4 4h16v16H4z' fill='currentColor'/></svg>");
    const Document colored = Document::Parse("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'><path d='M4 4h16v16H4z' fill='#08f'/></svg>");
    ESIA_CHECK(mono.Monochrome() && mono.UsesCurrentColor());
    ESIA_CHECK(!colored.Monochrome() && !colored.UsesCurrentColor());
    atlas.BeginFrame();
    RasterOptions m;
    m.mask = true;
    const Atlas::Entry* a = atlas.Get(mono, 24, 24, m);
    ESIA_CHECK(a && a->texture != 0 && a->uv1.x > a->uv0.x);
    ESIA_CHECK(atlas.Get(mono, 24, 24, m) && atlas.Rasterized() == 1);   // cached
    ESIA_CHECK(atlas.Get(mono, 48, 48, m) && atlas.Rasterized() == 2);   // another size
    RasterOptions c;
    c.currentColor = Color(1.0f, 0.0f, 0.0f);
    ESIA_CHECK(atlas.Get(colored, 24, 24, c) && atlas.Rasterized() == 3);
    c.currentColor = Color(0.0f, 1.0f, 0.0f);
    ESIA_CHECK(atlas.Get(colored, 24, 24, c) && atlas.Rasterized() == 3);   // currentColor paints nothing in it
    ESIA_CHECK(atlas.Get(Document(), 24, 24) == nullptr);
    // the texels it uploaded: the drawing inside its one-texel border
    TextureInfo info;
    ESIA_CHECK(textures.Info(a->texture, info) && info.format == TextureFormat::RGBA8);
}

ESIA_TEST(SvgAtlas, DrawOnThePixelGrid)
{
    TextureRegistry textures;
    Atlas atlas(textures);
    DrawList dl;
    dl.Reset(Rect(0, 0, 400, 300));
    PainterEnv env;
    env.pixelScale = 2.0f;
    Painter p(dl, env);
    const Document mono = Document::Parse("<svg viewBox='0 0 24 24'><path d='M4 4h16v16H4z' fill='currentColor'/></svg>");
    atlas.BeginFrame();
    // (10.3, 20.2) is (20.6, 40.4) pixels: drawn from pixel (21, 40), 24 x 24 pixels, an image quad in the color
    Draw(p, atlas, mono, Rect(10.3f, 20.2f, 22.3f, 32.2f), Color(1.0f, 0.0f, 0.0f, 0.5f));
    ESIA_CHECK(atlas.Rasterized() == 1);
    ESIA_CHECK(dl.Commands().size() == 1 && dl.Commands()[0].texture != 0);
    ESIA_CHECK(dl.Vertices().size() == 4);
    float x0 = 1e9f, y0 = 1e9f, x1 = -1e9f, y1 = -1e9f;
    for (const Vertex& v : dl.Vertices())
    {
        x0 = std::min(x0, v.pos.x), y0 = std::min(y0, v.pos.y), x1 = std::max(x1, v.pos.x), y1 = std::max(y1, v.pos.y);
        ESIA_CHECK(v.color == Color(1.0f, 0.0f, 0.0f, 0.5f).ToRgba8());
    }
    ESIA_CHECK(x0 == 10.5f && y0 == 20.0f && x1 == 22.5f && y1 == 32.0f);
    // another color: the same mask
    Draw(p, atlas, mono, Rect(40.0f, 20.0f, 52.0f, 32.0f), Color(0.0f, 0.5f, 1.0f));
    ESIA_CHECK(atlas.Rasterized() == 1 && dl.Vertices().size() == 8);
}

ESIA_TEST(SvgCoverage, ViewportAtFractionsOfAPixel)
{
    // fitted into (0.25, 0.5) - (10.25, 10.5) of 12 x 12 pixels, a square filling its viewBox covers in every pixel the
    // area of that box in it; so do a square reaching far beyond its viewBox and a stroke wider than all of it: cut off
    // at the viewport's sides exactly
    RasterOptions o;
    o.viewport = Rect(0.25f, 0.5f, 10.25f, 10.5f);
    for (const char* svg : {"<svg viewBox='0 0 10 10'><rect width='10' height='10'/></svg>",
                            "<svg viewBox='0 0 10 10'><rect x='-5' y='-5' width='20' height='20'/></svg>",
                            "<svg viewBox='0 0 10 10'><path d='M0 0h10v10H0z' fill='none' stroke='#000' stroke-width='30'/></svg>"})
    {
        const Image img = Raster(svg, 12, 12, o);
        float worst = 0.0f;
        for (int y = 0; y < 12; ++y)
            for (int x = 0; x < 12; ++x)
                worst = std::max(worst, std::fabs((float)Px(img, x, y)[3] / 255.0f - BoxIn(0.25f, 0.5f, 10.25f, 10.5f, x, y)));
        ESIA_CHECK(worst <= 0.6f / 255.0f);
    }
    // a wide viewport: the square viewBox in its middle (xMidYMid meet), 10 x 10 from x = 5.5
    o.viewport = Rect(0.5f, 0.5f, 20.5f, 10.5f);
    const Image wide = Raster("<svg viewBox='0 0 10 10'><rect width='10' height='10'/></svg>", 22, 12, o);
    float worst = 0.0f;
    for (int y = 0; y < 12; ++y)
        for (int x = 0; x < 22; ++x)
            worst = std::max(worst, std::fabs((float)Px(wide, x, y)[3] / 255.0f - BoxIn(5.5f, 0.5f, 15.5f, 10.5f, x, y)));
    ESIA_CHECK(worst <= 0.6f / 255.0f);
    // the whole image as the viewport: the same pixels as none
    o.viewport = Rect(0.0f, 0.0f, 12.0f, 12.0f);
    const char* circle = "<svg viewBox='0 0 10 10'><circle cx='5' cy='5' r='4.3' fill='#08f' stroke='#f80'/></svg>";
    ESIA_CHECK(Raster(circle, 12, 12, o).pixels == Raster(circle, 12, 12).pixels);
    o.viewport = Rect(0.0f, 0.0f, std::nanf(""), 12.0f);
    Image img;
    ESIA_CHECK(!Rasterize(Document::Parse(circle), 12, 12, o, img));
}

ESIA_TEST(SvgAtlas, DrawnWhereTheScalePutsIt)
{
    TextureRegistry textures;
    Atlas atlas(textures);
    DrawList dl;
    dl.Reset(Rect(0, 0, 400, 300));
    Painter p(dl, PainterEnv());
    const Document mono = Document::Parse("<svg viewBox='0 0 24 24'><path d='M4 4h16v16H4z' fill='currentColor'/></svg>");
    RasterOptions m;
    m.mask = true;
    m.currentColor = Color::White();
    const auto bounds = [&](std::size_t first) {
        Rect b(1e9f, 1e9f, -1e9f, -1e9f);
        for (std::size_t i = first; i < dl.Vertices().size(); ++i)
            b = Rect(std::min(b.min.x, dl.Vertices()[i].pos.x), std::min(b.min.y, dl.Vertices()[i].pos.y), std::max(b.max.x, dl.Vertices()[i].pos.x),
                     std::max(b.max.y, dl.Vertices()[i].pos.y));
        return b;
    };
    atlas.BeginFrame();
    // 1.5 times around (0, 0): (10, 10) - (34, 34) goes to (15, 15) - (51, 51), whole pixels - rasterized at 36 x 36
    // there, and PopScale leaves the image where it is
    p.PushScale(Vec2(0, 0), 1.5f);
    Draw(p, atlas, mono, Rect(10, 10, 34, 34), Color::White());
    p.PopScale();
    ESIA_CHECK(atlas.Rasterized() == 1 && bounds(0) == Rect(15, 15, 51, 51));
    ESIA_CHECK(atlas.Get(mono, 36, 36, m) && atlas.Rasterized() == 1);
    // 1.25 times: (12.5, 12.5) - (42.5, 42.5) - the drawing at those fractions, in pixels (12, 12) - (43, 43)
    p.PushScale(Vec2(0, 0), 1.25f);
    Draw(p, atlas, mono, Rect(10, 10, 34, 34), Color::White());
    p.PopScale();
    ESIA_CHECK(atlas.Rasterized() == 2 && bounds(4) == Rect(12, 12, 43, 43));
    m.viewport = Rect(0.5f, 0.5f, 30.5f, 30.5f);
    ESIA_CHECK(atlas.Get(mono, 31, 31, m) && atlas.Rasterized() == 2);
    // the next frame at the same scale: from the cache
    atlas.BeginFrame();
    p.PushScale(Vec2(0, 0), 1.25f);
    Draw(p, atlas, mono, Rect(10, 10, 34, 34), Color::White());
    p.PopScale();
    ESIA_CHECK(atlas.Rasterized() == 2 && bounds(8) == Rect(12, 12, 43, 43));
    // a scale of 1 is no scale: whole pixels
    p.PushScale(Vec2(5, 5), 1.0f);
    Draw(p, atlas, mono, Rect(10.3f, 10.0f, 34.3f, 34.0f), Color::White());
    p.PopScale();
    ESIA_CHECK(atlas.Rasterized() == 3 && bounds(12) == Rect(10, 10, 34, 34));
}

ESIA_TEST(SvgAtlas, AnimatedSizesLeaveTheLastingPagesAlone)
{
    // small pages; a drawing at a new size every frame (20 to 99 pixels: the recent pages fill and start over again and
    // again) beside one that stays: the one that stays is rasterized once
    TextureRegistry textures;
    Atlas atlas(textures, 128, 1);
    const Document still = Document::Parse("<svg viewBox='0 0 24 24'><path d='M4 4h16v16H4z' fill='#08f'/></svg>");
    const Document moving = Document::Parse("<svg viewBox='0 0 24 24'><circle cx='12' cy='12' r='10' fill='#f80'/></svg>");
    bool drawn = true;
    int stillAgain = 0;
    for (int frame = 0; frame < 200; ++frame)
    {
        atlas.BeginFrame();
        const int before = atlas.Rasterized();
        drawn = drawn && atlas.Get(still, 32, 32);
        if (frame > 0)
            stillAgain += atlas.Rasterized() - before;
        const int size = 20 + frame % 80;
        drawn = drawn && atlas.Get(moving, size, size) && atlas.Get(still, 32, 32);
    }
    ESIA_CHECK(drawn && stillAgain == 0);
    // an animation that comes back to the same sizes finds them among the recent drawings
    Atlas big(textures);
    for (int frame = 0; frame < 40; ++frame)
    {
        big.BeginFrame();
        ESIA_CHECK(big.Get(moving, 24 + frame % 4 * 8, 24 + frame % 4 * 8));
    }
    ESIA_CHECK(big.Rasterized() == 4);
}

ESIA_TEST(SvgAtlas, LargerThanAPage)
{
    TextureRegistry textures;
    Atlas atlas(textures, 64, 1);
    const Document doc = Document::Parse("<svg viewBox='0 0 24 24'><path d='M4 4h16v16H4z' fill='#08f'/></svg>");
    atlas.BeginFrame();
    const Atlas::Entry* e = atlas.Get(doc, 200, 100);
    ESIA_CHECK(e != nullptr);
    if (!e)
        return;
    // a texture of its own, the drawing inside its border
    const TextureId texture = e->texture;
    TextureInfo info;
    ESIA_CHECK(textures.Info(texture, info) && info.width == 202 && info.height == 102 && info.format == TextureFormat::RGBA8);
    ESIA_CHECK(std::fabs(e->uv0.x - 1.0f / 202.0f) < 1e-6f && std::fabs(e->uv1.y - 101.0f / 102.0f) < 1e-6f);
    // kept while it is drawn, gone once a frame did not draw it
    atlas.BeginFrame();
    e = atlas.Get(doc, 200, 100);
    ESIA_CHECK(e && e->texture == texture && atlas.Rasterized() == 1);
    atlas.BeginFrame();
    ESIA_CHECK(textures.Info(texture, info));
    atlas.BeginFrame();
    ESIA_CHECK(!textures.Info(texture, info));
}

namespace
{
    // curves, an even-odd hole, a dashed round stroke over them, a crisp square
    const char* const kPartSvg0 =
        "<svg viewBox='0 0 40 30'><path d='M3 4 C 20 -6 44 10 30 26 S 4 30 3 4 Z M12 10 h10 v8 h-10 Z' fill='#e04' fill-rule='evenodd'/>"
        "<path d='M2 15 Q 20 2 38 15 T 2 15' fill='none' stroke='#08f' stroke-width='2.5' stroke-dasharray='3 2' stroke-linecap='round'/>"
        "<rect x='31.3' y='3.7' width='5' height='5' fill='currentColor' shape-rendering='crispEdges'/></svg>";
    // gradients (a radial one with its focus off its center), a group's opacity and clip path, a nested clip
    const char* const kPartSvg1 =
        "<svg viewBox='0 0 40 30'><defs><linearGradient id='l' x1='0' y1='0' x2='1' y2='1'><stop offset='0' stop-color='#f80'/>"
        "<stop offset='1' stop-color='#08f' stop-opacity='0.5'/></linearGradient><radialGradient id='r' fx='0.3' fy='0.35'>"
        "<stop offset='0' stop-color='#fff'/><stop offset='0.6' stop-color='#4c4'/><stop offset='1' stop-color='#000' stop-opacity='0'/>"
        "</radialGradient><clipPath id='c'><circle cx='20' cy='15' r='13'/></clipPath><clipPath id='d' clip-path='url(#c)'>"
        "<rect x='9' y='0' width='30' height='24'/></clipPath></defs><rect width='40' height='30' fill='url(#l)'/>"
        "<g opacity='0.7' clip-path='url(#c)'><rect x='4' y='2' width='32' height='26' fill='url(#r)'/><circle cx='26' cy='12' r='7' "
        "fill='#c0f'/></g><path d='M0 30 L40 0' stroke='#ff0' stroke-width='3' clip-path='url(#d)'/></svg>";
}

// The image of a part of a drawing - the viewport moved by the part's corner - has the pixels the image of all of it has
// there, bit for bit: each pixel's coverage comes from the same row of the same sweep (the rows above the part are
// skipped, the columns beside it not painted), a gradient's color from the same place. With a border, the texels around
// the part are the drawing's there, so even the transparent texels' colors along its edges are the whole image's.
ESIA_TEST(SvgCoverage, PartsHaveTheWholeImagesPixels)
{
    const int W = 77, H = 58;
    const int parts[][4] = {{0, 0, 20, 15}, {13, 7, 50, 40}, {30, 21, 77, 58}, {-5, 40, 30, 70}, {60, -3, 90, 9}, {5, 5, 6, 6}, {-9, -9, -2, 70}};
    int checked = 0, differ = 0;
    for (const char* svg : {kPartSvg0, kPartSvg1})
    {
        const Document doc = Document::Parse(svg);
        for (const bool mask : {false, true})
            for (const Rect vp : {Rect(), Rect(0.375f, 0.25f, 76.625f, 57.75f), Rect(-12.5f, 3.0f, 70.25f, 58.0f)})
            {
                RasterOptions o;
                o.mask = mask;
                o.currentColor = Color(0.2f, 0.7f, 0.4f, 0.9f);
                o.border = 1;
                o.viewport = vp;
                // the whole image in its frame - for a viewport that is its frame (none, or the fractions of a pixel
                // svg::Draw leaves) Rasterize's pixels
                Image whole, plain;
                ESIA_CHECK(detail::RasterizePart(doc, W, H, o, whole));
                if (vp.min.x >= 0.0f && vp.min.y >= 0.0f)
                    ESIA_CHECK(Rasterize(doc, W, H, o, plain) && plain.pixels == whole.pixels);
                const Rect all = vp.Empty() ? Rect(0.0f, 0.0f, (float)W, (float)H) : vp;
                for (const auto& p : parts)
                {
                    const int pw = p[2] - p[0], ph = p[3] - p[1];
                    RasterOptions po = o;
                    po.viewport = Rect(all.min.x - (float)p[0], all.min.y - (float)p[1], all.max.x - (float)p[0], all.max.y - (float)p[1]);
                    Image part;
                    ESIA_CHECK(detail::RasterizePart(doc, pw, ph, po, part) && part.width == pw + 2 && part.height == ph + 2);
                    // every texel against the whole image's at the same place: alpha everywhere, the color of the texels
                    // with ink and of all inside both borders. Beyond the whole image (and its border) the part shows
                    // what the viewport puts there: nothing outside the whole pixels around it.
                    const float fx0 = std::floor(all.min.x), fy0 = std::floor(all.min.y), fx1 = std::ceil(all.max.x), fy1 = std::ceil(all.max.y);
                    for (int y = 0; y < part.height; ++y)
                        for (int x = 0; x < part.width; ++x)
                        {
                            const int wx = x + p[0], wy = y + p[1];   // in the whole image, its border included
                            const std::uint8_t* a = Px(part, x, y);
                            const bool in = wx >= 0 && wy >= 0 && wx < whole.width && wy < whole.height;
                            ++checked;
                            if (!in)
                            {
                                const float dx = (float)(wx - 1), dy = (float)(wy - 1);   // in the drawing's pixels
                                if (dx < fx0 || dy < fy0 || dx >= fx1 || dy >= fy1)
                                    differ += a[3] != 0 ? 1 : 0;
                                continue;
                            }
                            const std::uint8_t* b = Px(whole, wx, wy);
                            const bool inside = x >= 1 && y >= 1 && x <= pw && y <= ph && wx >= 1 && wy >= 1 && wx <= W && wy <= H;
                            differ += a[3] != b[3] || ((inside || a[3] != 0) && std::memcmp(a, b, 3) != 0) ? 1 : 0;
                        }
                }
            }
    }
    ESIA_CHECK(checked > 70000 && differ == 0);
    if (differ)
        std::printf("    %d of %d texels differ\n", differ, checked);
}

// In its frame, a viewport moved by whole pixels gives the same pixels moved by as many, wherever it lies in the image.
ESIA_TEST(SvgCoverage, ViewportMovedByWholePixels)
{
    const Document doc = Document::Parse(kPartSvg1);
    RasterOptions o;
    o.viewport = Rect(3.25f, 5.5f, 33.25f, 31.5f);
    Image a, b;
    ESIA_CHECK(detail::RasterizePart(doc, 40, 36, o, a));
    o.viewport = Rect(3.25f + 17.0f, 5.5f + 20.0f, 33.25f + 17.0f, 31.5f + 20.0f);
    ESIA_CHECK(detail::RasterizePart(doc, 70, 66, o, b));
    bool same = true;
    int ink = 0;
    for (int y = 0; y < 66; ++y)
        for (int x = 0; x < 70; ++x)
        {
            const int ax = x - 17, ay = y - 20;
            const bool inA = ax >= 0 && ay >= 0 && ax < 40 && ay < 36;
            ink += Px(b, x, y)[3] != 0 ? 1 : 0;
            same = same && (inA ? std::memcmp(Px(a, ax, ay), Px(b, x, y), 4) == 0 : Px(b, x, y)[3] == 0);
        }
    ESIA_CHECK(same && ink > 500);
}

namespace
{
    // A text system's pages as the FreeType system keeps them: glyphs (here one, every character's) and coverage tiles
    // in one Alpha8 text::GlyphAtlas, tiles under keys with the top bit set, gone when it starts over.
    struct PageText final : text::TextSystem
    {
        text::GlyphAtlas atlas;
        const text::GlyphSlot* glyph = nullptr;
        int added = 0;                       // tiles added
        std::vector<std::uint8_t> lastTile;  // the last one's coverage
        const text::GlyphSlot* lastSlot = nullptr;
        explicit PageText(TextureRegistry& textures) : atlas(textures, text::GlyphAtlasDesc{512, 2, 1}) { AddGlyph(); }
        void AddGlyph()
        {
            text::GlyphBitmap g;
            g.width = 6;
            g.height = 10;
            g.pixels.assign(60, 200);
            glyph = atlas.Add(1, g, Rect(0.0f, 0.0f, 6.0f, 10.0f));
        }
        text::FontId AddFontFile(const char*, int) override { return 1; }
        text::FontId AddFontMemory(const void*, std::size_t, int) override { return 1; }
        void AddFallback(text::FontId) override {}
        void NewFrame(const text::RasterParams&) override
        {
            atlas.BeginFrame();
            if (!atlas.Find(1))
                AddGlyph();
        }
        text::TextMetrics Measure(text::FontRef, std::string_view s, float, std::uint32_t) override { return {Vec2(6.0f * (float)s.size(), 10.0f), 8.0f, 1}; }
        Vec2 Draw(DrawList& dl, text::FontRef f, Vec2 pos, Color c, std::string_view s, float, std::uint32_t, float) override
        {
            for (std::size_t i = 0; i < s.size(); ++i)
                dl.AddQuad(glyph->page, Rect::FromSize(pos + Vec2(6.0f * (float)i, 0.0f), Vec2(6.0f, 10.0f)), glyph->uv0, glyph->uv1, c.ToRgba8());
            return Measure(f, s, 0.0f, 0).size;
        }
        void DrawGlyph(DrawList&, text::FontRef, char32_t, Vec2, Color) override {}
        const text::GlyphSlot* FindTile(std::uint64_t key) override { return atlas.Find(key | (1ull << 63)); }
        const text::GlyphSlot* AddTile(std::uint64_t key, const text::GlyphBitmap& b) override
        {
            ++added;
            lastTile = b.pixels;
            return lastSlot = atlas.Add(key | (1ull << 63), b, Rect());
        }
        std::uint64_t TileGeneration() override { return (std::uint64_t)atlas.Resets() + 1u; }
    };

    const char* const kIconSvg = "<svg viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='1.5' stroke-linecap='round' "
                                 "stroke-linejoin='round'><rect x='3' y='3' width='18' height='18' rx='5'/><path d='M8 12.5l2.5 2.5L16 9'/></svg>";

    int CoverageQuads(const DrawList& dl)
    {
        int n = 0;
        for (const QuadInstance& q : dl.Quads())
            n += q.IsCoverage() ? 1 : 0;
        return n;
    }
}

// A monochrome icon that lasts - drawn in the frame before too - is a tile of the text's pages: a plain-coverage quad in
// the command of the labels around it, its coverage the mask's alpha (from the pixels it was made with, not rasterized
// again). When the text's pages start over the tile goes with them: an image for a frame, then a tile again.
ESIA_TEST(SvgAtlas, LastingIconsAreTilesOfTheText)
{
    TextureRegistry textures;
    PageText text(textures);
    Atlas atlas(textures);
    const Document icon = Document::Parse(kIconSvg);
    ESIA_CHECK(icon.Monochrome());
    PainterEnv env;
    env.text = &text;
    env.pixelScale = 1.5f;
    const Color tint(0.9f, 0.3f, 0.2f, 0.8f);
    DrawList dl;
    const auto frame = [&] {
        atlas.BeginFrame();
        text.NewFrame({1.5f});
        dl.Reset(Rect(0, 0, 400, 300));
        Painter p(dl, env);
        p.Text(Vec2(40, 20), text::FontRef{1, 10.0f}, Color::White(), "label");
        Draw(p, atlas, icon, Rect(10.0f, 20.0f, 26.0f, 36.0f), tint);   // 24 x 24 pixels from (15, 30)
        p.Text(Vec2(40, 40), text::FontRef{1, 10.0f}, Color::White(), "more");
    };
    // new: an image of the recent pages, between the labels' commands
    frame();
    ESIA_CHECK(text.added == 0 && atlas.Rasterized() == 1 && dl.Vertices().size() == 4 && dl.Commands().size() == 3);
    // drawn again: a tile - one command with the labels
    frame();
    ESIA_CHECK(text.added == 1 && atlas.Rasterized() == 1 && dl.Vertices().empty());
    ESIA_CHECK(dl.Commands().size() == 1 && dl.Commands()[0].shapes && dl.Quads().size() == 10 && CoverageQuads(dl) == 1);
    if (dl.Quads().size() == 10 && text.lastSlot)
    {
        // where the image was, the tile's texels, in the color
        QuadInstance want;
        want.SetCoverage(Rect(10.0f, 20.0f, 26.0f, 36.0f), text.lastSlot->uv0, text.lastSlot->uv1, tint.ToRgba8());
        ESIA_CHECK(std::memcmp(&dl.Quads()[5], &want, sizeof(want)) == 0 && text.lastSlot->page == text.glyph->page);
    }
    // the tile's coverage: the mask's alpha, texel for texel
    RasterOptions m;
    m.mask = true;
    m.currentColor = Color::White();
    Image ref;
    ESIA_CHECK(Rasterize(icon, 24, 24, m, ref) && text.lastTile.size() == 24 * 24);
    bool same = text.lastTile.size() == 24 * 24;
    for (std::size_t i = 0; same && i < text.lastTile.size(); ++i)
        same = text.lastTile[i] == ref.pixels[i * 4 + 3];
    ESIA_CHECK(same);
    // found from then on
    frame();
    frame();
    ESIA_CHECK(text.added == 1 && atlas.Rasterized() == 1 && CoverageQuads(dl) == 1 && dl.Commands().size() == 1);
    // the text's pages start over: an image (from the recent pages) for a frame, then a tile again - rasterized again,
    // its pixels are not kept
    text.atlas.StartOver();
    frame();
    ESIA_CHECK(text.added == 1 && CoverageQuads(dl) == 0 && dl.Vertices().size() == 4 && atlas.Rasterized() == 1);
    frame();
    ESIA_CHECK(text.added == 2 && CoverageQuads(dl) == 1 && dl.Vertices().empty() && atlas.Rasterized() == 2);
    same = text.lastTile.size() == 24 * 24;
    for (std::size_t i = 0; same && i < text.lastTile.size(); ++i)
        same = text.lastTile[i] == ref.pixels[i * 4 + 3];
    ESIA_CHECK(same);
}

// What changes every frame stays out of the text's pages: an icon whose size animates, one under an animated scale, a
// drawing two frames alternate between. The text's atlas never starts over for them.
ESIA_TEST(SvgAtlas, AnimationsStayOutOfTheTextsPages)
{
    TextureRegistry textures;
    PageText text(textures);
    Atlas atlas(textures);
    const Document sized = Document::Parse(kIconSvg), scaled = Document::Parse(kIconSvg), turns = Document::Parse(kIconSvg);
    PainterEnv env;
    env.text = &text;
    for (int f = 0; f < 120; ++f)
    {
        atlas.BeginFrame();
        text.NewFrame({1.0f});
        DrawList dl;
        dl.Reset(Rect(0, 0, 400, 300));
        Painter p(dl, env);
        const float s = 16.0f + (float)(f % 40);   // a new size every frame, the same again 40 frames later
        Draw(p, atlas, sized, Rect(10.0f, 10.0f, 10.0f + s, 10.0f + s), Color::White());
        p.PushScale(Vec2(200, 100), 1.0f + 0.01f * (float)f);
        Draw(p, atlas, scaled, Rect(190.0f, 90.0f, 214.0f, 114.0f), Color::White());
        p.PopScale();
        const float alt = f % 2 ? 30.0f : 32.0f;   // two sizes in turn
        Draw(p, atlas, turns, Rect(300.0f, 10.0f, 300.0f + alt, 10.0f + alt), Color::White());
    }
    ESIA_CHECK(text.added == 0 && text.atlas.Resets() == 0);
}

// Tiles only where they draw what the image draws: not for a colored drawing, a mask larger than kMaxTile, one under a
// Painter mask, or with a text system without tiles - those stay images (rasterized once). Under a scale at rest an icon
// is a tile, which PopScale leaves where the scale put it.
ESIA_TEST(SvgAtlas, TilesWhereTheyDrawAsTheImage)
{
    TextureRegistry textures;
    const Document icon = Document::Parse(kIconSvg);
    const Document colored = Document::Parse("<svg viewBox='0 0 24 24'><circle cx='12' cy='12' r='9' fill='#08f'/></svg>");
    // a text system without tiles
    struct NoTiles final : text::TextSystem
    {
        text::FontId AddFontFile(const char*, int) override { return 1; }
        text::FontId AddFontMemory(const void*, std::size_t, int) override { return 1; }
        void AddFallback(text::FontId) override {}
        void NewFrame(const text::RasterParams&) override {}
        text::TextMetrics Measure(text::FontRef, std::string_view, float, std::uint32_t) override { return {}; }
        Vec2 Draw(DrawList&, text::FontRef, Vec2, Color, std::string_view, float, std::uint32_t, float) override { return Vec2(0, 0); }
        void DrawGlyph(DrawList&, text::FontRef, char32_t, Vec2, Color) override {}
    } noTiles;
    enum Kind { Pages, NoTileSystem, NoText };
    // three frames of `draw` with an atlas and text pages of their own: the last frame's coverage quads, and how many times
    // the atlas rasterized
    const auto frames = [&](Kind kind, auto draw) {
        PageText text(textures);
        Atlas atlas(textures);
        PainterEnv env;
        env.text = kind == Pages ? &text : kind == NoTileSystem ? static_cast<text::TextSystem*>(&noTiles) : nullptr;
        DrawList dl;
        for (int f = 0; f < 3; ++f)
        {
            atlas.BeginFrame();
            text.NewFrame({1.0f});
            dl.Reset(Rect(0, 0, 400, 300));
            Painter p(dl, env);
            draw(p, atlas);
        }
        return std::make_pair(CoverageQuads(dl), atlas.Rasterized());
    };
    auto r = frames(Pages, [&](Painter& p, Atlas& a) { Draw(p, a, colored, Rect(10, 10, 34, 34), Color::White()); });
    ESIA_CHECK(r.first == 0 && r.second == 1);
    r = frames(Pages, [&](Painter& p, Atlas& a) { Draw(p, a, icon, Rect(10, 10, 139, 139), Color::White()); });   // 129 pixels
    ESIA_CHECK(r.first == 0 && r.second == 1);
    r = frames(Pages, [&](Painter& p, Atlas& a) { Draw(p, a, icon, Rect(10, 10, 138, 138), Color::White()); });   // 128
    ESIA_CHECK(r.first == 1 && r.second == 1);
    r = frames(Pages, [&](Painter& p, Atlas& a) { Draw(p, a, colored, Rect(10, 10, 34, 34), Color::White(), true); });   // as a mask
    ESIA_CHECK(r.first == 1 && r.second == 1);
    // under a mask: an image (an FX shape, which takes the mask); the same icon outside it a tile - from the pixels the
    // first frame made, not rasterized again
    r = frames(Pages, [&](Painter& p, Atlas& a) {
        p.PushMask(Rect(0, 0, 200, 200), 24.0f);
        Draw(p, a, icon, Rect(60, 60, 84, 84), Color::White());
        p.PopMask();
        ESIA_CHECK(p.GetDrawList().FxInstances().size() == 1);
        Draw(p, a, icon, Rect(260, 60, 284, 84), Color::White());
    });
    ESIA_CHECK(r.first == 1 && r.second == 1);
    // under a scale at rest: the quad where the scale puts the drawing (to 1/64 pixel, rasterized there), PopScale leaves it
    Rect at;
    r = frames(Pages, [&](Painter& p, Atlas& a) {
        p.PushScale(Vec2(0, 0), 1.25f);
        Draw(p, a, icon, Rect(10, 10, 34, 34), Color::White());
        p.PopScale();
        const QuadVector& q = p.GetDrawList().Quads();
        if (!q.empty())
            at = Rect(q.back().rect[0], q.back().rect[1], q.back().rect[2], q.back().rect[3]);
    });
    ESIA_CHECK(r.first == 1 && r.second == 1 && at == Rect(12.0f, 12.0f, 43.0f, 43.0f));
    // without tiles, or without text: images, rasterized once
    r = frames(NoTileSystem, [&](Painter& p, Atlas& a) { Draw(p, a, icon, Rect(10, 10, 34, 34), Color::White()); });
    ESIA_CHECK(r.first == 0 && r.second == 1);
    r = frames(NoText, [&](Painter& p, Atlas& a) { Draw(p, a, icon, Rect(10, 10, 34, 34), Color::White()); });
    ESIA_CHECK(r.first == 0 && r.second == 1);
}

// Only what the clip rect shows is rasterized: a drawing outside it not at all; a large one reaching beyond it only in
// the steps of 256 pixels that hold the part inside - found again while it moves less than a step - when that is at most
// half of it; a drawing mostly inside, whole.
ESIA_TEST(SvgAtlas, OnlyWhatTheClipShows)
{
    TextureRegistry textures;
    Atlas atlas(textures);
    const Document doc = Document::Parse(kPartSvg1);   // 40 x 30: drawn 1200 x 900 below
    RasterOptions o;
    o.currentColor = Color::White();
    o.border = 1;
    Image whole;
    ESIA_CHECK(Rasterize(doc, 1200, 900, o, whole));
    DrawList dl;
    std::vector<TextureChange> changes;
    const auto bounds = [&] {
        Rect b(1e9f, 1e9f, -1e9f, -1e9f);
        for (const Vertex& v : dl.Vertices())
            b = Rect(std::min(b.min.x, v.pos.x), std::min(b.min.y, v.pos.y), std::max(b.max.x, v.pos.x), std::max(b.max.y, v.pos.y));
        return b;
    };
    // the pixels the frame uploaded for a part of w x h from (x0, y0) of the drawing (into a page: inside its border, the
    // padding beside it), against the whole image's there
    const auto uploaded = [&](int x0, int y0, int w, int h) {
        changes.clear();
        textures.TakeChanges(changes);
        for (const TextureChange& c : changes)
            if (c.kind == TextureChange::Kind::Update && c.width == w + 3 && c.height == h + 3 && !c.pixels.empty())
            {
                for (int y = 0; y < h; ++y)
                    if (std::memcmp(c.pixels.data() + ((std::size_t)(y + 1) * (std::size_t)c.width + 1) * 4, Px(whole, x0 + 1, y0 + y + 1), (std::size_t)w * 4) != 0)
                        return false;
                return true;
            }
        return false;
    };
    const auto frame = [&](const Rect& r) {
        atlas.BeginFrame();
        dl.Reset(Rect(0, 0, 400, 300));
        Painter p(dl, PainterEnv());
        Draw(p, atlas, doc, r, Color::White());
    };
    // outside: nothing
    frame(Rect(500, 50, 1700, 950));
    ESIA_CHECK(atlas.Rasterized() == 0 && dl.Commands().empty());
    // from (100, 50), 300 x 250 of it shown: its columns 0 - 512 and rows 0 - 256, an image from (100, 50) - the whole
    // image's pixels
    frame(Rect(100, 50, 1300, 950));
    ESIA_CHECK(atlas.Rasterized() == 1 && bounds() == Rect(100, 50, 612, 306) && uploaded(0, 0, 512, 256));
    // moved by less than a step: the same part
    frame(Rect(60, 50, 1260, 950));
    ESIA_CHECK(atlas.Rasterized() == 1 && bounds() == Rect(60, 50, 572, 306));
    // the next step down comes into view: another part
    frame(Rect(60, 30, 1260, 930));
    ESIA_CHECK(atlas.Rasterized() == 2 && bounds() == Rect(60, 30, 572, 542) && uploaded(0, 0, 512, 512));
    // its middle shown: columns and rows 256 - 768
    frame(Rect(-300, -400, 900, 500));
    ESIA_CHECK(atlas.Rasterized() == 3 && bounds() == Rect(-44, -144, 468, 368) && uploaded(256, 256, 512, 512));
    // under a clip of its own: the part inside that one
    atlas.BeginFrame();
    dl.Reset(Rect(0, 0, 400, 300));
    {
        Painter p(dl, PainterEnv());
        p.PushClip(Rect(0, 0, 40, 40));
        Draw(p, atlas, doc, Rect(0, 0, 1200, 900), Color::White());
        p.PopClip();
    }
    ESIA_CHECK(atlas.Rasterized() == 4 && bounds() == Rect(0, 0, 256, 256) && uploaded(0, 0, 256, 256));
    // most of a drawing shown: all of it
    frame(Rect(-20, 10, 380, 310));
    ESIA_CHECK(atlas.Rasterized() == 5 && bounds() == Rect(-20, 10, 380, 310));
    // scrolled down through the view, 60 pixels a frame: a new part each time the view reaches the next step - made
    // from the part before and the band it does not hold - the whole image's pixels every time
    Atlas scrolled(textures);
    int parts = 0, lastY0 = -1, lastY1 = -1;
    bool same = true;
    for (int s = 0; s <= 600; s += 60)
    {
        scrolled.BeginFrame();
        dl.Reset(Rect(0, 0, 400, 300));
        {
            Painter p(dl, PainterEnv());
            Draw(p, scrolled, doc, Rect(-100.0f, (float)-s, 1100.0f, (float)(900 - s)), Color::White());
        }
        const int y0 = s / 256 * 256, y1 = std::min(900, (s + 300 + 255) / 256 * 256);
        if (y0 != lastY0 || y1 != lastY1)
        {
            ++parts;
            same = same && uploaded(0, y0, 512, y1 - y0);
            lastY0 = y0;
            lastY1 = y1;
        }
        same = same && bounds() == Rect(-100.0f, (float)(y0 - s), 412.0f, (float)(y1 - s));
    }
    ESIA_CHECK(same && parts == 5 && scrolled.Rasterized() == parts);
}
