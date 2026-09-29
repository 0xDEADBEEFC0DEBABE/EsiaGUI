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

    std::unique_ptr<Host> CreateHost(const std::string& api)
    {
        for (const Api& a : kApis)
            if (api == a.name)
                return a.create();
        return nullptr;
    }

    const char* DefaultApi() { return kApis[0].name; }

    std::string BuiltApis()
    {
        std::string s;
        for (const Api& a : kApis)
            s += (s.empty() ? "" : ", ") + std::string(a.name);
        return s;
    }
}
