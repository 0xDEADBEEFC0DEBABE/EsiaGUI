// Esia OpenGL backend - the GL entry points and constants it uses, loaded at runtime (no GLEW / glad / system GL
// headers: the Windows SDK only ships GL 1.1, macOS and Android differ again, and the backend must build everywhere).
//
// One GlApi per device: on Windows (WGL) function pointers may differ between contexts. Everything listed in
// ESIA_GL_CORE exists in both OpenGL 3.3 core and OpenGL ES 3.0. ESIA_GL_OPTIONAL entries come from later versions
// or extensions: the device loads them by their exact name once it knows the version and the extensions (some
// lookups, Mesa's eglGetProcAddress among them, return a stub for any name, so a pointer proves nothing).
#pragma once
#include <cstddef>
#include <cstdint>

#if defined(_WIN32)
#define ESIA_GLAPI __stdcall
#else
#define ESIA_GLAPI
#endif

namespace esia::rhi::opengl
{
    using GLenum = unsigned int;
    using GLuint = unsigned int;
    using GLint = int;
    using GLsizei = int;
    using GLboolean = unsigned char;
    using GLbitfield = unsigned int;
    using GLfloat = float;
    using GLchar = char;
    using GLubyte = unsigned char;
    using GLintptr = std::ptrdiff_t;
    using GLsizeiptr = std::ptrdiff_t;
    using GLint64 = std::int64_t;
    using GLuint64 = std::uint64_t;
    using GLDEBUGPROC = void(ESIA_GLAPI*)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
                                          const void* user);

    // ------------------------------------------------------------------ constants
    constexpr GLenum GL_NO_ERROR = 0, GL_NONE = 0, GL_ZERO = 0, GL_ONE = 1, GL_FALSE = 0, GL_TRUE = 1;
    constexpr GLenum GL_TRIANGLES = 0x0004, GL_TRIANGLE_STRIP = 0x0005;
    constexpr GLenum GL_SRC_ALPHA = 0x0302, GL_ONE_MINUS_SRC_ALPHA = 0x0303, GL_FUNC_ADD = 0x8006;
    constexpr GLenum GL_BLEND_DST_RGB = 0x80C8, GL_BLEND_SRC_RGB = 0x80C9, GL_BLEND_DST_ALPHA = 0x80CA, GL_BLEND_SRC_ALPHA = 0x80CB,
                     GL_BLEND_EQUATION_RGB = 0x8009, GL_BLEND_EQUATION_ALPHA = 0x883D;
    constexpr GLenum GL_CULL_FACE = 0x0B44, GL_DEPTH_TEST = 0x0B71, GL_STENCIL_TEST = 0x0B90, GL_DITHER = 0x0BD0, GL_BLEND = 0x0BE2,
                     GL_SCISSOR_TEST = 0x0C11, GL_MULTISAMPLE = 0x809D, GL_SAMPLE_ALPHA_TO_COVERAGE = 0x809E, GL_RASTERIZER_DISCARD = 0x8C89,
                     GL_FRAMEBUFFER_SRGB = 0x8DB9, GL_PRIMITIVE_RESTART = 0x8F9D, GL_PRIMITIVE_RESTART_FIXED_INDEX = 0x8D69;
    constexpr GLenum GL_VIEWPORT = 0x0BA2, GL_SCISSOR_BOX = 0x0C10, GL_COLOR_CLEAR_VALUE = 0x0C22, GL_COLOR_WRITEMASK = 0x0C23,
                     GL_POLYGON_MODE = 0x0B40, GL_FRONT_AND_BACK = 0x0408, GL_FILL = 0x1B02;
    constexpr GLenum GL_TEXTURE_2D = 0x0DE1, GL_TEXTURE0 = 0x84C0, GL_ACTIVE_TEXTURE = 0x84E0, GL_TEXTURE_BINDING_2D = 0x8069,
                     GL_SAMPLER_BINDING = 0x8919, GL_TEXTURE_MAG_FILTER = 0x2800, GL_TEXTURE_MIN_FILTER = 0x2801, GL_TEXTURE_WRAP_S = 0x2802,
                     GL_TEXTURE_WRAP_T = 0x2803, GL_TEXTURE_BASE_LEVEL = 0x813C, GL_TEXTURE_MAX_LEVEL = 0x813D, GL_NEAREST = 0x2600,
                     GL_LINEAR = 0x2601, GL_CLAMP_TO_EDGE = 0x812F, GL_TEXTURE_SRGB_DECODE_EXT = 0x8A48, GL_SKIP_DECODE_EXT = 0x8A4A;
    constexpr GLenum GL_RED = 0x1903, GL_RGBA = 0x1908, GL_R8 = 0x8229, GL_RGBA8 = 0x8058, GL_RGB10_A2 = 0x8059, GL_SRGB8_ALPHA8 = 0x8C43,
                     GL_RGBA16F = 0x881A, GL_RGBA32F = 0x8814;
    constexpr GLenum GL_UNSIGNED_BYTE = 0x1401, GL_UNSIGNED_INT = 0x1405, GL_FLOAT = 0x1406, GL_HALF_FLOAT = 0x140B,
                     GL_UNSIGNED_INT_2_10_10_10_REV = 0x8368;
    constexpr GLenum GL_UNPACK_ROW_LENGTH = 0x0CF2, GL_UNPACK_SKIP_ROWS = 0x0CF3, GL_UNPACK_SKIP_PIXELS = 0x0CF4, GL_UNPACK_ALIGNMENT = 0x0CF5,
                     GL_PACK_ROW_LENGTH = 0x0D02, GL_PACK_SKIP_ROWS = 0x0D03, GL_PACK_SKIP_PIXELS = 0x0D04, GL_PACK_ALIGNMENT = 0x0D05,
                     GL_PIXEL_PACK_BUFFER = 0x88EB, GL_PIXEL_UNPACK_BUFFER = 0x88EC, GL_PIXEL_PACK_BUFFER_BINDING = 0x88ED,
                     GL_PIXEL_UNPACK_BUFFER_BINDING = 0x88EF;
    constexpr GLenum GL_FRAMEBUFFER = 0x8D40, GL_READ_FRAMEBUFFER = 0x8CA8, GL_DRAW_FRAMEBUFFER = 0x8CA9, GL_RENDERBUFFER = 0x8D41,
                     GL_COLOR_ATTACHMENT0 = 0x8CE0, GL_FRAMEBUFFER_COMPLETE = 0x8CD5, GL_DRAW_FRAMEBUFFER_BINDING = 0x8CA6,
                     GL_RENDERBUFFER_BINDING = 0x8CA7, GL_READ_FRAMEBUFFER_BINDING = 0x8CAA, GL_COLOR_BUFFER_BIT = 0x4000,
                     GL_MAX_SAMPLES = 0x8D57;
    constexpr GLenum GL_ARRAY_BUFFER = 0x8892, GL_ELEMENT_ARRAY_BUFFER = 0x8893, GL_UNIFORM_BUFFER = 0x8A11, GL_STREAM_DRAW = 0x88E0,
                     GL_DYNAMIC_DRAW = 0x88E8, GL_ARRAY_BUFFER_BINDING = 0x8894, GL_UNIFORM_BUFFER_BINDING = 0x8A28,
                     GL_UNIFORM_BUFFER_START = 0x8A29, GL_UNIFORM_BUFFER_SIZE = 0x8A2A, GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT = 0x8A34,
                     GL_VERTEX_ARRAY_BINDING = 0x85B5, GL_CURRENT_PROGRAM = 0x8B8D;
    constexpr GLenum GL_FRAGMENT_SHADER = 0x8B30, GL_VERTEX_SHADER = 0x8B31, GL_COMPILE_STATUS = 0x8B81, GL_LINK_STATUS = 0x8B82,
                     GL_INFO_LOG_LENGTH = 0x8B84;
    constexpr GLuint GL_INVALID_INDEX = 0xFFFFFFFFu;
    constexpr GLenum GL_MAJOR_VERSION = 0x821B, GL_MINOR_VERSION = 0x821C, GL_COPY_WRITE_BUFFER = 0x8F37, GL_COLOR = 0x1800;
    constexpr GLenum GL_VENDOR = 0x1F00, GL_RENDERER = 0x1F01, GL_VERSION = 0x1F02, GL_EXTENSIONS = 0x1F03, GL_NUM_EXTENSIONS = 0x821D,
                     GL_MAX_TEXTURE_SIZE = 0x0D33, GL_CONTEXT_FLAGS = 0x821E, GL_CONTEXT_FLAG_DEBUG_BIT = 0x0002;
    constexpr GLenum GL_TIMESTAMP = 0x8E28, GL_QUERY_RESULT = 0x8866, GL_QUERY_RESULT_AVAILABLE = 0x8867, GL_GPU_DISJOINT_EXT = 0x8FBB;
    constexpr GLenum GL_DEBUG_OUTPUT_SYNCHRONOUS = 0x8242, GL_DEBUG_TYPE_ERROR = 0x824C, GL_DEBUG_TYPE_OTHER = 0x8251,
                     GL_DEBUG_SEVERITY_NOTIFICATION = 0x826B, GL_DEBUG_SEVERITY_HIGH = 0x9146, GL_DEBUG_SEVERITY_MEDIUM = 0x9147,
                     GL_DEBUG_SEVERITY_LOW = 0x9148, GL_DEBUG_OUTPUT = 0x92E0, GL_DONT_CARE = 0x1100, GL_TEXTURE = 0x1702,
                     GL_BUFFER = 0x82E0, GL_PROGRAM = 0x82E2;

    // ------------------------------------------------------------------ entry points
    // X(return type, name without the gl prefix, parameter list)
#define ESIA_GL_CORE(X)                                                                                                         \
    X(GLenum, GetError, ())                                                                                                     \
    X(void, GetIntegerv, (GLenum pname, GLint * data))                                                                          \
    X(void, GetIntegeri_v, (GLenum target, GLuint index, GLint * data))                                                         \
    X(void, GetInteger64i_v, (GLenum target, GLuint index, GLint64 * data))                                                     \
    X(void, GetBooleanv, (GLenum pname, GLboolean * data))                                                                      \
    X(void, GetFloatv, (GLenum pname, GLfloat * data))                                                                          \
    X(const GLubyte*, GetString, (GLenum name))                                                                                 \
    X(const GLubyte*, GetStringi, (GLenum name, GLuint index))                                                                  \
    X(GLboolean, IsEnabled, (GLenum cap))                                                                                       \
    X(void, Enable, (GLenum cap))                                                                                               \
    X(void, Disable, (GLenum cap))                                                                                              \
    X(void, Flush, ())                                                                                                          \
    X(void, Finish, ())                                                                                                         \
    X(void, Viewport, (GLint x, GLint y, GLsizei w, GLsizei h))                                                                 \
    X(void, Scissor, (GLint x, GLint y, GLsizei w, GLsizei h))                                                                  \
    X(void, ClearColor, (GLfloat r, GLfloat g, GLfloat b, GLfloat a))                                                           \
    X(void, Clear, (GLbitfield mask))                                                                                           \
    X(void, ColorMask, (GLboolean r, GLboolean g, GLboolean b, GLboolean a))                                                    \
    X(void, BlendFuncSeparate, (GLenum srcRgb, GLenum dstRgb, GLenum srcAlpha, GLenum dstAlpha))                                \
    X(void, BlendEquationSeparate, (GLenum rgb, GLenum alpha))                                                                  \
    X(void, PixelStorei, (GLenum pname, GLint param))                                                                           \
    X(void, ReadPixels, (GLint x, GLint y, GLsizei w, GLsizei h, GLenum format, GLenum type, void* data))                       \
    X(void, GenTextures, (GLsizei n, GLuint * textures))                                                                        \
    X(void, DeleteTextures, (GLsizei n, const GLuint* textures))                                                                \
    X(void, BindTexture, (GLenum target, GLuint texture))                                                                       \
    X(void, ActiveTexture, (GLenum unit))                                                                                       \
    X(void, TexImage2D, (GLenum target, GLint level, GLint internalFormat, GLsizei w, GLsizei h, GLint border, GLenum format,    \
                         GLenum type, const void* data))                                                                        \
    X(void, TexSubImage2D, (GLenum target, GLint level, GLint x, GLint y, GLsizei w, GLsizei h, GLenum format, GLenum type,      \
                            const void* data))                                                                                  \
    X(void, TexParameteri, (GLenum target, GLenum pname, GLint param))                                                          \
    X(void, GenSamplers, (GLsizei n, GLuint * samplers))                                                                        \
    X(void, DeleteSamplers, (GLsizei n, const GLuint* samplers))                                                                \
    X(void, BindSampler, (GLuint unit, GLuint sampler))                                                                         \
    X(void, SamplerParameteri, (GLuint sampler, GLenum pname, GLint param))                                                     \
    X(void, GenFramebuffers, (GLsizei n, GLuint * framebuffers))                                                                \
    X(void, DeleteFramebuffers, (GLsizei n, const GLuint* framebuffers))                                                        \
    X(void, BindFramebuffer, (GLenum target, GLuint framebuffer))                                                               \
    X(void, FramebufferTexture2D, (GLenum target, GLenum attachment, GLenum texTarget, GLuint texture, GLint level))            \
    X(void, FramebufferRenderbuffer, (GLenum target, GLenum attachment, GLenum rbTarget, GLuint renderbuffer))                  \
    X(GLenum, CheckFramebufferStatus, (GLenum target))                                                                          \
    X(void, BlitFramebuffer, (GLint sx0, GLint sy0, GLint sx1, GLint sy1, GLint dx0, GLint dy0, GLint dx1, GLint dy1,           \
                              GLbitfield mask, GLenum filter))                                                                  \
    X(void, GenRenderbuffers, (GLsizei n, GLuint * renderbuffers))                                                              \
    X(void, DeleteRenderbuffers, (GLsizei n, const GLuint* renderbuffers))                                                      \
    X(void, BindRenderbuffer, (GLenum target, GLuint renderbuffer))                                                             \
    X(void, RenderbufferStorageMultisample, (GLenum target, GLsizei samples, GLenum format, GLsizei w, GLsizei h))              \
    X(void, GenBuffers, (GLsizei n, GLuint * buffers))                                                                          \
    X(void, DeleteBuffers, (GLsizei n, const GLuint* buffers))                                                                  \
    X(void, BindBuffer, (GLenum target, GLuint buffer))                                                                         \
    X(void, BufferData, (GLenum target, GLsizeiptr size, const void* data, GLenum usage))                                       \
    X(void, BufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, const void* data))                                 \
    X(void, BindBufferBase, (GLenum target, GLuint index, GLuint buffer))                                                       \
    X(void, BindBufferRange, (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size))                    \
    X(void, GenVertexArrays, (GLsizei n, GLuint * arrays))                                                                      \
    X(void, DeleteVertexArrays, (GLsizei n, const GLuint* arrays))                                                              \
    X(void, BindVertexArray, (GLuint array))                                                                                    \
    X(void, EnableVertexAttribArray, (GLuint index))                                                                            \
    X(void, VertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,                  \
                                  const void* offset))                                                                          \
    X(GLuint, CreateShader, (GLenum type))                                                                                      \
    X(void, ShaderSource, (GLuint shader, GLsizei count, const GLchar* const* strings, const GLint* lengths))                   \
    X(void, CompileShader, (GLuint shader))                                                                                     \
    X(void, GetShaderiv, (GLuint shader, GLenum pname, GLint * value))                                                          \
    X(void, GetShaderInfoLog, (GLuint shader, GLsizei size, GLsizei * length, GLchar * log))                                    \
    X(void, DeleteShader, (GLuint shader))                                                                                      \
    X(GLuint, CreateProgram, ())                                                                                                \
    X(void, AttachShader, (GLuint program, GLuint shader))                                                                      \
    X(void, DetachShader, (GLuint program, GLuint shader))                                                                      \
    X(void, LinkProgram, (GLuint program))                                                                                      \
    X(void, GetProgramiv, (GLuint program, GLenum pname, GLint * value))                                                        \
    X(void, GetProgramInfoLog, (GLuint program, GLsizei size, GLsizei * length, GLchar * log))                                  \
    X(void, DeleteProgram, (GLuint program))                                                                                    \
    X(void, UseProgram, (GLuint program))                                                                                       \
    X(GLint, GetUniformLocation, (GLuint program, const GLchar* name))                                                          \
    X(void, Uniform1i, (GLint location, GLint value))                                                                           \
    X(void, Uniform1f, (GLint location, GLfloat value))                                                                         \
    X(void, Uniform4f, (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w))                                            \
    X(GLuint, GetUniformBlockIndex, (GLuint program, const GLchar* name))                                                       \
    X(void, UniformBlockBinding, (GLuint program, GLuint block, GLuint binding))                                                \
    X(void, DrawArrays, (GLenum mode, GLint first, GLsizei count))                                                              \
    X(void, DrawElements, (GLenum mode, GLsizei count, GLenum type, const void* offset))                                        \
    X(void, DrawArraysInstanced, (GLenum mode, GLint first, GLsizei count, GLsizei instances))                                  \
    X(void, GenQueries, (GLsizei n, GLuint * ids))                                                                              \
    X(void, DeleteQueries, (GLsizei n, const GLuint* ids))                                                                      \
    X(void, GetQueryObjectuiv, (GLuint id, GLenum pname, GLuint * value))

    // May be null. GL 4.3 / ES 3.0, GL 3.3 / EXT_disjoint_timer_query, KHR_debug, desktop GL only (PolygonMode).
#define ESIA_GL_OPTIONAL(X)                                                                                                     \
    X(void, InvalidateFramebuffer, (GLenum target, GLsizei count, const GLenum* attachments))                                   \
    X(void, QueryCounter, (GLuint id, GLenum target))                                                                           \
    X(void, GetQueryObjectui64v, (GLuint id, GLenum pname, GLuint64 * value))                                                   \
    X(void, DebugMessageCallback, (GLDEBUGPROC callback, const void* user))                                                     \
    X(void, DebugMessageControl, (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids,                 \
                                  GLboolean enabled))                                                                           \
    X(void, ObjectLabel, (GLenum identifier, GLuint name, GLsizei length, const GLchar* label))                                 \
    X(void, PolygonMode, (GLenum face, GLenum mode))

    struct GlApi
    {
#define ESIA_GL_MEMBER(ret, name, params) ret(ESIA_GLAPI* name) params = nullptr;
        ESIA_GL_CORE(ESIA_GL_MEMBER)
        ESIA_GL_OPTIONAL(ESIA_GL_MEMBER)
#undef ESIA_GL_MEMBER

        // Resolves ESIA_GL_CORE with `getProc` (the platform default when null). False, with the first missing
        // function in `missing`, when the context lacks one.
        bool Load(void* (*getProc)(const char* name), const char*& missing);
        // One optional entry point by its full name ("glQueryCounterEXT"); stays null when unknown.
        template <class F>
        void LoadOptional(F& f, const char* name)
        {
            f = reinterpret_cast<F>(getProc(name));
        }

        void* (*getProc)(const char* name) = nullptr;
    };

    // The platform's own lookup for the current context (eglGetProcAddress, glXGetProcAddressARB, wglGetProcAddress +
    // opengl32.dll, the process image on macOS); null when this platform has none.
    void* DefaultGetProcAddress(const char* name);
}
