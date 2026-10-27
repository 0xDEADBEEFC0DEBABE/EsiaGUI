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

constant float2 _389 = {};

struct esia_main_out
{
    float4 _entryPointOutput [[color(0)]];
};

struct esia_main_in
{
    float4 i_col [[user(locn0)]];
    float2 i_uv [[user(locn1)]];
};

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _44 [[buffer(0)]], constant WgtDraw& _76 [[buffer(2)]], texture2d<float> gTex [[texture(3)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float4 _241 = gTex.sample(gLinear, in.i_uv);
    float4 _242 = in.i_col * _241;
    float2 _262 = gl_FragCoord.xy + _44.gConv.yy;
    float2 _381;
    if (_44.gConv.x > 0.5)
    {
        float2 _362;
        _362.y = _44.gTarget.y - _262.y;
        _381 = _362;
    }
    else
    {
        _381 = _262;
    }
    float _382;
    if (_76.gFade.z > 0.0)
    {
        _382 = smoothstep(0.0, 1.0, fast::clamp((_381.y - _76.gFade.x) / _76.gFade.z, 0.0, 1.0));
    }
    else
    {
        _382 = 1.0;
    }
    float _383;
    if (_76.gFade.w > 0.0)
    {
        _383 = _382 * smoothstep(0.0, 1.0, fast::clamp((_76.gFade.y - _381.y) / _76.gFade.w, 0.0, 1.0));
    }
    else
    {
        _383 = _382;
    }
    float _250 = _242.w * _383;
    float4 _365 = _242;
    _365.w = _250;
    float4 _387;
    if (_44.gTime.w > 0.5)
    {
        float3 _334 = fast::clamp(_365.xyz, float3(0.0), float3(1.0));
        _387 = float4(select(pow((_334 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _334 * float3(0.077399380505084991455078125), _334 <= float3(0.040449999272823333740234375)), _250);
    }
    else
    {
        _387 = _365;
    }
    out._entryPointOutput = _387;
    return out;
}

