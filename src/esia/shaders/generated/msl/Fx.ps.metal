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
constant uint _1262 = (512u & kEsiaFxFeatures);
constant uint _2362 = (512u & kEsiaFxFeatures);
constant uint _2397 = (1024u & kEsiaFxFeatures);
constant uint _2464 = (32u & kEsiaFxFeatures);
constant uint _2472 = (4u & kEsiaFxFeatures);
constant uint _2477 = (256u & kEsiaFxFeatures);
constant uint _2542 = (8u & kEsiaFxFeatures);
constant uint _2571 = (8192u & kEsiaFxFeatures);
constant uint _2788 = (32u & kEsiaFxFeatures);
constant uint _2852 = (1u & kEsiaFxFeatures);
constant uint _2881 = (64u & kEsiaFxFeatures);
constant uint _2936 = (16384u & kEsiaFxFeatures);
constant uint _3369 = (4u & kEsiaFxFeatures);
constant uint _3373 = (256u & kEsiaFxFeatures);
constant uint _3433 = (16u & kEsiaFxFeatures);
constant uint _3502 = (2u & kEsiaFxFeatures);
constant uint _3632 = (128u & kEsiaFxFeatures);
constant uint _3714 = (2048u & kEsiaFxFeatures);
constant uint _3754 = (8u & kEsiaFxFeatures);
constant uint _3758 = (4u & kEsiaFxFeatures);

constant spvUnsafeArray<float3, 6> _1726 = spvUnsafeArray<float3, 6>({ float3(1.0, 0.4199999868869781494140625, 0.2199999988079071044921875), float3(1.0, 0.699999988079071044921875, 0.300000011920928955078125), float3(0.800000011920928955078125, 0.920000016689300537109375, 0.4000000059604644775390625), float3(0.3499999940395355224609375, 0.89999997615814208984375, 0.699999988079071044921875), float3(0.4000000059604644775390625, 0.62000000476837158203125, 1.0), float3(0.660000026226043701171875, 0.5, 1.0) });
constant spvUnsafeArray<float4, 4> _3100 = spvUnsafeArray<float4, 4>({ float4(-1.0, 1.0, 3.400000095367431640625, 2.599999904632568359375), float4(-0.550000011920928955078125, 0.800000011920928955078125, 2.0, 0.800000011920928955078125), float4(0.300000011920928955078125, 1.0, 1.2000000476837158203125, 1.2999999523162841796875), float4(0.62000000476837158203125, 0.800000011920928955078125, 1.60000002384185791015625, 0.449999988079071044921875) });
constant spvUnsafeArray<float2, 4> _3117 = spvUnsafeArray<float2, 4>({ float2(0.0), float2(0.100000001490116119384765625, 1.2999999523162841796875), float2(0.550000011920928955078125, 3.900000095367431640625), float2(0.20000000298023223876953125, 5.19999980926513671875) });

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

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _184 [[buffer(0)]], constant WgtDraw& _227 [[buffer(2)]], const device gFxData& gFxData_1 [[buffer(10)]], texture2d<float> gTex [[texture(3)]], texture2d<float> gBackdrop0 [[texture(4)]], texture2d<float> gBackdrop1 [[texture(5)]], texture2d<float> gBackdrop2 [[texture(6)]], texture2d<float> gBackdrop3 [[texture(7)]], texture2d<float> gBackdrop4 [[texture(8)]], texture2d<float> gBackdrop5 [[texture(9)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float2 _5278 = gl_FragCoord.xy + _184.gConv.yy;
    float2 _15908;
    if (_184.gConv.x > 0.5)
    {
        float2 _14856 = _5278;
        _14856.y = _184.gTarget.y - _5278.y;
        _15908 = _14856;
    }
    else
    {
        _15908 = _5278;
    }
    float _4070 = dfdx(in.i_local.x);
    float _4074 = dfdy(in.i_local.x);
    float _4077 = fast::max(abs(_4070) + abs(_4074), 9.9999997473787516355514526367188e-05);
    float4 _15909;
    float4 _15911;
    if ((in.i_flags.z == 2u) || ((in.i_flags.x & _2362) != 0u))
    {
        _15911 = gFxData_1._data[(in.i_instance * 24u) + 15u];
        _15909 = gFxData_1._data[(in.i_instance * 24u) + 16u];
    }
    else
    {
        _15911 = float4(0.0);
        _15909 = float4(0.0);
    }
    float2 _5345 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
    float2 _5354 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
    float _15929;
    if (in.i_flags.z == 1u)
    {
        float2 _5360 = in.i_local - _5345;
        float _15928;
        do
        {
            if (in.i_shape.w >= 6.282185077667236328125)
            {
                _15928 = abs(length(_5360) - in.i_radii.x) - in.i_radii.y;
                break;
            }
            float _5459 = in.i_shape.z + (in.i_shape.w * 0.5);
            float _5461 = cos(_5459);
            float _5463 = sin(_5459);
            float _5472 = dot(_5360, float2(-_5463, _5461));
            float _5475 = dot(_5360, float2(_5461, _5463));
            float2 _5476 = float2(_5472, _5475);
            float _5479 = abs(_5472);
            _5476.x = _5479;
            float _5482 = in.i_shape.w * 0.5;
            float _5484 = sin(_5482);
            float _5486 = cos(_5482);
            _15928 = (((_5486 * _5479) > (_5484 * _5475)) ? length(_5476 - (float2(_5484, _5486) * in.i_radii.x)) : abs(length(_5476) - in.i_radii.x)) - in.i_radii.y;
            break;
        } while(false);
        _15929 = _15928;
    }
    else
    {
        float _15930;
        if (in.i_flags.z == 2u)
        {
            float2 _5522 = in.i_local - _15911.xy;
            float2 _5525 = _15911.zw - _15911.xy;
            _15930 = length(_5522 - (_5525 * fast::clamp(dot(_5522, _5525) / fast::max(dot(_5525, _5525), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
        }
        else
        {
            float2 _5387 = in.i_local - _5345;
            float _5578 = fast::min(_5354.x, _5354.y);
            float _5581 = fast::min((_5387.x > 0.0) ? ((_5387.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_5387.y > 0.0) ? in.i_radii.w : in.i_radii.x), _5578);
            float _5587 = _5581 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
            float _15913;
            float _15914;
            if (_5587 > _5578)
            {
                float _5601 = in.i_shape.y * fast::clamp((_5578 - _5581) / fast::max(0.60000002384185791015625 * _5581, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                _15914 = _5601;
                _15913 = _5581 * (1.0 + (0.60000002384185791015625 * _5601));
            }
            else
            {
                _15914 = in.i_shape.y;
                _15913 = _5587;
            }
            float2 _5614 = (abs(_5387) - _5354) + float2(_15913);
            float2 _5616 = fast::max(_5614, float2(0.0));
            float _15915;
            if ((_5616.x > 0.0) && (_5616.y > 0.0))
            {
                float _15916;
                if ((_15914 > 0.001000000047497451305389404296875) && (_15913 > 9.9999997473787516355514526367188e-05))
                {
                    float _5633 = 2.0 + (2.0 * _15914);
                    float2 _5638 = _5616 / float2(fast::max(_15913, 9.9999997473787516355514526367188e-05));
                    _15916 = pow(pow(_5638.x, _5633) + pow(_5638.y, _5633), 1.0 / _5633) * _15913;
                }
                else
                {
                    _15916 = length(_5616);
                }
                _15915 = _15916;
            }
            else
            {
                _15915 = fast::max(_5616.x, _5616.y);
            }
            float _5673 = (fast::min(fast::max(_5614.x, _5614.y), 0.0) + _15915) - _15913;
            float _15931;
            if ((in.i_flags.x & _1262) != 0u)
            {
                float2 _5415 = fast::max((_15911.zw - _15911.xy) * 0.5, float2(0.001000000047497451305389404296875));
                float2 _5418 = in.i_local - ((_15911.xy + _15911.zw) * 0.5);
                float _5709 = fast::min(_5415.x, _5415.y);
                float _5712 = fast::min((_5418.x > 0.0) ? ((_5418.y > 0.0) ? _15909.x : _15909.x) : ((_5418.y > 0.0) ? _15909.x : _15909.x), _5709);
                float _5718 = _5712 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                float _15919;
                float _15920;
                if (_5718 > _5709)
                {
                    float _5732 = in.i_shape.y * fast::clamp((_5709 - _5712) / fast::max(0.60000002384185791015625 * _5712, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _15920 = _5732;
                    _15919 = _5712 * (1.0 + (0.60000002384185791015625 * _5732));
                }
                else
                {
                    _15920 = in.i_shape.y;
                    _15919 = _5718;
                }
                float2 _5745 = (abs(_5418) - _5415) + float2(_15919);
                float2 _5747 = fast::max(_5745, float2(0.0));
                float _15921;
                if ((_5747.x > 0.0) && (_5747.y > 0.0))
                {
                    float _15922;
                    if ((_15920 > 0.001000000047497451305389404296875) && (_15919 > 9.9999997473787516355514526367188e-05))
                    {
                        float _5764 = 2.0 + (2.0 * _15920);
                        float2 _5769 = _5747 / float2(fast::max(_15919, 9.9999997473787516355514526367188e-05));
                        _15922 = pow(pow(_5769.x, _5764) + pow(_5769.y, _5764), 1.0 / _5764) * _15919;
                    }
                    else
                    {
                        _15922 = length(_5747);
                    }
                    _15921 = _15922;
                }
                else
                {
                    _15921 = fast::max(_5747.x, _5747.y);
                }
                float _5804 = (fast::min(fast::max(_5745.x, _5745.y), 0.0) + _15921) - _15919;
                float _5809 = fast::max(_15909.y, 9.9999997473787516355514526367188e-05);
                float _5818 = fast::max(_5809 - abs(_5673 - _5804), 0.0) / _5809;
                _15931 = fast::min(_5673, _5804) - (((_5818 * _5818) * _5809) * 0.25);
            }
            else
            {
                _15931 = _5673;
            }
            _15930 = _15931;
        }
        _15929 = _15930;
    }
    float _4102 = fast::clamp(0.5 - (_15929 / _4077), 0.0, 1.0);
    float _17854;
    if ((in.i_flags.x & _2397) != 0u)
    {
        uint _5834 = (in.i_instance * 24u) + 19u;
        uint _5842 = (in.i_instance * 24u) + 20u;
        float2 _4131 = fast::max((gFxData_1._data[_5834].zw - gFxData_1._data[_5834].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float2 _4134 = in.i_local - ((gFxData_1._data[_5834].xy + gFxData_1._data[_5834].zw) * 0.5);
        float _5880 = fast::min(_4131.x, _4131.y);
        float _5883 = fast::min((_4134.x > 0.0) ? ((_4134.y > 0.0) ? gFxData_1._data[_5842].x : gFxData_1._data[_5842].x) : ((_4134.y > 0.0) ? gFxData_1._data[_5842].x : gFxData_1._data[_5842].x), _5880);
        float _5889 = _5883 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_5842].y));
        float _15932;
        float _15933;
        if (_5889 > _5880)
        {
            float _5903 = gFxData_1._data[_5842].y * fast::clamp((_5880 - _5883) / fast::max(0.60000002384185791015625 * _5883, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _15933 = _5903;
            _15932 = _5883 * (1.0 + (0.60000002384185791015625 * _5903));
        }
        else
        {
            _15933 = gFxData_1._data[_5842].y;
            _15932 = _5889;
        }
        float2 _5916 = (abs(_4134) - _4131) + float2(_15932);
        float2 _5918 = fast::max(_5916, float2(0.0));
        float _15934;
        if ((_5918.x > 0.0) && (_5918.y > 0.0))
        {
            float _15935;
            if ((_15933 > 0.001000000047497451305389404296875) && (_15932 > 9.9999997473787516355514526367188e-05))
            {
                float _5935 = 2.0 + (2.0 * _15933);
                float2 _5940 = _5918 / float2(fast::max(_15932, 9.9999997473787516355514526367188e-05));
                _15935 = pow(pow(_5940.x, _5935) + pow(_5940.y, _5935), 1.0 / _5935) * _15932;
            }
            else
            {
                _15935 = length(_5918);
            }
            _15934 = _15935;
        }
        else
        {
            _15934 = fast::max(_5918.x, _5918.y);
        }
        float _4146 = fast::clamp(0.5 - (((fast::min(fast::max(_5916.x, _5916.y), 0.0) + _15934) - _15932) / _4077), 0.0, 1.0);
        if (_4146 <= 0.0)
        {
            discard_fragment();
        }
        _17854 = _4146;
    }
    else
    {
        _17854 = 1.0;
    }
    bool _4157 = ((in.i_flags.x & _2464) != 0u) && (_4102 >= 0.999000012874603271484375);
    float4 _16024;
    if ((((in.i_flags.x & _2472) != 0u) && (!((in.i_flags.x & _2477) != 0u))) && (!_4157))
    {
        uint _5981 = (in.i_instance * 24u) + 7u;
        uint _5989 = (in.i_instance * 24u) + 8u;
        float2 _4188 = in.i_local - gFxData_1._data[_5989].zw;
        float2 _6030 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _6039 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _15982;
        if (in.i_flags.z == 1u)
        {
            float2 _6045 = _4188 - _6030;
            float _15981;
            do
            {
                if (in.i_shape.w >= 6.282185077667236328125)
                {
                    _15981 = abs(length(_6045) - in.i_radii.x) - in.i_radii.y;
                    break;
                }
                float _6144 = in.i_shape.z + (in.i_shape.w * 0.5);
                float _6146 = cos(_6144);
                float _6148 = sin(_6144);
                float _6157 = dot(_6045, float2(-_6148, _6146));
                float _6160 = dot(_6045, float2(_6146, _6148));
                float2 _6161 = float2(_6157, _6160);
                float _6164 = abs(_6157);
                _6161.x = _6164;
                float _6167 = in.i_shape.w * 0.5;
                float _6169 = sin(_6167);
                float _6171 = cos(_6167);
                _15981 = (((_6171 * _6164) > (_6169 * _6160)) ? length(_6161 - (float2(_6169, _6171) * in.i_radii.x)) : abs(length(_6161) - in.i_radii.x)) - in.i_radii.y;
                break;
            } while(false);
            _15982 = _15981;
        }
        else
        {
            float _15983;
            if (in.i_flags.z == 2u)
            {
                float2 _6207 = _4188 - _15911.xy;
                float2 _6210 = _15911.zw - _15911.xy;
                _15983 = length(_6207 - (_6210 * fast::clamp(dot(_6207, _6210) / fast::max(dot(_6210, _6210), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
            }
            else
            {
                float2 _6072 = _4188 - _6030;
                float _6263 = fast::min(_6039.x, _6039.y);
                float _6266 = fast::min((_6072.x > 0.0) ? ((_6072.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_6072.y > 0.0) ? in.i_radii.w : in.i_radii.x), _6263);
                float _6272 = _6266 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                float _15966;
                float _15967;
                if (_6272 > _6263)
                {
                    float _6286 = in.i_shape.y * fast::clamp((_6263 - _6266) / fast::max(0.60000002384185791015625 * _6266, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _15967 = _6286;
                    _15966 = _6266 * (1.0 + (0.60000002384185791015625 * _6286));
                }
                else
                {
                    _15967 = in.i_shape.y;
                    _15966 = _6272;
                }
                float2 _6299 = (abs(_6072) - _6039) + float2(_15966);
                float2 _6301 = fast::max(_6299, float2(0.0));
                float _15968;
                if ((_6301.x > 0.0) && (_6301.y > 0.0))
                {
                    float _15969;
                    if ((_15967 > 0.001000000047497451305389404296875) && (_15966 > 9.9999997473787516355514526367188e-05))
                    {
                        float _6318 = 2.0 + (2.0 * _15967);
                        float2 _6323 = _6301 / float2(fast::max(_15966, 9.9999997473787516355514526367188e-05));
                        _15969 = pow(pow(_6323.x, _6318) + pow(_6323.y, _6318), 1.0 / _6318) * _15966;
                    }
                    else
                    {
                        _15969 = length(_6301);
                    }
                    _15968 = _15969;
                }
                else
                {
                    _15968 = fast::max(_6301.x, _6301.y);
                }
                float _6358 = (fast::min(fast::max(_6299.x, _6299.y), 0.0) + _15968) - _15966;
                float _15984;
                if ((in.i_flags.x & _1262) != 0u)
                {
                    float2 _6100 = fast::max((_15911.zw - _15911.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _6103 = _4188 - ((_15911.xy + _15911.zw) * 0.5);
                    float _6394 = fast::min(_6100.x, _6100.y);
                    float _6397 = fast::min((_6103.x > 0.0) ? ((_6103.y > 0.0) ? _15909.x : _15909.x) : ((_6103.y > 0.0) ? _15909.x : _15909.x), _6394);
                    float _6403 = _6397 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _15972;
                    float _15973;
                    if (_6403 > _6394)
                    {
                        float _6417 = in.i_shape.y * fast::clamp((_6394 - _6397) / fast::max(0.60000002384185791015625 * _6397, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _15973 = _6417;
                        _15972 = _6397 * (1.0 + (0.60000002384185791015625 * _6417));
                    }
                    else
                    {
                        _15973 = in.i_shape.y;
                        _15972 = _6403;
                    }
                    float2 _6430 = (abs(_6103) - _6100) + float2(_15972);
                    float2 _6432 = fast::max(_6430, float2(0.0));
                    float _15974;
                    if ((_6432.x > 0.0) && (_6432.y > 0.0))
                    {
                        float _15975;
                        if ((_15973 > 0.001000000047497451305389404296875) && (_15972 > 9.9999997473787516355514526367188e-05))
                        {
                            float _6449 = 2.0 + (2.0 * _15973);
                            float2 _6454 = _6432 / float2(fast::max(_15972, 9.9999997473787516355514526367188e-05));
                            _15975 = pow(pow(_6454.x, _6449) + pow(_6454.y, _6449), 1.0 / _6449) * _15972;
                        }
                        else
                        {
                            _15975 = length(_6432);
                        }
                        _15974 = _15975;
                    }
                    else
                    {
                        _15974 = fast::max(_6432.x, _6432.y);
                    }
                    float _6489 = (fast::min(fast::max(_6430.x, _6430.y), 0.0) + _15974) - _15972;
                    float _6494 = fast::max(_15909.y, 9.9999997473787516355514526367188e-05);
                    float _6503 = fast::max(_6494 - abs(_6358 - _6489), 0.0) / _6494;
                    _15984 = fast::min(_6358, _6489) - (((_6503 * _6503) * _6494) * 0.25);
                }
                else
                {
                    _15984 = _6358;
                }
                _15983 = _15984;
            }
            _15982 = _15983;
        }
        float _4197 = (_15982 - gFxData_1._data[_5989].y) / (fast::max(gFxData_1._data[_5989].x * 0.5, _4077 * 0.5) * 1.41421353816986083984375);
        float _6520 = sign(_4197);
        float _6522 = abs(_4197);
        float _6533 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_6522 * _6522))) * _6522)) * _6522);
        float _6536 = _6533 * _6533;
        float _6551 = fast::clamp(gFxData_1._data[_5981].w * (0.5 - (0.5 * (_6520 - (_6520 / (_6536 * _6536))))), 0.0, 1.0);
        _16024 = float4(gFxData_1._data[_5981].xyz * _6551, _6551);
    }
    else
    {
        _16024 = float4(0.0);
    }
    float4 _16915;
    if (((in.i_flags.x & _2542) != 0u) && (!_4157))
    {
        uint _6575 = (in.i_instance * 24u) + 9u;
        uint _6583 = (in.i_instance * 24u) + 10u;
        float _4225 = fast::max(gFxData_1._data[_6583].x, 0.001000000047497451305389404296875);
        float _16017;
        float _16020;
        if ((in.i_flags.x & _2571) != 0u)
        {
            uint _6591 = (in.i_instance * 24u) + 22u;
            float2 _4241 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _4250 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float2 _4257 = in.i_rect.xy - gFxData_1._data[_6591].xy;
            float2 _4264 = gFxData_1._data[_6591].zw - in.i_rect.zw;
            float2 _4294 = fast::clamp(float2((in.i_local.x < _4241.x) ? _4257.x : _4264.x, (in.i_local.y < _4241.y) ? _4257.y : _4264.y) * float2(0.58823525905609130859375), float2(fast::min(_4225, 1.5)), float2(_4225));
            float _4299 = fast::min(_4250.x, _4250.y);
            float _16015;
            if (in.i_flags.z == 0u)
            {
                _16015 = fast::min(((in.i_local.x > _4241.x) ? ((in.i_local.y > _4241.y) ? in.i_radii.z : in.i_radii.y) : ((in.i_local.y > _4241.y) ? in.i_radii.w : in.i_radii.x)) * (1.0 + (0.60000002384185791015625 * in.i_shape.y)), _4299);
            }
            else
            {
                _16015 = _4299;
            }
            float2 _4348 = fast::max(abs(in.i_local - _4241) - (_4250 - float2(_16015)), float2(0.0));
            float _4350 = length(_4348);
            float2 _4358 = select(float2(0.707099974155426025390625), _4348 / float2(_4350), bool2(_4350 > 9.9999997473787516355514526367188e-05));
            float2 _4373 = in.i_local - gFxData_1._data[_6591].xy;
            float2 _4378 = gFxData_1._data[_6591].zw - in.i_local;
            _16020 = fast::clamp(fast::min(fast::min(_4373.x, _4373.y), fast::min(_4378.x, _4378.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _16017 = rsqrt(dot(_4358 * _4358, float2(1.0) / (_4294 * _4294)));
        }
        else
        {
            _16020 = 1.0;
            _16017 = _4225;
        }
        float _4396 = fast::max(_15929, 0.0) / _16017;
        float _6601 = fast::clamp(gFxData_1._data[_6575].w * fast::clamp((exp(((-_4396) * _4396) * 2.2000000476837158203125) * gFxData_1._data[_6583].y) * _16020, 0.0, 1.0), 0.0, 1.0);
        _16915 = float4(gFxData_1._data[_6575].xyz * _6601, _6601) + (_16024 * (1.0 - _6601));
    }
    else
    {
        _16915 = _16024;
    }
    float4 _17426;
    float4 _17439;
    float _17820;
    if (((in.i_flags.x & _2788) != 0u) && (_4102 > 0.0))
    {
        uint _6625 = (in.i_instance * 24u) + 11u;
        uint _6633 = (in.i_instance * 24u) + 12u;
        uint _6641 = (in.i_instance * 24u) + 13u;
        uint _6649 = (in.i_instance * 24u) + 16u;
        float _6721 = gFxData_1._data[_6625].x * _184.gDisplay.z;
        float2 _6732 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(1.0));
        float _6740 = fast::clamp(gFxData_1._data[_6625].z, 0.001000000047497451305389404296875, fast::min(_6732.x, _6732.y));
        float _6749 = fast::clamp(1.0 - (fast::max(-_15929, 0.0) / _6740), 0.0, 1.0);
        float _6755 = sqrt(fast::clamp(1.0 - (_6749 * _6749), 0.0, 1.0));
        float2 _16116;
        float3 _16693;
        if (_6749 > 0.0)
        {
            float2 _7162 = in.i_local + float2(0.5, 0.0);
            float2 _7230 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _7239 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _16056;
            if (in.i_flags.z == 1u)
            {
                float2 _7245 = _7162 - _7230;
                float _16055;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _16055 = abs(length(_7245) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _7344 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _7346 = cos(_7344);
                    float _7348 = sin(_7344);
                    float _7357 = dot(_7245, float2(-_7348, _7346));
                    float _7360 = dot(_7245, float2(_7346, _7348));
                    float2 _7361 = float2(_7357, _7360);
                    float _7364 = abs(_7357);
                    _7361.x = _7364;
                    float _7367 = in.i_shape.w * 0.5;
                    float _7369 = sin(_7367);
                    float _7371 = cos(_7367);
                    _16055 = (((_7371 * _7364) > (_7369 * _7360)) ? length(_7361 - (float2(_7369, _7371) * in.i_radii.x)) : abs(length(_7361) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _16056 = _16055;
            }
            else
            {
                float _16057;
                if (in.i_flags.z == 2u)
                {
                    float2 _7407 = _7162 - _15911.xy;
                    float2 _7410 = _15911.zw - _15911.xy;
                    _16057 = length(_7407 - (_7410 * fast::clamp(dot(_7407, _7410) / fast::max(dot(_7410, _7410), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _7272 = _7162 - _7230;
                    float _7463 = fast::min(_7239.x, _7239.y);
                    float _7466 = fast::min((_7272.x > 0.0) ? ((_7272.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_7272.y > 0.0) ? in.i_radii.w : in.i_radii.x), _7463);
                    float _7472 = _7466 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _16040;
                    float _16041;
                    if (_7472 > _7463)
                    {
                        float _7486 = in.i_shape.y * fast::clamp((_7463 - _7466) / fast::max(0.60000002384185791015625 * _7466, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16041 = _7486;
                        _16040 = _7466 * (1.0 + (0.60000002384185791015625 * _7486));
                    }
                    else
                    {
                        _16041 = in.i_shape.y;
                        _16040 = _7472;
                    }
                    float2 _7499 = (abs(_7272) - _7239) + float2(_16040);
                    float2 _7501 = fast::max(_7499, float2(0.0));
                    float _16042;
                    if ((_7501.x > 0.0) && (_7501.y > 0.0))
                    {
                        float _16043;
                        if ((_16041 > 0.001000000047497451305389404296875) && (_16040 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7518 = 2.0 + (2.0 * _16041);
                            float2 _7523 = _7501 / float2(fast::max(_16040, 9.9999997473787516355514526367188e-05));
                            _16043 = pow(pow(_7523.x, _7518) + pow(_7523.y, _7518), 1.0 / _7518) * _16040;
                        }
                        else
                        {
                            _16043 = length(_7501);
                        }
                        _16042 = _16043;
                    }
                    else
                    {
                        _16042 = fast::max(_7501.x, _7501.y);
                    }
                    float _7558 = (fast::min(fast::max(_7499.x, _7499.y), 0.0) + _16042) - _16040;
                    float _16058;
                    if ((in.i_flags.x & _1262) != 0u)
                    {
                        float2 _7300 = fast::max((_15911.zw - _15911.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _7303 = _7162 - ((_15911.xy + _15911.zw) * 0.5);
                        float _7594 = fast::min(_7300.x, _7300.y);
                        float _7597 = fast::min((_7303.x > 0.0) ? ((_7303.y > 0.0) ? gFxData_1._data[_6649].x : gFxData_1._data[_6649].x) : ((_7303.y > 0.0) ? gFxData_1._data[_6649].x : gFxData_1._data[_6649].x), _7594);
                        float _7603 = _7597 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _16046;
                        float _16047;
                        if (_7603 > _7594)
                        {
                            float _7617 = in.i_shape.y * fast::clamp((_7594 - _7597) / fast::max(0.60000002384185791015625 * _7597, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _16047 = _7617;
                            _16046 = _7597 * (1.0 + (0.60000002384185791015625 * _7617));
                        }
                        else
                        {
                            _16047 = in.i_shape.y;
                            _16046 = _7603;
                        }
                        float2 _7630 = (abs(_7303) - _7300) + float2(_16046);
                        float2 _7632 = fast::max(_7630, float2(0.0));
                        float _16048;
                        if ((_7632.x > 0.0) && (_7632.y > 0.0))
                        {
                            float _16049;
                            if ((_16047 > 0.001000000047497451305389404296875) && (_16046 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7649 = 2.0 + (2.0 * _16047);
                                float2 _7654 = _7632 / float2(fast::max(_16046, 9.9999997473787516355514526367188e-05));
                                _16049 = pow(pow(_7654.x, _7649) + pow(_7654.y, _7649), 1.0 / _7649) * _16046;
                            }
                            else
                            {
                                _16049 = length(_7632);
                            }
                            _16048 = _16049;
                        }
                        else
                        {
                            _16048 = fast::max(_7632.x, _7632.y);
                        }
                        float _7689 = (fast::min(fast::max(_7630.x, _7630.y), 0.0) + _16048) - _16046;
                        float _7694 = fast::max(gFxData_1._data[_6649].y, 9.9999997473787516355514526367188e-05);
                        float _7703 = fast::max(_7694 - abs(_7558 - _7689), 0.0) / _7694;
                        _16058 = fast::min(_7558, _7689) - (((_7703 * _7703) * _7694) * 0.25);
                    }
                    else
                    {
                        _16058 = _7558;
                    }
                    _16057 = _16058;
                }
                _16056 = _16057;
            }
            float2 _7166 = in.i_local - float2(0.5, 0.0);
            float2 _7752 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _7761 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _16075;
            if (in.i_flags.z == 1u)
            {
                float2 _7767 = _7166 - _7752;
                float _16074;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _16074 = abs(length(_7767) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _7866 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _7868 = cos(_7866);
                    float _7870 = sin(_7866);
                    float _7879 = dot(_7767, float2(-_7870, _7868));
                    float _7882 = dot(_7767, float2(_7868, _7870));
                    float2 _7883 = float2(_7879, _7882);
                    float _7886 = abs(_7879);
                    _7883.x = _7886;
                    float _7889 = in.i_shape.w * 0.5;
                    float _7891 = sin(_7889);
                    float _7893 = cos(_7889);
                    _16074 = (((_7893 * _7886) > (_7891 * _7882)) ? length(_7883 - (float2(_7891, _7893) * in.i_radii.x)) : abs(length(_7883) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _16075 = _16074;
            }
            else
            {
                float _16076;
                if (in.i_flags.z == 2u)
                {
                    float2 _7929 = _7166 - _15911.xy;
                    float2 _7932 = _15911.zw - _15911.xy;
                    _16076 = length(_7929 - (_7932 * fast::clamp(dot(_7929, _7932) / fast::max(dot(_7932, _7932), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _7794 = _7166 - _7752;
                    float _7985 = fast::min(_7761.x, _7761.y);
                    float _7988 = fast::min((_7794.x > 0.0) ? ((_7794.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_7794.y > 0.0) ? in.i_radii.w : in.i_radii.x), _7985);
                    float _7994 = _7988 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _16059;
                    float _16060;
                    if (_7994 > _7985)
                    {
                        float _8008 = in.i_shape.y * fast::clamp((_7985 - _7988) / fast::max(0.60000002384185791015625 * _7988, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16060 = _8008;
                        _16059 = _7988 * (1.0 + (0.60000002384185791015625 * _8008));
                    }
                    else
                    {
                        _16060 = in.i_shape.y;
                        _16059 = _7994;
                    }
                    float2 _8021 = (abs(_7794) - _7761) + float2(_16059);
                    float2 _8023 = fast::max(_8021, float2(0.0));
                    float _16061;
                    if ((_8023.x > 0.0) && (_8023.y > 0.0))
                    {
                        float _16062;
                        if ((_16060 > 0.001000000047497451305389404296875) && (_16059 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8040 = 2.0 + (2.0 * _16060);
                            float2 _8045 = _8023 / float2(fast::max(_16059, 9.9999997473787516355514526367188e-05));
                            _16062 = pow(pow(_8045.x, _8040) + pow(_8045.y, _8040), 1.0 / _8040) * _16059;
                        }
                        else
                        {
                            _16062 = length(_8023);
                        }
                        _16061 = _16062;
                    }
                    else
                    {
                        _16061 = fast::max(_8023.x, _8023.y);
                    }
                    float _8080 = (fast::min(fast::max(_8021.x, _8021.y), 0.0) + _16061) - _16059;
                    float _16077;
                    if ((in.i_flags.x & _1262) != 0u)
                    {
                        float2 _7822 = fast::max((_15911.zw - _15911.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _7825 = _7166 - ((_15911.xy + _15911.zw) * 0.5);
                        float _8116 = fast::min(_7822.x, _7822.y);
                        float _8119 = fast::min((_7825.x > 0.0) ? ((_7825.y > 0.0) ? gFxData_1._data[_6649].x : gFxData_1._data[_6649].x) : ((_7825.y > 0.0) ? gFxData_1._data[_6649].x : gFxData_1._data[_6649].x), _8116);
                        float _8125 = _8119 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _16065;
                        float _16066;
                        if (_8125 > _8116)
                        {
                            float _8139 = in.i_shape.y * fast::clamp((_8116 - _8119) / fast::max(0.60000002384185791015625 * _8119, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _16066 = _8139;
                            _16065 = _8119 * (1.0 + (0.60000002384185791015625 * _8139));
                        }
                        else
                        {
                            _16066 = in.i_shape.y;
                            _16065 = _8125;
                        }
                        float2 _8152 = (abs(_7825) - _7822) + float2(_16065);
                        float2 _8154 = fast::max(_8152, float2(0.0));
                        float _16067;
                        if ((_8154.x > 0.0) && (_8154.y > 0.0))
                        {
                            float _16068;
                            if ((_16066 > 0.001000000047497451305389404296875) && (_16065 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8171 = 2.0 + (2.0 * _16066);
                                float2 _8176 = _8154 / float2(fast::max(_16065, 9.9999997473787516355514526367188e-05));
                                _16068 = pow(pow(_8176.x, _8171) + pow(_8176.y, _8171), 1.0 / _8171) * _16065;
                            }
                            else
                            {
                                _16068 = length(_8154);
                            }
                            _16067 = _16068;
                        }
                        else
                        {
                            _16067 = fast::max(_8154.x, _8154.y);
                        }
                        float _8211 = (fast::min(fast::max(_8152.x, _8152.y), 0.0) + _16067) - _16065;
                        float _8216 = fast::max(gFxData_1._data[_6649].y, 9.9999997473787516355514526367188e-05);
                        float _8225 = fast::max(_8216 - abs(_8080 - _8211), 0.0) / _8216;
                        _16077 = fast::min(_8080, _8211) - (((_8225 * _8225) * _8216) * 0.25);
                    }
                    else
                    {
                        _16077 = _8080;
                    }
                    _16076 = _16077;
                }
                _16075 = _16076;
            }
            float2 _7171 = in.i_local + float2(0.0, 0.5);
            float2 _8274 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _8283 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _16094;
            if (in.i_flags.z == 1u)
            {
                float2 _8289 = _7171 - _8274;
                float _16093;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _16093 = abs(length(_8289) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _8388 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _8390 = cos(_8388);
                    float _8392 = sin(_8388);
                    float _8401 = dot(_8289, float2(-_8392, _8390));
                    float _8404 = dot(_8289, float2(_8390, _8392));
                    float2 _8405 = float2(_8401, _8404);
                    float _8408 = abs(_8401);
                    _8405.x = _8408;
                    float _8411 = in.i_shape.w * 0.5;
                    float _8413 = sin(_8411);
                    float _8415 = cos(_8411);
                    _16093 = (((_8415 * _8408) > (_8413 * _8404)) ? length(_8405 - (float2(_8413, _8415) * in.i_radii.x)) : abs(length(_8405) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _16094 = _16093;
            }
            else
            {
                float _16095;
                if (in.i_flags.z == 2u)
                {
                    float2 _8451 = _7171 - _15911.xy;
                    float2 _8454 = _15911.zw - _15911.xy;
                    _16095 = length(_8451 - (_8454 * fast::clamp(dot(_8451, _8454) / fast::max(dot(_8454, _8454), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _8316 = _7171 - _8274;
                    float _8507 = fast::min(_8283.x, _8283.y);
                    float _8510 = fast::min((_8316.x > 0.0) ? ((_8316.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_8316.y > 0.0) ? in.i_radii.w : in.i_radii.x), _8507);
                    float _8516 = _8510 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _16078;
                    float _16079;
                    if (_8516 > _8507)
                    {
                        float _8530 = in.i_shape.y * fast::clamp((_8507 - _8510) / fast::max(0.60000002384185791015625 * _8510, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16079 = _8530;
                        _16078 = _8510 * (1.0 + (0.60000002384185791015625 * _8530));
                    }
                    else
                    {
                        _16079 = in.i_shape.y;
                        _16078 = _8516;
                    }
                    float2 _8543 = (abs(_8316) - _8283) + float2(_16078);
                    float2 _8545 = fast::max(_8543, float2(0.0));
                    float _16080;
                    if ((_8545.x > 0.0) && (_8545.y > 0.0))
                    {
                        float _16081;
                        if ((_16079 > 0.001000000047497451305389404296875) && (_16078 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8562 = 2.0 + (2.0 * _16079);
                            float2 _8567 = _8545 / float2(fast::max(_16078, 9.9999997473787516355514526367188e-05));
                            _16081 = pow(pow(_8567.x, _8562) + pow(_8567.y, _8562), 1.0 / _8562) * _16078;
                        }
                        else
                        {
                            _16081 = length(_8545);
                        }
                        _16080 = _16081;
                    }
                    else
                    {
                        _16080 = fast::max(_8545.x, _8545.y);
                    }
                    float _8602 = (fast::min(fast::max(_8543.x, _8543.y), 0.0) + _16080) - _16078;
                    float _16096;
                    if ((in.i_flags.x & _1262) != 0u)
                    {
                        float2 _8344 = fast::max((_15911.zw - _15911.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _8347 = _7171 - ((_15911.xy + _15911.zw) * 0.5);
                        float _8638 = fast::min(_8344.x, _8344.y);
                        float _8641 = fast::min((_8347.x > 0.0) ? ((_8347.y > 0.0) ? gFxData_1._data[_6649].x : gFxData_1._data[_6649].x) : ((_8347.y > 0.0) ? gFxData_1._data[_6649].x : gFxData_1._data[_6649].x), _8638);
                        float _8647 = _8641 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _16084;
                        float _16085;
                        if (_8647 > _8638)
                        {
                            float _8661 = in.i_shape.y * fast::clamp((_8638 - _8641) / fast::max(0.60000002384185791015625 * _8641, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _16085 = _8661;
                            _16084 = _8641 * (1.0 + (0.60000002384185791015625 * _8661));
                        }
                        else
                        {
                            _16085 = in.i_shape.y;
                            _16084 = _8647;
                        }
                        float2 _8674 = (abs(_8347) - _8344) + float2(_16084);
                        float2 _8676 = fast::max(_8674, float2(0.0));
                        float _16086;
                        if ((_8676.x > 0.0) && (_8676.y > 0.0))
                        {
                            float _16087;
                            if ((_16085 > 0.001000000047497451305389404296875) && (_16084 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8693 = 2.0 + (2.0 * _16085);
                                float2 _8698 = _8676 / float2(fast::max(_16084, 9.9999997473787516355514526367188e-05));
                                _16087 = pow(pow(_8698.x, _8693) + pow(_8698.y, _8693), 1.0 / _8693) * _16084;
                            }
                            else
                            {
                                _16087 = length(_8676);
                            }
                            _16086 = _16087;
                        }
                        else
                        {
                            _16086 = fast::max(_8676.x, _8676.y);
                        }
                        float _8733 = (fast::min(fast::max(_8674.x, _8674.y), 0.0) + _16086) - _16084;
                        float _8738 = fast::max(gFxData_1._data[_6649].y, 9.9999997473787516355514526367188e-05);
                        float _8747 = fast::max(_8738 - abs(_8602 - _8733), 0.0) / _8738;
                        _16096 = fast::min(_8602, _8733) - (((_8747 * _8747) * _8738) * 0.25);
                    }
                    else
                    {
                        _16096 = _8602;
                    }
                    _16095 = _16096;
                }
                _16094 = _16095;
            }
            float2 _7175 = in.i_local - float2(0.0, 0.5);
            float2 _8796 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _8805 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _16113;
            if (in.i_flags.z == 1u)
            {
                float2 _8811 = _7175 - _8796;
                float _16112;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _16112 = abs(length(_8811) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _8910 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _8912 = cos(_8910);
                    float _8914 = sin(_8910);
                    float _8923 = dot(_8811, float2(-_8914, _8912));
                    float _8926 = dot(_8811, float2(_8912, _8914));
                    float2 _8927 = float2(_8923, _8926);
                    float _8930 = abs(_8923);
                    _8927.x = _8930;
                    float _8933 = in.i_shape.w * 0.5;
                    float _8935 = sin(_8933);
                    float _8937 = cos(_8933);
                    _16112 = (((_8937 * _8930) > (_8935 * _8926)) ? length(_8927 - (float2(_8935, _8937) * in.i_radii.x)) : abs(length(_8927) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _16113 = _16112;
            }
            else
            {
                float _16114;
                if (in.i_flags.z == 2u)
                {
                    float2 _8973 = _7175 - _15911.xy;
                    float2 _8976 = _15911.zw - _15911.xy;
                    _16114 = length(_8973 - (_8976 * fast::clamp(dot(_8973, _8976) / fast::max(dot(_8976, _8976), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _8838 = _7175 - _8796;
                    float _9029 = fast::min(_8805.x, _8805.y);
                    float _9032 = fast::min((_8838.x > 0.0) ? ((_8838.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_8838.y > 0.0) ? in.i_radii.w : in.i_radii.x), _9029);
                    float _9038 = _9032 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _16097;
                    float _16098;
                    if (_9038 > _9029)
                    {
                        float _9052 = in.i_shape.y * fast::clamp((_9029 - _9032) / fast::max(0.60000002384185791015625 * _9032, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16098 = _9052;
                        _16097 = _9032 * (1.0 + (0.60000002384185791015625 * _9052));
                    }
                    else
                    {
                        _16098 = in.i_shape.y;
                        _16097 = _9038;
                    }
                    float2 _9065 = (abs(_8838) - _8805) + float2(_16097);
                    float2 _9067 = fast::max(_9065, float2(0.0));
                    float _16099;
                    if ((_9067.x > 0.0) && (_9067.y > 0.0))
                    {
                        float _16100;
                        if ((_16098 > 0.001000000047497451305389404296875) && (_16097 > 9.9999997473787516355514526367188e-05))
                        {
                            float _9084 = 2.0 + (2.0 * _16098);
                            float2 _9089 = _9067 / float2(fast::max(_16097, 9.9999997473787516355514526367188e-05));
                            _16100 = pow(pow(_9089.x, _9084) + pow(_9089.y, _9084), 1.0 / _9084) * _16097;
                        }
                        else
                        {
                            _16100 = length(_9067);
                        }
                        _16099 = _16100;
                    }
                    else
                    {
                        _16099 = fast::max(_9067.x, _9067.y);
                    }
                    float _9124 = (fast::min(fast::max(_9065.x, _9065.y), 0.0) + _16099) - _16097;
                    float _16115;
                    if ((in.i_flags.x & _1262) != 0u)
                    {
                        float2 _8866 = fast::max((_15911.zw - _15911.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _8869 = _7175 - ((_15911.xy + _15911.zw) * 0.5);
                        float _9160 = fast::min(_8866.x, _8866.y);
                        float _9163 = fast::min((_8869.x > 0.0) ? ((_8869.y > 0.0) ? gFxData_1._data[_6649].x : gFxData_1._data[_6649].x) : ((_8869.y > 0.0) ? gFxData_1._data[_6649].x : gFxData_1._data[_6649].x), _9160);
                        float _9169 = _9163 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _16103;
                        float _16104;
                        if (_9169 > _9160)
                        {
                            float _9183 = in.i_shape.y * fast::clamp((_9160 - _9163) / fast::max(0.60000002384185791015625 * _9163, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _16104 = _9183;
                            _16103 = _9163 * (1.0 + (0.60000002384185791015625 * _9183));
                        }
                        else
                        {
                            _16104 = in.i_shape.y;
                            _16103 = _9169;
                        }
                        float2 _9196 = (abs(_8869) - _8866) + float2(_16103);
                        float2 _9198 = fast::max(_9196, float2(0.0));
                        float _16105;
                        if ((_9198.x > 0.0) && (_9198.y > 0.0))
                        {
                            float _16106;
                            if ((_16104 > 0.001000000047497451305389404296875) && (_16103 > 9.9999997473787516355514526367188e-05))
                            {
                                float _9215 = 2.0 + (2.0 * _16104);
                                float2 _9220 = _9198 / float2(fast::max(_16103, 9.9999997473787516355514526367188e-05));
                                _16106 = pow(pow(_9220.x, _9215) + pow(_9220.y, _9215), 1.0 / _9215) * _16103;
                            }
                            else
                            {
                                _16106 = length(_9198);
                            }
                            _16105 = _16106;
                        }
                        else
                        {
                            _16105 = fast::max(_9198.x, _9198.y);
                        }
                        float _9255 = (fast::min(fast::max(_9196.x, _9196.y), 0.0) + _16105) - _16103;
                        float _9260 = fast::max(gFxData_1._data[_6649].y, 9.9999997473787516355514526367188e-05);
                        float _9269 = fast::max(_9260 - abs(_9124 - _9255), 0.0) / _9260;
                        _16115 = fast::min(_9124, _9255) - (((_9269 * _9269) * _9260) * 0.25);
                    }
                    else
                    {
                        _16115 = _9124;
                    }
                    _16114 = _16115;
                }
                _16113 = _16114;
            }
            float2 _7181 = float2(_16056 - _16075, _16094 - _16113);
            float _7183 = length(_7181);
            float2 _7191 = select(float2(0.0, -1.0), _7181 / float2(_7183), bool2(_7183 > 9.9999997473787516355514526367188e-06));
            _16693 = fast::normalize(float3(_7191 * fast::min(_6749 / fast::max(_6755, 0.001000000047497451305389404296875), 8.0), 1.0));
            _16116 = _7191;
        }
        else
        {
            _16693 = float3(0.0, 0.0, 1.0);
            _16116 = float2(0.0, -1.0);
        }
        float2 _6781 = ((-_16116) * gFxData_1._data[_6625].y) * (1.0 - _6755);
        float2 _16117;
        if (gFxData_1._data[_6649].z > 0.0)
        {
            _16117 = (((in.i_rect.xy + in.i_rect.zw) * 0.5) - in.i_local) * (gFxData_1._data[_6649].z / (1.0 + gFxData_1._data[_6649].z));
        }
        else
        {
            _16117 = float2(0.0);
        }
        float2 _6807 = _15908 * _184.gTarget.zw;
        float2 _6814 = _184.gDisplay.zw * _184.gTarget.zw;
        float2 _6819 = (_6781 + _16117) * _6814;
        float2 _6822 = _6781 * _6814;
        float3 _16575;
        float _16593;
        float3 _16796;
        if (_184.gTime.z > 0.5)
        {
            float2 _6829 = _6807 + _6819;
            float _9294 = fast::clamp(log2(fast::max(_6721, 1.0)) - 1.0, 0.0, 5.0);
            int _9297 = int(floor(_9294));
            float _9301 = _9294 - float(_9297);
            bool _9306 = (_9301 > 0.0199999995529651641845703125) && (_9297 < 5);
            float3 _16188;
            _16188 = float3(0.0);
            float3 _9335;
            for (int _16118 = 0; _16118 < 2; _16188 = _9335, _16118++)
            {
                if ((_16118 > 0) && (!_9306))
                {
                    break;
                }
                int _9321 = _9297 + _16118;
                int _9359 = clamp(_9321, 1, 5);
                float2 _9428 = (_6829 * _184.gLevel[_9359].xy) - float2(0.5);
                float2 _9430 = floor(_9428);
                float2 _9433 = _9428 - _9430;
                float2 _9436 = _9433 * _9433;
                float2 _9439 = _9436 * _9433;
                float2 _9458 = (((_9439 * 3.0) - (_9436 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                float2 _9471 = _9439 * 0.16666667163372039794921875;
                float2 _9474 = (((((-_9439) + (_9436 * 3.0)) - (_9433 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9458;
                float2 _9478 = (((((_9439 * (-3.0)) + (_9436 * 3.0)) + (_9433 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9471;
                float2 _9490 = ((_9430 - float2(0.5)) + (_9458 / _9474)) * _184.gLevel[_9359].zw;
                float2 _9502 = ((_9430 + float2(1.5)) + (_9471 / _9478)) * _184.gLevel[_9359].zw;
                float4 _16155;
                if (_9321 <= 0)
                {
                    float2 _16154;
                    if (_184.gConv.x > 0.5)
                    {
                        float2 _15239 = _6829;
                        _15239.y = 1.0 - _6829.y;
                        _16154 = _15239;
                    }
                    else
                    {
                        _16154 = _6829;
                    }
                    _16155 = gBackdrop0.sample(gLinear, _16154, level(0.0));
                }
                else
                {
                    float4 _16156;
                    if (_9321 == 1)
                    {
                        float2 _16147;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _15244 = _9490;
                            _15244.y = 1.0 - _9490.y;
                            _16147 = _15244;
                        }
                        else
                        {
                            _16147 = _9490;
                        }
                        float2 _9548 = float2(_9502.x, _9490.y);
                        float2 _16148;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _15251 = _9548;
                            _15251.y = 1.0 - _9490.y;
                            _16148 = _15251;
                        }
                        else
                        {
                            _16148 = _9548;
                        }
                        float2 _9566 = float2(_9490.x, _9502.y);
                        float2 _16150;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _15258 = _9566;
                            _15258.y = 1.0 - _9502.y;
                            _16150 = _15258;
                        }
                        else
                        {
                            _16150 = _9566;
                        }
                        float2 _16152;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _15265 = _9502;
                            _15265.y = 1.0 - _9502.y;
                            _16152 = _15265;
                        }
                        else
                        {
                            _16152 = _9502;
                        }
                        _16156 = (((gBackdrop1.sample(gLinear, _16147, level(0.0)) * (_9474.x * _9474.y)) + (gBackdrop1.sample(gLinear, _16148, level(0.0)) * (_9478.x * _9474.y))) + (gBackdrop1.sample(gLinear, _16150, level(0.0)) * (_9474.x * _9478.y))) + (gBackdrop1.sample(gLinear, _16152, level(0.0)) * (_9478.x * _9478.y));
                    }
                    else
                    {
                        float4 _16157;
                        if (_9321 == 2)
                        {
                            float2 _16140;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15272 = _9490;
                                _15272.y = 1.0 - _9490.y;
                                _16140 = _15272;
                            }
                            else
                            {
                                _16140 = _9490;
                            }
                            float2 _9678 = float2(_9502.x, _9490.y);
                            float2 _16141;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15279 = _9678;
                                _15279.y = 1.0 - _9490.y;
                                _16141 = _15279;
                            }
                            else
                            {
                                _16141 = _9678;
                            }
                            float2 _9696 = float2(_9490.x, _9502.y);
                            float2 _16143;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15286 = _9696;
                                _15286.y = 1.0 - _9502.y;
                                _16143 = _15286;
                            }
                            else
                            {
                                _16143 = _9696;
                            }
                            float2 _16145;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15293 = _9502;
                                _15293.y = 1.0 - _9502.y;
                                _16145 = _15293;
                            }
                            else
                            {
                                _16145 = _9502;
                            }
                            _16157 = (((gBackdrop2.sample(gLinear, _16140, level(0.0)) * (_9474.x * _9474.y)) + (gBackdrop2.sample(gLinear, _16141, level(0.0)) * (_9478.x * _9474.y))) + (gBackdrop2.sample(gLinear, _16143, level(0.0)) * (_9474.x * _9478.y))) + (gBackdrop2.sample(gLinear, _16145, level(0.0)) * (_9478.x * _9478.y));
                        }
                        else
                        {
                            float4 _16158;
                            if (_9321 == 3)
                            {
                                float2 _16133;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15300 = _9490;
                                    _15300.y = 1.0 - _9490.y;
                                    _16133 = _15300;
                                }
                                else
                                {
                                    _16133 = _9490;
                                }
                                float2 _9808 = float2(_9502.x, _9490.y);
                                float2 _16134;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15307 = _9808;
                                    _15307.y = 1.0 - _9490.y;
                                    _16134 = _15307;
                                }
                                else
                                {
                                    _16134 = _9808;
                                }
                                float2 _9826 = float2(_9490.x, _9502.y);
                                float2 _16136;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15314 = _9826;
                                    _15314.y = 1.0 - _9502.y;
                                    _16136 = _15314;
                                }
                                else
                                {
                                    _16136 = _9826;
                                }
                                float2 _16138;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15321 = _9502;
                                    _15321.y = 1.0 - _9502.y;
                                    _16138 = _15321;
                                }
                                else
                                {
                                    _16138 = _9502;
                                }
                                _16158 = (((gBackdrop3.sample(gLinear, _16133, level(0.0)) * (_9474.x * _9474.y)) + (gBackdrop3.sample(gLinear, _16134, level(0.0)) * (_9478.x * _9474.y))) + (gBackdrop3.sample(gLinear, _16136, level(0.0)) * (_9474.x * _9478.y))) + (gBackdrop3.sample(gLinear, _16138, level(0.0)) * (_9478.x * _9478.y));
                            }
                            else
                            {
                                float4 _16159;
                                if (_9321 == 4)
                                {
                                    float2 _16126;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15328 = _9490;
                                        _15328.y = 1.0 - _9490.y;
                                        _16126 = _15328;
                                    }
                                    else
                                    {
                                        _16126 = _9490;
                                    }
                                    float2 _9938 = float2(_9502.x, _9490.y);
                                    float2 _16127;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15335 = _9938;
                                        _15335.y = 1.0 - _9490.y;
                                        _16127 = _15335;
                                    }
                                    else
                                    {
                                        _16127 = _9938;
                                    }
                                    float2 _9956 = float2(_9490.x, _9502.y);
                                    float2 _16129;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15342 = _9956;
                                        _15342.y = 1.0 - _9502.y;
                                        _16129 = _15342;
                                    }
                                    else
                                    {
                                        _16129 = _9956;
                                    }
                                    float2 _16131;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15349 = _9502;
                                        _15349.y = 1.0 - _9502.y;
                                        _16131 = _15349;
                                    }
                                    else
                                    {
                                        _16131 = _9502;
                                    }
                                    _16159 = (((gBackdrop4.sample(gLinear, _16126, level(0.0)) * (_9474.x * _9474.y)) + (gBackdrop4.sample(gLinear, _16127, level(0.0)) * (_9478.x * _9474.y))) + (gBackdrop4.sample(gLinear, _16129, level(0.0)) * (_9474.x * _9478.y))) + (gBackdrop4.sample(gLinear, _16131, level(0.0)) * (_9478.x * _9478.y));
                                }
                                else
                                {
                                    float2 _16119;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15356 = _9490;
                                        _15356.y = 1.0 - _9490.y;
                                        _16119 = _15356;
                                    }
                                    else
                                    {
                                        _16119 = _9490;
                                    }
                                    float2 _10068 = float2(_9502.x, _9490.y);
                                    float2 _16120;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15363 = _10068;
                                        _15363.y = 1.0 - _9490.y;
                                        _16120 = _15363;
                                    }
                                    else
                                    {
                                        _16120 = _10068;
                                    }
                                    float2 _10086 = float2(_9490.x, _9502.y);
                                    float2 _16122;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15370 = _10086;
                                        _15370.y = 1.0 - _9502.y;
                                        _16122 = _15370;
                                    }
                                    else
                                    {
                                        _16122 = _10086;
                                    }
                                    float2 _16124;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15377 = _9502;
                                        _15377.y = 1.0 - _9502.y;
                                        _16124 = _15377;
                                    }
                                    else
                                    {
                                        _16124 = _9502;
                                    }
                                    _16159 = (((gBackdrop5.sample(gLinear, _16119, level(0.0)) * (_9474.x * _9474.y)) + (gBackdrop5.sample(gLinear, _16120, level(0.0)) * (_9478.x * _9474.y))) + (gBackdrop5.sample(gLinear, _16122, level(0.0)) * (_9474.x * _9478.y))) + (gBackdrop5.sample(gLinear, _16124, level(0.0)) * (_9478.x * _9478.y));
                                }
                                _16158 = _16159;
                            }
                            _16157 = _16158;
                        }
                        _16156 = _16157;
                    }
                    _16155 = _16156;
                }
                _9335 = _16188 + (_16155.xyz * (_9306 ? ((_16118 == 0) ? (1.0 - _9301) : _9301) : 1.0));
            }
            float3 _16578;
            if (((gFxData_1._data[_6625].w > 0.001000000047497451305389404296875) && (_6749 > 0.0)) && ((((gFxData_1._data[_6625].y * 0.300000011920928955078125) * gFxData_1._data[_6625].w) * _184.gDisplay.z) > (_6721 * 0.3499999940395355224609375)))
            {
                float _6849 = 0.300000011920928955078125 * gFxData_1._data[_6625].w;
                float2 _6856 = (_6807 + _6819) - (_6822 * _6849);
                float _10182 = fast::clamp(log2(fast::max(_6721, 1.0)) - 1.0, 0.0, 5.0);
                int _10185 = int(floor(_10182));
                float _10189 = _10182 - float(_10185);
                bool _10194 = (_10189 > 0.0199999995529651641845703125) && (_10185 < 5);
                float3 _16284;
                _16284 = float3(0.0);
                float3 _10223;
                for (int _16214 = 0; _16214 < 2; _16284 = _10223, _16214++)
                {
                    if ((_16214 > 0) && (!_10194))
                    {
                        break;
                    }
                    int _10209 = _10185 + _16214;
                    int _10247 = clamp(_10209, 1, 5);
                    float2 _10316 = (_6856 * _184.gLevel[_10247].xy) - float2(0.5);
                    float2 _10318 = floor(_10316);
                    float2 _10321 = _10316 - _10318;
                    float2 _10324 = _10321 * _10321;
                    float2 _10327 = _10324 * _10321;
                    float2 _10346 = (((_10327 * 3.0) - (_10324 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                    float2 _10359 = _10327 * 0.16666667163372039794921875;
                    float2 _10362 = (((((-_10327) + (_10324 * 3.0)) - (_10321 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10346;
                    float2 _10366 = (((((_10327 * (-3.0)) + (_10324 * 3.0)) + (_10321 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10359;
                    float2 _10378 = ((_10318 - float2(0.5)) + (_10346 / _10362)) * _184.gLevel[_10247].zw;
                    float2 _10390 = ((_10318 + float2(1.5)) + (_10359 / _10366)) * _184.gLevel[_10247].zw;
                    float4 _16251;
                    if (_10209 <= 0)
                    {
                        float2 _16250;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _15382 = _6856;
                            _15382.y = 1.0 - _6856.y;
                            _16250 = _15382;
                        }
                        else
                        {
                            _16250 = _6856;
                        }
                        _16251 = gBackdrop0.sample(gLinear, _16250, level(0.0));
                    }
                    else
                    {
                        float4 _16252;
                        if (_10209 == 1)
                        {
                            float2 _16243;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15387 = _10378;
                                _15387.y = 1.0 - _10378.y;
                                _16243 = _15387;
                            }
                            else
                            {
                                _16243 = _10378;
                            }
                            float _10435 = _10378.y;
                            float2 _10436 = float2(_10390.x, _10435);
                            float2 _16244;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15394 = _10436;
                                _15394.y = 1.0 - _10435;
                                _16244 = _15394;
                            }
                            else
                            {
                                _16244 = _10436;
                            }
                            float _10453 = _10390.y;
                            float2 _10454 = float2(_10378.x, _10453);
                            float2 _16246;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15401 = _10454;
                                _15401.y = 1.0 - _10453;
                                _16246 = _15401;
                            }
                            else
                            {
                                _16246 = _10454;
                            }
                            float2 _16248;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15408 = _10390;
                                _15408.y = 1.0 - _10390.y;
                                _16248 = _15408;
                            }
                            else
                            {
                                _16248 = _10390;
                            }
                            _16252 = (((gBackdrop1.sample(gLinear, _16243, level(0.0)) * (_10362.x * _10362.y)) + (gBackdrop1.sample(gLinear, _16244, level(0.0)) * (_10366.x * _10362.y))) + (gBackdrop1.sample(gLinear, _16246, level(0.0)) * (_10362.x * _10366.y))) + (gBackdrop1.sample(gLinear, _16248, level(0.0)) * (_10366.x * _10366.y));
                        }
                        else
                        {
                            float4 _16253;
                            if (_10209 == 2)
                            {
                                float2 _16236;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15415 = _10378;
                                    _15415.y = 1.0 - _10378.y;
                                    _16236 = _15415;
                                }
                                else
                                {
                                    _16236 = _10378;
                                }
                                float _10565 = _10378.y;
                                float2 _10566 = float2(_10390.x, _10565);
                                float2 _16237;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15422 = _10566;
                                    _15422.y = 1.0 - _10565;
                                    _16237 = _15422;
                                }
                                else
                                {
                                    _16237 = _10566;
                                }
                                float _10583 = _10390.y;
                                float2 _10584 = float2(_10378.x, _10583);
                                float2 _16239;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15429 = _10584;
                                    _15429.y = 1.0 - _10583;
                                    _16239 = _15429;
                                }
                                else
                                {
                                    _16239 = _10584;
                                }
                                float2 _16241;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15436 = _10390;
                                    _15436.y = 1.0 - _10390.y;
                                    _16241 = _15436;
                                }
                                else
                                {
                                    _16241 = _10390;
                                }
                                _16253 = (((gBackdrop2.sample(gLinear, _16236, level(0.0)) * (_10362.x * _10362.y)) + (gBackdrop2.sample(gLinear, _16237, level(0.0)) * (_10366.x * _10362.y))) + (gBackdrop2.sample(gLinear, _16239, level(0.0)) * (_10362.x * _10366.y))) + (gBackdrop2.sample(gLinear, _16241, level(0.0)) * (_10366.x * _10366.y));
                            }
                            else
                            {
                                float4 _16254;
                                if (_10209 == 3)
                                {
                                    float2 _16229;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15443 = _10378;
                                        _15443.y = 1.0 - _10378.y;
                                        _16229 = _15443;
                                    }
                                    else
                                    {
                                        _16229 = _10378;
                                    }
                                    float _10695 = _10378.y;
                                    float2 _10696 = float2(_10390.x, _10695);
                                    float2 _16230;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15450 = _10696;
                                        _15450.y = 1.0 - _10695;
                                        _16230 = _15450;
                                    }
                                    else
                                    {
                                        _16230 = _10696;
                                    }
                                    float _10713 = _10390.y;
                                    float2 _10714 = float2(_10378.x, _10713);
                                    float2 _16232;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15457 = _10714;
                                        _15457.y = 1.0 - _10713;
                                        _16232 = _15457;
                                    }
                                    else
                                    {
                                        _16232 = _10714;
                                    }
                                    float2 _16234;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15464 = _10390;
                                        _15464.y = 1.0 - _10390.y;
                                        _16234 = _15464;
                                    }
                                    else
                                    {
                                        _16234 = _10390;
                                    }
                                    _16254 = (((gBackdrop3.sample(gLinear, _16229, level(0.0)) * (_10362.x * _10362.y)) + (gBackdrop3.sample(gLinear, _16230, level(0.0)) * (_10366.x * _10362.y))) + (gBackdrop3.sample(gLinear, _16232, level(0.0)) * (_10362.x * _10366.y))) + (gBackdrop3.sample(gLinear, _16234, level(0.0)) * (_10366.x * _10366.y));
                                }
                                else
                                {
                                    float4 _16255;
                                    if (_10209 == 4)
                                    {
                                        float2 _16222;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15471 = _10378;
                                            _15471.y = 1.0 - _10378.y;
                                            _16222 = _15471;
                                        }
                                        else
                                        {
                                            _16222 = _10378;
                                        }
                                        float _10825 = _10378.y;
                                        float2 _10826 = float2(_10390.x, _10825);
                                        float2 _16223;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15478 = _10826;
                                            _15478.y = 1.0 - _10825;
                                            _16223 = _15478;
                                        }
                                        else
                                        {
                                            _16223 = _10826;
                                        }
                                        float _10843 = _10390.y;
                                        float2 _10844 = float2(_10378.x, _10843);
                                        float2 _16225;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15485 = _10844;
                                            _15485.y = 1.0 - _10843;
                                            _16225 = _15485;
                                        }
                                        else
                                        {
                                            _16225 = _10844;
                                        }
                                        float2 _16227;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15492 = _10390;
                                            _15492.y = 1.0 - _10390.y;
                                            _16227 = _15492;
                                        }
                                        else
                                        {
                                            _16227 = _10390;
                                        }
                                        _16255 = (((gBackdrop4.sample(gLinear, _16222, level(0.0)) * (_10362.x * _10362.y)) + (gBackdrop4.sample(gLinear, _16223, level(0.0)) * (_10366.x * _10362.y))) + (gBackdrop4.sample(gLinear, _16225, level(0.0)) * (_10362.x * _10366.y))) + (gBackdrop4.sample(gLinear, _16227, level(0.0)) * (_10366.x * _10366.y));
                                    }
                                    else
                                    {
                                        float2 _16215;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15499 = _10378;
                                            _15499.y = 1.0 - _10378.y;
                                            _16215 = _15499;
                                        }
                                        else
                                        {
                                            _16215 = _10378;
                                        }
                                        float _10955 = _10378.y;
                                        float2 _10956 = float2(_10390.x, _10955);
                                        float2 _16216;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15506 = _10956;
                                            _15506.y = 1.0 - _10955;
                                            _16216 = _15506;
                                        }
                                        else
                                        {
                                            _16216 = _10956;
                                        }
                                        float _10973 = _10390.y;
                                        float2 _10974 = float2(_10378.x, _10973);
                                        float2 _16218;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15513 = _10974;
                                            _15513.y = 1.0 - _10973;
                                            _16218 = _15513;
                                        }
                                        else
                                        {
                                            _16218 = _10974;
                                        }
                                        float2 _16220;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15520 = _10390;
                                            _15520.y = 1.0 - _10390.y;
                                            _16220 = _15520;
                                        }
                                        else
                                        {
                                            _16220 = _10390;
                                        }
                                        _16255 = (((gBackdrop5.sample(gLinear, _16215, level(0.0)) * (_10362.x * _10362.y)) + (gBackdrop5.sample(gLinear, _16216, level(0.0)) * (_10366.x * _10362.y))) + (gBackdrop5.sample(gLinear, _16218, level(0.0)) * (_10362.x * _10366.y))) + (gBackdrop5.sample(gLinear, _16220, level(0.0)) * (_10366.x * _10366.y));
                                    }
                                    _16254 = _16255;
                                }
                                _16253 = _16254;
                            }
                            _16252 = _16253;
                        }
                        _16251 = _16252;
                    }
                    _10223 = _16284 + (_16251.xyz * (_10194 ? ((_16214 == 0) ? (1.0 - _10189) : _10189) : 1.0));
                }
                float3 _15524 = _16188;
                _15524.x = _16284.x;
                float2 _6867 = (_6807 + _6819) + (_6822 * _6849);
                float _11070 = fast::clamp(log2(fast::max(_6721, 1.0)) - 1.0, 0.0, 5.0);
                int _11073 = int(floor(_11070));
                float _11077 = _11070 - float(_11073);
                bool _11082 = (_11077 > 0.0199999995529651641845703125) && (_11073 < 5);
                float3 _16408;
                _16408 = float3(0.0);
                float3 _11111;
                for (int _16338 = 0; _16338 < 2; _16408 = _11111, _16338++)
                {
                    if ((_16338 > 0) && (!_11082))
                    {
                        break;
                    }
                    int _11097 = _11073 + _16338;
                    int _11135 = clamp(_11097, 1, 5);
                    float2 _11204 = (_6867 * _184.gLevel[_11135].xy) - float2(0.5);
                    float2 _11206 = floor(_11204);
                    float2 _11209 = _11204 - _11206;
                    float2 _11212 = _11209 * _11209;
                    float2 _11215 = _11212 * _11209;
                    float2 _11234 = (((_11215 * 3.0) - (_11212 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                    float2 _11247 = _11215 * 0.16666667163372039794921875;
                    float2 _11250 = (((((-_11215) + (_11212 * 3.0)) - (_11209 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11234;
                    float2 _11254 = (((((_11215 * (-3.0)) + (_11212 * 3.0)) + (_11209 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11247;
                    float2 _11266 = ((_11206 - float2(0.5)) + (_11234 / _11250)) * _184.gLevel[_11135].zw;
                    float2 _11278 = ((_11206 + float2(1.5)) + (_11247 / _11254)) * _184.gLevel[_11135].zw;
                    float4 _16375;
                    if (_11097 <= 0)
                    {
                        float2 _16374;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _15527 = _6867;
                            _15527.y = 1.0 - _6867.y;
                            _16374 = _15527;
                        }
                        else
                        {
                            _16374 = _6867;
                        }
                        _16375 = gBackdrop0.sample(gLinear, _16374, level(0.0));
                    }
                    else
                    {
                        float4 _16376;
                        if (_11097 == 1)
                        {
                            float2 _16367;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15532 = _11266;
                                _15532.y = 1.0 - _11266.y;
                                _16367 = _15532;
                            }
                            else
                            {
                                _16367 = _11266;
                            }
                            float _11323 = _11266.y;
                            float2 _11324 = float2(_11278.x, _11323);
                            float2 _16368;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15539 = _11324;
                                _15539.y = 1.0 - _11323;
                                _16368 = _15539;
                            }
                            else
                            {
                                _16368 = _11324;
                            }
                            float _11341 = _11278.y;
                            float2 _11342 = float2(_11266.x, _11341);
                            float2 _16370;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15546 = _11342;
                                _15546.y = 1.0 - _11341;
                                _16370 = _15546;
                            }
                            else
                            {
                                _16370 = _11342;
                            }
                            float2 _16372;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15553 = _11278;
                                _15553.y = 1.0 - _11278.y;
                                _16372 = _15553;
                            }
                            else
                            {
                                _16372 = _11278;
                            }
                            _16376 = (((gBackdrop1.sample(gLinear, _16367, level(0.0)) * (_11250.x * _11250.y)) + (gBackdrop1.sample(gLinear, _16368, level(0.0)) * (_11254.x * _11250.y))) + (gBackdrop1.sample(gLinear, _16370, level(0.0)) * (_11250.x * _11254.y))) + (gBackdrop1.sample(gLinear, _16372, level(0.0)) * (_11254.x * _11254.y));
                        }
                        else
                        {
                            float4 _16377;
                            if (_11097 == 2)
                            {
                                float2 _16360;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15560 = _11266;
                                    _15560.y = 1.0 - _11266.y;
                                    _16360 = _15560;
                                }
                                else
                                {
                                    _16360 = _11266;
                                }
                                float _11453 = _11266.y;
                                float2 _11454 = float2(_11278.x, _11453);
                                float2 _16361;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15567 = _11454;
                                    _15567.y = 1.0 - _11453;
                                    _16361 = _15567;
                                }
                                else
                                {
                                    _16361 = _11454;
                                }
                                float _11471 = _11278.y;
                                float2 _11472 = float2(_11266.x, _11471);
                                float2 _16363;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15574 = _11472;
                                    _15574.y = 1.0 - _11471;
                                    _16363 = _15574;
                                }
                                else
                                {
                                    _16363 = _11472;
                                }
                                float2 _16365;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15581 = _11278;
                                    _15581.y = 1.0 - _11278.y;
                                    _16365 = _15581;
                                }
                                else
                                {
                                    _16365 = _11278;
                                }
                                _16377 = (((gBackdrop2.sample(gLinear, _16360, level(0.0)) * (_11250.x * _11250.y)) + (gBackdrop2.sample(gLinear, _16361, level(0.0)) * (_11254.x * _11250.y))) + (gBackdrop2.sample(gLinear, _16363, level(0.0)) * (_11250.x * _11254.y))) + (gBackdrop2.sample(gLinear, _16365, level(0.0)) * (_11254.x * _11254.y));
                            }
                            else
                            {
                                float4 _16378;
                                if (_11097 == 3)
                                {
                                    float2 _16353;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15588 = _11266;
                                        _15588.y = 1.0 - _11266.y;
                                        _16353 = _15588;
                                    }
                                    else
                                    {
                                        _16353 = _11266;
                                    }
                                    float _11583 = _11266.y;
                                    float2 _11584 = float2(_11278.x, _11583);
                                    float2 _16354;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15595 = _11584;
                                        _15595.y = 1.0 - _11583;
                                        _16354 = _15595;
                                    }
                                    else
                                    {
                                        _16354 = _11584;
                                    }
                                    float _11601 = _11278.y;
                                    float2 _11602 = float2(_11266.x, _11601);
                                    float2 _16356;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15602 = _11602;
                                        _15602.y = 1.0 - _11601;
                                        _16356 = _15602;
                                    }
                                    else
                                    {
                                        _16356 = _11602;
                                    }
                                    float2 _16358;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15609 = _11278;
                                        _15609.y = 1.0 - _11278.y;
                                        _16358 = _15609;
                                    }
                                    else
                                    {
                                        _16358 = _11278;
                                    }
                                    _16378 = (((gBackdrop3.sample(gLinear, _16353, level(0.0)) * (_11250.x * _11250.y)) + (gBackdrop3.sample(gLinear, _16354, level(0.0)) * (_11254.x * _11250.y))) + (gBackdrop3.sample(gLinear, _16356, level(0.0)) * (_11250.x * _11254.y))) + (gBackdrop3.sample(gLinear, _16358, level(0.0)) * (_11254.x * _11254.y));
                                }
                                else
                                {
                                    float4 _16379;
                                    if (_11097 == 4)
                                    {
                                        float2 _16346;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15616 = _11266;
                                            _15616.y = 1.0 - _11266.y;
                                            _16346 = _15616;
                                        }
                                        else
                                        {
                                            _16346 = _11266;
                                        }
                                        float _11713 = _11266.y;
                                        float2 _11714 = float2(_11278.x, _11713);
                                        float2 _16347;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15623 = _11714;
                                            _15623.y = 1.0 - _11713;
                                            _16347 = _15623;
                                        }
                                        else
                                        {
                                            _16347 = _11714;
                                        }
                                        float _11731 = _11278.y;
                                        float2 _11732 = float2(_11266.x, _11731);
                                        float2 _16349;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15630 = _11732;
                                            _15630.y = 1.0 - _11731;
                                            _16349 = _15630;
                                        }
                                        else
                                        {
                                            _16349 = _11732;
                                        }
                                        float2 _16351;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15637 = _11278;
                                            _15637.y = 1.0 - _11278.y;
                                            _16351 = _15637;
                                        }
                                        else
                                        {
                                            _16351 = _11278;
                                        }
                                        _16379 = (((gBackdrop4.sample(gLinear, _16346, level(0.0)) * (_11250.x * _11250.y)) + (gBackdrop4.sample(gLinear, _16347, level(0.0)) * (_11254.x * _11250.y))) + (gBackdrop4.sample(gLinear, _16349, level(0.0)) * (_11250.x * _11254.y))) + (gBackdrop4.sample(gLinear, _16351, level(0.0)) * (_11254.x * _11254.y));
                                    }
                                    else
                                    {
                                        float2 _16339;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15644 = _11266;
                                            _15644.y = 1.0 - _11266.y;
                                            _16339 = _15644;
                                        }
                                        else
                                        {
                                            _16339 = _11266;
                                        }
                                        float _11843 = _11266.y;
                                        float2 _11844 = float2(_11278.x, _11843);
                                        float2 _16340;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15651 = _11844;
                                            _15651.y = 1.0 - _11843;
                                            _16340 = _15651;
                                        }
                                        else
                                        {
                                            _16340 = _11844;
                                        }
                                        float _11861 = _11278.y;
                                        float2 _11862 = float2(_11266.x, _11861);
                                        float2 _16342;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15658 = _11862;
                                            _15658.y = 1.0 - _11861;
                                            _16342 = _15658;
                                        }
                                        else
                                        {
                                            _16342 = _11862;
                                        }
                                        float2 _16344;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15665 = _11278;
                                            _15665.y = 1.0 - _11278.y;
                                            _16344 = _15665;
                                        }
                                        else
                                        {
                                            _16344 = _11278;
                                        }
                                        _16379 = (((gBackdrop5.sample(gLinear, _16339, level(0.0)) * (_11250.x * _11250.y)) + (gBackdrop5.sample(gLinear, _16340, level(0.0)) * (_11254.x * _11250.y))) + (gBackdrop5.sample(gLinear, _16342, level(0.0)) * (_11250.x * _11254.y))) + (gBackdrop5.sample(gLinear, _16344, level(0.0)) * (_11254.x * _11254.y));
                                    }
                                    _16378 = _16379;
                                }
                                _16377 = _16378;
                            }
                            _16376 = _16377;
                        }
                        _16375 = _16376;
                    }
                    _11111 = _16408 + (_16375.xyz * (_11082 ? ((_16338 == 0) ? (1.0 - _11077) : _11077) : 1.0));
                }
                _15524.z = _16408.z;
                _16578 = _15524;
            }
            else
            {
                _16578 = _16188;
            }
            float3 _16798;
            if (_6749 > 0.0)
            {
                float2 _6890 = _6807 + (((_16116 * fast::min(_6740 * 0.5, 16.0)) * _184.gDisplay.zw) * _184.gTarget.zw);
                float _11958 = fast::clamp(log2(fast::max(16.0 * _184.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _11961 = int(floor(_11958));
                float _11965 = _11958 - float(_11961);
                bool _11970 = (_11965 > 0.0199999995529651641845703125) && (_11961 < 5);
                float3 _16563;
                _16563 = float3(0.0);
                float3 _11999;
                for (int _16548 = 0; _16548 < 2; _16563 = _11999, _16548++)
                {
                    if ((_16548 > 0) && (!_11970))
                    {
                        break;
                    }
                    int _11985 = _11961 + _16548;
                    float2 _16549;
                    if (_184.gConv.x > 0.5)
                    {
                        float2 _15672 = _6890;
                        _15672.y = 1.0 - _6890.y;
                        _16549 = _15672;
                    }
                    else
                    {
                        _16549 = _6890;
                    }
                    float4 _16550;
                    if (_11985 <= 0)
                    {
                        _16550 = gBackdrop0.sample(gLinear, _16549, level(0.0));
                    }
                    else
                    {
                        float4 _16551;
                        if (_11985 == 1)
                        {
                            _16551 = gBackdrop1.sample(gLinear, _16549, level(0.0));
                        }
                        else
                        {
                            float4 _16552;
                            if (_11985 == 2)
                            {
                                _16552 = gBackdrop2.sample(gLinear, _16549, level(0.0));
                            }
                            else
                            {
                                float4 _16553;
                                if (_11985 == 3)
                                {
                                    _16553 = gBackdrop3.sample(gLinear, _16549, level(0.0));
                                }
                                else
                                {
                                    float4 _16554;
                                    if (_11985 == 4)
                                    {
                                        _16554 = gBackdrop4.sample(gLinear, _16549, level(0.0));
                                    }
                                    else
                                    {
                                        _16554 = gBackdrop5.sample(gLinear, _16549, level(0.0));
                                    }
                                    _16553 = _16554;
                                }
                                _16552 = _16553;
                            }
                            _16551 = _16552;
                        }
                        _16550 = _16551;
                    }
                    _11999 = _16563 + (_16550.xyz * (_11970 ? ((_16548 == 0) ? (1.0 - _11965) : _11965) : 1.0));
                }
                _16798 = _16563;
            }
            else
            {
                _16798 = float3(0.5);
            }
            float _16594;
            if (in.i_shape.x > 0.001000000047497451305389404296875)
            {
                int _12088 = clamp(int(rint(log2(36.0 * _184.gDisplay.z) - 1.0)), 1, 4);
                float2 _16569;
                if (_184.gConv.x > 0.5)
                {
                    float2 _15676 = _6807;
                    _15676.y = 1.0 - _6807.y;
                    _16569 = _15676;
                }
                else
                {
                    _16569 = _6807;
                }
                float4 _16570;
                if (_12088 <= 0)
                {
                    _16570 = gBackdrop0.sample(gLinear, _16569, level(0.0));
                }
                else
                {
                    float4 _16571;
                    if (_12088 == 1)
                    {
                        _16571 = gBackdrop1.sample(gLinear, _16569, level(0.0));
                    }
                    else
                    {
                        float4 _16572;
                        if (_12088 == 2)
                        {
                            _16572 = gBackdrop2.sample(gLinear, _16569, level(0.0));
                        }
                        else
                        {
                            float4 _16573;
                            if (_12088 == 3)
                            {
                                _16573 = gBackdrop3.sample(gLinear, _16569, level(0.0));
                            }
                            else
                            {
                                float4 _16574;
                                if (_12088 == 4)
                                {
                                    _16574 = gBackdrop4.sample(gLinear, _16569, level(0.0));
                                }
                                else
                                {
                                    _16574 = gBackdrop5.sample(gLinear, _16569, level(0.0));
                                }
                                _16573 = _16574;
                            }
                            _16572 = _16573;
                        }
                        _16571 = _16572;
                    }
                    _16570 = _16571;
                }
                _16594 = dot(_16570.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _16594 = 0.5;
            }
            _16796 = _16798;
            _16593 = _16594;
            _16575 = _16578;
        }
        else
        {
            _16796 = float3(0.5);
            _16593 = 0.5;
            _16575 = float3(0.5);
        }
        float3 _6918 = mix(float3(dot(_16575, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _16575, float3(gFxData_1._data[_6641].x)) + float3(gFxData_1._data[_6641].y);
        float3 _16689;
        if (in.i_shape.x > 0.001000000047497451305389404296875)
        {
            float _6928 = fast::clamp(fast::max(_16593 + gFxData_1._data[_6641].y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _6936 = mix(_6928, dot(gFxData_1._data[_6633].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), in.i_shape.x);
            _16689 = select(mix(_6918, float3(1.0), float3((_6936 - _6928) / fast::max(1.0 - _6928, 0.001000000047497451305389404296875))), _6918 * (_6936 / _6928), bool3(_6936 < _6928));
        }
        else
        {
            _16689 = _6918;
        }
        float2 _6974 = fast::clamp((in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875)), float2(0.0), float2(1.0));
        float2 _7022 = float2(cos(gFxData_1._data[_6641].w), sin(gFxData_1._data[_6641].w));
        float _7025 = dot(_16116, _7022);
        float _7036 = -_15929;
        float _7043 = _4077 * ((0.5 * abs(_16116.x)) + 0.0199999995529651641845703125);
        float _7050 = _4077 * ((0.5 * abs(_16116.y)) + 0.0199999995529651641845703125);
        float _16908;
        if ((((_7036 + _7043) + _7050) > 0.0) && (((_7036 - _7043) - _7050) < (0.5 * _6740)))
        {
            float _12192 = fast::max((_7036 + _7043) + _7050, 0.0) / _6740;
            float _12196 = fast::clamp(sqrt(2.0 * _12192), 0.12399999797344207763671875, 1.0);
            float _12198 = 1.0 - _12196;
            float _12201 = _12198 * _12198;
            float _12255 = fast::max((_7036 + _7043) - _7050, 0.0) / _6740;
            float _12259 = fast::clamp(sqrt(2.0 * _12255), 0.12399999797344207763671875, 1.0);
            float _12261 = 1.0 - _12259;
            float _12264 = _12261 * _12261;
            float _12318 = fast::max((_7036 - _7043) + _7050, 0.0) / _6740;
            float _12322 = fast::clamp(sqrt(2.0 * _12318), 0.12399999797344207763671875, 1.0);
            float _12324 = 1.0 - _12322;
            float _12327 = _12324 * _12324;
            float _12381 = fast::max((_7036 - _7043) - _7050, 0.0) / _6740;
            float _12385 = fast::clamp(sqrt(2.0 * _12381), 0.12399999797344207763671875, 1.0);
            float _12387 = 1.0 - _12385;
            float _12390 = _12387 * _12387;
            float _7097 = ((((((_12192 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _12192) * _12192) : ((_12192 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _12196) * _12196)) + ((((_12201 * _12201) * _12201) * _12198) * (0.02380952425301074981689453125 + (_12198 * ((-0.0386904776096343994140625) + (_12198 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_12192 - 0.5))))) * _6740) * _6740) - ((((_12255 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _12255) * _12255) : ((_12255 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _12259) * _12259)) + ((((_12264 * _12264) * _12264) * _12261) * (0.02380952425301074981689453125 + (_12261 * ((-0.0386904776096343994140625) + (_12261 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_12255 - 0.5))))) * _6740) * _6740)) - ((((_12318 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _12318) * _12318) : ((_12318 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _12322) * _12322)) + ((((_12327 * _12327) * _12327) * _12324) * (0.02380952425301074981689453125 + (_12324 * ((-0.0386904776096343994140625) + (_12324 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_12318 - 0.5))))) * _6740) * _6740)) + ((((_12381 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _12381) * _12381) : ((_12381 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _12385) * _12385)) + ((((_12390 * _12390) * _12390) * _12387) * (0.02380952425301074981689453125 + (_12387 * ((-0.0386904776096343994140625) + (_12387 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_12381 - 0.5))))) * _6740) * _6740);
            _16908 = _7097 / ((4.0 * _7043) * _7050);
        }
        else
        {
            _16908 = 0.0;
        }
        float _7129 = fast::clamp(0.5 + (0.5 * dot((in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5)) / _6732, _7022)), 0.0, 1.0);
        _17820 = ((_16908 * (pow(fast::clamp(_7025, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(fast::clamp(-_7025, 0.0, 1.0), 1.5)))) * gFxData_1._data[_6641].z) * 1.60000002384185791015625;
        _17439 = gFxData_1._data[_6649];
        _17426 = float4((mix(mix(_16689, gFxData_1._data[_6633].xyz, float3(fast::clamp(gFxData_1._data[_6633].w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _6755))) + (0.3499999940395355224609375 * ((dot(gFxData_1._data[_6633].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _6974.y) : _6974.y))), 0.0, 1.0))), (_16796 * 1.10000002384185791015625) + float3(0.07999999821186065673828125), float3(pow(1.0 - _16693.z, 5.0) * 0.3499999940395355224609375)) + float3((((0.039999999105930328369140625 * _7129) * _7129) + (0.0500000007450580596923828125 * (1.0 - _6755))) * gFxData_1._data[_6641].z)) * _4102, _4102) + (_16915 * (1.0 - _4102));
    }
    else
    {
        _17820 = 0.0;
        _17439 = _15909;
        _17426 = _16915;
    }
    float4 _17436;
    if ((in.i_flags.x & _2852) != 0u)
    {
        float4 _17057;
        float4 _17235;
        if (in.i_flags.y != 0u)
        {
            _17235 = gFxData_1._data[(in.i_instance * 24u) + 3u];
            _17057 = gFxData_1._data[(in.i_instance * 24u) + 4u];
        }
        else
        {
            _17235 = float4(0.0);
            _17057 = float4(0.0);
        }
        float4 _17421;
        do
        {
            if (in.i_flags.y == 0u)
            {
                _17421 = in.i_fill0;
                break;
            }
            float2 _12494 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float2 _12504 = fwidth(in.i_local);
            float _12506 = fast::max(length(_12504), 9.9999997473787516355514526367188e-05);
            float _17413;
            float _17417;
            if ((in.i_flags.y == 1u) || (in.i_flags.y == 4u))
            {
                float _12515 = cos(_17057.x);
                float _12518 = sin(_17057.x);
                float _12544 = ((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_12515, _12518)) / fast::max(0.5 * ((abs(_12515) * _12494.x) + (abs(_12518) * _12494.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (in.i_flags.y == 4u)
                {
                    float _12559 = fast::clamp((_12544 - _17057.y) / fast::max(_17057.z - _17057.y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _17421 = float4(fast::clamp(abs((fract(float3(_12559 * 0.800000011920928955078125) + float3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - float3(3.0)) - float3(1.0), float3(0.0), float3(1.0)), in.i_fill0.w * pow(fast::max(sin(_12559 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _17417 = -1.0;
                _17413 = _12544;
            }
            else
            {
                float _17414;
                float _17418;
                if (in.i_flags.y == 2u)
                {
                    _17418 = -1.0;
                    _17414 = length(in.i_local - (in.i_rect.xy + (_17057.xy * _12494))) / fast::max(_17057.z * fast::max(_12494.x, _12494.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    float2 _12627 = in.i_local - (in.i_rect.xy + (_17057.xy * _12494));
                    float _12638 = fract(((precise::atan2(_12627.y, _12627.x) - _17057.z) * 0.15915493667125701904296875) + 1.0);
                    float _17415;
                    float _17419;
                    if (_17057.w > 0.5)
                    {
                        _17419 = -1.0;
                        _17415 = 0.5 - (0.5 * cos(_12638 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _12658 = (((_12638 < 0.5) ? _12638 : (_12638 - 1.0)) * 6.283185482025146484375) * length(_12627);
                        float _17420;
                        if (abs(_12658) < _12506)
                        {
                            _17420 = fast::clamp(((_12658 / _12506) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _17420 = -1.0;
                        }
                        _17419 = _17420;
                        _17415 = _12638;
                    }
                    _17418 = _17419;
                    _17414 = _17415;
                }
                _17417 = _17418;
                _17413 = _17414;
            }
            float4 _12687 = float4(in.i_fill0.xyz * in.i_fill0.w, in.i_fill0.w);
            float4 _12699 = float4(_17235.xyz * _17235.w, _17235.w);
            float4 _12713 = select(mix(_12687, _12699, float4(fast::clamp(_17413, 0.0, 1.0))), mix(_12699, _12687, float4(_17417)), bool4(_17417 >= 0.0));
            _17421 = select(float4(0.0), float4(_12713.xyz / float3(_12713.w), _12713.w), bool4(_12713.w > 9.9999997473787516355514526367188e-06));
            break;
        } while(false);
        float4 _17422;
        if ((in.i_flags.x & _2881) != 0u)
        {
            uint _12738 = (in.i_instance * 24u) + 18u;
            _17422 = _17421 * gTex.sample(gLinear, mix(gFxData_1._data[_12738].xy, gFxData_1._data[_12738].zw, (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875))));
        }
        else
        {
            _17422 = _17421;
        }
        float _12748 = fast::clamp(_17422.w * _4102, 0.0, 1.0);
        _17436 = float4(_17422.xyz * _12748, _12748) + (_17426 * (1.0 - _12748));
    }
    else
    {
        _17436 = _17426;
    }
    float4 _17806;
    if ((in.i_flags.x & _2936) != 0u)
    {
        uint _12772 = (in.i_instance * 24u) + 21u;
        uint _12780 = (in.i_instance * 24u) + 3u;
        float2 _4542 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float2 _4549 = (in.i_local - in.i_rect.xy) / _4542;
        float _4560 = ((gFxData_1._data[_12780].x >= 0.0) ? gFxData_1._data[_12780].x : _184.gTime.x) * gFxData_1._data[_12772].z;
        float _4563 = _4549.x * 2.0;
        float _4564 = _4563 - 1.0;
        float _4569 = _4549.y * _4542.y;
        float _4572 = fast::max(gFxData_1._data[_12772].w, 0.001000000047497451305389404296875);
        float _4577 = fast::clamp(1.0 - (_4564 * _4564), 0.0, 1.0);
        float _4579 = pow(_4577, 1.2999999523162841796875);
        float _4581 = pow(_4577, 0.699999988079071044921875);
        float _4584 = fast::clamp(_4560 * 1.4285714626312255859375, 0.0, 1.0);
        float _4600 = (0.25 + (0.75 * ((_4584 * _4584) * (3.0 - (2.0 * _4584))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_4560 * 2.099999904632568359375)));
        float _4609 = ((gFxData_1._data[_12772].y * _4542.y) * _4600) * _4579;
        float _4635 = (_4542.y * (0.5 + ((gFxData_1._data[_12772].x * (0.5 - (_4564 * _4564))) * 0.5))) + (((0.14000000059604644775390625 * _4542.y) * _4579) * sin(((_4564 * 2.400000095367431640625) - (_4560 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _17432;
        float _17433;
        float3 _17434;
        _17434 = float3(0.0);
        _17433 = _4635;
        _17432 = _4635;
        float3 _4707;
        float _18068;
        float _18069;
        for (int _17431 = 0; _17431 < 4; _17434 = _4707, _17433 = _18069, _17432 = _18068, _17431++)
        {
            float _4659 = _4635 + ((_4609 * _3100[_17431].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_4560 * 1.7000000476837158203125) + _3117[_17431].y))));
            _18068 = (_17431 == 0) ? _4659 : _17432;
            _18069 = (_17431 == 2) ? _4659 : _17433;
            float _4674 = _4572 * _3100[_17431].y;
            float _4679 = (_4569 - _4659) / _4674;
            float _4684 = _4572 * _3100[_17431].z;
            float3 _18042;
            _18042 = float3(0.0);
            for (int _18041 = 0; _18041 < 6; )
            {
                float _12805 = ((_4569 - _4659) - (_4684 * ((float(_18041) * 0.4000000059604644775390625) - 1.0))) / _4674;
                _18042 += (_1726[_18041] * exp((-_12805) * _12805));
                _18041++;
                continue;
            }
            _4707 = _17434 + (mix(_18042 * float3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), float3(exp((-_4679) * _4679)), float3(_3117[_17431].x)) * (_3100[_17431].w * _4581));
        }
        float _4713 = _4572 * 1.5;
        float _4743 = fast::clamp((_4569 - _17432) / fast::max(_17433 - _17432, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _4771 = (_4569 - (_17433 - (_4572 * 3.0))) / (((_4542.y * 0.0900000035762786865234375) + (_4609 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _4774 = (_4563 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _4799 = ((_4569 - _17432) + (_4572 * 5.0)) / (_4572 * 7.0);
        float3 _4825 = float3(1.0) - exp((-((((_17434 + (mix(float3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), float3(1.0), float3(_4743)) * ((((1.0 / (1.0 + exp((-((_4569 - _17432) - (_4572 * 2.0))) / _4713))) / (1.0 + exp((-(_17433 - _4569)) / _4713))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_4743, 2.5)))) * _4581))) + (float3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_4771) * _4771) - (_4774 * _4774)) * (0.5 + (1.10000002384185791015625 * _4600))))) + (float3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_4799) * _4799) * _4579) * 0.100000001490116119384765625))) * mix(float3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), float3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), float3(0.5 + (0.5 * sin(_4560 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _4835 = (in.i_fill0.w * smoothstep(0.0, 0.119999997317790985107421875, _4549.y)) * smoothstep(1.0, 0.87999999523162841796875, _4549.y);
        float _4854 = (fast::clamp(fast::max(_4825.x, fast::max(_4825.y, _4825.z)), 0.0, 1.0) * _4835) * _4102;
        _17806 = float4((_4825 * _4835) * _4102, _4854) + (_17436 * (1.0 - _4854));
    }
    else
    {
        _17806 = _17436;
    }
    float4 _17815;
    if (((in.i_flags.x & _3369) != 0u) && ((in.i_flags.x & _3373) != 0u))
    {
        uint _12837 = (in.i_instance * 24u) + 7u;
        uint _12845 = (in.i_instance * 24u) + 8u;
        float2 _4888 = in.i_local - gFxData_1._data[_12845].zw;
        float2 _12886 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _12895 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _17781;
        if (in.i_flags.z == 1u)
        {
            float2 _12901 = _4888 - _12886;
            float _17780;
            do
            {
                if (in.i_shape.w >= 6.282185077667236328125)
                {
                    _17780 = abs(length(_12901) - in.i_radii.x) - in.i_radii.y;
                    break;
                }
                float _13000 = in.i_shape.z + (in.i_shape.w * 0.5);
                float _13002 = cos(_13000);
                float _13004 = sin(_13000);
                float _13013 = dot(_12901, float2(-_13004, _13002));
                float _13016 = dot(_12901, float2(_13002, _13004));
                float2 _13017 = float2(_13013, _13016);
                float _13020 = abs(_13013);
                _13017.x = _13020;
                float _13023 = in.i_shape.w * 0.5;
                float _13025 = sin(_13023);
                float _13027 = cos(_13023);
                _17780 = (((_13027 * _13020) > (_13025 * _13016)) ? length(_13017 - (float2(_13025, _13027) * in.i_radii.x)) : abs(length(_13017) - in.i_radii.x)) - in.i_radii.y;
                break;
            } while(false);
            _17781 = _17780;
        }
        else
        {
            float _17782;
            if (in.i_flags.z == 2u)
            {
                float2 _13063 = _4888 - _15911.xy;
                float2 _13066 = _15911.zw - _15911.xy;
                _17782 = length(_13063 - (_13066 * fast::clamp(dot(_13063, _13066) / fast::max(dot(_13066, _13066), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
            }
            else
            {
                float2 _12928 = _4888 - _12886;
                float _13119 = fast::min(_12895.x, _12895.y);
                float _13122 = fast::min((_12928.x > 0.0) ? ((_12928.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_12928.y > 0.0) ? in.i_radii.w : in.i_radii.x), _13119);
                float _13128 = _13122 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                float _17765;
                float _17766;
                if (_13128 > _13119)
                {
                    float _13142 = in.i_shape.y * fast::clamp((_13119 - _13122) / fast::max(0.60000002384185791015625 * _13122, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _17766 = _13142;
                    _17765 = _13122 * (1.0 + (0.60000002384185791015625 * _13142));
                }
                else
                {
                    _17766 = in.i_shape.y;
                    _17765 = _13128;
                }
                float2 _13155 = (abs(_12928) - _12895) + float2(_17765);
                float2 _13157 = fast::max(_13155, float2(0.0));
                float _17767;
                if ((_13157.x > 0.0) && (_13157.y > 0.0))
                {
                    float _17768;
                    if ((_17766 > 0.001000000047497451305389404296875) && (_17765 > 9.9999997473787516355514526367188e-05))
                    {
                        float _13174 = 2.0 + (2.0 * _17766);
                        float2 _13179 = _13157 / float2(fast::max(_17765, 9.9999997473787516355514526367188e-05));
                        _17768 = pow(pow(_13179.x, _13174) + pow(_13179.y, _13174), 1.0 / _13174) * _17765;
                    }
                    else
                    {
                        _17768 = length(_13157);
                    }
                    _17767 = _17768;
                }
                else
                {
                    _17767 = fast::max(_13157.x, _13157.y);
                }
                float _13214 = (fast::min(fast::max(_13155.x, _13155.y), 0.0) + _17767) - _17765;
                float _17783;
                if ((in.i_flags.x & _1262) != 0u)
                {
                    float2 _12956 = fast::max((_15911.zw - _15911.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _12959 = _4888 - ((_15911.xy + _15911.zw) * 0.5);
                    float _13250 = fast::min(_12956.x, _12956.y);
                    float _13253 = fast::min((_12959.x > 0.0) ? ((_12959.y > 0.0) ? _17439.x : _17439.x) : ((_12959.y > 0.0) ? _17439.x : _17439.x), _13250);
                    float _13259 = _13253 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _17771;
                    float _17772;
                    if (_13259 > _13250)
                    {
                        float _13273 = in.i_shape.y * fast::clamp((_13250 - _13253) / fast::max(0.60000002384185791015625 * _13253, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _17772 = _13273;
                        _17771 = _13253 * (1.0 + (0.60000002384185791015625 * _13273));
                    }
                    else
                    {
                        _17772 = in.i_shape.y;
                        _17771 = _13259;
                    }
                    float2 _13286 = (abs(_12959) - _12956) + float2(_17771);
                    float2 _13288 = fast::max(_13286, float2(0.0));
                    float _17773;
                    if ((_13288.x > 0.0) && (_13288.y > 0.0))
                    {
                        float _17774;
                        if ((_17772 > 0.001000000047497451305389404296875) && (_17771 > 9.9999997473787516355514526367188e-05))
                        {
                            float _13305 = 2.0 + (2.0 * _17772);
                            float2 _13310 = _13288 / float2(fast::max(_17771, 9.9999997473787516355514526367188e-05));
                            _17774 = pow(pow(_13310.x, _13305) + pow(_13310.y, _13305), 1.0 / _13305) * _17771;
                        }
                        else
                        {
                            _17774 = length(_13288);
                        }
                        _17773 = _17774;
                    }
                    else
                    {
                        _17773 = fast::max(_13288.x, _13288.y);
                    }
                    float _13345 = (fast::min(fast::max(_13286.x, _13286.y), 0.0) + _17773) - _17771;
                    float _13350 = fast::max(_17439.y, 9.9999997473787516355514526367188e-05);
                    float _13359 = fast::max(_13350 - abs(_13214 - _13345), 0.0) / _13350;
                    _17783 = fast::min(_13214, _13345) - (((_13359 * _13359) * _13350) * 0.25);
                }
                else
                {
                    _17783 = _13214;
                }
                _17782 = _17783;
            }
            _17781 = _17782;
        }
        float _4897 = (_17781 + gFxData_1._data[_12845].y) / (fast::max(gFxData_1._data[_12845].x * 0.5, _4077 * 0.5) * 1.41421353816986083984375);
        float _13376 = sign(_4897);
        float _13378 = abs(_4897);
        float _13389 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_13378 * _13378))) * _13378)) * _13378);
        float _13392 = _13389 * _13389;
        float _13407 = fast::clamp(gFxData_1._data[_12837].w * ((0.5 + (0.5 * (_13376 - (_13376 / (_13392 * _13392))))) * _4102), 0.0, 1.0);
        _17815 = float4(gFxData_1._data[_12837].xyz * _13407, _13407) + (_17806 * (1.0 - _13407));
    }
    else
    {
        _17815 = _17806;
    }
    float4 _17839;
    if ((in.i_flags.x & _3433) != 0u)
    {
        uint _13431 = (in.i_instance * 24u) + 9u;
        uint _13439 = (in.i_instance * 24u) + 10u;
        float _4929 = fast::max(-_15929, 0.0) / fast::max(gFxData_1._data[_13439].z, 0.001000000047497451305389404296875);
        float _13449 = fast::clamp(gFxData_1._data[_13431].w * fast::clamp((exp(((-_4929) * _4929) * 2.2000000476837158203125) * gFxData_1._data[_13439].w) * _4102, 0.0, 1.0), 0.0, 1.0);
        _17839 = float4(gFxData_1._data[_13431].xyz * _13449, _13449) + (_17815 * (1.0 - _13449));
    }
    else
    {
        _17839 = _17815;
    }
    float3 _4960 = _17839.xyz + float3(_17820 * fast::clamp(_17839.w / fast::max(_4102, 0.001000000047497451305389404296875), 0.0, 1.0));
    float4 _15804 = _17839;
    _15804.x = _4960.x;
    _15804.y = _4960.y;
    _15804.z = _4960.z;
    float4 _17842;
    if ((in.i_flags.x & _3502) != 0u)
    {
        uint _13473 = (in.i_instance * 24u) + 5u;
        float4 _13475 = gFxData_1._data[_13473];
        uint _13481 = (in.i_instance * 24u) + 6u;
        float4 _17840;
        if (gFxData_1._data[_13481].z < 0.999000012874603271484375)
        {
            float2 _5018 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float _5029 = cos(gFxData_1._data[_13481].w);
            float _5032 = sin(gFxData_1._data[_13481].w);
            float4 _15821 = _13475;
            _15821.w = _13475.w * mix(1.0, gFxData_1._data[_13481].z, fast::clamp(((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_5029, _5032)) / fast::max(0.5 * ((abs(_5029) * _5018.x) + (abs(_5032) * _5018.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _17840 = _15821;
        }
        else
        {
            _17840 = _13475;
        }
        float _13491 = fast::clamp(_17840.w * (fast::clamp(0.5 - ((_15929 - (gFxData_1._data[_13481].x * gFxData_1._data[_13481].y)) / _4077), 0.0, 1.0) - fast::clamp(0.5 - ((_15929 + (gFxData_1._data[_13481].x * (1.0 - gFxData_1._data[_13481].y))) / _4077), 0.0, 1.0)), 0.0, 1.0);
        _17842 = float4(_17840.xyz * _13491, _13491) + (_15804 * (1.0 - _13491));
    }
    else
    {
        _17842 = _15804;
    }
    float4 _17843;
    if ((in.i_flags.x & _3632) != 0u)
    {
        float2 _5093 = (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float _5115 = exp(-pow((((_5093.x * 0.85000002384185791015625) + (_5093.y * 0.1500000059604644775390625)) - ((fract(_184.gTime.x * in.i_misc.w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _17843 = float4(_17842.xyz + float3(((_5115 * in.i_misc.z) * _4102) * fast::max(_17842.w, 0.3499999940395355224609375)), fast::max(_17842.w, ((_5115 * in.i_misc.z) * _4102) * 0.5));
    }
    else
    {
        _17843 = _17842;
    }
    float4 _18036;
    if ((in.i_flags.x & _3714) != 0u)
    {
        float3 _13516 = fract(floor(_15908).xyx * 0.103100001811981201171875);
        float3 _13525 = _13516 + float3(dot(_13516, _13516.yzx + float3(33.3300018310546875)));
        float3 _5166 = _17843.xyz + float3(((fract((_13525.x + _13525.y) * _13525.z) - 0.5) * in.i_misc.y) * _17843.w);
        float4 _15845 = _17843;
        _15845.x = _5166.x;
        _15845.y = _5166.y;
        _15845.z = _5166.z;
        _18036 = _15845;
    }
    else
    {
        _18036 = _17843;
    }
    float _18032;
    if (_227.gFade.z > 0.0)
    {
        _18032 = smoothstep(0.0, 1.0, fast::clamp((_15908.y - _227.gFade.x) / _227.gFade.z, 0.0, 1.0));
    }
    else
    {
        _18032 = 1.0;
    }
    float _18033;
    if (_227.gFade.w > 0.0)
    {
        _18033 = _18032 * smoothstep(0.0, 1.0, fast::clamp((_227.gFade.y - _15908.y) / _227.gFade.w, 0.0, 1.0));
    }
    else
    {
        _18033 = _18032;
    }
    float4 _5183 = _18036 * ((in.i_misc.x * _17854) * _18033);
    float4 _18037;
    if (((in.i_flags.x & _3754) != 0u) || ((in.i_flags.x & _3758) != 0u))
    {
        float3 _13577 = fract((floor(_15908) + float2(17.0)).xyx * 0.103100001811981201171875);
        float3 _13586 = _13577 + float3(dot(_13577, _13577.yzx + float3(33.3300018310546875)));
        float3 _5207 = _5183.xyz + float3(((fract((_13586.x + _13586.y) * _13586.z) - 0.5) * 0.0039215688593685626983642578125) * fast::clamp(_5183.w * 8.0, 0.0, 1.0));
        float4 _15857 = _5183;
        _15857.x = _5207.x;
        _15857.y = _5207.y;
        _15857.z = _5207.z;
        _18037 = _15857;
    }
    else
    {
        _18037 = _5183;
    }
    float3 _5217 = fast::max(_18037.xyz, float3(0.0));
    float4 _15863 = _18037;
    _15863.x = _5217.x;
    _15863.y = _5217.y;
    _15863.z = _5217.z;
    float4 _18038;
    if ((_184.gTime.w > 0.5) && (_18037.w > 9.9999997473787516355514526367188e-06))
    {
        float3 _13630 = fast::clamp(_15863.xyz / float3(_18037.w), float3(0.0), float3(1.0));
        float3 _13616 = select(pow((_13630 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _13630 * float3(0.077399380505084991455078125), _13630 <= float3(0.040449999272823333740234375)) * _18037.w;
        float4 _15872 = _15863;
        _15872.x = _13616.x;
        _15872.y = _13616.y;
        _15872.z = _13616.z;
        _18038 = _15872;
    }
    else
    {
        _18038 = _15863;
    }
    out._entryPointOutput = _18038;
    return out;
}

