// The FreeType + HarfBuzz text system with the fonts in tests/fonts: font loading, metrics from the
// font (kerning, marks, tabs), line breaking, trimming and alignment, pixel-aligned glyph quads on atlas pages, pixel
// density, fallback fonts, scaled and icon glyphs, caching - and the whole path end to end: the quads Draw emits,
// composited on the CPU from the pages the registry received, against a golden image.
#include "esia/text/freetype.hpp"
#include "esia/text/system_fonts.hpp"
#include <cstdio>
#include "font_test_util.hpp"
#include "ft_test_util.hpp"

using namespace esia;
using namespace esia::texttest;

namespace
{
    // DroidSans has 2048 design units per em: at this size one design unit is 0.01 UI units
    constexpr float kUnitSize = 20.48f;

    struct Fixture
    {
        TextureRegistry textures;
        std::unique_ptr<text::TextSystem> ts;
        text::FontId droid = 0;

        explicit Fixture(const text::FreeTypeDesc& desc = {}) : ts(text::CreateFreeTypeTextSystem(textures, desc))
        {
            ESIA_CHECK(ts != nullptr);
            droid = ts->AddFontFile((kFonts + "/DroidSans.ttf").c_str());
            ESIA_CHECK(droid != 0);
            ts->NewFrame({});
        }
    };
}

ESIA_TEST(FreeType, LoadsFontsFromFilesAndMemory)
{
    const std::vector<std::uint8_t> karlaBytes = ReadFile(kFonts + "/Karla-Regular.ttf");   // outlives the text system
    Fixture f;
    ESIA_CHECK(f.ts->AddFontFile((kFonts + "/missing.ttf").c_str()) == 0);
    ESIA_CHECK(f.ts->AddFontFile((kFonts + "/DroidSans.ttf").c_str(), 1) == 0);   // a single-face file
    ESIA_CHECK(f.ts->AddFontFile(nullptr) == 0);
    const char junk[64] = {};
    ESIA_CHECK(f.ts->AddFontMemory(junk, sizeof(junk)) == 0);
    ESIA_CHECK(f.ts->AddFontMemory(nullptr, 100) == 0);

    ESIA_CHECK(!karlaBytes.empty());
    const text::FontId karla = f.ts->AddFontMemory(karlaBytes.data(), karlaBytes.size());
    ESIA_CHECK(karla != 0 && karla != f.droid);
    ESIA_CHECK(f.ts->Measure({karla, 10.0f}, "x").size.x > 0.0f);

    // unknown fonts and empty sizes lay out nothing
    ESIA_CHECK(f.ts->Measure({99, 10.0f}, "x").lines == 0);
    ESIA_CHECK(f.ts->Measure({f.droid, 0.0f}, "x").lines == 0);
    DrawList dl = NewList();
    ESIA_CHECK(f.ts->Draw(dl, {99, 10.0f}, Vec2(0, 0), Color::Black(), "x") == Vec2(0, 0) && dl.Empty());
}

ESIA_TEST(FreeType, LoadsTheFacesOfACollection)
{
    // DroidSans and Karla in one .ttc: face 1 is Karla, measured as the single file measures it
    std::filesystem::create_directories(ESIA_TEXT_OUT_DIR);
    const std::string ttc = std::string(ESIA_TEXT_OUT_DIR) + "/droid-karla.ttc";
    ESIA_CHECK(WriteFile(ttc, MakeCollection({{ReadFile(kFonts + "/DroidSans.ttf")}, {ReadFile(kFonts + "/Karla-Regular.ttf")}})));
    Fixture f;
    const text::FontId droid = f.ts->AddFontFile(ttc.c_str(), 0);
    const text::FontId karla = f.ts->AddFontFile(ttc.c_str(), 1);
    const text::FontId karlaFile = f.ts->AddFontFile((kFonts + "/Karla-Regular.ttf").c_str());
    ESIA_CHECK(droid != 0 && karla != 0 && karlaFile != 0);
    ESIA_CHECK(f.ts->AddFontFile(ttc.c_str(), 2) == 0);
    ESIA_CHECK(f.ts->Measure({karla, 20.0f}, "Karla").size == f.ts->Measure({karlaFile, 20.0f}, "Karla").size);
    ESIA_CHECK(f.ts->Measure({droid, 20.0f}, "Droid").size == f.ts->Measure({f.droid, 20.0f}, "Droid").size);
    ESIA_CHECK(f.ts->Measure({karla, 20.0f}, "Karla").size != f.ts->Measure({droid, 20.0f}, "Karla").size);
}

ESIA_TEST(FreeType, MetricsComeFromTheFont)
{
    Fixture f;
    const text::FontRef fr{f.droid, kUnitSize};
    const text::TextMetrics m = f.ts->Measure(fr, "AV");
    ESIA_CHECK_NEAR(m.size.x, 23.47f, 1e-3f);   // kerned: 2347 design units, A and V alone are 2408
    ESIA_CHECK_NEAR(f.ts->Measure(fr, "A").size.x + f.ts->Measure(fr, "V").size.x, 24.08f, 1e-3f);
    ESIA_CHECK_NEAR(m.size.y, 23.84f, 1e-3f);    // ascent 1901 + descent 483 + line gap 0
    ESIA_CHECK_NEAR(m.baseline, 19.01f, 1e-3f);
    ESIA_CHECK(m.lines == 1);

    // an empty text is one empty line (a text field's height); a trailing hard break opens another line
    const text::TextMetrics e = f.ts->Measure(fr, "");
    ESIA_CHECK(e.size.x == 0.0f && e.lines == 1);
    ESIA_CHECK_NEAR(e.size.y, 23.84f, 1e-3f);
    ESIA_CHECK(f.ts->Measure(fr, "a\n").lines == 2);

    // unhinted, linear advances: twice the size is twice the width
    const float w = f.ts->Measure(fr, "Hamburgefonstiv").size.x;
    ESIA_CHECK_NEAR(f.ts->Measure({f.droid, 2.0f * kUnitSize}, "Hamburgefonstiv").size.x, 2.0f * w, 1e-3f);

    // a combining mark takes no room: e + U+0301 is as wide as e
    ESIA_CHECK_NEAR(f.ts->Measure(fr, "e\xCC\x81").size.x, f.ts->Measure(fr, "e").size.x, 1e-4f);
    // a tab advances like four spaces (532 units each); other controls take no room
    ESIA_CHECK_NEAR(f.ts->Measure(fr, "a\tb").size.x, f.ts->Measure(fr, "ab").size.x + 4.0f * 5.32f, 0.05f);
    ESIA_CHECK_NEAR(f.ts->Measure(fr, "a\x01" "b").size.x, f.ts->Measure(fr, "ab").size.x, 0.05f);
}

ESIA_TEST(FreeType, BreaksLinesAndTrims)
{
    Fixture f;
    const text::FontRef fr{f.droid, 16.0f};
    const float lh = f.ts->Measure(fr, "x").size.y;

    // at spaces: the words that fit, the rest on the next line
    const float w3 = f.ts->Measure(fr, "one two three").size.x;
    const text::TextMetrics m = f.ts->Measure(fr, "one two three four", w3 + 0.5f);
    ESIA_CHECK(m.lines == 2 && m.size.x <= w3 + 0.5f);
    ESIA_CHECK_NEAR(m.size.y, 2.0f * lh, 1e-4f);
    DrawList dl = NewList();
    f.ts->Draw(dl, fr, Vec2(0, 0), Color::Black(), "one two three four", w3 + 0.5f);
    ESIA_CHECK(QuadsPerLine(Quads(dl), lh) == std::vector<int>({11, 4}));

    // a word longer than the line breaks between characters
    const float w4 = f.ts->Measure(fr, "abcd").size.x;
    ESIA_CHECK(f.ts->Measure(fr, "abcdefghij", w4 + 0.1f).lines == 3);
    // after a hyphen between letters (both halves fit the width: no character breaks)
    const float halves = std::max(f.ts->Measure(fr, "well-").size.x, f.ts->Measure(fr, "known").size.x);
    ESIA_CHECK(f.ts->Measure(fr, "well-known", halves + 0.1f).lines == 2);
    // never before closing punctuation, even after a space: "a word !" breaks after "a", not before "!"
    dl = NewList();
    f.ts->Draw(dl, fr, Vec2(0, 0), Color::Black(), "a word !", f.ts->Measure(fr, "a word").size.x + 0.1f);
    ESIA_CHECK(QuadsPerLine(Quads(dl), lh) == std::vector<int>({1, 5}));
    // hard breaks: LF, CR LF, CR, U+2028 LINE SEPARATOR
    ESIA_CHECK(f.ts->Measure(fr, "a\nb\r\nc\rd\xE2\x80\xA8" "e").lines == 5);

    // ellipsis: one line within the width, ending in U+2026
    const char* longText = "Trimmed with an ellipsis when too long";
    const text::TextMetrics t = f.ts->Measure(fr, longText, 120.0f, text::TextFlags_Ellipsis);
    const float ellipsis = f.ts->Measure(fr, "\xE2\x80\xA6").size.x;
    ESIA_CHECK(t.lines == 1 && t.size.x <= 120.0f && t.size.x > 120.0f - ellipsis - f.ts->Measure(fr, "W").size.x);
    dl = NewList();
    f.ts->Draw(dl, fr, Vec2(0, 0), Color::Black(), longText, 120.0f, text::TextFlags_Ellipsis);
    const std::vector<Quad> q = Quads(dl);
    ESIA_CHECK(!q.empty() && q.back().r.max.x <= 123.0f);
    DrawList ref = NewList();
    f.ts->Draw(ref, fr, Vec2(t.size.x - ellipsis, 0), Color::Black(), "\xE2\x80\xA6");
    ESIA_CHECK(Quads(ref).size() == 1);
    if (Quads(ref).size() == 1)   // the last glyph is the three dots (its bitmap may sit at another quarter-pixel phase)
        ESIA_CHECK(std::fabs(q.back().r.Width() - Quads(ref)[0].r.Width()) <= 1.0f && q.back().r.Height() == Quads(ref)[0].r.Height());
    // text that fits is left alone
    ESIA_CHECK(f.ts->Measure(fr, "short", 120.0f, text::TextFlags_Ellipsis).size.x == f.ts->Measure(fr, "short").size.x);
}

ESIA_TEST(FreeType, AlignsLinesInTheColumn)
{
    Fixture f;
    const text::FontRef fr{f.droid, 16.0f};
    const float lh = f.ts->Measure(fr, "x").size.y;
    DrawList dl = NewList();
    const Vec2 size = f.ts->Draw(dl, fr, Vec2(0, 0), Color::Black(), "ab\nabcdef", 200.0f, text::TextFlags_AlignCenter);
    ESIA_CHECK(size.x == 200.0f);   // aligned lines sit in the wrap column
    float lo[2] = {1e9f, 1e9f}, hi[2] = {-1e9f, -1e9f};
    for (const Quad& q : Quads(dl))
    {
        const int line = q.r.Center().y < lh ? 0 : 1;
        lo[line] = std::min(lo[line], q.r.min.x);
        hi[line] = std::max(hi[line], q.r.max.x);
    }
    // each line centered (the quads carry 1 px of empty margin left, 2 px right)
    ESIA_CHECK_NEAR((lo[0] + hi[0]) * 0.5f, 100.5f, 1.5f);
    ESIA_CHECK_NEAR((lo[1] + hi[1]) * 0.5f, 100.5f, 1.5f);

    dl = NewList();
    f.ts->Draw(dl, fr, Vec2(0, 0), Color::Black(), "ab\nabcdef", 200.0f, text::TextFlags_AlignRight);
    for (const Quad& q : Quads(dl))
        ESIA_CHECK(q.r.max.x <= 203.5f);
    ESIA_CHECK(Quads(dl)[1].r.max.x >= 199.0f && Quads(dl).back().r.max.x >= 199.0f);   // "b" and "f" end at the column's edge

    // without a wrap width, lines align in the widest one
    const text::TextMetrics m = f.ts->Measure(fr, "ab\nabcdef", 0.0f, text::TextFlags_AlignCenter);
    ESIA_CHECK_NEAR(m.size.x, f.ts->Measure(fr, "abcdef").size.x, 1e-4f);
}

ESIA_TEST(FreeType, DrawsPixelAlignedQuadsOnAtlasPages)
{
    Fixture f;
    const text::FontRef fr{f.droid, 16.0f};
    const Color color(0.2f, 0.4f, 0.6f, 1.0f);
    DrawList dl = NewList();
    const Vec2 size = f.ts->Draw(dl, fr, Vec2(10.3f, 20.6f), color, "Esia");
    ESIA_CHECK(size == f.ts->Measure(fr, "Esia").size);
    const std::vector<Quad> quads = Quads(dl);
    ESIA_CHECK(quads.size() == 4);
    float lastX = 0.0f;
    for (const Quad& q : quads)
    {
        // whole pixels: the baseline snapped, the pen's fraction baked into the bitmap (quarter-pixel phases)
        ESIA_CHECK(IsWhole(q.r.min.x) && IsWhole(q.r.min.y) && IsWhole(q.r.max.x) && IsWhole(q.r.max.y));
        ESIA_CHECK(q.color == color.ToRgba8());
        TextureInfo info;
        ESIA_CHECK(f.textures.Info(q.texture, info) && info.format == TextureFormat::Alpha8);
        // texel for pixel
        ESIA_CHECK_NEAR(q.r.Width(), (q.uv1.x - q.uv0.x) * (float)info.width, 1e-3f);
        ESIA_CHECK_NEAR(q.r.Height(), (q.uv1.y - q.uv0.y) * (float)info.height, 1e-3f);
        ESIA_CHECK(q.r.min.x > lastX);
        lastX = q.r.min.x;
        ESIA_CHECK(q.r.min.y >= 20.0f && q.r.max.y <= 20.6f + size.y + 1.0f);
    }

    // clipped away: nothing; transparent: nothing, but measured
    DrawList clipped;
    clipped.Reset(Rect(0, 0, 5, 5));
    f.ts->Draw(clipped, fr, Vec2(100, 100), color, "Esia");
    ESIA_CHECK(clipped.Vertices().empty());
    DrawList clear = NewList();
    ESIA_CHECK(f.ts->Draw(clear, fr, Vec2(0, 0), Color::Clear(), "Esia") == size && clear.Empty());
}

ESIA_TEST(FreeType, PixelDensity)
{
    Fixture f;
    const text::FontRef fr{f.droid, 16.0f};
    DrawList a = NewList(), b = NewList();
    f.ts->Draw(a, fr, Vec2(3, 3), Color::Black(), "H");
    f.ts->NewFrame({2.0f});
    f.ts->Draw(b, fr, Vec2(3, 3), Color::Black(), "H");
    const Quad qa = Quads(a).at(0), qb = Quads(b).at(0);
    // rasterized at the physical size: twice the texels of ink (the bitmaps' two empty rows do not scale) for about
    // the same box in UI units, on the half-unit grid
    ESIA_CHECK_NEAR((qb.uv1.y - qb.uv0.y) * 2048.0f - 2.0f, 2.0f * ((qa.uv1.y - qa.uv0.y) * 2048.0f - 2.0f), 2.0f);
    ESIA_CHECK_NEAR(qb.r.Height(), qa.r.Height(), 1.5f);
    ESIA_CHECK(IsWhole(qb.r.min.y * 2.0f) && IsWhole(qb.r.max.x * 2.0f));
}

ESIA_TEST(FreeType, FallbackFontsKeepTheLineBox)
{
    Fixture f;
    const text::FontRef fr{f.droid, 20.0f};
    const char* rupee = "5 \xE2\x82\xB9";   // DroidSans has no U+20B9: its .notdef stands in
    const text::TextMetrics before = f.ts->Measure(fr, rupee);
    const text::FontId karla = f.ts->AddFontFile((kFonts + "/Karla-Regular.ttf").c_str());   // 1000 units per em
    ESIA_CHECK(karla != 0);
    f.ts->AddFallback(karla);
    f.ts->AddFallback(karla);   // once is enough
    const text::TextMetrics after = f.ts->Measure(fr, rupee);
    // the sign now comes from Karla, scaled to the same size; the line box stays DroidSans'
    ESIA_CHECK(after.size.x != before.size.x);
    ESIA_CHECK_NEAR(after.size.x, f.ts->Measure(fr, "5 ").size.x + f.ts->Measure({karla, 20.0f}, "\xE2\x82\xB9").size.x, 1e-3f);
    ESIA_CHECK(after.size.y == before.size.y && after.baseline == before.baseline);
    DrawList dl = NewList();
    f.ts->Draw(dl, fr, Vec2(0, 0), Color::Black(), rupee);
    ESIA_CHECK(Quads(dl).size() == 2);
    // a character nobody has draws the requested font's .notdef box
    dl = NewList();
    f.ts->Draw(dl, fr, Vec2(0, 0), Color::Black(), "\xE2\x99\xA5");
    ESIA_CHECK(Quads(dl).size() == 1);
}

ESIA_TEST(FreeType, ScaledTextIsRasterizedAtItsSize)
{
    Fixture f;
    const text::FontRef fr{f.droid, 16.0f};
    DrawList a = NewList(), b = NewList();
    const Vec2 pos(40, 30);
    const Vec2 s1 = f.ts->Draw(a, fr, pos, Color::Black(), "Wo", 0.0f, 0, 1.0f);
    const Vec2 s2 = f.ts->Draw(b, fr, pos, Color::Black(), "Wo", 0.0f, 0, 2.0f);
    ESIA_CHECK(s1 == s2);   // the box at scale 1: layout does not animate
    const std::vector<Quad> qa = Quads(a), qb = Quads(b);
    ESIA_CHECK(qa.size() == 2 && qb.size() == 2);
    for (std::size_t i = 0; i < qa.size() && i < qb.size(); ++i)
    {
        // new bitmaps at twice the size (texel for pixel), not stretched ones, placed around `pos`
        ESIA_CHECK_NEAR(qb[i].r.Height(), 2.0f * qa[i].r.Height(), 3.0f);
        ESIA_CHECK_NEAR(qb[i].r.Height(), (qb[i].uv1.y - qb[i].uv0.y) * 2048.0f, 1e-3f);
        ESIA_CHECK_NEAR(qb[i].r.min.x - pos.x, 2.0f * (qa[i].r.min.x - pos.x), 2.5f);
    }
}

ESIA_TEST(FreeType, IconGlyphsAreOpticallyCentered)
{
    Fixture f;
    DrawList dl = NewList();
    f.ts->DrawGlyph(dl, {f.droid, 24.0f}, U'O', Vec2(50.0f, 40.0f), Color::Black());
    const std::vector<Quad> q = Quads(dl);
    ESIA_CHECK(q.size() == 1);
    // the ink's center on the point (the bitmap box has one empty pixel left and above, two right, one below)
    ESIA_CHECK_NEAR(q.at(0).r.Center().x, 50.5f, 1.0f);
    ESIA_CHECK_NEAR(q.at(0).r.Center().y, 40.0f, 1.0f);
    // a code point no font has draws nothing (no .notdef box for icons)
    DrawList none = NewList();
    f.ts->DrawGlyph(none, {f.droid, 24.0f}, 0x2665, Vec2(50.0f, 40.0f), Color::Black());
    ESIA_CHECK(none.Empty());
}

// Color bitmap strikes (sbix / CBDT): an installed color emoji font draws its glyphs in color, from an RGBA8 page, in
// white with the text's alpha. Apple Color Emoji where the system has it (macOS), else skipped.
ESIA_TEST(FreeType, ColorEmojiFromBitmapStrikes)
{
    const std::optional<text::SystemFont> emoji = text::FindSystemFont("Apple Color Emoji");
    if (!emoji)
    {
        std::printf("  skipped: no Apple Color Emoji\n");
        return;
    }
    Fixture f;
    const text::FontId id = text::AddSystemFont(*f.ts, *emoji);
    ESIA_CHECK(id != 0);   // a font without outlines loads when it has color strikes
    f.ts->AddFallback(id);
    DrawList dl = NewList();
    f.ts->Draw(dl, {f.droid, 32.0f}, Vec2(10, 10), Color(0.2f, 0.4f, 0.6f, 0.5f), "A\xF0\x9F\x9A\x80");   // A + rocket
    const std::vector<Quad> q = Quads(dl);
    ESIA_CHECK(q.size() == 2);
    if (q.size() != 2)
        return;
    TextureInfo letter, rocket;
    ESIA_CHECK(f.textures.Info(q[0].texture, letter) && letter.Coverage());
    ESIA_CHECK(f.textures.Info(q[1].texture, rocket) && rocket.format == TextureFormat::RGBA8);
    ESIA_CHECK(q[1].color == Color::White(0.5f).ToRgba8() && q[0].color == Color(0.2f, 0.4f, 0.6f, 0.5f).ToRgba8());
    // about one em, on the line
    ESIA_CHECK(q[1].r.Width() > 20.0f && q[1].r.Width() < 44.0f && q[1].r.min.y < q[0].r.max.y && q[1].r.max.y > q[0].r.min.y);
}

ESIA_TEST(FreeType, CachesLayoutsAndGlyphs)
{
    Fixture f;
    const text::FontRef fr{f.droid, 14.0f};
    std::vector<TextureChange> changes;
    DrawList dl = NewList();
    f.ts->Draw(dl, fr, Vec2(0, 0), Color::Black(), "cache me");
    f.textures.TakeChanges(changes);
    ESIA_CHECK(changes.size() > 1 && changes[0].kind == TextureChange::Kind::Create);
    // the same text again, this frame and the next: no new uploads
    f.ts->Draw(dl, fr, Vec2(0, 20), Color::Black(), "cache me");
    f.ts->NewFrame({});
    f.ts->Draw(dl, fr, Vec2(0, 40), Color::Black(), "cache me");
    f.textures.TakeChanges(changes);
    ESIA_CHECK(changes.empty());
    // another sub-pixel phase is another bitmap
    f.ts->Draw(dl, fr, Vec2(0.25f, 60), Color::Black(), "cache me");
    f.textures.TakeChanges(changes);
    ESIA_CHECK(!changes.empty());
}

ESIA_TEST(FreeType, AtlasStartsOverWhenFull)
{
    text::FreeTypeDesc desc;
    desc.atlasPageSize = 128;
    desc.atlasMaxPages = 1;
    Fixture f(desc);
    DrawList dl = NewList();
    for (int size = 10; size <= 40; size += 2)
        f.ts->Draw(dl, {f.droid, (float)size}, Vec2(0, 0), Color::Black(), "Hamburgefonstiv");
    ESIA_CHECK(f.textures.Count() > 1);   // overflow pages: every glyph of this frame stays valid
    f.ts->NewFrame({});
    ESIA_CHECK(f.textures.Count() == 1);
    DrawList next = NewList();
    f.ts->Draw(next, {f.droid, 12.0f}, Vec2(0, 0), Color::Black(), "abc");
    ESIA_CHECK(Quads(next).size() == 3);
}

namespace
{
    // One sample of everything: kerning, accents and marks, Greek, Cyrillic, a fallback glyph, wrapping,
    // trimming, centering, sub-pixel phases and an icon.
    void DrawSample(text::TextSystem& ts, text::FontId droid, DrawList& dl)
    {
        const Color ink(0.08f, 0.08f, 0.10f, 1.0f);
        ts.Draw(dl, {droid, 24.0f}, Vec2(8, 6), ink, "Esia \xC2\xB7 AV To Wa fi \xC3\x80\xC3\x89\xC3\x8E\xC3\xB5\xC3\xBC");
        ts.Draw(dl, {droid, 15.0f}, Vec2(8, 38), Color(0.0f, 0.35f, 0.8f, 1.0f),
                "The quick brown fox jumps over the lazy dog, again and again and again.", 300.0f);
        ts.Draw(dl, {droid, 13.0f}, Vec2(8, 82), ink,
                "\xCE\xA9\xCE\xBC\xCE\xAD\xCE\xB3\xCE\xB1 \xC2\xB7 \xD0\x96\xD0\xB8\xD0\xB7\xD0\xBD\xD1\x8C \xC2\xB7 caf\xC3\xA9 e\xCC\x81 \xC2\xB7 "
                "1\xE2\x80\x93" "2 \xE2\x82\xAC \xE2\x88\x9E \xE2\x82\xB9");
        ts.Draw(dl, {droid, 14.0f}, Vec2(8, 104), ink, "Trimmed with an ellipsis when it is far too long", 200.0f, text::TextFlags_Ellipsis);
        ts.Draw(dl, {droid, 16.0f}, Vec2(240, 104), Color(0.75f, 0.1f, 0.2f, 1.0f), "Centered\nlines", 200.0f, text::TextFlags_AlignCenter);
        for (int i = 0; i < 4; ++i)
            ts.Draw(dl, {droid, 12.0f}, Vec2(8.0f + 0.25f * (float)i, 130.0f + 14.0f * (float)i), ink, "iiiiiiiiii llllllllll");
        ts.DrawGlyph(dl, {droid, 28.0f}, 0x221E, Vec2(440, 150), ink);
    }
}

ESIA_TEST(FreeType, RendersTheGoldenImage)
{
    Fixture f;
    const text::FontId karla = f.ts->AddFontFile((kFonts + "/Karla-Regular.ttf").c_str());
    f.ts->AddFallback(karla);
    DrawList dl = NewList();
    DrawSample(*f.ts, f.droid, dl);
    PageStore pages;
    pages.Collect(f.textures);
    CheckGolden(Composite(dl, pages, 480, 190), "ft_gray");
}

ESIA_TEST(FreeType, CaretStopsAtGraphemesWhereTheGlyphsAre)
{
    Fixture f;
    const text::FontRef fr{f.droid, 16.0f};
    std::vector<text::CaretStop> s;

    // one stop per character, where the text before it ends (its trailing space included)
    const std::string_view t = "ab c";
    f.ts->CaretStops(fr, t, s);
    ESIA_CHECK(s.size() == 5);
    for (std::size_t i = 0; i < s.size() && i < 5; ++i)
    {
        ESIA_CHECK(s[i].offset == i);
        ESIA_CHECK_NEAR(s[i].x, i > 0 ? f.ts->Measure(fr, t.substr(0, i)).size.x : 0.0f, 1e-3f);
    }

    // a combining mark belongs to the character before it: e + U+0301 is one grapheme
    f.ts->CaretStops(fr, "e\xCC\x81x", s);
    ESIA_CHECK(s.size() == 3 && s[1].offset == 3 && s[2].offset == 4);
    ESIA_CHECK(s.size() == 3 && std::fabs(s[1].x - f.ts->Measure(fr, "e").size.x) < 1e-3f);

    // kerned: the stop between A and V is where V is drawn, the last one the kerned width
    f.ts->CaretStops(fr, "AV", s);
    ESIA_CHECK(s.size() == 3 && s[1].x > 0.0f && s[1].x < s[2].x);
    ESIA_CHECK(s.size() == 3 && std::fabs(s[2].x - f.ts->Measure(fr, "AV").size.x) < 1e-3f);

    // a flag is two regional indicators, one grapheme (the font has no glyph for it)
    f.ts->CaretStops(fr, "\xF0\x9F\x87\xAF\xF0\x9F\x87\xB5" "a", s);
    ESIA_CHECK(s.size() == 3 && s[1].offset == 8 && s[2].offset == 9);

    // nothing: one stop at 0
    f.ts->CaretStops(fr, "", s);
    ESIA_CHECK(s.size() == 1 && s[0].offset == 0 && s[0].x == 0.0f);

    // a ligature (when the font has one for "fi") is split evenly between its characters
    DrawList dl = NewList();
    f.ts->Draw(dl, fr, Vec2(0, 0), Color::Black(), "fi");
    if (Quads(dl).size() == 1)
    {
        f.ts->CaretStops(fr, "fi", s);
        ESIA_CHECK(s.size() == 3 && std::fabs(s[1].x - s[2].x * 0.5f) < 1e-3f);
    }
}

namespace
{
    // A text system of fixed-width characters: what the interface's default CaretStops works on
    struct MonoTextSystem final : text::TextSystem
    {
        text::FontId AddFontFile(const char*, int) override { return 1; }
        text::FontId AddFontMemory(const void*, std::size_t, int) override { return 1; }
        void AddFallback(text::FontId) override {}
        void NewFrame(const text::RasterParams&) override {}
        text::TextMetrics Measure(text::FontRef, std::string_view text, float, std::uint32_t) override
        {
            float n = 0.0f;
            for (std::size_t i = 0; i < text.size();)
            {
                DecodeUtf8(text, i);
                n += 1.0f;
            }
            return {Vec2(10.0f * n, 16.0f), 12.0f, 1};
        }
        Vec2 Draw(DrawList&, text::FontRef, Vec2, Color, std::string_view, float, std::uint32_t, float) override { return Vec2(); }
        void DrawGlyph(DrawList&, text::FontRef, char32_t, Vec2, Color) override {}
    };
}

ESIA_TEST(TextSystem, DefaultCaretStopsAtEveryCodePoint)
{
    MonoTextSystem ts;
    std::vector<text::CaretStop> s;
    ts.CaretStops({1, 16.0f}, "a\xC3\xA9z", s);   // a, U+00E9 (2 bytes), z
    ESIA_CHECK(s.size() == 4);
    if (s.size() == 4)
    {
        ESIA_CHECK(s[0].offset == 0 && s[1].offset == 1 && s[2].offset == 3 && s[3].offset == 4);
        ESIA_CHECK(s[0].x == 0.0f && s[1].x == 10.0f && s[2].x == 20.0f && s[3].x == 30.0f);
    }
}

ESIA_TEST(FreeType, NeutralsAroundARightToLeftWordStayInPlace)
{
    // "a <Arabic AIN BEH> b": the Arabic word is drawn right to left (the font has no Arabic: its glyphs are the
    // notdef box, the direction comes from the script), the spaces around it stay where they were typed
    Fixture f;
    const text::FontRef fr{f.droid, 16.0f};
    const std::string_view t = "a \xD8\xB9\xD8\xA8 b";   // offsets: a 0, space 1, AIN 2, BEH 4, space 6, b 7, end 8
    std::vector<text::CaretStop> s;
    f.ts->CaretStops(fr, t, s);
    ESIA_CHECK(s.size() == 7);
    if (s.size() != 7)
        return;
    const float space = f.ts->Measure(fr, " ").size.x;
    const float word = s[1].x;   // "a" + nothing: the caret before the first space
    ESIA_CHECK(s[1].offset == 1 && s[2].offset == 2 && s[3].offset == 4 && s[4].offset == 6 && s[5].offset == 7);
    // inside the Arabic word the caret moves left as the offset grows: AIN is its rightmost glyph
    ESIA_CHECK(s[2].x > s[3].x && s[3].x > word + space - 1e-3f);
    // the space after it is right of the whole word, then b
    ESIA_CHECK(s[4].x >= s[2].x - 1e-3f && std::fabs(s[5].x - s[4].x - space) < 1e-3f && s[6].x > s[5].x);
}
