// Esia - Vulkan backend: the rhi::Device implementation (see vk_device.hpp)
#include "vk_device.hpp"
#include "esia/render/shader_library.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace esia::rhi::vulkan
{
    namespace
    {
        constexpr VkDeviceSize kConstantRange = 256;         // the largest constant block (Frame, 192 bytes) rounded up
        constexpr VkDeviceSize kStagingChunk = 1u << 20;
        constexpr VkDeviceSize kUniformChunk = 64u << 10;
        constexpr std::uint32_t kSetsPerPool = 256;
        constexpr std::uint32_t kMaxQueries = 2 + 2 * 256;   // frame start / end + 256 profile scopes
        constexpr VkPipelineStageFlags kShaderStages = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        VkDeviceSize AlignUp(VkDeviceSize v, VkDeviceSize a) { return (v + a - 1) / a * a; }

        // What a layout says about the last access to an image: the source scope of the barrier that leaves it.
        void SourceScope(VkImageLayout layout, VkPipelineStageFlags& stage, VkAccessFlags& access)
        {
            switch (layout)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED: stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; access = 0; break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: stage = kShaderStages; access = 0; break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: stage = VK_PIPELINE_STAGE_TRANSFER_BIT; access = 0; break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: stage = VK_PIPELINE_STAGE_TRANSFER_BIT; access = VK_ACCESS_TRANSFER_WRITE_BIT; break;
            default:   // a host layout (wrapped images): anything may have happened
                stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
                access = VK_ACCESS_MEMORY_WRITE_BIT;
                break;
            }
        }

        // ... and the destination scope of the barrier that enters it.
        void DestinationScope(VkImageLayout layout, VkPipelineStageFlags& stage, VkAccessFlags& access)
        {
            switch (layout)
            {
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                access = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: stage = kShaderStages; access = VK_ACCESS_SHADER_READ_BIT; break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: stage = VK_PIPELINE_STAGE_TRANSFER_BIT; access = VK_ACCESS_TRANSFER_READ_BIT; break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: stage = VK_PIPELINE_STAGE_TRANSFER_BIT; access = VK_ACCESS_TRANSFER_WRITE_BIT; break;
            default:   // handing a wrapped image back to the host: whatever it does next
                stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
                access = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
                break;
            }
        }

        VkSampleCountFlagBits SampleBits(int samples)
        {
            switch (samples)
            {
            case 2: return VK_SAMPLE_COUNT_2_BIT;
            case 4: return VK_SAMPLE_COUNT_4_BIT;
            case 8: return VK_SAMPLE_COUNT_8_BIT;
            case 16: return VK_SAMPLE_COUNT_16_BIT;
            default: return VK_SAMPLE_COUNT_1_BIT;
            }
        }

        VkAttachmentLoadOp ToLoadOp(LoadOp l)
        {
            switch (l)
            {
            case LoadOp::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
            case LoadOp::Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
            case LoadOp::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            }
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        }

        VkImageSubresourceRange ColorRange() { return VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}; }
        VkImageSubresourceLayers ColorLayers() { return VkImageSubresourceLayers{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}; }
    }

    // ------------------------------------------------------------------ creation
    VulkanDevice::VulkanDevice(const Desc& desc, const Functions& vk, Ownership&& own) : vk_(vk), desc_(desc), own_(std::move(own)) {}

    std::unique_ptr<VulkanDevice> VulkanDevice::Create(const Desc& desc, const Functions& vk, Ownership&& own, std::string& error)
    {
        std::unique_ptr<VulkanDevice> dev(new VulkanDevice(desc, vk, std::move(own)));
        if (!dev->Init(error))
            return nullptr;
        return dev;
    }

    bool VulkanDevice::Init(std::string& error)
    {
        if (!desc_.device || !desc_.physicalDevice || !desc_.queue)
        {
            error = "Desc needs a device, a physical device and a queue";
            return false;
        }
        if (desc_.apiVersion < VK_API_VERSION_1_1)
        {
            error = "the backend needs Vulkan 1.1";
            return false;
        }
        dynamicRendering_ = desc_.dynamicRendering;
        if (!vk_.LoadDevice(desc_.device, dynamicRendering_, desc_.apiVersion >= VK_API_VERSION_1_3, error))
            return false;
        if (!desc_.debugUtils)
            vk_.vkSetDebugUtilsObjectNameEXT = nullptr;

        vk_.vkGetPhysicalDeviceProperties(desc_.physicalDevice, &props_);
        vk_.vkGetPhysicalDeviceMemoryProperties(desc_.physicalDevice, &memory_);
        uboAlign_ = std::max<VkDeviceSize>(props_.limits.minUniformBufferOffsetAlignment, 16);
        cbStride_ = AlignUp(kConstantRange, uboAlign_);

        std::uint32_t familyCount = 0;
        vk_.vkGetPhysicalDeviceQueueFamilyProperties(desc_.physicalDevice, &familyCount, nullptr);
        std::vector<VkQueueFamilyProperties> families(familyCount);
        vk_.vkGetPhysicalDeviceQueueFamilyProperties(desc_.physicalDevice, &familyCount, families.data());
        if (desc_.queueFamily >= familyCount || !(families[desc_.queueFamily].queueFlags & VK_QUEUE_GRAPHICS_BIT))
        {
            error = "Desc::queueFamily is not a graphics queue family";
            return false;
        }
        const std::uint32_t timestampBits = families[desc_.queueFamily].timestampValidBits;
        timestampMask_ = timestampBits >= 64 ? ~0ull : ((1ull << timestampBits) - 1);
        timestampNs_ = props_.limits.timestampPeriod;

        VkFormatProperties half{};
        vk_.vkGetPhysicalDeviceFormatProperties(desc_.physicalDevice, VK_FORMAT_R16G16B16A16_SFLOAT, &half);
        const VkFormatFeatureFlags layerNeeds = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

        caps_.fxStorage = FxStorage::Buffer;
        caps_.shaderFormat = (std::uint8_t)shaders::Format::SpirV;
        caps_.framebufferOriginBottomLeft = false;
        caps_.clipSpaceYDown = true;   // no negative viewport: the renderer flips its projection instead
        caps_.halfPixelOffset = false;
        caps_.dualSourceBlend = desc_.dualSrcBlend;
        caps_.floatRenderTargets = (half.optimalTilingFeatures & layerNeeds) == layerNeeds;
        caps_.sampleRenderTarget = true;
        caps_.timestampQueries = timestampBits > 0 && timestampNs_ > 0.0;
        caps_.readback = true;
        caps_.runtimeEffects = false;
        caps_.fxFeatureVariants = false;
        caps_.maxTextureSize = (int)std::min<std::uint32_t>(props_.limits.maxImageDimension2D, 1u << 30);
        caps_.maxFxDataWidth = caps_.maxTextureSize;

        if (!shaders::Available(shaders::Format::SpirV))
        {
            error = "the shader library has no SPIR-V";
            return false;
        }

        // s0 linear clamp, s1 point clamp, one mip level: immutable in the set layout
        for (int i = 0; i < 2; ++i)
        {
            VkSamplerCreateInfo si{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
            si.magFilter = si.minFilter = i == 0 ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
            si.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            si.addressModeU = si.addressModeV = si.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            si.maxLod = 0.0f;
            if (vk_.vkCreateSampler(desc_.device, &si, nullptr, &samplers_[i]) != VK_SUCCESS)
            {
                error = "vkCreateSampler failed";
                return false;
            }
        }

        // set 0 of every program (docs/backends/README.md, "Binding numbers")
        VkDescriptorSetLayoutBinding bindings[13];
        for (std::uint32_t b = 0; b < 13; ++b)
        {
            bindings[b] = VkDescriptorSetLayoutBinding{};
            bindings[b].binding = b;
            bindings[b].descriptorCount = 1;
            bindings[b].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            bindings[b].descriptorType = b < 3    ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
                                         : b < 10 ? VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
                                         : b == 10 ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                                                   : VK_DESCRIPTOR_TYPE_SAMPLER;
        }
        bindings[11].pImmutableSamplers = &samplers_[0];
        bindings[12].pImmutableSamplers = &samplers_[1];
        VkDescriptorSetLayoutCreateInfo li{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        li.bindingCount = 13;
        li.pBindings = bindings;
        VkPipelineLayoutCreateInfo pli{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        pli.setLayoutCount = 1;
        pli.pSetLayouts = &setLayout_;
        if (vk_.vkCreateDescriptorSetLayout(desc_.device, &li, nullptr, &setLayout_) != VK_SUCCESS ||
            vk_.vkCreatePipelineLayout(desc_.device, &pli, nullptr, &pipelineLayout_) != VK_SUCCESS)
        {
            error = "cannot create the pipeline layout";
            return false;
        }

        pipelineCache_ = desc_.pipelineCache;
        if (!pipelineCache_)
        {
            VkPipelineCacheCreateInfo ci{VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
            if (vk_.vkCreatePipelineCache(desc_.device, &ci, nullptr, &pipelineCache_) != VK_SUCCESS)
                pipelineCache_ = VK_NULL_HANDLE;
            ownsPipelineCache_ = pipelineCache_ != VK_NULL_HANDLE;
        }

        slots_.resize((std::size_t)std::max(1, desc_.framesInFlight));
        for (Slot& s : slots_)
        {
            s.staging.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            s.staging.minChunk = kStagingChunk;
            s.uniforms.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            s.uniforms.reserved = cbStride_;
            s.uniforms.minChunk = kUniformChunk;
            if (caps_.timestampQueries)
            {
                VkQueryPoolCreateInfo qi{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
                qi.queryType = VK_QUERY_TYPE_TIMESTAMP;
                qi.queryCount = kMaxQueries;
                if (vk_.vkCreateQueryPool(desc_.device, &qi, nullptr, &s.queries) != VK_SUCCESS)
                    caps_.timestampQueries = false;
            }
        }
        immediateStaging_.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        immediateStaging_.minChunk = kStagingChunk;

        VkCommandPoolCreateInfo cpi{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        cpi.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        cpi.queueFamilyIndex = desc_.queueFamily;
        VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        if (vk_.vkCreateCommandPool(desc_.device, &cpi, nullptr, &immediatePool_) != VK_SUCCESS ||
            vk_.vkCreateFence(desc_.device, &fi, nullptr, &immediateFence_) != VK_SUCCESS)
        {
            error = "cannot create a command pool";
            return false;
        }
        VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        ai.commandPool = immediatePool_;
        ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        ai.commandBufferCount = 1;
        if (vk_.vkAllocateCommandBuffers(desc_.device, &ai, &immediateCmd_) != VK_SUCCESS)
        {
            error = "cannot allocate a command buffer";
            return false;
        }

        // what unbound slots hold: descriptor sets are always complete (no partially bound descriptors in 1.1)
        TextureDesc td;
        td.width = td.height = 1;
        td.debugName = "esia-dummy";
        dummyTexture_ = CreateTexture(td, nullptr, 0);
        BufferDesc bd;
        bd.kind = BufferKind::FxInstances;
        bd.size = 64;
        bd.debugName = "esia-dummy";
        dummyBuffer_ = CreateBuffer(bd);
        if (!dummyTexture_ || !dummyBuffer_)
        {
            error = "cannot create the backend's placeholder resources";
            return false;
        }
        const std::uint8_t zeros[64] = {};
        UpdateBuffer(dummyBuffer_, zeros, sizeof(zeros));
        ResetPassBindings();
        return true;
    }

    VulkanDevice::~VulkanDevice()
    {
        VkDevice d = desc_.device;
        if (d && vk_.vkQueueWaitIdle)   // device functions loaded (Init got that far)
            ReleaseAll();
        if (own_.device && d)
        {
            auto destroy = reinterpret_cast<PFN_vkDestroyDevice>(vk_.vkGetDeviceProcAddr(d, "vkDestroyDevice"));
            destroy(d, nullptr);
        }
        if (own_.messenger)
            vk_.vkDestroyDebugUtilsMessengerEXT(desc_.instance, own_.messenger, nullptr);
        if (own_.instance)
            vk_.vkDestroyInstance(desc_.instance, nullptr);
        CloseLoader(own_.loader);
    }

    void VulkanDevice::ReleaseAll()
    {
        VkDevice d = desc_.device;
        // the host guarantees the GPU is idle for its frames; the backend's own submissions are waited for here
        vk_.vkQueueWaitIdle(desc_.queue);
        for (auto& [id, t] : textures_)
            ReleaseTex(t);
        textures_.clear();
        for (auto& [id, b] : buffers_)
        {
            vk_.vkDestroyBuffer(d, b.buffer, nullptr);
            vk_.vkFreeMemory(d, b.memory, nullptr);
        }
        buffers_.clear();
        for (auto& [key, p] : pipelineEntries_)
            vk_.vkDestroyPipeline(d, p.pipeline, nullptr);
        for (const Garbage& g : garbage_)
            Release(g);
        for (auto& [key, rp] : renderPasses_)
            vk_.vkDestroyRenderPass(d, rp, nullptr);
        for (Slot& s : slots_)
        {
            DestroyRing(s.staging);
            DestroyRing(s.uniforms);
            for (VkDescriptorPool p : s.descriptorPools)
                vk_.vkDestroyDescriptorPool(d, p, nullptr);
            if (s.queries)
                vk_.vkDestroyQueryPool(d, s.queries, nullptr);
            if (s.fence)
                vk_.vkDestroyFence(d, s.fence, nullptr);
            if (s.pool)
                vk_.vkDestroyCommandPool(d, s.pool, nullptr);
        }
        DestroyRing(immediateStaging_);
        if (immediateFence_)
            vk_.vkDestroyFence(d, immediateFence_, nullptr);
        if (immediatePool_)
            vk_.vkDestroyCommandPool(d, immediatePool_, nullptr);
        for (auto& stages : modules_)
            for (VkShaderModule m : stages)
                if (m)
                    vk_.vkDestroyShaderModule(d, m, nullptr);
        if (ownsPipelineCache_)
            vk_.vkDestroyPipelineCache(d, pipelineCache_, nullptr);
        if (pipelineLayout_)
            vk_.vkDestroyPipelineLayout(d, pipelineLayout_, nullptr);
        if (setLayout_)
            vk_.vkDestroyDescriptorSetLayout(d, setLayout_, nullptr);
        for (VkSampler s : samplers_)
            if (s)
                vk_.vkDestroySampler(d, s, nullptr);
    }

    // ------------------------------------------------------------------ helpers
    std::uint32_t VulkanDevice::FindMemoryType(std::uint32_t typeBits, VkMemoryPropertyFlags required, VkMemoryPropertyFlags preferred) const
    {
        for (int pass = 0; pass < 2; ++pass)
        {
            const VkMemoryPropertyFlags want = pass == 0 ? (required | preferred) : required;
            for (std::uint32_t i = 0; i < memory_.memoryTypeCount; ++i)
                if ((typeBits & (1u << i)) && (memory_.memoryTypes[i].propertyFlags & want) == want)
                    return i;
        }
        return UINT32_MAX;
    }

    bool VulkanDevice::CreateBufferObject(VkDeviceSize size, VkBufferUsageFlags usage, bool hostVisible, VkBuffer& buffer, VkDeviceMemory& memory, void** mapped)
    {
        buffer = VK_NULL_HANDLE;
        memory = VK_NULL_HANDLE;
        VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bi.size = size;
        bi.usage = usage;
        bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vk_.vkCreateBuffer(desc_.device, &bi, nullptr, &buffer) != VK_SUCCESS)
            return false;
        VkMemoryRequirements req;
        vk_.vkGetBufferMemoryRequirements(desc_.device, buffer, &req);
        VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        ai.allocationSize = req.size;
        ai.memoryTypeIndex = hostVisible ? FindMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0)
                                         : FindMemoryType(req.memoryTypeBits, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (ai.memoryTypeIndex == UINT32_MAX || vk_.vkAllocateMemory(desc_.device, &ai, nullptr, &memory) != VK_SUCCESS ||
            vk_.vkBindBufferMemory(desc_.device, buffer, memory, 0) != VK_SUCCESS ||
            (mapped && vk_.vkMapMemory(desc_.device, memory, 0, VK_WHOLE_SIZE, 0, mapped) != VK_SUCCESS))
        {
            vk_.vkDestroyBuffer(desc_.device, buffer, nullptr);
            if (memory)
                vk_.vkFreeMemory(desc_.device, memory, nullptr);
            buffer = VK_NULL_HANDLE;
            memory = VK_NULL_HANDLE;
            return false;
        }
        return true;
    }

    void VulkanDevice::SetName(VkObjectType type, std::uint64_t handle, const char* name)
    {
        if (!vk_.vkSetDebugUtilsObjectNameEXT || !name)
            return;
        VkDebugUtilsObjectNameInfoEXT ni{VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
        ni.objectType = type;
        ni.objectHandle = handle;
        ni.pObjectName = name;
        vk_.vkSetDebugUtilsObjectNameEXT(desc_.device, &ni);
    }

    VulkanDevice::Tex* VulkanDevice::FindTex(Texture t)
    {
        auto it = textures_.find(t.id);
        return it != textures_.end() ? &it->second : nullptr;
    }

    const VulkanDevice::Tex* VulkanDevice::FindTex(Texture t) const
    {
        auto it = textures_.find(t.id);
        return it != textures_.end() ? &it->second : nullptr;
    }

    VulkanDevice::Buf* VulkanDevice::FindBuf(Buffer b)
    {
        auto it = buffers_.find(b.id);
        return it != buffers_.end() ? &it->second : nullptr;
    }

    // ------------------------------------------------------------------ rings
    bool VulkanDevice::AllocChunk(Ring& ring, VkDeviceSize bytes)
    {
        Chunk c;
        c.size = std::max(bytes, ring.minChunk);
        void* mapped = nullptr;
        if (!CreateBufferObject(c.size, ring.usage, true, c.buffer, c.memory, &mapped))
            return false;
        c.mapped = static_cast<std::uint8_t*>(mapped);
        std::memset(c.mapped, 0, (std::size_t)ring.reserved);
        c.used = ring.reserved;
        ring.chunks.push_back(c);
        return true;
    }

    VulkanDevice::Chunk* VulkanDevice::Alloc(Ring& ring, VkDeviceSize bytes, VkDeviceSize align, VkDeviceSize& offset)
    {
        if (!ring.chunks.empty())
        {
            Chunk& c = ring.chunks.back();
            const VkDeviceSize at = AlignUp(c.used, align);
            if (at + bytes <= c.size)
            {
                c.used = at + bytes;
                offset = at;
                return &c;
            }
        }
        // a full ring grows by a chunk; the next reset merges them into one of the total size
        const VkDeviceSize last = ring.chunks.empty() ? 0 : ring.chunks.back().size;
        if (!AllocChunk(ring, std::max(AlignUp(ring.reserved, align) + bytes, last * 2)))
            return nullptr;
        Chunk& c = ring.chunks.back();
        offset = AlignUp(c.used, align);
        c.used = offset + bytes;
        return &c;
    }

    void VulkanDevice::ResetRing(Ring& ring)
    {
        if (ring.chunks.size() > 1)
        {
            VkDeviceSize total = 0;
            for (const Chunk& c : ring.chunks)
                total += c.size;
            DestroyRing(ring);
            AllocChunk(ring, total);
        }
        for (Chunk& c : ring.chunks)
            c.used = ring.reserved;
    }

    void VulkanDevice::DestroyRing(Ring& ring)
    {
        for (Chunk& c : ring.chunks)
        {
            vk_.vkDestroyBuffer(desc_.device, c.buffer, nullptr);
            vk_.vkFreeMemory(desc_.device, c.memory, nullptr);
        }
        ring.chunks.clear();
    }

    // ------------------------------------------------------------------ layouts
    void VulkanDevice::AddTransition(Barriers& b, Tex& t, VkImageLayout layout, bool discard)
    {
        VkImageMemoryBarrier ib{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        VkPipelineStageFlags srcStage, dstStage;
        SourceScope(t.layout, srcStage, ib.srcAccessMask);
        DestinationScope(layout, dstStage, ib.dstAccessMask);
        ib.oldLayout = discard ? VK_IMAGE_LAYOUT_UNDEFINED : t.layout;
        ib.newLayout = layout;
        ib.srcQueueFamilyIndex = ib.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        ib.image = t.image;
        ib.subresourceRange = ColorRange();
        b.src |= srcStage;
        b.dst |= dstStage;
        b.images.push_back(ib);
        t.layout = layout;
    }

    void VulkanDevice::Submit(Barriers& b, VkCommandBuffer cmd)
    {
        if (b.images.empty() && !b.hasMemory)
            return;
        vk_.vkCmdPipelineBarrier(cmd, b.src, b.dst, 0, b.hasMemory ? 1u : 0u, b.hasMemory ? &b.memory : nullptr, 0, nullptr,
                                 (std::uint32_t)b.images.size(), b.images.data());
        b = Barriers();
    }

    void VulkanDevice::Transition(VkCommandBuffer cmd, Tex& t, VkImageLayout layout, bool discard)
    {
        Barriers b;
        AddTransition(b, t, layout, discard);
        Submit(b, cmd);
    }

    // Sampleable textures go back to SHADER_READ_ONLY after a pass, copy or upload: bound textures are always there.
    void VulkanDevice::Rest(VkCommandBuffer cmd, Tex& t)
    {
        if ((t.desc.usage & TextureUsage_Sampled) && t.layout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            Transition(cmd, t, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    // A wrapped image enters each frame in the layout the host promised.
    void VulkanDevice::Touch(std::uint32_t id, Tex& t)
    {
        if (!t.wrapped || !inFrame_ || t.frameSeen == frame_)
            return;
        t.frameSeen = frame_;
        t.layout = t.entry;
        wrappedThisFrame_.push_back(id);
    }

    // A sampleable wrapped image may be read before any Esia pass renders it (glass over what the host drew first:
    // the direct read samples the target), and textures are bound inside passes where no barrier can go. So the
    // frame's wrapped images enter SHADER_READ_ONLY at BeginFrame, like the backend's own resting textures.
    void VulkanDevice::PrepareWrapped(std::uint32_t id, Tex& t)
    {
        Touch(id, t);
        Rest(cmd_, t);
    }

    // ------------------------------------------------------------------ uploads
    void VulkanDevice::BeginUploads(VkCommandBuffer cmd)
    {
        if (uploadsOpen_)
            return;
        // after everything the GPU did before (earlier frames still read these resources): WAR and WAW hazards
        Barriers b;
        b.src = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        b.dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
        b.memory.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        b.memory.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        b.hasMemory = true;
        Submit(b, cmd);
        uploadsOpen_ = true;
    }

    // Two uploads to one resource in one block: the second copy waits for the first (write after write).
    void VulkanDevice::TransferAfterTransfer(VkCommandBuffer cmd)
    {
        Barriers b;
        b.src = b.dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
        b.memory.srcAccessMask = b.memory.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        b.hasMemory = true;
        Submit(b, cmd);
    }

    void VulkanDevice::FlushUploads(VkCommandBuffer cmd)
    {
        if (!uploadsOpen_)
            return;
        Barriers b;
        b.src = VK_PIPELINE_STAGE_TRANSFER_BIT;
        b.dst = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | kShaderStages | VK_PIPELINE_STAGE_TRANSFER_BIT;
        b.memory.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        b.memory.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                                 VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
        b.hasMemory = true;
        for (std::uint32_t id : uploadedImages_)
        {
            auto it = textures_.find(id);
            if (it != textures_.end() && (it->second.desc.usage & TextureUsage_Sampled) &&
                it->second.layout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                AddTransition(b, it->second, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        Submit(b, cmd);
        uploadedImages_.clear();
        uploadedBuffers_.clear();
        uploadsOpen_ = false;
    }

    void VulkanDevice::RecordTextureData(VkCommandBuffer cmd, Ring& staging, std::uint32_t id, const IRect& r, const void* data, int rowPitch)
    {
        auto it = textures_.find(id);
        if (it == textures_.end())
            return;
        Tex& t = it->second;
        const std::size_t bpp = (std::size_t)BytesPerPixel(t.desc.format);
        const std::size_t row = (std::size_t)r.Width() * bpp, pitch = rowPitch > 0 ? (std::size_t)rowPitch : row;
        VkDeviceSize offset = 0;
        Chunk* c = Alloc(staging, row * (std::size_t)r.Height(), 16, offset);
        if (!c)
            return;
        for (int y = 0; y < r.Height(); ++y)
            std::memcpy(c->mapped + offset + row * (std::size_t)y, static_cast<const std::uint8_t*>(data) + pitch * (std::size_t)y, row);
        BeginUploads(cmd);
        const bool whole = r.x0 == 0 && r.y0 == 0 && r.x1 == t.desc.width && r.y1 == t.desc.height;
        if (t.layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            Transition(cmd, t, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, whole);
        else
            TransferAfterTransfer(cmd);   // written already in this block (created with data, then updated)
        VkBufferImageCopy region{};
        region.bufferOffset = offset;
        region.imageSubresource = ColorLayers();
        region.imageOffset = {r.x0, r.y0, 0};
        region.imageExtent = {(std::uint32_t)r.Width(), (std::uint32_t)r.Height(), 1};
        vk_.vkCmdCopyBufferToImage(cmd, c->buffer, t.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        uploadedImages_.push_back(id);
    }

    void VulkanDevice::RecordTextureClear(VkCommandBuffer cmd, std::uint32_t id)
    {
        auto it = textures_.find(id);
        if (it == textures_.end())
            return;
        Tex& t = it->second;
        BeginUploads(cmd);
        Transition(cmd, t, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, true);
        const VkClearColorValue zero{};
        const VkImageSubresourceRange range = ColorRange();
        vk_.vkCmdClearColorImage(cmd, t.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &zero, 1, &range);
        uploadedImages_.push_back(id);
    }

    void VulkanDevice::RecordBufferData(VkCommandBuffer cmd, Ring& staging, std::uint32_t id, const void* data, std::size_t size)
    {
        Buf* b = FindBuf(Buffer{id});
        if (!b || size == 0)
            return;
        VkDeviceSize offset = 0;
        Chunk* c = Alloc(staging, size, 16, offset);
        if (!c)
            return;
        std::memcpy(c->mapped + offset, data, size);
        BeginUploads(cmd);
        if (std::find(uploadedBuffers_.begin(), uploadedBuffers_.end(), id) != uploadedBuffers_.end())
            TransferAfterTransfer(cmd);
        else
            uploadedBuffers_.push_back(id);
        const VkBufferCopy region{offset, 0, size};
        vk_.vkCmdCopyBuffer(cmd, c->buffer, b->buffer, 1, &region);
    }

    void VulkanDevice::RecordPending(VkCommandBuffer cmd, Ring& staging)
    {
        for (const PendingUpload& p : pending_)
        {
            switch (p.kind)
            {
            case PendingUpload::Kind::TextureData: RecordTextureData(cmd, staging, p.id, p.rect, p.bytes.data(), 0); break;
            case PendingUpload::Kind::TextureClear: RecordTextureClear(cmd, p.id); break;
            case PendingUpload::Kind::BufferData: RecordBufferData(cmd, staging, p.id, p.bytes.data(), p.bytes.size()); break;
            }
        }
        pending_.clear();
    }

    // ------------------------------------------------------------------ resources
    Texture VulkanDevice::CreateTexture(const TextureDesc& desc, const void* data, int rowPitch)
    {
        const VkFormat format = ToVkFormat(desc.format);
        if (desc.width <= 0 || desc.height <= 0 || desc.width > caps_.maxTextureSize || desc.height > caps_.maxTextureSize ||
            format == VK_FORMAT_UNDEFINED || desc.samples < 1)
            return {};
        const bool multisampled = desc.samples > 1;
        const bool sampled = (desc.usage & TextureUsage_Sampled) && !multisampled;   // shaders read 2D textures only
        if (multisampled && (data || (int)SampleBits(desc.samples) != desc.samples))
            return {};
        // transfers on every image: uploads, the zero-fill of new textures, copies, readback
        VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (sampled)
            usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (desc.usage & TextureUsage_RenderTarget)
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        const bool raw = sampled && RawVkFormat(format) != format;   // sRGB bits sampled through a UNORM view
        VkImageCreateInfo ii{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        ii.flags = raw ? VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT : 0;
        ii.imageType = VK_IMAGE_TYPE_2D;
        ii.format = format;
        ii.extent = {(std::uint32_t)desc.width, (std::uint32_t)desc.height, 1};
        ii.mipLevels = 1;
        ii.arrayLayers = 1;
        ii.samples = SampleBits(desc.samples);
        ii.tiling = VK_IMAGE_TILING_OPTIMAL;
        ii.usage = usage;
        ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageFormatProperties fp;
        if (vk_.vkGetPhysicalDeviceImageFormatProperties(desc_.physicalDevice, format, ii.imageType, ii.tiling, ii.usage, ii.flags, &fp) != VK_SUCCESS ||
            !(fp.sampleCounts & ii.samples) || fp.maxExtent.width < ii.extent.width || fp.maxExtent.height < ii.extent.height)
            return {};

        Tex t;
        t.desc = desc;
        t.desc.debugName = nullptr;
        if (!sampled)
            t.desc.usage &= ~(std::uint32_t)TextureUsage_Sampled;
        t.format = format;
        if (vk_.vkCreateImage(desc_.device, &ii, nullptr, &t.image) != VK_SUCCESS)
            return {};
        VkMemoryRequirements req;
        vk_.vkGetImageMemoryRequirements(desc_.device, t.image, &req);
        VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        ai.allocationSize = req.size;
        ai.memoryTypeIndex = FindMemoryType(req.memoryTypeBits, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        bool ok = ai.memoryTypeIndex != UINT32_MAX && vk_.vkAllocateMemory(desc_.device, &ai, nullptr, &t.memory) == VK_SUCCESS &&
                  vk_.vkBindImageMemory(desc_.device, t.image, t.memory, 0) == VK_SUCCESS;
        VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        vi.image = t.image;
        vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vi.subresourceRange = ColorRange();
        if (ok && (desc.usage & TextureUsage_RenderTarget))
        {
            vi.format = format;
            ok = vk_.vkCreateImageView(desc_.device, &vi, nullptr, &t.attachView) == VK_SUCCESS;
        }
        if (ok && sampled)
        {
            vi.format = RawVkFormat(format);
            ok = vk_.vkCreateImageView(desc_.device, &vi, nullptr, &t.sampleView) == VK_SUCCESS;
        }
        if (!ok)
        {
            ReleaseTex(t);
            return {};
        }
        SetName(VK_OBJECT_TYPE_IMAGE, (std::uint64_t)t.image, desc.debugName);

        const Texture handle{nextId_++};
        textures_[handle.id] = t;
        // new textures start defined (zeros), so every one can be bound as soon as it exists
        const IRect whole{0, 0, desc.width, desc.height};
        if (Recording())
        {
            if (data)
                RecordTextureData(cmd_, slot_->staging, handle.id, whole, data, rowPitch);
            else
                RecordTextureClear(cmd_, handle.id);
        }
        else if (data)
        {
            PendingUpload p{PendingUpload::Kind::TextureData, handle.id, whole, {}};
            const std::size_t row = (std::size_t)desc.width * (std::size_t)BytesPerPixel(desc.format), pitch = rowPitch > 0 ? (std::size_t)rowPitch : row;
            p.bytes.resize(row * (std::size_t)desc.height);
            for (int y = 0; y < desc.height; ++y)
                std::memcpy(p.bytes.data() + row * (std::size_t)y, static_cast<const std::uint8_t*>(data) + pitch * (std::size_t)y, row);
            pending_.push_back(std::move(p));
        }
        else
            pending_.push_back(PendingUpload{PendingUpload::Kind::TextureClear, handle.id, whole, {}});
        return handle;
    }

    Texture VulkanDevice::Wrap(VkImage image, VkImageView view, VkFormat format, int width, int height, int samples, VkImageLayout entry,
                               VkImageLayout exit, std::uint32_t usage)
    {
        const Format f = FromVkFormat(format);
        if (!image || !view || f == Format::Unknown || width <= 0 || height <= 0)
            return {};
        auto found = wrapped_.find(image);
        if (found != wrapped_.end())
        {
            Tex& t = textures_[found->second];
            if (t.attachView == view && t.format == format && t.desc.width == width && t.desc.height == height && t.desc.samples == samples &&
                t.desc.usage == ((usage | TextureUsage_RenderTarget) & ~(samples > 1 ? (std::uint32_t)TextureUsage_Sampled : 0u)))
            {
                t.entry = entry;
                t.exit = exit;
                MarkWrapped(found->second, t);
                return Texture{found->second};
            }
            DestroyTexture(Texture{found->second});   // recreated (a resized swap chain reuses handles)
        }
        Tex t;
        t.wrapped = true;
        t.image = image;
        t.attachView = view;
        t.format = format;
        t.entry = entry;
        t.exit = exit;
        t.layout = entry;
        t.desc.width = width;
        t.desc.height = height;
        t.desc.format = f;
        t.desc.samples = samples;
        t.desc.usage = usage | TextureUsage_RenderTarget;
        if (samples > 1)
            t.desc.usage &= ~(std::uint32_t)TextureUsage_Sampled;
        if (t.desc.usage & TextureUsage_Sampled)
        {
            VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            vi.image = image;
            vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
            vi.format = RawVkFormat(format);
            vi.subresourceRange = ColorRange();
            if (vk_.vkCreateImageView(desc_.device, &vi, nullptr, &t.sampleView) != VK_SUCCESS)
                t.desc.usage &= ~(std::uint32_t)TextureUsage_Sampled;
        }
        const Texture handle{nextId_++};
        wrapped_[image] = handle.id;
        MarkWrapped(handle.id, textures_[handle.id] = t);
        return handle;
    }

    // Wrapped for the current frame, or for the next one when called between frames (vulkan.hpp: once per frame).
    void VulkanDevice::MarkWrapped(std::uint32_t id, Tex& t)
    {
        t.wrapFrame = inFrame_ ? frame_ : frame_ + 1;
        if (inFrame_ && !inPass_ && (t.desc.usage & TextureUsage_Sampled))
            PrepareWrapped(id, t);
    }

    void VulkanDevice::UpdateTexture(Texture tex, const IRect& r, const void* data, int rowPitch)
    {
        Tex* t = FindTex(tex);
        if (!t || t->wrapped || !data || r.Empty() || r.x0 < 0 || r.y0 < 0 || r.x1 > t->desc.width || r.y1 > t->desc.height || t->desc.samples > 1)
            return;
        if (Recording())
        {
            RecordTextureData(cmd_, slot_->staging, tex.id, r, data, rowPitch);
            return;
        }
        PendingUpload p{PendingUpload::Kind::TextureData, tex.id, r, {}};
        const std::size_t row = (std::size_t)r.Width() * (std::size_t)BytesPerPixel(t->desc.format), pitch = rowPitch > 0 ? (std::size_t)rowPitch : row;
        p.bytes.resize(row * (std::size_t)r.Height());
        for (int y = 0; y < r.Height(); ++y)
            std::memcpy(p.bytes.data() + row * (std::size_t)y, static_cast<const std::uint8_t*>(data) + pitch * (std::size_t)y, row);
        pending_.push_back(std::move(p));
    }

    void VulkanDevice::ReleaseTex(Tex& t)
    {
        VkDevice d = desc_.device;
        if (t.framebuffer)
            vk_.vkDestroyFramebuffer(d, t.framebuffer, nullptr);
        if (t.sampleView)
            vk_.vkDestroyImageView(d, t.sampleView, nullptr);
        if (!t.wrapped)
        {
            if (t.attachView)
                vk_.vkDestroyImageView(d, t.attachView, nullptr);
            if (t.image)
                vk_.vkDestroyImage(d, t.image, nullptr);
            if (t.memory)
                vk_.vkFreeMemory(d, t.memory, nullptr);
        }
        t = Tex();
    }

    void VulkanDevice::DestroyTexture(Texture tex)
    {
        auto it = textures_.find(tex.id);
        if (it == textures_.end())
            return;
        Tex& t = it->second;
        Garbage g;
        g.framebuffer = t.framebuffer;
        g.views[0] = t.sampleView;
        if (!t.wrapped)
        {
            g.views[1] = t.attachView;
            g.image = t.image;
            g.memory = t.memory;
        }
        else
            wrapped_.erase(t.image);
        Retire(g);
        std::erase_if(pending_, [&](const PendingUpload& p) { return p.kind != PendingUpload::Kind::BufferData && p.id == tex.id; });
        std::erase(uploadedImages_, tex.id);
        std::erase(wrappedThisFrame_, tex.id);
        textures_.erase(it);
    }

    TextureDesc VulkanDevice::GetTextureDesc(Texture tex) const
    {
        const Tex* t = FindTex(tex);
        return t ? t->desc : TextureDesc{};
    }

    Buffer VulkanDevice::CreateBuffer(const BufferDesc& desc)
    {
        if (desc.size == 0)
            return {};
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        switch (desc.kind)
        {
        case BufferKind::Vertex: usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; break;
        case BufferKind::Index: usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT; break;
        case BufferKind::FxInstances: usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; break;
        }
        Buf b;
        b.desc = desc;
        b.desc.debugName = nullptr;
        if (!CreateBufferObject(desc.size, usage, false, b.buffer, b.memory, nullptr))
            return {};
        SetName(VK_OBJECT_TYPE_BUFFER, (std::uint64_t)b.buffer, desc.debugName);
        const Buffer handle{nextId_++};
        buffers_[handle.id] = b;
        return handle;
    }

    void VulkanDevice::UpdateBuffer(Buffer buf, const void* data, std::size_t size)
    {
        Buf* b = FindBuf(buf);
        if (!b || !data || size == 0 || size > b->desc.size)
            return;
        if (Recording())
            RecordBufferData(cmd_, slot_->staging, buf.id, data, size);
        else
            pending_.push_back(PendingUpload{PendingUpload::Kind::BufferData, buf.id, {},
                                             std::vector<std::uint8_t>(static_cast<const std::uint8_t*>(data), static_cast<const std::uint8_t*>(data) + size)});
    }

    void VulkanDevice::DestroyBuffer(Buffer buf)
    {
        auto it = buffers_.find(buf.id);
        if (it == buffers_.end())
            return;
        Garbage g;
        g.buffer = it->second.buffer;
        g.memory = it->second.memory;
        Retire(g);
        std::erase_if(pending_, [&](const PendingUpload& p) { return p.kind == PendingUpload::Kind::BufferData && p.id == buf.id; });
        std::erase(uploadedBuffers_, buf.id);
        buffers_.erase(it);
    }

    // ------------------------------------------------------------------ pipelines
    VkShaderModule VulkanDevice::Module(ShaderProgram program, int stage)
    {
        VkShaderModule& m = modules_[(int)program][stage];
        if (m)
            return m;
        const shaders::ShaderBlob* blob = shaders::Find(shaders::Format::SpirV, program, (shaders::Stage)stage);
        if (!blob || blob->size % 4 != 0)
            return VK_NULL_HANDLE;
        // the embedded bytes carry no alignment guarantee; pCode must be 4-byte aligned
        std::vector<std::uint32_t> code(blob->size / 4);
        std::memcpy(code.data(), blob->data, blob->size);
        VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        ci.codeSize = blob->size;
        ci.pCode = code.data();
        if (vk_.vkCreateShaderModule(desc_.device, &ci, nullptr, &m) != VK_SUCCESS)
            m = VK_NULL_HANDLE;
        return m;
    }

    VkPipeline VulkanDevice::BuildPipeline(const PipelineKey& k)
    {
        const shaders::ShaderBlob* vsBlob = shaders::Find(shaders::Format::SpirV, k.program, shaders::Stage::Vertex);
        const shaders::ShaderBlob* psBlob = shaders::Find(shaders::Format::SpirV, k.program, shaders::Stage::Pixel);
        const VkShaderModule vs = Module(k.program, 0), ps = Module(k.program, 1);
        const VkFormat format = ToVkFormat(k.format);
        if (!vs || !ps || !vsBlob || !psBlob || format == VK_FORMAT_UNDEFINED)
            return VK_NULL_HANDLE;
        VkPipelineShaderStageCreateInfo stages[2] = {{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO}, {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO}};
        stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = vs;
        stages[0].pName = vsBlob->entry;
        stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = ps;
        stages[1].pName = psBlob->entry;

        // esia::Vertex: float2 pos, float2 uv, RGBA8 color (20 bytes)
        const VkVertexInputBindingDescription binding{0, 20, VK_VERTEX_INPUT_RATE_VERTEX};
        const VkVertexInputAttributeDescription attributes[3] = {
            {0, 0, VK_FORMAT_R32G32_SFLOAT, 0}, {1, 0, VK_FORMAT_R32G32_SFLOAT, 8}, {2, 0, VK_FORMAT_R8G8B8A8_UNORM, 16}};
        VkPipelineVertexInputStateCreateInfo vertexInput{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        if (k.layout == VertexLayout::UiVertex)
        {
            vertexInput.vertexBindingDescriptionCount = 1;
            vertexInput.pVertexBindingDescriptions = &binding;
            vertexInput.vertexAttributeDescriptionCount = 3;
            vertexInput.pVertexAttributeDescriptions = attributes;
        }
        VkPipelineInputAssemblyStateCreateInfo assembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        assembly.topology = k.topology == Topology::TriangleStrip ? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkPipelineViewportStateCreateInfo viewport{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        viewport.viewportCount = 1;
        viewport.scissorCount = 1;
        VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        raster.polygonMode = VK_POLYGON_MODE_FILL;
        raster.cullMode = VK_CULL_MODE_NONE;
        raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        raster.lineWidth = 1.0f;
        VkPipelineMultisampleStateCreateInfo multisample{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        multisample.rasterizationSamples = SampleBits(k.samples);
        VkPipelineColorBlendAttachmentState blend{};
        blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blend.colorBlendOp = blend.alphaBlendOp = VK_BLEND_OP_ADD;
        switch (k.blend)
        {
        case BlendMode::Opaque: break;
        case BlendMode::Straight:
            blend.blendEnable = VK_TRUE;
            blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            break;
        case BlendMode::Premultiplied:
            blend.blendEnable = VK_TRUE;
            blend.srcColorBlendFactor = blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            blend.dstColorBlendFactor = blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            break;
        case BlendMode::DualSourceLcd:
            blend.blendEnable = VK_TRUE;
            blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC1_COLOR;
            blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
            blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC1_ALPHA;
            blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
            break;
        }
        VkPipelineColorBlendStateCreateInfo blendState{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        blendState.attachmentCount = 1;
        blendState.pAttachments = &blend;
        const VkDynamicState dynamics[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamic{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dynamic.dynamicStateCount = 2;
        dynamic.pDynamicStates = dynamics;

        VkPipelineRenderingCreateInfoKHR rendering{VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR};
        rendering.colorAttachmentCount = 1;
        rendering.pColorAttachmentFormats = &format;
        VkGraphicsPipelineCreateInfo pi{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        pi.stageCount = 2;
        pi.pStages = stages;
        pi.pVertexInputState = &vertexInput;
        pi.pInputAssemblyState = &assembly;
        pi.pViewportState = &viewport;
        pi.pRasterizationState = &raster;
        pi.pMultisampleState = &multisample;
        pi.pColorBlendState = &blendState;
        pi.pDynamicState = &dynamic;
        pi.layout = pipelineLayout_;
        if (dynamicRendering_)
            pi.pNext = &rendering;
        else if (!(pi.renderPass = RenderPass(format, k.samples, LoadOp::Load)))
            return VK_NULL_HANDLE;
        VkPipeline p = VK_NULL_HANDLE;
        if (vk_.vkCreateGraphicsPipelines(desc_.device, pipelineCache_, 1, &pi, nullptr, &p) != VK_SUCCESS)
            return VK_NULL_HANDLE;
        return p;
    }

    Pipeline VulkanDevice::CreatePipeline(const PipelineDesc& d)
    {
        // no runtime compilation here (Caps::runtimeEffects / fxFeatureVariants are false): the renderer falls back
        if (d.effect != 0 || d.fxFeatures != 0 || d.program >= ShaderProgram::Count)
            return {};
        if (d.blend == BlendMode::DualSourceLcd && !caps_.dualSourceBlend)
            return {};
        if (d.samples > 1 && (props_.limits.framebufferColorSampleCounts & SampleBits(d.samples)) == 0)
            return {};
        const PipelineKey key{d.program, d.layout, d.topology, d.blend, d.targetFormat, d.samples};
        CachedPipeline& c = pipelineEntries_[key];
        if (!c.pipeline)
        {
            c.pipeline = BuildPipeline(key);
            c.format = d.targetFormat;
            c.samples = d.samples;
            if (!c.pipeline)
            {
                pipelineEntries_.erase(key);
                return {};
            }
        }
        ++c.refs;
        const Pipeline handle{nextId_++};
        pipelines_[handle.id] = key;
        return handle;
    }

    void VulkanDevice::DestroyPipeline(Pipeline p)
    {
        auto it = pipelines_.find(p.id);
        if (it == pipelines_.end())
            return;
        auto c = pipelineEntries_.find(it->second);
        if (c != pipelineEntries_.end() && --c->second.refs == 0)
        {
            Garbage g;
            g.pipeline = c->second.pipeline;
            Retire(g);
            pipelineEntries_.erase(c);
        }
        pipelines_.erase(it);
    }

    VkRenderPass VulkanDevice::RenderPass(VkFormat format, int samples, LoadOp load)
    {
        const std::uint64_t key = (std::uint64_t)format | ((std::uint64_t)samples << 32) | ((std::uint64_t)load << 40);
        auto it = renderPasses_.find(key);
        if (it != renderPasses_.end())
            return it->second;
        // Layout transitions are explicit barriers around the pass (the same as with dynamic rendering), so the
        // attachment stays in COLOR_ATTACHMENT_OPTIMAL from start to end.
        VkAttachmentDescription a{};
        a.format = format;
        a.samples = SampleBits(samples);
        a.loadOp = ToLoadOp(load);
        a.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        a.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        a.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        a.initialLayout = a.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        const VkAttachmentReference ref{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkSubpassDescription sub{};
        sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sub.colorAttachmentCount = 1;
        sub.pColorAttachments = &ref;
        VkRenderPassCreateInfo ci{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        ci.attachmentCount = 1;
        ci.pAttachments = &a;
        ci.subpassCount = 1;
        ci.pSubpasses = &sub;
        VkRenderPass rp = VK_NULL_HANDLE;
        if (vk_.vkCreateRenderPass(desc_.device, &ci, nullptr, &rp) != VK_SUCCESS)
            return VK_NULL_HANDLE;
        renderPasses_[key] = rp;
        return rp;
    }

    VkFramebuffer VulkanDevice::Framebuffer(Tex& t)
    {
        if (t.framebuffer)
            return t.framebuffer;
        VkFramebufferCreateInfo ci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        ci.renderPass = RenderPass(t.format, t.desc.samples, LoadOp::Load);   // compatible with every load op
        ci.attachmentCount = 1;
        ci.pAttachments = &t.attachView;
        ci.width = (std::uint32_t)t.desc.width;
        ci.height = (std::uint32_t)t.desc.height;
        ci.layers = 1;
        if (!ci.renderPass || vk_.vkCreateFramebuffer(desc_.device, &ci, nullptr, &t.framebuffer) != VK_SUCCESS)
            t.framebuffer = VK_NULL_HANDLE;
        return t.framebuffer;
    }

    // ------------------------------------------------------------------ frames
    bool VulkanDevice::BeginFrame(const FrameDesc& desc)
    {
        if (inFrame_)
            return false;
        Slot& s = slots_[(std::size_t)((frame_ + 1) % slots_.size())];
        // the frame that used this slot before is done: the fence says so, or the host promised it (vulkan.hpp)
        if (s.submitted)
        {
            vk_.vkWaitForFences(desc_.device, 1, &s.fence, VK_TRUE, UINT64_MAX);
            vk_.vkResetFences(desc_.device, 1, &s.fence);
            s.submitted = false;
        }
        completedFrame_ = std::max(completedFrame_, s.frame);
        ReadTimestamps(s);
        CollectGarbage();

        ++frame_;
        slot_ = &s;
        s.frame = frame_;
        ResetRing(s.staging);
        ResetRing(s.uniforms);
        if (s.uniforms.chunks.empty() && !AllocChunk(s.uniforms, kUniformChunk))
            return false;
        for (VkDescriptorPool p : s.descriptorPools)
            vk_.vkResetDescriptorPool(desc_.device, p, 0);
        s.descriptorPool = 0;
        s.setsLeft = s.descriptorPools.empty() ? 0 : kSetsPerPool;

        cmd_ = static_cast<VkCommandBuffer>(desc.nativeContext);
        ownsCommands_ = cmd_ == VK_NULL_HANDLE;
        if (ownsCommands_)
        {
            if (!s.pool)
            {
                VkCommandPoolCreateInfo cpi{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
                cpi.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
                cpi.queueFamilyIndex = desc_.queueFamily;
                VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
                ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                ai.commandBufferCount = 1;
                VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
                if (vk_.vkCreateCommandPool(desc_.device, &cpi, nullptr, &s.pool) != VK_SUCCESS)
                    return false;
                ai.commandPool = s.pool;
                if (vk_.vkAllocateCommandBuffers(desc_.device, &ai, &s.cmd) != VK_SUCCESS || vk_.vkCreateFence(desc_.device, &fi, nullptr, &s.fence) != VK_SUCCESS)
                    return false;
            }
            vk_.vkResetCommandPool(desc_.device, s.pool, 0);
            VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
            bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vk_.vkBeginCommandBuffer(s.cmd, &bi);
            cmd_ = s.cmd;
        }
        inFrame_ = true;
        inPass_ = passStarted_ = false;
        wrappedThisFrame_.clear();
        s.scopes.clear();
        s.queriesUsed = 0;
        openScope_ = -1;
        if (caps_.timestampQueries)
        {
            vk_.vkCmdResetQueryPool(cmd_, s.queries, 0, kMaxQueries);
            vk_.vkCmdWriteTimestamp(cmd_, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, s.queries, s.queriesUsed++);
        }
        RecordPending(cmd_, s.staging);
        for (const auto& [image, id] : wrapped_)
        {
            Tex& t = textures_[id];
            if (t.wrapFrame == frame_ && (t.desc.usage & TextureUsage_Sampled))
                PrepareWrapped(id, t);
        }
        return true;
    }

    void VulkanDevice::EndFrame()
    {
        if (!inFrame_)
            return;
        if (inPass_)
            EndPass();
        if (openScope_ >= 0)
            EndProfile();
        FlushUploads(cmd_);
        // wrapped images go back to the host in the layout it asked for
        Barriers b;
        for (std::uint32_t id : wrappedThisFrame_)
        {
            auto it = textures_.find(id);
            if (it != textures_.end() && it->second.layout != it->second.exit)
                AddTransition(b, it->second, it->second.exit);
        }
        Submit(b, cmd_);
        Slot& s = *slot_;
        if (caps_.timestampQueries)
        {
            vk_.vkCmdWriteTimestamp(cmd_, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, s.queries, s.queriesUsed++);
            s.timestampsPending = true;
        }
        if (ownsCommands_)
        {
            vk_.vkEndCommandBuffer(cmd_);
            VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
            si.commandBufferCount = 1;
            si.pCommandBuffers = &cmd_;
            s.submitted = vk_.vkQueueSubmit(desc_.queue, 1, &si, s.fence) == VK_SUCCESS;
        }
        inFrame_ = false;
        cmd_ = VK_NULL_HANDLE;
    }

    void VulkanDevice::ResetPassBindings()
    {
        const Tex* dummy = FindTex(dummyTexture_);
        for (VkImageView& v : views_)
            v = dummy ? dummy->sampleView : VK_NULL_HANDLE;
        const auto b = buffers_.find(dummyBuffer_.id);
        fxBuffer_ = b != buffers_.end() ? b->second.buffer : VK_NULL_HANDLE;
        // unset constants read the zero block at the start of the uniform ring
        const VkBuffer zeros = slot_ && !slot_->uniforms.chunks.empty() ? slot_->uniforms.chunks[0].buffer : VK_NULL_HANDLE;
        for (int i = 0; i < 3; ++i)
        {
            cbBuffer_[i] = zeros;
            cbOffset_[i] = 0;
        }
        boundPipeline_ = VK_NULL_HANDLE;
        boundVertices_ = boundIndices_ = VK_NULL_HANDLE;
        viewportSet_ = false;
        setDirty_ = offsetsDirty_ = true;
        set_ = VK_NULL_HANDLE;
    }

    void VulkanDevice::BeginPass(const PassDesc& d)
    {
        Tex* t = FindTex(d.target);
        if (!inFrame_ || inPass_ || !t || !(t->desc.usage & TextureUsage_RenderTarget))
            return;
        FlushUploads(cmd_);
        Touch(d.target.id, *t);
        Transition(cmd_, *t, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, d.load != LoadOp::Load);
        VkClearValue clear{};
        std::memcpy(clear.color.float32, d.clearColor, sizeof(clear.color.float32));
        const VkRect2D area{{0, 0}, {(std::uint32_t)t->desc.width, (std::uint32_t)t->desc.height}};
        if (dynamicRendering_)
        {
            VkRenderingAttachmentInfoKHR color{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR};
            color.imageView = t->attachView;
            color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            color.loadOp = ToLoadOp(d.load);
            color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            color.clearValue = clear;
            VkRenderingInfoKHR ri{VK_STRUCTURE_TYPE_RENDERING_INFO_KHR};
            ri.renderArea = area;
            ri.layerCount = 1;
            ri.colorAttachmentCount = 1;
            ri.pColorAttachments = &color;
            vk_.vkCmdBeginRendering(cmd_, &ri);
        }
        else
        {
            VkRenderPassBeginInfo bi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            bi.renderPass = RenderPass(t->format, t->desc.samples, d.load);
            bi.framebuffer = Framebuffer(*t);
            bi.renderArea = area;
            bi.clearValueCount = 1;
            bi.pClearValues = &clear;
            if (!bi.renderPass || !bi.framebuffer)
                return;
            vk_.vkCmdBeginRenderPass(cmd_, &bi, VK_SUBPASS_CONTENTS_INLINE);
        }
        inPass_ = passStarted_ = true;
        passTarget_ = d.target.id;
        ResetPassBindings();
        vk_.vkCmdSetScissor(cmd_, 0, 1, &area);
    }

    void VulkanDevice::EndPass()
    {
        if (!inPass_)
            return;
        if (dynamicRendering_)
            vk_.vkCmdEndRendering(cmd_);
        else
            vk_.vkCmdEndRenderPass(cmd_);
        inPass_ = false;
        if (Tex* t = FindTex(Texture{passTarget_}))
            Rest(cmd_, *t);   // sampleable targets are read right after their pass (captures, pyramid levels)
        passTarget_ = 0;
    }

    // ------------------------------------------------------------------ state
    void VulkanDevice::SetPipeline(Pipeline p)
    {
        auto it = pipelines_.find(p.id);
        if (!inPass_ || it == pipelines_.end())
            return;
        const CachedPipeline& c = pipelineEntries_[it->second];
        if (c.pipeline != boundPipeline_)
        {
            vk_.vkCmdBindPipeline(cmd_, VK_PIPELINE_BIND_POINT_GRAPHICS, c.pipeline);
            boundPipeline_ = c.pipeline;
        }
    }

    void VulkanDevice::SetScissor(const IRect& r)
    {
        if (!inPass_)
            return;
        const VkRect2D s{{std::max(0, r.x0), std::max(0, r.y0)}, {(std::uint32_t)std::max(0, r.Width()), (std::uint32_t)std::max(0, r.Height())}};
        vk_.vkCmdSetScissor(cmd_, 0, 1, &s);
    }

    void VulkanDevice::SetConstants(ConstantSlot slot, const void* data, std::uint32_t size)
    {
        if (!inFrame_ || !data || size == 0 || size > kConstantRange)
            return;
        VkDeviceSize offset = 0;
        // a whole stride per block: the descriptor's range (kConstantRange) stays inside the chunk at every offset
        Chunk* c = Alloc(slot_->uniforms, cbStride_, uboAlign_, offset);
        if (!c)
            return;
        std::memcpy(c->mapped + offset, data, size);
        const int i = (int)slot;
        if (cbBuffer_[i] != c->buffer)
            setDirty_ = true;
        cbBuffer_[i] = c->buffer;
        cbOffset_[i] = (std::uint32_t)offset;
        offsetsDirty_ = true;
    }

    void VulkanDevice::SetTexture(int slot, Texture tex)
    {
        if (slot < 0 || slot >= kSlotBackdrop0 + kBackdropLevels)
            return;   // t7 is the FX storage buffer on this backend (SetFxBuffer)
        const Tex* t = FindTex(tex);
        const Tex* dummy = FindTex(dummyTexture_);
        const VkImageView v = t && t->sampleView ? t->sampleView : dummy->sampleView;
        if (views_[slot] != v)
        {
            views_[slot] = v;
            setDirty_ = true;
        }
    }

    void VulkanDevice::SetFxBuffer(Buffer buf)
    {
        const Buf* b = FindBuf(buf);
        const VkBuffer v = b && b->desc.kind == BufferKind::FxInstances ? b->buffer : FindBuf(dummyBuffer_)->buffer;
        if (fxBuffer_ != v)
        {
            fxBuffer_ = v;
            setDirty_ = true;
        }
    }

    void VulkanDevice::SetVertexBuffer(Buffer buf)
    {
        const Buf* b = FindBuf(buf);
        if (!inFrame_ || !b || b->buffer == boundVertices_)
            return;
        const VkDeviceSize offset = 0;
        vk_.vkCmdBindVertexBuffers(cmd_, 0, 1, &b->buffer, &offset);
        boundVertices_ = b->buffer;
    }

    void VulkanDevice::SetIndexBuffer(Buffer buf)
    {
        const Buf* b = FindBuf(buf);
        if (!inFrame_ || !b || b->buffer == boundIndices_)
            return;
        vk_.vkCmdBindIndexBuffer(cmd_, b->buffer, 0, VK_INDEX_TYPE_UINT32);
        boundIndices_ = b->buffer;
    }

    VkDescriptorSet VulkanDevice::AllocateSet()
    {
        Slot& s = *slot_;
        if (s.setsLeft == 0)
        {
            if (!s.descriptorPools.empty())
                ++s.descriptorPool;
            if (s.descriptorPool >= s.descriptorPools.size())
            {
                const VkDescriptorPoolSize sizes[4] = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 3 * kSetsPerPool},
                                                       {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 7 * kSetsPerPool},
                                                       {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, kSetsPerPool},
                                                       {VK_DESCRIPTOR_TYPE_SAMPLER, 2 * kSetsPerPool}};
                VkDescriptorPoolCreateInfo ci{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
                ci.maxSets = kSetsPerPool;
                ci.poolSizeCount = 4;
                ci.pPoolSizes = sizes;
                VkDescriptorPool pool = VK_NULL_HANDLE;
                if (vk_.vkCreateDescriptorPool(desc_.device, &ci, nullptr, &pool) != VK_SUCCESS)
                    return VK_NULL_HANDLE;
                s.descriptorPools.push_back(pool);
                s.descriptorPool = s.descriptorPools.size() - 1;
            }
            s.setsLeft = kSetsPerPool;
        }
        VkDescriptorSetAllocateInfo ai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        ai.descriptorPool = s.descriptorPools[s.descriptorPool];
        ai.descriptorSetCount = 1;
        ai.pSetLayouts = &setLayout_;
        VkDescriptorSet set = VK_NULL_HANDLE;
        if (vk_.vkAllocateDescriptorSets(desc_.device, &ai, &set) != VK_SUCCESS)
            return VK_NULL_HANDLE;
        --s.setsLeft;
        return set;
    }

    bool VulkanDevice::PrepareDraw()
    {
        if (!inPass_ || !boundPipeline_)
            return false;
        if (!viewportSet_)
        {
            const Tex* t = FindTex(Texture{passTarget_});
            const VkViewport vp{0.0f, 0.0f, (float)t->desc.width, (float)t->desc.height, 0.0f, 1.0f};
            vk_.vkCmdSetViewport(cmd_, 0, 1, &vp);
            viewportSet_ = true;
        }
        if (setDirty_)
        {
            const VkDescriptorSet set = AllocateSet();
            if (!set)
                return false;
            VkDescriptorBufferInfo cbs[3], fx{fxBuffer_, 0, VK_WHOLE_SIZE};
            VkDescriptorImageInfo images[kSlotBackdrop0 + kBackdropLevels];
            VkWriteDescriptorSet writes[3 + kSlotBackdrop0 + kBackdropLevels + 1];
            std::uint32_t n = 0;
            for (std::uint32_t i = 0; i < 3; ++i)
            {
                cbs[i] = VkDescriptorBufferInfo{cbBuffer_[i], 0, kConstantRange};
                writes[n] = VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                writes[n].dstSet = set;
                writes[n].dstBinding = i;
                writes[n].descriptorCount = 1;
                writes[n].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                writes[n++].pBufferInfo = &cbs[i];
            }
            for (std::uint32_t i = 0; i < (std::uint32_t)(kSlotBackdrop0 + kBackdropLevels); ++i)
            {
                images[i] = VkDescriptorImageInfo{VK_NULL_HANDLE, views_[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
                writes[n] = VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                writes[n].dstSet = set;
                writes[n].dstBinding = 3 + i;
                writes[n].descriptorCount = 1;
                writes[n].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                writes[n++].pImageInfo = &images[i];
            }
            writes[n] = VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            writes[n].dstSet = set;
            writes[n].dstBinding = 10;
            writes[n].descriptorCount = 1;
            writes[n].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writes[n++].pBufferInfo = &fx;
            vk_.vkUpdateDescriptorSets(desc_.device, n, writes, 0, nullptr);
            set_ = set;
            setDirty_ = false;
            offsetsDirty_ = true;
        }
        if (offsetsDirty_)
        {
            vk_.vkCmdBindDescriptorSets(cmd_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, 1, &set_, 3, cbOffset_);
            offsetsDirty_ = false;
        }
        return true;
    }

    void VulkanDevice::Draw(std::uint32_t vertexCount, std::uint32_t firstVertex)
    {
        if (PrepareDraw())
            vk_.vkCmdDraw(cmd_, vertexCount, 1, firstVertex, 0);
    }

    void VulkanDevice::DrawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex)
    {
        if (boundVertices_ && boundIndices_ && PrepareDraw())
            vk_.vkCmdDrawIndexed(cmd_, indexCount, 1, firstIndex, 0, 0);
    }

    void VulkanDevice::DrawInstanced(std::uint32_t vertexCount, std::uint32_t instanceCount)
    {
        if (PrepareDraw())
            vk_.vkCmdDraw(cmd_, vertexCount, instanceCount, 0, 0);
    }

    void* VulkanDevice::NativeRenderState()
    {
        if (!inPass_)
            return nullptr;
        // the host may bind anything: bind everything again before the next draw
        boundPipeline_ = VK_NULL_HANDLE;
        boundVertices_ = boundIndices_ = VK_NULL_HANDLE;
        viewportSet_ = false;
        offsetsDirty_ = true;
        return cmd_;
    }

    // ------------------------------------------------------------------ copies
    VulkanDevice::Tex* VulkanDevice::Scratch(VkFormat format, int width, int height)
    {
        Tex* s = FindTex(scratch_);
        if (s && s->format == format && s->desc.width >= width && s->desc.height >= height)
            return s;
        if (s)
            DestroyTexture(scratch_);
        TextureDesc d;
        d.width = std::max(width, s ? s->desc.width : 0);
        d.height = std::max(height, s ? s->desc.height : 0);
        d.format = FromVkFormat(format);
        d.usage = TextureUsage_CopySrc | TextureUsage_CopyDst;
        d.debugName = "esia-resolve";
        // created without a zero-fill: every use writes the region it reads first
        const bool recording = Recording();
        const std::size_t pendingBefore = pending_.size();
        scratch_ = CreateTexture(d, nullptr, 0);
        if (recording)
            std::erase(uploadedImages_, scratch_.id);
        else
            pending_.erase(pending_.begin() + (std::ptrdiff_t)pendingBefore, pending_.end());
        return FindTex(scratch_);
    }

    // Resolves `r` of a multisampled `src` to (dstX, dstY) of `dst` (same format), both already in TRANSFER layouts.
    void VulkanDevice::ResolveInto(VkCommandBuffer cmd, Tex& src, Tex& dst, const IRect& r, int dstX, int dstY)
    {
        VkImageResolve region{};
        region.srcSubresource = region.dstSubresource = ColorLayers();
        region.srcOffset = {r.x0, r.y0, 0};
        region.dstOffset = {dstX, dstY, 0};
        region.extent = {(std::uint32_t)r.Width(), (std::uint32_t)r.Height(), 1};
        vk_.vkCmdResolveImage(cmd, src.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    void VulkanDevice::CopyTexture(Texture dstTex, int dstX, int dstY, Texture srcTex, const IRect& r)
    {
        Tex* dst = FindTex(dstTex);
        Tex* src = FindTex(srcTex);
        if (!inFrame_ || inPass_ || !dst || !src || dst == src || r.Empty() || dst->desc.samples != 1 ||
            RawVkFormat(src->format) != RawVkFormat(dst->format))
            return;
        FlushUploads(cmd_);
        Touch(srcTex.id, *src);
        Touch(dstTex.id, *dst);
        const bool whole = dstX == 0 && dstY == 0 && r.Width() == dst->desc.width && r.Height() == dst->desc.height;
        if (src->desc.samples > 1 && src->format != dst->format)
        {
            // vkCmdResolveImage needs identical formats: resolve into a scratch image of the source's format, then
            // copy the bits (sRGB -> UNORM is a raw copy between size-compatible formats)
            Tex* scratch = Scratch(src->format, src->desc.width, src->desc.height);
            if (!scratch)
                return;
            src = FindTex(srcTex);
            dst = FindTex(dstTex);
            Barriers b;
            AddTransition(b, *src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            AddTransition(b, *scratch, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, true);
            Submit(b, cmd_);
            ResolveInto(cmd_, *src, *scratch, r, r.x0, r.y0);
            AddTransition(b, *scratch, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            AddTransition(b, *dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, whole);
            Submit(b, cmd_);
            src = scratch;
        }
        else
        {
            Barriers b;
            AddTransition(b, *src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            AddTransition(b, *dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, whole);
            Submit(b, cmd_);
        }
        if (src->desc.samples > 1)
            ResolveInto(cmd_, *src, *dst, r, dstX, dstY);
        else
        {
            VkImageCopy region{};
            region.srcSubresource = region.dstSubresource = ColorLayers();
            region.srcOffset = {r.x0, r.y0, 0};
            region.dstOffset = {dstX, dstY, 0};
            region.extent = {(std::uint32_t)r.Width(), (std::uint32_t)r.Height(), 1};
            vk_.vkCmdCopyImage(cmd_, src->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        }
        Rest(cmd_, *dst);
        if (Tex* s = FindTex(srcTex))
            Rest(cmd_, *s);
    }

    // ------------------------------------------------------------------ profiling
    void VulkanDevice::BeginProfile(ProfileCategory category)
    {
        if (!caps_.timestampQueries || !inFrame_ || openScope_ >= 0 || slot_->queriesUsed + 3 > kMaxQueries)
            return;   // the last query of the pool is kept for the frame's end
        openScope_ = (int)slot_->queriesUsed++;
        openCategory_ = category;
        vk_.vkCmdWriteTimestamp(cmd_, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, slot_->queries, (std::uint32_t)openScope_);
    }

    void VulkanDevice::EndProfile()
    {
        if (!inFrame_ || openScope_ < 0)
            return;
        const std::uint32_t end = slot_->queriesUsed++;
        vk_.vkCmdWriteTimestamp(cmd_, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, slot_->queries, end);
        slot_->scopes.push_back(Scope{openCategory_, (std::uint32_t)openScope_, end});
        openScope_ = -1;
    }

    bool VulkanDevice::ReadTimestamps(Slot& s)
    {
        if (!s.timestampsPending)
            return false;
        std::vector<std::uint64_t> t(s.queriesUsed);
        // without VK_QUERY_RESULT_WAIT_BIT: VK_NOT_READY while the frame runs, never a stall
        if (vk_.vkGetQueryPoolResults(desc_.device, s.queries, 0, s.queriesUsed, t.size() * sizeof(std::uint64_t), t.data(), sizeof(std::uint64_t),
                                      VK_QUERY_RESULT_64_BIT) != VK_SUCCESS)
            return false;
        s.timestampsPending = false;
        auto ms = [&](std::uint32_t a, std::uint32_t b) { return (float)((double)((t[b] - t[a]) & timestampMask_) * timestampNs_ * 1e-6); };
        GpuProfile p;
        p.valid = true;
        p.frame = s.frame;
        p.totalMs = ms(0, s.queriesUsed - 1);
        for (const Scope& sc : s.scopes)
            p.categoryMs[(int)sc.category] += ms(sc.begin, sc.end);
        if (!latest_.valid || p.frame > latest_.frame)
            latest_ = p;
        return true;
    }

    bool VulkanDevice::ReadProfile(GpuProfile& out)
    {
        // frames the backend submitted itself can be polled; the host's are read when their slot comes back
        for (Slot& s : slots_)
            if (s.submitted)
                ReadTimestamps(s);
        out = latest_;
        return latest_.valid;
    }

    // ------------------------------------------------------------------ readback
    bool VulkanDevice::ReadPixels(Texture tex, const IRect& r, std::vector<std::uint8_t>& rgba8)
    {
        rgba8.clear();
        Tex* t = FindTex(tex);
        if (inFrame_ || !t || r.Empty() || r.x0 < 0 || r.y0 < 0 || r.x1 > t->desc.width || r.y1 > t->desc.height)
            return false;
        VkCommandBuffer cmd = immediateCmd_;
        vk_.vkResetCommandPool(desc_.device, immediatePool_, 0);
        VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vk_.vkBeginCommandBuffer(cmd, &bi);
        RecordPending(cmd, immediateStaging_);
        FlushUploads(cmd);

        t = FindTex(tex);
        const VkImageLayout before = t->layout;
        Tex* src = t;
        IRect read = r;
        if (t->desc.samples > 1)
        {
            Tex* scratch = Scratch(t->format, t->desc.width, t->desc.height);
            t = FindTex(tex);
            if (!scratch)
            {
                vk_.vkEndCommandBuffer(cmd);
                return false;
            }
            Barriers b;
            AddTransition(b, *t, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            AddTransition(b, *scratch, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, true);
            Submit(b, cmd);
            ResolveInto(cmd, *t, *scratch, r, 0, 0);
            src = scratch;
            read = IRect{0, 0, r.Width(), r.Height()};
        }
        Transition(cmd, *src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        const std::size_t bpp = (std::size_t)BytesPerPixel(t->desc.format), count = (std::size_t)r.Width() * (std::size_t)r.Height();
        VkBuffer buffer;
        VkDeviceMemory memory;
        void* mapped = nullptr;
        if (!CreateBufferObject(count * bpp, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true, buffer, memory, &mapped))
        {
            vk_.vkEndCommandBuffer(cmd);
            return false;
        }
        VkBufferImageCopy region{};
        region.imageSubresource = ColorLayers();
        region.imageOffset = {read.x0, read.y0, 0};
        region.imageExtent = {(std::uint32_t)r.Width(), (std::uint32_t)r.Height(), 1};
        vk_.vkCmdCopyImageToBuffer(cmd, src->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &region);
        VkBufferMemoryBarrier host{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
        host.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        host.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
        host.srcQueueFamilyIndex = host.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        host.buffer = buffer;
        host.size = VK_WHOLE_SIZE;
        vk_.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 1, &host, 0, nullptr);
        // a wrapped image goes back to the layout the host left it in; the backend's own images rest as usual
        if (t->wrapped)
        {
            if (t->layout != before)
                Transition(cmd, *t, before);
        }
        else
            Rest(cmd, *t);
        vk_.vkEndCommandBuffer(cmd);

        VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        si.commandBufferCount = 1;
        si.pCommandBuffers = &cmd;
        bool ok = vk_.vkQueueSubmit(desc_.queue, 1, &si, immediateFence_) == VK_SUCCESS &&
                  vk_.vkWaitForFences(desc_.device, 1, &immediateFence_, VK_TRUE, UINT64_MAX) == VK_SUCCESS;
        vk_.vkResetFences(desc_.device, 1, &immediateFence_);
        ResetRing(immediateStaging_);
        if (ok)
        {
            rgba8.resize(count * 4);
            ConvertToRgba8(t->desc.format, mapped, count, rgba8.data());
        }
        vk_.vkDestroyBuffer(desc_.device, buffer, nullptr);
        vk_.vkFreeMemory(desc_.device, memory, nullptr);
        // the fence covers everything submitted to the queue before: the backend's own frames are complete
        for (Slot& s : slots_)
            if (s.submitted)
                completedFrame_ = std::max(completedFrame_, s.frame);
        CollectGarbage();
        if (own_.log && own_.log->messages.load() != 0)
        {
            // a test device: a validation message fails the test that reads the result
            std::fprintf(stderr, "esia vulkan: %u validation message(s) so far - readback refused\n", own_.log->messages.load());
            rgba8.clear();
            return false;
        }
        return ok;
    }

    // ------------------------------------------------------------------ deferred release
    void VulkanDevice::Retire(Garbage g)
    {
        g.frame = frame_;   // the latest frame that may still use it (frame_ is 0 before the first frame)
        if (g.frame <= completedFrame_ && !inFrame_)
            Release(g);
        else
            garbage_.push_back(g);
    }

    void VulkanDevice::CollectGarbage()
    {
        std::erase_if(garbage_, [&](const Garbage& g) {
            if (g.frame > completedFrame_)
                return false;
            Release(g);
            return true;
        });
    }

    void VulkanDevice::Release(const Garbage& g)
    {
        VkDevice d = desc_.device;
        if (g.pipeline)
            vk_.vkDestroyPipeline(d, g.pipeline, nullptr);
        if (g.framebuffer)
            vk_.vkDestroyFramebuffer(d, g.framebuffer, nullptr);
        for (VkImageView v : g.views)
            if (v)
                vk_.vkDestroyImageView(d, v, nullptr);
        if (g.image)
            vk_.vkDestroyImage(d, g.image, nullptr);
        if (g.buffer)
            vk_.vkDestroyBuffer(d, g.buffer, nullptr);
        if (g.memory)
            vk_.vkFreeMemory(d, g.memory, nullptr);
    }
}
