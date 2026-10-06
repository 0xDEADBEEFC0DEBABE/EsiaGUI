// Esia UI - the Ui (fonts, frame, theme), per-widget styles, keyed animation and interaction.
#include "ui_internal.hpp"
#include "esia/text/system_fonts.hpp"
#include <algorithm>

namespace esia::ui
{
    namespace
    {
        thread_local Ui* g_current = nullptr;

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

    // ============================================================= density
    namespace
    {
        // Regular: the design's sizes (iOS's). Compact: the controls 6 - 10 units shorter (WinUI: text fields
        // 32 -> 24, list items 40 -> 32), their details 0.8 of theirs; every text as it is.
        constexpr detail::ControlSizes kRegularSizes{
            .detail = 1, .button = {28, 36, 48}, .buttonPad = {12, 18, 24}, .field = 32, .stepperWidth = 96, .rowControl = 30,
            .textField = 38, .row = 34, .tableRow = 34, .tableHeader = 30, .tableCell = 30, .treeRow = 30,
            .toggleWidth = 50, .toggleHeight = 30, .checkbox = 22, .sliderTrack = 6, .knobWidth = 34, .knobHeight = 22,
            .heldKnobWidth = 46, .heldKnobHeight = 30, .colorBar = 26, .navigationBar = 44, .searchBar = 46, .tabBar = 60,
            .dockTabs = 44, .closeButton = 30, .windowIcon = 26};
        constexpr detail::ControlSizes kCompactSizes{
            .detail = 0.8f, .button = {24, 30, 40}, .buttonPad = {10, 15, 20}, .field = 26,
            .stepperWidth = 84, .rowControl = 26, .textField = 30, .row = 28, .tableRow = 28, .tableHeader = 26, .tableCell = 24, .treeRow = 26,
            .toggleWidth = 40, .toggleHeight = 24, .checkbox = 18, .sliderTrack = 4, .knobWidth = 28, .knobHeight = 18,
            .heldKnobWidth = 38, .heldKnobHeight = 24, .colorBar = 20, .navigationBar = 36, .searchBar = 36, .tabBar = 48,
            .dockTabs = 34, .closeButton = 24, .windowIcon = 20};
    }

    void Ui::Impl::Refresh()
    {
        effective = theme.current;
        effective.metrics = DensityMetrics(theme.current.metrics);
        const float c = std::clamp(theme.current.metrics.compact, 0.0f, 1.0f);
        if (c <= 0.0f)
        {
            sizes = kRegularSizes;   // the design's, exactly
            return;
        }
        // a switch animates: every size on its way between the tables
        constexpr int n = (int)(sizeof(detail::ControlSizes) / sizeof(float));
        const float* a = reinterpret_cast<const float*>(&kRegularSizes);
        const float* b = reinterpret_cast<const float*>(&kCompactSizes);
        float* r = reinterpret_cast<float*>(&sizes);
        for (int i = 0; i < n; ++i)
            r[i] = a[i] + (b[i] - a[i]) * c;
        // a smaller control's glass bends what is behind it less (bars keep theirs: they float over the content)
        effective.materials.control.refraction *= sizes.detail;
    }

    // ================================================================== Ui
    Ui::Ui(Context& context, const UiDesc& desc) : impl_(std::make_unique<Impl>())
    {
        Impl& m = *impl_;
        m.ctx = &context;
        m.text = desc.text;
        Theme theme = desc.theme;
        theme.metrics.compact = desc.density == Density::Compact ? 1.0f : 0.0f;
        m.theme.Set(theme, false);
        m.Refresh();
        m.look = desc.glassLook;
        m.flat = desc.flat;
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
        m.Refresh();
        // the core's spacing between items follows the theme (WGT's: the spacing across, 80 % of it down)
        const float spacing = m.effective.metrics.spacing * m.effective.metrics.scale;
        m.ctx->Metrics().itemSpacing = Vec2(spacing, spacing * 0.8f);
        m.animating = m.theme.Animating();
        // containers left open last frame (a missing End) must not leak into this one
        ESIA_ASSERT(m.styles.empty() && m.layouts.empty() && m.containers.empty() && m.sections.empty() && m.rowStyles.empty() && m.navs.empty() &&
                    m.pages.empty() && "a ui container was not ended");
        m.styles.clear();
        m.layouts.clear();
        m.containers.clear();
        m.sections.clear();
        m.sectionScopes.clear();
        m.rowStyles.clear();
        m.navs.clear();
        m.pages.clear();
        m.floats.clear();
        m.edgeFades.clear();
        m.next = ItemStyle();
        m.decorationSerial = 0;
        const std::uint32_t frame = (std::uint32_t)m.ctx->FrameCount();
        if (frame % 64 == 0)
            m.anims.Collect(frame, 600);   // as Context::State keeps its entries (ContextDesc::retainFrames)
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
        m.edgeFades.clear();
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
        Theme t = ThemeWithAccent(theme, m.accentOverride);
        t.metrics.compact = m.theme.to.metrics.compact;   // the density is the Ui's (SetDensity)
        m.theme.Set(t, animate);
        m.Refresh();
    }

    void Ui::SetDensity(Density density, bool animate)
    {
        Impl& m = *impl_;
        Theme t = m.theme.to;
        t.metrics.compact = density == Density::Compact ? 1.0f : 0.0f;
        m.theme.Set(t, animate);
        m.Refresh();
    }

    Density Ui::GetDensity() const { return impl_->theme.to.metrics.compact >= 0.5f ? Density::Compact : Density::Regular; }

    void Ui::SetFlat(bool flat) { impl_->flat = flat; }
    bool Ui::GetFlat() const { return impl_->flat; }

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
        m.Refresh();
    }

    void Ui::SetGlassLook(GlassLook look) { impl_->look = look; }
    GlassLook Ui::GetGlassLook() const { return impl_->look; }
    bool Ui::Animating() const { return impl_->animating; }

    text::FontRef Ui::Font(FontWeight weight, float size) const
    {
        const Impl& m = *impl_;
        // quarter units: sizes that differ by a hair share their glyphs
        return {m.fonts[(int)weight], std::round(size * m.effective.metrics.scale * 4.0f) * 0.25f};
    }

    text::FontRef Ui::Font(TextStyle style) const
    {
        const Typography& t = impl_->effective.type;
        return Font(t.weight[(int)style], t.size[(int)style]);
    }

    text::FontRef Ui::IconFont(float size) const { return {impl_->iconFont, size}; }

    Ui* Current() { return g_current; }

    // ========================================================= animation
    namespace detail
    {
        AnimTable::Entry& AnimTable::Get(Id id, std::uint32_t kind, std::uint32_t frame, bool& created)
        {
            if (slots.empty())
                Rehash(256);
            std::size_t mask = slots.size() - 1;
            for (std::size_t i = Home(id, kind, mask);; i = (i + 1) & mask)
            {
                Entry& e = slots[i];
                if (e.id == id && e.kind == kind)
                {
                    e.used = frame;
                    created = false;
                    return e;
                }
                if (e.kind == kFree)
                    break;
            }
            if ((count + 1) * 2 > slots.size())
            {
                Rehash(slots.size() * 2);
                mask = slots.size() - 1;
            }
            std::size_t i = Home(id, kind, mask);
            while (slots[i].kind != kFree)
                i = (i + 1) & mask;
            Entry& e = slots[i];
            e = Entry();
            e.id = id;
            e.kind = kind;
            e.used = frame;
            ++count;
            created = true;
            return e;
        }

        void AnimTable::Rehash(std::size_t size)
        {
            std::vector<Entry> old;
            old.swap(slots);
            slots.assign(size, Entry());
            count = 0;
            const std::size_t mask = size - 1;
            for (const Entry& e : old)
                if (e.kind != kFree)
                {
                    std::size_t i = Home(e.id, e.kind, mask);
                    while (slots[i].kind != kFree)
                        i = (i + 1) & mask;
                    slots[i] = e;
                    ++count;
                }
        }

        void AnimTable::Collect(std::uint32_t frame, std::uint32_t retain)
        {
            bool stale = false;
            for (Entry& e : slots)
                if (e.kind != kFree && frame - e.used > retain)
                {
                    e.kind = kFree;
                    stale = true;
                }
            if (stale)
                Rehash(slots.size());   // the probe chains closed again
        }
    }

    namespace
    {
        void StepMovingSpring(float& value, float& velocity, float target, const Spring& spring, float dt, bool& animating)
        {
            SpringState st;
            st.value = value;
            st.velocity = velocity;
            st.Step(target, spring, dt);
            value = st.value;
            velocity = st.velocity;
            if (value != target || velocity != 0.0f)
                animating = true;
        }

        // one step of a spring held as two floats (an AnimTable entry), and the Ui's `animating` while it moves; at
        // rest (most springs, most frames) nothing but the test
        inline void StepSpring(float& value, float& velocity, float target, const Spring& spring, float dt, bool& animating)
        {
            if (value != target || velocity != 0.0f)
                StepMovingSpring(value, velocity, target, spring, dt, animating);
        }
    }

    namespace detail
    {
        float ControlSpring(const InteractState& it, int slot, float target, const Spring& s, float& velocity)
        {
            Ui::Impl& m = M();
            float& v = it.anim->more[slot];
            float& vel = it.anim->more[slot + 1];
            if (it.animNew)
            {
                v = target;
                vel = 0.0f;
            }
            else if (it.animStep)
                StepSpring(v, vel, target, s, m.dt, m.animating);
            else if (v != target || vel != 0.0f)
                m.animating = true;
            velocity = vel;
            return v;
        }

        float ControlSpring(const InteractState& it, int slot, float target, const Spring& s)
        {
            float velocity;
            return ControlSpring(it, slot, target, s, velocity);
        }

        float ControlPulse(const InteractState& it, int slot, bool held, bool activated, float bloomSeconds)
        {
            // LiquidPulse's timer (more[slot]) and lens spring (more[slot + 1], more[slot + 2])
            Ui::Impl& m = M();
            float& t = it.anim->more[slot];
            float& lens = it.anim->more[slot + 1];
            float& lensVel = it.anim->more[slot + 2];
            if (it.animNew)
                t = 1.0f;   // no bloom until an activation
            if (activated)
                t = 0.0f;
            else if (t < 1.0f && it.animStep)
                t = std::min(1.0f, t + m.dt / std::max(bloomSeconds, 1e-3f));
            if (t < 1.0f)
                m.animating = true;
            static constexpr Spring kLens{0.34f, 0.62f};
            const float target = held || t < 1.0f ? 1.0f : 0.0f;
            if (it.animNew)
            {
                lens = target;
                lensVel = 0.0f;
            }
            else if (it.animStep)
                StepSpring(lens, lensVel, target, kLens, m.dt, m.animating);
            else if (lens != target || lensVel != 0.0f)
                m.animating = true;
            return Clamp(lens, 0.0f, 1.15f);
        }

        // Anim's entry, stepped
        AnimTable::Entry& SpringOf(Id id, float target, const Spring& spring, float initial)
        {
            Ui::Impl& m = M();
            const std::uint32_t frame = (std::uint32_t)m.ctx->FrameCount();
            bool created;
            AnimTable::Entry& a = m.anims.Get(id, AnimTable::kSpring, frame, created);
            if (created)
                a.value = std::isnan(initial) ? target : initial;   // stepped 0: it steps this frame already
            if (a.stepped != frame)
            {
                a.stepped = frame;
                StepSpring(a.value, a.velocity, target, spring, m.dt, m.animating);
            }
            else if (a.value != target || a.velocity != 0.0f)
                m.animating = true;
            return a;
        }

        float AnimWithVelocity(Id id, float target, const Spring& s, float& velocity)
        {
            const AnimTable::Entry& a = SpringOf(id, target, s, NAN);
            velocity = a.velocity;
            return a.value;
        }
    }

    float Anim(Id id, float target, const Spring& spring, float initial) { return detail::SpringOf(id, target, spring, initial).value; }

    float AnimVelocity(Id id)
    {
        Ui::Impl& m = detail::M();
        bool created;
        return m.anims.Get(id, detail::AnimTable::kSpring, (std::uint32_t)m.ctx->FrameCount(), created).velocity;
    }

    void AnimSet(Id id, float value, float velocity)
    {
        Ui::Impl& m = detail::M();
        bool created;
        detail::AnimTable::Entry& a = m.anims.Get(id, detail::AnimTable::kSpring, (std::uint32_t)m.ctx->FrameCount(), created);
        a.value = value;
        a.velocity = velocity;
        a.stepped = 0;   // the next Anim this frame steps from here
    }

    void AnimKick(Id id, float velocity)
    {
        Ui::Impl& m = detail::M();
        bool created;
        detail::AnimTable::Entry& a = m.anims.Get(id, detail::AnimTable::kSpring, (std::uint32_t)m.ctx->FrameCount(), created);
        a.velocity += velocity;
        m.animating = true;
    }

    float Timer(Id id, float duration, bool restart)
    {
        Ui::Impl& m = detail::M();
        const std::uint32_t frame = (std::uint32_t)m.ctx->FrameCount();
        bool created;
        detail::AnimTable::Entry& t = m.anims.Get(id, detail::AnimTable::kTimer, frame, created);
        if (created)
            t.value = 1.0f;   // done: a timer runs from its first restart
        if (restart)
            t.value = 0.0f;
        else if (t.value < 1.0f && t.stepped != frame)
            t.value = std::min(1.0f, t.value + m.dt / std::max(duration, 1e-3f));
        t.stepped = frame;
        if (t.value < 1.0f)
            m.animating = true;
        return t.value;
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

    GlassLook CurrentGlassLook() { return detail::Look(); }

    GlassMaterial LookMaterial(const GlassMaterial& themed) { return detail::StyledMaterial(themed); }

    // ========================================================= custom widgets
    Painter GetPainter()
    {
        Ui::Impl& m = detail::M();
        const Theme& t = m.effective;
        PainterEnv env;
        env.metricsScale = t.metrics.scale;
        env.cornerSmoothing = t.metrics.cornerSmoothing;
        env.pixelScale = m.ctx->Scale();
        env.alpha = detail::StyleAlpha();
        env.text = m.text;
        env.glow = detail::ItemMapGlow();
        env.flat = m.flat;
        env.flatSurface = t.colors.secondaryBackground;
        const ItemStyle& style = detail::ResolvedStyle();
        if (style.Has(ItemStyle::kTextOutline))
            env.textOutline = {style.textOutline.width * t.metrics.scale, style.textOutline.color};
        return Painter(m.ctx->WindowDrawList(), env);
    }

    float S(float value) { return detail::Sc(value); }

    IdScope::IdScope(std::string_view key) { detail::Ctx().PushId(key); }
    IdScope::IdScope(std::int64_t key) { detail::Ctx().PushId(key); }
    IdScope::IdScope(const void* key) { detail::Ctx().PushId(key); }
    IdScope::~IdScope() { detail::Ctx().PopId(); }

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
        const detail::InteractState s = detail::InteractImpl(id, rect, flags);
        Interaction it;
        it.id = s.id;
        it.rect = s.rect;
        it.visible = s.visible;
        it.hovered = s.hovered;
        it.held = s.held;
        it.pressed = s.pressed;
        it.hover = s.hover;
        it.press = s.press;
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

        InteractState InteractImpl(Id id, const Rect& r, std::uint32_t flags)
        {
            Context& c = Ctx();
            InteractState it;
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
            // hover and press: two springs of one entry, which also holds the control's own (ControlSpring)
            Ui::Impl& m = M();
            const std::uint32_t frame = (std::uint32_t)m.ctx->FrameCount();
            bool created;
            AnimTable::Entry& e = m.anims.Get(id, AnimTable::kPair, frame, created);
            const float hoverTarget = it.hovered ? 1.0f : 0.0f, pressTarget = it.held ? 1.0f : 0.0f;
            it.anim = &e;
            it.animNew = created;
            it.animStep = !created && e.stepped != frame;
            if (created)
            {
                e.value = hoverTarget;
                e.value2 = pressTarget;
            }
            else if (it.animStep)
            {
                const Spring& s = m.effective.motion.fast;
                StepSpring(e.value, e.velocity, hoverTarget, s, m.dt, m.animating);
                StepSpring(e.value2, e.velocity2, pressTarget, s, m.dt, m.animating);
            }
            e.stepped = frame;
            it.hover = e.value;
            it.press = e.value2;
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

        ScopedUnclip::ScopedUnclip(const Rect& r, float extent) : pushed_(!M().flat)
        {
            if (pushed_)
                Ctx().PushClipRect(r.Expanded(extent), false);
        }
        ScopedUnclip::~ScopedUnclip()
        {
            if (pushed_)
                Ctx().PopClipRect();
        }

        // ---- text
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
                if (f & ItemStyle::kTextOutline) into.textOutline = s.textOutline;
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

        void DrawPill(Painter& p, const Rect& r, const Style& s)
        {
            if (HasItemRadius())
            {
                Style c = s;
                p.Rect(r, c.Radius(std::min(ItemRadius(0.0f), std::min(r.Width(), r.Height()) * 0.5f)));
            }
            else
                p.Capsule(r, s);
        }

        GlassMaterial LensMaterial()
        {
            GlassMaterial m = T().materials.clear;
            m.blur = 0.0f;
            m.refraction = 10.0f * Sizes().detail;
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
            // the bloom's timer and the lens's spring in one entry
            Ui::Impl& m = M();
            const std::uint32_t frame = (std::uint32_t)m.ctx->FrameCount();
            bool created;
            AnimTable::Entry& e = m.anims.Get(Salt(id, 0xB100), AnimTable::kPulse, frame, created);
            if (created)
                e.value = 1.0f;   // no bloom until an activation
            const bool step = !created && e.stepped != frame;
            if (activated)
                e.value = 0.0f;
            else if (e.value < 1.0f && step)
                e.value = std::min(1.0f, e.value + m.dt / std::max(bloomSeconds, 1e-3f));
            if (e.value < 1.0f)
                m.animating = true;
            const bool on = held || e.value < 1.0f;
            static constexpr Spring kLens{0.34f, 0.62f};
            const float target = on ? 1.0f : 0.0f;
            if (created)
                e.value2 = target;
            else if (step)
                StepSpring(e.value2, e.velocity2, target, kLens, m.dt, m.animating);
            else if (e.value2 != target || e.velocity2 != 0.0f)
                m.animating = true;
            e.stepped = frame;
            return Clamp(e.value2, 0.0f, 1.15f);
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
