// Esia UI - number fields: a value scrubbed by dragging the field, typed (with arithmetic) after a click, stepped with
// Up / Down or the - / + buttons; vector fields side by side.
#include "ui_internal.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace esia::ui
{
    using namespace detail;

    namespace
    {
        // ---- typed values: + - * / and parentheses over numbers; a unit after the expression is ignored
        struct Parser
        {
            std::string_view s;
            std::size_t i = 0;
            bool ok = true;

            void Skip()
            {
                while (i < s.size() && (s[i] == ' ' || s[i] == '\t'))
                    ++i;
            }
            double Factor()
            {
                Skip();
                if (i < s.size() && (s[i] == '-' || s[i] == '+'))
                {
                    const bool neg = s[i++] == '-';
                    const double v = Factor();
                    return neg ? -v : v;
                }
                if (i < s.size() && s[i] == '(')
                {
                    ++i;
                    const double v = Expr();
                    Skip();
                    if (i < s.size() && s[i] == ')')
                        ++i;
                    else
                        ok = false;
                    return v;
                }
                // a number: digits, one point (or a comma, as many keyboards type it), an exponent
                std::string num;
                while (i < s.size() && (std::isdigit((unsigned char)s[i]) || s[i] == '.' || s[i] == ','))
                    num += s[i] == ',' ? '.' : s[i], ++i;
                if (i < s.size() && (s[i] == 'e' || s[i] == 'E') && !num.empty() && i + 1 < s.size() &&
                    (std::isdigit((unsigned char)s[i + 1]) || ((s[i + 1] == '-' || s[i + 1] == '+') && i + 2 < s.size() && std::isdigit((unsigned char)s[i + 2]))))
                {
                    num += s[i++];
                    num += s[i++];
                    while (i < s.size() && std::isdigit((unsigned char)s[i]))
                        num += s[i++];
                }
                char* end = nullptr;
                const double v = std::strtod(num.c_str(), &end);
                if (num.empty() || end != num.c_str() + num.size())
                    ok = false;
                return v;
            }
            double Term()
            {
                double v = Factor();
                for (;;)
                {
                    Skip();
                    if (i < s.size() && (s[i] == '*' || s[i] == '/'))
                    {
                        const char op = s[i++];
                        const double r = Factor();
                        v = op == '*' ? v * r : v / r;
                    }
                    else
                        return v;
                }
            }
            double Expr()
            {
                double v = Term();
                for (;;)
                {
                    Skip();
                    if (i < s.size() && (s[i] == '+' || s[i] == '-'))
                    {
                        const char op = s[i++];
                        const double r = Term();
                        v = op == '+' ? v + r : v - r;
                    }
                    else
                        return v;
                }
            }
        };

        bool Evaluate(std::string_view text, double& out)
        {
            Parser p{text};
            const double v = p.Expr();
            p.Skip();
            // what follows may be a unit ("m/s", "%", "px"), never more arithmetic
            for (std::size_t k = p.i; k < text.size(); ++k)
                if (std::isdigit((unsigned char)text[k]) || text[k] == '(' || text[k] == ')')
                    return false;
            if (!p.ok || !std::isfinite(v))
                return false;
            out = v;
            return true;
        }

        // "%.3f" without the zeros it does not need: 1.250 -> 1.25, 2.000 -> 2
        std::string FormatValue(double v, const char* format, bool integer)
        {
            char buf[96];
            if (format && *format)
            {
                if (integer)
                    std::snprintf(buf, sizeof(buf), format, (int)std::lround(v));
                else
                    std::snprintf(buf, sizeof(buf), format, v);
                return buf;
            }
            if (integer)
            {
                std::snprintf(buf, sizeof(buf), "%d", (int)std::lround(v));
                return buf;
            }
            std::snprintf(buf, sizeof(buf), "%.3f", v);
            std::string s = buf;
            if (s.find('.') != std::string::npos)
            {
                while (!s.empty() && s.back() == '0')
                    s.pop_back();
                if (!s.empty() && s.back() == '.')
                    s.pop_back();
            }
            if (s == "-0")
                s = "0";
            return s;
        }

        struct NumberState
        {
            bool editing = false;
            bool refocus = false;      // Up / Down while typing: the field takes the keyboard again next frame
            std::string text;          // what is typed
            bool pressed = false;      // the mouse went down on the field (a release then is a click or the end of a drag)
            bool dragging = false;     // scrubbing (past the threshold)
            float startX = 0.0f;
            double startValue = 0.0;
        };

        double DefaultStep(bool integer, const NumberOptions& o)
        {
            if (o.step > 0.0)
                return o.step;
            if (integer)
                return 1.0;
            if (std::isfinite(o.min) && std::isfinite(o.max) && o.max > o.min)
                return (o.max - o.min) / 100.0;
            return 0.1;
        }

        double Snap(double v, bool integer, const NumberOptions& o)
        {
            if (integer)
                v = std::round(v);
            return std::clamp(v, o.min, o.max);
        }

        bool NumberAt(Id id, const Rect& r, double* value, bool integer, const NumberOptions& o)
        {
            Ui::Impl& m = M();
            Context& c = *m.ctx;
            const InputState& in = c.Input();
            const Palette& pc = C();
            NumberState& s = c.State<NumberState>(id);
            const Id editId = Salt(id, 0xED17);
            const double step = DefaultStep(integer, o);
            const bool ranged = std::isfinite(o.min) && std::isfinite(o.max) && o.max > o.min;
            bool changed = false;
            const auto set = [&](double v) {
                v = Snap(v, integer, o);
                if (v != *value)
                {
                    *value = v;
                    changed = true;
                }
            };

            const float h = r.Height();
            const float bw = o.buttons ? h : 0.0f;   // the - and + buttons are square
            const Rect field(r.min.x + bw, r.min.y, r.max.x - bw, r.max.y);
            const float radius = ItemRadius(Sc(9));
            Painter p = GetPainter();

            if (s.refocus)
            {
                c.SetKeyboardFocusId(editId);
                s.refocus = false;
                s.editing = true;
            }
            if (s.editing)
            {
                // Up / Down step the typed value: it is taken, stepped and typed again
                if (c.KeyPressed(Key::Up, editId) || c.KeyPressed(Key::Down, editId))
                {
                    double v = *value;
                    Evaluate(s.text, v);
                    const double mul = (in.Mods() & Mod_Shift) ? 10.0 : 1.0;
                    set(v + (c.KeyPressed(Key::Up, editId) ? step : -step) * mul);
                    s.text = FormatValue(*value, nullptr, integer);
                    c.SetKeyboardFocusId(0);
                    s.refocus = true;
                }
                TextFieldOptions to;
                to.clearButton = false;
                to.selectOnFocus = true;
                to.background = true;
                const TextFieldResult tr = TextFieldAt(editId, field, &s.text, {}, to);
                const bool escape = in.KeyPressed(Key::Escape);
                if (!s.refocus && c.KeyboardFocusId() != editId)
                {
                    // Enter or a click elsewhere takes the value, Escape keeps the old one
                    double v = *value;
                    if (!escape && Evaluate(s.text, v))
                        set(v);
                    s.editing = false;
                }
                (void)tr;
            }
            else
            {
                const ButtonResult b = c.ItemAdd(id, field) ? c.ButtonBehavior(id, field, ButtonFlags_PressOnClick) : ButtonResult{};
                const float hover = Anim(id, 0xA1, b.hovered || b.held ? 1.0f : 0.0f, SpringFast());
                if (b.pressed)
                {
                    s.startX = in.MousePos().x;
                    s.startValue = *value;
                    s.dragging = false;
                    s.pressed = true;
                }
                if (b.held)
                {
                    const float dx = in.MousePos().x - s.startX;
                    const float threshold = Sc(3);
                    if (!s.dragging && std::fabs(dx) > threshold)
                        s.dragging = true;
                    if (s.dragging)
                    {
                        double speed = o.speed > 0.0 ? o.speed : (ranged ? (o.max - o.min) / std::max(Sc(240), field.Width()) : step / Sc(4));
                        if (in.Mods() & Mod_Shift)
                            speed *= 0.1;   // fine
                        double v = s.startValue + (double)(dx - (dx > 0 ? threshold : -threshold)) * speed;
                        if (!integer && o.step > 0.0)
                            v = std::round(v / o.step) * o.step;
                        set(v);
                    }
                }
                else if (s.pressed)
                {
                    // let go: the end of a drag, or a click, which types a value
                    if (!s.dragging && b.hovered)
                    {
                        s.editing = true;
                        s.text = FormatValue(*value, nullptr, integer);
                        c.SetKeyboardFocusId(editId);
                    }
                    s.pressed = false;
                    s.dragging = false;
                }
                if (b.hovered || s.dragging)
                    c.SetMouseCursor(MouseCursor::ResizeEW);

                // the field: a well, filled in proportion to the value when the range is finite
                Style ws = Style().Radius(radius);
                SurfaceFill(ws, FillOr(pc.tertiaryFill));
                p.Rect(field, ws);
                if (ranged)
                {
                    const double frac = std::clamp((*value - o.min) / (o.max - o.min), 0.0, 1.0);
                    const float shown = Anim(id, 0xF1, (float)frac, s.dragging ? Spring{0.08f, 1.0f} : SpringStd());
                    if (shown > 0.001f)
                    {
                        p.PushMask(field, radius);
                        p.FillRect(Rect(field.min.x, field.min.y, field.min.x + field.Width() * shown, field.max.y), Accent().Fade(0.22f + 0.08f * hover));
                        p.PopMask();
                    }
                }
                if (hover > 0.01f)
                    p.Rect(field, Style().Radius(radius).Fill(pc.highlight.Fade(hover * 0.6f)));

                // the label on the left, the value in the middle
                const text::FontRef f = Font(TextStyle::Callout);
                float textLeft = field.min.x + Sc(8);
                if (!o.label.empty())
                {
                    const text::FontRef lf = Font(FontWeight::Semibold, T().type.size[(int)TextStyle::Footnote]);
                    const Vec2 ls = MeasureText(lf, o.label);
                    p.Text(Vec2(textLeft, std::floor(field.Center().y - ls.y * 0.5f + 0.5f)), lf, o.labelColor.a > 0 ? o.labelColor : pc.secondaryLabel, o.label);
                    textLeft += ls.x + Sc(6);
                }
                const std::string shown = FormatValue(*value, o.format, integer);
                const Vec2 ts = MeasureText(f, shown);
                const float right = field.max.x - Sc(8);
                const float x = o.label.empty() ? std::max(textLeft, field.Center().x - ts.x * 0.5f) : std::max(textLeft, right - ts.x);
                p.TextBox(Rect(x, std::floor(field.Center().y - ts.y * 0.5f + 0.5f), right, std::floor(field.Center().y - ts.y * 0.5f + 0.5f) + ts.y), Vec2(0, 0), f,
                          LabelOr(pc.label), shown, text::TextFlags_Ellipsis);
            }

            if (o.buttons)
            {
                const Rect left(r.min.x, r.min.y, r.min.x + bw, r.max.y), right(r.max.x - bw, r.min.y, r.max.x, r.max.y);
                const InteractState a = InteractImpl(Salt(id, 1), left, InteractFlags_Repeat);
                const InteractState b = InteractImpl(Salt(id, 2), right, InteractFlags_Repeat);
                const double mul = (in.Mods() & Mod_Shift) ? 10.0 : 1.0;
                if (a.pressed)
                    set(*value - step * mul);
                if (b.pressed)
                    set(*value + step * mul);
                const Color fg = LabelOr(pc.label);
                for (const InteractState* it : {&a, &b})
                {
                    const Rect br = it->rect.Expanded(-Sc(2));
                    p.Circle(br.Center(), br.Height() * 0.5f, Surface(pc.fill.Fade(0.6f + 0.5f * it->hover + 0.6f * it->press)));
                }
                DrawIcon(p, left.Center(), icons::Remove, Sc(12), *value > o.min ? fg : fg.Fade(0.35f));
                DrawIcon(p, right.Center(), icons::Add, Sc(12), *value < o.max ? fg : fg.Fade(0.35f));
            }
            return changed;
        }

        Rect TakeRow(const NumberOptions& o)
        {
            Context& c = Ctx();
            const float width = o.width > 0.0f ? Sc(o.width) : AvailableWidth();
            const Vec2 pos = c.CursorPos();
            const Vec2 size(width, Sc(Sizes().field));
            c.ItemSize(size);
            return Rect::FromSize(pos, size);
        }
    }

    bool NumberField(std::string_view id, double* value, const NumberOptions& o)
    {
        ItemScope scope;
        const Rect r = TakeRow(o);
        return value && NumberAt(Ctx().GetId(id), r, value, false, o);
    }

    bool NumberField(std::string_view id, float* value, const NumberOptions& o)
    {
        ItemScope scope;
        const Rect r = TakeRow(o);
        if (!value)
            return false;
        double v = *value;
        const bool changed = NumberAt(Ctx().GetId(id), r, &v, false, o);
        if (changed)
            *value = (float)v;
        return changed;
    }

    bool NumberField(std::string_view id, int* value, const NumberOptions& o)
    {
        ItemScope scope;
        const Rect r = TakeRow(o);
        if (!value)
            return false;
        double v = *value;
        const bool changed = NumberAt(Ctx().GetId(id), r, &v, true, o);
        if (changed)
            *value = (int)std::lround(v);
        return changed;
    }

    bool VectorField(std::string_view id, float* values, int count, const NumberOptions& o)
    {
        ItemScope scope;
        count = std::clamp(count, 1, 4);
        const Rect r = TakeRow(o);
        if (!values)
            return false;
        static constexpr std::string_view kLabels[] = {"X", "Y", "Z", "W"};
        const Palette& pc = C();
        const Color colors[] = {pc.red, pc.green, pc.blue, pc.gray};
        const float gap = Sc(6);
        const float w = (r.Width() - gap * (float)(count - 1)) / (float)count;
        const Id base = Ctx().GetId(id);
        bool changed = false;
        for (int i = 0; i < count; ++i)
        {
            NumberOptions oi = o;
            oi.label = o.label.empty() ? kLabels[i] : o.label;
            oi.labelColor = o.labelColor.a > 0 ? o.labelColor : colors[i];
            oi.buttons = false;
            double v = values[i];
            const Rect cell = Rect::FromSize(Vec2(r.min.x + (w + gap) * (float)i, r.min.y), Vec2(w, r.Height()));
            if (NumberAt(Salt(base, (std::uint32_t)i + 1), cell, &v, false, oi))
            {
                values[i] = (float)v;
                changed = true;
            }
        }
        return changed;
    }
}
