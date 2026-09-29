// Esia - Vulkan backend: the function table.
//
// The backend never links the Vulkan loader: it takes vkGetInstanceProcAddr from the host (who may use volk, its
// own loader or a layer) or opens the system loader at runtime (libvulkan.so.1, vulkan-1.dll, libvulkan.1.dylib).
// So a binary with the backend built in still starts on a machine without Vulkan - the headless creator then
// reports why, and the conformance suite skips the backend instead of failing to load.
#pragma once
#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif
#include <vulkan/vulkan.h>
#include <string>

namespace esia::rhi::vulkan
{
// Before an instance exists.
#define ESIA_VK_GLOBAL_FUNCTIONS(X)              \
    X(vkCreateInstance)                          \
    X(vkEnumerateInstanceVersion)                \
    X(vkEnumerateInstanceLayerProperties)        \
    X(vkEnumerateInstanceExtensionProperties)

#define ESIA_VK_INSTANCE_FUNCTIONS(X)             \
    X(vkDestroyInstance)                          \
    X(vkEnumeratePhysicalDevices)                 \
    X(vkGetPhysicalDeviceProperties)              \
    X(vkGetPhysicalDeviceFeatures2)               \
    X(vkGetPhysicalDeviceQueueFamilyProperties)   \
    X(vkGetPhysicalDeviceMemoryProperties)        \
    X(vkGetPhysicalDeviceFormatProperties)        \
    X(vkGetPhysicalDeviceImageFormatProperties)   \
    X(vkEnumerateDeviceExtensionProperties)       \
    X(vkCreateDevice)                             \
    X(vkGetDeviceProcAddr)

// VK_EXT_debug_utils: null when the instance was created without it.
#define ESIA_VK_DEBUG_FUNCTIONS(X)                \
    X(vkCreateDebugUtilsMessengerEXT)             \
    X(vkDestroyDebugUtilsMessengerEXT)            \
    X(vkSetDebugUtilsObjectNameEXT)

#define ESIA_VK_DEVICE_FUNCTIONS(X)              \
    X(vkDestroyDevice)                           \
    X(vkGetDeviceQueue)                          \
    X(vkDeviceWaitIdle)                          \
    X(vkQueueSubmit)                             \
    X(vkQueueWaitIdle)                           \
    X(vkAllocateMemory)                          \
    X(vkFreeMemory)                              \
    X(vkMapMemory)                               \
    X(vkUnmapMemory)                             \
    X(vkCreateBuffer)                            \
    X(vkDestroyBuffer)                           \
    X(vkGetBufferMemoryRequirements)             \
    X(vkBindBufferMemory)                        \
    X(vkCreateImage)                             \
    X(vkDestroyImage)                            \
    X(vkGetImageMemoryRequirements)              \
    X(vkBindImageMemory)                         \
    X(vkCreateImageView)                         \
    X(vkDestroyImageView)                        \
    X(vkCreateSampler)                           \
    X(vkDestroySampler)                          \
    X(vkCreateShaderModule)                      \
    X(vkDestroyShaderModule)                     \
    X(vkCreatePipelineCache)                     \
    X(vkDestroyPipelineCache)                    \
    X(vkCreateGraphicsPipelines)                 \
    X(vkDestroyPipeline)                         \
    X(vkCreatePipelineLayout)                    \
    X(vkDestroyPipelineLayout)                   \
    X(vkCreateDescriptorSetLayout)               \
    X(vkDestroyDescriptorSetLayout)              \
    X(vkCreateDescriptorPool)                    \
    X(vkDestroyDescriptorPool)                   \
    X(vkResetDescriptorPool)                     \
    X(vkAllocateDescriptorSets)                  \
    X(vkUpdateDescriptorSets)                    \
    X(vkCreateRenderPass)                        \
    X(vkDestroyRenderPass)                       \
    X(vkCreateFramebuffer)                       \
    X(vkDestroyFramebuffer)                      \
    X(vkCreateCommandPool)                       \
    X(vkDestroyCommandPool)                      \
    X(vkResetCommandPool)                        \
    X(vkAllocateCommandBuffers)                  \
    X(vkBeginCommandBuffer)                      \
    X(vkEndCommandBuffer)                        \
    X(vkCreateFence)                             \
    X(vkDestroyFence)                            \
    X(vkResetFences)                             \
    X(vkWaitForFences)                           \
    X(vkCreateQueryPool)                         \
    X(vkDestroyQueryPool)                        \
    X(vkGetQueryPoolResults)                     \
    X(vkCmdResetQueryPool)                       \
    X(vkCmdWriteTimestamp)                       \
    X(vkCmdPipelineBarrier)                      \
    X(vkCmdBeginRenderPass)                      \
    X(vkCmdEndRenderPass)                        \
    X(vkCmdBindPipeline)                         \
    X(vkCmdBindDescriptorSets)                   \
    X(vkCmdBindVertexBuffers)                    \
    X(vkCmdBindIndexBuffer)                      \
    X(vkCmdSetViewport)                          \
    X(vkCmdSetScissor)                           \
    X(vkCmdDraw)                                 \
    X(vkCmdDrawIndexed)                          \
    X(vkCmdCopyBuffer)                           \
    X(vkCmdCopyBufferToImage)                    \
    X(vkCmdCopyImageToBuffer)                    \
    X(vkCmdCopyImage)                            \
    X(vkCmdResolveImage)                         \
    X(vkCmdClearColorImage)                      \
    X(vkCmdFillBuffer)

    struct Functions
    {
#define ESIA_VK_DECLARE(name) PFN_##name name = nullptr;
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
        ESIA_VK_GLOBAL_FUNCTIONS(ESIA_VK_DECLARE)
        ESIA_VK_INSTANCE_FUNCTIONS(ESIA_VK_DECLARE)
        ESIA_VK_DEBUG_FUNCTIONS(ESIA_VK_DECLARE)
        ESIA_VK_DEVICE_FUNCTIONS(ESIA_VK_DECLARE)
        // dynamic rendering: core 1.3 or VK_KHR_dynamic_rendering (null when the device renders with render passes)
        PFN_vkCmdBeginRenderingKHR vkCmdBeginRendering = nullptr;
        PFN_vkCmdEndRenderingKHR vkCmdEndRendering = nullptr;
#undef ESIA_VK_DECLARE

        // Each returns false (and names the missing function in `error`) when a required entry point is absent.
        bool LoadGlobal(PFN_vkGetInstanceProcAddr getInstanceProcAddr, std::string& error);
        bool LoadInstance(VkInstance instance, std::string& error);
        bool LoadDevice(VkDevice device, bool dynamicRendering, bool core13, std::string& error);
    };

    // The system's loader library, opened at runtime. Null (and `error` set) when it is not installed.
    struct LoaderLibrary
    {
        void* handle = nullptr;
        PFN_vkGetInstanceProcAddr getInstanceProcAddr = nullptr;
    };
    LoaderLibrary OpenLoader(std::string& error);
    void CloseLoader(LoaderLibrary& lib);

    const char* ResultName(VkResult r);
}
