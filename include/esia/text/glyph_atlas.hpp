// Esia - glyph atlas: rasterized glyphs packed into pages the TextureRegistry owns (Alpha8 for grayscale coverage,
// RGBA8 + TextureFlags_LcdCoverage for sub-pixel coverage), cached under a key the text system chooses.
//
// Shelf packing into square pages. When `maxPages` pages of a kind are full, the frame still gets overflow pages -
// a glyph drawn earlier in the frame is never overwritten while the frame is being built - and the next BeginFrame
// starts over: the pages beyond `maxPages` go away, the others are rewound and the cache is cleared, so glyphs are
// rasterized again as they are drawn. A long session with ever new sizes stays within bounded memory.
//
// Threading: owned by one text system, on its (UI) thread.
#pragma once
#include "esia/core/texture.hpp"
#include "esia/text/glyph_raster.hpp"
#include <unordered_map>

namespace esia::text
{
    struct GlyphSlot
    {
        TextureId page = 0;   // 0 = no ink, nothing to draw
        Vec2 uv0, uv1;
        int left = 0, top = 0, width = 0, height = 0;   // the bitmap, in pixels from the integer pen position
        Rect ink;             // outline bounds in pixels from the pen position (optical centering of icons)
    };

    struct GlyphAtlasDesc
    {
        int pageSize = 2048;
        int maxPages = 4;   // per kind (grayscale, sub-pixel) before the atlas starts over
        int padding = 1;    // empty texels right of and below every glyph
    };

    class ESIA_API GlyphAtlas
    {
    public:
        explicit GlyphAtlas(TextureRegistry& textures, const GlyphAtlasDesc& desc = {});
        ~GlyphAtlas();   // destroys its pages
        GlyphAtlas(const GlyphAtlas&) = delete;
        GlyphAtlas& operator=(const GlyphAtlas&) = delete;

        // Once per frame, before drawing: starts over if the last frame needed overflow pages.
        void BeginFrame();

        const GlyphSlot* Find(std::uint64_t key) const;
        // Packs `bitmap` into a page of its kind and caches it under `key`. Null when it cannot fit into an empty
        // page (a glyph larger than the page).
        const GlyphSlot* Add(std::uint64_t key, const GlyphBitmap& bitmap, const Rect& ink);

        std::size_t GlyphCount() const { return glyphs_.size(); }
        int PageCount(bool lcd) const { return (int)pages_[lcd ? 1 : 0].size(); }
        int Resets() const { return resets_; }   // how many times the atlas started over

    private:
        struct Page
        {
            TextureId texture = 0;
            int shelfX = 0, shelfY = 0, shelfH = 0;
        };

        bool Pack(int kind, int w, int h, int& page, int& x, int& y);

        TextureRegistry& textures_;
        GlyphAtlasDesc desc_;
        std::vector<Page> pages_[2];   // [0] Alpha8, [1] sub-pixel RGBA8
        std::unordered_map<std::uint64_t, GlyphSlot> glyphs_;
        std::vector<std::uint8_t> cell_;   // upload scratch: the glyph with its padding cleared
        bool overflow_ = false;
        int resets_ = 0;
    };
}
