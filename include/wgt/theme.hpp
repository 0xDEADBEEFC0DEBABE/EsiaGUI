// WGT UI - design tokens
//
// A Theme is a plain struct of tokens (colors, glass materials, metrics, typography, motion).
// Widgets never hard-code a color or size: everything is read from the *current* theme, which the
// ThemeManager animates when switching (e.g. light <-> dark cross-fade).
#pragma once
#include "math.hpp"
#include "text.hpp"

namespace wgt
{
    // Apple-style text styles.
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

    // SwiftUI-like spring: response = period of the undamped oscillation (s), damping = ratio (1 = critical).
    struct Spring
    {
        float response = 0.35f;
        float damping = 0.86f;

        static constexpr Spring Snappy() { return {0.25f, 0.90f}; }
        static constexpr Spring Smooth() { return {0.40f, 1.00f}; }
        static constexpr Spring Bouncy() { return {0.45f, 0.62f}; }
        static constexpr Spring Gentle() { return {0.60f, 1.00f}; }
    };

    // How glass surfaces render: Context::SetGlassLook for the whole UI, ui::PushGlassLook for a scope,
    // WindowFlags_ClearGlass / PanelFlags_ClearGlass for one window.
    enum class GlassLook : std::uint8_t
    {
        Theme,     // each surface's material from the theme
        Clear,     // fully transparent: only the lens, the rim light and the hairline - no frost, no tint, no fill
        Frosted,   // iOS frost everywhere: soft blur under a light, layered veil (heavier on the bevel), see-through
    };

    // Liquid-glass material (all distances in UI units at scale 1). The glass is a clear slab with a rounded
    // edge: the interior shows the content behind (frosted by `blur`), the edge refracts it like a lens.
    struct GlassMaterial
    {
        float blur = 2.0f;         // frost: backdrop blur radius (0 = crystal clear)
        float refraction = 8.0f;   // lensing: how far the content is pulled in (magnified) at the rim
        float bezel = 40.0f;       // radius of the curved edge; >= half the shape's size = one rounded rod / dome
        float dispersion = 0.35f;  // chromatic separation in the lensing band (0..1)
        float saturation = 1.30f;  // vibrancy: backdrop saturation
        float brightness = 0.03f;  // vibrancy: added light
        float specular = 0.85f;    // rim highlight strength (Fresnel-weighted, on the edge facing the light)
        float lightAngle = -2.2f;  // light direction (radians, screen space; default = from top-left)
        float noise = 0.012f;      // film grain to avoid banding
        float legibility = 0.0f;   // 0..1: exposes the backdrop (its wide-area brightness) toward the tint's, so dark
                                   // glass stays dark over a bright wallpaper and light glass stays light over a dark
                                   // one; detail and color behind survive
        float magnify = 0.0f;      // loupe: the content behind is enlarged by 1 + magnify around the shape's center
                                   // (iOS selection lenses and dragged knobs)
        Color tint = Color(1, 1, 1, 0.18f);  // rgb tint, a = amount (washes the content behind towards it)
        Color rim = Color(1, 1, 1, 0.35f);   // hairline rim stroke
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
        Color windowSurface;   // tint drawn over glass windows (readability)
        Color cardSurface;     // inset-grouped section cards on glass
        Color controlKnob;     // switch / slider knobs
        Color shadow;          // ambient shadow color
        Color highlight;       // press / hover highlight
    };
    constexpr std::size_t kPaletteColorCount = sizeof(Palette) / sizeof(Color);

    struct Materials
    {
        GlassMaterial window;   // floating windows / panels
        GlassMaterial bar;      // tab bars, docks, toolbars
        GlassMaterial control;  // glass buttons, knobs, pills
        GlassMaterial popover;  // menus, tooltips
        GlassMaterial clear;    // "clear" glass: minimal frost, maximal lensing
    };

    struct Metrics
    {
        float scale = 1.0f;            // DPI / user scale, multiplied into every metric below
        float windowRadius = 28.0f;
        float cardRadius = 16.0f;
        float controlRadius = 12.0f;
        float cornerSmoothing = 0.6f;  // 0 = circular corners, 1 = continuous (squircle) corners
        float padding = 16.0f;
        float spacing = 10.0f;
        float rowHeight = 44.0f;
        float controlHeight = 34.0f;
        float iconTile = 28.0f;
        float headerHeight = 54.0f;
        float sectionSpacing = 22.0f;
        float hairline = 1.0f;
        float scrollIndicator = 5.0f;
    };

    struct Typography
    {
        float size[(int)TextStyle::Count] = {
            30.0f,  // LargeTitle
            25.0f,  // Title1
            20.0f,  // Title2
            17.5f,  // Title3
            15.0f,  // Headline
            15.0f,  // Body
            14.0f,  // Callout
            13.5f,  // Subheadline
            12.5f,  // Footnote
            11.5f,  // Caption1
            11.0f,  // Caption2
            13.5f,  // Mono
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
        float themeTransition = 0.45f;  // seconds
    };

    struct Theme
    {
        bool dark = false;
        Palette colors;
        Materials materials;
        Metrics metrics;
        Typography type;
        Motion motion;

        // Scaled metric helper: theme.S(12) == 12 * metrics.scale
        float S(float v) const { return v * metrics.scale; }
    };

    // Built-in themes.
    WGT_API Theme ThemeLight();
    WGT_API Theme ThemeDark();
    // Accent variants.
    WGT_API Theme ThemeWithAccent(const Theme& base, Color accent);
    // Component-wise interpolation of two themes (used for animated transitions).
    WGT_API Theme LerpTheme(const Theme& a, const Theme& b, float t);
    // Maps the theme onto ImGuiStyle so stock ImGui widgets blend in.
    WGT_API void ApplyThemeToImGuiStyle(const Theme& theme, ImGuiStyle& style);
}
