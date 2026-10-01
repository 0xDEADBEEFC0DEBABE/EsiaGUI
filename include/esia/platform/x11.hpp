// Esia - the X11 platform layer (target esia_platform_x11, CMake option ESIA_PLATFORM_X11): Linux and other X11
// systems, and Wayland desktops through XWayland.
//
// Turns a window's events into the core's input (Context::QueueInput) and applies the core's PlatformRequests to the
// window (the cursor). Text comes through the input method (XIM: the text it commits; the composition is not shown in
// the field), the clipboard is the CLIPBOARD selection (UTF8_STRING), key repeat follows the X server's, and the UI
// scale is Xft.dpi / 96 (what the desktop's scaling sets) times Desc::uiScale. It works with a window it creates
// (CreateAppWindow) or with the host's own (Attach, then HandleEvent for each of its events).
//
//   esia::platform::x11::Platform platform;
//   platform.CreateAppWindow({.title = "Esia"});
//   esia::ContextDesc cd;
//   platform.Configure(cd);                        // clipboard, key repeat
//   esia::Context ctx(cd);
//   platform.SetContext(&ctx);                     // input starts flowing into ctx
//   ... a device on the window: EGL with platform.Display() / Window(), or VK_KHR_xlib_surface ...
//   while (platform.Pump())                        // the window's events; false once it was closed
//   {
//       const esia::platform::x11::FrameInfo fi = platform.Frame();
//       ctx.NewFrame(fi.Params(time));
//       ... widgets, ctx.EndFrame(), render ...
//       platform.ApplyRequests(ctx.Requests());    // the cursor
//   }
//   platform.SetContext(nullptr);                  // before ctx goes away
//
// Threading: Xlib is used from one thread, the UI thread (the Context's): events and frames alternate on it.
#pragma once
#include "esia/core/context.hpp"
#include <memory>
#include <string>

namespace esia::platform::x11
{
    struct Desc
    {
        float uiScale = 1.0f;   // multiplies the desktop's scale: pixels per UI unit = Xft.dpi / 96 * uiScale
    };

    struct FrameInfo
    {
        int width = 0, height = 0;   // the window, pixels
        float scale = 1.0f;          // pixels per UI unit
        bool minimized = false;      // unmapped (minimized, or on another workspace)
        bool focused = false;
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
        std::string title = "Esia";        // UTF-8
        float width = 1280, height = 800;  // UI units (times the scale: pixels)
        float scale = 0.0f;                // pixels per UI unit; 0 = the desktop's (times Desc::uiScale)
        std::string appClass = "Esia";     // WM_CLASS: how the desktop groups and names the window
    };

    class ESIA_API Platform
    {
    public:
        explicit Platform(const Desc& desc = {});
        ~Platform();
        Platform(const Platform&) = delete;
        Platform& operator=(const Platform&) = delete;

        // Opens the display and a window on it, shown before it returns (at the size the window manager gave it).
        bool CreateAppWindow(const WindowDesc& desc, std::string* error = nullptr);
        // Serves the host's window (`display` a Display*, `window` a Window): adds the event mask it needs and opens the
        // input method on it. The host then passes every event of the window to HandleEvent.
        bool Attach(void* display, unsigned long window, std::string* error = nullptr);
        void Detach();

        void* Display() const;          // Display*
        unsigned long Window() const;   // Window

        // CreateAppWindow's windows: handles every event waiting (`wait`: blocks for the next one first, for an idle
        // or minimized window); false once the window was asked to close.
        bool Pump(bool wait = false);
        // An event of the window (an XEvent*): true when it was the input method's and the host must skip it.
        bool HandleEvent(void* xevent);

        void SetContext(Context* ctx);
        // Before creating the context: its clipboard callbacks (the Platform must outlive the context) and the server's
        // key repeat.
        void Configure(ContextDesc& desc);
        FrameInfo Frame() const;
        void ApplyRequests(const PlatformRequests& requests);
        void SetUiScale(float scale);

        bool CloseRequested() const;
        bool WantCaptureMouse() const;
        bool WantCaptureKeyboard() const;

        struct Impl;

    private:
        std::unique_ptr<Impl> impl_;
    };
}
