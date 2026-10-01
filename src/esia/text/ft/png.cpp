// Esia - PNG decoding for color bitmap fonts (png.hpp). Inflate after zlib's puff.c, as in the test kit's image
// reader (tests/support/image.cpp); checksums are not verified (a font's images are read where they lie, a damaged
// one fails on its structure or draws wrong, never out of bounds).
#include "png.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace esia::text::detail
{
    namespace
    {
        constexpr int kMaxSide = 4096;   // larger than any glyph's strike

        const std::uint16_t kLenBase[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
        const std::uint8_t kLenExtra[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
        const std::uint16_t kDistBase[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073,
                                             4097, 6145, 8193, 12289, 16385, 24577};
        const std::uint8_t kDistExtra[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

        struct BitReader
        {
            const std::uint8_t* p;
            std::size_t n, pos = 0;
            std::uint32_t buf = 0;
            int count = 0;
            bool error = false;
            int Bits(int need)
            {
                std::uint32_t v = buf;
                while (count < need)
                {
                    if (pos >= n)
                    {
                        error = true;
                        return 0;
                    }
                    v |= (std::uint32_t)p[pos++] << count;
                    count += 8;
                }
                buf = v >> need;
                count -= need;
                return (int)(v & ((1u << need) - 1u));
            }
        };

        struct Huffman
        {
            std::int16_t count[16];
            std::int16_t symbol[320];
        };

        bool Build(Huffman& h, const std::int16_t* lengths, int n)
        {
            std::memset(h.count, 0, sizeof(h.count));
            for (int s = 0; s < n; ++s)
                ++h.count[lengths[s]];
            if (h.count[0] == n)
                return true;
            int left = 1;
            for (int len = 1; len < 16; ++len)
            {
                left <<= 1;
                left -= h.count[len];
                if (left < 0)
                    return false;   // over-subscribed
            }
            std::int16_t offs[16];
            offs[1] = 0;
            for (int len = 1; len < 15; ++len)
                offs[len + 1] = (std::int16_t)(offs[len] + h.count[len]);
            for (int s = 0; s < n; ++s)
                if (lengths[s] != 0)
                    h.symbol[offs[lengths[s]]++] = (std::int16_t)s;
            return true;
        }

        int Decode(BitReader& r, const Huffman& h)
        {
            int code = 0, first = 0, index = 0;
            for (int len = 1; len < 16; ++len)
            {
                code |= r.Bits(1);
                if (r.error)
                    return -1;
                const int count = h.count[len];
                if (code - count < first)
                    return h.symbol[index + (code - first)];
                index += count;
                first += count;
                first <<= 1;
                code <<= 1;
            }
            return -1;
        }

        bool Codes(BitReader& r, std::vector<std::uint8_t>& out, const Huffman& lencode, const Huffman& distcode, std::size_t limit)
        {
            for (;;)
            {
                int s = Decode(r, lencode);
                if (s < 0)
                    return false;
                if (s < 256)
                    out.push_back((std::uint8_t)s);
                else if (s == 256)
                    return true;
                else
                {
                    s -= 257;
                    if (s >= 29)
                        return false;
                    const int len = kLenBase[s] + r.Bits(kLenExtra[s]);
                    const int ds = Decode(r, distcode);
                    if (ds < 0 || ds >= 30)
                        return false;
                    const std::size_t dist = (std::size_t)(kDistBase[ds] + r.Bits(kDistExtra[ds]));
                    if (r.error || dist > out.size())
                        return false;
                    const std::size_t from = out.size() - dist;
                    for (int i = 0; i < len; ++i)
                        out.push_back(out[from + (std::size_t)i]);
                }
                if (out.size() > limit)
                    return false;
            }
        }

        // `limit`: the image's size - more data than that is a damaged stream
        bool Inflate(const std::uint8_t* data, std::size_t size, std::vector<std::uint8_t>& out, std::size_t limit)
        {
            if (size < 2 || (data[0] & 0x0F) != 8 || ((data[0] << 8) | data[1]) % 31 != 0 || (data[1] & 0x20))
                return false;
            BitReader r{data + 2, size - 2};
            out.clear();
            out.reserve(limit);
            int last = 0;
            do
            {
                last = r.Bits(1);
                const int type = r.Bits(2);
                if (r.error)
                    return false;
                if (type == 0)
                {
                    r.buf = 0;
                    r.count = 0;
                    if (r.pos + 4 > r.n)
                        return false;
                    const unsigned len = r.p[r.pos] | (r.p[r.pos + 1] << 8);
                    const unsigned nlen = r.p[r.pos + 2] | (r.p[r.pos + 3] << 8);
                    r.pos += 4;
                    if (len != (~nlen & 0xFFFFu) || r.pos + len > r.n || out.size() + len > limit)
                        return false;
                    out.insert(out.end(), r.p + r.pos, r.p + r.pos + len);
                    r.pos += len;
                }
                else if (type == 1)
                {
                    static const auto fixed = [] {
                        struct Fixed
                        {
                            Huffman lencode, distcode;
                        } f{};
                        std::int16_t lengths[288];
                        int s = 0;
                        for (; s < 144; ++s) lengths[s] = 8;
                        for (; s < 256; ++s) lengths[s] = 9;
                        for (; s < 280; ++s) lengths[s] = 7;
                        for (; s < 288; ++s) lengths[s] = 8;
                        Build(f.lencode, lengths, 288);
                        for (s = 0; s < 30; ++s)
                            lengths[s] = 5;
                        Build(f.distcode, lengths, 30);
                        return f;
                    }();
                    if (!Codes(r, out, fixed.lencode, fixed.distcode, limit))
                        return false;
                }
                else if (type == 2)
                {
                    const int nlen = r.Bits(5) + 257, ndist = r.Bits(5) + 1, ncode = r.Bits(4) + 4;
                    if (r.error || nlen > 286 || ndist > 30)
                        return false;
                    static const std::uint8_t order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
                    std::int16_t lengths[320] = {};
                    for (int i = 0; i < ncode; ++i)
                        lengths[order[i]] = (std::int16_t)r.Bits(3);
                    Huffman lencode, distcode;
                    if (r.error || !Build(lencode, lengths, 19))
                        return false;
                    int i = 0;
                    while (i < nlen + ndist)
                    {
                        const int sym = Decode(r, lencode);
                        if (sym < 0)
                            return false;
                        if (sym < 16)
                            lengths[i++] = (std::int16_t)sym;
                        else
                        {
                            std::int16_t len = 0;
                            int rep;
                            if (sym == 16)
                            {
                                if (i == 0)
                                    return false;
                                len = lengths[i - 1];
                                rep = 3 + r.Bits(2);
                            }
                            else if (sym == 17)
                                rep = 3 + r.Bits(3);
                            else
                                rep = 11 + r.Bits(7);
                            if (r.error || i + rep > nlen + ndist)
                                return false;
                            while (rep--)
                                lengths[i++] = len;
                        }
                    }
                    if (lengths[256] == 0 || !Build(lencode, lengths, nlen) || !Build(distcode, lengths + nlen, ndist))
                        return false;
                    if (!Codes(r, out, lencode, distcode, limit))
                        return false;
                }
                else
                    return false;
            } while (!last);
            return true;
        }

        std::uint32_t Get32(const std::uint8_t* p) { return ((std::uint32_t)p[0] << 24) | ((std::uint32_t)p[1] << 16) | ((std::uint32_t)p[2] << 8) | p[3]; }

        std::uint8_t Paeth(int a, int b, int c)
        {
            const int p = a + b - c, pa = std::abs(p - a), pb = std::abs(p - b), pc = std::abs(p - c);
            return (std::uint8_t)((pa <= pb && pa <= pc) ? a : (pb <= pc ? b : c));
        }
    }

    bool DecodePng(const std::uint8_t* data, std::size_t size, int& width, int& height, std::vector<std::uint8_t>& rgba)
    {
        static const std::uint8_t kSignature[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
        if (!data || size < 8 || std::memcmp(data, kSignature, 8) != 0)
            return false;
        std::size_t pos = 8;
        int w = 0, h = 0, depth = 0, colorType = -1, channels = 0;
        std::uint8_t palette[256][4] = {};
        int paletteSize = 0;
        int key[3] = {-1, -1, -1};   // tRNS of gray / RGB images: the transparent color
        std::vector<std::uint8_t> idat;
        while (pos + 12 <= size)
        {
            const std::uint32_t len = Get32(data + pos);
            if (len > size - pos - 12)
                return false;
            const std::uint8_t* type = data + pos + 4;
            const std::uint8_t* body = type + 4;
            if (std::memcmp(type, "IHDR", 4) == 0)
            {
                if (len < 13)
                    return false;
                const std::uint32_t uw = Get32(body), uh = Get32(body + 4);
                depth = body[8];
                colorType = body[9];
                if (uw == 0 || uh == 0 || uw > (std::uint32_t)kMaxSide || uh > (std::uint32_t)kMaxSide || body[10] != 0 || body[11] != 0 || body[12] != 0)
                    return false;   // empty, too large, or interlaced
                w = (int)uw;
                h = (int)uh;
                channels = colorType == 6 ? 4 : colorType == 2 ? 3 : colorType == 4 ? 2 : (colorType == 0 || colorType == 3) ? 1 : 0;
                const bool depthOk = colorType == 0 ? (depth == 1 || depth == 2 || depth == 4 || depth == 8 || depth == 16)
                                     : colorType == 3 ? (depth == 1 || depth == 2 || depth == 4 || depth == 8)
                                                      : (depth == 8 || depth == 16);
                if (!channels || !depthOk)
                    return false;
            }
            else if (std::memcmp(type, "PLTE", 4) == 0)
            {
                paletteSize = (int)std::min<std::uint32_t>(len / 3, 256);
                for (int i = 0; i < paletteSize; ++i)
                {
                    palette[i][0] = body[i * 3];
                    palette[i][1] = body[i * 3 + 1];
                    palette[i][2] = body[i * 3 + 2];
                    palette[i][3] = 255;
                }
            }
            else if (std::memcmp(type, "tRNS", 4) == 0)
            {
                if (colorType == 3)
                    for (std::uint32_t i = 0; i < len && i < 256; ++i)
                        palette[i][3] = body[i];
                else if (colorType == 0 && len >= 2)
                    key[0] = (body[0] << 8) | body[1];
                else if (colorType == 2 && len >= 6)
                    for (int c = 0; c < 3; ++c)
                        key[c] = (body[c * 2] << 8) | body[c * 2 + 1];
            }
            else if (std::memcmp(type, "IDAT", 4) == 0)
                idat.insert(idat.end(), body, body + len);
            else if (std::memcmp(type, "IEND", 4) == 0)
                break;
            pos += 12 + (std::size_t)len;
        }
        if (!channels || (colorType == 3 && paletteSize == 0))
            return false;

        // unfiltered rows: `bpp` bytes per pixel for the filters (at least one), `stride` bytes per row
        const int bits = depth * channels;
        const std::size_t stride = ((std::size_t)w * (std::size_t)bits + 7) / 8, bpp = (std::size_t)std::max(1, bits / 8);
        std::vector<std::uint8_t> raw;
        if (!Inflate(idat.data(), idat.size(), raw, (stride + 1) * (std::size_t)h) || raw.size() != (stride + 1) * (std::size_t)h)
            return false;
        std::vector<std::uint8_t> rows(stride * (std::size_t)h);
        for (int y = 0; y < h; ++y)
        {
            const int f = raw[(stride + 1) * (std::size_t)y];
            const std::uint8_t* src = raw.data() + (stride + 1) * (std::size_t)y + 1;
            std::uint8_t* row = rows.data() + stride * (std::size_t)y;
            const std::uint8_t* up = y > 0 ? row - stride : nullptr;
            for (std::size_t x = 0; x < stride; ++x)
            {
                const int a = x >= bpp ? row[x - bpp] : 0, b = up ? up[x] : 0, c = up && x >= bpp ? up[x - bpp] : 0;
                int v = src[x];
                switch (f)
                {
                case 0: break;
                case 1: v += a; break;
                case 2: v += b; break;
                case 3: v += (a + b) / 2; break;
                case 4: v += Paeth(a, b, c); break;
                default: return false;
                }
                row[x] = (std::uint8_t)v;
            }
        }

        // samples: sub-byte depths from the high bits first, 16-bit ones by their high byte (the key by its full value)
        const auto sample = [&](const std::uint8_t* row, int index) -> int {
            if (depth == 16)
                return (row[index * 2] << 8) | row[index * 2 + 1];
            if (depth == 8)
                return row[index];
            const int perByte = 8 / depth, shift = 8 - depth * (index % perByte + 1);
            return (row[index / perByte] >> shift) & ((1 << depth) - 1);
        };
        const auto to8 = [&](int v) -> std::uint8_t { return (std::uint8_t)(depth == 16 ? v >> 8 : v * 255 / ((1 << depth) - 1)); };
        width = w;
        height = h;
        rgba.assign((std::size_t)w * (std::size_t)h * 4, 0);
        for (int y = 0; y < h; ++y)
        {
            const std::uint8_t* row = rows.data() + stride * (std::size_t)y;
            std::uint8_t* d = rgba.data() + (std::size_t)y * (std::size_t)w * 4;
            for (int x = 0; x < w; ++x, d += 4)
            {
                switch (colorType)
                {
                case 3:
                {
                    const int i = sample(row, x);
                    if (i < paletteSize)
                        std::memcpy(d, palette[i], 4);
                    break;
                }
                case 0:
                {
                    const int g = sample(row, x);
                    d[0] = d[1] = d[2] = to8(g);
                    d[3] = g == key[0] ? 0 : 255;
                    break;
                }
                case 4:
                    d[0] = d[1] = d[2] = to8(sample(row, x * 2));
                    d[3] = to8(sample(row, x * 2 + 1));
                    break;
                case 2:
                {
                    const int r = sample(row, x * 3), g = sample(row, x * 3 + 1), b = sample(row, x * 3 + 2);
                    d[0] = to8(r);
                    d[1] = to8(g);
                    d[2] = to8(b);
                    d[3] = (r == key[0] && g == key[1] && b == key[2]) ? 0 : 255;
                    break;
                }
                default:
                    for (int c = 0; c < 4; ++c)
                        d[c] = to8(sample(row, x * 4 + c));
                    break;
                }
            }
        }
        return true;
    }
}
