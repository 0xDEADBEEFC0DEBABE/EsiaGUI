// Test kit: zlib streams, PNG files, image comparison.
#include "image.hpp"
#include "esia_test.hpp"
#include <cstdio>

using namespace esia::testkit;

ESIA_TEST(TestKit, DeflateRoundTrip)
{
    std::vector<std::uint8_t> data;
    for (int i = 0; i < 70000; ++i)   // longer than the 32K window, with repeats near and far
        data.push_back((std::uint8_t)((i % 251) ^ (i / 1000)));
    const std::vector<std::uint8_t> z = Deflate(data.data(), data.size());
    std::vector<std::uint8_t> back;
    ESIA_CHECK(Inflate(z.data(), z.size(), back) && back == data);
    ESIA_CHECK(z.size() < data.size() / 2);
    const std::vector<std::uint8_t> empty = Deflate(nullptr, 0);
    ESIA_CHECK(Inflate(empty.data(), empty.size(), back) && back.empty());
}

ESIA_TEST(TestKit, InflatesDynamicHuffmanBlocks)
{
    // zlib.compress(level 9) of 685 bytes: a dynamic-Huffman block, as other encoders write them
    const std::uint8_t z[] = {0x78, 0xda, 0xad, 0xd0, 0x41, 0x0a, 0xc0, 0x20, 0x10, 0x43, 0x51, 0x67, 0x12, 0x3d, 0x87, 0x97, 0xf1, 0x20, 0x22, 0x2d, 0x74,
                              0x51, 0x85, 0xf6, 0xfe, 0xe0, 0x1c, 0xa1, 0x81, 0xae, 0x1e, 0x64, 0x15, 0x7e, 0x32, 0xba, 0xd3, 0xd2, 0x77, 0x3c, 0x03, 0xd9,
                              0x4d, 0xa0, 0x90, 0x05, 0x0e, 0x01, 0xf1, 0x52, 0x20, 0x5e, 0x0a, 0xc4, 0x4b, 0x81, 0x78, 0x29, 0x30, 0x39, 0x16, 0xe4, 0x58,
                              0x94, 0x63, 0x41, 0x8e, 0x45, 0x39, 0x56, 0x7b, 0xaf, 0x5e, 0xc7, 0x9a, 0xe7, 0x7a, 0xee, 0x3e, 0xc7, 0x51, 0xff, 0x18, 0x36,
                              0x95, 0x49, 0x26, 0x89};
    std::vector<std::uint8_t> out;
    ESIA_CHECK(Inflate(z, sizeof(z), out));
    ESIA_CHECK(out.size() == 685);
    bool same = out.size() == 685;
    for (int i = 0; same && i < 600; ++i)
        same = out[(std::size_t)i] == (std::uint8_t)((i * i) % 7 + (i / 50) % 3);
    ESIA_CHECK(same);
    ESIA_CHECK(std::string(out.begin() + 600, out.begin() + 617) == "Esia conformance ");
}

ESIA_TEST(TestKit, ReadsForeignRgbPng)
{
    // a 7 x 5 RGB PNG with Sub-filtered rows written by Python's zlib / struct
    const std::uint8_t png[] = {
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        0x05, 0x08, 0x02, 0x00, 0x00, 0x00, 0x06, 0xf8, 0x61, 0x8f, 0x00, 0x00, 0x00, 0x34, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda, 0x63, 0x64, 0x60,
        0xf8, 0xaf, 0xca, 0xfa, 0x46, 0x95, 0xff, 0x8d, 0xaa, 0xe4, 0x1b, 0x55, 0xe5, 0x37, 0xaa, 0xba, 0x6f, 0x54, 0xcd, 0xdf, 0x30, 0x72, 0x33,
        0xff, 0xc0, 0x22, 0x2a, 0xc6, 0xf6, 0x11, 0x8b, 0xa8, 0x22, 0xe7, 0x2b, 0x2c, 0xa2, 0x3a, 0x3c, 0x8f, 0x31, 0x45, 0x01, 0x92, 0x3d, 0x28,
        0xc9, 0x63, 0xb6, 0x8a, 0x93, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};
    Image img;
    std::string error;
    ESIA_CHECK(DecodePng(png, sizeof(png), img, &error));
    ESIA_CHECK(img.width == 7 && img.height == 5);
    bool same = img.width == 7 && img.height == 5;
    for (int y = 0; same && y < 5; ++y)
        for (int x = 0; same && x < 7; ++x)
        {
            const std::uint8_t* p = img.At(x, y);
            same = p[0] == (x * 37 + y * 11) % 256 && p[1] == (x * x * 5 + y * 3) % 256 && p[2] == (255 - x * 20 - y * 7) % 256 && p[3] == 255;
        }
    ESIA_CHECK(same);
}

ESIA_TEST(TestKit, PngRoundTripAndRejectsCorruption)
{
    Image img(61, 37);
    for (int y = 0; y < img.height; ++y)
        for (int x = 0; x < img.width; ++x)
        {
            std::uint8_t* p = img.At(x, y);
            p[0] = (std::uint8_t)(x * 4);
            p[1] = (std::uint8_t)(y * 7);
            p[2] = (std::uint8_t)((x * y) & 0xFF);
            p[3] = (std::uint8_t)(x < 30 ? 255 : 128);
        }
    std::vector<std::uint8_t> png = EncodePng(img);
    Image back;
    ESIA_CHECK(DecodePng(png.data(), png.size(), back) && back.width == 61 && back.height == 37 && back.rgba == img.rgba);
    png[png.size() / 2] ^= 0x40;   // any flipped bit breaks a CRC
    std::string error;
    ESIA_CHECK(!DecodePng(png.data(), png.size(), back, &error) && !error.empty());
}

ESIA_TEST(TestKit, CompareWithTolerance)
{
    Image a(10, 10), b(10, 10);
    for (std::size_t i = 0; i < a.rgba.size(); ++i)
        a.rgba[i] = b.rgba[i] = 100;
    b.At(3, 3)[1] = 105;   // within the channel tolerance
    Tolerance t;
    t.channel = 8;
    t.fraction = 0.015;
    CompareResult r = Compare(a, b, t);
    ESIA_CHECK(r.pass && r.maxDelta == 5 && r.differing == 0);
    b.At(4, 4)[0] = 140;   // one pixel off: 1 % of the image
    Image diff;
    r = Compare(a, b, t, &diff);
    ESIA_CHECK(r.pass && r.differing == 1 && r.maxDelta == 40);
    ESIA_CHECK(diff.At(4, 4)[0] == 255 && diff.At(4, 4)[1] == 0);
    b.At(4, 4)[0] = 200;   // far off: over the default maxDelta (48), although 1 % of the pixels may differ
    ESIA_CHECK(!Compare(a, b, t).pass);
    t.maxDelta = 128;
    ESIA_CHECK(Compare(a, b, t).pass);
    b.At(5, 5)[2] = 0;   // a second one: 2 % > 1.5 %
    t.maxDelta = 255;
    ESIA_CHECK(!Compare(a, b, t).pass);
    ESIA_CHECK(!Compare(a, Image(9, 10), t).sizeMatches);
    // off by 2 in two channels everywhere: no pixel over `channel`, a mean of 1 over the default meanDelta (0.5)
    Image c = a;
    for (std::size_t i = 0; i < c.rgba.size(); i += 4)
        c.rgba[i] = c.rgba[i + 1] = 102;
    r = Compare(c, a, Tolerance());
    ESIA_CHECK(!r.pass && r.differing == 0 && r.maxDelta == 2 && r.meanDelta == 1.0);
}
