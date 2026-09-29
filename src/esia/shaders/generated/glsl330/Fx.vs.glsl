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
} _29;

layout(std140) uniform WgtDraw
{
    vec4 gFade;
    vec4 gDrawInfo;
} _88;

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
    uint _487 = uint(gl_InstanceID) + uint(_88.gDrawInfo.x);
    uint _687 = max(uint(_29.gConv.z), 1u);
    vec4 _721 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int((_487 % _687) * 24u), int(_487 / _687), 0).xy, 0);
    uint _728 = max(uint(_29.gConv.z), 1u);
    vec4 _762 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_487 % _728) * 24u) + 6u), int(_487 / _728), 0).xy, 0);
    uint _769 = max(uint(_29.gConv.z), 1u);
    vec4 _803 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_487 % _769) * 24u) + 8u), int(_487 / _769), 0).xy, 0);
    uint _810 = max(uint(_29.gConv.z), 1u);
    uint _851 = max(uint(_29.gConv.z), 1u);
    vec4 _885 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_487 % _851) * 24u) + 15u), int(_487 / _851), 0).xy, 0);
    uint _892 = max(uint(_29.gConv.z), 1u);
    uvec4 _502 = uvec4(texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_487 % _892) * 24u) + 23u), int(_487 / _892), 0).xy, 0));
    uint _504 = _502.x;
    vec2 _506 = _721.xy;
    vec2 _508 = _721.zw;
    vec2 _1266;
    vec2 _1270;
    if ((_504 & 512u) != 0u)
    {
        _1270 = max(_508, _885.zw);
        _1266 = min(_506, _885.xy);
    }
    else
    {
        _1270 = _508;
        _1266 = _506;
    }
    float _1260;
    if ((_504 & 2u) != 0u)
    {
        _1260 = max(2.0, (_762.x * _762.y) + 2.0);
    }
    else
    {
        _1260 = 2.0;
    }
    float _1261;
    if (((_504 & 4u) != 0u) && (!((_504 & 256u) != 0u)))
    {
        _1261 = max(_1260, (((_803.x * 1.60000002384185791015625) + max(_803.y, 0.0)) + max(abs(_803.z), abs(_803.w))) + 2.0);
    }
    else
    {
        _1261 = _1260;
    }
    float _1262;
    if ((_504 & 8u) != 0u)
    {
        _1262 = max(_1261, (texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_487 % _810) * 24u) + 10u), int(_487 / _810), 0).xy, 0).x * 1.7999999523162841796875) + 2.0);
    }
    else
    {
        _1262 = _1261;
    }
    vec2 _577 = _1266 - vec2(_1262);
    vec2 _581 = _1270 + vec2(_1262);
    vec2 _1271;
    vec2 _1272;
    if ((_504 & 1024u) != 0u)
    {
        uint _933 = max(uint(_29.gConv.z), 1u);
        vec4 _967 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_487 % _933) * 24u) + 19u), int(_487 / _933), 0).xy, 0);
        _1272 = min(_581, _967.zw + vec2(1.0));
        _1271 = max(_577, _967.xy - vec2(1.0));
    }
    else
    {
        _1272 = _581;
        _1271 = _577;
    }
    vec2 _1273;
    vec2 _1274;
    if (((_504 & 8192u) != 0u) && (!(((_504 & 4u) != 0u) && (!((_504 & 256u) != 0u)))))
    {
        uint _974 = max(uint(_29.gConv.z), 1u);
        vec4 _1008 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_487 % _974) * 24u) + 22u), int(_487 / _974), 0).xy, 0);
        _1274 = max(_1271, _1008.xy - vec2(1.0));
        _1273 = min(_1272, _1008.zw + vec2(1.0));
    }
    else
    {
        _1274 = _1271;
        _1273 = _1272;
    }
    vec2 _648 = vec2(((uint(gl_VertexID) & 1u) != 0u) ? _1273.x : _1274.x, ((uint(gl_VertexID) & 2u) != 0u) ? _1273.y : _1274.y);
    uint _1015 = max(uint(_29.gConv.z), 1u);
    uint _1056 = max(uint(_29.gConv.z), 1u);
    uint _1097 = max(uint(_29.gConv.z), 1u);
    uint _1138 = max(uint(_29.gConv.z), 1u);
    gl_Position = vec4((_648 * _29.gXform.xy) + _29.gXform.zw, 0.0, 1.0);
    esia_v0 = _648;
    esia_v1 = _487;
    esia_v2 = _721;
    esia_v3 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_487 % _1015) * 24u) + 1u), int(_487 / _1015), 0).xy, 0);
    esia_v4 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_487 % _1056) * 24u) + 2u), int(_487 / _1056), 0).xy, 0);
    esia_v5 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_487 % _1097) * 24u) + 14u), int(_487 / _1097), 0).xy, 0);
    esia_v6 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_487 % _1138) * 24u) + 17u), int(_487 / _1138), 0).xy, 0);
    esia_v7 = _502;
}

