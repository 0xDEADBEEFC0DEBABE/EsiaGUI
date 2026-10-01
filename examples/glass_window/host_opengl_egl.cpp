// glass_window - OpenGL host on Linux: an EGL window surface on the X11 window of app_linux.cpp and an OpenGL 3.3 core
// context, current on the frame's thread, with the default framebuffer as the target (host_opengl.cpp does the same
// with WGL on Windows). The config is the one whose native visual is the window's, which EGL needs for the surface.
#include "host.hpp"
#include "esia/rhi/opengl.hpp"
#include <EGL/egl.h>
#include <X11/Xlib.h>
#include <algorithm>
#include <vector>

namespace glass
{
    namespace
    {
        constexpr EGLenum kPlatformX11 = 0x31D5;   // EGL_PLATFORM_X11_KHR (EGL_KHR_platform_x11, part of EGL 1.5's set)
        constexpr EGLint kContextFlags = 0x30FC, kDebugBit = 0x0001;   // EGL_CONTEXT_FLAGS_KHR, EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR
        constexpr unsigned kGlRenderer = 0x1F01;   // GL_RENDERER

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

            const char* Name() const override { return "OpenGL"; }

            bool Init(const NativeWindow& window, int width, int height, const HostOptions& o, std::string& error) override
            {
                vsync_ = o.vsync;
                Display* x = static_cast<Display*>(window.display);
                display_ = ::eglGetPlatformDisplay(kPlatformX11, x, nullptr);   // EGL 1.5
                if (display_ == EGL_NO_DISPLAY)
                    display_ = ::eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(x));
                EGLint major = 0, minor = 0;
                if (display_ == EGL_NO_DISPLAY || !::eglInitialize(display_, &major, &minor))
                {
                    display_ = EGL_NO_DISPLAY;
                    error = "no EGL display for the X server";
                    return false;
                }
                if (!::eglBindAPI(EGL_OPENGL_API))
                {
                    error = "EGL has no desktop OpenGL";
                    return false;
                }
                XWindowAttributes wa = {};
                ::XGetWindowAttributes(x, static_cast<::Window>(window.window), &wa);
                const VisualID visual = ::XVisualIDFromVisual(wa.visual);
                const EGLint want[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
                                       EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_NONE};
                EGLint n = 0;
                ::eglChooseConfig(display_, want, nullptr, 0, &n);
                std::vector<EGLConfig> configs((std::size_t)std::max(n, 0));
                ::eglChooseConfig(display_, want, configs.data(), n, &n);
                EGLConfig config = nullptr;
                for (EGLint i = 0; i < n && !config; ++i)
                {
                    EGLint id = 0, r = 0, g = 0, b = 0;
                    ::eglGetConfigAttrib(display_, configs[i], EGL_NATIVE_VISUAL_ID, &id);
                    ::eglGetConfigAttrib(display_, configs[i], EGL_RED_SIZE, &r);
                    ::eglGetConfigAttrib(display_, configs[i], EGL_GREEN_SIZE, &g);
                    ::eglGetConfigAttrib(display_, configs[i], EGL_BLUE_SIZE, &b);
                    if ((VisualID)id == visual && r == 8 && g == 8 && b == 8)
                        config = configs[i];
                }
                if (!config)
                {
                    error = "no EGL config with the window's visual (RGB8, OpenGL)";
                    return false;
                }
                surface_ = ::eglCreateWindowSurface(display_, config, static_cast<EGLNativeWindowType>(window.window), nullptr);
                // EGL_KHR_create_context's names (EGL 1.4 displays have them too); the debug flag only with --debug
                const EGLint attribs[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 3,
                                          EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
                                          o.debug ? kContextFlags : EGL_NONE, kDebugBit, EGL_NONE};
                context_ = ::eglCreateContext(display_, config, EGL_NO_CONTEXT, attribs);
                if (surface_ == EGL_NO_SURFACE || context_ == EGL_NO_CONTEXT || !::eglMakeCurrent(display_, surface_, surface_, context_))
                {
                    error = "no OpenGL 3.3 core context on the window (EGL error " + std::to_string(::eglGetError()) + ")";
                    return false;
                }
                ::eglSwapInterval(display_, o.vsync ? 1 : 0);
                using GetString = const unsigned char* (*)(unsigned);
                if (const auto getString = reinterpret_cast<GetString>(GetProc("glGetString")))
                    if (const unsigned char* renderer = getString(kGlRenderer))
                        adapter_ = reinterpret_cast<const char*>(renderer);

                esia::rhi::opengl::Desc d;
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

            bool BeginFrame() override { return (bool)target_; }

        protected:
            void Present() override { ::eglSwapBuffers(display_, surface_); }

        private:
            EGLDisplay display_ = EGL_NO_DISPLAY;
            EGLSurface surface_ = EGL_NO_SURFACE;
            EGLContext context_ = EGL_NO_CONTEXT;
            std::unique_ptr<esia::rhi::Device> esia_;
        };
    }

    std::unique_ptr<Host> CreateHostOpenGL() { return std::make_unique<HostOpenGL>(); }
}
