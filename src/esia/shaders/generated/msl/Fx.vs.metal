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
constant uint _109 = (512u & kEsiaFxFeatures);
constant uint _128 = (2u & kEsiaFxFeatures);
constant uint _144 = (4u & kEsiaFxFeatures);
constant uint _149 = (256u & kEsiaFxFeatures);
constant uint _178 = (8u & kEsiaFxFeatures);
constant uint _200 = (1024u & kEsiaFxFeatures);
constant uint _226 = (8192u & kEsiaFxFeatures);
constant uint _230 = (4u & kEsiaFxFeatures);
constant uint _234 = (256u & kEsiaFxFeatures);

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

vertex esia_main_out esia_main(constant WgtFrame& _285 [[buffer(0)]], constant WgtDraw& _46 [[buffer(2)]], const device gFxData& gFxData_1 [[buffer(10)]], uint gl_VertexIndex [[vertex_id]], uint gl_InstanceIndex [[instance_id]])
{
    esia_main_out out = {};
    uint _465 = gl_InstanceIndex + uint(_46.gDrawInfo.x);
    uint _662 = _465 * 24u;
    uint _672 = (_465 * 24u) + 6u;
    uint _680 = (_465 * 24u) + 8u;
    uint _696 = (_465 * 24u) + 15u;
    uint4 _480 = uint4(gFxData_1._data[(_465 * 24u) + 23u]);
    uint _482 = _480.x;
    float2 _848;
    float2 _852;
    if ((_482 & _109) != 0u)
    {
        _852 = fast::max(gFxData_1._data[_662].zw, gFxData_1._data[_696].zw);
        _848 = fast::min(gFxData_1._data[_662].xy, gFxData_1._data[_696].xy);
    }
    else
    {
        _852 = gFxData_1._data[_662].zw;
        _848 = gFxData_1._data[_662].xy;
    }
    float _842;
    if ((_482 & _128) != 0u)
    {
        _842 = fast::max(2.0, (gFxData_1._data[_672].x * gFxData_1._data[_672].y) + 2.0);
    }
    else
    {
        _842 = 2.0;
    }
    float _843;
    if (((_482 & _144) != 0u) && (!((_482 & _149) != 0u)))
    {
        _843 = fast::max(_842, (((gFxData_1._data[_680].x * 1.60000002384185791015625) + fast::max(gFxData_1._data[_680].y, 0.0)) + fast::max(abs(gFxData_1._data[_680].z), abs(gFxData_1._data[_680].w))) + 2.0);
    }
    else
    {
        _843 = _842;
    }
    float _844;
    if ((_482 & _178) != 0u)
    {
        _844 = fast::max(_843, (gFxData_1._data[(_465 * 24u) + 10u].x * 1.7999999523162841796875) + 2.0);
    }
    else
    {
        _844 = _843;
    }
    float2 _555 = _848 - float2(_844);
    float2 _559 = _852 + float2(_844);
    float2 _853;
    float2 _854;
    if ((_482 & _200) != 0u)
    {
        uint _712 = (_465 * 24u) + 19u;
        _854 = fast::min(_559, gFxData_1._data[_712].zw + float2(1.0));
        _853 = fast::max(_555, gFxData_1._data[_712].xy - float2(1.0));
    }
    else
    {
        _854 = _559;
        _853 = _555;
    }
    float2 _855;
    float2 _856;
    if (((_482 & _226) != 0u) && (!(((_482 & _230) != 0u) && (!((_482 & _234) != 0u)))))
    {
        uint _720 = (_465 * 24u) + 22u;
        _856 = fast::max(_853, gFxData_1._data[_720].xy - float2(1.0));
        _855 = fast::min(_854, gFxData_1._data[_720].zw + float2(1.0));
    }
    else
    {
        _856 = _853;
        _855 = _854;
    }
    float2 _626 = float2(((gl_VertexIndex & 1u) != 0u) ? _855.x : _856.x, ((gl_VertexIndex & 2u) != 0u) ? _855.y : _856.y);
    out.gl_Position = float4((_626 * _285.gXform.xy) + _285.gXform.zw, 0.0, 1.0);
    out._entryPointOutput_local = _626;
    out._entryPointOutput_instance = _465;
    out._entryPointOutput_rect = gFxData_1._data[_662];
    out._entryPointOutput_radii = gFxData_1._data[(_465 * 24u) + 1u];
    out._entryPointOutput_fill0 = gFxData_1._data[(_465 * 24u) + 2u];
    out._entryPointOutput_shape = gFxData_1._data[(_465 * 24u) + 14u];
    out._entryPointOutput_misc = gFxData_1._data[(_465 * 24u) + 17u];
    out._entryPointOutput_flags = _480;
    return out;
}

