// Esia - UTF-8 decoding for text layout, input and editing. A malformed sequence decodes as U+FFFD and consumes
// only its lead byte (or the whole sequence when it is complete but overlong / a surrogate / out of range), so any
// byte string lays out and a decoding loop always advances.
#pragma once
#include <cstddef>
#include <string_view>

namespace esia
{
    constexpr char32_t kReplacementCharacter = 0xFFFD;

    // Decodes the code point that starts at s[i] (i < s.size()) and moves i past it.
    constexpr char32_t DecodeUtf8(std::string_view s, std::size_t& i)
    {
        const unsigned char lead = (unsigned char)s[i];
        if (lead < 0x80)
        {
            ++i;
            return lead;
        }
        std::size_t extra = 0;
        char32_t cp = 0, least = 0;
        if ((lead & 0xE0) == 0xC0)
        {
            extra = 1;
            cp = lead & 0x1Fu;
            least = 0x80;
        }
        else if ((lead & 0xF0) == 0xE0)
        {
            extra = 2;
            cp = lead & 0x0Fu;
            least = 0x800;
        }
        else if ((lead & 0xF8) == 0xF0)
        {
            extra = 3;
            cp = lead & 0x07u;
            least = 0x10000;
        }
        else
        {
            ++i;   // a continuation byte without a lead, or an invalid lead byte
            return kReplacementCharacter;
        }
        if (s.size() - i <= extra)
        {
            ++i;   // truncated at the end of the string
            return kReplacementCharacter;
        }
        for (std::size_t k = 1; k <= extra; ++k)
        {
            const unsigned char c = (unsigned char)s[i + k];
            if ((c & 0xC0) != 0x80)
            {
                ++i;   // the sequence breaks off: resynchronize on the next byte
                return kReplacementCharacter;
            }
            cp = (cp << 6) | (c & 0x3Fu);
        }
        i += extra + 1;
        if (cp < least || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
            return kReplacementCharacter;
        return cp;
    }
}
