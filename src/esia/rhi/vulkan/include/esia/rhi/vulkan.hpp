// Esia - the Vulkan RHI backend (Vulkan 1.1+, dynamic rendering where the device has it): host integration.
//
// Creating the device from the host's objects
//
//   esia::rhi::vulkan::Desc d;
//   d.instance = instance; d.physicalDevice = gpu; d.device = device; d.queueFamily = family; d.queue = queue;
//   d.apiVersion = VK_API_VERSION_1_3;      // what the device was created with (>= 1.1)
//   d.framesInFlight = 2;                   // the host's own frames in flight (see "Frames" below)
//   d.dynamicRendering = true;              // the features / extensions the host enabled on `device`
//   std::unique_ptr<esia::rhi::Device> dev = esia::rhi::vulkan::CreateDevice(d, &error);
//
// The backend loads every entry point through Desc::getInstanceProcAddr (null: it opens the system loader itself),
// so it works with volk or any loader the host uses, and needs only the Vulkan headers to build.
//
// Render targets
//
//   Texture t = WrapImage(*dev, swapchainImage, view, VK_FORMAT_B8G8R8A8_SRGB, w, h, 1,
//                         VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, TextureUsage_RenderTarget);
//
// `view` is the host's view the backend renders through (its format decides sRGB encoding). `usage` says what the
// image allows: RenderTarget always; CopySrc when it was created with TRANSFER_SRC (glass then copies the backdrop
// from it - without it, glass needs Sampled); Sampled when it was created with SAMPLED (and, for an *_SRGB format,
// with VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT: the backend samples the raw bits through a UNORM view of its own). A
// wrapped image is in `layoutOnEntry` when a frame first uses it and is left in `layoutOnExit` at EndFrame (UNDEFINED
// on entry discards the contents). Wrapping is cached by VkImage: call it every frame, before Renderer::Render, for
// the image of that frame (the swap-chain image) - a sampleable image wrapped for the frame is moved to
// SHADER_READ_ONLY_OPTIMAL when the frame begins, because glass may read what the host drew before Esia's first
// pass. DestroyTexture releases the wrapper only, never the host's image or view.
//
// Frames
//
//   FrameDesc::nativeContext = the host's VkCommandBuffer, recording and outside a render pass. The backend records
//   the whole frame into it (uploads, passes, copies, timestamps); the host submits it. Its ring buffers, descriptor
//   pools and deferred releases are indexed by device frame modulo Desc::framesInFlight, so when BeginFrame is
//   called for frame N the host must already have waited for frame N - framesInFlight (the usual fence per frame in
//   flight; count one device frame per Renderer::Render). A host that renders several targets per frame passes its
//   frame number in FrameDesc::hostFrame (RenderParams::frame): the device frames of one host frame then share a
//   slot, and the count is in host frames.
//   With nativeContext = null the backend records into its own command buffers and submits them to Desc::queue at
//   EndFrame, with a fence per frame in flight (the headless / test mode).
//
// Everything else that touches Desc::queue (ReadPixels, which waits for the GPU) must not run while the host uses
// the queue from another thread. Destroy the device only when the GPU is idle (vkDeviceWaitIdle).
//
// Host callbacks (DrawCmdKind::Callback): NativeRenderState() returns the frame's VkCommandBuffer inside the current
// pass (dynamic rendering or a render pass: UsesDynamicRendering / RenderPassFor give what the host's pipelines must
// be compatible with). The host may bind anything; viewport and scissor are dynamic state in every Esia pipeline.
#pragma once
#include "esia/rhi/backend_registry.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <string>

namespace esia::rhi::vulkan
{
    struct Desc
    {
        PFN_vkGetInstanceProcAddr getInstanceProcAddr = nullptr;   // null: the system loader (libvulkan.so.1 / vulkan-1.dll)
        VkInstance instance = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        std::uint32_t queueFamily = 0;                             // a graphics queue family
        VkQueue queue = VK_NULL_HANDLE;                            // of that family: readback and self-submitted frames
        std::uint32_t apiVersion = VK_API_VERSION_1_1;             // the version `device` was created with (>= 1.1)
        int framesInFlight = 2;
        VkPipelineCache pipelineCache = VK_NULL_HANDLE;            // null: the backend keeps its own
        // What the host enabled on `device` (the backend only uses what is enabled):
        bool dynamicRendering = false;   // Vulkan 1.3 dynamicRendering or VK_KHR_dynamic_rendering; else render passes
        bool debugUtils = false;         // VK_EXT_debug_utils on the instance: resources get their debug names
    };

    // Null (and `error` set) when the device lacks what the backend needs.
    std::unique_ptr<Device> CreateDevice(const Desc& desc, std::string* error = nullptr);

    // A host image as a render target (see above). {} when `dev` is not a Vulkan device or the format is unknown.
    Texture WrapImage(Device& dev, VkImage image, VkImageView view, VkFormat format, int width, int height, int samples,
                      VkImageLayout layoutOnEntry, VkImageLayout layoutOnExit, std::uint32_t usage);

    // What host pipelines drawn from a callback must be compatible with: dynamic rendering (a VkPipelineRenderingCreateInfo
    // with the pass target's format), or else the render pass RenderPassFor returns for that format and sample count.
    bool UsesDynamicRendering(const Device& dev);
    VkRenderPass RenderPassFor(Device& dev, Format format, int samples);

    // The VkFormat the backend uses for an RHI format (VK_FORMAT_UNDEFINED for Format::Unknown), and back.
    VkFormat ToVkFormat(Format f);
    Format FromVkFormat(VkFormat f);

    // ---- headless devices (tests, tools): an instance and device of the backend's own on the first suitable GPU.
    struct HeadlessOptions
    {
        // Khronos validation layer with synchronization validation, when installed. Every warning or error it
        // reports is printed to stderr and counted, and makes ReadPixels fail: a test cannot pass with one.
        bool validation = true;
        bool dynamicRendering = true;    // false: render passes even where dynamic rendering exists (tests both paths)
        int framesInFlight = 2;
        // The highest Vulkan version to use (>= 1.1): lower it to run the paths of older devices (dynamic rendering
        // through VK_KHR_dynamic_rendering below 1.3, and its 1.1 dependencies).
        std::uint32_t maxApiVersion = VK_API_VERSION_1_3;
    };
    // Empty device (and `error` set) when this machine cannot run the backend: the conformance suite then skips it.
    // The backend's registered creator reads ESIA_VULKAN_VALIDATION=0 and ESIA_VULKAN_RENDER_PASS=1 from the
    // environment to change the defaults.
    HeadlessDevice CreateHeadless(const HeadlessDesc& desc, const HeadlessOptions& options, std::string& error);
    // Validation messages (warnings and errors) reported so far for a headless device; 0 for other devices.
    std::uint32_t ValidationMessages(const Device& dev);
}
