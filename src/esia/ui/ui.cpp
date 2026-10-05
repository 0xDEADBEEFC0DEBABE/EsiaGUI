// Esia UI - the Ui (fonts, frame, theme), per-widget styles, keyed animation and interaction.
#include "ui_internal.hpp"
#include "esia/text/system_fonts.hpp"
#include <algorithm>

namespace esia::ui
{
    namespace
    {
        thread_local Ui* g_current = nullptr;

        struct FloatAnim
        {
            SpringState s;
            std::uint64_t lastFrame = 0;
            bool init = false;
        };
        struct TimerAnim
        {
            float t = 1.0f;
            std::uint64_t lastFrame = 0;
        };

        int WeightOf(FontWeight w)
        {
            switch (w)
            {
            case FontWeight::Semibold: return 600;
            case FontWeight::Bold: return 700;
            default: return 400;
            }
        }

        // the platform's monospace font: the first of these that is installed
        std::optional<text::SystemFont> FindMonospace()
        {
            for (std::string_view family : {"Cascadia Mono", "Consolas", "SF Mono", "Menlo", "DejaVu Sans Mono", "Liberation Mono", "Noto Sans Mono"})
                if (std::optional<text::SystemFont> f = text::FindSystemFont(family))
                    return f;
            return std::nullopt;
        }
    }

    // ================================================================== Ui
    Ui::Ui(Context& context, const UiDesc& desc) : impl_(std::make_unique<Impl>())
    {
        Impl& m = *impl_;
        m.ctx = &context;
        m.text = desc.text;
        m.theme.Set(desc.theme, false);
        m.look = desc.glassLook;
        m.islandEnabled = desc.island;
        if (!m.text)
            return;
        for (int w = 0; w < (int)FontWeight::Count; ++w)
        {
            text::FontId id = 0;
            if (!desc.fontFiles[w].empty())
                id = m.text->AddFontFile(desc.fontFiles[w].c_str());
            else
            {
                const std::optional<text::SystemFont> f =
                    (FontWeight)w == FontWeight::Mono ? FindMonospace() : text::FindSystemFont(text::kSystemUiFamily, WeightOf((FontWeight)w));
                if (f)
                    id = text::AddSystemFont(*m.text, *f);
            }
            // a weight that is missing draws with the regular one
            m.fonts[w] = id != 0 ? id : m.fonts[(int)FontWeight::Regular];
        }
        if (desc.fallbackChain)
            text::AddFallbackFonts(*m.text, text::FindDefaultFallbackFonts());
        if (!desc.iconFontFile.empty())
            m.iconFont = m.text->AddFontFile(desc.iconFontFile.c_str());
        else
            for (std::string_view family : {"Segoe Fluent Icons", "Segoe MDL2 Assets"})
                if (std::optional<text::SystemFont> f = text::FindSystemFont(family))
                {
                    m.iconFont = text::AddSystemFont(*m.text, *f);
                    break;
                }
    }

    Ui::~Ui()
    {
        if (g_current == this)
        {
            g_current = nullptr;
            detail::g_impl = nullptr;
        }
    }

    void Ui::NewFrame()
    {
        Impl& m = *impl_;
        const InputState& in = m.ctx->Input();
        m.time = in.Time();
        m.dt = std::clamp(in.DeltaTime(), 0.0f, 0.1f);   // a hitch must not fling springs
        m.theme.Step(m.dt);
        // the core's spacing between items follows the theme (WGT's: the spacing across, 80 % of it down)
        const float spacing = m.theme.current.metrics.spacing * m.theme.current.metrics.scale;
        m.ctx->Metrics().itemSpacing = Vec2(spacing, spacing * 0.8f);
        m.animating = m.theme.Animating();
        // containers left open last frame (a missing End) must not leak into this one
        ESIA_ASSERT(m.styles.empty() && m.layouts.empty() && m.containers.empty() && m.sections.empty() && m.rowStyles.empty() && m.navs.empty() &&
                    m.pages.empty() && "a ui container was not ended");
        m.styles.clear();
        m.layouts.clear();
        m.containers.clear();
        m.sections.clear();
        m.rowStyles.clear();
        m.navs.clear();
        m.pages.clear();
        m.floats.clear();
        m.next = ItemStyle();
        m.decorationSerial = 0;
        if (m.text)
            m.text->NewFrame({m.ctx->FramebufferScale().x});
        m.inFrame = true;
        g_current = this;
        detail::g_impl = impl_.get();
    }

    void Ui::EndFrame()
    {
        Impl& m = *impl_;
        if (m.islandEnabled && g_current == this)
            detail::IslandFrame();
        // floating bars to the end of their window's draw list, in the order they were submitted
        for (std::size_t i = 0; i < m.floats.size(); ++i)
        {
            const Impl::FloatBlock b = m.floats[i];
            const std::size_t n = b.to - b.from;
            if (n == 0)
                continue;
            b.list->MoveCommands(b.to, b.from);   // what follows the block goes before it
            for (std::size_t k = i + 1; k < m.floats.size(); ++k)
                if (m.floats[k].list == b.list && m.floats[k].from >= b.to)
                {
                    m.floats[k].from -= n;
                    m.floats[k].to -= n;
                }
        }
        m.floats.clear();
        m.inFrame = false;
        if (g_current == this)
        {
            g_current = nullptr;
            detail::g_impl = nullptr;
        }
    }

    Context& Ui::GetContext() const { return *impl_->ctx; }
    text::TextSystem* Ui::Text() const { return impl_->text; }
    const Theme& Ui::GetTheme() const { return impl_->theme.current; }

    void Ui::SetTheme(const Theme& theme, bool animate)
    {
        Impl& m = *impl_;
        m.theme.Set(ThemeWithAccent(theme, m.accentOverride), animate);
    }

    void Ui::SetDarkMode(bool dark, bool animate)
    {
        // the built-in colors and materials of that mode; the metrics (the scale), type and motion stay as they were set
        Theme t = dark ? ThemeDark() : ThemeLight();
        const Theme& now = impl_->theme.to;
        t.metrics = now.metrics;
        t.type = now.type;
        t.motion = now.motion;
        SetTheme(t, animate);
    }

    void Ui::SetAccent(Color accent)
    {
        Impl& m = *impl_;
        m.accentOverride = accent;
        Theme t = m.theme.to;
        t.colors.accent = accent.a > 0.0f ? accent : (t.dark ? ThemeDark() : ThemeLight()).colors.accent;
        m.theme.Set(t, true);
    }

    void Ui::SetGlassLook(GlassLook look) { impl_->look = look; }
    GlassLook Ui::GetGlassLook() const { return impl_->look; }
    bool Ui::Animating() const { return impl_->animating; }

    text::FontRef Ui::Font(FontWeight weight, float size) const
    {
        const Impl& m = *impl_;
        // quarter units: sizes that differ by a hair share their glyphs
        return {m.fonts[(int)weight], std::round(size * m.theme.current.metrics.scale * 4.0f) * 0.25f};
    }

    text::FontRef Ui::Font(TextStyle style) const
    {
        const Typography& t = impl_->theme.current.type;
        return Font(t.weight[(int)style], t.size[(int)style]);
    }

    text::FontRef Ui::IconFont(float size) const { return {impl_->iconFont, size}; }

    Ui* Current() { return g_current; }

    // ========================================================= animation
    float Anim(Id id, float target, const Spring& spring, float initial)
    {
        Ui::Impl& m = detail::M();
        FloatAnim& a = m.ctx->State<FloatAnim>(id);
        if (!a.init)
        {
            a.init = true;
            a.s.value = std::isnan(initial) ? target : initial;
        }
        const std::uint64_t frame = m.ctx->FrameCount();
        if (a.lastFrame != frame)
        {
            a.lastFrame = frame;
            a.s.Step(target, spring, m.dt);
        }
        if (a.s.value != target || a.s.velocity != 0.0f)
            m.animating = true;
        return a.s.value;
    }

    float AnimVelocity(Id id) { return detail::Ctx().State<FloatAnim>(id).s.velocity; }

    void AnimSet(Id id, float value, float velocity)
    {
        FloatAnim& a = detail::Ctx().State<FloatAnim>(id);
        a.init = true;
        a.s.value = value;
        a.s.velocity = velocity;
        a.lastFrame = 0;   // the next Anim this frame steps from here
    }

    void AnimKick(Id id, float velocity)
    {
        FloatAnim& a = detail::Ctx().State<FloatAnim>(id);
        a.s.velocity += velocity;
        detail::M().animating = true;
    }

    float Timer(Id id, float duration, bool restart)
    {
        Ui::Impl& m = detail::M();
        TimerAnim& t = m.ctx->State<TimerAnim>(id);
        const std::uint64_t frame = m.ctx->FrameCount();
        if (restart)
            t.t = 0.0f;
        else if (t.t < 1.0f && t.lastFrame != frame)
            t.t = std::min(1.0f, t.t + m.dt / std::max(duration, 1e-3f));
        t.lastFrame = frame;
        if (t.t < 1.0f)
            m.animating = true;
        return t.t;
    }

    double Time() { return detail::M().time; }
    float DeltaTime() { return detail::M().dt; }

    // ========================================================= style API
    ItemStyle& Next() { return detail::M().next; }
    void SetNextItemStyle(const ItemStyle& style) { detail::M().next = style; }
    StyleScope::StyleScope(const ItemStyle& style) { detail::PushStyle(style); }
    StyleScope::~StyleScope() { detail::PopStyle(); }
    ItemStyle CurrentItemStyle() { return detail::ResolvedStyle(); }
    Color AccentColor() { return detail::Accent(); }

    GlassLook CurrentGlassLook()
    {
        const ItemStyle& s = detail::ResolvedStyle();
        return s.Has(ItemStyle::kLook) ? s.look : detail::M().look;
    }

    GlassMaterial LookMaterial(const GlassMaterial& themed) { return detail::StyledMaterial(themed); }

    // ========================================================= custom widgets
    Painter GetPainter()
    {
        Ui::Impl& m = detail::M();
        const Theme& t = m.theme.current;
        PainterEnv env;
        env.metricsScale = t.metrics.scale;
        env.cornerSmoothing = t.metrics.cornerSmoothing;
        env.pixelScale = m.ctx->Scale();
        env.alpha = detail::StyleAlpha();
        env.text = m.text;
        env.glow = detail::ItemMapGlow();
        return Painter(m.ctx->WindowDrawList(), env);
    }

    float S(float value) { return detail::Sc(value); }

    float AvailableWidth()
    {
        detail::MarkFill();
        Context& c = detail::Ctx();
        const float avail = c.ContentRegionAvail().x;
        // What the items after it on its line (SameLine) took last frame stays theirs: a field filling the width does
        // not push the button after it out of view.
        const float room = c.LineRoom();
        return room > 0.0f ? std::max(avail - room, std::min(avail, detail::Sc(40))) : avail;
    }

    Interaction InteractRect(Id id, const Rect& rect, std::uint32_t flags)
    {
        Ui::Impl& m = detail::M();
        Interaction it = detail::InteractImpl(id, rect, flags);
        it.style = m.next;
        m.next = ItemStyle();
        return it;
    }

    Interaction Interact(std::string_view id, Vec2 size, std::uint32_t flags)
    {
        Context& c = detail::Ctx();
        const Vec2 pos = c.CursorPos();
        c.ItemSize(size);
        return InteractRect(c.GetId(id), Rect::FromSize(pos, size), flags);
    }

    SizeClass GetSizeClass(float width)
    {
        const float w = (width >= 0.0f ? width : detail::Ctx().ContentRegionAvail().x) / std::max(detail::T().metrics.scale, 1e-3f);
        return w < 420.0f ? SizeClass::Compact : w < 760.0f ? SizeClass::Regular : SizeClass::Expanded;
    }

    // ========================================================= internals
    namespace detail
    {
        thread_local Ui::Impl* g_impl = nullptr;

        Interaction InteractImpl(Id id, const Rect& r, std::uint32_t flags)
        {
            Context& c = Ctx();
            Interaction it;
            it.id = id;
            it.rect = r;
            it.visible = c.ItemAdd(id, r, (flags & InteractFlags_Disabled) ? ItemFlags_Disabled : ItemFlags_None);
            if (!it.visible)
                return it;
            std::uint32_t bf = ButtonFlags_None;
            if (flags & InteractFlags_PressOnClick)
                bf |= ButtonFlags_PressOnClick;
            if (flags & InteractFlags_Repeat)
                bf |= ButtonFlags_Repeat;
            const ButtonResult b = c.ButtonBehavior(id, r, bf);
            it.hovered = b.hovered;
            it.held = b.held;
            it.pressed = b.pressed;
            it.hover = Anim(id, 0xA1, it.hovered ? 1.0f : 0.0f, SpringFast());
            it.press = Anim(id, 0xA2, it.held ? 1.0f : 0.0f, SpringFast());
            return it;
        }

        void MoveCommands(DrawList& dl, std::size_t from, std::size_t to)
        {
            const std::size_t n = dl.Commands().size() - from;
            dl.MoveCommands(from, to);
            for (Ui::Impl::FloatBlock& b : M().floats)
                if (b.list == &dl && b.from >= to && b.to <= from)
                {
                    b.from += n;
                    b.to += n;
                }
        }

        ScopedUnclip::ScopedUnclip(const Rect& r, float extent) { Ctx().PushClipRect(r.Expanded(extent), false); }
        ScopedUnclip::~ScopedUnclip() { Ctx().PopClipRect(); }

        // ---- text
        text::FontRef Font(FontWeight w, float size) { return g_current->Font(w, size); }
        text::FontRef Font(TextStyle s) { return g_current->Font(s); }

        Vec2 MeasureText(text::FontRef f, std::string_view text, float wrapWidth)
        {
            text::TextSystem* ts = M().text;
            return ts && !text.empty() ? ts->Measure(f, text, wrapWidth).size : Vec2(0, 0);
        }

        void DrawIcon(Painter& p, Vec2 center, Icon icon, float size, Color color)
        {
            const Ui::Impl& m = M();
            if (icon != 0 && m.iconFont != 0)
                p.Icon(center, {m.iconFont, size}, (char32_t)icon, color);
        }

        std::string_view VisibleLabel(std::string_view label)
        {
            const std::size_t hide = label.find("##");
            return hide == std::string_view::npos ? label : label.substr(0, hide);
        }

        // ---- styles (WGT's look.cpp)
        namespace
        {
            // `s` over `into`, field by field
            void Merge(ItemStyle& into, const ItemStyle& s)
            {
                const std::uint32_t f = s.set;
                if (f & ItemStyle::kLook) into.look = s.look;
                if (f & ItemStyle::kBlur) into.glass.blur = s.glass.blur;
                if (f & ItemStyle::kRefraction) into.glass.refraction = s.glass.refraction;
                if (f & ItemStyle::kBezel) into.glass.bezel = s.glass.bezel;
                if (f & ItemStyle::kDispersion) into.glass.dispersion = s.glass.dispersion;
                if (f & ItemStyle::kSaturation) into.glass.saturation = s.glass.saturation;
                if (f & ItemStyle::kBrightness) into.glass.brightness = s.glass.brightness;
                if (f & ItemStyle::kSpecular) into.glass.specular = s.glass.specular;
                if (f & ItemStyle::kLegibility) into.glass.legibility = s.glass.legibility;
                if (f & ItemStyle::kMagnify) into.glass.magnify = s.glass.magnify;
                if (f & ItemStyle::kGlassTint) into.glass.tint = s.glass.tint;
                if (f & ItemStyle::kRim) into.glass.rim = s.glass.rim;
                if (f & ItemStyle::kTint) into.tint = s.tint;
                if (f & ItemStyle::kFill) into.fill = s.fill;
                if (f & ItemStyle::kRadius) into.radius = s.radius;
                if (f & ItemStyle::kOpacity) into.opacity = s.opacity;
                if (f & ItemStyle::kLabel) into.label = s.label;
                if (f & ItemStyle::kSelectedFill) into.selectedFill = s.selectedFill;
                if (f & ItemStyle::kMovingFill) into.movingFill = s.movingFill;
                if (f & ItemStyle::kSelectedLabel) into.selectedLabel = s.selectedLabel;
                into.set |= f;
            }

            // the glass fields a style sets, over a material
            GlassMaterial Override(GlassMaterial m, const ItemStyle& s)
            {
                if (!(s.set & ItemStyle::kGlassFields))
                    return m;
                ItemStyle base;
                base.glass = m;
                Merge(base, s);
                base.glass.lightAngle = m.lightAngle;
                base.glass.noise = m.noise;
                return base.glass;
            }

            GlassLook LookOf(const ItemStyle& s) { return s.Has(ItemStyle::kLook) ? s.look : M().look; }

            // the surface color joins the glass's tint (which the shader lays in layers) instead of a flat coat on top
            GlassMaterial FoldSurface(GlassMaterial m, Color surface)
            {
                if (surface.a <= 0.0f)
                    return m;
                const float a = 1.0f - (1.0f - m.tint.a) * (1.0f - surface.a);
                const float k = a > 1e-4f ? 1.0f / a : 0.0f;
                m.tint = Color((surface.r * surface.a + m.tint.r * m.tint.a * (1.0f - surface.a)) * k,
                               (surface.g * surface.a + m.tint.g * m.tint.a * (1.0f - surface.a)) * k,
                               (surface.b * surface.a + m.tint.b * m.tint.a * (1.0f - surface.a)) * k, a);
                return m;
            }
        }

        void MergeStyle(ItemStyle& into, const ItemStyle& s) { Merge(into, s); }

        const ItemStyle& ResolvedStyle()
        {
            static const ItemStyle kNone;
            const auto& st = M().styles;
            return st.empty() ? kNone : st.back().resolved;   // merged once, when the scope was pushed
        }

        void PushStyle(const ItemStyle& s)
        {
            Ui::Impl& m = M();
            Ui::Impl::StyleEntry e;
            if (!m.styles.empty())
                e = m.styles.back();
            Merge(e.resolved, s);
            if (s.Has(ItemStyle::kOpacity))
                e.alpha *= Saturate(s.opacity);
            m.styles.push_back(e);
        }

        void PopStyle()
        {
            Ui::Impl& m = M();
            ESIA_ASSERT(!m.styles.empty());
            if (!m.styles.empty())
                m.styles.pop_back();
        }

        bool TakeNextStyle()
        {
            Ui::Impl& m = M();
            if (m.next.set == 0)
                return false;
            const ItemStyle s = m.next;
            m.next = ItemStyle();
            PushStyle(s);
            return true;
        }

        float StyleAlpha()
        {
            const auto& st = M().styles;
            return st.empty() ? 1.0f : st.back().alpha;
        }

        GlassMaterial ApplyLook(GlassLook look, GlassMaterial m)
        {
            switch (look)
            {
            case GlassLook::Clear:
                // nothing but the glass itself: its lens, rim light and hairline
                m.blur = 0.0f;
                m.tint.a = 0.0f;
                m.legibility = 0.0f;
                m.saturation = std::min(m.saturation, 1.12f);
                m.brightness = std::min(m.brightness, 0.02f);
                break;
            case GlassLook::Frosted:
                // iOS frost: a soft blur and a light, layered veil (the shader lays it heavier on the bevel)
                m.blur = std::max(m.blur, 14.0f);
                m.tint.a = Clamp(m.tint.a, 0.08f, 0.14f);
                m.saturation = std::max(m.saturation, 1.4f);
                m.legibility = Clamp(m.legibility, 0.2f, 0.35f);   // a gentle exposure: the backdrop's color shows
                break;
            default:
                break;
            }
            return m;
        }

        GlassMaterial StyledMaterial(const GlassMaterial& themed)
        {
            const ItemStyle& s = ResolvedStyle();
            return Override(ApplyLook(LookOf(s), themed), s);
        }

        GlassMaterial GlassFieldsOver(const GlassMaterial& m) { return Override(m, ResolvedStyle()); }

        GlassMaterial SurfaceMaterial(const GlassMaterial& themed, Color surface)
        {
            const ItemStyle& s = ResolvedStyle();
            const GlassLook look = LookOf(s);
            GlassMaterial m = ApplyLook(look, themed);
            if (look == GlassLook::Theme || s.Has(ItemStyle::kFill))
                m = FoldSurface(m, s.Has(ItemStyle::kFill) ? s.fill : surface);   // clear / frosted: no surface of their own
            return Override(m, s);
        }

        bool LookClear() { return CurrentGlassLook() == GlassLook::Clear; }

        bool GlassSurface()
        {
            const ItemStyle& s = ResolvedStyle();
            if (s.Has(ItemStyle::kLook))
                return s.look != GlassLook::Theme;
            if (s.set & ItemStyle::kGlassFields)
                return true;
            return M().look == GlassLook::Clear;
        }

        Color Accent()
        {
            const ItemStyle& s = ResolvedStyle();
            return s.Has(ItemStyle::kTint) ? s.tint : C().accent;
        }

        Color AccentOr(Color themed)
        {
            const ItemStyle& s = ResolvedStyle();
            return s.Has(ItemStyle::kTint) ? s.tint : themed;
        }

        Color FillOr(Color themed)
        {
            const ItemStyle& s = ResolvedStyle();
            return s.Has(ItemStyle::kFill) ? s.fill : themed;
        }

        Color LabelOr(Color themed)
        {
            const ItemStyle& s = ResolvedStyle();
            return s.Has(ItemStyle::kLabel) ? s.label : themed;
        }

        bool HasItemRadius() { return ResolvedStyle().Has(ItemStyle::kRadius); }

        float ItemRadius(float themed)
        {
            const ItemStyle& s = ResolvedStyle();
            return s.Has(ItemStyle::kRadius) ? Sc(s.radius) : themed;
        }

        GlassMaterial SurfaceGlass(Color fill)
        {
            // clear: nothing but the glass; frosted / custom glass: the surface color tints it, see-through
            const GlassMaterial m = StyledMaterial(T().materials.control);
            if (LookClear())
                return m;
            return FoldSurface(m, fill.Fade(std::min(1.0f, 0.55f / std::max(fill.a, 1e-3f))));
        }

        Style& SurfaceFill(Style& s, Color fill)
        {
            if (!GlassSurface())
                return s.Fill(fill);
            s.Glass(SurfaceGlass(fill));
            if (LookClear() && ResolvedStyle().Has(ItemStyle::kFill))
                s.Fill(fill);   // an explicit fill still shows on clear glass
            return s;
        }

        Style Surface(Color fill)
        {
            Style s;
            SurfaceFill(s, fill);
            return s;
        }

        Color StateFill(Color c) { return LookClear() ? c.Fade(0.55f) : c; }

        void DrawPill(Painter& p, const Rect& r, Style s)
        {
            if (HasItemRadius())
                p.Rect(r, s.Radius(std::min(ItemRadius(0.0f), std::min(r.Width(), r.Height()) * 0.5f)));
            else
                p.Capsule(r, s);
        }

        GlassMaterial LensMaterial()
        {
            GlassMaterial m = T().materials.clear;
            m.blur = 0.0f;
            m.refraction = 10.0f;
            m.bezel = 40.0f;   // one rounded rod / dome
            m.dispersion = 1.0f;
            m.magnify = 0.28f;
            m.saturation = 1.1f;
            m.brightness = 0.0f;
            m.specular = 1.0f;
            m.legibility = 0.0f;
            m.tint = Color(1, 1, 1, 0.0f);
            m.rim = T().dark ? Color(0, 0, 0, 0.55f) : Color(0, 0, 0, 0.20f);
            return m;
        }

        float LiquidPulse(Id id, bool held, bool activated, float bloomSeconds)
        {
            const float since = Timer(Salt(id, 0xB100), bloomSeconds, activated);
            const bool on = held || since < 1.0f;
            static constexpr Spring kLens{0.34f, 0.62f};
            return Clamp(ui::Anim(Salt(id, 0xB101), on ? 1.0f : 0.0f, kLens), 0.0f, 1.15f);
        }

        Rect ContainLiquid(Rect pill, const Rect& track, float slack)
        {
            const float overR = std::max(0.0f, pill.max.x - (track.max.x + slack));
            const float overL = std::max(0.0f, (track.min.x - slack) - pill.min.x);
            pill.max.x -= overR;
            pill.min.x += overL;
            const float bulge = std::min((overR + overL) * 0.18f, pill.Height() * 0.10f);
            return pill.Expanded(0.0f, bulge);
        }
    }
}
