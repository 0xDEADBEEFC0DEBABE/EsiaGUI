// Esia UI - the multi-line editor: the text field's editing over paragraphs and wrapped lines, in a scrolling child
// region of the core (the wheel, smooth scrolling and the scroll indicator are the scroll areas').
//
// Layout: the text splits into paragraphs at '\n'; each paragraph's caret stops come from the text system once and are
// kept by content, so an edit reshapes only the paragraphs it touched. Wrapping breaks a paragraph's stops greedily
// after white space (anywhere when a word is wider than the line). Offsets are UTF-8 bytes on caret stops.
#include "ui_internal.hpp"
#include <algorithm>
#include <cstdio>
#include <unordered_map>

namespace esia::ui
{
    using namespace detail;

    namespace
    {
#if defined(__APPLE__)
        constexpr std::uint32_t kShortcutMod = Mod_Super;
        constexpr std::uint32_t kWordMod = Mod_Alt;
        constexpr std::uint32_t kDocMod = Mod_Super;   // Cmd+Up / Down: the text's ends
#else
        constexpr std::uint32_t kShortcutMod = Mod_Ctrl;
        constexpr std::uint32_t kWordMod = Mod_Ctrl;
        constexpr std::uint32_t kDocMod = Mod_Ctrl;
#endif

        bool IsSpace(char32_t c) { return c == U' ' || c == U'\t' || c == 0x00A0 || c == 0x3000 || (c >= 0x2000 && c <= 0x200B); }

        // A visual line: [start, end) of the text (end before a '\n'), its caret stops (absolute offsets, x from the
        // line's start) and whether the grapheme after each stop is white space.
        struct VLine
        {
            std::uint32_t start = 0, end = 0;
            int paragraph = 0;
            bool firstOfParagraph = true;
            bool lastOfParagraph = true;
            std::vector<text::CaretStop> stops;
            std::vector<std::uint8_t> space;
            float width = 0.0f;
        };

        struct Layout
        {
            std::string text;
            text::FontRef font;
            float wrapWidth = -1.0f;
            bool valid = false;
            std::vector<VLine> lines;
            int paragraphs = 0;
            float width = 0.0f;   // the widest line
            std::unordered_map<std::string, std::vector<text::CaretStop>> cache;   // a paragraph's stops, by content
        };

        const std::vector<text::CaretStop>& ParagraphStops(Layout& l, std::unordered_map<std::string, std::vector<text::CaretStop>>& old,
                                                           std::string_view para)
        {
            std::string key(para);
            if (auto it = l.cache.find(key); it != l.cache.end())
                return it->second;
            std::vector<text::CaretStop> stops;
            if (auto it = old.find(key); it != old.end())
                stops = std::move(it->second);
            else if (text::TextSystem* ts = M().text; ts && l.font.id != 0)
                ts->CaretStops(l.font, para, stops);
            else
                for (std::size_t i = 0;;)   // no text system (tests): every code point, at x 0
                {
                    stops.push_back({(std::uint32_t)i, 0.0f});
                    if (i >= para.size())
                        break;
                    DecodeUtf8(para, i);
                }
            if (stops.empty())
                stops.push_back({0, 0.0f});
            return l.cache.emplace(std::move(key), std::move(stops)).first->second;
        }

        void BuildLayout(Layout& l, std::string_view text, text::FontRef f, float wrapWidth)
        {
            if (l.valid && l.text == text && l.font.id == f.id && l.font.size == f.size && l.wrapWidth == wrapWidth)
                return;
            std::unordered_map<std::string, std::vector<text::CaretStop>> old = std::move(l.cache);
            l.cache.clear();
            l.text.assign(text);
            l.font = f;
            l.wrapWidth = wrapWidth;
            l.valid = true;
            l.lines.clear();
            l.width = 0.0f;
            int para = 0;
            for (std::size_t p0 = 0;;)
            {
                std::size_t p1 = text.find('\n', p0);
                if (p1 == std::string_view::npos)
                    p1 = text.size();
                const std::string_view ptext = text.substr(p0, p1 - p0);
                const std::vector<text::CaretStop>& stops = ParagraphStops(l, old, ptext);
                std::vector<std::uint8_t> space(stops.size(), 0);
                for (std::size_t i = 0; i + 1 < stops.size(); ++i)
                {
                    std::size_t k = stops[i].offset;
                    space[i] = IsSpace(DecodeUtf8(ptext, k)) ? 1 : 0;
                }
                // greedy wrap over the stops
                std::size_t s0 = 0;
                const std::size_t last = stops.size() - 1;
                for (;;)
                {
                    std::size_t s1 = last;
                    if (wrapWidth > 0.0f)
                    {
                        const float x0 = stops[s0].x;
                        std::size_t k = s0 + 1, brk = 0;
                        while (k <= last && stops[k].x - x0 <= wrapWidth)
                        {
                            if (space[k - 1] && !(k <= last && space[k]))
                                brk = k;   // after white space
                            ++k;
                        }
                        if (k <= last)
                            s1 = brk > s0 ? brk : std::max(s0 + 1, k - 1);   // a word wider than the line breaks anywhere
                    }
                    VLine v;
                    v.start = (std::uint32_t)(p0 + stops[s0].offset);
                    v.end = (std::uint32_t)(p0 + stops[s1].offset);
                    v.paragraph = para;
                    v.firstOfParagraph = s0 == 0;
                    v.lastOfParagraph = s1 == last;
                    const float x0 = stops[s0].x;
                    for (std::size_t i = s0; i <= s1; ++i)
                    {
                        v.stops.push_back({(std::uint32_t)(p0 + stops[i].offset), stops[i].x - x0});
                        v.space.push_back(space[i]);
                        v.width = std::max(v.width, stops[i].x - x0);
                    }
                    l.width = std::max(l.width, v.width);
                    l.lines.push_back(std::move(v));
                    if (s1 >= last)
                        break;
                    s0 = s1;
                }
                ++para;
                if (p1 >= text.size())
                    break;
                p0 = p1 + 1;
            }
            l.paragraphs = para;
        }

        // The visual line of a caret offset. At a wrap (the end of a line is the start of the next), `upstream` picks
        // the line that ends there.
        int LineOf(const Layout& l, std::uint32_t pos, bool upstream)
        {
            int lo = 0, hi = (int)l.lines.size() - 1;
            while (lo < hi)
            {
                const int mid = (lo + hi + 1) / 2;
                if (l.lines[(std::size_t)mid].start <= pos)
                    lo = mid;
                else
                    hi = mid - 1;
            }
            if (upstream && lo > 0 && l.lines[(std::size_t)lo].start == pos && !l.lines[(std::size_t)lo].firstOfParagraph)
                return lo - 1;
            return lo;
        }

        std::size_t StopIndex(const VLine& v, std::uint32_t pos)
        {
            std::size_t i = 0;
            while (i + 1 < v.stops.size() && v.stops[i + 1].offset <= pos)
                ++i;
            return i;
        }
        float CaretX(const VLine& v, std::uint32_t pos) { return v.stops[StopIndex(v, pos)].x; }
        std::uint32_t HitLine(const VLine& v, float x)
        {
            std::size_t best = 0;
            float bestD = 1e30f;
            for (std::size_t i = 0; i < v.stops.size(); ++i)
            {
                const float d = std::fabs(v.stops[i].x - x);
                if (d < bestD)
                {
                    bestD = d;
                    best = i;
                }
            }
            return v.stops[best].offset;
        }

        // the stop before / after `pos` in the whole text (a '\n' is one step)
        std::uint32_t PrevPos(const Layout& l, std::uint32_t pos)
        {
            if (pos == 0)
                return 0;
            // at a wrap, the line that ends here; at a paragraph's start, the line break before it is the step
            const VLine& v = l.lines[(std::size_t)LineOf(l, pos, true)];
            if (pos == v.start)
                return pos - 1;
            const std::size_t i = StopIndex(v, pos);
            return v.stops[(v.stops[i].offset == pos && i > 0) ? i - 1 : i].offset;
        }
        std::uint32_t NextPos(const Layout& l, std::uint32_t pos)
        {
            if (pos >= l.text.size())
                return (std::uint32_t)l.text.size();
            const VLine& v = l.lines[(std::size_t)LineOf(l, pos, false)];
            if (pos == v.end && v.lastOfParagraph)
                return pos + 1;   // over the '\n'
            const std::size_t i = StopIndex(v, pos);
            return v.stops[std::min(i + 1, v.stops.size() - 1)].offset;
        }
        bool SpaceAfter(const Layout& l, std::uint32_t pos)
        {
            if (pos >= l.text.size())
                return false;
            std::size_t k = pos;
            const char32_t c = DecodeUtf8(l.text, k);
            return c == U'\n' || IsSpace(c);
        }
        std::uint32_t WordLeft(const Layout& l, std::uint32_t pos)
        {
            while (pos > 0 && SpaceAfter(l, PrevPos(l, pos)))
                pos = PrevPos(l, pos);
            while (pos > 0 && !SpaceAfter(l, PrevPos(l, pos)))
                pos = PrevPos(l, pos);
            return pos;
        }
        std::uint32_t WordRight(const Layout& l, std::uint32_t pos)
        {
            const std::uint32_t n = (std::uint32_t)l.text.size();
            while (pos < n && !SpaceAfter(l, pos))
                pos = NextPos(l, pos);
            while (pos < n && SpaceAfter(l, pos) && l.text[pos] != '\n')
                pos = NextPos(l, pos);
            return pos;
        }

        struct EditorState
        {
            std::string text;                      // the working copy while focused
            std::uint32_t caret = 0, anchor = 0;
            bool upstream = false;                 // the caret stands at the end of a wrapped line
            float preferredX = -1.0f;              // Up / Down keep this x
            float blink = 0.0f;
            bool dragging = false;
            bool wasActive = false;
            bool reveal = false;                   // scroll the caret into view
            struct Snapshot
            {
                std::string text;
                std::uint32_t caret = 0, anchor = 0;
            };
            std::vector<Snapshot> undo, redo;
            double lastTyping = -10.0;
            Layout layout;
        };
    }

    TextFieldResult TextEditor(std::string_view id, std::string* value, const TextEditorOptions& o)
    {
        ItemScope scope;
        TextFieldResult result;
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const InputState& in = c.Input();
        const Palette& pc = C();

        const float width = o.size.x > 0.0f ? Sc(o.size.x) : AvailableWidth();
        const float height = Sc(o.size.y > 0.0f ? o.size.y : 200.0f);
        const Rect r = Rect::FromSize(c.CursorPos(), Vec2(width, height));
        c.ItemSize(r.Size());
        const Id fid = c.GetId(id);
        if (!value || !c.ItemAdd(fid, r, ItemFlags_Focusable))
            return result;
        EditorState& s = c.State<EditorState>(fid);
        const text::FontRef f = o.monospace ? Font(TextStyle::Mono) : Font(TextStyle::Body);
        const float lineH = std::max(MeasureText(f, "Hg").y, 1.0f);
        const ButtonResult b = c.ButtonBehavior(fid, r, ButtonFlags_PressOnClick, ItemFlags_Focusable);

        // ------------------------------------------------------------ focus
        if (b.pressed && c.KeyboardFocusId() != fid)
            c.SetKeyboardFocusId(fid);
        bool active = c.KeyboardFocusId() == fid;
        if (active && !s.wasActive)
        {
            s.text = *value;
            s.caret = s.anchor = std::min(s.caret, (std::uint32_t)s.text.size());
            s.undo.clear();
            s.redo.clear();
            s.blink = 0.0f;
        }
        if (!active)
        {
            s.text = *value;
            s.dragging = false;
            s.caret = std::min(s.caret, (std::uint32_t)s.text.size());
            s.anchor = std::min(s.anchor, (std::uint32_t)s.text.size());
        }

        // the text area: a gutter for line numbers, then the lines (inside the scrolling child)
        const float pad = Sc(10);
        const text::FontRef nf = Font(TextStyle::Mono);
        const float gutter = o.lineNumbers ? MeasureText(nf, "0000").x + Sc(14) : 0.0f;
        const float textW = std::max(Sc(20), r.Width() - pad * 2.0f - gutter - Sc(8));   // room for the scroll indicator
        const float wrapW = o.wrap ? textW : 0.0f;
        BuildLayout(s.layout, s.text, f, wrapW);

        // ------------------------------------------------------------ keys
        std::string comp;
        int compCursor = 0, compTargetA = 0, compTargetB = 0;
        const float viewLines = std::max(1.0f, std::floor((r.Height() - pad * 2.0f) / lineH));
        if (active)
        {
            static constexpr Key kKeys[] = {Key::Left, Key::Right, Key::Up, Key::Down, Key::Home, Key::End, Key::PageUp, Key::PageDown, Key::Backspace,
                                            Key::Delete, Key::Enter, Key::Escape, Key::A, Key::C, Key::X, Key::V, Key::Y, Key::Z};
            for (const Key k : kKeys)
                c.ClaimKey(k, fid);
            if (o.tabInput)
                c.ClaimKey(Key::Tab, fid);
            s.blink += DeltaTime();
            m.animating = true;

            const auto selMin = [&] { return std::min(s.caret, s.anchor); };
            const auto selMax = [&] { return std::max(s.caret, s.anchor); };
            const auto pushUndo = [&](bool typing) {
                const double now = Time();
                if (!(typing && now - s.lastTyping < 1.0 && !s.undo.empty()))
                    s.undo.push_back({s.text, s.caret, s.anchor});
                if (s.undo.size() > 200)
                    s.undo.erase(s.undo.begin());
                s.redo.clear();
                s.lastTyping = typing ? now : -10.0;
            };
            const auto replace = [&](std::uint32_t a, std::uint32_t z, std::string_view ins, bool typing) {
                if (o.readOnly || (a == z && ins.empty()))
                    return;
                pushUndo(typing);
                s.text.replace(a, z - a, ins);
                s.caret = s.anchor = a + (std::uint32_t)ins.size();
                s.upstream = false;
                s.preferredX = -1.0f;
                s.blink = 0.0f;
                s.reveal = true;
                result.changed = true;
                BuildLayout(s.layout, s.text, f, wrapW);
            };
            const auto move = [&](std::uint32_t to, bool extend, bool upstream = false, bool keepX = false) {
                s.caret = to;
                if (!extend)
                    s.anchor = to;
                s.upstream = upstream;
                if (!keepX)
                    s.preferredX = -1.0f;
                s.blink = 0.0f;
                s.lastTyping = -10.0;
                s.reveal = true;
            };
            const auto key = [&](Key k) { return c.KeyPressed(k, fid); };
            const auto has = [&](Key k, std::uint32_t mod) { return (in.KeyMods(k) & mod) != 0; };
            const Layout& L = s.layout;
            // Up / Down / Page: the line `delta` away at the remembered x
            const auto vertical = [&](int delta, bool extend) {
                const int line = LineOf(L, s.caret, s.upstream);
                const VLine& v = L.lines[(std::size_t)line];
                if (s.preferredX < 0.0f)
                    s.preferredX = CaretX(v, s.caret);
                const int target = line + delta;
                if (target < 0)
                    move(0, extend, false, true);
                else if (target >= (int)L.lines.size())
                    move((std::uint32_t)s.text.size(), extend, false, true);
                else
                {
                    const VLine& t = L.lines[(std::size_t)target];
                    const std::uint32_t at = HitLine(t, s.preferredX);
                    move(at, extend, at == t.end && !t.lastOfParagraph, true);
                }
            };

            // typed characters
            std::string typed;
            for (const char32_t ch : in.Text())
                if (ch >= 0x20 && ch != 0x7F)
                    EncodeUtf8(typed, ch);
            if (!typed.empty())
                replace(selMin(), selMax(), typed, true);

            if (key(Key::Left))
            {
                const bool shift = has(Key::Left, Mod_Shift);
                move(has(Key::Left, kWordMod) ? WordLeft(L, s.caret) : (!shift && s.caret != s.anchor ? selMin() : PrevPos(L, s.caret)), shift);
            }
            else if (key(Key::Right))
            {
                const bool shift = has(Key::Right, Mod_Shift);
                move(has(Key::Right, kWordMod) ? WordRight(L, s.caret) : (!shift && s.caret != s.anchor ? selMax() : NextPos(L, s.caret)), shift);
            }
            else if (key(Key::Up))
            {
                if (has(Key::Up, kDocMod) && kDocMod != kWordMod)
                    move(0, has(Key::Up, Mod_Shift));
                else
                    vertical(-1, has(Key::Up, Mod_Shift));
            }
            else if (key(Key::Down))
            {
                if (has(Key::Down, kDocMod) && kDocMod != kWordMod)
                    move((std::uint32_t)s.text.size(), has(Key::Down, Mod_Shift));
                else
                    vertical(1, has(Key::Down, Mod_Shift));
            }
            else if (key(Key::PageUp))
                vertical(-(int)viewLines, has(Key::PageUp, Mod_Shift));
            else if (key(Key::PageDown))
                vertical((int)viewLines, has(Key::PageDown, Mod_Shift));
            else if (key(Key::Home))
            {
                const bool shift = has(Key::Home, Mod_Shift);
                if (has(Key::Home, Mod_Ctrl))
                    move(0, shift);
                else
                    move(L.lines[(std::size_t)LineOf(L, s.caret, s.upstream)].start, shift);
            }
            else if (key(Key::End))
            {
                const bool shift = has(Key::End, Mod_Shift);
                if (has(Key::End, Mod_Ctrl))
                    move((std::uint32_t)s.text.size(), shift);
                else
                {
                    const VLine& v = L.lines[(std::size_t)LineOf(L, s.caret, s.upstream)];
                    move(v.end, shift, !v.lastOfParagraph);
                }
            }
            else if (key(Key::Backspace))
            {
                if (s.caret != s.anchor)
                    replace(selMin(), selMax(), {}, false);
                else if (s.caret > 0)
                    replace(has(Key::Backspace, kWordMod) ? WordLeft(L, s.caret) : PrevPos(L, s.caret), s.caret, {}, false);
            }
            else if (key(Key::Delete))
            {
                if (s.caret != s.anchor)
                    replace(selMin(), selMax(), {}, false);
                else if (s.caret < s.text.size())
                    replace(s.caret, has(Key::Delete, kWordMod) ? WordRight(L, s.caret) : NextPos(L, s.caret), {}, false);
            }
            else if (key(Key::Enter))
            {
                if (has(Key::Enter, kShortcutMod))
                    result.submitted = true;   // Ctrl+Enter: the host acts on the text
                else
                    replace(selMin(), selMax(), "\n", false);
            }
            else if (o.tabInput && key(Key::Tab))
                replace(selMin(), selMax(), "    ", true);
            else if (key(Key::A) && has(Key::A, kShortcutMod))
            {
                s.anchor = 0;
                s.caret = (std::uint32_t)s.text.size();
            }
            else if ((key(Key::C) && has(Key::C, kShortcutMod)) || (key(Key::X) && has(Key::X, kShortcutMod)))
            {
                if (s.caret != s.anchor)
                {
                    c.SetClipboardText(s.text.substr(selMin(), selMax() - selMin()));
                    if (key(Key::X))
                        replace(selMin(), selMax(), {}, false);
                }
            }
            else if (key(Key::V) && has(Key::V, kShortcutMod))
            {
                std::string paste = c.GetClipboardText();
                // line breaks become '\n' (Windows' CR LF, old Mac CR)
                std::string norm;
                norm.reserve(paste.size());
                for (std::size_t i = 0; i < paste.size(); ++i)
                    if (paste[i] == '\r')
                    {
                        norm += '\n';
                        if (i + 1 < paste.size() && paste[i + 1] == '\n')
                            ++i;
                    }
                    else
                        norm += paste[i];
                replace(selMin(), selMax(), norm, false);
            }
            else if ((key(Key::Z) && has(Key::Z, kShortcutMod)) || (key(Key::Y) && has(Key::Y, kShortcutMod)))
            {
                const bool redo = key(Key::Y) || has(Key::Z, Mod_Shift);
                auto& from = redo ? s.redo : s.undo;
                auto& to = redo ? s.undo : s.redo;
                if (!from.empty() && !o.readOnly)
                {
                    to.push_back({s.text, s.caret, s.anchor});
                    s.text = from.back().text;
                    s.caret = from.back().caret;
                    s.anchor = from.back().anchor;
                    from.pop_back();
                    s.lastTyping = -10.0;
                    s.reveal = true;
                    result.changed = true;
                    BuildLayout(s.layout, s.text, f, wrapW);
                }
            }
            else if (key(Key::Escape))
            {
                c.SetKeyboardFocusId(0);
                active = false;
            }

            if (result.changed)
            {
                if (o.maxBytes > 0 && s.text.size() > o.maxBytes && !s.undo.empty())
                {
                    s.text = s.undo.back().text;
                    s.caret = s.undo.back().caret;
                    s.anchor = s.undo.back().anchor;
                    s.undo.pop_back();
                    BuildLayout(s.layout, s.text, f, wrapW);
                }
                result.changed = s.text != *value;
                *value = s.text;
            }
            if (active && !o.readOnly)
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

        // ------------------------------------------------------------ the well
        Painter bg = GetPainter();
        const float focus = Anim(fid, 0x40, active ? 1.0f : 0.0f, SpringFast());
        {
            Style fs = Style().Radius(ItemRadius(Sc(12)));
            SurfaceFill(fs, FillOr(pc.tertiaryFill));
            if (focus > 0.01f)
                fs.Stroke(Sc(2), Accent().Fade(focus), 0.5f);
            bg.Rect(r, fs);
        }

        // what is shown: the text with the IME's composition in place of the selection
        std::uint32_t selA = std::min(s.caret, s.anchor), selB = std::max(s.caret, s.anchor);
        std::uint32_t caretShown = s.caret, compA = 0;
        Layout compLayout;
        const Layout* shown = &s.layout;
        if (!comp.empty())
        {
            const std::string text = s.text.substr(0, selA) + comp + s.text.substr(selB);
            BuildLayout(compLayout, text, f, wrapW);
            shown = &compLayout;
            compA = selA;
            caretShown = selA + (std::uint32_t)compCursor;
        }
        const Layout& L = *shown;

        // ------------------------------------------------------------ the scrolling lines
        c.SetCursorPos(r.min + Vec2(pad * 0.5f, pad * 0.5f));
        ChildOptions co;
        co.size = r.Size() - Vec2(pad, pad);
        co.padding = Vec2(pad * 0.5f, pad * 0.5f);
        co.flags = ChildFlags_ScrollY | ChildFlags_SmoothScroll | (o.wrap ? 0u : ChildFlags_ScrollX);
        c.PushId(id);   // the editor's id scope: two editors in a window scroll on their own
        const Id child = c.GetId("##lines");
        ScrollBegin(child);
        c.BeginChild("##lines", co);
        const bool fade = BeginScrollEdgeFade();
        const Rect view = c.ViewRect();
        const Vec2 origin = c.CursorPos();   // the content's top-left (scrolled)
        const float textX = origin.x + gutter;

        // the mouse: a click places the caret, a drag selects (scrolling at the edges), two clicks a word, three a line
        if (active || b.pressed)
        {
            const auto hit = [&](Vec2 mp, bool* upstream) {
                const int line = std::clamp((int)std::floor((mp.y - origin.y) / lineH), 0, (int)s.layout.lines.size() - 1);
                const VLine& v = s.layout.lines[(std::size_t)line];
                const std::uint32_t at = HitLine(v, mp.x - textX);
                *upstream = at == v.end && !v.lastOfParagraph;
                return at;
            };
            if (b.pressed)
            {
                bool up = false;
                const std::uint32_t at = hit(in.MousePos(), &up);
                if (b.clicks >= 3)
                {
                    // the paragraph
                    const VLine& v = s.layout.lines[(std::size_t)LineOf(s.layout, at, up)];
                    std::uint32_t a = v.start, z = v.end;
                    for (int i = LineOf(s.layout, at, up); i >= 0 && !s.layout.lines[(std::size_t)i].firstOfParagraph; --i)
                        a = s.layout.lines[(std::size_t)i - 1].start;
                    for (std::size_t i = (std::size_t)LineOf(s.layout, at, up); i < s.layout.lines.size(); ++i)
                        if (s.layout.lines[i].lastOfParagraph)
                        {
                            z = s.layout.lines[i].end;
                            break;
                        }
                    s.anchor = a;
                    s.caret = z;
                }
                else if (b.clicks == 2)
                {
                    s.anchor = WordLeft(s.layout, NextPos(s.layout, at));
                    s.caret = WordRight(s.layout, s.anchor);
                }
                else
                {
                    s.caret = at;
                    if (!(in.Mods() & Mod_Shift))
                        s.anchor = at;
                    s.upstream = up;
                    s.dragging = true;
                }
                s.preferredX = -1.0f;
                s.blink = 0.0f;
            }
            if (s.dragging)
            {
                if (b.held)
                {
                    bool up = false;
                    s.caret = hit(in.MousePos(), &up);
                    s.upstream = up;
                    s.reveal = !view.Contains(in.MousePos());   // past an edge: it scrolls toward the mouse
                }
                else
                    s.dragging = false;
            }
        }

        // the lines in view
        Painter p = GetPainter();
        const int first = std::clamp((int)std::floor((view.min.y - origin.y) / lineH), 0, (int)L.lines.size());
        const int last = std::clamp((int)std::ceil((view.max.y - origin.y) / lineH) + 1, first, (int)L.lines.size());
        const Color fg = LabelOr(pc.label);
        if (active && comp.empty() && selA != selB)
            for (int i = first; i < last; ++i)
            {
                const VLine& v = L.lines[(std::size_t)i];
                if (selB < v.start || selA > v.end)
                    continue;
                const float xa = CaretX(v, std::max(selA, v.start));
                float xb = CaretX(v, std::min(selB, v.end));
                if (selB > v.end && v.lastOfParagraph)
                    xb += Sc(6);   // the line break is selected too
                const float y = origin.y + (float)i * lineH;
                if (xb > xa)
                    p.Rect(Rect(textX + xa, y, textX + xb, y + lineH), Style().Radius(Sc(3)).Fill(Accent().Fade(0.28f)));
            }
        for (int i = first; i < last; ++i)
        {
            const VLine& v = L.lines[(std::size_t)i];
            const float y = origin.y + (float)i * lineH;
            if (v.end > v.start)
                p.Text(Vec2(textX, y), f, fg, std::string_view(L.text).substr(v.start, v.end - v.start));
            if (o.lineNumbers && v.firstOfParagraph)
            {
                char num[16];
                std::snprintf(num, sizeof(num), "%d", v.paragraph + 1);
                const Vec2 ns = MeasureText(nf, num);
                p.Text(Vec2(origin.x + gutter - Sc(10) - ns.x, y + (lineH - ns.y) * 0.5f), nf, pc.tertiaryLabel, num);
            }
        }
        if (o.lineNumbers)
            p.FillRect(Rect(origin.x + gutter - Sc(5) - 0.5f, view.min.y, origin.x + gutter - Sc(5) + 0.5f, view.max.y), pc.separator.Fade(0.7f));
        if (!comp.empty())
        {
            // the composition, underlined (the clause being converted thicker), on whichever lines it wraps onto
            const std::uint32_t compB = compA + (std::uint32_t)comp.size();
            for (int i = first; i < last; ++i)
            {
                const VLine& v = L.lines[(std::size_t)i];
                const std::uint32_t a = std::max(compA, v.start), z = std::min(compB, v.end);
                if (a >= z)
                    continue;
                const float y = origin.y + (float)i * lineH + lineH - Sc(2);
                p.HLine(textX + CaretX(v, a), textX + CaretX(v, z), y, Accent(), std::max(1.0f, Sc(1.0f)));
                const std::uint32_t ta = std::max(compA + (std::uint32_t)compTargetA, v.start), tz = std::min(compA + (std::uint32_t)compTargetB, v.end);
                if (tz > ta)
                    p.HLine(textX + CaretX(v, ta), textX + CaretX(v, tz), y, Accent(), std::max(1.0f, Sc(2.0f)));
            }
        }

        // the caret
        const int caretLine = LineOf(L, caretShown, comp.empty() && s.upstream);
        const VLine& cl = L.lines[(std::size_t)caretLine];
        const float cx = textX + CaretX(cl, caretShown), cy = origin.y + (float)caretLine * lineH;
        if (active)
        {
            const float ax = Anim(fid, 0x41, cx - origin.x, Spring{0.08f, 1.0f}, cx - origin.x) + origin.x;
            const float ay = Anim(fid, 0x43, cy - origin.y, Spring{0.08f, 1.0f}, cy - origin.y) + origin.y;
            const float phase = std::fmod(std::max(0.0f, s.blink - 0.5f), 1.06f);
            const float on = s.blink < 0.5f ? 1.0f : (phase < 0.53f ? 1.0f : 0.0f);
            const float alpha = Anim(fid, 0x42, on, Spring{0.10f, 1.0f});
            const float cw = std::max(1.0f, Sc(2.0f));
            p.Rect(Rect(ax - cw * 0.5f, ay - Sc(1), ax + cw * 0.5f, ay + lineH + Sc(1)), Style().Radius(cw * 0.5f).Fill(Accent().Fade(alpha)));
            c.RequestTextInput(Rect(cx, cy, cx + 1.0f, cy + lineH));
        }

        // the content's size (every line, not only those drawn) and the caret kept in view
        c.SetCursorPos(origin);
        c.ItemSize(Vec2(o.wrap ? textW + gutter : L.width + gutter + Sc(8), (float)L.lines.size() * lineH));
        if (s.reveal)
        {
            c.ScrollToRect(Rect(cx - Sc(4), cy, cx + Sc(4), cy + lineH));
            s.reveal = s.dragging && b.held && !view.Contains(in.MousePos());
        }
        EndScrollEdgeFade(fade);
        ScrollEnd(child, -1.0f);
        c.EndChild();
        c.PopId();
        if (!comp.empty())
            M().animating = true;
        return result;
    }
}
