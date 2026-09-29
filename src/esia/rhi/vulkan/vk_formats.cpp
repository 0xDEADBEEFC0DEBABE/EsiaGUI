// Esia - Vulkan backend: formats (see vk_formats.hpp)
#include "vk_formats.hpp"
#include "esia/rhi/vulkan.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace esia::rhi::vulkan
{
    VkFormat ToVkFormat(Format f)
    {
        switch (f)
        {
        case Format::RGBA8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
        case Format::RGBA8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
        case Format::BGRA8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
        case Format::BGRA8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
        // DXGI's R10G10B10A2 (red in the low bits) is Vulkan's A2B10G10R10 pack
        case Format::RGB10A2_UNORM: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case Format::RGBA16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case Format::RGBA32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Format::R8_UNORM: return VK_FORMAT_R8_UNORM;
        case Format::Unknown: break;
        }
        return VK_FORMAT_UNDEFINED;
    }

    Format FromVkFormat(VkFormat f)
    {
        switch (f)
        {
        case VK_FORMAT_R8G8B8A8_UNORM: return Format::RGBA8_UNORM;
        case VK_FORMAT_R8G8B8A8_SRGB: return Format::RGBA8_SRGB;
        case VK_FORMAT_B8G8R8A8_UNORM: return Format::BGRA8_UNORM;
        case VK_FORMAT_B8G8R8A8_SRGB: return Format::BGRA8_SRGB;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return Format::RGB10A2_UNORM;
        case VK_FORMAT_R16G16B16A16_SFLOAT: return Format::RGBA16_FLOAT;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return Format::RGBA32_FLOAT;
        case VK_FORMAT_R8_UNORM: return Format::R8_UNORM;
        default: return Format::Unknown;
        }
    }

    VkFormat RawVkFormat(VkFormat f)
    {
        switch (f)
        {
        case VK_FORMAT_R8G8B8A8_SRGB: return VK_FORMAT_R8G8B8A8_UNORM;
        case VK_FORMAT_B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_UNORM;
        default: return f;
        }
    }

    float HalfToFloat(std::uint16_t h)
    {
        const int sign = (h >> 15) & 1, exponent = (h >> 10) & 0x1F, mantissa = h & 0x3FF;
        float v;
        if (exponent == 0)
            v = std::ldexp((float)mantissa, -24);   // subnormal
        else if (exponent == 31)
            v = mantissa ? NAN : INFINITY;
        else
            v = std::ldexp((float)(mantissa | 0x400), exponent - 25);
        return sign ? -v : v;
    }

    namespace
    {
        std::uint8_t Unorm8(float v)
        {
            if (!(v > 0.0f))   // NaN too
                return 0;
            return (std::uint8_t)std::lround(std::min(v, 1.0f) * 255.0f);
        }
    }

    void ConvertToRgba8(Format format, const void* src, std::size_t count, std::uint8_t* out)
    {
        const std::uint8_t* s = static_cast<const std::uint8_t*>(src);
        switch (format)
        {
        case Format::RGBA8_UNORM:
        case Format::RGBA8_SRGB:
            std::memcpy(out, s, count * 4);
            break;
        case Format::BGRA8_UNORM:
        case Format::BGRA8_SRGB:
            for (std::size_t i = 0; i < count; ++i, s += 4, out += 4)
            {
                out[0] = s[2];
                out[1] = s[1];
                out[2] = s[0];
                out[3] = s[3];
            }
            break;
        case Format::RGB10A2_UNORM:
            for (std::size_t i = 0; i < count; ++i, s += 4, out += 4)
            {
                std::uint32_t p;
                std::memcpy(&p, s, 4);
                out[0] = (std::uint8_t)(((p & 0x3FF) * 255 + 511) / 1023);
                out[1] = (std::uint8_t)((((p >> 10) & 0x3FF) * 255 + 511) / 1023);
                out[2] = (std::uint8_t)((((p >> 20) & 0x3FF) * 255 + 511) / 1023);
                out[3] = (std::uint8_t)(((p >> 30) * 255 + 1) / 3);
            }
            break;
        case Format::RGBA16_FLOAT:
            for (std::size_t i = 0; i < count; ++i, s += 8, out += 4)
                for (int c = 0; c < 4; ++c)
                {
                    std::uint16_t h;
                    std::memcpy(&h, s + c * 2, 2);
                    out[c] = Unorm8(HalfToFloat(h));
                }
            break;
        case Format::RGBA32_FLOAT:
            for (std::size_t i = 0; i < count; ++i, s += 16, out += 4)
                for (int c = 0; c < 4; ++c)
                {
                    float f;
                    std::memcpy(&f, s + c * 4, 4);
                    out[c] = Unorm8(f);
                }
            break;
        case Format::R8_UNORM:
            for (std::size_t i = 0; i < count; ++i, out += 4)
            {
                out[0] = s[i];
                out[1] = out[2] = 0;
                out[3] = 255;
            }
            break;
        case Format::Unknown:
            std::memset(out, 0, count * 4);
            break;
        }
    }
}
