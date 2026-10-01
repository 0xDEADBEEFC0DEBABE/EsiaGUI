// glass_window - the hosts built into this executable (CMake defines GLASS_<API> for each backend it links).
#include "host.hpp"

namespace glass
{
    namespace
    {
        struct Api
        {
            const char* name;
            std::unique_ptr<Host> (*create)();
        };

        constexpr Api kApis[] = {
#if defined(GLASS_D3D11)
            {"d3d11", &CreateHostD3D11},
#endif
#if defined(GLASS_D3D12)
            {"d3d12", &CreateHostD3D12},
#endif
#if defined(GLASS_D3D10)
            {"d3d10", &CreateHostD3D10},
#endif
#if defined(GLASS_D3D9)
            {"d3d9", &CreateHostD3D9},
#endif
#if defined(GLASS_OPENGL)
            {"opengl", &CreateHostOpenGL},
#endif
#if defined(GLASS_VULKAN)
            {"vulkan", &CreateHostVulkan},
#endif
        };
    }

    std::vector<std::string> DirectXApis()
    {
        std::vector<std::string> v;
        for (const Api& a : kApis)   // listed in the order "directx" tries them
            if (std::string(a.name).starts_with("d3d"))
                v.push_back(a.name);
        return v;
    }

    std::unique_ptr<Host> CreateHost(const std::string& api)
    {
        if (api == "directx")   // the first version, until Init says otherwise (RunApp tries the next ones)
        {
            const std::vector<std::string> versions = DirectXApis();
            return versions.empty() ? nullptr : CreateHost(versions[0]);
        }
        for (const Api& a : kApis)
            if (api == a.name)
                return a.create();
        return nullptr;
    }

    const char* DefaultApi() { return DirectXApis().empty() ? kApis[0].name : "directx"; }

    std::string BuiltApis()
    {
        std::string s = DirectXApis().empty() ? "" : "directx";
        for (const Api& a : kApis)
            s += (s.empty() ? "" : ", ") + std::string(a.name);
        return s;
    }
}
