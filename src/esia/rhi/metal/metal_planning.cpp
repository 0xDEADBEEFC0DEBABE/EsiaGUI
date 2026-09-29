// Esia - Metal backend: pipeline keys, blit layouts, readback conversion, copy plans (see metal_planning.hpp).
// UNVERIFIED: needs macOS.
#include "metal_planning.hpp"
#include "metal_tables.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace esia::rhi::metal
{
    // ------------------------------------------------------------------ pipelines
    std::uint32_t PackPipelineKey(const PipelineKey& k)
    {
        return (std::uint32_t)k.program | ((std::uint32_t)k.blend << 8) | ((std::uint32_t)k.format << 16) | ((std::uint32_t)k.samples << 24);
    }

    std::size_t PipelineKeyHash::operator()(const PipelineKey& k) const
    {
        // the packed key is unique; spread it so the low bits of a power-of-two bucket count see every field
        return (std::size_t)((std::uint64_t)PackPipelineKey(k) * 0x9E3779B97F4A7C15ull >> 16);
    }

    bool ProgramUsesVertices(ShaderProgram p)
    {
        return p == ShaderProgram::UiGeometry || p == ShaderProgram::TextGray;
    }

    bool MakePipelineKey(const PipelineDesc& d, PipelineKey& key, std::string& why)
    {
        if (d.program >= ShaderProgram::Count)
        {
            why = "unknown program";
            return false;
        }
        if (d.effect != 0)
        {
            why = "user effects need runtime HLSL compilation (Caps::runtimeEffects is false on Metal)";
            return false;
        }
        if (ProgramUsesVertices(d.program) != (d.layout == VertexLayout::UiVertex))
        {
            why = "vertex layout does not match the program";
            return false;
        }
        const FormatInfo& f = InfoOf(d.targetFormat);
        if (f.format == Format::Unknown || !f.renderable)
        {
            why = std::string("not a render-target format: ") + FormatName(d.targetFormat);
            return false;
        }
        if (d.samples < 1 || d.samples > 8)
        {
            why = "sample count";
            return false;
        }
        // fxFeatures is 0 without Caps::fxFeatureVariants; the full shader is a superset of any variant anyway
        key.program = d.program;
        key.blend = d.blend;
        key.format = d.targetFormat;
        key.samples = (std::uint8_t)d.samples;
        return true;
    }

    // ------------------------------------------------------------------ uploads and readback
    BufferImageLayout LayoutOf(Format format, int width, int height)
    {
        BufferImageLayout l;
        if (width <= 0 || height <= 0)
            return l;
        l.rowBytes = (std::size_t)width * (std::size_t)InfoOf(format).bytesPerPixel;
        l.bytesPerRow = AlignUp(l.rowBytes, kBlitAlignment);
        l.bytes = l.bytesPerRow * (std::size_t)height;
        return l;
    }

    void CopyRows(void* dst, std::size_t dstPitch, const void* src, std::size_t srcPitch, std::size_t rowBytes, int rows)
    {
        auto* d = static_cast<std::uint8_t*>(dst);
        const auto* s = static_cast<const std::uint8_t*>(src);
        for (int y = 0; y < rows; ++y)
            std::memcpy(d + dstPitch * (std::size_t)y, s + srcPitch * (std::size_t)y, rowBytes);
    }

    float HalfToFloat(std::uint16_t h)
    {
        const int sign = (h >> 15) & 1, exp = (h >> 10) & 0x1F, mant = h & 0x3FF;
        float v;
        if (exp == 0)
            v = std::ldexp((float)mant, -24);                       // zero / subnormal
        else if (exp == 31)
            v = mant ? std::nanf("") : INFINITY;
        else
            v = std::ldexp((float)(mant | 0x400), exp - 25);
        return sign ? -v : v;
    }

    namespace
    {
        std::uint8_t Unorm8(float v)
        {
            if (!(v > 0.0f))   // negative and NaN
                return 0;
            if (v >= 1.0f)
                return 255;
            return (std::uint8_t)std::lround(v * 255.0f);
        }
    }

    bool ConvertToRgba8(Format format, const void* src, std::size_t srcPitch, int width, int height, std::uint8_t* dst)
    {
        const auto* base = static_cast<const std::uint8_t*>(src);
        for (int y = 0; y < height; ++y)
        {
            const std::uint8_t* s = base + srcPitch * (std::size_t)y;
            std::uint8_t* d = dst + (std::size_t)width * 4 * (std::size_t)y;
            for (int x = 0; x < width; ++x, d += 4)
            {
                switch (format)
                {
                case Format::RGBA8_UNORM:
                case Format::RGBA8_SRGB:
                    std::memcpy(d, s + x * 4, 4);
                    break;
                case Format::BGRA8_UNORM:
                case Format::BGRA8_SRGB:
                    d[0] = s[x * 4 + 2];
                    d[1] = s[x * 4 + 1];
                    d[2] = s[x * 4 + 0];
                    d[3] = s[x * 4 + 3];
                    break;
                case Format::RGB10A2_UNORM:
                {
                    std::uint32_t v;
                    std::memcpy(&v, s + x * 4, 4);
                    d[0] = (std::uint8_t)(((v & 0x3FF) * 255 + 511) / 1023);
                    d[1] = (std::uint8_t)((((v >> 10) & 0x3FF) * 255 + 511) / 1023);
                    d[2] = (std::uint8_t)((((v >> 20) & 0x3FF) * 255 + 511) / 1023);
                    d[3] = (std::uint8_t)((v >> 30) * 85);
                    break;
                }
                case Format::RGBA16_FLOAT:
                    for (int c = 0; c < 4; ++c)
                    {
                        std::uint16_t h;
                        std::memcpy(&h, s + x * 8 + c * 2, 2);
                        d[c] = Unorm8(HalfToFloat(h));
                    }
                    break;
                case Format::RGBA32_FLOAT:
                    for (int c = 0; c < 4; ++c)
                    {
                        float f;
                        std::memcpy(&f, s + x * 16 + c * 4, 4);
                        d[c] = Unorm8(f);
                    }
                    break;
                case Format::R8_UNORM:
                    d[0] = s[x];
                    d[1] = d[2] = 0;
                    d[3] = 255;
                    break;
                case Format::Unknown:
                    return false;
                }
            }
        }
        return true;
    }

    // ------------------------------------------------------------------ texture copies
    CopyPlan PlanCopy(const CopySide& src, const CopySide& dst)
    {
        CopyPlan p;
        if (src.format == Format::Unknown || dst.format == Format::Unknown)
        {
            p.why = "unknown format";
            return p;
        }
        if (dst.format != src.format && dst.format != RawFormat(src.format))
        {
            p.why = std::string("CopyTexture copies bits: ") + FormatName(src.format) + " into " + FormatName(dst.format);
            return p;
        }
        if (dst.samples != 1)
        {
            p.why = "multisampled destination";
            return p;
        }
        if (!src.copyable || !dst.copyable)
        {
            p.why = "framebufferOnly texture (set CAMetalLayer.framebufferOnly = NO for glass)";
            return p;
        }
        p.resolve = src.samples > 1;
        // the resolve scratch texture is the backend's own: it always has PixelFormatView
        const bool srcViewable = p.resolve || src.viewable;
        if (src.format != dst.format)
        {
            if (srcViewable)
            {
                p.view = CopyView::Source;
                p.viewFormat = dst.format;
            }
            else if (dst.viewable)
            {
                p.view = CopyView::Destination;
                p.viewFormat = src.format;
            }
            else
            {
                p.why = "copy between sRGB twins needs MTLTextureUsagePixelFormatView on one side";
                return p;
            }
        }
        p.valid = true;
        return p;
    }

    bool RectInside(const IRect& r, int width, int height)
    {
        return !r.Empty() && r.x0 >= 0 && r.y0 >= 0 && r.x1 <= width && r.y1 <= height;
    }
}
