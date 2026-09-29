// glass_window - Vulkan host: VkSurfaceKHR + swapchain, two frames in flight of its own (fence, acquire semaphore and
// command buffer per frame slot, a present semaphore per swapchain image), Esia recording into the frame's command
// buffer. vulkan-1.dll is loaded at run time, as the backend does: no import library needed.
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include "host.hpp"
#include "esia/rhi/vulkan.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace glass
{
    namespace
    {
        constexpr int kFramesInFlight = 2;

#define GLASS_VK_INSTANCE_FUNCTIONS(X)                                                                                  \
    X(vkDestroyInstance) X(vkEnumeratePhysicalDevices) X(vkGetPhysicalDeviceProperties) X(vkGetPhysicalDeviceFeatures2) \
    X(vkGetPhysicalDeviceQueueFamilyProperties) X(vkEnumerateDeviceExtensionProperties) X(vkCreateDevice)               \
    X(vkGetDeviceProcAddr) X(vkCreateWin32SurfaceKHR) X(vkDestroySurfaceKHR) X(vkGetPhysicalDeviceSurfaceSupportKHR)    \
    X(vkGetPhysicalDeviceSurfaceCapabilitiesKHR) X(vkGetPhysicalDeviceSurfaceFormatsKHR)                               \
    X(vkGetPhysicalDeviceSurfacePresentModesKHR)
#define GLASS_VK_DEVICE_FUNCTIONS(X)                                                                                    \
    X(vkDestroyDevice) X(vkGetDeviceQueue) X(vkDeviceWaitIdle) X(vkCreateSwapchainKHR) X(vkDestroySwapchainKHR)         \
    X(vkGetSwapchainImagesKHR) X(vkAcquireNextImageKHR) X(vkQueuePresentKHR) X(vkCreateImageView) X(vkDestroyImageView) \
    X(vkCreateCommandPool) X(vkDestroyCommandPool) X(vkAllocateCommandBuffers) X(vkBeginCommandBuffer)                  \
    X(vkEndCommandBuffer) X(vkResetCommandBuffer) X(vkQueueSubmit) X(vkCreateFence) X(vkDestroyFence)                   \
    X(vkWaitForFences) X(vkResetFences) X(vkCreateSemaphore) X(vkDestroySemaphore)

        struct Vk
        {
            PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
            PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion = nullptr;
            PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties = nullptr;
            PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties = nullptr;
            PFN_vkCreateInstance vkCreateInstance = nullptr;
            PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
            PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;
#define GLASS_VK_MEMBER(name) PFN_##name name = nullptr;
            GLASS_VK_INSTANCE_FUNCTIONS(GLASS_VK_MEMBER)
            GLASS_VK_DEVICE_FUNCTIONS(GLASS_VK_MEMBER)
#undef GLASS_VK_MEMBER
        };

        std::uint32_t g_validationMessages = 0;   // the validation layer's warnings and errors (--debug)

        VKAPI_ATTR VkBool32 VKAPI_CALL OnDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
                                                       const VkDebugUtilsMessengerCallbackDataEXT* data, void*)
        {
            ++g_validationMessages;
            std::fprintf(stderr, "vulkan validation: %s\n", data->pMessage);
            return VK_FALSE;
        }

        class HostVulkan final : public Host
        {
        public:
            ~HostVulkan() override
            {
                if (device_)
                {
                    vk_.vkDeviceWaitIdle(device_);
                    ReleaseSwapchain();
                    esia_.reset();
                    for (int i = 0; i < kFramesInFlight; ++i)
                    {
                        vk_.vkDestroyFence(device_, fences_[i], nullptr);
                        vk_.vkDestroySemaphore(device_, acquired_[i], nullptr);
                    }
                    vk_.vkDestroyCommandPool(device_, pool_, nullptr);
                    vk_.vkDestroyDevice(device_, nullptr);
                }
                if (instance_)
                {
                    if (surface_)
                        vk_.vkDestroySurfaceKHR(instance_, surface_, nullptr);
                    if (messenger_)
                        vk_.vkDestroyDebugUtilsMessengerEXT(instance_, messenger_, nullptr);
                    vk_.vkDestroyInstance(instance_, nullptr);
                }
                if (library_)
                    ::FreeLibrary(library_);
            }

            const char* Name() const override { return "Vulkan"; }

            bool Init(HWND hwnd, int width, int height, const HostOptions& o, std::string& error) override
            {
                vsync_ = o.vsync;
                width_ = width;
                height_ = height;
                return CreateInstance(hwnd, o.debug, error) && CreateDevice(error) && CreateSwapchain(error);
            }

            esia::rhi::Device& Device() override { return *esia_; }

            void Resize(int width, int height) override
            {
                width_ = width;
                height_ = height;
                Recreate();
            }

            bool BeginFrame() override
            {
                if (!swapchain_)
                    Recreate();   // it had none while the surface was 0 x 0
                if (!swapchain_)
                    return false;
                const int slot = (int)(frame_ % kFramesInFlight);
                // the frame that used this slot kFramesInFlight frames ago is done: Esia recycles its per-frame
                // memory on the same schedule (Desc::framesInFlight)
                vk_.vkWaitForFences(device_, 1, &fences_[slot], VK_TRUE, UINT64_MAX);
                const VkResult r = vk_.vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, acquired_[slot], VK_NULL_HANDLE, &image_);
                if (r == VK_ERROR_OUT_OF_DATE_KHR)
                {
                    Recreate();   // the surface changed before the platform told us
                    return false;
                }
                if (r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR)
                    return false;
                vk_.vkResetFences(device_, 1, &fences_[slot]);
                vk_.vkResetCommandBuffer(commands_[slot], 0);
                VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
                bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                vk_.vkBeginCommandBuffer(commands_[slot], &bi);
                // wrapped every frame (the backend's contract): UNDEFINED on entry discards what the image held
                Image& img = images_[image_];
                img.wrapped = esia::rhi::vulkan::WrapImage(*esia_, img.image, img.view, format_, (int)extent_.width, (int)extent_.height, 1,
                                                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, usage_);
                target_ = img.wrapped;
                return (bool)target_;
            }

            esia::rhi::FrameDesc FrameParams() override
            {
                esia::rhi::FrameDesc f;
                f.nativeContext = commands_[frame_ % kFramesInFlight];
                f.hostFrame = frame_ + 1;
                return f;
            }

            std::uint32_t ValidationMessages() override { return g_validationMessages + esia_->ValidationErrors(); }

        protected:
            void Submit() override
            {
                const int slot = (int)(frame_ % kFramesInFlight);
                vk_.vkEndCommandBuffer(commands_[slot]);
                // Esia's first use of the image may be any stage (a copy for glass, a pass): all of them wait
                const VkPipelineStageFlags wait = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
                VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
                si.waitSemaphoreCount = 1;
                si.pWaitSemaphores = &acquired_[slot];
                si.pWaitDstStageMask = &wait;
                si.commandBufferCount = 1;
                si.pCommandBuffers = &commands_[slot];
                si.signalSemaphoreCount = 1;
                si.pSignalSemaphores = &images_[image_].rendered;
                vk_.vkQueueSubmit(queue_, 1, &si, fences_[slot]);
            }

            void Present() override
            {
                VkPresentInfoKHR pi{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
                pi.waitSemaphoreCount = 1;
                pi.pWaitSemaphores = &images_[image_].rendered;
                pi.swapchainCount = 1;
                pi.pSwapchains = &swapchain_;
                pi.pImageIndices = &image_;
                const VkResult r = vk_.vkQueuePresentKHR(queue_, &pi);
                ++frame_;
                if (r == VK_ERROR_OUT_OF_DATE_KHR || r == VK_SUBOPTIMAL_KHR)
                    Recreate();
            }

        private:
            struct Image
            {
                VkImage image = VK_NULL_HANDLE;
                VkImageView view = VK_NULL_HANDLE;
                // signalled by the frame that renders the image, waited on by its present: one per image, since a
                // present's semaphore is free again only when that image is acquired again
                VkSemaphore rendered = VK_NULL_HANDLE;
                esia::rhi::Texture wrapped;
            };

            template <class Fn>
            Fn Load(const char* name) const
            {
                return reinterpret_cast<Fn>(vk_.vkGetInstanceProcAddr(instance_, name));
            }

            bool CreateInstance(HWND hwnd, bool debug, std::string& error)
            {
                library_ = ::LoadLibraryW(L"vulkan-1.dll");
                if (library_)
                    vk_.vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(reinterpret_cast<void*>(::GetProcAddress(library_, "vkGetInstanceProcAddr")));
                if (!vk_.vkGetInstanceProcAddr)
                {
                    error = "no Vulkan loader (vulkan-1.dll)";
                    return false;
                }
                vk_.vkEnumerateInstanceVersion = Load<PFN_vkEnumerateInstanceVersion>("vkEnumerateInstanceVersion");
                vk_.vkEnumerateInstanceExtensionProperties = Load<PFN_vkEnumerateInstanceExtensionProperties>("vkEnumerateInstanceExtensionProperties");
                vk_.vkEnumerateInstanceLayerProperties = Load<PFN_vkEnumerateInstanceLayerProperties>("vkEnumerateInstanceLayerProperties");
                vk_.vkCreateInstance = Load<PFN_vkCreateInstance>("vkCreateInstance");
                std::uint32_t version = VK_API_VERSION_1_0;
                if (vk_.vkEnumerateInstanceVersion)
                    vk_.vkEnumerateInstanceVersion(&version);
                if (version < VK_API_VERSION_1_1)
                {
                    error = "the Vulkan loader is older than 1.1";
                    return false;
                }
                apiVersion_ = std::min<std::uint32_t>(version, VK_API_VERSION_1_3);

                std::vector<const char*> extensions = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
                std::vector<const char*> layers;
                if (debug)
                {
                    std::uint32_t n = 0;
                    vk_.vkEnumerateInstanceLayerProperties(&n, nullptr);
                    std::vector<VkLayerProperties> available(n);
                    vk_.vkEnumerateInstanceLayerProperties(&n, available.data());
                    for (const VkLayerProperties& l : available)
                        if (std::strcmp(l.layerName, "VK_LAYER_KHRONOS_validation") == 0)
                            layers.push_back("VK_LAYER_KHRONOS_validation");
                    if (layers.empty())
                        std::fprintf(stderr, "glass_window: --debug: VK_LAYER_KHRONOS_validation is not installed (Vulkan SDK)\n");
                    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                }
                VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
                app.pApplicationName = "glass_window";
                app.apiVersion = apiVersion_;
                VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
                ici.pApplicationInfo = &app;
                ici.enabledExtensionCount = (std::uint32_t)extensions.size();
                ici.ppEnabledExtensionNames = extensions.data();
                ici.enabledLayerCount = (std::uint32_t)layers.size();
                ici.ppEnabledLayerNames = layers.data();
                VkResult r = vk_.vkCreateInstance(&ici, nullptr, &instance_);
                if (r != VK_SUCCESS)
                {
                    error = "vkCreateInstance failed (" + std::to_string(r) + ")";
                    return false;
                }
#define GLASS_VK_LOAD(name) vk_.name = Load<PFN_##name>(#name);
                GLASS_VK_INSTANCE_FUNCTIONS(GLASS_VK_LOAD)
#undef GLASS_VK_LOAD
                if (debug)
                {
                    vk_.vkCreateDebugUtilsMessengerEXT = Load<PFN_vkCreateDebugUtilsMessengerEXT>("vkCreateDebugUtilsMessengerEXT");
                    vk_.vkDestroyDebugUtilsMessengerEXT = Load<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT");
                    VkDebugUtilsMessengerCreateInfoEXT mi{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
                    mi.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                    mi.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                    mi.pfnUserCallback = OnDebugMessage;
                    if (vk_.vkCreateDebugUtilsMessengerEXT)
                        vk_.vkCreateDebugUtilsMessengerEXT(instance_, &mi, nullptr, &messenger_);
                    debugUtils_ = messenger_ != VK_NULL_HANDLE;
                }
                VkWin32SurfaceCreateInfoKHR si{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
                si.hinstance = ::GetModuleHandleW(nullptr);
                si.hwnd = hwnd;
                if ((r = vk_.vkCreateWin32SurfaceKHR(instance_, &si, nullptr, &surface_)) != VK_SUCCESS)
                {
                    error = "vkCreateWin32SurfaceKHR failed (" + std::to_string(r) + ")";
                    return false;
                }
                return true;
            }

            bool CreateDevice(std::string& error)
            {
                // a GPU with a queue family that draws and presents to the window; discrete GPUs first
                std::uint32_t n = 0;
                vk_.vkEnumeratePhysicalDevices(instance_, &n, nullptr);
                std::vector<VkPhysicalDevice> gpus(n);
                vk_.vkEnumeratePhysicalDevices(instance_, &n, gpus.data());
                int best = -1;
                for (VkPhysicalDevice gpu : gpus)
                {
                    VkPhysicalDeviceProperties props;
                    vk_.vkGetPhysicalDeviceProperties(gpu, &props);
                    if (props.apiVersion < VK_API_VERSION_1_1)
                        continue;
                    std::uint32_t families = 0;
                    vk_.vkGetPhysicalDeviceQueueFamilyProperties(gpu, &families, nullptr);
                    std::vector<VkQueueFamilyProperties> qf(families);
                    vk_.vkGetPhysicalDeviceQueueFamilyProperties(gpu, &families, qf.data());
                    for (std::uint32_t f = 0; f < families; ++f)
                    {
                        VkBool32 present = VK_FALSE;
                        vk_.vkGetPhysicalDeviceSurfaceSupportKHR(gpu, f, surface_, &present);
                        if (!(qf[f].queueFlags & VK_QUEUE_GRAPHICS_BIT) || !present)
                            continue;
                        const int score = props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 2 : 1;
                        if (score > best)
                        {
                            best = score;
                            gpu_ = gpu;
                            family_ = f;
                            apiVersion_ = std::min(apiVersion_, props.apiVersion);
                            adapter_ = props.deviceName;
                        }
                        break;
                    }
                }
                if (!gpu_)
                {
                    error = "no Vulkan 1.1 GPU presents to this window";
                    return false;
                }
                // dynamic rendering where Vulkan 1.3 has it; the backend's render-pass path otherwise
                VkPhysicalDeviceVulkan13Features f13{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
                if (apiVersion_ >= VK_API_VERSION_1_3)
                {
                    VkPhysicalDeviceFeatures2 f2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
                    f2.pNext = &f13;
                    vk_.vkGetPhysicalDeviceFeatures2(gpu_, &f2);
                }
                dynamicRendering_ = f13.dynamicRendering == VK_TRUE;
                VkPhysicalDeviceVulkan13Features enable13{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
                enable13.dynamicRendering = VK_TRUE;

                const float priority = 1.0f;
                VkDeviceQueueCreateInfo qi{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
                qi.queueFamilyIndex = family_;
                qi.queueCount = 1;
                qi.pQueuePriorities = &priority;
                const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
                VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
                dci.pNext = dynamicRendering_ ? &enable13 : nullptr;
                dci.queueCreateInfoCount = 1;
                dci.pQueueCreateInfos = &qi;
                dci.enabledExtensionCount = 1;
                dci.ppEnabledExtensionNames = extensions;
                VkResult r = vk_.vkCreateDevice(gpu_, &dci, nullptr, &device_);
                if (r != VK_SUCCESS)
                {
                    error = "vkCreateDevice failed (" + std::to_string(r) + ")";
                    return false;
                }
#define GLASS_VK_LOAD(name) vk_.name = reinterpret_cast<PFN_##name>(vk_.vkGetDeviceProcAddr(device_, #name));
                GLASS_VK_DEVICE_FUNCTIONS(GLASS_VK_LOAD)
#undef GLASS_VK_LOAD
                vk_.vkGetDeviceQueue(device_, family_, 0, &queue_);

                VkCommandPoolCreateInfo pci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
                pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                pci.queueFamilyIndex = family_;
                vk_.vkCreateCommandPool(device_, &pci, nullptr, &pool_);
                VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
                ai.commandPool = pool_;
                ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                ai.commandBufferCount = kFramesInFlight;
                vk_.vkAllocateCommandBuffers(device_, &ai, commands_);
                VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
                fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;   // the first wait of each slot returns at once
                VkSemaphoreCreateInfo sci{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
                for (int i = 0; i < kFramesInFlight; ++i)
                {
                    vk_.vkCreateFence(device_, &fi, nullptr, &fences_[i]);
                    vk_.vkCreateSemaphore(device_, &sci, nullptr, &acquired_[i]);
                }

                esia::rhi::vulkan::Desc d;
                d.getInstanceProcAddr = vk_.vkGetInstanceProcAddr;
                d.instance = instance_;
                d.physicalDevice = gpu_;
                d.device = device_;
                d.queueFamily = family_;
                d.queue = queue_;
                d.apiVersion = apiVersion_;
                d.framesInFlight = kFramesInFlight;
                d.dynamicRendering = dynamicRendering_;
                d.debugUtils = debugUtils_;
                esia_ = esia::rhi::vulkan::CreateDevice(d, &error);
                return (bool)esia_;
            }

            void Recreate()
            {
                std::string error;
                if (!CreateSwapchain(error))
                    std::fprintf(stderr, "glass_window: %s\n", error.c_str());
            }

            // (Re)creates the swapchain for the window's size; the old one is retired only when the GPU is idle.
            bool CreateSwapchain(std::string& error)
            {
                vk_.vkDeviceWaitIdle(device_);
                VkSurfaceCapabilitiesKHR caps;
                vk_.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu_, surface_, &caps);
                VkExtent2D extent = caps.currentExtent;
                if (extent.width == 0xFFFFFFFFu)   // the surface takes the swapchain's size
                    extent = {std::clamp((std::uint32_t)width_, caps.minImageExtent.width, caps.maxImageExtent.width),
                              std::clamp((std::uint32_t)height_, caps.minImageExtent.height, caps.maxImageExtent.height)};
                if (extent.width == 0 || extent.height == 0)   // minimized: no swapchain until it is restored
                {
                    ReleaseSwapchain();
                    return true;
                }

                std::uint32_t n = 0;
                vk_.vkGetPhysicalDeviceSurfaceFormatsKHR(gpu_, surface_, &n, nullptr);
                std::vector<VkSurfaceFormatKHR> formats(n);
                vk_.vkGetPhysicalDeviceSurfaceFormatsKHR(gpu_, surface_, &n, formats.data());
                VkSurfaceFormatKHR chosen = {VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
                for (const VkSurfaceFormatKHR& f : formats)   // UNORM, as the other hosts: the shaders write display values
                    if ((f.format == VK_FORMAT_B8G8R8A8_UNORM || f.format == VK_FORMAT_R8G8B8A8_UNORM) && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    {
                        chosen = f;
                        break;
                    }
                if (chosen.format == VK_FORMAT_UNDEFINED)
                {
                    error = "the surface has no 8-bit UNORM format";
                    return false;
                }
                vk_.vkGetPhysicalDeviceSurfacePresentModesKHR(gpu_, surface_, &n, nullptr);
                std::vector<VkPresentModeKHR> modes(n);
                vk_.vkGetPhysicalDeviceSurfacePresentModesKHR(gpu_, surface_, &n, modes.data());
                VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;   // vsync, always there
                if (!vsync_)
                    for (VkPresentModeKHR m : {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR})
                        if (std::find(modes.begin(), modes.end(), m) != modes.end())
                        {
                            mode = m;
                            break;
                        }

                // copies (glass backdrops, --screenshot) and sampling (glass without a copy) where the surface allows
                VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                usage_ = esia::rhi::TextureUsage_RenderTarget;
                if (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
                {
                    usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                    usage_ |= esia::rhi::TextureUsage_CopySrc;
                }
                if (caps.supportedUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT)
                {
                    usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
                    usage_ |= esia::rhi::TextureUsage_Sampled;
                }
                VkSwapchainCreateInfoKHR sci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
                sci.surface = surface_;
                sci.minImageCount = std::max(caps.minImageCount + 1, 2u);
                if (caps.maxImageCount)
                    sci.minImageCount = std::min(sci.minImageCount, caps.maxImageCount);
                sci.imageFormat = chosen.format;
                sci.imageColorSpace = chosen.colorSpace;
                sci.imageExtent = extent;
                sci.imageArrayLayers = 1;
                sci.imageUsage = usage;
                sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                sci.preTransform = caps.currentTransform;
                sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                sci.presentMode = mode;
                sci.clipped = VK_TRUE;
                sci.oldSwapchain = swapchain_;
                VkSwapchainKHR created = VK_NULL_HANDLE;
                const VkResult r = vk_.vkCreateSwapchainKHR(device_, &sci, nullptr, &created);
                ReleaseSwapchain();
                if (r != VK_SUCCESS)
                {
                    error = "vkCreateSwapchainKHR failed (" + std::to_string(r) + ")";
                    return false;
                }
                swapchain_ = created;
                format_ = chosen.format;
                extent_ = extent;

                vk_.vkGetSwapchainImagesKHR(device_, swapchain_, &n, nullptr);
                std::vector<VkImage> images(n);
                vk_.vkGetSwapchainImagesKHR(device_, swapchain_, &n, images.data());
                images_.resize(n);
                VkSemaphoreCreateInfo semi{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
                for (std::uint32_t i = 0; i < n; ++i)
                {
                    Image& img = images_[i];
                    img.image = images[i];
                    VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
                    vi.image = img.image;
                    vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    vi.format = format_;
                    vi.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
                    vk_.vkCreateImageView(device_, &vi, nullptr, &img.view);
                    vk_.vkCreateSemaphore(device_, &semi, nullptr, &img.rendered);
                }
                return true;
            }

            // The swapchain with its views, semaphores and Esia wrappers (the GPU is idle).
            void ReleaseSwapchain()
            {
                for (Image& img : images_)
                {
                    if (img.wrapped)
                        esia_->DestroyTexture(img.wrapped);
                    vk_.vkDestroyImageView(device_, img.view, nullptr);
                    vk_.vkDestroySemaphore(device_, img.rendered, nullptr);
                }
                images_.clear();
                target_ = {};
                vk_.vkDestroySwapchainKHR(device_, swapchain_, nullptr);
                swapchain_ = VK_NULL_HANDLE;
            }

            Vk vk_;
            HMODULE library_ = nullptr;
            std::uint32_t apiVersion_ = VK_API_VERSION_1_1;
            VkInstance instance_ = VK_NULL_HANDLE;
            VkDebugUtilsMessengerEXT messenger_ = VK_NULL_HANDLE;
            bool debugUtils_ = false;
            VkSurfaceKHR surface_ = VK_NULL_HANDLE;
            VkPhysicalDevice gpu_ = VK_NULL_HANDLE;
            std::uint32_t family_ = 0;
            bool dynamicRendering_ = false;
            VkDevice device_ = VK_NULL_HANDLE;
            VkQueue queue_ = VK_NULL_HANDLE;
            VkCommandPool pool_ = VK_NULL_HANDLE;
            VkCommandBuffer commands_[kFramesInFlight] = {};
            VkFence fences_[kFramesInFlight] = {};
            VkSemaphore acquired_[kFramesInFlight] = {};
            std::uint64_t frame_ = 0;
            VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
            VkFormat format_ = VK_FORMAT_UNDEFINED;
            VkExtent2D extent_ = {};
            int width_ = 0, height_ = 0;   // the client area the platform reported
            std::uint32_t usage_ = 0;
            std::uint32_t image_ = 0;
            std::vector<Image> images_;
            std::unique_ptr<esia::rhi::Device> esia_;
        };
    }

    std::unique_ptr<Host> CreateHostVulkan() { return std::make_unique<HostVulkan>(); }
}
