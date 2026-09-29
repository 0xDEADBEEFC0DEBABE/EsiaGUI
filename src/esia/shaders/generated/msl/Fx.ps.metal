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

constant spvUnsafeArray<float3, 6> _1662 = spvUnsafeArray<float3, 6>({ float3(1.0, 0.4199999868869781494140625, 0.2199999988079071044921875), float3(1.0, 0.699999988079071044921875, 0.300000011920928955078125), float3(0.800000011920928955078125, 0.920000016689300537109375, 0.4000000059604644775390625), float3(0.3499999940395355224609375, 0.89999997615814208984375, 0.699999988079071044921875), float3(0.4000000059604644775390625, 0.62000000476837158203125, 1.0), float3(0.660000026226043701171875, 0.5, 1.0) });
constant spvUnsafeArray<float4, 4> _2880 = spvUnsafeArray<float4, 4>({ float4(-1.0, 1.0, 3.400000095367431640625, 2.599999904632568359375), float4(-0.550000011920928955078125, 0.800000011920928955078125, 2.0, 0.800000011920928955078125), float4(0.300000011920928955078125, 1.0, 1.2000000476837158203125, 1.2999999523162841796875), float4(0.62000000476837158203125, 0.800000011920928955078125, 1.60000002384185791015625, 0.449999988079071044921875) });
constant spvUnsafeArray<float2, 4> _2897 = spvUnsafeArray<float2, 4>({ float2(0.0), float2(0.100000001490116119384765625, 1.2999999523162841796875), float2(0.550000011920928955078125, 3.900000095367431640625), float2(0.20000000298023223876953125, 5.19999980926513671875) });

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

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _172 [[buffer(0)]], constant WgtDraw& _215 [[buffer(2)]], const device gFxData& gFxData_1 [[buffer(10)]], texture2d<float> gTex [[texture(3)]], texture2d<float> gBackdrop0 [[texture(4)]], texture2d<float> gBackdrop1 [[texture(5)]], texture2d<float> gBackdrop2 [[texture(6)]], texture2d<float> gBackdrop3 [[texture(7)]], texture2d<float> gBackdrop4 [[texture(8)]], texture2d<float> gBackdrop5 [[texture(9)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float2 _5044 = gl_FragCoord.xy + _172.gConv.yy;
    float2 _22020;
    if (_172.gConv.x > 0.5)
    {
        float2 _20331 = _5044;
        _20331.y = _172.gTarget.y - _5044.y;
        _22020 = _20331;
    }
    else
    {
        _22020 = _5044;
    }
    float _3839 = dfdx(in.i_local.x);
    float _3843 = dfdy(in.i_local.x);
    float _3846 = fast::max(abs(_3839) + abs(_3843), 9.9999997473787516355514526367188e-05);
    float4 _22021;
    float4 _22023;
    if ((in.i_flags.z == 2u) || ((in.i_flags.x & 512u) != 0u))
    {
        _22023 = gFxData_1._data[(in.i_instance * 24u) + 15u];
        _22021 = gFxData_1._data[(in.i_instance * 24u) + 16u];
    }
    else
    {
        _22023 = float4(0.0);
        _22021 = float4(0.0);
    }
    float2 _5111 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
    float2 _5120 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
    float _22041;
    if (in.i_flags.z == 1u)
    {
        float2 _5126 = in.i_local - _5111;
        float _22040;
        do
        {
            if (in.i_shape.w >= 6.282185077667236328125)
            {
                _22040 = abs(length(_5126) - in.i_radii.x) - in.i_radii.y;
                break;
            }
            float _5225 = in.i_shape.z + (in.i_shape.w * 0.5);
            float _5227 = cos(_5225);
            float _5229 = sin(_5225);
            float _5238 = dot(_5126, float2(-_5229, _5227));
            float _5241 = dot(_5126, float2(_5227, _5229));
            float2 _5242 = float2(_5238, _5241);
            float _5245 = abs(_5238);
            _5242.x = _5245;
            float _5248 = in.i_shape.w * 0.5;
            float _5250 = sin(_5248);
            float _5252 = cos(_5248);
            _22040 = (((_5252 * _5245) > (_5250 * _5241)) ? length(_5242 - (float2(_5250, _5252) * in.i_radii.x)) : abs(length(_5242) - in.i_radii.x)) - in.i_radii.y;
            break;
        } while(false);
        _22041 = _22040;
    }
    else
    {
        float _22042;
        if (in.i_flags.z == 2u)
        {
            float2 _5288 = in.i_local - _22023.xy;
            float2 _5291 = _22023.zw - _22023.xy;
            _22042 = length(_5288 - (_5291 * fast::clamp(dot(_5288, _5291) / fast::max(dot(_5291, _5291), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
        }
        else
        {
            float2 _5153 = in.i_local - _5111;
            float _5344 = fast::min(_5120.x, _5120.y);
            float _5347 = fast::min((_5153.x > 0.0) ? ((_5153.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_5153.y > 0.0) ? in.i_radii.w : in.i_radii.x), _5344);
            float _5353 = _5347 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
            float _22025;
            float _22026;
            if (_5353 > _5344)
            {
                float _5367 = in.i_shape.y * fast::clamp((_5344 - _5347) / fast::max(0.60000002384185791015625 * _5347, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                _22026 = _5367;
                _22025 = _5347 * (1.0 + (0.60000002384185791015625 * _5367));
            }
            else
            {
                _22026 = in.i_shape.y;
                _22025 = _5353;
            }
            float2 _5380 = (abs(_5153) - _5120) + float2(_22025);
            float2 _5382 = fast::max(_5380, float2(0.0));
            float _22027;
            if ((_5382.x > 0.0) && (_5382.y > 0.0))
            {
                float _22028;
                if ((_22026 > 0.001000000047497451305389404296875) && (_22025 > 9.9999997473787516355514526367188e-05))
                {
                    float _5399 = 2.0 + (2.0 * _22026);
                    float2 _5404 = _5382 / float2(fast::max(_22025, 9.9999997473787516355514526367188e-05));
                    _22028 = pow(pow(_5404.x, _5399) + pow(_5404.y, _5399), 1.0 / _5399) * _22025;
                }
                else
                {
                    _22028 = length(_5382);
                }
                _22027 = _22028;
            }
            else
            {
                _22027 = fast::max(_5382.x, _5382.y);
            }
            float _5439 = (fast::min(fast::max(_5380.x, _5380.y), 0.0) + _22027) - _22025;
            float _22043;
            if ((in.i_flags.x & 512u) != 0u)
            {
                float2 _5181 = fast::max((_22023.zw - _22023.xy) * 0.5, float2(0.001000000047497451305389404296875));
                float2 _5184 = in.i_local - ((_22023.xy + _22023.zw) * 0.5);
                float _5475 = fast::min(_5181.x, _5181.y);
                float _5478 = fast::min((_5184.x > 0.0) ? ((_5184.y > 0.0) ? _22021.x : _22021.x) : ((_5184.y > 0.0) ? _22021.x : _22021.x), _5475);
                float _5484 = _5478 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                float _22031;
                float _22032;
                if (_5484 > _5475)
                {
                    float _5498 = in.i_shape.y * fast::clamp((_5475 - _5478) / fast::max(0.60000002384185791015625 * _5478, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _22032 = _5498;
                    _22031 = _5478 * (1.0 + (0.60000002384185791015625 * _5498));
                }
                else
                {
                    _22032 = in.i_shape.y;
                    _22031 = _5484;
                }
                float2 _5511 = (abs(_5184) - _5181) + float2(_22031);
                float2 _5513 = fast::max(_5511, float2(0.0));
                float _22033;
                if ((_5513.x > 0.0) && (_5513.y > 0.0))
                {
                    float _22034;
                    if ((_22032 > 0.001000000047497451305389404296875) && (_22031 > 9.9999997473787516355514526367188e-05))
                    {
                        float _5530 = 2.0 + (2.0 * _22032);
                        float2 _5535 = _5513 / float2(fast::max(_22031, 9.9999997473787516355514526367188e-05));
                        _22034 = pow(pow(_5535.x, _5530) + pow(_5535.y, _5530), 1.0 / _5530) * _22031;
                    }
                    else
                    {
                        _22034 = length(_5513);
                    }
                    _22033 = _22034;
                }
                else
                {
                    _22033 = fast::max(_5513.x, _5513.y);
                }
                float _5570 = (fast::min(fast::max(_5511.x, _5511.y), 0.0) + _22033) - _22031;
                float _5575 = fast::max(_22021.y, 9.9999997473787516355514526367188e-05);
                float _5584 = fast::max(_5575 - abs(_5439 - _5570), 0.0) / _5575;
                _22043 = fast::min(_5439, _5570) - (((_5584 * _5584) * _5575) * 0.25);
            }
            else
            {
                _22043 = _5439;
            }
            _22042 = _22043;
        }
        _22041 = _22042;
    }
    float _3871 = fast::clamp(0.5 - (_22041 / _3846), 0.0, 1.0);
    float _25175;
    if ((in.i_flags.x & 1024u) != 0u)
    {
        uint _5600 = (in.i_instance * 24u) + 19u;
        uint _5608 = (in.i_instance * 24u) + 20u;
        float2 _3900 = fast::max((gFxData_1._data[_5600].zw - gFxData_1._data[_5600].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float2 _3903 = in.i_local - ((gFxData_1._data[_5600].xy + gFxData_1._data[_5600].zw) * 0.5);
        float _5646 = fast::min(_3900.x, _3900.y);
        float _5649 = fast::min((_3903.x > 0.0) ? ((_3903.y > 0.0) ? gFxData_1._data[_5608].x : gFxData_1._data[_5608].x) : ((_3903.y > 0.0) ? gFxData_1._data[_5608].x : gFxData_1._data[_5608].x), _5646);
        float _5655 = _5649 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_5608].y));
        float _22044;
        float _22045;
        if (_5655 > _5646)
        {
            float _5669 = gFxData_1._data[_5608].y * fast::clamp((_5646 - _5649) / fast::max(0.60000002384185791015625 * _5649, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _22045 = _5669;
            _22044 = _5649 * (1.0 + (0.60000002384185791015625 * _5669));
        }
        else
        {
            _22045 = gFxData_1._data[_5608].y;
            _22044 = _5655;
        }
        float2 _5682 = (abs(_3903) - _3900) + float2(_22044);
        float2 _5684 = fast::max(_5682, float2(0.0));
        float _22046;
        if ((_5684.x > 0.0) && (_5684.y > 0.0))
        {
            float _22047;
            if ((_22045 > 0.001000000047497451305389404296875) && (_22044 > 9.9999997473787516355514526367188e-05))
            {
                float _5701 = 2.0 + (2.0 * _22045);
                float2 _5706 = _5684 / float2(fast::max(_22044, 9.9999997473787516355514526367188e-05));
                _22047 = pow(pow(_5706.x, _5701) + pow(_5706.y, _5701), 1.0 / _5701) * _22044;
            }
            else
            {
                _22047 = length(_5684);
            }
            _22046 = _22047;
        }
        else
        {
            _22046 = fast::max(_5684.x, _5684.y);
        }
        float _3915 = fast::clamp(0.5 - (((fast::min(fast::max(_5682.x, _5682.y), 0.0) + _22046) - _22044) / _3846), 0.0, 1.0);
        if (_3915 <= 0.0)
        {
            discard_fragment();
        }
        _25175 = _3915;
    }
    else
    {
        _25175 = 1.0;
    }
    bool _3926 = ((in.i_flags.x & 32u) != 0u) && (_3871 >= 0.999000012874603271484375);
    float4 _22136;
    if ((((in.i_flags.x & 4u) != 0u) && (!((in.i_flags.x & 256u) != 0u))) && (!_3926))
    {
        uint _5747 = (in.i_instance * 24u) + 7u;
        uint _5755 = (in.i_instance * 24u) + 8u;
        float2 _3957 = in.i_local - gFxData_1._data[_5755].zw;
        float2 _5796 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _5805 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _22094;
        if (in.i_flags.z == 1u)
        {
            float2 _5811 = _3957 - _5796;
            float _22093;
            do
            {
                if (in.i_shape.w >= 6.282185077667236328125)
                {
                    _22093 = abs(length(_5811) - in.i_radii.x) - in.i_radii.y;
                    break;
                }
                float _5910 = in.i_shape.z + (in.i_shape.w * 0.5);
                float _5912 = cos(_5910);
                float _5914 = sin(_5910);
                float _5923 = dot(_5811, float2(-_5914, _5912));
                float _5926 = dot(_5811, float2(_5912, _5914));
                float2 _5927 = float2(_5923, _5926);
                float _5930 = abs(_5923);
                _5927.x = _5930;
                float _5933 = in.i_shape.w * 0.5;
                float _5935 = sin(_5933);
                float _5937 = cos(_5933);
                _22093 = (((_5937 * _5930) > (_5935 * _5926)) ? length(_5927 - (float2(_5935, _5937) * in.i_radii.x)) : abs(length(_5927) - in.i_radii.x)) - in.i_radii.y;
                break;
            } while(false);
            _22094 = _22093;
        }
        else
        {
            float _22095;
            if (in.i_flags.z == 2u)
            {
                float2 _5973 = _3957 - _22023.xy;
                float2 _5976 = _22023.zw - _22023.xy;
                _22095 = length(_5973 - (_5976 * fast::clamp(dot(_5973, _5976) / fast::max(dot(_5976, _5976), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
            }
            else
            {
                float2 _5838 = _3957 - _5796;
                float _6029 = fast::min(_5805.x, _5805.y);
                float _6032 = fast::min((_5838.x > 0.0) ? ((_5838.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_5838.y > 0.0) ? in.i_radii.w : in.i_radii.x), _6029);
                float _6038 = _6032 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                float _22078;
                float _22079;
                if (_6038 > _6029)
                {
                    float _6052 = in.i_shape.y * fast::clamp((_6029 - _6032) / fast::max(0.60000002384185791015625 * _6032, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _22079 = _6052;
                    _22078 = _6032 * (1.0 + (0.60000002384185791015625 * _6052));
                }
                else
                {
                    _22079 = in.i_shape.y;
                    _22078 = _6038;
                }
                float2 _6065 = (abs(_5838) - _5805) + float2(_22078);
                float2 _6067 = fast::max(_6065, float2(0.0));
                float _22080;
                if ((_6067.x > 0.0) && (_6067.y > 0.0))
                {
                    float _22081;
                    if ((_22079 > 0.001000000047497451305389404296875) && (_22078 > 9.9999997473787516355514526367188e-05))
                    {
                        float _6084 = 2.0 + (2.0 * _22079);
                        float2 _6089 = _6067 / float2(fast::max(_22078, 9.9999997473787516355514526367188e-05));
                        _22081 = pow(pow(_6089.x, _6084) + pow(_6089.y, _6084), 1.0 / _6084) * _22078;
                    }
                    else
                    {
                        _22081 = length(_6067);
                    }
                    _22080 = _22081;
                }
                else
                {
                    _22080 = fast::max(_6067.x, _6067.y);
                }
                float _6124 = (fast::min(fast::max(_6065.x, _6065.y), 0.0) + _22080) - _22078;
                float _22096;
                if ((in.i_flags.x & 512u) != 0u)
                {
                    float2 _5866 = fast::max((_22023.zw - _22023.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _5869 = _3957 - ((_22023.xy + _22023.zw) * 0.5);
                    float _6160 = fast::min(_5866.x, _5866.y);
                    float _6163 = fast::min((_5869.x > 0.0) ? ((_5869.y > 0.0) ? _22021.x : _22021.x) : ((_5869.y > 0.0) ? _22021.x : _22021.x), _6160);
                    float _6169 = _6163 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _22084;
                    float _22085;
                    if (_6169 > _6160)
                    {
                        float _6183 = in.i_shape.y * fast::clamp((_6160 - _6163) / fast::max(0.60000002384185791015625 * _6163, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _22085 = _6183;
                        _22084 = _6163 * (1.0 + (0.60000002384185791015625 * _6183));
                    }
                    else
                    {
                        _22085 = in.i_shape.y;
                        _22084 = _6169;
                    }
                    float2 _6196 = (abs(_5869) - _5866) + float2(_22084);
                    float2 _6198 = fast::max(_6196, float2(0.0));
                    float _22086;
                    if ((_6198.x > 0.0) && (_6198.y > 0.0))
                    {
                        float _22087;
                        if ((_22085 > 0.001000000047497451305389404296875) && (_22084 > 9.9999997473787516355514526367188e-05))
                        {
                            float _6215 = 2.0 + (2.0 * _22085);
                            float2 _6220 = _6198 / float2(fast::max(_22084, 9.9999997473787516355514526367188e-05));
                            _22087 = pow(pow(_6220.x, _6215) + pow(_6220.y, _6215), 1.0 / _6215) * _22084;
                        }
                        else
                        {
                            _22087 = length(_6198);
                        }
                        _22086 = _22087;
                    }
                    else
                    {
                        _22086 = fast::max(_6198.x, _6198.y);
                    }
                    float _6255 = (fast::min(fast::max(_6196.x, _6196.y), 0.0) + _22086) - _22084;
                    float _6260 = fast::max(_22021.y, 9.9999997473787516355514526367188e-05);
                    float _6269 = fast::max(_6260 - abs(_6124 - _6255), 0.0) / _6260;
                    _22096 = fast::min(_6124, _6255) - (((_6269 * _6269) * _6260) * 0.25);
                }
                else
                {
                    _22096 = _6124;
                }
                _22095 = _22096;
            }
            _22094 = _22095;
        }
        float _3966 = (_22094 - gFxData_1._data[_5755].y) / (fast::max(gFxData_1._data[_5755].x * 0.5, _3846 * 0.5) * 1.41421353816986083984375);
        float _6286 = sign(_3966);
        float _6288 = abs(_3966);
        float _6299 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_6288 * _6288))) * _6288)) * _6288);
        float _6302 = _6299 * _6299;
        float _6317 = fast::clamp(gFxData_1._data[_5747].w * (0.5 - (0.5 * (_6286 - (_6286 / (_6302 * _6302))))), 0.0, 1.0);
        _22136 = float4(gFxData_1._data[_5747].xyz * _6317, _6317);
    }
    else
    {
        _22136 = float4(0.0);
    }
    float4 _23575;
    if (((in.i_flags.x & 8u) != 0u) && (!_3926))
    {
        uint _6341 = (in.i_instance * 24u) + 9u;
        uint _6349 = (in.i_instance * 24u) + 10u;
        float _3994 = fast::max(gFxData_1._data[_6349].x, 0.001000000047497451305389404296875);
        float _22129;
        float _22132;
        if ((in.i_flags.x & 8192u) != 0u)
        {
            uint _6357 = (in.i_instance * 24u) + 22u;
            float2 _4010 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _4019 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float2 _4026 = in.i_rect.xy - gFxData_1._data[_6357].xy;
            float2 _4033 = gFxData_1._data[_6357].zw - in.i_rect.zw;
            float2 _4063 = fast::clamp(float2((in.i_local.x < _4010.x) ? _4026.x : _4033.x, (in.i_local.y < _4010.y) ? _4026.y : _4033.y) * float2(0.58823525905609130859375), float2(fast::min(_3994, 1.5)), float2(_3994));
            float _4068 = fast::min(_4019.x, _4019.y);
            float _22127;
            if (in.i_flags.z == 0u)
            {
                _22127 = fast::min(((in.i_local.x > _4010.x) ? ((in.i_local.y > _4010.y) ? in.i_radii.z : in.i_radii.y) : ((in.i_local.y > _4010.y) ? in.i_radii.w : in.i_radii.x)) * (1.0 + (0.60000002384185791015625 * in.i_shape.y)), _4068);
            }
            else
            {
                _22127 = _4068;
            }
            float2 _4117 = fast::max(abs(in.i_local - _4010) - (_4019 - float2(_22127)), float2(0.0));
            float _4119 = length(_4117);
            float2 _4127 = select(float2(0.707099974155426025390625), _4117 / float2(_4119), bool2(_4119 > 9.9999997473787516355514526367188e-05));
            float2 _4142 = in.i_local - gFxData_1._data[_6357].xy;
            float2 _4147 = gFxData_1._data[_6357].zw - in.i_local;
            _22132 = fast::clamp(fast::min(fast::min(_4142.x, _4142.y), fast::min(_4147.x, _4147.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _22129 = rsqrt(dot(_4127 * _4127, float2(1.0) / (_4063 * _4063)));
        }
        else
        {
            _22132 = 1.0;
            _22129 = _3994;
        }
        float _4165 = fast::max(_22041, 0.0) / _22129;
        float _6367 = fast::clamp(gFxData_1._data[_6341].w * fast::clamp((exp(((-_4165) * _4165) * 2.2000000476837158203125) * gFxData_1._data[_6349].y) * _22132, 0.0, 1.0), 0.0, 1.0);
        _23575 = float4(gFxData_1._data[_6341].xyz * _6367, _6367) + (_22136 * (1.0 - _6367));
    }
    else
    {
        _23575 = _22136;
    }
    float4 _24483;
    float4 _24496;
    float _25141;
    if (((in.i_flags.x & 32u) != 0u) && (_3871 > 0.0))
    {
        uint _6391 = (in.i_instance * 24u) + 11u;
        uint _6399 = (in.i_instance * 24u) + 12u;
        uint _6407 = (in.i_instance * 24u) + 13u;
        uint _6415 = (in.i_instance * 24u) + 16u;
        float _6477 = gFxData_1._data[_6391].x * _172.gDisplay.z;
        float2 _6488 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(1.0));
        float _6496 = fast::clamp(gFxData_1._data[_6391].z, 0.001000000047497451305389404296875, fast::min(_6488.x, _6488.y));
        float _6505 = fast::clamp(1.0 - (fast::max(-_22041, 0.0) / _6496), 0.0, 1.0);
        float _6511 = sqrt(fast::clamp(1.0 - (_6505 * _6505), 0.0, 1.0));
        float2 _22228;
        float3 _23090;
        if (_6505 > 0.0)
        {
            float2 _6857 = in.i_local + float2(0.5, 0.0);
            float2 _6925 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _6934 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _22168;
            if (in.i_flags.z == 1u)
            {
                float2 _6940 = _6857 - _6925;
                float _22167;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _22167 = abs(length(_6940) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _7039 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _7041 = cos(_7039);
                    float _7043 = sin(_7039);
                    float _7052 = dot(_6940, float2(-_7043, _7041));
                    float _7055 = dot(_6940, float2(_7041, _7043));
                    float2 _7056 = float2(_7052, _7055);
                    float _7059 = abs(_7052);
                    _7056.x = _7059;
                    float _7062 = in.i_shape.w * 0.5;
                    float _7064 = sin(_7062);
                    float _7066 = cos(_7062);
                    _22167 = (((_7066 * _7059) > (_7064 * _7055)) ? length(_7056 - (float2(_7064, _7066) * in.i_radii.x)) : abs(length(_7056) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _22168 = _22167;
            }
            else
            {
                float _22169;
                if (in.i_flags.z == 2u)
                {
                    float2 _7102 = _6857 - _22023.xy;
                    float2 _7105 = _22023.zw - _22023.xy;
                    _22169 = length(_7102 - (_7105 * fast::clamp(dot(_7102, _7105) / fast::max(dot(_7105, _7105), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _6967 = _6857 - _6925;
                    float _7158 = fast::min(_6934.x, _6934.y);
                    float _7161 = fast::min((_6967.x > 0.0) ? ((_6967.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_6967.y > 0.0) ? in.i_radii.w : in.i_radii.x), _7158);
                    float _7167 = _7161 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _22152;
                    float _22153;
                    if (_7167 > _7158)
                    {
                        float _7181 = in.i_shape.y * fast::clamp((_7158 - _7161) / fast::max(0.60000002384185791015625 * _7161, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _22153 = _7181;
                        _22152 = _7161 * (1.0 + (0.60000002384185791015625 * _7181));
                    }
                    else
                    {
                        _22153 = in.i_shape.y;
                        _22152 = _7167;
                    }
                    float2 _7194 = (abs(_6967) - _6934) + float2(_22152);
                    float2 _7196 = fast::max(_7194, float2(0.0));
                    float _22154;
                    if ((_7196.x > 0.0) && (_7196.y > 0.0))
                    {
                        float _22155;
                        if ((_22153 > 0.001000000047497451305389404296875) && (_22152 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7213 = 2.0 + (2.0 * _22153);
                            float2 _7218 = _7196 / float2(fast::max(_22152, 9.9999997473787516355514526367188e-05));
                            _22155 = pow(pow(_7218.x, _7213) + pow(_7218.y, _7213), 1.0 / _7213) * _22152;
                        }
                        else
                        {
                            _22155 = length(_7196);
                        }
                        _22154 = _22155;
                    }
                    else
                    {
                        _22154 = fast::max(_7196.x, _7196.y);
                    }
                    float _7253 = (fast::min(fast::max(_7194.x, _7194.y), 0.0) + _22154) - _22152;
                    float _22170;
                    if ((in.i_flags.x & 512u) != 0u)
                    {
                        float2 _6995 = fast::max((_22023.zw - _22023.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _6998 = _6857 - ((_22023.xy + _22023.zw) * 0.5);
                        float _7289 = fast::min(_6995.x, _6995.y);
                        float _7292 = fast::min((_6998.x > 0.0) ? ((_6998.y > 0.0) ? gFxData_1._data[_6415].x : gFxData_1._data[_6415].x) : ((_6998.y > 0.0) ? gFxData_1._data[_6415].x : gFxData_1._data[_6415].x), _7289);
                        float _7298 = _7292 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _22158;
                        float _22159;
                        if (_7298 > _7289)
                        {
                            float _7312 = in.i_shape.y * fast::clamp((_7289 - _7292) / fast::max(0.60000002384185791015625 * _7292, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _22159 = _7312;
                            _22158 = _7292 * (1.0 + (0.60000002384185791015625 * _7312));
                        }
                        else
                        {
                            _22159 = in.i_shape.y;
                            _22158 = _7298;
                        }
                        float2 _7325 = (abs(_6998) - _6995) + float2(_22158);
                        float2 _7327 = fast::max(_7325, float2(0.0));
                        float _22160;
                        if ((_7327.x > 0.0) && (_7327.y > 0.0))
                        {
                            float _22161;
                            if ((_22159 > 0.001000000047497451305389404296875) && (_22158 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7344 = 2.0 + (2.0 * _22159);
                                float2 _7349 = _7327 / float2(fast::max(_22158, 9.9999997473787516355514526367188e-05));
                                _22161 = pow(pow(_7349.x, _7344) + pow(_7349.y, _7344), 1.0 / _7344) * _22158;
                            }
                            else
                            {
                                _22161 = length(_7327);
                            }
                            _22160 = _22161;
                        }
                        else
                        {
                            _22160 = fast::max(_7327.x, _7327.y);
                        }
                        float _7384 = (fast::min(fast::max(_7325.x, _7325.y), 0.0) + _22160) - _22158;
                        float _7389 = fast::max(gFxData_1._data[_6415].y, 9.9999997473787516355514526367188e-05);
                        float _7398 = fast::max(_7389 - abs(_7253 - _7384), 0.0) / _7389;
                        _22170 = fast::min(_7253, _7384) - (((_7398 * _7398) * _7389) * 0.25);
                    }
                    else
                    {
                        _22170 = _7253;
                    }
                    _22169 = _22170;
                }
                _22168 = _22169;
            }
            float2 _6861 = in.i_local - float2(0.5, 0.0);
            float2 _7447 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _7456 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _22187;
            if (in.i_flags.z == 1u)
            {
                float2 _7462 = _6861 - _7447;
                float _22186;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _22186 = abs(length(_7462) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _7561 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _7563 = cos(_7561);
                    float _7565 = sin(_7561);
                    float _7574 = dot(_7462, float2(-_7565, _7563));
                    float _7577 = dot(_7462, float2(_7563, _7565));
                    float2 _7578 = float2(_7574, _7577);
                    float _7581 = abs(_7574);
                    _7578.x = _7581;
                    float _7584 = in.i_shape.w * 0.5;
                    float _7586 = sin(_7584);
                    float _7588 = cos(_7584);
                    _22186 = (((_7588 * _7581) > (_7586 * _7577)) ? length(_7578 - (float2(_7586, _7588) * in.i_radii.x)) : abs(length(_7578) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _22187 = _22186;
            }
            else
            {
                float _22188;
                if (in.i_flags.z == 2u)
                {
                    float2 _7624 = _6861 - _22023.xy;
                    float2 _7627 = _22023.zw - _22023.xy;
                    _22188 = length(_7624 - (_7627 * fast::clamp(dot(_7624, _7627) / fast::max(dot(_7627, _7627), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _7489 = _6861 - _7447;
                    float _7680 = fast::min(_7456.x, _7456.y);
                    float _7683 = fast::min((_7489.x > 0.0) ? ((_7489.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_7489.y > 0.0) ? in.i_radii.w : in.i_radii.x), _7680);
                    float _7689 = _7683 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _22171;
                    float _22172;
                    if (_7689 > _7680)
                    {
                        float _7703 = in.i_shape.y * fast::clamp((_7680 - _7683) / fast::max(0.60000002384185791015625 * _7683, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _22172 = _7703;
                        _22171 = _7683 * (1.0 + (0.60000002384185791015625 * _7703));
                    }
                    else
                    {
                        _22172 = in.i_shape.y;
                        _22171 = _7689;
                    }
                    float2 _7716 = (abs(_7489) - _7456) + float2(_22171);
                    float2 _7718 = fast::max(_7716, float2(0.0));
                    float _22173;
                    if ((_7718.x > 0.0) && (_7718.y > 0.0))
                    {
                        float _22174;
                        if ((_22172 > 0.001000000047497451305389404296875) && (_22171 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7735 = 2.0 + (2.0 * _22172);
                            float2 _7740 = _7718 / float2(fast::max(_22171, 9.9999997473787516355514526367188e-05));
                            _22174 = pow(pow(_7740.x, _7735) + pow(_7740.y, _7735), 1.0 / _7735) * _22171;
                        }
                        else
                        {
                            _22174 = length(_7718);
                        }
                        _22173 = _22174;
                    }
                    else
                    {
                        _22173 = fast::max(_7718.x, _7718.y);
                    }
                    float _7775 = (fast::min(fast::max(_7716.x, _7716.y), 0.0) + _22173) - _22171;
                    float _22189;
                    if ((in.i_flags.x & 512u) != 0u)
                    {
                        float2 _7517 = fast::max((_22023.zw - _22023.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _7520 = _6861 - ((_22023.xy + _22023.zw) * 0.5);
                        float _7811 = fast::min(_7517.x, _7517.y);
                        float _7814 = fast::min((_7520.x > 0.0) ? ((_7520.y > 0.0) ? gFxData_1._data[_6415].x : gFxData_1._data[_6415].x) : ((_7520.y > 0.0) ? gFxData_1._data[_6415].x : gFxData_1._data[_6415].x), _7811);
                        float _7820 = _7814 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _22177;
                        float _22178;
                        if (_7820 > _7811)
                        {
                            float _7834 = in.i_shape.y * fast::clamp((_7811 - _7814) / fast::max(0.60000002384185791015625 * _7814, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _22178 = _7834;
                            _22177 = _7814 * (1.0 + (0.60000002384185791015625 * _7834));
                        }
                        else
                        {
                            _22178 = in.i_shape.y;
                            _22177 = _7820;
                        }
                        float2 _7847 = (abs(_7520) - _7517) + float2(_22177);
                        float2 _7849 = fast::max(_7847, float2(0.0));
                        float _22179;
                        if ((_7849.x > 0.0) && (_7849.y > 0.0))
                        {
                            float _22180;
                            if ((_22178 > 0.001000000047497451305389404296875) && (_22177 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7866 = 2.0 + (2.0 * _22178);
                                float2 _7871 = _7849 / float2(fast::max(_22177, 9.9999997473787516355514526367188e-05));
                                _22180 = pow(pow(_7871.x, _7866) + pow(_7871.y, _7866), 1.0 / _7866) * _22177;
                            }
                            else
                            {
                                _22180 = length(_7849);
                            }
                            _22179 = _22180;
                        }
                        else
                        {
                            _22179 = fast::max(_7849.x, _7849.y);
                        }
                        float _7906 = (fast::min(fast::max(_7847.x, _7847.y), 0.0) + _22179) - _22177;
                        float _7911 = fast::max(gFxData_1._data[_6415].y, 9.9999997473787516355514526367188e-05);
                        float _7920 = fast::max(_7911 - abs(_7775 - _7906), 0.0) / _7911;
                        _22189 = fast::min(_7775, _7906) - (((_7920 * _7920) * _7911) * 0.25);
                    }
                    else
                    {
                        _22189 = _7775;
                    }
                    _22188 = _22189;
                }
                _22187 = _22188;
            }
            float2 _6866 = in.i_local + float2(0.0, 0.5);
            float2 _7969 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _7978 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _22206;
            if (in.i_flags.z == 1u)
            {
                float2 _7984 = _6866 - _7969;
                float _22205;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _22205 = abs(length(_7984) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _8083 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _8085 = cos(_8083);
                    float _8087 = sin(_8083);
                    float _8096 = dot(_7984, float2(-_8087, _8085));
                    float _8099 = dot(_7984, float2(_8085, _8087));
                    float2 _8100 = float2(_8096, _8099);
                    float _8103 = abs(_8096);
                    _8100.x = _8103;
                    float _8106 = in.i_shape.w * 0.5;
                    float _8108 = sin(_8106);
                    float _8110 = cos(_8106);
                    _22205 = (((_8110 * _8103) > (_8108 * _8099)) ? length(_8100 - (float2(_8108, _8110) * in.i_radii.x)) : abs(length(_8100) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _22206 = _22205;
            }
            else
            {
                float _22207;
                if (in.i_flags.z == 2u)
                {
                    float2 _8146 = _6866 - _22023.xy;
                    float2 _8149 = _22023.zw - _22023.xy;
                    _22207 = length(_8146 - (_8149 * fast::clamp(dot(_8146, _8149) / fast::max(dot(_8149, _8149), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _8011 = _6866 - _7969;
                    float _8202 = fast::min(_7978.x, _7978.y);
                    float _8205 = fast::min((_8011.x > 0.0) ? ((_8011.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_8011.y > 0.0) ? in.i_radii.w : in.i_radii.x), _8202);
                    float _8211 = _8205 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _22190;
                    float _22191;
                    if (_8211 > _8202)
                    {
                        float _8225 = in.i_shape.y * fast::clamp((_8202 - _8205) / fast::max(0.60000002384185791015625 * _8205, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _22191 = _8225;
                        _22190 = _8205 * (1.0 + (0.60000002384185791015625 * _8225));
                    }
                    else
                    {
                        _22191 = in.i_shape.y;
                        _22190 = _8211;
                    }
                    float2 _8238 = (abs(_8011) - _7978) + float2(_22190);
                    float2 _8240 = fast::max(_8238, float2(0.0));
                    float _22192;
                    if ((_8240.x > 0.0) && (_8240.y > 0.0))
                    {
                        float _22193;
                        if ((_22191 > 0.001000000047497451305389404296875) && (_22190 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8257 = 2.0 + (2.0 * _22191);
                            float2 _8262 = _8240 / float2(fast::max(_22190, 9.9999997473787516355514526367188e-05));
                            _22193 = pow(pow(_8262.x, _8257) + pow(_8262.y, _8257), 1.0 / _8257) * _22190;
                        }
                        else
                        {
                            _22193 = length(_8240);
                        }
                        _22192 = _22193;
                    }
                    else
                    {
                        _22192 = fast::max(_8240.x, _8240.y);
                    }
                    float _8297 = (fast::min(fast::max(_8238.x, _8238.y), 0.0) + _22192) - _22190;
                    float _22208;
                    if ((in.i_flags.x & 512u) != 0u)
                    {
                        float2 _8039 = fast::max((_22023.zw - _22023.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _8042 = _6866 - ((_22023.xy + _22023.zw) * 0.5);
                        float _8333 = fast::min(_8039.x, _8039.y);
                        float _8336 = fast::min((_8042.x > 0.0) ? ((_8042.y > 0.0) ? gFxData_1._data[_6415].x : gFxData_1._data[_6415].x) : ((_8042.y > 0.0) ? gFxData_1._data[_6415].x : gFxData_1._data[_6415].x), _8333);
                        float _8342 = _8336 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _22196;
                        float _22197;
                        if (_8342 > _8333)
                        {
                            float _8356 = in.i_shape.y * fast::clamp((_8333 - _8336) / fast::max(0.60000002384185791015625 * _8336, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _22197 = _8356;
                            _22196 = _8336 * (1.0 + (0.60000002384185791015625 * _8356));
                        }
                        else
                        {
                            _22197 = in.i_shape.y;
                            _22196 = _8342;
                        }
                        float2 _8369 = (abs(_8042) - _8039) + float2(_22196);
                        float2 _8371 = fast::max(_8369, float2(0.0));
                        float _22198;
                        if ((_8371.x > 0.0) && (_8371.y > 0.0))
                        {
                            float _22199;
                            if ((_22197 > 0.001000000047497451305389404296875) && (_22196 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8388 = 2.0 + (2.0 * _22197);
                                float2 _8393 = _8371 / float2(fast::max(_22196, 9.9999997473787516355514526367188e-05));
                                _22199 = pow(pow(_8393.x, _8388) + pow(_8393.y, _8388), 1.0 / _8388) * _22196;
                            }
                            else
                            {
                                _22199 = length(_8371);
                            }
                            _22198 = _22199;
                        }
                        else
                        {
                            _22198 = fast::max(_8371.x, _8371.y);
                        }
                        float _8428 = (fast::min(fast::max(_8369.x, _8369.y), 0.0) + _22198) - _22196;
                        float _8433 = fast::max(gFxData_1._data[_6415].y, 9.9999997473787516355514526367188e-05);
                        float _8442 = fast::max(_8433 - abs(_8297 - _8428), 0.0) / _8433;
                        _22208 = fast::min(_8297, _8428) - (((_8442 * _8442) * _8433) * 0.25);
                    }
                    else
                    {
                        _22208 = _8297;
                    }
                    _22207 = _22208;
                }
                _22206 = _22207;
            }
            float2 _6870 = in.i_local - float2(0.0, 0.5);
            float2 _8491 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _8500 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _22225;
            if (in.i_flags.z == 1u)
            {
                float2 _8506 = _6870 - _8491;
                float _22224;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _22224 = abs(length(_8506) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _8605 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _8607 = cos(_8605);
                    float _8609 = sin(_8605);
                    float _8618 = dot(_8506, float2(-_8609, _8607));
                    float _8621 = dot(_8506, float2(_8607, _8609));
                    float2 _8622 = float2(_8618, _8621);
                    float _8625 = abs(_8618);
                    _8622.x = _8625;
                    float _8628 = in.i_shape.w * 0.5;
                    float _8630 = sin(_8628);
                    float _8632 = cos(_8628);
                    _22224 = (((_8632 * _8625) > (_8630 * _8621)) ? length(_8622 - (float2(_8630, _8632) * in.i_radii.x)) : abs(length(_8622) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _22225 = _22224;
            }
            else
            {
                float _22226;
                if (in.i_flags.z == 2u)
                {
                    float2 _8668 = _6870 - _22023.xy;
                    float2 _8671 = _22023.zw - _22023.xy;
                    _22226 = length(_8668 - (_8671 * fast::clamp(dot(_8668, _8671) / fast::max(dot(_8671, _8671), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _8533 = _6870 - _8491;
                    float _8724 = fast::min(_8500.x, _8500.y);
                    float _8727 = fast::min((_8533.x > 0.0) ? ((_8533.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_8533.y > 0.0) ? in.i_radii.w : in.i_radii.x), _8724);
                    float _8733 = _8727 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _22209;
                    float _22210;
                    if (_8733 > _8724)
                    {
                        float _8747 = in.i_shape.y * fast::clamp((_8724 - _8727) / fast::max(0.60000002384185791015625 * _8727, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _22210 = _8747;
                        _22209 = _8727 * (1.0 + (0.60000002384185791015625 * _8747));
                    }
                    else
                    {
                        _22210 = in.i_shape.y;
                        _22209 = _8733;
                    }
                    float2 _8760 = (abs(_8533) - _8500) + float2(_22209);
                    float2 _8762 = fast::max(_8760, float2(0.0));
                    float _22211;
                    if ((_8762.x > 0.0) && (_8762.y > 0.0))
                    {
                        float _22212;
                        if ((_22210 > 0.001000000047497451305389404296875) && (_22209 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8779 = 2.0 + (2.0 * _22210);
                            float2 _8784 = _8762 / float2(fast::max(_22209, 9.9999997473787516355514526367188e-05));
                            _22212 = pow(pow(_8784.x, _8779) + pow(_8784.y, _8779), 1.0 / _8779) * _22209;
                        }
                        else
                        {
                            _22212 = length(_8762);
                        }
                        _22211 = _22212;
                    }
                    else
                    {
                        _22211 = fast::max(_8762.x, _8762.y);
                    }
                    float _8819 = (fast::min(fast::max(_8760.x, _8760.y), 0.0) + _22211) - _22209;
                    float _22227;
                    if ((in.i_flags.x & 512u) != 0u)
                    {
                        float2 _8561 = fast::max((_22023.zw - _22023.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _8564 = _6870 - ((_22023.xy + _22023.zw) * 0.5);
                        float _8855 = fast::min(_8561.x, _8561.y);
                        float _8858 = fast::min((_8564.x > 0.0) ? ((_8564.y > 0.0) ? gFxData_1._data[_6415].x : gFxData_1._data[_6415].x) : ((_8564.y > 0.0) ? gFxData_1._data[_6415].x : gFxData_1._data[_6415].x), _8855);
                        float _8864 = _8858 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _22215;
                        float _22216;
                        if (_8864 > _8855)
                        {
                            float _8878 = in.i_shape.y * fast::clamp((_8855 - _8858) / fast::max(0.60000002384185791015625 * _8858, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _22216 = _8878;
                            _22215 = _8858 * (1.0 + (0.60000002384185791015625 * _8878));
                        }
                        else
                        {
                            _22216 = in.i_shape.y;
                            _22215 = _8864;
                        }
                        float2 _8891 = (abs(_8564) - _8561) + float2(_22215);
                        float2 _8893 = fast::max(_8891, float2(0.0));
                        float _22217;
                        if ((_8893.x > 0.0) && (_8893.y > 0.0))
                        {
                            float _22218;
                            if ((_22216 > 0.001000000047497451305389404296875) && (_22215 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8910 = 2.0 + (2.0 * _22216);
                                float2 _8915 = _8893 / float2(fast::max(_22215, 9.9999997473787516355514526367188e-05));
                                _22218 = pow(pow(_8915.x, _8910) + pow(_8915.y, _8910), 1.0 / _8910) * _22215;
                            }
                            else
                            {
                                _22218 = length(_8893);
                            }
                            _22217 = _22218;
                        }
                        else
                        {
                            _22217 = fast::max(_8893.x, _8893.y);
                        }
                        float _8950 = (fast::min(fast::max(_8891.x, _8891.y), 0.0) + _22217) - _22215;
                        float _8955 = fast::max(gFxData_1._data[_6415].y, 9.9999997473787516355514526367188e-05);
                        float _8964 = fast::max(_8955 - abs(_8819 - _8950), 0.0) / _8955;
                        _22227 = fast::min(_8819, _8950) - (((_8964 * _8964) * _8955) * 0.25);
                    }
                    else
                    {
                        _22227 = _8819;
                    }
                    _22226 = _22227;
                }
                _22225 = _22226;
            }
            float2 _6876 = float2(_22168 - _22187, _22206 - _22225);
            float _6878 = length(_6876);
            float2 _6886 = select(float2(0.0, -1.0), _6876 / float2(_6878), bool2(_6878 > 9.9999997473787516355514526367188e-06));
            _23090 = fast::normalize(float3(_6886 * fast::min(_6505 / fast::max(_6511, 0.001000000047497451305389404296875), 8.0), 1.0));
            _22228 = _6886;
        }
        else
        {
            _23090 = float3(0.0, 0.0, 1.0);
            _22228 = float2(0.0, -1.0);
        }
        float2 _6537 = ((-_22228) * gFxData_1._data[_6391].y) * (1.0 - _6511);
        float2 _22229;
        if (gFxData_1._data[_6415].z > 0.0)
        {
            _22229 = (((in.i_rect.xy + in.i_rect.zw) * 0.5) - in.i_local) * (gFxData_1._data[_6415].z / (1.0 + gFxData_1._data[_6415].z));
        }
        else
        {
            _22229 = float2(0.0);
        }
        float2 _6563 = _22020 * _172.gTarget.zw;
        float2 _6570 = _172.gDisplay.zw * _172.gTarget.zw;
        float2 _6575 = (_6537 + _22229) * _6570;
        float2 _6578 = _6537 * _6570;
        float3 _22834;
        float _22857;
        float3 _23326;
        if (_172.gTime.z > 0.5)
        {
            float3 _22837;
            if (((gFxData_1._data[_6391].w > 0.001000000047497451305389404296875) && (_6505 > 0.0)) && ((((gFxData_1._data[_6391].y * 0.300000011920928955078125) * gFxData_1._data[_6391].w) * _172.gDisplay.z) > (_6477 * 0.3499999940395355224609375)))
            {
                float _6600 = 0.300000011920928955078125 * gFxData_1._data[_6391].w;
                float2 _6607 = (_6563 + _6575) - (_6578 * _6600);
                float _8989 = fast::clamp(log2(fast::max(_6477, 1.0)) - 1.0, 0.0, 5.0);
                int _8992 = int(floor(_8989));
                float _8996 = _8989 - float(_8992);
                float4 _22304;
                if (_8992 <= 0)
                {
                    float2 _22303;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _20714 = _6607;
                        _20714.y = 1.0 - _6607.y;
                        _22303 = _20714;
                    }
                    else
                    {
                        _22303 = _6607;
                    }
                    _22304 = gBackdrop0.sample(gLinear, _22303, level(0.0));
                }
                else
                {
                    float4 _22305;
                    if (_8992 == 1)
                    {
                        float2 _9131 = (_6607 * _172.gLevel[1].xy) - float2(0.5);
                        float2 _9133 = floor(_9131);
                        float2 _9136 = _9131 - _9133;
                        float2 _9139 = _9136 * _9136;
                        float2 _9142 = _9139 * _9136;
                        float2 _9161 = (((_9142 * 3.0) - (_9139 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _9174 = _9142 * 0.16666667163372039794921875;
                        float2 _9177 = (((((-_9142) + (_9139 * 3.0)) - (_9136 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9161;
                        float2 _9180 = (((((_9142 * (-3.0)) + (_9139 * 3.0)) + (_9136 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9174;
                        float2 _9190 = ((_9133 - float2(0.5)) + (_9161 / _9177)) * _172.gLevel[1].zw;
                        float2 _9200 = ((_9133 + float2(1.5)) + (_9174 / _9180)) * _172.gLevel[1].zw;
                        float2 _22299;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20719 = _9190;
                            _20719.y = 1.0 - _9190.y;
                            _22299 = _20719;
                        }
                        else
                        {
                            _22299 = _9190;
                        }
                        float _9220 = _9190.y;
                        float2 _9221 = float2(_9200.x, _9220);
                        float2 _22300;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20725 = _9221;
                            _20725.y = 1.0 - _9220;
                            _22300 = _20725;
                        }
                        else
                        {
                            _22300 = _9221;
                        }
                        float2 _9238 = float2(_9190.x, _9200.y);
                        float2 _22301;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20732 = _9238;
                            _20732.y = 1.0 - _9200.y;
                            _22301 = _20732;
                        }
                        else
                        {
                            _22301 = _9238;
                        }
                        float2 _22302;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20738 = _9200;
                            _20738.y = 1.0 - _9200.y;
                            _22302 = _20738;
                        }
                        else
                        {
                            _22302 = _9200;
                        }
                        _22305 = (((gBackdrop1.sample(gLinear, _22299, level(0.0)) * _9177.x) + (gBackdrop1.sample(gLinear, _22300, level(0.0)) * _9180.x)) * _9177.y) + (((gBackdrop1.sample(gLinear, _22301, level(0.0)) * _9177.x) + (gBackdrop1.sample(gLinear, _22302, level(0.0)) * _9180.x)) * _9180.y);
                    }
                    else
                    {
                        float4 _22306;
                        if (_8992 == 2)
                        {
                            float2 _9338 = (_6607 * _172.gLevel[2].xy) - float2(0.5);
                            float2 _9340 = floor(_9338);
                            float2 _9343 = _9338 - _9340;
                            float2 _9346 = _9343 * _9343;
                            float2 _9349 = _9346 * _9343;
                            float2 _9368 = (((_9349 * 3.0) - (_9346 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _9381 = _9349 * 0.16666667163372039794921875;
                            float2 _9384 = (((((-_9349) + (_9346 * 3.0)) - (_9343 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9368;
                            float2 _9387 = (((((_9349 * (-3.0)) + (_9346 * 3.0)) + (_9343 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9381;
                            float2 _9397 = ((_9340 - float2(0.5)) + (_9368 / _9384)) * _172.gLevel[2].zw;
                            float2 _9407 = ((_9340 + float2(1.5)) + (_9381 / _9387)) * _172.gLevel[2].zw;
                            float2 _22295;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20745 = _9397;
                                _20745.y = 1.0 - _9397.y;
                                _22295 = _20745;
                            }
                            else
                            {
                                _22295 = _9397;
                            }
                            float _9427 = _9397.y;
                            float2 _9428 = float2(_9407.x, _9427);
                            float2 _22296;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20751 = _9428;
                                _20751.y = 1.0 - _9427;
                                _22296 = _20751;
                            }
                            else
                            {
                                _22296 = _9428;
                            }
                            float2 _9445 = float2(_9397.x, _9407.y);
                            float2 _22297;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20758 = _9445;
                                _20758.y = 1.0 - _9407.y;
                                _22297 = _20758;
                            }
                            else
                            {
                                _22297 = _9445;
                            }
                            float2 _22298;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20764 = _9407;
                                _20764.y = 1.0 - _9407.y;
                                _22298 = _20764;
                            }
                            else
                            {
                                _22298 = _9407;
                            }
                            _22306 = (((gBackdrop2.sample(gLinear, _22295, level(0.0)) * _9384.x) + (gBackdrop2.sample(gLinear, _22296, level(0.0)) * _9387.x)) * _9384.y) + (((gBackdrop2.sample(gLinear, _22297, level(0.0)) * _9384.x) + (gBackdrop2.sample(gLinear, _22298, level(0.0)) * _9387.x)) * _9387.y);
                        }
                        else
                        {
                            float4 _22307;
                            if (_8992 == 3)
                            {
                                float2 _9545 = (_6607 * _172.gLevel[3].xy) - float2(0.5);
                                float2 _9547 = floor(_9545);
                                float2 _9550 = _9545 - _9547;
                                float2 _9553 = _9550 * _9550;
                                float2 _9556 = _9553 * _9550;
                                float2 _9575 = (((_9556 * 3.0) - (_9553 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _9588 = _9556 * 0.16666667163372039794921875;
                                float2 _9591 = (((((-_9556) + (_9553 * 3.0)) - (_9550 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9575;
                                float2 _9594 = (((((_9556 * (-3.0)) + (_9553 * 3.0)) + (_9550 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9588;
                                float2 _9604 = ((_9547 - float2(0.5)) + (_9575 / _9591)) * _172.gLevel[3].zw;
                                float2 _9614 = ((_9547 + float2(1.5)) + (_9588 / _9594)) * _172.gLevel[3].zw;
                                float2 _22291;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20771 = _9604;
                                    _20771.y = 1.0 - _9604.y;
                                    _22291 = _20771;
                                }
                                else
                                {
                                    _22291 = _9604;
                                }
                                float _9634 = _9604.y;
                                float2 _9635 = float2(_9614.x, _9634);
                                float2 _22292;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20777 = _9635;
                                    _20777.y = 1.0 - _9634;
                                    _22292 = _20777;
                                }
                                else
                                {
                                    _22292 = _9635;
                                }
                                float2 _9652 = float2(_9604.x, _9614.y);
                                float2 _22293;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20784 = _9652;
                                    _20784.y = 1.0 - _9614.y;
                                    _22293 = _20784;
                                }
                                else
                                {
                                    _22293 = _9652;
                                }
                                float2 _22294;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20790 = _9614;
                                    _20790.y = 1.0 - _9614.y;
                                    _22294 = _20790;
                                }
                                else
                                {
                                    _22294 = _9614;
                                }
                                _22307 = (((gBackdrop3.sample(gLinear, _22291, level(0.0)) * _9591.x) + (gBackdrop3.sample(gLinear, _22292, level(0.0)) * _9594.x)) * _9591.y) + (((gBackdrop3.sample(gLinear, _22293, level(0.0)) * _9591.x) + (gBackdrop3.sample(gLinear, _22294, level(0.0)) * _9594.x)) * _9594.y);
                            }
                            else
                            {
                                float4 _22308;
                                if (_8992 == 4)
                                {
                                    float2 _9752 = (_6607 * _172.gLevel[4].xy) - float2(0.5);
                                    float2 _9754 = floor(_9752);
                                    float2 _9757 = _9752 - _9754;
                                    float2 _9760 = _9757 * _9757;
                                    float2 _9763 = _9760 * _9757;
                                    float2 _9782 = (((_9763 * 3.0) - (_9760 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _9795 = _9763 * 0.16666667163372039794921875;
                                    float2 _9798 = (((((-_9763) + (_9760 * 3.0)) - (_9757 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9782;
                                    float2 _9801 = (((((_9763 * (-3.0)) + (_9760 * 3.0)) + (_9757 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9795;
                                    float2 _9811 = ((_9754 - float2(0.5)) + (_9782 / _9798)) * _172.gLevel[4].zw;
                                    float2 _9821 = ((_9754 + float2(1.5)) + (_9795 / _9801)) * _172.gLevel[4].zw;
                                    float2 _22287;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20797 = _9811;
                                        _20797.y = 1.0 - _9811.y;
                                        _22287 = _20797;
                                    }
                                    else
                                    {
                                        _22287 = _9811;
                                    }
                                    float _9841 = _9811.y;
                                    float2 _9842 = float2(_9821.x, _9841);
                                    float2 _22288;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20803 = _9842;
                                        _20803.y = 1.0 - _9841;
                                        _22288 = _20803;
                                    }
                                    else
                                    {
                                        _22288 = _9842;
                                    }
                                    float2 _9859 = float2(_9811.x, _9821.y);
                                    float2 _22289;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20810 = _9859;
                                        _20810.y = 1.0 - _9821.y;
                                        _22289 = _20810;
                                    }
                                    else
                                    {
                                        _22289 = _9859;
                                    }
                                    float2 _22290;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20816 = _9821;
                                        _20816.y = 1.0 - _9821.y;
                                        _22290 = _20816;
                                    }
                                    else
                                    {
                                        _22290 = _9821;
                                    }
                                    _22308 = (((gBackdrop4.sample(gLinear, _22287, level(0.0)) * _9798.x) + (gBackdrop4.sample(gLinear, _22288, level(0.0)) * _9801.x)) * _9798.y) + (((gBackdrop4.sample(gLinear, _22289, level(0.0)) * _9798.x) + (gBackdrop4.sample(gLinear, _22290, level(0.0)) * _9801.x)) * _9801.y);
                                }
                                else
                                {
                                    float2 _9959 = (_6607 * _172.gLevel[5].xy) - float2(0.5);
                                    float2 _9961 = floor(_9959);
                                    float2 _9964 = _9959 - _9961;
                                    float2 _9967 = _9964 * _9964;
                                    float2 _9970 = _9967 * _9964;
                                    float2 _9989 = (((_9970 * 3.0) - (_9967 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _10002 = _9970 * 0.16666667163372039794921875;
                                    float2 _10005 = (((((-_9970) + (_9967 * 3.0)) - (_9964 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9989;
                                    float2 _10008 = (((((_9970 * (-3.0)) + (_9967 * 3.0)) + (_9964 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10002;
                                    float2 _10018 = ((_9961 - float2(0.5)) + (_9989 / _10005)) * _172.gLevel[5].zw;
                                    float2 _10028 = ((_9961 + float2(1.5)) + (_10002 / _10008)) * _172.gLevel[5].zw;
                                    float2 _22283;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20823 = _10018;
                                        _20823.y = 1.0 - _10018.y;
                                        _22283 = _20823;
                                    }
                                    else
                                    {
                                        _22283 = _10018;
                                    }
                                    float _10048 = _10018.y;
                                    float2 _10049 = float2(_10028.x, _10048);
                                    float2 _22284;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20829 = _10049;
                                        _20829.y = 1.0 - _10048;
                                        _22284 = _20829;
                                    }
                                    else
                                    {
                                        _22284 = _10049;
                                    }
                                    float2 _10066 = float2(_10018.x, _10028.y);
                                    float2 _22285;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20836 = _10066;
                                        _20836.y = 1.0 - _10028.y;
                                        _22285 = _20836;
                                    }
                                    else
                                    {
                                        _22285 = _10066;
                                    }
                                    float2 _22286;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20842 = _10028;
                                        _20842.y = 1.0 - _10028.y;
                                        _22286 = _20842;
                                    }
                                    else
                                    {
                                        _22286 = _10028;
                                    }
                                    _22308 = (((gBackdrop5.sample(gLinear, _22283, level(0.0)) * _10005.x) + (gBackdrop5.sample(gLinear, _22284, level(0.0)) * _10008.x)) * _10005.y) + (((gBackdrop5.sample(gLinear, _22285, level(0.0)) * _10005.x) + (gBackdrop5.sample(gLinear, _22286, level(0.0)) * _10008.x)) * _10008.y);
                                }
                                _22307 = _22308;
                            }
                            _22306 = _22307;
                        }
                        _22305 = _22306;
                    }
                    _22304 = _22305;
                }
                float3 _22335;
                if ((_8996 > 0.0199999995529651641845703125) && (_8992 < 5))
                {
                    int _9009 = _8992 + 1;
                    float4 _22330;
                    if (_9009 <= 0)
                    {
                        float2 _22329;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20847 = _6607;
                            _20847.y = 1.0 - _6607.y;
                            _22329 = _20847;
                        }
                        else
                        {
                            _22329 = _6607;
                        }
                        _22330 = gBackdrop0.sample(gLinear, _22329, level(0.0));
                    }
                    else
                    {
                        float4 _22331;
                        if (_9009 == 1)
                        {
                            float2 _10255 = (_6607 * _172.gLevel[1].xy) - float2(0.5);
                            float2 _10257 = floor(_10255);
                            float2 _10260 = _10255 - _10257;
                            float2 _10263 = _10260 * _10260;
                            float2 _10266 = _10263 * _10260;
                            float2 _10285 = (((_10266 * 3.0) - (_10263 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _10298 = _10266 * 0.16666667163372039794921875;
                            float2 _10301 = (((((-_10266) + (_10263 * 3.0)) - (_10260 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10285;
                            float2 _10304 = (((((_10266 * (-3.0)) + (_10263 * 3.0)) + (_10260 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10298;
                            float2 _10314 = ((_10257 - float2(0.5)) + (_10285 / _10301)) * _172.gLevel[1].zw;
                            float2 _10324 = ((_10257 + float2(1.5)) + (_10298 / _10304)) * _172.gLevel[1].zw;
                            float2 _22325;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20852 = _10314;
                                _20852.y = 1.0 - _10314.y;
                                _22325 = _20852;
                            }
                            else
                            {
                                _22325 = _10314;
                            }
                            float _10344 = _10314.y;
                            float2 _10345 = float2(_10324.x, _10344);
                            float2 _22326;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20858 = _10345;
                                _20858.y = 1.0 - _10344;
                                _22326 = _20858;
                            }
                            else
                            {
                                _22326 = _10345;
                            }
                            float2 _10362 = float2(_10314.x, _10324.y);
                            float2 _22327;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20865 = _10362;
                                _20865.y = 1.0 - _10324.y;
                                _22327 = _20865;
                            }
                            else
                            {
                                _22327 = _10362;
                            }
                            float2 _22328;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20871 = _10324;
                                _20871.y = 1.0 - _10324.y;
                                _22328 = _20871;
                            }
                            else
                            {
                                _22328 = _10324;
                            }
                            _22331 = (((gBackdrop1.sample(gLinear, _22325, level(0.0)) * _10301.x) + (gBackdrop1.sample(gLinear, _22326, level(0.0)) * _10304.x)) * _10301.y) + (((gBackdrop1.sample(gLinear, _22327, level(0.0)) * _10301.x) + (gBackdrop1.sample(gLinear, _22328, level(0.0)) * _10304.x)) * _10304.y);
                        }
                        else
                        {
                            float4 _22332;
                            if (_9009 == 2)
                            {
                                float2 _10462 = (_6607 * _172.gLevel[2].xy) - float2(0.5);
                                float2 _10464 = floor(_10462);
                                float2 _10467 = _10462 - _10464;
                                float2 _10470 = _10467 * _10467;
                                float2 _10473 = _10470 * _10467;
                                float2 _10492 = (((_10473 * 3.0) - (_10470 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _10505 = _10473 * 0.16666667163372039794921875;
                                float2 _10508 = (((((-_10473) + (_10470 * 3.0)) - (_10467 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10492;
                                float2 _10511 = (((((_10473 * (-3.0)) + (_10470 * 3.0)) + (_10467 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10505;
                                float2 _10521 = ((_10464 - float2(0.5)) + (_10492 / _10508)) * _172.gLevel[2].zw;
                                float2 _10531 = ((_10464 + float2(1.5)) + (_10505 / _10511)) * _172.gLevel[2].zw;
                                float2 _22321;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20878 = _10521;
                                    _20878.y = 1.0 - _10521.y;
                                    _22321 = _20878;
                                }
                                else
                                {
                                    _22321 = _10521;
                                }
                                float _10551 = _10521.y;
                                float2 _10552 = float2(_10531.x, _10551);
                                float2 _22322;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20884 = _10552;
                                    _20884.y = 1.0 - _10551;
                                    _22322 = _20884;
                                }
                                else
                                {
                                    _22322 = _10552;
                                }
                                float2 _10569 = float2(_10521.x, _10531.y);
                                float2 _22323;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20891 = _10569;
                                    _20891.y = 1.0 - _10531.y;
                                    _22323 = _20891;
                                }
                                else
                                {
                                    _22323 = _10569;
                                }
                                float2 _22324;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20897 = _10531;
                                    _20897.y = 1.0 - _10531.y;
                                    _22324 = _20897;
                                }
                                else
                                {
                                    _22324 = _10531;
                                }
                                _22332 = (((gBackdrop2.sample(gLinear, _22321, level(0.0)) * _10508.x) + (gBackdrop2.sample(gLinear, _22322, level(0.0)) * _10511.x)) * _10508.y) + (((gBackdrop2.sample(gLinear, _22323, level(0.0)) * _10508.x) + (gBackdrop2.sample(gLinear, _22324, level(0.0)) * _10511.x)) * _10511.y);
                            }
                            else
                            {
                                float4 _22333;
                                if (_9009 == 3)
                                {
                                    float2 _10669 = (_6607 * _172.gLevel[3].xy) - float2(0.5);
                                    float2 _10671 = floor(_10669);
                                    float2 _10674 = _10669 - _10671;
                                    float2 _10677 = _10674 * _10674;
                                    float2 _10680 = _10677 * _10674;
                                    float2 _10699 = (((_10680 * 3.0) - (_10677 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _10712 = _10680 * 0.16666667163372039794921875;
                                    float2 _10715 = (((((-_10680) + (_10677 * 3.0)) - (_10674 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10699;
                                    float2 _10718 = (((((_10680 * (-3.0)) + (_10677 * 3.0)) + (_10674 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10712;
                                    float2 _10728 = ((_10671 - float2(0.5)) + (_10699 / _10715)) * _172.gLevel[3].zw;
                                    float2 _10738 = ((_10671 + float2(1.5)) + (_10712 / _10718)) * _172.gLevel[3].zw;
                                    float2 _22317;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20904 = _10728;
                                        _20904.y = 1.0 - _10728.y;
                                        _22317 = _20904;
                                    }
                                    else
                                    {
                                        _22317 = _10728;
                                    }
                                    float _10758 = _10728.y;
                                    float2 _10759 = float2(_10738.x, _10758);
                                    float2 _22318;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20910 = _10759;
                                        _20910.y = 1.0 - _10758;
                                        _22318 = _20910;
                                    }
                                    else
                                    {
                                        _22318 = _10759;
                                    }
                                    float2 _10776 = float2(_10728.x, _10738.y);
                                    float2 _22319;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20917 = _10776;
                                        _20917.y = 1.0 - _10738.y;
                                        _22319 = _20917;
                                    }
                                    else
                                    {
                                        _22319 = _10776;
                                    }
                                    float2 _22320;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20923 = _10738;
                                        _20923.y = 1.0 - _10738.y;
                                        _22320 = _20923;
                                    }
                                    else
                                    {
                                        _22320 = _10738;
                                    }
                                    _22333 = (((gBackdrop3.sample(gLinear, _22317, level(0.0)) * _10715.x) + (gBackdrop3.sample(gLinear, _22318, level(0.0)) * _10718.x)) * _10715.y) + (((gBackdrop3.sample(gLinear, _22319, level(0.0)) * _10715.x) + (gBackdrop3.sample(gLinear, _22320, level(0.0)) * _10718.x)) * _10718.y);
                                }
                                else
                                {
                                    float4 _22334;
                                    if (_9009 == 4)
                                    {
                                        float2 _10876 = (_6607 * _172.gLevel[4].xy) - float2(0.5);
                                        float2 _10878 = floor(_10876);
                                        float2 _10881 = _10876 - _10878;
                                        float2 _10884 = _10881 * _10881;
                                        float2 _10887 = _10884 * _10881;
                                        float2 _10906 = (((_10887 * 3.0) - (_10884 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _10919 = _10887 * 0.16666667163372039794921875;
                                        float2 _10922 = (((((-_10887) + (_10884 * 3.0)) - (_10881 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10906;
                                        float2 _10925 = (((((_10887 * (-3.0)) + (_10884 * 3.0)) + (_10881 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10919;
                                        float2 _10935 = ((_10878 - float2(0.5)) + (_10906 / _10922)) * _172.gLevel[4].zw;
                                        float2 _10945 = ((_10878 + float2(1.5)) + (_10919 / _10925)) * _172.gLevel[4].zw;
                                        float2 _22313;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20930 = _10935;
                                            _20930.y = 1.0 - _10935.y;
                                            _22313 = _20930;
                                        }
                                        else
                                        {
                                            _22313 = _10935;
                                        }
                                        float _10965 = _10935.y;
                                        float2 _10966 = float2(_10945.x, _10965);
                                        float2 _22314;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20936 = _10966;
                                            _20936.y = 1.0 - _10965;
                                            _22314 = _20936;
                                        }
                                        else
                                        {
                                            _22314 = _10966;
                                        }
                                        float2 _10983 = float2(_10935.x, _10945.y);
                                        float2 _22315;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20943 = _10983;
                                            _20943.y = 1.0 - _10945.y;
                                            _22315 = _20943;
                                        }
                                        else
                                        {
                                            _22315 = _10983;
                                        }
                                        float2 _22316;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20949 = _10945;
                                            _20949.y = 1.0 - _10945.y;
                                            _22316 = _20949;
                                        }
                                        else
                                        {
                                            _22316 = _10945;
                                        }
                                        _22334 = (((gBackdrop4.sample(gLinear, _22313, level(0.0)) * _10922.x) + (gBackdrop4.sample(gLinear, _22314, level(0.0)) * _10925.x)) * _10922.y) + (((gBackdrop4.sample(gLinear, _22315, level(0.0)) * _10922.x) + (gBackdrop4.sample(gLinear, _22316, level(0.0)) * _10925.x)) * _10925.y);
                                    }
                                    else
                                    {
                                        float2 _11083 = (_6607 * _172.gLevel[5].xy) - float2(0.5);
                                        float2 _11085 = floor(_11083);
                                        float2 _11088 = _11083 - _11085;
                                        float2 _11091 = _11088 * _11088;
                                        float2 _11094 = _11091 * _11088;
                                        float2 _11113 = (((_11094 * 3.0) - (_11091 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _11126 = _11094 * 0.16666667163372039794921875;
                                        float2 _11129 = (((((-_11094) + (_11091 * 3.0)) - (_11088 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11113;
                                        float2 _11132 = (((((_11094 * (-3.0)) + (_11091 * 3.0)) + (_11088 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11126;
                                        float2 _11142 = ((_11085 - float2(0.5)) + (_11113 / _11129)) * _172.gLevel[5].zw;
                                        float2 _11152 = ((_11085 + float2(1.5)) + (_11126 / _11132)) * _172.gLevel[5].zw;
                                        float2 _22309;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20956 = _11142;
                                            _20956.y = 1.0 - _11142.y;
                                            _22309 = _20956;
                                        }
                                        else
                                        {
                                            _22309 = _11142;
                                        }
                                        float _11172 = _11142.y;
                                        float2 _11173 = float2(_11152.x, _11172);
                                        float2 _22310;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20962 = _11173;
                                            _20962.y = 1.0 - _11172;
                                            _22310 = _20962;
                                        }
                                        else
                                        {
                                            _22310 = _11173;
                                        }
                                        float2 _11190 = float2(_11142.x, _11152.y);
                                        float2 _22311;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20969 = _11190;
                                            _20969.y = 1.0 - _11152.y;
                                            _22311 = _20969;
                                        }
                                        else
                                        {
                                            _22311 = _11190;
                                        }
                                        float2 _22312;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20975 = _11152;
                                            _20975.y = 1.0 - _11152.y;
                                            _22312 = _20975;
                                        }
                                        else
                                        {
                                            _22312 = _11152;
                                        }
                                        _22334 = (((gBackdrop5.sample(gLinear, _22309, level(0.0)) * _11129.x) + (gBackdrop5.sample(gLinear, _22310, level(0.0)) * _11132.x)) * _11129.y) + (((gBackdrop5.sample(gLinear, _22311, level(0.0)) * _11129.x) + (gBackdrop5.sample(gLinear, _22312, level(0.0)) * _11132.x)) * _11132.y);
                                    }
                                    _22333 = _22334;
                                }
                                _22332 = _22333;
                            }
                            _22331 = _22332;
                        }
                        _22330 = _22331;
                    }
                    _22335 = mix(_22304.xyz, _22330.xyz, float3(_8996));
                }
                else
                {
                    _22335 = _22304.xyz;
                }
                float2 _6614 = _6563 + _6575;
                float _11280 = fast::clamp(log2(fast::max(_6477, 1.0)) - 1.0, 0.0, 5.0);
                int _11283 = int(floor(_11280));
                float _11287 = _11280 - float(_11283);
                float4 _22410;
                if (_11283 <= 0)
                {
                    float2 _22409;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _20982 = _6614;
                        _20982.y = 1.0 - _6614.y;
                        _22409 = _20982;
                    }
                    else
                    {
                        _22409 = _6614;
                    }
                    _22410 = gBackdrop0.sample(gLinear, _22409, level(0.0));
                }
                else
                {
                    float4 _22411;
                    if (_11283 == 1)
                    {
                        float2 _11422 = (_6614 * _172.gLevel[1].xy) - float2(0.5);
                        float2 _11424 = floor(_11422);
                        float2 _11427 = _11422 - _11424;
                        float2 _11430 = _11427 * _11427;
                        float2 _11433 = _11430 * _11427;
                        float2 _11452 = (((_11433 * 3.0) - (_11430 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _11465 = _11433 * 0.16666667163372039794921875;
                        float2 _11468 = (((((-_11433) + (_11430 * 3.0)) - (_11427 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11452;
                        float2 _11471 = (((((_11433 * (-3.0)) + (_11430 * 3.0)) + (_11427 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11465;
                        float2 _11481 = ((_11424 - float2(0.5)) + (_11452 / _11468)) * _172.gLevel[1].zw;
                        float2 _11491 = ((_11424 + float2(1.5)) + (_11465 / _11471)) * _172.gLevel[1].zw;
                        float2 _22405;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20987 = _11481;
                            _20987.y = 1.0 - _11481.y;
                            _22405 = _20987;
                        }
                        else
                        {
                            _22405 = _11481;
                        }
                        float _11511 = _11481.y;
                        float2 _11512 = float2(_11491.x, _11511);
                        float2 _22406;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20993 = _11512;
                            _20993.y = 1.0 - _11511;
                            _22406 = _20993;
                        }
                        else
                        {
                            _22406 = _11512;
                        }
                        float2 _11529 = float2(_11481.x, _11491.y);
                        float2 _22407;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21000 = _11529;
                            _21000.y = 1.0 - _11491.y;
                            _22407 = _21000;
                        }
                        else
                        {
                            _22407 = _11529;
                        }
                        float2 _22408;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21006 = _11491;
                            _21006.y = 1.0 - _11491.y;
                            _22408 = _21006;
                        }
                        else
                        {
                            _22408 = _11491;
                        }
                        _22411 = (((gBackdrop1.sample(gLinear, _22405, level(0.0)) * _11468.x) + (gBackdrop1.sample(gLinear, _22406, level(0.0)) * _11471.x)) * _11468.y) + (((gBackdrop1.sample(gLinear, _22407, level(0.0)) * _11468.x) + (gBackdrop1.sample(gLinear, _22408, level(0.0)) * _11471.x)) * _11471.y);
                    }
                    else
                    {
                        float4 _22412;
                        if (_11283 == 2)
                        {
                            float2 _11629 = (_6614 * _172.gLevel[2].xy) - float2(0.5);
                            float2 _11631 = floor(_11629);
                            float2 _11634 = _11629 - _11631;
                            float2 _11637 = _11634 * _11634;
                            float2 _11640 = _11637 * _11634;
                            float2 _11659 = (((_11640 * 3.0) - (_11637 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _11672 = _11640 * 0.16666667163372039794921875;
                            float2 _11675 = (((((-_11640) + (_11637 * 3.0)) - (_11634 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11659;
                            float2 _11678 = (((((_11640 * (-3.0)) + (_11637 * 3.0)) + (_11634 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11672;
                            float2 _11688 = ((_11631 - float2(0.5)) + (_11659 / _11675)) * _172.gLevel[2].zw;
                            float2 _11698 = ((_11631 + float2(1.5)) + (_11672 / _11678)) * _172.gLevel[2].zw;
                            float2 _22401;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21013 = _11688;
                                _21013.y = 1.0 - _11688.y;
                                _22401 = _21013;
                            }
                            else
                            {
                                _22401 = _11688;
                            }
                            float _11718 = _11688.y;
                            float2 _11719 = float2(_11698.x, _11718);
                            float2 _22402;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21019 = _11719;
                                _21019.y = 1.0 - _11718;
                                _22402 = _21019;
                            }
                            else
                            {
                                _22402 = _11719;
                            }
                            float2 _11736 = float2(_11688.x, _11698.y);
                            float2 _22403;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21026 = _11736;
                                _21026.y = 1.0 - _11698.y;
                                _22403 = _21026;
                            }
                            else
                            {
                                _22403 = _11736;
                            }
                            float2 _22404;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21032 = _11698;
                                _21032.y = 1.0 - _11698.y;
                                _22404 = _21032;
                            }
                            else
                            {
                                _22404 = _11698;
                            }
                            _22412 = (((gBackdrop2.sample(gLinear, _22401, level(0.0)) * _11675.x) + (gBackdrop2.sample(gLinear, _22402, level(0.0)) * _11678.x)) * _11675.y) + (((gBackdrop2.sample(gLinear, _22403, level(0.0)) * _11675.x) + (gBackdrop2.sample(gLinear, _22404, level(0.0)) * _11678.x)) * _11678.y);
                        }
                        else
                        {
                            float4 _22413;
                            if (_11283 == 3)
                            {
                                float2 _11836 = (_6614 * _172.gLevel[3].xy) - float2(0.5);
                                float2 _11838 = floor(_11836);
                                float2 _11841 = _11836 - _11838;
                                float2 _11844 = _11841 * _11841;
                                float2 _11847 = _11844 * _11841;
                                float2 _11866 = (((_11847 * 3.0) - (_11844 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _11879 = _11847 * 0.16666667163372039794921875;
                                float2 _11882 = (((((-_11847) + (_11844 * 3.0)) - (_11841 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11866;
                                float2 _11885 = (((((_11847 * (-3.0)) + (_11844 * 3.0)) + (_11841 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11879;
                                float2 _11895 = ((_11838 - float2(0.5)) + (_11866 / _11882)) * _172.gLevel[3].zw;
                                float2 _11905 = ((_11838 + float2(1.5)) + (_11879 / _11885)) * _172.gLevel[3].zw;
                                float2 _22397;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21039 = _11895;
                                    _21039.y = 1.0 - _11895.y;
                                    _22397 = _21039;
                                }
                                else
                                {
                                    _22397 = _11895;
                                }
                                float _11925 = _11895.y;
                                float2 _11926 = float2(_11905.x, _11925);
                                float2 _22398;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21045 = _11926;
                                    _21045.y = 1.0 - _11925;
                                    _22398 = _21045;
                                }
                                else
                                {
                                    _22398 = _11926;
                                }
                                float2 _11943 = float2(_11895.x, _11905.y);
                                float2 _22399;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21052 = _11943;
                                    _21052.y = 1.0 - _11905.y;
                                    _22399 = _21052;
                                }
                                else
                                {
                                    _22399 = _11943;
                                }
                                float2 _22400;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21058 = _11905;
                                    _21058.y = 1.0 - _11905.y;
                                    _22400 = _21058;
                                }
                                else
                                {
                                    _22400 = _11905;
                                }
                                _22413 = (((gBackdrop3.sample(gLinear, _22397, level(0.0)) * _11882.x) + (gBackdrop3.sample(gLinear, _22398, level(0.0)) * _11885.x)) * _11882.y) + (((gBackdrop3.sample(gLinear, _22399, level(0.0)) * _11882.x) + (gBackdrop3.sample(gLinear, _22400, level(0.0)) * _11885.x)) * _11885.y);
                            }
                            else
                            {
                                float4 _22414;
                                if (_11283 == 4)
                                {
                                    float2 _12043 = (_6614 * _172.gLevel[4].xy) - float2(0.5);
                                    float2 _12045 = floor(_12043);
                                    float2 _12048 = _12043 - _12045;
                                    float2 _12051 = _12048 * _12048;
                                    float2 _12054 = _12051 * _12048;
                                    float2 _12073 = (((_12054 * 3.0) - (_12051 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _12086 = _12054 * 0.16666667163372039794921875;
                                    float2 _12089 = (((((-_12054) + (_12051 * 3.0)) - (_12048 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12073;
                                    float2 _12092 = (((((_12054 * (-3.0)) + (_12051 * 3.0)) + (_12048 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12086;
                                    float2 _12102 = ((_12045 - float2(0.5)) + (_12073 / _12089)) * _172.gLevel[4].zw;
                                    float2 _12112 = ((_12045 + float2(1.5)) + (_12086 / _12092)) * _172.gLevel[4].zw;
                                    float2 _22393;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21065 = _12102;
                                        _21065.y = 1.0 - _12102.y;
                                        _22393 = _21065;
                                    }
                                    else
                                    {
                                        _22393 = _12102;
                                    }
                                    float _12132 = _12102.y;
                                    float2 _12133 = float2(_12112.x, _12132);
                                    float2 _22394;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21071 = _12133;
                                        _21071.y = 1.0 - _12132;
                                        _22394 = _21071;
                                    }
                                    else
                                    {
                                        _22394 = _12133;
                                    }
                                    float2 _12150 = float2(_12102.x, _12112.y);
                                    float2 _22395;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21078 = _12150;
                                        _21078.y = 1.0 - _12112.y;
                                        _22395 = _21078;
                                    }
                                    else
                                    {
                                        _22395 = _12150;
                                    }
                                    float2 _22396;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21084 = _12112;
                                        _21084.y = 1.0 - _12112.y;
                                        _22396 = _21084;
                                    }
                                    else
                                    {
                                        _22396 = _12112;
                                    }
                                    _22414 = (((gBackdrop4.sample(gLinear, _22393, level(0.0)) * _12089.x) + (gBackdrop4.sample(gLinear, _22394, level(0.0)) * _12092.x)) * _12089.y) + (((gBackdrop4.sample(gLinear, _22395, level(0.0)) * _12089.x) + (gBackdrop4.sample(gLinear, _22396, level(0.0)) * _12092.x)) * _12092.y);
                                }
                                else
                                {
                                    float2 _12250 = (_6614 * _172.gLevel[5].xy) - float2(0.5);
                                    float2 _12252 = floor(_12250);
                                    float2 _12255 = _12250 - _12252;
                                    float2 _12258 = _12255 * _12255;
                                    float2 _12261 = _12258 * _12255;
                                    float2 _12280 = (((_12261 * 3.0) - (_12258 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _12293 = _12261 * 0.16666667163372039794921875;
                                    float2 _12296 = (((((-_12261) + (_12258 * 3.0)) - (_12255 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12280;
                                    float2 _12299 = (((((_12261 * (-3.0)) + (_12258 * 3.0)) + (_12255 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12293;
                                    float2 _12309 = ((_12252 - float2(0.5)) + (_12280 / _12296)) * _172.gLevel[5].zw;
                                    float2 _12319 = ((_12252 + float2(1.5)) + (_12293 / _12299)) * _172.gLevel[5].zw;
                                    float2 _22389;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21091 = _12309;
                                        _21091.y = 1.0 - _12309.y;
                                        _22389 = _21091;
                                    }
                                    else
                                    {
                                        _22389 = _12309;
                                    }
                                    float _12339 = _12309.y;
                                    float2 _12340 = float2(_12319.x, _12339);
                                    float2 _22390;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21097 = _12340;
                                        _21097.y = 1.0 - _12339;
                                        _22390 = _21097;
                                    }
                                    else
                                    {
                                        _22390 = _12340;
                                    }
                                    float2 _12357 = float2(_12309.x, _12319.y);
                                    float2 _22391;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21104 = _12357;
                                        _21104.y = 1.0 - _12319.y;
                                        _22391 = _21104;
                                    }
                                    else
                                    {
                                        _22391 = _12357;
                                    }
                                    float2 _22392;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21110 = _12319;
                                        _21110.y = 1.0 - _12319.y;
                                        _22392 = _21110;
                                    }
                                    else
                                    {
                                        _22392 = _12319;
                                    }
                                    _22414 = (((gBackdrop5.sample(gLinear, _22389, level(0.0)) * _12296.x) + (gBackdrop5.sample(gLinear, _22390, level(0.0)) * _12299.x)) * _12296.y) + (((gBackdrop5.sample(gLinear, _22391, level(0.0)) * _12296.x) + (gBackdrop5.sample(gLinear, _22392, level(0.0)) * _12299.x)) * _12299.y);
                                }
                                _22413 = _22414;
                            }
                            _22412 = _22413;
                        }
                        _22411 = _22412;
                    }
                    _22410 = _22411;
                }
                float3 _22441;
                if ((_11287 > 0.0199999995529651641845703125) && (_11283 < 5))
                {
                    int _11300 = _11283 + 1;
                    float4 _22436;
                    if (_11300 <= 0)
                    {
                        float2 _22435;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21115 = _6614;
                            _21115.y = 1.0 - _6614.y;
                            _22435 = _21115;
                        }
                        else
                        {
                            _22435 = _6614;
                        }
                        _22436 = gBackdrop0.sample(gLinear, _22435, level(0.0));
                    }
                    else
                    {
                        float4 _22437;
                        if (_11300 == 1)
                        {
                            float2 _12546 = (_6614 * _172.gLevel[1].xy) - float2(0.5);
                            float2 _12548 = floor(_12546);
                            float2 _12551 = _12546 - _12548;
                            float2 _12554 = _12551 * _12551;
                            float2 _12557 = _12554 * _12551;
                            float2 _12576 = (((_12557 * 3.0) - (_12554 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _12589 = _12557 * 0.16666667163372039794921875;
                            float2 _12592 = (((((-_12557) + (_12554 * 3.0)) - (_12551 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12576;
                            float2 _12595 = (((((_12557 * (-3.0)) + (_12554 * 3.0)) + (_12551 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12589;
                            float2 _12605 = ((_12548 - float2(0.5)) + (_12576 / _12592)) * _172.gLevel[1].zw;
                            float2 _12615 = ((_12548 + float2(1.5)) + (_12589 / _12595)) * _172.gLevel[1].zw;
                            float2 _22431;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21120 = _12605;
                                _21120.y = 1.0 - _12605.y;
                                _22431 = _21120;
                            }
                            else
                            {
                                _22431 = _12605;
                            }
                            float _12635 = _12605.y;
                            float2 _12636 = float2(_12615.x, _12635);
                            float2 _22432;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21126 = _12636;
                                _21126.y = 1.0 - _12635;
                                _22432 = _21126;
                            }
                            else
                            {
                                _22432 = _12636;
                            }
                            float2 _12653 = float2(_12605.x, _12615.y);
                            float2 _22433;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21133 = _12653;
                                _21133.y = 1.0 - _12615.y;
                                _22433 = _21133;
                            }
                            else
                            {
                                _22433 = _12653;
                            }
                            float2 _22434;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21139 = _12615;
                                _21139.y = 1.0 - _12615.y;
                                _22434 = _21139;
                            }
                            else
                            {
                                _22434 = _12615;
                            }
                            _22437 = (((gBackdrop1.sample(gLinear, _22431, level(0.0)) * _12592.x) + (gBackdrop1.sample(gLinear, _22432, level(0.0)) * _12595.x)) * _12592.y) + (((gBackdrop1.sample(gLinear, _22433, level(0.0)) * _12592.x) + (gBackdrop1.sample(gLinear, _22434, level(0.0)) * _12595.x)) * _12595.y);
                        }
                        else
                        {
                            float4 _22438;
                            if (_11300 == 2)
                            {
                                float2 _12753 = (_6614 * _172.gLevel[2].xy) - float2(0.5);
                                float2 _12755 = floor(_12753);
                                float2 _12758 = _12753 - _12755;
                                float2 _12761 = _12758 * _12758;
                                float2 _12764 = _12761 * _12758;
                                float2 _12783 = (((_12764 * 3.0) - (_12761 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _12796 = _12764 * 0.16666667163372039794921875;
                                float2 _12799 = (((((-_12764) + (_12761 * 3.0)) - (_12758 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12783;
                                float2 _12802 = (((((_12764 * (-3.0)) + (_12761 * 3.0)) + (_12758 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12796;
                                float2 _12812 = ((_12755 - float2(0.5)) + (_12783 / _12799)) * _172.gLevel[2].zw;
                                float2 _12822 = ((_12755 + float2(1.5)) + (_12796 / _12802)) * _172.gLevel[2].zw;
                                float2 _22427;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21146 = _12812;
                                    _21146.y = 1.0 - _12812.y;
                                    _22427 = _21146;
                                }
                                else
                                {
                                    _22427 = _12812;
                                }
                                float _12842 = _12812.y;
                                float2 _12843 = float2(_12822.x, _12842);
                                float2 _22428;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21152 = _12843;
                                    _21152.y = 1.0 - _12842;
                                    _22428 = _21152;
                                }
                                else
                                {
                                    _22428 = _12843;
                                }
                                float2 _12860 = float2(_12812.x, _12822.y);
                                float2 _22429;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21159 = _12860;
                                    _21159.y = 1.0 - _12822.y;
                                    _22429 = _21159;
                                }
                                else
                                {
                                    _22429 = _12860;
                                }
                                float2 _22430;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21165 = _12822;
                                    _21165.y = 1.0 - _12822.y;
                                    _22430 = _21165;
                                }
                                else
                                {
                                    _22430 = _12822;
                                }
                                _22438 = (((gBackdrop2.sample(gLinear, _22427, level(0.0)) * _12799.x) + (gBackdrop2.sample(gLinear, _22428, level(0.0)) * _12802.x)) * _12799.y) + (((gBackdrop2.sample(gLinear, _22429, level(0.0)) * _12799.x) + (gBackdrop2.sample(gLinear, _22430, level(0.0)) * _12802.x)) * _12802.y);
                            }
                            else
                            {
                                float4 _22439;
                                if (_11300 == 3)
                                {
                                    float2 _12960 = (_6614 * _172.gLevel[3].xy) - float2(0.5);
                                    float2 _12962 = floor(_12960);
                                    float2 _12965 = _12960 - _12962;
                                    float2 _12968 = _12965 * _12965;
                                    float2 _12971 = _12968 * _12965;
                                    float2 _12990 = (((_12971 * 3.0) - (_12968 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _13003 = _12971 * 0.16666667163372039794921875;
                                    float2 _13006 = (((((-_12971) + (_12968 * 3.0)) - (_12965 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12990;
                                    float2 _13009 = (((((_12971 * (-3.0)) + (_12968 * 3.0)) + (_12965 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13003;
                                    float2 _13019 = ((_12962 - float2(0.5)) + (_12990 / _13006)) * _172.gLevel[3].zw;
                                    float2 _13029 = ((_12962 + float2(1.5)) + (_13003 / _13009)) * _172.gLevel[3].zw;
                                    float2 _22423;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21172 = _13019;
                                        _21172.y = 1.0 - _13019.y;
                                        _22423 = _21172;
                                    }
                                    else
                                    {
                                        _22423 = _13019;
                                    }
                                    float _13049 = _13019.y;
                                    float2 _13050 = float2(_13029.x, _13049);
                                    float2 _22424;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21178 = _13050;
                                        _21178.y = 1.0 - _13049;
                                        _22424 = _21178;
                                    }
                                    else
                                    {
                                        _22424 = _13050;
                                    }
                                    float2 _13067 = float2(_13019.x, _13029.y);
                                    float2 _22425;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21185 = _13067;
                                        _21185.y = 1.0 - _13029.y;
                                        _22425 = _21185;
                                    }
                                    else
                                    {
                                        _22425 = _13067;
                                    }
                                    float2 _22426;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21191 = _13029;
                                        _21191.y = 1.0 - _13029.y;
                                        _22426 = _21191;
                                    }
                                    else
                                    {
                                        _22426 = _13029;
                                    }
                                    _22439 = (((gBackdrop3.sample(gLinear, _22423, level(0.0)) * _13006.x) + (gBackdrop3.sample(gLinear, _22424, level(0.0)) * _13009.x)) * _13006.y) + (((gBackdrop3.sample(gLinear, _22425, level(0.0)) * _13006.x) + (gBackdrop3.sample(gLinear, _22426, level(0.0)) * _13009.x)) * _13009.y);
                                }
                                else
                                {
                                    float4 _22440;
                                    if (_11300 == 4)
                                    {
                                        float2 _13167 = (_6614 * _172.gLevel[4].xy) - float2(0.5);
                                        float2 _13169 = floor(_13167);
                                        float2 _13172 = _13167 - _13169;
                                        float2 _13175 = _13172 * _13172;
                                        float2 _13178 = _13175 * _13172;
                                        float2 _13197 = (((_13178 * 3.0) - (_13175 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _13210 = _13178 * 0.16666667163372039794921875;
                                        float2 _13213 = (((((-_13178) + (_13175 * 3.0)) - (_13172 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13197;
                                        float2 _13216 = (((((_13178 * (-3.0)) + (_13175 * 3.0)) + (_13172 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13210;
                                        float2 _13226 = ((_13169 - float2(0.5)) + (_13197 / _13213)) * _172.gLevel[4].zw;
                                        float2 _13236 = ((_13169 + float2(1.5)) + (_13210 / _13216)) * _172.gLevel[4].zw;
                                        float2 _22419;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21198 = _13226;
                                            _21198.y = 1.0 - _13226.y;
                                            _22419 = _21198;
                                        }
                                        else
                                        {
                                            _22419 = _13226;
                                        }
                                        float _13256 = _13226.y;
                                        float2 _13257 = float2(_13236.x, _13256);
                                        float2 _22420;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21204 = _13257;
                                            _21204.y = 1.0 - _13256;
                                            _22420 = _21204;
                                        }
                                        else
                                        {
                                            _22420 = _13257;
                                        }
                                        float2 _13274 = float2(_13226.x, _13236.y);
                                        float2 _22421;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21211 = _13274;
                                            _21211.y = 1.0 - _13236.y;
                                            _22421 = _21211;
                                        }
                                        else
                                        {
                                            _22421 = _13274;
                                        }
                                        float2 _22422;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21217 = _13236;
                                            _21217.y = 1.0 - _13236.y;
                                            _22422 = _21217;
                                        }
                                        else
                                        {
                                            _22422 = _13236;
                                        }
                                        _22440 = (((gBackdrop4.sample(gLinear, _22419, level(0.0)) * _13213.x) + (gBackdrop4.sample(gLinear, _22420, level(0.0)) * _13216.x)) * _13213.y) + (((gBackdrop4.sample(gLinear, _22421, level(0.0)) * _13213.x) + (gBackdrop4.sample(gLinear, _22422, level(0.0)) * _13216.x)) * _13216.y);
                                    }
                                    else
                                    {
                                        float2 _13374 = (_6614 * _172.gLevel[5].xy) - float2(0.5);
                                        float2 _13376 = floor(_13374);
                                        float2 _13379 = _13374 - _13376;
                                        float2 _13382 = _13379 * _13379;
                                        float2 _13385 = _13382 * _13379;
                                        float2 _13404 = (((_13385 * 3.0) - (_13382 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _13417 = _13385 * 0.16666667163372039794921875;
                                        float2 _13420 = (((((-_13385) + (_13382 * 3.0)) - (_13379 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13404;
                                        float2 _13423 = (((((_13385 * (-3.0)) + (_13382 * 3.0)) + (_13379 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13417;
                                        float2 _13433 = ((_13376 - float2(0.5)) + (_13404 / _13420)) * _172.gLevel[5].zw;
                                        float2 _13443 = ((_13376 + float2(1.5)) + (_13417 / _13423)) * _172.gLevel[5].zw;
                                        float2 _22415;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21224 = _13433;
                                            _21224.y = 1.0 - _13433.y;
                                            _22415 = _21224;
                                        }
                                        else
                                        {
                                            _22415 = _13433;
                                        }
                                        float _13463 = _13433.y;
                                        float2 _13464 = float2(_13443.x, _13463);
                                        float2 _22416;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21230 = _13464;
                                            _21230.y = 1.0 - _13463;
                                            _22416 = _21230;
                                        }
                                        else
                                        {
                                            _22416 = _13464;
                                        }
                                        float2 _13481 = float2(_13433.x, _13443.y);
                                        float2 _22417;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21237 = _13481;
                                            _21237.y = 1.0 - _13443.y;
                                            _22417 = _21237;
                                        }
                                        else
                                        {
                                            _22417 = _13481;
                                        }
                                        float2 _22418;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21243 = _13443;
                                            _21243.y = 1.0 - _13443.y;
                                            _22418 = _21243;
                                        }
                                        else
                                        {
                                            _22418 = _13443;
                                        }
                                        _22440 = (((gBackdrop5.sample(gLinear, _22415, level(0.0)) * _13420.x) + (gBackdrop5.sample(gLinear, _22416, level(0.0)) * _13423.x)) * _13420.y) + (((gBackdrop5.sample(gLinear, _22417, level(0.0)) * _13420.x) + (gBackdrop5.sample(gLinear, _22418, level(0.0)) * _13423.x)) * _13423.y);
                                    }
                                    _22439 = _22440;
                                }
                                _22438 = _22439;
                            }
                            _22437 = _22438;
                        }
                        _22436 = _22437;
                    }
                    _22441 = mix(_22410.xyz, _22436.xyz, float3(_11287));
                }
                else
                {
                    _22441 = _22410.xyz;
                }
                float2 _6625 = (_6563 + _6575) + (_6578 * _6600);
                float _13571 = fast::clamp(log2(fast::max(_6477, 1.0)) - 1.0, 0.0, 5.0);
                int _13574 = int(floor(_13571));
                float _13578 = _13571 - float(_13574);
                float4 _22516;
                if (_13574 <= 0)
                {
                    float2 _22515;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _21250 = _6625;
                        _21250.y = 1.0 - _6625.y;
                        _22515 = _21250;
                    }
                    else
                    {
                        _22515 = _6625;
                    }
                    _22516 = gBackdrop0.sample(gLinear, _22515, level(0.0));
                }
                else
                {
                    float4 _22517;
                    if (_13574 == 1)
                    {
                        float2 _13713 = (_6625 * _172.gLevel[1].xy) - float2(0.5);
                        float2 _13715 = floor(_13713);
                        float2 _13718 = _13713 - _13715;
                        float2 _13721 = _13718 * _13718;
                        float2 _13724 = _13721 * _13718;
                        float2 _13743 = (((_13724 * 3.0) - (_13721 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _13756 = _13724 * 0.16666667163372039794921875;
                        float2 _13759 = (((((-_13724) + (_13721 * 3.0)) - (_13718 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13743;
                        float2 _13762 = (((((_13724 * (-3.0)) + (_13721 * 3.0)) + (_13718 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13756;
                        float2 _13772 = ((_13715 - float2(0.5)) + (_13743 / _13759)) * _172.gLevel[1].zw;
                        float2 _13782 = ((_13715 + float2(1.5)) + (_13756 / _13762)) * _172.gLevel[1].zw;
                        float2 _22511;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21255 = _13772;
                            _21255.y = 1.0 - _13772.y;
                            _22511 = _21255;
                        }
                        else
                        {
                            _22511 = _13772;
                        }
                        float _13802 = _13772.y;
                        float2 _13803 = float2(_13782.x, _13802);
                        float2 _22512;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21261 = _13803;
                            _21261.y = 1.0 - _13802;
                            _22512 = _21261;
                        }
                        else
                        {
                            _22512 = _13803;
                        }
                        float2 _13820 = float2(_13772.x, _13782.y);
                        float2 _22513;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21268 = _13820;
                            _21268.y = 1.0 - _13782.y;
                            _22513 = _21268;
                        }
                        else
                        {
                            _22513 = _13820;
                        }
                        float2 _22514;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21274 = _13782;
                            _21274.y = 1.0 - _13782.y;
                            _22514 = _21274;
                        }
                        else
                        {
                            _22514 = _13782;
                        }
                        _22517 = (((gBackdrop1.sample(gLinear, _22511, level(0.0)) * _13759.x) + (gBackdrop1.sample(gLinear, _22512, level(0.0)) * _13762.x)) * _13759.y) + (((gBackdrop1.sample(gLinear, _22513, level(0.0)) * _13759.x) + (gBackdrop1.sample(gLinear, _22514, level(0.0)) * _13762.x)) * _13762.y);
                    }
                    else
                    {
                        float4 _22518;
                        if (_13574 == 2)
                        {
                            float2 _13920 = (_6625 * _172.gLevel[2].xy) - float2(0.5);
                            float2 _13922 = floor(_13920);
                            float2 _13925 = _13920 - _13922;
                            float2 _13928 = _13925 * _13925;
                            float2 _13931 = _13928 * _13925;
                            float2 _13950 = (((_13931 * 3.0) - (_13928 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _13963 = _13931 * 0.16666667163372039794921875;
                            float2 _13966 = (((((-_13931) + (_13928 * 3.0)) - (_13925 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13950;
                            float2 _13969 = (((((_13931 * (-3.0)) + (_13928 * 3.0)) + (_13925 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13963;
                            float2 _13979 = ((_13922 - float2(0.5)) + (_13950 / _13966)) * _172.gLevel[2].zw;
                            float2 _13989 = ((_13922 + float2(1.5)) + (_13963 / _13969)) * _172.gLevel[2].zw;
                            float2 _22507;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21281 = _13979;
                                _21281.y = 1.0 - _13979.y;
                                _22507 = _21281;
                            }
                            else
                            {
                                _22507 = _13979;
                            }
                            float _14009 = _13979.y;
                            float2 _14010 = float2(_13989.x, _14009);
                            float2 _22508;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21287 = _14010;
                                _21287.y = 1.0 - _14009;
                                _22508 = _21287;
                            }
                            else
                            {
                                _22508 = _14010;
                            }
                            float2 _14027 = float2(_13979.x, _13989.y);
                            float2 _22509;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21294 = _14027;
                                _21294.y = 1.0 - _13989.y;
                                _22509 = _21294;
                            }
                            else
                            {
                                _22509 = _14027;
                            }
                            float2 _22510;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21300 = _13989;
                                _21300.y = 1.0 - _13989.y;
                                _22510 = _21300;
                            }
                            else
                            {
                                _22510 = _13989;
                            }
                            _22518 = (((gBackdrop2.sample(gLinear, _22507, level(0.0)) * _13966.x) + (gBackdrop2.sample(gLinear, _22508, level(0.0)) * _13969.x)) * _13966.y) + (((gBackdrop2.sample(gLinear, _22509, level(0.0)) * _13966.x) + (gBackdrop2.sample(gLinear, _22510, level(0.0)) * _13969.x)) * _13969.y);
                        }
                        else
                        {
                            float4 _22519;
                            if (_13574 == 3)
                            {
                                float2 _14127 = (_6625 * _172.gLevel[3].xy) - float2(0.5);
                                float2 _14129 = floor(_14127);
                                float2 _14132 = _14127 - _14129;
                                float2 _14135 = _14132 * _14132;
                                float2 _14138 = _14135 * _14132;
                                float2 _14157 = (((_14138 * 3.0) - (_14135 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _14170 = _14138 * 0.16666667163372039794921875;
                                float2 _14173 = (((((-_14138) + (_14135 * 3.0)) - (_14132 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14157;
                                float2 _14176 = (((((_14138 * (-3.0)) + (_14135 * 3.0)) + (_14132 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14170;
                                float2 _14186 = ((_14129 - float2(0.5)) + (_14157 / _14173)) * _172.gLevel[3].zw;
                                float2 _14196 = ((_14129 + float2(1.5)) + (_14170 / _14176)) * _172.gLevel[3].zw;
                                float2 _22503;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21307 = _14186;
                                    _21307.y = 1.0 - _14186.y;
                                    _22503 = _21307;
                                }
                                else
                                {
                                    _22503 = _14186;
                                }
                                float _14216 = _14186.y;
                                float2 _14217 = float2(_14196.x, _14216);
                                float2 _22504;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21313 = _14217;
                                    _21313.y = 1.0 - _14216;
                                    _22504 = _21313;
                                }
                                else
                                {
                                    _22504 = _14217;
                                }
                                float2 _14234 = float2(_14186.x, _14196.y);
                                float2 _22505;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21320 = _14234;
                                    _21320.y = 1.0 - _14196.y;
                                    _22505 = _21320;
                                }
                                else
                                {
                                    _22505 = _14234;
                                }
                                float2 _22506;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21326 = _14196;
                                    _21326.y = 1.0 - _14196.y;
                                    _22506 = _21326;
                                }
                                else
                                {
                                    _22506 = _14196;
                                }
                                _22519 = (((gBackdrop3.sample(gLinear, _22503, level(0.0)) * _14173.x) + (gBackdrop3.sample(gLinear, _22504, level(0.0)) * _14176.x)) * _14173.y) + (((gBackdrop3.sample(gLinear, _22505, level(0.0)) * _14173.x) + (gBackdrop3.sample(gLinear, _22506, level(0.0)) * _14176.x)) * _14176.y);
                            }
                            else
                            {
                                float4 _22520;
                                if (_13574 == 4)
                                {
                                    float2 _14334 = (_6625 * _172.gLevel[4].xy) - float2(0.5);
                                    float2 _14336 = floor(_14334);
                                    float2 _14339 = _14334 - _14336;
                                    float2 _14342 = _14339 * _14339;
                                    float2 _14345 = _14342 * _14339;
                                    float2 _14364 = (((_14345 * 3.0) - (_14342 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _14377 = _14345 * 0.16666667163372039794921875;
                                    float2 _14380 = (((((-_14345) + (_14342 * 3.0)) - (_14339 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14364;
                                    float2 _14383 = (((((_14345 * (-3.0)) + (_14342 * 3.0)) + (_14339 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14377;
                                    float2 _14393 = ((_14336 - float2(0.5)) + (_14364 / _14380)) * _172.gLevel[4].zw;
                                    float2 _14403 = ((_14336 + float2(1.5)) + (_14377 / _14383)) * _172.gLevel[4].zw;
                                    float2 _22499;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21333 = _14393;
                                        _21333.y = 1.0 - _14393.y;
                                        _22499 = _21333;
                                    }
                                    else
                                    {
                                        _22499 = _14393;
                                    }
                                    float _14423 = _14393.y;
                                    float2 _14424 = float2(_14403.x, _14423);
                                    float2 _22500;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21339 = _14424;
                                        _21339.y = 1.0 - _14423;
                                        _22500 = _21339;
                                    }
                                    else
                                    {
                                        _22500 = _14424;
                                    }
                                    float2 _14441 = float2(_14393.x, _14403.y);
                                    float2 _22501;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21346 = _14441;
                                        _21346.y = 1.0 - _14403.y;
                                        _22501 = _21346;
                                    }
                                    else
                                    {
                                        _22501 = _14441;
                                    }
                                    float2 _22502;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21352 = _14403;
                                        _21352.y = 1.0 - _14403.y;
                                        _22502 = _21352;
                                    }
                                    else
                                    {
                                        _22502 = _14403;
                                    }
                                    _22520 = (((gBackdrop4.sample(gLinear, _22499, level(0.0)) * _14380.x) + (gBackdrop4.sample(gLinear, _22500, level(0.0)) * _14383.x)) * _14380.y) + (((gBackdrop4.sample(gLinear, _22501, level(0.0)) * _14380.x) + (gBackdrop4.sample(gLinear, _22502, level(0.0)) * _14383.x)) * _14383.y);
                                }
                                else
                                {
                                    float2 _14541 = (_6625 * _172.gLevel[5].xy) - float2(0.5);
                                    float2 _14543 = floor(_14541);
                                    float2 _14546 = _14541 - _14543;
                                    float2 _14549 = _14546 * _14546;
                                    float2 _14552 = _14549 * _14546;
                                    float2 _14571 = (((_14552 * 3.0) - (_14549 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _14584 = _14552 * 0.16666667163372039794921875;
                                    float2 _14587 = (((((-_14552) + (_14549 * 3.0)) - (_14546 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14571;
                                    float2 _14590 = (((((_14552 * (-3.0)) + (_14549 * 3.0)) + (_14546 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14584;
                                    float2 _14600 = ((_14543 - float2(0.5)) + (_14571 / _14587)) * _172.gLevel[5].zw;
                                    float2 _14610 = ((_14543 + float2(1.5)) + (_14584 / _14590)) * _172.gLevel[5].zw;
                                    float2 _22495;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21359 = _14600;
                                        _21359.y = 1.0 - _14600.y;
                                        _22495 = _21359;
                                    }
                                    else
                                    {
                                        _22495 = _14600;
                                    }
                                    float _14630 = _14600.y;
                                    float2 _14631 = float2(_14610.x, _14630);
                                    float2 _22496;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21365 = _14631;
                                        _21365.y = 1.0 - _14630;
                                        _22496 = _21365;
                                    }
                                    else
                                    {
                                        _22496 = _14631;
                                    }
                                    float2 _14648 = float2(_14600.x, _14610.y);
                                    float2 _22497;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21372 = _14648;
                                        _21372.y = 1.0 - _14610.y;
                                        _22497 = _21372;
                                    }
                                    else
                                    {
                                        _22497 = _14648;
                                    }
                                    float2 _22498;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21378 = _14610;
                                        _21378.y = 1.0 - _14610.y;
                                        _22498 = _21378;
                                    }
                                    else
                                    {
                                        _22498 = _14610;
                                    }
                                    _22520 = (((gBackdrop5.sample(gLinear, _22495, level(0.0)) * _14587.x) + (gBackdrop5.sample(gLinear, _22496, level(0.0)) * _14590.x)) * _14587.y) + (((gBackdrop5.sample(gLinear, _22497, level(0.0)) * _14587.x) + (gBackdrop5.sample(gLinear, _22498, level(0.0)) * _14590.x)) * _14590.y);
                                }
                                _22519 = _22520;
                            }
                            _22518 = _22519;
                        }
                        _22517 = _22518;
                    }
                    _22516 = _22517;
                }
                float3 _22547;
                if ((_13578 > 0.0199999995529651641845703125) && (_13574 < 5))
                {
                    int _13591 = _13574 + 1;
                    float4 _22542;
                    if (_13591 <= 0)
                    {
                        float2 _22541;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21383 = _6625;
                            _21383.y = 1.0 - _6625.y;
                            _22541 = _21383;
                        }
                        else
                        {
                            _22541 = _6625;
                        }
                        _22542 = gBackdrop0.sample(gLinear, _22541, level(0.0));
                    }
                    else
                    {
                        float4 _22543;
                        if (_13591 == 1)
                        {
                            float2 _14837 = (_6625 * _172.gLevel[1].xy) - float2(0.5);
                            float2 _14839 = floor(_14837);
                            float2 _14842 = _14837 - _14839;
                            float2 _14845 = _14842 * _14842;
                            float2 _14848 = _14845 * _14842;
                            float2 _14867 = (((_14848 * 3.0) - (_14845 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _14880 = _14848 * 0.16666667163372039794921875;
                            float2 _14883 = (((((-_14848) + (_14845 * 3.0)) - (_14842 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14867;
                            float2 _14886 = (((((_14848 * (-3.0)) + (_14845 * 3.0)) + (_14842 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14880;
                            float2 _14896 = ((_14839 - float2(0.5)) + (_14867 / _14883)) * _172.gLevel[1].zw;
                            float2 _14906 = ((_14839 + float2(1.5)) + (_14880 / _14886)) * _172.gLevel[1].zw;
                            float2 _22537;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21388 = _14896;
                                _21388.y = 1.0 - _14896.y;
                                _22537 = _21388;
                            }
                            else
                            {
                                _22537 = _14896;
                            }
                            float _14926 = _14896.y;
                            float2 _14927 = float2(_14906.x, _14926);
                            float2 _22538;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21394 = _14927;
                                _21394.y = 1.0 - _14926;
                                _22538 = _21394;
                            }
                            else
                            {
                                _22538 = _14927;
                            }
                            float2 _14944 = float2(_14896.x, _14906.y);
                            float2 _22539;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21401 = _14944;
                                _21401.y = 1.0 - _14906.y;
                                _22539 = _21401;
                            }
                            else
                            {
                                _22539 = _14944;
                            }
                            float2 _22540;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21407 = _14906;
                                _21407.y = 1.0 - _14906.y;
                                _22540 = _21407;
                            }
                            else
                            {
                                _22540 = _14906;
                            }
                            _22543 = (((gBackdrop1.sample(gLinear, _22537, level(0.0)) * _14883.x) + (gBackdrop1.sample(gLinear, _22538, level(0.0)) * _14886.x)) * _14883.y) + (((gBackdrop1.sample(gLinear, _22539, level(0.0)) * _14883.x) + (gBackdrop1.sample(gLinear, _22540, level(0.0)) * _14886.x)) * _14886.y);
                        }
                        else
                        {
                            float4 _22544;
                            if (_13591 == 2)
                            {
                                float2 _15044 = (_6625 * _172.gLevel[2].xy) - float2(0.5);
                                float2 _15046 = floor(_15044);
                                float2 _15049 = _15044 - _15046;
                                float2 _15052 = _15049 * _15049;
                                float2 _15055 = _15052 * _15049;
                                float2 _15074 = (((_15055 * 3.0) - (_15052 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _15087 = _15055 * 0.16666667163372039794921875;
                                float2 _15090 = (((((-_15055) + (_15052 * 3.0)) - (_15049 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15074;
                                float2 _15093 = (((((_15055 * (-3.0)) + (_15052 * 3.0)) + (_15049 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15087;
                                float2 _15103 = ((_15046 - float2(0.5)) + (_15074 / _15090)) * _172.gLevel[2].zw;
                                float2 _15113 = ((_15046 + float2(1.5)) + (_15087 / _15093)) * _172.gLevel[2].zw;
                                float2 _22533;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21414 = _15103;
                                    _21414.y = 1.0 - _15103.y;
                                    _22533 = _21414;
                                }
                                else
                                {
                                    _22533 = _15103;
                                }
                                float _15133 = _15103.y;
                                float2 _15134 = float2(_15113.x, _15133);
                                float2 _22534;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21420 = _15134;
                                    _21420.y = 1.0 - _15133;
                                    _22534 = _21420;
                                }
                                else
                                {
                                    _22534 = _15134;
                                }
                                float2 _15151 = float2(_15103.x, _15113.y);
                                float2 _22535;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21427 = _15151;
                                    _21427.y = 1.0 - _15113.y;
                                    _22535 = _21427;
                                }
                                else
                                {
                                    _22535 = _15151;
                                }
                                float2 _22536;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21433 = _15113;
                                    _21433.y = 1.0 - _15113.y;
                                    _22536 = _21433;
                                }
                                else
                                {
                                    _22536 = _15113;
                                }
                                _22544 = (((gBackdrop2.sample(gLinear, _22533, level(0.0)) * _15090.x) + (gBackdrop2.sample(gLinear, _22534, level(0.0)) * _15093.x)) * _15090.y) + (((gBackdrop2.sample(gLinear, _22535, level(0.0)) * _15090.x) + (gBackdrop2.sample(gLinear, _22536, level(0.0)) * _15093.x)) * _15093.y);
                            }
                            else
                            {
                                float4 _22545;
                                if (_13591 == 3)
                                {
                                    float2 _15251 = (_6625 * _172.gLevel[3].xy) - float2(0.5);
                                    float2 _15253 = floor(_15251);
                                    float2 _15256 = _15251 - _15253;
                                    float2 _15259 = _15256 * _15256;
                                    float2 _15262 = _15259 * _15256;
                                    float2 _15281 = (((_15262 * 3.0) - (_15259 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _15294 = _15262 * 0.16666667163372039794921875;
                                    float2 _15297 = (((((-_15262) + (_15259 * 3.0)) - (_15256 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15281;
                                    float2 _15300 = (((((_15262 * (-3.0)) + (_15259 * 3.0)) + (_15256 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15294;
                                    float2 _15310 = ((_15253 - float2(0.5)) + (_15281 / _15297)) * _172.gLevel[3].zw;
                                    float2 _15320 = ((_15253 + float2(1.5)) + (_15294 / _15300)) * _172.gLevel[3].zw;
                                    float2 _22529;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21440 = _15310;
                                        _21440.y = 1.0 - _15310.y;
                                        _22529 = _21440;
                                    }
                                    else
                                    {
                                        _22529 = _15310;
                                    }
                                    float _15340 = _15310.y;
                                    float2 _15341 = float2(_15320.x, _15340);
                                    float2 _22530;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21446 = _15341;
                                        _21446.y = 1.0 - _15340;
                                        _22530 = _21446;
                                    }
                                    else
                                    {
                                        _22530 = _15341;
                                    }
                                    float2 _15358 = float2(_15310.x, _15320.y);
                                    float2 _22531;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21453 = _15358;
                                        _21453.y = 1.0 - _15320.y;
                                        _22531 = _21453;
                                    }
                                    else
                                    {
                                        _22531 = _15358;
                                    }
                                    float2 _22532;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21459 = _15320;
                                        _21459.y = 1.0 - _15320.y;
                                        _22532 = _21459;
                                    }
                                    else
                                    {
                                        _22532 = _15320;
                                    }
                                    _22545 = (((gBackdrop3.sample(gLinear, _22529, level(0.0)) * _15297.x) + (gBackdrop3.sample(gLinear, _22530, level(0.0)) * _15300.x)) * _15297.y) + (((gBackdrop3.sample(gLinear, _22531, level(0.0)) * _15297.x) + (gBackdrop3.sample(gLinear, _22532, level(0.0)) * _15300.x)) * _15300.y);
                                }
                                else
                                {
                                    float4 _22546;
                                    if (_13591 == 4)
                                    {
                                        float2 _15458 = (_6625 * _172.gLevel[4].xy) - float2(0.5);
                                        float2 _15460 = floor(_15458);
                                        float2 _15463 = _15458 - _15460;
                                        float2 _15466 = _15463 * _15463;
                                        float2 _15469 = _15466 * _15463;
                                        float2 _15488 = (((_15469 * 3.0) - (_15466 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _15501 = _15469 * 0.16666667163372039794921875;
                                        float2 _15504 = (((((-_15469) + (_15466 * 3.0)) - (_15463 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15488;
                                        float2 _15507 = (((((_15469 * (-3.0)) + (_15466 * 3.0)) + (_15463 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15501;
                                        float2 _15517 = ((_15460 - float2(0.5)) + (_15488 / _15504)) * _172.gLevel[4].zw;
                                        float2 _15527 = ((_15460 + float2(1.5)) + (_15501 / _15507)) * _172.gLevel[4].zw;
                                        float2 _22525;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21466 = _15517;
                                            _21466.y = 1.0 - _15517.y;
                                            _22525 = _21466;
                                        }
                                        else
                                        {
                                            _22525 = _15517;
                                        }
                                        float _15547 = _15517.y;
                                        float2 _15548 = float2(_15527.x, _15547);
                                        float2 _22526;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21472 = _15548;
                                            _21472.y = 1.0 - _15547;
                                            _22526 = _21472;
                                        }
                                        else
                                        {
                                            _22526 = _15548;
                                        }
                                        float2 _15565 = float2(_15517.x, _15527.y);
                                        float2 _22527;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21479 = _15565;
                                            _21479.y = 1.0 - _15527.y;
                                            _22527 = _21479;
                                        }
                                        else
                                        {
                                            _22527 = _15565;
                                        }
                                        float2 _22528;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21485 = _15527;
                                            _21485.y = 1.0 - _15527.y;
                                            _22528 = _21485;
                                        }
                                        else
                                        {
                                            _22528 = _15527;
                                        }
                                        _22546 = (((gBackdrop4.sample(gLinear, _22525, level(0.0)) * _15504.x) + (gBackdrop4.sample(gLinear, _22526, level(0.0)) * _15507.x)) * _15504.y) + (((gBackdrop4.sample(gLinear, _22527, level(0.0)) * _15504.x) + (gBackdrop4.sample(gLinear, _22528, level(0.0)) * _15507.x)) * _15507.y);
                                    }
                                    else
                                    {
                                        float2 _15665 = (_6625 * _172.gLevel[5].xy) - float2(0.5);
                                        float2 _15667 = floor(_15665);
                                        float2 _15670 = _15665 - _15667;
                                        float2 _15673 = _15670 * _15670;
                                        float2 _15676 = _15673 * _15670;
                                        float2 _15695 = (((_15676 * 3.0) - (_15673 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _15708 = _15676 * 0.16666667163372039794921875;
                                        float2 _15711 = (((((-_15676) + (_15673 * 3.0)) - (_15670 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15695;
                                        float2 _15714 = (((((_15676 * (-3.0)) + (_15673 * 3.0)) + (_15670 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15708;
                                        float2 _15724 = ((_15667 - float2(0.5)) + (_15695 / _15711)) * _172.gLevel[5].zw;
                                        float2 _15734 = ((_15667 + float2(1.5)) + (_15708 / _15714)) * _172.gLevel[5].zw;
                                        float2 _22521;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21492 = _15724;
                                            _21492.y = 1.0 - _15724.y;
                                            _22521 = _21492;
                                        }
                                        else
                                        {
                                            _22521 = _15724;
                                        }
                                        float _15754 = _15724.y;
                                        float2 _15755 = float2(_15734.x, _15754);
                                        float2 _22522;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21498 = _15755;
                                            _21498.y = 1.0 - _15754;
                                            _22522 = _21498;
                                        }
                                        else
                                        {
                                            _22522 = _15755;
                                        }
                                        float2 _15772 = float2(_15724.x, _15734.y);
                                        float2 _22523;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21505 = _15772;
                                            _21505.y = 1.0 - _15734.y;
                                            _22523 = _21505;
                                        }
                                        else
                                        {
                                            _22523 = _15772;
                                        }
                                        float2 _22524;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21511 = _15734;
                                            _21511.y = 1.0 - _15734.y;
                                            _22524 = _21511;
                                        }
                                        else
                                        {
                                            _22524 = _15734;
                                        }
                                        _22546 = (((gBackdrop5.sample(gLinear, _22521, level(0.0)) * _15711.x) + (gBackdrop5.sample(gLinear, _22522, level(0.0)) * _15714.x)) * _15711.y) + (((gBackdrop5.sample(gLinear, _22523, level(0.0)) * _15711.x) + (gBackdrop5.sample(gLinear, _22524, level(0.0)) * _15714.x)) * _15714.y);
                                    }
                                    _22545 = _22546;
                                }
                                _22544 = _22545;
                            }
                            _22543 = _22544;
                        }
                        _22542 = _22543;
                    }
                    _22547 = mix(_22516.xyz, _22542.xyz, float3(_13578));
                }
                else
                {
                    _22547 = _22516.xyz;
                }
                _22837 = float3(_22335.x, _22441.y, _22547.z);
            }
            else
            {
                float2 _6633 = _6563 + _6575;
                float _15862 = fast::clamp(log2(fast::max(_6477, 1.0)) - 1.0, 0.0, 5.0);
                int _15865 = int(floor(_15862));
                float _15869 = _15862 - float(_15865);
                float4 _22251;
                if (_15865 <= 0)
                {
                    float2 _22250;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _21518 = _6633;
                        _21518.y = 1.0 - _6633.y;
                        _22250 = _21518;
                    }
                    else
                    {
                        _22250 = _6633;
                    }
                    _22251 = gBackdrop0.sample(gLinear, _22250, level(0.0));
                }
                else
                {
                    float4 _22252;
                    if (_15865 == 1)
                    {
                        float2 _16004 = (_6633 * _172.gLevel[1].xy) - float2(0.5);
                        float2 _16006 = floor(_16004);
                        float2 _16009 = _16004 - _16006;
                        float2 _16012 = _16009 * _16009;
                        float2 _16015 = _16012 * _16009;
                        float2 _16034 = (((_16015 * 3.0) - (_16012 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _16047 = _16015 * 0.16666667163372039794921875;
                        float2 _16050 = (((((-_16015) + (_16012 * 3.0)) - (_16009 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16034;
                        float2 _16053 = (((((_16015 * (-3.0)) + (_16012 * 3.0)) + (_16009 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16047;
                        float2 _16063 = ((_16006 - float2(0.5)) + (_16034 / _16050)) * _172.gLevel[1].zw;
                        float2 _16073 = ((_16006 + float2(1.5)) + (_16047 / _16053)) * _172.gLevel[1].zw;
                        float2 _22246;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21523 = _16063;
                            _21523.y = 1.0 - _16063.y;
                            _22246 = _21523;
                        }
                        else
                        {
                            _22246 = _16063;
                        }
                        float _16093 = _16063.y;
                        float2 _16094 = float2(_16073.x, _16093);
                        float2 _22247;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21529 = _16094;
                            _21529.y = 1.0 - _16093;
                            _22247 = _21529;
                        }
                        else
                        {
                            _22247 = _16094;
                        }
                        float2 _16111 = float2(_16063.x, _16073.y);
                        float2 _22248;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21536 = _16111;
                            _21536.y = 1.0 - _16073.y;
                            _22248 = _21536;
                        }
                        else
                        {
                            _22248 = _16111;
                        }
                        float2 _22249;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21542 = _16073;
                            _21542.y = 1.0 - _16073.y;
                            _22249 = _21542;
                        }
                        else
                        {
                            _22249 = _16073;
                        }
                        _22252 = (((gBackdrop1.sample(gLinear, _22246, level(0.0)) * _16050.x) + (gBackdrop1.sample(gLinear, _22247, level(0.0)) * _16053.x)) * _16050.y) + (((gBackdrop1.sample(gLinear, _22248, level(0.0)) * _16050.x) + (gBackdrop1.sample(gLinear, _22249, level(0.0)) * _16053.x)) * _16053.y);
                    }
                    else
                    {
                        float4 _22253;
                        if (_15865 == 2)
                        {
                            float2 _16211 = (_6633 * _172.gLevel[2].xy) - float2(0.5);
                            float2 _16213 = floor(_16211);
                            float2 _16216 = _16211 - _16213;
                            float2 _16219 = _16216 * _16216;
                            float2 _16222 = _16219 * _16216;
                            float2 _16241 = (((_16222 * 3.0) - (_16219 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _16254 = _16222 * 0.16666667163372039794921875;
                            float2 _16257 = (((((-_16222) + (_16219 * 3.0)) - (_16216 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16241;
                            float2 _16260 = (((((_16222 * (-3.0)) + (_16219 * 3.0)) + (_16216 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16254;
                            float2 _16270 = ((_16213 - float2(0.5)) + (_16241 / _16257)) * _172.gLevel[2].zw;
                            float2 _16280 = ((_16213 + float2(1.5)) + (_16254 / _16260)) * _172.gLevel[2].zw;
                            float2 _22242;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21549 = _16270;
                                _21549.y = 1.0 - _16270.y;
                                _22242 = _21549;
                            }
                            else
                            {
                                _22242 = _16270;
                            }
                            float _16300 = _16270.y;
                            float2 _16301 = float2(_16280.x, _16300);
                            float2 _22243;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21555 = _16301;
                                _21555.y = 1.0 - _16300;
                                _22243 = _21555;
                            }
                            else
                            {
                                _22243 = _16301;
                            }
                            float2 _16318 = float2(_16270.x, _16280.y);
                            float2 _22244;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21562 = _16318;
                                _21562.y = 1.0 - _16280.y;
                                _22244 = _21562;
                            }
                            else
                            {
                                _22244 = _16318;
                            }
                            float2 _22245;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21568 = _16280;
                                _21568.y = 1.0 - _16280.y;
                                _22245 = _21568;
                            }
                            else
                            {
                                _22245 = _16280;
                            }
                            _22253 = (((gBackdrop2.sample(gLinear, _22242, level(0.0)) * _16257.x) + (gBackdrop2.sample(gLinear, _22243, level(0.0)) * _16260.x)) * _16257.y) + (((gBackdrop2.sample(gLinear, _22244, level(0.0)) * _16257.x) + (gBackdrop2.sample(gLinear, _22245, level(0.0)) * _16260.x)) * _16260.y);
                        }
                        else
                        {
                            float4 _22254;
                            if (_15865 == 3)
                            {
                                float2 _16418 = (_6633 * _172.gLevel[3].xy) - float2(0.5);
                                float2 _16420 = floor(_16418);
                                float2 _16423 = _16418 - _16420;
                                float2 _16426 = _16423 * _16423;
                                float2 _16429 = _16426 * _16423;
                                float2 _16448 = (((_16429 * 3.0) - (_16426 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _16461 = _16429 * 0.16666667163372039794921875;
                                float2 _16464 = (((((-_16429) + (_16426 * 3.0)) - (_16423 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16448;
                                float2 _16467 = (((((_16429 * (-3.0)) + (_16426 * 3.0)) + (_16423 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16461;
                                float2 _16477 = ((_16420 - float2(0.5)) + (_16448 / _16464)) * _172.gLevel[3].zw;
                                float2 _16487 = ((_16420 + float2(1.5)) + (_16461 / _16467)) * _172.gLevel[3].zw;
                                float2 _22238;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21575 = _16477;
                                    _21575.y = 1.0 - _16477.y;
                                    _22238 = _21575;
                                }
                                else
                                {
                                    _22238 = _16477;
                                }
                                float _16507 = _16477.y;
                                float2 _16508 = float2(_16487.x, _16507);
                                float2 _22239;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21581 = _16508;
                                    _21581.y = 1.0 - _16507;
                                    _22239 = _21581;
                                }
                                else
                                {
                                    _22239 = _16508;
                                }
                                float2 _16525 = float2(_16477.x, _16487.y);
                                float2 _22240;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21588 = _16525;
                                    _21588.y = 1.0 - _16487.y;
                                    _22240 = _21588;
                                }
                                else
                                {
                                    _22240 = _16525;
                                }
                                float2 _22241;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21594 = _16487;
                                    _21594.y = 1.0 - _16487.y;
                                    _22241 = _21594;
                                }
                                else
                                {
                                    _22241 = _16487;
                                }
                                _22254 = (((gBackdrop3.sample(gLinear, _22238, level(0.0)) * _16464.x) + (gBackdrop3.sample(gLinear, _22239, level(0.0)) * _16467.x)) * _16464.y) + (((gBackdrop3.sample(gLinear, _22240, level(0.0)) * _16464.x) + (gBackdrop3.sample(gLinear, _22241, level(0.0)) * _16467.x)) * _16467.y);
                            }
                            else
                            {
                                float4 _22255;
                                if (_15865 == 4)
                                {
                                    float2 _16625 = (_6633 * _172.gLevel[4].xy) - float2(0.5);
                                    float2 _16627 = floor(_16625);
                                    float2 _16630 = _16625 - _16627;
                                    float2 _16633 = _16630 * _16630;
                                    float2 _16636 = _16633 * _16630;
                                    float2 _16655 = (((_16636 * 3.0) - (_16633 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _16668 = _16636 * 0.16666667163372039794921875;
                                    float2 _16671 = (((((-_16636) + (_16633 * 3.0)) - (_16630 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16655;
                                    float2 _16674 = (((((_16636 * (-3.0)) + (_16633 * 3.0)) + (_16630 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16668;
                                    float2 _16684 = ((_16627 - float2(0.5)) + (_16655 / _16671)) * _172.gLevel[4].zw;
                                    float2 _16694 = ((_16627 + float2(1.5)) + (_16668 / _16674)) * _172.gLevel[4].zw;
                                    float2 _22234;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21601 = _16684;
                                        _21601.y = 1.0 - _16684.y;
                                        _22234 = _21601;
                                    }
                                    else
                                    {
                                        _22234 = _16684;
                                    }
                                    float _16714 = _16684.y;
                                    float2 _16715 = float2(_16694.x, _16714);
                                    float2 _22235;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21607 = _16715;
                                        _21607.y = 1.0 - _16714;
                                        _22235 = _21607;
                                    }
                                    else
                                    {
                                        _22235 = _16715;
                                    }
                                    float2 _16732 = float2(_16684.x, _16694.y);
                                    float2 _22236;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21614 = _16732;
                                        _21614.y = 1.0 - _16694.y;
                                        _22236 = _21614;
                                    }
                                    else
                                    {
                                        _22236 = _16732;
                                    }
                                    float2 _22237;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21620 = _16694;
                                        _21620.y = 1.0 - _16694.y;
                                        _22237 = _21620;
                                    }
                                    else
                                    {
                                        _22237 = _16694;
                                    }
                                    _22255 = (((gBackdrop4.sample(gLinear, _22234, level(0.0)) * _16671.x) + (gBackdrop4.sample(gLinear, _22235, level(0.0)) * _16674.x)) * _16671.y) + (((gBackdrop4.sample(gLinear, _22236, level(0.0)) * _16671.x) + (gBackdrop4.sample(gLinear, _22237, level(0.0)) * _16674.x)) * _16674.y);
                                }
                                else
                                {
                                    float2 _16832 = (_6633 * _172.gLevel[5].xy) - float2(0.5);
                                    float2 _16834 = floor(_16832);
                                    float2 _16837 = _16832 - _16834;
                                    float2 _16840 = _16837 * _16837;
                                    float2 _16843 = _16840 * _16837;
                                    float2 _16862 = (((_16843 * 3.0) - (_16840 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _16875 = _16843 * 0.16666667163372039794921875;
                                    float2 _16878 = (((((-_16843) + (_16840 * 3.0)) - (_16837 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16862;
                                    float2 _16881 = (((((_16843 * (-3.0)) + (_16840 * 3.0)) + (_16837 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16875;
                                    float2 _16891 = ((_16834 - float2(0.5)) + (_16862 / _16878)) * _172.gLevel[5].zw;
                                    float2 _16901 = ((_16834 + float2(1.5)) + (_16875 / _16881)) * _172.gLevel[5].zw;
                                    float2 _22230;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21627 = _16891;
                                        _21627.y = 1.0 - _16891.y;
                                        _22230 = _21627;
                                    }
                                    else
                                    {
                                        _22230 = _16891;
                                    }
                                    float _16921 = _16891.y;
                                    float2 _16922 = float2(_16901.x, _16921);
                                    float2 _22231;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21633 = _16922;
                                        _21633.y = 1.0 - _16921;
                                        _22231 = _21633;
                                    }
                                    else
                                    {
                                        _22231 = _16922;
                                    }
                                    float2 _16939 = float2(_16891.x, _16901.y);
                                    float2 _22232;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21640 = _16939;
                                        _21640.y = 1.0 - _16901.y;
                                        _22232 = _21640;
                                    }
                                    else
                                    {
                                        _22232 = _16939;
                                    }
                                    float2 _22233;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21646 = _16901;
                                        _21646.y = 1.0 - _16901.y;
                                        _22233 = _21646;
                                    }
                                    else
                                    {
                                        _22233 = _16901;
                                    }
                                    _22255 = (((gBackdrop5.sample(gLinear, _22230, level(0.0)) * _16878.x) + (gBackdrop5.sample(gLinear, _22231, level(0.0)) * _16881.x)) * _16878.y) + (((gBackdrop5.sample(gLinear, _22232, level(0.0)) * _16878.x) + (gBackdrop5.sample(gLinear, _22233, level(0.0)) * _16881.x)) * _16881.y);
                                }
                                _22254 = _22255;
                            }
                            _22253 = _22254;
                        }
                        _22252 = _22253;
                    }
                    _22251 = _22252;
                }
                float3 _22282;
                if ((_15869 > 0.0199999995529651641845703125) && (_15865 < 5))
                {
                    int _15882 = _15865 + 1;
                    float4 _22277;
                    if (_15882 <= 0)
                    {
                        float2 _22276;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21651 = _6633;
                            _21651.y = 1.0 - _6633.y;
                            _22276 = _21651;
                        }
                        else
                        {
                            _22276 = _6633;
                        }
                        _22277 = gBackdrop0.sample(gLinear, _22276, level(0.0));
                    }
                    else
                    {
                        float4 _22278;
                        if (_15882 == 1)
                        {
                            float2 _17128 = (_6633 * _172.gLevel[1].xy) - float2(0.5);
                            float2 _17130 = floor(_17128);
                            float2 _17133 = _17128 - _17130;
                            float2 _17136 = _17133 * _17133;
                            float2 _17139 = _17136 * _17133;
                            float2 _17158 = (((_17139 * 3.0) - (_17136 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _17171 = _17139 * 0.16666667163372039794921875;
                            float2 _17174 = (((((-_17139) + (_17136 * 3.0)) - (_17133 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17158;
                            float2 _17177 = (((((_17139 * (-3.0)) + (_17136 * 3.0)) + (_17133 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17171;
                            float2 _17187 = ((_17130 - float2(0.5)) + (_17158 / _17174)) * _172.gLevel[1].zw;
                            float2 _17197 = ((_17130 + float2(1.5)) + (_17171 / _17177)) * _172.gLevel[1].zw;
                            float2 _22272;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21656 = _17187;
                                _21656.y = 1.0 - _17187.y;
                                _22272 = _21656;
                            }
                            else
                            {
                                _22272 = _17187;
                            }
                            float _17217 = _17187.y;
                            float2 _17218 = float2(_17197.x, _17217);
                            float2 _22273;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21662 = _17218;
                                _21662.y = 1.0 - _17217;
                                _22273 = _21662;
                            }
                            else
                            {
                                _22273 = _17218;
                            }
                            float2 _17235 = float2(_17187.x, _17197.y);
                            float2 _22274;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21669 = _17235;
                                _21669.y = 1.0 - _17197.y;
                                _22274 = _21669;
                            }
                            else
                            {
                                _22274 = _17235;
                            }
                            float2 _22275;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21675 = _17197;
                                _21675.y = 1.0 - _17197.y;
                                _22275 = _21675;
                            }
                            else
                            {
                                _22275 = _17197;
                            }
                            _22278 = (((gBackdrop1.sample(gLinear, _22272, level(0.0)) * _17174.x) + (gBackdrop1.sample(gLinear, _22273, level(0.0)) * _17177.x)) * _17174.y) + (((gBackdrop1.sample(gLinear, _22274, level(0.0)) * _17174.x) + (gBackdrop1.sample(gLinear, _22275, level(0.0)) * _17177.x)) * _17177.y);
                        }
                        else
                        {
                            float4 _22279;
                            if (_15882 == 2)
                            {
                                float2 _17335 = (_6633 * _172.gLevel[2].xy) - float2(0.5);
                                float2 _17337 = floor(_17335);
                                float2 _17340 = _17335 - _17337;
                                float2 _17343 = _17340 * _17340;
                                float2 _17346 = _17343 * _17340;
                                float2 _17365 = (((_17346 * 3.0) - (_17343 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _17378 = _17346 * 0.16666667163372039794921875;
                                float2 _17381 = (((((-_17346) + (_17343 * 3.0)) - (_17340 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17365;
                                float2 _17384 = (((((_17346 * (-3.0)) + (_17343 * 3.0)) + (_17340 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17378;
                                float2 _17394 = ((_17337 - float2(0.5)) + (_17365 / _17381)) * _172.gLevel[2].zw;
                                float2 _17404 = ((_17337 + float2(1.5)) + (_17378 / _17384)) * _172.gLevel[2].zw;
                                float2 _22268;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21682 = _17394;
                                    _21682.y = 1.0 - _17394.y;
                                    _22268 = _21682;
                                }
                                else
                                {
                                    _22268 = _17394;
                                }
                                float _17424 = _17394.y;
                                float2 _17425 = float2(_17404.x, _17424);
                                float2 _22269;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21688 = _17425;
                                    _21688.y = 1.0 - _17424;
                                    _22269 = _21688;
                                }
                                else
                                {
                                    _22269 = _17425;
                                }
                                float2 _17442 = float2(_17394.x, _17404.y);
                                float2 _22270;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21695 = _17442;
                                    _21695.y = 1.0 - _17404.y;
                                    _22270 = _21695;
                                }
                                else
                                {
                                    _22270 = _17442;
                                }
                                float2 _22271;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21701 = _17404;
                                    _21701.y = 1.0 - _17404.y;
                                    _22271 = _21701;
                                }
                                else
                                {
                                    _22271 = _17404;
                                }
                                _22279 = (((gBackdrop2.sample(gLinear, _22268, level(0.0)) * _17381.x) + (gBackdrop2.sample(gLinear, _22269, level(0.0)) * _17384.x)) * _17381.y) + (((gBackdrop2.sample(gLinear, _22270, level(0.0)) * _17381.x) + (gBackdrop2.sample(gLinear, _22271, level(0.0)) * _17384.x)) * _17384.y);
                            }
                            else
                            {
                                float4 _22280;
                                if (_15882 == 3)
                                {
                                    float2 _17542 = (_6633 * _172.gLevel[3].xy) - float2(0.5);
                                    float2 _17544 = floor(_17542);
                                    float2 _17547 = _17542 - _17544;
                                    float2 _17550 = _17547 * _17547;
                                    float2 _17553 = _17550 * _17547;
                                    float2 _17572 = (((_17553 * 3.0) - (_17550 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _17585 = _17553 * 0.16666667163372039794921875;
                                    float2 _17588 = (((((-_17553) + (_17550 * 3.0)) - (_17547 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17572;
                                    float2 _17591 = (((((_17553 * (-3.0)) + (_17550 * 3.0)) + (_17547 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17585;
                                    float2 _17601 = ((_17544 - float2(0.5)) + (_17572 / _17588)) * _172.gLevel[3].zw;
                                    float2 _17611 = ((_17544 + float2(1.5)) + (_17585 / _17591)) * _172.gLevel[3].zw;
                                    float2 _22264;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21708 = _17601;
                                        _21708.y = 1.0 - _17601.y;
                                        _22264 = _21708;
                                    }
                                    else
                                    {
                                        _22264 = _17601;
                                    }
                                    float _17631 = _17601.y;
                                    float2 _17632 = float2(_17611.x, _17631);
                                    float2 _22265;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21714 = _17632;
                                        _21714.y = 1.0 - _17631;
                                        _22265 = _21714;
                                    }
                                    else
                                    {
                                        _22265 = _17632;
                                    }
                                    float2 _17649 = float2(_17601.x, _17611.y);
                                    float2 _22266;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21721 = _17649;
                                        _21721.y = 1.0 - _17611.y;
                                        _22266 = _21721;
                                    }
                                    else
                                    {
                                        _22266 = _17649;
                                    }
                                    float2 _22267;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21727 = _17611;
                                        _21727.y = 1.0 - _17611.y;
                                        _22267 = _21727;
                                    }
                                    else
                                    {
                                        _22267 = _17611;
                                    }
                                    _22280 = (((gBackdrop3.sample(gLinear, _22264, level(0.0)) * _17588.x) + (gBackdrop3.sample(gLinear, _22265, level(0.0)) * _17591.x)) * _17588.y) + (((gBackdrop3.sample(gLinear, _22266, level(0.0)) * _17588.x) + (gBackdrop3.sample(gLinear, _22267, level(0.0)) * _17591.x)) * _17591.y);
                                }
                                else
                                {
                                    float4 _22281;
                                    if (_15882 == 4)
                                    {
                                        float2 _17749 = (_6633 * _172.gLevel[4].xy) - float2(0.5);
                                        float2 _17751 = floor(_17749);
                                        float2 _17754 = _17749 - _17751;
                                        float2 _17757 = _17754 * _17754;
                                        float2 _17760 = _17757 * _17754;
                                        float2 _17779 = (((_17760 * 3.0) - (_17757 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _17792 = _17760 * 0.16666667163372039794921875;
                                        float2 _17795 = (((((-_17760) + (_17757 * 3.0)) - (_17754 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17779;
                                        float2 _17798 = (((((_17760 * (-3.0)) + (_17757 * 3.0)) + (_17754 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17792;
                                        float2 _17808 = ((_17751 - float2(0.5)) + (_17779 / _17795)) * _172.gLevel[4].zw;
                                        float2 _17818 = ((_17751 + float2(1.5)) + (_17792 / _17798)) * _172.gLevel[4].zw;
                                        float2 _22260;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21734 = _17808;
                                            _21734.y = 1.0 - _17808.y;
                                            _22260 = _21734;
                                        }
                                        else
                                        {
                                            _22260 = _17808;
                                        }
                                        float _17838 = _17808.y;
                                        float2 _17839 = float2(_17818.x, _17838);
                                        float2 _22261;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21740 = _17839;
                                            _21740.y = 1.0 - _17838;
                                            _22261 = _21740;
                                        }
                                        else
                                        {
                                            _22261 = _17839;
                                        }
                                        float2 _17856 = float2(_17808.x, _17818.y);
                                        float2 _22262;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21747 = _17856;
                                            _21747.y = 1.0 - _17818.y;
                                            _22262 = _21747;
                                        }
                                        else
                                        {
                                            _22262 = _17856;
                                        }
                                        float2 _22263;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21753 = _17818;
                                            _21753.y = 1.0 - _17818.y;
                                            _22263 = _21753;
                                        }
                                        else
                                        {
                                            _22263 = _17818;
                                        }
                                        _22281 = (((gBackdrop4.sample(gLinear, _22260, level(0.0)) * _17795.x) + (gBackdrop4.sample(gLinear, _22261, level(0.0)) * _17798.x)) * _17795.y) + (((gBackdrop4.sample(gLinear, _22262, level(0.0)) * _17795.x) + (gBackdrop4.sample(gLinear, _22263, level(0.0)) * _17798.x)) * _17798.y);
                                    }
                                    else
                                    {
                                        float2 _17956 = (_6633 * _172.gLevel[5].xy) - float2(0.5);
                                        float2 _17958 = floor(_17956);
                                        float2 _17961 = _17956 - _17958;
                                        float2 _17964 = _17961 * _17961;
                                        float2 _17967 = _17964 * _17961;
                                        float2 _17986 = (((_17967 * 3.0) - (_17964 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _17999 = _17967 * 0.16666667163372039794921875;
                                        float2 _18002 = (((((-_17967) + (_17964 * 3.0)) - (_17961 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17986;
                                        float2 _18005 = (((((_17967 * (-3.0)) + (_17964 * 3.0)) + (_17961 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17999;
                                        float2 _18015 = ((_17958 - float2(0.5)) + (_17986 / _18002)) * _172.gLevel[5].zw;
                                        float2 _18025 = ((_17958 + float2(1.5)) + (_17999 / _18005)) * _172.gLevel[5].zw;
                                        float2 _22256;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21760 = _18015;
                                            _21760.y = 1.0 - _18015.y;
                                            _22256 = _21760;
                                        }
                                        else
                                        {
                                            _22256 = _18015;
                                        }
                                        float _18045 = _18015.y;
                                        float2 _18046 = float2(_18025.x, _18045);
                                        float2 _22257;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21766 = _18046;
                                            _21766.y = 1.0 - _18045;
                                            _22257 = _21766;
                                        }
                                        else
                                        {
                                            _22257 = _18046;
                                        }
                                        float2 _18063 = float2(_18015.x, _18025.y);
                                        float2 _22258;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21773 = _18063;
                                            _21773.y = 1.0 - _18025.y;
                                            _22258 = _21773;
                                        }
                                        else
                                        {
                                            _22258 = _18063;
                                        }
                                        float2 _22259;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21779 = _18025;
                                            _21779.y = 1.0 - _18025.y;
                                            _22259 = _21779;
                                        }
                                        else
                                        {
                                            _22259 = _18025;
                                        }
                                        _22281 = (((gBackdrop5.sample(gLinear, _22256, level(0.0)) * _18002.x) + (gBackdrop5.sample(gLinear, _22257, level(0.0)) * _18005.x)) * _18002.y) + (((gBackdrop5.sample(gLinear, _22258, level(0.0)) * _18002.x) + (gBackdrop5.sample(gLinear, _22259, level(0.0)) * _18005.x)) * _18005.y);
                                    }
                                    _22280 = _22281;
                                }
                                _22279 = _22280;
                            }
                            _22278 = _22279;
                        }
                        _22277 = _22278;
                    }
                    _22282 = mix(_22251.xyz, _22277.xyz, float3(_15869));
                }
                else
                {
                    _22282 = _22251.xyz;
                }
                _22837 = _22282;
            }
            float3 _23328;
            if (_6505 > 0.0)
            {
                float2 _6654 = _6563 + (((_22228 * fast::min(_6496 * 0.5, 16.0)) * _172.gDisplay.zw) * _172.gTarget.zw);
                float _18153 = fast::clamp(log2(fast::max(16.0 * _172.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _18156 = int(floor(_18153));
                float _18160 = _18153 - float(_18156);
                float2 _22815;
                if (_172.gConv.x > 0.5)
                {
                    float2 _21784 = _6654;
                    _21784.y = 1.0 - _6654.y;
                    _22815 = _21784;
                }
                else
                {
                    _22815 = _6654;
                }
                float4 _22816;
                if (_18156 <= 0)
                {
                    _22816 = gBackdrop0.sample(gLinear, _22815, level(0.0));
                }
                else
                {
                    float4 _22817;
                    if (_18156 == 1)
                    {
                        _22817 = gBackdrop1.sample(gLinear, _22815, level(0.0));
                    }
                    else
                    {
                        float4 _22818;
                        if (_18156 == 2)
                        {
                            _22818 = gBackdrop2.sample(gLinear, _22815, level(0.0));
                        }
                        else
                        {
                            float4 _22819;
                            if (_18156 == 3)
                            {
                                _22819 = gBackdrop3.sample(gLinear, _22815, level(0.0));
                            }
                            else
                            {
                                float4 _22820;
                                if (_18156 == 4)
                                {
                                    _22820 = gBackdrop4.sample(gLinear, _22815, level(0.0));
                                }
                                else
                                {
                                    _22820 = gBackdrop5.sample(gLinear, _22815, level(0.0));
                                }
                                _22819 = _22820;
                            }
                            _22818 = _22819;
                        }
                        _22817 = _22818;
                    }
                    _22816 = _22817;
                }
                float3 _22827;
                if ((_18160 > 0.0199999995529651641845703125) && (_18156 < 5))
                {
                    int _18173 = _18156 + 1;
                    float2 _22821;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _21787 = _6654;
                        _21787.y = 1.0 - _6654.y;
                        _22821 = _21787;
                    }
                    else
                    {
                        _22821 = _6654;
                    }
                    float4 _22822;
                    if (_18173 <= 0)
                    {
                        _22822 = gBackdrop0.sample(gLinear, _22821, level(0.0));
                    }
                    else
                    {
                        float4 _22823;
                        if (_18173 == 1)
                        {
                            _22823 = gBackdrop1.sample(gLinear, _22821, level(0.0));
                        }
                        else
                        {
                            float4 _22824;
                            if (_18173 == 2)
                            {
                                _22824 = gBackdrop2.sample(gLinear, _22821, level(0.0));
                            }
                            else
                            {
                                float4 _22825;
                                if (_18173 == 3)
                                {
                                    _22825 = gBackdrop3.sample(gLinear, _22821, level(0.0));
                                }
                                else
                                {
                                    float4 _22826;
                                    if (_18173 == 4)
                                    {
                                        _22826 = gBackdrop4.sample(gLinear, _22821, level(0.0));
                                    }
                                    else
                                    {
                                        _22826 = gBackdrop5.sample(gLinear, _22821, level(0.0));
                                    }
                                    _22825 = _22826;
                                }
                                _22824 = _22825;
                            }
                            _22823 = _22824;
                        }
                        _22822 = _22823;
                    }
                    _22827 = mix(_22816.xyz, _22822.xyz, float3(_18160));
                }
                else
                {
                    _22827 = _22816.xyz;
                }
                _23328 = _22827;
            }
            else
            {
                _23328 = float3(0.5);
            }
            float _22858;
            if (in.i_shape.x > 0.001000000047497451305389404296875)
            {
                int _18340 = clamp(int(rint(log2(36.0 * _172.gDisplay.z) - 1.0)), 1, 4);
                float2 _22828;
                if (_172.gConv.x > 0.5)
                {
                    float2 _21791 = _6563;
                    _21791.y = 1.0 - _6563.y;
                    _22828 = _21791;
                }
                else
                {
                    _22828 = _6563;
                }
                float4 _22829;
                if (_18340 <= 0)
                {
                    _22829 = gBackdrop0.sample(gLinear, _22828, level(0.0));
                }
                else
                {
                    float4 _22830;
                    if (_18340 == 1)
                    {
                        _22830 = gBackdrop1.sample(gLinear, _22828, level(0.0));
                    }
                    else
                    {
                        float4 _22831;
                        if (_18340 == 2)
                        {
                            _22831 = gBackdrop2.sample(gLinear, _22828, level(0.0));
                        }
                        else
                        {
                            float4 _22832;
                            if (_18340 == 3)
                            {
                                _22832 = gBackdrop3.sample(gLinear, _22828, level(0.0));
                            }
                            else
                            {
                                float4 _22833;
                                if (_18340 == 4)
                                {
                                    _22833 = gBackdrop4.sample(gLinear, _22828, level(0.0));
                                }
                                else
                                {
                                    _22833 = gBackdrop5.sample(gLinear, _22828, level(0.0));
                                }
                                _22832 = _22833;
                            }
                            _22831 = _22832;
                        }
                        _22830 = _22831;
                    }
                    _22829 = _22830;
                }
                _22858 = dot(_22829.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _22858 = 0.5;
            }
            _23326 = _23328;
            _22857 = _22858;
            _22834 = _22837;
        }
        else
        {
            _23326 = float3(0.5);
            _22857 = 0.5;
            _22834 = float3(0.5);
        }
        float3 _6682 = mix(float3(dot(_22834, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _22834, float3(gFxData_1._data[_6407].x)) + float3(gFxData_1._data[_6407].y);
        float3 _23086;
        if (in.i_shape.x > 0.001000000047497451305389404296875)
        {
            float _6692 = fast::clamp(fast::max(_22857 + gFxData_1._data[_6407].y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _6700 = mix(_6692, dot(gFxData_1._data[_6399].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), in.i_shape.x);
            _23086 = select(mix(_6682, float3(1.0), float3((_6700 - _6692) / fast::max(1.0 - _6692, 0.001000000047497451305389404296875))), _6682 * (_6700 / _6692), bool3(_6700 < _6692));
        }
        else
        {
            _23086 = _6682;
        }
        float2 _6738 = fast::clamp((in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875)), float2(0.0), float2(1.0));
        float _6770 = pow(1.0 - _23090.z, 5.0);
        float2 _6786 = float2(cos(gFxData_1._data[_6407].w), sin(gFxData_1._data[_6407].w));
        float _6789 = dot(_22228, _6786);
        float _6824 = fast::clamp(0.5 + (0.5 * dot((in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5)) / _6488, _6786)), 0.0, 1.0);
        _25141 = ((_6770 * (pow(fast::clamp(_6789, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(fast::clamp(-_6789, 0.0, 1.0), 1.5)))) * gFxData_1._data[_6407].z) * 1.60000002384185791015625;
        _24496 = gFxData_1._data[_6415];
        _24483 = float4((mix(mix(_23086, gFxData_1._data[_6399].xyz, float3(fast::clamp(gFxData_1._data[_6399].w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _6511))) + (0.3499999940395355224609375 * ((dot(gFxData_1._data[_6399].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _6738.y) : _6738.y))), 0.0, 1.0))), (_23326 * 1.10000002384185791015625) + float3(0.07999999821186065673828125), float3(_6770 * 0.3499999940395355224609375)) + float3((((0.039999999105930328369140625 * _6824) * _6824) + (0.0500000007450580596923828125 * (1.0 - _6511))) * gFxData_1._data[_6407].z)) * _3871, _3871) + (_23575 * (1.0 - _3871));
    }
    else
    {
        _25141 = 0.0;
        _24496 = _22021;
        _24483 = _23575;
    }
    float4 _24493;
    if ((in.i_flags.x & 1u) != 0u)
    {
        float4 _23850;
        float4 _24160;
        if (in.i_flags.y != 0u)
        {
            _24160 = gFxData_1._data[(in.i_instance * 24u) + 3u];
            _23850 = gFxData_1._data[(in.i_instance * 24u) + 4u];
        }
        else
        {
            _24160 = float4(0.0);
            _23850 = float4(0.0);
        }
        float4 _24478;
        do
        {
            if (in.i_flags.y == 0u)
            {
                _24478 = in.i_fill0;
                break;
            }
            float2 _18494 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float2 _18504 = fwidth(in.i_local);
            float _18506 = fast::max(length(_18504), 9.9999997473787516355514526367188e-05);
            float _24470;
            float _24474;
            if ((in.i_flags.y == 1u) || (in.i_flags.y == 4u))
            {
                float _18515 = cos(_23850.x);
                float _18518 = sin(_23850.x);
                float _18544 = ((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_18515, _18518)) / fast::max(0.5 * ((abs(_18515) * _18494.x) + (abs(_18518) * _18494.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (in.i_flags.y == 4u)
                {
                    float _18559 = fast::clamp((_18544 - _23850.y) / fast::max(_23850.z - _23850.y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _24478 = float4(fast::clamp(abs((fract(float3(_18559 * 0.800000011920928955078125) + float3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - float3(3.0)) - float3(1.0), float3(0.0), float3(1.0)), in.i_fill0.w * pow(fast::max(sin(_18559 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _24474 = -1.0;
                _24470 = _18544;
            }
            else
            {
                float _24471;
                float _24475;
                if (in.i_flags.y == 2u)
                {
                    _24475 = -1.0;
                    _24471 = length(in.i_local - (in.i_rect.xy + (_23850.xy * _18494))) / fast::max(_23850.z * fast::max(_18494.x, _18494.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    float2 _18627 = in.i_local - (in.i_rect.xy + (_23850.xy * _18494));
                    float _18638 = fract(((precise::atan2(_18627.y, _18627.x) - _23850.z) * 0.15915493667125701904296875) + 1.0);
                    float _24472;
                    float _24476;
                    if (_23850.w > 0.5)
                    {
                        _24476 = -1.0;
                        _24472 = 0.5 - (0.5 * cos(_18638 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _18658 = (((_18638 < 0.5) ? _18638 : (_18638 - 1.0)) * 6.283185482025146484375) * length(_18627);
                        float _24477;
                        if (abs(_18658) < _18506)
                        {
                            _24477 = fast::clamp(((_18658 / _18506) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _24477 = -1.0;
                        }
                        _24476 = _24477;
                        _24472 = _18638;
                    }
                    _24475 = _24476;
                    _24471 = _24472;
                }
                _24474 = _24475;
                _24470 = _24471;
            }
            float4 _18687 = float4(in.i_fill0.xyz * in.i_fill0.w, in.i_fill0.w);
            float4 _18699 = float4(_24160.xyz * _24160.w, _24160.w);
            float4 _18713 = select(mix(_18687, _18699, float4(fast::clamp(_24470, 0.0, 1.0))), mix(_18699, _18687, float4(_24474)), bool4(_24474 >= 0.0));
            _24478 = select(float4(0.0), float4(_18713.xyz / float3(_18713.w), _18713.w), bool4(_18713.w > 9.9999997473787516355514526367188e-06));
            break;
        } while(false);
        float4 _24479;
        if ((in.i_flags.x & 64u) != 0u)
        {
            uint _18738 = (in.i_instance * 24u) + 18u;
            _24479 = _24478 * gTex.sample(gLinear, mix(gFxData_1._data[_18738].xy, gFxData_1._data[_18738].zw, (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875))));
        }
        else
        {
            _24479 = _24478;
        }
        float _18748 = fast::clamp(_24479.w * _3871, 0.0, 1.0);
        _24493 = float4(_24479.xyz * _18748, _18748) + (_24483 * (1.0 - _18748));
    }
    else
    {
        _24493 = _24483;
    }
    float4 _25127;
    if ((in.i_flags.x & 16384u) != 0u)
    {
        uint _18772 = (in.i_instance * 24u) + 21u;
        uint _18780 = (in.i_instance * 24u) + 3u;
        float2 _4310 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float2 _4317 = (in.i_local - in.i_rect.xy) / _4310;
        float _4328 = ((gFxData_1._data[_18780].x >= 0.0) ? gFxData_1._data[_18780].x : _172.gTime.x) * gFxData_1._data[_18772].z;
        float _4331 = _4317.x * 2.0;
        float _4332 = _4331 - 1.0;
        float _4337 = _4317.y * _4310.y;
        float _4340 = fast::max(gFxData_1._data[_18772].w, 0.001000000047497451305389404296875);
        float _4345 = fast::clamp(1.0 - (_4332 * _4332), 0.0, 1.0);
        float _4347 = pow(_4345, 1.2999999523162841796875);
        float _4349 = pow(_4345, 0.699999988079071044921875);
        float _4352 = fast::clamp(_4328 * 1.4285714626312255859375, 0.0, 1.0);
        float _4368 = (0.25 + (0.75 * ((_4352 * _4352) * (3.0 - (2.0 * _4352))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_4328 * 2.099999904632568359375)));
        float _4377 = ((gFxData_1._data[_18772].y * _4310.y) * _4368) * _4347;
        float _4403 = (_4310.y * (0.5 + ((gFxData_1._data[_18772].x * (0.5 - (_4332 * _4332))) * 0.5))) + (((0.14000000059604644775390625 * _4310.y) * _4347) * sin(((_4332 * 2.400000095367431640625) - (_4328 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _24489;
        float _24490;
        float3 _24491;
        _24491 = float3(0.0);
        _24490 = _4403;
        _24489 = _4403;
        float3 _4475;
        float _25521;
        float _25522;
        for (int _24488 = 0; _24488 < 4; _24491 = _4475, _24490 = _25522, _24489 = _25521, _24488++)
        {
            float _4427 = _4403 + ((_4377 * _2880[_24488].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_4328 * 1.7000000476837158203125) + _2897[_24488].y))));
            _25521 = (_24488 == 0) ? _4427 : _24489;
            _25522 = (_24488 == 2) ? _4427 : _24490;
            float _4442 = _4340 * _2880[_24488].y;
            float _4447 = (_4337 - _4427) / _4442;
            float _4452 = _4340 * _2880[_24488].z;
            float3 _25495;
            _25495 = float3(0.0);
            for (int _25494 = 0; _25494 < 6; )
            {
                float _18805 = ((_4337 - _4427) - (_4452 * ((float(_25494) * 0.4000000059604644775390625) - 1.0))) / _4442;
                _25495 += (_1662[_25494] * exp((-_18805) * _18805));
                _25494++;
                continue;
            }
            _4475 = _24491 + (mix(_25495 * float3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), float3(exp((-_4447) * _4447)), float3(_2897[_24488].x)) * (_2880[_24488].w * _4349));
        }
        float _4481 = _4340 * 1.5;
        float _4511 = fast::clamp((_4337 - _24489) / fast::max(_24490 - _24489, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _4539 = (_4337 - (_24490 - (_4340 * 3.0))) / (((_4310.y * 0.0900000035762786865234375) + (_4377 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _4542 = (_4331 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _4567 = ((_4337 - _24489) + (_4340 * 5.0)) / (_4340 * 7.0);
        float3 _4593 = float3(1.0) - exp((-((((_24491 + (mix(float3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), float3(1.0), float3(_4511)) * ((((1.0 / (1.0 + exp((-((_4337 - _24489) - (_4340 * 2.0))) / _4481))) / (1.0 + exp((-(_24490 - _4337)) / _4481))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_4511, 2.5)))) * _4349))) + (float3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_4539) * _4539) - (_4542 * _4542)) * (0.5 + (1.10000002384185791015625 * _4368))))) + (float3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_4567) * _4567) * _4347) * 0.100000001490116119384765625))) * mix(float3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), float3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), float3(0.5 + (0.5 * sin(_4328 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _4603 = (in.i_fill0.w * smoothstep(0.0, 0.119999997317790985107421875, _4317.y)) * smoothstep(1.0, 0.87999999523162841796875, _4317.y);
        float _4622 = (fast::clamp(fast::max(_4593.x, fast::max(_4593.y, _4593.z)), 0.0, 1.0) * _4603) * _3871;
        _25127 = float4((_4593 * _4603) * _3871, _4622) + (_24493 * (1.0 - _4622));
    }
    else
    {
        _25127 = _24493;
    }
    float4 _25136;
    if (((in.i_flags.x & 4u) != 0u) && ((in.i_flags.x & 256u) != 0u))
    {
        uint _18837 = (in.i_instance * 24u) + 7u;
        uint _18845 = (in.i_instance * 24u) + 8u;
        float2 _4656 = in.i_local - gFxData_1._data[_18845].zw;
        float2 _18886 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _18895 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _25102;
        if (in.i_flags.z == 1u)
        {
            float2 _18901 = _4656 - _18886;
            float _25101;
            do
            {
                if (in.i_shape.w >= 6.282185077667236328125)
                {
                    _25101 = abs(length(_18901) - in.i_radii.x) - in.i_radii.y;
                    break;
                }
                float _19000 = in.i_shape.z + (in.i_shape.w * 0.5);
                float _19002 = cos(_19000);
                float _19004 = sin(_19000);
                float _19013 = dot(_18901, float2(-_19004, _19002));
                float _19016 = dot(_18901, float2(_19002, _19004));
                float2 _19017 = float2(_19013, _19016);
                float _19020 = abs(_19013);
                _19017.x = _19020;
                float _19023 = in.i_shape.w * 0.5;
                float _19025 = sin(_19023);
                float _19027 = cos(_19023);
                _25101 = (((_19027 * _19020) > (_19025 * _19016)) ? length(_19017 - (float2(_19025, _19027) * in.i_radii.x)) : abs(length(_19017) - in.i_radii.x)) - in.i_radii.y;
                break;
            } while(false);
            _25102 = _25101;
        }
        else
        {
            float _25103;
            if (in.i_flags.z == 2u)
            {
                float2 _19063 = _4656 - _22023.xy;
                float2 _19066 = _22023.zw - _22023.xy;
                _25103 = length(_19063 - (_19066 * fast::clamp(dot(_19063, _19066) / fast::max(dot(_19066, _19066), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
            }
            else
            {
                float2 _18928 = _4656 - _18886;
                float _19119 = fast::min(_18895.x, _18895.y);
                float _19122 = fast::min((_18928.x > 0.0) ? ((_18928.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_18928.y > 0.0) ? in.i_radii.w : in.i_radii.x), _19119);
                float _19128 = _19122 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                float _25086;
                float _25087;
                if (_19128 > _19119)
                {
                    float _19142 = in.i_shape.y * fast::clamp((_19119 - _19122) / fast::max(0.60000002384185791015625 * _19122, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _25087 = _19142;
                    _25086 = _19122 * (1.0 + (0.60000002384185791015625 * _19142));
                }
                else
                {
                    _25087 = in.i_shape.y;
                    _25086 = _19128;
                }
                float2 _19155 = (abs(_18928) - _18895) + float2(_25086);
                float2 _19157 = fast::max(_19155, float2(0.0));
                float _25088;
                if ((_19157.x > 0.0) && (_19157.y > 0.0))
                {
                    float _25089;
                    if ((_25087 > 0.001000000047497451305389404296875) && (_25086 > 9.9999997473787516355514526367188e-05))
                    {
                        float _19174 = 2.0 + (2.0 * _25087);
                        float2 _19179 = _19157 / float2(fast::max(_25086, 9.9999997473787516355514526367188e-05));
                        _25089 = pow(pow(_19179.x, _19174) + pow(_19179.y, _19174), 1.0 / _19174) * _25086;
                    }
                    else
                    {
                        _25089 = length(_19157);
                    }
                    _25088 = _25089;
                }
                else
                {
                    _25088 = fast::max(_19157.x, _19157.y);
                }
                float _19214 = (fast::min(fast::max(_19155.x, _19155.y), 0.0) + _25088) - _25086;
                float _25104;
                if ((in.i_flags.x & 512u) != 0u)
                {
                    float2 _18956 = fast::max((_22023.zw - _22023.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _18959 = _4656 - ((_22023.xy + _22023.zw) * 0.5);
                    float _19250 = fast::min(_18956.x, _18956.y);
                    float _19253 = fast::min((_18959.x > 0.0) ? ((_18959.y > 0.0) ? _24496.x : _24496.x) : ((_18959.y > 0.0) ? _24496.x : _24496.x), _19250);
                    float _19259 = _19253 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _25092;
                    float _25093;
                    if (_19259 > _19250)
                    {
                        float _19273 = in.i_shape.y * fast::clamp((_19250 - _19253) / fast::max(0.60000002384185791015625 * _19253, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _25093 = _19273;
                        _25092 = _19253 * (1.0 + (0.60000002384185791015625 * _19273));
                    }
                    else
                    {
                        _25093 = in.i_shape.y;
                        _25092 = _19259;
                    }
                    float2 _19286 = (abs(_18959) - _18956) + float2(_25092);
                    float2 _19288 = fast::max(_19286, float2(0.0));
                    float _25094;
                    if ((_19288.x > 0.0) && (_19288.y > 0.0))
                    {
                        float _25095;
                        if ((_25093 > 0.001000000047497451305389404296875) && (_25092 > 9.9999997473787516355514526367188e-05))
                        {
                            float _19305 = 2.0 + (2.0 * _25093);
                            float2 _19310 = _19288 / float2(fast::max(_25092, 9.9999997473787516355514526367188e-05));
                            _25095 = pow(pow(_19310.x, _19305) + pow(_19310.y, _19305), 1.0 / _19305) * _25092;
                        }
                        else
                        {
                            _25095 = length(_19288);
                        }
                        _25094 = _25095;
                    }
                    else
                    {
                        _25094 = fast::max(_19288.x, _19288.y);
                    }
                    float _19345 = (fast::min(fast::max(_19286.x, _19286.y), 0.0) + _25094) - _25092;
                    float _19350 = fast::max(_24496.y, 9.9999997473787516355514526367188e-05);
                    float _19359 = fast::max(_19350 - abs(_19214 - _19345), 0.0) / _19350;
                    _25104 = fast::min(_19214, _19345) - (((_19359 * _19359) * _19350) * 0.25);
                }
                else
                {
                    _25104 = _19214;
                }
                _25103 = _25104;
            }
            _25102 = _25103;
        }
        float _4665 = (_25102 + gFxData_1._data[_18845].y) / (fast::max(gFxData_1._data[_18845].x * 0.5, _3846 * 0.5) * 1.41421353816986083984375);
        float _19376 = sign(_4665);
        float _19378 = abs(_4665);
        float _19389 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_19378 * _19378))) * _19378)) * _19378);
        float _19392 = _19389 * _19389;
        float _19407 = fast::clamp(gFxData_1._data[_18837].w * ((0.5 + (0.5 * (_19376 - (_19376 / (_19392 * _19392))))) * _3871), 0.0, 1.0);
        _25136 = float4(gFxData_1._data[_18837].xyz * _19407, _19407) + (_25127 * (1.0 - _19407));
    }
    else
    {
        _25136 = _25127;
    }
    float4 _25160;
    if ((in.i_flags.x & 16u) != 0u)
    {
        uint _19431 = (in.i_instance * 24u) + 9u;
        uint _19439 = (in.i_instance * 24u) + 10u;
        float _4697 = fast::max(-_22041, 0.0) / fast::max(gFxData_1._data[_19439].z, 0.001000000047497451305389404296875);
        float _19449 = fast::clamp(gFxData_1._data[_19431].w * fast::clamp((exp(((-_4697) * _4697) * 2.2000000476837158203125) * gFxData_1._data[_19439].w) * _3871, 0.0, 1.0), 0.0, 1.0);
        _25160 = float4(gFxData_1._data[_19431].xyz * _19449, _19449) + (_25136 * (1.0 - _19449));
    }
    else
    {
        _25160 = _25136;
    }
    float3 _4726 = _25160.xyz + float3((_25141 * _3871) * _25160.w);
    float4 _21917 = _25160;
    _21917.x = _4726.x;
    _21917.y = _4726.y;
    _21917.z = _4726.z;
    float4 _25163;
    if ((in.i_flags.x & 2u) != 0u)
    {
        uint _19473 = (in.i_instance * 24u) + 5u;
        float4 _19475 = gFxData_1._data[_19473];
        uint _19481 = (in.i_instance * 24u) + 6u;
        float4 _25161;
        if (gFxData_1._data[_19481].z < 0.999000012874603271484375)
        {
            float2 _4784 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float _4795 = cos(gFxData_1._data[_19481].w);
            float _4798 = sin(gFxData_1._data[_19481].w);
            float4 _21934 = _19475;
            _21934.w = _19475.w * mix(1.0, gFxData_1._data[_19481].z, fast::clamp(((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_4795, _4798)) / fast::max(0.5 * ((abs(_4795) * _4784.x) + (abs(_4798) * _4784.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _25161 = _21934;
        }
        else
        {
            _25161 = _19475;
        }
        float _19491 = fast::clamp(_25161.w * (fast::clamp(0.5 - ((_22041 - (gFxData_1._data[_19481].x * gFxData_1._data[_19481].y)) / _3846), 0.0, 1.0) - fast::clamp(0.5 - ((_22041 + (gFxData_1._data[_19481].x * (1.0 - gFxData_1._data[_19481].y))) / _3846), 0.0, 1.0)), 0.0, 1.0);
        _25163 = float4(_25161.xyz * _19491, _19491) + (_21917 * (1.0 - _19491));
    }
    else
    {
        _25163 = _21917;
    }
    float4 _25164;
    if ((in.i_flags.x & 128u) != 0u)
    {
        float2 _4859 = (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float _4881 = exp(-pow((((_4859.x * 0.85000002384185791015625) + (_4859.y * 0.1500000059604644775390625)) - ((fract(_172.gTime.x * in.i_misc.w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _25164 = float4(_25163.xyz + float3(((_4881 * in.i_misc.z) * _3871) * fast::max(_25163.w, 0.3499999940395355224609375)), fast::max(_25163.w, ((_4881 * in.i_misc.z) * _3871) * 0.5));
    }
    else
    {
        _25164 = _25163;
    }
    float4 _25489;
    if ((in.i_flags.x & 2048u) != 0u)
    {
        float3 _19516 = fract(floor(_22020).xyx * 0.103100001811981201171875);
        float3 _19525 = _19516 + float3(dot(_19516, _19516.yzx + float3(33.3300018310546875)));
        float3 _4932 = _25164.xyz + float3(((fract((_19525.x + _19525.y) * _19525.z) - 0.5) * in.i_misc.y) * _25164.w);
        float4 _21958 = _25164;
        _21958.x = _4932.x;
        _21958.y = _4932.y;
        _21958.z = _4932.z;
        _25489 = _21958;
    }
    else
    {
        _25489 = _25164;
    }
    float _25485;
    if (_215.gFade.z > 0.0)
    {
        _25485 = smoothstep(0.0, 1.0, fast::clamp((_22020.y - _215.gFade.x) / _215.gFade.z, 0.0, 1.0));
    }
    else
    {
        _25485 = 1.0;
    }
    float _25486;
    if (_215.gFade.w > 0.0)
    {
        _25486 = _25485 * smoothstep(0.0, 1.0, fast::clamp((_215.gFade.y - _22020.y) / _215.gFade.w, 0.0, 1.0));
    }
    else
    {
        _25486 = _25485;
    }
    float4 _4949 = _25489 * ((in.i_misc.x * _25175) * _25486);
    float4 _25490;
    if (((in.i_flags.x & 8u) != 0u) || ((in.i_flags.x & 4u) != 0u))
    {
        float3 _19577 = fract((floor(_22020) + float2(17.0)).xyx * 0.103100001811981201171875);
        float3 _19586 = _19577 + float3(dot(_19577, _19577.yzx + float3(33.3300018310546875)));
        float3 _4973 = _4949.xyz + float3(((fract((_19586.x + _19586.y) * _19586.z) - 0.5) * 0.0039215688593685626983642578125) * fast::clamp(_4949.w * 8.0, 0.0, 1.0));
        float4 _21970 = _4949;
        _21970.x = _4973.x;
        _21970.y = _4973.y;
        _21970.z = _4973.z;
        _25490 = _21970;
    }
    else
    {
        _25490 = _4949;
    }
    float3 _4983 = fast::max(_25490.xyz, float3(0.0));
    float4 _21976 = _25490;
    _21976.x = _4983.x;
    _21976.y = _4983.y;
    _21976.z = _4983.z;
    float4 _25491;
    if ((_172.gTime.w > 0.5) && (_25490.w > 9.9999997473787516355514526367188e-06))
    {
        float3 _19630 = fast::clamp(_21976.xyz / float3(_25490.w), float3(0.0), float3(1.0));
        float3 _19616 = select(pow((_19630 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _19630 * float3(0.077399380505084991455078125), _19630 <= float3(0.040449999272823333740234375)) * _25490.w;
        float4 _21985 = _21976;
        _21985.x = _19616.x;
        _21985.y = _19616.y;
        _21985.z = _19616.z;
        _25491 = _21985;
    }
    else
    {
        _25491 = _21976;
    }
    out._entryPointOutput = _25491;
    return out;
}

