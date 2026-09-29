// Esia - render hardware interface (RHI).
//
// The renderer (esia/render) drives the GPU only through rhi::Device. The interface is deliberately small - it
// is what Esia's renderer needs, not a general graphics API - and shaped so that Direct3D 9 / 10 / 11 / 12,
// OpenGL 3.3 / GLES 3, Vulkan and Metal can all implement it (docs/backends/README.md maps every call).
//
// Model
//   * Resources are opaque handles (Texture, Buffer, Pipeline); 0 = none. Creation / destruction happens on the
//     render thread; a backend with frames in flight defers the release until the GPU is done.
//   * A frame is BeginFrame .. EndFrame. Every Update* of a frame happens BEFORE its first BeginPass; after that
//     the frame's resources are immutable (backends with frames in flight version them in a ring). Constants are
//     the exception: SetConstants is inline and versioned by the backend (ring / push / root constants).
//   * Drawing happens inside passes: BeginPass(target, load op) .. EndPass. CopyTexture only outside passes.
//     A pass keeps its target bound; the renderer ends and restarts passes around backdrop captures. The viewport
//     is always the whole target. Every pass starts with NOTHING bound (pipeline, textures, buffers, constants,
//     scissor = whole target): the renderer binds all it uses again after each BeginPass, so Metal encoders and
//     Vulkan render passes need no state carried across passes.
//   * Fixed binding model (docs/backends/README.md, "Binding model"), identical in every shader:
//       constants  Frame (b0), Pass (b1), Draw (b2)
//       textures   t0 main texture, t1..t6 backdrop pyramid, t7 FX instance data (FxStorage::Texture)
//       buffers    t7 FX instance data (FxStorage::Buffer: structured / storage buffer of float4)
//       samplers   s0 linear clamp, s1 point clamp (fixed per shader; the shader manifest says which texture
//                  uses which sampler, for APIs with combined samplers)
//   * Coordinates are render-target pixels with the origin at the top-left, y down, for scissors, copies and
//     readback, on every API. A backend whose framebuffer origin is bottom-left (OpenGL) converts them and sets
//     Caps::framebufferOriginBottomLeft so the shaders flip the pixel positions they read.
//   * Color: the shaders work on the stored (gamma-encoded) values and encode their own output for *_SRGB
//     targets, as WGT does. So sampling never decodes sRGB: a texture sampled by the renderer is never *_SRGB, and
//     a render target that is (RGBA8_SRGB / BGRA8_SRGB) is only sampled through a raw view (a host target reports
//     TextureUsage_Sampled only if the backend can give it one). CopyTexture copies bits, and may copy between a
//     format and its RawFormat (the backdrop copy of an sRGB target is RawFormat(target)).
#pragma once
#include "esia/base/config.hpp"
#include <cstddef>
#include <string>
#include <vector>

namespace esia::rhi
{
    // ------------------------------------------------------------------ formats
    enum class Format : std::uint8_t
    {
        Unknown,
        RGBA8_UNORM,
        RGBA8_SRGB,
        BGRA8_UNORM,
        BGRA8_SRGB,
        RGB10A2_UNORM,
        RGBA16_FLOAT,
        RGBA32_FLOAT,
        R8_UNORM,
    };
    ESIA_API bool IsSrgb(Format f);
    // The format with the same bits and no sRGB decoding (RGBA8_SRGB -> RGBA8_UNORM ...); others map to themselves.
    ESIA_API Format RawFormat(Format f);
    ESIA_API int BytesPerPixel(Format f);
    ESIA_API const char* FormatName(Format f);

    enum TextureUsage_ : std::uint32_t
    {
        TextureUsage_Sampled = 1u << 0,
        TextureUsage_RenderTarget = 1u << 1,
        TextureUsage_CopySrc = 1u << 2,
        TextureUsage_CopyDst = 1u << 3,
    };

    struct TextureDesc
    {
        int width = 0, height = 0;
        Format format = Format::RGBA8_UNORM;
        std::uint32_t usage = TextureUsage_Sampled;
        int samples = 1;
        const char* debugName = nullptr;
    };

    enum class BufferKind : std::uint8_t
    {
        Vertex,        // esia::Vertex (VertexLayout::UiVertex)
        Index,         // uint32 indices
        FxInstances,   // float4 array bound at t7 (Caps::fxStorage == FxStorage::Buffer)
    };

    struct BufferDesc
    {
        BufferKind kind = BufferKind::Vertex;
        std::size_t size = 0;   // bytes
        const char* debugName = nullptr;
    };

    // ------------------------------------------------------------------ handles
    struct Texture
    {
        std::uint32_t id = 0;
        explicit operator bool() const { return id != 0; }
        bool operator==(const Texture&) const = default;
    };
    struct Buffer
    {
        std::uint32_t id = 0;
        explicit operator bool() const { return id != 0; }
        bool operator==(const Buffer&) const = default;
    };
    struct Pipeline
    {
        std::uint32_t id = 0;
        explicit operator bool() const { return id != 0; }
        bool operator==(const Pipeline&) const = default;
    };

    struct IRect
    {
        int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        int Width() const { return x1 - x0; }
        int Height() const { return y1 - y0; }
        bool Empty() const { return x1 <= x0 || y1 <= y0; }
        bool operator==(const IRect&) const = default;
    };

    // ------------------------------------------------------------------ pipelines
    // The renderer's fixed set of shader programs (vertex + pixel entry points of src/esia/shaders). A backend
    // gets its bytecode / source from the generated shader library (esia/render/shader_library.hpp).
    enum class ShaderProgram : std::uint8_t
    {
        UiGeometry,       // UiVS + UiPS: images, lines, charts
        TextGray,         // UiVS + TextGrayPS: Alpha8 glyph coverage
        TextLcd,          // UiVS + TextLcdPS: sub-pixel coverage, dual-source blending
        TextLcdGray,      // UiVS + TextLcdGrayPS: sub-pixel pages drawn into alpha targets (layers)
        Fx,               // FxVS + FxPS: SDF shapes, instanced quads (4 vertices, triangle strip)
        Downsample,       // FullscreenVS + DownsamplePS: one pyramid level (3 vertices)
        LayerComposite,   // FullscreenVS + LayerCompositePS: glow layer onto the target
        Clear,            // FullscreenVS + ClearPS: clears the scissor rect to transparent black
        Count
    };
    ESIA_API const char* ShaderProgramName(ShaderProgram p);

    enum class VertexLayout : std::uint8_t
    {
        None,       // no vertex buffer: positions come from SV_VertexID (fullscreen passes, FX quads)
        UiVertex,   // float2 pos, float2 uv, RGBA8 unorm color (20 bytes)
    };

    enum class Topology : std::uint8_t { TriangleList, TriangleStrip };

    enum class BlendMode : std::uint8_t
    {
        Opaque,          // src
        Straight,        // rgb: src * srcA + dst * (1 - srcA); a: src + dst * (1 - srcA)
        Premultiplied,   // rgb: src + dst * (1 - srcA);        a: src + dst * (1 - srcA)
        DualSourceLcd,   // rgb: src0 * src1 + dst * (1 - src1); a: src1.a + dst * (1 - src1.a)  (Caps::dualSourceBlend)
    };

    struct PipelineDesc
    {
        ShaderProgram program = ShaderProgram::UiGeometry;
        VertexLayout layout = VertexLayout::UiVertex;
        Topology topology = Topology::TriangleList;
        BlendMode blend = BlendMode::Straight;
        Format targetFormat = Format::RGBA8_UNORM;
        int samples = 1;
        // Fx only: a user effect (EffectId != 0) whose HLSL defines `float4 WgtEffect(WgtFx fx)`. Backends without
        // Caps::runtimeEffects return an invalid pipeline and the renderer draws with the built-in shader.
        EffectId effect = 0;
        const char* effectSource = nullptr;
        // Fx only, with Caps::fxFeatureVariants: the fx::Feature bits the batch's instances use - compile the FX
        // shader with ESIA_FX_FEATURES = this mask (the rest of the shader is gone). 0 = every feature.
        std::uint32_t fxFeatures = 0;
        bool operator==(const PipelineDesc&) const = default;
    };

    // ------------------------------------------------------------------ binding model
    enum class ConstantSlot : std::uint8_t { Frame = 0, Pass = 1, Draw = 2 };
    constexpr int kSlotTexture = 0;       // t0
    constexpr int kSlotBackdrop0 = 1;     // t1 .. t6: backdrop pyramid levels 0 .. 5
    constexpr int kBackdropLevels = 6;
    constexpr int kSlotFxData = 7;        // t7: FX instance data
    constexpr int kTextureSlots = 8;

    // ------------------------------------------------------------------ passes / frames
    enum class LoadOp : std::uint8_t { Load, Clear, DontCare };

    struct PassDesc
    {
        Texture target;
        LoadOp load = LoadOp::Load;
        float clearColor[4] = {0, 0, 0, 0};
        const char* debugName = nullptr;
    };

    struct FrameDesc
    {
        // API specific recording context the host renders with (ID3D12GraphicsCommandList*, VkCommandBuffer,
        // id<MTLCommandBuffer>, IDirect3DDevice9* ...); null for backends that own their context (GL, D3D11).
        void* nativeContext = nullptr;
    };

    // GPU time per category (non-blocking timestamps, read a few frames later).
    enum class ProfileCategory : std::uint8_t
    {
        Capture,    // backdrop copy + blur pyramid
        Layer,      // glow layer pyramid + composite
        Fx,         // SDF batches without glass
        FxGlass,    // SDF batches with glass
        Geometry,   // indexed geometry and text
        Count
    };

    struct GpuProfile
    {
        bool valid = false;
        std::uint64_t frame = 0;   // frame the numbers belong to
        float totalMs = 0.0f;
        float categoryMs[(int)ProfileCategory::Count] = {};
    };

    // ------------------------------------------------------------------ capabilities
    enum class FxStorage : std::uint8_t
    {
        Buffer,    // structured / storage buffer (D3D11 / 12, Vulkan, Metal, GL 4.3)
        Texture,   // RGBA32F texture, fetched by texel (GL 3.3, GLES 3.0, D3D9, D3D10)
    };

    struct Caps
    {
        // how FX instances reach the shaders (esia/core/fx.hpp)
        FxStorage fxStorage = FxStorage::Buffer;
        // the shader library variant this backend loads (esia::shaders::Format, esia/render/shader_library.hpp)
        std::uint8_t shaderFormat = 0;
        // OpenGL: pixel (0, 0) is the bottom-left. The backend converts every IRect; shaders flip SV_Position and the
        // uv of render-target textures (gConv.x, WgtPixelPos / WgtRtUv in esia_common.hlsli).
        bool framebufferOriginBottomLeft = false;
        // Vulkan without a negative viewport height: clip-space +y points down. The renderer flips its projection
        // (and FullscreenVS follows the sign of gXform.y).
        bool clipSpaceYDown = false;
        // Direct3D 9: pixel centers sit on integer coordinates and VPOS holds integers. The renderer sets gConv.y =
        // 0.5 (WgtPixelPos then returns pixel centers); the backend's SM3 shader prelude moves clip-space positions
        // by half a pixel of the current target (docs/backends/README.md, "Direct3D 9").
        bool halfPixelOffset = false;
        // Sub-pixel text needs dual-source blending; without it sub-pixel glyph pages are drawn with the grayscale
        // coverage kept in their alpha (TextLcdGray).
        bool dualSourceBlend = true;
        // RGBA16F render targets (with blending) for the backdrop pyramid and glow layers (else RGBA8: some banding)
        bool floatRenderTargets = true;
        // Render targets the backend reports with TextureUsage_Sampled can be sampled right after their pass
        // ended (frosted glass then builds its pyramid from the target: no copy).
        bool sampleRenderTarget = true;
        bool timestampQueries = false;
        bool readback = false;          // ReadPixels works (required by the conformance suite)
        bool runtimeEffects = false;    // PipelineDesc::effectSource is compiled at runtime
        // The backend builds Fx pipelines specialized to PipelineDesc::fxFeatures (runtime compilation of the FX
        // shader with ESIA_FX_FEATURES): for shader models that cannot hold the whole shader (SM3), or for speed.
        bool fxFeatureVariants = false;
        int maxTextureSize = 4096;
        int maxFxDataWidth = 4096;      // FxStorage::Texture: width of the instance texture in texels
    };

    // ------------------------------------------------------------------ device
    class ESIA_API Device
    {
    public:
        virtual ~Device() = default;
        virtual const char* Name() const = 0;
        virtual const Caps& GetCaps() const = 0;

        // ---- resources (render thread)
        // `data` may be null; rowPitch 0 = tightly packed.
        virtual Texture CreateTexture(const TextureDesc& desc, const void* data = nullptr, int rowPitch = 0) = 0;
        virtual void UpdateTexture(Texture tex, const IRect& rect, const void* data, int rowPitch = 0) = 0;
        virtual void DestroyTexture(Texture tex) = 0;
        virtual TextureDesc GetTextureDesc(Texture tex) const = 0;
        virtual Buffer CreateBuffer(const BufferDesc& desc) = 0;
        // Replaces the first `size` bytes (size <= desc.size).
        virtual void UpdateBuffer(Buffer buf, const void* data, std::size_t size) = 0;
        virtual void DestroyBuffer(Buffer buf) = 0;
        virtual Pipeline CreatePipeline(const PipelineDesc& desc) = 0;
        virtual void DestroyPipeline(Pipeline p) = 0;

        // ---- frame
        virtual bool BeginFrame(const FrameDesc& desc) = 0;
        virtual void EndFrame() = 0;

        // ---- commands (between BeginFrame and EndFrame, in order)
        virtual void BeginPass(const PassDesc& desc) = 0;
        virtual void EndPass() = 0;
        virtual void SetPipeline(Pipeline p) = 0;
        virtual void SetScissor(const IRect& r) = 0;
        virtual void SetConstants(ConstantSlot slot, const void* data, std::uint32_t size) = 0;
        virtual void SetTexture(int slot, Texture tex) = 0;   // slots 0 .. kTextureSlots - 1
        virtual void SetFxBuffer(Buffer buf) = 0;              // FxStorage::Buffer, slot t7
        virtual void SetVertexBuffer(Buffer buf) = 0;
        virtual void SetIndexBuffer(Buffer buf) = 0;
        virtual void Draw(std::uint32_t vertexCount, std::uint32_t firstVertex) = 0;
        virtual void DrawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex) = 0;
        // Instance ids start at 0 in every draw; the renderer passes the first instance in the Draw constants.
        virtual void DrawInstanced(std::uint32_t vertexCount, std::uint32_t instanceCount) = 0;
        // Copies `srcRect` of `src` to (dstX, dstY) of `dst`. Outside passes. A multisampled source is resolved.
        // dst has src's format or its RawFormat (the bits are copied, never converted).
        virtual void CopyTexture(Texture dst, int dstX, int dstY, Texture src, const IRect& srcRect) = 0;

        // ---- host callbacks (DrawCmdKind::Callback): the API object host code records its own draws with, inside
        // the current pass (ID3D11DeviceContext*, ID3D12GraphicsCommandList*, VkCommandBuffer, the current
        // id<MTLRenderCommandEncoder>, IDirect3DDevice9*; nullptr for OpenGL, whose context is current). Host code may
        // change any state: the backend forgets what it had bound, and the renderer binds everything again.
        virtual void* NativeRenderState() = 0;

        // ---- profiling (Caps::timestampQueries; no-ops otherwise)
        virtual void BeginProfile(ProfileCategory category) = 0;
        virtual void EndProfile() = 0;
        virtual bool ReadProfile(GpuProfile& out) = 0;

        // ---- readback (Caps::readback): RGBA8 rows, top row first, whatever the texture format.
        // Waits for the GPU: tests and screenshots only.
        virtual bool ReadPixels(Texture tex, const IRect& rect, std::vector<std::uint8_t>& rgba8) = 0;
    };
}
