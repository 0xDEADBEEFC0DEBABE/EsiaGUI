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

constant float2 _385 = {};

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
    float4 _237 = gTex.sample(gLinear, in.i_uv);
    float4 _238 = in.i_col * _237;
    float2 _258 = gl_FragCoord.xy + _44.gConv.yy;
    float2 _377;
    if (_44.gConv.x > 0.5)
    {
        float2 _358;
        _358.y = _44.gTarget.y - _258.y;
        _377 = _358;
    }
    else
    {
        _377 = _258;
    }
    float _378;
    if (_76.gFade.z > 0.0)
    {
        _378 = smoothstep(0.0, 1.0, fast::clamp((_377.y - _76.gFade.x) / _76.gFade.z, 0.0, 1.0));
    }
    else
    {
        _378 = 1.0;
    }
    float _379;
    if (_76.gFade.w > 0.0)
    {
        _379 = _378 * smoothstep(0.0, 1.0, fast::clamp((_76.gFade.y - _377.y) / _76.gFade.w, 0.0, 1.0));
    }
    else
    {
        _379 = _378;
    }
    float _246 = _238.w * _379;
    float4 _361 = _238;
    _361.w = _246;
    float4 _383;
    if (_44.gTime.w > 0.5)
    {
        float3 _330 = fast::clamp(_361.xyz, float3(0.0), float3(1.0));
        _383 = float4(select(pow((_330 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _330 * float3(0.077399380505084991455078125), _330 <= float3(0.040449999272823333740234375)), _246);
    }
    else
    {
        _383 = _361;
    }
    out._entryPointOutput = _383;
    return out;
}

