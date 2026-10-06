// Esia - the text interface: what Painter and widgets need from a text stack.
//
// One interface, one implementation per platform family (docs/REWRITE.md, "Text"):
//   * FreeType + HarfBuzz (esia_text_ft, every platform; the only one on Linux / Android / consoles),
//   * DirectWrite on Windows and Core Text on Apple (later: system fallback fonts and the platform's rendering).
// A TextSystem shapes UTF-8 text, rasterizes glyphs into atlas pages it owns in the TextureRegistry and emits
// glyph quads into a DrawList (Geometry commands on those pages). Glyph pages are Alpha8 coverage, which the renderer
// recognizes from the page's TextureInfo and draws with its text pipeline, so text needs no special draw commands.
// Text is antialiased in grayscale on every platform: there is no sub-pixel (LCD / ClearType) text.
//
// Threading: a TextSystem belongs to one UI thread (like the Context that uses it).
#pragma once
#include "esia/base/math.hpp"
#include "esia/base/utf8.hpp"
#include <string_view>
#include <vector>

namespace esia
{
    class DrawList;
}

namespace esia::text
{
    struct GlyphSlot;
    struct GlyphBitmap;

    // Handle of a loaded font face (a file / memory blob + face index, or a system family). 0 = none.
    using FontId = std::uint32_t;

    // A font at a size in UI units (already multiplied by the metrics scale).
    struct FontRef
    {
        FontId id = 0;
        float size = 0.0f;
    };

    enum TextFlags_ : std::uint32_t
    {
        TextFlags_None = 0,
        TextFlags_Ellipsis = 1u << 0,      // trim with "…" when wider than the wrap width (single line)
        TextFlags_AlignCenter = 1u << 1,   // paragraph alignment of wrapped lines
        TextFlags_AlignRight = 1u << 2,
    };

    struct TextMetrics
    {
        Vec2 size;               // box of the laid-out text (UI units)
        float baseline = 0.0f;   // first baseline, from the top of the box
        int lines = 0;
    };

    // A place a caret can stand in a line of text: a grapheme cluster boundary.
    struct CaretStop
    {
        std::uint32_t offset = 0;   // bytes into the UTF-8 text
        float x = 0.0f;             // where the caret is drawn, from the start of the line (UI units)
    };

    // How glyphs are rasterized this frame.
    struct RasterParams
    {
        float pixelsPerUnit = 1.0f;   // render-target pixels per UI unit (metrics x render scale is already in the sizes)
    };

    class TextSystem
    {
    public:
        virtual ~TextSystem() = default;

        // ---- fonts
        // Loads a font file (TTF / OTF / TTC face `faceIndex`); 0 on failure.
        virtual FontId AddFontFile(const char* path, int faceIndex = 0) = 0;
        // The data must stay alive as long as the TextSystem (bundled fonts in the executable).
        virtual FontId AddFontMemory(const void* data, std::size_t size, int faceIndex = 0) = 0;
        // Characters a font lacks are looked up in the fallback chain, in the order the fonts were added here.
        virtual void AddFallback(FontId font) = 0;

        // ---- frame
        // Call once per frame before drawing: sets the pixel density and grows / trims the glyph pages.
        virtual void NewFrame(const RasterParams& params) = 0;

        // ---- layout and drawing (UTF-8)
        virtual TextMetrics Measure(FontRef font, std::string_view text, float wrapWidth = 0.0f, std::uint32_t flags = 0) = 0;
        // Draws `text` with its box's top-left at `pos` and returns the box size at scale 1 (what Measure returns).
        // `scale` draws it scaled around `pos` (press / pop animations) by rasterizing at the scaled size instead
        // of stretching the bitmaps.
        virtual Vec2 Draw(DrawList& dl, FontRef font, Vec2 pos, Color color, std::string_view text, float wrapWidth = 0.0f,
                          std::uint32_t flags = 0, float scale = 1.0f) = 0;
        // One glyph (icon fonts) optically centered on `center`; `size` = em size in UI units.
        virtual void DrawGlyph(DrawList& dl, FontRef font, char32_t codepoint, Vec2 center, Color color) = 0;
        // Coverage tiles other than glyphs, in the texture text is drawn with (the corners of Painter's flat shapes, so
        // they batch with the text) and kept apart from the glyphs: the tile cached under `key`, else null; AddTile packs
        // one the caller rasterized (8-bit coverage). Gone when the glyph pages start over. Null from both: no tiles.
        virtual const GlyphSlot* FindTile(std::uint64_t key)
        {
            (void)key;
            return nullptr;
        }
        virtual const GlyphSlot* AddTile(std::uint64_t key, const GlyphBitmap& bitmap)
        {
            (void)key;
            (void)bitmap;
            return nullptr;
        }
        // Changes when the tiles found so far are gone (the pages started over), and differs between text systems (one
        // made where another was): a caller may keep their slots while it stays.
        virtual std::uint64_t TileGeneration() { return 0; }

        // ---- editing
        // The caret stops of `text` laid out as one line (line breaks are not honored: single-line fields), in logical
        // order: the first at offset 0, the last at text.size(). x is where Draw puts the caret, so it decreases
        // through a right-to-left run; a ligature of several characters is split evenly. This default stops at every
        // code point and measures the text before it; the FreeType system stops at grapheme cluster boundaries.
        virtual void CaretStops(FontRef font, std::string_view text, std::vector<CaretStop>& out)
        {
            out.clear();
            for (std::size_t i = 0;;)
            {
                out.push_back({(std::uint32_t)i, i > 0 ? Measure(font, text.substr(0, i)).size.x : 0.0f});
                if (i >= text.size())
                    break;
                DecodeUtf8(text, i);
            }
        }
    };
}
