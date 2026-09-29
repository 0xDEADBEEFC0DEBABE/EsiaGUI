// WGT UI - analytic glyph rasterizer.
//
// Glyphs are rasterized from their *unhinted* outlines with exact area coverage (every pixel gets the
// fraction of its area covered by the shape, 8-bit), the way Core Text does it on macOS:
//   * no grid fitting: shapes, weights and spacing stay true to the design at every size,
//   * smooth, even anti-aliasing on curves and diagonals (no quantized coverage levels),
//   * any sub-pixel position (the caller passes the fractional pen offset).
// Perceptual weight (stem darkening for dark text, thinning for light text) is applied at draw time by the
// text shader, which knows the text color.
#pragma once
#include "text/text_engine.hpp"

namespace wgt
{
    // Rasterizes `glyph` of `face` at `emPixels` (physical pixels per em), shifted right by `offsetX` pixels.
    // `out.left/top` are relative to the (integer) baseline origin. Returns false for glyphs without an
    // outline (bitmap-only fonts): callers fall back to DirectWrite's own rasterizer.
    bool RasterizeGlyphOutline(IDWriteFontFace* face, std::uint16_t glyph, float emPixels, float offsetX, GlyphBitmap& out);

    // Sub-pixel (ClearType-style) variant: exact coverage at 3x horizontal resolution, filtered with a
    // 5-tap FIR (FreeType's default LCD filter) to keep color fringes mild. `out.pixels` is RGBA: one
    // coverage per R / G / B stripe (in the display's stripe order, `bgr`), A = grayscale coverage.
    bool RasterizeGlyphOutlineLcd(IDWriteFontFace* face, std::uint16_t glyph, float emPixels, float offsetX, bool bgr, GlyphBitmap& out);
}
