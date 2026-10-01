// Esia - what the system font lookup needs to know about a font file without a font library: the faces of an
// OpenType / TrueType font or collection with their family names, PostScript name, weight and style (the 'name',
// 'OS/2' and 'head' tables), and the choice of a face by weight and style. Internal to esia_text.
#pragma once
#include "esia/text/system_fonts.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace esia::text::detail
{
    struct FontFaceInfo
    {
        int faceIndex = 0;
        std::vector<std::string> families;   // every family name (IDs 1 and 16) in every language, UTF-8
        std::string family;                  // the typographic family (ID 16, else 1), English when present
        std::string postscriptName;          // ID 6
        int weight = 400;
        bool italic = false;
    };

    // The faces of a font file (reads the table directories and three small tables, not the whole file). Empty when
    // the file is not a font.
    std::vector<FontFaceInfo> ReadFontFaces(const std::filesystem::path& file);

    // Does `family` name the face? (any of its names, ASCII case-insensitive)
    bool HasFamily(const FontFaceInfo& face, std::string_view family);
    bool EqualsIgnoreCase(std::string_view a, std::string_view b);

    // CSS font matching (CSS Fonts 4, "font-weight" and "font-style"): the index of the best of `faces` for the
    // request, style first, then weight; -1 when `faces` is empty.
    struct FaceTraits
    {
        int weight = 400;
        bool italic = false;
    };
    int SelectFace(std::span<const FaceTraits> faces, int weight, FontStyle style);

    std::string PathToUtf8(const std::filesystem::path& path);
    std::filesystem::path PathFromUtf8(std::string_view utf8);

    // ---- directory scans (FindFontInDirectories; the system lookup where no platform API is used)
    struct FontFileFace
    {
        std::filesystem::path path;
        FontFaceInfo face;
    };
    // The faces of every font file (.ttf, .otf, .ttc, .otc) in `directories` and below; missing directories are
    // skipped.
    std::vector<FontFileFace> ScanFontDirectories(std::span<const std::filesystem::path> directories);
    // The best face of `family` among `fonts`.
    std::optional<SystemFont> MatchFamily(std::span<const FontFileFace> fonts, std::string_view family, int weight, FontStyle style);

    // ---- the platform's lookup (system_fonts_<platform>.cpp); FindSystemFont adds nothing but argument checks
    std::optional<SystemFont> FindPlatformFont(std::string_view family, int weight, FontStyle style);
    std::span<const std::string_view> PlatformFallbackFamilies();
}
