// Color glyphs of the FreeType text system: the PNG reader for color bitmap fonts FreeType cannot decode itself
// (src/esia/text/ft/png.hpp), and COLR fonts' vector emoji painted in color (colr.hpp) where the system has one.
#include "png.hpp"
#include "esia/text/freetype.hpp"
#include "esia/text/system_fonts.hpp"
#include "font_test_util.hpp"
#include "ft_test_util.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <set>
#include <tuple>

using namespace esia;
using namespace esia::texttest;

namespace
{
    // A PNG file of the given header and rows (each row: its filter byte, then the filtered bytes); chunk CRCs are
    // left 0 - the reader does not check them.
    struct PngBuilder
    {
        std::vector<std::uint8_t> out = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};

        void Chunk(const char* type, const std::vector<std::uint8_t>& body)
        {
            const std::uint32_t n = (std::uint32_t)body.size();
            out.insert(out.end(), {(std::uint8_t)(n >> 24), (std::uint8_t)(n >> 16), (std::uint8_t)(n >> 8), (std::uint8_t)n});
            out.insert(out.end(), type, type + 4);
            out.insert(out.end(), body.begin(), body.end());
            out.insert(out.end(), {0, 0, 0, 0});
        }

        void Header(int w, int h, int depth, int colorType, int interlace = 0)
        {
            Chunk("IHDR", {0, 0, 0, (std::uint8_t)w, 0, 0, 0, (std::uint8_t)h, (std::uint8_t)depth, (std::uint8_t)colorType, 0, 0, (std::uint8_t)interlace});
        }

        void Data(const std::vector<std::uint8_t>& rows)
        {
            Chunk("IDAT", testkit::Deflate(rows.data(), rows.size()));
            Chunk("IEND", {});
        }

        bool Decode(int& w, int& h, std::vector<std::uint8_t>& rgba) const { return text::detail::DecodePng(out.data(), out.size(), w, h, rgba); }
    };

    std::tuple<int, int, int, int> Px(const std::vector<std::uint8_t>& rgba, int w, int x, int y)
    {
        const std::uint8_t* p = rgba.data() + ((std::size_t)y * w + x) * 4;
        return {p[0], p[1], p[2], p[3]};
    }
}

// Palette images at 4 bits per pixel (the depth pngquant gives emoji with few colors) with a transparent entry, the
// second row through the Up filter.
ESIA_TEST(ColorGlyphPng, PaletteFourBitsWithTransparency)
{
    PngBuilder b;
    b.Header(3, 2, 4, 3);
    b.Chunk("PLTE", {255, 0, 0, 0, 255, 0, 0, 0, 255});
    b.Chunk("tRNS", {255, 128, 0});   // blue: transparent
    // row 0: indices 0 1 2 (0x01 0x20); row 1: 2 1 0 (0x21 0x00), Up-filtered against row 0
    b.Data({0, 0x01, 0x20, 2, (std::uint8_t)(0x21 - 0x01), (std::uint8_t)(0x00 - 0x20)});
    int w = 0, h = 0;
    std::vector<std::uint8_t> rgba;
    ESIA_CHECK(b.Decode(w, h, rgba));
    ESIA_CHECK(w == 3 && h == 2 && rgba.size() == 24);
    if (rgba.size() != 24)
        return;
    ESIA_CHECK(Px(rgba, w, 0, 0) == std::make_tuple(255, 0, 0, 255));
    ESIA_CHECK(Px(rgba, w, 1, 0) == std::make_tuple(0, 255, 0, 128));
    ESIA_CHECK(Px(rgba, w, 2, 0) == std::make_tuple(0, 0, 255, 0));
    ESIA_CHECK(Px(rgba, w, 0, 1) == std::make_tuple(0, 0, 255, 0));
    ESIA_CHECK(Px(rgba, w, 2, 1) == std::make_tuple(255, 0, 0, 255));
}

// RGB through the Sub and Paeth filters, gray + alpha, and 1-bit gray widened to 0 / 255.
ESIA_TEST(ColorGlyphPng, TrueColorGrayAndFilters)
{
    {
        PngBuilder b;
        b.Header(2, 2, 8, 2);
        // row 0 (Sub): (10,20,30) (15,25,35); row 1 (Paeth, a = left, b = up, c = up-left): (10,20,30) (20,40,60)
        b.Data({1, 10, 20, 30, 5, 5, 5, 4, 0, 0, 0, 5, 15, 25});
        int w = 0, h = 0;
        std::vector<std::uint8_t> rgba;
        ESIA_CHECK(b.Decode(w, h, rgba) && w == 2 && h == 2);
        if (rgba.size() == 16)
        {
            ESIA_CHECK(Px(rgba, w, 1, 0) == std::make_tuple(15, 25, 35, 255));
            ESIA_CHECK(Px(rgba, w, 0, 1) == std::make_tuple(10, 20, 30, 255));
            ESIA_CHECK(Px(rgba, w, 1, 1) == std::make_tuple(20, 40, 60, 255));
        }
    }
    {
        PngBuilder b;
        b.Header(1, 1, 8, 4);
        b.Data({0, 200, 50});
        int w = 0, h = 0;
        std::vector<std::uint8_t> rgba;
        ESIA_CHECK(b.Decode(w, h, rgba) && rgba.size() == 4 && Px(rgba, w, 0, 0) == std::make_tuple(200, 200, 200, 50));
    }
    {
        PngBuilder b;
        b.Header(10, 1, 1, 0);
        b.Data({0, 0b10100000, 0b01000000});   // pixels 0 and 2 white, then 9
        int w = 0, h = 0;
        std::vector<std::uint8_t> rgba;
        ESIA_CHECK(b.Decode(w, h, rgba) && rgba.size() == 40);
        if (rgba.size() == 40)
        {
            ESIA_CHECK(Px(rgba, w, 0, 0) == std::make_tuple(255, 255, 255, 255));
            ESIA_CHECK(Px(rgba, w, 1, 0) == std::make_tuple(0, 0, 0, 255));
            ESIA_CHECK(Px(rgba, w, 9, 0) == std::make_tuple(255, 255, 255, 255));
        }
    }
}

// What it cannot read fails cleanly: interlaced images, truncated data, a palette image without its palette, junk.
ESIA_TEST(ColorGlyphPng, RejectsWhatItCannotRead)
{
    int w = 0, h = 0;
    std::vector<std::uint8_t> rgba;
    {
        PngBuilder b;
        b.Header(1, 1, 8, 6, 1);
        b.Data({0, 1, 2, 3, 4});
        ESIA_CHECK(!b.Decode(w, h, rgba));
    }
    {
        PngBuilder b;
        b.Header(2, 2, 8, 6);
        b.Data({0, 1, 2, 3, 4});   // a row and a half short
        ESIA_CHECK(!b.Decode(w, h, rgba));
    }
    {
        PngBuilder b;
        b.Header(1, 1, 8, 3);
        b.Data({0, 0});
        ESIA_CHECK(!b.Decode(w, h, rgba));
    }
    {
        PngBuilder b;
        b.Header(1, 1, 8, 6);
        b.Data({0, 1, 2, 3, 4});
        std::vector<std::uint8_t> cut(b.out.begin(), b.out.end() - 20);   // IDAT cut off
        ESIA_CHECK(!text::detail::DecodePng(cut.data(), cut.size(), w, h, rgba));
    }
    const char junk[32] = "\x89PNG but not one";
    ESIA_CHECK(!text::detail::DecodePng(reinterpret_cast<const std::uint8_t*>(junk), sizeof(junk), w, h, rgba));
    ESIA_CHECK(!text::detail::DecodePng(nullptr, 0, w, h, rgba));
}

// COLR fonts (Windows' Segoe UI Emoji, Android's Noto Color Emoji: version 1, gradients and all) draw their emoji in
// color, from an RGBA8 page, white with the text's alpha, about an em wide; their other glyphs stay outlines. Skipped
// where the system has no such font.
ESIA_TEST(FreeType, ColorEmojiFromColrLayers)
{
    std::optional<text::SystemFont> emoji = text::FindSystemFont("Segoe UI Emoji");
    if (!emoji)
    {
        std::printf("  skipped: no Segoe UI Emoji\n");
        return;
    }
    TextureRegistry textures;
    std::unique_ptr<text::TextSystem> ts = text::CreateFreeTypeTextSystem(textures);
    const text::FontId droid = ts->AddFontFile((kFonts + "/DroidSans.ttf").c_str());
    const text::FontId id = text::AddSystemFont(*ts, *emoji);
    ESIA_CHECK(droid != 0 && id != 0);
    ts->AddFallback(id);
    ts->NewFrame({});
    DrawList dl = NewList();
    ts->Draw(dl, {droid, 48.0f}, Vec2(10, 10), Color(0.2f, 0.4f, 0.6f, 0.5f), "A\xF0\x9F\x9A\x80");   // A + rocket
    const std::vector<Quad> q = Quads(dl);
    ESIA_CHECK(q.size() == 2);
    if (q.size() != 2)
        return;
    PageStore store;
    store.Collect(textures);
    TextureInfo letter, rocket;
    ESIA_CHECK(textures.Info(q[0].texture, letter) && letter.Coverage());
    ESIA_CHECK(textures.Info(q[1].texture, rocket) && rocket.format == TextureFormat::RGBA8);
    ESIA_CHECK(q[1].color == Color::White(0.5f).ToRgba8());
    ESIA_CHECK(q[1].r.Width() > 30.0f && q[1].r.Width() < 66.0f && q[1].r.min.y < q[0].r.max.y && q[1].r.max.y > q[0].r.min.y);

    // the rocket's texels: opaque and see-through ones, many colors, not gray
    const auto pageIt = store.pages.find(q[1].texture);
    ESIA_CHECK(pageIt != store.pages.end());
    if (pageIt == store.pages.end())
        return;
    const PageStore::Page& page = pageIt->second;
    const int tx = (int)std::lround(q[1].uv0.x * (float)page.info.width), ty = (int)std::lround(q[1].uv0.y * (float)page.info.height);
    const int w = (int)std::lround(q[1].r.Width()), h = (int)std::lround(q[1].r.Height());
    std::set<std::uint32_t> colors;
    int opaque = 0, clear = 0, tinted = 0;
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            const std::uint8_t* p = page.pixels.data() + ((std::size_t)(ty + y) * page.info.width + (tx + x)) * 4;
            opaque += p[3] == 255;
            clear += p[3] == 0;
            if (p[3] == 255)
            {
                colors.insert((std::uint32_t)(p[0] >> 3) << 10 | (std::uint32_t)(p[1] >> 3) << 5 | (std::uint32_t)(p[2] >> 3));
                tinted += std::max({p[0], p[1], p[2]}) - std::min({p[0], p[1], p[2]}) > 40;
            }
        }
    std::printf("  rocket %d x %d px: %d opaque, %d clear, %d colors, %d tinted\n", w, h, opaque, clear, (int)colors.size(), tinted);
    ESIA_CHECK(opaque > w * h / 8 && clear > w * h / 8);
    ESIA_CHECK(colors.size() > 20 && tinted > opaque / 8);

    // a glyph of the font without colors (its space) is an advance like any other, nothing drawn
    DrawList space = NewList();
    ts->Draw(space, {id, 48.0f}, Vec2(10, 10), Color::Black(), " ");
    ESIA_CHECK(Quads(space).empty());
}
