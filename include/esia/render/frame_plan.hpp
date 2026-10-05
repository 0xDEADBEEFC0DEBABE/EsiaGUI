// Esia - backend-agnostic frame planner (a port of WGT's src/render/frame_plan.*).
//
// Walks DrawData once and produces a flat list of render operations plus the frame's merged buffers:
//   * geometry draws (indices rebased into one index buffer, vertex bounds for dirty tracking),
//   * FX batches: instances sharing clip / texture / effect become ONE instanced draw, split whenever a glass shape
//     would need to "see" something drawn earlier in the same batch. A draw also joins an earlier batch (FX or
//     geometry) when everything drawn since touches other pixels: the shapes and the text of a window alternate, and
//     they become one batch of shapes and one draw of text,
//   * glow-layer begin / end, edge fades (per-draw constants), host callbacks,
//   * the backdrop captures: before which glass batch the backdrop is captured and over which region, how many
//     pyramid levels it needs and whether the full-resolution level is read (PlanCaptures).
// The renderer only executes the plan, so every backend renders the same captures and batches.
#pragma once
#include "esia/core/draw_list.hpp"
#include "esia/core/texture.hpp"
#include <algorithm>
#include <functional>
#include <vector>

namespace esia::render
{
    // Rectangle in render-target pixels (top-left origin, y down).
    struct PxRect
    {
        float x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        bool Empty() const { return x1 <= x0 || y1 <= y0; }
        bool Overlaps(const PxRect& o) const { return !Empty() && !o.Empty() && o.x0 < x1 && o.x1 > x0 && o.y0 < y1 && o.y1 > y0; }
        bool Contains(const PxRect& o) const { return !Empty() && o.x0 >= x0 && o.y0 >= y0 && o.x1 <= x1 && o.y1 <= y1; }
        PxRect Union(const PxRect& o) const
        {
            if (Empty())
                return o;
            if (o.Empty())
                return *this;
            return {(std::min)(x0, o.x0), (std::min)(y0, o.y0), (std::max)(x1, o.x1), (std::max)(y1, o.y1)};
        }
        PxRect Intersect(const PxRect& o) const { return {(std::max)(x0, o.x0), (std::max)(y0, o.y0), (std::min)(x1, o.x1), (std::min)(y1, o.y1)}; }
        PxRect Expand(float a) const { return {x0 - a, y0 - a, x1 + a, y1 + a}; }
        bool operator==(const PxRect&) const = default;
    };

    struct RenderOp
    {
        enum Type : std::uint8_t
        {
            Draw,
            FxBatch,
            LayerBegin,
            LayerEnd,
            Callback,
        };

        Type type = Draw;
        PxRect clip;                 // scissor in render-target pixels
        bool clipFree = false;       // Draw / FxBatch: its clips cut nothing it draws - while planning it joins
                                     // batches of other clips that hold it whole; planned, `clip` is any rect that
                                     // holds its bounds on the target (the previous op's, else the whole target)
        PxRect bounds;               // pixels touched (dirty tracking, layer and capture regions)
        PxRect core;                 // visually significant part of `bounds` (shapes + the visible part of their shadows
                                     // / glows): what later glass must see. Liquid glass shapes go to `glassShape`.
        TextureId texture = 0;
        // Draw
        std::uint32_t idxCount = 0, idxOffset = 0;   // into FramePlan::indices
        std::uint32_t geometryFirst = 0, geometryCount = 0;   // direct plans: into FramePlan::geometry
        bool coverage = false;       // glyph coverage texture: the text pipeline
        // FxBatch
        std::uint32_t instStart = 0, instCount = 0;   // into FramePlan::instances
        std::uint32_t features = 0;  // union of the instances' fx::Feature bits (shader variants, Caps::fxFeatureVariants)
        EffectId effect = 0;
        bool glass = false;
        PxRect glassRegion;          // area whose backdrop must be valid before drawing (generous: capture extent)
        PxRect glassCore;            // area whose backdrop must be current: the liquid glass shapes (+2 px of filtering;
                                     // frost spreads what lies beside a shape into it too faintly to see), a user
                                     // effect's whole region
        PxRect glassShape;           // liquid glass shapes the batch draws (without shadows / glows)
        float blurPx = 0.0f;         // largest backdrop blur read by the batch (px), < 0 = unknown (user effect)
                                     // LayerEnd: bloom radius (px)
        bool readsLevel0 = false;    // glass batch: something reads the full-resolution backdrop (clear glass, a frost
                                     // under 4 px, user effects) - frosted glass only reads blurred levels
        PxRect level0Region;         // glass batch: where it reads level 0 (the regions of those instances)
        bool captureLevel0 = true;   // glass batch with a capture: that capture must provide level 0 (else the renderer
                                     // builds the pyramid straight from the render target when it can: no copy)
        PxRect captureLevel0Region;  // glass batch with a capture: the part of it copied for level 0 (the level-0
                                     // regions of the batches it serves; a frosted card around a clear control
                                     // is not copied)
        float fade[4] = {0, 0, 0, 0}; // Draw / FxBatch: edge fade (top y, bottom y, top width, bottom width; px)
        PxRect captureRegion;        // glass batch: capture the backdrop over this region first (empty = reuse)
        int captureLevels = 0;       // pyramid levels that capture must build
        // LayerBegin / LayerEnd
        fx::LayerParams layer{};
        // Callback
        const DrawList* list = nullptr;
        const DrawCmd* cmd = nullptr;
    };

    // Texture properties the planner needs (coverage pages pick the text pipelines). False = unknown texture.
    using TextureInfoFn = std::function<bool(TextureId id, TextureInfo& out)>;

    struct ESIA_API FramePlan
    {
        std::vector<RenderOp> ops;
        std::vector<fx::Instance> instances;
        std::vector<Vertex> vertices;        // every list's vertices, back to front
        IndexVector indices;                 // rebased onto `vertices`

        // A direct plan (Build's `direct`: a device with rhi::Caps::baseVertex) leaves `instances`, `vertices` and
        // `indices` empty: the lists' vertices and indices go to the GPU as they are, every list at its offset in the
        // frame's buffers (`lists`), and a Draw op is `geometryCount` draws from `geometry[geometryFirst]`, each with
        // its list's first vertex as base vertex. The FX instances are `instanceRuns`, in op order.
        struct ListGeometry
        {
            const DrawList* list = nullptr;
            std::uint32_t firstVertex = 0, firstIndex = 0;
        };
        struct GeometryDraw
        {
            std::uint32_t firstIndex = 0, count = 0, baseVertex = 0;
        };
        struct InstanceRun
        {
            const fx::Instance* first = nullptr;
            std::uint32_t count = 0;
        };
        bool direct = false;
        std::vector<ListGeometry> lists;
        std::vector<GeometryDraw> geometry;
        std::vector<InstanceRun> instanceRuns;
        std::uint32_t totalVertices = 0, totalIndices = 0;

        bool anyGlass = false;
        bool anyLayer = false;
        int fxCount = 0;
        int plannedCaptures = 0;

        void Build(const DrawData& dd, const TextureInfoFn& textureInfo, bool direct = false);

    private:
        // Decides before which glass batches the backdrop is captured and over which region (see .cpp).
        void PlanCaptures();

        // Build's scratch: what each op draws, as a chain of runs of instances or indices (see .cpp).
        struct Piece
        {
            const fx::Instance* inst = nullptr;   // FX batch: `count` instances from here
            const std::uint32_t* idx = nullptr;   // draw: `count` indices from here, rebased by `base`
            std::uint32_t count = 0, base = 0, next = 0;
            std::uint32_t firstIndex = 0;          // draw: where `idx` lands in the frame's index buffer (direct)
        };
        std::vector<Piece> pieces_;
        std::vector<std::uint32_t> opFirst_, opLast_;
        std::vector<int> layerStack_;   // the layers open while Build walks the lists
    };
}
