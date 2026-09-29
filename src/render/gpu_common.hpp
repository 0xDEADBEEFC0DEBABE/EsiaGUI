// WGT UI - constants and DXGI helpers shared by the D3D11 and D3D12 backends.
#pragma once
#include <dxgiformat.h>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include "render/frame_plan.hpp"

namespace wgt
{
    constexpr int kBackdropLevels = 6;           // [0] full-res copy, [1..5] = 1/2 .. 1/32
    constexpr DXGI_FORMAT kPyramidFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
    constexpr DXGI_FORMAT kLayerFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

    // b0 - mirrors cbuffer WgtFrame (wgt_common.hlsli)
    struct FrameConstants
    {
        float xform[4];
        float target[4];
        float display[4];
        float time[4];
        float level[kBackdropLevels][4];
        float text[4];   // x gamma, y grayscale enhanced contrast, z ClearType enhanced contrast, w ClearType level
    };
    static_assert(sizeof(FrameConstants) % 16 == 0, "cbuffer size");

    // b1 - mirrors cbuffer WgtPass
    struct PassConstants
    {
        float p0[4];
        float p1[4];
    };

    inline int LevelSize(int full, int level) { return std::max(1, (full + (1 << level) - 1) >> level); }

    inline bool IsSrgb(DXGI_FORMAT f)
    {
        return f == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB || f == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB || f == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB ||
               f == DXGI_FORMAT_BC1_UNORM_SRGB || f == DXGI_FORMAT_BC2_UNORM_SRGB || f == DXGI_FORMAT_BC3_UNORM_SRGB || f == DXGI_FORMAT_BC7_UNORM_SRGB;
    }

    // Storage format for the backdrop copy (typeless where possible so we can read it without sRGB decode).
    inline DXGI_FORMAT CopyStorageFormat(DXGI_FORMAT f)
    {
        switch (f)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM: case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            return DXGI_FORMAT_R8G8B8A8_TYPELESS;
        case DXGI_FORMAT_B8G8R8A8_UNORM: case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            return DXGI_FORMAT_B8G8R8A8_TYPELESS;
        case DXGI_FORMAT_B8G8R8X8_UNORM: case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: case DXGI_FORMAT_B8G8R8X8_TYPELESS:
            return DXGI_FORMAT_B8G8R8X8_TYPELESS;
        case DXGI_FORMAT_R10G10B10A2_UNORM: case DXGI_FORMAT_R10G10B10A2_TYPELESS:
            return DXGI_FORMAT_R10G10B10A2_TYPELESS;
        case DXGI_FORMAT_R16G16B16A16_FLOAT: case DXGI_FORMAT_R16G16B16A16_TYPELESS:
            return DXGI_FORMAT_R16G16B16A16_TYPELESS;
        default:
            return f;
        }
    }

    inline DXGI_FORMAT CopyViewFormat(DXGI_FORMAT storage)
    {
        switch (storage)
        {
        case DXGI_FORMAT_R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case DXGI_FORMAT_B8G8R8A8_TYPELESS: return DXGI_FORMAT_B8G8R8A8_UNORM;
        case DXGI_FORMAT_B8G8R8X8_TYPELESS: return DXGI_FORMAT_B8G8R8X8_UNORM;
        case DXGI_FORMAT_R10G10B10A2_TYPELESS: return DXGI_FORMAT_R10G10B10A2_UNORM;
        case DXGI_FORMAT_R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_FLOAT;
        default: return storage;
        }
    }

    // Typed format usable for ResolveSubresource.
    inline DXGI_FORMAT ResolveFormat(DXGI_FORMAT rt)
    {
        switch (rt)
        {
        case DXGI_FORMAT_R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case DXGI_FORMAT_B8G8R8A8_TYPELESS: return DXGI_FORMAT_B8G8R8A8_UNORM;
        case DXGI_FORMAT_R10G10B10A2_TYPELESS: return DXGI_FORMAT_R10G10B10A2_UNORM;
        case DXGI_FORMAT_R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_FLOAT;
        default: return rt;
        }
    }

    // Pyramid levels are only rebuilt inside the capture region; the downsample clamps its taps to the
    // refreshed region, so the only padding needed is the B-spline footprint at the coarsest level (2 texels).
    constexpr float kPyramidMargin = 64.0f;

    // b2: per-draw constants (edge fade of scroll views), see RenderOp::fade and WgtEdgeFade (wgt_common.hlsli)
    struct DrawConstants
    {
        float fade[4];
    };

    // Blur radii (UI units) the glass shader samples besides its own frost (wgt_fx.hlsl, EvalGlass): the rim's
    // reflection of the surroundings, and the neighbourhood that sets the legibility exposure.
    constexpr float kGlassEnvBlur = 16.0f;
    constexpr float kGlassAmbientBlur = 36.0f;
    // How far outside its edge the rim reads the surroundings it reflects (UI units, from the clamped bezel). Kept
    // short: the reflection is faint and blurred, and every unit here widens the backdrop capture around all glass.
    inline float GlassEnvReach(float bezel) { return std::min(bezel * 0.5f, 16.0f); }
    // The pyramid level the legibility exposure reads (one bilinear read; WgtAmbientLevel in wgt_fx.hlsl).
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

    // One downsample step of a region-limited pyramid build (shared by both backends).
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
            PxRect r{std::floor(region.x0 * k) - 2.0f, std::floor(region.y0 * k) - 2.0f, std::ceil(region.x1 * k) + 2.0f, std::ceil(region.y1 * k) + 2.0f};
            return r.Intersect(PxRect{0, 0, (float)width, (float)height});
        }

        // b1 of DownsamplePS: source texel size + valid source uv rect (inset half a texel, so bilinear
        // taps never touch a stale texel).
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

    inline void FillFrameConstants(FrameConstants& fc, const ImDrawData* dd, int rtW, int rtH, double time, float dt,
                                   bool backdropValid, bool outputLinear)
    {
        const float L = dd->DisplayPos.x, T = dd->DisplayPos.y;
        const float W = dd->DisplaySize.x, H = dd->DisplaySize.y;
        fc.xform[0] = 2.0f / W;
        fc.xform[1] = -2.0f / H;
        fc.xform[2] = -1.0f - 2.0f * L / W;
        fc.xform[3] = 1.0f + 2.0f * T / H;
        fc.target[0] = (float)rtW;
        fc.target[1] = (float)rtH;
        fc.target[2] = 1.0f / (float)rtW;
        fc.target[3] = 1.0f / (float)rtH;
        fc.display[0] = L;
        fc.display[1] = T;
        fc.display[2] = dd->FramebufferScale.x;
        fc.display[3] = dd->FramebufferScale.y;
        fc.time[0] = (float)std::fmod(time, 3600.0);
        fc.time[1] = dt;
        fc.time[2] = backdropValid ? 1.0f : 0.0f;
        fc.time[3] = outputLinear ? 1.0f : 0.0f;
        for (int l = 0; l < kBackdropLevels; ++l)
        {
            const int w = LevelSize(rtW, l), h = LevelSize(rtH, l);
            fc.level[l][0] = (float)w;
            fc.level[l][1] = (float)h;
            fc.level[l][2] = 1.0f / (float)w;
            fc.level[l][3] = 1.0f / (float)h;
        }
    }
}
