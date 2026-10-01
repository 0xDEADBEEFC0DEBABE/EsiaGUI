// Esia - font file facts for the system font lookup (font_file.hpp).
#include "font_file.hpp"
#include <algorithm>
#include <fstream>

namespace esia::text::detail
{
    namespace
    {
        constexpr std::uint32_t Tag(const char (&t)[5])
        {
            return (std::uint32_t)(unsigned char)t[0] << 24 | (std::uint32_t)(unsigned char)t[1] << 16 | (std::uint32_t)(unsigned char)t[2] << 8 |
                   (std::uint32_t)(unsigned char)t[3];
        }

        constexpr std::uint32_t kMaxFaces = 256;          // collections hold a handful; anything above is not a font
        constexpr std::uint32_t kMaxNameTable = 1u << 20;   // 'name' tables are a few KB; bounded against broken files

        std::uint16_t U16(const std::uint8_t* p) { return (std::uint16_t)(p[0] << 8 | p[1]); }
        std::uint32_t U32(const std::uint8_t* p) { return (std::uint32_t)p[0] << 24 | (std::uint32_t)p[1] << 16 | (std::uint32_t)p[2] << 8 | p[3]; }

        struct Reader
        {
            std::ifstream file;
            std::uint64_t size = 0;

            bool Read(std::uint64_t offset, std::size_t count, std::uint8_t* out)
            {
                if (offset > size || count > size - offset)
                    return false;
                file.clear();
                file.seekg((std::streamoff)offset);
                return (bool)file.read(reinterpret_cast<char*>(out), (std::streamsize)count);
            }
        };

        bool IsSfntVersion(std::uint32_t v) { return v == 0x00010000u || v == Tag("OTTO") || v == Tag("true"); }

        void AppendUtf8(std::string& out, char32_t c)
        {
            if (c < 0x80)
                out += (char)c;
            else if (c < 0x800)
            {
                out += (char)(0xC0 | (c >> 6));
                out += (char)(0x80 | (c & 0x3F));
            }
            else if (c < 0x10000)
            {
                out += (char)(0xE0 | (c >> 12));
                out += (char)(0x80 | ((c >> 6) & 0x3F));
                out += (char)(0x80 | (c & 0x3F));
            }
            else
            {
                out += (char)(0xF0 | (c >> 18));
                out += (char)(0x80 | ((c >> 12) & 0x3F));
                out += (char)(0x80 | ((c >> 6) & 0x3F));
                out += (char)(0x80 | (c & 0x3F));
            }
        }

        std::string FromUtf16Be(const std::uint8_t* p, std::size_t bytes)
        {
            std::string out;
            for (std::size_t i = 0; i + 1 < bytes; i += 2)
            {
                char32_t c = U16(p + i);
                if (c >= 0xD800 && c < 0xDC00 && i + 3 < bytes)
                {
                    const char32_t lo = U16(p + i + 2);
                    if (lo >= 0xDC00 && lo < 0xE000)
                    {
                        c = 0x10000 + ((c - 0xD800) << 10) + (lo - 0xDC00);
                        i += 2;
                    }
                }
                if (c == 0 || (c >= 0xD800 && c < 0xE000))
                    continue;
                AppendUtf8(out, c);
            }
            return out;
        }

        struct NameRecord
        {
            std::uint16_t platform, encoding, language, id;
            std::string text;
        };

        // Unicode and Windows names (UTF-16BE), and the ASCII-only Macintosh Roman ones (old fonts have no others).
        std::vector<NameRecord> ReadNames(const std::uint8_t* t, std::size_t size)
        {
            std::vector<NameRecord> out;
            if (size < 6)
                return out;
            const std::size_t count = U16(t + 2), strings = U16(t + 4);
            for (std::size_t i = 0; i < count && 6 + (i + 1) * 12 <= size; ++i)
            {
                const std::uint8_t* r = t + 6 + i * 12;
                NameRecord n{U16(r), U16(r + 2), U16(r + 4), U16(r + 6), {}};
                if (n.id != 1 && n.id != 6 && n.id != 16)
                    continue;
                const std::size_t length = U16(r + 8), offset = strings + U16(r + 10);
                if (offset > size || length > size - offset)
                    continue;
                const std::uint8_t* s = t + offset;
                if (n.platform == 0 || n.platform == 3)
                    n.text = FromUtf16Be(s, length);
                else if (n.platform == 1 && n.encoding == 0 && std::all_of(s, s + length, [](std::uint8_t b) { return b >= 0x20 && b < 0x80; }))
                    n.text.assign(reinterpret_cast<const char*>(s), length);
                if (!n.text.empty())
                    out.push_back(std::move(n));
            }
            return out;
        }

        // English (US, then any English, then Macintosh English) before other languages
        int LanguageRank(const NameRecord& n)
        {
            if (n.platform == 3 && n.language == 0x409)
                return 0;
            if (n.platform == 3 && (n.language & 0x3FF) == 0x09)
                return 1;
            if (n.platform == 1 && n.language == 0)
                return 2;
            return 3;
        }

        bool ReadFace(Reader& r, std::uint64_t offset, int index, FontFaceInfo& face)
        {
            std::uint8_t header[12];
            if (!r.Read(offset, sizeof(header), header) || !IsSfntVersion(U32(header)))
                return false;
            const std::size_t tables = U16(header + 4);
            std::vector<std::uint8_t> dir(tables * 16);
            if (!r.Read(offset + 12, dir.size(), dir.data()))
                return false;
            std::uint32_t nameAt = 0, nameSize = 0, os2At = 0, os2Size = 0, headAt = 0, headSize = 0;
            for (std::size_t i = 0; i < tables; ++i)
            {
                const std::uint8_t* e = dir.data() + i * 16;
                const std::uint32_t tag = U32(e), at = U32(e + 8), size = U32(e + 12);
                if (tag == Tag("name"))
                    nameAt = at, nameSize = size;
                else if (tag == Tag("OS/2"))
                    os2At = at, os2Size = size;
                else if (tag == Tag("head"))
                    headAt = at, headSize = size;
            }
            if (nameSize == 0 || nameSize > kMaxNameTable)
                return false;
            // table offsets count from the start of the file, in collections too
            std::vector<std::uint8_t> name(nameSize);
            if (!r.Read(nameAt, name.size(), name.data()))
                return false;
            std::vector<NameRecord> names = ReadNames(name.data(), name.size());
            std::stable_sort(names.begin(), names.end(), [](const NameRecord& a, const NameRecord& b) { return LanguageRank(a) < LanguageRank(b); });

            face.faceIndex = index;
            const NameRecord* typographic = nullptr;
            const NameRecord* legacy = nullptr;
            for (const NameRecord& n : names)
            {
                if (n.id == 6)
                {
                    if (face.postscriptName.empty())
                        face.postscriptName = n.text;
                    continue;
                }
                if (n.id == 16 && !typographic)
                    typographic = &n;
                if (n.id == 1 && !legacy)
                    legacy = &n;
                if (std::none_of(face.families.begin(), face.families.end(), [&](const std::string& f) { return f == n.text; }))
                    face.families.push_back(n.text);
            }
            if (!typographic && !legacy)
                return false;
            face.family = (typographic ? typographic : legacy)->text;

            std::uint8_t os2[64];
            if (os2Size >= sizeof(os2) && r.Read(os2At, sizeof(os2), os2))
            {
                int w = U16(os2 + 4);
                if (w > 0 && w < 10)
                    w *= 100;   // a few old fonts use 1 - 9
                if (w >= 1 && w <= 1000)
                    face.weight = w;
                face.italic = (U16(os2 + 62) & 0x0201) != 0;   // ITALIC or OBLIQUE
            }
            std::uint8_t head[46];
            if (headSize >= sizeof(head) && r.Read(headAt, sizeof(head), head))
                face.italic = face.italic || (U16(head + 44) & 2) != 0;   // macStyle italic
            return true;
        }

        char Lower(char c) { return c >= 'A' && c <= 'Z' ? (char)(c - 'A' + 'a') : c; }

        // CSS Fonts 4, 5.2 step 4: how far `have` is from `want` in the order the browser tries weights
        int WeightRank(int want, int have)
        {
            if (have == want)
                return 0;
            if (want >= 400 && want <= 500)
            {
                if (have > want && have <= 500)
                    return have - want;
                if (have < want)
                    return 1000 + (want - have);
                return 2000 + (have - want);
            }
            if (want < 400)
                return have < want ? want - have : 1000 + (have - want);
            return have > want ? have - want : 1000 + (want - have);
        }
    }

    std::vector<FontFaceInfo> ReadFontFaces(const std::filesystem::path& path)
    {
        std::vector<FontFaceInfo> out;
        Reader r;
        r.file.open(path, std::ios::binary | std::ios::ate);
        if (!r.file)
            return out;
        const std::streamoff size = r.file.tellg();
        if (size < 12)
            return out;
        r.size = (std::uint64_t)size;
        std::uint8_t header[12];
        if (!r.Read(0, sizeof(header), header))
            return out;
        if (U32(header) == Tag("ttcf"))
        {
            const std::uint32_t faces = std::min(U32(header + 8), kMaxFaces);
            std::vector<std::uint8_t> offsets(faces * 4);
            if (!r.Read(12, offsets.size(), offsets.data()))
                return out;
            for (std::uint32_t i = 0; i < faces; ++i)
            {
                FontFaceInfo face;
                if (ReadFace(r, U32(offsets.data() + i * 4), (int)i, face))
                    out.push_back(std::move(face));
            }
        }
        else
        {
            FontFaceInfo face;
            if (ReadFace(r, 0, 0, face))
                out.push_back(std::move(face));
        }
        return out;
    }

    bool EqualsIgnoreCase(std::string_view a, std::string_view b)
    {
        return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) { return Lower(x) == Lower(y); });
    }

    bool HasFamily(const FontFaceInfo& face, std::string_view family)
    {
        return std::any_of(face.families.begin(), face.families.end(), [&](const std::string& f) { return EqualsIgnoreCase(f, family); });
    }

    int SelectFace(std::span<const FaceTraits> faces, int weight, FontStyle style)
    {
        const bool italic = style == FontStyle::Italic;
        int best = -1;
        std::pair<bool, int> bestKey{};
        for (std::size_t i = 0; i < faces.size(); ++i)
        {
            const std::pair<bool, int> key{faces[i].italic != italic, WeightRank(weight, faces[i].weight)};
            if (best < 0 || key < bestKey)
            {
                best = (int)i;
                bestKey = key;
            }
        }
        return best;
    }

    std::string PathToUtf8(const std::filesystem::path& path)
    {
        const std::u8string s = path.u8string();
        return std::string(reinterpret_cast<const char*>(s.data()), s.size());
    }

    std::filesystem::path PathFromUtf8(std::string_view utf8)
    {
        return std::filesystem::path(std::u8string_view(reinterpret_cast<const char8_t*>(utf8.data()), utf8.size()));
    }
}
