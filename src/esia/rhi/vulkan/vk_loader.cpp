// Esia - Vulkan backend: the function table (see vk_loader.hpp)
#include "vk_loader.hpp"
#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace esia::rhi::vulkan
{
    bool Functions::LoadGlobal(PFN_vkGetInstanceProcAddr getInstanceProcAddr, std::string& error)
    {
        vkGetInstanceProcAddr = getInstanceProcAddr;
#define ESIA_VK_LOAD(name) name = reinterpret_cast<PFN_##name>(vkGetInstanceProcAddr(VK_NULL_HANDLE, #name));
        ESIA_VK_GLOBAL_FUNCTIONS(ESIA_VK_LOAD)
#undef ESIA_VK_LOAD
        if (!vkCreateInstance || !vkEnumerateInstanceLayerProperties || !vkEnumerateInstanceExtensionProperties)
        {
            error = "the Vulkan loader exports no vkCreateInstance";
            return false;
        }
        // vkEnumerateInstanceVersion is null on a 1.0 loader: the backend needs 1.1 (checked by the caller)
        return true;
    }

    bool Functions::LoadInstance(VkInstance instance, std::string& error)
    {
#define ESIA_VK_LOAD(name)                                                                \
    name = reinterpret_cast<PFN_##name>(vkGetInstanceProcAddr(instance, #name));          \
    if (!name)                                                                            \
    {                                                                                     \
        error = "missing instance function " #name;                                       \
        return false;                                                                     \
    }
        ESIA_VK_INSTANCE_FUNCTIONS(ESIA_VK_LOAD)
#undef ESIA_VK_LOAD
#define ESIA_VK_LOAD_OPTIONAL(name) name = reinterpret_cast<PFN_##name>(vkGetInstanceProcAddr(instance, #name));
        ESIA_VK_DEBUG_FUNCTIONS(ESIA_VK_LOAD_OPTIONAL)
#undef ESIA_VK_LOAD_OPTIONAL
        return true;
    }

    bool Functions::LoadDevice(VkDevice device, bool dynamicRendering, bool core13, std::string& error)
    {
#define ESIA_VK_LOAD(name)                                                                \
    name = reinterpret_cast<PFN_##name>(vkGetDeviceProcAddr(device, #name));              \
    if (!name)                                                                            \
    {                                                                                     \
        error = "missing device function " #name;                                         \
        return false;                                                                     \
    }
        ESIA_VK_DEVICE_FUNCTIONS(ESIA_VK_LOAD)
#undef ESIA_VK_LOAD
        vkCmdBeginRendering = nullptr;
        vkCmdEndRendering = nullptr;
        if (dynamicRendering)
        {
            // core names on 1.3, the extension's otherwise (same signatures)
            vkCmdBeginRendering = reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(
                vkGetDeviceProcAddr(device, core13 ? "vkCmdBeginRendering" : "vkCmdBeginRenderingKHR"));
            vkCmdEndRendering = reinterpret_cast<PFN_vkCmdEndRenderingKHR>(
                vkGetDeviceProcAddr(device, core13 ? "vkCmdEndRendering" : "vkCmdEndRenderingKHR"));
            if (!vkCmdBeginRendering || !vkCmdEndRendering)
            {
                error = "dynamic rendering enabled but vkCmdBeginRendering is missing";
                return false;
            }
        }
        return true;
    }

    LoaderLibrary OpenLoader(std::string& error)
    {
        LoaderLibrary lib;
#if defined(_WIN32)
        HMODULE m = LoadLibraryA("vulkan-1.dll");
        if (m)
        {
            lib.handle = m;
            lib.getInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(reinterpret_cast<void*>(GetProcAddress(m, "vkGetInstanceProcAddr")));
        }
#else
#if defined(__APPLE__)
        const char* names[] = {"libvulkan.1.dylib", "libvulkan.dylib", "libMoltenVK.dylib"};
#else
        const char* names[] = {"libvulkan.so.1", "libvulkan.so"};
#endif
        for (const char* n : names)
            if ((lib.handle = dlopen(n, RTLD_NOW | RTLD_LOCAL)) != nullptr)
                break;
        if (lib.handle)
            lib.getInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(lib.handle, "vkGetInstanceProcAddr"));
#endif
        if (!lib.getInstanceProcAddr)
        {
            error = lib.handle ? "the Vulkan loader has no vkGetInstanceProcAddr" : "no Vulkan loader installed";
            CloseLoader(lib);
        }
        return lib;
    }

    void CloseLoader(LoaderLibrary& lib)
    {
        if (lib.handle)
        {
#if defined(_WIN32)
            FreeLibrary(static_cast<HMODULE>(lib.handle));
#else
            dlclose(lib.handle);
#endif
        }
        lib = LoaderLibrary();
    }

    const char* ResultName(VkResult r)
    {
        switch (r)
        {
        case VK_SUCCESS: return "VK_SUCCESS";
        case VK_NOT_READY: return "VK_NOT_READY";
        case VK_TIMEOUT: return "VK_TIMEOUT";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
        default: return "VkResult error";
        }
    }
}
