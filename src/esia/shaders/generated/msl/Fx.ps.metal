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
constant spvUnsafeArray<float4, 4> _3087 = spvUnsafeArray<float4, 4>({ float4(-1.0, 1.0, 3.400000095367431640625, 2.599999904632568359375), float4(-0.550000011920928955078125, 0.800000011920928955078125, 2.0, 0.800000011920928955078125), float4(0.300000011920928955078125, 1.0, 1.2000000476837158203125, 1.2999999523162841796875), float4(0.62000000476837158203125, 0.800000011920928955078125, 1.60000002384185791015625, 0.449999988079071044921875) });
constant spvUnsafeArray<float2, 4> _3104 = spvUnsafeArray<float2, 4>({ float2(0.0), float2(0.100000001490116119384765625, 1.2999999523162841796875), float2(0.550000011920928955078125, 3.900000095367431640625), float2(0.20000000298023223876953125, 5.19999980926513671875) });

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
    float2 _5257 = gl_FragCoord.xy + _184.gConv.yy;
    float2 _15887;
    if (_184.gConv.x > 0.5)
    {
        float2 _14835 = _5257;
        _14835.y = _184.gTarget.y - _5257.y;
        _15887 = _14835;
    }
    else
    {
        _15887 = _5257;
    }
    float _4049 = dfdx(in.i_local.x);
    float _4053 = dfdy(in.i_local.x);
    float _4056 = fast::max(abs(_4049) + abs(_4053), 9.9999997473787516355514526367188e-05);
    float4 _15888;
    float4 _15890;
    if ((in.i_flags.z == 2u) || ((in.i_flags.x & 512u) != 0u))
    {
        _15890 = gFxData_1._data[(in.i_instance * 24u) + 15u];
        _15888 = gFxData_1._data[(in.i_instance * 24u) + 16u];
    }
    else
    {
        _15890 = float4(0.0);
        _15888 = float4(0.0);
    }
    float2 _5324 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
    float2 _5333 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
    float _15908;
    if (in.i_flags.z == 1u)
    {
        float2 _5339 = in.i_local - _5324;
        float _15907;
        do
        {
            if (in.i_shape.w >= 6.282185077667236328125)
            {
                _15907 = abs(length(_5339) - in.i_radii.x) - in.i_radii.y;
                break;
            }
            float _5438 = in.i_shape.z + (in.i_shape.w * 0.5);
            float _5440 = cos(_5438);
            float _5442 = sin(_5438);
            float _5451 = dot(_5339, float2(-_5442, _5440));
            float _5454 = dot(_5339, float2(_5440, _5442));
            float2 _5455 = float2(_5451, _5454);
            float _5458 = abs(_5451);
            _5455.x = _5458;
            float _5461 = in.i_shape.w * 0.5;
            float _5463 = sin(_5461);
            float _5465 = cos(_5461);
            _15907 = (((_5465 * _5458) > (_5463 * _5454)) ? length(_5455 - (float2(_5463, _5465) * in.i_radii.x)) : abs(length(_5455) - in.i_radii.x)) - in.i_radii.y;
            break;
        } while(false);
        _15908 = _15907;
    }
    else
    {
        float _15909;
        if (in.i_flags.z == 2u)
        {
            float2 _5501 = in.i_local - _15890.xy;
            float2 _5504 = _15890.zw - _15890.xy;
            _15909 = length(_5501 - (_5504 * fast::clamp(dot(_5501, _5504) / fast::max(dot(_5504, _5504), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
        }
        else
        {
            float2 _5366 = in.i_local - _5324;
            float _5557 = fast::min(_5333.x, _5333.y);
            float _5560 = fast::min((_5366.x > 0.0) ? ((_5366.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_5366.y > 0.0) ? in.i_radii.w : in.i_radii.x), _5557);
            float _5566 = _5560 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
            float _15892;
            float _15893;
            if (_5566 > _5557)
            {
                float _5580 = in.i_shape.y * fast::clamp((_5557 - _5560) / fast::max(0.60000002384185791015625 * _5560, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                _15893 = _5580;
                _15892 = _5560 * (1.0 + (0.60000002384185791015625 * _5580));
            }
            else
            {
                _15893 = in.i_shape.y;
                _15892 = _5566;
            }
            float2 _5593 = (abs(_5366) - _5333) + float2(_15892);
            float2 _5595 = fast::max(_5593, float2(0.0));
            float _15894;
            if ((_5595.x > 0.0) && (_5595.y > 0.0))
            {
                float _15895;
                if ((_15893 > 0.001000000047497451305389404296875) && (_15892 > 9.9999997473787516355514526367188e-05))
                {
                    float _5612 = 2.0 + (2.0 * _15893);
                    float2 _5617 = _5595 / float2(fast::max(_15892, 9.9999997473787516355514526367188e-05));
                    _15895 = pow(pow(_5617.x, _5612) + pow(_5617.y, _5612), 1.0 / _5612) * _15892;
                }
                else
                {
                    _15895 = length(_5595);
                }
                _15894 = _15895;
            }
            else
            {
                _15894 = fast::max(_5595.x, _5595.y);
            }
            float _5652 = (fast::min(fast::max(_5593.x, _5593.y), 0.0) + _15894) - _15892;
            float _15910;
            if ((in.i_flags.x & 512u) != 0u)
            {
                float2 _5394 = fast::max((_15890.zw - _15890.xy) * 0.5, float2(0.001000000047497451305389404296875));
                float2 _5397 = in.i_local - ((_15890.xy + _15890.zw) * 0.5);
                float _5688 = fast::min(_5394.x, _5394.y);
                float _5691 = fast::min((_5397.x > 0.0) ? ((_5397.y > 0.0) ? _15888.x : _15888.x) : ((_5397.y > 0.0) ? _15888.x : _15888.x), _5688);
                float _5697 = _5691 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                float _15898;
                float _15899;
                if (_5697 > _5688)
                {
                    float _5711 = in.i_shape.y * fast::clamp((_5688 - _5691) / fast::max(0.60000002384185791015625 * _5691, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _15899 = _5711;
                    _15898 = _5691 * (1.0 + (0.60000002384185791015625 * _5711));
                }
                else
                {
                    _15899 = in.i_shape.y;
                    _15898 = _5697;
                }
                float2 _5724 = (abs(_5397) - _5394) + float2(_15898);
                float2 _5726 = fast::max(_5724, float2(0.0));
                float _15900;
                if ((_5726.x > 0.0) && (_5726.y > 0.0))
                {
                    float _15901;
                    if ((_15899 > 0.001000000047497451305389404296875) && (_15898 > 9.9999997473787516355514526367188e-05))
                    {
                        float _5743 = 2.0 + (2.0 * _15899);
                        float2 _5748 = _5726 / float2(fast::max(_15898, 9.9999997473787516355514526367188e-05));
                        _15901 = pow(pow(_5748.x, _5743) + pow(_5748.y, _5743), 1.0 / _5743) * _15898;
                    }
                    else
                    {
                        _15901 = length(_5726);
                    }
                    _15900 = _15901;
                }
                else
                {
                    _15900 = fast::max(_5726.x, _5726.y);
                }
                float _5783 = (fast::min(fast::max(_5724.x, _5724.y), 0.0) + _15900) - _15898;
                float _5788 = fast::max(_15888.y, 9.9999997473787516355514526367188e-05);
                float _5797 = fast::max(_5788 - abs(_5652 - _5783), 0.0) / _5788;
                _15910 = fast::min(_5652, _5783) - (((_5797 * _5797) * _5788) * 0.25);
            }
            else
            {
                _15910 = _5652;
            }
            _15909 = _15910;
        }
        _15908 = _15909;
    }
    float _4081 = fast::clamp(0.5 - (_15908 / _4056), 0.0, 1.0);
    float _17833;
    if ((in.i_flags.x & 1024u) != 0u)
    {
        uint _5813 = (in.i_instance * 24u) + 19u;
        uint _5821 = (in.i_instance * 24u) + 20u;
        float2 _4110 = fast::max((gFxData_1._data[_5813].zw - gFxData_1._data[_5813].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float2 _4113 = in.i_local - ((gFxData_1._data[_5813].xy + gFxData_1._data[_5813].zw) * 0.5);
        float _5859 = fast::min(_4110.x, _4110.y);
        float _5862 = fast::min((_4113.x > 0.0) ? ((_4113.y > 0.0) ? gFxData_1._data[_5821].x : gFxData_1._data[_5821].x) : ((_4113.y > 0.0) ? gFxData_1._data[_5821].x : gFxData_1._data[_5821].x), _5859);
        float _5868 = _5862 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_5821].y));
        float _15911;
        float _15912;
        if (_5868 > _5859)
        {
            float _5882 = gFxData_1._data[_5821].y * fast::clamp((_5859 - _5862) / fast::max(0.60000002384185791015625 * _5862, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _15912 = _5882;
            _15911 = _5862 * (1.0 + (0.60000002384185791015625 * _5882));
        }
        else
        {
            _15912 = gFxData_1._data[_5821].y;
            _15911 = _5868;
        }
        float2 _5895 = (abs(_4113) - _4110) + float2(_15911);
        float2 _5897 = fast::max(_5895, float2(0.0));
        float _15913;
        if ((_5897.x > 0.0) && (_5897.y > 0.0))
        {
            float _15914;
            if ((_15912 > 0.001000000047497451305389404296875) && (_15911 > 9.9999997473787516355514526367188e-05))
            {
                float _5914 = 2.0 + (2.0 * _15912);
                float2 _5919 = _5897 / float2(fast::max(_15911, 9.9999997473787516355514526367188e-05));
                _15914 = pow(pow(_5919.x, _5914) + pow(_5919.y, _5914), 1.0 / _5914) * _15911;
            }
            else
            {
                _15914 = length(_5897);
            }
            _15913 = _15914;
        }
        else
        {
            _15913 = fast::max(_5897.x, _5897.y);
        }
        float _4125 = fast::clamp(0.5 - (((fast::min(fast::max(_5895.x, _5895.y), 0.0) + _15913) - _15911) / _4056), 0.0, 1.0);
        if (_4125 <= 0.0)
        {
            discard_fragment();
        }
        _17833 = _4125;
    }
    else
    {
        _17833 = 1.0;
    }
    bool _4136 = ((in.i_flags.x & 32u) != 0u) && (_4081 >= 0.999000012874603271484375);
    float4 _16003;
    if ((((in.i_flags.x & 4u) != 0u) && (!((in.i_flags.x & 256u) != 0u))) && (!_4136))
    {
        uint _5960 = (in.i_instance * 24u) + 7u;
        uint _5968 = (in.i_instance * 24u) + 8u;
        float2 _4167 = in.i_local - gFxData_1._data[_5968].zw;
        float2 _6009 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _6018 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _15961;
        if (in.i_flags.z == 1u)
        {
            float2 _6024 = _4167 - _6009;
            float _15960;
            do
            {
                if (in.i_shape.w >= 6.282185077667236328125)
                {
                    _15960 = abs(length(_6024) - in.i_radii.x) - in.i_radii.y;
                    break;
                }
                float _6123 = in.i_shape.z + (in.i_shape.w * 0.5);
                float _6125 = cos(_6123);
                float _6127 = sin(_6123);
                float _6136 = dot(_6024, float2(-_6127, _6125));
                float _6139 = dot(_6024, float2(_6125, _6127));
                float2 _6140 = float2(_6136, _6139);
                float _6143 = abs(_6136);
                _6140.x = _6143;
                float _6146 = in.i_shape.w * 0.5;
                float _6148 = sin(_6146);
                float _6150 = cos(_6146);
                _15960 = (((_6150 * _6143) > (_6148 * _6139)) ? length(_6140 - (float2(_6148, _6150) * in.i_radii.x)) : abs(length(_6140) - in.i_radii.x)) - in.i_radii.y;
                break;
            } while(false);
            _15961 = _15960;
        }
        else
        {
            float _15962;
            if (in.i_flags.z == 2u)
            {
                float2 _6186 = _4167 - _15890.xy;
                float2 _6189 = _15890.zw - _15890.xy;
                _15962 = length(_6186 - (_6189 * fast::clamp(dot(_6186, _6189) / fast::max(dot(_6189, _6189), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
            }
            else
            {
                float2 _6051 = _4167 - _6009;
                float _6242 = fast::min(_6018.x, _6018.y);
                float _6245 = fast::min((_6051.x > 0.0) ? ((_6051.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_6051.y > 0.0) ? in.i_radii.w : in.i_radii.x), _6242);
                float _6251 = _6245 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                float _15945;
                float _15946;
                if (_6251 > _6242)
                {
                    float _6265 = in.i_shape.y * fast::clamp((_6242 - _6245) / fast::max(0.60000002384185791015625 * _6245, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _15946 = _6265;
                    _15945 = _6245 * (1.0 + (0.60000002384185791015625 * _6265));
                }
                else
                {
                    _15946 = in.i_shape.y;
                    _15945 = _6251;
                }
                float2 _6278 = (abs(_6051) - _6018) + float2(_15945);
                float2 _6280 = fast::max(_6278, float2(0.0));
                float _15947;
                if ((_6280.x > 0.0) && (_6280.y > 0.0))
                {
                    float _15948;
                    if ((_15946 > 0.001000000047497451305389404296875) && (_15945 > 9.9999997473787516355514526367188e-05))
                    {
                        float _6297 = 2.0 + (2.0 * _15946);
                        float2 _6302 = _6280 / float2(fast::max(_15945, 9.9999997473787516355514526367188e-05));
                        _15948 = pow(pow(_6302.x, _6297) + pow(_6302.y, _6297), 1.0 / _6297) * _15945;
                    }
                    else
                    {
                        _15948 = length(_6280);
                    }
                    _15947 = _15948;
                }
                else
                {
                    _15947 = fast::max(_6280.x, _6280.y);
                }
                float _6337 = (fast::min(fast::max(_6278.x, _6278.y), 0.0) + _15947) - _15945;
                float _15963;
                if ((in.i_flags.x & 512u) != 0u)
                {
                    float2 _6079 = fast::max((_15890.zw - _15890.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _6082 = _4167 - ((_15890.xy + _15890.zw) * 0.5);
                    float _6373 = fast::min(_6079.x, _6079.y);
                    float _6376 = fast::min((_6082.x > 0.0) ? ((_6082.y > 0.0) ? _15888.x : _15888.x) : ((_6082.y > 0.0) ? _15888.x : _15888.x), _6373);
                    float _6382 = _6376 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _15951;
                    float _15952;
                    if (_6382 > _6373)
                    {
                        float _6396 = in.i_shape.y * fast::clamp((_6373 - _6376) / fast::max(0.60000002384185791015625 * _6376, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _15952 = _6396;
                        _15951 = _6376 * (1.0 + (0.60000002384185791015625 * _6396));
                    }
                    else
                    {
                        _15952 = in.i_shape.y;
                        _15951 = _6382;
                    }
                    float2 _6409 = (abs(_6082) - _6079) + float2(_15951);
                    float2 _6411 = fast::max(_6409, float2(0.0));
                    float _15953;
                    if ((_6411.x > 0.0) && (_6411.y > 0.0))
                    {
                        float _15954;
                        if ((_15952 > 0.001000000047497451305389404296875) && (_15951 > 9.9999997473787516355514526367188e-05))
                        {
                            float _6428 = 2.0 + (2.0 * _15952);
                            float2 _6433 = _6411 / float2(fast::max(_15951, 9.9999997473787516355514526367188e-05));
                            _15954 = pow(pow(_6433.x, _6428) + pow(_6433.y, _6428), 1.0 / _6428) * _15951;
                        }
                        else
                        {
                            _15954 = length(_6411);
                        }
                        _15953 = _15954;
                    }
                    else
                    {
                        _15953 = fast::max(_6411.x, _6411.y);
                    }
                    float _6468 = (fast::min(fast::max(_6409.x, _6409.y), 0.0) + _15953) - _15951;
                    float _6473 = fast::max(_15888.y, 9.9999997473787516355514526367188e-05);
                    float _6482 = fast::max(_6473 - abs(_6337 - _6468), 0.0) / _6473;
                    _15963 = fast::min(_6337, _6468) - (((_6482 * _6482) * _6473) * 0.25);
                }
                else
                {
                    _15963 = _6337;
                }
                _15962 = _15963;
            }
            _15961 = _15962;
        }
        float _4176 = (_15961 - gFxData_1._data[_5968].y) / (fast::max(gFxData_1._data[_5968].x * 0.5, _4056 * 0.5) * 1.41421353816986083984375);
        float _6499 = sign(_4176);
        float _6501 = abs(_4176);
        float _6512 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_6501 * _6501))) * _6501)) * _6501);
        float _6515 = _6512 * _6512;
        float _6530 = fast::clamp(gFxData_1._data[_5960].w * (0.5 - (0.5 * (_6499 - (_6499 / (_6515 * _6515))))), 0.0, 1.0);
        _16003 = float4(gFxData_1._data[_5960].xyz * _6530, _6530);
    }
    else
    {
        _16003 = float4(0.0);
    }
    float4 _16894;
    if (((in.i_flags.x & 8u) != 0u) && (!_4136))
    {
        uint _6554 = (in.i_instance * 24u) + 9u;
        uint _6562 = (in.i_instance * 24u) + 10u;
        float _4204 = fast::max(gFxData_1._data[_6562].x, 0.001000000047497451305389404296875);
        float _15996;
        float _15999;
        if ((in.i_flags.x & 8192u) != 0u)
        {
            uint _6570 = (in.i_instance * 24u) + 22u;
            float2 _4220 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _4229 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float2 _4236 = in.i_rect.xy - gFxData_1._data[_6570].xy;
            float2 _4243 = gFxData_1._data[_6570].zw - in.i_rect.zw;
            float2 _4273 = fast::clamp(float2((in.i_local.x < _4220.x) ? _4236.x : _4243.x, (in.i_local.y < _4220.y) ? _4236.y : _4243.y) * float2(0.58823525905609130859375), float2(fast::min(_4204, 1.5)), float2(_4204));
            float _4278 = fast::min(_4229.x, _4229.y);
            float _15994;
            if (in.i_flags.z == 0u)
            {
                _15994 = fast::min(((in.i_local.x > _4220.x) ? ((in.i_local.y > _4220.y) ? in.i_radii.z : in.i_radii.y) : ((in.i_local.y > _4220.y) ? in.i_radii.w : in.i_radii.x)) * (1.0 + (0.60000002384185791015625 * in.i_shape.y)), _4278);
            }
            else
            {
                _15994 = _4278;
            }
            float2 _4327 = fast::max(abs(in.i_local - _4220) - (_4229 - float2(_15994)), float2(0.0));
            float _4329 = length(_4327);
            float2 _4337 = select(float2(0.707099974155426025390625), _4327 / float2(_4329), bool2(_4329 > 9.9999997473787516355514526367188e-05));
            float2 _4352 = in.i_local - gFxData_1._data[_6570].xy;
            float2 _4357 = gFxData_1._data[_6570].zw - in.i_local;
            _15999 = fast::clamp(fast::min(fast::min(_4352.x, _4352.y), fast::min(_4357.x, _4357.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _15996 = rsqrt(dot(_4337 * _4337, float2(1.0) / (_4273 * _4273)));
        }
        else
        {
            _15999 = 1.0;
            _15996 = _4204;
        }
        float _4375 = fast::max(_15908, 0.0) / _15996;
        float _6580 = fast::clamp(gFxData_1._data[_6554].w * fast::clamp((exp(((-_4375) * _4375) * 2.2000000476837158203125) * gFxData_1._data[_6562].y) * _15999, 0.0, 1.0), 0.0, 1.0);
        _16894 = float4(gFxData_1._data[_6554].xyz * _6580, _6580) + (_16003 * (1.0 - _6580));
    }
    else
    {
        _16894 = _16003;
    }
    float4 _17405;
    float4 _17418;
    float _17799;
    if (((in.i_flags.x & 32u) != 0u) && (_4081 > 0.0))
    {
        uint _6604 = (in.i_instance * 24u) + 11u;
        uint _6612 = (in.i_instance * 24u) + 12u;
        uint _6620 = (in.i_instance * 24u) + 13u;
        uint _6628 = (in.i_instance * 24u) + 16u;
        float _6700 = gFxData_1._data[_6604].x * _184.gDisplay.z;
        float2 _6711 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(1.0));
        float _6719 = fast::clamp(gFxData_1._data[_6604].z, 0.001000000047497451305389404296875, fast::min(_6711.x, _6711.y));
        float _6728 = fast::clamp(1.0 - (fast::max(-_15908, 0.0) / _6719), 0.0, 1.0);
        float _6734 = sqrt(fast::clamp(1.0 - (_6728 * _6728), 0.0, 1.0));
        float2 _16095;
        float3 _16672;
        if (_6728 > 0.0)
        {
            float2 _7141 = in.i_local + float2(0.5, 0.0);
            float2 _7209 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _7218 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _16035;
            if (in.i_flags.z == 1u)
            {
                float2 _7224 = _7141 - _7209;
                float _16034;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _16034 = abs(length(_7224) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _7323 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _7325 = cos(_7323);
                    float _7327 = sin(_7323);
                    float _7336 = dot(_7224, float2(-_7327, _7325));
                    float _7339 = dot(_7224, float2(_7325, _7327));
                    float2 _7340 = float2(_7336, _7339);
                    float _7343 = abs(_7336);
                    _7340.x = _7343;
                    float _7346 = in.i_shape.w * 0.5;
                    float _7348 = sin(_7346);
                    float _7350 = cos(_7346);
                    _16034 = (((_7350 * _7343) > (_7348 * _7339)) ? length(_7340 - (float2(_7348, _7350) * in.i_radii.x)) : abs(length(_7340) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _16035 = _16034;
            }
            else
            {
                float _16036;
                if (in.i_flags.z == 2u)
                {
                    float2 _7386 = _7141 - _15890.xy;
                    float2 _7389 = _15890.zw - _15890.xy;
                    _16036 = length(_7386 - (_7389 * fast::clamp(dot(_7386, _7389) / fast::max(dot(_7389, _7389), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _7251 = _7141 - _7209;
                    float _7442 = fast::min(_7218.x, _7218.y);
                    float _7445 = fast::min((_7251.x > 0.0) ? ((_7251.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_7251.y > 0.0) ? in.i_radii.w : in.i_radii.x), _7442);
                    float _7451 = _7445 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _16019;
                    float _16020;
                    if (_7451 > _7442)
                    {
                        float _7465 = in.i_shape.y * fast::clamp((_7442 - _7445) / fast::max(0.60000002384185791015625 * _7445, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16020 = _7465;
                        _16019 = _7445 * (1.0 + (0.60000002384185791015625 * _7465));
                    }
                    else
                    {
                        _16020 = in.i_shape.y;
                        _16019 = _7451;
                    }
                    float2 _7478 = (abs(_7251) - _7218) + float2(_16019);
                    float2 _7480 = fast::max(_7478, float2(0.0));
                    float _16021;
                    if ((_7480.x > 0.0) && (_7480.y > 0.0))
                    {
                        float _16022;
                        if ((_16020 > 0.001000000047497451305389404296875) && (_16019 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7497 = 2.0 + (2.0 * _16020);
                            float2 _7502 = _7480 / float2(fast::max(_16019, 9.9999997473787516355514526367188e-05));
                            _16022 = pow(pow(_7502.x, _7497) + pow(_7502.y, _7497), 1.0 / _7497) * _16019;
                        }
                        else
                        {
                            _16022 = length(_7480);
                        }
                        _16021 = _16022;
                    }
                    else
                    {
                        _16021 = fast::max(_7480.x, _7480.y);
                    }
                    float _7537 = (fast::min(fast::max(_7478.x, _7478.y), 0.0) + _16021) - _16019;
                    float _16037;
                    if ((in.i_flags.x & 512u) != 0u)
                    {
                        float2 _7279 = fast::max((_15890.zw - _15890.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _7282 = _7141 - ((_15890.xy + _15890.zw) * 0.5);
                        float _7573 = fast::min(_7279.x, _7279.y);
                        float _7576 = fast::min((_7282.x > 0.0) ? ((_7282.y > 0.0) ? gFxData_1._data[_6628].x : gFxData_1._data[_6628].x) : ((_7282.y > 0.0) ? gFxData_1._data[_6628].x : gFxData_1._data[_6628].x), _7573);
                        float _7582 = _7576 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _16025;
                        float _16026;
                        if (_7582 > _7573)
                        {
                            float _7596 = in.i_shape.y * fast::clamp((_7573 - _7576) / fast::max(0.60000002384185791015625 * _7576, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _16026 = _7596;
                            _16025 = _7576 * (1.0 + (0.60000002384185791015625 * _7596));
                        }
                        else
                        {
                            _16026 = in.i_shape.y;
                            _16025 = _7582;
                        }
                        float2 _7609 = (abs(_7282) - _7279) + float2(_16025);
                        float2 _7611 = fast::max(_7609, float2(0.0));
                        float _16027;
                        if ((_7611.x > 0.0) && (_7611.y > 0.0))
                        {
                            float _16028;
                            if ((_16026 > 0.001000000047497451305389404296875) && (_16025 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7628 = 2.0 + (2.0 * _16026);
                                float2 _7633 = _7611 / float2(fast::max(_16025, 9.9999997473787516355514526367188e-05));
                                _16028 = pow(pow(_7633.x, _7628) + pow(_7633.y, _7628), 1.0 / _7628) * _16025;
                            }
                            else
                            {
                                _16028 = length(_7611);
                            }
                            _16027 = _16028;
                        }
                        else
                        {
                            _16027 = fast::max(_7611.x, _7611.y);
                        }
                        float _7668 = (fast::min(fast::max(_7609.x, _7609.y), 0.0) + _16027) - _16025;
                        float _7673 = fast::max(gFxData_1._data[_6628].y, 9.9999997473787516355514526367188e-05);
                        float _7682 = fast::max(_7673 - abs(_7537 - _7668), 0.0) / _7673;
                        _16037 = fast::min(_7537, _7668) - (((_7682 * _7682) * _7673) * 0.25);
                    }
                    else
                    {
                        _16037 = _7537;
                    }
                    _16036 = _16037;
                }
                _16035 = _16036;
            }
            float2 _7145 = in.i_local - float2(0.5, 0.0);
            float2 _7731 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _7740 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _16054;
            if (in.i_flags.z == 1u)
            {
                float2 _7746 = _7145 - _7731;
                float _16053;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _16053 = abs(length(_7746) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _7845 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _7847 = cos(_7845);
                    float _7849 = sin(_7845);
                    float _7858 = dot(_7746, float2(-_7849, _7847));
                    float _7861 = dot(_7746, float2(_7847, _7849));
                    float2 _7862 = float2(_7858, _7861);
                    float _7865 = abs(_7858);
                    _7862.x = _7865;
                    float _7868 = in.i_shape.w * 0.5;
                    float _7870 = sin(_7868);
                    float _7872 = cos(_7868);
                    _16053 = (((_7872 * _7865) > (_7870 * _7861)) ? length(_7862 - (float2(_7870, _7872) * in.i_radii.x)) : abs(length(_7862) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _16054 = _16053;
            }
            else
            {
                float _16055;
                if (in.i_flags.z == 2u)
                {
                    float2 _7908 = _7145 - _15890.xy;
                    float2 _7911 = _15890.zw - _15890.xy;
                    _16055 = length(_7908 - (_7911 * fast::clamp(dot(_7908, _7911) / fast::max(dot(_7911, _7911), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _7773 = _7145 - _7731;
                    float _7964 = fast::min(_7740.x, _7740.y);
                    float _7967 = fast::min((_7773.x > 0.0) ? ((_7773.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_7773.y > 0.0) ? in.i_radii.w : in.i_radii.x), _7964);
                    float _7973 = _7967 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _16038;
                    float _16039;
                    if (_7973 > _7964)
                    {
                        float _7987 = in.i_shape.y * fast::clamp((_7964 - _7967) / fast::max(0.60000002384185791015625 * _7967, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16039 = _7987;
                        _16038 = _7967 * (1.0 + (0.60000002384185791015625 * _7987));
                    }
                    else
                    {
                        _16039 = in.i_shape.y;
                        _16038 = _7973;
                    }
                    float2 _8000 = (abs(_7773) - _7740) + float2(_16038);
                    float2 _8002 = fast::max(_8000, float2(0.0));
                    float _16040;
                    if ((_8002.x > 0.0) && (_8002.y > 0.0))
                    {
                        float _16041;
                        if ((_16039 > 0.001000000047497451305389404296875) && (_16038 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8019 = 2.0 + (2.0 * _16039);
                            float2 _8024 = _8002 / float2(fast::max(_16038, 9.9999997473787516355514526367188e-05));
                            _16041 = pow(pow(_8024.x, _8019) + pow(_8024.y, _8019), 1.0 / _8019) * _16038;
                        }
                        else
                        {
                            _16041 = length(_8002);
                        }
                        _16040 = _16041;
                    }
                    else
                    {
                        _16040 = fast::max(_8002.x, _8002.y);
                    }
                    float _8059 = (fast::min(fast::max(_8000.x, _8000.y), 0.0) + _16040) - _16038;
                    float _16056;
                    if ((in.i_flags.x & 512u) != 0u)
                    {
                        float2 _7801 = fast::max((_15890.zw - _15890.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _7804 = _7145 - ((_15890.xy + _15890.zw) * 0.5);
                        float _8095 = fast::min(_7801.x, _7801.y);
                        float _8098 = fast::min((_7804.x > 0.0) ? ((_7804.y > 0.0) ? gFxData_1._data[_6628].x : gFxData_1._data[_6628].x) : ((_7804.y > 0.0) ? gFxData_1._data[_6628].x : gFxData_1._data[_6628].x), _8095);
                        float _8104 = _8098 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _16044;
                        float _16045;
                        if (_8104 > _8095)
                        {
                            float _8118 = in.i_shape.y * fast::clamp((_8095 - _8098) / fast::max(0.60000002384185791015625 * _8098, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _16045 = _8118;
                            _16044 = _8098 * (1.0 + (0.60000002384185791015625 * _8118));
                        }
                        else
                        {
                            _16045 = in.i_shape.y;
                            _16044 = _8104;
                        }
                        float2 _8131 = (abs(_7804) - _7801) + float2(_16044);
                        float2 _8133 = fast::max(_8131, float2(0.0));
                        float _16046;
                        if ((_8133.x > 0.0) && (_8133.y > 0.0))
                        {
                            float _16047;
                            if ((_16045 > 0.001000000047497451305389404296875) && (_16044 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8150 = 2.0 + (2.0 * _16045);
                                float2 _8155 = _8133 / float2(fast::max(_16044, 9.9999997473787516355514526367188e-05));
                                _16047 = pow(pow(_8155.x, _8150) + pow(_8155.y, _8150), 1.0 / _8150) * _16044;
                            }
                            else
                            {
                                _16047 = length(_8133);
                            }
                            _16046 = _16047;
                        }
                        else
                        {
                            _16046 = fast::max(_8133.x, _8133.y);
                        }
                        float _8190 = (fast::min(fast::max(_8131.x, _8131.y), 0.0) + _16046) - _16044;
                        float _8195 = fast::max(gFxData_1._data[_6628].y, 9.9999997473787516355514526367188e-05);
                        float _8204 = fast::max(_8195 - abs(_8059 - _8190), 0.0) / _8195;
                        _16056 = fast::min(_8059, _8190) - (((_8204 * _8204) * _8195) * 0.25);
                    }
                    else
                    {
                        _16056 = _8059;
                    }
                    _16055 = _16056;
                }
                _16054 = _16055;
            }
            float2 _7150 = in.i_local + float2(0.0, 0.5);
            float2 _8253 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _8262 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _16073;
            if (in.i_flags.z == 1u)
            {
                float2 _8268 = _7150 - _8253;
                float _16072;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _16072 = abs(length(_8268) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _8367 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _8369 = cos(_8367);
                    float _8371 = sin(_8367);
                    float _8380 = dot(_8268, float2(-_8371, _8369));
                    float _8383 = dot(_8268, float2(_8369, _8371));
                    float2 _8384 = float2(_8380, _8383);
                    float _8387 = abs(_8380);
                    _8384.x = _8387;
                    float _8390 = in.i_shape.w * 0.5;
                    float _8392 = sin(_8390);
                    float _8394 = cos(_8390);
                    _16072 = (((_8394 * _8387) > (_8392 * _8383)) ? length(_8384 - (float2(_8392, _8394) * in.i_radii.x)) : abs(length(_8384) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _16073 = _16072;
            }
            else
            {
                float _16074;
                if (in.i_flags.z == 2u)
                {
                    float2 _8430 = _7150 - _15890.xy;
                    float2 _8433 = _15890.zw - _15890.xy;
                    _16074 = length(_8430 - (_8433 * fast::clamp(dot(_8430, _8433) / fast::max(dot(_8433, _8433), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _8295 = _7150 - _8253;
                    float _8486 = fast::min(_8262.x, _8262.y);
                    float _8489 = fast::min((_8295.x > 0.0) ? ((_8295.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_8295.y > 0.0) ? in.i_radii.w : in.i_radii.x), _8486);
                    float _8495 = _8489 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _16057;
                    float _16058;
                    if (_8495 > _8486)
                    {
                        float _8509 = in.i_shape.y * fast::clamp((_8486 - _8489) / fast::max(0.60000002384185791015625 * _8489, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16058 = _8509;
                        _16057 = _8489 * (1.0 + (0.60000002384185791015625 * _8509));
                    }
                    else
                    {
                        _16058 = in.i_shape.y;
                        _16057 = _8495;
                    }
                    float2 _8522 = (abs(_8295) - _8262) + float2(_16057);
                    float2 _8524 = fast::max(_8522, float2(0.0));
                    float _16059;
                    if ((_8524.x > 0.0) && (_8524.y > 0.0))
                    {
                        float _16060;
                        if ((_16058 > 0.001000000047497451305389404296875) && (_16057 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8541 = 2.0 + (2.0 * _16058);
                            float2 _8546 = _8524 / float2(fast::max(_16057, 9.9999997473787516355514526367188e-05));
                            _16060 = pow(pow(_8546.x, _8541) + pow(_8546.y, _8541), 1.0 / _8541) * _16057;
                        }
                        else
                        {
                            _16060 = length(_8524);
                        }
                        _16059 = _16060;
                    }
                    else
                    {
                        _16059 = fast::max(_8524.x, _8524.y);
                    }
                    float _8581 = (fast::min(fast::max(_8522.x, _8522.y), 0.0) + _16059) - _16057;
                    float _16075;
                    if ((in.i_flags.x & 512u) != 0u)
                    {
                        float2 _8323 = fast::max((_15890.zw - _15890.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _8326 = _7150 - ((_15890.xy + _15890.zw) * 0.5);
                        float _8617 = fast::min(_8323.x, _8323.y);
                        float _8620 = fast::min((_8326.x > 0.0) ? ((_8326.y > 0.0) ? gFxData_1._data[_6628].x : gFxData_1._data[_6628].x) : ((_8326.y > 0.0) ? gFxData_1._data[_6628].x : gFxData_1._data[_6628].x), _8617);
                        float _8626 = _8620 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _16063;
                        float _16064;
                        if (_8626 > _8617)
                        {
                            float _8640 = in.i_shape.y * fast::clamp((_8617 - _8620) / fast::max(0.60000002384185791015625 * _8620, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _16064 = _8640;
                            _16063 = _8620 * (1.0 + (0.60000002384185791015625 * _8640));
                        }
                        else
                        {
                            _16064 = in.i_shape.y;
                            _16063 = _8626;
                        }
                        float2 _8653 = (abs(_8326) - _8323) + float2(_16063);
                        float2 _8655 = fast::max(_8653, float2(0.0));
                        float _16065;
                        if ((_8655.x > 0.0) && (_8655.y > 0.0))
                        {
                            float _16066;
                            if ((_16064 > 0.001000000047497451305389404296875) && (_16063 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8672 = 2.0 + (2.0 * _16064);
                                float2 _8677 = _8655 / float2(fast::max(_16063, 9.9999997473787516355514526367188e-05));
                                _16066 = pow(pow(_8677.x, _8672) + pow(_8677.y, _8672), 1.0 / _8672) * _16063;
                            }
                            else
                            {
                                _16066 = length(_8655);
                            }
                            _16065 = _16066;
                        }
                        else
                        {
                            _16065 = fast::max(_8655.x, _8655.y);
                        }
                        float _8712 = (fast::min(fast::max(_8653.x, _8653.y), 0.0) + _16065) - _16063;
                        float _8717 = fast::max(gFxData_1._data[_6628].y, 9.9999997473787516355514526367188e-05);
                        float _8726 = fast::max(_8717 - abs(_8581 - _8712), 0.0) / _8717;
                        _16075 = fast::min(_8581, _8712) - (((_8726 * _8726) * _8717) * 0.25);
                    }
                    else
                    {
                        _16075 = _8581;
                    }
                    _16074 = _16075;
                }
                _16073 = _16074;
            }
            float2 _7154 = in.i_local - float2(0.0, 0.5);
            float2 _8775 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _8784 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _16092;
            if (in.i_flags.z == 1u)
            {
                float2 _8790 = _7154 - _8775;
                float _16091;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _16091 = abs(length(_8790) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _8889 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _8891 = cos(_8889);
                    float _8893 = sin(_8889);
                    float _8902 = dot(_8790, float2(-_8893, _8891));
                    float _8905 = dot(_8790, float2(_8891, _8893));
                    float2 _8906 = float2(_8902, _8905);
                    float _8909 = abs(_8902);
                    _8906.x = _8909;
                    float _8912 = in.i_shape.w * 0.5;
                    float _8914 = sin(_8912);
                    float _8916 = cos(_8912);
                    _16091 = (((_8916 * _8909) > (_8914 * _8905)) ? length(_8906 - (float2(_8914, _8916) * in.i_radii.x)) : abs(length(_8906) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _16092 = _16091;
            }
            else
            {
                float _16093;
                if (in.i_flags.z == 2u)
                {
                    float2 _8952 = _7154 - _15890.xy;
                    float2 _8955 = _15890.zw - _15890.xy;
                    _16093 = length(_8952 - (_8955 * fast::clamp(dot(_8952, _8955) / fast::max(dot(_8955, _8955), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _8817 = _7154 - _8775;
                    float _9008 = fast::min(_8784.x, _8784.y);
                    float _9011 = fast::min((_8817.x > 0.0) ? ((_8817.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_8817.y > 0.0) ? in.i_radii.w : in.i_radii.x), _9008);
                    float _9017 = _9011 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _16076;
                    float _16077;
                    if (_9017 > _9008)
                    {
                        float _9031 = in.i_shape.y * fast::clamp((_9008 - _9011) / fast::max(0.60000002384185791015625 * _9011, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16077 = _9031;
                        _16076 = _9011 * (1.0 + (0.60000002384185791015625 * _9031));
                    }
                    else
                    {
                        _16077 = in.i_shape.y;
                        _16076 = _9017;
                    }
                    float2 _9044 = (abs(_8817) - _8784) + float2(_16076);
                    float2 _9046 = fast::max(_9044, float2(0.0));
                    float _16078;
                    if ((_9046.x > 0.0) && (_9046.y > 0.0))
                    {
                        float _16079;
                        if ((_16077 > 0.001000000047497451305389404296875) && (_16076 > 9.9999997473787516355514526367188e-05))
                        {
                            float _9063 = 2.0 + (2.0 * _16077);
                            float2 _9068 = _9046 / float2(fast::max(_16076, 9.9999997473787516355514526367188e-05));
                            _16079 = pow(pow(_9068.x, _9063) + pow(_9068.y, _9063), 1.0 / _9063) * _16076;
                        }
                        else
                        {
                            _16079 = length(_9046);
                        }
                        _16078 = _16079;
                    }
                    else
                    {
                        _16078 = fast::max(_9046.x, _9046.y);
                    }
                    float _9103 = (fast::min(fast::max(_9044.x, _9044.y), 0.0) + _16078) - _16076;
                    float _16094;
                    if ((in.i_flags.x & 512u) != 0u)
                    {
                        float2 _8845 = fast::max((_15890.zw - _15890.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _8848 = _7154 - ((_15890.xy + _15890.zw) * 0.5);
                        float _9139 = fast::min(_8845.x, _8845.y);
                        float _9142 = fast::min((_8848.x > 0.0) ? ((_8848.y > 0.0) ? gFxData_1._data[_6628].x : gFxData_1._data[_6628].x) : ((_8848.y > 0.0) ? gFxData_1._data[_6628].x : gFxData_1._data[_6628].x), _9139);
                        float _9148 = _9142 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _16082;
                        float _16083;
                        if (_9148 > _9139)
                        {
                            float _9162 = in.i_shape.y * fast::clamp((_9139 - _9142) / fast::max(0.60000002384185791015625 * _9142, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _16083 = _9162;
                            _16082 = _9142 * (1.0 + (0.60000002384185791015625 * _9162));
                        }
                        else
                        {
                            _16083 = in.i_shape.y;
                            _16082 = _9148;
                        }
                        float2 _9175 = (abs(_8848) - _8845) + float2(_16082);
                        float2 _9177 = fast::max(_9175, float2(0.0));
                        float _16084;
                        if ((_9177.x > 0.0) && (_9177.y > 0.0))
                        {
                            float _16085;
                            if ((_16083 > 0.001000000047497451305389404296875) && (_16082 > 9.9999997473787516355514526367188e-05))
                            {
                                float _9194 = 2.0 + (2.0 * _16083);
                                float2 _9199 = _9177 / float2(fast::max(_16082, 9.9999997473787516355514526367188e-05));
                                _16085 = pow(pow(_9199.x, _9194) + pow(_9199.y, _9194), 1.0 / _9194) * _16082;
                            }
                            else
                            {
                                _16085 = length(_9177);
                            }
                            _16084 = _16085;
                        }
                        else
                        {
                            _16084 = fast::max(_9177.x, _9177.y);
                        }
                        float _9234 = (fast::min(fast::max(_9175.x, _9175.y), 0.0) + _16084) - _16082;
                        float _9239 = fast::max(gFxData_1._data[_6628].y, 9.9999997473787516355514526367188e-05);
                        float _9248 = fast::max(_9239 - abs(_9103 - _9234), 0.0) / _9239;
                        _16094 = fast::min(_9103, _9234) - (((_9248 * _9248) * _9239) * 0.25);
                    }
                    else
                    {
                        _16094 = _9103;
                    }
                    _16093 = _16094;
                }
                _16092 = _16093;
            }
            float2 _7160 = float2(_16035 - _16054, _16073 - _16092);
            float _7162 = length(_7160);
            float2 _7170 = select(float2(0.0, -1.0), _7160 / float2(_7162), bool2(_7162 > 9.9999997473787516355514526367188e-06));
            _16672 = fast::normalize(float3(_7170 * fast::min(_6728 / fast::max(_6734, 0.001000000047497451305389404296875), 8.0), 1.0));
            _16095 = _7170;
        }
        else
        {
            _16672 = float3(0.0, 0.0, 1.0);
            _16095 = float2(0.0, -1.0);
        }
        float2 _6760 = ((-_16095) * gFxData_1._data[_6604].y) * (1.0 - _6734);
        float2 _16096;
        if (gFxData_1._data[_6628].z > 0.0)
        {
            _16096 = (((in.i_rect.xy + in.i_rect.zw) * 0.5) - in.i_local) * (gFxData_1._data[_6628].z / (1.0 + gFxData_1._data[_6628].z));
        }
        else
        {
            _16096 = float2(0.0);
        }
        float2 _6786 = _15887 * _184.gTarget.zw;
        float2 _6793 = _184.gDisplay.zw * _184.gTarget.zw;
        float2 _6798 = (_6760 + _16096) * _6793;
        float2 _6801 = _6760 * _6793;
        float3 _16554;
        float _16572;
        float3 _16775;
        if (_184.gTime.z > 0.5)
        {
            float2 _6808 = _6786 + _6798;
            float _9273 = fast::clamp(log2(fast::max(_6700, 1.0)) - 1.0, 0.0, 5.0);
            int _9276 = int(floor(_9273));
            float _9280 = _9273 - float(_9276);
            bool _9285 = (_9280 > 0.0199999995529651641845703125) && (_9276 < 5);
            float3 _16167;
            _16167 = float3(0.0);
            float3 _9314;
            for (int _16097 = 0; _16097 < 2; _16167 = _9314, _16097++)
            {
                if ((_16097 > 0) && (!_9285))
                {
                    break;
                }
                int _9300 = _9276 + _16097;
                int _9338 = clamp(_9300, 1, 5);
                float2 _9407 = (_6808 * _184.gLevel[_9338].xy) - float2(0.5);
                float2 _9409 = floor(_9407);
                float2 _9412 = _9407 - _9409;
                float2 _9415 = _9412 * _9412;
                float2 _9418 = _9415 * _9412;
                float2 _9437 = (((_9418 * 3.0) - (_9415 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                float2 _9450 = _9418 * 0.16666667163372039794921875;
                float2 _9453 = (((((-_9418) + (_9415 * 3.0)) - (_9412 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9437;
                float2 _9457 = (((((_9418 * (-3.0)) + (_9415 * 3.0)) + (_9412 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9450;
                float2 _9469 = ((_9409 - float2(0.5)) + (_9437 / _9453)) * _184.gLevel[_9338].zw;
                float2 _9481 = ((_9409 + float2(1.5)) + (_9450 / _9457)) * _184.gLevel[_9338].zw;
                float4 _16134;
                if (_9300 <= 0)
                {
                    float2 _16133;
                    if (_184.gConv.x > 0.5)
                    {
                        float2 _15218 = _6808;
                        _15218.y = 1.0 - _6808.y;
                        _16133 = _15218;
                    }
                    else
                    {
                        _16133 = _6808;
                    }
                    _16134 = gBackdrop0.sample(gLinear, _16133, level(0.0));
                }
                else
                {
                    float4 _16135;
                    if (_9300 == 1)
                    {
                        float2 _16126;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _15223 = _9469;
                            _15223.y = 1.0 - _9469.y;
                            _16126 = _15223;
                        }
                        else
                        {
                            _16126 = _9469;
                        }
                        float2 _9527 = float2(_9481.x, _9469.y);
                        float2 _16127;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _15230 = _9527;
                            _15230.y = 1.0 - _9469.y;
                            _16127 = _15230;
                        }
                        else
                        {
                            _16127 = _9527;
                        }
                        float2 _9545 = float2(_9469.x, _9481.y);
                        float2 _16129;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _15237 = _9545;
                            _15237.y = 1.0 - _9481.y;
                            _16129 = _15237;
                        }
                        else
                        {
                            _16129 = _9545;
                        }
                        float2 _16131;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _15244 = _9481;
                            _15244.y = 1.0 - _9481.y;
                            _16131 = _15244;
                        }
                        else
                        {
                            _16131 = _9481;
                        }
                        _16135 = (((gBackdrop1.sample(gLinear, _16126, level(0.0)) * (_9453.x * _9453.y)) + (gBackdrop1.sample(gLinear, _16127, level(0.0)) * (_9457.x * _9453.y))) + (gBackdrop1.sample(gLinear, _16129, level(0.0)) * (_9453.x * _9457.y))) + (gBackdrop1.sample(gLinear, _16131, level(0.0)) * (_9457.x * _9457.y));
                    }
                    else
                    {
                        float4 _16136;
                        if (_9300 == 2)
                        {
                            float2 _16119;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15251 = _9469;
                                _15251.y = 1.0 - _9469.y;
                                _16119 = _15251;
                            }
                            else
                            {
                                _16119 = _9469;
                            }
                            float2 _9657 = float2(_9481.x, _9469.y);
                            float2 _16120;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15258 = _9657;
                                _15258.y = 1.0 - _9469.y;
                                _16120 = _15258;
                            }
                            else
                            {
                                _16120 = _9657;
                            }
                            float2 _9675 = float2(_9469.x, _9481.y);
                            float2 _16122;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15265 = _9675;
                                _15265.y = 1.0 - _9481.y;
                                _16122 = _15265;
                            }
                            else
                            {
                                _16122 = _9675;
                            }
                            float2 _16124;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15272 = _9481;
                                _15272.y = 1.0 - _9481.y;
                                _16124 = _15272;
                            }
                            else
                            {
                                _16124 = _9481;
                            }
                            _16136 = (((gBackdrop2.sample(gLinear, _16119, level(0.0)) * (_9453.x * _9453.y)) + (gBackdrop2.sample(gLinear, _16120, level(0.0)) * (_9457.x * _9453.y))) + (gBackdrop2.sample(gLinear, _16122, level(0.0)) * (_9453.x * _9457.y))) + (gBackdrop2.sample(gLinear, _16124, level(0.0)) * (_9457.x * _9457.y));
                        }
                        else
                        {
                            float4 _16137;
                            if (_9300 == 3)
                            {
                                float2 _16112;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15279 = _9469;
                                    _15279.y = 1.0 - _9469.y;
                                    _16112 = _15279;
                                }
                                else
                                {
                                    _16112 = _9469;
                                }
                                float2 _9787 = float2(_9481.x, _9469.y);
                                float2 _16113;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15286 = _9787;
                                    _15286.y = 1.0 - _9469.y;
                                    _16113 = _15286;
                                }
                                else
                                {
                                    _16113 = _9787;
                                }
                                float2 _9805 = float2(_9469.x, _9481.y);
                                float2 _16115;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15293 = _9805;
                                    _15293.y = 1.0 - _9481.y;
                                    _16115 = _15293;
                                }
                                else
                                {
                                    _16115 = _9805;
                                }
                                float2 _16117;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15300 = _9481;
                                    _15300.y = 1.0 - _9481.y;
                                    _16117 = _15300;
                                }
                                else
                                {
                                    _16117 = _9481;
                                }
                                _16137 = (((gBackdrop3.sample(gLinear, _16112, level(0.0)) * (_9453.x * _9453.y)) + (gBackdrop3.sample(gLinear, _16113, level(0.0)) * (_9457.x * _9453.y))) + (gBackdrop3.sample(gLinear, _16115, level(0.0)) * (_9453.x * _9457.y))) + (gBackdrop3.sample(gLinear, _16117, level(0.0)) * (_9457.x * _9457.y));
                            }
                            else
                            {
                                float4 _16138;
                                if (_9300 == 4)
                                {
                                    float2 _16105;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15307 = _9469;
                                        _15307.y = 1.0 - _9469.y;
                                        _16105 = _15307;
                                    }
                                    else
                                    {
                                        _16105 = _9469;
                                    }
                                    float2 _9917 = float2(_9481.x, _9469.y);
                                    float2 _16106;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15314 = _9917;
                                        _15314.y = 1.0 - _9469.y;
                                        _16106 = _15314;
                                    }
                                    else
                                    {
                                        _16106 = _9917;
                                    }
                                    float2 _9935 = float2(_9469.x, _9481.y);
                                    float2 _16108;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15321 = _9935;
                                        _15321.y = 1.0 - _9481.y;
                                        _16108 = _15321;
                                    }
                                    else
                                    {
                                        _16108 = _9935;
                                    }
                                    float2 _16110;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15328 = _9481;
                                        _15328.y = 1.0 - _9481.y;
                                        _16110 = _15328;
                                    }
                                    else
                                    {
                                        _16110 = _9481;
                                    }
                                    _16138 = (((gBackdrop4.sample(gLinear, _16105, level(0.0)) * (_9453.x * _9453.y)) + (gBackdrop4.sample(gLinear, _16106, level(0.0)) * (_9457.x * _9453.y))) + (gBackdrop4.sample(gLinear, _16108, level(0.0)) * (_9453.x * _9457.y))) + (gBackdrop4.sample(gLinear, _16110, level(0.0)) * (_9457.x * _9457.y));
                                }
                                else
                                {
                                    float2 _16098;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15335 = _9469;
                                        _15335.y = 1.0 - _9469.y;
                                        _16098 = _15335;
                                    }
                                    else
                                    {
                                        _16098 = _9469;
                                    }
                                    float2 _10047 = float2(_9481.x, _9469.y);
                                    float2 _16099;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15342 = _10047;
                                        _15342.y = 1.0 - _9469.y;
                                        _16099 = _15342;
                                    }
                                    else
                                    {
                                        _16099 = _10047;
                                    }
                                    float2 _10065 = float2(_9469.x, _9481.y);
                                    float2 _16101;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15349 = _10065;
                                        _15349.y = 1.0 - _9481.y;
                                        _16101 = _15349;
                                    }
                                    else
                                    {
                                        _16101 = _10065;
                                    }
                                    float2 _16103;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15356 = _9481;
                                        _15356.y = 1.0 - _9481.y;
                                        _16103 = _15356;
                                    }
                                    else
                                    {
                                        _16103 = _9481;
                                    }
                                    _16138 = (((gBackdrop5.sample(gLinear, _16098, level(0.0)) * (_9453.x * _9453.y)) + (gBackdrop5.sample(gLinear, _16099, level(0.0)) * (_9457.x * _9453.y))) + (gBackdrop5.sample(gLinear, _16101, level(0.0)) * (_9453.x * _9457.y))) + (gBackdrop5.sample(gLinear, _16103, level(0.0)) * (_9457.x * _9457.y));
                                }
                                _16137 = _16138;
                            }
                            _16136 = _16137;
                        }
                        _16135 = _16136;
                    }
                    _16134 = _16135;
                }
                _9314 = _16167 + (_16134.xyz * (_9285 ? ((_16097 == 0) ? (1.0 - _9280) : _9280) : 1.0));
            }
            float3 _16557;
            if (((gFxData_1._data[_6604].w > 0.001000000047497451305389404296875) && (_6728 > 0.0)) && ((((gFxData_1._data[_6604].y * 0.300000011920928955078125) * gFxData_1._data[_6604].w) * _184.gDisplay.z) > (_6700 * 0.3499999940395355224609375)))
            {
                float _6828 = 0.300000011920928955078125 * gFxData_1._data[_6604].w;
                float2 _6835 = (_6786 + _6798) - (_6801 * _6828);
                float _10161 = fast::clamp(log2(fast::max(_6700, 1.0)) - 1.0, 0.0, 5.0);
                int _10164 = int(floor(_10161));
                float _10168 = _10161 - float(_10164);
                bool _10173 = (_10168 > 0.0199999995529651641845703125) && (_10164 < 5);
                float3 _16263;
                _16263 = float3(0.0);
                float3 _10202;
                for (int _16193 = 0; _16193 < 2; _16263 = _10202, _16193++)
                {
                    if ((_16193 > 0) && (!_10173))
                    {
                        break;
                    }
                    int _10188 = _10164 + _16193;
                    int _10226 = clamp(_10188, 1, 5);
                    float2 _10295 = (_6835 * _184.gLevel[_10226].xy) - float2(0.5);
                    float2 _10297 = floor(_10295);
                    float2 _10300 = _10295 - _10297;
                    float2 _10303 = _10300 * _10300;
                    float2 _10306 = _10303 * _10300;
                    float2 _10325 = (((_10306 * 3.0) - (_10303 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                    float2 _10338 = _10306 * 0.16666667163372039794921875;
                    float2 _10341 = (((((-_10306) + (_10303 * 3.0)) - (_10300 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10325;
                    float2 _10345 = (((((_10306 * (-3.0)) + (_10303 * 3.0)) + (_10300 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10338;
                    float2 _10357 = ((_10297 - float2(0.5)) + (_10325 / _10341)) * _184.gLevel[_10226].zw;
                    float2 _10369 = ((_10297 + float2(1.5)) + (_10338 / _10345)) * _184.gLevel[_10226].zw;
                    float4 _16230;
                    if (_10188 <= 0)
                    {
                        float2 _16229;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _15361 = _6835;
                            _15361.y = 1.0 - _6835.y;
                            _16229 = _15361;
                        }
                        else
                        {
                            _16229 = _6835;
                        }
                        _16230 = gBackdrop0.sample(gLinear, _16229, level(0.0));
                    }
                    else
                    {
                        float4 _16231;
                        if (_10188 == 1)
                        {
                            float2 _16222;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15366 = _10357;
                                _15366.y = 1.0 - _10357.y;
                                _16222 = _15366;
                            }
                            else
                            {
                                _16222 = _10357;
                            }
                            float _10414 = _10357.y;
                            float2 _10415 = float2(_10369.x, _10414);
                            float2 _16223;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15373 = _10415;
                                _15373.y = 1.0 - _10414;
                                _16223 = _15373;
                            }
                            else
                            {
                                _16223 = _10415;
                            }
                            float _10432 = _10369.y;
                            float2 _10433 = float2(_10357.x, _10432);
                            float2 _16225;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15380 = _10433;
                                _15380.y = 1.0 - _10432;
                                _16225 = _15380;
                            }
                            else
                            {
                                _16225 = _10433;
                            }
                            float2 _16227;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15387 = _10369;
                                _15387.y = 1.0 - _10369.y;
                                _16227 = _15387;
                            }
                            else
                            {
                                _16227 = _10369;
                            }
                            _16231 = (((gBackdrop1.sample(gLinear, _16222, level(0.0)) * (_10341.x * _10341.y)) + (gBackdrop1.sample(gLinear, _16223, level(0.0)) * (_10345.x * _10341.y))) + (gBackdrop1.sample(gLinear, _16225, level(0.0)) * (_10341.x * _10345.y))) + (gBackdrop1.sample(gLinear, _16227, level(0.0)) * (_10345.x * _10345.y));
                        }
                        else
                        {
                            float4 _16232;
                            if (_10188 == 2)
                            {
                                float2 _16215;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15394 = _10357;
                                    _15394.y = 1.0 - _10357.y;
                                    _16215 = _15394;
                                }
                                else
                                {
                                    _16215 = _10357;
                                }
                                float _10544 = _10357.y;
                                float2 _10545 = float2(_10369.x, _10544);
                                float2 _16216;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15401 = _10545;
                                    _15401.y = 1.0 - _10544;
                                    _16216 = _15401;
                                }
                                else
                                {
                                    _16216 = _10545;
                                }
                                float _10562 = _10369.y;
                                float2 _10563 = float2(_10357.x, _10562);
                                float2 _16218;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15408 = _10563;
                                    _15408.y = 1.0 - _10562;
                                    _16218 = _15408;
                                }
                                else
                                {
                                    _16218 = _10563;
                                }
                                float2 _16220;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15415 = _10369;
                                    _15415.y = 1.0 - _10369.y;
                                    _16220 = _15415;
                                }
                                else
                                {
                                    _16220 = _10369;
                                }
                                _16232 = (((gBackdrop2.sample(gLinear, _16215, level(0.0)) * (_10341.x * _10341.y)) + (gBackdrop2.sample(gLinear, _16216, level(0.0)) * (_10345.x * _10341.y))) + (gBackdrop2.sample(gLinear, _16218, level(0.0)) * (_10341.x * _10345.y))) + (gBackdrop2.sample(gLinear, _16220, level(0.0)) * (_10345.x * _10345.y));
                            }
                            else
                            {
                                float4 _16233;
                                if (_10188 == 3)
                                {
                                    float2 _16208;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15422 = _10357;
                                        _15422.y = 1.0 - _10357.y;
                                        _16208 = _15422;
                                    }
                                    else
                                    {
                                        _16208 = _10357;
                                    }
                                    float _10674 = _10357.y;
                                    float2 _10675 = float2(_10369.x, _10674);
                                    float2 _16209;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15429 = _10675;
                                        _15429.y = 1.0 - _10674;
                                        _16209 = _15429;
                                    }
                                    else
                                    {
                                        _16209 = _10675;
                                    }
                                    float _10692 = _10369.y;
                                    float2 _10693 = float2(_10357.x, _10692);
                                    float2 _16211;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15436 = _10693;
                                        _15436.y = 1.0 - _10692;
                                        _16211 = _15436;
                                    }
                                    else
                                    {
                                        _16211 = _10693;
                                    }
                                    float2 _16213;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15443 = _10369;
                                        _15443.y = 1.0 - _10369.y;
                                        _16213 = _15443;
                                    }
                                    else
                                    {
                                        _16213 = _10369;
                                    }
                                    _16233 = (((gBackdrop3.sample(gLinear, _16208, level(0.0)) * (_10341.x * _10341.y)) + (gBackdrop3.sample(gLinear, _16209, level(0.0)) * (_10345.x * _10341.y))) + (gBackdrop3.sample(gLinear, _16211, level(0.0)) * (_10341.x * _10345.y))) + (gBackdrop3.sample(gLinear, _16213, level(0.0)) * (_10345.x * _10345.y));
                                }
                                else
                                {
                                    float4 _16234;
                                    if (_10188 == 4)
                                    {
                                        float2 _16201;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15450 = _10357;
                                            _15450.y = 1.0 - _10357.y;
                                            _16201 = _15450;
                                        }
                                        else
                                        {
                                            _16201 = _10357;
                                        }
                                        float _10804 = _10357.y;
                                        float2 _10805 = float2(_10369.x, _10804);
                                        float2 _16202;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15457 = _10805;
                                            _15457.y = 1.0 - _10804;
                                            _16202 = _15457;
                                        }
                                        else
                                        {
                                            _16202 = _10805;
                                        }
                                        float _10822 = _10369.y;
                                        float2 _10823 = float2(_10357.x, _10822);
                                        float2 _16204;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15464 = _10823;
                                            _15464.y = 1.0 - _10822;
                                            _16204 = _15464;
                                        }
                                        else
                                        {
                                            _16204 = _10823;
                                        }
                                        float2 _16206;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15471 = _10369;
                                            _15471.y = 1.0 - _10369.y;
                                            _16206 = _15471;
                                        }
                                        else
                                        {
                                            _16206 = _10369;
                                        }
                                        _16234 = (((gBackdrop4.sample(gLinear, _16201, level(0.0)) * (_10341.x * _10341.y)) + (gBackdrop4.sample(gLinear, _16202, level(0.0)) * (_10345.x * _10341.y))) + (gBackdrop4.sample(gLinear, _16204, level(0.0)) * (_10341.x * _10345.y))) + (gBackdrop4.sample(gLinear, _16206, level(0.0)) * (_10345.x * _10345.y));
                                    }
                                    else
                                    {
                                        float2 _16194;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15478 = _10357;
                                            _15478.y = 1.0 - _10357.y;
                                            _16194 = _15478;
                                        }
                                        else
                                        {
                                            _16194 = _10357;
                                        }
                                        float _10934 = _10357.y;
                                        float2 _10935 = float2(_10369.x, _10934);
                                        float2 _16195;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15485 = _10935;
                                            _15485.y = 1.0 - _10934;
                                            _16195 = _15485;
                                        }
                                        else
                                        {
                                            _16195 = _10935;
                                        }
                                        float _10952 = _10369.y;
                                        float2 _10953 = float2(_10357.x, _10952);
                                        float2 _16197;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15492 = _10953;
                                            _15492.y = 1.0 - _10952;
                                            _16197 = _15492;
                                        }
                                        else
                                        {
                                            _16197 = _10953;
                                        }
                                        float2 _16199;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15499 = _10369;
                                            _15499.y = 1.0 - _10369.y;
                                            _16199 = _15499;
                                        }
                                        else
                                        {
                                            _16199 = _10369;
                                        }
                                        _16234 = (((gBackdrop5.sample(gLinear, _16194, level(0.0)) * (_10341.x * _10341.y)) + (gBackdrop5.sample(gLinear, _16195, level(0.0)) * (_10345.x * _10341.y))) + (gBackdrop5.sample(gLinear, _16197, level(0.0)) * (_10341.x * _10345.y))) + (gBackdrop5.sample(gLinear, _16199, level(0.0)) * (_10345.x * _10345.y));
                                    }
                                    _16233 = _16234;
                                }
                                _16232 = _16233;
                            }
                            _16231 = _16232;
                        }
                        _16230 = _16231;
                    }
                    _10202 = _16263 + (_16230.xyz * (_10173 ? ((_16193 == 0) ? (1.0 - _10168) : _10168) : 1.0));
                }
                float3 _15503 = _16167;
                _15503.x = _16263.x;
                float2 _6846 = (_6786 + _6798) + (_6801 * _6828);
                float _11049 = fast::clamp(log2(fast::max(_6700, 1.0)) - 1.0, 0.0, 5.0);
                int _11052 = int(floor(_11049));
                float _11056 = _11049 - float(_11052);
                bool _11061 = (_11056 > 0.0199999995529651641845703125) && (_11052 < 5);
                float3 _16387;
                _16387 = float3(0.0);
                float3 _11090;
                for (int _16317 = 0; _16317 < 2; _16387 = _11090, _16317++)
                {
                    if ((_16317 > 0) && (!_11061))
                    {
                        break;
                    }
                    int _11076 = _11052 + _16317;
                    int _11114 = clamp(_11076, 1, 5);
                    float2 _11183 = (_6846 * _184.gLevel[_11114].xy) - float2(0.5);
                    float2 _11185 = floor(_11183);
                    float2 _11188 = _11183 - _11185;
                    float2 _11191 = _11188 * _11188;
                    float2 _11194 = _11191 * _11188;
                    float2 _11213 = (((_11194 * 3.0) - (_11191 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                    float2 _11226 = _11194 * 0.16666667163372039794921875;
                    float2 _11229 = (((((-_11194) + (_11191 * 3.0)) - (_11188 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11213;
                    float2 _11233 = (((((_11194 * (-3.0)) + (_11191 * 3.0)) + (_11188 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11226;
                    float2 _11245 = ((_11185 - float2(0.5)) + (_11213 / _11229)) * _184.gLevel[_11114].zw;
                    float2 _11257 = ((_11185 + float2(1.5)) + (_11226 / _11233)) * _184.gLevel[_11114].zw;
                    float4 _16354;
                    if (_11076 <= 0)
                    {
                        float2 _16353;
                        if (_184.gConv.x > 0.5)
                        {
                            float2 _15506 = _6846;
                            _15506.y = 1.0 - _6846.y;
                            _16353 = _15506;
                        }
                        else
                        {
                            _16353 = _6846;
                        }
                        _16354 = gBackdrop0.sample(gLinear, _16353, level(0.0));
                    }
                    else
                    {
                        float4 _16355;
                        if (_11076 == 1)
                        {
                            float2 _16346;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15511 = _11245;
                                _15511.y = 1.0 - _11245.y;
                                _16346 = _15511;
                            }
                            else
                            {
                                _16346 = _11245;
                            }
                            float _11302 = _11245.y;
                            float2 _11303 = float2(_11257.x, _11302);
                            float2 _16347;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15518 = _11303;
                                _15518.y = 1.0 - _11302;
                                _16347 = _15518;
                            }
                            else
                            {
                                _16347 = _11303;
                            }
                            float _11320 = _11257.y;
                            float2 _11321 = float2(_11245.x, _11320);
                            float2 _16349;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15525 = _11321;
                                _15525.y = 1.0 - _11320;
                                _16349 = _15525;
                            }
                            else
                            {
                                _16349 = _11321;
                            }
                            float2 _16351;
                            if (_184.gConv.x > 0.5)
                            {
                                float2 _15532 = _11257;
                                _15532.y = 1.0 - _11257.y;
                                _16351 = _15532;
                            }
                            else
                            {
                                _16351 = _11257;
                            }
                            _16355 = (((gBackdrop1.sample(gLinear, _16346, level(0.0)) * (_11229.x * _11229.y)) + (gBackdrop1.sample(gLinear, _16347, level(0.0)) * (_11233.x * _11229.y))) + (gBackdrop1.sample(gLinear, _16349, level(0.0)) * (_11229.x * _11233.y))) + (gBackdrop1.sample(gLinear, _16351, level(0.0)) * (_11233.x * _11233.y));
                        }
                        else
                        {
                            float4 _16356;
                            if (_11076 == 2)
                            {
                                float2 _16339;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15539 = _11245;
                                    _15539.y = 1.0 - _11245.y;
                                    _16339 = _15539;
                                }
                                else
                                {
                                    _16339 = _11245;
                                }
                                float _11432 = _11245.y;
                                float2 _11433 = float2(_11257.x, _11432);
                                float2 _16340;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15546 = _11433;
                                    _15546.y = 1.0 - _11432;
                                    _16340 = _15546;
                                }
                                else
                                {
                                    _16340 = _11433;
                                }
                                float _11450 = _11257.y;
                                float2 _11451 = float2(_11245.x, _11450);
                                float2 _16342;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15553 = _11451;
                                    _15553.y = 1.0 - _11450;
                                    _16342 = _15553;
                                }
                                else
                                {
                                    _16342 = _11451;
                                }
                                float2 _16344;
                                if (_184.gConv.x > 0.5)
                                {
                                    float2 _15560 = _11257;
                                    _15560.y = 1.0 - _11257.y;
                                    _16344 = _15560;
                                }
                                else
                                {
                                    _16344 = _11257;
                                }
                                _16356 = (((gBackdrop2.sample(gLinear, _16339, level(0.0)) * (_11229.x * _11229.y)) + (gBackdrop2.sample(gLinear, _16340, level(0.0)) * (_11233.x * _11229.y))) + (gBackdrop2.sample(gLinear, _16342, level(0.0)) * (_11229.x * _11233.y))) + (gBackdrop2.sample(gLinear, _16344, level(0.0)) * (_11233.x * _11233.y));
                            }
                            else
                            {
                                float4 _16357;
                                if (_11076 == 3)
                                {
                                    float2 _16332;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15567 = _11245;
                                        _15567.y = 1.0 - _11245.y;
                                        _16332 = _15567;
                                    }
                                    else
                                    {
                                        _16332 = _11245;
                                    }
                                    float _11562 = _11245.y;
                                    float2 _11563 = float2(_11257.x, _11562);
                                    float2 _16333;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15574 = _11563;
                                        _15574.y = 1.0 - _11562;
                                        _16333 = _15574;
                                    }
                                    else
                                    {
                                        _16333 = _11563;
                                    }
                                    float _11580 = _11257.y;
                                    float2 _11581 = float2(_11245.x, _11580);
                                    float2 _16335;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15581 = _11581;
                                        _15581.y = 1.0 - _11580;
                                        _16335 = _15581;
                                    }
                                    else
                                    {
                                        _16335 = _11581;
                                    }
                                    float2 _16337;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        float2 _15588 = _11257;
                                        _15588.y = 1.0 - _11257.y;
                                        _16337 = _15588;
                                    }
                                    else
                                    {
                                        _16337 = _11257;
                                    }
                                    _16357 = (((gBackdrop3.sample(gLinear, _16332, level(0.0)) * (_11229.x * _11229.y)) + (gBackdrop3.sample(gLinear, _16333, level(0.0)) * (_11233.x * _11229.y))) + (gBackdrop3.sample(gLinear, _16335, level(0.0)) * (_11229.x * _11233.y))) + (gBackdrop3.sample(gLinear, _16337, level(0.0)) * (_11233.x * _11233.y));
                                }
                                else
                                {
                                    float4 _16358;
                                    if (_11076 == 4)
                                    {
                                        float2 _16325;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15595 = _11245;
                                            _15595.y = 1.0 - _11245.y;
                                            _16325 = _15595;
                                        }
                                        else
                                        {
                                            _16325 = _11245;
                                        }
                                        float _11692 = _11245.y;
                                        float2 _11693 = float2(_11257.x, _11692);
                                        float2 _16326;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15602 = _11693;
                                            _15602.y = 1.0 - _11692;
                                            _16326 = _15602;
                                        }
                                        else
                                        {
                                            _16326 = _11693;
                                        }
                                        float _11710 = _11257.y;
                                        float2 _11711 = float2(_11245.x, _11710);
                                        float2 _16328;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15609 = _11711;
                                            _15609.y = 1.0 - _11710;
                                            _16328 = _15609;
                                        }
                                        else
                                        {
                                            _16328 = _11711;
                                        }
                                        float2 _16330;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15616 = _11257;
                                            _15616.y = 1.0 - _11257.y;
                                            _16330 = _15616;
                                        }
                                        else
                                        {
                                            _16330 = _11257;
                                        }
                                        _16358 = (((gBackdrop4.sample(gLinear, _16325, level(0.0)) * (_11229.x * _11229.y)) + (gBackdrop4.sample(gLinear, _16326, level(0.0)) * (_11233.x * _11229.y))) + (gBackdrop4.sample(gLinear, _16328, level(0.0)) * (_11229.x * _11233.y))) + (gBackdrop4.sample(gLinear, _16330, level(0.0)) * (_11233.x * _11233.y));
                                    }
                                    else
                                    {
                                        float2 _16318;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15623 = _11245;
                                            _15623.y = 1.0 - _11245.y;
                                            _16318 = _15623;
                                        }
                                        else
                                        {
                                            _16318 = _11245;
                                        }
                                        float _11822 = _11245.y;
                                        float2 _11823 = float2(_11257.x, _11822);
                                        float2 _16319;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15630 = _11823;
                                            _15630.y = 1.0 - _11822;
                                            _16319 = _15630;
                                        }
                                        else
                                        {
                                            _16319 = _11823;
                                        }
                                        float _11840 = _11257.y;
                                        float2 _11841 = float2(_11245.x, _11840);
                                        float2 _16321;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15637 = _11841;
                                            _15637.y = 1.0 - _11840;
                                            _16321 = _15637;
                                        }
                                        else
                                        {
                                            _16321 = _11841;
                                        }
                                        float2 _16323;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            float2 _15644 = _11257;
                                            _15644.y = 1.0 - _11257.y;
                                            _16323 = _15644;
                                        }
                                        else
                                        {
                                            _16323 = _11257;
                                        }
                                        _16358 = (((gBackdrop5.sample(gLinear, _16318, level(0.0)) * (_11229.x * _11229.y)) + (gBackdrop5.sample(gLinear, _16319, level(0.0)) * (_11233.x * _11229.y))) + (gBackdrop5.sample(gLinear, _16321, level(0.0)) * (_11229.x * _11233.y))) + (gBackdrop5.sample(gLinear, _16323, level(0.0)) * (_11233.x * _11233.y));
                                    }
                                    _16357 = _16358;
                                }
                                _16356 = _16357;
                            }
                            _16355 = _16356;
                        }
                        _16354 = _16355;
                    }
                    _11090 = _16387 + (_16354.xyz * (_11061 ? ((_16317 == 0) ? (1.0 - _11056) : _11056) : 1.0));
                }
                _15503.z = _16387.z;
                _16557 = _15503;
            }
            else
            {
                _16557 = _16167;
            }
            float3 _16777;
            if (_6728 > 0.0)
            {
                float2 _6869 = _6786 + (((_16095 * fast::min(_6719 * 0.5, 16.0)) * _184.gDisplay.zw) * _184.gTarget.zw);
                float _11937 = fast::clamp(log2(fast::max(16.0 * _184.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _11940 = int(floor(_11937));
                float _11944 = _11937 - float(_11940);
                bool _11949 = (_11944 > 0.0199999995529651641845703125) && (_11940 < 5);
                float3 _16542;
                _16542 = float3(0.0);
                float3 _11978;
                for (int _16527 = 0; _16527 < 2; _16542 = _11978, _16527++)
                {
                    if ((_16527 > 0) && (!_11949))
                    {
                        break;
                    }
                    int _11964 = _11940 + _16527;
                    float2 _16528;
                    if (_184.gConv.x > 0.5)
                    {
                        float2 _15651 = _6869;
                        _15651.y = 1.0 - _6869.y;
                        _16528 = _15651;
                    }
                    else
                    {
                        _16528 = _6869;
                    }
                    float4 _16529;
                    if (_11964 <= 0)
                    {
                        _16529 = gBackdrop0.sample(gLinear, _16528, level(0.0));
                    }
                    else
                    {
                        float4 _16530;
                        if (_11964 == 1)
                        {
                            _16530 = gBackdrop1.sample(gLinear, _16528, level(0.0));
                        }
                        else
                        {
                            float4 _16531;
                            if (_11964 == 2)
                            {
                                _16531 = gBackdrop2.sample(gLinear, _16528, level(0.0));
                            }
                            else
                            {
                                float4 _16532;
                                if (_11964 == 3)
                                {
                                    _16532 = gBackdrop3.sample(gLinear, _16528, level(0.0));
                                }
                                else
                                {
                                    float4 _16533;
                                    if (_11964 == 4)
                                    {
                                        _16533 = gBackdrop4.sample(gLinear, _16528, level(0.0));
                                    }
                                    else
                                    {
                                        _16533 = gBackdrop5.sample(gLinear, _16528, level(0.0));
                                    }
                                    _16532 = _16533;
                                }
                                _16531 = _16532;
                            }
                            _16530 = _16531;
                        }
                        _16529 = _16530;
                    }
                    _11978 = _16542 + (_16529.xyz * (_11949 ? ((_16527 == 0) ? (1.0 - _11944) : _11944) : 1.0));
                }
                _16777 = _16542;
            }
            else
            {
                _16777 = float3(0.5);
            }
            float _16573;
            if (in.i_shape.x > 0.001000000047497451305389404296875)
            {
                int _12067 = clamp(int(rint(log2(36.0 * _184.gDisplay.z) - 1.0)), 1, 4);
                float2 _16548;
                if (_184.gConv.x > 0.5)
                {
                    float2 _15655 = _6786;
                    _15655.y = 1.0 - _6786.y;
                    _16548 = _15655;
                }
                else
                {
                    _16548 = _6786;
                }
                float4 _16549;
                if (_12067 <= 0)
                {
                    _16549 = gBackdrop0.sample(gLinear, _16548, level(0.0));
                }
                else
                {
                    float4 _16550;
                    if (_12067 == 1)
                    {
                        _16550 = gBackdrop1.sample(gLinear, _16548, level(0.0));
                    }
                    else
                    {
                        float4 _16551;
                        if (_12067 == 2)
                        {
                            _16551 = gBackdrop2.sample(gLinear, _16548, level(0.0));
                        }
                        else
                        {
                            float4 _16552;
                            if (_12067 == 3)
                            {
                                _16552 = gBackdrop3.sample(gLinear, _16548, level(0.0));
                            }
                            else
                            {
                                float4 _16553;
                                if (_12067 == 4)
                                {
                                    _16553 = gBackdrop4.sample(gLinear, _16548, level(0.0));
                                }
                                else
                                {
                                    _16553 = gBackdrop5.sample(gLinear, _16548, level(0.0));
                                }
                                _16552 = _16553;
                            }
                            _16551 = _16552;
                        }
                        _16550 = _16551;
                    }
                    _16549 = _16550;
                }
                _16573 = dot(_16549.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _16573 = 0.5;
            }
            _16775 = _16777;
            _16572 = _16573;
            _16554 = _16557;
        }
        else
        {
            _16775 = float3(0.5);
            _16572 = 0.5;
            _16554 = float3(0.5);
        }
        float3 _6897 = mix(float3(dot(_16554, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _16554, float3(gFxData_1._data[_6620].x)) + float3(gFxData_1._data[_6620].y);
        float3 _16668;
        if (in.i_shape.x > 0.001000000047497451305389404296875)
        {
            float _6907 = fast::clamp(fast::max(_16572 + gFxData_1._data[_6620].y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _6915 = mix(_6907, dot(gFxData_1._data[_6612].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), in.i_shape.x);
            _16668 = select(mix(_6897, float3(1.0), float3((_6915 - _6907) / fast::max(1.0 - _6907, 0.001000000047497451305389404296875))), _6897 * (_6915 / _6907), bool3(_6915 < _6907));
        }
        else
        {
            _16668 = _6897;
        }
        float2 _6953 = fast::clamp((in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875)), float2(0.0), float2(1.0));
        float2 _7001 = float2(cos(gFxData_1._data[_6620].w), sin(gFxData_1._data[_6620].w));
        float _7004 = dot(_16095, _7001);
        float _7015 = -_15908;
        float _7022 = _4056 * ((0.5 * abs(_16095.x)) + 0.0199999995529651641845703125);
        float _7029 = _4056 * ((0.5 * abs(_16095.y)) + 0.0199999995529651641845703125);
        float _16887;
        if ((((_7015 + _7022) + _7029) > 0.0) && (((_7015 - _7022) - _7029) < (0.5 * _6719)))
        {
            float _12171 = fast::max((_7015 + _7022) + _7029, 0.0) / _6719;
            float _12175 = fast::clamp(sqrt(2.0 * _12171), 0.12399999797344207763671875, 1.0);
            float _12177 = 1.0 - _12175;
            float _12180 = _12177 * _12177;
            float _12234 = fast::max((_7015 + _7022) - _7029, 0.0) / _6719;
            float _12238 = fast::clamp(sqrt(2.0 * _12234), 0.12399999797344207763671875, 1.0);
            float _12240 = 1.0 - _12238;
            float _12243 = _12240 * _12240;
            float _12297 = fast::max((_7015 - _7022) + _7029, 0.0) / _6719;
            float _12301 = fast::clamp(sqrt(2.0 * _12297), 0.12399999797344207763671875, 1.0);
            float _12303 = 1.0 - _12301;
            float _12306 = _12303 * _12303;
            float _12360 = fast::max((_7015 - _7022) - _7029, 0.0) / _6719;
            float _12364 = fast::clamp(sqrt(2.0 * _12360), 0.12399999797344207763671875, 1.0);
            float _12366 = 1.0 - _12364;
            float _12369 = _12366 * _12366;
            float _7076 = ((((((_12171 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _12171) * _12171) : ((_12171 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _12175) * _12175)) + ((((_12180 * _12180) * _12180) * _12177) * (0.02380952425301074981689453125 + (_12177 * ((-0.0386904776096343994140625) + (_12177 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_12171 - 0.5))))) * _6719) * _6719) - ((((_12234 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _12234) * _12234) : ((_12234 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _12238) * _12238)) + ((((_12243 * _12243) * _12243) * _12240) * (0.02380952425301074981689453125 + (_12240 * ((-0.0386904776096343994140625) + (_12240 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_12234 - 0.5))))) * _6719) * _6719)) - ((((_12297 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _12297) * _12297) : ((_12297 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _12301) * _12301)) + ((((_12306 * _12306) * _12306) * _12303) * (0.02380952425301074981689453125 + (_12303 * ((-0.0386904776096343994140625) + (_12303 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_12297 - 0.5))))) * _6719) * _6719)) + ((((_12360 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _12360) * _12360) : ((_12360 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _12364) * _12364)) + ((((_12369 * _12369) * _12369) * _12366) * (0.02380952425301074981689453125 + (_12366 * ((-0.0386904776096343994140625) + (_12366 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_12360 - 0.5))))) * _6719) * _6719);
            _16887 = _7076 / ((4.0 * _7022) * _7029);
        }
        else
        {
            _16887 = 0.0;
        }
        float _7108 = fast::clamp(0.5 + (0.5 * dot((in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5)) / _6711, _7001)), 0.0, 1.0);
        _17799 = ((_16887 * (pow(fast::clamp(_7004, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(fast::clamp(-_7004, 0.0, 1.0), 1.5)))) * gFxData_1._data[_6620].z) * 1.60000002384185791015625;
        _17418 = gFxData_1._data[_6628];
        _17405 = float4((mix(mix(_16668, gFxData_1._data[_6612].xyz, float3(fast::clamp(gFxData_1._data[_6612].w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _6734))) + (0.3499999940395355224609375 * ((dot(gFxData_1._data[_6612].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _6953.y) : _6953.y))), 0.0, 1.0))), (_16775 * 1.10000002384185791015625) + float3(0.07999999821186065673828125), float3(pow(1.0 - _16672.z, 5.0) * 0.3499999940395355224609375)) + float3((((0.039999999105930328369140625 * _7108) * _7108) + (0.0500000007450580596923828125 * (1.0 - _6734))) * gFxData_1._data[_6620].z)) * _4081, _4081) + (_16894 * (1.0 - _4081));
    }
    else
    {
        _17799 = 0.0;
        _17418 = _15888;
        _17405 = _16894;
    }
    float4 _17415;
    if ((in.i_flags.x & 1u) != 0u)
    {
        float4 _17036;
        float4 _17214;
        if (in.i_flags.y != 0u)
        {
            _17214 = gFxData_1._data[(in.i_instance * 24u) + 3u];
            _17036 = gFxData_1._data[(in.i_instance * 24u) + 4u];
        }
        else
        {
            _17214 = float4(0.0);
            _17036 = float4(0.0);
        }
        float4 _17400;
        do
        {
            if (in.i_flags.y == 0u)
            {
                _17400 = in.i_fill0;
                break;
            }
            float2 _12473 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float2 _12483 = fwidth(in.i_local);
            float _12485 = fast::max(length(_12483), 9.9999997473787516355514526367188e-05);
            float _17392;
            float _17396;
            if ((in.i_flags.y == 1u) || (in.i_flags.y == 4u))
            {
                float _12494 = cos(_17036.x);
                float _12497 = sin(_17036.x);
                float _12523 = ((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_12494, _12497)) / fast::max(0.5 * ((abs(_12494) * _12473.x) + (abs(_12497) * _12473.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (in.i_flags.y == 4u)
                {
                    float _12538 = fast::clamp((_12523 - _17036.y) / fast::max(_17036.z - _17036.y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _17400 = float4(fast::clamp(abs((fract(float3(_12538 * 0.800000011920928955078125) + float3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - float3(3.0)) - float3(1.0), float3(0.0), float3(1.0)), in.i_fill0.w * pow(fast::max(sin(_12538 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _17396 = -1.0;
                _17392 = _12523;
            }
            else
            {
                float _17393;
                float _17397;
                if (in.i_flags.y == 2u)
                {
                    _17397 = -1.0;
                    _17393 = length(in.i_local - (in.i_rect.xy + (_17036.xy * _12473))) / fast::max(_17036.z * fast::max(_12473.x, _12473.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    float2 _12606 = in.i_local - (in.i_rect.xy + (_17036.xy * _12473));
                    float _12617 = fract(((precise::atan2(_12606.y, _12606.x) - _17036.z) * 0.15915493667125701904296875) + 1.0);
                    float _17394;
                    float _17398;
                    if (_17036.w > 0.5)
                    {
                        _17398 = -1.0;
                        _17394 = 0.5 - (0.5 * cos(_12617 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _12637 = (((_12617 < 0.5) ? _12617 : (_12617 - 1.0)) * 6.283185482025146484375) * length(_12606);
                        float _17399;
                        if (abs(_12637) < _12485)
                        {
                            _17399 = fast::clamp(((_12637 / _12485) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _17399 = -1.0;
                        }
                        _17398 = _17399;
                        _17394 = _12617;
                    }
                    _17397 = _17398;
                    _17393 = _17394;
                }
                _17396 = _17397;
                _17392 = _17393;
            }
            float4 _12666 = float4(in.i_fill0.xyz * in.i_fill0.w, in.i_fill0.w);
            float4 _12678 = float4(_17214.xyz * _17214.w, _17214.w);
            float4 _12692 = select(mix(_12666, _12678, float4(fast::clamp(_17392, 0.0, 1.0))), mix(_12678, _12666, float4(_17396)), bool4(_17396 >= 0.0));
            _17400 = select(float4(0.0), float4(_12692.xyz / float3(_12692.w), _12692.w), bool4(_12692.w > 9.9999997473787516355514526367188e-06));
            break;
        } while(false);
        float4 _17401;
        if ((in.i_flags.x & 64u) != 0u)
        {
            uint _12717 = (in.i_instance * 24u) + 18u;
            _17401 = _17400 * gTex.sample(gLinear, mix(gFxData_1._data[_12717].xy, gFxData_1._data[_12717].zw, (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875))));
        }
        else
        {
            _17401 = _17400;
        }
        float _12727 = fast::clamp(_17401.w * _4081, 0.0, 1.0);
        _17415 = float4(_17401.xyz * _12727, _12727) + (_17405 * (1.0 - _12727));
    }
    else
    {
        _17415 = _17405;
    }
    float4 _17785;
    if ((in.i_flags.x & 16384u) != 0u)
    {
        uint _12751 = (in.i_instance * 24u) + 21u;
        uint _12759 = (in.i_instance * 24u) + 3u;
        float2 _4521 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float2 _4528 = (in.i_local - in.i_rect.xy) / _4521;
        float _4539 = ((gFxData_1._data[_12759].x >= 0.0) ? gFxData_1._data[_12759].x : _184.gTime.x) * gFxData_1._data[_12751].z;
        float _4542 = _4528.x * 2.0;
        float _4543 = _4542 - 1.0;
        float _4548 = _4528.y * _4521.y;
        float _4551 = fast::max(gFxData_1._data[_12751].w, 0.001000000047497451305389404296875);
        float _4556 = fast::clamp(1.0 - (_4543 * _4543), 0.0, 1.0);
        float _4558 = pow(_4556, 1.2999999523162841796875);
        float _4560 = pow(_4556, 0.699999988079071044921875);
        float _4563 = fast::clamp(_4539 * 1.4285714626312255859375, 0.0, 1.0);
        float _4579 = (0.25 + (0.75 * ((_4563 * _4563) * (3.0 - (2.0 * _4563))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_4539 * 2.099999904632568359375)));
        float _4588 = ((gFxData_1._data[_12751].y * _4521.y) * _4579) * _4558;
        float _4614 = (_4521.y * (0.5 + ((gFxData_1._data[_12751].x * (0.5 - (_4543 * _4543))) * 0.5))) + (((0.14000000059604644775390625 * _4521.y) * _4558) * sin(((_4543 * 2.400000095367431640625) - (_4539 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _17411;
        float _17412;
        float3 _17413;
        _17413 = float3(0.0);
        _17412 = _4614;
        _17411 = _4614;
        float3 _4686;
        float _18047;
        float _18048;
        for (int _17410 = 0; _17410 < 4; _17413 = _4686, _17412 = _18048, _17411 = _18047, _17410++)
        {
            float _4638 = _4614 + ((_4588 * _3087[_17410].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_4539 * 1.7000000476837158203125) + _3104[_17410].y))));
            _18047 = (_17410 == 0) ? _4638 : _17411;
            _18048 = (_17410 == 2) ? _4638 : _17412;
            float _4653 = _4551 * _3087[_17410].y;
            float _4658 = (_4548 - _4638) / _4653;
            float _4663 = _4551 * _3087[_17410].z;
            float3 _18021;
            _18021 = float3(0.0);
            for (int _18020 = 0; _18020 < 6; )
            {
                float _12784 = ((_4548 - _4638) - (_4663 * ((float(_18020) * 0.4000000059604644775390625) - 1.0))) / _4653;
                _18021 += (_1724[_18020] * exp((-_12784) * _12784));
                _18020++;
                continue;
            }
            _4686 = _17413 + (mix(_18021 * float3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), float3(exp((-_4658) * _4658)), float3(_3104[_17410].x)) * (_3087[_17410].w * _4560));
        }
        float _4692 = _4551 * 1.5;
        float _4722 = fast::clamp((_4548 - _17411) / fast::max(_17412 - _17411, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _4750 = (_4548 - (_17412 - (_4551 * 3.0))) / (((_4521.y * 0.0900000035762786865234375) + (_4588 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _4753 = (_4542 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _4778 = ((_4548 - _17411) + (_4551 * 5.0)) / (_4551 * 7.0);
        float3 _4804 = float3(1.0) - exp((-((((_17413 + (mix(float3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), float3(1.0), float3(_4722)) * ((((1.0 / (1.0 + exp((-((_4548 - _17411) - (_4551 * 2.0))) / _4692))) / (1.0 + exp((-(_17412 - _4548)) / _4692))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_4722, 2.5)))) * _4560))) + (float3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_4750) * _4750) - (_4753 * _4753)) * (0.5 + (1.10000002384185791015625 * _4579))))) + (float3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_4778) * _4778) * _4558) * 0.100000001490116119384765625))) * mix(float3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), float3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), float3(0.5 + (0.5 * sin(_4539 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _4814 = (in.i_fill0.w * smoothstep(0.0, 0.119999997317790985107421875, _4528.y)) * smoothstep(1.0, 0.87999999523162841796875, _4528.y);
        float _4833 = (fast::clamp(fast::max(_4804.x, fast::max(_4804.y, _4804.z)), 0.0, 1.0) * _4814) * _4081;
        _17785 = float4((_4804 * _4814) * _4081, _4833) + (_17415 * (1.0 - _4833));
    }
    else
    {
        _17785 = _17415;
    }
    float4 _17794;
    if (((in.i_flags.x & 4u) != 0u) && ((in.i_flags.x & 256u) != 0u))
    {
        uint _12816 = (in.i_instance * 24u) + 7u;
        uint _12824 = (in.i_instance * 24u) + 8u;
        float2 _4867 = in.i_local - gFxData_1._data[_12824].zw;
        float2 _12865 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _12874 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _17760;
        if (in.i_flags.z == 1u)
        {
            float2 _12880 = _4867 - _12865;
            float _17759;
            do
            {
                if (in.i_shape.w >= 6.282185077667236328125)
                {
                    _17759 = abs(length(_12880) - in.i_radii.x) - in.i_radii.y;
                    break;
                }
                float _12979 = in.i_shape.z + (in.i_shape.w * 0.5);
                float _12981 = cos(_12979);
                float _12983 = sin(_12979);
                float _12992 = dot(_12880, float2(-_12983, _12981));
                float _12995 = dot(_12880, float2(_12981, _12983));
                float2 _12996 = float2(_12992, _12995);
                float _12999 = abs(_12992);
                _12996.x = _12999;
                float _13002 = in.i_shape.w * 0.5;
                float _13004 = sin(_13002);
                float _13006 = cos(_13002);
                _17759 = (((_13006 * _12999) > (_13004 * _12995)) ? length(_12996 - (float2(_13004, _13006) * in.i_radii.x)) : abs(length(_12996) - in.i_radii.x)) - in.i_radii.y;
                break;
            } while(false);
            _17760 = _17759;
        }
        else
        {
            float _17761;
            if (in.i_flags.z == 2u)
            {
                float2 _13042 = _4867 - _15890.xy;
                float2 _13045 = _15890.zw - _15890.xy;
                _17761 = length(_13042 - (_13045 * fast::clamp(dot(_13042, _13045) / fast::max(dot(_13045, _13045), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
            }
            else
            {
                float2 _12907 = _4867 - _12865;
                float _13098 = fast::min(_12874.x, _12874.y);
                float _13101 = fast::min((_12907.x > 0.0) ? ((_12907.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_12907.y > 0.0) ? in.i_radii.w : in.i_radii.x), _13098);
                float _13107 = _13101 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                float _17744;
                float _17745;
                if (_13107 > _13098)
                {
                    float _13121 = in.i_shape.y * fast::clamp((_13098 - _13101) / fast::max(0.60000002384185791015625 * _13101, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _17745 = _13121;
                    _17744 = _13101 * (1.0 + (0.60000002384185791015625 * _13121));
                }
                else
                {
                    _17745 = in.i_shape.y;
                    _17744 = _13107;
                }
                float2 _13134 = (abs(_12907) - _12874) + float2(_17744);
                float2 _13136 = fast::max(_13134, float2(0.0));
                float _17746;
                if ((_13136.x > 0.0) && (_13136.y > 0.0))
                {
                    float _17747;
                    if ((_17745 > 0.001000000047497451305389404296875) && (_17744 > 9.9999997473787516355514526367188e-05))
                    {
                        float _13153 = 2.0 + (2.0 * _17745);
                        float2 _13158 = _13136 / float2(fast::max(_17744, 9.9999997473787516355514526367188e-05));
                        _17747 = pow(pow(_13158.x, _13153) + pow(_13158.y, _13153), 1.0 / _13153) * _17744;
                    }
                    else
                    {
                        _17747 = length(_13136);
                    }
                    _17746 = _17747;
                }
                else
                {
                    _17746 = fast::max(_13136.x, _13136.y);
                }
                float _13193 = (fast::min(fast::max(_13134.x, _13134.y), 0.0) + _17746) - _17744;
                float _17762;
                if ((in.i_flags.x & 512u) != 0u)
                {
                    float2 _12935 = fast::max((_15890.zw - _15890.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _12938 = _4867 - ((_15890.xy + _15890.zw) * 0.5);
                    float _13229 = fast::min(_12935.x, _12935.y);
                    float _13232 = fast::min((_12938.x > 0.0) ? ((_12938.y > 0.0) ? _17418.x : _17418.x) : ((_12938.y > 0.0) ? _17418.x : _17418.x), _13229);
                    float _13238 = _13232 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _17750;
                    float _17751;
                    if (_13238 > _13229)
                    {
                        float _13252 = in.i_shape.y * fast::clamp((_13229 - _13232) / fast::max(0.60000002384185791015625 * _13232, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _17751 = _13252;
                        _17750 = _13232 * (1.0 + (0.60000002384185791015625 * _13252));
                    }
                    else
                    {
                        _17751 = in.i_shape.y;
                        _17750 = _13238;
                    }
                    float2 _13265 = (abs(_12938) - _12935) + float2(_17750);
                    float2 _13267 = fast::max(_13265, float2(0.0));
                    float _17752;
                    if ((_13267.x > 0.0) && (_13267.y > 0.0))
                    {
                        float _17753;
                        if ((_17751 > 0.001000000047497451305389404296875) && (_17750 > 9.9999997473787516355514526367188e-05))
                        {
                            float _13284 = 2.0 + (2.0 * _17751);
                            float2 _13289 = _13267 / float2(fast::max(_17750, 9.9999997473787516355514526367188e-05));
                            _17753 = pow(pow(_13289.x, _13284) + pow(_13289.y, _13284), 1.0 / _13284) * _17750;
                        }
                        else
                        {
                            _17753 = length(_13267);
                        }
                        _17752 = _17753;
                    }
                    else
                    {
                        _17752 = fast::max(_13267.x, _13267.y);
                    }
                    float _13324 = (fast::min(fast::max(_13265.x, _13265.y), 0.0) + _17752) - _17750;
                    float _13329 = fast::max(_17418.y, 9.9999997473787516355514526367188e-05);
                    float _13338 = fast::max(_13329 - abs(_13193 - _13324), 0.0) / _13329;
                    _17762 = fast::min(_13193, _13324) - (((_13338 * _13338) * _13329) * 0.25);
                }
                else
                {
                    _17762 = _13193;
                }
                _17761 = _17762;
            }
            _17760 = _17761;
        }
        float _4876 = (_17760 + gFxData_1._data[_12824].y) / (fast::max(gFxData_1._data[_12824].x * 0.5, _4056 * 0.5) * 1.41421353816986083984375);
        float _13355 = sign(_4876);
        float _13357 = abs(_4876);
        float _13368 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_13357 * _13357))) * _13357)) * _13357);
        float _13371 = _13368 * _13368;
        float _13386 = fast::clamp(gFxData_1._data[_12816].w * ((0.5 + (0.5 * (_13355 - (_13355 / (_13371 * _13371))))) * _4081), 0.0, 1.0);
        _17794 = float4(gFxData_1._data[_12816].xyz * _13386, _13386) + (_17785 * (1.0 - _13386));
    }
    else
    {
        _17794 = _17785;
    }
    float4 _17818;
    if ((in.i_flags.x & 16u) != 0u)
    {
        uint _13410 = (in.i_instance * 24u) + 9u;
        uint _13418 = (in.i_instance * 24u) + 10u;
        float _4908 = fast::max(-_15908, 0.0) / fast::max(gFxData_1._data[_13418].z, 0.001000000047497451305389404296875);
        float _13428 = fast::clamp(gFxData_1._data[_13410].w * fast::clamp((exp(((-_4908) * _4908) * 2.2000000476837158203125) * gFxData_1._data[_13418].w) * _4081, 0.0, 1.0), 0.0, 1.0);
        _17818 = float4(gFxData_1._data[_13410].xyz * _13428, _13428) + (_17794 * (1.0 - _13428));
    }
    else
    {
        _17818 = _17794;
    }
    float3 _4939 = _17818.xyz + float3(_17799 * fast::clamp(_17818.w / fast::max(_4081, 0.001000000047497451305389404296875), 0.0, 1.0));
    float4 _15783 = _17818;
    _15783.x = _4939.x;
    _15783.y = _4939.y;
    _15783.z = _4939.z;
    float4 _17821;
    if ((in.i_flags.x & 2u) != 0u)
    {
        uint _13452 = (in.i_instance * 24u) + 5u;
        float4 _13454 = gFxData_1._data[_13452];
        uint _13460 = (in.i_instance * 24u) + 6u;
        float4 _17819;
        if (gFxData_1._data[_13460].z < 0.999000012874603271484375)
        {
            float2 _4997 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float _5008 = cos(gFxData_1._data[_13460].w);
            float _5011 = sin(gFxData_1._data[_13460].w);
            float4 _15800 = _13454;
            _15800.w = _13454.w * mix(1.0, gFxData_1._data[_13460].z, fast::clamp(((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_5008, _5011)) / fast::max(0.5 * ((abs(_5008) * _4997.x) + (abs(_5011) * _4997.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _17819 = _15800;
        }
        else
        {
            _17819 = _13454;
        }
        float _13470 = fast::clamp(_17819.w * (fast::clamp(0.5 - ((_15908 - (gFxData_1._data[_13460].x * gFxData_1._data[_13460].y)) / _4056), 0.0, 1.0) - fast::clamp(0.5 - ((_15908 + (gFxData_1._data[_13460].x * (1.0 - gFxData_1._data[_13460].y))) / _4056), 0.0, 1.0)), 0.0, 1.0);
        _17821 = float4(_17819.xyz * _13470, _13470) + (_15783 * (1.0 - _13470));
    }
    else
    {
        _17821 = _15783;
    }
    float4 _17822;
    if ((in.i_flags.x & 128u) != 0u)
    {
        float2 _5072 = (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float _5094 = exp(-pow((((_5072.x * 0.85000002384185791015625) + (_5072.y * 0.1500000059604644775390625)) - ((fract(_184.gTime.x * in.i_misc.w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _17822 = float4(_17821.xyz + float3(((_5094 * in.i_misc.z) * _4081) * fast::max(_17821.w, 0.3499999940395355224609375)), fast::max(_17821.w, ((_5094 * in.i_misc.z) * _4081) * 0.5));
    }
    else
    {
        _17822 = _17821;
    }
    float4 _18015;
    if ((in.i_flags.x & 2048u) != 0u)
    {
        float3 _13495 = fract(floor(_15887).xyx * 0.103100001811981201171875);
        float3 _13504 = _13495 + float3(dot(_13495, _13495.yzx + float3(33.3300018310546875)));
        float3 _5145 = _17822.xyz + float3(((fract((_13504.x + _13504.y) * _13504.z) - 0.5) * in.i_misc.y) * _17822.w);
        float4 _15824 = _17822;
        _15824.x = _5145.x;
        _15824.y = _5145.y;
        _15824.z = _5145.z;
        _18015 = _15824;
    }
    else
    {
        _18015 = _17822;
    }
    float _18011;
    if (_227.gFade.z > 0.0)
    {
        _18011 = smoothstep(0.0, 1.0, fast::clamp((_15887.y - _227.gFade.x) / _227.gFade.z, 0.0, 1.0));
    }
    else
    {
        _18011 = 1.0;
    }
    float _18012;
    if (_227.gFade.w > 0.0)
    {
        _18012 = _18011 * smoothstep(0.0, 1.0, fast::clamp((_227.gFade.y - _15887.y) / _227.gFade.w, 0.0, 1.0));
    }
    else
    {
        _18012 = _18011;
    }
    float4 _5162 = _18015 * ((in.i_misc.x * _17833) * _18012);
    float4 _18016;
    if (((in.i_flags.x & 8u) != 0u) || ((in.i_flags.x & 4u) != 0u))
    {
        float3 _13556 = fract((floor(_15887) + float2(17.0)).xyx * 0.103100001811981201171875);
        float3 _13565 = _13556 + float3(dot(_13556, _13556.yzx + float3(33.3300018310546875)));
        float3 _5186 = _5162.xyz + float3(((fract((_13565.x + _13565.y) * _13565.z) - 0.5) * 0.0039215688593685626983642578125) * fast::clamp(_5162.w * 8.0, 0.0, 1.0));
        float4 _15836 = _5162;
        _15836.x = _5186.x;
        _15836.y = _5186.y;
        _15836.z = _5186.z;
        _18016 = _15836;
    }
    else
    {
        _18016 = _5162;
    }
    float3 _5196 = fast::max(_18016.xyz, float3(0.0));
    float4 _15842 = _18016;
    _15842.x = _5196.x;
    _15842.y = _5196.y;
    _15842.z = _5196.z;
    float4 _18017;
    if ((_184.gTime.w > 0.5) && (_18016.w > 9.9999997473787516355514526367188e-06))
    {
        float3 _13609 = fast::clamp(_15842.xyz / float3(_18016.w), float3(0.0), float3(1.0));
        float3 _13595 = select(pow((_13609 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _13609 * float3(0.077399380505084991455078125), _13609 <= float3(0.040449999272823333740234375)) * _18016.w;
        float4 _15851 = _15842;
        _15851.x = _13595.x;
        _15851.y = _13595.y;
        _15851.z = _13595.z;
        _18017 = _15851;
    }
    else
    {
        _18017 = _15842;
    }
    out._entryPointOutput = _18017;
    return out;
}

