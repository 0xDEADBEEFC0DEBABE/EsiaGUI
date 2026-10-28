#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T, size_t Num>
struct spvUnsafeArray
{
    T elements[Num ? Num : 1];
    
    thread T& operator [] (size_t pos) thread
    {
        return elements[pos];
    }
    constexpr const thread T& operator [] (size_t pos) const thread
    {
        return elements[pos];
    }
    
    device T& operator [] (size_t pos) device
    {
        return elements[pos];
    }
    constexpr const device T& operator [] (size_t pos) const device
    {
        return elements[pos];
    }
    
    constexpr const constant T& operator [] (size_t pos) const constant
    {
        return elements[pos];
    }
    
    threadgroup T& operator [] (size_t pos) threadgroup
    {
        return elements[pos];
    }
    constexpr const threadgroup T& operator [] (size_t pos) const threadgroup
    {
        return elements[pos];
    }
};

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

struct WgtDraw
{
    float4 gFade;
    float4 gDrawInfo;
};

struct gFxData
{
    float4 _data[1];
};

constant uint kEsiaFxFeatures_tmp [[function_constant(0)]];
constant uint kEsiaFxFeatures = is_function_constant_defined(kEsiaFxFeatures_tmp) ? kEsiaFxFeatures_tmp : 4294967295u;
constant uint _1710 = (32768u & kEsiaFxFeatures);
constant uint _1725 = (262144u & kEsiaFxFeatures);
constant uint _1796 = (512u & kEsiaFxFeatures);
constant uint _1848 = (262144u & kEsiaFxFeatures);
constant uint _2953 = (512u & kEsiaFxFeatures);
constant uint _2974 = (32768u & kEsiaFxFeatures);
constant uint _3007 = (131072u & kEsiaFxFeatures);
constant uint _3143 = (2u & kEsiaFxFeatures);
constant uint _3153 = (1024u & kEsiaFxFeatures);
constant uint _3220 = (32u & kEsiaFxFeatures);
constant uint _3228 = (4u & kEsiaFxFeatures);
constant uint _3233 = (256u & kEsiaFxFeatures);
constant uint _3298 = (8u & kEsiaFxFeatures);
constant uint _3327 = (8192u & kEsiaFxFeatures);
constant uint _3544 = (65536u & kEsiaFxFeatures);
constant uint _3620 = (32u & kEsiaFxFeatures);
constant uint _3684 = (1u & kEsiaFxFeatures);
constant uint _3713 = (64u & kEsiaFxFeatures);
constant uint _3766 = (16384u & kEsiaFxFeatures);
constant uint _4197 = (4u & kEsiaFxFeatures);
constant uint _4201 = (256u & kEsiaFxFeatures);
constant uint _4261 = (16u & kEsiaFxFeatures);
constant uint _4330 = (2u & kEsiaFxFeatures);
constant uint _4464 = (128u & kEsiaFxFeatures);
constant uint _4546 = (2048u & kEsiaFxFeatures);
constant uint _4586 = (8u & kEsiaFxFeatures);
constant uint _4590 = (4u & kEsiaFxFeatures);

constant spvUnsafeArray<float3, 6> _2312 = spvUnsafeArray<float3, 6>({ float3(1.0, 0.4199999868869781494140625, 0.2199999988079071044921875), float3(1.0, 0.699999988079071044921875, 0.300000011920928955078125), float3(0.800000011920928955078125, 0.920000016689300537109375, 0.4000000059604644775390625), float3(0.3499999940395355224609375, 0.89999997615814208984375, 0.699999988079071044921875), float3(0.4000000059604644775390625, 0.62000000476837158203125, 1.0), float3(0.660000026226043701171875, 0.5, 1.0) });
constant spvUnsafeArray<float4, 4> _3928 = spvUnsafeArray<float4, 4>({ float4(-1.0, 1.0, 3.400000095367431640625, 2.599999904632568359375), float4(-0.550000011920928955078125, 0.800000011920928955078125, 2.0, 0.800000011920928955078125), float4(0.300000011920928955078125, 1.0, 1.2000000476837158203125, 1.2999999523162841796875), float4(0.62000000476837158203125, 0.800000011920928955078125, 1.60000002384185791015625, 0.449999988079071044921875) });
constant spvUnsafeArray<float2, 4> _3945 = spvUnsafeArray<float2, 4>({ float2(0.0), float2(0.100000001490116119384765625, 1.2999999523162841796875), float2(0.550000011920928955078125, 3.900000095367431640625), float2(0.20000000298023223876953125, 5.19999980926513671875) });

struct esia_main_out
{
    float4 _entryPointOutput [[color(0)]];
};

struct esia_main_in
{
    float2 i_local [[user(locn0)]];
    uint i_instance [[user(locn1)]];
    float4 i_rect [[user(locn2), flat]];
    float4 i_radii [[user(locn3), flat]];
    float4 i_fill0 [[user(locn4), flat]];
    float4 i_shape [[user(locn5), flat]];
    float4 i_misc [[user(locn6), flat]];
    uint4 i_flags [[user(locn7)]];
};

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _226 [[buffer(0)]], constant WgtDraw& _268 [[buffer(2)]], const device gFxData& gFxData_1 [[buffer(10)]], texture2d<float> gTex [[texture(3)]], texture2d<float> gBackdrop0 [[texture(4)]], texture2d<float> gBackdrop1 [[texture(5)]], texture2d<float> gBackdrop2 [[texture(6)]], texture2d<float> gBackdrop3 [[texture(7)]], texture2d<float> gBackdrop4 [[texture(8)]], texture2d<float> gBackdrop5 [[texture(9)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float2 _6355 = gl_FragCoord.xy + _226.gConv.yy;
    float2 _26218;
    if (_226.gConv.x > 0.5)
    {
        float2 _24560 = _6355;
        _24560.y = _226.gTarget.y - _6355.y;
        _26218 = _24560;
    }
    else
    {
        _26218 = _6355;
    }
    float _4956 = dfdx(in.i_local.x);
    float _4960 = dfdy(in.i_local.x);
    float _4963 = fast::max(abs(_4956) + abs(_4960), 9.9999997473787516355514526367188e-05);
    float4 _26223;
    float4 _26226;
    if ((in.i_flags.z == 2u) || ((in.i_flags.x & _2953) != 0u))
    {
        _26226 = gFxData_1._data[(in.i_instance * 24u) + 15u];
        _26223 = gFxData_1._data[(in.i_instance * 24u) + 16u];
    }
    else
    {
        _26226 = float4(0.0);
        _26223 = float4(0.0);
    }
    float4 _26219;
    float4 _26225;
    if ((in.i_flags.x & _2974) != 0u)
    {
        _26225 = gFxData_1._data[(in.i_instance * 24u) + 15u];
        _26219 = gFxData_1._data[(in.i_instance * 24u) + 18u];
    }
    else
    {
        _26225 = _26226;
        _26219 = float4(0.0);
    }
    float2 _6444 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
    float2 _6453 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
    float _26273;
    if ((in.i_flags.x & _1710) != 0u)
    {
        float _6570 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
        float4 _6604 = gTex.sample(gLinear, mix(_26219.xy, _26219.zw, (fast::clamp(in.i_local, _26225.xy, _26225.zw) - _26225.xy) / fast::max(_26225.zw - _26225.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
        float _6628 = (_6604.w - 0.5) * in.i_radii.y;
        float2 _6669 = abs(in.i_local - ((_26225.xy + _26225.zw) * 0.5)) - fast::max(((_26225.zw - _26225.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _6570), float2(0.0));
        _26273 = fast::max((-mix((fast::max(fast::min(_6604.x, _6604.y), fast::min(fast::max(_6604.x, _6604.y), _6604.z)) - 0.5) * in.i_radii.x, _6628, fast::clamp(abs(_6628) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _6570, length(fast::max(_6669, float2(0.0))) + fast::min(fast::max(_6669.x, _6669.y), 0.0)) - in.i_shape.w;
    }
    else
    {
        float _26274;
        if ((in.i_flags.x & _1725) != 0u)
        {
            float _26272;
            do
            {
                bool _6731 = in.i_radii.w > 0.5;
                float _6735 = floor((in.i_radii.z + 1.0) * 0.5);
                float _6739 = floor((_6735 + 127.0) * 0.0078125);
                float _6743 = floor((in.i_radii.z - 1.0) * 0.5);
                float _6871 = floor(_6743 * 0.0625);
                uint _6888 = (((in.i_instance + 1u) + uint(_6871)) * 24u) + uint(_6743 - (_6871 * 16.0));
                float2 _26249;
                if (_6731)
                {
                    float _6898 = floor(0.0);
                    _26249 = gFxData_1._data[(((in.i_instance + 1u) + uint(_6898)) * 24u) + uint(_6898 * (-16.0))].xy;
                }
                else
                {
                    _26249 = select(gFxData_1._data[_6888].zw, gFxData_1._data[_6888].xy, bool2(((in.i_radii.z - 1.0) - (_6743 * 2.0)) < 0.5));
                }
                float2 _26248;
                float _26251;
                float _26254;
                _26254 = 1.0;
                _26251 = 1000000015047466219876688855040.0;
                _26248 = _26249;
                float2 _26247;
                float _26250;
                float _26253;
                for (float _26244 = 0.0; _26244 < 16.0; _26254 = _26253, _26251 = _26250, _26248 = _26247, _26244 += 1.0)
                {
                    if (_26244 >= _6739)
                    {
                        break;
                    }
                    _26253 = _26254;
                    _26250 = _26251;
                    _26247 = _26248;
                    float2 _30794;
                    float _30796;
                    float _30798;
                    for (float _26245 = 0.0; _26245 < 128.0; _26253 = _30798, _26250 = _30796, _26247 = _30794, _26245 += 1.0)
                    {
                        float _6785 = (_26244 * 128.0) + _26245;
                        if (_6785 >= _6735)
                        {
                            break;
                        }
                        float _6925 = floor(_6785 * 0.0625);
                        uint _6942 = (((in.i_instance + 1u) + uint(_6925)) * 24u) + uint(_6785 - (_6925 * 16.0));
                        float2 _6955 = _26247 - gFxData_1._data[_6942].xy;
                        float2 _6958 = in.i_local - gFxData_1._data[_6942].xy;
                        float2 _6971 = _6958 - (_6955 * fast::clamp(dot(_6958, _6955) / fast::max(dot(_6955, _6955), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                        float _6974 = dot(_6971, _6971);
                        float _30999 = (_6974 < _26250) ? _6974 : _26250;
                        bool _6986 = in.i_local.y >= gFxData_1._data[_6942].y;
                        bool _6991 = in.i_local.y < _26247.y;
                        bool _7002 = (_6955.x * _6958.y) > (_6955.y * _6958.x);
                        float _26259;
                        if (((_6986 && _6991) && _7002) || (((!_6986) && (!_6991)) && (!_7002)))
                        {
                            _26259 = -_26253;
                        }
                        else
                        {
                            _26259 = _26253;
                        }
                        if (((2.0 * _6785) + 1.0) < in.i_radii.z)
                        {
                            float2 _7031 = gFxData_1._data[_6942].xy - gFxData_1._data[_6942].zw;
                            float2 _7034 = in.i_local - gFxData_1._data[_6942].zw;
                            float2 _7047 = _7034 - (_7031 * fast::clamp(dot(_7034, _7031) / fast::max(dot(_7031, _7031), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                            float _7050 = dot(_7047, _7047);
                            bool _7062 = in.i_local.y >= gFxData_1._data[_6942].w;
                            bool _7067 = in.i_local.y < gFxData_1._data[_6942].y;
                            bool _7078 = (_7031.x * _7034.y) > (_7031.y * _7034.x);
                            float _26263;
                            if (((_7062 && _7067) && _7078) || (((!_7062) && (!_7067)) && (!_7078)))
                            {
                                _26263 = -_26259;
                            }
                            else
                            {
                                _26263 = _26259;
                            }
                            _30798 = _26263;
                            _30796 = (_7050 < _30999) ? _7050 : _30999;
                            _30794 = gFxData_1._data[_6942].zw;
                        }
                        else
                        {
                            _30798 = _26259;
                            _30796 = _30999;
                            _30794 = gFxData_1._data[_6942].xy;
                        }
                    }
                }
                float _6837 = sqrt(_26251);
                if (_6731)
                {
                    _26272 = _6837 - in.i_radii.x;
                    break;
                }
                _26272 = (_26254 * _6837) - in.i_radii.x;
                break;
            } while(false);
            _26274 = _26272;
        }
        else
        {
            float _26275;
            if (in.i_flags.z == 1u)
            {
                float2 _6478 = in.i_local - _6444;
                float _26243;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _26243 = abs(length(_6478) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _7121 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _7123 = cos(_7121);
                    float _7125 = sin(_7121);
                    float _7134 = dot(_6478, float2(-_7125, _7123));
                    float _7137 = dot(_6478, float2(_7123, _7125));
                    float2 _7138 = float2(_7134, _7137);
                    float _7141 = abs(_7134);
                    _7138.x = _7141;
                    float _7144 = in.i_shape.w * 0.5;
                    float _7146 = sin(_7144);
                    float _7148 = cos(_7144);
                    _26243 = (((_7148 * _7141) > (_7146 * _7137)) ? length(_7138 - (float2(_7146, _7148) * in.i_radii.x)) : abs(length(_7138) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _26275 = _26243;
            }
            else
            {
                float _26276;
                if (in.i_flags.z == 2u)
                {
                    float2 _7184 = in.i_local - _26225.xy;
                    float2 _7187 = _26225.zw - _26225.xy;
                    _26276 = length(_7184 - (_7187 * fast::clamp(dot(_7184, _7187) / fast::max(dot(_7187, _7187), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _6505 = in.i_local - _6444;
                    float _7240 = fast::min(_6453.x, _6453.y);
                    float _7243 = fast::min((_6505.x > 0.0) ? ((_6505.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_6505.y > 0.0) ? in.i_radii.w : in.i_radii.x), _7240);
                    float _7249 = _7243 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _26228;
                    float _26229;
                    if (_7249 > _7240)
                    {
                        float _7263 = in.i_shape.y * fast::clamp((_7240 - _7243) / fast::max(0.60000002384185791015625 * _7243, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _26229 = _7263;
                        _26228 = _7243 * (1.0 + (0.60000002384185791015625 * _7263));
                    }
                    else
                    {
                        _26229 = in.i_shape.y;
                        _26228 = _7249;
                    }
                    float2 _7276 = (abs(_6505) - _6453) + float2(_26228);
                    float2 _7278 = fast::max(_7276, float2(0.0));
                    float _26230;
                    if ((_7278.x > 0.0) && (_7278.y > 0.0))
                    {
                        float _26231;
                        if ((_26229 > 0.001000000047497451305389404296875) && (_26228 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7295 = 2.0 + (2.0 * _26229);
                            float2 _7300 = _7278 / float2(fast::max(_26228, 9.9999997473787516355514526367188e-05));
                            _26231 = pow(pow(_7300.x, _7295) + pow(_7300.y, _7295), 1.0 / _7295) * _26228;
                        }
                        else
                        {
                            _26231 = length(_7278);
                        }
                        _26230 = _26231;
                    }
                    else
                    {
                        _26230 = fast::max(_7278.x, _7278.y);
                    }
                    float _7335 = (fast::min(fast::max(_7276.x, _7276.y), 0.0) + _26230) - _26228;
                    float _26277;
                    if ((in.i_flags.x & _1796) != 0u)
                    {
                        float2 _6533 = fast::max((_26225.zw - _26225.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _6536 = in.i_local - ((_26225.xy + _26225.zw) * 0.5);
                        float _7371 = fast::min(_6533.x, _6533.y);
                        float _7374 = fast::min((_6536.x > 0.0) ? ((_6536.y > 0.0) ? _26223.x : _26223.x) : ((_6536.y > 0.0) ? _26223.x : _26223.x), _7371);
                        float _7380 = _7374 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _26234;
                        float _26235;
                        if (_7380 > _7371)
                        {
                            float _7394 = in.i_shape.y * fast::clamp((_7371 - _7374) / fast::max(0.60000002384185791015625 * _7374, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _26235 = _7394;
                            _26234 = _7374 * (1.0 + (0.60000002384185791015625 * _7394));
                        }
                        else
                        {
                            _26235 = in.i_shape.y;
                            _26234 = _7380;
                        }
                        float2 _7407 = (abs(_6536) - _6533) + float2(_26234);
                        float2 _7409 = fast::max(_7407, float2(0.0));
                        float _26236;
                        if ((_7409.x > 0.0) && (_7409.y > 0.0))
                        {
                            float _26237;
                            if ((_26235 > 0.001000000047497451305389404296875) && (_26234 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7426 = 2.0 + (2.0 * _26235);
                                float2 _7431 = _7409 / float2(fast::max(_26234, 9.9999997473787516355514526367188e-05));
                                _26237 = pow(pow(_7431.x, _7426) + pow(_7431.y, _7426), 1.0 / _7426) * _26234;
                            }
                            else
                            {
                                _26237 = length(_7409);
                            }
                            _26236 = _26237;
                        }
                        else
                        {
                            _26236 = fast::max(_7409.x, _7409.y);
                        }
                        float _7466 = (fast::min(fast::max(_7407.x, _7407.y), 0.0) + _26236) - _26234;
                        float _7471 = fast::max(_26223.y, 9.9999997473787516355514526367188e-05);
                        float _7480 = fast::max(_7471 - abs(_7335 - _7466), 0.0) / _7471;
                        _26277 = fast::min(_7335, _7466) - (((_7480 * _7480) * _7471) * 0.25);
                    }
                    else
                    {
                        _26277 = _7335;
                    }
                    _26276 = _26277;
                }
                _26275 = _26276;
            }
            _26274 = _26275;
        }
        _26273 = _26274;
    }
    float _26624;
    float _30162;
    if ((in.i_flags.x & _3007) != 0u)
    {
        uint _7504 = (in.i_instance * 24u) + 6u;
        float _5023 = fast::clamp(ceil(length(gFxData_1._data[(in.i_instance * 24u) + 21u].xy) / (4.0 * _4963)), 1.0, 8.0);
        float _5028 = gFxData_1._data[_7504].x * gFxData_1._data[_7504].y;
        float _5034 = gFxData_1._data[_7504].x * (1.0 - gFxData_1._data[_7504].y);
        float2 _5038 = in.i_local + (gFxData_1._data[(in.i_instance * 24u) + 21u].xy * 0.5);
        float2 _7551 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _7560 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _26392;
        if ((in.i_flags.x & _1710) != 0u)
        {
            float _7677 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
            float4 _7711 = gTex.sample(gLinear, mix(_26219.xy, _26219.zw, (fast::clamp(_5038, _26225.xy, _26225.zw) - _26225.xy) / fast::max(_26225.zw - _26225.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
            float _7735 = (_7711.w - 0.5) * in.i_radii.y;
            float2 _7776 = abs(_5038 - ((_26225.xy + _26225.zw) * 0.5)) - fast::max(((_26225.zw - _26225.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _7677), float2(0.0));
            _26392 = fast::max((-mix((fast::max(fast::min(_7711.x, _7711.y), fast::min(fast::max(_7711.x, _7711.y), _7711.z)) - 0.5) * in.i_radii.x, _7735, fast::clamp(abs(_7735) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _7677, length(fast::max(_7776, float2(0.0))) + fast::min(fast::max(_7776.x, _7776.y), 0.0)) - in.i_shape.w;
        }
        else
        {
            float _26393;
            if ((in.i_flags.x & _1725) != 0u)
            {
                float _26391;
                do
                {
                    bool _7838 = in.i_radii.w > 0.5;
                    float _7842 = floor((in.i_radii.z + 1.0) * 0.5);
                    float _7846 = floor((_7842 + 127.0) * 0.0078125);
                    float _7850 = floor((in.i_radii.z - 1.0) * 0.5);
                    float _7978 = floor(_7850 * 0.0625);
                    uint _7995 = (((in.i_instance + 1u) + uint(_7978)) * 24u) + uint(_7850 - (_7978 * 16.0));
                    float2 _26368;
                    if (_7838)
                    {
                        float _8005 = floor(0.0);
                        _26368 = gFxData_1._data[(((in.i_instance + 1u) + uint(_8005)) * 24u) + uint(_8005 * (-16.0))].xy;
                    }
                    else
                    {
                        _26368 = select(gFxData_1._data[_7995].zw, gFxData_1._data[_7995].xy, bool2(((in.i_radii.z - 1.0) - (_7850 * 2.0)) < 0.5));
                    }
                    float2 _26367;
                    float _26370;
                    float _26373;
                    _26373 = 1.0;
                    _26370 = 1000000015047466219876688855040.0;
                    _26367 = _26368;
                    float2 _26366;
                    float _26369;
                    float _26372;
                    for (float _26363 = 0.0; _26363 < 16.0; _26373 = _26372, _26370 = _26369, _26367 = _26366, _26363 += 1.0)
                    {
                        if (_26363 >= _7846)
                        {
                            break;
                        }
                        _26372 = _26373;
                        _26369 = _26370;
                        _26366 = _26367;
                        float2 _30805;
                        float _30807;
                        float _30809;
                        for (float _26364 = 0.0; _26364 < 128.0; _26372 = _30809, _26369 = _30807, _26366 = _30805, _26364 += 1.0)
                        {
                            float _7892 = (_26363 * 128.0) + _26364;
                            if (_7892 >= _7842)
                            {
                                break;
                            }
                            float _8032 = floor(_7892 * 0.0625);
                            uint _8049 = (((in.i_instance + 1u) + uint(_8032)) * 24u) + uint(_7892 - (_8032 * 16.0));
                            float2 _8062 = _26366 - gFxData_1._data[_8049].xy;
                            float2 _8065 = _5038 - gFxData_1._data[_8049].xy;
                            float2 _8078 = _8065 - (_8062 * fast::clamp(dot(_8065, _8062) / fast::max(dot(_8062, _8062), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                            float _8081 = dot(_8078, _8078);
                            float _31001 = (_8081 < _26369) ? _8081 : _26369;
                            bool _8093 = _5038.y >= gFxData_1._data[_8049].y;
                            bool _8098 = _5038.y < _26366.y;
                            bool _8109 = (_8062.x * _8065.y) > (_8062.y * _8065.x);
                            float _26378;
                            if (((_8093 && _8098) && _8109) || (((!_8093) && (!_8098)) && (!_8109)))
                            {
                                _26378 = -_26372;
                            }
                            else
                            {
                                _26378 = _26372;
                            }
                            if (((2.0 * _7892) + 1.0) < in.i_radii.z)
                            {
                                float2 _8138 = gFxData_1._data[_8049].xy - gFxData_1._data[_8049].zw;
                                float2 _8141 = _5038 - gFxData_1._data[_8049].zw;
                                float2 _8154 = _8141 - (_8138 * fast::clamp(dot(_8141, _8138) / fast::max(dot(_8138, _8138), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _8157 = dot(_8154, _8154);
                                bool _8169 = _5038.y >= gFxData_1._data[_8049].w;
                                bool _8174 = _5038.y < gFxData_1._data[_8049].y;
                                bool _8185 = (_8138.x * _8141.y) > (_8138.y * _8141.x);
                                float _26382;
                                if (((_8169 && _8174) && _8185) || (((!_8169) && (!_8174)) && (!_8185)))
                                {
                                    _26382 = -_26378;
                                }
                                else
                                {
                                    _26382 = _26378;
                                }
                                _30809 = _26382;
                                _30807 = (_8157 < _31001) ? _8157 : _31001;
                                _30805 = gFxData_1._data[_8049].zw;
                            }
                            else
                            {
                                _30809 = _26378;
                                _30807 = _31001;
                                _30805 = gFxData_1._data[_8049].xy;
                            }
                        }
                    }
                    float _7944 = sqrt(_26370);
                    if (_7838)
                    {
                        _26391 = _7944 - in.i_radii.x;
                        break;
                    }
                    _26391 = (_26373 * _7944) - in.i_radii.x;
                    break;
                } while(false);
                _26393 = _26391;
            }
            else
            {
                float _26394;
                if (in.i_flags.z == 1u)
                {
                    float2 _7585 = _5038 - _7551;
                    float _26362;
                    do
                    {
                        if (in.i_shape.w >= 6.282185077667236328125)
                        {
                            _26362 = abs(length(_7585) - in.i_radii.x) - in.i_radii.y;
                            break;
                        }
                        float _8228 = in.i_shape.z + (in.i_shape.w * 0.5);
                        float _8230 = cos(_8228);
                        float _8232 = sin(_8228);
                        float _8241 = dot(_7585, float2(-_8232, _8230));
                        float _8244 = dot(_7585, float2(_8230, _8232));
                        float2 _8245 = float2(_8241, _8244);
                        float _8248 = abs(_8241);
                        _8245.x = _8248;
                        float _8251 = in.i_shape.w * 0.5;
                        float _8253 = sin(_8251);
                        float _8255 = cos(_8251);
                        _26362 = (((_8255 * _8248) > (_8253 * _8244)) ? length(_8245 - (float2(_8253, _8255) * in.i_radii.x)) : abs(length(_8245) - in.i_radii.x)) - in.i_radii.y;
                        break;
                    } while(false);
                    _26394 = _26362;
                }
                else
                {
                    float _26395;
                    if (in.i_flags.z == 2u)
                    {
                        float2 _8291 = _5038 - _26225.xy;
                        float2 _8294 = _26225.zw - _26225.xy;
                        _26395 = length(_8291 - (_8294 * fast::clamp(dot(_8291, _8294) / fast::max(dot(_8294, _8294), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                    }
                    else
                    {
                        float2 _7612 = _5038 - _7551;
                        float _8347 = fast::min(_7560.x, _7560.y);
                        float _8350 = fast::min((_7612.x > 0.0) ? ((_7612.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_7612.y > 0.0) ? in.i_radii.w : in.i_radii.x), _8347);
                        float _8356 = _8350 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _26347;
                        float _26348;
                        if (_8356 > _8347)
                        {
                            float _8370 = in.i_shape.y * fast::clamp((_8347 - _8350) / fast::max(0.60000002384185791015625 * _8350, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _26348 = _8370;
                            _26347 = _8350 * (1.0 + (0.60000002384185791015625 * _8370));
                        }
                        else
                        {
                            _26348 = in.i_shape.y;
                            _26347 = _8356;
                        }
                        float2 _8383 = (abs(_7612) - _7560) + float2(_26347);
                        float2 _8385 = fast::max(_8383, float2(0.0));
                        float _26349;
                        if ((_8385.x > 0.0) && (_8385.y > 0.0))
                        {
                            float _26350;
                            if ((_26348 > 0.001000000047497451305389404296875) && (_26347 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8402 = 2.0 + (2.0 * _26348);
                                float2 _8407 = _8385 / float2(fast::max(_26347, 9.9999997473787516355514526367188e-05));
                                _26350 = pow(pow(_8407.x, _8402) + pow(_8407.y, _8402), 1.0 / _8402) * _26347;
                            }
                            else
                            {
                                _26350 = length(_8385);
                            }
                            _26349 = _26350;
                        }
                        else
                        {
                            _26349 = fast::max(_8385.x, _8385.y);
                        }
                        float _8442 = (fast::min(fast::max(_8383.x, _8383.y), 0.0) + _26349) - _26347;
                        float _26396;
                        if ((in.i_flags.x & _1796) != 0u)
                        {
                            float2 _7640 = fast::max((_26225.zw - _26225.xy) * 0.5, float2(0.001000000047497451305389404296875));
                            float2 _7643 = _5038 - ((_26225.xy + _26225.zw) * 0.5);
                            float _8478 = fast::min(_7640.x, _7640.y);
                            float _8481 = fast::min((_7643.x > 0.0) ? ((_7643.y > 0.0) ? _26223.x : _26223.x) : ((_7643.y > 0.0) ? _26223.x : _26223.x), _8478);
                            float _8487 = _8481 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _26353;
                            float _26354;
                            if (_8487 > _8478)
                            {
                                float _8501 = in.i_shape.y * fast::clamp((_8478 - _8481) / fast::max(0.60000002384185791015625 * _8481, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _26354 = _8501;
                                _26353 = _8481 * (1.0 + (0.60000002384185791015625 * _8501));
                            }
                            else
                            {
                                _26354 = in.i_shape.y;
                                _26353 = _8487;
                            }
                            float2 _8514 = (abs(_7643) - _7640) + float2(_26353);
                            float2 _8516 = fast::max(_8514, float2(0.0));
                            float _26355;
                            if ((_8516.x > 0.0) && (_8516.y > 0.0))
                            {
                                float _26356;
                                if ((_26354 > 0.001000000047497451305389404296875) && (_26353 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _8533 = 2.0 + (2.0 * _26354);
                                    float2 _8538 = _8516 / float2(fast::max(_26353, 9.9999997473787516355514526367188e-05));
                                    _26356 = pow(pow(_8538.x, _8533) + pow(_8538.y, _8533), 1.0 / _8533) * _26353;
                                }
                                else
                                {
                                    _26356 = length(_8516);
                                }
                                _26355 = _26356;
                            }
                            else
                            {
                                _26355 = fast::max(_8516.x, _8516.y);
                            }
                            float _8573 = (fast::min(fast::max(_8514.x, _8514.y), 0.0) + _26355) - _26353;
                            float _8578 = fast::max(_26223.y, 9.9999997473787516355514526367188e-05);
                            float _8587 = fast::max(_8578 - abs(_8442 - _8573), 0.0) / _8578;
                            _26396 = fast::min(_8442, _8573) - (((_8587 * _8587) * _8578) * 0.25);
                        }
                        else
                        {
                            _26396 = _8442;
                        }
                        _26395 = _26396;
                    }
                    _26394 = _26395;
                }
                _26393 = _26394;
            }
            _26392 = _26393;
        }
        float _26545;
        float _26569;
        _26569 = 0.0;
        _26545 = 0.0;
        float _5073;
        float _5102;
        float _26515;
        int _26397 = 1;
        float _26521 = _26392;
        for (; _26397 <= 8; _26569 = _5102, _26545 = _5073, _26521 = _26515, _26397++)
        {
            if (float(_26397) > _5023)
            {
                break;
            }
            float2 _5060 = in.i_local + (gFxData_1._data[(in.i_instance * 24u) + 21u].xy * (0.5 - (float(_26397) / _5023)));
            float2 _8642 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _8651 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            if ((in.i_flags.x & _1710) != 0u)
            {
                float _8768 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                float4 _8802 = gTex.sample(gLinear, mix(_26219.xy, _26219.zw, (fast::clamp(_5060, _26225.xy, _26225.zw) - _26225.xy) / fast::max(_26225.zw - _26225.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                float _8826 = (_8802.w - 0.5) * in.i_radii.y;
                float2 _8867 = abs(_5060 - ((_26225.xy + _26225.zw) * 0.5)) - fast::max(((_26225.zw - _26225.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _8768), float2(0.0));
                _26515 = fast::max((-mix((fast::max(fast::min(_8802.x, _8802.y), fast::min(fast::max(_8802.x, _8802.y), _8802.z)) - 0.5) * in.i_radii.x, _8826, fast::clamp(abs(_8826) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _8768, length(fast::max(_8867, float2(0.0))) + fast::min(fast::max(_8867.x, _8867.y), 0.0)) - in.i_shape.w;
            }
            else
            {
                float _26516;
                if ((in.i_flags.x & _1725) != 0u)
                {
                    float _26514;
                    do
                    {
                        bool _8929 = in.i_radii.w > 0.5;
                        float _8933 = floor((in.i_radii.z + 1.0) * 0.5);
                        float _8937 = floor((_8933 + 127.0) * 0.0078125);
                        float _8941 = floor((in.i_radii.z - 1.0) * 0.5);
                        float _9069 = floor(_8941 * 0.0625);
                        uint _9086 = (((in.i_instance + 1u) + uint(_9069)) * 24u) + uint(_8941 - (_9069 * 16.0));
                        float2 _26491;
                        if (_8929)
                        {
                            float _9096 = floor(0.0);
                            _26491 = gFxData_1._data[(((in.i_instance + 1u) + uint(_9096)) * 24u) + uint(_9096 * (-16.0))].xy;
                        }
                        else
                        {
                            _26491 = select(gFxData_1._data[_9086].zw, gFxData_1._data[_9086].xy, bool2(((in.i_radii.z - 1.0) - (_8941 * 2.0)) < 0.5));
                        }
                        float2 _26490;
                        float _26493;
                        float _26496;
                        _26496 = 1.0;
                        _26493 = 1000000015047466219876688855040.0;
                        _26490 = _26491;
                        float2 _26489;
                        float _26492;
                        float _26495;
                        for (float _26486 = 0.0; _26486 < 16.0; _26496 = _26495, _26493 = _26492, _26490 = _26489, _26486 += 1.0)
                        {
                            if (_26486 >= _8937)
                            {
                                break;
                            }
                            _26495 = _26496;
                            _26492 = _26493;
                            _26489 = _26490;
                            float2 _30885;
                            float _30887;
                            float _30889;
                            for (float _26487 = 0.0; _26487 < 128.0; _26495 = _30889, _26492 = _30887, _26489 = _30885, _26487 += 1.0)
                            {
                                float _8983 = (_26486 * 128.0) + _26487;
                                if (_8983 >= _8933)
                                {
                                    break;
                                }
                                float _9123 = floor(_8983 * 0.0625);
                                uint _9140 = (((in.i_instance + 1u) + uint(_9123)) * 24u) + uint(_8983 - (_9123 * 16.0));
                                float2 _9153 = _26489 - gFxData_1._data[_9140].xy;
                                float2 _9156 = _5060 - gFxData_1._data[_9140].xy;
                                float2 _9169 = _9156 - (_9153 * fast::clamp(dot(_9156, _9153) / fast::max(dot(_9153, _9153), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _9172 = dot(_9169, _9169);
                                float _31003 = (_9172 < _26492) ? _9172 : _26492;
                                bool _9184 = _5060.y >= gFxData_1._data[_9140].y;
                                bool _9189 = _5060.y < _26489.y;
                                bool _9200 = (_9153.x * _9156.y) > (_9153.y * _9156.x);
                                float _26501;
                                if (((_9184 && _9189) && _9200) || (((!_9184) && (!_9189)) && (!_9200)))
                                {
                                    _26501 = -_26495;
                                }
                                else
                                {
                                    _26501 = _26495;
                                }
                                if (((2.0 * _8983) + 1.0) < in.i_radii.z)
                                {
                                    float2 _9229 = gFxData_1._data[_9140].xy - gFxData_1._data[_9140].zw;
                                    float2 _9232 = _5060 - gFxData_1._data[_9140].zw;
                                    float2 _9245 = _9232 - (_9229 * fast::clamp(dot(_9232, _9229) / fast::max(dot(_9229, _9229), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _9248 = dot(_9245, _9245);
                                    bool _9260 = _5060.y >= gFxData_1._data[_9140].w;
                                    bool _9265 = _5060.y < gFxData_1._data[_9140].y;
                                    bool _9276 = (_9229.x * _9232.y) > (_9229.y * _9232.x);
                                    float _26505;
                                    if (((_9260 && _9265) && _9276) || (((!_9260) && (!_9265)) && (!_9276)))
                                    {
                                        _26505 = -_26501;
                                    }
                                    else
                                    {
                                        _26505 = _26501;
                                    }
                                    _30889 = _26505;
                                    _30887 = (_9248 < _31003) ? _9248 : _31003;
                                    _30885 = gFxData_1._data[_9140].zw;
                                }
                                else
                                {
                                    _30889 = _26501;
                                    _30887 = _31003;
                                    _30885 = gFxData_1._data[_9140].xy;
                                }
                            }
                        }
                        float _9035 = sqrt(_26493);
                        if (_8929)
                        {
                            _26514 = _9035 - in.i_radii.x;
                            break;
                        }
                        _26514 = (_26496 * _9035) - in.i_radii.x;
                        break;
                    } while(false);
                    _26516 = _26514;
                }
                else
                {
                    float _26517;
                    if (in.i_flags.z == 1u)
                    {
                        float2 _8676 = _5060 - _8642;
                        float _26485;
                        do
                        {
                            if (in.i_shape.w >= 6.282185077667236328125)
                            {
                                _26485 = abs(length(_8676) - in.i_radii.x) - in.i_radii.y;
                                break;
                            }
                            float _9319 = in.i_shape.z + (in.i_shape.w * 0.5);
                            float _9321 = cos(_9319);
                            float _9323 = sin(_9319);
                            float _9332 = dot(_8676, float2(-_9323, _9321));
                            float _9335 = dot(_8676, float2(_9321, _9323));
                            float2 _9336 = float2(_9332, _9335);
                            float _9339 = abs(_9332);
                            _9336.x = _9339;
                            float _9342 = in.i_shape.w * 0.5;
                            float _9344 = sin(_9342);
                            float _9346 = cos(_9342);
                            _26485 = (((_9346 * _9339) > (_9344 * _9335)) ? length(_9336 - (float2(_9344, _9346) * in.i_radii.x)) : abs(length(_9336) - in.i_radii.x)) - in.i_radii.y;
                            break;
                        } while(false);
                        _26517 = _26485;
                    }
                    else
                    {
                        float _26518;
                        if (in.i_flags.z == 2u)
                        {
                            float2 _9382 = _5060 - _26225.xy;
                            float2 _9385 = _26225.zw - _26225.xy;
                            _26518 = length(_9382 - (_9385 * fast::clamp(dot(_9382, _9385) / fast::max(dot(_9385, _9385), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                        }
                        else
                        {
                            float2 _8703 = _5060 - _8642;
                            float _9438 = fast::min(_8651.x, _8651.y);
                            float _9441 = fast::min((_8703.x > 0.0) ? ((_8703.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_8703.y > 0.0) ? in.i_radii.w : in.i_radii.x), _9438);
                            float _9447 = _9441 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _26470;
                            float _26471;
                            if (_9447 > _9438)
                            {
                                float _9461 = in.i_shape.y * fast::clamp((_9438 - _9441) / fast::max(0.60000002384185791015625 * _9441, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _26471 = _9461;
                                _26470 = _9441 * (1.0 + (0.60000002384185791015625 * _9461));
                            }
                            else
                            {
                                _26471 = in.i_shape.y;
                                _26470 = _9447;
                            }
                            float2 _9474 = (abs(_8703) - _8651) + float2(_26470);
                            float2 _9476 = fast::max(_9474, float2(0.0));
                            float _26472;
                            if ((_9476.x > 0.0) && (_9476.y > 0.0))
                            {
                                float _26473;
                                if ((_26471 > 0.001000000047497451305389404296875) && (_26470 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _9493 = 2.0 + (2.0 * _26471);
                                    float2 _9498 = _9476 / float2(fast::max(_26470, 9.9999997473787516355514526367188e-05));
                                    _26473 = pow(pow(_9498.x, _9493) + pow(_9498.y, _9493), 1.0 / _9493) * _26470;
                                }
                                else
                                {
                                    _26473 = length(_9476);
                                }
                                _26472 = _26473;
                            }
                            else
                            {
                                _26472 = fast::max(_9476.x, _9476.y);
                            }
                            float _9533 = (fast::min(fast::max(_9474.x, _9474.y), 0.0) + _26472) - _26470;
                            float _26519;
                            if ((in.i_flags.x & _1796) != 0u)
                            {
                                float2 _8731 = fast::max((_26225.zw - _26225.xy) * 0.5, float2(0.001000000047497451305389404296875));
                                float2 _8734 = _5060 - ((_26225.xy + _26225.zw) * 0.5);
                                float _9569 = fast::min(_8731.x, _8731.y);
                                float _9572 = fast::min((_8734.x > 0.0) ? ((_8734.y > 0.0) ? _26223.x : _26223.x) : ((_8734.y > 0.0) ? _26223.x : _26223.x), _9569);
                                float _9578 = _9572 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                float _26476;
                                float _26477;
                                if (_9578 > _9569)
                                {
                                    float _9592 = in.i_shape.y * fast::clamp((_9569 - _9572) / fast::max(0.60000002384185791015625 * _9572, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                    _26477 = _9592;
                                    _26476 = _9572 * (1.0 + (0.60000002384185791015625 * _9592));
                                }
                                else
                                {
                                    _26477 = in.i_shape.y;
                                    _26476 = _9578;
                                }
                                float2 _9605 = (abs(_8734) - _8731) + float2(_26476);
                                float2 _9607 = fast::max(_9605, float2(0.0));
                                float _26478;
                                if ((_9607.x > 0.0) && (_9607.y > 0.0))
                                {
                                    float _26479;
                                    if ((_26477 > 0.001000000047497451305389404296875) && (_26476 > 9.9999997473787516355514526367188e-05))
                                    {
                                        float _9624 = 2.0 + (2.0 * _26477);
                                        float2 _9629 = _9607 / float2(fast::max(_26476, 9.9999997473787516355514526367188e-05));
                                        _26479 = pow(pow(_9629.x, _9624) + pow(_9629.y, _9624), 1.0 / _9624) * _26476;
                                    }
                                    else
                                    {
                                        _26479 = length(_9607);
                                    }
                                    _26478 = _26479;
                                }
                                else
                                {
                                    _26478 = fast::max(_9607.x, _9607.y);
                                }
                                float _9664 = (fast::min(fast::max(_9605.x, _9605.y), 0.0) + _26478) - _26476;
                                float _9669 = fast::max(_26223.y, 9.9999997473787516355514526367188e-05);
                                float _9678 = fast::max(_9669 - abs(_9533 - _9664), 0.0) / _9669;
                                _26519 = fast::min(_9533, _9664) - (((_9678 * _9678) * _9669) * 0.25);
                            }
                            else
                            {
                                _26519 = _9533;
                            }
                            _26518 = _26519;
                        }
                        _26517 = _26518;
                    }
                    _26516 = _26517;
                }
                _26515 = _26516;
            }
            float _5065 = _26521 / _4963;
            float _5066 = 0.5 - _5065;
            float _5069 = _26515 / _4963;
            float _5070 = 0.5 - _5069;
            float _9696 = _5070 - _5066;
            _5073 = _26545 + ((abs(_9696) < 0.001000000047497451305389404296875) ? fast::clamp(0.5 * (_5066 + _5070), 0.0, 1.0) : ((((_5070 <= 0.0) ? 0.0 : ((_5070 >= 1.0) ? (-_5069) : ((0.5 * _5070) * _5070))) - ((_5066 <= 0.0) ? 0.0 : ((_5066 >= 1.0) ? (-_5065) : ((0.5 * _5066) * _5066)))) / _9696));
            float _5078 = (_26521 - _5028) / _4963;
            float _5079 = 0.5 - _5078;
            float _5084 = (_26515 - _5028) / _4963;
            float _5085 = 0.5 - _5084;
            float _9748 = _5085 - _5079;
            float _5091 = (_26521 + _5034) / _4963;
            float _5092 = 0.5 - _5091;
            float _5097 = (_26515 + _5034) / _4963;
            float _5098 = 0.5 - _5097;
            float _9800 = _5098 - _5092;
            _5102 = _26569 + (((abs(_9748) < 0.001000000047497451305389404296875) ? fast::clamp(0.5 * (_5079 + _5085), 0.0, 1.0) : ((((_5085 <= 0.0) ? 0.0 : ((_5085 >= 1.0) ? (-_5084) : ((0.5 * _5085) * _5085))) - ((_5079 <= 0.0) ? 0.0 : ((_5079 >= 1.0) ? (-_5078) : ((0.5 * _5079) * _5079)))) / _9748)) - ((abs(_9800) < 0.001000000047497451305389404296875) ? fast::clamp(0.5 * (_5092 + _5098), 0.0, 1.0) : ((((_5098 <= 0.0) ? 0.0 : ((_5098 >= 1.0) ? (-_5097) : ((0.5 * _5098) * _5098))) - ((_5092 <= 0.0) ? 0.0 : ((_5092 >= 1.0) ? (-_5091) : ((0.5 * _5092) * _5092)))) / _9800)));
        }
        _30162 = ((in.i_flags.x & _3143) != 0u) ? (_26569 / _5023) : (-1.0);
        _26624 = _26545 / _5023;
    }
    else
    {
        _30162 = -1.0;
        _26624 = fast::clamp(0.5 - (_26273 / _4963), 0.0, 1.0);
    }
    float _30477;
    if ((in.i_flags.x & _3153) != 0u)
    {
        uint _9850 = (in.i_instance * 24u) + 19u;
        uint _9858 = (in.i_instance * 24u) + 20u;
        float2 _5147 = fast::max((gFxData_1._data[_9850].zw - gFxData_1._data[_9850].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float2 _5150 = in.i_local - ((gFxData_1._data[_9850].xy + gFxData_1._data[_9850].zw) * 0.5);
        float _9896 = fast::min(_5147.x, _5147.y);
        float _9899 = fast::min((_5150.x > 0.0) ? ((_5150.y > 0.0) ? gFxData_1._data[_9858].x : gFxData_1._data[_9858].x) : ((_5150.y > 0.0) ? gFxData_1._data[_9858].x : gFxData_1._data[_9858].x), _9896);
        float _9905 = _9899 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_9858].y));
        float _26617;
        float _26618;
        if (_9905 > _9896)
        {
            float _9919 = gFxData_1._data[_9858].y * fast::clamp((_9896 - _9899) / fast::max(0.60000002384185791015625 * _9899, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _26618 = _9919;
            _26617 = _9899 * (1.0 + (0.60000002384185791015625 * _9919));
        }
        else
        {
            _26618 = gFxData_1._data[_9858].y;
            _26617 = _9905;
        }
        float2 _9932 = (abs(_5150) - _5147) + float2(_26617);
        float2 _9934 = fast::max(_9932, float2(0.0));
        float _26619;
        if ((_9934.x > 0.0) && (_9934.y > 0.0))
        {
            float _26620;
            if ((_26618 > 0.001000000047497451305389404296875) && (_26617 > 9.9999997473787516355514526367188e-05))
            {
                float _9951 = 2.0 + (2.0 * _26618);
                float2 _9956 = _9934 / float2(fast::max(_26617, 9.9999997473787516355514526367188e-05));
                _26620 = pow(pow(_9956.x, _9951) + pow(_9956.y, _9951), 1.0 / _9951) * _26617;
            }
            else
            {
                _26620 = length(_9934);
            }
            _26619 = _26620;
        }
        else
        {
            _26619 = fast::max(_9934.x, _9934.y);
        }
        float _5162 = fast::clamp(0.5 - (((fast::min(fast::max(_9932.x, _9932.y), 0.0) + _26619) - _26617) / _4963), 0.0, 1.0);
        if (_5162 <= 0.0)
        {
            discard_fragment();
        }
        _30477 = _5162;
    }
    else
    {
        _30477 = 1.0;
    }
    bool _5173 = ((in.i_flags.x & _3220) != 0u) && (_26624 >= 0.999000012874603271484375);
    float4 _26774;
    if ((((in.i_flags.x & _3228) != 0u) && (!((in.i_flags.x & _3233) != 0u))) && (!_5173))
    {
        uint _9997 = (in.i_instance * 24u) + 7u;
        uint _10005 = (in.i_instance * 24u) + 8u;
        float2 _10052 = select(_268.gDrawInfo.yz, _268.gDrawInfo.yz - ((in.i_rect.xy + in.i_rect.zw) * 0.5), bool2(_268.gDrawInfo.w > 1.0));
        float2 _5203 = in.i_local - select(gFxData_1._data[_10005].zw, (-(_10052 / float2(fast::max(length(_10052), 9.9999997473787516355514526367188e-05)))) * length(gFxData_1._data[_10005].zw), bool2(_268.gDrawInfo.w > 0.5));
        float2 _10103 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _10112 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _26691;
        if ((in.i_flags.x & _1710) != 0u)
        {
            float _10229 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
            float4 _10263 = gTex.sample(gLinear, mix(_26219.xy, _26219.zw, (fast::clamp(_5203, _26225.xy, _26225.zw) - _26225.xy) / fast::max(_26225.zw - _26225.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
            float _10287 = (_10263.w - 0.5) * in.i_radii.y;
            float2 _10328 = abs(_5203 - ((_26225.xy + _26225.zw) * 0.5)) - fast::max(((_26225.zw - _26225.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _10229), float2(0.0));
            _26691 = fast::max((-mix((fast::max(fast::min(_10263.x, _10263.y), fast::min(fast::max(_10263.x, _10263.y), _10263.z)) - 0.5) * in.i_radii.x, _10287, fast::clamp(abs(_10287) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _10229, length(fast::max(_10328, float2(0.0))) + fast::min(fast::max(_10328.x, _10328.y), 0.0)) - in.i_shape.w;
        }
        else
        {
            float _26692;
            if ((in.i_flags.x & _1725) != 0u)
            {
                float _26690;
                do
                {
                    bool _10390 = in.i_radii.w > 0.5;
                    float _10394 = floor((in.i_radii.z + 1.0) * 0.5);
                    float _10398 = floor((_10394 + 127.0) * 0.0078125);
                    float _10402 = floor((in.i_radii.z - 1.0) * 0.5);
                    float _10530 = floor(_10402 * 0.0625);
                    uint _10547 = (((in.i_instance + 1u) + uint(_10530)) * 24u) + uint(_10402 - (_10530 * 16.0));
                    float2 _26667;
                    if (_10390)
                    {
                        float _10557 = floor(0.0);
                        _26667 = gFxData_1._data[(((in.i_instance + 1u) + uint(_10557)) * 24u) + uint(_10557 * (-16.0))].xy;
                    }
                    else
                    {
                        _26667 = select(gFxData_1._data[_10547].zw, gFxData_1._data[_10547].xy, bool2(((in.i_radii.z - 1.0) - (_10402 * 2.0)) < 0.5));
                    }
                    float2 _26666;
                    float _26669;
                    float _26672;
                    _26672 = 1.0;
                    _26669 = 1000000015047466219876688855040.0;
                    _26666 = _26667;
                    float2 _26665;
                    float _26668;
                    float _26671;
                    for (float _26662 = 0.0; _26662 < 16.0; _26672 = _26671, _26669 = _26668, _26666 = _26665, _26662 += 1.0)
                    {
                        if (_26662 >= _10398)
                        {
                            break;
                        }
                        _26671 = _26672;
                        _26668 = _26669;
                        _26665 = _26666;
                        float2 _30896;
                        float _30898;
                        float _30900;
                        for (float _26663 = 0.0; _26663 < 128.0; _26671 = _30900, _26668 = _30898, _26665 = _30896, _26663 += 1.0)
                        {
                            float _10444 = (_26662 * 128.0) + _26663;
                            if (_10444 >= _10394)
                            {
                                break;
                            }
                            float _10584 = floor(_10444 * 0.0625);
                            uint _10601 = (((in.i_instance + 1u) + uint(_10584)) * 24u) + uint(_10444 - (_10584 * 16.0));
                            float2 _10614 = _26665 - gFxData_1._data[_10601].xy;
                            float2 _10617 = _5203 - gFxData_1._data[_10601].xy;
                            float2 _10630 = _10617 - (_10614 * fast::clamp(dot(_10617, _10614) / fast::max(dot(_10614, _10614), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                            float _10633 = dot(_10630, _10630);
                            float _31005 = (_10633 < _26668) ? _10633 : _26668;
                            bool _10645 = _5203.y >= gFxData_1._data[_10601].y;
                            bool _10650 = _5203.y < _26665.y;
                            bool _10661 = (_10614.x * _10617.y) > (_10614.y * _10617.x);
                            float _26677;
                            if (((_10645 && _10650) && _10661) || (((!_10645) && (!_10650)) && (!_10661)))
                            {
                                _26677 = -_26671;
                            }
                            else
                            {
                                _26677 = _26671;
                            }
                            if (((2.0 * _10444) + 1.0) < in.i_radii.z)
                            {
                                float2 _10690 = gFxData_1._data[_10601].xy - gFxData_1._data[_10601].zw;
                                float2 _10693 = _5203 - gFxData_1._data[_10601].zw;
                                float2 _10706 = _10693 - (_10690 * fast::clamp(dot(_10693, _10690) / fast::max(dot(_10690, _10690), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _10709 = dot(_10706, _10706);
                                bool _10721 = _5203.y >= gFxData_1._data[_10601].w;
                                bool _10726 = _5203.y < gFxData_1._data[_10601].y;
                                bool _10737 = (_10690.x * _10693.y) > (_10690.y * _10693.x);
                                float _26681;
                                if (((_10721 && _10726) && _10737) || (((!_10721) && (!_10726)) && (!_10737)))
                                {
                                    _26681 = -_26677;
                                }
                                else
                                {
                                    _26681 = _26677;
                                }
                                _30900 = _26681;
                                _30898 = (_10709 < _31005) ? _10709 : _31005;
                                _30896 = gFxData_1._data[_10601].zw;
                            }
                            else
                            {
                                _30900 = _26677;
                                _30898 = _31005;
                                _30896 = gFxData_1._data[_10601].xy;
                            }
                        }
                    }
                    float _10496 = sqrt(_26669);
                    if (_10390)
                    {
                        _26690 = _10496 - in.i_radii.x;
                        break;
                    }
                    _26690 = (_26672 * _10496) - in.i_radii.x;
                    break;
                } while(false);
                _26692 = _26690;
            }
            else
            {
                float _26693;
                if (in.i_flags.z == 1u)
                {
                    float2 _10137 = _5203 - _10103;
                    float _26661;
                    do
                    {
                        if (in.i_shape.w >= 6.282185077667236328125)
                        {
                            _26661 = abs(length(_10137) - in.i_radii.x) - in.i_radii.y;
                            break;
                        }
                        float _10780 = in.i_shape.z + (in.i_shape.w * 0.5);
                        float _10782 = cos(_10780);
                        float _10784 = sin(_10780);
                        float _10793 = dot(_10137, float2(-_10784, _10782));
                        float _10796 = dot(_10137, float2(_10782, _10784));
                        float2 _10797 = float2(_10793, _10796);
                        float _10800 = abs(_10793);
                        _10797.x = _10800;
                        float _10803 = in.i_shape.w * 0.5;
                        float _10805 = sin(_10803);
                        float _10807 = cos(_10803);
                        _26661 = (((_10807 * _10800) > (_10805 * _10796)) ? length(_10797 - (float2(_10805, _10807) * in.i_radii.x)) : abs(length(_10797) - in.i_radii.x)) - in.i_radii.y;
                        break;
                    } while(false);
                    _26693 = _26661;
                }
                else
                {
                    float _26694;
                    if (in.i_flags.z == 2u)
                    {
                        float2 _10843 = _5203 - _26225.xy;
                        float2 _10846 = _26225.zw - _26225.xy;
                        _26694 = length(_10843 - (_10846 * fast::clamp(dot(_10843, _10846) / fast::max(dot(_10846, _10846), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                    }
                    else
                    {
                        float2 _10164 = _5203 - _10103;
                        float _10899 = fast::min(_10112.x, _10112.y);
                        float _10902 = fast::min((_10164.x > 0.0) ? ((_10164.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_10164.y > 0.0) ? in.i_radii.w : in.i_radii.x), _10899);
                        float _10908 = _10902 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _26646;
                        float _26647;
                        if (_10908 > _10899)
                        {
                            float _10922 = in.i_shape.y * fast::clamp((_10899 - _10902) / fast::max(0.60000002384185791015625 * _10902, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _26647 = _10922;
                            _26646 = _10902 * (1.0 + (0.60000002384185791015625 * _10922));
                        }
                        else
                        {
                            _26647 = in.i_shape.y;
                            _26646 = _10908;
                        }
                        float2 _10935 = (abs(_10164) - _10112) + float2(_26646);
                        float2 _10937 = fast::max(_10935, float2(0.0));
                        float _26648;
                        if ((_10937.x > 0.0) && (_10937.y > 0.0))
                        {
                            float _26649;
                            if ((_26647 > 0.001000000047497451305389404296875) && (_26646 > 9.9999997473787516355514526367188e-05))
                            {
                                float _10954 = 2.0 + (2.0 * _26647);
                                float2 _10959 = _10937 / float2(fast::max(_26646, 9.9999997473787516355514526367188e-05));
                                _26649 = pow(pow(_10959.x, _10954) + pow(_10959.y, _10954), 1.0 / _10954) * _26646;
                            }
                            else
                            {
                                _26649 = length(_10937);
                            }
                            _26648 = _26649;
                        }
                        else
                        {
                            _26648 = fast::max(_10937.x, _10937.y);
                        }
                        float _10994 = (fast::min(fast::max(_10935.x, _10935.y), 0.0) + _26648) - _26646;
                        float _26695;
                        if ((in.i_flags.x & _1796) != 0u)
                        {
                            float2 _10192 = fast::max((_26225.zw - _26225.xy) * 0.5, float2(0.001000000047497451305389404296875));
                            float2 _10195 = _5203 - ((_26225.xy + _26225.zw) * 0.5);
                            float _11030 = fast::min(_10192.x, _10192.y);
                            float _11033 = fast::min((_10195.x > 0.0) ? ((_10195.y > 0.0) ? _26223.x : _26223.x) : ((_10195.y > 0.0) ? _26223.x : _26223.x), _11030);
                            float _11039 = _11033 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _26652;
                            float _26653;
                            if (_11039 > _11030)
                            {
                                float _11053 = in.i_shape.y * fast::clamp((_11030 - _11033) / fast::max(0.60000002384185791015625 * _11033, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _26653 = _11053;
                                _26652 = _11033 * (1.0 + (0.60000002384185791015625 * _11053));
                            }
                            else
                            {
                                _26653 = in.i_shape.y;
                                _26652 = _11039;
                            }
                            float2 _11066 = (abs(_10195) - _10192) + float2(_26652);
                            float2 _11068 = fast::max(_11066, float2(0.0));
                            float _26654;
                            if ((_11068.x > 0.0) && (_11068.y > 0.0))
                            {
                                float _26655;
                                if ((_26653 > 0.001000000047497451305389404296875) && (_26652 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _11085 = 2.0 + (2.0 * _26653);
                                    float2 _11090 = _11068 / float2(fast::max(_26652, 9.9999997473787516355514526367188e-05));
                                    _26655 = pow(pow(_11090.x, _11085) + pow(_11090.y, _11085), 1.0 / _11085) * _26652;
                                }
                                else
                                {
                                    _26655 = length(_11068);
                                }
                                _26654 = _26655;
                            }
                            else
                            {
                                _26654 = fast::max(_11068.x, _11068.y);
                            }
                            float _11125 = (fast::min(fast::max(_11066.x, _11066.y), 0.0) + _26654) - _26652;
                            float _11130 = fast::max(_26223.y, 9.9999997473787516355514526367188e-05);
                            float _11139 = fast::max(_11130 - abs(_10994 - _11125), 0.0) / _11130;
                            _26695 = fast::min(_10994, _11125) - (((_11139 * _11139) * _11130) * 0.25);
                        }
                        else
                        {
                            _26695 = _10994;
                        }
                        _26694 = _26695;
                    }
                    _26693 = _26694;
                }
                _26692 = _26693;
            }
            _26691 = _26692;
        }
        float _5212 = (_26691 - gFxData_1._data[_10005].y) / (fast::max(gFxData_1._data[_10005].x * 0.5, _4963 * 0.5) * 1.41421353816986083984375);
        float _11156 = sign(_5212);
        float _11158 = abs(_5212);
        float _11169 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_11158 * _11158))) * _11158)) * _11158);
        float _11172 = _11169 * _11169;
        float _11187 = fast::clamp(gFxData_1._data[_9997].w * (0.5 - (0.5 * (_11156 - (_11156 / (_11172 * _11172))))), 0.0, 1.0);
        _26774 = float4(gFxData_1._data[_9997].xyz * _11187, _11187);
    }
    else
    {
        _26774 = float4(0.0);
    }
    float4 _26874;
    if (((in.i_flags.x & _3298) != 0u) && (!_5173))
    {
        uint _11211 = (in.i_instance * 24u) + 9u;
        uint _11219 = (in.i_instance * 24u) + 10u;
        float _5240 = fast::max(gFxData_1._data[_11219].x, 0.001000000047497451305389404296875);
        float _26767;
        float _26770;
        if ((in.i_flags.x & _3327) != 0u)
        {
            uint _11227 = (in.i_instance * 24u) + 22u;
            float2 _5256 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _5265 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float2 _5272 = in.i_rect.xy - gFxData_1._data[_11227].xy;
            float2 _5279 = gFxData_1._data[_11227].zw - in.i_rect.zw;
            float2 _5309 = fast::clamp(float2((in.i_local.x < _5256.x) ? _5272.x : _5279.x, (in.i_local.y < _5256.y) ? _5272.y : _5279.y) * float2(0.58823525905609130859375), float2(fast::min(_5240, 1.5)), float2(_5240));
            float _5314 = fast::min(_5265.x, _5265.y);
            float _26765;
            if (in.i_flags.z == 0u)
            {
                _26765 = fast::min(((in.i_local.x > _5256.x) ? ((in.i_local.y > _5256.y) ? in.i_radii.z : in.i_radii.y) : ((in.i_local.y > _5256.y) ? in.i_radii.w : in.i_radii.x)) * (1.0 + (0.60000002384185791015625 * in.i_shape.y)), _5314);
            }
            else
            {
                _26765 = _5314;
            }
            float2 _5363 = fast::max(abs(in.i_local - _5256) - (_5265 - float2(_26765)), float2(0.0));
            float _5365 = length(_5363);
            float2 _5373 = select(float2(0.707099974155426025390625), _5363 / float2(_5365), bool2(_5365 > 9.9999997473787516355514526367188e-05));
            float2 _5388 = in.i_local - gFxData_1._data[_11227].xy;
            float2 _5393 = gFxData_1._data[_11227].zw - in.i_local;
            _26770 = fast::clamp(fast::min(fast::min(_5388.x, _5388.y), fast::min(_5393.x, _5393.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _26767 = rsqrt(dot(_5373 * _5373, float2(1.0) / (_5309 * _5309)));
        }
        else
        {
            _26770 = 1.0;
            _26767 = _5240;
        }
        float _5411 = fast::max(_26273, 0.0) / _26767;
        float _11237 = fast::clamp(gFxData_1._data[_11211].w * fast::clamp((exp(((-_5411) * _5411) * 2.2000000476837158203125) * gFxData_1._data[_11219].y) * _26770, 0.0, 1.0), 0.0, 1.0);
        _26874 = float4(gFxData_1._data[_11211].xyz * _11237, _11237) + (_26774 * (1.0 - _11237));
    }
    else
    {
        _26874 = _26774;
    }
    float4 _28316;
    if ((in.i_flags.x & _3544) != 0u)
    {
        float2 _5442 = _26218 * _226.gTarget.zw;
        float _11270 = fast::clamp(log2(fast::max(fast::max(in.i_radii.w / _4963, 4.0), 1.0)) - 1.0, 0.0, 5.0);
        int _11273 = int(floor(_11270));
        float _11277 = _11270 - float(_11273);
        bool _11282 = (_11277 > 0.0199999995529651641845703125) && (_11273 < 5);
        float3 _26846;
        _26846 = float3(0.0);
        float3 _11311;
        for (int _26776 = 0; _26776 < 2; _26846 = _11311, _26776++)
        {
            if ((_26776 > 0) && (!_11282))
            {
                break;
            }
            int _11297 = _11273 + _26776;
            int _11335 = clamp(_11297, 1, 5);
            float2 _11404 = (_5442 * _226.gLevel[_11335].xy) - float2(0.5);
            float2 _11406 = floor(_11404);
            float2 _11409 = _11404 - _11406;
            float2 _11412 = _11409 * _11409;
            float2 _11415 = _11412 * _11409;
            float2 _11434 = (((_11415 * 3.0) - (_11412 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
            float2 _11447 = _11415 * 0.16666667163372039794921875;
            float2 _11450 = (((((-_11415) + (_11412 * 3.0)) - (_11409 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11434;
            float2 _11454 = (((((_11415 * (-3.0)) + (_11412 * 3.0)) + (_11409 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11447;
            float2 _11466 = ((_11406 - float2(0.5)) + (_11434 / _11450)) * _226.gLevel[_11335].zw;
            float2 _11478 = ((_11406 + float2(1.5)) + (_11447 / _11454)) * _226.gLevel[_11335].zw;
            float4 _26813;
            if (_11297 <= 0)
            {
                float2 _26812;
                if (_226.gConv.x > 0.5)
                {
                    float2 _24979 = _5442;
                    _24979.y = 1.0 - _5442.y;
                    _26812 = _24979;
                }
                else
                {
                    _26812 = _5442;
                }
                _26813 = gBackdrop0.sample(gLinear, _26812, level(0.0));
            }
            else
            {
                float4 _26814;
                if (_11297 == 1)
                {
                    float2 _26805;
                    if (_226.gConv.x > 0.5)
                    {
                        float2 _24984 = _11466;
                        _24984.y = 1.0 - _11466.y;
                        _26805 = _24984;
                    }
                    else
                    {
                        _26805 = _11466;
                    }
                    float _11523 = _11466.y;
                    float2 _11524 = float2(_11478.x, _11523);
                    float2 _26806;
                    if (_226.gConv.x > 0.5)
                    {
                        float2 _24991 = _11524;
                        _24991.y = 1.0 - _11523;
                        _26806 = _24991;
                    }
                    else
                    {
                        _26806 = _11524;
                    }
                    float _11541 = _11478.y;
                    float2 _11542 = float2(_11466.x, _11541);
                    float2 _26808;
                    if (_226.gConv.x > 0.5)
                    {
                        float2 _24998 = _11542;
                        _24998.y = 1.0 - _11541;
                        _26808 = _24998;
                    }
                    else
                    {
                        _26808 = _11542;
                    }
                    float2 _26810;
                    if (_226.gConv.x > 0.5)
                    {
                        float2 _25005 = _11478;
                        _25005.y = 1.0 - _11478.y;
                        _26810 = _25005;
                    }
                    else
                    {
                        _26810 = _11478;
                    }
                    _26814 = (((gBackdrop1.sample(gLinear, _26805, level(0.0)) * (_11450.x * _11450.y)) + (gBackdrop1.sample(gLinear, _26806, level(0.0)) * (_11454.x * _11450.y))) + (gBackdrop1.sample(gLinear, _26808, level(0.0)) * (_11450.x * _11454.y))) + (gBackdrop1.sample(gLinear, _26810, level(0.0)) * (_11454.x * _11454.y));
                }
                else
                {
                    float4 _26815;
                    if (_11297 == 2)
                    {
                        float2 _26798;
                        if (_226.gConv.x > 0.5)
                        {
                            float2 _25012 = _11466;
                            _25012.y = 1.0 - _11466.y;
                            _26798 = _25012;
                        }
                        else
                        {
                            _26798 = _11466;
                        }
                        float _11653 = _11466.y;
                        float2 _11654 = float2(_11478.x, _11653);
                        float2 _26799;
                        if (_226.gConv.x > 0.5)
                        {
                            float2 _25019 = _11654;
                            _25019.y = 1.0 - _11653;
                            _26799 = _25019;
                        }
                        else
                        {
                            _26799 = _11654;
                        }
                        float _11671 = _11478.y;
                        float2 _11672 = float2(_11466.x, _11671);
                        float2 _26801;
                        if (_226.gConv.x > 0.5)
                        {
                            float2 _25026 = _11672;
                            _25026.y = 1.0 - _11671;
                            _26801 = _25026;
                        }
                        else
                        {
                            _26801 = _11672;
                        }
                        float2 _26803;
                        if (_226.gConv.x > 0.5)
                        {
                            float2 _25033 = _11478;
                            _25033.y = 1.0 - _11478.y;
                            _26803 = _25033;
                        }
                        else
                        {
                            _26803 = _11478;
                        }
                        _26815 = (((gBackdrop2.sample(gLinear, _26798, level(0.0)) * (_11450.x * _11450.y)) + (gBackdrop2.sample(gLinear, _26799, level(0.0)) * (_11454.x * _11450.y))) + (gBackdrop2.sample(gLinear, _26801, level(0.0)) * (_11450.x * _11454.y))) + (gBackdrop2.sample(gLinear, _26803, level(0.0)) * (_11454.x * _11454.y));
                    }
                    else
                    {
                        float4 _26816;
                        if (_11297 == 3)
                        {
                            float2 _26791;
                            if (_226.gConv.x > 0.5)
                            {
                                float2 _25040 = _11466;
                                _25040.y = 1.0 - _11466.y;
                                _26791 = _25040;
                            }
                            else
                            {
                                _26791 = _11466;
                            }
                            float _11783 = _11466.y;
                            float2 _11784 = float2(_11478.x, _11783);
                            float2 _26792;
                            if (_226.gConv.x > 0.5)
                            {
                                float2 _25047 = _11784;
                                _25047.y = 1.0 - _11783;
                                _26792 = _25047;
                            }
                            else
                            {
                                _26792 = _11784;
                            }
                            float _11801 = _11478.y;
                            float2 _11802 = float2(_11466.x, _11801);
                            float2 _26794;
                            if (_226.gConv.x > 0.5)
                            {
                                float2 _25054 = _11802;
                                _25054.y = 1.0 - _11801;
                                _26794 = _25054;
                            }
                            else
                            {
                                _26794 = _11802;
                            }
                            float2 _26796;
                            if (_226.gConv.x > 0.5)
                            {
                                float2 _25061 = _11478;
                                _25061.y = 1.0 - _11478.y;
                                _26796 = _25061;
                            }
                            else
                            {
                                _26796 = _11478;
                            }
                            _26816 = (((gBackdrop3.sample(gLinear, _26791, level(0.0)) * (_11450.x * _11450.y)) + (gBackdrop3.sample(gLinear, _26792, level(0.0)) * (_11454.x * _11450.y))) + (gBackdrop3.sample(gLinear, _26794, level(0.0)) * (_11450.x * _11454.y))) + (gBackdrop3.sample(gLinear, _26796, level(0.0)) * (_11454.x * _11454.y));
                        }
                        else
                        {
                            float4 _26817;
                            if (_11297 == 4)
                            {
                                float2 _26784;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25068 = _11466;
                                    _25068.y = 1.0 - _11466.y;
                                    _26784 = _25068;
                                }
                                else
                                {
                                    _26784 = _11466;
                                }
                                float _11913 = _11466.y;
                                float2 _11914 = float2(_11478.x, _11913);
                                float2 _26785;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25075 = _11914;
                                    _25075.y = 1.0 - _11913;
                                    _26785 = _25075;
                                }
                                else
                                {
                                    _26785 = _11914;
                                }
                                float _11931 = _11478.y;
                                float2 _11932 = float2(_11466.x, _11931);
                                float2 _26787;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25082 = _11932;
                                    _25082.y = 1.0 - _11931;
                                    _26787 = _25082;
                                }
                                else
                                {
                                    _26787 = _11932;
                                }
                                float2 _26789;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25089 = _11478;
                                    _25089.y = 1.0 - _11478.y;
                                    _26789 = _25089;
                                }
                                else
                                {
                                    _26789 = _11478;
                                }
                                _26817 = (((gBackdrop4.sample(gLinear, _26784, level(0.0)) * (_11450.x * _11450.y)) + (gBackdrop4.sample(gLinear, _26785, level(0.0)) * (_11454.x * _11450.y))) + (gBackdrop4.sample(gLinear, _26787, level(0.0)) * (_11450.x * _11454.y))) + (gBackdrop4.sample(gLinear, _26789, level(0.0)) * (_11454.x * _11454.y));
                            }
                            else
                            {
                                float2 _26777;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25096 = _11466;
                                    _25096.y = 1.0 - _11466.y;
                                    _26777 = _25096;
                                }
                                else
                                {
                                    _26777 = _11466;
                                }
                                float _12043 = _11466.y;
                                float2 _12044 = float2(_11478.x, _12043);
                                float2 _26778;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25103 = _12044;
                                    _25103.y = 1.0 - _12043;
                                    _26778 = _25103;
                                }
                                else
                                {
                                    _26778 = _12044;
                                }
                                float _12061 = _11478.y;
                                float2 _12062 = float2(_11466.x, _12061);
                                float2 _26780;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25110 = _12062;
                                    _25110.y = 1.0 - _12061;
                                    _26780 = _25110;
                                }
                                else
                                {
                                    _26780 = _12062;
                                }
                                float2 _26782;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25117 = _11478;
                                    _25117.y = 1.0 - _11478.y;
                                    _26782 = _25117;
                                }
                                else
                                {
                                    _26782 = _11478;
                                }
                                _26817 = (((gBackdrop5.sample(gLinear, _26777, level(0.0)) * (_11450.x * _11450.y)) + (gBackdrop5.sample(gLinear, _26778, level(0.0)) * (_11454.x * _11450.y))) + (gBackdrop5.sample(gLinear, _26780, level(0.0)) * (_11450.x * _11454.y))) + (gBackdrop5.sample(gLinear, _26782, level(0.0)) * (_11454.x * _11454.y));
                            }
                            _26816 = _26817;
                        }
                        _26815 = _26816;
                    }
                    _26814 = _26815;
                }
                _26813 = _26814;
            }
            _11311 = _26846 + (_26813.xyz * (_11282 ? ((_26776 == 0) ? (1.0 - _11277) : _11277) : 1.0));
        }
        float _12147 = dot(in.i_fill0.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
        float _5472 = fast::max(_26273, 0.0) / fast::max(in.i_radii.w, 0.001000000047497451305389404296875);
        float _5475 = (_12147 > 0.5) ? 0.0 : 1.0;
        float _12159 = fast::clamp(0.85000002384185791015625 * ((fast::clamp((0.5 - abs(_12147 - dot(select(float3(0.5), _26846, bool3(_226.gTime.z > 0.5)), float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)))) * 4.0, 0.0, 1.0) * in.i_radii.z) * exp(((-_5472) * _5472) * 2.2000000476837158203125)), 0.0, 1.0);
        _28316 = float4(float4(_5475, _5475, _5475, 0.85000002384185791015625).xyz * _12159, _12159) + (_26874 * (1.0 - _12159));
    }
    else
    {
        _28316 = _26874;
    }
    float4 _29237;
    float4 _29473;
    float _30119;
    if (((in.i_flags.x & _3620) != 0u) && (_26624 > 0.0))
    {
        uint _12183 = (in.i_instance * 24u) + 11u;
        uint _12191 = (in.i_instance * 24u) + 12u;
        uint _12199 = (in.i_instance * 24u) + 13u;
        uint _12207 = (in.i_instance * 24u) + 16u;
        float _12280 = gFxData_1._data[_12183].x * _226.gDisplay.z;
        float2 _12291 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(1.0));
        float _12299 = fast::clamp(gFxData_1._data[_12183].z, 0.001000000047497451305389404296875, fast::min(_12291.x, _12291.y));
        float _12308 = fast::clamp(1.0 - (fast::max(-_26273, 0.0) / _12299), 0.0, 1.0);
        float _12314 = sqrt(fast::clamp(1.0 - (_12308 * _12308), 0.0, 1.0));
        float2 _27307;
        float3 _27884;
        if (_12308 > 0.0)
        {
            float2 _27306;
            do
            {
                if ((in.i_flags.x & _1848) != 0u)
                {
                    float2 _27305;
                    do
                    {
                        bool _12818 = in.i_radii.w > 0.5;
                        float _12822 = floor((in.i_radii.z + 1.0) * 0.5);
                        float _12826 = floor((_12822 + 127.0) * 0.0078125);
                        float _12830 = floor((in.i_radii.z - 1.0) * 0.5);
                        float _12958 = floor(_12830 * 0.0625);
                        uint _12975 = (((in.i_instance + 1u) + uint(_12958)) * 24u) + uint(_12830 - (_12958 * 16.0));
                        float2 _27274;
                        if (_12818)
                        {
                            float _12985 = floor(0.0);
                            _27274 = gFxData_1._data[(((in.i_instance + 1u) + uint(_12985)) * 24u) + uint(_12985 * (-16.0))].xy;
                        }
                        else
                        {
                            _27274 = select(gFxData_1._data[_12975].zw, gFxData_1._data[_12975].xy, bool2(((in.i_radii.z - 1.0) - (_12830 * 2.0)) < 0.5));
                        }
                        float2 _27273;
                        float _27276;
                        float2 _27279;
                        float _27282;
                        _27282 = 1.0;
                        _27279 = float2(0.0, -1.0);
                        _27276 = 1000000015047466219876688855040.0;
                        _27273 = _27274;
                        float2 _27272;
                        float _27275;
                        float2 _27278;
                        float _27281;
                        for (float _27269 = 0.0; _27269 < 16.0; _27282 = _27281, _27279 = _27278, _27276 = _27275, _27273 = _27272, _27269 += 1.0)
                        {
                            if (_27269 >= _12826)
                            {
                                break;
                            }
                            _27281 = _27282;
                            _27278 = _27279;
                            _27275 = _27276;
                            _27272 = _27273;
                            float2 _30951;
                            float _30953;
                            float2 _30955;
                            float _30957;
                            for (float _27270 = 0.0; _27270 < 128.0; _27281 = _30957, _27278 = _30955, _27275 = _30953, _27272 = _30951, _27270 += 1.0)
                            {
                                float _12872 = (_27269 * 128.0) + _27270;
                                if (_12872 >= _12822)
                                {
                                    break;
                                }
                                float _13012 = floor(_12872 * 0.0625);
                                uint _13029 = (((in.i_instance + 1u) + uint(_13012)) * 24u) + uint(_12872 - (_13012 * 16.0));
                                float2 _13042 = _27272 - gFxData_1._data[_13029].xy;
                                float2 _13045 = in.i_local - gFxData_1._data[_13029].xy;
                                float2 _13058 = _13045 - (_13042 * fast::clamp(dot(_13045, _13042) / fast::max(dot(_13042, _13042), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _13061 = dot(_13058, _13058);
                                bool _13064 = _13061 < _27275;
                                float2 _31008 = select(_27278, _13058, bool2(_13064));
                                float _31009 = _13064 ? _13061 : _27275;
                                bool _13073 = in.i_local.y >= gFxData_1._data[_13029].y;
                                bool _13078 = in.i_local.y < _27272.y;
                                bool _13089 = (_13042.x * _13045.y) > (_13042.y * _13045.x);
                                float _27289;
                                if (((_13073 && _13078) && _13089) || (((!_13073) && (!_13078)) && (!_13089)))
                                {
                                    _27289 = -_27281;
                                }
                                else
                                {
                                    _27289 = _27281;
                                }
                                if (((2.0 * _12872) + 1.0) < in.i_radii.z)
                                {
                                    float2 _13118 = gFxData_1._data[_13029].xy - gFxData_1._data[_13029].zw;
                                    float2 _13121 = in.i_local - gFxData_1._data[_13029].zw;
                                    float2 _13134 = _13121 - (_13118 * fast::clamp(dot(_13121, _13118) / fast::max(dot(_13118, _13118), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _13137 = dot(_13134, _13134);
                                    bool _13140 = _13137 < _31009;
                                    bool _13149 = in.i_local.y >= gFxData_1._data[_13029].w;
                                    bool _13154 = in.i_local.y < gFxData_1._data[_13029].y;
                                    bool _13165 = (_13118.x * _13121.y) > (_13118.y * _13121.x);
                                    float _27295;
                                    if (((_13149 && _13154) && _13165) || (((!_13149) && (!_13154)) && (!_13165)))
                                    {
                                        _27295 = -_27289;
                                    }
                                    else
                                    {
                                        _27295 = _27289;
                                    }
                                    _30957 = _27295;
                                    _30955 = select(_31008, _13134, bool2(_13140));
                                    _30953 = _13140 ? _13137 : _31009;
                                    _30951 = gFxData_1._data[_13029].zw;
                                }
                                else
                                {
                                    _30957 = _27289;
                                    _30955 = _31008;
                                    _30953 = _31009;
                                    _30951 = gFxData_1._data[_13029].xy;
                                }
                            }
                        }
                        float _12924 = sqrt(_27276);
                        float2 _12932 = select(float2(0.0, -1.0), _27279 / float2(_12924), bool2(_12924 > 9.9999999747524270787835121154785e-07));
                        if (_12818)
                        {
                            _27305 = _12932;
                            break;
                        }
                        _27305 = _12932 * _27282;
                        break;
                    } while(false);
                    _27306 = _27305;
                    break;
                }
                float2 _12744 = in.i_local + float2(0.5, 0.0);
                float2 _13228 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
                float2 _13237 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                float _27114;
                if ((in.i_flags.x & _1710) != 0u)
                {
                    float _13354 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                    float4 _13388 = gTex.sample(gLinear, mix(_26219.xy, _26219.zw, (fast::clamp(_12744, _26225.xy, _26225.zw) - _26225.xy) / fast::max(_26225.zw - _26225.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                    float _13412 = (_13388.w - 0.5) * in.i_radii.y;
                    float2 _13453 = abs(_12744 - ((_26225.xy + _26225.zw) * 0.5)) - fast::max(((_26225.zw - _26225.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _13354), float2(0.0));
                    _27114 = fast::max((-mix((fast::max(fast::min(_13388.x, _13388.y), fast::min(fast::max(_13388.x, _13388.y), _13388.z)) - 0.5) * in.i_radii.x, _13412, fast::clamp(abs(_13412) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _13354, length(fast::max(_13453, float2(0.0))) + fast::min(fast::max(_13453.x, _13453.y), 0.0)) - in.i_shape.w;
                }
                else
                {
                    float _27115;
                    if ((in.i_flags.x & _1725) != 0u)
                    {
                        float _27113;
                        do
                        {
                            bool _13515 = in.i_radii.w > 0.5;
                            float _13519 = floor((in.i_radii.z + 1.0) * 0.5);
                            float _13523 = floor((_13519 + 127.0) * 0.0078125);
                            float _13527 = floor((in.i_radii.z - 1.0) * 0.5);
                            float _13655 = floor(_13527 * 0.0625);
                            uint _13672 = (((in.i_instance + 1u) + uint(_13655)) * 24u) + uint(_13527 - (_13655 * 16.0));
                            float2 _27090;
                            if (_13515)
                            {
                                float _13682 = floor(0.0);
                                _27090 = gFxData_1._data[(((in.i_instance + 1u) + uint(_13682)) * 24u) + uint(_13682 * (-16.0))].xy;
                            }
                            else
                            {
                                _27090 = select(gFxData_1._data[_13672].zw, gFxData_1._data[_13672].xy, bool2(((in.i_radii.z - 1.0) - (_13527 * 2.0)) < 0.5));
                            }
                            float2 _27089;
                            float _27092;
                            float _27095;
                            _27095 = 1.0;
                            _27092 = 1000000015047466219876688855040.0;
                            _27089 = _27090;
                            float2 _27088;
                            float _27091;
                            float _27094;
                            for (float _27085 = 0.0; _27085 < 16.0; _27095 = _27094, _27092 = _27091, _27089 = _27088, _27085 += 1.0)
                            {
                                if (_27085 >= _13523)
                                {
                                    break;
                                }
                                _27094 = _27095;
                                _27091 = _27092;
                                _27088 = _27089;
                                float2 _30907;
                                float _30909;
                                float _30911;
                                for (float _27086 = 0.0; _27086 < 128.0; _27094 = _30911, _27091 = _30909, _27088 = _30907, _27086 += 1.0)
                                {
                                    float _13569 = (_27085 * 128.0) + _27086;
                                    if (_13569 >= _13519)
                                    {
                                        break;
                                    }
                                    float _13709 = floor(_13569 * 0.0625);
                                    uint _13726 = (((in.i_instance + 1u) + uint(_13709)) * 24u) + uint(_13569 - (_13709 * 16.0));
                                    float2 _13739 = _27088 - gFxData_1._data[_13726].xy;
                                    float2 _13742 = _12744 - gFxData_1._data[_13726].xy;
                                    float2 _13755 = _13742 - (_13739 * fast::clamp(dot(_13742, _13739) / fast::max(dot(_13739, _13739), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _13758 = dot(_13755, _13755);
                                    float _31013 = (_13758 < _27091) ? _13758 : _27091;
                                    bool _13770 = _12744.y >= gFxData_1._data[_13726].y;
                                    bool _13775 = _12744.y < _27088.y;
                                    bool _13786 = (_13739.x * _13742.y) > (_13739.y * _13742.x);
                                    float _27100;
                                    if (((_13770 && _13775) && _13786) || (((!_13770) && (!_13775)) && (!_13786)))
                                    {
                                        _27100 = -_27094;
                                    }
                                    else
                                    {
                                        _27100 = _27094;
                                    }
                                    if (((2.0 * _13569) + 1.0) < in.i_radii.z)
                                    {
                                        float2 _13815 = gFxData_1._data[_13726].xy - gFxData_1._data[_13726].zw;
                                        float2 _13818 = _12744 - gFxData_1._data[_13726].zw;
                                        float2 _13831 = _13818 - (_13815 * fast::clamp(dot(_13818, _13815) / fast::max(dot(_13815, _13815), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _13834 = dot(_13831, _13831);
                                        bool _13846 = _12744.y >= gFxData_1._data[_13726].w;
                                        bool _13851 = _12744.y < gFxData_1._data[_13726].y;
                                        bool _13862 = (_13815.x * _13818.y) > (_13815.y * _13818.x);
                                        float _27104;
                                        if (((_13846 && _13851) && _13862) || (((!_13846) && (!_13851)) && (!_13862)))
                                        {
                                            _27104 = -_27100;
                                        }
                                        else
                                        {
                                            _27104 = _27100;
                                        }
                                        _30911 = _27104;
                                        _30909 = (_13834 < _31013) ? _13834 : _31013;
                                        _30907 = gFxData_1._data[_13726].zw;
                                    }
                                    else
                                    {
                                        _30911 = _27100;
                                        _30909 = _31013;
                                        _30907 = gFxData_1._data[_13726].xy;
                                    }
                                }
                            }
                            float _13621 = sqrt(_27092);
                            if (_13515)
                            {
                                _27113 = _13621 - in.i_radii.x;
                                break;
                            }
                            _27113 = (_27095 * _13621) - in.i_radii.x;
                            break;
                        } while(false);
                        _27115 = _27113;
                    }
                    else
                    {
                        float _27116;
                        if (in.i_flags.z == 1u)
                        {
                            float2 _13262 = _12744 - _13228;
                            float _27084;
                            do
                            {
                                if (in.i_shape.w >= 6.282185077667236328125)
                                {
                                    _27084 = abs(length(_13262) - in.i_radii.x) - in.i_radii.y;
                                    break;
                                }
                                float _13905 = in.i_shape.z + (in.i_shape.w * 0.5);
                                float _13907 = cos(_13905);
                                float _13909 = sin(_13905);
                                float _13918 = dot(_13262, float2(-_13909, _13907));
                                float _13921 = dot(_13262, float2(_13907, _13909));
                                float2 _13922 = float2(_13918, _13921);
                                float _13925 = abs(_13918);
                                _13922.x = _13925;
                                float _13928 = in.i_shape.w * 0.5;
                                float _13930 = sin(_13928);
                                float _13932 = cos(_13928);
                                _27084 = (((_13932 * _13925) > (_13930 * _13921)) ? length(_13922 - (float2(_13930, _13932) * in.i_radii.x)) : abs(length(_13922) - in.i_radii.x)) - in.i_radii.y;
                                break;
                            } while(false);
                            _27116 = _27084;
                        }
                        else
                        {
                            float _27117;
                            if (in.i_flags.z == 2u)
                            {
                                float2 _13968 = _12744 - _26225.xy;
                                float2 _13971 = _26225.zw - _26225.xy;
                                _27117 = length(_13968 - (_13971 * fast::clamp(dot(_13968, _13971) / fast::max(dot(_13971, _13971), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                            }
                            else
                            {
                                float2 _13289 = _12744 - _13228;
                                float _14024 = fast::min(_13237.x, _13237.y);
                                float _14027 = fast::min((_13289.x > 0.0) ? ((_13289.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_13289.y > 0.0) ? in.i_radii.w : in.i_radii.x), _14024);
                                float _14033 = _14027 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                float _27069;
                                float _27070;
                                if (_14033 > _14024)
                                {
                                    float _14047 = in.i_shape.y * fast::clamp((_14024 - _14027) / fast::max(0.60000002384185791015625 * _14027, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                    _27070 = _14047;
                                    _27069 = _14027 * (1.0 + (0.60000002384185791015625 * _14047));
                                }
                                else
                                {
                                    _27070 = in.i_shape.y;
                                    _27069 = _14033;
                                }
                                float2 _14060 = (abs(_13289) - _13237) + float2(_27069);
                                float2 _14062 = fast::max(_14060, float2(0.0));
                                float _27071;
                                if ((_14062.x > 0.0) && (_14062.y > 0.0))
                                {
                                    float _27072;
                                    if ((_27070 > 0.001000000047497451305389404296875) && (_27069 > 9.9999997473787516355514526367188e-05))
                                    {
                                        float _14079 = 2.0 + (2.0 * _27070);
                                        float2 _14084 = _14062 / float2(fast::max(_27069, 9.9999997473787516355514526367188e-05));
                                        _27072 = pow(pow(_14084.x, _14079) + pow(_14084.y, _14079), 1.0 / _14079) * _27069;
                                    }
                                    else
                                    {
                                        _27072 = length(_14062);
                                    }
                                    _27071 = _27072;
                                }
                                else
                                {
                                    _27071 = fast::max(_14062.x, _14062.y);
                                }
                                float _14119 = (fast::min(fast::max(_14060.x, _14060.y), 0.0) + _27071) - _27069;
                                float _27118;
                                if ((in.i_flags.x & _1796) != 0u)
                                {
                                    float2 _13317 = fast::max((_26225.zw - _26225.xy) * 0.5, float2(0.001000000047497451305389404296875));
                                    float2 _13320 = _12744 - ((_26225.xy + _26225.zw) * 0.5);
                                    float _14155 = fast::min(_13317.x, _13317.y);
                                    float _14158 = fast::min((_13320.x > 0.0) ? ((_13320.y > 0.0) ? gFxData_1._data[_12207].x : gFxData_1._data[_12207].x) : ((_13320.y > 0.0) ? gFxData_1._data[_12207].x : gFxData_1._data[_12207].x), _14155);
                                    float _14164 = _14158 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                    float _27075;
                                    float _27076;
                                    if (_14164 > _14155)
                                    {
                                        float _14178 = in.i_shape.y * fast::clamp((_14155 - _14158) / fast::max(0.60000002384185791015625 * _14158, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                        _27076 = _14178;
                                        _27075 = _14158 * (1.0 + (0.60000002384185791015625 * _14178));
                                    }
                                    else
                                    {
                                        _27076 = in.i_shape.y;
                                        _27075 = _14164;
                                    }
                                    float2 _14191 = (abs(_13320) - _13317) + float2(_27075);
                                    float2 _14193 = fast::max(_14191, float2(0.0));
                                    float _27077;
                                    if ((_14193.x > 0.0) && (_14193.y > 0.0))
                                    {
                                        float _27078;
                                        if ((_27076 > 0.001000000047497451305389404296875) && (_27075 > 9.9999997473787516355514526367188e-05))
                                        {
                                            float _14210 = 2.0 + (2.0 * _27076);
                                            float2 _14215 = _14193 / float2(fast::max(_27075, 9.9999997473787516355514526367188e-05));
                                            _27078 = pow(pow(_14215.x, _14210) + pow(_14215.y, _14210), 1.0 / _14210) * _27075;
                                        }
                                        else
                                        {
                                            _27078 = length(_14193);
                                        }
                                        _27077 = _27078;
                                    }
                                    else
                                    {
                                        _27077 = fast::max(_14193.x, _14193.y);
                                    }
                                    float _14250 = (fast::min(fast::max(_14191.x, _14191.y), 0.0) + _27077) - _27075;
                                    float _14255 = fast::max(gFxData_1._data[_12207].y, 9.9999997473787516355514526367188e-05);
                                    float _14264 = fast::max(_14255 - abs(_14119 - _14250), 0.0) / _14255;
                                    _27118 = fast::min(_14119, _14250) - (((_14264 * _14264) * _14255) * 0.25);
                                }
                                else
                                {
                                    _27118 = _14119;
                                }
                                _27117 = _27118;
                            }
                            _27116 = _27117;
                        }
                        _27115 = _27116;
                    }
                    _27114 = _27115;
                }
                float2 _12748 = in.i_local - float2(0.5, 0.0);
                float2 _14319 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
                float2 _14328 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                float _27164;
                if ((in.i_flags.x & _1710) != 0u)
                {
                    float _14445 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                    float4 _14479 = gTex.sample(gLinear, mix(_26219.xy, _26219.zw, (fast::clamp(_12748, _26225.xy, _26225.zw) - _26225.xy) / fast::max(_26225.zw - _26225.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                    float _14503 = (_14479.w - 0.5) * in.i_radii.y;
                    float2 _14544 = abs(_12748 - ((_26225.xy + _26225.zw) * 0.5)) - fast::max(((_26225.zw - _26225.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _14445), float2(0.0));
                    _27164 = fast::max((-mix((fast::max(fast::min(_14479.x, _14479.y), fast::min(fast::max(_14479.x, _14479.y), _14479.z)) - 0.5) * in.i_radii.x, _14503, fast::clamp(abs(_14503) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _14445, length(fast::max(_14544, float2(0.0))) + fast::min(fast::max(_14544.x, _14544.y), 0.0)) - in.i_shape.w;
                }
                else
                {
                    float _27165;
                    if ((in.i_flags.x & _1725) != 0u)
                    {
                        float _27163;
                        do
                        {
                            bool _14606 = in.i_radii.w > 0.5;
                            float _14610 = floor((in.i_radii.z + 1.0) * 0.5);
                            float _14614 = floor((_14610 + 127.0) * 0.0078125);
                            float _14618 = floor((in.i_radii.z - 1.0) * 0.5);
                            float _14746 = floor(_14618 * 0.0625);
                            uint _14763 = (((in.i_instance + 1u) + uint(_14746)) * 24u) + uint(_14618 - (_14746 * 16.0));
                            float2 _27140;
                            if (_14606)
                            {
                                float _14773 = floor(0.0);
                                _27140 = gFxData_1._data[(((in.i_instance + 1u) + uint(_14773)) * 24u) + uint(_14773 * (-16.0))].xy;
                            }
                            else
                            {
                                _27140 = select(gFxData_1._data[_14763].zw, gFxData_1._data[_14763].xy, bool2(((in.i_radii.z - 1.0) - (_14618 * 2.0)) < 0.5));
                            }
                            float2 _27139;
                            float _27142;
                            float _27145;
                            _27145 = 1.0;
                            _27142 = 1000000015047466219876688855040.0;
                            _27139 = _27140;
                            float2 _27138;
                            float _27141;
                            float _27144;
                            for (float _27135 = 0.0; _27135 < 16.0; _27145 = _27144, _27142 = _27141, _27139 = _27138, _27135 += 1.0)
                            {
                                if (_27135 >= _14614)
                                {
                                    break;
                                }
                                _27144 = _27145;
                                _27141 = _27142;
                                _27138 = _27139;
                                float2 _30918;
                                float _30920;
                                float _30922;
                                for (float _27136 = 0.0; _27136 < 128.0; _27144 = _30922, _27141 = _30920, _27138 = _30918, _27136 += 1.0)
                                {
                                    float _14660 = (_27135 * 128.0) + _27136;
                                    if (_14660 >= _14610)
                                    {
                                        break;
                                    }
                                    float _14800 = floor(_14660 * 0.0625);
                                    uint _14817 = (((in.i_instance + 1u) + uint(_14800)) * 24u) + uint(_14660 - (_14800 * 16.0));
                                    float2 _14830 = _27138 - gFxData_1._data[_14817].xy;
                                    float2 _14833 = _12748 - gFxData_1._data[_14817].xy;
                                    float2 _14846 = _14833 - (_14830 * fast::clamp(dot(_14833, _14830) / fast::max(dot(_14830, _14830), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _14849 = dot(_14846, _14846);
                                    float _31015 = (_14849 < _27141) ? _14849 : _27141;
                                    bool _14861 = _12748.y >= gFxData_1._data[_14817].y;
                                    bool _14866 = _12748.y < _27138.y;
                                    bool _14877 = (_14830.x * _14833.y) > (_14830.y * _14833.x);
                                    float _27150;
                                    if (((_14861 && _14866) && _14877) || (((!_14861) && (!_14866)) && (!_14877)))
                                    {
                                        _27150 = -_27144;
                                    }
                                    else
                                    {
                                        _27150 = _27144;
                                    }
                                    if (((2.0 * _14660) + 1.0) < in.i_radii.z)
                                    {
                                        float2 _14906 = gFxData_1._data[_14817].xy - gFxData_1._data[_14817].zw;
                                        float2 _14909 = _12748 - gFxData_1._data[_14817].zw;
                                        float2 _14922 = _14909 - (_14906 * fast::clamp(dot(_14909, _14906) / fast::max(dot(_14906, _14906), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _14925 = dot(_14922, _14922);
                                        bool _14937 = _12748.y >= gFxData_1._data[_14817].w;
                                        bool _14942 = _12748.y < gFxData_1._data[_14817].y;
                                        bool _14953 = (_14906.x * _14909.y) > (_14906.y * _14909.x);
                                        float _27154;
                                        if (((_14937 && _14942) && _14953) || (((!_14937) && (!_14942)) && (!_14953)))
                                        {
                                            _27154 = -_27150;
                                        }
                                        else
                                        {
                                            _27154 = _27150;
                                        }
                                        _30922 = _27154;
                                        _30920 = (_14925 < _31015) ? _14925 : _31015;
                                        _30918 = gFxData_1._data[_14817].zw;
                                    }
                                    else
                                    {
                                        _30922 = _27150;
                                        _30920 = _31015;
                                        _30918 = gFxData_1._data[_14817].xy;
                                    }
                                }
                            }
                            float _14712 = sqrt(_27142);
                            if (_14606)
                            {
                                _27163 = _14712 - in.i_radii.x;
                                break;
                            }
                            _27163 = (_27145 * _14712) - in.i_radii.x;
                            break;
                        } while(false);
                        _27165 = _27163;
                    }
                    else
                    {
                        float _27166;
                        if (in.i_flags.z == 1u)
                        {
                            float2 _14353 = _12748 - _14319;
                            float _27134;
                            do
                            {
                                if (in.i_shape.w >= 6.282185077667236328125)
                                {
                                    _27134 = abs(length(_14353) - in.i_radii.x) - in.i_radii.y;
                                    break;
                                }
                                float _14996 = in.i_shape.z + (in.i_shape.w * 0.5);
                                float _14998 = cos(_14996);
                                float _15000 = sin(_14996);
                                float _15009 = dot(_14353, float2(-_15000, _14998));
                                float _15012 = dot(_14353, float2(_14998, _15000));
                                float2 _15013 = float2(_15009, _15012);
                                float _15016 = abs(_15009);
                                _15013.x = _15016;
                                float _15019 = in.i_shape.w * 0.5;
                                float _15021 = sin(_15019);
                                float _15023 = cos(_15019);
                                _27134 = (((_15023 * _15016) > (_15021 * _15012)) ? length(_15013 - (float2(_15021, _15023) * in.i_radii.x)) : abs(length(_15013) - in.i_radii.x)) - in.i_radii.y;
                                break;
                            } while(false);
                            _27166 = _27134;
                        }
                        else
                        {
                            float _27167;
                            if (in.i_flags.z == 2u)
                            {
                                float2 _15059 = _12748 - _26225.xy;
                                float2 _15062 = _26225.zw - _26225.xy;
                                _27167 = length(_15059 - (_15062 * fast::clamp(dot(_15059, _15062) / fast::max(dot(_15062, _15062), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                            }
                            else
                            {
                                float2 _14380 = _12748 - _14319;
                                float _15115 = fast::min(_14328.x, _14328.y);
                                float _15118 = fast::min((_14380.x > 0.0) ? ((_14380.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_14380.y > 0.0) ? in.i_radii.w : in.i_radii.x), _15115);
                                float _15124 = _15118 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                float _27119;
                                float _27120;
                                if (_15124 > _15115)
                                {
                                    float _15138 = in.i_shape.y * fast::clamp((_15115 - _15118) / fast::max(0.60000002384185791015625 * _15118, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                    _27120 = _15138;
                                    _27119 = _15118 * (1.0 + (0.60000002384185791015625 * _15138));
                                }
                                else
                                {
                                    _27120 = in.i_shape.y;
                                    _27119 = _15124;
                                }
                                float2 _15151 = (abs(_14380) - _14328) + float2(_27119);
                                float2 _15153 = fast::max(_15151, float2(0.0));
                                float _27121;
                                if ((_15153.x > 0.0) && (_15153.y > 0.0))
                                {
                                    float _27122;
                                    if ((_27120 > 0.001000000047497451305389404296875) && (_27119 > 9.9999997473787516355514526367188e-05))
                                    {
                                        float _15170 = 2.0 + (2.0 * _27120);
                                        float2 _15175 = _15153 / float2(fast::max(_27119, 9.9999997473787516355514526367188e-05));
                                        _27122 = pow(pow(_15175.x, _15170) + pow(_15175.y, _15170), 1.0 / _15170) * _27119;
                                    }
                                    else
                                    {
                                        _27122 = length(_15153);
                                    }
                                    _27121 = _27122;
                                }
                                else
                                {
                                    _27121 = fast::max(_15153.x, _15153.y);
                                }
                                float _15210 = (fast::min(fast::max(_15151.x, _15151.y), 0.0) + _27121) - _27119;
                                float _27168;
                                if ((in.i_flags.x & _1796) != 0u)
                                {
                                    float2 _14408 = fast::max((_26225.zw - _26225.xy) * 0.5, float2(0.001000000047497451305389404296875));
                                    float2 _14411 = _12748 - ((_26225.xy + _26225.zw) * 0.5);
                                    float _15246 = fast::min(_14408.x, _14408.y);
                                    float _15249 = fast::min((_14411.x > 0.0) ? ((_14411.y > 0.0) ? gFxData_1._data[_12207].x : gFxData_1._data[_12207].x) : ((_14411.y > 0.0) ? gFxData_1._data[_12207].x : gFxData_1._data[_12207].x), _15246);
                                    float _15255 = _15249 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                    float _27125;
                                    float _27126;
                                    if (_15255 > _15246)
                                    {
                                        float _15269 = in.i_shape.y * fast::clamp((_15246 - _15249) / fast::max(0.60000002384185791015625 * _15249, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                        _27126 = _15269;
                                        _27125 = _15249 * (1.0 + (0.60000002384185791015625 * _15269));
                                    }
                                    else
                                    {
                                        _27126 = in.i_shape.y;
                                        _27125 = _15255;
                                    }
                                    float2 _15282 = (abs(_14411) - _14408) + float2(_27125);
                                    float2 _15284 = fast::max(_15282, float2(0.0));
                                    float _27127;
                                    if ((_15284.x > 0.0) && (_15284.y > 0.0))
                                    {
                                        float _27128;
                                        if ((_27126 > 0.001000000047497451305389404296875) && (_27125 > 9.9999997473787516355514526367188e-05))
                                        {
                                            float _15301 = 2.0 + (2.0 * _27126);
                                            float2 _15306 = _15284 / float2(fast::max(_27125, 9.9999997473787516355514526367188e-05));
                                            _27128 = pow(pow(_15306.x, _15301) + pow(_15306.y, _15301), 1.0 / _15301) * _27125;
                                        }
                                        else
                                        {
                                            _27128 = length(_15284);
                                        }
                                        _27127 = _27128;
                                    }
                                    else
                                    {
                                        _27127 = fast::max(_15284.x, _15284.y);
                                    }
                                    float _15341 = (fast::min(fast::max(_15282.x, _15282.y), 0.0) + _27127) - _27125;
                                    float _15346 = fast::max(gFxData_1._data[_12207].y, 9.9999997473787516355514526367188e-05);
                                    float _15355 = fast::max(_15346 - abs(_15210 - _15341), 0.0) / _15346;
                                    _27168 = fast::min(_15210, _15341) - (((_15355 * _15355) * _15346) * 0.25);
                                }
                                else
                                {
                                    _27168 = _15210;
                                }
                                _27167 = _27168;
                            }
                            _27166 = _27167;
                        }
                        _27165 = _27166;
                    }
                    _27164 = _27165;
                }
                float2 _12753 = in.i_local + float2(0.0, 0.5);
                float2 _15410 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
                float2 _15419 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                float _27214;
                if ((in.i_flags.x & _1710) != 0u)
                {
                    float _15536 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                    float4 _15570 = gTex.sample(gLinear, mix(_26219.xy, _26219.zw, (fast::clamp(_12753, _26225.xy, _26225.zw) - _26225.xy) / fast::max(_26225.zw - _26225.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                    float _15594 = (_15570.w - 0.5) * in.i_radii.y;
                    float2 _15635 = abs(_12753 - ((_26225.xy + _26225.zw) * 0.5)) - fast::max(((_26225.zw - _26225.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _15536), float2(0.0));
                    _27214 = fast::max((-mix((fast::max(fast::min(_15570.x, _15570.y), fast::min(fast::max(_15570.x, _15570.y), _15570.z)) - 0.5) * in.i_radii.x, _15594, fast::clamp(abs(_15594) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _15536, length(fast::max(_15635, float2(0.0))) + fast::min(fast::max(_15635.x, _15635.y), 0.0)) - in.i_shape.w;
                }
                else
                {
                    float _27215;
                    if ((in.i_flags.x & _1725) != 0u)
                    {
                        float _27213;
                        do
                        {
                            bool _15697 = in.i_radii.w > 0.5;
                            float _15701 = floor((in.i_radii.z + 1.0) * 0.5);
                            float _15705 = floor((_15701 + 127.0) * 0.0078125);
                            float _15709 = floor((in.i_radii.z - 1.0) * 0.5);
                            float _15837 = floor(_15709 * 0.0625);
                            uint _15854 = (((in.i_instance + 1u) + uint(_15837)) * 24u) + uint(_15709 - (_15837 * 16.0));
                            float2 _27190;
                            if (_15697)
                            {
                                float _15864 = floor(0.0);
                                _27190 = gFxData_1._data[(((in.i_instance + 1u) + uint(_15864)) * 24u) + uint(_15864 * (-16.0))].xy;
                            }
                            else
                            {
                                _27190 = select(gFxData_1._data[_15854].zw, gFxData_1._data[_15854].xy, bool2(((in.i_radii.z - 1.0) - (_15709 * 2.0)) < 0.5));
                            }
                            float2 _27189;
                            float _27192;
                            float _27195;
                            _27195 = 1.0;
                            _27192 = 1000000015047466219876688855040.0;
                            _27189 = _27190;
                            float2 _27188;
                            float _27191;
                            float _27194;
                            for (float _27185 = 0.0; _27185 < 16.0; _27195 = _27194, _27192 = _27191, _27189 = _27188, _27185 += 1.0)
                            {
                                if (_27185 >= _15705)
                                {
                                    break;
                                }
                                _27194 = _27195;
                                _27191 = _27192;
                                _27188 = _27189;
                                float2 _30929;
                                float _30931;
                                float _30933;
                                for (float _27186 = 0.0; _27186 < 128.0; _27194 = _30933, _27191 = _30931, _27188 = _30929, _27186 += 1.0)
                                {
                                    float _15751 = (_27185 * 128.0) + _27186;
                                    if (_15751 >= _15701)
                                    {
                                        break;
                                    }
                                    float _15891 = floor(_15751 * 0.0625);
                                    uint _15908 = (((in.i_instance + 1u) + uint(_15891)) * 24u) + uint(_15751 - (_15891 * 16.0));
                                    float2 _15921 = _27188 - gFxData_1._data[_15908].xy;
                                    float2 _15924 = _12753 - gFxData_1._data[_15908].xy;
                                    float2 _15937 = _15924 - (_15921 * fast::clamp(dot(_15924, _15921) / fast::max(dot(_15921, _15921), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _15940 = dot(_15937, _15937);
                                    float _31017 = (_15940 < _27191) ? _15940 : _27191;
                                    bool _15952 = _12753.y >= gFxData_1._data[_15908].y;
                                    bool _15957 = _12753.y < _27188.y;
                                    bool _15968 = (_15921.x * _15924.y) > (_15921.y * _15924.x);
                                    float _27200;
                                    if (((_15952 && _15957) && _15968) || (((!_15952) && (!_15957)) && (!_15968)))
                                    {
                                        _27200 = -_27194;
                                    }
                                    else
                                    {
                                        _27200 = _27194;
                                    }
                                    if (((2.0 * _15751) + 1.0) < in.i_radii.z)
                                    {
                                        float2 _15997 = gFxData_1._data[_15908].xy - gFxData_1._data[_15908].zw;
                                        float2 _16000 = _12753 - gFxData_1._data[_15908].zw;
                                        float2 _16013 = _16000 - (_15997 * fast::clamp(dot(_16000, _15997) / fast::max(dot(_15997, _15997), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _16016 = dot(_16013, _16013);
                                        bool _16028 = _12753.y >= gFxData_1._data[_15908].w;
                                        bool _16033 = _12753.y < gFxData_1._data[_15908].y;
                                        bool _16044 = (_15997.x * _16000.y) > (_15997.y * _16000.x);
                                        float _27204;
                                        if (((_16028 && _16033) && _16044) || (((!_16028) && (!_16033)) && (!_16044)))
                                        {
                                            _27204 = -_27200;
                                        }
                                        else
                                        {
                                            _27204 = _27200;
                                        }
                                        _30933 = _27204;
                                        _30931 = (_16016 < _31017) ? _16016 : _31017;
                                        _30929 = gFxData_1._data[_15908].zw;
                                    }
                                    else
                                    {
                                        _30933 = _27200;
                                        _30931 = _31017;
                                        _30929 = gFxData_1._data[_15908].xy;
                                    }
                                }
                            }
                            float _15803 = sqrt(_27192);
                            if (_15697)
                            {
                                _27213 = _15803 - in.i_radii.x;
                                break;
                            }
                            _27213 = (_27195 * _15803) - in.i_radii.x;
                            break;
                        } while(false);
                        _27215 = _27213;
                    }
                    else
                    {
                        float _27216;
                        if (in.i_flags.z == 1u)
                        {
                            float2 _15444 = _12753 - _15410;
                            float _27184;
                            do
                            {
                                if (in.i_shape.w >= 6.282185077667236328125)
                                {
                                    _27184 = abs(length(_15444) - in.i_radii.x) - in.i_radii.y;
                                    break;
                                }
                                float _16087 = in.i_shape.z + (in.i_shape.w * 0.5);
                                float _16089 = cos(_16087);
                                float _16091 = sin(_16087);
                                float _16100 = dot(_15444, float2(-_16091, _16089));
                                float _16103 = dot(_15444, float2(_16089, _16091));
                                float2 _16104 = float2(_16100, _16103);
                                float _16107 = abs(_16100);
                                _16104.x = _16107;
                                float _16110 = in.i_shape.w * 0.5;
                                float _16112 = sin(_16110);
                                float _16114 = cos(_16110);
                                _27184 = (((_16114 * _16107) > (_16112 * _16103)) ? length(_16104 - (float2(_16112, _16114) * in.i_radii.x)) : abs(length(_16104) - in.i_radii.x)) - in.i_radii.y;
                                break;
                            } while(false);
                            _27216 = _27184;
                        }
                        else
                        {
                            float _27217;
                            if (in.i_flags.z == 2u)
                            {
                                float2 _16150 = _12753 - _26225.xy;
                                float2 _16153 = _26225.zw - _26225.xy;
                                _27217 = length(_16150 - (_16153 * fast::clamp(dot(_16150, _16153) / fast::max(dot(_16153, _16153), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                            }
                            else
                            {
                                float2 _15471 = _12753 - _15410;
                                float _16206 = fast::min(_15419.x, _15419.y);
                                float _16209 = fast::min((_15471.x > 0.0) ? ((_15471.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_15471.y > 0.0) ? in.i_radii.w : in.i_radii.x), _16206);
                                float _16215 = _16209 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                float _27169;
                                float _27170;
                                if (_16215 > _16206)
                                {
                                    float _16229 = in.i_shape.y * fast::clamp((_16206 - _16209) / fast::max(0.60000002384185791015625 * _16209, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                    _27170 = _16229;
                                    _27169 = _16209 * (1.0 + (0.60000002384185791015625 * _16229));
                                }
                                else
                                {
                                    _27170 = in.i_shape.y;
                                    _27169 = _16215;
                                }
                                float2 _16242 = (abs(_15471) - _15419) + float2(_27169);
                                float2 _16244 = fast::max(_16242, float2(0.0));
                                float _27171;
                                if ((_16244.x > 0.0) && (_16244.y > 0.0))
                                {
                                    float _27172;
                                    if ((_27170 > 0.001000000047497451305389404296875) && (_27169 > 9.9999997473787516355514526367188e-05))
                                    {
                                        float _16261 = 2.0 + (2.0 * _27170);
                                        float2 _16266 = _16244 / float2(fast::max(_27169, 9.9999997473787516355514526367188e-05));
                                        _27172 = pow(pow(_16266.x, _16261) + pow(_16266.y, _16261), 1.0 / _16261) * _27169;
                                    }
                                    else
                                    {
                                        _27172 = length(_16244);
                                    }
                                    _27171 = _27172;
                                }
                                else
                                {
                                    _27171 = fast::max(_16244.x, _16244.y);
                                }
                                float _16301 = (fast::min(fast::max(_16242.x, _16242.y), 0.0) + _27171) - _27169;
                                float _27218;
                                if ((in.i_flags.x & _1796) != 0u)
                                {
                                    float2 _15499 = fast::max((_26225.zw - _26225.xy) * 0.5, float2(0.001000000047497451305389404296875));
                                    float2 _15502 = _12753 - ((_26225.xy + _26225.zw) * 0.5);
                                    float _16337 = fast::min(_15499.x, _15499.y);
                                    float _16340 = fast::min((_15502.x > 0.0) ? ((_15502.y > 0.0) ? gFxData_1._data[_12207].x : gFxData_1._data[_12207].x) : ((_15502.y > 0.0) ? gFxData_1._data[_12207].x : gFxData_1._data[_12207].x), _16337);
                                    float _16346 = _16340 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                    float _27175;
                                    float _27176;
                                    if (_16346 > _16337)
                                    {
                                        float _16360 = in.i_shape.y * fast::clamp((_16337 - _16340) / fast::max(0.60000002384185791015625 * _16340, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                        _27176 = _16360;
                                        _27175 = _16340 * (1.0 + (0.60000002384185791015625 * _16360));
                                    }
                                    else
                                    {
                                        _27176 = in.i_shape.y;
                                        _27175 = _16346;
                                    }
                                    float2 _16373 = (abs(_15502) - _15499) + float2(_27175);
                                    float2 _16375 = fast::max(_16373, float2(0.0));
                                    float _27177;
                                    if ((_16375.x > 0.0) && (_16375.y > 0.0))
                                    {
                                        float _27178;
                                        if ((_27176 > 0.001000000047497451305389404296875) && (_27175 > 9.9999997473787516355514526367188e-05))
                                        {
                                            float _16392 = 2.0 + (2.0 * _27176);
                                            float2 _16397 = _16375 / float2(fast::max(_27175, 9.9999997473787516355514526367188e-05));
                                            _27178 = pow(pow(_16397.x, _16392) + pow(_16397.y, _16392), 1.0 / _16392) * _27175;
                                        }
                                        else
                                        {
                                            _27178 = length(_16375);
                                        }
                                        _27177 = _27178;
                                    }
                                    else
                                    {
                                        _27177 = fast::max(_16375.x, _16375.y);
                                    }
                                    float _16432 = (fast::min(fast::max(_16373.x, _16373.y), 0.0) + _27177) - _27175;
                                    float _16437 = fast::max(gFxData_1._data[_12207].y, 9.9999997473787516355514526367188e-05);
                                    float _16446 = fast::max(_16437 - abs(_16301 - _16432), 0.0) / _16437;
                                    _27218 = fast::min(_16301, _16432) - (((_16446 * _16446) * _16437) * 0.25);
                                }
                                else
                                {
                                    _27218 = _16301;
                                }
                                _27217 = _27218;
                            }
                            _27216 = _27217;
                        }
                        _27215 = _27216;
                    }
                    _27214 = _27215;
                }
                float2 _12757 = in.i_local - float2(0.0, 0.5);
                float2 _16501 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
                float2 _16510 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                float _27264;
                if ((in.i_flags.x & _1710) != 0u)
                {
                    float _16627 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                    float4 _16661 = gTex.sample(gLinear, mix(_26219.xy, _26219.zw, (fast::clamp(_12757, _26225.xy, _26225.zw) - _26225.xy) / fast::max(_26225.zw - _26225.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                    float _16685 = (_16661.w - 0.5) * in.i_radii.y;
                    float2 _16726 = abs(_12757 - ((_26225.xy + _26225.zw) * 0.5)) - fast::max(((_26225.zw - _26225.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _16627), float2(0.0));
                    _27264 = fast::max((-mix((fast::max(fast::min(_16661.x, _16661.y), fast::min(fast::max(_16661.x, _16661.y), _16661.z)) - 0.5) * in.i_radii.x, _16685, fast::clamp(abs(_16685) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _16627, length(fast::max(_16726, float2(0.0))) + fast::min(fast::max(_16726.x, _16726.y), 0.0)) - in.i_shape.w;
                }
                else
                {
                    float _27265;
                    if ((in.i_flags.x & _1725) != 0u)
                    {
                        float _27263;
                        do
                        {
                            bool _16788 = in.i_radii.w > 0.5;
                            float _16792 = floor((in.i_radii.z + 1.0) * 0.5);
                            float _16796 = floor((_16792 + 127.0) * 0.0078125);
                            float _16800 = floor((in.i_radii.z - 1.0) * 0.5);
                            float _16928 = floor(_16800 * 0.0625);
                            uint _16945 = (((in.i_instance + 1u) + uint(_16928)) * 24u) + uint(_16800 - (_16928 * 16.0));
                            float2 _27240;
                            if (_16788)
                            {
                                float _16955 = floor(0.0);
                                _27240 = gFxData_1._data[(((in.i_instance + 1u) + uint(_16955)) * 24u) + uint(_16955 * (-16.0))].xy;
                            }
                            else
                            {
                                _27240 = select(gFxData_1._data[_16945].zw, gFxData_1._data[_16945].xy, bool2(((in.i_radii.z - 1.0) - (_16800 * 2.0)) < 0.5));
                            }
                            float2 _27239;
                            float _27242;
                            float _27245;
                            _27245 = 1.0;
                            _27242 = 1000000015047466219876688855040.0;
                            _27239 = _27240;
                            float2 _27238;
                            float _27241;
                            float _27244;
                            for (float _27235 = 0.0; _27235 < 16.0; _27245 = _27244, _27242 = _27241, _27239 = _27238, _27235 += 1.0)
                            {
                                if (_27235 >= _16796)
                                {
                                    break;
                                }
                                _27244 = _27245;
                                _27241 = _27242;
                                _27238 = _27239;
                                float2 _30940;
                                float _30942;
                                float _30944;
                                for (float _27236 = 0.0; _27236 < 128.0; _27244 = _30944, _27241 = _30942, _27238 = _30940, _27236 += 1.0)
                                {
                                    float _16842 = (_27235 * 128.0) + _27236;
                                    if (_16842 >= _16792)
                                    {
                                        break;
                                    }
                                    float _16982 = floor(_16842 * 0.0625);
                                    uint _16999 = (((in.i_instance + 1u) + uint(_16982)) * 24u) + uint(_16842 - (_16982 * 16.0));
                                    float2 _17012 = _27238 - gFxData_1._data[_16999].xy;
                                    float2 _17015 = _12757 - gFxData_1._data[_16999].xy;
                                    float2 _17028 = _17015 - (_17012 * fast::clamp(dot(_17015, _17012) / fast::max(dot(_17012, _17012), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _17031 = dot(_17028, _17028);
                                    float _31019 = (_17031 < _27241) ? _17031 : _27241;
                                    bool _17043 = _12757.y >= gFxData_1._data[_16999].y;
                                    bool _17048 = _12757.y < _27238.y;
                                    bool _17059 = (_17012.x * _17015.y) > (_17012.y * _17015.x);
                                    float _27250;
                                    if (((_17043 && _17048) && _17059) || (((!_17043) && (!_17048)) && (!_17059)))
                                    {
                                        _27250 = -_27244;
                                    }
                                    else
                                    {
                                        _27250 = _27244;
                                    }
                                    if (((2.0 * _16842) + 1.0) < in.i_radii.z)
                                    {
                                        float2 _17088 = gFxData_1._data[_16999].xy - gFxData_1._data[_16999].zw;
                                        float2 _17091 = _12757 - gFxData_1._data[_16999].zw;
                                        float2 _17104 = _17091 - (_17088 * fast::clamp(dot(_17091, _17088) / fast::max(dot(_17088, _17088), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _17107 = dot(_17104, _17104);
                                        bool _17119 = _12757.y >= gFxData_1._data[_16999].w;
                                        bool _17124 = _12757.y < gFxData_1._data[_16999].y;
                                        bool _17135 = (_17088.x * _17091.y) > (_17088.y * _17091.x);
                                        float _27254;
                                        if (((_17119 && _17124) && _17135) || (((!_17119) && (!_17124)) && (!_17135)))
                                        {
                                            _27254 = -_27250;
                                        }
                                        else
                                        {
                                            _27254 = _27250;
                                        }
                                        _30944 = _27254;
                                        _30942 = (_17107 < _31019) ? _17107 : _31019;
                                        _30940 = gFxData_1._data[_16999].zw;
                                    }
                                    else
                                    {
                                        _30944 = _27250;
                                        _30942 = _31019;
                                        _30940 = gFxData_1._data[_16999].xy;
                                    }
                                }
                            }
                            float _16894 = sqrt(_27242);
                            if (_16788)
                            {
                                _27263 = _16894 - in.i_radii.x;
                                break;
                            }
                            _27263 = (_27245 * _16894) - in.i_radii.x;
                            break;
                        } while(false);
                        _27265 = _27263;
                    }
                    else
                    {
                        float _27266;
                        if (in.i_flags.z == 1u)
                        {
                            float2 _16535 = _12757 - _16501;
                            float _27234;
                            do
                            {
                                if (in.i_shape.w >= 6.282185077667236328125)
                                {
                                    _27234 = abs(length(_16535) - in.i_radii.x) - in.i_radii.y;
                                    break;
                                }
                                float _17178 = in.i_shape.z + (in.i_shape.w * 0.5);
                                float _17180 = cos(_17178);
                                float _17182 = sin(_17178);
                                float _17191 = dot(_16535, float2(-_17182, _17180));
                                float _17194 = dot(_16535, float2(_17180, _17182));
                                float2 _17195 = float2(_17191, _17194);
                                float _17198 = abs(_17191);
                                _17195.x = _17198;
                                float _17201 = in.i_shape.w * 0.5;
                                float _17203 = sin(_17201);
                                float _17205 = cos(_17201);
                                _27234 = (((_17205 * _17198) > (_17203 * _17194)) ? length(_17195 - (float2(_17203, _17205) * in.i_radii.x)) : abs(length(_17195) - in.i_radii.x)) - in.i_radii.y;
                                break;
                            } while(false);
                            _27266 = _27234;
                        }
                        else
                        {
                            float _27267;
                            if (in.i_flags.z == 2u)
                            {
                                float2 _17241 = _12757 - _26225.xy;
                                float2 _17244 = _26225.zw - _26225.xy;
                                _27267 = length(_17241 - (_17244 * fast::clamp(dot(_17241, _17244) / fast::max(dot(_17244, _17244), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                            }
                            else
                            {
                                float2 _16562 = _12757 - _16501;
                                float _17297 = fast::min(_16510.x, _16510.y);
                                float _17300 = fast::min((_16562.x > 0.0) ? ((_16562.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_16562.y > 0.0) ? in.i_radii.w : in.i_radii.x), _17297);
                                float _17306 = _17300 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                float _27219;
                                float _27220;
                                if (_17306 > _17297)
                                {
                                    float _17320 = in.i_shape.y * fast::clamp((_17297 - _17300) / fast::max(0.60000002384185791015625 * _17300, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                    _27220 = _17320;
                                    _27219 = _17300 * (1.0 + (0.60000002384185791015625 * _17320));
                                }
                                else
                                {
                                    _27220 = in.i_shape.y;
                                    _27219 = _17306;
                                }
                                float2 _17333 = (abs(_16562) - _16510) + float2(_27219);
                                float2 _17335 = fast::max(_17333, float2(0.0));
                                float _27221;
                                if ((_17335.x > 0.0) && (_17335.y > 0.0))
                                {
                                    float _27222;
                                    if ((_27220 > 0.001000000047497451305389404296875) && (_27219 > 9.9999997473787516355514526367188e-05))
                                    {
                                        float _17352 = 2.0 + (2.0 * _27220);
                                        float2 _17357 = _17335 / float2(fast::max(_27219, 9.9999997473787516355514526367188e-05));
                                        _27222 = pow(pow(_17357.x, _17352) + pow(_17357.y, _17352), 1.0 / _17352) * _27219;
                                    }
                                    else
                                    {
                                        _27222 = length(_17335);
                                    }
                                    _27221 = _27222;
                                }
                                else
                                {
                                    _27221 = fast::max(_17335.x, _17335.y);
                                }
                                float _17392 = (fast::min(fast::max(_17333.x, _17333.y), 0.0) + _27221) - _27219;
                                float _27268;
                                if ((in.i_flags.x & _1796) != 0u)
                                {
                                    float2 _16590 = fast::max((_26225.zw - _26225.xy) * 0.5, float2(0.001000000047497451305389404296875));
                                    float2 _16593 = _12757 - ((_26225.xy + _26225.zw) * 0.5);
                                    float _17428 = fast::min(_16590.x, _16590.y);
                                    float _17431 = fast::min((_16593.x > 0.0) ? ((_16593.y > 0.0) ? gFxData_1._data[_12207].x : gFxData_1._data[_12207].x) : ((_16593.y > 0.0) ? gFxData_1._data[_12207].x : gFxData_1._data[_12207].x), _17428);
                                    float _17437 = _17431 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                    float _27225;
                                    float _27226;
                                    if (_17437 > _17428)
                                    {
                                        float _17451 = in.i_shape.y * fast::clamp((_17428 - _17431) / fast::max(0.60000002384185791015625 * _17431, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                        _27226 = _17451;
                                        _27225 = _17431 * (1.0 + (0.60000002384185791015625 * _17451));
                                    }
                                    else
                                    {
                                        _27226 = in.i_shape.y;
                                        _27225 = _17437;
                                    }
                                    float2 _17464 = (abs(_16593) - _16590) + float2(_27225);
                                    float2 _17466 = fast::max(_17464, float2(0.0));
                                    float _27227;
                                    if ((_17466.x > 0.0) && (_17466.y > 0.0))
                                    {
                                        float _27228;
                                        if ((_27226 > 0.001000000047497451305389404296875) && (_27225 > 9.9999997473787516355514526367188e-05))
                                        {
                                            float _17483 = 2.0 + (2.0 * _27226);
                                            float2 _17488 = _17466 / float2(fast::max(_27225, 9.9999997473787516355514526367188e-05));
                                            _27228 = pow(pow(_17488.x, _17483) + pow(_17488.y, _17483), 1.0 / _17483) * _27225;
                                        }
                                        else
                                        {
                                            _27228 = length(_17466);
                                        }
                                        _27227 = _27228;
                                    }
                                    else
                                    {
                                        _27227 = fast::max(_17466.x, _17466.y);
                                    }
                                    float _17523 = (fast::min(fast::max(_17464.x, _17464.y), 0.0) + _27227) - _27225;
                                    float _17528 = fast::max(gFxData_1._data[_12207].y, 9.9999997473787516355514526367188e-05);
                                    float _17537 = fast::max(_17528 - abs(_17392 - _17523), 0.0) / _17528;
                                    _27268 = fast::min(_17392, _17523) - (((_17537 * _17537) * _17528) * 0.25);
                                }
                                else
                                {
                                    _27268 = _17392;
                                }
                                _27267 = _27268;
                            }
                            _27266 = _27267;
                        }
                        _27265 = _27266;
                    }
                    _27264 = _27265;
                }
                float2 _12763 = float2(_27114 - _27164, _27214 - _27264);
                float _12765 = length(_12763);
                _27306 = select(float2(0.0, -1.0), _12763 / float2(_12765), bool2(_12765 > 9.9999997473787516355514526367188e-06));
                break;
            } while(false);
            _27884 = fast::normalize(float3(_27306 * fast::min(_12308 / fast::max(_12314, 0.001000000047497451305389404296875), 8.0), 1.0));
            _27307 = _27306;
        }
        else
        {
            _27884 = float3(0.0, 0.0, 1.0);
            _27307 = float2(0.0, -1.0);
        }
        float2 _12340 = ((-_27307) * gFxData_1._data[_12183].y) * (1.0 - _12314);
        float2 _27308;
        if (gFxData_1._data[_12207].z > 0.0)
        {
            _27308 = (((in.i_rect.xy + in.i_rect.zw) * 0.5) - in.i_local) * (gFxData_1._data[_12207].z / (1.0 + gFxData_1._data[_12207].z));
        }
        else
        {
            _27308 = float2(0.0);
        }
        float2 _12366 = _26218 * _226.gTarget.zw;
        float2 _12373 = _226.gDisplay.zw * _226.gTarget.zw;
        float2 _12378 = (_12340 + _27308) * _12373;
        float2 _12381 = _12340 * _12373;
        float3 _27766;
        float _27784;
        float3 _27987;
        if (_226.gTime.z > 0.5)
        {
            float2 _12388 = _12366 + _12378;
            float _17562 = fast::clamp(log2(fast::max(_12280, 1.0)) - 1.0, 0.0, 5.0);
            int _17565 = int(floor(_17562));
            float _17569 = _17562 - float(_17565);
            bool _17574 = (_17569 > 0.0199999995529651641845703125) && (_17565 < 5);
            float3 _27379;
            _27379 = float3(0.0);
            float3 _17603;
            for (int _27309 = 0; _27309 < 2; _27379 = _17603, _27309++)
            {
                if ((_27309 > 0) && (!_17574))
                {
                    break;
                }
                int _17589 = _17565 + _27309;
                int _17627 = clamp(_17589, 1, 5);
                float2 _17696 = (_12388 * _226.gLevel[_17627].xy) - float2(0.5);
                float2 _17698 = floor(_17696);
                float2 _17701 = _17696 - _17698;
                float2 _17704 = _17701 * _17701;
                float2 _17707 = _17704 * _17701;
                float2 _17726 = (((_17707 * 3.0) - (_17704 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                float2 _17739 = _17707 * 0.16666667163372039794921875;
                float2 _17742 = (((((-_17707) + (_17704 * 3.0)) - (_17701 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17726;
                float2 _17746 = (((((_17707 * (-3.0)) + (_17704 * 3.0)) + (_17701 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17739;
                float2 _17758 = ((_17698 - float2(0.5)) + (_17726 / _17742)) * _226.gLevel[_17627].zw;
                float2 _17770 = ((_17698 + float2(1.5)) + (_17739 / _17746)) * _226.gLevel[_17627].zw;
                float4 _27346;
                if (_17589 <= 0)
                {
                    float2 _27345;
                    if (_226.gConv.x > 0.5)
                    {
                        float2 _25510 = _12388;
                        _25510.y = 1.0 - _12388.y;
                        _27345 = _25510;
                    }
                    else
                    {
                        _27345 = _12388;
                    }
                    _27346 = gBackdrop0.sample(gLinear, _27345, level(0.0));
                }
                else
                {
                    float4 _27347;
                    if (_17589 == 1)
                    {
                        float2 _27338;
                        if (_226.gConv.x > 0.5)
                        {
                            float2 _25515 = _17758;
                            _25515.y = 1.0 - _17758.y;
                            _27338 = _25515;
                        }
                        else
                        {
                            _27338 = _17758;
                        }
                        float2 _17816 = float2(_17770.x, _17758.y);
                        float2 _27339;
                        if (_226.gConv.x > 0.5)
                        {
                            float2 _25522 = _17816;
                            _25522.y = 1.0 - _17758.y;
                            _27339 = _25522;
                        }
                        else
                        {
                            _27339 = _17816;
                        }
                        float2 _17834 = float2(_17758.x, _17770.y);
                        float2 _27341;
                        if (_226.gConv.x > 0.5)
                        {
                            float2 _25529 = _17834;
                            _25529.y = 1.0 - _17770.y;
                            _27341 = _25529;
                        }
                        else
                        {
                            _27341 = _17834;
                        }
                        float2 _27343;
                        if (_226.gConv.x > 0.5)
                        {
                            float2 _25536 = _17770;
                            _25536.y = 1.0 - _17770.y;
                            _27343 = _25536;
                        }
                        else
                        {
                            _27343 = _17770;
                        }
                        _27347 = (((gBackdrop1.sample(gLinear, _27338, level(0.0)) * (_17742.x * _17742.y)) + (gBackdrop1.sample(gLinear, _27339, level(0.0)) * (_17746.x * _17742.y))) + (gBackdrop1.sample(gLinear, _27341, level(0.0)) * (_17742.x * _17746.y))) + (gBackdrop1.sample(gLinear, _27343, level(0.0)) * (_17746.x * _17746.y));
                    }
                    else
                    {
                        float4 _27348;
                        if (_17589 == 2)
                        {
                            float2 _27331;
                            if (_226.gConv.x > 0.5)
                            {
                                float2 _25543 = _17758;
                                _25543.y = 1.0 - _17758.y;
                                _27331 = _25543;
                            }
                            else
                            {
                                _27331 = _17758;
                            }
                            float2 _17946 = float2(_17770.x, _17758.y);
                            float2 _27332;
                            if (_226.gConv.x > 0.5)
                            {
                                float2 _25550 = _17946;
                                _25550.y = 1.0 - _17758.y;
                                _27332 = _25550;
                            }
                            else
                            {
                                _27332 = _17946;
                            }
                            float2 _17964 = float2(_17758.x, _17770.y);
                            float2 _27334;
                            if (_226.gConv.x > 0.5)
                            {
                                float2 _25557 = _17964;
                                _25557.y = 1.0 - _17770.y;
                                _27334 = _25557;
                            }
                            else
                            {
                                _27334 = _17964;
                            }
                            float2 _27336;
                            if (_226.gConv.x > 0.5)
                            {
                                float2 _25564 = _17770;
                                _25564.y = 1.0 - _17770.y;
                                _27336 = _25564;
                            }
                            else
                            {
                                _27336 = _17770;
                            }
                            _27348 = (((gBackdrop2.sample(gLinear, _27331, level(0.0)) * (_17742.x * _17742.y)) + (gBackdrop2.sample(gLinear, _27332, level(0.0)) * (_17746.x * _17742.y))) + (gBackdrop2.sample(gLinear, _27334, level(0.0)) * (_17742.x * _17746.y))) + (gBackdrop2.sample(gLinear, _27336, level(0.0)) * (_17746.x * _17746.y));
                        }
                        else
                        {
                            float4 _27349;
                            if (_17589 == 3)
                            {
                                float2 _27324;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25571 = _17758;
                                    _25571.y = 1.0 - _17758.y;
                                    _27324 = _25571;
                                }
                                else
                                {
                                    _27324 = _17758;
                                }
                                float2 _18076 = float2(_17770.x, _17758.y);
                                float2 _27325;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25578 = _18076;
                                    _25578.y = 1.0 - _17758.y;
                                    _27325 = _25578;
                                }
                                else
                                {
                                    _27325 = _18076;
                                }
                                float2 _18094 = float2(_17758.x, _17770.y);
                                float2 _27327;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25585 = _18094;
                                    _25585.y = 1.0 - _17770.y;
                                    _27327 = _25585;
                                }
                                else
                                {
                                    _27327 = _18094;
                                }
                                float2 _27329;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25592 = _17770;
                                    _25592.y = 1.0 - _17770.y;
                                    _27329 = _25592;
                                }
                                else
                                {
                                    _27329 = _17770;
                                }
                                _27349 = (((gBackdrop3.sample(gLinear, _27324, level(0.0)) * (_17742.x * _17742.y)) + (gBackdrop3.sample(gLinear, _27325, level(0.0)) * (_17746.x * _17742.y))) + (gBackdrop3.sample(gLinear, _27327, level(0.0)) * (_17742.x * _17746.y))) + (gBackdrop3.sample(gLinear, _27329, level(0.0)) * (_17746.x * _17746.y));
                            }
                            else
                            {
                                float4 _27350;
                                if (_17589 == 4)
                                {
                                    float2 _27317;
                                    if (_226.gConv.x > 0.5)
                                    {
                                        float2 _25599 = _17758;
                                        _25599.y = 1.0 - _17758.y;
                                        _27317 = _25599;
                                    }
                                    else
                                    {
                                        _27317 = _17758;
                                    }
                                    float2 _18206 = float2(_17770.x, _17758.y);
                                    float2 _27318;
                                    if (_226.gConv.x > 0.5)
                                    {
                                        float2 _25606 = _18206;
                                        _25606.y = 1.0 - _17758.y;
                                        _27318 = _25606;
                                    }
                                    else
                                    {
                                        _27318 = _18206;
                                    }
                                    float2 _18224 = float2(_17758.x, _17770.y);
                                    float2 _27320;
                                    if (_226.gConv.x > 0.5)
                                    {
                                        float2 _25613 = _18224;
                                        _25613.y = 1.0 - _17770.y;
                                        _27320 = _25613;
                                    }
                                    else
                                    {
                                        _27320 = _18224;
                                    }
                                    float2 _27322;
                                    if (_226.gConv.x > 0.5)
                                    {
                                        float2 _25620 = _17770;
                                        _25620.y = 1.0 - _17770.y;
                                        _27322 = _25620;
                                    }
                                    else
                                    {
                                        _27322 = _17770;
                                    }
                                    _27350 = (((gBackdrop4.sample(gLinear, _27317, level(0.0)) * (_17742.x * _17742.y)) + (gBackdrop4.sample(gLinear, _27318, level(0.0)) * (_17746.x * _17742.y))) + (gBackdrop4.sample(gLinear, _27320, level(0.0)) * (_17742.x * _17746.y))) + (gBackdrop4.sample(gLinear, _27322, level(0.0)) * (_17746.x * _17746.y));
                                }
                                else
                                {
                                    float2 _27310;
                                    if (_226.gConv.x > 0.5)
                                    {
                                        float2 _25627 = _17758;
                                        _25627.y = 1.0 - _17758.y;
                                        _27310 = _25627;
                                    }
                                    else
                                    {
                                        _27310 = _17758;
                                    }
                                    float2 _18336 = float2(_17770.x, _17758.y);
                                    float2 _27311;
                                    if (_226.gConv.x > 0.5)
                                    {
                                        float2 _25634 = _18336;
                                        _25634.y = 1.0 - _17758.y;
                                        _27311 = _25634;
                                    }
                                    else
                                    {
                                        _27311 = _18336;
                                    }
                                    float2 _18354 = float2(_17758.x, _17770.y);
                                    float2 _27313;
                                    if (_226.gConv.x > 0.5)
                                    {
                                        float2 _25641 = _18354;
                                        _25641.y = 1.0 - _17770.y;
                                        _27313 = _25641;
                                    }
                                    else
                                    {
                                        _27313 = _18354;
                                    }
                                    float2 _27315;
                                    if (_226.gConv.x > 0.5)
                                    {
                                        float2 _25648 = _17770;
                                        _25648.y = 1.0 - _17770.y;
                                        _27315 = _25648;
                                    }
                                    else
                                    {
                                        _27315 = _17770;
                                    }
                                    _27350 = (((gBackdrop5.sample(gLinear, _27310, level(0.0)) * (_17742.x * _17742.y)) + (gBackdrop5.sample(gLinear, _27311, level(0.0)) * (_17746.x * _17742.y))) + (gBackdrop5.sample(gLinear, _27313, level(0.0)) * (_17742.x * _17746.y))) + (gBackdrop5.sample(gLinear, _27315, level(0.0)) * (_17746.x * _17746.y));
                                }
                                _27349 = _27350;
                            }
                            _27348 = _27349;
                        }
                        _27347 = _27348;
                    }
                    _27346 = _27347;
                }
                _17603 = _27379 + (_27346.xyz * (_17574 ? ((_27309 == 0) ? (1.0 - _17569) : _17569) : 1.0));
            }
            float3 _27769;
            if (((gFxData_1._data[_12183].w > 0.001000000047497451305389404296875) && (_12308 > 0.0)) && ((((gFxData_1._data[_12183].y * 0.300000011920928955078125) * gFxData_1._data[_12183].w) * _226.gDisplay.z) > (_12280 * 0.3499999940395355224609375)))
            {
                float _12408 = 0.300000011920928955078125 * gFxData_1._data[_12183].w;
                float2 _12415 = (_12366 + _12378) - (_12381 * _12408);
                float _18450 = fast::clamp(log2(fast::max(_12280, 1.0)) - 1.0, 0.0, 5.0);
                int _18453 = int(floor(_18450));
                float _18457 = _18450 - float(_18453);
                bool _18462 = (_18457 > 0.0199999995529651641845703125) && (_18453 < 5);
                float3 _27475;
                _27475 = float3(0.0);
                float3 _18491;
                for (int _27405 = 0; _27405 < 2; _27475 = _18491, _27405++)
                {
                    if ((_27405 > 0) && (!_18462))
                    {
                        break;
                    }
                    int _18477 = _18453 + _27405;
                    int _18515 = clamp(_18477, 1, 5);
                    float2 _18584 = (_12415 * _226.gLevel[_18515].xy) - float2(0.5);
                    float2 _18586 = floor(_18584);
                    float2 _18589 = _18584 - _18586;
                    float2 _18592 = _18589 * _18589;
                    float2 _18595 = _18592 * _18589;
                    float2 _18614 = (((_18595 * 3.0) - (_18592 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                    float2 _18627 = _18595 * 0.16666667163372039794921875;
                    float2 _18630 = (((((-_18595) + (_18592 * 3.0)) - (_18589 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _18614;
                    float2 _18634 = (((((_18595 * (-3.0)) + (_18592 * 3.0)) + (_18589 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _18627;
                    float2 _18646 = ((_18586 - float2(0.5)) + (_18614 / _18630)) * _226.gLevel[_18515].zw;
                    float2 _18658 = ((_18586 + float2(1.5)) + (_18627 / _18634)) * _226.gLevel[_18515].zw;
                    float4 _27442;
                    if (_18477 <= 0)
                    {
                        float2 _27441;
                        if (_226.gConv.x > 0.5)
                        {
                            float2 _25653 = _12415;
                            _25653.y = 1.0 - _12415.y;
                            _27441 = _25653;
                        }
                        else
                        {
                            _27441 = _12415;
                        }
                        _27442 = gBackdrop0.sample(gLinear, _27441, level(0.0));
                    }
                    else
                    {
                        float4 _27443;
                        if (_18477 == 1)
                        {
                            float2 _27434;
                            if (_226.gConv.x > 0.5)
                            {
                                float2 _25658 = _18646;
                                _25658.y = 1.0 - _18646.y;
                                _27434 = _25658;
                            }
                            else
                            {
                                _27434 = _18646;
                            }
                            float _18703 = _18646.y;
                            float2 _18704 = float2(_18658.x, _18703);
                            float2 _27435;
                            if (_226.gConv.x > 0.5)
                            {
                                float2 _25665 = _18704;
                                _25665.y = 1.0 - _18703;
                                _27435 = _25665;
                            }
                            else
                            {
                                _27435 = _18704;
                            }
                            float _18721 = _18658.y;
                            float2 _18722 = float2(_18646.x, _18721);
                            float2 _27437;
                            if (_226.gConv.x > 0.5)
                            {
                                float2 _25672 = _18722;
                                _25672.y = 1.0 - _18721;
                                _27437 = _25672;
                            }
                            else
                            {
                                _27437 = _18722;
                            }
                            float2 _27439;
                            if (_226.gConv.x > 0.5)
                            {
                                float2 _25679 = _18658;
                                _25679.y = 1.0 - _18658.y;
                                _27439 = _25679;
                            }
                            else
                            {
                                _27439 = _18658;
                            }
                            _27443 = (((gBackdrop1.sample(gLinear, _27434, level(0.0)) * (_18630.x * _18630.y)) + (gBackdrop1.sample(gLinear, _27435, level(0.0)) * (_18634.x * _18630.y))) + (gBackdrop1.sample(gLinear, _27437, level(0.0)) * (_18630.x * _18634.y))) + (gBackdrop1.sample(gLinear, _27439, level(0.0)) * (_18634.x * _18634.y));
                        }
                        else
                        {
                            float4 _27444;
                            if (_18477 == 2)
                            {
                                float2 _27427;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25686 = _18646;
                                    _25686.y = 1.0 - _18646.y;
                                    _27427 = _25686;
                                }
                                else
                                {
                                    _27427 = _18646;
                                }
                                float _18833 = _18646.y;
                                float2 _18834 = float2(_18658.x, _18833);
                                float2 _27428;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25693 = _18834;
                                    _25693.y = 1.0 - _18833;
                                    _27428 = _25693;
                                }
                                else
                                {
                                    _27428 = _18834;
                                }
                                float _18851 = _18658.y;
                                float2 _18852 = float2(_18646.x, _18851);
                                float2 _27430;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25700 = _18852;
                                    _25700.y = 1.0 - _18851;
                                    _27430 = _25700;
                                }
                                else
                                {
                                    _27430 = _18852;
                                }
                                float2 _27432;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25707 = _18658;
                                    _25707.y = 1.0 - _18658.y;
                                    _27432 = _25707;
                                }
                                else
                                {
                                    _27432 = _18658;
                                }
                                _27444 = (((gBackdrop2.sample(gLinear, _27427, level(0.0)) * (_18630.x * _18630.y)) + (gBackdrop2.sample(gLinear, _27428, level(0.0)) * (_18634.x * _18630.y))) + (gBackdrop2.sample(gLinear, _27430, level(0.0)) * (_18630.x * _18634.y))) + (gBackdrop2.sample(gLinear, _27432, level(0.0)) * (_18634.x * _18634.y));
                            }
                            else
                            {
                                float4 _27445;
                                if (_18477 == 3)
                                {
                                    float2 _27420;
                                    if (_226.gConv.x > 0.5)
                                    {
                                        float2 _25714 = _18646;
                                        _25714.y = 1.0 - _18646.y;
                                        _27420 = _25714;
                                    }
                                    else
                                    {
                                        _27420 = _18646;
                                    }
                                    float _18963 = _18646.y;
                                    float2 _18964 = float2(_18658.x, _18963);
                                    float2 _27421;
                                    if (_226.gConv.x > 0.5)
                                    {
                                        float2 _25721 = _18964;
                                        _25721.y = 1.0 - _18963;
                                        _27421 = _25721;
                                    }
                                    else
                                    {
                                        _27421 = _18964;
                                    }
                                    float _18981 = _18658.y;
                                    float2 _18982 = float2(_18646.x, _18981);
                                    float2 _27423;
                                    if (_226.gConv.x > 0.5)
                                    {
                                        float2 _25728 = _18982;
                                        _25728.y = 1.0 - _18981;
                                        _27423 = _25728;
                                    }
                                    else
                                    {
                                        _27423 = _18982;
                                    }
                                    float2 _27425;
                                    if (_226.gConv.x > 0.5)
                                    {
                                        float2 _25735 = _18658;
                                        _25735.y = 1.0 - _18658.y;
                                        _27425 = _25735;
                                    }
                                    else
                                    {
                                        _27425 = _18658;
                                    }
                                    _27445 = (((gBackdrop3.sample(gLinear, _27420, level(0.0)) * (_18630.x * _18630.y)) + (gBackdrop3.sample(gLinear, _27421, level(0.0)) * (_18634.x * _18630.y))) + (gBackdrop3.sample(gLinear, _27423, level(0.0)) * (_18630.x * _18634.y))) + (gBackdrop3.sample(gLinear, _27425, level(0.0)) * (_18634.x * _18634.y));
                                }
                                else
                                {
                                    float4 _27446;
                                    if (_18477 == 4)
                                    {
                                        float2 _27413;
                                        if (_226.gConv.x > 0.5)
                                        {
                                            float2 _25742 = _18646;
                                            _25742.y = 1.0 - _18646.y;
                                            _27413 = _25742;
                                        }
                                        else
                                        {
                                            _27413 = _18646;
                                        }
                                        float _19093 = _18646.y;
                                        float2 _19094 = float2(_18658.x, _19093);
                                        float2 _27414;
                                        if (_226.gConv.x > 0.5)
                                        {
                                            float2 _25749 = _19094;
                                            _25749.y = 1.0 - _19093;
                                            _27414 = _25749;
                                        }
                                        else
                                        {
                                            _27414 = _19094;
                                        }
                                        float _19111 = _18658.y;
                                        float2 _19112 = float2(_18646.x, _19111);
                                        float2 _27416;
                                        if (_226.gConv.x > 0.5)
                                        {
                                            float2 _25756 = _19112;
                                            _25756.y = 1.0 - _19111;
                                            _27416 = _25756;
                                        }
                                        else
                                        {
                                            _27416 = _19112;
                                        }
                                        float2 _27418;
                                        if (_226.gConv.x > 0.5)
                                        {
                                            float2 _25763 = _18658;
                                            _25763.y = 1.0 - _18658.y;
                                            _27418 = _25763;
                                        }
                                        else
                                        {
                                            _27418 = _18658;
                                        }
                                        _27446 = (((gBackdrop4.sample(gLinear, _27413, level(0.0)) * (_18630.x * _18630.y)) + (gBackdrop4.sample(gLinear, _27414, level(0.0)) * (_18634.x * _18630.y))) + (gBackdrop4.sample(gLinear, _27416, level(0.0)) * (_18630.x * _18634.y))) + (gBackdrop4.sample(gLinear, _27418, level(0.0)) * (_18634.x * _18634.y));
                                    }
                                    else
                                    {
                                        float2 _27406;
                                        if (_226.gConv.x > 0.5)
                                        {
                                            float2 _25770 = _18646;
                                            _25770.y = 1.0 - _18646.y;
                                            _27406 = _25770;
                                        }
                                        else
                                        {
                                            _27406 = _18646;
                                        }
                                        float _19223 = _18646.y;
                                        float2 _19224 = float2(_18658.x, _19223);
                                        float2 _27407;
                                        if (_226.gConv.x > 0.5)
                                        {
                                            float2 _25777 = _19224;
                                            _25777.y = 1.0 - _19223;
                                            _27407 = _25777;
                                        }
                                        else
                                        {
                                            _27407 = _19224;
                                        }
                                        float _19241 = _18658.y;
                                        float2 _19242 = float2(_18646.x, _19241);
                                        float2 _27409;
                                        if (_226.gConv.x > 0.5)
                                        {
                                            float2 _25784 = _19242;
                                            _25784.y = 1.0 - _19241;
                                            _27409 = _25784;
                                        }
                                        else
                                        {
                                            _27409 = _19242;
                                        }
                                        float2 _27411;
                                        if (_226.gConv.x > 0.5)
                                        {
                                            float2 _25791 = _18658;
                                            _25791.y = 1.0 - _18658.y;
                                            _27411 = _25791;
                                        }
                                        else
                                        {
                                            _27411 = _18658;
                                        }
                                        _27446 = (((gBackdrop5.sample(gLinear, _27406, level(0.0)) * (_18630.x * _18630.y)) + (gBackdrop5.sample(gLinear, _27407, level(0.0)) * (_18634.x * _18630.y))) + (gBackdrop5.sample(gLinear, _27409, level(0.0)) * (_18630.x * _18634.y))) + (gBackdrop5.sample(gLinear, _27411, level(0.0)) * (_18634.x * _18634.y));
                                    }
                                    _27445 = _27446;
                                }
                                _27444 = _27445;
                            }
                            _27443 = _27444;
                        }
                        _27442 = _27443;
                    }
                    _18491 = _27475 + (_27442.xyz * (_18462 ? ((_27405 == 0) ? (1.0 - _18457) : _18457) : 1.0));
                }
                float3 _25795 = _27379;
                _25795.x = _27475.x;
                float2 _12426 = (_12366 + _12378) + (_12381 * _12408);
                float _19338 = fast::clamp(log2(fast::max(_12280, 1.0)) - 1.0, 0.0, 5.0);
                int _19341 = int(floor(_19338));
                float _19345 = _19338 - float(_19341);
                bool _19350 = (_19345 > 0.0199999995529651641845703125) && (_19341 < 5);
                float3 _27599;
                _27599 = float3(0.0);
                float3 _19379;
                for (int _27529 = 0; _27529 < 2; _27599 = _19379, _27529++)
                {
                    if ((_27529 > 0) && (!_19350))
                    {
                        break;
                    }
                    int _19365 = _19341 + _27529;
                    int _19403 = clamp(_19365, 1, 5);
                    float2 _19472 = (_12426 * _226.gLevel[_19403].xy) - float2(0.5);
                    float2 _19474 = floor(_19472);
                    float2 _19477 = _19472 - _19474;
                    float2 _19480 = _19477 * _19477;
                    float2 _19483 = _19480 * _19477;
                    float2 _19502 = (((_19483 * 3.0) - (_19480 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                    float2 _19515 = _19483 * 0.16666667163372039794921875;
                    float2 _19518 = (((((-_19483) + (_19480 * 3.0)) - (_19477 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _19502;
                    float2 _19522 = (((((_19483 * (-3.0)) + (_19480 * 3.0)) + (_19477 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _19515;
                    float2 _19534 = ((_19474 - float2(0.5)) + (_19502 / _19518)) * _226.gLevel[_19403].zw;
                    float2 _19546 = ((_19474 + float2(1.5)) + (_19515 / _19522)) * _226.gLevel[_19403].zw;
                    float4 _27566;
                    if (_19365 <= 0)
                    {
                        float2 _27565;
                        if (_226.gConv.x > 0.5)
                        {
                            float2 _25798 = _12426;
                            _25798.y = 1.0 - _12426.y;
                            _27565 = _25798;
                        }
                        else
                        {
                            _27565 = _12426;
                        }
                        _27566 = gBackdrop0.sample(gLinear, _27565, level(0.0));
                    }
                    else
                    {
                        float4 _27567;
                        if (_19365 == 1)
                        {
                            float2 _27558;
                            if (_226.gConv.x > 0.5)
                            {
                                float2 _25803 = _19534;
                                _25803.y = 1.0 - _19534.y;
                                _27558 = _25803;
                            }
                            else
                            {
                                _27558 = _19534;
                            }
                            float _19591 = _19534.y;
                            float2 _19592 = float2(_19546.x, _19591);
                            float2 _27559;
                            if (_226.gConv.x > 0.5)
                            {
                                float2 _25810 = _19592;
                                _25810.y = 1.0 - _19591;
                                _27559 = _25810;
                            }
                            else
                            {
                                _27559 = _19592;
                            }
                            float _19609 = _19546.y;
                            float2 _19610 = float2(_19534.x, _19609);
                            float2 _27561;
                            if (_226.gConv.x > 0.5)
                            {
                                float2 _25817 = _19610;
                                _25817.y = 1.0 - _19609;
                                _27561 = _25817;
                            }
                            else
                            {
                                _27561 = _19610;
                            }
                            float2 _27563;
                            if (_226.gConv.x > 0.5)
                            {
                                float2 _25824 = _19546;
                                _25824.y = 1.0 - _19546.y;
                                _27563 = _25824;
                            }
                            else
                            {
                                _27563 = _19546;
                            }
                            _27567 = (((gBackdrop1.sample(gLinear, _27558, level(0.0)) * (_19518.x * _19518.y)) + (gBackdrop1.sample(gLinear, _27559, level(0.0)) * (_19522.x * _19518.y))) + (gBackdrop1.sample(gLinear, _27561, level(0.0)) * (_19518.x * _19522.y))) + (gBackdrop1.sample(gLinear, _27563, level(0.0)) * (_19522.x * _19522.y));
                        }
                        else
                        {
                            float4 _27568;
                            if (_19365 == 2)
                            {
                                float2 _27551;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25831 = _19534;
                                    _25831.y = 1.0 - _19534.y;
                                    _27551 = _25831;
                                }
                                else
                                {
                                    _27551 = _19534;
                                }
                                float _19721 = _19534.y;
                                float2 _19722 = float2(_19546.x, _19721);
                                float2 _27552;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25838 = _19722;
                                    _25838.y = 1.0 - _19721;
                                    _27552 = _25838;
                                }
                                else
                                {
                                    _27552 = _19722;
                                }
                                float _19739 = _19546.y;
                                float2 _19740 = float2(_19534.x, _19739);
                                float2 _27554;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25845 = _19740;
                                    _25845.y = 1.0 - _19739;
                                    _27554 = _25845;
                                }
                                else
                                {
                                    _27554 = _19740;
                                }
                                float2 _27556;
                                if (_226.gConv.x > 0.5)
                                {
                                    float2 _25852 = _19546;
                                    _25852.y = 1.0 - _19546.y;
                                    _27556 = _25852;
                                }
                                else
                                {
                                    _27556 = _19546;
                                }
                                _27568 = (((gBackdrop2.sample(gLinear, _27551, level(0.0)) * (_19518.x * _19518.y)) + (gBackdrop2.sample(gLinear, _27552, level(0.0)) * (_19522.x * _19518.y))) + (gBackdrop2.sample(gLinear, _27554, level(0.0)) * (_19518.x * _19522.y))) + (gBackdrop2.sample(gLinear, _27556, level(0.0)) * (_19522.x * _19522.y));
                            }
                            else
                            {
                                float4 _27569;
                                if (_19365 == 3)
                                {
                                    float2 _27544;
                                    if (_226.gConv.x > 0.5)
                                    {
                                        float2 _25859 = _19534;
                                        _25859.y = 1.0 - _19534.y;
                                        _27544 = _25859;
                                    }
                                    else
                                    {
                                        _27544 = _19534;
                                    }
                                    float _19851 = _19534.y;
                                    float2 _19852 = float2(_19546.x, _19851);
                                    float2 _27545;
                                    if (_226.gConv.x > 0.5)
                                    {
                                        float2 _25866 = _19852;
                                        _25866.y = 1.0 - _19851;
                                        _27545 = _25866;
                                    }
                                    else
                                    {
                                        _27545 = _19852;
                                    }
                                    float _19869 = _19546.y;
                                    float2 _19870 = float2(_19534.x, _19869);
                                    float2 _27547;
                                    if (_226.gConv.x > 0.5)
                                    {
                                        float2 _25873 = _19870;
                                        _25873.y = 1.0 - _19869;
                                        _27547 = _25873;
                                    }
                                    else
                                    {
                                        _27547 = _19870;
                                    }
                                    float2 _27549;
                                    if (_226.gConv.x > 0.5)
                                    {
                                        float2 _25880 = _19546;
                                        _25880.y = 1.0 - _19546.y;
                                        _27549 = _25880;
                                    }
                                    else
                                    {
                                        _27549 = _19546;
                                    }
                                    _27569 = (((gBackdrop3.sample(gLinear, _27544, level(0.0)) * (_19518.x * _19518.y)) + (gBackdrop3.sample(gLinear, _27545, level(0.0)) * (_19522.x * _19518.y))) + (gBackdrop3.sample(gLinear, _27547, level(0.0)) * (_19518.x * _19522.y))) + (gBackdrop3.sample(gLinear, _27549, level(0.0)) * (_19522.x * _19522.y));
                                }
                                else
                                {
                                    float4 _27570;
                                    if (_19365 == 4)
                                    {
                                        float2 _27537;
                                        if (_226.gConv.x > 0.5)
                                        {
                                            float2 _25887 = _19534;
                                            _25887.y = 1.0 - _19534.y;
                                            _27537 = _25887;
                                        }
                                        else
                                        {
                                            _27537 = _19534;
                                        }
                                        float _19981 = _19534.y;
                                        float2 _19982 = float2(_19546.x, _19981);
                                        float2 _27538;
                                        if (_226.gConv.x > 0.5)
                                        {
                                            float2 _25894 = _19982;
                                            _25894.y = 1.0 - _19981;
                                            _27538 = _25894;
                                        }
                                        else
                                        {
                                            _27538 = _19982;
                                        }
                                        float _19999 = _19546.y;
                                        float2 _20000 = float2(_19534.x, _19999);
                                        float2 _27540;
                                        if (_226.gConv.x > 0.5)
                                        {
                                            float2 _25901 = _20000;
                                            _25901.y = 1.0 - _19999;
                                            _27540 = _25901;
                                        }
                                        else
                                        {
                                            _27540 = _20000;
                                        }
                                        float2 _27542;
                                        if (_226.gConv.x > 0.5)
                                        {
                                            float2 _25908 = _19546;
                                            _25908.y = 1.0 - _19546.y;
                                            _27542 = _25908;
                                        }
                                        else
                                        {
                                            _27542 = _19546;
                                        }
                                        _27570 = (((gBackdrop4.sample(gLinear, _27537, level(0.0)) * (_19518.x * _19518.y)) + (gBackdrop4.sample(gLinear, _27538, level(0.0)) * (_19522.x * _19518.y))) + (gBackdrop4.sample(gLinear, _27540, level(0.0)) * (_19518.x * _19522.y))) + (gBackdrop4.sample(gLinear, _27542, level(0.0)) * (_19522.x * _19522.y));
                                    }
                                    else
                                    {
                                        float2 _27530;
                                        if (_226.gConv.x > 0.5)
                                        {
                                            float2 _25915 = _19534;
                                            _25915.y = 1.0 - _19534.y;
                                            _27530 = _25915;
                                        }
                                        else
                                        {
                                            _27530 = _19534;
                                        }
                                        float _20111 = _19534.y;
                                        float2 _20112 = float2(_19546.x, _20111);
                                        float2 _27531;
                                        if (_226.gConv.x > 0.5)
                                        {
                                            float2 _25922 = _20112;
                                            _25922.y = 1.0 - _20111;
                                            _27531 = _25922;
                                        }
                                        else
                                        {
                                            _27531 = _20112;
                                        }
                                        float _20129 = _19546.y;
                                        float2 _20130 = float2(_19534.x, _20129);
                                        float2 _27533;
                                        if (_226.gConv.x > 0.5)
                                        {
                                            float2 _25929 = _20130;
                                            _25929.y = 1.0 - _20129;
                                            _27533 = _25929;
                                        }
                                        else
                                        {
                                            _27533 = _20130;
                                        }
                                        float2 _27535;
                                        if (_226.gConv.x > 0.5)
                                        {
                                            float2 _25936 = _19546;
                                            _25936.y = 1.0 - _19546.y;
                                            _27535 = _25936;
                                        }
                                        else
                                        {
                                            _27535 = _19546;
                                        }
                                        _27570 = (((gBackdrop5.sample(gLinear, _27530, level(0.0)) * (_19518.x * _19518.y)) + (gBackdrop5.sample(gLinear, _27531, level(0.0)) * (_19522.x * _19518.y))) + (gBackdrop5.sample(gLinear, _27533, level(0.0)) * (_19518.x * _19522.y))) + (gBackdrop5.sample(gLinear, _27535, level(0.0)) * (_19522.x * _19522.y));
                                    }
                                    _27569 = _27570;
                                }
                                _27568 = _27569;
                            }
                            _27567 = _27568;
                        }
                        _27566 = _27567;
                    }
                    _19379 = _27599 + (_27566.xyz * (_19350 ? ((_27529 == 0) ? (1.0 - _19345) : _19345) : 1.0));
                }
                _25795.z = _27599.z;
                _27769 = _25795;
            }
            else
            {
                _27769 = _27379;
            }
            float3 _27989;
            if (_12308 > 0.0)
            {
                float2 _12449 = _12366 + (((_27307 * fast::min(_12299 * 0.5, 16.0)) * _226.gDisplay.zw) * _226.gTarget.zw);
                float _20226 = fast::clamp(log2(fast::max(16.0 * _226.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _20229 = int(floor(_20226));
                float _20233 = _20226 - float(_20229);
                bool _20238 = (_20233 > 0.0199999995529651641845703125) && (_20229 < 5);
                float3 _27754;
                _27754 = float3(0.0);
                float3 _20267;
                for (int _27739 = 0; _27739 < 2; _27754 = _20267, _27739++)
                {
                    if ((_27739 > 0) && (!_20238))
                    {
                        break;
                    }
                    int _20253 = _20229 + _27739;
                    float2 _27740;
                    if (_226.gConv.x > 0.5)
                    {
                        float2 _25943 = _12449;
                        _25943.y = 1.0 - _12449.y;
                        _27740 = _25943;
                    }
                    else
                    {
                        _27740 = _12449;
                    }
                    float4 _27741;
                    if (_20253 <= 0)
                    {
                        _27741 = gBackdrop0.sample(gLinear, _27740, level(0.0));
                    }
                    else
                    {
                        float4 _27742;
                        if (_20253 == 1)
                        {
                            _27742 = gBackdrop1.sample(gLinear, _27740, level(0.0));
                        }
                        else
                        {
                            float4 _27743;
                            if (_20253 == 2)
                            {
                                _27743 = gBackdrop2.sample(gLinear, _27740, level(0.0));
                            }
                            else
                            {
                                float4 _27744;
                                if (_20253 == 3)
                                {
                                    _27744 = gBackdrop3.sample(gLinear, _27740, level(0.0));
                                }
                                else
                                {
                                    float4 _27745;
                                    if (_20253 == 4)
                                    {
                                        _27745 = gBackdrop4.sample(gLinear, _27740, level(0.0));
                                    }
                                    else
                                    {
                                        _27745 = gBackdrop5.sample(gLinear, _27740, level(0.0));
                                    }
                                    _27744 = _27745;
                                }
                                _27743 = _27744;
                            }
                            _27742 = _27743;
                        }
                        _27741 = _27742;
                    }
                    _20267 = _27754 + (_27741.xyz * (_20238 ? ((_27739 == 0) ? (1.0 - _20233) : _20233) : 1.0));
                }
                _27989 = _27754;
            }
            else
            {
                _27989 = float3(0.5);
            }
            float _27785;
            if (in.i_shape.x > 0.001000000047497451305389404296875)
            {
                int _20356 = clamp(int(rint(log2(36.0 * _226.gDisplay.z) - 1.0)), 1, 4);
                float2 _27760;
                if (_226.gConv.x > 0.5)
                {
                    float2 _25947 = _12366;
                    _25947.y = 1.0 - _12366.y;
                    _27760 = _25947;
                }
                else
                {
                    _27760 = _12366;
                }
                float4 _27761;
                if (_20356 <= 0)
                {
                    _27761 = gBackdrop0.sample(gLinear, _27760, level(0.0));
                }
                else
                {
                    float4 _27762;
                    if (_20356 == 1)
                    {
                        _27762 = gBackdrop1.sample(gLinear, _27760, level(0.0));
                    }
                    else
                    {
                        float4 _27763;
                        if (_20356 == 2)
                        {
                            _27763 = gBackdrop2.sample(gLinear, _27760, level(0.0));
                        }
                        else
                        {
                            float4 _27764;
                            if (_20356 == 3)
                            {
                                _27764 = gBackdrop3.sample(gLinear, _27760, level(0.0));
                            }
                            else
                            {
                                float4 _27765;
                                if (_20356 == 4)
                                {
                                    _27765 = gBackdrop4.sample(gLinear, _27760, level(0.0));
                                }
                                else
                                {
                                    _27765 = gBackdrop5.sample(gLinear, _27760, level(0.0));
                                }
                                _27764 = _27765;
                            }
                            _27763 = _27764;
                        }
                        _27762 = _27763;
                    }
                    _27761 = _27762;
                }
                _27785 = dot(_27761.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _27785 = 0.5;
            }
            _27987 = _27989;
            _27784 = _27785;
            _27766 = _27769;
        }
        else
        {
            _27987 = float3(0.5);
            _27784 = 0.5;
            _27766 = float3(0.5);
        }
        float3 _12477 = mix(float3(dot(_27766, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _27766, float3(gFxData_1._data[_12199].x)) + float3(gFxData_1._data[_12199].y);
        float3 _27880;
        if (in.i_shape.x > 0.001000000047497451305389404296875)
        {
            float _12487 = fast::clamp(fast::max(_27784 + gFxData_1._data[_12199].y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _12495 = mix(_12487, dot(gFxData_1._data[_12191].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), in.i_shape.x);
            _27880 = select(mix(_12477, float3(1.0), float3((_12495 - _12487) / fast::max(1.0 - _12487, 0.001000000047497451305389404296875))), _12477 * (_12495 / _12487), bool3(_12495 < _12487));
        }
        else
        {
            _27880 = _12477;
        }
        float2 _12533 = fast::clamp((in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875)), float2(0.0), float2(1.0));
        float2 _20468 = select(_268.gDrawInfo.yz, _268.gDrawInfo.yz - in.i_local, bool2(_268.gDrawInfo.w > 1.0));
        float2 _12586 = select(float2(cos(gFxData_1._data[_12199].w), sin(gFxData_1._data[_12199].w)), _20468 / float2(fast::max(length(_20468), 9.9999997473787516355514526367188e-05)), bool2(_268.gDrawInfo.w > 0.5));
        float _12589 = dot(_27307, _12586);
        float _12600 = -_26273;
        float _12607 = _4963 * ((0.5 * abs(_27307.x)) + 0.0199999995529651641845703125);
        float _12614 = _4963 * ((0.5 * abs(_27307.y)) + 0.0199999995529651641845703125);
        float _28099;
        if ((((_12600 + _12607) + _12614) > 0.0) && (((_12600 - _12607) - _12614) < (0.5 * _12299)))
        {
            float _20487 = fast::max((_12600 + _12607) + _12614, 0.0) / _12299;
            float _20491 = fast::clamp(sqrt(2.0 * _20487), 0.12399999797344207763671875, 1.0);
            float _20493 = 1.0 - _20491;
            float _20496 = _20493 * _20493;
            float _20550 = fast::max((_12600 + _12607) - _12614, 0.0) / _12299;
            float _20554 = fast::clamp(sqrt(2.0 * _20550), 0.12399999797344207763671875, 1.0);
            float _20556 = 1.0 - _20554;
            float _20559 = _20556 * _20556;
            float _20613 = fast::max((_12600 - _12607) + _12614, 0.0) / _12299;
            float _20617 = fast::clamp(sqrt(2.0 * _20613), 0.12399999797344207763671875, 1.0);
            float _20619 = 1.0 - _20617;
            float _20622 = _20619 * _20619;
            float _20676 = fast::max((_12600 - _12607) - _12614, 0.0) / _12299;
            float _20680 = fast::clamp(sqrt(2.0 * _20676), 0.12399999797344207763671875, 1.0);
            float _20682 = 1.0 - _20680;
            float _20685 = _20682 * _20682;
            float _12661 = ((((((_20487 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _20487) * _20487) : ((_20487 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _20491) * _20491)) + ((((_20496 * _20496) * _20496) * _20493) * (0.02380952425301074981689453125 + (_20493 * ((-0.0386904776096343994140625) + (_20493 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_20487 - 0.5))))) * _12299) * _12299) - ((((_20550 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _20550) * _20550) : ((_20550 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _20554) * _20554)) + ((((_20559 * _20559) * _20559) * _20556) * (0.02380952425301074981689453125 + (_20556 * ((-0.0386904776096343994140625) + (_20556 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_20550 - 0.5))))) * _12299) * _12299)) - ((((_20613 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _20613) * _20613) : ((_20613 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _20617) * _20617)) + ((((_20622 * _20622) * _20622) * _20619) * (0.02380952425301074981689453125 + (_20619 * ((-0.0386904776096343994140625) + (_20619 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_20613 - 0.5))))) * _12299) * _12299)) + ((((_20676 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _20676) * _20676) : ((_20676 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _20680) * _20680)) + ((((_20685 * _20685) * _20685) * _20682) * (0.02380952425301074981689453125 + (_20682 * ((-0.0386904776096343994140625) + (_20682 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_20676 - 0.5))))) * _12299) * _12299);
            _28099 = _12661 / ((4.0 * _12607) * _12614);
        }
        else
        {
            _28099 = 0.0;
        }
        float _12693 = fast::clamp(0.5 + (0.5 * dot((in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5)) / _12291, _12586)), 0.0, 1.0);
        _30119 = ((_28099 * (pow(fast::clamp(_12589, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(fast::clamp(-_12589, 0.0, 1.0), 1.5)))) * gFxData_1._data[_12199].z) * 1.60000002384185791015625;
        _29473 = gFxData_1._data[_12207];
        _29237 = float4((mix(mix(_27880, gFxData_1._data[_12191].xyz, float3(fast::clamp(gFxData_1._data[_12191].w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _12314))) + (0.3499999940395355224609375 * ((dot(gFxData_1._data[_12191].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _12533.y) : _12533.y))), 0.0, 1.0))), (_27987 * 1.10000002384185791015625) + float3(0.07999999821186065673828125), float3(pow(1.0 - _27884.z, 5.0) * 0.3499999940395355224609375)) + float3((((0.039999999105930328369140625 * _12693) * _12693) + (0.0500000007450580596923828125 * (1.0 - _12314))) * gFxData_1._data[_12199].z)) * _26624, _26624) + (_28316 * (1.0 - _26624));
    }
    else
    {
        _30119 = 0.0;
        _29473 = _26223;
        _29237 = _28316;
    }
    float4 _29249;
    float4 _29251;
    if ((in.i_flags.x & _3684) != 0u)
    {
        float4 _28522;
        float4 _28869;
        if (in.i_flags.y != 0u)
        {
            _28869 = gFxData_1._data[(in.i_instance * 24u) + 3u];
            _28522 = gFxData_1._data[(in.i_instance * 24u) + 4u];
        }
        else
        {
            _28869 = float4(0.0);
            _28522 = float4(0.0);
        }
        float4 _29224;
        do
        {
            if (in.i_flags.y == 0u)
            {
                _29224 = in.i_fill0;
                break;
            }
            float2 _20789 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float2 _20799 = fwidth(in.i_local);
            float _20801 = fast::max(length(_20799), 9.9999997473787516355514526367188e-05);
            float _29216;
            float _29220;
            if ((in.i_flags.y == 1u) || (in.i_flags.y == 4u))
            {
                float _20810 = cos(_28522.x);
                float _20813 = sin(_28522.x);
                float _20839 = ((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_20810, _20813)) / fast::max(0.5 * ((abs(_20810) * _20789.x) + (abs(_20813) * _20789.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (in.i_flags.y == 4u)
                {
                    float _20854 = fast::clamp((_20839 - _28522.y) / fast::max(_28522.z - _28522.y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _29224 = float4(fast::clamp(abs((fract(float3(_20854 * 0.800000011920928955078125) + float3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - float3(3.0)) - float3(1.0), float3(0.0), float3(1.0)), in.i_fill0.w * pow(fast::max(sin(_20854 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _29220 = -1.0;
                _29216 = _20839;
            }
            else
            {
                float _29217;
                float _29221;
                if (in.i_flags.y == 2u)
                {
                    _29221 = -1.0;
                    _29217 = length(in.i_local - (in.i_rect.xy + (_28522.xy * _20789))) / fast::max(_28522.z * fast::max(_20789.x, _20789.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    float2 _20922 = in.i_local - (in.i_rect.xy + (_28522.xy * _20789));
                    float _20933 = fract(((precise::atan2(_20922.y, _20922.x) - _28522.z) * 0.15915493667125701904296875) + 1.0);
                    float _29218;
                    float _29222;
                    if (_28522.w > 0.5)
                    {
                        _29222 = -1.0;
                        _29218 = 0.5 - (0.5 * cos(_20933 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _20953 = (((_20933 < 0.5) ? _20933 : (_20933 - 1.0)) * 6.283185482025146484375) * length(_20922);
                        float _29223;
                        if (abs(_20953) < _20801)
                        {
                            _29223 = fast::clamp(((_20953 / _20801) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _29223 = -1.0;
                        }
                        _29222 = _29223;
                        _29218 = _20933;
                    }
                    _29221 = _29222;
                    _29217 = _29218;
                }
                _29220 = _29221;
                _29216 = _29217;
            }
            float4 _20982 = float4(in.i_fill0.xyz * in.i_fill0.w, in.i_fill0.w);
            float4 _20994 = float4(_28869.xyz * _28869.w, _28869.w);
            float4 _21008 = select(mix(_20982, _20994, float4(fast::clamp(_29216, 0.0, 1.0))), mix(_20994, _20982, float4(_29220)), bool4(_29220 >= 0.0));
            _29224 = select(float4(0.0), float4(_21008.xyz / float3(_21008.w), _21008.w), bool4(_21008.w > 9.9999997473787516355514526367188e-06));
            break;
        } while(false);
        float4 _29225;
        float4 _29463;
        if ((in.i_flags.x & _3713) != 0u)
        {
            uint _21033 = (in.i_instance * 24u) + 18u;
            _29463 = gFxData_1._data[_21033];
            _29225 = _29224 * gTex.sample(gLinear, mix(gFxData_1._data[_21033].xy, gFxData_1._data[_21033].zw, (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875))));
        }
        else
        {
            _29463 = _26219;
            _29225 = _29224;
        }
        float _21043 = fast::clamp(_29225.w * _26624, 0.0, 1.0);
        _29251 = _29463;
        _29249 = float4(_29225.xyz * _21043, _21043) + (_29237 * (1.0 - _21043));
    }
    else
    {
        _29251 = _26219;
        _29249 = _29237;
    }
    float4 _30090;
    if ((in.i_flags.x & _3766) != 0u)
    {
        uint _21067 = (in.i_instance * 24u) + 21u;
        uint _21075 = (in.i_instance * 24u) + 3u;
        float2 _5616 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float2 _5623 = (in.i_local - in.i_rect.xy) / _5616;
        float _5634 = ((gFxData_1._data[_21075].x >= 0.0) ? gFxData_1._data[_21075].x : _226.gTime.x) * gFxData_1._data[_21067].z;
        float _5637 = _5623.x * 2.0;
        float _5638 = _5637 - 1.0;
        float _5643 = _5623.y * _5616.y;
        float _5646 = fast::max(gFxData_1._data[_21067].w, 0.001000000047497451305389404296875);
        float _5651 = fast::clamp(1.0 - (_5638 * _5638), 0.0, 1.0);
        float _5653 = pow(_5651, 1.2999999523162841796875);
        float _5655 = pow(_5651, 0.699999988079071044921875);
        float _5658 = fast::clamp(_5634 * 1.4285714626312255859375, 0.0, 1.0);
        float _5674 = (0.25 + (0.75 * ((_5658 * _5658) * (3.0 - (2.0 * _5658))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_5634 * 2.099999904632568359375)));
        float _5683 = ((gFxData_1._data[_21067].y * _5616.y) * _5674) * _5653;
        float _5709 = (_5616.y * (0.5 + ((gFxData_1._data[_21067].x * (0.5 - (_5638 * _5638))) * 0.5))) + (((0.14000000059604644775390625 * _5616.y) * _5653) * sin(((_5638 * 2.400000095367431640625) - (_5634 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _29243;
        float _29244;
        float3 _29245;
        _29245 = float3(0.0);
        _29244 = _5709;
        _29243 = _5709;
        float3 _5781;
        float _31021;
        float _31022;
        for (int _29242 = 0; _29242 < 4; _29245 = _5781, _29244 = _31022, _29243 = _31021, _29242++)
        {
            float _5733 = _5709 + ((_5683 * _3928[_29242].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_5634 * 1.7000000476837158203125) + _3945[_29242].y))));
            _31021 = (_29242 == 0) ? _5733 : _29243;
            _31022 = (_29242 == 2) ? _5733 : _29244;
            float _5748 = _5646 * _3928[_29242].y;
            float _5753 = (_5643 - _5733) / _5748;
            float _5758 = _5646 * _3928[_29242].z;
            float3 _30783;
            _30783 = float3(0.0);
            for (int _30782 = 0; _30782 < 6; )
            {
                float _21100 = ((_5643 - _5733) - (_5758 * ((float(_30782) * 0.4000000059604644775390625) - 1.0))) / _5748;
                _30783 += (_2312[_30782] * exp((-_21100) * _21100));
                _30782++;
                continue;
            }
            _5781 = _29245 + (mix(_30783 * float3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), float3(exp((-_5753) * _5753)), float3(_3945[_29242].x)) * (_3928[_29242].w * _5655));
        }
        float _5787 = _5646 * 1.5;
        float _5817 = fast::clamp((_5643 - _29243) / fast::max(_29244 - _29243, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _5845 = (_5643 - (_29244 - (_5646 * 3.0))) / (((_5616.y * 0.0900000035762786865234375) + (_5683 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _5848 = (_5637 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _5873 = ((_5643 - _29243) + (_5646 * 5.0)) / (_5646 * 7.0);
        float3 _5899 = float3(1.0) - exp((-((((_29245 + (mix(float3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), float3(1.0), float3(_5817)) * ((((1.0 / (1.0 + exp((-((_5643 - _29243) - (_5646 * 2.0))) / _5787))) / (1.0 + exp((-(_29244 - _5643)) / _5787))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_5817, 2.5)))) * _5655))) + (float3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_5845) * _5845) - (_5848 * _5848)) * (0.5 + (1.10000002384185791015625 * _5674))))) + (float3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_5873) * _5873) * _5653) * 0.100000001490116119384765625))) * mix(float3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), float3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), float3(0.5 + (0.5 * sin(_5634 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _5909 = (in.i_fill0.w * smoothstep(0.0, 0.119999997317790985107421875, _5623.y)) * smoothstep(1.0, 0.87999999523162841796875, _5623.y);
        float _5928 = (fast::clamp(fast::max(_5899.x, fast::max(_5899.y, _5899.z)), 0.0, 1.0) * _5909) * _26624;
        _30090 = float4((_5899 * _5909) * _26624, _5928) + (_29249 * (1.0 - _5928));
    }
    else
    {
        _30090 = _29249;
    }
    float4 _30114;
    if (((in.i_flags.x & _4197) != 0u) && ((in.i_flags.x & _4201) != 0u))
    {
        uint _21132 = (in.i_instance * 24u) + 7u;
        uint _21140 = (in.i_instance * 24u) + 8u;
        float2 _21187 = select(_268.gDrawInfo.yz, _268.gDrawInfo.yz - ((in.i_rect.xy + in.i_rect.zw) * 0.5), bool2(_268.gDrawInfo.w > 1.0));
        float2 _5961 = in.i_local - select(gFxData_1._data[_21140].zw, (-(_21187 / float2(fast::max(length(_21187), 9.9999997473787516355514526367188e-05)))) * length(gFxData_1._data[_21140].zw), bool2(_268.gDrawInfo.w > 0.5));
        float2 _21238 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _21247 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _30014;
        if ((in.i_flags.x & _1710) != 0u)
        {
            float _21364 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
            float4 _21398 = gTex.sample(gLinear, mix(_29251.xy, _29251.zw, (fast::clamp(_5961, _26225.xy, _26225.zw) - _26225.xy) / fast::max(_26225.zw - _26225.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
            float _21422 = (_21398.w - 0.5) * in.i_radii.y;
            float2 _21463 = abs(_5961 - ((_26225.xy + _26225.zw) * 0.5)) - fast::max(((_26225.zw - _26225.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _21364), float2(0.0));
            _30014 = fast::max((-mix((fast::max(fast::min(_21398.x, _21398.y), fast::min(fast::max(_21398.x, _21398.y), _21398.z)) - 0.5) * in.i_radii.x, _21422, fast::clamp(abs(_21422) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _21364, length(fast::max(_21463, float2(0.0))) + fast::min(fast::max(_21463.x, _21463.y), 0.0)) - in.i_shape.w;
        }
        else
        {
            float _30015;
            if ((in.i_flags.x & _1725) != 0u)
            {
                float _30013;
                do
                {
                    bool _21525 = in.i_radii.w > 0.5;
                    float _21529 = floor((in.i_radii.z + 1.0) * 0.5);
                    float _21533 = floor((_21529 + 127.0) * 0.0078125);
                    float _21537 = floor((in.i_radii.z - 1.0) * 0.5);
                    float _21665 = floor(_21537 * 0.0625);
                    uint _21682 = (((in.i_instance + 1u) + uint(_21665)) * 24u) + uint(_21537 - (_21665 * 16.0));
                    float2 _29990;
                    if (_21525)
                    {
                        float _21692 = floor(0.0);
                        _29990 = gFxData_1._data[(((in.i_instance + 1u) + uint(_21692)) * 24u) + uint(_21692 * (-16.0))].xy;
                    }
                    else
                    {
                        _29990 = select(gFxData_1._data[_21682].zw, gFxData_1._data[_21682].xy, bool2(((in.i_radii.z - 1.0) - (_21537 * 2.0)) < 0.5));
                    }
                    float2 _29989;
                    float _29992;
                    float _29995;
                    _29995 = 1.0;
                    _29992 = 1000000015047466219876688855040.0;
                    _29989 = _29990;
                    float2 _29988;
                    float _29991;
                    float _29994;
                    for (float _29985 = 0.0; _29985 < 16.0; _29995 = _29994, _29992 = _29991, _29989 = _29988, _29985 += 1.0)
                    {
                        if (_29985 >= _21533)
                        {
                            break;
                        }
                        _29994 = _29995;
                        _29991 = _29992;
                        _29988 = _29989;
                        float2 _30985;
                        float _30987;
                        float _30989;
                        for (float _29986 = 0.0; _29986 < 128.0; _29994 = _30989, _29991 = _30987, _29988 = _30985, _29986 += 1.0)
                        {
                            float _21579 = (_29985 * 128.0) + _29986;
                            if (_21579 >= _21529)
                            {
                                break;
                            }
                            float _21719 = floor(_21579 * 0.0625);
                            uint _21736 = (((in.i_instance + 1u) + uint(_21719)) * 24u) + uint(_21579 - (_21719 * 16.0));
                            float2 _21749 = _29988 - gFxData_1._data[_21736].xy;
                            float2 _21752 = _5961 - gFxData_1._data[_21736].xy;
                            float2 _21765 = _21752 - (_21749 * fast::clamp(dot(_21752, _21749) / fast::max(dot(_21749, _21749), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                            float _21768 = dot(_21765, _21765);
                            float _31023 = (_21768 < _29991) ? _21768 : _29991;
                            bool _21780 = _5961.y >= gFxData_1._data[_21736].y;
                            bool _21785 = _5961.y < _29988.y;
                            bool _21796 = (_21749.x * _21752.y) > (_21749.y * _21752.x);
                            float _30000;
                            if (((_21780 && _21785) && _21796) || (((!_21780) && (!_21785)) && (!_21796)))
                            {
                                _30000 = -_29994;
                            }
                            else
                            {
                                _30000 = _29994;
                            }
                            if (((2.0 * _21579) + 1.0) < in.i_radii.z)
                            {
                                float2 _21825 = gFxData_1._data[_21736].xy - gFxData_1._data[_21736].zw;
                                float2 _21828 = _5961 - gFxData_1._data[_21736].zw;
                                float2 _21841 = _21828 - (_21825 * fast::clamp(dot(_21828, _21825) / fast::max(dot(_21825, _21825), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _21844 = dot(_21841, _21841);
                                bool _21856 = _5961.y >= gFxData_1._data[_21736].w;
                                bool _21861 = _5961.y < gFxData_1._data[_21736].y;
                                bool _21872 = (_21825.x * _21828.y) > (_21825.y * _21828.x);
                                float _30004;
                                if (((_21856 && _21861) && _21872) || (((!_21856) && (!_21861)) && (!_21872)))
                                {
                                    _30004 = -_30000;
                                }
                                else
                                {
                                    _30004 = _30000;
                                }
                                _30989 = _30004;
                                _30987 = (_21844 < _31023) ? _21844 : _31023;
                                _30985 = gFxData_1._data[_21736].zw;
                            }
                            else
                            {
                                _30989 = _30000;
                                _30987 = _31023;
                                _30985 = gFxData_1._data[_21736].xy;
                            }
                        }
                    }
                    float _21631 = sqrt(_29992);
                    if (_21525)
                    {
                        _30013 = _21631 - in.i_radii.x;
                        break;
                    }
                    _30013 = (_29995 * _21631) - in.i_radii.x;
                    break;
                } while(false);
                _30015 = _30013;
            }
            else
            {
                float _30016;
                if (in.i_flags.z == 1u)
                {
                    float2 _21272 = _5961 - _21238;
                    float _29984;
                    do
                    {
                        if (in.i_shape.w >= 6.282185077667236328125)
                        {
                            _29984 = abs(length(_21272) - in.i_radii.x) - in.i_radii.y;
                            break;
                        }
                        float _21915 = in.i_shape.z + (in.i_shape.w * 0.5);
                        float _21917 = cos(_21915);
                        float _21919 = sin(_21915);
                        float _21928 = dot(_21272, float2(-_21919, _21917));
                        float _21931 = dot(_21272, float2(_21917, _21919));
                        float2 _21932 = float2(_21928, _21931);
                        float _21935 = abs(_21928);
                        _21932.x = _21935;
                        float _21938 = in.i_shape.w * 0.5;
                        float _21940 = sin(_21938);
                        float _21942 = cos(_21938);
                        _29984 = (((_21942 * _21935) > (_21940 * _21931)) ? length(_21932 - (float2(_21940, _21942) * in.i_radii.x)) : abs(length(_21932) - in.i_radii.x)) - in.i_radii.y;
                        break;
                    } while(false);
                    _30016 = _29984;
                }
                else
                {
                    float _30017;
                    if (in.i_flags.z == 2u)
                    {
                        float2 _21978 = _5961 - _26225.xy;
                        float2 _21981 = _26225.zw - _26225.xy;
                        _30017 = length(_21978 - (_21981 * fast::clamp(dot(_21978, _21981) / fast::max(dot(_21981, _21981), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                    }
                    else
                    {
                        float2 _21299 = _5961 - _21238;
                        float _22034 = fast::min(_21247.x, _21247.y);
                        float _22037 = fast::min((_21299.x > 0.0) ? ((_21299.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_21299.y > 0.0) ? in.i_radii.w : in.i_radii.x), _22034);
                        float _22043 = _22037 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _29969;
                        float _29970;
                        if (_22043 > _22034)
                        {
                            float _22057 = in.i_shape.y * fast::clamp((_22034 - _22037) / fast::max(0.60000002384185791015625 * _22037, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _29970 = _22057;
                            _29969 = _22037 * (1.0 + (0.60000002384185791015625 * _22057));
                        }
                        else
                        {
                            _29970 = in.i_shape.y;
                            _29969 = _22043;
                        }
                        float2 _22070 = (abs(_21299) - _21247) + float2(_29969);
                        float2 _22072 = fast::max(_22070, float2(0.0));
                        float _29971;
                        if ((_22072.x > 0.0) && (_22072.y > 0.0))
                        {
                            float _29972;
                            if ((_29970 > 0.001000000047497451305389404296875) && (_29969 > 9.9999997473787516355514526367188e-05))
                            {
                                float _22089 = 2.0 + (2.0 * _29970);
                                float2 _22094 = _22072 / float2(fast::max(_29969, 9.9999997473787516355514526367188e-05));
                                _29972 = pow(pow(_22094.x, _22089) + pow(_22094.y, _22089), 1.0 / _22089) * _29969;
                            }
                            else
                            {
                                _29972 = length(_22072);
                            }
                            _29971 = _29972;
                        }
                        else
                        {
                            _29971 = fast::max(_22072.x, _22072.y);
                        }
                        float _22129 = (fast::min(fast::max(_22070.x, _22070.y), 0.0) + _29971) - _29969;
                        float _30018;
                        if ((in.i_flags.x & _1796) != 0u)
                        {
                            float2 _21327 = fast::max((_26225.zw - _26225.xy) * 0.5, float2(0.001000000047497451305389404296875));
                            float2 _21330 = _5961 - ((_26225.xy + _26225.zw) * 0.5);
                            float _22165 = fast::min(_21327.x, _21327.y);
                            float _22168 = fast::min((_21330.x > 0.0) ? ((_21330.y > 0.0) ? _29473.x : _29473.x) : ((_21330.y > 0.0) ? _29473.x : _29473.x), _22165);
                            float _22174 = _22168 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _29975;
                            float _29976;
                            if (_22174 > _22165)
                            {
                                float _22188 = in.i_shape.y * fast::clamp((_22165 - _22168) / fast::max(0.60000002384185791015625 * _22168, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _29976 = _22188;
                                _29975 = _22168 * (1.0 + (0.60000002384185791015625 * _22188));
                            }
                            else
                            {
                                _29976 = in.i_shape.y;
                                _29975 = _22174;
                            }
                            float2 _22201 = (abs(_21330) - _21327) + float2(_29975);
                            float2 _22203 = fast::max(_22201, float2(0.0));
                            float _29977;
                            if ((_22203.x > 0.0) && (_22203.y > 0.0))
                            {
                                float _29978;
                                if ((_29976 > 0.001000000047497451305389404296875) && (_29975 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _22220 = 2.0 + (2.0 * _29976);
                                    float2 _22225 = _22203 / float2(fast::max(_29975, 9.9999997473787516355514526367188e-05));
                                    _29978 = pow(pow(_22225.x, _22220) + pow(_22225.y, _22220), 1.0 / _22220) * _29975;
                                }
                                else
                                {
                                    _29978 = length(_22203);
                                }
                                _29977 = _29978;
                            }
                            else
                            {
                                _29977 = fast::max(_22203.x, _22203.y);
                            }
                            float _22260 = (fast::min(fast::max(_22201.x, _22201.y), 0.0) + _29977) - _29975;
                            float _22265 = fast::max(_29473.y, 9.9999997473787516355514526367188e-05);
                            float _22274 = fast::max(_22265 - abs(_22129 - _22260), 0.0) / _22265;
                            _30018 = fast::min(_22129, _22260) - (((_22274 * _22274) * _22265) * 0.25);
                        }
                        else
                        {
                            _30018 = _22129;
                        }
                        _30017 = _30018;
                    }
                    _30016 = _30017;
                }
                _30015 = _30016;
            }
            _30014 = _30015;
        }
        float _5970 = (_30014 + gFxData_1._data[_21140].y) / (fast::max(gFxData_1._data[_21140].x * 0.5, _4963 * 0.5) * 1.41421353816986083984375);
        float _22291 = sign(_5970);
        float _22293 = abs(_5970);
        float _22304 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_22293 * _22293))) * _22293)) * _22293);
        float _22307 = _22304 * _22304;
        float _22322 = fast::clamp(gFxData_1._data[_21132].w * ((0.5 + (0.5 * (_22291 - (_22291 / (_22307 * _22307))))) * _26624), 0.0, 1.0);
        _30114 = float4(gFxData_1._data[_21132].xyz * _22322, _22322) + (_30090 * (1.0 - _22322));
    }
    else
    {
        _30114 = _30090;
    }
    float4 _30151;
    if ((in.i_flags.x & _4261) != 0u)
    {
        uint _22346 = (in.i_instance * 24u) + 9u;
        uint _22354 = (in.i_instance * 24u) + 10u;
        float _6002 = fast::max(-_26273, 0.0) / fast::max(gFxData_1._data[_22354].z, 0.001000000047497451305389404296875);
        float _22364 = fast::clamp(gFxData_1._data[_22346].w * fast::clamp((exp(((-_6002) * _6002) * 2.2000000476837158203125) * gFxData_1._data[_22354].w) * _26624, 0.0, 1.0), 0.0, 1.0);
        _30151 = float4(gFxData_1._data[_22346].xyz * _22364, _22364) + (_30114 * (1.0 - _22364));
    }
    else
    {
        _30151 = _30114;
    }
    float3 _6033 = _30151.xyz + float3(_30119 * fast::clamp(_30151.w / fast::max(_26624, 0.001000000047497451305389404296875), 0.0, 1.0));
    float4 _26111 = _30151;
    _26111.x = _6033.x;
    _26111.y = _6033.y;
    _26111.z = _6033.z;
    float4 _30464;
    if ((in.i_flags.x & _4330) != 0u)
    {
        uint _22388 = (in.i_instance * 24u) + 5u;
        float4 _22390 = gFxData_1._data[_22388];
        uint _22396 = (in.i_instance * 24u) + 6u;
        float4 _30460;
        if (gFxData_1._data[_22396].z < 0.999000012874603271484375)
        {
            float2 _6095 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float _6106 = cos(gFxData_1._data[_22396].w);
            float _6109 = sin(gFxData_1._data[_22396].w);
            float4 _26128 = _22390;
            _26128.w = _22390.w * mix(1.0, gFxData_1._data[_22396].z, fast::clamp(((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_6106, _6109)) / fast::max(0.5 * ((abs(_6106) * _6095.x) + (abs(_6109) * _6095.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _30460 = _26128;
        }
        else
        {
            _30460 = _22390;
        }
        float _22406 = fast::clamp(_30460.w * ((_30162 >= 0.0) ? _30162 : (fast::clamp(0.5 - ((_26273 - (gFxData_1._data[_22396].x * gFxData_1._data[_22396].y)) / _4963), 0.0, 1.0) - fast::clamp(0.5 - ((_26273 + (gFxData_1._data[_22396].x * (1.0 - gFxData_1._data[_22396].y))) / _4963), 0.0, 1.0))), 0.0, 1.0);
        _30464 = float4(_30460.xyz * _22406, _22406) + (_26111 * (1.0 - _22406));
    }
    else
    {
        _30464 = _26111;
    }
    float4 _30465;
    if ((in.i_flags.x & _4464) != 0u)
    {
        float2 _6170 = (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float _6192 = exp(-pow((((_6170.x * 0.85000002384185791015625) + (_6170.y * 0.1500000059604644775390625)) - ((fract(_226.gTime.x * in.i_misc.w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _30465 = float4(_30464.xyz + float3(((_6192 * in.i_misc.z) * _26624) * fast::max(_30464.w, 0.3499999940395355224609375)), fast::max(_30464.w, ((_6192 * in.i_misc.z) * _26624) * 0.5));
    }
    else
    {
        _30465 = _30464;
    }
    float4 _30777;
    if ((in.i_flags.x & _4546) != 0u)
    {
        float3 _22431 = fract(floor(_26218).xyx * 0.103100001811981201171875);
        float3 _22440 = _22431 + float3(dot(_22431, _22431.yzx + float3(33.3300018310546875)));
        float3 _6243 = _30465.xyz + float3(((fract((_22440.x + _22440.y) * _22440.z) - 0.5) * in.i_misc.y) * _30465.w);
        float4 _26152 = _30465;
        _26152.x = _6243.x;
        _26152.y = _6243.y;
        _26152.z = _6243.z;
        _30777 = _26152;
    }
    else
    {
        _30777 = _30465;
    }
    float _30773;
    if (_268.gFade.z > 0.0)
    {
        _30773 = smoothstep(0.0, 1.0, fast::clamp((_26218.y - _268.gFade.x) / _268.gFade.z, 0.0, 1.0));
    }
    else
    {
        _30773 = 1.0;
    }
    float _30774;
    if (_268.gFade.w > 0.0)
    {
        _30774 = _30773 * smoothstep(0.0, 1.0, fast::clamp((_268.gFade.y - _26218.y) / _268.gFade.w, 0.0, 1.0));
    }
    else
    {
        _30774 = _30773;
    }
    float4 _6260 = _30777 * ((in.i_misc.x * _30477) * _30774);
    float4 _30778;
    if (((in.i_flags.x & _4586) != 0u) || ((in.i_flags.x & _4590) != 0u))
    {
        float3 _22492 = fract((floor(_26218) + float2(17.0)).xyx * 0.103100001811981201171875);
        float3 _22501 = _22492 + float3(dot(_22492, _22492.yzx + float3(33.3300018310546875)));
        float3 _6284 = _6260.xyz + float3(((fract((_22501.x + _22501.y) * _22501.z) - 0.5) * 0.0039215688593685626983642578125) * fast::clamp(_6260.w * 8.0, 0.0, 1.0));
        float4 _26164 = _6260;
        _26164.x = _6284.x;
        _26164.y = _6284.y;
        _26164.z = _6284.z;
        _30778 = _26164;
    }
    else
    {
        _30778 = _6260;
    }
    float3 _6294 = fast::max(_30778.xyz, float3(0.0));
    float4 _26170 = _30778;
    _26170.x = _6294.x;
    _26170.y = _6294.y;
    _26170.z = _6294.z;
    float4 _30779;
    if ((_226.gTime.w > 0.5) && (_30778.w > 9.9999997473787516355514526367188e-06))
    {
        float3 _22545 = fast::clamp(_26170.xyz / float3(_30778.w), float3(0.0), float3(1.0));
        float3 _22531 = select(pow((_22545 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _22545 * float3(0.077399380505084991455078125), _22545 <= float3(0.040449999272823333740234375)) * _30778.w;
        float4 _26179 = _26170;
        _26179.x = _22531.x;
        _26179.y = _22531.y;
        _26179.z = _22531.z;
        _30779 = _26179;
    }
    else
    {
        _30779 = _26170;
    }
    out._entryPointOutput = _30779;
    return out;
}

