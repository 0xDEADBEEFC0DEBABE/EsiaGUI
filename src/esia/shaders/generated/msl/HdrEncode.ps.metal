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

struct WgtPass
{
    float4 gPass0;
    float4 gPass1;
};

struct esia_main_out
{
    float4 _entryPointOutput [[color(0)]];
};

fragment esia_main_out esia_main(constant WgtFrame& _45 [[buffer(0)]], constant WgtPass& _197 [[buffer(1)]], texture2d<float> gTex [[texture(3)]], sampler gPoint [[sampler(12)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float2 _305 = gl_FragCoord.xy + _45.gConv.yy;
    float2 _411;
    if (_45.gConv.x > 0.5)
    {
        float2 _390 = _305;
        _390.y = _45.gTarget.y - _305.y;
        _411 = _390;
    }
    else
    {
        _411 = _305;
    }
    float2 _267 = _411 * _45.gTarget.zw;
    float2 _412;
    if (_45.gConv.x > 0.5)
    {
        float2 _393 = _267;
        _393.y = 1.0 - _267.y;
        _412 = _393;
    }
    else
    {
        _412 = _267;
    }
    float4 _273 = gTex.sample(gPoint, _412, level(0.0));
    float3 _275 = _273.xyz;
    float3 _335 = abs(_275);
    float3 _279 = (sign(_275) * select(pow((_335 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _335 * float3(0.077399380505084991455078125), _335 <= float3(0.040449999272823333740234375))) * _197.gPass0.x;
    float3 _369 = pow(fast::clamp(float3(dot(_279, float3(0.627399981021881103515625, 0.329299986362457275390625, 0.0432999990880489349365234375)), dot(_279, float3(0.069099999964237213134765625, 0.91949999332427978515625, 0.011400000192224979400634765625)), dot(_279, float3(0.01640000008046627044677734375, 0.087999999523162841796875, 0.895600020885467529296875))) * float3(9.9999997473787516355514526367188e-05), float3(0.0), float3(1.0)), float3(0.1593017578125));
    out._entryPointOutput = float4(select(_279 * float3(0.012500000186264514923095703125), pow((float3(0.8359375) + (_369 * 18.8515625)) / (float3(1.0) + (_369 * 18.6875)), float3(78.84375)), bool3(_197.gPass0.y > 1.5)), _273.w);
    return out;
}

