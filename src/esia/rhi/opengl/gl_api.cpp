// Esia OpenGL backend - runtime loading of the GL entry points (see gl_api.hpp)
#include "gl_api.hpp"
#include <cstdint>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace esia::rhi::opengl
{
    bool GlApi::Load(void* (*proc)(const char*), const char*& missing)
    {
        getProc = proc ? proc : &DefaultGetProcAddress;
        missing = nullptr;
#define ESIA_GL_LOAD_CORE(ret, name, params)                                    \
    name = reinterpret_cast<ret(ESIA_GLAPI*) params>(getProc("gl" #name));      \
    if (!name && !missing)                                                      \
        missing = "gl" #name;
        ESIA_GL_CORE(ESIA_GL_LOAD_CORE)
#undef ESIA_GL_LOAD_CORE
        return missing == nullptr;
    }

#if defined(_WIN32)
    void* DefaultGetProcAddress(const char* name)
    {
        // wglGetProcAddress knows only extension / post-1.1 functions and signals failure with small values too;
        // GL 1.1 functions come from opengl32.dll itself
        static HMODULE gl = LoadLibraryA("opengl32.dll");
        if (!gl)
            return nullptr;
        using WglGetProcAddress = PROC(WINAPI*)(LPCSTR);
        static auto wglGet = reinterpret_cast<WglGetProcAddress>(reinterpret_cast<void*>(GetProcAddress(gl, "wglGetProcAddress")));
        if (wglGet)
        {
            void* p = reinterpret_cast<void*>(wglGet(name));
            const std::intptr_t v = reinterpret_cast<std::intptr_t>(p);
            if (v != 0 && v != 1 && v != 2 && v != 3 && v != -1)
                return p;
        }
        return reinterpret_cast<void*>(GetProcAddress(gl, name));
    }
#elif defined(__APPLE__)
    void* DefaultGetProcAddress(const char* name)
    {
        // the host linked the OpenGL framework (or ANGLE): its symbols are in the process image
        return dlsym(RTLD_DEFAULT, name);
    }
#else
    void* DefaultGetProcAddress(const char* name)
    {
        // Ask the window system library that owns the current context (EGL or GLX, whichever the host loaded);
        // fall back to the symbols the process was linked with.
        using GetProc = void* (*)(const char*);
        using GetCurrent = void* (*)();
        static const GetProc egl = reinterpret_cast<GetProc>(dlsym(RTLD_DEFAULT, "eglGetProcAddress"));
        static const GetCurrent eglCurrent = reinterpret_cast<GetCurrent>(dlsym(RTLD_DEFAULT, "eglGetCurrentContext"));
        static const GetProc glx = reinterpret_cast<GetProc>(dlsym(RTLD_DEFAULT, "glXGetProcAddressARB"));
        if (egl && (!glx || (eglCurrent && eglCurrent())))
            if (void* p = egl(name))
                return p;
        if (glx)
            if (void* p = glx(name))
                return p;
        return dlsym(RTLD_DEFAULT, name);
    }
#endif
}
