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

constant spvUnsafeArray<float3, 6> _1668 = spvUnsafeArray<float3, 6>({ float3(1.0, 0.4199999868869781494140625, 0.2199999988079071044921875), float3(1.0, 0.699999988079071044921875, 0.300000011920928955078125), float3(0.800000011920928955078125, 0.920000016689300537109375, 0.4000000059604644775390625), float3(0.3499999940395355224609375, 0.89999997615814208984375, 0.699999988079071044921875), float3(0.4000000059604644775390625, 0.62000000476837158203125, 1.0), float3(0.660000026226043701171875, 0.5, 1.0) });
constant spvUnsafeArray<float4, 4> _2964 = spvUnsafeArray<float4, 4>({ float4(-1.0, 1.0, 3.400000095367431640625, 2.599999904632568359375), float4(-0.550000011920928955078125, 0.800000011920928955078125, 2.0, 0.800000011920928955078125), float4(0.300000011920928955078125, 1.0, 1.2000000476837158203125, 1.2999999523162841796875), float4(0.62000000476837158203125, 0.800000011920928955078125, 1.60000002384185791015625, 0.449999988079071044921875) });
constant spvUnsafeArray<float2, 4> _2981 = spvUnsafeArray<float2, 4>({ float2(0.0), float2(0.100000001490116119384765625, 1.2999999523162841796875), float2(0.550000011920928955078125, 3.900000095367431640625), float2(0.20000000298023223876953125, 5.19999980926513671875) });

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

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _178 [[buffer(0)]], constant WgtDraw& _221 [[buffer(2)]], const device gFxData& gFxData_1 [[buffer(10)]], texture2d<float> gTex [[texture(3)]], texture2d<float> gBackdrop0 [[texture(4)]], texture2d<float> gBackdrop1 [[texture(5)]], texture2d<float> gBackdrop2 [[texture(6)]], texture2d<float> gBackdrop3 [[texture(7)]], texture2d<float> gBackdrop4 [[texture(8)]], texture2d<float> gBackdrop5 [[texture(9)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float2 _5139 = gl_FragCoord.xy + _178.gConv.yy;
    float2 _22196;
    if (_178.gConv.x > 0.5)
    {
        float2 _20504 = _5139;
        _20504.y = _178.gTarget.y - _5139.y;
        _22196 = _20504;
    }
    else
    {
        _22196 = _5139;
    }
    float _3931 = dfdx(in.i_local.x);
    float _3935 = dfdy(in.i_local.x);
    float _3938 = fast::max(abs(_3931) + abs(_3935), 9.9999997473787516355514526367188e-05);
    float4 _22197;
    float4 _22199;
    if ((in.i_flags.z == 2u) || ((in.i_flags.x & 512u) != 0u))
    {
        _22199 = gFxData_1._data[(in.i_instance * 24u) + 15u];
        _22197 = gFxData_1._data[(in.i_instance * 24u) + 16u];
    }
    else
    {
        _22199 = float4(0.0);
        _22197 = float4(0.0);
    }
    float2 _5206 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
    float2 _5215 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
    float _22217;
    if (in.i_flags.z == 1u)
    {
        float2 _5221 = in.i_local - _5206;
        float _22216;
        do
        {
            if (in.i_shape.w >= 6.282185077667236328125)
            {
                _22216 = abs(length(_5221) - in.i_radii.x) - in.i_radii.y;
                break;
            }
            float _5320 = in.i_shape.z + (in.i_shape.w * 0.5);
            float _5322 = cos(_5320);
            float _5324 = sin(_5320);
            float _5333 = dot(_5221, float2(-_5324, _5322));
            float _5336 = dot(_5221, float2(_5322, _5324));
            float2 _5337 = float2(_5333, _5336);
            float _5340 = abs(_5333);
            _5337.x = _5340;
            float _5343 = in.i_shape.w * 0.5;
            float _5345 = sin(_5343);
            float _5347 = cos(_5343);
            _22216 = (((_5347 * _5340) > (_5345 * _5336)) ? length(_5337 - (float2(_5345, _5347) * in.i_radii.x)) : abs(length(_5337) - in.i_radii.x)) - in.i_radii.y;
            break;
        } while(false);
        _22217 = _22216;
    }
    else
    {
        float _22218;
        if (in.i_flags.z == 2u)
        {
            float2 _5383 = in.i_local - _22199.xy;
            float2 _5386 = _22199.zw - _22199.xy;
            _22218 = length(_5383 - (_5386 * fast::clamp(dot(_5383, _5386) / fast::max(dot(_5386, _5386), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
        }
        else
        {
            float2 _5248 = in.i_local - _5206;
            float _5439 = fast::min(_5215.x, _5215.y);
            float _5442 = fast::min((_5248.x > 0.0) ? ((_5248.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_5248.y > 0.0) ? in.i_radii.w : in.i_radii.x), _5439);
            float _5448 = _5442 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
            float _22201;
            float _22202;
            if (_5448 > _5439)
            {
                float _5462 = in.i_shape.y * fast::clamp((_5439 - _5442) / fast::max(0.60000002384185791015625 * _5442, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                _22202 = _5462;
                _22201 = _5442 * (1.0 + (0.60000002384185791015625 * _5462));
            }
            else
            {
                _22202 = in.i_shape.y;
                _22201 = _5448;
            }
            float2 _5475 = (abs(_5248) - _5215) + float2(_22201);
            float2 _5477 = fast::max(_5475, float2(0.0));
            float _22203;
            if ((_5477.x > 0.0) && (_5477.y > 0.0))
            {
                float _22204;
                if ((_22202 > 0.001000000047497451305389404296875) && (_22201 > 9.9999997473787516355514526367188e-05))
                {
                    float _5494 = 2.0 + (2.0 * _22202);
                    float2 _5499 = _5477 / float2(fast::max(_22201, 9.9999997473787516355514526367188e-05));
                    _22204 = pow(pow(_5499.x, _5494) + pow(_5499.y, _5494), 1.0 / _5494) * _22201;
                }
                else
                {
                    _22204 = length(_5477);
                }
                _22203 = _22204;
            }
            else
            {
                _22203 = fast::max(_5477.x, _5477.y);
            }
            float _5534 = (fast::min(fast::max(_5475.x, _5475.y), 0.0) + _22203) - _22201;
            float _22219;
            if ((in.i_flags.x & 512u) != 0u)
            {
                float2 _5276 = fast::max((_22199.zw - _22199.xy) * 0.5, float2(0.001000000047497451305389404296875));
                float2 _5279 = in.i_local - ((_22199.xy + _22199.zw) * 0.5);
                float _5570 = fast::min(_5276.x, _5276.y);
                float _5573 = fast::min((_5279.x > 0.0) ? ((_5279.y > 0.0) ? _22197.x : _22197.x) : ((_5279.y > 0.0) ? _22197.x : _22197.x), _5570);
                float _5579 = _5573 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                float _22207;
                float _22208;
                if (_5579 > _5570)
                {
                    float _5593 = in.i_shape.y * fast::clamp((_5570 - _5573) / fast::max(0.60000002384185791015625 * _5573, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _22208 = _5593;
                    _22207 = _5573 * (1.0 + (0.60000002384185791015625 * _5593));
                }
                else
                {
                    _22208 = in.i_shape.y;
                    _22207 = _5579;
                }
                float2 _5606 = (abs(_5279) - _5276) + float2(_22207);
                float2 _5608 = fast::max(_5606, float2(0.0));
                float _22209;
                if ((_5608.x > 0.0) && (_5608.y > 0.0))
                {
                    float _22210;
                    if ((_22208 > 0.001000000047497451305389404296875) && (_22207 > 9.9999997473787516355514526367188e-05))
                    {
                        float _5625 = 2.0 + (2.0 * _22208);
                        float2 _5630 = _5608 / float2(fast::max(_22207, 9.9999997473787516355514526367188e-05));
                        _22210 = pow(pow(_5630.x, _5625) + pow(_5630.y, _5625), 1.0 / _5625) * _22207;
                    }
                    else
                    {
                        _22210 = length(_5608);
                    }
                    _22209 = _22210;
                }
                else
                {
                    _22209 = fast::max(_5608.x, _5608.y);
                }
                float _5665 = (fast::min(fast::max(_5606.x, _5606.y), 0.0) + _22209) - _22207;
                float _5670 = fast::max(_22197.y, 9.9999997473787516355514526367188e-05);
                float _5679 = fast::max(_5670 - abs(_5534 - _5665), 0.0) / _5670;
                _22219 = fast::min(_5534, _5665) - (((_5679 * _5679) * _5670) * 0.25);
            }
            else
            {
                _22219 = _5534;
            }
            _22218 = _22219;
        }
        _22217 = _22218;
    }
    float _3963 = fast::clamp(0.5 - (_22217 / _3938), 0.0, 1.0);
    float _25359;
    if ((in.i_flags.x & 1024u) != 0u)
    {
        uint _5695 = (in.i_instance * 24u) + 19u;
        uint _5703 = (in.i_instance * 24u) + 20u;
        float2 _3992 = fast::max((gFxData_1._data[_5695].zw - gFxData_1._data[_5695].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float2 _3995 = in.i_local - ((gFxData_1._data[_5695].xy + gFxData_1._data[_5695].zw) * 0.5);
        float _5741 = fast::min(_3992.x, _3992.y);
        float _5744 = fast::min((_3995.x > 0.0) ? ((_3995.y > 0.0) ? gFxData_1._data[_5703].x : gFxData_1._data[_5703].x) : ((_3995.y > 0.0) ? gFxData_1._data[_5703].x : gFxData_1._data[_5703].x), _5741);
        float _5750 = _5744 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_5703].y));
        float _22220;
        float _22221;
        if (_5750 > _5741)
        {
            float _5764 = gFxData_1._data[_5703].y * fast::clamp((_5741 - _5744) / fast::max(0.60000002384185791015625 * _5744, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _22221 = _5764;
            _22220 = _5744 * (1.0 + (0.60000002384185791015625 * _5764));
        }
        else
        {
            _22221 = gFxData_1._data[_5703].y;
            _22220 = _5750;
        }
        float2 _5777 = (abs(_3995) - _3992) + float2(_22220);
        float2 _5779 = fast::max(_5777, float2(0.0));
        float _22222;
        if ((_5779.x > 0.0) && (_5779.y > 0.0))
        {
            float _22223;
            if ((_22221 > 0.001000000047497451305389404296875) && (_22220 > 9.9999997473787516355514526367188e-05))
            {
                float _5796 = 2.0 + (2.0 * _22221);
                float2 _5801 = _5779 / float2(fast::max(_22220, 9.9999997473787516355514526367188e-05));
                _22223 = pow(pow(_5801.x, _5796) + pow(_5801.y, _5796), 1.0 / _5796) * _22220;
            }
            else
            {
                _22223 = length(_5779);
            }
            _22222 = _22223;
        }
        else
        {
            _22222 = fast::max(_5779.x, _5779.y);
        }
        float _4007 = fast::clamp(0.5 - (((fast::min(fast::max(_5777.x, _5777.y), 0.0) + _22222) - _22220) / _3938), 0.0, 1.0);
        if (_4007 <= 0.0)
        {
            discard_fragment();
        }
        _25359 = _4007;
    }
    else
    {
        _25359 = 1.0;
    }
    bool _4018 = ((in.i_flags.x & 32u) != 0u) && (_3963 >= 0.999000012874603271484375);
    float4 _22312;
    if ((((in.i_flags.x & 4u) != 0u) && (!((in.i_flags.x & 256u) != 0u))) && (!_4018))
    {
        uint _5842 = (in.i_instance * 24u) + 7u;
        uint _5850 = (in.i_instance * 24u) + 8u;
        float2 _4049 = in.i_local - gFxData_1._data[_5850].zw;
        float2 _5891 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _5900 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _22270;
        if (in.i_flags.z == 1u)
        {
            float2 _5906 = _4049 - _5891;
            float _22269;
            do
            {
                if (in.i_shape.w >= 6.282185077667236328125)
                {
                    _22269 = abs(length(_5906) - in.i_radii.x) - in.i_radii.y;
                    break;
                }
                float _6005 = in.i_shape.z + (in.i_shape.w * 0.5);
                float _6007 = cos(_6005);
                float _6009 = sin(_6005);
                float _6018 = dot(_5906, float2(-_6009, _6007));
                float _6021 = dot(_5906, float2(_6007, _6009));
                float2 _6022 = float2(_6018, _6021);
                float _6025 = abs(_6018);
                _6022.x = _6025;
                float _6028 = in.i_shape.w * 0.5;
                float _6030 = sin(_6028);
                float _6032 = cos(_6028);
                _22269 = (((_6032 * _6025) > (_6030 * _6021)) ? length(_6022 - (float2(_6030, _6032) * in.i_radii.x)) : abs(length(_6022) - in.i_radii.x)) - in.i_radii.y;
                break;
            } while(false);
            _22270 = _22269;
        }
        else
        {
            float _22271;
            if (in.i_flags.z == 2u)
            {
                float2 _6068 = _4049 - _22199.xy;
                float2 _6071 = _22199.zw - _22199.xy;
                _22271 = length(_6068 - (_6071 * fast::clamp(dot(_6068, _6071) / fast::max(dot(_6071, _6071), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
            }
            else
            {
                float2 _5933 = _4049 - _5891;
                float _6124 = fast::min(_5900.x, _5900.y);
                float _6127 = fast::min((_5933.x > 0.0) ? ((_5933.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_5933.y > 0.0) ? in.i_radii.w : in.i_radii.x), _6124);
                float _6133 = _6127 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                float _22254;
                float _22255;
                if (_6133 > _6124)
                {
                    float _6147 = in.i_shape.y * fast::clamp((_6124 - _6127) / fast::max(0.60000002384185791015625 * _6127, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _22255 = _6147;
                    _22254 = _6127 * (1.0 + (0.60000002384185791015625 * _6147));
                }
                else
                {
                    _22255 = in.i_shape.y;
                    _22254 = _6133;
                }
                float2 _6160 = (abs(_5933) - _5900) + float2(_22254);
                float2 _6162 = fast::max(_6160, float2(0.0));
                float _22256;
                if ((_6162.x > 0.0) && (_6162.y > 0.0))
                {
                    float _22257;
                    if ((_22255 > 0.001000000047497451305389404296875) && (_22254 > 9.9999997473787516355514526367188e-05))
                    {
                        float _6179 = 2.0 + (2.0 * _22255);
                        float2 _6184 = _6162 / float2(fast::max(_22254, 9.9999997473787516355514526367188e-05));
                        _22257 = pow(pow(_6184.x, _6179) + pow(_6184.y, _6179), 1.0 / _6179) * _22254;
                    }
                    else
                    {
                        _22257 = length(_6162);
                    }
                    _22256 = _22257;
                }
                else
                {
                    _22256 = fast::max(_6162.x, _6162.y);
                }
                float _6219 = (fast::min(fast::max(_6160.x, _6160.y), 0.0) + _22256) - _22254;
                float _22272;
                if ((in.i_flags.x & 512u) != 0u)
                {
                    float2 _5961 = fast::max((_22199.zw - _22199.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _5964 = _4049 - ((_22199.xy + _22199.zw) * 0.5);
                    float _6255 = fast::min(_5961.x, _5961.y);
                    float _6258 = fast::min((_5964.x > 0.0) ? ((_5964.y > 0.0) ? _22197.x : _22197.x) : ((_5964.y > 0.0) ? _22197.x : _22197.x), _6255);
                    float _6264 = _6258 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _22260;
                    float _22261;
                    if (_6264 > _6255)
                    {
                        float _6278 = in.i_shape.y * fast::clamp((_6255 - _6258) / fast::max(0.60000002384185791015625 * _6258, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _22261 = _6278;
                        _22260 = _6258 * (1.0 + (0.60000002384185791015625 * _6278));
                    }
                    else
                    {
                        _22261 = in.i_shape.y;
                        _22260 = _6264;
                    }
                    float2 _6291 = (abs(_5964) - _5961) + float2(_22260);
                    float2 _6293 = fast::max(_6291, float2(0.0));
                    float _22262;
                    if ((_6293.x > 0.0) && (_6293.y > 0.0))
                    {
                        float _22263;
                        if ((_22261 > 0.001000000047497451305389404296875) && (_22260 > 9.9999997473787516355514526367188e-05))
                        {
                            float _6310 = 2.0 + (2.0 * _22261);
                            float2 _6315 = _6293 / float2(fast::max(_22260, 9.9999997473787516355514526367188e-05));
                            _22263 = pow(pow(_6315.x, _6310) + pow(_6315.y, _6310), 1.0 / _6310) * _22260;
                        }
                        else
                        {
                            _22263 = length(_6293);
                        }
                        _22262 = _22263;
                    }
                    else
                    {
                        _22262 = fast::max(_6293.x, _6293.y);
                    }
                    float _6350 = (fast::min(fast::max(_6291.x, _6291.y), 0.0) + _22262) - _22260;
                    float _6355 = fast::max(_22197.y, 9.9999997473787516355514526367188e-05);
                    float _6364 = fast::max(_6355 - abs(_6219 - _6350), 0.0) / _6355;
                    _22272 = fast::min(_6219, _6350) - (((_6364 * _6364) * _6355) * 0.25);
                }
                else
                {
                    _22272 = _6219;
                }
                _22271 = _22272;
            }
            _22270 = _22271;
        }
        float _4058 = (_22270 - gFxData_1._data[_5850].y) / (fast::max(gFxData_1._data[_5850].x * 0.5, _3938 * 0.5) * 1.41421353816986083984375);
        float _6381 = sign(_4058);
        float _6383 = abs(_4058);
        float _6394 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_6383 * _6383))) * _6383)) * _6383);
        float _6397 = _6394 * _6394;
        float _6412 = fast::clamp(gFxData_1._data[_5842].w * (0.5 - (0.5 * (_6381 - (_6381 / (_6397 * _6397))))), 0.0, 1.0);
        _22312 = float4(gFxData_1._data[_5842].xyz * _6412, _6412);
    }
    else
    {
        _22312 = float4(0.0);
    }
    float4 _23755;
    if (((in.i_flags.x & 8u) != 0u) && (!_4018))
    {
        uint _6436 = (in.i_instance * 24u) + 9u;
        uint _6444 = (in.i_instance * 24u) + 10u;
        float _4086 = fast::max(gFxData_1._data[_6444].x, 0.001000000047497451305389404296875);
        float _22305;
        float _22308;
        if ((in.i_flags.x & 8192u) != 0u)
        {
            uint _6452 = (in.i_instance * 24u) + 22u;
            float2 _4102 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _4111 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float2 _4118 = in.i_rect.xy - gFxData_1._data[_6452].xy;
            float2 _4125 = gFxData_1._data[_6452].zw - in.i_rect.zw;
            float2 _4155 = fast::clamp(float2((in.i_local.x < _4102.x) ? _4118.x : _4125.x, (in.i_local.y < _4102.y) ? _4118.y : _4125.y) * float2(0.58823525905609130859375), float2(fast::min(_4086, 1.5)), float2(_4086));
            float _4160 = fast::min(_4111.x, _4111.y);
            float _22303;
            if (in.i_flags.z == 0u)
            {
                _22303 = fast::min(((in.i_local.x > _4102.x) ? ((in.i_local.y > _4102.y) ? in.i_radii.z : in.i_radii.y) : ((in.i_local.y > _4102.y) ? in.i_radii.w : in.i_radii.x)) * (1.0 + (0.60000002384185791015625 * in.i_shape.y)), _4160);
            }
            else
            {
                _22303 = _4160;
            }
            float2 _4209 = fast::max(abs(in.i_local - _4102) - (_4111 - float2(_22303)), float2(0.0));
            float _4211 = length(_4209);
            float2 _4219 = select(float2(0.707099974155426025390625), _4209 / float2(_4211), bool2(_4211 > 9.9999997473787516355514526367188e-05));
            float2 _4234 = in.i_local - gFxData_1._data[_6452].xy;
            float2 _4239 = gFxData_1._data[_6452].zw - in.i_local;
            _22308 = fast::clamp(fast::min(fast::min(_4234.x, _4234.y), fast::min(_4239.x, _4239.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _22305 = rsqrt(dot(_4219 * _4219, float2(1.0) / (_4155 * _4155)));
        }
        else
        {
            _22308 = 1.0;
            _22305 = _4086;
        }
        float _4257 = fast::max(_22217, 0.0) / _22305;
        float _6462 = fast::clamp(gFxData_1._data[_6436].w * fast::clamp((exp(((-_4257) * _4257) * 2.2000000476837158203125) * gFxData_1._data[_6444].y) * _22308, 0.0, 1.0), 0.0, 1.0);
        _23755 = float4(gFxData_1._data[_6436].xyz * _6462, _6462) + (_22312 * (1.0 - _6462));
    }
    else
    {
        _23755 = _22312;
    }
    float4 _24665;
    float4 _24678;
    float _25325;
    if (((in.i_flags.x & 32u) != 0u) && (_3963 > 0.0))
    {
        uint _6486 = (in.i_instance * 24u) + 11u;
        uint _6494 = (in.i_instance * 24u) + 12u;
        uint _6502 = (in.i_instance * 24u) + 13u;
        uint _6510 = (in.i_instance * 24u) + 16u;
        float _6577 = gFxData_1._data[_6486].x * _178.gDisplay.z;
        float2 _6588 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(1.0));
        float _6596 = fast::clamp(gFxData_1._data[_6486].z, 0.001000000047497451305389404296875, fast::min(_6588.x, _6588.y));
        float _6605 = fast::clamp(1.0 - (fast::max(-_22217, 0.0) / _6596), 0.0, 1.0);
        float _6611 = sqrt(fast::clamp(1.0 - (_6605 * _6605), 0.0, 1.0));
        float2 _22404;
        float3 _23266;
        if (_6605 > 0.0)
        {
            float2 _6993 = in.i_local + float2(0.5, 0.0);
            float2 _7061 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _7070 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _22344;
            if (in.i_flags.z == 1u)
            {
                float2 _7076 = _6993 - _7061;
                float _22343;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _22343 = abs(length(_7076) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _7175 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _7177 = cos(_7175);
                    float _7179 = sin(_7175);
                    float _7188 = dot(_7076, float2(-_7179, _7177));
                    float _7191 = dot(_7076, float2(_7177, _7179));
                    float2 _7192 = float2(_7188, _7191);
                    float _7195 = abs(_7188);
                    _7192.x = _7195;
                    float _7198 = in.i_shape.w * 0.5;
                    float _7200 = sin(_7198);
                    float _7202 = cos(_7198);
                    _22343 = (((_7202 * _7195) > (_7200 * _7191)) ? length(_7192 - (float2(_7200, _7202) * in.i_radii.x)) : abs(length(_7192) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _22344 = _22343;
            }
            else
            {
                float _22345;
                if (in.i_flags.z == 2u)
                {
                    float2 _7238 = _6993 - _22199.xy;
                    float2 _7241 = _22199.zw - _22199.xy;
                    _22345 = length(_7238 - (_7241 * fast::clamp(dot(_7238, _7241) / fast::max(dot(_7241, _7241), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _7103 = _6993 - _7061;
                    float _7294 = fast::min(_7070.x, _7070.y);
                    float _7297 = fast::min((_7103.x > 0.0) ? ((_7103.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_7103.y > 0.0) ? in.i_radii.w : in.i_radii.x), _7294);
                    float _7303 = _7297 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _22328;
                    float _22329;
                    if (_7303 > _7294)
                    {
                        float _7317 = in.i_shape.y * fast::clamp((_7294 - _7297) / fast::max(0.60000002384185791015625 * _7297, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _22329 = _7317;
                        _22328 = _7297 * (1.0 + (0.60000002384185791015625 * _7317));
                    }
                    else
                    {
                        _22329 = in.i_shape.y;
                        _22328 = _7303;
                    }
                    float2 _7330 = (abs(_7103) - _7070) + float2(_22328);
                    float2 _7332 = fast::max(_7330, float2(0.0));
                    float _22330;
                    if ((_7332.x > 0.0) && (_7332.y > 0.0))
                    {
                        float _22331;
                        if ((_22329 > 0.001000000047497451305389404296875) && (_22328 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7349 = 2.0 + (2.0 * _22329);
                            float2 _7354 = _7332 / float2(fast::max(_22328, 9.9999997473787516355514526367188e-05));
                            _22331 = pow(pow(_7354.x, _7349) + pow(_7354.y, _7349), 1.0 / _7349) * _22328;
                        }
                        else
                        {
                            _22331 = length(_7332);
                        }
                        _22330 = _22331;
                    }
                    else
                    {
                        _22330 = fast::max(_7332.x, _7332.y);
                    }
                    float _7389 = (fast::min(fast::max(_7330.x, _7330.y), 0.0) + _22330) - _22328;
                    float _22346;
                    if ((in.i_flags.x & 512u) != 0u)
                    {
                        float2 _7131 = fast::max((_22199.zw - _22199.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _7134 = _6993 - ((_22199.xy + _22199.zw) * 0.5);
                        float _7425 = fast::min(_7131.x, _7131.y);
                        float _7428 = fast::min((_7134.x > 0.0) ? ((_7134.y > 0.0) ? gFxData_1._data[_6510].x : gFxData_1._data[_6510].x) : ((_7134.y > 0.0) ? gFxData_1._data[_6510].x : gFxData_1._data[_6510].x), _7425);
                        float _7434 = _7428 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _22334;
                        float _22335;
                        if (_7434 > _7425)
                        {
                            float _7448 = in.i_shape.y * fast::clamp((_7425 - _7428) / fast::max(0.60000002384185791015625 * _7428, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _22335 = _7448;
                            _22334 = _7428 * (1.0 + (0.60000002384185791015625 * _7448));
                        }
                        else
                        {
                            _22335 = in.i_shape.y;
                            _22334 = _7434;
                        }
                        float2 _7461 = (abs(_7134) - _7131) + float2(_22334);
                        float2 _7463 = fast::max(_7461, float2(0.0));
                        float _22336;
                        if ((_7463.x > 0.0) && (_7463.y > 0.0))
                        {
                            float _22337;
                            if ((_22335 > 0.001000000047497451305389404296875) && (_22334 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7480 = 2.0 + (2.0 * _22335);
                                float2 _7485 = _7463 / float2(fast::max(_22334, 9.9999997473787516355514526367188e-05));
                                _22337 = pow(pow(_7485.x, _7480) + pow(_7485.y, _7480), 1.0 / _7480) * _22334;
                            }
                            else
                            {
                                _22337 = length(_7463);
                            }
                            _22336 = _22337;
                        }
                        else
                        {
                            _22336 = fast::max(_7463.x, _7463.y);
                        }
                        float _7520 = (fast::min(fast::max(_7461.x, _7461.y), 0.0) + _22336) - _22334;
                        float _7525 = fast::max(gFxData_1._data[_6510].y, 9.9999997473787516355514526367188e-05);
                        float _7534 = fast::max(_7525 - abs(_7389 - _7520), 0.0) / _7525;
                        _22346 = fast::min(_7389, _7520) - (((_7534 * _7534) * _7525) * 0.25);
                    }
                    else
                    {
                        _22346 = _7389;
                    }
                    _22345 = _22346;
                }
                _22344 = _22345;
            }
            float2 _6997 = in.i_local - float2(0.5, 0.0);
            float2 _7583 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _7592 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _22363;
            if (in.i_flags.z == 1u)
            {
                float2 _7598 = _6997 - _7583;
                float _22362;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _22362 = abs(length(_7598) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _7697 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _7699 = cos(_7697);
                    float _7701 = sin(_7697);
                    float _7710 = dot(_7598, float2(-_7701, _7699));
                    float _7713 = dot(_7598, float2(_7699, _7701));
                    float2 _7714 = float2(_7710, _7713);
                    float _7717 = abs(_7710);
                    _7714.x = _7717;
                    float _7720 = in.i_shape.w * 0.5;
                    float _7722 = sin(_7720);
                    float _7724 = cos(_7720);
                    _22362 = (((_7724 * _7717) > (_7722 * _7713)) ? length(_7714 - (float2(_7722, _7724) * in.i_radii.x)) : abs(length(_7714) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _22363 = _22362;
            }
            else
            {
                float _22364;
                if (in.i_flags.z == 2u)
                {
                    float2 _7760 = _6997 - _22199.xy;
                    float2 _7763 = _22199.zw - _22199.xy;
                    _22364 = length(_7760 - (_7763 * fast::clamp(dot(_7760, _7763) / fast::max(dot(_7763, _7763), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _7625 = _6997 - _7583;
                    float _7816 = fast::min(_7592.x, _7592.y);
                    float _7819 = fast::min((_7625.x > 0.0) ? ((_7625.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_7625.y > 0.0) ? in.i_radii.w : in.i_radii.x), _7816);
                    float _7825 = _7819 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _22347;
                    float _22348;
                    if (_7825 > _7816)
                    {
                        float _7839 = in.i_shape.y * fast::clamp((_7816 - _7819) / fast::max(0.60000002384185791015625 * _7819, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _22348 = _7839;
                        _22347 = _7819 * (1.0 + (0.60000002384185791015625 * _7839));
                    }
                    else
                    {
                        _22348 = in.i_shape.y;
                        _22347 = _7825;
                    }
                    float2 _7852 = (abs(_7625) - _7592) + float2(_22347);
                    float2 _7854 = fast::max(_7852, float2(0.0));
                    float _22349;
                    if ((_7854.x > 0.0) && (_7854.y > 0.0))
                    {
                        float _22350;
                        if ((_22348 > 0.001000000047497451305389404296875) && (_22347 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7871 = 2.0 + (2.0 * _22348);
                            float2 _7876 = _7854 / float2(fast::max(_22347, 9.9999997473787516355514526367188e-05));
                            _22350 = pow(pow(_7876.x, _7871) + pow(_7876.y, _7871), 1.0 / _7871) * _22347;
                        }
                        else
                        {
                            _22350 = length(_7854);
                        }
                        _22349 = _22350;
                    }
                    else
                    {
                        _22349 = fast::max(_7854.x, _7854.y);
                    }
                    float _7911 = (fast::min(fast::max(_7852.x, _7852.y), 0.0) + _22349) - _22347;
                    float _22365;
                    if ((in.i_flags.x & 512u) != 0u)
                    {
                        float2 _7653 = fast::max((_22199.zw - _22199.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _7656 = _6997 - ((_22199.xy + _22199.zw) * 0.5);
                        float _7947 = fast::min(_7653.x, _7653.y);
                        float _7950 = fast::min((_7656.x > 0.0) ? ((_7656.y > 0.0) ? gFxData_1._data[_6510].x : gFxData_1._data[_6510].x) : ((_7656.y > 0.0) ? gFxData_1._data[_6510].x : gFxData_1._data[_6510].x), _7947);
                        float _7956 = _7950 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _22353;
                        float _22354;
                        if (_7956 > _7947)
                        {
                            float _7970 = in.i_shape.y * fast::clamp((_7947 - _7950) / fast::max(0.60000002384185791015625 * _7950, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _22354 = _7970;
                            _22353 = _7950 * (1.0 + (0.60000002384185791015625 * _7970));
                        }
                        else
                        {
                            _22354 = in.i_shape.y;
                            _22353 = _7956;
                        }
                        float2 _7983 = (abs(_7656) - _7653) + float2(_22353);
                        float2 _7985 = fast::max(_7983, float2(0.0));
                        float _22355;
                        if ((_7985.x > 0.0) && (_7985.y > 0.0))
                        {
                            float _22356;
                            if ((_22354 > 0.001000000047497451305389404296875) && (_22353 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8002 = 2.0 + (2.0 * _22354);
                                float2 _8007 = _7985 / float2(fast::max(_22353, 9.9999997473787516355514526367188e-05));
                                _22356 = pow(pow(_8007.x, _8002) + pow(_8007.y, _8002), 1.0 / _8002) * _22353;
                            }
                            else
                            {
                                _22356 = length(_7985);
                            }
                            _22355 = _22356;
                        }
                        else
                        {
                            _22355 = fast::max(_7985.x, _7985.y);
                        }
                        float _8042 = (fast::min(fast::max(_7983.x, _7983.y), 0.0) + _22355) - _22353;
                        float _8047 = fast::max(gFxData_1._data[_6510].y, 9.9999997473787516355514526367188e-05);
                        float _8056 = fast::max(_8047 - abs(_7911 - _8042), 0.0) / _8047;
                        _22365 = fast::min(_7911, _8042) - (((_8056 * _8056) * _8047) * 0.25);
                    }
                    else
                    {
                        _22365 = _7911;
                    }
                    _22364 = _22365;
                }
                _22363 = _22364;
            }
            float2 _7002 = in.i_local + float2(0.0, 0.5);
            float2 _8105 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _8114 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _22382;
            if (in.i_flags.z == 1u)
            {
                float2 _8120 = _7002 - _8105;
                float _22381;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _22381 = abs(length(_8120) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _8219 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _8221 = cos(_8219);
                    float _8223 = sin(_8219);
                    float _8232 = dot(_8120, float2(-_8223, _8221));
                    float _8235 = dot(_8120, float2(_8221, _8223));
                    float2 _8236 = float2(_8232, _8235);
                    float _8239 = abs(_8232);
                    _8236.x = _8239;
                    float _8242 = in.i_shape.w * 0.5;
                    float _8244 = sin(_8242);
                    float _8246 = cos(_8242);
                    _22381 = (((_8246 * _8239) > (_8244 * _8235)) ? length(_8236 - (float2(_8244, _8246) * in.i_radii.x)) : abs(length(_8236) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _22382 = _22381;
            }
            else
            {
                float _22383;
                if (in.i_flags.z == 2u)
                {
                    float2 _8282 = _7002 - _22199.xy;
                    float2 _8285 = _22199.zw - _22199.xy;
                    _22383 = length(_8282 - (_8285 * fast::clamp(dot(_8282, _8285) / fast::max(dot(_8285, _8285), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _8147 = _7002 - _8105;
                    float _8338 = fast::min(_8114.x, _8114.y);
                    float _8341 = fast::min((_8147.x > 0.0) ? ((_8147.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_8147.y > 0.0) ? in.i_radii.w : in.i_radii.x), _8338);
                    float _8347 = _8341 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _22366;
                    float _22367;
                    if (_8347 > _8338)
                    {
                        float _8361 = in.i_shape.y * fast::clamp((_8338 - _8341) / fast::max(0.60000002384185791015625 * _8341, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _22367 = _8361;
                        _22366 = _8341 * (1.0 + (0.60000002384185791015625 * _8361));
                    }
                    else
                    {
                        _22367 = in.i_shape.y;
                        _22366 = _8347;
                    }
                    float2 _8374 = (abs(_8147) - _8114) + float2(_22366);
                    float2 _8376 = fast::max(_8374, float2(0.0));
                    float _22368;
                    if ((_8376.x > 0.0) && (_8376.y > 0.0))
                    {
                        float _22369;
                        if ((_22367 > 0.001000000047497451305389404296875) && (_22366 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8393 = 2.0 + (2.0 * _22367);
                            float2 _8398 = _8376 / float2(fast::max(_22366, 9.9999997473787516355514526367188e-05));
                            _22369 = pow(pow(_8398.x, _8393) + pow(_8398.y, _8393), 1.0 / _8393) * _22366;
                        }
                        else
                        {
                            _22369 = length(_8376);
                        }
                        _22368 = _22369;
                    }
                    else
                    {
                        _22368 = fast::max(_8376.x, _8376.y);
                    }
                    float _8433 = (fast::min(fast::max(_8374.x, _8374.y), 0.0) + _22368) - _22366;
                    float _22384;
                    if ((in.i_flags.x & 512u) != 0u)
                    {
                        float2 _8175 = fast::max((_22199.zw - _22199.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _8178 = _7002 - ((_22199.xy + _22199.zw) * 0.5);
                        float _8469 = fast::min(_8175.x, _8175.y);
                        float _8472 = fast::min((_8178.x > 0.0) ? ((_8178.y > 0.0) ? gFxData_1._data[_6510].x : gFxData_1._data[_6510].x) : ((_8178.y > 0.0) ? gFxData_1._data[_6510].x : gFxData_1._data[_6510].x), _8469);
                        float _8478 = _8472 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _22372;
                        float _22373;
                        if (_8478 > _8469)
                        {
                            float _8492 = in.i_shape.y * fast::clamp((_8469 - _8472) / fast::max(0.60000002384185791015625 * _8472, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _22373 = _8492;
                            _22372 = _8472 * (1.0 + (0.60000002384185791015625 * _8492));
                        }
                        else
                        {
                            _22373 = in.i_shape.y;
                            _22372 = _8478;
                        }
                        float2 _8505 = (abs(_8178) - _8175) + float2(_22372);
                        float2 _8507 = fast::max(_8505, float2(0.0));
                        float _22374;
                        if ((_8507.x > 0.0) && (_8507.y > 0.0))
                        {
                            float _22375;
                            if ((_22373 > 0.001000000047497451305389404296875) && (_22372 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8524 = 2.0 + (2.0 * _22373);
                                float2 _8529 = _8507 / float2(fast::max(_22372, 9.9999997473787516355514526367188e-05));
                                _22375 = pow(pow(_8529.x, _8524) + pow(_8529.y, _8524), 1.0 / _8524) * _22372;
                            }
                            else
                            {
                                _22375 = length(_8507);
                            }
                            _22374 = _22375;
                        }
                        else
                        {
                            _22374 = fast::max(_8507.x, _8507.y);
                        }
                        float _8564 = (fast::min(fast::max(_8505.x, _8505.y), 0.0) + _22374) - _22372;
                        float _8569 = fast::max(gFxData_1._data[_6510].y, 9.9999997473787516355514526367188e-05);
                        float _8578 = fast::max(_8569 - abs(_8433 - _8564), 0.0) / _8569;
                        _22384 = fast::min(_8433, _8564) - (((_8578 * _8578) * _8569) * 0.25);
                    }
                    else
                    {
                        _22384 = _8433;
                    }
                    _22383 = _22384;
                }
                _22382 = _22383;
            }
            float2 _7006 = in.i_local - float2(0.0, 0.5);
            float2 _8627 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _8636 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _22401;
            if (in.i_flags.z == 1u)
            {
                float2 _8642 = _7006 - _8627;
                float _22400;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _22400 = abs(length(_8642) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _8741 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _8743 = cos(_8741);
                    float _8745 = sin(_8741);
                    float _8754 = dot(_8642, float2(-_8745, _8743));
                    float _8757 = dot(_8642, float2(_8743, _8745));
                    float2 _8758 = float2(_8754, _8757);
                    float _8761 = abs(_8754);
                    _8758.x = _8761;
                    float _8764 = in.i_shape.w * 0.5;
                    float _8766 = sin(_8764);
                    float _8768 = cos(_8764);
                    _22400 = (((_8768 * _8761) > (_8766 * _8757)) ? length(_8758 - (float2(_8766, _8768) * in.i_radii.x)) : abs(length(_8758) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _22401 = _22400;
            }
            else
            {
                float _22402;
                if (in.i_flags.z == 2u)
                {
                    float2 _8804 = _7006 - _22199.xy;
                    float2 _8807 = _22199.zw - _22199.xy;
                    _22402 = length(_8804 - (_8807 * fast::clamp(dot(_8804, _8807) / fast::max(dot(_8807, _8807), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _8669 = _7006 - _8627;
                    float _8860 = fast::min(_8636.x, _8636.y);
                    float _8863 = fast::min((_8669.x > 0.0) ? ((_8669.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_8669.y > 0.0) ? in.i_radii.w : in.i_radii.x), _8860);
                    float _8869 = _8863 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _22385;
                    float _22386;
                    if (_8869 > _8860)
                    {
                        float _8883 = in.i_shape.y * fast::clamp((_8860 - _8863) / fast::max(0.60000002384185791015625 * _8863, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _22386 = _8883;
                        _22385 = _8863 * (1.0 + (0.60000002384185791015625 * _8883));
                    }
                    else
                    {
                        _22386 = in.i_shape.y;
                        _22385 = _8869;
                    }
                    float2 _8896 = (abs(_8669) - _8636) + float2(_22385);
                    float2 _8898 = fast::max(_8896, float2(0.0));
                    float _22387;
                    if ((_8898.x > 0.0) && (_8898.y > 0.0))
                    {
                        float _22388;
                        if ((_22386 > 0.001000000047497451305389404296875) && (_22385 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8915 = 2.0 + (2.0 * _22386);
                            float2 _8920 = _8898 / float2(fast::max(_22385, 9.9999997473787516355514526367188e-05));
                            _22388 = pow(pow(_8920.x, _8915) + pow(_8920.y, _8915), 1.0 / _8915) * _22385;
                        }
                        else
                        {
                            _22388 = length(_8898);
                        }
                        _22387 = _22388;
                    }
                    else
                    {
                        _22387 = fast::max(_8898.x, _8898.y);
                    }
                    float _8955 = (fast::min(fast::max(_8896.x, _8896.y), 0.0) + _22387) - _22385;
                    float _22403;
                    if ((in.i_flags.x & 512u) != 0u)
                    {
                        float2 _8697 = fast::max((_22199.zw - _22199.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _8700 = _7006 - ((_22199.xy + _22199.zw) * 0.5);
                        float _8991 = fast::min(_8697.x, _8697.y);
                        float _8994 = fast::min((_8700.x > 0.0) ? ((_8700.y > 0.0) ? gFxData_1._data[_6510].x : gFxData_1._data[_6510].x) : ((_8700.y > 0.0) ? gFxData_1._data[_6510].x : gFxData_1._data[_6510].x), _8991);
                        float _9000 = _8994 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _22391;
                        float _22392;
                        if (_9000 > _8991)
                        {
                            float _9014 = in.i_shape.y * fast::clamp((_8991 - _8994) / fast::max(0.60000002384185791015625 * _8994, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _22392 = _9014;
                            _22391 = _8994 * (1.0 + (0.60000002384185791015625 * _9014));
                        }
                        else
                        {
                            _22392 = in.i_shape.y;
                            _22391 = _9000;
                        }
                        float2 _9027 = (abs(_8700) - _8697) + float2(_22391);
                        float2 _9029 = fast::max(_9027, float2(0.0));
                        float _22393;
                        if ((_9029.x > 0.0) && (_9029.y > 0.0))
                        {
                            float _22394;
                            if ((_22392 > 0.001000000047497451305389404296875) && (_22391 > 9.9999997473787516355514526367188e-05))
                            {
                                float _9046 = 2.0 + (2.0 * _22392);
                                float2 _9051 = _9029 / float2(fast::max(_22391, 9.9999997473787516355514526367188e-05));
                                _22394 = pow(pow(_9051.x, _9046) + pow(_9051.y, _9046), 1.0 / _9046) * _22391;
                            }
                            else
                            {
                                _22394 = length(_9029);
                            }
                            _22393 = _22394;
                        }
                        else
                        {
                            _22393 = fast::max(_9029.x, _9029.y);
                        }
                        float _9086 = (fast::min(fast::max(_9027.x, _9027.y), 0.0) + _22393) - _22391;
                        float _9091 = fast::max(gFxData_1._data[_6510].y, 9.9999997473787516355514526367188e-05);
                        float _9100 = fast::max(_9091 - abs(_8955 - _9086), 0.0) / _9091;
                        _22403 = fast::min(_8955, _9086) - (((_9100 * _9100) * _9091) * 0.25);
                    }
                    else
                    {
                        _22403 = _8955;
                    }
                    _22402 = _22403;
                }
                _22401 = _22402;
            }
            float2 _7012 = float2(_22344 - _22363, _22382 - _22401);
            float _7014 = length(_7012);
            float2 _7022 = select(float2(0.0, -1.0), _7012 / float2(_7014), bool2(_7014 > 9.9999997473787516355514526367188e-06));
            _23266 = fast::normalize(float3(_7022 * fast::min(_6605 / fast::max(_6611, 0.001000000047497451305389404296875), 8.0), 1.0));
            _22404 = _7022;
        }
        else
        {
            _23266 = float3(0.0, 0.0, 1.0);
            _22404 = float2(0.0, -1.0);
        }
        float2 _6637 = ((-_22404) * gFxData_1._data[_6486].y) * (1.0 - _6611);
        float2 _22405;
        if (gFxData_1._data[_6510].z > 0.0)
        {
            _22405 = (((in.i_rect.xy + in.i_rect.zw) * 0.5) - in.i_local) * (gFxData_1._data[_6510].z / (1.0 + gFxData_1._data[_6510].z));
        }
        else
        {
            _22405 = float2(0.0);
        }
        float2 _6663 = _22196 * _178.gTarget.zw;
        float2 _6670 = _178.gDisplay.zw * _178.gTarget.zw;
        float2 _6675 = (_6637 + _22405) * _6670;
        float2 _6678 = _6637 * _6670;
        float3 _23010;
        float _23033;
        float3 _23502;
        if (_178.gTime.z > 0.5)
        {
            float3 _23013;
            if (((gFxData_1._data[_6486].w > 0.001000000047497451305389404296875) && (_6605 > 0.0)) && ((((gFxData_1._data[_6486].y * 0.300000011920928955078125) * gFxData_1._data[_6486].w) * _178.gDisplay.z) > (_6577 * 0.3499999940395355224609375)))
            {
                float _6700 = 0.300000011920928955078125 * gFxData_1._data[_6486].w;
                float2 _6707 = (_6663 + _6675) - (_6678 * _6700);
                float _9125 = fast::clamp(log2(fast::max(_6577, 1.0)) - 1.0, 0.0, 5.0);
                int _9128 = int(floor(_9125));
                float _9132 = _9125 - float(_9128);
                float4 _22480;
                if (_9128 <= 0)
                {
                    float2 _22479;
                    if (_178.gConv.x > 0.5)
                    {
                        float2 _20887 = _6707;
                        _20887.y = 1.0 - _6707.y;
                        _22479 = _20887;
                    }
                    else
                    {
                        _22479 = _6707;
                    }
                    _22480 = gBackdrop0.sample(gLinear, _22479, level(0.0));
                }
                else
                {
                    float4 _22481;
                    if (_9128 == 1)
                    {
                        float2 _9267 = (_6707 * _178.gLevel[1].xy) - float2(0.5);
                        float2 _9269 = floor(_9267);
                        float2 _9272 = _9267 - _9269;
                        float2 _9275 = _9272 * _9272;
                        float2 _9278 = _9275 * _9272;
                        float2 _9297 = (((_9278 * 3.0) - (_9275 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _9310 = _9278 * 0.16666667163372039794921875;
                        float2 _9313 = (((((-_9278) + (_9275 * 3.0)) - (_9272 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9297;
                        float2 _9316 = (((((_9278 * (-3.0)) + (_9275 * 3.0)) + (_9272 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9310;
                        float2 _9326 = ((_9269 - float2(0.5)) + (_9297 / _9313)) * _178.gLevel[1].zw;
                        float2 _9336 = ((_9269 + float2(1.5)) + (_9310 / _9316)) * _178.gLevel[1].zw;
                        float2 _22475;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _20892 = _9326;
                            _20892.y = 1.0 - _9326.y;
                            _22475 = _20892;
                        }
                        else
                        {
                            _22475 = _9326;
                        }
                        float _9356 = _9326.y;
                        float2 _9357 = float2(_9336.x, _9356);
                        float2 _22476;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _20898 = _9357;
                            _20898.y = 1.0 - _9356;
                            _22476 = _20898;
                        }
                        else
                        {
                            _22476 = _9357;
                        }
                        float2 _9374 = float2(_9326.x, _9336.y);
                        float2 _22477;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _20905 = _9374;
                            _20905.y = 1.0 - _9336.y;
                            _22477 = _20905;
                        }
                        else
                        {
                            _22477 = _9374;
                        }
                        float2 _22478;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _20911 = _9336;
                            _20911.y = 1.0 - _9336.y;
                            _22478 = _20911;
                        }
                        else
                        {
                            _22478 = _9336;
                        }
                        _22481 = (((gBackdrop1.sample(gLinear, _22475, level(0.0)) * _9313.x) + (gBackdrop1.sample(gLinear, _22476, level(0.0)) * _9316.x)) * _9313.y) + (((gBackdrop1.sample(gLinear, _22477, level(0.0)) * _9313.x) + (gBackdrop1.sample(gLinear, _22478, level(0.0)) * _9316.x)) * _9316.y);
                    }
                    else
                    {
                        float4 _22482;
                        if (_9128 == 2)
                        {
                            float2 _9474 = (_6707 * _178.gLevel[2].xy) - float2(0.5);
                            float2 _9476 = floor(_9474);
                            float2 _9479 = _9474 - _9476;
                            float2 _9482 = _9479 * _9479;
                            float2 _9485 = _9482 * _9479;
                            float2 _9504 = (((_9485 * 3.0) - (_9482 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _9517 = _9485 * 0.16666667163372039794921875;
                            float2 _9520 = (((((-_9485) + (_9482 * 3.0)) - (_9479 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9504;
                            float2 _9523 = (((((_9485 * (-3.0)) + (_9482 * 3.0)) + (_9479 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9517;
                            float2 _9533 = ((_9476 - float2(0.5)) + (_9504 / _9520)) * _178.gLevel[2].zw;
                            float2 _9543 = ((_9476 + float2(1.5)) + (_9517 / _9523)) * _178.gLevel[2].zw;
                            float2 _22471;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _20918 = _9533;
                                _20918.y = 1.0 - _9533.y;
                                _22471 = _20918;
                            }
                            else
                            {
                                _22471 = _9533;
                            }
                            float _9563 = _9533.y;
                            float2 _9564 = float2(_9543.x, _9563);
                            float2 _22472;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _20924 = _9564;
                                _20924.y = 1.0 - _9563;
                                _22472 = _20924;
                            }
                            else
                            {
                                _22472 = _9564;
                            }
                            float2 _9581 = float2(_9533.x, _9543.y);
                            float2 _22473;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _20931 = _9581;
                                _20931.y = 1.0 - _9543.y;
                                _22473 = _20931;
                            }
                            else
                            {
                                _22473 = _9581;
                            }
                            float2 _22474;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _20937 = _9543;
                                _20937.y = 1.0 - _9543.y;
                                _22474 = _20937;
                            }
                            else
                            {
                                _22474 = _9543;
                            }
                            _22482 = (((gBackdrop2.sample(gLinear, _22471, level(0.0)) * _9520.x) + (gBackdrop2.sample(gLinear, _22472, level(0.0)) * _9523.x)) * _9520.y) + (((gBackdrop2.sample(gLinear, _22473, level(0.0)) * _9520.x) + (gBackdrop2.sample(gLinear, _22474, level(0.0)) * _9523.x)) * _9523.y);
                        }
                        else
                        {
                            float4 _22483;
                            if (_9128 == 3)
                            {
                                float2 _9681 = (_6707 * _178.gLevel[3].xy) - float2(0.5);
                                float2 _9683 = floor(_9681);
                                float2 _9686 = _9681 - _9683;
                                float2 _9689 = _9686 * _9686;
                                float2 _9692 = _9689 * _9686;
                                float2 _9711 = (((_9692 * 3.0) - (_9689 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _9724 = _9692 * 0.16666667163372039794921875;
                                float2 _9727 = (((((-_9692) + (_9689 * 3.0)) - (_9686 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9711;
                                float2 _9730 = (((((_9692 * (-3.0)) + (_9689 * 3.0)) + (_9686 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9724;
                                float2 _9740 = ((_9683 - float2(0.5)) + (_9711 / _9727)) * _178.gLevel[3].zw;
                                float2 _9750 = ((_9683 + float2(1.5)) + (_9724 / _9730)) * _178.gLevel[3].zw;
                                float2 _22467;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _20944 = _9740;
                                    _20944.y = 1.0 - _9740.y;
                                    _22467 = _20944;
                                }
                                else
                                {
                                    _22467 = _9740;
                                }
                                float _9770 = _9740.y;
                                float2 _9771 = float2(_9750.x, _9770);
                                float2 _22468;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _20950 = _9771;
                                    _20950.y = 1.0 - _9770;
                                    _22468 = _20950;
                                }
                                else
                                {
                                    _22468 = _9771;
                                }
                                float2 _9788 = float2(_9740.x, _9750.y);
                                float2 _22469;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _20957 = _9788;
                                    _20957.y = 1.0 - _9750.y;
                                    _22469 = _20957;
                                }
                                else
                                {
                                    _22469 = _9788;
                                }
                                float2 _22470;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _20963 = _9750;
                                    _20963.y = 1.0 - _9750.y;
                                    _22470 = _20963;
                                }
                                else
                                {
                                    _22470 = _9750;
                                }
                                _22483 = (((gBackdrop3.sample(gLinear, _22467, level(0.0)) * _9727.x) + (gBackdrop3.sample(gLinear, _22468, level(0.0)) * _9730.x)) * _9727.y) + (((gBackdrop3.sample(gLinear, _22469, level(0.0)) * _9727.x) + (gBackdrop3.sample(gLinear, _22470, level(0.0)) * _9730.x)) * _9730.y);
                            }
                            else
                            {
                                float4 _22484;
                                if (_9128 == 4)
                                {
                                    float2 _9888 = (_6707 * _178.gLevel[4].xy) - float2(0.5);
                                    float2 _9890 = floor(_9888);
                                    float2 _9893 = _9888 - _9890;
                                    float2 _9896 = _9893 * _9893;
                                    float2 _9899 = _9896 * _9893;
                                    float2 _9918 = (((_9899 * 3.0) - (_9896 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _9931 = _9899 * 0.16666667163372039794921875;
                                    float2 _9934 = (((((-_9899) + (_9896 * 3.0)) - (_9893 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9918;
                                    float2 _9937 = (((((_9899 * (-3.0)) + (_9896 * 3.0)) + (_9893 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9931;
                                    float2 _9947 = ((_9890 - float2(0.5)) + (_9918 / _9934)) * _178.gLevel[4].zw;
                                    float2 _9957 = ((_9890 + float2(1.5)) + (_9931 / _9937)) * _178.gLevel[4].zw;
                                    float2 _22463;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _20970 = _9947;
                                        _20970.y = 1.0 - _9947.y;
                                        _22463 = _20970;
                                    }
                                    else
                                    {
                                        _22463 = _9947;
                                    }
                                    float _9977 = _9947.y;
                                    float2 _9978 = float2(_9957.x, _9977);
                                    float2 _22464;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _20976 = _9978;
                                        _20976.y = 1.0 - _9977;
                                        _22464 = _20976;
                                    }
                                    else
                                    {
                                        _22464 = _9978;
                                    }
                                    float2 _9995 = float2(_9947.x, _9957.y);
                                    float2 _22465;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _20983 = _9995;
                                        _20983.y = 1.0 - _9957.y;
                                        _22465 = _20983;
                                    }
                                    else
                                    {
                                        _22465 = _9995;
                                    }
                                    float2 _22466;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _20989 = _9957;
                                        _20989.y = 1.0 - _9957.y;
                                        _22466 = _20989;
                                    }
                                    else
                                    {
                                        _22466 = _9957;
                                    }
                                    _22484 = (((gBackdrop4.sample(gLinear, _22463, level(0.0)) * _9934.x) + (gBackdrop4.sample(gLinear, _22464, level(0.0)) * _9937.x)) * _9934.y) + (((gBackdrop4.sample(gLinear, _22465, level(0.0)) * _9934.x) + (gBackdrop4.sample(gLinear, _22466, level(0.0)) * _9937.x)) * _9937.y);
                                }
                                else
                                {
                                    float2 _10095 = (_6707 * _178.gLevel[5].xy) - float2(0.5);
                                    float2 _10097 = floor(_10095);
                                    float2 _10100 = _10095 - _10097;
                                    float2 _10103 = _10100 * _10100;
                                    float2 _10106 = _10103 * _10100;
                                    float2 _10125 = (((_10106 * 3.0) - (_10103 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _10138 = _10106 * 0.16666667163372039794921875;
                                    float2 _10141 = (((((-_10106) + (_10103 * 3.0)) - (_10100 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10125;
                                    float2 _10144 = (((((_10106 * (-3.0)) + (_10103 * 3.0)) + (_10100 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10138;
                                    float2 _10154 = ((_10097 - float2(0.5)) + (_10125 / _10141)) * _178.gLevel[5].zw;
                                    float2 _10164 = ((_10097 + float2(1.5)) + (_10138 / _10144)) * _178.gLevel[5].zw;
                                    float2 _22459;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _20996 = _10154;
                                        _20996.y = 1.0 - _10154.y;
                                        _22459 = _20996;
                                    }
                                    else
                                    {
                                        _22459 = _10154;
                                    }
                                    float _10184 = _10154.y;
                                    float2 _10185 = float2(_10164.x, _10184);
                                    float2 _22460;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21002 = _10185;
                                        _21002.y = 1.0 - _10184;
                                        _22460 = _21002;
                                    }
                                    else
                                    {
                                        _22460 = _10185;
                                    }
                                    float2 _10202 = float2(_10154.x, _10164.y);
                                    float2 _22461;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21009 = _10202;
                                        _21009.y = 1.0 - _10164.y;
                                        _22461 = _21009;
                                    }
                                    else
                                    {
                                        _22461 = _10202;
                                    }
                                    float2 _22462;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21015 = _10164;
                                        _21015.y = 1.0 - _10164.y;
                                        _22462 = _21015;
                                    }
                                    else
                                    {
                                        _22462 = _10164;
                                    }
                                    _22484 = (((gBackdrop5.sample(gLinear, _22459, level(0.0)) * _10141.x) + (gBackdrop5.sample(gLinear, _22460, level(0.0)) * _10144.x)) * _10141.y) + (((gBackdrop5.sample(gLinear, _22461, level(0.0)) * _10141.x) + (gBackdrop5.sample(gLinear, _22462, level(0.0)) * _10144.x)) * _10144.y);
                                }
                                _22483 = _22484;
                            }
                            _22482 = _22483;
                        }
                        _22481 = _22482;
                    }
                    _22480 = _22481;
                }
                float3 _22511;
                if ((_9132 > 0.0199999995529651641845703125) && (_9128 < 5))
                {
                    int _9145 = _9128 + 1;
                    float4 _22506;
                    if (_9145 <= 0)
                    {
                        float2 _22505;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _21020 = _6707;
                            _21020.y = 1.0 - _6707.y;
                            _22505 = _21020;
                        }
                        else
                        {
                            _22505 = _6707;
                        }
                        _22506 = gBackdrop0.sample(gLinear, _22505, level(0.0));
                    }
                    else
                    {
                        float4 _22507;
                        if (_9145 == 1)
                        {
                            float2 _10391 = (_6707 * _178.gLevel[1].xy) - float2(0.5);
                            float2 _10393 = floor(_10391);
                            float2 _10396 = _10391 - _10393;
                            float2 _10399 = _10396 * _10396;
                            float2 _10402 = _10399 * _10396;
                            float2 _10421 = (((_10402 * 3.0) - (_10399 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _10434 = _10402 * 0.16666667163372039794921875;
                            float2 _10437 = (((((-_10402) + (_10399 * 3.0)) - (_10396 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10421;
                            float2 _10440 = (((((_10402 * (-3.0)) + (_10399 * 3.0)) + (_10396 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10434;
                            float2 _10450 = ((_10393 - float2(0.5)) + (_10421 / _10437)) * _178.gLevel[1].zw;
                            float2 _10460 = ((_10393 + float2(1.5)) + (_10434 / _10440)) * _178.gLevel[1].zw;
                            float2 _22501;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21025 = _10450;
                                _21025.y = 1.0 - _10450.y;
                                _22501 = _21025;
                            }
                            else
                            {
                                _22501 = _10450;
                            }
                            float _10480 = _10450.y;
                            float2 _10481 = float2(_10460.x, _10480);
                            float2 _22502;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21031 = _10481;
                                _21031.y = 1.0 - _10480;
                                _22502 = _21031;
                            }
                            else
                            {
                                _22502 = _10481;
                            }
                            float2 _10498 = float2(_10450.x, _10460.y);
                            float2 _22503;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21038 = _10498;
                                _21038.y = 1.0 - _10460.y;
                                _22503 = _21038;
                            }
                            else
                            {
                                _22503 = _10498;
                            }
                            float2 _22504;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21044 = _10460;
                                _21044.y = 1.0 - _10460.y;
                                _22504 = _21044;
                            }
                            else
                            {
                                _22504 = _10460;
                            }
                            _22507 = (((gBackdrop1.sample(gLinear, _22501, level(0.0)) * _10437.x) + (gBackdrop1.sample(gLinear, _22502, level(0.0)) * _10440.x)) * _10437.y) + (((gBackdrop1.sample(gLinear, _22503, level(0.0)) * _10437.x) + (gBackdrop1.sample(gLinear, _22504, level(0.0)) * _10440.x)) * _10440.y);
                        }
                        else
                        {
                            float4 _22508;
                            if (_9145 == 2)
                            {
                                float2 _10598 = (_6707 * _178.gLevel[2].xy) - float2(0.5);
                                float2 _10600 = floor(_10598);
                                float2 _10603 = _10598 - _10600;
                                float2 _10606 = _10603 * _10603;
                                float2 _10609 = _10606 * _10603;
                                float2 _10628 = (((_10609 * 3.0) - (_10606 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _10641 = _10609 * 0.16666667163372039794921875;
                                float2 _10644 = (((((-_10609) + (_10606 * 3.0)) - (_10603 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10628;
                                float2 _10647 = (((((_10609 * (-3.0)) + (_10606 * 3.0)) + (_10603 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10641;
                                float2 _10657 = ((_10600 - float2(0.5)) + (_10628 / _10644)) * _178.gLevel[2].zw;
                                float2 _10667 = ((_10600 + float2(1.5)) + (_10641 / _10647)) * _178.gLevel[2].zw;
                                float2 _22497;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21051 = _10657;
                                    _21051.y = 1.0 - _10657.y;
                                    _22497 = _21051;
                                }
                                else
                                {
                                    _22497 = _10657;
                                }
                                float _10687 = _10657.y;
                                float2 _10688 = float2(_10667.x, _10687);
                                float2 _22498;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21057 = _10688;
                                    _21057.y = 1.0 - _10687;
                                    _22498 = _21057;
                                }
                                else
                                {
                                    _22498 = _10688;
                                }
                                float2 _10705 = float2(_10657.x, _10667.y);
                                float2 _22499;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21064 = _10705;
                                    _21064.y = 1.0 - _10667.y;
                                    _22499 = _21064;
                                }
                                else
                                {
                                    _22499 = _10705;
                                }
                                float2 _22500;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21070 = _10667;
                                    _21070.y = 1.0 - _10667.y;
                                    _22500 = _21070;
                                }
                                else
                                {
                                    _22500 = _10667;
                                }
                                _22508 = (((gBackdrop2.sample(gLinear, _22497, level(0.0)) * _10644.x) + (gBackdrop2.sample(gLinear, _22498, level(0.0)) * _10647.x)) * _10644.y) + (((gBackdrop2.sample(gLinear, _22499, level(0.0)) * _10644.x) + (gBackdrop2.sample(gLinear, _22500, level(0.0)) * _10647.x)) * _10647.y);
                            }
                            else
                            {
                                float4 _22509;
                                if (_9145 == 3)
                                {
                                    float2 _10805 = (_6707 * _178.gLevel[3].xy) - float2(0.5);
                                    float2 _10807 = floor(_10805);
                                    float2 _10810 = _10805 - _10807;
                                    float2 _10813 = _10810 * _10810;
                                    float2 _10816 = _10813 * _10810;
                                    float2 _10835 = (((_10816 * 3.0) - (_10813 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _10848 = _10816 * 0.16666667163372039794921875;
                                    float2 _10851 = (((((-_10816) + (_10813 * 3.0)) - (_10810 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10835;
                                    float2 _10854 = (((((_10816 * (-3.0)) + (_10813 * 3.0)) + (_10810 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10848;
                                    float2 _10864 = ((_10807 - float2(0.5)) + (_10835 / _10851)) * _178.gLevel[3].zw;
                                    float2 _10874 = ((_10807 + float2(1.5)) + (_10848 / _10854)) * _178.gLevel[3].zw;
                                    float2 _22493;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21077 = _10864;
                                        _21077.y = 1.0 - _10864.y;
                                        _22493 = _21077;
                                    }
                                    else
                                    {
                                        _22493 = _10864;
                                    }
                                    float _10894 = _10864.y;
                                    float2 _10895 = float2(_10874.x, _10894);
                                    float2 _22494;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21083 = _10895;
                                        _21083.y = 1.0 - _10894;
                                        _22494 = _21083;
                                    }
                                    else
                                    {
                                        _22494 = _10895;
                                    }
                                    float2 _10912 = float2(_10864.x, _10874.y);
                                    float2 _22495;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21090 = _10912;
                                        _21090.y = 1.0 - _10874.y;
                                        _22495 = _21090;
                                    }
                                    else
                                    {
                                        _22495 = _10912;
                                    }
                                    float2 _22496;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21096 = _10874;
                                        _21096.y = 1.0 - _10874.y;
                                        _22496 = _21096;
                                    }
                                    else
                                    {
                                        _22496 = _10874;
                                    }
                                    _22509 = (((gBackdrop3.sample(gLinear, _22493, level(0.0)) * _10851.x) + (gBackdrop3.sample(gLinear, _22494, level(0.0)) * _10854.x)) * _10851.y) + (((gBackdrop3.sample(gLinear, _22495, level(0.0)) * _10851.x) + (gBackdrop3.sample(gLinear, _22496, level(0.0)) * _10854.x)) * _10854.y);
                                }
                                else
                                {
                                    float4 _22510;
                                    if (_9145 == 4)
                                    {
                                        float2 _11012 = (_6707 * _178.gLevel[4].xy) - float2(0.5);
                                        float2 _11014 = floor(_11012);
                                        float2 _11017 = _11012 - _11014;
                                        float2 _11020 = _11017 * _11017;
                                        float2 _11023 = _11020 * _11017;
                                        float2 _11042 = (((_11023 * 3.0) - (_11020 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _11055 = _11023 * 0.16666667163372039794921875;
                                        float2 _11058 = (((((-_11023) + (_11020 * 3.0)) - (_11017 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11042;
                                        float2 _11061 = (((((_11023 * (-3.0)) + (_11020 * 3.0)) + (_11017 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11055;
                                        float2 _11071 = ((_11014 - float2(0.5)) + (_11042 / _11058)) * _178.gLevel[4].zw;
                                        float2 _11081 = ((_11014 + float2(1.5)) + (_11055 / _11061)) * _178.gLevel[4].zw;
                                        float2 _22489;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21103 = _11071;
                                            _21103.y = 1.0 - _11071.y;
                                            _22489 = _21103;
                                        }
                                        else
                                        {
                                            _22489 = _11071;
                                        }
                                        float _11101 = _11071.y;
                                        float2 _11102 = float2(_11081.x, _11101);
                                        float2 _22490;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21109 = _11102;
                                            _21109.y = 1.0 - _11101;
                                            _22490 = _21109;
                                        }
                                        else
                                        {
                                            _22490 = _11102;
                                        }
                                        float2 _11119 = float2(_11071.x, _11081.y);
                                        float2 _22491;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21116 = _11119;
                                            _21116.y = 1.0 - _11081.y;
                                            _22491 = _21116;
                                        }
                                        else
                                        {
                                            _22491 = _11119;
                                        }
                                        float2 _22492;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21122 = _11081;
                                            _21122.y = 1.0 - _11081.y;
                                            _22492 = _21122;
                                        }
                                        else
                                        {
                                            _22492 = _11081;
                                        }
                                        _22510 = (((gBackdrop4.sample(gLinear, _22489, level(0.0)) * _11058.x) + (gBackdrop4.sample(gLinear, _22490, level(0.0)) * _11061.x)) * _11058.y) + (((gBackdrop4.sample(gLinear, _22491, level(0.0)) * _11058.x) + (gBackdrop4.sample(gLinear, _22492, level(0.0)) * _11061.x)) * _11061.y);
                                    }
                                    else
                                    {
                                        float2 _11219 = (_6707 * _178.gLevel[5].xy) - float2(0.5);
                                        float2 _11221 = floor(_11219);
                                        float2 _11224 = _11219 - _11221;
                                        float2 _11227 = _11224 * _11224;
                                        float2 _11230 = _11227 * _11224;
                                        float2 _11249 = (((_11230 * 3.0) - (_11227 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _11262 = _11230 * 0.16666667163372039794921875;
                                        float2 _11265 = (((((-_11230) + (_11227 * 3.0)) - (_11224 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11249;
                                        float2 _11268 = (((((_11230 * (-3.0)) + (_11227 * 3.0)) + (_11224 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11262;
                                        float2 _11278 = ((_11221 - float2(0.5)) + (_11249 / _11265)) * _178.gLevel[5].zw;
                                        float2 _11288 = ((_11221 + float2(1.5)) + (_11262 / _11268)) * _178.gLevel[5].zw;
                                        float2 _22485;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21129 = _11278;
                                            _21129.y = 1.0 - _11278.y;
                                            _22485 = _21129;
                                        }
                                        else
                                        {
                                            _22485 = _11278;
                                        }
                                        float _11308 = _11278.y;
                                        float2 _11309 = float2(_11288.x, _11308);
                                        float2 _22486;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21135 = _11309;
                                            _21135.y = 1.0 - _11308;
                                            _22486 = _21135;
                                        }
                                        else
                                        {
                                            _22486 = _11309;
                                        }
                                        float2 _11326 = float2(_11278.x, _11288.y);
                                        float2 _22487;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21142 = _11326;
                                            _21142.y = 1.0 - _11288.y;
                                            _22487 = _21142;
                                        }
                                        else
                                        {
                                            _22487 = _11326;
                                        }
                                        float2 _22488;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21148 = _11288;
                                            _21148.y = 1.0 - _11288.y;
                                            _22488 = _21148;
                                        }
                                        else
                                        {
                                            _22488 = _11288;
                                        }
                                        _22510 = (((gBackdrop5.sample(gLinear, _22485, level(0.0)) * _11265.x) + (gBackdrop5.sample(gLinear, _22486, level(0.0)) * _11268.x)) * _11265.y) + (((gBackdrop5.sample(gLinear, _22487, level(0.0)) * _11265.x) + (gBackdrop5.sample(gLinear, _22488, level(0.0)) * _11268.x)) * _11268.y);
                                    }
                                    _22509 = _22510;
                                }
                                _22508 = _22509;
                            }
                            _22507 = _22508;
                        }
                        _22506 = _22507;
                    }
                    _22511 = mix(_22480.xyz, _22506.xyz, float3(_9132));
                }
                else
                {
                    _22511 = _22480.xyz;
                }
                float2 _6714 = _6663 + _6675;
                float _11416 = fast::clamp(log2(fast::max(_6577, 1.0)) - 1.0, 0.0, 5.0);
                int _11419 = int(floor(_11416));
                float _11423 = _11416 - float(_11419);
                float4 _22586;
                if (_11419 <= 0)
                {
                    float2 _22585;
                    if (_178.gConv.x > 0.5)
                    {
                        float2 _21155 = _6714;
                        _21155.y = 1.0 - _6714.y;
                        _22585 = _21155;
                    }
                    else
                    {
                        _22585 = _6714;
                    }
                    _22586 = gBackdrop0.sample(gLinear, _22585, level(0.0));
                }
                else
                {
                    float4 _22587;
                    if (_11419 == 1)
                    {
                        float2 _11558 = (_6714 * _178.gLevel[1].xy) - float2(0.5);
                        float2 _11560 = floor(_11558);
                        float2 _11563 = _11558 - _11560;
                        float2 _11566 = _11563 * _11563;
                        float2 _11569 = _11566 * _11563;
                        float2 _11588 = (((_11569 * 3.0) - (_11566 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _11601 = _11569 * 0.16666667163372039794921875;
                        float2 _11604 = (((((-_11569) + (_11566 * 3.0)) - (_11563 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11588;
                        float2 _11607 = (((((_11569 * (-3.0)) + (_11566 * 3.0)) + (_11563 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11601;
                        float2 _11617 = ((_11560 - float2(0.5)) + (_11588 / _11604)) * _178.gLevel[1].zw;
                        float2 _11627 = ((_11560 + float2(1.5)) + (_11601 / _11607)) * _178.gLevel[1].zw;
                        float2 _22581;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _21160 = _11617;
                            _21160.y = 1.0 - _11617.y;
                            _22581 = _21160;
                        }
                        else
                        {
                            _22581 = _11617;
                        }
                        float _11647 = _11617.y;
                        float2 _11648 = float2(_11627.x, _11647);
                        float2 _22582;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _21166 = _11648;
                            _21166.y = 1.0 - _11647;
                            _22582 = _21166;
                        }
                        else
                        {
                            _22582 = _11648;
                        }
                        float2 _11665 = float2(_11617.x, _11627.y);
                        float2 _22583;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _21173 = _11665;
                            _21173.y = 1.0 - _11627.y;
                            _22583 = _21173;
                        }
                        else
                        {
                            _22583 = _11665;
                        }
                        float2 _22584;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _21179 = _11627;
                            _21179.y = 1.0 - _11627.y;
                            _22584 = _21179;
                        }
                        else
                        {
                            _22584 = _11627;
                        }
                        _22587 = (((gBackdrop1.sample(gLinear, _22581, level(0.0)) * _11604.x) + (gBackdrop1.sample(gLinear, _22582, level(0.0)) * _11607.x)) * _11604.y) + (((gBackdrop1.sample(gLinear, _22583, level(0.0)) * _11604.x) + (gBackdrop1.sample(gLinear, _22584, level(0.0)) * _11607.x)) * _11607.y);
                    }
                    else
                    {
                        float4 _22588;
                        if (_11419 == 2)
                        {
                            float2 _11765 = (_6714 * _178.gLevel[2].xy) - float2(0.5);
                            float2 _11767 = floor(_11765);
                            float2 _11770 = _11765 - _11767;
                            float2 _11773 = _11770 * _11770;
                            float2 _11776 = _11773 * _11770;
                            float2 _11795 = (((_11776 * 3.0) - (_11773 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _11808 = _11776 * 0.16666667163372039794921875;
                            float2 _11811 = (((((-_11776) + (_11773 * 3.0)) - (_11770 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11795;
                            float2 _11814 = (((((_11776 * (-3.0)) + (_11773 * 3.0)) + (_11770 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11808;
                            float2 _11824 = ((_11767 - float2(0.5)) + (_11795 / _11811)) * _178.gLevel[2].zw;
                            float2 _11834 = ((_11767 + float2(1.5)) + (_11808 / _11814)) * _178.gLevel[2].zw;
                            float2 _22577;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21186 = _11824;
                                _21186.y = 1.0 - _11824.y;
                                _22577 = _21186;
                            }
                            else
                            {
                                _22577 = _11824;
                            }
                            float _11854 = _11824.y;
                            float2 _11855 = float2(_11834.x, _11854);
                            float2 _22578;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21192 = _11855;
                                _21192.y = 1.0 - _11854;
                                _22578 = _21192;
                            }
                            else
                            {
                                _22578 = _11855;
                            }
                            float2 _11872 = float2(_11824.x, _11834.y);
                            float2 _22579;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21199 = _11872;
                                _21199.y = 1.0 - _11834.y;
                                _22579 = _21199;
                            }
                            else
                            {
                                _22579 = _11872;
                            }
                            float2 _22580;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21205 = _11834;
                                _21205.y = 1.0 - _11834.y;
                                _22580 = _21205;
                            }
                            else
                            {
                                _22580 = _11834;
                            }
                            _22588 = (((gBackdrop2.sample(gLinear, _22577, level(0.0)) * _11811.x) + (gBackdrop2.sample(gLinear, _22578, level(0.0)) * _11814.x)) * _11811.y) + (((gBackdrop2.sample(gLinear, _22579, level(0.0)) * _11811.x) + (gBackdrop2.sample(gLinear, _22580, level(0.0)) * _11814.x)) * _11814.y);
                        }
                        else
                        {
                            float4 _22589;
                            if (_11419 == 3)
                            {
                                float2 _11972 = (_6714 * _178.gLevel[3].xy) - float2(0.5);
                                float2 _11974 = floor(_11972);
                                float2 _11977 = _11972 - _11974;
                                float2 _11980 = _11977 * _11977;
                                float2 _11983 = _11980 * _11977;
                                float2 _12002 = (((_11983 * 3.0) - (_11980 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _12015 = _11983 * 0.16666667163372039794921875;
                                float2 _12018 = (((((-_11983) + (_11980 * 3.0)) - (_11977 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12002;
                                float2 _12021 = (((((_11983 * (-3.0)) + (_11980 * 3.0)) + (_11977 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12015;
                                float2 _12031 = ((_11974 - float2(0.5)) + (_12002 / _12018)) * _178.gLevel[3].zw;
                                float2 _12041 = ((_11974 + float2(1.5)) + (_12015 / _12021)) * _178.gLevel[3].zw;
                                float2 _22573;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21212 = _12031;
                                    _21212.y = 1.0 - _12031.y;
                                    _22573 = _21212;
                                }
                                else
                                {
                                    _22573 = _12031;
                                }
                                float _12061 = _12031.y;
                                float2 _12062 = float2(_12041.x, _12061);
                                float2 _22574;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21218 = _12062;
                                    _21218.y = 1.0 - _12061;
                                    _22574 = _21218;
                                }
                                else
                                {
                                    _22574 = _12062;
                                }
                                float2 _12079 = float2(_12031.x, _12041.y);
                                float2 _22575;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21225 = _12079;
                                    _21225.y = 1.0 - _12041.y;
                                    _22575 = _21225;
                                }
                                else
                                {
                                    _22575 = _12079;
                                }
                                float2 _22576;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21231 = _12041;
                                    _21231.y = 1.0 - _12041.y;
                                    _22576 = _21231;
                                }
                                else
                                {
                                    _22576 = _12041;
                                }
                                _22589 = (((gBackdrop3.sample(gLinear, _22573, level(0.0)) * _12018.x) + (gBackdrop3.sample(gLinear, _22574, level(0.0)) * _12021.x)) * _12018.y) + (((gBackdrop3.sample(gLinear, _22575, level(0.0)) * _12018.x) + (gBackdrop3.sample(gLinear, _22576, level(0.0)) * _12021.x)) * _12021.y);
                            }
                            else
                            {
                                float4 _22590;
                                if (_11419 == 4)
                                {
                                    float2 _12179 = (_6714 * _178.gLevel[4].xy) - float2(0.5);
                                    float2 _12181 = floor(_12179);
                                    float2 _12184 = _12179 - _12181;
                                    float2 _12187 = _12184 * _12184;
                                    float2 _12190 = _12187 * _12184;
                                    float2 _12209 = (((_12190 * 3.0) - (_12187 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _12222 = _12190 * 0.16666667163372039794921875;
                                    float2 _12225 = (((((-_12190) + (_12187 * 3.0)) - (_12184 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12209;
                                    float2 _12228 = (((((_12190 * (-3.0)) + (_12187 * 3.0)) + (_12184 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12222;
                                    float2 _12238 = ((_12181 - float2(0.5)) + (_12209 / _12225)) * _178.gLevel[4].zw;
                                    float2 _12248 = ((_12181 + float2(1.5)) + (_12222 / _12228)) * _178.gLevel[4].zw;
                                    float2 _22569;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21238 = _12238;
                                        _21238.y = 1.0 - _12238.y;
                                        _22569 = _21238;
                                    }
                                    else
                                    {
                                        _22569 = _12238;
                                    }
                                    float _12268 = _12238.y;
                                    float2 _12269 = float2(_12248.x, _12268);
                                    float2 _22570;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21244 = _12269;
                                        _21244.y = 1.0 - _12268;
                                        _22570 = _21244;
                                    }
                                    else
                                    {
                                        _22570 = _12269;
                                    }
                                    float2 _12286 = float2(_12238.x, _12248.y);
                                    float2 _22571;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21251 = _12286;
                                        _21251.y = 1.0 - _12248.y;
                                        _22571 = _21251;
                                    }
                                    else
                                    {
                                        _22571 = _12286;
                                    }
                                    float2 _22572;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21257 = _12248;
                                        _21257.y = 1.0 - _12248.y;
                                        _22572 = _21257;
                                    }
                                    else
                                    {
                                        _22572 = _12248;
                                    }
                                    _22590 = (((gBackdrop4.sample(gLinear, _22569, level(0.0)) * _12225.x) + (gBackdrop4.sample(gLinear, _22570, level(0.0)) * _12228.x)) * _12225.y) + (((gBackdrop4.sample(gLinear, _22571, level(0.0)) * _12225.x) + (gBackdrop4.sample(gLinear, _22572, level(0.0)) * _12228.x)) * _12228.y);
                                }
                                else
                                {
                                    float2 _12386 = (_6714 * _178.gLevel[5].xy) - float2(0.5);
                                    float2 _12388 = floor(_12386);
                                    float2 _12391 = _12386 - _12388;
                                    float2 _12394 = _12391 * _12391;
                                    float2 _12397 = _12394 * _12391;
                                    float2 _12416 = (((_12397 * 3.0) - (_12394 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _12429 = _12397 * 0.16666667163372039794921875;
                                    float2 _12432 = (((((-_12397) + (_12394 * 3.0)) - (_12391 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12416;
                                    float2 _12435 = (((((_12397 * (-3.0)) + (_12394 * 3.0)) + (_12391 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12429;
                                    float2 _12445 = ((_12388 - float2(0.5)) + (_12416 / _12432)) * _178.gLevel[5].zw;
                                    float2 _12455 = ((_12388 + float2(1.5)) + (_12429 / _12435)) * _178.gLevel[5].zw;
                                    float2 _22565;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21264 = _12445;
                                        _21264.y = 1.0 - _12445.y;
                                        _22565 = _21264;
                                    }
                                    else
                                    {
                                        _22565 = _12445;
                                    }
                                    float _12475 = _12445.y;
                                    float2 _12476 = float2(_12455.x, _12475);
                                    float2 _22566;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21270 = _12476;
                                        _21270.y = 1.0 - _12475;
                                        _22566 = _21270;
                                    }
                                    else
                                    {
                                        _22566 = _12476;
                                    }
                                    float2 _12493 = float2(_12445.x, _12455.y);
                                    float2 _22567;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21277 = _12493;
                                        _21277.y = 1.0 - _12455.y;
                                        _22567 = _21277;
                                    }
                                    else
                                    {
                                        _22567 = _12493;
                                    }
                                    float2 _22568;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21283 = _12455;
                                        _21283.y = 1.0 - _12455.y;
                                        _22568 = _21283;
                                    }
                                    else
                                    {
                                        _22568 = _12455;
                                    }
                                    _22590 = (((gBackdrop5.sample(gLinear, _22565, level(0.0)) * _12432.x) + (gBackdrop5.sample(gLinear, _22566, level(0.0)) * _12435.x)) * _12432.y) + (((gBackdrop5.sample(gLinear, _22567, level(0.0)) * _12432.x) + (gBackdrop5.sample(gLinear, _22568, level(0.0)) * _12435.x)) * _12435.y);
                                }
                                _22589 = _22590;
                            }
                            _22588 = _22589;
                        }
                        _22587 = _22588;
                    }
                    _22586 = _22587;
                }
                float3 _22617;
                if ((_11423 > 0.0199999995529651641845703125) && (_11419 < 5))
                {
                    int _11436 = _11419 + 1;
                    float4 _22612;
                    if (_11436 <= 0)
                    {
                        float2 _22611;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _21288 = _6714;
                            _21288.y = 1.0 - _6714.y;
                            _22611 = _21288;
                        }
                        else
                        {
                            _22611 = _6714;
                        }
                        _22612 = gBackdrop0.sample(gLinear, _22611, level(0.0));
                    }
                    else
                    {
                        float4 _22613;
                        if (_11436 == 1)
                        {
                            float2 _12682 = (_6714 * _178.gLevel[1].xy) - float2(0.5);
                            float2 _12684 = floor(_12682);
                            float2 _12687 = _12682 - _12684;
                            float2 _12690 = _12687 * _12687;
                            float2 _12693 = _12690 * _12687;
                            float2 _12712 = (((_12693 * 3.0) - (_12690 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _12725 = _12693 * 0.16666667163372039794921875;
                            float2 _12728 = (((((-_12693) + (_12690 * 3.0)) - (_12687 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12712;
                            float2 _12731 = (((((_12693 * (-3.0)) + (_12690 * 3.0)) + (_12687 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12725;
                            float2 _12741 = ((_12684 - float2(0.5)) + (_12712 / _12728)) * _178.gLevel[1].zw;
                            float2 _12751 = ((_12684 + float2(1.5)) + (_12725 / _12731)) * _178.gLevel[1].zw;
                            float2 _22607;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21293 = _12741;
                                _21293.y = 1.0 - _12741.y;
                                _22607 = _21293;
                            }
                            else
                            {
                                _22607 = _12741;
                            }
                            float _12771 = _12741.y;
                            float2 _12772 = float2(_12751.x, _12771);
                            float2 _22608;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21299 = _12772;
                                _21299.y = 1.0 - _12771;
                                _22608 = _21299;
                            }
                            else
                            {
                                _22608 = _12772;
                            }
                            float2 _12789 = float2(_12741.x, _12751.y);
                            float2 _22609;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21306 = _12789;
                                _21306.y = 1.0 - _12751.y;
                                _22609 = _21306;
                            }
                            else
                            {
                                _22609 = _12789;
                            }
                            float2 _22610;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21312 = _12751;
                                _21312.y = 1.0 - _12751.y;
                                _22610 = _21312;
                            }
                            else
                            {
                                _22610 = _12751;
                            }
                            _22613 = (((gBackdrop1.sample(gLinear, _22607, level(0.0)) * _12728.x) + (gBackdrop1.sample(gLinear, _22608, level(0.0)) * _12731.x)) * _12728.y) + (((gBackdrop1.sample(gLinear, _22609, level(0.0)) * _12728.x) + (gBackdrop1.sample(gLinear, _22610, level(0.0)) * _12731.x)) * _12731.y);
                        }
                        else
                        {
                            float4 _22614;
                            if (_11436 == 2)
                            {
                                float2 _12889 = (_6714 * _178.gLevel[2].xy) - float2(0.5);
                                float2 _12891 = floor(_12889);
                                float2 _12894 = _12889 - _12891;
                                float2 _12897 = _12894 * _12894;
                                float2 _12900 = _12897 * _12894;
                                float2 _12919 = (((_12900 * 3.0) - (_12897 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _12932 = _12900 * 0.16666667163372039794921875;
                                float2 _12935 = (((((-_12900) + (_12897 * 3.0)) - (_12894 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12919;
                                float2 _12938 = (((((_12900 * (-3.0)) + (_12897 * 3.0)) + (_12894 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12932;
                                float2 _12948 = ((_12891 - float2(0.5)) + (_12919 / _12935)) * _178.gLevel[2].zw;
                                float2 _12958 = ((_12891 + float2(1.5)) + (_12932 / _12938)) * _178.gLevel[2].zw;
                                float2 _22603;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21319 = _12948;
                                    _21319.y = 1.0 - _12948.y;
                                    _22603 = _21319;
                                }
                                else
                                {
                                    _22603 = _12948;
                                }
                                float _12978 = _12948.y;
                                float2 _12979 = float2(_12958.x, _12978);
                                float2 _22604;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21325 = _12979;
                                    _21325.y = 1.0 - _12978;
                                    _22604 = _21325;
                                }
                                else
                                {
                                    _22604 = _12979;
                                }
                                float2 _12996 = float2(_12948.x, _12958.y);
                                float2 _22605;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21332 = _12996;
                                    _21332.y = 1.0 - _12958.y;
                                    _22605 = _21332;
                                }
                                else
                                {
                                    _22605 = _12996;
                                }
                                float2 _22606;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21338 = _12958;
                                    _21338.y = 1.0 - _12958.y;
                                    _22606 = _21338;
                                }
                                else
                                {
                                    _22606 = _12958;
                                }
                                _22614 = (((gBackdrop2.sample(gLinear, _22603, level(0.0)) * _12935.x) + (gBackdrop2.sample(gLinear, _22604, level(0.0)) * _12938.x)) * _12935.y) + (((gBackdrop2.sample(gLinear, _22605, level(0.0)) * _12935.x) + (gBackdrop2.sample(gLinear, _22606, level(0.0)) * _12938.x)) * _12938.y);
                            }
                            else
                            {
                                float4 _22615;
                                if (_11436 == 3)
                                {
                                    float2 _13096 = (_6714 * _178.gLevel[3].xy) - float2(0.5);
                                    float2 _13098 = floor(_13096);
                                    float2 _13101 = _13096 - _13098;
                                    float2 _13104 = _13101 * _13101;
                                    float2 _13107 = _13104 * _13101;
                                    float2 _13126 = (((_13107 * 3.0) - (_13104 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _13139 = _13107 * 0.16666667163372039794921875;
                                    float2 _13142 = (((((-_13107) + (_13104 * 3.0)) - (_13101 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13126;
                                    float2 _13145 = (((((_13107 * (-3.0)) + (_13104 * 3.0)) + (_13101 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13139;
                                    float2 _13155 = ((_13098 - float2(0.5)) + (_13126 / _13142)) * _178.gLevel[3].zw;
                                    float2 _13165 = ((_13098 + float2(1.5)) + (_13139 / _13145)) * _178.gLevel[3].zw;
                                    float2 _22599;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21345 = _13155;
                                        _21345.y = 1.0 - _13155.y;
                                        _22599 = _21345;
                                    }
                                    else
                                    {
                                        _22599 = _13155;
                                    }
                                    float _13185 = _13155.y;
                                    float2 _13186 = float2(_13165.x, _13185);
                                    float2 _22600;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21351 = _13186;
                                        _21351.y = 1.0 - _13185;
                                        _22600 = _21351;
                                    }
                                    else
                                    {
                                        _22600 = _13186;
                                    }
                                    float2 _13203 = float2(_13155.x, _13165.y);
                                    float2 _22601;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21358 = _13203;
                                        _21358.y = 1.0 - _13165.y;
                                        _22601 = _21358;
                                    }
                                    else
                                    {
                                        _22601 = _13203;
                                    }
                                    float2 _22602;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21364 = _13165;
                                        _21364.y = 1.0 - _13165.y;
                                        _22602 = _21364;
                                    }
                                    else
                                    {
                                        _22602 = _13165;
                                    }
                                    _22615 = (((gBackdrop3.sample(gLinear, _22599, level(0.0)) * _13142.x) + (gBackdrop3.sample(gLinear, _22600, level(0.0)) * _13145.x)) * _13142.y) + (((gBackdrop3.sample(gLinear, _22601, level(0.0)) * _13142.x) + (gBackdrop3.sample(gLinear, _22602, level(0.0)) * _13145.x)) * _13145.y);
                                }
                                else
                                {
                                    float4 _22616;
                                    if (_11436 == 4)
                                    {
                                        float2 _13303 = (_6714 * _178.gLevel[4].xy) - float2(0.5);
                                        float2 _13305 = floor(_13303);
                                        float2 _13308 = _13303 - _13305;
                                        float2 _13311 = _13308 * _13308;
                                        float2 _13314 = _13311 * _13308;
                                        float2 _13333 = (((_13314 * 3.0) - (_13311 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _13346 = _13314 * 0.16666667163372039794921875;
                                        float2 _13349 = (((((-_13314) + (_13311 * 3.0)) - (_13308 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13333;
                                        float2 _13352 = (((((_13314 * (-3.0)) + (_13311 * 3.0)) + (_13308 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13346;
                                        float2 _13362 = ((_13305 - float2(0.5)) + (_13333 / _13349)) * _178.gLevel[4].zw;
                                        float2 _13372 = ((_13305 + float2(1.5)) + (_13346 / _13352)) * _178.gLevel[4].zw;
                                        float2 _22595;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21371 = _13362;
                                            _21371.y = 1.0 - _13362.y;
                                            _22595 = _21371;
                                        }
                                        else
                                        {
                                            _22595 = _13362;
                                        }
                                        float _13392 = _13362.y;
                                        float2 _13393 = float2(_13372.x, _13392);
                                        float2 _22596;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21377 = _13393;
                                            _21377.y = 1.0 - _13392;
                                            _22596 = _21377;
                                        }
                                        else
                                        {
                                            _22596 = _13393;
                                        }
                                        float2 _13410 = float2(_13362.x, _13372.y);
                                        float2 _22597;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21384 = _13410;
                                            _21384.y = 1.0 - _13372.y;
                                            _22597 = _21384;
                                        }
                                        else
                                        {
                                            _22597 = _13410;
                                        }
                                        float2 _22598;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21390 = _13372;
                                            _21390.y = 1.0 - _13372.y;
                                            _22598 = _21390;
                                        }
                                        else
                                        {
                                            _22598 = _13372;
                                        }
                                        _22616 = (((gBackdrop4.sample(gLinear, _22595, level(0.0)) * _13349.x) + (gBackdrop4.sample(gLinear, _22596, level(0.0)) * _13352.x)) * _13349.y) + (((gBackdrop4.sample(gLinear, _22597, level(0.0)) * _13349.x) + (gBackdrop4.sample(gLinear, _22598, level(0.0)) * _13352.x)) * _13352.y);
                                    }
                                    else
                                    {
                                        float2 _13510 = (_6714 * _178.gLevel[5].xy) - float2(0.5);
                                        float2 _13512 = floor(_13510);
                                        float2 _13515 = _13510 - _13512;
                                        float2 _13518 = _13515 * _13515;
                                        float2 _13521 = _13518 * _13515;
                                        float2 _13540 = (((_13521 * 3.0) - (_13518 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _13553 = _13521 * 0.16666667163372039794921875;
                                        float2 _13556 = (((((-_13521) + (_13518 * 3.0)) - (_13515 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13540;
                                        float2 _13559 = (((((_13521 * (-3.0)) + (_13518 * 3.0)) + (_13515 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13553;
                                        float2 _13569 = ((_13512 - float2(0.5)) + (_13540 / _13556)) * _178.gLevel[5].zw;
                                        float2 _13579 = ((_13512 + float2(1.5)) + (_13553 / _13559)) * _178.gLevel[5].zw;
                                        float2 _22591;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21397 = _13569;
                                            _21397.y = 1.0 - _13569.y;
                                            _22591 = _21397;
                                        }
                                        else
                                        {
                                            _22591 = _13569;
                                        }
                                        float _13599 = _13569.y;
                                        float2 _13600 = float2(_13579.x, _13599);
                                        float2 _22592;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21403 = _13600;
                                            _21403.y = 1.0 - _13599;
                                            _22592 = _21403;
                                        }
                                        else
                                        {
                                            _22592 = _13600;
                                        }
                                        float2 _13617 = float2(_13569.x, _13579.y);
                                        float2 _22593;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21410 = _13617;
                                            _21410.y = 1.0 - _13579.y;
                                            _22593 = _21410;
                                        }
                                        else
                                        {
                                            _22593 = _13617;
                                        }
                                        float2 _22594;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21416 = _13579;
                                            _21416.y = 1.0 - _13579.y;
                                            _22594 = _21416;
                                        }
                                        else
                                        {
                                            _22594 = _13579;
                                        }
                                        _22616 = (((gBackdrop5.sample(gLinear, _22591, level(0.0)) * _13556.x) + (gBackdrop5.sample(gLinear, _22592, level(0.0)) * _13559.x)) * _13556.y) + (((gBackdrop5.sample(gLinear, _22593, level(0.0)) * _13556.x) + (gBackdrop5.sample(gLinear, _22594, level(0.0)) * _13559.x)) * _13559.y);
                                    }
                                    _22615 = _22616;
                                }
                                _22614 = _22615;
                            }
                            _22613 = _22614;
                        }
                        _22612 = _22613;
                    }
                    _22617 = mix(_22586.xyz, _22612.xyz, float3(_11423));
                }
                else
                {
                    _22617 = _22586.xyz;
                }
                float2 _6725 = (_6663 + _6675) + (_6678 * _6700);
                float _13707 = fast::clamp(log2(fast::max(_6577, 1.0)) - 1.0, 0.0, 5.0);
                int _13710 = int(floor(_13707));
                float _13714 = _13707 - float(_13710);
                float4 _22692;
                if (_13710 <= 0)
                {
                    float2 _22691;
                    if (_178.gConv.x > 0.5)
                    {
                        float2 _21423 = _6725;
                        _21423.y = 1.0 - _6725.y;
                        _22691 = _21423;
                    }
                    else
                    {
                        _22691 = _6725;
                    }
                    _22692 = gBackdrop0.sample(gLinear, _22691, level(0.0));
                }
                else
                {
                    float4 _22693;
                    if (_13710 == 1)
                    {
                        float2 _13849 = (_6725 * _178.gLevel[1].xy) - float2(0.5);
                        float2 _13851 = floor(_13849);
                        float2 _13854 = _13849 - _13851;
                        float2 _13857 = _13854 * _13854;
                        float2 _13860 = _13857 * _13854;
                        float2 _13879 = (((_13860 * 3.0) - (_13857 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _13892 = _13860 * 0.16666667163372039794921875;
                        float2 _13895 = (((((-_13860) + (_13857 * 3.0)) - (_13854 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13879;
                        float2 _13898 = (((((_13860 * (-3.0)) + (_13857 * 3.0)) + (_13854 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13892;
                        float2 _13908 = ((_13851 - float2(0.5)) + (_13879 / _13895)) * _178.gLevel[1].zw;
                        float2 _13918 = ((_13851 + float2(1.5)) + (_13892 / _13898)) * _178.gLevel[1].zw;
                        float2 _22687;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _21428 = _13908;
                            _21428.y = 1.0 - _13908.y;
                            _22687 = _21428;
                        }
                        else
                        {
                            _22687 = _13908;
                        }
                        float _13938 = _13908.y;
                        float2 _13939 = float2(_13918.x, _13938);
                        float2 _22688;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _21434 = _13939;
                            _21434.y = 1.0 - _13938;
                            _22688 = _21434;
                        }
                        else
                        {
                            _22688 = _13939;
                        }
                        float2 _13956 = float2(_13908.x, _13918.y);
                        float2 _22689;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _21441 = _13956;
                            _21441.y = 1.0 - _13918.y;
                            _22689 = _21441;
                        }
                        else
                        {
                            _22689 = _13956;
                        }
                        float2 _22690;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _21447 = _13918;
                            _21447.y = 1.0 - _13918.y;
                            _22690 = _21447;
                        }
                        else
                        {
                            _22690 = _13918;
                        }
                        _22693 = (((gBackdrop1.sample(gLinear, _22687, level(0.0)) * _13895.x) + (gBackdrop1.sample(gLinear, _22688, level(0.0)) * _13898.x)) * _13895.y) + (((gBackdrop1.sample(gLinear, _22689, level(0.0)) * _13895.x) + (gBackdrop1.sample(gLinear, _22690, level(0.0)) * _13898.x)) * _13898.y);
                    }
                    else
                    {
                        float4 _22694;
                        if (_13710 == 2)
                        {
                            float2 _14056 = (_6725 * _178.gLevel[2].xy) - float2(0.5);
                            float2 _14058 = floor(_14056);
                            float2 _14061 = _14056 - _14058;
                            float2 _14064 = _14061 * _14061;
                            float2 _14067 = _14064 * _14061;
                            float2 _14086 = (((_14067 * 3.0) - (_14064 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _14099 = _14067 * 0.16666667163372039794921875;
                            float2 _14102 = (((((-_14067) + (_14064 * 3.0)) - (_14061 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14086;
                            float2 _14105 = (((((_14067 * (-3.0)) + (_14064 * 3.0)) + (_14061 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14099;
                            float2 _14115 = ((_14058 - float2(0.5)) + (_14086 / _14102)) * _178.gLevel[2].zw;
                            float2 _14125 = ((_14058 + float2(1.5)) + (_14099 / _14105)) * _178.gLevel[2].zw;
                            float2 _22683;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21454 = _14115;
                                _21454.y = 1.0 - _14115.y;
                                _22683 = _21454;
                            }
                            else
                            {
                                _22683 = _14115;
                            }
                            float _14145 = _14115.y;
                            float2 _14146 = float2(_14125.x, _14145);
                            float2 _22684;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21460 = _14146;
                                _21460.y = 1.0 - _14145;
                                _22684 = _21460;
                            }
                            else
                            {
                                _22684 = _14146;
                            }
                            float2 _14163 = float2(_14115.x, _14125.y);
                            float2 _22685;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21467 = _14163;
                                _21467.y = 1.0 - _14125.y;
                                _22685 = _21467;
                            }
                            else
                            {
                                _22685 = _14163;
                            }
                            float2 _22686;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21473 = _14125;
                                _21473.y = 1.0 - _14125.y;
                                _22686 = _21473;
                            }
                            else
                            {
                                _22686 = _14125;
                            }
                            _22694 = (((gBackdrop2.sample(gLinear, _22683, level(0.0)) * _14102.x) + (gBackdrop2.sample(gLinear, _22684, level(0.0)) * _14105.x)) * _14102.y) + (((gBackdrop2.sample(gLinear, _22685, level(0.0)) * _14102.x) + (gBackdrop2.sample(gLinear, _22686, level(0.0)) * _14105.x)) * _14105.y);
                        }
                        else
                        {
                            float4 _22695;
                            if (_13710 == 3)
                            {
                                float2 _14263 = (_6725 * _178.gLevel[3].xy) - float2(0.5);
                                float2 _14265 = floor(_14263);
                                float2 _14268 = _14263 - _14265;
                                float2 _14271 = _14268 * _14268;
                                float2 _14274 = _14271 * _14268;
                                float2 _14293 = (((_14274 * 3.0) - (_14271 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _14306 = _14274 * 0.16666667163372039794921875;
                                float2 _14309 = (((((-_14274) + (_14271 * 3.0)) - (_14268 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14293;
                                float2 _14312 = (((((_14274 * (-3.0)) + (_14271 * 3.0)) + (_14268 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14306;
                                float2 _14322 = ((_14265 - float2(0.5)) + (_14293 / _14309)) * _178.gLevel[3].zw;
                                float2 _14332 = ((_14265 + float2(1.5)) + (_14306 / _14312)) * _178.gLevel[3].zw;
                                float2 _22679;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21480 = _14322;
                                    _21480.y = 1.0 - _14322.y;
                                    _22679 = _21480;
                                }
                                else
                                {
                                    _22679 = _14322;
                                }
                                float _14352 = _14322.y;
                                float2 _14353 = float2(_14332.x, _14352);
                                float2 _22680;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21486 = _14353;
                                    _21486.y = 1.0 - _14352;
                                    _22680 = _21486;
                                }
                                else
                                {
                                    _22680 = _14353;
                                }
                                float2 _14370 = float2(_14322.x, _14332.y);
                                float2 _22681;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21493 = _14370;
                                    _21493.y = 1.0 - _14332.y;
                                    _22681 = _21493;
                                }
                                else
                                {
                                    _22681 = _14370;
                                }
                                float2 _22682;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21499 = _14332;
                                    _21499.y = 1.0 - _14332.y;
                                    _22682 = _21499;
                                }
                                else
                                {
                                    _22682 = _14332;
                                }
                                _22695 = (((gBackdrop3.sample(gLinear, _22679, level(0.0)) * _14309.x) + (gBackdrop3.sample(gLinear, _22680, level(0.0)) * _14312.x)) * _14309.y) + (((gBackdrop3.sample(gLinear, _22681, level(0.0)) * _14309.x) + (gBackdrop3.sample(gLinear, _22682, level(0.0)) * _14312.x)) * _14312.y);
                            }
                            else
                            {
                                float4 _22696;
                                if (_13710 == 4)
                                {
                                    float2 _14470 = (_6725 * _178.gLevel[4].xy) - float2(0.5);
                                    float2 _14472 = floor(_14470);
                                    float2 _14475 = _14470 - _14472;
                                    float2 _14478 = _14475 * _14475;
                                    float2 _14481 = _14478 * _14475;
                                    float2 _14500 = (((_14481 * 3.0) - (_14478 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _14513 = _14481 * 0.16666667163372039794921875;
                                    float2 _14516 = (((((-_14481) + (_14478 * 3.0)) - (_14475 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14500;
                                    float2 _14519 = (((((_14481 * (-3.0)) + (_14478 * 3.0)) + (_14475 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14513;
                                    float2 _14529 = ((_14472 - float2(0.5)) + (_14500 / _14516)) * _178.gLevel[4].zw;
                                    float2 _14539 = ((_14472 + float2(1.5)) + (_14513 / _14519)) * _178.gLevel[4].zw;
                                    float2 _22675;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21506 = _14529;
                                        _21506.y = 1.0 - _14529.y;
                                        _22675 = _21506;
                                    }
                                    else
                                    {
                                        _22675 = _14529;
                                    }
                                    float _14559 = _14529.y;
                                    float2 _14560 = float2(_14539.x, _14559);
                                    float2 _22676;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21512 = _14560;
                                        _21512.y = 1.0 - _14559;
                                        _22676 = _21512;
                                    }
                                    else
                                    {
                                        _22676 = _14560;
                                    }
                                    float2 _14577 = float2(_14529.x, _14539.y);
                                    float2 _22677;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21519 = _14577;
                                        _21519.y = 1.0 - _14539.y;
                                        _22677 = _21519;
                                    }
                                    else
                                    {
                                        _22677 = _14577;
                                    }
                                    float2 _22678;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21525 = _14539;
                                        _21525.y = 1.0 - _14539.y;
                                        _22678 = _21525;
                                    }
                                    else
                                    {
                                        _22678 = _14539;
                                    }
                                    _22696 = (((gBackdrop4.sample(gLinear, _22675, level(0.0)) * _14516.x) + (gBackdrop4.sample(gLinear, _22676, level(0.0)) * _14519.x)) * _14516.y) + (((gBackdrop4.sample(gLinear, _22677, level(0.0)) * _14516.x) + (gBackdrop4.sample(gLinear, _22678, level(0.0)) * _14519.x)) * _14519.y);
                                }
                                else
                                {
                                    float2 _14677 = (_6725 * _178.gLevel[5].xy) - float2(0.5);
                                    float2 _14679 = floor(_14677);
                                    float2 _14682 = _14677 - _14679;
                                    float2 _14685 = _14682 * _14682;
                                    float2 _14688 = _14685 * _14682;
                                    float2 _14707 = (((_14688 * 3.0) - (_14685 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _14720 = _14688 * 0.16666667163372039794921875;
                                    float2 _14723 = (((((-_14688) + (_14685 * 3.0)) - (_14682 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14707;
                                    float2 _14726 = (((((_14688 * (-3.0)) + (_14685 * 3.0)) + (_14682 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14720;
                                    float2 _14736 = ((_14679 - float2(0.5)) + (_14707 / _14723)) * _178.gLevel[5].zw;
                                    float2 _14746 = ((_14679 + float2(1.5)) + (_14720 / _14726)) * _178.gLevel[5].zw;
                                    float2 _22671;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21532 = _14736;
                                        _21532.y = 1.0 - _14736.y;
                                        _22671 = _21532;
                                    }
                                    else
                                    {
                                        _22671 = _14736;
                                    }
                                    float _14766 = _14736.y;
                                    float2 _14767 = float2(_14746.x, _14766);
                                    float2 _22672;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21538 = _14767;
                                        _21538.y = 1.0 - _14766;
                                        _22672 = _21538;
                                    }
                                    else
                                    {
                                        _22672 = _14767;
                                    }
                                    float2 _14784 = float2(_14736.x, _14746.y);
                                    float2 _22673;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21545 = _14784;
                                        _21545.y = 1.0 - _14746.y;
                                        _22673 = _21545;
                                    }
                                    else
                                    {
                                        _22673 = _14784;
                                    }
                                    float2 _22674;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21551 = _14746;
                                        _21551.y = 1.0 - _14746.y;
                                        _22674 = _21551;
                                    }
                                    else
                                    {
                                        _22674 = _14746;
                                    }
                                    _22696 = (((gBackdrop5.sample(gLinear, _22671, level(0.0)) * _14723.x) + (gBackdrop5.sample(gLinear, _22672, level(0.0)) * _14726.x)) * _14723.y) + (((gBackdrop5.sample(gLinear, _22673, level(0.0)) * _14723.x) + (gBackdrop5.sample(gLinear, _22674, level(0.0)) * _14726.x)) * _14726.y);
                                }
                                _22695 = _22696;
                            }
                            _22694 = _22695;
                        }
                        _22693 = _22694;
                    }
                    _22692 = _22693;
                }
                float3 _22723;
                if ((_13714 > 0.0199999995529651641845703125) && (_13710 < 5))
                {
                    int _13727 = _13710 + 1;
                    float4 _22718;
                    if (_13727 <= 0)
                    {
                        float2 _22717;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _21556 = _6725;
                            _21556.y = 1.0 - _6725.y;
                            _22717 = _21556;
                        }
                        else
                        {
                            _22717 = _6725;
                        }
                        _22718 = gBackdrop0.sample(gLinear, _22717, level(0.0));
                    }
                    else
                    {
                        float4 _22719;
                        if (_13727 == 1)
                        {
                            float2 _14973 = (_6725 * _178.gLevel[1].xy) - float2(0.5);
                            float2 _14975 = floor(_14973);
                            float2 _14978 = _14973 - _14975;
                            float2 _14981 = _14978 * _14978;
                            float2 _14984 = _14981 * _14978;
                            float2 _15003 = (((_14984 * 3.0) - (_14981 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _15016 = _14984 * 0.16666667163372039794921875;
                            float2 _15019 = (((((-_14984) + (_14981 * 3.0)) - (_14978 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15003;
                            float2 _15022 = (((((_14984 * (-3.0)) + (_14981 * 3.0)) + (_14978 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15016;
                            float2 _15032 = ((_14975 - float2(0.5)) + (_15003 / _15019)) * _178.gLevel[1].zw;
                            float2 _15042 = ((_14975 + float2(1.5)) + (_15016 / _15022)) * _178.gLevel[1].zw;
                            float2 _22713;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21561 = _15032;
                                _21561.y = 1.0 - _15032.y;
                                _22713 = _21561;
                            }
                            else
                            {
                                _22713 = _15032;
                            }
                            float _15062 = _15032.y;
                            float2 _15063 = float2(_15042.x, _15062);
                            float2 _22714;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21567 = _15063;
                                _21567.y = 1.0 - _15062;
                                _22714 = _21567;
                            }
                            else
                            {
                                _22714 = _15063;
                            }
                            float2 _15080 = float2(_15032.x, _15042.y);
                            float2 _22715;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21574 = _15080;
                                _21574.y = 1.0 - _15042.y;
                                _22715 = _21574;
                            }
                            else
                            {
                                _22715 = _15080;
                            }
                            float2 _22716;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21580 = _15042;
                                _21580.y = 1.0 - _15042.y;
                                _22716 = _21580;
                            }
                            else
                            {
                                _22716 = _15042;
                            }
                            _22719 = (((gBackdrop1.sample(gLinear, _22713, level(0.0)) * _15019.x) + (gBackdrop1.sample(gLinear, _22714, level(0.0)) * _15022.x)) * _15019.y) + (((gBackdrop1.sample(gLinear, _22715, level(0.0)) * _15019.x) + (gBackdrop1.sample(gLinear, _22716, level(0.0)) * _15022.x)) * _15022.y);
                        }
                        else
                        {
                            float4 _22720;
                            if (_13727 == 2)
                            {
                                float2 _15180 = (_6725 * _178.gLevel[2].xy) - float2(0.5);
                                float2 _15182 = floor(_15180);
                                float2 _15185 = _15180 - _15182;
                                float2 _15188 = _15185 * _15185;
                                float2 _15191 = _15188 * _15185;
                                float2 _15210 = (((_15191 * 3.0) - (_15188 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _15223 = _15191 * 0.16666667163372039794921875;
                                float2 _15226 = (((((-_15191) + (_15188 * 3.0)) - (_15185 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15210;
                                float2 _15229 = (((((_15191 * (-3.0)) + (_15188 * 3.0)) + (_15185 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15223;
                                float2 _15239 = ((_15182 - float2(0.5)) + (_15210 / _15226)) * _178.gLevel[2].zw;
                                float2 _15249 = ((_15182 + float2(1.5)) + (_15223 / _15229)) * _178.gLevel[2].zw;
                                float2 _22709;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21587 = _15239;
                                    _21587.y = 1.0 - _15239.y;
                                    _22709 = _21587;
                                }
                                else
                                {
                                    _22709 = _15239;
                                }
                                float _15269 = _15239.y;
                                float2 _15270 = float2(_15249.x, _15269);
                                float2 _22710;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21593 = _15270;
                                    _21593.y = 1.0 - _15269;
                                    _22710 = _21593;
                                }
                                else
                                {
                                    _22710 = _15270;
                                }
                                float2 _15287 = float2(_15239.x, _15249.y);
                                float2 _22711;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21600 = _15287;
                                    _21600.y = 1.0 - _15249.y;
                                    _22711 = _21600;
                                }
                                else
                                {
                                    _22711 = _15287;
                                }
                                float2 _22712;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21606 = _15249;
                                    _21606.y = 1.0 - _15249.y;
                                    _22712 = _21606;
                                }
                                else
                                {
                                    _22712 = _15249;
                                }
                                _22720 = (((gBackdrop2.sample(gLinear, _22709, level(0.0)) * _15226.x) + (gBackdrop2.sample(gLinear, _22710, level(0.0)) * _15229.x)) * _15226.y) + (((gBackdrop2.sample(gLinear, _22711, level(0.0)) * _15226.x) + (gBackdrop2.sample(gLinear, _22712, level(0.0)) * _15229.x)) * _15229.y);
                            }
                            else
                            {
                                float4 _22721;
                                if (_13727 == 3)
                                {
                                    float2 _15387 = (_6725 * _178.gLevel[3].xy) - float2(0.5);
                                    float2 _15389 = floor(_15387);
                                    float2 _15392 = _15387 - _15389;
                                    float2 _15395 = _15392 * _15392;
                                    float2 _15398 = _15395 * _15392;
                                    float2 _15417 = (((_15398 * 3.0) - (_15395 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _15430 = _15398 * 0.16666667163372039794921875;
                                    float2 _15433 = (((((-_15398) + (_15395 * 3.0)) - (_15392 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15417;
                                    float2 _15436 = (((((_15398 * (-3.0)) + (_15395 * 3.0)) + (_15392 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15430;
                                    float2 _15446 = ((_15389 - float2(0.5)) + (_15417 / _15433)) * _178.gLevel[3].zw;
                                    float2 _15456 = ((_15389 + float2(1.5)) + (_15430 / _15436)) * _178.gLevel[3].zw;
                                    float2 _22705;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21613 = _15446;
                                        _21613.y = 1.0 - _15446.y;
                                        _22705 = _21613;
                                    }
                                    else
                                    {
                                        _22705 = _15446;
                                    }
                                    float _15476 = _15446.y;
                                    float2 _15477 = float2(_15456.x, _15476);
                                    float2 _22706;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21619 = _15477;
                                        _21619.y = 1.0 - _15476;
                                        _22706 = _21619;
                                    }
                                    else
                                    {
                                        _22706 = _15477;
                                    }
                                    float2 _15494 = float2(_15446.x, _15456.y);
                                    float2 _22707;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21626 = _15494;
                                        _21626.y = 1.0 - _15456.y;
                                        _22707 = _21626;
                                    }
                                    else
                                    {
                                        _22707 = _15494;
                                    }
                                    float2 _22708;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21632 = _15456;
                                        _21632.y = 1.0 - _15456.y;
                                        _22708 = _21632;
                                    }
                                    else
                                    {
                                        _22708 = _15456;
                                    }
                                    _22721 = (((gBackdrop3.sample(gLinear, _22705, level(0.0)) * _15433.x) + (gBackdrop3.sample(gLinear, _22706, level(0.0)) * _15436.x)) * _15433.y) + (((gBackdrop3.sample(gLinear, _22707, level(0.0)) * _15433.x) + (gBackdrop3.sample(gLinear, _22708, level(0.0)) * _15436.x)) * _15436.y);
                                }
                                else
                                {
                                    float4 _22722;
                                    if (_13727 == 4)
                                    {
                                        float2 _15594 = (_6725 * _178.gLevel[4].xy) - float2(0.5);
                                        float2 _15596 = floor(_15594);
                                        float2 _15599 = _15594 - _15596;
                                        float2 _15602 = _15599 * _15599;
                                        float2 _15605 = _15602 * _15599;
                                        float2 _15624 = (((_15605 * 3.0) - (_15602 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _15637 = _15605 * 0.16666667163372039794921875;
                                        float2 _15640 = (((((-_15605) + (_15602 * 3.0)) - (_15599 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15624;
                                        float2 _15643 = (((((_15605 * (-3.0)) + (_15602 * 3.0)) + (_15599 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15637;
                                        float2 _15653 = ((_15596 - float2(0.5)) + (_15624 / _15640)) * _178.gLevel[4].zw;
                                        float2 _15663 = ((_15596 + float2(1.5)) + (_15637 / _15643)) * _178.gLevel[4].zw;
                                        float2 _22701;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21639 = _15653;
                                            _21639.y = 1.0 - _15653.y;
                                            _22701 = _21639;
                                        }
                                        else
                                        {
                                            _22701 = _15653;
                                        }
                                        float _15683 = _15653.y;
                                        float2 _15684 = float2(_15663.x, _15683);
                                        float2 _22702;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21645 = _15684;
                                            _21645.y = 1.0 - _15683;
                                            _22702 = _21645;
                                        }
                                        else
                                        {
                                            _22702 = _15684;
                                        }
                                        float2 _15701 = float2(_15653.x, _15663.y);
                                        float2 _22703;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21652 = _15701;
                                            _21652.y = 1.0 - _15663.y;
                                            _22703 = _21652;
                                        }
                                        else
                                        {
                                            _22703 = _15701;
                                        }
                                        float2 _22704;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21658 = _15663;
                                            _21658.y = 1.0 - _15663.y;
                                            _22704 = _21658;
                                        }
                                        else
                                        {
                                            _22704 = _15663;
                                        }
                                        _22722 = (((gBackdrop4.sample(gLinear, _22701, level(0.0)) * _15640.x) + (gBackdrop4.sample(gLinear, _22702, level(0.0)) * _15643.x)) * _15640.y) + (((gBackdrop4.sample(gLinear, _22703, level(0.0)) * _15640.x) + (gBackdrop4.sample(gLinear, _22704, level(0.0)) * _15643.x)) * _15643.y);
                                    }
                                    else
                                    {
                                        float2 _15801 = (_6725 * _178.gLevel[5].xy) - float2(0.5);
                                        float2 _15803 = floor(_15801);
                                        float2 _15806 = _15801 - _15803;
                                        float2 _15809 = _15806 * _15806;
                                        float2 _15812 = _15809 * _15806;
                                        float2 _15831 = (((_15812 * 3.0) - (_15809 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _15844 = _15812 * 0.16666667163372039794921875;
                                        float2 _15847 = (((((-_15812) + (_15809 * 3.0)) - (_15806 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15831;
                                        float2 _15850 = (((((_15812 * (-3.0)) + (_15809 * 3.0)) + (_15806 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15844;
                                        float2 _15860 = ((_15803 - float2(0.5)) + (_15831 / _15847)) * _178.gLevel[5].zw;
                                        float2 _15870 = ((_15803 + float2(1.5)) + (_15844 / _15850)) * _178.gLevel[5].zw;
                                        float2 _22697;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21665 = _15860;
                                            _21665.y = 1.0 - _15860.y;
                                            _22697 = _21665;
                                        }
                                        else
                                        {
                                            _22697 = _15860;
                                        }
                                        float _15890 = _15860.y;
                                        float2 _15891 = float2(_15870.x, _15890);
                                        float2 _22698;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21671 = _15891;
                                            _21671.y = 1.0 - _15890;
                                            _22698 = _21671;
                                        }
                                        else
                                        {
                                            _22698 = _15891;
                                        }
                                        float2 _15908 = float2(_15860.x, _15870.y);
                                        float2 _22699;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21678 = _15908;
                                            _21678.y = 1.0 - _15870.y;
                                            _22699 = _21678;
                                        }
                                        else
                                        {
                                            _22699 = _15908;
                                        }
                                        float2 _22700;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21684 = _15870;
                                            _21684.y = 1.0 - _15870.y;
                                            _22700 = _21684;
                                        }
                                        else
                                        {
                                            _22700 = _15870;
                                        }
                                        _22722 = (((gBackdrop5.sample(gLinear, _22697, level(0.0)) * _15847.x) + (gBackdrop5.sample(gLinear, _22698, level(0.0)) * _15850.x)) * _15847.y) + (((gBackdrop5.sample(gLinear, _22699, level(0.0)) * _15847.x) + (gBackdrop5.sample(gLinear, _22700, level(0.0)) * _15850.x)) * _15850.y);
                                    }
                                    _22721 = _22722;
                                }
                                _22720 = _22721;
                            }
                            _22719 = _22720;
                        }
                        _22718 = _22719;
                    }
                    _22723 = mix(_22692.xyz, _22718.xyz, float3(_13714));
                }
                else
                {
                    _22723 = _22692.xyz;
                }
                _23013 = float3(_22511.x, _22617.y, _22723.z);
            }
            else
            {
                float2 _6733 = _6663 + _6675;
                float _15998 = fast::clamp(log2(fast::max(_6577, 1.0)) - 1.0, 0.0, 5.0);
                int _16001 = int(floor(_15998));
                float _16005 = _15998 - float(_16001);
                float4 _22427;
                if (_16001 <= 0)
                {
                    float2 _22426;
                    if (_178.gConv.x > 0.5)
                    {
                        float2 _21691 = _6733;
                        _21691.y = 1.0 - _6733.y;
                        _22426 = _21691;
                    }
                    else
                    {
                        _22426 = _6733;
                    }
                    _22427 = gBackdrop0.sample(gLinear, _22426, level(0.0));
                }
                else
                {
                    float4 _22428;
                    if (_16001 == 1)
                    {
                        float2 _16140 = (_6733 * _178.gLevel[1].xy) - float2(0.5);
                        float2 _16142 = floor(_16140);
                        float2 _16145 = _16140 - _16142;
                        float2 _16148 = _16145 * _16145;
                        float2 _16151 = _16148 * _16145;
                        float2 _16170 = (((_16151 * 3.0) - (_16148 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _16183 = _16151 * 0.16666667163372039794921875;
                        float2 _16186 = (((((-_16151) + (_16148 * 3.0)) - (_16145 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16170;
                        float2 _16189 = (((((_16151 * (-3.0)) + (_16148 * 3.0)) + (_16145 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16183;
                        float2 _16199 = ((_16142 - float2(0.5)) + (_16170 / _16186)) * _178.gLevel[1].zw;
                        float2 _16209 = ((_16142 + float2(1.5)) + (_16183 / _16189)) * _178.gLevel[1].zw;
                        float2 _22422;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _21696 = _16199;
                            _21696.y = 1.0 - _16199.y;
                            _22422 = _21696;
                        }
                        else
                        {
                            _22422 = _16199;
                        }
                        float _16229 = _16199.y;
                        float2 _16230 = float2(_16209.x, _16229);
                        float2 _22423;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _21702 = _16230;
                            _21702.y = 1.0 - _16229;
                            _22423 = _21702;
                        }
                        else
                        {
                            _22423 = _16230;
                        }
                        float2 _16247 = float2(_16199.x, _16209.y);
                        float2 _22424;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _21709 = _16247;
                            _21709.y = 1.0 - _16209.y;
                            _22424 = _21709;
                        }
                        else
                        {
                            _22424 = _16247;
                        }
                        float2 _22425;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _21715 = _16209;
                            _21715.y = 1.0 - _16209.y;
                            _22425 = _21715;
                        }
                        else
                        {
                            _22425 = _16209;
                        }
                        _22428 = (((gBackdrop1.sample(gLinear, _22422, level(0.0)) * _16186.x) + (gBackdrop1.sample(gLinear, _22423, level(0.0)) * _16189.x)) * _16186.y) + (((gBackdrop1.sample(gLinear, _22424, level(0.0)) * _16186.x) + (gBackdrop1.sample(gLinear, _22425, level(0.0)) * _16189.x)) * _16189.y);
                    }
                    else
                    {
                        float4 _22429;
                        if (_16001 == 2)
                        {
                            float2 _16347 = (_6733 * _178.gLevel[2].xy) - float2(0.5);
                            float2 _16349 = floor(_16347);
                            float2 _16352 = _16347 - _16349;
                            float2 _16355 = _16352 * _16352;
                            float2 _16358 = _16355 * _16352;
                            float2 _16377 = (((_16358 * 3.0) - (_16355 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _16390 = _16358 * 0.16666667163372039794921875;
                            float2 _16393 = (((((-_16358) + (_16355 * 3.0)) - (_16352 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16377;
                            float2 _16396 = (((((_16358 * (-3.0)) + (_16355 * 3.0)) + (_16352 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16390;
                            float2 _16406 = ((_16349 - float2(0.5)) + (_16377 / _16393)) * _178.gLevel[2].zw;
                            float2 _16416 = ((_16349 + float2(1.5)) + (_16390 / _16396)) * _178.gLevel[2].zw;
                            float2 _22418;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21722 = _16406;
                                _21722.y = 1.0 - _16406.y;
                                _22418 = _21722;
                            }
                            else
                            {
                                _22418 = _16406;
                            }
                            float _16436 = _16406.y;
                            float2 _16437 = float2(_16416.x, _16436);
                            float2 _22419;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21728 = _16437;
                                _21728.y = 1.0 - _16436;
                                _22419 = _21728;
                            }
                            else
                            {
                                _22419 = _16437;
                            }
                            float2 _16454 = float2(_16406.x, _16416.y);
                            float2 _22420;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21735 = _16454;
                                _21735.y = 1.0 - _16416.y;
                                _22420 = _21735;
                            }
                            else
                            {
                                _22420 = _16454;
                            }
                            float2 _22421;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21741 = _16416;
                                _21741.y = 1.0 - _16416.y;
                                _22421 = _21741;
                            }
                            else
                            {
                                _22421 = _16416;
                            }
                            _22429 = (((gBackdrop2.sample(gLinear, _22418, level(0.0)) * _16393.x) + (gBackdrop2.sample(gLinear, _22419, level(0.0)) * _16396.x)) * _16393.y) + (((gBackdrop2.sample(gLinear, _22420, level(0.0)) * _16393.x) + (gBackdrop2.sample(gLinear, _22421, level(0.0)) * _16396.x)) * _16396.y);
                        }
                        else
                        {
                            float4 _22430;
                            if (_16001 == 3)
                            {
                                float2 _16554 = (_6733 * _178.gLevel[3].xy) - float2(0.5);
                                float2 _16556 = floor(_16554);
                                float2 _16559 = _16554 - _16556;
                                float2 _16562 = _16559 * _16559;
                                float2 _16565 = _16562 * _16559;
                                float2 _16584 = (((_16565 * 3.0) - (_16562 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _16597 = _16565 * 0.16666667163372039794921875;
                                float2 _16600 = (((((-_16565) + (_16562 * 3.0)) - (_16559 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16584;
                                float2 _16603 = (((((_16565 * (-3.0)) + (_16562 * 3.0)) + (_16559 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16597;
                                float2 _16613 = ((_16556 - float2(0.5)) + (_16584 / _16600)) * _178.gLevel[3].zw;
                                float2 _16623 = ((_16556 + float2(1.5)) + (_16597 / _16603)) * _178.gLevel[3].zw;
                                float2 _22414;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21748 = _16613;
                                    _21748.y = 1.0 - _16613.y;
                                    _22414 = _21748;
                                }
                                else
                                {
                                    _22414 = _16613;
                                }
                                float _16643 = _16613.y;
                                float2 _16644 = float2(_16623.x, _16643);
                                float2 _22415;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21754 = _16644;
                                    _21754.y = 1.0 - _16643;
                                    _22415 = _21754;
                                }
                                else
                                {
                                    _22415 = _16644;
                                }
                                float2 _16661 = float2(_16613.x, _16623.y);
                                float2 _22416;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21761 = _16661;
                                    _21761.y = 1.0 - _16623.y;
                                    _22416 = _21761;
                                }
                                else
                                {
                                    _22416 = _16661;
                                }
                                float2 _22417;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21767 = _16623;
                                    _21767.y = 1.0 - _16623.y;
                                    _22417 = _21767;
                                }
                                else
                                {
                                    _22417 = _16623;
                                }
                                _22430 = (((gBackdrop3.sample(gLinear, _22414, level(0.0)) * _16600.x) + (gBackdrop3.sample(gLinear, _22415, level(0.0)) * _16603.x)) * _16600.y) + (((gBackdrop3.sample(gLinear, _22416, level(0.0)) * _16600.x) + (gBackdrop3.sample(gLinear, _22417, level(0.0)) * _16603.x)) * _16603.y);
                            }
                            else
                            {
                                float4 _22431;
                                if (_16001 == 4)
                                {
                                    float2 _16761 = (_6733 * _178.gLevel[4].xy) - float2(0.5);
                                    float2 _16763 = floor(_16761);
                                    float2 _16766 = _16761 - _16763;
                                    float2 _16769 = _16766 * _16766;
                                    float2 _16772 = _16769 * _16766;
                                    float2 _16791 = (((_16772 * 3.0) - (_16769 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _16804 = _16772 * 0.16666667163372039794921875;
                                    float2 _16807 = (((((-_16772) + (_16769 * 3.0)) - (_16766 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16791;
                                    float2 _16810 = (((((_16772 * (-3.0)) + (_16769 * 3.0)) + (_16766 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16804;
                                    float2 _16820 = ((_16763 - float2(0.5)) + (_16791 / _16807)) * _178.gLevel[4].zw;
                                    float2 _16830 = ((_16763 + float2(1.5)) + (_16804 / _16810)) * _178.gLevel[4].zw;
                                    float2 _22410;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21774 = _16820;
                                        _21774.y = 1.0 - _16820.y;
                                        _22410 = _21774;
                                    }
                                    else
                                    {
                                        _22410 = _16820;
                                    }
                                    float _16850 = _16820.y;
                                    float2 _16851 = float2(_16830.x, _16850);
                                    float2 _22411;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21780 = _16851;
                                        _21780.y = 1.0 - _16850;
                                        _22411 = _21780;
                                    }
                                    else
                                    {
                                        _22411 = _16851;
                                    }
                                    float2 _16868 = float2(_16820.x, _16830.y);
                                    float2 _22412;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21787 = _16868;
                                        _21787.y = 1.0 - _16830.y;
                                        _22412 = _21787;
                                    }
                                    else
                                    {
                                        _22412 = _16868;
                                    }
                                    float2 _22413;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21793 = _16830;
                                        _21793.y = 1.0 - _16830.y;
                                        _22413 = _21793;
                                    }
                                    else
                                    {
                                        _22413 = _16830;
                                    }
                                    _22431 = (((gBackdrop4.sample(gLinear, _22410, level(0.0)) * _16807.x) + (gBackdrop4.sample(gLinear, _22411, level(0.0)) * _16810.x)) * _16807.y) + (((gBackdrop4.sample(gLinear, _22412, level(0.0)) * _16807.x) + (gBackdrop4.sample(gLinear, _22413, level(0.0)) * _16810.x)) * _16810.y);
                                }
                                else
                                {
                                    float2 _16968 = (_6733 * _178.gLevel[5].xy) - float2(0.5);
                                    float2 _16970 = floor(_16968);
                                    float2 _16973 = _16968 - _16970;
                                    float2 _16976 = _16973 * _16973;
                                    float2 _16979 = _16976 * _16973;
                                    float2 _16998 = (((_16979 * 3.0) - (_16976 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _17011 = _16979 * 0.16666667163372039794921875;
                                    float2 _17014 = (((((-_16979) + (_16976 * 3.0)) - (_16973 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16998;
                                    float2 _17017 = (((((_16979 * (-3.0)) + (_16976 * 3.0)) + (_16973 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17011;
                                    float2 _17027 = ((_16970 - float2(0.5)) + (_16998 / _17014)) * _178.gLevel[5].zw;
                                    float2 _17037 = ((_16970 + float2(1.5)) + (_17011 / _17017)) * _178.gLevel[5].zw;
                                    float2 _22406;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21800 = _17027;
                                        _21800.y = 1.0 - _17027.y;
                                        _22406 = _21800;
                                    }
                                    else
                                    {
                                        _22406 = _17027;
                                    }
                                    float _17057 = _17027.y;
                                    float2 _17058 = float2(_17037.x, _17057);
                                    float2 _22407;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21806 = _17058;
                                        _21806.y = 1.0 - _17057;
                                        _22407 = _21806;
                                    }
                                    else
                                    {
                                        _22407 = _17058;
                                    }
                                    float2 _17075 = float2(_17027.x, _17037.y);
                                    float2 _22408;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21813 = _17075;
                                        _21813.y = 1.0 - _17037.y;
                                        _22408 = _21813;
                                    }
                                    else
                                    {
                                        _22408 = _17075;
                                    }
                                    float2 _22409;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21819 = _17037;
                                        _21819.y = 1.0 - _17037.y;
                                        _22409 = _21819;
                                    }
                                    else
                                    {
                                        _22409 = _17037;
                                    }
                                    _22431 = (((gBackdrop5.sample(gLinear, _22406, level(0.0)) * _17014.x) + (gBackdrop5.sample(gLinear, _22407, level(0.0)) * _17017.x)) * _17014.y) + (((gBackdrop5.sample(gLinear, _22408, level(0.0)) * _17014.x) + (gBackdrop5.sample(gLinear, _22409, level(0.0)) * _17017.x)) * _17017.y);
                                }
                                _22430 = _22431;
                            }
                            _22429 = _22430;
                        }
                        _22428 = _22429;
                    }
                    _22427 = _22428;
                }
                float3 _22458;
                if ((_16005 > 0.0199999995529651641845703125) && (_16001 < 5))
                {
                    int _16018 = _16001 + 1;
                    float4 _22453;
                    if (_16018 <= 0)
                    {
                        float2 _22452;
                        if (_178.gConv.x > 0.5)
                        {
                            float2 _21824 = _6733;
                            _21824.y = 1.0 - _6733.y;
                            _22452 = _21824;
                        }
                        else
                        {
                            _22452 = _6733;
                        }
                        _22453 = gBackdrop0.sample(gLinear, _22452, level(0.0));
                    }
                    else
                    {
                        float4 _22454;
                        if (_16018 == 1)
                        {
                            float2 _17264 = (_6733 * _178.gLevel[1].xy) - float2(0.5);
                            float2 _17266 = floor(_17264);
                            float2 _17269 = _17264 - _17266;
                            float2 _17272 = _17269 * _17269;
                            float2 _17275 = _17272 * _17269;
                            float2 _17294 = (((_17275 * 3.0) - (_17272 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _17307 = _17275 * 0.16666667163372039794921875;
                            float2 _17310 = (((((-_17275) + (_17272 * 3.0)) - (_17269 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17294;
                            float2 _17313 = (((((_17275 * (-3.0)) + (_17272 * 3.0)) + (_17269 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17307;
                            float2 _17323 = ((_17266 - float2(0.5)) + (_17294 / _17310)) * _178.gLevel[1].zw;
                            float2 _17333 = ((_17266 + float2(1.5)) + (_17307 / _17313)) * _178.gLevel[1].zw;
                            float2 _22448;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21829 = _17323;
                                _21829.y = 1.0 - _17323.y;
                                _22448 = _21829;
                            }
                            else
                            {
                                _22448 = _17323;
                            }
                            float _17353 = _17323.y;
                            float2 _17354 = float2(_17333.x, _17353);
                            float2 _22449;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21835 = _17354;
                                _21835.y = 1.0 - _17353;
                                _22449 = _21835;
                            }
                            else
                            {
                                _22449 = _17354;
                            }
                            float2 _17371 = float2(_17323.x, _17333.y);
                            float2 _22450;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21842 = _17371;
                                _21842.y = 1.0 - _17333.y;
                                _22450 = _21842;
                            }
                            else
                            {
                                _22450 = _17371;
                            }
                            float2 _22451;
                            if (_178.gConv.x > 0.5)
                            {
                                float2 _21848 = _17333;
                                _21848.y = 1.0 - _17333.y;
                                _22451 = _21848;
                            }
                            else
                            {
                                _22451 = _17333;
                            }
                            _22454 = (((gBackdrop1.sample(gLinear, _22448, level(0.0)) * _17310.x) + (gBackdrop1.sample(gLinear, _22449, level(0.0)) * _17313.x)) * _17310.y) + (((gBackdrop1.sample(gLinear, _22450, level(0.0)) * _17310.x) + (gBackdrop1.sample(gLinear, _22451, level(0.0)) * _17313.x)) * _17313.y);
                        }
                        else
                        {
                            float4 _22455;
                            if (_16018 == 2)
                            {
                                float2 _17471 = (_6733 * _178.gLevel[2].xy) - float2(0.5);
                                float2 _17473 = floor(_17471);
                                float2 _17476 = _17471 - _17473;
                                float2 _17479 = _17476 * _17476;
                                float2 _17482 = _17479 * _17476;
                                float2 _17501 = (((_17482 * 3.0) - (_17479 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _17514 = _17482 * 0.16666667163372039794921875;
                                float2 _17517 = (((((-_17482) + (_17479 * 3.0)) - (_17476 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17501;
                                float2 _17520 = (((((_17482 * (-3.0)) + (_17479 * 3.0)) + (_17476 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17514;
                                float2 _17530 = ((_17473 - float2(0.5)) + (_17501 / _17517)) * _178.gLevel[2].zw;
                                float2 _17540 = ((_17473 + float2(1.5)) + (_17514 / _17520)) * _178.gLevel[2].zw;
                                float2 _22444;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21855 = _17530;
                                    _21855.y = 1.0 - _17530.y;
                                    _22444 = _21855;
                                }
                                else
                                {
                                    _22444 = _17530;
                                }
                                float _17560 = _17530.y;
                                float2 _17561 = float2(_17540.x, _17560);
                                float2 _22445;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21861 = _17561;
                                    _21861.y = 1.0 - _17560;
                                    _22445 = _21861;
                                }
                                else
                                {
                                    _22445 = _17561;
                                }
                                float2 _17578 = float2(_17530.x, _17540.y);
                                float2 _22446;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21868 = _17578;
                                    _21868.y = 1.0 - _17540.y;
                                    _22446 = _21868;
                                }
                                else
                                {
                                    _22446 = _17578;
                                }
                                float2 _22447;
                                if (_178.gConv.x > 0.5)
                                {
                                    float2 _21874 = _17540;
                                    _21874.y = 1.0 - _17540.y;
                                    _22447 = _21874;
                                }
                                else
                                {
                                    _22447 = _17540;
                                }
                                _22455 = (((gBackdrop2.sample(gLinear, _22444, level(0.0)) * _17517.x) + (gBackdrop2.sample(gLinear, _22445, level(0.0)) * _17520.x)) * _17517.y) + (((gBackdrop2.sample(gLinear, _22446, level(0.0)) * _17517.x) + (gBackdrop2.sample(gLinear, _22447, level(0.0)) * _17520.x)) * _17520.y);
                            }
                            else
                            {
                                float4 _22456;
                                if (_16018 == 3)
                                {
                                    float2 _17678 = (_6733 * _178.gLevel[3].xy) - float2(0.5);
                                    float2 _17680 = floor(_17678);
                                    float2 _17683 = _17678 - _17680;
                                    float2 _17686 = _17683 * _17683;
                                    float2 _17689 = _17686 * _17683;
                                    float2 _17708 = (((_17689 * 3.0) - (_17686 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _17721 = _17689 * 0.16666667163372039794921875;
                                    float2 _17724 = (((((-_17689) + (_17686 * 3.0)) - (_17683 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17708;
                                    float2 _17727 = (((((_17689 * (-3.0)) + (_17686 * 3.0)) + (_17683 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17721;
                                    float2 _17737 = ((_17680 - float2(0.5)) + (_17708 / _17724)) * _178.gLevel[3].zw;
                                    float2 _17747 = ((_17680 + float2(1.5)) + (_17721 / _17727)) * _178.gLevel[3].zw;
                                    float2 _22440;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21881 = _17737;
                                        _21881.y = 1.0 - _17737.y;
                                        _22440 = _21881;
                                    }
                                    else
                                    {
                                        _22440 = _17737;
                                    }
                                    float _17767 = _17737.y;
                                    float2 _17768 = float2(_17747.x, _17767);
                                    float2 _22441;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21887 = _17768;
                                        _21887.y = 1.0 - _17767;
                                        _22441 = _21887;
                                    }
                                    else
                                    {
                                        _22441 = _17768;
                                    }
                                    float2 _17785 = float2(_17737.x, _17747.y);
                                    float2 _22442;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21894 = _17785;
                                        _21894.y = 1.0 - _17747.y;
                                        _22442 = _21894;
                                    }
                                    else
                                    {
                                        _22442 = _17785;
                                    }
                                    float2 _22443;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        float2 _21900 = _17747;
                                        _21900.y = 1.0 - _17747.y;
                                        _22443 = _21900;
                                    }
                                    else
                                    {
                                        _22443 = _17747;
                                    }
                                    _22456 = (((gBackdrop3.sample(gLinear, _22440, level(0.0)) * _17724.x) + (gBackdrop3.sample(gLinear, _22441, level(0.0)) * _17727.x)) * _17724.y) + (((gBackdrop3.sample(gLinear, _22442, level(0.0)) * _17724.x) + (gBackdrop3.sample(gLinear, _22443, level(0.0)) * _17727.x)) * _17727.y);
                                }
                                else
                                {
                                    float4 _22457;
                                    if (_16018 == 4)
                                    {
                                        float2 _17885 = (_6733 * _178.gLevel[4].xy) - float2(0.5);
                                        float2 _17887 = floor(_17885);
                                        float2 _17890 = _17885 - _17887;
                                        float2 _17893 = _17890 * _17890;
                                        float2 _17896 = _17893 * _17890;
                                        float2 _17915 = (((_17896 * 3.0) - (_17893 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _17928 = _17896 * 0.16666667163372039794921875;
                                        float2 _17931 = (((((-_17896) + (_17893 * 3.0)) - (_17890 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17915;
                                        float2 _17934 = (((((_17896 * (-3.0)) + (_17893 * 3.0)) + (_17890 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17928;
                                        float2 _17944 = ((_17887 - float2(0.5)) + (_17915 / _17931)) * _178.gLevel[4].zw;
                                        float2 _17954 = ((_17887 + float2(1.5)) + (_17928 / _17934)) * _178.gLevel[4].zw;
                                        float2 _22436;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21907 = _17944;
                                            _21907.y = 1.0 - _17944.y;
                                            _22436 = _21907;
                                        }
                                        else
                                        {
                                            _22436 = _17944;
                                        }
                                        float _17974 = _17944.y;
                                        float2 _17975 = float2(_17954.x, _17974);
                                        float2 _22437;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21913 = _17975;
                                            _21913.y = 1.0 - _17974;
                                            _22437 = _21913;
                                        }
                                        else
                                        {
                                            _22437 = _17975;
                                        }
                                        float2 _17992 = float2(_17944.x, _17954.y);
                                        float2 _22438;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21920 = _17992;
                                            _21920.y = 1.0 - _17954.y;
                                            _22438 = _21920;
                                        }
                                        else
                                        {
                                            _22438 = _17992;
                                        }
                                        float2 _22439;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21926 = _17954;
                                            _21926.y = 1.0 - _17954.y;
                                            _22439 = _21926;
                                        }
                                        else
                                        {
                                            _22439 = _17954;
                                        }
                                        _22457 = (((gBackdrop4.sample(gLinear, _22436, level(0.0)) * _17931.x) + (gBackdrop4.sample(gLinear, _22437, level(0.0)) * _17934.x)) * _17931.y) + (((gBackdrop4.sample(gLinear, _22438, level(0.0)) * _17931.x) + (gBackdrop4.sample(gLinear, _22439, level(0.0)) * _17934.x)) * _17934.y);
                                    }
                                    else
                                    {
                                        float2 _18092 = (_6733 * _178.gLevel[5].xy) - float2(0.5);
                                        float2 _18094 = floor(_18092);
                                        float2 _18097 = _18092 - _18094;
                                        float2 _18100 = _18097 * _18097;
                                        float2 _18103 = _18100 * _18097;
                                        float2 _18122 = (((_18103 * 3.0) - (_18100 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _18135 = _18103 * 0.16666667163372039794921875;
                                        float2 _18138 = (((((-_18103) + (_18100 * 3.0)) - (_18097 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _18122;
                                        float2 _18141 = (((((_18103 * (-3.0)) + (_18100 * 3.0)) + (_18097 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _18135;
                                        float2 _18151 = ((_18094 - float2(0.5)) + (_18122 / _18138)) * _178.gLevel[5].zw;
                                        float2 _18161 = ((_18094 + float2(1.5)) + (_18135 / _18141)) * _178.gLevel[5].zw;
                                        float2 _22432;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21933 = _18151;
                                            _21933.y = 1.0 - _18151.y;
                                            _22432 = _21933;
                                        }
                                        else
                                        {
                                            _22432 = _18151;
                                        }
                                        float _18181 = _18151.y;
                                        float2 _18182 = float2(_18161.x, _18181);
                                        float2 _22433;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21939 = _18182;
                                            _21939.y = 1.0 - _18181;
                                            _22433 = _21939;
                                        }
                                        else
                                        {
                                            _22433 = _18182;
                                        }
                                        float2 _18199 = float2(_18151.x, _18161.y);
                                        float2 _22434;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21946 = _18199;
                                            _21946.y = 1.0 - _18161.y;
                                            _22434 = _21946;
                                        }
                                        else
                                        {
                                            _22434 = _18199;
                                        }
                                        float2 _22435;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            float2 _21952 = _18161;
                                            _21952.y = 1.0 - _18161.y;
                                            _22435 = _21952;
                                        }
                                        else
                                        {
                                            _22435 = _18161;
                                        }
                                        _22457 = (((gBackdrop5.sample(gLinear, _22432, level(0.0)) * _18138.x) + (gBackdrop5.sample(gLinear, _22433, level(0.0)) * _18141.x)) * _18138.y) + (((gBackdrop5.sample(gLinear, _22434, level(0.0)) * _18138.x) + (gBackdrop5.sample(gLinear, _22435, level(0.0)) * _18141.x)) * _18141.y);
                                    }
                                    _22456 = _22457;
                                }
                                _22455 = _22456;
                            }
                            _22454 = _22455;
                        }
                        _22453 = _22454;
                    }
                    _22458 = mix(_22427.xyz, _22453.xyz, float3(_16005));
                }
                else
                {
                    _22458 = _22427.xyz;
                }
                _23013 = _22458;
            }
            float3 _23504;
            if (_6605 > 0.0)
            {
                float2 _6754 = _6663 + (((_22404 * fast::min(_6596 * 0.5, 16.0)) * _178.gDisplay.zw) * _178.gTarget.zw);
                float _18289 = fast::clamp(log2(fast::max(16.0 * _178.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _18292 = int(floor(_18289));
                float _18296 = _18289 - float(_18292);
                float2 _22991;
                if (_178.gConv.x > 0.5)
                {
                    float2 _21957 = _6754;
                    _21957.y = 1.0 - _6754.y;
                    _22991 = _21957;
                }
                else
                {
                    _22991 = _6754;
                }
                float4 _22992;
                if (_18292 <= 0)
                {
                    _22992 = gBackdrop0.sample(gLinear, _22991, level(0.0));
                }
                else
                {
                    float4 _22993;
                    if (_18292 == 1)
                    {
                        _22993 = gBackdrop1.sample(gLinear, _22991, level(0.0));
                    }
                    else
                    {
                        float4 _22994;
                        if (_18292 == 2)
                        {
                            _22994 = gBackdrop2.sample(gLinear, _22991, level(0.0));
                        }
                        else
                        {
                            float4 _22995;
                            if (_18292 == 3)
                            {
                                _22995 = gBackdrop3.sample(gLinear, _22991, level(0.0));
                            }
                            else
                            {
                                float4 _22996;
                                if (_18292 == 4)
                                {
                                    _22996 = gBackdrop4.sample(gLinear, _22991, level(0.0));
                                }
                                else
                                {
                                    _22996 = gBackdrop5.sample(gLinear, _22991, level(0.0));
                                }
                                _22995 = _22996;
                            }
                            _22994 = _22995;
                        }
                        _22993 = _22994;
                    }
                    _22992 = _22993;
                }
                float3 _23003;
                if ((_18296 > 0.0199999995529651641845703125) && (_18292 < 5))
                {
                    int _18309 = _18292 + 1;
                    float2 _22997;
                    if (_178.gConv.x > 0.5)
                    {
                        float2 _21960 = _6754;
                        _21960.y = 1.0 - _6754.y;
                        _22997 = _21960;
                    }
                    else
                    {
                        _22997 = _6754;
                    }
                    float4 _22998;
                    if (_18309 <= 0)
                    {
                        _22998 = gBackdrop0.sample(gLinear, _22997, level(0.0));
                    }
                    else
                    {
                        float4 _22999;
                        if (_18309 == 1)
                        {
                            _22999 = gBackdrop1.sample(gLinear, _22997, level(0.0));
                        }
                        else
                        {
                            float4 _23000;
                            if (_18309 == 2)
                            {
                                _23000 = gBackdrop2.sample(gLinear, _22997, level(0.0));
                            }
                            else
                            {
                                float4 _23001;
                                if (_18309 == 3)
                                {
                                    _23001 = gBackdrop3.sample(gLinear, _22997, level(0.0));
                                }
                                else
                                {
                                    float4 _23002;
                                    if (_18309 == 4)
                                    {
                                        _23002 = gBackdrop4.sample(gLinear, _22997, level(0.0));
                                    }
                                    else
                                    {
                                        _23002 = gBackdrop5.sample(gLinear, _22997, level(0.0));
                                    }
                                    _23001 = _23002;
                                }
                                _23000 = _23001;
                            }
                            _22999 = _23000;
                        }
                        _22998 = _22999;
                    }
                    _23003 = mix(_22992.xyz, _22998.xyz, float3(_18296));
                }
                else
                {
                    _23003 = _22992.xyz;
                }
                _23504 = _23003;
            }
            else
            {
                _23504 = float3(0.5);
            }
            float _23034;
            if (in.i_shape.x > 0.001000000047497451305389404296875)
            {
                int _18476 = clamp(int(rint(log2(36.0 * _178.gDisplay.z) - 1.0)), 1, 4);
                float2 _23004;
                if (_178.gConv.x > 0.5)
                {
                    float2 _21964 = _6663;
                    _21964.y = 1.0 - _6663.y;
                    _23004 = _21964;
                }
                else
                {
                    _23004 = _6663;
                }
                float4 _23005;
                if (_18476 <= 0)
                {
                    _23005 = gBackdrop0.sample(gLinear, _23004, level(0.0));
                }
                else
                {
                    float4 _23006;
                    if (_18476 == 1)
                    {
                        _23006 = gBackdrop1.sample(gLinear, _23004, level(0.0));
                    }
                    else
                    {
                        float4 _23007;
                        if (_18476 == 2)
                        {
                            _23007 = gBackdrop2.sample(gLinear, _23004, level(0.0));
                        }
                        else
                        {
                            float4 _23008;
                            if (_18476 == 3)
                            {
                                _23008 = gBackdrop3.sample(gLinear, _23004, level(0.0));
                            }
                            else
                            {
                                float4 _23009;
                                if (_18476 == 4)
                                {
                                    _23009 = gBackdrop4.sample(gLinear, _23004, level(0.0));
                                }
                                else
                                {
                                    _23009 = gBackdrop5.sample(gLinear, _23004, level(0.0));
                                }
                                _23008 = _23009;
                            }
                            _23007 = _23008;
                        }
                        _23006 = _23007;
                    }
                    _23005 = _23006;
                }
                _23034 = dot(_23005.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _23034 = 0.5;
            }
            _23502 = _23504;
            _23033 = _23034;
            _23010 = _23013;
        }
        else
        {
            _23502 = float3(0.5);
            _23033 = 0.5;
            _23010 = float3(0.5);
        }
        float3 _6782 = mix(float3(dot(_23010, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _23010, float3(gFxData_1._data[_6502].x)) + float3(gFxData_1._data[_6502].y);
        float3 _23262;
        if (in.i_shape.x > 0.001000000047497451305389404296875)
        {
            float _6792 = fast::clamp(fast::max(_23033 + gFxData_1._data[_6502].y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _6800 = mix(_6792, dot(gFxData_1._data[_6494].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), in.i_shape.x);
            _23262 = select(mix(_6782, float3(1.0), float3((_6800 - _6792) / fast::max(1.0 - _6792, 0.001000000047497451305389404296875))), _6782 * (_6800 / _6792), bool3(_6800 < _6792));
        }
        else
        {
            _23262 = _6782;
        }
        float2 _6838 = fast::clamp((in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875)), float2(0.0), float2(1.0));
        float2 _6886 = float2(cos(gFxData_1._data[_6502].w), sin(gFxData_1._data[_6502].w));
        float _6889 = dot(_22404, _6886);
        float _6907 = _3938 * (abs(_22404.x) + abs(_22404.y));
        float _23748;
        _23748 = 0.0;
        float _6929;
        for (int _23747 = -2; _23747 <= 2; _23748 = _6929, _23747++)
        {
            float _6924 = (-_22217) + ((float(_23747) * 0.4000000059604644775390625) * _6907);
            float _25685;
            do
            {
                if (_6924 < 0.0)
                {
                    _25685 = 0.0;
                    break;
                }
                float _18584 = fast::clamp(1.0 - (_6924 / _6596), 0.0, 1.0);
                float _18595 = fast::min(_18584 / fast::max(sqrt(fast::clamp(1.0 - (_18584 * _18584), 0.0, 1.0)), 0.001000000047497451305389404296875), 8.0);
                _25685 = pow(1.0 - rsqrt(1.0 + (_18595 * _18595)), 5.0);
                break;
            } while(false);
            _6929 = _23748 + ((3.0 - abs(float(_23747))) * _25685);
        }
        float _6960 = fast::clamp(0.5 + (0.5 * dot((in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5)) / _6588, _6886)), 0.0, 1.0);
        _25325 = (((_23748 * 0.111111111938953399658203125) * (pow(fast::clamp(_6889, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(fast::clamp(-_6889, 0.0, 1.0), 1.5)))) * gFxData_1._data[_6502].z) * 1.60000002384185791015625;
        _24678 = gFxData_1._data[_6510];
        _24665 = float4((mix(mix(_23262, gFxData_1._data[_6494].xyz, float3(fast::clamp(gFxData_1._data[_6494].w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _6611))) + (0.3499999940395355224609375 * ((dot(gFxData_1._data[_6494].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _6838.y) : _6838.y))), 0.0, 1.0))), (_23502 * 1.10000002384185791015625) + float3(0.07999999821186065673828125), float3(pow(1.0 - _23266.z, 5.0) * 0.3499999940395355224609375)) + float3((((0.039999999105930328369140625 * _6960) * _6960) + (0.0500000007450580596923828125 * (1.0 - _6611))) * gFxData_1._data[_6502].z)) * _3963, _3963) + (_23755 * (1.0 - _3963));
    }
    else
    {
        _25325 = 0.0;
        _24678 = _22197;
        _24665 = _23755;
    }
    float4 _24675;
    if ((in.i_flags.x & 1u) != 0u)
    {
        float4 _24030;
        float4 _24341;
        if (in.i_flags.y != 0u)
        {
            _24341 = gFxData_1._data[(in.i_instance * 24u) + 3u];
            _24030 = gFxData_1._data[(in.i_instance * 24u) + 4u];
        }
        else
        {
            _24341 = float4(0.0);
            _24030 = float4(0.0);
        }
        float4 _24660;
        do
        {
            if (in.i_flags.y == 0u)
            {
                _24660 = in.i_fill0;
                break;
            }
            float2 _18667 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float2 _18677 = fwidth(in.i_local);
            float _18679 = fast::max(length(_18677), 9.9999997473787516355514526367188e-05);
            float _24652;
            float _24656;
            if ((in.i_flags.y == 1u) || (in.i_flags.y == 4u))
            {
                float _18688 = cos(_24030.x);
                float _18691 = sin(_24030.x);
                float _18717 = ((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_18688, _18691)) / fast::max(0.5 * ((abs(_18688) * _18667.x) + (abs(_18691) * _18667.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (in.i_flags.y == 4u)
                {
                    float _18732 = fast::clamp((_18717 - _24030.y) / fast::max(_24030.z - _24030.y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _24660 = float4(fast::clamp(abs((fract(float3(_18732 * 0.800000011920928955078125) + float3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - float3(3.0)) - float3(1.0), float3(0.0), float3(1.0)), in.i_fill0.w * pow(fast::max(sin(_18732 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _24656 = -1.0;
                _24652 = _18717;
            }
            else
            {
                float _24653;
                float _24657;
                if (in.i_flags.y == 2u)
                {
                    _24657 = -1.0;
                    _24653 = length(in.i_local - (in.i_rect.xy + (_24030.xy * _18667))) / fast::max(_24030.z * fast::max(_18667.x, _18667.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    float2 _18800 = in.i_local - (in.i_rect.xy + (_24030.xy * _18667));
                    float _18811 = fract(((precise::atan2(_18800.y, _18800.x) - _24030.z) * 0.15915493667125701904296875) + 1.0);
                    float _24654;
                    float _24658;
                    if (_24030.w > 0.5)
                    {
                        _24658 = -1.0;
                        _24654 = 0.5 - (0.5 * cos(_18811 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _18831 = (((_18811 < 0.5) ? _18811 : (_18811 - 1.0)) * 6.283185482025146484375) * length(_18800);
                        float _24659;
                        if (abs(_18831) < _18679)
                        {
                            _24659 = fast::clamp(((_18831 / _18679) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _24659 = -1.0;
                        }
                        _24658 = _24659;
                        _24654 = _18811;
                    }
                    _24657 = _24658;
                    _24653 = _24654;
                }
                _24656 = _24657;
                _24652 = _24653;
            }
            float4 _18860 = float4(in.i_fill0.xyz * in.i_fill0.w, in.i_fill0.w);
            float4 _18872 = float4(_24341.xyz * _24341.w, _24341.w);
            float4 _18886 = select(mix(_18860, _18872, float4(fast::clamp(_24652, 0.0, 1.0))), mix(_18872, _18860, float4(_24656)), bool4(_24656 >= 0.0));
            _24660 = select(float4(0.0), float4(_18886.xyz / float3(_18886.w), _18886.w), bool4(_18886.w > 9.9999997473787516355514526367188e-06));
            break;
        } while(false);
        float4 _24661;
        if ((in.i_flags.x & 64u) != 0u)
        {
            uint _18911 = (in.i_instance * 24u) + 18u;
            _24661 = _24660 * gTex.sample(gLinear, mix(gFxData_1._data[_18911].xy, gFxData_1._data[_18911].zw, (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875))));
        }
        else
        {
            _24661 = _24660;
        }
        float _18921 = fast::clamp(_24661.w * _3963, 0.0, 1.0);
        _24675 = float4(_24661.xyz * _18921, _18921) + (_24665 * (1.0 - _18921));
    }
    else
    {
        _24675 = _24665;
    }
    float4 _25311;
    if ((in.i_flags.x & 16384u) != 0u)
    {
        uint _18945 = (in.i_instance * 24u) + 21u;
        uint _18953 = (in.i_instance * 24u) + 3u;
        float2 _4403 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float2 _4410 = (in.i_local - in.i_rect.xy) / _4403;
        float _4421 = ((gFxData_1._data[_18953].x >= 0.0) ? gFxData_1._data[_18953].x : _178.gTime.x) * gFxData_1._data[_18945].z;
        float _4424 = _4410.x * 2.0;
        float _4425 = _4424 - 1.0;
        float _4430 = _4410.y * _4403.y;
        float _4433 = fast::max(gFxData_1._data[_18945].w, 0.001000000047497451305389404296875);
        float _4438 = fast::clamp(1.0 - (_4425 * _4425), 0.0, 1.0);
        float _4440 = pow(_4438, 1.2999999523162841796875);
        float _4442 = pow(_4438, 0.699999988079071044921875);
        float _4445 = fast::clamp(_4421 * 1.4285714626312255859375, 0.0, 1.0);
        float _4461 = (0.25 + (0.75 * ((_4445 * _4445) * (3.0 - (2.0 * _4445))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_4421 * 2.099999904632568359375)));
        float _4470 = ((gFxData_1._data[_18945].y * _4403.y) * _4461) * _4440;
        float _4496 = (_4403.y * (0.5 + ((gFxData_1._data[_18945].x * (0.5 - (_4425 * _4425))) * 0.5))) + (((0.14000000059604644775390625 * _4403.y) * _4440) * sin(((_4425 * 2.400000095367431640625) - (_4421 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _24671;
        float _24672;
        float3 _24673;
        _24673 = float3(0.0);
        _24672 = _4496;
        _24671 = _4496;
        float3 _4568;
        float _25716;
        float _25717;
        for (int _24670 = 0; _24670 < 4; _24673 = _4568, _24672 = _25717, _24671 = _25716, _24670++)
        {
            float _4520 = _4496 + ((_4470 * _2964[_24670].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_4421 * 1.7000000476837158203125) + _2981[_24670].y))));
            _25716 = (_24670 == 0) ? _4520 : _24671;
            _25717 = (_24670 == 2) ? _4520 : _24672;
            float _4535 = _4433 * _2964[_24670].y;
            float _4540 = (_4430 - _4520) / _4535;
            float _4545 = _4433 * _2964[_24670].z;
            float3 _25680;
            _25680 = float3(0.0);
            for (int _25679 = 0; _25679 < 6; )
            {
                float _18978 = ((_4430 - _4520) - (_4545 * ((float(_25679) * 0.4000000059604644775390625) - 1.0))) / _4535;
                _25680 += (_1668[_25679] * exp((-_18978) * _18978));
                _25679++;
                continue;
            }
            _4568 = _24673 + (mix(_25680 * float3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), float3(exp((-_4540) * _4540)), float3(_2981[_24670].x)) * (_2964[_24670].w * _4442));
        }
        float _4574 = _4433 * 1.5;
        float _4604 = fast::clamp((_4430 - _24671) / fast::max(_24672 - _24671, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _4632 = (_4430 - (_24672 - (_4433 * 3.0))) / (((_4403.y * 0.0900000035762786865234375) + (_4470 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _4635 = (_4424 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _4660 = ((_4430 - _24671) + (_4433 * 5.0)) / (_4433 * 7.0);
        float3 _4686 = float3(1.0) - exp((-((((_24673 + (mix(float3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), float3(1.0), float3(_4604)) * ((((1.0 / (1.0 + exp((-((_4430 - _24671) - (_4433 * 2.0))) / _4574))) / (1.0 + exp((-(_24672 - _4430)) / _4574))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_4604, 2.5)))) * _4442))) + (float3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_4632) * _4632) - (_4635 * _4635)) * (0.5 + (1.10000002384185791015625 * _4461))))) + (float3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_4660) * _4660) * _4440) * 0.100000001490116119384765625))) * mix(float3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), float3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), float3(0.5 + (0.5 * sin(_4421 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _4696 = (in.i_fill0.w * smoothstep(0.0, 0.119999997317790985107421875, _4410.y)) * smoothstep(1.0, 0.87999999523162841796875, _4410.y);
        float _4715 = (fast::clamp(fast::max(_4686.x, fast::max(_4686.y, _4686.z)), 0.0, 1.0) * _4696) * _3963;
        _25311 = float4((_4686 * _4696) * _3963, _4715) + (_24675 * (1.0 - _4715));
    }
    else
    {
        _25311 = _24675;
    }
    float4 _25320;
    if (((in.i_flags.x & 4u) != 0u) && ((in.i_flags.x & 256u) != 0u))
    {
        uint _19010 = (in.i_instance * 24u) + 7u;
        uint _19018 = (in.i_instance * 24u) + 8u;
        float2 _4749 = in.i_local - gFxData_1._data[_19018].zw;
        float2 _19059 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _19068 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _25286;
        if (in.i_flags.z == 1u)
        {
            float2 _19074 = _4749 - _19059;
            float _25285;
            do
            {
                if (in.i_shape.w >= 6.282185077667236328125)
                {
                    _25285 = abs(length(_19074) - in.i_radii.x) - in.i_radii.y;
                    break;
                }
                float _19173 = in.i_shape.z + (in.i_shape.w * 0.5);
                float _19175 = cos(_19173);
                float _19177 = sin(_19173);
                float _19186 = dot(_19074, float2(-_19177, _19175));
                float _19189 = dot(_19074, float2(_19175, _19177));
                float2 _19190 = float2(_19186, _19189);
                float _19193 = abs(_19186);
                _19190.x = _19193;
                float _19196 = in.i_shape.w * 0.5;
                float _19198 = sin(_19196);
                float _19200 = cos(_19196);
                _25285 = (((_19200 * _19193) > (_19198 * _19189)) ? length(_19190 - (float2(_19198, _19200) * in.i_radii.x)) : abs(length(_19190) - in.i_radii.x)) - in.i_radii.y;
                break;
            } while(false);
            _25286 = _25285;
        }
        else
        {
            float _25287;
            if (in.i_flags.z == 2u)
            {
                float2 _19236 = _4749 - _22199.xy;
                float2 _19239 = _22199.zw - _22199.xy;
                _25287 = length(_19236 - (_19239 * fast::clamp(dot(_19236, _19239) / fast::max(dot(_19239, _19239), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
            }
            else
            {
                float2 _19101 = _4749 - _19059;
                float _19292 = fast::min(_19068.x, _19068.y);
                float _19295 = fast::min((_19101.x > 0.0) ? ((_19101.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_19101.y > 0.0) ? in.i_radii.w : in.i_radii.x), _19292);
                float _19301 = _19295 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                float _25270;
                float _25271;
                if (_19301 > _19292)
                {
                    float _19315 = in.i_shape.y * fast::clamp((_19292 - _19295) / fast::max(0.60000002384185791015625 * _19295, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _25271 = _19315;
                    _25270 = _19295 * (1.0 + (0.60000002384185791015625 * _19315));
                }
                else
                {
                    _25271 = in.i_shape.y;
                    _25270 = _19301;
                }
                float2 _19328 = (abs(_19101) - _19068) + float2(_25270);
                float2 _19330 = fast::max(_19328, float2(0.0));
                float _25272;
                if ((_19330.x > 0.0) && (_19330.y > 0.0))
                {
                    float _25273;
                    if ((_25271 > 0.001000000047497451305389404296875) && (_25270 > 9.9999997473787516355514526367188e-05))
                    {
                        float _19347 = 2.0 + (2.0 * _25271);
                        float2 _19352 = _19330 / float2(fast::max(_25270, 9.9999997473787516355514526367188e-05));
                        _25273 = pow(pow(_19352.x, _19347) + pow(_19352.y, _19347), 1.0 / _19347) * _25270;
                    }
                    else
                    {
                        _25273 = length(_19330);
                    }
                    _25272 = _25273;
                }
                else
                {
                    _25272 = fast::max(_19330.x, _19330.y);
                }
                float _19387 = (fast::min(fast::max(_19328.x, _19328.y), 0.0) + _25272) - _25270;
                float _25288;
                if ((in.i_flags.x & 512u) != 0u)
                {
                    float2 _19129 = fast::max((_22199.zw - _22199.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _19132 = _4749 - ((_22199.xy + _22199.zw) * 0.5);
                    float _19423 = fast::min(_19129.x, _19129.y);
                    float _19426 = fast::min((_19132.x > 0.0) ? ((_19132.y > 0.0) ? _24678.x : _24678.x) : ((_19132.y > 0.0) ? _24678.x : _24678.x), _19423);
                    float _19432 = _19426 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _25276;
                    float _25277;
                    if (_19432 > _19423)
                    {
                        float _19446 = in.i_shape.y * fast::clamp((_19423 - _19426) / fast::max(0.60000002384185791015625 * _19426, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _25277 = _19446;
                        _25276 = _19426 * (1.0 + (0.60000002384185791015625 * _19446));
                    }
                    else
                    {
                        _25277 = in.i_shape.y;
                        _25276 = _19432;
                    }
                    float2 _19459 = (abs(_19132) - _19129) + float2(_25276);
                    float2 _19461 = fast::max(_19459, float2(0.0));
                    float _25278;
                    if ((_19461.x > 0.0) && (_19461.y > 0.0))
                    {
                        float _25279;
                        if ((_25277 > 0.001000000047497451305389404296875) && (_25276 > 9.9999997473787516355514526367188e-05))
                        {
                            float _19478 = 2.0 + (2.0 * _25277);
                            float2 _19483 = _19461 / float2(fast::max(_25276, 9.9999997473787516355514526367188e-05));
                            _25279 = pow(pow(_19483.x, _19478) + pow(_19483.y, _19478), 1.0 / _19478) * _25276;
                        }
                        else
                        {
                            _25279 = length(_19461);
                        }
                        _25278 = _25279;
                    }
                    else
                    {
                        _25278 = fast::max(_19461.x, _19461.y);
                    }
                    float _19518 = (fast::min(fast::max(_19459.x, _19459.y), 0.0) + _25278) - _25276;
                    float _19523 = fast::max(_24678.y, 9.9999997473787516355514526367188e-05);
                    float _19532 = fast::max(_19523 - abs(_19387 - _19518), 0.0) / _19523;
                    _25288 = fast::min(_19387, _19518) - (((_19532 * _19532) * _19523) * 0.25);
                }
                else
                {
                    _25288 = _19387;
                }
                _25287 = _25288;
            }
            _25286 = _25287;
        }
        float _4758 = (_25286 + gFxData_1._data[_19018].y) / (fast::max(gFxData_1._data[_19018].x * 0.5, _3938 * 0.5) * 1.41421353816986083984375);
        float _19549 = sign(_4758);
        float _19551 = abs(_4758);
        float _19562 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_19551 * _19551))) * _19551)) * _19551);
        float _19565 = _19562 * _19562;
        float _19580 = fast::clamp(gFxData_1._data[_19010].w * ((0.5 + (0.5 * (_19549 - (_19549 / (_19565 * _19565))))) * _3963), 0.0, 1.0);
        _25320 = float4(gFxData_1._data[_19010].xyz * _19580, _19580) + (_25311 * (1.0 - _19580));
    }
    else
    {
        _25320 = _25311;
    }
    float4 _25344;
    if ((in.i_flags.x & 16u) != 0u)
    {
        uint _19604 = (in.i_instance * 24u) + 9u;
        uint _19612 = (in.i_instance * 24u) + 10u;
        float _4790 = fast::max(-_22217, 0.0) / fast::max(gFxData_1._data[_19612].z, 0.001000000047497451305389404296875);
        float _19622 = fast::clamp(gFxData_1._data[_19604].w * fast::clamp((exp(((-_4790) * _4790) * 2.2000000476837158203125) * gFxData_1._data[_19612].w) * _3963, 0.0, 1.0), 0.0, 1.0);
        _25344 = float4(gFxData_1._data[_19604].xyz * _19622, _19622) + (_25320 * (1.0 - _19622));
    }
    else
    {
        _25344 = _25320;
    }
    float3 _4821 = _25344.xyz + float3(_25325 * fast::clamp(_25344.w / fast::max(_3963, 0.001000000047497451305389404296875), 0.0, 1.0));
    float4 _22092 = _25344;
    _22092.x = _4821.x;
    _22092.y = _4821.y;
    _22092.z = _4821.z;
    float4 _25347;
    if ((in.i_flags.x & 2u) != 0u)
    {
        uint _19646 = (in.i_instance * 24u) + 5u;
        float4 _19648 = gFxData_1._data[_19646];
        uint _19654 = (in.i_instance * 24u) + 6u;
        float4 _25345;
        if (gFxData_1._data[_19654].z < 0.999000012874603271484375)
        {
            float2 _4879 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float _4890 = cos(gFxData_1._data[_19654].w);
            float _4893 = sin(gFxData_1._data[_19654].w);
            float4 _22109 = _19648;
            _22109.w = _19648.w * mix(1.0, gFxData_1._data[_19654].z, fast::clamp(((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_4890, _4893)) / fast::max(0.5 * ((abs(_4890) * _4879.x) + (abs(_4893) * _4879.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _25345 = _22109;
        }
        else
        {
            _25345 = _19648;
        }
        float _19664 = fast::clamp(_25345.w * (fast::clamp(0.5 - ((_22217 - (gFxData_1._data[_19654].x * gFxData_1._data[_19654].y)) / _3938), 0.0, 1.0) - fast::clamp(0.5 - ((_22217 + (gFxData_1._data[_19654].x * (1.0 - gFxData_1._data[_19654].y))) / _3938), 0.0, 1.0)), 0.0, 1.0);
        _25347 = float4(_25345.xyz * _19664, _19664) + (_22092 * (1.0 - _19664));
    }
    else
    {
        _25347 = _22092;
    }
    float4 _25348;
    if ((in.i_flags.x & 128u) != 0u)
    {
        float2 _4954 = (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float _4976 = exp(-pow((((_4954.x * 0.85000002384185791015625) + (_4954.y * 0.1500000059604644775390625)) - ((fract(_178.gTime.x * in.i_misc.w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _25348 = float4(_25347.xyz + float3(((_4976 * in.i_misc.z) * _3963) * fast::max(_25347.w, 0.3499999940395355224609375)), fast::max(_25347.w, ((_4976 * in.i_misc.z) * _3963) * 0.5));
    }
    else
    {
        _25348 = _25347;
    }
    float4 _25674;
    if ((in.i_flags.x & 2048u) != 0u)
    {
        float3 _19689 = fract(floor(_22196).xyx * 0.103100001811981201171875);
        float3 _19698 = _19689 + float3(dot(_19689, _19689.yzx + float3(33.3300018310546875)));
        float3 _5027 = _25348.xyz + float3(((fract((_19698.x + _19698.y) * _19698.z) - 0.5) * in.i_misc.y) * _25348.w);
        float4 _22133 = _25348;
        _22133.x = _5027.x;
        _22133.y = _5027.y;
        _22133.z = _5027.z;
        _25674 = _22133;
    }
    else
    {
        _25674 = _25348;
    }
    float _25670;
    if (_221.gFade.z > 0.0)
    {
        _25670 = smoothstep(0.0, 1.0, fast::clamp((_22196.y - _221.gFade.x) / _221.gFade.z, 0.0, 1.0));
    }
    else
    {
        _25670 = 1.0;
    }
    float _25671;
    if (_221.gFade.w > 0.0)
    {
        _25671 = _25670 * smoothstep(0.0, 1.0, fast::clamp((_221.gFade.y - _22196.y) / _221.gFade.w, 0.0, 1.0));
    }
    else
    {
        _25671 = _25670;
    }
    float4 _5044 = _25674 * ((in.i_misc.x * _25359) * _25671);
    float4 _25675;
    if (((in.i_flags.x & 8u) != 0u) || ((in.i_flags.x & 4u) != 0u))
    {
        float3 _19750 = fract((floor(_22196) + float2(17.0)).xyx * 0.103100001811981201171875);
        float3 _19759 = _19750 + float3(dot(_19750, _19750.yzx + float3(33.3300018310546875)));
        float3 _5068 = _5044.xyz + float3(((fract((_19759.x + _19759.y) * _19759.z) - 0.5) * 0.0039215688593685626983642578125) * fast::clamp(_5044.w * 8.0, 0.0, 1.0));
        float4 _22145 = _5044;
        _22145.x = _5068.x;
        _22145.y = _5068.y;
        _22145.z = _5068.z;
        _25675 = _22145;
    }
    else
    {
        _25675 = _5044;
    }
    float3 _5078 = fast::max(_25675.xyz, float3(0.0));
    float4 _22151 = _25675;
    _22151.x = _5078.x;
    _22151.y = _5078.y;
    _22151.z = _5078.z;
    float4 _25676;
    if ((_178.gTime.w > 0.5) && (_25675.w > 9.9999997473787516355514526367188e-06))
    {
        float3 _19803 = fast::clamp(_22151.xyz / float3(_25675.w), float3(0.0), float3(1.0));
        float3 _19789 = select(pow((_19803 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _19803 * float3(0.077399380505084991455078125), _19803 <= float3(0.040449999272823333740234375)) * _25675.w;
        float4 _22160 = _22151;
        _22160.x = _19789.x;
        _22160.y = _19789.y;
        _22160.z = _19789.z;
        _25676 = _22160;
    }
    else
    {
        _25676 = _22151;
    }
    out._entryPointOutput = _25676;
    return out;
}

