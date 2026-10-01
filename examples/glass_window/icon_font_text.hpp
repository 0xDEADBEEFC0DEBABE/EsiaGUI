// glass_window - the frame's text system on Android: text through an inner text system (FreeType), and the icons of
// esia/ui/icons.hpp for UiDesc::iconFontFile = kSystemSymbolsFont from an icon font the app brings (Material Icons,
// icons_material.hpp), drawn by the inner system under the font's own code points. symbol_text.hpp does the same with
// the system's symbols where there are some (macOS, iOS, Linux).
#pragma once
#include "app.hpp"
#include "esia/text/text.hpp"
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

namespace glass
{
    // The icon font's code point for icon `c` of esia/ui/icons.hpp; 0 when it has none (nothing is drawn).
    using MapIconFn = char32_t (*)(char32_t c);

    class IconFontTextSystem final : public esia::text::TextSystem
    {
    public:
        // `font`: the icon font file (kept: the inner system reads it in place; empty: no icons); `scale`: the font's em
        // size for an icon of em size 1.
        IconFontTextSystem(std::unique_ptr<esia::text::TextSystem> inner, std::vector<std::uint8_t> font, MapIconFn map, float scale)
            : font_(std::move(font)), inner_(std::move(inner)), map_(map), scale_(scale)
        {
        }

        esia::text::FontId AddFontFile(const char* path, int faceIndex) override
        {
            if (!path || std::strcmp(path, kSystemSymbolsFont) != 0)
                return inner_->AddFontFile(path, faceIndex);
            if (!icons_ && !font_.empty())
                icons_ = inner_->AddFontMemory(font_.data(), font_.size(), 0);
            return icons_;
        }
        esia::text::FontId AddFontMemory(const void* data, std::size_t size, int faceIndex) override { return inner_->AddFontMemory(data, size, faceIndex); }
        void AddFallback(esia::text::FontId font) override
        {
            if (font != icons_ || !icons_)
                inner_->AddFallback(font);
        }
        void NewFrame(const esia::text::RasterParams& params) override { inner_->NewFrame(params); }
        esia::text::TextMetrics Measure(esia::text::FontRef font, std::string_view text, float wrapWidth, std::uint32_t flags) override
        {
            return inner_->Measure(font, text, wrapWidth, flags);
        }
        esia::Vec2 Draw(esia::DrawList& dl, esia::text::FontRef font, esia::Vec2 pos, esia::Color color, std::string_view text, float wrapWidth,
                        std::uint32_t flags, float scale) override
        {
            return inner_->Draw(dl, font, pos, color, text, wrapWidth, flags, scale);
        }
        void DrawGlyph(esia::DrawList& dl, esia::text::FontRef font, char32_t codepoint, esia::Vec2 center, esia::Color color) override
        {
            if (!icons_ || font.id != icons_)
                return inner_->DrawGlyph(dl, font, codepoint, center, color);
            if (const char32_t c = map_(codepoint))
                inner_->DrawGlyph(dl, esia::text::FontRef{font.id, font.size * scale_}, c, center, color);
        }
        void CaretStops(esia::text::FontRef font, std::string_view text, std::vector<esia::text::CaretStop>& out) override
        {
            inner_->CaretStops(font, text, out);
        }

    private:
        std::vector<std::uint8_t> font_;   // declared before inner_: outlives it
        std::unique_ptr<esia::text::TextSystem> inner_;
        MapIconFn map_;
        float scale_;
        esia::text::FontId icons_ = 0;
    };
}
