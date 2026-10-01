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

layout(std140) uniform WgtPass
{
    vec4 gPass0;
    vec4 gPass1;
} _64;

uniform sampler2D SPIRV_Cross_CombinedgTexgLinear;

in vec2 esia_v0;
layout(location = 0) out vec4 _entryPointOutput;

void main()
{
    vec2 _432 = clamp(esia_v0 + (_64.gPass0.xy * vec2(-2.0)), _64.gPass1.xy, _64.gPass1.zw);
    vec2 _843;
    if (_29.gConv.x > 0.5)
    {
        vec2 _806 = _432;
        _806.y = 1.0 - _432.y;
        _843 = _806;
    }
    else
    {
        _843 = _432;
    }
    vec2 _461 = clamp(esia_v0 + (_64.gPass0.xy * vec2(0.0, -2.0)), _64.gPass1.xy, _64.gPass1.zw);
    vec2 _844;
    if (_29.gConv.x > 0.5)
    {
        vec2 _809 = _461;
        _809.y = 1.0 - _461.y;
        _844 = _809;
    }
    else
    {
        _844 = _461;
    }
    vec4 _463 = textureLod(SPIRV_Cross_CombinedgTexgLinear, _844, 0.0);
    vec2 _490 = clamp(esia_v0 + (_64.gPass0.xy * vec2(2.0, -2.0)), _64.gPass1.xy, _64.gPass1.zw);
    vec2 _845;
    if (_29.gConv.x > 0.5)
    {
        vec2 _812 = _490;
        _812.y = 1.0 - _490.y;
        _845 = _812;
    }
    else
    {
        _845 = _490;
    }
    vec2 _519 = clamp(esia_v0 + (_64.gPass0.xy * vec2(-1.0)), _64.gPass1.xy, _64.gPass1.zw);
    vec2 _846;
    if (_29.gConv.x > 0.5)
    {
        vec2 _815 = _519;
        _815.y = 1.0 - _519.y;
        _846 = _815;
    }
    else
    {
        _846 = _519;
    }
    vec2 _548 = clamp(esia_v0 + (_64.gPass0.xy * vec2(1.0, -1.0)), _64.gPass1.xy, _64.gPass1.zw);
    vec2 _847;
    if (_29.gConv.x > 0.5)
    {
        vec2 _818 = _548;
        _818.y = 1.0 - _548.y;
        _847 = _818;
    }
    else
    {
        _847 = _548;
    }
    vec2 _577 = clamp(esia_v0 + (_64.gPass0.xy * vec2(-2.0, 0.0)), _64.gPass1.xy, _64.gPass1.zw);
    vec2 _848;
    if (_29.gConv.x > 0.5)
    {
        vec2 _821 = _577;
        _821.y = 1.0 - _577.y;
        _848 = _821;
    }
    else
    {
        _848 = _577;
    }
    vec4 _579 = textureLod(SPIRV_Cross_CombinedgTexgLinear, _848, 0.0);
    vec2 _606 = clamp(esia_v0, _64.gPass1.xy, _64.gPass1.zw);
    vec2 _849;
    if (_29.gConv.x > 0.5)
    {
        vec2 _824 = _606;
        _824.y = 1.0 - _606.y;
        _849 = _824;
    }
    else
    {
        _849 = _606;
    }
    vec4 _608 = textureLod(SPIRV_Cross_CombinedgTexgLinear, _849, 0.0);
    vec2 _635 = clamp(esia_v0 + (_64.gPass0.xy * vec2(2.0, 0.0)), _64.gPass1.xy, _64.gPass1.zw);
    vec2 _850;
    if (_29.gConv.x > 0.5)
    {
        vec2 _827 = _635;
        _827.y = 1.0 - _635.y;
        _850 = _827;
    }
    else
    {
        _850 = _635;
    }
    vec4 _637 = textureLod(SPIRV_Cross_CombinedgTexgLinear, _850, 0.0);
    vec2 _664 = clamp(esia_v0 + (_64.gPass0.xy * vec2(-1.0, 1.0)), _64.gPass1.xy, _64.gPass1.zw);
    vec2 _851;
    if (_29.gConv.x > 0.5)
    {
        vec2 _830 = _664;
        _830.y = 1.0 - _664.y;
        _851 = _830;
    }
    else
    {
        _851 = _664;
    }
    vec2 _693 = clamp(esia_v0 + _64.gPass0.xy, _64.gPass1.xy, _64.gPass1.zw);
    vec2 _852;
    if (_29.gConv.x > 0.5)
    {
        vec2 _833 = _693;
        _833.y = 1.0 - _693.y;
        _852 = _833;
    }
    else
    {
        _852 = _693;
    }
    vec2 _722 = clamp(esia_v0 + (_64.gPass0.xy * vec2(-2.0, 2.0)), _64.gPass1.xy, _64.gPass1.zw);
    vec2 _853;
    if (_29.gConv.x > 0.5)
    {
        vec2 _836 = _722;
        _836.y = 1.0 - _722.y;
        _853 = _836;
    }
    else
    {
        _853 = _722;
    }
    vec2 _751 = clamp(esia_v0 + (_64.gPass0.xy * vec2(0.0, 2.0)), _64.gPass1.xy, _64.gPass1.zw);
    vec2 _854;
    if (_29.gConv.x > 0.5)
    {
        vec2 _839 = _751;
        _839.y = 1.0 - _751.y;
        _854 = _839;
    }
    else
    {
        _854 = _751;
    }
    vec4 _753 = textureLod(SPIRV_Cross_CombinedgTexgLinear, _854, 0.0);
    vec2 _780 = clamp(esia_v0 + (_64.gPass0.xy * vec2(2.0)), _64.gPass1.xy, _64.gPass1.zw);
    vec2 _855;
    if (_29.gConv.x > 0.5)
    {
        vec2 _842 = _780;
        _842.y = 1.0 - _780.y;
        _855 = _842;
    }
    else
    {
        _855 = _780;
    }
    _entryPointOutput = (((((((textureLod(SPIRV_Cross_CombinedgTexgLinear, _846, 0.0) + textureLod(SPIRV_Cross_CombinedgTexgLinear, _847, 0.0)) + textureLod(SPIRV_Cross_CombinedgTexgLinear, _851, 0.0)) + textureLod(SPIRV_Cross_CombinedgTexgLinear, _852, 0.0)) * 0.125) + ((((textureLod(SPIRV_Cross_CombinedgTexgLinear, _843, 0.0) + _463) + _579) + _608) * 0.03125)) + ((((_463 + textureLod(SPIRV_Cross_CombinedgTexgLinear, _845, 0.0)) + _608) + _637) * 0.03125)) + ((((_579 + _608) + textureLod(SPIRV_Cross_CombinedgTexgLinear, _853, 0.0)) + _753) * 0.03125)) + ((((_608 + _637) + _753) + textureLod(SPIRV_Cross_CombinedgTexgLinear, _855, 0.0)) * 0.03125);
}

