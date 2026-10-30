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

vertex esia_main_out esia_main(constant WgtFrame& _46 [[buffer(0)]], constant WgtDraw& _147 [[buffer(2)]], const device gFxData& gFxData_1 [[buffer(10)]], uint gl_VertexIndex [[vertex_id]], uint gl_InstanceIndex [[instance_id]])
{
    esia_main_out out = {};
    uint _282 = gl_InstanceIndex + uint(_147.gDrawInfo.x);
    uint _337 = uint(_46.gConv.w) + (_282 * 2u);
    uint _351 = (uint(_46.gConv.w) + (_282 * 2u)) + 1u;
    bool _290 = (gl_VertexIndex & 1u) != 0u;
    bool _294 = (gl_VertexIndex & 2u) != 0u;
    float _362 = floor(gFxData_1._data[_351].z * 1.52587890625e-05);
    float _369 = floor((gFxData_1._data[_351].z - (_362 * 65536.0)) * 0.00390625);
    float _395 = floor(gFxData_1._data[_351].x * 0.000244140625);
    float _404 = floor(gFxData_1._data[_351].y * 0.000244140625);
    out.gl_Position = float4((float2(_290 ? gFxData_1._data[_337].z : gFxData_1._data[_337].x, _294 ? gFxData_1._data[_337].w : gFxData_1._data[_337].y) * _46.gXform.xy) + _46.gXform.zw, 0.0, 1.0);
    out._entryPointOutput_col = float4((gFxData_1._data[_351].z - (_362 * 65536.0)) - (_369 * 256.0), _369, _362, gFxData_1._data[_351].w) * float4(0.0039215688593685626983642578125);
    out._entryPointOutput_uv = float2((gFxData_1._data[_351].x - (_395 * 4096.0)) + (_290 ? (gFxData_1._data[_351].y - (_404 * 4096.0)) : 0.0), _395 + (_294 ? _404 : 0.0)) * float2(0.000244140625);
    return out;
}

