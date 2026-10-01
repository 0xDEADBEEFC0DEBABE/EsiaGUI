// Core Text's own measurements for the text tests (Apple only). A file of its own: Core Text's headers bring MacTypes'
// Rect, Point and Style, which clash with esia's in the tests (`using namespace esia`).
#include <CoreText/CoreText.h>
#include <string>
#include <vector>

namespace esia::texttest
{
    // Core Text's own advance of `text` in the font `postScriptName` at `size` (the sum over its characters, one glyph
    // each); -1 when the font lacks one of them.
    float CoreTextAdvance(const char* postScriptName, float size, const std::u16string& text)
    {
        CFStringRef name = CFStringCreateWithCString(nullptr, postScriptName, kCFStringEncodingUTF8);
        CTFontRef font = CTFontCreateWithName(name, (CGFloat)size, nullptr);
        CFRelease(name);
        std::vector<UniChar> chars(text.begin(), text.end());
        std::vector<CGGlyph> glyphs(chars.size());
        float advance = -1.0f;
        if (CTFontGetGlyphsForCharacters(font, chars.data(), glyphs.data(), (CFIndex)chars.size()))
            advance = (float)CTFontGetAdvancesForGlyphs(font, kCTFontOrientationHorizontal, glyphs.data(), nullptr, (CFIndex)glyphs.size());
        CFRelease(font);
        return advance;
    }
}
