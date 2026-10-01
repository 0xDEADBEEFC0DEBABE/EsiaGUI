// Esia - the Android platform layer (target esia_platform_android, built for Android).
//
// Turns an activity's input into the core's input (Context::QueueInput) and applies the core's PlatformRequests:
//   * touch is the mouse: the first finger is the left button, marked as a finger's (InputEvent::touch). A tap is a
//     click; a drag along a scroll area scrolls it, also from a row or a button (which then does not press); two
//     fingers scroll anywhere, a dp of movement a tenth of a notch. A mouse (ChromeOS, the emulator
//     with a mouse) moves, clicks and scrolls as on a desktop;
//   * keys by key code, with the text they type (KeyEvent.getUnicodeChar) - hardware keyboards and what soft keyboards
//     send as key events; the soft keyboard shows while a text field has the keyboard;
//   * the clipboard (ClipboardManager), and the screen's density as the UI scale: UI units are dp.
// It works with a NativeActivity (android_native_app_glue): the app hands over the activity, then its window, its
// configuration changes and every input event, all on the app's thread (android_main's), which it attaches to the Java
// VM for the calls it makes into Java.
//
//   esia::platform::android::Platform platform(app->activity);
//   app->onInputEvent = ...: return platform.HandleInputEvent(event) ? 1 : 0;
//   APP_CMD_INIT_WINDOW: platform.SetWindow(app->window), then a surface on it (EGL, VK_KHR_android_surface)
//   APP_CMD_TERM_WINDOW: drop the surface, then platform.SetWindow(nullptr)
//   APP_CMD_GAINED_FOCUS / LOST_FOCUS: platform.SetFocused(...); APP_CMD_CONFIG_CHANGED: platform.ConfigurationChanged()
//   every frame: fi = platform.Frame(); ctx.NewFrame(fi.Params(time)); ...; platform.ApplyRequests(ctx.Requests());
#pragma once
#include "esia/core/context.hpp"
#include <memory>
#include <string>

namespace esia::platform::android
{
    struct FrameInfo
    {
        int width = 0, height = 0;   // the window, pixels
        float scale = 1.0f;          // pixels per UI unit: the density / 160 (UI units are dp) times the UI scale
        bool visible = false;        // there is a window
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

    class ESIA_API Platform
    {
    public:
        explicit Platform(void* activity);   // ANativeActivity*
        ~Platform();
        Platform(const Platform&) = delete;
        Platform& operator=(const Platform&) = delete;

        void SetWindow(void* window);   // ANativeWindow*; null when it goes away
        void ConfigurationChanged();    // the density or the orientation may have changed
        void SetFocused(bool focused);
        // An input event (AInputEvent*): true when the UI took it; false lets the system have it (the back key with no
        // text field focused: the activity goes back).
        bool HandleInputEvent(const void* event);

        void SetContext(Context* ctx);
        // Before creating the context: its clipboard callbacks (the Platform must outlive the context).
        void Configure(ContextDesc& desc);
        FrameInfo Frame() const;
        // The soft keyboard, shown while a text field wants text.
        void ApplyRequests(const PlatformRequests& requests);
        void SetUiScale(float scale);

        bool WantCaptureMouse() const;
        bool WantCaptureKeyboard() const;
        // The directory of the app's own files (screenshots, settings): ANativeActivity::internalDataPath.
        std::string FilesDirectory() const;

        struct Impl;

    private:
        std::unique_ptr<Impl> impl_;
    };
}
