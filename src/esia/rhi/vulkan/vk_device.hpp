// Esia - Vulkan backend: the rhi::Device implementation (private header; hosts use esia/rhi/vulkan.hpp).
//
// How the RHI maps onto Vulkan
//   * Layouts. Every image has one tracked layout. A sampleable texture rests in SHADER_READ_ONLY_OPTIMAL: the RHI
//     never samples a texture in the pass that renders it, so it only leaves that layout for its own pass
//     (COLOR_ATTACHMENT_OPTIMAL from BeginPass to EndPass), for copies (TRANSFER_*) and uploads, always outside a
//     render pass, and returns at the end of each. The barrier for a transition takes its source scope from the
//     layout it leaves (each layout implies the last access), so no per-draw bookkeeping is needed.
//   * Uploads (texture / buffer data, the zero-fill of new textures) are recorded into the frame's command buffer
//     before its first pass, through a staging ring of the frame slot, between one barrier after everything the GPU
//     did before and one before everything that follows. Updates outside a frame are kept on the CPU until the next
//     frame (or readback) records them, so they never wait and work when the host owns the command buffer.
//   * Constants go into a per-slot uniform ring and reach the shaders as dynamic uniform-buffer offsets; descriptor
//     sets (one layout for every program, set 0 of the shader library) come from per-slot pools and are only
//     rewritten when a texture or buffer binding changes.
//   * Pipelines are cached by PipelineDesc; passes use dynamic rendering when the device has it, else render passes
//     (per format / samples / load op) with a framebuffer per render target.
//   * Frames in flight: a slot per frame (rings, descriptor pools, timestamp queries, and - when the backend
//     submits itself - a command pool and a fence). Destroyed resources are released once the last frame that could
//     use them completed.
#pragma once
#include "vk_formats.hpp"
#include "vk_loader.hpp"
#include "esia/rhi/vulkan.hpp"
#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace esia::rhi::vulkan
{
    // Counts what the validation layer reports for a headless device (the messenger's user data: stable address).
    struct ValidationLog
    {
        std::atomic<std::uint32_t> messages{0};
    };

    // What the device owns besides the resources it created: headless devices own their instance and device.
    struct Ownership
    {
        LoaderLibrary loader;
        bool instance = false, device = false;
        VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
        std::unique_ptr<ValidationLog> log;
    };

    class VulkanDevice final : public Device
    {
    public:
        // `vk` has its global and instance functions loaded; the device functions are loaded here.
        static std::unique_ptr<VulkanDevice> Create(const Desc& desc, const Functions& vk, Ownership&& own, std::string& error);
        ~VulkanDevice() override;

        const char* Name() const override { return "vulkan"; }
        const Caps& GetCaps() const override { return caps_; }

        Texture CreateTexture(const TextureDesc& desc, const void* data, int rowPitch) override;
        void UpdateTexture(Texture tex, const IRect& rect, const void* data, int rowPitch) override;
        void DestroyTexture(Texture tex) override;
        TextureDesc GetTextureDesc(Texture tex) const override;
        Buffer CreateBuffer(const BufferDesc& desc) override;
        void UpdateBuffer(Buffer buf, const void* data, std::size_t size) override;
        void DestroyBuffer(Buffer buf) override;
        Pipeline CreatePipeline(const PipelineDesc& desc) override;
        void DestroyPipeline(Pipeline p) override;

        bool BeginFrame(const FrameDesc& desc) override;
        void EndFrame() override;
        void BeginPass(const PassDesc& desc) override;
        void EndPass() override;
        void SetPipeline(Pipeline p) override;
        void SetScissor(const IRect& r) override;
        void SetConstants(ConstantSlot slot, const void* data, std::uint32_t size) override;
        void SetTexture(int slot, Texture tex) override;
        void SetFxBuffer(Buffer buf) override;
        void SetVertexBuffer(Buffer buf) override;
        void SetIndexBuffer(Buffer buf) override;
        void Draw(std::uint32_t vertexCount, std::uint32_t firstVertex) override;
        void DrawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex) override;
        void DrawInstanced(std::uint32_t vertexCount, std::uint32_t instanceCount) override;
        void CopyTexture(Texture dst, int dstX, int dstY, Texture src, const IRect& srcRect) override;
        void* NativeRenderState() override;
        void BeginProfile(ProfileCategory category) override;
        void EndProfile() override;
        bool ReadProfile(GpuProfile& out) override;
        bool ReadPixels(Texture tex, const IRect& rect, std::vector<std::uint8_t>& rgba8) override;

        Texture Wrap(VkImage image, VkImageView view, VkFormat format, int width, int height, int samples, VkImageLayout entry,
                     VkImageLayout exit, std::uint32_t usage);
        bool DynamicRendering() const { return dynamicRendering_; }
        VkRenderPass RenderPass(VkFormat format, int samples, LoadOp load);
        std::uint32_t ValidationMessageCount() const { return own_.log ? own_.log->messages.load() : 0u; }

    private:
        VulkanDevice(const Desc& desc, const Functions& vk, Ownership&& own);
        bool Init(std::string& error);
        void ReleaseAll();

        struct Tex
        {
            TextureDesc desc;                 // as reported (debugName dropped: the pointer is the caller's)
            VkFormat format = VK_FORMAT_UNDEFINED;
            VkImage image = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;   // null for wrapped images
            VkImageView attachView = VK_NULL_HANDLE;  // render-target view (the host's for wrapped images)
            VkImageView sampleView = VK_NULL_HANDLE;  // raw (UNORM) view, when sampleable
            VkFramebuffer framebuffer = VK_NULL_HANDLE;   // render-pass path only
            VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
            bool wrapped = false;
            VkImageLayout entry = VK_IMAGE_LAYOUT_UNDEFINED, exit = VK_IMAGE_LAYOUT_UNDEFINED;
            std::uint64_t frameSeen = 0;      // wrapped: the frame whose entry layout `layout` started from
            std::uint64_t wrapFrame = 0;      // wrapped: the frame WrapImage was last called for
        };
        struct Buf
        {
            BufferDesc desc;
            VkBuffer buffer = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;
        };
        struct PipelineKey
        {
            ShaderProgram program;
            VertexLayout layout;
            Topology topology;
            BlendMode blend;
            Format format;
            int samples;
            bool operator==(const PipelineKey&) const = default;
        };
        struct PipelineKeyHash
        {
            std::size_t operator()(const PipelineKey& k) const
            {
                return std::hash<std::uint64_t>()((std::uint64_t)k.program | ((std::uint64_t)k.layout << 8) | ((std::uint64_t)k.topology << 16) |
                                                  ((std::uint64_t)k.blend << 24) | ((std::uint64_t)k.format << 32) | ((std::uint64_t)k.samples << 40));
            }
        };
        struct CachedPipeline
        {
            VkPipeline pipeline = VK_NULL_HANDLE;
            int refs = 0;
        };

        // A host-visible, persistently mapped buffer suballocated linearly and reset when its frame slot is reused.
        struct Chunk
        {
            VkBuffer buffer = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;
            std::uint8_t* mapped = nullptr;
            VkDeviceSize size = 0, used = 0;
        };
        struct Ring
        {
            std::vector<Chunk> chunks;
            VkBufferUsageFlags usage = 0;
            VkDeviceSize reserved = 0;        // zero bytes kept at the start of every chunk (default constants)
            VkDeviceSize minChunk = 0;
        };

        struct Scope
        {
            ProfileCategory category;
            std::uint32_t begin, end;
        };
        struct Slot
        {
            std::uint64_t frame = 0;          // the device frame that used this slot last
            VkCommandPool pool = VK_NULL_HANDLE;
            VkCommandBuffer cmd = VK_NULL_HANDLE;
            VkFence fence = VK_NULL_HANDLE;
            bool submitted = false;           // the fence will signal (the backend submitted this slot's frame)
            Ring staging, uniforms;
            std::vector<VkDescriptorPool> descriptorPools;
            std::size_t descriptorPool = 0;
            std::uint32_t setsLeft = 0;
            VkQueryPool queries = VK_NULL_HANDLE;
            std::uint32_t queriesUsed = 0;
            std::vector<Scope> scopes;
            bool timestampsPending = false;   // written, not read yet
        };

        struct PendingUpload
        {
            enum class Kind : std::uint8_t { TextureData, TextureClear, BufferData } kind;
            std::uint32_t id;
            IRect rect;
            std::vector<std::uint8_t> bytes;
        };

        // Released when the last frame that could use them completed.
        struct Garbage
        {
            std::uint64_t frame = 0;
            VkImage image = VK_NULL_HANDLE;
            VkImageView views[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
            VkFramebuffer framebuffer = VK_NULL_HANDLE;
            VkBuffer buffer = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;
            VkPipeline pipeline = VK_NULL_HANDLE;
        };

        // Collects image / memory barriers into one vkCmdPipelineBarrier.
        struct Barriers
        {
            VkPipelineStageFlags src = 0, dst = 0;
            std::vector<VkImageMemoryBarrier> images;
            VkMemoryBarrier memory{VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, 0, 0};
            bool hasMemory = false;
        };

        // ---- helpers
        std::uint32_t FindMemoryType(std::uint32_t typeBits, VkMemoryPropertyFlags required, VkMemoryPropertyFlags preferred) const;
        bool CreateBufferObject(VkDeviceSize size, VkBufferUsageFlags usage, bool hostVisible, VkBuffer& buffer, VkDeviceMemory& memory, void** mapped);
        void SetName(VkObjectType type, std::uint64_t handle, const char* name);
        Tex* FindTex(Texture t);
        const Tex* FindTex(Texture t) const;
        Buf* FindBuf(Buffer b);
        VkShaderModule Module(ShaderProgram program, int stage);
        VkPipeline BuildPipeline(const PipelineKey& key);
        VkFramebuffer Framebuffer(Tex& t);

        bool AllocChunk(Ring& ring, VkDeviceSize bytes);
        Chunk* Alloc(Ring& ring, VkDeviceSize bytes, VkDeviceSize align, VkDeviceSize& offset);
        void ResetRing(Ring& ring);
        void DestroyRing(Ring& ring);

        void AddTransition(Barriers& b, Tex& t, VkImageLayout layout, bool discard = false);
        void Submit(Barriers& b, VkCommandBuffer cmd);
        void Transition(VkCommandBuffer cmd, Tex& t, VkImageLayout layout, bool discard = false);
        void Rest(VkCommandBuffer cmd, Tex& t);
        void Touch(std::uint32_t id, Tex& t);
        void PrepareWrapped(std::uint32_t id, Tex& t);
        void MarkWrapped(std::uint32_t id, Tex& t);

        bool Recording() const { return inFrame_ && !passStarted_; }
        void BeginUploads(VkCommandBuffer cmd);
        void FlushUploads(VkCommandBuffer cmd);
        void TransferAfterTransfer(VkCommandBuffer cmd);
        void RecordTextureData(VkCommandBuffer cmd, Ring& staging, std::uint32_t id, const IRect& r, const void* data, int rowPitch);
        void RecordTextureClear(VkCommandBuffer cmd, std::uint32_t id);
        void RecordBufferData(VkCommandBuffer cmd, Ring& staging, std::uint32_t id, const void* data, std::size_t size);
        void RecordPending(VkCommandBuffer cmd, Ring& staging);

        VkDescriptorSet AllocateSet();
        bool PrepareDraw();
        void ResetPassBindings();
        Tex* Scratch(VkFormat format, int width, int height);
        void ResolveInto(VkCommandBuffer cmd, Tex& src, Tex& dst, const IRect& r, int dstX, int dstY);

        void Retire(Garbage g);
        void CollectGarbage();
        void Release(const Garbage& g);
        void ReleaseTex(Tex& t);
        bool ReadTimestamps(Slot& s);

        Functions vk_;
        Desc desc_;
        Ownership own_;
        Caps caps_;
        VkPhysicalDeviceProperties props_{};
        VkPhysicalDeviceMemoryProperties memory_{};
        bool dynamicRendering_ = false;
        VkDeviceSize uboAlign_ = 256, cbStride_ = 256;
        double timestampNs_ = 1.0;
        std::uint64_t timestampMask_ = ~0ull;

        VkSampler samplers_[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
        VkDescriptorSetLayout setLayout_ = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
        VkPipelineCache pipelineCache_ = VK_NULL_HANDLE;
        bool ownsPipelineCache_ = false;
        VkShaderModule modules_[(int)ShaderProgram::Count][2] = {};

        std::uint32_t nextId_ = 1;
        std::unordered_map<std::uint32_t, Tex> textures_;
        std::unordered_map<std::uint32_t, Buf> buffers_;
        std::unordered_map<PipelineKey, CachedPipeline, PipelineKeyHash> pipelineEntries_;
        std::unordered_map<std::uint32_t, PipelineKey> pipelines_;
        std::unordered_map<VkImage, std::uint32_t> wrapped_;
        std::map<std::uint64_t, VkRenderPass> renderPasses_;
        std::vector<Garbage> garbage_;
        std::vector<PendingUpload> pending_;
        Texture dummyTexture_, scratch_;
        Buffer dummyBuffer_;

        std::vector<Slot> slots_;
        Slot* slot_ = nullptr;
        std::uint64_t frame_ = 0, completedFrame_ = 0;
        bool inFrame_ = false, inPass_ = false, passStarted_ = false, ownsCommands_ = false;
        VkCommandBuffer cmd_ = VK_NULL_HANDLE;
        std::vector<std::uint32_t> wrappedThisFrame_;

        // readback outside frames
        VkCommandPool immediatePool_ = VK_NULL_HANDLE;
        VkCommandBuffer immediateCmd_ = VK_NULL_HANDLE;
        VkFence immediateFence_ = VK_NULL_HANDLE;
        Ring immediateStaging_;

        // uploads being recorded (between BeginUploads and FlushUploads)
        bool uploadsOpen_ = false;
        std::vector<std::uint32_t> uploadedImages_, uploadedBuffers_;

        // current pass
        std::uint32_t passTarget_ = 0;
        VkPipeline boundPipeline_ = VK_NULL_HANDLE;
        VkBuffer boundVertices_ = VK_NULL_HANDLE, boundIndices_ = VK_NULL_HANDLE;
        bool viewportSet_ = false;
        VkImageView views_[kSlotBackdrop0 + kBackdropLevels] = {};
        VkBuffer fxBuffer_ = VK_NULL_HANDLE;
        VkBuffer cbBuffer_[3] = {};
        std::uint32_t cbOffset_[3] = {};
        bool setDirty_ = true, offsetsDirty_ = true;
        VkDescriptorSet set_ = VK_NULL_HANDLE;
        int openScope_ = -1;
        ProfileCategory openCategory_ = ProfileCategory::Capture;

        GpuProfile latest_;
    };
}
