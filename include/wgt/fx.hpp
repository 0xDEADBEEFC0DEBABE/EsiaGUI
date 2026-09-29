// WGT UI - FX command stream (the contract between Painter and render backends)
//
// Painter encodes every rich primitive as an ImDrawList callback whose payload is an fx::Command.
// Backends recognise the callback pointer returned by fx::CommandCallback() and render the payload
// with the SDF pipeline instead of calling it. Stock ImGui backends simply call the (no-op) callback,
// so WGT draw lists degrade gracefully instead of crashing.
//
// Layout of fx::Instance must match `FxInst` in src/shaders/wgt_fx.hlsl.
#pragma once
#include "config.hpp"
#include <cstring>

namespace wgt::fx
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
        kCaustic     = 1u << 14,   // light streak (Painter::LightStreak): custom = smile, open, speed, thickness; fill0.a = intensity, fill1.x = clock (< 0: frame clock)
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

    enum class CommandKind : std::uint32_t
    {
        Shape = 1,       // payload: Instance
        LayerBegin = 2,  // payload: LayerParams
        LayerEnd = 3,    // no payload
        FadeBegin = 4,   // payload: FadeParams - the draws that follow in this draw list fade out at the edges
        FadeEnd = 5,     // no payload
    };

    struct LayerParams
    {
        float color[4];    // glow tint rgb, a = tint amount (0 keeps content colors)
        float intensity;   // bloom gain
        float radius;      // bloom radius (px)
        float opacity;     // opacity of the sharp content
        float reserved;
    };

    // Edge fade (scroll views): alpha falls to 0 over `top` / `bottom` towards the edges y0 / y1 (UI units).
    struct FadeParams
    {
        float y0, y1;
        float top, bottom;
    };

    constexpr std::uint32_t kMagic = 0x57475446u;  // 'WGTF'

    struct CommandHeader
    {
        std::uint32_t magic;
        CommandKind kind;
        EffectId effect;          // 0 = built-in shader
        std::uint32_t reserved;
        ImTextureID texture;      // image fill texture (0 = none)
    };

    struct ShapeCommand
    {
        CommandHeader header;
        Instance instance;
    };

    struct LayerCommand
    {
        CommandHeader header;
        LayerParams params;
    };

    struct FadeCommand
    {
        CommandHeader header;
        FadeParams params;
    };

    // The callback pointer identifying WGT commands inside ImDrawList command buffers.
    WGT_API ImDrawCallback CommandCallback();

    // Safe accessors for backends (payload may be unaligned inside the draw list buffer).
    inline bool ReadHeader(const ImDrawCmd& cmd, CommandHeader& out)
    {
        if (cmd.UserCallback != CommandCallback() || cmd.UserCallbackData == nullptr || cmd.UserCallbackDataSize < (int)sizeof(CommandHeader))
            return false;
        std::memcpy(&out, cmd.UserCallbackData, sizeof(CommandHeader));
        return out.magic == kMagic;
    }
    inline bool ReadShape(const ImDrawCmd& cmd, ShapeCommand& out)
    {
        if (cmd.UserCallbackDataSize < (int)sizeof(ShapeCommand))
            return false;
        std::memcpy(&out, cmd.UserCallbackData, sizeof(ShapeCommand));
        return out.header.magic == kMagic && out.header.kind == CommandKind::Shape;
    }
    inline bool ReadFade(const ImDrawCmd& cmd, FadeCommand& out)
    {
        if (cmd.UserCallbackDataSize < (int)sizeof(FadeCommand))
            return false;
        std::memcpy(&out, cmd.UserCallbackData, sizeof(FadeCommand));
        return out.header.magic == kMagic && out.header.kind == CommandKind::FadeBegin;
    }
    inline bool ReadLayer(const ImDrawCmd& cmd, LayerCommand& out)
    {
        if (cmd.UserCallbackDataSize < (int)sizeof(LayerCommand))
            return false;
        std::memcpy(&out, cmd.UserCallbackData, sizeof(LayerCommand));
        return out.header.magic == kMagic && out.header.kind == CommandKind::LayerBegin;
    }
}
