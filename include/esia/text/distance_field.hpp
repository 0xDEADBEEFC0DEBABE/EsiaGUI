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
//
// A text that appears with many glyphs never drawn before needs their fields at once: FieldMaker makes them on worker
// threads together with the thread that waits for them, and keeps them on disk between runs.
#pragma once
#include "esia/text/glyph_raster.hpp"
#include <memory>
#include <span>
#include <string>

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

    // The version of what GenerateMtsdf makes: raised whenever it makes other bits for the same shape, so that the
    // fields kept on disk before (FieldMakerDesc::cacheDirectory) are made again.
    inline constexpr std::uint32_t kDistanceFieldVersion = 1;

    struct FieldMakerDesc
    {
        // threads besides the one that waits: -1 = one per processor but one (at most 15), 0 = none (the waiting
        // thread makes every field)
        int workers = -1;
        // A directory where made fields are kept between runs (UTF-8; made when missing). Empty: none.
        std::string cacheDirectory;
    };

    // The fields of many shapes at once - the glyphs of a text that appears with glyphs never drawn before. Every field
    // is GenerateMtsdf's, the same bits whichever thread makes it: the workers start on a shape as soon as it is added,
    // and Make has the waiting thread make the rest with them. With a cache directory a field is first looked for
    // there (on the thread that would make it), found by what makes its bits - every point of the shape as the
    // generator gets it, the ranges, kDistanceFieldVersion, the kind of processor and compiler - so another font file,
    // face, instance or range never finds a field that is not its own; fields made are written there afterwards by one
    // worker at a time, behind any batch, each in a file of its own, written beside and renamed over (processes may
    // share the directory; nothing is ever removed from it).
    //
    // Threading: a FieldMaker belongs to one thread, which adds the shapes and waits for their fields; the workers see
    // only the shapes handed over and the cache's files.
    class ESIA_API FieldMaker
    {
    public:
        explicit FieldMaker(const FieldMakerDesc& desc = {});
        // Writes the fields still waiting for the disk, then stops the workers.
        ~FieldMaker();
        FieldMaker(const FieldMaker&) = delete;
        FieldMaker& operator=(const FieldMaker&) = delete;

        // Hands over the field of `shape` with `desc` and returns its index in the batch (0, 1, ... since the last
        // Make). The workers start when the first shape comes.
        std::size_t Add(GlyphShape&& shape, const DistanceFieldDesc& desc);
        // Makes the fields of the batch no worker has taken yet, on this thread, and returns when every one is done.
        void Make();
        // After Make, the batch's i-th field: GenerateMtsdf's bitmap, or null where it returned false. Valid until the
        // next Add.
        const GlyphBitmap* Field(std::size_t i) const;
        // ... read from the cache directory rather than made
        bool FromDisk(std::size_t i) const;
        int Workers() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
