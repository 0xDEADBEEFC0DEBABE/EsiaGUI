// Vulkan backend in a host: the test plays an application with its own Vulkan 1.1 instance and device (render
// passes, no dynamic rendering), its own sRGB image and command buffers. The backend is created from those objects
// (CreateDevice), the image is wrapped (WrapImage) and every frame is recorded into the host's command buffer
// (FrameDesc::nativeContext) with a host callback in the middle; the host copies the image out after its exit
// layout. The pixels must equal the headless device's rendering of the same scene.
#include "esia/render/painter.hpp"
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

    // The host's render target (sRGB, sampleable through a UNORM view, copyable), a readback buffer and two frames
    // in flight with the host's own command buffers and fences.
    struct HostTarget
    {
        static constexpr int kFrames = 2;
        const Host& host;
        int W, H;
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory imageMemory = VK_NULL_HANDLE, readbackMemory = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkBuffer readback = VK_NULL_HANDLE;
        VkCommandPool pools[kFrames] = {};
        VkCommandBuffer cmds[kFrames] = {};
        VkFence fences[kFrames] = {};
        int frame = 0;

        HostTarget(const Host& h, int w, int hh) : host(h), W(w), H(hh)
        {
            const vulkan::Functions& vk = host.vk;
            VkImageCreateInfo ii{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
            ii.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
            ii.imageType = VK_IMAGE_TYPE_2D;
            ii.format = VK_FORMAT_R8G8B8A8_SRGB;
            ii.extent = {(std::uint32_t)W, (std::uint32_t)H, 1};
            ii.mipLevels = ii.arrayLayers = 1;
            ii.samples = VK_SAMPLE_COUNT_1_BIT;
            ii.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            vk.vkCreateImage(host.device, &ii, nullptr, &image);
            VkMemoryRequirements req;
            vk.vkGetImageMemoryRequirements(host.device, image, &req);
            VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
            ai.allocationSize = req.size;
            ai.memoryTypeIndex = host.Memory(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            vk.vkAllocateMemory(host.device, &ai, nullptr, &imageMemory);
            vk.vkBindImageMemory(host.device, image, imageMemory, 0);
            VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            vi.image = image;
            vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
            vi.format = VK_FORMAT_R8G8B8A8_SRGB;
            vi.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            vk.vkCreateImageView(host.device, &vi, nullptr, &view);

            VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            bi.size = (VkDeviceSize)W * H * 4;
            bi.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            vk.vkCreateBuffer(host.device, &bi, nullptr, &readback);
            vk.vkGetBufferMemoryRequirements(host.device, readback, &req);
            ai.allocationSize = req.size;
            ai.memoryTypeIndex = host.Memory(req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            vk.vkAllocateMemory(host.device, &ai, nullptr, &readbackMemory);
            vk.vkBindBufferMemory(host.device, readback, readbackMemory, 0);

            VkCommandPoolCreateInfo cpi{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
            cpi.queueFamilyIndex = host.family;
            VkFenceCreateInfo fci{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            for (int i = 0; i < kFrames; ++i)
            {
                vk.vkCreateCommandPool(host.device, &cpi, nullptr, &pools[i]);
                VkCommandBufferAllocateInfo cai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
                cai.commandPool = pools[i];
                cai.commandBufferCount = 1;
                vk.vkAllocateCommandBuffers(host.device, &cai, &cmds[i]);
                vk.vkCreateFence(host.device, &fci, nullptr, &fences[i]);
            }
        }

        ~HostTarget()
        {
            const vulkan::Functions& vk = host.vk;
            vk.vkDeviceWaitIdle(host.device);
            for (int i = 0; i < kFrames; ++i)
            {
                vk.vkDestroyFence(host.device, fences[i], nullptr);
                vk.vkDestroyCommandPool(host.device, pools[i], nullptr);
            }
            vk.vkDestroyBuffer(host.device, readback, nullptr);
            vk.vkFreeMemory(host.device, readbackMemory, nullptr);
            vk.vkDestroyImageView(host.device, view, nullptr);
            vk.vkDestroyImage(host.device, image, nullptr);
            vk.vkFreeMemory(host.device, imageMemory, nullptr);
        }

        // Waits for the frame that used this slot (the host's side of the frames-in-flight contract) and begins
        // recording.
        VkCommandBuffer Begin()
        {
            const vulkan::Functions& vk = host.vk;
            const int slot = frame % kFrames;
            vk.vkWaitForFences(host.device, 1, &fences[slot], VK_TRUE, UINT64_MAX);
            vk.vkResetFences(host.device, 1, &fences[slot]);
            vk.vkResetCommandPool(host.device, pools[slot], 0);
            VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
            vk.vkBeginCommandBuffer(cmds[slot], &begin);
            return cmds[slot];
        }

        // The image is in TRANSFER_SRC (the exit layout the tests wrap it with): copy it out and submit.
        bool End()
        {
            const vulkan::Functions& vk = host.vk;
            const int slot = frame++ % kFrames;
            VkCommandBuffer cmd = cmds[slot];
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
            return vk.vkQueueSubmit(host.queue, 1, &si, fences[slot]) == VK_SUCCESS;
        }

        std::vector<std::uint8_t> Pixels()
        {
            const vulkan::Functions& vk = host.vk;
            vk.vkDeviceWaitIdle(host.device);
            void* mapped = nullptr;
            vk.vkMapMemory(host.device, readbackMemory, 0, VK_WHOLE_SIZE, 0, &mapped);
            const auto* px = static_cast<const std::uint8_t*>(mapped);
            std::vector<std::uint8_t> out(px, px + (std::size_t)W * H * 4);
            vk.vkUnmapMemory(host.device, readbackMemory);
            return out;
        }

        Texture Wrap(Device& dev, VkImageLayout entry)
        {
            return vulkan::WrapImage(dev, image, view, VK_FORMAT_R8G8B8A8_SRGB, W, H, 1, entry, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                     TextureUsage_RenderTarget | TextureUsage_CopySrc | TextureUsage_Sampled);
        }
    };

    std::unique_ptr<Device> CreateBackend(const Host& host, std::string& error)
    {
        vulkan::Desc desc;
        desc.getInstanceProcAddr = host.lib.getInstanceProcAddr;
        desc.instance = host.instance;
        desc.physicalDevice = host.gpu;
        desc.device = host.device;
        desc.queueFamily = host.family;
        desc.queue = host.queue;
        desc.apiVersion = VK_API_VERSION_1_1;
        desc.framesInFlight = HostTarget::kFrames;
        desc.dualSrcBlend = host.dualSrcBlend;
        desc.debugUtils = host.messenger != VK_NULL_HANDLE;
        return vulkan::CreateDevice(desc, &error);
    }
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
    const bool created = host.Create(error);
    ESIA_CHECK(created);
    if (created)
    {
        HostTarget target(host, W, H);
        std::unique_ptr<Device> dev = CreateBackend(host, error);
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
                // wrapped every frame, as a swap-chain image would be: the same handle comes back
                const Texture wrapped = target.Wrap(*dev, VK_IMAGE_LAYOUT_UNDEFINED);
                ESIA_CHECK((bool)wrapped);
                if (f == 0)
                    first = wrapped;
                ESIA_CHECK(wrapped == first);
                VkCommandBuffer cmd = target.Begin();
                conformance::SceneFrame frame;
                conformance::BuildScene(*scene, frame);
                check.expected = cmd;
                frame.NewList().AddCallback(&Callback, &check);
                render::RenderParams params;
                params.frame.nativeContext = cmd;
                ESIA_CHECK(renderer.Render(frame.data, &frame.textures, wrapped, params));
                ESIA_CHECK(target.End());
            }
            ESIA_CHECK(check.calls == 4 && check.matches == 4);
            ESIA_CHECK(target.Pixels() == reference);
            GpuProfile profile;
            ESIA_CHECK(!dev->GetCaps().timestampQueries || renderer.Stats().gpu.valid || dev->ReadProfile(profile));
            dev->DestroyTexture(first);   // the wrapper only: the host's image and view stay
        }
    }
    ESIA_CHECK(gHostMessages.load() == 0);
    host.Destroy();
}

// The host drew first (here: a clear) and Esia's first operation reads it: glass over host content captures the
// wrapped image straight from its entry layout (the direct read samples it before any Esia pass touched it).
ESIA_TEST(VulkanHost, GlassOverHostContent)
{
    Host host;
    std::string error;
    if (!host.Create(error))
    {
        std::printf("  (no Vulkan device: %s)\n", error.c_str());
        host.Destroy();
        return;
    }
    {
        const int W = 160, H = 120;
        HostTarget target(host, W, H);
        std::unique_ptr<Device> dev = CreateBackend(host, error);
        ESIA_CHECK(dev != nullptr);
        if (dev)
        {
            render::Renderer renderer(*dev);
            for (int f = 0; f < 3; ++f)
            {
                const Texture wrapped = target.Wrap(*dev, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                VkCommandBuffer cmd = target.Begin();
                VkImageMemoryBarrier toDst{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
                toDst.srcAccessMask = 0;
                toDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                toDst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                toDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                toDst.srcQueueFamilyIndex = toDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                toDst.image = target.image;
                toDst.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
                host.vk.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toDst);
                const VkClearColorValue red{{0.8f, 0.1f, 0.1f, 1.0f}};
                host.vk.vkCmdClearColorImage(cmd, target.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &red, 1, &toDst.subresourceRange);

                DrawList dl;
                dl.Reset(Rect(Vec2(0, 0), Vec2((float)W, (float)H)));
                Painter p(dl);
                GlassMaterial frost;
                frost.blur = 10.0f;
                p.Rect(Rect(40, 30, 120, 90), Style().Radius(16).Glass(frost));
                DrawData dd;
                dd.lists.push_back(&dl);
                dd.displaySize = Vec2((float)W, (float)H);
                render::RenderParams params;
                params.frame.nativeContext = cmd;
                ESIA_CHECK(renderer.Render(dd, nullptr, wrapped, params));
                ESIA_CHECK(target.End());
            }
            ESIA_CHECK(renderer.Stats().backdropCaptures == 1);
            const std::vector<std::uint8_t> px = target.Pixels();
            const std::uint8_t* corner = px.data();
            const std::uint8_t* center = px.data() + ((std::size_t)60 * W + 80) * 4;
            // outside the glass: the host's clear (sRGB-encoded 0.8 / 0.1); inside: frosted red, not garbage
            ESIA_CHECK(corner[0] > 220 && corner[1] < 100 && corner[2] < 100);
            ESIA_CHECK(center[0] > 150 && center[0] > center[1] + 60 && center[3] == 255);
        }
    }
    ESIA_CHECK(gHostMessages.load() == 0);
    host.Destroy();
}
