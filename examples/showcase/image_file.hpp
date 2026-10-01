// showcase - image files (the wallpaper) as straight-alpha RGBA8: the Windows Imaging Component (image_file.cpp), ImageIO
// on Apple systems (image_file_apple.mm), PNG and JPEG on Linux (image_file_linux.cpp).
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace showcase
{
    struct ImageFile
    {
        int width = 0, height = 0;
        std::vector<std::uint8_t> rgba;
    };

    // JPEG, PNG, BMP ... `maxSide` > 0: scaled down (Fant) so that neither side is longer. False when it cannot be read.
    bool LoadImageFile(const std::wstring& path, ImageFile& out, int maxSide = 0);
}
