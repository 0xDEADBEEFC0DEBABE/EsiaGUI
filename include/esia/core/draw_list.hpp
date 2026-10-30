// Esia - draw lists: what the UI core produces and the renderer consumes.
//
// A DrawList holds ordinary indexed geometry (lines, charts, images - `Vertex` triangles), quads drawn instanced
// (glyphs, the Painter's flat tiles and plain shapes - `QuadInstance`) and the FX command stream (SDF shapes as
// fx::Instance, glow-layer and edge-fade brackets, host callbacks) in one ordered command list, each command with its
// clip rect. Consecutive geometry with the same clip and texture merges into one command, so do consecutive quads
// with the same clip and texture (a plain shape reads no texture: it joins quads on any), and consecutive FX shapes
// with the same clip, texture and effect.
//
// Indices are 32-bit and local to the list (0 = the list's first vertex); the renderer rebases them when it
// concatenates lists, so backends never need a base-vertex draw.
#pragma once
#include "esia/base/math.hpp"
#include "esia/core/fx.hpp"
#include <memory>
#include <utility>
#include <vector>

namespace esia
{
    struct Vertex
    {
        Vec2 pos;             // UI units
        Vec2 uv;
        std::uint32_t color;  // RGBA8, R in the lowest byte (Color::ToRgba8)
    };
    static_assert(sizeof(Vertex) == 20, "Vertex is the 20-byte UI vertex format (rhi::VertexLayout::UiVertex)");

    // An allocator whose value-initialization leaves the memory as it is: a list reserves a run of quads and writes
    // them in place (QuadWriter), and std::vector's resize zeroed every one first (as much again as writing them).
    template <class T>
    struct UninitAllocator : std::allocator<T>
    {
        template <class U>
        struct rebind
        {
            using other = UninitAllocator<U>;
        };
        UninitAllocator() = default;
        template <class U>
        UninitAllocator(const UninitAllocator<U>&) noexcept {}
        template <class U>
        void construct(U*) noexcept {}
        template <class U, class... A>
        void construct(U* p, A&&... args) { ::new ((void*)p) U(std::forward<A>(args)...); }
    };
    using VertexVector = std::vector<Vertex, UninitAllocator<Vertex>>;
    using IndexVector = std::vector<std::uint32_t, UninitAllocator<std::uint32_t>>;

    // What a quad that is a plain shape draws (QuadInstance::SetShape): the fill or the stroke of a rounded rectangle
    // (one radius on every corner, the continuous corners of fx::ShapeKind::RoundRect) or of a segment (round caps; its
    // stroke runs round the capsule) - the coverage the FX shader gives the same shape's fill or stroke, from the same
    // distance (esia_quad.hlsl) - or a solid rectangle, or a polyline's piece or round end: the pixels and the coverage its
    // geometry's triangles give them (Painter::Polyline).
    enum class QuadShape : std::uint32_t
    {
        Fill = 0,            // rect: the rectangle; the radius of its corners
        Stroke = 1,          // ... its stroke: the stroke's width and align (0 inside .. 1 outside)
        SegmentFill = 2,     // rect: the end points (x0, y0, x1, y1); the radius: half the thickness
        SegmentStroke = 3,
        Solid = 4,           // rect: the rectangle's pixels as two triangles cover them, without antialiasing (geometry's)
        LinePiece = 5,       // a polyline's segment between the lines of its two joints (SetLinePiece): rect its end points
        LineCap = 6,         // a polyline's round end (SetLineCap): rect the end and the point before it
    };

    // A quad as the instanced quad programs read it (rhi::ShaderProgram::QuadImage .. QuadLcdGray, QuadShapeImage ..
    // QuadShapeLcdGray): two float4 rows of the frame's instance data, 32 bytes where a quad of vertices and indices
    // takes 104 - a textured quad (a glyph, a flat tile: Set) or a plain shape (SetShape), so that shapes and the text
    // among them draw in one draw. Whole numbers and plain floats, exact in a float, so every shader model decodes them
    // (SM3 without integers too):
    //   rect     x0, y0, x1, y1 (UI units); a segment's end points
    //   packed   a textured quad: u0 + 4096 v0, (u1 - u0) + 4096 (v1 - v0) (its uv in 1/4096 - exact for whole texels of
    //            power-of-two textures up to 4096 wide, as glyph pages and tiles are), R + 256 G + 65536 B, A (0 .. 255)
    //            a shape: -1 - (kind + 8 (smoothing in 1/1023 + 1024 stroke align in 1/64)), its radius (any float),
    //            R + 256 G + 65536 B, A + 256 stroke width (in 1/64 UI units, below 1024)
    //            a polyline's piece or end: -1 - (kind + 8 (reach + 32 joint)), half its thickness, R + 256 G + 65536 B,
    //            A + 256 (the piece's other joint | the end's sides); a joint: its line's direction (LineDirection)
    // uv1 >= uv0: no mirrored quads (images that need them are geometry).
    struct QuadInstance
    {
        float rect[4];
        float packed[4];

        void Set(const Rect& r, Vec2 uv0, Vec2 uv1, std::uint32_t color)
        {
            rect[0] = r.min.x;
            rect[1] = r.min.y;
            rect[2] = r.max.x;
            rect[3] = r.max.y;
            PackUv(uv0, uv1, packed);
            PackColor(color, packed + 2);
        }
        // `r`: the rectangle, or a segment's end points (min = the first, max = the second). Smoothing and align from 0 to
        // 1, the stroke's width below 1024 UI units.
        void SetShape(QuadShape kind, const Rect& r, float radius, float smoothing, float strokeWidth, float strokeAlign, std::uint32_t color)
        {
            rect[0] = r.min.x;
            rect[1] = r.min.y;
            rect[2] = r.max.x;
            rect[3] = r.max.y;
            const std::uint32_t s = (std::uint32_t)((smoothing > 1.0f ? 1.0f : smoothing > 0.0f ? smoothing : 0.0f) * 1023.0f + 0.5f);
            const std::uint32_t a = (std::uint32_t)((strokeAlign > 1.0f ? 1.0f : strokeAlign > 0.0f ? strokeAlign : 0.0f) * 64.0f + 0.5f);
            const std::uint32_t w = (std::uint32_t)((strokeWidth < 1023.98f ? (strokeWidth > 0.0f ? strokeWidth : 0.0f) : 1023.98f) * 64.0f + 0.5f);
            packed[0] = -1.0f - (float)((std::uint32_t)kind + 8u * (s + 1024u * a));
            packed[1] = radius;
            packed[2] = (float)(color & 0xFFFFFFu);
            packed[3] = (float)((color >> 24) + 256u * w);
        }
        // A polyline's piece (QuadShape::LinePiece): the segment from `a` to `b`, `halfWidth` either side of it, between
        // the lines through its ends in the directions `jointA` and `jointB` (LineDirection: the lines its geometry's
        // triangles end on, shared with the pieces before and after). The quad reaches along them (1 + reach / 16) times
        // the line's half thickness and half pixel of antialiasing, either side (reach 0 .. 31).
        void SetLinePiece(Vec2 a, Vec2 b, float halfWidth, std::uint32_t jointA, std::uint32_t jointB, std::uint32_t reach, std::uint32_t color)
        {
            SetLine(QuadShape::LinePiece, a, b, halfWidth, jointA, reach, jointB, color);
        }
        // Its round end (QuadShape::LineCap): half a polygon of `sides` sides (3 .. 12) around `end`, away from `prev`,
        // beyond the line through `end` in the direction `joint` (the end piece's, as far along it).
        void SetLineCap(Vec2 end, Vec2 prev, float halfWidth, std::uint32_t joint, std::uint32_t reach, std::uint32_t sides, std::uint32_t color)
        {
            SetLine(QuadShape::LineCap, end, prev, halfWidth, joint, reach, sides, color);
        }
        void SetLine(QuadShape kind, Vec2 a, Vec2 b, float halfWidth, std::uint32_t joint, std::uint32_t reach, std::uint32_t other, std::uint32_t color)
        {
            rect[0] = a.x;
            rect[1] = a.y;
            rect[2] = b.x;
            rect[3] = b.y;
            packed[0] = -1.0f - (float)((std::uint32_t)kind + 8u * ((reach & 31u) + 32u * (joint & 0xFFFFu)));
            packed[1] = halfWidth;
            packed[2] = (float)(color & 0xFFFFFFu);
            packed[3] = (float)((color >> 24) + 256u * (other & 0xFFFFu));
        }
        // A line's direction in 16 bits, the same for both of its senses: its diamond angle - y / (|x| + y) on the upper
        // half-plane, 0 to 2 for half a turn - in steps of 1/32768. Exact to decode (esia_quad.hlsl: the vector
        // (1 - d, d) or (1 - d, 2 - d), normalized), no trigonometry either way; at most 3e-5 radians off.
        static std::uint32_t LineDirection(Vec2 v) { return (std::uint32_t)LineDirectionCode(v); }
        // ... as a float (a whole number, exact): the diamond angle around the whole turn is 1 - x / (|x| + |y|) above
        // the x axis and 3 + x / (|x| + |y|) below it; half a turn later it is 2 more, so 2 apart - 65536 steps - is the
        // same line, and the steps wrap at 65536 (one division and a sign, no branch: the joints of a line across a
        // chart turn either way at random).
        static float LineDirectionCode(Vec2 v)
        {
            // (a statement each: no compiler fuses the product and the sum - every level of the polyline loops computes
            // this alike, render/line_kernels.cpp)
            const float s = (v.x < 0.0f ? -v.x : v.x) + (v.y < 0.0f ? -v.y : v.y);
            const float p = v.x / (s > 1e-30f ? s : 1e-30f);
            const float t = 1.0f - (v.y < 0.0f ? -p : p);
            const float scaled = t * 32768.0f;
            const float steps = (scaled + 12582912.0f) - 12582912.0f;   // rounded: 1.5 2^23, whole floats
            return steps >= 65536.0f ? steps - 65536.0f : steps;
        }
        bool IsShape() const { return packed[0] < 0.0f; }
        // a shape's fields (SetShape's, after the rounding to their steps)
        std::uint32_t ShapeCode() const { return (std::uint32_t)(-packed[0]) - 1u; }
        QuadShape Shape() const { return (QuadShape)(ShapeCode() & 7u); }
        float Radius() const { return packed[1]; }
        float Smoothing() const { return (float)((ShapeCode() >> 3) & 1023u) / 1023.0f; }
        float StrokeAlign() const { return (float)(ShapeCode() >> 13) / 64.0f; }
        float StrokeWidth() const { return (float)((std::uint32_t)packed[3] >> 8) / 64.0f; }
        std::uint32_t Alpha() const { return (std::uint32_t)packed[3] & 255u; }
        // a polyline's piece or end (SetLine's): its (first) joint's direction, its reach, the piece's other joint | the sides
        std::uint32_t LineJoint() const { return ShapeCode() >> 8; }
        std::uint32_t LineReach() const { return (ShapeCode() >> 3) & 31u; }
        std::uint32_t LineOther() const { return (std::uint32_t)packed[3] >> 8; }
        // What it draws on (UI units): a textured quad its rect; a shape's reach, with the part of a stroke outside the
        // edge (the antialiasing's pixel around it is the renderer's); a polyline's piece as far as its miters go, its
        // end the half disc
        Rect Bounds() const
        {
            if (!IsShape())
                return Rect(rect[0], rect[1], rect[2], rect[3]);
            const QuadShape kind = Shape();
            if (kind == QuadShape::LineCap)
                return Rect(rect[0] - packed[1], rect[1] - packed[1], rect[0] + packed[1], rect[1] + packed[1]);
            if (kind == QuadShape::LinePiece)
            {
                const float e = packed[1] * (1.0f + (float)LineReach() / 16.0f);
                return Rect((rect[0] < rect[2] ? rect[0] : rect[2]) - e, (rect[1] < rect[3] ? rect[1] : rect[3]) - e,
                            (rect[0] > rect[2] ? rect[0] : rect[2]) + e, (rect[1] > rect[3] ? rect[1] : rect[3]) + e);
            }
            const float out = (kind == QuadShape::Stroke || kind == QuadShape::SegmentStroke) ? StrokeWidth() * StrokeAlign() : 0.0f;
            if (kind != QuadShape::SegmentFill && kind != QuadShape::SegmentStroke)
                return Rect(rect[0] - out, rect[1] - out, rect[2] + out, rect[3] + out);
            const float e = Radius() + out;
            return Rect((rect[0] < rect[2] ? rect[0] : rect[2]) - e, (rect[1] < rect[3] ? rect[1] : rect[3]) - e,
                        (rect[0] > rect[2] ? rect[0] : rect[2]) + e, (rect[1] > rect[3] ? rect[1] : rect[3]) + e);
        }
        static void PackUv(Vec2 uv0, Vec2 uv1, float* out)
        {
            const float u0 = (float)(std::int32_t)(uv0.x * 4096.0f + 0.5f), v0 = (float)(std::int32_t)(uv0.y * 4096.0f + 0.5f);
            const float u1 = (float)(std::int32_t)(uv1.x * 4096.0f + 0.5f), v1 = (float)(std::int32_t)(uv1.y * 4096.0f + 0.5f);
            out[0] = u0 + 4096.0f * v0;
            out[1] = (u1 - u0) + 4096.0f * (v1 - v0);
        }
        static void PackColor(std::uint32_t color, float* out)
        {
            out[0] = (float)(color & 0xFFFFFFu);   // RGBA8 has R in the lowest byte: R + 256 G + 65536 B
            out[1] = (float)(color >> 24);
        }
    };
    static_assert(sizeof(QuadInstance) == 32, "QuadInstance is two float4 rows of the instance data");
    using QuadVector = std::vector<QuadInstance, UninitAllocator<QuadInstance>>;

    enum class DrawCmdKind : std::uint8_t
    {
        Geometry,     // indices [first, first + count)
        Quads,        // quads [first, first + count) of Quads(), drawn instanced
        Fx,           // instances [first, first + count) of FxInstances()
        LayerBegin,   // Layers()[payload]
        LayerEnd,
        FadeBegin,    // Fades()[payload]; the commands that follow in this list fade out at the edges
        FadeEnd,
        Callback,     // host code runs at this point of the frame (renderer-specific state in `renderState`)
    };

    class DrawList;
    struct DrawCmd;
    using DrawCallback = void (*)(const DrawList& list, const DrawCmd& cmd, void* renderState);

    struct DrawCmd
    {
        DrawCmdKind kind = DrawCmdKind::Geometry;
        Rect clip;
        TextureId texture = 0;       // Geometry / Quads / Fx: 0 = white
        // Quads: plain shapes only so far (they read no texture) - quads on any texture join them, and the first textured
        // one gives the command its texture
        bool anyTexture = false;
        bool shapes = false;         // Quads: plain shapes among them (the programs that draw shapes; text alone: lighter ones)
        std::uint32_t first = 0;
        std::uint32_t count = 0;
        EffectId effect = 0;         // Fx
        std::uint32_t payload = 0;   // LayerBegin / FadeBegin
        DrawCallback callback = nullptr;
        void* userData = nullptr;
        // Geometry: its vertices [vtxFirst, vtxEnd) and their bounds (empty: none), kept as they are written - the
        // renderer's planner needs every command's bounds, and scanning them back through the indices cost it more
        // than writing them did. Quads: the bounds of its quads.
        std::uint32_t vtxFirst = 0, vtxEnd = 0;
        Rect vtxBounds{1e30f, 1e30f, -1e30f, -1e30f};
    };

    class ESIA_API DrawList
    {
    public:
        // Clears everything and starts over with `clip` as the outermost clip rect.
        void Reset(const Rect& clip);

        // ---- state
        void PushClipRect(const Rect& r, bool intersect = true);
        void PopClipRect();
        const Rect& ClipRect() const { return clipStack_.back(); }
        void PushTexture(TextureId texture);
        void PopTexture();
        TextureId CurrentTexture() const { return textureStack_.empty() ? 0 : textureStack_.back(); }

        // ---- geometry (uses the current clip rect and texture)
        // Reserves room and returns the index of the first new vertex; write with WriteVertex / WriteIndex.
        std::uint32_t PrimBegin(std::uint32_t indexCount, std::uint32_t vertexCount);
        void WriteVertex(Vec2 pos, Vec2 uv, std::uint32_t color)
        {
            vtx_.push_back({pos, uv, color});
            DrawCmd& c = cmds_.back();
            c.vtxEnd = (std::uint32_t)vtx_.size();
            c.vtxBounds = c.vtxBounds.Union(Rect(pos, pos));
        }
        void WriteIndex(std::uint32_t i) { idx_.push_back(i); ++cmds_.back().count; }
        void WriteTriangle(std::uint32_t a, std::uint32_t b, std::uint32_t c) { WriteIndex(a); WriteIndex(b); WriteIndex(c); }

        void AddRectFilled(const Rect& r, std::uint32_t color);
        void AddRectFilledUV(const Rect& r, Vec2 uv0, Vec2 uv1, std::uint32_t color);
        void AddImage(TextureId texture, const Rect& r, Vec2 uv0, Vec2 uv1, std::uint32_t color);
        void AddTriangleFilled(Vec2 a, Vec2 b, Vec2 c, std::uint32_t color);
        void AddConvexPolyFilled(const Vec2* points, int count, std::uint32_t color);

        // ---- textured quads, drawn instanced (glyphs, flat tiles: QuadInstance; the current clip rect)
        // A run of quads on `texture` (a text's glyphs): room for `maxQuads` reserved once, the quads written in place.
        // Nothing else may be added to the list until EndQuads, which keeps the quads written.
        struct QuadWriter
        {
            QuadInstance* quad = nullptr;
            std::uint32_t written = 0;
            Rect bounds{1e30f, 1e30f, -1e30f, -1e30f};
            void Add(const Rect& r, Vec2 uv0, Vec2 uv1, std::uint32_t color)
            {
                bounds = bounds.Union(r);
                quad[written++].Set(r, uv0, uv1, color);
            }
        };
        QuadWriter BeginQuads(TextureId texture, std::uint32_t maxQuads);
        void EndQuads(const QuadWriter& w);
        // One quad (an image of a glyph or a tile); nothing for an empty rect or a transparent color.
        void AddQuad(TextureId texture, const Rect& r, Vec2 uv0, Vec2 uv1, std::uint32_t color);
        // A plain shape (QuadInstance::SetShape) in the current clip rect: it joins the quads before it whatever their
        // texture, so a shape among labels takes no draw of its own. Nothing for a transparent color. `bounds`: what
        // it draws on, as QuadInstance::Bounds gives it (a caller that knows them saves decoding them).
        void AddShapeQuad(const QuadInstance& shape) { AddShapeQuad(shape, shape.Bounds()); }
        void AddShapeQuad(const QuadInstance& shape, const Rect& bounds);
        // Shapes of one color in one go (a polyline's pieces and ends), `bounds` holding all of them; nothing for a
        // transparent color.
        void AddShapeQuads(const QuadInstance* shapes, std::size_t count, const Rect& bounds);
        // ... written in place: room for `count` shapes at the end of the quads, in a command that draws them, for the
        // caller to fill - every one of them, in a color that is not transparent.
        QuadInstance* AppendShapeQuads(std::size_t count, const Rect& bounds);
        // Room for `vertexCount` vertices and `indexCount` indices written in place (the indices count from `base`),
        // then PrimCommit with their bounds: what WriteVertex / WriteIndex do one at a time. The current clip and
        // texture; nothing else may be added in between.
        struct PrimWriter
        {
            Vertex* vtx = nullptr;
            std::uint32_t* idx = nullptr;
            std::uint32_t base = 0;
            DrawCmd* cmd = nullptr;
        };
        PrimWriter PrimReserve(std::uint32_t indexCount, std::uint32_t vertexCount);
        // The same on `texture` (not the current one: nothing pushed or popped)
        PrimWriter PrimReserve(TextureId texture, std::uint32_t indexCount, std::uint32_t vertexCount);
        void PrimCommit(std::uint32_t indexCount, const Rect& bounds);
        void PrimCommit(const PrimWriter& w, std::uint32_t indexCount, const Rect& bounds)
        {
            DrawCmd& c = *w.cmd;
            c.count += indexCount;
            c.vtxEnd = (std::uint32_t)vtx_.size();
            Rect& b = c.vtxBounds;
            b.min.x = bounds.min.x < b.min.x ? bounds.min.x : b.min.x;
            b.min.y = bounds.min.y < b.min.y ? bounds.min.y : b.min.y;
            b.max.x = bounds.max.x > b.max.x ? bounds.max.x : b.max.x;
            b.max.y = bounds.max.y > b.max.y ? bounds.max.y : b.max.y;
        }

        // ---- FX command stream
        void AddFx(const fx::Instance& instance, EffectId effect = 0, TextureId texture = 0);
        void BeginLayer(const fx::LayerParams& params);
        void EndLayer();
        void BeginFade(const fx::FadeParams& params);
        void EndFade();
        void AddCallback(DrawCallback callback, void* userData);

        // ---- command order
        // A position between commands: nothing drawn after it merges into a command before it. With MoveCommands
        // a widget draws something after its content and moves it under the content (a card's background, whose
        // height is known at its end). Marks are used last-in, first-out (a card in a card): moving commands to
        // a mark shifts the positions after it.
        std::size_t Mark();
        // Moves commands [from, end) to position `to` (<= from), keeping their order. Vertices, indices and FX
        // instances stay where they are: commands address them by range.
        void MoveCommands(std::size_t from, std::size_t to);

        // ---- read access (renderer, tests)
        const VertexVector& Vertices() const { return vtx_; }
        // Painter::PopScale rewrites positions: whoever does calls RefreshBounds with the first vertex and the first
        // quad it changed
        VertexVector& Vertices() { return vtx_; }
        void RefreshBounds(std::size_t fromVertex, std::size_t fromQuad = (std::size_t)-1);
        const IndexVector& Indices() const { return idx_; }
        const QuadVector& Quads() const { return quads_; }
        QuadVector& Quads() { return quads_; }
        const std::vector<DrawCmd>& Commands() const { return cmds_; }
        const std::vector<fx::Instance>& FxInstances() const { return fx_; }
        const std::vector<fx::LayerParams>& Layers() const { return layers_; }
        const std::vector<fx::FadeParams>& Fades() const { return fades_; }
        // A fade begun earlier in the frame may still change (a bar floating over the bottom of its area)
        std::vector<fx::FadeParams>& Fades() { return fades_; }
        bool Empty() const { return cmds_.empty(); }

    private:
        DrawCmd& Geometry() { return Geometry(CurrentTexture()); }   // current geometry command (opens one when the state changed)
        DrawCmd& Geometry(TextureId texture);
        // the current quad command on `texture` (opens one when the state changed); a shape's (`shape`) takes any texture
        DrawCmd& QuadCommand(TextureId texture, bool shape = false);
        void Reserve(std::uint32_t indexCount, std::uint32_t vertexCount);
        DrawCmd& Push(DrawCmdKind kind);

        VertexVector vtx_;
        IndexVector idx_;
        QuadVector quads_;
        std::vector<DrawCmd> cmds_;
        std::vector<fx::Instance> fx_;
        std::vector<fx::LayerParams> layers_;
        std::vector<fx::FadeParams> fades_;
        std::vector<Rect> clipStack_{Rect(-8192, -8192, 8192, 8192)};
        std::vector<TextureId> textureStack_;
        std::size_t mergeBarrier_ = 0;   // commands before this index take no more primitives (Mark)
    };

    // One frame of output: draw lists in back-to-front order.
    struct DrawData
    {
        std::vector<const DrawList*> lists;
        Vec2 displayPos;                  // UI-unit origin of the target
        Vec2 displaySize;                 // UI units
        Vec2 framebufferScale{1, 1};      // render-target pixels per UI unit
        double time = 0.0;                // seconds (shader clock)
        float deltaTime = 0.0f;

        std::size_t TotalVertices() const;
        std::size_t TotalIndices() const;
        std::size_t TotalQuads() const;
        std::size_t TotalFx() const;
    };
}
