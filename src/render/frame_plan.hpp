// WGT UI - backend-agnostic frame planner.
//
// Walks ImDrawData once and produces a flat list of render operations:
//   * regular ImGui draws (with vertex bounds for dirty tracking),
//   * FX batches: consecutive fx shape commands sharing clip/texture/effect become ONE instanced draw,
//     split whenever a glass shape would need to "see" something drawn earlier in the same batch,
//   * glow-layer begin/end,
//   * foreign user callbacks.
// Backends only execute the plan; batching and backdrop-capture decisions are shared by DX11 and DX12.
#pragma once
#include "wgt/fx.hpp"
#include "wgt/math.hpp"
#include <algorithm>
#include <cfloat>
#include <cstddef>
#include <vector>

namespace wgt
{
    struct PxRect
    {
        float x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        bool Empty() const { return x1 <= x0 || y1 <= y0; }
        bool Overlaps(const PxRect& o) const { return !Empty() && !o.Empty() && o.x0 < x1 && o.x1 > x0 && o.y0 < y1 && o.y1 > y0; }
        bool Contains(const PxRect& o) const { return !Empty() && o.x0 >= x0 && o.y0 >= y0 && o.x1 <= x1 && o.y1 <= y1; }
        PxRect Union(const PxRect& o) const
        {
            if (Empty()) return o;
            if (o.Empty()) return *this;
            return {std::min(x0, o.x0), std::min(y0, o.y0), std::max(x1, o.x1), std::max(y1, o.y1)};
        }
        PxRect Intersect(const PxRect& o) const { return {std::max(x0, o.x0), std::max(y0, o.y0), std::min(x1, o.x1), std::min(y1, o.y1)}; }
        PxRect Expand(float a) const { return {x0 - a, y0 - a, x1 + a, y1 + a}; }
    };

    struct RenderOp
    {
        enum Type : std::uint8_t
        {
            Draw,
            FxBatch,
            LayerBegin,
            LayerEnd,
            ResetState,
            UserCallback,
        };

        Type type = Draw;
        PxRect clip;                 // scissor in framebuffer pixels
        PxRect bounds;               // pixels touched (for dirty tracking / layers / capture region)
        PxRect core;                 // visually significant part of `bounds` (shapes + the visible part of their
                                     // shadows / glows): what later glass must actually see (capture planning).
                                     // Liquid glass shapes are left out: they go to `glassShape`.
        ImTextureID texture = ImTextureID_Invalid;
        // Draw
        std::uint32_t idxCount = 0, idxOffset = 0, vtxOffset = 0;
        bool coverage = false;       // glyph coverage texture: render with a text pipeline
        bool lcd = false;            // ... sub-pixel (RGB) coverage: dual-source blended text pipeline
        // FxBatch
        std::uint32_t instStart = 0, instCount = 0;
        EffectId effect = 0;
        bool glass = false;
        PxRect glassRegion;          // area whose backdrop must be valid before drawing (generous: capture extent)
        PxRect glassCore;            // area whose content visibly shows through (shape + blur + refraction)
        PxRect glassShape;           // liquid glass shapes the batch draws (without shadows / glows)
        float blurPx = 0.0f;         // largest backdrop blur read by the batch (px), < 0 = unknown (user effect)
                                     // LayerEnd: bloom radius (px)
        bool readsLevel0 = false;    // glass batch: something reads the full-resolution backdrop (clear glass, a
                                     // frost under 4 px, user effects) - frosted glass only reads blurred levels
        bool captureLevel0 = true;   // glass batch with a capture: that capture must provide level 0 (else the
                                     // backend may build the pyramid straight from the render target, no copy)
        float fade[4] = {0, 0, 0, 0}; // Draw / FxBatch: edge fade (top y, bottom y, top width, bottom width; px)
        PxRect captureRegion;        // glass batch: capture the backdrop over this region first (empty = reuse)
        int captureLevels = 0;       // pyramid levels that capture must build
        // LayerBegin
        fx::LayerParams layer{};
        // UserCallback
        const ImDrawList* list = nullptr;
        const ImDrawCmd* cmd = nullptr;
    };

    struct FramePlan
    {
        std::vector<RenderOp> ops;
        std::vector<fx::Instance> instances;
        int totalVtx = 0;
        int totalIdx = 0;
        bool anyGlass = false;
        bool anyLayer = false;
        int fxCount = 0;
        int plannedCaptures = 0;
        // resetCallback: the backend's platform_io.DrawCallback_ResetRenderState marker.
        void Build(const ImDrawData* dd, ImDrawCallback resetCallback);

    private:
        // Decides before which glass batches the backdrop is captured and over which region (see .cpp).
        void PlanCaptures();
    };
}
