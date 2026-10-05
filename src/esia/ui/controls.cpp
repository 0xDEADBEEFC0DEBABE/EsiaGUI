// Esia UI - text and controls: buttons, switches, sliders, steppers, progress, badges (WGT's, ported).
#include "ui_internal.hpp"
#include <algorithm>
#include <charconv>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace esia::ui
{
    using namespace detail;

    // ================================================================= text
    void detail::TextImpl(text::FontRef f, Color color, std::string_view str, bool wrap)
    {
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const Vec2 pos = c.CursorPos();
        float wrapWidth = 0.0f;
        if (wrap)
        {
            MarkFill();   // wrapping text stretches to the width it is offered (auto layout)
            wrapWidth = std::max(c.ContentRegionAvail().x, 1.0f);
        }
        text::TextMetrics tm = m.text && !str.empty() ? m.text->Measure(f, str, wrapWidth) : text::TextMetrics{};
        if (!wrap && !InLayoutContainer())
        {
            // Text never runs past its container: wider than what is left of its line (after SameLine items too),
            // it wraps at the container's edge. Auto-layout containers size and place their children themselves.
            const float room = c.ContentRegionAvail().x;
            if (room > 0.0f && tm.size.x > room + 0.5f && m.text)
            {
                wrapWidth = std::max(room, f.size * 4.0f);
                tm = m.text->Measure(f, str, wrapWidth);
            }
        }
        const Vec2 size(tm.size.x, std::max(tm.size.y, f.size));
        c.ItemSize(size, tm.baseline > 0.0f ? tm.baseline : -1.0f);
        if (!c.ItemAdd(0, Rect::FromSize(pos, size)) || str.empty())
            return;
        Painter p = GetPainter();
        p.Text(pos, f, color, str, wrapWidth);
    }

    std::size_t detail::FormatV(char* buf, std::size_t size, const char* fmt, va_list args)
    {
        if (size == 0)
            return 0;
        // the conversions first, so a format this does not write goes to vsnprintf with its arguments untouched
        for (const char* p = fmt; *p; ++p)
        {
            if (*p != '%')
                continue;
            ++p;
            if (*p == '%')
                continue;
            if (*p == 'z')
                ++p;
            else if (*p == 'l')
                p += p[1] == 'l' ? 2 : 1;
            if (!*p || !std::strchr("diuxXsc", *p) || (*p == 'c' && p[-1] != '%') || (*p == 's' && p[-1] != '%'))
            {
                const int n = std::vsnprintf(buf, size, fmt, args);
                return n < 0 ? 0 : std::min((std::size_t)n, size - 1);
            }
        }
        char* out = buf;
        char* const end = buf + size - 1;
        auto put = [&](const char* s, std::size_t n) {
            n = std::min(n, (std::size_t)(end - out));
            std::memcpy(out, s, n);
            out += n;
        };
        for (const char* p = fmt; *p && out < end; ++p)
        {
            if (*p != '%')
            {
                *out++ = *p;
                continue;
            }
            ++p;
            int length = 0;   // 1 = l, 2 = ll, 3 = z
            if (*p == 'z')
            {
                length = 3;
                ++p;
            }
            else if (*p == 'l')
            {
                length = p[1] == 'l' ? 2 : 1;
                p += length;
            }
            char num[24];
            std::to_chars_result r{num, std::errc()};
            switch (*p)
            {
            case '%':
                *out++ = '%';
                break;
            case 'd':
            case 'i':
                r = length == 0 ? std::to_chars(num, num + sizeof(num), va_arg(args, int))
                    : length == 1 ? std::to_chars(num, num + sizeof(num), va_arg(args, long))
                    : length == 2 ? std::to_chars(num, num + sizeof(num), va_arg(args, long long))
                                  : std::to_chars(num, num + sizeof(num), va_arg(args, std::ptrdiff_t));
                put(num, (std::size_t)(r.ptr - num));
                break;
            case 'u':
            case 'x':
            case 'X':
            {
                const int base = *p == 'u' ? 10 : 16;
                r = length == 0 ? std::to_chars(num, num + sizeof(num), va_arg(args, unsigned), base)
                    : length == 1 ? std::to_chars(num, num + sizeof(num), va_arg(args, unsigned long), base)
                    : length == 2 ? std::to_chars(num, num + sizeof(num), va_arg(args, unsigned long long), base)
                                  : std::to_chars(num, num + sizeof(num), va_arg(args, std::size_t), base);
                if (*p == 'X')
                    for (char* c = num; c < r.ptr; ++c)
                        if (*c >= 'a' && *c <= 'f')
                            *c = (char)(*c - 'a' + 'A');
                put(num, (std::size_t)(r.ptr - num));
                break;
            }
            case 's':
            {
                const char* s = va_arg(args, const char*);
                if (!s)
                    s = "(null)";
                put(s, std::strlen(s));
                break;
            }
            case 'c':
                *out++ = (char)va_arg(args, int);
                break;
            default:
                break;
            }
        }
        *out = '\0';
        return (std::size_t)(out - buf);
    }

    namespace
    {
        void TextV(TextStyle style, Color color, const char* fmt, va_list args)
        {
            char buf[1024];
            FormatV(buf, sizeof(buf), fmt, args);
            TextImpl(Font(style), color, buf, false);   // up to a %c of 0, as ever
        }
    }

    void Text(TextStyle style, const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        TextV(style, LabelOr(C().label), fmt, args);
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

    void TextWrapped(TextStyle style, Color color, std::string_view text) { TextImpl(Font(style), color, text, true); }
    void TextUnformatted(TextStyle style, Color color, std::string_view text) { TextImpl(Font(style), color, text, false); }
    void LargeTitle(std::string_view text) { TextImpl(Font(TextStyle::LargeTitle), LabelOr(C().label), text, false); }
    void Headline(std::string_view text) { TextImpl(Font(TextStyle::Headline), LabelOr(C().label), text, false); }

    void Spacer(float height)
    {
        Context& c = Ctx();
        const float h = height < 0.0f ? Sc(T().metrics.spacing) : Sc(height);
        c.ItemSize(Vec2(0.0f, h));
    }

    void Divider()
    {
        Context& c = Ctx();
        const Vec2 pos = c.CursorPos();
        const float w = AvailableWidth();
        c.ItemSize(Vec2(w, Sc(9)));
        if (!c.ItemAdd(0, Rect::FromSize(pos, Vec2(w, Sc(9)))))
            return;
        Painter p = GetPainter();
        p.HLine(pos.x, pos.x + w, pos.y + Sc(4), C().separator, std::max(1.0f, Sc(T().metrics.hairline)));
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
            const Color tint = o.kind == ButtonKind::Destructive ? (o.tint.a > 0 ? o.tint : AccentOr(c.red)) : Tint(o.tint);
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

        void DrawButton(const Interaction& it, const ButtonOptions& o, std::string_view label, Icon icon, bool circle)
        {
            const ButtonMetrics bm = MetricsFor(o.size);
            ButtonColors bc = ColorsFor(o);
            const Theme& t = T();
            if (ResolvedStyle().Has(ItemStyle::kFill))
                bc.bg = FillOr(bc.bg);
            // a flat button the style turns into glass: clear glass (labels that sat on a solid color take the label
            // color, legible over anything) or glass tinted by its color
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
            Painter p = GetPainter();
            // liquid glass bulges when pressed, solid buttons compress
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
                // hover lightens (dark) / dims (light), press fades
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
            {
                ScopedUnclip unclip(r, ShadowExtent(Sc(20), Vec2(0, Sc(5))));
                if (circle && !HasItemRadius())
                    p.Circle(r.Center(), r.Height() * 0.5f, s);
                else
                    p.Rect(r, s);
            }

            // content
            const text::FontRef f = Font(FontWeight::Semibold, bm.font);
            Color fg = LabelOr(bc.fg);
            if (o.kind == ButtonKind::Plain)
                fg = fg.Fade(1.0f - 0.4f * it.press);
            const float iconSize = f.size * 1.1f;
            const Vec2 ts = MeasureText(f, label);
            const float gap = (icon && !label.empty()) ? Sc(7) : 0.0f;
            const float iconW = icon ? iconSize : 0.0f;
            float x = r.Center().x - (iconW + gap + ts.x) * 0.5f;
            if (icon)
            {
                DrawIcon(p, Vec2(x + iconW * 0.5f, r.Center().y), icon, iconSize * 0.95f, fg);
                x += iconW + gap;
            }
            if (!label.empty())
                p.Text(Vec2(std::floor(x + 0.5f), std::floor(r.Center().y - ts.y * 0.5f + 0.5f)), f, fg, label);
            p.PopScale();
        }
    }

    bool Button(std::string_view label, const ButtonOptions& o)
    {
        ItemScope scope;   // takes a ui::Next() style
        const ButtonMetrics bm = MetricsFor(o.size);
        const std::string_view shown = VisibleLabel(label);
        const text::FontRef f = Font(FontWeight::Semibold, bm.font);
        const Vec2 ts = MeasureText(f, shown);
        const float iconW = o.icon ? f.size * 1.1f + (!shown.empty() ? Sc(7) : 0.0f) : 0.0f;
        float width = ts.x + iconW + Sc(bm.padX) * 2.0f;
        if (o.width > 0.0f)
            width = Sc(o.width);
        else if (o.width < 0.0f)
            width = AvailableWidth();
        const Interaction it = InteractImpl(Ctx().GetId(label), Rect::FromSize(Ctx().CursorPos(), Vec2(width, Sc(bm.height))), InteractFlags_None);
        Ctx().ItemSize(Vec2(width, Sc(bm.height)));
        if (!it.visible)
            return false;
        DrawButton(it, o, shown, o.icon, false);
        return it.pressed;
    }

    bool IconButton(std::string_view id, Icon icon, const ButtonOptions& o)
    {
        ItemScope scope;
        const float d = Sc(MetricsFor(o.size).height);
        const Interaction it = InteractImpl(Ctx().GetId(id), Rect::FromSize(Ctx().CursorPos(), Vec2(d, d)), InteractFlags_None);
        Ctx().ItemSize(Vec2(d, d));
        if (!it.visible)
            return false;
        DrawButton(it, o, {}, icon, true);
        return it.pressed;
    }

    // ================================================================ toggle
    Vec2 detail::ToggleSize() { return Vec2(Sc(50), Sc(30)); }

    bool detail::ToggleAt(Id id, const Rect& r, bool* value)
    {
        const Interaction it = InteractImpl(id, r, InteractFlags_None);
        if (!it.visible)
            return false;
        bool changed = false;
        if (it.pressed)
        {
            *value = !*value;
            changed = true;
        }
        const Palette& c = C();
        // position: under-damped, the knob overshoots and squashes against the end of the track; speed: it stretches
        // along its motion and thins (a liquid drop); press: it reaches toward where it will travel; lens: held or
        // just toggled, it swells into a clear lens (iOS 26); color: its own critically damped spring (no bounce)
        const Id posId = Salt(id, 0x10);
        static constexpr Spring kKnob{0.42f, 0.60f};
        const float pos = ui::Anim(posId, *value ? 1.0f : 0.0f, kKnob);
        const float vel = AnimVelocity(posId);
        const float colorT = Saturate(Anim(id, 0x11, *value ? 1.0f : 0.0f, Spring{0.30f, 1.0f}));
        const float lens = LiquidPulse(id, it.held, changed);

        const float pad = Sc(2);
        const float kd = r.Height() - pad * 2.0f;
        const float travel = r.Width() - pad * 2.0f - kd;
        const float overshoot = pos - Saturate(pos);
        const float squash = std::fabs(overshoot) * travel * 1.4f;
        const float speed = std::min(std::fabs(vel) * travel * 0.045f, kd * 0.55f);
        const float dir = *value ? -1.0f : 1.0f;
        const float elong = Sc(7) * it.press;

        float w = kd + speed + elong - squash + Sc(12) * lens;
        const float h = kd - std::min(speed * 0.14f, kd * 0.12f) + squash * 0.3f + Sc(8) * lens;
        const float cx = r.min.x + pad + kd * 0.5f + travel * Saturate(pos) + dir * elong * 0.5f - (overshoot > 0 ? 1.0f : -1.0f) * squash * 0.5f;
        w = std::max(w, kd * 0.7f);
        const Rect knob = Rect::FromCenter(Vec2(cx, r.Center().y), Vec2(w, h));

        Painter p = GetPainter();
        const Color onColor = AccentOr(c.green);
        DrawPill(p, r, Surface(Lerp(FillOr(c.secondaryFill), onColor, colorT)));
        if (GlassSurface() && colorT > 0.001f)
            DrawPill(p, r, Style().Fill(StateFill(onColor).Fade(colorT)));
        Style ks;
        ks.Radius(knob.Height() * 0.5f).Shadow(Color::Black(0.18f + 0.12f * lens), Sc(5 + 10 * lens), Vec2(0, Sc(2 + 2.5f * lens)));
        if (lens > 0.02f)
        {
            GlassMaterial lm = LensMaterial();
            lm.refraction *= 0.6f + 0.6f * Saturate(lens);
            ks.Glass(lm).Fill(c.controlKnob.Fade(1.0f - 0.9f * Saturate(lens)));
        }
        else
            ks.Fill(c.controlKnob);
        ScopedUnclip unclip(knob, Sc(24));
        p.Rect(knob, ks);
        return changed;
    }

    bool Toggle(std::string_view id, bool* value)
    {
        ItemScope scope;
        Context& c = Ctx();
        const Vec2 sz = ToggleSize();
        const Vec2 pos = c.CursorPos();
        c.ItemSize(sz);
        return ToggleAt(c.GetId(id), Rect::FromSize(pos, sz), value);
    }

    bool Checkbox(std::string_view label, bool* value)
    {
        ItemScope scope;
        Context& c = Ctx();
        const std::string_view shown = VisibleLabel(label);
        const text::FontRef f = Font(TextStyle::Body);
        const Vec2 ts = MeasureText(f, shown);
        const float d = Sc(22);
        const Vec2 size(d + (ts.x > 0 ? Sc(10) + ts.x : 0), std::max(d, ts.y));
        const Interaction it = InteractImpl(c.GetId(label), Rect::FromSize(c.CursorPos(), size), InteractFlags_None);
        c.ItemSize(size);
        if (!it.visible)
            return false;
        if (it.pressed)
            *value = !*value;
        const Palette& pc = C();
        // the fill cross-fades, the check mark draws itself, the dot pops on every click
        const float on = Saturate(Anim(it.id, 0x11, *value ? 1.0f : 0.0f, Spring{0.26f, 1.0f}));
        const float draw = Saturate(Anim(it.id, 0x12, *value ? 1.0f : 0.0f, Spring{0.36f, 0.92f}));
        const float pop = LiquidPulse(it.id, it.held, it.pressed, 0.12f);
        Painter p = GetPainter();
        const Vec2 cc(it.rect.min.x + d * 0.5f, it.rect.Center().y);
        p.PushScale(cc, 1.0f - 0.10f * it.press + 0.10f * pop * (1.0f - it.press));
        p.Circle(cc, d * 0.5f, Style().Stroke(Sc(1.6f), pc.tertiaryLabel.Fade(1.0f - on)).Fill(StateFill(Accent()).Fade(on)));
        if (draw > 0.01f)
        {
            const Vec2 a(cc.x - d * 0.22f, cc.y + d * 0.01f), b(cc.x - d * 0.05f, cc.y + d * 0.18f), e(cc.x + d * 0.24f, cc.y - d * 0.17f);
            const Style ls = Style().Fill(pc.onAccent.Fade(draw));
            p.Line(a, Lerp(a, b, std::min(1.0f, draw * 2.0f)), Sc(2.2f), ls);
            if (draw > 0.5f)
                p.Line(b, Lerp(b, e, (draw - 0.5f) * 2.0f), Sc(2.2f), ls);
        }
        p.PopScale();
        if (ts.x > 0)
            p.Text(Vec2(it.rect.min.x + d + Sc(10), std::floor(it.rect.Center().y - ts.y * 0.5f)), f, LabelOr(pc.label), shown);
        return it.pressed;
    }

    // ========================================================= toggle button
    // iOS Control Center toggle. Off: clear glass. Pressed: the glass sinks and lights up under the pointer. Released:
    // the tint floods in from the press point, the button pops on a spring and the symbol bounces. Switching off
    // drains the tint back into the press point.
    bool ToggleButton(std::string_view id, bool* value, Icon icon, const ToggleButtonOptions& o)
    {
        ItemScope scope;
        Context& c = Ctx();
        const float d = Sc(o.diameter);
        const Interaction it = InteractImpl(c.GetId(id), Rect::FromSize(c.CursorPos(), Vec2(d, d)), InteractFlags_None);
        c.ItemSize(Vec2(d, d));
        if (!it.visible)
            return false;
        const Theme& t = T();
        const Palette& pc = t.colors;
        const Color tint = Tint(o.tint);
        const Vec2 center = it.rect.Center();
        const float r = d * 0.5f;

        struct PressPoint
        {
            Vec2 at{0.0f, 0.0f};   // relative to the center, in radii
        };
        PressPoint& press = c.State<PressPoint>(it.id);
        if ((it.held || it.pressed) && c.Input().MouseValid())
        {
            Vec2 rel = (c.Input().MousePos() - center) * (1.0f / r);
            const float len = Length(rel);
            if (len > 0.8f)
                rel = rel * (0.8f / len);
            press.at = rel;
        }
        const Id popId = Salt(it.id, 0x72), bounceId = Salt(it.id, 0x73);
        if (it.pressed)
        {
            *value = !*value;
            AnimKick(popId, *value ? 26.0f : 16.0f);
            AnimKick(bounceId, 34.0f);
        }
        static constexpr Spring kFlood{0.34f, 0.95f}, kPop{0.34f, 0.46f}, kBounce{0.30f, 0.38f};
        const float on = Anim(it.id, 0x74, *value ? 1.0f : 0.0f, kFlood);
        const float pop = ui::Anim(popId, 0.0f, kPop);
        const float bounce = ui::Anim(bounceId, 0.0f, kBounce);
        const float light = Anim(it.id, 0x75, it.held ? 1.0f : 0.0f, Spring{0.18f, 1.0f});
        const float k = Saturate(on);

        Painter p = GetPainter();
        p.PushScale(center, 1.0f - 0.07f * it.press + 0.06f * pop);
        Style base = Style().Glass(LookMaterial(t.materials.control)).Shadow(pc.shadow.Fade(0.6f), Sc(12), Vec2(0, Sc(4)));
        if (!LookClear())
            base.Fill(FillOr(pc.fill).Fade(1.0f - 0.6f * k));
        if (o.glow && k > 0.01f)
            base.Glow(tint, Sc(12), 0.45f * k);
        {
            ScopedUnclip unclip(it.rect, ShadowExtent(Sc(12), Vec2(0, Sc(4))));
            p.Circle(center, r, base);
        }

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
        if (light > 0.01f)   // touch illumination under the pointer
            p.Circle(origin, r * 0.9f, Style().Fill(Paint::Radial(Color::White(0.45f * light), Color::White(0.0f))));
        p.PopMask();

        DrawIcon(p, center, icon, d * 0.40f * (1.0f + 0.12f * bounce), Lerp(LabelOr(pc.label), Color::White(), k));
        p.PopScale();
        return it.pressed;
    }

    // ================================================================ slider
    bool detail::SliderAt(Id id, const Rect& r, float* value, float mn, float mx, const SliderOptions& o)
    {
        const Palette& pc = C();
        const float iconSize = Sc(15);
        const float iconPad = Sc(26);
        Rect area = r;
        if (o.minIcon)
            area.min.x += iconPad;
        if (o.maxIcon)
            area.max.x -= iconPad;

        const Interaction it = InteractImpl(id, r, InteractFlags_PressOnClick);
        if (!it.visible)
            return false;

        const float kwRest = Sc(34), khRest = Sc(22);
        const float x0 = area.min.x + kwRest * 0.5f, x1 = std::max(area.max.x - kwRest * 0.5f, x0);   // never inverted
        bool changed = false;
        const InputState& in = Ctx().Input();
        if (it.held && mx > mn && in.MouseValid())
        {
            const float t = Saturate((in.MousePos().x - x0) / std::max(x1 - x0, 1.0f));
            float nv = mn + t * (mx - mn);
            if (o.step > 0.0f)
                nv = mn + std::round((nv - mn) / o.step) * o.step;
            nv = Clamp(nv, mn, mx);
            if (nv != *value)
            {
                *value = nv;
                changed = true;
            }
        }
        const float frac = mx > mn ? Saturate((*value - mn) / (mx - mn)) : 0.0f;
        // dragging follows the pointer tightly, jumps (a click on the track) glide on a softer spring
        const Id shownId = Salt(id, 0x20);
        static constexpr Spring kFollow{0.10f, 1.0f}, kGlide{0.38f, 0.78f};
        const float shown = ui::Anim(shownId, frac, it.held ? kFollow : kGlide);
        const float vel = AnimVelocity(shownId);
        const float press = LiquidPulse(id, it.held, it.pressed);

        Painter p = GetPainter();
        const float cy = r.Center().y;
        const float th = Sc(6);
        const Rect track(area.min.x, cy - th * 0.5f, area.max.x, cy + th * 0.5f);
        const Color tint = Tint(o.tint);
        DrawPill(p, track, Surface(FillOr(pc.fill)));
        const float kx = Lerp(x0, x1, shown);
        DrawPill(p, Rect(track.min.x, track.min.y, std::max(kx, track.min.x + th), track.max.y), Style().Fill(StateFill(tint)));

        if (o.minIcon)
            DrawIcon(p, Vec2(r.min.x + iconPad * 0.4f, cy), o.minIcon, iconSize, pc.secondaryLabel);
        if (o.maxIcon)
            DrawIcon(p, Vec2(r.max.x - iconPad * 0.4f, cy), o.maxIcon, iconSize * 1.15f, pc.secondaryLabel);

        // the lens swells when grabbed, stretches along fast motion and thins a little (a liquid drop)
        const float speed = std::min(std::fabs(vel) * (x1 - x0) * 0.03f, kwRest * 0.6f);
        const float kw = Lerp(kwRest, Sc(46), press) + speed;
        const float kh = Lerp(khRest, Sc(30), press) - std::min(speed * 0.12f, Sc(3));
        const Rect knob = Rect::FromCenter(Vec2(kx, cy), Vec2(kw, kh));
        Style ks;
        ks.Radius(kh * 0.5f).Shadow(Color::Black(0.18f + 0.1f * press), Sc(6 + 10 * press), Vec2(0, Sc(2 + 2 * press)));
        if (press > 0.02f)
            ks.Glass(LensMaterial()).Fill(pc.controlKnob.Fade(1.0f - 0.9f * Saturate(press)));
        else
            ks.Fill(pc.controlKnob);
        ScopedUnclip unclip(knob, Sc(28));
        p.Rect(knob, ks);
        return changed;
    }

    bool Slider(std::string_view id, float* value, float mn, float mx, const SliderOptions& o)
    {
        ItemScope scope;
        Context& c = Ctx();
        const float width = o.width > 0.0f ? Sc(o.width) : AvailableWidth();
        const Vec2 size(width, Sc(32));
        const Vec2 pos = c.CursorPos();
        c.ItemSize(size);
        return SliderAt(c.GetId(id), Rect::FromSize(pos, size), value, mn, mx, o);
    }

    // =============================================================== stepper
    Vec2 detail::StepperSize() { return Vec2(Sc(96), Sc(32)); }

    bool detail::StepperAt(Id id, const Rect& r, int* value, int mn, int mx, int step)
    {
        const Palette& pc = C();
        Painter p = GetPainter();
        const float radius = ItemRadius(Sc(9));
        Style well = Style().Radius(radius);
        p.Rect(r, SurfaceFill(well, FillOr(pc.tertiaryFill)));
        const Rect left(r.min.x, r.min.y, r.Center().x, r.max.y), right(r.Center().x, r.min.y, r.max.x, r.max.y);
        bool changed = false;
        const Interaction a = InteractImpl(Salt(id, 1), left, InteractFlags_Repeat);
        const Interaction b = InteractImpl(Salt(id, 2), right, InteractFlags_Repeat);
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
        if (a.press > 0.01f)
            p.Rect(left, Style().Radius(radius, 0, 0, radius).Fill(pc.highlight.Fade(a.press * 1.5f)));
        if (b.press > 0.01f)
            p.Rect(right, Style().Radius(0, radius, radius, 0).Fill(pc.highlight.Fade(b.press * 1.5f)));
        const float mid = r.Center().x;
        p.FillRect(Rect(mid - 0.5f, r.min.y + r.Height() * 0.25f, mid + 0.5f, r.max.y - r.Height() * 0.25f), pc.separator);
        const float is = Sc(14);
        const Color fg = LabelOr(pc.label);
        DrawIcon(p, left.Center(), icons::Remove, is, canDec ? fg : fg.Fade(0.35f));
        DrawIcon(p, right.Center(), icons::Add, is, canInc ? fg : fg.Fade(0.35f));
        return changed;
    }

    bool Stepper(std::string_view id, int* value, int mn, int mx, int step)
    {
        ItemScope scope;
        Context& c = Ctx();
        const Vec2 sz = StepperSize();
        const Vec2 pos = c.CursorPos();
        c.ItemSize(sz);
        return StepperAt(c.GetId(id), Rect::FromSize(pos, sz), value, mn, mx, step);
    }

    // ============================================================== progress
    namespace
    {
        // An id for a widget without one (progress): its kind under the id stack and its place in the frame's order of
        // such widgets, which stays the same from frame to frame. WGT keyed it by the screen position, which broke the
        // animation whenever the content scrolled.
        Id DecorationId(std::string_view kind) { return Salt(Ctx().GetId(kind), M().decorationSerial++); }
    }

    void ProgressBar(float fraction, const ProgressOptions& o)
    {
        ItemScope scope;
        Context& c = Ctx();
        const float w = o.width > 0.0f ? Sc(o.width) : AvailableWidth();
        const Vec2 pos = c.CursorPos();
        const Vec2 size(w, Sc(6));
        c.ItemSize(size);
        const Id id = DecorationId("##progress");
        if (!c.ItemAdd(0, Rect::FromSize(pos, size)))
            return;
        const float shown = Anim(id, 0x70, Saturate(fraction), SpringStd());
        Painter p = GetPainter();
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

    void ProgressRing(float fraction, const ProgressOptions& o)
    {
        ItemScope scope;
        Context& c = Ctx();
        const float d = o.diameter > 0.0f ? Sc(o.diameter) : Sc(44);
        const float th = o.thickness > 0.0f ? Sc(o.thickness) : std::max(Sc(3), d * 0.12f);
        const Vec2 pos = c.CursorPos();
        c.ItemSize(Vec2(d, d));
        const Id id = DecorationId("##ring");
        if (!c.ItemAdd(0, Rect::FromSize(pos, Vec2(d, d))))
            return;
        const float shown = Anim(id, 0x71, Saturate(fraction), SpringStd());
        const Vec2 center(pos.x + d * 0.5f, pos.y + d * 0.5f);
        const float rad = (d - th) * 0.5f;
        const Color t = Tint(o.tint);
        Painter p = GetPainter();
        p.Ring(center, rad, th, Style().Fill(t.Fade(0.18f)));
        if (shown > 0.001f)
        {
            Style s = Style().Fill(Paint::Conic(StateFill(t.Lighter(0.35f)), StateFill(t), -90));
            if (o.glow)
                s.Glow(t, Sc(5), 0.4f);
            p.Arc(center, rad, th, -kPi * 0.5f, kTau * shown, s);
        }
    }

    void ActivityIndicator(float diameter, Color tint)
    {
        ItemScope scope;
        Context& c = Ctx();
        const float d = diameter > 0.0f ? Sc(diameter) : Sc(24);
        const Vec2 pos = c.CursorPos();
        c.ItemSize(Vec2(d, d));
        if (!c.ItemAdd(0, Rect::FromSize(pos, Vec2(d, d))))
            return;
        M().animating = true;   // it spins as long as it is shown
        const Color col = tint.a > 0 ? tint : C().secondaryLabel;
        const Vec2 center(pos.x + d * 0.5f, pos.y + d * 0.5f);
        const float t = (float)std::fmod(Time(), 1.0);
        constexpr int kSpokes = 8;
        Painter p = GetPainter();
        for (int i = 0; i < kSpokes; ++i)
        {
            const float a = kTau * (float)i / kSpokes - kPi * 0.5f;
            const float phase = std::fmod((float)i / kSpokes - t + 1.0f, 1.0f);
            const float alpha = 0.18f + 0.82f * phase * phase;
            const Vec2 dir(std::cos(a), std::sin(a));
            p.Line(center + dir * (d * 0.24f), center + dir * (d * 0.44f), d * 0.09f, Style().Fill(col.Fade(alpha)));
        }
    }

    void Badge(std::string_view text, Color tint)
    {
        ItemScope scope;
        Context& c = Ctx();
        const text::FontRef f = Font(FontWeight::Semibold, 11.5f);
        const Vec2 ts = MeasureText(f, text);
        const Vec2 size(std::max(ts.x + Sc(14), Sc(20)), Sc(20));
        const Vec2 pos = c.CursorPos();
        c.ItemSize(size);
        if (!c.ItemAdd(0, Rect::FromSize(pos, size)))
            return;
        const Color t = tint.a > 0 ? tint : FillOr(AccentOr(C().red));   // the explicit color wins over the style
        Painter p = GetPainter();
        const Rect r = Rect::FromSize(pos, size);
        const bool clear = LookClear();   // transparent: clear glass, the text takes the badge color
        DrawPill(p, r, Surface(t));
        p.Text(Vec2(std::floor(r.Center().x - ts.x * 0.5f), std::floor(r.Center().y - ts.y * 0.5f)), f, LabelOr(clear ? t : Color::White()), text);
    }

    void Image(TextureId texture, Vec2 size, float radius, Vec2 uv0, Vec2 uv1)
    {
        Context& c = Ctx();
        const Vec2 sz(Sc(size.x), Sc(size.y));
        const Vec2 pos = c.CursorPos();
        c.ItemSize(sz);
        if (!c.ItemAdd(0, Rect::FromSize(pos, sz)))
            return;
        Painter p = GetPainter();
        p.Image(texture, Rect::FromSize(pos, sz), radius < 0.0f ? Sc(T().metrics.controlRadius) : Sc(radius), Color::White(), uv0, uv1);
    }

    bool ColorSwatches(std::string_view id, int* selected, const Color* colors, int count, float diameter)
    {
        ItemScope scope;
        Context& c = Ctx();
        const float d = diameter > 0.0f ? Sc(diameter) : Sc(28);
        const float gap = Sc(10);
        const Vec2 pos = c.CursorPos();
        const Vec2 size((float)count * d + (float)std::max(count - 1, 0) * gap, d + Sc(6));
        c.ItemSize(size);
        c.PushId(id);
        bool changed = false;
        Painter p = GetPainter();
        for (int i = 0; i < count; ++i)
        {
            const Vec2 center(pos.x + (float)i * (d + gap) + d * 0.5f, pos.y + size.y * 0.5f);
            const Interaction it = InteractImpl(c.GetId(i), Rect::FromCenter(center, Vec2(d, d)), InteractFlags_None);
            if (it.pressed && *selected != i)
            {
                *selected = i;
                changed = true;
            }
            const float on = Anim(it.id, 0x80, *selected == i ? 1.0f : 0.0f, SpringBouncy());
            const float r = d * 0.5f * (1.0f - 0.08f * it.press);
            p.Circle(center, r - Sc(3) * on, Style().Fill(Paint::Radial(colors[i].Lighter(0.18f), colors[i], Vec2(0.35f, 0.3f), 0.8f)));
            if (on > 0.01f)
                p.Circle(center, r + Sc(1.5f), Style().Stroke(Sc(2), colors[i].Fade(on), 0.0f));
        }
        c.PopId();
        return changed;
    }

    // ============================================================ icon tile
    void detail::IconTile(Painter& p, const Rect& r, Icon icon, Color color)
    {
        const Color base = Tint(color);
        p.Rect(r, Style().Radius(r.Height() * 0.26f).Fill(Paint::Linear(StateFill(base.Lighter(0.14f)), StateFill(base.Darker(0.06f)), 90)));
        DrawIcon(p, r.Center(), icon, r.Height() * 0.56f, Color::White());
    }
}
