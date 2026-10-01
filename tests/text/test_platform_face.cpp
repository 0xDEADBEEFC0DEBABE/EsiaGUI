// The platform's faces (src/esia/text/ft/platform_face.hpp): system fonts FreeType cannot draw, through Core Text on
// Apple - checked against the same fonts through FreeType where FreeType can read them (macOS' files).
#include "platform_face.hpp"
#include "esia/text/freetype.hpp"
#include "esia/text/system_fonts.hpp"
#include "ft_test_util.hpp"
#include <cstdio>
#include <cstdlib>

using namespace esia;
using namespace esia::texttest;

ESIA_TEST(PlatformFace, OnlyFontsCoreTextHas)
{
    ESIA_CHECK(!text::detail::OpenPlatformFace("/System/Library/Fonts/Helvetica.ttc"));   // a file: FreeType's
    ESIA_CHECK(!text::detail::OpenPlatformFace("coretext:EsiaNoSuchFont-Regular"));       // Core Text would substitute
}

// iOS' Apple Color Emoji has its bitmaps in Apple's emjc compression, which FreeType cannot decode: Core Text draws
// them. macOS' copy has PNG strikes, so the same glyph through FreeType shows where Core Text's has to be.
ESIA_TEST(PlatformFace, ColorGlyphsAsFreeTypeDecodesThem)
{
    std::unique_ptr<text::detail::PlatformFace> emoji = text::detail::OpenPlatformFace("coretext:AppleColorEmoji");
    if (!emoji)
    {
        std::printf("  skipped: no Core Text\n");
        return;
    }
    hb_face_t* face = emoji->CreateHbFace();
    hb_font_t* font = hb_font_create(face);
    hb_face_destroy(face);
    hb_codepoint_t rocket = 0;
    ESIA_CHECK(hb_font_get_nominal_glyph(font, 0x1F680, &rocket));
    hb_font_destroy(font);
    text::GlyphBitmap b;
    ESIA_CHECK(emoji->ColorGlyph((std::uint16_t)rocket, 64.0f, b));
    ESIA_CHECK(b.channels == 4 && b.pixels.size() == (std::size_t)b.width * (std::size_t)b.height * 4);
    ESIA_CHECK(b.width > 48 && b.width <= 72 && b.height > 48 && b.height <= 72);
    ESIA_CHECK(b.top < -40 && b.top + b.height > -20);   // on the baseline, mostly above it
    int colored = 0;   // opaque pixels that are no gray: in color
    for (std::size_t i = 0; i < b.pixels.size(); i += 4)
        if (b.pixels[i + 3] > 200 && (std::abs(b.pixels[i] - b.pixels[i + 1]) > 60 || std::abs(b.pixels[i + 1] - b.pixels[i + 2]) > 60))
            ++colored;
    ESIA_CHECK(colored > 100);

    // the same glyph from the file through FreeType, where its strikes are PNG: the same box within a pixel
    const std::optional<text::SystemFont> file = text::FindSystemFont("Apple Color Emoji");
    if (!file || file->path.starts_with(text::kCoreTextFontScheme))
        return;
    TextureRegistry textures;
    std::unique_ptr<text::TextSystem> ts = text::CreateFreeTypeTextSystem(textures);
    const text::FontId id = text::AddSystemFont(*ts, *file);
    ESIA_CHECK(id != 0);
    ts->NewFrame({});
    DrawList dl = NewList();
    ts->Draw(dl, {id, 64.0f}, Vec2(0, 0), Color::White(), "\xF0\x9F\x9A\x80");
    const std::vector<Quad> q = Quads(dl);
    ESIA_CHECK(q.size() == 1);
    if (q.size() == 1)
        ESIA_CHECK(std::abs(q[0].r.Width() - (float)b.width) <= 2.0f && std::abs(q[0].r.Height() - (float)b.height) <= 2.0f);
}
