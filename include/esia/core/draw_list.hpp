// Esia - draw lists: what the UI core produces and the renderer consumes.
//
// A DrawList holds ordinary indexed geometry (text, lines, charts, images - `Vertex` triangles) and the FX
// command stream (SDF shapes as fx::Instance, glow-layer and edge-fade brackets, host callbacks) in one ordered
// command list, each command with its clip rect. Consecutive geometry with the same clip and texture merges
// into one command, so do consecutive FX shapes with the same clip, texture and effect.
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

    enum class DrawCmdKind : std::uint8_t
    {
        Geometry,     // indices [first, first + count)
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
        TextureId texture = 0;       // Geometry / Fx: 0 = white
        std::uint32_t first = 0;
        std::uint32_t count = 0;
        EffectId effect = 0;         // Fx
        std::uint32_t payload = 0;   // LayerBegin / FadeBegin
        DrawCallback callback = nullptr;
        void* userData = nullptr;
        // Geometry: its vertices [vtxFirst, vtxEnd) and their bounds (empty: none), kept as they are written - the
        // renderer's planner needs every command's bounds, and scanning them back through the indices cost it more
        // than writing them did
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

        // A run of textured quads (a text's glyphs): room for `maxQuads` reserved once with `texture` current, the
        // quads written in place - the same vertices, indices and command as AddRectFilledUV one by one. Nothing else
        // may be added to the list until EndQuads, which keeps the quads written.
        struct QuadWriter
        {
            Vertex* vtx = nullptr;
            std::uint32_t* idx = nullptr;
            std::uint32_t base = 0, written = 0;
            Rect bounds{1e30f, 1e30f, -1e30f, -1e30f};
            void Add(const Rect& r, Vec2 uv0, Vec2 uv1, std::uint32_t color)
            {
                bounds = bounds.Union(r);
                Vertex* w = vtx + (std::size_t)written * 4;
                w[0] = {r.min, uv0, color};
                w[1] = {Vec2(r.max.x, r.min.y), Vec2(uv1.x, uv0.y), color};
                w[2] = {r.max, uv1, color};
                w[3] = {Vec2(r.min.x, r.max.y), Vec2(uv0.x, uv1.y), color};
                std::uint32_t* x = idx + (std::size_t)written * 6;
                const std::uint32_t b = base + written * 4;
                x[0] = b;
                x[1] = b + 1;
                x[2] = b + 2;
                x[3] = b;
                x[4] = b + 2;
                x[5] = b + 3;
                ++written;
            }
        };
        QuadWriter BeginQuads(TextureId texture, std::uint32_t maxQuads);
        void EndQuads(const QuadWriter& w);

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
        // Painter::PopScale rewrites positions: whoever does calls RefreshBounds with the first vertex it changed
        VertexVector& Vertices() { return vtx_; }
        void RefreshBounds(std::size_t fromVertex);
        const IndexVector& Indices() const { return idx_; }
        const std::vector<DrawCmd>& Commands() const { return cmds_; }
        const std::vector<fx::Instance>& FxInstances() const { return fx_; }
        const std::vector<fx::LayerParams>& Layers() const { return layers_; }
        const std::vector<fx::FadeParams>& Fades() const { return fades_; }
        bool Empty() const { return cmds_.empty(); }

    private:
        DrawCmd& Geometry();   // current geometry command (opens one when the state changed)
        DrawCmd& Push(DrawCmdKind kind);

        VertexVector vtx_;
        IndexVector idx_;
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
        std::size_t TotalFx() const;
    };
}
