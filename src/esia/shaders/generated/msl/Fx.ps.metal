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

constant spvUnsafeArray<float3, 6> _1758 = spvUnsafeArray<float3, 6>({ float3(1.0, 0.4199999868869781494140625, 0.2199999988079071044921875), float3(1.0, 0.699999988079071044921875, 0.300000011920928955078125), float3(0.800000011920928955078125, 0.920000016689300537109375, 0.4000000059604644775390625), float3(0.3499999940395355224609375, 0.89999997615814208984375, 0.699999988079071044921875), float3(0.4000000059604644775390625, 0.62000000476837158203125, 1.0), float3(0.660000026226043701171875, 0.5, 1.0) });
constant spvUnsafeArray<float4, 4> _2841 = spvUnsafeArray<float4, 4>({ float4(-1.0, 1.0, 3.400000095367431640625, 2.599999904632568359375), float4(-0.550000011920928955078125, 0.800000011920928955078125, 2.0, 0.800000011920928955078125), float4(0.300000011920928955078125, 1.0, 1.2000000476837158203125, 1.2999999523162841796875), float4(0.62000000476837158203125, 0.800000011920928955078125, 1.60000002384185791015625, 0.449999988079071044921875) });
constant spvUnsafeArray<float2, 4> _2858 = spvUnsafeArray<float2, 4>({ float2(0.0), float2(0.100000001490116119384765625, 1.2999999523162841796875), float2(0.550000011920928955078125, 3.900000095367431640625), float2(0.20000000298023223876953125, 5.19999980926513671875) });

struct esia_main_out
{
    float4 _entryPointOutput [[color(0)]];
};

struct esia_main_in
{
    float2 i_local [[user(locn0)]];
    uint i_instance [[user(locn1)]];
};

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _172 [[buffer(0)]], constant WgtDraw& _215 [[buffer(2)]], const device gFxData& gFxData_1 [[buffer(10)]], texture2d<float> gTex [[texture(3)]], texture2d<float> gBackdrop0 [[texture(4)]], texture2d<float> gBackdrop1 [[texture(5)]], texture2d<float> gBackdrop2 [[texture(6)]], texture2d<float> gBackdrop3 [[texture(7)]], texture2d<float> gBackdrop4 [[texture(8)]], texture2d<float> gBackdrop5 [[texture(9)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    uint _4861 = in.i_instance * 24u;
    uint _4871 = (in.i_instance * 24u) + 1u;
    uint _4879 = (in.i_instance * 24u) + 2u;
    uint _4887 = (in.i_instance * 24u) + 3u;
    uint _4895 = (in.i_instance * 24u) + 4u;
    uint _4903 = (in.i_instance * 24u) + 5u;
    float4 _4905 = gFxData_1._data[_4903];
    uint _4911 = (in.i_instance * 24u) + 6u;
    uint _4919 = (in.i_instance * 24u) + 7u;
    uint _4927 = (in.i_instance * 24u) + 8u;
    uint _4935 = (in.i_instance * 24u) + 9u;
    uint _4943 = (in.i_instance * 24u) + 10u;
    uint _4951 = (in.i_instance * 24u) + 11u;
    uint _4959 = (in.i_instance * 24u) + 12u;
    uint _4967 = (in.i_instance * 24u) + 13u;
    uint _4975 = (in.i_instance * 24u) + 14u;
    uint _4983 = (in.i_instance * 24u) + 15u;
    uint _4991 = (in.i_instance * 24u) + 16u;
    uint _4999 = (in.i_instance * 24u) + 17u;
    uint _5007 = (in.i_instance * 24u) + 18u;
    uint _5015 = (in.i_instance * 24u) + 19u;
    uint _5023 = (in.i_instance * 24u) + 20u;
    uint _5031 = (in.i_instance * 24u) + 21u;
    uint _5039 = (in.i_instance * 24u) + 22u;
    uint4 _4855 = uint4(gFxData_1._data[(in.i_instance * 24u) + 23u]);
    uint _3685 = _4855.x;
    float2 _5058 = gl_FragCoord.xy + _172.gConv.yy;
    float2 _21731;
    if (_172.gConv.x > 0.5)
    {
        float2 _20044 = _5058;
        _20044.y = _172.gTarget.y - _5058.y;
        _21731 = _20044;
    }
    else
    {
        _21731 = _5058;
    }
    float _3693 = dfdx(in.i_local.x);
    float _3697 = dfdy(in.i_local.x);
    float _3700 = fast::max(abs(_3693) + abs(_3697), 9.9999997473787516355514526367188e-05);
    uint _5101 = _4855.z;
    float2 _5109 = (gFxData_1._data[_4861].xy + gFxData_1._data[_4861].zw) * 0.5;
    float2 _5118 = fast::max((gFxData_1._data[_4861].zw - gFxData_1._data[_4861].xy) * 0.5, float2(0.001000000047497451305389404296875));
    float _21748;
    if (_5101 == 1u)
    {
        float2 _5124 = in.i_local - _5109;
        float _21747;
        do
        {
            if (gFxData_1._data[_4975].w >= 6.282185077667236328125)
            {
                _21747 = abs(length(_5124) - gFxData_1._data[_4871].x) - gFxData_1._data[_4871].y;
                break;
            }
            float _5223 = gFxData_1._data[_4975].z + (gFxData_1._data[_4975].w * 0.5);
            float _5225 = cos(_5223);
            float _5227 = sin(_5223);
            float _5236 = dot(_5124, float2(-_5227, _5225));
            float _5239 = dot(_5124, float2(_5225, _5227));
            float2 _5240 = float2(_5236, _5239);
            float _5243 = abs(_5236);
            _5240.x = _5243;
            float _5246 = gFxData_1._data[_4975].w * 0.5;
            float _5248 = sin(_5246);
            float _5250 = cos(_5246);
            _21747 = (((_5250 * _5243) > (_5248 * _5239)) ? length(_5240 - (float2(_5248, _5250) * gFxData_1._data[_4871].x)) : abs(length(_5240) - gFxData_1._data[_4871].x)) - gFxData_1._data[_4871].y;
            break;
        } while(false);
        _21748 = _21747;
    }
    else
    {
        float _21749;
        if (_5101 == 2u)
        {
            float2 _5286 = in.i_local - gFxData_1._data[_4983].xy;
            float2 _5289 = gFxData_1._data[_4983].zw - gFxData_1._data[_4983].xy;
            _21749 = length(_5286 - (_5289 * fast::clamp(dot(_5286, _5289) / fast::max(dot(_5289, _5289), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4871].x;
        }
        else
        {
            float2 _5151 = in.i_local - _5109;
            float _5342 = fast::min(_5118.x, _5118.y);
            float _5345 = fast::min((_5151.x > 0.0) ? ((_5151.y > 0.0) ? gFxData_1._data[_4871].z : gFxData_1._data[_4871].y) : ((_5151.y > 0.0) ? gFxData_1._data[_4871].w : gFxData_1._data[_4871].x), _5342);
            float _5351 = _5345 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4975].y));
            float _21732;
            float _21733;
            if (_5351 > _5342)
            {
                float _5365 = gFxData_1._data[_4975].y * fast::clamp((_5342 - _5345) / fast::max(0.60000002384185791015625 * _5345, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                _21733 = _5365;
                _21732 = _5345 * (1.0 + (0.60000002384185791015625 * _5365));
            }
            else
            {
                _21733 = gFxData_1._data[_4975].y;
                _21732 = _5351;
            }
            float2 _5378 = (abs(_5151) - _5118) + float2(_21732);
            float2 _5380 = fast::max(_5378, float2(0.0));
            float _21734;
            if ((_5380.x > 0.0) && (_5380.y > 0.0))
            {
                float _21735;
                if (_21733 > 0.001000000047497451305389404296875)
                {
                    float _5394 = 2.0 + (2.0 * _21733);
                    float2 _5399 = _5380 / float2(fast::max(_21732, 9.9999997473787516355514526367188e-05));
                    _21735 = pow(pow(_5399.x, _5394) + pow(_5399.y, _5394), 1.0 / _5394) * _21732;
                }
                else
                {
                    _21735 = length(_5380);
                }
                _21734 = _21735;
            }
            else
            {
                _21734 = fast::max(_5380.x, _5380.y);
            }
            float _5434 = (fast::min(fast::max(_5378.x, _5378.y), 0.0) + _21734) - _21732;
            float _21750;
            if ((_4855.x & 512u) != 0u)
            {
                float2 _5179 = fast::max((gFxData_1._data[_4983].zw - gFxData_1._data[_4983].xy) * 0.5, float2(0.001000000047497451305389404296875));
                float2 _5182 = in.i_local - ((gFxData_1._data[_4983].xy + gFxData_1._data[_4983].zw) * 0.5);
                float _5470 = fast::min(_5179.x, _5179.y);
                float _5473 = fast::min((_5182.x > 0.0) ? ((_5182.y > 0.0) ? gFxData_1._data[_4991].x : gFxData_1._data[_4991].x) : ((_5182.y > 0.0) ? gFxData_1._data[_4991].x : gFxData_1._data[_4991].x), _5470);
                float _5479 = _5473 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4975].y));
                float _21738;
                float _21739;
                if (_5479 > _5470)
                {
                    float _5493 = gFxData_1._data[_4975].y * fast::clamp((_5470 - _5473) / fast::max(0.60000002384185791015625 * _5473, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _21739 = _5493;
                    _21738 = _5473 * (1.0 + (0.60000002384185791015625 * _5493));
                }
                else
                {
                    _21739 = gFxData_1._data[_4975].y;
                    _21738 = _5479;
                }
                float2 _5506 = (abs(_5182) - _5179) + float2(_21738);
                float2 _5508 = fast::max(_5506, float2(0.0));
                float _21740;
                if ((_5508.x > 0.0) && (_5508.y > 0.0))
                {
                    float _21741;
                    if (_21739 > 0.001000000047497451305389404296875)
                    {
                        float _5522 = 2.0 + (2.0 * _21739);
                        float2 _5527 = _5508 / float2(fast::max(_21738, 9.9999997473787516355514526367188e-05));
                        _21741 = pow(pow(_5527.x, _5522) + pow(_5527.y, _5522), 1.0 / _5522) * _21738;
                    }
                    else
                    {
                        _21741 = length(_5508);
                    }
                    _21740 = _21741;
                }
                else
                {
                    _21740 = fast::max(_5508.x, _5508.y);
                }
                float _5562 = (fast::min(fast::max(_5506.x, _5506.y), 0.0) + _21740) - _21738;
                float _5567 = fast::max(gFxData_1._data[_4991].y, 9.9999997473787516355514526367188e-05);
                float _5576 = fast::max(_5567 - abs(_5434 - _5562), 0.0) / _5567;
                _21750 = fast::min(_5434, _5562) - (((_5576 * _5576) * _5567) * 0.25);
            }
            else
            {
                _21750 = _5434;
            }
            _21749 = _21750;
        }
        _21748 = _21749;
    }
    float _3708 = fast::clamp(0.5 - (_21748 / _3700), 0.0, 1.0);
    float _23581;
    if ((_3685 & 1024u) != 0u)
    {
        float2 _3729 = fast::max((gFxData_1._data[_5015].zw - gFxData_1._data[_5015].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float2 _3732 = in.i_local - ((gFxData_1._data[_5015].xy + gFxData_1._data[_5015].zw) * 0.5);
        float _5622 = fast::min(_3729.x, _3729.y);
        float _5625 = fast::min((_3732.x > 0.0) ? ((_3732.y > 0.0) ? gFxData_1._data[_5023].x : gFxData_1._data[_5023].x) : ((_3732.y > 0.0) ? gFxData_1._data[_5023].x : gFxData_1._data[_5023].x), _5622);
        float _5631 = _5625 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_5023].y));
        float _21751;
        float _21752;
        if (_5631 > _5622)
        {
            float _5645 = gFxData_1._data[_5023].y * fast::clamp((_5622 - _5625) / fast::max(0.60000002384185791015625 * _5625, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _21752 = _5645;
            _21751 = _5625 * (1.0 + (0.60000002384185791015625 * _5645));
        }
        else
        {
            _21752 = gFxData_1._data[_5023].y;
            _21751 = _5631;
        }
        float2 _5658 = (abs(_3732) - _3729) + float2(_21751);
        float2 _5660 = fast::max(_5658, float2(0.0));
        float _21753;
        if ((_5660.x > 0.0) && (_5660.y > 0.0))
        {
            float _21754;
            if (_21752 > 0.001000000047497451305389404296875)
            {
                float _5674 = 2.0 + (2.0 * _21752);
                float2 _5679 = _5660 / float2(fast::max(_21751, 9.9999997473787516355514526367188e-05));
                _21754 = pow(pow(_5679.x, _5674) + pow(_5679.y, _5674), 1.0 / _5674) * _21751;
            }
            else
            {
                _21754 = length(_5660);
            }
            _21753 = _21754;
        }
        else
        {
            _21753 = fast::max(_5660.x, _5660.y);
        }
        float _3744 = fast::clamp(0.5 - (((fast::min(fast::max(_5658.x, _5658.y), 0.0) + _21753) - _21751) / _3700), 0.0, 1.0);
        if (_3744 <= 0.0)
        {
            discard_fragment();
        }
        _23581 = _3744;
    }
    else
    {
        _23581 = 1.0;
    }
    bool _3755 = ((_3685 & 32u) != 0u) && (_3708 >= 0.999000012874603271484375);
    float4 _21790;
    if ((((_3685 & 4u) != 0u) && (!((_3685 & 256u) != 0u))) && (!_3755))
    {
        float2 _3778 = in.i_local - gFxData_1._data[_4927].zw;
        uint _5745 = _4855.z;
        float2 _5753 = (gFxData_1._data[_4861].xy + gFxData_1._data[_4861].zw) * 0.5;
        float2 _5762 = fast::max((gFxData_1._data[_4861].zw - gFxData_1._data[_4861].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _21773;
        if (_5745 == 1u)
        {
            float2 _5768 = _3778 - _5753;
            float _21772;
            do
            {
                if (gFxData_1._data[_4975].w >= 6.282185077667236328125)
                {
                    _21772 = abs(length(_5768) - gFxData_1._data[_4871].x) - gFxData_1._data[_4871].y;
                    break;
                }
                float _5867 = gFxData_1._data[_4975].z + (gFxData_1._data[_4975].w * 0.5);
                float _5869 = cos(_5867);
                float _5871 = sin(_5867);
                float _5880 = dot(_5768, float2(-_5871, _5869));
                float _5883 = dot(_5768, float2(_5869, _5871));
                float2 _5884 = float2(_5880, _5883);
                float _5887 = abs(_5880);
                _5884.x = _5887;
                float _5890 = gFxData_1._data[_4975].w * 0.5;
                float _5892 = sin(_5890);
                float _5894 = cos(_5890);
                _21772 = (((_5894 * _5887) > (_5892 * _5883)) ? length(_5884 - (float2(_5892, _5894) * gFxData_1._data[_4871].x)) : abs(length(_5884) - gFxData_1._data[_4871].x)) - gFxData_1._data[_4871].y;
                break;
            } while(false);
            _21773 = _21772;
        }
        else
        {
            float _21774;
            if (_5745 == 2u)
            {
                float2 _5930 = _3778 - gFxData_1._data[_4983].xy;
                float2 _5933 = gFxData_1._data[_4983].zw - gFxData_1._data[_4983].xy;
                _21774 = length(_5930 - (_5933 * fast::clamp(dot(_5930, _5933) / fast::max(dot(_5933, _5933), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4871].x;
            }
            else
            {
                float2 _5795 = _3778 - _5753;
                float _5986 = fast::min(_5762.x, _5762.y);
                float _5989 = fast::min((_5795.x > 0.0) ? ((_5795.y > 0.0) ? gFxData_1._data[_4871].z : gFxData_1._data[_4871].y) : ((_5795.y > 0.0) ? gFxData_1._data[_4871].w : gFxData_1._data[_4871].x), _5986);
                float _5995 = _5989 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4975].y));
                float _21757;
                float _21758;
                if (_5995 > _5986)
                {
                    float _6009 = gFxData_1._data[_4975].y * fast::clamp((_5986 - _5989) / fast::max(0.60000002384185791015625 * _5989, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _21758 = _6009;
                    _21757 = _5989 * (1.0 + (0.60000002384185791015625 * _6009));
                }
                else
                {
                    _21758 = gFxData_1._data[_4975].y;
                    _21757 = _5995;
                }
                float2 _6022 = (abs(_5795) - _5762) + float2(_21757);
                float2 _6024 = fast::max(_6022, float2(0.0));
                float _21759;
                if ((_6024.x > 0.0) && (_6024.y > 0.0))
                {
                    float _21760;
                    if (_21758 > 0.001000000047497451305389404296875)
                    {
                        float _6038 = 2.0 + (2.0 * _21758);
                        float2 _6043 = _6024 / float2(fast::max(_21757, 9.9999997473787516355514526367188e-05));
                        _21760 = pow(pow(_6043.x, _6038) + pow(_6043.y, _6038), 1.0 / _6038) * _21757;
                    }
                    else
                    {
                        _21760 = length(_6024);
                    }
                    _21759 = _21760;
                }
                else
                {
                    _21759 = fast::max(_6024.x, _6024.y);
                }
                float _6078 = (fast::min(fast::max(_6022.x, _6022.y), 0.0) + _21759) - _21757;
                float _21775;
                if ((_4855.x & 512u) != 0u)
                {
                    float2 _5823 = fast::max((gFxData_1._data[_4983].zw - gFxData_1._data[_4983].xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _5826 = _3778 - ((gFxData_1._data[_4983].xy + gFxData_1._data[_4983].zw) * 0.5);
                    float _6114 = fast::min(_5823.x, _5823.y);
                    float _6117 = fast::min((_5826.x > 0.0) ? ((_5826.y > 0.0) ? gFxData_1._data[_4991].x : gFxData_1._data[_4991].x) : ((_5826.y > 0.0) ? gFxData_1._data[_4991].x : gFxData_1._data[_4991].x), _6114);
                    float _6123 = _6117 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4975].y));
                    float _21763;
                    float _21764;
                    if (_6123 > _6114)
                    {
                        float _6137 = gFxData_1._data[_4975].y * fast::clamp((_6114 - _6117) / fast::max(0.60000002384185791015625 * _6117, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21764 = _6137;
                        _21763 = _6117 * (1.0 + (0.60000002384185791015625 * _6137));
                    }
                    else
                    {
                        _21764 = gFxData_1._data[_4975].y;
                        _21763 = _6123;
                    }
                    float2 _6150 = (abs(_5826) - _5823) + float2(_21763);
                    float2 _6152 = fast::max(_6150, float2(0.0));
                    float _21765;
                    if ((_6152.x > 0.0) && (_6152.y > 0.0))
                    {
                        float _21766;
                        if (_21764 > 0.001000000047497451305389404296875)
                        {
                            float _6166 = 2.0 + (2.0 * _21764);
                            float2 _6171 = _6152 / float2(fast::max(_21763, 9.9999997473787516355514526367188e-05));
                            _21766 = pow(pow(_6171.x, _6166) + pow(_6171.y, _6166), 1.0 / _6166) * _21763;
                        }
                        else
                        {
                            _21766 = length(_6152);
                        }
                        _21765 = _21766;
                    }
                    else
                    {
                        _21765 = fast::max(_6152.x, _6152.y);
                    }
                    float _6206 = (fast::min(fast::max(_6150.x, _6150.y), 0.0) + _21765) - _21763;
                    float _6211 = fast::max(gFxData_1._data[_4991].y, 9.9999997473787516355514526367188e-05);
                    float _6220 = fast::max(_6211 - abs(_6078 - _6206), 0.0) / _6211;
                    _21775 = fast::min(_6078, _6206) - (((_6220 * _6220) * _6211) * 0.25);
                }
                else
                {
                    _21775 = _6078;
                }
                _21774 = _21775;
            }
            _21773 = _21774;
        }
        float _3787 = (_21773 - gFxData_1._data[_4927].y) / (fast::max(gFxData_1._data[_4927].x * 0.5, _3700 * 0.5) * 1.41421353816986083984375);
        float _6237 = sign(_3787);
        float _6239 = abs(_3787);
        float _6250 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_6239 * _6239))) * _6239)) * _6239);
        float _6253 = _6250 * _6250;
        float _6268 = fast::clamp(gFxData_1._data[_4919].w * (0.5 - (0.5 * (_6237 - (_6237 / (_6253 * _6253))))), 0.0, 1.0);
        _21790 = float4(gFxData_1._data[_4919].xyz * _6268, _6268);
    }
    else
    {
        _21790 = float4(0.0);
    }
    float4 _23215;
    if (((_3685 & 8u) != 0u) && (!_3755))
    {
        float _3807 = fast::max(gFxData_1._data[_4943].x, 0.001000000047497451305389404296875);
        float _21787;
        float _21788;
        if ((_3685 & 8192u) != 0u)
        {
            float2 _3819 = (gFxData_1._data[_4861].xy + gFxData_1._data[_4861].zw) * 0.5;
            float2 _3828 = fast::max((gFxData_1._data[_4861].zw - gFxData_1._data[_4861].xy) * 0.5, float2(0.001000000047497451305389404296875));
            float2 _3835 = gFxData_1._data[_4861].xy - gFxData_1._data[_5039].xy;
            float2 _3842 = gFxData_1._data[_5039].zw - gFxData_1._data[_4861].zw;
            float2 _3872 = fast::clamp(float2((in.i_local.x < _3819.x) ? _3835.x : _3842.x, (in.i_local.y < _3819.y) ? _3835.y : _3842.y) * float2(0.58823525905609130859375), float2(fast::min(_3807, 1.5)), float2(_3807));
            float _3877 = fast::min(_3828.x, _3828.y);
            float _21786;
            if (_4855.z == 0u)
            {
                _21786 = fast::min(((in.i_local.x > _3819.x) ? ((in.i_local.y > _3819.y) ? gFxData_1._data[_4871].z : gFxData_1._data[_4871].y) : ((in.i_local.y > _3819.y) ? gFxData_1._data[_4871].w : gFxData_1._data[_4871].x)) * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4975].y)), _3877);
            }
            else
            {
                _21786 = _3877;
            }
            float2 _3926 = fast::max(abs(in.i_local - _3819) - (_3828 - float2(_21786)), float2(0.0));
            float _3928 = length(_3926);
            float2 _3936 = select(float2(0.707099974155426025390625), _3926 / float2(_3928), bool2(_3928 > 9.9999997473787516355514526367188e-05));
            float2 _3951 = in.i_local - gFxData_1._data[_5039].xy;
            float2 _3956 = gFxData_1._data[_5039].zw - in.i_local;
            _21788 = fast::clamp(fast::min(fast::min(_3951.x, _3951.y), fast::min(_3956.x, _3956.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _21787 = rsqrt(dot(_3936 * _3936, float2(1.0) / (_3872 * _3872)));
        }
        else
        {
            _21788 = 1.0;
            _21787 = _3807;
        }
        float _3974 = fast::max(_21748, 0.0) / _21787;
        float _6294 = fast::clamp(gFxData_1._data[_4935].w * fast::clamp((exp(((-_3974) * _3974) * 2.2000000476837158203125) * gFxData_1._data[_4943].y) * _21788, 0.0, 1.0), 0.0, 1.0);
        _23215 = float4(gFxData_1._data[_4935].xyz * _6294, _6294) + (_21790 * (1.0 - _6294));
    }
    else
    {
        _23215 = _21790;
    }
    float4 _23502;
    float _23548;
    if (((_3685 & 32u) != 0u) && (_3708 > 0.0))
    {
        float _6372 = gFxData_1._data[_4951].x * _172.gDisplay.z;
        float2 _6383 = fast::max((gFxData_1._data[_4861].zw - gFxData_1._data[_4861].xy) * 0.5, float2(1.0));
        float _6391 = fast::clamp(gFxData_1._data[_4951].z, 0.001000000047497451305389404296875, fast::min(_6383.x, _6383.y));
        float _6400 = fast::clamp(1.0 - (fast::max(-_21748, 0.0) / _6391), 0.0, 1.0);
        float _6406 = sqrt(fast::clamp(1.0 - (_6400 * _6400), 0.0, 1.0));
        float2 _21868;
        float3 _22730;
        if (_6400 > 0.0)
        {
            float2 _6752 = in.i_local + float2(0.5, 0.0);
            uint _6812 = _4855.z;
            float2 _6820 = (gFxData_1._data[_4861].xy + gFxData_1._data[_4861].zw) * 0.5;
            float2 _6829 = fast::max((gFxData_1._data[_4861].zw - gFxData_1._data[_4861].xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _21808;
            if (_6812 == 1u)
            {
                float2 _6835 = _6752 - _6820;
                float _21807;
                do
                {
                    if (gFxData_1._data[_4975].w >= 6.282185077667236328125)
                    {
                        _21807 = abs(length(_6835) - gFxData_1._data[_4871].x) - gFxData_1._data[_4871].y;
                        break;
                    }
                    float _6934 = gFxData_1._data[_4975].z + (gFxData_1._data[_4975].w * 0.5);
                    float _6936 = cos(_6934);
                    float _6938 = sin(_6934);
                    float _6947 = dot(_6835, float2(-_6938, _6936));
                    float _6950 = dot(_6835, float2(_6936, _6938));
                    float2 _6951 = float2(_6947, _6950);
                    float _6954 = abs(_6947);
                    _6951.x = _6954;
                    float _6957 = gFxData_1._data[_4975].w * 0.5;
                    float _6959 = sin(_6957);
                    float _6961 = cos(_6957);
                    _21807 = (((_6961 * _6954) > (_6959 * _6950)) ? length(_6951 - (float2(_6959, _6961) * gFxData_1._data[_4871].x)) : abs(length(_6951) - gFxData_1._data[_4871].x)) - gFxData_1._data[_4871].y;
                    break;
                } while(false);
                _21808 = _21807;
            }
            else
            {
                float _21809;
                if (_6812 == 2u)
                {
                    float2 _6997 = _6752 - gFxData_1._data[_4983].xy;
                    float2 _7000 = gFxData_1._data[_4983].zw - gFxData_1._data[_4983].xy;
                    _21809 = length(_6997 - (_7000 * fast::clamp(dot(_6997, _7000) / fast::max(dot(_7000, _7000), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4871].x;
                }
                else
                {
                    float2 _6862 = _6752 - _6820;
                    float _7053 = fast::min(_6829.x, _6829.y);
                    float _7056 = fast::min((_6862.x > 0.0) ? ((_6862.y > 0.0) ? gFxData_1._data[_4871].z : gFxData_1._data[_4871].y) : ((_6862.y > 0.0) ? gFxData_1._data[_4871].w : gFxData_1._data[_4871].x), _7053);
                    float _7062 = _7056 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4975].y));
                    float _21792;
                    float _21793;
                    if (_7062 > _7053)
                    {
                        float _7076 = gFxData_1._data[_4975].y * fast::clamp((_7053 - _7056) / fast::max(0.60000002384185791015625 * _7056, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21793 = _7076;
                        _21792 = _7056 * (1.0 + (0.60000002384185791015625 * _7076));
                    }
                    else
                    {
                        _21793 = gFxData_1._data[_4975].y;
                        _21792 = _7062;
                    }
                    float2 _7089 = (abs(_6862) - _6829) + float2(_21792);
                    float2 _7091 = fast::max(_7089, float2(0.0));
                    float _21794;
                    if ((_7091.x > 0.0) && (_7091.y > 0.0))
                    {
                        float _21795;
                        if (_21793 > 0.001000000047497451305389404296875)
                        {
                            float _7105 = 2.0 + (2.0 * _21793);
                            float2 _7110 = _7091 / float2(fast::max(_21792, 9.9999997473787516355514526367188e-05));
                            _21795 = pow(pow(_7110.x, _7105) + pow(_7110.y, _7105), 1.0 / _7105) * _21792;
                        }
                        else
                        {
                            _21795 = length(_7091);
                        }
                        _21794 = _21795;
                    }
                    else
                    {
                        _21794 = fast::max(_7091.x, _7091.y);
                    }
                    float _7145 = (fast::min(fast::max(_7089.x, _7089.y), 0.0) + _21794) - _21792;
                    float _21810;
                    if ((_4855.x & 512u) != 0u)
                    {
                        float2 _6890 = fast::max((gFxData_1._data[_4983].zw - gFxData_1._data[_4983].xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _6893 = _6752 - ((gFxData_1._data[_4983].xy + gFxData_1._data[_4983].zw) * 0.5);
                        float _7181 = fast::min(_6890.x, _6890.y);
                        float _7184 = fast::min((_6893.x > 0.0) ? ((_6893.y > 0.0) ? gFxData_1._data[_4991].x : gFxData_1._data[_4991].x) : ((_6893.y > 0.0) ? gFxData_1._data[_4991].x : gFxData_1._data[_4991].x), _7181);
                        float _7190 = _7184 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4975].y));
                        float _21798;
                        float _21799;
                        if (_7190 > _7181)
                        {
                            float _7204 = gFxData_1._data[_4975].y * fast::clamp((_7181 - _7184) / fast::max(0.60000002384185791015625 * _7184, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21799 = _7204;
                            _21798 = _7184 * (1.0 + (0.60000002384185791015625 * _7204));
                        }
                        else
                        {
                            _21799 = gFxData_1._data[_4975].y;
                            _21798 = _7190;
                        }
                        float2 _7217 = (abs(_6893) - _6890) + float2(_21798);
                        float2 _7219 = fast::max(_7217, float2(0.0));
                        float _21800;
                        if ((_7219.x > 0.0) && (_7219.y > 0.0))
                        {
                            float _21801;
                            if (_21799 > 0.001000000047497451305389404296875)
                            {
                                float _7233 = 2.0 + (2.0 * _21799);
                                float2 _7238 = _7219 / float2(fast::max(_21798, 9.9999997473787516355514526367188e-05));
                                _21801 = pow(pow(_7238.x, _7233) + pow(_7238.y, _7233), 1.0 / _7233) * _21798;
                            }
                            else
                            {
                                _21801 = length(_7219);
                            }
                            _21800 = _21801;
                        }
                        else
                        {
                            _21800 = fast::max(_7219.x, _7219.y);
                        }
                        float _7273 = (fast::min(fast::max(_7217.x, _7217.y), 0.0) + _21800) - _21798;
                        float _7278 = fast::max(gFxData_1._data[_4991].y, 9.9999997473787516355514526367188e-05);
                        float _7287 = fast::max(_7278 - abs(_7145 - _7273), 0.0) / _7278;
                        _21810 = fast::min(_7145, _7273) - (((_7287 * _7287) * _7278) * 0.25);
                    }
                    else
                    {
                        _21810 = _7145;
                    }
                    _21809 = _21810;
                }
                _21808 = _21809;
            }
            float2 _6756 = in.i_local - float2(0.5, 0.0);
            uint _7328 = _4855.z;
            float2 _7336 = (gFxData_1._data[_4861].xy + gFxData_1._data[_4861].zw) * 0.5;
            float2 _7345 = fast::max((gFxData_1._data[_4861].zw - gFxData_1._data[_4861].xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _21827;
            if (_7328 == 1u)
            {
                float2 _7351 = _6756 - _7336;
                float _21826;
                do
                {
                    if (gFxData_1._data[_4975].w >= 6.282185077667236328125)
                    {
                        _21826 = abs(length(_7351) - gFxData_1._data[_4871].x) - gFxData_1._data[_4871].y;
                        break;
                    }
                    float _7450 = gFxData_1._data[_4975].z + (gFxData_1._data[_4975].w * 0.5);
                    float _7452 = cos(_7450);
                    float _7454 = sin(_7450);
                    float _7463 = dot(_7351, float2(-_7454, _7452));
                    float _7466 = dot(_7351, float2(_7452, _7454));
                    float2 _7467 = float2(_7463, _7466);
                    float _7470 = abs(_7463);
                    _7467.x = _7470;
                    float _7473 = gFxData_1._data[_4975].w * 0.5;
                    float _7475 = sin(_7473);
                    float _7477 = cos(_7473);
                    _21826 = (((_7477 * _7470) > (_7475 * _7466)) ? length(_7467 - (float2(_7475, _7477) * gFxData_1._data[_4871].x)) : abs(length(_7467) - gFxData_1._data[_4871].x)) - gFxData_1._data[_4871].y;
                    break;
                } while(false);
                _21827 = _21826;
            }
            else
            {
                float _21828;
                if (_7328 == 2u)
                {
                    float2 _7513 = _6756 - gFxData_1._data[_4983].xy;
                    float2 _7516 = gFxData_1._data[_4983].zw - gFxData_1._data[_4983].xy;
                    _21828 = length(_7513 - (_7516 * fast::clamp(dot(_7513, _7516) / fast::max(dot(_7516, _7516), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4871].x;
                }
                else
                {
                    float2 _7378 = _6756 - _7336;
                    float _7569 = fast::min(_7345.x, _7345.y);
                    float _7572 = fast::min((_7378.x > 0.0) ? ((_7378.y > 0.0) ? gFxData_1._data[_4871].z : gFxData_1._data[_4871].y) : ((_7378.y > 0.0) ? gFxData_1._data[_4871].w : gFxData_1._data[_4871].x), _7569);
                    float _7578 = _7572 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4975].y));
                    float _21811;
                    float _21812;
                    if (_7578 > _7569)
                    {
                        float _7592 = gFxData_1._data[_4975].y * fast::clamp((_7569 - _7572) / fast::max(0.60000002384185791015625 * _7572, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21812 = _7592;
                        _21811 = _7572 * (1.0 + (0.60000002384185791015625 * _7592));
                    }
                    else
                    {
                        _21812 = gFxData_1._data[_4975].y;
                        _21811 = _7578;
                    }
                    float2 _7605 = (abs(_7378) - _7345) + float2(_21811);
                    float2 _7607 = fast::max(_7605, float2(0.0));
                    float _21813;
                    if ((_7607.x > 0.0) && (_7607.y > 0.0))
                    {
                        float _21814;
                        if (_21812 > 0.001000000047497451305389404296875)
                        {
                            float _7621 = 2.0 + (2.0 * _21812);
                            float2 _7626 = _7607 / float2(fast::max(_21811, 9.9999997473787516355514526367188e-05));
                            _21814 = pow(pow(_7626.x, _7621) + pow(_7626.y, _7621), 1.0 / _7621) * _21811;
                        }
                        else
                        {
                            _21814 = length(_7607);
                        }
                        _21813 = _21814;
                    }
                    else
                    {
                        _21813 = fast::max(_7607.x, _7607.y);
                    }
                    float _7661 = (fast::min(fast::max(_7605.x, _7605.y), 0.0) + _21813) - _21811;
                    float _21829;
                    if ((_4855.x & 512u) != 0u)
                    {
                        float2 _7406 = fast::max((gFxData_1._data[_4983].zw - gFxData_1._data[_4983].xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _7409 = _6756 - ((gFxData_1._data[_4983].xy + gFxData_1._data[_4983].zw) * 0.5);
                        float _7697 = fast::min(_7406.x, _7406.y);
                        float _7700 = fast::min((_7409.x > 0.0) ? ((_7409.y > 0.0) ? gFxData_1._data[_4991].x : gFxData_1._data[_4991].x) : ((_7409.y > 0.0) ? gFxData_1._data[_4991].x : gFxData_1._data[_4991].x), _7697);
                        float _7706 = _7700 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4975].y));
                        float _21817;
                        float _21818;
                        if (_7706 > _7697)
                        {
                            float _7720 = gFxData_1._data[_4975].y * fast::clamp((_7697 - _7700) / fast::max(0.60000002384185791015625 * _7700, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21818 = _7720;
                            _21817 = _7700 * (1.0 + (0.60000002384185791015625 * _7720));
                        }
                        else
                        {
                            _21818 = gFxData_1._data[_4975].y;
                            _21817 = _7706;
                        }
                        float2 _7733 = (abs(_7409) - _7406) + float2(_21817);
                        float2 _7735 = fast::max(_7733, float2(0.0));
                        float _21819;
                        if ((_7735.x > 0.0) && (_7735.y > 0.0))
                        {
                            float _21820;
                            if (_21818 > 0.001000000047497451305389404296875)
                            {
                                float _7749 = 2.0 + (2.0 * _21818);
                                float2 _7754 = _7735 / float2(fast::max(_21817, 9.9999997473787516355514526367188e-05));
                                _21820 = pow(pow(_7754.x, _7749) + pow(_7754.y, _7749), 1.0 / _7749) * _21817;
                            }
                            else
                            {
                                _21820 = length(_7735);
                            }
                            _21819 = _21820;
                        }
                        else
                        {
                            _21819 = fast::max(_7735.x, _7735.y);
                        }
                        float _7789 = (fast::min(fast::max(_7733.x, _7733.y), 0.0) + _21819) - _21817;
                        float _7794 = fast::max(gFxData_1._data[_4991].y, 9.9999997473787516355514526367188e-05);
                        float _7803 = fast::max(_7794 - abs(_7661 - _7789), 0.0) / _7794;
                        _21829 = fast::min(_7661, _7789) - (((_7803 * _7803) * _7794) * 0.25);
                    }
                    else
                    {
                        _21829 = _7661;
                    }
                    _21828 = _21829;
                }
                _21827 = _21828;
            }
            float2 _6761 = in.i_local + float2(0.0, 0.5);
            uint _7844 = _4855.z;
            float2 _7852 = (gFxData_1._data[_4861].xy + gFxData_1._data[_4861].zw) * 0.5;
            float2 _7861 = fast::max((gFxData_1._data[_4861].zw - gFxData_1._data[_4861].xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _21846;
            if (_7844 == 1u)
            {
                float2 _7867 = _6761 - _7852;
                float _21845;
                do
                {
                    if (gFxData_1._data[_4975].w >= 6.282185077667236328125)
                    {
                        _21845 = abs(length(_7867) - gFxData_1._data[_4871].x) - gFxData_1._data[_4871].y;
                        break;
                    }
                    float _7966 = gFxData_1._data[_4975].z + (gFxData_1._data[_4975].w * 0.5);
                    float _7968 = cos(_7966);
                    float _7970 = sin(_7966);
                    float _7979 = dot(_7867, float2(-_7970, _7968));
                    float _7982 = dot(_7867, float2(_7968, _7970));
                    float2 _7983 = float2(_7979, _7982);
                    float _7986 = abs(_7979);
                    _7983.x = _7986;
                    float _7989 = gFxData_1._data[_4975].w * 0.5;
                    float _7991 = sin(_7989);
                    float _7993 = cos(_7989);
                    _21845 = (((_7993 * _7986) > (_7991 * _7982)) ? length(_7983 - (float2(_7991, _7993) * gFxData_1._data[_4871].x)) : abs(length(_7983) - gFxData_1._data[_4871].x)) - gFxData_1._data[_4871].y;
                    break;
                } while(false);
                _21846 = _21845;
            }
            else
            {
                float _21847;
                if (_7844 == 2u)
                {
                    float2 _8029 = _6761 - gFxData_1._data[_4983].xy;
                    float2 _8032 = gFxData_1._data[_4983].zw - gFxData_1._data[_4983].xy;
                    _21847 = length(_8029 - (_8032 * fast::clamp(dot(_8029, _8032) / fast::max(dot(_8032, _8032), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4871].x;
                }
                else
                {
                    float2 _7894 = _6761 - _7852;
                    float _8085 = fast::min(_7861.x, _7861.y);
                    float _8088 = fast::min((_7894.x > 0.0) ? ((_7894.y > 0.0) ? gFxData_1._data[_4871].z : gFxData_1._data[_4871].y) : ((_7894.y > 0.0) ? gFxData_1._data[_4871].w : gFxData_1._data[_4871].x), _8085);
                    float _8094 = _8088 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4975].y));
                    float _21830;
                    float _21831;
                    if (_8094 > _8085)
                    {
                        float _8108 = gFxData_1._data[_4975].y * fast::clamp((_8085 - _8088) / fast::max(0.60000002384185791015625 * _8088, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21831 = _8108;
                        _21830 = _8088 * (1.0 + (0.60000002384185791015625 * _8108));
                    }
                    else
                    {
                        _21831 = gFxData_1._data[_4975].y;
                        _21830 = _8094;
                    }
                    float2 _8121 = (abs(_7894) - _7861) + float2(_21830);
                    float2 _8123 = fast::max(_8121, float2(0.0));
                    float _21832;
                    if ((_8123.x > 0.0) && (_8123.y > 0.0))
                    {
                        float _21833;
                        if (_21831 > 0.001000000047497451305389404296875)
                        {
                            float _8137 = 2.0 + (2.0 * _21831);
                            float2 _8142 = _8123 / float2(fast::max(_21830, 9.9999997473787516355514526367188e-05));
                            _21833 = pow(pow(_8142.x, _8137) + pow(_8142.y, _8137), 1.0 / _8137) * _21830;
                        }
                        else
                        {
                            _21833 = length(_8123);
                        }
                        _21832 = _21833;
                    }
                    else
                    {
                        _21832 = fast::max(_8123.x, _8123.y);
                    }
                    float _8177 = (fast::min(fast::max(_8121.x, _8121.y), 0.0) + _21832) - _21830;
                    float _21848;
                    if ((_4855.x & 512u) != 0u)
                    {
                        float2 _7922 = fast::max((gFxData_1._data[_4983].zw - gFxData_1._data[_4983].xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _7925 = _6761 - ((gFxData_1._data[_4983].xy + gFxData_1._data[_4983].zw) * 0.5);
                        float _8213 = fast::min(_7922.x, _7922.y);
                        float _8216 = fast::min((_7925.x > 0.0) ? ((_7925.y > 0.0) ? gFxData_1._data[_4991].x : gFxData_1._data[_4991].x) : ((_7925.y > 0.0) ? gFxData_1._data[_4991].x : gFxData_1._data[_4991].x), _8213);
                        float _8222 = _8216 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4975].y));
                        float _21836;
                        float _21837;
                        if (_8222 > _8213)
                        {
                            float _8236 = gFxData_1._data[_4975].y * fast::clamp((_8213 - _8216) / fast::max(0.60000002384185791015625 * _8216, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21837 = _8236;
                            _21836 = _8216 * (1.0 + (0.60000002384185791015625 * _8236));
                        }
                        else
                        {
                            _21837 = gFxData_1._data[_4975].y;
                            _21836 = _8222;
                        }
                        float2 _8249 = (abs(_7925) - _7922) + float2(_21836);
                        float2 _8251 = fast::max(_8249, float2(0.0));
                        float _21838;
                        if ((_8251.x > 0.0) && (_8251.y > 0.0))
                        {
                            float _21839;
                            if (_21837 > 0.001000000047497451305389404296875)
                            {
                                float _8265 = 2.0 + (2.0 * _21837);
                                float2 _8270 = _8251 / float2(fast::max(_21836, 9.9999997473787516355514526367188e-05));
                                _21839 = pow(pow(_8270.x, _8265) + pow(_8270.y, _8265), 1.0 / _8265) * _21836;
                            }
                            else
                            {
                                _21839 = length(_8251);
                            }
                            _21838 = _21839;
                        }
                        else
                        {
                            _21838 = fast::max(_8251.x, _8251.y);
                        }
                        float _8305 = (fast::min(fast::max(_8249.x, _8249.y), 0.0) + _21838) - _21836;
                        float _8310 = fast::max(gFxData_1._data[_4991].y, 9.9999997473787516355514526367188e-05);
                        float _8319 = fast::max(_8310 - abs(_8177 - _8305), 0.0) / _8310;
                        _21848 = fast::min(_8177, _8305) - (((_8319 * _8319) * _8310) * 0.25);
                    }
                    else
                    {
                        _21848 = _8177;
                    }
                    _21847 = _21848;
                }
                _21846 = _21847;
            }
            float2 _6765 = in.i_local - float2(0.0, 0.5);
            uint _8360 = _4855.z;
            float2 _8368 = (gFxData_1._data[_4861].xy + gFxData_1._data[_4861].zw) * 0.5;
            float2 _8377 = fast::max((gFxData_1._data[_4861].zw - gFxData_1._data[_4861].xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _21865;
            if (_8360 == 1u)
            {
                float2 _8383 = _6765 - _8368;
                float _21864;
                do
                {
                    if (gFxData_1._data[_4975].w >= 6.282185077667236328125)
                    {
                        _21864 = abs(length(_8383) - gFxData_1._data[_4871].x) - gFxData_1._data[_4871].y;
                        break;
                    }
                    float _8482 = gFxData_1._data[_4975].z + (gFxData_1._data[_4975].w * 0.5);
                    float _8484 = cos(_8482);
                    float _8486 = sin(_8482);
                    float _8495 = dot(_8383, float2(-_8486, _8484));
                    float _8498 = dot(_8383, float2(_8484, _8486));
                    float2 _8499 = float2(_8495, _8498);
                    float _8502 = abs(_8495);
                    _8499.x = _8502;
                    float _8505 = gFxData_1._data[_4975].w * 0.5;
                    float _8507 = sin(_8505);
                    float _8509 = cos(_8505);
                    _21864 = (((_8509 * _8502) > (_8507 * _8498)) ? length(_8499 - (float2(_8507, _8509) * gFxData_1._data[_4871].x)) : abs(length(_8499) - gFxData_1._data[_4871].x)) - gFxData_1._data[_4871].y;
                    break;
                } while(false);
                _21865 = _21864;
            }
            else
            {
                float _21866;
                if (_8360 == 2u)
                {
                    float2 _8545 = _6765 - gFxData_1._data[_4983].xy;
                    float2 _8548 = gFxData_1._data[_4983].zw - gFxData_1._data[_4983].xy;
                    _21866 = length(_8545 - (_8548 * fast::clamp(dot(_8545, _8548) / fast::max(dot(_8548, _8548), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4871].x;
                }
                else
                {
                    float2 _8410 = _6765 - _8368;
                    float _8601 = fast::min(_8377.x, _8377.y);
                    float _8604 = fast::min((_8410.x > 0.0) ? ((_8410.y > 0.0) ? gFxData_1._data[_4871].z : gFxData_1._data[_4871].y) : ((_8410.y > 0.0) ? gFxData_1._data[_4871].w : gFxData_1._data[_4871].x), _8601);
                    float _8610 = _8604 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4975].y));
                    float _21849;
                    float _21850;
                    if (_8610 > _8601)
                    {
                        float _8624 = gFxData_1._data[_4975].y * fast::clamp((_8601 - _8604) / fast::max(0.60000002384185791015625 * _8604, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21850 = _8624;
                        _21849 = _8604 * (1.0 + (0.60000002384185791015625 * _8624));
                    }
                    else
                    {
                        _21850 = gFxData_1._data[_4975].y;
                        _21849 = _8610;
                    }
                    float2 _8637 = (abs(_8410) - _8377) + float2(_21849);
                    float2 _8639 = fast::max(_8637, float2(0.0));
                    float _21851;
                    if ((_8639.x > 0.0) && (_8639.y > 0.0))
                    {
                        float _21852;
                        if (_21850 > 0.001000000047497451305389404296875)
                        {
                            float _8653 = 2.0 + (2.0 * _21850);
                            float2 _8658 = _8639 / float2(fast::max(_21849, 9.9999997473787516355514526367188e-05));
                            _21852 = pow(pow(_8658.x, _8653) + pow(_8658.y, _8653), 1.0 / _8653) * _21849;
                        }
                        else
                        {
                            _21852 = length(_8639);
                        }
                        _21851 = _21852;
                    }
                    else
                    {
                        _21851 = fast::max(_8639.x, _8639.y);
                    }
                    float _8693 = (fast::min(fast::max(_8637.x, _8637.y), 0.0) + _21851) - _21849;
                    float _21867;
                    if ((_4855.x & 512u) != 0u)
                    {
                        float2 _8438 = fast::max((gFxData_1._data[_4983].zw - gFxData_1._data[_4983].xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _8441 = _6765 - ((gFxData_1._data[_4983].xy + gFxData_1._data[_4983].zw) * 0.5);
                        float _8729 = fast::min(_8438.x, _8438.y);
                        float _8732 = fast::min((_8441.x > 0.0) ? ((_8441.y > 0.0) ? gFxData_1._data[_4991].x : gFxData_1._data[_4991].x) : ((_8441.y > 0.0) ? gFxData_1._data[_4991].x : gFxData_1._data[_4991].x), _8729);
                        float _8738 = _8732 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4975].y));
                        float _21855;
                        float _21856;
                        if (_8738 > _8729)
                        {
                            float _8752 = gFxData_1._data[_4975].y * fast::clamp((_8729 - _8732) / fast::max(0.60000002384185791015625 * _8732, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21856 = _8752;
                            _21855 = _8732 * (1.0 + (0.60000002384185791015625 * _8752));
                        }
                        else
                        {
                            _21856 = gFxData_1._data[_4975].y;
                            _21855 = _8738;
                        }
                        float2 _8765 = (abs(_8441) - _8438) + float2(_21855);
                        float2 _8767 = fast::max(_8765, float2(0.0));
                        float _21857;
                        if ((_8767.x > 0.0) && (_8767.y > 0.0))
                        {
                            float _21858;
                            if (_21856 > 0.001000000047497451305389404296875)
                            {
                                float _8781 = 2.0 + (2.0 * _21856);
                                float2 _8786 = _8767 / float2(fast::max(_21855, 9.9999997473787516355514526367188e-05));
                                _21858 = pow(pow(_8786.x, _8781) + pow(_8786.y, _8781), 1.0 / _8781) * _21855;
                            }
                            else
                            {
                                _21858 = length(_8767);
                            }
                            _21857 = _21858;
                        }
                        else
                        {
                            _21857 = fast::max(_8767.x, _8767.y);
                        }
                        float _8821 = (fast::min(fast::max(_8765.x, _8765.y), 0.0) + _21857) - _21855;
                        float _8826 = fast::max(gFxData_1._data[_4991].y, 9.9999997473787516355514526367188e-05);
                        float _8835 = fast::max(_8826 - abs(_8693 - _8821), 0.0) / _8826;
                        _21867 = fast::min(_8693, _8821) - (((_8835 * _8835) * _8826) * 0.25);
                    }
                    else
                    {
                        _21867 = _8693;
                    }
                    _21866 = _21867;
                }
                _21865 = _21866;
            }
            float2 _6771 = float2(_21808 - _21827, _21846 - _21865);
            float _6773 = length(_6771);
            float2 _6781 = select(float2(0.0, -1.0), _6771 / float2(_6773), bool2(_6773 > 9.9999997473787516355514526367188e-06));
            _22730 = fast::normalize(float3(_6781 * fast::min(_6400 / fast::max(_6406, 0.001000000047497451305389404296875), 8.0), 1.0));
            _21868 = _6781;
        }
        else
        {
            _22730 = float3(0.0, 0.0, 1.0);
            _21868 = float2(0.0, -1.0);
        }
        float2 _6432 = ((-_21868) * gFxData_1._data[_4951].y) * (1.0 - _6406);
        float2 _21869;
        if (gFxData_1._data[_4991].z > 0.0)
        {
            _21869 = (((gFxData_1._data[_4861].xy + gFxData_1._data[_4861].zw) * 0.5) - in.i_local) * (gFxData_1._data[_4991].z / (1.0 + gFxData_1._data[_4991].z));
        }
        else
        {
            _21869 = float2(0.0);
        }
        float2 _6458 = _21731 * _172.gTarget.zw;
        float2 _6465 = _172.gDisplay.zw * _172.gTarget.zw;
        float2 _6470 = (_6432 + _21869) * _6465;
        float2 _6473 = _6432 * _6465;
        float3 _22474;
        float _22497;
        float3 _22966;
        if (_172.gTime.z > 0.5)
        {
            float3 _22477;
            if (((gFxData_1._data[_4951].w > 0.001000000047497451305389404296875) && (_6400 > 0.0)) && ((((gFxData_1._data[_4951].y * 0.300000011920928955078125) * gFxData_1._data[_4951].w) * _172.gDisplay.z) > (_6372 * 0.3499999940395355224609375)))
            {
                float _6495 = 0.300000011920928955078125 * gFxData_1._data[_4951].w;
                float2 _6502 = (_6458 + _6470) - (_6473 * _6495);
                float _8860 = fast::clamp(log2(fast::max(_6372, 1.0)) - 1.0, 0.0, 5.0);
                int _8863 = int(floor(_8860));
                float _8867 = _8860 - float(_8863);
                float4 _21944;
                if (_8863 <= 0)
                {
                    float2 _21943;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _20426 = _6502;
                        _20426.y = 1.0 - _6502.y;
                        _21943 = _20426;
                    }
                    else
                    {
                        _21943 = _6502;
                    }
                    _21944 = gBackdrop0.sample(gLinear, _21943, level(0.0));
                }
                else
                {
                    float4 _21945;
                    if (_8863 == 1)
                    {
                        float2 _9002 = (_6502 * _172.gLevel[1].xy) - float2(0.5);
                        float2 _9004 = floor(_9002);
                        float2 _9007 = _9002 - _9004;
                        float2 _9010 = _9007 * _9007;
                        float2 _9013 = _9010 * _9007;
                        float2 _9032 = (((_9013 * 3.0) - (_9010 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _9045 = _9013 * 0.16666667163372039794921875;
                        float2 _9048 = (((((-_9013) + (_9010 * 3.0)) - (_9007 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9032;
                        float2 _9051 = (((((_9013 * (-3.0)) + (_9010 * 3.0)) + (_9007 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9045;
                        float2 _9061 = ((_9004 - float2(0.5)) + (_9032 / _9048)) * _172.gLevel[1].zw;
                        float2 _9071 = ((_9004 + float2(1.5)) + (_9045 / _9051)) * _172.gLevel[1].zw;
                        float2 _21939;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20431 = _9061;
                            _20431.y = 1.0 - _9061.y;
                            _21939 = _20431;
                        }
                        else
                        {
                            _21939 = _9061;
                        }
                        float _9091 = _9061.y;
                        float2 _9092 = float2(_9071.x, _9091);
                        float2 _21940;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20437 = _9092;
                            _20437.y = 1.0 - _9091;
                            _21940 = _20437;
                        }
                        else
                        {
                            _21940 = _9092;
                        }
                        float _9108 = _9071.y;
                        float2 _9109 = float2(_9061.x, _9108);
                        float2 _21941;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20444 = _9109;
                            _20444.y = 1.0 - _9108;
                            _21941 = _20444;
                        }
                        else
                        {
                            _21941 = _9109;
                        }
                        float2 _21942;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20450 = _9071;
                            _20450.y = 1.0 - _9071.y;
                            _21942 = _20450;
                        }
                        else
                        {
                            _21942 = _9071;
                        }
                        _21945 = (((gBackdrop1.sample(gLinear, _21939, level(0.0)) * _9048.x) + (gBackdrop1.sample(gLinear, _21940, level(0.0)) * _9051.x)) * _9048.y) + (((gBackdrop1.sample(gLinear, _21941, level(0.0)) * _9048.x) + (gBackdrop1.sample(gLinear, _21942, level(0.0)) * _9051.x)) * _9051.y);
                    }
                    else
                    {
                        float4 _21946;
                        if (_8863 == 2)
                        {
                            float2 _9209 = (_6502 * _172.gLevel[2].xy) - float2(0.5);
                            float2 _9211 = floor(_9209);
                            float2 _9214 = _9209 - _9211;
                            float2 _9217 = _9214 * _9214;
                            float2 _9220 = _9217 * _9214;
                            float2 _9239 = (((_9220 * 3.0) - (_9217 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _9252 = _9220 * 0.16666667163372039794921875;
                            float2 _9255 = (((((-_9220) + (_9217 * 3.0)) - (_9214 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9239;
                            float2 _9258 = (((((_9220 * (-3.0)) + (_9217 * 3.0)) + (_9214 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9252;
                            float2 _9268 = ((_9211 - float2(0.5)) + (_9239 / _9255)) * _172.gLevel[2].zw;
                            float2 _9278 = ((_9211 + float2(1.5)) + (_9252 / _9258)) * _172.gLevel[2].zw;
                            float2 _21935;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20457 = _9268;
                                _20457.y = 1.0 - _9268.y;
                                _21935 = _20457;
                            }
                            else
                            {
                                _21935 = _9268;
                            }
                            float _9298 = _9268.y;
                            float2 _9299 = float2(_9278.x, _9298);
                            float2 _21936;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20463 = _9299;
                                _20463.y = 1.0 - _9298;
                                _21936 = _20463;
                            }
                            else
                            {
                                _21936 = _9299;
                            }
                            float _9315 = _9278.y;
                            float2 _9316 = float2(_9268.x, _9315);
                            float2 _21937;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20470 = _9316;
                                _20470.y = 1.0 - _9315;
                                _21937 = _20470;
                            }
                            else
                            {
                                _21937 = _9316;
                            }
                            float2 _21938;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20476 = _9278;
                                _20476.y = 1.0 - _9278.y;
                                _21938 = _20476;
                            }
                            else
                            {
                                _21938 = _9278;
                            }
                            _21946 = (((gBackdrop2.sample(gLinear, _21935, level(0.0)) * _9255.x) + (gBackdrop2.sample(gLinear, _21936, level(0.0)) * _9258.x)) * _9255.y) + (((gBackdrop2.sample(gLinear, _21937, level(0.0)) * _9255.x) + (gBackdrop2.sample(gLinear, _21938, level(0.0)) * _9258.x)) * _9258.y);
                        }
                        else
                        {
                            float4 _21947;
                            if (_8863 == 3)
                            {
                                float2 _9416 = (_6502 * _172.gLevel[3].xy) - float2(0.5);
                                float2 _9418 = floor(_9416);
                                float2 _9421 = _9416 - _9418;
                                float2 _9424 = _9421 * _9421;
                                float2 _9427 = _9424 * _9421;
                                float2 _9446 = (((_9427 * 3.0) - (_9424 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _9459 = _9427 * 0.16666667163372039794921875;
                                float2 _9462 = (((((-_9427) + (_9424 * 3.0)) - (_9421 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9446;
                                float2 _9465 = (((((_9427 * (-3.0)) + (_9424 * 3.0)) + (_9421 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9459;
                                float2 _9475 = ((_9418 - float2(0.5)) + (_9446 / _9462)) * _172.gLevel[3].zw;
                                float2 _9485 = ((_9418 + float2(1.5)) + (_9459 / _9465)) * _172.gLevel[3].zw;
                                float2 _21931;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20483 = _9475;
                                    _20483.y = 1.0 - _9475.y;
                                    _21931 = _20483;
                                }
                                else
                                {
                                    _21931 = _9475;
                                }
                                float _9505 = _9475.y;
                                float2 _9506 = float2(_9485.x, _9505);
                                float2 _21932;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20489 = _9506;
                                    _20489.y = 1.0 - _9505;
                                    _21932 = _20489;
                                }
                                else
                                {
                                    _21932 = _9506;
                                }
                                float _9522 = _9485.y;
                                float2 _9523 = float2(_9475.x, _9522);
                                float2 _21933;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20496 = _9523;
                                    _20496.y = 1.0 - _9522;
                                    _21933 = _20496;
                                }
                                else
                                {
                                    _21933 = _9523;
                                }
                                float2 _21934;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20502 = _9485;
                                    _20502.y = 1.0 - _9485.y;
                                    _21934 = _20502;
                                }
                                else
                                {
                                    _21934 = _9485;
                                }
                                _21947 = (((gBackdrop3.sample(gLinear, _21931, level(0.0)) * _9462.x) + (gBackdrop3.sample(gLinear, _21932, level(0.0)) * _9465.x)) * _9462.y) + (((gBackdrop3.sample(gLinear, _21933, level(0.0)) * _9462.x) + (gBackdrop3.sample(gLinear, _21934, level(0.0)) * _9465.x)) * _9465.y);
                            }
                            else
                            {
                                float4 _21948;
                                if (_8863 == 4)
                                {
                                    float2 _9623 = (_6502 * _172.gLevel[4].xy) - float2(0.5);
                                    float2 _9625 = floor(_9623);
                                    float2 _9628 = _9623 - _9625;
                                    float2 _9631 = _9628 * _9628;
                                    float2 _9634 = _9631 * _9628;
                                    float2 _9653 = (((_9634 * 3.0) - (_9631 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _9666 = _9634 * 0.16666667163372039794921875;
                                    float2 _9669 = (((((-_9634) + (_9631 * 3.0)) - (_9628 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9653;
                                    float2 _9672 = (((((_9634 * (-3.0)) + (_9631 * 3.0)) + (_9628 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9666;
                                    float2 _9682 = ((_9625 - float2(0.5)) + (_9653 / _9669)) * _172.gLevel[4].zw;
                                    float2 _9692 = ((_9625 + float2(1.5)) + (_9666 / _9672)) * _172.gLevel[4].zw;
                                    float2 _21927;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20509 = _9682;
                                        _20509.y = 1.0 - _9682.y;
                                        _21927 = _20509;
                                    }
                                    else
                                    {
                                        _21927 = _9682;
                                    }
                                    float _9712 = _9682.y;
                                    float2 _9713 = float2(_9692.x, _9712);
                                    float2 _21928;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20515 = _9713;
                                        _20515.y = 1.0 - _9712;
                                        _21928 = _20515;
                                    }
                                    else
                                    {
                                        _21928 = _9713;
                                    }
                                    float _9729 = _9692.y;
                                    float2 _9730 = float2(_9682.x, _9729);
                                    float2 _21929;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20522 = _9730;
                                        _20522.y = 1.0 - _9729;
                                        _21929 = _20522;
                                    }
                                    else
                                    {
                                        _21929 = _9730;
                                    }
                                    float2 _21930;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20528 = _9692;
                                        _20528.y = 1.0 - _9692.y;
                                        _21930 = _20528;
                                    }
                                    else
                                    {
                                        _21930 = _9692;
                                    }
                                    _21948 = (((gBackdrop4.sample(gLinear, _21927, level(0.0)) * _9669.x) + (gBackdrop4.sample(gLinear, _21928, level(0.0)) * _9672.x)) * _9669.y) + (((gBackdrop4.sample(gLinear, _21929, level(0.0)) * _9669.x) + (gBackdrop4.sample(gLinear, _21930, level(0.0)) * _9672.x)) * _9672.y);
                                }
                                else
                                {
                                    float2 _9830 = (_6502 * _172.gLevel[5].xy) - float2(0.5);
                                    float2 _9832 = floor(_9830);
                                    float2 _9835 = _9830 - _9832;
                                    float2 _9838 = _9835 * _9835;
                                    float2 _9841 = _9838 * _9835;
                                    float2 _9860 = (((_9841 * 3.0) - (_9838 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _9873 = _9841 * 0.16666667163372039794921875;
                                    float2 _9876 = (((((-_9841) + (_9838 * 3.0)) - (_9835 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9860;
                                    float2 _9879 = (((((_9841 * (-3.0)) + (_9838 * 3.0)) + (_9835 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9873;
                                    float2 _9889 = ((_9832 - float2(0.5)) + (_9860 / _9876)) * _172.gLevel[5].zw;
                                    float2 _9899 = ((_9832 + float2(1.5)) + (_9873 / _9879)) * _172.gLevel[5].zw;
                                    float2 _21923;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20535 = _9889;
                                        _20535.y = 1.0 - _9889.y;
                                        _21923 = _20535;
                                    }
                                    else
                                    {
                                        _21923 = _9889;
                                    }
                                    float _9919 = _9889.y;
                                    float2 _9920 = float2(_9899.x, _9919);
                                    float2 _21924;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20541 = _9920;
                                        _20541.y = 1.0 - _9919;
                                        _21924 = _20541;
                                    }
                                    else
                                    {
                                        _21924 = _9920;
                                    }
                                    float _9936 = _9899.y;
                                    float2 _9937 = float2(_9889.x, _9936);
                                    float2 _21925;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20548 = _9937;
                                        _20548.y = 1.0 - _9936;
                                        _21925 = _20548;
                                    }
                                    else
                                    {
                                        _21925 = _9937;
                                    }
                                    float2 _21926;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20554 = _9899;
                                        _20554.y = 1.0 - _9899.y;
                                        _21926 = _20554;
                                    }
                                    else
                                    {
                                        _21926 = _9899;
                                    }
                                    _21948 = (((gBackdrop5.sample(gLinear, _21923, level(0.0)) * _9876.x) + (gBackdrop5.sample(gLinear, _21924, level(0.0)) * _9879.x)) * _9876.y) + (((gBackdrop5.sample(gLinear, _21925, level(0.0)) * _9876.x) + (gBackdrop5.sample(gLinear, _21926, level(0.0)) * _9879.x)) * _9879.y);
                                }
                                _21947 = _21948;
                            }
                            _21946 = _21947;
                        }
                        _21945 = _21946;
                    }
                    _21944 = _21945;
                }
                float3 _21975;
                if ((_8867 > 0.0199999995529651641845703125) && (_8863 < 5))
                {
                    int _8880 = _8863 + 1;
                    float4 _21970;
                    if (_8880 <= 0)
                    {
                        float2 _21969;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20559 = _6502;
                            _20559.y = 1.0 - _6502.y;
                            _21969 = _20559;
                        }
                        else
                        {
                            _21969 = _6502;
                        }
                        _21970 = gBackdrop0.sample(gLinear, _21969, level(0.0));
                    }
                    else
                    {
                        float4 _21971;
                        if (_8880 == 1)
                        {
                            float2 _10126 = (_6502 * _172.gLevel[1].xy) - float2(0.5);
                            float2 _10128 = floor(_10126);
                            float2 _10131 = _10126 - _10128;
                            float2 _10134 = _10131 * _10131;
                            float2 _10137 = _10134 * _10131;
                            float2 _10156 = (((_10137 * 3.0) - (_10134 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _10169 = _10137 * 0.16666667163372039794921875;
                            float2 _10172 = (((((-_10137) + (_10134 * 3.0)) - (_10131 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10156;
                            float2 _10175 = (((((_10137 * (-3.0)) + (_10134 * 3.0)) + (_10131 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10169;
                            float2 _10185 = ((_10128 - float2(0.5)) + (_10156 / _10172)) * _172.gLevel[1].zw;
                            float2 _10195 = ((_10128 + float2(1.5)) + (_10169 / _10175)) * _172.gLevel[1].zw;
                            float2 _21965;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20564 = _10185;
                                _20564.y = 1.0 - _10185.y;
                                _21965 = _20564;
                            }
                            else
                            {
                                _21965 = _10185;
                            }
                            float _10215 = _10185.y;
                            float2 _10216 = float2(_10195.x, _10215);
                            float2 _21966;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20570 = _10216;
                                _20570.y = 1.0 - _10215;
                                _21966 = _20570;
                            }
                            else
                            {
                                _21966 = _10216;
                            }
                            float _10232 = _10195.y;
                            float2 _10233 = float2(_10185.x, _10232);
                            float2 _21967;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20577 = _10233;
                                _20577.y = 1.0 - _10232;
                                _21967 = _20577;
                            }
                            else
                            {
                                _21967 = _10233;
                            }
                            float2 _21968;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20583 = _10195;
                                _20583.y = 1.0 - _10195.y;
                                _21968 = _20583;
                            }
                            else
                            {
                                _21968 = _10195;
                            }
                            _21971 = (((gBackdrop1.sample(gLinear, _21965, level(0.0)) * _10172.x) + (gBackdrop1.sample(gLinear, _21966, level(0.0)) * _10175.x)) * _10172.y) + (((gBackdrop1.sample(gLinear, _21967, level(0.0)) * _10172.x) + (gBackdrop1.sample(gLinear, _21968, level(0.0)) * _10175.x)) * _10175.y);
                        }
                        else
                        {
                            float4 _21972;
                            if (_8880 == 2)
                            {
                                float2 _10333 = (_6502 * _172.gLevel[2].xy) - float2(0.5);
                                float2 _10335 = floor(_10333);
                                float2 _10338 = _10333 - _10335;
                                float2 _10341 = _10338 * _10338;
                                float2 _10344 = _10341 * _10338;
                                float2 _10363 = (((_10344 * 3.0) - (_10341 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _10376 = _10344 * 0.16666667163372039794921875;
                                float2 _10379 = (((((-_10344) + (_10341 * 3.0)) - (_10338 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10363;
                                float2 _10382 = (((((_10344 * (-3.0)) + (_10341 * 3.0)) + (_10338 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10376;
                                float2 _10392 = ((_10335 - float2(0.5)) + (_10363 / _10379)) * _172.gLevel[2].zw;
                                float2 _10402 = ((_10335 + float2(1.5)) + (_10376 / _10382)) * _172.gLevel[2].zw;
                                float2 _21961;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20590 = _10392;
                                    _20590.y = 1.0 - _10392.y;
                                    _21961 = _20590;
                                }
                                else
                                {
                                    _21961 = _10392;
                                }
                                float _10422 = _10392.y;
                                float2 _10423 = float2(_10402.x, _10422);
                                float2 _21962;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20596 = _10423;
                                    _20596.y = 1.0 - _10422;
                                    _21962 = _20596;
                                }
                                else
                                {
                                    _21962 = _10423;
                                }
                                float _10439 = _10402.y;
                                float2 _10440 = float2(_10392.x, _10439);
                                float2 _21963;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20603 = _10440;
                                    _20603.y = 1.0 - _10439;
                                    _21963 = _20603;
                                }
                                else
                                {
                                    _21963 = _10440;
                                }
                                float2 _21964;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20609 = _10402;
                                    _20609.y = 1.0 - _10402.y;
                                    _21964 = _20609;
                                }
                                else
                                {
                                    _21964 = _10402;
                                }
                                _21972 = (((gBackdrop2.sample(gLinear, _21961, level(0.0)) * _10379.x) + (gBackdrop2.sample(gLinear, _21962, level(0.0)) * _10382.x)) * _10379.y) + (((gBackdrop2.sample(gLinear, _21963, level(0.0)) * _10379.x) + (gBackdrop2.sample(gLinear, _21964, level(0.0)) * _10382.x)) * _10382.y);
                            }
                            else
                            {
                                float4 _21973;
                                if (_8880 == 3)
                                {
                                    float2 _10540 = (_6502 * _172.gLevel[3].xy) - float2(0.5);
                                    float2 _10542 = floor(_10540);
                                    float2 _10545 = _10540 - _10542;
                                    float2 _10548 = _10545 * _10545;
                                    float2 _10551 = _10548 * _10545;
                                    float2 _10570 = (((_10551 * 3.0) - (_10548 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _10583 = _10551 * 0.16666667163372039794921875;
                                    float2 _10586 = (((((-_10551) + (_10548 * 3.0)) - (_10545 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10570;
                                    float2 _10589 = (((((_10551 * (-3.0)) + (_10548 * 3.0)) + (_10545 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10583;
                                    float2 _10599 = ((_10542 - float2(0.5)) + (_10570 / _10586)) * _172.gLevel[3].zw;
                                    float2 _10609 = ((_10542 + float2(1.5)) + (_10583 / _10589)) * _172.gLevel[3].zw;
                                    float2 _21957;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20616 = _10599;
                                        _20616.y = 1.0 - _10599.y;
                                        _21957 = _20616;
                                    }
                                    else
                                    {
                                        _21957 = _10599;
                                    }
                                    float _10629 = _10599.y;
                                    float2 _10630 = float2(_10609.x, _10629);
                                    float2 _21958;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20622 = _10630;
                                        _20622.y = 1.0 - _10629;
                                        _21958 = _20622;
                                    }
                                    else
                                    {
                                        _21958 = _10630;
                                    }
                                    float _10646 = _10609.y;
                                    float2 _10647 = float2(_10599.x, _10646);
                                    float2 _21959;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20629 = _10647;
                                        _20629.y = 1.0 - _10646;
                                        _21959 = _20629;
                                    }
                                    else
                                    {
                                        _21959 = _10647;
                                    }
                                    float2 _21960;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20635 = _10609;
                                        _20635.y = 1.0 - _10609.y;
                                        _21960 = _20635;
                                    }
                                    else
                                    {
                                        _21960 = _10609;
                                    }
                                    _21973 = (((gBackdrop3.sample(gLinear, _21957, level(0.0)) * _10586.x) + (gBackdrop3.sample(gLinear, _21958, level(0.0)) * _10589.x)) * _10586.y) + (((gBackdrop3.sample(gLinear, _21959, level(0.0)) * _10586.x) + (gBackdrop3.sample(gLinear, _21960, level(0.0)) * _10589.x)) * _10589.y);
                                }
                                else
                                {
                                    float4 _21974;
                                    if (_8880 == 4)
                                    {
                                        float2 _10747 = (_6502 * _172.gLevel[4].xy) - float2(0.5);
                                        float2 _10749 = floor(_10747);
                                        float2 _10752 = _10747 - _10749;
                                        float2 _10755 = _10752 * _10752;
                                        float2 _10758 = _10755 * _10752;
                                        float2 _10777 = (((_10758 * 3.0) - (_10755 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _10790 = _10758 * 0.16666667163372039794921875;
                                        float2 _10793 = (((((-_10758) + (_10755 * 3.0)) - (_10752 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10777;
                                        float2 _10796 = (((((_10758 * (-3.0)) + (_10755 * 3.0)) + (_10752 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10790;
                                        float2 _10806 = ((_10749 - float2(0.5)) + (_10777 / _10793)) * _172.gLevel[4].zw;
                                        float2 _10816 = ((_10749 + float2(1.5)) + (_10790 / _10796)) * _172.gLevel[4].zw;
                                        float2 _21953;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20642 = _10806;
                                            _20642.y = 1.0 - _10806.y;
                                            _21953 = _20642;
                                        }
                                        else
                                        {
                                            _21953 = _10806;
                                        }
                                        float _10836 = _10806.y;
                                        float2 _10837 = float2(_10816.x, _10836);
                                        float2 _21954;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20648 = _10837;
                                            _20648.y = 1.0 - _10836;
                                            _21954 = _20648;
                                        }
                                        else
                                        {
                                            _21954 = _10837;
                                        }
                                        float _10853 = _10816.y;
                                        float2 _10854 = float2(_10806.x, _10853);
                                        float2 _21955;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20655 = _10854;
                                            _20655.y = 1.0 - _10853;
                                            _21955 = _20655;
                                        }
                                        else
                                        {
                                            _21955 = _10854;
                                        }
                                        float2 _21956;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20661 = _10816;
                                            _20661.y = 1.0 - _10816.y;
                                            _21956 = _20661;
                                        }
                                        else
                                        {
                                            _21956 = _10816;
                                        }
                                        _21974 = (((gBackdrop4.sample(gLinear, _21953, level(0.0)) * _10793.x) + (gBackdrop4.sample(gLinear, _21954, level(0.0)) * _10796.x)) * _10793.y) + (((gBackdrop4.sample(gLinear, _21955, level(0.0)) * _10793.x) + (gBackdrop4.sample(gLinear, _21956, level(0.0)) * _10796.x)) * _10796.y);
                                    }
                                    else
                                    {
                                        float2 _10954 = (_6502 * _172.gLevel[5].xy) - float2(0.5);
                                        float2 _10956 = floor(_10954);
                                        float2 _10959 = _10954 - _10956;
                                        float2 _10962 = _10959 * _10959;
                                        float2 _10965 = _10962 * _10959;
                                        float2 _10984 = (((_10965 * 3.0) - (_10962 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _10997 = _10965 * 0.16666667163372039794921875;
                                        float2 _11000 = (((((-_10965) + (_10962 * 3.0)) - (_10959 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10984;
                                        float2 _11003 = (((((_10965 * (-3.0)) + (_10962 * 3.0)) + (_10959 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10997;
                                        float2 _11013 = ((_10956 - float2(0.5)) + (_10984 / _11000)) * _172.gLevel[5].zw;
                                        float2 _11023 = ((_10956 + float2(1.5)) + (_10997 / _11003)) * _172.gLevel[5].zw;
                                        float2 _21949;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20668 = _11013;
                                            _20668.y = 1.0 - _11013.y;
                                            _21949 = _20668;
                                        }
                                        else
                                        {
                                            _21949 = _11013;
                                        }
                                        float _11043 = _11013.y;
                                        float2 _11044 = float2(_11023.x, _11043);
                                        float2 _21950;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20674 = _11044;
                                            _20674.y = 1.0 - _11043;
                                            _21950 = _20674;
                                        }
                                        else
                                        {
                                            _21950 = _11044;
                                        }
                                        float _11060 = _11023.y;
                                        float2 _11061 = float2(_11013.x, _11060);
                                        float2 _21951;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20681 = _11061;
                                            _20681.y = 1.0 - _11060;
                                            _21951 = _20681;
                                        }
                                        else
                                        {
                                            _21951 = _11061;
                                        }
                                        float2 _21952;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20687 = _11023;
                                            _20687.y = 1.0 - _11023.y;
                                            _21952 = _20687;
                                        }
                                        else
                                        {
                                            _21952 = _11023;
                                        }
                                        _21974 = (((gBackdrop5.sample(gLinear, _21949, level(0.0)) * _11000.x) + (gBackdrop5.sample(gLinear, _21950, level(0.0)) * _11003.x)) * _11000.y) + (((gBackdrop5.sample(gLinear, _21951, level(0.0)) * _11000.x) + (gBackdrop5.sample(gLinear, _21952, level(0.0)) * _11003.x)) * _11003.y);
                                    }
                                    _21973 = _21974;
                                }
                                _21972 = _21973;
                            }
                            _21971 = _21972;
                        }
                        _21970 = _21971;
                    }
                    _21975 = mix(_21944.xyz, _21970.xyz, float3(_8867));
                }
                else
                {
                    _21975 = _21944.xyz;
                }
                float2 _6509 = _6458 + _6470;
                float _11151 = fast::clamp(log2(fast::max(_6372, 1.0)) - 1.0, 0.0, 5.0);
                int _11154 = int(floor(_11151));
                float _11158 = _11151 - float(_11154);
                float4 _22050;
                if (_11154 <= 0)
                {
                    float2 _22049;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _20694 = _6509;
                        _20694.y = 1.0 - _6509.y;
                        _22049 = _20694;
                    }
                    else
                    {
                        _22049 = _6509;
                    }
                    _22050 = gBackdrop0.sample(gLinear, _22049, level(0.0));
                }
                else
                {
                    float4 _22051;
                    if (_11154 == 1)
                    {
                        float2 _11293 = (_6509 * _172.gLevel[1].xy) - float2(0.5);
                        float2 _11295 = floor(_11293);
                        float2 _11298 = _11293 - _11295;
                        float2 _11301 = _11298 * _11298;
                        float2 _11304 = _11301 * _11298;
                        float2 _11323 = (((_11304 * 3.0) - (_11301 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _11336 = _11304 * 0.16666667163372039794921875;
                        float2 _11339 = (((((-_11304) + (_11301 * 3.0)) - (_11298 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11323;
                        float2 _11342 = (((((_11304 * (-3.0)) + (_11301 * 3.0)) + (_11298 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11336;
                        float2 _11352 = ((_11295 - float2(0.5)) + (_11323 / _11339)) * _172.gLevel[1].zw;
                        float2 _11362 = ((_11295 + float2(1.5)) + (_11336 / _11342)) * _172.gLevel[1].zw;
                        float2 _22045;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20699 = _11352;
                            _20699.y = 1.0 - _11352.y;
                            _22045 = _20699;
                        }
                        else
                        {
                            _22045 = _11352;
                        }
                        float _11382 = _11352.y;
                        float2 _11383 = float2(_11362.x, _11382);
                        float2 _22046;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20705 = _11383;
                            _20705.y = 1.0 - _11382;
                            _22046 = _20705;
                        }
                        else
                        {
                            _22046 = _11383;
                        }
                        float _11399 = _11362.y;
                        float2 _11400 = float2(_11352.x, _11399);
                        float2 _22047;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20712 = _11400;
                            _20712.y = 1.0 - _11399;
                            _22047 = _20712;
                        }
                        else
                        {
                            _22047 = _11400;
                        }
                        float2 _22048;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20718 = _11362;
                            _20718.y = 1.0 - _11362.y;
                            _22048 = _20718;
                        }
                        else
                        {
                            _22048 = _11362;
                        }
                        _22051 = (((gBackdrop1.sample(gLinear, _22045, level(0.0)) * _11339.x) + (gBackdrop1.sample(gLinear, _22046, level(0.0)) * _11342.x)) * _11339.y) + (((gBackdrop1.sample(gLinear, _22047, level(0.0)) * _11339.x) + (gBackdrop1.sample(gLinear, _22048, level(0.0)) * _11342.x)) * _11342.y);
                    }
                    else
                    {
                        float4 _22052;
                        if (_11154 == 2)
                        {
                            float2 _11500 = (_6509 * _172.gLevel[2].xy) - float2(0.5);
                            float2 _11502 = floor(_11500);
                            float2 _11505 = _11500 - _11502;
                            float2 _11508 = _11505 * _11505;
                            float2 _11511 = _11508 * _11505;
                            float2 _11530 = (((_11511 * 3.0) - (_11508 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _11543 = _11511 * 0.16666667163372039794921875;
                            float2 _11546 = (((((-_11511) + (_11508 * 3.0)) - (_11505 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11530;
                            float2 _11549 = (((((_11511 * (-3.0)) + (_11508 * 3.0)) + (_11505 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11543;
                            float2 _11559 = ((_11502 - float2(0.5)) + (_11530 / _11546)) * _172.gLevel[2].zw;
                            float2 _11569 = ((_11502 + float2(1.5)) + (_11543 / _11549)) * _172.gLevel[2].zw;
                            float2 _22041;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20725 = _11559;
                                _20725.y = 1.0 - _11559.y;
                                _22041 = _20725;
                            }
                            else
                            {
                                _22041 = _11559;
                            }
                            float _11589 = _11559.y;
                            float2 _11590 = float2(_11569.x, _11589);
                            float2 _22042;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20731 = _11590;
                                _20731.y = 1.0 - _11589;
                                _22042 = _20731;
                            }
                            else
                            {
                                _22042 = _11590;
                            }
                            float _11606 = _11569.y;
                            float2 _11607 = float2(_11559.x, _11606);
                            float2 _22043;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20738 = _11607;
                                _20738.y = 1.0 - _11606;
                                _22043 = _20738;
                            }
                            else
                            {
                                _22043 = _11607;
                            }
                            float2 _22044;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20744 = _11569;
                                _20744.y = 1.0 - _11569.y;
                                _22044 = _20744;
                            }
                            else
                            {
                                _22044 = _11569;
                            }
                            _22052 = (((gBackdrop2.sample(gLinear, _22041, level(0.0)) * _11546.x) + (gBackdrop2.sample(gLinear, _22042, level(0.0)) * _11549.x)) * _11546.y) + (((gBackdrop2.sample(gLinear, _22043, level(0.0)) * _11546.x) + (gBackdrop2.sample(gLinear, _22044, level(0.0)) * _11549.x)) * _11549.y);
                        }
                        else
                        {
                            float4 _22053;
                            if (_11154 == 3)
                            {
                                float2 _11707 = (_6509 * _172.gLevel[3].xy) - float2(0.5);
                                float2 _11709 = floor(_11707);
                                float2 _11712 = _11707 - _11709;
                                float2 _11715 = _11712 * _11712;
                                float2 _11718 = _11715 * _11712;
                                float2 _11737 = (((_11718 * 3.0) - (_11715 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _11750 = _11718 * 0.16666667163372039794921875;
                                float2 _11753 = (((((-_11718) + (_11715 * 3.0)) - (_11712 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11737;
                                float2 _11756 = (((((_11718 * (-3.0)) + (_11715 * 3.0)) + (_11712 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11750;
                                float2 _11766 = ((_11709 - float2(0.5)) + (_11737 / _11753)) * _172.gLevel[3].zw;
                                float2 _11776 = ((_11709 + float2(1.5)) + (_11750 / _11756)) * _172.gLevel[3].zw;
                                float2 _22037;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20751 = _11766;
                                    _20751.y = 1.0 - _11766.y;
                                    _22037 = _20751;
                                }
                                else
                                {
                                    _22037 = _11766;
                                }
                                float _11796 = _11766.y;
                                float2 _11797 = float2(_11776.x, _11796);
                                float2 _22038;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20757 = _11797;
                                    _20757.y = 1.0 - _11796;
                                    _22038 = _20757;
                                }
                                else
                                {
                                    _22038 = _11797;
                                }
                                float _11813 = _11776.y;
                                float2 _11814 = float2(_11766.x, _11813);
                                float2 _22039;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20764 = _11814;
                                    _20764.y = 1.0 - _11813;
                                    _22039 = _20764;
                                }
                                else
                                {
                                    _22039 = _11814;
                                }
                                float2 _22040;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20770 = _11776;
                                    _20770.y = 1.0 - _11776.y;
                                    _22040 = _20770;
                                }
                                else
                                {
                                    _22040 = _11776;
                                }
                                _22053 = (((gBackdrop3.sample(gLinear, _22037, level(0.0)) * _11753.x) + (gBackdrop3.sample(gLinear, _22038, level(0.0)) * _11756.x)) * _11753.y) + (((gBackdrop3.sample(gLinear, _22039, level(0.0)) * _11753.x) + (gBackdrop3.sample(gLinear, _22040, level(0.0)) * _11756.x)) * _11756.y);
                            }
                            else
                            {
                                float4 _22054;
                                if (_11154 == 4)
                                {
                                    float2 _11914 = (_6509 * _172.gLevel[4].xy) - float2(0.5);
                                    float2 _11916 = floor(_11914);
                                    float2 _11919 = _11914 - _11916;
                                    float2 _11922 = _11919 * _11919;
                                    float2 _11925 = _11922 * _11919;
                                    float2 _11944 = (((_11925 * 3.0) - (_11922 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _11957 = _11925 * 0.16666667163372039794921875;
                                    float2 _11960 = (((((-_11925) + (_11922 * 3.0)) - (_11919 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11944;
                                    float2 _11963 = (((((_11925 * (-3.0)) + (_11922 * 3.0)) + (_11919 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11957;
                                    float2 _11973 = ((_11916 - float2(0.5)) + (_11944 / _11960)) * _172.gLevel[4].zw;
                                    float2 _11983 = ((_11916 + float2(1.5)) + (_11957 / _11963)) * _172.gLevel[4].zw;
                                    float2 _22033;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20777 = _11973;
                                        _20777.y = 1.0 - _11973.y;
                                        _22033 = _20777;
                                    }
                                    else
                                    {
                                        _22033 = _11973;
                                    }
                                    float _12003 = _11973.y;
                                    float2 _12004 = float2(_11983.x, _12003);
                                    float2 _22034;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20783 = _12004;
                                        _20783.y = 1.0 - _12003;
                                        _22034 = _20783;
                                    }
                                    else
                                    {
                                        _22034 = _12004;
                                    }
                                    float _12020 = _11983.y;
                                    float2 _12021 = float2(_11973.x, _12020);
                                    float2 _22035;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20790 = _12021;
                                        _20790.y = 1.0 - _12020;
                                        _22035 = _20790;
                                    }
                                    else
                                    {
                                        _22035 = _12021;
                                    }
                                    float2 _22036;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20796 = _11983;
                                        _20796.y = 1.0 - _11983.y;
                                        _22036 = _20796;
                                    }
                                    else
                                    {
                                        _22036 = _11983;
                                    }
                                    _22054 = (((gBackdrop4.sample(gLinear, _22033, level(0.0)) * _11960.x) + (gBackdrop4.sample(gLinear, _22034, level(0.0)) * _11963.x)) * _11960.y) + (((gBackdrop4.sample(gLinear, _22035, level(0.0)) * _11960.x) + (gBackdrop4.sample(gLinear, _22036, level(0.0)) * _11963.x)) * _11963.y);
                                }
                                else
                                {
                                    float2 _12121 = (_6509 * _172.gLevel[5].xy) - float2(0.5);
                                    float2 _12123 = floor(_12121);
                                    float2 _12126 = _12121 - _12123;
                                    float2 _12129 = _12126 * _12126;
                                    float2 _12132 = _12129 * _12126;
                                    float2 _12151 = (((_12132 * 3.0) - (_12129 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _12164 = _12132 * 0.16666667163372039794921875;
                                    float2 _12167 = (((((-_12132) + (_12129 * 3.0)) - (_12126 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12151;
                                    float2 _12170 = (((((_12132 * (-3.0)) + (_12129 * 3.0)) + (_12126 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12164;
                                    float2 _12180 = ((_12123 - float2(0.5)) + (_12151 / _12167)) * _172.gLevel[5].zw;
                                    float2 _12190 = ((_12123 + float2(1.5)) + (_12164 / _12170)) * _172.gLevel[5].zw;
                                    float2 _22029;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20803 = _12180;
                                        _20803.y = 1.0 - _12180.y;
                                        _22029 = _20803;
                                    }
                                    else
                                    {
                                        _22029 = _12180;
                                    }
                                    float _12210 = _12180.y;
                                    float2 _12211 = float2(_12190.x, _12210);
                                    float2 _22030;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20809 = _12211;
                                        _20809.y = 1.0 - _12210;
                                        _22030 = _20809;
                                    }
                                    else
                                    {
                                        _22030 = _12211;
                                    }
                                    float _12227 = _12190.y;
                                    float2 _12228 = float2(_12180.x, _12227);
                                    float2 _22031;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20816 = _12228;
                                        _20816.y = 1.0 - _12227;
                                        _22031 = _20816;
                                    }
                                    else
                                    {
                                        _22031 = _12228;
                                    }
                                    float2 _22032;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20822 = _12190;
                                        _20822.y = 1.0 - _12190.y;
                                        _22032 = _20822;
                                    }
                                    else
                                    {
                                        _22032 = _12190;
                                    }
                                    _22054 = (((gBackdrop5.sample(gLinear, _22029, level(0.0)) * _12167.x) + (gBackdrop5.sample(gLinear, _22030, level(0.0)) * _12170.x)) * _12167.y) + (((gBackdrop5.sample(gLinear, _22031, level(0.0)) * _12167.x) + (gBackdrop5.sample(gLinear, _22032, level(0.0)) * _12170.x)) * _12170.y);
                                }
                                _22053 = _22054;
                            }
                            _22052 = _22053;
                        }
                        _22051 = _22052;
                    }
                    _22050 = _22051;
                }
                float3 _22081;
                if ((_11158 > 0.0199999995529651641845703125) && (_11154 < 5))
                {
                    int _11171 = _11154 + 1;
                    float4 _22076;
                    if (_11171 <= 0)
                    {
                        float2 _22075;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20827 = _6509;
                            _20827.y = 1.0 - _6509.y;
                            _22075 = _20827;
                        }
                        else
                        {
                            _22075 = _6509;
                        }
                        _22076 = gBackdrop0.sample(gLinear, _22075, level(0.0));
                    }
                    else
                    {
                        float4 _22077;
                        if (_11171 == 1)
                        {
                            float2 _12417 = (_6509 * _172.gLevel[1].xy) - float2(0.5);
                            float2 _12419 = floor(_12417);
                            float2 _12422 = _12417 - _12419;
                            float2 _12425 = _12422 * _12422;
                            float2 _12428 = _12425 * _12422;
                            float2 _12447 = (((_12428 * 3.0) - (_12425 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _12460 = _12428 * 0.16666667163372039794921875;
                            float2 _12463 = (((((-_12428) + (_12425 * 3.0)) - (_12422 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12447;
                            float2 _12466 = (((((_12428 * (-3.0)) + (_12425 * 3.0)) + (_12422 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12460;
                            float2 _12476 = ((_12419 - float2(0.5)) + (_12447 / _12463)) * _172.gLevel[1].zw;
                            float2 _12486 = ((_12419 + float2(1.5)) + (_12460 / _12466)) * _172.gLevel[1].zw;
                            float2 _22071;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20832 = _12476;
                                _20832.y = 1.0 - _12476.y;
                                _22071 = _20832;
                            }
                            else
                            {
                                _22071 = _12476;
                            }
                            float _12506 = _12476.y;
                            float2 _12507 = float2(_12486.x, _12506);
                            float2 _22072;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20838 = _12507;
                                _20838.y = 1.0 - _12506;
                                _22072 = _20838;
                            }
                            else
                            {
                                _22072 = _12507;
                            }
                            float _12523 = _12486.y;
                            float2 _12524 = float2(_12476.x, _12523);
                            float2 _22073;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20845 = _12524;
                                _20845.y = 1.0 - _12523;
                                _22073 = _20845;
                            }
                            else
                            {
                                _22073 = _12524;
                            }
                            float2 _22074;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20851 = _12486;
                                _20851.y = 1.0 - _12486.y;
                                _22074 = _20851;
                            }
                            else
                            {
                                _22074 = _12486;
                            }
                            _22077 = (((gBackdrop1.sample(gLinear, _22071, level(0.0)) * _12463.x) + (gBackdrop1.sample(gLinear, _22072, level(0.0)) * _12466.x)) * _12463.y) + (((gBackdrop1.sample(gLinear, _22073, level(0.0)) * _12463.x) + (gBackdrop1.sample(gLinear, _22074, level(0.0)) * _12466.x)) * _12466.y);
                        }
                        else
                        {
                            float4 _22078;
                            if (_11171 == 2)
                            {
                                float2 _12624 = (_6509 * _172.gLevel[2].xy) - float2(0.5);
                                float2 _12626 = floor(_12624);
                                float2 _12629 = _12624 - _12626;
                                float2 _12632 = _12629 * _12629;
                                float2 _12635 = _12632 * _12629;
                                float2 _12654 = (((_12635 * 3.0) - (_12632 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _12667 = _12635 * 0.16666667163372039794921875;
                                float2 _12670 = (((((-_12635) + (_12632 * 3.0)) - (_12629 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12654;
                                float2 _12673 = (((((_12635 * (-3.0)) + (_12632 * 3.0)) + (_12629 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12667;
                                float2 _12683 = ((_12626 - float2(0.5)) + (_12654 / _12670)) * _172.gLevel[2].zw;
                                float2 _12693 = ((_12626 + float2(1.5)) + (_12667 / _12673)) * _172.gLevel[2].zw;
                                float2 _22067;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20858 = _12683;
                                    _20858.y = 1.0 - _12683.y;
                                    _22067 = _20858;
                                }
                                else
                                {
                                    _22067 = _12683;
                                }
                                float _12713 = _12683.y;
                                float2 _12714 = float2(_12693.x, _12713);
                                float2 _22068;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20864 = _12714;
                                    _20864.y = 1.0 - _12713;
                                    _22068 = _20864;
                                }
                                else
                                {
                                    _22068 = _12714;
                                }
                                float _12730 = _12693.y;
                                float2 _12731 = float2(_12683.x, _12730);
                                float2 _22069;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20871 = _12731;
                                    _20871.y = 1.0 - _12730;
                                    _22069 = _20871;
                                }
                                else
                                {
                                    _22069 = _12731;
                                }
                                float2 _22070;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20877 = _12693;
                                    _20877.y = 1.0 - _12693.y;
                                    _22070 = _20877;
                                }
                                else
                                {
                                    _22070 = _12693;
                                }
                                _22078 = (((gBackdrop2.sample(gLinear, _22067, level(0.0)) * _12670.x) + (gBackdrop2.sample(gLinear, _22068, level(0.0)) * _12673.x)) * _12670.y) + (((gBackdrop2.sample(gLinear, _22069, level(0.0)) * _12670.x) + (gBackdrop2.sample(gLinear, _22070, level(0.0)) * _12673.x)) * _12673.y);
                            }
                            else
                            {
                                float4 _22079;
                                if (_11171 == 3)
                                {
                                    float2 _12831 = (_6509 * _172.gLevel[3].xy) - float2(0.5);
                                    float2 _12833 = floor(_12831);
                                    float2 _12836 = _12831 - _12833;
                                    float2 _12839 = _12836 * _12836;
                                    float2 _12842 = _12839 * _12836;
                                    float2 _12861 = (((_12842 * 3.0) - (_12839 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _12874 = _12842 * 0.16666667163372039794921875;
                                    float2 _12877 = (((((-_12842) + (_12839 * 3.0)) - (_12836 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12861;
                                    float2 _12880 = (((((_12842 * (-3.0)) + (_12839 * 3.0)) + (_12836 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12874;
                                    float2 _12890 = ((_12833 - float2(0.5)) + (_12861 / _12877)) * _172.gLevel[3].zw;
                                    float2 _12900 = ((_12833 + float2(1.5)) + (_12874 / _12880)) * _172.gLevel[3].zw;
                                    float2 _22063;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20884 = _12890;
                                        _20884.y = 1.0 - _12890.y;
                                        _22063 = _20884;
                                    }
                                    else
                                    {
                                        _22063 = _12890;
                                    }
                                    float _12920 = _12890.y;
                                    float2 _12921 = float2(_12900.x, _12920);
                                    float2 _22064;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20890 = _12921;
                                        _20890.y = 1.0 - _12920;
                                        _22064 = _20890;
                                    }
                                    else
                                    {
                                        _22064 = _12921;
                                    }
                                    float _12937 = _12900.y;
                                    float2 _12938 = float2(_12890.x, _12937);
                                    float2 _22065;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20897 = _12938;
                                        _20897.y = 1.0 - _12937;
                                        _22065 = _20897;
                                    }
                                    else
                                    {
                                        _22065 = _12938;
                                    }
                                    float2 _22066;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20903 = _12900;
                                        _20903.y = 1.0 - _12900.y;
                                        _22066 = _20903;
                                    }
                                    else
                                    {
                                        _22066 = _12900;
                                    }
                                    _22079 = (((gBackdrop3.sample(gLinear, _22063, level(0.0)) * _12877.x) + (gBackdrop3.sample(gLinear, _22064, level(0.0)) * _12880.x)) * _12877.y) + (((gBackdrop3.sample(gLinear, _22065, level(0.0)) * _12877.x) + (gBackdrop3.sample(gLinear, _22066, level(0.0)) * _12880.x)) * _12880.y);
                                }
                                else
                                {
                                    float4 _22080;
                                    if (_11171 == 4)
                                    {
                                        float2 _13038 = (_6509 * _172.gLevel[4].xy) - float2(0.5);
                                        float2 _13040 = floor(_13038);
                                        float2 _13043 = _13038 - _13040;
                                        float2 _13046 = _13043 * _13043;
                                        float2 _13049 = _13046 * _13043;
                                        float2 _13068 = (((_13049 * 3.0) - (_13046 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _13081 = _13049 * 0.16666667163372039794921875;
                                        float2 _13084 = (((((-_13049) + (_13046 * 3.0)) - (_13043 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13068;
                                        float2 _13087 = (((((_13049 * (-3.0)) + (_13046 * 3.0)) + (_13043 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13081;
                                        float2 _13097 = ((_13040 - float2(0.5)) + (_13068 / _13084)) * _172.gLevel[4].zw;
                                        float2 _13107 = ((_13040 + float2(1.5)) + (_13081 / _13087)) * _172.gLevel[4].zw;
                                        float2 _22059;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20910 = _13097;
                                            _20910.y = 1.0 - _13097.y;
                                            _22059 = _20910;
                                        }
                                        else
                                        {
                                            _22059 = _13097;
                                        }
                                        float _13127 = _13097.y;
                                        float2 _13128 = float2(_13107.x, _13127);
                                        float2 _22060;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20916 = _13128;
                                            _20916.y = 1.0 - _13127;
                                            _22060 = _20916;
                                        }
                                        else
                                        {
                                            _22060 = _13128;
                                        }
                                        float _13144 = _13107.y;
                                        float2 _13145 = float2(_13097.x, _13144);
                                        float2 _22061;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20923 = _13145;
                                            _20923.y = 1.0 - _13144;
                                            _22061 = _20923;
                                        }
                                        else
                                        {
                                            _22061 = _13145;
                                        }
                                        float2 _22062;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20929 = _13107;
                                            _20929.y = 1.0 - _13107.y;
                                            _22062 = _20929;
                                        }
                                        else
                                        {
                                            _22062 = _13107;
                                        }
                                        _22080 = (((gBackdrop4.sample(gLinear, _22059, level(0.0)) * _13084.x) + (gBackdrop4.sample(gLinear, _22060, level(0.0)) * _13087.x)) * _13084.y) + (((gBackdrop4.sample(gLinear, _22061, level(0.0)) * _13084.x) + (gBackdrop4.sample(gLinear, _22062, level(0.0)) * _13087.x)) * _13087.y);
                                    }
                                    else
                                    {
                                        float2 _13245 = (_6509 * _172.gLevel[5].xy) - float2(0.5);
                                        float2 _13247 = floor(_13245);
                                        float2 _13250 = _13245 - _13247;
                                        float2 _13253 = _13250 * _13250;
                                        float2 _13256 = _13253 * _13250;
                                        float2 _13275 = (((_13256 * 3.0) - (_13253 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _13288 = _13256 * 0.16666667163372039794921875;
                                        float2 _13291 = (((((-_13256) + (_13253 * 3.0)) - (_13250 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13275;
                                        float2 _13294 = (((((_13256 * (-3.0)) + (_13253 * 3.0)) + (_13250 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13288;
                                        float2 _13304 = ((_13247 - float2(0.5)) + (_13275 / _13291)) * _172.gLevel[5].zw;
                                        float2 _13314 = ((_13247 + float2(1.5)) + (_13288 / _13294)) * _172.gLevel[5].zw;
                                        float2 _22055;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20936 = _13304;
                                            _20936.y = 1.0 - _13304.y;
                                            _22055 = _20936;
                                        }
                                        else
                                        {
                                            _22055 = _13304;
                                        }
                                        float _13334 = _13304.y;
                                        float2 _13335 = float2(_13314.x, _13334);
                                        float2 _22056;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20942 = _13335;
                                            _20942.y = 1.0 - _13334;
                                            _22056 = _20942;
                                        }
                                        else
                                        {
                                            _22056 = _13335;
                                        }
                                        float _13351 = _13314.y;
                                        float2 _13352 = float2(_13304.x, _13351);
                                        float2 _22057;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20949 = _13352;
                                            _20949.y = 1.0 - _13351;
                                            _22057 = _20949;
                                        }
                                        else
                                        {
                                            _22057 = _13352;
                                        }
                                        float2 _22058;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20955 = _13314;
                                            _20955.y = 1.0 - _13314.y;
                                            _22058 = _20955;
                                        }
                                        else
                                        {
                                            _22058 = _13314;
                                        }
                                        _22080 = (((gBackdrop5.sample(gLinear, _22055, level(0.0)) * _13291.x) + (gBackdrop5.sample(gLinear, _22056, level(0.0)) * _13294.x)) * _13291.y) + (((gBackdrop5.sample(gLinear, _22057, level(0.0)) * _13291.x) + (gBackdrop5.sample(gLinear, _22058, level(0.0)) * _13294.x)) * _13294.y);
                                    }
                                    _22079 = _22080;
                                }
                                _22078 = _22079;
                            }
                            _22077 = _22078;
                        }
                        _22076 = _22077;
                    }
                    _22081 = mix(_22050.xyz, _22076.xyz, float3(_11158));
                }
                else
                {
                    _22081 = _22050.xyz;
                }
                float2 _6520 = (_6458 + _6470) + (_6473 * _6495);
                float _13442 = fast::clamp(log2(fast::max(_6372, 1.0)) - 1.0, 0.0, 5.0);
                int _13445 = int(floor(_13442));
                float _13449 = _13442 - float(_13445);
                float4 _22156;
                if (_13445 <= 0)
                {
                    float2 _22155;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _20962 = _6520;
                        _20962.y = 1.0 - _6520.y;
                        _22155 = _20962;
                    }
                    else
                    {
                        _22155 = _6520;
                    }
                    _22156 = gBackdrop0.sample(gLinear, _22155, level(0.0));
                }
                else
                {
                    float4 _22157;
                    if (_13445 == 1)
                    {
                        float2 _13584 = (_6520 * _172.gLevel[1].xy) - float2(0.5);
                        float2 _13586 = floor(_13584);
                        float2 _13589 = _13584 - _13586;
                        float2 _13592 = _13589 * _13589;
                        float2 _13595 = _13592 * _13589;
                        float2 _13614 = (((_13595 * 3.0) - (_13592 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _13627 = _13595 * 0.16666667163372039794921875;
                        float2 _13630 = (((((-_13595) + (_13592 * 3.0)) - (_13589 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13614;
                        float2 _13633 = (((((_13595 * (-3.0)) + (_13592 * 3.0)) + (_13589 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13627;
                        float2 _13643 = ((_13586 - float2(0.5)) + (_13614 / _13630)) * _172.gLevel[1].zw;
                        float2 _13653 = ((_13586 + float2(1.5)) + (_13627 / _13633)) * _172.gLevel[1].zw;
                        float2 _22151;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20967 = _13643;
                            _20967.y = 1.0 - _13643.y;
                            _22151 = _20967;
                        }
                        else
                        {
                            _22151 = _13643;
                        }
                        float _13673 = _13643.y;
                        float2 _13674 = float2(_13653.x, _13673);
                        float2 _22152;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20973 = _13674;
                            _20973.y = 1.0 - _13673;
                            _22152 = _20973;
                        }
                        else
                        {
                            _22152 = _13674;
                        }
                        float _13690 = _13653.y;
                        float2 _13691 = float2(_13643.x, _13690);
                        float2 _22153;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20980 = _13691;
                            _20980.y = 1.0 - _13690;
                            _22153 = _20980;
                        }
                        else
                        {
                            _22153 = _13691;
                        }
                        float2 _22154;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20986 = _13653;
                            _20986.y = 1.0 - _13653.y;
                            _22154 = _20986;
                        }
                        else
                        {
                            _22154 = _13653;
                        }
                        _22157 = (((gBackdrop1.sample(gLinear, _22151, level(0.0)) * _13630.x) + (gBackdrop1.sample(gLinear, _22152, level(0.0)) * _13633.x)) * _13630.y) + (((gBackdrop1.sample(gLinear, _22153, level(0.0)) * _13630.x) + (gBackdrop1.sample(gLinear, _22154, level(0.0)) * _13633.x)) * _13633.y);
                    }
                    else
                    {
                        float4 _22158;
                        if (_13445 == 2)
                        {
                            float2 _13791 = (_6520 * _172.gLevel[2].xy) - float2(0.5);
                            float2 _13793 = floor(_13791);
                            float2 _13796 = _13791 - _13793;
                            float2 _13799 = _13796 * _13796;
                            float2 _13802 = _13799 * _13796;
                            float2 _13821 = (((_13802 * 3.0) - (_13799 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _13834 = _13802 * 0.16666667163372039794921875;
                            float2 _13837 = (((((-_13802) + (_13799 * 3.0)) - (_13796 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13821;
                            float2 _13840 = (((((_13802 * (-3.0)) + (_13799 * 3.0)) + (_13796 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13834;
                            float2 _13850 = ((_13793 - float2(0.5)) + (_13821 / _13837)) * _172.gLevel[2].zw;
                            float2 _13860 = ((_13793 + float2(1.5)) + (_13834 / _13840)) * _172.gLevel[2].zw;
                            float2 _22147;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20993 = _13850;
                                _20993.y = 1.0 - _13850.y;
                                _22147 = _20993;
                            }
                            else
                            {
                                _22147 = _13850;
                            }
                            float _13880 = _13850.y;
                            float2 _13881 = float2(_13860.x, _13880);
                            float2 _22148;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20999 = _13881;
                                _20999.y = 1.0 - _13880;
                                _22148 = _20999;
                            }
                            else
                            {
                                _22148 = _13881;
                            }
                            float _13897 = _13860.y;
                            float2 _13898 = float2(_13850.x, _13897);
                            float2 _22149;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21006 = _13898;
                                _21006.y = 1.0 - _13897;
                                _22149 = _21006;
                            }
                            else
                            {
                                _22149 = _13898;
                            }
                            float2 _22150;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21012 = _13860;
                                _21012.y = 1.0 - _13860.y;
                                _22150 = _21012;
                            }
                            else
                            {
                                _22150 = _13860;
                            }
                            _22158 = (((gBackdrop2.sample(gLinear, _22147, level(0.0)) * _13837.x) + (gBackdrop2.sample(gLinear, _22148, level(0.0)) * _13840.x)) * _13837.y) + (((gBackdrop2.sample(gLinear, _22149, level(0.0)) * _13837.x) + (gBackdrop2.sample(gLinear, _22150, level(0.0)) * _13840.x)) * _13840.y);
                        }
                        else
                        {
                            float4 _22159;
                            if (_13445 == 3)
                            {
                                float2 _13998 = (_6520 * _172.gLevel[3].xy) - float2(0.5);
                                float2 _14000 = floor(_13998);
                                float2 _14003 = _13998 - _14000;
                                float2 _14006 = _14003 * _14003;
                                float2 _14009 = _14006 * _14003;
                                float2 _14028 = (((_14009 * 3.0) - (_14006 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _14041 = _14009 * 0.16666667163372039794921875;
                                float2 _14044 = (((((-_14009) + (_14006 * 3.0)) - (_14003 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14028;
                                float2 _14047 = (((((_14009 * (-3.0)) + (_14006 * 3.0)) + (_14003 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14041;
                                float2 _14057 = ((_14000 - float2(0.5)) + (_14028 / _14044)) * _172.gLevel[3].zw;
                                float2 _14067 = ((_14000 + float2(1.5)) + (_14041 / _14047)) * _172.gLevel[3].zw;
                                float2 _22143;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21019 = _14057;
                                    _21019.y = 1.0 - _14057.y;
                                    _22143 = _21019;
                                }
                                else
                                {
                                    _22143 = _14057;
                                }
                                float _14087 = _14057.y;
                                float2 _14088 = float2(_14067.x, _14087);
                                float2 _22144;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21025 = _14088;
                                    _21025.y = 1.0 - _14087;
                                    _22144 = _21025;
                                }
                                else
                                {
                                    _22144 = _14088;
                                }
                                float _14104 = _14067.y;
                                float2 _14105 = float2(_14057.x, _14104);
                                float2 _22145;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21032 = _14105;
                                    _21032.y = 1.0 - _14104;
                                    _22145 = _21032;
                                }
                                else
                                {
                                    _22145 = _14105;
                                }
                                float2 _22146;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21038 = _14067;
                                    _21038.y = 1.0 - _14067.y;
                                    _22146 = _21038;
                                }
                                else
                                {
                                    _22146 = _14067;
                                }
                                _22159 = (((gBackdrop3.sample(gLinear, _22143, level(0.0)) * _14044.x) + (gBackdrop3.sample(gLinear, _22144, level(0.0)) * _14047.x)) * _14044.y) + (((gBackdrop3.sample(gLinear, _22145, level(0.0)) * _14044.x) + (gBackdrop3.sample(gLinear, _22146, level(0.0)) * _14047.x)) * _14047.y);
                            }
                            else
                            {
                                float4 _22160;
                                if (_13445 == 4)
                                {
                                    float2 _14205 = (_6520 * _172.gLevel[4].xy) - float2(0.5);
                                    float2 _14207 = floor(_14205);
                                    float2 _14210 = _14205 - _14207;
                                    float2 _14213 = _14210 * _14210;
                                    float2 _14216 = _14213 * _14210;
                                    float2 _14235 = (((_14216 * 3.0) - (_14213 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _14248 = _14216 * 0.16666667163372039794921875;
                                    float2 _14251 = (((((-_14216) + (_14213 * 3.0)) - (_14210 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14235;
                                    float2 _14254 = (((((_14216 * (-3.0)) + (_14213 * 3.0)) + (_14210 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14248;
                                    float2 _14264 = ((_14207 - float2(0.5)) + (_14235 / _14251)) * _172.gLevel[4].zw;
                                    float2 _14274 = ((_14207 + float2(1.5)) + (_14248 / _14254)) * _172.gLevel[4].zw;
                                    float2 _22139;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21045 = _14264;
                                        _21045.y = 1.0 - _14264.y;
                                        _22139 = _21045;
                                    }
                                    else
                                    {
                                        _22139 = _14264;
                                    }
                                    float _14294 = _14264.y;
                                    float2 _14295 = float2(_14274.x, _14294);
                                    float2 _22140;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21051 = _14295;
                                        _21051.y = 1.0 - _14294;
                                        _22140 = _21051;
                                    }
                                    else
                                    {
                                        _22140 = _14295;
                                    }
                                    float _14311 = _14274.y;
                                    float2 _14312 = float2(_14264.x, _14311);
                                    float2 _22141;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21058 = _14312;
                                        _21058.y = 1.0 - _14311;
                                        _22141 = _21058;
                                    }
                                    else
                                    {
                                        _22141 = _14312;
                                    }
                                    float2 _22142;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21064 = _14274;
                                        _21064.y = 1.0 - _14274.y;
                                        _22142 = _21064;
                                    }
                                    else
                                    {
                                        _22142 = _14274;
                                    }
                                    _22160 = (((gBackdrop4.sample(gLinear, _22139, level(0.0)) * _14251.x) + (gBackdrop4.sample(gLinear, _22140, level(0.0)) * _14254.x)) * _14251.y) + (((gBackdrop4.sample(gLinear, _22141, level(0.0)) * _14251.x) + (gBackdrop4.sample(gLinear, _22142, level(0.0)) * _14254.x)) * _14254.y);
                                }
                                else
                                {
                                    float2 _14412 = (_6520 * _172.gLevel[5].xy) - float2(0.5);
                                    float2 _14414 = floor(_14412);
                                    float2 _14417 = _14412 - _14414;
                                    float2 _14420 = _14417 * _14417;
                                    float2 _14423 = _14420 * _14417;
                                    float2 _14442 = (((_14423 * 3.0) - (_14420 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _14455 = _14423 * 0.16666667163372039794921875;
                                    float2 _14458 = (((((-_14423) + (_14420 * 3.0)) - (_14417 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14442;
                                    float2 _14461 = (((((_14423 * (-3.0)) + (_14420 * 3.0)) + (_14417 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14455;
                                    float2 _14471 = ((_14414 - float2(0.5)) + (_14442 / _14458)) * _172.gLevel[5].zw;
                                    float2 _14481 = ((_14414 + float2(1.5)) + (_14455 / _14461)) * _172.gLevel[5].zw;
                                    float2 _22135;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21071 = _14471;
                                        _21071.y = 1.0 - _14471.y;
                                        _22135 = _21071;
                                    }
                                    else
                                    {
                                        _22135 = _14471;
                                    }
                                    float _14501 = _14471.y;
                                    float2 _14502 = float2(_14481.x, _14501);
                                    float2 _22136;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21077 = _14502;
                                        _21077.y = 1.0 - _14501;
                                        _22136 = _21077;
                                    }
                                    else
                                    {
                                        _22136 = _14502;
                                    }
                                    float _14518 = _14481.y;
                                    float2 _14519 = float2(_14471.x, _14518);
                                    float2 _22137;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21084 = _14519;
                                        _21084.y = 1.0 - _14518;
                                        _22137 = _21084;
                                    }
                                    else
                                    {
                                        _22137 = _14519;
                                    }
                                    float2 _22138;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21090 = _14481;
                                        _21090.y = 1.0 - _14481.y;
                                        _22138 = _21090;
                                    }
                                    else
                                    {
                                        _22138 = _14481;
                                    }
                                    _22160 = (((gBackdrop5.sample(gLinear, _22135, level(0.0)) * _14458.x) + (gBackdrop5.sample(gLinear, _22136, level(0.0)) * _14461.x)) * _14458.y) + (((gBackdrop5.sample(gLinear, _22137, level(0.0)) * _14458.x) + (gBackdrop5.sample(gLinear, _22138, level(0.0)) * _14461.x)) * _14461.y);
                                }
                                _22159 = _22160;
                            }
                            _22158 = _22159;
                        }
                        _22157 = _22158;
                    }
                    _22156 = _22157;
                }
                float3 _22187;
                if ((_13449 > 0.0199999995529651641845703125) && (_13445 < 5))
                {
                    int _13462 = _13445 + 1;
                    float4 _22182;
                    if (_13462 <= 0)
                    {
                        float2 _22181;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21095 = _6520;
                            _21095.y = 1.0 - _6520.y;
                            _22181 = _21095;
                        }
                        else
                        {
                            _22181 = _6520;
                        }
                        _22182 = gBackdrop0.sample(gLinear, _22181, level(0.0));
                    }
                    else
                    {
                        float4 _22183;
                        if (_13462 == 1)
                        {
                            float2 _14708 = (_6520 * _172.gLevel[1].xy) - float2(0.5);
                            float2 _14710 = floor(_14708);
                            float2 _14713 = _14708 - _14710;
                            float2 _14716 = _14713 * _14713;
                            float2 _14719 = _14716 * _14713;
                            float2 _14738 = (((_14719 * 3.0) - (_14716 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _14751 = _14719 * 0.16666667163372039794921875;
                            float2 _14754 = (((((-_14719) + (_14716 * 3.0)) - (_14713 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14738;
                            float2 _14757 = (((((_14719 * (-3.0)) + (_14716 * 3.0)) + (_14713 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14751;
                            float2 _14767 = ((_14710 - float2(0.5)) + (_14738 / _14754)) * _172.gLevel[1].zw;
                            float2 _14777 = ((_14710 + float2(1.5)) + (_14751 / _14757)) * _172.gLevel[1].zw;
                            float2 _22177;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21100 = _14767;
                                _21100.y = 1.0 - _14767.y;
                                _22177 = _21100;
                            }
                            else
                            {
                                _22177 = _14767;
                            }
                            float _14797 = _14767.y;
                            float2 _14798 = float2(_14777.x, _14797);
                            float2 _22178;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21106 = _14798;
                                _21106.y = 1.0 - _14797;
                                _22178 = _21106;
                            }
                            else
                            {
                                _22178 = _14798;
                            }
                            float _14814 = _14777.y;
                            float2 _14815 = float2(_14767.x, _14814);
                            float2 _22179;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21113 = _14815;
                                _21113.y = 1.0 - _14814;
                                _22179 = _21113;
                            }
                            else
                            {
                                _22179 = _14815;
                            }
                            float2 _22180;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21119 = _14777;
                                _21119.y = 1.0 - _14777.y;
                                _22180 = _21119;
                            }
                            else
                            {
                                _22180 = _14777;
                            }
                            _22183 = (((gBackdrop1.sample(gLinear, _22177, level(0.0)) * _14754.x) + (gBackdrop1.sample(gLinear, _22178, level(0.0)) * _14757.x)) * _14754.y) + (((gBackdrop1.sample(gLinear, _22179, level(0.0)) * _14754.x) + (gBackdrop1.sample(gLinear, _22180, level(0.0)) * _14757.x)) * _14757.y);
                        }
                        else
                        {
                            float4 _22184;
                            if (_13462 == 2)
                            {
                                float2 _14915 = (_6520 * _172.gLevel[2].xy) - float2(0.5);
                                float2 _14917 = floor(_14915);
                                float2 _14920 = _14915 - _14917;
                                float2 _14923 = _14920 * _14920;
                                float2 _14926 = _14923 * _14920;
                                float2 _14945 = (((_14926 * 3.0) - (_14923 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _14958 = _14926 * 0.16666667163372039794921875;
                                float2 _14961 = (((((-_14926) + (_14923 * 3.0)) - (_14920 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14945;
                                float2 _14964 = (((((_14926 * (-3.0)) + (_14923 * 3.0)) + (_14920 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14958;
                                float2 _14974 = ((_14917 - float2(0.5)) + (_14945 / _14961)) * _172.gLevel[2].zw;
                                float2 _14984 = ((_14917 + float2(1.5)) + (_14958 / _14964)) * _172.gLevel[2].zw;
                                float2 _22173;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21126 = _14974;
                                    _21126.y = 1.0 - _14974.y;
                                    _22173 = _21126;
                                }
                                else
                                {
                                    _22173 = _14974;
                                }
                                float _15004 = _14974.y;
                                float2 _15005 = float2(_14984.x, _15004);
                                float2 _22174;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21132 = _15005;
                                    _21132.y = 1.0 - _15004;
                                    _22174 = _21132;
                                }
                                else
                                {
                                    _22174 = _15005;
                                }
                                float _15021 = _14984.y;
                                float2 _15022 = float2(_14974.x, _15021);
                                float2 _22175;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21139 = _15022;
                                    _21139.y = 1.0 - _15021;
                                    _22175 = _21139;
                                }
                                else
                                {
                                    _22175 = _15022;
                                }
                                float2 _22176;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21145 = _14984;
                                    _21145.y = 1.0 - _14984.y;
                                    _22176 = _21145;
                                }
                                else
                                {
                                    _22176 = _14984;
                                }
                                _22184 = (((gBackdrop2.sample(gLinear, _22173, level(0.0)) * _14961.x) + (gBackdrop2.sample(gLinear, _22174, level(0.0)) * _14964.x)) * _14961.y) + (((gBackdrop2.sample(gLinear, _22175, level(0.0)) * _14961.x) + (gBackdrop2.sample(gLinear, _22176, level(0.0)) * _14964.x)) * _14964.y);
                            }
                            else
                            {
                                float4 _22185;
                                if (_13462 == 3)
                                {
                                    float2 _15122 = (_6520 * _172.gLevel[3].xy) - float2(0.5);
                                    float2 _15124 = floor(_15122);
                                    float2 _15127 = _15122 - _15124;
                                    float2 _15130 = _15127 * _15127;
                                    float2 _15133 = _15130 * _15127;
                                    float2 _15152 = (((_15133 * 3.0) - (_15130 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _15165 = _15133 * 0.16666667163372039794921875;
                                    float2 _15168 = (((((-_15133) + (_15130 * 3.0)) - (_15127 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15152;
                                    float2 _15171 = (((((_15133 * (-3.0)) + (_15130 * 3.0)) + (_15127 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15165;
                                    float2 _15181 = ((_15124 - float2(0.5)) + (_15152 / _15168)) * _172.gLevel[3].zw;
                                    float2 _15191 = ((_15124 + float2(1.5)) + (_15165 / _15171)) * _172.gLevel[3].zw;
                                    float2 _22169;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21152 = _15181;
                                        _21152.y = 1.0 - _15181.y;
                                        _22169 = _21152;
                                    }
                                    else
                                    {
                                        _22169 = _15181;
                                    }
                                    float _15211 = _15181.y;
                                    float2 _15212 = float2(_15191.x, _15211);
                                    float2 _22170;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21158 = _15212;
                                        _21158.y = 1.0 - _15211;
                                        _22170 = _21158;
                                    }
                                    else
                                    {
                                        _22170 = _15212;
                                    }
                                    float _15228 = _15191.y;
                                    float2 _15229 = float2(_15181.x, _15228);
                                    float2 _22171;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21165 = _15229;
                                        _21165.y = 1.0 - _15228;
                                        _22171 = _21165;
                                    }
                                    else
                                    {
                                        _22171 = _15229;
                                    }
                                    float2 _22172;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21171 = _15191;
                                        _21171.y = 1.0 - _15191.y;
                                        _22172 = _21171;
                                    }
                                    else
                                    {
                                        _22172 = _15191;
                                    }
                                    _22185 = (((gBackdrop3.sample(gLinear, _22169, level(0.0)) * _15168.x) + (gBackdrop3.sample(gLinear, _22170, level(0.0)) * _15171.x)) * _15168.y) + (((gBackdrop3.sample(gLinear, _22171, level(0.0)) * _15168.x) + (gBackdrop3.sample(gLinear, _22172, level(0.0)) * _15171.x)) * _15171.y);
                                }
                                else
                                {
                                    float4 _22186;
                                    if (_13462 == 4)
                                    {
                                        float2 _15329 = (_6520 * _172.gLevel[4].xy) - float2(0.5);
                                        float2 _15331 = floor(_15329);
                                        float2 _15334 = _15329 - _15331;
                                        float2 _15337 = _15334 * _15334;
                                        float2 _15340 = _15337 * _15334;
                                        float2 _15359 = (((_15340 * 3.0) - (_15337 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _15372 = _15340 * 0.16666667163372039794921875;
                                        float2 _15375 = (((((-_15340) + (_15337 * 3.0)) - (_15334 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15359;
                                        float2 _15378 = (((((_15340 * (-3.0)) + (_15337 * 3.0)) + (_15334 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15372;
                                        float2 _15388 = ((_15331 - float2(0.5)) + (_15359 / _15375)) * _172.gLevel[4].zw;
                                        float2 _15398 = ((_15331 + float2(1.5)) + (_15372 / _15378)) * _172.gLevel[4].zw;
                                        float2 _22165;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21178 = _15388;
                                            _21178.y = 1.0 - _15388.y;
                                            _22165 = _21178;
                                        }
                                        else
                                        {
                                            _22165 = _15388;
                                        }
                                        float _15418 = _15388.y;
                                        float2 _15419 = float2(_15398.x, _15418);
                                        float2 _22166;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21184 = _15419;
                                            _21184.y = 1.0 - _15418;
                                            _22166 = _21184;
                                        }
                                        else
                                        {
                                            _22166 = _15419;
                                        }
                                        float _15435 = _15398.y;
                                        float2 _15436 = float2(_15388.x, _15435);
                                        float2 _22167;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21191 = _15436;
                                            _21191.y = 1.0 - _15435;
                                            _22167 = _21191;
                                        }
                                        else
                                        {
                                            _22167 = _15436;
                                        }
                                        float2 _22168;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21197 = _15398;
                                            _21197.y = 1.0 - _15398.y;
                                            _22168 = _21197;
                                        }
                                        else
                                        {
                                            _22168 = _15398;
                                        }
                                        _22186 = (((gBackdrop4.sample(gLinear, _22165, level(0.0)) * _15375.x) + (gBackdrop4.sample(gLinear, _22166, level(0.0)) * _15378.x)) * _15375.y) + (((gBackdrop4.sample(gLinear, _22167, level(0.0)) * _15375.x) + (gBackdrop4.sample(gLinear, _22168, level(0.0)) * _15378.x)) * _15378.y);
                                    }
                                    else
                                    {
                                        float2 _15536 = (_6520 * _172.gLevel[5].xy) - float2(0.5);
                                        float2 _15538 = floor(_15536);
                                        float2 _15541 = _15536 - _15538;
                                        float2 _15544 = _15541 * _15541;
                                        float2 _15547 = _15544 * _15541;
                                        float2 _15566 = (((_15547 * 3.0) - (_15544 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _15579 = _15547 * 0.16666667163372039794921875;
                                        float2 _15582 = (((((-_15547) + (_15544 * 3.0)) - (_15541 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15566;
                                        float2 _15585 = (((((_15547 * (-3.0)) + (_15544 * 3.0)) + (_15541 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15579;
                                        float2 _15595 = ((_15538 - float2(0.5)) + (_15566 / _15582)) * _172.gLevel[5].zw;
                                        float2 _15605 = ((_15538 + float2(1.5)) + (_15579 / _15585)) * _172.gLevel[5].zw;
                                        float2 _22161;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21204 = _15595;
                                            _21204.y = 1.0 - _15595.y;
                                            _22161 = _21204;
                                        }
                                        else
                                        {
                                            _22161 = _15595;
                                        }
                                        float _15625 = _15595.y;
                                        float2 _15626 = float2(_15605.x, _15625);
                                        float2 _22162;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21210 = _15626;
                                            _21210.y = 1.0 - _15625;
                                            _22162 = _21210;
                                        }
                                        else
                                        {
                                            _22162 = _15626;
                                        }
                                        float _15642 = _15605.y;
                                        float2 _15643 = float2(_15595.x, _15642);
                                        float2 _22163;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21217 = _15643;
                                            _21217.y = 1.0 - _15642;
                                            _22163 = _21217;
                                        }
                                        else
                                        {
                                            _22163 = _15643;
                                        }
                                        float2 _22164;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21223 = _15605;
                                            _21223.y = 1.0 - _15605.y;
                                            _22164 = _21223;
                                        }
                                        else
                                        {
                                            _22164 = _15605;
                                        }
                                        _22186 = (((gBackdrop5.sample(gLinear, _22161, level(0.0)) * _15582.x) + (gBackdrop5.sample(gLinear, _22162, level(0.0)) * _15585.x)) * _15582.y) + (((gBackdrop5.sample(gLinear, _22163, level(0.0)) * _15582.x) + (gBackdrop5.sample(gLinear, _22164, level(0.0)) * _15585.x)) * _15585.y);
                                    }
                                    _22185 = _22186;
                                }
                                _22184 = _22185;
                            }
                            _22183 = _22184;
                        }
                        _22182 = _22183;
                    }
                    _22187 = mix(_22156.xyz, _22182.xyz, float3(_13449));
                }
                else
                {
                    _22187 = _22156.xyz;
                }
                _22477 = float3(_21975.x, _22081.y, _22187.z);
            }
            else
            {
                float2 _6528 = _6458 + _6470;
                float _15733 = fast::clamp(log2(fast::max(_6372, 1.0)) - 1.0, 0.0, 5.0);
                int _15736 = int(floor(_15733));
                float _15740 = _15733 - float(_15736);
                float4 _21891;
                if (_15736 <= 0)
                {
                    float2 _21890;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _21230 = _6528;
                        _21230.y = 1.0 - _6528.y;
                        _21890 = _21230;
                    }
                    else
                    {
                        _21890 = _6528;
                    }
                    _21891 = gBackdrop0.sample(gLinear, _21890, level(0.0));
                }
                else
                {
                    float4 _21892;
                    if (_15736 == 1)
                    {
                        float2 _15875 = (_6528 * _172.gLevel[1].xy) - float2(0.5);
                        float2 _15877 = floor(_15875);
                        float2 _15880 = _15875 - _15877;
                        float2 _15883 = _15880 * _15880;
                        float2 _15886 = _15883 * _15880;
                        float2 _15905 = (((_15886 * 3.0) - (_15883 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _15918 = _15886 * 0.16666667163372039794921875;
                        float2 _15921 = (((((-_15886) + (_15883 * 3.0)) - (_15880 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15905;
                        float2 _15924 = (((((_15886 * (-3.0)) + (_15883 * 3.0)) + (_15880 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15918;
                        float2 _15934 = ((_15877 - float2(0.5)) + (_15905 / _15921)) * _172.gLevel[1].zw;
                        float2 _15944 = ((_15877 + float2(1.5)) + (_15918 / _15924)) * _172.gLevel[1].zw;
                        float2 _21886;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21235 = _15934;
                            _21235.y = 1.0 - _15934.y;
                            _21886 = _21235;
                        }
                        else
                        {
                            _21886 = _15934;
                        }
                        float _15964 = _15934.y;
                        float2 _15965 = float2(_15944.x, _15964);
                        float2 _21887;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21241 = _15965;
                            _21241.y = 1.0 - _15964;
                            _21887 = _21241;
                        }
                        else
                        {
                            _21887 = _15965;
                        }
                        float _15981 = _15944.y;
                        float2 _15982 = float2(_15934.x, _15981);
                        float2 _21888;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21248 = _15982;
                            _21248.y = 1.0 - _15981;
                            _21888 = _21248;
                        }
                        else
                        {
                            _21888 = _15982;
                        }
                        float2 _21889;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21254 = _15944;
                            _21254.y = 1.0 - _15944.y;
                            _21889 = _21254;
                        }
                        else
                        {
                            _21889 = _15944;
                        }
                        _21892 = (((gBackdrop1.sample(gLinear, _21886, level(0.0)) * _15921.x) + (gBackdrop1.sample(gLinear, _21887, level(0.0)) * _15924.x)) * _15921.y) + (((gBackdrop1.sample(gLinear, _21888, level(0.0)) * _15921.x) + (gBackdrop1.sample(gLinear, _21889, level(0.0)) * _15924.x)) * _15924.y);
                    }
                    else
                    {
                        float4 _21893;
                        if (_15736 == 2)
                        {
                            float2 _16082 = (_6528 * _172.gLevel[2].xy) - float2(0.5);
                            float2 _16084 = floor(_16082);
                            float2 _16087 = _16082 - _16084;
                            float2 _16090 = _16087 * _16087;
                            float2 _16093 = _16090 * _16087;
                            float2 _16112 = (((_16093 * 3.0) - (_16090 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _16125 = _16093 * 0.16666667163372039794921875;
                            float2 _16128 = (((((-_16093) + (_16090 * 3.0)) - (_16087 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16112;
                            float2 _16131 = (((((_16093 * (-3.0)) + (_16090 * 3.0)) + (_16087 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16125;
                            float2 _16141 = ((_16084 - float2(0.5)) + (_16112 / _16128)) * _172.gLevel[2].zw;
                            float2 _16151 = ((_16084 + float2(1.5)) + (_16125 / _16131)) * _172.gLevel[2].zw;
                            float2 _21882;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21261 = _16141;
                                _21261.y = 1.0 - _16141.y;
                                _21882 = _21261;
                            }
                            else
                            {
                                _21882 = _16141;
                            }
                            float _16171 = _16141.y;
                            float2 _16172 = float2(_16151.x, _16171);
                            float2 _21883;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21267 = _16172;
                                _21267.y = 1.0 - _16171;
                                _21883 = _21267;
                            }
                            else
                            {
                                _21883 = _16172;
                            }
                            float _16188 = _16151.y;
                            float2 _16189 = float2(_16141.x, _16188);
                            float2 _21884;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21274 = _16189;
                                _21274.y = 1.0 - _16188;
                                _21884 = _21274;
                            }
                            else
                            {
                                _21884 = _16189;
                            }
                            float2 _21885;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21280 = _16151;
                                _21280.y = 1.0 - _16151.y;
                                _21885 = _21280;
                            }
                            else
                            {
                                _21885 = _16151;
                            }
                            _21893 = (((gBackdrop2.sample(gLinear, _21882, level(0.0)) * _16128.x) + (gBackdrop2.sample(gLinear, _21883, level(0.0)) * _16131.x)) * _16128.y) + (((gBackdrop2.sample(gLinear, _21884, level(0.0)) * _16128.x) + (gBackdrop2.sample(gLinear, _21885, level(0.0)) * _16131.x)) * _16131.y);
                        }
                        else
                        {
                            float4 _21894;
                            if (_15736 == 3)
                            {
                                float2 _16289 = (_6528 * _172.gLevel[3].xy) - float2(0.5);
                                float2 _16291 = floor(_16289);
                                float2 _16294 = _16289 - _16291;
                                float2 _16297 = _16294 * _16294;
                                float2 _16300 = _16297 * _16294;
                                float2 _16319 = (((_16300 * 3.0) - (_16297 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _16332 = _16300 * 0.16666667163372039794921875;
                                float2 _16335 = (((((-_16300) + (_16297 * 3.0)) - (_16294 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16319;
                                float2 _16338 = (((((_16300 * (-3.0)) + (_16297 * 3.0)) + (_16294 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16332;
                                float2 _16348 = ((_16291 - float2(0.5)) + (_16319 / _16335)) * _172.gLevel[3].zw;
                                float2 _16358 = ((_16291 + float2(1.5)) + (_16332 / _16338)) * _172.gLevel[3].zw;
                                float2 _21878;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21287 = _16348;
                                    _21287.y = 1.0 - _16348.y;
                                    _21878 = _21287;
                                }
                                else
                                {
                                    _21878 = _16348;
                                }
                                float _16378 = _16348.y;
                                float2 _16379 = float2(_16358.x, _16378);
                                float2 _21879;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21293 = _16379;
                                    _21293.y = 1.0 - _16378;
                                    _21879 = _21293;
                                }
                                else
                                {
                                    _21879 = _16379;
                                }
                                float _16395 = _16358.y;
                                float2 _16396 = float2(_16348.x, _16395);
                                float2 _21880;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21300 = _16396;
                                    _21300.y = 1.0 - _16395;
                                    _21880 = _21300;
                                }
                                else
                                {
                                    _21880 = _16396;
                                }
                                float2 _21881;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21306 = _16358;
                                    _21306.y = 1.0 - _16358.y;
                                    _21881 = _21306;
                                }
                                else
                                {
                                    _21881 = _16358;
                                }
                                _21894 = (((gBackdrop3.sample(gLinear, _21878, level(0.0)) * _16335.x) + (gBackdrop3.sample(gLinear, _21879, level(0.0)) * _16338.x)) * _16335.y) + (((gBackdrop3.sample(gLinear, _21880, level(0.0)) * _16335.x) + (gBackdrop3.sample(gLinear, _21881, level(0.0)) * _16338.x)) * _16338.y);
                            }
                            else
                            {
                                float4 _21895;
                                if (_15736 == 4)
                                {
                                    float2 _16496 = (_6528 * _172.gLevel[4].xy) - float2(0.5);
                                    float2 _16498 = floor(_16496);
                                    float2 _16501 = _16496 - _16498;
                                    float2 _16504 = _16501 * _16501;
                                    float2 _16507 = _16504 * _16501;
                                    float2 _16526 = (((_16507 * 3.0) - (_16504 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _16539 = _16507 * 0.16666667163372039794921875;
                                    float2 _16542 = (((((-_16507) + (_16504 * 3.0)) - (_16501 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16526;
                                    float2 _16545 = (((((_16507 * (-3.0)) + (_16504 * 3.0)) + (_16501 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16539;
                                    float2 _16555 = ((_16498 - float2(0.5)) + (_16526 / _16542)) * _172.gLevel[4].zw;
                                    float2 _16565 = ((_16498 + float2(1.5)) + (_16539 / _16545)) * _172.gLevel[4].zw;
                                    float2 _21874;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21313 = _16555;
                                        _21313.y = 1.0 - _16555.y;
                                        _21874 = _21313;
                                    }
                                    else
                                    {
                                        _21874 = _16555;
                                    }
                                    float _16585 = _16555.y;
                                    float2 _16586 = float2(_16565.x, _16585);
                                    float2 _21875;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21319 = _16586;
                                        _21319.y = 1.0 - _16585;
                                        _21875 = _21319;
                                    }
                                    else
                                    {
                                        _21875 = _16586;
                                    }
                                    float _16602 = _16565.y;
                                    float2 _16603 = float2(_16555.x, _16602);
                                    float2 _21876;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21326 = _16603;
                                        _21326.y = 1.0 - _16602;
                                        _21876 = _21326;
                                    }
                                    else
                                    {
                                        _21876 = _16603;
                                    }
                                    float2 _21877;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21332 = _16565;
                                        _21332.y = 1.0 - _16565.y;
                                        _21877 = _21332;
                                    }
                                    else
                                    {
                                        _21877 = _16565;
                                    }
                                    _21895 = (((gBackdrop4.sample(gLinear, _21874, level(0.0)) * _16542.x) + (gBackdrop4.sample(gLinear, _21875, level(0.0)) * _16545.x)) * _16542.y) + (((gBackdrop4.sample(gLinear, _21876, level(0.0)) * _16542.x) + (gBackdrop4.sample(gLinear, _21877, level(0.0)) * _16545.x)) * _16545.y);
                                }
                                else
                                {
                                    float2 _16703 = (_6528 * _172.gLevel[5].xy) - float2(0.5);
                                    float2 _16705 = floor(_16703);
                                    float2 _16708 = _16703 - _16705;
                                    float2 _16711 = _16708 * _16708;
                                    float2 _16714 = _16711 * _16708;
                                    float2 _16733 = (((_16714 * 3.0) - (_16711 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _16746 = _16714 * 0.16666667163372039794921875;
                                    float2 _16749 = (((((-_16714) + (_16711 * 3.0)) - (_16708 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16733;
                                    float2 _16752 = (((((_16714 * (-3.0)) + (_16711 * 3.0)) + (_16708 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16746;
                                    float2 _16762 = ((_16705 - float2(0.5)) + (_16733 / _16749)) * _172.gLevel[5].zw;
                                    float2 _16772 = ((_16705 + float2(1.5)) + (_16746 / _16752)) * _172.gLevel[5].zw;
                                    float2 _21870;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21339 = _16762;
                                        _21339.y = 1.0 - _16762.y;
                                        _21870 = _21339;
                                    }
                                    else
                                    {
                                        _21870 = _16762;
                                    }
                                    float _16792 = _16762.y;
                                    float2 _16793 = float2(_16772.x, _16792);
                                    float2 _21871;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21345 = _16793;
                                        _21345.y = 1.0 - _16792;
                                        _21871 = _21345;
                                    }
                                    else
                                    {
                                        _21871 = _16793;
                                    }
                                    float _16809 = _16772.y;
                                    float2 _16810 = float2(_16762.x, _16809);
                                    float2 _21872;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21352 = _16810;
                                        _21352.y = 1.0 - _16809;
                                        _21872 = _21352;
                                    }
                                    else
                                    {
                                        _21872 = _16810;
                                    }
                                    float2 _21873;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21358 = _16772;
                                        _21358.y = 1.0 - _16772.y;
                                        _21873 = _21358;
                                    }
                                    else
                                    {
                                        _21873 = _16772;
                                    }
                                    _21895 = (((gBackdrop5.sample(gLinear, _21870, level(0.0)) * _16749.x) + (gBackdrop5.sample(gLinear, _21871, level(0.0)) * _16752.x)) * _16749.y) + (((gBackdrop5.sample(gLinear, _21872, level(0.0)) * _16749.x) + (gBackdrop5.sample(gLinear, _21873, level(0.0)) * _16752.x)) * _16752.y);
                                }
                                _21894 = _21895;
                            }
                            _21893 = _21894;
                        }
                        _21892 = _21893;
                    }
                    _21891 = _21892;
                }
                float3 _21922;
                if ((_15740 > 0.0199999995529651641845703125) && (_15736 < 5))
                {
                    int _15753 = _15736 + 1;
                    float4 _21917;
                    if (_15753 <= 0)
                    {
                        float2 _21916;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21363 = _6528;
                            _21363.y = 1.0 - _6528.y;
                            _21916 = _21363;
                        }
                        else
                        {
                            _21916 = _6528;
                        }
                        _21917 = gBackdrop0.sample(gLinear, _21916, level(0.0));
                    }
                    else
                    {
                        float4 _21918;
                        if (_15753 == 1)
                        {
                            float2 _16999 = (_6528 * _172.gLevel[1].xy) - float2(0.5);
                            float2 _17001 = floor(_16999);
                            float2 _17004 = _16999 - _17001;
                            float2 _17007 = _17004 * _17004;
                            float2 _17010 = _17007 * _17004;
                            float2 _17029 = (((_17010 * 3.0) - (_17007 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _17042 = _17010 * 0.16666667163372039794921875;
                            float2 _17045 = (((((-_17010) + (_17007 * 3.0)) - (_17004 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17029;
                            float2 _17048 = (((((_17010 * (-3.0)) + (_17007 * 3.0)) + (_17004 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17042;
                            float2 _17058 = ((_17001 - float2(0.5)) + (_17029 / _17045)) * _172.gLevel[1].zw;
                            float2 _17068 = ((_17001 + float2(1.5)) + (_17042 / _17048)) * _172.gLevel[1].zw;
                            float2 _21912;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21368 = _17058;
                                _21368.y = 1.0 - _17058.y;
                                _21912 = _21368;
                            }
                            else
                            {
                                _21912 = _17058;
                            }
                            float _17088 = _17058.y;
                            float2 _17089 = float2(_17068.x, _17088);
                            float2 _21913;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21374 = _17089;
                                _21374.y = 1.0 - _17088;
                                _21913 = _21374;
                            }
                            else
                            {
                                _21913 = _17089;
                            }
                            float _17105 = _17068.y;
                            float2 _17106 = float2(_17058.x, _17105);
                            float2 _21914;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21381 = _17106;
                                _21381.y = 1.0 - _17105;
                                _21914 = _21381;
                            }
                            else
                            {
                                _21914 = _17106;
                            }
                            float2 _21915;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21387 = _17068;
                                _21387.y = 1.0 - _17068.y;
                                _21915 = _21387;
                            }
                            else
                            {
                                _21915 = _17068;
                            }
                            _21918 = (((gBackdrop1.sample(gLinear, _21912, level(0.0)) * _17045.x) + (gBackdrop1.sample(gLinear, _21913, level(0.0)) * _17048.x)) * _17045.y) + (((gBackdrop1.sample(gLinear, _21914, level(0.0)) * _17045.x) + (gBackdrop1.sample(gLinear, _21915, level(0.0)) * _17048.x)) * _17048.y);
                        }
                        else
                        {
                            float4 _21919;
                            if (_15753 == 2)
                            {
                                float2 _17206 = (_6528 * _172.gLevel[2].xy) - float2(0.5);
                                float2 _17208 = floor(_17206);
                                float2 _17211 = _17206 - _17208;
                                float2 _17214 = _17211 * _17211;
                                float2 _17217 = _17214 * _17211;
                                float2 _17236 = (((_17217 * 3.0) - (_17214 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _17249 = _17217 * 0.16666667163372039794921875;
                                float2 _17252 = (((((-_17217) + (_17214 * 3.0)) - (_17211 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17236;
                                float2 _17255 = (((((_17217 * (-3.0)) + (_17214 * 3.0)) + (_17211 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17249;
                                float2 _17265 = ((_17208 - float2(0.5)) + (_17236 / _17252)) * _172.gLevel[2].zw;
                                float2 _17275 = ((_17208 + float2(1.5)) + (_17249 / _17255)) * _172.gLevel[2].zw;
                                float2 _21908;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21394 = _17265;
                                    _21394.y = 1.0 - _17265.y;
                                    _21908 = _21394;
                                }
                                else
                                {
                                    _21908 = _17265;
                                }
                                float _17295 = _17265.y;
                                float2 _17296 = float2(_17275.x, _17295);
                                float2 _21909;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21400 = _17296;
                                    _21400.y = 1.0 - _17295;
                                    _21909 = _21400;
                                }
                                else
                                {
                                    _21909 = _17296;
                                }
                                float _17312 = _17275.y;
                                float2 _17313 = float2(_17265.x, _17312);
                                float2 _21910;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21407 = _17313;
                                    _21407.y = 1.0 - _17312;
                                    _21910 = _21407;
                                }
                                else
                                {
                                    _21910 = _17313;
                                }
                                float2 _21911;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21413 = _17275;
                                    _21413.y = 1.0 - _17275.y;
                                    _21911 = _21413;
                                }
                                else
                                {
                                    _21911 = _17275;
                                }
                                _21919 = (((gBackdrop2.sample(gLinear, _21908, level(0.0)) * _17252.x) + (gBackdrop2.sample(gLinear, _21909, level(0.0)) * _17255.x)) * _17252.y) + (((gBackdrop2.sample(gLinear, _21910, level(0.0)) * _17252.x) + (gBackdrop2.sample(gLinear, _21911, level(0.0)) * _17255.x)) * _17255.y);
                            }
                            else
                            {
                                float4 _21920;
                                if (_15753 == 3)
                                {
                                    float2 _17413 = (_6528 * _172.gLevel[3].xy) - float2(0.5);
                                    float2 _17415 = floor(_17413);
                                    float2 _17418 = _17413 - _17415;
                                    float2 _17421 = _17418 * _17418;
                                    float2 _17424 = _17421 * _17418;
                                    float2 _17443 = (((_17424 * 3.0) - (_17421 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _17456 = _17424 * 0.16666667163372039794921875;
                                    float2 _17459 = (((((-_17424) + (_17421 * 3.0)) - (_17418 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17443;
                                    float2 _17462 = (((((_17424 * (-3.0)) + (_17421 * 3.0)) + (_17418 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17456;
                                    float2 _17472 = ((_17415 - float2(0.5)) + (_17443 / _17459)) * _172.gLevel[3].zw;
                                    float2 _17482 = ((_17415 + float2(1.5)) + (_17456 / _17462)) * _172.gLevel[3].zw;
                                    float2 _21904;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21420 = _17472;
                                        _21420.y = 1.0 - _17472.y;
                                        _21904 = _21420;
                                    }
                                    else
                                    {
                                        _21904 = _17472;
                                    }
                                    float _17502 = _17472.y;
                                    float2 _17503 = float2(_17482.x, _17502);
                                    float2 _21905;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21426 = _17503;
                                        _21426.y = 1.0 - _17502;
                                        _21905 = _21426;
                                    }
                                    else
                                    {
                                        _21905 = _17503;
                                    }
                                    float _17519 = _17482.y;
                                    float2 _17520 = float2(_17472.x, _17519);
                                    float2 _21906;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21433 = _17520;
                                        _21433.y = 1.0 - _17519;
                                        _21906 = _21433;
                                    }
                                    else
                                    {
                                        _21906 = _17520;
                                    }
                                    float2 _21907;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21439 = _17482;
                                        _21439.y = 1.0 - _17482.y;
                                        _21907 = _21439;
                                    }
                                    else
                                    {
                                        _21907 = _17482;
                                    }
                                    _21920 = (((gBackdrop3.sample(gLinear, _21904, level(0.0)) * _17459.x) + (gBackdrop3.sample(gLinear, _21905, level(0.0)) * _17462.x)) * _17459.y) + (((gBackdrop3.sample(gLinear, _21906, level(0.0)) * _17459.x) + (gBackdrop3.sample(gLinear, _21907, level(0.0)) * _17462.x)) * _17462.y);
                                }
                                else
                                {
                                    float4 _21921;
                                    if (_15753 == 4)
                                    {
                                        float2 _17620 = (_6528 * _172.gLevel[4].xy) - float2(0.5);
                                        float2 _17622 = floor(_17620);
                                        float2 _17625 = _17620 - _17622;
                                        float2 _17628 = _17625 * _17625;
                                        float2 _17631 = _17628 * _17625;
                                        float2 _17650 = (((_17631 * 3.0) - (_17628 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _17663 = _17631 * 0.16666667163372039794921875;
                                        float2 _17666 = (((((-_17631) + (_17628 * 3.0)) - (_17625 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17650;
                                        float2 _17669 = (((((_17631 * (-3.0)) + (_17628 * 3.0)) + (_17625 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17663;
                                        float2 _17679 = ((_17622 - float2(0.5)) + (_17650 / _17666)) * _172.gLevel[4].zw;
                                        float2 _17689 = ((_17622 + float2(1.5)) + (_17663 / _17669)) * _172.gLevel[4].zw;
                                        float2 _21900;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21446 = _17679;
                                            _21446.y = 1.0 - _17679.y;
                                            _21900 = _21446;
                                        }
                                        else
                                        {
                                            _21900 = _17679;
                                        }
                                        float _17709 = _17679.y;
                                        float2 _17710 = float2(_17689.x, _17709);
                                        float2 _21901;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21452 = _17710;
                                            _21452.y = 1.0 - _17709;
                                            _21901 = _21452;
                                        }
                                        else
                                        {
                                            _21901 = _17710;
                                        }
                                        float _17726 = _17689.y;
                                        float2 _17727 = float2(_17679.x, _17726);
                                        float2 _21902;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21459 = _17727;
                                            _21459.y = 1.0 - _17726;
                                            _21902 = _21459;
                                        }
                                        else
                                        {
                                            _21902 = _17727;
                                        }
                                        float2 _21903;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21465 = _17689;
                                            _21465.y = 1.0 - _17689.y;
                                            _21903 = _21465;
                                        }
                                        else
                                        {
                                            _21903 = _17689;
                                        }
                                        _21921 = (((gBackdrop4.sample(gLinear, _21900, level(0.0)) * _17666.x) + (gBackdrop4.sample(gLinear, _21901, level(0.0)) * _17669.x)) * _17666.y) + (((gBackdrop4.sample(gLinear, _21902, level(0.0)) * _17666.x) + (gBackdrop4.sample(gLinear, _21903, level(0.0)) * _17669.x)) * _17669.y);
                                    }
                                    else
                                    {
                                        float2 _17827 = (_6528 * _172.gLevel[5].xy) - float2(0.5);
                                        float2 _17829 = floor(_17827);
                                        float2 _17832 = _17827 - _17829;
                                        float2 _17835 = _17832 * _17832;
                                        float2 _17838 = _17835 * _17832;
                                        float2 _17857 = (((_17838 * 3.0) - (_17835 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _17870 = _17838 * 0.16666667163372039794921875;
                                        float2 _17873 = (((((-_17838) + (_17835 * 3.0)) - (_17832 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17857;
                                        float2 _17876 = (((((_17838 * (-3.0)) + (_17835 * 3.0)) + (_17832 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17870;
                                        float2 _17886 = ((_17829 - float2(0.5)) + (_17857 / _17873)) * _172.gLevel[5].zw;
                                        float2 _17896 = ((_17829 + float2(1.5)) + (_17870 / _17876)) * _172.gLevel[5].zw;
                                        float2 _21896;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21472 = _17886;
                                            _21472.y = 1.0 - _17886.y;
                                            _21896 = _21472;
                                        }
                                        else
                                        {
                                            _21896 = _17886;
                                        }
                                        float _17916 = _17886.y;
                                        float2 _17917 = float2(_17896.x, _17916);
                                        float2 _21897;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21478 = _17917;
                                            _21478.y = 1.0 - _17916;
                                            _21897 = _21478;
                                        }
                                        else
                                        {
                                            _21897 = _17917;
                                        }
                                        float _17933 = _17896.y;
                                        float2 _17934 = float2(_17886.x, _17933);
                                        float2 _21898;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21485 = _17934;
                                            _21485.y = 1.0 - _17933;
                                            _21898 = _21485;
                                        }
                                        else
                                        {
                                            _21898 = _17934;
                                        }
                                        float2 _21899;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21491 = _17896;
                                            _21491.y = 1.0 - _17896.y;
                                            _21899 = _21491;
                                        }
                                        else
                                        {
                                            _21899 = _17896;
                                        }
                                        _21921 = (((gBackdrop5.sample(gLinear, _21896, level(0.0)) * _17873.x) + (gBackdrop5.sample(gLinear, _21897, level(0.0)) * _17876.x)) * _17873.y) + (((gBackdrop5.sample(gLinear, _21898, level(0.0)) * _17873.x) + (gBackdrop5.sample(gLinear, _21899, level(0.0)) * _17876.x)) * _17876.y);
                                    }
                                    _21920 = _21921;
                                }
                                _21919 = _21920;
                            }
                            _21918 = _21919;
                        }
                        _21917 = _21918;
                    }
                    _21922 = mix(_21891.xyz, _21917.xyz, float3(_15740));
                }
                else
                {
                    _21922 = _21891.xyz;
                }
                _22477 = _21922;
            }
            float3 _22968;
            if (_6400 > 0.0)
            {
                float2 _6549 = _6458 + (((_21868 * fast::min(_6391 * 0.5, 16.0)) * _172.gDisplay.zw) * _172.gTarget.zw);
                float _18024 = fast::clamp(log2(fast::max(16.0 * _172.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _18027 = int(floor(_18024));
                float _18031 = _18024 - float(_18027);
                float2 _22455;
                if (_172.gConv.x > 0.5)
                {
                    float2 _21496 = _6549;
                    _21496.y = 1.0 - _6549.y;
                    _22455 = _21496;
                }
                else
                {
                    _22455 = _6549;
                }
                float4 _22456;
                if (_18027 <= 0)
                {
                    _22456 = gBackdrop0.sample(gLinear, _22455, level(0.0));
                }
                else
                {
                    float4 _22457;
                    if (_18027 == 1)
                    {
                        _22457 = gBackdrop1.sample(gLinear, _22455, level(0.0));
                    }
                    else
                    {
                        float4 _22458;
                        if (_18027 == 2)
                        {
                            _22458 = gBackdrop2.sample(gLinear, _22455, level(0.0));
                        }
                        else
                        {
                            float4 _22459;
                            if (_18027 == 3)
                            {
                                _22459 = gBackdrop3.sample(gLinear, _22455, level(0.0));
                            }
                            else
                            {
                                float4 _22460;
                                if (_18027 == 4)
                                {
                                    _22460 = gBackdrop4.sample(gLinear, _22455, level(0.0));
                                }
                                else
                                {
                                    _22460 = gBackdrop5.sample(gLinear, _22455, level(0.0));
                                }
                                _22459 = _22460;
                            }
                            _22458 = _22459;
                        }
                        _22457 = _22458;
                    }
                    _22456 = _22457;
                }
                float3 _22467;
                if ((_18031 > 0.0199999995529651641845703125) && (_18027 < 5))
                {
                    int _18044 = _18027 + 1;
                    float2 _22461;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _21499 = _6549;
                        _21499.y = 1.0 - _6549.y;
                        _22461 = _21499;
                    }
                    else
                    {
                        _22461 = _6549;
                    }
                    float4 _22462;
                    if (_18044 <= 0)
                    {
                        _22462 = gBackdrop0.sample(gLinear, _22461, level(0.0));
                    }
                    else
                    {
                        float4 _22463;
                        if (_18044 == 1)
                        {
                            _22463 = gBackdrop1.sample(gLinear, _22461, level(0.0));
                        }
                        else
                        {
                            float4 _22464;
                            if (_18044 == 2)
                            {
                                _22464 = gBackdrop2.sample(gLinear, _22461, level(0.0));
                            }
                            else
                            {
                                float4 _22465;
                                if (_18044 == 3)
                                {
                                    _22465 = gBackdrop3.sample(gLinear, _22461, level(0.0));
                                }
                                else
                                {
                                    float4 _22466;
                                    if (_18044 == 4)
                                    {
                                        _22466 = gBackdrop4.sample(gLinear, _22461, level(0.0));
                                    }
                                    else
                                    {
                                        _22466 = gBackdrop5.sample(gLinear, _22461, level(0.0));
                                    }
                                    _22465 = _22466;
                                }
                                _22464 = _22465;
                            }
                            _22463 = _22464;
                        }
                        _22462 = _22463;
                    }
                    _22467 = mix(_22456.xyz, _22462.xyz, float3(_18031));
                }
                else
                {
                    _22467 = _22456.xyz;
                }
                _22968 = _22467;
            }
            else
            {
                _22968 = float3(0.5);
            }
            float _22498;
            if (gFxData_1._data[_4975].x > 0.001000000047497451305389404296875)
            {
                int _18211 = clamp(int(rint(log2(36.0 * _172.gDisplay.z) - 1.0)), 1, 4);
                float2 _22468;
                if (_172.gConv.x > 0.5)
                {
                    float2 _21503 = _6458;
                    _21503.y = 1.0 - _6458.y;
                    _22468 = _21503;
                }
                else
                {
                    _22468 = _6458;
                }
                float4 _22469;
                if (_18211 <= 0)
                {
                    _22469 = gBackdrop0.sample(gLinear, _22468, level(0.0));
                }
                else
                {
                    float4 _22470;
                    if (_18211 == 1)
                    {
                        _22470 = gBackdrop1.sample(gLinear, _22468, level(0.0));
                    }
                    else
                    {
                        float4 _22471;
                        if (_18211 == 2)
                        {
                            _22471 = gBackdrop2.sample(gLinear, _22468, level(0.0));
                        }
                        else
                        {
                            float4 _22472;
                            if (_18211 == 3)
                            {
                                _22472 = gBackdrop3.sample(gLinear, _22468, level(0.0));
                            }
                            else
                            {
                                float4 _22473;
                                if (_18211 == 4)
                                {
                                    _22473 = gBackdrop4.sample(gLinear, _22468, level(0.0));
                                }
                                else
                                {
                                    _22473 = gBackdrop5.sample(gLinear, _22468, level(0.0));
                                }
                                _22472 = _22473;
                            }
                            _22471 = _22472;
                        }
                        _22470 = _22471;
                    }
                    _22469 = _22470;
                }
                _22498 = dot(_22469.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _22498 = 0.5;
            }
            _22966 = _22968;
            _22497 = _22498;
            _22474 = _22477;
        }
        else
        {
            _22966 = float3(0.5);
            _22497 = 0.5;
            _22474 = float3(0.5);
        }
        float3 _6577 = mix(float3(dot(_22474, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _22474, float3(gFxData_1._data[_4967].x)) + float3(gFxData_1._data[_4967].y);
        float3 _22726;
        if (gFxData_1._data[_4975].x > 0.001000000047497451305389404296875)
        {
            float _6587 = fast::clamp(fast::max(_22497 + gFxData_1._data[_4967].y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _6595 = mix(_6587, dot(gFxData_1._data[_4959].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), gFxData_1._data[_4975].x);
            _22726 = select(mix(_6577, float3(1.0), float3((_6595 - _6587) / fast::max(1.0 - _6587, 0.001000000047497451305389404296875))), _6577 * (_6595 / _6587), bool3(_6595 < _6587));
        }
        else
        {
            _22726 = _6577;
        }
        float2 _6633 = fast::clamp((in.i_local - gFxData_1._data[_4861].xy) / fast::max(gFxData_1._data[_4861].zw - gFxData_1._data[_4861].xy, float2(0.001000000047497451305389404296875)), float2(0.0), float2(1.0));
        float _6665 = pow(1.0 - _22730.z, 5.0);
        float2 _6681 = float2(cos(gFxData_1._data[_4967].w), sin(gFxData_1._data[_4967].w));
        float _6684 = dot(_21868, _6681);
        float _6719 = fast::clamp(0.5 + (0.5 * dot((in.i_local - ((gFxData_1._data[_4861].xy + gFxData_1._data[_4861].zw) * 0.5)) / _6383, _6681)), 0.0, 1.0);
        _23548 = ((_6665 * (pow(fast::clamp(_6684, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(fast::clamp(-_6684, 0.0, 1.0), 1.5)))) * gFxData_1._data[_4967].z) * 1.60000002384185791015625;
        _23502 = float4((mix(mix(_22726, gFxData_1._data[_4959].xyz, float3(fast::clamp(gFxData_1._data[_4959].w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _6406))) + (0.3499999940395355224609375 * ((dot(gFxData_1._data[_4959].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _6633.y) : _6633.y))), 0.0, 1.0))), (_22966 * 1.10000002384185791015625) + float3(0.07999999821186065673828125), float3(_6665 * 0.3499999940395355224609375)) + float3((((0.039999999105930328369140625 * _6719) * _6719) + (0.0500000007450580596923828125 * (1.0 - _6406))) * gFxData_1._data[_4967].z)) * _3708, _3708) + (_23215 * (1.0 - _3708));
    }
    else
    {
        _23548 = 0.0;
        _23502 = _23215;
    }
    float4 _23512;
    if ((_3685 & 1u) != 0u)
    {
        float4 _23498;
        do
        {
            uint _18335 = _4855.y;
            if (_18335 == 0u)
            {
                _23498 = gFxData_1._data[_4879];
                break;
            }
            float2 _18349 = fast::max(gFxData_1._data[_4861].zw - gFxData_1._data[_4861].xy, float2(0.001000000047497451305389404296875));
            float2 _18359 = fwidth(in.i_local);
            float _18361 = fast::max(length(_18359), 9.9999997473787516355514526367188e-05);
            float _23490;
            float _23494;
            if ((_18335 == 1u) || (_18335 == 4u))
            {
                float _18370 = cos(gFxData_1._data[_4895].x);
                float _18373 = sin(gFxData_1._data[_4895].x);
                float _18399 = ((dot(in.i_local - ((gFxData_1._data[_4861].xy + gFxData_1._data[_4861].zw) * 0.5), float2(_18370, _18373)) / fast::max(0.5 * ((abs(_18370) * _18349.x) + (abs(_18373) * _18349.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (_18335 == 4u)
                {
                    float _18414 = fast::clamp((_18399 - gFxData_1._data[_4895].y) / fast::max(gFxData_1._data[_4895].z - gFxData_1._data[_4895].y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _23498 = float4(fast::clamp(abs((fract(float3(_18414 * 0.800000011920928955078125) + float3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - float3(3.0)) - float3(1.0), float3(0.0), float3(1.0)), gFxData_1._data[_4879].w * pow(fast::max(sin(_18414 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _23494 = -1.0;
                _23490 = _18399;
            }
            else
            {
                float _23491;
                float _23495;
                if (_18335 == 2u)
                {
                    _23495 = -1.0;
                    _23491 = length(in.i_local - (gFxData_1._data[_4861].xy + (gFxData_1._data[_4895].xy * _18349))) / fast::max(gFxData_1._data[_4895].z * fast::max(_18349.x, _18349.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    float2 _18482 = in.i_local - (gFxData_1._data[_4861].xy + (gFxData_1._data[_4895].xy * _18349));
                    float _18493 = fract(((precise::atan2(_18482.y, _18482.x) - gFxData_1._data[_4895].z) * 0.15915493667125701904296875) + 1.0);
                    float _23492;
                    float _23496;
                    if (gFxData_1._data[_4895].w > 0.5)
                    {
                        _23496 = -1.0;
                        _23492 = 0.5 - (0.5 * cos(_18493 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _18513 = (((_18493 < 0.5) ? _18493 : (_18493 - 1.0)) * 6.283185482025146484375) * length(_18482);
                        float _23497;
                        if (abs(_18513) < _18361)
                        {
                            _23497 = fast::clamp(((_18513 / _18361) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _23497 = -1.0;
                        }
                        _23496 = _23497;
                        _23492 = _18493;
                    }
                    _23495 = _23496;
                    _23491 = _23492;
                }
                _23494 = _23495;
                _23490 = _23491;
            }
            float4 _18542 = float4(gFxData_1._data[_4879].xyz * gFxData_1._data[_4879].w, gFxData_1._data[_4879].w);
            float4 _18554 = float4(gFxData_1._data[_4887].xyz * gFxData_1._data[_4887].w, gFxData_1._data[_4887].w);
            float4 _18568 = select(mix(_18542, _18554, float4(fast::clamp(_23490, 0.0, 1.0))), mix(_18554, _18542, float4(_23494)), bool4(_23494 >= 0.0));
            _23498 = select(float4(0.0), float4(_18568.xyz / float3(_18568.w), _18568.w), bool4(_18568.w > 9.9999997473787516355514526367188e-06));
            break;
        } while(false);
        float4 _23499;
        if ((_3685 & 64u) != 0u)
        {
            _23499 = _23498 * gTex.sample(gLinear, mix(gFxData_1._data[_5007].xy, gFxData_1._data[_5007].zw, (in.i_local - gFxData_1._data[_4861].xy) / fast::max(gFxData_1._data[_4861].zw - gFxData_1._data[_4861].xy, float2(0.001000000047497451305389404296875))));
        }
        else
        {
            _23499 = _23498;
        }
        float _18595 = fast::clamp(_23499.w * _3708, 0.0, 1.0);
        _23512 = float4(_23499.xyz * _18595, _18595) + (_23502 * (1.0 - _18595));
    }
    else
    {
        _23512 = _23502;
    }
    float4 _23534;
    if ((_3685 & 16384u) != 0u)
    {
        float2 _4078 = fast::max(gFxData_1._data[_4861].zw - gFxData_1._data[_4861].xy, float2(0.001000000047497451305389404296875));
        float2 _4085 = (in.i_local - gFxData_1._data[_4861].xy) / _4078;
        float _4096 = ((gFxData_1._data[_4887].x >= 0.0) ? gFxData_1._data[_4887].x : _172.gTime.x) * gFxData_1._data[_5031].z;
        float _4099 = _4085.x * 2.0;
        float _4100 = _4099 - 1.0;
        float _4105 = _4085.y * _4078.y;
        float _4108 = fast::max(gFxData_1._data[_5031].w, 0.001000000047497451305389404296875);
        float _4113 = fast::clamp(1.0 - (_4100 * _4100), 0.0, 1.0);
        float _4115 = pow(_4113, 1.2999999523162841796875);
        float _4117 = pow(_4113, 0.699999988079071044921875);
        float _4120 = fast::clamp(_4096 * 1.4285714626312255859375, 0.0, 1.0);
        float _4136 = (0.25 + (0.75 * ((_4120 * _4120) * (3.0 - (2.0 * _4120))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_4096 * 2.099999904632568359375)));
        float _4145 = ((gFxData_1._data[_5031].y * _4078.y) * _4136) * _4115;
        float _4171 = (_4078.y * (0.5 + ((gFxData_1._data[_5031].x * (0.5 - (_4100 * _4100))) * 0.5))) + (((0.14000000059604644775390625 * _4078.y) * _4115) * sin(((_4100 * 2.400000095367431640625) - (_4096 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _23508;
        float _23509;
        float3 _23510;
        _23510 = float3(0.0);
        _23509 = _4171;
        _23508 = _4171;
        float3 _4243;
        float _23920;
        float _23921;
        for (int _23507 = 0; _23507 < 4; _23510 = _4243, _23509 = _23921, _23508 = _23920, _23507++)
        {
            float _4195 = _4171 + ((_4145 * _2841[_23507].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_4096 * 1.7000000476837158203125) + _2858[_23507].y))));
            _23920 = (_23507 == 0) ? _4195 : _23508;
            _23921 = (_23507 == 2) ? _4195 : _23509;
            float _4210 = _4108 * _2841[_23507].y;
            float _4215 = (_4105 - _4195) / _4210;
            float _4220 = _4108 * _2841[_23507].z;
            float3 _23900;
            _23900 = float3(0.0);
            for (int _23899 = 0; _23899 < 6; )
            {
                float _18636 = ((_4105 - _4195) - (_4220 * ((float(_23899) * 0.4000000059604644775390625) - 1.0))) / _4210;
                _23900 += (_1758[_23899] * exp((-_18636) * _18636));
                _23899++;
                continue;
            }
            _4243 = _23510 + (mix(_23900 * float3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), float3(exp((-_4215) * _4215)), float3(_2858[_23507].x)) * (_2841[_23507].w * _4117));
        }
        float _4249 = _4108 * 1.5;
        float _4279 = fast::clamp((_4105 - _23508) / fast::max(_23509 - _23508, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _4307 = (_4105 - (_23509 - (_4108 * 3.0))) / (((_4078.y * 0.0900000035762786865234375) + (_4145 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _4310 = (_4099 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _4335 = ((_4105 - _23508) + (_4108 * 5.0)) / (_4108 * 7.0);
        float3 _4361 = float3(1.0) - exp((-((((_23510 + (mix(float3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), float3(1.0), float3(_4279)) * ((((1.0 / (1.0 + exp((-((_4105 - _23508) - (_4108 * 2.0))) / _4249))) / (1.0 + exp((-(_23509 - _4105)) / _4249))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_4279, 2.5)))) * _4117))) + (float3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_4307) * _4307) - (_4310 * _4310)) * (0.5 + (1.10000002384185791015625 * _4136))))) + (float3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_4335) * _4335) * _4115) * 0.100000001490116119384765625))) * mix(float3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), float3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), float3(0.5 + (0.5 * sin(_4096 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _4371 = (gFxData_1._data[_4879].w * smoothstep(0.0, 0.119999997317790985107421875, _4085.y)) * smoothstep(1.0, 0.87999999523162841796875, _4085.y);
        float _4390 = (fast::clamp(fast::max(_4361.x, fast::max(_4361.y, _4361.z)), 0.0, 1.0) * _4371) * _3708;
        _23534 = float4((_4361 * _4371) * _3708, _4390) + (_23512 * (1.0 - _4390));
    }
    else
    {
        _23534 = _23512;
    }
    float4 _23543;
    if (((_3685 & 4u) != 0u) && ((_3685 & 256u) != 0u))
    {
        float2 _4416 = in.i_local - gFxData_1._data[_4927].zw;
        uint _18693 = _4855.z;
        float2 _18701 = (gFxData_1._data[_4861].xy + gFxData_1._data[_4861].zw) * 0.5;
        float2 _18710 = fast::max((gFxData_1._data[_4861].zw - gFxData_1._data[_4861].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _23529;
        if (_18693 == 1u)
        {
            float2 _18716 = _4416 - _18701;
            float _23528;
            do
            {
                if (gFxData_1._data[_4975].w >= 6.282185077667236328125)
                {
                    _23528 = abs(length(_18716) - gFxData_1._data[_4871].x) - gFxData_1._data[_4871].y;
                    break;
                }
                float _18815 = gFxData_1._data[_4975].z + (gFxData_1._data[_4975].w * 0.5);
                float _18817 = cos(_18815);
                float _18819 = sin(_18815);
                float _18828 = dot(_18716, float2(-_18819, _18817));
                float _18831 = dot(_18716, float2(_18817, _18819));
                float2 _18832 = float2(_18828, _18831);
                float _18835 = abs(_18828);
                _18832.x = _18835;
                float _18838 = gFxData_1._data[_4975].w * 0.5;
                float _18840 = sin(_18838);
                float _18842 = cos(_18838);
                _23528 = (((_18842 * _18835) > (_18840 * _18831)) ? length(_18832 - (float2(_18840, _18842) * gFxData_1._data[_4871].x)) : abs(length(_18832) - gFxData_1._data[_4871].x)) - gFxData_1._data[_4871].y;
                break;
            } while(false);
            _23529 = _23528;
        }
        else
        {
            float _23530;
            if (_18693 == 2u)
            {
                float2 _18878 = _4416 - gFxData_1._data[_4983].xy;
                float2 _18881 = gFxData_1._data[_4983].zw - gFxData_1._data[_4983].xy;
                _23530 = length(_18878 - (_18881 * fast::clamp(dot(_18878, _18881) / fast::max(dot(_18881, _18881), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4871].x;
            }
            else
            {
                float2 _18743 = _4416 - _18701;
                float _18934 = fast::min(_18710.x, _18710.y);
                float _18937 = fast::min((_18743.x > 0.0) ? ((_18743.y > 0.0) ? gFxData_1._data[_4871].z : gFxData_1._data[_4871].y) : ((_18743.y > 0.0) ? gFxData_1._data[_4871].w : gFxData_1._data[_4871].x), _18934);
                float _18943 = _18937 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4975].y));
                float _23513;
                float _23514;
                if (_18943 > _18934)
                {
                    float _18957 = gFxData_1._data[_4975].y * fast::clamp((_18934 - _18937) / fast::max(0.60000002384185791015625 * _18937, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _23514 = _18957;
                    _23513 = _18937 * (1.0 + (0.60000002384185791015625 * _18957));
                }
                else
                {
                    _23514 = gFxData_1._data[_4975].y;
                    _23513 = _18943;
                }
                float2 _18970 = (abs(_18743) - _18710) + float2(_23513);
                float2 _18972 = fast::max(_18970, float2(0.0));
                float _23515;
                if ((_18972.x > 0.0) && (_18972.y > 0.0))
                {
                    float _23516;
                    if (_23514 > 0.001000000047497451305389404296875)
                    {
                        float _18986 = 2.0 + (2.0 * _23514);
                        float2 _18991 = _18972 / float2(fast::max(_23513, 9.9999997473787516355514526367188e-05));
                        _23516 = pow(pow(_18991.x, _18986) + pow(_18991.y, _18986), 1.0 / _18986) * _23513;
                    }
                    else
                    {
                        _23516 = length(_18972);
                    }
                    _23515 = _23516;
                }
                else
                {
                    _23515 = fast::max(_18972.x, _18972.y);
                }
                float _19026 = (fast::min(fast::max(_18970.x, _18970.y), 0.0) + _23515) - _23513;
                float _23531;
                if ((_4855.x & 512u) != 0u)
                {
                    float2 _18771 = fast::max((gFxData_1._data[_4983].zw - gFxData_1._data[_4983].xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _18774 = _4416 - ((gFxData_1._data[_4983].xy + gFxData_1._data[_4983].zw) * 0.5);
                    float _19062 = fast::min(_18771.x, _18771.y);
                    float _19065 = fast::min((_18774.x > 0.0) ? ((_18774.y > 0.0) ? gFxData_1._data[_4991].x : gFxData_1._data[_4991].x) : ((_18774.y > 0.0) ? gFxData_1._data[_4991].x : gFxData_1._data[_4991].x), _19062);
                    float _19071 = _19065 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4975].y));
                    float _23519;
                    float _23520;
                    if (_19071 > _19062)
                    {
                        float _19085 = gFxData_1._data[_4975].y * fast::clamp((_19062 - _19065) / fast::max(0.60000002384185791015625 * _19065, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _23520 = _19085;
                        _23519 = _19065 * (1.0 + (0.60000002384185791015625 * _19085));
                    }
                    else
                    {
                        _23520 = gFxData_1._data[_4975].y;
                        _23519 = _19071;
                    }
                    float2 _19098 = (abs(_18774) - _18771) + float2(_23519);
                    float2 _19100 = fast::max(_19098, float2(0.0));
                    float _23521;
                    if ((_19100.x > 0.0) && (_19100.y > 0.0))
                    {
                        float _23522;
                        if (_23520 > 0.001000000047497451305389404296875)
                        {
                            float _19114 = 2.0 + (2.0 * _23520);
                            float2 _19119 = _19100 / float2(fast::max(_23519, 9.9999997473787516355514526367188e-05));
                            _23522 = pow(pow(_19119.x, _19114) + pow(_19119.y, _19114), 1.0 / _19114) * _23519;
                        }
                        else
                        {
                            _23522 = length(_19100);
                        }
                        _23521 = _23522;
                    }
                    else
                    {
                        _23521 = fast::max(_19100.x, _19100.y);
                    }
                    float _19154 = (fast::min(fast::max(_19098.x, _19098.y), 0.0) + _23521) - _23519;
                    float _19159 = fast::max(gFxData_1._data[_4991].y, 9.9999997473787516355514526367188e-05);
                    float _19168 = fast::max(_19159 - abs(_19026 - _19154), 0.0) / _19159;
                    _23531 = fast::min(_19026, _19154) - (((_19168 * _19168) * _19159) * 0.25);
                }
                else
                {
                    _23531 = _19026;
                }
                _23530 = _23531;
            }
            _23529 = _23530;
        }
        float _4425 = (_23529 + gFxData_1._data[_4927].y) / (fast::max(gFxData_1._data[_4927].x * 0.5, _3700 * 0.5) * 1.41421353816986083984375);
        float _19185 = sign(_4425);
        float _19187 = abs(_4425);
        float _19198 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_19187 * _19187))) * _19187)) * _19187);
        float _19201 = _19198 * _19198;
        float _19216 = fast::clamp(gFxData_1._data[_4919].w * ((0.5 + (0.5 * (_19185 - (_19185 / (_19201 * _19201))))) * _3708), 0.0, 1.0);
        _23543 = float4(gFxData_1._data[_4919].xyz * _19216, _19216) + (_23534 * (1.0 - _19216));
    }
    else
    {
        _23543 = _23534;
    }
    float4 _23566;
    if ((_3685 & 16u) != 0u)
    {
        float _4449 = fast::max(-_21748, 0.0) / fast::max(gFxData_1._data[_4943].z, 0.001000000047497451305389404296875);
        float _19242 = fast::clamp(gFxData_1._data[_4935].w * fast::clamp((exp(((-_4449) * _4449) * 2.2000000476837158203125) * gFxData_1._data[_4943].w) * _3708, 0.0, 1.0), 0.0, 1.0);
        _23566 = float4(gFxData_1._data[_4935].xyz * _19242, _19242) + (_23543 * (1.0 - _19242));
    }
    else
    {
        _23566 = _23543;
    }
    float3 _4478 = _23566.xyz + float3((_23548 * _3708) * _23566.w);
    float4 _21628 = _23566;
    _21628.x = _4478.x;
    _21628.y = _4478.y;
    _21628.z = _4478.z;
    float4 _23569;
    if ((_3685 & 2u) != 0u)
    {
        float4 _23567;
        if (gFxData_1._data[_4911].z < 0.999000012874603271484375)
        {
            float2 _4528 = fast::max(gFxData_1._data[_4861].zw - gFxData_1._data[_4861].xy, float2(0.001000000047497451305389404296875));
            float _4539 = cos(gFxData_1._data[_4911].w);
            float _4542 = sin(gFxData_1._data[_4911].w);
            float4 _21645 = _4905;
            _21645.w = _4905.w * mix(1.0, gFxData_1._data[_4911].z, fast::clamp(((dot(in.i_local - ((gFxData_1._data[_4861].xy + gFxData_1._data[_4861].zw) * 0.5), float2(_4539, _4542)) / fast::max(0.5 * ((abs(_4539) * _4528.x) + (abs(_4542) * _4528.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _23567 = _21645;
        }
        else
        {
            _23567 = _4905;
        }
        float _19268 = fast::clamp(_23567.w * (fast::clamp(0.5 - ((_21748 - (gFxData_1._data[_4911].x * gFxData_1._data[_4911].y)) / _3700), 0.0, 1.0) - fast::clamp(0.5 - ((_21748 + (gFxData_1._data[_4911].x * (1.0 - gFxData_1._data[_4911].y))) / _3700), 0.0, 1.0)), 0.0, 1.0);
        _23569 = float4(_23567.xyz * _19268, _19268) + (_21628 * (1.0 - _19268));
    }
    else
    {
        _23569 = _21628;
    }
    float4 _23570;
    if ((_3685 & 128u) != 0u)
    {
        float2 _4603 = (in.i_local - gFxData_1._data[_4861].xy) / fast::max(gFxData_1._data[_4861].zw - gFxData_1._data[_4861].xy, float2(0.001000000047497451305389404296875));
        float _4625 = exp(-pow((((_4603.x * 0.85000002384185791015625) + (_4603.y * 0.1500000059604644775390625)) - ((fract(_172.gTime.x * gFxData_1._data[_4999].w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _23570 = float4(_23569.xyz + float3(((_4625 * gFxData_1._data[_4999].z) * _3708) * fast::max(_23569.w, 0.3499999940395355224609375)), fast::max(_23569.w, ((_4625 * gFxData_1._data[_4999].z) * _3708) * 0.5));
    }
    else
    {
        _23570 = _23569;
    }
    float4 _23894;
    if ((_3685 & 2048u) != 0u)
    {
        float3 _19293 = fract(floor(_21731).xyx * 0.103100001811981201171875);
        float3 _19302 = _19293 + float3(dot(_19293, _19293.yzx + float3(33.3300018310546875)));
        float3 _4676 = _23570.xyz + float3(((fract((_19302.x + _19302.y) * _19302.z) - 0.5) * gFxData_1._data[_4999].y) * _23570.w);
        float4 _21669 = _23570;
        _21669.x = _4676.x;
        _21669.y = _4676.y;
        _21669.z = _4676.z;
        _23894 = _21669;
    }
    else
    {
        _23894 = _23570;
    }
    float _23890;
    if (_215.gFade.z > 0.0)
    {
        _23890 = smoothstep(0.0, 1.0, fast::clamp((_21731.y - _215.gFade.x) / _215.gFade.z, 0.0, 1.0));
    }
    else
    {
        _23890 = 1.0;
    }
    float _23891;
    if (_215.gFade.w > 0.0)
    {
        _23891 = _23890 * smoothstep(0.0, 1.0, fast::clamp((_215.gFade.y - _21731.y) / _215.gFade.w, 0.0, 1.0));
    }
    else
    {
        _23891 = _23890;
    }
    float4 _4693 = _23894 * ((gFxData_1._data[_4999].x * _23581) * _23891);
    float4 _23895;
    if ((_3685 & 12u) != 0u)
    {
        float3 _19354 = fract((floor(_21731) + float2(17.0)).xyx * 0.103100001811981201171875);
        float3 _19363 = _19354 + float3(dot(_19354, _19354.yzx + float3(33.3300018310546875)));
        float3 _4713 = _4693.xyz + float3(((fract((_19363.x + _19363.y) * _19363.z) - 0.5) * 0.0039215688593685626983642578125) * fast::clamp(_4693.w * 8.0, 0.0, 1.0));
        float4 _21681 = _4693;
        _21681.x = _4713.x;
        _21681.y = _4713.y;
        _21681.z = _4713.z;
        _23895 = _21681;
    }
    else
    {
        _23895 = _4693;
    }
    float3 _4723 = fast::max(_23895.xyz, float3(0.0));
    float4 _21687 = _23895;
    _21687.x = _4723.x;
    _21687.y = _4723.y;
    _21687.z = _4723.z;
    float4 _23896;
    if ((_172.gTime.w > 0.5) && (_23895.w > 9.9999997473787516355514526367188e-06))
    {
        float3 _19407 = fast::clamp(_21687.xyz / float3(_23895.w), float3(0.0), float3(1.0));
        float3 _19393 = select(pow((_19407 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _19407 * float3(0.077399380505084991455078125), _19407 <= float3(0.040449999272823333740234375)) * _23895.w;
        float4 _21696 = _21687;
        _21696.x = _19393.x;
        _21696.y = _19393.y;
        _21696.z = _19393.z;
        _23896 = _21696;
    }
    else
    {
        _23896 = _21687;
    }
    out._entryPointOutput = _23896;
    return out;
}

