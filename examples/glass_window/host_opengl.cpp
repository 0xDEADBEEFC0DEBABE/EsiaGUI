// glass_window - OpenGL host: a WGL 3.3 core context on the window's own device context (the class is CS_OWNDC),
// current on the render thread, and the default framebuffer as the target.
#include "host.hpp"
#include "esia/rhi/opengl.hpp"
#include <GL/gl.h>

namespace glass
{
    namespace
    {
        // WGL_ARB_create_context and WGL_EXT_swap_control, from the registry: the Windows SDK has no GL/wglext.h
        // (mingw-w64 does), and these few are all the host needs (the backend's gl_headless.cpp does the same).
        constexpr int WGL_CONTEXT_MAJOR_VERSION_ARB = 0x2091, WGL_CONTEXT_MINOR_VERSION_ARB = 0x2092, WGL_CONTEXT_FLAGS_ARB = 0x2094,
                      WGL_CONTEXT_PROFILE_MASK_ARB = 0x9126, WGL_CONTEXT_DEBUG_BIT_ARB = 0x0001, WGL_CONTEXT_CORE_PROFILE_BIT_ARB = 0x0001;
        using PFNWGLCREATECONTEXTATTRIBSARBPROC = HGLRC(WINAPI*)(HDC, HGLRC, const int*);
        using PFNWGLSWAPINTERVALEXTPROC = BOOL(WINAPI*)(int);

        template <class Fn>
        Fn Wgl(const char* name)
        {
            return reinterpret_cast<Fn>(reinterpret_cast<void*>(::wglGetProcAddress(name)));
        }

        class HostOpenGL final : public Host
        {
        public:
            ~HostOpenGL() override
            {
                esia_.reset();   // while its context is current
                if (context_)
                {
                    ::wglMakeCurrent(nullptr, nullptr);
                    ::wglDeleteContext(context_);
                }
            }

            const char* Name() const override { return "OpenGL"; }

            bool Init(const NativeWindow& window, int width, int height, const HostOptions& o, std::string& error) override
            {
                const HWND hwnd = window.hwnd;
                vsync_ = o.vsync;
                dc_ = ::GetDC(hwnd);   // CS_OWNDC: the window's one device context, valid on any thread
                PIXELFORMATDESCRIPTOR pfd = {};
                pfd.nSize = sizeof(pfd);
                pfd.nVersion = 1;
                pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
                pfd.iPixelType = PFD_TYPE_RGBA;
                pfd.cColorBits = 32;
                pfd.cAlphaBits = 8;
                const int format = ::ChoosePixelFormat(dc_, &pfd);
                if (!format || !::SetPixelFormat(dc_, format, &pfd))
                {
                    error = "no double-buffered RGBA8 pixel format";
                    return false;
                }
                // a legacy context first: wglCreateContextAttribsARB is only found with a context current
                HGLRC legacy = ::wglCreateContext(dc_);
                if (!legacy || !::wglMakeCurrent(dc_, legacy))
                {
                    error = "wglCreateContext failed";
                    return false;
                }
                const auto createContext = Wgl<PFNWGLCREATECONTEXTATTRIBSARBPROC>("wglCreateContextAttribsARB");
                const int attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 3,
                                       WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                                       WGL_CONTEXT_FLAGS_ARB, o.debug ? WGL_CONTEXT_DEBUG_BIT_ARB : 0, 0};
                context_ = createContext ? createContext(dc_, nullptr, attribs) : nullptr;
                ::wglMakeCurrent(dc_, context_);
                ::wglDeleteContext(legacy);
                if (!context_)
                {
                    error = "no OpenGL 3.3 core context (wglCreateContextAttribsARB)";
                    return false;
                }
                if (const auto swapInterval = Wgl<PFNWGLSWAPINTERVALEXTPROC>("wglSwapIntervalEXT"))
                    swapInterval(o.vsync ? 1 : 0);
                if (const char* renderer = reinterpret_cast<const char*>(::glGetString(GL_RENDERER)))
                    adapter_ = renderer;

                esia::rhi::opengl::Desc d;
                d.debug = o.debug;   // KHR_debug messages to stderr, counted
                esia_ = esia::rhi::opengl::CreateDevice(d, &error);
                if (!esia_)
                    return false;
                Resize(width, height);
                return (bool)target_;
            }

            esia::rhi::Device& Device() override { return *esia_; }

            // The default framebuffer follows the window: only the wrapper learns the new size.
            void Resize(int width, int height) override
            {
                target_ = esia::rhi::opengl::WrapFramebuffer(*esia_, 0, width, height, esia::rhi::Format::RGBA8_UNORM);
            }

            bool BeginFrame() override { return (bool)target_; }

        protected:
            void Present() override { ::SwapBuffers(dc_); }

        private:
            HDC dc_ = nullptr;
            HGLRC context_ = nullptr;
            std::unique_ptr<esia::rhi::Device> esia_;
        };
    }

    std::unique_ptr<Host> CreateHostOpenGL() { return std::make_unique<HostOpenGL>(); }
}
