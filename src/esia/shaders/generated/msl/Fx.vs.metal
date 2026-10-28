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
    uint _637 = gl_InstanceIndex + uint(_51.gDrawInfo.x);
    uint _942 = _637 * 24u;
    uint _952 = (_637 * 24u) + 6u;
    uint _960 = (_637 * 24u) + 8u;
    uint _976 = (_637 * 24u) + 15u;
    uint4 _652 = uint4(gFxData_1._data[(_637 * 24u) + 23u]);
    uint _654 = _652.x;
    float2 _1169;
    float2 _1171;
    if ((_654 & _139) != 0u)
    {
        _1171 = fast::max(gFxData_1._data[_942].zw, gFxData_1._data[_976].zw);
        _1169 = fast::min(gFxData_1._data[_942].xy, gFxData_1._data[_976].xy);
    }
    else
    {
        _1171 = gFxData_1._data[_942].zw;
        _1169 = gFxData_1._data[_942].xy;
    }
    float2 _1168;
    float2 _1170;
    if ((_654 & _154) != 0u)
    {
        _1170 = gFxData_1._data[_976].zw;
        _1168 = gFxData_1._data[_976].xy;
    }
    else
    {
        _1170 = _1171;
        _1168 = _1169;
    }
    float2 _1180;
    float2 _1185;
    if ((_654 & _165) != 0u)
    {
        float2 _689 = abs(gFxData_1._data[(_637 * 24u) + 21u].xy) * 0.5;
        _1185 = _1170 + _689;
        _1180 = _1168 - _689;
    }
    else
    {
        _1185 = _1170;
        _1180 = _1168;
    }
    float _1172;
    if ((_654 & _189) != 0u)
    {
        _1172 = fast::max(2.0, (gFxData_1._data[_952].x * gFxData_1._data[_952].y) + 2.0);
    }
    else
    {
        _1172 = 2.0;
    }
    float _1173;
    if (((_654 & _204) != 0u) && (!((_654 & _209) != 0u)))
    {
        _1173 = fast::max(_1172, (((gFxData_1._data[_960].x * 1.60000002384185791015625) + fast::max(gFxData_1._data[_960].y, 0.0)) + ((_51.gDrawInfo.w > 0.5) ? length(gFxData_1._data[_960].zw) : fast::max(abs(gFxData_1._data[_960].z), abs(gFxData_1._data[_960].w)))) + 2.0);
    }
    else
    {
        _1173 = _1172;
    }
    float _1174;
    if ((_654 & _234) != 0u)
    {
        _1174 = fast::max(_1173, (gFxData_1._data[(_637 * 24u) + 10u].x * 1.7999999523162841796875) + 2.0);
    }
    else
    {
        _1174 = _1173;
    }
    float _1175;
    if ((_654 & _248) != 0u)
    {
        _1175 = fast::max(_1174, (gFxData_1._data[(_637 * 24u) + 1u].w * 1.7999999523162841796875) + 2.0);
    }
    else
    {
        _1175 = _1174;
    }
    float2 _760 = _1180 - float2(_1175);
    float2 _764 = _1185 + float2(_1175);
    float2 _1186;
    float2 _1187;
    if ((_654 & _272) != 0u)
    {
        uint _1027 = (_637 * 24u) + 19u;
        _1187 = fast::min(_764, gFxData_1._data[_1027].zw + float2(1.0));
        _1186 = fast::max(_760, gFxData_1._data[_1027].xy - float2(1.0));
    }
    else
    {
        _1187 = _764;
        _1186 = _760;
    }
    float2 _1188;
    float2 _1189;
    if (((_654 & _298) != 0u) && (!(((_654 & _302) != 0u) && (!((_654 & _306) != 0u)))))
    {
        uint _1035 = (_637 * 24u) + 22u;
        _1189 = fast::max(_1186, gFxData_1._data[_1035].xy - float2(1.0));
        _1188 = fast::min(_1187, gFxData_1._data[_1035].zw + float2(1.0));
    }
    else
    {
        _1189 = _1186;
        _1188 = _1187;
    }
    float2 _1192;
    if ((((_652.z == 2u) && (!((_654 & _357) != 0u))) && (!((_654 & _363) != 0u))) && (!((_654 & _369) != 0u)))
    {
        float2 _855 = gFxData_1._data[_976].zw - gFxData_1._data[_976].xy;
        float _857 = length(_855);
        float2 _865 = select(float2(1.0, 0.0), _855 / float2(_857), bool2(_857 > 9.9999997473787516355514526367188e-05));
        float _876 = gFxData_1._data[(_637 * 24u) + 1u].x + _1175;
        _1192 = select(gFxData_1._data[_976].xy - (_865 * _876), gFxData_1._data[_976].zw + (_865 * _876), bool2((gl_VertexIndex & 1u) != 0u)) + (float2(-_865.y, _865.x) * (((gl_VertexIndex & 2u) != 0u) ? _876 : (-_876)));
    }
    else
    {
        _1192 = float2(((gl_VertexIndex & 1u) != 0u) ? _1188.x : _1189.x, ((gl_VertexIndex & 2u) != 0u) ? _1188.y : _1189.y);
    }
    out.gl_Position = float4((_1192 * _445.gXform.xy) + _445.gXform.zw, 0.0, 1.0);
    out._entryPointOutput_local = _1192;
    out._entryPointOutput_instance = _637;
    out._entryPointOutput_rect = gFxData_1._data[_942];
    out._entryPointOutput_radii = gFxData_1._data[(_637 * 24u) + 1u];
    out._entryPointOutput_fill0 = gFxData_1._data[(_637 * 24u) + 2u];
    out._entryPointOutput_shape = gFxData_1._data[(_637 * 24u) + 14u];
    out._entryPointOutput_misc = gFxData_1._data[(_637 * 24u) + 17u];
    out._entryPointOutput_flags = _652;
    return out;
}

