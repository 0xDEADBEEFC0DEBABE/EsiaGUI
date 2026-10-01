// glass_window - esia::ui's icons on Android: Material Icons (Outlined), the icons Android apps use. Android has no icon
// font of its own for apps: the font is packed into each example's APK (the build downloads it, glass_add_app) and
// drawn by the FreeType text system under the code points of esia/ui/icons.hpp (icon_font_text.hpp).
#pragma once

namespace glass::material_icons
{
    // The APK asset with the font (Google's Material Icons Outlined, Apache License 2.0).
    inline constexpr const char* kAsset = "MaterialIconsOutlined-Regular.otf";

    // Material draws its icons in 20 of the 24 units of its em, Segoe Fluent fills the em: the em size that makes a
    // Material icon as large as the Segoe one at the same font size.
    inline constexpr float kScale = 1.2f;

    // The Material code point for icon `c` of esia/ui/icons.hpp (Segoe Fluent code points); 0 when it has none.
    char32_t Map(char32_t c);
}
