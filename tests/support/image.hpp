// Esia test kit - RGBA8 images, PNG files and tolerant image comparison (the conformance suite's goldens).
//
// Self-contained on purpose (no zlib / libpng): the suite must build wherever a backend does, Windows included.
// The PNG writer uses LZ77 + fixed Huffman codes and adaptive row filters; the reader inflates any zlib stream
// (stored, fixed and dynamic blocks) of 8-bit gray, gray + alpha, RGB and RGBA images, not interlaced.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace esia::testkit
{
    struct Image
    {
        int width = 0, height = 0;
        std::vector<std::uint8_t> rgba;   // top row first, 4 bytes per pixel (straight alpha)

        Image() = default;
        Image(int w, int h) : width(w), height(h), rgba((std::size_t)w * (std::size_t)h * 4, 0) {}
        bool Empty() const { return width <= 0 || height <= 0; }
        std::uint8_t* At(int x, int y) { return rgba.data() + ((std::size_t)y * (std::size_t)width + (std::size_t)x) * 4; }
        const std::uint8_t* At(int x, int y) const { return rgba.data() + ((std::size_t)y * (std::size_t)width + (std::size_t)x) * 4; }
    };

    std::vector<std::uint8_t> EncodePng(const Image& image);
    bool DecodePng(const std::uint8_t* data, std::size_t size, Image& out, std::string* error = nullptr);
    bool WritePng(const std::string& path, const Image& image, std::string* error = nullptr);
    bool ReadPng(const std::string& path, Image& out, std::string* error = nullptr);

    // zlib streams (exposed for tests)
    std::vector<std::uint8_t> Deflate(const std::uint8_t* data, std::size_t size);
    bool Inflate(const std::uint8_t* data, std::size_t size, std::vector<std::uint8_t>& out);

    // How far a rendering may be from its golden: GPUs and APIs differ in filtering precision, float16 pyramids and
    // rounding, never in what is drawn where. A pixel "differs" when a channel is more than `channel` away; the
    // image passes when at most `fraction` of the pixels differ, no channel is more than `maxDelta` away and the mean
    // channel difference is at most `meanDelta`.
    // The fraction alone lets through what the other two catch: a missing hairline or glyph is a few hundred pixels
    // off by ~200 (maxDelta), a wrong blend, gamma or blur radius moves most pixels by a little, under `channel`
    // (meanDelta). The defaults leave headroom over an NVIDIA GPU's D3D9 - 12, GL, GLES and Vulkan renderings
    // against the llvmpipe goldens: max delta 25 (D3D9 msaa_target, on 0.02 % of the pixels), mean delta <= 0.09.
    struct Tolerance
    {
        int channel = 8;
        double fraction = 0.005;
        int maxDelta = 48;
        double meanDelta = 0.5;
    };

    struct CompareResult
    {
        bool sizeMatches = false;
        int maxDelta = 0;             // largest channel difference
        std::size_t differing = 0;    // pixels over Tolerance::channel
        double fraction = 0.0;
        double meanDelta = 0.0;       // mean channel difference over all pixels
        bool pass = false;
    };

    // `diff` (optional): the differing pixels in red over a faded copy of the golden.
    CompareResult Compare(const Image& image, const Image& golden, const Tolerance& tolerance, Image* diff = nullptr);
}
