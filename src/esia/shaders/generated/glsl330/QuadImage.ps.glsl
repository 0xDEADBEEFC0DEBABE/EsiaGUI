#version 330

vec2 _386;

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
    vec4 _238 = texture(SPIRV_Cross_CombinedgTexgLinear, esia_v1);
    vec4 _239 = esia_v0 * _238;
    vec2 _259 = gl_FragCoord.xy + _44.gConv.yy;
    vec2 _378;
    if (_44.gConv.x > 0.5)
    {
        vec2 _359;
        _359.y = _44.gTarget.y - _259.y;
        _378 = _359;
    }
    else
    {
        _378 = _259;
    }
    float _379;
    if (_76.gFade.z > 0.0)
    {
        _379 = smoothstep(0.0, 1.0, clamp((_378.y - _76.gFade.x) / _76.gFade.z, 0.0, 1.0));
    }
    else
    {
        _379 = 1.0;
    }
    float _380;
    if (_76.gFade.w > 0.0)
    {
        _380 = _379 * smoothstep(0.0, 1.0, clamp((_76.gFade.y - _378.y) / _76.gFade.w, 0.0, 1.0));
    }
    else
    {
        _380 = _379;
    }
    float _247 = _239.w * _380;
    vec4 _362 = _239;
    _362.w = _247;
    vec4 _384;
    if (_44.gTime.w > 0.5)
    {
        vec3 _331 = clamp(_362.xyz, vec3(0.0), vec3(1.0));
        vec3 _337 = pow((_331 + vec3(0.054999999701976776123046875)) * vec3(0.947867333889007568359375), vec3(2.400000095367431640625));
        vec3 _340 = _331 * vec3(0.077399380505084991455078125);
        bvec3 _342 = lessThanEqual(_331, vec3(0.040449999272823333740234375));
        _384 = vec4(vec3(_342.x ? _340.x : _337.x, _342.y ? _340.y : _337.y, _342.z ? _340.z : _337.z), _247);
    }
    else
    {
        _384 = _362;
    }
    _entryPointOutput = _384;
}

