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

constant spvUnsafeArray<float3, 6> _1724 = spvUnsafeArray<float3, 6>({ float3(1.0, 0.4199999868869781494140625, 0.2199999988079071044921875), float3(1.0, 0.699999988079071044921875, 0.300000011920928955078125), float3(0.800000011920928955078125, 0.920000016689300537109375, 0.4000000059604644775390625), float3(0.3499999940395355224609375, 0.89999997615814208984375, 0.699999988079071044921875), float3(0.4000000059604644775390625, 0.62000000476837158203125, 1.0), float3(0.660000026226043701171875, 0.5, 1.0) });
constant spvUnsafeArray<float4, 4> _3006 = spvUnsafeArray<float4, 4>({ float4(-1.0, 1.0, 3.400000095367431640625, 2.599999904632568359375), float4(-0.550000011920928955078125, 0.800000011920928955078125, 2.0, 0.800000011920928955078125), float4(0.300000011920928955078125, 1.0, 1.2000000476837158203125, 1.2999999523162841796875), float4(0.62000000476837158203125, 0.800000011920928955078125, 1.60000002384185791015625, 0.449999988079071044921875) });
constant spvUnsafeArray<float2, 4> _3023 = spvUnsafeArray<float2, 4>({ float2(0.0), float2(0.100000001490116119384765625, 1.2999999523162841796875), float2(0.550000011920928955078125, 3.900000095367431640625), float2(0.20000000298023223876953125, 5.19999980926513671875) });

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
    float2 _5176 = gl_FragCoord.xy + _184.gConv.yy;
    float2 _15543;
    if (_184.gConv.x > 0.5)
    {
        float2 _14491 = _5176;
        _14491.y = _184.gTarget.y - _5176.y;
        _15543 = _14491;
    }
    else
    {
        _15543 = _5176;
    }
    float _3968 = dfdx(in.i_local.x);
    float _3972 = dfdy(in.i_local.x);
    float _3975 = fast::max(abs(_3968) + abs(_3972), 9.9999997473787516355514526367188e-05);
    float4 _15544;
    float4 _15546;
    if ((in.i_flags.z == 2u) || ((in.i_flags.x & 512u) != 0u))
    {
        _15546 = gFxData_1._data[(in.i_instance * 24u) + 15u];
        _15544 = gFxData_1._data[(in.i_instance * 24u) + 16u];
    }
    else
    {
        _15546 = float4(0.0);
        _15544 = float4(0.0);
    }
    float2 _5243 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
    float2 _5252 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
    float _15564;
    if (in.i_flags.z == 1u)
    {
        float2 _5258 = in.i_local - _5243;
        float _15563;
        do
        {
            if (in.i_shape.w >= 6.282185077667236328125)
            {
                _15563 = abs(length(_5258) - in.i_radii.x) - in.i_radii.y;
                break;
            }
            float _5357 = in.i_shape.z + (in.i_shape.w * 0.5);
            float _5359 = cos(_5357);
            float _5361 = sin(_5357);
            float _5370 = dot(_5258, float2(-_5361, _5359));
            float _5373 = dot(_5258, float2(_5359, _5361));
            float2 _5374 = float2(_5370, _5373);
            float _5377 = abs(_5370);
            _5374.x = _5377;
            float _5380 = in.i_shape.w * 0.5;
            float _5382 = sin(_5380);
            float _5384 = cos(_5380);
            _15563 = (((_5384 * _5377) > (_5382 * _5373)) ? length(_5374 - (float2(_5382, _5384) * in.i_radii.x)) : abs(length(_5374) - in.i_radii.x)) - in.i_radii.y;
            break;
        } while(false);
        _15564 = _15563;
    }
    else
    {
        float _15565;
        if (in.i_flags.z == 2u)
        {
            float2 _5420 = in.i_local - _15546.xy;
            float2 _5423 = _15546.zw - _15546.xy;
            _15565 = length(_5420 - (_5423 * fast::clamp(dot(_5420, _5423) / fast::max(dot(_5423, _5423), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
        }
        else
        {
            float2 _5285 = in.i_local - _5243;
            float _5476 = fast::min(_5252.x, _5252.y);
            float _5479 = fast::min((_5285.x > 0.0) ? ((_5285.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_5285.y > 0.0) ? in.i_radii.w : in.i_radii.x), _5476);
            float _5485 = _5479 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
            float _15548;
            float _15549;
            if (_5485 > _5476)
            {
                float _5499 = in.i_shape.y * fast::clamp((_5476 - _5479) / fast::max(0.60000002384185791015625 * _5479, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                _15549 = _5499;
                _15548 = _5479 * (1.0 + (0.60000002384185791015625 * _5499));
            }
            else
            {
                _15549 = in.i_shape.y;
                _15548 = _5485;
            }
            float2 _5512 = (abs(_5285) - _5252) + float2(_15548);
            float2 _5514 = fast::max(_5512, float2(0.0));
            float _15550;
            if ((_5514.x > 0.0) && (_5514.y > 0.0))
            {
                float _15551;
                if ((_15549 > 0.001000000047497451305389404296875) && (_15548 > 9.9999997473787516355514526367188e-05))
                {
                    float _5531 = 2.0 + (2.0 * _15549);
                    float2 _5536 = _5514 / float2(fast::max(_15548, 9.9999997473787516355514526367188e-05));
                    _15551 = pow(pow(_5536.x, _5531) + pow(_5536.y, _5531), 1.0 / _5531) * _15548;
                }
                else
                {
                    _15551 = length(_5514);
                }
                _15550 = _15551;
            }
            else
            {
                _15550 = fast::max(_5514.x, _5514.y);
            }
            float _5571 = (fast::min(fast::max(_5512.x, _5512.y), 0.0) + _15550) - _15548;
            float _15566;
            if ((in.i_flags.x & 512u) != 0u)
            {
                float2 _5313 = fast::max((_15546.zw - _15546.xy) * 0.5, float2(0.001000000047497451305389404296875));
                float2 _5316 = in.i_local - ((_15546.xy + _15546.zw) * 0.5);
                float _5607 = fast::min(_5313.x, _5313.y);
                float _5610 = fast::min((_5316.x > 0.0) ? ((_5316.y > 0.0) ? _15544.x : _15544.x) : ((_5316.y > 0.0) ? _15544.x : _15544.x), _5607);
                float _5616 = _5610 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                float _15554;
                float _15555;
                if (_5616 > _5607)
                {
                    float _5630 = in.i_shape.y * fast::clamp((_5607 - _5610) / fast::max(0.60000002384185791015625 * _5610, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _15555 = _5630;
                    _15554 = _5610 * (1.0 + (0.60000002384185791015625 * _5630));
                }
                else
                {
                    _15555 = in.i_shape.y;
                    _15554 = _5616;
                }
                float2 _5643 = (abs(_5316) - _5313) + float2(_15554);
                float2 _5645 = fast::max(_5643, float2(0.0));
                float _15556;
                if ((_5645.x > 0.0) && (_5645.y > 0.0))
                {
                    float _15557;
                    if ((_15555 > 0.001000000047497451305389404296875) && (_15554 > 9.9999997473787516355514526367188e-05))
                    {
                        float _5662 = 2.0 + (2.0 * _15555);
                        float2 _5667 = _5645 / float2(fast::max(_15554, 9.9999997473787516355514526367188e-05));
                        _15557 = pow(pow(_5667.x, _5662) + pow(_5667.y, _5662), 1.0 / _5662) * _15554;
                    }
                    else
                    {
                        _15557 = length(_5645);
                    }
                    _15556 = _15557;
                }
                else
                {
                    _15556 = fast::max(_5645.x, _5645.y);
                }
                float _5702 = (fast::min(fast::max(_5643.x, _5643.y), 0.0) + _15556) - _15554;
                float _5707 = fast::max(_15544.y, 9.9999997473787516355514526367188e-05);
                float _5716 = fast::max(_5707 - abs(_5571 - _5702), 0.0) / _5707;
                _15566 = fast::min(_5571, _5702) - (((_5716 * _5716) * _5707) * 0.25);
            }
            else
            {
                _15566 = _5571;
            }
            _15565 = _15566;
        }
        _15564 = _15565;
    }
    float _4000 = fast::clamp(0.5 - (_15564 / _3975), 0.0, 1.0);
    float _17490;
    if ((in.i_flags.x & 1024u) != 0u)
    {
        uint _5732 = (in.i_instance * 24u) + 19u;
        uint _5740 = (in.i_instance * 24u) + 20u;
        float2 _4029 = fast::max((gFxData_1._data[_5732].zw - gFxData_1._data[_5732].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float2 _4032 = in.i_local - ((gFxData_1._data[_5732].xy + gFxData_1._data[_5732].zw) * 0.5);
        float _5778 = fast::min(_4029.x, _4029.y);
        float _5781 = fast::min((_4032.x > 0.0) ? ((_4032.y > 0.0) ? gFxData_1._data[_5740].x : gFxData_1._data[_5740].x) : ((_4032.y > 0.0) ? gFxData_1._data[_5740].x : gFxData_1._data[_5740].x), _5778);
        float _5787 = _5781 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_5740].y));
        float _15567;
        float _15568;
        if (_5787 > _5778)
        {
            float _5801 = gFxData_1._data[_5740].y * fast::clamp((_5778 - _5781) / fast::max(0.60000002384185791015625 * _5781, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _15568 = _5801;
            _15567 = _5781 * (1.0 + (0.60000002384185791015625 * _5801));
        }
        else
        {
            _15568 = gFxData_1._data[_5740].y;
            _15567 = _5787;
        }
        float2 _5814 = (abs(_4032) - _4029) + float2(_15567);
        float2 _5816 = fast::max(_5814, float2(0.0));
        float _15569;
        if ((_5816.x > 0.0) && (_5816.y > 0.0))
        {
            float _15570;
            if ((_15568 > 0.001000000047497451305389404296875) && (_15567 > 9.9999997473787516355514526367188e-05))
            {
                float _5833 = 2.0 + (2.0 * _15568);
                float2 _5838 = _5816 / float2(fast::max(_15567, 9.9999997473787516355514526367188e-05));
                _15570 = pow(pow(_5838.x, _5833) + pow(_5838.y, _5833), 1.0 / _5833) * _15567;
            }
            else
            {
                _15570 = length(_5816);
            }
            _15569 = _15570;
        }
        else
        {
            _15569 = fast::max(_5816.x, _5816.y);
        }
        float _4044 = fast::clamp(0.5 - (((fast::min(fast::max(_5814.x, _5814.y), 0.0) + _15569) - _15567) / _3975), 0.0, 1.0);
        if (_4044 <= 0.0)
        {
            discard_fragment();
        }
        _17490 = _4044;
    }
    else
    {
        _17490 = 1.0;
    }
    bool _4055 = ((in.i_flags.x & 32u) != 0u) && (_4000 >= 0.999000012874603271484375);
    float4 _15659;
    if ((((in.i_flags.x & 4u) != 0u) && (!((in.i_flags.x & 256u) != 0u))) && (!_4055))
    {
        uint _5879 = (in.i_instance * 24u) + 7u;
        uint _5887 = (in.i_instance * 24u) + 8u;
        float2 _4086 = in.i_local - gFxData_1._data[_5887].zw;
        float2 _5928 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _5937 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _15617;
        if (in.i_flags.z == 1u)
        {
            float2 _5943 = _4086 - _5928;
            float _15616;
            do
            {
                if (in.i_shape.w >= 6.282185077667236328125)
                {
                    _15616 = abs(length(_5943) - in.i_radii.x) - in.i_radii.y;
                    break;
                }
                float _6042 = in.i_shape.z + (in.i_shape.w * 0.5);
                float _6044 = cos(_6042);
                float _6046 = sin(_6042);
                float _6055 = dot(_5943, float2(-_6046, _6044));
                float _6058 = dot(_5943, float2(_6044, _6046));
                float2 _6059 = float2(_6055, _6058);
                float _6062 = abs(_6055);
                _6059.x = _6062;
                float _6065 = in.i_shape.w * 0.5;
                float _6067 = sin(_6065);
                float _6069 = cos(_6065);
                _15616 = (((_6069 * _6062) > (_6067 * _6058)) ? length(_6059 - (float2(_6067, _6069) * in.i_radii.x)) : abs(length(_6059) - in.i_radii.x)) - in.i_radii.y;
                break;
            } while(false);
            _15617 = _15616;
        }
        else
        {
            float _15618;
            if (in.i_flags.z == 2u)
            {
                float2 _6105 = _4086 - _15546.xy;
                float2 _6108 = _15546.zw - _15546.xy;
                _15618 = length(_6105 - (_6108 * fast::clamp(dot(_6105, _6108) / fast::max(dot(_6108, _6108), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
            }
            else
            {
                float2 _5970 = _4086 - _5928;
                float _6161 = fast::min(_5937.x, _5937.y);
                float _6164 = fast::min((_5970.x > 0.0) ? ((_5970.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_5970.y > 0.0) ? in.i_radii.w : in.i_radii.x), _6161);
                float _6170 = _6164 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                float _15601;
                float _15602;
                if (_6170 > _6161)
                {
                    float _6184 = in.i_shape.y * fast::clamp((_6161 - _6164) / fast::max(0.60000002384185791015625 * _6164, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _15602 = _6184;
                    _15601 = _6164 * (1.0 + (0.60000002384185791015625 * _6184));
                }
                else
                {
                    _15602 = in.i_shape.y;
                    _15601 = _6170;
                }
                float2 _6197 = (abs(_5970) - _5937) + float2(_15601);
                float2 _6199 = fast::max(_6197, float2(0.0));
                float _15603;
                if ((_6199.x > 0.0) && (_6199.y > 0.0))
                {
                    float _15604;
                    if ((_15602 > 0.001000000047497451305389404296875) && (_15601 > 9.9999997473787516355514526367188e-05))
                    {
                        float _6216 = 2.0 + (2.0 * _15602);
                        float2 _6221 = _6199 / float2(fast::max(_15601, 9.9999997473787516355514526367188e-05));
                        _15604 = pow(pow(_6221.x, _6216) + pow(_6221.y, _6216), 1.0 / _6216) * _15601;
                    }
                    else
                    {
                        _15604 = length(_6199);
                    }
                    _15603 = _15604;
                }
                else
                {
                    _15603 = fast::max(_6199.x, _6199.y);
                }
                float _6256 = (fast::min(fast::max(_6197.x, _6197.y), 0.0) + _15603) - _15601;
                float _15619;
                if ((in.i_flags.x & 512u) != 0u)
                {
                    float2 _5998 = fast::max((_15546.zw - _15546.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _6001 = _4086 - ((_15546.xy + _15546.zw) * 0.5);
                    float _6292 = fast::min(_5998.x, _5998.y);
                    float _6295 = fast::min((_6001.x > 0.0) ? ((_6001.y > 0.0) ? _15544.x : _15544.x) : ((_6001.y > 0.0) ? _15544.x : _15544.x), _6292);
                    float _6301 = _6295 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _15607;
                    float _15608;
                    if (_6301 > _6292)
                    {
                        float _6315 = in.i_shape.y * fast::clamp((_6292 - _6295) / fast::max(0.60000002384185791015625 * _6295, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _15608 = _6315;
                        _15607 = _6295 * (1.0 + (0.60000002384185791015625 * _6315));
                    }
                    else
                    {
                        _15608 = in.i_shape.y;
                        _15607 = _6301;
                    }
                    float2 _6328 = (abs(_6001) - _5998) + float2(_15607);
                    float2 _6330 = fast::max(_6328, float2(0.0));
                    float _15609;
                    if ((_6330.x > 0.0) && (_6330.y > 0.0))
                    {
                        float _15610;
                        if ((_15608 > 0.001000000047497451305389404296875) && (_15607 > 9.9999997473787516355514526367188e-05))
                        {
                            float _6347 = 2.0 + (2.0 * _15608);
                            float2 _6352 = _6330 / float2(fast::max(_15607, 9.9999997473787516355514526367188e-05));
                            _15610 = pow(pow(_6352.x, _6347) + pow(_6352.y, _6347), 1.0 / _6347) * _15607;
                        }
                        else
                        {
                            _15610 = length(_6330);
                        }
                        _15609 = _15610;
                    }
                    else
                    {
                        _15609 = fast::max(_6330.x, _6330.y);
                    }
                    float _6387 = (fast::min(fast::max(_6328.x, _6328.y), 0.0) + _15609) - _15607;
                    float _6392 = fast::max(_15544.y, 9.9999997473787516355514526367188e-05);
                    float _6401 = fast::max(_6392 - abs(_6256 - _6387), 0.0) / _6392;
                    _15619 = fast::min(_6256, _6387) - (((_6401 * _6401) * _6392) * 0.25);
                }
                else
                {
                    _15619 = _6256;
                }
                _15618 = _15619;
            }
            _15617 = _15618;
        }
        float _4095 = (_15617 - gFxData_1._data[_5887].y) / (fast::max(gFxData_1._data[_5887].x * 0.5, _3975 * 0.5) * 1.41421353816986083984375);
        float _6418 = sign(_4095);
        float _6420 = abs(_4095);
        float _6431 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_6420 * _6420))) * _6420)) * _6420);
        float _6434 = _6431 * _6431;
        float _6449 = fast::clamp(gFxData_1._data[_5879].w * (0.5 - (0.5 * (_6418 - (_6418 / (_6434 * _6434))))), 0.0, 1.0);
        _15659 = float4(gFxData_1._data[_5879].xyz * _6449, _6449);
    }
    else
    {
        _15659 = float4(0.0);
    }
    float4 _16551;
    if (((in.i_flags.x & 8u) != 0u) && (!_4055))
    {
        uint _6473 = (in.i_instance * 24u) + 9u;
        uint _6481 = (in.i_instance * 24u) + 10u;
        float _4123 = fast::max(gFxData_1._data[_6481].x, 0.001000000047497451305389404296875);
        float _15652;
        float _15655;
        if ((in.i_flags.x & 8192u) != 0u)
        {
            uint _6489 = (in.i_instance * 24u) + 22u;
            float2 _4139 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _4148 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float2 _4155 = in.i_rect.xy - gFxData_1._data[_6489].xy;
            float2 _4162 = gFxData_1._data[_6489].zw - in.i_rect.zw;
            float2 _4192 = fast::clamp(float2((in.i_local.x < _4139.x) ? _4155.x : _4162.x, (in.i_local.y < _4139.y) ? _4155.y : _4162.y) * float2(0.58823525905609130859375), float2(fast::min(_4123, 1.5)), float2(_4123));
            float _4197 = fast::min(_4148.x, _4148.y);
            float _15650;
            if (in.i_flags.z == 0u)
            {
                _15650 = fast::min(((in.i_local.x > _4139.x) ? ((in.i_local.y > _4139.y) ? in.i_radii.z : in.i_radii.y) : ((in.i_local.y > _4139.y) ? in.i_radii.w : in.i_radii.x)) * (1.0 + (0.60000002384185791015625 * in.i_shape.y)), _4197);
            }
            else
            {
                _15650 = _4197;
            }
            float2 _4246 = fast::max(abs(in.i_local - _4139) - (_4148 - float2(_15650)), float2(0.0));
            float _4248 = length(_4246);
            float2 _4256 = select(float2(0.707099974155426025390625), _4246 / float2(_4248), bool2(_4248 > 9.9999997473787516355514526367188e-05));
            float2 _4271 = in.i_local - gFxData_1._data[_6489].xy;
            float2 _4276 = gFxData_1._data[_6489].zw - in.i_local;
            _15655 = fast::clamp(fast::min(fast::min(_4271.x, _4271.y), fast::min(_4276.x, _4276.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _15652 = rsqrt(dot(_4256 * _4256, float2(1.0) / (_4192 * _4192)));
        }
        else
        {
            _15655 = 1.0;
            _15652 = _4123;
        }
        float _4294 = fast::max(_15564, 0.0) / _15652;
        float _6499 = fast::clamp(gFxData_1._data[_6473].w * fast::clamp((exp(((-_4294) * _4294) * 2.2000000476837158203125) * gFxData_1._data[_6481].y) * _15655, 0.0, 1.0), 0.0, 1.0);
        _16551 = float4(gFxData_1._data[_6473].xyz * _6499, _6499) + (_15659 * (1.0 - _6499));
    }
    else
    {
        _16551 = _15659;
    }
    float4 _17062;
    float4 _17075;
    float _17456;
    if (((in.i_flags.x & 32u) != 0u) && (_4000 > 0.0))
    {
        uint _6523 = (in.i_instance * 24u) + 11u;
        uint _6531 = (in.i_instance * 24u) + 12u;
        uint _6539 = (in.i_instance * 24u) + 13u;
        uint _6547 = (in.i_instance * 24u) + 16u;
        float _6612 = gFxData_1._data[_6523].x * _184.gDisplay.z;
        float2 _6623 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(1.0));
        float _6631 = fast::clamp(gFxData_1._data[_6523].z, 0.001000000047497451305389404296875, fast::min(_6623.x, _6623.y));
        float _6640 = fast::clamp(1.0 - (fast::max(-_15564, 0.0) / _6631), 0.0, 1.0);
        float _6646 = sqrt(fast::clamp(1.0 - (_6640 * _6640), 0.0, 1.0));
        float2 _15751;
        float3 _16328;
        if (_6640 > 0.0)
        {
            float2 _7020 = in.i_local + float2(0.5, 0.0);
            float2 _7088 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _7097 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _15691;
            if (in.i_flags.z == 1u)
            {
                float2 _7103 = _7020 - _7088;
                float _15690;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _15690 = abs(length(_7103) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _7202 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _7204 = cos(_7202);
                    float _7206 = sin(_7202);
                    float _7215 = dot(_7103, float2(-_7206, _7204));
                    float _7218 = dot(_7103, float2(_7204, _7206));
                    float2 _7219 = float2(_7215, _7218);
                    float _7222 = abs(_7215);
                    _7219.x = _7222;
                    float _7225 = in.i_shape.w * 0.5;
                    float _7227 = sin(_7225);
                    float _7229 = cos(_7225);
                    _15690 = (((_7229 * _7222) > (_7227 * _7218)) ? length(_7219 - (float2(_7227, _7229) * in.i_radii.x)) : abs(length(_7219) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _15691 = _15690;
            }
            else
            {
                float _15692;
                if (in.i_flags.z == 2u)
                {
                    float2 _7265 = _7020 - _15546.xy;
                    float2 _7268 = _15546.zw - _15546.xy;
                    _15692 = length(_7265 - (_7268 * fast::clamp(dot(_7265, _7268) / fast::max(dot(_7268, _7268), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _7130 = _7020 - _7088;
                    float _7321 = fast::min(_7097.x, _7097.y);
                    float _7324 = fast::min((_7130.x > 0.0) ? ((_7130.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_7130.y > 0.0) ? in.i_radii.w : in.i_radii.x), _7321);
                    float _7330 = _7324 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _15675;
                    float _15676;
                    if (_7330 > _7321)
                    {
                        float _7344 = in.i_shape.y * fast::clamp((_7321 - _7324) / fast::max(0.60000002384185791015625 * _7324, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _15676 = _7344;
                        _15675 = _7324 * (1.0 + (0.60000002384185791015625 * _7344));
                    }
                    else
                    {
                        _15676 = in.i_shape.y;
                        _15675 = _7330;
                    }
                    float2 _7357 = (abs(_7130) - _7097) + float2(_15675);
                    float2 _7359 = fast::max(_7357, float2(0.0));
                    float _15677;
                    if ((_7359.x > 0.0) && (_7359.y > 0.0))
                    {
                        float _15678;
                        if ((_15676 > 0.001000000047497451305389404296875) && (_15675 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7376 = 2.0 + (2.0 * _15676);
                            float2 _7381 = _7359 / float2(fast::max(_15675, 9.9999997473787516355514526367188e-05));
                            _15678 = pow(pow(_7381.x, _7376) + pow(_7381.y, _7376), 1.0 / _7376) * _15675;
                        }
                        else
                        {
                            _15678 = length(_7359);
                        }
                        _15677 = _15678;
                    }
                    else
                    {
                        _15677 = fast::max(_7359.x, _7359.y);
                    }
                    float _7416 = (fast::min(fast::max(_7357.x, _7357.y), 0.0) + _15677) - _15675;
                    float _15693;
                    if ((in.i_flags.x & 512u) != 0u)
                    {
                        float2 _7158 = fast::max((_15546.zw - _15546.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _7161 = _7020 - ((_15546.xy + _15546.zw) * 0.5);
                        float _7452 = fast::min(_7158.x, _7158.y);
                        float _7455 = fast::min((_7161.x > 0.0) ? ((_7161.y > 0.0) ? gFxData_1._data[_6547].x : gFxData_1._data[_6547].x) : ((_7161.y > 0.0) ? gFxData_1._data[_6547].x : gFxData_1._data[_6547].x), _7452);
                        float _7461 = _7455 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _15681;
                        float _15682;
                        if (_7461 > _7452)
                        {
                            float _7475 = in.i_shape.y * fast::clamp((_7452 - _7455) / fast::max(0.60000002384185791015625 * _7455, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _15682 = _7475;
                            _15681 = _7455 * (1.0 + (0.60000002384185791015625 * _7475));
                        }
                        else
                        {
                            _15682 = in.i_shape.y;
                            _15681 = _7461;
                        }
                        float2 _7488 = (abs(_7161) - _7158) + float2(_15681);
                        float2 _7490 = fast::max(_7488, float2(0.0));
                        float _15683;
                        if ((_7490.x > 0.0) && (_7490.y > 0.0))
                        {
                            float _15684;
                            if ((_15682 > 0.001000000047497451305389404296875) && (_15681 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7507 = 2.0 + (2.0 * _15682);
                                float2 _7512 = _7490 / float2(fast::max(_15681, 9.9999997473787516355514526367188e-05));
                                _15684 = pow(pow(_7512.x, _7507) + pow(_7512.y, _7507), 1.0 / _7507) * _15681;
                            }
                            else
                            {
                                _15684 = length(_7490);
                            }
                            _15683 = _15684;
                        }
                        else
                        {
                            _15683 = fast::max(_7490.x, _7490.y);
                        }
                        float _7547 = (fast::min(fast::max(_7488.x, _7488.y), 0.0) + _15683) - _15681;
                        float _7552 = fast::max(gFxData_1._data[_6547].y, 9.9999997473787516355514526367188e-05);
                        float _7561 = fast::max(_7552 - abs(_7416 - _7547), 0.0) / _7552;
                        _15693 = fast::min(_7416, _7547) - (((_7561 * _7561) * _7552) * 0.25);
                    }
                    else
                    {
                        _15693 = _7416;
                    }
                    _15692 = _15693;
                }
                _15691 = _15692;
            }
            float2 _7024 = in.i_local - float2(0.5, 0.0);
            float2 _7610 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _7619 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _15710;
            if (in.i_flags.z == 1u)
            {
                float2 _7625 = _7024 - _7610;
                float _15709;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _15709 = abs(length(_7625) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _7724 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _7726 = cos(_7724);
                    float _7728 = sin(_7724);
                    float _7737 = dot(_7625, float2(-_7728, _7726));
                    float _7740 = dot(_7625, float2(_7726, _7728));
                    float2 _7741 = float2(_7737, _7740);
                    float _7744 = abs(_7737);
                    _7741.x = _7744;
                    float _7747 = in.i_shape.w * 0.5;
                    float _7749 = sin(_7747);
                    float _7751 = cos(_7747);
                    _15709 = (((_7751 * _7744) > (_7749 * _7740)) ? length(_7741 - (float2(_7749, _7751) * in.i_radii.x)) : abs(length(_7741) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _15710 = _15709;
            }
            else
            {
                float _15711;
                if (in.i_flags.z == 2u)
                {
                    float2 _7787 = _7024 - _15546.xy;
                    float2 _7790 = _15546.zw - _15546.xy;
                    _15711 = length(_7787 - (_7790 * fast::clamp(dot(_7787, _7790) / fast::max(dot(_7790, _7790), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _7652 = _7024 - _7610;
                    float _7843 = fast::min(_7619.x, _7619.y);
                    float _7846 = fast::min((_7652.x > 0.0) ? ((_7652.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_7652.y > 0.0) ? in.i_radii.w : in.i_radii.x), _7843);
                    float _7852 = _7846 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _15694;
                    float _15695;
                    if (_7852 > _7843)
                    {
                        float _7866 = in.i_shape.y * fast::clamp((_7843 - _7846) / fast::max(0.60000002384185791015625 * _7846, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _15695 = _7866;
                        _15694 = _7846 * (1.0 + (0.60000002384185791015625 * _7866));
                    }
                    else
                    {
                        _15695 = in.i_shape.y;
                        _15694 = _7852;
                    }
                    float2 _7879 = (abs(_7652) - _7619) + float2(_15694);
                    float2 _7881 = fast::max(_7879, float2(0.0));
                    float _15696;
                    if ((_7881.x > 0.0) && (_7881.y > 0.0))
                    {
                        float _15697;
                        if ((_15695 > 0.001000000047497451305389404296875) && (_15694 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7898 = 2.0 + (2.0 * _15695);
                            float2 _7903 = _7881 / float2(fast::max(_15694, 9.9999997473787516355514526367188e-05));
                            _15697 = pow(pow(_7903.x, _7898) + pow(_7903.y, _7898), 1.0 / _7898) * _15694;
                        }
                        else
                        {
                            _15697 = length(_7881);
                        }
                        _15696 = _15697;
                    }
                    else
                    {
                        _15696 = fast::max(_7881.x, _7881.y);
                    }
                    float _7938 = (fast::min(fast::max(_7879.x, _7879.y), 0.0) + _15696) - _15694;
                    float _15712;
                    if ((in.i_flags.x & 512u) != 0u)
                    {
                        float2 _7680 = fast::max((_15546.zw - _15546.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _7683 = _7024 - ((_15546.xy + _15546.zw) * 0.5);
                        float _7974 = fast::min(_7680.x, _7680.y);
                        float _7977 = fast::min((_7683.x > 0.0) ? ((_7683.y > 0.0) ? gFxData_1._data[_6547].x : gFxData_1._data[_6547].x) : ((_7683.y > 0.0) ? gFxData_1._data[_6547].x : gFxData_1._data[_6547].x), _7974);
                        float _7983 = _7977 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _15700;
                        float _15701;
                        if (_7983 > _7974)
                        {
                            float _7997 = in.i_shape.y * fast::clamp((_7974 - _7977) / fast::max(0.60000002384185791015625 * _7977, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _15701 = _7997;
                            _15700 = _7977 * (1.0 + (0.60000002384185791015625 * _7997));
                        }
                        else
                        {
                            _15701 = in.i_shape.y;
                            _15700 = _7983;
                        }
                        float2 _8010 = (abs(_7683) - _7680) + float2(_15700);
                        float2 _8012 = fast::max(_8010, float2(0.0));
                        float _15702;
                        if ((_8012.x > 0.0) && (_8012.y > 0.0))
                        {
                            float _15703;
                            if ((_15701 > 0.001000000047497451305389404296875) && (_15700 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8029 = 2.0 + (2.0 * _15701);
                                float2 _8034 = _8012 / float2(fast::max(_15700, 9.9999997473787516355514526367188e-05));
                                _15703 = pow(pow(_8034.x, _8029) + pow(_8034.y, _8029), 1.0 / _8029) * _15700;
                            }
                            else
                            {
                                _15703 = length(_8012);
                            }
                            _15702 = _15703;
                        }
                        else
                        {
                            _15702 = fast::max(_8012.x, _8012.y);
                        }
                        float _8069 = (fast::min(fast::max(_8010.x, _8010.y), 0.0) + _15702) - _15700;
                        float _8074 = fast::max(gFxData_1._data[_6547].y, 9.9999997473787516355514526367188e-05);
                        float _8083 = fast::max(_8074 - abs(_7938 - _8069), 0.0) / _8074;
                        _15712 = fast::min(_7938, _8069) - (((_8083 * _8083) * _8074) * 0.25);
                    }
                    else
                    {
                        _15712 = _7938;
                    }
                    _15711 = _15712;
                }
                _15710 = _15711;
            }
            float2 _7029 = in.i_local + float2(0.0, 0.5);
            float2 _8132 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _8141 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _15729;
            if (in.i_flags.z == 1u)
            {
                float2 _8147 = _7029 - _8132;
                float _15728;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _15728 = abs(length(_8147) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _8246 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _8248 = cos(_8246);
                    float _8250 = sin(_8246);
                    float _8259 = dot(_8147, float2(-_8250, _8248));
                    float _8262 = dot(_8147, float2(_8248, _8250));
                    float2 _8263 = float2(_8259, _8262);
                    float _8266 = abs(_8259);
                    _8263.x = _8266;
                    float _8269 = in.i_shape.w * 0.5;
                    float _8271 = sin(_8269);
                    float _8273 = cos(_8269);
                    _15728 = (((_8273 * _8266) > (_8271 * _8262)) ? length(_8263 - (float2(_8271, _8273) * in.i_radii.x)) : abs(length(_8263) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _15729 = _15728;
            }
            else
            {
                float _15730;
                if (in.i_flags.z == 2u)
                {
                    float2 _8309 = _7029 - _15546.xy;
                    float2 _8312 = _15546.zw - _15546.xy;
                    _15730 = length(_8309 - (_8312 * fast::clamp(dot(_8309, _8312) / fast::max(dot(_8312, _8312), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _8174 = _7029 - _8132;
                    float _8365 = fast::min(_8141.x, _8141.y);
                    float _8368 = fast::min((_8174.x > 0.0) ? ((_8174.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_8174.y > 0.0) ? in.i_radii.w : in.i_radii.x), _8365);
                    float _8374 = _8368 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _15713;
                    float _15714;
                    if (_8374 > _8365)
                    {
                        float _8388 = in.i_shape.y * fast::clamp((_8365 - _8368) / fast::max(0.60000002384185791015625 * _8368, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _15714 = _8388;
                        _15713 = _8368 * (1.0 + (0.60000002384185791015625 * _8388));
                    }
                    else
                    {
                        _15714 = in.i_shape.y;
                        _15713 = _8374;
                    }
                    float2 _8401 = (abs(_8174) - _8141) + float2(_15713);
                    float2 _8403 = fast::max(_8401, float2(0.0));
                    float _15715;
                    if ((_8403.x > 0.0) && (_8403.y > 0.0))
                    {
                        float _15716;
                        if ((_15714 > 0.001000000047497451305389404296875) && (_15713 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8420 = 2.0 + (2.0 * _15714);
                            float2 _8425 = _8403 / float2(fast::max(_15713, 9.9999997473787516355514526367188e-05));
                            _15716 = pow(pow(_8425.x, _8420) + pow(_8425.y, _8420), 1.0 / _8420) * _15713;
                        }
                        else
                        {
                            _15716 = length(_8403);
                        }
                        _15715 = _15716;
                    }
                    else
                    {
                        _15715 = fast::max(_8403.x, _8403.y);
                    }
                    float _8460 = (fast::min(fast::max(_8401.x, _8401.y), 0.0) + _15715) - _15713;
                    float _15731;
                    if ((in.i_flags.x & 512u) != 0u)
                    {
                        float2 _8202 = fast::max((_15546.zw - _15546.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _8205 = _7029 - ((_15546.xy + _15546.zw) * 0.5);
                        float _8496 = fast::min(_8202.x, _8202.y);
                        float _8499 = fast::min((_8205.x > 0.0) ? ((_8205.y > 0.0) ? gFxData_1._data[_6547].x : gFxData_1._data[_6547].x) : ((_8205.y > 0.0) ? gFxData_1._data[_6547].x : gFxData_1._data[_6547].x), _8496);
                        float _8505 = _8499 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _15719;
                        float _15720;
                        if (_8505 > _8496)
                        {
                            float _8519 = in.i_shape.y * fast::clamp((_8496 - _8499) / fast::max(0.60000002384185791015625 * _8499, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _15720 = _8519;
                            _15719 = _8499 * (1.0 + (0.60000002384185791015625 * _8519));
                        }
                        else
                        {
                            _15720 = in.i_shape.y;
                            _15719 = _8505;
                        }
                        float2 _8532 = (abs(_8205) - _8202) + float2(_15719);
                        float2 _8534 = fast::max(_8532, float2(0.0));
                        float _15721;
                        if ((_8534.x > 0.0) && (_8534.y > 0.0))
                        {
                            float _15722;
                            if ((_15720 > 0.001000000047497451305389404296875) && (_15719 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8551 = 2.0 + (2.0 * _15720);
                                float2 _8556 = _8534 / float2(fast::max(_15719, 9.9999997473787516355514526367188e-05));
                                _15722 = pow(pow(_8556.x, _8551) + pow(_8556.y, _8551), 1.0 / _8551) * _15719;
                            }
                            else
                            {
                                _15722 = length(_8534);
                            }
                            _15721 = _15722;
                        }
                        else
                        {
                            _15721 = fast::max(_8534.x, _8534.y);
                        }
                        float _8591 = (fast::min(fast::max(_8532.x, _8532.y), 0.0) + _15721) - _15719;
                        float _8596 = fast::max(gFxData_1._data[_6547].y, 9.9999997473787516355514526367188e-05);
                        float _8605 = fast::max(_8596 - abs(_8460 - _8591), 0.0) / _8596;
                        _15731 = fast::min(_8460, _8591) - (((_8605 * _8605) * _8596) * 0.25);
                    }
                    else
                    {
                        _15731 = _8460;
                    }
                    _15730 = _15731;
                }
                _15729 = _15730;
            }
            float2 _7033 = in.i_local - float2(0.0, 0.5);
            float2 _8654 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _8663 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _15748;
            if (in.i_flags.z == 1u)
            {
                float2 _8669 = _7033 - _8654;
                float _15747;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _15747 = abs(length(_8669) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _8768 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _8770 = cos(_8768);
                    float _8772 = sin(_8768);
                    float _8781 = dot(_8669, float2(-_8772, _8770));
                    float _8784 = dot(_8669, float2(_8770, _8772));
                    float2 _8785 = float2(_8781, _8784);
                    float _8788 = abs(_8781);
                    _8785.x = _8788;
                    float _8791 = in.i_shape.w * 0.5;
                    float _8793 = sin(_8791);
                    float _8795 = cos(_8791);
                    _15747 = (((_8795 * _8788) > (_8793 * _8784)) ? length(_8785 - (float2(_8793, _8795) * in.i_radii.x)) : abs(length(_8785) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _15748 = _15747;
            }
            else
            {
                float _15749;
                if (in.i_flags.z == 2u)
                {
                    float2 _8831 = _7033 - _15546.xy;
                    float2 _8834 = _15546.zw - _15546.xy;
                    _15749 = length(_8831 - (_8834 * fast::clamp(dot(_8831, _8834) / fast::max(dot(_8834, _8834), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _8696 = _7033 - _8654;
                    float _8887 = fast::min(_8663.x, _8663.y);
                    float _8890 = fast::min((_8696.x > 0.0) ? ((_8696.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_8696.y > 0.0) ? in.i_radii.w : in.i_radii.x), _8887);
                    float _8896 = _8890 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _15732;
                    float _15733;
                    if (_8896 > _8887)
                    {
                        float _8910 = in.i_shape.y * fast::clamp((_8887 - _8890) / fast::max(0.60000002384185791015625 * _8890, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _15733 = _8910;
                        _15732 = _8890 * (1.0 + (0.60000002384185791015625 * _8910));
                    }
                    else
                    {
                        _15733 = in.i_shape.y;
                        _15732 = _8896;
                    }
                    float2 _8923 = (abs(_8696) - _8663) + float2(_15732);
                    float2 _8925 = fast::max(_8923, float2(0.0));
                    float _15734;
                    if ((_8925.x > 0.0) && (_8925.y > 0.0))
                    {
                        float _15735;
                        if ((_15733 > 0.001000000047497451305389404296875) && (_15732 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8942 = 2.0 + (2.0 * _15733);
                            float2 _8947 = _8925 / float2(fast::max(_15732, 9.9999997473787516355514526367188e-05));
                            _15735 = pow(pow(_8947.x, _8942) + pow(_8947.y, _8942), 1.0 / _8942) * _15732;
                        }
                        else
                        {
                            _15735 = length(_8925);
                        }
                        _15734 = _15735;
                    }
                    else
                    {
                        _15734 = fast::max(_8925.x, _8925.y);
                    }
                    float _8982 = (fast::min(fast::max(_8923.x, _8923.y), 0.0) + _15734) - _15732;
                    float _15750;
                    if ((in.i_flags.x & 512u) != 0u)
                    {
                        float2 _8724 = fast::max((_15546.zw - _15546.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _8727 = _7033 - ((_15546.xy + _15546.zw) * 0.5);
                        float _9018 = fast::min(_8724.x, _8724.y);
                        float _9021 = fast::min((_8727.x > 0.0) ? ((_8727.y > 0.0) ? gFxData_1._data[_6547].x : gFxData_1._data[_6547].x) : ((_8727.y > 0.0) ? gFxData_1._data[_6547].x : gFxData_1._data[_6547].x), _9018);
                        float _9027 = _9021 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _15738;
                        float _15739;
                        if (_9027 > _9018)
                        {
                            float _9041 = in.i_shape.y * fast::clamp((_9018 - _9021) / fast::max(0.60000002384185791015625 * _9021, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _15739 = _9041;
                            _15738 = _9021 * (1.0 + (0.60000002384185791015625 * _9041));
                        }
                        else
                        {
                            _15739 = in.i_shape.y;
                            _15738 = _9027;
                        }
                        float2 _9054 = (abs(_8727) - _8724) + float2(_15738);
                        float2 _9056 = fast::max(_9054, float2(0.0));
                        float _15740;
                        if ((_9056.x > 0.0) && (_9056.y > 0.0))
                        {
                            float _15741;
                            if ((_15739 > 0.001000000047497451305389404296875) && (_15738 > 9.9999997473787516355514526367188e-05))
                            {
                                float _9073 = 2.0 + (2.0 * _15739);
                                float2 _9078 = _9056 / float2(fast::max(_15738, 9.9999997473787516355514526367188e-05));
                                _15741 = pow(pow(_9078.x, _9073) + pow(_9078.y, _9073), 1.0 / _9073) * _15738;
                            }
                            else
                            {
                                _15741 = length(_9056);
                            }
                            _15740 = _15741;
                        }
                        else
                        {
                            _15740 = fast::max(_9056.x, _9056.y);
                        }
                        float _9113 = (fast::min(fast::max(_9054.x, _9054.y), 0.0) + _15740) - _15738;
                        float _9118 = fast::max(gFxData_1._data[_6547].y, 9.9999997473787516355514526367188e-05);
                        float _9127 = fast::max(_9118 - abs(_8982 - _9113), 0.0) / _9118;
                        _15750 = fast::min(_8982, _9113) - (((_9127 * _9127) * _9118) * 0.25);
                    }
                    else
                    {
                        _15750 = _8982;
                    }
                    _15749 = _15750;
                }
                _15748 = _15749;
            }
            float2 _7039 = float2(_15691 - _15710, _15729 - _15748);
            float _7041 = length(_7039);
            float2 _7049 = select(float2(0.0, -1.0), _7039 / float2(_7041), bool2(_7041 > 9.9999997473787516355514526367188e-06));
            _16328 = fast::normalize(float3(_7049 * fast::min(_6640 / fast::max(_6646, 0.001000000047497451305389404296875), 8.0), 1.0));
            _15751 = _7049;
        }
        else
        {
            _16328 = float3(0.0, 0.0, 1.0);
            _15751 = float2(0.0, -1.0);
        }
        float2 _6672 = ((-_15751) * gFxData_1._data[_6523].y) * (1.0 - _6646);
        float2 _15752;
        if (gFxData_1._data[_6547].z > 0.0)
        {
            _15752 = (((in.i_rect.xy + in.i_rect.zw) * 0.5) - in.i_local) * (gFxData_1._data[_6547].z / (1.0 + gFxData_1._data[_6547].z));
        }
        else
        {
            _15752 = float2(0.0);
        }
        float2 _6698 = _15543 * _184.gTarget.zw;
        float2 _6705 = _184.gDisplay.zw * _184.gTarget.zw;
        float2 _6710 = (_6672 + _15752) * _6705;
        float2 _6713 = _6672 * _6705;
        float3 _16210;
        float _16228;
        float3 _16431;
        if (_184.gTime.z > 0.5)
        {
            float2 _6720 = _6698 + _6710;
            float _9152 = fast::clamp(log2(fast::max(_6612, 1.0)) - 1.0, 0.0, 5.0);
            int _9155 = int(floor(_9152));
            float _9159 = _9152 - float(_9155);
            bool _9164 = (_9159 > 0.0199999995529651641845703125) && (_9155 < 5);
            float3 _15823;
            _15823 = float3(0.0);
            float3 _9193;
            for (int _15753 = 0; _15753 < 2; _15823 = _9193, _15753++)
            {
                if ((_15753 > 0) && (!_9164))
                {
                    break;
                }
                int _9179 = _9155 + _15753;
                int _9217 = clamp(_9179, 1, 5);
                float2 _9286 = (_6720 * _184.gLevel[_9217].xy) - float2(0.5);
                float2 _9288 = floor(_9286);
                float2 _9291 = _9286 - _9288;
                float2 _9294 = _9291 * _9291;
                float2 _9297 = _9294 * _9291;
                float2 _9316 = (((_9297 * 3.0) - (_9294 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                float2 _9329 = _9297 * 0.16666667163372039794921875;
                float2 _9332 = (((((-_9297) + (_9294 * 3.0)) - (_9291 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9316;
                float2 _9336 = (((((_9297 * (-3.0)) + (_9294 * 3.0)) + (_9291 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9329;
                float2 _9348 = ((_9288 - float2(0.5)) + (_9316 / _9332)) * _184.gLevel[_9217].zw;
                float2 _9360 = ((_9288 + float2(1.5)) + (_9329 / _9336)) * _184.gLevel[_9217].zw;
                float4 _15790;
                if (_9179 <= 0)
                {
                    float2 _15789;
                    if (_184.gConv.x > 0.5)
                    {
                        float2 _14874 = _6720;
                        _14874.y = 1.0 - _6720.y;
                        _15789 = _14874;
                    }
                    else
                    {
                        _15789 = _6720;
                    }
                    _15790 = gBackdrop0.sample(gLinear, _15789, level(0.0));
                }
                else
                {
                    float4 _15791;
                    if (_9179 == 1)
                    {
                        float2 _15782;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _14879 = _9348;
                            _14879.y = 1.0 - _9348.y;
                            _15782 = _14879;
                        }
                        else
                        {
                            _15782 = _9348;
                        }
                        float2 _9406 = float2(_9360.x, _9348.y);
                        float2 _15783;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _14886 = _9406;
                            _14886.y = 1.0 - _9348.y;
                            _15783 = _14886;
                        }
                        else
                        {
                            _15783 = _9406;
                        }
                        float2 _9424 = float2(_9348.x, _9360.y);
                        float2 _15785;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _14893 = _9424;
                            _14893.y = 1.0 - _9360.y;
                            _15785 = _14893;
                        }
                        else
                        {
                            _15785 = _9424;
                        }
                        float2 _15787;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _14900 = _9360;
                            _14900.y = 1.0 - _9360.y;
                            _15787 = _14900;
                        }
                        else
                        {
                            _15787 = _9360;
                        }
                        _15791 = (((gBackdrop1.sample(gLinear, _15782, level(0.0)) * (_9332.x * _9332.y)) + (gBackdrop1.sample(gLinear, _15783, level(0.0)) * (_9336.x * _9332.y))) + (gBackdrop1.sample(gLinear, _15785, level(0.0)) * (_9332.x * _9336.y))) + (gBackdrop1.sample(gLinear, _15787, level(0.0)) * (_9336.x * _9336.y));
                    }
                    else
                    {
                        float4 _15792;
                        if (_9179 == 2)
                        {
                            float2 _15775;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _14907 = _9348;
                                _14907.y = 1.0 - _9348.y;
                                _15775 = _14907;
                            }
                            else
                            {
                                _15775 = _9348;
                            }
                            float2 _9536 = float2(_9360.x, _9348.y);
                            float2 _15776;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _14914 = _9536;
                                _14914.y = 1.0 - _9348.y;
                                _15776 = _14914;
                            }
                            else
                            {
                                _15776 = _9536;
                            }
                            float2 _9554 = float2(_9348.x, _9360.y);
                            float2 _15778;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _14921 = _9554;
                                _14921.y = 1.0 - _9360.y;
                                _15778 = _14921;
                            }
                            else
                            {
                                _15778 = _9554;
                            }
                            float2 _15780;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _14928 = _9360;
                                _14928.y = 1.0 - _9360.y;
                                _15780 = _14928;
                            }
                            else
                            {
                                _15780 = _9360;
                            }
                            _15792 = (((gBackdrop2.sample(gLinear, _15775, level(0.0)) * (_9332.x * _9332.y)) + (gBackdrop2.sample(gLinear, _15776, level(0.0)) * (_9336.x * _9332.y))) + (gBackdrop2.sample(gLinear, _15778, level(0.0)) * (_9332.x * _9336.y))) + (gBackdrop2.sample(gLinear, _15780, level(0.0)) * (_9336.x * _9336.y));
                        }
                        else
                        {
                            float4 _15793;
                            if (_9179 == 3)
                            {
                                float2 _15768;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _14935 = _9348;
                                    _14935.y = 1.0 - _9348.y;
                                    _15768 = _14935;
                                }
                                else
                                {
                                    _15768 = _9348;
                                }
                                float2 _9666 = float2(_9360.x, _9348.y);
                                float2 _15769;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _14942 = _9666;
                                    _14942.y = 1.0 - _9348.y;
                                    _15769 = _14942;
                                }
                                else
                                {
                                    _15769 = _9666;
                                }
                                float2 _9684 = float2(_9348.x, _9360.y);
                                float2 _15771;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _14949 = _9684;
                                    _14949.y = 1.0 - _9360.y;
                                    _15771 = _14949;
                                }
                                else
                                {
                                    _15771 = _9684;
                                }
                                float2 _15773;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _14956 = _9360;
                                    _14956.y = 1.0 - _9360.y;
                                    _15773 = _14956;
                                }
                                else
                                {
                                    _15773 = _9360;
                                }
                                _15793 = (((gBackdrop3.sample(gLinear, _15768, level(0.0)) * (_9332.x * _9332.y)) + (gBackdrop3.sample(gLinear, _15769, level(0.0)) * (_9336.x * _9332.y))) + (gBackdrop3.sample(gLinear, _15771, level(0.0)) * (_9332.x * _9336.y))) + (gBackdrop3.sample(gLinear, _15773, level(0.0)) * (_9336.x * _9336.y));
                            }
                            else
                            {
                                float4 _15794;
                                if (_9179 == 4)
                                {
                                    float2 _15761;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _14963 = _9348;
                                        _14963.y = 1.0 - _9348.y;
                                        _15761 = _14963;
                                    }
                                    else
                                    {
                                        _15761 = _9348;
                                    }
                                    float2 _9796 = float2(_9360.x, _9348.y);
                                    float2 _15762;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _14970 = _9796;
                                        _14970.y = 1.0 - _9348.y;
                                        _15762 = _14970;
                                    }
                                    else
                                    {
                                        _15762 = _9796;
                                    }
                                    float2 _9814 = float2(_9348.x, _9360.y);
                                    float2 _15764;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _14977 = _9814;
                                        _14977.y = 1.0 - _9360.y;
                                        _15764 = _14977;
                                    }
                                    else
                                    {
                                        _15764 = _9814;
                                    }
                                    float2 _15766;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _14984 = _9360;
                                        _14984.y = 1.0 - _9360.y;
                                        _15766 = _14984;
                                    }
                                    else
                                    {
                                        _15766 = _9360;
                                    }
                                    _15794 = (((gBackdrop4.sample(gLinear, _15761, level(0.0)) * (_9332.x * _9332.y)) + (gBackdrop4.sample(gLinear, _15762, level(0.0)) * (_9336.x * _9332.y))) + (gBackdrop4.sample(gLinear, _15764, level(0.0)) * (_9332.x * _9336.y))) + (gBackdrop4.sample(gLinear, _15766, level(0.0)) * (_9336.x * _9336.y));
                                }
                                else
                                {
                                    float2 _15754;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _14991 = _9348;
                                        _14991.y = 1.0 - _9348.y;
                                        _15754 = _14991;
                                    }
                                    else
                                    {
                                        _15754 = _9348;
                                    }
                                    float2 _9926 = float2(_9360.x, _9348.y);
                                    float2 _15755;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _14998 = _9926;
                                        _14998.y = 1.0 - _9348.y;
                                        _15755 = _14998;
                                    }
                                    else
                                    {
                                        _15755 = _9926;
                                    }
                                    float2 _9944 = float2(_9348.x, _9360.y);
                                    float2 _15757;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15005 = _9944;
                                        _15005.y = 1.0 - _9360.y;
                                        _15757 = _15005;
                                    }
                                    else
                                    {
                                        _15757 = _9944;
                                    }
                                    float2 _15759;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15012 = _9360;
                                        _15012.y = 1.0 - _9360.y;
                                        _15759 = _15012;
                                    }
                                    else
                                    {
                                        _15759 = _9360;
                                    }
                                    _15794 = (((gBackdrop5.sample(gLinear, _15754, level(0.0)) * (_9332.x * _9332.y)) + (gBackdrop5.sample(gLinear, _15755, level(0.0)) * (_9336.x * _9332.y))) + (gBackdrop5.sample(gLinear, _15757, level(0.0)) * (_9332.x * _9336.y))) + (gBackdrop5.sample(gLinear, _15759, level(0.0)) * (_9336.x * _9336.y));
                                }
                                _15793 = _15794;
                            }
                            _15792 = _15793;
                        }
                        _15791 = _15792;
                    }
                    _15790 = _15791;
                }
                _9193 = _15823 + (_15790.xyz * (_9164 ? ((_15753 == 0) ? (1.0 - _9159) : _9159) : 1.0));
            }
            float3 _16213;
            if (((gFxData_1._data[_6523].w > 0.001000000047497451305389404296875) && (_6640 > 0.0)) && ((((gFxData_1._data[_6523].y * 0.300000011920928955078125) * gFxData_1._data[_6523].w) * _184.gDisplay.z) > (_6612 * 0.3499999940395355224609375)))
            {
                float _6740 = 0.300000011920928955078125 * gFxData_1._data[_6523].w;
                float2 _6747 = (_6698 + _6710) - (_6713 * _6740);
                float _10040 = fast::clamp(log2(fast::max(_6612, 1.0)) - 1.0, 0.0, 5.0);
                int _10043 = int(floor(_10040));
                float _10047 = _10040 - float(_10043);
                bool _10052 = (_10047 > 0.0199999995529651641845703125) && (_10043 < 5);
                float3 _15919;
                _15919 = float3(0.0);
                float3 _10081;
                for (int _15849 = 0; _15849 < 2; _15919 = _10081, _15849++)
                {
                    if ((_15849 > 0) && (!_10052))
                    {
                        break;
                    }
                    int _10067 = _10043 + _15849;
                    int _10105 = clamp(_10067, 1, 5);
                    float2 _10174 = (_6747 * _184.gLevel[_10105].xy) - float2(0.5);
                    float2 _10176 = floor(_10174);
                    float2 _10179 = _10174 - _10176;
                    float2 _10182 = _10179 * _10179;
                    float2 _10185 = _10182 * _10179;
                    float2 _10204 = (((_10185 * 3.0) - (_10182 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                    float2 _10217 = _10185 * 0.16666667163372039794921875;
                    float2 _10220 = (((((-_10185) + (_10182 * 3.0)) - (_10179 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10204;
                    float2 _10224 = (((((_10185 * (-3.0)) + (_10182 * 3.0)) + (_10179 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10217;
                    float2 _10236 = ((_10176 - float2(0.5)) + (_10204 / _10220)) * _184.gLevel[_10105].zw;
                    float2 _10248 = ((_10176 + float2(1.5)) + (_10217 / _10224)) * _184.gLevel[_10105].zw;
                    float4 _15886;
                    if (_10067 <= 0)
                    {
                        float2 _15885;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _15017 = _6747;
                            _15017.y = 1.0 - _6747.y;
                            _15885 = _15017;
                        }
                        else
                        {
                            _15885 = _6747;
                        }
                        _15886 = gBackdrop0.sample(gLinear, _15885, level(0.0));
                    }
                    else
                    {
                        float4 _15887;
                        if (_10067 == 1)
                        {
                            float2 _15878;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15022 = _10236;
                                _15022.y = 1.0 - _10236.y;
                                _15878 = _15022;
                            }
                            else
                            {
                                _15878 = _10236;
                            }
                            float _10293 = _10236.y;
                            float2 _10294 = float2(_10248.x, _10293);
                            float2 _15879;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15029 = _10294;
                                _15029.y = 1.0 - _10293;
                                _15879 = _15029;
                            }
                            else
                            {
                                _15879 = _10294;
                            }
                            float _10311 = _10248.y;
                            float2 _10312 = float2(_10236.x, _10311);
                            float2 _15881;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15036 = _10312;
                                _15036.y = 1.0 - _10311;
                                _15881 = _15036;
                            }
                            else
                            {
                                _15881 = _10312;
                            }
                            float2 _15883;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15043 = _10248;
                                _15043.y = 1.0 - _10248.y;
                                _15883 = _15043;
                            }
                            else
                            {
                                _15883 = _10248;
                            }
                            _15887 = (((gBackdrop1.sample(gLinear, _15878, level(0.0)) * (_10220.x * _10220.y)) + (gBackdrop1.sample(gLinear, _15879, level(0.0)) * (_10224.x * _10220.y))) + (gBackdrop1.sample(gLinear, _15881, level(0.0)) * (_10220.x * _10224.y))) + (gBackdrop1.sample(gLinear, _15883, level(0.0)) * (_10224.x * _10224.y));
                        }
                        else
                        {
                            float4 _15888;
                            if (_10067 == 2)
                            {
                                float2 _15871;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15050 = _10236;
                                    _15050.y = 1.0 - _10236.y;
                                    _15871 = _15050;
                                }
                                else
                                {
                                    _15871 = _10236;
                                }
                                float _10423 = _10236.y;
                                float2 _10424 = float2(_10248.x, _10423);
                                float2 _15872;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15057 = _10424;
                                    _15057.y = 1.0 - _10423;
                                    _15872 = _15057;
                                }
                                else
                                {
                                    _15872 = _10424;
                                }
                                float _10441 = _10248.y;
                                float2 _10442 = float2(_10236.x, _10441);
                                float2 _15874;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15064 = _10442;
                                    _15064.y = 1.0 - _10441;
                                    _15874 = _15064;
                                }
                                else
                                {
                                    _15874 = _10442;
                                }
                                float2 _15876;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15071 = _10248;
                                    _15071.y = 1.0 - _10248.y;
                                    _15876 = _15071;
                                }
                                else
                                {
                                    _15876 = _10248;
                                }
                                _15888 = (((gBackdrop2.sample(gLinear, _15871, level(0.0)) * (_10220.x * _10220.y)) + (gBackdrop2.sample(gLinear, _15872, level(0.0)) * (_10224.x * _10220.y))) + (gBackdrop2.sample(gLinear, _15874, level(0.0)) * (_10220.x * _10224.y))) + (gBackdrop2.sample(gLinear, _15876, level(0.0)) * (_10224.x * _10224.y));
                            }
                            else
                            {
                                float4 _15889;
                                if (_10067 == 3)
                                {
                                    float2 _15864;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15078 = _10236;
                                        _15078.y = 1.0 - _10236.y;
                                        _15864 = _15078;
                                    }
                                    else
                                    {
                                        _15864 = _10236;
                                    }
                                    float _10553 = _10236.y;
                                    float2 _10554 = float2(_10248.x, _10553);
                                    float2 _15865;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15085 = _10554;
                                        _15085.y = 1.0 - _10553;
                                        _15865 = _15085;
                                    }
                                    else
                                    {
                                        _15865 = _10554;
                                    }
                                    float _10571 = _10248.y;
                                    float2 _10572 = float2(_10236.x, _10571);
                                    float2 _15867;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15092 = _10572;
                                        _15092.y = 1.0 - _10571;
                                        _15867 = _15092;
                                    }
                                    else
                                    {
                                        _15867 = _10572;
                                    }
                                    float2 _15869;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15099 = _10248;
                                        _15099.y = 1.0 - _10248.y;
                                        _15869 = _15099;
                                    }
                                    else
                                    {
                                        _15869 = _10248;
                                    }
                                    _15889 = (((gBackdrop3.sample(gLinear, _15864, level(0.0)) * (_10220.x * _10220.y)) + (gBackdrop3.sample(gLinear, _15865, level(0.0)) * (_10224.x * _10220.y))) + (gBackdrop3.sample(gLinear, _15867, level(0.0)) * (_10220.x * _10224.y))) + (gBackdrop3.sample(gLinear, _15869, level(0.0)) * (_10224.x * _10224.y));
                                }
                                else
                                {
                                    float4 _15890;
                                    if (_10067 == 4)
                                    {
                                        float2 _15857;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15106 = _10236;
                                            _15106.y = 1.0 - _10236.y;
                                            _15857 = _15106;
                                        }
                                        else
                                        {
                                            _15857 = _10236;
                                        }
                                        float _10683 = _10236.y;
                                        float2 _10684 = float2(_10248.x, _10683);
                                        float2 _15858;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15113 = _10684;
                                            _15113.y = 1.0 - _10683;
                                            _15858 = _15113;
                                        }
                                        else
                                        {
                                            _15858 = _10684;
                                        }
                                        float _10701 = _10248.y;
                                        float2 _10702 = float2(_10236.x, _10701);
                                        float2 _15860;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15120 = _10702;
                                            _15120.y = 1.0 - _10701;
                                            _15860 = _15120;
                                        }
                                        else
                                        {
                                            _15860 = _10702;
                                        }
                                        float2 _15862;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15127 = _10248;
                                            _15127.y = 1.0 - _10248.y;
                                            _15862 = _15127;
                                        }
                                        else
                                        {
                                            _15862 = _10248;
                                        }
                                        _15890 = (((gBackdrop4.sample(gLinear, _15857, level(0.0)) * (_10220.x * _10220.y)) + (gBackdrop4.sample(gLinear, _15858, level(0.0)) * (_10224.x * _10220.y))) + (gBackdrop4.sample(gLinear, _15860, level(0.0)) * (_10220.x * _10224.y))) + (gBackdrop4.sample(gLinear, _15862, level(0.0)) * (_10224.x * _10224.y));
                                    }
                                    else
                                    {
                                        float2 _15850;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15134 = _10236;
                                            _15134.y = 1.0 - _10236.y;
                                            _15850 = _15134;
                                        }
                                        else
                                        {
                                            _15850 = _10236;
                                        }
                                        float _10813 = _10236.y;
                                        float2 _10814 = float2(_10248.x, _10813);
                                        float2 _15851;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15141 = _10814;
                                            _15141.y = 1.0 - _10813;
                                            _15851 = _15141;
                                        }
                                        else
                                        {
                                            _15851 = _10814;
                                        }
                                        float _10831 = _10248.y;
                                        float2 _10832 = float2(_10236.x, _10831);
                                        float2 _15853;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15148 = _10832;
                                            _15148.y = 1.0 - _10831;
                                            _15853 = _15148;
                                        }
                                        else
                                        {
                                            _15853 = _10832;
                                        }
                                        float2 _15855;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15155 = _10248;
                                            _15155.y = 1.0 - _10248.y;
                                            _15855 = _15155;
                                        }
                                        else
                                        {
                                            _15855 = _10248;
                                        }
                                        _15890 = (((gBackdrop5.sample(gLinear, _15850, level(0.0)) * (_10220.x * _10220.y)) + (gBackdrop5.sample(gLinear, _15851, level(0.0)) * (_10224.x * _10220.y))) + (gBackdrop5.sample(gLinear, _15853, level(0.0)) * (_10220.x * _10224.y))) + (gBackdrop5.sample(gLinear, _15855, level(0.0)) * (_10224.x * _10224.y));
                                    }
                                    _15889 = _15890;
                                }
                                _15888 = _15889;
                            }
                            _15887 = _15888;
                        }
                        _15886 = _15887;
                    }
                    _10081 = _15919 + (_15886.xyz * (_10052 ? ((_15849 == 0) ? (1.0 - _10047) : _10047) : 1.0));
                }
                float3 _15159 = _15823;
                _15159.x = _15919.x;
                float2 _6758 = (_6698 + _6710) + (_6713 * _6740);
                float _10928 = fast::clamp(log2(fast::max(_6612, 1.0)) - 1.0, 0.0, 5.0);
                int _10931 = int(floor(_10928));
                float _10935 = _10928 - float(_10931);
                bool _10940 = (_10935 > 0.0199999995529651641845703125) && (_10931 < 5);
                float3 _16043;
                _16043 = float3(0.0);
                float3 _10969;
                for (int _15973 = 0; _15973 < 2; _16043 = _10969, _15973++)
                {
                    if ((_15973 > 0) && (!_10940))
                    {
                        break;
                    }
                    int _10955 = _10931 + _15973;
                    int _10993 = clamp(_10955, 1, 5);
                    float2 _11062 = (_6758 * _184.gLevel[_10993].xy) - float2(0.5);
                    float2 _11064 = floor(_11062);
                    float2 _11067 = _11062 - _11064;
                    float2 _11070 = _11067 * _11067;
                    float2 _11073 = _11070 * _11067;
                    float2 _11092 = (((_11073 * 3.0) - (_11070 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                    float2 _11105 = _11073 * 0.16666667163372039794921875;
                    float2 _11108 = (((((-_11073) + (_11070 * 3.0)) - (_11067 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11092;
                    float2 _11112 = (((((_11073 * (-3.0)) + (_11070 * 3.0)) + (_11067 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11105;
                    float2 _11124 = ((_11064 - float2(0.5)) + (_11092 / _11108)) * _184.gLevel[_10993].zw;
                    float2 _11136 = ((_11064 + float2(1.5)) + (_11105 / _11112)) * _184.gLevel[_10993].zw;
                    float4 _16010;
                    if (_10955 <= 0)
                    {
                        float2 _16009;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _15162 = _6758;
                            _15162.y = 1.0 - _6758.y;
                            _16009 = _15162;
                        }
                        else
                        {
                            _16009 = _6758;
                        }
                        _16010 = gBackdrop0.sample(gLinear, _16009, level(0.0));
                    }
                    else
                    {
                        float4 _16011;
                        if (_10955 == 1)
                        {
                            float2 _16002;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15167 = _11124;
                                _15167.y = 1.0 - _11124.y;
                                _16002 = _15167;
                            }
                            else
                            {
                                _16002 = _11124;
                            }
                            float _11181 = _11124.y;
                            float2 _11182 = float2(_11136.x, _11181);
                            float2 _16003;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15174 = _11182;
                                _15174.y = 1.0 - _11181;
                                _16003 = _15174;
                            }
                            else
                            {
                                _16003 = _11182;
                            }
                            float _11199 = _11136.y;
                            float2 _11200 = float2(_11124.x, _11199);
                            float2 _16005;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15181 = _11200;
                                _15181.y = 1.0 - _11199;
                                _16005 = _15181;
                            }
                            else
                            {
                                _16005 = _11200;
                            }
                            float2 _16007;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15188 = _11136;
                                _15188.y = 1.0 - _11136.y;
                                _16007 = _15188;
                            }
                            else
                            {
                                _16007 = _11136;
                            }
                            _16011 = (((gBackdrop1.sample(gLinear, _16002, level(0.0)) * (_11108.x * _11108.y)) + (gBackdrop1.sample(gLinear, _16003, level(0.0)) * (_11112.x * _11108.y))) + (gBackdrop1.sample(gLinear, _16005, level(0.0)) * (_11108.x * _11112.y))) + (gBackdrop1.sample(gLinear, _16007, level(0.0)) * (_11112.x * _11112.y));
                        }
                        else
                        {
                            float4 _16012;
                            if (_10955 == 2)
                            {
                                float2 _15995;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15195 = _11124;
                                    _15195.y = 1.0 - _11124.y;
                                    _15995 = _15195;
                                }
                                else
                                {
                                    _15995 = _11124;
                                }
                                float _11311 = _11124.y;
                                float2 _11312 = float2(_11136.x, _11311);
                                float2 _15996;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15202 = _11312;
                                    _15202.y = 1.0 - _11311;
                                    _15996 = _15202;
                                }
                                else
                                {
                                    _15996 = _11312;
                                }
                                float _11329 = _11136.y;
                                float2 _11330 = float2(_11124.x, _11329);
                                float2 _15998;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15209 = _11330;
                                    _15209.y = 1.0 - _11329;
                                    _15998 = _15209;
                                }
                                else
                                {
                                    _15998 = _11330;
                                }
                                float2 _16000;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15216 = _11136;
                                    _15216.y = 1.0 - _11136.y;
                                    _16000 = _15216;
                                }
                                else
                                {
                                    _16000 = _11136;
                                }
                                _16012 = (((gBackdrop2.sample(gLinear, _15995, level(0.0)) * (_11108.x * _11108.y)) + (gBackdrop2.sample(gLinear, _15996, level(0.0)) * (_11112.x * _11108.y))) + (gBackdrop2.sample(gLinear, _15998, level(0.0)) * (_11108.x * _11112.y))) + (gBackdrop2.sample(gLinear, _16000, level(0.0)) * (_11112.x * _11112.y));
                            }
                            else
                            {
                                float4 _16013;
                                if (_10955 == 3)
                                {
                                    float2 _15988;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15223 = _11124;
                                        _15223.y = 1.0 - _11124.y;
                                        _15988 = _15223;
                                    }
                                    else
                                    {
                                        _15988 = _11124;
                                    }
                                    float _11441 = _11124.y;
                                    float2 _11442 = float2(_11136.x, _11441);
                                    float2 _15989;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15230 = _11442;
                                        _15230.y = 1.0 - _11441;
                                        _15989 = _15230;
                                    }
                                    else
                                    {
                                        _15989 = _11442;
                                    }
                                    float _11459 = _11136.y;
                                    float2 _11460 = float2(_11124.x, _11459);
                                    float2 _15991;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15237 = _11460;
                                        _15237.y = 1.0 - _11459;
                                        _15991 = _15237;
                                    }
                                    else
                                    {
                                        _15991 = _11460;
                                    }
                                    float2 _15993;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15244 = _11136;
                                        _15244.y = 1.0 - _11136.y;
                                        _15993 = _15244;
                                    }
                                    else
                                    {
                                        _15993 = _11136;
                                    }
                                    _16013 = (((gBackdrop3.sample(gLinear, _15988, level(0.0)) * (_11108.x * _11108.y)) + (gBackdrop3.sample(gLinear, _15989, level(0.0)) * (_11112.x * _11108.y))) + (gBackdrop3.sample(gLinear, _15991, level(0.0)) * (_11108.x * _11112.y))) + (gBackdrop3.sample(gLinear, _15993, level(0.0)) * (_11112.x * _11112.y));
                                }
                                else
                                {
                                    float4 _16014;
                                    if (_10955 == 4)
                                    {
                                        float2 _15981;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15251 = _11124;
                                            _15251.y = 1.0 - _11124.y;
                                            _15981 = _15251;
                                        }
                                        else
                                        {
                                            _15981 = _11124;
                                        }
                                        float _11571 = _11124.y;
                                        float2 _11572 = float2(_11136.x, _11571);
                                        float2 _15982;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15258 = _11572;
                                            _15258.y = 1.0 - _11571;
                                            _15982 = _15258;
                                        }
                                        else
                                        {
                                            _15982 = _11572;
                                        }
                                        float _11589 = _11136.y;
                                        float2 _11590 = float2(_11124.x, _11589);
                                        float2 _15984;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15265 = _11590;
                                            _15265.y = 1.0 - _11589;
                                            _15984 = _15265;
                                        }
                                        else
                                        {
                                            _15984 = _11590;
                                        }
                                        float2 _15986;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15272 = _11136;
                                            _15272.y = 1.0 - _11136.y;
                                            _15986 = _15272;
                                        }
                                        else
                                        {
                                            _15986 = _11136;
                                        }
                                        _16014 = (((gBackdrop4.sample(gLinear, _15981, level(0.0)) * (_11108.x * _11108.y)) + (gBackdrop4.sample(gLinear, _15982, level(0.0)) * (_11112.x * _11108.y))) + (gBackdrop4.sample(gLinear, _15984, level(0.0)) * (_11108.x * _11112.y))) + (gBackdrop4.sample(gLinear, _15986, level(0.0)) * (_11112.x * _11112.y));
                                    }
                                    else
                                    {
                                        float2 _15974;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15279 = _11124;
                                            _15279.y = 1.0 - _11124.y;
                                            _15974 = _15279;
                                        }
                                        else
                                        {
                                            _15974 = _11124;
                                        }
                                        float _11701 = _11124.y;
                                        float2 _11702 = float2(_11136.x, _11701);
                                        float2 _15975;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15286 = _11702;
                                            _15286.y = 1.0 - _11701;
                                            _15975 = _15286;
                                        }
                                        else
                                        {
                                            _15975 = _11702;
                                        }
                                        float _11719 = _11136.y;
                                        float2 _11720 = float2(_11124.x, _11719);
                                        float2 _15977;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15293 = _11720;
                                            _15293.y = 1.0 - _11719;
                                            _15977 = _15293;
                                        }
                                        else
                                        {
                                            _15977 = _11720;
                                        }
                                        float2 _15979;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15300 = _11136;
                                            _15300.y = 1.0 - _11136.y;
                                            _15979 = _15300;
                                        }
                                        else
                                        {
                                            _15979 = _11136;
                                        }
                                        _16014 = (((gBackdrop5.sample(gLinear, _15974, level(0.0)) * (_11108.x * _11108.y)) + (gBackdrop5.sample(gLinear, _15975, level(0.0)) * (_11112.x * _11108.y))) + (gBackdrop5.sample(gLinear, _15977, level(0.0)) * (_11108.x * _11112.y))) + (gBackdrop5.sample(gLinear, _15979, level(0.0)) * (_11112.x * _11112.y));
                                    }
                                    _16013 = _16014;
                                }
                                _16012 = _16013;
                            }
                            _16011 = _16012;
                        }
                        _16010 = _16011;
                    }
                    _10969 = _16043 + (_16010.xyz * (_10940 ? ((_15973 == 0) ? (1.0 - _10935) : _10935) : 1.0));
                }
                _15159.z = _16043.z;
                _16213 = _15159;
            }
            else
            {
                _16213 = _15823;
            }
            float3 _16433;
            if (_6640 > 0.0)
            {
                float2 _6781 = _6698 + (((_15751 * fast::min(_6631 * 0.5, 16.0)) * _184.gDisplay.zw) * _184.gTarget.zw);
                float _11816 = fast::clamp(log2(fast::max(16.0 * _184.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _11819 = int(floor(_11816));
                float _11823 = _11816 - float(_11819);
                bool _11828 = (_11823 > 0.0199999995529651641845703125) && (_11819 < 5);
                float3 _16198;
                _16198 = float3(0.0);
                float3 _11857;
                for (int _16183 = 0; _16183 < 2; _16198 = _11857, _16183++)
                {
                    if ((_16183 > 0) && (!_11828))
                    {
                        break;
                    }
                    int _11843 = _11819 + _16183;
                    float2 _16184;
                    if (_184.gConv.x > 0.5)
                    {
                        float2 _15307 = _6781;
                        _15307.y = 1.0 - _6781.y;
                        _16184 = _15307;
                    }
                    else
                    {
                        _16184 = _6781;
                    }
                    float4 _16185;
                    if (_11843 <= 0)
                    {
                        _16185 = gBackdrop0.sample(gLinear, _16184, level(0.0));
                    }
                    else
                    {
                        float4 _16186;
                        if (_11843 == 1)
                        {
                            _16186 = gBackdrop1.sample(gLinear, _16184, level(0.0));
                        }
                        else
                        {
                            float4 _16187;
                            if (_11843 == 2)
                            {
                                _16187 = gBackdrop2.sample(gLinear, _16184, level(0.0));
                            }
                            else
                            {
                                float4 _16188;
                                if (_11843 == 3)
                                {
                                    _16188 = gBackdrop3.sample(gLinear, _16184, level(0.0));
                                }
                                else
                                {
                                    float4 _16189;
                                    if (_11843 == 4)
                                    {
                                        _16189 = gBackdrop4.sample(gLinear, _16184, level(0.0));
                                    }
                                    else
                                    {
                                        _16189 = gBackdrop5.sample(gLinear, _16184, level(0.0));
                                    }
                                    _16188 = _16189;
                                }
                                _16187 = _16188;
                            }
                            _16186 = _16187;
                        }
                        _16185 = _16186;
                    }
                    _11857 = _16198 + (_16185.xyz * (_11828 ? ((_16183 == 0) ? (1.0 - _11823) : _11823) : 1.0));
                }
                _16433 = _16198;
            }
            else
            {
                _16433 = float3(0.5);
            }
            float _16229;
            if (in.i_shape.x > 0.001000000047497451305389404296875)
            {
                int _11946 = clamp(int(rint(log2(36.0 * _184.gDisplay.z) - 1.0)), 1, 4);
                float2 _16204;
                if (_184.gConv.x > 0.5)
                {
                    float2 _15311 = _6698;
                    _15311.y = 1.0 - _6698.y;
                    _16204 = _15311;
                }
                else
                {
                    _16204 = _6698;
                }
                float4 _16205;
                if (_11946 <= 0)
                {
                    _16205 = gBackdrop0.sample(gLinear, _16204, level(0.0));
                }
                else
                {
                    float4 _16206;
                    if (_11946 == 1)
                    {
                        _16206 = gBackdrop1.sample(gLinear, _16204, level(0.0));
                    }
                    else
                    {
                        float4 _16207;
                        if (_11946 == 2)
                        {
                            _16207 = gBackdrop2.sample(gLinear, _16204, level(0.0));
                        }
                        else
                        {
                            float4 _16208;
                            if (_11946 == 3)
                            {
                                _16208 = gBackdrop3.sample(gLinear, _16204, level(0.0));
                            }
                            else
                            {
                                float4 _16209;
                                if (_11946 == 4)
                                {
                                    _16209 = gBackdrop4.sample(gLinear, _16204, level(0.0));
                                }
                                else
                                {
                                    _16209 = gBackdrop5.sample(gLinear, _16204, level(0.0));
                                }
                                _16208 = _16209;
                            }
                            _16207 = _16208;
                        }
                        _16206 = _16207;
                    }
                    _16205 = _16206;
                }
                _16229 = dot(_16205.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _16229 = 0.5;
            }
            _16431 = _16433;
            _16228 = _16229;
            _16210 = _16213;
        }
        else
        {
            _16431 = float3(0.5);
            _16228 = 0.5;
            _16210 = float3(0.5);
        }
        float3 _6809 = mix(float3(dot(_16210, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _16210, float3(gFxData_1._data[_6539].x)) + float3(gFxData_1._data[_6539].y);
        float3 _16324;
        if (in.i_shape.x > 0.001000000047497451305389404296875)
        {
            float _6819 = fast::clamp(fast::max(_16228 + gFxData_1._data[_6539].y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _6827 = mix(_6819, dot(gFxData_1._data[_6531].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), in.i_shape.x);
            _16324 = select(mix(_6809, float3(1.0), float3((_6827 - _6819) / fast::max(1.0 - _6819, 0.001000000047497451305389404296875))), _6809 * (_6827 / _6819), bool3(_6827 < _6819));
        }
        else
        {
            _16324 = _6809;
        }
        float2 _6865 = fast::clamp((in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875)), float2(0.0), float2(1.0));
        float2 _6913 = float2(cos(gFxData_1._data[_6539].w), sin(gFxData_1._data[_6539].w));
        float _6916 = dot(_15751, _6913);
        float _6934 = _3975 * (abs(_15751.x) + abs(_15751.y));
        float _16544;
        _16544 = 0.0;
        for (int _16543 = -2; _16543 <= 2; )
        {
            float _6951 = (-_15564) + ((float(_16543) * 0.4000000059604644775390625) * _6934);
            float _12047 = fast::clamp(1.0 - (_6951 / _6631), 0.0, 1.0);
            float _12055 = 1.0 - fast::max(sqrt(fast::clamp(1.0 - (_12047 * _12047), 0.0, 1.0)), 0.12399999797344207763671875);
            float _12058 = _12055 * _12055;
            _16544 += ((3.0 - abs(float(_16543))) * ((_6951 < 0.0) ? 0.0 : ((_12058 * _12058) * _12055)));
            _16543++;
            continue;
        }
        float _6987 = fast::clamp(0.5 + (0.5 * dot((in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5)) / _6623, _6913)), 0.0, 1.0);
        _17456 = (((_16544 * 0.111111111938953399658203125) * (pow(fast::clamp(_6916, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(fast::clamp(-_6916, 0.0, 1.0), 1.5)))) * gFxData_1._data[_6539].z) * 1.60000002384185791015625;
        _17075 = gFxData_1._data[_6547];
        _17062 = float4((mix(mix(_16324, gFxData_1._data[_6531].xyz, float3(fast::clamp(gFxData_1._data[_6531].w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _6646))) + (0.3499999940395355224609375 * ((dot(gFxData_1._data[_6531].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _6865.y) : _6865.y))), 0.0, 1.0))), (_16431 * 1.10000002384185791015625) + float3(0.07999999821186065673828125), float3(pow(1.0 - _16328.z, 5.0) * 0.3499999940395355224609375)) + float3((((0.039999999105930328369140625 * _6987) * _6987) + (0.0500000007450580596923828125 * (1.0 - _6646))) * gFxData_1._data[_6539].z)) * _4000, _4000) + (_16551 * (1.0 - _4000));
    }
    else
    {
        _17456 = 0.0;
        _17075 = _15544;
        _17062 = _16551;
    }
    float4 _17072;
    if ((in.i_flags.x & 1u) != 0u)
    {
        float4 _16693;
        float4 _16871;
        if (in.i_flags.y != 0u)
        {
            _16871 = gFxData_1._data[(in.i_instance * 24u) + 3u];
            _16693 = gFxData_1._data[(in.i_instance * 24u) + 4u];
        }
        else
        {
            _16871 = float4(0.0);
            _16693 = float4(0.0);
        }
        float4 _17057;
        do
        {
            if (in.i_flags.y == 0u)
            {
                _17057 = in.i_fill0;
                break;
            }
            float2 _12129 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float2 _12139 = fwidth(in.i_local);
            float _12141 = fast::max(length(_12139), 9.9999997473787516355514526367188e-05);
            float _17049;
            float _17053;
            if ((in.i_flags.y == 1u) || (in.i_flags.y == 4u))
            {
                float _12150 = cos(_16693.x);
                float _12153 = sin(_16693.x);
                float _12179 = ((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_12150, _12153)) / fast::max(0.5 * ((abs(_12150) * _12129.x) + (abs(_12153) * _12129.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (in.i_flags.y == 4u)
                {
                    float _12194 = fast::clamp((_12179 - _16693.y) / fast::max(_16693.z - _16693.y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _17057 = float4(fast::clamp(abs((fract(float3(_12194 * 0.800000011920928955078125) + float3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - float3(3.0)) - float3(1.0), float3(0.0), float3(1.0)), in.i_fill0.w * pow(fast::max(sin(_12194 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _17053 = -1.0;
                _17049 = _12179;
            }
            else
            {
                float _17050;
                float _17054;
                if (in.i_flags.y == 2u)
                {
                    _17054 = -1.0;
                    _17050 = length(in.i_local - (in.i_rect.xy + (_16693.xy * _12129))) / fast::max(_16693.z * fast::max(_12129.x, _12129.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    float2 _12262 = in.i_local - (in.i_rect.xy + (_16693.xy * _12129));
                    float _12273 = fract(((precise::atan2(_12262.y, _12262.x) - _16693.z) * 0.15915493667125701904296875) + 1.0);
                    float _17051;
                    float _17055;
                    if (_16693.w > 0.5)
                    {
                        _17055 = -1.0;
                        _17051 = 0.5 - (0.5 * cos(_12273 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _12293 = (((_12273 < 0.5) ? _12273 : (_12273 - 1.0)) * 6.283185482025146484375) * length(_12262);
                        float _17056;
                        if (abs(_12293) < _12141)
                        {
                            _17056 = fast::clamp(((_12293 / _12141) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _17056 = -1.0;
                        }
                        _17055 = _17056;
                        _17051 = _12273;
                    }
                    _17054 = _17055;
                    _17050 = _17051;
                }
                _17053 = _17054;
                _17049 = _17050;
            }
            float4 _12322 = float4(in.i_fill0.xyz * in.i_fill0.w, in.i_fill0.w);
            float4 _12334 = float4(_16871.xyz * _16871.w, _16871.w);
            float4 _12348 = select(mix(_12322, _12334, float4(fast::clamp(_17049, 0.0, 1.0))), mix(_12334, _12322, float4(_17053)), bool4(_17053 >= 0.0));
            _17057 = select(float4(0.0), float4(_12348.xyz / float3(_12348.w), _12348.w), bool4(_12348.w > 9.9999997473787516355514526367188e-06));
            break;
        } while(false);
        float4 _17058;
        if ((in.i_flags.x & 64u) != 0u)
        {
            uint _12373 = (in.i_instance * 24u) + 18u;
            _17058 = _17057 * gTex.sample(gLinear, mix(gFxData_1._data[_12373].xy, gFxData_1._data[_12373].zw, (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875))));
        }
        else
        {
            _17058 = _17057;
        }
        float _12383 = fast::clamp(_17058.w * _4000, 0.0, 1.0);
        _17072 = float4(_17058.xyz * _12383, _12383) + (_17062 * (1.0 - _12383));
    }
    else
    {
        _17072 = _17062;
    }
    float4 _17442;
    if ((in.i_flags.x & 16384u) != 0u)
    {
        uint _12407 = (in.i_instance * 24u) + 21u;
        uint _12415 = (in.i_instance * 24u) + 3u;
        float2 _4440 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float2 _4447 = (in.i_local - in.i_rect.xy) / _4440;
        float _4458 = ((gFxData_1._data[_12415].x >= 0.0) ? gFxData_1._data[_12415].x : _184.gTime.x) * gFxData_1._data[_12407].z;
        float _4461 = _4447.x * 2.0;
        float _4462 = _4461 - 1.0;
        float _4467 = _4447.y * _4440.y;
        float _4470 = fast::max(gFxData_1._data[_12407].w, 0.001000000047497451305389404296875);
        float _4475 = fast::clamp(1.0 - (_4462 * _4462), 0.0, 1.0);
        float _4477 = pow(_4475, 1.2999999523162841796875);
        float _4479 = pow(_4475, 0.699999988079071044921875);
        float _4482 = fast::clamp(_4458 * 1.4285714626312255859375, 0.0, 1.0);
        float _4498 = (0.25 + (0.75 * ((_4482 * _4482) * (3.0 - (2.0 * _4482))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_4458 * 2.099999904632568359375)));
        float _4507 = ((gFxData_1._data[_12407].y * _4440.y) * _4498) * _4477;
        float _4533 = (_4440.y * (0.5 + ((gFxData_1._data[_12407].x * (0.5 - (_4462 * _4462))) * 0.5))) + (((0.14000000059604644775390625 * _4440.y) * _4477) * sin(((_4462 * 2.400000095367431640625) - (_4458 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _17068;
        float _17069;
        float3 _17070;
        _17070 = float3(0.0);
        _17069 = _4533;
        _17068 = _4533;
        float3 _4605;
        float _17704;
        float _17705;
        for (int _17067 = 0; _17067 < 4; _17070 = _4605, _17069 = _17705, _17068 = _17704, _17067++)
        {
            float _4557 = _4533 + ((_4507 * _3006[_17067].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_4458 * 1.7000000476837158203125) + _3023[_17067].y))));
            _17704 = (_17067 == 0) ? _4557 : _17068;
            _17705 = (_17067 == 2) ? _4557 : _17069;
            float _4572 = _4470 * _3006[_17067].y;
            float _4577 = (_4467 - _4557) / _4572;
            float _4582 = _4470 * _3006[_17067].z;
            float3 _17678;
            _17678 = float3(0.0);
            for (int _17677 = 0; _17677 < 6; )
            {
                float _12440 = ((_4467 - _4557) - (_4582 * ((float(_17677) * 0.4000000059604644775390625) - 1.0))) / _4572;
                _17678 += (_1724[_17677] * exp((-_12440) * _12440));
                _17677++;
                continue;
            }
            _4605 = _17070 + (mix(_17678 * float3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), float3(exp((-_4577) * _4577)), float3(_3023[_17067].x)) * (_3006[_17067].w * _4479));
        }
        float _4611 = _4470 * 1.5;
        float _4641 = fast::clamp((_4467 - _17068) / fast::max(_17069 - _17068, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _4669 = (_4467 - (_17069 - (_4470 * 3.0))) / (((_4440.y * 0.0900000035762786865234375) + (_4507 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _4672 = (_4461 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _4697 = ((_4467 - _17068) + (_4470 * 5.0)) / (_4470 * 7.0);
        float3 _4723 = float3(1.0) - exp((-((((_17070 + (mix(float3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), float3(1.0), float3(_4641)) * ((((1.0 / (1.0 + exp((-((_4467 - _17068) - (_4470 * 2.0))) / _4611))) / (1.0 + exp((-(_17069 - _4467)) / _4611))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_4641, 2.5)))) * _4479))) + (float3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_4669) * _4669) - (_4672 * _4672)) * (0.5 + (1.10000002384185791015625 * _4498))))) + (float3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_4697) * _4697) * _4477) * 0.100000001490116119384765625))) * mix(float3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), float3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), float3(0.5 + (0.5 * sin(_4458 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _4733 = (in.i_fill0.w * smoothstep(0.0, 0.119999997317790985107421875, _4447.y)) * smoothstep(1.0, 0.87999999523162841796875, _4447.y);
        float _4752 = (fast::clamp(fast::max(_4723.x, fast::max(_4723.y, _4723.z)), 0.0, 1.0) * _4733) * _4000;
        _17442 = float4((_4723 * _4733) * _4000, _4752) + (_17072 * (1.0 - _4752));
    }
    else
    {
        _17442 = _17072;
    }
    float4 _17451;
    if (((in.i_flags.x & 4u) != 0u) && ((in.i_flags.x & 256u) != 0u))
    {
        uint _12472 = (in.i_instance * 24u) + 7u;
        uint _12480 = (in.i_instance * 24u) + 8u;
        float2 _4786 = in.i_local - gFxData_1._data[_12480].zw;
        float2 _12521 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _12530 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _17417;
        if (in.i_flags.z == 1u)
        {
            float2 _12536 = _4786 - _12521;
            float _17416;
            do
            {
                if (in.i_shape.w >= 6.282185077667236328125)
                {
                    _17416 = abs(length(_12536) - in.i_radii.x) - in.i_radii.y;
                    break;
                }
                float _12635 = in.i_shape.z + (in.i_shape.w * 0.5);
                float _12637 = cos(_12635);
                float _12639 = sin(_12635);
                float _12648 = dot(_12536, float2(-_12639, _12637));
                float _12651 = dot(_12536, float2(_12637, _12639));
                float2 _12652 = float2(_12648, _12651);
                float _12655 = abs(_12648);
                _12652.x = _12655;
                float _12658 = in.i_shape.w * 0.5;
                float _12660 = sin(_12658);
                float _12662 = cos(_12658);
                _17416 = (((_12662 * _12655) > (_12660 * _12651)) ? length(_12652 - (float2(_12660, _12662) * in.i_radii.x)) : abs(length(_12652) - in.i_radii.x)) - in.i_radii.y;
                break;
            } while(false);
            _17417 = _17416;
        }
        else
        {
            float _17418;
            if (in.i_flags.z == 2u)
            {
                float2 _12698 = _4786 - _15546.xy;
                float2 _12701 = _15546.zw - _15546.xy;
                _17418 = length(_12698 - (_12701 * fast::clamp(dot(_12698, _12701) / fast::max(dot(_12701, _12701), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
            }
            else
            {
                float2 _12563 = _4786 - _12521;
                float _12754 = fast::min(_12530.x, _12530.y);
                float _12757 = fast::min((_12563.x > 0.0) ? ((_12563.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_12563.y > 0.0) ? in.i_radii.w : in.i_radii.x), _12754);
                float _12763 = _12757 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                float _17401;
                float _17402;
                if (_12763 > _12754)
                {
                    float _12777 = in.i_shape.y * fast::clamp((_12754 - _12757) / fast::max(0.60000002384185791015625 * _12757, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _17402 = _12777;
                    _17401 = _12757 * (1.0 + (0.60000002384185791015625 * _12777));
                }
                else
                {
                    _17402 = in.i_shape.y;
                    _17401 = _12763;
                }
                float2 _12790 = (abs(_12563) - _12530) + float2(_17401);
                float2 _12792 = fast::max(_12790, float2(0.0));
                float _17403;
                if ((_12792.x > 0.0) && (_12792.y > 0.0))
                {
                    float _17404;
                    if ((_17402 > 0.001000000047497451305389404296875) && (_17401 > 9.9999997473787516355514526367188e-05))
                    {
                        float _12809 = 2.0 + (2.0 * _17402);
                        float2 _12814 = _12792 / float2(fast::max(_17401, 9.9999997473787516355514526367188e-05));
                        _17404 = pow(pow(_12814.x, _12809) + pow(_12814.y, _12809), 1.0 / _12809) * _17401;
                    }
                    else
                    {
                        _17404 = length(_12792);
                    }
                    _17403 = _17404;
                }
                else
                {
                    _17403 = fast::max(_12792.x, _12792.y);
                }
                float _12849 = (fast::min(fast::max(_12790.x, _12790.y), 0.0) + _17403) - _17401;
                float _17419;
                if ((in.i_flags.x & 512u) != 0u)
                {
                    float2 _12591 = fast::max((_15546.zw - _15546.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _12594 = _4786 - ((_15546.xy + _15546.zw) * 0.5);
                    float _12885 = fast::min(_12591.x, _12591.y);
                    float _12888 = fast::min((_12594.x > 0.0) ? ((_12594.y > 0.0) ? _17075.x : _17075.x) : ((_12594.y > 0.0) ? _17075.x : _17075.x), _12885);
                    float _12894 = _12888 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _17407;
                    float _17408;
                    if (_12894 > _12885)
                    {
                        float _12908 = in.i_shape.y * fast::clamp((_12885 - _12888) / fast::max(0.60000002384185791015625 * _12888, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _17408 = _12908;
                        _17407 = _12888 * (1.0 + (0.60000002384185791015625 * _12908));
                    }
                    else
                    {
                        _17408 = in.i_shape.y;
                        _17407 = _12894;
                    }
                    float2 _12921 = (abs(_12594) - _12591) + float2(_17407);
                    float2 _12923 = fast::max(_12921, float2(0.0));
                    float _17409;
                    if ((_12923.x > 0.0) && (_12923.y > 0.0))
                    {
                        float _17410;
                        if ((_17408 > 0.001000000047497451305389404296875) && (_17407 > 9.9999997473787516355514526367188e-05))
                        {
                            float _12940 = 2.0 + (2.0 * _17408);
                            float2 _12945 = _12923 / float2(fast::max(_17407, 9.9999997473787516355514526367188e-05));
                            _17410 = pow(pow(_12945.x, _12940) + pow(_12945.y, _12940), 1.0 / _12940) * _17407;
                        }
                        else
                        {
                            _17410 = length(_12923);
                        }
                        _17409 = _17410;
                    }
                    else
                    {
                        _17409 = fast::max(_12923.x, _12923.y);
                    }
                    float _12980 = (fast::min(fast::max(_12921.x, _12921.y), 0.0) + _17409) - _17407;
                    float _12985 = fast::max(_17075.y, 9.9999997473787516355514526367188e-05);
                    float _12994 = fast::max(_12985 - abs(_12849 - _12980), 0.0) / _12985;
                    _17419 = fast::min(_12849, _12980) - (((_12994 * _12994) * _12985) * 0.25);
                }
                else
                {
                    _17419 = _12849;
                }
                _17418 = _17419;
            }
            _17417 = _17418;
        }
        float _4795 = (_17417 + gFxData_1._data[_12480].y) / (fast::max(gFxData_1._data[_12480].x * 0.5, _3975 * 0.5) * 1.41421353816986083984375);
        float _13011 = sign(_4795);
        float _13013 = abs(_4795);
        float _13024 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_13013 * _13013))) * _13013)) * _13013);
        float _13027 = _13024 * _13024;
        float _13042 = fast::clamp(gFxData_1._data[_12472].w * ((0.5 + (0.5 * (_13011 - (_13011 / (_13027 * _13027))))) * _4000), 0.0, 1.0);
        _17451 = float4(gFxData_1._data[_12472].xyz * _13042, _13042) + (_17442 * (1.0 - _13042));
    }
    else
    {
        _17451 = _17442;
    }
    float4 _17475;
    if ((in.i_flags.x & 16u) != 0u)
    {
        uint _13066 = (in.i_instance * 24u) + 9u;
        uint _13074 = (in.i_instance * 24u) + 10u;
        float _4827 = fast::max(-_15564, 0.0) / fast::max(gFxData_1._data[_13074].z, 0.001000000047497451305389404296875);
        float _13084 = fast::clamp(gFxData_1._data[_13066].w * fast::clamp((exp(((-_4827) * _4827) * 2.2000000476837158203125) * gFxData_1._data[_13074].w) * _4000, 0.0, 1.0), 0.0, 1.0);
        _17475 = float4(gFxData_1._data[_13066].xyz * _13084, _13084) + (_17451 * (1.0 - _13084));
    }
    else
    {
        _17475 = _17451;
    }
    float3 _4858 = _17475.xyz + float3(_17456 * fast::clamp(_17475.w / fast::max(_4000, 0.001000000047497451305389404296875), 0.0, 1.0));
    float4 _15439 = _17475;
    _15439.x = _4858.x;
    _15439.y = _4858.y;
    _15439.z = _4858.z;
    float4 _17478;
    if ((in.i_flags.x & 2u) != 0u)
    {
        uint _13108 = (in.i_instance * 24u) + 5u;
        float4 _13110 = gFxData_1._data[_13108];
        uint _13116 = (in.i_instance * 24u) + 6u;
        float4 _17476;
        if (gFxData_1._data[_13116].z < 0.999000012874603271484375)
        {
            float2 _4916 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float _4927 = cos(gFxData_1._data[_13116].w);
            float _4930 = sin(gFxData_1._data[_13116].w);
            float4 _15456 = _13110;
            _15456.w = _13110.w * mix(1.0, gFxData_1._data[_13116].z, fast::clamp(((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_4927, _4930)) / fast::max(0.5 * ((abs(_4927) * _4916.x) + (abs(_4930) * _4916.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _17476 = _15456;
        }
        else
        {
            _17476 = _13110;
        }
        float _13126 = fast::clamp(_17476.w * (fast::clamp(0.5 - ((_15564 - (gFxData_1._data[_13116].x * gFxData_1._data[_13116].y)) / _3975), 0.0, 1.0) - fast::clamp(0.5 - ((_15564 + (gFxData_1._data[_13116].x * (1.0 - gFxData_1._data[_13116].y))) / _3975), 0.0, 1.0)), 0.0, 1.0);
        _17478 = float4(_17476.xyz * _13126, _13126) + (_15439 * (1.0 - _13126));
    }
    else
    {
        _17478 = _15439;
    }
    float4 _17479;
    if ((in.i_flags.x & 128u) != 0u)
    {
        float2 _4991 = (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float _5013 = exp(-pow((((_4991.x * 0.85000002384185791015625) + (_4991.y * 0.1500000059604644775390625)) - ((fract(_184.gTime.x * in.i_misc.w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _17479 = float4(_17478.xyz + float3(((_5013 * in.i_misc.z) * _4000) * fast::max(_17478.w, 0.3499999940395355224609375)), fast::max(_17478.w, ((_5013 * in.i_misc.z) * _4000) * 0.5));
    }
    else
    {
        _17479 = _17478;
    }
    float4 _17672;
    if ((in.i_flags.x & 2048u) != 0u)
    {
        float3 _13151 = fract(floor(_15543).xyx * 0.103100001811981201171875);
        float3 _13160 = _13151 + float3(dot(_13151, _13151.yzx + float3(33.3300018310546875)));
        float3 _5064 = _17479.xyz + float3(((fract((_13160.x + _13160.y) * _13160.z) - 0.5) * in.i_misc.y) * _17479.w);
        float4 _15480 = _17479;
        _15480.x = _5064.x;
        _15480.y = _5064.y;
        _15480.z = _5064.z;
        _17672 = _15480;
    }
    else
    {
        _17672 = _17479;
    }
    float _17668;
    if (_227.gFade.z > 0.0)
    {
        _17668 = smoothstep(0.0, 1.0, fast::clamp((_15543.y - _227.gFade.x) / _227.gFade.z, 0.0, 1.0));
    }
    else
    {
        _17668 = 1.0;
    }
    float _17669;
    if (_227.gFade.w > 0.0)
    {
        _17669 = _17668 * smoothstep(0.0, 1.0, fast::clamp((_227.gFade.y - _15543.y) / _227.gFade.w, 0.0, 1.0));
    }
    else
    {
        _17669 = _17668;
    }
    float4 _5081 = _17672 * ((in.i_misc.x * _17490) * _17669);
    float4 _17673;
    if (((in.i_flags.x & 8u) != 0u) || ((in.i_flags.x & 4u) != 0u))
    {
        float3 _13212 = fract((floor(_15543) + float2(17.0)).xyx * 0.103100001811981201171875);
        float3 _13221 = _13212 + float3(dot(_13212, _13212.yzx + float3(33.3300018310546875)));
        float3 _5105 = _5081.xyz + float3(((fract((_13221.x + _13221.y) * _13221.z) - 0.5) * 0.0039215688593685626983642578125) * fast::clamp(_5081.w * 8.0, 0.0, 1.0));
        float4 _15492 = _5081;
        _15492.x = _5105.x;
        _15492.y = _5105.y;
        _15492.z = _5105.z;
        _17673 = _15492;
    }
    else
    {
        _17673 = _5081;
    }
    float3 _5115 = fast::max(_17673.xyz, float3(0.0));
    float4 _15498 = _17673;
    _15498.x = _5115.x;
    _15498.y = _5115.y;
    _15498.z = _5115.z;
    float4 _17674;
    if ((_184.gTime.w > 0.5) && (_17673.w > 9.9999997473787516355514526367188e-06))
    {
        float3 _13265 = fast::clamp(_15498.xyz / float3(_17673.w), float3(0.0), float3(1.0));
        float3 _13251 = select(pow((_13265 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _13265 * float3(0.077399380505084991455078125), _13265 <= float3(0.040449999272823333740234375)) * _17673.w;
        float4 _15507 = _15498;
        _15507.x = _13251.x;
        _15507.y = _13251.y;
        _15507.z = _13251.z;
        _17674 = _15507;
    }
    else
    {
        _17674 = _15498;
    }
    out._entryPointOutput = _17674;
    return out;
}

