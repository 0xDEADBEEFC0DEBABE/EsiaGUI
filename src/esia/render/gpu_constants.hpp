// Esia - constants shared by the frame planner, the renderer and the shaders (the port of WGT's
// src/render/gpu_common.hpp without its DXGI part: formats are rhi::Format now).
#pragma once
#include "esia/render/frame_plan.hpp"
#include "esia/rhi/rhi.hpp"
#include <algorithm>
#include <cmath>

namespace esia::render
{
    constexpr int kBackdropLevels = rhi::kBackdropLevels;   // [0] full-res copy, [1..5] = 1/2 .. 1/32

    // b0 - mirrors cbuffer WgtFrame (esia_common.hlsli)
    struct FrameConstants
    {
        float xform[4];      // clip.xy = pos.xy * xform.xy + xform.zw
        float target[4];     // render target size, 1 / size
        float display[4];    // UI display pos, framebuffer scale
        float time[4];       // seconds, delta, backdrop valid, 1 = write linear (sRGB target)
        float level[kBackdropLevels][4];
        float text[4];       // gamma, grayscale contrast, 0, 0
        float conv[4];       // 1 = framebuffer origin bottom-left, SV_Position offset, FX instances per texture row, 0
    };
    static_assert(sizeof(FrameConstants) == 12 * 16, "FrameConstants must mirror WgtFrame");

    // b1 - mirrors cbuffer WgtPass
    struct PassConstants
    {
        float p0[4];
        float p1[4];
    };

    // b2 - mirrors cbuffer WgtDraw: edge fade of scroll views (RenderOp::fade) and the draw's first FX instance
    struct DrawConstants
    {
        float fade[4];
        float info[4];
    };

    inline int LevelSize(int full, int level) { return std::max(1, (full + (1 << level) - 1) >> level); }

    // Pyramid levels are only rebuilt inside the capture region; the downsample clamps its taps to the refreshed
    // region, so the only padding needed is the B-spline footprint at the coarsest level (2 texels).
    constexpr float kPyramidMargin = 64.0f;

    // Blur radii (UI units) the glass shader samples besides its own frost (esia_fx.hlsl, EvalGlass): the rim's
    // reflection of the surroundings, and the neighbourhood that sets the legibility exposure.
    constexpr float kGlassEnvBlur = 16.0f;
    constexpr float kGlassAmbientBlur = 36.0f;
    // How far outside its edge the rim reads the surroundings it reflects (UI units, from the clamped bezel). Kept
    // short: the reflection is faint and blurred, and every unit here widens the backdrop capture around all glass.
    inline float GlassEnvReach(float bezel) { return std::min(bezel * 0.5f, 16.0f); }
    // The pyramid level the legibility exposure reads (one bilinear read; WgtAmbientLevel in esia_fx.hlsl).
    inline int AmbientLevel(float renderScale)
    {
        return std::clamp((int)std::lround(std::log2(kGlassAmbientBlur * renderScale) - 1.0f), 1, 4);
    }

    // Pyramid levels needed to reconstruct a blur radius (px) with WgtSampleBackdrop (level + next for lerp).
    inline int LevelsForBlur(float radiusPx)
    {
        const float lv = std::clamp(std::log2(std::max(radiusPx, 1.0f)) - 1.0f, 0.0f, 5.0f);
        return std::clamp((int)std::floor(lv) + 1, 1, kBackdropLevels - 1);
    }
    // Levels needed by the glow-layer composite (level, next, and the wide tail two levels down).
    inline int LevelsForBloom(float radiusPx)
    {
        const float lv = std::clamp(std::log2(std::max(radiusPx, 2.0f)) - 1.0f, 1.0f, 5.0f);
        return std::clamp((int)std::floor(lv) + 2, 1, kBackdropLevels - 1);
    }

    // Pads + aligns a capture region to the coarsest pyramid level and clamps it to the target.
    inline PxRect AlignCaptureRegion(const PxRect& r, int width, int height)
    {
        const float a = 32.0f;
        const float m = kPyramidMargin + a;
        PxRect o;
        o.x0 = std::max(0.0f, std::floor((r.x0 - m) / a) * a);
        o.y0 = std::max(0.0f, std::floor((r.y0 - m) / a) * a);
        o.x1 = std::min((float)width, std::ceil((r.x1 + m) / a) * a);
        o.y1 = std::min((float)height, std::ceil((r.y1 + m) / a) * a);
        return o;
    }

    // The region copied for level 0: whole 32-pixel blocks around `r` (level 0 is read where it is, no pyramid margin).
    inline PxRect AlignCopyRegion(const PxRect& r, int width, int height)
    {
        const float a = 32.0f;
        return {std::max(0.0f, std::floor(r.x0 / a) * a), std::max(0.0f, std::floor(r.y0 / a) * a), std::min((float)width, std::ceil(r.x1 / a) * a),
                std::min((float)height, std::ceil(r.y1 / a) * a)};
    }

    // One downsample step of a region-limited pyramid build.
    struct PyramidStep
    {
        PxRect valid;   // texels of the current source level that hold fresh data

        static PyramidStep First(const PxRect& region, int width, int height)
        {
            PyramidStep s;
            s.valid = region.Intersect(PxRect{0, 0, (float)width, (float)height});
            return s;
        }

        // Texels of `level` to refresh: the scaled region plus the B-spline footprint (2 texels).
        PxRect Next(const PxRect& region, int level, int width, int height) const
        {
            const float k = 1.0f / (float)(1 << level);
            const PxRect r{std::floor(region.x0 * k) - 2.0f, std::floor(region.y0 * k) - 2.0f, std::ceil(region.x1 * k) + 2.0f, std::ceil(region.y1 * k) + 2.0f};
            return r.Intersect(PxRect{0, 0, (float)width, (float)height});
        }

        // b1 of DownsamplePS: source texel size + valid source uv rect (inset half a texel, so bilinear taps never
        // touch a stale texel). Top-left uv: the shader flips it for bottom-left APIs (WgtRtUv).
        PassConstants Constants(int srcW, int srcH) const
        {
            PassConstants pc{};
            pc.p0[0] = 1.0f / (float)srcW;
            pc.p0[1] = 1.0f / (float)srcH;
            pc.p1[0] = (valid.x0 + 0.5f) / (float)srcW;
            pc.p1[1] = (valid.y0 + 0.5f) / (float)srcH;
            pc.p1[2] = std::max(valid.x1 - 0.5f, valid.x0 + 0.5f) / (float)srcW;
            pc.p1[3] = std::max(valid.y1 - 0.5f, valid.y0 + 0.5f) / (float)srcH;
            return pc;
        }
    };
}
