// Shader library check on a real driver: every GLSL 330 and ESSL 300 program of the generated library compiles and
// links (vertex outputs match fragment inputs, uniform blocks and samplers exist) with the system's OpenGL / OpenGL
// ES through EGL - Mesa's llvmpipe in CI, no window and no GPU needed. This is not a backend: it only proves the
// shaders the OpenGL backend will load are valid. Skipped (exit 0) when EGL offers no such context.
#include "esia/render/shader_library.hpp"
#include "esia_test.hpp"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <cstdio>
#include <cstring>
#include <string>

using namespace esia;

namespace
{
    // The few GL entry points the check needs, loaded through eglGetProcAddress (EGL 1.5 returns core functions),
    // so neither libGL nor libGLESv2 has to be linked.
    using GLuint = unsigned int;
    using GLint = int;
    using GLenum = unsigned int;
    using GLsizei = int;
    using GLchar = char;
    constexpr GLenum kVertexShader = 0x8B31, kFragmentShader = 0x8B30, kCompileStatus = 0x8B81, kLinkStatus = 0x8B82;
    constexpr GLenum kVersion = 0x1F02;
    constexpr GLuint kInvalidIndex = 0xFFFFFFFFu;

    struct Gl
    {
        GLuint (*CreateShader)(GLenum);
        void (*ShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*);
        void (*CompileShader)(GLuint);
        void (*GetShaderiv)(GLuint, GLenum, GLint*);
        void (*GetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
        void (*DeleteShader)(GLuint);
        GLuint (*CreateProgram)();
        void (*AttachShader)(GLuint, GLuint);
        void (*LinkProgram)(GLuint);
        void (*GetProgramiv)(GLuint, GLenum, GLint*);
        void (*GetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
        void (*DeleteProgram)(GLuint);
        GLuint (*GetUniformBlockIndex)(GLuint, const GLchar*);
        const unsigned char* (*GetString)(GLenum);

        template <class F>
        static bool Load(F& f, const char* name)
        {
            f = reinterpret_cast<F>(eglGetProcAddress(name));
            return f != nullptr;
        }
        bool Load()
        {
            return Load(CreateShader, "glCreateShader") && Load(ShaderSource, "glShaderSource") && Load(CompileShader, "glCompileShader") &&
                   Load(GetShaderiv, "glGetShaderiv") && Load(GetShaderInfoLog, "glGetShaderInfoLog") && Load(DeleteShader, "glDeleteShader") &&
                   Load(CreateProgram, "glCreateProgram") && Load(AttachShader, "glAttachShader") && Load(LinkProgram, "glLinkProgram") &&
                   Load(GetProgramiv, "glGetProgramiv") && Load(GetProgramInfoLog, "glGetProgramInfoLog") && Load(DeleteProgram, "glDeleteProgram") &&
                   Load(GetUniformBlockIndex, "glGetUniformBlockIndex") && Load(GetString, "glGetString");
        }
    };

    struct EglContext
    {
        EGLDisplay display = EGL_NO_DISPLAY;
        EGLContext context = EGL_NO_CONTEXT;

        // Surfaceless (EGL_MESA_platform_surfaceless) context of the API; false when this machine has none.
        bool Create(bool es)
        {
            auto getDisplay = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
            display = getDisplay ? getDisplay(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr) : EGL_NO_DISPLAY;
            if (display == EGL_NO_DISPLAY || !eglInitialize(display, nullptr, nullptr))
                return false;
            if (!eglBindAPI(es ? EGL_OPENGL_ES_API : EGL_OPENGL_API))
                return false;
            const EGLint config[] = {EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, es ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_BIT, EGL_NONE};
            EGLConfig cfg;
            EGLint n = 0;
            if (!eglChooseConfig(display, config, &cfg, 1, &n) || n < 1)
                return false;
            const EGLint gl[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 3, EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT, EGL_NONE};
            const EGLint gles[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 0, EGL_NONE};
            context = eglCreateContext(display, cfg, EGL_NO_CONTEXT, es ? gles : gl);
            return context != EGL_NO_CONTEXT && eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context);
        }
        ~EglContext()
        {
            if (context != EGL_NO_CONTEXT)
            {
                eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                eglDestroyContext(display, context);
            }
        }
    };

    bool Compile(const Gl& gl, GLenum type, const shaders::ShaderBlob& blob, GLuint& out)
    {
        out = gl.CreateShader(type);
        const char* src = reinterpret_cast<const char*>(blob.data);
        const GLint len = (GLint)blob.size;
        gl.ShaderSource(out, 1, &src, &len);
        gl.CompileShader(out);
        GLint ok = 0;
        gl.GetShaderiv(out, kCompileStatus, &ok);
        if (!ok)
        {
            char log[2048] = {};
            gl.GetShaderInfoLog(out, sizeof(log), nullptr, log);
            std::fprintf(stderr, "    %s: %s\n", blob.entry, log);
        }
        return ok != 0;
    }

    // Returns the number of programs checked (-1 = no context of that API here).
    int CheckFormat(shaders::Format format)
    {
        const bool es = format == shaders::Format::Essl300;
        EglContext egl;
        Gl gl{};
        if (!egl.Create(es) || !gl.Load())
            return -1;
        std::printf("  %s on %s\n", shaders::FormatName(format), reinterpret_cast<const char*>(gl.GetString(kVersion)));
        int checked = 0;
        for (int p = 0; p < (int)rhi::ShaderProgram::Count; ++p)
        {
            const auto program = (rhi::ShaderProgram)p;
            const shaders::ShaderBlob* vs = shaders::Find(format, program, shaders::Stage::Vertex);
            const shaders::ShaderBlob* ps = shaders::Find(format, program, shaders::Stage::Pixel);
            ESIA_CHECK(vs && ps && vs->text && ps->text);
            if (!vs || !ps)
                continue;
            GLuint v = 0, f = 0;
            const bool vsOk = Compile(gl, kVertexShader, *vs, v);
            const bool psOk = Compile(gl, kFragmentShader, *ps, f);   // both, so both logs are printed
            const bool compiled = vsOk && psOk;
            ESIA_CHECK(compiled);
            const GLuint prog = gl.CreateProgram();
            gl.AttachShader(prog, v);
            gl.AttachShader(prog, f);
            gl.LinkProgram(prog);
            GLint linked = 0;
            gl.GetProgramiv(prog, kLinkStatus, &linked);
            if (!linked)
            {
                char log[2048] = {};
                gl.GetProgramInfoLog(prog, sizeof(log), nullptr, log);
                std::fprintf(stderr, "    link %s: %s\n", rhi::ShaderProgramName(program), log);
            }
            ESIA_CHECK(compiled && linked);
            // every program reads the frame constants (the backend binds the blocks by these names)
            ESIA_CHECK(!linked || gl.GetUniformBlockIndex(prog, "WgtFrame") != kInvalidIndex);
            gl.DeleteProgram(prog);
            gl.DeleteShader(v);
            gl.DeleteShader(f);
            ++checked;
        }
        return checked;
    }
}

ESIA_TEST(ShaderLibrary, GlslAndEsslLinkOnTheDriver)
{
    for (shaders::Format f : {shaders::Format::Glsl330, shaders::Format::Essl300})
    {
        const int n = CheckFormat(f);
        if (n < 0)
            std::printf("  %s: no EGL context of that API on this machine - skipped\n", shaders::FormatName(f));
        else
            ESIA_CHECK(n == (int)rhi::ShaderProgram::Count);
    }
}
