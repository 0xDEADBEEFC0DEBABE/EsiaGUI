// Esia - the PNG images of color bitmap fonts (CBDT, sbix), for a FreeType built without libpng (Esia's bundled
// FreeType): Android's flags font, the CBDT Noto Color Emoji of Linux distributions. Gray, gray + alpha, RGB, RGBA and
// palette images (with tRNS) of every bit depth, not interlaced - what the emoji fonts hold (palette images: pngquant).
#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace esia::text::detail
{
    // `data` decoded to straight-alpha RGBA8, rows top first; false for a file it cannot read.
    bool DecodePng(const std::uint8_t* data, std::size_t size, int& width, int& height, std::vector<std::uint8_t>& rgba);
}
