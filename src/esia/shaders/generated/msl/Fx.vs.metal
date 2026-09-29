#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct gFxData
{
    float4 _data[1];
};

struct WgtDraw
{
    float4 gFade;
    float4 gDrawInfo;
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

struct esia_main_out
{
    float2 _entryPointOutput_local [[user(locn0)]];
    uint _entryPointOutput_instance [[user(locn1)]];
    float4 gl_Position [[position]];
};

vertex esia_main_out esia_main(constant WgtFrame& _271 [[buffer(0)]], constant WgtDraw& _45 [[buffer(2)]], const device gFxData& gFxData_1 [[buffer(10)]], uint gl_VertexIndex [[vertex_id]], uint gl_InstanceIndex [[instance_id]])
{
    esia_main_out out = {};
    uint _391 = gl_InstanceIndex + uint(_45.gDrawInfo.x);
    uint _571 = _391 * 24u;
    uint _581 = (_391 * 24u) + 6u;
    uint _589 = (_391 * 24u) + 8u;
    uint _605 = (_391 * 24u) + 15u;
    uint _407 = uint(gFxData_1._data[(_391 * 24u) + 23u].x);
    float2 _682;
    float2 _686;
    if ((_407 & 512u) != 0u)
    {
        _686 = fast::max(gFxData_1._data[_571].zw, gFxData_1._data[_605].zw);
        _682 = fast::min(gFxData_1._data[_571].xy, gFxData_1._data[_605].xy);
    }
    else
    {
        _686 = gFxData_1._data[_571].zw;
        _682 = gFxData_1._data[_571].xy;
    }
    float _676;
    if ((_407 & 2u) != 0u)
    {
        _676 = fast::max(2.0, (gFxData_1._data[_581].x * gFxData_1._data[_581].y) + 2.0);
    }
    else
    {
        _676 = 2.0;
    }
    float _677;
    if (((_407 & 4u) != 0u) && (!((_407 & 256u) != 0u)))
    {
        _677 = fast::max(_676, (((gFxData_1._data[_589].x * 1.60000002384185791015625) + fast::max(gFxData_1._data[_589].y, 0.0)) + fast::max(abs(gFxData_1._data[_589].z), abs(gFxData_1._data[_589].w))) + 2.0);
    }
    else
    {
        _677 = _676;
    }
    float _678;
    if ((_407 & 8u) != 0u)
    {
        _678 = fast::max(_677, (gFxData_1._data[(_391 * 24u) + 10u].x * 1.7999999523162841796875) + 2.0);
    }
    else
    {
        _678 = _677;
    }
    float2 _480 = _682 - float2(_678);
    float2 _484 = _686 + float2(_678);
    float2 _687;
    float2 _688;
    if ((_407 & 1024u) != 0u)
    {
        uint _621 = (_391 * 24u) + 19u;
        _688 = fast::min(_484, gFxData_1._data[_621].zw + float2(1.0));
        _687 = fast::max(_480, gFxData_1._data[_621].xy - float2(1.0));
    }
    else
    {
        _688 = _484;
        _687 = _480;
    }
    float2 _689;
    float2 _690;
    if (((_407 & 8192u) != 0u) && (!(((_407 & 4u) != 0u) && (!((_407 & 256u) != 0u)))))
    {
        uint _629 = (_391 * 24u) + 22u;
        _690 = fast::max(_687, gFxData_1._data[_629].xy - float2(1.0));
        _689 = fast::min(_688, gFxData_1._data[_629].zw + float2(1.0));
    }
    else
    {
        _690 = _687;
        _689 = _688;
    }
    float2 _551 = float2(((gl_VertexIndex & 1u) != 0u) ? _689.x : _690.x, ((gl_VertexIndex & 2u) != 0u) ? _689.y : _690.y);
    out.gl_Position = float4((_551 * _271.gXform.xy) + _271.gXform.zw, 0.0, 1.0);
    out._entryPointOutput_local = _551;
    out._entryPointOutput_instance = _391;
    return out;
}

