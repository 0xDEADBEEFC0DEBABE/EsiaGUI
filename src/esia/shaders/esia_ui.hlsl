// Esia - indexed UI geometry (images, lines, charts) and grayscale text. Programs UiGeometry, TextGray.
#include "esia_common.hlsli"

// esia::Vertex: attribute locations 0, 1, 2 (GL / Vulkan / Metal vertex descriptors)
struct UiVSIn
{
    ESIA_LOCATION(0) float2 pos : POSITION;
    ESIA_LOCATION(1) float2 uv  : TEXCOORD0;
    ESIA_LOCATION(2) float4 col : COLOR0;
};

struct UiPSIn
{
    float4 pos : SV_Position;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

UiPSIn UiVS(UiVSIn v)
{
    UiPSIn o;
    o.pos = ESIA_CLIP_POSITION(float4(v.pos * gXform.xy + gXform.zw, 0.0, 1.0));
    o.col = v.col;
    o.uv = v.uv;
    return o;
}

float4 UiPS(UiPSIn i) : SV_Target
{
    float4 c = i.col * ESIA_SAMPLE(gTex, gLinear, i.uv);
    c.a *= WgtEdgeFade(WgtPixelPos(i.pos).y);
    return WgtOutputStraight(c);
}

// ------------------------------------------------------------------ text ----
// Glyph coverage is the exact covered area (linear). Blending it straight into a gamma-encoded target
// makes the anti-aliasing ramp perceptually uneven: dark ink looks heavy and stair-stepped, light ink thin.
// Text is composed the way DirectWrite composes it, with the system's parameters (gText):
//   1. enhanced contrast (stem weight), full for dark ink, fading out for light ink,
//   2. alpha correction toward a linear-light blend for the display gamma (DirectWrite's formula).
float WgtEnhanceContrast(float a, float k) { return a * (k + 1.0) / (a * k + 1.0); }

float WgtLightOnDarkContrast(float k, float3 ink)
{
    return k * saturate(4.0 * (0.75 - dot(ink, float3(0.30, 0.59, 0.11))));
}

// DirectWrite's alpha correction: a + a(1-a)((r0 f + r1) a + (r2 f + r3)), f = ink intensity, r = the
// "gamma ratios" for the display gamma (Direct2D's table, 1.0 .. 2.2). A partial correction toward a
// linear-light blend: dark ink loses a little weight at the edges, light ink gains some.
float4 WgtGammaRatios(float gamma)
{
    static const float4 kRatios[13] = {
        float4(0.0000, 0.0000, 0.0000, 0.0000), float4(0.0166, -0.0807, 0.2227, -0.0751), float4(0.0350, -0.1760, 0.4325, -0.1370),
        float4(0.0543, -0.2821, 0.6302, -0.1876), float4(0.0739, -0.3963, 0.8167, -0.2287), float4(0.0933, -0.5161, 0.9926, -0.2616),
        float4(0.1121, -0.6395, 1.1588, -0.2877), float4(0.1300, -0.7649, 1.3159, -0.3080), float4(0.1469, -0.8911, 1.4644, -0.3234),
        float4(0.1627, -1.0170, 1.6051, -0.3347), float4(0.1773, -1.1420, 1.7385, -0.3426), float4(0.1908, -1.2652, 1.8650, -0.3476),
        float4(0.2031, -1.3864, 1.9851, -0.3501)};
    const float t = clamp((gamma - 1.0) * 10.0, 0.0, 12.0);
    const int i = min((int)t, 11);
    return lerp(kRatios[i], kRatios[i + 1], t - (float)i) * 0.25;
}

float WgtTextAlpha(float a, float ink, float gamma)
{
    const float4 r = WgtGammaRatios(gamma);
    return saturate(a + a * (1.0 - a) * ((r.x * ink + r.y) * a + (r.z * ink + r.w)));
}

float WgtGrayText(float cov, float3 ink)
{
    const float k = WgtLightOnDarkContrast(gText.y, ink);
    const float intensity = dot(ink, float3(0.25, 0.5, 0.25));
    return WgtTextAlpha(WgtEnhanceContrast(cov, k), intensity, gText.x);
}

// Grayscale coverage (Alpha8 glyph atlas).
float4 TextGrayPS(UiPSIn i) : SV_Target
{
    const float cov = ESIA_SAMPLE(gTex, gLinear, i.uv).r;
    return WgtOutputStraight(float4(i.col.rgb, i.col.a * WgtGrayText(cov, i.col.rgb) * WgtEdgeFade(WgtPixelPos(i.pos).y)));
}
