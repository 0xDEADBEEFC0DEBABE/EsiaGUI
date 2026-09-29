// Esia - installed fonts on macOS / iOS (include/esia/text/system_fonts.hpp): Core Text font descriptors, resolved to
// their file URLs. Core Text does not say which face of a collection a descriptor is (PingFang.ttc holds PingFang SC,
// TC, HK ...): the face whose PostScript name matches the descriptor's is looked up in the file.
#include "font_file.hpp"
#include <CoreText/CoreText.h>
#include <algorithm>
#include <climits>

namespace esia::text::detail
{
    namespace
    {
        constexpr std::string_view kChain[] = {
            kSystemUiFamily, "Helvetica Neue", "PingFang SC", "PingFang TC", "Hiragino Sans", "Apple SD Gothic Neo", "Apple Symbols",
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
            // the named instances of a variable font (SF) share face 0, whose name table knows only the default one
            const std::vector<FontFaceInfo> faces = ReadFontFaces(PathFromUtf8(out.path));
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

        std::vector<FaceTraits> traits;
        const CFIndex n = CFArrayGetCount(matches.Get());
        for (CFIndex i = 0; i < n; ++i)
            traits.push_back(Traits(static_cast<CTFontDescriptorRef>(CFArrayGetValueAtIndex(matches.Get(), i))));
        const int best = SelectFace(traits, weight, style);
        if (best < 0)
            return std::nullopt;
        return Resolve(static_cast<CTFontDescriptorRef>(CFArrayGetValueAtIndex(matches.Get(), best)));
    }

    std::span<const std::string_view> PlatformFallbackFamilies() { return kChain; }
}
