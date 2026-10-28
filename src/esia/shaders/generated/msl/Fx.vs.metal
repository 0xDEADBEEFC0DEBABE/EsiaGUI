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
constant uint _357 = (1024u & kEsiaFxFeatures);
constant uint _363 = (8192u & kEsiaFxFeatures);
constant uint _369 = (131072u & kEsiaFxFeatures);

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

vertex esia_main_out esia_main(constant WgtFrame& _445 [[buffer(0)]], constant WgtDraw& _51 [[buffer(2)]], const device gFxData& gFxData_1 [[buffer(10)]], uint gl_VertexIndex [[vertex_id]], uint gl_InstanceIndex [[instance_id]])
{
    esia_main_out out = {};
    uint _646 = gl_InstanceIndex + uint(_51.gDrawInfo.x);
    uint _957 = _646 * 24u;
    uint _967 = (_646 * 24u) + 6u;
    uint _975 = (_646 * 24u) + 8u;
    uint _991 = (_646 * 24u) + 15u;
    uint4 _661 = uint4(gFxData_1._data[(_646 * 24u) + 23u]);
    uint _663 = _661.x;
    float2 _1185;
    float2 _1187;
    if ((_663 & _139) != 0u)
    {
        _1187 = fast::max(gFxData_1._data[_957].zw, gFxData_1._data[_991].zw);
        _1185 = fast::min(gFxData_1._data[_957].xy, gFxData_1._data[_991].xy);
    }
    else
    {
        _1187 = gFxData_1._data[_957].zw;
        _1185 = gFxData_1._data[_957].xy;
    }
    float2 _1184;
    float2 _1186;
    if ((_663 & _154) != 0u)
    {
        _1186 = gFxData_1._data[_991].zw;
        _1184 = gFxData_1._data[_991].xy;
    }
    else
    {
        _1186 = _1187;
        _1184 = _1185;
    }
    float2 _1196;
    float2 _1201;
    if ((_663 & _165) != 0u)
    {
        float2 _698 = abs(gFxData_1._data[(_646 * 24u) + 21u].xy) * 0.5;
        _1201 = _1186 + _698;
        _1196 = _1184 - _698;
    }
    else
    {
        _1201 = _1186;
        _1196 = _1184;
    }
    float _1188;
    if ((_663 & _189) != 0u)
    {
        _1188 = fast::max(2.0, (gFxData_1._data[_967].x * gFxData_1._data[_967].y) + 2.0);
    }
    else
    {
        _1188 = 2.0;
    }
    float _1189;
    if (((_663 & _204) != 0u) && (!((_663 & _209) != 0u)))
    {
        _1189 = fast::max(_1188, (((gFxData_1._data[_975].x * 1.60000002384185791015625) + fast::max(gFxData_1._data[_975].y, 0.0)) + ((_51.gDrawInfo.w > 0.5) ? length(gFxData_1._data[_975].zw) : fast::max(abs(gFxData_1._data[_975].z), abs(gFxData_1._data[_975].w)))) + 2.0);
    }
    else
    {
        _1189 = _1188;
    }
    float _1190;
    if ((_663 & _234) != 0u)
    {
        _1190 = fast::max(_1189, (gFxData_1._data[(_646 * 24u) + 10u].x * 1.7999999523162841796875) + 2.0);
    }
    else
    {
        _1190 = _1189;
    }
    float _1191;
    if ((_663 & _248) != 0u)
    {
        _1191 = fast::max(_1190, (gFxData_1._data[(_646 * 24u) + 1u].w * 1.7999999523162841796875) + 2.0);
    }
    else
    {
        _1191 = _1190;
    }
    float2 _769 = _1196 - float2(_1191);
    float2 _773 = _1201 + float2(_1191);
    float2 _1202;
    float2 _1203;
    if ((_663 & _272) != 0u)
    {
        uint _1042 = (_646 * 24u) + 19u;
        _1203 = fast::min(_773, gFxData_1._data[_1042].zw + float2(1.0));
        _1202 = fast::max(_769, gFxData_1._data[_1042].xy - float2(1.0));
    }
    else
    {
        _1203 = _773;
        _1202 = _769;
    }
    float2 _1204;
    float2 _1205;
    if (((_663 & _298) != 0u) && (!(((_663 & _302) != 0u) && (!((_663 & _306) != 0u)))))
    {
        uint _1050 = (_646 * 24u) + 22u;
        _1205 = fast::max(_1202, gFxData_1._data[_1050].xy - float2(1.0));
        _1204 = fast::min(_1203, gFxData_1._data[_1050].zw + float2(1.0));
    }
    else
    {
        _1205 = _1202;
        _1204 = _1203;
    }
    float2 _1208;
    if ((((_661.z == 2u) && (!((_663 & _357) != 0u))) && (!((_663 & _363) != 0u))) && (!((_663 & _369) != 0u)))
    {
        float2 _864 = gFxData_1._data[_991].zw - gFxData_1._data[_991].xy;
        float _866 = length(_864);
        float2 _874 = select(float2(1.0, 0.0), _864 / float2(_866), bool2(_866 > 9.9999997473787516355514526367188e-05));
        float _885 = gFxData_1._data[(_646 * 24u) + 1u].x + _1191;
        _1208 = select(gFxData_1._data[_991].xy - (_874 * _885), gFxData_1._data[_991].zw + (_874 * _885), bool2((gl_VertexIndex & 1u) != 0u)) + (float2(-_874.y, _874.x) * (((gl_VertexIndex & 2u) != 0u) ? _885 : (-_885)));
    }
    else
    {
        _1208 = float2(((gl_VertexIndex & 1u) != 0u) ? _1204.x : _1205.x, ((gl_VertexIndex & 2u) != 0u) ? _1204.y : _1205.y);
    }
    out.gl_Position = select(float4((_1208 * _445.gXform.xy) + _445.gXform.zw, 0.0, 1.0), float4(-2.0, -2.0, 0.0, 1.0), bool4(_661.z == 5u));
    out._entryPointOutput_local = _1208;
    out._entryPointOutput_instance = _646;
    out._entryPointOutput_rect = gFxData_1._data[_957];
    out._entryPointOutput_radii = gFxData_1._data[(_646 * 24u) + 1u];
    out._entryPointOutput_fill0 = gFxData_1._data[(_646 * 24u) + 2u];
    out._entryPointOutput_shape = gFxData_1._data[(_646 * 24u) + 14u];
    out._entryPointOutput_misc = gFxData_1._data[(_646 * 24u) + 17u];
    out._entryPointOutput_flags = _661;
    return out;
}

