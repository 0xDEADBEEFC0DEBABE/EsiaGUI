// WGT UI - Dear ImGui geometry (text, lines, legacy widgets)
#include "wgt_common.hlsli"

struct ImGuiVSIn
{
    float2 pos : POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};

struct ImGuiPSIn
{
    float4 pos : SV_Position;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

ImGuiPSIn ImGuiVS(ImGuiVSIn v)
{
    ImGuiPSIn o;
    o.pos = float4(v.pos * gXform.xy + gXform.zw, 0.0, 1.0);
    o.col = v.col;
    o.uv = v.uv;
    return o;
}

float4 ImGuiPS(ImGuiPSIn i) : SV_Target
{
    float4 c = i.col * gTex.Sample(gLinear, i.uv);
    c.a *= WgtEdgeFade(i.pos.y);
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

// Grayscale coverage (Alpha8 glyph atlas, Dear ImGui's Alpha8 atlas).
float4 ImGuiTextPS(ImGuiPSIn i) : SV_Target
{
    const float cov = gTex.Sample(gLinear, i.uv).r;
    return WgtOutputStraight(float4(i.col.rgb, i.col.a * WgtGrayText(cov, i.col.rgb) * WgtEdgeFade(i.pos.y)));
}

// Sub-pixel coverage drawn into an alpha layer (glow layers): per-channel alpha cannot be stored there,
// the grayscale coverage kept in A is used instead.
float4 ImGuiTextLcdGrayPS(ImGuiPSIn i) : SV_Target
{
    const float cov = gTex.Sample(gLinear, i.uv).a;
    return WgtOutputStraight(float4(i.col.rgb, i.col.a * WgtGrayText(cov, i.col.rgb) * WgtEdgeFade(i.pos.y)));
}

// Sub-pixel (ClearType-style) text: one coverage per R/G/B stripe, composed per channel with dual-source
// blending: target = ink * alpha + target * (1 - alpha), alpha being an RGB triple.
struct TextLcdOut
{
    float4 color : SV_Target0;
    float4 alpha : SV_Target1;
};

TextLcdOut ImGuiTextLcdPS(ImGuiPSIn i)
{
    float3 cov = gTex.Sample(gLinear, i.uv).rgb;
    cov = lerp(dot(cov, 1.0 / 3.0).xxx, cov, gText.w);   // ClearType level (0 = grayscale)
    const float k = WgtLightOnDarkContrast(gText.z, i.col.rgb);
    float3 a;
    a.r = WgtTextAlpha(WgtEnhanceContrast(cov.r, k), i.col.r, gText.x);
    a.g = WgtTextAlpha(WgtEnhanceContrast(cov.g, k), i.col.g, gText.x);
    a.b = WgtTextAlpha(WgtEnhanceContrast(cov.b, k), i.col.b, gText.x);
    a *= i.col.a * WgtEdgeFade(i.pos.y);
    TextLcdOut o;
    o.color = WgtOutputStraight(float4(i.col.rgb, 1.0));
    o.alpha = float4(a, max(a.r, max(a.g, a.b)));
    return o;
}
