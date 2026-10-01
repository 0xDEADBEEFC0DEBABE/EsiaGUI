// Esia - color glyphs of COLR fonts (the FreeType text system's): the vector emoji of Windows (Segoe UI Emoji) and
// Android (Noto Color Emoji), which are COLR version 1 - layers of glyph outlines filled with solid colors and linear,
// radial and sweep gradients, under affine transforms, combined by Porter-Duff and blend modes. Version 0 fonts (flat
// layers of palette colors) go the same way. The outlines are rasterized by Esia's rasterizer (glyph_raster.hpp); the
// paint graph is FreeType's (FT_Get_Color_Glyph_Paint and friends).
#pragma once
#include "esia/text/glyph_raster.hpp"
#include <cstdint>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace esia::text::detail
{
    // Whether `face` has a COLR table with a palette (its color glyphs are drawn by DrawColrGlyph).
    bool HasColrGlyphs(FT_Face face);

    // Whether `glyph` is one of `face`'s color glyphs (a COLR version 1 paint or version 0 layers).
    bool IsColrGlyph(FT_Face face, std::uint16_t glyph);

    // `glyph` drawn at `emPixels` per em, shifted right by `offsetX` pixels (the pen's sub-pixel phase): straight-alpha
    // RGBA8 (out.channels 4), placed like a gray glyph's bitmap (pixels from the integer pen position, y down). Palette
    // 0; the text color (palette index 0xFFFF) is black. `ink` gets the bounds of the glyph's outlines. False when the
    // glyph is not a color glyph or is larger than 4096 pixels.
    bool DrawColrGlyph(FT_Face face, std::uint16_t glyph, float emPixels, float offsetX, GlyphBitmap& out, Rect& ink);
}
