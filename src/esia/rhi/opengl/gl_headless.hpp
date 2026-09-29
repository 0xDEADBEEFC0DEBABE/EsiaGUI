// Esia OpenGL backend - headless devices on an EGL context of their own (private; the backends "opengl" and "gles"
// and the backend's tests use it).
#pragma once
#include "esia/rhi/backend_registry.hpp"
#include "esia/rhi/opengl.hpp"

namespace esia::rhi::opengl
{
    // A device on a new EGL context of `desc.es`'s API (its getProcAddress is EGL's), with a render target of
    // `target`'s size, format and samples. Empty, with the reason in `error`, when this machine cannot run it.
    HeadlessDevice CreateHeadlessDevice(Desc desc, const HeadlessDesc& target, std::string& error);

    // eglGetProcAddress (null without EGL): GL functions of the current headless context.
    using GetProcAddressFn = void* (*)(const char* name);
    GetProcAddressFn HeadlessGetProcAddress();
}
