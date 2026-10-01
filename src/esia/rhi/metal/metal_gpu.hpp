// Esia - Metal backend: the thin layer between the portable device logic and Metal.
//
// MetalDevice (metal_device_core.cpp) implements rhi::Device in plain C++: state tracking, buffer versions, upload
// staging, copy plans, readback, deferred releases, profiling. Everything it asks of Metal goes through Gpu, one
// virtual per Metal call it makes, with objects named by small integer ids. metal_device.mm implements Gpu with
// Metal (MTLDevice, MTLCommandBuffer, encoders); the tests implement it with a fake that enforces Metal's rules
// (tests/fake_metal_gpu.cpp), so the logic above runs - through the real renderer - on hosts without Metal.
//
// Conventions: ids are never 0; every call names Metal's own API in its comment; rectangles are top-left based
// pixels (Metal's convention too). UNVERIFIED: needs macOS.
#pragma once
#include "metal_profiler.hpp"
#include "metal_tables.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace esia::rhi::metal
{
    class FrameTracker;

    struct GpuTextureDesc
    {
        int width = 0, height = 0;
        std::uint32_t pixelFormat = mtl::PixelFormatInvalid;
        int samples = 1;            // > 1: MTLTextureType2DMultisample
        std::uint32_t usage = 0;    // MTLTextureUsage
        const char* label = nullptr;
    };

    // What the backend learns about a host texture (-[MTLTexture pixelFormat] ...).
    struct HostTextureInfo
    {
        int width = 0, height = 0;
        std::uint32_t pixelFormat = mtl::PixelFormatInvalid;
        int samples = 1;
        std::uint32_t usage = 0;
        bool framebufferOnly = false;
        bool is2D = true;           // MTLTextureType2D or 2DMultisample, one mip level is all the renderer uses
    };

    struct GpuPipelineDesc
    {
        ShaderProgram program = ShaderProgram::UiGeometry;
        const char* vertexSource = nullptr;     // MSL (the shader library), entry point esia_main
        const char* fragmentSource = nullptr;
        bool uiVertexLayout = false;            // vertex descriptor: kUiVertexAttributes at binding::kVertexBuffer
        std::uint32_t pixelFormat = mtl::PixelFormatInvalid;
        int samples = 1;
        BlendState blend;
        // Programs whose MSL declares the FX feature mask as function constant 0 (MslDeclaresFxFeatures): the value
        // it is specialized to, 0 = every feature. Ignored by the other programs.
        std::uint32_t fxFeatures = 0;
    };

    struct RenderPassDesc
    {
        std::uint32_t target = 0;               // colorAttachments[0].texture
        std::uint32_t loadAction = mtl::LoadActionLoad;
        std::uint32_t storeAction = mtl::StoreActionStore;
        float clearColor[4] = {0, 0, 0, 0};
        std::uint32_t resolveTarget = 0;        // with StoreActionStoreAndMultisampleResolve
        EncoderSamples samples;                 // sampleBufferAttachments[0] (start / end of vertex / fragment)
        const char* label = nullptr;
    };

    enum StageMask : std::uint32_t { StageVertex = 1, StageFragment = 2 };

    struct GpuCaps
    {
        int maxTextureSize = 8192;
        bool timestamps = false;                // stage-boundary timestamp sampling (MTLCommonCounterSetTimestamp)
        bool asyncPipelines = false;            // CreatePipelineAsync compiles off the calling thread
    };

    class Gpu
    {
    public:
        virtual ~Gpu() = default;
        virtual GpuCaps Caps() const = 0;
        virtual bool SupportsSampleCount(int samples) const = 0;   // -[MTLDevice supportsTextureSampleCount:]

        // ---- objects
        virtual std::uint32_t CreateTexture(const GpuTextureDesc& desc) = 0;   // MTLStorageModePrivate
        virtual std::uint32_t CreateTextureView(std::uint32_t texture, std::uint32_t pixelFormat) = 0;   // newTextureViewWithPixelFormat:
        // Registers a host texture (id<MTLTexture> as void*); 0 if it is not usable.
        virtual std::uint32_t AdoptTexture(void* native, HostTextureInfo& info) = 0;
        virtual void ReleaseTexture(std::uint32_t texture) = 0;
        virtual std::uint32_t CreateBuffer(std::size_t bytes, const char* label) = 0;   // MTLStorageModeShared
        virtual void* BufferContents(std::uint32_t buffer) = 0;
        virtual void ReleaseBuffer(std::uint32_t buffer) = 0;
        // newLibraryWithSource (MSL 2.0) + newRenderPipelineStateWithDescriptor; 0 and `error` on failure
        virtual std::uint32_t CreatePipeline(const GpuPipelineDesc& desc, std::string& error) = 0;
        // The same off the calling thread (GpuCaps::asyncPipelines): an id at once, PipelineStatusOf says when it can
        // be bound. Without the cap it compiles here (a failure: PipelineStatus::Failed, its error in `error`).
        virtual std::uint32_t CreatePipelineAsync(const GpuPipelineDesc& desc, std::string& error) { return CreatePipeline(desc, error); }
        // Ready for pipelines CreatePipeline made; Pending while an asynchronous one compiles, then Ready or Failed
        // (with its error in `error`, reported once). 0 (a failed synchronous compile) is Failed.
        virtual PipelineStatus PipelineStatusOf(std::uint32_t pipeline, std::string& error)
        {
            (void)error;
            return pipeline ? PipelineStatus::Ready : PipelineStatus::Failed;
        }
        virtual void ReleasePipeline(std::uint32_t pipeline) = 0;

        // ---- command buffers: one per device frame (the host's, or the backend's own) and per readback
        // Starts recording into `native` (id<MTLCommandBuffer>) or a new command buffer of the backend's queue;
        // either way adds a completed handler that calls tracker->Complete(serial).
        virtual bool BeginCommandBuffer(void* native, std::uint64_t serial, const std::shared_ptr<FrameTracker>& tracker) = 0;
        // Ends recording: commits the backend's own command buffer (a host's is committed by the host).
        virtual void EndCommandBuffer() = 0;
        // Waits until frame `serial` completed; false if its command buffer was never committed (a host's that is
        // still recording): waiting would never return.
        virtual bool WaitForFrame(std::uint64_t serial) = 0;

        // ---- encoders (one open at a time)
        virtual bool BeginRenderPass(const RenderPassDesc& desc) = 0;   // renderCommandEncoderWithDescriptor:
        virtual bool BeginBlitPass(const EncoderSamples& samples) = 0;  // blitCommandEncoderWithDescriptor:
        virtual void EndEncoder() = 0;                                  // endEncoding
        virtual void* NativeRenderEncoder() = 0;                        // the id<MTLRenderCommandEncoder>

        // ---- render encoder
        virtual void SetPipeline(std::uint32_t pipeline) = 0;
        virtual void SetViewport(int width, int height) = 0;            // origin 0, depth 0..1
        virtual void SetScissor(const IRect& r) = 0;                    // setScissorRect: (non-empty)
        virtual void SetBytes(std::uint32_t stages, std::uint32_t index, const void* data, std::uint32_t size) = 0;
        virtual void SetBuffer(std::uint32_t stages, std::uint32_t index, std::uint32_t buffer, std::size_t offset) = 0;
        virtual void SetFragmentTexture(std::uint32_t index, std::uint32_t texture) = 0;
        virtual void SetFragmentSamplers() = 0;   // linear / point clamp at binding::kSamplerLinear / kSamplerPoint
        virtual void Draw(std::uint32_t primitive, std::uint32_t first, std::uint32_t count, std::uint32_t instances) = 0;
        virtual void DrawIndexed(std::uint32_t count, std::uint32_t indexBuffer, std::size_t offset) = 0;   // UInt32, triangles

        // ---- blit encoder
        virtual void CopyTexture(std::uint32_t src, const IRect& srcRect, std::uint32_t dst, int dstX, int dstY) = 0;
        virtual void CopyBufferToTexture(std::uint32_t buffer, std::size_t offset, std::size_t bytesPerRow, std::uint32_t dst, const IRect& dstRect) = 0;
        virtual void CopyTextureToBuffer(std::uint32_t src, const IRect& srcRect, std::uint32_t buffer, std::size_t offset, std::size_t bytesPerRow) = 0;

        // ---- timestamps (GpuCaps::timestamps)
        virtual std::uint32_t CreateCounterBuffer(std::uint32_t samples) = 0;   // MTLCounterSampleBuffer, shared
        virtual void ReleaseCounterBuffer(std::uint32_t buffer) = 0;
        virtual bool ResolveCounters(std::uint32_t buffer, std::uint32_t first, std::uint32_t count, std::uint64_t* out) = 0;
        virtual bool SampleClock(std::uint64_t& cpuNs, std::uint64_t& gpuTicks) = 0;   // sampleTimestamps:gpuTimestamp:
    };
}
