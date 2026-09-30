// Esia - FX shape renderer (program Fx: FxVS + FxPS, 4-vertex triangle-strip quads, instanced).
// Ported unchanged from WGT's src/shaders/wgt_fx.hlsl except for how instances arrive (the hot rows as flat varyings,
// the others fetched where they are read: FxPixelRows, FX_FETCH) and the pixel position (WgtPixelPos: top-left pixels
// on every API).
//
// Every WGT shape (rounded rect, capsule, circle, arc, segment, liquid merge) is one GPU instance.
// The instance carries the full material: paint, stroke, drop/inner shadow, outer/inner glow,
// liquid glass (backdrop blur + refraction + chromatic dispersion + specular rim), image fill,
// shimmer, noise and a rounded mask. Layout must match esia::fx::Instance in include/esia/core/fx.hpp.
#include "esia_common.hlsli"

// Feature bits (esia::fx::Feature), as plain literals: shader models without integer shifts fold them too.
// ESIA_FX_FEATURES compiles a variant for a subset of them (rhi::Caps::fxFeatureVariants): a feature outside the
// mask is a constant false and its code is gone - how Direct3D 9 fits the shader into SM3's instruction slots.
#ifndef ESIA_FX_FEATURES
#define ESIA_FX_FEATURES 0xFFFFFFFFu
#endif
#define FX_HAS(feat, flag) ESIA_HAS(feat, (flag) & ESIA_FX_FEATURES)
#define F_FILL         1u
#define F_STROKE       2u
#define F_SHADOW       4u
#define F_GLOW         8u
#define F_INNER_GLOW   16u
#define F_GLASS        32u
#define F_IMAGE        64u
#define F_SHIMMER      128u
#define F_INNER_SHADOW 256u
#define F_MERGE        512u
#define F_MASK         1024u
#define F_NOISE        2048u
#define F_CUSTOM       4096u
#define F_HALO         8192u
#define F_CAUSTIC      16384u

// Shape kinds (esia::fx::ShapeKind) and paint kinds (esia::fx::PaintKind), compared with the flags row: plain
// literals take the type of what they are compared with (a uint on SM4+, the float SM3 emulates it with), so
// neither model warns about a signed / unsigned mismatch
#define SHAPE_RRECT    0
#define SHAPE_ARC      1
#define SHAPE_SEGMENT  2

#define PAINT_SOLID    0
#define PAINT_LINEAR   1
#define PAINT_RADIAL   2
#define PAINT_CONIC    3
#define PAINT_SPECTRUM 4

struct FxInst
{
    float4 rect;         //  0 shape bounds (min.xy, max.xy) in UI units
    float4 radii;        //  1 corner radii tl,tr,br,bl | arc: radius, half thickness | segment: half thickness
    float4 fill0;        //  2 paint color A (straight alpha)
    float4 fill1;        //  3 paint color B
    float4 fillParams;   //  4 linear: x=angle | radial: xy=center(0..1) z=radius(0..1) | conic: xy=center z=angle w=loop
    float4 stroke;       //  5 stroke color
    float4 strokeParams; //  6 x=width y=align(0 in,0.5 center,1 out) z=fade-to alpha w=fade angle
    float4 shadow;       //  7 shadow color
    float4 shadowParams; //  8 x=blur y=spread zw=offset
    float4 glow;         //  9 glow color
    float4 glowParams;   // 10 x=outer radius y=outer intensity z=inner radius w=inner intensity
    float4 glass;        // 11 x=blur y=refraction z=bezel w=dispersion
    float4 glassTint;    // 12 rgb tint, a=amount
    float4 glassParams;  // 13 x=saturation y=brightness z=specular w=light angle
    float4 shape;        // 14 x=glass legibility y=corner smoothing | arc: z=start angle w=sweep
    float4 shape2;       // 15 merge: second rect | segment: a.xy b.zw
    float4 shape2Params; // 16 merge: x=radius y=smoothness
    float4 misc;         // 17 x=opacity y=noise z=shimmer intensity w=shimmer speed
    float4 uvRect;       // 18 image uv min.xy max.xy
    float4 mask;         // 19 mask rect
    float4 maskParams;   // 20 x=radius y=smoothing z=halo fade width
    float4 custom;       // 21 user effect parameters
    float4 halo;         // 22 halo bounds: the outer glow fades out before reaching them
    uint4  flags;        // 23 x=features y=paint kind z=shape kind w=reserved
    uint   index;        // the instance, where FX_FETCH reads the rows the pixel shader was not given
};

#define FX_ROW_rect          0u
#define FX_ROW_radii         1u
#define FX_ROW_fill0         2u
#define FX_ROW_fill1         3u
#define FX_ROW_fillParams    4u
#define FX_ROW_stroke        5u
#define FX_ROW_strokeParams  6u
#define FX_ROW_shadow        7u
#define FX_ROW_shadowParams  8u
#define FX_ROW_glow          9u
#define FX_ROW_glowParams   10u
#define FX_ROW_glass        11u
#define FX_ROW_glassTint    12u
#define FX_ROW_glassParams  13u
#define FX_ROW_shape        14u
#define FX_ROW_shape2       15u
#define FX_ROW_shape2Params 16u
#define FX_ROW_misc         17u
#define FX_ROW_uvRect       18u
#define FX_ROW_mask         19u
#define FX_ROW_maskParams   20u
#define FX_ROW_custom       21u
#define FX_ROW_halo         22u
#define FX_ROW_flags        23u

// The hot rows are what every pixel of every shape reads (bounds, radii, fill color, shape parameters, opacity and
// the flags): the vertex shader fetches them and hands them to the pixel shader as flat varyings, the way WGT
// passed its instance - 8 interpolators in all, within every target's budget (GL 3.3 / GLES 3 / D3D10: 15 - 16, SM3:
// 10). The cold rows are fetched by the branches that read them (FX_FETCH), so a plain rounded rect reads none.
// ESIA_FX_FETCH_ALL restores the previous path - all 24 rows fetched at the start of every pixel - for A / B timing.
#define FX_HOT_ROWS(X) X(rect) X(radii) X(fill0) X(shape) X(misc)
#define FX_COLD_ROWS(X) \
    X(fill1) X(fillParams) X(stroke) X(strokeParams) X(shadow) X(shadowParams) X(glow) X(glowParams) X(glass) \
    X(glassTint) X(glassParams) X(shape2) X(shape2Params) X(uvRect) X(mask) X(maskParams) X(custom) X(halo)
#ifdef ESIA_FX_FETCH_ALL
#define FX_FETCH(I, name)
#else
#define FX_FETCH(I, name) I.name = FxFetch(I.index, FX_ROW_##name);
#endif

// Instances are not vertex attributes (24 would exceed the 16 that GL 3.3 / GLES 3 / D3D10 guarantee): the
// stages fetch the rows they need from the instance data by instance index. Row 23 (flags) holds the integer
// flags as float values (exact below 2^24).
#ifdef ESIA_FX_STORAGE_TEXTURE
ESIA_TEXTURE(gFxData, t7, 10);
float4 FxFetch(uint instance, uint field)
{
    // gConv.z instances per row, 24 texels each
    const uint perRow = max((uint)gConv.z, 1u);
    return ESIA_LOAD(gFxData, int2((int)((instance % perRow) * 24u + field), (int)(instance / perRow)));
}
#else
ESIA_BINDING(10) StructuredBuffer<float4> gFxData : register(t7);
float4 FxFetch(uint instance, uint field) { return gFxData[instance * 24u + field]; }
#endif

struct FxVSIn
{
    ESIA_VERTEX_ID(vid);
    ESIA_INSTANCE_ID(iid);
};

struct FxPSIn
{
    float4 pos : SV_Position;
    float2 local : TEXCOORD0;
    ESIA_FLAT uint instance : TEXCOORD1;
#ifndef ESIA_FX_FETCH_ALL
    ESIA_FLAT float4 rect : TEXCOORD2;    // the hot rows
    ESIA_FLAT float4 radii : TEXCOORD3;
    ESIA_FLAT float4 fill0 : TEXCOORD4;
    ESIA_FLAT float4 shape : TEXCOORD5;
    ESIA_FLAT float4 misc : TEXCOORD6;
    ESIA_FLAT uint4 flags : TEXCOORD7;
#endif
};

// The instance as the pixel shader starts with it: the hot rows from the vertex shader, the cold ones zero until a
// branch fetches them (or every row, with ESIA_FX_FETCH_ALL).
FxInst FxPixelRows(FxPSIn i)
{
    FxInst I;
    I.index = ESIA_FLAT_UINT(i.instance);
#ifdef ESIA_FX_FETCH_ALL
#define X(name) I.name = FxFetch(I.index, FX_ROW_##name);
    FX_HOT_ROWS(X)
    FX_COLD_ROWS(X)
#undef X
    I.flags = (uint4)FxFetch(I.index, FX_ROW_flags);
#else
#define X(name) I.name = i.name;
    FX_HOT_ROWS(X)
#undef X
#define X(name) I.name = float4(0.0, 0.0, 0.0, 0.0);
    FX_COLD_ROWS(X)
#undef X
    I.flags = (uint4)ESIA_FLAT_UINT(i.flags);
#endif
    return I;
}

FxPSIn FxVS(FxVSIn v)
{
    FxPSIn o;
    // instance ids start at 0 in every draw on every API: the draw's first instance comes in the constants
    const uint instance = v.iid + (uint)gDrawInfo.x;
    o.instance = instance;
    const float4 rect = FxFetch(instance, FX_ROW_rect);
    const float4 strokeParams = FxFetch(instance, FX_ROW_strokeParams);
    const float4 shadowParams = FxFetch(instance, FX_ROW_shadowParams);
    const float4 glowParams = FxFetch(instance, FX_ROW_glowParams);
    const float4 shape2 = FxFetch(instance, FX_ROW_shape2);
    const uint4 flags = (uint4)FxFetch(instance, FX_ROW_flags);
    const uint feat = flags.x;

    float2 mn = rect.xy, mx = rect.zw;
    if FX_HAS(feat, F_MERGE)
    {
        mn = min(mn, shape2.xy);
        mx = max(mx, shape2.zw);
    }
    float pad = 2.0;
    if FX_HAS(feat, F_STROKE)
        pad = max(pad, strokeParams.x * strokeParams.y + 2.0);
    if (FX_HAS(feat, F_SHADOW) && !FX_HAS(feat, F_INNER_SHADOW))
        pad = max(pad, shadowParams.x * 1.6 + max(shadowParams.y, 0.0) + max(abs(shadowParams.z), abs(shadowParams.w)) + 2.0);
    if FX_HAS(feat, F_GLOW)
        pad = max(pad, glowParams.x * 1.8 + 2.0);
    mn -= pad;
    mx += pad;
    if FX_HAS(feat, F_MASK)
    {
        const float4 mask = FxFetch(instance, FX_ROW_mask);
        mn = max(mn, mask.xy - 1.0);
        mx = min(mx, mask.zw + 1.0);
    }
    // the halo bounds everything outside the shape except a drop shadow: skip the dead area
    if (FX_HAS(feat, F_HALO) && !(FX_HAS(feat, F_SHADOW) && !FX_HAS(feat, F_INNER_SHADOW)))
    {
        const float4 halo = FxFetch(instance, FX_ROW_halo);
        mn = max(mn, halo.xy - 1.0);
        mx = min(mx, halo.zw + 1.0);
    }
    float2 c = float2(ESIA_HAS(v.vid, 1u) ? mx.x : mn.x, ESIA_HAS(v.vid, 2u) ? mx.y : mn.y);
    o.pos = ESIA_CLIP_POSITION(float4(c * gXform.xy + gXform.zw, 0.0, 1.0));
    o.local = c;
#ifndef ESIA_FX_FETCH_ALL
    o.rect = rect;
    o.radii = FxFetch(instance, FX_ROW_radii);
    o.fill0 = FxFetch(instance, FX_ROW_fill0);
    o.shape = FxFetch(instance, FX_ROW_shape);
    o.misc = FxFetch(instance, FX_ROW_misc);
    o.flags = flags;
#endif
    return o;
}

// ------------------------------------------------------------------ SDFs ----

// Rounded box with per-corner radii and optional "continuous" (squircle-like) corners.
float SdRoundRect(float2 p, float2 halfSize, float4 radii, float smoothing)
{
    float r = (p.x > 0.0) ? ((p.y > 0.0) ? radii.z : radii.y) : ((p.y > 0.0) ? radii.w : radii.x);
    float maxR = min(halfSize.x, halfSize.y);
    r = min(r, maxR);
    float s = smoothing;
    float R = r * (1.0 + 0.6 * s);
    if (R > maxR)
    {
        s *= saturate((maxR - r) / max(0.6 * r, 1e-4));
        R = r * (1.0 + 0.6 * s);
    }
    float2 q = abs(p) - halfSize + R;
    float2 m = max(q, 0.0);
    float corner = 0.0;
    [branch] if (m.x > 0.0 && m.y > 0.0)   // only the corner quadrants need the superellipse
    {
        // (a sharp corner - R under the 1e-4 floor below - has no curve to normalize by: its distance is the
        // Euclidean one; normalizing by the floor instead collapsed it to 0 and put a half-covered pixel outside it)
        if (s > 0.001 && R > 1e-4)
        {
            float n = 2.0 + 2.0 * s;
            float2 mn = m / max(R, 1e-4);
            corner = pow(pow(mn.x, n) + pow(mn.y, n), 1.0 / n) * R;
        }
        else
            corner = length(m);
    }
    else
        corner = max(m.x, m.y);
    return min(max(q.x, q.y), 0.0) + corner - R;
}

float SdArc(float2 p, float radius, float halfThickness, float start, float sweep)
{
    if (sweep >= WGT_TAU - 1e-3)
        return abs(length(p) - radius) - halfThickness;
    float mid = start + sweep * 0.5;
    float2 dir = float2(cos(mid), sin(mid));
    // rotate so that the arc's middle points to +y
    float2 q = float2(dot(p, float2(-dir.y, dir.x)), dot(p, dir));
    q.x = abs(q.x);
    float half_ = sweep * 0.5;
    float2 sc = float2(sin(half_), cos(half_));
    return ((sc.y * q.x > sc.x * q.y) ? length(q - sc * radius) : abs(length(q) - radius)) - halfThickness;
}

float SdSegment(float2 p, float2 a, float2 b, float r)
{
    float2 pa = p - a, ba = b - a;
    float h = saturate(dot(pa, ba) / max(dot(ba, ba), 1e-6));
    return length(pa - ba * h) - r;
}

float SminPoly(float a, float b, float k)
{
    k = max(k, 1e-4);
    float h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * k * 0.25;
}

float ShapeSD(float2 p, FxInst I)
{
    uint kind = I.flags.z;
    float2 center = (I.rect.xy + I.rect.zw) * 0.5;
    float2 halfSize = max((I.rect.zw - I.rect.xy) * 0.5, 1e-3);
    float d;
    [branch] if (kind == SHAPE_ARC)
    {
        d = SdArc(p - center, I.radii.x, I.radii.y, I.shape.z, I.shape.w);
    }
    else if (kind == SHAPE_SEGMENT)
    {
        d = SdSegment(p, I.shape2.xy, I.shape2.zw, I.radii.x);
    }
    else
    {
        d = SdRoundRect(p - center, halfSize, I.radii, I.shape.y);
        [branch] if (FX_HAS(I.flags.x, F_MERGE))
        {
            float2 c2 = (I.shape2.xy + I.shape2.zw) * 0.5;
            float2 h2 = max((I.shape2.zw - I.shape2.xy) * 0.5, 1e-3);
            float d2 = SdRoundRect(p - c2, h2, I.shape2Params.xxxx, I.shape.y);
            d = SminPoly(d, d2, I.shape2Params.y);
        }
    }
    return d;
}

float2 ShapeNormal(float2 p, FxInst I)
{
    const float e = 0.5;
    float dx = ShapeSD(p + float2(e, 0), I) - ShapeSD(p - float2(e, 0), I);
    float dy = ShapeSD(p + float2(0, e), I) - ShapeSD(p - float2(0, e), I);
    float2 g = float2(dx, dy);
    float l = length(g);
    return l > 1e-5 ? g / l : float2(0, -1);
}

// ---------------------------------------------------------------- paint ----

float4 Premul(float4 c, float a)
{
    float alpha = saturate(c.a * a);
    return float4(c.rgb * alpha, alpha);
}

float4 Over(float4 top, float4 bottom)
{
    return top + bottom * (1.0 - top.a);
}

float4 EvalPaint(float2 p, FxInst I)
{
    uint kind = I.flags.y;
    if (kind == PAINT_SOLID)
        return I.fill0;
    float2 size = max(I.rect.zw - I.rect.xy, 1e-3);
    float2 center = (I.rect.xy + I.rect.zw) * 0.5;
    const float pxw = max(length(fwidth(p)), 1e-4);   // before any branch: derivatives stay defined
    float t = 0.0;
    float seam = -1.0;   // conic sweep: blend weight across the seam (< 0 = not near it)
    if (kind == PAINT_LINEAR || kind == PAINT_SPECTRUM)
    {
        float2 dir = float2(cos(I.fillParams.x), sin(I.fillParams.x));
        float ext = abs(dir.x) * size.x * 0.5 + abs(dir.y) * size.y * 0.5;
        t = dot(p - center, dir) / max(ext, 1e-3) * 0.5 + 0.5;
        if (kind == PAINT_SPECTRUM)
        {
            // red .. violet between `from` and `to`, fading out at both ends
            const float s = saturate((t - I.fillParams.y) / max(I.fillParams.z - I.fillParams.y, 1e-3));
            const float3 rgb = saturate(abs(frac(s * 0.8 + float3(1.0, 0.6667, 0.3333)) * 6.0 - 3.0) - 1.0);
            return float4(rgb, I.fill0.a * pow(max(sin(s * WGT_PI), 0.0), 0.6));
        }
    }
    else if (kind == PAINT_RADIAL)
    {
        float2 c = I.rect.xy + I.fillParams.xy * size;
        t = length(p - c) / max(I.fillParams.z * max(size.x, size.y), 1e-3);
    }
    else
    {
        float2 c = I.rect.xy + I.fillParams.xy * size;
        float2 q = p - c;
        t = frac((atan2(q.y, q.x) - I.fillParams.z) / WGT_TAU + 1.0);
        if (I.fillParams.w > 0.5)
            t = 0.5 - 0.5 * cos(t * WGT_TAU);   // loop: a -> b -> a, no seam
        else
        {
            // sweep a -> b: where it wraps (b next to a), blend over one pixel instead of a jagged edge
            const float arc = (t < 0.5 ? t : t - 1.0) * WGT_TAU * length(q);   // signed distance along the circle
            if (abs(arc) < pxw)
                seam = saturate(arc / pxw * 0.5 + 0.5);
        }
    }
    t = saturate(t);
    // interpolate in premultiplied space to avoid dark fringes between colors of different alpha
    float4 a = float4(I.fill0.rgb * I.fill0.a, I.fill0.a);
    float4 b = float4(I.fill1.rgb * I.fill1.a, I.fill1.a);
    float4 c = seam >= 0.0 ? lerp(b, a, seam) : lerp(a, b, t);
    return c.a > 1e-5 ? float4(c.rgb / c.a, c.a) : float4(0, 0, 0, 0);
}

// ---------------------------------------------------------------- glass ----
// Liquid glass: a clear slab whose edge is rounded with radius `bezel`.
//   * interior: the content behind comes straight through - a light frost, a little richer (vibrancy);
//   * edge: the surface curves down over `bezel` from the flat top to the rim (circle profile). A small
//     control or bar is one rounded rod / dome - the bezel is clamped to half its size - like iOS glass.
//     The content behind is pulled towards the center by the sag of that curve, up to `refraction` at the
//     rim: magnified towards the edge, folded into a thin mirrored band right at the rim, colors separating
//     slightly (dispersion); the steep rim reflects the surroundings (Fresnel);
//   * light: a highlight on the rim that faces the light, a weaker bounce on the opposite side;
//   * legibility: the overall brightness of what is behind (a wide neighbourhood) moves towards the tint's,
//     like an exposure change - detail and color survive, so the glass stays transparent.
// Keep in sync with the frame planner (src/esia/render/frame_plan.cpp): the pyramid must reach these blur radii.
static const float kGlassEnvBlur = 16.0;       // UI units: blur of the surroundings the rim reflects
static const float kGlassAmbientBlur = 36.0;   // UI units: neighbourhood that sets the legibility exposure

// The legibility exposure is a wide, smooth average: one bilinear read of the pyramid level closest to its blur
// (at most level 4, so a capture never has to build level 5 for it). Must match AmbientLevel() in src/esia/render/gpu_constants.hpp.
int WgtAmbientLevel(float rs) { return clamp((int)round(log2(kGlassAmbientBlur * rs) - 1.0), 1, 4); }

struct GlassSample
{
    float3 color;
    float  rim;
};

float Luma(float3 c) { return dot(c, float3(0.2126, 0.7152, 0.0722)); }

// A soft ribbon of white light split into a smooth spectrum the way glass disperses it: a warm edge above, then
// green, then periwinkle and lavender below; white where the colors overlap. `spread` = how far they fan out.
static const float3 kLightSpectrum[6] = {
    float3(1.00, 0.42, 0.22), float3(1.00, 0.70, 0.30), float3(0.80, 0.92, 0.40),
    float3(0.35, 0.90, 0.70), float3(0.40, 0.62, 1.00), float3(0.66, 0.50, 1.00) };
float3 DispersedRibbon(float y, float yc, float sigma, float spread)
{
    float3 c = float3(0.0, 0.0, 0.0);
    [unroll] for (int k = 0; k < 6; ++k)
    {
        const float d = (y - yc - spread * ((float)k * 0.4 - 1.0)) / sigma;
        c += kLightSpectrum[k] * exp(-d * d);
    }
    return c / float3(4.21, 4.06, 3.62);   // the taps add up to white
}

// The bevel's Fresnel term `inside` UI units within the outline (0 outside it): (1 - N.z)^5 of the circle profile,
// as EvalGlass has it at the pixel itself. On a circle profile N.z is the height h (the wall's slope u / h capped at
// 8: N.z >= 0.124), and the fifth power is three multiplies.
float BevelFresnel(float inside, float bezel)
{
    const float u = saturate(1.0 - inside / bezel);
    const float f = 1.0 - max(sqrt(saturate(1.0 - u * u)), 0.124);
    const float f2 = f * f;
    return inside < 0.0 ? 0.0 : f2 * f2 * f;
}

// px: UI units per pixel
GlassSample EvalGlass(float2 svpos, float2 p, float d, float px, FxInst I)
{
    GlassSample gs;
    const float rs = gDisplay.z;                 // UI units -> render-target pixels (render scale)
    const float blur = I.glass.x * rs;
    const float lens = I.glass.y;                // UI units: how far the content is pulled in at the rim
    const float2 halfSize = max((I.rect.zw - I.rect.xy) * 0.5, 1.0);
    const float bezel = clamp(I.glass.z, 1e-3, min(halfSize.x, halfSize.y));   // small shapes: one rod / dome
    const float dispersion = I.glass.w;

    // Surface: circle profile. u = 1 at the rim .. 0 where the flat top begins (the center line of a rod).
    const float u = saturate(1.0 - max(-d, 0.0) / bezel);
    const float h = sqrt(saturate(1.0 - u * u));  // surface height: 1 on the flat top .. 0 at the rim
    float2 n = float2(0.0, -1.0);                 // outward normal of the outline
    float3 N = float3(0.0, 0.0, 1.0);             // surface normal (flat top: straight up)
    [branch] if (u > 0.0)
    {
        n = ShapeNormal(p, I);
        const float slope = min(u / max(h, 1e-3), 8.0);   // the wall is vertical at the rim
        N = normalize(float3(n * slope, 1.0));
    }
    // pulled inwards by the sag of the curve: 0 on the flat top, `lens` at the rim. Where the pull grows
    // faster than the distance to the rim shrinks, the image folds over: the thin mirrored edge of real glass.
    const float2 rimOff = -n * lens * (1.0 - h);
    // loupe: everything under the glass enlarged around its center
    float2 zoomOff = float2(0.0, 0.0);
    const float magnify = I.shape2Params.z;
    [branch] if (magnify > 0.0)
        zoomOff = ((I.rect.xy + I.rect.zw) * 0.5 - p) * (magnify / (1.0 + magnify));
    const float2 uv = svpos * gTarget.zw;
    const float2 toUv = gDisplay.zw * gTarget.zw;
    const float2 duv = (rimOff + zoomOff) * toUv;
    const float2 rimUv = rimOff * toUv;   // dispersion follows the rim's bending only

    float3 col = float3(0.5, 0.5, 0.5);
    float3 env = float3(0.5, 0.5, 0.5);
    float amb = 0.5;
    if (gTime.z > 0.5)
    {
        col = WgtSampleBackdrop(uv + duv, blur);
        // rainbow fringes narrower than the frost cannot be seen: frosted glass skips the extra reads
        [branch] if (dispersion > 0.001 && u > 0.0 && lens * 0.3 * dispersion * rs > blur * 0.35)
        {
            // shorter wavelengths bend more (green is where the plain read looked)
            const float k = 0.3 * dispersion;
            col.r = WgtSampleBackdrop(uv + duv - rimUv * k, blur).r;
            col.b = WgtSampleBackdrop(uv + duv + rimUv * k, blur).b;
        }
        [branch] if (u > 0.0)
            env = WgtSampleBackdropSoft(uv + n * min(bezel * 0.5, 16.0) * gDisplay.zw * gTarget.zw, kGlassEnvBlur * rs);   // GlassEnvReach
        [branch] if (I.shape.x > 0.001)
            amb = Luma(WgtSampleLevelBilinear(WgtAmbientLevel(rs), uv).rgb);
    }

    // vibrancy: richer color, a touch of light
    col = lerp(Luma(col).xxx, col, I.glassParams.x) + I.glassParams.y;
    // legibility: expose the whole neighbourhood towards the tint's brightness
    [branch] if (I.shape.x > 0.001)
    {
        const float a = saturate(max(amb + I.glassParams.y, 1e-3));
        const float a2 = lerp(a, Luma(I.glassTint.rgb), I.shape.x);
        // darken by scaling (keeps hue); brighten by mixing in white (scaling would clip into cyan/magenta)
        col = a2 < a ? col * (a2 / a) : lerp(col, 1.0.xxx, (a2 - a) / max(1.0 - a, 1e-3));
    }
    // Veil, layered like iOS frost rather than a flat coat: heavier on the curved bevel, and a light veil gathers
    // towards the lit top, a dark one towards the bottom - the glass reads as a thick, lit slab
    const float2 lp = saturate((p - I.rect.xy) / max(I.rect.zw - I.rect.xy, 1e-3));
    const float vert = Luma(I.glassTint.rgb) > 0.5 ? 1.0 - lp.y : lp.y;
    col = lerp(col, I.glassTint.rgb, saturate(I.glassTint.a * (0.72 + 0.55 * (1.0 - h) + 0.35 * vert)));

    // Fresnel (Schlick's angular term): the steep outer rim mirrors its surroundings. The flat top's constant
    // 4% is left out - it would only veil the glass, and must not start at the edge of the bevel (a visible line).
    const float fres = pow(1.0 - N.z, 5.0);
    col = lerp(col, env * 1.1 + 0.08, fres * 0.35);

    // light catches the rim facing it, and bounces weaker on the opposite side
    const float2 L = float2(cos(I.glassParams.w), sin(I.glassParams.w));
    const float facing = dot(n, L);
    const float lit = pow(saturate(facing), 1.5) + 0.4 * pow(saturate(-facing), 1.5);
    // The Fresnel term peaks within a fraction of a pixel of the outline: sampled at the pixel center, the rim line
    // flickered along a curve (brighter and darker from pixel to pixel, its peak jumping in and out, so a round
    // button looked dented). It is filtered across the outline with a tent 1.6 px wide; taps outside the shape
    // count 0, so the rim carries its own coverage (applied once, in FxPS).
    const float pxn = px * (abs(n.x) + abs(n.y));   // the pixel's width across the outline
    float rimFresnel = 0.0;
    [unroll] for (int k = -2; k <= 2; ++k)
        rimFresnel += (3.0 - abs((float)k)) * BevelFresnel(-d + (float)k * 0.4 * pxn, bezel);
    gs.rim = rimFresnel / 9.0 * lit * I.glassParams.z * 1.6;
    // light: the side of the slab facing it and the whole bevel glow a little (clear glass too)
    const float2 center = (I.rect.xy + I.rect.zw) * 0.5;
    const float toward = saturate(0.5 + 0.5 * dot((p - center) / halfSize, L));
    col += (0.04 * toward * toward + 0.05 * (1.0 - h)) * I.glassParams.z;
    gs.color = col;
    return gs;
}

// ---------------------------------------------------------------- effect ----

struct WgtFx
{
    float2 pos;       // UI-space position
    float2 uv;        // 0..1 inside the shape bounds
    float2 size;      // shape bounds size
    float2 screenUV;  // 0..1 inside the render target
    float  sd;        // signed distance to the shape (negative inside)
    float  coverage;  // anti-aliased coverage
    float  px;        // UI units per pixel
    float  time;      // seconds
    float4 params;    // Style::Effect(id, p0, p1, p2, p3)
    float4 fill;      // evaluated fill (straight alpha)
};

#ifdef ESIA_CUSTOM_EFFECT
float3 WgtBackdrop(float2 screenUV, float blurPx) { return gTime.z > 0.5 ? WgtSampleBackdrop(screenUV, blurPx) : float3(0.5, 0.5, 0.5); }
float4 WgtTexture(float2 uv) { return ESIA_SAMPLE(gTex, gLinear, uv); }
#include "esia_user_effect.hlsli"   // must define: float4 WgtEffect(WgtFx fx)  -> premultiplied color
#endif

// ------------------------------------------------------------------ main ----

float4 FxPS(FxPSIn i) : SV_Target
{
    FxInst I = FxPixelRows(i);
    uint feat = I.flags.x;
    const float2 spos = WgtPixelPos(i.pos);
    float2 p = i.local;
    float px = max(abs(ddx(p.x)) + abs(ddy(p.x)), 1e-4);

    // the second shape of a liquid merge, the end points of a segment
    [branch] if (I.flags.z == SHAPE_SEGMENT || FX_HAS(feat, F_MERGE))
    {
        FX_FETCH(I, shape2)
        FX_FETCH(I, shape2Params)
    }
    float d = ShapeSD(p, I);
    float cov = saturate(0.5 - d / px);

    float maskCov = 1.0;
    [branch] if FX_HAS(feat, F_MASK)
    {
        FX_FETCH(I, mask)
        FX_FETCH(I, maskParams)
        float2 mc = (I.mask.xy + I.mask.zw) * 0.5;
        float2 mh = max((I.mask.zw - I.mask.xy) * 0.5, 1e-3);
        float dm = SdRoundRect(p - mc, mh, I.maskParams.xxxx, I.maskParams.y);
        maskCov = saturate(0.5 - dm / px);
        if (maskCov <= 0.0)
            discard;
    }

    float4 acc = float4(0, 0, 0, 0);
    // inside an opaque glass body the drop shadow and outer glow are fully covered: skip them
    const bool underOpaqueGlass = FX_HAS(feat, F_GLASS) && cov >= 0.999;

    // 1. drop shadow
    [branch] if (FX_HAS(feat, F_SHADOW) && !FX_HAS(feat, F_INNER_SHADOW) && !underOpaqueGlass)
    {
        FX_FETCH(I, shadow)
        FX_FETCH(I, shadowParams)
        float sigma = max(I.shadowParams.x * 0.5, px * 0.5);
        float ds = ShapeSD(p - I.shadowParams.zw, I) - I.shadowParams.y;
        float a = 0.5 - 0.5 * WgtErf(ds / (sigma * 1.41421356));
        acc = Over(Premul(I.shadow, a), acc);
    }

    // 2. outer glow
    [branch] if (FX_HAS(feat, F_GLOW) && !underOpaqueGlass)
    {
        FX_FETCH(I, glow)
        FX_FETCH(I, glowParams)
        float r = max(I.glowParams.x, 1e-3);
        float edgeFade = 1.0;
        [branch] if FX_HAS(feat, F_HALO)
        {
            FX_FETCH(I, halo)
            // Neighbours in reach (halo bounds): the glow keeps its gaussian profile but its radius shrinks
            // towards each constrained side, so it has died out before the neighbour instead of being cut off.
            const float2 c = (I.rect.xy + I.rect.zw) * 0.5;
            const float2 hs = max((I.rect.zw - I.rect.xy) * 0.5, 1e-3);
            const float2 lo = I.rect.xy - I.halo.xy, hi = I.halo.zw - I.rect.zw;   // room left / top, right / bottom
            const float2 room = float2(p.x < c.x ? lo.x : hi.x, p.y < c.y ? lo.y : hi.y);
            const float2 rr = clamp(room / 1.7, min(r, 1.5), r);                   // radius along x / y
            // direction of the shape's outward normal: measured from its core (the rect minus the corner
            // radius), so it turns smoothly around rounded corners and capsule ends
            float cr = min(hs.x, hs.y);
            [branch] if (I.flags.z == SHAPE_RRECT)
            {
                const float qr = p.x > c.x ? (p.y > c.y ? I.radii.z : I.radii.y) : (p.y > c.y ? I.radii.w : I.radii.x);
                cr = min(qr * (1.0 + 0.6 * I.shape.y), cr);
            }
            const float2 outside = max(abs(p - c) - (hs - cr), 0.0);
            const float ol = length(outside);
            // elliptical radius: rx along x, ry along y, blended by the normal's angle
            const float2 n = ol > 1e-4 ? outside / ol : float2(0.7071, 0.7071);
            r = rsqrt(dot(n * n, 1.0 / (rr * rr)));
            // guarantee: nothing at all past the bounds
            const float2 blo = p - I.halo.xy, bhi = I.halo.zw - p;
            edgeFade = saturate(min(min(blo.x, blo.y), min(bhi.x, bhi.y)) / 1.5);
        }
        const float x = max(d, 0.0) / r;
        const float g = exp(-x * x * 2.2) * I.glowParams.y * edgeFade;
        acc = Over(Premul(I.glow, saturate(g)), acc);
    }

    // 3. liquid glass body
    float rim = 0.0;
    [branch] if (FX_HAS(feat, F_GLASS) && cov > 0.0)
    {
        FX_FETCH(I, glass)
        FX_FETCH(I, glassTint)
        FX_FETCH(I, glassParams)
        FX_FETCH(I, shape2Params)
        GlassSample gs = EvalGlass(spos, p, d, px, I);
        rim = gs.rim;
        acc = Over(float4(gs.color * cov, cov), acc);
    }

    // 4. fill (solid / gradient / image)
    float4 fillColor = float4(0, 0, 0, 0);
    [branch] if FX_HAS(feat, F_FILL)
    {
        [branch] if (I.flags.y != PAINT_SOLID)
        {
            FX_FETCH(I, fill1)
            FX_FETCH(I, fillParams)
        }
        fillColor = EvalPaint(p, I);
        [branch] if FX_HAS(feat, F_IMAGE)
        {
            FX_FETCH(I, uvRect)
            float2 uvl = (p - I.rect.xy) / max(I.rect.zw - I.rect.xy, 1e-3);
            float2 tuv = lerp(I.uvRect.xy, I.uvRect.zw, uvl);
            fillColor *= ESIA_SAMPLE(gTex, gLinear, tuv);
        }
        acc = Over(Premul(fillColor, cov), acc);
    }

    // 4b. light streak (Painter::LightStreak): the Siri orb's light. A few thin lines of light, softened just
    // enough, held together at both ends and opening up in the middle: the top one dispersed into thin warm,
    // green and lavender lines, a faint one under it, a white one along the bottom edge with a white bloom above
    // it, and a pale sheet of light between them. The lines part and gather as the light breathes; brightness
    // saturates softly towards white (never clips).
    [branch] if FX_HAS(feat, F_CAUSTIC)
    {
        FX_FETCH(I, custom)
        FX_FETCH(I, fill1)
        const float2 sz = max(I.rect.zw - I.rect.xy, 1e-3);
        const float2 q = (p - I.rect.xy) / sz;
        const float tm = (I.fill1.x >= 0.0 ? I.fill1.x : gTime.x) * I.custom.z;
        const float xx = q.x * 2.0 - 1.0;
        const float y = q.y * sz.y;
        const float w = max(I.custom.w, 1e-3);                                          // line softness (px)
        const float ends = saturate(1.0 - xx * xx);
        const float pinch = pow(ends, 1.3);                                              // 0 at the ends: the lines meet there
        const float env = pow(ends, 0.7);
        float grow = saturate(tm / 0.7);
        grow = grow * grow * (3.0 - 2.0 * grow);
        const float opening = (0.25 + 0.75 * grow) * (0.85 + 0.15 * sin(tm * 2.1));   // opens up, then breathes
        const float A = I.custom.y * sz.y * opening * pinch;
        const float yMid = sz.y * (0.5 + I.custom.x * (0.5 - xx * xx) * 0.5) + 0.14 * sz.y * pinch * sin(xx * 2.4 - tm * 1.2 + 0.6);

        // lines: bulge (-1 = top), softness, dispersion, gain | whiteness, phase
        const float4 shape[4] = { float4(-1.0, 1.0, 3.4, 2.6), float4(-0.55, 0.8, 2.0, 0.8), float4(0.30, 1.0, 1.2, 1.3), float4(0.62, 0.8, 1.6, 0.45) };
        const float2 look[4] = { float2(0.0, 0.0), float2(0.1, 1.3), float2(0.55, 3.9), float2(0.2, 5.2) };
        float3 light = float3(0.0, 0.0, 0.0);
        float yTop = yMid, yBot = yMid;
        [unroll] for (int s = 0; s < 4; ++s)
        {
            const float yc = yMid + A * shape[s].x * (0.8 + 0.2 * sin(tm * 1.7 + look[s].y));
            if (s == 0) yTop = yc;
            if (s == 2) yBot = yc;
            const float sg = w * shape[s].y;
            const float d0 = (y - yc) / sg;
            light += lerp(DispersedRibbon(y, yc, sg, w * shape[s].z), exp(-d0 * d0).xxx, look[s].x) * (shape[s].w * env);
        }
        // the sheet between the lines: lavender under the top one, brightening to white towards the bottom
        const float e = w * 1.5;
        const float inside = 1.0 / (1.0 + exp(-(y - yTop - w * 2.0) / e)) / (1.0 + exp(-(yBot - y) / e));
        const float sv = saturate((y - yTop) / max(yBot - yTop, 1e-3));
        light += inside * (0.06 + 0.35 * pow(sv, 2.5)) * env * lerp(float3(0.78, 0.80, 1.0), float3(1.0, 1.0, 1.0), sv);
        // the bloom low in the middle, a faint warm haze above the top line
        const float db = (y - (yBot - w * 3.0)) / (sz.y * 0.09 + A * 0.2 + 1e-3);
        const float dx = (xx - 0.05) / 0.36;
        light += exp(-db * db - dx * dx) * (0.5 + 1.1 * opening) * float3(1.0, 0.98, 0.95);
        const float dh = (y - yTop + w * 5.0) / (w * 7.0);
        light += exp(-dh * dh) * pinch * 0.10 * float3(1.0, 0.68, 0.42);
        light *= lerp(float3(1.0, 0.97, 0.94), float3(0.94, 0.97, 1.0), 0.5 + 0.5 * sin(tm * 0.8));

        const float3 col = 1.0 - exp(-light * 1.4);                                      // soft, never clips
        const float k = I.fill0.a * smoothstep(0.0, 0.12, q.y) * smoothstep(1.0, 0.88, q.y);
        const float a = saturate(max(col.r, max(col.g, col.b)));
        acc = Over(float4(col * k * cov, a * k * cov), acc);                             // premultiplied light
    }

#ifdef ESIA_CUSTOM_EFFECT
    {
        FX_FETCH(I, custom)
        WgtFx fx;
        fx.pos = p;
        fx.size = max(I.rect.zw - I.rect.xy, 1e-3);
        fx.uv = (p - I.rect.xy) / fx.size;
        fx.screenUV = spos * gTarget.zw;
        fx.sd = d;
        fx.coverage = cov;
        fx.px = px;
        fx.time = gTime.x;
        fx.params = I.custom;
        fx.fill = fillColor;
        acc = Over(WgtEffect(fx), acc);
    }
#endif

    // 5. inner shadow
    [branch] if (FX_HAS(feat, F_SHADOW) && FX_HAS(feat, F_INNER_SHADOW))
    {
        FX_FETCH(I, shadow)
        FX_FETCH(I, shadowParams)
        float sigma = max(I.shadowParams.x * 0.5, px * 0.5);
        float ds = ShapeSD(p - I.shadowParams.zw, I) + I.shadowParams.y;
        float a = (0.5 + 0.5 * WgtErf(ds / (sigma * 1.41421356))) * cov;
        acc = Over(Premul(I.shadow, a), acc);
    }

    // 6. inner glow
    [branch] if FX_HAS(feat, F_INNER_GLOW)
    {
        FX_FETCH(I, glow)
        FX_FETCH(I, glowParams)
        float r = max(I.glowParams.z, 1e-3);
        float x = max(-d, 0.0) / r;
        float g = exp(-x * x * 2.2) * I.glowParams.w * cov;
        acc = Over(Premul(I.glow, saturate(g)), acc);
    }

    // 7. glass specular rim (additive light): it carries its own coverage (EvalGlass), not multiplied by it again
    acc.rgb += rim * saturate(acc.a / max(cov, 1e-3));

    // 8. stroke
    [branch] if FX_HAS(feat, F_STROKE)
    {
        FX_FETCH(I, stroke)
        FX_FETCH(I, strokeParams)
        float w = I.strokeParams.x;
        float align = I.strokeParams.y;
        float outer = d - w * align;
        float inner = d + w * (1.0 - align);
        float sc = saturate(0.5 - outer / px) - saturate(0.5 - inner / px);
        float4 scol = I.stroke;
        if (I.strokeParams.z < 0.999)
        {
            float2 size = max(I.rect.zw - I.rect.xy, 1e-3);
            float2 center = (I.rect.xy + I.rect.zw) * 0.5;
            float2 dir = float2(cos(I.strokeParams.w), sin(I.strokeParams.w));
            float ext = abs(dir.x) * size.x * 0.5 + abs(dir.y) * size.y * 0.5;
            float t = saturate(dot(p - center, dir) / max(ext, 1e-3) * 0.5 + 0.5);
            scol.a *= lerp(1.0, I.strokeParams.z, t);
        }
        acc = Over(Premul(scol, sc), acc);
    }

    // 9. shimmer sweep
    [branch] if FX_HAS(feat, F_SHIMMER)
    {
        float2 size = max(I.rect.zw - I.rect.xy, 1e-3);
        float2 uvl = (p - I.rect.xy) / size;
        float u = uvl.x * 0.85 + uvl.y * 0.15;
        float phase = frac(gTime.x * I.misc.w) * 1.8 - 0.4;
        float band = exp(-pow((u - phase) / 0.11, 2.0));
        acc.rgb += band * I.misc.z * cov * max(acc.a, 0.35);
        acc.a = max(acc.a, band * I.misc.z * cov * 0.5);
    }

    // 10. film grain
    [branch] if FX_HAS(feat, F_NOISE)
    {
        float n = WgtHash(floor(spos)) - 0.5;
        acc.rgb += n * I.misc.y * acc.a;
    }

    acc *= I.misc.x * maskCov * WgtEdgeFade(spos.y);
    // soft falloffs (glows, shadows) span many pixels with few 8-bit levels: +-1/2 level of dither breaks
    // the banding (invisible elsewhere)
    [branch] if (FX_HAS(feat, F_GLOW) || FX_HAS(feat, F_SHADOW))
        acc.rgb += (WgtHash(floor(spos) + 17.0) - 0.5) * (1.0 / 255.0) * saturate(acc.a * 8.0);
    acc.rgb = max(acc.rgb, 0.0);
    return WgtOutputPremul(acc);
}
