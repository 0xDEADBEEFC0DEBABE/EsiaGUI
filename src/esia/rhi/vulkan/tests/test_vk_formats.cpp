// Vulkan backend: format mapping and the readback conversion (no GPU needed).
#include "esia_test.hpp"
#include "esia/rhi/vulkan.hpp"
#include "vk_formats.hpp"
#include <cstring>

using namespace esia::rhi;
using namespace esia::rhi::vulkan;

ESIA_TEST(VulkanFormats, RoundTrip)
{
    for (Format f : {Format::RGBA8_UNORM, Format::RGBA8_SRGB, Format::BGRA8_UNORM, Format::BGRA8_SRGB, Format::RGB10A2_UNORM, Format::RGBA16_FLOAT,
                     Format::RGBA32_FLOAT, Format::R8_UNORM})
    {
        ESIA_CHECK(ToVkFormat(f) != VK_FORMAT_UNDEFINED);
        ESIA_CHECK(FromVkFormat(ToVkFormat(f)) == f);
        // the raw view of a format is the view of its RawFormat
        ESIA_CHECK(RawVkFormat(ToVkFormat(f)) == ToVkFormat(RawFormat(f)));
    }
    ESIA_CHECK(ToVkFormat(Format::Unknown) == VK_FORMAT_UNDEFINED);
    ESIA_CHECK(FromVkFormat(VK_FORMAT_D32_SFLOAT) == Format::Unknown);
}

ESIA_TEST(VulkanFormats, Half)
{
    ESIA_CHECK(HalfToFloat(0x3C00) == 1.0f);
    ESIA_CHECK(HalfToFloat(0xC000) == -2.0f);
    ESIA_CHECK(HalfToFloat(0x3800) == 0.5f);
    ESIA_CHECK(HalfToFloat(0x0001) > 0.0f);   // subnormal
    ESIA_CHECK(HalfToFloat(0x0000) == 0.0f);
}

ESIA_TEST(VulkanFormats, ConvertToRgba8)
{
    std::uint8_t out[8];
    const std::uint8_t bgra[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    ConvertToRgba8(Format::BGRA8_UNORM, bgra, 2, out);
    ESIA_CHECK(out[0] == 3 && out[1] == 2 && out[2] == 1 && out[3] == 4 && out[4] == 7 && out[7] == 8);

    const std::uint32_t packed = 1023u | (512u << 10) | (0u << 20) | (3u << 30);   // red in the low bits
    ConvertToRgba8(Format::RGB10A2_UNORM, &packed, 1, out);
    ESIA_CHECK(out[0] == 255 && out[1] == 128 && out[2] == 0 && out[3] == 255);

    const std::uint16_t half[4] = {0x3C00, 0x3800, 0xBC00, 0x4000};   // 1, 0.5, -1, 2
    ConvertToRgba8(Format::RGBA16_FLOAT, half, 1, out);
    ESIA_CHECK(out[0] == 255 && out[1] == 128 && out[2] == 0 && out[3] == 255);

    const float f32[4] = {0.25f, 1.5f, -0.1f, 0.0f};
    ConvertToRgba8(Format::RGBA32_FLOAT, f32, 1, out);
    ESIA_CHECK(out[0] == 64 && out[1] == 255 && out[2] == 0 && out[3] == 0);

    const std::uint8_t r8[2] = {7, 200};
    ConvertToRgba8(Format::R8_UNORM, r8, 2, out);
    ESIA_CHECK(out[0] == 7 && out[1] == 0 && out[2] == 0 && out[3] == 255 && out[4] == 200);
}
