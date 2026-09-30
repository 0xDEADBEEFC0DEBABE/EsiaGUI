// Chinese, Japanese and Korean text with the FreeType + HarfBuzz text system: NotoSansSC-Subset.otf (tests/fonts, a
// CFF-outline OpenType font of a few dozen characters) shaped alone and as the fallback of a Latin font, line breaks
// between CJK characters with the kinsoku rules, trimming, and a golden image of mixed Chinese and Latin text. Then the
// platform's own fallback chain (system_fonts.hpp), where its fonts are installed.
#include "esia/text/freetype.hpp"
#include "esia/text/system_fonts.hpp"
#include "font_test_util.hpp"
#include "ft_test_util.hpp"

using namespace esia;
using namespace esia::texttest;

namespace
{
    // Noto Sans SC: 1000 units per em, every ideograph and full-width punctuation mark 1000 units wide; its line box
    // is 1160 + 288 units (hhea ascent / descent, no gap)
    constexpr float kSize = 20.0f;

    struct Fixture
    {
        TextureRegistry textures;
        std::unique_ptr<text::TextSystem> ts;
        text::FontId droid = 0, noto = 0;

        Fixture() : ts(text::CreateFreeTypeTextSystem(textures))
        {
            ESIA_CHECK(ts != nullptr);
            droid = ts->AddFontFile((kFonts + "/DroidSans.ttf").c_str());
            noto = ts->AddFontFile((kFonts + "/NotoSansSC-Subset.otf").c_str());
            ESIA_CHECK(droid != 0 && noto != 0);
            ts->NewFrame({});
        }
    };

    // the left edges of the quads, sorted (the glyphs' pen positions plus their bitmaps' left bearing)
    std::vector<float> Lefts(const DrawList& dl)
    {
        std::vector<float> x;
        for (const Quad& q : Quads(dl))
            x.push_back(q.r.min.x);
        std::sort(x.begin(), x.end());
        return x;
    }
}

ESIA_TEST(FreeTypeCjk, ShapesOneFullWidthGlyphPerCharacter)
{
    Fixture f;
    const text::FontRef fr{f.noto, kSize};
    const text::TextMetrics m = f.ts->Measure(fr, "中文排版");
    ESIA_CHECK_NEAR(m.size.x, 4.0f * kSize, 1e-3f);
    ESIA_CHECK_NEAR(m.size.y, 1.448f * kSize, 1e-3f);
    ESIA_CHECK_NEAR(m.baseline, 1.16f * kSize, 1e-3f);
    ESIA_CHECK(m.lines == 1);
    // full-width punctuation, Traditional characters and kana: one em each
    for (const char* s : {"，", "。", "「", "》", "體", "の", "カ"})
        ESIA_CHECK_NEAR(f.ts->Measure(fr, s).size.x, kSize, 1e-3f);
    // the ideographic space is a space: one em, no glyph
    ESIA_CHECK_NEAR(f.ts->Measure(fr, "中\xE3\x80\x80文").size.x, 3.0f * kSize, 1e-3f);

    // one quad per character, a whole em apart (the pen at 0, 20, 40, 60: every glyph at phase 0)
    DrawList dl = NewList();
    f.ts->Draw(dl, fr, Vec2(10, 10), Color::Black(), "中文排版");
    const std::vector<float> x = Lefts(dl);
    ESIA_CHECK(x.size() == 4);
    for (std::size_t i = 1; i < x.size(); ++i)
        ESIA_CHECK_NEAR(x[i] - x[i - 1], kSize, 1.0f);
    for (const Quad& q : Quads(dl))
        ESIA_CHECK(q.r.Width() >= 0.75f * kSize && q.r.Width() <= kSize + 3.0f);   // ideographs fill most of their em
    DrawList space = NewList();
    f.ts->Draw(space, fr, Vec2(10, 10), Color::Black(), "中\xE3\x80\x80文");
    ESIA_CHECK(Quads(space).size() == 2);
}

ESIA_TEST(FreeTypeCjk, FallsBackFromALatinFont)
{
    Fixture f;
    const text::FontRef fr{f.droid, kSize};
    const char* text = "Esia 中文";
    const text::TextMetrics before = f.ts->Measure(fr, text);   // DroidSans' .notdef boxes
    f.ts->AddFallback(f.noto);
    const text::TextMetrics after = f.ts->Measure(fr, text);
    ESIA_CHECK(after.size.x != before.size.x);
    ESIA_CHECK_NEAR(after.size.x, f.ts->Measure(fr, "Esia ").size.x + 2.0f * kSize, 1e-3f);
    // the requested font's line box, whatever the fallback's
    ESIA_CHECK(after.size.y == before.size.y && after.baseline == before.baseline);
    ESIA_CHECK(after.size.y != f.ts->Measure({f.noto, kSize}, "中").size.y);

    // digits stay with DroidSans between the ideographs, and so does a space: the requested font has one
    ESIA_CHECK_NEAR(f.ts->Measure(fr, "2026年9月").size.x, f.ts->Measure(fr, "20269").size.x + 2.0f * kSize, 1e-3f);
    const float space = f.ts->Measure(fr, "E E").size.x - f.ts->Measure(fr, "EE").size.x;
    ESIA_CHECK(space > 0.0f && std::fabs(space - f.ts->Measure({f.noto, kSize}, "中 文").size.x + 2.0f * kSize) > 0.5f);   // Noto's differs
    ESIA_CHECK_NEAR(f.ts->Measure(fr, "中 文").size.x, 2.0f * kSize + space, 1e-3f);

    DrawList dl = NewList();
    f.ts->Draw(dl, fr, Vec2(0, 0), Color::Black(), text);
    const std::vector<Quad> q = Quads(dl);
    ESIA_CHECK(q.size() == 6);   // E s i a 中 文 (the space draws nothing)
    if (q.size() == 6)
    {
        // the ideographs are Noto's, an em wide and on DroidSans' baseline: their ink reaches below "E"'s
        ESIA_CHECK(q[4].r.Width() >= 0.75f * kSize && q[5].r.Width() >= 0.75f * kSize);
        ESIA_CHECK_NEAR(q[5].r.min.x - q[4].r.min.x, kSize, 1.0f);
        ESIA_CHECK(q[4].r.max.y > q[0].r.max.y);
    }
}

ESIA_TEST(FreeTypeCjk, BreaksLinesBetweenCharacters)
{
    Fixture f;
    const text::FontRef fr{f.noto, kSize};
    const float lh = f.ts->Measure(fr, "中").size.y;
    auto lines = [&](const char* s, float width) {
        DrawList dl = NewList();
        f.ts->Draw(dl, fr, Vec2(0, 0), Color::Black(), s, width);
        return QuadsPerLine(Quads(dl), lh);
    };

    // any two ideographs may part: three to a line of 3.5 em
    const text::TextMetrics m = f.ts->Measure(fr, "中文字体测试排版", 3.5f * kSize);
    ESIA_CHECK(m.lines == 3);
    ESIA_CHECK_NEAR(m.size.x, 3.0f * kSize, 1e-3f);
    ESIA_CHECK(lines("中文字体测试排版", 3.5f * kSize) == std::vector<int>({3, 3, 2}));

    // kinsoku: closing punctuation never starts a line, opening brackets never end one
    ESIA_CHECK(lines("中文，测试。", 2.5f * kSize) == std::vector<int>({1, 2, 1, 2}));
    ESIA_CHECK(lines("看「文字」", 2.5f * kSize) == std::vector<int>({1, 2, 2}));
    ESIA_CHECK(lines("「中文」、「测试」", 5.5f * kSize) == std::vector<int>({5, 4}));

    // a Latin word keeps together; the ideographs after it may start the next line
    f.ts->AddFallback(f.droid);
    const float esia = f.ts->Measure(fr, "Esia").size.x;
    ESIA_CHECK(lines("Esia是界面库", esia + 1.5f * kSize) == std::vector<int>({5, 3}));
    ESIA_CHECK(lines("界面Esia", 2.5f * kSize) == std::vector<int>({2, 4}));
}

ESIA_TEST(FreeTypeCjk, TrimsWithAnEllipsis)
{
    Fixture f;
    const text::FontRef fr{f.noto, kSize};
    // five ems: four ideographs and Noto's full-width U+2026
    const text::TextMetrics m = f.ts->Measure(fr, "这是一段很长的文字", 5.0f * kSize, text::TextFlags_Ellipsis);
    ESIA_CHECK(m.lines == 1);
    ESIA_CHECK_NEAR(m.size.x, 5.0f * kSize, 1e-3f);
    DrawList dl = NewList();
    f.ts->Draw(dl, fr, Vec2(0, 0), Color::Black(), "这是一段很长的文字", 5.0f * kSize, text::TextFlags_Ellipsis);
    const std::vector<float> x = Lefts(dl);
    ESIA_CHECK(x.size() == 5 && x.back() >= 4.0f * kSize - 1.0f);
}

namespace
{
    // Chinese as the owner writes it: mixed with Latin, wrapped, punctuated, trimmed and centered; Traditional
    // characters, kana and Hangul from the same font; an ideograph as an icon.
    void DrawCjkSample(text::TextSystem& ts, text::FontId droid, text::FontId noto, DrawList& dl)
    {
        const Color ink(0.08f, 0.08f, 0.10f, 1.0f);
        ts.Draw(dl, {droid, 24.0f}, Vec2(8, 6), ink, "Esia \xC2\xB7 液态玻璃界面");
        ts.Draw(dl, {noto, 15.0f}, Vec2(8, 40), Color(0.0f, 0.35f, 0.8f, 1.0f),
                "中文排版：汉字之间可以换行，标点符号不会出现在行首。英文单词 Esia 与中文混排。", 300.0f);
        ts.Draw(dl, {noto, 15.0f}, Vec2(8, 90), ink, "繁體中文：介面與字型　日本語のかな");
        ts.Draw(dl, {noto, 14.0f}, Vec2(8, 118), ink, "这是一段很长的文字，超出宽度时以省略号结尾", 200.0f, text::TextFlags_Ellipsis);
        ts.Draw(dl, {noto, 16.0f}, Vec2(240, 112), Color(0.75f, 0.1f, 0.2f, 1.0f), "居中\n对齐", 200.0f, text::TextFlags_AlignCenter);
        for (int i = 0; i < 2; ++i)
            ts.Draw(dl, {noto, 12.0f}, Vec2(8.0f + 0.25f * (float)i, 144.0f + 16.0f * (float)i), ink, "「引号」《书名》（括号）");
        ts.DrawGlyph(dl, {noto, 28.0f}, U'字', Vec2(440, 160), ink);
    }
}

ESIA_TEST(FreeTypeCjk, RendersTheGoldenImage)
{
    Fixture f;
    f.ts->AddFallback(f.noto);
    f.ts->AddFallback(f.droid);
    DrawList dl = NewList();
    DrawCjkSample(*f.ts, f.droid, f.noto, dl);
    PageStore pages;
    pages.Collect(f.textures);
    CheckGolden(Composite(dl, pages, 480, 190), "ft_cjk");
}

// ------------------------------------------------------------------ the platform's fallback chain
ESIA_TEST(SystemFonts, FallbackChainCoversEveryScript)
{
    const std::vector<text::SystemFont> fonts = text::FindDefaultFallbackFonts();
    if (fonts.empty())
    {
        ESIA_CHECK(!PlatformShipsItsChain());
        std::printf("  skipped: no font of the default fallback chain is installed\n");
        return;
    }
    TextureRegistry textures;
    std::unique_ptr<text::TextSystem> ts = text::CreateFreeTypeTextSystem(textures);
    const std::vector<text::FontId> ids = text::AddFallbackFonts(*ts, fonts);
    ESIA_CHECK(ids.size() == fonts.size());   // every font found loads
    ts->NewFrame({});
    for (const text::SystemFont& font : fonts)
        std::printf("  %s: %s (face %d, weight %d)\n", font.family.c_str(), font.path.c_str(), font.faceIndex, font.weight);

    // DrawGlyph draws nothing when no font of the chain has the character
    const struct
    {
        const char* script;
        char32_t c;
    } samples[] = {{"Latin", U'A'},    {"Simplified Chinese", U'语'}, {"Traditional Chinese", U'體'}, {"Japanese kana", U'の'},
                   {"Korean", U'한'}, {"symbols", U'→'},
#if defined(_WIN32)
                   // the scripts of WGT's Languages panel that Segoe UI does not have
                   {"Devanagari", U'ह'}, {"Thai", U'ไ'},
#endif
    };
    for (const auto& s : samples)
    {
        DrawList dl = NewList();
        ts->DrawGlyph(dl, {ids.empty() ? 0 : ids[0], 24.0f}, s.c, Vec2(20, 20), Color::Black());
        if (!dl.Empty())
            continue;
        std::printf("  %s: no installed font of the chain covers %s (U+%04X)\n", PlatformShipsItsChain() ? "FAILED" : "skipped", s.script, (unsigned)s.c);
        ESIA_CHECK(!PlatformShipsItsChain());
    }
}

#if defined(_WIN32)
ESIA_TEST(SystemFonts, KanaTakeAJapaneseFont)
{
    // Microsoft YaHei comes first among Windows' CJK fonts and has kana too (GB 18030), full width; they go to Yu
    // Gothic UI, as with DirectWrite's own fallback
    const std::vector<text::SystemFont> fonts = text::FindDefaultFallbackFonts();
    const auto installed = [&](std::string_view family) {
        return std::any_of(fonts.begin(), fonts.end(), [&](const text::SystemFont& f) { return f.family == family; });
    };
    if (!installed("Microsoft YaHei") || !installed("Yu Gothic UI"))
    {
        std::printf("  skipped: Microsoft YaHei or Yu Gothic UI is not installed\n");
        return;
    }
    TextureRegistry textures;
    std::unique_ptr<text::TextSystem> ts = text::CreateFreeTypeTextSystem(textures);
    const std::vector<text::FontId> ids = text::AddFallbackFonts(*ts, fonts);
    ts->NewFrame({});
    const text::FontRef fr{ids[0], kSize};
    ESIA_CHECK(ts->Measure(fr, "リキッド").size.x < 3.6f * kSize);   // Yu Gothic UI's kana are proportional
    ESIA_CHECK_NEAR(ts->Measure(fr, "中文").size.x, 2.0f * kSize, 1e-3f);

    // CJK punctuation (and Han) take the font of the paragraph's CJK text before them: the ideographic comma is
    // 0.664 em in Yu Gothic UI, an em in YaHei
    const auto comma = [&](const char* before, const char* with) { return ts->Measure(fr, with).size.x - ts->Measure(fr, before).size.x; };
    ESIA_CHECK_NEAR(comma("の", "の、"), 0.664f * kSize, 0.05f * kSize);
    ESIA_CHECK_NEAR(comma("の UI", "の UI、"), 0.664f * kSize, 0.05f * kSize);   // across Latin text
    ESIA_CHECK_NEAR(comma("中", "中、"), kSize, 0.05f * kSize);
}
#endif
