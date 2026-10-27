// Esia - Metal backend: the RHI -> Metal translation tables (formats, usage, blend, vertex layout, binding indices).
//
// Plain C++ without Apple headers, so that it is compiled and unit-tested on every host (tests/test_metal_tables.cpp).
// The raw values mirror Apple's <Metal/MTL*.h> enums; metal_device.mm static_asserts every one of them against the
// SDK, so a wrong value cannot survive a macOS build.
//
// UNVERIFIED: needs macOS (the values were written from Apple's documentation on Linux; the static_asserts that
// check them only compile with the macOS / iOS SDK).
#pragma once
#include "esia/rhi/rhi.hpp"
#include <cstddef>
#include <cstdint>

namespace esia::rhi::metal
{
    // ------------------------------------------------------------------ raw Metal values
    namespace mtl
    {
        // MTLPixelFormat
        enum PixelFormat : std::uint32_t
        {
            PixelFormatInvalid = 0,
            PixelFormatR8Unorm = 10,
            PixelFormatRGBA8Unorm = 70,
            PixelFormatRGBA8Unorm_sRGB = 71,
            PixelFormatBGRA8Unorm = 80,
            PixelFormatBGRA8Unorm_sRGB = 81,
            PixelFormatRGB10A2Unorm = 90,
            PixelFormatRGBA16Float = 115,
            PixelFormatRGBA32Float = 125,
        };
        // MTLTextureUsage (bits)
        enum TextureUsage : std::uint32_t
        {
            TextureUsageShaderRead = 0x01,
            TextureUsageShaderWrite = 0x02,
            TextureUsageRenderTarget = 0x04,
            TextureUsagePixelFormatView = 0x10,
        };
        // MTLBlendFactor
        enum BlendFactor : std::uint32_t
        {
            BlendFactorZero = 0,
            BlendFactorOne = 1,
            BlendFactorSourceAlpha = 4,
            BlendFactorOneMinusSourceAlpha = 5,
            BlendFactorSource1Color = 15,
            BlendFactorOneMinusSource1Color = 16,
            BlendFactorSource1Alpha = 17,
            BlendFactorOneMinusSource1Alpha = 18,
        };
        enum BlendOperation : std::uint32_t { BlendOperationAdd = 0 };   // MTLBlendOperation
        enum LoadAction : std::uint32_t { LoadActionDontCare = 0, LoadActionLoad = 1, LoadActionClear = 2 };
        enum StoreAction : std::uint32_t { StoreActionDontCare = 0, StoreActionStore = 1, StoreActionMultisampleResolve = 2, StoreActionStoreAndMultisampleResolve = 3 };
        enum PrimitiveType : std::uint32_t { PrimitiveTypeTriangle = 3, PrimitiveTypeTriangleStrip = 4 };
        enum VertexFormat : std::uint32_t { VertexFormatUChar4Normalized = 9, VertexFormatFloat2 = 29 };
        enum VertexStepFunction : std::uint32_t { VertexStepFunctionPerVertex = 1 };
        enum IndexType : std::uint32_t { IndexTypeUInt32 = 1 };
        // MTLCounterDontSample (NSUInteger) and MTLCounterErrorValue (MTLCounterResultTimestamp on failure)
        constexpr std::uint64_t kCounterDontSample = ~0ull;
        constexpr std::uint64_t kCounterErrorValue = ~0ull;
    }

    // ------------------------------------------------------------------ formats
    struct FormatInfo
    {
        Format format;
        std::uint32_t pixelFormat;      // mtl::PixelFormat
        int bytesPerPixel;
        bool srgb;
        // the format has an sRGB / UNORM twin with the same bits: views between the two need
        // MTLTextureUsagePixelFormatView (raw reads of sRGB targets, copies between a format and its RawFormat)
        bool hasSrgbTwin;
        bool renderable;                // with blending, on every Metal GPU of macOS 11 / iOS 14
    };

    // The table entry of a format; Format::Unknown for formats Metal does not get from the renderer.
    const FormatInfo& InfoOf(Format f);
    std::uint32_t PixelFormatOf(Format f);                 // mtl::PixelFormatInvalid if none
    Format FormatOfPixelFormat(std::uint32_t pixelFormat); // Format::Unknown if the RHI has no such format

    // MTLTextureUsage for a texture the backend creates: ShaderRead for Sampled, RenderTarget, and PixelFormatView
    // when the format has an sRGB twin (a raw view for sampling an sRGB target, or for a copy between the twins).
    std::uint32_t MetalUsageFor(const TextureDesc& desc);

    // RHI usage of a host texture (WrapTexture): what the backend can do with it.
    //   * framebufferOnly (CAMetalLayer's default) textures can only be rendered to: no copy, no sampling;
    //   * multisampled textures are never sampled (the shaders read texture2d), only resolved;
    //   * an sRGB texture is sampled through a UNORM view, which needs MTLTextureUsagePixelFormatView.
    std::uint32_t RhiUsageOfHostTexture(Format format, std::uint32_t metalUsage, bool framebufferOnly, int samples);

    // ------------------------------------------------------------------ blending
    struct BlendState
    {
        bool enabled = false;
        std::uint32_t rgbSrc = mtl::BlendFactorOne, rgbDst = mtl::BlendFactorZero;
        std::uint32_t alphaSrc = mtl::BlendFactorOne, alphaDst = mtl::BlendFactorZero;
        std::uint32_t rgbOp = mtl::BlendOperationAdd, alphaOp = mtl::BlendOperationAdd;
    };
    BlendState BlendStateOf(BlendMode mode);

    // ------------------------------------------------------------------ binding model (docs/backends/README.md 4.1)
    // The generated MSL uses the Vulkan binding numbers as Metal indices (tests/test_metal_msl.cpp checks every one).
    namespace binding
    {
        constexpr std::uint32_t kFrameConstants = 0;   // buffer(0) WgtFrame
        constexpr std::uint32_t kPassConstants = 1;    // buffer(1) WgtPass
        constexpr std::uint32_t kDrawConstants = 2;    // buffer(2) WgtDraw
        constexpr std::uint32_t kFirstTexture = 3;     // texture(3) gTex = t0, texture(4..9) gBackdrop0..5 = t1..t6
        constexpr std::uint32_t kFxData = 10;          // buffer(10) gFxData (const device float4*)
        constexpr std::uint32_t kSamplerLinear = 11;   // sampler(11) gLinear
        constexpr std::uint32_t kSamplerPoint = 12;    // sampler(12) gPoint
        // The UI vertex buffer: an index no shader declares. Metal has buffer indices 0..30.
        constexpr std::uint32_t kVertexBuffer = 30;
        // setVertexBytes / setFragmentBytes are documented for data under 4 KB
        constexpr std::uint32_t kMaxInlineBytes = 4096;

        constexpr std::uint32_t ConstantIndex(ConstantSlot s) { return (std::uint32_t)s; }
        // The Metal texture index of RHI texture slot t0..t6 (t7 is the FX buffer on Metal: FxStorage::Buffer).
        constexpr std::uint32_t TextureIndex(int slot) { return kFirstTexture + (std::uint32_t)slot; }
    }

    // ------------------------------------------------------------------ vertex layout (VertexLayout::UiVertex)
    struct VertexAttribute
    {
        std::uint32_t format;   // mtl::VertexFormat
        std::uint32_t offset;
    };
    constexpr std::uint32_t kUiVertexStride = 20;
    // attributes 0 (pos, float2), 1 (uv, float2), 2 (color, RGBA8 unorm, R in the lowest byte)
    constexpr VertexAttribute kUiVertexAttributes[3] = {
        {mtl::VertexFormatFloat2, 0},
        {mtl::VertexFormatFloat2, 8},
        {mtl::VertexFormatUChar4Normalized, 16},
    };

    // ------------------------------------------------------------------ passes and draws
    std::uint32_t LoadActionOf(LoadOp op);
    std::uint32_t PrimitiveTypeOf(Topology t);
}
