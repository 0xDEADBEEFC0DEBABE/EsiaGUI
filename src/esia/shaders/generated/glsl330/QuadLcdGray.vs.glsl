#version 330

layout(std140) uniform WgtFrame
{
    vec4 gXform;
    vec4 gTarget;
    vec4 gDisplay;
    vec4 gTime;
    vec4 gLevel[6];
    vec4 gText;
    vec4 gConv;
} _28;

layout(std140) uniform WgtDraw
{
    vec4 gFade;
    vec4 gDrawInfo;
} _97;

uniform sampler2D SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler;

out vec4 esia_v0;
out vec2 esia_v1;

void main()
{
    uint _299 = uint(gl_InstanceID) + uint(_97.gDrawInfo.x);
    uint _406 = max(uint(_28.gConv.z), 1u) * 8u;
    vec4 _448 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int((_299 % _406) * 2u), int(uint(_28.gConv.w) + (_299 / _406)), 0).xy, 0);
    uint _456 = max(uint(_28.gConv.z), 1u) * 8u;
    vec4 _498 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_299 % _456) * 2u) + 1u), int(uint(_28.gConv.w) + (_299 / _456)), 0).xy, 0);
    float _307 = floor(_498.x * 0.000244140625);
    float _316 = floor(_498.y * 0.000244140625);
    float _325 = floor(_498.z * 1.52587890625e-05);
    float _332 = floor((_498.z - (_325 * 65536.0)) * 0.00390625);
    bool _344 = (uint(gl_VertexID) & 1u) != 0u;
    bool _348 = (uint(gl_VertexID) & 2u) != 0u;
    gl_Position = vec4((vec2(_344 ? _448.z : _448.x, _348 ? _448.w : _448.y) * _28.gXform.xy) + _28.gXform.zw, 0.0, 1.0);
    esia_v0 = vec4((_498.z - (_325 * 65536.0)) - (_332 * 256.0), _332, _325, _498.w) * vec4(0.0039215688593685626983642578125);
    esia_v1 = vec2((_498.x - (_307 * 4096.0)) + (_344 ? (_498.y - (_316 * 4096.0)) : 0.0), _307 + (_348 ? _316 : 0.0)) * vec2(0.000244140625);
}

