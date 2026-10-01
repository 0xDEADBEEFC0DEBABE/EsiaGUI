// Esia - RHI helpers and the backend registry (see esia/rhi/rhi.hpp, esia/rhi/backend_registry.hpp)
#include "esia/rhi/backend_registry.hpp"
#include <cstring>

namespace esia::rhi
{
    bool IsSrgb(Format f) { return f == Format::RGBA8_SRGB || f == Format::BGRA8_SRGB; }

    Format RawFormat(Format f)
    {
        switch (f)
        {
        case Format::RGBA8_SRGB: return Format::RGBA8_UNORM;
        case Format::BGRA8_SRGB: return Format::BGRA8_UNORM;
        default: return f;
        }
    }

    int BytesPerPixel(Format f)
    {
        switch (f)
        {
        case Format::R8_UNORM: return 1;
        case Format::RGBA16_FLOAT: return 8;
        case Format::RGBA32_FLOAT: return 16;
        case Format::Unknown: return 0;
        default: return 4;
        }
    }

    const char* FormatName(Format f)
    {
        switch (f)
        {
        case Format::Unknown: return "Unknown";
        case Format::RGBA8_UNORM: return "RGBA8_UNORM";
        case Format::RGBA8_SRGB: return "RGBA8_SRGB";
        case Format::BGRA8_UNORM: return "BGRA8_UNORM";
        case Format::BGRA8_SRGB: return "BGRA8_SRGB";
        case Format::RGB10A2_UNORM: return "RGB10A2_UNORM";
        case Format::RGBA16_FLOAT: return "RGBA16_FLOAT";
        case Format::RGBA32_FLOAT: return "RGBA32_FLOAT";
        case Format::R8_UNORM: return "R8_UNORM";
        }
        return "?";
    }

    const char* ShaderProgramName(ShaderProgram p)
    {
        switch (p)
        {
        case ShaderProgram::UiGeometry: return "UiGeometry";
        case ShaderProgram::TextGray: return "TextGray";
        case ShaderProgram::Fx: return "Fx";
        case ShaderProgram::Downsample: return "Downsample";
        case ShaderProgram::LayerComposite: return "LayerComposite";
        case ShaderProgram::Clear: return "Clear";
        case ShaderProgram::Count: break;
        }
        return "?";
    }

    namespace
    {
        std::vector<BackendInfo>& Registry()
        {
            static std::vector<BackendInfo> r;
            return r;
        }
    }

    void RegisterBackend(const BackendInfo& info)
    {
        for (const BackendInfo& b : Registry())
            if (std::strcmp(b.name, info.name) == 0)
                return;
        Registry().push_back(info);
    }

    const std::vector<BackendInfo>& Backends() { return Registry(); }

    const BackendInfo* FindBackend(const std::string& name)
    {
        for (const BackendInfo& b : Registry())
            if (name == b.name)
                return &b;
        return nullptr;
    }
}
