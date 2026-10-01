// Esia test kit - PNG and image comparison (see image.hpp).
#include "image.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace esia::testkit
{
    namespace
    {
        // ------------------------------------------------------------------ checksums
        std::uint32_t Crc32(const std::uint8_t* p, std::size_t n, std::uint32_t crc = 0)
        {
            static std::uint32_t table[256];
            static bool init = false;
            if (!init)
            {
                for (std::uint32_t i = 0; i < 256; ++i)
                {
                    std::uint32_t c = i;
                    for (int k = 0; k < 8; ++k)
                        c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
                    table[i] = c;
                }
                init = true;
            }
            crc = ~crc;
            for (std::size_t i = 0; i < n; ++i)
                crc = table[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);
            return ~crc;
        }

        std::uint32_t Adler32(const std::uint8_t* p, std::size_t n)
        {
            std::uint32_t a = 1, b = 0;
            for (std::size_t i = 0; i < n; ++i)
            {
                a = (a + p[i]) % 65521u;
                b = (b + a) % 65521u;
            }
            return (b << 16) | a;
        }

        // ------------------------------------------------------------------ deflate tables (RFC 1951)
        const std::uint16_t kLenBase[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
        const std::uint8_t kLenExtra[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
        const std::uint16_t kDistBase[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073,
                                             4097, 6145, 8193, 12289, 16385, 24577};
        const std::uint8_t kDistExtra[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

        struct BitWriter
        {
            std::vector<std::uint8_t>& out;
            std::uint32_t buf = 0;
            int count = 0;
            void Put(std::uint32_t v, int n)   // LSB first
            {
                buf |= v << count;
                count += n;
                while (count >= 8)
                {
                    out.push_back((std::uint8_t)buf);
                    buf >>= 8;
                    count -= 8;
                }
            }
            void PutCode(std::uint32_t code, int n)   // Huffman codes go MSB first
            {
                std::uint32_t r = 0;
                for (int i = 0; i < n; ++i)
                    r |= ((code >> i) & 1u) << (n - 1 - i);
                Put(r, n);
            }
            void Flush()
            {
                if (count > 0)
                    out.push_back((std::uint8_t)buf);
                buf = 0;
                count = 0;
            }
        };

        void PutLiteral(BitWriter& w, int s)
        {
            if (s < 144)
                w.PutCode(0x30u + (std::uint32_t)s, 8);
            else if (s < 256)
                w.PutCode(0x190u + (std::uint32_t)(s - 144), 9);
            else if (s < 280)
                w.PutCode((std::uint32_t)(s - 256), 7);
            else
                w.PutCode(0xC0u + (std::uint32_t)(s - 280), 8);
        }

        void PutMatch(BitWriter& w, int len, int dist)
        {
            int li = 28;
            while (kLenBase[li] > len)
                --li;
            PutLiteral(w, 257 + li);
            if (kLenExtra[li])
                w.Put((std::uint32_t)(len - kLenBase[li]), kLenExtra[li]);
            int di = 29;
            while (kDistBase[di] > dist)
                --di;
            w.PutCode((std::uint32_t)di, 5);
            if (kDistExtra[di])
                w.Put((std::uint32_t)(dist - kDistBase[di]), kDistExtra[di]);
        }

        // ------------------------------------------------------------------ inflate (after zlib's puff.c)
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

        bool Codes(BitReader& r, std::vector<std::uint8_t>& out, const Huffman& lencode, const Huffman& distcode)
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
            }
        }

        bool InflateRaw(BitReader& r, std::vector<std::uint8_t>& out)
        {
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
                    if (len != (~nlen & 0xFFFFu) || r.pos + len > r.n)
                        return false;
                    out.insert(out.end(), r.p + r.pos, r.p + r.pos + len);
                    r.pos += len;
                }
                else if (type == 1)
                {
                    static Huffman lencode, distcode;
                    static bool built = false;
                    if (!built)
                    {
                        std::int16_t lengths[288];
                        int s = 0;
                        for (; s < 144; ++s) lengths[s] = 8;
                        for (; s < 256; ++s) lengths[s] = 9;
                        for (; s < 280; ++s) lengths[s] = 7;
                        for (; s < 288; ++s) lengths[s] = 8;
                        Build(lencode, lengths, 288);
                        for (s = 0; s < 30; ++s)
                            lengths[s] = 5;
                        Build(distcode, lengths, 30);
                        built = true;
                    }
                    if (!Codes(r, out, lencode, distcode))
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
                        int sym = Decode(r, lencode);
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
                    if (!Codes(r, out, lencode, distcode))
                        return false;
                }
                else
                    return false;
            } while (!last);
            return true;
        }

        // ------------------------------------------------------------------ PNG
        const std::uint8_t kSignature[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};

        void Put32(std::vector<std::uint8_t>& o, std::uint32_t v)
        {
            o.push_back((std::uint8_t)(v >> 24));
            o.push_back((std::uint8_t)(v >> 16));
            o.push_back((std::uint8_t)(v >> 8));
            o.push_back((std::uint8_t)v);
        }

        std::uint32_t Get32(const std::uint8_t* p) { return ((std::uint32_t)p[0] << 24) | ((std::uint32_t)p[1] << 16) | ((std::uint32_t)p[2] << 8) | p[3]; }

        void Chunk(std::vector<std::uint8_t>& o, const char* type, const std::vector<std::uint8_t>& data)
        {
            Put32(o, (std::uint32_t)data.size());
            const std::size_t start = o.size();
            o.insert(o.end(), type, type + 4);
            o.insert(o.end(), data.begin(), data.end());
            Put32(o, Crc32(o.data() + start, o.size() - start));
        }

        std::uint8_t Paeth(int a, int b, int c)
        {
            const int p = a + b - c, pa = std::abs(p - a), pb = std::abs(p - b), pc = std::abs(p - c);
            return (std::uint8_t)((pa <= pb && pa <= pc) ? a : (pb <= pc ? b : c));
        }

        bool Fail(std::string* error, const char* what)
        {
            if (error)
                *error = what;
            return false;
        }
    }

    std::vector<std::uint8_t> Deflate(const std::uint8_t* data, std::size_t size)
    {
        std::vector<std::uint8_t> out = {0x78, 0x01};   // zlib: deflate, 32K window, no dictionary
        BitWriter w{out};
        w.Put(1, 1);   // last block
        w.Put(1, 2);   // fixed Huffman codes
        constexpr int kWindow = 32768, kHashBits = 15, kChain = 48;
        std::vector<int> head((std::size_t)1 << kHashBits, -1), prev((std::size_t)kWindow, -1);
        auto hash = [&](std::size_t i) { return (std::size_t)(((data[i] << 10) ^ (data[i + 1] << 5) ^ data[i + 2]) & ((1 << kHashBits) - 1)); };
        std::size_t i = 0;
        while (i < size)
        {
            int bestLen = 0, bestDist = 0;
            if (i + 3 <= size)
            {
                const std::size_t h = hash(i);
                int cand = head[h];
                const int maxLen = (int)std::min<std::size_t>(258, size - i);
                for (int chain = 0; cand >= 0 && chain < kChain && (int)i - cand <= kWindow; ++chain)
                {
                    int len = 0;
                    while (len < maxLen && data[(std::size_t)cand + (std::size_t)len] == data[i + (std::size_t)len])
                        ++len;
                    if (len > bestLen)
                    {
                        bestLen = len;
                        bestDist = (int)i - cand;
                        if (len == maxLen)
                            break;
                    }
                    cand = prev[(std::size_t)cand % kWindow];
                }
            }
            const std::size_t step = bestLen >= 3 ? (std::size_t)bestLen : 1;
            if (bestLen >= 3)
                PutMatch(w, bestLen, bestDist);
            else
                PutLiteral(w, data[i]);
            for (std::size_t k = 0; k < step; ++k, ++i)
                if (i + 3 <= size)
                {
                    const std::size_t h = hash(i);
                    prev[i % kWindow] = head[h];
                    head[h] = (int)i;
                }
        }
        PutLiteral(w, 256);
        w.Flush();
        Put32(out, Adler32(data, size));
        return out;
    }

    bool Inflate(const std::uint8_t* data, std::size_t size, std::vector<std::uint8_t>& out)
    {
        if (size < 6 || (data[0] & 0x0F) != 8 || ((data[0] << 8) | data[1]) % 31 != 0 || (data[1] & 0x20))
            return false;
        BitReader r{data + 2, size - 6};
        out.clear();
        if (!InflateRaw(r, out))
            return false;
        return Get32(data + size - 4) == Adler32(out.data(), out.size());
    }

    std::vector<std::uint8_t> EncodePng(const Image& img)
    {
        // adaptive filtering: per row, the filter whose output has the smallest sum of |signed bytes|
        const std::size_t stride = (std::size_t)img.width * 4;
        std::vector<std::uint8_t> raw;
        raw.reserve((stride + 1) * (std::size_t)img.height);
        std::vector<std::uint8_t> best(stride), trial(stride);
        const std::vector<std::uint8_t> zero(stride, 0);
        for (int y = 0; y < img.height; ++y)
        {
            const std::uint8_t* row = img.rgba.data() + stride * (std::size_t)y;
            const std::uint8_t* up = y > 0 ? row - stride : zero.data();
            long bestCost = -1;
            int bestFilter = 0;
            for (int f = 0; f < 5; ++f)
            {
                long cost = 0;
                for (std::size_t x = 0; x < stride; ++x)
                {
                    const int a = x >= 4 ? row[x - 4] : 0, b = up[x], c = x >= 4 ? up[x - 4] : 0;
                    int v = row[x];
                    switch (f)
                    {
                    case 1: v -= a; break;
                    case 2: v -= b; break;
                    case 3: v -= (a + b) / 2; break;
                    case 4: v -= Paeth(a, b, c); break;
                    default: break;
                    }
                    trial[x] = (std::uint8_t)v;
                    cost += std::abs((int)(std::int8_t)trial[x]);
                }
                if (bestCost < 0 || cost < bestCost)
                {
                    bestCost = cost;
                    bestFilter = f;
                    best.swap(trial);
                }
            }
            raw.push_back((std::uint8_t)bestFilter);
            raw.insert(raw.end(), best.begin(), best.end());
        }
        std::vector<std::uint8_t> png(kSignature, kSignature + 8);
        std::vector<std::uint8_t> ihdr;
        Put32(ihdr, (std::uint32_t)img.width);
        Put32(ihdr, (std::uint32_t)img.height);
        ihdr.insert(ihdr.end(), {8, 6, 0, 0, 0});   // 8-bit RGBA, deflate, adaptive filters, no interlace
        Chunk(png, "IHDR", ihdr);
        Chunk(png, "IDAT", Deflate(raw.data(), raw.size()));
        Chunk(png, "IEND", {});
        return png;
    }

    bool DecodePng(const std::uint8_t* data, std::size_t size, Image& out, std::string* error)
    {
        if (size < 8 || std::memcmp(data, kSignature, 8) != 0)
            return Fail(error, "not a PNG file");
        std::size_t pos = 8;
        int w = 0, h = 0, colorType = -1, channels = 0;
        std::vector<std::uint8_t> idat;
        bool end = false;
        while (pos + 12 <= size && !end)
        {
            const std::uint32_t len = Get32(data + pos);
            if (pos + 12 + len > size)
                return Fail(error, "truncated chunk");
            const std::uint8_t* type = data + pos + 4;
            const std::uint8_t* body = type + 4;
            if (Crc32(type, len + 4) != Get32(body + len))
                return Fail(error, "chunk CRC mismatch");
            if (std::memcmp(type, "IHDR", 4) == 0)
            {
                if (len < 13)
                    return Fail(error, "bad IHDR");
                w = (int)Get32(body);
                h = (int)Get32(body + 4);
                colorType = body[9];
                if (body[8] != 8 || body[12] != 0 || body[10] != 0 || body[11] != 0)
                    return Fail(error, "only 8-bit, non-interlaced PNGs are supported");
                channels = colorType == 6 ? 4 : colorType == 2 ? 3 : colorType == 4 ? 2 : colorType == 0 ? 1 : 0;
                if (!channels || w <= 0 || h <= 0)
                    return Fail(error, "unsupported color type");
            }
            else if (std::memcmp(type, "IDAT", 4) == 0)
                idat.insert(idat.end(), body, body + len);
            else if (std::memcmp(type, "IEND", 4) == 0)
                end = true;
            pos += 12 + len;
        }
        if (!channels)
            return Fail(error, "no IHDR");
        std::vector<std::uint8_t> raw;
        if (!Inflate(idat.data(), idat.size(), raw))
            return Fail(error, "corrupt image data");
        const std::size_t stride = (std::size_t)w * (std::size_t)channels;
        if (raw.size() != (stride + 1) * (std::size_t)h)
            return Fail(error, "image data size mismatch");
        std::vector<std::uint8_t> pixels(stride * (std::size_t)h);
        for (int y = 0; y < h; ++y)
        {
            const int f = raw[(stride + 1) * (std::size_t)y];
            const std::uint8_t* src = raw.data() + (stride + 1) * (std::size_t)y + 1;
            std::uint8_t* row = pixels.data() + stride * (std::size_t)y;
            const std::uint8_t* up = y > 0 ? row - stride : nullptr;
            for (std::size_t x = 0; x < stride; ++x)
            {
                const int a = x >= (std::size_t)channels ? row[x - (std::size_t)channels] : 0, b = up ? up[x] : 0;
                const int c = up && x >= (std::size_t)channels ? up[x - (std::size_t)channels] : 0;
                int v = src[x];
                switch (f)
                {
                case 0: break;
                case 1: v += a; break;
                case 2: v += b; break;
                case 3: v += (a + b) / 2; break;
                case 4: v += Paeth(a, b, c); break;
                default: return Fail(error, "bad filter type");
                }
                row[x] = (std::uint8_t)v;
            }
        }
        out = Image(w, h);
        for (std::size_t i = 0; i < (std::size_t)w * (std::size_t)h; ++i)
        {
            const std::uint8_t* s = pixels.data() + i * (std::size_t)channels;
            std::uint8_t* d = out.rgba.data() + i * 4;
            switch (channels)
            {
            case 4: std::memcpy(d, s, 4); break;
            case 3: d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = 255; break;
            case 2: d[0] = d[1] = d[2] = s[0]; d[3] = s[1]; break;
            default: d[0] = d[1] = d[2] = s[0]; d[3] = 255; break;
            }
        }
        return true;
    }

    bool WritePng(const std::string& path, const Image& image, std::string* error)
    {
        const std::vector<std::uint8_t> png = EncodePng(image);
        std::FILE* f = std::fopen(path.c_str(), "wb");
        if (!f)
            return Fail(error, "cannot create the file");
        const bool ok = std::fwrite(png.data(), 1, png.size(), f) == png.size();
        return (std::fclose(f) == 0 && ok) || Fail(error, "write failed");
    }

    bool ReadPng(const std::string& path, Image& out, std::string* error)
    {
        std::FILE* f = std::fopen(path.c_str(), "rb");
        if (!f)
            return Fail(error, "cannot open the file");
        std::vector<std::uint8_t> data;
        std::uint8_t buf[65536];
        std::size_t n;
        while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0)
            data.insert(data.end(), buf, buf + n);
        std::fclose(f);
        return DecodePng(data.data(), data.size(), out, error);
    }

    CompareResult Compare(const Image& image, const Image& golden, const Tolerance& t, Image* diff)
    {
        CompareResult r;
        r.sizeMatches = image.width == golden.width && image.height == golden.height && !image.Empty();
        if (!r.sizeMatches)
            return r;
        if (diff)
            *diff = Image(image.width, image.height);
        double sum = 0.0;
        for (int y = 0; y < image.height; ++y)
            for (int x = 0; x < image.width; ++x)
            {
                const std::uint8_t* a = image.At(x, y);
                const std::uint8_t* b = golden.At(x, y);
                int d = 0;
                for (int c = 0; c < 4; ++c)
                {
                    const int e = std::abs((int)a[c] - (int)b[c]);
                    d = std::max(d, e);
                    sum += e;
                }
                r.maxDelta = std::max(r.maxDelta, d);
                const bool over = d > t.channel;
                r.differing += over ? 1u : 0u;
                if (diff)
                {
                    std::uint8_t* o = diff->At(x, y);
                    if (over)
                    {
                        o[0] = 255;
                        o[1] = o[2] = 0;
                    }
                    else
                        for (int c = 0; c < 3; ++c)
                            o[c] = (std::uint8_t)(b[c] / 4);
                    o[3] = 255;
                }
            }
        const double n = (double)image.width * (double)image.height;
        r.fraction = (double)r.differing / n;
        r.meanDelta = sum / (n * 4.0);
        r.pass = r.fraction <= t.fraction && r.maxDelta <= t.maxDelta && r.meanDelta <= t.meanDelta;
        return r;
    }
}
