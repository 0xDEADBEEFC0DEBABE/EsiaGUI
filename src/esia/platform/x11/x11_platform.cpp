// Esia - the X11 platform layer (include/esia/platform/x11.hpp): a window's events into the core's input, the core's
// requests onto the window. Xlib on the UI thread only.
#include "esia/platform/x11.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <clocale>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iterator>
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

namespace esia::platform::x11
{
    namespace
    {
        constexpr long kXNone = 0L;   // Xlib's None

        // The desktop's UI scale: Xft.dpi / 96 (GNOME, KDE and others publish their scaling there), else 1.
        float DesktopScale(::Display* dpy)
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
    }

    struct Platform::Impl
    {
        ~Impl()
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
            if (window_ && ownsWindow_)
                ::XDestroyWindow(dpy_, window_);
            if (ownsDisplay_)
                ::XCloseDisplay(dpy_);
        }

        bool Create(const WindowDesc& wd, std::string& error)
        {
            dpy_ = ::XOpenDisplay(nullptr);
            if (!dpy_)
            {
                error = "cannot open the X display (DISPLAY is not set, or no X server / XWayland)";
                return false;
            }
            ownsDisplay_ = true;
            desktopScale_ = DesktopScale(dpy_);
            scale_ = wd.scale > 0.0f ? wd.scale : desktopScale_ * uiScale_;
            width_ = std::max(1, (int)std::lround(wd.width * scale_));
            height_ = std::max(1, (int)std::lround(wd.height * scale_));
            const std::string& title = wd.title;
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
            std::string resName = title, resClass = wd.appClass;
            hint.res_name = resName.data();
            hint.res_class = resClass.data();
            ::XSetClassHint(dpy_, window_, &hint);
            deleteWindow_ = Intern("WM_DELETE_WINDOW");
            ::XSetWMProtocols(dpy_, window_, &deleteWindow_, 1);
            Serve(swa.event_mask);
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

        // The window's input: key repeat, the input method, the clipboard's atoms, the event mask.
        void Serve(long mask)
        {
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
            unsigned long filter = 0;   // the events the input method wants too
            if (ic_)
                ::XGetICValues(ic_, XNFilterEvents, &filter, nullptr);
            ::XSelectInput(dpy_, window_, mask | (long)filter);
            clipboard_ = Intern("CLIPBOARD");
            utf8_ = Intern("UTF8_STRING");
            targets_ = Intern("TARGETS");
            transfer_ = Intern("ESIA_CLIPBOARD");
        }

        bool Attach(::Display* dpy, ::Window window, std::string& error)
        {
            if (window_)
            {
                error = "a window is attached already";
                return false;
            }
            dpy_ = dpy;
            window_ = window;
            ownsWindow_ = false;
            desktopScale_ = DesktopScale(dpy_);
            scale_ = desktopScale_ * uiScale_;
            XWindowAttributes wa = {};
            ::XGetWindowAttributes(dpy_, window_, &wa);
            width_ = std::max(1, wa.width);
            height_ = std::max(1, wa.height);
            mapped_ = wa.map_state == IsViewable;
            Serve(wa.your_event_mask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask |
                  LeaveWindowMask | FocusChangeMask | StructureNotifyMask);
            return true;
        }

        ::Display* Dpy() const { return dpy_; }
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
                focused_ = e.type == FocusIn;
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

        ::Display* dpy_ = nullptr;
        ::Window window_ = 0;
        XIM im_ = nullptr;
        XIC ic_ = nullptr;
        Atom deleteWindow_ = 0, clipboard_ = 0, utf8_ = 0, targets_ = 0, transfer_ = 0;
        Cursor cursors_[8] = {};
        esia::MouseCursor cursor_ = esia::MouseCursor::Arrow;
        esia::Context* ctx_ = nullptr;
        std::array<bool, (std::size_t)esia::Key::Count> down_ = {};
        std::string owned_;
        float scale_ = 1.0f, desktopScale_ = 1.0f, uiScale_ = 1.0f;
        int width_ = 1, height_ = 1;
        bool close_ = false, mapped_ = false, focused_ = false;
        bool ownsDisplay_ = false, ownsWindow_ = true;
        bool captureMouse_ = false, captureKeyboard_ = false;
    };

    Platform::Platform(const Desc& desc) : impl_(std::make_unique<Impl>()) { impl_->uiScale_ = desc.uiScale > 0.0f ? desc.uiScale : 1.0f; }
    Platform::~Platform() = default;

    bool Platform::CreateAppWindow(const WindowDesc& desc, std::string* error)
    {
        std::string e;
        const bool ok = impl_->Create(desc, e);
        if (!ok && error)
            *error = e;
        return ok;
    }

    bool Platform::Attach(void* display, unsigned long window, std::string* error)
    {
        std::string e;
        const bool ok = display && window && impl_->Attach(static_cast<::Display*>(display), window, e);
        if (!ok && error)
            *error = e.empty() ? "no display or window" : e;
        return ok;
    }

    void Platform::Detach()
    {
        Impl& m = *impl_;
        if (m.ic_)
            ::XDestroyIC(m.ic_);
        if (m.im_)
            ::XCloseIM(m.im_);
        m.ic_ = nullptr;
        m.im_ = nullptr;
        if (!m.ownsWindow_)
        {
            m.window_ = 0;
            m.dpy_ = nullptr;
        }
    }

    void* Platform::Display() const { return impl_->dpy_; }
    unsigned long Platform::Window() const { return impl_->window_; }

    bool Platform::Pump(bool wait)
    {
        impl_->Pump(wait);
        return !impl_->close_;
    }

    bool Platform::HandleEvent(void* xevent)
    {
        XEvent& e = *static_cast<XEvent*>(xevent);
        if (::XFilterEvent(&e, kXNone))
            return true;
        impl_->Handle(e);
        return false;
    }

    void Platform::SetContext(Context* ctx) { impl_->SetContext(ctx); }
    void Platform::Configure(ContextDesc& desc) { impl_->Configure(desc); }

    FrameInfo Platform::Frame() const
    {
        const Impl& m = *impl_;
        FrameInfo f;
        f.width = m.width_;
        f.height = m.height_;
        f.scale = m.scale_;
        f.minimized = !m.mapped_;
        f.focused = m.focused_;
        return f;
    }

    void Platform::ApplyRequests(const PlatformRequests& requests)
    {
        impl_->SetCursor(requests.cursor);
        impl_->captureMouse_ = requests.wantCaptureMouse;
        impl_->captureKeyboard_ = requests.wantCaptureKeyboard;
    }

    void Platform::SetUiScale(float scale)
    {
        Impl& m = *impl_;
        m.uiScale_ = scale > 0.0f ? scale : 1.0f;
        m.scale_ = m.desktopScale_ * m.uiScale_;
    }

    bool Platform::CloseRequested() const { return impl_->close_; }
    bool Platform::WantCaptureMouse() const { return impl_->captureMouse_; }
    bool Platform::WantCaptureKeyboard() const { return impl_->captureKeyboard_; }
}
