// Esia - Vulkan backend: formats (RHI <-> VkFormat) and the readback conversion to RGBA8.
#pragma once
#include "vk_loader.hpp"
#include "esia/rhi/rhi.hpp"
#include <cstddef>

namespace esia::rhi::vulkan
{
    // The format of the raw (never sRGB-decoding) view of an image: what the backend samples render targets through.
    VkFormat RawVkFormat(VkFormat f);

    // Converts `count` pixels of `format` (as stored: sRGB values are not decoded) to RGBA8. R8 becomes (r, 0, 0, 255)
    // like a GL readback of a red texture; float formats are clamped to [0, 1] and rounded.
    void ConvertToRgba8(Format format, const void* src, std::size_t count, std::uint8_t* rgba8);

    float HalfToFloat(std::uint16_t h);
}
