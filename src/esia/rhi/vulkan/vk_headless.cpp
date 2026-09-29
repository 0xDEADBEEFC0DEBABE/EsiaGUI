// Esia - Vulkan backend: device creation (from the host's objects, or headless with an instance of its own), the
// host integration functions (esia/rhi/vulkan.hpp) and the backend's registration.
#include "vk_device.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace esia::rhi::vulkan
{
    namespace
    {
        VulkanDevice* AsVulkan(Device& dev) { return dynamic_cast<VulkanDevice*>(&dev); }

        bool Has(const std::vector<VkExtensionProperties>& list, const char* name)
        {
            for (const VkExtensionProperties& e : list)
                if (std::strcmp(e.extensionName, name) == 0)
                    return true;
            return false;
        }

        bool EnvIs(const char* name, const char* value)
        {
            const char* v = std::getenv(name);
            return v && std::strcmp(v, value) == 0;
        }

        VKAPI_ATTR VkBool32 VKAPI_CALL OnValidationMessage(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT types,
                                                           const VkDebugUtilsMessengerCallbackDataEXT* data, void* user)
        {
            // The loader reports through the same messenger with GENERAL messages: the layer manifests of other
            // software on the machine (overlays of recorders and launchers: missing or duplicate JSON files), not this
            // device's use of the API. Shown, not counted.
            if (!(types & (VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)))
            {
                std::fprintf(stderr, "esia vulkan [loader] %s\n", data && data->pMessage ? data->pMessage : "");
                return VK_FALSE;
            }
            auto* log = static_cast<ValidationLog*>(user);
            ++log->messages;
            std::fprintf(stderr, "esia vulkan [%s] %s\n", severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ? "validation error" : "validation warning",
                         data && data->pMessage ? data->pMessage : "");
            return VK_FALSE;
        }

        // Scores a physical device for headless use: a real GPU before a software one, Vulkan 1.1 and a graphics queue
        // required.
        int Score(const Functions& vk, VkPhysicalDevice pd, std::uint32_t& family)
        {
            VkPhysicalDeviceProperties p;
            vk.vkGetPhysicalDeviceProperties(pd, &p);
            if (p.apiVersion < VK_API_VERSION_1_1)
                return -1;
            std::uint32_t n = 0;
            vk.vkGetPhysicalDeviceQueueFamilyProperties(pd, &n, nullptr);
            std::vector<VkQueueFamilyProperties> families(n);
            vk.vkGetPhysicalDeviceQueueFamilyProperties(pd, &n, families.data());
            family = UINT32_MAX;
            for (std::uint32_t i = 0; i < n && family == UINT32_MAX; ++i)
                if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    family = i;
            if (family == UINT32_MAX)
                return -1;
            switch (p.deviceType)
            {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return 4;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return 3;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return 2;
            case VK_PHYSICAL_DEVICE_TYPE_CPU: return 1;
            default: return 0;
            }
        }
    }

    std::unique_ptr<Device> CreateDevice(const Desc& desc, std::string* error)
    {
        std::string e;
        Functions vk;
        Ownership own;
        PFN_vkGetInstanceProcAddr gipa = desc.getInstanceProcAddr;
        if (!gipa)
        {
            own.loader = OpenLoader(e);
            gipa = own.loader.getInstanceProcAddr;
        }
        std::unique_ptr<VulkanDevice> dev;
        if (gipa && vk.LoadGlobal(gipa, e) && vk.LoadInstance(desc.instance, e))
            dev = VulkanDevice::Create(desc, vk, std::move(own), e);
        else
            CloseLoader(own.loader);
        if (!dev && error)
            *error = e;
        return dev;
    }

    Texture WrapImage(Device& dev, VkImage image, VkImageView view, VkFormat format, int width, int height, int samples, VkImageLayout layoutOnEntry,
                      VkImageLayout layoutOnExit, std::uint32_t usage)
    {
        VulkanDevice* v = AsVulkan(dev);
        return v ? v->Wrap(image, view, format, width, height, samples, layoutOnEntry, layoutOnExit, usage) : Texture{};
    }

    bool UsesDynamicRendering(const Device& dev)
    {
        const auto* v = dynamic_cast<const VulkanDevice*>(&dev);
        return v && v->DynamicRendering();
    }

    VkRenderPass RenderPassFor(Device& dev, Format format, int samples)
    {
        VulkanDevice* v = AsVulkan(dev);
        if (!v || v->DynamicRendering())
            return VK_NULL_HANDLE;
        return v->RenderPass(ToVkFormat(format), samples, LoadOp::Load);
    }

    std::uint32_t ValidationMessages(const Device& dev)
    {
        const auto* v = dynamic_cast<const VulkanDevice*>(&dev);
        return v ? v->ValidationMessageCount() : 0u;
    }

    HeadlessDevice CreateHeadless(const HeadlessDesc& hd, const HeadlessOptions& options, std::string& error)
    {
        Functions vk;
        Ownership own;
        own.loader = OpenLoader(error);
        if (!own.loader.getInstanceProcAddr || !vk.LoadGlobal(own.loader.getInstanceProcAddr, error))
        {
            CloseLoader(own.loader);
            return {};
        }
        std::uint32_t instanceVersion = VK_API_VERSION_1_0;
        if (vk.vkEnumerateInstanceVersion)
            vk.vkEnumerateInstanceVersion(&instanceVersion);
        if (instanceVersion < VK_API_VERSION_1_1)
        {
            error = "the Vulkan loader is older than 1.1";
            CloseLoader(own.loader);
            return {};
        }

        // ---- instance: the validation layer (with synchronization validation) when installed and wanted
        std::uint32_t n = 0;
        vk.vkEnumerateInstanceExtensionProperties(nullptr, &n, nullptr);
        std::vector<VkExtensionProperties> instanceExts(n);
        vk.vkEnumerateInstanceExtensionProperties(nullptr, &n, instanceExts.data());
        const char* kValidation = "VK_LAYER_KHRONOS_validation";
        bool validation = false;
        if (options.validation)
        {
            vk.vkEnumerateInstanceLayerProperties(&n, nullptr);
            std::vector<VkLayerProperties> layers(n);
            vk.vkEnumerateInstanceLayerProperties(&n, layers.data());
            for (const VkLayerProperties& l : layers)
                validation |= std::strcmp(l.layerName, kValidation) == 0;
            if (validation)
            {
                vk.vkEnumerateInstanceExtensionProperties(kValidation, &n, nullptr);
                std::vector<VkExtensionProperties> layerExts(n);
                vk.vkEnumerateInstanceExtensionProperties(kValidation, &n, layerExts.data());
                instanceExts.insert(instanceExts.end(), layerExts.begin(), layerExts.end());
            }
        }
        std::vector<const char*> exts;
        const bool debugUtils = Has(instanceExts, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        if (debugUtils)
            exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        const bool features = validation && Has(instanceExts, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
        if (features)
            exts.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
        const bool portability = Has(instanceExts, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);   // MoltenVK
        if (portability)
            exts.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

        own.log = std::make_unique<ValidationLog>();
        VkDebugUtilsMessengerCreateInfoEXT mi{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
        mi.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        mi.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        mi.pfnUserCallback = &OnValidationMessage;
        mi.pUserData = own.log.get();
        const VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};
        VkValidationFeaturesEXT vf{VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT};
        vf.enabledValidationFeatureCount = 1;
        vf.pEnabledValidationFeatures = enables;

        VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        app.pApplicationName = "esia-headless";
        app.pEngineName = "esia";
        app.apiVersion = std::min<std::uint32_t>(instanceVersion, std::clamp<std::uint32_t>(options.maxApiVersion, VK_API_VERSION_1_1, VK_API_VERSION_1_3));
        VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        ici.pApplicationInfo = &app;
        ici.flags = portability ? VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR : 0;
        ici.enabledExtensionCount = (std::uint32_t)exts.size();
        ici.ppEnabledExtensionNames = exts.data();
        ici.enabledLayerCount = validation ? 1u : 0u;
        ici.ppEnabledLayerNames = &kValidation;
        // the messenger also sees what vkCreateInstance / vkDestroyInstance report
        if (validation && debugUtils)
        {
            ici.pNext = &mi;
            if (features)
                mi.pNext = &vf;
        }
        Desc desc;
        desc.getInstanceProcAddr = own.loader.getInstanceProcAddr;
        const VkResult ir = vk.vkCreateInstance(&ici, nullptr, &desc.instance);
        if (ir != VK_SUCCESS)
        {
            error = std::string("vkCreateInstance: ") + ResultName(ir);
            CloseLoader(own.loader);
            return {};
        }
        own.instance = true;
        mi.pNext = nullptr;
        if (!vk.LoadInstance(desc.instance, error))
        {
            vk.vkDestroyInstance(desc.instance, nullptr);
            CloseLoader(own.loader);
            return {};
        }
        if (validation && debugUtils && vk.vkCreateDebugUtilsMessengerEXT)
            vk.vkCreateDebugUtilsMessengerEXT(desc.instance, &mi, nullptr, &own.messenger);
        desc.debugUtils = debugUtils;

        // until VulkanDevice::Create takes ownership, failures release by hand
        auto fail = [&](const std::string& why) {
            error = why;
            if (own.messenger)
                vk.vkDestroyDebugUtilsMessengerEXT(desc.instance, own.messenger, nullptr);
            if (own.device)
                vk.vkDestroyDevice(desc.device, nullptr);
            vk.vkDestroyInstance(desc.instance, nullptr);
            CloseLoader(own.loader);
            return HeadlessDevice{};
        };

        // ---- physical device
        vk.vkEnumeratePhysicalDevices(desc.instance, &n, nullptr);
        std::vector<VkPhysicalDevice> gpus(n);
        vk.vkEnumeratePhysicalDevices(desc.instance, &n, gpus.data());
        int best = -1;
        for (VkPhysicalDevice pd : gpus)
        {
            std::uint32_t family;
            const int s = Score(vk, pd, family);
            if (s > best)
            {
                best = s;
                desc.physicalDevice = pd;
                desc.queueFamily = family;
            }
        }
        if (best < 0)
            return fail("no Vulkan 1.1 device with a graphics queue");
        VkPhysicalDeviceProperties props;
        vk.vkGetPhysicalDeviceProperties(desc.physicalDevice, &props);
        desc.apiVersion = std::min<std::uint32_t>(props.apiVersion, app.apiVersion);

        // ---- device: dual-source blending and dynamic rendering when there
        vk.vkEnumerateDeviceExtensionProperties(desc.physicalDevice, nullptr, &n, nullptr);
        std::vector<VkExtensionProperties> deviceExts(n);
        vk.vkEnumerateDeviceExtensionProperties(desc.physicalDevice, nullptr, &n, deviceExts.data());
        std::vector<const char*> dexts;
        const bool core13 = desc.apiVersion >= VK_API_VERSION_1_3;
        bool dynamicRendering = options.dynamicRendering && (core13 || Has(deviceExts, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME));
        if (dynamicRendering && !core13)
        {
            dexts.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
            if (desc.apiVersion < VK_API_VERSION_1_2)   // its dependencies, core in 1.2
            {
                for (const char* dep : {VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME})
                    if (Has(deviceExts, dep))
                        dexts.push_back(dep);
                    else
                        dynamicRendering = false;
                if (!dynamicRendering)
                    dexts.clear();
            }
        }
        if (Has(deviceExts, "VK_KHR_portability_subset"))   // must be enabled where it exists (MoltenVK)
            dexts.push_back("VK_KHR_portability_subset");

        VkPhysicalDeviceDynamicRenderingFeaturesKHR dr{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR};
        VkPhysicalDeviceFeatures2 supported{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        supported.pNext = dynamicRendering ? &dr : nullptr;
        vk.vkGetPhysicalDeviceFeatures2(desc.physicalDevice, &supported);
        dynamicRendering = dynamicRendering && dr.dynamicRendering;
        if (!dynamicRendering)
            std::erase_if(dexts, [](const char* e) { return std::strcmp(e, "VK_KHR_portability_subset") != 0; });

        VkPhysicalDeviceFeatures2 enabled{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        VkPhysicalDeviceDynamicRenderingFeaturesKHR drOn{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR};
        drOn.dynamicRendering = VK_TRUE;
        enabled.pNext = dynamicRendering ? &drOn : nullptr;
        const float priority = 1.0f;
        VkDeviceQueueCreateInfo qi{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        qi.queueFamilyIndex = desc.queueFamily;
        qi.queueCount = 1;
        qi.pQueuePriorities = &priority;
        VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        dci.pNext = &enabled;
        dci.queueCreateInfoCount = 1;
        dci.pQueueCreateInfos = &qi;
        dci.enabledExtensionCount = (std::uint32_t)dexts.size();
        dci.ppEnabledExtensionNames = dexts.data();
        const VkResult dr2 = vk.vkCreateDevice(desc.physicalDevice, &dci, nullptr, &desc.device);
        if (dr2 != VK_SUCCESS)
            return fail(std::string("vkCreateDevice: ") + ResultName(dr2));
        own.device = true;
        auto getQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(vk.vkGetDeviceProcAddr(desc.device, "vkGetDeviceQueue"));
        getQueue(desc.device, desc.queueFamily, 0, &desc.queue);
        desc.dynamicRendering = dynamicRendering;
        desc.framesInFlight = options.framesInFlight;

        // the device owns everything from here on: a failed Create released it all already
        std::unique_ptr<VulkanDevice> dev = VulkanDevice::Create(desc, vk, std::move(own), error);
        if (!dev)
            return {};

        TextureDesc td;
        td.width = hd.width;
        td.height = hd.height;
        td.format = hd.format;
        td.samples = hd.samples;
        td.usage = TextureUsage_RenderTarget | TextureUsage_CopySrc | (hd.sampleable ? TextureUsage_Sampled : 0u);
        td.debugName = "headless-target";
        HeadlessDevice h;
        h.target = dev->CreateTexture(td, nullptr, 0);
        if (!h.target)
        {
            error = std::string("the device cannot render to ") + FormatName(hd.format) + (hd.samples > 1 ? " with " + std::to_string(hd.samples) + " samples" : "");
            return {};
        }
        h.device = std::move(dev);
        return h;
    }

    namespace
    {
        HeadlessDevice CreateRegisteredHeadless(const HeadlessDesc& desc, std::string& error)
        {
            HeadlessOptions o;
            o.validation = !EnvIs("ESIA_VULKAN_VALIDATION", "0");
            o.dynamicRendering = !EnvIs("ESIA_VULKAN_RENDER_PASS", "1");
            return CreateHeadless(desc, o, error);
        }
    }
}

void EsiaRegisterBackend_vulkan()
{
    esia::rhi::BackendInfo info;
    info.name = "vulkan";
    info.createHeadless = &esia::rhi::vulkan::CreateRegisteredHeadless;
    esia::rhi::RegisterBackend(info);
}
