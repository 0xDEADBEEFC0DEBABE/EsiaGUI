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

constant uint kEsiaFxFeatures_tmp [[function_constant(0)]];
constant uint kEsiaFxFeatures = is_function_constant_defined(kEsiaFxFeatures_tmp) ? kEsiaFxFeatures_tmp : 4294967295u;
constant uint _139 = (512u & kEsiaFxFeatures);
constant uint _154 = (32768u & kEsiaFxFeatures);
constant uint _165 = (131072u & kEsiaFxFeatures);
constant uint _189 = (2u & kEsiaFxFeatures);
constant uint _204 = (4u & kEsiaFxFeatures);
constant uint _209 = (256u & kEsiaFxFeatures);
constant uint _234 = (8u & kEsiaFxFeatures);
constant uint _248 = (65536u & kEsiaFxFeatures);
constant uint _272 = (1024u & kEsiaFxFeatures);
constant uint _298 = (8192u & kEsiaFxFeatures);
constant uint _302 = (4u & kEsiaFxFeatures);
constant uint _306 = (256u & kEsiaFxFeatures);

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

vertex esia_main_out esia_main(constant WgtFrame& _357 [[buffer(0)]], constant WgtDraw& _51 [[buffer(2)]], const device gFxData& gFxData_1 [[buffer(10)]], uint gl_VertexIndex [[vertex_id]], uint gl_InstanceIndex [[instance_id]])
{
    esia_main_out out = {};
    uint _542 = gl_InstanceIndex + uint(_51.gDrawInfo.x);
    uint _772 = _542 * 24u;
    uint _782 = (_542 * 24u) + 6u;
    uint _790 = (_542 * 24u) + 8u;
    uint _806 = (_542 * 24u) + 15u;
    uint4 _557 = uint4(gFxData_1._data[(_542 * 24u) + 23u]);
    uint _559 = _557.x;
    float2 _988;
    float2 _990;
    if ((_559 & _139) != 0u)
    {
        _990 = fast::max(gFxData_1._data[_772].zw, gFxData_1._data[_806].zw);
        _988 = fast::min(gFxData_1._data[_772].xy, gFxData_1._data[_806].xy);
    }
    else
    {
        _990 = gFxData_1._data[_772].zw;
        _988 = gFxData_1._data[_772].xy;
    }
    float2 _987;
    float2 _989;
    if ((_559 & _154) != 0u)
    {
        _989 = gFxData_1._data[_806].zw;
        _987 = gFxData_1._data[_806].xy;
    }
    else
    {
        _989 = _990;
        _987 = _988;
    }
    float2 _999;
    float2 _1004;
    if ((_559 & _165) != 0u)
    {
        float2 _594 = abs(gFxData_1._data[(_542 * 24u) + 21u].xy) * 0.5;
        _1004 = _989 + _594;
        _999 = _987 - _594;
    }
    else
    {
        _1004 = _989;
        _999 = _987;
    }
    float _991;
    if ((_559 & _189) != 0u)
    {
        _991 = fast::max(2.0, (gFxData_1._data[_782].x * gFxData_1._data[_782].y) + 2.0);
    }
    else
    {
        _991 = 2.0;
    }
    float _992;
    if (((_559 & _204) != 0u) && (!((_559 & _209) != 0u)))
    {
        _992 = fast::max(_991, (((gFxData_1._data[_790].x * 1.60000002384185791015625) + fast::max(gFxData_1._data[_790].y, 0.0)) + ((_51.gDrawInfo.w > 0.5) ? length(gFxData_1._data[_790].zw) : fast::max(abs(gFxData_1._data[_790].z), abs(gFxData_1._data[_790].w)))) + 2.0);
    }
    else
    {
        _992 = _991;
    }
    float _993;
    if ((_559 & _234) != 0u)
    {
        _993 = fast::max(_992, (gFxData_1._data[(_542 * 24u) + 10u].x * 1.7999999523162841796875) + 2.0);
    }
    else
    {
        _993 = _992;
    }
    float _994;
    if ((_559 & _248) != 0u)
    {
        _994 = fast::max(_993, (gFxData_1._data[(_542 * 24u) + 1u].w * 1.7999999523162841796875) + 2.0);
    }
    else
    {
        _994 = _993;
    }
    float2 _665 = _999 - float2(_994);
    float2 _669 = _1004 + float2(_994);
    float2 _1005;
    float2 _1006;
    if ((_559 & _272) != 0u)
    {
        uint _857 = (_542 * 24u) + 19u;
        _1006 = fast::min(_669, gFxData_1._data[_857].zw + float2(1.0));
        _1005 = fast::max(_665, gFxData_1._data[_857].xy - float2(1.0));
    }
    else
    {
        _1006 = _669;
        _1005 = _665;
    }
    float2 _1007;
    float2 _1008;
    if (((_559 & _298) != 0u) && (!(((_559 & _302) != 0u) && (!((_559 & _306) != 0u)))))
    {
        uint _865 = (_542 * 24u) + 22u;
        _1008 = fast::max(_1005, gFxData_1._data[_865].xy - float2(1.0));
        _1007 = fast::min(_1006, gFxData_1._data[_865].zw + float2(1.0));
    }
    else
    {
        _1008 = _1005;
        _1007 = _1006;
    }
    float2 _736 = float2(((gl_VertexIndex & 1u) != 0u) ? _1007.x : _1008.x, ((gl_VertexIndex & 2u) != 0u) ? _1007.y : _1008.y);
    out.gl_Position = float4((_736 * _357.gXform.xy) + _357.gXform.zw, 0.0, 1.0);
    out._entryPointOutput_local = _736;
    out._entryPointOutput_instance = _542;
    out._entryPointOutput_rect = gFxData_1._data[_772];
    out._entryPointOutput_radii = gFxData_1._data[(_542 * 24u) + 1u];
    out._entryPointOutput_fill0 = gFxData_1._data[(_542 * 24u) + 2u];
    out._entryPointOutput_shape = gFxData_1._data[(_542 * 24u) + 14u];
    out._entryPointOutput_misc = gFxData_1._data[(_542 * 24u) + 17u];
    out._entryPointOutput_flags = _557;
    return out;
}

