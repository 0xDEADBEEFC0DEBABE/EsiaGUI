// Metal backend tests: a fake Gpu (metal_gpu.hpp) that runs anywhere and enforces the Metal rules the backend
// relies on, so that MetalDevice can be driven by the real renderer on hosts without Metal:
//   * command buffers: one recording at a time, one encoder open at a time, blits and render passes never nested;
//     objects are not released while a command buffer that uses them can still run (the backend's deferred
//     releases must make this hold even for unretained host command buffers);
//   * render passes: attachments are render targets, resolves have matching formats / sizes, the pipeline's pixel
//     format and sample count match the attachment, scissors are non-empty and inside it;
//   * draws: everything the pipeline's MSL declares (parsed from the generated shaders) is bound - constants with at
//     least the struct's size, textures, samplers, the FX buffer, the vertex buffer at index 30 - with the right
//     primitive type; a sampled texture is never the pass's attachment, never multisampled, never read through an
//     sRGB format (the RHI's "sampling never decodes"), and has ShaderRead usage;
//   * blits: identical pixel formats (views count), no multisampled side, regions inside, no framebufferOnly
//     texture, row pitches / offsets that are multiples of the pixel size; views need PixelFormatView usage.
// Textures and buffers keep their bytes, so uploads, copies (with views), resolves and readback are executed on the
// CPU and can be checked; draws are validated but not rasterized.
// UNVERIFIED: needs macOS - the rules below are Metal's as documented, never compared with the Metal validation layer.
#pragma once
#include "metal_frames.hpp"
#include "metal_gpu.hpp"
#include "msl_interface.hpp"
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace esia::rhi::metal::test
{
    struct FakeGpuOptions
    {
        GpuCaps caps{16384, true, true};
        std::vector<int> sampleCounts{1, 2, 4, 8};
        bool autoComplete = true;   // the backend's own command buffers complete when committed
    };

    class FakeMetalGpu final : public Gpu
    {
    public:
        using Options = FakeGpuOptions;
        struct Stats
        {
            int commandBuffers = 0, renderPasses = 0, blitPasses = 0, resolves = 0, draws = 0, copies = 0, uploads = 0, readbacks = 0, views = 0;
            int pipelines = 0, liveTextures = 0, liveBuffers = 0;
        };
        // The state the fake shares with the test after the device took ownership of it.
        struct Shared
        {
            std::vector<std::string> errors;
            Stats stats;
        };

        explicit FakeMetalGpu(Options o = Options());
        ~FakeMetalGpu() override;
        std::shared_ptr<Shared> State() const { return shared_; }

        // test controls
        void CompleteAll();                 // every committed command buffer finishes
        void CommitHost(std::uint64_t serial);   // the host commits the command buffer of frame `serial`
        // A host texture for WrapTexture: returns the native pointer the backend is given.
        void* MakeHostTexture(const HostTextureInfo& info);
        const std::vector<std::uint8_t>* Pixels(std::uint32_t texture) const;

        GpuCaps Caps() const override { return options_.caps; }
        bool SupportsSampleCount(int samples) const override;
        std::uint32_t CreateTexture(const GpuTextureDesc& desc) override;
        std::uint32_t CreateTextureView(std::uint32_t texture, std::uint32_t pixelFormat) override;
        std::uint32_t AdoptTexture(void* native, HostTextureInfo& info) override;
        void ReleaseTexture(std::uint32_t texture) override;
        std::uint32_t CreateBuffer(std::size_t bytes, const char* label) override;
        void* BufferContents(std::uint32_t buffer) override;
        void ReleaseBuffer(std::uint32_t buffer) override;
        std::uint32_t CreatePipeline(const GpuPipelineDesc& desc, std::string& error) override;
        void ReleasePipeline(std::uint32_t pipeline) override;
        bool BeginCommandBuffer(void* native, std::uint64_t serial, const std::shared_ptr<FrameTracker>& tracker) override;
        void EndCommandBuffer() override;
        bool WaitForFrame(std::uint64_t serial) override;
        bool BeginRenderPass(const RenderPassDesc& desc) override;
        bool BeginBlitPass(const EncoderSamples& samples) override;
        void EndEncoder() override;
        void* NativeRenderEncoder() override;
        void SetPipeline(std::uint32_t pipeline) override;
        void SetViewport(int width, int height) override;
        void SetScissor(const IRect& r) override;
        void SetBytes(std::uint32_t stages, std::uint32_t index, const void* data, std::uint32_t size) override;
        void SetBuffer(std::uint32_t stages, std::uint32_t index, std::uint32_t buffer, std::size_t offset) override;
        void SetFragmentTexture(std::uint32_t index, std::uint32_t texture) override;
        void SetFragmentSamplers() override;
        void Draw(std::uint32_t primitive, std::uint32_t first, std::uint32_t count, std::uint32_t instances) override;
        void DrawIndexed(std::uint32_t count, std::uint32_t indexBuffer, std::size_t offset) override;
        void CopyTexture(std::uint32_t src, const IRect& srcRect, std::uint32_t dst, int dstX, int dstY) override;
        void CopyBufferToTexture(std::uint32_t buffer, std::size_t offset, std::size_t bytesPerRow, std::uint32_t dst, const IRect& dstRect) override;
        void CopyTextureToBuffer(std::uint32_t src, const IRect& srcRect, std::uint32_t buffer, std::size_t offset, std::size_t bytesPerRow) override;
        std::uint32_t CreateCounterBuffer(std::uint32_t samples) override;
        void ReleaseCounterBuffer(std::uint32_t buffer) override;
        bool ResolveCounters(std::uint32_t buffer, std::uint32_t first, std::uint32_t count, std::uint64_t* out) override;
        bool SampleClock(std::uint64_t& cpuNs, std::uint64_t& gpuTicks) override;

    private:
        struct Tex
        {
            GpuTextureDesc desc;
            std::uint32_t base = 0;             // a view's texture (0: this is a texture)
            bool live = true, framebufferOnly = false;
            std::vector<std::uint8_t> pixels;   // on the base texture only (a multisampled texture keeps one value per pixel)
        };
        struct Buf
        {
            std::vector<std::uint8_t> data;
            bool live = true;
        };
        struct Pso
        {
            GpuPipelineDesc desc;
            MslInterface vs, ps;
            bool live = true;
        };
        struct Cb
        {
            std::uint64_t serial = 0;
            bool host = false, committed = false, completed = false;
            std::shared_ptr<FrameTracker> tracker;
            std::set<std::uint32_t> textures, buffers, counters;
        };
        struct Bound
        {
            std::uint32_t size = 0;   // bytes (setBytes) or buffer bytes past the offset
        };

        void Error(const std::string& what);
        Tex* T(std::uint32_t id, const char* call);
        std::uint32_t BaseOf(std::uint32_t id) const;
        Buf* B(std::uint32_t id, const char* call);
        bool InUse(bool texture, std::uint32_t id) const;
        void Complete(Cb& cb);
        void UseTexture(std::uint32_t id);
        void UseBuffer(std::uint32_t id);
        bool UseSamples(const EncoderSamples& s, int expected);
        bool Blit(const char* call);
        bool Render(const char* call);
        void CheckDraw(const char* call, std::uint32_t primitive);

        Options options_;
        std::shared_ptr<Shared> shared_;
        std::vector<Tex> textures_;   // id - 1
        std::vector<Buf> buffers_;
        std::vector<Pso> psos_;
        std::vector<std::uint32_t> counters_;   // sample count per counter buffer (0 = released)
        std::map<std::uint64_t, Cb> cbs_;
        Cb* recording_ = nullptr;
        enum class Enc { None, Render, Blit } enc_ = Enc::None;
        std::uint32_t attachment_ = 0, pipeline_ = 0;
        std::map<std::uint32_t, Bound> vertexBound_, fragmentBound_;
        std::set<std::uint32_t> fragmentTextures_;
        bool samplers_ = false;
        std::vector<std::unique_ptr<HostTextureInfo>> hostTextures_;
        std::uint64_t clock_ = 0;
    };
}
