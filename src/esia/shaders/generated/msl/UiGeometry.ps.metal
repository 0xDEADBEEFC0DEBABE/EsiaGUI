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

constant float2 _409 = {};

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
    float4 _249 = gTex.sample(gLinear, in.i_uv);
    float4 _263 = in.i_col * _249;
    float2 _282 = gl_FragCoord.xy + _50.gConv.yy;
    float2 _401;
    if (_50.gConv.x > 0.5)
    {
        float2 _382;
        _382.y = _50.gTarget.y - _282.y;
        _401 = _382;
    }
    else
    {
        _401 = _282;
    }
    float _402;
    if (_82.gFade.z > 0.0)
    {
        _402 = smoothstep(0.0, 1.0, fast::clamp((_401.y - _82.gFade.x) / _82.gFade.z, 0.0, 1.0));
    }
    else
    {
        _402 = 1.0;
    }
    float _403;
    if (_82.gFade.w > 0.0)
    {
        _403 = _402 * smoothstep(0.0, 1.0, fast::clamp((_82.gFade.y - _401.y) / _82.gFade.w, 0.0, 1.0));
    }
    else
    {
        _403 = _402;
    }
    float _270 = _263.w * _403;
    float4 _385 = _263;
    _385.w = _270;
    float4 _407;
    if (_50.gTime.w > 0.5)
    {
        float3 _354 = fast::clamp(_385.xyz, float3(0.0), float3(1.0));
        _407 = float4(select(pow((_354 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _354 * float3(0.077399380505084991455078125), _354 <= float3(0.040449999272823333740234375)), _270);
    }
    else
    {
        _407 = _385;
    }
    out._entryPointOutput = _407;
    return out;
}

