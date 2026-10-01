// glass_window - the scene (scene.hpp): everything in UI units; the renderer scales it by FrameParams::framebufferScale.
#include "scene.hpp"
#include "esia/base/utf8.hpp"
#include "esia/render/painter.hpp"
#include <cmath>
#include <cstdio>

namespace glass
{
    using namespace esia;

    namespace
    {
        const Color kBlue = Color::Hex(0x0A84FF), kIndigo = Color::Hex(0x5E5CE6), kOrange = Color::Hex(0xFF9F0A),
                    kPink = Color::Hex(0xFF375F), kTeal = Color::Hex(0x40C8E0), kGreen = Color::Hex(0x30D158);
        constexpr float kLabel = 15.0f, kSmall = 13.0f;   // font sizes

        GlassMaterial Glass(float blur, float refraction, float bezel, Color tint = Color(1, 1, 1, 0.18f))
        {
            GlassMaterial g;
            g.blur = blur;
            g.refraction = refraction;
            g.bezel = bezel;
            g.tint = tint;
            return g;
        }

        void PopUtf8Char(std::string& s)
        {
            while (!s.empty() && ((unsigned char)s.back() & 0xC0) == 0x80)
                s.pop_back();
            if (!s.empty())
                s.pop_back();
        }

        int CountChars(std::string_view s)
        {
            int n = 0;
            for (char c : s)
                n += ((unsigned char)c & 0xC0) != 0x80;
            return n;
        }

        // Begin's companion: the window's glass surface, its title, and the layout cursor below the title.
        void Panel(Context& ctx, Painter& p, text::TextSystem* text, text::FontId font, const char* title)
        {
            Window* w = ctx.CurrentWindow();
            DrawList& dl = w->GetDrawList();
            const bool focused = ctx.FocusedWindow() == w;
            dl.PushClipRect(Rect(Vec2(0, 0), ctx.DisplaySize()), false);   // the shadow reaches past the window's clip
            p.Rect(w->GetRect(), Style()
                                     .Radius(22)
                                     .Glass(Glass(14, 10, 26, Color(0.08f, 0.08f, 0.11f, 0.38f)))
                                     .Shadow(Color::Black(focused ? 0.45f : 0.3f), focused ? 30.0f : 20.0f, Vec2(0, 10))
                                     .Stroke(1.0f, Color::White(focused ? 0.35f : 0.18f)));
            dl.PopClipRect();
            if (text)
                p.Text(ctx.CursorPos(), {font, kLabel}, Color::White(0.92f), title);
            ctx.ItemSize(Vec2(0, 24));
        }
    }

    float Scene::TextWidth(std::string_view s) const
    {
        return text_ && !s.empty() ? text_->Measure({font_, kLabel}, s).size.x : 0.0f;
    }

    void Scene::Frame(Context& ctx, const SceneInfo& info)
    {
        const float t = (float)ctx.Input().Time();
        Wallpaper(ctx, t);

        // Windows are placed on first use only: afterwards they are where the user dragged them.
        ctx.SetNextWindowPos(Vec2(60, 110), Cond::FirstUse);
        ctx.SetNextWindowSize(Vec2(340, 230), Cond::FirstUse);
        ControlsWindow(ctx, info);
        ctx.SetNextWindowPos(Vec2(440, 110), Cond::FirstUse);
        ctx.SetNextWindowSize(Vec2(480, 230), Cond::FirstUse);
        TextWindow(ctx);

        if (text_)
        {
            PainterEnv env;
            env.pixelScale = info.scale;
            env.text = text_;
            Painter fg(ctx.ForegroundDrawList(), env);
            char status[256];
            std::snprintf(status, sizeof(status), "Esia  |  %s  |  %s  |  %d x %d px at %.2fx  |  %.0f fps", info.api, info.adapter.c_str(), info.width,
                          info.height, info.scale, info.fps);
            fg.Text(Vec2(24, 18), {font_, kSmall}, Color::White(0.85f), status);
            fg.Text(Vec2(24, 40), {font_, kSmall}, Color::White(0.6f), "Drag a window by its empty area, resize it from its edges.");
        }
    }

    void Scene::Wallpaper(Context& ctx, float t)
    {
        const Rect d(Vec2(0, 0), ctx.DisplaySize());
        PainterEnv env;
        env.pixelScale = ctx.FramebufferScale().x;
        env.text = text_;
        DrawList& dl = ctx.BackgroundDrawList();
        Painter p(dl, env);
        p.Rect(d, Style().Fill(Paint::Linear(Color::Hex(0x1E3A8A), Color::Hex(0x9D174D), 35)));
        // slow color blobs and hard stripes: what the glass bends and blurs
        const float w = d.Width(), h = d.Height();
        p.Circle(Vec2(w * (0.22f + 0.04f * std::sin(t * 0.4f)), h * 0.30f), h * 0.20f, Style().Fill(kOrange));
        p.Circle(Vec2(w * 0.74f, h * (0.62f + 0.05f * std::sin(t * 0.3f + 1.0f))), h * 0.26f, Style().Fill(kTeal));
        p.Circle(Vec2(w * 0.48f, h * 0.88f), h * 0.16f, Style().Fill(kPink));
        for (float x = 18.0f; x < w; x += 64.0f)
            dl.AddRectFilled(Rect(x, 0, x + 7.0f, h), Color(1, 1, 1, 0.5f).ToRgba8());

        // three glass cards drifting over it: clear, frosted, tinted
        struct Card
        {
            const char* name;
            GlassMaterial glass;
        };
        const Card cards[3] = {{"clear", Glass(0, 14, 36)}, {"frosted", Glass(12, 10, 30)}, {"tinted", Glass(6, 10, 30, Color(kIndigo.r, kIndigo.g, kIndigo.b, 0.35f))}};
        for (int i = 0; i < 3; ++i)
        {
            const Vec2 pos(60.0f + (float)i * 230.0f + 16.0f * std::sin(t * 0.7f + (float)i), h - 190.0f + 10.0f * std::cos(t * 0.5f + (float)i * 2.0f));
            const Rect r = Rect::FromSize(pos, Vec2(200, 130));
            p.Rect(r, Style().Radius(28).Glass(cards[i].glass).Shadow(Color::Black(0.25f), 22, Vec2(0, 8)));
            if (text_)
                p.Text(r.min + Vec2(20, 16), {font_, kLabel}, Color::White(0.9f), cards[i].name);
        }
    }

    void Scene::ControlsWindow(Context& ctx, const SceneInfo& info)
    {
        ctx.Begin("Controls");
        PainterEnv env;
        env.pixelScale = info.scale;
        env.text = text_;
        Painter p(ctx.WindowDrawList(), env);
        Panel(ctx, p, text_, font_, "Controls");

        const Id id = ctx.GetId("press");
        const Rect bb = Rect::FromSize(ctx.CursorPos(), Vec2(180, 44));
        ctx.ItemSize(bb.Size());
        if (ctx.ItemAdd(id, bb, ItemFlags_Focusable))
        {
            const ButtonResult b = ctx.ButtonBehavior(id, bb);
            if (b.pressed)
                ++clicks_;
            if (b.hovered)
                ctx.SetMouseCursor(MouseCursor::Hand);
            const Color base = b.held ? Color::Hex(0x0060DF) : b.hovered ? Color::Hex(0x3D9BFF) : kBlue;
            Style s = Style().Fill(Paint::Linear(base, Color(base.r * 0.8f, base.g * 0.8f, base.b * 0.95f), 90)).Shadow(Color::Black(0.3f), 12, Vec2(0, 4));
            if (b.hovered && !b.held)
                s.Glow(kBlue, 18, 0.7f);
            if (ctx.KeyboardFocusId() == id)
                s.Stroke(2.0f, Color::White(0.8f), 1.0f);
            p.PushScale(bb.Center(), b.held ? 0.96f : 1.0f);   // pressed: sinks a little
            p.Capsule(bb, s);
            if (text_)
                p.TextBox(bb, Vec2(0.5f, 0.5f), {font_, kLabel}, Color::White(), b.held ? "Pressed" : b.hovered ? "Hovered" : "Press me");
            p.PopScale();
        }
        // the click count as a row of dots (and as text when there is a font)
        const Vec2 c = ctx.CursorPos();
        for (int i = 0; i < 10; ++i)
            p.Circle(c + Vec2(8.0f + (float)i * 18.0f, 10.0f), 6.0f, Style().Fill(i < clicks_ % 11 ? kGreen : Color::White(0.2f)));
        ctx.ItemSize(Vec2(190, 20));
        if (text_)
        {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "Pressed %d time%s", clicks_, clicks_ == 1 ? "" : "s");
            p.Text(ctx.CursorPos(), {font_, kSmall}, Color::White(0.75f), buf);
            ctx.ItemSize(Vec2(190, 18));
        }
        ctx.End();
    }

    void Scene::TextWindow(Context& ctx)
    {
        ctx.Begin("Text");
        PainterEnv env;
        env.pixelScale = ctx.FramebufferScale().x;
        env.text = text_;
        Painter p(ctx.WindowDrawList(), env);
        Panel(ctx, p, text_, font_, "Text input (IME)");
        const Rect bb = Rect::FromSize(ctx.CursorPos(), Vec2(std::max(ctx.ContentRegionAvail().x, 120.0f), 44));
        ctx.ItemSize(bb.Size());
        TextField(ctx, ctx.GetId("field"), bb);
        if (text_)
        {
#if defined(__APPLE__)
            p.Text(ctx.CursorPos(), {font_, kSmall}, Color::White(0.65f), "Enter prints the text to the console, Cmd+C / Cmd+V copy / paste,");
#else
            p.Text(ctx.CursorPos(), {font_, kSmall}, Color::White(0.65f), "Enter prints the text to the console, Ctrl+C / Ctrl+V copy / paste,");
#endif
            ctx.ItemSize(Vec2(0, 16));
#if defined(__APPLE__)
            p.Text(ctx.CursorPos(), {font_, kSmall}, Color::White(0.65f), "Esc leaves the field. \xE4\xB8\xAD\xE6\x96\x87\xE8\xBE\x93\xE5\x85\xA5: Pinyin - Simplified");
#elif defined(_WIN32)
            p.Text(ctx.CursorPos(), {font_, kSmall}, Color::White(0.65f), "Esc leaves the field. \xE4\xB8\xAD\xE6\x96\x87\xE8\xBE\x93\xE5\x85\xA5: Microsoft Pinyin");
#else
            p.Text(ctx.CursorPos(), {font_, kSmall}, Color::White(0.65f), "Esc leaves the field. \xE4\xB8\xAD\xE6\x96\x87\xE8\xBE\x93\xE5\x85\xA5: IBus / Fcitx (XIM)");
#endif
            ctx.ItemSize(Vec2(0, 16));
        }
        ctx.End();
    }

    void Scene::TextField(Context& ctx, Id id, const Rect& bb)
    {
        if (!ctx.ItemAdd(id, bb, ItemFlags_Focusable))
            return;
        const ButtonResult b = ctx.ButtonBehavior(id, bb, ButtonFlags_PressOnClick | ButtonFlags_FocusOnClick);
        if (b.hovered)
            ctx.SetMouseCursor(MouseCursor::TextInput);
        const bool focused = ctx.KeyboardFocusId() == id;
        const InputState& in = ctx.Input();
        const std::string& comp = in.Composition();
        if (focused)
        {
            for (char32_t c : in.Text())
                EncodeUtf8(field_, c);
            // shortcuts read the modifiers of their own press (KeyMods): a Ctrl released in the same frame counts
#if defined(__APPLE__)
            constexpr std::uint32_t kShortcutMod = Mod_Super;   // Cmd+C, Cmd+V
#else
            constexpr std::uint32_t kShortcutMod = Mod_Ctrl;
#endif
            auto shortcut = [&](Key k) { return in.KeyPressed(k, false) && (in.KeyMods(k) & kShortcutMod) != 0; };
            // while the IME composes, its keys are its own (they arrive as VK_PROCESSKEY and never get here)
            if (comp.empty())
            {
                if (in.KeyPressed(Key::Backspace))
                    PopUtf8Char(field_);
                if (shortcut(Key::V))
                    for (char c : ctx.GetClipboardText())
                        if (c != '\n' && c != '\r')
                            field_.push_back(c);
                if (shortcut(Key::C))
                    ctx.SetClipboardText(field_);
                if (in.KeyPressed(Key::Enter, false))
                {
                    std::printf("text field: %s\n", field_.c_str());
                    std::fflush(stdout);
                }
                if (in.KeyPressed(Key::Escape, false))
                    ctx.SetKeyboardFocusId(0);
            }
        }

        PainterEnv env;
        env.pixelScale = ctx.FramebufferScale().x;
        env.text = text_;
        Painter p(ctx.WindowDrawList(), env);
        p.Rect(bb, Style().Fill(Color(0, 0, 0, 0.3f)).Radius(12).Stroke(focused ? 2.0f : 1.0f, focused ? kBlue : Color::White(0.25f)));
        p.PushClip(bb.Shrunk(3));
        const std::string_view shownComp = focused ? std::string_view(comp) : std::string_view();
        const int compCaret = focused ? std::clamp(in.CompositionCursor(), 0, (int)comp.size()) : 0;
        const float lineH = 22.0f;
        const Vec2 origin(bb.min.x + 12, bb.Center().y - lineH * 0.5f);
        float caretX = origin.x;
        if (text_)
        {
            // committed text, then the composition in the accent color, underlined: the IME only shows candidates.
            // The clause the IME is converting gets a thicker line, as in Windows' own editors.
            const float committed = TextWidth(field_);
            p.Text(origin, {font_, kLabel}, Color::White(), field_);
            if (!shownComp.empty())
            {
                const Vec2 at = origin + Vec2(committed, 0);
                p.Text(at, {font_, kLabel}, Color::Hex(0x9ECBFF), shownComp);
                p.HLine(at.x, at.x + TextWidth(shownComp), origin.y + lineH, Color::Hex(0x9ECBFF), 1.0f);
                const auto target0 = (std::size_t)in.CompositionTargetBegin(), target1 = (std::size_t)in.CompositionTargetEnd();
                if (target1 > target0)
                    p.HLine(at.x + TextWidth(shownComp.substr(0, target0)), at.x + TextWidth(shownComp.substr(0, target1)),
                            origin.y + lineH, Color::Hex(0x9ECBFF), 2.5f);
            }
            caretX = origin.x + committed + TextWidth(shownComp.substr(0, (std::size_t)compCaret));
        }
        else
        {
            // no font: one dot per character, rings for the composition
            const int n = CountChars(field_), m = CountChars(shownComp);
            for (int i = 0; i < n + m; ++i)
            {
                const Vec2 c(origin.x + 5.0f + (float)i * 12.0f, origin.y + lineH * 0.5f);
                p.Circle(c, 4.0f, i < n ? Style().Fill(Color::White(0.9f)) : Style().Stroke(1.5f, Color::Hex(0x9ECBFF)));
            }
            caretX = origin.x + (float)(n + CountChars(shownComp.substr(0, (std::size_t)compCaret))) * 12.0f;
        }
        p.PopClip();
        if (!focused)
            return;
        const Rect caret(Vec2(caretX, origin.y), Vec2(caretX + 1.5f, origin.y + lineH));
        if (std::fmod(in.Time(), 1.0) < 0.6)
            p.Rect(caret, Style().Fill(Color::White()));
        ctx.RequestTextInput(caret);
    }
}
