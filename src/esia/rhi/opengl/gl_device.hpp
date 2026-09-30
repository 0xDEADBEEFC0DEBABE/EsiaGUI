// Esia OpenGL backend - the rhi::Device on an OpenGL 3.3 core or OpenGL ES 3.0 context (private).
//
// How the RHI maps to GL (docs/backends/README.md, "OpenGL 3.3 core / OpenGL ES 3.0"):
//   * textures are GL_TEXTURE_2D with one level; render targets get an FBO, multisampled ones a renderbuffer (GLES 3.0
//     has no multisample textures, and the RHI never samples them); copy destinations and readback sources get an FBO
//     when first needed (blits and glReadPixels work on framebuffers);
//   * GL's framebuffer origin is the bottom-left: every rectangle on a render target is converted (y = h - y1), and
//     textures remember whether their rows are in GL's order (rendered or copied) or top row first (uploaded);
//   * texture unit n = RHI slot n, with the program's sampler object on it (each program's GLSL names the sampler
//     of every slot); unit kScratchUnit is the backend's own for uploads, so the slot bindings survive them;
//   * constants go through one uniform-buffer ring bound with glBindBufferRange at binding points 0 / 1 / 2;
//   * GL keeps an object alive while queued commands use it, so destruction is immediate.
#pragma once
#include "esia/rhi/opengl.hpp"
#include "gl_api.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace esia::rhi::opengl
{
    // A context the device owns (headless devices): made current by the calls that may follow another device's
    // work, and destroyed after the device's GL objects.
    class OwnedContext
    {
    public:
        virtual ~OwnedContext() = default;
        virtual void MakeCurrent() = 0;
    };

    class GlDevice final : public Device
    {
    public:
        GlDevice(const Desc& desc, std::unique_ptr<OwnedContext> owned);
        ~GlDevice() override;
        // Loads the functions and reads the context's limits and extensions; false (with `error`) if it cannot run.
        bool Init(std::string& error);
        // GL_RENDERER of the context ("NVIDIA GeForce RTX 4080 SUPER/PCIe/SSE2", "llvmpipe (LLVM 20.1.2, 256 bits)")
        const std::string& RendererName() const { return renderer_; }

        const char* Name() const override { return desc_.es ? "gles" : "opengl"; }
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
        std::uint32_t ValidationErrors() const override { return errors_; }   // KHR_debug / glGetError errors

        Texture Wrap(GLuint fbo, int width, int height, Format format, GLuint colorTexture, int samples);
        GLuint NativeTexture(Texture t) const;
        // Sampling reads the stored bits: sRGB textures only where the sampler can skip the decode.
        bool SamplesRaw(Format f) const { return !IsSrgb(f) || srgbDecodeControl_; }
        std::uint32_t Errors() const { return errors_; }
        void OnDebugMessage(GLenum type, GLenum severity, const char* message);

    private:
        static constexpr GLuint kUnknown = 0xFFFFFFFFu;   // a cached binding the host may have changed
        static constexpr int kScratchUnit = kTextureSlots;
        static constexpr int kUnits = kTextureSlots + 1;

        struct Tex
        {
            TextureDesc desc;
            GLuint tex = 0;          // GL_TEXTURE_2D (0: multisampled, or a host framebuffer without a texture)
            GLuint rbo = 0;          // multisampled color renderbuffer
            GLuint fbo = 0;          // framebuffer with the texture / renderbuffer as color 0
            bool wrapped = false;    // a host framebuffer: fbo and tex belong to the host
            bool bottomUp = false;   // rows in GL's bottom-left order (rendered, or copied from a render target)
        };
        struct Buf
        {
            BufferDesc desc;
            GLuint id = 0;
        };
        struct Program
        {
            GLuint id = 0;
            bool tried = false;
            int sampler[kTextureSlots];   // sampler object of each slot's unit (0 linear, 1 point), -1 = unused
        };
        struct Pipe
        {
            PipelineDesc desc;
            Program* program = nullptr;
        };

        // Everything the device changes, saved at BeginFrame (and around resource calls outside frames) and put
        // back afterwards (Desc::restoreHostState).
        struct HostState
        {
            GLint drawFbo, readFbo, renderbuffer, program, vao, arrayBuffer, copyWriteBuffer, uniformBuffer, packBuffer, unpackBuffer;
            GLint activeTexture, tex[kUnits], sampler[kUnits];
            GLint ubo[3];
            GLint64 uboStart[3], uboSize[3];
            GLint viewport[4], scissor[4], polygonMode[2];
            GLfloat clearColor[4];
            GLboolean colorMask[4];
            GLint blendSrcRgb, blendDstRgb, blendSrcAlpha, blendDstAlpha, blendEqRgb, blendEqAlpha;
            bool blend, scissorTest, depthTest, stencilTest, cullFace, rasterizerDiscard, alphaToCoverage, multisample, srgb;
            GLint pack[4], unpack[4];   // alignment, row length, skip rows, skip pixels
        };

        // Saves the host's state for a call outside a frame and restores it when it goes out of scope.
        class OutsideFrame
        {
        public:
            explicit OutsideFrame(GlDevice& d);
            ~OutsideFrame();
            OutsideFrame(const OutsideFrame&) = delete;
            OutsideFrame& operator=(const OutsideFrame&) = delete;

        private:
            GlDevice& d_;
            bool active_;
        };

        // GPU time per category: timestamps around the frame and every profile scope, read back a few frames later
        // without waiting (the scheme of WGT's D3D11 backend).
        struct TimerSlot
        {
            std::vector<GLuint> queries;
            int used = 0, frameEnd = -1;
            struct Interval
            {
                int start, end;
                ProfileCategory category;
            };
            std::vector<Interval> intervals;
            std::uint64_t frame = 0;
            bool pending = false;
        };
        static constexpr int kTimerSlots = 4, kMaxStamps = 512;

        // state
        void SaveHostState(HostState& s);
        void RestoreHostState(const HostState& s);
        void ForgetGlState();
        void ApplyFixedState();
        void EnsureFixedState();
        void RestorePassState();
        void BindFramebuffer(GLenum target, GLuint fbo);
        void BindUnit(int unit, GLuint tex);
        void BindSampler(int unit, GLuint sampler);
        void SetScissorTest(bool on);
        void SetSrgbWrite(bool on);
        void SetBlend(BlendMode mode);
        void UseProgram(GLuint program);
        void BindVertexArray(GLuint vao);
        void PrepareDraw();
        void CheckErrors(const char* where);

        // resources
        Tex* FindTex(Texture t);
        const Tex* FindTex(Texture t) const;
        GLuint EnsureFbo(Tex& t);
        bool AttachAndCheck(Tex& t);
        void Upload(const Tex& t, int x, int y, int w, int h, const void* data, int rowPitch);
        void ReleaseTex(Tex& t);
        Program* GetProgram(ShaderProgram p);
        const Tex* ResolveTarget(const Tex& src);
        void Blit(const Tex& src, const Tex& dst, const IRect& srcRect, int dstX, int dstY);
        bool DrawCopy(const Tex& src, const Tex& dst, const IRect& srcRect, int dstX, int dstY);
        void Label(GLenum type, GLuint id, const char* name);

        // timing
        int Stamp();
        void TimerBeginFrame();
        void TimerEndFrame();
        bool TimerRead(TimerSlot& s);

        Desc desc_;
        std::unique_ptr<OwnedContext> owned_;   // destroyed last: the GL objects go first
        GlApi gl_;
        Caps caps_;
        int glMajor_ = 0, glMinor_ = 0;
        std::string renderer_;
        bool ready_ = false;               // Init got far enough to create objects: the destructor releases them
        bool srgbDecodeControl_ = false;   // EXT_texture_sRGB_decode: sRGB textures can be sampled raw
        bool debugOutput_ = false;         // our KHR_debug callback is installed
        bool disjointQuery_ = false;       // GL_GPU_DISJOINT_EXT can be read (GLES timer queries)
        int maxSamples_ = 1;
        GLint uboAlign_ = 256;
        std::uint32_t errors_ = 0;

        std::uint32_t next_ = 1;
        std::unordered_map<std::uint32_t, Tex> textures_;
        std::unordered_map<std::uint32_t, Buf> buffers_;
        std::unordered_map<std::uint32_t, Pipe> pipelines_;
        Program programs_[(int)ShaderProgram::Count];
        Tex resolve_;   // single-sample copy of a multisampled source (readback, format-changing copies)
        GLuint copyProgram_ = 0;         // DrawCopy's program, built at the first sRGB -> raw copy
        bool copyTried_ = false;
        GLint copyMap_ = -1, copyEncode_ = -1;

        GLuint samplers_[2] = {};   // linear clamp, point clamp
        GLuint uiVao_ = 0, emptyVao_ = 0;
        GLuint ubo_ = 0;
        GLintptr uboSize_ = 0, uboOffset_ = 0;
        std::uint8_t lastConstants_[3][256] = {};
        std::uint32_t lastConstantSize_[3] = {};

        // frame / pass
        bool inFrame_ = false, inPass_ = false, hostTouched_ = false;
        HostState frameState_{};
        const Tex* pass_ = nullptr;
        const Pipe* pipe_ = nullptr;
        GLuint vb_ = 0, ib_ = 0;

        // GL state as last set by the device (kUnknown / -1: must be set before use)
        GLuint drawFbo_ = kUnknown, readFbo_ = kUnknown, program_ = kUnknown, vao_ = kUnknown, activeUnit_ = kUnknown;
        GLuint unitTex_[kUnits], unitSampler_[kUnits];
        GLuint vaoVb_ = kUnknown, vaoIb_ = kUnknown;
        int scissorTest_ = -1, srgbWrite_ = -1, blend_ = -1;

        // timing
        TimerSlot timer_[kTimerSlots];
        TimerSlot* timerCur_ = nullptr;
        int profileStart_ = -1;
        ProfileCategory profileCategory_ = ProfileCategory::Capture;
        std::uint64_t frame_ = 0;
        GpuProfile profile_;
    };
}
