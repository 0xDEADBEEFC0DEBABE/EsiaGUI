// Esia - shared shader declarations. One HLSL source for every API (docs/REWRITE.md, "Shader strategy"):
//   * Direct3D 10 / 11 / 12: compiled as is (fxc SM4 / SM5, or DXC SM6);
//   * Vulkan: HLSL -> SPIR-V (glslang or DXC) with the [[vk::binding]] numbers below (ESIA_SPIRV defined);
//   * OpenGL 3.3 / GLES 3 / Metal: SPIR-V -> GLSL / ESSL / MSL with SPIRV-Cross (tools/shaders/build_shaders.py).
//
// Binding model (identical everywhere; Vulkan binding numbers in brackets, all in descriptor set 0):
//   b0 [0]  WgtFrame   per-frame constants
//   b1 [1]  WgtPass    per-pass constants (pyramid step, layer composite)
//   b2 [2]  WgtDraw    per-draw constants (edge fade, first FX instance)
//   t0 [3]  gTex       primary texture (glyph atlas / image / layer / pyramid source)
//   t1..t6 [4..9]      backdrop pyramid (t1 = level 0 = full resolution, t2..t6 = 1/2 .. 1/32)
//   t7 [10] gFxData    FX instance data: StructuredBuffer<float4> or, with ESIA_FX_STORAGE_TEXTURE, an RGBA32F
//                      texture fetched by texel (APIs without structured / storage buffers)
//   s0 [11] linear clamp, s1 [12] point clamp
//
// Pixel positions: SV_Position is read through WgtPixelPos, render-target textures are sampled through
// WgtRtUv. Both flip y when the API's framebuffer origin is the bottom-left (OpenGL: gConv.x = 1), so every
// shader works in top-left render-target pixels.
#ifndef ESIA_COMMON_HLSLI
#define ESIA_COMMON_HLSLI

#ifdef ESIA_SPIRV
#define ESIA_BINDING(n) [[vk::binding(n, 0)]]
#define ESIA_LOCATION(n) [[vk::location(n)]]
#else
#define ESIA_BINDING(n)
#define ESIA_LOCATION(n)
#endif

ESIA_BINDING(0) cbuffer WgtFrame : register(b0)
{
    float4 gXform;      // clip.xy = pos.xy * gXform.xy + gXform.zw
    float4 gTarget;     // xy: render target size (px)   zw: 1 / size
    float4 gDisplay;    // xy: UI display pos            zw: framebuffer scale
    float4 gTime;       // x: seconds  y: delta  z: backdrop valid  w: 1 = target is sRGB (write linear)
    float4 gLevel[6];   // backdrop pyramid: xy size (px), zw 1/size. [0] = full-res copy
    float4 gText;       // x gamma, y grayscale enhanced contrast, z ClearType enhanced contrast, w ClearType level
    float4 gConv;       // x: 1 = framebuffer origin bottom-left (flip y)  y: SV_Position offset to pixel centers
                        // z: FX instances per row of the instance texture  w: unused
};

ESIA_BINDING(1) cbuffer WgtPass : register(b1)
{
    float4 gPass0;
    float4 gPass1;
};

ESIA_BINDING(2) cbuffer WgtDraw : register(b2)
{
    float4 gFade;       // edge fade: x top edge, y bottom edge (render-target px), z / w fade widths (0 = none)
    float4 gDrawInfo;   // x: first FX instance of the draw
};

ESIA_BINDING(3)  Texture2D gTex       : register(t0);
ESIA_BINDING(4)  Texture2D gBackdrop0 : register(t1);
ESIA_BINDING(5)  Texture2D gBackdrop1 : register(t2);
ESIA_BINDING(6)  Texture2D gBackdrop2 : register(t3);
ESIA_BINDING(7)  Texture2D gBackdrop3 : register(t4);
ESIA_BINDING(8)  Texture2D gBackdrop4 : register(t5);
ESIA_BINDING(9)  Texture2D gBackdrop5 : register(t6);
ESIA_BINDING(11) SamplerState gLinear : register(s0);
ESIA_BINDING(12) SamplerState gPoint  : register(s1);

static const float WGT_PI  = 3.14159265359;
static const float WGT_TAU = 6.28318530718;

// Top-left render-target pixel position of a fragment (SV_Position), on every API.
float2 WgtPixelPos(float4 svpos)
{
    float2 p = svpos.xy + gConv.yy;
    if (gConv.x > 0.5)
        p.y = gTarget.y - p.y;
    return p;
}

// Texture coordinates of a render-target texture (backdrop copy, pyramid, glow layer) from top-left uv.
float2 WgtRtUv(float2 uv)
{
    if (gConv.x > 0.5)
        uv.y = 1.0 - uv.y;
    return uv;
}

// Scroll views: content dissolves towards the edges instead of being cut (a smooth alpha ramp).
float WgtEdgeFade(float y)
{
    float a = 1.0;
    if (gFade.z > 0.0)
        a *= smoothstep(0.0, 1.0, saturate((y - gFade.x) / gFade.z));
    if (gFade.w > 0.0)
        a *= smoothstep(0.0, 1.0, saturate((gFade.y - y) / gFade.w));
    return a;
}

float3 WgtSrgbToLinear(float3 c)
{
    c = saturate(c);
    return (c <= 0.04045) ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4);
}

float4 WgtOutputStraight(float4 c)
{
    if (gTime.w > 0.5)
        c.rgb = WgtSrgbToLinear(c.rgb);
    return c;
}

float4 WgtOutputPremul(float4 c)
{
    if (gTime.w > 0.5 && c.a > 1e-5)
        c.rgb = WgtSrgbToLinear(c.rgb / c.a) * c.a;
    return c;
}

float WgtHash(float2 p)
{
    float3 p3 = frac(float3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return frac((p3.x + p3.y) * p3.z);
}

float WgtErf(float x)
{
    float s = sign(x), a = abs(x);
    float t = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
    t *= t;
    return s - s / (t * t);
}

// Cubic B-spline reconstruction with 4 bilinear taps: smooth magnification of low-res blur levels.
// `uv` is top-left based; the flip for bottom-left APIs happens on the final taps.
float4 WgtSampleBSpline(Texture2D tex, float2 uv, float4 level)
{
    float2 texel = uv * level.xy - 0.5;
    float2 tc = floor(texel);
    float2 f = texel - tc;
    float2 f2 = f * f, f3 = f2 * f;
    float2 w0 = (1.0 / 6.0) * (-f3 + 3.0 * f2 - 3.0 * f + 1.0);
    float2 w1 = (1.0 / 6.0) * (3.0 * f3 - 6.0 * f2 + 4.0);
    float2 w2 = (1.0 / 6.0) * (-3.0 * f3 + 3.0 * f2 + 3.0 * f + 1.0);
    float2 w3 = (1.0 / 6.0) * f3;
    float2 s0 = w0 + w1, s1 = w2 + w3;
    float2 t0 = (tc - 0.5 + w1 / s0) * level.zw;
    float2 t1 = (tc + 1.5 + w3 / s1) * level.zw;
    return (tex.SampleLevel(gLinear, WgtRtUv(float2(t0.x, t0.y)), 0) * s0.x + tex.SampleLevel(gLinear, WgtRtUv(float2(t1.x, t0.y)), 0) * s1.x) * s0.y
         + (tex.SampleLevel(gLinear, WgtRtUv(float2(t0.x, t1.y)), 0) * s0.x + tex.SampleLevel(gLinear, WgtRtUv(float2(t1.x, t1.y)), 0) * s1.x) * s1.y;
}

float4 WgtSampleLevel(int level, float2 uv)
{
    float4 r;
    [branch] if (level <= 0)      r = gBackdrop0.SampleLevel(gLinear, WgtRtUv(uv), 0);
    else if (level == 1)          r = WgtSampleBSpline(gBackdrop1, uv, gLevel[1]);
    else if (level == 2)          r = WgtSampleBSpline(gBackdrop2, uv, gLevel[2]);
    else if (level == 3)          r = WgtSampleBSpline(gBackdrop3, uv, gLevel[3]);
    else if (level == 4)          r = WgtSampleBSpline(gBackdrop4, uv, gLevel[4]);
    else                          r = WgtSampleBSpline(gBackdrop5, uv, gLevel[5]);
    return r;
}

// One level, plain bilinear: for wide, low-contrast reads (reflections, exposure) where the B-spline's smoothing
// cannot be seen - one texture read instead of four.
float4 WgtSampleLevelBilinear(int level, float2 uv)
{
    const float2 t = WgtRtUv(uv);
    float4 r;
    [branch] if (level <= 0)      r = gBackdrop0.SampleLevel(gLinear, t, 0);
    else if (level == 1)          r = gBackdrop1.SampleLevel(gLinear, t, 0);
    else if (level == 2)          r = gBackdrop2.SampleLevel(gLinear, t, 0);
    else if (level == 3)          r = gBackdrop3.SampleLevel(gLinear, t, 0);
    else if (level == 4)          r = gBackdrop4.SampleLevel(gLinear, t, 0);
    else                          r = gBackdrop5.SampleLevel(gLinear, t, 0);
    return r;
}

// WgtSampleBackdrop's soft sibling (bilinear per level): the blurred surroundings a rim reflects.
float3 WgtSampleBackdropSoft(float2 uv, float radiusPx)
{
    float lv = clamp(log2(max(radiusPx, 1.0)) - 1.0, 0.0, 5.0);
    int l0 = (int)floor(lv);
    float f = lv - (float)l0;
    float3 a = WgtSampleLevelBilinear(l0, uv).rgb;
    [branch] if (f > 0.02 && l0 < 5)
        a = lerp(a, WgtSampleLevelBilinear(l0 + 1, uv).rgb, f);
    return a;
}

// Samples the captured backdrop at screen uv (top-left based) with an (approximate) gaussian blur radius in pixels.
float3 WgtSampleBackdrop(float2 uv, float radiusPx)
{
    float lv = clamp(log2(max(radiusPx, 1.0)) - 1.0, 0.0, 5.0);
    int l0 = (int)floor(lv);
    float f = lv - (float)l0;
    float3 a = WgtSampleLevel(l0, uv).rgb;
    [branch] if (f > 0.02 && l0 < 5)
        a = lerp(a, WgtSampleLevel(l0 + 1, uv).rgb, f);
    return a;
}

#endif
