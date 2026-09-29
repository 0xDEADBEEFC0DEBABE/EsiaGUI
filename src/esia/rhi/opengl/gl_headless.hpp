// Esia OpenGL backend - headless devices on a context of their own (WGL on Windows, else EGL; private: the backends
// "opengl" and "gles" and the backend's tests use it).
#pragma once
#include "esia/rhi/backend_registry.hpp"
#include "esia/rhi/opengl.hpp"

namespace esia::rhi::opengl
{
    // A device on a new context of `desc.es`'s API (its getProcAddress is that context's), with a render target of
    // `target`'s size, format and samples. Empty, with the reason in `error`, when this machine cannot run it.
    HeadlessDevice CreateHeadlessDevice(Desc desc, const HeadlessDesc& target, std::string& error);

    // GL functions of the current headless context (WGL's lookup when a WGL context is current, else eglGetProcAddress,
    // null without EGL).
    using GetProcAddressFn = void* (*)(const char* name);
    GetProcAddressFn HeadlessGetProcAddress();
}
