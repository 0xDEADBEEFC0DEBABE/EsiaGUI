// Esia - Win32 platform layer: the app window, the message loop, DPI awareness and the clipboard.
#include "win32_platform.hpp"
#include <dwmapi.h>
#include <algorithm>

namespace esia::platform::win32
{
    namespace
    {
        constexpr const wchar_t* kWindowClass = L"EsiaAppWindow";
        constexpr DWORD kDwmUseImmersiveDarkMode = 20;   // DWMWA_USE_IMMERSIVE_DARK_MODE (Windows 11 SDK; not in mingw-w64)

        LRESULT CALLBACK AppWindowProc(HWND w, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            if (msg == WM_NCCREATE)
            {
                const auto* cs = reinterpret_cast<const CREATESTRUCTW*>(lparam);
                ::SetWindowLongPtrW(w, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
            }
            std::intptr_t result = 0;
            if (auto* platform = reinterpret_cast<Platform*>(::GetWindowLongPtrW(w, GWLP_USERDATA)))
                if (platform->HandleMessage(w, msg, wparam, lparam, &result))
                    return (LRESULT)result;
            return ::DefWindowProcW(w, msg, wparam, lparam);
        }

        bool OpenClipboardFor(HWND w)
        {
            // another process may hold the clipboard for a moment (clipboard managers, remote desktop)
            for (int attempt = 0; attempt < 5; ++attempt)
            {
                if (::OpenClipboard(w))
                    return true;
                ::Sleep(2);
            }
            return false;
        }

        std::string GetClipboardUtf8(HWND w)
        {
            std::string out;
            if (!OpenClipboardFor(w))
                return out;
            if (HANDLE h = ::GetClipboardData(CF_UNICODETEXT))
                if (const auto* text = static_cast<const wchar_t*>(::GlobalLock(h)))
                {
                    std::wstring s;
                    for (const wchar_t* c = text; *c; ++c)   // "\r\n" -> "\n"
                        if (!(*c == L'\r' && c[1] == L'\n'))
                            s.push_back(*c);
                    ::GlobalUnlock(h);
                    out = Narrow(s.data(), s.size());
                }
            ::CloseClipboard();
            return out;
        }

        void SetClipboardUtf8(HWND w, const std::string& utf8)
        {
            std::wstring s;
            for (wchar_t c : Widen(utf8))   // "\n" -> "\r\n", what other Windows programs paste
            {
                if (c == L'\n' && (s.empty() || s.back() != L'\r'))
                    s.push_back(L'\r');
                s.push_back(c);
            }
            HGLOBAL mem = ::GlobalAlloc(GMEM_MOVEABLE, (s.size() + 1) * sizeof(wchar_t));
            if (!mem)
                return;
            if (auto* dst = static_cast<wchar_t*>(::GlobalLock(mem)))
            {
                std::copy(s.c_str(), s.c_str() + s.size() + 1, dst);
                ::GlobalUnlock(mem);
            }
            // the window owns what it puts there (with a null owner EmptyClipboard makes SetClipboardData fail)
            if (w && OpenClipboardFor(w))
            {
                ::EmptyClipboard();
                if (::SetClipboardData(CF_UNICODETEXT, mem))
                    mem = nullptr;   // the clipboard owns it now
                ::CloseClipboard();
            }
            if (mem)
                ::GlobalFree(mem);
        }
    }

    void Platform::Configure(ContextDesc& desc)
    {
        Impl* m = impl_.get();
        desc.getClipboard = [m] { return GetClipboardUtf8(m->hwnd.load()); };
        desc.setClipboard = [m](const std::string& s) { SetClipboardUtf8(m->hwnd.load(), s); };

        desc.input.doubleClickTime = (float)::GetDoubleClickTime() / 1000.0f;
        // SPI_GETKEYBOARDDELAY: 0 .. 3 = 250 .. 1000 ms; SPI_GETKEYBOARDSPEED: 0 .. 31 = about 2.5 .. 30 repeats a second
        UINT delay = 1;
        DWORD speed = 31;
        if (::SystemParametersInfoW(SPI_GETKEYBOARDDELAY, 0, &delay, 0))
            desc.input.keyRepeatDelay = 0.25f * (float)(std::min<UINT>(delay, 3) + 1);
        if (::SystemParametersInfoW(SPI_GETKEYBOARDSPEED, 0, &speed, 0))
            desc.input.keyRepeatRate = 1.0f / (2.5f + (float)std::min<DWORD>(speed, 31) * (27.5f / 31.0f));
    }

    bool EnableDpiAwareness()
    {
        if (::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
            return true;
        // set before (manifest, an earlier call): fine as long as it is per-monitor
        return ::GetAwarenessFromDpiAwarenessContext(::GetThreadDpiAwarenessContext()) == DPI_AWARENESS_PER_MONITOR_AWARE;
    }

    void* CreateAppWindow(Platform& platform, const WindowDesc& desc, std::string* error)
    {
        const auto fail = [&](const char* what) -> void* {
            if (error)
                *error = std::string(what) + " failed (error " + std::to_string(::GetLastError()) + ")";
            return nullptr;
        };
        if (platform.Hwnd())
        {
            if (error)
                *error = "CreateAppWindow: the platform serves a window already";
            return nullptr;
        }
        EnableDpiAwareness();

        HINSTANCE instance = ::GetModuleHandleW(nullptr);
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(wc);
        // CS_OWNDC: an OpenGL host keeps one device context (and its pixel format) for the window's life
        wc.style = CS_OWNDC;
        wc.lpfnWndProc = AppWindowProc;
        wc.hInstance = instance;
        wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = kWindowClass;
        if (!::RegisterClassExW(&wc) && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
            return fail("RegisterClassExW");

        const DWORD style = desc.resizable ? WS_OVERLAPPEDWINDOW : (WS_OVERLAPPEDWINDOW & ~(DWORD)(WS_THICKFRAME | WS_MAXIMIZEBOX));
        const auto outerSize = [&](UINT dpi) {
            RECT r = {0, 0, ::MulDiv(desc.width, (int)dpi, USER_DEFAULT_SCREEN_DPI), ::MulDiv(desc.height, (int)dpi, USER_DEFAULT_SCREEN_DPI)};
            ::AdjustWindowRectExForDpi(&r, style, FALSE, 0, dpi);
            return SIZE{r.right - r.left, r.bottom - r.top};
        };
        // sized for the primary monitor first, corrected below when Windows places it on another one
        const UINT systemDpi = ::GetDpiForSystem();
        SIZE size = outerSize(systemDpi);
        HWND w = ::CreateWindowExW(0, kWindowClass, Widen(desc.title).c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT, size.cx, size.cy,
                                   nullptr, nullptr, instance, &platform);
        if (!w)
            return fail("CreateWindowExW");
        const UINT dpi = ::GetDpiForWindow(w);
        if (dpi != 0 && dpi != systemDpi)
        {
            size = outerSize(dpi);
            ::SetWindowPos(w, nullptr, 0, 0, size.cx, size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        if (desc.darkTitleBar)
        {
            const BOOL dark = TRUE;
            ::DwmSetWindowAttribute(w, kDwmUseImmersiveDarkMode, &dark, sizeof(dark));
        }
        platform.impl_->owned = true;
        platform.Attach(w);
        ::ShowWindow(w, SW_SHOWDEFAULT);
        return w;
    }

    void DestroyAppWindow(Platform& platform)
    {
        if (HWND w = static_cast<HWND>(platform.Hwnd()))
            ::PostMessageW(w, platform.impl_->destroyMessage, 0, 0);
    }

    int RunMessageLoop()
    {
        MSG msg;
        while (::GetMessageW(&msg, nullptr, 0, 0) > 0)
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
        return (int)msg.wParam;
    }
}
