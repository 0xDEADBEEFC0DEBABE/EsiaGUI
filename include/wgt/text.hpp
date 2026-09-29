// WGT UI - text
//
// WGT has its own text stack (DirectWrite based) instead of Dear ImGui's stb_truetype fonts:
//   * fonts are picked by family name from the system (and from registered files) - no TTF blobs,
//   * automatic system fallback per character (CJK, symbols, any installed script),
//   * OpenType shaping: kerning, ligatures, bidi, complex scripts, CJK line breaking,
//   * optical sizes (Segoe UI Variable Small / Text / Display, like SF Pro Text / Display),
//   * unhinted outlines rasterized with exact area coverage at the physical pixel density (DPI x render
//     scale), quarter-pixel horizontal positioning, baselines snapped to physical pixels,
//   * grayscale or ClearType-style sub-pixel coverage (TextAntialiasing), composed with the system's
//     DirectWrite gamma / contrast in a dedicated text shader.
// Dear ImGui's own widgets (InputText, stock widgets) get DirectWrite glyphs too, through a font loader shim.
#pragma once
#include "math.hpp"

namespace wgt
{
    enum class FontWeight : std::uint8_t
    {
        Regular,
        Semibold,
        Bold,
        Mono,
        Count
    };

    // Handle of a registered font (family + weight + slant). 0 = invalid.
    using FontId = std::uint32_t;

    // A font at a size. `size` is in UI units (already multiplied by the metrics scale).
    struct FontRef
    {
        FontId id = 0;
        float size = 0.0f;
    };

    enum TextFlags_ : std::uint32_t
    {
        TextFlags_None = 0,
        TextFlags_Ellipsis = 1u << 0,     // trim with "…" when wider than the wrap/max width (single line)
        TextFlags_AlignCenter = 1u << 1,  // multi-line paragraph alignment
        TextFlags_AlignRight = 1u << 2,
    };

    struct TextMetrics
    {
        Vec2 size;               // bounding box of the laid-out text (UI units)
        float baseline = 0.0f;   // first baseline, from the top of the box
        int lines = 0;
    };

    // Registers a font by family name ("Segoe UI Variable Text", "Inter", "Microsoft YaHei UI"...).
    // weight: 100..950 (400 regular, 600 semibold, 700 bold). UI thread of the current context.
    // Families from Context::AddFontFile() / ContextDesc::fontFiles are available too; a family whose file was
    // just queued with AddFontFile renders with the UI family until the file is applied at the next frame.
    // Unknown families fall back to the UI family. Characters the font lacks fall back per character.
    WGT_API FontId RegisterFont(const char* family, int weight = 400, bool italic = false);
    // Built-in fonts of the current context (theme typography uses these).
    WGT_API FontId GetFontId(FontWeight weight);

    WGT_API TextMetrics MeasureText(FontRef font, const char* text, const char* textEnd = nullptr, float wrapWidth = 0.0f, std::uint32_t flags = 0);
}
