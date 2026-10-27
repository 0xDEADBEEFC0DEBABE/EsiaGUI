// Esia - WGT's analytic glyph rasterizer without the platform: an outline in pixels in, exact area coverage out.
//
// Glyphs are rasterized from their *unhinted* outlines: every pixel gets the fraction of its area the shape covers
// (8-bit), with no grid fitting - shapes, weights and spacing stay true to the design at every size, curves and
// diagonals get smooth, even anti-aliasing, and the pen may sit at any sub-pixel position. Perceptual weight (stem
// darkening / thinning by text color) is the text shader's job. The sub-pixel (LCD) variant covers at three times the
// horizontal resolution and filters across the stripes like FreeType's LCD filters.
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
        std::vector<std::uint8_t> pixels;   // rows top first, tightly packed: 8-bit coverage, or RGBA8 (channels 4)
        int channels = 1;            // 1: coverage; 4: a color glyph, straight-alpha RGBA8 (emoji), or sub-pixel coverage
        bool lcd = false;            // channels 4: R, G, B the coverage of those stripes, A the grayscale coverage
        bool sdf = false;            // channels 4: a multi-channel signed distance field (distance_field.hpp)
    };

    // FreeType's LCD filters: 5 weights out of 256 across the stripes (they sum to 256).
    inline constexpr std::uint16_t kLcdFilterDefault[5] = {8, 77, 86, 77, 8};   // FT_LCD_FILTER_DEFAULT
    inline constexpr std::uint16_t kLcdFilterLight[5] = {0, 85, 86, 85, 0};     // FT_LCD_FILTER_LIGHT
    inline constexpr std::uint16_t kLcdFilterNone[5] = {0, 0, 256, 0, 0};       // each stripe its own coverage

    // Rasterizes `outline` shifted by `offset` pixels (the pen's sub-pixel phase: right, and down for text in
    // motion). Every edge of the bitmap is a row / column of zero coverage, so neighbours in an atlas never bleed in.
    // Returns false when the glyph is larger than 4096 pixels; an outline without contours gives an empty, valid
    // bitmap. The float forms shift right only.
    ESIA_API bool RasterizeGray(const Outline& outline, Vec2 offset, GlyphBitmap& out);
    inline bool RasterizeGray(const Outline& outline, float offsetX, GlyphBitmap& out) { return RasterizeGray(outline, Vec2(offsetX, 0.0f), out); }
    // The same glyph grown by `radius` pixels all around (a text outline's coverage, TextOutline): a pixel is covered
    // as much as the glyph covers it, or by how far its center lies within radius + 1/2 of the glyph's edge, round at
    // the corners. The bitmap is larger by the growth on every side (zero edges as RasterizeGray's).
    ESIA_API bool RasterizeGrown(const Outline& outline, Vec2 offset, float radius, GlyphBitmap& out);
    inline bool RasterizeGrown(const Outline& outline, float offsetX, float radius, GlyphBitmap& out)
    {
        return RasterizeGrown(outline, Vec2(offsetX, 0.0f), radius, out);
    }
    // Sub-pixel coverage (GlyphBitmap::lcd): the outline covered per third of a pixel across, filtered with `filter`
    // over neighbouring stripes, R / G / B the red / green / blue stripes of a panel whose stripes run red, green,
    // blue from the left (`bgr`: blue, green, red), A the pixel's exact grayscale coverage - RasterizeGray's - for
    // where per-channel alpha cannot be kept. One more empty column on each side than RasterizeGray (the filter's
    // reach).
    ESIA_API bool RasterizeLcd(const Outline& outline, Vec2 offset, bool bgr, const std::uint16_t (&filter)[5], GlyphBitmap& out);
    inline bool RasterizeLcd(const Outline& outline, float offsetX, bool bgr, const std::uint16_t (&filter)[5], GlyphBitmap& out)
    {
        return RasterizeLcd(outline, Vec2(offsetX, 0.0f), bgr, filter, out);
    }
}
