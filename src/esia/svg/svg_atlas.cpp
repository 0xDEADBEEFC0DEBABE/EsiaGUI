// Esia - SVG textures (svg.hpp): rasterized documents packed into RGBA8 pages by text::GlyphAtlas - those drawn frame
// after frame apart from the others - or in textures of their own when larger than a page; masks of icons that last as
// tiles of the text's pages; and Draw - a document rasterized at the size and place it is drawn, its texels on the
// target's pixels, as far as the clip rect shows it.
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
        // a large drawing partly outside the clip rect is rasterized in steps of this many pixels from its corner
        constexpr int kPartStep = 256;

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

        // the masks Tile takes
        bool Tiled(int width, int height, const RasterOptions& options)
        {
            return options.mask && width <= Atlas::kMaxTile && height <= Atlas::kMaxTile;
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
        std::swap(found_, foundBefore_);
        found_.clear();
        // textures of their own that the last frame did not draw, the parts of drawings it did not draw
        for (auto it = own_.begin(); it != own_.end();)
            if (it->second.frame + 1 < frame_)
            {
                textures_.Destroy(it->second.texture);
                it = own_.erase(it);
            }
            else
                ++it;
        for (auto it = parts_.begin(); it != parts_.end();)
            if (it->second.frame + 1 < frame_)
                it = parts_.erase(it);
            else
                ++it;
    }

    std::uint64_t Atlas::Key(const Document& doc, int width, int height, const RasterOptions& options)
    {
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
        return key;
    }

    const Atlas::Entry* Atlas::Get(const Document& doc, int width, int height, const RasterOptions& options)
    {
        if (doc.Empty() || width <= 0 || height <= 0)
            return nullptr;
        const std::uint64_t key = Key(doc, width, height, options);
        // drawn frame after frame: the usual case
        if (const text::GlyphSlot* slot = lasting_->Find(key))
        {
            if (Tiled(width, height, options))
                found_[key] = 1;   // a mask that lasts: Tile takes it the next frame
            return Placed(*slot);
        }
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

    const Atlas::Entry* Atlas::GetPart(const Document& doc, int width, int height, const RasterOptions& options, const Rect& part)
    {
        // whole pixels inside the drawing's
        const auto inside = [](float v, int hi) { return !(v > 0.0f) ? 0 : (v < (float)hi ? (int)std::lround(v) : hi); };
        const int x0 = inside(part.min.x, width), y0 = inside(part.min.y, height), x1 = inside(part.max.x, width), y1 = inside(part.max.y, height);
        if (doc.Empty() || x1 <= x0 || y1 <= y0)
            return nullptr;
        if (x0 == 0 && y0 == 0 && x1 == width && y1 == height)
            return Get(doc, width, height, options);
        // the part's viewport: the drawing's moved by the part's corner (its pixels the whole image's there)
        PartOf of;
        of.viewport = options.viewport.Empty() ? Rect(0.0f, 0.0f, (float)width, (float)height) : options.viewport;
        of.x0 = x0;
        of.y0 = y0;
        of.whole = Key(doc, width, height, options);
        RasterOptions po = options;
        po.viewport = Rect(of.viewport.min.x - (float)x0, of.viewport.min.y - (float)y0, of.viewport.max.x - (float)x0, of.viewport.max.y - (float)y0);
        const auto kept = parts_.find(of.whole);
        if (kept != parts_.end())
            kept->second.frame = frame_;   // the pixels of its part made last stay while it is drawn
        const std::uint64_t key = Key(doc, x1 - x0, y1 - y0, po);
        if (const text::GlyphSlot* slot = lasting_->Find(key))
            return Placed(*slot);
        return Make(doc, x1 - x0, y1 - y0, po, key, &of);
    }

    bool Atlas::Pixels(const Document& doc, int width, int height, const RasterOptions& options, const PartOf* part, Image& img)
    {
        RasterOptions o = options;
        o.border = kBorder;
        if (!part)
            return Rasterize(doc, width, height, o, img);
        const auto kept = parts_.find(part->whole);
        if (kept == parts_.end())
            return detail::RasterizePart(doc, width, height, o, img);
        // A part of a drawing whose part made last is kept: the texels they share - inside that one's border, where every
        // texel is the whole image's - copied from it, the rest rasterized in up to four bands around them (each with a
        // border: its texels the whole image's up to its edges). All in the whole drawing's pixels.
        const Part& prev = kept->second;
        const int rx0 = part->x0 - kBorder, ry0 = part->y0 - kBorder, rx1 = part->x0 + width + kBorder, ry1 = part->y0 + height + kBorder;
        const int sx0 = std::max(rx0, prev.x0), sy0 = std::max(ry0, prev.y0), sx1 = std::min(rx1, prev.x1), sy1 = std::min(ry1, prev.y1);
        if (sx0 >= sx1 || sy0 >= sy1 || 4 * (std::int64_t)(sx1 - sx0) * (sy1 - sy0) < (std::int64_t)(rx1 - rx0) * (ry1 - ry0))
            return detail::RasterizePart(doc, width, height, o, img);   // little in common
        const int iw = rx1 - rx0, ih = ry1 - ry0;
        img.width = iw;
        img.height = ih;
        img.pixels.resize((std::size_t)iw * (std::size_t)ih * 4);
        const auto row = [](Image& i, int x, int y) { return i.pixels.data() + ((std::size_t)y * (std::size_t)i.width + (std::size_t)x) * 4; };
        for (int y = sy0; y < sy1; ++y)
            std::memcpy(row(img, sx0 - rx0, y - ry0), prev.img.pixels.data() + ((std::size_t)(y - prev.y0 + kBorder) * (std::size_t)prev.img.width + (std::size_t)(sx0 - prev.x0 + kBorder)) * 4,
                        (std::size_t)(sx1 - sx0) * 4);
        const int bands[4][4] = {{rx0, ry0, rx1, sy0}, {rx0, sy1, rx1, ry1}, {rx0, sy0, sx0, sy1}, {sx1, sy0, rx1, sy1}};   // above, below, beside
        for (const auto& b : bands)
        {
            if (b[2] <= b[0] || b[3] <= b[1])
                continue;
            RasterOptions bo = o;
            const Rect& vp = part->viewport;
            bo.viewport = Rect(vp.min.x - (float)b[0], vp.min.y - (float)b[1], vp.max.x - (float)b[0], vp.max.y - (float)b[1]);
            if (!detail::RasterizePart(doc, b[2] - b[0], b[3] - b[1], bo, band_))
                return false;
            for (int y = b[1]; y < b[3]; ++y)
                std::memcpy(row(img, b[0] - rx0, y - ry0), row(band_, kBorder, y - b[1] + kBorder), (std::size_t)(b[2] - b[0]) * 4);
        }
        return true;
    }

    const Atlas::Entry* Atlas::Make(const Document& doc, int width, int height, const RasterOptions& options, std::uint64_t key, const PartOf* part)
    {
        const auto rasterize = [&](Image& img) {
            if (!Pixels(doc, width, height, options, part, img))
                return false;
            ++rasterized_;
            return true;
        };
        // a part's pixels, kept for the next part of its drawing
        const auto keep = [&](Image& img, bool move) {
            if (!part)
                return;
            Part& p = parts_[part->whole];
            p.x0 = part->x0;
            p.y0 = part->y0;
            p.x1 = part->x0 + width;
            p.y1 = part->y0 + height;
            p.frame = frame_;
            if (move)
                p.img = std::move(img);
            else
                p.img = img;
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
                keep(img, true);
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
            // rasterized the last frame and drawn again: into the lasting pages, from the pixels it had then (kept to the
            // next BeginFrame: Tile may take them too)
            slot = Pack(*lasting_, key, before->second, width, height);
        }
        else if ((slot = recent_->Find(key)))
        {
            if (Tiled(width, height, options))
                found_[key] = 1;   // drawn again from the recent pages: if the next frame draws it too, it lasts
        }
        else
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
            keep(img, false);
            made_[key] = std::move(img);
        }
        return slot ? Placed(*slot) : nullptr;
    }

    const text::GlyphSlot* Atlas::Tile(text::TextSystem& text, const Document& doc, int width, int height, const RasterOptions& options)
    {
        if (doc.Empty() || width <= 0 || height <= 0 || !Tiled(width, height, options) || text.TileGeneration() == 0)
            return nullptr;
        const std::uint64_t key = Key(doc, width, height, options);
        const std::uint64_t tileKey = Mix(key, 0x5356475449u);   // apart from Get's keys ("SVGTI")
        if (const text::GlyphSlot* slot = text.FindTile(tileKey))
            return slot;
        // the coverage of a drawing the last frame drew too: from the pixels it was made with then (kept to the next
        // BeginFrame), else rasterized again - in the text's pages it batches with the text
        coverage_.resize((std::size_t)width * (std::size_t)height);
        const auto before = madeBefore_.find(key);
        if (before != madeBefore_.end())
        {
            const Image& img = before->second;
            for (int y = 0; y < height; ++y)
            {
                const std::uint8_t* src = img.pixels.data() + ((std::size_t)(y + kBorder) * (std::size_t)img.width + kBorder) * 4 + 3;
                std::uint8_t* dst = coverage_.data() + (std::size_t)y * (std::size_t)width;
                for (int x = 0; x < width; ++x)
                    dst[x] = src[(std::size_t)x * 4];
            }
        }
        else if (foundBefore_.count(key))
        {
            if (!Rasterize(doc, width, height, options, tileImage_))
                return nullptr;
            ++rasterized_;
            for (std::size_t i = 0; i < coverage_.size(); ++i)
                coverage_[i] = tileImage_.pixels[i * 4 + 3];
        }
        else
            return nullptr;   // new this frame, or not drawn the last: Get's recent pages
        text::GlyphBitmap bitmap;
        bitmap.width = width;
        bitmap.height = height;
        bitmap.channels = 1;
        bitmap.pixels = std::move(coverage_);
        const text::GlyphSlot* slot = text.AddTile(tileKey, bitmap);
        coverage_ = std::move(bitmap.pixels);
        return slot;
    }

    void Draw(Painter& painter, Atlas& atlas, const Document& doc, const Rect& r, Color color, bool mask)
    {
        if (doc.Empty() || r.Empty() || !(color.a > 0.0f))
            return;
        const float k = painter.ScaleFactor();
        if (!(k > 0.0f))
            return;
        painter.GetDrawList().BreakRecording();   // its quads depend on where its pixels are this frame (a draw memo keeps none)
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

        // what the clip rect shows of it, in its pixels (the scissor's whole pixels inside these): none - nothing to
        // rasterize
        const Rect& clip = painter.GetDrawList().ClipRect();
        const auto inside = [](float v, int lo, int hi) { return !(v > (float)lo) ? lo : (v < (float)hi ? (int)v : hi); };
        const int sx0 = inside(std::floor(clip.min.x * px), x0, x0 + w) - x0, sx1 = inside(std::ceil(clip.max.x * px), x0, x0 + w) - x0;
        const int sy0 = inside(std::floor(clip.min.y * px), y0, y0 + h) - y0, sy1 = inside(std::ceil(clip.max.y * px), y0, y0 + h) - y0;
        if (sx1 <= sx0 || sy1 <= sy0)
            return;

        // a monochrome icon that lasts: a tile of the text's pages, drawn in the draw of the text around it (outside
        // masks: the image is an FX shape under one)
        if (o.mask && w <= Atlas::kMaxTile && h <= Atlas::kMaxTile && painter.Env().text && !painter.Masked())
            if (const text::GlyphSlot* tile = atlas.Tile(*painter.Env().text, doc, w, h, o))
            {
                painter.UnscaledCoverage(*tile, Rect((float)x0 / px, (float)y0 / px, (float)(x0 + w) / px, (float)(y0 + h) / px), tint);
                return;
            }

        // A large drawing reaching beyond the clip rect: only the part inside it, when that is at most half of it - in
        // steps of kPartStep pixels from its corner, so that the part is found again while the drawing moves less than
        // that, and a new part takes what it shares with the last (GetPart): a drawing moving through the view
        // rasterizes what comes into view.
        const int px0 = sx0 / kPartStep * kPartStep, py0 = sy0 / kPartStep * kPartStep;
        const int px1 = std::min(w, (sx1 + kPartStep - 1) / kPartStep * kPartStep), py1 = std::min(h, (sy1 + kPartStep - 1) / kPartStep * kPartStep);
        const Atlas::Entry* e = nullptr;
        if ((px1 - px0 < w || py1 - py0 < h) && 2 * (std::int64_t)(px1 - px0) * (py1 - py0) <= (std::int64_t)w * h)
        {
            e = atlas.GetPart(doc, w, h, o, Rect((float)px0, (float)py0, (float)px1, (float)py1));
            x0 += px0;
            y0 += py0;
            w = px1 - px0;
            h = py1 - py0;
        }
        else
            e = atlas.Get(doc, w, h, o);
        if (e)
            painter.UnscaledImage(e->texture, Rect((float)x0 / px, (float)y0 / px, (float)(x0 + w) / px, (float)(y0 + h) / px), tint,
                                  e->uv0, e->uv1);
    }
}
