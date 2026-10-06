// Esia UI - the built-in themes (WGT's values), their interpolation and the animator.
#include "esia/ui/anim.hpp"
#include "esia/ui/theme.hpp"
#include <algorithm>
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

namespace esia::ui
{
    namespace
    {
        Color Rgba(int r, int g, int b, float a = 1.0f) { return Color(r / 255.0f, g / 255.0f, b / 255.0f, a); }

        GlassMaterial Mat(float blur, float refraction, float bezel, float dispersion, float saturation, float brightness,
                          float specular, Color tint, Color rim, float noise = 0.012f)
        {
            GlassMaterial m;
            m.blur = blur;
            m.refraction = refraction;
            m.bezel = bezel;
            m.dispersion = dispersion;
            m.saturation = saturation;
            m.brightness = brightness;
            m.specular = specular;
            m.tint = tint;
            m.rim = rim;
            m.noise = noise;
            return m;
        }

        GlassMaterial LerpMaterial(const GlassMaterial& a, const GlassMaterial& b, float t)
        {
            GlassMaterial m;
            m.blur = Lerp(a.blur, b.blur, t);
            m.refraction = Lerp(a.refraction, b.refraction, t);
            m.bezel = Lerp(a.bezel, b.bezel, t);
            m.dispersion = Lerp(a.dispersion, b.dispersion, t);
            m.saturation = Lerp(a.saturation, b.saturation, t);
            m.brightness = Lerp(a.brightness, b.brightness, t);
            m.specular = Lerp(a.specular, b.specular, t);
            m.lightAngle = Lerp(a.lightAngle, b.lightAngle, t);
            m.noise = Lerp(a.noise, b.noise, t);
            m.legibility = Lerp(a.legibility, b.legibility, t);
            m.magnify = Lerp(a.magnify, b.magnify, t);
            m.tint = Lerp(a.tint, b.tint, t);
            m.rim = Lerp(a.rim, b.rim, t);
            return m;
        }

        // the palette as an array of colors, for the component-wise blend (Palette is only Colors)
        static_assert(sizeof(Palette) == kPaletteColorCount * sizeof(Color), "Palette holds colors only");
        static_assert(sizeof(Metrics) % sizeof(float) == 0, "Metrics holds floats only");
    }

    namespace
    {
        // phones hide scroll indicators at rest, desktops keep them (Metrics::scrollIndicatorAlways)
#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IPHONE)
        constexpr float kIndicatorsAlways = 0.0f;
#else
        constexpr float kIndicatorsAlways = 1.0f;
#endif
    }

    Theme ThemeLight()
    {
        Theme t;
        t.dark = false;
        t.metrics.scrollIndicatorAlways = kIndicatorsAlways;
        Palette& c = t.colors;
        c.accent = Color::Hex(0x007AFF);
        c.onAccent = Color::White();
        c.label = Color::Black();
        c.secondaryLabel = Rgba(60, 60, 67, 0.62f);
        c.tertiaryLabel = Rgba(60, 60, 67, 0.32f);
        c.quaternaryLabel = Rgba(60, 60, 67, 0.18f);
        c.background = Color::White();
        c.secondaryBackground = Color::Hex(0xF2F2F7);
        c.tertiaryBackground = Color::White();
        c.groupedBackground = Color::Hex(0xF2F2F7);
        c.secondaryGroupedBackground = Color::White();
        c.tertiaryGroupedBackground = Color::Hex(0xF2F2F7);
        c.fill = Rgba(120, 120, 128, 0.20f);
        c.secondaryFill = Rgba(120, 120, 128, 0.16f);
        c.tertiaryFill = Rgba(118, 118, 128, 0.12f);
        c.quaternaryFill = Rgba(116, 116, 128, 0.08f);
        c.separator = Rgba(60, 60, 67, 0.22f);
        c.opaqueSeparator = Color::Hex(0xC6C6C8);
        c.red = Color::Hex(0xFF3B30);
        c.orange = Color::Hex(0xFF9500);
        c.yellow = Color::Hex(0xFFCC00);
        c.green = Color::Hex(0x34C759);
        c.mint = Color::Hex(0x00C7BE);
        c.teal = Color::Hex(0x30B0C7);
        c.cyan = Color::Hex(0x32ADE6);
        c.blue = Color::Hex(0x007AFF);
        c.indigo = Color::Hex(0x5856D6);
        c.purple = Color::Hex(0xAF52DE);
        c.pink = Color::Hex(0xFF2D55);
        c.brown = Color::Hex(0xA2845E);
        c.gray = Color::Hex(0x8E8E93);
        c.windowSurface = Color(1, 1, 1, 0.12f);
        c.cardSurface = Color(1, 1, 1, 0.66f);
        c.controlKnob = Color::White();
        c.shadow = Color(0, 0, 0, 0.16f);
        c.highlight = Color(0, 0, 0, 0.06f);

        Materials& m = t.materials;
        // Liquid glass (iOS): what is behind stays sharp (little frost) and is washed toward the glass's own tone for
        // legibility; bars and controls are one rounded rod (bezel >= half their size) that magnifies the content
        // toward the edge. refraction = how far the content is pulled in at the rim.
        //             frost lens bezel disp   sat    bright spec   tint                   rim
        m.window = Mat(10, 14, 26, 0.30f, 1.40f, 0.03f, 0.60f, Color(1, 1, 1, 0.10f), Color(1, 1, 1, 0.55f));
        m.bar = Mat(3, 12, 40, 0.12f, 1.25f, 0.04f, 0.85f, Color(1, 1, 1, 0.56f), Color(0, 0, 0, 0.22f));
        m.control = Mat(2, 8, 40, 0.15f, 1.30f, 0.04f, 0.90f, Color(1, 1, 1, 0.18f), Color(0, 0, 0, 0.15f));
        m.popover = Mat(22, 8, 16, 0.20f, 1.50f, 0.04f, 0.55f, Color(1, 1, 1, 0.38f), Color(1, 1, 1, 0.60f));
        m.clear = Mat(0.6f, 14, 40, 0.50f, 1.15f, 0.02f, 1.00f, Color(1, 1, 1, 0.00f), Color(1, 1, 1, 0.65f), 0.0f);
        m.window.legibility = 0.30f;
        m.bar.legibility = 0.25f;
        m.popover.legibility = 0.40f;
        return t;
    }

    Theme ThemeDark()
    {
        Theme t;
        t.dark = true;
        t.metrics.scrollIndicatorAlways = kIndicatorsAlways;
        Palette& c = t.colors;
        c.accent = Color::Hex(0x0A84FF);
        c.onAccent = Color::White();
        c.label = Color::White();
        c.secondaryLabel = Rgba(235, 235, 245, 0.62f);
        c.tertiaryLabel = Rgba(235, 235, 245, 0.32f);
        c.quaternaryLabel = Rgba(235, 235, 245, 0.18f);
        c.background = Color::Black();
        c.secondaryBackground = Color::Hex(0x1C1C1E);
        c.tertiaryBackground = Color::Hex(0x2C2C2E);
        c.groupedBackground = Color::Black();
        c.secondaryGroupedBackground = Color::Hex(0x1C1C1E);
        c.tertiaryGroupedBackground = Color::Hex(0x2C2C2E);
        c.fill = Rgba(120, 120, 128, 0.36f);
        c.secondaryFill = Rgba(120, 120, 128, 0.32f);
        c.tertiaryFill = Rgba(118, 118, 128, 0.24f);
        c.quaternaryFill = Rgba(118, 118, 128, 0.18f);
        c.separator = Rgba(84, 84, 88, 0.60f);
        c.opaqueSeparator = Color::Hex(0x38383A);
        c.red = Color::Hex(0xFF453A);
        c.orange = Color::Hex(0xFF9F0A);
        c.yellow = Color::Hex(0xFFD60A);
        c.green = Color::Hex(0x30D158);
        c.mint = Color::Hex(0x63E6E2);
        c.teal = Color::Hex(0x40C8E0);
        c.cyan = Color::Hex(0x64D2FF);
        c.blue = Color::Hex(0x0A84FF);
        c.indigo = Color::Hex(0x5E5CE6);
        c.purple = Color::Hex(0xBF5AF2);
        c.pink = Color::Hex(0xFF375F);
        c.brown = Color::Hex(0xAC8E68);
        c.gray = Color::Hex(0x8E8E93);
        c.windowSurface = Rgba(22, 22, 26, 0.20f);
        c.cardSurface = Rgba(48, 48, 54, 0.52f);
        c.controlKnob = Color::White();
        c.shadow = Color(0, 0, 0, 0.42f);
        c.highlight = Color(1, 1, 1, 0.08f);

        Materials& m = t.materials;
        //             frost lens bezel disp   sat    bright  spec   tint                     rim
        m.window = Mat(10, 14, 26, 0.30f, 1.25f, -0.02f, 0.45f, Rgba(16, 16, 20, 0.22f), Color(1, 1, 1, 0.20f));
        m.bar = Mat(3, 12, 40, 0.20f, 1.20f, 0.00f, 0.60f, Rgba(40, 40, 43, 0.66f), Color(1, 1, 1, 0.20f));
        m.control = Mat(2, 8, 40, 0.20f, 1.25f, 0.02f, 0.75f, Rgba(40, 40, 46, 0.24f), Color(1, 1, 1, 0.26f));
        m.popover = Mat(22, 8, 16, 0.20f, 1.50f, 0.00f, 0.45f, Rgba(30, 30, 34, 0.52f), Color(1, 1, 1, 0.22f));
        m.clear = Mat(0.6f, 14, 40, 0.50f, 1.15f, 0.00f, 0.85f, Color(0, 0, 0, 0.04f), Color(1, 1, 1, 0.40f), 0.0f);
        m.window.legibility = 0.58f;   // dark glass over a bright backdrop must stay dark enough for white labels
        m.bar.legibility = 0.30f;
        m.popover.legibility = 0.50f;
        return t;
    }

    Metrics DensityMetrics(const Metrics& metrics)
    {
        Metrics m = metrics;
        const float c = std::clamp(m.compact, 0.0f, 1.0f);
        if (c <= 0.0f)
            return m;   // iOS's sizes, untouched
        // The text keeps its size (the scale and the type are left alone, as WinUI's and Material's compact
        // densities leave them); the space around it shrinks. Each size at `k` of itself when fully compact - the
        // built-in theme's then: rows 34, window headers 40, padding 12, spacing 8 - and in between while a switch
        // animates.
        const auto at = [c](float k) { return 1.0f + (k - 1.0f) * c; };
        m.controlHeight *= at(28.0f / 34.0f);   // custom widgets that follow it, as the built-in controls (ControlSizes)
        m.rowHeight *= at(34.0f / 44.0f);       // WinUI: list items 40 -> 32; a switch in the row keeps its margin
        m.headerHeight *= at(40.0f / 54.0f);
        m.iconTile *= at(22.0f / 28.0f);
        m.padding *= at(12.0f / 16.0f);
        m.spacing *= at(8.0f / 10.0f);           // WinUI's 8 between buttons: the glass keeps apart
        m.sectionSpacing *= at(16.0f / 22.0f);
        m.windowRadius *= at(20.0f / 28.0f);
        m.cardRadius *= at(12.0f / 16.0f);
        m.controlRadius *= at(8.0f / 12.0f);
        return m;
    }

    Theme ThemeWithAccent(const Theme& base, Color accent)
    {
        Theme t = base;
        if (accent.a > 0.0f)
            t.colors.accent = accent;
        return t;
    }

    Theme LerpTheme(const Theme& a, const Theme& b, float t)
    {
        if (t <= 0.0f)
            return a;
        if (t >= 1.0f)
            return b;
        Theme r = b;
        r.dark = t < 0.5f ? a.dark : b.dark;
        for (int i = 0; i < (int)TextStyle::Count; ++i)
        {
            r.type.size[i] = Lerp(a.type.size[i], b.type.size[i], t);
            r.type.weight[i] = t < 0.5f ? a.type.weight[i] : b.type.weight[i];
        }
        r.motion = t < 0.5f ? a.motion : b.motion;
        const Color* ca = reinterpret_cast<const Color*>(&a.colors);
        const Color* cb = reinterpret_cast<const Color*>(&b.colors);
        Color* cr = reinterpret_cast<Color*>(&r.colors);
        for (std::size_t i = 0; i < kPaletteColorCount; ++i)
            cr[i] = Lerp(ca[i], cb[i], t);
        r.materials.window = LerpMaterial(a.materials.window, b.materials.window, t);
        r.materials.bar = LerpMaterial(a.materials.bar, b.materials.bar, t);
        r.materials.control = LerpMaterial(a.materials.control, b.materials.control, t);
        r.materials.popover = LerpMaterial(a.materials.popover, b.materials.popover, t);
        r.materials.clear = LerpMaterial(a.materials.clear, b.materials.clear, t);
        const float* ma = reinterpret_cast<const float*>(&a.metrics);
        const float* mb = reinterpret_cast<const float*>(&b.metrics);
        float* mr = reinterpret_cast<float*>(&r.metrics);
        for (std::size_t i = 0; i < sizeof(Metrics) / sizeof(float); ++i)
            mr[i] = Lerp(ma[i], mb[i], t);
        return r;
    }

    void ThemeAnimator::Set(const Theme& target, bool animate)
    {
        // a transition that is still running starts the next one from where it is
        from = animate ? current : target;
        to = target;
        duration = std::max(0.01f, target.motion.themeTransition);
        t = animate ? 0.0f : 1.0f;
        if (!animate)
            current = target;
    }

    void ThemeAnimator::Step(float dt)
    {
        if (t >= 1.0f)
        {
            current = to;
            return;
        }
        t = std::min(1.0f, t + dt / duration);
        current = LerpTheme(from, to, ease::InOutCubic(t));
    }
}
