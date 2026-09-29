// Esia - installed fonts on Linux and other Unix systems (include/esia/text/system_fonts.hpp): fontconfig when Esia
// was built with it (ESIA_TEXT_FONTCONFIG), else a scan of the standard font directories, made once per process.
#include "font_file.hpp"
#include <algorithm>
#include <cstdlib>
#include <mutex>

#if ESIA_TEXT_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif

namespace esia::text::detail
{
    namespace
    {
        constexpr std::string_view kChain[] = {
            "Noto Sans",         "Noto Sans CJK SC",    "Noto Sans CJK TC",  "Noto Sans CJK JP",   "Noto Sans CJK KR",
            "DejaVu Sans",       "Noto Sans Symbols",   "Noto Sans Symbols 2", "WenQuanYi Zen Hei", "Droid Sans Fallback",
        };
        // system-ui: the first of these that is installed
        constexpr std::string_view kUiFamilies[] = {"Noto Sans", "DejaVu Sans"};

#if ESIA_TEXT_FONTCONFIG
        // Generic names fontconfig resolves itself (it answers them with some family: never compared with the result).
        bool IsGeneric(std::string_view family)
        {
            for (const std::string_view g : {"sans-serif", "serif", "monospace", "cursive", "fantasy", "emoji", "math"})
                if (EqualsIgnoreCase(family, g))
                    return true;
            return false;
        }

        std::optional<SystemFont> FindFontconfig(std::string_view family, int weight, FontStyle style)
        {
            FcPattern* pattern = FcPatternCreate();
            if (!pattern)
                return std::nullopt;
            const std::string name(family);
            FcPatternAddString(pattern, FC_FAMILY, reinterpret_cast<const FcChar8*>(name.c_str()));
            FcPatternAddInteger(pattern, FC_WEIGHT, FcWeightFromOpenType(weight));
            FcPatternAddInteger(pattern, FC_SLANT, style == FontStyle::Italic ? FC_SLANT_ITALIC : FC_SLANT_ROMAN);
            FcPatternAddBool(pattern, FC_OUTLINE, FcTrue);
            FcConfigSubstitute(nullptr, pattern, FcMatchPattern);
            FcDefaultSubstitute(pattern);
            FcResult result = FcResultNoMatch;
            FcPattern* match = FcFontMatch(nullptr, pattern, &result);
            FcPatternDestroy(pattern);
            if (!match)
                return std::nullopt;

            std::optional<SystemFont> out;
            // fontconfig always answers, with a substitute when the family is missing: only the family itself counts
            bool same = IsGeneric(family);
            FcChar8* value = nullptr;
            std::string first;
            for (int i = 0; FcPatternGetString(match, FC_FAMILY, i, &value) == FcResultMatch; ++i)
            {
                const std::string_view f(reinterpret_cast<const char*>(value));
                if (i == 0)
                    first = f;
                same = same || EqualsIgnoreCase(f, family);
            }
            FcChar8* file = nullptr;
            int index = 0, fcWeight = FC_WEIGHT_REGULAR, slant = FC_SLANT_ROMAN;
            FcBool outline = FcFalse;
            if (same && FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch &&
                FcPatternGetBool(match, FC_OUTLINE, 0, &outline) == FcResultMatch && outline)
            {
                FcPatternGetInteger(match, FC_INDEX, 0, &index);
                FcPatternGetInteger(match, FC_WEIGHT, 0, &fcWeight);
                FcPatternGetInteger(match, FC_SLANT, 0, &slant);
                SystemFont font;
                font.path = reinterpret_cast<const char*>(file);
                font.faceIndex = index & 0xFFFF;   // the high bits number a variable font's named instance
                font.family = first;
                font.weight = FcWeightToOpenType(fcWeight);
                font.style = slant == FC_SLANT_ROMAN ? FontStyle::Upright : FontStyle::Italic;
                out = std::move(font);
            }
            FcPatternDestroy(match);
            return out;
        }
#else
        std::vector<std::filesystem::path> StandardFontDirectories()
        {
            namespace fs = std::filesystem;
            std::vector<fs::path> dirs;
            const char* home = std::getenv("HOME");
            const char* dataHome = std::getenv("XDG_DATA_HOME");
            if (dataHome && *dataHome)
                dirs.push_back(fs::path(dataHome) / "fonts");
            else if (home && *home)
                dirs.push_back(fs::path(home) / ".local/share/fonts");
            if (home && *home)
                dirs.push_back(fs::path(home) / ".fonts");
            const char* dataDirs = std::getenv("XDG_DATA_DIRS");
            const std::string_view list = dataDirs && *dataDirs ? dataDirs : "/usr/local/share:/usr/share";
            for (std::size_t p = 0; p <= list.size();)
            {
                const std::size_t q = std::min(list.find(':', p), list.size());
                if (q > p)
                    dirs.push_back(fs::path(list.substr(p, q - p)) / "fonts");
                p = q + 1;
            }
            for (const char* d : {"/usr/share/fonts", "/usr/local/share/fonts", "/system/fonts"})   // the last: Android
                dirs.emplace_back(d);
            // a directory listed twice would be scanned twice
            std::vector<fs::path> unique;
            for (fs::path& d : dirs)
            {
                d = d.lexically_normal();
                if (std::find(unique.begin(), unique.end(), d) == unique.end())
                    unique.push_back(d);
            }
            return unique;
        }

        std::optional<SystemFont> FindScanned(std::string_view family, int weight, FontStyle style)
        {
            static std::mutex mutex;
            static std::vector<FontFileFace> fonts;
            static bool scanned = false;
            std::lock_guard lock(mutex);
            if (!scanned)
            {
                fonts = ScanFontDirectories(StandardFontDirectories());
                scanned = true;
            }
            return MatchFamily(fonts, family, weight, style);
        }
#endif

        std::optional<SystemFont> Find(std::string_view family, int weight, FontStyle style)
        {
#if ESIA_TEXT_FONTCONFIG
            return FindFontconfig(family, weight, style);
#else
            return FindScanned(family, weight, style);
#endif
        }
    }

    std::optional<SystemFont> FindPlatformFont(std::string_view family, int weight, FontStyle style)
    {
        if (EqualsIgnoreCase(family, kSystemUiFamily))
        {
            for (const std::string_view f : kUiFamilies)
                if (std::optional<SystemFont> font = Find(f, weight, style))
                    return font;
            return std::nullopt;
        }
        return Find(family, weight, style);
    }

    std::span<const std::string_view> PlatformFallbackFamilies() { return kChain; }
}
