// Esia - glyph atlas (include/esia/text/glyph_atlas.hpp).
#include "esia/text/glyph_atlas.hpp"
#include <algorithm>
#include <cstring>

namespace esia::text
{
    GlyphAtlas::GlyphAtlas(TextureRegistry& textures, const GlyphAtlasDesc& desc) : textures_(textures), desc_(desc) {}

    GlyphAtlas::~GlyphAtlas()
    {
        for (const std::vector<Page>& kind : pages_)
            for (const Page& p : kind)
                textures_.Destroy(p.texture);
    }

    void GlyphAtlas::BeginFrame()
    {
        if (!overflow_)
            return;
        overflow_ = false;
        ++resets_;
        glyphs_.clear();
        for (std::vector<Page>& kind : pages_)
        {
            // the pages beyond maxPages were the last frame's overflow; the others are refilled from the top
            for (std::size_t i = (std::size_t)desc_.maxPages; i < kind.size(); ++i)
                textures_.Destroy(kind[i].texture);
            if (kind.size() > (std::size_t)desc_.maxPages)
                kind.resize((std::size_t)desc_.maxPages);
            for (Page& p : kind)
                p.shelfX = p.shelfY = p.shelfH = 0;
        }
    }

    const GlyphSlot* GlyphAtlas::Find(std::uint64_t key) const
    {
        const auto it = glyphs_.find(key);
        return it != glyphs_.end() ? &it->second : nullptr;
    }

    bool GlyphAtlas::Pack(int kind, int w, int h, int& page, int& x, int& y)
    {
        if (w > desc_.pageSize || h > desc_.pageSize)
            return false;
        std::vector<Page>& pages = pages_[kind];
        for (int i = 0; i < (int)pages.size(); ++i)
        {
            Page& p = pages[i];
            if (p.shelfX + w > desc_.pageSize)
            {
                p.shelfY += p.shelfH;   // the shelf is full: open the next one below it
                p.shelfX = 0;
                p.shelfH = 0;
            }
            if (p.shelfY + h <= desc_.pageSize)
            {
                page = i;
                x = p.shelfX;
                y = p.shelfY;
                p.shelfX += w;
                p.shelfH = std::max(p.shelfH, h);
                return true;
            }
        }
        TextureInfo info;
        info.format = kind == 1 ? TextureFormat::RGBA8 : TextureFormat::Alpha8;
        info.width = info.height = desc_.pageSize;
        info.flags = kind == 1 ? TextureFlags_LcdCoverage : TextureFlags_None;
        Page p;
        p.texture = textures_.Create(info);
        if (!p.texture)
            return false;
        if ((int)pages.size() >= desc_.maxPages)
            overflow_ = true;   // keep this frame's glyphs intact; start over at the next BeginFrame
        p.shelfX = w;
        p.shelfH = h;
        pages.push_back(p);
        page = (int)pages.size() - 1;
        x = y = 0;
        return true;
    }

    const GlyphSlot* GlyphAtlas::Add(std::uint64_t key, const GlyphBitmap& bitmap, const Rect& ink)
    {
        GlyphSlot slot;
        slot.left = bitmap.left;
        slot.top = bitmap.top;
        slot.width = bitmap.width;
        slot.height = bitmap.height;
        slot.ink = ink;
        if (bitmap.width > 0 && bitmap.height > 0)
        {
            const int kind = bitmap.lcd ? 1 : 0;
            const int cw = bitmap.width + desc_.padding, ch = bitmap.height + desc_.padding;
            int page = 0, x = 0, y = 0;
            if (!Pack(kind, cw, ch, page, x, y))
                return nullptr;
            // the whole cell goes up, padding cleared: after a restart the texels next to a glyph may hold an old one
            const std::size_t bpp = bitmap.lcd ? 4 : 1;
            cell_.assign((std::size_t)cw * ch * bpp, 0);
            for (int row = 0; row < bitmap.height; ++row)
                std::memcpy(cell_.data() + (std::size_t)row * cw * bpp, bitmap.pixels.data() + (std::size_t)row * bitmap.width * bpp,
                            (std::size_t)bitmap.width * bpp);
            slot.page = pages_[kind][page].texture;
            textures_.Update(slot.page, x, y, cw, ch, cell_.data());
            const float inv = 1.0f / (float)desc_.pageSize;
            slot.uv0 = Vec2((float)x * inv, (float)y * inv);
            slot.uv1 = Vec2((float)(x + bitmap.width) * inv, (float)(y + bitmap.height) * inv);
        }
        return &glyphs_.insert_or_assign(key, slot).first->second;
    }
}
