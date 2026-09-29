// Esia - registry of the RHI backends built into this binary (the conformance suite and tools pick them by name).
//
// Every backend target registers itself with the CMake function esia_register_backend(<name> <target>); CMake
// then generates a translation unit that calls `void EsiaRegisterBackend_<name>()` of each one before the
// suite runs (no static-initializer tricks: static libraries would drop them). That function calls
// esia::rhi::RegisterBackend with a BackendInfo.
#pragma once
#include "esia/rhi/rhi.hpp"
#include <memory>
#include <string>
#include <vector>

namespace esia::rhi
{
    // An offscreen device for tests: no window, no swap chain.
    struct HeadlessDesc
    {
        int width = 256, height = 256;
        Format format = Format::RGBA8_UNORM;
        bool sampleable = true;   // the target is created with TextureUsage_Sampled (the direct-read capture path)
        int samples = 1;
    };

    struct HeadlessDevice
    {
        std::unique_ptr<Device> device;
        Texture target;   // render target of HeadlessDesc's size / format, owned by the device
    };

    struct BackendInfo
    {
        const char* name = nullptr;   // "null", "opengl", "gles", "vulkan", "metal", "d3d9", "d3d10", "d3d11", "d3d12"
        // Creates a headless device; returns an empty device (and says why in `error`) when this machine cannot
        // run the backend (no driver, no GPU): the suite then skips it instead of failing.
        HeadlessDevice (*createHeadless)(const HeadlessDesc& desc, std::string& error) = nullptr;
    };

    ESIA_API void RegisterBackend(const BackendInfo& info);
    ESIA_API const std::vector<BackendInfo>& Backends();
    ESIA_API const BackendInfo* FindBackend(const std::string& name);
    // Calls the generated EsiaRegisterBackend_<name>() of every backend linked into this binary (once).
    ESIA_API void RegisterBuiltinBackends();
}
