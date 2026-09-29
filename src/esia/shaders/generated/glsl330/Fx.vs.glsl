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
} _87;

uniform sampler2D SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler;

out vec2 esia_v0;
flat out uint esia_v1;

void main()
{
    uint _424 = uint(gl_InstanceID) + uint(_87.gDrawInfo.x);
    uint _607 = max(uint(_28.gConv.z), 1u);
    vec4 _641 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int((_424 % _607) * 24u), int(_424 / _607), 0).xy, 0);
    uint _648 = max(uint(_28.gConv.z), 1u);
    vec4 _682 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_424 % _648) * 24u) + 6u), int(_424 / _648), 0).xy, 0);
    uint _689 = max(uint(_28.gConv.z), 1u);
    vec4 _723 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_424 % _689) * 24u) + 8u), int(_424 / _689), 0).xy, 0);
    uint _730 = max(uint(_28.gConv.z), 1u);
    uint _771 = max(uint(_28.gConv.z), 1u);
    vec4 _805 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_424 % _771) * 24u) + 15u), int(_424 / _771), 0).xy, 0);
    uint _812 = max(uint(_28.gConv.z), 1u);
    uint _440 = uint(texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_424 % _812) * 24u) + 23u), int(_424 / _812), 0).xy, 0).x);
    vec2 _442 = _641.xy;
    vec2 _444 = _641.zw;
    vec2 _979;
    vec2 _983;
    if ((_440 & 512u) != 0u)
    {
        _983 = max(_444, _805.zw);
        _979 = min(_442, _805.xy);
    }
    else
    {
        _983 = _444;
        _979 = _442;
    }
    float _973;
    if ((_440 & 2u) != 0u)
    {
        _973 = max(2.0, (_682.x * _682.y) + 2.0);
    }
    else
    {
        _973 = 2.0;
    }
    float _974;
    if (((_440 & 4u) != 0u) && (!((_440 & 256u) != 0u)))
    {
        _974 = max(_973, (((_723.x * 1.60000002384185791015625) + max(_723.y, 0.0)) + max(abs(_723.z), abs(_723.w))) + 2.0);
    }
    else
    {
        _974 = _973;
    }
    float _975;
    if ((_440 & 8u) != 0u)
    {
        _975 = max(_974, (texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_424 % _730) * 24u) + 10u), int(_424 / _730), 0).xy, 0).x * 1.7999999523162841796875) + 2.0);
    }
    else
    {
        _975 = _974;
    }
    vec2 _513 = _979 - vec2(_975);
    vec2 _517 = _983 + vec2(_975);
    vec2 _984;
    vec2 _985;
    if ((_440 & 1024u) != 0u)
    {
        uint _853 = max(uint(_28.gConv.z), 1u);
        vec4 _887 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_424 % _853) * 24u) + 19u), int(_424 / _853), 0).xy, 0);
        _985 = min(_517, _887.zw + vec2(1.0));
        _984 = max(_513, _887.xy - vec2(1.0));
    }
    else
    {
        _985 = _517;
        _984 = _513;
    }
    vec2 _986;
    vec2 _987;
    if (((_440 & 8192u) != 0u) && (!(((_440 & 4u) != 0u) && (!((_440 & 256u) != 0u)))))
    {
        uint _894 = max(uint(_28.gConv.z), 1u);
        vec4 _928 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_424 % _894) * 24u) + 22u), int(_424 / _894), 0).xy, 0);
        _987 = max(_984, _928.xy - vec2(1.0));
        _986 = min(_985, _928.zw + vec2(1.0));
    }
    else
    {
        _987 = _984;
        _986 = _985;
    }
    vec2 _584 = vec2(((uint(gl_VertexID) & 1u) != 0u) ? _986.x : _987.x, ((uint(gl_VertexID) & 2u) != 0u) ? _986.y : _987.y);
    gl_Position = vec4((_584 * _28.gXform.xy) + _28.gXform.zw, 0.0, 1.0);
    esia_v0 = _584;
    esia_v1 = _424;
}

