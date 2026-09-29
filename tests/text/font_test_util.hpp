// Font files for the text tests: reading a font, and a font collection (.ttc) put together from single fonts, with
// the weight and style of each face changeable - the lookup's face selection and FreeType's collection support are
// tested with the fonts in tests/fonts, without a collection file in the repository.
#pragma once
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace esia::texttest
{
    inline const std::string kFonts = ESIA_TEST_FONT_DIR;

    inline std::vector<std::uint8_t> ReadFile(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary);
        return std::vector<std::uint8_t>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

    struct CollectionFace
    {
        std::vector<std::uint8_t> font;   // a single TrueType / OpenType font
        int weight = 0;                   // > 0: written into OS/2 usWeightClass
        bool italic = false;              // written into OS/2 fsSelection (with weight > 0)
    };

    // A TrueType collection of the faces, in order. Table offsets in a collection count from the start of the file,
    // so each font's table directory is moved by the place its copy lands at; the checksums are left as they are.
    inline std::vector<std::uint8_t> MakeCollection(const std::vector<CollectionFace>& faces)
    {
        auto u16 = [](const std::uint8_t* p) { return (std::uint32_t)(p[0] << 8 | p[1]); };
        auto u32 = [](const std::uint8_t* p) { return (std::uint32_t)p[0] << 24 | (std::uint32_t)p[1] << 16 | (std::uint32_t)p[2] << 8 | p[3]; };
        auto put16 = [](std::uint8_t* p, std::uint32_t v) {
            p[0] = (std::uint8_t)(v >> 8);
            p[1] = (std::uint8_t)v;
        };
        auto put32 = [](std::uint8_t* p, std::uint32_t v) {
            for (int i = 0; i < 4; ++i)
                p[i] = (std::uint8_t)(v >> (24 - 8 * i));
        };
        std::vector<std::uint8_t> out(12 + 4 * faces.size());
        put32(out.data(), 0x74746366);   // 'ttcf'
        put32(out.data() + 4, 0x00010000);
        put32(out.data() + 8, (std::uint32_t)faces.size());
        for (std::size_t f = 0; f < faces.size(); ++f)
        {
            while (out.size() % 4)
                out.push_back(0);
            const std::uint32_t base = (std::uint32_t)out.size();
            put32(out.data() + 12 + 4 * f, base);
            out.insert(out.end(), faces[f].font.begin(), faces[f].font.end());
            std::uint8_t* font = out.data() + base;
            const std::uint32_t tables = u16(font + 4);
            for (std::uint32_t t = 0; t < tables; ++t)
            {
                std::uint8_t* record = font + 12 + 16 * t;
                const std::uint32_t offset = u32(record + 8);
                put32(record + 8, offset + base);
                if (faces[f].weight > 0 && u32(record) == 0x4F532F32)   // 'OS/2'
                {
                    std::uint8_t* os2 = font + offset;
                    put16(os2 + 4, (std::uint32_t)faces[f].weight);
                    put16(os2 + 62, faces[f].italic ? 0x01u : 0x40u);   // ITALIC or REGULAR
                }
            }
        }
        return out;
    }

    inline bool WriteFile(const std::string& path, const std::vector<std::uint8_t>& bytes)
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        file.write(reinterpret_cast<const char*>(bytes.data()), (std::streamsize)bytes.size());
        return (bool)file;
    }
}
