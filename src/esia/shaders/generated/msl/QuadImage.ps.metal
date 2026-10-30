#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct WgtFrame
{
    float4 gXform;
    float4 gTarget;
    float4 gDisplay;
    float4 gTime;
    float4 gLevel[6];
    float4 gText;
    float4 gConv;
};

struct WgtDraw
{
    float4 gFade;
    float4 gDrawInfo;
};

constant float2 _413 = {};

struct esia_main_out
{
    float4 _entryPointOutput [[color(0)]];
};

struct esia_main_in
{
    float4 i_col [[user(locn0)]];
    float2 i_uv [[user(locn1)]];
};

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _50 [[buffer(0)]], constant WgtDraw& _82 [[buffer(2)]], texture2d<float> gTex [[texture(3)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float4 _253 = gTex.sample(gLinear, in.i_uv);
    float4 _267 = in.i_col * _253;
    float2 _286 = gl_FragCoord.xy + _50.gConv.yy;
    float2 _405;
    if (_50.gConv.x > 0.5)
    {
        float2 _386;
        _386.y = _50.gTarget.y - _286.y;
        _405 = _386;
    }
    else
    {
        _405 = _286;
    }
    float _406;
    if (_82.gFade.z > 0.0)
    {
        _406 = smoothstep(0.0, 1.0, fast::clamp((_405.y - _82.gFade.x) / _82.gFade.z, 0.0, 1.0));
    }
    else
    {
        _406 = 1.0;
    }
    float _407;
    if (_82.gFade.w > 0.0)
    {
        _407 = _406 * smoothstep(0.0, 1.0, fast::clamp((_82.gFade.y - _405.y) / _82.gFade.w, 0.0, 1.0));
    }
    else
    {
        _407 = _406;
    }
    float _274 = _267.w * _407;
    float4 _389 = _267;
    _389.w = _274;
    float4 _411;
    if (_50.gTime.w > 0.5)
    {
        float3 _358 = fast::clamp(_389.xyz, float3(0.0), float3(1.0));
        _411 = float4(select(pow((_358 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _358 * float3(0.077399380505084991455078125), _358 <= float3(0.040449999272823333740234375)), _274);
    }
    else
    {
        _411 = _389;
    }
    out._entryPointOutput = _411;
    return out;
}

