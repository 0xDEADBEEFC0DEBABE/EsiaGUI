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

fragment esia_main_out esia_main(constant WgtFrame& _45 [[buffer(0)]], constant WgtPass& _191 [[buffer(1)]], texture2d<float> gTex [[texture(3)]], sampler gPoint [[sampler(12)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float2 _305 = gl_FragCoord.xy + _45.gConv.yy;
    float2 _396;
    if (_45.gConv.x > 0.5)
    {
        float2 _387 = _305;
        _387.y = _45.gTarget.y - _305.y;
        _396 = _387;
    }
    else
    {
        _396 = _305;
    }
    float2 _266 = _396 * _45.gTarget.zw;
    float2 _397;
    if (_45.gConv.x > 0.5)
    {
        float2 _390 = _266;
        _390.y = 1.0 - _266.y;
        _397 = _390;
    }
    else
    {
        _397 = _266;
    }
    float4 _272 = gTex.sample(gPoint, _397, level(0.0));
    float3 _338 = pow(fast::clamp(_272.xyz, float3(0.0), float3(1.0)), float3(0.0126833133399486541748046875));
    float3 _349 = pow(fast::max(_338 - float3(0.8359375), float3(0.0)) / (float3(18.8515625) - (_338 * 18.6875)), float3(6.277394771575927734375)) * 10000.0;
    float3 _289 = select(_272.xyz * 80.0, float3(dot(_349, float3(1.660500049591064453125, -0.5875999927520751953125, -0.072800002992153167724609375)), dot(_349, float3(-0.124600000679492950439453125, 1.1328999996185302734375, -0.008299999870359897613525390625)), dot(_349, float3(-0.01820000074803829193115234375, -0.100599996745586395263671875, 1.1187000274658203125))), bool3(_191.gPass0.y > 1.5)) / float3(_191.gPass0.x);
    float3 _363 = abs(_289);
    out._entryPointOutput = float4(sign(_289) * select((pow(_363, float3(0.4166666567325592041015625)) * 1.05499994754791259765625) - float3(0.054999999701976776123046875), _363 * 12.9200000762939453125, _363 <= float3(0.003130800090730190277099609375)), _272.w);
    return out;
}

