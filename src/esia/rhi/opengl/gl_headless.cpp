// Esia OpenGL backend - headless devices for tests (EGL, no window) and the registration of the two backends.
//
// EGL is loaded at runtime (libEGL.so.1; libEGL.dll / libEGL.dylib, e.g. ANGLE's, elsewhere), so the backend links
// no GL or EGL library and a machine without EGL only skips the conformance suite. The display is Mesa's
// surfaceless platform when offered (EGL_MESA_platform_surfaceless: llvmpipe in the cloud sessions, no X or
// Wayland needed), else the default display; the context is made current without a surface
// (EGL_KHR_surfaceless_context) or with a 1 x 1 pbuffer. The config asks for EGL_PBUFFER_BIT: Mesa's surfaceless
// platform offers no config without it.
#include "gl_device.hpp"
#include "esia/rhi/backend_registry.hpp"
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
                         EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT = 0x0001, EGL_CONTEXT_OPENGL_DEBUG = 0x31B0, EGL_TRUE = 1,
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
                void* lib = dlopen("libEGL.so.1", RTLD_NOW | RTLD_LOCAL);
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
        std::unique_ptr<EglContext> CreateContext(bool es, std::string& error)
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
            const EGLint debug = 0;
#else
            const EGLint debug = EGL_TRUE;   // full KHR_debug output in debug builds
#endif
            const EGLint glAttribs[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 3, EGL_CONTEXT_OPENGL_PROFILE_MASK,
                                        EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT, EGL_CONTEXT_OPENGL_DEBUG, debug, EGL_NONE};
            const EGLint esAttribs[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 0, EGL_CONTEXT_OPENGL_DEBUG, debug, EGL_NONE};
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

        HeadlessDevice CreateHeadless(bool es, const HeadlessDesc& hd, std::string& error)
        {
            auto context = CreateContext(es, error);
            if (!context)
                return {};
            Desc desc;
            desc.es = es;
            desc.getProcAddress = LoadEgl().GetProcAddress;
            // ESIA_GL_CORE_ONLY=1 runs the suite on the GL 3.3 / GLES 3.0 minimum (Desc::coreOnly)
            const char* coreOnly = std::getenv("ESIA_GL_CORE_ONLY");
            desc.coreOnly = coreOnly && std::strcmp(coreOnly, "0") != 0;
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
            h.device = std::move(device);
            return h;
        }

        HeadlessDevice CreateHeadlessGl(const HeadlessDesc& d, std::string& error) { return CreateHeadless(false, d, error); }
        HeadlessDevice CreateHeadlessGles(const HeadlessDesc& d, std::string& error) { return CreateHeadless(true, d, error); }
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
