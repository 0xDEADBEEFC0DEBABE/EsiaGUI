// glass_window - the frame's text system on systems without Windows' icon fonts (macOS, iOS, Linux): text through an
// inner text system (FreeType), and the icons of esia/ui/icons.hpp for UiDesc::iconFontFile = kSystemSymbolsFont from
// the system's own symbols - SF Symbols (app_apple.mm), the desktop's icon theme (icons_linux.cpp).
#pragma once
#include "app.hpp"
#include "esia/core/draw_list.hpp"
#include "esia/core/texture.hpp"
#include "esia/text/text.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace glass
{
    // The coverage of the symbol for code point `c` (esia/ui/icons.hpp) fitted into a px x px em box, w x h bytes;
    // false when the system has none for it.
    using RasterizeSymbolFn = bool (*)(char32_t c, int px, std::vector<std::uint8_t>& coverage, int& w, int& h);

    // The frame's text system: `inner` (FreeType) for text, the system's symbols for kSystemSymbolsFont (everything else
    // is forwarded), rasterized by `rasterize` into Alpha8 atlas pages of its own.
    // A symbol is rasterized at its exact pixel size, the first time it is drawn and then for every size that holds
    // for two frames. A size that changes every frame (pressed and bouncing controls scale their icons) draws from
    // the symbol's raster at the next size step, scaled: rasterizing each frame's size would fill the atlas within
    // a few presses, and then icons went missing. Up to kMaxPages atlas pages; once they are full, the next frame
    // starts them over with the rasters drawn recently.
    class SymbolTextSystem final : public esia::text::TextSystem
    {
    public:
        SymbolTextSystem(std::unique_ptr<esia::text::TextSystem> inner, esia::TextureRegistry& textures, RasterizeSymbolFn rasterize)
            : inner_(std::move(inner)), textures_(textures), rasterize_(rasterize)
        {
        }
        ~SymbolTextSystem() override
        {
            for (const Page& page : pages_)
                textures_.Destroy(page.texture);
        }

        esia::text::FontId AddFontFile(const char* path, int faceIndex) override
        {
            return path && std::strcmp(path, kSystemSymbolsFont) == 0 ? kSymbols : inner_->AddFontFile(path, faceIndex);
        }
        esia::text::FontId AddFontMemory(const void* data, std::size_t size, int faceIndex) override { return inner_->AddFontMemory(data, size, faceIndex); }
        void AddFallback(esia::text::FontId font) override
        {
            if (font != kSymbols)
                inner_->AddFallback(font);
        }
        void NewFrame(const esia::text::RasterParams& params) override
        {
            pixelsPerUnit_ = params.pixelsPerUnit > 0.0f ? params.pixelsPerUnit : 1.0f;
            ++frame_;
            if (full_)
                StartOver();
            inner_->NewFrame(params);
        }
        esia::text::TextMetrics Measure(esia::text::FontRef font, std::string_view text, float wrapWidth, std::uint32_t flags) override
        {
            if (font.id == kSymbols)
                return {esia::Vec2(font.size, font.size), font.size * 0.8f, 1};
            return inner_->Measure(font, text, wrapWidth, flags);
        }
        esia::Vec2 Draw(esia::DrawList& dl, esia::text::FontRef font, esia::Vec2 pos, esia::Color color, std::string_view text, float wrapWidth,
                        std::uint32_t flags, float scale) override
        {
            if (font.id == kSymbols)
                return esia::Vec2(font.size, font.size);
            return inner_->Draw(dl, font, pos, color, text, wrapWidth, flags, scale);
        }
        // the inner system's stops (FreeType: grapheme clusters, right-to-left runs), not the default per code point
        void CaretStops(esia::text::FontRef font, std::string_view text, std::vector<esia::text::CaretStop>& out) override
        {
            inner_->CaretStops(font, text, out);
        }
        void DrawGlyph(esia::DrawList& dl, esia::text::FontRef font, char32_t codepoint, esia::Vec2 center, esia::Color color) override
        {
            if (font.id != kSymbols)
                return inner_->DrawGlyph(dl, font, codepoint, center, color);
            const int px = std::clamp((int)std::lround(font.size * pixelsPerUnit_), 1, kMaxPx);
            const Slot* slot = Exact(codepoint, px);
            float k = 1.0f;   // the raster's pixels -> the size asked for
            if (!slot)
            {
                const int step = Step(px);
                slot = Get(codepoint, step);
                k = (float)px / (float)step;
            }
            if (!slot)
                return;
            const float w = (float)slot->w * k / pixelsPerUnit_, h = (float)slot->h * k / pixelsPerUnit_;
            // an exact raster on whole pixels, as text; a scaled one is in motion
            float x = center.x - w * 0.5f, y = center.y - h * 0.5f;
            if (k == 1.0f)
            {
                x = std::round(x * pixelsPerUnit_) / pixelsPerUnit_;
                y = std::round(y * pixelsPerUnit_) / pixelsPerUnit_;
            }
            const float inv = 1.0f / (float)kPage;
            dl.AddImage(pages_[(std::size_t)slot->page].texture, esia::Rect(x, y, x + w, y + h), esia::Vec2((float)slot->x * inv, (float)slot->y * inv),
                        esia::Vec2((float)(slot->x + slot->w) * inv, (float)(slot->y + slot->h) * inv), color.ToRgba8());
        }

    private:
        static constexpr esia::text::FontId kSymbols = 0x7FFF5F01;
        static constexpr int kPage = 1024, kMaxPages = 4, kMaxPx = 256;
        static constexpr std::uint64_t kRecent = 120;   // frames: what starting over keeps
        struct Page
        {
            esia::TextureId texture = 0;
            int shelfX = 1, shelfY = 1, shelfH = 0;
        };
        struct Slot
        {
            int page = 0, x = 0, y = 0, w = 0, h = 0;
            bool ok = false;           // false: no such symbol (never retried)
            std::uint64_t used = 0;    // the last frame it was drawn
            std::vector<std::uint8_t> coverage;   // kept for starting over
        };

        static std::uint64_t Key(char32_t c, int px) { return ((std::uint64_t)c << 16) | (std::uint64_t)px; }
        // the size step at or above px: a scaled raster is at most 1 / 8 larger than drawn
        static int Step(int px)
        {
            const int step = px <= 32 ? 4 : px <= 64 ? 8 : px <= 128 ? 16 : 32;
            return std::min(kMaxPx, (px + step - 1) / step * step);
        }

        // The symbol's raster at exactly px, when there is one or the size holds (asked for last frame too);
        // null: draw from a size step.
        const Slot* Exact(char32_t c, int px)
        {
            const std::uint64_t key = Key(c, px);
            if (auto it = slots_.find(key); it != slots_.end())
            {
                it->second.used = frame_;
                return it->second.ok ? &it->second : nullptr;
            }
            std::uint64_t& asked = asked_[key];
            const bool holds = asked != 0 && asked + 1 >= frame_;
            asked = frame_;
            return holds || !drawn_.count(c) ? Get(c, px) : nullptr;
        }

        // The symbol at px, rasterized now if need be.
        const Slot* Get(char32_t c, int px)
        {
            const std::uint64_t key = Key(c, px);
            if (auto it = slots_.find(key); it != slots_.end())
            {
                it->second.used = frame_;
                return it->second.ok ? &it->second : nullptr;
            }
            Slot slot;
            slot.used = frame_;
            slot.ok = rasterize_(c, px, slot.coverage, slot.w, slot.h);
            if (slot.ok && !Place(slot))
                return nullptr;   // the pages are full: the next frame starts over (not remembered as missing)
            if (slot.ok)
                drawn_.insert(c);
            Slot& kept = slots_[key] = std::move(slot);
            return kept.ok ? &kept : nullptr;
        }

        // A place on a page (shelf packing), the coverage uploaded; false when every page is full.
        bool Place(Slot& s)
        {
            for (std::size_t i = 0; i <= pages_.size() && i < (std::size_t)kMaxPages; ++i)
            {
                if (i == pages_.size())
                    pages_.push_back({textures_.Create({esia::TextureFormat::Alpha8, kPage, kPage})});
                Page& p = pages_[i];
                if (p.shelfX + s.w + 1 > kPage)
                {
                    p.shelfX = 1;
                    p.shelfY += p.shelfH + 1;
                    p.shelfH = 0;
                }
                if (p.shelfY + s.h + 1 > kPage)
                    continue;
                s.page = (int)i;
                s.x = p.shelfX;
                s.y = p.shelfY;
                textures_.Update(p.texture, s.x, s.y, s.w, s.h, s.coverage.data());
                p.shelfX += s.w + 1;
                p.shelfH = std::max(p.shelfH, s.h);
                return true;
            }
            full_ = true;
            return false;
        }

        // Between frames, once the pages were full: new pages with the rasters drawn recently.
        void StartOver()
        {
            full_ = false;
            for (const Page& page : pages_)
                textures_.Destroy(page.texture);
            pages_.clear();
            for (auto it = slots_.begin(); it != slots_.end();)
            {
                Slot& s = it->second;
                if (s.ok && (s.used + kRecent < frame_ || !Place(s)))
                    it = slots_.erase(it);
                else
                    ++it;
            }
            std::erase_if(asked_, [&](const auto& a) { return a.second + 2 < frame_; });
        }

        std::unique_ptr<esia::text::TextSystem> inner_;
        esia::TextureRegistry& textures_;
        RasterizeSymbolFn rasterize_;
        float pixelsPerUnit_ = 1.0f;
        std::uint64_t frame_ = 1;
        bool full_ = false;
        std::vector<Page> pages_;
        std::unordered_map<std::uint64_t, Slot> slots_;
        std::unordered_map<std::uint64_t, std::uint64_t> asked_;   // exact sizes not rasterized: the last frame asked for
        std::unordered_set<char32_t> drawn_;                       // symbols with a raster
    };
}
