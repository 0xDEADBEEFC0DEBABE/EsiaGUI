// glass_window - OpenGL host through EGL: on Linux an OpenGL 3.3 core context on the X11 window of app_linux.cpp, on
// Android an OpenGL ES 3.0 context on the activity's window (app_android.cpp), current on the frame's thread, with the
// default framebuffer as the target (host_opengl.cpp does the same with WGL on Windows). The config is the one whose
// native visual is the window's (X11), or whose format the window takes (Android). On Android the window can go away
// and come back: the context, and so the device and all it holds, stays; only the surface is made again.
#include "host.hpp"
#include "esia/rhi/opengl.hpp"
#include <EGL/egl.h>
#if defined(__ANDROID__)
#include <android/native_window.h>
#else
#include <X11/Xlib.h>
#endif
#include <algorithm>
#include <vector>

namespace glass
{
    namespace
    {
        constexpr EGLint kContextFlags = 0x30FC, kDebugBit = 0x0001;   // EGL_CONTEXT_FLAGS_KHR, EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR
        constexpr unsigned kGlRenderer = 0x1F01;   // GL_RENDERER
#if defined(__ANDROID__)
        constexpr bool kEs = true;
#else
        constexpr EGLenum kPlatformX11 = 0x31D5;   // EGL_PLATFORM_X11_KHR (EGL_KHR_platform_x11, part of EGL 1.5's set)
        constexpr bool kEs = false;
#endif

        void* GetProc(const char* name) { return reinterpret_cast<void*>(::eglGetProcAddress(name)); }

        class HostOpenGL final : public Host
        {
        public:
            ~HostOpenGL() override
            {
                esia_.reset();   // while its context is current
                if (display_ != EGL_NO_DISPLAY)
                {
                    ::eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                    if (context_ != EGL_NO_CONTEXT)
                        ::eglDestroyContext(display_, context_);
                    if (surface_ != EGL_NO_SURFACE)
                        ::eglDestroySurface(display_, surface_);
                    ::eglTerminate(display_);
                }
            }

            const char* Name() const override { return kEs ? "OpenGL ES" : "OpenGL"; }

            bool Init(const NativeWindow& window, int width, int height, const HostOptions& o, std::string& error) override
            {
                vsync_ = o.vsync;
#if defined(__ANDROID__)
                display_ = ::eglGetDisplay(EGL_DEFAULT_DISPLAY);
#else
                Display* x = static_cast<Display*>(window.display);
                display_ = ::eglGetPlatformDisplay(kPlatformX11, x, nullptr);   // EGL 1.5
                if (display_ == EGL_NO_DISPLAY)
                    display_ = ::eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(x));
#endif
                EGLint major = 0, minor = 0;
                if (display_ == EGL_NO_DISPLAY || !::eglInitialize(display_, &major, &minor))
                {
                    display_ = EGL_NO_DISPLAY;
                    error = "no EGL display";
                    return false;
                }
                if (!::eglBindAPI(kEs ? EGL_OPENGL_ES_API : EGL_OPENGL_API))
                {
                    error = kEs ? "EGL has no OpenGL ES" : "EGL has no desktop OpenGL";
                    return false;
                }
                const EGLint want[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RENDERABLE_TYPE, kEs ? 0x0040 /* EGL_OPENGL_ES3_BIT */ : EGL_OPENGL_BIT,
                                       EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_NONE};
                EGLint n = 0;
                ::eglChooseConfig(display_, want, nullptr, 0, &n);
                std::vector<EGLConfig> configs((std::size_t)std::max(n, 0));
                ::eglChooseConfig(display_, want, configs.data(), n, &n);
#if !defined(__ANDROID__)
                XWindowAttributes wa = {};
                ::XGetWindowAttributes(x, static_cast<::Window>(window.window), &wa);
                const VisualID visual = ::XVisualIDFromVisual(wa.visual);
#endif
                for (EGLint i = 0; i < n && !config_; ++i)
                {
                    EGLint id = 0, r = 0, g = 0, b = 0;
                    ::eglGetConfigAttrib(display_, configs[(std::size_t)i], EGL_NATIVE_VISUAL_ID, &id);
                    ::eglGetConfigAttrib(display_, configs[(std::size_t)i], EGL_RED_SIZE, &r);
                    ::eglGetConfigAttrib(display_, configs[(std::size_t)i], EGL_GREEN_SIZE, &g);
                    ::eglGetConfigAttrib(display_, configs[(std::size_t)i], EGL_BLUE_SIZE, &b);
#if defined(__ANDROID__)
                    const bool fits = true;   // the window takes the config's format (ANativeWindow_setBuffersGeometry)
#else
                    const bool fits = (VisualID)id == visual;
#endif
                    if (fits && r == 8 && g == 8 && b == 8)
                        config_ = configs[(std::size_t)i];
                }
                if (!config_)
                {
                    error = "no EGL config for the window (RGB8)";
                    return false;
                }
                // EGL_KHR_create_context's names (EGL 1.4 displays have them too); the debug flag only with --debug
                const EGLint attribs[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, kEs ? 0 : 3,
                                          kEs ? EGL_NONE : EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
                                          o.debug ? kContextFlags : EGL_NONE, kDebugBit, EGL_NONE};
                const EGLint esAttribs[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 0, o.debug ? kContextFlags : EGL_NONE, kDebugBit,
                                            EGL_NONE};
                context_ = ::eglCreateContext(display_, config_, EGL_NO_CONTEXT, kEs ? esAttribs : attribs);
                if (context_ == EGL_NO_CONTEXT || !MakeSurface(window, error))
                {
                    if (error.empty())
                        error = std::string("no ") + (kEs ? "OpenGL ES 3.0" : "OpenGL 3.3 core") + " context on the window (EGL error " +
                                std::to_string(::eglGetError()) + ")";
                    return false;
                }
                using GetString = const unsigned char* (*)(unsigned);
                if (const auto getString = reinterpret_cast<GetString>(GetProc("glGetString")))
                    if (const unsigned char* renderer = getString(kGlRenderer))
                        adapter_ = reinterpret_cast<const char*>(renderer);

                esia::rhi::opengl::Desc d;
                d.es = kEs;
                d.debug = o.debug;   // KHR_debug messages to stderr, counted
                d.getProcAddress = &GetProc;
                esia_ = esia::rhi::opengl::CreateDevice(d, &error);
                if (!esia_)
                    return false;
                Resize(width, height);
                return (bool)target_;
            }

            esia::rhi::Device& Device() override { return *esia_; }

            // The window surface follows the window: only the wrapper learns the new size.
            void Resize(int width, int height) override
            {
                target_ = esia::rhi::opengl::WrapFramebuffer(*esia_, 0, width, height, esia::rhi::Format::RGBA8_UNORM);
            }

            // The context stays current without a surface (EGL_KHR_surfaceless_context, which Android has).
            void ReleaseWindow() override
            {
                if (surface_ == EGL_NO_SURFACE)
                    return;
                ::eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, context_);
                ::eglDestroySurface(display_, surface_);
                surface_ = EGL_NO_SURFACE;
                target_ = {};
            }

            bool AttachWindow(const NativeWindow& window, int width, int height, std::string& error) override
            {
                ReleaseWindow();
                if (!MakeSurface(window, error))
                    return false;
                Resize(width, height);
                return true;
            }

            bool BeginFrame() override { return (bool)target_ && surface_ != EGL_NO_SURFACE; }

        protected:
            void Present() override
            {
                if (surface_ != EGL_NO_SURFACE)
                    ::eglSwapBuffers(display_, surface_);
            }

        private:
            bool MakeSurface(const NativeWindow& window, std::string& error)
            {
#if defined(__ANDROID__)
                EGLint format = 0;
                ::eglGetConfigAttrib(display_, config_, EGL_NATIVE_VISUAL_ID, &format);
                ::ANativeWindow_setBuffersGeometry(static_cast<ANativeWindow*>(window.window), 0, 0, format);
                surface_ = ::eglCreateWindowSurface(display_, config_, static_cast<EGLNativeWindowType>(window.window), nullptr);
#else
                surface_ = ::eglCreateWindowSurface(display_, config_, static_cast<EGLNativeWindowType>(window.window), nullptr);
#endif
                if (surface_ == EGL_NO_SURFACE || !::eglMakeCurrent(display_, surface_, surface_, context_))
                {
                    error = "no EGL window surface (EGL error " + std::to_string(::eglGetError()) + ")";
                    return false;
                }
                ::eglSwapInterval(display_, vsync_ ? 1 : 0);
                return true;
            }

            EGLDisplay display_ = EGL_NO_DISPLAY;
            EGLConfig config_ = nullptr;
            EGLSurface surface_ = EGL_NO_SURFACE;
            EGLContext context_ = EGL_NO_CONTEXT;
            std::unique_ptr<esia::rhi::Device> esia_;
        };
    }

    std::unique_ptr<Host> CreateHostOpenGL() { return std::make_unique<HostOpenGL>(); }
}
