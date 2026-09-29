// Esia - the null RHI backend: records and validates, draws nothing.
//
// It runs everywhere (no GPU) and backs the renderer's tests and the conformance suite's command-stream goldens:
//   * every call is appended to Log() as one deterministic text line (handles are numbered in creation order,
//     constants are printed as values), so a renderer change shows up as a readable diff;
//   * every call is checked against the RHI contract (esia/rhi/rhi.hpp) and violations go to Errors(): draws
//     outside passes or with state from an earlier pass (every pass starts with nothing bound), updates after the
//     first pass, copies inside a pass or between incompatible formats, sampling the bound target, scissors
//     outside the target, pipelines the caps or the target do not allow ...
// Caps are configurable, so the renderer's paths for every API family (texture instance storage, bottom-left
// origin, no dual-source blending ...) are exercised here.
#pragma once
#include "esia/rhi/rhi.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace esia::rhi
{
    struct NullOptions
    {
        Caps caps;
        bool record = true;     // keep the log (validation runs either way)
        bool keepData = false;  // keep what was uploaded into buffers and textures (Data())
        // Caps::asyncPipelines as a backend would do it: a PipelineDesc::background pipeline stays Pending for this
        // many frames (BeginFrame calls), then turns Ready - or Failed with failBackground
        int pendingFrames = 0;
        bool failBackground = false;
        bool refuseFxVariants = false;   // CreatePipeline returns {} for Fx pipelines with fxFeatures (SM3 overflow)
        std::uint32_t refusePrograms = 0;   // ... and for these programs (bit 1 << ShaderProgram): broken backends
    };

    struct NullStats
    {
        int passes = 0, draws = 0, instancedDraws = 0, indexedDraws = 0, copies = 0, textureUpdates = 0, bufferUpdates = 0;
    };

    class ESIA_API NullDevice final : public Device
    {
    public:
        explicit NullDevice(const NullOptions& options);

        const char* Name() const override { return "null"; }
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
        PipelineStatus GetPipelineStatus(Pipeline p) const override;

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
        void BeginProfile(ProfileCategory category) override;
        void EndProfile() override;
        bool ReadProfile(GpuProfile& out) override;
        bool ReadPixels(Texture tex, const IRect& rect, std::vector<std::uint8_t>& rgba8) override;
        void* NativeRenderState() override;
        std::uint32_t ValidationErrors() const override { return (std::uint32_t)errors_.size(); }

        // A texture standing for a host render target (what a real backend's WrapRenderTarget returns).
        Texture CreateHostTarget(int width, int height, Format format, bool sampleable, int samples = 1);

        const std::vector<std::string>& Log() const { return log_; }
        const std::vector<std::string>& Errors() const { return errors_; }
        const NullStats& Stats() const { return stats_; }
        void ClearLog() { log_.clear(); errors_.clear(); stats_ = NullStats(); }
        // keepData: the bytes of a buffer, or of a texture (tightly packed rows); null if unknown
        const std::vector<std::uint8_t>* Data(Buffer b) const;
        const std::vector<std::uint8_t>* Data(Texture t) const;
        std::size_t LiveTextures() const { return textures_.size(); }
        std::size_t LiveBuffers() const { return buffers_.size(); }
        std::size_t LivePipelines() const { return pipelines_.size(); }

    private:
        void Record(const std::string& line);
        void Error(const std::string& what);
        bool InFrame(const char* call);
        bool InPass(const char* call);
        void CheckDraw(const char* call, bool instanced);
        std::string TexName(Texture t) const;

        Caps caps_;
        NullOptions options_;
        bool record_ = true;
        bool keepData_ = false;
        std::unordered_map<std::uint32_t, std::vector<std::uint8_t>> data_;
        std::uint32_t next_ = 1;
        std::unordered_map<std::uint32_t, TextureDesc> textures_;
        std::unordered_map<std::uint32_t, BufferDesc> buffers_;
        std::unordered_map<std::uint32_t, PipelineDesc> pipelines_;
        std::unordered_map<std::uint32_t, std::uint64_t> pipelineReadyAt_;   // background pipelines: first Ready frame
        bool inFrame_ = false, inPass_ = false, passStarted_ = false;
        Texture passTarget_;
        Pipeline pipeline_;
        Texture bound_[kTextureSlots];
        Buffer fxBuffer_, vertexBuffer_, indexBuffer_;
        bool constantsSet_[3] = {};
        int profileDepth_ = 0;
        std::uint64_t frame_ = 0;
        std::vector<std::string> log_, errors_;
        NullStats stats_;
    };
}
