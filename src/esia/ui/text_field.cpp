// Esia UI - the text field: WGT's single-line editor (text_edit.cpp) on the core's keyboard focus, typed text and IME
// composition, with the caret stops of the text system (TextSystem::CaretStops: graphemes, right-to-left runs).
//
// It edits a copy of the text while it has the keyboard and writes every change back at once. Offsets are UTF-8
// bytes and always sit on caret stops.
#include "ui_internal.hpp"
#include <algorithm>

namespace esia::ui
{
    using namespace detail;

    namespace
    {
#if defined(__APPLE__)
        constexpr std::uint32_t kShortcutMod = Mod_Super;   // Cmd+C, Cmd+V ...
        constexpr std::uint32_t kWordMod = Mod_Alt;         // Option+arrows move by words
#else
        constexpr std::uint32_t kShortcutMod = Mod_Ctrl;
        constexpr std::uint32_t kWordMod = Mod_Ctrl;
#endif
        constexpr std::string_view kBullet = "\xE2\x80\xA2";   // U+2022, what a password shows per grapheme

        bool IsSpace(char32_t c) { return c == U' ' || c == U'\t' || c == 0x00A0 || c == 0x3000 || (c >= 0x2000 && c <= 0x200B); }

        // The caret stops of a string, and for each whether the grapheme after it is white space (word moves).
        struct CaretMap
        {
            std::string text;
            text::FontRef font;
            std::vector<text::CaretStop> stops;
            std::vector<std::uint8_t> space;
            bool valid = false;
        };

        const CaretMap& Geometry(CaretMap& m, text::FontRef f, std::string_view text)
        {
            if (m.valid && m.text == text && m.font.id == f.id && m.font.size == f.size)
                return m;
            m.text.assign(text);
            m.font = f;
            m.valid = true;
            if (text::TextSystem* ts = M().text; ts && f.id != 0)
                ts->CaretStops(f, text, m.stops);
            else
            {
                // no text system (tests): a stop at every code point, all at x 0
                m.stops.clear();
                for (std::size_t i = 0;;)
                {
                    m.stops.push_back({(std::uint32_t)i, 0.0f});
                    if (i >= text.size())
                        break;
                    DecodeUtf8(text, i);
                }
            }
            m.space.assign(m.stops.size(), 0);
            for (std::size_t i = 0; i + 1 < m.stops.size(); ++i)
            {
                std::size_t k = m.stops[i].offset;
                m.space[i] = IsSpace(DecodeUtf8(text, k)) ? 1 : 0;
            }
            return m;
        }

        // index of the last caret stop <= pos
        std::size_t StopIndex(const CaretMap& m, std::uint32_t pos)
        {
            std::size_t i = 0;
            while (i + 1 < m.stops.size() && m.stops[i + 1].offset <= pos)
                ++i;
            return i;
        }
        std::uint32_t PrevStop(const CaretMap& m, std::uint32_t pos)
        {
            const std::size_t i = StopIndex(m, pos);
            return m.stops[(m.stops[i].offset == pos && i > 0) ? i - 1 : i].offset;
        }
        std::uint32_t NextStop(const CaretMap& m, std::uint32_t pos) { return m.stops[std::min(StopIndex(m, pos) + 1, m.stops.size() - 1)].offset; }
        std::uint32_t WordLeft(const CaretMap& m, std::uint32_t pos)
        {
            std::size_t i = StopIndex(m, pos);
            while (i > 0 && m.space[i - 1])
                --i;
            while (i > 0 && !m.space[i - 1])
                --i;
            return m.stops[i].offset;
        }
        std::uint32_t WordRight(const CaretMap& m, std::uint32_t pos)
        {
            std::size_t i = StopIndex(m, pos);
            const std::size_t last = m.stops.size() - 1;
            while (i < last && !m.space[i])
                ++i;
            while (i < last && m.space[i])
                ++i;
            return m.stops[i].offset;
        }
        float CaretX(const CaretMap& m, std::uint32_t pos) { return m.stops[StopIndex(m, pos)].x; }
        float Width(const CaretMap& m)
        {
            float w = 0.0f;
            for (const text::CaretStop& s : m.stops)
                w = std::max(w, s.x);
            return w;
        }
        std::size_t HitIndex(const CaretMap& m, float x)
        {
            std::size_t best = 0;
            float bestD = 1e30f;
            for (std::size_t i = 0; i < m.stops.size(); ++i)
            {
                const float d = std::fabs(m.stops[i].x - x);
                if (d < bestD)
                {
                    bestD = d;
                    best = i;
                }
            }
            return best;
        }

        std::string Bullets(std::size_t n)
        {
            std::string s;
            s.reserve(n * kBullet.size());
            for (std::size_t i = 0; i < n; ++i)
                s += kBullet;
            return s;
        }

        struct EditState
        {
            std::string text;                      // the working copy while focused
            std::uint32_t caret = 0, anchor = 0;   // bytes, on caret stops
            float scroll = 0.0f;
            float blink = 0.0f;                    // seconds since the caret last moved
            bool dragging = false;
            bool wasActive = false;
            struct Snapshot
            {
                std::string text;
                std::uint32_t caret = 0, anchor = 0;
            };
            std::vector<Snapshot> undo, redo;
            double lastTyping = -10.0;
            CaretMap real, shown;                  // the caret stops of the text, and of what is displayed
        };
    }

    TextFieldResult detail::TextFieldAt(Id fid, const Rect& r, std::string* value, std::string_view placeholder, const TextFieldOptions& o)
    {
        TextFieldResult result;
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const InputState& in = c.Input();
        const Palette& pc = C();
        if (!value || !c.ItemAdd(fid, r, ItemFlags_Focusable))
            return result;
        EditState& s = c.State<EditState>(fid);
        const text::FontRef f = Font(TextStyle::Body);
        const ButtonResult b = c.ButtonBehavior(fid, r, ButtonFlags_PressOnClick, ItemFlags_Focusable);

        // ---------------------------------------------------------- focus
        if (b.pressed && c.KeyboardFocusId() != fid)
            c.SetKeyboardFocusId(fid);
        bool active = c.KeyboardFocusId() == fid;
        bool focusedNow = false;   // this frame (the click that focused it keeps a select-on-focus selection)
        if (active && !s.wasActive)
        {
            focusedNow = true;
            // focused (a click, or Tab): edit a copy, the caret at the end (a click moves it below)
            s.text = *value;
            s.caret = s.anchor = (std::uint32_t)s.text.size();
            if (o.selectOnFocus)
                s.anchor = 0;
            s.undo.clear();
            s.redo.clear();
            s.blink = 0.0f;
        }
        if (!active)
        {
            s.text = *value;   // changes from outside show at once
            s.dragging = false;
        }

        // layout: [icon] text [clear]
        const float padX = Sc(12);
        const float x0 = r.min.x + padX + (o.icon ? Sc(24) : 0.0f);
        const bool showClear = o.clearButton && !s.text.empty();
        const float innerW = std::max(1.0f, r.max.x - padX - (showClear ? Sc(26) : 0.0f) - x0);
        const float lineH = std::max(MeasureText(f, "Hg").y, 1.0f);
        const float textTop = std::floor(r.Center().y - lineH * 0.5f + 0.5f);

        // ---------------------------------------------------------- editing
        std::string comp;
        int compCursor = 0, compTargetA = 0, compTargetB = 0;
        if (active)
        {
            // the keys the field uses are its own (Tab still moves the focus)
            static constexpr Key kKeys[] = {Key::Left, Key::Right, Key::Up, Key::Down, Key::Home, Key::End, Key::Backspace, Key::Delete, Key::Enter,
                                            Key::Escape, Key::A, Key::C, Key::X, Key::V, Key::Y, Key::Z};
            for (const Key k : kKeys)
                c.ClaimKey(k, fid);
            s.blink += DeltaTime();
            m.animating = true;   // the caret blinks

            const auto selMin = [&] { return std::min(s.caret, s.anchor); };
            const auto selMax = [&] { return std::max(s.caret, s.anchor); };
            const auto pushUndo = [&](bool typing) {
                const double now = Time();
                if (!(typing && now - s.lastTyping < 1.0 && !s.undo.empty()))   // typing within a second is one step
                    s.undo.push_back({s.text, s.caret, s.anchor});
                if (s.undo.size() > 200)
                    s.undo.erase(s.undo.begin());
                s.redo.clear();
                s.lastTyping = typing ? now : -10.0;
            };
            const auto replace = [&](std::uint32_t a, std::uint32_t z, std::string_view ins, bool typing) {
                if (a == z && ins.empty())
                    return;
                pushUndo(typing);
                s.text.replace(a, z - a, ins);
                s.caret = s.anchor = a + (std::uint32_t)ins.size();
                s.blink = 0.0f;
                result.changed = true;
            };
            const auto move = [&](std::uint32_t to, bool extend) {
                s.caret = to;
                if (!extend)
                    s.anchor = to;
                s.blink = 0.0f;
                s.lastTyping = -10.0;
            };

            // typed characters (the platform's text: keyboard and IME results)
            std::string typed;
            for (const char32_t ch : in.Text())
                if (ch >= 0x20 && ch != 0x7F)
                    EncodeUtf8(typed, ch);
            if (!typed.empty())
                replace(selMin(), selMax(), typed, !o.password);

            const CaretMap& gm = Geometry(s.real, f, s.text);
            const auto key = [&](Key k) { return c.KeyPressed(k, fid); };
            const auto has = [&](Key k, std::uint32_t mod) { return (in.KeyMods(k) & mod) != 0; };
            if (key(Key::Left))
            {
                const bool shift = has(Key::Left, Mod_Shift);
                move(has(Key::Left, kWordMod) ? WordLeft(gm, s.caret) : (!shift && s.caret != s.anchor ? selMin() : PrevStop(gm, s.caret)), shift);
            }
            else if (key(Key::Right))
            {
                const bool shift = has(Key::Right, Mod_Shift);
                move(has(Key::Right, kWordMod) ? WordRight(gm, s.caret) : (!shift && s.caret != s.anchor ? selMax() : NextStop(gm, s.caret)), shift);
            }
            else if (key(Key::Home) || key(Key::Up))
                move(0, has(Key::Home, Mod_Shift) || has(Key::Up, Mod_Shift));
            else if (key(Key::End) || key(Key::Down))
                move((std::uint32_t)s.text.size(), has(Key::End, Mod_Shift) || has(Key::Down, Mod_Shift));
            else if (key(Key::Backspace))
            {
                if (s.caret != s.anchor)
                    replace(selMin(), selMax(), {}, false);
                else if (s.caret > 0)
                    replace(has(Key::Backspace, kWordMod) ? WordLeft(gm, s.caret) : PrevStop(gm, s.caret), s.caret, {}, false);
            }
            else if (key(Key::Delete))
            {
                if (s.caret != s.anchor)
                    replace(selMin(), selMax(), {}, false);
                else if (s.caret < s.text.size())
                    replace(s.caret, has(Key::Delete, kWordMod) ? WordRight(gm, s.caret) : NextStop(gm, s.caret), {}, false);
            }
            else if (key(Key::A) && has(Key::A, kShortcutMod))
            {
                s.anchor = 0;
                s.caret = (std::uint32_t)s.text.size();
            }
            else if ((key(Key::C) && has(Key::C, kShortcutMod)) || (key(Key::X) && has(Key::X, kShortcutMod)))
            {
                if (s.caret != s.anchor && !o.password)   // a password is not copied
                {
                    c.SetClipboardText(s.text.substr(selMin(), selMax() - selMin()));
                    if (key(Key::X))
                        replace(selMin(), selMax(), {}, false);
                }
            }
            else if (key(Key::V) && has(Key::V, kShortcutMod))
            {
                std::string paste = c.GetClipboardText();
                std::erase_if(paste, [](char ch) { return ch == '\r' || ch == '\n' || ch == '\t'; });   // one line
                replace(selMin(), selMax(), paste, false);
            }
            else if ((key(Key::Z) && has(Key::Z, kShortcutMod)) || (key(Key::Y) && has(Key::Y, kShortcutMod)))
            {
                const bool redo = key(Key::Y) || has(Key::Z, Mod_Shift);
                auto& from = redo ? s.redo : s.undo;
                auto& to = redo ? s.undo : s.redo;
                if (!from.empty())
                {
                    to.push_back({s.text, s.caret, s.anchor});
                    s.text = from.back().text;
                    s.caret = from.back().caret;
                    s.anchor = from.back().anchor;
                    from.pop_back();
                    s.lastTyping = -10.0;
                    result.changed = true;
                }
            }
            else if (key(Key::Enter) || key(Key::Escape))
            {
                result.submitted = key(Key::Enter);
                c.SetKeyboardFocusId(0);
                active = false;
            }

            // mouse: a click places the caret, a drag selects, a double click selects a word, a triple click all
            const CaretMap& gm2 = Geometry(s.real, f, s.text);
            // a password is hit on its bullets: the n-th stop of the bullets is the n-th of the text
            const CaretMap& hm = o.password ? Geometry(s.shown, f, Bullets(gm2.stops.size() - 1)) : gm2;
            const float originX = x0 - s.scroll;
            const auto hit = [&] { return gm2.stops[std::min(HitIndex(hm, in.MousePos().x - originX), gm2.stops.size() - 1)].offset; };
            if (b.pressed && !(focusedNow && o.selectOnFocus))
            {
                const int clicks = b.clicks;
                const std::uint32_t at = hit();
                if (clicks >= 3)
                {
                    s.anchor = 0;
                    s.caret = (std::uint32_t)s.text.size();
                }
                else if (clicks == 2)
                {
                    s.anchor = WordLeft(gm2, NextStop(gm2, at));
                    s.caret = WordRight(gm2, s.anchor);
                }
                else
                {
                    move(at, (in.Mods() & Mod_Shift) != 0);
                    s.dragging = true;
                }
            }
            if (s.dragging)
            {
                if (b.held)
                    s.caret = hit();
                else
                    s.dragging = false;
            }

            // write back (what is longer than o.maxBytes is refused)
            if (result.changed)
            {
                if (o.maxBytes > 0 && s.text.size() > o.maxBytes && !s.undo.empty())
                {
                    s.text = s.undo.back().text;
                    s.caret = s.undo.back().caret;
                    s.anchor = s.undo.back().anchor;
                    s.undo.pop_back();
                }
                result.changed = s.text != *value;
                *value = s.text;
            }
            if (!o.password && active)
            {
                comp = in.Composition();
                compCursor = std::clamp(in.CompositionCursor(), 0, (int)comp.size());
                compTargetA = std::clamp(in.CompositionTargetBegin(), 0, (int)comp.size());
                compTargetB = std::clamp(in.CompositionTargetEnd(), compTargetA, (int)comp.size());
            }
        }
        s.wasActive = active;
        if (b.hovered || s.dragging)
            c.SetMouseCursor(MouseCursor::TextInput);

        // ---------------------------------------------------------- display
        // what is shown: the text with the IME composition in place of the selection, or bullets (password)
        const CaretMap& real = Geometry(s.real, f, s.text);
        std::string shown;
        std::uint32_t caretShown = s.caret, selA = std::min(s.caret, s.anchor), selB = std::max(s.caret, s.anchor), compA = 0;
        if (o.password)
        {
            shown = Bullets(real.stops.size() - 1);
            caretShown = (std::uint32_t)(StopIndex(real, s.caret) * kBullet.size());
            selA = (std::uint32_t)(StopIndex(real, selA) * kBullet.size());
            selB = (std::uint32_t)(StopIndex(real, selB) * kBullet.size());
        }
        else if (!comp.empty())
        {
            shown = s.text.substr(0, selA) + comp + s.text.substr(selB);
            compA = selA;
            caretShown = selA + (std::uint32_t)compCursor;
        }
        else
            shown = s.text;
        const CaretMap& dm = Geometry(s.shown, f, shown);
        const float caretX = CaretX(dm, caretShown);
        if (active)
        {
            if (caretX - s.scroll > innerW - Sc(2))
                s.scroll = caretX - innerW + Sc(2);
            if (caretX - s.scroll < 0.0f)
                s.scroll = caretX;
        }
        s.scroll = active ? std::clamp(s.scroll, 0.0f, std::max(0.0f, Width(dm) - innerW + Sc(2))) : 0.0f;
        const float originX = x0 - s.scroll;

        Painter p = GetPainter();
        const float focus = Anim(fid, 0x40, active ? 1.0f : 0.0f, SpringFast());
        if (o.background)
        {
            Style fs = Style().Radius(ItemRadius(Sc(12)));
            SurfaceFill(fs, FillOr(pc.tertiaryFill));
            if (focus > 0.01f)
                fs.Stroke(Sc(2), Accent().Fade(focus), 0.5f);
            p.Rect(r, fs);
        }
        if (o.icon)   // on glass the symbol is a full-strength label (iOS search bar)
            DrawIcon(p, Vec2(r.min.x + padX + Sc(8), r.Center().y), o.icon, Sc(o.background ? 14.0f : 16.0f), o.background ? pc.secondaryLabel : pc.label);

        p.PushClip(Rect(x0 - Sc(1), r.min.y, x0 + innerW + Sc(1), r.max.y));
        if (active && selA != selB && comp.empty())
        {
            const float xa = CaretX(dm, selA), xb = CaretX(dm, selB);
            p.Rect(Rect(originX + std::min(xa, xb), textTop, originX + std::max(xa, xb), textTop + lineH), Style().Radius(Sc(3)).Fill(Accent().Fade(0.28f)));
        }
        if (shown.empty() && !placeholder.empty())
            p.Text(Vec2(x0, textTop), f, o.background ? pc.tertiaryLabel : pc.secondaryLabel, placeholder);   // on glass: stands out from what shows through
        else
            p.Text(Vec2(originX, textTop), f, LabelOr(pc.label), shown);
        if (!comp.empty())
        {
            // the composition is underlined in place; the clause the IME converts, thicker
            const float y = textTop + lineH - Sc(2);
            const std::uint32_t compB = compA + (std::uint32_t)comp.size();
            p.HLine(originX + CaretX(dm, compA), originX + CaretX(dm, compB), y, Accent(), std::max(1.0f, Sc(1.0f)));
            if (compTargetB > compTargetA)
                p.HLine(originX + CaretX(dm, compA + (std::uint32_t)compTargetA), originX + CaretX(dm, compA + (std::uint32_t)compTargetB), y, Accent(),
                        std::max(1.0f, Sc(2.0f)));
        }
        if (active)
        {
            // an iOS caret: accent, 2 pt, glides between positions and blinks softly after a pause
            const float cx = Anim(fid, 0x41, caretX, Spring{0.08f, 1.0f}, caretX);
            const float phase = std::fmod(std::max(0.0f, s.blink - 0.5f), 1.06f);
            const float on = s.blink < 0.5f ? 1.0f : (phase < 0.53f ? 1.0f : 0.0f);
            const float alpha = Anim(fid, 0x42, on, Spring{0.10f, 1.0f});
            const float cw = std::max(1.0f, Sc(2.0f));
            p.Rect(Rect(originX + cx - cw * 0.5f, textTop - Sc(1), originX + cx + cw * 0.5f, textTop + lineH + Sc(1)), Style().Radius(cw * 0.5f).Fill(Accent().Fade(alpha)));
            // the IME's candidate window opens at the caret
            c.RequestTextInput(Rect(originX + caretX, textTop, originX + caretX + 1.0f, textTop + lineH));
        }
        p.PopClip();

        if (showClear)
        {
            const Vec2 cc(r.max.x - padX - Sc(8), r.Center().y);
            const Interaction it = InteractImpl(Salt(fid, 0xC1EA), Rect::FromCenter(cc, Vec2(Sc(22), Sc(22))), InteractFlags_None);
            p.Circle(cc, Sc(8.5f), Style().Fill(pc.tertiaryLabel.Fade(0.9f + 0.1f * it.hover)));
            DrawIcon(p, cc, icons::Close, Sc(8), T().dark ? Color::Black(0.8f) : Color::White());
            if (it.pressed)
            {
                if (active)
                {
                    s.undo.push_back({s.text, s.caret, s.anchor});
                    s.caret = s.anchor = 0;
                }
                s.text.clear();
                value->clear();
                result.changed = true;
            }
        }
        return result;
    }

    TextFieldResult TextField(std::string_view id, std::string* value, std::string_view placeholder, const TextFieldOptions& o)
    {
        ItemScope scope;
        Context& c = Ctx();
        const float width = o.width > 0.0f ? Sc(o.width) : AvailableWidth();
        const Rect r = Rect::FromSize(c.CursorPos(), Vec2(width, Sc(38)));
        c.ItemSize(r.Size());
        return TextFieldAt(c.GetId(id), r, value, placeholder, o);
    }

    TextFieldResult SearchField(std::string_view id, std::string* value, std::string_view placeholder)
    {
        ItemScope scope;
        TextFieldOptions o;
        o.icon = icons::Search;
        return TextField(id, value, placeholder, o);
    }
}
