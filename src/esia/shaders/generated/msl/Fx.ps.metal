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
    uint _4864 = in.i_instance * 24u;
    uint _4874 = (in.i_instance * 24u) + 1u;
    uint _4882 = (in.i_instance * 24u) + 2u;
    uint _4890 = (in.i_instance * 24u) + 3u;
    uint _4898 = (in.i_instance * 24u) + 4u;
    uint _4906 = (in.i_instance * 24u) + 5u;
    float4 _4908 = gFxData_1._data[_4906];
    uint _4914 = (in.i_instance * 24u) + 6u;
    uint _4922 = (in.i_instance * 24u) + 7u;
    uint _4930 = (in.i_instance * 24u) + 8u;
    uint _4938 = (in.i_instance * 24u) + 9u;
    uint _4946 = (in.i_instance * 24u) + 10u;
    uint _4954 = (in.i_instance * 24u) + 11u;
    uint _4962 = (in.i_instance * 24u) + 12u;
    uint _4970 = (in.i_instance * 24u) + 13u;
    uint _4978 = (in.i_instance * 24u) + 14u;
    uint _4986 = (in.i_instance * 24u) + 15u;
    uint _4994 = (in.i_instance * 24u) + 16u;
    uint _5002 = (in.i_instance * 24u) + 17u;
    uint _5010 = (in.i_instance * 24u) + 18u;
    uint _5018 = (in.i_instance * 24u) + 19u;
    uint _5026 = (in.i_instance * 24u) + 20u;
    uint _5034 = (in.i_instance * 24u) + 21u;
    uint _5042 = (in.i_instance * 24u) + 22u;
    uint4 _4858 = uint4(gFxData_1._data[(in.i_instance * 24u) + 23u]);
    uint _3688 = _4858.x;
    float2 _5061 = gl_FragCoord.xy + _172.gConv.yy;
    float2 _21779;
    if (_172.gConv.x > 0.5)
    {
        float2 _20092 = _5061;
        _20092.y = _172.gTarget.y - _5061.y;
        _21779 = _20092;
    }
    else
    {
        _21779 = _5061;
    }
    float _3696 = dfdx(in.i_local.x);
    float _3700 = dfdy(in.i_local.x);
    float _3703 = fast::max(abs(_3696) + abs(_3700), 9.9999997473787516355514526367188e-05);
    uint _5104 = _4858.z;
    float2 _5112 = (gFxData_1._data[_4864].xy + gFxData_1._data[_4864].zw) * 0.5;
    float2 _5121 = fast::max((gFxData_1._data[_4864].zw - gFxData_1._data[_4864].xy) * 0.5, float2(0.001000000047497451305389404296875));
    float _21796;
    if (_5104 == 1u)
    {
        float2 _5127 = in.i_local - _5112;
        float _21795;
        do
        {
            if (gFxData_1._data[_4978].w >= 6.282185077667236328125)
            {
                _21795 = abs(length(_5127) - gFxData_1._data[_4874].x) - gFxData_1._data[_4874].y;
                break;
            }
            float _5226 = gFxData_1._data[_4978].z + (gFxData_1._data[_4978].w * 0.5);
            float _5228 = cos(_5226);
            float _5230 = sin(_5226);
            float _5239 = dot(_5127, float2(-_5230, _5228));
            float _5242 = dot(_5127, float2(_5228, _5230));
            float2 _5243 = float2(_5239, _5242);
            float _5246 = abs(_5239);
            _5243.x = _5246;
            float _5249 = gFxData_1._data[_4978].w * 0.5;
            float _5251 = sin(_5249);
            float _5253 = cos(_5249);
            _21795 = (((_5253 * _5246) > (_5251 * _5242)) ? length(_5243 - (float2(_5251, _5253) * gFxData_1._data[_4874].x)) : abs(length(_5243) - gFxData_1._data[_4874].x)) - gFxData_1._data[_4874].y;
            break;
        } while(false);
        _21796 = _21795;
    }
    else
    {
        float _21797;
        if (_5104 == 2u)
        {
            float2 _5289 = in.i_local - gFxData_1._data[_4986].xy;
            float2 _5292 = gFxData_1._data[_4986].zw - gFxData_1._data[_4986].xy;
            _21797 = length(_5289 - (_5292 * fast::clamp(dot(_5289, _5292) / fast::max(dot(_5292, _5292), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4874].x;
        }
        else
        {
            float2 _5154 = in.i_local - _5112;
            float _5345 = fast::min(_5121.x, _5121.y);
            float _5348 = fast::min((_5154.x > 0.0) ? ((_5154.y > 0.0) ? gFxData_1._data[_4874].z : gFxData_1._data[_4874].y) : ((_5154.y > 0.0) ? gFxData_1._data[_4874].w : gFxData_1._data[_4874].x), _5345);
            float _5354 = _5348 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4978].y));
            float _21780;
            float _21781;
            if (_5354 > _5345)
            {
                float _5368 = gFxData_1._data[_4978].y * fast::clamp((_5345 - _5348) / fast::max(0.60000002384185791015625 * _5348, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                _21781 = _5368;
                _21780 = _5348 * (1.0 + (0.60000002384185791015625 * _5368));
            }
            else
            {
                _21781 = gFxData_1._data[_4978].y;
                _21780 = _5354;
            }
            float2 _5381 = (abs(_5154) - _5121) + float2(_21780);
            float2 _5383 = fast::max(_5381, float2(0.0));
            float _21782;
            if ((_5383.x > 0.0) && (_5383.y > 0.0))
            {
                float _21783;
                if ((_21781 > 0.001000000047497451305389404296875) && (_21780 > 9.9999997473787516355514526367188e-05))
                {
                    float _5400 = 2.0 + (2.0 * _21781);
                    float2 _5405 = _5383 / float2(fast::max(_21780, 9.9999997473787516355514526367188e-05));
                    _21783 = pow(pow(_5405.x, _5400) + pow(_5405.y, _5400), 1.0 / _5400) * _21780;
                }
                else
                {
                    _21783 = length(_5383);
                }
                _21782 = _21783;
            }
            else
            {
                _21782 = fast::max(_5383.x, _5383.y);
            }
            float _5440 = (fast::min(fast::max(_5381.x, _5381.y), 0.0) + _21782) - _21780;
            float _21798;
            if ((_4858.x & 512u) != 0u)
            {
                float2 _5182 = fast::max((gFxData_1._data[_4986].zw - gFxData_1._data[_4986].xy) * 0.5, float2(0.001000000047497451305389404296875));
                float2 _5185 = in.i_local - ((gFxData_1._data[_4986].xy + gFxData_1._data[_4986].zw) * 0.5);
                float _5476 = fast::min(_5182.x, _5182.y);
                float _5479 = fast::min((_5185.x > 0.0) ? ((_5185.y > 0.0) ? gFxData_1._data[_4994].x : gFxData_1._data[_4994].x) : ((_5185.y > 0.0) ? gFxData_1._data[_4994].x : gFxData_1._data[_4994].x), _5476);
                float _5485 = _5479 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4978].y));
                float _21786;
                float _21787;
                if (_5485 > _5476)
                {
                    float _5499 = gFxData_1._data[_4978].y * fast::clamp((_5476 - _5479) / fast::max(0.60000002384185791015625 * _5479, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _21787 = _5499;
                    _21786 = _5479 * (1.0 + (0.60000002384185791015625 * _5499));
                }
                else
                {
                    _21787 = gFxData_1._data[_4978].y;
                    _21786 = _5485;
                }
                float2 _5512 = (abs(_5185) - _5182) + float2(_21786);
                float2 _5514 = fast::max(_5512, float2(0.0));
                float _21788;
                if ((_5514.x > 0.0) && (_5514.y > 0.0))
                {
                    float _21789;
                    if ((_21787 > 0.001000000047497451305389404296875) && (_21786 > 9.9999997473787516355514526367188e-05))
                    {
                        float _5531 = 2.0 + (2.0 * _21787);
                        float2 _5536 = _5514 / float2(fast::max(_21786, 9.9999997473787516355514526367188e-05));
                        _21789 = pow(pow(_5536.x, _5531) + pow(_5536.y, _5531), 1.0 / _5531) * _21786;
                    }
                    else
                    {
                        _21789 = length(_5514);
                    }
                    _21788 = _21789;
                }
                else
                {
                    _21788 = fast::max(_5514.x, _5514.y);
                }
                float _5571 = (fast::min(fast::max(_5512.x, _5512.y), 0.0) + _21788) - _21786;
                float _5576 = fast::max(gFxData_1._data[_4994].y, 9.9999997473787516355514526367188e-05);
                float _5585 = fast::max(_5576 - abs(_5440 - _5571), 0.0) / _5576;
                _21798 = fast::min(_5440, _5571) - (((_5585 * _5585) * _5576) * 0.25);
            }
            else
            {
                _21798 = _5440;
            }
            _21797 = _21798;
        }
        _21796 = _21797;
    }
    float _3711 = fast::clamp(0.5 - (_21796 / _3703), 0.0, 1.0);
    float _23629;
    if ((_3688 & 1024u) != 0u)
    {
        float2 _3732 = fast::max((gFxData_1._data[_5018].zw - gFxData_1._data[_5018].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float2 _3735 = in.i_local - ((gFxData_1._data[_5018].xy + gFxData_1._data[_5018].zw) * 0.5);
        float _5631 = fast::min(_3732.x, _3732.y);
        float _5634 = fast::min((_3735.x > 0.0) ? ((_3735.y > 0.0) ? gFxData_1._data[_5026].x : gFxData_1._data[_5026].x) : ((_3735.y > 0.0) ? gFxData_1._data[_5026].x : gFxData_1._data[_5026].x), _5631);
        float _5640 = _5634 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_5026].y));
        float _21799;
        float _21800;
        if (_5640 > _5631)
        {
            float _5654 = gFxData_1._data[_5026].y * fast::clamp((_5631 - _5634) / fast::max(0.60000002384185791015625 * _5634, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _21800 = _5654;
            _21799 = _5634 * (1.0 + (0.60000002384185791015625 * _5654));
        }
        else
        {
            _21800 = gFxData_1._data[_5026].y;
            _21799 = _5640;
        }
        float2 _5667 = (abs(_3735) - _3732) + float2(_21799);
        float2 _5669 = fast::max(_5667, float2(0.0));
        float _21801;
        if ((_5669.x > 0.0) && (_5669.y > 0.0))
        {
            float _21802;
            if ((_21800 > 0.001000000047497451305389404296875) && (_21799 > 9.9999997473787516355514526367188e-05))
            {
                float _5686 = 2.0 + (2.0 * _21800);
                float2 _5691 = _5669 / float2(fast::max(_21799, 9.9999997473787516355514526367188e-05));
                _21802 = pow(pow(_5691.x, _5686) + pow(_5691.y, _5686), 1.0 / _5686) * _21799;
            }
            else
            {
                _21802 = length(_5669);
            }
            _21801 = _21802;
        }
        else
        {
            _21801 = fast::max(_5669.x, _5669.y);
        }
        float _3747 = fast::clamp(0.5 - (((fast::min(fast::max(_5667.x, _5667.y), 0.0) + _21801) - _21799) / _3703), 0.0, 1.0);
        if (_3747 <= 0.0)
        {
            discard_fragment();
        }
        _23629 = _3747;
    }
    else
    {
        _23629 = 1.0;
    }
    bool _3758 = ((_3688 & 32u) != 0u) && (_3711 >= 0.999000012874603271484375);
    float4 _21838;
    if ((((_3688 & 4u) != 0u) && (!((_3688 & 256u) != 0u))) && (!_3758))
    {
        float2 _3781 = in.i_local - gFxData_1._data[_4930].zw;
        uint _5757 = _4858.z;
        float2 _5765 = (gFxData_1._data[_4864].xy + gFxData_1._data[_4864].zw) * 0.5;
        float2 _5774 = fast::max((gFxData_1._data[_4864].zw - gFxData_1._data[_4864].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _21821;
        if (_5757 == 1u)
        {
            float2 _5780 = _3781 - _5765;
            float _21820;
            do
            {
                if (gFxData_1._data[_4978].w >= 6.282185077667236328125)
                {
                    _21820 = abs(length(_5780) - gFxData_1._data[_4874].x) - gFxData_1._data[_4874].y;
                    break;
                }
                float _5879 = gFxData_1._data[_4978].z + (gFxData_1._data[_4978].w * 0.5);
                float _5881 = cos(_5879);
                float _5883 = sin(_5879);
                float _5892 = dot(_5780, float2(-_5883, _5881));
                float _5895 = dot(_5780, float2(_5881, _5883));
                float2 _5896 = float2(_5892, _5895);
                float _5899 = abs(_5892);
                _5896.x = _5899;
                float _5902 = gFxData_1._data[_4978].w * 0.5;
                float _5904 = sin(_5902);
                float _5906 = cos(_5902);
                _21820 = (((_5906 * _5899) > (_5904 * _5895)) ? length(_5896 - (float2(_5904, _5906) * gFxData_1._data[_4874].x)) : abs(length(_5896) - gFxData_1._data[_4874].x)) - gFxData_1._data[_4874].y;
                break;
            } while(false);
            _21821 = _21820;
        }
        else
        {
            float _21822;
            if (_5757 == 2u)
            {
                float2 _5942 = _3781 - gFxData_1._data[_4986].xy;
                float2 _5945 = gFxData_1._data[_4986].zw - gFxData_1._data[_4986].xy;
                _21822 = length(_5942 - (_5945 * fast::clamp(dot(_5942, _5945) / fast::max(dot(_5945, _5945), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4874].x;
            }
            else
            {
                float2 _5807 = _3781 - _5765;
                float _5998 = fast::min(_5774.x, _5774.y);
                float _6001 = fast::min((_5807.x > 0.0) ? ((_5807.y > 0.0) ? gFxData_1._data[_4874].z : gFxData_1._data[_4874].y) : ((_5807.y > 0.0) ? gFxData_1._data[_4874].w : gFxData_1._data[_4874].x), _5998);
                float _6007 = _6001 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4978].y));
                float _21805;
                float _21806;
                if (_6007 > _5998)
                {
                    float _6021 = gFxData_1._data[_4978].y * fast::clamp((_5998 - _6001) / fast::max(0.60000002384185791015625 * _6001, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _21806 = _6021;
                    _21805 = _6001 * (1.0 + (0.60000002384185791015625 * _6021));
                }
                else
                {
                    _21806 = gFxData_1._data[_4978].y;
                    _21805 = _6007;
                }
                float2 _6034 = (abs(_5807) - _5774) + float2(_21805);
                float2 _6036 = fast::max(_6034, float2(0.0));
                float _21807;
                if ((_6036.x > 0.0) && (_6036.y > 0.0))
                {
                    float _21808;
                    if ((_21806 > 0.001000000047497451305389404296875) && (_21805 > 9.9999997473787516355514526367188e-05))
                    {
                        float _6053 = 2.0 + (2.0 * _21806);
                        float2 _6058 = _6036 / float2(fast::max(_21805, 9.9999997473787516355514526367188e-05));
                        _21808 = pow(pow(_6058.x, _6053) + pow(_6058.y, _6053), 1.0 / _6053) * _21805;
                    }
                    else
                    {
                        _21808 = length(_6036);
                    }
                    _21807 = _21808;
                }
                else
                {
                    _21807 = fast::max(_6036.x, _6036.y);
                }
                float _6093 = (fast::min(fast::max(_6034.x, _6034.y), 0.0) + _21807) - _21805;
                float _21823;
                if ((_4858.x & 512u) != 0u)
                {
                    float2 _5835 = fast::max((gFxData_1._data[_4986].zw - gFxData_1._data[_4986].xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _5838 = _3781 - ((gFxData_1._data[_4986].xy + gFxData_1._data[_4986].zw) * 0.5);
                    float _6129 = fast::min(_5835.x, _5835.y);
                    float _6132 = fast::min((_5838.x > 0.0) ? ((_5838.y > 0.0) ? gFxData_1._data[_4994].x : gFxData_1._data[_4994].x) : ((_5838.y > 0.0) ? gFxData_1._data[_4994].x : gFxData_1._data[_4994].x), _6129);
                    float _6138 = _6132 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4978].y));
                    float _21811;
                    float _21812;
                    if (_6138 > _6129)
                    {
                        float _6152 = gFxData_1._data[_4978].y * fast::clamp((_6129 - _6132) / fast::max(0.60000002384185791015625 * _6132, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21812 = _6152;
                        _21811 = _6132 * (1.0 + (0.60000002384185791015625 * _6152));
                    }
                    else
                    {
                        _21812 = gFxData_1._data[_4978].y;
                        _21811 = _6138;
                    }
                    float2 _6165 = (abs(_5838) - _5835) + float2(_21811);
                    float2 _6167 = fast::max(_6165, float2(0.0));
                    float _21813;
                    if ((_6167.x > 0.0) && (_6167.y > 0.0))
                    {
                        float _21814;
                        if ((_21812 > 0.001000000047497451305389404296875) && (_21811 > 9.9999997473787516355514526367188e-05))
                        {
                            float _6184 = 2.0 + (2.0 * _21812);
                            float2 _6189 = _6167 / float2(fast::max(_21811, 9.9999997473787516355514526367188e-05));
                            _21814 = pow(pow(_6189.x, _6184) + pow(_6189.y, _6184), 1.0 / _6184) * _21811;
                        }
                        else
                        {
                            _21814 = length(_6167);
                        }
                        _21813 = _21814;
                    }
                    else
                    {
                        _21813 = fast::max(_6167.x, _6167.y);
                    }
                    float _6224 = (fast::min(fast::max(_6165.x, _6165.y), 0.0) + _21813) - _21811;
                    float _6229 = fast::max(gFxData_1._data[_4994].y, 9.9999997473787516355514526367188e-05);
                    float _6238 = fast::max(_6229 - abs(_6093 - _6224), 0.0) / _6229;
                    _21823 = fast::min(_6093, _6224) - (((_6238 * _6238) * _6229) * 0.25);
                }
                else
                {
                    _21823 = _6093;
                }
                _21822 = _21823;
            }
            _21821 = _21822;
        }
        float _3790 = (_21821 - gFxData_1._data[_4930].y) / (fast::max(gFxData_1._data[_4930].x * 0.5, _3703 * 0.5) * 1.41421353816986083984375);
        float _6255 = sign(_3790);
        float _6257 = abs(_3790);
        float _6268 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_6257 * _6257))) * _6257)) * _6257);
        float _6271 = _6268 * _6268;
        float _6286 = fast::clamp(gFxData_1._data[_4922].w * (0.5 - (0.5 * (_6255 - (_6255 / (_6271 * _6271))))), 0.0, 1.0);
        _21838 = float4(gFxData_1._data[_4922].xyz * _6286, _6286);
    }
    else
    {
        _21838 = float4(0.0);
    }
    float4 _23263;
    if (((_3688 & 8u) != 0u) && (!_3758))
    {
        float _3810 = fast::max(gFxData_1._data[_4946].x, 0.001000000047497451305389404296875);
        float _21835;
        float _21836;
        if ((_3688 & 8192u) != 0u)
        {
            float2 _3822 = (gFxData_1._data[_4864].xy + gFxData_1._data[_4864].zw) * 0.5;
            float2 _3831 = fast::max((gFxData_1._data[_4864].zw - gFxData_1._data[_4864].xy) * 0.5, float2(0.001000000047497451305389404296875));
            float2 _3838 = gFxData_1._data[_4864].xy - gFxData_1._data[_5042].xy;
            float2 _3845 = gFxData_1._data[_5042].zw - gFxData_1._data[_4864].zw;
            float2 _3875 = fast::clamp(float2((in.i_local.x < _3822.x) ? _3838.x : _3845.x, (in.i_local.y < _3822.y) ? _3838.y : _3845.y) * float2(0.58823525905609130859375), float2(fast::min(_3810, 1.5)), float2(_3810));
            float _3880 = fast::min(_3831.x, _3831.y);
            float _21834;
            if (_4858.z == 0u)
            {
                _21834 = fast::min(((in.i_local.x > _3822.x) ? ((in.i_local.y > _3822.y) ? gFxData_1._data[_4874].z : gFxData_1._data[_4874].y) : ((in.i_local.y > _3822.y) ? gFxData_1._data[_4874].w : gFxData_1._data[_4874].x)) * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4978].y)), _3880);
            }
            else
            {
                _21834 = _3880;
            }
            float2 _3929 = fast::max(abs(in.i_local - _3822) - (_3831 - float2(_21834)), float2(0.0));
            float _3931 = length(_3929);
            float2 _3939 = select(float2(0.707099974155426025390625), _3929 / float2(_3931), bool2(_3931 > 9.9999997473787516355514526367188e-05));
            float2 _3954 = in.i_local - gFxData_1._data[_5042].xy;
            float2 _3959 = gFxData_1._data[_5042].zw - in.i_local;
            _21836 = fast::clamp(fast::min(fast::min(_3954.x, _3954.y), fast::min(_3959.x, _3959.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _21835 = rsqrt(dot(_3939 * _3939, float2(1.0) / (_3875 * _3875)));
        }
        else
        {
            _21836 = 1.0;
            _21835 = _3810;
        }
        float _3977 = fast::max(_21796, 0.0) / _21835;
        float _6312 = fast::clamp(gFxData_1._data[_4938].w * fast::clamp((exp(((-_3977) * _3977) * 2.2000000476837158203125) * gFxData_1._data[_4946].y) * _21836, 0.0, 1.0), 0.0, 1.0);
        _23263 = float4(gFxData_1._data[_4938].xyz * _6312, _6312) + (_21838 * (1.0 - _6312));
    }
    else
    {
        _23263 = _21838;
    }
    float4 _23550;
    float _23596;
    if (((_3688 & 32u) != 0u) && (_3711 > 0.0))
    {
        float _6390 = gFxData_1._data[_4954].x * _172.gDisplay.z;
        float2 _6401 = fast::max((gFxData_1._data[_4864].zw - gFxData_1._data[_4864].xy) * 0.5, float2(1.0));
        float _6409 = fast::clamp(gFxData_1._data[_4954].z, 0.001000000047497451305389404296875, fast::min(_6401.x, _6401.y));
        float _6418 = fast::clamp(1.0 - (fast::max(-_21796, 0.0) / _6409), 0.0, 1.0);
        float _6424 = sqrt(fast::clamp(1.0 - (_6418 * _6418), 0.0, 1.0));
        float2 _21916;
        float3 _22778;
        if (_6418 > 0.0)
        {
            float2 _6770 = in.i_local + float2(0.5, 0.0);
            uint _6830 = _4858.z;
            float2 _6838 = (gFxData_1._data[_4864].xy + gFxData_1._data[_4864].zw) * 0.5;
            float2 _6847 = fast::max((gFxData_1._data[_4864].zw - gFxData_1._data[_4864].xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _21856;
            if (_6830 == 1u)
            {
                float2 _6853 = _6770 - _6838;
                float _21855;
                do
                {
                    if (gFxData_1._data[_4978].w >= 6.282185077667236328125)
                    {
                        _21855 = abs(length(_6853) - gFxData_1._data[_4874].x) - gFxData_1._data[_4874].y;
                        break;
                    }
                    float _6952 = gFxData_1._data[_4978].z + (gFxData_1._data[_4978].w * 0.5);
                    float _6954 = cos(_6952);
                    float _6956 = sin(_6952);
                    float _6965 = dot(_6853, float2(-_6956, _6954));
                    float _6968 = dot(_6853, float2(_6954, _6956));
                    float2 _6969 = float2(_6965, _6968);
                    float _6972 = abs(_6965);
                    _6969.x = _6972;
                    float _6975 = gFxData_1._data[_4978].w * 0.5;
                    float _6977 = sin(_6975);
                    float _6979 = cos(_6975);
                    _21855 = (((_6979 * _6972) > (_6977 * _6968)) ? length(_6969 - (float2(_6977, _6979) * gFxData_1._data[_4874].x)) : abs(length(_6969) - gFxData_1._data[_4874].x)) - gFxData_1._data[_4874].y;
                    break;
                } while(false);
                _21856 = _21855;
            }
            else
            {
                float _21857;
                if (_6830 == 2u)
                {
                    float2 _7015 = _6770 - gFxData_1._data[_4986].xy;
                    float2 _7018 = gFxData_1._data[_4986].zw - gFxData_1._data[_4986].xy;
                    _21857 = length(_7015 - (_7018 * fast::clamp(dot(_7015, _7018) / fast::max(dot(_7018, _7018), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4874].x;
                }
                else
                {
                    float2 _6880 = _6770 - _6838;
                    float _7071 = fast::min(_6847.x, _6847.y);
                    float _7074 = fast::min((_6880.x > 0.0) ? ((_6880.y > 0.0) ? gFxData_1._data[_4874].z : gFxData_1._data[_4874].y) : ((_6880.y > 0.0) ? gFxData_1._data[_4874].w : gFxData_1._data[_4874].x), _7071);
                    float _7080 = _7074 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4978].y));
                    float _21840;
                    float _21841;
                    if (_7080 > _7071)
                    {
                        float _7094 = gFxData_1._data[_4978].y * fast::clamp((_7071 - _7074) / fast::max(0.60000002384185791015625 * _7074, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21841 = _7094;
                        _21840 = _7074 * (1.0 + (0.60000002384185791015625 * _7094));
                    }
                    else
                    {
                        _21841 = gFxData_1._data[_4978].y;
                        _21840 = _7080;
                    }
                    float2 _7107 = (abs(_6880) - _6847) + float2(_21840);
                    float2 _7109 = fast::max(_7107, float2(0.0));
                    float _21842;
                    if ((_7109.x > 0.0) && (_7109.y > 0.0))
                    {
                        float _21843;
                        if ((_21841 > 0.001000000047497451305389404296875) && (_21840 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7126 = 2.0 + (2.0 * _21841);
                            float2 _7131 = _7109 / float2(fast::max(_21840, 9.9999997473787516355514526367188e-05));
                            _21843 = pow(pow(_7131.x, _7126) + pow(_7131.y, _7126), 1.0 / _7126) * _21840;
                        }
                        else
                        {
                            _21843 = length(_7109);
                        }
                        _21842 = _21843;
                    }
                    else
                    {
                        _21842 = fast::max(_7109.x, _7109.y);
                    }
                    float _7166 = (fast::min(fast::max(_7107.x, _7107.y), 0.0) + _21842) - _21840;
                    float _21858;
                    if ((_4858.x & 512u) != 0u)
                    {
                        float2 _6908 = fast::max((gFxData_1._data[_4986].zw - gFxData_1._data[_4986].xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _6911 = _6770 - ((gFxData_1._data[_4986].xy + gFxData_1._data[_4986].zw) * 0.5);
                        float _7202 = fast::min(_6908.x, _6908.y);
                        float _7205 = fast::min((_6911.x > 0.0) ? ((_6911.y > 0.0) ? gFxData_1._data[_4994].x : gFxData_1._data[_4994].x) : ((_6911.y > 0.0) ? gFxData_1._data[_4994].x : gFxData_1._data[_4994].x), _7202);
                        float _7211 = _7205 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4978].y));
                        float _21846;
                        float _21847;
                        if (_7211 > _7202)
                        {
                            float _7225 = gFxData_1._data[_4978].y * fast::clamp((_7202 - _7205) / fast::max(0.60000002384185791015625 * _7205, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21847 = _7225;
                            _21846 = _7205 * (1.0 + (0.60000002384185791015625 * _7225));
                        }
                        else
                        {
                            _21847 = gFxData_1._data[_4978].y;
                            _21846 = _7211;
                        }
                        float2 _7238 = (abs(_6911) - _6908) + float2(_21846);
                        float2 _7240 = fast::max(_7238, float2(0.0));
                        float _21848;
                        if ((_7240.x > 0.0) && (_7240.y > 0.0))
                        {
                            float _21849;
                            if ((_21847 > 0.001000000047497451305389404296875) && (_21846 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7257 = 2.0 + (2.0 * _21847);
                                float2 _7262 = _7240 / float2(fast::max(_21846, 9.9999997473787516355514526367188e-05));
                                _21849 = pow(pow(_7262.x, _7257) + pow(_7262.y, _7257), 1.0 / _7257) * _21846;
                            }
                            else
                            {
                                _21849 = length(_7240);
                            }
                            _21848 = _21849;
                        }
                        else
                        {
                            _21848 = fast::max(_7240.x, _7240.y);
                        }
                        float _7297 = (fast::min(fast::max(_7238.x, _7238.y), 0.0) + _21848) - _21846;
                        float _7302 = fast::max(gFxData_1._data[_4994].y, 9.9999997473787516355514526367188e-05);
                        float _7311 = fast::max(_7302 - abs(_7166 - _7297), 0.0) / _7302;
                        _21858 = fast::min(_7166, _7297) - (((_7311 * _7311) * _7302) * 0.25);
                    }
                    else
                    {
                        _21858 = _7166;
                    }
                    _21857 = _21858;
                }
                _21856 = _21857;
            }
            float2 _6774 = in.i_local - float2(0.5, 0.0);
            uint _7352 = _4858.z;
            float2 _7360 = (gFxData_1._data[_4864].xy + gFxData_1._data[_4864].zw) * 0.5;
            float2 _7369 = fast::max((gFxData_1._data[_4864].zw - gFxData_1._data[_4864].xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _21875;
            if (_7352 == 1u)
            {
                float2 _7375 = _6774 - _7360;
                float _21874;
                do
                {
                    if (gFxData_1._data[_4978].w >= 6.282185077667236328125)
                    {
                        _21874 = abs(length(_7375) - gFxData_1._data[_4874].x) - gFxData_1._data[_4874].y;
                        break;
                    }
                    float _7474 = gFxData_1._data[_4978].z + (gFxData_1._data[_4978].w * 0.5);
                    float _7476 = cos(_7474);
                    float _7478 = sin(_7474);
                    float _7487 = dot(_7375, float2(-_7478, _7476));
                    float _7490 = dot(_7375, float2(_7476, _7478));
                    float2 _7491 = float2(_7487, _7490);
                    float _7494 = abs(_7487);
                    _7491.x = _7494;
                    float _7497 = gFxData_1._data[_4978].w * 0.5;
                    float _7499 = sin(_7497);
                    float _7501 = cos(_7497);
                    _21874 = (((_7501 * _7494) > (_7499 * _7490)) ? length(_7491 - (float2(_7499, _7501) * gFxData_1._data[_4874].x)) : abs(length(_7491) - gFxData_1._data[_4874].x)) - gFxData_1._data[_4874].y;
                    break;
                } while(false);
                _21875 = _21874;
            }
            else
            {
                float _21876;
                if (_7352 == 2u)
                {
                    float2 _7537 = _6774 - gFxData_1._data[_4986].xy;
                    float2 _7540 = gFxData_1._data[_4986].zw - gFxData_1._data[_4986].xy;
                    _21876 = length(_7537 - (_7540 * fast::clamp(dot(_7537, _7540) / fast::max(dot(_7540, _7540), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4874].x;
                }
                else
                {
                    float2 _7402 = _6774 - _7360;
                    float _7593 = fast::min(_7369.x, _7369.y);
                    float _7596 = fast::min((_7402.x > 0.0) ? ((_7402.y > 0.0) ? gFxData_1._data[_4874].z : gFxData_1._data[_4874].y) : ((_7402.y > 0.0) ? gFxData_1._data[_4874].w : gFxData_1._data[_4874].x), _7593);
                    float _7602 = _7596 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4978].y));
                    float _21859;
                    float _21860;
                    if (_7602 > _7593)
                    {
                        float _7616 = gFxData_1._data[_4978].y * fast::clamp((_7593 - _7596) / fast::max(0.60000002384185791015625 * _7596, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21860 = _7616;
                        _21859 = _7596 * (1.0 + (0.60000002384185791015625 * _7616));
                    }
                    else
                    {
                        _21860 = gFxData_1._data[_4978].y;
                        _21859 = _7602;
                    }
                    float2 _7629 = (abs(_7402) - _7369) + float2(_21859);
                    float2 _7631 = fast::max(_7629, float2(0.0));
                    float _21861;
                    if ((_7631.x > 0.0) && (_7631.y > 0.0))
                    {
                        float _21862;
                        if ((_21860 > 0.001000000047497451305389404296875) && (_21859 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7648 = 2.0 + (2.0 * _21860);
                            float2 _7653 = _7631 / float2(fast::max(_21859, 9.9999997473787516355514526367188e-05));
                            _21862 = pow(pow(_7653.x, _7648) + pow(_7653.y, _7648), 1.0 / _7648) * _21859;
                        }
                        else
                        {
                            _21862 = length(_7631);
                        }
                        _21861 = _21862;
                    }
                    else
                    {
                        _21861 = fast::max(_7631.x, _7631.y);
                    }
                    float _7688 = (fast::min(fast::max(_7629.x, _7629.y), 0.0) + _21861) - _21859;
                    float _21877;
                    if ((_4858.x & 512u) != 0u)
                    {
                        float2 _7430 = fast::max((gFxData_1._data[_4986].zw - gFxData_1._data[_4986].xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _7433 = _6774 - ((gFxData_1._data[_4986].xy + gFxData_1._data[_4986].zw) * 0.5);
                        float _7724 = fast::min(_7430.x, _7430.y);
                        float _7727 = fast::min((_7433.x > 0.0) ? ((_7433.y > 0.0) ? gFxData_1._data[_4994].x : gFxData_1._data[_4994].x) : ((_7433.y > 0.0) ? gFxData_1._data[_4994].x : gFxData_1._data[_4994].x), _7724);
                        float _7733 = _7727 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4978].y));
                        float _21865;
                        float _21866;
                        if (_7733 > _7724)
                        {
                            float _7747 = gFxData_1._data[_4978].y * fast::clamp((_7724 - _7727) / fast::max(0.60000002384185791015625 * _7727, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21866 = _7747;
                            _21865 = _7727 * (1.0 + (0.60000002384185791015625 * _7747));
                        }
                        else
                        {
                            _21866 = gFxData_1._data[_4978].y;
                            _21865 = _7733;
                        }
                        float2 _7760 = (abs(_7433) - _7430) + float2(_21865);
                        float2 _7762 = fast::max(_7760, float2(0.0));
                        float _21867;
                        if ((_7762.x > 0.0) && (_7762.y > 0.0))
                        {
                            float _21868;
                            if ((_21866 > 0.001000000047497451305389404296875) && (_21865 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7779 = 2.0 + (2.0 * _21866);
                                float2 _7784 = _7762 / float2(fast::max(_21865, 9.9999997473787516355514526367188e-05));
                                _21868 = pow(pow(_7784.x, _7779) + pow(_7784.y, _7779), 1.0 / _7779) * _21865;
                            }
                            else
                            {
                                _21868 = length(_7762);
                            }
                            _21867 = _21868;
                        }
                        else
                        {
                            _21867 = fast::max(_7762.x, _7762.y);
                        }
                        float _7819 = (fast::min(fast::max(_7760.x, _7760.y), 0.0) + _21867) - _21865;
                        float _7824 = fast::max(gFxData_1._data[_4994].y, 9.9999997473787516355514526367188e-05);
                        float _7833 = fast::max(_7824 - abs(_7688 - _7819), 0.0) / _7824;
                        _21877 = fast::min(_7688, _7819) - (((_7833 * _7833) * _7824) * 0.25);
                    }
                    else
                    {
                        _21877 = _7688;
                    }
                    _21876 = _21877;
                }
                _21875 = _21876;
            }
            float2 _6779 = in.i_local + float2(0.0, 0.5);
            uint _7874 = _4858.z;
            float2 _7882 = (gFxData_1._data[_4864].xy + gFxData_1._data[_4864].zw) * 0.5;
            float2 _7891 = fast::max((gFxData_1._data[_4864].zw - gFxData_1._data[_4864].xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _21894;
            if (_7874 == 1u)
            {
                float2 _7897 = _6779 - _7882;
                float _21893;
                do
                {
                    if (gFxData_1._data[_4978].w >= 6.282185077667236328125)
                    {
                        _21893 = abs(length(_7897) - gFxData_1._data[_4874].x) - gFxData_1._data[_4874].y;
                        break;
                    }
                    float _7996 = gFxData_1._data[_4978].z + (gFxData_1._data[_4978].w * 0.5);
                    float _7998 = cos(_7996);
                    float _8000 = sin(_7996);
                    float _8009 = dot(_7897, float2(-_8000, _7998));
                    float _8012 = dot(_7897, float2(_7998, _8000));
                    float2 _8013 = float2(_8009, _8012);
                    float _8016 = abs(_8009);
                    _8013.x = _8016;
                    float _8019 = gFxData_1._data[_4978].w * 0.5;
                    float _8021 = sin(_8019);
                    float _8023 = cos(_8019);
                    _21893 = (((_8023 * _8016) > (_8021 * _8012)) ? length(_8013 - (float2(_8021, _8023) * gFxData_1._data[_4874].x)) : abs(length(_8013) - gFxData_1._data[_4874].x)) - gFxData_1._data[_4874].y;
                    break;
                } while(false);
                _21894 = _21893;
            }
            else
            {
                float _21895;
                if (_7874 == 2u)
                {
                    float2 _8059 = _6779 - gFxData_1._data[_4986].xy;
                    float2 _8062 = gFxData_1._data[_4986].zw - gFxData_1._data[_4986].xy;
                    _21895 = length(_8059 - (_8062 * fast::clamp(dot(_8059, _8062) / fast::max(dot(_8062, _8062), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4874].x;
                }
                else
                {
                    float2 _7924 = _6779 - _7882;
                    float _8115 = fast::min(_7891.x, _7891.y);
                    float _8118 = fast::min((_7924.x > 0.0) ? ((_7924.y > 0.0) ? gFxData_1._data[_4874].z : gFxData_1._data[_4874].y) : ((_7924.y > 0.0) ? gFxData_1._data[_4874].w : gFxData_1._data[_4874].x), _8115);
                    float _8124 = _8118 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4978].y));
                    float _21878;
                    float _21879;
                    if (_8124 > _8115)
                    {
                        float _8138 = gFxData_1._data[_4978].y * fast::clamp((_8115 - _8118) / fast::max(0.60000002384185791015625 * _8118, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21879 = _8138;
                        _21878 = _8118 * (1.0 + (0.60000002384185791015625 * _8138));
                    }
                    else
                    {
                        _21879 = gFxData_1._data[_4978].y;
                        _21878 = _8124;
                    }
                    float2 _8151 = (abs(_7924) - _7891) + float2(_21878);
                    float2 _8153 = fast::max(_8151, float2(0.0));
                    float _21880;
                    if ((_8153.x > 0.0) && (_8153.y > 0.0))
                    {
                        float _21881;
                        if ((_21879 > 0.001000000047497451305389404296875) && (_21878 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8170 = 2.0 + (2.0 * _21879);
                            float2 _8175 = _8153 / float2(fast::max(_21878, 9.9999997473787516355514526367188e-05));
                            _21881 = pow(pow(_8175.x, _8170) + pow(_8175.y, _8170), 1.0 / _8170) * _21878;
                        }
                        else
                        {
                            _21881 = length(_8153);
                        }
                        _21880 = _21881;
                    }
                    else
                    {
                        _21880 = fast::max(_8153.x, _8153.y);
                    }
                    float _8210 = (fast::min(fast::max(_8151.x, _8151.y), 0.0) + _21880) - _21878;
                    float _21896;
                    if ((_4858.x & 512u) != 0u)
                    {
                        float2 _7952 = fast::max((gFxData_1._data[_4986].zw - gFxData_1._data[_4986].xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _7955 = _6779 - ((gFxData_1._data[_4986].xy + gFxData_1._data[_4986].zw) * 0.5);
                        float _8246 = fast::min(_7952.x, _7952.y);
                        float _8249 = fast::min((_7955.x > 0.0) ? ((_7955.y > 0.0) ? gFxData_1._data[_4994].x : gFxData_1._data[_4994].x) : ((_7955.y > 0.0) ? gFxData_1._data[_4994].x : gFxData_1._data[_4994].x), _8246);
                        float _8255 = _8249 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4978].y));
                        float _21884;
                        float _21885;
                        if (_8255 > _8246)
                        {
                            float _8269 = gFxData_1._data[_4978].y * fast::clamp((_8246 - _8249) / fast::max(0.60000002384185791015625 * _8249, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21885 = _8269;
                            _21884 = _8249 * (1.0 + (0.60000002384185791015625 * _8269));
                        }
                        else
                        {
                            _21885 = gFxData_1._data[_4978].y;
                            _21884 = _8255;
                        }
                        float2 _8282 = (abs(_7955) - _7952) + float2(_21884);
                        float2 _8284 = fast::max(_8282, float2(0.0));
                        float _21886;
                        if ((_8284.x > 0.0) && (_8284.y > 0.0))
                        {
                            float _21887;
                            if ((_21885 > 0.001000000047497451305389404296875) && (_21884 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8301 = 2.0 + (2.0 * _21885);
                                float2 _8306 = _8284 / float2(fast::max(_21884, 9.9999997473787516355514526367188e-05));
                                _21887 = pow(pow(_8306.x, _8301) + pow(_8306.y, _8301), 1.0 / _8301) * _21884;
                            }
                            else
                            {
                                _21887 = length(_8284);
                            }
                            _21886 = _21887;
                        }
                        else
                        {
                            _21886 = fast::max(_8284.x, _8284.y);
                        }
                        float _8341 = (fast::min(fast::max(_8282.x, _8282.y), 0.0) + _21886) - _21884;
                        float _8346 = fast::max(gFxData_1._data[_4994].y, 9.9999997473787516355514526367188e-05);
                        float _8355 = fast::max(_8346 - abs(_8210 - _8341), 0.0) / _8346;
                        _21896 = fast::min(_8210, _8341) - (((_8355 * _8355) * _8346) * 0.25);
                    }
                    else
                    {
                        _21896 = _8210;
                    }
                    _21895 = _21896;
                }
                _21894 = _21895;
            }
            float2 _6783 = in.i_local - float2(0.0, 0.5);
            uint _8396 = _4858.z;
            float2 _8404 = (gFxData_1._data[_4864].xy + gFxData_1._data[_4864].zw) * 0.5;
            float2 _8413 = fast::max((gFxData_1._data[_4864].zw - gFxData_1._data[_4864].xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _21913;
            if (_8396 == 1u)
            {
                float2 _8419 = _6783 - _8404;
                float _21912;
                do
                {
                    if (gFxData_1._data[_4978].w >= 6.282185077667236328125)
                    {
                        _21912 = abs(length(_8419) - gFxData_1._data[_4874].x) - gFxData_1._data[_4874].y;
                        break;
                    }
                    float _8518 = gFxData_1._data[_4978].z + (gFxData_1._data[_4978].w * 0.5);
                    float _8520 = cos(_8518);
                    float _8522 = sin(_8518);
                    float _8531 = dot(_8419, float2(-_8522, _8520));
                    float _8534 = dot(_8419, float2(_8520, _8522));
                    float2 _8535 = float2(_8531, _8534);
                    float _8538 = abs(_8531);
                    _8535.x = _8538;
                    float _8541 = gFxData_1._data[_4978].w * 0.5;
                    float _8543 = sin(_8541);
                    float _8545 = cos(_8541);
                    _21912 = (((_8545 * _8538) > (_8543 * _8534)) ? length(_8535 - (float2(_8543, _8545) * gFxData_1._data[_4874].x)) : abs(length(_8535) - gFxData_1._data[_4874].x)) - gFxData_1._data[_4874].y;
                    break;
                } while(false);
                _21913 = _21912;
            }
            else
            {
                float _21914;
                if (_8396 == 2u)
                {
                    float2 _8581 = _6783 - gFxData_1._data[_4986].xy;
                    float2 _8584 = gFxData_1._data[_4986].zw - gFxData_1._data[_4986].xy;
                    _21914 = length(_8581 - (_8584 * fast::clamp(dot(_8581, _8584) / fast::max(dot(_8584, _8584), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4874].x;
                }
                else
                {
                    float2 _8446 = _6783 - _8404;
                    float _8637 = fast::min(_8413.x, _8413.y);
                    float _8640 = fast::min((_8446.x > 0.0) ? ((_8446.y > 0.0) ? gFxData_1._data[_4874].z : gFxData_1._data[_4874].y) : ((_8446.y > 0.0) ? gFxData_1._data[_4874].w : gFxData_1._data[_4874].x), _8637);
                    float _8646 = _8640 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4978].y));
                    float _21897;
                    float _21898;
                    if (_8646 > _8637)
                    {
                        float _8660 = gFxData_1._data[_4978].y * fast::clamp((_8637 - _8640) / fast::max(0.60000002384185791015625 * _8640, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21898 = _8660;
                        _21897 = _8640 * (1.0 + (0.60000002384185791015625 * _8660));
                    }
                    else
                    {
                        _21898 = gFxData_1._data[_4978].y;
                        _21897 = _8646;
                    }
                    float2 _8673 = (abs(_8446) - _8413) + float2(_21897);
                    float2 _8675 = fast::max(_8673, float2(0.0));
                    float _21899;
                    if ((_8675.x > 0.0) && (_8675.y > 0.0))
                    {
                        float _21900;
                        if ((_21898 > 0.001000000047497451305389404296875) && (_21897 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8692 = 2.0 + (2.0 * _21898);
                            float2 _8697 = _8675 / float2(fast::max(_21897, 9.9999997473787516355514526367188e-05));
                            _21900 = pow(pow(_8697.x, _8692) + pow(_8697.y, _8692), 1.0 / _8692) * _21897;
                        }
                        else
                        {
                            _21900 = length(_8675);
                        }
                        _21899 = _21900;
                    }
                    else
                    {
                        _21899 = fast::max(_8675.x, _8675.y);
                    }
                    float _8732 = (fast::min(fast::max(_8673.x, _8673.y), 0.0) + _21899) - _21897;
                    float _21915;
                    if ((_4858.x & 512u) != 0u)
                    {
                        float2 _8474 = fast::max((gFxData_1._data[_4986].zw - gFxData_1._data[_4986].xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _8477 = _6783 - ((gFxData_1._data[_4986].xy + gFxData_1._data[_4986].zw) * 0.5);
                        float _8768 = fast::min(_8474.x, _8474.y);
                        float _8771 = fast::min((_8477.x > 0.0) ? ((_8477.y > 0.0) ? gFxData_1._data[_4994].x : gFxData_1._data[_4994].x) : ((_8477.y > 0.0) ? gFxData_1._data[_4994].x : gFxData_1._data[_4994].x), _8768);
                        float _8777 = _8771 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4978].y));
                        float _21903;
                        float _21904;
                        if (_8777 > _8768)
                        {
                            float _8791 = gFxData_1._data[_4978].y * fast::clamp((_8768 - _8771) / fast::max(0.60000002384185791015625 * _8771, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21904 = _8791;
                            _21903 = _8771 * (1.0 + (0.60000002384185791015625 * _8791));
                        }
                        else
                        {
                            _21904 = gFxData_1._data[_4978].y;
                            _21903 = _8777;
                        }
                        float2 _8804 = (abs(_8477) - _8474) + float2(_21903);
                        float2 _8806 = fast::max(_8804, float2(0.0));
                        float _21905;
                        if ((_8806.x > 0.0) && (_8806.y > 0.0))
                        {
                            float _21906;
                            if ((_21904 > 0.001000000047497451305389404296875) && (_21903 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8823 = 2.0 + (2.0 * _21904);
                                float2 _8828 = _8806 / float2(fast::max(_21903, 9.9999997473787516355514526367188e-05));
                                _21906 = pow(pow(_8828.x, _8823) + pow(_8828.y, _8823), 1.0 / _8823) * _21903;
                            }
                            else
                            {
                                _21906 = length(_8806);
                            }
                            _21905 = _21906;
                        }
                        else
                        {
                            _21905 = fast::max(_8806.x, _8806.y);
                        }
                        float _8863 = (fast::min(fast::max(_8804.x, _8804.y), 0.0) + _21905) - _21903;
                        float _8868 = fast::max(gFxData_1._data[_4994].y, 9.9999997473787516355514526367188e-05);
                        float _8877 = fast::max(_8868 - abs(_8732 - _8863), 0.0) / _8868;
                        _21915 = fast::min(_8732, _8863) - (((_8877 * _8877) * _8868) * 0.25);
                    }
                    else
                    {
                        _21915 = _8732;
                    }
                    _21914 = _21915;
                }
                _21913 = _21914;
            }
            float2 _6789 = float2(_21856 - _21875, _21894 - _21913);
            float _6791 = length(_6789);
            float2 _6799 = select(float2(0.0, -1.0), _6789 / float2(_6791), bool2(_6791 > 9.9999997473787516355514526367188e-06));
            _22778 = fast::normalize(float3(_6799 * fast::min(_6418 / fast::max(_6424, 0.001000000047497451305389404296875), 8.0), 1.0));
            _21916 = _6799;
        }
        else
        {
            _22778 = float3(0.0, 0.0, 1.0);
            _21916 = float2(0.0, -1.0);
        }
        float2 _6450 = ((-_21916) * gFxData_1._data[_4954].y) * (1.0 - _6424);
        float2 _21917;
        if (gFxData_1._data[_4994].z > 0.0)
        {
            _21917 = (((gFxData_1._data[_4864].xy + gFxData_1._data[_4864].zw) * 0.5) - in.i_local) * (gFxData_1._data[_4994].z / (1.0 + gFxData_1._data[_4994].z));
        }
        else
        {
            _21917 = float2(0.0);
        }
        float2 _6476 = _21779 * _172.gTarget.zw;
        float2 _6483 = _172.gDisplay.zw * _172.gTarget.zw;
        float2 _6488 = (_6450 + _21917) * _6483;
        float2 _6491 = _6450 * _6483;
        float3 _22522;
        float _22545;
        float3 _23014;
        if (_172.gTime.z > 0.5)
        {
            float3 _22525;
            if (((gFxData_1._data[_4954].w > 0.001000000047497451305389404296875) && (_6418 > 0.0)) && ((((gFxData_1._data[_4954].y * 0.300000011920928955078125) * gFxData_1._data[_4954].w) * _172.gDisplay.z) > (_6390 * 0.3499999940395355224609375)))
            {
                float _6513 = 0.300000011920928955078125 * gFxData_1._data[_4954].w;
                float2 _6520 = (_6476 + _6488) - (_6491 * _6513);
                float _8902 = fast::clamp(log2(fast::max(_6390, 1.0)) - 1.0, 0.0, 5.0);
                int _8905 = int(floor(_8902));
                float _8909 = _8902 - float(_8905);
                float4 _21992;
                if (_8905 <= 0)
                {
                    float2 _21991;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _20474 = _6520;
                        _20474.y = 1.0 - _6520.y;
                        _21991 = _20474;
                    }
                    else
                    {
                        _21991 = _6520;
                    }
                    _21992 = gBackdrop0.sample(gLinear, _21991, level(0.0));
                }
                else
                {
                    float4 _21993;
                    if (_8905 == 1)
                    {
                        float2 _9044 = (_6520 * _172.gLevel[1].xy) - float2(0.5);
                        float2 _9046 = floor(_9044);
                        float2 _9049 = _9044 - _9046;
                        float2 _9052 = _9049 * _9049;
                        float2 _9055 = _9052 * _9049;
                        float2 _9074 = (((_9055 * 3.0) - (_9052 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _9087 = _9055 * 0.16666667163372039794921875;
                        float2 _9090 = (((((-_9055) + (_9052 * 3.0)) - (_9049 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9074;
                        float2 _9093 = (((((_9055 * (-3.0)) + (_9052 * 3.0)) + (_9049 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9087;
                        float2 _9103 = ((_9046 - float2(0.5)) + (_9074 / _9090)) * _172.gLevel[1].zw;
                        float2 _9113 = ((_9046 + float2(1.5)) + (_9087 / _9093)) * _172.gLevel[1].zw;
                        float2 _21987;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20479 = _9103;
                            _20479.y = 1.0 - _9103.y;
                            _21987 = _20479;
                        }
                        else
                        {
                            _21987 = _9103;
                        }
                        float _9133 = _9103.y;
                        float2 _9134 = float2(_9113.x, _9133);
                        float2 _21988;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20485 = _9134;
                            _20485.y = 1.0 - _9133;
                            _21988 = _20485;
                        }
                        else
                        {
                            _21988 = _9134;
                        }
                        float _9150 = _9113.y;
                        float2 _9151 = float2(_9103.x, _9150);
                        float2 _21989;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20492 = _9151;
                            _20492.y = 1.0 - _9150;
                            _21989 = _20492;
                        }
                        else
                        {
                            _21989 = _9151;
                        }
                        float2 _21990;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20498 = _9113;
                            _20498.y = 1.0 - _9113.y;
                            _21990 = _20498;
                        }
                        else
                        {
                            _21990 = _9113;
                        }
                        _21993 = (((gBackdrop1.sample(gLinear, _21987, level(0.0)) * _9090.x) + (gBackdrop1.sample(gLinear, _21988, level(0.0)) * _9093.x)) * _9090.y) + (((gBackdrop1.sample(gLinear, _21989, level(0.0)) * _9090.x) + (gBackdrop1.sample(gLinear, _21990, level(0.0)) * _9093.x)) * _9093.y);
                    }
                    else
                    {
                        float4 _21994;
                        if (_8905 == 2)
                        {
                            float2 _9251 = (_6520 * _172.gLevel[2].xy) - float2(0.5);
                            float2 _9253 = floor(_9251);
                            float2 _9256 = _9251 - _9253;
                            float2 _9259 = _9256 * _9256;
                            float2 _9262 = _9259 * _9256;
                            float2 _9281 = (((_9262 * 3.0) - (_9259 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _9294 = _9262 * 0.16666667163372039794921875;
                            float2 _9297 = (((((-_9262) + (_9259 * 3.0)) - (_9256 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9281;
                            float2 _9300 = (((((_9262 * (-3.0)) + (_9259 * 3.0)) + (_9256 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9294;
                            float2 _9310 = ((_9253 - float2(0.5)) + (_9281 / _9297)) * _172.gLevel[2].zw;
                            float2 _9320 = ((_9253 + float2(1.5)) + (_9294 / _9300)) * _172.gLevel[2].zw;
                            float2 _21983;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20505 = _9310;
                                _20505.y = 1.0 - _9310.y;
                                _21983 = _20505;
                            }
                            else
                            {
                                _21983 = _9310;
                            }
                            float _9340 = _9310.y;
                            float2 _9341 = float2(_9320.x, _9340);
                            float2 _21984;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20511 = _9341;
                                _20511.y = 1.0 - _9340;
                                _21984 = _20511;
                            }
                            else
                            {
                                _21984 = _9341;
                            }
                            float _9357 = _9320.y;
                            float2 _9358 = float2(_9310.x, _9357);
                            float2 _21985;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20518 = _9358;
                                _20518.y = 1.0 - _9357;
                                _21985 = _20518;
                            }
                            else
                            {
                                _21985 = _9358;
                            }
                            float2 _21986;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20524 = _9320;
                                _20524.y = 1.0 - _9320.y;
                                _21986 = _20524;
                            }
                            else
                            {
                                _21986 = _9320;
                            }
                            _21994 = (((gBackdrop2.sample(gLinear, _21983, level(0.0)) * _9297.x) + (gBackdrop2.sample(gLinear, _21984, level(0.0)) * _9300.x)) * _9297.y) + (((gBackdrop2.sample(gLinear, _21985, level(0.0)) * _9297.x) + (gBackdrop2.sample(gLinear, _21986, level(0.0)) * _9300.x)) * _9300.y);
                        }
                        else
                        {
                            float4 _21995;
                            if (_8905 == 3)
                            {
                                float2 _9458 = (_6520 * _172.gLevel[3].xy) - float2(0.5);
                                float2 _9460 = floor(_9458);
                                float2 _9463 = _9458 - _9460;
                                float2 _9466 = _9463 * _9463;
                                float2 _9469 = _9466 * _9463;
                                float2 _9488 = (((_9469 * 3.0) - (_9466 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _9501 = _9469 * 0.16666667163372039794921875;
                                float2 _9504 = (((((-_9469) + (_9466 * 3.0)) - (_9463 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9488;
                                float2 _9507 = (((((_9469 * (-3.0)) + (_9466 * 3.0)) + (_9463 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9501;
                                float2 _9517 = ((_9460 - float2(0.5)) + (_9488 / _9504)) * _172.gLevel[3].zw;
                                float2 _9527 = ((_9460 + float2(1.5)) + (_9501 / _9507)) * _172.gLevel[3].zw;
                                float2 _21979;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20531 = _9517;
                                    _20531.y = 1.0 - _9517.y;
                                    _21979 = _20531;
                                }
                                else
                                {
                                    _21979 = _9517;
                                }
                                float _9547 = _9517.y;
                                float2 _9548 = float2(_9527.x, _9547);
                                float2 _21980;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20537 = _9548;
                                    _20537.y = 1.0 - _9547;
                                    _21980 = _20537;
                                }
                                else
                                {
                                    _21980 = _9548;
                                }
                                float _9564 = _9527.y;
                                float2 _9565 = float2(_9517.x, _9564);
                                float2 _21981;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20544 = _9565;
                                    _20544.y = 1.0 - _9564;
                                    _21981 = _20544;
                                }
                                else
                                {
                                    _21981 = _9565;
                                }
                                float2 _21982;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20550 = _9527;
                                    _20550.y = 1.0 - _9527.y;
                                    _21982 = _20550;
                                }
                                else
                                {
                                    _21982 = _9527;
                                }
                                _21995 = (((gBackdrop3.sample(gLinear, _21979, level(0.0)) * _9504.x) + (gBackdrop3.sample(gLinear, _21980, level(0.0)) * _9507.x)) * _9504.y) + (((gBackdrop3.sample(gLinear, _21981, level(0.0)) * _9504.x) + (gBackdrop3.sample(gLinear, _21982, level(0.0)) * _9507.x)) * _9507.y);
                            }
                            else
                            {
                                float4 _21996;
                                if (_8905 == 4)
                                {
                                    float2 _9665 = (_6520 * _172.gLevel[4].xy) - float2(0.5);
                                    float2 _9667 = floor(_9665);
                                    float2 _9670 = _9665 - _9667;
                                    float2 _9673 = _9670 * _9670;
                                    float2 _9676 = _9673 * _9670;
                                    float2 _9695 = (((_9676 * 3.0) - (_9673 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _9708 = _9676 * 0.16666667163372039794921875;
                                    float2 _9711 = (((((-_9676) + (_9673 * 3.0)) - (_9670 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9695;
                                    float2 _9714 = (((((_9676 * (-3.0)) + (_9673 * 3.0)) + (_9670 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9708;
                                    float2 _9724 = ((_9667 - float2(0.5)) + (_9695 / _9711)) * _172.gLevel[4].zw;
                                    float2 _9734 = ((_9667 + float2(1.5)) + (_9708 / _9714)) * _172.gLevel[4].zw;
                                    float2 _21975;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20557 = _9724;
                                        _20557.y = 1.0 - _9724.y;
                                        _21975 = _20557;
                                    }
                                    else
                                    {
                                        _21975 = _9724;
                                    }
                                    float _9754 = _9724.y;
                                    float2 _9755 = float2(_9734.x, _9754);
                                    float2 _21976;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20563 = _9755;
                                        _20563.y = 1.0 - _9754;
                                        _21976 = _20563;
                                    }
                                    else
                                    {
                                        _21976 = _9755;
                                    }
                                    float _9771 = _9734.y;
                                    float2 _9772 = float2(_9724.x, _9771);
                                    float2 _21977;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20570 = _9772;
                                        _20570.y = 1.0 - _9771;
                                        _21977 = _20570;
                                    }
                                    else
                                    {
                                        _21977 = _9772;
                                    }
                                    float2 _21978;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20576 = _9734;
                                        _20576.y = 1.0 - _9734.y;
                                        _21978 = _20576;
                                    }
                                    else
                                    {
                                        _21978 = _9734;
                                    }
                                    _21996 = (((gBackdrop4.sample(gLinear, _21975, level(0.0)) * _9711.x) + (gBackdrop4.sample(gLinear, _21976, level(0.0)) * _9714.x)) * _9711.y) + (((gBackdrop4.sample(gLinear, _21977, level(0.0)) * _9711.x) + (gBackdrop4.sample(gLinear, _21978, level(0.0)) * _9714.x)) * _9714.y);
                                }
                                else
                                {
                                    float2 _9872 = (_6520 * _172.gLevel[5].xy) - float2(0.5);
                                    float2 _9874 = floor(_9872);
                                    float2 _9877 = _9872 - _9874;
                                    float2 _9880 = _9877 * _9877;
                                    float2 _9883 = _9880 * _9877;
                                    float2 _9902 = (((_9883 * 3.0) - (_9880 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _9915 = _9883 * 0.16666667163372039794921875;
                                    float2 _9918 = (((((-_9883) + (_9880 * 3.0)) - (_9877 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9902;
                                    float2 _9921 = (((((_9883 * (-3.0)) + (_9880 * 3.0)) + (_9877 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9915;
                                    float2 _9931 = ((_9874 - float2(0.5)) + (_9902 / _9918)) * _172.gLevel[5].zw;
                                    float2 _9941 = ((_9874 + float2(1.5)) + (_9915 / _9921)) * _172.gLevel[5].zw;
                                    float2 _21971;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20583 = _9931;
                                        _20583.y = 1.0 - _9931.y;
                                        _21971 = _20583;
                                    }
                                    else
                                    {
                                        _21971 = _9931;
                                    }
                                    float _9961 = _9931.y;
                                    float2 _9962 = float2(_9941.x, _9961);
                                    float2 _21972;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20589 = _9962;
                                        _20589.y = 1.0 - _9961;
                                        _21972 = _20589;
                                    }
                                    else
                                    {
                                        _21972 = _9962;
                                    }
                                    float _9978 = _9941.y;
                                    float2 _9979 = float2(_9931.x, _9978);
                                    float2 _21973;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20596 = _9979;
                                        _20596.y = 1.0 - _9978;
                                        _21973 = _20596;
                                    }
                                    else
                                    {
                                        _21973 = _9979;
                                    }
                                    float2 _21974;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20602 = _9941;
                                        _20602.y = 1.0 - _9941.y;
                                        _21974 = _20602;
                                    }
                                    else
                                    {
                                        _21974 = _9941;
                                    }
                                    _21996 = (((gBackdrop5.sample(gLinear, _21971, level(0.0)) * _9918.x) + (gBackdrop5.sample(gLinear, _21972, level(0.0)) * _9921.x)) * _9918.y) + (((gBackdrop5.sample(gLinear, _21973, level(0.0)) * _9918.x) + (gBackdrop5.sample(gLinear, _21974, level(0.0)) * _9921.x)) * _9921.y);
                                }
                                _21995 = _21996;
                            }
                            _21994 = _21995;
                        }
                        _21993 = _21994;
                    }
                    _21992 = _21993;
                }
                float3 _22023;
                if ((_8909 > 0.0199999995529651641845703125) && (_8905 < 5))
                {
                    int _8922 = _8905 + 1;
                    float4 _22018;
                    if (_8922 <= 0)
                    {
                        float2 _22017;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20607 = _6520;
                            _20607.y = 1.0 - _6520.y;
                            _22017 = _20607;
                        }
                        else
                        {
                            _22017 = _6520;
                        }
                        _22018 = gBackdrop0.sample(gLinear, _22017, level(0.0));
                    }
                    else
                    {
                        float4 _22019;
                        if (_8922 == 1)
                        {
                            float2 _10168 = (_6520 * _172.gLevel[1].xy) - float2(0.5);
                            float2 _10170 = floor(_10168);
                            float2 _10173 = _10168 - _10170;
                            float2 _10176 = _10173 * _10173;
                            float2 _10179 = _10176 * _10173;
                            float2 _10198 = (((_10179 * 3.0) - (_10176 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _10211 = _10179 * 0.16666667163372039794921875;
                            float2 _10214 = (((((-_10179) + (_10176 * 3.0)) - (_10173 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10198;
                            float2 _10217 = (((((_10179 * (-3.0)) + (_10176 * 3.0)) + (_10173 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10211;
                            float2 _10227 = ((_10170 - float2(0.5)) + (_10198 / _10214)) * _172.gLevel[1].zw;
                            float2 _10237 = ((_10170 + float2(1.5)) + (_10211 / _10217)) * _172.gLevel[1].zw;
                            float2 _22013;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20612 = _10227;
                                _20612.y = 1.0 - _10227.y;
                                _22013 = _20612;
                            }
                            else
                            {
                                _22013 = _10227;
                            }
                            float _10257 = _10227.y;
                            float2 _10258 = float2(_10237.x, _10257);
                            float2 _22014;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20618 = _10258;
                                _20618.y = 1.0 - _10257;
                                _22014 = _20618;
                            }
                            else
                            {
                                _22014 = _10258;
                            }
                            float _10274 = _10237.y;
                            float2 _10275 = float2(_10227.x, _10274);
                            float2 _22015;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20625 = _10275;
                                _20625.y = 1.0 - _10274;
                                _22015 = _20625;
                            }
                            else
                            {
                                _22015 = _10275;
                            }
                            float2 _22016;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20631 = _10237;
                                _20631.y = 1.0 - _10237.y;
                                _22016 = _20631;
                            }
                            else
                            {
                                _22016 = _10237;
                            }
                            _22019 = (((gBackdrop1.sample(gLinear, _22013, level(0.0)) * _10214.x) + (gBackdrop1.sample(gLinear, _22014, level(0.0)) * _10217.x)) * _10214.y) + (((gBackdrop1.sample(gLinear, _22015, level(0.0)) * _10214.x) + (gBackdrop1.sample(gLinear, _22016, level(0.0)) * _10217.x)) * _10217.y);
                        }
                        else
                        {
                            float4 _22020;
                            if (_8922 == 2)
                            {
                                float2 _10375 = (_6520 * _172.gLevel[2].xy) - float2(0.5);
                                float2 _10377 = floor(_10375);
                                float2 _10380 = _10375 - _10377;
                                float2 _10383 = _10380 * _10380;
                                float2 _10386 = _10383 * _10380;
                                float2 _10405 = (((_10386 * 3.0) - (_10383 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _10418 = _10386 * 0.16666667163372039794921875;
                                float2 _10421 = (((((-_10386) + (_10383 * 3.0)) - (_10380 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10405;
                                float2 _10424 = (((((_10386 * (-3.0)) + (_10383 * 3.0)) + (_10380 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10418;
                                float2 _10434 = ((_10377 - float2(0.5)) + (_10405 / _10421)) * _172.gLevel[2].zw;
                                float2 _10444 = ((_10377 + float2(1.5)) + (_10418 / _10424)) * _172.gLevel[2].zw;
                                float2 _22009;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20638 = _10434;
                                    _20638.y = 1.0 - _10434.y;
                                    _22009 = _20638;
                                }
                                else
                                {
                                    _22009 = _10434;
                                }
                                float _10464 = _10434.y;
                                float2 _10465 = float2(_10444.x, _10464);
                                float2 _22010;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20644 = _10465;
                                    _20644.y = 1.0 - _10464;
                                    _22010 = _20644;
                                }
                                else
                                {
                                    _22010 = _10465;
                                }
                                float _10481 = _10444.y;
                                float2 _10482 = float2(_10434.x, _10481);
                                float2 _22011;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20651 = _10482;
                                    _20651.y = 1.0 - _10481;
                                    _22011 = _20651;
                                }
                                else
                                {
                                    _22011 = _10482;
                                }
                                float2 _22012;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20657 = _10444;
                                    _20657.y = 1.0 - _10444.y;
                                    _22012 = _20657;
                                }
                                else
                                {
                                    _22012 = _10444;
                                }
                                _22020 = (((gBackdrop2.sample(gLinear, _22009, level(0.0)) * _10421.x) + (gBackdrop2.sample(gLinear, _22010, level(0.0)) * _10424.x)) * _10421.y) + (((gBackdrop2.sample(gLinear, _22011, level(0.0)) * _10421.x) + (gBackdrop2.sample(gLinear, _22012, level(0.0)) * _10424.x)) * _10424.y);
                            }
                            else
                            {
                                float4 _22021;
                                if (_8922 == 3)
                                {
                                    float2 _10582 = (_6520 * _172.gLevel[3].xy) - float2(0.5);
                                    float2 _10584 = floor(_10582);
                                    float2 _10587 = _10582 - _10584;
                                    float2 _10590 = _10587 * _10587;
                                    float2 _10593 = _10590 * _10587;
                                    float2 _10612 = (((_10593 * 3.0) - (_10590 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _10625 = _10593 * 0.16666667163372039794921875;
                                    float2 _10628 = (((((-_10593) + (_10590 * 3.0)) - (_10587 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10612;
                                    float2 _10631 = (((((_10593 * (-3.0)) + (_10590 * 3.0)) + (_10587 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10625;
                                    float2 _10641 = ((_10584 - float2(0.5)) + (_10612 / _10628)) * _172.gLevel[3].zw;
                                    float2 _10651 = ((_10584 + float2(1.5)) + (_10625 / _10631)) * _172.gLevel[3].zw;
                                    float2 _22005;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20664 = _10641;
                                        _20664.y = 1.0 - _10641.y;
                                        _22005 = _20664;
                                    }
                                    else
                                    {
                                        _22005 = _10641;
                                    }
                                    float _10671 = _10641.y;
                                    float2 _10672 = float2(_10651.x, _10671);
                                    float2 _22006;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20670 = _10672;
                                        _20670.y = 1.0 - _10671;
                                        _22006 = _20670;
                                    }
                                    else
                                    {
                                        _22006 = _10672;
                                    }
                                    float _10688 = _10651.y;
                                    float2 _10689 = float2(_10641.x, _10688);
                                    float2 _22007;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20677 = _10689;
                                        _20677.y = 1.0 - _10688;
                                        _22007 = _20677;
                                    }
                                    else
                                    {
                                        _22007 = _10689;
                                    }
                                    float2 _22008;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20683 = _10651;
                                        _20683.y = 1.0 - _10651.y;
                                        _22008 = _20683;
                                    }
                                    else
                                    {
                                        _22008 = _10651;
                                    }
                                    _22021 = (((gBackdrop3.sample(gLinear, _22005, level(0.0)) * _10628.x) + (gBackdrop3.sample(gLinear, _22006, level(0.0)) * _10631.x)) * _10628.y) + (((gBackdrop3.sample(gLinear, _22007, level(0.0)) * _10628.x) + (gBackdrop3.sample(gLinear, _22008, level(0.0)) * _10631.x)) * _10631.y);
                                }
                                else
                                {
                                    float4 _22022;
                                    if (_8922 == 4)
                                    {
                                        float2 _10789 = (_6520 * _172.gLevel[4].xy) - float2(0.5);
                                        float2 _10791 = floor(_10789);
                                        float2 _10794 = _10789 - _10791;
                                        float2 _10797 = _10794 * _10794;
                                        float2 _10800 = _10797 * _10794;
                                        float2 _10819 = (((_10800 * 3.0) - (_10797 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _10832 = _10800 * 0.16666667163372039794921875;
                                        float2 _10835 = (((((-_10800) + (_10797 * 3.0)) - (_10794 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10819;
                                        float2 _10838 = (((((_10800 * (-3.0)) + (_10797 * 3.0)) + (_10794 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _10832;
                                        float2 _10848 = ((_10791 - float2(0.5)) + (_10819 / _10835)) * _172.gLevel[4].zw;
                                        float2 _10858 = ((_10791 + float2(1.5)) + (_10832 / _10838)) * _172.gLevel[4].zw;
                                        float2 _22001;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20690 = _10848;
                                            _20690.y = 1.0 - _10848.y;
                                            _22001 = _20690;
                                        }
                                        else
                                        {
                                            _22001 = _10848;
                                        }
                                        float _10878 = _10848.y;
                                        float2 _10879 = float2(_10858.x, _10878);
                                        float2 _22002;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20696 = _10879;
                                            _20696.y = 1.0 - _10878;
                                            _22002 = _20696;
                                        }
                                        else
                                        {
                                            _22002 = _10879;
                                        }
                                        float _10895 = _10858.y;
                                        float2 _10896 = float2(_10848.x, _10895);
                                        float2 _22003;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20703 = _10896;
                                            _20703.y = 1.0 - _10895;
                                            _22003 = _20703;
                                        }
                                        else
                                        {
                                            _22003 = _10896;
                                        }
                                        float2 _22004;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20709 = _10858;
                                            _20709.y = 1.0 - _10858.y;
                                            _22004 = _20709;
                                        }
                                        else
                                        {
                                            _22004 = _10858;
                                        }
                                        _22022 = (((gBackdrop4.sample(gLinear, _22001, level(0.0)) * _10835.x) + (gBackdrop4.sample(gLinear, _22002, level(0.0)) * _10838.x)) * _10835.y) + (((gBackdrop4.sample(gLinear, _22003, level(0.0)) * _10835.x) + (gBackdrop4.sample(gLinear, _22004, level(0.0)) * _10838.x)) * _10838.y);
                                    }
                                    else
                                    {
                                        float2 _10996 = (_6520 * _172.gLevel[5].xy) - float2(0.5);
                                        float2 _10998 = floor(_10996);
                                        float2 _11001 = _10996 - _10998;
                                        float2 _11004 = _11001 * _11001;
                                        float2 _11007 = _11004 * _11001;
                                        float2 _11026 = (((_11007 * 3.0) - (_11004 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _11039 = _11007 * 0.16666667163372039794921875;
                                        float2 _11042 = (((((-_11007) + (_11004 * 3.0)) - (_11001 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11026;
                                        float2 _11045 = (((((_11007 * (-3.0)) + (_11004 * 3.0)) + (_11001 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11039;
                                        float2 _11055 = ((_10998 - float2(0.5)) + (_11026 / _11042)) * _172.gLevel[5].zw;
                                        float2 _11065 = ((_10998 + float2(1.5)) + (_11039 / _11045)) * _172.gLevel[5].zw;
                                        float2 _21997;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20716 = _11055;
                                            _20716.y = 1.0 - _11055.y;
                                            _21997 = _20716;
                                        }
                                        else
                                        {
                                            _21997 = _11055;
                                        }
                                        float _11085 = _11055.y;
                                        float2 _11086 = float2(_11065.x, _11085);
                                        float2 _21998;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20722 = _11086;
                                            _20722.y = 1.0 - _11085;
                                            _21998 = _20722;
                                        }
                                        else
                                        {
                                            _21998 = _11086;
                                        }
                                        float _11102 = _11065.y;
                                        float2 _11103 = float2(_11055.x, _11102);
                                        float2 _21999;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20729 = _11103;
                                            _20729.y = 1.0 - _11102;
                                            _21999 = _20729;
                                        }
                                        else
                                        {
                                            _21999 = _11103;
                                        }
                                        float2 _22000;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20735 = _11065;
                                            _20735.y = 1.0 - _11065.y;
                                            _22000 = _20735;
                                        }
                                        else
                                        {
                                            _22000 = _11065;
                                        }
                                        _22022 = (((gBackdrop5.sample(gLinear, _21997, level(0.0)) * _11042.x) + (gBackdrop5.sample(gLinear, _21998, level(0.0)) * _11045.x)) * _11042.y) + (((gBackdrop5.sample(gLinear, _21999, level(0.0)) * _11042.x) + (gBackdrop5.sample(gLinear, _22000, level(0.0)) * _11045.x)) * _11045.y);
                                    }
                                    _22021 = _22022;
                                }
                                _22020 = _22021;
                            }
                            _22019 = _22020;
                        }
                        _22018 = _22019;
                    }
                    _22023 = mix(_21992.xyz, _22018.xyz, float3(_8909));
                }
                else
                {
                    _22023 = _21992.xyz;
                }
                float2 _6527 = _6476 + _6488;
                float _11193 = fast::clamp(log2(fast::max(_6390, 1.0)) - 1.0, 0.0, 5.0);
                int _11196 = int(floor(_11193));
                float _11200 = _11193 - float(_11196);
                float4 _22098;
                if (_11196 <= 0)
                {
                    float2 _22097;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _20742 = _6527;
                        _20742.y = 1.0 - _6527.y;
                        _22097 = _20742;
                    }
                    else
                    {
                        _22097 = _6527;
                    }
                    _22098 = gBackdrop0.sample(gLinear, _22097, level(0.0));
                }
                else
                {
                    float4 _22099;
                    if (_11196 == 1)
                    {
                        float2 _11335 = (_6527 * _172.gLevel[1].xy) - float2(0.5);
                        float2 _11337 = floor(_11335);
                        float2 _11340 = _11335 - _11337;
                        float2 _11343 = _11340 * _11340;
                        float2 _11346 = _11343 * _11340;
                        float2 _11365 = (((_11346 * 3.0) - (_11343 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _11378 = _11346 * 0.16666667163372039794921875;
                        float2 _11381 = (((((-_11346) + (_11343 * 3.0)) - (_11340 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11365;
                        float2 _11384 = (((((_11346 * (-3.0)) + (_11343 * 3.0)) + (_11340 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11378;
                        float2 _11394 = ((_11337 - float2(0.5)) + (_11365 / _11381)) * _172.gLevel[1].zw;
                        float2 _11404 = ((_11337 + float2(1.5)) + (_11378 / _11384)) * _172.gLevel[1].zw;
                        float2 _22093;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20747 = _11394;
                            _20747.y = 1.0 - _11394.y;
                            _22093 = _20747;
                        }
                        else
                        {
                            _22093 = _11394;
                        }
                        float _11424 = _11394.y;
                        float2 _11425 = float2(_11404.x, _11424);
                        float2 _22094;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20753 = _11425;
                            _20753.y = 1.0 - _11424;
                            _22094 = _20753;
                        }
                        else
                        {
                            _22094 = _11425;
                        }
                        float _11441 = _11404.y;
                        float2 _11442 = float2(_11394.x, _11441);
                        float2 _22095;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20760 = _11442;
                            _20760.y = 1.0 - _11441;
                            _22095 = _20760;
                        }
                        else
                        {
                            _22095 = _11442;
                        }
                        float2 _22096;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20766 = _11404;
                            _20766.y = 1.0 - _11404.y;
                            _22096 = _20766;
                        }
                        else
                        {
                            _22096 = _11404;
                        }
                        _22099 = (((gBackdrop1.sample(gLinear, _22093, level(0.0)) * _11381.x) + (gBackdrop1.sample(gLinear, _22094, level(0.0)) * _11384.x)) * _11381.y) + (((gBackdrop1.sample(gLinear, _22095, level(0.0)) * _11381.x) + (gBackdrop1.sample(gLinear, _22096, level(0.0)) * _11384.x)) * _11384.y);
                    }
                    else
                    {
                        float4 _22100;
                        if (_11196 == 2)
                        {
                            float2 _11542 = (_6527 * _172.gLevel[2].xy) - float2(0.5);
                            float2 _11544 = floor(_11542);
                            float2 _11547 = _11542 - _11544;
                            float2 _11550 = _11547 * _11547;
                            float2 _11553 = _11550 * _11547;
                            float2 _11572 = (((_11553 * 3.0) - (_11550 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _11585 = _11553 * 0.16666667163372039794921875;
                            float2 _11588 = (((((-_11553) + (_11550 * 3.0)) - (_11547 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11572;
                            float2 _11591 = (((((_11553 * (-3.0)) + (_11550 * 3.0)) + (_11547 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11585;
                            float2 _11601 = ((_11544 - float2(0.5)) + (_11572 / _11588)) * _172.gLevel[2].zw;
                            float2 _11611 = ((_11544 + float2(1.5)) + (_11585 / _11591)) * _172.gLevel[2].zw;
                            float2 _22089;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20773 = _11601;
                                _20773.y = 1.0 - _11601.y;
                                _22089 = _20773;
                            }
                            else
                            {
                                _22089 = _11601;
                            }
                            float _11631 = _11601.y;
                            float2 _11632 = float2(_11611.x, _11631);
                            float2 _22090;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20779 = _11632;
                                _20779.y = 1.0 - _11631;
                                _22090 = _20779;
                            }
                            else
                            {
                                _22090 = _11632;
                            }
                            float _11648 = _11611.y;
                            float2 _11649 = float2(_11601.x, _11648);
                            float2 _22091;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20786 = _11649;
                                _20786.y = 1.0 - _11648;
                                _22091 = _20786;
                            }
                            else
                            {
                                _22091 = _11649;
                            }
                            float2 _22092;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20792 = _11611;
                                _20792.y = 1.0 - _11611.y;
                                _22092 = _20792;
                            }
                            else
                            {
                                _22092 = _11611;
                            }
                            _22100 = (((gBackdrop2.sample(gLinear, _22089, level(0.0)) * _11588.x) + (gBackdrop2.sample(gLinear, _22090, level(0.0)) * _11591.x)) * _11588.y) + (((gBackdrop2.sample(gLinear, _22091, level(0.0)) * _11588.x) + (gBackdrop2.sample(gLinear, _22092, level(0.0)) * _11591.x)) * _11591.y);
                        }
                        else
                        {
                            float4 _22101;
                            if (_11196 == 3)
                            {
                                float2 _11749 = (_6527 * _172.gLevel[3].xy) - float2(0.5);
                                float2 _11751 = floor(_11749);
                                float2 _11754 = _11749 - _11751;
                                float2 _11757 = _11754 * _11754;
                                float2 _11760 = _11757 * _11754;
                                float2 _11779 = (((_11760 * 3.0) - (_11757 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _11792 = _11760 * 0.16666667163372039794921875;
                                float2 _11795 = (((((-_11760) + (_11757 * 3.0)) - (_11754 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11779;
                                float2 _11798 = (((((_11760 * (-3.0)) + (_11757 * 3.0)) + (_11754 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11792;
                                float2 _11808 = ((_11751 - float2(0.5)) + (_11779 / _11795)) * _172.gLevel[3].zw;
                                float2 _11818 = ((_11751 + float2(1.5)) + (_11792 / _11798)) * _172.gLevel[3].zw;
                                float2 _22085;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20799 = _11808;
                                    _20799.y = 1.0 - _11808.y;
                                    _22085 = _20799;
                                }
                                else
                                {
                                    _22085 = _11808;
                                }
                                float _11838 = _11808.y;
                                float2 _11839 = float2(_11818.x, _11838);
                                float2 _22086;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20805 = _11839;
                                    _20805.y = 1.0 - _11838;
                                    _22086 = _20805;
                                }
                                else
                                {
                                    _22086 = _11839;
                                }
                                float _11855 = _11818.y;
                                float2 _11856 = float2(_11808.x, _11855);
                                float2 _22087;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20812 = _11856;
                                    _20812.y = 1.0 - _11855;
                                    _22087 = _20812;
                                }
                                else
                                {
                                    _22087 = _11856;
                                }
                                float2 _22088;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20818 = _11818;
                                    _20818.y = 1.0 - _11818.y;
                                    _22088 = _20818;
                                }
                                else
                                {
                                    _22088 = _11818;
                                }
                                _22101 = (((gBackdrop3.sample(gLinear, _22085, level(0.0)) * _11795.x) + (gBackdrop3.sample(gLinear, _22086, level(0.0)) * _11798.x)) * _11795.y) + (((gBackdrop3.sample(gLinear, _22087, level(0.0)) * _11795.x) + (gBackdrop3.sample(gLinear, _22088, level(0.0)) * _11798.x)) * _11798.y);
                            }
                            else
                            {
                                float4 _22102;
                                if (_11196 == 4)
                                {
                                    float2 _11956 = (_6527 * _172.gLevel[4].xy) - float2(0.5);
                                    float2 _11958 = floor(_11956);
                                    float2 _11961 = _11956 - _11958;
                                    float2 _11964 = _11961 * _11961;
                                    float2 _11967 = _11964 * _11961;
                                    float2 _11986 = (((_11967 * 3.0) - (_11964 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _11999 = _11967 * 0.16666667163372039794921875;
                                    float2 _12002 = (((((-_11967) + (_11964 * 3.0)) - (_11961 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11986;
                                    float2 _12005 = (((((_11967 * (-3.0)) + (_11964 * 3.0)) + (_11961 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _11999;
                                    float2 _12015 = ((_11958 - float2(0.5)) + (_11986 / _12002)) * _172.gLevel[4].zw;
                                    float2 _12025 = ((_11958 + float2(1.5)) + (_11999 / _12005)) * _172.gLevel[4].zw;
                                    float2 _22081;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20825 = _12015;
                                        _20825.y = 1.0 - _12015.y;
                                        _22081 = _20825;
                                    }
                                    else
                                    {
                                        _22081 = _12015;
                                    }
                                    float _12045 = _12015.y;
                                    float2 _12046 = float2(_12025.x, _12045);
                                    float2 _22082;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20831 = _12046;
                                        _20831.y = 1.0 - _12045;
                                        _22082 = _20831;
                                    }
                                    else
                                    {
                                        _22082 = _12046;
                                    }
                                    float _12062 = _12025.y;
                                    float2 _12063 = float2(_12015.x, _12062);
                                    float2 _22083;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20838 = _12063;
                                        _20838.y = 1.0 - _12062;
                                        _22083 = _20838;
                                    }
                                    else
                                    {
                                        _22083 = _12063;
                                    }
                                    float2 _22084;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20844 = _12025;
                                        _20844.y = 1.0 - _12025.y;
                                        _22084 = _20844;
                                    }
                                    else
                                    {
                                        _22084 = _12025;
                                    }
                                    _22102 = (((gBackdrop4.sample(gLinear, _22081, level(0.0)) * _12002.x) + (gBackdrop4.sample(gLinear, _22082, level(0.0)) * _12005.x)) * _12002.y) + (((gBackdrop4.sample(gLinear, _22083, level(0.0)) * _12002.x) + (gBackdrop4.sample(gLinear, _22084, level(0.0)) * _12005.x)) * _12005.y);
                                }
                                else
                                {
                                    float2 _12163 = (_6527 * _172.gLevel[5].xy) - float2(0.5);
                                    float2 _12165 = floor(_12163);
                                    float2 _12168 = _12163 - _12165;
                                    float2 _12171 = _12168 * _12168;
                                    float2 _12174 = _12171 * _12168;
                                    float2 _12193 = (((_12174 * 3.0) - (_12171 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _12206 = _12174 * 0.16666667163372039794921875;
                                    float2 _12209 = (((((-_12174) + (_12171 * 3.0)) - (_12168 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12193;
                                    float2 _12212 = (((((_12174 * (-3.0)) + (_12171 * 3.0)) + (_12168 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12206;
                                    float2 _12222 = ((_12165 - float2(0.5)) + (_12193 / _12209)) * _172.gLevel[5].zw;
                                    float2 _12232 = ((_12165 + float2(1.5)) + (_12206 / _12212)) * _172.gLevel[5].zw;
                                    float2 _22077;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20851 = _12222;
                                        _20851.y = 1.0 - _12222.y;
                                        _22077 = _20851;
                                    }
                                    else
                                    {
                                        _22077 = _12222;
                                    }
                                    float _12252 = _12222.y;
                                    float2 _12253 = float2(_12232.x, _12252);
                                    float2 _22078;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20857 = _12253;
                                        _20857.y = 1.0 - _12252;
                                        _22078 = _20857;
                                    }
                                    else
                                    {
                                        _22078 = _12253;
                                    }
                                    float _12269 = _12232.y;
                                    float2 _12270 = float2(_12222.x, _12269);
                                    float2 _22079;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20864 = _12270;
                                        _20864.y = 1.0 - _12269;
                                        _22079 = _20864;
                                    }
                                    else
                                    {
                                        _22079 = _12270;
                                    }
                                    float2 _22080;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20870 = _12232;
                                        _20870.y = 1.0 - _12232.y;
                                        _22080 = _20870;
                                    }
                                    else
                                    {
                                        _22080 = _12232;
                                    }
                                    _22102 = (((gBackdrop5.sample(gLinear, _22077, level(0.0)) * _12209.x) + (gBackdrop5.sample(gLinear, _22078, level(0.0)) * _12212.x)) * _12209.y) + (((gBackdrop5.sample(gLinear, _22079, level(0.0)) * _12209.x) + (gBackdrop5.sample(gLinear, _22080, level(0.0)) * _12212.x)) * _12212.y);
                                }
                                _22101 = _22102;
                            }
                            _22100 = _22101;
                        }
                        _22099 = _22100;
                    }
                    _22098 = _22099;
                }
                float3 _22129;
                if ((_11200 > 0.0199999995529651641845703125) && (_11196 < 5))
                {
                    int _11213 = _11196 + 1;
                    float4 _22124;
                    if (_11213 <= 0)
                    {
                        float2 _22123;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _20875 = _6527;
                            _20875.y = 1.0 - _6527.y;
                            _22123 = _20875;
                        }
                        else
                        {
                            _22123 = _6527;
                        }
                        _22124 = gBackdrop0.sample(gLinear, _22123, level(0.0));
                    }
                    else
                    {
                        float4 _22125;
                        if (_11213 == 1)
                        {
                            float2 _12459 = (_6527 * _172.gLevel[1].xy) - float2(0.5);
                            float2 _12461 = floor(_12459);
                            float2 _12464 = _12459 - _12461;
                            float2 _12467 = _12464 * _12464;
                            float2 _12470 = _12467 * _12464;
                            float2 _12489 = (((_12470 * 3.0) - (_12467 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _12502 = _12470 * 0.16666667163372039794921875;
                            float2 _12505 = (((((-_12470) + (_12467 * 3.0)) - (_12464 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12489;
                            float2 _12508 = (((((_12470 * (-3.0)) + (_12467 * 3.0)) + (_12464 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12502;
                            float2 _12518 = ((_12461 - float2(0.5)) + (_12489 / _12505)) * _172.gLevel[1].zw;
                            float2 _12528 = ((_12461 + float2(1.5)) + (_12502 / _12508)) * _172.gLevel[1].zw;
                            float2 _22119;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20880 = _12518;
                                _20880.y = 1.0 - _12518.y;
                                _22119 = _20880;
                            }
                            else
                            {
                                _22119 = _12518;
                            }
                            float _12548 = _12518.y;
                            float2 _12549 = float2(_12528.x, _12548);
                            float2 _22120;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20886 = _12549;
                                _20886.y = 1.0 - _12548;
                                _22120 = _20886;
                            }
                            else
                            {
                                _22120 = _12549;
                            }
                            float _12565 = _12528.y;
                            float2 _12566 = float2(_12518.x, _12565);
                            float2 _22121;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20893 = _12566;
                                _20893.y = 1.0 - _12565;
                                _22121 = _20893;
                            }
                            else
                            {
                                _22121 = _12566;
                            }
                            float2 _22122;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _20899 = _12528;
                                _20899.y = 1.0 - _12528.y;
                                _22122 = _20899;
                            }
                            else
                            {
                                _22122 = _12528;
                            }
                            _22125 = (((gBackdrop1.sample(gLinear, _22119, level(0.0)) * _12505.x) + (gBackdrop1.sample(gLinear, _22120, level(0.0)) * _12508.x)) * _12505.y) + (((gBackdrop1.sample(gLinear, _22121, level(0.0)) * _12505.x) + (gBackdrop1.sample(gLinear, _22122, level(0.0)) * _12508.x)) * _12508.y);
                        }
                        else
                        {
                            float4 _22126;
                            if (_11213 == 2)
                            {
                                float2 _12666 = (_6527 * _172.gLevel[2].xy) - float2(0.5);
                                float2 _12668 = floor(_12666);
                                float2 _12671 = _12666 - _12668;
                                float2 _12674 = _12671 * _12671;
                                float2 _12677 = _12674 * _12671;
                                float2 _12696 = (((_12677 * 3.0) - (_12674 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _12709 = _12677 * 0.16666667163372039794921875;
                                float2 _12712 = (((((-_12677) + (_12674 * 3.0)) - (_12671 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12696;
                                float2 _12715 = (((((_12677 * (-3.0)) + (_12674 * 3.0)) + (_12671 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12709;
                                float2 _12725 = ((_12668 - float2(0.5)) + (_12696 / _12712)) * _172.gLevel[2].zw;
                                float2 _12735 = ((_12668 + float2(1.5)) + (_12709 / _12715)) * _172.gLevel[2].zw;
                                float2 _22115;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20906 = _12725;
                                    _20906.y = 1.0 - _12725.y;
                                    _22115 = _20906;
                                }
                                else
                                {
                                    _22115 = _12725;
                                }
                                float _12755 = _12725.y;
                                float2 _12756 = float2(_12735.x, _12755);
                                float2 _22116;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20912 = _12756;
                                    _20912.y = 1.0 - _12755;
                                    _22116 = _20912;
                                }
                                else
                                {
                                    _22116 = _12756;
                                }
                                float _12772 = _12735.y;
                                float2 _12773 = float2(_12725.x, _12772);
                                float2 _22117;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20919 = _12773;
                                    _20919.y = 1.0 - _12772;
                                    _22117 = _20919;
                                }
                                else
                                {
                                    _22117 = _12773;
                                }
                                float2 _22118;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _20925 = _12735;
                                    _20925.y = 1.0 - _12735.y;
                                    _22118 = _20925;
                                }
                                else
                                {
                                    _22118 = _12735;
                                }
                                _22126 = (((gBackdrop2.sample(gLinear, _22115, level(0.0)) * _12712.x) + (gBackdrop2.sample(gLinear, _22116, level(0.0)) * _12715.x)) * _12712.y) + (((gBackdrop2.sample(gLinear, _22117, level(0.0)) * _12712.x) + (gBackdrop2.sample(gLinear, _22118, level(0.0)) * _12715.x)) * _12715.y);
                            }
                            else
                            {
                                float4 _22127;
                                if (_11213 == 3)
                                {
                                    float2 _12873 = (_6527 * _172.gLevel[3].xy) - float2(0.5);
                                    float2 _12875 = floor(_12873);
                                    float2 _12878 = _12873 - _12875;
                                    float2 _12881 = _12878 * _12878;
                                    float2 _12884 = _12881 * _12878;
                                    float2 _12903 = (((_12884 * 3.0) - (_12881 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _12916 = _12884 * 0.16666667163372039794921875;
                                    float2 _12919 = (((((-_12884) + (_12881 * 3.0)) - (_12878 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12903;
                                    float2 _12922 = (((((_12884 * (-3.0)) + (_12881 * 3.0)) + (_12878 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _12916;
                                    float2 _12932 = ((_12875 - float2(0.5)) + (_12903 / _12919)) * _172.gLevel[3].zw;
                                    float2 _12942 = ((_12875 + float2(1.5)) + (_12916 / _12922)) * _172.gLevel[3].zw;
                                    float2 _22111;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20932 = _12932;
                                        _20932.y = 1.0 - _12932.y;
                                        _22111 = _20932;
                                    }
                                    else
                                    {
                                        _22111 = _12932;
                                    }
                                    float _12962 = _12932.y;
                                    float2 _12963 = float2(_12942.x, _12962);
                                    float2 _22112;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20938 = _12963;
                                        _20938.y = 1.0 - _12962;
                                        _22112 = _20938;
                                    }
                                    else
                                    {
                                        _22112 = _12963;
                                    }
                                    float _12979 = _12942.y;
                                    float2 _12980 = float2(_12932.x, _12979);
                                    float2 _22113;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20945 = _12980;
                                        _20945.y = 1.0 - _12979;
                                        _22113 = _20945;
                                    }
                                    else
                                    {
                                        _22113 = _12980;
                                    }
                                    float2 _22114;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _20951 = _12942;
                                        _20951.y = 1.0 - _12942.y;
                                        _22114 = _20951;
                                    }
                                    else
                                    {
                                        _22114 = _12942;
                                    }
                                    _22127 = (((gBackdrop3.sample(gLinear, _22111, level(0.0)) * _12919.x) + (gBackdrop3.sample(gLinear, _22112, level(0.0)) * _12922.x)) * _12919.y) + (((gBackdrop3.sample(gLinear, _22113, level(0.0)) * _12919.x) + (gBackdrop3.sample(gLinear, _22114, level(0.0)) * _12922.x)) * _12922.y);
                                }
                                else
                                {
                                    float4 _22128;
                                    if (_11213 == 4)
                                    {
                                        float2 _13080 = (_6527 * _172.gLevel[4].xy) - float2(0.5);
                                        float2 _13082 = floor(_13080);
                                        float2 _13085 = _13080 - _13082;
                                        float2 _13088 = _13085 * _13085;
                                        float2 _13091 = _13088 * _13085;
                                        float2 _13110 = (((_13091 * 3.0) - (_13088 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _13123 = _13091 * 0.16666667163372039794921875;
                                        float2 _13126 = (((((-_13091) + (_13088 * 3.0)) - (_13085 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13110;
                                        float2 _13129 = (((((_13091 * (-3.0)) + (_13088 * 3.0)) + (_13085 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13123;
                                        float2 _13139 = ((_13082 - float2(0.5)) + (_13110 / _13126)) * _172.gLevel[4].zw;
                                        float2 _13149 = ((_13082 + float2(1.5)) + (_13123 / _13129)) * _172.gLevel[4].zw;
                                        float2 _22107;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20958 = _13139;
                                            _20958.y = 1.0 - _13139.y;
                                            _22107 = _20958;
                                        }
                                        else
                                        {
                                            _22107 = _13139;
                                        }
                                        float _13169 = _13139.y;
                                        float2 _13170 = float2(_13149.x, _13169);
                                        float2 _22108;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20964 = _13170;
                                            _20964.y = 1.0 - _13169;
                                            _22108 = _20964;
                                        }
                                        else
                                        {
                                            _22108 = _13170;
                                        }
                                        float _13186 = _13149.y;
                                        float2 _13187 = float2(_13139.x, _13186);
                                        float2 _22109;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20971 = _13187;
                                            _20971.y = 1.0 - _13186;
                                            _22109 = _20971;
                                        }
                                        else
                                        {
                                            _22109 = _13187;
                                        }
                                        float2 _22110;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20977 = _13149;
                                            _20977.y = 1.0 - _13149.y;
                                            _22110 = _20977;
                                        }
                                        else
                                        {
                                            _22110 = _13149;
                                        }
                                        _22128 = (((gBackdrop4.sample(gLinear, _22107, level(0.0)) * _13126.x) + (gBackdrop4.sample(gLinear, _22108, level(0.0)) * _13129.x)) * _13126.y) + (((gBackdrop4.sample(gLinear, _22109, level(0.0)) * _13126.x) + (gBackdrop4.sample(gLinear, _22110, level(0.0)) * _13129.x)) * _13129.y);
                                    }
                                    else
                                    {
                                        float2 _13287 = (_6527 * _172.gLevel[5].xy) - float2(0.5);
                                        float2 _13289 = floor(_13287);
                                        float2 _13292 = _13287 - _13289;
                                        float2 _13295 = _13292 * _13292;
                                        float2 _13298 = _13295 * _13292;
                                        float2 _13317 = (((_13298 * 3.0) - (_13295 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _13330 = _13298 * 0.16666667163372039794921875;
                                        float2 _13333 = (((((-_13298) + (_13295 * 3.0)) - (_13292 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13317;
                                        float2 _13336 = (((((_13298 * (-3.0)) + (_13295 * 3.0)) + (_13292 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13330;
                                        float2 _13346 = ((_13289 - float2(0.5)) + (_13317 / _13333)) * _172.gLevel[5].zw;
                                        float2 _13356 = ((_13289 + float2(1.5)) + (_13330 / _13336)) * _172.gLevel[5].zw;
                                        float2 _22103;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20984 = _13346;
                                            _20984.y = 1.0 - _13346.y;
                                            _22103 = _20984;
                                        }
                                        else
                                        {
                                            _22103 = _13346;
                                        }
                                        float _13376 = _13346.y;
                                        float2 _13377 = float2(_13356.x, _13376);
                                        float2 _22104;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20990 = _13377;
                                            _20990.y = 1.0 - _13376;
                                            _22104 = _20990;
                                        }
                                        else
                                        {
                                            _22104 = _13377;
                                        }
                                        float _13393 = _13356.y;
                                        float2 _13394 = float2(_13346.x, _13393);
                                        float2 _22105;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _20997 = _13394;
                                            _20997.y = 1.0 - _13393;
                                            _22105 = _20997;
                                        }
                                        else
                                        {
                                            _22105 = _13394;
                                        }
                                        float2 _22106;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21003 = _13356;
                                            _21003.y = 1.0 - _13356.y;
                                            _22106 = _21003;
                                        }
                                        else
                                        {
                                            _22106 = _13356;
                                        }
                                        _22128 = (((gBackdrop5.sample(gLinear, _22103, level(0.0)) * _13333.x) + (gBackdrop5.sample(gLinear, _22104, level(0.0)) * _13336.x)) * _13333.y) + (((gBackdrop5.sample(gLinear, _22105, level(0.0)) * _13333.x) + (gBackdrop5.sample(gLinear, _22106, level(0.0)) * _13336.x)) * _13336.y);
                                    }
                                    _22127 = _22128;
                                }
                                _22126 = _22127;
                            }
                            _22125 = _22126;
                        }
                        _22124 = _22125;
                    }
                    _22129 = mix(_22098.xyz, _22124.xyz, float3(_11200));
                }
                else
                {
                    _22129 = _22098.xyz;
                }
                float2 _6538 = (_6476 + _6488) + (_6491 * _6513);
                float _13484 = fast::clamp(log2(fast::max(_6390, 1.0)) - 1.0, 0.0, 5.0);
                int _13487 = int(floor(_13484));
                float _13491 = _13484 - float(_13487);
                float4 _22204;
                if (_13487 <= 0)
                {
                    float2 _22203;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _21010 = _6538;
                        _21010.y = 1.0 - _6538.y;
                        _22203 = _21010;
                    }
                    else
                    {
                        _22203 = _6538;
                    }
                    _22204 = gBackdrop0.sample(gLinear, _22203, level(0.0));
                }
                else
                {
                    float4 _22205;
                    if (_13487 == 1)
                    {
                        float2 _13626 = (_6538 * _172.gLevel[1].xy) - float2(0.5);
                        float2 _13628 = floor(_13626);
                        float2 _13631 = _13626 - _13628;
                        float2 _13634 = _13631 * _13631;
                        float2 _13637 = _13634 * _13631;
                        float2 _13656 = (((_13637 * 3.0) - (_13634 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _13669 = _13637 * 0.16666667163372039794921875;
                        float2 _13672 = (((((-_13637) + (_13634 * 3.0)) - (_13631 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13656;
                        float2 _13675 = (((((_13637 * (-3.0)) + (_13634 * 3.0)) + (_13631 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13669;
                        float2 _13685 = ((_13628 - float2(0.5)) + (_13656 / _13672)) * _172.gLevel[1].zw;
                        float2 _13695 = ((_13628 + float2(1.5)) + (_13669 / _13675)) * _172.gLevel[1].zw;
                        float2 _22199;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21015 = _13685;
                            _21015.y = 1.0 - _13685.y;
                            _22199 = _21015;
                        }
                        else
                        {
                            _22199 = _13685;
                        }
                        float _13715 = _13685.y;
                        float2 _13716 = float2(_13695.x, _13715);
                        float2 _22200;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21021 = _13716;
                            _21021.y = 1.0 - _13715;
                            _22200 = _21021;
                        }
                        else
                        {
                            _22200 = _13716;
                        }
                        float _13732 = _13695.y;
                        float2 _13733 = float2(_13685.x, _13732);
                        float2 _22201;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21028 = _13733;
                            _21028.y = 1.0 - _13732;
                            _22201 = _21028;
                        }
                        else
                        {
                            _22201 = _13733;
                        }
                        float2 _22202;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21034 = _13695;
                            _21034.y = 1.0 - _13695.y;
                            _22202 = _21034;
                        }
                        else
                        {
                            _22202 = _13695;
                        }
                        _22205 = (((gBackdrop1.sample(gLinear, _22199, level(0.0)) * _13672.x) + (gBackdrop1.sample(gLinear, _22200, level(0.0)) * _13675.x)) * _13672.y) + (((gBackdrop1.sample(gLinear, _22201, level(0.0)) * _13672.x) + (gBackdrop1.sample(gLinear, _22202, level(0.0)) * _13675.x)) * _13675.y);
                    }
                    else
                    {
                        float4 _22206;
                        if (_13487 == 2)
                        {
                            float2 _13833 = (_6538 * _172.gLevel[2].xy) - float2(0.5);
                            float2 _13835 = floor(_13833);
                            float2 _13838 = _13833 - _13835;
                            float2 _13841 = _13838 * _13838;
                            float2 _13844 = _13841 * _13838;
                            float2 _13863 = (((_13844 * 3.0) - (_13841 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _13876 = _13844 * 0.16666667163372039794921875;
                            float2 _13879 = (((((-_13844) + (_13841 * 3.0)) - (_13838 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13863;
                            float2 _13882 = (((((_13844 * (-3.0)) + (_13841 * 3.0)) + (_13838 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13876;
                            float2 _13892 = ((_13835 - float2(0.5)) + (_13863 / _13879)) * _172.gLevel[2].zw;
                            float2 _13902 = ((_13835 + float2(1.5)) + (_13876 / _13882)) * _172.gLevel[2].zw;
                            float2 _22195;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21041 = _13892;
                                _21041.y = 1.0 - _13892.y;
                                _22195 = _21041;
                            }
                            else
                            {
                                _22195 = _13892;
                            }
                            float _13922 = _13892.y;
                            float2 _13923 = float2(_13902.x, _13922);
                            float2 _22196;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21047 = _13923;
                                _21047.y = 1.0 - _13922;
                                _22196 = _21047;
                            }
                            else
                            {
                                _22196 = _13923;
                            }
                            float _13939 = _13902.y;
                            float2 _13940 = float2(_13892.x, _13939);
                            float2 _22197;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21054 = _13940;
                                _21054.y = 1.0 - _13939;
                                _22197 = _21054;
                            }
                            else
                            {
                                _22197 = _13940;
                            }
                            float2 _22198;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21060 = _13902;
                                _21060.y = 1.0 - _13902.y;
                                _22198 = _21060;
                            }
                            else
                            {
                                _22198 = _13902;
                            }
                            _22206 = (((gBackdrop2.sample(gLinear, _22195, level(0.0)) * _13879.x) + (gBackdrop2.sample(gLinear, _22196, level(0.0)) * _13882.x)) * _13879.y) + (((gBackdrop2.sample(gLinear, _22197, level(0.0)) * _13879.x) + (gBackdrop2.sample(gLinear, _22198, level(0.0)) * _13882.x)) * _13882.y);
                        }
                        else
                        {
                            float4 _22207;
                            if (_13487 == 3)
                            {
                                float2 _14040 = (_6538 * _172.gLevel[3].xy) - float2(0.5);
                                float2 _14042 = floor(_14040);
                                float2 _14045 = _14040 - _14042;
                                float2 _14048 = _14045 * _14045;
                                float2 _14051 = _14048 * _14045;
                                float2 _14070 = (((_14051 * 3.0) - (_14048 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _14083 = _14051 * 0.16666667163372039794921875;
                                float2 _14086 = (((((-_14051) + (_14048 * 3.0)) - (_14045 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14070;
                                float2 _14089 = (((((_14051 * (-3.0)) + (_14048 * 3.0)) + (_14045 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14083;
                                float2 _14099 = ((_14042 - float2(0.5)) + (_14070 / _14086)) * _172.gLevel[3].zw;
                                float2 _14109 = ((_14042 + float2(1.5)) + (_14083 / _14089)) * _172.gLevel[3].zw;
                                float2 _22191;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21067 = _14099;
                                    _21067.y = 1.0 - _14099.y;
                                    _22191 = _21067;
                                }
                                else
                                {
                                    _22191 = _14099;
                                }
                                float _14129 = _14099.y;
                                float2 _14130 = float2(_14109.x, _14129);
                                float2 _22192;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21073 = _14130;
                                    _21073.y = 1.0 - _14129;
                                    _22192 = _21073;
                                }
                                else
                                {
                                    _22192 = _14130;
                                }
                                float _14146 = _14109.y;
                                float2 _14147 = float2(_14099.x, _14146);
                                float2 _22193;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21080 = _14147;
                                    _21080.y = 1.0 - _14146;
                                    _22193 = _21080;
                                }
                                else
                                {
                                    _22193 = _14147;
                                }
                                float2 _22194;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21086 = _14109;
                                    _21086.y = 1.0 - _14109.y;
                                    _22194 = _21086;
                                }
                                else
                                {
                                    _22194 = _14109;
                                }
                                _22207 = (((gBackdrop3.sample(gLinear, _22191, level(0.0)) * _14086.x) + (gBackdrop3.sample(gLinear, _22192, level(0.0)) * _14089.x)) * _14086.y) + (((gBackdrop3.sample(gLinear, _22193, level(0.0)) * _14086.x) + (gBackdrop3.sample(gLinear, _22194, level(0.0)) * _14089.x)) * _14089.y);
                            }
                            else
                            {
                                float4 _22208;
                                if (_13487 == 4)
                                {
                                    float2 _14247 = (_6538 * _172.gLevel[4].xy) - float2(0.5);
                                    float2 _14249 = floor(_14247);
                                    float2 _14252 = _14247 - _14249;
                                    float2 _14255 = _14252 * _14252;
                                    float2 _14258 = _14255 * _14252;
                                    float2 _14277 = (((_14258 * 3.0) - (_14255 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _14290 = _14258 * 0.16666667163372039794921875;
                                    float2 _14293 = (((((-_14258) + (_14255 * 3.0)) - (_14252 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14277;
                                    float2 _14296 = (((((_14258 * (-3.0)) + (_14255 * 3.0)) + (_14252 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14290;
                                    float2 _14306 = ((_14249 - float2(0.5)) + (_14277 / _14293)) * _172.gLevel[4].zw;
                                    float2 _14316 = ((_14249 + float2(1.5)) + (_14290 / _14296)) * _172.gLevel[4].zw;
                                    float2 _22187;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21093 = _14306;
                                        _21093.y = 1.0 - _14306.y;
                                        _22187 = _21093;
                                    }
                                    else
                                    {
                                        _22187 = _14306;
                                    }
                                    float _14336 = _14306.y;
                                    float2 _14337 = float2(_14316.x, _14336);
                                    float2 _22188;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21099 = _14337;
                                        _21099.y = 1.0 - _14336;
                                        _22188 = _21099;
                                    }
                                    else
                                    {
                                        _22188 = _14337;
                                    }
                                    float _14353 = _14316.y;
                                    float2 _14354 = float2(_14306.x, _14353);
                                    float2 _22189;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21106 = _14354;
                                        _21106.y = 1.0 - _14353;
                                        _22189 = _21106;
                                    }
                                    else
                                    {
                                        _22189 = _14354;
                                    }
                                    float2 _22190;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21112 = _14316;
                                        _21112.y = 1.0 - _14316.y;
                                        _22190 = _21112;
                                    }
                                    else
                                    {
                                        _22190 = _14316;
                                    }
                                    _22208 = (((gBackdrop4.sample(gLinear, _22187, level(0.0)) * _14293.x) + (gBackdrop4.sample(gLinear, _22188, level(0.0)) * _14296.x)) * _14293.y) + (((gBackdrop4.sample(gLinear, _22189, level(0.0)) * _14293.x) + (gBackdrop4.sample(gLinear, _22190, level(0.0)) * _14296.x)) * _14296.y);
                                }
                                else
                                {
                                    float2 _14454 = (_6538 * _172.gLevel[5].xy) - float2(0.5);
                                    float2 _14456 = floor(_14454);
                                    float2 _14459 = _14454 - _14456;
                                    float2 _14462 = _14459 * _14459;
                                    float2 _14465 = _14462 * _14459;
                                    float2 _14484 = (((_14465 * 3.0) - (_14462 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _14497 = _14465 * 0.16666667163372039794921875;
                                    float2 _14500 = (((((-_14465) + (_14462 * 3.0)) - (_14459 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14484;
                                    float2 _14503 = (((((_14465 * (-3.0)) + (_14462 * 3.0)) + (_14459 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14497;
                                    float2 _14513 = ((_14456 - float2(0.5)) + (_14484 / _14500)) * _172.gLevel[5].zw;
                                    float2 _14523 = ((_14456 + float2(1.5)) + (_14497 / _14503)) * _172.gLevel[5].zw;
                                    float2 _22183;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21119 = _14513;
                                        _21119.y = 1.0 - _14513.y;
                                        _22183 = _21119;
                                    }
                                    else
                                    {
                                        _22183 = _14513;
                                    }
                                    float _14543 = _14513.y;
                                    float2 _14544 = float2(_14523.x, _14543);
                                    float2 _22184;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21125 = _14544;
                                        _21125.y = 1.0 - _14543;
                                        _22184 = _21125;
                                    }
                                    else
                                    {
                                        _22184 = _14544;
                                    }
                                    float _14560 = _14523.y;
                                    float2 _14561 = float2(_14513.x, _14560);
                                    float2 _22185;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21132 = _14561;
                                        _21132.y = 1.0 - _14560;
                                        _22185 = _21132;
                                    }
                                    else
                                    {
                                        _22185 = _14561;
                                    }
                                    float2 _22186;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21138 = _14523;
                                        _21138.y = 1.0 - _14523.y;
                                        _22186 = _21138;
                                    }
                                    else
                                    {
                                        _22186 = _14523;
                                    }
                                    _22208 = (((gBackdrop5.sample(gLinear, _22183, level(0.0)) * _14500.x) + (gBackdrop5.sample(gLinear, _22184, level(0.0)) * _14503.x)) * _14500.y) + (((gBackdrop5.sample(gLinear, _22185, level(0.0)) * _14500.x) + (gBackdrop5.sample(gLinear, _22186, level(0.0)) * _14503.x)) * _14503.y);
                                }
                                _22207 = _22208;
                            }
                            _22206 = _22207;
                        }
                        _22205 = _22206;
                    }
                    _22204 = _22205;
                }
                float3 _22235;
                if ((_13491 > 0.0199999995529651641845703125) && (_13487 < 5))
                {
                    int _13504 = _13487 + 1;
                    float4 _22230;
                    if (_13504 <= 0)
                    {
                        float2 _22229;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21143 = _6538;
                            _21143.y = 1.0 - _6538.y;
                            _22229 = _21143;
                        }
                        else
                        {
                            _22229 = _6538;
                        }
                        _22230 = gBackdrop0.sample(gLinear, _22229, level(0.0));
                    }
                    else
                    {
                        float4 _22231;
                        if (_13504 == 1)
                        {
                            float2 _14750 = (_6538 * _172.gLevel[1].xy) - float2(0.5);
                            float2 _14752 = floor(_14750);
                            float2 _14755 = _14750 - _14752;
                            float2 _14758 = _14755 * _14755;
                            float2 _14761 = _14758 * _14755;
                            float2 _14780 = (((_14761 * 3.0) - (_14758 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _14793 = _14761 * 0.16666667163372039794921875;
                            float2 _14796 = (((((-_14761) + (_14758 * 3.0)) - (_14755 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14780;
                            float2 _14799 = (((((_14761 * (-3.0)) + (_14758 * 3.0)) + (_14755 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14793;
                            float2 _14809 = ((_14752 - float2(0.5)) + (_14780 / _14796)) * _172.gLevel[1].zw;
                            float2 _14819 = ((_14752 + float2(1.5)) + (_14793 / _14799)) * _172.gLevel[1].zw;
                            float2 _22225;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21148 = _14809;
                                _21148.y = 1.0 - _14809.y;
                                _22225 = _21148;
                            }
                            else
                            {
                                _22225 = _14809;
                            }
                            float _14839 = _14809.y;
                            float2 _14840 = float2(_14819.x, _14839);
                            float2 _22226;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21154 = _14840;
                                _21154.y = 1.0 - _14839;
                                _22226 = _21154;
                            }
                            else
                            {
                                _22226 = _14840;
                            }
                            float _14856 = _14819.y;
                            float2 _14857 = float2(_14809.x, _14856);
                            float2 _22227;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21161 = _14857;
                                _21161.y = 1.0 - _14856;
                                _22227 = _21161;
                            }
                            else
                            {
                                _22227 = _14857;
                            }
                            float2 _22228;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21167 = _14819;
                                _21167.y = 1.0 - _14819.y;
                                _22228 = _21167;
                            }
                            else
                            {
                                _22228 = _14819;
                            }
                            _22231 = (((gBackdrop1.sample(gLinear, _22225, level(0.0)) * _14796.x) + (gBackdrop1.sample(gLinear, _22226, level(0.0)) * _14799.x)) * _14796.y) + (((gBackdrop1.sample(gLinear, _22227, level(0.0)) * _14796.x) + (gBackdrop1.sample(gLinear, _22228, level(0.0)) * _14799.x)) * _14799.y);
                        }
                        else
                        {
                            float4 _22232;
                            if (_13504 == 2)
                            {
                                float2 _14957 = (_6538 * _172.gLevel[2].xy) - float2(0.5);
                                float2 _14959 = floor(_14957);
                                float2 _14962 = _14957 - _14959;
                                float2 _14965 = _14962 * _14962;
                                float2 _14968 = _14965 * _14962;
                                float2 _14987 = (((_14968 * 3.0) - (_14965 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _15000 = _14968 * 0.16666667163372039794921875;
                                float2 _15003 = (((((-_14968) + (_14965 * 3.0)) - (_14962 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14987;
                                float2 _15006 = (((((_14968 * (-3.0)) + (_14965 * 3.0)) + (_14962 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15000;
                                float2 _15016 = ((_14959 - float2(0.5)) + (_14987 / _15003)) * _172.gLevel[2].zw;
                                float2 _15026 = ((_14959 + float2(1.5)) + (_15000 / _15006)) * _172.gLevel[2].zw;
                                float2 _22221;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21174 = _15016;
                                    _21174.y = 1.0 - _15016.y;
                                    _22221 = _21174;
                                }
                                else
                                {
                                    _22221 = _15016;
                                }
                                float _15046 = _15016.y;
                                float2 _15047 = float2(_15026.x, _15046);
                                float2 _22222;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21180 = _15047;
                                    _21180.y = 1.0 - _15046;
                                    _22222 = _21180;
                                }
                                else
                                {
                                    _22222 = _15047;
                                }
                                float _15063 = _15026.y;
                                float2 _15064 = float2(_15016.x, _15063);
                                float2 _22223;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21187 = _15064;
                                    _21187.y = 1.0 - _15063;
                                    _22223 = _21187;
                                }
                                else
                                {
                                    _22223 = _15064;
                                }
                                float2 _22224;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21193 = _15026;
                                    _21193.y = 1.0 - _15026.y;
                                    _22224 = _21193;
                                }
                                else
                                {
                                    _22224 = _15026;
                                }
                                _22232 = (((gBackdrop2.sample(gLinear, _22221, level(0.0)) * _15003.x) + (gBackdrop2.sample(gLinear, _22222, level(0.0)) * _15006.x)) * _15003.y) + (((gBackdrop2.sample(gLinear, _22223, level(0.0)) * _15003.x) + (gBackdrop2.sample(gLinear, _22224, level(0.0)) * _15006.x)) * _15006.y);
                            }
                            else
                            {
                                float4 _22233;
                                if (_13504 == 3)
                                {
                                    float2 _15164 = (_6538 * _172.gLevel[3].xy) - float2(0.5);
                                    float2 _15166 = floor(_15164);
                                    float2 _15169 = _15164 - _15166;
                                    float2 _15172 = _15169 * _15169;
                                    float2 _15175 = _15172 * _15169;
                                    float2 _15194 = (((_15175 * 3.0) - (_15172 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _15207 = _15175 * 0.16666667163372039794921875;
                                    float2 _15210 = (((((-_15175) + (_15172 * 3.0)) - (_15169 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15194;
                                    float2 _15213 = (((((_15175 * (-3.0)) + (_15172 * 3.0)) + (_15169 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15207;
                                    float2 _15223 = ((_15166 - float2(0.5)) + (_15194 / _15210)) * _172.gLevel[3].zw;
                                    float2 _15233 = ((_15166 + float2(1.5)) + (_15207 / _15213)) * _172.gLevel[3].zw;
                                    float2 _22217;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21200 = _15223;
                                        _21200.y = 1.0 - _15223.y;
                                        _22217 = _21200;
                                    }
                                    else
                                    {
                                        _22217 = _15223;
                                    }
                                    float _15253 = _15223.y;
                                    float2 _15254 = float2(_15233.x, _15253);
                                    float2 _22218;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21206 = _15254;
                                        _21206.y = 1.0 - _15253;
                                        _22218 = _21206;
                                    }
                                    else
                                    {
                                        _22218 = _15254;
                                    }
                                    float _15270 = _15233.y;
                                    float2 _15271 = float2(_15223.x, _15270);
                                    float2 _22219;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21213 = _15271;
                                        _21213.y = 1.0 - _15270;
                                        _22219 = _21213;
                                    }
                                    else
                                    {
                                        _22219 = _15271;
                                    }
                                    float2 _22220;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21219 = _15233;
                                        _21219.y = 1.0 - _15233.y;
                                        _22220 = _21219;
                                    }
                                    else
                                    {
                                        _22220 = _15233;
                                    }
                                    _22233 = (((gBackdrop3.sample(gLinear, _22217, level(0.0)) * _15210.x) + (gBackdrop3.sample(gLinear, _22218, level(0.0)) * _15213.x)) * _15210.y) + (((gBackdrop3.sample(gLinear, _22219, level(0.0)) * _15210.x) + (gBackdrop3.sample(gLinear, _22220, level(0.0)) * _15213.x)) * _15213.y);
                                }
                                else
                                {
                                    float4 _22234;
                                    if (_13504 == 4)
                                    {
                                        float2 _15371 = (_6538 * _172.gLevel[4].xy) - float2(0.5);
                                        float2 _15373 = floor(_15371);
                                        float2 _15376 = _15371 - _15373;
                                        float2 _15379 = _15376 * _15376;
                                        float2 _15382 = _15379 * _15376;
                                        float2 _15401 = (((_15382 * 3.0) - (_15379 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _15414 = _15382 * 0.16666667163372039794921875;
                                        float2 _15417 = (((((-_15382) + (_15379 * 3.0)) - (_15376 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15401;
                                        float2 _15420 = (((((_15382 * (-3.0)) + (_15379 * 3.0)) + (_15376 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15414;
                                        float2 _15430 = ((_15373 - float2(0.5)) + (_15401 / _15417)) * _172.gLevel[4].zw;
                                        float2 _15440 = ((_15373 + float2(1.5)) + (_15414 / _15420)) * _172.gLevel[4].zw;
                                        float2 _22213;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21226 = _15430;
                                            _21226.y = 1.0 - _15430.y;
                                            _22213 = _21226;
                                        }
                                        else
                                        {
                                            _22213 = _15430;
                                        }
                                        float _15460 = _15430.y;
                                        float2 _15461 = float2(_15440.x, _15460);
                                        float2 _22214;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21232 = _15461;
                                            _21232.y = 1.0 - _15460;
                                            _22214 = _21232;
                                        }
                                        else
                                        {
                                            _22214 = _15461;
                                        }
                                        float _15477 = _15440.y;
                                        float2 _15478 = float2(_15430.x, _15477);
                                        float2 _22215;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21239 = _15478;
                                            _21239.y = 1.0 - _15477;
                                            _22215 = _21239;
                                        }
                                        else
                                        {
                                            _22215 = _15478;
                                        }
                                        float2 _22216;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21245 = _15440;
                                            _21245.y = 1.0 - _15440.y;
                                            _22216 = _21245;
                                        }
                                        else
                                        {
                                            _22216 = _15440;
                                        }
                                        _22234 = (((gBackdrop4.sample(gLinear, _22213, level(0.0)) * _15417.x) + (gBackdrop4.sample(gLinear, _22214, level(0.0)) * _15420.x)) * _15417.y) + (((gBackdrop4.sample(gLinear, _22215, level(0.0)) * _15417.x) + (gBackdrop4.sample(gLinear, _22216, level(0.0)) * _15420.x)) * _15420.y);
                                    }
                                    else
                                    {
                                        float2 _15578 = (_6538 * _172.gLevel[5].xy) - float2(0.5);
                                        float2 _15580 = floor(_15578);
                                        float2 _15583 = _15578 - _15580;
                                        float2 _15586 = _15583 * _15583;
                                        float2 _15589 = _15586 * _15583;
                                        float2 _15608 = (((_15589 * 3.0) - (_15586 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _15621 = _15589 * 0.16666667163372039794921875;
                                        float2 _15624 = (((((-_15589) + (_15586 * 3.0)) - (_15583 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15608;
                                        float2 _15627 = (((((_15589 * (-3.0)) + (_15586 * 3.0)) + (_15583 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15621;
                                        float2 _15637 = ((_15580 - float2(0.5)) + (_15608 / _15624)) * _172.gLevel[5].zw;
                                        float2 _15647 = ((_15580 + float2(1.5)) + (_15621 / _15627)) * _172.gLevel[5].zw;
                                        float2 _22209;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21252 = _15637;
                                            _21252.y = 1.0 - _15637.y;
                                            _22209 = _21252;
                                        }
                                        else
                                        {
                                            _22209 = _15637;
                                        }
                                        float _15667 = _15637.y;
                                        float2 _15668 = float2(_15647.x, _15667);
                                        float2 _22210;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21258 = _15668;
                                            _21258.y = 1.0 - _15667;
                                            _22210 = _21258;
                                        }
                                        else
                                        {
                                            _22210 = _15668;
                                        }
                                        float _15684 = _15647.y;
                                        float2 _15685 = float2(_15637.x, _15684);
                                        float2 _22211;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21265 = _15685;
                                            _21265.y = 1.0 - _15684;
                                            _22211 = _21265;
                                        }
                                        else
                                        {
                                            _22211 = _15685;
                                        }
                                        float2 _22212;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21271 = _15647;
                                            _21271.y = 1.0 - _15647.y;
                                            _22212 = _21271;
                                        }
                                        else
                                        {
                                            _22212 = _15647;
                                        }
                                        _22234 = (((gBackdrop5.sample(gLinear, _22209, level(0.0)) * _15624.x) + (gBackdrop5.sample(gLinear, _22210, level(0.0)) * _15627.x)) * _15624.y) + (((gBackdrop5.sample(gLinear, _22211, level(0.0)) * _15624.x) + (gBackdrop5.sample(gLinear, _22212, level(0.0)) * _15627.x)) * _15627.y);
                                    }
                                    _22233 = _22234;
                                }
                                _22232 = _22233;
                            }
                            _22231 = _22232;
                        }
                        _22230 = _22231;
                    }
                    _22235 = mix(_22204.xyz, _22230.xyz, float3(_13491));
                }
                else
                {
                    _22235 = _22204.xyz;
                }
                _22525 = float3(_22023.x, _22129.y, _22235.z);
            }
            else
            {
                float2 _6546 = _6476 + _6488;
                float _15775 = fast::clamp(log2(fast::max(_6390, 1.0)) - 1.0, 0.0, 5.0);
                int _15778 = int(floor(_15775));
                float _15782 = _15775 - float(_15778);
                float4 _21939;
                if (_15778 <= 0)
                {
                    float2 _21938;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _21278 = _6546;
                        _21278.y = 1.0 - _6546.y;
                        _21938 = _21278;
                    }
                    else
                    {
                        _21938 = _6546;
                    }
                    _21939 = gBackdrop0.sample(gLinear, _21938, level(0.0));
                }
                else
                {
                    float4 _21940;
                    if (_15778 == 1)
                    {
                        float2 _15917 = (_6546 * _172.gLevel[1].xy) - float2(0.5);
                        float2 _15919 = floor(_15917);
                        float2 _15922 = _15917 - _15919;
                        float2 _15925 = _15922 * _15922;
                        float2 _15928 = _15925 * _15922;
                        float2 _15947 = (((_15928 * 3.0) - (_15925 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                        float2 _15960 = _15928 * 0.16666667163372039794921875;
                        float2 _15963 = (((((-_15928) + (_15925 * 3.0)) - (_15922 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15947;
                        float2 _15966 = (((((_15928 * (-3.0)) + (_15925 * 3.0)) + (_15922 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15960;
                        float2 _15976 = ((_15919 - float2(0.5)) + (_15947 / _15963)) * _172.gLevel[1].zw;
                        float2 _15986 = ((_15919 + float2(1.5)) + (_15960 / _15966)) * _172.gLevel[1].zw;
                        float2 _21934;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21283 = _15976;
                            _21283.y = 1.0 - _15976.y;
                            _21934 = _21283;
                        }
                        else
                        {
                            _21934 = _15976;
                        }
                        float _16006 = _15976.y;
                        float2 _16007 = float2(_15986.x, _16006);
                        float2 _21935;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21289 = _16007;
                            _21289.y = 1.0 - _16006;
                            _21935 = _21289;
                        }
                        else
                        {
                            _21935 = _16007;
                        }
                        float _16023 = _15986.y;
                        float2 _16024 = float2(_15976.x, _16023);
                        float2 _21936;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21296 = _16024;
                            _21296.y = 1.0 - _16023;
                            _21936 = _21296;
                        }
                        else
                        {
                            _21936 = _16024;
                        }
                        float2 _21937;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21302 = _15986;
                            _21302.y = 1.0 - _15986.y;
                            _21937 = _21302;
                        }
                        else
                        {
                            _21937 = _15986;
                        }
                        _21940 = (((gBackdrop1.sample(gLinear, _21934, level(0.0)) * _15963.x) + (gBackdrop1.sample(gLinear, _21935, level(0.0)) * _15966.x)) * _15963.y) + (((gBackdrop1.sample(gLinear, _21936, level(0.0)) * _15963.x) + (gBackdrop1.sample(gLinear, _21937, level(0.0)) * _15966.x)) * _15966.y);
                    }
                    else
                    {
                        float4 _21941;
                        if (_15778 == 2)
                        {
                            float2 _16124 = (_6546 * _172.gLevel[2].xy) - float2(0.5);
                            float2 _16126 = floor(_16124);
                            float2 _16129 = _16124 - _16126;
                            float2 _16132 = _16129 * _16129;
                            float2 _16135 = _16132 * _16129;
                            float2 _16154 = (((_16135 * 3.0) - (_16132 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _16167 = _16135 * 0.16666667163372039794921875;
                            float2 _16170 = (((((-_16135) + (_16132 * 3.0)) - (_16129 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16154;
                            float2 _16173 = (((((_16135 * (-3.0)) + (_16132 * 3.0)) + (_16129 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16167;
                            float2 _16183 = ((_16126 - float2(0.5)) + (_16154 / _16170)) * _172.gLevel[2].zw;
                            float2 _16193 = ((_16126 + float2(1.5)) + (_16167 / _16173)) * _172.gLevel[2].zw;
                            float2 _21930;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21309 = _16183;
                                _21309.y = 1.0 - _16183.y;
                                _21930 = _21309;
                            }
                            else
                            {
                                _21930 = _16183;
                            }
                            float _16213 = _16183.y;
                            float2 _16214 = float2(_16193.x, _16213);
                            float2 _21931;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21315 = _16214;
                                _21315.y = 1.0 - _16213;
                                _21931 = _21315;
                            }
                            else
                            {
                                _21931 = _16214;
                            }
                            float _16230 = _16193.y;
                            float2 _16231 = float2(_16183.x, _16230);
                            float2 _21932;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21322 = _16231;
                                _21322.y = 1.0 - _16230;
                                _21932 = _21322;
                            }
                            else
                            {
                                _21932 = _16231;
                            }
                            float2 _21933;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21328 = _16193;
                                _21328.y = 1.0 - _16193.y;
                                _21933 = _21328;
                            }
                            else
                            {
                                _21933 = _16193;
                            }
                            _21941 = (((gBackdrop2.sample(gLinear, _21930, level(0.0)) * _16170.x) + (gBackdrop2.sample(gLinear, _21931, level(0.0)) * _16173.x)) * _16170.y) + (((gBackdrop2.sample(gLinear, _21932, level(0.0)) * _16170.x) + (gBackdrop2.sample(gLinear, _21933, level(0.0)) * _16173.x)) * _16173.y);
                        }
                        else
                        {
                            float4 _21942;
                            if (_15778 == 3)
                            {
                                float2 _16331 = (_6546 * _172.gLevel[3].xy) - float2(0.5);
                                float2 _16333 = floor(_16331);
                                float2 _16336 = _16331 - _16333;
                                float2 _16339 = _16336 * _16336;
                                float2 _16342 = _16339 * _16336;
                                float2 _16361 = (((_16342 * 3.0) - (_16339 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _16374 = _16342 * 0.16666667163372039794921875;
                                float2 _16377 = (((((-_16342) + (_16339 * 3.0)) - (_16336 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16361;
                                float2 _16380 = (((((_16342 * (-3.0)) + (_16339 * 3.0)) + (_16336 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16374;
                                float2 _16390 = ((_16333 - float2(0.5)) + (_16361 / _16377)) * _172.gLevel[3].zw;
                                float2 _16400 = ((_16333 + float2(1.5)) + (_16374 / _16380)) * _172.gLevel[3].zw;
                                float2 _21926;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21335 = _16390;
                                    _21335.y = 1.0 - _16390.y;
                                    _21926 = _21335;
                                }
                                else
                                {
                                    _21926 = _16390;
                                }
                                float _16420 = _16390.y;
                                float2 _16421 = float2(_16400.x, _16420);
                                float2 _21927;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21341 = _16421;
                                    _21341.y = 1.0 - _16420;
                                    _21927 = _21341;
                                }
                                else
                                {
                                    _21927 = _16421;
                                }
                                float _16437 = _16400.y;
                                float2 _16438 = float2(_16390.x, _16437);
                                float2 _21928;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21348 = _16438;
                                    _21348.y = 1.0 - _16437;
                                    _21928 = _21348;
                                }
                                else
                                {
                                    _21928 = _16438;
                                }
                                float2 _21929;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21354 = _16400;
                                    _21354.y = 1.0 - _16400.y;
                                    _21929 = _21354;
                                }
                                else
                                {
                                    _21929 = _16400;
                                }
                                _21942 = (((gBackdrop3.sample(gLinear, _21926, level(0.0)) * _16377.x) + (gBackdrop3.sample(gLinear, _21927, level(0.0)) * _16380.x)) * _16377.y) + (((gBackdrop3.sample(gLinear, _21928, level(0.0)) * _16377.x) + (gBackdrop3.sample(gLinear, _21929, level(0.0)) * _16380.x)) * _16380.y);
                            }
                            else
                            {
                                float4 _21943;
                                if (_15778 == 4)
                                {
                                    float2 _16538 = (_6546 * _172.gLevel[4].xy) - float2(0.5);
                                    float2 _16540 = floor(_16538);
                                    float2 _16543 = _16538 - _16540;
                                    float2 _16546 = _16543 * _16543;
                                    float2 _16549 = _16546 * _16543;
                                    float2 _16568 = (((_16549 * 3.0) - (_16546 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _16581 = _16549 * 0.16666667163372039794921875;
                                    float2 _16584 = (((((-_16549) + (_16546 * 3.0)) - (_16543 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16568;
                                    float2 _16587 = (((((_16549 * (-3.0)) + (_16546 * 3.0)) + (_16543 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16581;
                                    float2 _16597 = ((_16540 - float2(0.5)) + (_16568 / _16584)) * _172.gLevel[4].zw;
                                    float2 _16607 = ((_16540 + float2(1.5)) + (_16581 / _16587)) * _172.gLevel[4].zw;
                                    float2 _21922;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21361 = _16597;
                                        _21361.y = 1.0 - _16597.y;
                                        _21922 = _21361;
                                    }
                                    else
                                    {
                                        _21922 = _16597;
                                    }
                                    float _16627 = _16597.y;
                                    float2 _16628 = float2(_16607.x, _16627);
                                    float2 _21923;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21367 = _16628;
                                        _21367.y = 1.0 - _16627;
                                        _21923 = _21367;
                                    }
                                    else
                                    {
                                        _21923 = _16628;
                                    }
                                    float _16644 = _16607.y;
                                    float2 _16645 = float2(_16597.x, _16644);
                                    float2 _21924;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21374 = _16645;
                                        _21374.y = 1.0 - _16644;
                                        _21924 = _21374;
                                    }
                                    else
                                    {
                                        _21924 = _16645;
                                    }
                                    float2 _21925;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21380 = _16607;
                                        _21380.y = 1.0 - _16607.y;
                                        _21925 = _21380;
                                    }
                                    else
                                    {
                                        _21925 = _16607;
                                    }
                                    _21943 = (((gBackdrop4.sample(gLinear, _21922, level(0.0)) * _16584.x) + (gBackdrop4.sample(gLinear, _21923, level(0.0)) * _16587.x)) * _16584.y) + (((gBackdrop4.sample(gLinear, _21924, level(0.0)) * _16584.x) + (gBackdrop4.sample(gLinear, _21925, level(0.0)) * _16587.x)) * _16587.y);
                                }
                                else
                                {
                                    float2 _16745 = (_6546 * _172.gLevel[5].xy) - float2(0.5);
                                    float2 _16747 = floor(_16745);
                                    float2 _16750 = _16745 - _16747;
                                    float2 _16753 = _16750 * _16750;
                                    float2 _16756 = _16753 * _16750;
                                    float2 _16775 = (((_16756 * 3.0) - (_16753 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _16788 = _16756 * 0.16666667163372039794921875;
                                    float2 _16791 = (((((-_16756) + (_16753 * 3.0)) - (_16750 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16775;
                                    float2 _16794 = (((((_16756 * (-3.0)) + (_16753 * 3.0)) + (_16750 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _16788;
                                    float2 _16804 = ((_16747 - float2(0.5)) + (_16775 / _16791)) * _172.gLevel[5].zw;
                                    float2 _16814 = ((_16747 + float2(1.5)) + (_16788 / _16794)) * _172.gLevel[5].zw;
                                    float2 _21918;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21387 = _16804;
                                        _21387.y = 1.0 - _16804.y;
                                        _21918 = _21387;
                                    }
                                    else
                                    {
                                        _21918 = _16804;
                                    }
                                    float _16834 = _16804.y;
                                    float2 _16835 = float2(_16814.x, _16834);
                                    float2 _21919;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21393 = _16835;
                                        _21393.y = 1.0 - _16834;
                                        _21919 = _21393;
                                    }
                                    else
                                    {
                                        _21919 = _16835;
                                    }
                                    float _16851 = _16814.y;
                                    float2 _16852 = float2(_16804.x, _16851);
                                    float2 _21920;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21400 = _16852;
                                        _21400.y = 1.0 - _16851;
                                        _21920 = _21400;
                                    }
                                    else
                                    {
                                        _21920 = _16852;
                                    }
                                    float2 _21921;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21406 = _16814;
                                        _21406.y = 1.0 - _16814.y;
                                        _21921 = _21406;
                                    }
                                    else
                                    {
                                        _21921 = _16814;
                                    }
                                    _21943 = (((gBackdrop5.sample(gLinear, _21918, level(0.0)) * _16791.x) + (gBackdrop5.sample(gLinear, _21919, level(0.0)) * _16794.x)) * _16791.y) + (((gBackdrop5.sample(gLinear, _21920, level(0.0)) * _16791.x) + (gBackdrop5.sample(gLinear, _21921, level(0.0)) * _16794.x)) * _16794.y);
                                }
                                _21942 = _21943;
                            }
                            _21941 = _21942;
                        }
                        _21940 = _21941;
                    }
                    _21939 = _21940;
                }
                float3 _21970;
                if ((_15782 > 0.0199999995529651641845703125) && (_15778 < 5))
                {
                    int _15795 = _15778 + 1;
                    float4 _21965;
                    if (_15795 <= 0)
                    {
                        float2 _21964;
                        if (_172.gConv.x > 0.5)
                        {
                            float2 _21411 = _6546;
                            _21411.y = 1.0 - _6546.y;
                            _21964 = _21411;
                        }
                        else
                        {
                            _21964 = _6546;
                        }
                        _21965 = gBackdrop0.sample(gLinear, _21964, level(0.0));
                    }
                    else
                    {
                        float4 _21966;
                        if (_15795 == 1)
                        {
                            float2 _17041 = (_6546 * _172.gLevel[1].xy) - float2(0.5);
                            float2 _17043 = floor(_17041);
                            float2 _17046 = _17041 - _17043;
                            float2 _17049 = _17046 * _17046;
                            float2 _17052 = _17049 * _17046;
                            float2 _17071 = (((_17052 * 3.0) - (_17049 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                            float2 _17084 = _17052 * 0.16666667163372039794921875;
                            float2 _17087 = (((((-_17052) + (_17049 * 3.0)) - (_17046 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17071;
                            float2 _17090 = (((((_17052 * (-3.0)) + (_17049 * 3.0)) + (_17046 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17084;
                            float2 _17100 = ((_17043 - float2(0.5)) + (_17071 / _17087)) * _172.gLevel[1].zw;
                            float2 _17110 = ((_17043 + float2(1.5)) + (_17084 / _17090)) * _172.gLevel[1].zw;
                            float2 _21960;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21416 = _17100;
                                _21416.y = 1.0 - _17100.y;
                                _21960 = _21416;
                            }
                            else
                            {
                                _21960 = _17100;
                            }
                            float _17130 = _17100.y;
                            float2 _17131 = float2(_17110.x, _17130);
                            float2 _21961;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21422 = _17131;
                                _21422.y = 1.0 - _17130;
                                _21961 = _21422;
                            }
                            else
                            {
                                _21961 = _17131;
                            }
                            float _17147 = _17110.y;
                            float2 _17148 = float2(_17100.x, _17147);
                            float2 _21962;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21429 = _17148;
                                _21429.y = 1.0 - _17147;
                                _21962 = _21429;
                            }
                            else
                            {
                                _21962 = _17148;
                            }
                            float2 _21963;
                            if (_172.gConv.x > 0.5)
                            {
                                float2 _21435 = _17110;
                                _21435.y = 1.0 - _17110.y;
                                _21963 = _21435;
                            }
                            else
                            {
                                _21963 = _17110;
                            }
                            _21966 = (((gBackdrop1.sample(gLinear, _21960, level(0.0)) * _17087.x) + (gBackdrop1.sample(gLinear, _21961, level(0.0)) * _17090.x)) * _17087.y) + (((gBackdrop1.sample(gLinear, _21962, level(0.0)) * _17087.x) + (gBackdrop1.sample(gLinear, _21963, level(0.0)) * _17090.x)) * _17090.y);
                        }
                        else
                        {
                            float4 _21967;
                            if (_15795 == 2)
                            {
                                float2 _17248 = (_6546 * _172.gLevel[2].xy) - float2(0.5);
                                float2 _17250 = floor(_17248);
                                float2 _17253 = _17248 - _17250;
                                float2 _17256 = _17253 * _17253;
                                float2 _17259 = _17256 * _17253;
                                float2 _17278 = (((_17259 * 3.0) - (_17256 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                float2 _17291 = _17259 * 0.16666667163372039794921875;
                                float2 _17294 = (((((-_17259) + (_17256 * 3.0)) - (_17253 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17278;
                                float2 _17297 = (((((_17259 * (-3.0)) + (_17256 * 3.0)) + (_17253 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17291;
                                float2 _17307 = ((_17250 - float2(0.5)) + (_17278 / _17294)) * _172.gLevel[2].zw;
                                float2 _17317 = ((_17250 + float2(1.5)) + (_17291 / _17297)) * _172.gLevel[2].zw;
                                float2 _21956;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21442 = _17307;
                                    _21442.y = 1.0 - _17307.y;
                                    _21956 = _21442;
                                }
                                else
                                {
                                    _21956 = _17307;
                                }
                                float _17337 = _17307.y;
                                float2 _17338 = float2(_17317.x, _17337);
                                float2 _21957;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21448 = _17338;
                                    _21448.y = 1.0 - _17337;
                                    _21957 = _21448;
                                }
                                else
                                {
                                    _21957 = _17338;
                                }
                                float _17354 = _17317.y;
                                float2 _17355 = float2(_17307.x, _17354);
                                float2 _21958;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21455 = _17355;
                                    _21455.y = 1.0 - _17354;
                                    _21958 = _21455;
                                }
                                else
                                {
                                    _21958 = _17355;
                                }
                                float2 _21959;
                                if (_172.gConv.x > 0.5)
                                {
                                    float2 _21461 = _17317;
                                    _21461.y = 1.0 - _17317.y;
                                    _21959 = _21461;
                                }
                                else
                                {
                                    _21959 = _17317;
                                }
                                _21967 = (((gBackdrop2.sample(gLinear, _21956, level(0.0)) * _17294.x) + (gBackdrop2.sample(gLinear, _21957, level(0.0)) * _17297.x)) * _17294.y) + (((gBackdrop2.sample(gLinear, _21958, level(0.0)) * _17294.x) + (gBackdrop2.sample(gLinear, _21959, level(0.0)) * _17297.x)) * _17297.y);
                            }
                            else
                            {
                                float4 _21968;
                                if (_15795 == 3)
                                {
                                    float2 _17455 = (_6546 * _172.gLevel[3].xy) - float2(0.5);
                                    float2 _17457 = floor(_17455);
                                    float2 _17460 = _17455 - _17457;
                                    float2 _17463 = _17460 * _17460;
                                    float2 _17466 = _17463 * _17460;
                                    float2 _17485 = (((_17466 * 3.0) - (_17463 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                    float2 _17498 = _17466 * 0.16666667163372039794921875;
                                    float2 _17501 = (((((-_17466) + (_17463 * 3.0)) - (_17460 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17485;
                                    float2 _17504 = (((((_17466 * (-3.0)) + (_17463 * 3.0)) + (_17460 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17498;
                                    float2 _17514 = ((_17457 - float2(0.5)) + (_17485 / _17501)) * _172.gLevel[3].zw;
                                    float2 _17524 = ((_17457 + float2(1.5)) + (_17498 / _17504)) * _172.gLevel[3].zw;
                                    float2 _21952;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21468 = _17514;
                                        _21468.y = 1.0 - _17514.y;
                                        _21952 = _21468;
                                    }
                                    else
                                    {
                                        _21952 = _17514;
                                    }
                                    float _17544 = _17514.y;
                                    float2 _17545 = float2(_17524.x, _17544);
                                    float2 _21953;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21474 = _17545;
                                        _21474.y = 1.0 - _17544;
                                        _21953 = _21474;
                                    }
                                    else
                                    {
                                        _21953 = _17545;
                                    }
                                    float _17561 = _17524.y;
                                    float2 _17562 = float2(_17514.x, _17561);
                                    float2 _21954;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21481 = _17562;
                                        _21481.y = 1.0 - _17561;
                                        _21954 = _21481;
                                    }
                                    else
                                    {
                                        _21954 = _17562;
                                    }
                                    float2 _21955;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        float2 _21487 = _17524;
                                        _21487.y = 1.0 - _17524.y;
                                        _21955 = _21487;
                                    }
                                    else
                                    {
                                        _21955 = _17524;
                                    }
                                    _21968 = (((gBackdrop3.sample(gLinear, _21952, level(0.0)) * _17501.x) + (gBackdrop3.sample(gLinear, _21953, level(0.0)) * _17504.x)) * _17501.y) + (((gBackdrop3.sample(gLinear, _21954, level(0.0)) * _17501.x) + (gBackdrop3.sample(gLinear, _21955, level(0.0)) * _17504.x)) * _17504.y);
                                }
                                else
                                {
                                    float4 _21969;
                                    if (_15795 == 4)
                                    {
                                        float2 _17662 = (_6546 * _172.gLevel[4].xy) - float2(0.5);
                                        float2 _17664 = floor(_17662);
                                        float2 _17667 = _17662 - _17664;
                                        float2 _17670 = _17667 * _17667;
                                        float2 _17673 = _17670 * _17667;
                                        float2 _17692 = (((_17673 * 3.0) - (_17670 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _17705 = _17673 * 0.16666667163372039794921875;
                                        float2 _17708 = (((((-_17673) + (_17670 * 3.0)) - (_17667 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17692;
                                        float2 _17711 = (((((_17673 * (-3.0)) + (_17670 * 3.0)) + (_17667 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17705;
                                        float2 _17721 = ((_17664 - float2(0.5)) + (_17692 / _17708)) * _172.gLevel[4].zw;
                                        float2 _17731 = ((_17664 + float2(1.5)) + (_17705 / _17711)) * _172.gLevel[4].zw;
                                        float2 _21948;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21494 = _17721;
                                            _21494.y = 1.0 - _17721.y;
                                            _21948 = _21494;
                                        }
                                        else
                                        {
                                            _21948 = _17721;
                                        }
                                        float _17751 = _17721.y;
                                        float2 _17752 = float2(_17731.x, _17751);
                                        float2 _21949;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21500 = _17752;
                                            _21500.y = 1.0 - _17751;
                                            _21949 = _21500;
                                        }
                                        else
                                        {
                                            _21949 = _17752;
                                        }
                                        float _17768 = _17731.y;
                                        float2 _17769 = float2(_17721.x, _17768);
                                        float2 _21950;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21507 = _17769;
                                            _21507.y = 1.0 - _17768;
                                            _21950 = _21507;
                                        }
                                        else
                                        {
                                            _21950 = _17769;
                                        }
                                        float2 _21951;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21513 = _17731;
                                            _21513.y = 1.0 - _17731.y;
                                            _21951 = _21513;
                                        }
                                        else
                                        {
                                            _21951 = _17731;
                                        }
                                        _21969 = (((gBackdrop4.sample(gLinear, _21948, level(0.0)) * _17708.x) + (gBackdrop4.sample(gLinear, _21949, level(0.0)) * _17711.x)) * _17708.y) + (((gBackdrop4.sample(gLinear, _21950, level(0.0)) * _17708.x) + (gBackdrop4.sample(gLinear, _21951, level(0.0)) * _17711.x)) * _17711.y);
                                    }
                                    else
                                    {
                                        float2 _17869 = (_6546 * _172.gLevel[5].xy) - float2(0.5);
                                        float2 _17871 = floor(_17869);
                                        float2 _17874 = _17869 - _17871;
                                        float2 _17877 = _17874 * _17874;
                                        float2 _17880 = _17877 * _17874;
                                        float2 _17899 = (((_17880 * 3.0) - (_17877 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                                        float2 _17912 = _17880 * 0.16666667163372039794921875;
                                        float2 _17915 = (((((-_17880) + (_17877 * 3.0)) - (_17874 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17899;
                                        float2 _17918 = (((((_17880 * (-3.0)) + (_17877 * 3.0)) + (_17874 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _17912;
                                        float2 _17928 = ((_17871 - float2(0.5)) + (_17899 / _17915)) * _172.gLevel[5].zw;
                                        float2 _17938 = ((_17871 + float2(1.5)) + (_17912 / _17918)) * _172.gLevel[5].zw;
                                        float2 _21944;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21520 = _17928;
                                            _21520.y = 1.0 - _17928.y;
                                            _21944 = _21520;
                                        }
                                        else
                                        {
                                            _21944 = _17928;
                                        }
                                        float _17958 = _17928.y;
                                        float2 _17959 = float2(_17938.x, _17958);
                                        float2 _21945;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21526 = _17959;
                                            _21526.y = 1.0 - _17958;
                                            _21945 = _21526;
                                        }
                                        else
                                        {
                                            _21945 = _17959;
                                        }
                                        float _17975 = _17938.y;
                                        float2 _17976 = float2(_17928.x, _17975);
                                        float2 _21946;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21533 = _17976;
                                            _21533.y = 1.0 - _17975;
                                            _21946 = _21533;
                                        }
                                        else
                                        {
                                            _21946 = _17976;
                                        }
                                        float2 _21947;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            float2 _21539 = _17938;
                                            _21539.y = 1.0 - _17938.y;
                                            _21947 = _21539;
                                        }
                                        else
                                        {
                                            _21947 = _17938;
                                        }
                                        _21969 = (((gBackdrop5.sample(gLinear, _21944, level(0.0)) * _17915.x) + (gBackdrop5.sample(gLinear, _21945, level(0.0)) * _17918.x)) * _17915.y) + (((gBackdrop5.sample(gLinear, _21946, level(0.0)) * _17915.x) + (gBackdrop5.sample(gLinear, _21947, level(0.0)) * _17918.x)) * _17918.y);
                                    }
                                    _21968 = _21969;
                                }
                                _21967 = _21968;
                            }
                            _21966 = _21967;
                        }
                        _21965 = _21966;
                    }
                    _21970 = mix(_21939.xyz, _21965.xyz, float3(_15782));
                }
                else
                {
                    _21970 = _21939.xyz;
                }
                _22525 = _21970;
            }
            float3 _23016;
            if (_6418 > 0.0)
            {
                float2 _6567 = _6476 + (((_21916 * fast::min(_6409 * 0.5, 16.0)) * _172.gDisplay.zw) * _172.gTarget.zw);
                float _18066 = fast::clamp(log2(fast::max(16.0 * _172.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _18069 = int(floor(_18066));
                float _18073 = _18066 - float(_18069);
                float2 _22503;
                if (_172.gConv.x > 0.5)
                {
                    float2 _21544 = _6567;
                    _21544.y = 1.0 - _6567.y;
                    _22503 = _21544;
                }
                else
                {
                    _22503 = _6567;
                }
                float4 _22504;
                if (_18069 <= 0)
                {
                    _22504 = gBackdrop0.sample(gLinear, _22503, level(0.0));
                }
                else
                {
                    float4 _22505;
                    if (_18069 == 1)
                    {
                        _22505 = gBackdrop1.sample(gLinear, _22503, level(0.0));
                    }
                    else
                    {
                        float4 _22506;
                        if (_18069 == 2)
                        {
                            _22506 = gBackdrop2.sample(gLinear, _22503, level(0.0));
                        }
                        else
                        {
                            float4 _22507;
                            if (_18069 == 3)
                            {
                                _22507 = gBackdrop3.sample(gLinear, _22503, level(0.0));
                            }
                            else
                            {
                                float4 _22508;
                                if (_18069 == 4)
                                {
                                    _22508 = gBackdrop4.sample(gLinear, _22503, level(0.0));
                                }
                                else
                                {
                                    _22508 = gBackdrop5.sample(gLinear, _22503, level(0.0));
                                }
                                _22507 = _22508;
                            }
                            _22506 = _22507;
                        }
                        _22505 = _22506;
                    }
                    _22504 = _22505;
                }
                float3 _22515;
                if ((_18073 > 0.0199999995529651641845703125) && (_18069 < 5))
                {
                    int _18086 = _18069 + 1;
                    float2 _22509;
                    if (_172.gConv.x > 0.5)
                    {
                        float2 _21547 = _6567;
                        _21547.y = 1.0 - _6567.y;
                        _22509 = _21547;
                    }
                    else
                    {
                        _22509 = _6567;
                    }
                    float4 _22510;
                    if (_18086 <= 0)
                    {
                        _22510 = gBackdrop0.sample(gLinear, _22509, level(0.0));
                    }
                    else
                    {
                        float4 _22511;
                        if (_18086 == 1)
                        {
                            _22511 = gBackdrop1.sample(gLinear, _22509, level(0.0));
                        }
                        else
                        {
                            float4 _22512;
                            if (_18086 == 2)
                            {
                                _22512 = gBackdrop2.sample(gLinear, _22509, level(0.0));
                            }
                            else
                            {
                                float4 _22513;
                                if (_18086 == 3)
                                {
                                    _22513 = gBackdrop3.sample(gLinear, _22509, level(0.0));
                                }
                                else
                                {
                                    float4 _22514;
                                    if (_18086 == 4)
                                    {
                                        _22514 = gBackdrop4.sample(gLinear, _22509, level(0.0));
                                    }
                                    else
                                    {
                                        _22514 = gBackdrop5.sample(gLinear, _22509, level(0.0));
                                    }
                                    _22513 = _22514;
                                }
                                _22512 = _22513;
                            }
                            _22511 = _22512;
                        }
                        _22510 = _22511;
                    }
                    _22515 = mix(_22504.xyz, _22510.xyz, float3(_18073));
                }
                else
                {
                    _22515 = _22504.xyz;
                }
                _23016 = _22515;
            }
            else
            {
                _23016 = float3(0.5);
            }
            float _22546;
            if (gFxData_1._data[_4978].x > 0.001000000047497451305389404296875)
            {
                int _18253 = clamp(int(rint(log2(36.0 * _172.gDisplay.z) - 1.0)), 1, 4);
                float2 _22516;
                if (_172.gConv.x > 0.5)
                {
                    float2 _21551 = _6476;
                    _21551.y = 1.0 - _6476.y;
                    _22516 = _21551;
                }
                else
                {
                    _22516 = _6476;
                }
                float4 _22517;
                if (_18253 <= 0)
                {
                    _22517 = gBackdrop0.sample(gLinear, _22516, level(0.0));
                }
                else
                {
                    float4 _22518;
                    if (_18253 == 1)
                    {
                        _22518 = gBackdrop1.sample(gLinear, _22516, level(0.0));
                    }
                    else
                    {
                        float4 _22519;
                        if (_18253 == 2)
                        {
                            _22519 = gBackdrop2.sample(gLinear, _22516, level(0.0));
                        }
                        else
                        {
                            float4 _22520;
                            if (_18253 == 3)
                            {
                                _22520 = gBackdrop3.sample(gLinear, _22516, level(0.0));
                            }
                            else
                            {
                                float4 _22521;
                                if (_18253 == 4)
                                {
                                    _22521 = gBackdrop4.sample(gLinear, _22516, level(0.0));
                                }
                                else
                                {
                                    _22521 = gBackdrop5.sample(gLinear, _22516, level(0.0));
                                }
                                _22520 = _22521;
                            }
                            _22519 = _22520;
                        }
                        _22518 = _22519;
                    }
                    _22517 = _22518;
                }
                _22546 = dot(_22517.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _22546 = 0.5;
            }
            _23014 = _23016;
            _22545 = _22546;
            _22522 = _22525;
        }
        else
        {
            _23014 = float3(0.5);
            _22545 = 0.5;
            _22522 = float3(0.5);
        }
        float3 _6595 = mix(float3(dot(_22522, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _22522, float3(gFxData_1._data[_4970].x)) + float3(gFxData_1._data[_4970].y);
        float3 _22774;
        if (gFxData_1._data[_4978].x > 0.001000000047497451305389404296875)
        {
            float _6605 = fast::clamp(fast::max(_22545 + gFxData_1._data[_4970].y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _6613 = mix(_6605, dot(gFxData_1._data[_4962].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), gFxData_1._data[_4978].x);
            _22774 = select(mix(_6595, float3(1.0), float3((_6613 - _6605) / fast::max(1.0 - _6605, 0.001000000047497451305389404296875))), _6595 * (_6613 / _6605), bool3(_6613 < _6605));
        }
        else
        {
            _22774 = _6595;
        }
        float2 _6651 = fast::clamp((in.i_local - gFxData_1._data[_4864].xy) / fast::max(gFxData_1._data[_4864].zw - gFxData_1._data[_4864].xy, float2(0.001000000047497451305389404296875)), float2(0.0), float2(1.0));
        float _6683 = pow(1.0 - _22778.z, 5.0);
        float2 _6699 = float2(cos(gFxData_1._data[_4970].w), sin(gFxData_1._data[_4970].w));
        float _6702 = dot(_21916, _6699);
        float _6737 = fast::clamp(0.5 + (0.5 * dot((in.i_local - ((gFxData_1._data[_4864].xy + gFxData_1._data[_4864].zw) * 0.5)) / _6401, _6699)), 0.0, 1.0);
        _23596 = ((_6683 * (pow(fast::clamp(_6702, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(fast::clamp(-_6702, 0.0, 1.0), 1.5)))) * gFxData_1._data[_4970].z) * 1.60000002384185791015625;
        _23550 = float4((mix(mix(_22774, gFxData_1._data[_4962].xyz, float3(fast::clamp(gFxData_1._data[_4962].w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _6424))) + (0.3499999940395355224609375 * ((dot(gFxData_1._data[_4962].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _6651.y) : _6651.y))), 0.0, 1.0))), (_23014 * 1.10000002384185791015625) + float3(0.07999999821186065673828125), float3(_6683 * 0.3499999940395355224609375)) + float3((((0.039999999105930328369140625 * _6737) * _6737) + (0.0500000007450580596923828125 * (1.0 - _6424))) * gFxData_1._data[_4970].z)) * _3711, _3711) + (_23263 * (1.0 - _3711));
    }
    else
    {
        _23596 = 0.0;
        _23550 = _23263;
    }
    float4 _23560;
    if ((_3688 & 1u) != 0u)
    {
        float4 _23546;
        do
        {
            uint _18377 = _4858.y;
            if (_18377 == 0u)
            {
                _23546 = gFxData_1._data[_4882];
                break;
            }
            float2 _18391 = fast::max(gFxData_1._data[_4864].zw - gFxData_1._data[_4864].xy, float2(0.001000000047497451305389404296875));
            float2 _18401 = fwidth(in.i_local);
            float _18403 = fast::max(length(_18401), 9.9999997473787516355514526367188e-05);
            float _23538;
            float _23542;
            if ((_18377 == 1u) || (_18377 == 4u))
            {
                float _18412 = cos(gFxData_1._data[_4898].x);
                float _18415 = sin(gFxData_1._data[_4898].x);
                float _18441 = ((dot(in.i_local - ((gFxData_1._data[_4864].xy + gFxData_1._data[_4864].zw) * 0.5), float2(_18412, _18415)) / fast::max(0.5 * ((abs(_18412) * _18391.x) + (abs(_18415) * _18391.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (_18377 == 4u)
                {
                    float _18456 = fast::clamp((_18441 - gFxData_1._data[_4898].y) / fast::max(gFxData_1._data[_4898].z - gFxData_1._data[_4898].y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _23546 = float4(fast::clamp(abs((fract(float3(_18456 * 0.800000011920928955078125) + float3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - float3(3.0)) - float3(1.0), float3(0.0), float3(1.0)), gFxData_1._data[_4882].w * pow(fast::max(sin(_18456 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _23542 = -1.0;
                _23538 = _18441;
            }
            else
            {
                float _23539;
                float _23543;
                if (_18377 == 2u)
                {
                    _23543 = -1.0;
                    _23539 = length(in.i_local - (gFxData_1._data[_4864].xy + (gFxData_1._data[_4898].xy * _18391))) / fast::max(gFxData_1._data[_4898].z * fast::max(_18391.x, _18391.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    float2 _18524 = in.i_local - (gFxData_1._data[_4864].xy + (gFxData_1._data[_4898].xy * _18391));
                    float _18535 = fract(((precise::atan2(_18524.y, _18524.x) - gFxData_1._data[_4898].z) * 0.15915493667125701904296875) + 1.0);
                    float _23540;
                    float _23544;
                    if (gFxData_1._data[_4898].w > 0.5)
                    {
                        _23544 = -1.0;
                        _23540 = 0.5 - (0.5 * cos(_18535 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _18555 = (((_18535 < 0.5) ? _18535 : (_18535 - 1.0)) * 6.283185482025146484375) * length(_18524);
                        float _23545;
                        if (abs(_18555) < _18403)
                        {
                            _23545 = fast::clamp(((_18555 / _18403) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _23545 = -1.0;
                        }
                        _23544 = _23545;
                        _23540 = _18535;
                    }
                    _23543 = _23544;
                    _23539 = _23540;
                }
                _23542 = _23543;
                _23538 = _23539;
            }
            float4 _18584 = float4(gFxData_1._data[_4882].xyz * gFxData_1._data[_4882].w, gFxData_1._data[_4882].w);
            float4 _18596 = float4(gFxData_1._data[_4890].xyz * gFxData_1._data[_4890].w, gFxData_1._data[_4890].w);
            float4 _18610 = select(mix(_18584, _18596, float4(fast::clamp(_23538, 0.0, 1.0))), mix(_18596, _18584, float4(_23542)), bool4(_23542 >= 0.0));
            _23546 = select(float4(0.0), float4(_18610.xyz / float3(_18610.w), _18610.w), bool4(_18610.w > 9.9999997473787516355514526367188e-06));
            break;
        } while(false);
        float4 _23547;
        if ((_3688 & 64u) != 0u)
        {
            _23547 = _23546 * gTex.sample(gLinear, mix(gFxData_1._data[_5010].xy, gFxData_1._data[_5010].zw, (in.i_local - gFxData_1._data[_4864].xy) / fast::max(gFxData_1._data[_4864].zw - gFxData_1._data[_4864].xy, float2(0.001000000047497451305389404296875))));
        }
        else
        {
            _23547 = _23546;
        }
        float _18637 = fast::clamp(_23547.w * _3711, 0.0, 1.0);
        _23560 = float4(_23547.xyz * _18637, _18637) + (_23550 * (1.0 - _18637));
    }
    else
    {
        _23560 = _23550;
    }
    float4 _23582;
    if ((_3688 & 16384u) != 0u)
    {
        float2 _4081 = fast::max(gFxData_1._data[_4864].zw - gFxData_1._data[_4864].xy, float2(0.001000000047497451305389404296875));
        float2 _4088 = (in.i_local - gFxData_1._data[_4864].xy) / _4081;
        float _4099 = ((gFxData_1._data[_4890].x >= 0.0) ? gFxData_1._data[_4890].x : _172.gTime.x) * gFxData_1._data[_5034].z;
        float _4102 = _4088.x * 2.0;
        float _4103 = _4102 - 1.0;
        float _4108 = _4088.y * _4081.y;
        float _4111 = fast::max(gFxData_1._data[_5034].w, 0.001000000047497451305389404296875);
        float _4116 = fast::clamp(1.0 - (_4103 * _4103), 0.0, 1.0);
        float _4118 = pow(_4116, 1.2999999523162841796875);
        float _4120 = pow(_4116, 0.699999988079071044921875);
        float _4123 = fast::clamp(_4099 * 1.4285714626312255859375, 0.0, 1.0);
        float _4139 = (0.25 + (0.75 * ((_4123 * _4123) * (3.0 - (2.0 * _4123))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_4099 * 2.099999904632568359375)));
        float _4148 = ((gFxData_1._data[_5034].y * _4081.y) * _4139) * _4118;
        float _4174 = (_4081.y * (0.5 + ((gFxData_1._data[_5034].x * (0.5 - (_4103 * _4103))) * 0.5))) + (((0.14000000059604644775390625 * _4081.y) * _4118) * sin(((_4103 * 2.400000095367431640625) - (_4099 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _23556;
        float _23557;
        float3 _23558;
        _23558 = float3(0.0);
        _23557 = _4174;
        _23556 = _4174;
        float3 _4246;
        float _23968;
        float _23969;
        for (int _23555 = 0; _23555 < 4; _23558 = _4246, _23557 = _23969, _23556 = _23968, _23555++)
        {
            float _4198 = _4174 + ((_4148 * _2844[_23555].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_4099 * 1.7000000476837158203125) + _2861[_23555].y))));
            _23968 = (_23555 == 0) ? _4198 : _23556;
            _23969 = (_23555 == 2) ? _4198 : _23557;
            float _4213 = _4111 * _2844[_23555].y;
            float _4218 = (_4108 - _4198) / _4213;
            float _4223 = _4111 * _2844[_23555].z;
            float3 _23948;
            _23948 = float3(0.0);
            for (int _23947 = 0; _23947 < 6; )
            {
                float _18678 = ((_4108 - _4198) - (_4223 * ((float(_23947) * 0.4000000059604644775390625) - 1.0))) / _4213;
                _23948 += (_1761[_23947] * exp((-_18678) * _18678));
                _23947++;
                continue;
            }
            _4246 = _23558 + (mix(_23948 * float3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), float3(exp((-_4218) * _4218)), float3(_2861[_23555].x)) * (_2844[_23555].w * _4120));
        }
        float _4252 = _4111 * 1.5;
        float _4282 = fast::clamp((_4108 - _23556) / fast::max(_23557 - _23556, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _4310 = (_4108 - (_23557 - (_4111 * 3.0))) / (((_4081.y * 0.0900000035762786865234375) + (_4148 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _4313 = (_4102 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _4338 = ((_4108 - _23556) + (_4111 * 5.0)) / (_4111 * 7.0);
        float3 _4364 = float3(1.0) - exp((-((((_23558 + (mix(float3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), float3(1.0), float3(_4282)) * ((((1.0 / (1.0 + exp((-((_4108 - _23556) - (_4111 * 2.0))) / _4252))) / (1.0 + exp((-(_23557 - _4108)) / _4252))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_4282, 2.5)))) * _4120))) + (float3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_4310) * _4310) - (_4313 * _4313)) * (0.5 + (1.10000002384185791015625 * _4139))))) + (float3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_4338) * _4338) * _4118) * 0.100000001490116119384765625))) * mix(float3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), float3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), float3(0.5 + (0.5 * sin(_4099 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _4374 = (gFxData_1._data[_4882].w * smoothstep(0.0, 0.119999997317790985107421875, _4088.y)) * smoothstep(1.0, 0.87999999523162841796875, _4088.y);
        float _4393 = (fast::clamp(fast::max(_4364.x, fast::max(_4364.y, _4364.z)), 0.0, 1.0) * _4374) * _3711;
        _23582 = float4((_4364 * _4374) * _3711, _4393) + (_23560 * (1.0 - _4393));
    }
    else
    {
        _23582 = _23560;
    }
    float4 _23591;
    if (((_3688 & 4u) != 0u) && ((_3688 & 256u) != 0u))
    {
        float2 _4419 = in.i_local - gFxData_1._data[_4930].zw;
        uint _18735 = _4858.z;
        float2 _18743 = (gFxData_1._data[_4864].xy + gFxData_1._data[_4864].zw) * 0.5;
        float2 _18752 = fast::max((gFxData_1._data[_4864].zw - gFxData_1._data[_4864].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _23577;
        if (_18735 == 1u)
        {
            float2 _18758 = _4419 - _18743;
            float _23576;
            do
            {
                if (gFxData_1._data[_4978].w >= 6.282185077667236328125)
                {
                    _23576 = abs(length(_18758) - gFxData_1._data[_4874].x) - gFxData_1._data[_4874].y;
                    break;
                }
                float _18857 = gFxData_1._data[_4978].z + (gFxData_1._data[_4978].w * 0.5);
                float _18859 = cos(_18857);
                float _18861 = sin(_18857);
                float _18870 = dot(_18758, float2(-_18861, _18859));
                float _18873 = dot(_18758, float2(_18859, _18861));
                float2 _18874 = float2(_18870, _18873);
                float _18877 = abs(_18870);
                _18874.x = _18877;
                float _18880 = gFxData_1._data[_4978].w * 0.5;
                float _18882 = sin(_18880);
                float _18884 = cos(_18880);
                _23576 = (((_18884 * _18877) > (_18882 * _18873)) ? length(_18874 - (float2(_18882, _18884) * gFxData_1._data[_4874].x)) : abs(length(_18874) - gFxData_1._data[_4874].x)) - gFxData_1._data[_4874].y;
                break;
            } while(false);
            _23577 = _23576;
        }
        else
        {
            float _23578;
            if (_18735 == 2u)
            {
                float2 _18920 = _4419 - gFxData_1._data[_4986].xy;
                float2 _18923 = gFxData_1._data[_4986].zw - gFxData_1._data[_4986].xy;
                _23578 = length(_18920 - (_18923 * fast::clamp(dot(_18920, _18923) / fast::max(dot(_18923, _18923), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - gFxData_1._data[_4874].x;
            }
            else
            {
                float2 _18785 = _4419 - _18743;
                float _18976 = fast::min(_18752.x, _18752.y);
                float _18979 = fast::min((_18785.x > 0.0) ? ((_18785.y > 0.0) ? gFxData_1._data[_4874].z : gFxData_1._data[_4874].y) : ((_18785.y > 0.0) ? gFxData_1._data[_4874].w : gFxData_1._data[_4874].x), _18976);
                float _18985 = _18979 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4978].y));
                float _23561;
                float _23562;
                if (_18985 > _18976)
                {
                    float _18999 = gFxData_1._data[_4978].y * fast::clamp((_18976 - _18979) / fast::max(0.60000002384185791015625 * _18979, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _23562 = _18999;
                    _23561 = _18979 * (1.0 + (0.60000002384185791015625 * _18999));
                }
                else
                {
                    _23562 = gFxData_1._data[_4978].y;
                    _23561 = _18985;
                }
                float2 _19012 = (abs(_18785) - _18752) + float2(_23561);
                float2 _19014 = fast::max(_19012, float2(0.0));
                float _23563;
                if ((_19014.x > 0.0) && (_19014.y > 0.0))
                {
                    float _23564;
                    if ((_23562 > 0.001000000047497451305389404296875) && (_23561 > 9.9999997473787516355514526367188e-05))
                    {
                        float _19031 = 2.0 + (2.0 * _23562);
                        float2 _19036 = _19014 / float2(fast::max(_23561, 9.9999997473787516355514526367188e-05));
                        _23564 = pow(pow(_19036.x, _19031) + pow(_19036.y, _19031), 1.0 / _19031) * _23561;
                    }
                    else
                    {
                        _23564 = length(_19014);
                    }
                    _23563 = _23564;
                }
                else
                {
                    _23563 = fast::max(_19014.x, _19014.y);
                }
                float _19071 = (fast::min(fast::max(_19012.x, _19012.y), 0.0) + _23563) - _23561;
                float _23579;
                if ((_4858.x & 512u) != 0u)
                {
                    float2 _18813 = fast::max((gFxData_1._data[_4986].zw - gFxData_1._data[_4986].xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _18816 = _4419 - ((gFxData_1._data[_4986].xy + gFxData_1._data[_4986].zw) * 0.5);
                    float _19107 = fast::min(_18813.x, _18813.y);
                    float _19110 = fast::min((_18816.x > 0.0) ? ((_18816.y > 0.0) ? gFxData_1._data[_4994].x : gFxData_1._data[_4994].x) : ((_18816.y > 0.0) ? gFxData_1._data[_4994].x : gFxData_1._data[_4994].x), _19107);
                    float _19116 = _19110 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_4978].y));
                    float _23567;
                    float _23568;
                    if (_19116 > _19107)
                    {
                        float _19130 = gFxData_1._data[_4978].y * fast::clamp((_19107 - _19110) / fast::max(0.60000002384185791015625 * _19110, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _23568 = _19130;
                        _23567 = _19110 * (1.0 + (0.60000002384185791015625 * _19130));
                    }
                    else
                    {
                        _23568 = gFxData_1._data[_4978].y;
                        _23567 = _19116;
                    }
                    float2 _19143 = (abs(_18816) - _18813) + float2(_23567);
                    float2 _19145 = fast::max(_19143, float2(0.0));
                    float _23569;
                    if ((_19145.x > 0.0) && (_19145.y > 0.0))
                    {
                        float _23570;
                        if ((_23568 > 0.001000000047497451305389404296875) && (_23567 > 9.9999997473787516355514526367188e-05))
                        {
                            float _19162 = 2.0 + (2.0 * _23568);
                            float2 _19167 = _19145 / float2(fast::max(_23567, 9.9999997473787516355514526367188e-05));
                            _23570 = pow(pow(_19167.x, _19162) + pow(_19167.y, _19162), 1.0 / _19162) * _23567;
                        }
                        else
                        {
                            _23570 = length(_19145);
                        }
                        _23569 = _23570;
                    }
                    else
                    {
                        _23569 = fast::max(_19145.x, _19145.y);
                    }
                    float _19202 = (fast::min(fast::max(_19143.x, _19143.y), 0.0) + _23569) - _23567;
                    float _19207 = fast::max(gFxData_1._data[_4994].y, 9.9999997473787516355514526367188e-05);
                    float _19216 = fast::max(_19207 - abs(_19071 - _19202), 0.0) / _19207;
                    _23579 = fast::min(_19071, _19202) - (((_19216 * _19216) * _19207) * 0.25);
                }
                else
                {
                    _23579 = _19071;
                }
                _23578 = _23579;
            }
            _23577 = _23578;
        }
        float _4428 = (_23577 + gFxData_1._data[_4930].y) / (fast::max(gFxData_1._data[_4930].x * 0.5, _3703 * 0.5) * 1.41421353816986083984375);
        float _19233 = sign(_4428);
        float _19235 = abs(_4428);
        float _19246 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_19235 * _19235))) * _19235)) * _19235);
        float _19249 = _19246 * _19246;
        float _19264 = fast::clamp(gFxData_1._data[_4922].w * ((0.5 + (0.5 * (_19233 - (_19233 / (_19249 * _19249))))) * _3711), 0.0, 1.0);
        _23591 = float4(gFxData_1._data[_4922].xyz * _19264, _19264) + (_23582 * (1.0 - _19264));
    }
    else
    {
        _23591 = _23582;
    }
    float4 _23614;
    if ((_3688 & 16u) != 0u)
    {
        float _4452 = fast::max(-_21796, 0.0) / fast::max(gFxData_1._data[_4946].z, 0.001000000047497451305389404296875);
        float _19290 = fast::clamp(gFxData_1._data[_4938].w * fast::clamp((exp(((-_4452) * _4452) * 2.2000000476837158203125) * gFxData_1._data[_4946].w) * _3711, 0.0, 1.0), 0.0, 1.0);
        _23614 = float4(gFxData_1._data[_4938].xyz * _19290, _19290) + (_23591 * (1.0 - _19290));
    }
    else
    {
        _23614 = _23591;
    }
    float3 _4481 = _23614.xyz + float3((_23596 * _3711) * _23614.w);
    float4 _21676 = _23614;
    _21676.x = _4481.x;
    _21676.y = _4481.y;
    _21676.z = _4481.z;
    float4 _23617;
    if ((_3688 & 2u) != 0u)
    {
        float4 _23615;
        if (gFxData_1._data[_4914].z < 0.999000012874603271484375)
        {
            float2 _4531 = fast::max(gFxData_1._data[_4864].zw - gFxData_1._data[_4864].xy, float2(0.001000000047497451305389404296875));
            float _4542 = cos(gFxData_1._data[_4914].w);
            float _4545 = sin(gFxData_1._data[_4914].w);
            float4 _21693 = _4908;
            _21693.w = _4908.w * mix(1.0, gFxData_1._data[_4914].z, fast::clamp(((dot(in.i_local - ((gFxData_1._data[_4864].xy + gFxData_1._data[_4864].zw) * 0.5), float2(_4542, _4545)) / fast::max(0.5 * ((abs(_4542) * _4531.x) + (abs(_4545) * _4531.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _23615 = _21693;
        }
        else
        {
            _23615 = _4908;
        }
        float _19316 = fast::clamp(_23615.w * (fast::clamp(0.5 - ((_21796 - (gFxData_1._data[_4914].x * gFxData_1._data[_4914].y)) / _3703), 0.0, 1.0) - fast::clamp(0.5 - ((_21796 + (gFxData_1._data[_4914].x * (1.0 - gFxData_1._data[_4914].y))) / _3703), 0.0, 1.0)), 0.0, 1.0);
        _23617 = float4(_23615.xyz * _19316, _19316) + (_21676 * (1.0 - _19316));
    }
    else
    {
        _23617 = _21676;
    }
    float4 _23618;
    if ((_3688 & 128u) != 0u)
    {
        float2 _4606 = (in.i_local - gFxData_1._data[_4864].xy) / fast::max(gFxData_1._data[_4864].zw - gFxData_1._data[_4864].xy, float2(0.001000000047497451305389404296875));
        float _4628 = exp(-pow((((_4606.x * 0.85000002384185791015625) + (_4606.y * 0.1500000059604644775390625)) - ((fract(_172.gTime.x * gFxData_1._data[_5002].w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _23618 = float4(_23617.xyz + float3(((_4628 * gFxData_1._data[_5002].z) * _3711) * fast::max(_23617.w, 0.3499999940395355224609375)), fast::max(_23617.w, ((_4628 * gFxData_1._data[_5002].z) * _3711) * 0.5));
    }
    else
    {
        _23618 = _23617;
    }
    float4 _23942;
    if ((_3688 & 2048u) != 0u)
    {
        float3 _19341 = fract(floor(_21779).xyx * 0.103100001811981201171875);
        float3 _19350 = _19341 + float3(dot(_19341, _19341.yzx + float3(33.3300018310546875)));
        float3 _4679 = _23618.xyz + float3(((fract((_19350.x + _19350.y) * _19350.z) - 0.5) * gFxData_1._data[_5002].y) * _23618.w);
        float4 _21717 = _23618;
        _21717.x = _4679.x;
        _21717.y = _4679.y;
        _21717.z = _4679.z;
        _23942 = _21717;
    }
    else
    {
        _23942 = _23618;
    }
    float _23938;
    if (_215.gFade.z > 0.0)
    {
        _23938 = smoothstep(0.0, 1.0, fast::clamp((_21779.y - _215.gFade.x) / _215.gFade.z, 0.0, 1.0));
    }
    else
    {
        _23938 = 1.0;
    }
    float _23939;
    if (_215.gFade.w > 0.0)
    {
        _23939 = _23938 * smoothstep(0.0, 1.0, fast::clamp((_215.gFade.y - _21779.y) / _215.gFade.w, 0.0, 1.0));
    }
    else
    {
        _23939 = _23938;
    }
    float4 _4696 = _23942 * ((gFxData_1._data[_5002].x * _23629) * _23939);
    float4 _23943;
    if ((_3688 & 12u) != 0u)
    {
        float3 _19402 = fract((floor(_21779) + float2(17.0)).xyx * 0.103100001811981201171875);
        float3 _19411 = _19402 + float3(dot(_19402, _19402.yzx + float3(33.3300018310546875)));
        float3 _4716 = _4696.xyz + float3(((fract((_19411.x + _19411.y) * _19411.z) - 0.5) * 0.0039215688593685626983642578125) * fast::clamp(_4696.w * 8.0, 0.0, 1.0));
        float4 _21729 = _4696;
        _21729.x = _4716.x;
        _21729.y = _4716.y;
        _21729.z = _4716.z;
        _23943 = _21729;
    }
    else
    {
        _23943 = _4696;
    }
    float3 _4726 = fast::max(_23943.xyz, float3(0.0));
    float4 _21735 = _23943;
    _21735.x = _4726.x;
    _21735.y = _4726.y;
    _21735.z = _4726.z;
    float4 _23944;
    if ((_172.gTime.w > 0.5) && (_23943.w > 9.9999997473787516355514526367188e-06))
    {
        float3 _19455 = fast::clamp(_21735.xyz / float3(_23943.w), float3(0.0), float3(1.0));
        float3 _19441 = select(pow((_19455 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _19455 * float3(0.077399380505084991455078125), _19455 <= float3(0.040449999272823333740234375)) * _23943.w;
        float4 _21744 = _21735;
        _21744.x = _19441.x;
        _21744.y = _19441.y;
        _21744.z = _19441.z;
        _23944 = _21744;
    }
    else
    {
        _23944 = _21735;
    }
    out._entryPointOutput = _23944;
    return out;
}

