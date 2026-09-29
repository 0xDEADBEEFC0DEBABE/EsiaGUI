// WGT demo - WIC based image loading / PNG saving
#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct ImageRGBA
{
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> pixels;   // RGBA8
};

bool LoadImageRGBA(const std::wstring& path, ImageRGBA& out, int maxDimension = 0);
// Writes BGRA or RGBA pixels (rowPitch in bytes) to PNG.
bool SavePng(const std::wstring& path, const std::uint8_t* pixels, int width, int height, int rowPitch, bool bgra);
