#version 330

#ifndef SPIRV_CROSS_CONSTANT_ID_0
#define SPIRV_CROSS_CONSTANT_ID_0 4294967295u
#endif
const uint kEsiaFxFeatures = SPIRV_CROSS_CONSTANT_ID_0;
const uint _178 = (512u & kEsiaFxFeatures);
const uint _193 = (32768u & kEsiaFxFeatures);
const uint _204 = (131072u & kEsiaFxFeatures);
const uint _227 = (2u & kEsiaFxFeatures);
const uint _242 = (4u & kEsiaFxFeatures);
const uint _247 = (256u & kEsiaFxFeatures);
const uint _272 = (8u & kEsiaFxFeatures);
const uint _286 = (65536u & kEsiaFxFeatures);
const uint _310 = (1024u & kEsiaFxFeatures);
const uint _336 = (8192u & kEsiaFxFeatures);
const uint _340 = (4u & kEsiaFxFeatures);
const uint _344 = (256u & kEsiaFxFeatures);
const uint _395 = (1024u & kEsiaFxFeatures);
const uint _401 = (8192u & kEsiaFxFeatures);
const uint _407 = (131072u & kEsiaFxFeatures);

layout(std140) uniform WgtFrame
{
    vec4 gXform;
    vec4 gTarget;
    vec4 gDisplay;
    vec4 gTime;
    vec4 gLevel[6];
    vec4 gText;
    vec4 gConv;
} _38;

layout(std140) uniform WgtDraw
{
    vec4 gFade;
    vec4 gDrawInfo;
} _93;

uniform sampler2D SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler;

out vec2 esia_v0;
flat out uint esia_v1;
flat out vec4 esia_v2;
flat out vec4 esia_v3;
flat out vec4 esia_v4;
flat out vec4 esia_v5;
flat out vec4 esia_v6;
flat out uvec4 esia_v7;

void main()
{
    uint _678 = uint(gl_InstanceID) + uint(_93.gDrawInfo.x);
    uint _992 = max(uint(_38.gConv.z), 1u);
    vec4 _1026 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int((_678 % _992) * 24u), int(_678 / _992), 0).xy, 0);
    uint _1033 = max(uint(_38.gConv.z), 1u);
    vec4 _1067 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_678 % _1033) * 24u) + 6u), int(_678 / _1033), 0).xy, 0);
    uint _1074 = max(uint(_38.gConv.z), 1u);
    vec4 _1108 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_678 % _1074) * 24u) + 8u), int(_678 / _1074), 0).xy, 0);
    uint _1115 = max(uint(_38.gConv.z), 1u);
    uint _1156 = max(uint(_38.gConv.z), 1u);
    vec4 _1190 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_678 % _1156) * 24u) + 15u), int(_678 / _1156), 0).xy, 0);
    uint _1197 = max(uint(_38.gConv.z), 1u);
    uvec4 _693 = uvec4(texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_678 % _1197) * 24u) + 23u), int(_678 / _1197), 0).xy, 0));
    uint _695 = _693.x;
    vec2 _697 = _1026.xy;
    vec2 _699 = _1026.zw;
    vec2 _1712;
    vec2 _1714;
    if ((_695 & _178) != 0u)
    {
        _1714 = max(_699, _1190.zw);
        _1712 = min(_697, _1190.xy);
    }
    else
    {
        _1714 = _699;
        _1712 = _697;
    }
    vec2 _1711;
    vec2 _1713;
    if ((_695 & _193) != 0u)
    {
        _1713 = _1190.zw;
        _1711 = _1190.xy;
    }
    else
    {
        _1713 = _1714;
        _1711 = _1712;
    }
    vec2 _1723;
    vec2 _1728;
    if ((_695 & _204) != 0u)
    {
        uint _1238 = max(uint(_38.gConv.z), 1u);
        vec2 _730 = abs(texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_678 % _1238) * 24u) + 21u), int(_678 / _1238), 0).xy, 0).xy) * 0.5;
        _1728 = _1713 + _730;
        _1723 = _1711 - _730;
    }
    else
    {
        _1728 = _1713;
        _1723 = _1711;
    }
    float _1715;
    if ((_695 & _227) != 0u)
    {
        _1715 = max(2.0, (_1067.x * _1067.y) + 2.0);
    }
    else
    {
        _1715 = 2.0;
    }
    float _1716;
    if (((_695 & _242) != 0u) && (!((_695 & _247) != 0u)))
    {
        _1716 = max(_1715, (((_1108.x * 1.60000002384185791015625) + max(_1108.y, 0.0)) + ((_93.gDrawInfo.w > 0.5) ? length(_1108.zw) : max(abs(_1108.z), abs(_1108.w)))) + 2.0);
    }
    else
    {
        _1716 = _1715;
    }
    float _1717;
    if ((_695 & _272) != 0u)
    {
        _1717 = max(_1716, (texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_678 % _1115) * 24u) + 10u), int(_678 / _1115), 0).xy, 0).x * 1.7999999523162841796875) + 2.0);
    }
    else
    {
        _1717 = _1716;
    }
    float _1718;
    if ((_695 & _286) != 0u)
    {
        uint _1298 = max(uint(_38.gConv.z), 1u);
        _1718 = max(_1717, (texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_678 % _1298) * 24u) + 1u), int(_678 / _1298), 0).xy, 0).w * 1.7999999523162841796875) + 2.0);
    }
    else
    {
        _1718 = _1717;
    }
    vec2 _801 = _1723 - vec2(_1718);
    vec2 _805 = _1728 + vec2(_1718);
    vec2 _1729;
    vec2 _1730;
    if ((_695 & _310) != 0u)
    {
        uint _1339 = max(uint(_38.gConv.z), 1u);
        vec4 _1373 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_678 % _1339) * 24u) + 19u), int(_678 / _1339), 0).xy, 0);
        _1730 = min(_805, _1373.zw + vec2(1.0));
        _1729 = max(_801, _1373.xy - vec2(1.0));
    }
    else
    {
        _1730 = _805;
        _1729 = _801;
    }
    vec2 _1731;
    vec2 _1732;
    if (((_695 & _336) != 0u) && (!(((_695 & _340) != 0u) && (!((_695 & _344) != 0u)))))
    {
        uint _1380 = max(uint(_38.gConv.z), 1u);
        vec4 _1414 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_678 % _1380) * 24u) + 22u), int(_678 / _1380), 0).xy, 0);
        _1732 = max(_1729, _1414.xy - vec2(1.0));
        _1731 = min(_1730, _1414.zw + vec2(1.0));
    }
    else
    {
        _1732 = _1729;
        _1731 = _1730;
    }
    vec2 _1735;
    if ((((_693.z == 2u) && (!((_695 & _395) != 0u))) && (!((_695 & _401) != 0u))) && (!((_695 & _407) != 0u)))
    {
        vec2 _896 = _1190.zw - _1190.xy;
        float _898 = length(_896);
        vec2 _904 = _896 / vec2(_898);
        bvec2 _905 = bvec2(_898 > 9.9999997473787516355514526367188e-05);
        vec2 _906 = vec2(_905.x ? _904.x : vec2(1.0, 0.0).x, _905.y ? _904.y : vec2(1.0, 0.0).y);
        uint _1421 = max(uint(_38.gConv.z), 1u);
        float _917 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_678 % _1421) * 24u) + 1u), int(_678 / _1421), 0).xy, 0).x + _1718;
        vec2 _927 = _1190.zw + (_906 * _917);
        vec2 _933 = _1190.xy - (_906 * _917);
        bvec2 _934 = bvec2((uint(gl_VertexID) & 1u) != 0u);
        _1735 = vec2(_934.x ? _927.x : _933.x, _934.y ? _927.y : _933.y) + (vec2(-_906.y, _906.x) * (((uint(gl_VertexID) & 2u) != 0u) ? _917 : (-_917)));
    }
    else
    {
        _1735 = vec2(((uint(gl_VertexID) & 1u) != 0u) ? _1731.x : _1732.x, ((uint(gl_VertexID) & 2u) != 0u) ? _1731.y : _1732.y);
    }
    vec4 _959 = vec4((_1735 * _38.gXform.xy) + _38.gXform.zw, 0.0, 1.0);
    bvec4 _1739 = bvec4(_693.z == 5u);
    uint _1462 = max(uint(_38.gConv.z), 1u);
    uint _1503 = max(uint(_38.gConv.z), 1u);
    uint _1544 = max(uint(_38.gConv.z), 1u);
    uint _1585 = max(uint(_38.gConv.z), 1u);
    gl_Position = vec4(_1739.x ? vec4(-2.0, -2.0, 0.0, 1.0).x : _959.x, _1739.y ? vec4(-2.0, -2.0, 0.0, 1.0).y : _959.y, _1739.z ? vec4(-2.0, -2.0, 0.0, 1.0).z : _959.z, _1739.w ? vec4(-2.0, -2.0, 0.0, 1.0).w : _959.w);
    esia_v0 = _1735;
    esia_v1 = _678;
    esia_v2 = _1026;
    esia_v3 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_678 % _1462) * 24u) + 1u), int(_678 / _1462), 0).xy, 0);
    esia_v4 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_678 % _1503) * 24u) + 2u), int(_678 / _1503), 0).xy, 0);
    esia_v5 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_678 % _1544) * 24u) + 14u), int(_678 / _1544), 0).xy, 0);
    esia_v6 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_678 % _1585) * 24u) + 17u), int(_678 / _1585), 0).xy, 0);
    esia_v7 = _693;
}

