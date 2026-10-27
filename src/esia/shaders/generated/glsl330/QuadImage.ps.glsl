#version 330

vec2 _385;

layout(std140) uniform WgtFrame
{
    vec4 gXform;
    vec4 gTarget;
    vec4 gDisplay;
    vec4 gTime;
    vec4 gLevel[6];
    vec4 gText;
    vec4 gConv;
} _44;

layout(std140) uniform WgtDraw
{
    vec4 gFade;
    vec4 gDrawInfo;
} _76;

uniform sampler2D SPIRV_Cross_CombinedgTexgLinear;

in vec4 esia_v0;
in vec2 esia_v1;
layout(location = 0) out vec4 _entryPointOutput;

void main()
{
    vec4 _237 = texture(SPIRV_Cross_CombinedgTexgLinear, esia_v1);
    vec4 _238 = esia_v0 * _237;
    vec2 _258 = gl_FragCoord.xy + _44.gConv.yy;
    vec2 _377;
    if (_44.gConv.x > 0.5)
    {
        vec2 _358;
        _358.y = _44.gTarget.y - _258.y;
        _377 = _358;
    }
    else
    {
        _377 = _258;
    }
    float _378;
    if (_76.gFade.z > 0.0)
    {
        _378 = smoothstep(0.0, 1.0, clamp((_377.y - _76.gFade.x) / _76.gFade.z, 0.0, 1.0));
    }
    else
    {
        _378 = 1.0;
    }
    float _379;
    if (_76.gFade.w > 0.0)
    {
        _379 = _378 * smoothstep(0.0, 1.0, clamp((_76.gFade.y - _377.y) / _76.gFade.w, 0.0, 1.0));
    }
    else
    {
        _379 = _378;
    }
    float _246 = _238.w * _379;
    vec4 _361 = _238;
    _361.w = _246;
    vec4 _383;
    if (_44.gTime.w > 0.5)
    {
        vec3 _330 = clamp(_361.xyz, vec3(0.0), vec3(1.0));
        vec3 _336 = pow((_330 + vec3(0.054999999701976776123046875)) * vec3(0.947867333889007568359375), vec3(2.400000095367431640625));
        vec3 _339 = _330 * vec3(0.077399380505084991455078125);
        bvec3 _341 = lessThanEqual(_330, vec3(0.040449999272823333740234375));
        _383 = vec4(vec3(_341.x ? _339.x : _336.x, _341.y ? _339.y : _336.y, _341.z ? _339.z : _336.z), _246);
    }
    else
    {
        _383 = _361;
    }
    _entryPointOutput = _383;
}

