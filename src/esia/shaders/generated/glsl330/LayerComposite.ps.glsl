#version 330
#if defined(GL_EXT_control_flow_attributes)
#extension GL_EXT_control_flow_attributes : require
#define SPIRV_CROSS_FLATTEN [[flatten]]
#define SPIRV_CROSS_BRANCH [[dont_flatten]]
#define SPIRV_CROSS_UNROLL [[unroll]]
#define SPIRV_CROSS_LOOP [[dont_unroll]]
#else
#define SPIRV_CROSS_FLATTEN
#define SPIRV_CROSS_BRANCH
#define SPIRV_CROSS_UNROLL
#define SPIRV_CROSS_LOOP
#endif

layout(std140) uniform WgtFrame
{
    vec4 gXform;
    vec4 gTarget;
    vec4 gDisplay;
    vec4 gTime;
    vec4 gLevel[6];
    vec4 gText;
    vec4 gConv;
} _60;

layout(std140) uniform WgtPass
{
    vec4 gPass0;
    vec4 gPass1;
} _414;

uniform sampler2D SPIRV_Cross_CombinedgTexgPoint;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop0gLinear;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop1gLinear;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop2gLinear;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop3gLinear;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop4gLinear;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop5gLinear;

layout(location = 0) out vec4 _entryPointOutput;

void main()
{
    vec2 _724 = gl_FragCoord.xy + _60.gConv.yy;
    vec2 _4239;
    if (_60.gConv.x > 0.5)
    {
        vec2 _3784 = _724;
        _3784.y = _60.gTarget.y - _724.y;
        _4239 = _3784;
    }
    else
    {
        _4239 = _724;
    }
    vec2 _603 = _4239 * _60.gTarget.zw;
    vec2 _4240;
    if (_60.gConv.x > 0.5)
    {
        vec2 _3787 = _603;
        _3787.y = 1.0 - _603.y;
        _4240 = _3787;
    }
    else
    {
        _4240 = _603;
    }
    float _619 = clamp(log2(max(_414.gPass1.y * _60.gDisplay.z, 2.0)) - 1.0, 1.0, 5.0);
    int _622 = int(floor(_619));
    int _768 = clamp(_622, 1, 5);
    vec2 _837 = (_603 * _60.gLevel[_768].xy) - vec2(0.5);
    vec2 _839 = floor(_837);
    vec2 _842 = _837 - _839;
    vec2 _845 = _842 * _842;
    vec2 _848 = _845 * _842;
    vec2 _867 = (((_848 * 3.0) - (_845 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
    vec2 _880 = _848 * 0.16666667163372039794921875;
    vec2 _883 = (((((-_848) + (_845 * 3.0)) - (_842 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _867;
    vec2 _887 = (((((_848 * (-3.0)) + (_845 * 3.0)) + (_842 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _880;
    vec2 _899 = ((_839 - vec2(0.5)) + (_867 / _883)) * _60.gLevel[_768].zw;
    vec2 _911 = ((_839 + vec2(1.5)) + (_880 / _887)) * _60.gLevel[_768].zw;
    vec4 _4277;
    SPIRV_CROSS_BRANCH
    if (_622 <= 0)
    {
        vec2 _4276;
        if (_60.gConv.x > 0.5)
        {
            vec2 _3790 = _603;
            _3790.y = 1.0 - _603.y;
            _4276 = _3790;
        }
        else
        {
            _4276 = _603;
        }
        _4277 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _4276, 0.0);
    }
    else
    {
        vec4 _4278;
        if (_622 == 1)
        {
            vec2 _4269;
            if (_60.gConv.x > 0.5)
            {
                vec2 _3795 = _899;
                _3795.y = 1.0 - _899.y;
                _4269 = _3795;
            }
            else
            {
                _4269 = _899;
            }
            float _956 = _899.y;
            vec2 _957 = vec2(_911.x, _956);
            vec2 _4270;
            if (_60.gConv.x > 0.5)
            {
                vec2 _3802 = _957;
                _3802.y = 1.0 - _956;
                _4270 = _3802;
            }
            else
            {
                _4270 = _957;
            }
            float _974 = _911.y;
            vec2 _975 = vec2(_899.x, _974);
            vec2 _4272;
            if (_60.gConv.x > 0.5)
            {
                vec2 _3809 = _975;
                _3809.y = 1.0 - _974;
                _4272 = _3809;
            }
            else
            {
                _4272 = _975;
            }
            vec2 _4274;
            if (_60.gConv.x > 0.5)
            {
                vec2 _3816 = _911;
                _3816.y = 1.0 - _911.y;
                _4274 = _3816;
            }
            else
            {
                _4274 = _911;
            }
            _4278 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _4269, 0.0) * (_883.x * _883.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _4270, 0.0) * (_887.x * _883.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _4272, 0.0) * (_883.x * _887.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _4274, 0.0) * (_887.x * _887.y));
        }
        else
        {
            vec4 _4279;
            if (_622 == 2)
            {
                vec2 _4262;
                if (_60.gConv.x > 0.5)
                {
                    vec2 _3823 = _899;
                    _3823.y = 1.0 - _899.y;
                    _4262 = _3823;
                }
                else
                {
                    _4262 = _899;
                }
                float _1086 = _899.y;
                vec2 _1087 = vec2(_911.x, _1086);
                vec2 _4263;
                if (_60.gConv.x > 0.5)
                {
                    vec2 _3830 = _1087;
                    _3830.y = 1.0 - _1086;
                    _4263 = _3830;
                }
                else
                {
                    _4263 = _1087;
                }
                float _1104 = _911.y;
                vec2 _1105 = vec2(_899.x, _1104);
                vec2 _4265;
                if (_60.gConv.x > 0.5)
                {
                    vec2 _3837 = _1105;
                    _3837.y = 1.0 - _1104;
                    _4265 = _3837;
                }
                else
                {
                    _4265 = _1105;
                }
                vec2 _4267;
                if (_60.gConv.x > 0.5)
                {
                    vec2 _3844 = _911;
                    _3844.y = 1.0 - _911.y;
                    _4267 = _3844;
                }
                else
                {
                    _4267 = _911;
                }
                _4279 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _4262, 0.0) * (_883.x * _883.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _4263, 0.0) * (_887.x * _883.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _4265, 0.0) * (_883.x * _887.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _4267, 0.0) * (_887.x * _887.y));
            }
            else
            {
                vec4 _4280;
                if (_622 == 3)
                {
                    vec2 _4255;
                    if (_60.gConv.x > 0.5)
                    {
                        vec2 _3851 = _899;
                        _3851.y = 1.0 - _899.y;
                        _4255 = _3851;
                    }
                    else
                    {
                        _4255 = _899;
                    }
                    float _1216 = _899.y;
                    vec2 _1217 = vec2(_911.x, _1216);
                    vec2 _4256;
                    if (_60.gConv.x > 0.5)
                    {
                        vec2 _3858 = _1217;
                        _3858.y = 1.0 - _1216;
                        _4256 = _3858;
                    }
                    else
                    {
                        _4256 = _1217;
                    }
                    float _1234 = _911.y;
                    vec2 _1235 = vec2(_899.x, _1234);
                    vec2 _4258;
                    if (_60.gConv.x > 0.5)
                    {
                        vec2 _3865 = _1235;
                        _3865.y = 1.0 - _1234;
                        _4258 = _3865;
                    }
                    else
                    {
                        _4258 = _1235;
                    }
                    vec2 _4260;
                    if (_60.gConv.x > 0.5)
                    {
                        vec2 _3872 = _911;
                        _3872.y = 1.0 - _911.y;
                        _4260 = _3872;
                    }
                    else
                    {
                        _4260 = _911;
                    }
                    _4280 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _4255, 0.0) * (_883.x * _883.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _4256, 0.0) * (_887.x * _883.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _4258, 0.0) * (_883.x * _887.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _4260, 0.0) * (_887.x * _887.y));
                }
                else
                {
                    vec4 _4281;
                    if (_622 == 4)
                    {
                        vec2 _4248;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _3879 = _899;
                            _3879.y = 1.0 - _899.y;
                            _4248 = _3879;
                        }
                        else
                        {
                            _4248 = _899;
                        }
                        float _1346 = _899.y;
                        vec2 _1347 = vec2(_911.x, _1346);
                        vec2 _4249;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _3886 = _1347;
                            _3886.y = 1.0 - _1346;
                            _4249 = _3886;
                        }
                        else
                        {
                            _4249 = _1347;
                        }
                        float _1364 = _911.y;
                        vec2 _1365 = vec2(_899.x, _1364);
                        vec2 _4251;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _3893 = _1365;
                            _3893.y = 1.0 - _1364;
                            _4251 = _3893;
                        }
                        else
                        {
                            _4251 = _1365;
                        }
                        vec2 _4253;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _3900 = _911;
                            _3900.y = 1.0 - _911.y;
                            _4253 = _3900;
                        }
                        else
                        {
                            _4253 = _911;
                        }
                        _4281 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _4248, 0.0) * (_883.x * _883.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _4249, 0.0) * (_887.x * _883.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _4251, 0.0) * (_883.x * _887.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _4253, 0.0) * (_887.x * _887.y));
                    }
                    else
                    {
                        vec2 _4241;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _3907 = _899;
                            _3907.y = 1.0 - _899.y;
                            _4241 = _3907;
                        }
                        else
                        {
                            _4241 = _899;
                        }
                        float _1476 = _899.y;
                        vec2 _1477 = vec2(_911.x, _1476);
                        vec2 _4242;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _3914 = _1477;
                            _3914.y = 1.0 - _1476;
                            _4242 = _3914;
                        }
                        else
                        {
                            _4242 = _1477;
                        }
                        float _1494 = _911.y;
                        vec2 _1495 = vec2(_899.x, _1494);
                        vec2 _4244;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _3921 = _1495;
                            _3921.y = 1.0 - _1494;
                            _4244 = _3921;
                        }
                        else
                        {
                            _4244 = _1495;
                        }
                        vec2 _4246;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _3928 = _911;
                            _3928.y = 1.0 - _911.y;
                            _4246 = _3928;
                        }
                        else
                        {
                            _4246 = _911;
                        }
                        _4281 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _4241, 0.0) * (_883.x * _883.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _4242, 0.0) * (_887.x * _883.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _4244, 0.0) * (_883.x * _887.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _4246, 0.0) * (_887.x * _887.y));
                    }
                    _4280 = _4281;
                }
                _4279 = _4280;
            }
            _4278 = _4279;
        }
        _4277 = _4278;
    }
    vec4 _4366;
    if (_622 < 5)
    {
        int _635 = _622 + 1;
        int _1595 = clamp(_635, 1, 5);
        vec2 _1664 = (_603 * _60.gLevel[_1595].xy) - vec2(0.5);
        vec2 _1666 = floor(_1664);
        vec2 _1669 = _1664 - _1666;
        vec2 _1672 = _1669 * _1669;
        vec2 _1675 = _1672 * _1669;
        vec2 _1694 = (((_1675 * 3.0) - (_1672 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
        vec2 _1707 = _1675 * 0.16666667163372039794921875;
        vec2 _1710 = (((((-_1675) + (_1672 * 3.0)) - (_1669 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _1694;
        vec2 _1714 = (((((_1675 * (-3.0)) + (_1672 * 3.0)) + (_1669 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _1707;
        vec2 _1726 = ((_1666 - vec2(0.5)) + (_1694 / _1710)) * _60.gLevel[_1595].zw;
        vec2 _1738 = ((_1666 + vec2(1.5)) + (_1707 / _1714)) * _60.gLevel[_1595].zw;
        vec4 _4318;
        SPIRV_CROSS_BRANCH
        if (_635 <= 0)
        {
            vec2 _4317;
            if (_60.gConv.x > 0.5)
            {
                vec2 _3933 = _603;
                _3933.y = 1.0 - _603.y;
                _4317 = _3933;
            }
            else
            {
                _4317 = _603;
            }
            _4318 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _4317, 0.0);
        }
        else
        {
            vec4 _4319;
            if (_635 == 1)
            {
                vec2 _4310;
                if (_60.gConv.x > 0.5)
                {
                    vec2 _3938 = _1726;
                    _3938.y = 1.0 - _1726.y;
                    _4310 = _3938;
                }
                else
                {
                    _4310 = _1726;
                }
                float _1783 = _1726.y;
                vec2 _1784 = vec2(_1738.x, _1783);
                vec2 _4311;
                if (_60.gConv.x > 0.5)
                {
                    vec2 _3945 = _1784;
                    _3945.y = 1.0 - _1783;
                    _4311 = _3945;
                }
                else
                {
                    _4311 = _1784;
                }
                float _1801 = _1738.y;
                vec2 _1802 = vec2(_1726.x, _1801);
                vec2 _4313;
                if (_60.gConv.x > 0.5)
                {
                    vec2 _3952 = _1802;
                    _3952.y = 1.0 - _1801;
                    _4313 = _3952;
                }
                else
                {
                    _4313 = _1802;
                }
                vec2 _4315;
                if (_60.gConv.x > 0.5)
                {
                    vec2 _3959 = _1738;
                    _3959.y = 1.0 - _1738.y;
                    _4315 = _3959;
                }
                else
                {
                    _4315 = _1738;
                }
                _4319 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _4310, 0.0) * (_1710.x * _1710.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _4311, 0.0) * (_1714.x * _1710.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _4313, 0.0) * (_1710.x * _1714.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _4315, 0.0) * (_1714.x * _1714.y));
            }
            else
            {
                vec4 _4320;
                if (_635 == 2)
                {
                    vec2 _4303;
                    if (_60.gConv.x > 0.5)
                    {
                        vec2 _3966 = _1726;
                        _3966.y = 1.0 - _1726.y;
                        _4303 = _3966;
                    }
                    else
                    {
                        _4303 = _1726;
                    }
                    float _1913 = _1726.y;
                    vec2 _1914 = vec2(_1738.x, _1913);
                    vec2 _4304;
                    if (_60.gConv.x > 0.5)
                    {
                        vec2 _3973 = _1914;
                        _3973.y = 1.0 - _1913;
                        _4304 = _3973;
                    }
                    else
                    {
                        _4304 = _1914;
                    }
                    float _1931 = _1738.y;
                    vec2 _1932 = vec2(_1726.x, _1931);
                    vec2 _4306;
                    if (_60.gConv.x > 0.5)
                    {
                        vec2 _3980 = _1932;
                        _3980.y = 1.0 - _1931;
                        _4306 = _3980;
                    }
                    else
                    {
                        _4306 = _1932;
                    }
                    vec2 _4308;
                    if (_60.gConv.x > 0.5)
                    {
                        vec2 _3987 = _1738;
                        _3987.y = 1.0 - _1738.y;
                        _4308 = _3987;
                    }
                    else
                    {
                        _4308 = _1738;
                    }
                    _4320 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _4303, 0.0) * (_1710.x * _1710.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _4304, 0.0) * (_1714.x * _1710.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _4306, 0.0) * (_1710.x * _1714.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _4308, 0.0) * (_1714.x * _1714.y));
                }
                else
                {
                    vec4 _4321;
                    if (_635 == 3)
                    {
                        vec2 _4296;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _3994 = _1726;
                            _3994.y = 1.0 - _1726.y;
                            _4296 = _3994;
                        }
                        else
                        {
                            _4296 = _1726;
                        }
                        float _2043 = _1726.y;
                        vec2 _2044 = vec2(_1738.x, _2043);
                        vec2 _4297;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _4001 = _2044;
                            _4001.y = 1.0 - _2043;
                            _4297 = _4001;
                        }
                        else
                        {
                            _4297 = _2044;
                        }
                        float _2061 = _1738.y;
                        vec2 _2062 = vec2(_1726.x, _2061);
                        vec2 _4299;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _4008 = _2062;
                            _4008.y = 1.0 - _2061;
                            _4299 = _4008;
                        }
                        else
                        {
                            _4299 = _2062;
                        }
                        vec2 _4301;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _4015 = _1738;
                            _4015.y = 1.0 - _1738.y;
                            _4301 = _4015;
                        }
                        else
                        {
                            _4301 = _1738;
                        }
                        _4321 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _4296, 0.0) * (_1710.x * _1710.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _4297, 0.0) * (_1714.x * _1710.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _4299, 0.0) * (_1710.x * _1714.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _4301, 0.0) * (_1714.x * _1714.y));
                    }
                    else
                    {
                        vec4 _4322;
                        if (_635 == 4)
                        {
                            vec2 _4289;
                            if (_60.gConv.x > 0.5)
                            {
                                vec2 _4022 = _1726;
                                _4022.y = 1.0 - _1726.y;
                                _4289 = _4022;
                            }
                            else
                            {
                                _4289 = _1726;
                            }
                            float _2173 = _1726.y;
                            vec2 _2174 = vec2(_1738.x, _2173);
                            vec2 _4290;
                            if (_60.gConv.x > 0.5)
                            {
                                vec2 _4029 = _2174;
                                _4029.y = 1.0 - _2173;
                                _4290 = _4029;
                            }
                            else
                            {
                                _4290 = _2174;
                            }
                            float _2191 = _1738.y;
                            vec2 _2192 = vec2(_1726.x, _2191);
                            vec2 _4292;
                            if (_60.gConv.x > 0.5)
                            {
                                vec2 _4036 = _2192;
                                _4036.y = 1.0 - _2191;
                                _4292 = _4036;
                            }
                            else
                            {
                                _4292 = _2192;
                            }
                            vec2 _4294;
                            if (_60.gConv.x > 0.5)
                            {
                                vec2 _4043 = _1738;
                                _4043.y = 1.0 - _1738.y;
                                _4294 = _4043;
                            }
                            else
                            {
                                _4294 = _1738;
                            }
                            _4322 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _4289, 0.0) * (_1710.x * _1710.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _4290, 0.0) * (_1714.x * _1710.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _4292, 0.0) * (_1710.x * _1714.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _4294, 0.0) * (_1714.x * _1714.y));
                        }
                        else
                        {
                            vec2 _4282;
                            if (_60.gConv.x > 0.5)
                            {
                                vec2 _4050 = _1726;
                                _4050.y = 1.0 - _1726.y;
                                _4282 = _4050;
                            }
                            else
                            {
                                _4282 = _1726;
                            }
                            float _2303 = _1726.y;
                            vec2 _2304 = vec2(_1738.x, _2303);
                            vec2 _4283;
                            if (_60.gConv.x > 0.5)
                            {
                                vec2 _4057 = _2304;
                                _4057.y = 1.0 - _2303;
                                _4283 = _4057;
                            }
                            else
                            {
                                _4283 = _2304;
                            }
                            float _2321 = _1738.y;
                            vec2 _2322 = vec2(_1726.x, _2321);
                            vec2 _4285;
                            if (_60.gConv.x > 0.5)
                            {
                                vec2 _4064 = _2322;
                                _4064.y = 1.0 - _2321;
                                _4285 = _4064;
                            }
                            else
                            {
                                _4285 = _2322;
                            }
                            vec2 _4287;
                            if (_60.gConv.x > 0.5)
                            {
                                vec2 _4071 = _1738;
                                _4071.y = 1.0 - _1738.y;
                                _4287 = _4071;
                            }
                            else
                            {
                                _4287 = _1738;
                            }
                            _4322 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _4282, 0.0) * (_1710.x * _1710.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _4283, 0.0) * (_1714.x * _1710.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _4285, 0.0) * (_1710.x * _1714.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _4287, 0.0) * (_1714.x * _1714.y));
                        }
                        _4321 = _4322;
                    }
                    _4320 = _4321;
                }
                _4319 = _4320;
            }
            _4318 = _4319;
        }
        _4366 = mix(_4277, _4318, vec4(_619 - float(_622)));
    }
    else
    {
        _4366 = _4277;
    }
    int _644 = min((_622 + 2), 5);
    int _2422 = clamp(_644, 1, 5);
    vec2 _2491 = (_603 * _60.gLevel[_2422].xy) - vec2(0.5);
    vec2 _2493 = floor(_2491);
    vec2 _2496 = _2491 - _2493;
    vec2 _2499 = _2496 * _2496;
    vec2 _2502 = _2499 * _2496;
    vec2 _2521 = (((_2502 * 3.0) - (_2499 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
    vec2 _2534 = _2502 * 0.16666667163372039794921875;
    vec2 _2537 = (((((-_2502) + (_2499 * 3.0)) - (_2496 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _2521;
    vec2 _2541 = (((((_2502 * (-3.0)) + (_2499 * 3.0)) + (_2496 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _2534;
    vec2 _2553 = ((_2493 - vec2(0.5)) + (_2521 / _2537)) * _60.gLevel[_2422].zw;
    vec2 _2565 = ((_2493 + vec2(1.5)) + (_2534 / _2541)) * _60.gLevel[_2422].zw;
    vec4 _4359;
    SPIRV_CROSS_BRANCH
    if (_644 <= 0)
    {
        vec2 _4358;
        if (_60.gConv.x > 0.5)
        {
            vec2 _4076 = _603;
            _4076.y = 1.0 - _603.y;
            _4358 = _4076;
        }
        else
        {
            _4358 = _603;
        }
        _4359 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _4358, 0.0);
    }
    else
    {
        vec4 _4360;
        if (_644 == 1)
        {
            vec2 _4351;
            if (_60.gConv.x > 0.5)
            {
                vec2 _4081 = _2553;
                _4081.y = 1.0 - _2553.y;
                _4351 = _4081;
            }
            else
            {
                _4351 = _2553;
            }
            float _2610 = _2553.y;
            vec2 _2611 = vec2(_2565.x, _2610);
            vec2 _4352;
            if (_60.gConv.x > 0.5)
            {
                vec2 _4088 = _2611;
                _4088.y = 1.0 - _2610;
                _4352 = _4088;
            }
            else
            {
                _4352 = _2611;
            }
            float _2628 = _2565.y;
            vec2 _2629 = vec2(_2553.x, _2628);
            vec2 _4354;
            if (_60.gConv.x > 0.5)
            {
                vec2 _4095 = _2629;
                _4095.y = 1.0 - _2628;
                _4354 = _4095;
            }
            else
            {
                _4354 = _2629;
            }
            vec2 _4356;
            if (_60.gConv.x > 0.5)
            {
                vec2 _4102 = _2565;
                _4102.y = 1.0 - _2565.y;
                _4356 = _4102;
            }
            else
            {
                _4356 = _2565;
            }
            _4360 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _4351, 0.0) * (_2537.x * _2537.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _4352, 0.0) * (_2541.x * _2537.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _4354, 0.0) * (_2537.x * _2541.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _4356, 0.0) * (_2541.x * _2541.y));
        }
        else
        {
            vec4 _4361;
            if (_644 == 2)
            {
                vec2 _4344;
                if (_60.gConv.x > 0.5)
                {
                    vec2 _4109 = _2553;
                    _4109.y = 1.0 - _2553.y;
                    _4344 = _4109;
                }
                else
                {
                    _4344 = _2553;
                }
                float _2740 = _2553.y;
                vec2 _2741 = vec2(_2565.x, _2740);
                vec2 _4345;
                if (_60.gConv.x > 0.5)
                {
                    vec2 _4116 = _2741;
                    _4116.y = 1.0 - _2740;
                    _4345 = _4116;
                }
                else
                {
                    _4345 = _2741;
                }
                float _2758 = _2565.y;
                vec2 _2759 = vec2(_2553.x, _2758);
                vec2 _4347;
                if (_60.gConv.x > 0.5)
                {
                    vec2 _4123 = _2759;
                    _4123.y = 1.0 - _2758;
                    _4347 = _4123;
                }
                else
                {
                    _4347 = _2759;
                }
                vec2 _4349;
                if (_60.gConv.x > 0.5)
                {
                    vec2 _4130 = _2565;
                    _4130.y = 1.0 - _2565.y;
                    _4349 = _4130;
                }
                else
                {
                    _4349 = _2565;
                }
                _4361 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _4344, 0.0) * (_2537.x * _2537.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _4345, 0.0) * (_2541.x * _2537.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _4347, 0.0) * (_2537.x * _2541.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _4349, 0.0) * (_2541.x * _2541.y));
            }
            else
            {
                vec4 _4362;
                if (_644 == 3)
                {
                    vec2 _4337;
                    if (_60.gConv.x > 0.5)
                    {
                        vec2 _4137 = _2553;
                        _4137.y = 1.0 - _2553.y;
                        _4337 = _4137;
                    }
                    else
                    {
                        _4337 = _2553;
                    }
                    float _2870 = _2553.y;
                    vec2 _2871 = vec2(_2565.x, _2870);
                    vec2 _4338;
                    if (_60.gConv.x > 0.5)
                    {
                        vec2 _4144 = _2871;
                        _4144.y = 1.0 - _2870;
                        _4338 = _4144;
                    }
                    else
                    {
                        _4338 = _2871;
                    }
                    float _2888 = _2565.y;
                    vec2 _2889 = vec2(_2553.x, _2888);
                    vec2 _4340;
                    if (_60.gConv.x > 0.5)
                    {
                        vec2 _4151 = _2889;
                        _4151.y = 1.0 - _2888;
                        _4340 = _4151;
                    }
                    else
                    {
                        _4340 = _2889;
                    }
                    vec2 _4342;
                    if (_60.gConv.x > 0.5)
                    {
                        vec2 _4158 = _2565;
                        _4158.y = 1.0 - _2565.y;
                        _4342 = _4158;
                    }
                    else
                    {
                        _4342 = _2565;
                    }
                    _4362 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _4337, 0.0) * (_2537.x * _2537.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _4338, 0.0) * (_2541.x * _2537.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _4340, 0.0) * (_2537.x * _2541.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _4342, 0.0) * (_2541.x * _2541.y));
                }
                else
                {
                    vec4 _4363;
                    if (_644 == 4)
                    {
                        vec2 _4330;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _4165 = _2553;
                            _4165.y = 1.0 - _2553.y;
                            _4330 = _4165;
                        }
                        else
                        {
                            _4330 = _2553;
                        }
                        float _3000 = _2553.y;
                        vec2 _3001 = vec2(_2565.x, _3000);
                        vec2 _4331;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _4172 = _3001;
                            _4172.y = 1.0 - _3000;
                            _4331 = _4172;
                        }
                        else
                        {
                            _4331 = _3001;
                        }
                        float _3018 = _2565.y;
                        vec2 _3019 = vec2(_2553.x, _3018);
                        vec2 _4333;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _4179 = _3019;
                            _4179.y = 1.0 - _3018;
                            _4333 = _4179;
                        }
                        else
                        {
                            _4333 = _3019;
                        }
                        vec2 _4335;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _4186 = _2565;
                            _4186.y = 1.0 - _2565.y;
                            _4335 = _4186;
                        }
                        else
                        {
                            _4335 = _2565;
                        }
                        _4363 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _4330, 0.0) * (_2537.x * _2537.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _4331, 0.0) * (_2541.x * _2537.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _4333, 0.0) * (_2537.x * _2541.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _4335, 0.0) * (_2541.x * _2541.y));
                    }
                    else
                    {
                        vec2 _4323;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _4193 = _2553;
                            _4193.y = 1.0 - _2553.y;
                            _4323 = _4193;
                        }
                        else
                        {
                            _4323 = _2553;
                        }
                        float _3130 = _2553.y;
                        vec2 _3131 = vec2(_2565.x, _3130);
                        vec2 _4324;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _4200 = _3131;
                            _4200.y = 1.0 - _3130;
                            _4324 = _4200;
                        }
                        else
                        {
                            _4324 = _3131;
                        }
                        float _3148 = _2565.y;
                        vec2 _3149 = vec2(_2553.x, _3148);
                        vec2 _4326;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _4207 = _3149;
                            _4207.y = 1.0 - _3148;
                            _4326 = _4207;
                        }
                        else
                        {
                            _4326 = _3149;
                        }
                        vec2 _4328;
                        if (_60.gConv.x > 0.5)
                        {
                            vec2 _4214 = _2565;
                            _4214.y = 1.0 - _2565.y;
                            _4328 = _4214;
                        }
                        else
                        {
                            _4328 = _2565;
                        }
                        _4363 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _4323, 0.0) * (_2537.x * _2537.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _4324, 0.0) * (_2541.x * _2537.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _4326, 0.0) * (_2537.x * _2541.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _4328, 0.0) * (_2541.x * _2541.y));
                    }
                    _4362 = _4363;
                }
                _4361 = _4362;
            }
            _4360 = _4361;
        }
        _4359 = _4360;
    }
    vec4 _649 = _4366 + (_4359 * 0.3499999940395355224609375);
    vec4 _673 = textureLod(SPIRV_Cross_CombinedgTexgPoint, _4240, 0.0) * _414.gPass1.z;
    vec3 _677 = _673.xyz + max(mix(_649.xyz, (_414.gPass0.xyz * dot(_649.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))) * 1.60000002384185791015625, vec3(_414.gPass0.w)) * _414.gPass1.x, vec3(0.0));
    vec3 _689 = vec3(0.60000002384185791015625) + ((vec3(1.0) - exp((vec3(0.60000002384185791015625) - _677) * vec3(2.5))) * 0.4000000059604644775390625);
    bvec3 _692 = lessThan(_677, vec3(0.60000002384185791015625));
    vec4 _700 = vec4(vec3(_692.x ? _677.x : _689.x, _692.y ? _677.y : _689.y, _692.z ? _677.z : _689.z), _673.w);
    vec4 _4391;
    if (_60.gTime.w > 0.5)
    {
        vec3 _3236 = clamp(_700.xyz, vec3(0.0), vec3(1.0));
        vec3 _3242 = pow((_3236 + vec3(0.054999999701976776123046875)) * vec3(0.947867333889007568359375), vec3(2.400000095367431640625));
        vec3 _3245 = _3236 * vec3(0.077399380505084991455078125);
        bvec3 _3247 = lessThanEqual(_3236, vec3(0.040449999272823333740234375));
        vec3 _3248 = vec3(_3247.x ? _3245.x : _3242.x, _3247.y ? _3245.y : _3242.y, _3247.z ? _3245.z : _3242.z);
        vec4 _4219 = _700;
        _4219.x = _3248.x;
        _4219.y = _3248.y;
        _4219.z = _3248.z;
        _4391 = _4219;
    }
    else
    {
        _4391 = _700;
    }
    _entryPointOutput = _4391;
}

