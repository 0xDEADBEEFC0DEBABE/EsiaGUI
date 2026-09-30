// Esia - installed fonts: where the platform keeps a font family, and a default fallback chain per platform that
// covers Latin, Chinese (Simplified and Traditional), Japanese, Korean and symbols.
//
// The lookup only locates font files; every text system reads them itself and draws them with the shared rasterizer:
//   * Windows: DirectWrite's system font collection (the user's own fonts included),
//   * macOS: Core Text font descriptors, resolved to their file URLs,
//   * Linux and other Unix: fontconfig when Esia was built with it, else the standard font directories
//     ($XDG_DATA_HOME/fonts, ~/.fonts, $XDG_DATA_DIRS/fonts, /usr/share/fonts, /usr/local/share/fonts), scanned once.
// Families match any of their localized names ("Microsoft YaHei" and "微软雅黑" alike), ASCII case-insensitively;
// the face is chosen by style first, then by weight with the CSS font-matching rules. Color emoji fonts are not part
// of the chains: the rasterizer draws outlines only.
//
// Threading: every function may be called from any thread.
#pragma once
#include "esia/text/text.hpp"
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace esia::text
{
    enum class FontStyle : std::uint8_t
    {
        Upright,
        Italic,   // oblique faces count as italic
    };

    // One face of a font file on disk.
    struct SystemFont
    {
        std::string path;        // UTF-8
        int faceIndex = 0;       // face in a collection (.ttc / .otc); 0 for a single font
        std::string family;      // the family name the face reports (English when it has one)
        // The face as stored in the file, which is what a text system loads: never a simulated bold / oblique, and a
        // variable font's default instance. It can differ from the request when the family has no closer face.
        int weight = 400;        // 100 (thin) - 900 (black)
        FontStyle style = FontStyle::Upright;
    };

    // The platform's user-interface font: SF on macOS, Segoe UI on Windows, Noto Sans or else DejaVu Sans on Linux.
    inline constexpr std::string_view kSystemUiFamily = "system-ui";

    // The installed face of `family` closest to `weight` (100 - 900; 400 regular, 700 bold) and `style`, or nothing
    // when no such family is installed (a family is never substituted by another one).
    ESIA_API std::optional<SystemFont> FindSystemFont(std::string_view family, int weight = 400, FontStyle style = FontStyle::Upright);

    // The same search among the font files (.ttf, .otf, .ttc, .otc) in `directories` and their subdirectories, read
    // on every call (fonts shipped with an application, tests).
    ESIA_API std::optional<SystemFont> FindFontInDirectories(std::span<const std::string> directories, std::string_view family,
                                                             int weight = 400, FontStyle style = FontStyle::Upright);

    // This platform's fallback chain, in order:
    //   Windows  Segoe UI, Microsoft YaHei, Microsoft JhengHei, Yu Gothic UI, Malgun Gothic, Nirmala UI, Leelawadee UI,
    //            Ebrima, Gadugi, Myanmar Text, Javanese Text, Segoe UI Historic, Segoe UI Symbol, Segoe UI Emoji
    //   macOS    system-ui (SF), Helvetica Neue, Hiragino Sans GB, Heiti SC, Heiti TC, Hiragino Sans,
    //            Apple SD Gothic Neo, Apple Symbols (not PingFang: since macOS 12 it is the system UI's private font,
    //            which is not installed for the process)
    //   Linux    Noto Sans, Noto Sans CJK SC / TC / JP / KR, DejaVu Sans, Noto Sans Devanagari, Noto Sans Thai,
    //            Noto Sans Symbols, Noto Sans Symbols 2, WenQuanYi Zen Hei, Droid Sans Fallback
    // Simplified Chinese comes before the other CJK fonts: characters the four share (Han unification) take its forms,
    // except after kana or Hangul in the same paragraph. The FreeType text system gives kana and Hangul to the first
    // fallback whose OS/2 code pages name Japanese or Korean (Yu Gothic UI, Malgun Gothic), not to the first that
    // has them (YaHei's kana: full width).
    ESIA_API std::span<const std::string_view> DefaultFallbackFamilies();

    // The installed fonts of DefaultFallbackFamilies(), in its order; missing families are left out, a face found for
    // two names is listed once.
    ESIA_API std::vector<SystemFont> FindDefaultFallbackFonts(int weight = 400, FontStyle style = FontStyle::Upright);

    // Loads `font` into `ts`: TextSystem::AddFontFile with its path and face (0 on failure).
    ESIA_API FontId AddSystemFont(TextSystem& ts, const SystemFont& font);

    // Loads `fonts` into `ts` and makes each one a fallback (TextSystem::AddFallback), in order. Returns their ids
    // (those that failed to load are left out); the first one also serves as a primary font. A typical setup:
    //     const auto chain = text::AddFallbackFonts(*ts, text::FindDefaultFallbackFonts());
    ESIA_API std::vector<FontId> AddFallbackFonts(TextSystem& ts, std::span<const SystemFont> fonts);
}
