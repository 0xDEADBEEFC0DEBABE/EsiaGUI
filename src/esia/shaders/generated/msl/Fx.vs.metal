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
    float4 _entryPointOutput_rect [[user(locn2)]];
    float4 _entryPointOutput_radii [[user(locn3)]];
    float4 _entryPointOutput_fill0 [[user(locn4)]];
    float4 _entryPointOutput_shape [[user(locn5)]];
    float4 _entryPointOutput_misc [[user(locn6)]];
    uint4 _entryPointOutput_flags [[user(locn7)]];
    float4 gl_Position [[position]];
};

vertex esia_main_out esia_main(constant WgtFrame& _275 [[buffer(0)]], constant WgtDraw& _46 [[buffer(2)]], const device gFxData& gFxData_1 [[buffer(10)]], uint gl_VertexIndex [[vertex_id]], uint gl_InstanceIndex [[instance_id]])
{
    esia_main_out out = {};
    uint _455 = gl_InstanceIndex + uint(_46.gDrawInfo.x);
    uint _652 = _455 * 24u;
    uint _662 = (_455 * 24u) + 6u;
    uint _670 = (_455 * 24u) + 8u;
    uint _686 = (_455 * 24u) + 15u;
    uint4 _470 = uint4(gFxData_1._data[(_455 * 24u) + 23u]);
    uint _472 = _470.x;
    float2 _838;
    float2 _842;
    if ((_472 & 512u) != 0u)
    {
        _842 = fast::max(gFxData_1._data[_652].zw, gFxData_1._data[_686].zw);
        _838 = fast::min(gFxData_1._data[_652].xy, gFxData_1._data[_686].xy);
    }
    else
    {
        _842 = gFxData_1._data[_652].zw;
        _838 = gFxData_1._data[_652].xy;
    }
    float _832;
    if ((_472 & 2u) != 0u)
    {
        _832 = fast::max(2.0, (gFxData_1._data[_662].x * gFxData_1._data[_662].y) + 2.0);
    }
    else
    {
        _832 = 2.0;
    }
    float _833;
    if (((_472 & 4u) != 0u) && (!((_472 & 256u) != 0u)))
    {
        _833 = fast::max(_832, (((gFxData_1._data[_670].x * 1.60000002384185791015625) + fast::max(gFxData_1._data[_670].y, 0.0)) + fast::max(abs(gFxData_1._data[_670].z), abs(gFxData_1._data[_670].w))) + 2.0);
    }
    else
    {
        _833 = _832;
    }
    float _834;
    if ((_472 & 8u) != 0u)
    {
        _834 = fast::max(_833, (gFxData_1._data[(_455 * 24u) + 10u].x * 1.7999999523162841796875) + 2.0);
    }
    else
    {
        _834 = _833;
    }
    float2 _545 = _838 - float2(_834);
    float2 _549 = _842 + float2(_834);
    float2 _843;
    float2 _844;
    if ((_472 & 1024u) != 0u)
    {
        uint _702 = (_455 * 24u) + 19u;
        _844 = fast::min(_549, gFxData_1._data[_702].zw + float2(1.0));
        _843 = fast::max(_545, gFxData_1._data[_702].xy - float2(1.0));
    }
    else
    {
        _844 = _549;
        _843 = _545;
    }
    float2 _845;
    float2 _846;
    if (((_472 & 8192u) != 0u) && (!(((_472 & 4u) != 0u) && (!((_472 & 256u) != 0u)))))
    {
        uint _710 = (_455 * 24u) + 22u;
        _846 = fast::max(_843, gFxData_1._data[_710].xy - float2(1.0));
        _845 = fast::min(_844, gFxData_1._data[_710].zw + float2(1.0));
    }
    else
    {
        _846 = _843;
        _845 = _844;
    }
    float2 _616 = float2(((gl_VertexIndex & 1u) != 0u) ? _845.x : _846.x, ((gl_VertexIndex & 2u) != 0u) ? _845.y : _846.y);
    out.gl_Position = float4((_616 * _275.gXform.xy) + _275.gXform.zw, 0.0, 1.0);
    out._entryPointOutput_local = _616;
    out._entryPointOutput_instance = _455;
    out._entryPointOutput_rect = gFxData_1._data[_652];
    out._entryPointOutput_radii = gFxData_1._data[(_455 * 24u) + 1u];
    out._entryPointOutput_fill0 = gFxData_1._data[(_455 * 24u) + 2u];
    out._entryPointOutput_shape = gFxData_1._data[(_455 * 24u) + 14u];
    out._entryPointOutput_misc = gFxData_1._data[(_455 * 24u) + 17u];
    out._entryPointOutput_flags = _470;
    return out;
}

