// glass_window - what the example draws: a wallpaper and liquid-glass cards through esia::Painter, and two core
// windows (dragged by their empty area, resized from their edges) holding a button and a text field.
#pragma once
#include "app.hpp"
#include "esia/text/text.hpp"
#include <string>

namespace glass
{
    class Scene
    {
    public:
        // `text` may be null (built without esia_text_ft, or no font found): labels are left out and the text field
        // shows one dot per character.
        Scene(esia::text::TextSystem* text, esia::text::FontId font) : text_(text), font_(font) {}

        // Between Context::NewFrame and EndFrame.
        void Frame(esia::Context& ctx, const SceneInfo& info);

    private:
        void Wallpaper(esia::Context& ctx, float t);
        void ControlsWindow(esia::Context& ctx, const SceneInfo& info);
        void TextWindow(esia::Context& ctx);
        void TextField(esia::Context& ctx, esia::Id id, const esia::Rect& bb);
        float TextWidth(std::string_view s) const;

        esia::text::TextSystem* text_;
        esia::text::FontId font_;
        int clicks_ = 0;
        std::string field_;
    };
}
