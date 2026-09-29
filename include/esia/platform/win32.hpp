// Esia - the Win32 platform layer (target esia_platform_win32, CMake option ESIA_PLATFORM_WIN32).
//
// Turns a window's messages into the core's input (Context::QueueInput) and applies the core's PlatformRequests to
// the window: cursor shape, the IME (on only while a text field wants it, candidate window at the caret), the
// window's clipboard, and the capture flags a game needs to know which input is the UI's. It works with a window it
// creates (CreateAppWindow) or with the host's own window: a game hands over its HWND (Attach) and forwards every
// message of it (HandleMessage).
//
//   // window thread: creates the window and pumps its messages
//   esia::platform::win32::Platform platform;
//   esia::platform::win32::CreateAppWindow(platform, {.title = "Esia"});
//   std::thread ui(UiMain, std::ref(platform));
//   esia::platform::win32::RunMessageLoop();              // returns once the window is destroyed
//   ui.join();
//
//   // render thread (the UI thread of the Context): owns the device and the frame loop
//   esia::ContextDesc cd;
//   platform.InstallClipboard(cd);
//   esia::Context ctx(cd);
//   platform.SetContext(&ctx);                            // input starts flowing into ctx
//   while (!platform.CloseRequested())
//   {
//       const FrameInfo fi = platform.Frame();            // size in pixels, DPI scale
//       ctx.NewFrame(fi.Params(time));                    // UI units = pixels / fi.scale
//       ... widgets ...
//       ctx.EndFrame();
//       renderer.Render(ctx.GetDrawData(), &ctx.Textures(), target);
//       platform.ApplyRequests(ctx.Requests());          // cursor, IME, capture flags
//   }
//   platform.SetContext(nullptr);                         // before ctx goes away
//   ... release the swap chain / surface ...
//   esia::platform::win32::DestroyAppWindow(platform);    // ends RunMessageLoop
//
// A game's own window instead: Attach(hwnd) on the window thread, then first thing in its window procedure
//
//   std::intptr_t r;
//   if (platform.HandleMessage(hwnd, msg, wParam, lParam, &r))
//       return r;                                         // the platform answered it (IME, cursor ...)
//   if (IsMouseMessage(msg) && platform.WantCaptureMouse()) ...   // the UI has the mouse: the game ignores it
//
// Threading rules
//   * Window thread = the thread that created the HWND. Attach, Detach, HandleMessage, CreateAppWindow and
//     RunMessageLoop run there, and everything that must touch the window happens there: capture, mouse-leave
//     tracking, cursor, IME contexts, the system caret, SetWindowPos on DPI changes. It never waits for the render
//     thread, so moving or resizing the window (a modal loop inside DefWindowProc) never stalls rendering.
//   * UI thread = the thread of the Context (NewFrame .. EndFrame, the renderer): SetContext, Frame, ApplyRequests
//     and the clipboard callbacks run there. It never sends messages to the window (which could wait on a window
//     thread busy in a modal loop): requests reach the window thread as one posted message.
//   * Input crosses over through Context::QueueInput, which is thread-safe; mouse positions are converted to UI
//     units on the window thread with the scale of the moment. SetContext(nullptr) returns once no message is being
//     queued into the old context, so the context can be destroyed right after it.
//   * WantCaptureMouse / WantCaptureKeyboard / WantTextInput / CloseRequested / Frame may be read on any thread.
//     The capture flags are those of the last frame that called ApplyRequests (one frame late, as in every
//     immediate-mode UI).
//   * One Platform per window. Destroy it after the window (or after Detach) and after SetContext(nullptr).
//
// Requires Windows 10 1703 or later (per-monitor v2 DPI awareness).
#pragma once
#include "esia/core/context.hpp"
#include <cstdint>
#include <memory>
#include <string>

namespace esia::platform::win32
{
    enum class CursorMode : std::uint8_t
    {
        Always,          // the platform sets the cursor over the whole client area (an app's own window)
        WhenCapturing,   // only while the UI wants the mouse (an overlay in a game that sets its own cursor)
        Never,
    };

    struct Desc
    {
        // The UI draws the IME composition string itself (InputState::Composition at the caret), so the IME shows
        // only its candidate window. False: the IME draws its own composition window at the caret (a UI that cannot
        // draw it); the committed text then arrives as WM_CHAR.
        bool inlineComposition = true;
        // The IME is enabled on the window only while PlatformRequests::wantTextInput, so typing into a game or a
        // button never opens a candidate window. A game that uses the IME itself when the UI does not want text
        // turns this off: the platform then never enables or disables the window's input context.
        bool imeOnlyForTextInput = true;
        CursorMode cursor = CursorMode::Always;
        float uiScale = 1.0f;   // multiplies the monitor's DPI scale (a user zoom): pixels per UI unit = DPI / 96 * uiScale
    };

    // The window as the UI thread uses it for a frame (Platform::Frame).
    struct FrameInfo
    {
        int width = 0, height = 0;   // client area, pixels
        float dpiScale = 1.0f;       // the window's monitor DPI / 96
        float scale = 1.0f;          // pixels per UI unit: dpiScale * Desc::uiScale
        bool minimized = false;      // width and height keep the last restored size
        bool focused = false;

        // displaySize = pixels / scale (UI units), framebufferScale = scale: the UI renders at the native resolution.
        FrameParams Params(double time) const
        {
            FrameParams p;
            p.displaySize = Vec2((float)width / scale, (float)height / scale);
            p.framebufferScale = Vec2(scale, scale);
            p.time = time;
            return p;
        }
    };

    struct WindowDesc
    {
        std::string title = "Esia";   // UTF-8
        int width = 1280, height = 800;   // client area in UI units (pixels at 100 % scale, times the monitor's scale)
        bool resizable = true;
        bool darkTitleBar = true;
    };

    class ESIA_API Platform
    {
    public:
        explicit Platform(const Desc& desc = {});
        ~Platform();
        Platform(const Platform&) = delete;
        Platform& operator=(const Platform&) = delete;

        // ---- window thread
        // Starts serving `hwnd` (an HWND): reads its size, DPI and focus, and turns the IME off until a text field
        // wants it (Desc::imeOnlyForTextInput). False if a window is attached already.
        bool Attach(void* hwnd);
        // Stops serving the window: releases the mouse capture and the system caret, gives the IME back to it.
        // Destroying the window detaches too (WM_NCDESTROY).
        void Detach();
        // Call first in the window procedure, with every message of the window. Input messages are queued into
        // the context and return false: the host goes on with them (a game checks WantCaptureMouse / Keyboard).
        // True means the platform answered the message: return *result without calling DefWindowProc. That is the
        // IME's messages (the composition is the UI's), WM_SETCURSOR over the client area, and for a window made
        // by CreateAppWindow also WM_CLOSE, WM_DPICHANGED and the Alt key menu.
        bool HandleMessage(void* hwnd, std::uint32_t msg, std::uintptr_t wparam, std::intptr_t lparam, std::intptr_t* result);

        // ---- UI thread
        // The context that receives the input (null: input is dropped). Returns once no window-thread call is
        // queueing into the previous one.
        void SetContext(Context* ctx);
        FrameInfo Frame() const;
        // After Context::EndFrame: publishes the capture flags and, when the cursor or the text input changed, posts
        // one message to the window thread that applies them.
        void ApplyRequests(const PlatformRequests& requests);
        // Points the context's clipboard callbacks at the window's clipboard (UTF-8 <-> UTF-16, "\n" <-> "\r\n").
        // The Platform must outlive the context.
        void InstallClipboard(ContextDesc& desc);
        void SetUiScale(float scale);

        // ---- any thread
        bool WantCaptureMouse() const;
        bool WantCaptureKeyboard() const;
        bool WantTextInput() const;
        // A window made by CreateAppWindow got WM_CLOSE: the render thread finishes, then calls DestroyAppWindow.
        bool CloseRequested() const;
        void* Hwnd() const;

    private:
        friend void* CreateAppWindow(Platform&, const WindowDesc&, std::string*);
        friend void DestroyAppWindow(Platform&);
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

    // Window thread. Makes the process per-monitor-v2 DPI aware (EnableDpiAwareness), creates a top-level window
    // whose window procedure is `platform`'s (HandleMessage, then DefWindowProc), attaches it and shows it. Returns
    // the HWND, or null with the reason in `error`.
    ESIA_API void* CreateAppWindow(Platform& platform, const WindowDesc& desc, std::string* error = nullptr);
    // Any thread: destroys the window made by CreateAppWindow (posted to the window thread), which ends
    // RunMessageLoop. Call it after releasing everything that renders into the window.
    ESIA_API void DestroyAppWindow(Platform& platform);
    // Window thread: pumps messages (TranslateMessage feeds the IME and WM_CHAR) until WM_QUIT; returns its code.
    ESIA_API int RunMessageLoop();
    // Per-monitor v2 DPI awareness for the process, unless something set its awareness already (a manifest, an earlier
    // call). True when the process ends up per-monitor aware (v1 or v2).
    ESIA_API bool EnableDpiAwareness();
}
