#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

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

struct WgtPass
{
    float4 gPass0;
    float4 gPass1;
};

struct esia_main_out
{
    float4 _entryPointOutput [[color(0)]];
};

fragment esia_main_out esia_main(constant WgtFrame& _60 [[buffer(0)]], constant WgtPass& _414 [[buffer(1)]], texture2d<float> gTex [[texture(3)]], texture2d<float> gBackdrop0 [[texture(4)]], texture2d<float> gBackdrop1 [[texture(5)]], texture2d<float> gBackdrop2 [[texture(6)]], texture2d<float> gBackdrop3 [[texture(7)]], texture2d<float> gBackdrop4 [[texture(8)]], texture2d<float> gBackdrop5 [[texture(9)]], sampler gLinear [[sampler(11)]], sampler gPoint [[sampler(12)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float2 _724 = gl_FragCoord.xy + _60.gConv.yy;
    float2 _4239;
    if (_60.gConv.x > 0.5)
    {
        float2 _3784 = _724;
        _3784.y = _60.gTarget.y - _724.y;
        _4239 = _3784;
    }
    else
    {
        _4239 = _724;
    }
    float2 _603 = _4239 * _60.gTarget.zw;
    float2 _4240;
    if (_60.gConv.x > 0.5)
    {
        float2 _3787 = _603;
        _3787.y = 1.0 - _603.y;
        _4240 = _3787;
    }
    else
    {
        _4240 = _603;
    }
    float _619 = fast::clamp(log2(fast::max(_414.gPass1.y * _60.gDisplay.z, 2.0)) - 1.0, 1.0, 5.0);
    int _622 = int(floor(_619));
    int _768 = clamp(_622, 1, 5);
    float2 _837 = (_603 * _60.gLevel[_768].xy) - float2(0.5);
    float2 _839 = floor(_837);
    float2 _842 = _837 - _839;
    float2 _845 = _842 * _842;
    float2 _848 = _845 * _842;
    float2 _867 = (((_848 * 3.0) - (_845 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
    float2 _880 = _848 * 0.16666667163372039794921875;
    float2 _883 = (((((-_848) + (_845 * 3.0)) - (_842 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _867;
    float2 _887 = (((((_848 * (-3.0)) + (_845 * 3.0)) + (_842 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _880;
    float2 _899 = ((_839 - float2(0.5)) + (_867 / _883)) * _60.gLevel[_768].zw;
    float2 _911 = ((_839 + float2(1.5)) + (_880 / _887)) * _60.gLevel[_768].zw;
    float4 _4277;
    if (_622 <= 0)
    {
        float2 _4276;
        if (_60.gConv.x > 0.5)
        {
            float2 _3790 = _603;
            _3790.y = 1.0 - _603.y;
            _4276 = _3790;
        }
        else
        {
            _4276 = _603;
        }
        _4277 = gBackdrop0.sample(gLinear, _4276, level(0.0));
    }
    else
    {
        float4 _4278;
        if (_622 == 1)
        {
            float2 _4269;
            if (_60.gConv.x > 0.5)
            {
                float2 _3795 = _899;
                _3795.y = 1.0 - _899.y;
                _4269 = _3795;
            }
            else
            {
                _4269 = _899;
            }
            float _956 = _899.y;
            float2 _957 = float2(_911.x, _956);
            float2 _4270;
            if (_60.gConv.x > 0.5)
            {
                float2 _3802 = _957;
                _3802.y = 1.0 - _956;
                _4270 = _3802;
            }
            else
            {
                _4270 = _957;
            }
            float _974 = _911.y;
            float2 _975 = float2(_899.x, _974);
            float2 _4272;
            if (_60.gConv.x > 0.5)
            {
                float2 _3809 = _975;
                _3809.y = 1.0 - _974;
                _4272 = _3809;
            }
            else
            {
                _4272 = _975;
            }
            float2 _4274;
            if (_60.gConv.x > 0.5)
            {
                float2 _3816 = _911;
                _3816.y = 1.0 - _911.y;
                _4274 = _3816;
            }
            else
            {
                _4274 = _911;
            }
            _4278 = (((gBackdrop1.sample(gLinear, _4269, level(0.0)) * (_883.x * _883.y)) + (gBackdrop1.sample(gLinear, _4270, level(0.0)) * (_887.x * _883.y))) + (gBackdrop1.sample(gLinear, _4272, level(0.0)) * (_883.x * _887.y))) + (gBackdrop1.sample(gLinear, _4274, level(0.0)) * (_887.x * _887.y));
        }
        else
        {
            float4 _4279;
            if (_622 == 2)
            {
                float2 _4262;
                if (_60.gConv.x > 0.5)
                {
                    float2 _3823 = _899;
                    _3823.y = 1.0 - _899.y;
                    _4262 = _3823;
                }
                else
                {
                    _4262 = _899;
                }
                float _1086 = _899.y;
                float2 _1087 = float2(_911.x, _1086);
                float2 _4263;
                if (_60.gConv.x > 0.5)
                {
                    float2 _3830 = _1087;
                    _3830.y = 1.0 - _1086;
                    _4263 = _3830;
                }
                else
                {
                    _4263 = _1087;
                }
                float _1104 = _911.y;
                float2 _1105 = float2(_899.x, _1104);
                float2 _4265;
                if (_60.gConv.x > 0.5)
                {
                    float2 _3837 = _1105;
                    _3837.y = 1.0 - _1104;
                    _4265 = _3837;
                }
                else
                {
                    _4265 = _1105;
                }
                float2 _4267;
                if (_60.gConv.x > 0.5)
                {
                    float2 _3844 = _911;
                    _3844.y = 1.0 - _911.y;
                    _4267 = _3844;
                }
                else
                {
                    _4267 = _911;
                }
                _4279 = (((gBackdrop2.sample(gLinear, _4262, level(0.0)) * (_883.x * _883.y)) + (gBackdrop2.sample(gLinear, _4263, level(0.0)) * (_887.x * _883.y))) + (gBackdrop2.sample(gLinear, _4265, level(0.0)) * (_883.x * _887.y))) + (gBackdrop2.sample(gLinear, _4267, level(0.0)) * (_887.x * _887.y));
            }
            else
            {
                float4 _4280;
                if (_622 == 3)
                {
                    float2 _4255;
                    if (_60.gConv.x > 0.5)
                    {
                        float2 _3851 = _899;
                        _3851.y = 1.0 - _899.y;
                        _4255 = _3851;
                    }
                    else
                    {
                        _4255 = _899;
                    }
                    float _1216 = _899.y;
                    float2 _1217 = float2(_911.x, _1216);
                    float2 _4256;
                    if (_60.gConv.x > 0.5)
                    {
                        float2 _3858 = _1217;
                        _3858.y = 1.0 - _1216;
                        _4256 = _3858;
                    }
                    else
                    {
                        _4256 = _1217;
                    }
                    float _1234 = _911.y;
                    float2 _1235 = float2(_899.x, _1234);
                    float2 _4258;
                    if (_60.gConv.x > 0.5)
                    {
                        float2 _3865 = _1235;
                        _3865.y = 1.0 - _1234;
                        _4258 = _3865;
                    }
                    else
                    {
                        _4258 = _1235;
                    }
                    float2 _4260;
                    if (_60.gConv.x > 0.5)
                    {
                        float2 _3872 = _911;
                        _3872.y = 1.0 - _911.y;
                        _4260 = _3872;
                    }
                    else
                    {
                        _4260 = _911;
                    }
                    _4280 = (((gBackdrop3.sample(gLinear, _4255, level(0.0)) * (_883.x * _883.y)) + (gBackdrop3.sample(gLinear, _4256, level(0.0)) * (_887.x * _883.y))) + (gBackdrop3.sample(gLinear, _4258, level(0.0)) * (_883.x * _887.y))) + (gBackdrop3.sample(gLinear, _4260, level(0.0)) * (_887.x * _887.y));
                }
                else
                {
                    float4 _4281;
                    if (_622 == 4)
                    {
                        float2 _4248;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _3879 = _899;
                            _3879.y = 1.0 - _899.y;
                            _4248 = _3879;
                        }
                        else
                        {
                            _4248 = _899;
                        }
                        float _1346 = _899.y;
                        float2 _1347 = float2(_911.x, _1346);
                        float2 _4249;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _3886 = _1347;
                            _3886.y = 1.0 - _1346;
                            _4249 = _3886;
                        }
                        else
                        {
                            _4249 = _1347;
                        }
                        float _1364 = _911.y;
                        float2 _1365 = float2(_899.x, _1364);
                        float2 _4251;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _3893 = _1365;
                            _3893.y = 1.0 - _1364;
                            _4251 = _3893;
                        }
                        else
                        {
                            _4251 = _1365;
                        }
                        float2 _4253;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _3900 = _911;
                            _3900.y = 1.0 - _911.y;
                            _4253 = _3900;
                        }
                        else
                        {
                            _4253 = _911;
                        }
                        _4281 = (((gBackdrop4.sample(gLinear, _4248, level(0.0)) * (_883.x * _883.y)) + (gBackdrop4.sample(gLinear, _4249, level(0.0)) * (_887.x * _883.y))) + (gBackdrop4.sample(gLinear, _4251, level(0.0)) * (_883.x * _887.y))) + (gBackdrop4.sample(gLinear, _4253, level(0.0)) * (_887.x * _887.y));
                    }
                    else
                    {
                        float2 _4241;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _3907 = _899;
                            _3907.y = 1.0 - _899.y;
                            _4241 = _3907;
                        }
                        else
                        {
                            _4241 = _899;
                        }
                        float _1476 = _899.y;
                        float2 _1477 = float2(_911.x, _1476);
                        float2 _4242;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _3914 = _1477;
                            _3914.y = 1.0 - _1476;
                            _4242 = _3914;
                        }
                        else
                        {
                            _4242 = _1477;
                        }
                        float _1494 = _911.y;
                        float2 _1495 = float2(_899.x, _1494);
                        float2 _4244;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _3921 = _1495;
                            _3921.y = 1.0 - _1494;
                            _4244 = _3921;
                        }
                        else
                        {
                            _4244 = _1495;
                        }
                        float2 _4246;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _3928 = _911;
                            _3928.y = 1.0 - _911.y;
                            _4246 = _3928;
                        }
                        else
                        {
                            _4246 = _911;
                        }
                        _4281 = (((gBackdrop5.sample(gLinear, _4241, level(0.0)) * (_883.x * _883.y)) + (gBackdrop5.sample(gLinear, _4242, level(0.0)) * (_887.x * _883.y))) + (gBackdrop5.sample(gLinear, _4244, level(0.0)) * (_883.x * _887.y))) + (gBackdrop5.sample(gLinear, _4246, level(0.0)) * (_887.x * _887.y));
                    }
                    _4280 = _4281;
                }
                _4279 = _4280;
            }
            _4278 = _4279;
        }
        _4277 = _4278;
    }
    float4 _4366;
    if (_622 < 5)
    {
        int _635 = _622 + 1;
        int _1595 = clamp(_635, 1, 5);
        float2 _1664 = (_603 * _60.gLevel[_1595].xy) - float2(0.5);
        float2 _1666 = floor(_1664);
        float2 _1669 = _1664 - _1666;
        float2 _1672 = _1669 * _1669;
        float2 _1675 = _1672 * _1669;
        float2 _1694 = (((_1675 * 3.0) - (_1672 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
        float2 _1707 = _1675 * 0.16666667163372039794921875;
        float2 _1710 = (((((-_1675) + (_1672 * 3.0)) - (_1669 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _1694;
        float2 _1714 = (((((_1675 * (-3.0)) + (_1672 * 3.0)) + (_1669 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _1707;
        float2 _1726 = ((_1666 - float2(0.5)) + (_1694 / _1710)) * _60.gLevel[_1595].zw;
        float2 _1738 = ((_1666 + float2(1.5)) + (_1707 / _1714)) * _60.gLevel[_1595].zw;
        float4 _4318;
        if (_635 <= 0)
        {
            float2 _4317;
            if (_60.gConv.x > 0.5)
            {
                float2 _3933 = _603;
                _3933.y = 1.0 - _603.y;
                _4317 = _3933;
            }
            else
            {
                _4317 = _603;
            }
            _4318 = gBackdrop0.sample(gLinear, _4317, level(0.0));
        }
        else
        {
            float4 _4319;
            if (_635 == 1)
            {
                float2 _4310;
                if (_60.gConv.x > 0.5)
                {
                    float2 _3938 = _1726;
                    _3938.y = 1.0 - _1726.y;
                    _4310 = _3938;
                }
                else
                {
                    _4310 = _1726;
                }
                float _1783 = _1726.y;
                float2 _1784 = float2(_1738.x, _1783);
                float2 _4311;
                if (_60.gConv.x > 0.5)
                {
                    float2 _3945 = _1784;
                    _3945.y = 1.0 - _1783;
                    _4311 = _3945;
                }
                else
                {
                    _4311 = _1784;
                }
                float _1801 = _1738.y;
                float2 _1802 = float2(_1726.x, _1801);
                float2 _4313;
                if (_60.gConv.x > 0.5)
                {
                    float2 _3952 = _1802;
                    _3952.y = 1.0 - _1801;
                    _4313 = _3952;
                }
                else
                {
                    _4313 = _1802;
                }
                float2 _4315;
                if (_60.gConv.x > 0.5)
                {
                    float2 _3959 = _1738;
                    _3959.y = 1.0 - _1738.y;
                    _4315 = _3959;
                }
                else
                {
                    _4315 = _1738;
                }
                _4319 = (((gBackdrop1.sample(gLinear, _4310, level(0.0)) * (_1710.x * _1710.y)) + (gBackdrop1.sample(gLinear, _4311, level(0.0)) * (_1714.x * _1710.y))) + (gBackdrop1.sample(gLinear, _4313, level(0.0)) * (_1710.x * _1714.y))) + (gBackdrop1.sample(gLinear, _4315, level(0.0)) * (_1714.x * _1714.y));
            }
            else
            {
                float4 _4320;
                if (_635 == 2)
                {
                    float2 _4303;
                    if (_60.gConv.x > 0.5)
                    {
                        float2 _3966 = _1726;
                        _3966.y = 1.0 - _1726.y;
                        _4303 = _3966;
                    }
                    else
                    {
                        _4303 = _1726;
                    }
                    float _1913 = _1726.y;
                    float2 _1914 = float2(_1738.x, _1913);
                    float2 _4304;
                    if (_60.gConv.x > 0.5)
                    {
                        float2 _3973 = _1914;
                        _3973.y = 1.0 - _1913;
                        _4304 = _3973;
                    }
                    else
                    {
                        _4304 = _1914;
                    }
                    float _1931 = _1738.y;
                    float2 _1932 = float2(_1726.x, _1931);
                    float2 _4306;
                    if (_60.gConv.x > 0.5)
                    {
                        float2 _3980 = _1932;
                        _3980.y = 1.0 - _1931;
                        _4306 = _3980;
                    }
                    else
                    {
                        _4306 = _1932;
                    }
                    float2 _4308;
                    if (_60.gConv.x > 0.5)
                    {
                        float2 _3987 = _1738;
                        _3987.y = 1.0 - _1738.y;
                        _4308 = _3987;
                    }
                    else
                    {
                        _4308 = _1738;
                    }
                    _4320 = (((gBackdrop2.sample(gLinear, _4303, level(0.0)) * (_1710.x * _1710.y)) + (gBackdrop2.sample(gLinear, _4304, level(0.0)) * (_1714.x * _1710.y))) + (gBackdrop2.sample(gLinear, _4306, level(0.0)) * (_1710.x * _1714.y))) + (gBackdrop2.sample(gLinear, _4308, level(0.0)) * (_1714.x * _1714.y));
                }
                else
                {
                    float4 _4321;
                    if (_635 == 3)
                    {
                        float2 _4296;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _3994 = _1726;
                            _3994.y = 1.0 - _1726.y;
                            _4296 = _3994;
                        }
                        else
                        {
                            _4296 = _1726;
                        }
                        float _2043 = _1726.y;
                        float2 _2044 = float2(_1738.x, _2043);
                        float2 _4297;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _4001 = _2044;
                            _4001.y = 1.0 - _2043;
                            _4297 = _4001;
                        }
                        else
                        {
                            _4297 = _2044;
                        }
                        float _2061 = _1738.y;
                        float2 _2062 = float2(_1726.x, _2061);
                        float2 _4299;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _4008 = _2062;
                            _4008.y = 1.0 - _2061;
                            _4299 = _4008;
                        }
                        else
                        {
                            _4299 = _2062;
                        }
                        float2 _4301;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _4015 = _1738;
                            _4015.y = 1.0 - _1738.y;
                            _4301 = _4015;
                        }
                        else
                        {
                            _4301 = _1738;
                        }
                        _4321 = (((gBackdrop3.sample(gLinear, _4296, level(0.0)) * (_1710.x * _1710.y)) + (gBackdrop3.sample(gLinear, _4297, level(0.0)) * (_1714.x * _1710.y))) + (gBackdrop3.sample(gLinear, _4299, level(0.0)) * (_1710.x * _1714.y))) + (gBackdrop3.sample(gLinear, _4301, level(0.0)) * (_1714.x * _1714.y));
                    }
                    else
                    {
                        float4 _4322;
                        if (_635 == 4)
                        {
                            float2 _4289;
                            if (_60.gConv.x > 0.5)
                            {
                                float2 _4022 = _1726;
                                _4022.y = 1.0 - _1726.y;
                                _4289 = _4022;
                            }
                            else
                            {
                                _4289 = _1726;
                            }
                            float _2173 = _1726.y;
                            float2 _2174 = float2(_1738.x, _2173);
                            float2 _4290;
                            if (_60.gConv.x > 0.5)
                            {
                                float2 _4029 = _2174;
                                _4029.y = 1.0 - _2173;
                                _4290 = _4029;
                            }
                            else
                            {
                                _4290 = _2174;
                            }
                            float _2191 = _1738.y;
                            float2 _2192 = float2(_1726.x, _2191);
                            float2 _4292;
                            if (_60.gConv.x > 0.5)
                            {
                                float2 _4036 = _2192;
                                _4036.y = 1.0 - _2191;
                                _4292 = _4036;
                            }
                            else
                            {
                                _4292 = _2192;
                            }
                            float2 _4294;
                            if (_60.gConv.x > 0.5)
                            {
                                float2 _4043 = _1738;
                                _4043.y = 1.0 - _1738.y;
                                _4294 = _4043;
                            }
                            else
                            {
                                _4294 = _1738;
                            }
                            _4322 = (((gBackdrop4.sample(gLinear, _4289, level(0.0)) * (_1710.x * _1710.y)) + (gBackdrop4.sample(gLinear, _4290, level(0.0)) * (_1714.x * _1710.y))) + (gBackdrop4.sample(gLinear, _4292, level(0.0)) * (_1710.x * _1714.y))) + (gBackdrop4.sample(gLinear, _4294, level(0.0)) * (_1714.x * _1714.y));
                        }
                        else
                        {
                            float2 _4282;
                            if (_60.gConv.x > 0.5)
                            {
                                float2 _4050 = _1726;
                                _4050.y = 1.0 - _1726.y;
                                _4282 = _4050;
                            }
                            else
                            {
                                _4282 = _1726;
                            }
                            float _2303 = _1726.y;
                            float2 _2304 = float2(_1738.x, _2303);
                            float2 _4283;
                            if (_60.gConv.x > 0.5)
                            {
                                float2 _4057 = _2304;
                                _4057.y = 1.0 - _2303;
                                _4283 = _4057;
                            }
                            else
                            {
                                _4283 = _2304;
                            }
                            float _2321 = _1738.y;
                            float2 _2322 = float2(_1726.x, _2321);
                            float2 _4285;
                            if (_60.gConv.x > 0.5)
                            {
                                float2 _4064 = _2322;
                                _4064.y = 1.0 - _2321;
                                _4285 = _4064;
                            }
                            else
                            {
                                _4285 = _2322;
                            }
                            float2 _4287;
                            if (_60.gConv.x > 0.5)
                            {
                                float2 _4071 = _1738;
                                _4071.y = 1.0 - _1738.y;
                                _4287 = _4071;
                            }
                            else
                            {
                                _4287 = _1738;
                            }
                            _4322 = (((gBackdrop5.sample(gLinear, _4282, level(0.0)) * (_1710.x * _1710.y)) + (gBackdrop5.sample(gLinear, _4283, level(0.0)) * (_1714.x * _1710.y))) + (gBackdrop5.sample(gLinear, _4285, level(0.0)) * (_1710.x * _1714.y))) + (gBackdrop5.sample(gLinear, _4287, level(0.0)) * (_1714.x * _1714.y));
                        }
                        _4321 = _4322;
                    }
                    _4320 = _4321;
                }
                _4319 = _4320;
            }
            _4318 = _4319;
        }
        _4366 = mix(_4277, _4318, float4(_619 - float(_622)));
    }
    else
    {
        _4366 = _4277;
    }
    int _644 = min((_622 + 2), 5);
    int _2422 = clamp(_644, 1, 5);
    float2 _2491 = (_603 * _60.gLevel[_2422].xy) - float2(0.5);
    float2 _2493 = floor(_2491);
    float2 _2496 = _2491 - _2493;
    float2 _2499 = _2496 * _2496;
    float2 _2502 = _2499 * _2496;
    float2 _2521 = (((_2502 * 3.0) - (_2499 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
    float2 _2534 = _2502 * 0.16666667163372039794921875;
    float2 _2537 = (((((-_2502) + (_2499 * 3.0)) - (_2496 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _2521;
    float2 _2541 = (((((_2502 * (-3.0)) + (_2499 * 3.0)) + (_2496 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _2534;
    float2 _2553 = ((_2493 - float2(0.5)) + (_2521 / _2537)) * _60.gLevel[_2422].zw;
    float2 _2565 = ((_2493 + float2(1.5)) + (_2534 / _2541)) * _60.gLevel[_2422].zw;
    float4 _4359;
    if (_644 <= 0)
    {
        float2 _4358;
        if (_60.gConv.x > 0.5)
        {
            float2 _4076 = _603;
            _4076.y = 1.0 - _603.y;
            _4358 = _4076;
        }
        else
        {
            _4358 = _603;
        }
        _4359 = gBackdrop0.sample(gLinear, _4358, level(0.0));
    }
    else
    {
        float4 _4360;
        if (_644 == 1)
        {
            float2 _4351;
            if (_60.gConv.x > 0.5)
            {
                float2 _4081 = _2553;
                _4081.y = 1.0 - _2553.y;
                _4351 = _4081;
            }
            else
            {
                _4351 = _2553;
            }
            float _2610 = _2553.y;
            float2 _2611 = float2(_2565.x, _2610);
            float2 _4352;
            if (_60.gConv.x > 0.5)
            {
                float2 _4088 = _2611;
                _4088.y = 1.0 - _2610;
                _4352 = _4088;
            }
            else
            {
                _4352 = _2611;
            }
            float _2628 = _2565.y;
            float2 _2629 = float2(_2553.x, _2628);
            float2 _4354;
            if (_60.gConv.x > 0.5)
            {
                float2 _4095 = _2629;
                _4095.y = 1.0 - _2628;
                _4354 = _4095;
            }
            else
            {
                _4354 = _2629;
            }
            float2 _4356;
            if (_60.gConv.x > 0.5)
            {
                float2 _4102 = _2565;
                _4102.y = 1.0 - _2565.y;
                _4356 = _4102;
            }
            else
            {
                _4356 = _2565;
            }
            _4360 = (((gBackdrop1.sample(gLinear, _4351, level(0.0)) * (_2537.x * _2537.y)) + (gBackdrop1.sample(gLinear, _4352, level(0.0)) * (_2541.x * _2537.y))) + (gBackdrop1.sample(gLinear, _4354, level(0.0)) * (_2537.x * _2541.y))) + (gBackdrop1.sample(gLinear, _4356, level(0.0)) * (_2541.x * _2541.y));
        }
        else
        {
            float4 _4361;
            if (_644 == 2)
            {
                float2 _4344;
                if (_60.gConv.x > 0.5)
                {
                    float2 _4109 = _2553;
                    _4109.y = 1.0 - _2553.y;
                    _4344 = _4109;
                }
                else
                {
                    _4344 = _2553;
                }
                float _2740 = _2553.y;
                float2 _2741 = float2(_2565.x, _2740);
                float2 _4345;
                if (_60.gConv.x > 0.5)
                {
                    float2 _4116 = _2741;
                    _4116.y = 1.0 - _2740;
                    _4345 = _4116;
                }
                else
                {
                    _4345 = _2741;
                }
                float _2758 = _2565.y;
                float2 _2759 = float2(_2553.x, _2758);
                float2 _4347;
                if (_60.gConv.x > 0.5)
                {
                    float2 _4123 = _2759;
                    _4123.y = 1.0 - _2758;
                    _4347 = _4123;
                }
                else
                {
                    _4347 = _2759;
                }
                float2 _4349;
                if (_60.gConv.x > 0.5)
                {
                    float2 _4130 = _2565;
                    _4130.y = 1.0 - _2565.y;
                    _4349 = _4130;
                }
                else
                {
                    _4349 = _2565;
                }
                _4361 = (((gBackdrop2.sample(gLinear, _4344, level(0.0)) * (_2537.x * _2537.y)) + (gBackdrop2.sample(gLinear, _4345, level(0.0)) * (_2541.x * _2537.y))) + (gBackdrop2.sample(gLinear, _4347, level(0.0)) * (_2537.x * _2541.y))) + (gBackdrop2.sample(gLinear, _4349, level(0.0)) * (_2541.x * _2541.y));
            }
            else
            {
                float4 _4362;
                if (_644 == 3)
                {
                    float2 _4337;
                    if (_60.gConv.x > 0.5)
                    {
                        float2 _4137 = _2553;
                        _4137.y = 1.0 - _2553.y;
                        _4337 = _4137;
                    }
                    else
                    {
                        _4337 = _2553;
                    }
                    float _2870 = _2553.y;
                    float2 _2871 = float2(_2565.x, _2870);
                    float2 _4338;
                    if (_60.gConv.x > 0.5)
                    {
                        float2 _4144 = _2871;
                        _4144.y = 1.0 - _2870;
                        _4338 = _4144;
                    }
                    else
                    {
                        _4338 = _2871;
                    }
                    float _2888 = _2565.y;
                    float2 _2889 = float2(_2553.x, _2888);
                    float2 _4340;
                    if (_60.gConv.x > 0.5)
                    {
                        float2 _4151 = _2889;
                        _4151.y = 1.0 - _2888;
                        _4340 = _4151;
                    }
                    else
                    {
                        _4340 = _2889;
                    }
                    float2 _4342;
                    if (_60.gConv.x > 0.5)
                    {
                        float2 _4158 = _2565;
                        _4158.y = 1.0 - _2565.y;
                        _4342 = _4158;
                    }
                    else
                    {
                        _4342 = _2565;
                    }
                    _4362 = (((gBackdrop3.sample(gLinear, _4337, level(0.0)) * (_2537.x * _2537.y)) + (gBackdrop3.sample(gLinear, _4338, level(0.0)) * (_2541.x * _2537.y))) + (gBackdrop3.sample(gLinear, _4340, level(0.0)) * (_2537.x * _2541.y))) + (gBackdrop3.sample(gLinear, _4342, level(0.0)) * (_2541.x * _2541.y));
                }
                else
                {
                    float4 _4363;
                    if (_644 == 4)
                    {
                        float2 _4330;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _4165 = _2553;
                            _4165.y = 1.0 - _2553.y;
                            _4330 = _4165;
                        }
                        else
                        {
                            _4330 = _2553;
                        }
                        float _3000 = _2553.y;
                        float2 _3001 = float2(_2565.x, _3000);
                        float2 _4331;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _4172 = _3001;
                            _4172.y = 1.0 - _3000;
                            _4331 = _4172;
                        }
                        else
                        {
                            _4331 = _3001;
                        }
                        float _3018 = _2565.y;
                        float2 _3019 = float2(_2553.x, _3018);
                        float2 _4333;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _4179 = _3019;
                            _4179.y = 1.0 - _3018;
                            _4333 = _4179;
                        }
                        else
                        {
                            _4333 = _3019;
                        }
                        float2 _4335;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _4186 = _2565;
                            _4186.y = 1.0 - _2565.y;
                            _4335 = _4186;
                        }
                        else
                        {
                            _4335 = _2565;
                        }
                        _4363 = (((gBackdrop4.sample(gLinear, _4330, level(0.0)) * (_2537.x * _2537.y)) + (gBackdrop4.sample(gLinear, _4331, level(0.0)) * (_2541.x * _2537.y))) + (gBackdrop4.sample(gLinear, _4333, level(0.0)) * (_2537.x * _2541.y))) + (gBackdrop4.sample(gLinear, _4335, level(0.0)) * (_2541.x * _2541.y));
                    }
                    else
                    {
                        float2 _4323;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _4193 = _2553;
                            _4193.y = 1.0 - _2553.y;
                            _4323 = _4193;
                        }
                        else
                        {
                            _4323 = _2553;
                        }
                        float _3130 = _2553.y;
                        float2 _3131 = float2(_2565.x, _3130);
                        float2 _4324;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _4200 = _3131;
                            _4200.y = 1.0 - _3130;
                            _4324 = _4200;
                        }
                        else
                        {
                            _4324 = _3131;
                        }
                        float _3148 = _2565.y;
                        float2 _3149 = float2(_2553.x, _3148);
                        float2 _4326;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _4207 = _3149;
                            _4207.y = 1.0 - _3148;
                            _4326 = _4207;
                        }
                        else
                        {
                            _4326 = _3149;
                        }
                        float2 _4328;
                        if (_60.gConv.x > 0.5)
                        {
                            float2 _4214 = _2565;
                            _4214.y = 1.0 - _2565.y;
                            _4328 = _4214;
                        }
                        else
                        {
                            _4328 = _2565;
                        }
                        _4363 = (((gBackdrop5.sample(gLinear, _4323, level(0.0)) * (_2537.x * _2537.y)) + (gBackdrop5.sample(gLinear, _4324, level(0.0)) * (_2541.x * _2537.y))) + (gBackdrop5.sample(gLinear, _4326, level(0.0)) * (_2537.x * _2541.y))) + (gBackdrop5.sample(gLinear, _4328, level(0.0)) * (_2541.x * _2541.y));
                    }
                    _4362 = _4363;
                }
                _4361 = _4362;
            }
            _4360 = _4361;
        }
        _4359 = _4360;
    }
    float4 _649 = _4366 + (_4359 * 0.3499999940395355224609375);
    float4 _673 = gTex.sample(gPoint, _4240, level(0.0)) * _414.gPass1.z;
    float3 _677 = _673.xyz + fast::max(mix(_649.xyz, (_414.gPass0.xyz * dot(_649.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))) * 1.60000002384185791015625, float3(_414.gPass0.w)) * _414.gPass1.x, float3(0.0));
    float4 _700 = float4(select(float3(0.60000002384185791015625) + ((float3(1.0) - exp((float3(0.60000002384185791015625) - _677) * float3(2.5))) * 0.4000000059604644775390625), _677, _677 < float3(0.60000002384185791015625)), _673.w);
    float4 _4391;
    if (_60.gTime.w > 0.5)
    {
        float3 _3236 = fast::clamp(_700.xyz, float3(0.0), float3(1.0));
        float3 _3248 = select(pow((_3236 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _3236 * float3(0.077399380505084991455078125), _3236 <= float3(0.040449999272823333740234375));
        float4 _4219 = _700;
        _4219.x = _3248.x;
        _4219.y = _3248.y;
        _4219.z = _3248.z;
        _4391 = _4219;
    }
    else
    {
        _4391 = _700;
    }
    out._entryPointOutput = _4391;
    return out;
}

