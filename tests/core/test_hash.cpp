#include "esia/base/hash.hpp"
#include "esia/base/math.hpp"
#include "esia_test.hpp"

using namespace esia;

ESIA_TEST(Hash, SeedChangesId)
{
    ESIA_CHECK(HashLabel("OK", 1) != HashLabel("OK", 2));
    ESIA_CHECK(HashLabel("OK", 1) == HashLabel("OK", 1));
    ESIA_CHECK(HashLabel("OK", 1) != HashLabel("Cancel", 1));
}

ESIA_TEST(Hash, LabelConventions)
{
    // "##" keeps the suffix in the hash, "###" hashes only from there
    ESIA_CHECK(HashLabel("Save##a", 7) != HashLabel("Save##b", 7));
    ESIA_CHECK(HashLabel("Save###x", 7) == HashLabel("Load###x", 7));
    ESIA_CHECK(LabelText("Save##a") == "Save");
    ESIA_CHECK(LabelText("Save###x") == "Save");
    ESIA_CHECK(LabelText("Plain") == "Plain");
}

ESIA_TEST(Hash, NeverZeroAndConstexpr)
{
    constexpr Id id = HashLabel("compile time", 0);
    static_assert(id != 0);
    ESIA_CHECK(HashInt(0, 0) != 0);
    ESIA_CHECK(HashInt(1, 5) != HashInt(2, 5));
}

ESIA_TEST(Math, RectOps)
{
    const Rect a(0, 0, 10, 10), b(5, 5, 20, 20);
    ESIA_CHECK(a.Overlaps(b));
    ESIA_CHECK(a.Intersect(b) == Rect(5, 5, 10, 10));
    ESIA_CHECK(a.Union(b) == Rect(0, 0, 20, 20));
    ESIA_CHECK(!a.Overlaps(Rect(10, 0, 20, 10)));   // max is exclusive
    ESIA_CHECK(a.Contains(Vec2(0, 0)) && !a.Contains(Vec2(10, 5)));
    ESIA_CHECK(Rect(3, 3, 3, 9).Empty());
}

ESIA_TEST(Math, ColorPacking)
{
    const Color c(1.0f, 0.5f, 0.0f, 1.0f);
    const std::uint32_t p = c.ToRgba8();
    ESIA_CHECK((p & 0xFF) == 255);
    ESIA_CHECK(((p >> 8) & 0xFF) == 128);
    ESIA_CHECK(((p >> 16) & 0xFF) == 0);
    ESIA_CHECK((p >> 24) == 255);
    ESIA_CHECK_NEAR(Color::FromRgba8(p).g, 128.0f / 255.0f, 1e-6f);
    const Color h = Color::Hsv(0.0f, 1.0f, 1.0f);
    ESIA_CHECK(h == Color(1, 0, 0, 1));
    ESIA_CHECK_NEAR(Color::Hsv(1.0f / 3.0f, 1.0f, 1.0f).g, 1.0f, 1e-5f);
}
