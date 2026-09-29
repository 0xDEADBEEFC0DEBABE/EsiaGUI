// Esia - the OpenGL 3.3 core / OpenGL ES 3.0 RHI backend: host integration (docs/backends/README.md, section 1).
//
// The device renders with the host's context, which must be current on the renderer's thread whenever the device
// is called (creation, every frame, destruction). It loads its GL functions itself (no GLEW / glad needed by the
// host) and needs no GL header here.
//
//   auto device = esia::rhi::opengl::CreateDevice({.es = false}, &error);              // context is current
//   esia::render::Renderer renderer(*device);
//   ...every frame:
//   rhi::Texture target = esia::rhi::opengl::WrapFramebuffer(*device, 0, w, h, rhi::Format::RGBA8_UNORM);
//   renderer.Render(drawData, &textures, target);                                        // FrameDesc::nativeContext: null
//
// Framebuffer origin: OpenGL's bottom-left. The backend converts every RHI rectangle and reports
// Caps::framebufferOriginBottomLeft; the host does nothing. Host callbacks (DrawCmdKind::Callback) get a null
// NativeRenderState(): they draw with the current context into the bound framebuffer, may change any state, and
// the backend binds its own again afterwards.
#pragma once
#include "esia/rhi/rhi.hpp"
#include <memory>
#include <string>

namespace esia::rhi::opengl
{
    struct Desc
    {
        // OpenGL ES 3.0+ (ESSL 300 shaders) instead of OpenGL 3.3+ core (GLSL 330).
        bool es = false;
        // Function lookup of the host's context (SDL_GL_GetProcAddress, glfwGetProcAddress, eglGetProcAddress ...);
        // null: the platform's (EGL / GLX, WGL + opengl32.dll, the process image on macOS).
        void* (*getProcAddress)(const char* name) = nullptr;
        // KHR_debug output: GL errors and warnings go to stderr and are counted (ErrorCount). Installs the device's
        // debug callback on the context. Default: on in debug builds.
#ifdef NDEBUG
        bool debug = false;
#else
        bool debug = true;
#endif
        // Save the GL state the device changes when a frame (or a resource call outside frames) starts and restore
        // it when it ends, so the host's own rendering is not disturbed. Off: the host resets what it relies on.
        bool restoreHostState = true;
        // Use only what the core version guarantees (GL 3.3 core / GLES 3.0), ignoring every extension and later
        // version the driver offers - tests the minimum feature set on drivers that have more.
        bool coreOnly = false;
    };

    // Creates the device on the current context. Null, with the reason in `error`, when the context is too old or
    // lacks a function.
    std::unique_ptr<Device> CreateDevice(const Desc& desc, std::string* error = nullptr);

    // A host framebuffer as an RHI render target (RenderTarget | CopySrc). `fbo` 0 is the default framebuffer.
    // `colorTexture` is the GL_TEXTURE_2D attached to it as color 0, if any: then the target also reports
    // TextureUsage_Sampled (frosted glass reads it without a copy) - for sRGB formats only where the device can
    // sample it raw (EXT_texture_sRGB_decode). `samples` > 1: a multisampled framebuffer (copies and readback
    // resolve it). BGRA8 formats are stored as RGBA in GL; they only tell the backend the target's encoding.
    // Wrapping the same (fbo, colorTexture) again returns the same handle with the new size and format, so wrap the
    // swap chain's framebuffer every frame. DestroyTexture releases the wrapper, never the host's objects.
    Texture WrapFramebuffer(Device& device, unsigned int fbo, int width, int height, Format format, unsigned int colorTexture = 0,
                            int samples = 1);

    // The GL texture object of a device texture (0 for a multisampled or wrapped window target): lets the host
    // sample what Esia rendered, e.g. an offscreen target.
    unsigned int NativeTexture(Device& device, Texture texture);

    // GL errors and KHR_debug error messages seen by this device so far (Desc::debug; 0 without it).
    std::uint32_t ErrorCount(const Device& device);
}
