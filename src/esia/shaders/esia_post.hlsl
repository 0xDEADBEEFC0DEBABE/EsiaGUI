// Esia - full-screen passes: backdrop pyramid downsampling, glow-layer compositing, scissored clears, HDR output.
// Programs Downsample, LayerComposite, Clear, HdrDecode, HdrEncode (FullscreenVS: 3 vertices, no vertex buffer).
#include "esia_common.hlsli"

struct FsOut
{
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;   // top-left based uv of the target
};

FsOut FullscreenVS(ESIA_VERTEX_ID(vid))
{
    FsOut o;
    float2 uv = float2(ESIA_HAS(vid, 1u) ? 2.0 : 0.0, ESIA_HAS(vid, 2u) ? 2.0 : 0.0);   // (0,0) (2,0) (0,2)
    o.uv = uv;
    // gXform.y's sign carries the API's clip-space y direction (Caps::clipSpaceYDown)
    o.pos = ESIA_CLIP_POSITION(float4(uv * float2(2.0, gXform.y < 0.0 ? -2.0 : 2.0) + float2(-1.0, gXform.y < 0.0 ? 1.0 : -1.0), 0.0, 1.0));
    return o;
}

// 13-tap "dual box" downsample (Jimenez 2014): stable, alias-free, cheap.
// gPass0.xy = source texel size, gPass1 = valid source region (top-left uv min.xy, max.xy).
// Taps are clamped to the region that was actually refreshed this capture, so stale texels from earlier
// captures can never leak into the pyramid (lets captures stay tightly region-limited).
float4 Tap(float2 uv)
{
    return ESIA_SAMPLE_LEVEL(gTex, gLinear, WgtRtUv(clamp(uv, gPass1.xy, gPass1.zw)));
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
    float4 sharp = ESIA_SAMPLE_LEVEL(gTex, gPoint, WgtRtUv(uv));
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

// ------------------------------------------------------------------ HDR output
// RenderParams::hdr: the UI draws into a working target holding what an SDR target would - sRGB values, 1 = paper
// white - with what lies past them kept: values above 1 (brighter than paper white) and below 0 (outside BT.709),
// "extended sRGB". HdrDecode brings the region the frame covers into it from the target; HdrEncode takes it back in
// the target's encoding. gPass0.x = paper white (nits), gPass0.y = the target's encoding: 1 scRGB (linear BT.709,
// 1 = 80 nits), 2 PQ (SMPTE ST 2084) with BT.2020 primaries.

// sRGB's transfer functions continued past [0, 1]: mirrored below 0, the same curve above 1
float3 WgtSrgbToLinearExt(float3 v)
{
    const float3 a = abs(v);
    return sign(v) * (a <= 0.04045 ? a / 12.92 : pow((a + 0.055) / 1.055, 2.4));
}

float3 WgtLinearToSrgbExt(float3 l)
{
    const float3 a = abs(l);
    return sign(l) * (a <= 0.0031308 ? a * 12.92 : 1.055 * pow(a, 1.0 / 2.4) - 0.055);
}

// SMPTE ST 2084: code values <-> nits
float3 WgtPqToNits(float3 e)
{
    const float3 p = pow(saturate(e), 1.0 / 78.84375);
    return 10000.0 * pow(max(p - 0.8359375, 0.0) / (18.8515625 - 18.6875 * p), 1.0 / 0.1593017578125);
}

float3 WgtNitsToPq(float3 n)
{
    const float3 y = pow(saturate(n / 10000.0), 0.1593017578125);
    return pow((0.8359375 + 18.8515625 * y) / (1.0 + 18.6875 * y), 78.84375);
}

// BT.2020 <-> BT.709 primaries, linear light (ITU-R BT.2087)
float3 WgtRec2020To709(float3 c)
{
    return float3(dot(c, float3(1.6605, -0.5876, -0.0728)), dot(c, float3(-0.1246, 1.1329, -0.0083)), dot(c, float3(-0.0182, -0.1006, 1.1187)));
}

float3 WgtRec709To2020(float3 c)
{
    return float3(dot(c, float3(0.6274, 0.3293, 0.0433)), dot(c, float3(0.0691, 0.9195, 0.0114)), dot(c, float3(0.0164, 0.0880, 0.8956)));
}

float4 HdrDecodePS(FsOut i) : SV_Target
{
    const float2 uv = WgtPixelPos(i.pos) * gTarget.zw;
    const float4 c = ESIA_SAMPLE_LEVEL(gTex, gPoint, WgtRtUv(uv));
    const float3 nits = gPass0.y > 1.5 ? WgtRec2020To709(WgtPqToNits(c.rgb)) : c.rgb * 80.0;
    return float4(WgtLinearToSrgbExt(nits / gPass0.x), c.a);
}

float4 HdrEncodePS(FsOut i) : SV_Target
{
    const float2 uv = WgtPixelPos(i.pos) * gTarget.zw;
    const float4 c = ESIA_SAMPLE_LEVEL(gTex, gPoint, WgtRtUv(uv));
    const float3 nits = WgtSrgbToLinearExt(c.rgb) * gPass0.x;
    return float4(gPass0.y > 1.5 ? WgtNitsToPq(WgtRec709To2020(nits)) : nits / 80.0, c.a);
}
