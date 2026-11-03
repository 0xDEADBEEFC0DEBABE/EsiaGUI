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

fragment esia_main_out esia_main(constant WgtFrame& _89 [[buffer(0)]], constant WgtPass& _662 [[buffer(1)]], texture2d<float> gTex [[texture(3)]], texture2d<float> gBackdrop0 [[texture(4)]], texture2d<float> gBackdrop1 [[texture(5)]], texture2d<float> gBackdrop2 [[texture(6)]], texture2d<float> gBackdrop3 [[texture(7)]], texture2d<float> gBackdrop4 [[texture(8)]], texture2d<float> gBackdrop5 [[texture(9)]], sampler gLinear [[sampler(11)]], sampler gPoint [[sampler(12)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float2 _949 = gl_FragCoord.xy + _89.gConv.yy;
    float2 _3560;
    if (_89.gConv.x > 0.5)
    {
        float2 _3229 = _949;
        _3229.y = _89.gTarget.y - _949.y;
        _3560 = _3229;
    }
    else
    {
        _3560 = _949;
    }
    float2 _835 = _3560 * _89.gTarget.zw;
    float2 _3561;
    if (_89.gConv.x > 0.5)
    {
        float2 _3232 = _835;
        _3232.y = 1.0 - _835.y;
        _3561 = _3232;
    }
    else
    {
        _3561 = _835;
    }
    float4 _841 = gTex.sample(gPoint, _3561, level(0.0));
    float4 _3652;
    if (_662.gPass1.y > 0.0)
    {
        int _861 = ((int(_662.gPass1.y > 1.756000041961669921875) + int(_662.gPass1.y > 3.7530000209808349609375)) + int(_662.gPass1.y > 7.620999813079833984375)) + int(_662.gPass1.y > 15.30000019073486328125);
        float _991 = (_861 <= 0) ? 0.0 : ((_861 == 1) ? 1.756000041961669921875 : ((_861 == 2) ? 3.7530000209808349609375 : ((_861 == 3) ? 7.620999813079833984375 : ((_861 == 4) ? 15.30000019073486328125 : 30.6280002593994140625))));
        int _865 = _861 + 1;
        float _1008 = (_865 <= 0) ? 0.0 : ((_865 == 1) ? 1.756000041961669921875 : ((_865 == 2) ? 3.7530000209808349609375 : ((_865 == 3) ? 7.620999813079833984375 : ((_865 == 4) ? 15.30000019073486328125 : 30.6280002593994140625))));
        float4 _3603;
        if (_861 > 0)
        {
            int _1027 = clamp(_861, 1, 5);
            float2 _1096 = (_835 * _89.gLevel[_1027].xy) - float2(0.5);
            float2 _1098 = floor(_1096);
            float2 _1101 = _1096 - _1098;
            float2 _1104 = _1101 * _1101;
            float2 _1107 = _1104 * _1101;
            float2 _1126 = (((_1107 * 3.0) - (_1104 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
            float2 _1139 = _1107 * 0.16666667163372039794921875;
            float2 _1142 = (((((-_1107) + (_1104 * 3.0)) - (_1101 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _1126;
            float2 _1146 = (((((_1107 * (-3.0)) + (_1104 * 3.0)) + (_1101 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _1139;
            float2 _1158 = ((_1098 - float2(0.5)) + (_1126 / _1142)) * _89.gLevel[_1027].zw;
            float2 _1170 = ((_1098 + float2(1.5)) + (_1139 / _1146)) * _89.gLevel[_1027].zw;
            float4 _3598;
            if (_861 <= 0)
            {
                float2 _3597;
                if (_89.gConv.x > 0.5)
                {
                    float2 _3235 = _835;
                    _3235.y = 1.0 - _835.y;
                    _3597 = _3235;
                }
                else
                {
                    _3597 = _835;
                }
                _3598 = gBackdrop0.sample(gLinear, _3597, level(0.0));
            }
            else
            {
                float4 _3599;
                if (_861 == 1)
                {
                    float2 _3590;
                    if (_89.gConv.x > 0.5)
                    {
                        float2 _3240 = _1158;
                        _3240.y = 1.0 - _1158.y;
                        _3590 = _3240;
                    }
                    else
                    {
                        _3590 = _1158;
                    }
                    float _1215 = _1158.y;
                    float2 _1216 = float2(_1170.x, _1215);
                    float2 _3591;
                    if (_89.gConv.x > 0.5)
                    {
                        float2 _3247 = _1216;
                        _3247.y = 1.0 - _1215;
                        _3591 = _3247;
                    }
                    else
                    {
                        _3591 = _1216;
                    }
                    float _1233 = _1170.y;
                    float2 _1234 = float2(_1158.x, _1233);
                    float2 _3593;
                    if (_89.gConv.x > 0.5)
                    {
                        float2 _3254 = _1234;
                        _3254.y = 1.0 - _1233;
                        _3593 = _3254;
                    }
                    else
                    {
                        _3593 = _1234;
                    }
                    float2 _3595;
                    if (_89.gConv.x > 0.5)
                    {
                        float2 _3261 = _1170;
                        _3261.y = 1.0 - _1170.y;
                        _3595 = _3261;
                    }
                    else
                    {
                        _3595 = _1170;
                    }
                    _3599 = (((gBackdrop1.sample(gLinear, _3590, level(0.0)) * (_1142.x * _1142.y)) + (gBackdrop1.sample(gLinear, _3591, level(0.0)) * (_1146.x * _1142.y))) + (gBackdrop1.sample(gLinear, _3593, level(0.0)) * (_1142.x * _1146.y))) + (gBackdrop1.sample(gLinear, _3595, level(0.0)) * (_1146.x * _1146.y));
                }
                else
                {
                    float4 _3600;
                    if (_861 == 2)
                    {
                        float2 _3583;
                        if (_89.gConv.x > 0.5)
                        {
                            float2 _3268 = _1158;
                            _3268.y = 1.0 - _1158.y;
                            _3583 = _3268;
                        }
                        else
                        {
                            _3583 = _1158;
                        }
                        float _1345 = _1158.y;
                        float2 _1346 = float2(_1170.x, _1345);
                        float2 _3584;
                        if (_89.gConv.x > 0.5)
                        {
                            float2 _3275 = _1346;
                            _3275.y = 1.0 - _1345;
                            _3584 = _3275;
                        }
                        else
                        {
                            _3584 = _1346;
                        }
                        float _1363 = _1170.y;
                        float2 _1364 = float2(_1158.x, _1363);
                        float2 _3586;
                        if (_89.gConv.x > 0.5)
                        {
                            float2 _3282 = _1364;
                            _3282.y = 1.0 - _1363;
                            _3586 = _3282;
                        }
                        else
                        {
                            _3586 = _1364;
                        }
                        float2 _3588;
                        if (_89.gConv.x > 0.5)
                        {
                            float2 _3289 = _1170;
                            _3289.y = 1.0 - _1170.y;
                            _3588 = _3289;
                        }
                        else
                        {
                            _3588 = _1170;
                        }
                        _3600 = (((gBackdrop2.sample(gLinear, _3583, level(0.0)) * (_1142.x * _1142.y)) + (gBackdrop2.sample(gLinear, _3584, level(0.0)) * (_1146.x * _1142.y))) + (gBackdrop2.sample(gLinear, _3586, level(0.0)) * (_1142.x * _1146.y))) + (gBackdrop2.sample(gLinear, _3588, level(0.0)) * (_1146.x * _1146.y));
                    }
                    else
                    {
                        float4 _3601;
                        if (_861 == 3)
                        {
                            float2 _3576;
                            if (_89.gConv.x > 0.5)
                            {
                                float2 _3296 = _1158;
                                _3296.y = 1.0 - _1158.y;
                                _3576 = _3296;
                            }
                            else
                            {
                                _3576 = _1158;
                            }
                            float _1475 = _1158.y;
                            float2 _1476 = float2(_1170.x, _1475);
                            float2 _3577;
                            if (_89.gConv.x > 0.5)
                            {
                                float2 _3303 = _1476;
                                _3303.y = 1.0 - _1475;
                                _3577 = _3303;
                            }
                            else
                            {
                                _3577 = _1476;
                            }
                            float _1493 = _1170.y;
                            float2 _1494 = float2(_1158.x, _1493);
                            float2 _3579;
                            if (_89.gConv.x > 0.5)
                            {
                                float2 _3310 = _1494;
                                _3310.y = 1.0 - _1493;
                                _3579 = _3310;
                            }
                            else
                            {
                                _3579 = _1494;
                            }
                            float2 _3581;
                            if (_89.gConv.x > 0.5)
                            {
                                float2 _3317 = _1170;
                                _3317.y = 1.0 - _1170.y;
                                _3581 = _3317;
                            }
                            else
                            {
                                _3581 = _1170;
                            }
                            _3601 = (((gBackdrop3.sample(gLinear, _3576, level(0.0)) * (_1142.x * _1142.y)) + (gBackdrop3.sample(gLinear, _3577, level(0.0)) * (_1146.x * _1142.y))) + (gBackdrop3.sample(gLinear, _3579, level(0.0)) * (_1142.x * _1146.y))) + (gBackdrop3.sample(gLinear, _3581, level(0.0)) * (_1146.x * _1146.y));
                        }
                        else
                        {
                            float4 _3602;
                            if (_861 == 4)
                            {
                                float2 _3569;
                                if (_89.gConv.x > 0.5)
                                {
                                    float2 _3324 = _1158;
                                    _3324.y = 1.0 - _1158.y;
                                    _3569 = _3324;
                                }
                                else
                                {
                                    _3569 = _1158;
                                }
                                float _1605 = _1158.y;
                                float2 _1606 = float2(_1170.x, _1605);
                                float2 _3570;
                                if (_89.gConv.x > 0.5)
                                {
                                    float2 _3331 = _1606;
                                    _3331.y = 1.0 - _1605;
                                    _3570 = _3331;
                                }
                                else
                                {
                                    _3570 = _1606;
                                }
                                float _1623 = _1170.y;
                                float2 _1624 = float2(_1158.x, _1623);
                                float2 _3572;
                                if (_89.gConv.x > 0.5)
                                {
                                    float2 _3338 = _1624;
                                    _3338.y = 1.0 - _1623;
                                    _3572 = _3338;
                                }
                                else
                                {
                                    _3572 = _1624;
                                }
                                float2 _3574;
                                if (_89.gConv.x > 0.5)
                                {
                                    float2 _3345 = _1170;
                                    _3345.y = 1.0 - _1170.y;
                                    _3574 = _3345;
                                }
                                else
                                {
                                    _3574 = _1170;
                                }
                                _3602 = (((gBackdrop4.sample(gLinear, _3569, level(0.0)) * (_1142.x * _1142.y)) + (gBackdrop4.sample(gLinear, _3570, level(0.0)) * (_1146.x * _1142.y))) + (gBackdrop4.sample(gLinear, _3572, level(0.0)) * (_1142.x * _1146.y))) + (gBackdrop4.sample(gLinear, _3574, level(0.0)) * (_1146.x * _1146.y));
                            }
                            else
                            {
                                float2 _3562;
                                if (_89.gConv.x > 0.5)
                                {
                                    float2 _3352 = _1158;
                                    _3352.y = 1.0 - _1158.y;
                                    _3562 = _3352;
                                }
                                else
                                {
                                    _3562 = _1158;
                                }
                                float _1735 = _1158.y;
                                float2 _1736 = float2(_1170.x, _1735);
                                float2 _3563;
                                if (_89.gConv.x > 0.5)
                                {
                                    float2 _3359 = _1736;
                                    _3359.y = 1.0 - _1735;
                                    _3563 = _3359;
                                }
                                else
                                {
                                    _3563 = _1736;
                                }
                                float _1753 = _1170.y;
                                float2 _1754 = float2(_1158.x, _1753);
                                float2 _3565;
                                if (_89.gConv.x > 0.5)
                                {
                                    float2 _3366 = _1754;
                                    _3366.y = 1.0 - _1753;
                                    _3565 = _3366;
                                }
                                else
                                {
                                    _3565 = _1754;
                                }
                                float2 _3567;
                                if (_89.gConv.x > 0.5)
                                {
                                    float2 _3373 = _1170;
                                    _3373.y = 1.0 - _1170.y;
                                    _3567 = _3373;
                                }
                                else
                                {
                                    _3567 = _1170;
                                }
                                _3602 = (((gBackdrop5.sample(gLinear, _3562, level(0.0)) * (_1142.x * _1142.y)) + (gBackdrop5.sample(gLinear, _3563, level(0.0)) * (_1146.x * _1142.y))) + (gBackdrop5.sample(gLinear, _3565, level(0.0)) * (_1142.x * _1146.y))) + (gBackdrop5.sample(gLinear, _3567, level(0.0)) * (_1146.x * _1146.y));
                            }
                            _3601 = _3602;
                        }
                        _3600 = _3601;
                    }
                    _3599 = _3600;
                }
                _3598 = _3599;
            }
            _3603 = _3598;
        }
        else
        {
            _3603 = _841;
        }
        int _893 = _861 + 1;
        int _1854 = clamp(_893, 1, 5);
        float2 _1923 = (_835 * _89.gLevel[_1854].xy) - float2(0.5);
        float2 _1925 = floor(_1923);
        float2 _1928 = _1923 - _1925;
        float2 _1931 = _1928 * _1928;
        float2 _1934 = _1931 * _1928;
        float2 _1953 = (((_1934 * 3.0) - (_1931 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
        float2 _1966 = _1934 * 0.16666667163372039794921875;
        float2 _1969 = (((((-_1934) + (_1931 * 3.0)) - (_1928 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _1953;
        float2 _1973 = (((((_1934 * (-3.0)) + (_1931 * 3.0)) + (_1928 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _1966;
        float2 _1985 = ((_1925 - float2(0.5)) + (_1953 / _1969)) * _89.gLevel[_1854].zw;
        float2 _1997 = ((_1925 + float2(1.5)) + (_1966 / _1973)) * _89.gLevel[_1854].zw;
        float4 _3640;
        if (_893 <= 0)
        {
            float2 _3639;
            if (_89.gConv.x > 0.5)
            {
                float2 _3378 = _835;
                _3378.y = 1.0 - _835.y;
                _3639 = _3378;
            }
            else
            {
                _3639 = _835;
            }
            _3640 = gBackdrop0.sample(gLinear, _3639, level(0.0));
        }
        else
        {
            float4 _3641;
            if (_893 == 1)
            {
                float2 _3632;
                if (_89.gConv.x > 0.5)
                {
                    float2 _3383 = _1985;
                    _3383.y = 1.0 - _1985.y;
                    _3632 = _3383;
                }
                else
                {
                    _3632 = _1985;
                }
                float _2042 = _1985.y;
                float2 _2043 = float2(_1997.x, _2042);
                float2 _3633;
                if (_89.gConv.x > 0.5)
                {
                    float2 _3390 = _2043;
                    _3390.y = 1.0 - _2042;
                    _3633 = _3390;
                }
                else
                {
                    _3633 = _2043;
                }
                float _2060 = _1997.y;
                float2 _2061 = float2(_1985.x, _2060);
                float2 _3635;
                if (_89.gConv.x > 0.5)
                {
                    float2 _3397 = _2061;
                    _3397.y = 1.0 - _2060;
                    _3635 = _3397;
                }
                else
                {
                    _3635 = _2061;
                }
                float2 _3637;
                if (_89.gConv.x > 0.5)
                {
                    float2 _3404 = _1997;
                    _3404.y = 1.0 - _1997.y;
                    _3637 = _3404;
                }
                else
                {
                    _3637 = _1997;
                }
                _3641 = (((gBackdrop1.sample(gLinear, _3632, level(0.0)) * (_1969.x * _1969.y)) + (gBackdrop1.sample(gLinear, _3633, level(0.0)) * (_1973.x * _1969.y))) + (gBackdrop1.sample(gLinear, _3635, level(0.0)) * (_1969.x * _1973.y))) + (gBackdrop1.sample(gLinear, _3637, level(0.0)) * (_1973.x * _1973.y));
            }
            else
            {
                float4 _3642;
                if (_893 == 2)
                {
                    float2 _3625;
                    if (_89.gConv.x > 0.5)
                    {
                        float2 _3411 = _1985;
                        _3411.y = 1.0 - _1985.y;
                        _3625 = _3411;
                    }
                    else
                    {
                        _3625 = _1985;
                    }
                    float _2172 = _1985.y;
                    float2 _2173 = float2(_1997.x, _2172);
                    float2 _3626;
                    if (_89.gConv.x > 0.5)
                    {
                        float2 _3418 = _2173;
                        _3418.y = 1.0 - _2172;
                        _3626 = _3418;
                    }
                    else
                    {
                        _3626 = _2173;
                    }
                    float _2190 = _1997.y;
                    float2 _2191 = float2(_1985.x, _2190);
                    float2 _3628;
                    if (_89.gConv.x > 0.5)
                    {
                        float2 _3425 = _2191;
                        _3425.y = 1.0 - _2190;
                        _3628 = _3425;
                    }
                    else
                    {
                        _3628 = _2191;
                    }
                    float2 _3630;
                    if (_89.gConv.x > 0.5)
                    {
                        float2 _3432 = _1997;
                        _3432.y = 1.0 - _1997.y;
                        _3630 = _3432;
                    }
                    else
                    {
                        _3630 = _1997;
                    }
                    _3642 = (((gBackdrop2.sample(gLinear, _3625, level(0.0)) * (_1969.x * _1969.y)) + (gBackdrop2.sample(gLinear, _3626, level(0.0)) * (_1973.x * _1969.y))) + (gBackdrop2.sample(gLinear, _3628, level(0.0)) * (_1969.x * _1973.y))) + (gBackdrop2.sample(gLinear, _3630, level(0.0)) * (_1973.x * _1973.y));
                }
                else
                {
                    float4 _3643;
                    if (_893 == 3)
                    {
                        float2 _3618;
                        if (_89.gConv.x > 0.5)
                        {
                            float2 _3439 = _1985;
                            _3439.y = 1.0 - _1985.y;
                            _3618 = _3439;
                        }
                        else
                        {
                            _3618 = _1985;
                        }
                        float _2302 = _1985.y;
                        float2 _2303 = float2(_1997.x, _2302);
                        float2 _3619;
                        if (_89.gConv.x > 0.5)
                        {
                            float2 _3446 = _2303;
                            _3446.y = 1.0 - _2302;
                            _3619 = _3446;
                        }
                        else
                        {
                            _3619 = _2303;
                        }
                        float _2320 = _1997.y;
                        float2 _2321 = float2(_1985.x, _2320);
                        float2 _3621;
                        if (_89.gConv.x > 0.5)
                        {
                            float2 _3453 = _2321;
                            _3453.y = 1.0 - _2320;
                            _3621 = _3453;
                        }
                        else
                        {
                            _3621 = _2321;
                        }
                        float2 _3623;
                        if (_89.gConv.x > 0.5)
                        {
                            float2 _3460 = _1997;
                            _3460.y = 1.0 - _1997.y;
                            _3623 = _3460;
                        }
                        else
                        {
                            _3623 = _1997;
                        }
                        _3643 = (((gBackdrop3.sample(gLinear, _3618, level(0.0)) * (_1969.x * _1969.y)) + (gBackdrop3.sample(gLinear, _3619, level(0.0)) * (_1973.x * _1969.y))) + (gBackdrop3.sample(gLinear, _3621, level(0.0)) * (_1969.x * _1973.y))) + (gBackdrop3.sample(gLinear, _3623, level(0.0)) * (_1973.x * _1973.y));
                    }
                    else
                    {
                        float4 _3644;
                        if (_893 == 4)
                        {
                            float2 _3611;
                            if (_89.gConv.x > 0.5)
                            {
                                float2 _3467 = _1985;
                                _3467.y = 1.0 - _1985.y;
                                _3611 = _3467;
                            }
                            else
                            {
                                _3611 = _1985;
                            }
                            float _2432 = _1985.y;
                            float2 _2433 = float2(_1997.x, _2432);
                            float2 _3612;
                            if (_89.gConv.x > 0.5)
                            {
                                float2 _3474 = _2433;
                                _3474.y = 1.0 - _2432;
                                _3612 = _3474;
                            }
                            else
                            {
                                _3612 = _2433;
                            }
                            float _2450 = _1997.y;
                            float2 _2451 = float2(_1985.x, _2450);
                            float2 _3614;
                            if (_89.gConv.x > 0.5)
                            {
                                float2 _3481 = _2451;
                                _3481.y = 1.0 - _2450;
                                _3614 = _3481;
                            }
                            else
                            {
                                _3614 = _2451;
                            }
                            float2 _3616;
                            if (_89.gConv.x > 0.5)
                            {
                                float2 _3488 = _1997;
                                _3488.y = 1.0 - _1997.y;
                                _3616 = _3488;
                            }
                            else
                            {
                                _3616 = _1997;
                            }
                            _3644 = (((gBackdrop4.sample(gLinear, _3611, level(0.0)) * (_1969.x * _1969.y)) + (gBackdrop4.sample(gLinear, _3612, level(0.0)) * (_1973.x * _1969.y))) + (gBackdrop4.sample(gLinear, _3614, level(0.0)) * (_1969.x * _1973.y))) + (gBackdrop4.sample(gLinear, _3616, level(0.0)) * (_1973.x * _1973.y));
                        }
                        else
                        {
                            float2 _3604;
                            if (_89.gConv.x > 0.5)
                            {
                                float2 _3495 = _1985;
                                _3495.y = 1.0 - _1985.y;
                                _3604 = _3495;
                            }
                            else
                            {
                                _3604 = _1985;
                            }
                            float _2562 = _1985.y;
                            float2 _2563 = float2(_1997.x, _2562);
                            float2 _3605;
                            if (_89.gConv.x > 0.5)
                            {
                                float2 _3502 = _2563;
                                _3502.y = 1.0 - _2562;
                                _3605 = _3502;
                            }
                            else
                            {
                                _3605 = _2563;
                            }
                            float _2580 = _1997.y;
                            float2 _2581 = float2(_1985.x, _2580);
                            float2 _3607;
                            if (_89.gConv.x > 0.5)
                            {
                                float2 _3509 = _2581;
                                _3509.y = 1.0 - _2580;
                                _3607 = _3509;
                            }
                            else
                            {
                                _3607 = _2581;
                            }
                            float2 _3609;
                            if (_89.gConv.x > 0.5)
                            {
                                float2 _3516 = _1997;
                                _3516.y = 1.0 - _1997.y;
                                _3609 = _3516;
                            }
                            else
                            {
                                _3609 = _1997;
                            }
                            _3644 = (((gBackdrop5.sample(gLinear, _3604, level(0.0)) * (_1969.x * _1969.y)) + (gBackdrop5.sample(gLinear, _3605, level(0.0)) * (_1973.x * _1969.y))) + (gBackdrop5.sample(gLinear, _3607, level(0.0)) * (_1969.x * _1973.y))) + (gBackdrop5.sample(gLinear, _3609, level(0.0)) * (_1973.x * _1973.y));
                        }
                        _3643 = _3644;
                    }
                    _3642 = _3643;
                }
                _3641 = _3642;
            }
            _3640 = _3641;
        }
        _3652 = mix(_3603, _3640, float4(fast::clamp(((_662.gPass1.y * _662.gPass1.y) - (_991 * _991)) / ((_1008 * _1008) - (_991 * _991)), 0.0, 1.0)));
    }
    else
    {
        _3652 = _841;
    }
    float4 _3653;
    if (_662.gPass1.x >= 0.0)
    {
        float2 _920 = fast::max((_662.gPass0.zw - _662.gPass0.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float2 _923 = _3560 - ((_662.gPass0.xy + _662.gPass0.zw) * 0.5);
        float _2710 = fast::min(_920.x, _920.y);
        float _2713 = fast::min((_923.x > 0.0) ? ((_923.y > 0.0) ? _662.gPass1.x : _662.gPass1.x) : ((_923.y > 0.0) ? _662.gPass1.x : _662.gPass1.x), _2710);
        float _2719 = _2713 * (1.0 + (0.60000002384185791015625 * _662.gPass1.w));
        float _3645;
        float _3646;
        if (_2719 > _2710)
        {
            float _2733 = _662.gPass1.w * fast::clamp((_2710 - _2713) / fast::max(0.60000002384185791015625 * _2713, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _3646 = _2733;
            _3645 = _2713 * (1.0 + (0.60000002384185791015625 * _2733));
        }
        else
        {
            _3646 = _662.gPass1.w;
            _3645 = _2719;
        }
        float2 _2762 = (abs(_923) - _920) + float2(_3645);
        float2 _2764 = fast::max(_2762, float2(0.0));
        float _3647;
        if ((_2764.x > 0.0) && (_2764.y > 0.0))
        {
            float _3648;
            if ((_3646 > 0.001000000047497451305389404296875) && (_3645 > 9.9999997473787516355514526367188e-05))
            {
                float _2781 = 2.0 + (2.0 * _3646);
                float2 _2786 = _2764 / float2(fast::max(_3645, 9.9999997473787516355514526367188e-05));
                _3648 = pow(pow(_2786.x, _2781) + pow(_2786.y, _2781), 1.0 / _2781) * _3645;
            }
            else
            {
                _3648 = length(_2764);
            }
            _3647 = _3648;
        }
        else
        {
            _3647 = fast::max(_2764.x, _2764.y);
        }
        _3653 = _3652 * fast::clamp(0.5 - ((fast::min(fast::max(_2762.x, _2762.y), 0.0) + _3647) - _3645), 0.0, 1.0);
    }
    else
    {
        _3653 = _3652;
    }
    float4 _939 = _3653 * _662.gPass1.z;
    float4 _3654;
    if ((_89.gTime.w > 0.5) && (_939.w > 9.9999997473787516355514526367188e-06))
    {
        float3 _2856 = fast::clamp(_939.xyz / float3(_939.w), float3(0.0), float3(1.0));
        float3 _2842 = select(pow((_2856 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _2856 * float3(0.077399380505084991455078125), _2856 <= float3(0.040449999272823333740234375)) * _939.w;
        float4 _3542 = _939;
        _3542.x = _2842.x;
        _3542.y = _2842.y;
        _3542.z = _2842.z;
        _3654 = _3542;
    }
    else
    {
        _3654 = _939;
    }
    out._entryPointOutput = _3654;
    return out;
}

