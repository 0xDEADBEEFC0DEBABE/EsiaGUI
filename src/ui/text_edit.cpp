// WGT UI - text fields: WGT's own single-line editor on the WGT text engine.
//
//   * any script with no setup: DirectWrite shaping and system font fallback (CJK, Arabic, Hebrew,
//     Devanagari, Thai, emoji ...), nothing to load or configure,
//   * the caret moves by grapheme clusters, caret positions are bidi-correct, Ctrl moves by words,
//   * selection (drag, double-click word, triple-click all, Shift + keys), clipboard, undo / redo,
//   * IME composition (Chinese / Japanese / Korean) drawn inline in the field with the candidate window
//     opening at the caret; the committed text is delivered by the context (HandleWin32Message),
//   * Dear ImGui's InputText and font system are not involved at all.
#include "ui/ui_internal.hpp"
#include "text/text_engine.hpp"

namespace wgt::ui
{
    using namespace detail;

    namespace
    {
        std::wstring Widen(const char* s)
        {
            const int len = s ? (int)std::strlen(s) : 0;
            if (len <= 0)
                return {};
            const int n = MultiByteToWideChar(CP_UTF8, 0, s, len, nullptr, 0);
            std::wstring w((size_t)n, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, s, len, w.data(), n);
            return w;
        }

        std::string Narrow(const std::wstring& w)
        {
            if (w.empty())
                return {};
            const int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
            std::string s((size_t)n, '\0');
            WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), s.data(), n, nullptr, nullptr);
            return s;
        }

        void AppendCodepoint(std::wstring& w, std::uint32_t cp)
        {
            if (cp >= 0x10000)
            {
                cp -= 0x10000;
                w.push_back((wchar_t)(0xD800 + (cp >> 10)));
                w.push_back((wchar_t)(0xDC00 + (cp & 0x3FF)));
            }
            else
                w.push_back((wchar_t)cp);
        }

        struct EditState
        {
            std::wstring text;                    // working copy while focused (UTF-16)
            std::uint32_t caret = 0, anchor = 0;  // UTF-16 offsets, always on caret stops
            float scroll = 0.0f;
            float blink = 0.0f;                   // seconds since the caret last moved
            bool dragging = false;
            struct Snapshot
            {
                std::wstring text;
                std::uint32_t caret, anchor;
            };
            std::vector<Snapshot> undo, redo;
            double lastTyping = -10.0;
            // caret geometry cache (of whatever string is displayed)
            std::wstring mapText;
            FontRef mapFont{};
            CaretMap map;
        };

        const CaretMap& Geometry(EditState& s, FontRef f, const std::wstring& text)
        {
            if (s.map.stops.empty() || s.mapText != text || s.mapFont.id != f.id || s.mapFont.size != f.size)
            {
                TextEngine* e = Ctx().fonts.engine.get();
                if (!e || !e->CaretGeometry(f, text, s.map))
                {
                    s.map = {};
                    s.map.stops = {0, (std::uint32_t)text.size()};
                    s.map.x = {0.0f, 0.0f};
                    s.map.space = {0, 0};
                }
                s.mapText = text;
                s.mapFont = f;
            }
            return s.map;
        }

        // index of the last caret stop <= pos
        size_t StopIndex(const CaretMap& m, std::uint32_t pos)
        {
            size_t i = 0;
            while (i + 1 < m.stops.size() && m.stops[i + 1] <= pos)
                ++i;
            return i;
        }
        std::uint32_t PrevStop(const CaretMap& m, std::uint32_t pos)
        {
            const size_t i = StopIndex(m, pos);
            return m.stops[(m.stops[i] == pos && i > 0) ? i - 1 : i];
        }
        std::uint32_t NextStop(const CaretMap& m, std::uint32_t pos)
        {
            const size_t i = StopIndex(m, pos);
            return m.stops[std::min(i + 1, m.stops.size() - 1)];
        }
        std::uint32_t WordLeft(const CaretMap& m, std::uint32_t pos)
        {
            size_t i = StopIndex(m, pos);
            while (i > 0 && m.space[i - 1])
                --i;
            while (i > 0 && !m.space[i - 1])
                --i;
            return m.stops[i];
        }
        std::uint32_t WordRight(const CaretMap& m, std::uint32_t pos)
        {
            size_t i = StopIndex(m, pos);
            const size_t last = m.stops.size() - 1;
            while (i < last && !m.space[i])
                ++i;
            while (i < last && m.space[i])
                ++i;
            return m.stops[i];
        }
        float CaretX(const CaretMap& m, std::uint32_t pos) { return m.x[StopIndex(m, pos)]; }
        std::uint32_t HitStop(const CaretMap& m, float x)
        {
            size_t best = 0;
            float bestD = 1e30f;
            for (size_t i = 0; i < m.x.size(); ++i)
            {
                const float d = std::fabs(m.x[i] - x);
                if (d < bestD)
                {
                    bestD = d;
                    best = i;
                }
            }
            return m.stops[best];
        }
    }

    bool TextField(const char* id, char* buffer, std::size_t bufferSize, const char* placeholder, const TextFieldOptions& o)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems || !buffer || bufferSize == 0)
            return false;
        Context::Impl& m = Ctx();
        ImGuiContext& g = *ImGui::GetCurrentContext();
        ImGuiIO& io = ImGui::GetIO();
        const Palette& c = C();

        const float width = o.width > 0.0f ? Sc(o.width) : AvailWidth();
        const float h = Sc(38);
        const Vec2 pos = w->DC.CursorPos;
        const Rect r = Rect::FromSize(pos, Vec2(width, h));
        const ImGuiID fid = w->GetID(id);
        ImGui::ItemSize(r.Size());
        if (!ImGui::ItemAdd(ImRect(r.min, r.max), fid))
            return false;

        EditState& s = m.state.Get<EditState>(fid);
        const FontRef f = GetFont(TextStyle::Body);
        const bool hovered = ImGui::ItemHoverable(ImRect(r.min, r.max), fid, g.LastItemData.ItemFlags);
        bool active = g.ActiveId == fid;
        bool changed = false;

        // ---------------------------------------------------------- focus
        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !active)
        {
            ImGui::SetActiveID(fid, w);
            ImGui::SetFocusID(fid, w);
            ImGui::FocusWindow(w);
            active = true;
            s.text = Widen(buffer);
            s.caret = s.anchor = (std::uint32_t)s.text.size();
            s.undo.clear();
            s.redo.clear();
        }
        else if (active && !hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            ImGui::ClearActiveID();
            active = false;
        }
        if (!active)
            s.text = Widen(buffer);   // external changes show up immediately

        // layout: [icon] text [clear]
        const float padX = Sc(12);
        float x0 = r.min.x + padX;
        if (o.icon)
            x0 += Sc(24);
        const bool showClear = o.clearButton && !s.text.empty();
        const float innerW = std::max(1.0f, r.max.x - padX - (showClear ? Sc(26) : 0.0f) - x0);
        const Vec2 lineSize = Painter::MeasureText(f, "Hg");
        const float textTop = std::floor(r.Center().y - lineSize.y * 0.5f + 0.5f);

        // ---------------------------------------------------------- editing
        std::wstring comp;
        int compCursor = 0;
        if (active)
        {
            ImGui::SetActiveIdUsingAllKeyboardKeys();   // arrows, Home / End, Backspace ... belong to the field
            s.blink += io.DeltaTime;
            const auto selMin = [&] { return std::min(s.caret, s.anchor); };
            const auto selMax = [&] { return std::max(s.caret, s.anchor); };
            const auto pushUndo = [&](bool typing) {
                const double now = anim::Time();
                if (!(typing && now - s.lastTyping < 1.0 && !s.undo.empty()))
                    s.undo.push_back({s.text, s.caret, s.anchor});
                if (s.undo.size() > 200)
                    s.undo.erase(s.undo.begin());
                s.redo.clear();
                s.lastTyping = typing ? now : -10.0;
            };
            const auto replace = [&](std::uint32_t a, std::uint32_t b, const std::wstring& ins, bool typing) {
                if (a == b && ins.empty())
                    return;
                pushUndo(typing);
                s.text.replace(a, b - a, ins);
                s.caret = s.anchor = a + (std::uint32_t)ins.size();
                s.blink = 0.0f;
                changed = true;
            };
            const auto move = [&](std::uint32_t to, bool extend) {
                s.caret = to;
                if (!extend)
                    s.anchor = to;
                s.blink = 0.0f;
                s.lastTyping = -10.0;
            };

            // typed characters (from Dear ImGui's input queue: plain keyboard typing)
            std::wstring typed;
            for (ImWchar ch : io.InputQueueCharacters)
                if (ch >= 0x20 && ch != 0x7F)
                    AppendCodepoint(typed, (std::uint32_t)ch);
            io.InputQueueCharacters.resize(0);
            // IME results (from the context)
            typed += m.TakeImeCommitted();
            if (!typed.empty() && !o.password)
                replace(selMin(), selMax(), typed, true);
            else if (!typed.empty())
                replace(selMin(), selMax(), typed, false);

            const CaretMap& gm = Geometry(s, f, s.text);
            const ImGuiInputFlags rep = ImGuiInputFlags_Repeat;
            const bool ctrl = io.KeyCtrl, shift = io.KeyShift;
            const auto key = [&](ImGuiKey k) { return ImGui::IsKeyPressed(k, rep, fid); };
            if (key(ImGuiKey_LeftArrow))
                move(ctrl ? WordLeft(gm, s.caret) : (!shift && s.caret != s.anchor ? selMin() : PrevStop(gm, s.caret)), shift);
            else if (key(ImGuiKey_RightArrow))
                move(ctrl ? WordRight(gm, s.caret) : (!shift && s.caret != s.anchor ? selMax() : NextStop(gm, s.caret)), shift);
            else if (key(ImGuiKey_Home))
                move(0, shift);
            else if (key(ImGuiKey_End))
                move((std::uint32_t)s.text.size(), shift);
            else if (key(ImGuiKey_Backspace))
            {
                if (s.caret != s.anchor)
                    replace(selMin(), selMax(), {}, false);
                else if (s.caret > 0)
                    replace(ctrl ? WordLeft(gm, s.caret) : PrevStop(gm, s.caret), s.caret, {}, false);
            }
            else if (key(ImGuiKey_Delete))
            {
                if (s.caret != s.anchor)
                    replace(selMin(), selMax(), {}, false);
                else if (s.caret < s.text.size())
                    replace(s.caret, ctrl ? WordRight(gm, s.caret) : NextStop(gm, s.caret), {}, false);
            }
            else if (ctrl && key(ImGuiKey_A))
            {
                s.anchor = 0;
                s.caret = (std::uint32_t)s.text.size();
            }
            else if (ctrl && (key(ImGuiKey_C) || key(ImGuiKey_X)) && s.caret != s.anchor && !o.password)
            {
                ImGui::SetClipboardText(Narrow(s.text.substr(selMin(), selMax() - selMin())).c_str());
                if (ImGui::IsKeyPressed(ImGuiKey_X, rep, fid))
                    replace(selMin(), selMax(), {}, false);
            }
            else if (ctrl && key(ImGuiKey_V))
            {
                std::wstring paste = Widen(ImGui::GetClipboardText());
                paste.erase(std::remove_if(paste.begin(), paste.end(), [](wchar_t ch) { return ch == L'\r' || ch == L'\n' || ch == L'\t'; }), paste.end());
                replace(selMin(), selMax(), paste, false);
            }
            else if (ctrl && (key(ImGuiKey_Z) || key(ImGuiKey_Y)))
            {
                const bool redo = ImGui::IsKeyPressed(ImGuiKey_Y, rep, fid) || shift;
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
                    changed = true;
                }
            }
            else if (key(ImGuiKey_Enter) || key(ImGuiKey_KeypadEnter) || key(ImGuiKey_Escape))
            {
                ImGui::ClearActiveID();
                active = false;
            }

            // mouse: click places the caret, drag selects, double-click selects a word, triple-click all
            const CaretMap& gm2 = Geometry(s, f, s.text);
            const float originX = x0 - s.scroll;
            if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                const std::uint32_t hit = HitStop(gm2, io.MousePos.x - originX);
                const int clicks = io.MouseClickedCount[ImGuiMouseButton_Left];
                if (clicks >= 3)
                {
                    s.anchor = 0;
                    s.caret = (std::uint32_t)s.text.size();
                }
                else if (clicks == 2)
                {
                    s.anchor = WordLeft(gm2, NextStop(gm2, hit));
                    s.caret = WordRight(gm2, s.anchor);
                }
                else
                {
                    move(hit, shift);
                    s.dragging = true;
                }
            }
            if (s.dragging)
            {
                if (io.MouseDown[ImGuiMouseButton_Left])
                    s.caret = HitStop(gm2, io.MousePos.x - originX);
                else
                    s.dragging = false;
            }

            // keep the buffer in sync (reject what does not fit)
            if (changed)
            {
                std::string u8 = Narrow(s.text);
                if (u8.size() + 1 > bufferSize && !s.undo.empty())
                {
                    s.text = s.undo.back().text;
                    s.caret = s.undo.back().caret;
                    s.anchor = s.undo.back().anchor;
                    s.undo.pop_back();
                    u8 = Narrow(s.text);
                }
                const size_t n = std::min(u8.size(), bufferSize - 1);
                std::memcpy(buffer, u8.data(), n);
                buffer[n] = '\0';
            }
            if (!o.password)
                m.GetImeComposition(comp, compCursor);
        }
        if (hovered || s.dragging)
            ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);

        // ---------------------------------------------------------- display
        // what is shown: the text with the IME composition in place of the selection, or bullets (password)
        std::wstring shown = s.text;
        std::uint32_t caretShown = s.caret, compA = 0, compB = 0;
        if (o.password)
        {
            const CaretMap& real = Geometry(s, f, s.text);
            const size_t caretIndex = StopIndex(real, s.caret);
            shown.assign(real.stops.size() - 1, L'\x2022');
            caretShown = (std::uint32_t)caretIndex;
        }
        else if (!comp.empty())
        {
            const std::uint32_t a = std::min(s.caret, s.anchor), b = std::max(s.caret, s.anchor);
            shown = s.text.substr(0, a) + comp + s.text.substr(b);
            compA = a;
            compB = a + (std::uint32_t)comp.size();
            caretShown = a + (std::uint32_t)compCursor;
        }
        const CaretMap& dm = Geometry(s, f, shown);
        const float caretX = CaretX(dm, caretShown);
        if (active)
        {
            if (caretX - s.scroll > innerW - Sc(2))
                s.scroll = caretX - innerW + Sc(2);
            if (caretX - s.scroll < 0.0f)
                s.scroll = caretX;
        }
        s.scroll = std::clamp(s.scroll, 0.0f, std::max(0.0f, dm.width - innerW + Sc(2)));
        if (!active)
            s.scroll = 0.0f;
        const float originX = x0 - s.scroll;

        Painter p;
        const float focus = Anim(fid, 0x40, active ? 1.0f : 0.0f, SpringFast());
        if (o.background)
        {
            Style fs = Style().Radius(ItemRadius(Sc(12)));
            SurfaceFill(fs, FillOr(c.tertiaryFill));
            if (focus > 0.01f)
                fs.Stroke(Sc(2), Accent().Fade(focus), 0.5f);
            p.Rect(r, fs);
        }
        if (o.icon)   // on glass the symbol is a full-strength label (iOS search bar)
            p.Icon(Vec2(r.min.x + padX + Sc(8), r.Center().y), o.icon, Sc(o.background ? 14.0f : 16.0f), o.background ? c.secondaryLabel : c.label);

        p.PushClip(Rect(x0 - Sc(1), r.min.y, x0 + innerW + Sc(1), r.max.y));
        if (active && s.caret != s.anchor && comp.empty())
        {
            const CaretMap& sm = o.password ? dm : Geometry(s, f, shown);
            const std::uint32_t a = o.password ? (std::uint32_t)StopIndex(Geometry(s, f, s.text), std::min(s.caret, s.anchor)) : std::min(s.caret, s.anchor);
            const std::uint32_t b = o.password ? (std::uint32_t)StopIndex(Geometry(s, f, s.text), std::max(s.caret, s.anchor)) : std::max(s.caret, s.anchor);
            const float xa = CaretX(sm, a), xb = CaretX(sm, b);
            p.Rect(Rect(originX + std::min(xa, xb), textTop, originX + std::max(xa, xb), textTop + lineSize.y),
                   Style().Radius(Sc(3)).Fill(Accent().Fade(0.28f)));
        }
        if (shown.empty() && placeholder && *placeholder)
            p.Text(Vec2(x0, textTop), f, o.background ? c.tertiaryLabel : c.secondaryLabel, placeholder);   // on glass: stands out from what shows through
        else
            p.Text(Vec2(originX, textTop), f, LabelOr(c.label), Narrow(shown).c_str());
        if (!comp.empty())
        {
            // composition: underlined in place
            const CaretMap& cm = Geometry(s, f, shown);
            p.HLine(originX + CaretX(cm, compA), originX + CaretX(cm, compB), textTop + lineSize.y - Sc(2), Accent(), std::max(1.0f, Sc(1.5f)));
        }
        if (active)
        {
            // iOS-like caret: accent, 2 pt, glides between positions, blinks softly after a pause
            const float cx = Anim(fid, 0x41, caretX, Spring{0.08f, 1.0f}, caretX);
            const float phase = std::fmod(std::max(0.0f, s.blink - 0.5f), 1.06f);
            const float a = s.blink < 0.5f ? 1.0f : (phase < 0.53f ? 1.0f : 0.0f);
            const float shownA = Anim(fid, 0x42, a, Spring{0.10f, 1.0f});
            const float cw = std::max(1.0f, Sc(2.0f));
            p.Rect(Rect(originX + cx - cw * 0.5f, textTop - Sc(1), originX + cx + cw * 0.5f, textTop + lineSize.y + Sc(1)),
                   Style().Radius(cw * 0.5f).Fill(Accent().Fade(shownA)));
            // the IME candidate window opens at the caret (client pixels = UI units)
            m.SetImeFocus(fid, Vec2(originX + caretX, textTop), lineSize.y);
            ImGuiPlatformImeData& ime = g.PlatformImeData;
            ime.WantVisible = !o.password;
            ime.WantTextInput = true;
            ime.InputPos = Vec2(originX + caretX, textTop);
            ime.InputLineHeight = lineSize.y;
            ime.ViewportId = w->Viewport->ID;
        }
        p.PopClip();

        if (showClear)
        {
            const Vec2 cc(r.max.x - padX - Sc(8), r.Center().y);
            Interaction it = InteractImpl(Salt(fid, 0xC1EA), Rect::FromCenter(cc, Vec2(Sc(22), Sc(22))), InteractFlags_None);
            p.Circle(cc, Sc(8.5f), Style().Fill(c.tertiaryLabel.Fade(0.9f + 0.1f * it.hover)));
            p.Icon(cc, icons::Close, Sc(8), T().dark ? Color::Black(0.8f) : Color::White());
            if (it.pressed)
            {
                if (active)
                {
                    s.undo.push_back({s.text, s.caret, s.anchor});
                    s.text.clear();
                    s.caret = s.anchor = 0;
                }
                buffer[0] = '\0';
                changed = true;
            }
        }
        return changed;
    }

    bool SearchField(const char* id, char* buffer, std::size_t bufferSize, const char* placeholder)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        TextFieldOptions o;
        o.icon = icons::Search;
        return TextField(id, buffer, bufferSize, placeholder, o);
    }
}
