// Esia - the text interface: what Painter and widgets need from a text stack.
//
// One interface, one implementation per platform family (docs/REWRITE.md, "Text"):
//   * FreeType + HarfBuzz (esia_text_ft, every platform; the only one on Linux / Android / consoles),
//   * DirectWrite on Windows and Core Text on Apple (later: system fallback fonts and the platform's rendering).
// A TextSystem shapes UTF-8 text, rasterizes glyphs into atlas pages it owns in the TextureRegistry and emits
// glyph quads into a DrawList (Geometry commands on those pages). Glyph pages are Alpha8 coverage, or RGBA8 with
// TextureFlags_LcdCoverage for sub-pixel text; the renderer picks the matching text pipeline from the page's
// TextureInfo, so text needs no special draw commands.
//
// Threading: a TextSystem belongs to one UI thread (like the Context that uses it).
#pragma once
#include "esia/base/math.hpp"
#include <string_view>

namespace esia
{
    class DrawList;
}

namespace esia::text
{
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

    enum class Antialiasing : std::uint8_t
    {
        Grayscale,   // Alpha8 coverage pages
        Subpixel,    // RGBA8 pages, one coverage per R/G/B stripe (TextureFlags_LcdCoverage), dual-source blended
    };

    // How glyphs are rasterized this frame.
    struct RasterParams
    {
        float pixelsPerUnit = 1.0f;   // render-target pixels per UI unit (metrics x render scale is already in the sizes)
        Antialiasing antialiasing = Antialiasing::Grayscale;
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
    };
}
