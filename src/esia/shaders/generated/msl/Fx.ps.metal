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

constant spvUnsafeArray<float3, 6> _1761 = spvUnsafeArray<float3, 6>({ float3(1.0, 0.4199999868869781494140625, 0.2199999988079071044921875), float3(1.0, 0.699999988079071044921875, 0.300000011920928955078125), float3(0.800000011920928955078125, 0.920000016689300537109375, 0.4000000059604644775390625), float3(0.3499999940395355224609375, 0.89999997615814208984375, 0.699999988079071044921875), float3(0.4000000059604644775390625, 0.62000000476837158203125, 1.0), float3(0.660000026226043701171875, 0.5, 1.0) });
constant spvUnsafeArray<float4, 4> _2844 = spvUnsafeArray<float4, 4>({ float4(-1.0, 1.0, 3.400000095367431640625, 2.599999904632568359375), float4(-0.550000011920928955078125, 0.800000011920928955078125, 2.0, 0.800000011920928955078125), float4(0.300000011920928955078125, 1.0, 1.2000000476837158203125, 1.2999999523162841796875), float4(0.62000000476837158203125, 0.800000011920928955078125, 1.60000002384185791015625, 0.449999988079071044921875) });
constant spvUnsafeArray<float2, 4> _2861 = spvUnsafeArray<float2, 4>({ float2(0.0), float2(0.100000001490116119384765625, 1.2999999523162841796875), float2(0.550000011920928955078125, 3.900000095367431640625), float2(0.20000000298023223876953125, 5.19999980926513671875) });

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
    uint _4872 = in.i_instance * 24u;
    uint _4882 = (in.i_instance * 24u) + 1u;
    uint _4890 = (in.i_instance * 24u) + 2u;
    uint _4898 = (in.i_instance * 24u) + 3u;
    uint _4906 = (in.i_instance * 24u) + 4u;
    uint _4914 = (in.i_instance * 24u) + 5u;
    float4 _4916 = gFxData_1._data[_4914];
    uint _4922 = (in.i_instance * 24u) + 6u;
    uint _4930 = (in.i_instance * 24u) + 7u;
    uint _4938 = (in.i_instance * 24u) + 8u;
    uint _4946 = (in.i_instance * 24u) + 9u;
    uint _4954 = (in.i_instance * 24u) + 10u;
    uint _4962 = (in.i_instance * 24u) + 11u;
    uint _4970 = (in.i_instance * 24u) + 12u;
    uint _4978 = (in.i_instance * 24u) + 13u;
    uint _4986 = (in.i_instance * 24u) + 14u;
    uint _4994 = (in.i_instance * 24u) + 15u;
    uint _5002 = (in.i_instance * 24u) + 16u;
    uint _5010 = (in.i_instance * 24u) + 17u;
    uint _5018 = (in.i_instance * 24u) + 18u;
    uint _5026 = (in.i_instance * 24u) + 19u;
    uint _5034 = (in.i_instance * 24u) + 20u;
    uint _5042 = (in.i_instance * 24u) + 21u;
    uint _5050 = (in.i_instance * 24u) + 22u;
    uint4 _4866 = uint4(gFxData_1._data[(in.i_instance * 24u) + 23u]);
    uint _3692 = _4866.x;
    float2 _5069 = gl_FragCoord.xy + _172.gConv.yy;
    float2 _21787;
    if (_172.gConv.x > 0.5)
    {
        float2 _20100 = _5069;
        _20100.y = _172.gTarget.y - _5069.y;
        _21787 = _20100;
    }
    else
    {
        _21787 = _5069;
    }
    float _3700 = dfdx(in.i_local.x);
    float _3704 = dfdy(in.i_local.x);
    float _3707 = fast::max(abs(_3700) + abs(_3704), 9.9999997473787516355514526367188e-05);
    uint _5112 = _4866.z;
    float2 _5120 = (gFxData_1._data[_4872].xy + gFxData_1._data[_4872].zw) * 0.5;
    float2 _5129 = fast::max((gFxData_1._data[_4872].zw - gFxData_1._data[_4872].xy) * 0.5, float2(0.001000000047497451305389404296875));
    float _21804;
    if (_5112 == 1u)
    {
        float2 _5135 = in.i_local - _5120;
        float _21803;
        do
        {
            if (gFxData_1._data[_4986].w >= 6.282185077667236328125)
            {
                _21803 = abs(length(_5135) - gFxData_1._data[_4882].x) - gFxData_1._data[_4882].y;
                break;
            }
            float _5234 = gFxData_1._data[_4986].z + (gFxData_1._data[_4986].w * 0.5);
            float _5236 = cos(_5234);
            float _5238 = sin(_5234);
            float _5247 = dot(_5135, float2(-_5238, _5236));
            float _5250 = dot(_5135, float2(_5236, _5238));
            float2 _5251 = float2(_5247, _5250);
            float _5254 = abs(_5247);
            _5251.x = _5254;
            float _5257 = gFxData_1._data[_4986].w * 0.5;
            float _5259 = sin(_5257);
            float _5261 = cos(_5257);
            _21803 = (((_5261 * _5254) > (_5259 * _5250)) ? length(_5251 - (float2(_5259, _5261) * gFxData_1._data[_4882].x)) : abs(length(_5251) - gFxData_1._data[_4882].x)) - gFxData_1._data[_4882].y;
            break;
        } while(false);
        _21804 = _21803;
    }
    else
    {
        float _21805;
        if (_5112 == 2u)
        {
            float2 _5297 = in.i_local - gFxData_1._data[_4994].xy;
            float2 _5300 = gFxData_1._data[_4994].zw - gFxData_1._data[_4994].xy;
            _21805 = length(_5297 - (_5300 * fast::clamp(dot(_5297, _5300) / fast::max(dot(_5300, _5300), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4882].x;
        }
        else
        {
            float2 _5162 = in.i_local - _5120;
            float _5353 = fast::min(_5129.x, _5129.y);
            float _5356 = fast::min((_5162.x > 0.0) ? ((_5162.y > 0.0) ? gFxData_1._data[_4882].z : gFxData_1._data[_4882].y) : ((_5162.y > 0.0) ? gFxData_1._data[_4882].w : gFxData_1._data[_4882].x), _5353);
            float _5362 = _5356 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4986].y));
            float _21788;
            float _21789;
            if (_5362 > _5353)
            {
                float _5376 = gFxData_1._data[_4986].y * fast::clamp((_5353 - _5356) / fast::max(0.60000002384185791015625 * _5356, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                _21789 = _5376;
                _21788 = _5356 * (1.0 + (0.60000002384185791015625 * _5376));
            }
            else
            {
                _21789 = gFxData_1._data[_4986].y;
                _21788 = _5362;
            }
            float2 _5389 = (abs(_5162) - _5129) + float2(_21788);
            float2 _5391 = fast::max(_5389, float2(0.0));
            float _21790;
            if ((_5391.x > 0.0) && (_5391.y > 0.0))
            {
                float _21791;
                if ((_21789 > 0.001000000047497451305389404296875) && (_21788 > 9.9999997473787516355514526367188e-05))
                {
                    float _5408 = 2.0 + (2.0 * _21789);
                    float2 _5413 = _5391 / float2(fast::max(_21788, 9.9999997473787516355514526367188e-05));
                    _21791 = pow(pow(_5413.x, _5408) + pow(_5413.y, _5408), 1.0 / _5408) * _21788;
                }
                else
                {
                    _21791 = length(_5391);
                }
                _21790 = _21791;
            }
            else
            {
                _21790 = fast::max(_5391.x, _5391.y);
            }
            float _5448 = (fast::min(fast::max(_5389.x, _5389.y), 0.0) + _21790) - _21788;
            float _21806;
            if ((_4866.x & 512u) != 0u)
            {
                float2 _5190 = fast::max((gFxData_1._data[_4994].zw - gFxData_1._data[_4994].xy) * 0.5, float2(0.001000000047497451305389404296875));
                float2 _5193 = in.i_local - ((gFxData_1._data[_4994].xy + gFxData_1._data[_4994].zw) * 0.5);
                float _5484 = fast::min(_5190.x, _5190.y);
                float _5487 = fast::min((_5193.x > 0.0) ? ((_5193.y > 0.0) ? gFxData_1._data[_5002].x : gFxData_1._data[_5002].x) : ((_5193.y > 0.0) ? gFxData_1._data[_5002].x : gFxData_1._data[_5002].x), _5484);
                float _5493 = _5487 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4986].y));
                float _21794;
                float _21795;
                if (_5493 > _5484)
                {
                    float _5507 = gFxData_1._data[_4986].y * fast::clamp((_5484 - _5487) / fast::max(0.60000002384185791015625 * _5487, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _21795 = _5507;
                    _21794 = _5487 * (1.0 + (0.60000002384185791015625 * _5507));
                }
                else
                {
                    _21795 = gFxData_1._data[_4986].y;
                    _21794 = _5493;
                }
                float2 _5520 = (abs(_5193) - _5190) + float2(_21794);
                float2 _5522 = fast::max(_5520, float2(0.0));
                float _21796;
                if ((_5522.x > 0.0) && (_5522.y > 0.0))
                {
                    float _21797;
                    if ((_21795 > 0.001000000047497451305389404296875) && (_21794 > 9.9999997473787516355514526367188e-05))
                    {
                        float _5539 = 2.0 + (2.0 * _21795);
                        float2 _5544 = _5522 / float2(fast::max(_21794, 9.9999997473787516355514526367188e-05));
                        _21797 = pow(pow(_5544.x, _5539) + pow(_5544.y, _5539), 1.0 / _5539) * _21794;
                    }
                    else
                    {
                        _21797 = length(_5522);
                    }
                    _21796 = _21797;
                }
                else
                {
                    _21796 = fast::max(_5522.x, _5522.y);
                }
                float _5579 = (fast::min(fast::max(_5520.x, _5520.y), 0.0) + _21796) - _21794;
                float _5584 = fast::max(gFxData_1._data[_5002].y, 9.9999997473787516355514526367188e-05);
                float _5593 = fast::max(_5584 - abs(_5448 - _5579), 0.0) / _5584;
                _21806 = fast::min(_5448, _5579) - (((_5593 * _5593) * _5584) * 0.25);
            }
            else
            {
                _21806 = _5448;
            }
            _21805 = _21806;
        }
        _21804 = _21805;
    }
    float _3715 = fast::clamp(0.5 - (_21804 / _3707), 0.0, 1.0);
    float _23637;
    if ((_3692 & 1024u) != 0u)
    {
        float2 _3736 = fast::max((gFxData_1._data[_5026].zw - gFxData_1._data[_5026].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float2 _3739 = in.i_local - ((gFxData_1._data[_5026].xy + gFxData_1._data[_5026].zw) * 0.5);
        float _5639 = fast::min(_3736.x, _3736.y);
        float _5642 = fast::min((_3739.x > 0.0) ? ((_3739.y > 0.0) ? gFxData_1._data[_5034].x : gFxData_1._data[_5034].x) : ((_3739.y > 0.0) ? gFxData_1._data[_5034].x : gFxData_1._data[_5034].x), _5639);
        float _5648 = _5642 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_5034].y));
        float _21807;
        float _21808;
        if (_5648 > _5639)
        {
            float _5662 = gFxData_1._data[_5034].y * fast::clamp((_5639 - _5642) / fast::max(0.60000002384185791015625 * _5642, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _21808 = _5662;
            _21807 = _5642 * (1.0 + (0.60000002384185791015625 * _5662));
        }
        else
        {
            _21808 = gFxData_1._data[_5034].y;
            _21807 = _5648;
        }
        float2 _5675 = (abs(_3739) - _3736) + float2(_21807);
        float2 _5677 = fast::max(_5675, float2(0.0));
        float _21809;
        if ((_5677.x > 0.0) && (_5677.y > 0.0))
        {
            float _21810;
            if ((_21808 > 0.001000000047497451305389404296875) && (_21807 > 9.9999997473787516355514526367188e-05))
            {
                float _5694 = 2.0 + (2.0 * _21808);
                float2 _5699 = _5677 / float2(fast::max(_21807, 9.9999997473787516355514526367188e-05));
                _21810 = pow(pow(_5699.x, _5694) + pow(_5699.y, _5694), 1.0 / _5694) * _21807;
            }
            else
            {
                _21810 = length(_5677);
            }
            _21809 = _21810;
        }
        else
        {
            _21809 = fast::max(_5677.x, _5677.y);
        }
        float _3751 = fast::clamp(0.5 - (((fast::min(fast::max(_5675.x, _5675.y), 0.0) + _21809) - _21807) / _3707), 0.0, 1.0);
        if (_3751 <= 0.0)
        {
            discard_fragment();
        }
        _23637 = _3751;
    }
    else
    {
        _23637 = 1.0;
    }
    bool _3762 = ((_3692 & 32u) != 0u) && (_3715 >= 0.999000012874603271484375);
    float4 _21846;
    if ((((_3692 & 4u) != 0u) && (!((_3692 & 256u) != 0u))) && (!_3762))
    {
        float2 _3785 = in.i_local - gFxData_1._data[_4938].zw;
        uint _5765 = _4866.z;
        float2 _5773 = (gFxData_1._data[_4872].xy + gFxData_1._data[_4872].zw) * 0.5;
        float2 _5782 = fast::max((gFxData_1._data[_4872].zw - gFxData_1._data[_4872].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _21829;
        if (_5765 == 1u)
        {
            float2 _5788 = _3785 - _5773;
            float _21828;
            do
            {
                if (gFxData_1._data[_4986].w >= 6.282185077667236328125)
                {
                    _21828 = abs(length(_5788) - gFxData_1._data[_4882].x) - gFxData_1._data[_4882].y;
                    break;
                }
                float _5887 = gFxData_1._data[_4986].z + (gFxData_1._data[_4986].w * 0.5);
                float _5889 = cos(_5887);
                float _5891 = sin(_5887);
                float _5900 = dot(_5788, float2(-_5891, _5889));
                float _5903 = dot(_5788, float2(_5889, _5891));
                float2 _5904 = float2(_5900, _5903);
                float _5907 = abs(_5900);
                _5904.x = _5907;
                float _5910 = gFxData_1._data[_4986].w * 0.5;
                float _5912 = sin(_5910);
                float _5914 = cos(_5910);
                _21828 = (((_5914 * _5907) > (_5912 * _5903)) ? length(_5904 - (float2(_5912, _5914) * gFxData_1._data[_4882].x)) : abs(length(_5904) - gFxData_1._data[_4882].x)) - gFxData_1._data[_4882].y;
                break;
            } while(false);
            _21829 = _21828;
        }
        else
        {
            float _21830;
            if (_5765 == 2u)
            {
                float2 _5950 = _3785 - gFxData_1._data[_4994].xy;
                float2 _5953 = gFxData_1._data[_4994].zw - gFxData_1._data[_4994].xy;
                _21830 = length(_5950 - (_5953 * fast::clamp(dot(_5950, _5953) / fast::max(dot(_5953, _5953), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4882].x;
            }
            else
            {
                float2 _5815 = _3785 - _5773;
                float _6006 = fast::min(_5782.x, _5782.y);
                float _6009 = fast::min((_5815.x > 0.0) ? ((_5815.y > 0.0) ? gFxData_1._data[_4882].z : gFxData_1._data[_4882].y) : ((_5815.y > 0.0) ? gFxData_1._data[_4882].w : gFxData_1._data[_4882].x), _6006);
                float _6015 = _6009 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4986].y));
                float _21813;
                float _21814;
                if (_6015 > _6006)
                {
                    float _6029 = gFxData_1._data[_4986].y * fast::clamp((_6006 - _6009) / fast::max(0.60000002384185791015625 * _6009, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _21814 = _6029;
                    _21813 = _6009 * (1.0 + (0.60000002384185791015625 * _6029));
                }
                else
                {
                    _21814 = gFxData_1._data[_4986].y;
                    _21813 = _6015;
                }
                float2 _6042 = (abs(_5815) - _5782) + float2(_21813);
                float2 _6044 = fast::max(_6042, float2(0.0));
                float _21815;
                if ((_6044.x > 0.0) && (_6044.y > 0.0))
                {
                    float _21816;
                    if ((_21814 > 0.001000000047497451305389404296875) && (_21813 > 9.9999997473787516355514526367188e-05))
                    {
                        float _6061 = 2.0 + (2.0 * _21814);
                        float2 _6066 = _6044 / float2(fast::max(_21813, 9.9999997473787516355514526367188e-05));
                        _21816 = pow(pow(_6066.x, _6061) + pow(_6066.y, _6061), 1.0 / _6061) * _21813;
                    }
                    else
                    {
                        _21816 = length(_6044);
                    }
                    _21815 = _21816;
                }
                else
                {
                    _21815 = fast::max(_6044.x, _6044.y);
                }
                float _6101 = (fast::min(fast::max(_6042.x, _6042.y), 0.0) + _21815) - _21813;
                float _21831;
                if ((_4866.x & 512u) != 0u)
                {
                    float2 _5843 = fast::max((gFxData_1._data[_4994].zw - gFxData_1._data[_4994].xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _5846 = _3785 - ((gFxData_1._data[_4994].xy + gFxData_1._data[_4994].zw) * 0.5);
                    float _6137 = fast::min(_5843.x, _5843.y);
                    float _6140 = fast::min((_5846.x > 0.0) ? ((_5846.y > 0.0) ? gFxData_1._data[_5002].x : gFxData_1._data[_5002].x) : ((_5846.y > 0.0) ? gFxData_1._data[_5002].x : gFxData_1._data[_5002].x), _6137);
                    float _6146 = _6140 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4986].y));
                    float _21819;
                    float _21820;
                    if (_6146 > _6137)
                    {
                        float _6160 = gFxData_1._data[_4986].y * fast::clamp((_6137 - _6140) / fast::max(0.60000002384185791015625 * _6140, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21820 = _6160;
                        _21819 = _6140 * (1.0 + (0.60000002384185791015625 * _6160));
                    }
                    else
                    {
                        _21820 = gFxData_1._data[_4986].y;
                        _21819 = _6146;
                    }
                    float2 _6173 = (abs(_5846) - _5843) + float2(_21819);
                    float2 _6175 = fast::max(_6173, float2(0.0));
                    float _21821;
                    if ((_6175.x > 0.0) && (_6175.y > 0.0))
                    {
                        float _21822;
                        if ((_21820 > 0.001000000047497451305389404296875) && (_21819 > 9.9999997473787516355514526367188e-05))
                        {
                            float _6192 = 2.0 + (2.0 * _21820);
                            float2 _6197 = _6175 / float2(fast::max(_21819, 9.9999997473787516355514526367188e-05));
                            _21822 = pow(pow(_6197.x, _6192) + pow(_6197.y, _6192), 1.0 / _6192) * _21819;
                        }
                        else
                        {
                            _21822 = length(_6175);
                        }
                        _21821 = _21822;
                    }
                    else
                    {
                        _21821 = fast::max(_6175.x, _6175.y);
                    }
                    float _6232 = (fast::min(fast::max(_6173.x, _6173.y), 0.0) + _21821) - _21819;
                    float _6237 = fast::max(gFxData_1._data[_5002].y, 9.9999997473787516355514526367188e-05);
                    float _6246 = fast::max(_6237 - abs(_6101 - _6232), 0.0) / _6237;
                    _21831 = fast::min(_6101, _6232) - (((_6246 * _6246) * _6237) * 0.25);
                }
                else
                {
                    _21831 = _6101;
                }
                _21830 = _21831;
            }
            _21829 = _21830;
        }
        float _3794 = (_21829 - gFxData_1._data[_4938].y) / (fast::max(gFxData_1._data[_4938].x * 0.5, _3707 * 0.5) * 1.41421353816986083984375);
        float _6263 = sign(_3794);
        float _6265 = abs(_3794);
        float _6276 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_6265 * _6265))) * _6265)) * _6265);
        float _6279 = _6276 * _6276;
        float _6294 = fast::clamp(gFxData_1._data[_4930].w * (0.5 - (0.5 * (_6263 - (_6263 / (_6279 * _6279))))), 0.0, 1.0);
        _21846 = float4(gFxData_1._data[_4930].xyz * _6294, _6294);
    }
    else
    {
        _21846 = float4(0.0);
    }
    float4 _23271;
    if (((_3692 & 8u) != 0u) && (!_3762))
    {
        float _3814 = fast::max(gFxData_1._data[_4954].x, 0.001000000047497451305389404296875);
        float _21843;
        float _21844;
        if ((_3692 & 8192u) != 0u)
        {
            float2 _3826 = (gFxData_1._data[_4872].xy + gFxData_1._data[_4872].zw) * 0.5;
            float2 _3835 = fast::max((gFxData_1._data[_4872].zw - gFxData_1._data[_4872].xy) * 0.5, float2(0.001000000047497451305389404296875));
            float2 _3842 = gFxData_1._data[_4872].xy - gFxData_1._data[_5050].xy;
            float2 _3849 = gFxData_1._data[_5050].zw - gFxData_1._data[_4872].zw;
            float2 _3879 = fast::clamp(float2((in.i_local.x < _3826.x) ? _3842.x : _3849.x, (in.i_local.y < _3826.y) ? _3842.y : _3849.y) * float2(0.58823525905609130859375), float2(fast::min(_3814, 1.5)), float2(_3814));
            float _3884 = fast::min(_3835.x, _3835.y);
            float _21842;
            if (_4866.z == 0u)
            {
                _21842 = fast::min(((in.i_local.x > _3826.x) ? ((in.i_local.y > _3826.y) ? gFxData_1._data[_4882].z : gFxData_1._data[_4882].y) : ((in.i_local.y > _3826.y) ? gFxData_1._data[_4882].w : gFxData_1._data[_4882].x)) * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4986].y)), _3884);
            }
            else
            {
                _21842 = _3884;
            }
            float2 _3933 = fast::max(abs(in.i_local - _3826) - (_3835 - float2(_21842)), float2(0.0));
            float _3935 = length(_3933);
            float2 _3943 = select(float2(0.707099974155426025390625), _3933 / float2(_3935), bool2(_3935 > 9.9999997473787516355514526367188e-05));
            float2 _3958 = in.i_local - gFxData_1._data[_5050].xy;
            float2 _3963 = gFxData_1._data[_5050].zw - in.i_local;
            _21844 = fast::clamp(fast::min(fast::min(_3958.x, _3958.y), fast::min(_3963.x, _3963.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _21843 = rsqrt(dot(_3943 * _3943, float2(1.0) / (_3879 * _3879)));
        }
        else
        {
            _21844 = 1.0;
            _21843 = _3814;
        }
        float _3981 = fast::max(_21804, 0.0) / _21843;
        float _6320 = fast::clamp(gFxData_1._data[_4946].w * fast::clamp((exp(((-_3981) * _3981) * 2.2000000476837158203125) * gFxData_1._data[_4954].y) * _21844, 0.0, 1.0), 0.0, 1.0);
        _23271 = float4(gFxData_1._data[_4946].xyz * _6320, _6320) + (_21846 * (1.0 - _6320));
    }
    else
    {
        _23271 = _21846;
    }
    float4 _23558;
    float _23604;
    if (((_3692 & 32u) != 0u) && (_3715 > 0.0))
    {
        float _6398 = gFxData_1._data[_4962].x * _172.gDisplay.z;
        float2 _6409 = fast::max((gFxData_1._data[_4872].zw - gFxData_1._data[_4872].xy) * 0.5, float2(1.0));
        float _6417 = fast::clamp(gFxData_1._data[_4962].z, 0.001000000047497451305389404296875, fast::min(_6409.x, _6409.y));
        float _6426 = fast::clamp(1.0 - (fast::max(-_21804, 0.0) / _6417), 0.0, 1.0);
        float _6432 = sqrt(fast::clamp(1.0 - (_6426 * _6426), 0.0, 1.0));
        float2 _21924;
        float3 _22786;
        if (_6426 > 0.0)
        {
            float2 _6778 = in.i_local + float2(0.5, 0.0);
            uint _6838 = _4866.z;
            float2 _6846 = (gFxData_1._data[_4872].xy + gFxData_1._data[_4872].zw) * 0.5;
            float2 _6855 = fast::max((gFxData_1._data[_4872].zw - gFxData_1._data[_4872].xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _21864;
            if (_6838 == 1u)
            {
                float2 _6861 = _6778 - _6846;
                float _21863;
                do
                {
                    if (gFxData_1._data[_4986].w >= 6.282185077667236328125)
                    {
                        _21863 = abs(length(_6861) - gFxData_1._data[_4882].x) - gFxData_1._data[_4882].y;
                        break;
                    }
                    float _6960 = gFxData_1._data[_4986].z + (gFxData_1._data[_4986].w * 0.5);
                    float _6962 = cos(_6960);
                    float _6964 = sin(_6960);
                    float _6973 = dot(_6861, float2(-_6964, _6962));
                    float _6976 = dot(_6861, float2(_6962, _6964));
                    float2 _6977 = float2(_6973, _6976);
                    float _6980 = abs(_6973);
                    _6977.x = _6980;
                    float _6983 = gFxData_1._data[_4986].w * 0.5;
                    float _6985 = sin(_6983);
                    float _6987 = cos(_6983);
                    _21863 = (((_6987 * _6980) > (_6985 * _6976)) ? length(_6977 - (float2(_6985, _6987) * gFxData_1._data[_4882].x)) : abs(length(_6977) - gFxData_1._data[_4882].x)) - gFxData_1._data[_4882].y;
                    break;
                } while(false);
                _21864 = _21863;
            }
            else
            {
                float _21865;
                if (_6838 == 2u)
                {
                    float2 _7023 = _6778 - gFxData_1._data[_4994].xy;
                    float2 _7026 = gFxData_1._data[_4994].zw - gFxData_1._data[_4994].xy;
                    _21865 = length(_7023 - (_7026 * fast::clamp(dot(_7023, _7026) / fast::max(dot(_7026, _7026), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4882].x;
                }
                else
                {
                    float2 _6888 = _6778 - _6846;
                    float _7079 = fast::min(_6855.x, _6855.y);
                    float _7082 = fast::min((_6888.x > 0.0) ? ((_6888.y > 0.0) ? gFxData_1._data[_4882].z : gFxData_1._data[_4882].y) : ((_6888.y > 0.0) ? gFxData_1._data[_4882].w : gFxData_1._data[_4882].x), _7079);
                    float _7088 = _7082 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4986].y));
                    float _21848;
                    float _21849;
                    if (_7088 > _7079)
                    {
                        float _7102 = gFxData_1._data[_4986].y * fast::clamp((_7079 - _7082) / fast::max(0.60000002384185791015625 * _7082, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21849 = _7102;
                        _21848 = _7082 * (1.0 + (0.60000002384185791015625 * _7102));
                    }
                    else
                    {
                        _21849 = gFxData_1._data[_4986].y;
                        _21848 = _7088;
                    }
                    float2 _7115 = (abs(_6888) - _6855) + float2(_21848);
                    float2 _7117 = fast::max(_7115, float2(0.0));
                    float _21850;
                    if ((_7117.x > 0.0) && (_7117.y > 0.0))
                    {
                        float _21851;
                        if ((_21849 > 0.001000000047497451305389404296875) && (_21848 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7134 = 2.0 + (2.0 * _21849);
                            float2 _7139 = _7117 / float2(fast::max(_21848, 9.9999997473787516355514526367188e-05));
                            _21851 = pow(pow(_7139.x, _7134) + pow(_7139.y, _7134), 1.0 / _7134) * _21848;
                        }
                        else
                        {
                            _21851 = length(_7117);
                        }
                        _21850 = _21851;
                    }
                    else
                    {
                        _21850 = fast::max(_7117.x, _7117.y);
                    }
                    float _7174 = (fast::min(fast::max(_7115.x, _7115.y), 0.0) + _21850) - _21848;
                    float _21866;
                    if ((_4866.x & 512u) != 0u)
                    {
                        float2 _6916 = fast::max((gFxData_1._data[_4994].zw - gFxData_1._data[_4994].xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _6919 = _6778 - ((gFxData_1._data[_4994].xy + gFxData_1._data[_4994].zw) * 0.5);
                        float _7210 = fast::min(_6916.x, _6916.y);
                        float _7213 = fast::min((_6919.x > 0.0) ? ((_6919.y > 0.0) ? gFxData_1._data[_5002].x : gFxData_1._data[_5002].x) : ((_6919.y > 0.0) ? gFxData_1._data[_5002].x : gFxData_1._data[_5002].x), _7210);
                        float _7219 = _7213 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4986].y));
                        float _21854;
                        float _21855;
                        if (_7219 > _7210)
                        {
                            float _7233 = gFxData_1._data[_4986].y * fast::clamp((_7210 - _7213) / fast::max(0.60000002384185791015625 * _7213, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21855 = _7233;
                            _21854 = _7213 * (1.0 + (0.60000002384185791015625 * _7233));
                        }
                        else
                        {
                            _21855 = gFxData_1._data[_4986].y;
                            _21854 = _7219;
                        }
                        float2 _7246 = (abs(_6919) - _6916) + float2(_21854);
                        float2 _7248 = fast::max(_7246, float2(0.0));
                        float _21856;
                        if ((_7248.x > 0.0) && (_7248.y > 0.0))
                        {
                            float _21857;
                            if ((_21855 > 0.001000000047497451305389404296875) && (_21854 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7265 = 2.0 + (2.0 * _21855);
                                float2 _7270 = _7248 / float2(fast::max(_21854, 9.9999997473787516355514526367188e-05));
                                _21857 = pow(pow(_7270.x, _7265) + pow(_7270.y, _7265), 1.0 / _7265) * _21854;
                            }
                            else
                            {
                                _21857 = length(_7248);
                            }
                            _21856 = _21857;
                        }
                        else
                        {
                            _21856 = fast::max(_7248.x, _7248.y);
                        }
                        float _7305 = (fast::min(fast::max(_7246.x, _7246.y), 0.0) + _21856) - _21854;
                        float _7310 = fast::max(gFxData_1._data[_5002].y, 9.9999997473787516355514526367188e-05);
                        float _7319 = fast::max(_7310 - abs(_7174 - _7305), 0.0) / _7310;
                        _21866 = fast::min(_7174, _7305) - (((_7319 * _7319) * _7310) * 0.25);
                    }
                    else
                    {
                        _21866 = _7174;
                    }
                    _21865 = _21866;
                }
                _21864 = _21865;
            }
            float2 _6782 = in.i_local - float2(0.5, 0.0);
            uint _7360 = _4866.z;
            float2 _7368 = (gFxData_1._data[_4872].xy + gFxData_1._data[_4872].zw) * 0.5;
            float2 _7377 = fast::max((gFxData_1._data[_4872].zw - gFxData_1._data[_4872].xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _21883;
            if (_7360 == 1u)
            {
                float2 _7383 = _6782 - _7368;
                float _21882;
                do
                {
                    if (gFxData_1._data[_4986].w >= 6.282185077667236328125)
                    {
                        _21882 = abs(length(_7383) - gFxData_1._data[_4882].x) - gFxData_1._data[_4882].y;
                        break;
                    }
                    float _7482 = gFxData_1._data[_4986].z + (gFxData_1._data[_4986].w * 0.5);
                    float _7484 = cos(_7482);
                    float _7486 = sin(_7482);
                    float _7495 = dot(_7383, float2(-_7486, _7484));
                    float _7498 = dot(_7383, float2(_7484, _7486));
                    float2 _7499 = float2(_7495, _7498);
                    float _7502 = abs(_7495);
                    _7499.x = _7502;
                    float _7505 = gFxData_1._data[_4986].w * 0.5;
                    float _7507 = sin(_7505);
                    float _7509 = cos(_7505);
                    _21882 = (((_7509 * _7502) > (_7507 * _7498)) ? length(_7499 - (float2(_7507, _7509) * gFxData_1._data[_4882].x)) : abs(length(_7499) - gFxData_1._data[_4882].x)) - gFxData_1._data[_4882].y;
                    break;
                } while(false);
                _21883 = _21882;
            }
            else
            {
                float _21884;
                if (_7360 == 2u)
                {
                    float2 _7545 = _6782 - gFxData_1._data[_4994].xy;
                    float2 _7548 = gFxData_1._data[_4994].zw - gFxData_1._data[_4994].xy;
                    _21884 = length(_7545 - (_7548 * fast::clamp(dot(_7545, _7548) / fast::max(dot(_7548, _7548), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4882].x;
                }
                else
                {
                    float2 _7410 = _6782 - _7368;
                    float _7601 = fast::min(_7377.x, _7377.y);
                    float _7604 = fast::min((_7410.x > 0.0) ? ((_7410.y > 0.0) ? gFxData_1._data[_4882].z : gFxData_1._data[_4882].y) : ((_7410.y > 0.0) ? gFxData_1._data[_4882].w : gFxData_1._data[_4882].x), _7601);
                    float _7610 = _7604 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4986].y));
                    float _21867;
                    float _21868;
                    if (_7610 > _7601)
                    {
                        float _7624 = gFxData_1._data[_4986].y * fast::clamp((_7601 - _7604) / fast::max(0.60000002384185791015625 * _7604, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21868 = _7624;
                        _21867 = _7604 * (1.0 + (0.60000002384185791015625 * _7624));
                    }
                    else
                    {
                        _21868 = gFxData_1._data[_4986].y;
                        _21867 = _7610;
                    }
                    float2 _7637 = (abs(_7410) - _7377) + float2(_21867);
                    float2 _7639 = fast::max(_7637, float2(0.0));
                    float _21869;
                    if ((_7639.x > 0.0) && (_7639.y > 0.0))
                    {
                        float _21870;
                        if ((_21868 > 0.001000000047497451305389404296875) && (_21867 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7656 = 2.0 + (2.0 * _21868);
                            float2 _7661 = _7639 / float2(fast::max(_21867, 9.9999997473787516355514526367188e-05));
                            _21870 = pow(pow(_7661.x, _7656) + pow(_7661.y, _7656), 1.0 / _7656) * _21867;
                        }
                        else
                        {
                            _21870 = length(_7639);
                        }
                        _21869 = _21870;
                    }
                    else
                    {
                        _21869 = fast::max(_7639.x, _7639.y);
                    }
                    float _7696 = (fast::min(fast::max(_7637.x, _7637.y), 0.0) + _21869) - _21867;
                    float _21885;
                    if ((_4866.x & 512u) != 0u)
                    {
                        float2 _7438 = fast::max((gFxData_1._data[_4994].zw - gFxData_1._data[_4994].xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _7441 = _6782 - ((gFxData_1._data[_4994].xy + gFxData_1._data[_4994].zw) * 0.5);
                        float _7732 = fast::min(_7438.x, _7438.y);
                        float _7735 = fast::min((_7441.x > 0.0) ? ((_7441.y > 0.0) ? gFxData_1._data[_5002].x : gFxData_1._data[_5002].x) : ((_7441.y > 0.0) ? gFxData_1._data[_5002].x : gFxData_1._data[_5002].x), _7732);
                        float _7741 = _7735 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4986].y));
                        float _21873;
                        float _21874;
                        if (_7741 > _7732)
                        {
                            float _7755 = gFxData_1._data[_4986].y * fast::clamp((_7732 - _7735) / fast::max(0.60000002384185791015625 * _7735, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21874 = _7755;
                            _21873 = _7735 * (1.0 + (0.60000002384185791015625 * _7755));
                        }
                        else
                        {
                            _21874 = gFxData_1._data[_4986].y;
                            _21873 = _7741;
                        }
                        float2 _7768 = (abs(_7441) - _7438) + float2(_21873);
                        float2 _7770 = fast::max(_7768, float2(0.0));
                        float _21875;
                        if ((_7770.x > 0.0) && (_7770.y > 0.0))
                        {
                            float _21876;
                            if ((_21874 > 0.001000000047497451305389404296875) && (_21873 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7787 = 2.0 + (2.0 * _21874);
                                float2 _7792 = _7770 / float2(fast::max(_21873, 9.9999997473787516355514526367188e-05));
                                _21876 = pow(pow(_7792.x, _7787) + pow(_7792.y, _7787), 1.0 / _7787) * _21873;
                            }
                            else
                            {
                                _21876 = length(_7770);
                            }
                            _21875 = _21876;
                        }
                        else
                        {
                            _21875 = fast::max(_7770.x, _7770.y);
                        }
                        float _7827 = (fast::min(fast::max(_7768.x, _7768.y), 0.0) + _21875) - _21873;
                        float _7832 = fast::max(gFxData_1._data[_5002].y, 9.9999997473787516355514526367188e-05);
                        float _7841 = fast::max(_7832 - abs(_7696 - _7827), 0.0) / _7832;
                        _21885 = fast::min(_7696, _7827) - (((_7841 * _7841) * _7832) * 0.25);
                    }
                    else
                    {
                        _21885 = _7696;
                    }
                    _21884 = _21885;
                }
                _21883 = _21884;
            }
            float2 _6787 = in.i_local + float2(0.0, 0.5);
            uint _7882 = _4866.z;
            float2 _7890 = (gFxData_1._data[_4872].xy + gFxData_1._data[_4872].zw) * 0.5;
            float2 _7899 = fast::max((gFxData_1._data[_4872].zw - gFxData_1._data[_4872].xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _21902;
            if (_7882 == 1u)
            {
                float2 _7905 = _6787 - _7890;
                float _21901;
                do
                {
                    if (gFxData_1._data[_4986].w >= 6.282185077667236328125)
                    {
                        _21901 = abs(length(_7905) - gFxData_1._data[_4882].x) - gFxData_1._data[_4882].y;
                        break;
                    }
                    float _8004 = gFxData_1._data[_4986].z + (gFxData_1._data[_4986].w * 0.5);
                    float _8006 = cos(_8004);
                    float _8008 = sin(_8004);
                    float _8017 = dot(_7905, float2(-_8008, _8006));
                    float _8020 = dot(_7905, float2(_8006, _8008));
                    float2 _8021 = float2(_8017, _8020);
                    float _8024 = abs(_8017);
                    _8021.x = _8024;
                    float _8027 = gFxData_1._data[_4986].w * 0.5;
                    float _8029 = sin(_8027);
                    float _8031 = cos(_8027);
                    _21901 = (((_8031 * _8024) > (_8029 * _8020)) ? length(_8021 - (float2(_8029, _8031) * gFxData_1._data[_4882].x)) : abs(length(_8021) - gFxData_1._data[_4882].x)) - gFxData_1._data[_4882].y;
                    break;
                } while(false);
                _21902 = _21901;
            }
            else
            {
                float _21903;
                if (_7882 == 2u)
                {
                    float2 _8067 = _6787 - gFxData_1._data[_4994].xy;
                    float2 _8070 = gFxData_1._data[_4994].zw - gFxData_1._data[_4994].xy;
                    _21903 = length(_8067 - (_8070 * fast::clamp(dot(_8067, _8070) / fast::max(dot(_8070, _8070), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4882].x;
                }
                else
                {
                    float2 _7932 = _6787 - _7890;
                    float _8123 = fast::min(_7899.x, _7899.y);
                    float _8126 = fast::min((_7932.x > 0.0) ? ((_7932.y > 0.0) ? gFxData_1._data[_4882].z : gFxData_1._data[_4882].y) : ((_7932.y > 0.0) ? gFxData_1._data[_4882].w : gFxData_1._data[_4882].x), _8123);
                    float _8132 = _8126 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4986].y));
                    float _21886;
                    float _21887;
                    if (_8132 > _8123)
                    {
                        float _8146 = gFxData_1._data[_4986].y * fast::clamp((_8123 - _8126) / fast::max(0.60000002384185791015625 * _8126, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21887 = _8146;
                        _21886 = _8126 * (1.0 + (0.60000002384185791015625 * _8146));
                    }
                    else
                    {
                        _21887 = gFxData_1._data[_4986].y;
                        _21886 = _8132;
                    }
                    float2 _8159 = (abs(_7932) - _7899) + float2(_21886);
                    float2 _8161 = fast::max(_8159, float2(0.0));
                    float _21888;
                    if ((_8161.x > 0.0) && (_8161.y > 0.0))
                    {
                        float _21889;
                        if ((_21887 > 0.001000000047497451305389404296875) && (_21886 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8178 = 2.0 + (2.0 * _21887);
                            float2 _8183 = _8161 / float2(fast::max(_21886, 9.9999997473787516355514526367188e-05));
                            _21889 = pow(pow(_8183.x, _8178) + pow(_8183.y, _8178), 1.0 / _8178) * _21886;
                        }
                        else
                        {
                            _21889 = length(_8161);
                        }
                        _21888 = _21889;
                    }
                    else
                    {
                        _21888 = fast::max(_8161.x, _8161.y);
                    }
                    float _8218 = (fast::min(fast::max(_8159.x, _8159.y), 0.0) + _21888) - _21886;
                    float _21904;
                    if ((_4866.x & 512u) != 0u)
                    {
                        float2 _7960 = fast::max((gFxData_1._data[_4994].zw - gFxData_1._data[_4994].xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _7963 = _6787 - ((gFxData_1._data[_4994].xy + gFxData_1._data[_4994].zw) * 0.5);
                        float _8254 = fast::min(_7960.x, _7960.y);
                        float _8257 = fast::min((_7963.x > 0.0) ? ((_7963.y > 0.0) ? gFxData_1._data[_5002].x : gFxData_1._data[_5002].x) : ((_7963.y > 0.0) ? gFxData_1._data[_5002].x : gFxData_1._data[_5002].x), _8254);
                        float _8263 = _8257 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4986].y));
                        float _21892;
                        float _21893;
                        if (_8263 > _8254)
                        {
                            float _8277 = gFxData_1._data[_4986].y * fast::clamp((_8254 - _8257) / fast::max(0.60000002384185791015625 * _8257, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21893 = _8277;
                            _21892 = _8257 * (1.0 + (0.60000002384185791015625 * _8277));
                        }
                        else
                        {
                            _21893 = gFxData_1._data[_4986].y;
                            _21892 = _8263;
                        }
                        float2 _8290 = (abs(_7963) - _7960) + float2(_21892);
                        float2 _8292 = fast::max(_8290, float2(0.0));
                        float _21894;
                        if ((_8292.x > 0.0) && (_8292.y > 0.0))
                        {
                            float _21895;
                            if ((_21893 > 0.001000000047497451305389404296875) && (_21892 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8309 = 2.0 + (2.0 * _21893);
                                float2 _8314 = _8292 / float2(fast::max(_21892, 9.9999997473787516355514526367188e-05));
                                _21895 = pow(pow(_8314.x, _8309) + pow(_8314.y, _8309), 1.0 / _8309) * _21892;
                            }
                            else
                            {
                                _21895 = length(_8292);
                            }
                            _21894 = _21895;
                        }
                        else
                        {
                            _21894 = fast::max(_8292.x, _8292.y);
                        }
                        float _8349 = (fast::min(fast::max(_8290.x, _8290.y), 0.0) + _21894) - _21892;
                        float _8354 = fast::max(gFxData_1._data[_5002].y, 9.9999997473787516355514526367188e-05);
                        float _8363 = fast::max(_8354 - abs(_8218 - _8349), 0.0) / _8354;
                        _21904 = fast::min(_8218, _8349) - (((_8363 * _8363) * _8354) * 0.25);
                    }
                    else
                    {
                        _21904 = _8218;
                    }
                    _21903 = _21904;
                }
                _21902 = _21903;
            }
            float2 _6791 = in.i_local - float2(0.0, 0.5);
            uint _8404 = _4866.z;
            float2 _8412 = (gFxData_1._data[_4872].xy + gFxData_1._data[_4872].zw) * 0.5;
            float2 _8421 = fast::max((gFxData_1._data[_4872].zw - gFxData_1._data[_4872].xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _21921;
            if (_8404 == 1u)
            {
                float2 _8427 = _6791 - _8412;
                float _21920;
                do
                {
                    if (gFxData_1._data[_4986].w >= 6.282185077667236328125)
                    {
                        _21920 = abs(length(_8427) - gFxData_1._data[_4882].x) - gFxData_1._data[_4882].y;
                        break;
                    }
                    float _8526 = gFxData_1._data[_4986].z + (gFxData_1._data[_4986].w * 0.5);
                    float _8528 = cos(_8526);
                    float _8530 = sin(_8526);
                    float _8539 = dot(_8427, float2(-_8530, _8528));
                    float _8542 = dot(_8427, float2(_8528, _8530));
                    float2 _8543 = float2(_8539, _8542);
                    float _8546 = abs(_8539);
                    _8543.x = _8546;
                    float _8549 = gFxData_1._data[_4986].w * 0.5;
                    float _8551 = sin(_8549);
                    float _8553 = cos(_8549);
                    _21920 = (((_8553 * _8546) > (_8551 * _8542)) ? length(_8543 - (float2(_8551, _8553) * gFxData_1._data[_4882].x)) : abs(length(_8543) - gFxData_1._data[_4882].x)) - gFxData_1._data[_4882].y;
                    break;
                } while(false);
                _21921 = _21920;
            }
            else
            {
                float _21922;
                if (_8404 == 2u)
                {
                    float2 _8589 = _6791 - gFxData_1._data[_4994].xy;
                    float2 _8592 = gFxData_1._data[_4994].zw - gFxData_1._data[_4994].xy;
                    _21922 = length(_8589 - (_8592 * fast::clamp(dot(_8589, _8592) / fast::max(dot(_8592, _8592), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4882].x;
                }
                else
                {
                    float2 _8454 = _6791 - _8412;
                    float _8645 = fast::min(_8421.x, _8421.y);
                    float _8648 = fast::min((_8454.x > 0.0) ? ((_8454.y > 0.0) ? gFxData_1._data[_4882].z : gFxData_1._data[_4882].y) : ((_8454.y > 0.0) ? gFxData_1._data[_4882].w : gFxData_1._data[_4882].x), _8645);
                    float _8654 = _8648 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4986].y));
                    float _21905;
                    float _21906;
                    if (_8654 > _8645)
                    {
                        float _8668 = gFxData_1._data[_4986].y * fast::clamp((_8645 - _8648) / fast::max(0.60000002384185791015625 * _8648, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21906 = _8668;
                        _21905 = _8648 * (1.0 + (0.60000002384185791015625 * _8668));
                    }
                    else
                    {
                        _21906 = gFxData_1._data[_4986].y;
                        _21905 = _8654;
                    }
                    float2 _8681 = (abs(_8454) - _8421) + float2(_21905);
                    float2 _8683 = fast::max(_8681, float2(0.0));
                    float _21907;
                    if ((_8683.x > 0.0) && (_8683.y > 0.0))
                    {
                        float _21908;
                        if ((_21906 > 0.001000000047497451305389404296875) && (_21905 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8700 = 2.0 + (2.0 * _21906);
                            float2 _8705 = _8683 / float2(fast::max(_21905, 9.9999997473787516355514526367188e-05));
                            _21908 = pow(pow(_8705.x, _8700) + pow(_8705.y, _8700), 1.0 / _8700) * _21905;
                        }
                        else
                        {
                            _21908 = length(_8683);
                        }
                        _21907 = _21908;
                    }
                    else
                    {
                        _21907 = fast::max(_8683.x, _8683.y);
                    }
                    float _8740 = (fast::min(fast::max(_8681.x, _8681.y), 0.0) + _21907) - _21905;
                    float _21923;
                    if ((_4866.x & 512u) != 0u)
                    {
                        float2 _8482 = fast::max((gFxData_1._data[_4994].zw - gFxData_1._data[_4994].xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _8485 = _6791 - ((gFxData_1._data[_4994].xy + gFxData_1._data[_4994].zw) * 0.5);
                        float _8776 = fast::min(_8482.x, _8482.y);
                        float _8779 = fast::min((_8485.x > 0.0) ? ((_8485.y > 0.0) ? gFxData_1._data[_5002].x : gFxData_1._data[_5002].x) : ((_8485.y > 0.0) ? gFxData_1._data[_5002].x : gFxData_1._data[_5002].x), _8776);
                        float _8785 = _8779 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4986].y));
                        float _21911;
                        float _21912;
                        if (_8785 > _8776)
                        {
                            float _8799 = gFxData_1._data[_4986].y * fast::clamp((_8776 - _8779) / fast::max(0.60000002384185791015625 * _8779, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21912 = _8799;
                            _21911 = _8779 * (1.0 + (0.60000002384185791015625 * _8799));
                        }
                        else
                        {
                            _21912 = gFxData_1._data[_4986].y;
                            _21911 = _8785;
                        }
                        float2 _8812 = (abs(_8485) - _8482) + float2(_21911);
                        float2 _8814 = fast::max(_8812, float2(0.0));
                        float _21913;
                        if ((_8814.x > 0.0) && (_8814.y > 0.0))
                        {
                            float _21914;
                            if ((_21912 > 0.001000000047497451305389404296875) && (_21911 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8831 = 2.0 + (2.0 * _21912);
                                float2 _8836 = _8814 / float2(fast::max(_21911, 9.9999997473787516355514526367188e-05));
                                _21914 = pow(pow(_8836.x, _8831) + pow(_8836.y, _8831), 1.0 / _8831) * _21911;
                            }
                            else
                            {
                                _21914 = length(_8814);
                            }
                            _21913 = _21914;
                        }
                        else
                        {
                            _21913 = fast::max(_8814.x, _8814.y);
                        }
                        float _8871 = (fast::min(fast::max(_8812.x, _8812.y), 0.0) + _21913) - _21911;
                        float _8876 = fast::max(gFxData_1._data[_5002].y, 9.9999997473787516355514526367188e-05);
                        float _8885 = fast::max(_8876 - abs(_8740 - _8871), 0.0) / _8876;
                        _21923 = fast::min(_8740, _8871) - (((_8885 * _8885) * _8876) * 0.25);
                    }
                    else
                    {
                        _21923 = _8740;
                    }
                    _21922 = _21923;
                }
                _21921 = _21922;
            }
            float2 _6797 = float2(_21864 - _21883, _21902 - _21921);
            float _6799 = length(_6797);
            float2 _6807 = select(float2(0.0, -1.0), _6797 / float2(_6799), bool2(_6799 > 9.9999997473787516355514526367188e-06));
            _22786 = fast::normalize(float3(_6807 * fast::min(_6426 / fast::max(_6432, 0.001000000047497451305389404296875), 8.0), 1.0));
            _21924 = _6807;
        }
        else
        {
            _22786 = float3(0.0, 0.0, 1.0);
            _21924 = float2(0.0, -1.0);
        }
        float2 _6458 = ((-_21924) * gFxData_1._data[_4962].y) * (1.0 - _6432);
        float2 _21925;
        if (gFxData_1._data[_5002].z > 0.0)
        {
            _21925 = (((gFxData_1._data[_4872].xy + gFxData_1._data[_4872].zw) * 0.5) - in.i_local) * (gFxData_1._data[_5002].z / (1.0 + gFxData_1._data[_5002].z));
        }
        else
        {
            _21925 = float2(0.0);
        }
        float2 _6484 = _21787 * _172.gTarget.zw;
        float2 _6491 = _172.gDisplay.zw * _172.gTarget.zw;
        float2 _6496 = (_6458 + _21925) * _6491;
        float2 _6499 = _6458 * _6491;
        float3 _22530;
        float _22553;
        float3 _23022;
        if (_172.gTime.z > 0.5)
        {
            float3 _22533;
            if (((gFxData_1._data[_4962].w > 0.001000000047497451305389404296875) && (_6426 > 0.0)) && ((((gFxData_1._data[_4962].y * 0.300000011920928955078125) * gFxData_1._data[_4962].w) * _172.gDisplay.z) > (_6398 * 0.3499999940395355224609375)))
            {
                float _6521 = 0.300000011920928955078125 * gFxData_1._data[_4962].w;
                float2 _6528 = (_6484 + _6496) - (_6499 * _6521);
                float _8910 = fast::clamp(log2(fast::max(_6398, 1.0)) - 1.0, 0.0, 5.0);
                int _8913 = int(floor(_8910));
                float _8917 = _8910 - float(_8913);
                float4 _22000;
                if (_8913 <= 0)
                {
                    float2 _21999;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _20482 = _6528;
                        _20482.y = 1.0 - _6528.y;
                        _21999 = _20482;
                    }
                    else
                    {
                        _21999 = _6528;
                    }
                    _22000 = gBackdrop0.sample(gLinear, _21999, level(0.0));
                }
                else
                {
                    float4 _22001;
                    if (_8913 == 1)
                    {
                        float2 _9052 = (_6528 * _172.gLevel[1].xy) - float2(0.5);
                        float2 _9054 = floor(_9052);
                        float2 _9057 = _9052 - _9054;
                        float2 _9060 = _9057 * _9057;
                        float2 _9063 = _9060 * _9057;
                        float2 _9082 = (((_9063 * 3.0) - (_9060 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _9095 = _9063 * 0.16666667163372039794921875;
                        float2 _9098 = (((((-_9063) + (_9060 * 3.0)) - (_9057 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9082;
                        float2 _9101 = (((((_9063 * (-3.0)) + (_9060 * 3.0)) + (_9057 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9095;
                        float2 _9111 = ((_9054 - float2(0.5)) + (_9082 / _9098)) * _172.gLevel[1].zw;
                        float2 _9121 = ((_9054 + float2(1.5)) + (_9095 / _9101)) * _172.gLevel[1].zw;
                        float2 _21995;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20487 = _9111;
                            _20487.y = 1.0 - _9111.y;
                            _21995 = _20487;
                        }
                        else
                        {
                            _21995 = _9111;
                        }
                        float _9141 = _9111.y;
                        float2 _9142 = float2(_9121.x, _9141);
                        float2 _21996;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20493 = _9142;
                            _20493.y = 1.0 - _9141;
                            _21996 = _20493;
                        }
                        else
                        {
                            _21996 = _9142;
                        }
                        float _9158 = _9121.y;
                        float2 _9159 = float2(_9111.x, _9158);
                        float2 _21997;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20500 = _9159;
                            _20500.y = 1.0 - _9158;
                            _21997 = _20500;
                        }
                        else
                        {
                            _21997 = _9159;
                        }
                        float2 _21998;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20506 = _9121;
                            _20506.y = 1.0 - _9121.y;
                            _21998 = _20506;
                        }
                        else
                        {
                            _21998 = _9121;
                        }
                        _22001 = (((gBackdrop1.sample(gLinear, _21995, level(0.0)) * _9098.x) + (gBackdrop1.sample(gLinear, _21996, level(0.0)) * _9101.x)) * _9098.y) + (((gBackdrop1.sample(gLinear, _21997, level(0.0)) * _9098.x) + (gBackdrop1.sample(gLinear, _21998, level(0.0)) * _9101.x)) * _9101.y);
                    }
                    else
                    {
                        float4 _22002;
                        if (_8913 == 2)
                        {
                            float2 _9259 = (_6528 * _172.gLevel[2].xy) - float2(0.5);
                            float2 _9261 = floor(_9259);
                            float2 _9264 = _9259 - _9261;
                            float2 _9267 = _9264 * _9264;
                            float2 _9270 = _9267 * _9264;
                            float2 _9289 = (((_9270 * 3.0) - (_9267 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _9302 = _9270 * 0.16666667163372039794921875;
                            float2 _9305 = (((((-_9270) + (_9267 * 3.0)) - (_9264 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9289;
                            float2 _9308 = (((((_9270 * (-3.0)) + (_9267 * 3.0)) + (_9264 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9302;
                            float2 _9318 = ((_9261 - float2(0.5)) + (_9289 / _9305)) * _172.gLevel[2].zw;
                            float2 _9328 = ((_9261 + float2(1.5)) + (_9302 / _9308)) * _172.gLevel[2].zw;
                            float2 _21991;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20513 = _9318;
                                _20513.y = 1.0 - _9318.y;
                                _21991 = _20513;
                            }
                            else
                            {
                                _21991 = _9318;
                            }
                            float _9348 = _9318.y;
                            float2 _9349 = float2(_9328.x, _9348);
                            float2 _21992;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20519 = _9349;
                                _20519.y = 1.0 - _9348;
                                _21992 = _20519;
                            }
                            else
                            {
                                _21992 = _9349;
                            }
                            float _9365 = _9328.y;
                            float2 _9366 = float2(_9318.x, _9365);
                            float2 _21993;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20526 = _9366;
                                _20526.y = 1.0 - _9365;
                                _21993 = _20526;
                            }
                            else
                            {
                                _21993 = _9366;
                            }
                            float2 _21994;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20532 = _9328;
                                _20532.y = 1.0 - _9328.y;
                                _21994 = _20532;
                            }
                            else
                            {
                                _21994 = _9328;
                            }
                            _22002 = (((gBackdrop2.sample(gLinear, _21991, level(0.0)) * _9305.x) + (gBackdrop2.sample(gLinear, _21992, level(0.0)) * _9308.x)) * _9305.y) + (((gBackdrop2.sample(gLinear, _21993, level(0.0)) * _9305.x) + (gBackdrop2.sample(gLinear, _21994, level(0.0)) * _9308.x)) * _9308.y);
                        }
                        else
                        {
                            float4 _22003;
                            if (_8913 == 3)
                            {
                                float2 _9466 = (_6528 * _172.gLevel[3].xy) - float2(0.5);
                                float2 _9468 = floor(_9466);
                                float2 _9471 = _9466 - _9468;
                                float2 _9474 = _9471 * _9471;
                                float2 _9477 = _9474 * _9471;
                                float2 _9496 = (((_9477 * 3.0) - (_9474 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _9509 = _9477 * 0.16666667163372039794921875;
                                float2 _9512 = (((((-_9477) + (_9474 * 3.0)) - (_9471 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9496;
                                float2 _9515 = (((((_9477 * (-3.0)) + (_9474 * 3.0)) + (_9471 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9509;
                                float2 _9525 = ((_9468 - float2(0.5)) + (_9496 / _9512)) * _172.gLevel[3].zw;
                                float2 _9535 = ((_9468 + float2(1.5)) + (_9509 / _9515)) * _172.gLevel[3].zw;
                                float2 _21987;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20539 = _9525;
                                    _20539.y = 1.0 - _9525.y;
                                    _21987 = _20539;
                                }
                                else
                                {
                                    _21987 = _9525;
                                }
                                float _9555 = _9525.y;
                                float2 _9556 = float2(_9535.x, _9555);
                                float2 _21988;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20545 = _9556;
                                    _20545.y = 1.0 - _9555;
                                    _21988 = _20545;
                                }
                                else
                                {
                                    _21988 = _9556;
                                }
                                float _9572 = _9535.y;
                                float2 _9573 = float2(_9525.x, _9572);
                                float2 _21989;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20552 = _9573;
                                    _20552.y = 1.0 - _9572;
                                    _21989 = _20552;
                                }
                                else
                                {
                                    _21989 = _9573;
                                }
                                float2 _21990;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20558 = _9535;
                                    _20558.y = 1.0 - _9535.y;
                                    _21990 = _20558;
                                }
                                else
                                {
                                    _21990 = _9535;
                                }
                                _22003 = (((gBackdrop3.sample(gLinear, _21987, level(0.0)) * _9512.x) + (gBackdrop3.sample(gLinear, _21988, level(0.0)) * _9515.x)) * _9512.y) + (((gBackdrop3.sample(gLinear, _21989, level(0.0)) * _9512.x) + (gBackdrop3.sample(gLinear, _21990, level(0.0)) * _9515.x)) * _9515.y);
                            }
                            else
                            {
                                float4 _22004;
                                if (_8913 == 4)
                                {
                                    float2 _9673 = (_6528 * _172.gLevel[4].xy) - float2(0.5);
                                    float2 _9675 = floor(_9673);
                                    float2 _9678 = _9673 - _9675;
                                    float2 _9681 = _9678 * _9678;
                                    float2 _9684 = _9681 * _9678;
                                    float2 _9703 = (((_9684 * 3.0) - (_9681 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _9716 = _9684 * 0.16666667163372039794921875;
                                    float2 _9719 = (((((-_9684) + (_9681 * 3.0)) - (_9678 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9703;
                                    float2 _9722 = (((((_9684 * (-3.0)) + (_9681 * 3.0)) + (_9678 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9716;
                                    float2 _9732 = ((_9675 - float2(0.5)) + (_9703 / _9719)) * _172.gLevel[4].zw;
                                    float2 _9742 = ((_9675 + float2(1.5)) + (_9716 / _9722)) * _172.gLevel[4].zw;
                                    float2 _21983;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20565 = _9732;
                                        _20565.y = 1.0 - _9732.y;
                                        _21983 = _20565;
                                    }
                                    else
                                    {
                                        _21983 = _9732;
                                    }
                                    float _9762 = _9732.y;
                                    float2 _9763 = float2(_9742.x, _9762);
                                    float2 _21984;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20571 = _9763;
                                        _20571.y = 1.0 - _9762;
                                        _21984 = _20571;
                                    }
                                    else
                                    {
                                        _21984 = _9763;
                                    }
                                    float _9779 = _9742.y;
                                    float2 _9780 = float2(_9732.x, _9779);
                                    float2 _21985;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20578 = _9780;
                                        _20578.y = 1.0 - _9779;
                                        _21985 = _20578;
                                    }
                                    else
                                    {
                                        _21985 = _9780;
                                    }
                                    float2 _21986;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20584 = _9742;
                                        _20584.y = 1.0 - _9742.y;
                                        _21986 = _20584;
                                    }
                                    else
                                    {
                                        _21986 = _9742;
                                    }
                                    _22004 = (((gBackdrop4.sample(gLinear, _21983, level(0.0)) * _9719.x) + (gBackdrop4.sample(gLinear, _21984, level(0.0)) * _9722.x)) * _9719.y) + (((gBackdrop4.sample(gLinear, _21985, level(0.0)) * _9719.x) + (gBackdrop4.sample(gLinear, _21986, level(0.0)) * _9722.x)) * _9722.y);
                                }
                                else
                                {
                                    float2 _9880 = (_6528 * _172.gLevel[5].xy) - float2(0.5);
                                    float2 _9882 = floor(_9880);
                                    float2 _9885 = _9880 - _9882;
                                    float2 _9888 = _9885 * _9885;
                                    float2 _9891 = _9888 * _9885;
                                    float2 _9910 = (((_9891 * 3.0) - (_9888 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _9923 = _9891 * 0.16666667163372039794921875;
                                    float2 _9926 = (((((-_9891) + (_9888 * 3.0)) - (_9885 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9910;
                                    float2 _9929 = (((((_9891 * (-3.0)) + (_9888 * 3.0)) + (_9885 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9923;
                                    float2 _9939 = ((_9882 - float2(0.5)) + (_9910 / _9926)) * _172.gLevel[5].zw;
                                    float2 _9949 = ((_9882 + float2(1.5)) + (_9923 / _9929)) * _172.gLevel[5].zw;
                                    float2 _21979;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20591 = _9939;
                                        _20591.y = 1.0 - _9939.y;
                                        _21979 = _20591;
                                    }
                                    else
                                    {
                                        _21979 = _9939;
                                    }
                                    float _9969 = _9939.y;
                                    float2 _9970 = float2(_9949.x, _9969);
                                    float2 _21980;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20597 = _9970;
                                        _20597.y = 1.0 - _9969;
                                        _21980 = _20597;
                                    }
                                    else
                                    {
                                        _21980 = _9970;
                                    }
                                    float _9986 = _9949.y;
                                    float2 _9987 = float2(_9939.x, _9986);
                                    float2 _21981;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20604 = _9987;
                                        _20604.y = 1.0 - _9986;
                                        _21981 = _20604;
                                    }
                                    else
                                    {
                                        _21981 = _9987;
                                    }
                                    float2 _21982;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20610 = _9949;
                                        _20610.y = 1.0 - _9949.y;
                                        _21982 = _20610;
                                    }
                                    else
                                    {
                                        _21982 = _9949;
                                    }
                                    _22004 = (((gBackdrop5.sample(gLinear, _21979, level(0.0)) * _9926.x) + (gBackdrop5.sample(gLinear, _21980, level(0.0)) * _9929.x)) * _9926.y) + (((gBackdrop5.sample(gLinear, _21981, level(0.0)) * _9926.x) + (gBackdrop5.sample(gLinear, _21982, level(0.0)) * _9929.x)) * _9929.y);
                                }
                                _22003 = _22004;
                            }
                            _22002 = _22003;
                        }
                        _22001 = _22002;
                    }
                    _22000 = _22001;
                }
                float3 _22031;
                if ((_8917 > 0.0199999995529651641845703125) && (_8913 < 5))
                {
                    int _8930 = _8913 + 1;
                    float4 _22026;
                    if (_8930 <= 0)
                    {
                        float2 _22025;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20615 = _6528;
                            _20615.y = 1.0 - _6528.y;
                            _22025 = _20615;
                        }
                        else
                        {
                            _22025 = _6528;
                        }
                        _22026 = gBackdrop0.sample(gLinear, _22025, level(0.0));
                    }
                    else
                    {
                        float4 _22027;
                        if (_8930 == 1)
                        {
                            float2 _10176 = (_6528 * _172.gLevel[1].xy) - float2(0.5);
                            float2 _10178 = floor(_10176);
                            float2 _10181 = _10176 - _10178;
                            float2 _10184 = _10181 * _10181;
                            float2 _10187 = _10184 * _10181;
                            float2 _10206 = (((_10187 * 3.0) - (_10184 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _10219 = _10187 * 0.16666667163372039794921875;
                            float2 _10222 = (((((-_10187) + (_10184 * 3.0)) - (_10181 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10206;
                            float2 _10225 = (((((_10187 * (-3.0)) + (_10184 * 3.0)) + (_10181 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10219;
                            float2 _10235 = ((_10178 - float2(0.5)) + (_10206 / _10222)) * _172.gLevel[1].zw;
                            float2 _10245 = ((_10178 + float2(1.5)) + (_10219 / _10225)) * _172.gLevel[1].zw;
                            float2 _22021;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20620 = _10235;
                                _20620.y = 1.0 - _10235.y;
                                _22021 = _20620;
                            }
                            else
                            {
                                _22021 = _10235;
                            }
                            float _10265 = _10235.y;
                            float2 _10266 = float2(_10245.x, _10265);
                            float2 _22022;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20626 = _10266;
                                _20626.y = 1.0 - _10265;
                                _22022 = _20626;
                            }
                            else
                            {
                                _22022 = _10266;
                            }
                            float _10282 = _10245.y;
                            float2 _10283 = float2(_10235.x, _10282);
                            float2 _22023;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20633 = _10283;
                                _20633.y = 1.0 - _10282;
                                _22023 = _20633;
                            }
                            else
                            {
                                _22023 = _10283;
                            }
                            float2 _22024;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20639 = _10245;
                                _20639.y = 1.0 - _10245.y;
                                _22024 = _20639;
                            }
                            else
                            {
                                _22024 = _10245;
                            }
                            _22027 = (((gBackdrop1.sample(gLinear, _22021, level(0.0)) * _10222.x) + (gBackdrop1.sample(gLinear, _22022, level(0.0)) * _10225.x)) * _10222.y) + (((gBackdrop1.sample(gLinear, _22023, level(0.0)) * _10222.x) + (gBackdrop1.sample(gLinear, _22024, level(0.0)) * _10225.x)) * _10225.y);
                        }
                        else
                        {
                            float4 _22028;
                            if (_8930 == 2)
                            {
                                float2 _10383 = (_6528 * _172.gLevel[2].xy) - float2(0.5);
                                float2 _10385 = floor(_10383);
                                float2 _10388 = _10383 - _10385;
                                float2 _10391 = _10388 * _10388;
                                float2 _10394 = _10391 * _10388;
                                float2 _10413 = (((_10394 * 3.0) - (_10391 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _10426 = _10394 * 0.16666667163372039794921875;
                                float2 _10429 = (((((-_10394) + (_10391 * 3.0)) - (_10388 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10413;
                                float2 _10432 = (((((_10394 * (-3.0)) + (_10391 * 3.0)) + (_10388 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10426;
                                float2 _10442 = ((_10385 - float2(0.5)) + (_10413 / _10429)) * _172.gLevel[2].zw;
                                float2 _10452 = ((_10385 + float2(1.5)) + (_10426 / _10432)) * _172.gLevel[2].zw;
                                float2 _22017;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20646 = _10442;
                                    _20646.y = 1.0 - _10442.y;
                                    _22017 = _20646;
                                }
                                else
                                {
                                    _22017 = _10442;
                                }
                                float _10472 = _10442.y;
                                float2 _10473 = float2(_10452.x, _10472);
                                float2 _22018;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20652 = _10473;
                                    _20652.y = 1.0 - _10472;
                                    _22018 = _20652;
                                }
                                else
                                {
                                    _22018 = _10473;
                                }
                                float _10489 = _10452.y;
                                float2 _10490 = float2(_10442.x, _10489);
                                float2 _22019;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20659 = _10490;
                                    _20659.y = 1.0 - _10489;
                                    _22019 = _20659;
                                }
                                else
                                {
                                    _22019 = _10490;
                                }
                                float2 _22020;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20665 = _10452;
                                    _20665.y = 1.0 - _10452.y;
                                    _22020 = _20665;
                                }
                                else
                                {
                                    _22020 = _10452;
                                }
                                _22028 = (((gBackdrop2.sample(gLinear, _22017, level(0.0)) * _10429.x) + (gBackdrop2.sample(gLinear, _22018, level(0.0)) * _10432.x)) * _10429.y) + (((gBackdrop2.sample(gLinear, _22019, level(0.0)) * _10429.x) + (gBackdrop2.sample(gLinear, _22020, level(0.0)) * _10432.x)) * _10432.y);
                            }
                            else
                            {
                                float4 _22029;
                                if (_8930 == 3)
                                {
                                    float2 _10590 = (_6528 * _172.gLevel[3].xy) - float2(0.5);
                                    float2 _10592 = floor(_10590);
                                    float2 _10595 = _10590 - _10592;
                                    float2 _10598 = _10595 * _10595;
                                    float2 _10601 = _10598 * _10595;
                                    float2 _10620 = (((_10601 * 3.0) - (_10598 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _10633 = _10601 * 0.16666667163372039794921875;
                                    float2 _10636 = (((((-_10601) + (_10598 * 3.0)) - (_10595 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10620;
                                    float2 _10639 = (((((_10601 * (-3.0)) + (_10598 * 3.0)) + (_10595 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10633;
                                    float2 _10649 = ((_10592 - float2(0.5)) + (_10620 / _10636)) * _172.gLevel[3].zw;
                                    float2 _10659 = ((_10592 + float2(1.5)) + (_10633 / _10639)) * _172.gLevel[3].zw;
                                    float2 _22013;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20672 = _10649;
                                        _20672.y = 1.0 - _10649.y;
                                        _22013 = _20672;
                                    }
                                    else
                                    {
                                        _22013 = _10649;
                                    }
                                    float _10679 = _10649.y;
                                    float2 _10680 = float2(_10659.x, _10679);
                                    float2 _22014;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20678 = _10680;
                                        _20678.y = 1.0 - _10679;
                                        _22014 = _20678;
                                    }
                                    else
                                    {
                                        _22014 = _10680;
                                    }
                                    float _10696 = _10659.y;
                                    float2 _10697 = float2(_10649.x, _10696);
                                    float2 _22015;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20685 = _10697;
                                        _20685.y = 1.0 - _10696;
                                        _22015 = _20685;
                                    }
                                    else
                                    {
                                        _22015 = _10697;
                                    }
                                    float2 _22016;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20691 = _10659;
                                        _20691.y = 1.0 - _10659.y;
                                        _22016 = _20691;
                                    }
                                    else
                                    {
                                        _22016 = _10659;
                                    }
                                    _22029 = (((gBackdrop3.sample(gLinear, _22013, level(0.0)) * _10636.x) + (gBackdrop3.sample(gLinear, _22014, level(0.0)) * _10639.x)) * _10636.y) + (((gBackdrop3.sample(gLinear, _22015, level(0.0)) * _10636.x) + (gBackdrop3.sample(gLinear, _22016, level(0.0)) * _10639.x)) * _10639.y);
                                }
                                else
                                {
                                    float4 _22030;
                                    if (_8930 == 4)
                                    {
                                        float2 _10797 = (_6528 * _172.gLevel[4].xy) - float2(0.5);
                                        float2 _10799 = floor(_10797);
                                        float2 _10802 = _10797 - _10799;
                                        float2 _10805 = _10802 * _10802;
                                        float2 _10808 = _10805 * _10802;
                                        float2 _10827 = (((_10808 * 3.0) - (_10805 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _10840 = _10808 * 0.16666667163372039794921875;
                                        float2 _10843 = (((((-_10808) + (_10805 * 3.0)) - (_10802 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10827;
                                        float2 _10846 = (((((_10808 * (-3.0)) + (_10805 * 3.0)) + (_10802 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10840;
                                        float2 _10856 = ((_10799 - float2(0.5)) + (_10827 / _10843)) * _172.gLevel[4].zw;
                                        float2 _10866 = ((_10799 + float2(1.5)) + (_10840 / _10846)) * _172.gLevel[4].zw;
                                        float2 _22009;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20698 = _10856;
                                            _20698.y = 1.0 - _10856.y;
                                            _22009 = _20698;
                                        }
                                        else
                                        {
                                            _22009 = _10856;
                                        }
                                        float _10886 = _10856.y;
                                        float2 _10887 = float2(_10866.x, _10886);
                                        float2 _22010;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20704 = _10887;
                                            _20704.y = 1.0 - _10886;
                                            _22010 = _20704;
                                        }
                                        else
                                        {
                                            _22010 = _10887;
                                        }
                                        float _10903 = _10866.y;
                                        float2 _10904 = float2(_10856.x, _10903);
                                        float2 _22011;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20711 = _10904;
                                            _20711.y = 1.0 - _10903;
                                            _22011 = _20711;
                                        }
                                        else
                                        {
                                            _22011 = _10904;
                                        }
                                        float2 _22012;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20717 = _10866;
                                            _20717.y = 1.0 - _10866.y;
                                            _22012 = _20717;
                                        }
                                        else
                                        {
                                            _22012 = _10866;
                                        }
                                        _22030 = (((gBackdrop4.sample(gLinear, _22009, level(0.0)) * _10843.x) + (gBackdrop4.sample(gLinear, _22010, level(0.0)) * _10846.x)) * _10843.y) + (((gBackdrop4.sample(gLinear, _22011, level(0.0)) * _10843.x) + (gBackdrop4.sample(gLinear, _22012, level(0.0)) * _10846.x)) * _10846.y);
                                    }
                                    else
                                    {
                                        float2 _11004 = (_6528 * _172.gLevel[5].xy) - float2(0.5);
                                        float2 _11006 = floor(_11004);
                                        float2 _11009 = _11004 - _11006;
                                        float2 _11012 = _11009 * _11009;
                                        float2 _11015 = _11012 * _11009;
                                        float2 _11034 = (((_11015 * 3.0) - (_11012 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _11047 = _11015 * 0.16666667163372039794921875;
                                        float2 _11050 = (((((-_11015) + (_11012 * 3.0)) - (_11009 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11034;
                                        float2 _11053 = (((((_11015 * (-3.0)) + (_11012 * 3.0)) + (_11009 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11047;
                                        float2 _11063 = ((_11006 - float2(0.5)) + (_11034 / _11050)) * _172.gLevel[5].zw;
                                        float2 _11073 = ((_11006 + float2(1.5)) + (_11047 / _11053)) * _172.gLevel[5].zw;
                                        float2 _22005;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20724 = _11063;
                                            _20724.y = 1.0 - _11063.y;
                                            _22005 = _20724;
                                        }
                                        else
                                        {
                                            _22005 = _11063;
                                        }
                                        float _11093 = _11063.y;
                                        float2 _11094 = float2(_11073.x, _11093);
                                        float2 _22006;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20730 = _11094;
                                            _20730.y = 1.0 - _11093;
                                            _22006 = _20730;
                                        }
                                        else
                                        {
                                            _22006 = _11094;
                                        }
                                        float _11110 = _11073.y;
                                        float2 _11111 = float2(_11063.x, _11110);
                                        float2 _22007;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20737 = _11111;
                                            _20737.y = 1.0 - _11110;
                                            _22007 = _20737;
                                        }
                                        else
                                        {
                                            _22007 = _11111;
                                        }
                                        float2 _22008;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20743 = _11073;
                                            _20743.y = 1.0 - _11073.y;
                                            _22008 = _20743;
                                        }
                                        else
                                        {
                                            _22008 = _11073;
                                        }
                                        _22030 = (((gBackdrop5.sample(gLinear, _22005, level(0.0)) * _11050.x) + (gBackdrop5.sample(gLinear, _22006, level(0.0)) * _11053.x)) * _11050.y) + (((gBackdrop5.sample(gLinear, _22007, level(0.0)) * _11050.x) + (gBackdrop5.sample(gLinear, _22008, level(0.0)) * _11053.x)) * _11053.y);
                                    }
                                    _22029 = _22030;
                                }
                                _22028 = _22029;
                            }
                            _22027 = _22028;
                        }
                        _22026 = _22027;
                    }
                    _22031 = mix(_22000.xyz, _22026.xyz, float3(_8917));
                }
                else
                {
                    _22031 = _22000.xyz;
                }
                float2 _6535 = _6484 + _6496;
                float _11201 = fast::clamp(log2(fast::max(_6398, 1.0)) - 1.0, 0.0, 5.0);
                int _11204 = int(floor(_11201));
                float _11208 = _11201 - float(_11204);
                float4 _22106;
                if (_11204 <= 0)
                {
                    float2 _22105;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _20750 = _6535;
                        _20750.y = 1.0 - _6535.y;
                        _22105 = _20750;
                    }
                    else
                    {
                        _22105 = _6535;
                    }
                    _22106 = gBackdrop0.sample(gLinear, _22105, level(0.0));
                }
                else
                {
                    float4 _22107;
                    if (_11204 == 1)
                    {
                        float2 _11343 = (_6535 * _172.gLevel[1].xy) - float2(0.5);
                        float2 _11345 = floor(_11343);
                        float2 _11348 = _11343 - _11345;
                        float2 _11351 = _11348 * _11348;
                        float2 _11354 = _11351 * _11348;
                        float2 _11373 = (((_11354 * 3.0) - (_11351 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _11386 = _11354 * 0.16666667163372039794921875;
                        float2 _11389 = (((((-_11354) + (_11351 * 3.0)) - (_11348 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11373;
                        float2 _11392 = (((((_11354 * (-3.0)) + (_11351 * 3.0)) + (_11348 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11386;
                        float2 _11402 = ((_11345 - float2(0.5)) + (_11373 / _11389)) * _172.gLevel[1].zw;
                        float2 _11412 = ((_11345 + float2(1.5)) + (_11386 / _11392)) * _172.gLevel[1].zw;
                        float2 _22101;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20755 = _11402;
                            _20755.y = 1.0 - _11402.y;
                            _22101 = _20755;
                        }
                        else
                        {
                            _22101 = _11402;
                        }
                        float _11432 = _11402.y;
                        float2 _11433 = float2(_11412.x, _11432);
                        float2 _22102;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20761 = _11433;
                            _20761.y = 1.0 - _11432;
                            _22102 = _20761;
                        }
                        else
                        {
                            _22102 = _11433;
                        }
                        float _11449 = _11412.y;
                        float2 _11450 = float2(_11402.x, _11449);
                        float2 _22103;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20768 = _11450;
                            _20768.y = 1.0 - _11449;
                            _22103 = _20768;
                        }
                        else
                        {
                            _22103 = _11450;
                        }
                        float2 _22104;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20774 = _11412;
                            _20774.y = 1.0 - _11412.y;
                            _22104 = _20774;
                        }
                        else
                        {
                            _22104 = _11412;
                        }
                        _22107 = (((gBackdrop1.sample(gLinear, _22101, level(0.0)) * _11389.x) + (gBackdrop1.sample(gLinear, _22102, level(0.0)) * _11392.x)) * _11389.y) + (((gBackdrop1.sample(gLinear, _22103, level(0.0)) * _11389.x) + (gBackdrop1.sample(gLinear, _22104, level(0.0)) * _11392.x)) * _11392.y);
                    }
                    else
                    {
                        float4 _22108;
                        if (_11204 == 2)
                        {
                            float2 _11550 = (_6535 * _172.gLevel[2].xy) - float2(0.5);
                            float2 _11552 = floor(_11550);
                            float2 _11555 = _11550 - _11552;
                            float2 _11558 = _11555 * _11555;
                            float2 _11561 = _11558 * _11555;
                            float2 _11580 = (((_11561 * 3.0) - (_11558 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _11593 = _11561 * 0.16666667163372039794921875;
                            float2 _11596 = (((((-_11561) + (_11558 * 3.0)) - (_11555 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11580;
                            float2 _11599 = (((((_11561 * (-3.0)) + (_11558 * 3.0)) + (_11555 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11593;
                            float2 _11609 = ((_11552 - float2(0.5)) + (_11580 / _11596)) * _172.gLevel[2].zw;
                            float2 _11619 = ((_11552 + float2(1.5)) + (_11593 / _11599)) * _172.gLevel[2].zw;
                            float2 _22097;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20781 = _11609;
                                _20781.y = 1.0 - _11609.y;
                                _22097 = _20781;
                            }
                            else
                            {
                                _22097 = _11609;
                            }
                            float _11639 = _11609.y;
                            float2 _11640 = float2(_11619.x, _11639);
                            float2 _22098;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20787 = _11640;
                                _20787.y = 1.0 - _11639;
                                _22098 = _20787;
                            }
                            else
                            {
                                _22098 = _11640;
                            }
                            float _11656 = _11619.y;
                            float2 _11657 = float2(_11609.x, _11656);
                            float2 _22099;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20794 = _11657;
                                _20794.y = 1.0 - _11656;
                                _22099 = _20794;
                            }
                            else
                            {
                                _22099 = _11657;
                            }
                            float2 _22100;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20800 = _11619;
                                _20800.y = 1.0 - _11619.y;
                                _22100 = _20800;
                            }
                            else
                            {
                                _22100 = _11619;
                            }
                            _22108 = (((gBackdrop2.sample(gLinear, _22097, level(0.0)) * _11596.x) + (gBackdrop2.sample(gLinear, _22098, level(0.0)) * _11599.x)) * _11596.y) + (((gBackdrop2.sample(gLinear, _22099, level(0.0)) * _11596.x) + (gBackdrop2.sample(gLinear, _22100, level(0.0)) * _11599.x)) * _11599.y);
                        }
                        else
                        {
                            float4 _22109;
                            if (_11204 == 3)
                            {
                                float2 _11757 = (_6535 * _172.gLevel[3].xy) - float2(0.5);
                                float2 _11759 = floor(_11757);
                                float2 _11762 = _11757 - _11759;
                                float2 _11765 = _11762 * _11762;
                                float2 _11768 = _11765 * _11762;
                                float2 _11787 = (((_11768 * 3.0) - (_11765 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _11800 = _11768 * 0.16666667163372039794921875;
                                float2 _11803 = (((((-_11768) + (_11765 * 3.0)) - (_11762 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11787;
                                float2 _11806 = (((((_11768 * (-3.0)) + (_11765 * 3.0)) + (_11762 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11800;
                                float2 _11816 = ((_11759 - float2(0.5)) + (_11787 / _11803)) * _172.gLevel[3].zw;
                                float2 _11826 = ((_11759 + float2(1.5)) + (_11800 / _11806)) * _172.gLevel[3].zw;
                                float2 _22093;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20807 = _11816;
                                    _20807.y = 1.0 - _11816.y;
                                    _22093 = _20807;
                                }
                                else
                                {
                                    _22093 = _11816;
                                }
                                float _11846 = _11816.y;
                                float2 _11847 = float2(_11826.x, _11846);
                                float2 _22094;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20813 = _11847;
                                    _20813.y = 1.0 - _11846;
                                    _22094 = _20813;
                                }
                                else
                                {
                                    _22094 = _11847;
                                }
                                float _11863 = _11826.y;
                                float2 _11864 = float2(_11816.x, _11863);
                                float2 _22095;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20820 = _11864;
                                    _20820.y = 1.0 - _11863;
                                    _22095 = _20820;
                                }
                                else
                                {
                                    _22095 = _11864;
                                }
                                float2 _22096;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20826 = _11826;
                                    _20826.y = 1.0 - _11826.y;
                                    _22096 = _20826;
                                }
                                else
                                {
                                    _22096 = _11826;
                                }
                                _22109 = (((gBackdrop3.sample(gLinear, _22093, level(0.0)) * _11803.x) + (gBackdrop3.sample(gLinear, _22094, level(0.0)) * _11806.x)) * _11803.y) + (((gBackdrop3.sample(gLinear, _22095, level(0.0)) * _11803.x) + (gBackdrop3.sample(gLinear, _22096, level(0.0)) * _11806.x)) * _11806.y);
                            }
                            else
                            {
                                float4 _22110;
                                if (_11204 == 4)
                                {
                                    float2 _11964 = (_6535 * _172.gLevel[4].xy) - float2(0.5);
                                    float2 _11966 = floor(_11964);
                                    float2 _11969 = _11964 - _11966;
                                    float2 _11972 = _11969 * _11969;
                                    float2 _11975 = _11972 * _11969;
                                    float2 _11994 = (((_11975 * 3.0) - (_11972 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _12007 = _11975 * 0.16666667163372039794921875;
                                    float2 _12010 = (((((-_11975) + (_11972 * 3.0)) - (_11969 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11994;
                                    float2 _12013 = (((((_11975 * (-3.0)) + (_11972 * 3.0)) + (_11969 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12007;
                                    float2 _12023 = ((_11966 - float2(0.5)) + (_11994 / _12010)) * _172.gLevel[4].zw;
                                    float2 _12033 = ((_11966 + float2(1.5)) + (_12007 / _12013)) * _172.gLevel[4].zw;
                                    float2 _22089;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20833 = _12023;
                                        _20833.y = 1.0 - _12023.y;
                                        _22089 = _20833;
                                    }
                                    else
                                    {
                                        _22089 = _12023;
                                    }
                                    float _12053 = _12023.y;
                                    float2 _12054 = float2(_12033.x, _12053);
                                    float2 _22090;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20839 = _12054;
                                        _20839.y = 1.0 - _12053;
                                        _22090 = _20839;
                                    }
                                    else
                                    {
                                        _22090 = _12054;
                                    }
                                    float _12070 = _12033.y;
                                    float2 _12071 = float2(_12023.x, _12070);
                                    float2 _22091;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20846 = _12071;
                                        _20846.y = 1.0 - _12070;
                                        _22091 = _20846;
                                    }
                                    else
                                    {
                                        _22091 = _12071;
                                    }
                                    float2 _22092;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20852 = _12033;
                                        _20852.y = 1.0 - _12033.y;
                                        _22092 = _20852;
                                    }
                                    else
                                    {
                                        _22092 = _12033;
                                    }
                                    _22110 = (((gBackdrop4.sample(gLinear, _22089, level(0.0)) * _12010.x) + (gBackdrop4.sample(gLinear, _22090, level(0.0)) * _12013.x)) * _12010.y) + (((gBackdrop4.sample(gLinear, _22091, level(0.0)) * _12010.x) + (gBackdrop4.sample(gLinear, _22092, level(0.0)) * _12013.x)) * _12013.y);
                                }
                                else
                                {
                                    float2 _12171 = (_6535 * _172.gLevel[5].xy) - float2(0.5);
                                    float2 _12173 = floor(_12171);
                                    float2 _12176 = _12171 - _12173;
                                    float2 _12179 = _12176 * _12176;
                                    float2 _12182 = _12179 * _12176;
                                    float2 _12201 = (((_12182 * 3.0) - (_12179 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _12214 = _12182 * 0.16666667163372039794921875;
                                    float2 _12217 = (((((-_12182) + (_12179 * 3.0)) - (_12176 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12201;
                                    float2 _12220 = (((((_12182 * (-3.0)) + (_12179 * 3.0)) + (_12176 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12214;
                                    float2 _12230 = ((_12173 - float2(0.5)) + (_12201 / _12217)) * _172.gLevel[5].zw;
                                    float2 _12240 = ((_12173 + float2(1.5)) + (_12214 / _12220)) * _172.gLevel[5].zw;
                                    float2 _22085;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20859 = _12230;
                                        _20859.y = 1.0 - _12230.y;
                                        _22085 = _20859;
                                    }
                                    else
                                    {
                                        _22085 = _12230;
                                    }
                                    float _12260 = _12230.y;
                                    float2 _12261 = float2(_12240.x, _12260);
                                    float2 _22086;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20865 = _12261;
                                        _20865.y = 1.0 - _12260;
                                        _22086 = _20865;
                                    }
                                    else
                                    {
                                        _22086 = _12261;
                                    }
                                    float _12277 = _12240.y;
                                    float2 _12278 = float2(_12230.x, _12277);
                                    float2 _22087;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20872 = _12278;
                                        _20872.y = 1.0 - _12277;
                                        _22087 = _20872;
                                    }
                                    else
                                    {
                                        _22087 = _12278;
                                    }
                                    float2 _22088;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20878 = _12240;
                                        _20878.y = 1.0 - _12240.y;
                                        _22088 = _20878;
                                    }
                                    else
                                    {
                                        _22088 = _12240;
                                    }
                                    _22110 = (((gBackdrop5.sample(gLinear, _22085, level(0.0)) * _12217.x) + (gBackdrop5.sample(gLinear, _22086, level(0.0)) * _12220.x)) * _12217.y) + (((gBackdrop5.sample(gLinear, _22087, level(0.0)) * _12217.x) + (gBackdrop5.sample(gLinear, _22088, level(0.0)) * _12220.x)) * _12220.y);
                                }
                                _22109 = _22110;
                            }
                            _22108 = _22109;
                        }
                        _22107 = _22108;
                    }
                    _22106 = _22107;
                }
                float3 _22137;
                if ((_11208 > 0.0199999995529651641845703125) && (_11204 < 5))
                {
                    int _11221 = _11204 + 1;
                    float4 _22132;
                    if (_11221 <= 0)
                    {
                        float2 _22131;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20883 = _6535;
                            _20883.y = 1.0 - _6535.y;
                            _22131 = _20883;
                        }
                        else
                        {
                            _22131 = _6535;
                        }
                        _22132 = gBackdrop0.sample(gLinear, _22131, level(0.0));
                    }
                    else
                    {
                        float4 _22133;
                        if (_11221 == 1)
                        {
                            float2 _12467 = (_6535 * _172.gLevel[1].xy) - float2(0.5);
                            float2 _12469 = floor(_12467);
                            float2 _12472 = _12467 - _12469;
                            float2 _12475 = _12472 * _12472;
                            float2 _12478 = _12475 * _12472;
                            float2 _12497 = (((_12478 * 3.0) - (_12475 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _12510 = _12478 * 0.16666667163372039794921875;
                            float2 _12513 = (((((-_12478) + (_12475 * 3.0)) - (_12472 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12497;
                            float2 _12516 = (((((_12478 * (-3.0)) + (_12475 * 3.0)) + (_12472 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12510;
                            float2 _12526 = ((_12469 - float2(0.5)) + (_12497 / _12513)) * _172.gLevel[1].zw;
                            float2 _12536 = ((_12469 + float2(1.5)) + (_12510 / _12516)) * _172.gLevel[1].zw;
                            float2 _22127;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20888 = _12526;
                                _20888.y = 1.0 - _12526.y;
                                _22127 = _20888;
                            }
                            else
                            {
                                _22127 = _12526;
                            }
                            float _12556 = _12526.y;
                            float2 _12557 = float2(_12536.x, _12556);
                            float2 _22128;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20894 = _12557;
                                _20894.y = 1.0 - _12556;
                                _22128 = _20894;
                            }
                            else
                            {
                                _22128 = _12557;
                            }
                            float _12573 = _12536.y;
                            float2 _12574 = float2(_12526.x, _12573);
                            float2 _22129;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20901 = _12574;
                                _20901.y = 1.0 - _12573;
                                _22129 = _20901;
                            }
                            else
                            {
                                _22129 = _12574;
                            }
                            float2 _22130;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20907 = _12536;
                                _20907.y = 1.0 - _12536.y;
                                _22130 = _20907;
                            }
                            else
                            {
                                _22130 = _12536;
                            }
                            _22133 = (((gBackdrop1.sample(gLinear, _22127, level(0.0)) * _12513.x) + (gBackdrop1.sample(gLinear, _22128, level(0.0)) * _12516.x)) * _12513.y) + (((gBackdrop1.sample(gLinear, _22129, level(0.0)) * _12513.x) + (gBackdrop1.sample(gLinear, _22130, level(0.0)) * _12516.x)) * _12516.y);
                        }
                        else
                        {
                            float4 _22134;
                            if (_11221 == 2)
                            {
                                float2 _12674 = (_6535 * _172.gLevel[2].xy) - float2(0.5);
                                float2 _12676 = floor(_12674);
                                float2 _12679 = _12674 - _12676;
                                float2 _12682 = _12679 * _12679;
                                float2 _12685 = _12682 * _12679;
                                float2 _12704 = (((_12685 * 3.0) - (_12682 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _12717 = _12685 * 0.16666667163372039794921875;
                                float2 _12720 = (((((-_12685) + (_12682 * 3.0)) - (_12679 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12704;
                                float2 _12723 = (((((_12685 * (-3.0)) + (_12682 * 3.0)) + (_12679 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12717;
                                float2 _12733 = ((_12676 - float2(0.5)) + (_12704 / _12720)) * _172.gLevel[2].zw;
                                float2 _12743 = ((_12676 + float2(1.5)) + (_12717 / _12723)) * _172.gLevel[2].zw;
                                float2 _22123;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20914 = _12733;
                                    _20914.y = 1.0 - _12733.y;
                                    _22123 = _20914;
                                }
                                else
                                {
                                    _22123 = _12733;
                                }
                                float _12763 = _12733.y;
                                float2 _12764 = float2(_12743.x, _12763);
                                float2 _22124;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20920 = _12764;
                                    _20920.y = 1.0 - _12763;
                                    _22124 = _20920;
                                }
                                else
                                {
                                    _22124 = _12764;
                                }
                                float _12780 = _12743.y;
                                float2 _12781 = float2(_12733.x, _12780);
                                float2 _22125;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20927 = _12781;
                                    _20927.y = 1.0 - _12780;
                                    _22125 = _20927;
                                }
                                else
                                {
                                    _22125 = _12781;
                                }
                                float2 _22126;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20933 = _12743;
                                    _20933.y = 1.0 - _12743.y;
                                    _22126 = _20933;
                                }
                                else
                                {
                                    _22126 = _12743;
                                }
                                _22134 = (((gBackdrop2.sample(gLinear, _22123, level(0.0)) * _12720.x) + (gBackdrop2.sample(gLinear, _22124, level(0.0)) * _12723.x)) * _12720.y) + (((gBackdrop2.sample(gLinear, _22125, level(0.0)) * _12720.x) + (gBackdrop2.sample(gLinear, _22126, level(0.0)) * _12723.x)) * _12723.y);
                            }
                            else
                            {
                                float4 _22135;
                                if (_11221 == 3)
                                {
                                    float2 _12881 = (_6535 * _172.gLevel[3].xy) - float2(0.5);
                                    float2 _12883 = floor(_12881);
                                    float2 _12886 = _12881 - _12883;
                                    float2 _12889 = _12886 * _12886;
                                    float2 _12892 = _12889 * _12886;
                                    float2 _12911 = (((_12892 * 3.0) - (_12889 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _12924 = _12892 * 0.16666667163372039794921875;
                                    float2 _12927 = (((((-_12892) + (_12889 * 3.0)) - (_12886 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12911;
                                    float2 _12930 = (((((_12892 * (-3.0)) + (_12889 * 3.0)) + (_12886 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12924;
                                    float2 _12940 = ((_12883 - float2(0.5)) + (_12911 / _12927)) * _172.gLevel[3].zw;
                                    float2 _12950 = ((_12883 + float2(1.5)) + (_12924 / _12930)) * _172.gLevel[3].zw;
                                    float2 _22119;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20940 = _12940;
                                        _20940.y = 1.0 - _12940.y;
                                        _22119 = _20940;
                                    }
                                    else
                                    {
                                        _22119 = _12940;
                                    }
                                    float _12970 = _12940.y;
                                    float2 _12971 = float2(_12950.x, _12970);
                                    float2 _22120;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20946 = _12971;
                                        _20946.y = 1.0 - _12970;
                                        _22120 = _20946;
                                    }
                                    else
                                    {
                                        _22120 = _12971;
                                    }
                                    float _12987 = _12950.y;
                                    float2 _12988 = float2(_12940.x, _12987);
                                    float2 _22121;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20953 = _12988;
                                        _20953.y = 1.0 - _12987;
                                        _22121 = _20953;
                                    }
                                    else
                                    {
                                        _22121 = _12988;
                                    }
                                    float2 _22122;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20959 = _12950;
                                        _20959.y = 1.0 - _12950.y;
                                        _22122 = _20959;
                                    }
                                    else
                                    {
                                        _22122 = _12950;
                                    }
                                    _22135 = (((gBackdrop3.sample(gLinear, _22119, level(0.0)) * _12927.x) + (gBackdrop3.sample(gLinear, _22120, level(0.0)) * _12930.x)) * _12927.y) + (((gBackdrop3.sample(gLinear, _22121, level(0.0)) * _12927.x) + (gBackdrop3.sample(gLinear, _22122, level(0.0)) * _12930.x)) * _12930.y);
                                }
                                else
                                {
                                    float4 _22136;
                                    if (_11221 == 4)
                                    {
                                        float2 _13088 = (_6535 * _172.gLevel[4].xy) - float2(0.5);
                                        float2 _13090 = floor(_13088);
                                        float2 _13093 = _13088 - _13090;
                                        float2 _13096 = _13093 * _13093;
                                        float2 _13099 = _13096 * _13093;
                                        float2 _13118 = (((_13099 * 3.0) - (_13096 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _13131 = _13099 * 0.16666667163372039794921875;
                                        float2 _13134 = (((((-_13099) + (_13096 * 3.0)) - (_13093 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13118;
                                        float2 _13137 = (((((_13099 * (-3.0)) + (_13096 * 3.0)) + (_13093 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13131;
                                        float2 _13147 = ((_13090 - float2(0.5)) + (_13118 / _13134)) * _172.gLevel[4].zw;
                                        float2 _13157 = ((_13090 + float2(1.5)) + (_13131 / _13137)) * _172.gLevel[4].zw;
                                        float2 _22115;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20966 = _13147;
                                            _20966.y = 1.0 - _13147.y;
                                            _22115 = _20966;
                                        }
                                        else
                                        {
                                            _22115 = _13147;
                                        }
                                        float _13177 = _13147.y;
                                        float2 _13178 = float2(_13157.x, _13177);
                                        float2 _22116;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20972 = _13178;
                                            _20972.y = 1.0 - _13177;
                                            _22116 = _20972;
                                        }
                                        else
                                        {
                                            _22116 = _13178;
                                        }
                                        float _13194 = _13157.y;
                                        float2 _13195 = float2(_13147.x, _13194);
                                        float2 _22117;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20979 = _13195;
                                            _20979.y = 1.0 - _13194;
                                            _22117 = _20979;
                                        }
                                        else
                                        {
                                            _22117 = _13195;
                                        }
                                        float2 _22118;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20985 = _13157;
                                            _20985.y = 1.0 - _13157.y;
                                            _22118 = _20985;
                                        }
                                        else
                                        {
                                            _22118 = _13157;
                                        }
                                        _22136 = (((gBackdrop4.sample(gLinear, _22115, level(0.0)) * _13134.x) + (gBackdrop4.sample(gLinear, _22116, level(0.0)) * _13137.x)) * _13134.y) + (((gBackdrop4.sample(gLinear, _22117, level(0.0)) * _13134.x) + (gBackdrop4.sample(gLinear, _22118, level(0.0)) * _13137.x)) * _13137.y);
                                    }
                                    else
                                    {
                                        float2 _13295 = (_6535 * _172.gLevel[5].xy) - float2(0.5);
                                        float2 _13297 = floor(_13295);
                                        float2 _13300 = _13295 - _13297;
                                        float2 _13303 = _13300 * _13300;
                                        float2 _13306 = _13303 * _13300;
                                        float2 _13325 = (((_13306 * 3.0) - (_13303 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _13338 = _13306 * 0.16666667163372039794921875;
                                        float2 _13341 = (((((-_13306) + (_13303 * 3.0)) - (_13300 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13325;
                                        float2 _13344 = (((((_13306 * (-3.0)) + (_13303 * 3.0)) + (_13300 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13338;
                                        float2 _13354 = ((_13297 - float2(0.5)) + (_13325 / _13341)) * _172.gLevel[5].zw;
                                        float2 _13364 = ((_13297 + float2(1.5)) + (_13338 / _13344)) * _172.gLevel[5].zw;
                                        float2 _22111;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20992 = _13354;
                                            _20992.y = 1.0 - _13354.y;
                                            _22111 = _20992;
                                        }
                                        else
                                        {
                                            _22111 = _13354;
                                        }
                                        float _13384 = _13354.y;
                                        float2 _13385 = float2(_13364.x, _13384);
                                        float2 _22112;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20998 = _13385;
                                            _20998.y = 1.0 - _13384;
                                            _22112 = _20998;
                                        }
                                        else
                                        {
                                            _22112 = _13385;
                                        }
                                        float _13401 = _13364.y;
                                        float2 _13402 = float2(_13354.x, _13401);
                                        float2 _22113;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21005 = _13402;
                                            _21005.y = 1.0 - _13401;
                                            _22113 = _21005;
                                        }
                                        else
                                        {
                                            _22113 = _13402;
                                        }
                                        float2 _22114;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21011 = _13364;
                                            _21011.y = 1.0 - _13364.y;
                                            _22114 = _21011;
                                        }
                                        else
                                        {
                                            _22114 = _13364;
                                        }
                                        _22136 = (((gBackdrop5.sample(gLinear, _22111, level(0.0)) * _13341.x) + (gBackdrop5.sample(gLinear, _22112, level(0.0)) * _13344.x)) * _13341.y) + (((gBackdrop5.sample(gLinear, _22113, level(0.0)) * _13341.x) + (gBackdrop5.sample(gLinear, _22114, level(0.0)) * _13344.x)) * _13344.y);
                                    }
                                    _22135 = _22136;
                                }
                                _22134 = _22135;
                            }
                            _22133 = _22134;
                        }
                        _22132 = _22133;
                    }
                    _22137 = mix(_22106.xyz, _22132.xyz, float3(_11208));
                }
                else
                {
                    _22137 = _22106.xyz;
                }
                float2 _6546 = (_6484 + _6496) + (_6499 * _6521);
                float _13492 = fast::clamp(log2(fast::max(_6398, 1.0)) - 1.0, 0.0, 5.0);
                int _13495 = int(floor(_13492));
                float _13499 = _13492 - float(_13495);
                float4 _22212;
                if (_13495 <= 0)
                {
                    float2 _22211;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _21018 = _6546;
                        _21018.y = 1.0 - _6546.y;
                        _22211 = _21018;
                    }
                    else
                    {
                        _22211 = _6546;
                    }
                    _22212 = gBackdrop0.sample(gLinear, _22211, level(0.0));
                }
                else
                {
                    float4 _22213;
                    if (_13495 == 1)
                    {
                        float2 _13634 = (_6546 * _172.gLevel[1].xy) - float2(0.5);
                        float2 _13636 = floor(_13634);
                        float2 _13639 = _13634 - _13636;
                        float2 _13642 = _13639 * _13639;
                        float2 _13645 = _13642 * _13639;
                        float2 _13664 = (((_13645 * 3.0) - (_13642 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _13677 = _13645 * 0.16666667163372039794921875;
                        float2 _13680 = (((((-_13645) + (_13642 * 3.0)) - (_13639 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13664;
                        float2 _13683 = (((((_13645 * (-3.0)) + (_13642 * 3.0)) + (_13639 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13677;
                        float2 _13693 = ((_13636 - float2(0.5)) + (_13664 / _13680)) * _172.gLevel[1].zw;
                        float2 _13703 = ((_13636 + float2(1.5)) + (_13677 / _13683)) * _172.gLevel[1].zw;
                        float2 _22207;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21023 = _13693;
                            _21023.y = 1.0 - _13693.y;
                            _22207 = _21023;
                        }
                        else
                        {
                            _22207 = _13693;
                        }
                        float _13723 = _13693.y;
                        float2 _13724 = float2(_13703.x, _13723);
                        float2 _22208;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21029 = _13724;
                            _21029.y = 1.0 - _13723;
                            _22208 = _21029;
                        }
                        else
                        {
                            _22208 = _13724;
                        }
                        float _13740 = _13703.y;
                        float2 _13741 = float2(_13693.x, _13740);
                        float2 _22209;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21036 = _13741;
                            _21036.y = 1.0 - _13740;
                            _22209 = _21036;
                        }
                        else
                        {
                            _22209 = _13741;
                        }
                        float2 _22210;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21042 = _13703;
                            _21042.y = 1.0 - _13703.y;
                            _22210 = _21042;
                        }
                        else
                        {
                            _22210 = _13703;
                        }
                        _22213 = (((gBackdrop1.sample(gLinear, _22207, level(0.0)) * _13680.x) + (gBackdrop1.sample(gLinear, _22208, level(0.0)) * _13683.x)) * _13680.y) + (((gBackdrop1.sample(gLinear, _22209, level(0.0)) * _13680.x) + (gBackdrop1.sample(gLinear, _22210, level(0.0)) * _13683.x)) * _13683.y);
                    }
                    else
                    {
                        float4 _22214;
                        if (_13495 == 2)
                        {
                            float2 _13841 = (_6546 * _172.gLevel[2].xy) - float2(0.5);
                            float2 _13843 = floor(_13841);
                            float2 _13846 = _13841 - _13843;
                            float2 _13849 = _13846 * _13846;
                            float2 _13852 = _13849 * _13846;
                            float2 _13871 = (((_13852 * 3.0) - (_13849 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _13884 = _13852 * 0.16666667163372039794921875;
                            float2 _13887 = (((((-_13852) + (_13849 * 3.0)) - (_13846 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13871;
                            float2 _13890 = (((((_13852 * (-3.0)) + (_13849 * 3.0)) + (_13846 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13884;
                            float2 _13900 = ((_13843 - float2(0.5)) + (_13871 / _13887)) * _172.gLevel[2].zw;
                            float2 _13910 = ((_13843 + float2(1.5)) + (_13884 / _13890)) * _172.gLevel[2].zw;
                            float2 _22203;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21049 = _13900;
                                _21049.y = 1.0 - _13900.y;
                                _22203 = _21049;
                            }
                            else
                            {
                                _22203 = _13900;
                            }
                            float _13930 = _13900.y;
                            float2 _13931 = float2(_13910.x, _13930);
                            float2 _22204;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21055 = _13931;
                                _21055.y = 1.0 - _13930;
                                _22204 = _21055;
                            }
                            else
                            {
                                _22204 = _13931;
                            }
                            float _13947 = _13910.y;
                            float2 _13948 = float2(_13900.x, _13947);
                            float2 _22205;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21062 = _13948;
                                _21062.y = 1.0 - _13947;
                                _22205 = _21062;
                            }
                            else
                            {
                                _22205 = _13948;
                            }
                            float2 _22206;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21068 = _13910;
                                _21068.y = 1.0 - _13910.y;
                                _22206 = _21068;
                            }
                            else
                            {
                                _22206 = _13910;
                            }
                            _22214 = (((gBackdrop2.sample(gLinear, _22203, level(0.0)) * _13887.x) + (gBackdrop2.sample(gLinear, _22204, level(0.0)) * _13890.x)) * _13887.y) + (((gBackdrop2.sample(gLinear, _22205, level(0.0)) * _13887.x) + (gBackdrop2.sample(gLinear, _22206, level(0.0)) * _13890.x)) * _13890.y);
                        }
                        else
                        {
                            float4 _22215;
                            if (_13495 == 3)
                            {
                                float2 _14048 = (_6546 * _172.gLevel[3].xy) - float2(0.5);
                                float2 _14050 = floor(_14048);
                                float2 _14053 = _14048 - _14050;
                                float2 _14056 = _14053 * _14053;
                                float2 _14059 = _14056 * _14053;
                                float2 _14078 = (((_14059 * 3.0) - (_14056 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _14091 = _14059 * 0.16666667163372039794921875;
                                float2 _14094 = (((((-_14059) + (_14056 * 3.0)) - (_14053 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14078;
                                float2 _14097 = (((((_14059 * (-3.0)) + (_14056 * 3.0)) + (_14053 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14091;
                                float2 _14107 = ((_14050 - float2(0.5)) + (_14078 / _14094)) * _172.gLevel[3].zw;
                                float2 _14117 = ((_14050 + float2(1.5)) + (_14091 / _14097)) * _172.gLevel[3].zw;
                                float2 _22199;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21075 = _14107;
                                    _21075.y = 1.0 - _14107.y;
                                    _22199 = _21075;
                                }
                                else
                                {
                                    _22199 = _14107;
                                }
                                float _14137 = _14107.y;
                                float2 _14138 = float2(_14117.x, _14137);
                                float2 _22200;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21081 = _14138;
                                    _21081.y = 1.0 - _14137;
                                    _22200 = _21081;
                                }
                                else
                                {
                                    _22200 = _14138;
                                }
                                float _14154 = _14117.y;
                                float2 _14155 = float2(_14107.x, _14154);
                                float2 _22201;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21088 = _14155;
                                    _21088.y = 1.0 - _14154;
                                    _22201 = _21088;
                                }
                                else
                                {
                                    _22201 = _14155;
                                }
                                float2 _22202;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21094 = _14117;
                                    _21094.y = 1.0 - _14117.y;
                                    _22202 = _21094;
                                }
                                else
                                {
                                    _22202 = _14117;
                                }
                                _22215 = (((gBackdrop3.sample(gLinear, _22199, level(0.0)) * _14094.x) + (gBackdrop3.sample(gLinear, _22200, level(0.0)) * _14097.x)) * _14094.y) + (((gBackdrop3.sample(gLinear, _22201, level(0.0)) * _14094.x) + (gBackdrop3.sample(gLinear, _22202, level(0.0)) * _14097.x)) * _14097.y);
                            }
                            else
                            {
                                float4 _22216;
                                if (_13495 == 4)
                                {
                                    float2 _14255 = (_6546 * _172.gLevel[4].xy) - float2(0.5);
                                    float2 _14257 = floor(_14255);
                                    float2 _14260 = _14255 - _14257;
                                    float2 _14263 = _14260 * _14260;
                                    float2 _14266 = _14263 * _14260;
                                    float2 _14285 = (((_14266 * 3.0) - (_14263 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _14298 = _14266 * 0.16666667163372039794921875;
                                    float2 _14301 = (((((-_14266) + (_14263 * 3.0)) - (_14260 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14285;
                                    float2 _14304 = (((((_14266 * (-3.0)) + (_14263 * 3.0)) + (_14260 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14298;
                                    float2 _14314 = ((_14257 - float2(0.5)) + (_14285 / _14301)) * _172.gLevel[4].zw;
                                    float2 _14324 = ((_14257 + float2(1.5)) + (_14298 / _14304)) * _172.gLevel[4].zw;
                                    float2 _22195;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21101 = _14314;
                                        _21101.y = 1.0 - _14314.y;
                                        _22195 = _21101;
                                    }
                                    else
                                    {
                                        _22195 = _14314;
                                    }
                                    float _14344 = _14314.y;
                                    float2 _14345 = float2(_14324.x, _14344);
                                    float2 _22196;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21107 = _14345;
                                        _21107.y = 1.0 - _14344;
                                        _22196 = _21107;
                                    }
                                    else
                                    {
                                        _22196 = _14345;
                                    }
                                    float _14361 = _14324.y;
                                    float2 _14362 = float2(_14314.x, _14361);
                                    float2 _22197;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21114 = _14362;
                                        _21114.y = 1.0 - _14361;
                                        _22197 = _21114;
                                    }
                                    else
                                    {
                                        _22197 = _14362;
                                    }
                                    float2 _22198;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21120 = _14324;
                                        _21120.y = 1.0 - _14324.y;
                                        _22198 = _21120;
                                    }
                                    else
                                    {
                                        _22198 = _14324;
                                    }
                                    _22216 = (((gBackdrop4.sample(gLinear, _22195, level(0.0)) * _14301.x) + (gBackdrop4.sample(gLinear, _22196, level(0.0)) * _14304.x)) * _14301.y) + (((gBackdrop4.sample(gLinear, _22197, level(0.0)) * _14301.x) + (gBackdrop4.sample(gLinear, _22198, level(0.0)) * _14304.x)) * _14304.y);
                                }
                                else
                                {
                                    float2 _14462 = (_6546 * _172.gLevel[5].xy) - float2(0.5);
                                    float2 _14464 = floor(_14462);
                                    float2 _14467 = _14462 - _14464;
                                    float2 _14470 = _14467 * _14467;
                                    float2 _14473 = _14470 * _14467;
                                    float2 _14492 = (((_14473 * 3.0) - (_14470 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _14505 = _14473 * 0.16666667163372039794921875;
                                    float2 _14508 = (((((-_14473) + (_14470 * 3.0)) - (_14467 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14492;
                                    float2 _14511 = (((((_14473 * (-3.0)) + (_14470 * 3.0)) + (_14467 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14505;
                                    float2 _14521 = ((_14464 - float2(0.5)) + (_14492 / _14508)) * _172.gLevel[5].zw;
                                    float2 _14531 = ((_14464 + float2(1.5)) + (_14505 / _14511)) * _172.gLevel[5].zw;
                                    float2 _22191;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21127 = _14521;
                                        _21127.y = 1.0 - _14521.y;
                                        _22191 = _21127;
                                    }
                                    else
                                    {
                                        _22191 = _14521;
                                    }
                                    float _14551 = _14521.y;
                                    float2 _14552 = float2(_14531.x, _14551);
                                    float2 _22192;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21133 = _14552;
                                        _21133.y = 1.0 - _14551;
                                        _22192 = _21133;
                                    }
                                    else
                                    {
                                        _22192 = _14552;
                                    }
                                    float _14568 = _14531.y;
                                    float2 _14569 = float2(_14521.x, _14568);
                                    float2 _22193;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21140 = _14569;
                                        _21140.y = 1.0 - _14568;
                                        _22193 = _21140;
                                    }
                                    else
                                    {
                                        _22193 = _14569;
                                    }
                                    float2 _22194;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21146 = _14531;
                                        _21146.y = 1.0 - _14531.y;
                                        _22194 = _21146;
                                    }
                                    else
                                    {
                                        _22194 = _14531;
                                    }
                                    _22216 = (((gBackdrop5.sample(gLinear, _22191, level(0.0)) * _14508.x) + (gBackdrop5.sample(gLinear, _22192, level(0.0)) * _14511.x)) * _14508.y) + (((gBackdrop5.sample(gLinear, _22193, level(0.0)) * _14508.x) + (gBackdrop5.sample(gLinear, _22194, level(0.0)) * _14511.x)) * _14511.y);
                                }
                                _22215 = _22216;
                            }
                            _22214 = _22215;
                        }
                        _22213 = _22214;
                    }
                    _22212 = _22213;
                }
                float3 _22243;
                if ((_13499 > 0.0199999995529651641845703125) && (_13495 < 5))
                {
                    int _13512 = _13495 + 1;
                    float4 _22238;
                    if (_13512 <= 0)
                    {
                        float2 _22237;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21151 = _6546;
                            _21151.y = 1.0 - _6546.y;
                            _22237 = _21151;
                        }
                        else
                        {
                            _22237 = _6546;
                        }
                        _22238 = gBackdrop0.sample(gLinear, _22237, level(0.0));
                    }
                    else
                    {
                        float4 _22239;
                        if (_13512 == 1)
                        {
                            float2 _14758 = (_6546 * _172.gLevel[1].xy) - float2(0.5);
                            float2 _14760 = floor(_14758);
                            float2 _14763 = _14758 - _14760;
                            float2 _14766 = _14763 * _14763;
                            float2 _14769 = _14766 * _14763;
                            float2 _14788 = (((_14769 * 3.0) - (_14766 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _14801 = _14769 * 0.16666667163372039794921875;
                            float2 _14804 = (((((-_14769) + (_14766 * 3.0)) - (_14763 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14788;
                            float2 _14807 = (((((_14769 * (-3.0)) + (_14766 * 3.0)) + (_14763 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14801;
                            float2 _14817 = ((_14760 - float2(0.5)) + (_14788 / _14804)) * _172.gLevel[1].zw;
                            float2 _14827 = ((_14760 + float2(1.5)) + (_14801 / _14807)) * _172.gLevel[1].zw;
                            float2 _22233;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21156 = _14817;
                                _21156.y = 1.0 - _14817.y;
                                _22233 = _21156;
                            }
                            else
                            {
                                _22233 = _14817;
                            }
                            float _14847 = _14817.y;
                            float2 _14848 = float2(_14827.x, _14847);
                            float2 _22234;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21162 = _14848;
                                _21162.y = 1.0 - _14847;
                                _22234 = _21162;
                            }
                            else
                            {
                                _22234 = _14848;
                            }
                            float _14864 = _14827.y;
                            float2 _14865 = float2(_14817.x, _14864);
                            float2 _22235;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21169 = _14865;
                                _21169.y = 1.0 - _14864;
                                _22235 = _21169;
                            }
                            else
                            {
                                _22235 = _14865;
                            }
                            float2 _22236;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21175 = _14827;
                                _21175.y = 1.0 - _14827.y;
                                _22236 = _21175;
                            }
                            else
                            {
                                _22236 = _14827;
                            }
                            _22239 = (((gBackdrop1.sample(gLinear, _22233, level(0.0)) * _14804.x) + (gBackdrop1.sample(gLinear, _22234, level(0.0)) * _14807.x)) * _14804.y) + (((gBackdrop1.sample(gLinear, _22235, level(0.0)) * _14804.x) + (gBackdrop1.sample(gLinear, _22236, level(0.0)) * _14807.x)) * _14807.y);
                        }
                        else
                        {
                            float4 _22240;
                            if (_13512 == 2)
                            {
                                float2 _14965 = (_6546 * _172.gLevel[2].xy) - float2(0.5);
                                float2 _14967 = floor(_14965);
                                float2 _14970 = _14965 - _14967;
                                float2 _14973 = _14970 * _14970;
                                float2 _14976 = _14973 * _14970;
                                float2 _14995 = (((_14976 * 3.0) - (_14973 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _15008 = _14976 * 0.16666667163372039794921875;
                                float2 _15011 = (((((-_14976) + (_14973 * 3.0)) - (_14970 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14995;
                                float2 _15014 = (((((_14976 * (-3.0)) + (_14973 * 3.0)) + (_14970 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15008;
                                float2 _15024 = ((_14967 - float2(0.5)) + (_14995 / _15011)) * _172.gLevel[2].zw;
                                float2 _15034 = ((_14967 + float2(1.5)) + (_15008 / _15014)) * _172.gLevel[2].zw;
                                float2 _22229;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21182 = _15024;
                                    _21182.y = 1.0 - _15024.y;
                                    _22229 = _21182;
                                }
                                else
                                {
                                    _22229 = _15024;
                                }
                                float _15054 = _15024.y;
                                float2 _15055 = float2(_15034.x, _15054);
                                float2 _22230;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21188 = _15055;
                                    _21188.y = 1.0 - _15054;
                                    _22230 = _21188;
                                }
                                else
                                {
                                    _22230 = _15055;
                                }
                                float _15071 = _15034.y;
                                float2 _15072 = float2(_15024.x, _15071);
                                float2 _22231;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21195 = _15072;
                                    _21195.y = 1.0 - _15071;
                                    _22231 = _21195;
                                }
                                else
                                {
                                    _22231 = _15072;
                                }
                                float2 _22232;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21201 = _15034;
                                    _21201.y = 1.0 - _15034.y;
                                    _22232 = _21201;
                                }
                                else
                                {
                                    _22232 = _15034;
                                }
                                _22240 = (((gBackdrop2.sample(gLinear, _22229, level(0.0)) * _15011.x) + (gBackdrop2.sample(gLinear, _22230, level(0.0)) * _15014.x)) * _15011.y) + (((gBackdrop2.sample(gLinear, _22231, level(0.0)) * _15011.x) + (gBackdrop2.sample(gLinear, _22232, level(0.0)) * _15014.x)) * _15014.y);
                            }
                            else
                            {
                                float4 _22241;
                                if (_13512 == 3)
                                {
                                    float2 _15172 = (_6546 * _172.gLevel[3].xy) - float2(0.5);
                                    float2 _15174 = floor(_15172);
                                    float2 _15177 = _15172 - _15174;
                                    float2 _15180 = _15177 * _15177;
                                    float2 _15183 = _15180 * _15177;
                                    float2 _15202 = (((_15183 * 3.0) - (_15180 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _15215 = _15183 * 0.16666667163372039794921875;
                                    float2 _15218 = (((((-_15183) + (_15180 * 3.0)) - (_15177 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15202;
                                    float2 _15221 = (((((_15183 * (-3.0)) + (_15180 * 3.0)) + (_15177 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15215;
                                    float2 _15231 = ((_15174 - float2(0.5)) + (_15202 / _15218)) * _172.gLevel[3].zw;
                                    float2 _15241 = ((_15174 + float2(1.5)) + (_15215 / _15221)) * _172.gLevel[3].zw;
                                    float2 _22225;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21208 = _15231;
                                        _21208.y = 1.0 - _15231.y;
                                        _22225 = _21208;
                                    }
                                    else
                                    {
                                        _22225 = _15231;
                                    }
                                    float _15261 = _15231.y;
                                    float2 _15262 = float2(_15241.x, _15261);
                                    float2 _22226;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21214 = _15262;
                                        _21214.y = 1.0 - _15261;
                                        _22226 = _21214;
                                    }
                                    else
                                    {
                                        _22226 = _15262;
                                    }
                                    float _15278 = _15241.y;
                                    float2 _15279 = float2(_15231.x, _15278);
                                    float2 _22227;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21221 = _15279;
                                        _21221.y = 1.0 - _15278;
                                        _22227 = _21221;
                                    }
                                    else
                                    {
                                        _22227 = _15279;
                                    }
                                    float2 _22228;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21227 = _15241;
                                        _21227.y = 1.0 - _15241.y;
                                        _22228 = _21227;
                                    }
                                    else
                                    {
                                        _22228 = _15241;
                                    }
                                    _22241 = (((gBackdrop3.sample(gLinear, _22225, level(0.0)) * _15218.x) + (gBackdrop3.sample(gLinear, _22226, level(0.0)) * _15221.x)) * _15218.y) + (((gBackdrop3.sample(gLinear, _22227, level(0.0)) * _15218.x) + (gBackdrop3.sample(gLinear, _22228, level(0.0)) * _15221.x)) * _15221.y);
                                }
                                else
                                {
                                    float4 _22242;
                                    if (_13512 == 4)
                                    {
                                        float2 _15379 = (_6546 * _172.gLevel[4].xy) - float2(0.5);
                                        float2 _15381 = floor(_15379);
                                        float2 _15384 = _15379 - _15381;
                                        float2 _15387 = _15384 * _15384;
                                        float2 _15390 = _15387 * _15384;
                                        float2 _15409 = (((_15390 * 3.0) - (_15387 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _15422 = _15390 * 0.16666667163372039794921875;
                                        float2 _15425 = (((((-_15390) + (_15387 * 3.0)) - (_15384 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15409;
                                        float2 _15428 = (((((_15390 * (-3.0)) + (_15387 * 3.0)) + (_15384 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15422;
                                        float2 _15438 = ((_15381 - float2(0.5)) + (_15409 / _15425)) * _172.gLevel[4].zw;
                                        float2 _15448 = ((_15381 + float2(1.5)) + (_15422 / _15428)) * _172.gLevel[4].zw;
                                        float2 _22221;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21234 = _15438;
                                            _21234.y = 1.0 - _15438.y;
                                            _22221 = _21234;
                                        }
                                        else
                                        {
                                            _22221 = _15438;
                                        }
                                        float _15468 = _15438.y;
                                        float2 _15469 = float2(_15448.x, _15468);
                                        float2 _22222;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21240 = _15469;
                                            _21240.y = 1.0 - _15468;
                                            _22222 = _21240;
                                        }
                                        else
                                        {
                                            _22222 = _15469;
                                        }
                                        float _15485 = _15448.y;
                                        float2 _15486 = float2(_15438.x, _15485);
                                        float2 _22223;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21247 = _15486;
                                            _21247.y = 1.0 - _15485;
                                            _22223 = _21247;
                                        }
                                        else
                                        {
                                            _22223 = _15486;
                                        }
                                        float2 _22224;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21253 = _15448;
                                            _21253.y = 1.0 - _15448.y;
                                            _22224 = _21253;
                                        }
                                        else
                                        {
                                            _22224 = _15448;
                                        }
                                        _22242 = (((gBackdrop4.sample(gLinear, _22221, level(0.0)) * _15425.x) + (gBackdrop4.sample(gLinear, _22222, level(0.0)) * _15428.x)) * _15425.y) + (((gBackdrop4.sample(gLinear, _22223, level(0.0)) * _15425.x) + (gBackdrop4.sample(gLinear, _22224, level(0.0)) * _15428.x)) * _15428.y);
                                    }
                                    else
                                    {
                                        float2 _15586 = (_6546 * _172.gLevel[5].xy) - float2(0.5);
                                        float2 _15588 = floor(_15586);
                                        float2 _15591 = _15586 - _15588;
                                        float2 _15594 = _15591 * _15591;
                                        float2 _15597 = _15594 * _15591;
                                        float2 _15616 = (((_15597 * 3.0) - (_15594 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _15629 = _15597 * 0.16666667163372039794921875;
                                        float2 _15632 = (((((-_15597) + (_15594 * 3.0)) - (_15591 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15616;
                                        float2 _15635 = (((((_15597 * (-3.0)) + (_15594 * 3.0)) + (_15591 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15629;
                                        float2 _15645 = ((_15588 - float2(0.5)) + (_15616 / _15632)) * _172.gLevel[5].zw;
                                        float2 _15655 = ((_15588 + float2(1.5)) + (_15629 / _15635)) * _172.gLevel[5].zw;
                                        float2 _22217;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21260 = _15645;
                                            _21260.y = 1.0 - _15645.y;
                                            _22217 = _21260;
                                        }
                                        else
                                        {
                                            _22217 = _15645;
                                        }
                                        float _15675 = _15645.y;
                                        float2 _15676 = float2(_15655.x, _15675);
                                        float2 _22218;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21266 = _15676;
                                            _21266.y = 1.0 - _15675;
                                            _22218 = _21266;
                                        }
                                        else
                                        {
                                            _22218 = _15676;
                                        }
                                        float _15692 = _15655.y;
                                        float2 _15693 = float2(_15645.x, _15692);
                                        float2 _22219;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21273 = _15693;
                                            _21273.y = 1.0 - _15692;
                                            _22219 = _21273;
                                        }
                                        else
                                        {
                                            _22219 = _15693;
                                        }
                                        float2 _22220;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21279 = _15655;
                                            _21279.y = 1.0 - _15655.y;
                                            _22220 = _21279;
                                        }
                                        else
                                        {
                                            _22220 = _15655;
                                        }
                                        _22242 = (((gBackdrop5.sample(gLinear, _22217, level(0.0)) * _15632.x) + (gBackdrop5.sample(gLinear, _22218, level(0.0)) * _15635.x)) * _15632.y) + (((gBackdrop5.sample(gLinear, _22219, level(0.0)) * _15632.x) + (gBackdrop5.sample(gLinear, _22220, level(0.0)) * _15635.x)) * _15635.y);
                                    }
                                    _22241 = _22242;
                                }
                                _22240 = _22241;
                            }
                            _22239 = _22240;
                        }
                        _22238 = _22239;
                    }
                    _22243 = mix(_22212.xyz, _22238.xyz, float3(_13499));
                }
                else
                {
                    _22243 = _22212.xyz;
                }
                _22533 = float3(_22031.x, _22137.y, _22243.z);
            }
            else
            {
                float2 _6554 = _6484 + _6496;
                float _15783 = fast::clamp(log2(fast::max(_6398, 1.0)) - 1.0, 0.0, 5.0);
                int _15786 = int(floor(_15783));
                float _15790 = _15783 - float(_15786);
                float4 _21947;
                if (_15786 <= 0)
                {
                    float2 _21946;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _21286 = _6554;
                        _21286.y = 1.0 - _6554.y;
                        _21946 = _21286;
                    }
                    else
                    {
                        _21946 = _6554;
                    }
                    _21947 = gBackdrop0.sample(gLinear, _21946, level(0.0));
                }
                else
                {
                    float4 _21948;
                    if (_15786 == 1)
                    {
                        float2 _15925 = (_6554 * _172.gLevel[1].xy) - float2(0.5);
                        float2 _15927 = floor(_15925);
                        float2 _15930 = _15925 - _15927;
                        float2 _15933 = _15930 * _15930;
                        float2 _15936 = _15933 * _15930;
                        float2 _15955 = (((_15936 * 3.0) - (_15933 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _15968 = _15936 * 0.16666667163372039794921875;
                        float2 _15971 = (((((-_15936) + (_15933 * 3.0)) - (_15930 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15955;
                        float2 _15974 = (((((_15936 * (-3.0)) + (_15933 * 3.0)) + (_15930 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15968;
                        float2 _15984 = ((_15927 - float2(0.5)) + (_15955 / _15971)) * _172.gLevel[1].zw;
                        float2 _15994 = ((_15927 + float2(1.5)) + (_15968 / _15974)) * _172.gLevel[1].zw;
                        float2 _21942;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21291 = _15984;
                            _21291.y = 1.0 - _15984.y;
                            _21942 = _21291;
                        }
                        else
                        {
                            _21942 = _15984;
                        }
                        float _16014 = _15984.y;
                        float2 _16015 = float2(_15994.x, _16014);
                        float2 _21943;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21297 = _16015;
                            _21297.y = 1.0 - _16014;
                            _21943 = _21297;
                        }
                        else
                        {
                            _21943 = _16015;
                        }
                        float _16031 = _15994.y;
                        float2 _16032 = float2(_15984.x, _16031);
                        float2 _21944;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21304 = _16032;
                            _21304.y = 1.0 - _16031;
                            _21944 = _21304;
                        }
                        else
                        {
                            _21944 = _16032;
                        }
                        float2 _21945;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21310 = _15994;
                            _21310.y = 1.0 - _15994.y;
                            _21945 = _21310;
                        }
                        else
                        {
                            _21945 = _15994;
                        }
                        _21948 = (((gBackdrop1.sample(gLinear, _21942, level(0.0)) * _15971.x) + (gBackdrop1.sample(gLinear, _21943, level(0.0)) * _15974.x)) * _15971.y) + (((gBackdrop1.sample(gLinear, _21944, level(0.0)) * _15971.x) + (gBackdrop1.sample(gLinear, _21945, level(0.0)) * _15974.x)) * _15974.y);
                    }
                    else
                    {
                        float4 _21949;
                        if (_15786 == 2)
                        {
                            float2 _16132 = (_6554 * _172.gLevel[2].xy) - float2(0.5);
                            float2 _16134 = floor(_16132);
                            float2 _16137 = _16132 - _16134;
                            float2 _16140 = _16137 * _16137;
                            float2 _16143 = _16140 * _16137;
                            float2 _16162 = (((_16143 * 3.0) - (_16140 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _16175 = _16143 * 0.16666667163372039794921875;
                            float2 _16178 = (((((-_16143) + (_16140 * 3.0)) - (_16137 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16162;
                            float2 _16181 = (((((_16143 * (-3.0)) + (_16140 * 3.0)) + (_16137 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16175;
                            float2 _16191 = ((_16134 - float2(0.5)) + (_16162 / _16178)) * _172.gLevel[2].zw;
                            float2 _16201 = ((_16134 + float2(1.5)) + (_16175 / _16181)) * _172.gLevel[2].zw;
                            float2 _21938;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21317 = _16191;
                                _21317.y = 1.0 - _16191.y;
                                _21938 = _21317;
                            }
                            else
                            {
                                _21938 = _16191;
                            }
                            float _16221 = _16191.y;
                            float2 _16222 = float2(_16201.x, _16221);
                            float2 _21939;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21323 = _16222;
                                _21323.y = 1.0 - _16221;
                                _21939 = _21323;
                            }
                            else
                            {
                                _21939 = _16222;
                            }
                            float _16238 = _16201.y;
                            float2 _16239 = float2(_16191.x, _16238);
                            float2 _21940;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21330 = _16239;
                                _21330.y = 1.0 - _16238;
                                _21940 = _21330;
                            }
                            else
                            {
                                _21940 = _16239;
                            }
                            float2 _21941;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21336 = _16201;
                                _21336.y = 1.0 - _16201.y;
                                _21941 = _21336;
                            }
                            else
                            {
                                _21941 = _16201;
                            }
                            _21949 = (((gBackdrop2.sample(gLinear, _21938, level(0.0)) * _16178.x) + (gBackdrop2.sample(gLinear, _21939, level(0.0)) * _16181.x)) * _16178.y) + (((gBackdrop2.sample(gLinear, _21940, level(0.0)) * _16178.x) + (gBackdrop2.sample(gLinear, _21941, level(0.0)) * _16181.x)) * _16181.y);
                        }
                        else
                        {
                            float4 _21950;
                            if (_15786 == 3)
                            {
                                float2 _16339 = (_6554 * _172.gLevel[3].xy) - float2(0.5);
                                float2 _16341 = floor(_16339);
                                float2 _16344 = _16339 - _16341;
                                float2 _16347 = _16344 * _16344;
                                float2 _16350 = _16347 * _16344;
                                float2 _16369 = (((_16350 * 3.0) - (_16347 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _16382 = _16350 * 0.16666667163372039794921875;
                                float2 _16385 = (((((-_16350) + (_16347 * 3.0)) - (_16344 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16369;
                                float2 _16388 = (((((_16350 * (-3.0)) + (_16347 * 3.0)) + (_16344 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16382;
                                float2 _16398 = ((_16341 - float2(0.5)) + (_16369 / _16385)) * _172.gLevel[3].zw;
                                float2 _16408 = ((_16341 + float2(1.5)) + (_16382 / _16388)) * _172.gLevel[3].zw;
                                float2 _21934;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21343 = _16398;
                                    _21343.y = 1.0 - _16398.y;
                                    _21934 = _21343;
                                }
                                else
                                {
                                    _21934 = _16398;
                                }
                                float _16428 = _16398.y;
                                float2 _16429 = float2(_16408.x, _16428);
                                float2 _21935;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21349 = _16429;
                                    _21349.y = 1.0 - _16428;
                                    _21935 = _21349;
                                }
                                else
                                {
                                    _21935 = _16429;
                                }
                                float _16445 = _16408.y;
                                float2 _16446 = float2(_16398.x, _16445);
                                float2 _21936;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21356 = _16446;
                                    _21356.y = 1.0 - _16445;
                                    _21936 = _21356;
                                }
                                else
                                {
                                    _21936 = _16446;
                                }
                                float2 _21937;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21362 = _16408;
                                    _21362.y = 1.0 - _16408.y;
                                    _21937 = _21362;
                                }
                                else
                                {
                                    _21937 = _16408;
                                }
                                _21950 = (((gBackdrop3.sample(gLinear, _21934, level(0.0)) * _16385.x) + (gBackdrop3.sample(gLinear, _21935, level(0.0)) * _16388.x)) * _16385.y) + (((gBackdrop3.sample(gLinear, _21936, level(0.0)) * _16385.x) + (gBackdrop3.sample(gLinear, _21937, level(0.0)) * _16388.x)) * _16388.y);
                            }
                            else
                            {
                                float4 _21951;
                                if (_15786 == 4)
                                {
                                    float2 _16546 = (_6554 * _172.gLevel[4].xy) - float2(0.5);
                                    float2 _16548 = floor(_16546);
                                    float2 _16551 = _16546 - _16548;
                                    float2 _16554 = _16551 * _16551;
                                    float2 _16557 = _16554 * _16551;
                                    float2 _16576 = (((_16557 * 3.0) - (_16554 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _16589 = _16557 * 0.16666667163372039794921875;
                                    float2 _16592 = (((((-_16557) + (_16554 * 3.0)) - (_16551 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16576;
                                    float2 _16595 = (((((_16557 * (-3.0)) + (_16554 * 3.0)) + (_16551 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16589;
                                    float2 _16605 = ((_16548 - float2(0.5)) + (_16576 / _16592)) * _172.gLevel[4].zw;
                                    float2 _16615 = ((_16548 + float2(1.5)) + (_16589 / _16595)) * _172.gLevel[4].zw;
                                    float2 _21930;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21369 = _16605;
                                        _21369.y = 1.0 - _16605.y;
                                        _21930 = _21369;
                                    }
                                    else
                                    {
                                        _21930 = _16605;
                                    }
                                    float _16635 = _16605.y;
                                    float2 _16636 = float2(_16615.x, _16635);
                                    float2 _21931;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21375 = _16636;
                                        _21375.y = 1.0 - _16635;
                                        _21931 = _21375;
                                    }
                                    else
                                    {
                                        _21931 = _16636;
                                    }
                                    float _16652 = _16615.y;
                                    float2 _16653 = float2(_16605.x, _16652);
                                    float2 _21932;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21382 = _16653;
                                        _21382.y = 1.0 - _16652;
                                        _21932 = _21382;
                                    }
                                    else
                                    {
                                        _21932 = _16653;
                                    }
                                    float2 _21933;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21388 = _16615;
                                        _21388.y = 1.0 - _16615.y;
                                        _21933 = _21388;
                                    }
                                    else
                                    {
                                        _21933 = _16615;
                                    }
                                    _21951 = (((gBackdrop4.sample(gLinear, _21930, level(0.0)) * _16592.x) + (gBackdrop4.sample(gLinear, _21931, level(0.0)) * _16595.x)) * _16592.y) + (((gBackdrop4.sample(gLinear, _21932, level(0.0)) * _16592.x) + (gBackdrop4.sample(gLinear, _21933, level(0.0)) * _16595.x)) * _16595.y);
                                }
                                else
                                {
                                    float2 _16753 = (_6554 * _172.gLevel[5].xy) - float2(0.5);
                                    float2 _16755 = floor(_16753);
                                    float2 _16758 = _16753 - _16755;
                                    float2 _16761 = _16758 * _16758;
                                    float2 _16764 = _16761 * _16758;
                                    float2 _16783 = (((_16764 * 3.0) - (_16761 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _16796 = _16764 * 0.16666667163372039794921875;
                                    float2 _16799 = (((((-_16764) + (_16761 * 3.0)) - (_16758 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16783;
                                    float2 _16802 = (((((_16764 * (-3.0)) + (_16761 * 3.0)) + (_16758 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16796;
                                    float2 _16812 = ((_16755 - float2(0.5)) + (_16783 / _16799)) * _172.gLevel[5].zw;
                                    float2 _16822 = ((_16755 + float2(1.5)) + (_16796 / _16802)) * _172.gLevel[5].zw;
                                    float2 _21926;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21395 = _16812;
                                        _21395.y = 1.0 - _16812.y;
                                        _21926 = _21395;
                                    }
                                    else
                                    {
                                        _21926 = _16812;
                                    }
                                    float _16842 = _16812.y;
                                    float2 _16843 = float2(_16822.x, _16842);
                                    float2 _21927;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21401 = _16843;
                                        _21401.y = 1.0 - _16842;
                                        _21927 = _21401;
                                    }
                                    else
                                    {
                                        _21927 = _16843;
                                    }
                                    float _16859 = _16822.y;
                                    float2 _16860 = float2(_16812.x, _16859);
                                    float2 _21928;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21408 = _16860;
                                        _21408.y = 1.0 - _16859;
                                        _21928 = _21408;
                                    }
                                    else
                                    {
                                        _21928 = _16860;
                                    }
                                    float2 _21929;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21414 = _16822;
                                        _21414.y = 1.0 - _16822.y;
                                        _21929 = _21414;
                                    }
                                    else
                                    {
                                        _21929 = _16822;
                                    }
                                    _21951 = (((gBackdrop5.sample(gLinear, _21926, level(0.0)) * _16799.x) + (gBackdrop5.sample(gLinear, _21927, level(0.0)) * _16802.x)) * _16799.y) + (((gBackdrop5.sample(gLinear, _21928, level(0.0)) * _16799.x) + (gBackdrop5.sample(gLinear, _21929, level(0.0)) * _16802.x)) * _16802.y);
                                }
                                _21950 = _21951;
                            }
                            _21949 = _21950;
                        }
                        _21948 = _21949;
                    }
                    _21947 = _21948;
                }
                float3 _21978;
                if ((_15790 > 0.0199999995529651641845703125) && (_15786 < 5))
                {
                    int _15803 = _15786 + 1;
                    float4 _21973;
                    if (_15803 <= 0)
                    {
                        float2 _21972;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21419 = _6554;
                            _21419.y = 1.0 - _6554.y;
                            _21972 = _21419;
                        }
                        else
                        {
                            _21972 = _6554;
                        }
                        _21973 = gBackdrop0.sample(gLinear, _21972, level(0.0));
                    }
                    else
                    {
                        float4 _21974;
                        if (_15803 == 1)
                        {
                            float2 _17049 = (_6554 * _172.gLevel[1].xy) - float2(0.5);
                            float2 _17051 = floor(_17049);
                            float2 _17054 = _17049 - _17051;
                            float2 _17057 = _17054 * _17054;
                            float2 _17060 = _17057 * _17054;
                            float2 _17079 = (((_17060 * 3.0) - (_17057 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _17092 = _17060 * 0.16666667163372039794921875;
                            float2 _17095 = (((((-_17060) + (_17057 * 3.0)) - (_17054 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17079;
                            float2 _17098 = (((((_17060 * (-3.0)) + (_17057 * 3.0)) + (_17054 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17092;
                            float2 _17108 = ((_17051 - float2(0.5)) + (_17079 / _17095)) * _172.gLevel[1].zw;
                            float2 _17118 = ((_17051 + float2(1.5)) + (_17092 / _17098)) * _172.gLevel[1].zw;
                            float2 _21968;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21424 = _17108;
                                _21424.y = 1.0 - _17108.y;
                                _21968 = _21424;
                            }
                            else
                            {
                                _21968 = _17108;
                            }
                            float _17138 = _17108.y;
                            float2 _17139 = float2(_17118.x, _17138);
                            float2 _21969;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21430 = _17139;
                                _21430.y = 1.0 - _17138;
                                _21969 = _21430;
                            }
                            else
                            {
                                _21969 = _17139;
                            }
                            float _17155 = _17118.y;
                            float2 _17156 = float2(_17108.x, _17155);
                            float2 _21970;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21437 = _17156;
                                _21437.y = 1.0 - _17155;
                                _21970 = _21437;
                            }
                            else
                            {
                                _21970 = _17156;
                            }
                            float2 _21971;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21443 = _17118;
                                _21443.y = 1.0 - _17118.y;
                                _21971 = _21443;
                            }
                            else
                            {
                                _21971 = _17118;
                            }
                            _21974 = (((gBackdrop1.sample(gLinear, _21968, level(0.0)) * _17095.x) + (gBackdrop1.sample(gLinear, _21969, level(0.0)) * _17098.x)) * _17095.y) + (((gBackdrop1.sample(gLinear, _21970, level(0.0)) * _17095.x) + (gBackdrop1.sample(gLinear, _21971, level(0.0)) * _17098.x)) * _17098.y);
                        }
                        else
                        {
                            float4 _21975;
                            if (_15803 == 2)
                            {
                                float2 _17256 = (_6554 * _172.gLevel[2].xy) - float2(0.5);
                                float2 _17258 = floor(_17256);
                                float2 _17261 = _17256 - _17258;
                                float2 _17264 = _17261 * _17261;
                                float2 _17267 = _17264 * _17261;
                                float2 _17286 = (((_17267 * 3.0) - (_17264 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _17299 = _17267 * 0.16666667163372039794921875;
                                float2 _17302 = (((((-_17267) + (_17264 * 3.0)) - (_17261 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17286;
                                float2 _17305 = (((((_17267 * (-3.0)) + (_17264 * 3.0)) + (_17261 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17299;
                                float2 _17315 = ((_17258 - float2(0.5)) + (_17286 / _17302)) * _172.gLevel[2].zw;
                                float2 _17325 = ((_17258 + float2(1.5)) + (_17299 / _17305)) * _172.gLevel[2].zw;
                                float2 _21964;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21450 = _17315;
                                    _21450.y = 1.0 - _17315.y;
                                    _21964 = _21450;
                                }
                                else
                                {
                                    _21964 = _17315;
                                }
                                float _17345 = _17315.y;
                                float2 _17346 = float2(_17325.x, _17345);
                                float2 _21965;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21456 = _17346;
                                    _21456.y = 1.0 - _17345;
                                    _21965 = _21456;
                                }
                                else
                                {
                                    _21965 = _17346;
                                }
                                float _17362 = _17325.y;
                                float2 _17363 = float2(_17315.x, _17362);
                                float2 _21966;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21463 = _17363;
                                    _21463.y = 1.0 - _17362;
                                    _21966 = _21463;
                                }
                                else
                                {
                                    _21966 = _17363;
                                }
                                float2 _21967;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21469 = _17325;
                                    _21469.y = 1.0 - _17325.y;
                                    _21967 = _21469;
                                }
                                else
                                {
                                    _21967 = _17325;
                                }
                                _21975 = (((gBackdrop2.sample(gLinear, _21964, level(0.0)) * _17302.x) + (gBackdrop2.sample(gLinear, _21965, level(0.0)) * _17305.x)) * _17302.y) + (((gBackdrop2.sample(gLinear, _21966, level(0.0)) * _17302.x) + (gBackdrop2.sample(gLinear, _21967, level(0.0)) * _17305.x)) * _17305.y);
                            }
                            else
                            {
                                float4 _21976;
                                if (_15803 == 3)
                                {
                                    float2 _17463 = (_6554 * _172.gLevel[3].xy) - float2(0.5);
                                    float2 _17465 = floor(_17463);
                                    float2 _17468 = _17463 - _17465;
                                    float2 _17471 = _17468 * _17468;
                                    float2 _17474 = _17471 * _17468;
                                    float2 _17493 = (((_17474 * 3.0) - (_17471 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _17506 = _17474 * 0.16666667163372039794921875;
                                    float2 _17509 = (((((-_17474) + (_17471 * 3.0)) - (_17468 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17493;
                                    float2 _17512 = (((((_17474 * (-3.0)) + (_17471 * 3.0)) + (_17468 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17506;
                                    float2 _17522 = ((_17465 - float2(0.5)) + (_17493 / _17509)) * _172.gLevel[3].zw;
                                    float2 _17532 = ((_17465 + float2(1.5)) + (_17506 / _17512)) * _172.gLevel[3].zw;
                                    float2 _21960;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21476 = _17522;
                                        _21476.y = 1.0 - _17522.y;
                                        _21960 = _21476;
                                    }
                                    else
                                    {
                                        _21960 = _17522;
                                    }
                                    float _17552 = _17522.y;
                                    float2 _17553 = float2(_17532.x, _17552);
                                    float2 _21961;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21482 = _17553;
                                        _21482.y = 1.0 - _17552;
                                        _21961 = _21482;
                                    }
                                    else
                                    {
                                        _21961 = _17553;
                                    }
                                    float _17569 = _17532.y;
                                    float2 _17570 = float2(_17522.x, _17569);
                                    float2 _21962;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21489 = _17570;
                                        _21489.y = 1.0 - _17569;
                                        _21962 = _21489;
                                    }
                                    else
                                    {
                                        _21962 = _17570;
                                    }
                                    float2 _21963;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21495 = _17532;
                                        _21495.y = 1.0 - _17532.y;
                                        _21963 = _21495;
                                    }
                                    else
                                    {
                                        _21963 = _17532;
                                    }
                                    _21976 = (((gBackdrop3.sample(gLinear, _21960, level(0.0)) * _17509.x) + (gBackdrop3.sample(gLinear, _21961, level(0.0)) * _17512.x)) * _17509.y) + (((gBackdrop3.sample(gLinear, _21962, level(0.0)) * _17509.x) + (gBackdrop3.sample(gLinear, _21963, level(0.0)) * _17512.x)) * _17512.y);
                                }
                                else
                                {
                                    float4 _21977;
                                    if (_15803 == 4)
                                    {
                                        float2 _17670 = (_6554 * _172.gLevel[4].xy) - float2(0.5);
                                        float2 _17672 = floor(_17670);
                                        float2 _17675 = _17670 - _17672;
                                        float2 _17678 = _17675 * _17675;
                                        float2 _17681 = _17678 * _17675;
                                        float2 _17700 = (((_17681 * 3.0) - (_17678 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _17713 = _17681 * 0.16666667163372039794921875;
                                        float2 _17716 = (((((-_17681) + (_17678 * 3.0)) - (_17675 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17700;
                                        float2 _17719 = (((((_17681 * (-3.0)) + (_17678 * 3.0)) + (_17675 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17713;
                                        float2 _17729 = ((_17672 - float2(0.5)) + (_17700 / _17716)) * _172.gLevel[4].zw;
                                        float2 _17739 = ((_17672 + float2(1.5)) + (_17713 / _17719)) * _172.gLevel[4].zw;
                                        float2 _21956;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21502 = _17729;
                                            _21502.y = 1.0 - _17729.y;
                                            _21956 = _21502;
                                        }
                                        else
                                        {
                                            _21956 = _17729;
                                        }
                                        float _17759 = _17729.y;
                                        float2 _17760 = float2(_17739.x, _17759);
                                        float2 _21957;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21508 = _17760;
                                            _21508.y = 1.0 - _17759;
                                            _21957 = _21508;
                                        }
                                        else
                                        {
                                            _21957 = _17760;
                                        }
                                        float _17776 = _17739.y;
                                        float2 _17777 = float2(_17729.x, _17776);
                                        float2 _21958;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21515 = _17777;
                                            _21515.y = 1.0 - _17776;
                                            _21958 = _21515;
                                        }
                                        else
                                        {
                                            _21958 = _17777;
                                        }
                                        float2 _21959;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21521 = _17739;
                                            _21521.y = 1.0 - _17739.y;
                                            _21959 = _21521;
                                        }
                                        else
                                        {
                                            _21959 = _17739;
                                        }
                                        _21977 = (((gBackdrop4.sample(gLinear, _21956, level(0.0)) * _17716.x) + (gBackdrop4.sample(gLinear, _21957, level(0.0)) * _17719.x)) * _17716.y) + (((gBackdrop4.sample(gLinear, _21958, level(0.0)) * _17716.x) + (gBackdrop4.sample(gLinear, _21959, level(0.0)) * _17719.x)) * _17719.y);
                                    }
                                    else
                                    {
                                        float2 _17877 = (_6554 * _172.gLevel[5].xy) - float2(0.5);
                                        float2 _17879 = floor(_17877);
                                        float2 _17882 = _17877 - _17879;
                                        float2 _17885 = _17882 * _17882;
                                        float2 _17888 = _17885 * _17882;
                                        float2 _17907 = (((_17888 * 3.0) - (_17885 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _17920 = _17888 * 0.16666667163372039794921875;
                                        float2 _17923 = (((((-_17888) + (_17885 * 3.0)) - (_17882 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17907;
                                        float2 _17926 = (((((_17888 * (-3.0)) + (_17885 * 3.0)) + (_17882 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17920;
                                        float2 _17936 = ((_17879 - float2(0.5)) + (_17907 / _17923)) * _172.gLevel[5].zw;
                                        float2 _17946 = ((_17879 + float2(1.5)) + (_17920 / _17926)) * _172.gLevel[5].zw;
                                        float2 _21952;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21528 = _17936;
                                            _21528.y = 1.0 - _17936.y;
                                            _21952 = _21528;
                                        }
                                        else
                                        {
                                            _21952 = _17936;
                                        }
                                        float _17966 = _17936.y;
                                        float2 _17967 = float2(_17946.x, _17966);
                                        float2 _21953;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21534 = _17967;
                                            _21534.y = 1.0 - _17966;
                                            _21953 = _21534;
                                        }
                                        else
                                        {
                                            _21953 = _17967;
                                        }
                                        float _17983 = _17946.y;
                                        float2 _17984 = float2(_17936.x, _17983);
                                        float2 _21954;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21541 = _17984;
                                            _21541.y = 1.0 - _17983;
                                            _21954 = _21541;
                                        }
                                        else
                                        {
                                            _21954 = _17984;
                                        }
                                        float2 _21955;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21547 = _17946;
                                            _21547.y = 1.0 - _17946.y;
                                            _21955 = _21547;
                                        }
                                        else
                                        {
                                            _21955 = _17946;
                                        }
                                        _21977 = (((gBackdrop5.sample(gLinear, _21952, level(0.0)) * _17923.x) + (gBackdrop5.sample(gLinear, _21953, level(0.0)) * _17926.x)) * _17923.y) + (((gBackdrop5.sample(gLinear, _21954, level(0.0)) * _17923.x) + (gBackdrop5.sample(gLinear, _21955, level(0.0)) * _17926.x)) * _17926.y);
                                    }
                                    _21976 = _21977;
                                }
                                _21975 = _21976;
                            }
                            _21974 = _21975;
                        }
                        _21973 = _21974;
                    }
                    _21978 = mix(_21947.xyz, _21973.xyz, float3(_15790));
                }
                else
                {
                    _21978 = _21947.xyz;
                }
                _22533 = _21978;
            }
            float3 _23024;
            if (_6426 > 0.0)
            {
                float2 _6575 = _6484 + (((_21924 * fast::min(_6417 * 0.5, 16.0)) * _172.gDisplay.zw) * _172.gTarget.zw);
                float _18074 = fast::clamp(log2(fast::max(16.0 * _172.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _18077 = int(floor(_18074));
                float _18081 = _18074 - float(_18077);
                float2 _22511;
                if (_172.gConv.x > 0.5)
                {
                    float2 _21552 = _6575;
                    _21552.y = 1.0 - _6575.y;
                    _22511 = _21552;
                }
                else
                {
                    _22511 = _6575;
                }
                float4 _22512;
                if (_18077 <= 0)
                {
                    _22512 = gBackdrop0.sample(gLinear, _22511, level(0.0));
                }
                else
                {
                    float4 _22513;
                    if (_18077 == 1)
                    {
                        _22513 = gBackdrop1.sample(gLinear, _22511, level(0.0));
                    }
                    else
                    {
                        float4 _22514;
                        if (_18077 == 2)
                        {
                            _22514 = gBackdrop2.sample(gLinear, _22511, level(0.0));
                        }
                        else
                        {
                            float4 _22515;
                            if (_18077 == 3)
                            {
                                _22515 = gBackdrop3.sample(gLinear, _22511, level(0.0));
                            }
                            else
                            {
                                float4 _22516;
                                if (_18077 == 4)
                                {
                                    _22516 = gBackdrop4.sample(gLinear, _22511, level(0.0));
                                }
                                else
                                {
                                    _22516 = gBackdrop5.sample(gLinear, _22511, level(0.0));
                                }
                                _22515 = _22516;
                            }
                            _22514 = _22515;
                        }
                        _22513 = _22514;
                    }
                    _22512 = _22513;
                }
                float3 _22523;
                if ((_18081 > 0.0199999995529651641845703125) && (_18077 < 5))
                {
                    int _18094 = _18077 + 1;
                    float2 _22517;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _21555 = _6575;
                        _21555.y = 1.0 - _6575.y;
                        _22517 = _21555;
                    }
                    else
                    {
                        _22517 = _6575;
                    }
                    float4 _22518;
                    if (_18094 <= 0)
                    {
                        _22518 = gBackdrop0.sample(gLinear, _22517, level(0.0));
                    }
                    else
                    {
                        float4 _22519;
                        if (_18094 == 1)
                        {
                            _22519 = gBackdrop1.sample(gLinear, _22517, level(0.0));
                        }
                        else
                        {
                            float4 _22520;
                            if (_18094 == 2)
                            {
                                _22520 = gBackdrop2.sample(gLinear, _22517, level(0.0));
                            }
                            else
                            {
                                float4 _22521;
                                if (_18094 == 3)
                                {
                                    _22521 = gBackdrop3.sample(gLinear, _22517, level(0.0));
                                }
                                else
                                {
                                    float4 _22522;
                                    if (_18094 == 4)
                                    {
                                        _22522 = gBackdrop4.sample(gLinear, _22517, level(0.0));
                                    }
                                    else
                                    {
                                        _22522 = gBackdrop5.sample(gLinear, _22517, level(0.0));
                                    }
                                    _22521 = _22522;
                                }
                                _22520 = _22521;
                            }
                            _22519 = _22520;
                        }
                        _22518 = _22519;
                    }
                    _22523 = mix(_22512.xyz, _22518.xyz, float3(_18081));
                }
                else
                {
                    _22523 = _22512.xyz;
                }
                _23024 = _22523;
            }
            else
            {
                _23024 = float3(0.5);
            }
            float _22554;
            if (gFxData_1._data[_4986].x > 0.001000000047497451305389404296875)
            {
                int _18261 = clamp(int(rint(log2(36.0 * _172.gDisplay.z) - 1.0)), 1, 4);
                float2 _22524;
                if (_172.gConv.x > 0.5)
                {
                    float2 _21559 = _6484;
                    _21559.y = 1.0 - _6484.y;
                    _22524 = _21559;
                }
                else
                {
                    _22524 = _6484;
                }
                float4 _22525;
                if (_18261 <= 0)
                {
                    _22525 = gBackdrop0.sample(gLinear, _22524, level(0.0));
                }
                else
                {
                    float4 _22526;
                    if (_18261 == 1)
                    {
                        _22526 = gBackdrop1.sample(gLinear, _22524, level(0.0));
                    }
                    else
                    {
                        float4 _22527;
                        if (_18261 == 2)
                        {
                            _22527 = gBackdrop2.sample(gLinear, _22524, level(0.0));
                        }
                        else
                        {
                            float4 _22528;
                            if (_18261 == 3)
                            {
                                _22528 = gBackdrop3.sample(gLinear, _22524, level(0.0));
                            }
                            else
                            {
                                float4 _22529;
                                if (_18261 == 4)
                                {
                                    _22529 = gBackdrop4.sample(gLinear, _22524, level(0.0));
                                }
                                else
                                {
                                    _22529 = gBackdrop5.sample(gLinear, _22524, level(0.0));
                                }
                                _22528 = _22529;
                            }
                            _22527 = _22528;
                        }
                        _22526 = _22527;
                    }
                    _22525 = _22526;
                }
                _22554 = dot(_22525.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _22554 = 0.5;
            }
            _23022 = _23024;
            _22553 = _22554;
            _22530 = _22533;
        }
        else
        {
            _23022 = float3(0.5);
            _22553 = 0.5;
            _22530 = float3(0.5);
        }
        float3 _6603 = mix(float3(dot(_22530, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _22530, float3(gFxData_1._data[_4978].x)) + float3(gFxData_1._data[_4978].y);
        float3 _22782;
        if (gFxData_1._data[_4986].x > 0.001000000047497451305389404296875)
        {
            float _6613 = fast::clamp(fast::max(_22553 + gFxData_1._data[_4978].y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _6621 = mix(_6613, dot(gFxData_1._data[_4970].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), gFxData_1._data[_4986].x);
            _22782 = select(mix(_6603, float3(1.0), float3((_6621 - _6613) / fast::max(1.0 - _6613, 0.001000000047497451305389404296875))), _6603 * (_6621 / _6613), bool3(_6621 < _6613));
        }
        else
        {
            _22782 = _6603;
        }
        float2 _6659 = fast::clamp((in.i_local - gFxData_1._data[_4872].xy) / fast::max(gFxData_1._data[_4872].zw - gFxData_1._data[_4872].xy, float2(0.001000000047497451305389404296875)), float2(0.0), float2(1.0));
        float _6691 = pow(1.0 - _22786.z, 5.0);
        float2 _6707 = float2(cos(gFxData_1._data[_4978].w), sin(gFxData_1._data[_4978].w));
        float _6710 = dot(_21924, _6707);
        float _6745 = fast::clamp(0.5 + (0.5 * dot((in.i_local - ((gFxData_1._data[_4872].xy + gFxData_1._data[_4872].zw) * 0.5)) / _6409, _6707)), 0.0, 1.0);
        _23604 = ((_6691 * (pow(fast::clamp(_6710, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(fast::clamp(-_6710, 0.0, 1.0), 1.5)))) * gFxData_1._data[_4978].z) * 1.60000002384185791015625;
        _23558 = float4((mix(mix(_22782, gFxData_1._data[_4970].xyz, float3(fast::clamp(gFxData_1._data[_4970].w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _6432))) + (0.3499999940395355224609375 * ((dot(gFxData_1._data[_4970].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _6659.y) : _6659.y))), 0.0, 1.0))), (_23022 * 1.10000002384185791015625) + float3(0.07999999821186065673828125), float3(_6691 * 0.3499999940395355224609375)) + float3((((0.039999999105930328369140625 * _6745) * _6745) + (0.0500000007450580596923828125 * (1.0 - _6432))) * gFxData_1._data[_4978].z)) * _3715, _3715) + (_23271 * (1.0 - _3715));
    }
    else
    {
        _23604 = 0.0;
        _23558 = _23271;
    }
    float4 _23568;
    if ((_3692 & 1u) != 0u)
    {
        float4 _23554;
        do
        {
            uint _18385 = _4866.y;
            if (_18385 == 0u)
            {
                _23554 = gFxData_1._data[_4890];
                break;
            }
            float2 _18399 = fast::max(gFxData_1._data[_4872].zw - gFxData_1._data[_4872].xy, float2(0.001000000047497451305389404296875));
            float2 _18409 = fwidth(in.i_local);
            float _18411 = fast::max(length(_18409), 9.9999997473787516355514526367188e-05);
            float _23546;
            float _23550;
            if ((_18385 == 1u) || (_18385 == 4u))
            {
                float _18420 = cos(gFxData_1._data[_4906].x);
                float _18423 = sin(gFxData_1._data[_4906].x);
                float _18449 = ((dot(in.i_local - ((gFxData_1._data[_4872].xy + gFxData_1._data[_4872].zw) * 0.5), float2(_18420, _18423)) / fast::max(0.5 * ((abs(_18420) * _18399.x) + (abs(_18423) * _18399.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (_18385 == 4u)
                {
                    float _18464 = fast::clamp((_18449 - gFxData_1._data[_4906].y) / fast::max(gFxData_1._data[_4906].z - gFxData_1._data[_4906].y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _23554 = float4(fast::clamp(abs((fract(float3(_18464 * 0.800000011920928955078125) + float3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - float3(3.0)) - float3(1.0), float3(0.0), float3(1.0)), gFxData_1._data[_4890].w * pow(fast::max(sin(_18464 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _23550 = -1.0;
                _23546 = _18449;
            }
            else
            {
                float _23547;
                float _23551;
                if (_18385 == 2u)
                {
                    _23551 = -1.0;
                    _23547 = length(in.i_local - (gFxData_1._data[_4872].xy + (gFxData_1._data[_4906].xy * _18399))) / fast::max(gFxData_1._data[_4906].z * fast::max(_18399.x, _18399.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    float2 _18532 = in.i_local - (gFxData_1._data[_4872].xy + (gFxData_1._data[_4906].xy * _18399));
                    float _18543 = fract(((precise::atan2(_18532.y, _18532.x) - gFxData_1._data[_4906].z) * 0.15915493667125701904296875) + 1.0);
                    float _23548;
                    float _23552;
                    if (gFxData_1._data[_4906].w > 0.5)
                    {
                        _23552 = -1.0;
                        _23548 = 0.5 - (0.5 * cos(_18543 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _18563 = (((_18543 < 0.5) ? _18543 : (_18543 - 1.0)) * 6.283185482025146484375) * length(_18532);
                        float _23553;
                        if (abs(_18563) < _18411)
                        {
                            _23553 = fast::clamp(((_18563 / _18411) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _23553 = -1.0;
                        }
                        _23552 = _23553;
                        _23548 = _18543;
                    }
                    _23551 = _23552;
                    _23547 = _23548;
                }
                _23550 = _23551;
                _23546 = _23547;
            }
            float4 _18592 = float4(gFxData_1._data[_4890].xyz * gFxData_1._data[_4890].w, gFxData_1._data[_4890].w);
            float4 _18604 = float4(gFxData_1._data[_4898].xyz * gFxData_1._data[_4898].w, gFxData_1._data[_4898].w);
            float4 _18618 = select(mix(_18592, _18604, float4(fast::clamp(_23546, 0.0, 1.0))), mix(_18604, _18592, float4(_23550)), bool4(_23550 >= 0.0));
            _23554 = select(float4(0.0), float4(_18618.xyz / float3(_18618.w), _18618.w), bool4(_18618.w > 9.9999997473787516355514526367188e-06));
            break;
        } while(false);
        float4 _23555;
        if ((_3692 & 64u) != 0u)
        {
            _23555 = _23554 * gTex.sample(gLinear, mix(gFxData_1._data[_5018].xy, gFxData_1._data[_5018].zw, (in.i_local - gFxData_1._data[_4872].xy) / fast::max(gFxData_1._data[_4872].zw - gFxData_1._data[_4872].xy, float2(0.001000000047497451305389404296875))));
        }
        else
        {
            _23555 = _23554;
        }
        float _18645 = fast::clamp(_23555.w * _3715, 0.0, 1.0);
        _23568 = float4(_23555.xyz * _18645, _18645) + (_23558 * (1.0 - _18645));
    }
    else
    {
        _23568 = _23558;
    }
    float4 _23590;
    if ((_3692 & 16384u) != 0u)
    {
        float2 _4085 = fast::max(gFxData_1._data[_4872].zw - gFxData_1._data[_4872].xy, float2(0.001000000047497451305389404296875));
        float2 _4092 = (in.i_local - gFxData_1._data[_4872].xy) / _4085;
        float _4103 = ((gFxData_1._data[_4898].x >= 0.0) ? gFxData_1._data[_4898].x : _172.gTime.x) * gFxData_1._data[_5042].z;
        float _4106 = _4092.x * 2.0;
        float _4107 = _4106 - 1.0;
        float _4112 = _4092.y * _4085.y;
        float _4115 = fast::max(gFxData_1._data[_5042].w, 0.001000000047497451305389404296875);
        float _4120 = fast::clamp(1.0 - (_4107 * _4107), 0.0, 1.0);
        float _4122 = pow(_4120, 1.2999999523162841796875);
        float _4124 = pow(_4120, 0.699999988079071044921875);
        float _4127 = fast::clamp(_4103 * 1.4285714626312255859375, 0.0, 1.0);
        float _4143 = (0.25 + (0.75 * ((_4127 * _4127) * (3.0 - (2.0 * _4127))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_4103 * 2.099999904632568359375)));
        float _4152 = ((gFxData_1._data[_5042].y * _4085.y) * _4143) * _4122;
        float _4178 = (_4085.y * (0.5 + ((gFxData_1._data[_5042].x * (0.5 - (_4107 * _4107))) * 0.5))) + (((0.14000000059604644775390625 * _4085.y) * _4122) * sin(((_4107 * 2.400000095367431640625) - (_4103 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _23564;
        float _23565;
        float3 _23566;
        _23566 = float3(0.0);
        _23565 = _4178;
        _23564 = _4178;
        float3 _4250;
        float _23976;
        float _23977;
        for (int _23563 = 0; _23563 < 4; _23566 = _4250, _23565 = _23977, _23564 = _23976, _23563++)
        {
            float _4202 = _4178 + ((_4152 * _2844[_23563].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_4103 * 1.7000000476837158203125) + _2861[_23563].y))));
            _23976 = (_23563 == 0) ? _4202 : _23564;
            _23977 = (_23563 == 2) ? _4202 : _23565;
            float _4217 = _4115 * _2844[_23563].y;
            float _4222 = (_4112 - _4202) / _4217;
            float _4227 = _4115 * _2844[_23563].z;
            float3 _23956;
            _23956 = float3(0.0);
            for (int _23955 = 0; _23955 < 6; )
            {
                float _18686 = ((_4112 - _4202) - (_4227 * ((float(_23955) * 0.4000000059604644775390625) - 1.0))) / _4217;
                _23956 += (_1761[_23955] * exp((-_18686) * _18686));
                _23955++;
                continue;
            }
            _4250 = _23566 + (mix(_23956 * float3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), float3(exp((-_4222) * _4222)), float3(_2861[_23563].x)) * (_2844[_23563].w * _4124));
        }
        float _4256 = _4115 * 1.5;
        float _4286 = fast::clamp((_4112 - _23564) / fast::max(_23565 - _23564, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _4314 = (_4112 - (_23565 - (_4115 * 3.0))) / (((_4085.y * 0.0900000035762786865234375) + (_4152 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _4317 = (_4106 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _4342 = ((_4112 - _23564) + (_4115 * 5.0)) / (_4115 * 7.0);
        float3 _4368 = float3(1.0) - exp((-((((_23566 + (mix(float3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), float3(1.0), float3(_4286)) * ((((1.0 / (1.0 + exp((-((_4112 - _23564) - (_4115 * 2.0))) / _4256))) / (1.0 + exp((-(_23565 - _4112)) / _4256))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_4286, 2.5)))) * _4124))) + (float3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_4314) * _4314) - (_4317 * _4317)) * (0.5 + (1.10000002384185791015625 * _4143))))) + (float3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_4342) * _4342) * _4122) * 0.100000001490116119384765625))) * mix(float3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), float3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), float3(0.5 + (0.5 * sin(_4103 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _4378 = (gFxData_1._data[_4890].w * smoothstep(0.0, 0.119999997317790985107421875, _4092.y)) * smoothstep(1.0, 0.87999999523162841796875, _4092.y);
        float _4397 = (fast::clamp(fast::max(_4368.x, fast::max(_4368.y, _4368.z)), 0.0, 1.0) * _4378) * _3715;
        _23590 = float4((_4368 * _4378) * _3715, _4397) + (_23568 * (1.0 - _4397));
    }
    else
    {
        _23590 = _23568;
    }
    float4 _23599;
    if (((_3692 & 4u) != 0u) && ((_3692 & 256u) != 0u))
    {
        float2 _4423 = in.i_local - gFxData_1._data[_4938].zw;
        uint _18743 = _4866.z;
        float2 _18751 = (gFxData_1._data[_4872].xy + gFxData_1._data[_4872].zw) * 0.5;
        float2 _18760 = fast::max((gFxData_1._data[_4872].zw - gFxData_1._data[_4872].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _23585;
        if (_18743 == 1u)
        {
            float2 _18766 = _4423 - _18751;
            float _23584;
            do
            {
                if (gFxData_1._data[_4986].w >= 6.282185077667236328125)
                {
                    _23584 = abs(length(_18766) - gFxData_1._data[_4882].x) - gFxData_1._data[_4882].y;
                    break;
                }
                float _18865 = gFxData_1._data[_4986].z + (gFxData_1._data[_4986].w * 0.5);
                float _18867 = cos(_18865);
                float _18869 = sin(_18865);
                float _18878 = dot(_18766, float2(-_18869, _18867));
                float _18881 = dot(_18766, float2(_18867, _18869));
                float2 _18882 = float2(_18878, _18881);
                float _18885 = abs(_18878);
                _18882.x = _18885;
                float _18888 = gFxData_1._data[_4986].w * 0.5;
                float _18890 = sin(_18888);
                float _18892 = cos(_18888);
                _23584 = (((_18892 * _18885) > (_18890 * _18881)) ? length(_18882 - (float2(_18890, _18892) * gFxData_1._data[_4882].x)) : abs(length(_18882) - gFxData_1._data[_4882].x)) - gFxData_1._data[_4882].y;
                break;
            } while(false);
            _23585 = _23584;
        }
        else
        {
            float _23586;
            if (_18743 == 2u)
            {
                float2 _18928 = _4423 - gFxData_1._data[_4994].xy;
                float2 _18931 = gFxData_1._data[_4994].zw - gFxData_1._data[_4994].xy;
                _23586 = length(_18928 - (_18931 * fast::clamp(dot(_18928, _18931) / fast::max(dot(_18931, _18931), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4882].x;
            }
            else
            {
                float2 _18793 = _4423 - _18751;
                float _18984 = fast::min(_18760.x, _18760.y);
                float _18987 = fast::min((_18793.x > 0.0) ? ((_18793.y > 0.0) ? gFxData_1._data[_4882].z : gFxData_1._data[_4882].y) : ((_18793.y > 0.0) ? gFxData_1._data[_4882].w : gFxData_1._data[_4882].x), _18984);
                float _18993 = _18987 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4986].y));
                float _23569;
                float _23570;
                if (_18993 > _18984)
                {
                    float _19007 = gFxData_1._data[_4986].y * fast::clamp((_18984 - _18987) / fast::max(0.60000002384185791015625 * _18987, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _23570 = _19007;
                    _23569 = _18987 * (1.0 + (0.60000002384185791015625 * _19007));
                }
                else
                {
                    _23570 = gFxData_1._data[_4986].y;
                    _23569 = _18993;
                }
                float2 _19020 = (abs(_18793) - _18760) + float2(_23569);
                float2 _19022 = fast::max(_19020, float2(0.0));
                float _23571;
                if ((_19022.x > 0.0) && (_19022.y > 0.0))
                {
                    float _23572;
                    if ((_23570 > 0.001000000047497451305389404296875) && (_23569 > 9.9999997473787516355514526367188e-05))
                    {
                        float _19039 = 2.0 + (2.0 * _23570);
                        float2 _19044 = _19022 / float2(fast::max(_23569, 9.9999997473787516355514526367188e-05));
                        _23572 = pow(pow(_19044.x, _19039) + pow(_19044.y, _19039), 1.0 / _19039) * _23569;
                    }
                    else
                    {
                        _23572 = length(_19022);
                    }
                    _23571 = _23572;
                }
                else
                {
                    _23571 = fast::max(_19022.x, _19022.y);
                }
                float _19079 = (fast::min(fast::max(_19020.x, _19020.y), 0.0) + _23571) - _23569;
                float _23587;
                if ((_4866.x & 512u) != 0u)
                {
                    float2 _18821 = fast::max((gFxData_1._data[_4994].zw - gFxData_1._data[_4994].xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _18824 = _4423 - ((gFxData_1._data[_4994].xy + gFxData_1._data[_4994].zw) * 0.5);
                    float _19115 = fast::min(_18821.x, _18821.y);
                    float _19118 = fast::min((_18824.x > 0.0) ? ((_18824.y > 0.0) ? gFxData_1._data[_5002].x : gFxData_1._data[_5002].x) : ((_18824.y > 0.0) ? gFxData_1._data[_5002].x : gFxData_1._data[_5002].x), _19115);
                    float _19124 = _19118 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4986].y));
                    float _23575;
                    float _23576;
                    if (_19124 > _19115)
                    {
                        float _19138 = gFxData_1._data[_4986].y * fast::clamp((_19115 - _19118) / fast::max(0.60000002384185791015625 * _19118, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _23576 = _19138;
                        _23575 = _19118 * (1.0 + (0.60000002384185791015625 * _19138));
                    }
                    else
                    {
                        _23576 = gFxData_1._data[_4986].y;
                        _23575 = _19124;
                    }
                    float2 _19151 = (abs(_18824) - _18821) + float2(_23575);
                    float2 _19153 = fast::max(_19151, float2(0.0));
                    float _23577;
                    if ((_19153.x > 0.0) && (_19153.y > 0.0))
                    {
                        float _23578;
                        if ((_23576 > 0.001000000047497451305389404296875) && (_23575 > 9.9999997473787516355514526367188e-05))
                        {
                            float _19170 = 2.0 + (2.0 * _23576);
                            float2 _19175 = _19153 / float2(fast::max(_23575, 9.9999997473787516355514526367188e-05));
                            _23578 = pow(pow(_19175.x, _19170) + pow(_19175.y, _19170), 1.0 / _19170) * _23575;
                        }
                        else
                        {
                            _23578 = length(_19153);
                        }
                        _23577 = _23578;
                    }
                    else
                    {
                        _23577 = fast::max(_19153.x, _19153.y);
                    }
                    float _19210 = (fast::min(fast::max(_19151.x, _19151.y), 0.0) + _23577) - _23575;
                    float _19215 = fast::max(gFxData_1._data[_5002].y, 9.9999997473787516355514526367188e-05);
                    float _19224 = fast::max(_19215 - abs(_19079 - _19210), 0.0) / _19215;
                    _23587 = fast::min(_19079, _19210) - (((_19224 * _19224) * _19215) * 0.25);
                }
                else
                {
                    _23587 = _19079;
                }
                _23586 = _23587;
            }
            _23585 = _23586;
        }
        float _4432 = (_23585 + gFxData_1._data[_4938].y) / (fast::max(gFxData_1._data[_4938].x * 0.5, _3707 * 0.5) * 1.41421353816986083984375);
        float _19241 = sign(_4432);
        float _19243 = abs(_4432);
        float _19254 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_19243 * _19243))) * _19243)) * _19243);
        float _19257 = _19254 * _19254;
        float _19272 = fast::clamp(gFxData_1._data[_4930].w * ((0.5 + (0.5 * (_19241 - (_19241 / (_19257 * _19257))))) * _3715), 0.0, 1.0);
        _23599 = float4(gFxData_1._data[_4930].xyz * _19272, _19272) + (_23590 * (1.0 - _19272));
    }
    else
    {
        _23599 = _23590;
    }
    float4 _23622;
    if ((_3692 & 16u) != 0u)
    {
        float _4456 = fast::max(-_21804, 0.0) / fast::max(gFxData_1._data[_4954].z, 0.001000000047497451305389404296875);
        float _19298 = fast::clamp(gFxData_1._data[_4946].w * fast::clamp((exp(((-_4456) * _4456) * 2.2000000476837158203125) * gFxData_1._data[_4954].w) * _3715, 0.0, 1.0), 0.0, 1.0);
        _23622 = float4(gFxData_1._data[_4946].xyz * _19298, _19298) + (_23599 * (1.0 - _19298));
    }
    else
    {
        _23622 = _23599;
    }
    float3 _4485 = _23622.xyz + float3((_23604 * _3715) * _23622.w);
    float4 _21684 = _23622;
    _21684.x = _4485.x;
    _21684.y = _4485.y;
    _21684.z = _4485.z;
    float4 _23625;
    if ((_3692 & 2u) != 0u)
    {
        float4 _23623;
        if (gFxData_1._data[_4922].z < 0.999000012874603271484375)
        {
            float2 _4535 = fast::max(gFxData_1._data[_4872].zw - gFxData_1._data[_4872].xy, float2(0.001000000047497451305389404296875));
            float _4546 = cos(gFxData_1._data[_4922].w);
            float _4549 = sin(gFxData_1._data[_4922].w);
            float4 _21701 = _4916;
            _21701.w = _4916.w * mix(1.0, gFxData_1._data[_4922].z, fast::clamp(((dot(in.i_local - ((gFxData_1._data[_4872].xy + gFxData_1._data[_4872].zw) * 0.5), float2(_4546, _4549)) / fast::max(0.5 * ((abs(_4546) * _4535.x) + (abs(_4549) * _4535.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _23623 = _21701;
        }
        else
        {
            _23623 = _4916;
        }
        float _19324 = fast::clamp(_23623.w * (fast::clamp(0.5 - ((_21804 - (gFxData_1._data[_4922].x * gFxData_1._data[_4922].y)) / _3707), 0.0, 1.0) - fast::clamp(0.5 - ((_21804 + (gFxData_1._data[_4922].x * (1.0 - gFxData_1._data[_4922].y))) / _3707), 0.0, 1.0)), 0.0, 1.0);
        _23625 = float4(_23623.xyz * _19324, _19324) + (_21684 * (1.0 - _19324));
    }
    else
    {
        _23625 = _21684;
    }
    float4 _23626;
    if ((_3692 & 128u) != 0u)
    {
        float2 _4610 = (in.i_local - gFxData_1._data[_4872].xy) / fast::max(gFxData_1._data[_4872].zw - gFxData_1._data[_4872].xy, float2(0.001000000047497451305389404296875));
        float _4632 = exp(-pow((((_4610.x * 0.85000002384185791015625) + (_4610.y * 0.1500000059604644775390625)) - ((fract(_172.gTime.x * gFxData_1._data[_5010].w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _23626 = float4(_23625.xyz + float3(((_4632 * gFxData_1._data[_5010].z) * _3715) * fast::max(_23625.w, 0.3499999940395355224609375)), fast::max(_23625.w, ((_4632 * gFxData_1._data[_5010].z) * _3715) * 0.5));
    }
    else
    {
        _23626 = _23625;
    }
    float4 _23950;
    if ((_3692 & 2048u) != 0u)
    {
        float3 _19349 = fract(floor(_21787).xyx * 0.103100001811981201171875);
        float3 _19358 = _19349 + float3(dot(_19349, _19349.yzx + float3(33.3300018310546875)));
        float3 _4683 = _23626.xyz + float3(((fract((_19358.x + _19358.y) * _19358.z) - 0.5) * gFxData_1._data[_5010].y) * _23626.w);
        float4 _21725 = _23626;
        _21725.x = _4683.x;
        _21725.y = _4683.y;
        _21725.z = _4683.z;
        _23950 = _21725;
    }
    else
    {
        _23950 = _23626;
    }
    float _23946;
    if (_215.gFade.z > 0.0)
    {
        _23946 = smoothstep(0.0, 1.0, fast::clamp((_21787.y - _215.gFade.x) / _215.gFade.z, 0.0, 1.0));
    }
    else
    {
        _23946 = 1.0;
    }
    float _23947;
    if (_215.gFade.w > 0.0)
    {
        _23947 = _23946 * smoothstep(0.0, 1.0, fast::clamp((_215.gFade.y - _21787.y) / _215.gFade.w, 0.0, 1.0));
    }
    else
    {
        _23947 = _23946;
    }
    float4 _4700 = _23950 * ((gFxData_1._data[_5010].x * _23637) * _23947);
    float4 _23951;
    if (((_3692 & 8u) != 0u) || ((_3692 & 4u) != 0u))
    {
        float3 _19410 = fract((floor(_21787) + float2(17.0)).xyx * 0.103100001811981201171875);
        float3 _19419 = _19410 + float3(dot(_19410, _19410.yzx + float3(33.3300018310546875)));
        float3 _4724 = _4700.xyz + float3(((fract((_19419.x + _19419.y) * _19419.z) - 0.5) * 0.0039215688593685626983642578125) * fast::clamp(_4700.w * 8.0, 0.0, 1.0));
        float4 _21737 = _4700;
        _21737.x = _4724.x;
        _21737.y = _4724.y;
        _21737.z = _4724.z;
        _23951 = _21737;
    }
    else
    {
        _23951 = _4700;
    }
    float3 _4734 = fast::max(_23951.xyz, float3(0.0));
    float4 _21743 = _23951;
    _21743.x = _4734.x;
    _21743.y = _4734.y;
    _21743.z = _4734.z;
    float4 _23952;
    if ((_172.gTime.w > 0.5) && (_23951.w > 9.9999997473787516355514526367188e-06))
    {
        float3 _19463 = fast::clamp(_21743.xyz / float3(_23951.w), float3(0.0), float3(1.0));
        float3 _19449 = select(pow((_19463 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _19463 * float3(0.077399380505084991455078125), _19463 <= float3(0.040449999272823333740234375)) * _23951.w;
        float4 _21752 = _21743;
        _21752.x = _19449.x;
        _21752.y = _19449.y;
        _21752.z = _19449.z;
        _23952 = _21752;
    }
    else
    {
        _23952 = _21743;
    }
    out._entryPointOutput = _23952;
    return out;
}

