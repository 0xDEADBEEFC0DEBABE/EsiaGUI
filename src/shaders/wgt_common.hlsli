// WGT UI - shared shader declarations (DX11 + DX12, shader model 5.0)
//
// Register layout (identical for both APIs):
//   b0  WgtFrame   per-frame constants   (DX12: root CBV)
//   b1  WgtPass    per-pass constants    (DX12: 8 root constants)
//   b2  WgtDraw    per-draw constants    (DX12: 4 root constants, pixel shader)
//   t0  gTex       primary texture (font atlas / image / layer)
//   t1..t6         backdrop pyramid (t1 = full-res copy, t2..t6 = 1/2 .. 1/32)
//   s0  linear clamp, s1 point clamp
#ifndef WGT_COMMON_HLSLI
#define WGT_COMMON_HLSLI

cbuffer WgtFrame : register(b0)
{
    float4 gXform;      // clip.xy = pos.xy * gXform.xy + gXform.zw
    float4 gTarget;     // xy: render target size (px)   zw: 1 / size
    float4 gDisplay;    // xy: ImGui display pos         zw: framebuffer scale
    float4 gTime;       // x: seconds  y: delta  z: backdrop valid  w: 1 = target is sRGB (write linear)
    float4 gLevel[6];   // backdrop pyramid: xy size (px), zw 1/size. [0] = full-res copy
    float4 gText;       // x gamma, y grayscale enhanced contrast, z ClearType enhanced contrast, w ClearType level
};

cbuffer WgtPass : register(b1)
{
    float4 gPass0;
    float4 gPass1;
};

cbuffer WgtDraw : register(b2)
{
    float4 gFade;   // edge fade: x top edge, y bottom edge (render-target px), z / w fade widths (0 = none)
};

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

SamplerState gLinear : register(s0);
SamplerState gPoint  : register(s1);

Texture2D gTex       : register(t0);
Texture2D gBackdrop0 : register(t1);
Texture2D gBackdrop1 : register(t2);
Texture2D gBackdrop2 : register(t3);
Texture2D gBackdrop3 : register(t4);
Texture2D gBackdrop4 : register(t5);
Texture2D gBackdrop5 : register(t6);

static const float WGT_PI  = 3.14159265359;
static const float WGT_TAU = 6.28318530718;

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
    return (tex.SampleLevel(gLinear, float2(t0.x, t0.y), 0) * s0.x + tex.SampleLevel(gLinear, float2(t1.x, t0.y), 0) * s1.x) * s0.y
         + (tex.SampleLevel(gLinear, float2(t0.x, t1.y), 0) * s0.x + tex.SampleLevel(gLinear, float2(t1.x, t1.y), 0) * s1.x) * s1.y;
}

float4 WgtSampleLevel(int level, float2 uv)
{
    float4 r;
    [branch] if (level <= 0)      r = gBackdrop0.SampleLevel(gLinear, uv, 0);
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
    float4 r;
    [branch] if (level <= 0)      r = gBackdrop0.SampleLevel(gLinear, uv, 0);
    else if (level == 1)          r = gBackdrop1.SampleLevel(gLinear, uv, 0);
    else if (level == 2)          r = gBackdrop2.SampleLevel(gLinear, uv, 0);
    else if (level == 3)          r = gBackdrop3.SampleLevel(gLinear, uv, 0);
    else if (level == 4)          r = gBackdrop4.SampleLevel(gLinear, uv, 0);
    else                          r = gBackdrop5.SampleLevel(gLinear, uv, 0);
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

// Samples the captured backdrop at screen uv with an (approximate) gaussian blur radius in pixels.
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
