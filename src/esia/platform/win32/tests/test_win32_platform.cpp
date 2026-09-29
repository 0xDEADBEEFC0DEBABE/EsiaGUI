// Esia - Win32 platform layer tests: a hidden window receives the messages Windows would send (SendMessage on the
// test's thread, which is both the window thread and the UI thread here), and the core's InputState shows what the
// platform queued. Needs a desktop session (under Wine: an X server, e.g. xvfb-run).
#include "esia/platform/win32.hpp"
#include "esia_test.hpp"
#include <windows.h>
#include <imm.h>
#include <atomic>
#include <thread>

namespace
{
    using namespace esia;
    namespace pw = esia::platform::win32;

    pw::Platform* g_platform = nullptr;   // the window procedure of the test windows forwards to it

    LRESULT CALLBACK TestProc(HWND w, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        std::intptr_t result = 0;
        if (g_platform && g_platform->HandleMessage(w, msg, wparam, lparam, &result))
            return (LRESULT)result;
        return ::DefWindowProcW(w, msg, wparam, lparam);
    }

    ContextDesc WithClipboard(pw::Platform& p)
    {
        ContextDesc d;
        p.InstallClipboard(d);
        return d;
    }

    LPARAM KeyLParam(UINT scanCode, bool up) { return (LPARAM)(1u | (scanCode << 16) | (up ? (3u << 30) : 0u)); }

    void Pump()
    {
        MSG msg;
        while (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
    }

    // A game-style host window: created by the test, attached, messages forwarded by TestProc.
    struct Fixture
    {
        pw::Platform platform;
        Context ctx;
        HWND hwnd = nullptr;
        double time = 0.0;
        bool inFrame = false;

        explicit Fixture(const pw::Desc& desc = {}) : platform(desc), ctx(WithClipboard(platform))
        {
            static const bool registered = [] {
                WNDCLASSEXW wc = {};
                wc.cbSize = sizeof(wc);
                wc.lpfnWndProc = TestProc;
                wc.hInstance = ::GetModuleHandleW(nullptr);
                wc.lpszClassName = L"EsiaPlatformTest";
                return ::RegisterClassExW(&wc) != 0;
            }();
            ESIA_CHECK(registered);
            hwnd = ::CreateWindowExW(0, L"EsiaPlatformTest", L"esia test", WS_OVERLAPPEDWINDOW, 0, 0, 400, 300, nullptr, nullptr,
                                     ::GetModuleHandleW(nullptr), nullptr);
            ESIA_CHECK(hwnd != nullptr);
            g_platform = &platform;
            ESIA_CHECK(platform.Attach(hwnd));
            platform.SetContext(&ctx);
        }

        ~Fixture()
        {
            if (inFrame)
                ctx.EndFrame();
            platform.SetContext(nullptr);
            ::DestroyWindow(hwnd);   // WM_NCDESTROY detaches
            g_platform = nullptr;
            Pump();
        }

        LRESULT Send(UINT msg, WPARAM wparam = 0, LPARAM lparam = 0) { return ::SendMessageW(hwnd, msg, wparam, lparam); }

        // The next frame's input (applies what the platform queued since the last one).
        const InputState& Frame()
        {
            if (inFrame)
                ctx.EndFrame();
            time += 1.0 / 60.0;
            ctx.NewFrame(platform.Frame().Params(time));
            inFrame = true;
            return ctx.Input();
        }

        float Scale() const { return platform.Frame().scale; }
    };
}

ESIA_TEST(Win32Platform, MouseIsInUiUnits)
{
    Fixture f;
    f.platform.SetUiScale(2.0f);
    const float s = f.Scale();
    ESIA_CHECK_NEAR(s, 2.0f * f.platform.Frame().dpiScale, 1e-6f);
    f.Send(WM_MOUSEMOVE, 0, MAKELPARAM(100, 60));
    const InputState& in = f.Frame();
    ESIA_CHECK_NEAR(in.MousePos().x, 100.0f / s, 1e-4f);
    ESIA_CHECK_NEAR(in.MousePos().y, 60.0f / s, 1e-4f);
    f.Send(WM_MOUSELEAVE);
    ESIA_CHECK(!f.Frame().MouseValid());
}

ESIA_TEST(Win32Platform, ButtonsCaptureTheMouse)
{
    Fixture f;
    f.Send(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(10, 20));
    ESIA_CHECK(::GetCapture() == f.hwnd);
    const InputState& in = f.Frame();
    ESIA_CHECK(in.MouseClicked(MouseButton::Left));
    // the click lands where the button went down, even without a WM_MOUSEMOVE before it
    ESIA_CHECK_NEAR(in.MouseClickedPos(MouseButton::Left).x, 10.0f / f.Scale(), 1e-4f);
    f.Send(WM_RBUTTONDOWN, MK_RBUTTON | MK_LBUTTON, MAKELPARAM(10, 20));
    f.Send(WM_LBUTTONUP, MK_RBUTTON, MAKELPARAM(10, 20));
    ESIA_CHECK(::GetCapture() == f.hwnd);   // the right button is still down
    f.Send(WM_RBUTTONUP, 0, MAKELPARAM(10, 20));
    ESIA_CHECK(::GetCapture() == nullptr);
    f.Frame();
    const InputState& in2 = f.Frame();
    ESIA_CHECK(!in2.MouseDown(MouseButton::Left) && !in2.MouseDown(MouseButton::Right));
    // a release without a press in this window is not the UI's
    f.Send(WM_MBUTTONUP, 0, MAKELPARAM(10, 20));
    ESIA_CHECK(!f.Frame().MouseReleased(MouseButton::Middle));
}

ESIA_TEST(Win32Platform, LosingTheCaptureReleasesTheButtons)
{
    Fixture f;
    f.Send(WM_XBUTTONDOWN, MAKEWPARAM(MK_XBUTTON1, XBUTTON1), MAKELPARAM(5, 5));
    ESIA_CHECK(f.Frame().MouseDown(MouseButton::X1));
    f.Send(WM_CAPTURECHANGED, 0, 0);   // another window took it
    ESIA_CHECK(!f.Frame().MouseDown(MouseButton::X1));
}

ESIA_TEST(Win32Platform, WheelNotches)
{
    Fixture f;
    f.Send(WM_MOUSEWHEEL, MAKEWPARAM(0, 2 * WHEEL_DELTA), 0);
    f.Send(WM_MOUSEHWHEEL, MAKEWPARAM(0, WHEEL_DELTA / 2), 0);
    const Vec2 wheel = f.Frame().Wheel();
    ESIA_CHECK_NEAR(wheel.y, 2.0f, 1e-6f);
    ESIA_CHECK_NEAR(wheel.x, -0.5f, 1e-6f);   // tilted right = scroll toward the right = x < 0
}

ESIA_TEST(Win32Platform, KeysAndModifiers)
{
    Fixture f;
    BYTE state[256] = {};
    state[VK_CONTROL] = state[VK_LCONTROL] = 0x80;
    ::SetKeyboardState(state);
    f.Send(WM_KEYDOWN, VK_CONTROL, KeyLParam(0x1D, false));
    f.Send(WM_KEYDOWN, 'C', KeyLParam(0x2E, false));
    f.Send(WM_KEYDOWN, 'C', KeyLParam(0x2E, false) | (1 << 30));   // auto-repeat: the core repeats itself
    const InputState& in = f.Frame();
    ESIA_CHECK(in.KeyDown(Key::LeftCtrl) && in.KeyPressed(Key::C, false));
    ESIA_CHECK((in.Mods() & Mod_Ctrl) != 0);
    state[VK_CONTROL] = state[VK_LCONTROL] = 0;
    ::SetKeyboardState(state);
    f.Send(WM_KEYUP, 'C', KeyLParam(0x2E, true));
    f.Send(WM_KEYUP, VK_CONTROL, KeyLParam(0x1D, true));
    f.Frame();
    const InputState& in2 = f.Frame();
    ESIA_CHECK(!in2.KeyDown(Key::C) && !in2.KeyDown(Key::LeftCtrl) && in2.Mods() == 0);
    // the right Ctrl is the extended one
    f.Send(WM_KEYDOWN, VK_CONTROL, KeyLParam(0x1D, false) | (1 << 24));
    ESIA_CHECK(f.Frame().KeyDown(Key::RightCtrl));
}

ESIA_TEST(Win32Platform, KeysTheImeTakesAreNotTheUis)
{
    Fixture f;
    f.Send(WM_KEYDOWN, VK_PROCESSKEY, KeyLParam(0x30, false));
    f.Send(WM_KEYUP, 'B', KeyLParam(0x30, true));   // the IME's key comes back up with its real code
    const InputState& in = f.Frame();
    ESIA_CHECK(!in.KeyPressed(Key::B, false) && !in.KeyReleased(Key::B));
}

ESIA_TEST(Win32Platform, BothShiftKeysRelease)
{
    Fixture f;
    BYTE state[256] = {};
    state[VK_SHIFT] = state[VK_LSHIFT] = state[VK_RSHIFT] = 0x80;
    ::SetKeyboardState(state);
    f.Send(WM_KEYDOWN, VK_SHIFT, KeyLParam(0x2A, false));   // left shift
    f.Send(WM_KEYDOWN, VK_SHIFT, KeyLParam(0x36, false));   // right shift
    f.Frame();
    const InputState& in = f.Frame();
    ESIA_CHECK(in.KeyDown(Key::LeftShift) && in.KeyDown(Key::RightShift));
    state[VK_SHIFT] = state[VK_LSHIFT] = state[VK_RSHIFT] = 0;
    ::SetKeyboardState(state);
    f.Send(WM_KEYUP, VK_SHIFT, KeyLParam(0x2A, true));      // Windows sends one key-up for both
    f.Frame();
    const InputState& in2 = f.Frame();
    ESIA_CHECK(!in2.KeyDown(Key::LeftShift) && !in2.KeyDown(Key::RightShift));
}

ESIA_TEST(Win32Platform, TextWithSurrogatePairs)
{
    Fixture f;
    for (wchar_t c : {L'h', (wchar_t)0x4F60, (wchar_t)0xD83D, (wchar_t)0xDE00, (wchar_t)0x08, (wchar_t)0x7F, (wchar_t)0x597D})
        f.Send(WM_CHAR, c, 1);
    f.Send(WM_CHAR, 0xDE00, 1);   // a lone low half is dropped
    ESIA_CHECK(f.Frame().Text() == U"h你\U0001F600好");
}

ESIA_TEST(Win32Platform, FocusLossWhilePressing)
{
    Fixture f;
    f.Send(WM_SETFOCUS);
    f.Send(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(30, 30));
    f.Send(WM_KEYDOWN, 'A', KeyLParam(0x1E, false));
    const InputState& in = f.Frame();
    ESIA_CHECK(in.MouseDown(MouseButton::Left) && in.KeyDown(Key::A) && in.Focused());
    f.Send(WM_KILLFOCUS);
    ESIA_CHECK(::GetCapture() == nullptr);
    const InputState& in2 = f.Frame();
    ESIA_CHECK(!in2.MouseDown(MouseButton::Left) && !in2.KeyDown(Key::A) && !in2.Focused());
    ESIA_CHECK(!f.platform.Frame().focused);
    // the key's release after the focus came back is not a release for the UI (it was already released)
    f.Send(WM_SETFOCUS);
    f.Send(WM_KEYUP, 'A', KeyLParam(0x1E, true));
    const InputState& in3 = f.Frame();
    ESIA_CHECK(in3.Focused() && !in3.KeyReleased(Key::A));
}

ESIA_TEST(Win32Platform, SizeAndDpi)
{
    Fixture f;
    f.Send(WM_SIZE, SIZE_RESTORED, MAKELPARAM(640, 480));
    pw::FrameInfo fi = f.platform.Frame();
    ESIA_CHECK(fi.width == 640 && fi.height == 480 && !fi.minimized);
    f.Send(WM_SIZE, SIZE_MINIMIZED, 0);
    fi = f.platform.Frame();
    ESIA_CHECK(fi.width == 640 && fi.height == 480 && fi.minimized);
    RECT suggested = {0, 0, 900, 700};
    f.Send(WM_DPICHANGED, MAKEWPARAM(144, 144), (LPARAM)&suggested);
    fi = f.platform.Frame();
    ESIA_CHECK_NEAR(fi.dpiScale, 1.5f, 1e-6f);
    ESIA_CHECK_NEAR(fi.scale, 1.5f, 1e-6f);
    const FrameParams p = fi.Params(1.0);
    ESIA_CHECK_NEAR(p.displaySize.x, (float)fi.width / 1.5f, 1e-4f);
    ESIA_CHECK_NEAR(p.framebufferScale.y, 1.5f, 1e-6f);
    // a host window keeps its size: the host decides what to do with the suggested rect
    RECT wr = {};
    ::GetWindowRect(f.hwnd, &wr);
    ESIA_CHECK(wr.right - wr.left == 400);
    f.Send(WM_MOUSEMOVE, 0, MAKELPARAM(150, 30));
    ESIA_CHECK_NEAR(f.Frame().MousePos().x, 100.0f, 1e-4f);
}

ESIA_TEST(Win32Platform, RequestsReachTheWindow)
{
    Fixture f;
    PlatformRequests r;
    r.cursor = MouseCursor::Hand;
    r.wantCaptureMouse = true;
    r.wantTextInput = true;
    r.imeRect = Rect(Vec2(10, 20), Vec2(11, 40));
    f.platform.ApplyRequests(r);
    ESIA_CHECK(f.platform.WantCaptureMouse() && f.platform.WantTextInput() && !f.platform.WantCaptureKeyboard());
    Pump();
    const float s = f.Scale();
    POINT caret = {};
    ESIA_CHECK(::GetCaretPos(&caret));
    ESIA_CHECK(caret.x == (LONG)(10 * s) && caret.y == (LONG)(20 * s));
    HIMC himc = ::ImmGetContext(f.hwnd);
    ESIA_CHECK(himc != nullptr);   // text input: the window's input context is back
    if (himc)
        ::ImmReleaseContext(f.hwnd, himc);
    // WM_SETCURSOR over the client area sets the requested shape
    ESIA_CHECK(f.Send(WM_SETCURSOR, (WPARAM)f.hwnd, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE)) == TRUE);
    ESIA_CHECK(::GetCursor() == ::LoadCursorW(nullptr, IDC_HAND));
    // TSF IMEs ask where the caret is, in screen coordinates
    IMECHARPOSITION cp = {};
    cp.dwSize = sizeof(cp);
    ESIA_CHECK(f.Send(WM_IME_REQUEST, IMR_QUERYCHARPOSITION, (LPARAM)&cp) == TRUE);
    POINT expected = {(LONG)(10 * s), (LONG)(20 * s)};
    ::ClientToScreen(f.hwnd, &expected);
    ESIA_CHECK(cp.pt.x == expected.x && cp.pt.y == expected.y && cp.cLineHeight == (UINT)(20 * s));

    f.platform.ApplyRequests(PlatformRequests());
    Pump();
    ESIA_CHECK(!f.platform.WantTextInput());
    himc = ::ImmGetContext(f.hwnd);
    ESIA_CHECK(himc == nullptr);   // no text field: no input context, the IME is off for the window
    if (himc)
        ::ImmReleaseContext(f.hwnd, himc);
    ESIA_CHECK(f.Send(WM_IME_REQUEST, IMR_QUERYCHARPOSITION, (LPARAM)&cp) == 0);
}

ESIA_TEST(Win32Platform, ImeLeftToTheHost)
{
    pw::Desc d;
    d.imeOnlyForTextInput = false;
    Fixture f(d);
    HIMC himc = ::ImmGetContext(f.hwnd);
    ESIA_CHECK(himc != nullptr);   // a game using the IME itself keeps it while the UI takes no text
    if (himc)
        ::ImmReleaseContext(f.hwnd, himc);
}

ESIA_TEST(Win32Platform, Clipboard)
{
    Fixture f;
    const std::string text = "line 1\nline 2: \xE4\xBD\xA0\xE5\xA5\xBD";   // "你好"
    f.ctx.SetClipboardText(text);
    ESIA_CHECK(::OpenClipboard(f.hwnd));
    std::wstring raw;
    if (HANDLE h = ::GetClipboardData(CF_UNICODETEXT))
    {
        raw = static_cast<const wchar_t*>(::GlobalLock(h));
        ::GlobalUnlock(h);
    }
    ::CloseClipboard();
    ESIA_CHECK(raw == L"line 1\r\nline 2: 你好");
    ESIA_CHECK(f.ctx.GetClipboardText() == text);
}

ESIA_TEST(Win32Platform, NoContextNoInput)
{
    Fixture f;
    f.platform.SetContext(nullptr);
    f.Send(WM_MOUSEMOVE, 0, MAKELPARAM(40, 40));
    f.platform.SetContext(&f.ctx);
    ESIA_CHECK(!f.Frame().MouseValid());
}

ESIA_TEST(Win32Platform, AppWindowLifecycle)
{
    pw::Platform platform;
    std::string error;
    HWND w = static_cast<HWND>(pw::CreateAppWindow(platform, {.title = "esia \xE6\xB5\x8B\xE8\xAF\x95", .width = 320, .height = 200}, &error));
    ESIA_CHECK(w != nullptr && error.empty());
    ESIA_CHECK(platform.Hwnd() == w);
    ESIA_CHECK(pw::CreateAppWindow(platform, {}, &error) == nullptr && !error.empty());
    wchar_t title[32] = {};
    ::GetWindowTextW(w, title, 32);
    ESIA_CHECK(std::wstring(title) == L"esia 测试");
    // an app window takes the size Windows suggests for the new monitor
    RECT suggested = {100, 100, 700, 560};
    ::SendMessageW(w, WM_DPICHANGED, MAKEWPARAM(192, 192), (LPARAM)&suggested);
    RECT wr = {};
    ::GetWindowRect(w, &wr);
    ESIA_CHECK(wr.right - wr.left == 600 && wr.bottom - wr.top == 460);
    ESIA_CHECK_NEAR(platform.Frame().dpiScale, 2.0f, 1e-6f);
    // closing only asks: the render thread releases its device first
    ::PostMessageW(w, WM_CLOSE, 0, 0);
    Pump();
    ESIA_CHECK(platform.CloseRequested() && ::IsWindow(w));
    pw::DestroyAppWindow(platform);
    ESIA_CHECK(pw::RunMessageLoop() == 0);
    ESIA_CHECK(!::IsWindow(w) && platform.Hwnd() == nullptr);
}

// The layout the example uses: a window thread pumping messages, the UI on another thread.
ESIA_TEST(Win32Platform, WindowThreadAndUiThread)
{
    pw::Platform platform;
    Context ctx;
    platform.SetContext(&ctx);
    std::atomic<HWND> window{nullptr};
    std::thread windowThread([&] {
        window = static_cast<HWND>(pw::CreateAppWindow(platform, {.width = 300, .height = 200}));
        pw::RunMessageLoop();
    });
    while (!window.load())
        std::this_thread::yield();
    const float s = platform.Frame().scale;
    ::PostMessageW(window.load(), WM_MOUSEMOVE, 0, MAKELPARAM(60, 30));
    ::PostMessageW(window.load(), WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(60, 30));
    bool clicked = false;
    for (int frame = 0; frame < 200 && !clicked; ++frame)
    {
        ctx.NewFrame(platform.Frame().Params(frame / 60.0));
        clicked = ctx.Input().MouseDown(MouseButton::Left);
        if (clicked)
            ESIA_CHECK_NEAR(ctx.Input().MousePos().x, 60.0f / s, 1e-4f);
        PlatformRequests r;
        r.cursor = MouseCursor::ResizeEW;
        ctx.EndFrame();
        platform.ApplyRequests(r);
        ::Sleep(5);
    }
    ESIA_CHECK(clicked);
    platform.SetContext(nullptr);   // no more queueing into ctx once this returns
    pw::DestroyAppWindow(platform);
    windowThread.join();
    ESIA_CHECK(platform.Hwnd() == nullptr);
}
