// The glyph atlas: Alpha8 pages in the TextureRegistry, shelf packing with cleared padding, overflow pages that keep a
// frame's glyphs intact, and the restart at the next frame; sub-pixel pages and a change of kind.
#include "esia/text/glyph_atlas.hpp"
#include "esia_test.hpp"

using namespace esia;
using namespace esia::text;

namespace
{
    GlyphBitmap Solid(int w, int h, std::uint8_t value)
    {
        GlyphBitmap b;
        b.left = -1;
        b.top = -h;
        b.width = w;
        b.height = h;
        b.pixels.assign((std::size_t)w * h, value);
        return b;
    }
}

ESIA_TEST(Atlas, PacksGlyphsIntoRegistryPagesWithClearedPadding)
{
    TextureRegistry reg;
    GlyphAtlas atlas(reg, GlyphAtlasDesc{64, 2, 1});
    const GlyphSlot* a = atlas.Add(1, Solid(10, 12, 200), Rect(0.5f, -11.0f, 9.0f, 0.0f));
    ESIA_CHECK(a && a->page != 0);
    TextureInfo info;
    ESIA_CHECK(reg.Info(a->page, info) && info.format == TextureFormat::Alpha8 && info.width == 64 && info.height == 64 && info.Coverage());
    ESIA_CHECK(a->left == -1 && a->top == -12 && a->width == 10 && a->height == 12);
    ESIA_CHECK(a->uv0 == Vec2(0, 0) && a->uv1 == Vec2(10.0f / 64.0f, 12.0f / 64.0f));
    ESIA_CHECK(a->ink == Rect(0.5f, -11.0f, 9.0f, 0.0f));

    // the next glyph goes right of the first one's padding, on the same page
    const GlyphSlot* b = atlas.Add(2, Solid(5, 5, 90), Rect());
    ESIA_CHECK(b && b->page == a->page && b->uv0 == Vec2(11.0f / 64.0f, 0.0f));
    ESIA_CHECK(atlas.Find(1) == a && atlas.Find(2) == b && atlas.Find(3) == nullptr);
    ESIA_CHECK(atlas.GlyphCount() == 2 && atlas.PageCount() == 1);

    // the registry got the page and one upload per glyph: the glyph plus its padding, cleared
    std::vector<TextureChange> changes;
    reg.TakeChanges(changes);
    ESIA_CHECK(changes.size() == 3);
    ESIA_CHECK(changes[0].kind == TextureChange::Kind::Create && changes[0].id == a->page);
    const TextureChange& up = changes[1];
    ESIA_CHECK(up.kind == TextureChange::Kind::Update && up.x == 0 && up.y == 0 && up.width == 11 && up.height == 13);
    ESIA_CHECK(up.pixels.size() == 11 * 13);
    ESIA_CHECK(up.pixels[0] == 200 && up.pixels[9] == 200 && up.pixels[10] == 0 && up.pixels[12 * 11] == 0 && up.pixels[11 * 11 + 9] == 200);
    ESIA_CHECK(changes[2].x == 11 && changes[2].width == 6 && changes[2].height == 6);
}

ESIA_TEST(Atlas, ColorPagesHoldRgbaGlyphs)
{
    TextureRegistry reg;
    GlyphAtlas atlas(reg, GlyphAtlasDesc{64, 2, 1, TextureFormat::RGBA8});
    GlyphBitmap rgba;
    rgba.width = 2;
    rgba.height = 2;
    rgba.channels = 4;
    rgba.pixels = {255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 255, 255, 255, 128};
    const GlyphSlot* s = atlas.Add(1, rgba, Rect(0, -2, 2, 0));
    TextureInfo info;
    ESIA_CHECK(s && s->color && reg.Info(s->page, info) && info.format == TextureFormat::RGBA8 && !info.Coverage());
    std::vector<TextureChange> changes;
    reg.TakeChanges(changes);
    ESIA_CHECK(changes.size() == 2 && changes[1].width == 3 && changes[1].height == 3 && changes[1].pixels.size() == 3 * 3 * 4);
    const std::vector<std::uint8_t>& px = changes[1].pixels;   // rows of 3 texels: the glyph's 2, then padding
    ESIA_CHECK(px[4] == 0 && px[5] == 255 && px[8] == 0 && px[11] == 0);           // row 0: green, padding cleared
    ESIA_CHECK(px[12] == 0 && px[14] == 255 && px[15] == 255 && px[16 + 3] == 128);   // row 1: blue, half-transparent white
    // a coverage bitmap does not go into a color atlas (nor the other way round)
    ESIA_CHECK(atlas.Add(2, Solid(2, 2, 200), Rect()) == nullptr);
    GlyphAtlas gray(reg, GlyphAtlasDesc{64, 2, 1});
    ESIA_CHECK(gray.Add(1, rgba, Rect()) == nullptr && !gray.Add(2, Solid(2, 2, 200), Rect())->color);
}

ESIA_TEST(Atlas, GlyphsWithoutInkAndGlyphsLargerThanAPage)
{
    TextureRegistry reg;
    GlyphAtlas atlas(reg, GlyphAtlasDesc{64, 2, 1});
    const GlyphSlot* ink = atlas.Add(1, Solid(4, 4, 255), Rect());
    ESIA_CHECK(ink && ink->page != 0);

    // a glyph without ink takes no space and has no page
    const GlyphSlot* space = atlas.Add(2, GlyphBitmap{}, Rect());
    ESIA_CHECK(space && space->page == 0 && space->width == 0);
    ESIA_CHECK(atlas.PageCount() == 1 && atlas.GlyphCount() == 2);

    // larger than a page: refused
    ESIA_CHECK(atlas.Add(3, Solid(64, 8, 255), Rect()) == nullptr);
}

ESIA_TEST(Atlas, OverflowKeepsTheFrameThenStartsOver)
{
    TextureRegistry reg;
    GlyphAtlas atlas(reg, GlyphAtlasDesc{32, 1, 1});
    // 15 x 15 glyphs + padding: four per 32 x 32 page
    const GlyphSlot* slots[5] = {};
    for (int i = 0; i < 5; ++i)
        slots[i] = atlas.Add((std::uint64_t)i + 1, Solid(15, 15, (std::uint8_t)(50 + i)), Rect());
    for (int i = 0; i < 5; ++i)
        ESIA_CHECK(slots[i] && atlas.Find((std::uint64_t)i + 1) == slots[i]);
    // the fifth needed a page beyond maxPages: nothing drawn this frame was overwritten
    ESIA_CHECK(atlas.PageCount() == 2 && reg.Count() == 2);
    ESIA_CHECK(slots[4]->page != slots[0]->page && slots[3]->uv0 == Vec2(16.0f / 32.0f, 16.0f / 32.0f));

    // the next frame starts over: overflow page gone, the first rewound, the cache empty
    atlas.BeginFrame();
    ESIA_CHECK(atlas.Resets() == 1 && atlas.GlyphCount() == 0 && atlas.PageCount() == 1 && reg.Count() == 1);
    const GlyphSlot* again = atlas.Add(9, Solid(15, 15, 7), Rect());
    ESIA_CHECK(again && again->uv0 == Vec2(0, 0));

    // no overflow, no restart
    atlas.BeginFrame();
    ESIA_CHECK(atlas.Resets() == 1 && atlas.Find(9) != nullptr);
}

ESIA_TEST(Atlas, DestroysItsPages)
{
    TextureRegistry reg;
    {
        GlyphAtlas atlas(reg, GlyphAtlasDesc{32, 4, 1});
        atlas.Add(1, Solid(20, 20, 1), Rect());
        atlas.Add(2, Solid(20, 20, 1), Rect());   // 21 x 21 with its padding: a second page
        ESIA_CHECK(atlas.PageCount() == 2 && reg.Count() == 2);
    }
    ESIA_CHECK(reg.Count() == 0);
}

// Sub-pixel pages (RGBA8, TextureFlags_LcdCoverage) take sub-pixel bitmaps as they are and 8-bit coverage as gray
// (R = G = B = A); their glyphs take the text's color. Other kinds are refused both ways.
ESIA_TEST(Atlas, LcdPagesTakeSubpixelAndGrayCoverage)
{
    TextureRegistry reg;
    GlyphAtlas atlas(reg, GlyphAtlasDesc{64, 2, 1, TextureFormat::RGBA8, TextureFlags_LcdCoverage});
    ESIA_CHECK(atlas.Lcd());
    GlyphBitmap lcd = Solid(2, 2, 0);
    lcd.channels = 4;
    lcd.lcd = true;
    lcd.pixels = {10, 20, 30, 20, 11, 21, 31, 21, 12, 22, 32, 22, 13, 23, 33, 23};
    const GlyphSlot* a = atlas.Add(1, lcd, Rect());
    ESIA_CHECK(a && !a->color);
    TextureInfo info;
    ESIA_CHECK(reg.Info(a->page, info) && info.Lcd() && info.Coverage() && info.format == TextureFormat::RGBA8 && info.width == 64);
    const GlyphSlot* g = atlas.Add(2, Solid(2, 1, 77), Rect());
    ESIA_CHECK(g && g->page == a->page && !g->color);
    std::vector<TextureChange> changes;
    reg.TakeChanges(changes);
    ESIA_CHECK(changes.size() == 3 && changes[0].kind == TextureChange::Kind::Create && changes[0].info.Lcd());
    // the first glyph's cell (3 x 3 with its padding), then the gray one's, four channels each
    ESIA_CHECK(changes[1].width == 3 && changes[1].height == 3 && changes[1].pixels.size() == 3 * 3 * 4);
    ESIA_CHECK(changes[1].pixels[0] == 10 && changes[1].pixels[3] == 20 && changes[1].pixels[4] == 11 && changes[1].pixels[8] == 0);
    ESIA_CHECK(changes[2].width == 3 && changes[2].height == 2 && changes[2].pixels.size() == 3 * 2 * 4);
    for (int c = 0; c < 4; ++c)
        ESIA_CHECK(changes[2].pixels[(std::size_t)c] == 77 && changes[2].pixels[4 + (std::size_t)c] == 77 && changes[2].pixels[8 + (std::size_t)c] == 0);
    // a color glyph does not go into sub-pixel pages, nor a sub-pixel glyph into grayscale or color pages
    GlyphBitmap color = lcd;
    color.lcd = false;
    ESIA_CHECK(atlas.Add(3, color, Rect()) == nullptr);
    GlyphAtlas grayAtlas(reg, GlyphAtlasDesc{64, 2, 1});
    GlyphAtlas colorAtlas(reg, GlyphAtlasDesc{64, 2, 1, TextureFormat::RGBA8});
    ESIA_CHECK(grayAtlas.Add(1, lcd, Rect()) == nullptr && colorAtlas.Add(1, lcd, Rect()) == nullptr);
    ESIA_CHECK(!grayAtlas.Lcd() && !colorAtlas.Lcd());
}

// Reformat: pages of another kind from now on - the old pages and every glyph go at once (a restart).
ESIA_TEST(Atlas, ReformatDropsThePagesForTheOtherKind)
{
    TextureRegistry reg;
    GlyphAtlas atlas(reg, GlyphAtlasDesc{64, 2, 1});
    const GlyphSlot* a = atlas.Add(1, Solid(4, 4, 255), Rect());
    ESIA_CHECK(a != nullptr);
    const TextureId page = a ? a->page : 0;
    const int resets = atlas.Resets();
    atlas.Reformat(TextureFormat::Alpha8, TextureFlags_None);   // the same kind: nothing changes
    ESIA_CHECK(atlas.Find(1) != nullptr && atlas.Resets() == resets);
    atlas.Reformat(TextureFormat::RGBA8, TextureFlags_LcdCoverage);
    TextureInfo info;
    ESIA_CHECK(!reg.Info(page, info) && atlas.Find(1) == nullptr && atlas.PageCount() == 0 && atlas.GlyphCount() == 0);
    ESIA_CHECK(atlas.Resets() == resets + 1 && atlas.Lcd());
    const GlyphSlot* b = atlas.Add(1, Solid(4, 4, 255), Rect());
    ESIA_CHECK(b && reg.Info(b->page, info) && info.Lcd());
    atlas.Reformat(TextureFormat::Alpha8, TextureFlags_None);
    const GlyphSlot* c = atlas.Add(1, Solid(4, 4, 255), Rect());
    ESIA_CHECK(c && reg.Info(c->page, info) && info.format == TextureFormat::Alpha8 && !atlas.Lcd());
}
