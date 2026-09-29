// Esia - Win32 platform layer: messages -> InputEvents, PlatformRequests -> the window (esia/platform/win32.hpp).
#include "win32_platform.hpp"
#include <windowsx.h>
#include <algorithm>
#include <cmath>
#include <iterator>

namespace esia::platform::win32
{
    namespace
    {
        std::uint32_t ButtonBit(MouseButton b) { return 1u << (unsigned)b; }

        std::uint32_t CurrentMods()
        {
            // GetKeyState: the keyboard as of the message being handled (window thread), not as of "now"
            const auto down = [](int vk) { return ::GetKeyState(vk) < 0; };
            std::uint32_t m = Mod_None;
            if (down(VK_CONTROL))
                m |= Mod_Ctrl;
            if (down(VK_SHIFT))
                m |= Mod_Shift;
            if (down(VK_MENU))
                m |= Mod_Alt;
            if (down(VK_LWIN) || down(VK_RWIN))
                m |= Mod_Super;
            return m;
        }

        Key TranslateKey(WPARAM vk, LPARAM lparam)
        {
            const bool extended = (lparam & (1 << 24)) != 0;
            if (vk >= 'A' && vk <= 'Z')
                return (Key)((int)Key::A + (int)(vk - 'A'));
            if (vk >= '0' && vk <= '9')
                return (Key)((int)Key::Num0 + (int)(vk - '0'));
            if (vk >= VK_F1 && vk <= VK_F12)
                return (Key)((int)Key::F1 + (int)(vk - VK_F1));
            switch (vk)
            {
            case VK_TAB: return Key::Tab;
            case VK_LEFT: return Key::Left;
            case VK_RIGHT: return Key::Right;
            case VK_UP: return Key::Up;
            case VK_DOWN: return Key::Down;
            case VK_PRIOR: return Key::PageUp;
            case VK_NEXT: return Key::PageDown;
            case VK_HOME: return Key::Home;
            case VK_END: return Key::End;
            case VK_INSERT: return Key::Insert;
            case VK_DELETE: return Key::Delete;
            case VK_BACK: return Key::Backspace;
            case VK_SPACE: return Key::Space;
            case VK_RETURN: return Key::Enter;   // the keypad's Enter too (extended)
            case VK_ESCAPE: return Key::Escape;
            // the messages carry VK_SHIFT / VK_CONTROL / VK_MENU: the side is in the scan code / extended bit
            case VK_SHIFT: return ::MapVirtualKeyW((UINT)(lparam >> 16) & 0xFF, MAPVK_VSC_TO_VK_EX) == VK_RSHIFT ? Key::RightShift : Key::LeftShift;
            case VK_CONTROL: return extended ? Key::RightCtrl : Key::LeftCtrl;
            case VK_MENU: return extended ? Key::RightAlt : Key::LeftAlt;
            case VK_LSHIFT: return Key::LeftShift;
            case VK_RSHIFT: return Key::RightShift;
            case VK_LCONTROL: return Key::LeftCtrl;
            case VK_RCONTROL: return Key::RightCtrl;
            case VK_LMENU: return Key::LeftAlt;
            case VK_RMENU: return Key::RightAlt;
            case VK_LWIN: return Key::LeftSuper;
            case VK_RWIN: return Key::RightSuper;
            default: return Key::None;   // not in esia::Key (keypad, punctuation, F13+ ...)
            }
        }

        // Bytes of UTF-8 in the first `units` UTF-16 units of `s` (the IME's caret -> InputEvent::imeCursor).
        int Utf8Offset(const std::wstring& s, int units)
        {
            units = std::clamp(units, 0, (int)s.size());
            return units > 0 ? ::WideCharToMultiByte(CP_UTF8, 0, s.data(), units, nullptr, 0, nullptr, nullptr) : 0;
        }

        std::wstring ReadCompositionString(HIMC himc, DWORD kind)
        {
            const LONG bytes = ::ImmGetCompositionStringW(himc, kind, nullptr, 0);
            if (bytes <= 0)
                return {};
            std::wstring s((std::size_t)bytes / sizeof(wchar_t), L'\0');
            ::ImmGetCompositionStringW(himc, kind, s.data(), (DWORD)bytes);
            return s;
        }
    }

    std::wstring Widen(std::string_view utf8)
    {
        if (utf8.empty())
            return {};
        const int n = ::MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int)utf8.size(), nullptr, 0);
        std::wstring w((std::size_t)std::max(n, 0), L'\0');
        if (n > 0)
            ::MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int)utf8.size(), w.data(), n);
        return w;
    }

    std::string Narrow(const wchar_t* utf16, std::size_t count)
    {
        if (count == 0)
            return {};
        const int n = ::WideCharToMultiByte(CP_UTF8, 0, utf16, (int)count, nullptr, 0, nullptr, nullptr);
        std::string s((std::size_t)std::max(n, 0), '\0');
        if (n > 0)
            ::WideCharToMultiByte(CP_UTF8, 0, utf16, (int)count, s.data(), n, nullptr, nullptr);
        return s;
    }

    // ================================================================== Impl
    Platform::Impl::Impl(const Desc& d)
        : desc(d),
          updateMessage(::RegisterWindowMessageW(L"EsiaPlatformWin32.Update")),
          destroyMessage(::RegisterWindowMessageW(L"EsiaPlatformWin32.Destroy")),
          uiScale(d.uiScale > 0.0f ? d.uiScale : 1.0f)
    {
        static const LPCWSTR shapes[] = {IDC_ARROW, IDC_IBEAM, IDC_HAND, IDC_SIZEWE, IDC_SIZENS, IDC_SIZENWSE, IDC_SIZENESW, IDC_SIZEALL};
        static_assert(std::size(shapes) == (std::size_t)MouseCursor::ResizeAll + 1, "one system cursor per MouseCursor");
        for (std::size_t i = 0; i < std::size(shapes); ++i)
            cursors[i] = ::LoadCursorW(nullptr, shapes[i]);
    }

    void Platform::Impl::Queue(InputEvent e)
    {
        std::lock_guard lock(contextMutex);
        if (context)
            context->QueueInput(std::move(e));
    }

    void Platform::Impl::Publish(int width, int height, bool minimized)
    {
        std::lock_guard lock(frameMutex);
        if (!minimized)   // a minimized window reports 0 x 0: keep the size it had
        {
            frame.width = width;
            frame.height = height;
        }
        frame.minimized = minimized;
        frame.dpiScale = (float)dpi / (float)USER_DEFAULT_SCREEN_DPI;
    }

    void Platform::Impl::PublishClientRect(HWND w)
    {
        RECT rc = {};
        ::GetClientRect(w, &rc);
        Publish(rc.right - rc.left, rc.bottom - rc.top, ::IsIconic(w) != FALSE);
    }

    void Platform::Impl::PublishFocus(bool focused)
    {
        std::lock_guard lock(frameMutex);
        frame.focused = focused;
    }

    // ------------------------------------------------------------------ mouse
    void Platform::Impl::OnMouseMove(LPARAM lparam)
    {
        const float s = Scale();
        const Vec2 p((float)GET_X_LPARAM(lparam) / s, (float)GET_Y_LPARAM(lparam) / s);
        if (p == lastMousePos)
            return;
        lastMousePos = p;
        // While captured the position may be negative (dragged past the left / top edge), which the core also
        // reads as "outside the window": the drag goes on, hover is lost until the mouse is back (core request).
        Queue(InputEvent::MouseMove(p));
    }

    void Platform::Impl::OnMouseButton(HWND w, MouseButton b, bool down, LPARAM lparam)
    {
        // the button's own position first: a click without a move before it lands where it happened
        OnMouseMove(lparam);
        const std::uint32_t bit = ButtonBit(b);
        if (down)
        {
            // capture while any button is down, so the release (and the drag) arrive even outside the window
            if (buttonsDown == 0 && ::GetCapture() == nullptr)
            {
                ::SetCapture(w);
                capturedMouse = true;
            }
            buttonsDown |= bit;
            Queue(InputEvent::Button(b, true));
            return;
        }
        if (!(buttonsDown & bit))   // pressed elsewhere, released over the window: not the UI's click
            return;
        buttonsDown &= ~bit;
        Queue(InputEvent::Button(b, false));
        if (buttonsDown == 0)
        {
            if (capturedMouse && ::GetCapture() == w)
                ::ReleaseCapture();
            capturedMouse = false;
            LeaveIfOutside(w);
        }
    }

    void Platform::Impl::ReleaseButtons()
    {
        for (int i = 0; i < (int)MouseButton::Count; ++i)
            if (buttonsDown & ButtonBit((MouseButton)i))
                Queue(InputEvent::Button((MouseButton)i, false));
        buttonsDown = 0;
        capturedMouse = false;
    }

    // WM_MOUSELEAVE is held back while the mouse is captured: after the release, tell the core if it is outside.
    void Platform::Impl::LeaveIfOutside(HWND w)
    {
        POINT pt;
        RECT rc;
        if (!::GetCursorPos(&pt) || !::GetClientRect(w, &rc))
            return;
        const POINT screen = pt;
        ::ScreenToClient(w, &pt);
        if (::PtInRect(&rc, pt) && ::WindowFromPoint(screen) == w)
            return;
        mouseInside = false;
        lastMousePos = Vec2(-1, -1);
        Queue(InputEvent::MouseMove(lastMousePos));
    }

    // ------------------------------------------------------------------ keyboard, text, focus
    void Platform::Impl::OnKey(bool down, WPARAM vk, LPARAM lparam)
    {
        // VK_PROCESSKEY: the IME took the key (composition, candidate selection); the UI must not see it. Its
        // release comes with the real key code and is dropped below: that key was never down for the UI.
        if (vk == VK_PROCESSKEY)
            return;
        const std::uint32_t mods = CurrentMods();
        const auto set = [&](Key k, bool isDown) {
            if (k == Key::None || keysDown[(std::size_t)k] == isDown)   // auto-repeat: the core repeats itself
                return;
            keysDown[(std::size_t)k] = isDown;
            Queue(InputEvent::KeyEvent(k, isDown, mods));
        };
        set(TranslateKey(vk, lparam), down);
        // with both shift keys held, Windows sends one key-up for the two
        if (!down && vk == VK_SHIFT)
        {
            if (::GetKeyState(VK_LSHIFT) >= 0)
                set(Key::LeftShift, false);
            if (::GetKeyState(VK_RSHIFT) >= 0)
                set(Key::RightShift, false);
        }
    }

    void Platform::Impl::OnChar(wchar_t c)
    {
        char32_t cp = c;
        if (IS_HIGH_SURROGATE(c))
        {
            highSurrogate = c;   // the low half follows in the next WM_CHAR
            return;
        }
        if (IS_LOW_SURROGATE(c))
        {
            if (!highSurrogate)
                return;
            cp = 0x10000 + (((char32_t)highSurrogate - 0xD800) << 10) + ((char32_t)c - 0xDC00);
        }
        highSurrogate = 0;
        // control characters (Enter, Tab, Backspace, Ctrl+letter, Ctrl+Backspace's DEL) reach the UI as keys
        if (cp < 0x20 || cp == 0x7F)
            return;
        std::string utf8;
        AppendUtf32(utf8, cp);
        Queue(InputEvent::TextEvent(std::move(utf8)));
    }

    void Platform::Impl::LoseFocus(HWND w)
    {
        // the core releases every button and key on focus loss; the window thread forgets them too
        keysDown.reset();
        buttonsDown = 0;
        highSurrogate = 0;
        if (capturedMouse && ::GetCapture() == w)
            ::ReleaseCapture();
        capturedMouse = false;
        EndComposition();
        Queue(InputEvent::FocusEvent(false));
        PublishFocus(false);
    }

    // ------------------------------------------------------------------ requests
    bool Platform::Impl::OwnsCursor() const
    {
        return desc.cursor == CursorMode::Always || (desc.cursor == CursorMode::WhenCapturing && wantMouse.load());
    }

    void Platform::Impl::ApplyPending(HWND w)
    {
        PlatformRequests r;
        {
            std::lock_guard lock(requestMutex);
            r = requests;
            updatePosted = false;
        }
        if (r.cursor != cursor)
        {
            cursor = r.cursor;
            // WM_SETCURSOR comes only with the next mouse move: a shape that changes under a still mouse (a
            // button that became a text field, a drag that ended) changes now
            if (mouseInside && OwnsCursor())
                ::SetCursor(cursors[(int)cursor]);
        }
        const bool moved = r.imeRect != imeRect;
        imeRect = r.imeRect;
        if (r.wantTextInput != textInput)
            SetTextInput(w, r.wantTextInput);   // places the IME when it turns on
        else if (textInput && moved)
            PlaceIme(w);
    }

    // ------------------------------------------------------------------ IME
    void Platform::Impl::SetTextInput(HWND w, bool on)
    {
        textInput = on;
        if (on)
        {
            if (desc.imeOnlyForTextInput)
                ::ImmAssociateContextEx(w, nullptr, IACE_DEFAULT);   // the window's own input context again
            PlaceIme(w);
            return;
        }
        // The text field lost the keyboard: a composition in progress is cancelled, not committed - its text would
        // arrive in the next frame, when no field has the focus any more.
        if (composing)
        {
            if (HIMC himc = ::ImmGetContext(w))
            {
                ::ImmNotifyIME(himc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
                ::ImmReleaseContext(w, himc);
            }
            EndComposition();
        }
        if (desc.imeOnlyForTextInput)
            ::ImmAssociateContextEx(w, nullptr, 0);   // no input context: the IME is off for the window
        if (caretHeight > 0)
        {
            ::DestroyCaret();
            caretHeight = 0;
        }
    }

    RECT Platform::Impl::CaretPixels() const
    {
        const float s = Scale();
        RECT r;
        r.left = (LONG)std::floor(imeRect.min.x * s);
        r.top = (LONG)std::floor(imeRect.min.y * s);
        r.right = std::max((LONG)std::ceil(imeRect.max.x * s), r.left + 1);
        r.bottom = std::max((LONG)std::ceil(imeRect.max.y * s), r.top + 1);
        return r;
    }

    // Every way an IME finds the caret, so the candidate window opens under it with IMM32 IMEs and TSF IMEs
    // alike (Microsoft Pinyin runs through the IMM32 compatibility layer here): the composition form, the
    // candidate form excluding the caret line, the hidden system caret (IMEs and accessibility tools that follow
    // GetCaretPos) and IMR_QUERYCHARPOSITION (HandleMessage).
    void Platform::Impl::PlaceIme(HWND w)
    {
        if (!textInput)
            return;
        const RECT r = CaretPixels();
        if (HIMC himc = ::ImmGetContext(w))
        {
            COMPOSITIONFORM cf = {};
            cf.dwStyle = CFS_FORCE_POSITION;
            cf.ptCurrentPos = {r.left, r.top};
            ::ImmSetCompositionWindow(himc, &cf);
            if (!desc.inlineComposition)
            {
                // the IME's own composition window: text as tall as the caret line
                LOGFONTW lf = {};
                if (::ImmGetCompositionFontW(himc, &lf))
                {
                    lf.lfHeight = -(r.bottom - r.top);
                    lf.lfWidth = 0;
                    ::ImmSetCompositionFontW(himc, &lf);
                }
            }
            CANDIDATEFORM cand = {};
            cand.dwIndex = 0;
            cand.dwStyle = CFS_EXCLUDE;   // below the caret line, never on top of it
            cand.ptCurrentPos = {r.left, r.bottom};
            cand.rcArea = r;
            ::ImmSetCandidateWindow(himc, &cand);
            ::ImmReleaseContext(w, himc);
        }
        const int h = (int)(r.bottom - r.top);
        if (caretHeight != h)
        {
            if (caretHeight > 0)
                ::DestroyCaret();
            caretHeight = ::CreateCaret(w, nullptr, 1, h) ? h : 0;   // never shown: it only marks the position
        }
        if (caretHeight > 0)
            ::SetCaretPos(r.left, r.top);
    }

    void Platform::Impl::ReadComposition(HWND w, LPARAM flags)
    {
        HIMC himc = ::ImmGetContext(w);
        if (!himc)
            return;
        // the committed text first: a result and the next composition can come in one message
        if (flags & GCS_RESULTSTR)
        {
            const std::wstring result = ReadCompositionString(himc, GCS_RESULTSTR);
            if (!result.empty())
                Queue(InputEvent::TextEvent(Narrow(result.data(), result.size())));
        }
        if (flags & GCS_COMPSTR)
        {
            const std::wstring comp = ReadCompositionString(himc, GCS_COMPSTR);
            const int caret = (flags & GCS_CURSORPOS) ? (int)LOWORD(::ImmGetCompositionStringW(himc, GCS_CURSORPOS, nullptr, 0)) : (int)comp.size();
            composing = !comp.empty();
            Queue(InputEvent::Composition(Narrow(comp.data(), comp.size()), Utf8Offset(comp, caret)));
        }
        else if (composing)   // a result without a new composition, or none at all (lParam 0: cancelled)
            EndComposition();
        ::ImmReleaseContext(w, himc);
    }

    void Platform::Impl::EndComposition()
    {
        if (!composing)
            return;
        composing = false;
        Queue(InputEvent::Composition(std::string(), 0));
    }

    // ================================================================== Platform
    Platform::Platform(const Desc& desc) : impl_(std::make_unique<Impl>(desc)) {}

    Platform::~Platform() = default;

    bool Platform::Attach(void* hwndPtr)
    {
        Impl& m = *impl_;
        HWND w = static_cast<HWND>(hwndPtr);
        if (!w || m.hwnd.load())
            return false;
        m.dpi = ::GetDpiForWindow(w);
        if (m.dpi == 0)
            m.dpi = USER_DEFAULT_SCREEN_DPI;
        m.hwnd = w;
        m.PublishClientRect(w);
        const bool focused = ::GetFocus() == w;
        m.PublishFocus(focused);
        if (focused)
            m.Queue(InputEvent::FocusEvent(true));
        if (m.desc.imeOnlyForTextInput)
            ::ImmAssociateContextEx(w, nullptr, 0);
        return true;
    }

    void Platform::Detach()
    {
        Impl& m = *impl_;
        HWND w = m.hwnd.load();
        if (!w)
            return;
        m.LoseFocus(w);
        if (m.caretHeight > 0)
            ::DestroyCaret();
        if (m.desc.imeOnlyForTextInput)
            ::ImmAssociateContextEx(w, nullptr, IACE_DEFAULT);
        m.hwnd = nullptr;
        m.owned = false;
        m.trackingLeave = m.mouseInside = m.textInput = false;
        m.caretHeight = 0;
        m.lastMousePos = Vec2(-1, -1);
        m.cursor = MouseCursor::Arrow;
        m.imeRect = Rect();
        std::lock_guard lock(m.requestMutex);
        m.requests = PlatformRequests();
        m.updatePosted = false;
    }

    bool Platform::HandleMessage(void* hwndPtr, std::uint32_t msg, std::uintptr_t wparamIn, std::intptr_t lparamIn, std::intptr_t* result)
    {
        Impl& m = *impl_;
        HWND w = static_cast<HWND>(hwndPtr);
        if (!w || w != m.hwnd.load())
            return false;
        const WPARAM wparam = (WPARAM)wparamIn;
        const LPARAM lparam = (LPARAM)lparamIn;
        std::intptr_t unused = 0;
        std::intptr_t& res = result ? *result : unused;
        res = 0;

        if (msg == m.updateMessage)
        {
            m.ApplyPending(w);
            return true;
        }
        if (msg == m.destroyMessage)
        {
            if (m.owned)
                ::DestroyWindow(w);
            return true;
        }
        // IME messages are the UI's only while it takes text (or finishes a composition it started): otherwise
        // they go on to the host, e.g. a game's own chat box with Desc::imeOnlyForTextInput off
        const bool imeIsOurs = m.textInput || m.composing;

        switch (msg)
        {
        // ---- mouse
        case WM_MOUSEMOVE:
            if (!m.trackingLeave)
            {
                TRACKMOUSEEVENT tme = {sizeof(tme), TME_LEAVE, w, 0};
                m.trackingLeave = ::TrackMouseEvent(&tme) != FALSE;
            }
            m.mouseInside = true;
            m.OnMouseMove(lparam);
            return false;
        case WM_MOUSELEAVE:
            m.trackingLeave = false;
            if (m.buttonsDown == 0)   // captured: moves keep coming, LeaveIfOutside runs at the release
            {
                m.mouseInside = false;
                m.lastMousePos = Vec2(-1, -1);
                m.Queue(InputEvent::MouseMove(m.lastMousePos));
            }
            return false;
        // double clicks come as a second down (with CS_DBLCLKS): the core detects double clicks itself
        case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK: m.OnMouseButton(w, MouseButton::Left, true, lparam); return false;
        case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK: m.OnMouseButton(w, MouseButton::Right, true, lparam); return false;
        case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK: m.OnMouseButton(w, MouseButton::Middle, true, lparam); return false;
        case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
            m.OnMouseButton(w, GET_XBUTTON_WPARAM(wparam) == XBUTTON1 ? MouseButton::X1 : MouseButton::X2, true, lparam);
            return false;
        case WM_LBUTTONUP: m.OnMouseButton(w, MouseButton::Left, false, lparam); return false;
        case WM_RBUTTONUP: m.OnMouseButton(w, MouseButton::Right, false, lparam); return false;
        case WM_MBUTTONUP: m.OnMouseButton(w, MouseButton::Middle, false, lparam); return false;
        case WM_XBUTTONUP:
            m.OnMouseButton(w, GET_XBUTTON_WPARAM(wparam) == XBUTTON1 ? MouseButton::X1 : MouseButton::X2, false, lparam);
            return false;
        case WM_CAPTURECHANGED:
            // another window took our capture mid-press (a dialog, a drag and drop): the press is over. A capture
            // the host set itself is the host's business.
            if ((HWND)lparam != w && m.capturedMouse)
            {
                m.ReleaseButtons();
                m.LeaveIfOutside(w);
            }
            return false;
        case WM_MOUSEWHEEL:
            m.Queue(InputEvent::Wheel(0.0f, (float)GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA));
            return false;
        case WM_MOUSEHWHEEL:   // Windows: > 0 = tilted right; esia: x > 0 scrolls toward the left, like y > 0 toward the top
            m.Queue(InputEvent::Wheel(-(float)GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA, 0.0f));
            return false;
        case WM_SETCURSOR:
            if (LOWORD(lparam) == HTCLIENT && m.OwnsCursor())
            {
                ::SetCursor(m.cursors[(int)m.cursor]);
                res = TRUE;
                return true;
            }
            return false;

        // ---- keyboard, text, focus
        case WM_KEYDOWN: case WM_SYSKEYDOWN:
            m.OnKey(true, wparam, lparam);
            return false;
        case WM_KEYUP: case WM_SYSKEYUP:
            m.OnKey(false, wparam, lparam);
            return false;
        case WM_CHAR:
            m.OnChar((wchar_t)wparam);
            return false;
        case WM_SETFOCUS:
            m.Queue(InputEvent::FocusEvent(true));
            m.PublishFocus(true);
            return false;
        case WM_KILLFOCUS:
            m.LoseFocus(w);
            return false;

        // ---- IME
        case WM_IME_SETCONTEXT:
            if (!m.textInput || !m.desc.inlineComposition)
                return false;
            // the UI draws the composition: the IME keeps only its candidate window
            res = ::DefWindowProcW(w, msg, wparam, lparam & ~(LPARAM)ISC_SHOWUICOMPOSITIONWINDOW);
            return true;
        case WM_IME_STARTCOMPOSITION:
            if (!imeIsOurs)
                return false;
            m.PlaceIme(w);
            return m.desc.inlineComposition;   // DefWindowProc would open the IME's composition window
        case WM_IME_COMPOSITION:
            if (!imeIsOurs)
                return false;
            m.PlaceIme(w);
            if (!m.desc.inlineComposition)
                return false;   // the IME shows it; the result arrives as WM_IME_CHAR -> WM_CHAR
            // answered here, so DefWindowProc does not turn the result into WM_IME_CHAR / WM_CHAR a second time
            m.ReadComposition(w, lparam);
            return true;
        case WM_IME_ENDCOMPOSITION:
            if (!imeIsOurs)
                return false;
            m.EndComposition();
            return m.desc.inlineComposition;
        case WM_IME_NOTIFY:
            if (imeIsOurs && (wparam == IMN_OPENCANDIDATE || wparam == IMN_CHANGECANDIDATE))
                m.PlaceIme(w);
            return false;
        case WM_IME_REQUEST:
            if (m.textInput && wparam == IMR_QUERYCHARPOSITION)
            {
                // TSF IMEs ask where the text is: the caret, in screen pixels (per-monitor aware window)
                auto* cp = reinterpret_cast<IMECHARPOSITION*>(lparam);
                if (!cp || cp->dwSize < sizeof(IMECHARPOSITION))
                    return false;
                const RECT r = m.CaretPixels();
                POINT pt = {r.left, r.top};
                ::ClientToScreen(w, &pt);
                cp->pt = pt;
                cp->cLineHeight = (UINT)(r.bottom - r.top);
                RECT doc = {};
                ::GetClientRect(w, &doc);
                ::MapWindowPoints(w, nullptr, reinterpret_cast<POINT*>(&doc), 2);
                cp->rcDocument = doc;
                res = TRUE;
                return true;
            }
            return false;

        // ---- window
        case WM_SIZE:
            m.Publish((int)LOWORD(lparam), (int)HIWORD(lparam), wparam == SIZE_MINIMIZED);
            return false;
        case WM_DPICHANGED:
        {
            m.dpi = HIWORD(wparam);
            if (m.owned)
            {
                // the size Windows suggests for the new monitor keeps the window the same size in UI units
                const RECT* r = reinterpret_cast<const RECT*>(lparam);
                ::SetWindowPos(w, nullptr, r->left, r->top, r->right - r->left, r->bottom - r->top, SWP_NOZORDER | SWP_NOACTIVATE);
            }
            m.PublishClientRect(w);   // the size and the scale change together, even without a WM_SIZE
            m.lastMousePos = Vec2(-1, -1);
            m.PlaceIme(w);
            return m.owned;
        }
        case WM_NCDESTROY:
            Detach();
            return false;
        default:
            break;
        }

        if (!m.owned)
            return false;
        switch (msg)
        {
        case WM_CLOSE:
            m.closeRequested = true;   // the render thread releases the device, then calls DestroyAppWindow
            return true;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return true;
        case WM_ERASEBKGND:
            res = 1;   // the renderer paints every pixel: no flash of the class brush on resize
            return true;
        case WM_SYSCOMMAND:
            // no menu bar: Alt alone (or Alt+letter) would enter menu mode and swallow the next keys; Alt+Space
            // still opens the system menu
            return (wparam & 0xFFF0) == SC_KEYMENU && lparam != ' ';
        default:
            return false;
        }
    }

    void Platform::SetContext(Context* ctx)
    {
        std::lock_guard lock(impl_->contextMutex);
        impl_->context = ctx;
    }

    FrameInfo Platform::Frame() const
    {
        FrameInfo f;
        {
            std::lock_guard lock(impl_->frameMutex);
            f = impl_->frame;
        }
        f.scale = f.dpiScale * impl_->uiScale.load();
        return f;
    }

    void Platform::ApplyRequests(const PlatformRequests& r)
    {
        Impl& m = *impl_;
        m.wantMouse = r.wantCaptureMouse;
        m.wantKeyboard = r.wantCaptureKeyboard;
        m.wantText = r.wantTextInput;
        HWND w = m.hwnd.load();
        if (!w)
            return;
        std::lock_guard lock(m.requestMutex);
        const bool changed = r.cursor != m.requests.cursor || r.wantTextInput != m.requests.wantTextInput ||
                             (r.wantTextInput && r.imeRect != m.requests.imeRect);
        m.requests = r;
        // one message in flight at most: the window thread applies whatever is latest when it gets there
        if (changed && !m.updatePosted)
            m.updatePosted = ::PostMessageW(w, m.updateMessage, 0, 0) != FALSE;
    }

    void Platform::SetUiScale(float scale) { impl_->uiScale = scale > 0.0f ? scale : 1.0f; }
    bool Platform::WantCaptureMouse() const { return impl_->wantMouse.load(); }
    bool Platform::WantCaptureKeyboard() const { return impl_->wantKeyboard.load(); }
    bool Platform::WantTextInput() const { return impl_->wantText.load(); }
    bool Platform::CloseRequested() const { return impl_->closeRequested.load(); }
    void* Platform::Hwnd() const { return impl_->hwnd.load(); }
}
