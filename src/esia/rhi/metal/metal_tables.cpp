// Esia - Metal backend: translation tables (see metal_tables.hpp). UNVERIFIED: needs macOS.
#include "metal_tables.hpp"

namespace esia::rhi::metal
{
    namespace
    {
        // RGBA32Float is only the FX instance texture of FxStorage::Texture backends; Metal uses a buffer, and
        // RGBA32Float is not blendable on every Apple GPU, so it is never a render target here.
        constexpr FormatInfo kFormats[] = {
            {Format::Unknown, mtl::PixelFormatInvalid, 0, false, false, false},
            {Format::RGBA8_UNORM, mtl::PixelFormatRGBA8Unorm, 4, false, true, true},
            {Format::RGBA8_SRGB, mtl::PixelFormatRGBA8Unorm_sRGB, 4, true, true, true},
            {Format::BGRA8_UNORM, mtl::PixelFormatBGRA8Unorm, 4, false, true, true},
            {Format::BGRA8_SRGB, mtl::PixelFormatBGRA8Unorm_sRGB, 4, true, true, true},
            {Format::RGB10A2_UNORM, mtl::PixelFormatRGB10A2Unorm, 4, false, false, true},
            {Format::RGBA16_FLOAT, mtl::PixelFormatRGBA16Float, 8, false, false, true},
            {Format::RGBA32_FLOAT, mtl::PixelFormatRGBA32Float, 16, false, false, false},
            {Format::R8_UNORM, mtl::PixelFormatR8Unorm, 1, false, false, true},
        };
    }

    const FormatInfo& InfoOf(Format f)
    {
        for (const FormatInfo& i : kFormats)
            if (i.format == f)
                return i;
        return kFormats[0];
    }

    std::uint32_t PixelFormatOf(Format f) { return InfoOf(f).pixelFormat; }

    Format FormatOfPixelFormat(std::uint32_t pixelFormat)
    {
        if (pixelFormat == mtl::PixelFormatInvalid)
            return Format::Unknown;
        for (const FormatInfo& i : kFormats)
            if (i.pixelFormat == pixelFormat)
                return i.format;
        return Format::Unknown;
    }

    std::uint32_t MetalUsageFor(const TextureDesc& desc)
    {
        std::uint32_t u = 0;
        if (desc.usage & TextureUsage_Sampled)
            u |= mtl::TextureUsageShaderRead;
        if (desc.usage & TextureUsage_RenderTarget)
            u |= mtl::TextureUsageRenderTarget;
        if (InfoOf(desc.format).hasSrgbTwin)
            u |= mtl::TextureUsagePixelFormatView;
        return u;
    }

    std::uint32_t RhiUsageOfHostTexture(Format format, std::uint32_t metalUsage, bool framebufferOnly, int samples)
    {
        std::uint32_t u = 0;
        if (metalUsage & mtl::TextureUsageRenderTarget)
            u |= TextureUsage_RenderTarget;
        if (framebufferOnly)
            return u;
        u |= TextureUsage_CopySrc | TextureUsage_CopyDst;
        const bool raw = !IsSrgb(format) || (metalUsage & mtl::TextureUsagePixelFormatView);
        if (samples == 1 && (metalUsage & mtl::TextureUsageShaderRead) && raw)
            u |= TextureUsage_Sampled;
        return u;
    }

    BlendState BlendStateOf(BlendMode mode)
    {
        BlendState b;
        switch (mode)
        {
        case BlendMode::Opaque:
            break;
        case BlendMode::Straight:
            b.enabled = true;
            b.rgbSrc = mtl::BlendFactorSourceAlpha;
            b.rgbDst = mtl::BlendFactorOneMinusSourceAlpha;
            b.alphaSrc = mtl::BlendFactorOne;
            b.alphaDst = mtl::BlendFactorOneMinusSourceAlpha;
            break;
        case BlendMode::Premultiplied:
            b.enabled = true;
            b.rgbSrc = mtl::BlendFactorOne;
            b.rgbDst = mtl::BlendFactorOneMinusSourceAlpha;
            b.alphaSrc = mtl::BlendFactorOne;
            b.alphaDst = mtl::BlendFactorOneMinusSourceAlpha;
            break;
        case BlendMode::DualSourceLcd:
            // rgb: src0 * src1 + dst * (1 - src1). The alpha factor multiplies src0.a, which TextLcdPS writes as 1,
            // so Source1Alpha gives a = src1.a + dst * (1 - src1.a) as the RHI defines it.
            b.enabled = true;
            b.rgbSrc = mtl::BlendFactorSource1Color;
            b.rgbDst = mtl::BlendFactorOneMinusSource1Color;
            b.alphaSrc = mtl::BlendFactorSource1Alpha;
            b.alphaDst = mtl::BlendFactorOneMinusSource1Alpha;
            break;
        }
        return b;
    }

    std::uint32_t LoadActionOf(LoadOp op)
    {
        switch (op)
        {
        case LoadOp::Load: return mtl::LoadActionLoad;
        case LoadOp::Clear: return mtl::LoadActionClear;
        case LoadOp::DontCare: return mtl::LoadActionDontCare;
        }
        return mtl::LoadActionLoad;
    }

    std::uint32_t PrimitiveTypeOf(Topology t)
    {
        return t == Topology::TriangleStrip ? mtl::PrimitiveTypeTriangleStrip : mtl::PrimitiveTypeTriangle;
    }
}
