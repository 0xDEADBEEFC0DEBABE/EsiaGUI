#version 330

vec2 _409;

layout(std140) uniform WgtFrame
{
    vec4 gXform;
    vec4 gTarget;
    vec4 gDisplay;
    vec4 gTime;
    vec4 gLevel[6];
    vec4 gText;
    vec4 gConv;
} _50;

layout(std140) uniform WgtDraw
{
    vec4 gFade;
    vec4 gDrawInfo;
} _82;

uniform sampler2D SPIRV_Cross_CombinedgTexgLinear;

in vec4 esia_v0;
in vec2 esia_v1;
layout(location = 0) out vec4 _entryPointOutput;

void main()
{
    vec4 _249 = texture(SPIRV_Cross_CombinedgTexgLinear, esia_v1);
    vec4 _263 = esia_v0 * _249;
    vec2 _282 = gl_FragCoord.xy + _50.gConv.yy;
    vec2 _401;
    if (_50.gConv.x > 0.5)
    {
        vec2 _382;
        _382.y = _50.gTarget.y - _282.y;
        _401 = _382;
    }
    else
    {
        _401 = _282;
    }
    float _402;
    if (_82.gFade.z > 0.0)
    {
        _402 = smoothstep(0.0, 1.0, clamp((_401.y - _82.gFade.x) / _82.gFade.z, 0.0, 1.0));
    }
    else
    {
        _402 = 1.0;
    }
    float _403;
    if (_82.gFade.w > 0.0)
    {
        _403 = _402 * smoothstep(0.0, 1.0, clamp((_82.gFade.y - _401.y) / _82.gFade.w, 0.0, 1.0));
    }
    else
    {
        _403 = _402;
    }
    float _270 = _263.w * _403;
    vec4 _385 = _263;
    _385.w = _270;
    vec4 _407;
    if (_50.gTime.w > 0.5)
    {
        vec3 _354 = clamp(_385.xyz, vec3(0.0), vec3(1.0));
        vec3 _360 = pow((_354 + vec3(0.054999999701976776123046875)) * vec3(0.947867333889007568359375), vec3(2.400000095367431640625));
        vec3 _363 = _354 * vec3(0.077399380505084991455078125);
        bvec3 _365 = lessThanEqual(_354, vec3(0.040449999272823333740234375));
        _407 = vec4(vec3(_365.x ? _363.x : _360.x, _365.y ? _363.y : _360.y, _365.z ? _363.z : _360.z), _270);
    }
    else
    {
        _407 = _385;
    }
    _entryPointOutput = _407;
}

