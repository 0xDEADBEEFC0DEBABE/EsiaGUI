// Esia - the FX contract between Painter, the frame planner and the shaders.
//
// Every rich primitive (rounded rect, capsule, circle, arc, segment, liquid merge, light streak) is one
// fx::Instance: 24 x float4 describing the geometry and the full material. It is byte-for-byte the instance of
// WGT's include/wgt/fx.hpp, so Painter code, the planner and the shader math port unchanged; only the transport
// differs: instead of an ImDrawList callback payload, instances live in DrawList::FxInstances() and are
// referenced by DrawCmdKind::Fx commands (core/draw_list.hpp).
//
// On the GPU the instances are not vertex attributes (24 attributes exceed the 16 that GL 3.3, GLES 3, D3D10
// and many Vulkan / D3D9 drivers guarantee). The renderer uploads them as flat float4 arrays - a structured /
// storage buffer or an RGBA32F texture, whichever the backend supports (rhi::Caps::fxStorage) - and the shaders
// fetch them by instance index. Every row is float4 on the GPU: the integer row `flags` is uploaded as float
// values (exact below 2^24), so formats without integer texels work too. Layout must match FxLoad in
// src/esia/shaders/esia_fx.hlsl.
#pragma once
#include "esia/base/config.hpp"

namespace esia::fx
{
    enum Feature : std::uint32_t
    {
        kFill        = 1u << 0,
        kStroke      = 1u << 1,
        kShadow      = 1u << 2,
        kGlow        = 1u << 3,
        kInnerGlow   = 1u << 4,
        kGlass       = 1u << 5,
        kImage       = 1u << 6,
        kShimmer     = 1u << 7,
        kInnerShadow = 1u << 8,
        kMerge       = 1u << 9,
        kMask        = 1u << 10,
        kNoise       = 1u << 11,
        kCustom      = 1u << 12,
        kHalo        = 1u << 13,   // outer glow fades out before `halo` (keeps glows off neighbouring items)
        kCaustic     = 1u << 14,   // light streak: custom = smile, open, speed, thickness; fill0.a = intensity, fill1.x = clock
    };

    enum class ShapeKind : std::uint32_t { RoundRect = 0, Arc = 1, Segment = 2 };
    enum class PaintKind : std::uint32_t { Solid = 0, Linear = 1, Radial = 2, Conic = 3, Spectrum = 4 };

    struct Instance
    {
        float rect[4];          //  0 bounds min.xy max.xy
        float radii[4];         //  1 tl tr br bl | arc: radius, half thickness | segment: half thickness
        float fill0[4];         //  2
        float fill1[4];         //  3
        float fillParams[4];    //  4
        float stroke[4];        //  5
        float strokeParams[4];  //  6 width, align, fade-to, fade angle
        float shadow[4];        //  7
        float shadowParams[4];  //  8 blur, spread, offset.xy
        float glow[4];          //  9
        float glowParams[4];    // 10 outer radius, outer intensity, inner radius, inner intensity
        float glass[4];         // 11 blur, refraction, bezel, dispersion
        float glassTint[4];     // 12
        float glassParams[4];   // 13 saturation, brightness, specular, light angle
        float shape[4];         // 14 glass legibility, smoothing, arc start, arc sweep
        float shape2[4];        // 15 merge rect | segment endpoints
        float shape2Params[4];  // 16 merge radius, merge smoothness, glass magnify
        float misc[4];          // 17 opacity, noise, shimmer intensity, shimmer speed
        float uvRect[4];        // 18
        float mask[4];          // 19
        float maskParams[4];    // 20 radius, smoothing, halo fade width
        float custom[4];        // 21
        float halo[4];          // 22 halo bounds min.xy max.xy (kHalo)
        std::uint32_t flags[4]; // 23 features, paint kind, shape kind, reserved
    };
    static_assert(sizeof(Instance) == 24 * 16, "fx::Instance must stay 24 float4");
    constexpr std::uint32_t kInstanceVec4Count = 24;

    // Glow layer: everything drawn between LayerBegin / LayerEnd gets a bloom halo.
    struct LayerParams
    {
        float color[4];    // glow tint rgb, a = tint amount (0 keeps content colors)
        float intensity;   // bloom gain
        float radius;      // bloom radius (UI units)
        float opacity;     // opacity of the sharp content
        float reserved;
    };

    // Edge fade (scroll views): alpha falls to 0 over `top` / `bottom` towards the edges y0 / y1 (UI units).
    struct FadeParams
    {
        float y0, y1;
        float top, bottom;
    };
}
