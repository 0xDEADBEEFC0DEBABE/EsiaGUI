// Esia - system fonts FreeType cannot draw, read through the platform instead (the FreeType text system's fallback for
// them). Apple: Core Text, for the `coretext:<PostScript name>` paths of system_fonts_apple.cpp - fonts such as
// PingFang, whose glyphs are in Apple's own `hvgl` format (no `glyf`, no `CFF `) and whose file is the system UI's.
// HarfBuzz shapes with the tables the platform hands out; the outlines are the platform's glyph paths, rasterized by
// Esia's rasterizer like every other glyph. Color glyphs FreeType cannot decode are drawn by the platform too (the
// FreeType face of such a font opens its platform face by PostScript name). Elsewhere there are no such fonts.
#pragma once
#include "esia/text/glyph_raster.hpp"
#include <hb.h>
#include <cstdint>
#include <memory>
#include <string_view>

namespace esia::text::detail
{
    class PlatformFace
    {
    public:
        virtual ~PlatformFace() = default;
        // HarfBuzz's face on the font's tables (the caller owns it).
        virtual hb_face_t* CreateHbFace() = 0;
        // The instance's axis values on `font` (a named instance of a variable font: PingFang SC Semibold).
        virtual void SetVariations(hb_font_t* font) = 0;
        // `glyph`'s outline, design units times `scale` (pixels), y down; false when it has none (a space).
        virtual bool GlyphOutline(std::uint16_t glyph, float scale, Outline& out) = 0;
        // A color glyph (emoji) drawn at `emPixels` per em: straight-alpha RGBA8 (out.channels 4), placed as FreeType's
        // color bitmaps are; false when it has none. For the color bitmaps FreeType cannot decode (iOS' Apple Color
        // Emoji: Apple's emjc compression, not PNG).
        virtual bool ColorGlyph(std::uint16_t glyph, float emPixels, GlyphBitmap& out) = 0;
    };

    // The font `path` names when it is one the platform draws (else null: an ordinary font file).
    std::unique_ptr<PlatformFace> OpenPlatformFace(std::string_view path);
}
