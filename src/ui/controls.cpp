// WGT UI - controls: buttons, switches, sliders, steppers, segmented controls, fields, menus, progress.
#include "ui/ui_internal.hpp"
#include <cstdarg>

namespace wgt::ui
{
    using namespace detail;

    // ============================================================ interaction
    Interaction detail::InteractImpl(ImGuiID id, const Rect& r, std::uint32_t flags)
    {
        Interaction it;
        it.id = id;
        it.rect = r;
        const ImRect bb(r.min, r.max);
        it.visible = ImGui::ItemAdd(bb, id);
        if (!it.visible)
            return it;
        ImGuiButtonFlags bf = ImGuiButtonFlags_MouseButtonLeft;
        if (flags & InteractFlags_PressOnClick)
            bf |= ImGuiButtonFlags_PressedOnClick;
        if (flags & InteractFlags_AllowOverlap)
            bf |= ImGuiButtonFlags_AllowOverlap;
        if (flags & InteractFlags_Repeat)
            ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
        it.pressed = ImGui::ButtonBehavior(bb, id, &it.hovered, &it.held, bf);
        if (flags & InteractFlags_Repeat)
            ImGui::PopItemFlag();
        it.hover = Anim(id, 0xA1, it.hovered ? 1.0f : 0.0f, SpringFast());
        it.press = Anim(id, 0xA2, it.held ? 1.0f : 0.0f, SpringFast());
        return it;
    }

    // A custom component takes a pending ui::Next() style into Interaction::style (it draws inside a StyleScope).
    static Interaction WithLook(Interaction it)
    {
        Context::Impl& m = Ctx();
        it.style = m.ui.nextStyle;
        m.ui.nextStyle = ItemStyle();
        return it;
    }

    Interaction InteractRect(ImGuiID id, const Rect& rect, std::uint32_t flags) { return WithLook(InteractImpl(id, rect, flags)); }

    Interaction Interact(const char* id, Vec2 size, std::uint32_t flags)
    {
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return WithLook({});
        const ImGuiID gid = w->GetID(id);
        const Vec2 pos = w->DC.CursorPos;
        ImGui::ItemSize(size);
        return WithLook(InteractImpl(gid, Rect::FromSize(pos, size), flags));
    }

    float S(float value) { return Sc(value); }
    float AvailableWidth() { return AvailWidth(); }

    // ================================================================= text
    void detail::TextImpl(FontRef f, Color color, const char* text, const char* end, bool wrap)
    {
        // Laid out and rendered by the WGT text engine (shaping, fallback, crisp physical-pixel glyphs);
        // Dear ImGui only provides the layout cursor. Honors PushTextWrapPos().
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems || !text)
            return;
        float wrapPos = w->DC.TextWrapPos;
        if (wrap && wrapPos < 0.0f)
            wrapPos = 0.0f;
        const Vec2 pos = w->DC.CursorPos;
        if (wrapPos >= 0.0f)
            AvailWidth();   // wrapping text stretches to the width it is offered (auto layout)
        float wrapWidth = wrapPos >= 0.0f ? ImGui::CalcWrapWidthForPos(pos, wrapPos) : 0.0f;
        TextMetrics m = MeasureText(f, text, end, wrapWidth, 0);
        if (wrapWidth <= 0.0f && !InLayoutContainer())
        {
            // Text never runs past its container: wider than what is left of its line (after SameLine items
            // too), it wraps at the container's edge. Auto-layout containers size and place their children
            // themselves (they stack, flow or ellipsize).
            const float room = ImGui::CalcWrapWidthForPos(pos, 0.0f);
            if (room > 0.0f && m.size.x > room + 0.5f)
            {
                wrapWidth = std::max(room, f.size * 4.0f);
                m = MeasureText(f, text, end, wrapWidth, 0);
            }
        }
        const Vec2 size(m.size.x, std::max(m.size.y, f.size));
        ImGui::ItemSize(size, m.baseline > 0.0f ? m.baseline : -1.0f);
        if (!ImGui::ItemAdd(ImRect(pos, pos + size), 0))
            return;
        Painter p(w->DrawList);
        p.Text(pos, f, color, text, end, wrapWidth, 0);
    }

    static void TextV(TextStyle style, Color color, const char* fmt, va_list args)
    {
        const char* begin;
        const char* end;
        ImFormatStringToTempBufferV(&begin, &end, fmt, args);
        TextImpl(GetFont(style), color, begin, end, false);
    }

    void Text(TextStyle style, const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        TextV(style, C().label, fmt, args);
        va_end(args);
    }

    void TextColored(TextStyle style, Color color, const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        TextV(style, color, fmt, args);
        va_end(args);
    }

    void TextSecondary(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        TextV(TextStyle::Subheadline, C().secondaryLabel, fmt, args);
        va_end(args);
    }

    void TextWrapped(TextStyle style, Color color, const char* text) { TextImpl(GetFont(style), color, text, nullptr, true); }
    void LargeTitle(const char* text) { TextImpl(GetFont(TextStyle::LargeTitle), C().label, text, nullptr, false); }
    void Headline(const char* text) { TextImpl(GetFont(TextStyle::Headline), C().label, text, nullptr, false); }

    void Spacer(float height) { ImGui::Dummy(Vec2(0.0f, height < 0.0f ? Sc(T().metrics.spacing) : Sc(height))); }

    void Divider()
    {
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return;
        const float y = w->DC.CursorPos.y + Sc(4);
        Painter p;
        p.HLine(w->DC.CursorPos.x, w->DC.CursorPos.x + AvailWidth(), y, C().separator, std::max(1.0f, Sc(T().metrics.hairline)));
        ImGui::Dummy(Vec2(0, Sc(9)));
    }

    // =============================================================== buttons
    namespace
    {
        struct ButtonMetrics
        {
            float height, padX, font;
        };

        ButtonMetrics MetricsFor(ControlSize s)
        {
            switch (s)
            {
            case ControlSize::Small: return {28, 12, 13};
            case ControlSize::Large: return {48, 24, 17};
            default: return {36, 18, 15};
            }
        }

        struct ButtonColors
        {
            Color bg, fg;
            bool glass = false;
        };

        ButtonColors ColorsFor(const ButtonOptions& o)
        {
            const Palette& c = C();
            const bool dark = T().dark;
            Color tint = o.kind == ButtonKind::Destructive ? (o.tint.a > 0 ? o.tint : AccentOr(c.red)) : Tint(o.tint);
            switch (o.kind)
            {
            case ButtonKind::Filled: return {tint, c.onAccent};
            case ButtonKind::Tinted: return {tint.Fade(dark ? 0.24f : 0.15f), dark ? tint.Lighter(0.15f) : tint};
            case ButtonKind::Gray: return {c.secondaryFill, tint};
            case ButtonKind::Plain: return {Color::Clear(), tint};
            case ButtonKind::Glass: return {Color::Clear(), c.label, true};
            case ButtonKind::GlassProminent: return {tint.Fade(0.78f), c.onAccent, true};
            case ButtonKind::Destructive: return {tint.Fade(dark ? 0.24f : 0.14f), tint};
            }
            return {tint, c.onAccent};
        }

        void DrawButton(const Interaction& it, const ButtonOptions& o, const char* label, const char* labelEnd, Icon icon, bool circle)
        {
            const ButtonMetrics m = MetricsFor(o.size);
            ButtonColors bc = ColorsFor(o);
            const Theme& t = T();
            if (ResolvedStyle().Has(ItemStyle::kFill))
                bc.bg = FillOr(bc.bg);
            // a flat button the style turns into glass: clear glass (labels that sat on a solid color take the
            // label color, legible over anything) or glass tinted by its color
            bool styledGlass = false;
            GlassMaterial surfaceGlass;
            if (GlassSurface() && !bc.glass && (o.kind != ButtonKind::Plain || bc.bg.a > 0.0f))
            {
                surfaceGlass = SurfaceGlass(bc.bg);
                if (LookClear() && o.kind == ButtonKind::Filled)
                    bc.fg = C().label;
                bc.bg = LookClear() && ResolvedStyle().Has(ItemStyle::kFill) ? bc.bg : Color::Clear();   // else its color tints the glass
                bc.glass = styledGlass = true;
            }
            else if (LookClear() && o.kind == ButtonKind::GlassProminent)
            {
                bc.fg = C().label;
                bc.bg = Color::Clear();
            }
            const Rect r = it.rect;
            Painter p;
            // liquid glass "bulges" when pressed, solid buttons compress
            const float scale = bc.glass ? 1.0f + 0.06f * it.press : 1.0f - 0.035f * it.press;
            p.PushScale(r.Center(), scale);

            Style s;
            if (circle || o.capsule)
                s.Radius(ItemRadius(r.Height() * 0.5f));
            else
                s.Radius(ItemRadius(Sc(t.metrics.controlRadius)));
            if (bc.glass)
            {
                GlassMaterial g = styledGlass ? surfaceGlass : LookMaterial(t.materials.control);
                g.brightness += 0.05f * it.hover + 0.06f * it.press;
                g.specular += 0.3f * it.press;
                s.Glass(g).Shadow(C().shadow.Fade(0.8f), Sc(14 + 6 * it.press), Vec2(0, Sc(5)));
            }
            Color bg = bc.bg;
            if (bg.a > 0.0f)
            {
                // hover lightens / press dims (classic), handled in color space
                bg = t.dark ? bg.Lighter(0.10f * it.hover) : bg.Darker(0.05f * it.hover);
                if (!bc.glass)
                    bg = bg.Fade(1.0f - 0.25f * it.press);
                s.Fill(bg);
            }
            else if (o.kind == ButtonKind::Plain && it.hover > 0.001f)
                s.Fill(C().highlight.Fade(it.hover));
            if (o.glow)
                s.Glow(Tint(o.tint), Sc(16), 0.55f + 0.45f * it.hover);
            if (o.kind == ButtonKind::Filled && !bc.glass)
                s.Shadow(Tint(o.tint).Fade(0.35f * (1.0f - it.press)), Sc(12), Vec2(0, Sc(4)));
            if (circle && !HasItemRadius())
                p.Circle(r.Center(), r.Height() * 0.5f, s);
            else
                p.Rect(r, s);

            // content
            const FontRef f = Font(FontWeight::Semibold, m.font);
            Color fg = LabelOr(bc.fg);
            if (o.kind == ButtonKind::Plain)
                fg = fg.Fade(1.0f - 0.4f * it.press);
            const float iconSize = f.size * 1.1f;
            const bool hasText = label && label != labelEnd;
            const Vec2 ts = hasText ? Painter::MeasureText(f, label, labelEnd) : Vec2(0, 0);
            const float gap = (icon && hasText) ? Sc(7) : 0.0f;
            const float iconW = icon ? iconSize : 0.0f;
            const float total = iconW + gap + ts.x;
            float x = r.Center().x - total * 0.5f;
            if (icon)
            {
                p.Icon(Vec2(x + iconW * 0.5f, r.Center().y), icon, iconSize * 0.95f, fg);
                x += iconW + gap;
            }
            if (hasText)
                p.Text(Vec2(std::floor(x + 0.5f), std::floor(r.Center().y - ts.y * 0.5f + 0.5f)), f, fg, label, labelEnd);
            p.PopScale();
        }
    }

    bool Button(const char* label, const ButtonOptions& o)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return false;
        const ButtonMetrics m = MetricsFor(o.size);
        const char* labelEnd = ImGui::FindRenderedTextEnd(label);
        const FontRef f = Font(FontWeight::Semibold, m.font);
        const Vec2 ts = Painter::MeasureText(f, label, labelEnd);
        const float iconW = o.icon ? f.size * 1.1f + (label != labelEnd ? Sc(7) : 0.0f) : 0.0f;
        float width = ts.x + iconW + Sc(m.padX) * 2.0f;
        if (o.width > 0.0f)
            width = Sc(o.width);
        else if (o.width < 0.0f)
            width = AvailWidth();
        const Vec2 size(width, Sc(m.height));
        Interaction it = Interact(label, size, InteractFlags_None);
        if (!it.visible)
            return false;
        DrawButton(it, o, label, labelEnd, o.icon, false);
        return it.pressed;
    }

    bool IconButton(const char* id, Icon icon, const ButtonOptions& o)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return false;
        const ButtonMetrics m = MetricsFor(o.size);
        const float d = Sc(m.height);
        Interaction it = Interact(id, Vec2(d, d), InteractFlags_None);
        if (!it.visible)
            return false;
        DrawButton(it, o, nullptr, nullptr, icon, true);
        return it.pressed;
    }

    // ================================================================ toggle
    Vec2 detail::ToggleSize() { return Vec2(Sc(50), Sc(30)); }

    bool detail::ToggleAt(ImGuiID id, const Rect& r, bool* value)
    {
        Interaction it = InteractImpl(id, r, InteractFlags_None);
        if (!it.visible)
            return false;
        bool changed = false;
        if (it.pressed)
        {
            *value = !*value;
            changed = true;
            ImGui::MarkItemEdited(id);
        }
        const Palette& c = C();

        // --- motion model -------------------------------------------------------------------------
        // position : underdamped spring, the knob overshoots and squashes against the end of the track
        // speed    : the knob stretches along its motion and thins slightly (liquid drop)
        // press    : while held the knob elongates towards where it will travel (iOS)
        // lens     : held or just toggled -> the knob swells into a clear refractive lens (iOS 26)
        // color    : cross-fades on its own critically damped spring (never bounces)
        const ImGuiID posId = Salt(id, 0x10);
        static const Spring kKnob{0.42f, 0.60f};
        const float pos = anim::Float(posId, *value ? 1.0f : 0.0f, &kKnob);
        const float vel = anim::Velocity(posId);
        const float colorT = Saturate(Anim(id, 0x11, *value ? 1.0f : 0.0f, Spring{0.30f, 1.0f}));
        const float lens = LiquidPulse(id, it.held, changed);
        const float press = it.press;

        const float pad = Sc(2);
        const float kd = r.Height() - pad * 2.0f;
        const float travel = r.Width() - pad * 2.0f - kd;
        const float overshoot = pos - Saturate(pos);
        const float squash = std::fabs(overshoot) * travel * 1.4f;
        const float speed = std::min(std::fabs(vel) * travel * 0.045f, kd * 0.55f);
        const float dir = *value ? -1.0f : 1.0f;
        const float elong = Sc(7) * press;

        float w = kd + speed + elong - squash + Sc(12) * lens;
        float h = kd - std::min(speed * 0.14f, kd * 0.12f) + squash * 0.3f + Sc(8) * lens;
        float cx = r.min.x + pad + kd * 0.5f + travel * Saturate(pos) + dir * elong * 0.5f - (overshoot > 0 ? 1.0f : -1.0f) * squash * 0.5f;
        w = std::max(w, kd * 0.7f);
        const Rect knob = Rect::FromCenter(Vec2(cx, r.Center().y), Vec2(w, h));

        Painter p;
        const Color onColor = AccentOr(c.green);
        DrawPill(p, r, Surface(Lerp(FillOr(c.secondaryFill), onColor, colorT)));
        if (GlassSurface() && colorT > 0.001f)
            DrawPill(p, r, Style().Fill(StateFill(onColor).Fade(colorT)));
        Style ks;
        ks.Radius(knob.Height() * 0.5f).Shadow(Color::Black(0.18f + 0.12f * lens), Sc(5 + 10 * lens), Vec2(0, Sc(2 + 2.5f * lens)));
        if (lens > 0.02f)
        {
            GlassMaterial m = LensMaterial();
            m.refraction *= 0.6f + 0.6f * Saturate(lens);
            ks.Glass(m).Fill(c.controlKnob.Fade(1.0f - 0.9f * Saturate(lens)));
        }
        else
            ks.Fill(c.controlKnob);
        p.Rect(knob, ks);
        return changed;
    }

    bool Toggle(const char* id, bool* value)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return false;
        const Vec2 sz = ToggleSize();
        const Vec2 pos = w->DC.CursorPos;
        ImGui::ItemSize(sz);
        return ToggleAt(w->GetID(id), Rect::FromSize(pos, sz), value);
    }

    bool Checkbox(const char* label, bool* value)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return false;
        const char* labelEnd = ImGui::FindRenderedTextEnd(label);
        const FontRef f = GetFont(TextStyle::Body);
        const Vec2 ts = Painter::MeasureText(f, label, labelEnd);
        const float d = Sc(22);
        const float h = std::max(d, ts.y);
        Interaction it = Interact(label, Vec2(d + (ts.x > 0 ? Sc(10) + ts.x : 0), h), InteractFlags_None);
        if (!it.visible)
            return false;
        if (it.pressed)
        {
            *value = !*value;
            ImGui::MarkItemEdited(it.id);
        }
        const Palette& c = C();
        // fill cross-fades smoothly, the check mark draws itself, the dot "pops" (bouncy) on every click
        const float on = Saturate(Anim(it.id, 0x11, *value ? 1.0f : 0.0f, Spring{0.26f, 1.0f}));
        const float draw = Saturate(Anim(it.id, 0x12, *value ? 1.0f : 0.0f, Spring{0.36f, 0.92f}));
        const float pop = LiquidPulse(it.id, it.held, it.pressed, 0.12f);
        Painter p;
        const Vec2 cc(it.rect.min.x + d * 0.5f, it.rect.Center().y);
        const float scale = 1.0f - 0.10f * it.press + 0.10f * pop * (1.0f - it.press);
        p.PushScale(cc, scale);
        p.Circle(cc, d * 0.5f, Style().Stroke(Sc(1.6f), c.tertiaryLabel.Fade(1.0f - on)).Fill(StateFill(Accent()).Fade(on)));
        if (draw > 0.01f)
        {
            const float k = draw;
            const Vec2 a(cc.x - d * 0.22f, cc.y + d * 0.01f), b(cc.x - d * 0.05f, cc.y + d * 0.18f), e(cc.x + d * 0.24f, cc.y - d * 0.17f);
            Style ls = Style().Fill(c.onAccent.Fade(k));
            p.Line(a, Lerp(a, b, std::min(1.0f, k * 2.0f)), Sc(2.2f), ls);
            if (k > 0.5f)
                p.Line(b, Lerp(b, e, (k - 0.5f) * 2.0f), Sc(2.2f), ls);
        }
        p.PopScale();
        if (ts.x > 0)
            p.Text(Vec2(it.rect.min.x + d + Sc(10), std::floor(it.rect.Center().y - ts.y * 0.5f)), f, LabelOr(c.label), label, labelEnd);
        return it.pressed;
    }

    // ========================================================= toggle button
    // iOS Control Center toggle. Off: clear glass. Pressed: the glass sinks and lights up under the pointer.
    // Released: the tint floods in from the press point (a masked disc growing past the rim), the button pops
    // on a spring and the symbol bounces. Switching off drains the tint back towards the press point.
    bool ToggleButton(const char* id, bool* value, Icon icon, const ToggleButtonOptions& o)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return false;
        const float d = Sc(o.diameter);
        Interaction it = Interact(id, Vec2(d, d), InteractFlags_None);
        if (!it.visible)
            return false;
        const Theme& t = T();
        const Palette& c = t.colors;
        const Color tint = Tint(o.tint);
        const Vec2 center = it.rect.Center();
        const float r = d * 0.5f;

        struct PressPoint
        {
            Vec2 at{0.0f, 0.0f};   // relative to the center, in radii
        };
        PressPoint& press = Ctx().state.Get<PressPoint>(it.id);
        if (it.held || it.pressed)
        {
            Vec2 rel = (ImGui::GetIO().MousePos - center) / r;
            const float len = Length(rel);
            if (len > 0.8f)
                rel = rel * (0.8f / len);
            press.at = rel;
        }
        const ImGuiID popId = Salt(it.id, 0x72), bounceId = Salt(it.id, 0x73);
        if (it.pressed)
        {
            *value = !*value;
            ImGui::MarkItemEdited(it.id);
            anim::Kick(popId, *value ? 26.0f : 16.0f);
            anim::Kick(bounceId, 34.0f);
        }
        static const Spring kFlood{0.34f, 0.95f};
        static const Spring kPop{0.34f, 0.46f};
        static const Spring kBounce{0.30f, 0.38f};
        const float on = Anim(it.id, 0x74, *value ? 1.0f : 0.0f, kFlood);
        const float pop = anim::Float(popId, 0.0f, &kPop);
        const float bounce = anim::Float(bounceId, 0.0f, &kBounce);
        const float light = Anim(it.id, 0x75, it.held ? 1.0f : 0.0f, Spring{0.18f, 1.0f});
        const float k = Saturate(on);

        Painter p;
        p.PushScale(center, 1.0f - 0.07f * it.press + 0.06f * pop);
        Style base = Style().Glass(LookMaterial(t.materials.control)).Shadow(c.shadow.Fade(0.6f), Sc(12), Vec2(0, Sc(4)));
        if (!LookClear())
            base.Fill(FillOr(c.fill).Fade(1.0f - 0.6f * k));
        if (o.glow && k > 0.01f)
            base.Glow(tint, Sc(12), 0.45f * k);
        p.Circle(center, r, base);

        const Vec2 origin = center + press.at * r;
        p.PushMask(it.rect, r);
        if (on > 0.002f)
        {
            // flood: grows from the press point (fast start, soft landing) until it covers the whole disc
            const float cover = r + Length(origin - center) + 1.0f;
            const float grow = on >= 1.0f ? on : 1.0f - (1.0f - on) * (1.0f - on);
            const float seed = *value ? 0.15f : 0.0f;   // switching off drains all the way into the press point
            p.Circle(origin, cover * (seed + (1.0f - seed) * std::min(grow, 1.04f)),
                     Style().Fill(Paint::Radial(tint.Lighter(0.22f), tint)).Opacity(Saturate(on * 5.0f)));
        }
        if (light > 0.01f)
        {
            // touch illumination under the pointer
            p.Circle(origin, r * 0.9f, Style().Fill(Paint::Radial(Color::White(0.45f * light), Color::White(0.0f))));
        }
        p.PopMask();

        p.Icon(center, icon, d * 0.40f * (1.0f + 0.12f * bounce), Lerp(LabelOr(c.label), Color::White(), k));
        p.PopScale();
        return it.pressed;
    }

    // ================================================================ slider
    bool detail::SliderAt(ImGuiID id, const Rect& r, float* value, float mn, float mx, const SliderOptions& o)
    {
        const Palette& c = C();
        const float iconSize = Sc(15);
        const float iconPad = Sc(26);
        Rect area = r;
        if (o.minIcon)
            area.min.x += iconPad;
        if (o.maxIcon)
            area.max.x -= iconPad;

        Interaction it = InteractImpl(id, r, InteractFlags_PressOnClick);
        if (!it.visible)
            return false;

        const float kwRest = Sc(34), khRest = Sc(22);
        const float x0 = area.min.x + kwRest * 0.5f, x1 = std::max(area.max.x - kwRest * 0.5f, x0);   // never inverted
        bool changed = false;
        if (it.held && mx > mn)
        {
            float t = Saturate((ImGui::GetIO().MousePos.x - x0) / std::max(x1 - x0, 1.0f));
            float nv = mn + t * (mx - mn);
            if (o.step > 0.0f)
                nv = mn + std::round((nv - mn) / o.step) * o.step;
            nv = Clamp(nv, mn, mx);
            if (nv != *value)
            {
                *value = nv;
                changed = true;
                ImGui::MarkItemEdited(id);
            }
        }
        const float frac = mx > mn ? Saturate((*value - mn) / (mx - mn)) : 0.0f;
        // dragging follows the pointer tightly, jumps (click on the track) glide with a soft spring
        const ImGuiID shownId = Salt(id, 0x20);
        static const Spring kFollow{0.10f, 1.0f}, kGlide{0.38f, 0.78f};
        const float shown = anim::Float(shownId, frac, it.held ? &kFollow : &kGlide);
        const float vel = anim::Velocity(shownId);
        const float press = LiquidPulse(id, it.held, it.pressed);

        Painter p;
        const float cy = r.Center().y;
        const float th = Sc(6);
        const Rect track(area.min.x, cy - th * 0.5f, area.max.x, cy + th * 0.5f);
        const Color tint = Tint(o.tint);
        DrawPill(p, track, Surface(FillOr(c.fill)));
        const float kx = Lerp(x0, x1, shown);
        DrawPill(p, Rect(track.min.x, track.min.y, std::max(kx, track.min.x + th), track.max.y), Style().Fill(StateFill(tint)));

        if (o.minIcon)
            p.Icon(Vec2(r.min.x + iconPad * 0.4f, cy), o.minIcon, iconSize, c.secondaryLabel);
        if (o.maxIcon)
            p.Icon(Vec2(r.max.x - iconPad * 0.4f, cy), o.maxIcon, iconSize * 1.15f, c.secondaryLabel);

        // the lens swells when grabbed, stretches along fast motion and thins a little (liquid drop)
        const float speed = std::min(std::fabs(vel) * (x1 - x0) * 0.03f, kwRest * 0.6f);
        const float kw = Lerp(kwRest, Sc(46), press) + speed;
        const float kh = Lerp(khRest, Sc(30), press) - std::min(speed * 0.12f, Sc(3));
        const Rect knob = Rect::FromCenter(Vec2(kx, cy), Vec2(kw, kh));
        Style ks;
        ks.Radius(kh * 0.5f).Shadow(Color::Black(0.18f + 0.1f * press), Sc(6 + 10 * press), Vec2(0, Sc(2 + 2 * press)));
        if (press > 0.02f)
            ks.Glass(LensMaterial()).Fill(c.controlKnob.Fade(1.0f - 0.9f * Saturate(press)));
        else
            ks.Fill(c.controlKnob);
        p.Rect(knob, ks);
        return changed;
    }

    bool Slider(const char* id, float* value, float mn, float mx, const SliderOptions& o)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return false;
        const float width = o.width > 0.0f ? Sc(o.width) : AvailWidth();
        const Vec2 size(width, Sc(32));
        const Vec2 pos = w->DC.CursorPos;
        ImGui::ItemSize(size);
        return SliderAt(w->GetID(id), Rect::FromSize(pos, size), value, mn, mx, o);
    }

    // =============================================================== stepper
    Vec2 detail::StepperSize() { return Vec2(Sc(96), Sc(32)); }

    bool detail::StepperAt(ImGuiID id, const Rect& r, int* value, int mn, int mx, int step)
    {
        const Palette& c = C();
        Painter p;
        const float radius = ItemRadius(Sc(9));
        Style well = Style().Radius(radius);
        p.Rect(r, SurfaceFill(well, FillOr(c.tertiaryFill)));
        const Rect left = r.Left(r.Width() * 0.5f), right = r.Right(r.Width() * 0.5f);
        bool changed = false;
        Interaction a = InteractImpl(Salt(id, 1), left, InteractFlags_Repeat);
        Interaction b = InteractImpl(Salt(id, 2), right, InteractFlags_Repeat);
        const bool canDec = *value > mn, canInc = *value < mx;
        if (a.pressed && canDec)
        {
            *value = std::max(mn, *value - step);
            changed = true;
        }
        if (b.pressed && canInc)
        {
            *value = std::min(mx, *value + step);
            changed = true;
        }
        if (changed)
            ImGui::MarkItemEdited(id);
        if (a.press > 0.01f)
            p.Rect(left, Style().Radius(radius, 0, 0, radius).Fill(c.highlight.Fade(a.press * 1.5f)));
        if (b.press > 0.01f)
            p.Rect(right, Style().Radius(0, radius, radius, 0).Fill(c.highlight.Fade(b.press * 1.5f)));
        const float mid = r.Center().x;
        p.FillRect(Rect(mid - 0.5f, r.min.y + r.Height() * 0.25f, mid + 0.5f, r.max.y - r.Height() * 0.25f), c.separator);
        const float is = Sc(14);
        const Color fg = LabelOr(c.label);
        p.Icon(left.Center(), icons::Remove, is, canDec ? fg : fg.Fade(0.35f));
        p.Icon(right.Center(), icons::Add, is, canInc ? fg : fg.Fade(0.35f));
        return changed;
    }

    bool Stepper(const char* id, int* value, int mn, int mx, int step)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return false;
        const Vec2 sz = StepperSize();
        const Vec2 pos = w->DC.CursorPos;
        ImGui::ItemSize(sz);
        ImGui::PushID(id);
        const bool changed = StepperAt(ImGui::GetID("##stepper"), Rect::FromSize(pos, sz), value, mn, mx, step);
        ImGui::PopID();
        return changed;
    }

    // ============================================================= segmented
    bool detail::SegmentedAt(ImGuiID id, const Rect& r, int* selected, const char* const* items, int count)
    {
        if (count <= 0)
            return false;
        const Palette& c = C();
        const Theme& t = T();
        Painter p;
        const float trackR = std::min(ItemRadius(r.Height() * 0.5f), r.Height() * 0.5f);
        DrawPill(p, r, Surface(FillOr(c.tertiaryFill)));
        const float segW = r.Width() / count;
        // tap a segment or drag the selection: it travels as a clear lens and lands as a solid pill (iOS 26+)
        const LiquidSelection sel = LiquidSelect(id, r, count, selected);
        if (sel.changed)
            ImGui::MarkItemEdited(id);
        const Rect pill = LiquidSelectionRect(sel, r, count, Sc(2.5f));
        const float solid = 1.0f - Saturate(sel.lens);
        // the selected segment's background: the style's, else the tint, else - on a custom track - a light,
        // see-through pill that sits on any color, else the iOS gray / white
        const ItemStyle& st = ResolvedStyle();
        Color selFill = t.dark ? Color::Hex(0x636366) : Color::White();
        if (st.Has(ItemStyle::kFill))
            selFill = Color::White(t.dark ? 0.26f : 0.90f);
        if (st.Has(ItemStyle::kTint))
            selFill = st.tint;
        if (st.Has(ItemStyle::kSelectedFill))
            selFill = st.selectedFill;
        const Color labelColor = LabelOr(c.label);
        const Color selLabel = st.Has(ItemStyle::kSelectedLabel) ? st.selectedLabel : labelColor;
        if (solid > 0.01f)
        {
            Style ps = Style().Radius(std::min(pill.Height() * 0.5f, std::max(trackR - (pill.min.y - r.min.y), 0.0f))).Fill(StateFill(selFill).Fade(solid));
            if (!LookClear())
                ps.Shadow(Color::Black((t.dark ? 0.3f : 0.12f) * solid), Sc(8), Vec2(0, Sc(2)));
            p.Rect(pill, ps);
        }
        const float sIdx = sel.pos;

        // labels follow the pill, not the index: a segment turns semibold as the pill slides over it
        for (int i = 0; i < count; ++i)
        {
            Rect sr(r.min.x + segW * i, r.min.y, r.min.x + segW * (i + 1), r.max.y);
            const float cover = Saturate(1.0f - std::fabs(sIdx - (float)i));
            const char* end = ImGui::FindRenderedTextEnd(items[i]);
            auto label = [&](FontWeight weight, float alpha) {
                if (alpha <= 0.01f)
                    return;
                const FontRef f = Font(weight, 13.5f);
                const Vec2 ts = Painter::MeasureText(f, items[i], end);
                p.Text(Vec2(std::floor(sr.Center().x - ts.x * 0.5f + 0.5f), std::floor(sr.Center().y - ts.y * 0.5f + 0.5f)), f,
                       Lerp(labelColor, selLabel, cover * solid).Fade(alpha), items[i], end);   // the selected color once the pill lands
            };
            const float bold = SmoothStep(0.25f, 0.75f, cover);
            label(FontWeight::Regular, 1.0f - bold);
            label(FontWeight::Semibold, bold);
        }
        DrawSelectionLens(p, pill, sel, Sc(4), st.Has(ItemStyle::kMovingFill) ? st.movingFill : Color::Clear());
        return sel.changed;
    }

    bool Segmented(const char* id, int* selected, const char* const* items, int count, float width)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return false;
        const float wdt = width > 0.0f ? Sc(width) : AvailWidth();
        const Vec2 size(wdt, Sc(32));
        const Vec2 pos = w->DC.CursorPos;
        ImGui::ItemSize(size);
        const ImGuiID gid = w->GetID(id);
        ImGui::ItemAdd(ImRect(pos, pos + size), gid, nullptr, ImGuiItemFlags_NoNav);
        return SegmentedAt(gid, Rect::FromSize(pos, size), selected, items, count);
    }

    // ================================================================= menus
    bool detail::BeginGlassPopup(ImGuiID id, float minWidth)
    {
        ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Vec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2(Sc(6), Sc(6)));
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
        ImGui::SetNextWindowSizeConstraints(Vec2(minWidth, 0), Vec2(FLT_MAX, FLT_MAX));
        const bool open = ImGui::BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove);
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
        if (!open)
        {
            ImGui::PopStyleVar();
            return false;
        }
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (ImGui::IsWindowAppearing())
            anim::Set(Salt(w->ID, 0x50), 0.0f);
        const float a = Anim(w->ID, 0x50, 1.0f, SpringStd(), 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * Saturate(a));
        Painter p(w->DrawList);
        const Rect wr(w->Pos, w->Pos + w->Size);
        {
            ScopedUnclip unclip(w->DrawList, wr, ShadowExtent(Sc(28), Vec2(0, Sc(10))));
            p.Rect(wr.Scaled(0.96f + 0.04f * a), Style().Radius(Sc(14)).Glass(LookMaterial(T().materials.popover)).Shadow(C().shadow.Fade(1.2f), Sc(28), Vec2(0, Sc(10))));
        }
        return true;
    }

    void detail::EndGlassPopup()
    {
        ImGui::PopStyleVar(2);   // alpha + item spacing
        ImGui::EndPopup();
    }

    bool detail::PopupRow(const char* label, bool selected, Icon icon)
    {
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        const FontRef f = GetFont(TextStyle::Body);
        const char* end = ImGui::FindRenderedTextEnd(label);
        const Vec2 ts = Painter::MeasureText(f, label, end);
        const float h = Sc(34);
        const float width = std::max(AvailWidth(), ts.x + Sc(64));
        Interaction it = Interact(label, Vec2(width, h), InteractFlags_None);
        if (!it.visible)
            return false;
        const Palette& c = C();
        Painter p(w->DrawList);
        if (it.hover > 0.01f)
            p.Rect(it.rect, Style().Radius(Sc(9)).Fill(c.highlight.Fade(it.hover * 1.4f)));
        float x = it.rect.min.x + Sc(10);
        if (selected)
            p.Icon(Vec2(x + Sc(7), it.rect.Center().y), icons::Checkmark, Sc(13), c.label);
        x += Sc(24);
        if (icon)
        {
            p.Icon(Vec2(x + Sc(8), it.rect.Center().y), icon, Sc(14), c.label);
            x += Sc(24);
        }
        p.Text(Vec2(x, std::floor(it.rect.Center().y - ts.y * 0.5f)), f, c.label, label, end);
        return it.pressed;
    }

    bool detail::PickerAt(ImGuiID id, const Rect& r, int* selected, const char* const* items, int count, bool plain)
    {
        const Palette& c = C();
        const int sel = (count > 0) ? std::clamp(*selected, 0, count - 1) : -1;
        const char* current = sel >= 0 ? items[sel] : "";
        Interaction it = InteractImpl(id, r, InteractFlags_None);
        Painter p;
        const FontRef f = GetFont(TextStyle::Body);
        const char* end = ImGui::FindRenderedTextEnd(current);
        const Vec2 ts = Painter::MeasureText(f, current, end);
        const float chevron = Sc(10);
        if (!plain)
        {
            Style well = Style().Radius(ItemRadius(Sc(10)));
            p.Rect(r, SurfaceFill(well, FillOr(c.tertiaryFill).Fade(1.0f + 0.5f * it.hover)));
        }
        const float right = r.max.x - (plain ? Sc(2) : Sc(10));
        const Color fg = LabelOr(plain ? c.secondaryLabel : c.label);
        p.Icon(Vec2(right - chevron * 0.5f, r.Center().y), icons::ChevronDown, chevron, fg.Fade(0.9f));
        const float tx = plain ? right - chevron - Sc(6) - ts.x : r.min.x + Sc(10);
        p.Text(Vec2(std::floor(tx), std::floor(r.Center().y - ts.y * 0.5f)), f, fg.Fade(1.0f - 0.3f * it.press), current, end);

        const ImGuiID popupId = Salt(id, 0x60);
        if (it.pressed)
            ImGui::OpenPopupEx(popupId);
        bool changed = false;
        ImGui::SetNextWindowPos(Vec2(r.max.x, r.max.y + Sc(6)), ImGuiCond_Appearing, Vec2(1.0f, 0.0f));
        if (BeginGlassPopup(popupId, std::max(r.Width(), Sc(180))))
        {
            for (int i = 0; i < count; ++i)
            {
                ImGui::PushID(i);
                if (PopupRow(items[i], i == sel))
                {
                    *selected = i;
                    changed = true;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopID();
            }
            EndGlassPopup();
        }
        if (changed)
            ImGui::MarkItemEdited(id);
        return changed;
    }

    bool Picker(const char* id, int* selected, const char* const* items, int count, float width)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return false;
        float wdt = width > 0.0f ? Sc(width) : 0.0f;
        if (width < 0.0f)
            wdt = AvailWidth();
        if (wdt == 0.0f)
        {
            const FontRef f = GetFont(TextStyle::Body);
            for (int i = 0; i < count; ++i)
                wdt = std::max(wdt, Painter::MeasureText(f, items[i]).x);
            wdt += Sc(44);
        }
        const Vec2 size(wdt, Sc(34));
        const Vec2 pos = w->DC.CursorPos;
        ImGui::ItemSize(size);
        return PickerAt(w->GetID(id), Rect::FromSize(pos, size), selected, items, count, false);
    }

    // ============================================================== progress
    void ProgressBar(float fraction, float width, Color tint)
    {
        ProgressOptions o;
        o.width = width;
        o.tint = tint;
        ProgressBar(fraction, o);
    }

    void ProgressBar(float fraction, const ProgressOptions& o)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return;
        const float wdt = o.width > 0.0f ? Sc(o.width) : AvailWidth();
        const Vec2 size(wdt, Sc(6));
        const Vec2 pos = w->DC.CursorPos;
        ImGui::ItemSize(size);
        const ImGuiID id = w->GetID("##progress") ^ (ImGuiID)(pos.y * 131.0f);
        if (!ImGui::ItemAdd(ImRect(pos, pos + size), 0))
            return;
        const float shown = Anim(id, 0x70, Saturate(fraction), SpringStd());
        Painter p;
        const Rect r = Rect::FromSize(pos, size);
        DrawPill(p, r, Surface(FillOr(C().fill)));
        if (shown > 0.001f)
        {
            const Color t = Tint(o.tint);
            Style s = Style().Fill(Paint::Linear(StateFill(t.Lighter(0.25f)), StateFill(t), 0));
            if (o.glow)
                s.Glow(t, Sc(6), 0.35f);
            DrawPill(p, Rect(r.min.x, r.min.y, r.min.x + std::max(r.Height(), r.Width() * shown), r.max.y), s);
        }
    }

    void ProgressRing(float fraction, float diameter, float thickness, Color tint)
    {
        ProgressOptions o;
        o.diameter = diameter;
        o.thickness = thickness;
        o.tint = tint;
        ProgressRing(fraction, o);
    }

    void ProgressRing(float fraction, const ProgressOptions& o)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return;
        const float d = o.diameter > 0.0f ? Sc(o.diameter) : Sc(44);
        const float th = o.thickness > 0.0f ? Sc(o.thickness) : std::max(Sc(3), d * 0.12f);
        const Vec2 pos = w->DC.CursorPos;
        ImGui::ItemSize(Vec2(d, d));
        if (!ImGui::ItemAdd(ImRect(pos, pos + Vec2(d, d)), 0))
            return;
        const ImGuiID id = w->GetID("##ring") ^ (ImGuiID)(pos.x * 31.0f + pos.y * 131.0f);
        const float shown = Anim(id, 0x71, Saturate(fraction), SpringStd());
        const Vec2 c(pos.x + d * 0.5f, pos.y + d * 0.5f);
        const float rad = (d - th) * 0.5f;
        const Color t = Tint(o.tint);
        Painter p;
        p.Ring(c, rad, th, Style().Fill(t.Fade(0.18f)));
        if (shown > 0.001f)
        {
            Style s = Style().Fill(Paint::Conic(StateFill(t.Lighter(0.35f)), StateFill(t), -90));
            if (o.glow)
                s.Glow(t, Sc(5), 0.4f);
            p.Arc(c, rad, th, -kPi * 0.5f, kTau * shown, s);
        }
    }

    void LineChart(const char* id, const float* values, int count, const LineChartOptions& o)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return;
        const ImGuiID gid = w->GetID(id);
        Rect r = o.rect;
        if (r.Empty())
        {
            const Vec2 size(o.width > 0.0f ? Sc(o.width) : AvailWidth(), Sc(o.height));
            const Vec2 pos = w->DC.CursorPos;
            ImGui::ItemSize(size);
            if (!ImGui::ItemAdd(ImRect(pos, pos + size), gid))
                return;
            r = Rect::FromSize(pos, size);
        }
        if (!values || count < 2)
            return;

        // value range: fixed, or fitted to the data (from 0 for positive data) and eased so it never jumps
        float lo = o.min, hi = o.max;
        if (!(hi > lo))
        {
            float dmin = values[0], dmax = values[0];
            for (int i = 1; i < count; ++i)
            {
                dmin = std::min(dmin, values[i]);
                dmax = std::max(dmax, values[i]);
            }
            lo = std::min(dmin, 0.0f);
            hi = dmax + (dmax - lo) * 0.1f;
            if (!(hi > lo))
                hi = lo + 1.0f;
            lo = Anim(gid, 0x80, lo, SpringStd(), lo);
            hi = Anim(gid, 0x81, hi, SpringStd(), hi);
        }

        const float th = Sc(o.thickness);
        const float dot = o.lastPoint ? th * 1.6f : 0.0f;
        const float pad = th * 0.5f + dot + Sc(1);
        const Rect plot(r.min.x + th * 0.5f, r.min.y + pad, r.max.x - std::max(th * 0.5f, dot + Sc(1)), r.max.y - th * 0.5f);
        static thread_local ImVector<Vec2> pts;
        pts.resize(count);
        for (int i = 0; i < count; ++i)
        {
            const float v = values[((o.offset % count) + count + i) % count];
            const float k = Saturate((v - lo) / (hi - lo));
            pts[i] = Vec2(plot.min.x + plot.Width() * (float)i / (float)(count - 1), plot.max.y - plot.Height() * k);
        }
        const Color t = Tint(o.tint);
        const std::uint32_t flags = o.smooth ? PolylineFlags_Smooth : PolylineFlags_None;
        Painter p;
        if (o.fill)
            p.Area(pts.Data, count, r.max.y, Paint::Linear(t.Fade(0.30f), t.Fade(0.0f), 90), flags);
        Style s = Style().Fill(t);
        if (o.glow)
            s.Glow(t, Sc(10), 0.9f);
        p.Polyline(pts.Data, count, th, s, flags);
        if (o.lastPoint)
        {
            const Vec2 last = pts[count - 1];
            p.Circle(last, dot * 1.9f, Style().Fill(t.Fade(0.22f)));
            p.Circle(last, dot, Style().Fill(t));
        }
    }

    void ActivityIndicator(float diameter, Color tint)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return;
        const float d = diameter > 0.0f ? Sc(diameter) : Sc(24);
        const Vec2 pos = w->DC.CursorPos;
        ImGui::ItemSize(Vec2(d, d));
        if (!ImGui::ItemAdd(ImRect(pos, pos + Vec2(d, d)), 0))
            return;
        const Color col = tint.a > 0 ? tint : C().secondaryLabel;
        const Vec2 c(pos.x + d * 0.5f, pos.y + d * 0.5f);
        const float t = (float)std::fmod(anim::Time(), 1.0);
        const int spokes = 8;
        Painter p;
        for (int i = 0; i < spokes; ++i)
        {
            const float a = kTau * i / spokes - kPi * 0.5f;
            const float phase = std::fmod((float)i / spokes - t + 1.0f, 1.0f);
            const float alpha = 0.18f + 0.82f * phase * phase;
            const Vec2 dir(std::cos(a), std::sin(a));
            p.Line(c + dir * (d * 0.24f), c + dir * (d * 0.44f), d * 0.09f, Style().Fill(col.Fade(alpha)));
        }
    }

    void Badge(const char* text, Color tint)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return;
        const FontRef f = Font(FontWeight::Semibold, 11.5f);
        const Vec2 ts = Painter::MeasureText(f, text);
        const Vec2 size(std::max(ts.x + Sc(14), Sc(20)), Sc(20));
        const Vec2 pos = w->DC.CursorPos;
        ImGui::ItemSize(size);
        if (!ImGui::ItemAdd(ImRect(pos, pos + size), 0))
            return;
        const Color t = tint.a > 0 ? tint : FillOr(AccentOr(C().red));   // the explicit color wins over the style
        Painter p;
        const Rect r = Rect::FromSize(pos, size);
        const bool clear = LookClear();   // transparent: clear glass, the text takes the badge color
        DrawPill(p, r, Surface(t));
        p.Text(Vec2(std::floor(r.Center().x - ts.x * 0.5f), std::floor(r.Center().y - ts.y * 0.5f)), f, LabelOr(clear ? t : Color::White()), text);
    }

    void Image(ImTextureID texture, Vec2 size, float radius, Vec2 uv0, Vec2 uv1)
    {
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return;
        const Vec2 sz(Sc(size.x), Sc(size.y));
        const Vec2 pos = w->DC.CursorPos;
        ImGui::ItemSize(sz);
        if (!ImGui::ItemAdd(ImRect(pos, pos + sz), 0))
            return;
        Painter p;
        p.Image(texture, Rect::FromSize(pos, sz), radius < 0.0f ? Sc(T().metrics.controlRadius) : Sc(radius), Color::White(), uv0, uv1);
    }

    bool ColorSwatches(const char* id, int* selected, const Color* colors, int count, float diameter)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return false;
        const float d = diameter > 0.0f ? Sc(diameter) : Sc(28);
        const float gap = Sc(10);
        const Vec2 pos = w->DC.CursorPos;
        const Vec2 size(count * d + (count - 1) * gap, d + Sc(6));
        ImGui::ItemSize(size);
        ImGui::PushID(id);
        bool changed = false;
        Painter p;
        for (int i = 0; i < count; ++i)
        {
            const Vec2 c(pos.x + i * (d + gap) + d * 0.5f, pos.y + size.y * 0.5f);
            Interaction it = InteractImpl(ImGui::GetID(i), Rect::FromCenter(c, Vec2(d, d)), InteractFlags_None);
            if (it.pressed && *selected != i)
            {
                *selected = i;
                changed = true;
            }
            const float on = Anim(it.id, 0x80, *selected == i ? 1.0f : 0.0f, SpringBouncy());
            const float r = d * 0.5f * (1.0f - 0.08f * it.press);
            p.Circle(c, r - Sc(3) * on, Style().Fill(Paint::Radial(colors[i].Lighter(0.18f), colors[i], Vec2(0.35f, 0.3f), 0.8f)));
            if (on > 0.01f)
                p.Circle(c, r + Sc(1.5f), Style().Stroke(Sc(2), colors[i].Fade(on), 0.0f));
        }
        ImGui::PopID();
        return changed;
    }

    void Tooltip(const char* text)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        if (!ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            return;
        ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2(Sc(10), Sc(6)));
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
        if (ImGui::BeginTooltip())
        {
            ImGuiWindow* w = ImGui::GetCurrentWindow();
            Painter p(w->DrawList);
            const Rect tr(w->Pos, w->Pos + w->Size);
            {
                ScopedUnclip unclip(w->DrawList, tr, ShadowExtent(Sc(16), Vec2(0, Sc(6))));
                p.Rect(tr, Style().Radius(Sc(10)).Glass(LookMaterial(T().materials.popover)).Shadow(C().shadow, Sc(16), Vec2(0, Sc(6))));
            }
            TextImpl(GetFont(TextStyle::Footnote), C().label, text, nullptr, false);
            ImGui::EndTooltip();
        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }

    // ============================================================ icon tile
    void detail::IconTile(Painter& p, const Rect& r, Icon icon, Color color)
    {
        const Color base = Tint(color);
        p.Rect(r, Style().Radius(r.Height() * 0.26f).Fill(Paint::Linear(StateFill(base.Lighter(0.14f)), StateFill(base.Darker(0.06f)), 90)));
        p.Icon(r.Center(), icon, r.Height() * 0.56f, Color::White());
    }
}
