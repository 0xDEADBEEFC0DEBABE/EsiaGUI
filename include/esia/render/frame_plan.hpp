// Esia - backend-agnostic frame planner (a port of WGT's src/render/frame_plan.*).
//
// Walks DrawData once and produces a flat list of render operations plus the frame's merged buffers:
//   * geometry draws (indices rebased into one index buffer, vertex bounds for dirty tracking),
//   * FX batches: consecutive instances sharing clip / texture / effect become ONE instanced draw, split whenever a
//     glass shape would need to "see" something drawn earlier in the same batch,
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
        PxRect bounds;               // pixels touched (dirty tracking, layer and capture regions)
        PxRect core;                 // visually significant part of `bounds` (shapes + the visible part of their shadows
                                     // / glows): what later glass must see. Liquid glass shapes go to `glassShape`.
        TextureId texture = 0;
        // Draw
        std::uint32_t idxCount = 0, idxOffset = 0;   // into FramePlan::indices
        bool coverage = false;       // glyph coverage texture: the text pipeline
        // FxBatch
        std::uint32_t instStart = 0, instCount = 0;   // into FramePlan::instances
        std::uint32_t features = 0;  // union of the instances' fx::Feature bits (shader variants, Caps::fxFeatureVariants)
        EffectId effect = 0;
        bool glass = false;
        PxRect glassRegion;          // area whose backdrop must be valid before drawing (generous: capture extent)
        PxRect glassCore;            // area whose content visibly shows through (shape + blur + refraction)
        PxRect glassShape;           // liquid glass shapes the batch draws (without shadows / glows)
        float blurPx = 0.0f;         // largest backdrop blur read by the batch (px), < 0 = unknown (user effect)
                                     // LayerEnd: bloom radius (px)
        bool readsLevel0 = false;    // glass batch: something reads the full-resolution backdrop (clear glass, a frost
                                     // under 4 px, user effects) - frosted glass only reads blurred levels
        bool captureLevel0 = true;   // glass batch with a capture: that capture must provide level 0 (else the renderer
                                     // builds the pyramid straight from the render target when it can: no copy)
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
        std::vector<std::uint32_t> indices;  // rebased onto `vertices`
        bool anyGlass = false;
        bool anyLayer = false;
        int fxCount = 0;
        int plannedCaptures = 0;

        void Build(const DrawData& dd, const TextureInfoFn& textureInfo);

    private:
        // Decides before which glass batches the backdrop is captured and over which region (see .cpp).
        void PlanCaptures();
    };
}
