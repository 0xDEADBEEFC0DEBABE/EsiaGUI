// Esia - the FX contract between Painter, the frame planner and the shaders.
//
// Every rich primitive (rounded rect, capsule, circle, arc, segment, liquid merge, light streak) is one
// fx::Instance: 24 x float4 describing the geometry and the full material. It is byte-for-byte the instance of
// WGT's include/wgt/fx.hpp, so Painter code, the planner and the shader math port unchanged; only the transport
// differs: instead of an ImDrawList callback payload, instances live in DrawList::FxInstances() and are
// referenced by DrawCmdKind::Fx commands (core/draw_list.hpp). A path (any polygon or polyline, Esia's own) is one
// instance too, with its points in data instances right after it (ShapeKind::Path).
//
// On the GPU the instances are not vertex attributes (24 attributes exceed the 16 that GL 3.3, GLES 3, D3D10
// and many Vulkan / D3D9 drivers guarantee). The renderer uploads them as flat float4 arrays - a structured /
// storage buffer or an RGBA32F texture, whichever the backend supports (rhi::Caps::fxStorage) - and the shaders
// fetch them by instance index. Every row is float4 on the GPU: the integer row `flags` is uploaded as float
// values (exact below 2^24), so formats without integer texels work too. Layout must match the FX_ROW_* indices
// in src/esia/shaders/esia_fx.hlsl.
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
        // a glyph's distance field (with ShapeKind::Glyph): the glyph code of the shaders is a feature, so variants of
        // other batches leave it out (and SM3's full shader, which has no room for it)
        kGlyph       = 1u << 15,
        // glyph shapes: where the backdrop under the text is too close to its fill in lightness, a soft halo of the
        // other tone (radii.z strength 0 .. 1, radii.w radius in UI units) - none where the contrast is ample. Reads
        // the captured backdrop, as glass does.
        kLegible     = 1u << 16,
        // motion blur: the shape moves custom.xy (UI units) while the frame is shown - its edges are blurred along it,
        // in closed form (the coverage ramp averaged over the distance the edge sweeps across each pixel). The custom
        // row is a user effect's otherwise: an effect's shape is not blurred.
        kMotion      = 1u << 17,
        // a path (with ShapeKind::Path): a polygon or a polyline through points carried by the instances after it
        // (ShapeKind::PathData), its exact distance taken in the shader - every material on any outline
        kPath        = 1u << 18,
    };

    // Features the full FX shader leaves out where rhi::Caps::fullFxGlyphs is false (every backend but Metal without
    // function constants): their code would cost every FX pixel registers and instructions for the few batches that
    // use them - and SM3 has no room for it. Those batches draw with their feature variant, built at once.
    constexpr std::uint32_t kVariantOnlyFeatures = kGlyph | kLegible | kMotion | kPath;
    // The features of the full FX shader where it leaves those out (the mask backends specialize it to).
    constexpr std::uint32_t kFullShaderFeatures = ~kVariantOnlyFeatures;

    // Glyph: a glyph's distance field (MTSDF, text/distance_field.hpp) on the instance's texture - shape2 = the field's
    // extent (UI units: the quad), uvRect = its texels, radii.x / y = the multi-channel / true distance ranges (field
    // pixels), shape.z = field pixels per UI unit, shape.w = weight (UI units the glyph grows by), rect = the whole
    // text's box (what paints, glass and shimmer span); with the feature kGlyph. TextSystem::DrawShapes makes them;
    // every material row applies (but an image fill: uvRect is the field's).
    //
    // Path: the outline through n points (with the feature kPath) - closed (a polygon, any simple outline: convex or
    // not, either winding) or open (a polyline: its distance less half its thickness, so round joins and caps). rect =
    // the shape's bounds (what paints, glass and the quad span), radii.x = what the distance is lessened by (closed: the
    // rounding the points were inset by - convex corners come out as circular arcs; open: half the thickness), radii.y
    // = 0 (reserved), radii.z = n, radii.w = 1 open / 0 closed. Glass's bevel (glass.z) comes no deeper than the shape
    // goes (Painter clamps it, as for arcs and segments): a thin shape is one rounded rod, without a crease along its
    // middle. The points follow in ceil(n / kPathPointsPerData) instances of
    // ShapeKind::PathData, right after the path in its list: two points per row (x0, y0, x1, y1) in rows 0 ..
    // kPathPointsPerData / 2 - 1, the flags row marking the instance as data (its features 0). They are drawn with the
    // path (an empty quad each) and never on their own: the planner keeps them behind it in the same batch.
    enum class ShapeKind : std::uint32_t { RoundRect = 0, Arc = 1, Segment = 2, Glyph = 3, Path = 4, PathData = 5 };
    // A power of two: SM3 finds a point's row with float arithmetic, exact only then (esia_fx.hlsl, PathRow).
    constexpr std::uint32_t kPathPointsPerData = 32;
    // The most points one path carries (Painter thins longer outlines out).
    constexpr std::uint32_t kPathMaxPoints = 4096;
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
