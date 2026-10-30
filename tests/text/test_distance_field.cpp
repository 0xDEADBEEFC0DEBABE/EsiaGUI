// The multi-channel signed distance fields (MTSDF) of glyph outlines: true distances in A, the multi-channel median
// that keeps corners sharp under bilinear filtering, the filled area of overlapping contours, holes, either winding,
// curves; FieldMaker's workers and cache on disk, which give the generator's bits.
#include "esia/text/distance_field.hpp"
#include "esia_test.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <vector>

using namespace esia;
using namespace esia::text;

namespace
{
    constexpr float kRange = 8.0f, kAlphaRange = 24.0f;

    void AddRect(GlyphShape& s, float x0, float y0, float x1, float y1, bool reverse = false)
    {
        s.MoveTo(Vec2(x0, y0));
        if (reverse)
        {
            s.LineTo(Vec2(x0, y1));
            s.LineTo(Vec2(x1, y1));
            s.LineTo(Vec2(x1, y0));
        }
        else
        {
            s.LineTo(Vec2(x1, y0));
            s.LineTo(Vec2(x1, y1));
            s.LineTo(Vec2(x0, y1));
        }
    }

    void AddCircle(GlyphShape& s, Vec2 c, float r)
    {
        const float k = 0.5522847f * r;
        s.MoveTo(c + Vec2(r, 0));
        s.CubicTo(c + Vec2(r, k), c + Vec2(k, r), c + Vec2(0, r));
        s.CubicTo(c + Vec2(-k, r), c + Vec2(-r, k), c + Vec2(-r, 0));
        s.CubicTo(c + Vec2(-r, -k), c + Vec2(-k, -r), c + Vec2(0, -r));
        s.CubicTo(c + Vec2(k, -r), c + Vec2(r, -k), c + Vec2(r, 0));
    }

    GlyphBitmap Field(const GlyphShape& s)
    {
        GlyphBitmap b;
        DistanceFieldDesc d;
        d.range = kRange;
        d.alphaRange = kAlphaRange;
        ESIA_CHECK(GenerateMtsdf(s, d, b));
        ESIA_CHECK(b.sdf && b.channels == 4 && b.pixels.size() == (std::size_t)b.width * b.height * 4);
        return b;
    }

    // channel c bilinearly at p (field pixels; texel centers on .5), as the GPU filters it
    float Sample(const GlyphBitmap& b, Vec2 p, int c)
    {
        const float fx = p.x - (float)b.left - 0.5f, fy = p.y - (float)b.top - 0.5f;
        const int x0 = (int)std::floor(fx), y0 = (int)std::floor(fy);
        const float tx = fx - (float)x0, ty = fy - (float)y0;
        const auto at = [&](int x, int y) {
            x = std::clamp(x, 0, b.width - 1);
            y = std::clamp(y, 0, b.height - 1);
            return (float)b.pixels[((std::size_t)y * b.width + x) * 4 + (std::size_t)c] / 255.0f;
        };
        return Lerp(Lerp(at(x0, y0), at(x0 + 1, y0), tx), Lerp(at(x0, y0 + 1), at(x0 + 1, y0 + 1), tx), ty);
    }

    // the edge distance the shader reads: the median of the filtered channels (field pixels, inside positive)
    float MedianDistance(const GlyphBitmap& b, Vec2 p)
    {
        const float r = Sample(b, p, 0), g = Sample(b, p, 1), bl = Sample(b, p, 2);
        return (std::max(std::min(r, g), std::min(std::max(r, g), bl)) - 0.5f) * kRange;
    }

    float TrueDistance(const GlyphBitmap& b, Vec2 p) { return (Sample(b, p, 3) - 0.5f) * kAlphaRange; }

    // A at texel (x, y) - not filtered: across a ridge of the distance (the medial axis) filtering is a little short
    // of it - and that texel's center
    float TexelTrue(const GlyphBitmap& b, int x, int y) { return ((float)b.pixels[((std::size_t)y * b.width + x) * 4 + 3] / 255.0f - 0.5f) * kAlphaRange; }
    Vec2 TexelCenter(const GlyphBitmap& b, int x, int y) { return Vec2((float)(b.left + x) + 0.5f, (float)(b.top + y) + 0.5f); }

    // signed distance to an axis-aligned rectangle (inside positive)
    float RectDistance(Vec2 p, float x0, float y0, float x1, float y1)
    {
        const float dx = std::max(x0 - p.x, p.x - x1), dy = std::max(y0 - p.y, p.y - y1);
        const float outside = Length(Vec2(std::max(dx, 0.0f), std::max(dy, 0.0f)));
        return dx <= 0.0f && dy <= 0.0f ? -std::max(dx, dy) : -outside;
    }

    // ... and what the multi-channel median gives for it: the same inside, mitred outside its corners
    float RectMitre(Vec2 p, float x0, float y0, float x1, float y1) { return -std::max(std::max(x0 - p.x, p.x - x1), std::max(y0 - p.y, p.y - y1)); }

    // signed distance to a polygon (non-zero inside, positive)
    float PolygonDistance(const std::vector<Vec2>& poly, Vec2 p)
    {
        float best = 1e9f;
        int winding = 0;
        for (std::size_t i = 0; i < poly.size(); ++i)
        {
            const Vec2 a = poly[i], b = poly[(i + 1) % poly.size()], ab = b - a;
            const float t = Clamp(Dot(p - a, ab) / LengthSq(ab), 0.0f, 1.0f);
            best = std::min(best, Length(p - (a + ab * t)));
            if ((a.y <= p.y) != (b.y <= p.y) && a.x + (p.y - a.y) * (b.x - a.x) / (b.y - a.y) > p.x)
                winding += b.y > a.y ? 1 : -1;
        }
        return winding != 0 ? best : -best;
    }

    Vec2 Cubic(Vec2 a, Vec2 b, Vec2 c, Vec2 d, float t)
    {
        const float u = 1.0f - t;
        return a * (u * u * u) + b * (3.0f * u * u * t) + c * (3.0f * u * t * t) + d * (t * t * t);
    }
}

// A square: both fields measure the distance to its sides, A the true one farther out.
ESIA_TEST(DistanceField, ASquaresDistances)
{
    GlyphShape s;
    AddRect(s, 4, 6, 24, 26);
    const GlyphBitmap b = Field(s);
    ESIA_CHECK(b.left <= 4 - 13 && b.top <= 6 - 13 && b.left + b.width >= 24 + 13 && b.top + b.height >= 26 + 13);
    float worstMedian = 0.0f, worstTrue = 0.0f;
    for (float y = -2.0f; y <= 34.0f; y += 0.75f)
        for (float x = -4.0f; x <= 32.0f; x += 0.75f)
        {
            const Vec2 p(x, y);
            const float m = RectMitre(p, 4, 6, 24, 26);
            if (std::fabs(m) < kRange * 0.5f - 1.0f)
                worstMedian = std::max(worstMedian, std::fabs(MedianDistance(b, p) - m));
        }
    for (int y = 0; y < b.height; ++y)
        for (int x = 0; x < b.width; ++x)
        {
            const float d = RectDistance(TexelCenter(b, x, y), 4, 6, 24, 26);
            if (std::fabs(d) < kAlphaRange * 0.5f - 0.5f)
                worstTrue = std::max(worstTrue, std::fabs(TexelTrue(b, x, y) - d));
        }
    ESIA_CHECK(worstMedian < 0.1f);
    ESIA_CHECK(worstTrue < 0.1f);   // 8 bits over 24 px: 0.05
    // saturated where the range ends: the multi-channel inside the middle
    ESIA_CHECK(Sample(b, Vec2(14, 16), 0) == 1.0f && Sample(b, Vec2(14, 16), 1) == 1.0f && Sample(b, Vec2(14, 16), 2) == 1.0f);
}

// The corners stay sharp: the median's zero line follows the square up to its corner point, where the single
// channel of true distance (A) rounds it off.
ESIA_TEST(DistanceField, CornersStaySharp)
{
    GlyphShape s;
    AddRect(s, 4, 6, 24, 26);
    const GlyphBitmap b = Field(s);
    for (const Vec2 corner : {Vec2(4, 6), Vec2(24, 6), Vec2(24, 26), Vec2(4, 26)})
    {
        const Vec2 in = Vec2(corner.x == 4 ? 1.0f : -1.0f, corner.y == 6 ? 1.0f : -1.0f);   // towards the inside
        // a quarter pixel inside the corner along the diagonal: inside; a quarter outside: outside
        ESIA_CHECK(MedianDistance(b, corner + in * 0.25f) > 0.0f);
        ESIA_CHECK(MedianDistance(b, corner - in * 0.25f) < 0.0f);
        // farther out the true distance's iso-lines go round the corner and the median's stay mitred: 2 px out along
        // the diagonal the true distance is 2.83, the median's 2 (an outline of the glyph keeps its corners)
        ESIA_CHECK(std::fabs(TrueDistance(b, corner - in * 2.0f) + 2.83f) < 0.2f);
        ESIA_CHECK(std::fabs(MedianDistance(b, corner - in * 2.0f) + 2.0f) < 0.25f);
        // and sampled between texels close to the corner the median holds it to a twentieth of a pixel
        ESIA_CHECK(std::fabs(MedianDistance(b, corner - in * 0.4f) + 0.4f) < 0.05f);
    }
}

// Either winding gives the same field; a hole (the other winding inside) is outside.
ESIA_TEST(DistanceField, WindingAndHoles)
{
    GlyphShape cw, ccw;
    AddRect(cw, 4, 6, 24, 26);
    AddRect(ccw, 4, 6, 24, 26, true);
    const GlyphBitmap a = Field(cw), b = Field(ccw);
    ESIA_CHECK(a.width == b.width && a.height == b.height && a.left == b.left && a.top == b.top);
    int differ = 0;
    for (std::size_t i = 3; i < a.pixels.size(); i += 4)
        differ += std::abs((int)a.pixels[i] - (int)b.pixels[i]) > 1;
    ESIA_CHECK(differ == 0);
    for (float y = 8.0f; y <= 24.0f; y += 2.0f)
        ESIA_CHECK(std::fabs(MedianDistance(a, Vec2(10.0f, y)) - MedianDistance(b, Vec2(10.0f, y))) < 0.05f);

    GlyphShape ring;
    AddRect(ring, 0, 0, 30, 30);
    AddRect(ring, 10, 10, 20, 20, true);
    const GlyphBitmap r = Field(ring);
    ESIA_CHECK(MedianDistance(r, Vec2(15, 15)) < -3.0f && TrueDistance(r, Vec2(15, 15)) < -4.0f);   // the hole's middle
    ESIA_CHECK(MedianDistance(r, Vec2(5, 15)) > 3.0f);                                              // the ring
    ESIA_CHECK(std::fabs(MedianDistance(r, Vec2(11, 15)) + 1.0f) < 0.15f);                          // 1 px into the hole
}

// Overlapping contours fill their union: no edge inside it (a seam), and A measures the distance to the union's
// boundary - where the contours cross is a sharp corner of the union.
ESIA_TEST(DistanceField, OverlapsFillTheirUnion)
{
    GlyphShape s;
    AddRect(s, 4, 4, 20, 20);
    AddRect(s, 12, 12, 28, 28);   // the same winding: it overlaps the first on 12 .. 20
    const GlyphBitmap b = Field(s);
    // where the first square's right edge runs inside the second, and the other way round
    for (const Vec2 p : {Vec2(20, 16), Vec2(20, 18), Vec2(16, 12), Vec2(14, 12), Vec2(19.5f, 15), Vec2(16, 12.5f)})
    {
        // 3 px or more from the union's boundary (and 0 .. 0.5 px from a hidden edge)
        ESIA_CHECK(MedianDistance(b, p) > 2.5f);
        ESIA_CHECK(TrueDistance(b, p) > 2.5f);
    }
    // the union's boundary: its re-entrant corners at (20, 12) and (12, 20) stay sharp
    ESIA_CHECK(MedianDistance(b, Vec2(20.5f, 11.5f)) < 0.0f && MedianDistance(b, Vec2(19.5f, 12.5f)) > 0.0f);
    ESIA_CHECK(MedianDistance(b, Vec2(11.5f, 20.5f)) < 0.0f && MedianDistance(b, Vec2(12.5f, 19.5f)) > 0.0f);
    // distances to the union's outline (inside it the nearest point may be a re-entrant corner: not the larger of the
    // two squares' distances), not to the hidden edges
    const std::vector<Vec2> outline = {Vec2(4, 4), Vec2(20, 4), Vec2(20, 12), Vec2(28, 12), Vec2(28, 28), Vec2(12, 28), Vec2(12, 20), Vec2(4, 20)};
    float worst = 0.0f;
    for (int y = 0; y < b.height; ++y)
        for (int x = 0; x < b.width; ++x)
        {
            const Vec2 p = TexelCenter(b, x, y);
            const float d = PolygonDistance(outline, p);
            if (std::fabs(d) < kAlphaRange * 0.5f - 0.5f)
                worst = std::max(worst, std::fabs(TexelTrue(b, x, y) - d));
        }
    ESIA_CHECK(worst < 0.1f);
}

// Curves: a circle of cubics, its distance in both fields; no corner, so the median is the true distance.
ESIA_TEST(DistanceField, Curves)
{
    GlyphShape s;
    AddCircle(s, Vec2(20, 20), 12);
    const GlyphBitmap b = Field(s);
    float worstMedian = 0.0f, worstTrue = 0.0f;
    for (float a = 0.0f; a < 6.283f; a += 0.1f)
        for (float r = 6.0f; r <= 18.0f; r += 0.5f)
        {
            const Vec2 p = Vec2(20, 20) + Vec2(std::cos(a), std::sin(a)) * r;
            const float d = 12.0f - r;
            if (std::fabs(d) < kRange * 0.5f - 1.0f)
                worstMedian = std::max(worstMedian, std::fabs(MedianDistance(b, p) - d));
            worstTrue = std::max(worstTrue, std::fabs(TrueDistance(b, p) - d));
        }
    ESIA_CHECK(worstMedian < 0.12f);   // the cubic circle is within 0.03 % of a true one
    ESIA_CHECK(worstTrue < 0.2f);
}

// The shapes text has: a triangle (acute corners), an L (a re-entrant corner), a drop (one corner on a curve) -
// each sampled at 6 x magnification from its field on the same side of its outline as the exact shape, but within a
// tenth of a pixel of the outline (8-bit rounding).
ESIA_TEST(DistanceField, MagnifiedShapesMatchTheirOutlines)
{
    std::vector<Vec2> drop;
    const Vec2 tip(16, 3), bottom(16, 28);
    for (int i = 0; i < 400; ++i)
        drop.push_back(Cubic(tip, Vec2(22, 12), Vec2(28, 28), bottom, (float)i / 400.0f));
    for (int i = 0; i < 400; ++i)
        drop.push_back(Cubic(bottom, Vec2(4, 28), Vec2(10, 12), tip, (float)i / 400.0f));
    const std::vector<Vec2> triangle = {Vec2(4, 26), Vec2(16, 3), Vec2(28, 26)};
    const std::vector<Vec2> ell = {Vec2(4, 4), Vec2(12, 4), Vec2(12, 20), Vec2(26, 20), Vec2(26, 28), Vec2(4, 28)};
    for (int k = 0; k < 3; ++k)
    {
        GlyphShape s;
        const std::vector<Vec2>& poly = k == 0 ? triangle : k == 1 ? ell : drop;
        if (k < 2)
        {
            s.MoveTo(poly[0]);
            for (std::size_t i = 1; i < poly.size(); ++i)
                s.LineTo(poly[i]);
        }
        else
        {
            s.MoveTo(tip);
            s.CubicTo(Vec2(22, 12), Vec2(28, 28), bottom);
            s.CubicTo(Vec2(4, 28), Vec2(10, 12), tip);
        }
        const GlyphBitmap b = Field(s);
        int wrong = 0;
        for (float y = 1.0f; y < 31.0f; y += 1.0f / 6.0f)
            for (float x = 1.0f; x < 31.0f; x += 1.0f / 6.0f)
            {
                const Vec2 p(x + 1.0f / 12.0f, y + 1.0f / 12.0f);
                const float d = PolygonDistance(poly, p);
                if (std::fabs(d) > 0.1f && (MedianDistance(b, p) > 0.0f) != (d > 0.0f))
                    ++wrong;
            }
        if (wrong > 0)
            std::printf("  %s: %d magnified samples on the wrong side\n", k == 0 ? "triangle" : k == 1 ? "L" : "drop", wrong);
        ESIA_CHECK(wrong == 0);
    }
}

ESIA_TEST(DistanceField, EmptyAndOversizedShapes)
{
    GlyphShape empty;
    GlyphBitmap b;
    ESIA_CHECK(GenerateMtsdf(empty, {}, b) && b.width == 0 && b.pixels.empty() && b.sdf);
    GlyphShape huge;
    AddRect(huge, 0, 0, 5000, 10);
    ESIA_CHECK(!GenerateMtsdf(huge, {}, b));
}

namespace
{
    // Shapes of every kind the generator meets: corners, holes, overlaps, curves, a polygon of many sides, nothing at
    // all, and one too large for a field
    std::vector<GlyphShape> TestShapes()
    {
        std::vector<GlyphShape> shapes(9);
        AddRect(shapes[0], 4, 6, 24, 26);
        AddRect(shapes[1], 0, 0, 30, 30);
        AddRect(shapes[1], 10, 10, 20, 20, true);
        AddRect(shapes[2], 4, 4, 20, 20);
        AddRect(shapes[2], 12, 12, 28, 28);
        AddCircle(shapes[3], Vec2(20, 20), 12);
        shapes[4].MoveTo(Vec2(4, 26));
        shapes[4].LineTo(Vec2(16, 3));
        shapes[4].LineTo(Vec2(28, 26));
        shapes[5].MoveTo(Vec2(16, 3));
        shapes[5].CubicTo(Vec2(22, 12), Vec2(28, 28), Vec2(16, 28));
        shapes[5].QuadTo(Vec2(4, 28), Vec2(16, 3));
        for (int i = 0; i < 24; ++i)   // a star
        {
            const float a = (float)i * kTau / 24.0f, r = i % 2 ? 7.0f : 15.0f;
            const Vec2 p = Vec2(18, 18) + Vec2(std::cos(a), std::sin(a)) * r;
            if (i == 0)
                shapes[6].MoveTo(p);
            else
                shapes[6].LineTo(p);
        }
        // shapes[7]: empty
        AddRect(shapes[8], 0, 0, 5000, 10);
        return shapes;
    }

    bool SameField(const GlyphBitmap& a, const GlyphBitmap& b)
    {
        return a.left == b.left && a.top == b.top && a.width == b.width && a.height == b.height && a.channels == b.channels && a.sdf == b.sdf &&
               a.lcd == b.lcd && a.pixels == b.pixels;
    }

    // `maker`'s fields of `shapes` against GenerateMtsdf's on this thread, bit for bit (null where it refuses one)
    int Different(FieldMaker& maker, const std::vector<GlyphShape>& shapes, const DistanceFieldDesc& desc, int* fromDisk = nullptr)
    {
        for (GlyphShape s : shapes)
            maker.Add(std::move(s), desc);
        maker.Make();
        int different = 0;
        if (fromDisk)
            *fromDisk = 0;
        for (std::size_t i = 0; i < shapes.size(); ++i)
        {
            GlyphBitmap alone;
            const bool ok = GenerateMtsdf(shapes[i], desc, alone);
            const GlyphBitmap* made = maker.Field(i);
            different += ok != (made != nullptr) || (ok && !SameField(*made, alone));
            if (fromDisk)
                *fromDisk += maker.FromDisk(i);
        }
        return different;
    }

    std::filesystem::path FreshDirectory(const char* name)
    {
        const std::filesystem::path dir = std::filesystem::path(ESIA_TEXT_OUT_DIR) / name;
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
        return dir;
    }

    std::vector<std::filesystem::path> FilesIn(const std::filesystem::path& dir)
    {
        std::vector<std::filesystem::path> files;
        std::error_code ec;
        for (const auto& e : std::filesystem::directory_iterator(dir, ec))
            files.push_back(e.path());
        std::sort(files.begin(), files.end());
        return files;
    }
}

// A batch of fields made by the workers and the waiting thread: GenerateMtsdf's bits, in the order added, whatever
// thread made each; batch after batch; with no workers at all.
ESIA_TEST(FieldMaker, SameBitsAsMadeOneByOne)
{
    const std::vector<GlyphShape> shapes = TestShapes();
    for (const int workers : {0, 3, 8})
    {
        FieldMakerDesc d;
        d.workers = workers;
        FieldMaker maker(d);
        ESIA_CHECK(maker.Workers() == workers);
        for (int round = 0; round < 3; ++round)
            ESIA_CHECK(Different(maker, shapes, DistanceFieldDesc{8.0f, 24.0f}) == 0);
        ESIA_CHECK(Different(maker, shapes, DistanceFieldDesc{6.0f, 12.0f}) == 0);
        ESIA_CHECK(maker.Field(7) && maker.Field(7)->width == 0 && !maker.Field(8));   // empty: a field of nothing; too large: none
    }
    ESIA_CHECK(FieldMaker().Workers() >= 0);
}

// The cache on disk: a second maker finds the fields the first one made, the same bits, from the same shapes and
// ranges only - another range, a point moved by the least step a float has, a damaged or cut file, another field's
// file under its name are made again (and the file put right).
ESIA_TEST(FieldMaker, CacheOnDiskKeepsTheSameFields)
{
    const std::filesystem::path dir = FreshDirectory("field_cache_test");
    FieldMakerDesc d;
    d.workers = 2;
    d.cacheDirectory = reinterpret_cast<const char*>(dir.u8string().c_str());
    const std::vector<GlyphShape> shapes = TestShapes();
    const DistanceFieldDesc desc{8.0f, 24.0f};
    int fromDisk = -1;
    {
        FieldMaker first(d);
        ESIA_CHECK(Different(first, shapes, desc, &fromDisk) == 0 && fromDisk == 0);
    }   // its fields written as it goes
    const std::vector<std::filesystem::path> files = FilesIn(dir);
    ESIA_CHECK(files.size() == 7);   // not the empty shape, not the one too large
    for (const int workers : {2, 0})
    {
        d.workers = workers;
        FieldMaker second(d);
        ESIA_CHECK(Different(second, shapes, desc, &fromDisk) == 0 && fromDisk == 7);
    }

    // what makes another field misses: another range, a point moved by one float step
    {
        FieldMaker other(d);
        ESIA_CHECK(Different(other, shapes, DistanceFieldDesc{6.0f, 24.0f}, &fromDisk) == 0 && fromDisk == 0);
        std::vector<GlyphShape> moved(1);
        AddRect(moved[0], 4, 6, 24, std::nextafter(26.0f, 100.0f));
        ESIA_CHECK(Different(other, moved, desc, &fromDisk) == 0 && fromDisk == 0);
    }

    // files damaged on disk are not taken: one texel changed, one cut short, one longer, one another field's
    const auto rewrite = [](const std::filesystem::path& file, auto change) {
        std::FILE* f = std::fopen(file.string().c_str(), "rb");
        std::vector<unsigned char> bytes;
        for (int c; f && (c = std::fgetc(f)) != EOF;)
            bytes.push_back((unsigned char)c);
        if (f)
            std::fclose(f);
        change(bytes);
        f = std::fopen(file.string().c_str(), "wb");
        if (f)
        {
            std::fwrite(bytes.data(), 1, bytes.size(), f);
            std::fclose(f);
        }
    };
    ESIA_CHECK(files.size() == 7);
    if (files.size() == 7)
    {
        rewrite(files[0], [](std::vector<unsigned char>& b) { b[b.size() / 2] ^= 1; });
        rewrite(files[1], [](std::vector<unsigned char>& b) { b.resize(b.size() - 4); });
        rewrite(files[2], [](std::vector<unsigned char>& b) { b.push_back(0); });
        std::error_code ec;
        std::filesystem::copy_file(files[4], files[3], std::filesystem::copy_options::overwrite_existing, ec);
        {
            FieldMaker damaged(d);
            ESIA_CHECK(Different(damaged, shapes, desc, &fromDisk) == 0 && fromDisk == 3);
        }
        FieldMaker mended(d);
        ESIA_CHECK(Different(mended, shapes, desc, &fromDisk) == 0 && fromDisk == 7);
    }
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
}
