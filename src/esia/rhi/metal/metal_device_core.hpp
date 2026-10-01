// Esia - Metal backend: rhi::Device in plain C++ on top of the Gpu layer (metal_gpu.hpp).
//
// How RHI calls become Metal work:
//   * a device frame records into one MTLCommandBuffer (the host's, FrameDesc::nativeContext, or the backend's own
//     committed at EndFrame); a pass is a render command encoder, copies and uploads share a blit encoder opened
//     between passes;
//   * bindings are cached and applied at the next draw (setVertexBytes / setFragmentBytes for constants, textures
//     at 3..9, the FX buffer at 10, samplers at 11 / 12, the vertex buffer at 30); every pass and every host
//     callback starts with nothing bound, as the RHI says - a stale binding could name the new pass's target;
//   * textures live in private storage and are uploaded through staging buffers by blits in the frame's command
//     buffer, so an update never races a frame in flight that still samples the old contents; uploads made
//     outside a frame wait (as CPU copies) for the next command buffer;
//   * buffers are CPU-written shared buffers with one version per frame in flight (VersionRing);
//   * destroyed objects are released when the frames that may use them completed.
// Everything here is compiled and exercised on Linux by the tests (the renderer's conformance scenes through a
// fake Gpu that enforces Metal's rules); only metal_device.mm talks to Metal. UNVERIFIED: needs macOS.
#pragma once
#include "esia/rhi/backend_registry.hpp"
#include "metal_frames.hpp"
#include "metal_gpu.hpp"
#include "metal_planning.hpp"
#include "metal_profiler.hpp"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace esia::rhi::metal
{
    struct DeviceOptions
    {
        // GPU times per category (RenderStats::gpu) where the GPU samples timestamps at encoder boundaries.
        bool timestamps = true;
        // FX feature variants (Caps::fxFeatureVariants): -1 = when the Fx MSL declares the feature mask as a function
        // constant (MslDeclaresFxFeatures), 0 / 1 = off / on whatever the MSL (tests)
        int fxFeatureVariants = -1;
        // Called for every RHI contract violation or Metal failure the backend detects (also kept in Errors()).
        std::function<void(const std::string&)> onError;
    };

    class MetalDevice final : public Device
    {
    public:
        MetalDevice(std::unique_ptr<Gpu> gpu, const DeviceOptions& options);
        ~MetalDevice() override;

        // Host integration (include/esia/rhi/metal.hpp): a host texture as a render target / copy source; the same
        // native texture gives the same handle while its wrapper lives. DestroyTexture releases the wrapper only.
        Texture WrapTexture(void* nativeTexture);
        Gpu& GpuLayer() { return *gpu_; }
        const std::vector<std::string>& Errors() const { return errors_; }

        const char* Name() const override { return "metal"; }
        const Caps& GetCaps() const override { return caps_; }

        Texture CreateTexture(const TextureDesc& desc, const void* data, int rowPitch) override;
        void UpdateTexture(Texture tex, const IRect& rect, const void* data, int rowPitch) override;
        void DestroyTexture(Texture tex) override;
        TextureDesc GetTextureDesc(Texture tex) const override;
        Buffer CreateBuffer(const BufferDesc& desc) override;
        void UpdateBuffer(Buffer buf, const void* data, std::size_t size) override;
        void DestroyBuffer(Buffer buf) override;
        Pipeline CreatePipeline(const PipelineDesc& desc) override;
        PipelineStatus GetPipelineStatus(Pipeline p) const override;
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

    private:
        struct TextureRec
        {
            TextureDesc desc;                   // debugName dropped (not owned)
            std::uint32_t gpu = 0;
            std::uint32_t metalUsage = 0;
            std::uint32_t sampleView = 0;       // what shaders sample: gpu, or its raw view for sRGB formats
            bool framebufferOnly = false;
            const void* native = nullptr;       // host texture (WrapTexture)
            std::vector<std::pair<std::uint32_t, std::uint32_t>> views;   // pixel format -> view id
        };
        struct BufferRec
        {
            BufferDesc desc;
            VersionRing<std::uint32_t> ring;
            std::size_t valid = 0;              // bytes ever written (kept across versions)
        };
        struct PipelineRec
        {
            PipelineKey key;
            std::uint32_t pso = 0;
            Topology topology = Topology::TriangleList;
        };
        struct PendingUpload
        {
            std::uint32_t texture;
            IRect rect;
            std::vector<std::uint8_t> pixels;   // tightly packed rows
        };
        struct Doomed
        {
            bool texture;
            std::uint32_t id;
        };
        enum class Encoder : std::uint8_t { None, Render, Blit };
        static constexpr std::uint32_t kMaxConstantBytes = 256;

        void Error(const std::string& what);
        bool InFrame(const char* call);
        bool InPass(const char* call);
        TextureRec* FindTexture(Texture t);
        const TextureRec* FindTexture(Texture t) const;
        BufferRec* FindBuffer(Buffer b);
        std::uint32_t ViewOf(TextureRec& rec, std::uint32_t pixelFormat);
        void Doom(TextureRec& rec);
        void Collect();
        std::uint64_t Completed() const { return tracker_->CompletedPrefix(); }

        bool StartCommandBuffer(void* native);
        void EnsureBlit();
        void EndEncoder();
        bool StageUpload(TextureRec& rec, const IRect& rect, const void* data, std::size_t pitch);
        void FlushPendingUploads();
        std::uint32_t CurrentVersion(BufferRec& rec);
        TextureRec* ResolveScratch(const TextureRec& src);
        bool Resolve(TextureRec& src, TextureRec& scratch);
        void ResetBindings();
        bool PrepareDraw(const char* call);
        bool WaitForFramesInFlight();

        std::unique_ptr<Gpu> gpu_;
        DeviceOptions options_;
        Caps caps_;
        std::shared_ptr<FrameTracker> tracker_;
        StagingArena staging_;
        DeferredReleases<Doomed> doomed_;
        std::vector<std::string> errors_;

        std::uint32_t next_ = 1;
        std::unordered_map<std::uint32_t, TextureRec> textures_;
        std::unordered_map<std::uint32_t, BufferRec> buffers_;
        std::unordered_map<std::uint32_t, PipelineRec> pipelines_;
        std::unordered_map<PipelineKey, std::uint32_t, PipelineKeyHash> psoCache_;
        std::unordered_map<const void*, std::uint32_t> wrappers_;
        std::vector<PendingUpload> pending_;
        TextureRec scratch_;                    // MSAA resolve target (CopyTexture, ReadPixels)

        // frame state
        bool inFrame_ = false, passStarted_ = false, inPass_ = false, passOk_ = false;
        std::uint64_t serial_ = 0;              // command buffer being recorded (frames and readbacks)
        std::uint64_t frame_ = 0;               // device frames (GpuProfile::frame)
        Encoder encoder_ = Encoder::None;
        Texture passTarget_;

        // bindings of the current pass, applied at the next draw
        std::uint32_t pipeline_ = 0;
        bool pipelineDirty_ = false, samplersDirty_ = false, viewportDirty_ = false, scissorDirty_ = false, scissorEmpty_ = false;
        IRect scissor_;
        std::uint8_t constants_[3][kMaxConstantBytes] = {};
        std::uint32_t constantSize_[3] = {};
        bool constantDirty_[3] = {};
        Texture textureSlot_[kTextureSlots];
        std::uint32_t textureDirty_ = 0;
        Buffer fxBuffer_, vertexBuffer_, indexBuffer_;
        bool fxDirty_ = false, vertexDirty_ = false;

        // timestamps
        ProfileRecorder profiler_;
        TimestampClock clock_;
        std::vector<std::uint32_t> counterFree_;
        std::uint32_t counterCount_ = 0;
        bool profileScope_ = false;
    };

    // The headless device of the conformance suite (esia::rhi::BackendInfo::createHeadless) on any Gpu: a device and
    // an offscreen target of `desc` (RenderTarget | CopySrc, Sampled when sampleable and single-sampled).
    HeadlessDevice CreateHeadlessOn(std::unique_ptr<Gpu> gpu, const HeadlessDesc& desc, const DeviceOptions& options, std::string& error);
}
