// Esia - installed fonts on Windows (include/esia/text/system_fonts.hpp): DirectWrite's system font collection (the
// fonts of every user included since Windows 10 1809), used only to find the file behind a family.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "font_file.hpp"
#include <windows.h>
#include <dwrite.h>
#include <algorithm>

namespace esia::text::detail
{
    namespace
    {
        constexpr std::string_view kChain[] = {
            "Segoe UI", "Microsoft YaHei", "Microsoft JhengHei", "Yu Gothic UI", "Malgun Gothic",
            // the scripts Segoe UI leaves to others: Indic (Nirmala UI), Thai and Lao (Leelawadee UI), Ethiopic and N'Ko
            // (Ebrima), Cherokee and Canadian syllabics (Gadugi), Myanmar, Javanese, historic scripts; then symbols and emoji
            // (their outlines: color glyphs are not drawn yet)
            "Nirmala UI", "Leelawadee UI", "Ebrima", "Gadugi", "Myanmar Text", "Javanese Text", "Segoe UI Historic", "Segoe UI Symbol",
            "Segoe UI Emoji",
        };

        template <typename T>
        class Com
        {
        public:
            Com() = default;
            Com(const Com&) = delete;
            Com& operator=(const Com&) = delete;
            ~Com()
            {
                if (p_)
                    p_->Release();
            }
            T* operator->() const { return p_; }
            T* Get() const { return p_; }
            T** Put() { return &p_; }
            explicit operator bool() const { return p_ != nullptr; }

        private:
            T* p_ = nullptr;
        };

        std::wstring Widen(std::string_view s)
        {
            const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
            std::wstring out((std::size_t)std::max(n, 0), L'\0');
            if (n > 0)
                MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n);
            return out;
        }

        std::string Narrow(const wchar_t* s, std::size_t length)
        {
            const int n = WideCharToMultiByte(CP_UTF8, 0, s, (int)length, nullptr, 0, nullptr, nullptr);
            std::string out((std::size_t)std::max(n, 0), '\0');
            if (n > 0)
                WideCharToMultiByte(CP_UTF8, 0, s, (int)length, out.data(), n, nullptr, nullptr);
            return out;
        }

        // The factory is shared and thread-safe; the collection is taken once (fonts installed later need a restart).
        IDWriteFontCollection* SystemCollection()
        {
            static IDWriteFontCollection* collection = [] {
                IDWriteFontCollection* c = nullptr;
                Com<IDWriteFactory> factory;
                if (SUCCEEDED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(factory.Put()))))
                    factory->GetSystemFontCollection(&c, FALSE);
                return c;   // kept for the process's lifetime
            }();
            return collection;
        }

        std::string EnglishName(IDWriteLocalizedStrings* names)
        {
            UINT32 index = 0;
            BOOL exists = FALSE;
            if (FAILED(names->FindLocaleName(L"en-us", &index, &exists)) || !exists)
                index = 0;
            UINT32 length = 0;
            if (names->GetCount() == 0 || FAILED(names->GetStringLength(index, &length)))
                return {};
            std::wstring s(length + 1, L'\0');
            if (FAILED(names->GetString(index, s.data(), length + 1)))
                return {};
            return Narrow(s.c_str(), length);
        }

        std::optional<SystemFont> Find(std::string_view family, int weight, FontStyle style)
        {
            IDWriteFontCollection* collection = SystemCollection();
            if (!collection)
                return std::nullopt;
            UINT32 familyIndex = 0;
            BOOL exists = FALSE;
            // FindFamilyName matches every localized name of a family
            if (FAILED(collection->FindFamilyName(Widen(family).c_str(), &familyIndex, &exists)) || !exists)
                return std::nullopt;
            Com<IDWriteFontFamily> fontFamily;
            Com<IDWriteFont> font;
            if (FAILED(collection->GetFontFamily(familyIndex, fontFamily.Put())) ||
                FAILED(fontFamily->GetFirstMatchingFont((DWRITE_FONT_WEIGHT)weight, DWRITE_FONT_STRETCH_NORMAL,
                                                        style == FontStyle::Italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
                                                        font.Put())))
                return std::nullopt;
            // OpenType faces live in one file (only Type 1 fonts have two)
            Com<IDWriteFontFace> face;
            UINT32 files = 0;
            Com<IDWriteFontFile> file;
            if (FAILED(font->CreateFontFace(face.Put())) || FAILED(face->GetFiles(&files, nullptr)) || files != 1 ||
                FAILED(face->GetFiles(&files, file.Put())) || !file)
                return std::nullopt;
            // only fonts on disk have a path (not those of a custom or remote loader)
            const void* key = nullptr;
            UINT32 keySize = 0;
            Com<IDWriteFontFileLoader> loader;
            Com<IDWriteLocalFontFileLoader> local;
            if (FAILED(file->GetReferenceKey(&key, &keySize)) || FAILED(file->GetLoader(loader.Put())) ||
                FAILED(loader->QueryInterface(__uuidof(IDWriteLocalFontFileLoader), reinterpret_cast<void**>(local.Put()))))
                return std::nullopt;
            UINT32 length = 0;
            if (FAILED(local->GetFilePathLengthFromKey(key, keySize, &length)))
                return std::nullopt;
            std::wstring path(length + 1, L'\0');
            if (FAILED(local->GetFilePathFromKey(key, keySize, path.data(), length + 1)))
                return std::nullopt;

            SystemFont out;
            out.path = Narrow(path.c_str(), length);
            out.faceIndex = (int)face->GetIndex();
            Com<IDWriteLocalizedStrings> names;
            if (SUCCEEDED(fontFamily->GetFamilyNames(names.Put())))
                out.family = EnglishName(names.Get());
            // DirectWrite's weight and style count what the file does not hold: a simulated bold or oblique (SimSun
            // at 700) and the named instance of a variable font (Segoe UI Variable at 700), while a text system
            // loads the stored face (FreeType: a variable font's default instance). Report the face as stored.
            out.weight = (int)font->GetWeight();
            out.style = font->GetStyle() == DWRITE_FONT_STYLE_NORMAL ? FontStyle::Upright : FontStyle::Italic;
            for (const FontFaceInfo& stored : ReadFontFaces(PathFromUtf8(out.path)))
                if (stored.faceIndex == out.faceIndex)
                {
                    out.weight = stored.weight;
                    out.style = stored.italic ? FontStyle::Italic : FontStyle::Upright;
                }
            return out;
        }
    }

    std::optional<SystemFont> FindPlatformFont(std::string_view family, int weight, FontStyle style)
    {
        return Find(EqualsIgnoreCase(family, kSystemUiFamily) ? kChain[0] : family, weight, style);
    }

    std::span<const std::string_view> PlatformFallbackFamilies() { return kChain; }
}
