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
constant uint _1753 = (32768u & kEsiaFxFeatures);
constant uint _1768 = (262144u & kEsiaFxFeatures);
constant uint _1839 = (512u & kEsiaFxFeatures);
constant uint _1891 = (262144u & kEsiaFxFeatures);
constant uint _1911 = (32768u & kEsiaFxFeatures);
constant uint _1918 = (512u & kEsiaFxFeatures);
constant uint _3104 = (512u & kEsiaFxFeatures);
constant uint _3125 = (32768u & kEsiaFxFeatures);
constant uint _3158 = (131072u & kEsiaFxFeatures);
constant uint _3294 = (2u & kEsiaFxFeatures);
constant uint _3304 = (1024u & kEsiaFxFeatures);
constant uint _3371 = (32u & kEsiaFxFeatures);
constant uint _3379 = (4u & kEsiaFxFeatures);
constant uint _3384 = (256u & kEsiaFxFeatures);
constant uint _3449 = (8u & kEsiaFxFeatures);
constant uint _3478 = (8192u & kEsiaFxFeatures);
constant uint _3695 = (65536u & kEsiaFxFeatures);
constant uint _3771 = (32u & kEsiaFxFeatures);
constant uint _3835 = (1u & kEsiaFxFeatures);
constant uint _3864 = (64u & kEsiaFxFeatures);
constant uint _3917 = (16384u & kEsiaFxFeatures);
constant uint _4348 = (4u & kEsiaFxFeatures);
constant uint _4352 = (256u & kEsiaFxFeatures);
constant uint _4412 = (16u & kEsiaFxFeatures);
constant uint _4481 = (2u & kEsiaFxFeatures);
constant uint _4615 = (128u & kEsiaFxFeatures);
constant uint _4697 = (2048u & kEsiaFxFeatures);
constant uint _4737 = (8u & kEsiaFxFeatures);
constant uint _4741 = (4u & kEsiaFxFeatures);

constant spvUnsafeArray<float3, 6> _2461 = spvUnsafeArray<float3, 6>({ float3(1.0, 0.4199999868869781494140625, 0.2199999988079071044921875), float3(1.0, 0.699999988079071044921875, 0.300000011920928955078125), float3(0.800000011920928955078125, 0.920000016689300537109375, 0.4000000059604644775390625), float3(0.3499999940395355224609375, 0.89999997615814208984375, 0.699999988079071044921875), float3(0.4000000059604644775390625, 0.62000000476837158203125, 1.0), float3(0.660000026226043701171875, 0.5, 1.0) });
constant spvUnsafeArray<float4, 4> _4079 = spvUnsafeArray<float4, 4>({ float4(-1.0, 1.0, 3.400000095367431640625, 2.599999904632568359375), float4(-0.550000011920928955078125, 0.800000011920928955078125, 2.0, 0.800000011920928955078125), float4(0.300000011920928955078125, 1.0, 1.2000000476837158203125, 1.2999999523162841796875), float4(0.62000000476837158203125, 0.800000011920928955078125, 1.60000002384185791015625, 0.449999988079071044921875) });
constant spvUnsafeArray<float2, 4> _4096 = spvUnsafeArray<float2, 4>({ float2(0.0), float2(0.100000001490116119384765625, 1.2999999523162841796875), float2(0.550000011920928955078125, 3.900000095367431640625), float2(0.20000000298023223876953125, 5.19999980926513671875) });

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

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _239 [[buffer(0)]], constant WgtDraw& _281 [[buffer(2)]], const device gFxData& gFxData_1 [[buffer(10)]], texture2d<float> gTex [[texture(3)]], texture2d<float> gBackdrop0 [[texture(4)]], texture2d<float> gBackdrop1 [[texture(5)]], texture2d<float> gBackdrop2 [[texture(6)]], texture2d<float> gBackdrop3 [[texture(7)]], texture2d<float> gBackdrop4 [[texture(8)]], texture2d<float> gBackdrop5 [[texture(9)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float2 _6506 = gl_FragCoord.xy + _239.gConv.yy;
    float2 _27489;
    if (_239.gConv.x > 0.5)
    {
        float2 _25736 = _6506;
        _25736.y = _239.gTarget.y - _6506.y;
        _27489 = _25736;
    }
    else
    {
        _27489 = _6506;
    }
    float _5107 = dfdx(in.i_local.x);
    float _5111 = dfdy(in.i_local.x);
    float _5114 = fast::max(abs(_5107) + abs(_5111), 9.9999997473787516355514526367188e-05);
    float4 _27494;
    float4 _27497;
    if ((in.i_flags.z == 2u) || ((in.i_flags.x & _3104) != 0u))
    {
        _27497 = gFxData_1._data[(in.i_instance * 24u) + 15u];
        _27494 = gFxData_1._data[(in.i_instance * 24u) + 16u];
    }
    else
    {
        _27497 = float4(0.0);
        _27494 = float4(0.0);
    }
    float4 _27490;
    float4 _27496;
    if ((in.i_flags.x & _3125) != 0u)
    {
        _27496 = gFxData_1._data[(in.i_instance * 24u) + 15u];
        _27490 = gFxData_1._data[(in.i_instance * 24u) + 18u];
    }
    else
    {
        _27496 = _27497;
        _27490 = float4(0.0);
    }
    float2 _6595 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
    float2 _6604 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
    float _27540;
    if ((in.i_flags.x & _1753) != 0u)
    {
        float _6721 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
        float4 _6755 = gTex.sample(gLinear, mix(_27490.xy, _27490.zw, (fast::clamp(in.i_local, _27496.xy, _27496.zw) - _27496.xy) / fast::max(_27496.zw - _27496.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
        float _6779 = (_6755.w - 0.5) * in.i_radii.y;
        float2 _6820 = abs(in.i_local - ((_27496.xy + _27496.zw) * 0.5)) - fast::max(((_27496.zw - _27496.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _6721), float2(0.0));
        _27540 = fast::max((-mix((fast::max(fast::min(_6755.x, _6755.y), fast::min(fast::max(_6755.x, _6755.y), _6755.z)) - 0.5) * in.i_radii.x, _6779, fast::clamp(abs(_6779) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _6721, length(fast::max(_6820, float2(0.0))) + fast::min(fast::max(_6820.x, _6820.y), 0.0)) - in.i_shape.w;
    }
    else
    {
        float _27541;
        if ((in.i_flags.x & _1768) != 0u)
        {
            float _27539;
            do
            {
                bool _6882 = in.i_radii.w > 0.5;
                float _6886 = floor((in.i_radii.z + 1.0) * 0.5);
                float _6890 = floor((_6886 + 127.0) * 0.0078125);
                float _6894 = floor((in.i_radii.z - 1.0) * 0.5);
                float _7022 = floor(_6894 * 0.0625);
                uint _7039 = (((in.i_instance + 1u) + uint(_7022)) * 24u) + uint(_6894 - (_7022 * 16.0));
                float2 _27516;
                if (_6882)
                {
                    float _7049 = floor(0.0);
                    _27516 = gFxData_1._data[(((in.i_instance + 1u) + uint(_7049)) * 24u) + uint(_7049 * (-16.0))].xy;
                }
                else
                {
                    _27516 = select(gFxData_1._data[_7039].zw, gFxData_1._data[_7039].xy, bool2(((in.i_radii.z - 1.0) - (_6894 * 2.0)) < 0.5));
                }
                float2 _27515;
                float _27518;
                float _27521;
                _27521 = 1.0;
                _27518 = 1000000015047466219876688855040.0;
                _27515 = _27516;
                float2 _27514;
                float _27517;
                float _27520;
                for (float _27511 = 0.0; _27511 < 16.0; _27521 = _27520, _27518 = _27517, _27515 = _27514, _27511 += 1.0)
                {
                    if (_27511 >= _6890)
                    {
                        break;
                    }
                    _27520 = _27521;
                    _27517 = _27518;
                    _27514 = _27515;
                    float2 _32310;
                    float _32312;
                    float _32314;
                    for (float _27512 = 0.0; _27512 < 128.0; _27520 = _32314, _27517 = _32312, _27514 = _32310, _27512 += 1.0)
                    {
                        float _6936 = (_27511 * 128.0) + _27512;
                        if (_6936 >= _6886)
                        {
                            break;
                        }
                        float _7076 = floor(_6936 * 0.0625);
                        uint _7093 = (((in.i_instance + 1u) + uint(_7076)) * 24u) + uint(_6936 - (_7076 * 16.0));
                        float2 _7106 = _27514 - gFxData_1._data[_7093].xy;
                        float2 _7109 = in.i_local - gFxData_1._data[_7093].xy;
                        float2 _7122 = _7109 - (_7106 * fast::clamp(dot(_7109, _7106) / fast::max(dot(_7106, _7106), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                        float _7125 = dot(_7122, _7122);
                        float _32515 = (_7125 < _27517) ? _7125 : _27517;
                        bool _7137 = in.i_local.y >= gFxData_1._data[_7093].y;
                        bool _7142 = in.i_local.y < _27514.y;
                        bool _7153 = (_7106.x * _7109.y) > (_7106.y * _7109.x);
                        float _27526;
                        if (((_7137 && _7142) && _7153) || (((!_7137) && (!_7142)) && (!_7153)))
                        {
                            _27526 = -_27520;
                        }
                        else
                        {
                            _27526 = _27520;
                        }
                        if (((2.0 * _6936) + 1.0) < in.i_radii.z)
                        {
                            float2 _7182 = gFxData_1._data[_7093].xy - gFxData_1._data[_7093].zw;
                            float2 _7185 = in.i_local - gFxData_1._data[_7093].zw;
                            float2 _7198 = _7185 - (_7182 * fast::clamp(dot(_7185, _7182) / fast::max(dot(_7182, _7182), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                            float _7201 = dot(_7198, _7198);
                            bool _7213 = in.i_local.y >= gFxData_1._data[_7093].w;
                            bool _7218 = in.i_local.y < gFxData_1._data[_7093].y;
                            bool _7229 = (_7182.x * _7185.y) > (_7182.y * _7185.x);
                            float _27530;
                            if (((_7213 && _7218) && _7229) || (((!_7213) && (!_7218)) && (!_7229)))
                            {
                                _27530 = -_27526;
                            }
                            else
                            {
                                _27530 = _27526;
                            }
                            _32314 = _27530;
                            _32312 = (_7201 < _32515) ? _7201 : _32515;
                            _32310 = gFxData_1._data[_7093].zw;
                        }
                        else
                        {
                            _32314 = _27526;
                            _32312 = _32515;
                            _32310 = gFxData_1._data[_7093].xy;
                        }
                    }
                }
                float _6988 = sqrt(_27518);
                if (_6882)
                {
                    _27539 = _6988 - in.i_radii.x;
                    break;
                }
                _27539 = (_27521 * _6988) - in.i_radii.x;
                break;
            } while(false);
            _27541 = _27539;
        }
        else
        {
            float _27542;
            if (in.i_flags.z == 1u)
            {
                float2 _6629 = in.i_local - _6595;
                float _27510;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _27510 = abs(length(_6629) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _7272 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _7274 = cos(_7272);
                    float _7276 = sin(_7272);
                    float _7285 = dot(_6629, float2(-_7276, _7274));
                    float _7288 = dot(_6629, float2(_7274, _7276));
                    float2 _7289 = float2(_7285, _7288);
                    float _7292 = abs(_7285);
                    _7289.x = _7292;
                    float _7295 = in.i_shape.w * 0.5;
                    float _7297 = sin(_7295);
                    float _7299 = cos(_7295);
                    _27510 = (((_7299 * _7292) > (_7297 * _7288)) ? length(_7289 - (float2(_7297, _7299) * in.i_radii.x)) : abs(length(_7289) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _27542 = _27510;
            }
            else
            {
                float _27543;
                if (in.i_flags.z == 2u)
                {
                    float2 _7335 = in.i_local - _27496.xy;
                    float2 _7338 = _27496.zw - _27496.xy;
                    _27543 = length(_7335 - (_7338 * fast::clamp(dot(_7335, _7338) / fast::max(dot(_7338, _7338), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _6656 = in.i_local - _6595;
                    float _7403 = fast::min(_6604.x, _6604.y);
                    float _7406 = fast::min((_6656.x > 0.0) ? ((_6656.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_6656.y > 0.0) ? in.i_radii.w : in.i_radii.x), _7403);
                    float _7412 = _7406 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _27499;
                    float _27500;
                    if (_7412 > _7403)
                    {
                        float _7426 = in.i_shape.y * fast::clamp((_7403 - _7406) / fast::max(0.60000002384185791015625 * _7406, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _27500 = _7426;
                        _27499 = _7406 * (1.0 + (0.60000002384185791015625 * _7426));
                    }
                    else
                    {
                        _27500 = in.i_shape.y;
                        _27499 = _7412;
                    }
                    float2 _7455 = (abs(_6656) - _6604) + float2(_27499);
                    float2 _7457 = fast::max(_7455, float2(0.0));
                    float _27501;
                    if ((_7457.x > 0.0) && (_7457.y > 0.0))
                    {
                        float _27502;
                        if ((_27500 > 0.001000000047497451305389404296875) && (_27499 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7474 = 2.0 + (2.0 * _27500);
                            float2 _7479 = _7457 / float2(fast::max(_27499, 9.9999997473787516355514526367188e-05));
                            _27502 = pow(pow(_7479.x, _7474) + pow(_7479.y, _7474), 1.0 / _7474) * _27499;
                        }
                        else
                        {
                            _27502 = length(_7457);
                        }
                        _27501 = _27502;
                    }
                    else
                    {
                        _27501 = fast::max(_7457.x, _7457.y);
                    }
                    float _7514 = (fast::min(fast::max(_7455.x, _7455.y), 0.0) + _27501) - _27499;
                    float _27544;
                    if ((in.i_flags.x & _1839) != 0u)
                    {
                        float2 _6684 = fast::max((_27496.zw - _27496.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _6687 = in.i_local - ((_27496.xy + _27496.zw) * 0.5);
                        float _7562 = fast::min(_6684.x, _6684.y);
                        float _7565 = fast::min((_6687.x > 0.0) ? ((_6687.y > 0.0) ? _27494.x : _27494.x) : ((_6687.y > 0.0) ? _27494.x : _27494.x), _7562);
                        float _7571 = _7565 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _27503;
                        float _27504;
                        if (_7571 > _7562)
                        {
                            float _7585 = in.i_shape.y * fast::clamp((_7562 - _7565) / fast::max(0.60000002384185791015625 * _7565, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _27504 = _7585;
                            _27503 = _7565 * (1.0 + (0.60000002384185791015625 * _7585));
                        }
                        else
                        {
                            _27504 = in.i_shape.y;
                            _27503 = _7571;
                        }
                        float2 _7614 = (abs(_6687) - _6684) + float2(_27503);
                        float2 _7616 = fast::max(_7614, float2(0.0));
                        float _27505;
                        if ((_7616.x > 0.0) && (_7616.y > 0.0))
                        {
                            float _27506;
                            if ((_27504 > 0.001000000047497451305389404296875) && (_27503 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7633 = 2.0 + (2.0 * _27504);
                                float2 _7638 = _7616 / float2(fast::max(_27503, 9.9999997473787516355514526367188e-05));
                                _27506 = pow(pow(_7638.x, _7633) + pow(_7638.y, _7633), 1.0 / _7633) * _27503;
                            }
                            else
                            {
                                _27506 = length(_7616);
                            }
                            _27505 = _27506;
                        }
                        else
                        {
                            _27505 = fast::max(_7616.x, _7616.y);
                        }
                        float _7673 = (fast::min(fast::max(_7614.x, _7614.y), 0.0) + _27505) - _27503;
                        float _7678 = fast::max(_27494.y, 9.9999997473787516355514526367188e-05);
                        float _7687 = fast::max(_7678 - abs(_7514 - _7673), 0.0) / _7678;
                        _27544 = fast::min(_7514, _7673) - (((_7687 * _7687) * _7678) * 0.25);
                    }
                    else
                    {
                        _27544 = _7514;
                    }
                    _27543 = _27544;
                }
                _27542 = _27543;
            }
            _27541 = _27542;
        }
        _27540 = _27541;
    }
    float _27881;
    float _31652;
    if ((in.i_flags.x & _3158) != 0u)
    {
        uint _7711 = (in.i_instance * 24u) + 6u;
        float _5174 = fast::clamp(ceil(length(gFxData_1._data[(in.i_instance * 24u) + 21u].xy) / (4.0 * _5114)), 1.0, 8.0);
        float _5179 = gFxData_1._data[_7711].x * gFxData_1._data[_7711].y;
        float _5185 = gFxData_1._data[_7711].x * (1.0 - gFxData_1._data[_7711].y);
        float2 _5189 = in.i_local + (gFxData_1._data[(in.i_instance * 24u) + 21u].xy * 0.5);
        float2 _7758 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _7767 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _27655;
        if ((in.i_flags.x & _1753) != 0u)
        {
            float _7884 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
            float4 _7918 = gTex.sample(gLinear, mix(_27490.xy, _27490.zw, (fast::clamp(_5189, _27496.xy, _27496.zw) - _27496.xy) / fast::max(_27496.zw - _27496.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
            float _7942 = (_7918.w - 0.5) * in.i_radii.y;
            float2 _7983 = abs(_5189 - ((_27496.xy + _27496.zw) * 0.5)) - fast::max(((_27496.zw - _27496.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _7884), float2(0.0));
            _27655 = fast::max((-mix((fast::max(fast::min(_7918.x, _7918.y), fast::min(fast::max(_7918.x, _7918.y), _7918.z)) - 0.5) * in.i_radii.x, _7942, fast::clamp(abs(_7942) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _7884, length(fast::max(_7983, float2(0.0))) + fast::min(fast::max(_7983.x, _7983.y), 0.0)) - in.i_shape.w;
        }
        else
        {
            float _27656;
            if ((in.i_flags.x & _1768) != 0u)
            {
                float _27654;
                do
                {
                    bool _8045 = in.i_radii.w > 0.5;
                    float _8049 = floor((in.i_radii.z + 1.0) * 0.5);
                    float _8053 = floor((_8049 + 127.0) * 0.0078125);
                    float _8057 = floor((in.i_radii.z - 1.0) * 0.5);
                    float _8185 = floor(_8057 * 0.0625);
                    uint _8202 = (((in.i_instance + 1u) + uint(_8185)) * 24u) + uint(_8057 - (_8185 * 16.0));
                    float2 _27631;
                    if (_8045)
                    {
                        float _8212 = floor(0.0);
                        _27631 = gFxData_1._data[(((in.i_instance + 1u) + uint(_8212)) * 24u) + uint(_8212 * (-16.0))].xy;
                    }
                    else
                    {
                        _27631 = select(gFxData_1._data[_8202].zw, gFxData_1._data[_8202].xy, bool2(((in.i_radii.z - 1.0) - (_8057 * 2.0)) < 0.5));
                    }
                    float2 _27630;
                    float _27633;
                    float _27636;
                    _27636 = 1.0;
                    _27633 = 1000000015047466219876688855040.0;
                    _27630 = _27631;
                    float2 _27629;
                    float _27632;
                    float _27635;
                    for (float _27626 = 0.0; _27626 < 16.0; _27636 = _27635, _27633 = _27632, _27630 = _27629, _27626 += 1.0)
                    {
                        if (_27626 >= _8053)
                        {
                            break;
                        }
                        _27635 = _27636;
                        _27632 = _27633;
                        _27629 = _27630;
                        float2 _32321;
                        float _32323;
                        float _32325;
                        for (float _27627 = 0.0; _27627 < 128.0; _27635 = _32325, _27632 = _32323, _27629 = _32321, _27627 += 1.0)
                        {
                            float _8099 = (_27626 * 128.0) + _27627;
                            if (_8099 >= _8049)
                            {
                                break;
                            }
                            float _8239 = floor(_8099 * 0.0625);
                            uint _8256 = (((in.i_instance + 1u) + uint(_8239)) * 24u) + uint(_8099 - (_8239 * 16.0));
                            float2 _8269 = _27629 - gFxData_1._data[_8256].xy;
                            float2 _8272 = _5189 - gFxData_1._data[_8256].xy;
                            float2 _8285 = _8272 - (_8269 * fast::clamp(dot(_8272, _8269) / fast::max(dot(_8269, _8269), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                            float _8288 = dot(_8285, _8285);
                            float _32517 = (_8288 < _27632) ? _8288 : _27632;
                            bool _8300 = _5189.y >= gFxData_1._data[_8256].y;
                            bool _8305 = _5189.y < _27629.y;
                            bool _8316 = (_8269.x * _8272.y) > (_8269.y * _8272.x);
                            float _27641;
                            if (((_8300 && _8305) && _8316) || (((!_8300) && (!_8305)) && (!_8316)))
                            {
                                _27641 = -_27635;
                            }
                            else
                            {
                                _27641 = _27635;
                            }
                            if (((2.0 * _8099) + 1.0) < in.i_radii.z)
                            {
                                float2 _8345 = gFxData_1._data[_8256].xy - gFxData_1._data[_8256].zw;
                                float2 _8348 = _5189 - gFxData_1._data[_8256].zw;
                                float2 _8361 = _8348 - (_8345 * fast::clamp(dot(_8348, _8345) / fast::max(dot(_8345, _8345), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _8364 = dot(_8361, _8361);
                                bool _8376 = _5189.y >= gFxData_1._data[_8256].w;
                                bool _8381 = _5189.y < gFxData_1._data[_8256].y;
                                bool _8392 = (_8345.x * _8348.y) > (_8345.y * _8348.x);
                                float _27645;
                                if (((_8376 && _8381) && _8392) || (((!_8376) && (!_8381)) && (!_8392)))
                                {
                                    _27645 = -_27641;
                                }
                                else
                                {
                                    _27645 = _27641;
                                }
                                _32325 = _27645;
                                _32323 = (_8364 < _32517) ? _8364 : _32517;
                                _32321 = gFxData_1._data[_8256].zw;
                            }
                            else
                            {
                                _32325 = _27641;
                                _32323 = _32517;
                                _32321 = gFxData_1._data[_8256].xy;
                            }
                        }
                    }
                    float _8151 = sqrt(_27633);
                    if (_8045)
                    {
                        _27654 = _8151 - in.i_radii.x;
                        break;
                    }
                    _27654 = (_27636 * _8151) - in.i_radii.x;
                    break;
                } while(false);
                _27656 = _27654;
            }
            else
            {
                float _27657;
                if (in.i_flags.z == 1u)
                {
                    float2 _7792 = _5189 - _7758;
                    float _27625;
                    do
                    {
                        if (in.i_shape.w >= 6.282185077667236328125)
                        {
                            _27625 = abs(length(_7792) - in.i_radii.x) - in.i_radii.y;
                            break;
                        }
                        float _8435 = in.i_shape.z + (in.i_shape.w * 0.5);
                        float _8437 = cos(_8435);
                        float _8439 = sin(_8435);
                        float _8448 = dot(_7792, float2(-_8439, _8437));
                        float _8451 = dot(_7792, float2(_8437, _8439));
                        float2 _8452 = float2(_8448, _8451);
                        float _8455 = abs(_8448);
                        _8452.x = _8455;
                        float _8458 = in.i_shape.w * 0.5;
                        float _8460 = sin(_8458);
                        float _8462 = cos(_8458);
                        _27625 = (((_8462 * _8455) > (_8460 * _8451)) ? length(_8452 - (float2(_8460, _8462) * in.i_radii.x)) : abs(length(_8452) - in.i_radii.x)) - in.i_radii.y;
                        break;
                    } while(false);
                    _27657 = _27625;
                }
                else
                {
                    float _27658;
                    if (in.i_flags.z == 2u)
                    {
                        float2 _8498 = _5189 - _27496.xy;
                        float2 _8501 = _27496.zw - _27496.xy;
                        _27658 = length(_8498 - (_8501 * fast::clamp(dot(_8498, _8501) / fast::max(dot(_8501, _8501), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                    }
                    else
                    {
                        float2 _7819 = _5189 - _7758;
                        float _8566 = fast::min(_7767.x, _7767.y);
                        float _8569 = fast::min((_7819.x > 0.0) ? ((_7819.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_7819.y > 0.0) ? in.i_radii.w : in.i_radii.x), _8566);
                        float _8575 = _8569 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _27614;
                        float _27615;
                        if (_8575 > _8566)
                        {
                            float _8589 = in.i_shape.y * fast::clamp((_8566 - _8569) / fast::max(0.60000002384185791015625 * _8569, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _27615 = _8589;
                            _27614 = _8569 * (1.0 + (0.60000002384185791015625 * _8589));
                        }
                        else
                        {
                            _27615 = in.i_shape.y;
                            _27614 = _8575;
                        }
                        float2 _8618 = (abs(_7819) - _7767) + float2(_27614);
                        float2 _8620 = fast::max(_8618, float2(0.0));
                        float _27616;
                        if ((_8620.x > 0.0) && (_8620.y > 0.0))
                        {
                            float _27617;
                            if ((_27615 > 0.001000000047497451305389404296875) && (_27614 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8637 = 2.0 + (2.0 * _27615);
                                float2 _8642 = _8620 / float2(fast::max(_27614, 9.9999997473787516355514526367188e-05));
                                _27617 = pow(pow(_8642.x, _8637) + pow(_8642.y, _8637), 1.0 / _8637) * _27614;
                            }
                            else
                            {
                                _27617 = length(_8620);
                            }
                            _27616 = _27617;
                        }
                        else
                        {
                            _27616 = fast::max(_8620.x, _8620.y);
                        }
                        float _8677 = (fast::min(fast::max(_8618.x, _8618.y), 0.0) + _27616) - _27614;
                        float _27659;
                        if ((in.i_flags.x & _1839) != 0u)
                        {
                            float2 _7847 = fast::max((_27496.zw - _27496.xy) * 0.5, float2(0.001000000047497451305389404296875));
                            float2 _7850 = _5189 - ((_27496.xy + _27496.zw) * 0.5);
                            float _8725 = fast::min(_7847.x, _7847.y);
                            float _8728 = fast::min((_7850.x > 0.0) ? ((_7850.y > 0.0) ? _27494.x : _27494.x) : ((_7850.y > 0.0) ? _27494.x : _27494.x), _8725);
                            float _8734 = _8728 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _27618;
                            float _27619;
                            if (_8734 > _8725)
                            {
                                float _8748 = in.i_shape.y * fast::clamp((_8725 - _8728) / fast::max(0.60000002384185791015625 * _8728, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _27619 = _8748;
                                _27618 = _8728 * (1.0 + (0.60000002384185791015625 * _8748));
                            }
                            else
                            {
                                _27619 = in.i_shape.y;
                                _27618 = _8734;
                            }
                            float2 _8777 = (abs(_7850) - _7847) + float2(_27618);
                            float2 _8779 = fast::max(_8777, float2(0.0));
                            float _27620;
                            if ((_8779.x > 0.0) && (_8779.y > 0.0))
                            {
                                float _27621;
                                if ((_27619 > 0.001000000047497451305389404296875) && (_27618 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _8796 = 2.0 + (2.0 * _27619);
                                    float2 _8801 = _8779 / float2(fast::max(_27618, 9.9999997473787516355514526367188e-05));
                                    _27621 = pow(pow(_8801.x, _8796) + pow(_8801.y, _8796), 1.0 / _8796) * _27618;
                                }
                                else
                                {
                                    _27621 = length(_8779);
                                }
                                _27620 = _27621;
                            }
                            else
                            {
                                _27620 = fast::max(_8779.x, _8779.y);
                            }
                            float _8836 = (fast::min(fast::max(_8777.x, _8777.y), 0.0) + _27620) - _27618;
                            float _8841 = fast::max(_27494.y, 9.9999997473787516355514526367188e-05);
                            float _8850 = fast::max(_8841 - abs(_8677 - _8836), 0.0) / _8841;
                            _27659 = fast::min(_8677, _8836) - (((_8850 * _8850) * _8841) * 0.25);
                        }
                        else
                        {
                            _27659 = _8677;
                        }
                        _27658 = _27659;
                    }
                    _27657 = _27658;
                }
                _27656 = _27657;
            }
            _27655 = _27656;
        }
        float _27804;
        float _27828;
        _27828 = 0.0;
        _27804 = 0.0;
        float _5224;
        float _5253;
        float _27774;
        int _27660 = 1;
        float _27780 = _27655;
        for (; _27660 <= 8; _27828 = _5253, _27804 = _5224, _27780 = _27774, _27660++)
        {
            if (float(_27660) > _5174)
            {
                break;
            }
            float2 _5211 = in.i_local + (gFxData_1._data[(in.i_instance * 24u) + 21u].xy * (0.5 - (float(_27660) / _5174)));
            float2 _8905 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _8914 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            if ((in.i_flags.x & _1753) != 0u)
            {
                float _9031 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                float4 _9065 = gTex.sample(gLinear, mix(_27490.xy, _27490.zw, (fast::clamp(_5211, _27496.xy, _27496.zw) - _27496.xy) / fast::max(_27496.zw - _27496.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                float _9089 = (_9065.w - 0.5) * in.i_radii.y;
                float2 _9130 = abs(_5211 - ((_27496.xy + _27496.zw) * 0.5)) - fast::max(((_27496.zw - _27496.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _9031), float2(0.0));
                _27774 = fast::max((-mix((fast::max(fast::min(_9065.x, _9065.y), fast::min(fast::max(_9065.x, _9065.y), _9065.z)) - 0.5) * in.i_radii.x, _9089, fast::clamp(abs(_9089) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _9031, length(fast::max(_9130, float2(0.0))) + fast::min(fast::max(_9130.x, _9130.y), 0.0)) - in.i_shape.w;
            }
            else
            {
                float _27775;
                if ((in.i_flags.x & _1768) != 0u)
                {
                    float _27773;
                    do
                    {
                        bool _9192 = in.i_radii.w > 0.5;
                        float _9196 = floor((in.i_radii.z + 1.0) * 0.5);
                        float _9200 = floor((_9196 + 127.0) * 0.0078125);
                        float _9204 = floor((in.i_radii.z - 1.0) * 0.5);
                        float _9332 = floor(_9204 * 0.0625);
                        uint _9349 = (((in.i_instance + 1u) + uint(_9332)) * 24u) + uint(_9204 - (_9332 * 16.0));
                        float2 _27750;
                        if (_9192)
                        {
                            float _9359 = floor(0.0);
                            _27750 = gFxData_1._data[(((in.i_instance + 1u) + uint(_9359)) * 24u) + uint(_9359 * (-16.0))].xy;
                        }
                        else
                        {
                            _27750 = select(gFxData_1._data[_9349].zw, gFxData_1._data[_9349].xy, bool2(((in.i_radii.z - 1.0) - (_9204 * 2.0)) < 0.5));
                        }
                        float2 _27749;
                        float _27752;
                        float _27755;
                        _27755 = 1.0;
                        _27752 = 1000000015047466219876688855040.0;
                        _27749 = _27750;
                        float2 _27748;
                        float _27751;
                        float _27754;
                        for (float _27745 = 0.0; _27745 < 16.0; _27755 = _27754, _27752 = _27751, _27749 = _27748, _27745 += 1.0)
                        {
                            if (_27745 >= _9200)
                            {
                                break;
                            }
                            _27754 = _27755;
                            _27751 = _27752;
                            _27748 = _27749;
                            float2 _32401;
                            float _32403;
                            float _32405;
                            for (float _27746 = 0.0; _27746 < 128.0; _27754 = _32405, _27751 = _32403, _27748 = _32401, _27746 += 1.0)
                            {
                                float _9246 = (_27745 * 128.0) + _27746;
                                if (_9246 >= _9196)
                                {
                                    break;
                                }
                                float _9386 = floor(_9246 * 0.0625);
                                uint _9403 = (((in.i_instance + 1u) + uint(_9386)) * 24u) + uint(_9246 - (_9386 * 16.0));
                                float2 _9416 = _27748 - gFxData_1._data[_9403].xy;
                                float2 _9419 = _5211 - gFxData_1._data[_9403].xy;
                                float2 _9432 = _9419 - (_9416 * fast::clamp(dot(_9419, _9416) / fast::max(dot(_9416, _9416), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _9435 = dot(_9432, _9432);
                                float _32519 = (_9435 < _27751) ? _9435 : _27751;
                                bool _9447 = _5211.y >= gFxData_1._data[_9403].y;
                                bool _9452 = _5211.y < _27748.y;
                                bool _9463 = (_9416.x * _9419.y) > (_9416.y * _9419.x);
                                float _27760;
                                if (((_9447 && _9452) && _9463) || (((!_9447) && (!_9452)) && (!_9463)))
                                {
                                    _27760 = -_27754;
                                }
                                else
                                {
                                    _27760 = _27754;
                                }
                                if (((2.0 * _9246) + 1.0) < in.i_radii.z)
                                {
                                    float2 _9492 = gFxData_1._data[_9403].xy - gFxData_1._data[_9403].zw;
                                    float2 _9495 = _5211 - gFxData_1._data[_9403].zw;
                                    float2 _9508 = _9495 - (_9492 * fast::clamp(dot(_9495, _9492) / fast::max(dot(_9492, _9492), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _9511 = dot(_9508, _9508);
                                    bool _9523 = _5211.y >= gFxData_1._data[_9403].w;
                                    bool _9528 = _5211.y < gFxData_1._data[_9403].y;
                                    bool _9539 = (_9492.x * _9495.y) > (_9492.y * _9495.x);
                                    float _27764;
                                    if (((_9523 && _9528) && _9539) || (((!_9523) && (!_9528)) && (!_9539)))
                                    {
                                        _27764 = -_27760;
                                    }
                                    else
                                    {
                                        _27764 = _27760;
                                    }
                                    _32405 = _27764;
                                    _32403 = (_9511 < _32519) ? _9511 : _32519;
                                    _32401 = gFxData_1._data[_9403].zw;
                                }
                                else
                                {
                                    _32405 = _27760;
                                    _32403 = _32519;
                                    _32401 = gFxData_1._data[_9403].xy;
                                }
                            }
                        }
                        float _9298 = sqrt(_27752);
                        if (_9192)
                        {
                            _27773 = _9298 - in.i_radii.x;
                            break;
                        }
                        _27773 = (_27755 * _9298) - in.i_radii.x;
                        break;
                    } while(false);
                    _27775 = _27773;
                }
                else
                {
                    float _27776;
                    if (in.i_flags.z == 1u)
                    {
                        float2 _8939 = _5211 - _8905;
                        float _27744;
                        do
                        {
                            if (in.i_shape.w >= 6.282185077667236328125)
                            {
                                _27744 = abs(length(_8939) - in.i_radii.x) - in.i_radii.y;
                                break;
                            }
                            float _9582 = in.i_shape.z + (in.i_shape.w * 0.5);
                            float _9584 = cos(_9582);
                            float _9586 = sin(_9582);
                            float _9595 = dot(_8939, float2(-_9586, _9584));
                            float _9598 = dot(_8939, float2(_9584, _9586));
                            float2 _9599 = float2(_9595, _9598);
                            float _9602 = abs(_9595);
                            _9599.x = _9602;
                            float _9605 = in.i_shape.w * 0.5;
                            float _9607 = sin(_9605);
                            float _9609 = cos(_9605);
                            _27744 = (((_9609 * _9602) > (_9607 * _9598)) ? length(_9599 - (float2(_9607, _9609) * in.i_radii.x)) : abs(length(_9599) - in.i_radii.x)) - in.i_radii.y;
                            break;
                        } while(false);
                        _27776 = _27744;
                    }
                    else
                    {
                        float _27777;
                        if (in.i_flags.z == 2u)
                        {
                            float2 _9645 = _5211 - _27496.xy;
                            float2 _9648 = _27496.zw - _27496.xy;
                            _27777 = length(_9645 - (_9648 * fast::clamp(dot(_9645, _9648) / fast::max(dot(_9648, _9648), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                        }
                        else
                        {
                            float2 _8966 = _5211 - _8905;
                            float _9713 = fast::min(_8914.x, _8914.y);
                            float _9716 = fast::min((_8966.x > 0.0) ? ((_8966.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_8966.y > 0.0) ? in.i_radii.w : in.i_radii.x), _9713);
                            float _9722 = _9716 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _27733;
                            float _27734;
                            if (_9722 > _9713)
                            {
                                float _9736 = in.i_shape.y * fast::clamp((_9713 - _9716) / fast::max(0.60000002384185791015625 * _9716, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _27734 = _9736;
                                _27733 = _9716 * (1.0 + (0.60000002384185791015625 * _9736));
                            }
                            else
                            {
                                _27734 = in.i_shape.y;
                                _27733 = _9722;
                            }
                            float2 _9765 = (abs(_8966) - _8914) + float2(_27733);
                            float2 _9767 = fast::max(_9765, float2(0.0));
                            float _27735;
                            if ((_9767.x > 0.0) && (_9767.y > 0.0))
                            {
                                float _27736;
                                if ((_27734 > 0.001000000047497451305389404296875) && (_27733 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _9784 = 2.0 + (2.0 * _27734);
                                    float2 _9789 = _9767 / float2(fast::max(_27733, 9.9999997473787516355514526367188e-05));
                                    _27736 = pow(pow(_9789.x, _9784) + pow(_9789.y, _9784), 1.0 / _9784) * _27733;
                                }
                                else
                                {
                                    _27736 = length(_9767);
                                }
                                _27735 = _27736;
                            }
                            else
                            {
                                _27735 = fast::max(_9767.x, _9767.y);
                            }
                            float _9824 = (fast::min(fast::max(_9765.x, _9765.y), 0.0) + _27735) - _27733;
                            float _27778;
                            if ((in.i_flags.x & _1839) != 0u)
                            {
                                float2 _8994 = fast::max((_27496.zw - _27496.xy) * 0.5, float2(0.001000000047497451305389404296875));
                                float2 _8997 = _5211 - ((_27496.xy + _27496.zw) * 0.5);
                                float _9872 = fast::min(_8994.x, _8994.y);
                                float _9875 = fast::min((_8997.x > 0.0) ? ((_8997.y > 0.0) ? _27494.x : _27494.x) : ((_8997.y > 0.0) ? _27494.x : _27494.x), _9872);
                                float _9881 = _9875 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                float _27737;
                                float _27738;
                                if (_9881 > _9872)
                                {
                                    float _9895 = in.i_shape.y * fast::clamp((_9872 - _9875) / fast::max(0.60000002384185791015625 * _9875, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                    _27738 = _9895;
                                    _27737 = _9875 * (1.0 + (0.60000002384185791015625 * _9895));
                                }
                                else
                                {
                                    _27738 = in.i_shape.y;
                                    _27737 = _9881;
                                }
                                float2 _9924 = (abs(_8997) - _8994) + float2(_27737);
                                float2 _9926 = fast::max(_9924, float2(0.0));
                                float _27739;
                                if ((_9926.x > 0.0) && (_9926.y > 0.0))
                                {
                                    float _27740;
                                    if ((_27738 > 0.001000000047497451305389404296875) && (_27737 > 9.9999997473787516355514526367188e-05))
                                    {
                                        float _9943 = 2.0 + (2.0 * _27738);
                                        float2 _9948 = _9926 / float2(fast::max(_27737, 9.9999997473787516355514526367188e-05));
                                        _27740 = pow(pow(_9948.x, _9943) + pow(_9948.y, _9943), 1.0 / _9943) * _27737;
                                    }
                                    else
                                    {
                                        _27740 = length(_9926);
                                    }
                                    _27739 = _27740;
                                }
                                else
                                {
                                    _27739 = fast::max(_9926.x, _9926.y);
                                }
                                float _9983 = (fast::min(fast::max(_9924.x, _9924.y), 0.0) + _27739) - _27737;
                                float _9988 = fast::max(_27494.y, 9.9999997473787516355514526367188e-05);
                                float _9997 = fast::max(_9988 - abs(_9824 - _9983), 0.0) / _9988;
                                _27778 = fast::min(_9824, _9983) - (((_9997 * _9997) * _9988) * 0.25);
                            }
                            else
                            {
                                _27778 = _9824;
                            }
                            _27777 = _27778;
                        }
                        _27776 = _27777;
                    }
                    _27775 = _27776;
                }
                _27774 = _27775;
            }
            float _5216 = _27780 / _5114;
            float _5217 = 0.5 - _5216;
            float _5220 = _27774 / _5114;
            float _5221 = 0.5 - _5220;
            float _10015 = _5221 - _5217;
            _5224 = _27804 + ((abs(_10015) < 0.001000000047497451305389404296875) ? fast::clamp(0.5 * (_5217 + _5221), 0.0, 1.0) : ((((_5221 <= 0.0) ? 0.0 : ((_5221 >= 1.0) ? (-_5220) : ((0.5 * _5221) * _5221))) - ((_5217 <= 0.0) ? 0.0 : ((_5217 >= 1.0) ? (-_5216) : ((0.5 * _5217) * _5217)))) / _10015));
            float _5229 = (_27780 - _5179) / _5114;
            float _5230 = 0.5 - _5229;
            float _5235 = (_27774 - _5179) / _5114;
            float _5236 = 0.5 - _5235;
            float _10067 = _5236 - _5230;
            float _5242 = (_27780 + _5185) / _5114;
            float _5243 = 0.5 - _5242;
            float _5248 = (_27774 + _5185) / _5114;
            float _5249 = 0.5 - _5248;
            float _10119 = _5249 - _5243;
            _5253 = _27828 + (((abs(_10067) < 0.001000000047497451305389404296875) ? fast::clamp(0.5 * (_5230 + _5236), 0.0, 1.0) : ((((_5236 <= 0.0) ? 0.0 : ((_5236 >= 1.0) ? (-_5235) : ((0.5 * _5236) * _5236))) - ((_5230 <= 0.0) ? 0.0 : ((_5230 >= 1.0) ? (-_5229) : ((0.5 * _5230) * _5230)))) / _10067)) - ((abs(_10119) < 0.001000000047497451305389404296875) ? fast::clamp(0.5 * (_5243 + _5249), 0.0, 1.0) : ((((_5249 <= 0.0) ? 0.0 : ((_5249 >= 1.0) ? (-_5248) : ((0.5 * _5249) * _5249))) - ((_5243 <= 0.0) ? 0.0 : ((_5243 >= 1.0) ? (-_5242) : ((0.5 * _5243) * _5243)))) / _10119)));
        }
        _31652 = ((in.i_flags.x & _3294) != 0u) ? (_27828 / _5174) : (-1.0);
        _27881 = _27804 / _5174;
    }
    else
    {
        _31652 = -1.0;
        _27881 = fast::clamp(0.5 - (_27540 / _5114), 0.0, 1.0);
    }
    float _31980;
    if ((in.i_flags.x & _3304) != 0u)
    {
        uint _10169 = (in.i_instance * 24u) + 19u;
        uint _10177 = (in.i_instance * 24u) + 20u;
        float2 _5298 = fast::max((gFxData_1._data[_10169].zw - gFxData_1._data[_10169].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float2 _5301 = in.i_local - ((gFxData_1._data[_10169].xy + gFxData_1._data[_10169].zw) * 0.5);
        float _10227 = fast::min(_5298.x, _5298.y);
        float _10230 = fast::min((_5301.x > 0.0) ? ((_5301.y > 0.0) ? gFxData_1._data[_10177].x : gFxData_1._data[_10177].x) : ((_5301.y > 0.0) ? gFxData_1._data[_10177].x : gFxData_1._data[_10177].x), _10227);
        float _10236 = _10230 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_10177].y));
        float _27876;
        float _27877;
        if (_10236 > _10227)
        {
            float _10250 = gFxData_1._data[_10177].y * fast::clamp((_10227 - _10230) / fast::max(0.60000002384185791015625 * _10230, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _27877 = _10250;
            _27876 = _10230 * (1.0 + (0.60000002384185791015625 * _10250));
        }
        else
        {
            _27877 = gFxData_1._data[_10177].y;
            _27876 = _10236;
        }
        float2 _10279 = (abs(_5301) - _5298) + float2(_27876);
        float2 _10281 = fast::max(_10279, float2(0.0));
        float _27878;
        if ((_10281.x > 0.0) && (_10281.y > 0.0))
        {
            float _27879;
            if ((_27877 > 0.001000000047497451305389404296875) && (_27876 > 9.9999997473787516355514526367188e-05))
            {
                float _10298 = 2.0 + (2.0 * _27877);
                float2 _10303 = _10281 / float2(fast::max(_27876, 9.9999997473787516355514526367188e-05));
                _27879 = pow(pow(_10303.x, _10298) + pow(_10303.y, _10298), 1.0 / _10298) * _27876;
            }
            else
            {
                _27879 = length(_10281);
            }
            _27878 = _27879;
        }
        else
        {
            _27878 = fast::max(_10281.x, _10281.y);
        }
        float _5313 = fast::clamp(0.5 - (((fast::min(fast::max(_10279.x, _10279.y), 0.0) + _27878) - _27876) / _5114), 0.0, 1.0);
        if (_5313 <= 0.0)
        {
            discard_fragment();
        }
        _31980 = _5313;
    }
    else
    {
        _31980 = 1.0;
    }
    bool _5324 = ((in.i_flags.x & _3371) != 0u) && (_27881 >= 0.999000012874603271484375);
    float4 _28034;
    if ((((in.i_flags.x & _3379) != 0u) && (!((in.i_flags.x & _3384) != 0u))) && (!_5324))
    {
        uint _10344 = (in.i_instance * 24u) + 7u;
        uint _10352 = (in.i_instance * 24u) + 8u;
        float2 _27885;
        if (_281.gDrawInfo.w > 0.5)
        {
            float2 _10401 = select(_281.gDrawInfo.yz, _281.gDrawInfo.yz - ((in.i_rect.xy + in.i_rect.zw) * 0.5), bool2(_281.gDrawInfo.w > 1.0));
            _27885 = (-(_10401 / float2(fast::max(length(_10401), 9.9999997473787516355514526367188e-05)))) * length(gFxData_1._data[_10352].zw);
        }
        else
        {
            _27885 = gFxData_1._data[_10352].zw;
        }
        float2 _5354 = in.i_local - _27885;
        float2 _10452 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _10461 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _27948;
        if ((in.i_flags.x & _1753) != 0u)
        {
            float _10578 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
            float4 _10612 = gTex.sample(gLinear, mix(_27490.xy, _27490.zw, (fast::clamp(_5354, _27496.xy, _27496.zw) - _27496.xy) / fast::max(_27496.zw - _27496.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
            float _10636 = (_10612.w - 0.5) * in.i_radii.y;
            float2 _10677 = abs(_5354 - ((_27496.xy + _27496.zw) * 0.5)) - fast::max(((_27496.zw - _27496.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _10578), float2(0.0));
            _27948 = fast::max((-mix((fast::max(fast::min(_10612.x, _10612.y), fast::min(fast::max(_10612.x, _10612.y), _10612.z)) - 0.5) * in.i_radii.x, _10636, fast::clamp(abs(_10636) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _10578, length(fast::max(_10677, float2(0.0))) + fast::min(fast::max(_10677.x, _10677.y), 0.0)) - in.i_shape.w;
        }
        else
        {
            float _27949;
            if ((in.i_flags.x & _1768) != 0u)
            {
                float _27947;
                do
                {
                    bool _10739 = in.i_radii.w > 0.5;
                    float _10743 = floor((in.i_radii.z + 1.0) * 0.5);
                    float _10747 = floor((_10743 + 127.0) * 0.0078125);
                    float _10751 = floor((in.i_radii.z - 1.0) * 0.5);
                    float _10879 = floor(_10751 * 0.0625);
                    uint _10896 = (((in.i_instance + 1u) + uint(_10879)) * 24u) + uint(_10751 - (_10879 * 16.0));
                    float2 _27924;
                    if (_10739)
                    {
                        float _10906 = floor(0.0);
                        _27924 = gFxData_1._data[(((in.i_instance + 1u) + uint(_10906)) * 24u) + uint(_10906 * (-16.0))].xy;
                    }
                    else
                    {
                        _27924 = select(gFxData_1._data[_10896].zw, gFxData_1._data[_10896].xy, bool2(((in.i_radii.z - 1.0) - (_10751 * 2.0)) < 0.5));
                    }
                    float2 _27923;
                    float _27926;
                    float _27929;
                    _27929 = 1.0;
                    _27926 = 1000000015047466219876688855040.0;
                    _27923 = _27924;
                    float2 _27922;
                    float _27925;
                    float _27928;
                    for (float _27919 = 0.0; _27919 < 16.0; _27929 = _27928, _27926 = _27925, _27923 = _27922, _27919 += 1.0)
                    {
                        if (_27919 >= _10747)
                        {
                            break;
                        }
                        _27928 = _27929;
                        _27925 = _27926;
                        _27922 = _27923;
                        float2 _32412;
                        float _32414;
                        float _32416;
                        for (float _27920 = 0.0; _27920 < 128.0; _27928 = _32416, _27925 = _32414, _27922 = _32412, _27920 += 1.0)
                        {
                            float _10793 = (_27919 * 128.0) + _27920;
                            if (_10793 >= _10743)
                            {
                                break;
                            }
                            float _10933 = floor(_10793 * 0.0625);
                            uint _10950 = (((in.i_instance + 1u) + uint(_10933)) * 24u) + uint(_10793 - (_10933 * 16.0));
                            float2 _10963 = _27922 - gFxData_1._data[_10950].xy;
                            float2 _10966 = _5354 - gFxData_1._data[_10950].xy;
                            float2 _10979 = _10966 - (_10963 * fast::clamp(dot(_10966, _10963) / fast::max(dot(_10963, _10963), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                            float _10982 = dot(_10979, _10979);
                            float _32521 = (_10982 < _27925) ? _10982 : _27925;
                            bool _10994 = _5354.y >= gFxData_1._data[_10950].y;
                            bool _10999 = _5354.y < _27922.y;
                            bool _11010 = (_10963.x * _10966.y) > (_10963.y * _10966.x);
                            float _27934;
                            if (((_10994 && _10999) && _11010) || (((!_10994) && (!_10999)) && (!_11010)))
                            {
                                _27934 = -_27928;
                            }
                            else
                            {
                                _27934 = _27928;
                            }
                            if (((2.0 * _10793) + 1.0) < in.i_radii.z)
                            {
                                float2 _11039 = gFxData_1._data[_10950].xy - gFxData_1._data[_10950].zw;
                                float2 _11042 = _5354 - gFxData_1._data[_10950].zw;
                                float2 _11055 = _11042 - (_11039 * fast::clamp(dot(_11042, _11039) / fast::max(dot(_11039, _11039), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _11058 = dot(_11055, _11055);
                                bool _11070 = _5354.y >= gFxData_1._data[_10950].w;
                                bool _11075 = _5354.y < gFxData_1._data[_10950].y;
                                bool _11086 = (_11039.x * _11042.y) > (_11039.y * _11042.x);
                                float _27938;
                                if (((_11070 && _11075) && _11086) || (((!_11070) && (!_11075)) && (!_11086)))
                                {
                                    _27938 = -_27934;
                                }
                                else
                                {
                                    _27938 = _27934;
                                }
                                _32416 = _27938;
                                _32414 = (_11058 < _32521) ? _11058 : _32521;
                                _32412 = gFxData_1._data[_10950].zw;
                            }
                            else
                            {
                                _32416 = _27934;
                                _32414 = _32521;
                                _32412 = gFxData_1._data[_10950].xy;
                            }
                        }
                    }
                    float _10845 = sqrt(_27926);
                    if (_10739)
                    {
                        _27947 = _10845 - in.i_radii.x;
                        break;
                    }
                    _27947 = (_27929 * _10845) - in.i_radii.x;
                    break;
                } while(false);
                _27949 = _27947;
            }
            else
            {
                float _27950;
                if (in.i_flags.z == 1u)
                {
                    float2 _10486 = _5354 - _10452;
                    float _27918;
                    do
                    {
                        if (in.i_shape.w >= 6.282185077667236328125)
                        {
                            _27918 = abs(length(_10486) - in.i_radii.x) - in.i_radii.y;
                            break;
                        }
                        float _11129 = in.i_shape.z + (in.i_shape.w * 0.5);
                        float _11131 = cos(_11129);
                        float _11133 = sin(_11129);
                        float _11142 = dot(_10486, float2(-_11133, _11131));
                        float _11145 = dot(_10486, float2(_11131, _11133));
                        float2 _11146 = float2(_11142, _11145);
                        float _11149 = abs(_11142);
                        _11146.x = _11149;
                        float _11152 = in.i_shape.w * 0.5;
                        float _11154 = sin(_11152);
                        float _11156 = cos(_11152);
                        _27918 = (((_11156 * _11149) > (_11154 * _11145)) ? length(_11146 - (float2(_11154, _11156) * in.i_radii.x)) : abs(length(_11146) - in.i_radii.x)) - in.i_radii.y;
                        break;
                    } while(false);
                    _27950 = _27918;
                }
                else
                {
                    float _27951;
                    if (in.i_flags.z == 2u)
                    {
                        float2 _11192 = _5354 - _27496.xy;
                        float2 _11195 = _27496.zw - _27496.xy;
                        _27951 = length(_11192 - (_11195 * fast::clamp(dot(_11192, _11195) / fast::max(dot(_11195, _11195), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                    }
                    else
                    {
                        float2 _10513 = _5354 - _10452;
                        float _11260 = fast::min(_10461.x, _10461.y);
                        float _11263 = fast::min((_10513.x > 0.0) ? ((_10513.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_10513.y > 0.0) ? in.i_radii.w : in.i_radii.x), _11260);
                        float _11269 = _11263 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _27907;
                        float _27908;
                        if (_11269 > _11260)
                        {
                            float _11283 = in.i_shape.y * fast::clamp((_11260 - _11263) / fast::max(0.60000002384185791015625 * _11263, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _27908 = _11283;
                            _27907 = _11263 * (1.0 + (0.60000002384185791015625 * _11283));
                        }
                        else
                        {
                            _27908 = in.i_shape.y;
                            _27907 = _11269;
                        }
                        float2 _11312 = (abs(_10513) - _10461) + float2(_27907);
                        float2 _11314 = fast::max(_11312, float2(0.0));
                        float _27909;
                        if ((_11314.x > 0.0) && (_11314.y > 0.0))
                        {
                            float _27910;
                            if ((_27908 > 0.001000000047497451305389404296875) && (_27907 > 9.9999997473787516355514526367188e-05))
                            {
                                float _11331 = 2.0 + (2.0 * _27908);
                                float2 _11336 = _11314 / float2(fast::max(_27907, 9.9999997473787516355514526367188e-05));
                                _27910 = pow(pow(_11336.x, _11331) + pow(_11336.y, _11331), 1.0 / _11331) * _27907;
                            }
                            else
                            {
                                _27910 = length(_11314);
                            }
                            _27909 = _27910;
                        }
                        else
                        {
                            _27909 = fast::max(_11314.x, _11314.y);
                        }
                        float _11371 = (fast::min(fast::max(_11312.x, _11312.y), 0.0) + _27909) - _27907;
                        float _27952;
                        if ((in.i_flags.x & _1839) != 0u)
                        {
                            float2 _10541 = fast::max((_27496.zw - _27496.xy) * 0.5, float2(0.001000000047497451305389404296875));
                            float2 _10544 = _5354 - ((_27496.xy + _27496.zw) * 0.5);
                            float _11419 = fast::min(_10541.x, _10541.y);
                            float _11422 = fast::min((_10544.x > 0.0) ? ((_10544.y > 0.0) ? _27494.x : _27494.x) : ((_10544.y > 0.0) ? _27494.x : _27494.x), _11419);
                            float _11428 = _11422 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _27911;
                            float _27912;
                            if (_11428 > _11419)
                            {
                                float _11442 = in.i_shape.y * fast::clamp((_11419 - _11422) / fast::max(0.60000002384185791015625 * _11422, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _27912 = _11442;
                                _27911 = _11422 * (1.0 + (0.60000002384185791015625 * _11442));
                            }
                            else
                            {
                                _27912 = in.i_shape.y;
                                _27911 = _11428;
                            }
                            float2 _11471 = (abs(_10544) - _10541) + float2(_27911);
                            float2 _11473 = fast::max(_11471, float2(0.0));
                            float _27913;
                            if ((_11473.x > 0.0) && (_11473.y > 0.0))
                            {
                                float _27914;
                                if ((_27912 > 0.001000000047497451305389404296875) && (_27911 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _11490 = 2.0 + (2.0 * _27912);
                                    float2 _11495 = _11473 / float2(fast::max(_27911, 9.9999997473787516355514526367188e-05));
                                    _27914 = pow(pow(_11495.x, _11490) + pow(_11495.y, _11490), 1.0 / _11490) * _27911;
                                }
                                else
                                {
                                    _27914 = length(_11473);
                                }
                                _27913 = _27914;
                            }
                            else
                            {
                                _27913 = fast::max(_11473.x, _11473.y);
                            }
                            float _11530 = (fast::min(fast::max(_11471.x, _11471.y), 0.0) + _27913) - _27911;
                            float _11535 = fast::max(_27494.y, 9.9999997473787516355514526367188e-05);
                            float _11544 = fast::max(_11535 - abs(_11371 - _11530), 0.0) / _11535;
                            _27952 = fast::min(_11371, _11530) - (((_11544 * _11544) * _11535) * 0.25);
                        }
                        else
                        {
                            _27952 = _11371;
                        }
                        _27951 = _27952;
                    }
                    _27950 = _27951;
                }
                _27949 = _27950;
            }
            _27948 = _27949;
        }
        float _5363 = (_27948 - gFxData_1._data[_10352].y) / (fast::max(gFxData_1._data[_10352].x * 0.5, _5114 * 0.5) * 1.41421353816986083984375);
        float _11561 = sign(_5363);
        float _11563 = abs(_5363);
        float _11574 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_11563 * _11563))) * _11563)) * _11563);
        float _11577 = _11574 * _11574;
        float _11592 = fast::clamp(gFxData_1._data[_10344].w * (0.5 - (0.5 * (_11561 - (_11561 / (_11577 * _11577))))), 0.0, 1.0);
        _28034 = float4(gFxData_1._data[_10344].xyz * _11592, _11592);
    }
    else
    {
        _28034 = float4(0.0);
    }
    float4 _28134;
    if (((in.i_flags.x & _3449) != 0u) && (!_5324))
    {
        uint _11616 = (in.i_instance * 24u) + 9u;
        uint _11624 = (in.i_instance * 24u) + 10u;
        float _5391 = fast::max(gFxData_1._data[_11624].x, 0.001000000047497451305389404296875);
        float _28027;
        float _28030;
        if ((in.i_flags.x & _3478) != 0u)
        {
            uint _11632 = (in.i_instance * 24u) + 22u;
            float2 _5407 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _5416 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float2 _5423 = in.i_rect.xy - gFxData_1._data[_11632].xy;
            float2 _5430 = gFxData_1._data[_11632].zw - in.i_rect.zw;
            float2 _5460 = fast::clamp(float2((in.i_local.x < _5407.x) ? _5423.x : _5430.x, (in.i_local.y < _5407.y) ? _5423.y : _5430.y) * float2(0.58823525905609130859375), float2(fast::min(_5391, 1.5)), float2(_5391));
            float _5465 = fast::min(_5416.x, _5416.y);
            float _28025;
            if (in.i_flags.z == 0u)
            {
                _28025 = fast::min(((in.i_local.x > _5407.x) ? ((in.i_local.y > _5407.y) ? in.i_radii.z : in.i_radii.y) : ((in.i_local.y > _5407.y) ? in.i_radii.w : in.i_radii.x)) * (1.0 + (0.60000002384185791015625 * in.i_shape.y)), _5465);
            }
            else
            {
                _28025 = _5465;
            }
            float2 _5514 = fast::max(abs(in.i_local - _5407) - (_5416 - float2(_28025)), float2(0.0));
            float _5516 = length(_5514);
            float2 _5524 = select(float2(0.707099974155426025390625), _5514 / float2(_5516), bool2(_5516 > 9.9999997473787516355514526367188e-05));
            float2 _5539 = in.i_local - gFxData_1._data[_11632].xy;
            float2 _5544 = gFxData_1._data[_11632].zw - in.i_local;
            _28030 = fast::clamp(fast::min(fast::min(_5539.x, _5539.y), fast::min(_5544.x, _5544.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _28027 = rsqrt(dot(_5524 * _5524, float2(1.0) / (_5460 * _5460)));
        }
        else
        {
            _28030 = 1.0;
            _28027 = _5391;
        }
        float _5562 = fast::max(_27540, 0.0) / _28027;
        float _11642 = fast::clamp(gFxData_1._data[_11616].w * fast::clamp((exp(((-_5562) * _5562) * 2.2000000476837158203125) * gFxData_1._data[_11624].y) * _28030, 0.0, 1.0), 0.0, 1.0);
        _28134 = float4(gFxData_1._data[_11616].xyz * _11642, _11642) + (_28034 * (1.0 - _11642));
    }
    else
    {
        _28134 = _28034;
    }
    float4 _29734;
    if ((in.i_flags.x & _3695) != 0u)
    {
        float2 _5593 = _27489 * _239.gTarget.zw;
        float _11675 = fast::clamp(log2(fast::max(fast::max(in.i_radii.w / _5114, 4.0), 1.0)) - 1.0, 0.0, 5.0);
        int _11678 = int(floor(_11675));
        float _11682 = _11675 - float(_11678);
        bool _11687 = (_11682 > 0.0199999995529651641845703125) && (_11678 < 5);
        float3 _28106;
        _28106 = float3(0.0);
        float3 _11716;
        for (int _28036 = 0; _28036 < 2; _28106 = _11716, _28036++)
        {
            if ((_28036 > 0) && (!_11687))
            {
                break;
            }
            int _11702 = _11678 + _28036;
            int _11740 = clamp(_11702, 1, 5);
            float2 _11809 = (_5593 * _239.gLevel[_11740].xy) - float2(0.5);
            float2 _11811 = floor(_11809);
            float2 _11814 = _11809 - _11811;
            float2 _11817 = _11814 * _11814;
            float2 _11820 = _11817 * _11814;
            float2 _11839 = (((_11820 * 3.0) - (_11817 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
            float2 _11852 = _11820 * 0.16666667163372039794921875;
            float2 _11855 = (((((-_11820) + (_11817 * 3.0)) - (_11814 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11839;
            float2 _11859 = (((((_11820 * (-3.0)) + (_11817 * 3.0)) + (_11814 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11852;
            float2 _11871 = ((_11811 - float2(0.5)) + (_11839 / _11855)) * _239.gLevel[_11740].zw;
            float2 _11883 = ((_11811 + float2(1.5)) + (_11852 / _11859)) * _239.gLevel[_11740].zw;
            float4 _28073;
            if (_11702 <= 0)
            {
                float2 _28072;
                if (_239.gConv.x > 0.5)
                {
                    float2 _26173 = _5593;
                    _26173.y = 1.0 - _5593.y;
                    _28072 = _26173;
                }
                else
                {
                    _28072 = _5593;
                }
                _28073 = gBackdrop0.sample(gLinear, _28072, level(0.0));
            }
            else
            {
                float4 _28074;
                if (_11702 == 1)
                {
                    float2 _28065;
                    if (_239.gConv.x > 0.5)
                    {
                        float2 _26178 = _11871;
                        _26178.y = 1.0 - _11871.y;
                        _28065 = _26178;
                    }
                    else
                    {
                        _28065 = _11871;
                    }
                    float _11928 = _11871.y;
                    float2 _11929 = float2(_11883.x, _11928);
                    float2 _28066;
                    if (_239.gConv.x > 0.5)
                    {
                        float2 _26185 = _11929;
                        _26185.y = 1.0 - _11928;
                        _28066 = _26185;
                    }
                    else
                    {
                        _28066 = _11929;
                    }
                    float _11946 = _11883.y;
                    float2 _11947 = float2(_11871.x, _11946);
                    float2 _28068;
                    if (_239.gConv.x > 0.5)
                    {
                        float2 _26192 = _11947;
                        _26192.y = 1.0 - _11946;
                        _28068 = _26192;
                    }
                    else
                    {
                        _28068 = _11947;
                    }
                    float2 _28070;
                    if (_239.gConv.x > 0.5)
                    {
                        float2 _26199 = _11883;
                        _26199.y = 1.0 - _11883.y;
                        _28070 = _26199;
                    }
                    else
                    {
                        _28070 = _11883;
                    }
                    _28074 = (((gBackdrop1.sample(gLinear, _28065, level(0.0)) * (_11855.x * _11855.y)) + (gBackdrop1.sample(gLinear, _28066, level(0.0)) * (_11859.x * _11855.y))) + (gBackdrop1.sample(gLinear, _28068, level(0.0)) * (_11855.x * _11859.y))) + (gBackdrop1.sample(gLinear, _28070, level(0.0)) * (_11859.x * _11859.y));
                }
                else
                {
                    float4 _28075;
                    if (_11702 == 2)
                    {
                        float2 _28058;
                        if (_239.gConv.x > 0.5)
                        {
                            float2 _26206 = _11871;
                            _26206.y = 1.0 - _11871.y;
                            _28058 = _26206;
                        }
                        else
                        {
                            _28058 = _11871;
                        }
                        float _12058 = _11871.y;
                        float2 _12059 = float2(_11883.x, _12058);
                        float2 _28059;
                        if (_239.gConv.x > 0.5)
                        {
                            float2 _26213 = _12059;
                            _26213.y = 1.0 - _12058;
                            _28059 = _26213;
                        }
                        else
                        {
                            _28059 = _12059;
                        }
                        float _12076 = _11883.y;
                        float2 _12077 = float2(_11871.x, _12076);
                        float2 _28061;
                        if (_239.gConv.x > 0.5)
                        {
                            float2 _26220 = _12077;
                            _26220.y = 1.0 - _12076;
                            _28061 = _26220;
                        }
                        else
                        {
                            _28061 = _12077;
                        }
                        float2 _28063;
                        if (_239.gConv.x > 0.5)
                        {
                            float2 _26227 = _11883;
                            _26227.y = 1.0 - _11883.y;
                            _28063 = _26227;
                        }
                        else
                        {
                            _28063 = _11883;
                        }
                        _28075 = (((gBackdrop2.sample(gLinear, _28058, level(0.0)) * (_11855.x * _11855.y)) + (gBackdrop2.sample(gLinear, _28059, level(0.0)) * (_11859.x * _11855.y))) + (gBackdrop2.sample(gLinear, _28061, level(0.0)) * (_11855.x * _11859.y))) + (gBackdrop2.sample(gLinear, _28063, level(0.0)) * (_11859.x * _11859.y));
                    }
                    else
                    {
                        float4 _28076;
                        if (_11702 == 3)
                        {
                            float2 _28051;
                            if (_239.gConv.x > 0.5)
                            {
                                float2 _26234 = _11871;
                                _26234.y = 1.0 - _11871.y;
                                _28051 = _26234;
                            }
                            else
                            {
                                _28051 = _11871;
                            }
                            float _12188 = _11871.y;
                            float2 _12189 = float2(_11883.x, _12188);
                            float2 _28052;
                            if (_239.gConv.x > 0.5)
                            {
                                float2 _26241 = _12189;
                                _26241.y = 1.0 - _12188;
                                _28052 = _26241;
                            }
                            else
                            {
                                _28052 = _12189;
                            }
                            float _12206 = _11883.y;
                            float2 _12207 = float2(_11871.x, _12206);
                            float2 _28054;
                            if (_239.gConv.x > 0.5)
                            {
                                float2 _26248 = _12207;
                                _26248.y = 1.0 - _12206;
                                _28054 = _26248;
                            }
                            else
                            {
                                _28054 = _12207;
                            }
                            float2 _28056;
                            if (_239.gConv.x > 0.5)
                            {
                                float2 _26255 = _11883;
                                _26255.y = 1.0 - _11883.y;
                                _28056 = _26255;
                            }
                            else
                            {
                                _28056 = _11883;
                            }
                            _28076 = (((gBackdrop3.sample(gLinear, _28051, level(0.0)) * (_11855.x * _11855.y)) + (gBackdrop3.sample(gLinear, _28052, level(0.0)) * (_11859.x * _11855.y))) + (gBackdrop3.sample(gLinear, _28054, level(0.0)) * (_11855.x * _11859.y))) + (gBackdrop3.sample(gLinear, _28056, level(0.0)) * (_11859.x * _11859.y));
                        }
                        else
                        {
                            float4 _28077;
                            if (_11702 == 4)
                            {
                                float2 _28044;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _26262 = _11871;
                                    _26262.y = 1.0 - _11871.y;
                                    _28044 = _26262;
                                }
                                else
                                {
                                    _28044 = _11871;
                                }
                                float _12318 = _11871.y;
                                float2 _12319 = float2(_11883.x, _12318);
                                float2 _28045;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _26269 = _12319;
                                    _26269.y = 1.0 - _12318;
                                    _28045 = _26269;
                                }
                                else
                                {
                                    _28045 = _12319;
                                }
                                float _12336 = _11883.y;
                                float2 _12337 = float2(_11871.x, _12336);
                                float2 _28047;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _26276 = _12337;
                                    _26276.y = 1.0 - _12336;
                                    _28047 = _26276;
                                }
                                else
                                {
                                    _28047 = _12337;
                                }
                                float2 _28049;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _26283 = _11883;
                                    _26283.y = 1.0 - _11883.y;
                                    _28049 = _26283;
                                }
                                else
                                {
                                    _28049 = _11883;
                                }
                                _28077 = (((gBackdrop4.sample(gLinear, _28044, level(0.0)) * (_11855.x * _11855.y)) + (gBackdrop4.sample(gLinear, _28045, level(0.0)) * (_11859.x * _11855.y))) + (gBackdrop4.sample(gLinear, _28047, level(0.0)) * (_11855.x * _11859.y))) + (gBackdrop4.sample(gLinear, _28049, level(0.0)) * (_11859.x * _11859.y));
                            }
                            else
                            {
                                float2 _28037;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _26290 = _11871;
                                    _26290.y = 1.0 - _11871.y;
                                    _28037 = _26290;
                                }
                                else
                                {
                                    _28037 = _11871;
                                }
                                float _12448 = _11871.y;
                                float2 _12449 = float2(_11883.x, _12448);
                                float2 _28038;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _26297 = _12449;
                                    _26297.y = 1.0 - _12448;
                                    _28038 = _26297;
                                }
                                else
                                {
                                    _28038 = _12449;
                                }
                                float _12466 = _11883.y;
                                float2 _12467 = float2(_11871.x, _12466);
                                float2 _28040;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _26304 = _12467;
                                    _26304.y = 1.0 - _12466;
                                    _28040 = _26304;
                                }
                                else
                                {
                                    _28040 = _12467;
                                }
                                float2 _28042;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _26311 = _11883;
                                    _26311.y = 1.0 - _11883.y;
                                    _28042 = _26311;
                                }
                                else
                                {
                                    _28042 = _11883;
                                }
                                _28077 = (((gBackdrop5.sample(gLinear, _28037, level(0.0)) * (_11855.x * _11855.y)) + (gBackdrop5.sample(gLinear, _28038, level(0.0)) * (_11859.x * _11855.y))) + (gBackdrop5.sample(gLinear, _28040, level(0.0)) * (_11855.x * _11859.y))) + (gBackdrop5.sample(gLinear, _28042, level(0.0)) * (_11859.x * _11859.y));
                            }
                            _28076 = _28077;
                        }
                        _28075 = _28076;
                    }
                    _28074 = _28075;
                }
                _28073 = _28074;
            }
            _11716 = _28106 + (_28073.xyz * (_11687 ? ((_28036 == 0) ? (1.0 - _11682) : _11682) : 1.0));
        }
        float _12552 = dot(in.i_fill0.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
        float _5623 = fast::max(_27540, 0.0) / fast::max(in.i_radii.w, 0.001000000047497451305389404296875);
        float _5626 = (_12552 > 0.5) ? 0.0 : 1.0;
        float _12564 = fast::clamp(0.85000002384185791015625 * ((fast::clamp((0.5 - abs(_12552 - dot(select(float3(0.5), _28106, bool3(_239.gTime.z > 0.5)), float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)))) * 4.0, 0.0, 1.0) * in.i_radii.z) * exp(((-_5623) * _5623) * 2.2000000476837158203125)), 0.0, 1.0);
        _29734 = float4(float4(_5626, _5626, _5626, 0.85000002384185791015625).xyz * _12564, _12564) + (_28134 * (1.0 - _12564));
    }
    else
    {
        _29734 = _28134;
    }
    float4 _30689;
    float4 _30939;
    float _31608;
    if (((in.i_flags.x & _3771) != 0u) && (_27881 > 0.0))
    {
        uint _12588 = (in.i_instance * 24u) + 11u;
        uint _12596 = (in.i_instance * 24u) + 12u;
        uint _12604 = (in.i_instance * 24u) + 13u;
        uint _12612 = (in.i_instance * 24u) + 16u;
        float _12685 = gFxData_1._data[_12588].x * _239.gDisplay.z;
        float2 _12696 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(1.0));
        float _12704 = fast::clamp(gFxData_1._data[_12588].z, 0.001000000047497451305389404296875, fast::min(_12696.x, _12696.y));
        float _12713 = fast::clamp(1.0 - (fast::max(-_27540, 0.0) / _12704), 0.0, 1.0);
        float _12719 = sqrt(fast::clamp(1.0 - (_12713 * _12713), 0.0, 1.0));
        float2 _28709;
        float3 _29286;
        if (_12713 > 0.0)
        {
            float2 _28708;
            do
            {
                if ((in.i_flags.x & _1891) != 0u)
                {
                    float2 _28707;
                    do
                    {
                        bool _13329 = in.i_radii.w > 0.5;
                        float _13333 = floor((in.i_radii.z + 1.0) * 0.5);
                        float _13337 = floor((_13333 + 127.0) * 0.0078125);
                        float _13341 = floor((in.i_radii.z - 1.0) * 0.5);
                        float _13469 = floor(_13341 * 0.0625);
                        uint _13486 = (((in.i_instance + 1u) + uint(_13469)) * 24u) + uint(_13341 - (_13469 * 16.0));
                        float2 _28676;
                        if (_13329)
                        {
                            float _13496 = floor(0.0);
                            _28676 = gFxData_1._data[(((in.i_instance + 1u) + uint(_13496)) * 24u) + uint(_13496 * (-16.0))].xy;
                        }
                        else
                        {
                            _28676 = select(gFxData_1._data[_13486].zw, gFxData_1._data[_13486].xy, bool2(((in.i_radii.z - 1.0) - (_13341 * 2.0)) < 0.5));
                        }
                        float2 _28675;
                        float _28678;
                        float2 _28681;
                        float _28684;
                        _28684 = 1.0;
                        _28681 = float2(0.0, -1.0);
                        _28678 = 1000000015047466219876688855040.0;
                        _28675 = _28676;
                        float2 _28674;
                        float _28677;
                        float2 _28680;
                        float _28683;
                        for (float _28671 = 0.0; _28671 < 16.0; _28684 = _28683, _28681 = _28680, _28678 = _28677, _28675 = _28674, _28671 += 1.0)
                        {
                            if (_28671 >= _13337)
                            {
                                break;
                            }
                            _28683 = _28684;
                            _28680 = _28681;
                            _28677 = _28678;
                            _28674 = _28675;
                            float2 _32467;
                            float _32469;
                            float2 _32471;
                            float _32473;
                            for (float _28672 = 0.0; _28672 < 128.0; _28683 = _32473, _28680 = _32471, _28677 = _32469, _28674 = _32467, _28672 += 1.0)
                            {
                                float _13383 = (_28671 * 128.0) + _28672;
                                if (_13383 >= _13333)
                                {
                                    break;
                                }
                                float _13523 = floor(_13383 * 0.0625);
                                uint _13540 = (((in.i_instance + 1u) + uint(_13523)) * 24u) + uint(_13383 - (_13523 * 16.0));
                                float2 _13553 = _28674 - gFxData_1._data[_13540].xy;
                                float2 _13556 = in.i_local - gFxData_1._data[_13540].xy;
                                float2 _13569 = _13556 - (_13553 * fast::clamp(dot(_13556, _13553) / fast::max(dot(_13553, _13553), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _13572 = dot(_13569, _13569);
                                bool _13575 = _13572 < _28677;
                                float2 _32524 = select(_28680, _13569, bool2(_13575));
                                float _32525 = _13575 ? _13572 : _28677;
                                bool _13584 = in.i_local.y >= gFxData_1._data[_13540].y;
                                bool _13589 = in.i_local.y < _28674.y;
                                bool _13600 = (_13553.x * _13556.y) > (_13553.y * _13556.x);
                                float _28691;
                                if (((_13584 && _13589) && _13600) || (((!_13584) && (!_13589)) && (!_13600)))
                                {
                                    _28691 = -_28683;
                                }
                                else
                                {
                                    _28691 = _28683;
                                }
                                if (((2.0 * _13383) + 1.0) < in.i_radii.z)
                                {
                                    float2 _13629 = gFxData_1._data[_13540].xy - gFxData_1._data[_13540].zw;
                                    float2 _13632 = in.i_local - gFxData_1._data[_13540].zw;
                                    float2 _13645 = _13632 - (_13629 * fast::clamp(dot(_13632, _13629) / fast::max(dot(_13629, _13629), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _13648 = dot(_13645, _13645);
                                    bool _13651 = _13648 < _32525;
                                    bool _13660 = in.i_local.y >= gFxData_1._data[_13540].w;
                                    bool _13665 = in.i_local.y < gFxData_1._data[_13540].y;
                                    bool _13676 = (_13629.x * _13632.y) > (_13629.y * _13632.x);
                                    float _28697;
                                    if (((_13660 && _13665) && _13676) || (((!_13660) && (!_13665)) && (!_13676)))
                                    {
                                        _28697 = -_28691;
                                    }
                                    else
                                    {
                                        _28697 = _28691;
                                    }
                                    _32473 = _28697;
                                    _32471 = select(_32524, _13645, bool2(_13651));
                                    _32469 = _13651 ? _13648 : _32525;
                                    _32467 = gFxData_1._data[_13540].zw;
                                }
                                else
                                {
                                    _32473 = _28691;
                                    _32471 = _32524;
                                    _32469 = _32525;
                                    _32467 = gFxData_1._data[_13540].xy;
                                }
                            }
                        }
                        float _13435 = sqrt(_28678);
                        float2 _13443 = select(float2(0.0, -1.0), _28681 / float2(_13435), bool2(_13435 > 9.9999999747524270787835121154785e-07));
                        if (_13329)
                        {
                            _28707 = _13443;
                            break;
                        }
                        _28707 = _13443 * _28684;
                        break;
                    } while(false);
                    _28708 = _28707;
                    break;
                }
                float2 _28670;
                if ((((in.i_flags.z == 0u) && (!((in.i_flags.x & _1911) != 0u))) && (!((in.i_flags.x & _1918) != 0u))) && all(in.i_radii.xyz == in.i_radii.yzw))
                {
                    float2 _13201 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
                    float2 _13210 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _13213 = in.i_local - _13201;
                    float _13725 = fast::min(_13210.x, _13210.y);
                    float _13728 = fast::min((_13213.x > 0.0) ? ((_13213.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_13213.y > 0.0) ? in.i_radii.w : in.i_radii.x), _13725);
                    float _13734 = _13728 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _28514;
                    float _28515;
                    if (_13734 > _13725)
                    {
                        float _13748 = in.i_shape.y * fast::clamp((_13725 - _13728) / fast::max(0.60000002384185791015625 * _13728, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _28515 = _13748;
                        _28514 = _13728 * (1.0 + (0.60000002384185791015625 * _13748));
                    }
                    else
                    {
                        _28515 = in.i_shape.y;
                        _28514 = _13734;
                    }
                    float2 _13777 = (abs((in.i_local + float2(0.5, 0.0)) - _13201) - _13210) + float2(_28514);
                    float2 _13779 = fast::max(_13777, float2(0.0));
                    float _28516;
                    if ((_13779.x > 0.0) && (_13779.y > 0.0))
                    {
                        float _28517;
                        if ((_28515 > 0.001000000047497451305389404296875) && (_28514 > 9.9999997473787516355514526367188e-05))
                        {
                            float _13796 = 2.0 + (2.0 * _28515);
                            float2 _13801 = _13779 / float2(fast::max(_28514, 9.9999997473787516355514526367188e-05));
                            _28517 = pow(pow(_13801.x, _13796) + pow(_13801.y, _13796), 1.0 / _13796) * _28514;
                        }
                        else
                        {
                            _28517 = length(_13779);
                        }
                        _28516 = _28517;
                    }
                    else
                    {
                        _28516 = fast::max(_13779.x, _13779.y);
                    }
                    float2 _13856 = (abs((in.i_local - float2(0.5, 0.0)) - _13201) - _13210) + float2(_28514);
                    float2 _13858 = fast::max(_13856, float2(0.0));
                    float _28518;
                    if ((_13858.x > 0.0) && (_13858.y > 0.0))
                    {
                        float _28519;
                        if ((_28515 > 0.001000000047497451305389404296875) && (_28514 > 9.9999997473787516355514526367188e-05))
                        {
                            float _13875 = 2.0 + (2.0 * _28515);
                            float2 _13880 = _13858 / float2(fast::max(_28514, 9.9999997473787516355514526367188e-05));
                            _28519 = pow(pow(_13880.x, _13875) + pow(_13880.y, _13875), 1.0 / _13875) * _28514;
                        }
                        else
                        {
                            _28519 = length(_13858);
                        }
                        _28518 = _28519;
                    }
                    else
                    {
                        _28518 = fast::max(_13858.x, _13858.y);
                    }
                    float2 _13935 = (abs((in.i_local + float2(0.0, 0.5)) - _13201) - _13210) + float2(_28514);
                    float2 _13937 = fast::max(_13935, float2(0.0));
                    float _28662;
                    if ((_13937.x > 0.0) && (_13937.y > 0.0))
                    {
                        float _28663;
                        if ((_28515 > 0.001000000047497451305389404296875) && (_28514 > 9.9999997473787516355514526367188e-05))
                        {
                            float _13954 = 2.0 + (2.0 * _28515);
                            float2 _13959 = _13937 / float2(fast::max(_28514, 9.9999997473787516355514526367188e-05));
                            _28663 = pow(pow(_13959.x, _13954) + pow(_13959.y, _13954), 1.0 / _13954) * _28514;
                        }
                        else
                        {
                            _28663 = length(_13937);
                        }
                        _28662 = _28663;
                    }
                    else
                    {
                        _28662 = fast::max(_13937.x, _13937.y);
                    }
                    float2 _14014 = (abs((in.i_local - float2(0.0, 0.5)) - _13201) - _13210) + float2(_28514);
                    float2 _14016 = fast::max(_14014, float2(0.0));
                    float _28664;
                    if ((_14016.x > 0.0) && (_14016.y > 0.0))
                    {
                        float _28665;
                        if ((_28515 > 0.001000000047497451305389404296875) && (_28514 > 9.9999997473787516355514526367188e-05))
                        {
                            float _14033 = 2.0 + (2.0 * _28515);
                            float2 _14038 = _14016 / float2(fast::max(_28514, 9.9999997473787516355514526367188e-05));
                            _28665 = pow(pow(_14038.x, _14033) + pow(_14038.y, _14033), 1.0 / _14033) * _28514;
                        }
                        else
                        {
                            _28665 = length(_14016);
                        }
                        _28664 = _28665;
                    }
                    else
                    {
                        _28664 = fast::max(_14016.x, _14016.y);
                    }
                    _28670 = float2(((fast::min(fast::max(_13777.x, _13777.y), 0.0) + _28516) - _28514) - ((fast::min(fast::max(_13856.x, _13856.y), 0.0) + _28518) - _28514), ((fast::min(fast::max(_13935.x, _13935.y), 0.0) + _28662) - _28514) - ((fast::min(fast::max(_14014.x, _14014.y), 0.0) + _28664) - _28514));
                }
                else
                {
                    float2 _13254 = in.i_local + float2(0.5, 0.0);
                    float2 _14118 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
                    float2 _14127 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float _28371;
                    if ((in.i_flags.x & _1753) != 0u)
                    {
                        float _14244 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                        float4 _14278 = gTex.sample(gLinear, mix(_27490.xy, _27490.zw, (fast::clamp(_13254, _27496.xy, _27496.zw) - _27496.xy) / fast::max(_27496.zw - _27496.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                        float _14302 = (_14278.w - 0.5) * in.i_radii.y;
                        float2 _14343 = abs(_13254 - ((_27496.xy + _27496.zw) * 0.5)) - fast::max(((_27496.zw - _27496.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _14244), float2(0.0));
                        _28371 = fast::max((-mix((fast::max(fast::min(_14278.x, _14278.y), fast::min(fast::max(_14278.x, _14278.y), _14278.z)) - 0.5) * in.i_radii.x, _14302, fast::clamp(abs(_14302) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _14244, length(fast::max(_14343, float2(0.0))) + fast::min(fast::max(_14343.x, _14343.y), 0.0)) - in.i_shape.w;
                    }
                    else
                    {
                        float _28372;
                        if ((in.i_flags.x & _1768) != 0u)
                        {
                            float _28370;
                            do
                            {
                                bool _14405 = in.i_radii.w > 0.5;
                                float _14409 = floor((in.i_radii.z + 1.0) * 0.5);
                                float _14413 = floor((_14409 + 127.0) * 0.0078125);
                                float _14417 = floor((in.i_radii.z - 1.0) * 0.5);
                                float _14545 = floor(_14417 * 0.0625);
                                uint _14562 = (((in.i_instance + 1u) + uint(_14545)) * 24u) + uint(_14417 - (_14545 * 16.0));
                                float2 _28347;
                                if (_14405)
                                {
                                    float _14572 = floor(0.0);
                                    _28347 = gFxData_1._data[(((in.i_instance + 1u) + uint(_14572)) * 24u) + uint(_14572 * (-16.0))].xy;
                                }
                                else
                                {
                                    _28347 = select(gFxData_1._data[_14562].zw, gFxData_1._data[_14562].xy, bool2(((in.i_radii.z - 1.0) - (_14417 * 2.0)) < 0.5));
                                }
                                float2 _28346;
                                float _28349;
                                float _28352;
                                _28352 = 1.0;
                                _28349 = 1000000015047466219876688855040.0;
                                _28346 = _28347;
                                float2 _28345;
                                float _28348;
                                float _28351;
                                for (float _28342 = 0.0; _28342 < 16.0; _28352 = _28351, _28349 = _28348, _28346 = _28345, _28342 += 1.0)
                                {
                                    if (_28342 >= _14413)
                                    {
                                        break;
                                    }
                                    _28351 = _28352;
                                    _28348 = _28349;
                                    _28345 = _28346;
                                    float2 _32423;
                                    float _32425;
                                    float _32427;
                                    for (float _28343 = 0.0; _28343 < 128.0; _28351 = _32427, _28348 = _32425, _28345 = _32423, _28343 += 1.0)
                                    {
                                        float _14459 = (_28342 * 128.0) + _28343;
                                        if (_14459 >= _14409)
                                        {
                                            break;
                                        }
                                        float _14599 = floor(_14459 * 0.0625);
                                        uint _14616 = (((in.i_instance + 1u) + uint(_14599)) * 24u) + uint(_14459 - (_14599 * 16.0));
                                        float2 _14629 = _28345 - gFxData_1._data[_14616].xy;
                                        float2 _14632 = _13254 - gFxData_1._data[_14616].xy;
                                        float2 _14645 = _14632 - (_14629 * fast::clamp(dot(_14632, _14629) / fast::max(dot(_14629, _14629), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _14648 = dot(_14645, _14645);
                                        float _32529 = (_14648 < _28348) ? _14648 : _28348;
                                        bool _14660 = _13254.y >= gFxData_1._data[_14616].y;
                                        bool _14665 = _13254.y < _28345.y;
                                        bool _14676 = (_14629.x * _14632.y) > (_14629.y * _14632.x);
                                        float _28357;
                                        if (((_14660 && _14665) && _14676) || (((!_14660) && (!_14665)) && (!_14676)))
                                        {
                                            _28357 = -_28351;
                                        }
                                        else
                                        {
                                            _28357 = _28351;
                                        }
                                        if (((2.0 * _14459) + 1.0) < in.i_radii.z)
                                        {
                                            float2 _14705 = gFxData_1._data[_14616].xy - gFxData_1._data[_14616].zw;
                                            float2 _14708 = _13254 - gFxData_1._data[_14616].zw;
                                            float2 _14721 = _14708 - (_14705 * fast::clamp(dot(_14708, _14705) / fast::max(dot(_14705, _14705), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _14724 = dot(_14721, _14721);
                                            bool _14736 = _13254.y >= gFxData_1._data[_14616].w;
                                            bool _14741 = _13254.y < gFxData_1._data[_14616].y;
                                            bool _14752 = (_14705.x * _14708.y) > (_14705.y * _14708.x);
                                            float _28361;
                                            if (((_14736 && _14741) && _14752) || (((!_14736) && (!_14741)) && (!_14752)))
                                            {
                                                _28361 = -_28357;
                                            }
                                            else
                                            {
                                                _28361 = _28357;
                                            }
                                            _32427 = _28361;
                                            _32425 = (_14724 < _32529) ? _14724 : _32529;
                                            _32423 = gFxData_1._data[_14616].zw;
                                        }
                                        else
                                        {
                                            _32427 = _28357;
                                            _32425 = _32529;
                                            _32423 = gFxData_1._data[_14616].xy;
                                        }
                                    }
                                }
                                float _14511 = sqrt(_28349);
                                if (_14405)
                                {
                                    _28370 = _14511 - in.i_radii.x;
                                    break;
                                }
                                _28370 = (_28352 * _14511) - in.i_radii.x;
                                break;
                            } while(false);
                            _28372 = _28370;
                        }
                        else
                        {
                            float _28373;
                            if (in.i_flags.z == 1u)
                            {
                                float2 _14152 = _13254 - _14118;
                                float _28341;
                                do
                                {
                                    if (in.i_shape.w >= 6.282185077667236328125)
                                    {
                                        _28341 = abs(length(_14152) - in.i_radii.x) - in.i_radii.y;
                                        break;
                                    }
                                    float _14795 = in.i_shape.z + (in.i_shape.w * 0.5);
                                    float _14797 = cos(_14795);
                                    float _14799 = sin(_14795);
                                    float _14808 = dot(_14152, float2(-_14799, _14797));
                                    float _14811 = dot(_14152, float2(_14797, _14799));
                                    float2 _14812 = float2(_14808, _14811);
                                    float _14815 = abs(_14808);
                                    _14812.x = _14815;
                                    float _14818 = in.i_shape.w * 0.5;
                                    float _14820 = sin(_14818);
                                    float _14822 = cos(_14818);
                                    _28341 = (((_14822 * _14815) > (_14820 * _14811)) ? length(_14812 - (float2(_14820, _14822) * in.i_radii.x)) : abs(length(_14812) - in.i_radii.x)) - in.i_radii.y;
                                    break;
                                } while(false);
                                _28373 = _28341;
                            }
                            else
                            {
                                float _28374;
                                if (in.i_flags.z == 2u)
                                {
                                    float2 _14858 = _13254 - _27496.xy;
                                    float2 _14861 = _27496.zw - _27496.xy;
                                    _28374 = length(_14858 - (_14861 * fast::clamp(dot(_14858, _14861) / fast::max(dot(_14861, _14861), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                                }
                                else
                                {
                                    float2 _14179 = _13254 - _14118;
                                    float _14926 = fast::min(_14127.x, _14127.y);
                                    float _14929 = fast::min((_14179.x > 0.0) ? ((_14179.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_14179.y > 0.0) ? in.i_radii.w : in.i_radii.x), _14926);
                                    float _14935 = _14929 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                    float _28330;
                                    float _28331;
                                    if (_14935 > _14926)
                                    {
                                        float _14949 = in.i_shape.y * fast::clamp((_14926 - _14929) / fast::max(0.60000002384185791015625 * _14929, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                        _28331 = _14949;
                                        _28330 = _14929 * (1.0 + (0.60000002384185791015625 * _14949));
                                    }
                                    else
                                    {
                                        _28331 = in.i_shape.y;
                                        _28330 = _14935;
                                    }
                                    float2 _14978 = (abs(_14179) - _14127) + float2(_28330);
                                    float2 _14980 = fast::max(_14978, float2(0.0));
                                    float _28332;
                                    if ((_14980.x > 0.0) && (_14980.y > 0.0))
                                    {
                                        float _28333;
                                        if ((_28331 > 0.001000000047497451305389404296875) && (_28330 > 9.9999997473787516355514526367188e-05))
                                        {
                                            float _14997 = 2.0 + (2.0 * _28331);
                                            float2 _15002 = _14980 / float2(fast::max(_28330, 9.9999997473787516355514526367188e-05));
                                            _28333 = pow(pow(_15002.x, _14997) + pow(_15002.y, _14997), 1.0 / _14997) * _28330;
                                        }
                                        else
                                        {
                                            _28333 = length(_14980);
                                        }
                                        _28332 = _28333;
                                    }
                                    else
                                    {
                                        _28332 = fast::max(_14980.x, _14980.y);
                                    }
                                    float _15037 = (fast::min(fast::max(_14978.x, _14978.y), 0.0) + _28332) - _28330;
                                    float _28375;
                                    if ((in.i_flags.x & _1839) != 0u)
                                    {
                                        float2 _14207 = fast::max((_27496.zw - _27496.xy) * 0.5, float2(0.001000000047497451305389404296875));
                                        float2 _14210 = _13254 - ((_27496.xy + _27496.zw) * 0.5);
                                        float _15085 = fast::min(_14207.x, _14207.y);
                                        float _15088 = fast::min((_14210.x > 0.0) ? ((_14210.y > 0.0) ? gFxData_1._data[_12612].x : gFxData_1._data[_12612].x) : ((_14210.y > 0.0) ? gFxData_1._data[_12612].x : gFxData_1._data[_12612].x), _15085);
                                        float _15094 = _15088 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                        float _28334;
                                        float _28335;
                                        if (_15094 > _15085)
                                        {
                                            float _15108 = in.i_shape.y * fast::clamp((_15085 - _15088) / fast::max(0.60000002384185791015625 * _15088, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                            _28335 = _15108;
                                            _28334 = _15088 * (1.0 + (0.60000002384185791015625 * _15108));
                                        }
                                        else
                                        {
                                            _28335 = in.i_shape.y;
                                            _28334 = _15094;
                                        }
                                        float2 _15137 = (abs(_14210) - _14207) + float2(_28334);
                                        float2 _15139 = fast::max(_15137, float2(0.0));
                                        float _28336;
                                        if ((_15139.x > 0.0) && (_15139.y > 0.0))
                                        {
                                            float _28337;
                                            if ((_28335 > 0.001000000047497451305389404296875) && (_28334 > 9.9999997473787516355514526367188e-05))
                                            {
                                                float _15156 = 2.0 + (2.0 * _28335);
                                                float2 _15161 = _15139 / float2(fast::max(_28334, 9.9999997473787516355514526367188e-05));
                                                _28337 = pow(pow(_15161.x, _15156) + pow(_15161.y, _15156), 1.0 / _15156) * _28334;
                                            }
                                            else
                                            {
                                                _28337 = length(_15139);
                                            }
                                            _28336 = _28337;
                                        }
                                        else
                                        {
                                            _28336 = fast::max(_15139.x, _15139.y);
                                        }
                                        float _15196 = (fast::min(fast::max(_15137.x, _15137.y), 0.0) + _28336) - _28334;
                                        float _15201 = fast::max(gFxData_1._data[_12612].y, 9.9999997473787516355514526367188e-05);
                                        float _15210 = fast::max(_15201 - abs(_15037 - _15196), 0.0) / _15201;
                                        _28375 = fast::min(_15037, _15196) - (((_15210 * _15210) * _15201) * 0.25);
                                    }
                                    else
                                    {
                                        _28375 = _15037;
                                    }
                                    _28374 = _28375;
                                }
                                _28373 = _28374;
                            }
                            _28372 = _28373;
                        }
                        _28371 = _28372;
                    }
                    float2 _13258 = in.i_local - float2(0.5, 0.0);
                    float2 _15265 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
                    float2 _15274 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float _28417;
                    if ((in.i_flags.x & _1753) != 0u)
                    {
                        float _15391 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                        float4 _15425 = gTex.sample(gLinear, mix(_27490.xy, _27490.zw, (fast::clamp(_13258, _27496.xy, _27496.zw) - _27496.xy) / fast::max(_27496.zw - _27496.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                        float _15449 = (_15425.w - 0.5) * in.i_radii.y;
                        float2 _15490 = abs(_13258 - ((_27496.xy + _27496.zw) * 0.5)) - fast::max(((_27496.zw - _27496.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _15391), float2(0.0));
                        _28417 = fast::max((-mix((fast::max(fast::min(_15425.x, _15425.y), fast::min(fast::max(_15425.x, _15425.y), _15425.z)) - 0.5) * in.i_radii.x, _15449, fast::clamp(abs(_15449) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _15391, length(fast::max(_15490, float2(0.0))) + fast::min(fast::max(_15490.x, _15490.y), 0.0)) - in.i_shape.w;
                    }
                    else
                    {
                        float _28418;
                        if ((in.i_flags.x & _1768) != 0u)
                        {
                            float _28416;
                            do
                            {
                                bool _15552 = in.i_radii.w > 0.5;
                                float _15556 = floor((in.i_radii.z + 1.0) * 0.5);
                                float _15560 = floor((_15556 + 127.0) * 0.0078125);
                                float _15564 = floor((in.i_radii.z - 1.0) * 0.5);
                                float _15692 = floor(_15564 * 0.0625);
                                uint _15709 = (((in.i_instance + 1u) + uint(_15692)) * 24u) + uint(_15564 - (_15692 * 16.0));
                                float2 _28393;
                                if (_15552)
                                {
                                    float _15719 = floor(0.0);
                                    _28393 = gFxData_1._data[(((in.i_instance + 1u) + uint(_15719)) * 24u) + uint(_15719 * (-16.0))].xy;
                                }
                                else
                                {
                                    _28393 = select(gFxData_1._data[_15709].zw, gFxData_1._data[_15709].xy, bool2(((in.i_radii.z - 1.0) - (_15564 * 2.0)) < 0.5));
                                }
                                float2 _28392;
                                float _28395;
                                float _28398;
                                _28398 = 1.0;
                                _28395 = 1000000015047466219876688855040.0;
                                _28392 = _28393;
                                float2 _28391;
                                float _28394;
                                float _28397;
                                for (float _28388 = 0.0; _28388 < 16.0; _28398 = _28397, _28395 = _28394, _28392 = _28391, _28388 += 1.0)
                                {
                                    if (_28388 >= _15560)
                                    {
                                        break;
                                    }
                                    _28397 = _28398;
                                    _28394 = _28395;
                                    _28391 = _28392;
                                    float2 _32434;
                                    float _32436;
                                    float _32438;
                                    for (float _28389 = 0.0; _28389 < 128.0; _28397 = _32438, _28394 = _32436, _28391 = _32434, _28389 += 1.0)
                                    {
                                        float _15606 = (_28388 * 128.0) + _28389;
                                        if (_15606 >= _15556)
                                        {
                                            break;
                                        }
                                        float _15746 = floor(_15606 * 0.0625);
                                        uint _15763 = (((in.i_instance + 1u) + uint(_15746)) * 24u) + uint(_15606 - (_15746 * 16.0));
                                        float2 _15776 = _28391 - gFxData_1._data[_15763].xy;
                                        float2 _15779 = _13258 - gFxData_1._data[_15763].xy;
                                        float2 _15792 = _15779 - (_15776 * fast::clamp(dot(_15779, _15776) / fast::max(dot(_15776, _15776), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _15795 = dot(_15792, _15792);
                                        float _32531 = (_15795 < _28394) ? _15795 : _28394;
                                        bool _15807 = _13258.y >= gFxData_1._data[_15763].y;
                                        bool _15812 = _13258.y < _28391.y;
                                        bool _15823 = (_15776.x * _15779.y) > (_15776.y * _15779.x);
                                        float _28403;
                                        if (((_15807 && _15812) && _15823) || (((!_15807) && (!_15812)) && (!_15823)))
                                        {
                                            _28403 = -_28397;
                                        }
                                        else
                                        {
                                            _28403 = _28397;
                                        }
                                        if (((2.0 * _15606) + 1.0) < in.i_radii.z)
                                        {
                                            float2 _15852 = gFxData_1._data[_15763].xy - gFxData_1._data[_15763].zw;
                                            float2 _15855 = _13258 - gFxData_1._data[_15763].zw;
                                            float2 _15868 = _15855 - (_15852 * fast::clamp(dot(_15855, _15852) / fast::max(dot(_15852, _15852), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _15871 = dot(_15868, _15868);
                                            bool _15883 = _13258.y >= gFxData_1._data[_15763].w;
                                            bool _15888 = _13258.y < gFxData_1._data[_15763].y;
                                            bool _15899 = (_15852.x * _15855.y) > (_15852.y * _15855.x);
                                            float _28407;
                                            if (((_15883 && _15888) && _15899) || (((!_15883) && (!_15888)) && (!_15899)))
                                            {
                                                _28407 = -_28403;
                                            }
                                            else
                                            {
                                                _28407 = _28403;
                                            }
                                            _32438 = _28407;
                                            _32436 = (_15871 < _32531) ? _15871 : _32531;
                                            _32434 = gFxData_1._data[_15763].zw;
                                        }
                                        else
                                        {
                                            _32438 = _28403;
                                            _32436 = _32531;
                                            _32434 = gFxData_1._data[_15763].xy;
                                        }
                                    }
                                }
                                float _15658 = sqrt(_28395);
                                if (_15552)
                                {
                                    _28416 = _15658 - in.i_radii.x;
                                    break;
                                }
                                _28416 = (_28398 * _15658) - in.i_radii.x;
                                break;
                            } while(false);
                            _28418 = _28416;
                        }
                        else
                        {
                            float _28419;
                            if (in.i_flags.z == 1u)
                            {
                                float2 _15299 = _13258 - _15265;
                                float _28387;
                                do
                                {
                                    if (in.i_shape.w >= 6.282185077667236328125)
                                    {
                                        _28387 = abs(length(_15299) - in.i_radii.x) - in.i_radii.y;
                                        break;
                                    }
                                    float _15942 = in.i_shape.z + (in.i_shape.w * 0.5);
                                    float _15944 = cos(_15942);
                                    float _15946 = sin(_15942);
                                    float _15955 = dot(_15299, float2(-_15946, _15944));
                                    float _15958 = dot(_15299, float2(_15944, _15946));
                                    float2 _15959 = float2(_15955, _15958);
                                    float _15962 = abs(_15955);
                                    _15959.x = _15962;
                                    float _15965 = in.i_shape.w * 0.5;
                                    float _15967 = sin(_15965);
                                    float _15969 = cos(_15965);
                                    _28387 = (((_15969 * _15962) > (_15967 * _15958)) ? length(_15959 - (float2(_15967, _15969) * in.i_radii.x)) : abs(length(_15959) - in.i_radii.x)) - in.i_radii.y;
                                    break;
                                } while(false);
                                _28419 = _28387;
                            }
                            else
                            {
                                float _28420;
                                if (in.i_flags.z == 2u)
                                {
                                    float2 _16005 = _13258 - _27496.xy;
                                    float2 _16008 = _27496.zw - _27496.xy;
                                    _28420 = length(_16005 - (_16008 * fast::clamp(dot(_16005, _16008) / fast::max(dot(_16008, _16008), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                                }
                                else
                                {
                                    float2 _15326 = _13258 - _15265;
                                    float _16073 = fast::min(_15274.x, _15274.y);
                                    float _16076 = fast::min((_15326.x > 0.0) ? ((_15326.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_15326.y > 0.0) ? in.i_radii.w : in.i_radii.x), _16073);
                                    float _16082 = _16076 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                    float _28376;
                                    float _28377;
                                    if (_16082 > _16073)
                                    {
                                        float _16096 = in.i_shape.y * fast::clamp((_16073 - _16076) / fast::max(0.60000002384185791015625 * _16076, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                        _28377 = _16096;
                                        _28376 = _16076 * (1.0 + (0.60000002384185791015625 * _16096));
                                    }
                                    else
                                    {
                                        _28377 = in.i_shape.y;
                                        _28376 = _16082;
                                    }
                                    float2 _16125 = (abs(_15326) - _15274) + float2(_28376);
                                    float2 _16127 = fast::max(_16125, float2(0.0));
                                    float _28378;
                                    if ((_16127.x > 0.0) && (_16127.y > 0.0))
                                    {
                                        float _28379;
                                        if ((_28377 > 0.001000000047497451305389404296875) && (_28376 > 9.9999997473787516355514526367188e-05))
                                        {
                                            float _16144 = 2.0 + (2.0 * _28377);
                                            float2 _16149 = _16127 / float2(fast::max(_28376, 9.9999997473787516355514526367188e-05));
                                            _28379 = pow(pow(_16149.x, _16144) + pow(_16149.y, _16144), 1.0 / _16144) * _28376;
                                        }
                                        else
                                        {
                                            _28379 = length(_16127);
                                        }
                                        _28378 = _28379;
                                    }
                                    else
                                    {
                                        _28378 = fast::max(_16127.x, _16127.y);
                                    }
                                    float _16184 = (fast::min(fast::max(_16125.x, _16125.y), 0.0) + _28378) - _28376;
                                    float _28421;
                                    if ((in.i_flags.x & _1839) != 0u)
                                    {
                                        float2 _15354 = fast::max((_27496.zw - _27496.xy) * 0.5, float2(0.001000000047497451305389404296875));
                                        float2 _15357 = _13258 - ((_27496.xy + _27496.zw) * 0.5);
                                        float _16232 = fast::min(_15354.x, _15354.y);
                                        float _16235 = fast::min((_15357.x > 0.0) ? ((_15357.y > 0.0) ? gFxData_1._data[_12612].x : gFxData_1._data[_12612].x) : ((_15357.y > 0.0) ? gFxData_1._data[_12612].x : gFxData_1._data[_12612].x), _16232);
                                        float _16241 = _16235 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                        float _28380;
                                        float _28381;
                                        if (_16241 > _16232)
                                        {
                                            float _16255 = in.i_shape.y * fast::clamp((_16232 - _16235) / fast::max(0.60000002384185791015625 * _16235, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                            _28381 = _16255;
                                            _28380 = _16235 * (1.0 + (0.60000002384185791015625 * _16255));
                                        }
                                        else
                                        {
                                            _28381 = in.i_shape.y;
                                            _28380 = _16241;
                                        }
                                        float2 _16284 = (abs(_15357) - _15354) + float2(_28380);
                                        float2 _16286 = fast::max(_16284, float2(0.0));
                                        float _28382;
                                        if ((_16286.x > 0.0) && (_16286.y > 0.0))
                                        {
                                            float _28383;
                                            if ((_28381 > 0.001000000047497451305389404296875) && (_28380 > 9.9999997473787516355514526367188e-05))
                                            {
                                                float _16303 = 2.0 + (2.0 * _28381);
                                                float2 _16308 = _16286 / float2(fast::max(_28380, 9.9999997473787516355514526367188e-05));
                                                _28383 = pow(pow(_16308.x, _16303) + pow(_16308.y, _16303), 1.0 / _16303) * _28380;
                                            }
                                            else
                                            {
                                                _28383 = length(_16286);
                                            }
                                            _28382 = _28383;
                                        }
                                        else
                                        {
                                            _28382 = fast::max(_16286.x, _16286.y);
                                        }
                                        float _16343 = (fast::min(fast::max(_16284.x, _16284.y), 0.0) + _28382) - _28380;
                                        float _16348 = fast::max(gFxData_1._data[_12612].y, 9.9999997473787516355514526367188e-05);
                                        float _16357 = fast::max(_16348 - abs(_16184 - _16343), 0.0) / _16348;
                                        _28421 = fast::min(_16184, _16343) - (((_16357 * _16357) * _16348) * 0.25);
                                    }
                                    else
                                    {
                                        _28421 = _16184;
                                    }
                                    _28420 = _28421;
                                }
                                _28419 = _28420;
                            }
                            _28418 = _28419;
                        }
                        _28417 = _28418;
                    }
                    float2 _13263 = in.i_local + float2(0.0, 0.5);
                    float2 _16412 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
                    float2 _16421 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float _28463;
                    if ((in.i_flags.x & _1753) != 0u)
                    {
                        float _16538 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                        float4 _16572 = gTex.sample(gLinear, mix(_27490.xy, _27490.zw, (fast::clamp(_13263, _27496.xy, _27496.zw) - _27496.xy) / fast::max(_27496.zw - _27496.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                        float _16596 = (_16572.w - 0.5) * in.i_radii.y;
                        float2 _16637 = abs(_13263 - ((_27496.xy + _27496.zw) * 0.5)) - fast::max(((_27496.zw - _27496.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _16538), float2(0.0));
                        _28463 = fast::max((-mix((fast::max(fast::min(_16572.x, _16572.y), fast::min(fast::max(_16572.x, _16572.y), _16572.z)) - 0.5) * in.i_radii.x, _16596, fast::clamp(abs(_16596) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _16538, length(fast::max(_16637, float2(0.0))) + fast::min(fast::max(_16637.x, _16637.y), 0.0)) - in.i_shape.w;
                    }
                    else
                    {
                        float _28464;
                        if ((in.i_flags.x & _1768) != 0u)
                        {
                            float _28462;
                            do
                            {
                                bool _16699 = in.i_radii.w > 0.5;
                                float _16703 = floor((in.i_radii.z + 1.0) * 0.5);
                                float _16707 = floor((_16703 + 127.0) * 0.0078125);
                                float _16711 = floor((in.i_radii.z - 1.0) * 0.5);
                                float _16839 = floor(_16711 * 0.0625);
                                uint _16856 = (((in.i_instance + 1u) + uint(_16839)) * 24u) + uint(_16711 - (_16839 * 16.0));
                                float2 _28439;
                                if (_16699)
                                {
                                    float _16866 = floor(0.0);
                                    _28439 = gFxData_1._data[(((in.i_instance + 1u) + uint(_16866)) * 24u) + uint(_16866 * (-16.0))].xy;
                                }
                                else
                                {
                                    _28439 = select(gFxData_1._data[_16856].zw, gFxData_1._data[_16856].xy, bool2(((in.i_radii.z - 1.0) - (_16711 * 2.0)) < 0.5));
                                }
                                float2 _28438;
                                float _28441;
                                float _28444;
                                _28444 = 1.0;
                                _28441 = 1000000015047466219876688855040.0;
                                _28438 = _28439;
                                float2 _28437;
                                float _28440;
                                float _28443;
                                for (float _28434 = 0.0; _28434 < 16.0; _28444 = _28443, _28441 = _28440, _28438 = _28437, _28434 += 1.0)
                                {
                                    if (_28434 >= _16707)
                                    {
                                        break;
                                    }
                                    _28443 = _28444;
                                    _28440 = _28441;
                                    _28437 = _28438;
                                    float2 _32445;
                                    float _32447;
                                    float _32449;
                                    for (float _28435 = 0.0; _28435 < 128.0; _28443 = _32449, _28440 = _32447, _28437 = _32445, _28435 += 1.0)
                                    {
                                        float _16753 = (_28434 * 128.0) + _28435;
                                        if (_16753 >= _16703)
                                        {
                                            break;
                                        }
                                        float _16893 = floor(_16753 * 0.0625);
                                        uint _16910 = (((in.i_instance + 1u) + uint(_16893)) * 24u) + uint(_16753 - (_16893 * 16.0));
                                        float2 _16923 = _28437 - gFxData_1._data[_16910].xy;
                                        float2 _16926 = _13263 - gFxData_1._data[_16910].xy;
                                        float2 _16939 = _16926 - (_16923 * fast::clamp(dot(_16926, _16923) / fast::max(dot(_16923, _16923), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _16942 = dot(_16939, _16939);
                                        float _32533 = (_16942 < _28440) ? _16942 : _28440;
                                        bool _16954 = _13263.y >= gFxData_1._data[_16910].y;
                                        bool _16959 = _13263.y < _28437.y;
                                        bool _16970 = (_16923.x * _16926.y) > (_16923.y * _16926.x);
                                        float _28449;
                                        if (((_16954 && _16959) && _16970) || (((!_16954) && (!_16959)) && (!_16970)))
                                        {
                                            _28449 = -_28443;
                                        }
                                        else
                                        {
                                            _28449 = _28443;
                                        }
                                        if (((2.0 * _16753) + 1.0) < in.i_radii.z)
                                        {
                                            float2 _16999 = gFxData_1._data[_16910].xy - gFxData_1._data[_16910].zw;
                                            float2 _17002 = _13263 - gFxData_1._data[_16910].zw;
                                            float2 _17015 = _17002 - (_16999 * fast::clamp(dot(_17002, _16999) / fast::max(dot(_16999, _16999), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _17018 = dot(_17015, _17015);
                                            bool _17030 = _13263.y >= gFxData_1._data[_16910].w;
                                            bool _17035 = _13263.y < gFxData_1._data[_16910].y;
                                            bool _17046 = (_16999.x * _17002.y) > (_16999.y * _17002.x);
                                            float _28453;
                                            if (((_17030 && _17035) && _17046) || (((!_17030) && (!_17035)) && (!_17046)))
                                            {
                                                _28453 = -_28449;
                                            }
                                            else
                                            {
                                                _28453 = _28449;
                                            }
                                            _32449 = _28453;
                                            _32447 = (_17018 < _32533) ? _17018 : _32533;
                                            _32445 = gFxData_1._data[_16910].zw;
                                        }
                                        else
                                        {
                                            _32449 = _28449;
                                            _32447 = _32533;
                                            _32445 = gFxData_1._data[_16910].xy;
                                        }
                                    }
                                }
                                float _16805 = sqrt(_28441);
                                if (_16699)
                                {
                                    _28462 = _16805 - in.i_radii.x;
                                    break;
                                }
                                _28462 = (_28444 * _16805) - in.i_radii.x;
                                break;
                            } while(false);
                            _28464 = _28462;
                        }
                        else
                        {
                            float _28465;
                            if (in.i_flags.z == 1u)
                            {
                                float2 _16446 = _13263 - _16412;
                                float _28433;
                                do
                                {
                                    if (in.i_shape.w >= 6.282185077667236328125)
                                    {
                                        _28433 = abs(length(_16446) - in.i_radii.x) - in.i_radii.y;
                                        break;
                                    }
                                    float _17089 = in.i_shape.z + (in.i_shape.w * 0.5);
                                    float _17091 = cos(_17089);
                                    float _17093 = sin(_17089);
                                    float _17102 = dot(_16446, float2(-_17093, _17091));
                                    float _17105 = dot(_16446, float2(_17091, _17093));
                                    float2 _17106 = float2(_17102, _17105);
                                    float _17109 = abs(_17102);
                                    _17106.x = _17109;
                                    float _17112 = in.i_shape.w * 0.5;
                                    float _17114 = sin(_17112);
                                    float _17116 = cos(_17112);
                                    _28433 = (((_17116 * _17109) > (_17114 * _17105)) ? length(_17106 - (float2(_17114, _17116) * in.i_radii.x)) : abs(length(_17106) - in.i_radii.x)) - in.i_radii.y;
                                    break;
                                } while(false);
                                _28465 = _28433;
                            }
                            else
                            {
                                float _28466;
                                if (in.i_flags.z == 2u)
                                {
                                    float2 _17152 = _13263 - _27496.xy;
                                    float2 _17155 = _27496.zw - _27496.xy;
                                    _28466 = length(_17152 - (_17155 * fast::clamp(dot(_17152, _17155) / fast::max(dot(_17155, _17155), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                                }
                                else
                                {
                                    float2 _16473 = _13263 - _16412;
                                    float _17220 = fast::min(_16421.x, _16421.y);
                                    float _17223 = fast::min((_16473.x > 0.0) ? ((_16473.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_16473.y > 0.0) ? in.i_radii.w : in.i_radii.x), _17220);
                                    float _17229 = _17223 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                    float _28422;
                                    float _28423;
                                    if (_17229 > _17220)
                                    {
                                        float _17243 = in.i_shape.y * fast::clamp((_17220 - _17223) / fast::max(0.60000002384185791015625 * _17223, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                        _28423 = _17243;
                                        _28422 = _17223 * (1.0 + (0.60000002384185791015625 * _17243));
                                    }
                                    else
                                    {
                                        _28423 = in.i_shape.y;
                                        _28422 = _17229;
                                    }
                                    float2 _17272 = (abs(_16473) - _16421) + float2(_28422);
                                    float2 _17274 = fast::max(_17272, float2(0.0));
                                    float _28424;
                                    if ((_17274.x > 0.0) && (_17274.y > 0.0))
                                    {
                                        float _28425;
                                        if ((_28423 > 0.001000000047497451305389404296875) && (_28422 > 9.9999997473787516355514526367188e-05))
                                        {
                                            float _17291 = 2.0 + (2.0 * _28423);
                                            float2 _17296 = _17274 / float2(fast::max(_28422, 9.9999997473787516355514526367188e-05));
                                            _28425 = pow(pow(_17296.x, _17291) + pow(_17296.y, _17291), 1.0 / _17291) * _28422;
                                        }
                                        else
                                        {
                                            _28425 = length(_17274);
                                        }
                                        _28424 = _28425;
                                    }
                                    else
                                    {
                                        _28424 = fast::max(_17274.x, _17274.y);
                                    }
                                    float _17331 = (fast::min(fast::max(_17272.x, _17272.y), 0.0) + _28424) - _28422;
                                    float _28467;
                                    if ((in.i_flags.x & _1839) != 0u)
                                    {
                                        float2 _16501 = fast::max((_27496.zw - _27496.xy) * 0.5, float2(0.001000000047497451305389404296875));
                                        float2 _16504 = _13263 - ((_27496.xy + _27496.zw) * 0.5);
                                        float _17379 = fast::min(_16501.x, _16501.y);
                                        float _17382 = fast::min((_16504.x > 0.0) ? ((_16504.y > 0.0) ? gFxData_1._data[_12612].x : gFxData_1._data[_12612].x) : ((_16504.y > 0.0) ? gFxData_1._data[_12612].x : gFxData_1._data[_12612].x), _17379);
                                        float _17388 = _17382 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                        float _28426;
                                        float _28427;
                                        if (_17388 > _17379)
                                        {
                                            float _17402 = in.i_shape.y * fast::clamp((_17379 - _17382) / fast::max(0.60000002384185791015625 * _17382, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                            _28427 = _17402;
                                            _28426 = _17382 * (1.0 + (0.60000002384185791015625 * _17402));
                                        }
                                        else
                                        {
                                            _28427 = in.i_shape.y;
                                            _28426 = _17388;
                                        }
                                        float2 _17431 = (abs(_16504) - _16501) + float2(_28426);
                                        float2 _17433 = fast::max(_17431, float2(0.0));
                                        float _28428;
                                        if ((_17433.x > 0.0) && (_17433.y > 0.0))
                                        {
                                            float _28429;
                                            if ((_28427 > 0.001000000047497451305389404296875) && (_28426 > 9.9999997473787516355514526367188e-05))
                                            {
                                                float _17450 = 2.0 + (2.0 * _28427);
                                                float2 _17455 = _17433 / float2(fast::max(_28426, 9.9999997473787516355514526367188e-05));
                                                _28429 = pow(pow(_17455.x, _17450) + pow(_17455.y, _17450), 1.0 / _17450) * _28426;
                                            }
                                            else
                                            {
                                                _28429 = length(_17433);
                                            }
                                            _28428 = _28429;
                                        }
                                        else
                                        {
                                            _28428 = fast::max(_17433.x, _17433.y);
                                        }
                                        float _17490 = (fast::min(fast::max(_17431.x, _17431.y), 0.0) + _28428) - _28426;
                                        float _17495 = fast::max(gFxData_1._data[_12612].y, 9.9999997473787516355514526367188e-05);
                                        float _17504 = fast::max(_17495 - abs(_17331 - _17490), 0.0) / _17495;
                                        _28467 = fast::min(_17331, _17490) - (((_17504 * _17504) * _17495) * 0.25);
                                    }
                                    else
                                    {
                                        _28467 = _17331;
                                    }
                                    _28466 = _28467;
                                }
                                _28465 = _28466;
                            }
                            _28464 = _28465;
                        }
                        _28463 = _28464;
                    }
                    float2 _13267 = in.i_local - float2(0.0, 0.5);
                    float2 _17559 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
                    float2 _17568 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float _28509;
                    if ((in.i_flags.x & _1753) != 0u)
                    {
                        float _17685 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                        float4 _17719 = gTex.sample(gLinear, mix(_27490.xy, _27490.zw, (fast::clamp(_13267, _27496.xy, _27496.zw) - _27496.xy) / fast::max(_27496.zw - _27496.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                        float _17743 = (_17719.w - 0.5) * in.i_radii.y;
                        float2 _17784 = abs(_13267 - ((_27496.xy + _27496.zw) * 0.5)) - fast::max(((_27496.zw - _27496.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _17685), float2(0.0));
                        _28509 = fast::max((-mix((fast::max(fast::min(_17719.x, _17719.y), fast::min(fast::max(_17719.x, _17719.y), _17719.z)) - 0.5) * in.i_radii.x, _17743, fast::clamp(abs(_17743) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _17685, length(fast::max(_17784, float2(0.0))) + fast::min(fast::max(_17784.x, _17784.y), 0.0)) - in.i_shape.w;
                    }
                    else
                    {
                        float _28510;
                        if ((in.i_flags.x & _1768) != 0u)
                        {
                            float _28508;
                            do
                            {
                                bool _17846 = in.i_radii.w > 0.5;
                                float _17850 = floor((in.i_radii.z + 1.0) * 0.5);
                                float _17854 = floor((_17850 + 127.0) * 0.0078125);
                                float _17858 = floor((in.i_radii.z - 1.0) * 0.5);
                                float _17986 = floor(_17858 * 0.0625);
                                uint _18003 = (((in.i_instance + 1u) + uint(_17986)) * 24u) + uint(_17858 - (_17986 * 16.0));
                                float2 _28485;
                                if (_17846)
                                {
                                    float _18013 = floor(0.0);
                                    _28485 = gFxData_1._data[(((in.i_instance + 1u) + uint(_18013)) * 24u) + uint(_18013 * (-16.0))].xy;
                                }
                                else
                                {
                                    _28485 = select(gFxData_1._data[_18003].zw, gFxData_1._data[_18003].xy, bool2(((in.i_radii.z - 1.0) - (_17858 * 2.0)) < 0.5));
                                }
                                float2 _28484;
                                float _28487;
                                float _28490;
                                _28490 = 1.0;
                                _28487 = 1000000015047466219876688855040.0;
                                _28484 = _28485;
                                float2 _28483;
                                float _28486;
                                float _28489;
                                for (float _28480 = 0.0; _28480 < 16.0; _28490 = _28489, _28487 = _28486, _28484 = _28483, _28480 += 1.0)
                                {
                                    if (_28480 >= _17854)
                                    {
                                        break;
                                    }
                                    _28489 = _28490;
                                    _28486 = _28487;
                                    _28483 = _28484;
                                    float2 _32456;
                                    float _32458;
                                    float _32460;
                                    for (float _28481 = 0.0; _28481 < 128.0; _28489 = _32460, _28486 = _32458, _28483 = _32456, _28481 += 1.0)
                                    {
                                        float _17900 = (_28480 * 128.0) + _28481;
                                        if (_17900 >= _17850)
                                        {
                                            break;
                                        }
                                        float _18040 = floor(_17900 * 0.0625);
                                        uint _18057 = (((in.i_instance + 1u) + uint(_18040)) * 24u) + uint(_17900 - (_18040 * 16.0));
                                        float2 _18070 = _28483 - gFxData_1._data[_18057].xy;
                                        float2 _18073 = _13267 - gFxData_1._data[_18057].xy;
                                        float2 _18086 = _18073 - (_18070 * fast::clamp(dot(_18073, _18070) / fast::max(dot(_18070, _18070), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _18089 = dot(_18086, _18086);
                                        float _32535 = (_18089 < _28486) ? _18089 : _28486;
                                        bool _18101 = _13267.y >= gFxData_1._data[_18057].y;
                                        bool _18106 = _13267.y < _28483.y;
                                        bool _18117 = (_18070.x * _18073.y) > (_18070.y * _18073.x);
                                        float _28495;
                                        if (((_18101 && _18106) && _18117) || (((!_18101) && (!_18106)) && (!_18117)))
                                        {
                                            _28495 = -_28489;
                                        }
                                        else
                                        {
                                            _28495 = _28489;
                                        }
                                        if (((2.0 * _17900) + 1.0) < in.i_radii.z)
                                        {
                                            float2 _18146 = gFxData_1._data[_18057].xy - gFxData_1._data[_18057].zw;
                                            float2 _18149 = _13267 - gFxData_1._data[_18057].zw;
                                            float2 _18162 = _18149 - (_18146 * fast::clamp(dot(_18149, _18146) / fast::max(dot(_18146, _18146), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _18165 = dot(_18162, _18162);
                                            bool _18177 = _13267.y >= gFxData_1._data[_18057].w;
                                            bool _18182 = _13267.y < gFxData_1._data[_18057].y;
                                            bool _18193 = (_18146.x * _18149.y) > (_18146.y * _18149.x);
                                            float _28499;
                                            if (((_18177 && _18182) && _18193) || (((!_18177) && (!_18182)) && (!_18193)))
                                            {
                                                _28499 = -_28495;
                                            }
                                            else
                                            {
                                                _28499 = _28495;
                                            }
                                            _32460 = _28499;
                                            _32458 = (_18165 < _32535) ? _18165 : _32535;
                                            _32456 = gFxData_1._data[_18057].zw;
                                        }
                                        else
                                        {
                                            _32460 = _28495;
                                            _32458 = _32535;
                                            _32456 = gFxData_1._data[_18057].xy;
                                        }
                                    }
                                }
                                float _17952 = sqrt(_28487);
                                if (_17846)
                                {
                                    _28508 = _17952 - in.i_radii.x;
                                    break;
                                }
                                _28508 = (_28490 * _17952) - in.i_radii.x;
                                break;
                            } while(false);
                            _28510 = _28508;
                        }
                        else
                        {
                            float _28511;
                            if (in.i_flags.z == 1u)
                            {
                                float2 _17593 = _13267 - _17559;
                                float _28479;
                                do
                                {
                                    if (in.i_shape.w >= 6.282185077667236328125)
                                    {
                                        _28479 = abs(length(_17593) - in.i_radii.x) - in.i_radii.y;
                                        break;
                                    }
                                    float _18236 = in.i_shape.z + (in.i_shape.w * 0.5);
                                    float _18238 = cos(_18236);
                                    float _18240 = sin(_18236);
                                    float _18249 = dot(_17593, float2(-_18240, _18238));
                                    float _18252 = dot(_17593, float2(_18238, _18240));
                                    float2 _18253 = float2(_18249, _18252);
                                    float _18256 = abs(_18249);
                                    _18253.x = _18256;
                                    float _18259 = in.i_shape.w * 0.5;
                                    float _18261 = sin(_18259);
                                    float _18263 = cos(_18259);
                                    _28479 = (((_18263 * _18256) > (_18261 * _18252)) ? length(_18253 - (float2(_18261, _18263) * in.i_radii.x)) : abs(length(_18253) - in.i_radii.x)) - in.i_radii.y;
                                    break;
                                } while(false);
                                _28511 = _28479;
                            }
                            else
                            {
                                float _28512;
                                if (in.i_flags.z == 2u)
                                {
                                    float2 _18299 = _13267 - _27496.xy;
                                    float2 _18302 = _27496.zw - _27496.xy;
                                    _28512 = length(_18299 - (_18302 * fast::clamp(dot(_18299, _18302) / fast::max(dot(_18302, _18302), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                                }
                                else
                                {
                                    float2 _17620 = _13267 - _17559;
                                    float _18367 = fast::min(_17568.x, _17568.y);
                                    float _18370 = fast::min((_17620.x > 0.0) ? ((_17620.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_17620.y > 0.0) ? in.i_radii.w : in.i_radii.x), _18367);
                                    float _18376 = _18370 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                    float _28468;
                                    float _28469;
                                    if (_18376 > _18367)
                                    {
                                        float _18390 = in.i_shape.y * fast::clamp((_18367 - _18370) / fast::max(0.60000002384185791015625 * _18370, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                        _28469 = _18390;
                                        _28468 = _18370 * (1.0 + (0.60000002384185791015625 * _18390));
                                    }
                                    else
                                    {
                                        _28469 = in.i_shape.y;
                                        _28468 = _18376;
                                    }
                                    float2 _18419 = (abs(_17620) - _17568) + float2(_28468);
                                    float2 _18421 = fast::max(_18419, float2(0.0));
                                    float _28470;
                                    if ((_18421.x > 0.0) && (_18421.y > 0.0))
                                    {
                                        float _28471;
                                        if ((_28469 > 0.001000000047497451305389404296875) && (_28468 > 9.9999997473787516355514526367188e-05))
                                        {
                                            float _18438 = 2.0 + (2.0 * _28469);
                                            float2 _18443 = _18421 / float2(fast::max(_28468, 9.9999997473787516355514526367188e-05));
                                            _28471 = pow(pow(_18443.x, _18438) + pow(_18443.y, _18438), 1.0 / _18438) * _28468;
                                        }
                                        else
                                        {
                                            _28471 = length(_18421);
                                        }
                                        _28470 = _28471;
                                    }
                                    else
                                    {
                                        _28470 = fast::max(_18421.x, _18421.y);
                                    }
                                    float _18478 = (fast::min(fast::max(_18419.x, _18419.y), 0.0) + _28470) - _28468;
                                    float _28513;
                                    if ((in.i_flags.x & _1839) != 0u)
                                    {
                                        float2 _17648 = fast::max((_27496.zw - _27496.xy) * 0.5, float2(0.001000000047497451305389404296875));
                                        float2 _17651 = _13267 - ((_27496.xy + _27496.zw) * 0.5);
                                        float _18526 = fast::min(_17648.x, _17648.y);
                                        float _18529 = fast::min((_17651.x > 0.0) ? ((_17651.y > 0.0) ? gFxData_1._data[_12612].x : gFxData_1._data[_12612].x) : ((_17651.y > 0.0) ? gFxData_1._data[_12612].x : gFxData_1._data[_12612].x), _18526);
                                        float _18535 = _18529 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                        float _28472;
                                        float _28473;
                                        if (_18535 > _18526)
                                        {
                                            float _18549 = in.i_shape.y * fast::clamp((_18526 - _18529) / fast::max(0.60000002384185791015625 * _18529, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                            _28473 = _18549;
                                            _28472 = _18529 * (1.0 + (0.60000002384185791015625 * _18549));
                                        }
                                        else
                                        {
                                            _28473 = in.i_shape.y;
                                            _28472 = _18535;
                                        }
                                        float2 _18578 = (abs(_17651) - _17648) + float2(_28472);
                                        float2 _18580 = fast::max(_18578, float2(0.0));
                                        float _28474;
                                        if ((_18580.x > 0.0) && (_18580.y > 0.0))
                                        {
                                            float _28475;
                                            if ((_28473 > 0.001000000047497451305389404296875) && (_28472 > 9.9999997473787516355514526367188e-05))
                                            {
                                                float _18597 = 2.0 + (2.0 * _28473);
                                                float2 _18602 = _18580 / float2(fast::max(_28472, 9.9999997473787516355514526367188e-05));
                                                _28475 = pow(pow(_18602.x, _18597) + pow(_18602.y, _18597), 1.0 / _18597) * _28472;
                                            }
                                            else
                                            {
                                                _28475 = length(_18580);
                                            }
                                            _28474 = _28475;
                                        }
                                        else
                                        {
                                            _28474 = fast::max(_18580.x, _18580.y);
                                        }
                                        float _18637 = (fast::min(fast::max(_18578.x, _18578.y), 0.0) + _28474) - _28472;
                                        float _18642 = fast::max(gFxData_1._data[_12612].y, 9.9999997473787516355514526367188e-05);
                                        float _18651 = fast::max(_18642 - abs(_18478 - _18637), 0.0) / _18642;
                                        _28513 = fast::min(_18478, _18637) - (((_18651 * _18651) * _18642) * 0.25);
                                    }
                                    else
                                    {
                                        _28513 = _18478;
                                    }
                                    _28512 = _28513;
                                }
                                _28511 = _28512;
                            }
                            _28510 = _28511;
                        }
                        _28509 = _28510;
                    }
                    _28670 = float2(_28371 - _28417, _28463 - _28509);
                }
                float _13276 = length(_28670);
                _28708 = select(float2(0.0, -1.0), _28670 / float2(_13276), bool2(_13276 > 9.9999997473787516355514526367188e-06));
                break;
            } while(false);
            _29286 = fast::normalize(float3(_28708 * fast::min(_12713 / fast::max(_12719, 0.001000000047497451305389404296875), 8.0), 1.0));
            _28709 = _28708;
        }
        else
        {
            _29286 = float3(0.0, 0.0, 1.0);
            _28709 = float2(0.0, -1.0);
        }
        float2 _12745 = ((-_28709) * gFxData_1._data[_12588].y) * (1.0 - _12719);
        float2 _28710;
        if (gFxData_1._data[_12612].z > 0.0)
        {
            _28710 = (((in.i_rect.xy + in.i_rect.zw) * 0.5) - in.i_local) * (gFxData_1._data[_12612].z / (1.0 + gFxData_1._data[_12612].z));
        }
        else
        {
            _28710 = float2(0.0);
        }
        float2 _12771 = _27489 * _239.gTarget.zw;
        float2 _12778 = _239.gDisplay.zw * _239.gTarget.zw;
        float2 _12783 = (_12745 + _28710) * _12778;
        float2 _12786 = _12745 * _12778;
        float3 _29168;
        float _29186;
        float3 _29389;
        if (_239.gTime.z > 0.5)
        {
            float2 _12793 = _12771 + _12783;
            float _18676 = fast::clamp(log2(fast::max(_12685, 1.0)) - 1.0, 0.0, 5.0);
            int _18679 = int(floor(_18676));
            float _18683 = _18676 - float(_18679);
            bool _18688 = (_18683 > 0.0199999995529651641845703125) && (_18679 < 5);
            float3 _28781;
            _28781 = float3(0.0);
            float3 _18717;
            for (int _28711 = 0; _28711 < 2; _28781 = _18717, _28711++)
            {
                if ((_28711 > 0) && (!_18688))
                {
                    break;
                }
                int _18703 = _18679 + _28711;
                int _18741 = clamp(_18703, 1, 5);
                float2 _18810 = (_12793 * _239.gLevel[_18741].xy) - float2(0.5);
                float2 _18812 = floor(_18810);
                float2 _18815 = _18810 - _18812;
                float2 _18818 = _18815 * _18815;
                float2 _18821 = _18818 * _18815;
                float2 _18840 = (((_18821 * 3.0) - (_18818 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                float2 _18853 = _18821 * 0.16666667163372039794921875;
                float2 _18856 = (((((-_18821) + (_18818 * 3.0)) - (_18815 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _18840;
                float2 _18860 = (((((_18821 * (-3.0)) + (_18818 * 3.0)) + (_18815 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _18853;
                float2 _18872 = ((_18812 - float2(0.5)) + (_18840 / _18856)) * _239.gLevel[_18741].zw;
                float2 _18884 = ((_18812 + float2(1.5)) + (_18853 / _18860)) * _239.gLevel[_18741].zw;
                float4 _28748;
                if (_18703 <= 0)
                {
                    float2 _28747;
                    if (_239.gConv.x > 0.5)
                    {
                        float2 _26777 = _12793;
                        _26777.y = 1.0 - _12793.y;
                        _28747 = _26777;
                    }
                    else
                    {
                        _28747 = _12793;
                    }
                    _28748 = gBackdrop0.sample(gLinear, _28747, level(0.0));
                }
                else
                {
                    float4 _28749;
                    if (_18703 == 1)
                    {
                        float2 _28740;
                        if (_239.gConv.x > 0.5)
                        {
                            float2 _26782 = _18872;
                            _26782.y = 1.0 - _18872.y;
                            _28740 = _26782;
                        }
                        else
                        {
                            _28740 = _18872;
                        }
                        float2 _18930 = float2(_18884.x, _18872.y);
                        float2 _28741;
                        if (_239.gConv.x > 0.5)
                        {
                            float2 _26789 = _18930;
                            _26789.y = 1.0 - _18872.y;
                            _28741 = _26789;
                        }
                        else
                        {
                            _28741 = _18930;
                        }
                        float2 _18948 = float2(_18872.x, _18884.y);
                        float2 _28743;
                        if (_239.gConv.x > 0.5)
                        {
                            float2 _26796 = _18948;
                            _26796.y = 1.0 - _18884.y;
                            _28743 = _26796;
                        }
                        else
                        {
                            _28743 = _18948;
                        }
                        float2 _28745;
                        if (_239.gConv.x > 0.5)
                        {
                            float2 _26803 = _18884;
                            _26803.y = 1.0 - _18884.y;
                            _28745 = _26803;
                        }
                        else
                        {
                            _28745 = _18884;
                        }
                        _28749 = (((gBackdrop1.sample(gLinear, _28740, level(0.0)) * (_18856.x * _18856.y)) + (gBackdrop1.sample(gLinear, _28741, level(0.0)) * (_18860.x * _18856.y))) + (gBackdrop1.sample(gLinear, _28743, level(0.0)) * (_18856.x * _18860.y))) + (gBackdrop1.sample(gLinear, _28745, level(0.0)) * (_18860.x * _18860.y));
                    }
                    else
                    {
                        float4 _28750;
                        if (_18703 == 2)
                        {
                            float2 _28733;
                            if (_239.gConv.x > 0.5)
                            {
                                float2 _26810 = _18872;
                                _26810.y = 1.0 - _18872.y;
                                _28733 = _26810;
                            }
                            else
                            {
                                _28733 = _18872;
                            }
                            float2 _19060 = float2(_18884.x, _18872.y);
                            float2 _28734;
                            if (_239.gConv.x > 0.5)
                            {
                                float2 _26817 = _19060;
                                _26817.y = 1.0 - _18872.y;
                                _28734 = _26817;
                            }
                            else
                            {
                                _28734 = _19060;
                            }
                            float2 _19078 = float2(_18872.x, _18884.y);
                            float2 _28736;
                            if (_239.gConv.x > 0.5)
                            {
                                float2 _26824 = _19078;
                                _26824.y = 1.0 - _18884.y;
                                _28736 = _26824;
                            }
                            else
                            {
                                _28736 = _19078;
                            }
                            float2 _28738;
                            if (_239.gConv.x > 0.5)
                            {
                                float2 _26831 = _18884;
                                _26831.y = 1.0 - _18884.y;
                                _28738 = _26831;
                            }
                            else
                            {
                                _28738 = _18884;
                            }
                            _28750 = (((gBackdrop2.sample(gLinear, _28733, level(0.0)) * (_18856.x * _18856.y)) + (gBackdrop2.sample(gLinear, _28734, level(0.0)) * (_18860.x * _18856.y))) + (gBackdrop2.sample(gLinear, _28736, level(0.0)) * (_18856.x * _18860.y))) + (gBackdrop2.sample(gLinear, _28738, level(0.0)) * (_18860.x * _18860.y));
                        }
                        else
                        {
                            float4 _28751;
                            if (_18703 == 3)
                            {
                                float2 _28726;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _26838 = _18872;
                                    _26838.y = 1.0 - _18872.y;
                                    _28726 = _26838;
                                }
                                else
                                {
                                    _28726 = _18872;
                                }
                                float2 _19190 = float2(_18884.x, _18872.y);
                                float2 _28727;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _26845 = _19190;
                                    _26845.y = 1.0 - _18872.y;
                                    _28727 = _26845;
                                }
                                else
                                {
                                    _28727 = _19190;
                                }
                                float2 _19208 = float2(_18872.x, _18884.y);
                                float2 _28729;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _26852 = _19208;
                                    _26852.y = 1.0 - _18884.y;
                                    _28729 = _26852;
                                }
                                else
                                {
                                    _28729 = _19208;
                                }
                                float2 _28731;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _26859 = _18884;
                                    _26859.y = 1.0 - _18884.y;
                                    _28731 = _26859;
                                }
                                else
                                {
                                    _28731 = _18884;
                                }
                                _28751 = (((gBackdrop3.sample(gLinear, _28726, level(0.0)) * (_18856.x * _18856.y)) + (gBackdrop3.sample(gLinear, _28727, level(0.0)) * (_18860.x * _18856.y))) + (gBackdrop3.sample(gLinear, _28729, level(0.0)) * (_18856.x * _18860.y))) + (gBackdrop3.sample(gLinear, _28731, level(0.0)) * (_18860.x * _18860.y));
                            }
                            else
                            {
                                float4 _28752;
                                if (_18703 == 4)
                                {
                                    float2 _28719;
                                    if (_239.gConv.x > 0.5)
                                    {
                                        float2 _26866 = _18872;
                                        _26866.y = 1.0 - _18872.y;
                                        _28719 = _26866;
                                    }
                                    else
                                    {
                                        _28719 = _18872;
                                    }
                                    float2 _19320 = float2(_18884.x, _18872.y);
                                    float2 _28720;
                                    if (_239.gConv.x > 0.5)
                                    {
                                        float2 _26873 = _19320;
                                        _26873.y = 1.0 - _18872.y;
                                        _28720 = _26873;
                                    }
                                    else
                                    {
                                        _28720 = _19320;
                                    }
                                    float2 _19338 = float2(_18872.x, _18884.y);
                                    float2 _28722;
                                    if (_239.gConv.x > 0.5)
                                    {
                                        float2 _26880 = _19338;
                                        _26880.y = 1.0 - _18884.y;
                                        _28722 = _26880;
                                    }
                                    else
                                    {
                                        _28722 = _19338;
                                    }
                                    float2 _28724;
                                    if (_239.gConv.x > 0.5)
                                    {
                                        float2 _26887 = _18884;
                                        _26887.y = 1.0 - _18884.y;
                                        _28724 = _26887;
                                    }
                                    else
                                    {
                                        _28724 = _18884;
                                    }
                                    _28752 = (((gBackdrop4.sample(gLinear, _28719, level(0.0)) * (_18856.x * _18856.y)) + (gBackdrop4.sample(gLinear, _28720, level(0.0)) * (_18860.x * _18856.y))) + (gBackdrop4.sample(gLinear, _28722, level(0.0)) * (_18856.x * _18860.y))) + (gBackdrop4.sample(gLinear, _28724, level(0.0)) * (_18860.x * _18860.y));
                                }
                                else
                                {
                                    float2 _28712;
                                    if (_239.gConv.x > 0.5)
                                    {
                                        float2 _26894 = _18872;
                                        _26894.y = 1.0 - _18872.y;
                                        _28712 = _26894;
                                    }
                                    else
                                    {
                                        _28712 = _18872;
                                    }
                                    float2 _19450 = float2(_18884.x, _18872.y);
                                    float2 _28713;
                                    if (_239.gConv.x > 0.5)
                                    {
                                        float2 _26901 = _19450;
                                        _26901.y = 1.0 - _18872.y;
                                        _28713 = _26901;
                                    }
                                    else
                                    {
                                        _28713 = _19450;
                                    }
                                    float2 _19468 = float2(_18872.x, _18884.y);
                                    float2 _28715;
                                    if (_239.gConv.x > 0.5)
                                    {
                                        float2 _26908 = _19468;
                                        _26908.y = 1.0 - _18884.y;
                                        _28715 = _26908;
                                    }
                                    else
                                    {
                                        _28715 = _19468;
                                    }
                                    float2 _28717;
                                    if (_239.gConv.x > 0.5)
                                    {
                                        float2 _26915 = _18884;
                                        _26915.y = 1.0 - _18884.y;
                                        _28717 = _26915;
                                    }
                                    else
                                    {
                                        _28717 = _18884;
                                    }
                                    _28752 = (((gBackdrop5.sample(gLinear, _28712, level(0.0)) * (_18856.x * _18856.y)) + (gBackdrop5.sample(gLinear, _28713, level(0.0)) * (_18860.x * _18856.y))) + (gBackdrop5.sample(gLinear, _28715, level(0.0)) * (_18856.x * _18860.y))) + (gBackdrop5.sample(gLinear, _28717, level(0.0)) * (_18860.x * _18860.y));
                                }
                                _28751 = _28752;
                            }
                            _28750 = _28751;
                        }
                        _28749 = _28750;
                    }
                    _28748 = _28749;
                }
                _18717 = _28781 + (_28748.xyz * (_18688 ? ((_28711 == 0) ? (1.0 - _18683) : _18683) : 1.0));
            }
            float3 _29171;
            if (((gFxData_1._data[_12588].w > 0.001000000047497451305389404296875) && (_12713 > 0.0)) && ((((gFxData_1._data[_12588].y * 0.300000011920928955078125) * gFxData_1._data[_12588].w) * _239.gDisplay.z) > (_12685 * 0.3499999940395355224609375)))
            {
                float _12813 = 0.300000011920928955078125 * gFxData_1._data[_12588].w;
                float2 _12820 = (_12771 + _12783) - (_12786 * _12813);
                float _19564 = fast::clamp(log2(fast::max(_12685, 1.0)) - 1.0, 0.0, 5.0);
                int _19567 = int(floor(_19564));
                float _19571 = _19564 - float(_19567);
                bool _19576 = (_19571 > 0.0199999995529651641845703125) && (_19567 < 5);
                float3 _28877;
                _28877 = float3(0.0);
                float3 _19605;
                for (int _28807 = 0; _28807 < 2; _28877 = _19605, _28807++)
                {
                    if ((_28807 > 0) && (!_19576))
                    {
                        break;
                    }
                    int _19591 = _19567 + _28807;
                    int _19629 = clamp(_19591, 1, 5);
                    float2 _19698 = (_12820 * _239.gLevel[_19629].xy) - float2(0.5);
                    float2 _19700 = floor(_19698);
                    float2 _19703 = _19698 - _19700;
                    float2 _19706 = _19703 * _19703;
                    float2 _19709 = _19706 * _19703;
                    float2 _19728 = (((_19709 * 3.0) - (_19706 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                    float2 _19741 = _19709 * 0.16666667163372039794921875;
                    float2 _19744 = (((((-_19709) + (_19706 * 3.0)) - (_19703 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _19728;
                    float2 _19748 = (((((_19709 * (-3.0)) + (_19706 * 3.0)) + (_19703 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _19741;
                    float2 _19760 = ((_19700 - float2(0.5)) + (_19728 / _19744)) * _239.gLevel[_19629].zw;
                    float2 _19772 = ((_19700 + float2(1.5)) + (_19741 / _19748)) * _239.gLevel[_19629].zw;
                    float4 _28844;
                    if (_19591 <= 0)
                    {
                        float2 _28843;
                        if (_239.gConv.x > 0.5)
                        {
                            float2 _26920 = _12820;
                            _26920.y = 1.0 - _12820.y;
                            _28843 = _26920;
                        }
                        else
                        {
                            _28843 = _12820;
                        }
                        _28844 = gBackdrop0.sample(gLinear, _28843, level(0.0));
                    }
                    else
                    {
                        float4 _28845;
                        if (_19591 == 1)
                        {
                            float2 _28836;
                            if (_239.gConv.x > 0.5)
                            {
                                float2 _26925 = _19760;
                                _26925.y = 1.0 - _19760.y;
                                _28836 = _26925;
                            }
                            else
                            {
                                _28836 = _19760;
                            }
                            float _19817 = _19760.y;
                            float2 _19818 = float2(_19772.x, _19817);
                            float2 _28837;
                            if (_239.gConv.x > 0.5)
                            {
                                float2 _26932 = _19818;
                                _26932.y = 1.0 - _19817;
                                _28837 = _26932;
                            }
                            else
                            {
                                _28837 = _19818;
                            }
                            float _19835 = _19772.y;
                            float2 _19836 = float2(_19760.x, _19835);
                            float2 _28839;
                            if (_239.gConv.x > 0.5)
                            {
                                float2 _26939 = _19836;
                                _26939.y = 1.0 - _19835;
                                _28839 = _26939;
                            }
                            else
                            {
                                _28839 = _19836;
                            }
                            float2 _28841;
                            if (_239.gConv.x > 0.5)
                            {
                                float2 _26946 = _19772;
                                _26946.y = 1.0 - _19772.y;
                                _28841 = _26946;
                            }
                            else
                            {
                                _28841 = _19772;
                            }
                            _28845 = (((gBackdrop1.sample(gLinear, _28836, level(0.0)) * (_19744.x * _19744.y)) + (gBackdrop1.sample(gLinear, _28837, level(0.0)) * (_19748.x * _19744.y))) + (gBackdrop1.sample(gLinear, _28839, level(0.0)) * (_19744.x * _19748.y))) + (gBackdrop1.sample(gLinear, _28841, level(0.0)) * (_19748.x * _19748.y));
                        }
                        else
                        {
                            float4 _28846;
                            if (_19591 == 2)
                            {
                                float2 _28829;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _26953 = _19760;
                                    _26953.y = 1.0 - _19760.y;
                                    _28829 = _26953;
                                }
                                else
                                {
                                    _28829 = _19760;
                                }
                                float _19947 = _19760.y;
                                float2 _19948 = float2(_19772.x, _19947);
                                float2 _28830;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _26960 = _19948;
                                    _26960.y = 1.0 - _19947;
                                    _28830 = _26960;
                                }
                                else
                                {
                                    _28830 = _19948;
                                }
                                float _19965 = _19772.y;
                                float2 _19966 = float2(_19760.x, _19965);
                                float2 _28832;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _26967 = _19966;
                                    _26967.y = 1.0 - _19965;
                                    _28832 = _26967;
                                }
                                else
                                {
                                    _28832 = _19966;
                                }
                                float2 _28834;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _26974 = _19772;
                                    _26974.y = 1.0 - _19772.y;
                                    _28834 = _26974;
                                }
                                else
                                {
                                    _28834 = _19772;
                                }
                                _28846 = (((gBackdrop2.sample(gLinear, _28829, level(0.0)) * (_19744.x * _19744.y)) + (gBackdrop2.sample(gLinear, _28830, level(0.0)) * (_19748.x * _19744.y))) + (gBackdrop2.sample(gLinear, _28832, level(0.0)) * (_19744.x * _19748.y))) + (gBackdrop2.sample(gLinear, _28834, level(0.0)) * (_19748.x * _19748.y));
                            }
                            else
                            {
                                float4 _28847;
                                if (_19591 == 3)
                                {
                                    float2 _28822;
                                    if (_239.gConv.x > 0.5)
                                    {
                                        float2 _26981 = _19760;
                                        _26981.y = 1.0 - _19760.y;
                                        _28822 = _26981;
                                    }
                                    else
                                    {
                                        _28822 = _19760;
                                    }
                                    float _20077 = _19760.y;
                                    float2 _20078 = float2(_19772.x, _20077);
                                    float2 _28823;
                                    if (_239.gConv.x > 0.5)
                                    {
                                        float2 _26988 = _20078;
                                        _26988.y = 1.0 - _20077;
                                        _28823 = _26988;
                                    }
                                    else
                                    {
                                        _28823 = _20078;
                                    }
                                    float _20095 = _19772.y;
                                    float2 _20096 = float2(_19760.x, _20095);
                                    float2 _28825;
                                    if (_239.gConv.x > 0.5)
                                    {
                                        float2 _26995 = _20096;
                                        _26995.y = 1.0 - _20095;
                                        _28825 = _26995;
                                    }
                                    else
                                    {
                                        _28825 = _20096;
                                    }
                                    float2 _28827;
                                    if (_239.gConv.x > 0.5)
                                    {
                                        float2 _27002 = _19772;
                                        _27002.y = 1.0 - _19772.y;
                                        _28827 = _27002;
                                    }
                                    else
                                    {
                                        _28827 = _19772;
                                    }
                                    _28847 = (((gBackdrop3.sample(gLinear, _28822, level(0.0)) * (_19744.x * _19744.y)) + (gBackdrop3.sample(gLinear, _28823, level(0.0)) * (_19748.x * _19744.y))) + (gBackdrop3.sample(gLinear, _28825, level(0.0)) * (_19744.x * _19748.y))) + (gBackdrop3.sample(gLinear, _28827, level(0.0)) * (_19748.x * _19748.y));
                                }
                                else
                                {
                                    float4 _28848;
                                    if (_19591 == 4)
                                    {
                                        float2 _28815;
                                        if (_239.gConv.x > 0.5)
                                        {
                                            float2 _27009 = _19760;
                                            _27009.y = 1.0 - _19760.y;
                                            _28815 = _27009;
                                        }
                                        else
                                        {
                                            _28815 = _19760;
                                        }
                                        float _20207 = _19760.y;
                                        float2 _20208 = float2(_19772.x, _20207);
                                        float2 _28816;
                                        if (_239.gConv.x > 0.5)
                                        {
                                            float2 _27016 = _20208;
                                            _27016.y = 1.0 - _20207;
                                            _28816 = _27016;
                                        }
                                        else
                                        {
                                            _28816 = _20208;
                                        }
                                        float _20225 = _19772.y;
                                        float2 _20226 = float2(_19760.x, _20225);
                                        float2 _28818;
                                        if (_239.gConv.x > 0.5)
                                        {
                                            float2 _27023 = _20226;
                                            _27023.y = 1.0 - _20225;
                                            _28818 = _27023;
                                        }
                                        else
                                        {
                                            _28818 = _20226;
                                        }
                                        float2 _28820;
                                        if (_239.gConv.x > 0.5)
                                        {
                                            float2 _27030 = _19772;
                                            _27030.y = 1.0 - _19772.y;
                                            _28820 = _27030;
                                        }
                                        else
                                        {
                                            _28820 = _19772;
                                        }
                                        _28848 = (((gBackdrop4.sample(gLinear, _28815, level(0.0)) * (_19744.x * _19744.y)) + (gBackdrop4.sample(gLinear, _28816, level(0.0)) * (_19748.x * _19744.y))) + (gBackdrop4.sample(gLinear, _28818, level(0.0)) * (_19744.x * _19748.y))) + (gBackdrop4.sample(gLinear, _28820, level(0.0)) * (_19748.x * _19748.y));
                                    }
                                    else
                                    {
                                        float2 _28808;
                                        if (_239.gConv.x > 0.5)
                                        {
                                            float2 _27037 = _19760;
                                            _27037.y = 1.0 - _19760.y;
                                            _28808 = _27037;
                                        }
                                        else
                                        {
                                            _28808 = _19760;
                                        }
                                        float _20337 = _19760.y;
                                        float2 _20338 = float2(_19772.x, _20337);
                                        float2 _28809;
                                        if (_239.gConv.x > 0.5)
                                        {
                                            float2 _27044 = _20338;
                                            _27044.y = 1.0 - _20337;
                                            _28809 = _27044;
                                        }
                                        else
                                        {
                                            _28809 = _20338;
                                        }
                                        float _20355 = _19772.y;
                                        float2 _20356 = float2(_19760.x, _20355);
                                        float2 _28811;
                                        if (_239.gConv.x > 0.5)
                                        {
                                            float2 _27051 = _20356;
                                            _27051.y = 1.0 - _20355;
                                            _28811 = _27051;
                                        }
                                        else
                                        {
                                            _28811 = _20356;
                                        }
                                        float2 _28813;
                                        if (_239.gConv.x > 0.5)
                                        {
                                            float2 _27058 = _19772;
                                            _27058.y = 1.0 - _19772.y;
                                            _28813 = _27058;
                                        }
                                        else
                                        {
                                            _28813 = _19772;
                                        }
                                        _28848 = (((gBackdrop5.sample(gLinear, _28808, level(0.0)) * (_19744.x * _19744.y)) + (gBackdrop5.sample(gLinear, _28809, level(0.0)) * (_19748.x * _19744.y))) + (gBackdrop5.sample(gLinear, _28811, level(0.0)) * (_19744.x * _19748.y))) + (gBackdrop5.sample(gLinear, _28813, level(0.0)) * (_19748.x * _19748.y));
                                    }
                                    _28847 = _28848;
                                }
                                _28846 = _28847;
                            }
                            _28845 = _28846;
                        }
                        _28844 = _28845;
                    }
                    _19605 = _28877 + (_28844.xyz * (_19576 ? ((_28807 == 0) ? (1.0 - _19571) : _19571) : 1.0));
                }
                float3 _27062 = _28781;
                _27062.x = _28877.x;
                float2 _12831 = (_12771 + _12783) + (_12786 * _12813);
                float _20452 = fast::clamp(log2(fast::max(_12685, 1.0)) - 1.0, 0.0, 5.0);
                int _20455 = int(floor(_20452));
                float _20459 = _20452 - float(_20455);
                bool _20464 = (_20459 > 0.0199999995529651641845703125) && (_20455 < 5);
                float3 _29001;
                _29001 = float3(0.0);
                float3 _20493;
                for (int _28931 = 0; _28931 < 2; _29001 = _20493, _28931++)
                {
                    if ((_28931 > 0) && (!_20464))
                    {
                        break;
                    }
                    int _20479 = _20455 + _28931;
                    int _20517 = clamp(_20479, 1, 5);
                    float2 _20586 = (_12831 * _239.gLevel[_20517].xy) - float2(0.5);
                    float2 _20588 = floor(_20586);
                    float2 _20591 = _20586 - _20588;
                    float2 _20594 = _20591 * _20591;
                    float2 _20597 = _20594 * _20591;
                    float2 _20616 = (((_20597 * 3.0) - (_20594 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                    float2 _20629 = _20597 * 0.16666667163372039794921875;
                    float2 _20632 = (((((-_20597) + (_20594 * 3.0)) - (_20591 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _20616;
                    float2 _20636 = (((((_20597 * (-3.0)) + (_20594 * 3.0)) + (_20591 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _20629;
                    float2 _20648 = ((_20588 - float2(0.5)) + (_20616 / _20632)) * _239.gLevel[_20517].zw;
                    float2 _20660 = ((_20588 + float2(1.5)) + (_20629 / _20636)) * _239.gLevel[_20517].zw;
                    float4 _28968;
                    if (_20479 <= 0)
                    {
                        float2 _28967;
                        if (_239.gConv.x > 0.5)
                        {
                            float2 _27065 = _12831;
                            _27065.y = 1.0 - _12831.y;
                            _28967 = _27065;
                        }
                        else
                        {
                            _28967 = _12831;
                        }
                        _28968 = gBackdrop0.sample(gLinear, _28967, level(0.0));
                    }
                    else
                    {
                        float4 _28969;
                        if (_20479 == 1)
                        {
                            float2 _28960;
                            if (_239.gConv.x > 0.5)
                            {
                                float2 _27070 = _20648;
                                _27070.y = 1.0 - _20648.y;
                                _28960 = _27070;
                            }
                            else
                            {
                                _28960 = _20648;
                            }
                            float _20705 = _20648.y;
                            float2 _20706 = float2(_20660.x, _20705);
                            float2 _28961;
                            if (_239.gConv.x > 0.5)
                            {
                                float2 _27077 = _20706;
                                _27077.y = 1.0 - _20705;
                                _28961 = _27077;
                            }
                            else
                            {
                                _28961 = _20706;
                            }
                            float _20723 = _20660.y;
                            float2 _20724 = float2(_20648.x, _20723);
                            float2 _28963;
                            if (_239.gConv.x > 0.5)
                            {
                                float2 _27084 = _20724;
                                _27084.y = 1.0 - _20723;
                                _28963 = _27084;
                            }
                            else
                            {
                                _28963 = _20724;
                            }
                            float2 _28965;
                            if (_239.gConv.x > 0.5)
                            {
                                float2 _27091 = _20660;
                                _27091.y = 1.0 - _20660.y;
                                _28965 = _27091;
                            }
                            else
                            {
                                _28965 = _20660;
                            }
                            _28969 = (((gBackdrop1.sample(gLinear, _28960, level(0.0)) * (_20632.x * _20632.y)) + (gBackdrop1.sample(gLinear, _28961, level(0.0)) * (_20636.x * _20632.y))) + (gBackdrop1.sample(gLinear, _28963, level(0.0)) * (_20632.x * _20636.y))) + (gBackdrop1.sample(gLinear, _28965, level(0.0)) * (_20636.x * _20636.y));
                        }
                        else
                        {
                            float4 _28970;
                            if (_20479 == 2)
                            {
                                float2 _28953;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _27098 = _20648;
                                    _27098.y = 1.0 - _20648.y;
                                    _28953 = _27098;
                                }
                                else
                                {
                                    _28953 = _20648;
                                }
                                float _20835 = _20648.y;
                                float2 _20836 = float2(_20660.x, _20835);
                                float2 _28954;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _27105 = _20836;
                                    _27105.y = 1.0 - _20835;
                                    _28954 = _27105;
                                }
                                else
                                {
                                    _28954 = _20836;
                                }
                                float _20853 = _20660.y;
                                float2 _20854 = float2(_20648.x, _20853);
                                float2 _28956;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _27112 = _20854;
                                    _27112.y = 1.0 - _20853;
                                    _28956 = _27112;
                                }
                                else
                                {
                                    _28956 = _20854;
                                }
                                float2 _28958;
                                if (_239.gConv.x > 0.5)
                                {
                                    float2 _27119 = _20660;
                                    _27119.y = 1.0 - _20660.y;
                                    _28958 = _27119;
                                }
                                else
                                {
                                    _28958 = _20660;
                                }
                                _28970 = (((gBackdrop2.sample(gLinear, _28953, level(0.0)) * (_20632.x * _20632.y)) + (gBackdrop2.sample(gLinear, _28954, level(0.0)) * (_20636.x * _20632.y))) + (gBackdrop2.sample(gLinear, _28956, level(0.0)) * (_20632.x * _20636.y))) + (gBackdrop2.sample(gLinear, _28958, level(0.0)) * (_20636.x * _20636.y));
                            }
                            else
                            {
                                float4 _28971;
                                if (_20479 == 3)
                                {
                                    float2 _28946;
                                    if (_239.gConv.x > 0.5)
                                    {
                                        float2 _27126 = _20648;
                                        _27126.y = 1.0 - _20648.y;
                                        _28946 = _27126;
                                    }
                                    else
                                    {
                                        _28946 = _20648;
                                    }
                                    float _20965 = _20648.y;
                                    float2 _20966 = float2(_20660.x, _20965);
                                    float2 _28947;
                                    if (_239.gConv.x > 0.5)
                                    {
                                        float2 _27133 = _20966;
                                        _27133.y = 1.0 - _20965;
                                        _28947 = _27133;
                                    }
                                    else
                                    {
                                        _28947 = _20966;
                                    }
                                    float _20983 = _20660.y;
                                    float2 _20984 = float2(_20648.x, _20983);
                                    float2 _28949;
                                    if (_239.gConv.x > 0.5)
                                    {
                                        float2 _27140 = _20984;
                                        _27140.y = 1.0 - _20983;
                                        _28949 = _27140;
                                    }
                                    else
                                    {
                                        _28949 = _20984;
                                    }
                                    float2 _28951;
                                    if (_239.gConv.x > 0.5)
                                    {
                                        float2 _27147 = _20660;
                                        _27147.y = 1.0 - _20660.y;
                                        _28951 = _27147;
                                    }
                                    else
                                    {
                                        _28951 = _20660;
                                    }
                                    _28971 = (((gBackdrop3.sample(gLinear, _28946, level(0.0)) * (_20632.x * _20632.y)) + (gBackdrop3.sample(gLinear, _28947, level(0.0)) * (_20636.x * _20632.y))) + (gBackdrop3.sample(gLinear, _28949, level(0.0)) * (_20632.x * _20636.y))) + (gBackdrop3.sample(gLinear, _28951, level(0.0)) * (_20636.x * _20636.y));
                                }
                                else
                                {
                                    float4 _28972;
                                    if (_20479 == 4)
                                    {
                                        float2 _28939;
                                        if (_239.gConv.x > 0.5)
                                        {
                                            float2 _27154 = _20648;
                                            _27154.y = 1.0 - _20648.y;
                                            _28939 = _27154;
                                        }
                                        else
                                        {
                                            _28939 = _20648;
                                        }
                                        float _21095 = _20648.y;
                                        float2 _21096 = float2(_20660.x, _21095);
                                        float2 _28940;
                                        if (_239.gConv.x > 0.5)
                                        {
                                            float2 _27161 = _21096;
                                            _27161.y = 1.0 - _21095;
                                            _28940 = _27161;
                                        }
                                        else
                                        {
                                            _28940 = _21096;
                                        }
                                        float _21113 = _20660.y;
                                        float2 _21114 = float2(_20648.x, _21113);
                                        float2 _28942;
                                        if (_239.gConv.x > 0.5)
                                        {
                                            float2 _27168 = _21114;
                                            _27168.y = 1.0 - _21113;
                                            _28942 = _27168;
                                        }
                                        else
                                        {
                                            _28942 = _21114;
                                        }
                                        float2 _28944;
                                        if (_239.gConv.x > 0.5)
                                        {
                                            float2 _27175 = _20660;
                                            _27175.y = 1.0 - _20660.y;
                                            _28944 = _27175;
                                        }
                                        else
                                        {
                                            _28944 = _20660;
                                        }
                                        _28972 = (((gBackdrop4.sample(gLinear, _28939, level(0.0)) * (_20632.x * _20632.y)) + (gBackdrop4.sample(gLinear, _28940, level(0.0)) * (_20636.x * _20632.y))) + (gBackdrop4.sample(gLinear, _28942, level(0.0)) * (_20632.x * _20636.y))) + (gBackdrop4.sample(gLinear, _28944, level(0.0)) * (_20636.x * _20636.y));
                                    }
                                    else
                                    {
                                        float2 _28932;
                                        if (_239.gConv.x > 0.5)
                                        {
                                            float2 _27182 = _20648;
                                            _27182.y = 1.0 - _20648.y;
                                            _28932 = _27182;
                                        }
                                        else
                                        {
                                            _28932 = _20648;
                                        }
                                        float _21225 = _20648.y;
                                        float2 _21226 = float2(_20660.x, _21225);
                                        float2 _28933;
                                        if (_239.gConv.x > 0.5)
                                        {
                                            float2 _27189 = _21226;
                                            _27189.y = 1.0 - _21225;
                                            _28933 = _27189;
                                        }
                                        else
                                        {
                                            _28933 = _21226;
                                        }
                                        float _21243 = _20660.y;
                                        float2 _21244 = float2(_20648.x, _21243);
                                        float2 _28935;
                                        if (_239.gConv.x > 0.5)
                                        {
                                            float2 _27196 = _21244;
                                            _27196.y = 1.0 - _21243;
                                            _28935 = _27196;
                                        }
                                        else
                                        {
                                            _28935 = _21244;
                                        }
                                        float2 _28937;
                                        if (_239.gConv.x > 0.5)
                                        {
                                            float2 _27203 = _20660;
                                            _27203.y = 1.0 - _20660.y;
                                            _28937 = _27203;
                                        }
                                        else
                                        {
                                            _28937 = _20660;
                                        }
                                        _28972 = (((gBackdrop5.sample(gLinear, _28932, level(0.0)) * (_20632.x * _20632.y)) + (gBackdrop5.sample(gLinear, _28933, level(0.0)) * (_20636.x * _20632.y))) + (gBackdrop5.sample(gLinear, _28935, level(0.0)) * (_20632.x * _20636.y))) + (gBackdrop5.sample(gLinear, _28937, level(0.0)) * (_20636.x * _20636.y));
                                    }
                                    _28971 = _28972;
                                }
                                _28970 = _28971;
                            }
                            _28969 = _28970;
                        }
                        _28968 = _28969;
                    }
                    _20493 = _29001 + (_28968.xyz * (_20464 ? ((_28931 == 0) ? (1.0 - _20459) : _20459) : 1.0));
                }
                _27062.z = _29001.z;
                _29171 = _27062;
            }
            else
            {
                _29171 = _28781;
            }
            float3 _29391;
            if (_12713 > 0.0)
            {
                float2 _12854 = _12771 + (((_28709 * fast::min(_12704 * 0.5, 16.0)) * _239.gDisplay.zw) * _239.gTarget.zw);
                float _21340 = fast::clamp(log2(fast::max(16.0 * _239.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _21343 = int(floor(_21340));
                float _21347 = _21340 - float(_21343);
                bool _21352 = (_21347 > 0.0199999995529651641845703125) && (_21343 < 5);
                float3 _29156;
                _29156 = float3(0.0);
                float3 _21381;
                for (int _29141 = 0; _29141 < 2; _29156 = _21381, _29141++)
                {
                    if ((_29141 > 0) && (!_21352))
                    {
                        break;
                    }
                    int _21367 = _21343 + _29141;
                    float2 _29142;
                    if (_239.gConv.x > 0.5)
                    {
                        float2 _27210 = _12854;
                        _27210.y = 1.0 - _12854.y;
                        _29142 = _27210;
                    }
                    else
                    {
                        _29142 = _12854;
                    }
                    float4 _29143;
                    if (_21367 <= 0)
                    {
                        _29143 = gBackdrop0.sample(gLinear, _29142, level(0.0));
                    }
                    else
                    {
                        float4 _29144;
                        if (_21367 == 1)
                        {
                            _29144 = gBackdrop1.sample(gLinear, _29142, level(0.0));
                        }
                        else
                        {
                            float4 _29145;
                            if (_21367 == 2)
                            {
                                _29145 = gBackdrop2.sample(gLinear, _29142, level(0.0));
                            }
                            else
                            {
                                float4 _29146;
                                if (_21367 == 3)
                                {
                                    _29146 = gBackdrop3.sample(gLinear, _29142, level(0.0));
                                }
                                else
                                {
                                    float4 _29147;
                                    if (_21367 == 4)
                                    {
                                        _29147 = gBackdrop4.sample(gLinear, _29142, level(0.0));
                                    }
                                    else
                                    {
                                        _29147 = gBackdrop5.sample(gLinear, _29142, level(0.0));
                                    }
                                    _29146 = _29147;
                                }
                                _29145 = _29146;
                            }
                            _29144 = _29145;
                        }
                        _29143 = _29144;
                    }
                    _21381 = _29156 + (_29143.xyz * (_21352 ? ((_29141 == 0) ? (1.0 - _21347) : _21347) : 1.0));
                }
                _29391 = _29156;
            }
            else
            {
                _29391 = float3(0.5);
            }
            float _29187;
            if (in.i_shape.x > 0.001000000047497451305389404296875)
            {
                int _21470 = clamp(int(rint(log2(36.0 * _239.gDisplay.z) - 1.0)), 1, 4);
                float2 _29162;
                if (_239.gConv.x > 0.5)
                {
                    float2 _27214 = _12771;
                    _27214.y = 1.0 - _12771.y;
                    _29162 = _27214;
                }
                else
                {
                    _29162 = _12771;
                }
                float4 _29163;
                if (_21470 <= 0)
                {
                    _29163 = gBackdrop0.sample(gLinear, _29162, level(0.0));
                }
                else
                {
                    float4 _29164;
                    if (_21470 == 1)
                    {
                        _29164 = gBackdrop1.sample(gLinear, _29162, level(0.0));
                    }
                    else
                    {
                        float4 _29165;
                        if (_21470 == 2)
                        {
                            _29165 = gBackdrop2.sample(gLinear, _29162, level(0.0));
                        }
                        else
                        {
                            float4 _29166;
                            if (_21470 == 3)
                            {
                                _29166 = gBackdrop3.sample(gLinear, _29162, level(0.0));
                            }
                            else
                            {
                                float4 _29167;
                                if (_21470 == 4)
                                {
                                    _29167 = gBackdrop4.sample(gLinear, _29162, level(0.0));
                                }
                                else
                                {
                                    _29167 = gBackdrop5.sample(gLinear, _29162, level(0.0));
                                }
                                _29166 = _29167;
                            }
                            _29165 = _29166;
                        }
                        _29164 = _29165;
                    }
                    _29163 = _29164;
                }
                _29187 = dot(_29163.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _29187 = 0.5;
            }
            _29389 = _29391;
            _29186 = _29187;
            _29168 = _29171;
        }
        else
        {
            _29389 = float3(0.5);
            _29186 = 0.5;
            _29168 = float3(0.5);
        }
        float3 _12882 = mix(float3(dot(_29168, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _29168, float3(gFxData_1._data[_12604].x)) + float3(gFxData_1._data[_12604].y);
        float3 _29282;
        if (in.i_shape.x > 0.001000000047497451305389404296875)
        {
            float _12892 = fast::clamp(fast::max(_29186 + gFxData_1._data[_12604].y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _12900 = mix(_12892, dot(gFxData_1._data[_12596].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), in.i_shape.x);
            _29282 = select(mix(_12882, float3(1.0), float3((_12900 - _12892) / fast::max(1.0 - _12892, 0.001000000047497451305389404296875))), _12882 * (_12900 / _12892), bool3(_12900 < _12892));
        }
        else
        {
            _29282 = _12882;
        }
        float2 _12938 = fast::clamp((in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875)), float2(0.0), float2(1.0));
        float2 _29502;
        if (_281.gDrawInfo.w > 0.5)
        {
            float2 _21582 = select(_281.gDrawInfo.yz, _281.gDrawInfo.yz - in.i_local, bool2(_281.gDrawInfo.w > 1.0));
            _29502 = _21582 / float2(fast::max(length(_21582), 9.9999997473787516355514526367188e-05));
        }
        else
        {
            _29502 = float2(cos(gFxData_1._data[_12604].w), sin(gFxData_1._data[_12604].w));
        }
        float _12994 = -_27540;
        float _13001 = _5114 * ((0.5 * abs(_28709.x)) + 0.0199999995529651641845703125);
        float _13008 = _5114 * ((0.5 * abs(_28709.y)) + 0.0199999995529651641845703125);
        float _29506;
        if ((((_12994 + _13001) + _13008) > 0.0) && (((_12994 - _13001) - _13008) < (0.5 * _12704)))
        {
            float _21601 = fast::max((_12994 + _13001) + _13008, 0.0) / _12704;
            float _21605 = fast::clamp(sqrt(2.0 * _21601), 0.12399999797344207763671875, 1.0);
            float _21607 = 1.0 - _21605;
            float _21610 = _21607 * _21607;
            float _21664 = fast::max((_12994 + _13001) - _13008, 0.0) / _12704;
            float _21668 = fast::clamp(sqrt(2.0 * _21664), 0.12399999797344207763671875, 1.0);
            float _21670 = 1.0 - _21668;
            float _21673 = _21670 * _21670;
            float _21727 = fast::max((_12994 - _13001) + _13008, 0.0) / _12704;
            float _21731 = fast::clamp(sqrt(2.0 * _21727), 0.12399999797344207763671875, 1.0);
            float _21733 = 1.0 - _21731;
            float _21736 = _21733 * _21733;
            float _21790 = fast::max((_12994 - _13001) - _13008, 0.0) / _12704;
            float _21794 = fast::clamp(sqrt(2.0 * _21790), 0.12399999797344207763671875, 1.0);
            float _21796 = 1.0 - _21794;
            float _21799 = _21796 * _21796;
            float _13056 = ((((((_21601 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _21601) * _21601) : ((_21601 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _21605) * _21605)) + ((((_21610 * _21610) * _21610) * _21607) * (0.02380952425301074981689453125 + (_21607 * ((-0.0386904776096343994140625) + (_21607 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_21601 - 0.5))))) * _12704) * _12704) - ((((_21664 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _21664) * _21664) : ((_21664 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _21668) * _21668)) + ((((_21673 * _21673) * _21673) * _21670) * (0.02380952425301074981689453125 + (_21670 * ((-0.0386904776096343994140625) + (_21670 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_21664 - 0.5))))) * _12704) * _12704)) - ((((_21727 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _21727) * _21727) : ((_21727 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _21731) * _21731)) + ((((_21736 * _21736) * _21736) * _21733) * (0.02380952425301074981689453125 + (_21733 * ((-0.0386904776096343994140625) + (_21733 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_21727 - 0.5))))) * _12704) * _12704)) + ((((_21790 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _21790) * _21790) : ((_21790 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _21794) * _21794)) + ((((_21799 * _21799) * _21799) * _21796) * (0.02380952425301074981689453125 + (_21796 * ((-0.0386904776096343994140625) + (_21796 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_21790 - 0.5))))) * _12704) * _12704);
            float _13064 = dot(_28709, _29502);
            _29506 = (((_13056 / ((4.0 * _13001) * _13008)) * (pow(fast::clamp(_13064, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(fast::clamp(-_13064, 0.0, 1.0), 1.5)))) * gFxData_1._data[_12604].z) * 1.60000002384185791015625;
        }
        else
        {
            _29506 = 0.0;
        }
        float _13100 = fast::clamp(0.5 + (0.5 * dot((in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5)) / _12696, _29502)), 0.0, 1.0);
        _31608 = _29506;
        _30939 = gFxData_1._data[_12612];
        _30689 = float4((mix(mix(_29282, gFxData_1._data[_12596].xyz, float3(fast::clamp(gFxData_1._data[_12596].w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _12719))) + (0.3499999940395355224609375 * ((dot(gFxData_1._data[_12596].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _12938.y) : _12938.y))), 0.0, 1.0))), (_29389 * 1.10000002384185791015625) + float3(0.07999999821186065673828125), float3(pow(1.0 - _29286.z, 5.0) * 0.3499999940395355224609375)) + float3((((0.039999999105930328369140625 * _13100) * _13100) + (0.0500000007450580596923828125 * (1.0 - _12719))) * gFxData_1._data[_12604].z)) * _27881, _27881) + (_29734 * (1.0 - _27881));
    }
    else
    {
        _31608 = 0.0;
        _30939 = _27494;
        _30689 = _29734;
    }
    float4 _30701;
    float4 _30705;
    if ((in.i_flags.x & _3835) != 0u)
    {
        float4 _29950;
        float4 _30309;
        if (in.i_flags.y != 0u)
        {
            _30309 = gFxData_1._data[(in.i_instance * 24u) + 3u];
            _29950 = gFxData_1._data[(in.i_instance * 24u) + 4u];
        }
        else
        {
            _30309 = float4(0.0);
            _29950 = float4(0.0);
        }
        float4 _30676;
        do
        {
            if (in.i_flags.y == 0u)
            {
                _30676 = in.i_fill0;
                break;
            }
            float2 _21903 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float2 _21913 = fwidth(in.i_local);
            float _21915 = fast::max(length(_21913), 9.9999997473787516355514526367188e-05);
            float _30668;
            float _30672;
            if ((in.i_flags.y == 1u) || (in.i_flags.y == 4u))
            {
                float _21924 = cos(_29950.x);
                float _21927 = sin(_29950.x);
                float _21953 = ((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_21924, _21927)) / fast::max(0.5 * ((abs(_21924) * _21903.x) + (abs(_21927) * _21903.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (in.i_flags.y == 4u)
                {
                    float _21968 = fast::clamp((_21953 - _29950.y) / fast::max(_29950.z - _29950.y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _30676 = float4(fast::clamp(abs((fract(float3(_21968 * 0.800000011920928955078125) + float3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - float3(3.0)) - float3(1.0), float3(0.0), float3(1.0)), in.i_fill0.w * pow(fast::max(sin(_21968 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _30672 = -1.0;
                _30668 = _21953;
            }
            else
            {
                float _30669;
                float _30673;
                if (in.i_flags.y == 2u)
                {
                    _30673 = -1.0;
                    _30669 = length(in.i_local - (in.i_rect.xy + (_29950.xy * _21903))) / fast::max(_29950.z * fast::max(_21903.x, _21903.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    float2 _22036 = in.i_local - (in.i_rect.xy + (_29950.xy * _21903));
                    float _22047 = fract(((precise::atan2(_22036.y, _22036.x) - _29950.z) * 0.15915493667125701904296875) + 1.0);
                    float _30670;
                    float _30674;
                    if (_29950.w > 0.5)
                    {
                        _30674 = -1.0;
                        _30670 = 0.5 - (0.5 * cos(_22047 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _22067 = (((_22047 < 0.5) ? _22047 : (_22047 - 1.0)) * 6.283185482025146484375) * length(_22036);
                        float _30675;
                        if (abs(_22067) < _21915)
                        {
                            _30675 = fast::clamp(((_22067 / _21915) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _30675 = -1.0;
                        }
                        _30674 = _30675;
                        _30670 = _22047;
                    }
                    _30673 = _30674;
                    _30669 = _30670;
                }
                _30672 = _30673;
                _30668 = _30669;
            }
            float4 _22096 = float4(in.i_fill0.xyz * in.i_fill0.w, in.i_fill0.w);
            float4 _22108 = float4(_30309.xyz * _30309.w, _30309.w);
            float4 _22122 = select(mix(_22096, _22108, float4(fast::clamp(_30668, 0.0, 1.0))), mix(_22108, _22096, float4(_30672)), bool4(_30672 >= 0.0));
            _30676 = select(float4(0.0), float4(_22122.xyz / float3(_22122.w), _22122.w), bool4(_22122.w > 9.9999997473787516355514526367188e-06));
            break;
        } while(false);
        float4 _30677;
        float4 _30928;
        if ((in.i_flags.x & _3864) != 0u)
        {
            uint _22147 = (in.i_instance * 24u) + 18u;
            _30928 = gFxData_1._data[_22147];
            _30677 = _30676 * gTex.sample(gLinear, mix(gFxData_1._data[_22147].xy, gFxData_1._data[_22147].zw, (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875))));
        }
        else
        {
            _30928 = _27490;
            _30677 = _30676;
        }
        float _22157 = fast::clamp(_30677.w * _27881, 0.0, 1.0);
        _30705 = _30928;
        _30701 = float4(_30677.xyz * _22157, _22157) + (_30689 * (1.0 - _22157));
    }
    else
    {
        _30705 = _27490;
        _30701 = _30689;
    }
    float4 _31579;
    if ((in.i_flags.x & _3917) != 0u)
    {
        uint _22181 = (in.i_instance * 24u) + 21u;
        uint _22189 = (in.i_instance * 24u) + 3u;
        float2 _5767 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float2 _5774 = (in.i_local - in.i_rect.xy) / _5767;
        float _5785 = ((gFxData_1._data[_22189].x >= 0.0) ? gFxData_1._data[_22189].x : _239.gTime.x) * gFxData_1._data[_22181].z;
        float _5788 = _5774.x * 2.0;
        float _5789 = _5788 - 1.0;
        float _5794 = _5774.y * _5767.y;
        float _5797 = fast::max(gFxData_1._data[_22181].w, 0.001000000047497451305389404296875);
        float _5802 = fast::clamp(1.0 - (_5789 * _5789), 0.0, 1.0);
        float _5804 = pow(_5802, 1.2999999523162841796875);
        float _5806 = pow(_5802, 0.699999988079071044921875);
        float _5809 = fast::clamp(_5785 * 1.4285714626312255859375, 0.0, 1.0);
        float _5825 = (0.25 + (0.75 * ((_5809 * _5809) * (3.0 - (2.0 * _5809))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_5785 * 2.099999904632568359375)));
        float _5834 = ((gFxData_1._data[_22181].y * _5767.y) * _5825) * _5804;
        float _5860 = (_5767.y * (0.5 + ((gFxData_1._data[_22181].x * (0.5 - (_5789 * _5789))) * 0.5))) + (((0.14000000059604644775390625 * _5767.y) * _5804) * sin(((_5789 * 2.400000095367431640625) - (_5785 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _30695;
        float _30696;
        float3 _30697;
        _30697 = float3(0.0);
        _30696 = _5860;
        _30695 = _5860;
        float3 _5932;
        float _32537;
        float _32538;
        for (int _30694 = 0; _30694 < 4; _30697 = _5932, _30696 = _32538, _30695 = _32537, _30694++)
        {
            float _5884 = _5860 + ((_5834 * _4079[_30694].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_5785 * 1.7000000476837158203125) + _4096[_30694].y))));
            _32537 = (_30694 == 0) ? _5884 : _30695;
            _32538 = (_30694 == 2) ? _5884 : _30696;
            float _5899 = _5797 * _4079[_30694].y;
            float _5904 = (_5794 - _5884) / _5899;
            float _5909 = _5797 * _4079[_30694].z;
            float3 _32299;
            _32299 = float3(0.0);
            for (int _32298 = 0; _32298 < 6; )
            {
                float _22214 = ((_5794 - _5884) - (_5909 * ((float(_32298) * 0.4000000059604644775390625) - 1.0))) / _5899;
                _32299 += (_2461[_32298] * exp((-_22214) * _22214));
                _32298++;
                continue;
            }
            _5932 = _30697 + (mix(_32299 * float3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), float3(exp((-_5904) * _5904)), float3(_4096[_30694].x)) * (_4079[_30694].w * _5806));
        }
        float _5938 = _5797 * 1.5;
        float _5968 = fast::clamp((_5794 - _30695) / fast::max(_30696 - _30695, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _5996 = (_5794 - (_30696 - (_5797 * 3.0))) / (((_5767.y * 0.0900000035762786865234375) + (_5834 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _5999 = (_5788 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _6024 = ((_5794 - _30695) + (_5797 * 5.0)) / (_5797 * 7.0);
        float3 _6050 = float3(1.0) - exp((-((((_30697 + (mix(float3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), float3(1.0), float3(_5968)) * ((((1.0 / (1.0 + exp((-((_5794 - _30695) - (_5797 * 2.0))) / _5938))) / (1.0 + exp((-(_30696 - _5794)) / _5938))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_5968, 2.5)))) * _5806))) + (float3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_5996) * _5996) - (_5999 * _5999)) * (0.5 + (1.10000002384185791015625 * _5825))))) + (float3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_6024) * _6024) * _5804) * 0.100000001490116119384765625))) * mix(float3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), float3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), float3(0.5 + (0.5 * sin(_5785 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _6060 = (in.i_fill0.w * smoothstep(0.0, 0.119999997317790985107421875, _5774.y)) * smoothstep(1.0, 0.87999999523162841796875, _5774.y);
        float _6079 = (fast::clamp(fast::max(_6050.x, fast::max(_6050.y, _6050.z)), 0.0, 1.0) * _6060) * _27881;
        _31579 = float4((_6050 * _6060) * _27881, _6079) + (_30701 * (1.0 - _6079));
    }
    else
    {
        _31579 = _30701;
    }
    float4 _31603;
    if (((in.i_flags.x & _4348) != 0u) && ((in.i_flags.x & _4352) != 0u))
    {
        uint _22246 = (in.i_instance * 24u) + 7u;
        uint _22254 = (in.i_instance * 24u) + 8u;
        float2 _30702;
        if (_281.gDrawInfo.w > 0.5)
        {
            float2 _22303 = select(_281.gDrawInfo.yz, _281.gDrawInfo.yz - ((in.i_rect.xy + in.i_rect.zw) * 0.5), bool2(_281.gDrawInfo.w > 1.0));
            _30702 = (-(_22303 / float2(fast::max(length(_22303), 9.9999997473787516355514526367188e-05)))) * length(gFxData_1._data[_22254].zw);
        }
        else
        {
            _30702 = gFxData_1._data[_22254].zw;
        }
        float2 _6112 = in.i_local - _30702;
        float2 _22354 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _22363 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _31499;
        if ((in.i_flags.x & _1753) != 0u)
        {
            float _22480 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
            float4 _22514 = gTex.sample(gLinear, mix(_30705.xy, _30705.zw, (fast::clamp(_6112, _27496.xy, _27496.zw) - _27496.xy) / fast::max(_27496.zw - _27496.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
            float _22538 = (_22514.w - 0.5) * in.i_radii.y;
            float2 _22579 = abs(_6112 - ((_27496.xy + _27496.zw) * 0.5)) - fast::max(((_27496.zw - _27496.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _22480), float2(0.0));
            _31499 = fast::max((-mix((fast::max(fast::min(_22514.x, _22514.y), fast::min(fast::max(_22514.x, _22514.y), _22514.z)) - 0.5) * in.i_radii.x, _22538, fast::clamp(abs(_22538) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _22480, length(fast::max(_22579, float2(0.0))) + fast::min(fast::max(_22579.x, _22579.y), 0.0)) - in.i_shape.w;
        }
        else
        {
            float _31500;
            if ((in.i_flags.x & _1768) != 0u)
            {
                float _31498;
                do
                {
                    bool _22641 = in.i_radii.w > 0.5;
                    float _22645 = floor((in.i_radii.z + 1.0) * 0.5);
                    float _22649 = floor((_22645 + 127.0) * 0.0078125);
                    float _22653 = floor((in.i_radii.z - 1.0) * 0.5);
                    float _22781 = floor(_22653 * 0.0625);
                    uint _22798 = (((in.i_instance + 1u) + uint(_22781)) * 24u) + uint(_22653 - (_22781 * 16.0));
                    float2 _31475;
                    if (_22641)
                    {
                        float _22808 = floor(0.0);
                        _31475 = gFxData_1._data[(((in.i_instance + 1u) + uint(_22808)) * 24u) + uint(_22808 * (-16.0))].xy;
                    }
                    else
                    {
                        _31475 = select(gFxData_1._data[_22798].zw, gFxData_1._data[_22798].xy, bool2(((in.i_radii.z - 1.0) - (_22653 * 2.0)) < 0.5));
                    }
                    float2 _31474;
                    float _31477;
                    float _31480;
                    _31480 = 1.0;
                    _31477 = 1000000015047466219876688855040.0;
                    _31474 = _31475;
                    float2 _31473;
                    float _31476;
                    float _31479;
                    for (float _31470 = 0.0; _31470 < 16.0; _31480 = _31479, _31477 = _31476, _31474 = _31473, _31470 += 1.0)
                    {
                        if (_31470 >= _22649)
                        {
                            break;
                        }
                        _31479 = _31480;
                        _31476 = _31477;
                        _31473 = _31474;
                        float2 _32501;
                        float _32503;
                        float _32505;
                        for (float _31471 = 0.0; _31471 < 128.0; _31479 = _32505, _31476 = _32503, _31473 = _32501, _31471 += 1.0)
                        {
                            float _22695 = (_31470 * 128.0) + _31471;
                            if (_22695 >= _22645)
                            {
                                break;
                            }
                            float _22835 = floor(_22695 * 0.0625);
                            uint _22852 = (((in.i_instance + 1u) + uint(_22835)) * 24u) + uint(_22695 - (_22835 * 16.0));
                            float2 _22865 = _31473 - gFxData_1._data[_22852].xy;
                            float2 _22868 = _6112 - gFxData_1._data[_22852].xy;
                            float2 _22881 = _22868 - (_22865 * fast::clamp(dot(_22868, _22865) / fast::max(dot(_22865, _22865), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                            float _22884 = dot(_22881, _22881);
                            float _32539 = (_22884 < _31476) ? _22884 : _31476;
                            bool _22896 = _6112.y >= gFxData_1._data[_22852].y;
                            bool _22901 = _6112.y < _31473.y;
                            bool _22912 = (_22865.x * _22868.y) > (_22865.y * _22868.x);
                            float _31485;
                            if (((_22896 && _22901) && _22912) || (((!_22896) && (!_22901)) && (!_22912)))
                            {
                                _31485 = -_31479;
                            }
                            else
                            {
                                _31485 = _31479;
                            }
                            if (((2.0 * _22695) + 1.0) < in.i_radii.z)
                            {
                                float2 _22941 = gFxData_1._data[_22852].xy - gFxData_1._data[_22852].zw;
                                float2 _22944 = _6112 - gFxData_1._data[_22852].zw;
                                float2 _22957 = _22944 - (_22941 * fast::clamp(dot(_22944, _22941) / fast::max(dot(_22941, _22941), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _22960 = dot(_22957, _22957);
                                bool _22972 = _6112.y >= gFxData_1._data[_22852].w;
                                bool _22977 = _6112.y < gFxData_1._data[_22852].y;
                                bool _22988 = (_22941.x * _22944.y) > (_22941.y * _22944.x);
                                float _31489;
                                if (((_22972 && _22977) && _22988) || (((!_22972) && (!_22977)) && (!_22988)))
                                {
                                    _31489 = -_31485;
                                }
                                else
                                {
                                    _31489 = _31485;
                                }
                                _32505 = _31489;
                                _32503 = (_22960 < _32539) ? _22960 : _32539;
                                _32501 = gFxData_1._data[_22852].zw;
                            }
                            else
                            {
                                _32505 = _31485;
                                _32503 = _32539;
                                _32501 = gFxData_1._data[_22852].xy;
                            }
                        }
                    }
                    float _22747 = sqrt(_31477);
                    if (_22641)
                    {
                        _31498 = _22747 - in.i_radii.x;
                        break;
                    }
                    _31498 = (_31480 * _22747) - in.i_radii.x;
                    break;
                } while(false);
                _31500 = _31498;
            }
            else
            {
                float _31501;
                if (in.i_flags.z == 1u)
                {
                    float2 _22388 = _6112 - _22354;
                    float _31469;
                    do
                    {
                        if (in.i_shape.w >= 6.282185077667236328125)
                        {
                            _31469 = abs(length(_22388) - in.i_radii.x) - in.i_radii.y;
                            break;
                        }
                        float _23031 = in.i_shape.z + (in.i_shape.w * 0.5);
                        float _23033 = cos(_23031);
                        float _23035 = sin(_23031);
                        float _23044 = dot(_22388, float2(-_23035, _23033));
                        float _23047 = dot(_22388, float2(_23033, _23035));
                        float2 _23048 = float2(_23044, _23047);
                        float _23051 = abs(_23044);
                        _23048.x = _23051;
                        float _23054 = in.i_shape.w * 0.5;
                        float _23056 = sin(_23054);
                        float _23058 = cos(_23054);
                        _31469 = (((_23058 * _23051) > (_23056 * _23047)) ? length(_23048 - (float2(_23056, _23058) * in.i_radii.x)) : abs(length(_23048) - in.i_radii.x)) - in.i_radii.y;
                        break;
                    } while(false);
                    _31501 = _31469;
                }
                else
                {
                    float _31502;
                    if (in.i_flags.z == 2u)
                    {
                        float2 _23094 = _6112 - _27496.xy;
                        float2 _23097 = _27496.zw - _27496.xy;
                        _31502 = length(_23094 - (_23097 * fast::clamp(dot(_23094, _23097) / fast::max(dot(_23097, _23097), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                    }
                    else
                    {
                        float2 _22415 = _6112 - _22354;
                        float _23162 = fast::min(_22363.x, _22363.y);
                        float _23165 = fast::min((_22415.x > 0.0) ? ((_22415.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_22415.y > 0.0) ? in.i_radii.w : in.i_radii.x), _23162);
                        float _23171 = _23165 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _31458;
                        float _31459;
                        if (_23171 > _23162)
                        {
                            float _23185 = in.i_shape.y * fast::clamp((_23162 - _23165) / fast::max(0.60000002384185791015625 * _23165, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _31459 = _23185;
                            _31458 = _23165 * (1.0 + (0.60000002384185791015625 * _23185));
                        }
                        else
                        {
                            _31459 = in.i_shape.y;
                            _31458 = _23171;
                        }
                        float2 _23214 = (abs(_22415) - _22363) + float2(_31458);
                        float2 _23216 = fast::max(_23214, float2(0.0));
                        float _31460;
                        if ((_23216.x > 0.0) && (_23216.y > 0.0))
                        {
                            float _31461;
                            if ((_31459 > 0.001000000047497451305389404296875) && (_31458 > 9.9999997473787516355514526367188e-05))
                            {
                                float _23233 = 2.0 + (2.0 * _31459);
                                float2 _23238 = _23216 / float2(fast::max(_31458, 9.9999997473787516355514526367188e-05));
                                _31461 = pow(pow(_23238.x, _23233) + pow(_23238.y, _23233), 1.0 / _23233) * _31458;
                            }
                            else
                            {
                                _31461 = length(_23216);
                            }
                            _31460 = _31461;
                        }
                        else
                        {
                            _31460 = fast::max(_23216.x, _23216.y);
                        }
                        float _23273 = (fast::min(fast::max(_23214.x, _23214.y), 0.0) + _31460) - _31458;
                        float _31503;
                        if ((in.i_flags.x & _1839) != 0u)
                        {
                            float2 _22443 = fast::max((_27496.zw - _27496.xy) * 0.5, float2(0.001000000047497451305389404296875));
                            float2 _22446 = _6112 - ((_27496.xy + _27496.zw) * 0.5);
                            float _23321 = fast::min(_22443.x, _22443.y);
                            float _23324 = fast::min((_22446.x > 0.0) ? ((_22446.y > 0.0) ? _30939.x : _30939.x) : ((_22446.y > 0.0) ? _30939.x : _30939.x), _23321);
                            float _23330 = _23324 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _31462;
                            float _31463;
                            if (_23330 > _23321)
                            {
                                float _23344 = in.i_shape.y * fast::clamp((_23321 - _23324) / fast::max(0.60000002384185791015625 * _23324, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _31463 = _23344;
                                _31462 = _23324 * (1.0 + (0.60000002384185791015625 * _23344));
                            }
                            else
                            {
                                _31463 = in.i_shape.y;
                                _31462 = _23330;
                            }
                            float2 _23373 = (abs(_22446) - _22443) + float2(_31462);
                            float2 _23375 = fast::max(_23373, float2(0.0));
                            float _31464;
                            if ((_23375.x > 0.0) && (_23375.y > 0.0))
                            {
                                float _31465;
                                if ((_31463 > 0.001000000047497451305389404296875) && (_31462 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _23392 = 2.0 + (2.0 * _31463);
                                    float2 _23397 = _23375 / float2(fast::max(_31462, 9.9999997473787516355514526367188e-05));
                                    _31465 = pow(pow(_23397.x, _23392) + pow(_23397.y, _23392), 1.0 / _23392) * _31462;
                                }
                                else
                                {
                                    _31465 = length(_23375);
                                }
                                _31464 = _31465;
                            }
                            else
                            {
                                _31464 = fast::max(_23375.x, _23375.y);
                            }
                            float _23432 = (fast::min(fast::max(_23373.x, _23373.y), 0.0) + _31464) - _31462;
                            float _23437 = fast::max(_30939.y, 9.9999997473787516355514526367188e-05);
                            float _23446 = fast::max(_23437 - abs(_23273 - _23432), 0.0) / _23437;
                            _31503 = fast::min(_23273, _23432) - (((_23446 * _23446) * _23437) * 0.25);
                        }
                        else
                        {
                            _31503 = _23273;
                        }
                        _31502 = _31503;
                    }
                    _31501 = _31502;
                }
                _31500 = _31501;
            }
            _31499 = _31500;
        }
        float _6121 = (_31499 + gFxData_1._data[_22254].y) / (fast::max(gFxData_1._data[_22254].x * 0.5, _5114 * 0.5) * 1.41421353816986083984375);
        float _23463 = sign(_6121);
        float _23465 = abs(_6121);
        float _23476 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_23465 * _23465))) * _23465)) * _23465);
        float _23479 = _23476 * _23476;
        float _23494 = fast::clamp(gFxData_1._data[_22246].w * ((0.5 + (0.5 * (_23463 - (_23463 / (_23479 * _23479))))) * _27881), 0.0, 1.0);
        _31603 = float4(gFxData_1._data[_22246].xyz * _23494, _23494) + (_31579 * (1.0 - _23494));
    }
    else
    {
        _31603 = _31579;
    }
    float4 _31641;
    if ((in.i_flags.x & _4412) != 0u)
    {
        uint _23518 = (in.i_instance * 24u) + 9u;
        uint _23526 = (in.i_instance * 24u) + 10u;
        float _6153 = fast::max(-_27540, 0.0) / fast::max(gFxData_1._data[_23526].z, 0.001000000047497451305389404296875);
        float _23536 = fast::clamp(gFxData_1._data[_23518].w * fast::clamp((exp(((-_6153) * _6153) * 2.2000000476837158203125) * gFxData_1._data[_23526].w) * _27881, 0.0, 1.0), 0.0, 1.0);
        _31641 = float4(gFxData_1._data[_23518].xyz * _23536, _23536) + (_31603 * (1.0 - _23536));
    }
    else
    {
        _31641 = _31603;
    }
    float3 _6184 = _31641.xyz + float3(_31608 * fast::clamp(_31641.w / fast::max(_27881, 0.001000000047497451305389404296875), 0.0, 1.0));
    float4 _27382 = _31641;
    _27382.x = _6184.x;
    _27382.y = _6184.y;
    _27382.z = _6184.z;
    float4 _31967;
    if ((in.i_flags.x & _4481) != 0u)
    {
        uint _23560 = (in.i_instance * 24u) + 5u;
        float4 _23562 = gFxData_1._data[_23560];
        uint _23568 = (in.i_instance * 24u) + 6u;
        float4 _31963;
        if (gFxData_1._data[_23568].z < 0.999000012874603271484375)
        {
            float2 _6246 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float _6257 = cos(gFxData_1._data[_23568].w);
            float _6260 = sin(gFxData_1._data[_23568].w);
            float4 _27399 = _23562;
            _27399.w = _23562.w * mix(1.0, gFxData_1._data[_23568].z, fast::clamp(((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_6257, _6260)) / fast::max(0.5 * ((abs(_6257) * _6246.x) + (abs(_6260) * _6246.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _31963 = _27399;
        }
        else
        {
            _31963 = _23562;
        }
        float _23578 = fast::clamp(_31963.w * ((_31652 >= 0.0) ? _31652 : (fast::clamp(0.5 - ((_27540 - (gFxData_1._data[_23568].x * gFxData_1._data[_23568].y)) / _5114), 0.0, 1.0) - fast::clamp(0.5 - ((_27540 + (gFxData_1._data[_23568].x * (1.0 - gFxData_1._data[_23568].y))) / _5114), 0.0, 1.0))), 0.0, 1.0);
        _31967 = float4(_31963.xyz * _23578, _23578) + (_27382 * (1.0 - _23578));
    }
    else
    {
        _31967 = _27382;
    }
    float4 _31968;
    if ((in.i_flags.x & _4615) != 0u)
    {
        float2 _6321 = (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float _6343 = exp(-pow((((_6321.x * 0.85000002384185791015625) + (_6321.y * 0.1500000059604644775390625)) - ((fract(_239.gTime.x * in.i_misc.w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _31968 = float4(_31967.xyz + float3(((_6343 * in.i_misc.z) * _27881) * fast::max(_31967.w, 0.3499999940395355224609375)), fast::max(_31967.w, ((_6343 * in.i_misc.z) * _27881) * 0.5));
    }
    else
    {
        _31968 = _31967;
    }
    float4 _32293;
    if ((in.i_flags.x & _4697) != 0u)
    {
        float3 _23603 = fract(floor(_27489).xyx * 0.103100001811981201171875);
        float3 _23612 = _23603 + float3(dot(_23603, _23603.yzx + float3(33.3300018310546875)));
        float3 _6394 = _31968.xyz + float3(((fract((_23612.x + _23612.y) * _23612.z) - 0.5) * in.i_misc.y) * _31968.w);
        float4 _27423 = _31968;
        _27423.x = _6394.x;
        _27423.y = _6394.y;
        _27423.z = _6394.z;
        _32293 = _27423;
    }
    else
    {
        _32293 = _31968;
    }
    float _32289;
    if (_281.gFade.z > 0.0)
    {
        _32289 = smoothstep(0.0, 1.0, fast::clamp((_27489.y - _281.gFade.x) / _281.gFade.z, 0.0, 1.0));
    }
    else
    {
        _32289 = 1.0;
    }
    float _32290;
    if (_281.gFade.w > 0.0)
    {
        _32290 = _32289 * smoothstep(0.0, 1.0, fast::clamp((_281.gFade.y - _27489.y) / _281.gFade.w, 0.0, 1.0));
    }
    else
    {
        _32290 = _32289;
    }
    float4 _6411 = _32293 * ((in.i_misc.x * _31980) * _32290);
    float4 _32294;
    if (((in.i_flags.x & _4737) != 0u) || ((in.i_flags.x & _4741) != 0u))
    {
        float3 _23664 = fract((floor(_27489) + float2(17.0)).xyx * 0.103100001811981201171875);
        float3 _23673 = _23664 + float3(dot(_23664, _23664.yzx + float3(33.3300018310546875)));
        float3 _6435 = _6411.xyz + float3(((fract((_23673.x + _23673.y) * _23673.z) - 0.5) * 0.0039215688593685626983642578125) * fast::clamp(_6411.w * 8.0, 0.0, 1.0));
        float4 _27435 = _6411;
        _27435.x = _6435.x;
        _27435.y = _6435.y;
        _27435.z = _6435.z;
        _32294 = _27435;
    }
    else
    {
        _32294 = _6411;
    }
    float3 _6445 = fast::max(_32294.xyz, float3(0.0));
    float4 _27441 = _32294;
    _27441.x = _6445.x;
    _27441.y = _6445.y;
    _27441.z = _6445.z;
    float4 _32295;
    if ((_239.gTime.w > 0.5) && (_32294.w > 9.9999997473787516355514526367188e-06))
    {
        float3 _23717 = fast::clamp(_27441.xyz / float3(_32294.w), float3(0.0), float3(1.0));
        float3 _23703 = select(pow((_23717 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _23717 * float3(0.077399380505084991455078125), _23717 <= float3(0.040449999272823333740234375)) * _32294.w;
        float4 _27450 = _27441;
        _27450.x = _23703.x;
        _27450.y = _23703.y;
        _27450.z = _23703.z;
        _32295 = _27450;
    }
    else
    {
        _32295 = _27441;
    }
    out._entryPointOutput = _32295;
    return out;
}

