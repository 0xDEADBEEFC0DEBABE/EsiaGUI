// Esia - SVG textures (svg.hpp): rasterized documents packed into RGBA8 pages by text::GlyphAtlas - those drawn frame
// after frame apart from the others - or in textures of their own when larger than a page; and Draw - a document
// rasterized at the size and place it is drawn, its texels on the target's pixels.
#include "esia/core/texture.hpp"
#include "esia/render/painter.hpp"
#include "esia/text/glyph_atlas.hpp"
#include "svg_internal.hpp"
#include <cmath>
#include <cstring>

namespace esia::svg
{
    namespace
    {
        // one texel of transparent border around every drawing: sampled between texels, its edges keep their color
        constexpr int kBorder = 1;

        std::uint64_t Mix(std::uint64_t h, std::uint64_t v)
        {
            h ^= v + 0x9E3779B97F4A7C15ull + (h << 6) + (h >> 2);
            return h * 0x100000001B3ull;
        }

        std::uint64_t Bits(float a, float b)
        {
            std::uint32_t x = 0, y = 0;
            std::memcpy(&x, &a, sizeof(x));
            std::memcpy(&y, &b, sizeof(y));
            return ((std::uint64_t)x << 32) | y;
        }

        // `img` (width x height inside its border) into a page under `key`; the pixels stay with `img`
        const text::GlyphSlot* Pack(text::GlyphAtlas& pages, std::uint64_t key, Image& img, int width, int height)
        {
            text::GlyphBitmap bitmap;
            bitmap.width = img.width;
            bitmap.height = img.height;
            bitmap.channels = 4;
            bitmap.pixels = std::move(img.pixels);
            const text::GlyphSlot* slot = pages.Add(key, bitmap, Rect(0.0f, 0.0f, (float)width, (float)height));
            img.pixels = std::move(bitmap.pixels);
            return slot;
        }
    }

    Atlas::Atlas(TextureRegistry& textures, int pageSize, int maxPages) : textures_(textures), pageSize_(pageSize)
    {
        text::GlyphAtlasDesc desc;
        desc.pageSize = pageSize;
        desc.maxPages = maxPages;
        desc.format = TextureFormat::RGBA8;
        lasting_ = std::make_unique<text::GlyphAtlas>(textures, desc);
        recent_ = std::make_unique<text::GlyphAtlas>(textures, desc);
    }

    Atlas::~Atlas()
    {
        for (const auto& o : own_)
            textures_.Destroy(o.second.texture);
    }

    void Atlas::Recycle(Image& img)
    {
        // a few buffers of icon sizes (up to 256 x 256 pixels), not the memory of every drawing ever made
        if (spare_.size() < 32 && img.pixels.capacity() <= 256 * 256 * 4)
            spare_.push_back(std::move(img.pixels));
    }

    void Atlas::BeginFrame()
    {
        ++frame_;
        lasting_->BeginFrame();
        recent_->BeginFrame();
        // the pixels of the last frame's new drawings stay at hand one frame more: drawn again, they go into the lasting
        // pages from them. The buffers of those from the frame before are rasterized into again.
        std::swap(made_, madeBefore_);
        for (auto& m : made_)
            Recycle(m.second);
        made_.clear();
        // textures of their own that the last frame did not draw
        for (auto it = own_.begin(); it != own_.end();)
            if (it->second.frame + 1 < frame_)
            {
                textures_.Destroy(it->second.texture);
                it = own_.erase(it);
            }
            else
                ++it;
    }

    const Atlas::Entry* Atlas::Get(const Document& doc, int width, int height, const RasterOptions& options)
    {
        if (doc.Empty() || width <= 0 || height <= 0)
            return nullptr;
        std::uint64_t key = Mix(doc.Id(), ((std::uint64_t)(std::uint32_t)width << 32) | (std::uint32_t)height);
        key = Mix(key, options.mask ? 1u : 0u);
        // currentColor only where it paints something (and only its alpha for a mask)
        if (doc.UsesCurrentColor())
        {
            const Color c = options.currentColor;
            key = Mix(key, options.mask ? Color(1.0f, 1.0f, 1.0f, c.a).ToRgba8() : c.ToRgba8());
        }
        const Rect& vp = options.viewport;
        if (!vp.Empty())
            key = Mix(Mix(key, Bits(vp.min.x, vp.min.y)), Bits(vp.max.x, vp.max.y));
        // drawn frame after frame: the usual case
        if (const text::GlyphSlot* slot = lasting_->Find(key))
            return Placed(*slot);
        return Make(doc, width, height, options, key);
    }

    const Atlas::Entry* Atlas::Placed(const text::GlyphSlot& slot)
    {
        if (slot.page == 0)
            return nullptr;
        // the drawing inside its border
        const float texel = 1.0f / (float)pageSize_;
        entry_.texture = slot.page;
        entry_.uv0 = slot.uv0 + Vec2(texel, texel) * (float)kBorder;
        entry_.uv1 = slot.uv1 - Vec2(texel, texel) * (float)kBorder;
        return &entry_;
    }

    const Atlas::Entry* Atlas::Make(const Document& doc, int width, int height, const RasterOptions& options, std::uint64_t key)
    {
        const auto rasterize = [&](Image& img) {
            RasterOptions o = options;
            o.border = kBorder;
            if (!Rasterize(doc, width, height, o, img))
                return false;
            ++rasterized_;
            return true;
        };

        if (width + 2 * kBorder + 1 > pageSize_ || height + 2 * kBorder + 1 > pageSize_)
        {
            // larger than a page: a texture of its own
            auto it = own_.find(key);
            if (it == own_.end())
            {
                Image img;
                if (!rasterize(img))
                    return nullptr;
                TextureInfo info;
                info.format = TextureFormat::RGBA8;
                info.width = img.width;
                info.height = img.height;
                const TextureId texture = textures_.Create(info, img.pixels.data());
                if (texture == 0)
                    return nullptr;
                it = own_.emplace(key, Own{texture, img.width, img.height, frame_}).first;
            }
            it->second.frame = frame_;
            entry_.texture = it->second.texture;
            entry_.uv0 = Vec2((float)kBorder / (float)it->second.width, (float)kBorder / (float)it->second.height);
            entry_.uv1 = Vec2(1.0f, 1.0f) - entry_.uv0;
            return &entry_;
        }

        const text::GlyphSlot* slot = nullptr;
        const auto before = madeBefore_.find(key);
        if (before != madeBefore_.end())
        {
            // rasterized the last frame and drawn again: into the lasting pages, from the pixels it had then
            slot = Pack(*lasting_, key, before->second, width, height);
            Recycle(before->second);
            madeBefore_.erase(before);
        }
        else if (!(slot = recent_->Find(key)))
        {
            Image img;
            if (!spare_.empty())
            {
                img.pixels = std::move(spare_.back());
                spare_.pop_back();
            }
            if (!rasterize(img))
                return nullptr;
            slot = Pack(*recent_, key, img, width, height);
            made_[key] = std::move(img);
        }
        return slot ? Placed(*slot) : nullptr;
    }

    void Draw(Painter& painter, Atlas& atlas, const Document& doc, const Rect& r, Color color, bool mask)
    {
        if (doc.Empty() || r.Empty() || !(color.a > 0.0f))
            return;
        const float k = painter.ScaleFactor();
        if (!(k > 0.0f))
            return;
        const float px = painter.Env().pixelScale > 0.0f ? painter.Env().pixelScale : 1.0f;
        RasterOptions o;
        o.mask = mask || doc.Monochrome();
        Color tint = Color::White(color.a);
        if (o.mask)
        {
            o.currentColor = Color::White();   // one mask for every color: the color tints it
            tint = color;
        }
        else
            o.currentColor = color.WithAlpha(1.0f);

        int x0 = 0, y0 = 0, w = 0, h = 0;
        const Vec2 at = painter.Scaled(r.min);
        if (k == 1.0f && at == r.min)
        {
            // whole physical pixels: the texels land on pixels one to one
            x0 = (int)std::round(r.min.x * px);
            y0 = (int)std::round(r.min.y * px);
            w = std::max(1, (int)std::lround(r.Width() * px));
            h = std::max(1, (int)std::lround(r.Height() * px));
        }
        else
        {
            // under a scale: exactly where it puts the drawing (to 1/64 pixel, so that a scale at rest finds it in the
            // cache), rasterized into the whole pixels around that
            const auto pixel = [px](float v) { return std::round(v * px * 64.0f) / 64.0f; };
            const Vec2 to = painter.Scaled(r.max);
            const Vec2 a(pixel(at.x), pixel(at.y)), b(pixel(to.x), pixel(to.y));
            if (!(b.x > a.x && b.y > a.y))
                return;
            x0 = (int)std::floor(a.x);
            y0 = (int)std::floor(a.y);
            w = (int)std::ceil(b.x) - x0;
            h = (int)std::ceil(b.y) - y0;
            if (a.x != (float)x0 || a.y != (float)y0 || b.x != (float)(x0 + w) || b.y != (float)(y0 + h))
                o.viewport = Rect(a.x - (float)x0, a.y - (float)y0, b.x - (float)x0, b.y - (float)y0);
        }
        const Atlas::Entry* e = atlas.Get(doc, w, h, o);
        if (e)
            painter.UnscaledImage(e->texture, Rect((float)x0 / px, (float)y0 / px, (float)(x0 + w) / px, (float)(y0 + h) / px), tint,
                                  e->uv0, e->uv1);
    }
}
