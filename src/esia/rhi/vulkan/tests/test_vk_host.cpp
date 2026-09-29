// Vulkan backend in a host: the test plays an application with its own Vulkan 1.1 instance and device (render
// passes, no dynamic rendering), its own sRGB image and command buffers. The backend is created from those objects
// (CreateDevice), the image is wrapped (WrapImage) and every frame is recorded into the host's command buffer
// (FrameDesc::nativeContext) with a host callback in the middle; the host copies the image out after its exit
// layout. The pixels must equal the headless device's rendering of the same scene.
#include "esia/render/renderer.hpp"
#include "esia/rhi/vulkan.hpp"
#include "esia_test.hpp"
#include "scenes.hpp"
#include "vk_loader.hpp"
#include <atomic>
#include <cstdio>
#include <cstring>

using namespace esia;
using namespace esia::rhi;

namespace
{
    std::atomic<int> gHostMessages{0};

    VKAPI_ATTR VkBool32 VKAPI_CALL OnMessage(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
                                             const VkDebugUtilsMessengerCallbackDataEXT* data, void*)
    {
        ++gHostMessages;
        std::fprintf(stderr, "esia vulkan [validation error] (host test) %s\n", data && data->pMessage ? data->pMessage : "");
        return VK_FALSE;
    }

    struct CallbackCheck
    {
        VkCommandBuffer expected = VK_NULL_HANDLE;
        int calls = 0, matches = 0;
    };

    void Callback(const DrawList&, const DrawCmd& cmd, void* state)
    {
        auto* check = static_cast<CallbackCheck*>(cmd.userData);
        ++check->calls;
        check->matches += state == check->expected ? 1 : 0;
    }

    // The host: an instance, a device and the objects an application would own.
    struct Host
    {
        vulkan::LoaderLibrary lib;
        vulkan::Functions vk;
        VkInstance instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
        VkPhysicalDevice gpu = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        std::uint32_t family = 0;
        VkQueue queue = VK_NULL_HANDLE;
        bool dualSrcBlend = false;

        bool Create(std::string& error)
        {
            lib = vulkan::OpenLoader(error);
            if (!lib.getInstanceProcAddr || !vk.LoadGlobal(lib.getInstanceProcAddr, error))
                return false;
            const char* layer = "VK_LAYER_KHRONOS_validation";
            std::uint32_t n = 0;
            vk.vkEnumerateInstanceLayerProperties(&n, nullptr);
            std::vector<VkLayerProperties> layers(n);
            vk.vkEnumerateInstanceLayerProperties(&n, layers.data());
            bool validation = false;
            for (const VkLayerProperties& l : layers)
                validation |= std::strcmp(l.layerName, layer) == 0;
            const char* ext = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
            VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
            app.apiVersion = VK_API_VERSION_1_1;
            VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
            ici.pApplicationInfo = &app;
            ici.enabledLayerCount = validation ? 1u : 0u;
            ici.ppEnabledLayerNames = &layer;
            ici.enabledExtensionCount = validation ? 1u : 0u;
            ici.ppEnabledExtensionNames = &ext;
            if (vk.vkCreateInstance(&ici, nullptr, &instance) != VK_SUCCESS || !vk.LoadInstance(instance, error))
                return false;
            if (validation)
            {
                VkDebugUtilsMessengerCreateInfoEXT mi{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
                mi.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                mi.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
                mi.pfnUserCallback = &OnMessage;
                vk.vkCreateDebugUtilsMessengerEXT(instance, &mi, nullptr, &messenger);
            }
            vk.vkEnumeratePhysicalDevices(instance, &n, nullptr);
            if (n == 0)
            {
                error = "no physical device";
                return false;
            }
            std::vector<VkPhysicalDevice> gpus(n);
            vk.vkEnumeratePhysicalDevices(instance, &n, gpus.data());
            gpu = gpus[0];
            vk.vkGetPhysicalDeviceQueueFamilyProperties(gpu, &n, nullptr);
            std::vector<VkQueueFamilyProperties> families(n);
            vk.vkGetPhysicalDeviceQueueFamilyProperties(gpu, &n, families.data());
            while (family < n && !(families[family].queueFlags & VK_QUEUE_GRAPHICS_BIT))
                ++family;
            VkPhysicalDeviceFeatures2 f{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
            vk.vkGetPhysicalDeviceFeatures2(gpu, &f);
            dualSrcBlend = f.features.dualSrcBlend == VK_TRUE;
            VkPhysicalDeviceFeatures enabled{};
            enabled.dualSrcBlend = f.features.dualSrcBlend;
            const float priority = 1.0f;
            VkDeviceQueueCreateInfo qi{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
            qi.queueFamilyIndex = family;
            qi.queueCount = 1;
            qi.pQueuePriorities = &priority;
            VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
            dci.queueCreateInfoCount = 1;
            dci.pQueueCreateInfos = &qi;
            dci.pEnabledFeatures = &enabled;
            if (family >= n || vk.vkCreateDevice(gpu, &dci, nullptr, &device) != VK_SUCCESS || !vk.LoadDevice(device, false, false, error))
                return false;
            vk.vkGetDeviceQueue(device, family, 0, &queue);
            return true;
        }

        void Destroy()
        {
            if (device)
                vk.vkDestroyDevice(device, nullptr);
            if (messenger)
                vk.vkDestroyDebugUtilsMessengerEXT(instance, messenger, nullptr);
            if (instance)
                vk.vkDestroyInstance(instance, nullptr);
            vulkan::CloseLoader(lib);
        }

        std::uint32_t Memory(std::uint32_t bits, VkMemoryPropertyFlags flags) const
        {
            VkPhysicalDeviceMemoryProperties mp;
            vk.vkGetPhysicalDeviceMemoryProperties(gpu, &mp);
            for (std::uint32_t i = 0; i < mp.memoryTypeCount; ++i)
                if ((bits & (1u << i)) && (mp.memoryTypes[i].propertyFlags & flags) == flags)
                    return i;
            return 0;
        }
    };
}

ESIA_TEST(VulkanHost, HostDeviceImageAndCommandBuffers)
{
    const conformance::Scene* scene = conformance::FindScene("srgb_target");
    const int W = scene->width, H = scene->height;

    // the reference: the headless device (dynamic rendering where available) renders the same scene
    std::vector<std::uint8_t> reference;
    {
        HeadlessDesc hd;
        hd.width = W;
        hd.height = H;
        hd.format = scene->format;
        std::string error;
        HeadlessDevice h = vulkan::CreateHeadless(hd, vulkan::HeadlessOptions{}, error);
        if (!h.device)
        {
            std::printf("  (no Vulkan device: %s)\n", error.c_str());
            return;
        }
        render::Renderer renderer(*h.device);
        conformance::SceneFrame frame;
        conformance::BuildScene(*scene, frame);
        ESIA_CHECK(renderer.Render(frame.data, &frame.textures, h.target));
        ESIA_CHECK(h.device->ReadPixels(h.target, IRect{0, 0, W, H}, reference));
    }

    Host host;
    std::string error;
    if (!host.Create(error))
    {
        std::printf("  (host device: %s)\n", error.c_str());
        host.Destroy();
        ESIA_CHECK(false);
        return;
    }
    const vulkan::Functions& vk = host.vk;

    // the host's render target: sRGB, sampleable through a UNORM view (mutable format), copyable
    VkImageCreateInfo ii{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    ii.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    ii.imageType = VK_IMAGE_TYPE_2D;
    ii.format = VK_FORMAT_R8G8B8A8_SRGB;
    ii.extent = {(std::uint32_t)W, (std::uint32_t)H, 1};
    ii.mipLevels = ii.arrayLayers = 1;
    ii.samples = VK_SAMPLE_COUNT_1_BIT;
    ii.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    VkImage image;
    ESIA_CHECK(vk.vkCreateImage(host.device, &ii, nullptr, &image) == VK_SUCCESS);
    VkMemoryRequirements req;
    vk.vkGetImageMemoryRequirements(host.device, image, &req);
    VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    ai.allocationSize = req.size;
    ai.memoryTypeIndex = host.Memory(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkDeviceMemory imageMemory;
    vk.vkAllocateMemory(host.device, &ai, nullptr, &imageMemory);
    vk.vkBindImageMemory(host.device, image, imageMemory, 0);
    VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    vi.image = image;
    vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vi.format = VK_FORMAT_R8G8B8A8_SRGB;
    vi.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkImageView view;
    vk.vkCreateImageView(host.device, &vi, nullptr, &view);

    // readback buffer the host copies into
    VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bi.size = (VkDeviceSize)W * H * 4;
    bi.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkBuffer readback;
    vk.vkCreateBuffer(host.device, &bi, nullptr, &readback);
    vk.vkGetBufferMemoryRequirements(host.device, readback, &req);
    ai.allocationSize = req.size;
    ai.memoryTypeIndex = host.Memory(req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDeviceMemory readbackMemory;
    vk.vkAllocateMemory(host.device, &ai, nullptr, &readbackMemory);
    vk.vkBindBufferMemory(host.device, readback, readbackMemory, 0);

    // two frames in flight, the host's own command buffers and fences
    constexpr int kFrames = 2;
    VkCommandPoolCreateInfo cpi{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    cpi.queueFamilyIndex = host.family;
    VkCommandPool pools[kFrames];
    VkCommandBuffer cmds[kFrames];
    for (int i = 0; i < kFrames; ++i)
    {
        vk.vkCreateCommandPool(host.device, &cpi, nullptr, &pools[i]);
        VkCommandBufferAllocateInfo cai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        cai.commandPool = pools[i];
        cai.commandBufferCount = 1;
        vk.vkAllocateCommandBuffers(host.device, &cai, &cmds[i]);
    }
    VkFence fences[kFrames];
    VkFenceCreateInfo fci{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (VkFence& f : fences)
        vk.vkCreateFence(host.device, &fci, nullptr, &f);

    {
        vulkan::Desc desc;
        desc.getInstanceProcAddr = host.lib.getInstanceProcAddr;
        desc.instance = host.instance;
        desc.physicalDevice = host.gpu;
        desc.device = host.device;
        desc.queueFamily = host.family;
        desc.queue = host.queue;
        desc.apiVersion = VK_API_VERSION_1_1;
        desc.framesInFlight = kFrames;
        desc.dualSrcBlend = host.dualSrcBlend;
        desc.debugUtils = host.messenger != VK_NULL_HANDLE;
        std::unique_ptr<Device> dev = vulkan::CreateDevice(desc, &error);
        ESIA_CHECK(dev != nullptr);
        if (!dev)
            std::printf("  CreateDevice: %s\n", error.c_str());
        else
        {
            ESIA_CHECK(!vulkan::UsesDynamicRendering(*dev));
            ESIA_CHECK(vulkan::RenderPassFor(*dev, Format::RGBA8_SRGB, 1) != VK_NULL_HANDLE);
            render::Renderer renderer(*dev);
            Texture first;
            CallbackCheck check;
            for (int f = 0; f < 4; ++f)
            {
                const int slot = f % kFrames;
                vk.vkWaitForFences(host.device, 1, &fences[slot], VK_TRUE, UINT64_MAX);
                vk.vkResetFences(host.device, 1, &fences[slot]);
                VkCommandBuffer cmd = cmds[slot];
                vk.vkResetCommandPool(host.device, pools[slot], 0);
                VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
                vk.vkBeginCommandBuffer(cmd, &begin);

                // wrapped every frame, as a swap-chain image would be: the same handle comes back
                const Texture target = vulkan::WrapImage(*dev, image, view, VK_FORMAT_R8G8B8A8_SRGB, W, H, 1, VK_IMAGE_LAYOUT_UNDEFINED,
                                                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                         TextureUsage_RenderTarget | TextureUsage_CopySrc | TextureUsage_Sampled);
                ESIA_CHECK((bool)target);
                if (f == 0)
                    first = target;
                ESIA_CHECK(target == first);

                conformance::SceneFrame frame;
                conformance::BuildScene(*scene, frame);
                check.expected = cmd;
                DrawList& callbacks = frame.NewList();
                callbacks.AddCallback(&Callback, &check);
                render::RenderParams params;
                params.frame.nativeContext = cmd;
                ESIA_CHECK(renderer.Render(frame.data, &frame.textures, target, params));

                // the image is in its exit layout (TRANSFER_SRC) now; the host copies it out
                VkBufferImageCopy region{};
                region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
                region.imageExtent = {(std::uint32_t)W, (std::uint32_t)H, 1};
                vk.vkCmdCopyImageToBuffer(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readback, 1, &region);
                VkMemoryBarrier toHost{VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_HOST_READ_BIT};
                vk.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &toHost, 0, nullptr, 0, nullptr);
                vk.vkEndCommandBuffer(cmd);
                VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
                si.commandBufferCount = 1;
                si.pCommandBuffers = &cmd;
                ESIA_CHECK(vk.vkQueueSubmit(host.queue, 1, &si, fences[slot]) == VK_SUCCESS);
            }
            vk.vkDeviceWaitIdle(host.device);
            ESIA_CHECK(check.calls == 4 && check.matches == 4);
            GpuProfile profile;
            ESIA_CHECK(!dev->GetCaps().timestampQueries || renderer.Stats().gpu.valid || dev->ReadProfile(profile));

            void* mapped = nullptr;
            vk.vkMapMemory(host.device, readbackMemory, 0, VK_WHOLE_SIZE, 0, &mapped);
            const auto* px = static_cast<const std::uint8_t*>(mapped);
            ESIA_CHECK(reference.size() == (std::size_t)W * H * 4 && std::memcmp(px, reference.data(), reference.size()) == 0);
            vk.vkUnmapMemory(host.device, readbackMemory);
            dev->DestroyTexture(first);   // the wrapper only: the host's image and view stay
        }
    }
    ESIA_CHECK(gHostMessages.load() == 0);

    for (VkFence f : fences)
        vk.vkDestroyFence(host.device, f, nullptr);
    for (VkCommandPool p : pools)
        vk.vkDestroyCommandPool(host.device, p, nullptr);
    vk.vkDestroyBuffer(host.device, readback, nullptr);
    vk.vkFreeMemory(host.device, readbackMemory, nullptr);
    vk.vkDestroyImageView(host.device, view, nullptr);
    vk.vkDestroyImage(host.device, image, nullptr);
    vk.vkFreeMemory(host.device, imageMemory, nullptr);
    host.Destroy();
}
