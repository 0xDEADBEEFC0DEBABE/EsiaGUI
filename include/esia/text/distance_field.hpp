// Esia - multi-channel signed distance fields of glyph outlines (MTSDF), made by Esia itself.
//
// A glyph drawn from a distance field keeps sharp edges at any size and under any transform from one bitmap: large
// and animated text, and text effects (outlines, glows, shadows, weight) drawn in the same pass. The field is an
// MTSDF: R, G, B a multi-channel distance (Chlumsky's MSDF - the edges are colored so that the median of the three
// channels keeps corners sharp under bilinear filtering), A the true signed distance, for effects farther out.
//
// How it is made:
//   * curves are flattened finely and the shape is resolved to the area it fills (non-zero winding): the pieces of
//     edges that lie inside the shape - overlapping contours, which variable fonts keep - are dropped, so neither the
//     glyph's edge nor an effect shows them, and the places where contours cross become corners;
//   * the boundary loops are cut at their corners into runs and colored: two channels per run, switching at every
//     corner, so that at a corner the two runs share one channel;
//   * every texel takes, per channel, the signed pseudo-distance to its nearest run (the run's end tangents
//     extended), and in A the true distance to the nearest boundary; signs come from the non-zero winding test;
//   * texels whose channels would make bilinear filtering cross the edge where there is none (clashes) are flattened
//     to their median.
#pragma once
#include "esia/text/glyph_raster.hpp"
#include <span>

namespace esia::text
{
    // A glyph outline with its curves, in pixels of the distance field, y down, origin = the pen on the baseline.
    // Contours are closed implicitly (the last point back to the first).
    class ESIA_API GlyphShape
    {
    public:
        struct Edge
        {
            Vec2 p[4];        // p[0] .. p[degree]
            int degree = 1;   // 1 line, 2 quadratic, 3 cubic Bezier
        };

        void MoveTo(Vec2 p);
        void LineTo(Vec2 p);
        void QuadTo(Vec2 control, Vec2 p);
        void CubicTo(Vec2 control1, Vec2 control2, Vec2 p);
        void Clear();

        std::size_t ContourCount() const { return starts_.size(); }
        std::span<const Edge> Contour(std::size_t i) const;
        bool Empty() const { return edges_.empty(); }
        Rect Bounds() const;   // of every end and control point; an empty Rect without edges

    private:
        std::vector<Edge> edges_;
        std::vector<std::size_t> starts_;   // first edge of each contour
        Vec2 pen_;
    };

    struct DistanceFieldDesc
    {
        float range = 8.0f;         // R, G, B: distances from -range / 2 to +range / 2 field pixels span 0 .. 1
        float alphaRange = 24.0f;   // A: the true distance, likewise (wider: glows and shadows reach farther)
    };

    // The MTSDF of `shape` (GlyphBitmap::sdf: channels 4). 0.5 is the edge, inside is above. The bitmap reaches
    // alphaRange / 2 + 1 pixels past the shape on every side; left / top place it like a coverage bitmap. False
    // when it would be larger than 4096 pixels; a shape without edges gives an empty, valid bitmap.
    ESIA_API bool GenerateMtsdf(const GlyphShape& shape, const DistanceFieldDesc& desc, GlyphBitmap& out);
}
