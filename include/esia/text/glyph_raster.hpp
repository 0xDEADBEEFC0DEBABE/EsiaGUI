// Esia - WGT's analytic glyph rasterizer without the platform: an outline in pixels in, exact area coverage out.
//
// Glyphs are rasterized from their *unhinted* outlines: every pixel gets the fraction of its area the shape covers
// (8-bit), with no grid fitting - shapes, weights and spacing stay true to the design at every size, curves and
// diagonals get smooth, even anti-aliasing, and the pen may sit at any sub-pixel position. Perceptual weight (stem
// darkening / thinning by text color) is the text shader's job. Coverage is grayscale: Esia has no sub-pixel (LCD)
// text.
//
// Every text system feeds its outlines here (FreeType now; DirectWrite and Core Text the same way later), so a
// glyph looks the same whichever library read the font.
#pragma once
#include "esia/base/math.hpp"
#include <cstdint>
#include <span>
#include <vector>

namespace esia::text
{
    // A glyph outline in physical pixels, y down, origin = the pen position on the baseline. Curves are flattened
    // as they are added (error below 1/12 px). Contours are closed implicitly (last point back to the first).
    class ESIA_API Outline
    {
    public:
        void MoveTo(Vec2 p);
        void LineTo(Vec2 p);
        void QuadTo(Vec2 control, Vec2 p);                   // TrueType curves
        void CubicTo(Vec2 control1, Vec2 control2, Vec2 p);  // CFF / PostScript curves
        void Clear();

        std::size_t ContourCount() const { return starts_.size(); }
        std::span<const Vec2> Contour(std::size_t i) const;
        // Bounds of the flattened points (the ink box); an empty Rect when there are none.
        Rect Bounds() const;

    private:
        std::vector<Vec2> points_;
        std::vector<std::size_t> starts_;   // first point of each contour
    };

    struct GlyphBitmap
    {
        int left = 0, top = 0;       // of the bitmap, in pixels from the integer pen position on the baseline
        int width = 0, height = 0;   // 0 x 0 for a glyph without ink (a space)
        std::vector<std::uint8_t> pixels;   // 8-bit coverage, rows top first, tightly packed
    };

    // Rasterizes `outline` shifted right by `offsetX` pixels (the pen's sub-pixel phase). Every edge of the bitmap
    // is a row / column of zero coverage, so neighbours in an atlas never bleed in. Returns false when the glyph is
    // larger than 4096 pixels; an outline without contours gives an empty, valid bitmap.
    ESIA_API bool RasterizeGray(const Outline& outline, float offsetX, GlyphBitmap& out);
}
