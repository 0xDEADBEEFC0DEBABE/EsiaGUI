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

// Portability hooks. The defaults below are shader model 4+ HLSL (every format of the shader library). A backend
// whose shader model lacks something - Direct3D 9 / SM3: no integer bit operations, no SV_VertexID / SV_InstanceID,
// no texture objects, no flat interpolation, constants in c registers - compiles these same sources with
// ESIA_SHADER_PRELUDE naming a file of its own (e.g. /DESIA_SHADER_PRELUDE="\"esia_sm3_prelude.hlsli\"") that
// defines the macros its way; everything it leaves undefined keeps the default. docs/backends/README.md lists them.
#ifdef ESIA_SHADER_PRELUDE
#include ESIA_SHADER_PRELUDE
#endif

#ifndef ESIA_BINDING
#ifdef ESIA_SPIRV
#define ESIA_BINDING(n) [[vk::binding(n, 0)]]
#define ESIA_LOCATION(n) [[vk::location(n)]]
#else
#define ESIA_BINDING(n)
#define ESIA_LOCATION(n)
#endif
#endif
#ifndef ESIA_CBUFFER
#define ESIA_CBUFFER(name, reg, binding) ESIA_BINDING(binding) cbuffer name : register(reg)
#endif
#ifndef ESIA_TEXTURE
#define ESIA_TEXTURE(name, reg, binding) ESIA_BINDING(binding) Texture2D name : register(reg)
#define ESIA_TEXTURE_ARG Texture2D
#define ESIA_SAMPLE(tex, smp, uv) tex.Sample(smp, uv)
#define ESIA_SAMPLE_LEVEL(tex, smp, uv) tex.SampleLevel(smp, uv, 0)
#define ESIA_LOAD(tex, texel) tex.Load(int3(texel, 0))
#endif
#ifndef ESIA_SAMPLER
#define ESIA_SAMPLER(name, reg, binding) ESIA_BINDING(binding) SamplerState name : register(reg)
#endif
#ifndef ESIA_HAS
#define ESIA_HAS(bits, flag) (((bits) & (flag)) != 0u)   // flag set in an integer bit field
#endif
#ifndef ESIA_VERTEX_ID
#define ESIA_VERTEX_ID(name) uint name : SV_VertexID
#define ESIA_INSTANCE_ID(name) uint name : SV_InstanceID
#endif
#ifndef ESIA_FLAT
#define ESIA_FLAT nointerpolation                         // integer varyings (the FX instance index)
#define ESIA_FLAT_UINT(v) (v)                             // ... read in the pixel shader
#endif
#ifndef ESIA_CLIP_POSITION
#define ESIA_CLIP_POSITION(p) (p)                         // every SV_Position a vertex shader writes (SM3: half pixel)
#endif

ESIA_CBUFFER(WgtFrame, b0, 0)
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

ESIA_CBUFFER(WgtPass, b1, 1)
{
    float4 gPass0;
    float4 gPass1;
};

ESIA_CBUFFER(WgtDraw, b2, 2)
{
    float4 gFade;       // edge fade: x top edge, y bottom edge (render-target px), z / w fade widths (0 = none)
    float4 gDrawInfo;   // x: first FX instance of the draw
};

ESIA_TEXTURE(gTex, t0, 3);
ESIA_TEXTURE(gBackdrop0, t1, 4);
ESIA_TEXTURE(gBackdrop1, t2, 5);
ESIA_TEXTURE(gBackdrop2, t3, 6);
ESIA_TEXTURE(gBackdrop3, t4, 7);
ESIA_TEXTURE(gBackdrop4, t5, 8);
ESIA_TEXTURE(gBackdrop5, t6, 9);
ESIA_SAMPLER(gLinear, s0, 11);
ESIA_SAMPLER(gPoint, s1, 12);

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
float4 WgtSampleBSpline(ESIA_TEXTURE_ARG tex, float2 uv, float4 level)
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
    return (ESIA_SAMPLE_LEVEL(tex, gLinear, WgtRtUv(float2(t0.x, t0.y))) * s0.x + ESIA_SAMPLE_LEVEL(tex, gLinear, WgtRtUv(float2(t1.x, t0.y))) * s1.x) * s0.y
         + (ESIA_SAMPLE_LEVEL(tex, gLinear, WgtRtUv(float2(t0.x, t1.y))) * s0.x + ESIA_SAMPLE_LEVEL(tex, gLinear, WgtRtUv(float2(t1.x, t1.y))) * s1.x) * s1.y;
}

float4 WgtSampleLevel(int level, float2 uv)
{
    float4 r;
    [branch] if (level <= 0)      r = ESIA_SAMPLE_LEVEL(gBackdrop0, gLinear, WgtRtUv(uv));
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
    [branch] if (level <= 0)      r = ESIA_SAMPLE_LEVEL(gBackdrop0, gLinear, t);
    else if (level == 1)          r = ESIA_SAMPLE_LEVEL(gBackdrop1, gLinear, t);
    else if (level == 2)          r = ESIA_SAMPLE_LEVEL(gBackdrop2, gLinear, t);
    else if (level == 3)          r = ESIA_SAMPLE_LEVEL(gBackdrop3, gLinear, t);
    else if (level == 4)          r = ESIA_SAMPLE_LEVEL(gBackdrop4, gLinear, t);
    else                          r = ESIA_SAMPLE_LEVEL(gBackdrop5, gLinear, t);
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
