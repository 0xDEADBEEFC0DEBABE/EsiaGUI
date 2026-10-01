// glass_window - the examples' frame on Linux (app.hpp): an X11 window (Xlib; on a Wayland desktop through XWayland),
// its events and the frame loop on one thread, OpenGL through EGL (host_opengl_egl.cpp) or Vulkan through
// VK_KHR_xlib_surface (host_vulkan.cpp).
//
// Input: the pointer, its buttons (4 / 5 and 6 / 7 are the wheel's notches), keys by keysym, text through the input
// method (XIM: what it commits; no inline composition), focus, the pointer leaving the window. The UI scale is
// Xft.dpi / 96 (the desktop's scaling sets it), or --scale. The clipboard is the CLIPBOARD selection (UTF8_STRING);
// key repeat follows the X server's. esia::ui's icons come from the desktop's icon theme (icons_linux.cpp).
#include "app.hpp"
#include "host.hpp"
#include "icons_linux.hpp"
#include "image.hpp"
#include "symbol_text.hpp"
#include "esia/render/renderer.hpp"
#if defined(GLASS_TEXT)
#include "esia/text/freetype.hpp"
#include "esia/text/system_fonts.hpp"
#endif
#include <algorithm>
#include <array>
#include <chrono>
#include <clocale>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <memory>
#include <thread>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
// Xlib's macros that are also names in C++ code (esia::Key::None ...)
#undef None
#undef Bool
#undef Status

namespace glass
{
    namespace
    {
        constexpr long kXNone = 0L;   // Xlib's None

        struct Options
        {
            std::string api;
            float width = 1280.0f, height = 800.0f;   // UI units (fractions: a pixel size at a --scale)
            float scale = 0.0f;   // pixels per UI unit (0 = the desktop's)
            bool vsync = true, debug = false;
            double fixedDt = 0.0;
            int frames = 0;
            std::string screenshot;
            std::vector<std::string> fonts;
        };

        int Usage(const App& app, const char* problem)
        {
            std::fprintf(stderr,
                         "%s: %s\n"
                         "usage: %s [--api %s] [--size WxH] [--scale s] [--vsync on|off] [--debug] [--fixed-dt seconds]\n"
                         "       [--frames N] [--screenshot out.png] [--font file]...\n",
                         app.name, problem, app.name, BuiltApis().c_str());
            return 2;
        }

        bool Parse(App& app, int argc, char** argv, Options& o, std::string& problem)
        {
            for (int i = 1; i < argc; ++i)
            {
                const std::string a = argv[i];
                const char* value = i + 1 < argc ? argv[i + 1] : nullptr;
                const auto needs = [&]() {
                    if (!value)
                        problem = a + " needs a value";
                    else
                        ++i;
                    return value != nullptr;
                };
                bool usedValue = false;
                if (a == "--debug")
                    o.debug = true;
                else if (a == "--api" && needs())
                    o.api = value;
                else if (a == "--vsync" && needs())
                    o.vsync = std::strcmp(value, "off") != 0;
                else if (a == "--size" && needs())
                {
                    if (std::sscanf(value, "%fx%f", &o.width, &o.height) != 2 || !(o.width > 0.0f) || !(o.height > 0.0f))
                        problem = "--size wants WxH";
                }
                else if (a == "--scale" && needs())
                {
                    o.scale = (float)std::atof(value);
                    if (!(o.scale > 0.0f))
                        problem = "--scale wants a number above 0";
                }
                else if (a == "--fixed-dt" && needs())
                    o.fixedDt = std::atof(value);
                else if (a == "--frames" && needs())
                    o.frames = std::atoi(value);
                else if (a == "--screenshot" && needs())
                    o.screenshot = value;
                else if (a == "--font" && needs())
                    o.fonts.push_back(value);
                else if (app.option && app.option(a, value, usedValue))
                    i += usedValue ? 1 : 0;
                else if (problem.empty())
                    problem = "unknown option " + a;
                if (!problem.empty())
                    return false;
            }
            if (o.api.empty())
                o.api = DefaultApi();
            if (!CreateHost(o.api))
                problem = "--api " + o.api + " is not built in (built: " + BuiltApis() + ")";
            else if (!o.screenshot.empty() && o.frames <= 0)
                o.frames = 60;
            return problem.empty();
        }

#if defined(GLASS_TEXT)
        // The main font and its fallbacks: --font, or the system's chain (Noto Sans, Noto Sans CJK, DejaVu Sans ...).
        esia::text::FontId LoadFonts(esia::text::TextSystem& text, const std::vector<std::string>& files)
        {
            if (files.empty())
            {
                const std::vector<esia::text::FontId> chain = esia::text::AddFallbackFonts(text, esia::text::FindDefaultFallbackFonts());
                return chain.empty() ? 0 : chain.front();
            }
            esia::text::FontId main = 0;
            for (const std::string& path : files)
                if (const esia::text::FontId id = text.AddFontFile(path.c_str()))
                {
                    if (!main)
                        main = id;
                    else
                        text.AddFallback(id);
                }
            return main;
        }
#endif

        // The desktop's UI scale: Xft.dpi / 96 (GNOME, KDE and others publish their scaling there), else 1.
        float DesktopScale(Display* dpy)
        {
            float dpi = 0.0f;
            if (const char* resources = ::XResourceManagerString(dpy))
            {
                ::XrmInitialize();
                if (XrmDatabase db = ::XrmGetStringDatabase(resources))
                {
                    char* type = nullptr;
                    XrmValue value = {};
                    if (::XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value) && value.addr)
                        dpi = (float)std::atof(value.addr);
                    ::XrmDestroyDatabase(db);
                }
            }
            return dpi > 0.0f ? dpi / 96.0f : 1.0f;
        }

        esia::Key KeyOf(KeySym k)
        {
            using K = esia::Key;
            const auto offset = [](K first, KeySym k, KeySym base) { return static_cast<K>((int)first + (int)(k - base)); };
            if (k >= XK_a && k <= XK_z)
                return offset(K::A, k, XK_a);
            if (k >= XK_A && k <= XK_Z)
                return offset(K::A, k, XK_A);
            if (k >= XK_0 && k <= XK_9)
                return offset(K::Num0, k, XK_0);
            if (k >= XK_F1 && k <= XK_F24)
                return offset(K::F1, k, XK_F1);
            if (k >= XK_KP_0 && k <= XK_KP_9)
                return offset(K::Keypad0, k, XK_KP_0);
            switch (k)
            {
            case XK_Tab: case XK_ISO_Left_Tab: return K::Tab;
            case XK_Left: case XK_KP_Left: return K::Left;
            case XK_Right: case XK_KP_Right: return K::Right;
            case XK_Up: case XK_KP_Up: return K::Up;
            case XK_Down: case XK_KP_Down: return K::Down;
            case XK_Page_Up: case XK_KP_Page_Up: return K::PageUp;
            case XK_Page_Down: case XK_KP_Page_Down: return K::PageDown;
            case XK_Home: case XK_KP_Home: return K::Home;
            case XK_End: case XK_KP_End: return K::End;
            case XK_Insert: case XK_KP_Insert: return K::Insert;
            case XK_Delete: case XK_KP_Delete: return K::Delete;
            case XK_BackSpace: return K::Backspace;
            case XK_space: return K::Space;
            case XK_Return: case XK_KP_Enter: return K::Enter;
            case XK_Escape: return K::Escape;
            case XK_Control_L: return K::LeftCtrl;
            case XK_Control_R: return K::RightCtrl;
            case XK_Shift_L: return K::LeftShift;
            case XK_Shift_R: return K::RightShift;
            case XK_Alt_L: case XK_Meta_L: return K::LeftAlt;
            case XK_Alt_R: case XK_Meta_R: case XK_ISO_Level3_Shift: return K::RightAlt;
            case XK_Super_L: return K::LeftSuper;
            case XK_Super_R: return K::RightSuper;
            case XK_KP_Decimal: return K::KeypadDecimal;
            case XK_KP_Divide: return K::KeypadDivide;
            case XK_KP_Multiply: return K::KeypadMultiply;
            case XK_KP_Subtract: return K::KeypadSubtract;
            case XK_KP_Add: return K::KeypadAdd;
            case XK_apostrophe: return K::Apostrophe;
            case XK_comma: return K::Comma;
            case XK_minus: return K::Minus;
            case XK_period: return K::Period;
            case XK_slash: return K::Slash;
            case XK_semicolon: return K::Semicolon;
            case XK_equal: return K::Equal;
            case XK_bracketleft: return K::LeftBracket;
            case XK_backslash: return K::Backslash;
            case XK_bracketright: return K::RightBracket;
            case XK_grave: return K::GraveAccent;
            case XK_Caps_Lock: return K::CapsLock;
            case XK_Scroll_Lock: return K::ScrollLock;
            case XK_Num_Lock: return K::NumLock;
            case XK_Print: return K::PrintScreen;
            case XK_Pause: return K::Pause;
            case XK_Menu: return K::Menu;
            default: return K::None;
            }
        }

        std::uint32_t ModsOf(unsigned state)
        {
            return ((state & ControlMask) ? esia::Mod_Ctrl : 0u) | ((state & ShiftMask) ? esia::Mod_Shift : 0u) |
                   ((state & Mod1Mask) ? esia::Mod_Alt : 0u) | ((state & Mod4Mask) ? esia::Mod_Super : 0u);
        }

        // A key's own modifier bit: X reports the state before the event, the core wants it after.
        std::uint32_t ModOfKey(esia::Key k)
        {
            using K = esia::Key;
            switch (k)
            {
            case K::LeftCtrl: case K::RightCtrl: return esia::Mod_Ctrl;
            case K::LeftShift: case K::RightShift: return esia::Mod_Shift;
            case K::LeftAlt: case K::RightAlt: return esia::Mod_Alt;
            case K::LeftSuper: case K::RightSuper: return esia::Mod_Super;
            default: return 0;
            }
        }

        // The window, its input method and cursors, the clipboard and the input translation.
        class Frame
        {
        public:
            ~Frame()
            {
                if (!dpy_)
                    return;
                for (Cursor c : cursors_)
                    if (c)
                        ::XFreeCursor(dpy_, c);
                if (ic_)
                    ::XDestroyIC(ic_);
                if (im_)
                    ::XCloseIM(im_);
                if (window_)
                    ::XDestroyWindow(dpy_, window_);
                ::XCloseDisplay(dpy_);
            }

            bool Create(const std::string& title, const Options& opt, std::string& error)
            {
                dpy_ = ::XOpenDisplay(nullptr);
                if (!dpy_)
                {
                    error = "cannot open the X display (DISPLAY is not set, or no X server / XWayland)";
                    return false;
                }
                scale_ = opt.scale > 0.0f ? opt.scale : DesktopScale(dpy_);
                width_ = std::max(1, (int)std::lround(opt.width * scale_));
                height_ = std::max(1, (int)std::lround(opt.height * scale_));
                const int screen = DefaultScreen(dpy_);
                XSetWindowAttributes swa = {};
                swa.background_pixel = BlackPixel(dpy_, screen);
                swa.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                                 EnterWindowMask | LeaveWindowMask | FocusChangeMask | StructureNotifyMask | ExposureMask;
                window_ = ::XCreateWindow(dpy_, RootWindow(dpy_, screen), 0, 0, (unsigned)width_, (unsigned)height_, 0, CopyFromParent,
                                          InputOutput, CopyFromParent, CWBackPixel | CWEventMask, &swa);
                if (!window_)
                {
                    error = "XCreateWindow failed";
                    return false;
                }
                // title (UTF-8 for the window manager, Latin-1 WM_NAME for old ones), class, close button
                const Atom utf8 = Intern("UTF8_STRING");
                ::XChangeProperty(dpy_, window_, Intern("_NET_WM_NAME"), utf8, 8, PropModeReplace,
                                  reinterpret_cast<const unsigned char*>(title.data()), (int)title.size());
                ::XStoreName(dpy_, window_, title.c_str());
                XClassHint hint = {};
                std::string resName = title, resClass = "Esia";
                hint.res_name = resName.data();
                hint.res_class = resClass.data();
                ::XSetClassHint(dpy_, window_, &hint);
                deleteWindow_ = Intern("WM_DELETE_WINDOW");
                ::XSetWMProtocols(dpy_, window_, &deleteWindow_, 1);
                // key repeat as presses without releases (the core counts repeats itself)
                int supported = 0;
                ::XkbSetDetectableAutoRepeat(dpy_, True, &supported);
                // the input method: the system's (ibus, fcitx through XMODIFIERS) or the locale's compose sequences
                std::setlocale(LC_CTYPE, "");
                ::XSetLocaleModifiers("");
                im_ = ::XOpenIM(dpy_, nullptr, nullptr, nullptr);
                if (!im_)
                {
                    ::XSetLocaleModifiers("@im=none");
                    im_ = ::XOpenIM(dpy_, nullptr, nullptr, nullptr);
                }
                if (im_)
                    ic_ = ::XCreateIC(im_, XNInputStyle, (XIMStyle)(XIMPreeditNothing | XIMStatusNothing), XNClientWindow, window_,
                                      XNFocusWindow, window_, nullptr);
                if (ic_)
                {
                    unsigned long filter = 0;   // the events the input method wants too
                    ::XGetICValues(ic_, XNFilterEvents, &filter, nullptr);
                    ::XSelectInput(dpy_, window_, swa.event_mask | (long)filter);
                }
                clipboard_ = Intern("CLIPBOARD");
                utf8_ = utf8;
                targets_ = Intern("TARGETS");
                transfer_ = Intern("ESIA_CLIPBOARD");
                ::XMapWindow(dpy_, window_);
                mapped_ = false;
                while (!mapped_)   // shown before the first frame, at the size the window manager gave it
                {
                    XEvent e;
                    ::XWindowEvent(dpy_, window_, StructureNotifyMask, &e);
                    Handle(e);
                }
                return true;
            }

            Display* Dpy() const { return dpy_; }
            ::Window Handle() const { return window_; }
            float Scale() const { return scale_; }
            int Width() const { return width_; }
            int Height() const { return height_; }
            bool CloseRequested() const { return close_; }
            bool Visible() const { return mapped_; }
            void SetContext(esia::Context* ctx) { ctx_ = ctx; }

            esia::FrameParams Params(double time) const
            {
                esia::FrameParams p;
                p.displaySize = esia::Vec2((float)width_ / scale_, (float)height_ / scale_);
                p.framebufferScale = esia::Vec2(scale_, scale_);
                p.time = time;
                return p;
            }

            void Configure(esia::ContextDesc& desc)
            {
                desc.getClipboard = [this] { return GetClipboard(); };
                desc.setClipboard = [this](const std::string& s) { SetClipboard(s); };
                unsigned delay = 0, interval = 0;   // the server's key repeat (milliseconds)
                if (::XkbGetAutoRepeatRate(dpy_, XkbUseCoreKbd, &delay, &interval) && delay > 0 && interval > 0)
                {
                    desc.input.keyRepeatDelay = (float)delay / 1000.0f;
                    desc.input.keyRepeatRate = (float)interval / 1000.0f;
                }
            }

            // Every event waiting (`wait`: blocks for the next one first, while there is nothing to draw).
            void Pump(bool wait)
            {
                if (wait && !close_)
                {
                    XEvent e;
                    ::XPeekEvent(dpy_, &e);
                }
                while (::XPending(dpy_) > 0)
                {
                    XEvent e;
                    ::XNextEvent(dpy_, &e);
                    if (::XFilterEvent(&e, kXNone))   // the input method's
                        continue;
                    Handle(e);
                }
            }

            void SetCursor(esia::MouseCursor c)
            {
                if (c == cursor_)
                    return;
                cursor_ = c;
                static constexpr unsigned kShapes[] = {XC_left_ptr, XC_xterm, XC_hand2, XC_sb_h_double_arrow, XC_sb_v_double_arrow,
                                                       XC_bottom_right_corner, XC_bottom_left_corner, XC_fleur};
                const std::size_t i = std::min<std::size_t>((std::size_t)c, std::size(kShapes) - 1);
                if (!cursors_[i])
                    cursors_[i] = ::XCreateFontCursor(dpy_, kShapes[i]);   // themed (Xcursor) where the system has a theme
                ::XDefineCursor(dpy_, window_, cursors_[i]);
            }

        private:
            Atom Intern(const char* name) { return ::XInternAtom(dpy_, name, False); }

            esia::Vec2 Pos(int x, int y) const { return esia::Vec2((float)x / scale_, (float)y / scale_); }

            void Queue(esia::InputEvent e)
            {
                if (ctx_)
                    ctx_->QueueInput(std::move(e));
            }

            void Handle(XEvent& e)
            {
                switch (e.type)
                {
                case ConfigureNotify:
                    width_ = std::max(1, e.xconfigure.width);
                    height_ = std::max(1, e.xconfigure.height);
                    break;
                case MapNotify:
                    mapped_ = true;
                    break;
                case UnmapNotify:
                    mapped_ = false;
                    break;
                case ClientMessage:
                    if ((Atom)e.xclient.data.l[0] == deleteWindow_)
                        close_ = true;
                    break;
                case MotionNotify:
                    Queue(esia::InputEvent::MouseMove(Pos(e.xmotion.x, e.xmotion.y)));
                    break;
                case EnterNotify:
                    Queue(esia::InputEvent::MouseMove(Pos(e.xcrossing.x, e.xcrossing.y)));
                    break;
                case LeaveNotify:
                    // while a button is held the pointer is grabbed and still reported: a drag goes on outside
                    if (e.xcrossing.mode == NotifyNormal && !(e.xcrossing.state & (Button1Mask | Button2Mask | Button3Mask)))
                        Queue(esia::InputEvent::MouseLeave());
                    break;
                case ButtonPress:
                case ButtonRelease:
                    Button(e.xbutton, e.type == ButtonPress);
                    break;
                case KeyPress:
                case KeyRelease:
                    Key(e.xkey, e.type == KeyPress);
                    break;
                case FocusIn:
                case FocusOut:
                    if (e.xfocus.mode == NotifyGrab || e.xfocus.mode == NotifyUngrab)
                        break;   // the window manager's grabs (Alt+Tab ...) do not move the focus
                    if (ic_)
                        e.type == FocusIn ? ::XSetICFocus(ic_) : ::XUnsetICFocus(ic_);
                    if (e.type == FocusOut)
                        down_.fill(false);
                    Queue(esia::InputEvent::FocusEvent(e.type == FocusIn));
                    break;
                case SelectionRequest:
                    AnswerSelection(e.xselectionrequest);
                    break;
                case SelectionClear:
                    if (e.xselectionclear.selection == clipboard_)
                        owned_.clear();
                    break;
                default:
                    break;
                }
            }

            void Button(const XButtonEvent& b, bool down)
            {
                Queue(esia::InputEvent::MouseMove(Pos(b.x, b.y)));
                switch (b.button)
                {
                case Button1: Queue(esia::InputEvent::Button(esia::MouseButton::Left, down)); break;
                case Button2: Queue(esia::InputEvent::Button(esia::MouseButton::Middle, down)); break;
                case Button3: Queue(esia::InputEvent::Button(esia::MouseButton::Right, down)); break;
                case 8: Queue(esia::InputEvent::Button(esia::MouseButton::X1, down)); break;
                case 9: Queue(esia::InputEvent::Button(esia::MouseButton::X2, down)); break;
                // the wheel: a notch is a press (and a release, ignored); 6 / 7 tilt it left / right
                case Button4: if (down) Queue(esia::InputEvent::Wheel(0.0f, 1.0f)); break;
                case Button5: if (down) Queue(esia::InputEvent::Wheel(0.0f, -1.0f)); break;
                case 6: if (down) Queue(esia::InputEvent::Wheel(1.0f, 0.0f)); break;
                case 7: if (down) Queue(esia::InputEvent::Wheel(-1.0f, 0.0f)); break;
                default: break;
                }
            }

            void Key(XKeyEvent& k, bool down)
            {
                // the keypad's digits with Num Lock on (level 1), its navigation keys without
                KeySym sym = ::XLookupKeysym(&k, 0);
                if (sym >= XK_KP_Home && sym <= XK_KP_Delete && (k.state & Mod2Mask))
                    sym = ::XLookupKeysym(&k, 1);
                const esia::Key key = k.keycode ? KeyOf(sym) : esia::Key::None;   // keycode 0: text the input method sent
                if (key != esia::Key::None)
                {
                    const std::size_t i = (std::size_t)key;
                    std::uint32_t mods = ModsOf(k.state);
                    mods = down ? (mods | ModOfKey(key)) : (mods & ~ModOfKey(key));
                    if (!down || !down_[i])   // a repeat (pressed again without a release) is the core's to make
                        Queue(esia::InputEvent::KeyEvent(key, down, mods));
                    down_[i] = down;
                }
                if (!down || (k.state & (ControlMask | Mod1Mask)))
                    return;   // shortcuts type nothing
                char buf[64];
                int n = 0;
                KeySym ignored = 0;
                if (ic_)
                {
                    int status = 0;
                    n = ::Xutf8LookupString(ic_, &k, buf, (int)sizeof(buf) - 1, &ignored, &status);
                    if (status != XLookupChars && status != XLookupBoth)
                        n = 0;
                }
                else
                {
                    char latin1[32];   // no input method: Latin-1, as UTF-8
                    const int m = ::XLookupString(&k, latin1, (int)sizeof(latin1), &ignored, nullptr);
                    for (int i = 0; i < m && n < (int)sizeof(buf) - 3; ++i)
                    {
                        const unsigned char c = (unsigned char)latin1[i];
                        if (c < 0x80)
                            buf[n++] = (char)c;
                        else
                        {
                            buf[n++] = (char)(0xC0 | (c >> 6));
                            buf[n++] = (char)(0x80 | (c & 0x3F));
                        }
                    }
                }
                if (n > 0 && (unsigned char)buf[0] >= 0x20 && buf[0] != 0x7F)   // not Tab, Enter, Backspace, Delete
                    Queue(esia::InputEvent::TextEvent(std::string(buf, (std::size_t)n)));
            }

            // ---- the clipboard: CLIPBOARD as UTF8_STRING; ours is answered from owned_
            std::string GetClipboard()
            {
                if (::XGetSelectionOwner(dpy_, clipboard_) == window_)
                    return owned_;
                ::XConvertSelection(dpy_, clipboard_, utf8_, transfer_, window_, CurrentTime);
                ::XFlush(dpy_);
                XEvent e;
                const auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(250);
                while (!::XCheckTypedWindowEvent(dpy_, window_, SelectionNotify, &e))
                {
                    if (std::chrono::steady_clock::now() > until)
                        return {};   // no owner answered
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                if (e.xselection.property == (Atom)kXNone)
                    return {};
                Atom type = 0;
                int format = 0;
                unsigned long items = 0, left = 0;
                unsigned char* data = nullptr;
                std::string text;
                if (::XGetWindowProperty(dpy_, window_, transfer_, 0, 1 << 24, True, AnyPropertyType, &type, &format, &items, &left, &data) ==
                        0 /* Success */ && data && format == 8)
                    text.assign(reinterpret_cast<const char*>(data), items);
                if (data)
                    ::XFree(data);
                return text;
            }

            void SetClipboard(const std::string& s)
            {
                owned_ = s;
                ::XSetSelectionOwner(dpy_, clipboard_, window_, CurrentTime);
            }

            void AnswerSelection(const XSelectionRequestEvent& r)
            {
                XEvent reply = {};
                reply.xselection.type = SelectionNotify;
                reply.xselection.requestor = r.requestor;
                reply.xselection.selection = r.selection;
                reply.xselection.target = r.target;
                reply.xselection.time = r.time;
                reply.xselection.property = r.property ? r.property : r.target;
                if (r.target == targets_)
                {
                    const Atom offered[] = {targets_, utf8_, XA_STRING};
                    ::XChangeProperty(dpy_, r.requestor, reply.xselection.property, XA_ATOM, 32, PropModeReplace,
                                      reinterpret_cast<const unsigned char*>(offered), (int)std::size(offered));
                }
                else if (r.target == utf8_ || r.target == XA_STRING)
                    ::XChangeProperty(dpy_, r.requestor, reply.xselection.property, r.target, 8, PropModeReplace,
                                      reinterpret_cast<const unsigned char*>(owned_.data()), (int)owned_.size());
                else
                    reply.xselection.property = (Atom)kXNone;
                ::XSendEvent(dpy_, r.requestor, False, 0, &reply);
            }

            Display* dpy_ = nullptr;
            ::Window window_ = 0;
            XIM im_ = nullptr;
            XIC ic_ = nullptr;
            Atom deleteWindow_ = 0, clipboard_ = 0, utf8_ = 0, targets_ = 0, transfer_ = 0;
            Cursor cursors_[8] = {};
            esia::MouseCursor cursor_ = esia::MouseCursor::Arrow;
            esia::Context* ctx_ = nullptr;
            std::array<bool, (std::size_t)esia::Key::Count> down_ = {};
            std::string owned_;
            float scale_ = 1.0f;
            int width_ = 1, height_ = 1;
            bool close_ = false, mapped_ = false;
        };

        int Run(App& app, Frame& window, const Options& opt)
        {
            std::string error;
            std::unique_ptr<Host> host = CreateHost(opt.api);
            NativeWindow native;
            native.display = window.Dpy();
            native.window = window.Handle();
            int width = window.Width(), height = window.Height();
            if (!host || !host->Init(native, width, height, {opt.vsync, opt.debug}, error))
            {
                std::fprintf(stderr, "%s: %s: %s\n", app.name, opt.api.c_str(), error.c_str());
                return 1;
            }
            std::printf("%s: %s on %s, %d x %d px, scale %.2f\n", app.name, host->Name(), host->Adapter().c_str(), width, height, window.Scale());
            std::fflush(stdout);

            esia::ContextDesc cd;
            window.Configure(cd);
            esia::Context ctx(cd);
            esia::text::TextSystem* text = nullptr;
            esia::text::FontId font = 0;
#if defined(GLASS_TEXT)
            std::unique_ptr<esia::text::TextSystem> textSystem;   // FreeType, and the icon theme for kSystemSymbolsFont
            if (std::unique_ptr<esia::text::TextSystem> ft = esia::text::CreateFreeTypeTextSystem(ctx.Textures()))
                textSystem = std::make_unique<SymbolTextSystem>(std::move(ft), ctx.Textures(), &linux_icons::Rasterize);
            if (textSystem && !app.loadFonts)
                text = textSystem.get();
            else if (textSystem && (font = LoadFonts(*textSystem, opt.fonts)) != 0)
                text = textSystem.get();
            else
                std::fprintf(stderr, "%s: no font loaded (--font): labels are left out\n", app.name);
#endif
            esia::render::Renderer renderer(host->Device());
            if (app.init)
                app.init(ctx, text, font, opt.fonts);
            if (app.renderer)
                app.renderer(renderer);
            window.SetContext(&ctx);   // input flows from here on

            using Clock = std::chrono::steady_clock;
            const Clock::time_point start = Clock::now();
            Clock::time_point fpsStart = start;
            int fpsFrames = 0, frame = 0, result = 0;
            float fps = 0.0f, cpuMs = 0.0f;
            const float scale = window.Scale();
            while (!window.CloseRequested())
            {
                window.Pump(!window.Visible());   // hidden (minimized): wait for the next event
                if (window.CloseRequested())
                    break;
                if (!window.Visible())
                    continue;
                if (window.Width() != width || window.Height() != height)
                {
                    width = window.Width();
                    height = window.Height();
                    host->Resize(width, height);
                }
                if (!host->BeginFrame())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }
                const double now = std::chrono::duration<double>(Clock::now() - start).count();
                // fixed steps: frame N (from 1) is at N x dt (captures at the same moment)
                ctx.NewFrame(window.Params(opt.fixedDt > 0.0 ? (frame + 1) * opt.fixedDt : now));
                if (text && app.loadFonts)
                    text->NewFrame({scale});
                const Clock::time_point uiStart = Clock::now();
                if (app.frame)
                    app.frame(ctx, {host->Name(), host->Adapter(), width, height, scale, fps, cpuMs, &renderer.Stats()});
                ctx.EndFrame();
                cpuMs = std::chrono::duration<float, std::milli>(Clock::now() - uiStart).count();
                esia::render::RenderParams rp;
                rp.frame = host->Frame();
                if (!renderer.Render(ctx.GetDrawData(), &ctx.Textures(), host->Target(), rp))
                    std::fprintf(stderr, "%s: the device refused frame %d\n", app.name, frame);
                window.SetCursor(ctx.Requests().cursor);

                ++frame;
                const bool last = opt.frames > 0 && frame >= opt.frames;
                std::vector<std::uint8_t> pixels;
                const bool capture = last && !opt.screenshot.empty();
                if (!host->EndFrame(capture ? &pixels : nullptr))
                {
                    std::fprintf(stderr, "%s: reading the window's image back failed\n", app.name);
                    result = 1;
                }
                else if (capture)
                {
                    esia::testkit::Image image(width, height);
                    image.rgba = std::move(pixels);
                    for (std::size_t i = 3; i < image.rgba.size(); i += 4)
                        image.rgba[i] = 255;   // a window has no alpha
                    if (!esia::testkit::WritePng(opt.screenshot, image, &error))
                    {
                        std::fprintf(stderr, "%s: %s\n", app.name, error.c_str());
                        result = 1;
                    }
                    else
                        std::printf("%s: frame %d written to %s\n", app.name, frame, opt.screenshot.c_str());
                }
                ++fpsFrames;
                const double since = std::chrono::duration<double>(Clock::now() - fpsStart).count();
                if (since >= 0.5)
                {
                    fps = (float)(fpsFrames / since);
                    fpsFrames = 0;
                    fpsStart = Clock::now();
                }
                if (last)
                    break;
            }
            window.SetContext(nullptr);   // before ctx goes away
            if (app.shutdown)
                app.shutdown();

            const std::uint32_t messages = host->ValidationMessages();
            std::printf("%s: %d frames, %u validation / debug-layer messages%s\n", app.name, frame, messages, opt.debug ? "" : " (--debug to enable the layer)");
            return (opt.debug && messages > 0 && result == 0) ? 3 : result;
        }
    }

    int RunApp(int argc, char** argv, App& app)
    {
        Options opt;
        std::string problem;
        if (!Parse(app, argc, argv, opt, problem))
            return Usage(app, problem.c_str());
        Frame window;
        std::string error;
        if (!window.Create("Esia " + std::string(app.name) + " - " + CreateHost(opt.api)->Name(), opt, error))
        {
            std::fprintf(stderr, "%s: %s\n", app.name, error.c_str());
            return 1;
        }
        return Run(app, window, opt);   // the host goes before the window
    }
}
