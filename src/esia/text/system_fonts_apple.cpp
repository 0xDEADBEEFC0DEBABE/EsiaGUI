// Esia - installed fonts on macOS / iOS (include/esia/text/system_fonts.hpp): Core Text font descriptors, resolved to
// their file URLs. Core Text does not say which face of a collection a descriptor is (PingFang.ttc holds PingFang SC,
// TC, HK ...): the face whose PostScript name matches the descriptor's is looked up in the file.
#include "font_file.hpp"
#include "esia/text/system_fonts.hpp"
#include <CoreText/CoreText.h>
#include <TargetConditionals.h>
#include <algorithm>
#include <climits>

namespace esia::text::detail
{
    namespace
    {
        // No PingFang: since macOS 12 it is the system UI's private font (below), which FreeType cannot load. Chinese
        // comes from Hiragino Sans GB (Simplified) and Heiti SC / TC (STHeiti), ordinary files in /System/Library/Fonts
        // on every macOS version. The scripts SF does not cover come from the public fonts macOS installs for them
        // (SF Arabic, SF Hebrew ... are the UI's private faces, like PingFang): what Segoe UI, Nirmala UI and
        // Leelawadee UI are in the Windows chain. Then Apple Color Emoji (color bitmap strikes, 200 MB: the text
        // system maps its fonts, it does not read them), before Apple Symbols so that emoji keep their color.
        // iOS has neither Hiragino Sans GB nor Heiti: its Chinese is PingFang's alone (the other CJK fonts lack a third
        // of GB 2312's characters), drawn through Core Text (kCoreTextFontScheme).
        constexpr std::string_view kChain[] = {
#if TARGET_OS_IPHONE
            kSystemUiFamily, "Helvetica Neue", "PingFang SC", "PingFang TC", "PingFang HK", "Hiragino Sans", "Apple SD Gothic Neo",
#else
            kSystemUiFamily, "Helvetica Neue", "Hiragino Sans GB", "Heiti SC", "Heiti TC", "Hiragino Sans", "Apple SD Gothic Neo",
#endif
            "Geeza Pro",              // Arabic
            "Arial Hebrew",           // Hebrew
            "Kohinoor Devanagari",    // Devanagari (Hindi, Marathi, Nepali)
            "Kohinoor Bangla",        // Bengali
            "Kohinoor Gujarati",      // Gujarati
            "Kohinoor Telugu",        // Telugu
            "Mukta Mahee",            // Gurmukhi
            "Tamil Sangam MN",        // Tamil
            "Noto Sans Kannada",      // Kannada
            "Malayalam Sangam MN",    // Malayalam
            "Noto Sans Oriya",        // Odia
            "Sinhala Sangam MN",      // Sinhala
            "Thonburi",               // Thai
            "Lao Sangam MN",          // Lao
            "Khmer Sangam MN",        // Khmer
            "Noto Sans Myanmar",      // Myanmar
            "Noto Sans Armenian",     // Armenian
            "Apple Color Emoji",      // emoji
            "Apple Symbols",
        };

        template <typename T>
        class Cf
        {
        public:
            explicit Cf(T ref = nullptr) : ref_(ref) {}
            Cf(const Cf&) = delete;
            Cf& operator=(const Cf&) = delete;
            ~Cf()
            {
                if (ref_)
                    CFRelease(ref_);
            }
            T Get() const { return ref_; }
            explicit operator bool() const { return ref_ != nullptr; }

        private:
            T ref_;
        };

        std::string ToUtf8(CFStringRef s)
        {
            if (!s)
                return {};
            const CFIndex max = CFStringGetMaximumSizeForEncoding(CFStringGetLength(s), kCFStringEncodingUTF8) + 1;
            std::string out((std::size_t)max, '\0');
            if (!CFStringGetCString(s, out.data(), max, kCFStringEncodingUTF8))
                return {};
            out.resize(std::char_traits<char>::length(out.c_str()));
            return out;
        }

        // Core Text's weight trait (-1 ... 1) as an OpenType weight: the values of NSFontWeight* for 100 ... 900
        int OpenTypeWeight(double ct)
        {
            static constexpr double kCt[] = {-0.8, -0.6, -0.4, 0.0, 0.23, 0.3, 0.4, 0.56, 0.62};
            if (ct <= kCt[0])
                return 100;
            for (int i = 1; i < 9; ++i)
                if (ct <= kCt[i])
                    return 100 * i + (int)(100.0 * (ct - kCt[i - 1]) / (kCt[i] - kCt[i - 1]) + 0.5);
            return 900;
        }

        FaceTraits Traits(CTFontDescriptorRef d)
        {
            FaceTraits t;
            Cf<CFDictionaryRef> traits(static_cast<CFDictionaryRef>(CTFontDescriptorCopyAttribute(d, kCTFontTraitsAttribute)));
            if (!traits)
                return t;
            double weight = 0.0;
            if (const auto w = static_cast<CFNumberRef>(CFDictionaryGetValue(traits.Get(), kCTFontWeightTrait)); w && CFNumberGetValue(w, kCFNumberDoubleType, &weight))
                t.weight = OpenTypeWeight(weight);
            std::int64_t symbolic = 0;
            if (const auto s = static_cast<CFNumberRef>(CFDictionaryGetValue(traits.Get(), kCTFontSymbolicTrait)); s && CFNumberGetValue(s, kCFNumberSInt64Type, &symbolic))
                t.italic = (symbolic & kCTFontItalicTrait) != 0;
            return t;
        }

        // The file and face of a descriptor.
        std::optional<SystemFont> Resolve(CTFontDescriptorRef d)
        {
            Cf<CFURLRef> url(static_cast<CFURLRef>(CTFontDescriptorCopyAttribute(d, kCTFontURLAttribute)));
            char path[PATH_MAX];
            if (!url || !CFURLGetFileSystemRepresentation(url.Get(), true, reinterpret_cast<UInt8*>(path), sizeof(path)))
                return std::nullopt;
            Cf<CFStringRef> psName(static_cast<CFStringRef>(CTFontDescriptorCopyAttribute(d, kCTFontNameAttribute)));
            Cf<CFStringRef> family(static_cast<CFStringRef>(CTFontDescriptorCopyAttribute(d, kCTFontFamilyNameAttribute)));
            const std::string ps = ToUtf8(psName.Get());
            const FaceTraits traits = Traits(d);

            SystemFont out;
            out.path = path;
            out.family = ToUtf8(family.Get());
            out.weight = traits.weight;
            out.style = traits.italic ? FontStyle::Italic : FontStyle::Upright;
            // Core Text also lists the system UI's private fonts (PingFang in PrivateFrameworks/FontServices.framework/
            // Resources/Reserved since macOS 12, CorePrivate on iOS): their glyphs are in Apple's hvgl format, which
            // FreeType cannot load (CI, macOS 15), and the process need not be able to read the file. Core Text draws
            // them: a path for it, by PostScript name (each named instance is a face of its own there).
            if (out.path.find("/PrivateFrameworks/") != std::string::npos)
            {
                if (ps.empty())
                    return std::nullopt;
                out.path = std::string(kCoreTextFontScheme) + ps;
                out.faceIndex = 0;
                return out;
            }
            const std::vector<FontFaceInfo> faces = ReadFontFaces(PathFromUtf8(out.path));
            if (faces.empty())
                return std::nullopt;
            // the named instances of a variable font (SF) share face 0, whose name table knows only the default one
            const auto face = std::find_if(faces.begin(), faces.end(), [&](const FontFaceInfo& f) { return f.postscriptName == ps; });
            out.faceIndex = face != faces.end() ? face->faceIndex : 0;
            return out;
        }

        std::optional<SystemFont> FindSystemUi(int weight, FontStyle style)
        {
            Cf<CTFontRef> ui(CTFontCreateUIFontForLanguage(weight >= 600 ? kCTFontUIFontEmphasizedSystem : kCTFontUIFontSystem, 0.0, nullptr));
            if (!ui)
                return std::nullopt;
            if (style == FontStyle::Italic)
            {
                Cf<CTFontRef> italic(CTFontCreateCopyWithSymbolicTraits(ui.Get(), 0.0, nullptr, kCTFontItalicTrait, kCTFontItalicTrait));
                if (italic)
                {
                    Cf<CTFontDescriptorRef> d(CTFontCopyFontDescriptor(italic.Get()));
                    return d ? Resolve(d.Get()) : std::nullopt;
                }
            }
            Cf<CTFontDescriptorRef> d(CTFontCopyFontDescriptor(ui.Get()));
            return d ? Resolve(d.Get()) : std::nullopt;
        }
    }

    std::optional<SystemFont> FindPlatformFont(std::string_view family, int weight, FontStyle style)
    {
        if (EqualsIgnoreCase(family, kSystemUiFamily))
            return FindSystemUi(weight, style);

        Cf<CFStringRef> name(CFStringCreateWithBytes(nullptr, reinterpret_cast<const UInt8*>(family.data()), (CFIndex)family.size(),
                                                     kCFStringEncodingUTF8, false));
        if (!name)
            return std::nullopt;
        const void* keys[] = {kCTFontFamilyNameAttribute};
        const void* values[] = {name.Get()};
        Cf<CFDictionaryRef> attributes(CFDictionaryCreate(nullptr, keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
        Cf<CTFontDescriptorRef> request(CTFontDescriptorCreateWithAttributes(attributes.Get()));
        // the family is mandatory: without it Core Text answers with the closest other font
        Cf<CFSetRef> mandatory(CFSetCreate(nullptr, keys, 1, &kCFTypeSetCallBacks));
        Cf<CFArrayRef> matches(CTFontDescriptorCreateMatchingFontDescriptors(request.Get(), mandatory.Get()));
        if (!matches)
            return std::nullopt;

        // the best face among those that resolve to a readable file
        std::vector<SystemFont> found;
        std::vector<FaceTraits> traits;
        const CFIndex n = CFArrayGetCount(matches.Get());
        for (CFIndex i = 0; i < n; ++i)
        {
            const auto d = static_cast<CTFontDescriptorRef>(CFArrayGetValueAtIndex(matches.Get(), i));
            if (std::optional<SystemFont> f = Resolve(d))
            {
                found.push_back(std::move(*f));
                traits.push_back(Traits(d));
            }
        }
        const int best = SelectFace(traits, weight, style);
        if (best < 0)
            return std::nullopt;
        return found[(std::size_t)best];
    }

    std::span<const std::string_view> PlatformFallbackFamilies() { return kChain; }
}
