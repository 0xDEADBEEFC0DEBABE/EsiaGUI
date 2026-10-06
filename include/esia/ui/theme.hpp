// Esia UI - design tokens: colors, glass materials, metrics, typography and motion (WGT's theme, ported).
//
// Widgets never hard-code a color or a size: they read the current theme, which the Ui animates when it changes
// (the light / dark cross-fade). A Theme is a plain struct: copy one of the built-in themes and change what you want.
#pragma once
#include "esia/render/painter.hpp"

namespace esia::ui
{
    // Text styles of Apple's type ramp.
    enum class TextStyle : std::uint8_t
    {
        LargeTitle,
        Title1,
        Title2,
        Title3,
        Headline,
        Body,
        Callout,
        Subheadline,
        Footnote,
        Caption1,
        Caption2,
        Mono,
        Count
    };

    enum class FontWeight : std::uint8_t { Regular, Semibold, Bold, Mono, Count };

    // SwiftUI-like spring: response = period of the undamped oscillation (seconds), damping = ratio (1 = critical).
    struct Spring
    {
        float response = 0.35f;
        float damping = 0.86f;

        static constexpr Spring Snappy() { return {0.25f, 0.90f}; }
        static constexpr Spring Smooth() { return {0.40f, 1.00f}; }
        static constexpr Spring Bouncy() { return {0.45f, 0.62f}; }
        static constexpr Spring Gentle() { return {0.60f, 1.00f}; }
    };

    // How glass surfaces render: Ui::SetGlassLook for the whole UI, ui::Next().Look / a container's style for a part.
    enum class GlassLook : std::uint8_t
    {
        Theme,     // each surface's material from the theme
        Clear,     // fully transparent: only the lens, the rim light and the hairline - no frost, no tint, no fill
        Frosted,   // iOS frost everywhere: soft blur under a light veil (heavier on the bevel), see-through
    };

    struct Palette
    {
        Color accent;
        Color onAccent;
        // labels
        Color label, secondaryLabel, tertiaryLabel, quaternaryLabel;
        // backgrounds
        Color background, secondaryBackground, tertiaryBackground;
        Color groupedBackground, secondaryGroupedBackground, tertiaryGroupedBackground;
        // fills (controls)
        Color fill, secondaryFill, tertiaryFill, quaternaryFill;
        Color separator, opaqueSeparator;
        // system colors
        Color red, orange, yellow, green, mint, teal, cyan, blue, indigo, purple, pink, brown, gray;
        // surfaces
        Color windowSurface;   // tint over glass windows (readability)
        Color cardSurface;     // inset-grouped section cards on glass
        Color controlKnob;     // switch / slider knobs
        Color shadow;          // ambient shadow color
        Color highlight;       // press / hover highlight
    };
    constexpr std::size_t kPaletteColorCount = sizeof(Palette) / sizeof(Color);

    struct Materials
    {
        GlassMaterial window;    // floating windows / panels
        GlassMaterial bar;       // tab bars, docks, toolbars
        GlassMaterial control;   // glass buttons, knobs, pills
        GlassMaterial popover;   // menus, tooltips
        GlassMaterial clear;     // "clear" glass: minimal frost, maximal lensing
    };

    // Sizes in UI units. Esia's UI units already follow the display's pixel density (FrameParams::framebufferScale),
    // so `scale` is only a design zoom on top of it (1 by default).
    struct Metrics
    {
        float scale = 1.0f;
        float windowRadius = 28.0f;
        float cardRadius = 16.0f;
        float controlRadius = 12.0f;
        float cornerSmoothing = 0.6f;  // 0 = circular corners, 1 = continuous (squircle) corners
        float padding = 16.0f;
        float spacing = 10.0f;
        float rowHeight = 44.0f;       // a list row (Row*, ListRow)
        // for custom widgets: the built-in controls are 32 - 34 tall (at scale 1) whatever it says
        float controlHeight = 34.0f;
        float iconTile = 28.0f;
        float headerHeight = 54.0f;
        float sectionSpacing = 22.0f;
        float hairline = 1.0f;
        float scrollIndicator = 5.0f;
        // 0: scroll indicators show while an area scrolls and hide at rest (iOS); 1: an area that scrolls always shows
        // its indicator, dimmed at rest, on a faint track (desktop apps, where users look for a scroll bar). It is 1
        // in ThemeLight / ThemeDark on Windows, Linux and macOS, 0 on iOS and Android; after a finger's press the
        // indicators hide at rest anyway. A float, as every metric: a theme transition blends it.
        float scrollIndicatorAlways = 1.0f;
    };

    struct Typography
    {
        float size[(int)TextStyle::Count] = {
            30.0f,   // LargeTitle
            25.0f,   // Title1
            20.0f,   // Title2
            17.5f,   // Title3
            15.0f,   // Headline
            15.0f,   // Body
            14.0f,   // Callout
            13.5f,   // Subheadline
            12.5f,   // Footnote
            11.5f,   // Caption1
            11.0f,   // Caption2
            13.5f,   // Mono
        };
        FontWeight weight[(int)TextStyle::Count] = {
            FontWeight::Bold, FontWeight::Bold, FontWeight::Bold, FontWeight::Semibold,
            FontWeight::Semibold, FontWeight::Regular, FontWeight::Regular, FontWeight::Regular,
            FontWeight::Regular, FontWeight::Regular, FontWeight::Regular, FontWeight::Mono,
        };
    };

    struct Motion
    {
        Spring fast = Spring::Snappy();
        Spring standard = {0.35f, 0.86f};
        Spring bouncy = Spring::Bouncy();
        Spring gentle = Spring::Gentle();
        float themeTransition = 0.45f;   // seconds
    };

    struct Theme
    {
        bool dark = false;
        Palette colors;
        Materials materials;
        Metrics metrics;
        Typography type;
        Motion motion;

        float S(float v) const { return v * metrics.scale; }
    };

    ESIA_API Theme ThemeLight();
    ESIA_API Theme ThemeDark();
    // `base` at a desktop tool's density: everything at 0.87 of its size (body text 13 px, controls about 29),
    // shorter list rows and headers, less padding and spacing, smaller corners. SetDarkMode keeps it.
    ESIA_API Theme ThemeCompact(const Theme& base);
    // `base` with another accent (a Clear accent leaves it as it is).
    ESIA_API Theme ThemeWithAccent(const Theme& base, Color accent);
    // Component-wise interpolation (animated transitions): colors, materials, metrics and type sizes blend; `dark`,
    // font weights and motion switch at the middle.
    ESIA_API Theme LerpTheme(const Theme& a, const Theme& b, float t);

    // The theme in effect while a transition runs: Set() starts one, Step() advances it.
    struct ThemeAnimator
    {
        Theme current = ThemeLight();
        Theme from = current, to = current;
        float t = 1.0f;
        float duration = 0.45f;

        ESIA_API void Set(const Theme& target, bool animate);
        ESIA_API void Step(float dt);
        bool Animating() const { return t < 1.0f; }
    };
}
