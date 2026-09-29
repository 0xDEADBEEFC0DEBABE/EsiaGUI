// Esia - full-screen passes: backdrop pyramid downsampling, glow-layer compositing, scissored clears.
// Programs Downsample, LayerComposite, Clear (FullscreenVS: 3 vertices, no vertex buffer).
#include "esia_common.hlsli"

struct FsOut
{
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;   // top-left based uv of the target
};

FsOut FullscreenVS(uint vid : SV_VertexID)
{
    FsOut o;
    float2 uv = float2((vid << 1) & 2, vid & 2);
    o.uv = uv;
    // gXform.y's sign carries the API's clip-space y direction (Caps::clipSpaceYDown)
    o.pos = float4(uv * float2(2.0, gXform.y < 0.0 ? -2.0 : 2.0) + float2(-1.0, gXform.y < 0.0 ? 1.0 : -1.0), 0.0, 1.0);
    return o;
}

// 13-tap "dual box" downsample (Jimenez 2014): stable, alias-free, cheap.
// gPass0.xy = source texel size, gPass1 = valid source region (top-left uv min.xy, max.xy).
// Taps are clamped to the region that was actually refreshed this capture, so stale texels from earlier
// captures can never leak into the pyramid (lets captures stay tightly region-limited).
float4 Tap(float2 uv)
{
    return gTex.SampleLevel(gLinear, WgtRtUv(clamp(uv, gPass1.xy, gPass1.zw)), 0);
}

float4 DownsamplePS(FsOut i) : SV_Target
{
    float2 t = gPass0.xy;
    float2 uv = i.uv;
    float4 a = Tap(uv + t * float2(-2, -2));
    float4 b = Tap(uv + t * float2( 0, -2));
    float4 c = Tap(uv + t * float2( 2, -2));
    float4 d = Tap(uv + t * float2(-1, -1));
    float4 e = Tap(uv + t * float2( 1, -1));
    float4 f = Tap(uv + t * float2(-2,  0));
    float4 g = Tap(uv);
    float4 h = Tap(uv + t * float2( 2,  0));
    float4 j = Tap(uv + t * float2(-1,  1));
    float4 k = Tap(uv + t * float2( 1,  1));
    float4 l = Tap(uv + t * float2(-2,  2));
    float4 m = Tap(uv + t * float2( 0,  2));
    float4 n = Tap(uv + t * float2( 2,  2));
    float4 r = (d + e + j + k) * 0.125;
    r += (a + b + f + g) * 0.03125;
    r += (b + c + g + h) * 0.03125;
    r += (f + g + l + m) * 0.03125;
    r += (g + h + m + n) * 0.03125;
    return r;
}

// Glow layer composite. gTex = sharp layer (premultiplied), t2..t6 = pyramid built from it.
// gPass0 = glow color (rgb) + tint amount (a); gPass1.x = intensity, y = radius (UI units), z = content opacity
float4 LayerCompositePS(FsOut i) : SV_Target
{
    float2 uv = WgtPixelPos(i.pos) * gTarget.zw;
    float4 sharp = gTex.SampleLevel(gPoint, WgtRtUv(uv), 0);
    float radius = max(gPass1.y * gDisplay.z, 2.0);   // UI units -> render-target pixels
    float lv = clamp(log2(radius) - 1.0, 1.0, 5.0);
    int l0 = (int)floor(lv);
    float f = lv - (float)l0;
    float4 bloom = WgtSampleLevel(l0, uv);
    if (l0 < 5)
        bloom = lerp(bloom, WgtSampleLevel(l0 + 1, uv), f);
    // a wide, faint tail makes the glow feel luminous rather than blurred
    bloom += WgtSampleLevel(min(l0 + 2, 5), uv) * 0.35;
    float luma = dot(bloom.rgb, float3(0.2126, 0.7152, 0.0722));
    float3 tinted = lerp(bloom.rgb, luma * gPass0.rgb * 1.6, gPass0.a);
    float3 glow = max(tinted * gPass1.x, 0.0);
    float4 content = sharp * gPass1.z;
    // Content + bloom through a smooth shoulder instead of a hard clip at 1: the letters stay bright, but an
    // edge pixel (partial coverage) stays darker than an interior one, so the anti-aliasing ramp survives.
    float3 sum = content.rgb + glow;
    const float knee = 0.6;
    sum = sum < knee ? sum : knee + (1.0 - knee) * (1.0 - exp(-(sum - knee) / (1.0 - knee)));
    float4 outc = float4(sum, content.a);
    if (gTime.w > 0.5)
        outc.rgb = WgtSrgbToLinear(outc.rgb);
    return outc;
}

// Scissored clear to transparent black (glow layers clear only the region they use). A draw instead of an API
// clear: Metal has no partial clear, and it keeps every backend's pass structure the same.
float4 ClearPS(FsOut i) : SV_Target
{
    return float4(0.0, 0.0, 0.0, 0.0);
}
