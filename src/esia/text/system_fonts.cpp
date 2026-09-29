// Esia - installed fonts, the portable part (include/esia/text/system_fonts.hpp): directory scans, face matching and
// the fallback chain. The platforms' lookups are in system_fonts_<platform>.cpp.
#include "font_file.hpp"
#include <algorithm>
#include <system_error>

namespace esia::text
{
    namespace detail
    {
        namespace
        {
            bool IsFontFile(const std::filesystem::path& path)
            {
                std::string ext = PathToUtf8(path.extension());
                std::transform(ext.begin(), ext.end(), ext.begin(), [](char c) { return c >= 'A' && c <= 'Z' ? (char)(c - 'A' + 'a') : c; });
                return ext == ".ttf" || ext == ".otf" || ext == ".ttc" || ext == ".otc";
            }
        }

        std::vector<FontFileFace> ScanFontDirectories(std::span<const std::filesystem::path> directories)
        {
            namespace fs = std::filesystem;
            std::vector<FontFileFace> out;
            for (const fs::path& dir : directories)
            {
                std::error_code ec;
                if (!fs::is_directory(dir, ec))
                    continue;
                // symbolic links to directories are not followed: font trees link to each other in circles
                for (fs::recursive_directory_iterator it(dir, fs::directory_options::skip_permission_denied, ec), end; !ec && it != end;
                     it.increment(ec))
                {
                    if (!IsFontFile(it->path()) || !it->is_regular_file(ec))
                        continue;
                    for (FontFaceInfo& face : ReadFontFaces(it->path()))
                        out.push_back({it->path(), std::move(face)});
                }
            }
            return out;
        }

        std::optional<SystemFont> MatchFamily(std::span<const FontFileFace> fonts, std::string_view family, int weight, FontStyle style)
        {
            std::vector<const FontFileFace*> candidates;
            std::vector<FaceTraits> traits;
            for (const FontFileFace& f : fonts)
            {
                if (!HasFamily(f.face, family))
                    continue;
                candidates.push_back(&f);
                traits.push_back({f.face.weight, f.face.italic});
            }
            const int best = SelectFace(traits, weight, style);
            if (best < 0)
                return std::nullopt;
            const FontFileFace& f = *candidates[(std::size_t)best];
            return SystemFont{PathToUtf8(f.path), f.face.faceIndex, f.face.family, f.face.weight, f.face.italic ? FontStyle::Italic : FontStyle::Upright};
        }
    }

    namespace
    {
        int ClampWeight(int weight) { return std::clamp(weight, 1, 1000); }
    }

    std::optional<SystemFont> FindSystemFont(std::string_view family, int weight, FontStyle style)
    {
        if (family.empty())
            return std::nullopt;
        return detail::FindPlatformFont(family, ClampWeight(weight), style);
    }

    std::optional<SystemFont> FindFontInDirectories(std::span<const std::string> directories, std::string_view family, int weight, FontStyle style)
    {
        if (family.empty())
            return std::nullopt;
        std::vector<std::filesystem::path> dirs;
        for (const std::string& d : directories)
            dirs.push_back(detail::PathFromUtf8(d));
        return detail::MatchFamily(detail::ScanFontDirectories(dirs), family, ClampWeight(weight), style);
    }

    std::span<const std::string_view> DefaultFallbackFamilies() { return detail::PlatformFallbackFamilies(); }

    std::vector<SystemFont> FindDefaultFallbackFonts(int weight, FontStyle style)
    {
        std::vector<SystemFont> out;
        for (const std::string_view family : DefaultFallbackFamilies())
        {
            std::optional<SystemFont> font = FindSystemFont(family, weight, style);
            if (!font)
                continue;
            // "system-ui" and a family of the chain may be the same face
            const bool listed = std::any_of(out.begin(), out.end(), [&](const SystemFont& f) { return f.path == font->path && f.faceIndex == font->faceIndex; });
            if (!listed)
                out.push_back(std::move(*font));
        }
        return out;
    }

    FontId AddSystemFont(TextSystem& ts, const SystemFont& font) { return ts.AddFontFile(font.path.c_str(), font.faceIndex); }

    std::vector<FontId> AddFallbackFonts(TextSystem& ts, std::span<const SystemFont> fonts)
    {
        std::vector<FontId> ids;
        for (const SystemFont& font : fonts)
        {
            const FontId id = AddSystemFont(ts, font);
            if (id == 0)
                continue;
            ts.AddFallback(id);
            ids.push_back(id);
        }
        return ids;
    }
}
