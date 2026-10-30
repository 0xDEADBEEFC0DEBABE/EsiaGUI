// Esia - SVG internals shared by the parser, the geometry and the rasterizer (include/esia/svg/svg.hpp).
#pragma once
#include "esia/svg/svg.hpp"
#include <cstdint>
#include <string_view>
#include <vector>

namespace esia::svg::detail
{
    // ------------------------------------------------------------------ transforms
    // (x, y) -> (a x + c y + e, b x + d y + f): SVG's matrix(a b c d e f)
    struct Affine
    {
        float a = 1.0f, b = 0.0f, c = 0.0f, d = 1.0f, e = 0.0f, f = 0.0f;

        Vec2 Apply(Vec2 p) const { return {a * p.x + c * p.y + e, b * p.x + d * p.y + f}; }
        Vec2 ApplyVector(Vec2 v) const { return {a * v.x + c * v.y, b * v.x + d * v.y}; }
        // `o` first, then this
        Affine operator*(const Affine& o) const
        {
            return {a * o.a + c * o.b, b * o.a + d * o.b, a * o.c + c * o.d, b * o.c + d * o.d, a * o.e + c * o.f + e, b * o.e + d * o.f + f};
        }
        float Det() const { return a * d - b * c; }
        bool Invert(Affine& out) const;
        // the largest singular value: how far a unit can stretch
        float MaxScale() const;
        static Affine Translate(float x, float y) { return {1.0f, 0.0f, 0.0f, 1.0f, x, y}; }
        static Affine Scale(float sx, float sy) { return {sx, 0.0f, 0.0f, sy, 0.0f, 0.0f}; }
        static Affine Rotate(float rad);
    };

    // A number in SVG's grammar at s[i] (sign, digits, fraction, exponent), i moved past it. false: none there.
    bool ParseNumber(std::string_view s, std::size_t& i, float& out);

    // ------------------------------------------------------------------ paths
    enum class Verb : std::uint8_t { Move, Line, Quad, Cubic, Arc, Close };
    // Floats per verb: Move / Line x y; Quad cx cy x y; Cubic c1x c1y c2x c2y x y; Arc cx cy rx ry phi start sweep x y (an
    // elliptical arc about its center, from the current point - its start angle - to x y); Close none.
    inline constexpr int kVerbFloats[6] = {2, 2, 4, 6, 9, 0};

    struct PathData
    {
        std::vector<Verb> verbs;
        std::vector<float> pts;
        Rect bounds;   // the geometry's exact bounds, curves' extremes included (ComputeBounds)
    };

    // SVG path data appended to `out`. false: an error ended it - what came before it stays, as browsers draw it.
    bool ParsePathData(std::string_view d, PathData& out);
    // The basic shapes as SVG defines their outlines (starting points and directions: where dashes begin).
    void AddRect(PathData& out, float x, float y, float w, float h, float rx, float ry);
    void AddEllipse(PathData& out, float cx, float cy, float rx, float ry);
    void AddPoints(PathData& out, const std::vector<float>& xy, bool close);
    void ComputeBounds(PathData& path);

    // ------------------------------------------------------------------ paint
    enum class PaintKind : std::uint8_t { None, Solid, Current, Gradient };
    struct Paint
    {
        PaintKind kind = PaintKind::None;
        Color color;                  // Solid: straight alpha
        std::int32_t gradient = -1;   // Gradient
    };

    enum class Spread : std::uint8_t { Pad, Reflect, Repeat };
    // current: currentColor's color (color.a is the stop's opacity)
    struct Stop
    {
        float offset = 0.0f;
        Color color;
        bool current = false;
    };
    struct Gradient
    {
        bool radial = false;
        bool userSpace = false;   // gradientUnits="userSpaceOnUse"; else fractions of the shape's bounding box
        Spread spread = Spread::Pad;
        Affine transform;         // gradientTransform
        float x1 = 0.0f, y1 = 0.0f, x2 = 1.0f, y2 = 0.0f;
        float cx = 0.5f, cy = 0.5f, r = 0.5f, fx = 0.5f, fy = 0.5f, fr = 0.0f;
        std::vector<Stop> stops;   // offsets in order, 0 .. 1
    };

    // ------------------------------------------------------------------ strokes
    enum class Cap : std::uint8_t { Butt, Round, Square };
    enum class Join : std::uint8_t { Miter, Round, Bevel };
    struct StrokeStyle
    {
        float width = 1.0f;
        Cap cap = Cap::Butt;
        Join join = Join::Miter;
        float miterLimit = 4.0f;
        std::vector<float> dashes;   // an even count with a positive sum; empty: solid
        float dashOffset = 0.0f;
    };

    // ------------------------------------------------------------------ the drawing
    struct Shape
    {
        std::uint32_t path = 0;
        Affine transform;   // user units -> the root's
        Paint fill, stroke;
        float fillOpacity = 1.0f, strokeOpacity = 1.0f;   // with the opacity folded into them
        std::int32_t strokeStyle = -1;
        std::int32_t clip = -1;     // a clip path on this shape's coverage (one paint: no layer needed)
        bool evenOdd = false;
        bool strokeFirst = false;   // paint-order: the stroke under the fill
        bool crisp = false;         // shape-rendering: crispEdges
    };

    struct ClipItem
    {
        std::uint32_t path = 0;
        Affine transform;
        bool evenOdd = false;
    };
    // The union of its items (none: everything clipped away), within its own clip path's.
    struct Clip
    {
        std::vector<ClipItem> items;
        std::int32_t clip = -1;
    };

    struct Layer
    {
        float opacity = 1.0f;
        std::int32_t clip = -1;
    };

    enum class OpKind : std::uint8_t { Shape, Begin, End };
    struct Op
    {
        OpKind kind = OpKind::Shape;
        std::uint32_t index = 0;   // Shape: into shapes; Begin / End: into layers
        std::uint32_t match = 0;   // Begin: its End's position in ops
    };

    struct Tree
    {
        float width = 0.0f, height = 0.0f;   // intrinsic size, pixels
        Rect viewBox;                        // Empty(): none
        bool alignNone = false, slice = false;
        std::uint8_t alignX = 1, alignY = 1;   // preserveAspectRatio: 0 min, 1 mid, 2 max
        std::vector<PathData> paths;
        std::vector<Shape> shapes;
        std::vector<Gradient> gradients;
        std::vector<StrokeStyle> strokes;
        std::vector<Clip> clips;
        std::vector<Layer> layers;
        std::vector<Op> ops;
        bool usesCurrent = false;   // currentColor paints something
        bool onlyCurrent = false;   // every paint is currentColor
        std::uint64_t id = 0;
    };

    // The root's units into width x height pixels (viewBox and preserveAspectRatio).
    Affine ViewTransform(const Tree& tree, float width, float height);

    // ------------------------------------------------------------------ geometry
    // Closed contours in pixels, each a member of a group: an operand of a union, with a fill rule of its own.
    struct Contours
    {
        std::vector<Vec2> pts;
        std::vector<std::uint32_t> starts;   // each contour's first point
        std::vector<std::uint16_t> groups;
        void Begin(std::uint16_t group)
        {
            starts.push_back((std::uint32_t)pts.size());
            groups.push_back(group);
        }
        void Clear()
        {
            pts.clear();
            starts.clear();
            groups.clear();
        }
    };

    // How far a flattened curve may stray from the curve, in pixels.
    inline constexpr float kTolerance = 1.0f / 32.0f;

    // `path`'s subpaths through `m`, flattened within `tol`: a closed contour each (a fill closes them).
    void FlattenFill(const PathData& path, const Affine& m, float tol, std::uint16_t group, Contours& out);
    // The stroke of `path` (`style` in its user units) through `m`: pieces whose union is the stroke - bands along the
    // segments, the joins and caps where they need their own - all wound the same way, within `tol` pixels.
    void StrokeOutline(const PathData& path, const StrokeStyle& style, const Affine& m, float tol, std::uint16_t group, Contours& out);

    // ------------------------------------------------------------------ coverage
    // The cells of a row of pixels: an edge piece leaves in them the area right of it, and the running sum across the
    // row is each pixel's coverage. A bit per cell marks the cells that hold something, so a row costs its edges and its
    // spans, not its width.
    class Cells
    {
    public:
        void Reset(int width);
        int Width() const { return width_; }
        // An edge piece from (xa, ya) to (xb, yb), y from the row's top (0 <= ya < yb <= 1): +1 where coverage starts
        // right of it, -1 where it ends.
        void Add(float xa, float ya, float xb, float yb, float sign);
        // value >= 0: a run of constant coverage; < 0: pixels with coverage of their own, in cov[x0 .. x1)
        struct Span
        {
            int x0, x1;
            float value;
        };
        // The row's coverage, the cells cleared for the next row. crisp: coverage rounded to 0 or 1.
        void Resolve(std::vector<Span>& spans, float* cov, bool crisp);

    private:
        void Piece(float xa, float xb, float d);
        void Mark(int x0, int x1);
        int width_ = 0;
        std::vector<float> acc_;
        std::vector<std::uint64_t> bits_;
        int lo_ = 0, hi_ = -1;   // the words of bits_ that may be set
    };

    // An outline swept row by row: its monotone chains, their order across the row, and where the filled area's
    // boundary runs - for every rule, where outlines overlap or cross - left in the row's cells.
    class Sweep
    {
    public:
        // `evenOdd[g]`: group g's rule. The outline is clipped to width x height pixels.
        void Build(const Contours& contours, const std::uint8_t* evenOdd, int groups, int width, int height);
        bool Empty() const { return top_ >= bottom_; }
        int Top() const { return top_; }         // the first row it covers
        int Bottom() const { return bottom_; }   // past the last
        int Left() const { return left_; }       // the first column it covers
        int Right() const { return right_; }     // past the last
        // Row y's coverage into `cells` (rows in increasing order from Top()).
        void Row(int y, Cells& cells);
        // Row y passed over: the chains across it in the order Row leaves them, without its coverage. Where chains tie
        // across a row, their order is the one the rows before left; an image of part of a drawing passes over the rows
        // above it as the whole image draws them, so each of its rows is the whole image's.
        void Pass(int y);

    private:
        struct Chain
        {
            std::uint32_t first, last;   // its points, top to bottom
            float dir;                   // +1 where the contour runs down, -1 up
            std::uint16_t group;
        };
        struct Active
        {
            std::uint32_t chain;
            std::uint32_t seg;    // the segment the row starts in
            float xmin, xmax;     // the chain's extent across the row
        };
        struct Piece
        {
            std::uint32_t chain;
            float xa, xb;   // at the strip's top and bottom
        };

        float XAt(const Chain& c, std::uint32_t& seg, float y) const;
        void SimpleRow(float y0, Cells& cells);
        void ComplexRow(float y0, Cells& cells);
        void Walk(std::vector<Piece>& pieces, float ya, float yb, float y0, Cells& cells);
        void Thin(std::vector<Piece>& pieces, float ya, float yb, float y0, Cells& cells);
        void Deposit(const Active& a, float y0, Cells& cells, float sign) const;

        std::vector<Vec2> pts_;
        std::vector<Chain> chains_;   // by their top
        std::vector<std::uint8_t> evenOdd_;
        std::vector<int> winding_;
        std::uint32_t next_ = 0;
        std::vector<Active> active_;
        std::vector<float> events_;
        std::vector<Piece> pieces_, thin_;
        std::vector<std::uint32_t> segs_;
        int budget_ = 0;   // crossings this row may still cut at
        int top_ = 0, bottom_ = 0, left_ = 0, right_ = 0;
        int width_ = 0;
    };

    // `frame`: the drawing made in the whole pixels around the viewport (its frame, up to 16384 x 16384 of them; the
    // viewport may reach beyond the image) and the image holds the ones it covers - a viewport moved by whole pixels, its
    // corners the same fractions after the move, gives the same pixels moved by as many. Else in the image's pixels.
    bool Render(const Tree& tree, int width, int height, const RasterOptions& options, Image& out, bool frame = false);

    // The image of a part of a drawing (Atlas::GetPart): Rasterize with the drawing's viewport moved by the part's corner,
    // made in its frame (Render's `frame`) - the pixels the image of all of it has there, bit for bit, where that image's
    // viewport is its frame (svg::Draw's: the whole pixels around where the drawing goes). With a border, the texels
    // around the part are the drawing's there.
    bool RasterizePart(const Document& doc, int width, int height, const RasterOptions& options, Image& out);
}
