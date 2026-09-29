// Metal backend: the RHI -> Metal tables (formats, usage, blend, vertex layout, binding indices).
// UNVERIFIED: needs macOS (passes on Linux and under Wine; the raw values are checked by metal_device.mm on macOS).
#include "esia/core/draw_list.hpp"
#include "esia_test.hpp"
#include "metal_tables.hpp"
#include <cstddef>

using namespace esia;
using namespace esia::rhi;
using namespace esia::rhi::metal;

namespace
{
    constexpr Format kFormats[] = {Format::RGBA8_UNORM, Format::RGBA8_SRGB, Format::BGRA8_UNORM, Format::BGRA8_SRGB, Format::RGB10A2_UNORM,
                                   Format::RGBA16_FLOAT, Format::RGBA32_FLOAT, Format::R8_UNORM};
}

ESIA_TEST(MetalTables, EveryRhiFormatHasAPixelFormat)
{
    for (Format f : kFormats)
    {
        const FormatInfo& i = InfoOf(f);
        ESIA_CHECK(i.format == f);
        ESIA_CHECK(i.pixelFormat != mtl::PixelFormatInvalid);
        ESIA_CHECK(FormatOfPixelFormat(i.pixelFormat) == f);
        ESIA_CHECK(i.bytesPerPixel == BytesPerPixel(f));
        ESIA_CHECK(i.srgb == IsSrgb(f));
        // the twins are exactly the formats whose RawFormat differs, in both directions
        ESIA_CHECK(i.hasSrgbTwin == (f != RawFormat(f) || f == Format::RGBA8_UNORM || f == Format::BGRA8_UNORM));
    }
    ESIA_CHECK(PixelFormatOf(Format::Unknown) == mtl::PixelFormatInvalid);
    ESIA_CHECK(FormatOfPixelFormat(mtl::PixelFormatInvalid) == Format::Unknown);
    ESIA_CHECK(FormatOfPixelFormat(94) == Format::Unknown);   // BGR10A2Unorm: a host format the RHI does not have
    // the render targets the renderer creates or gets: the host formats, the pyramid / layer formats
    for (Format f : {Format::RGBA8_UNORM, Format::RGBA8_SRGB, Format::BGRA8_UNORM, Format::BGRA8_SRGB, Format::RGB10A2_UNORM, Format::RGBA16_FLOAT})
        ESIA_CHECK(InfoOf(f).renderable);
    ESIA_CHECK(!InfoOf(Format::RGBA32_FLOAT).renderable);
}

ESIA_TEST(MetalTables, UsageOfCreatedTextures)
{
    TextureDesc d;
    d.format = Format::RGBA16_FLOAT;
    d.usage = TextureUsage_RenderTarget | TextureUsage_Sampled;
    ESIA_CHECK(MetalUsageFor(d) == (mtl::TextureUsageRenderTarget | mtl::TextureUsageShaderRead));
    d.format = Format::RGBA8_SRGB;   // sampled through its UNORM view
    ESIA_CHECK(MetalUsageFor(d) == (mtl::TextureUsageRenderTarget | mtl::TextureUsageShaderRead | mtl::TextureUsagePixelFormatView));
    d.format = Format::RGBA8_UNORM;  // a backdrop copy may be aliased as sRGB for the blit from an sRGB target
    d.usage = TextureUsage_Sampled | TextureUsage_CopyDst;
    ESIA_CHECK(MetalUsageFor(d) == (mtl::TextureUsageShaderRead | mtl::TextureUsagePixelFormatView));
    d.format = Format::R8_UNORM;     // blits need no usage bit
    ESIA_CHECK(MetalUsageFor(d) == mtl::TextureUsageShaderRead);
}

ESIA_TEST(MetalTables, UsageOfHostTextures)
{
    const std::uint32_t rt = mtl::TextureUsageRenderTarget, read = mtl::TextureUsageShaderRead, view = mtl::TextureUsagePixelFormatView;
    // CAMetalLayer's default drawables: render target only
    ESIA_CHECK(RhiUsageOfHostTexture(Format::BGRA8_UNORM, rt, true, 1) == TextureUsage_RenderTarget);
    // framebufferOnly = NO: copies (the backdrop copy of glass)
    ESIA_CHECK(RhiUsageOfHostTexture(Format::BGRA8_UNORM, rt, false, 1) == (TextureUsage_RenderTarget | TextureUsage_CopySrc | TextureUsage_CopyDst));
    // readable: the direct capture path
    ESIA_CHECK(RhiUsageOfHostTexture(Format::BGRA8_UNORM, rt | read, false, 1) & TextureUsage_Sampled);
    // sRGB: only through a raw view
    ESIA_CHECK(!(RhiUsageOfHostTexture(Format::BGRA8_SRGB, rt | read, false, 1) & TextureUsage_Sampled));
    ESIA_CHECK(RhiUsageOfHostTexture(Format::BGRA8_SRGB, rt | read | view, false, 1) & TextureUsage_Sampled);
    // multisampled: resolved, never sampled
    ESIA_CHECK(!(RhiUsageOfHostTexture(Format::RGBA8_UNORM, rt | read, false, 4) & TextureUsage_Sampled));
}

ESIA_TEST(MetalTables, BlendStates)
{
    ESIA_CHECK(!BlendStateOf(BlendMode::Opaque).enabled);
    const BlendState s = BlendStateOf(BlendMode::Straight);
    ESIA_CHECK(s.enabled && s.rgbSrc == mtl::BlendFactorSourceAlpha && s.rgbDst == mtl::BlendFactorOneMinusSourceAlpha);
    ESIA_CHECK(s.alphaSrc == mtl::BlendFactorOne && s.alphaDst == mtl::BlendFactorOneMinusSourceAlpha);
    const BlendState p = BlendStateOf(BlendMode::Premultiplied);
    ESIA_CHECK(p.enabled && p.rgbSrc == mtl::BlendFactorOne && p.rgbDst == mtl::BlendFactorOneMinusSourceAlpha);
    ESIA_CHECK(p.alphaSrc == mtl::BlendFactorOne && p.alphaDst == mtl::BlendFactorOneMinusSourceAlpha);
    for (BlendMode m : {BlendMode::Opaque, BlendMode::Straight, BlendMode::Premultiplied})
        ESIA_CHECK(BlendStateOf(m).rgbOp == mtl::BlendOperationAdd && BlendStateOf(m).alphaOp == mtl::BlendOperationAdd);
}

ESIA_TEST(MetalTables, BindingsAndVertexLayout)
{
    ESIA_CHECK(binding::TextureIndex(kSlotTexture) == 3);
    ESIA_CHECK(binding::TextureIndex(kSlotBackdrop0 + kBackdropLevels - 1) == 9);
    ESIA_CHECK(binding::ConstantIndex(ConstantSlot::Frame) == 0 && binding::ConstantIndex(ConstantSlot::Pass) == 1 &&
               binding::ConstantIndex(ConstantSlot::Draw) == 2);
    // Metal has buffer indices 0..30; the vertex buffer must not collide with the shaders' 0, 1, 2, 10
    static_assert(binding::kVertexBuffer <= 30 && binding::kVertexBuffer > binding::kFxData);
    // samplers: Metal guarantees 16 per stage
    static_assert(binding::kSamplerLinear < 16 && binding::kSamplerPoint < 16);

    static_assert(kUiVertexStride == sizeof(Vertex));
    ESIA_CHECK(kUiVertexAttributes[0].offset == offsetof(Vertex, pos) && kUiVertexAttributes[0].format == mtl::VertexFormatFloat2);
    ESIA_CHECK(kUiVertexAttributes[1].offset == offsetof(Vertex, uv) && kUiVertexAttributes[1].format == mtl::VertexFormatFloat2);
    ESIA_CHECK(kUiVertexAttributes[2].offset == offsetof(Vertex, color) && kUiVertexAttributes[2].format == mtl::VertexFormatUChar4Normalized);

    ESIA_CHECK(LoadActionOf(LoadOp::Load) == mtl::LoadActionLoad && LoadActionOf(LoadOp::Clear) == mtl::LoadActionClear &&
               LoadActionOf(LoadOp::DontCare) == mtl::LoadActionDontCare);
    ESIA_CHECK(PrimitiveTypeOf(Topology::TriangleList) == mtl::PrimitiveTypeTriangle && PrimitiveTypeOf(Topology::TriangleStrip) == mtl::PrimitiveTypeTriangleStrip);
}
