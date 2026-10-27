#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct gFxData
{
    float4 _data[1];
};

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

struct esia_main_out
{
    float4 _entryPointOutput_col [[user(locn0)]];
    float2 _entryPointOutput_uv [[user(locn1)]];
    float4 gl_Position [[position]];
};

vertex esia_main_out esia_main(constant WgtFrame& _33 [[buffer(0)]], constant WgtDraw& _57 [[buffer(2)]], const device gFxData& gFxData_1 [[buffer(10)]], uint gl_VertexIndex [[vertex_id]], uint gl_InstanceIndex [[instance_id]])
{
    esia_main_out out = {};
    uint _261 = gl_InstanceIndex + uint(_57.gDrawInfo.x);
    uint _368 = uint(_33.gConv.w) + (_261 * 2u);
    uint _382 = (uint(_33.gConv.w) + (_261 * 2u)) + 1u;
    float _269 = floor(gFxData_1._data[_382].x * 0.000244140625);
    float _278 = floor(gFxData_1._data[_382].y * 0.000244140625);
    float _287 = floor(gFxData_1._data[_382].z * 1.52587890625e-05);
    float _294 = floor((gFxData_1._data[_382].z - (_287 * 65536.0)) * 0.00390625);
    bool _306 = (gl_VertexIndex & 1u) != 0u;
    bool _310 = (gl_VertexIndex & 2u) != 0u;
    out.gl_Position = float4((float2(_306 ? gFxData_1._data[_368].z : gFxData_1._data[_368].x, _310 ? gFxData_1._data[_368].w : gFxData_1._data[_368].y) * _33.gXform.xy) + _33.gXform.zw, 0.0, 1.0);
    out._entryPointOutput_col = float4((gFxData_1._data[_382].z - (_287 * 65536.0)) - (_294 * 256.0), _294, _287, gFxData_1._data[_382].w) * float4(0.0039215688593685626983642578125);
    out._entryPointOutput_uv = float2((gFxData_1._data[_382].x - (_269 * 4096.0)) + (_306 ? (gFxData_1._data[_382].y - (_278 * 4096.0)) : 0.0), _269 + (_310 ? _278 : 0.0)) * float2(0.000244140625);
    return out;
}

