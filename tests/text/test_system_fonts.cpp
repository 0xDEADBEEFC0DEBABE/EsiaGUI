// The system font lookup (system_fonts.hpp): families found by any of their names in font directories, collections,
// the face chosen by style and weight (CSS rules) - with the fonts in tests/fonts and collections made from them -, and
// the platform's own lookup where its fonts are installed.
#include "esia/text/system_fonts.hpp"
#include "esia_test.hpp"
#include "font_test_util.hpp"
#include <cstdio>
#include <filesystem>

using namespace esia;
using namespace esia::texttest;

namespace
{
    std::string FileName(const std::string& path) { return std::filesystem::path(path).filename().string(); }

    // a directory of its own under the build tree, holding `name` = `bytes`
    std::string MakeFontDirectory(const std::string& dir, const std::string& name, const std::vector<std::uint8_t>& bytes)
    {
        const std::string path = std::string(ESIA_TEXT_OUT_DIR) + "/" + dir;
        std::filesystem::remove_all(path);
        std::filesystem::create_directories(path);
        ESIA_CHECK(WriteFile(path + "/" + name, bytes));
        return path;
    }
}

ESIA_TEST(SystemFonts, FindsFamiliesInDirectories)
{
    const std::vector<std::string> dirs = {kFonts};
    const std::optional<text::SystemFont> droid = text::FindFontInDirectories(dirs, "Droid Sans");
    ESIA_CHECK(droid && FileName(droid->path) == "DroidSans.ttf" && droid->faceIndex == 0);
    ESIA_CHECK(droid && droid->family == "Droid Sans" && droid->weight == 400 && droid->style == text::FontStyle::Upright);
    ESIA_CHECK(droid && std::filesystem::exists(droid->path));

    // ASCII case does not matter; a CFF-outline OpenType font is found like a TrueType one
    const std::optional<text::SystemFont> karla = text::FindFontInDirectories(dirs, "KARLA");
    ESIA_CHECK(karla && FileName(karla->path) == "Karla-Regular.ttf" && karla->family == "Karla");
    const std::optional<text::SystemFont> noto = text::FindFontInDirectories(dirs, "noto sans sc");
    ESIA_CHECK(noto && FileName(noto->path) == "NotoSansSC-Subset.otf" && noto->family == "Noto Sans SC");

    // a family's only face serves every request; nothing is substituted for a missing family
    const std::optional<text::SystemFont> bold = text::FindFontInDirectories(dirs, "Droid Sans", 700, text::FontStyle::Italic);
    ESIA_CHECK(bold && bold->path == droid->path && bold->weight == 400);
    ESIA_CHECK(!text::FindFontInDirectories(dirs, "Droid"));
    ESIA_CHECK(!text::FindFontInDirectories(dirs, "No Such Family"));
    ESIA_CHECK(!text::FindFontInDirectories(dirs, ""));
    const std::vector<std::string> missing = {kFonts + "/missing-directory"};
    ESIA_CHECK(!text::FindFontInDirectories(missing, "Droid Sans"));
}

ESIA_TEST(SystemFonts, FindsFacesInCollections)
{
    const std::string dir = MakeFontDirectory("fonts-ttc", "droid-karla.TTC",
                                              MakeCollection({{ReadFile(kFonts + "/DroidSans.ttf")}, {ReadFile(kFonts + "/Karla-Regular.ttf")}}));
    const std::vector<std::string> dirs = {dir};
    const std::optional<text::SystemFont> droid = text::FindFontInDirectories(dirs, "Droid Sans");
    const std::optional<text::SystemFont> karla = text::FindFontInDirectories(dirs, "Karla");
    ESIA_CHECK(droid && droid->faceIndex == 0 && FileName(droid->path) == "droid-karla.TTC");
    ESIA_CHECK(karla && karla->faceIndex == 1 && karla->path == droid->path);
}

ESIA_TEST(SystemFonts, ChoosesTheFaceByStyleThenWeight)
{
    // one family in five faces: light, regular, bold, black and a regular italic
    const std::vector<std::uint8_t> droid = ReadFile(kFonts + "/DroidSans.ttf");
    const std::string dir = MakeFontDirectory("fonts-weights", "droid-family.ttc",
                                              MakeCollection({{droid, 300}, {droid, 400}, {droid, 700}, {droid, 900}, {droid, 400, true}}));
    const std::vector<std::string> dirs = {dir};
    auto face = [&](int weight, text::FontStyle style = text::FontStyle::Upright) {
        const std::optional<text::SystemFont> f = text::FindFontInDirectories(dirs, "Droid Sans", weight, style);
        return f ? f->faceIndex : -1;
    };
    ESIA_CHECK(face(400) == 1);
    ESIA_CHECK(face(700) == 2);
    // CSS: 400 - 500 look up to 500, then down, then up; below 400 down first; above 500 up first
    ESIA_CHECK(face(500) == 1);
    ESIA_CHECK(face(450) == 1);
    ESIA_CHECK(face(350) == 0);
    ESIA_CHECK(face(200) == 0);
    ESIA_CHECK(face(600) == 2);
    ESIA_CHECK(face(800) == 3);
    ESIA_CHECK(face(950) == 3);
    // style before weight: the only italic face answers every italic request
    ESIA_CHECK(face(400, text::FontStyle::Italic) == 4);
    ESIA_CHECK(face(900, text::FontStyle::Italic) == 4);
    const std::optional<text::SystemFont> italic = text::FindFontInDirectories(dirs, "Droid Sans", 400, text::FontStyle::Italic);
    ESIA_CHECK(italic && italic->style == text::FontStyle::Italic && italic->weight == 400);
    const std::optional<text::SystemFont> black = text::FindFontInDirectories(dirs, "Droid Sans", 900);
    ESIA_CHECK(black && black->weight == 900 && black->style == text::FontStyle::Upright);
}

ESIA_TEST(SystemFonts, PlatformLookup)
{
    // no family is substituted (fontconfig would answer with its default font)
    ESIA_CHECK(!text::FindSystemFont("Esia No Such Family 0xC0DE"));
    ESIA_CHECK(!text::FindSystemFont(""));

    const std::span<const std::string_view> chain = text::DefaultFallbackFamilies();
    ESIA_CHECK(chain.size() >= 5);
    int found = 0;
    for (const std::string_view family : chain)
    {
        const std::optional<text::SystemFont> font = text::FindSystemFont(family);
        if (!font)
        {
            std::printf("  %s: %.*s is not installed\n", PlatformShipsItsChain() ? "FAILED" : "skipped", (int)family.size(), family.data());
            ESIA_CHECK(!PlatformShipsItsChain());
            continue;
        }
        ++found;
        std::printf("  %.*s: %s (face %d, weight %d)\n", (int)family.size(), family.data(), font->path.c_str(), font->faceIndex, font->weight);
        std::error_code ec;
        ESIA_CHECK(std::filesystem::is_regular_file(std::filesystem::path(std::u8string_view(reinterpret_cast<const char8_t*>(font->path.data()), font->path.size())), ec));
        ESIA_CHECK(font->faceIndex >= 0 && !font->family.empty());
        // asking for the family it reports finds the same face (not for system-ui: macOS reports a hidden family)
        if (family == text::kSystemUiFamily)
            continue;
        const std::optional<text::SystemFont> again = text::FindSystemFont(font->family, font->weight, font->style);
        ESIA_CHECK(again && again->path == font->path && again->faceIndex == font->faceIndex);
    }
#if defined(_WIN32)
    // Windows and Wine both ship Tahoma: DirectWrite's lookup itself is checked even where the chain is missing
    const std::optional<text::SystemFont> tahoma = text::FindSystemFont("tahoma");
    ESIA_CHECK(tahoma && tahoma->family == "Tahoma" && tahoma->faceIndex == 0);
    // Tahoma has no italic: DirectWrite answers with an oblique it simulates, but the file's face is upright, and
    // that is what a text system gets
    const std::optional<text::SystemFont> oblique = text::FindSystemFont("Tahoma", 400, text::FontStyle::Italic);
    ESIA_CHECK(oblique && oblique->style == text::FontStyle::Upright && oblique->weight == 400);
#endif
    // a family by its localized name: the same face as by its English one
    const std::pair<const char*, const char*> localized[] = {{"Microsoft YaHei", "微软雅黑"}, {"WenQuanYi Zen Hei", "文泉驛正黑"}};
    for (const auto& [english, native] : localized)
    {
        const std::optional<text::SystemFont> a = text::FindSystemFont(english);
        if (!a)
            continue;
        const std::optional<text::SystemFont> b = text::FindSystemFont(native);
        std::printf("  %s = %s: %s\n", english, native, b ? b->path.c_str() : "not found");
        ESIA_CHECK(b && b->path == a->path && b->faceIndex == a->faceIndex);
    }

    const std::vector<text::SystemFont> fonts = text::FindDefaultFallbackFonts();
    ESIA_CHECK((int)fonts.size() <= found && (found == 0) == fonts.empty());
    if (PlatformShipsItsChain())
        ESIA_CHECK(text::FindSystemFont(text::kSystemUiFamily).has_value());
}
