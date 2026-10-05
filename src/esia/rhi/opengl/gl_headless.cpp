// Esia OpenGL backend - headless devices for tests (no visible window) and the registration of the two backends.
//
// EGL is loaded at runtime (libEGL.so.1 or libEGL.so; libEGL.dll / libEGL.dylib, e.g. ANGLE's, elsewhere), so the backend links
// no GL or EGL library and a machine without EGL only skips the conformance suite. The display is Mesa's
// surfaceless platform when offered (EGL_MESA_platform_surfaceless: llvmpipe in the cloud sessions, no X or
// Wayland needed), else the default display; the context is made current without a surface
// (EGL_KHR_surfaceless_context) or with a 1 x 1 pbuffer. The config asks for EGL_PBUFFER_BIT: Mesa's surfaceless
// platform offers no config without it.
//
// Windows GPU drivers ship no EGL, so there the context comes from WGL first (opengl32.dll, loaded at runtime too):
// a hidden window only lends its pixel format, the device renders into its own targets. The GL 3.3 core context is
// the driver's; the ES 3.0 context needs WGL_EXT_create_context_es2_profile (NVIDIA, AMD). EGL (ANGLE's DLLs next
// to the executable) remains the fallback for what WGL cannot create.
#include "gl_headless.hpp"
#include "gl_device.hpp"
#include <cstdlib>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace esia::rhi::opengl
{
    namespace
    {
        using EGLDisplay = void*;
        using EGLConfig = void*;
        using EGLContext = void*;
        using EGLSurface = void*;
        using EGLint = std::int32_t;
        using EGLenum = unsigned int;
        using EGLBoolean = unsigned int;
        constexpr EGLint EGL_NONE = 0x3038, EGL_SURFACE_TYPE = 0x3033, EGL_PBUFFER_BIT = 0x0001, EGL_RENDERABLE_TYPE = 0x3040,
                         EGL_OPENGL_BIT = 0x0008, EGL_OPENGL_ES3_BIT = 0x0040, EGL_WIDTH = 0x3057, EGL_HEIGHT = 0x3056,
                         EGL_CONTEXT_MAJOR_VERSION = 0x3098, EGL_CONTEXT_MINOR_VERSION = 0x30FB, EGL_CONTEXT_OPENGL_PROFILE_MASK = 0x30FD,
                         EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT = 0x0001,
                         EGL_EXTENSIONS = 0x3055, EGL_PLATFORM_SURFACELESS_MESA = 0x31DD;
        constexpr EGLenum EGL_OPENGL_API = 0x30A2, EGL_OPENGL_ES_API = 0x30A0;

        struct Egl
        {
            void* (*GetProcAddress)(const char*) = nullptr;
            EGLDisplay (*GetDisplay)(void*) = nullptr;
            EGLDisplay (*GetPlatformDisplayEXT)(EGLenum, void*, const EGLint*) = nullptr;
            EGLBoolean (*Initialize)(EGLDisplay, EGLint*, EGLint*) = nullptr;
            const char* (*QueryString)(EGLDisplay, EGLint) = nullptr;
            EGLBoolean (*BindAPI)(EGLenum) = nullptr;
            EGLBoolean (*ChooseConfig)(EGLDisplay, const EGLint*, EGLConfig*, EGLint, EGLint*) = nullptr;
            EGLContext (*CreateContext)(EGLDisplay, EGLConfig, EGLContext, const EGLint*) = nullptr;
            EGLBoolean (*DestroyContext)(EGLDisplay, EGLContext) = nullptr;
            EGLSurface (*CreatePbufferSurface)(EGLDisplay, EGLConfig, const EGLint*) = nullptr;
            EGLBoolean (*DestroySurface)(EGLDisplay, EGLSurface) = nullptr;
            EGLBoolean (*MakeCurrent)(EGLDisplay, EGLSurface, EGLSurface, EGLContext) = nullptr;
            EGLint (*GetError)() = nullptr;
            EGLDisplay display = nullptr;
            bool surfaceless = false;   // EGL_KHR_surfaceless_context
            std::string error;
        };

        bool HasExtension(const char* list, const char* name)
        {
            const std::size_t n = std::strlen(name);
            for (const char* p = list ? std::strstr(list, name) : nullptr; p; p = std::strstr(p + n, name))
                if ((p == list || p[-1] == ' ') && (p[n] == ' ' || p[n] == 0))
                    return true;
            return false;
        }

        // The library and the display, once per process (never terminated: other devices may still use it).
        const Egl& LoadEgl()
        {
            static const Egl egl = [] {
                Egl e;
#if defined(_WIN32)
                HMODULE lib = LoadLibraryA("libEGL.dll");
                auto sym = [&](const char* n) { return lib ? reinterpret_cast<void*>(GetProcAddress(lib, n)) : nullptr; };
#elif defined(__APPLE__)
                void* lib = dlopen("libEGL.dylib", RTLD_NOW | RTLD_LOCAL);
                auto sym = [&](const char* n) { return lib ? dlsym(lib, n) : nullptr; };
#else
                void* lib = dlopen("libEGL.so.1", RTLD_NOW | RTLD_LOCAL);   // glvnd / Mesa
                if (!lib)
                    lib = dlopen("libEGL.so", RTLD_NOW | RTLD_LOCAL);       // Android
                auto sym = [&](const char* n) { return lib ? dlsym(lib, n) : nullptr; };
#endif
                if (!lib)
                {
                    e.error = "no EGL library on this machine";
                    return e;
                }
                auto load = [&](auto& f, const char* name) { f = reinterpret_cast<std::remove_reference_t<decltype(f)>>(sym(name)); };
                load(e.GetProcAddress, "eglGetProcAddress");
                load(e.GetDisplay, "eglGetDisplay");
                load(e.Initialize, "eglInitialize");
                load(e.QueryString, "eglQueryString");
                load(e.BindAPI, "eglBindAPI");
                load(e.ChooseConfig, "eglChooseConfig");
                load(e.CreateContext, "eglCreateContext");
                load(e.DestroyContext, "eglDestroyContext");
                load(e.CreatePbufferSurface, "eglCreatePbufferSurface");
                load(e.DestroySurface, "eglDestroySurface");
                load(e.MakeCurrent, "eglMakeCurrent");
                load(e.GetError, "eglGetError");
                if (!e.GetProcAddress || !e.GetDisplay || !e.Initialize || !e.QueryString || !e.BindAPI || !e.ChooseConfig || !e.CreateContext ||
                    !e.DestroyContext || !e.CreatePbufferSurface || !e.DestroySurface || !e.MakeCurrent || !e.GetError)
                {
                    e.error = "the EGL library lacks EGL 1.4 functions";
                    return e;
                }
                const char* client = e.QueryString(nullptr, EGL_EXTENSIONS);   // client extensions (EGL 1.5 / EXT_client_extensions)
                if (HasExtension(client, "EGL_MESA_platform_surfaceless") && HasExtension(client, "EGL_EXT_platform_base"))
                {
                    e.GetPlatformDisplayEXT = reinterpret_cast<decltype(e.GetPlatformDisplayEXT)>(e.GetProcAddress("eglGetPlatformDisplayEXT"));
                    if (e.GetPlatformDisplayEXT)
                        e.display = e.GetPlatformDisplayEXT(EGL_PLATFORM_SURFACELESS_MESA, nullptr, nullptr);
                }
                if (!e.display)
                    e.display = e.GetDisplay(nullptr);
                if (!e.display || !e.Initialize(e.display, nullptr, nullptr))
                {
                    e.display = nullptr;
                    e.error = "no EGL display (no GPU driver, and no Mesa surfaceless platform)";
                    return e;
                }
                e.surfaceless = HasExtension(e.QueryString(e.display, EGL_EXTENSIONS), "EGL_KHR_surfaceless_context");
                return e;
            }();
            return egl;
        }

        class EglContext final : public OwnedContext
        {
        public:
            EglContext(const Egl& egl, EGLContext context, EGLSurface surface) : egl_(egl), context_(context), surface_(surface) {}
            ~EglContext() override
            {
                egl_.MakeCurrent(egl_.display, nullptr, nullptr, nullptr);
                if (surface_)
                    egl_.DestroySurface(egl_.display, surface_);
                egl_.DestroyContext(egl_.display, context_);
            }
            void MakeCurrent() override { egl_.MakeCurrent(egl_.display, surface_, surface_, context_); }

        private:
            const Egl& egl_;
            EGLContext context_;
            EGLSurface surface_;
        };

        // A current context of the API, owned by the returned object; null (and `error`) when EGL cannot make one.
        std::unique_ptr<EglContext> CreateEglContext(bool es, std::string& error)
        {
            const Egl& egl = LoadEgl();
            if (!egl.display)
            {
                error = egl.error;
                return nullptr;
            }
            if (!egl.BindAPI(es ? EGL_OPENGL_ES_API : EGL_OPENGL_API))
            {
                error = es ? "EGL has no OpenGL ES API" : "EGL has no desktop OpenGL API";
                return nullptr;
            }
            const EGLint configAttribs[] = {EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, es ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_BIT, EGL_NONE};
            EGLConfig config = nullptr;
            EGLint configs = 0;
            if (!egl.ChooseConfig(egl.display, configAttribs, &config, 1, &configs) || configs < 1)
            {
                error = es ? "no EGL config for OpenGL ES 3" : "no EGL config for desktop OpenGL";
                return nullptr;
            }
#ifdef NDEBUG
            constexpr bool kDebugContext = false;
#else
            constexpr bool kDebugContext = true;   // full KHR_debug output in debug builds
#endif
            // EGL_KHR_create_context's names, which EGL 1.4 displays (Android's) take as well as 1.5 ones; the debug flag
            // only when it is wanted (an EGL 1.4 driver refuses the 1.5 attribute EGL_CONTEXT_OPENGL_DEBUG outright)
            constexpr EGLint kContextFlags = 0x30FC, kDebugBit = 0x0001;   // EGL_CONTEXT_FLAGS_KHR, EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR
            const EGLint debugKey = kDebugContext ? kContextFlags : EGL_NONE;
            const EGLint glAttribs[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 3, EGL_CONTEXT_OPENGL_PROFILE_MASK,
                                        EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT, debugKey, kDebugBit, EGL_NONE};
            const EGLint esAttribs[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 0, debugKey, kDebugBit, EGL_NONE};
            EGLContext context = egl.CreateContext(egl.display, config, nullptr, es ? esAttribs : glAttribs);
            if (!context)
            {
                error = es ? "EGL cannot create an OpenGL ES 3.0 context" : "EGL cannot create an OpenGL 3.3 core context";
                return nullptr;
            }
            EGLSurface surface = nullptr;
            if (!egl.surfaceless)
            {
                const EGLint pbuffer[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};
                surface = egl.CreatePbufferSurface(egl.display, config, pbuffer);
            }
            auto owned = std::make_unique<EglContext>(egl, context, surface);
            if (!egl.MakeCurrent(egl.display, surface, surface, context))
            {
                error = "eglMakeCurrent failed";
                return nullptr;
            }
            return owned;
        }

#if defined(_WIN32)
        constexpr int WGL_CONTEXT_MAJOR_VERSION_ARB = 0x2091, WGL_CONTEXT_MINOR_VERSION_ARB = 0x2092, WGL_CONTEXT_FLAGS_ARB = 0x2094,
                      WGL_CONTEXT_PROFILE_MASK_ARB = 0x9126, WGL_CONTEXT_DEBUG_BIT_ARB = 0x0001, WGL_CONTEXT_CORE_PROFILE_BIT_ARB = 0x0001,
                      WGL_CONTEXT_ES2_PROFILE_BIT_EXT = 0x0004;

        struct Wgl
        {
            HGLRC(WINAPI* CreateContext)(HDC) = nullptr;
            BOOL(WINAPI* DeleteContext)(HGLRC) = nullptr;
            BOOL(WINAPI* MakeCurrent)(HDC, HGLRC) = nullptr;
            HGLRC(WINAPI* GetCurrentContext)() = nullptr;
            PROC(WINAPI* GetProcAddress)(LPCSTR) = nullptr;
        };

        // opengl32.dll's wgl functions, once per process (all null when the DLL or one of them is missing)
        const Wgl& LoadWgl()
        {
            static const Wgl wgl = [] {
                Wgl w;
                HMODULE lib = LoadLibraryA("opengl32.dll");
                if (!lib)
                    return w;
                auto load = [&](auto& f, const char* name) {
                    f = reinterpret_cast<std::remove_reference_t<decltype(f)>>(reinterpret_cast<void*>(::GetProcAddress(lib, name)));
                };
                load(w.CreateContext, "wglCreateContext");
                load(w.DeleteContext, "wglDeleteContext");
                load(w.MakeCurrent, "wglMakeCurrent");
                load(w.GetCurrentContext, "wglGetCurrentContext");
                load(w.GetProcAddress, "wglGetProcAddress");
                if (!w.CreateContext || !w.DeleteContext || !w.MakeCurrent || !w.GetCurrentContext || !w.GetProcAddress)
                    return Wgl{};
                return w;
            }();
            return wgl;
        }

        class WglContext final : public OwnedContext
        {
        public:
            WglContext(HWND window, HDC dc) : window_(window), dc_(dc) {}
            ~WglContext() override
            {
                const Wgl& wgl = LoadWgl();
                if (context_)
                {
                    if (wgl.GetCurrentContext() == context_)
                        wgl.MakeCurrent(nullptr, nullptr);
                    wgl.DeleteContext(context_);
                }
                ReleaseDC(window_, dc_);
                DestroyWindow(window_);
            }
            void MakeCurrent() override { LoadWgl().MakeCurrent(dc_, context_); }

            HDC Dc() const { return dc_; }
            void SetContext(HGLRC context) { context_ = context; }

        private:
            HWND window_;
            HDC dc_;
            HGLRC context_ = nullptr;
        };

        // A current context of the API on the GPU driver's WGL; null (and `error`) when the driver cannot make one.
        std::unique_ptr<WglContext> CreateWglContext(bool es, std::string& error)
        {
            const Wgl& wgl = LoadWgl();
            if (!wgl.CreateContext)
            {
                error = "no opengl32.dll";
                return nullptr;
            }
            static const wchar_t* const kClass = [] {
                WNDCLASSEXW wc{};
                wc.cbSize = sizeof(wc);
                wc.style = CS_OWNDC;
                wc.lpfnWndProc = DefWindowProcW;
                wc.hInstance = GetModuleHandleW(nullptr);
                wc.lpszClassName = L"EsiaGlHeadless";
                RegisterClassExW(&wc);
                return wc.lpszClassName;
            }();
            // never shown: the window only gives the context a pixel format
            HWND window = CreateWindowExW(0, kClass, L"", WS_OVERLAPPEDWINDOW, 0, 0, 16, 16, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
            HDC dc = window ? GetDC(window) : nullptr;
            if (!dc)
            {
                if (window)
                    DestroyWindow(window);
                error = "cannot create a hidden window for WGL";
                return nullptr;
            }
            auto owned = std::make_unique<WglContext>(window, dc);
            PIXELFORMATDESCRIPTOR pfd{};
            pfd.nSize = sizeof(pfd);
            pfd.nVersion = 1;
            pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.cColorBits = 32;
            const int format = ChoosePixelFormat(dc, &pfd);
            if (format == 0 || !SetPixelFormat(dc, format, &pfd))
            {
                error = "no OpenGL pixel format (no GPU driver with OpenGL)";
                return nullptr;
            }
            // wglCreateContextAttribsARB is only reachable through a current legacy context
            HGLRC legacy = wgl.CreateContext(dc);
            if (!legacy || !wgl.MakeCurrent(dc, legacy))
            {
                if (legacy)
                    wgl.DeleteContext(legacy);
                error = "wglCreateContext failed";
                return nullptr;
            }
            using CreateContextAttribs = HGLRC(WINAPI*)(HDC, HGLRC, const int*);
            using GetExtensionsString = const char*(WINAPI*)(HDC);
            const auto createAttribs = reinterpret_cast<CreateContextAttribs>(reinterpret_cast<void*>(wgl.GetProcAddress("wglCreateContextAttribsARB")));
            const auto extensions = reinterpret_cast<GetExtensionsString>(reinterpret_cast<void*>(wgl.GetProcAddress("wglGetExtensionsStringARB")));
            const char* ext = extensions ? extensions(dc) : nullptr;
            const bool esProfile = HasExtension(ext, "WGL_EXT_create_context_es2_profile") || HasExtension(ext, "WGL_EXT_create_context_es_profile");
            HGLRC context = nullptr;
            if (!createAttribs)
                error = "the driver has no WGL_ARB_create_context";
            else if (es && !esProfile)
                error = "the driver has no WGL_EXT_create_context_es2_profile";
            else
            {
#ifdef NDEBUG
                constexpr bool kDebugContext = false;
#else
                constexpr bool kDebugContext = true;   // full KHR_debug output in debug builds
#endif
                constexpr int kFlags = kDebugContext ? WGL_CONTEXT_DEBUG_BIT_ARB : 0;   // (used either way: -Werror release builds)
                const int glAttribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 3, WGL_CONTEXT_PROFILE_MASK_ARB,
                                         WGL_CONTEXT_CORE_PROFILE_BIT_ARB, WGL_CONTEXT_FLAGS_ARB, kFlags, 0};
                const int esAttribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 0, WGL_CONTEXT_PROFILE_MASK_ARB,
                                         WGL_CONTEXT_ES2_PROFILE_BIT_EXT, WGL_CONTEXT_FLAGS_ARB, kFlags, 0};
                context = createAttribs(dc, nullptr, es ? esAttribs : glAttribs);
                if (!context)
                    error = es ? "WGL cannot create an OpenGL ES 3.0 context" : "WGL cannot create an OpenGL 3.3 core context";
            }
            wgl.MakeCurrent(nullptr, nullptr);
            wgl.DeleteContext(legacy);
            if (!context)
                return nullptr;
            owned->SetContext(context);
            if (!wgl.MakeCurrent(dc, context))
            {
                error = "wglMakeCurrent failed";
                return nullptr;
            }
            return owned;
        }
#endif

        // A current context of the API, owned by the returned object, and the lookup of its GL functions in `proc`;
        // null (and `error`) when this machine cannot make one.
        std::unique_ptr<OwnedContext> CreateContext(bool es, GetProcAddressFn& proc, std::string& error)
        {
#if defined(_WIN32)
            std::string wglError;
            if (auto wgl = CreateWglContext(es, wglError))
            {
                proc = &DefaultGetProcAddress;   // wglGetProcAddress, and opengl32.dll for GL 1.1
                return wgl;
            }
#endif
            if (auto egl = CreateEglContext(es, error))
            {
                proc = LoadEgl().GetProcAddress;
                return egl;
            }
#if defined(_WIN32)
            error = "WGL: " + wglError + "; EGL: " + error;
#endif
            return nullptr;
        }

        // ESIA_GL_CORE_ONLY=1 runs the conformance suite on the GL 3.3 / GLES 3.0 minimum (Desc::coreOnly)
        HeadlessDevice CreateRegistered(bool es, const HeadlessDesc& hd, std::string& error)
        {
            Desc desc;
            desc.es = es;
            const char* coreOnly = std::getenv("ESIA_GL_CORE_ONLY");
            desc.coreOnly = coreOnly && std::strcmp(coreOnly, "0") != 0;
            return CreateHeadlessDevice(desc, hd, error);
        }

        HeadlessDevice CreateHeadlessGl(const HeadlessDesc& d, std::string& error) { return CreateRegistered(false, d, error); }
        HeadlessDevice CreateHeadlessGles(const HeadlessDesc& d, std::string& error) { return CreateRegistered(true, d, error); }
    }

    GetProcAddressFn HeadlessGetProcAddress()
    {
#if defined(_WIN32)
        if (LoadWgl().GetCurrentContext && LoadWgl().GetCurrentContext())
            return &DefaultGetProcAddress;
#endif
        return LoadEgl().GetProcAddress;
    }

    HeadlessDevice CreateHeadlessDevice(Desc desc, const HeadlessDesc& hd, std::string& error)
    {
        GetProcAddressFn proc = nullptr;
        auto context = CreateContext(desc.es, proc, error);
        if (!context)
            return {};
        desc.getProcAddress = proc;
        auto device = std::make_unique<GlDevice>(desc, std::move(context));
        if (!device->Init(error))
            return {};
        TextureDesc td;
        td.width = hd.width;
        td.height = hd.height;
        td.format = hd.format;
        td.samples = hd.samples;
        td.usage = TextureUsage_RenderTarget | TextureUsage_CopySrc;
        if (hd.sampleable && hd.samples == 1 && device->SamplesRaw(hd.format))
            td.usage |= TextureUsage_Sampled;
        td.debugName = "headless-target";
        HeadlessDevice h;
        h.target = device->CreateTexture(td, nullptr, 0);
        if (!h.target)
        {
            error = std::string(FormatName(hd.format)) + (hd.samples > 1 ? " x" + std::to_string(hd.samples) + " samples" : "") +
                    " is not a render target format here";
            return {};
        }
        h.adapter = device->RendererName();
        h.device = std::move(device);
        return h;
    }
}

void EsiaRegisterBackend_opengl()
{
    esia::rhi::BackendInfo info;
    info.name = "opengl";
    info.createHeadless = &esia::rhi::opengl::CreateHeadlessGl;
    esia::rhi::RegisterBackend(info);
}

void EsiaRegisterBackend_gles()
{
    esia::rhi::BackendInfo info;
    info.name = "gles";
    info.createHeadless = &esia::rhi::opengl::CreateHeadlessGles;
    esia::rhi::RegisterBackend(info);
}
