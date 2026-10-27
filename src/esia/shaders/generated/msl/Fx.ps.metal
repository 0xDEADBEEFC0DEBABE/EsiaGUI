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
constant uint _1416 = (32768u & kEsiaFxFeatures);
constant uint _1485 = (512u & kEsiaFxFeatures);
constant uint _2628 = (512u & kEsiaFxFeatures);
constant uint _2649 = (32768u & kEsiaFxFeatures);
constant uint _2682 = (131072u & kEsiaFxFeatures);
constant uint _2818 = (2u & kEsiaFxFeatures);
constant uint _2828 = (1024u & kEsiaFxFeatures);
constant uint _2895 = (32u & kEsiaFxFeatures);
constant uint _2903 = (4u & kEsiaFxFeatures);
constant uint _2908 = (256u & kEsiaFxFeatures);
constant uint _2973 = (8u & kEsiaFxFeatures);
constant uint _3002 = (8192u & kEsiaFxFeatures);
constant uint _3219 = (65536u & kEsiaFxFeatures);
constant uint _3295 = (32u & kEsiaFxFeatures);
constant uint _3359 = (1u & kEsiaFxFeatures);
constant uint _3388 = (64u & kEsiaFxFeatures);
constant uint _3441 = (16384u & kEsiaFxFeatures);
constant uint _3872 = (4u & kEsiaFxFeatures);
constant uint _3876 = (256u & kEsiaFxFeatures);
constant uint _3936 = (16u & kEsiaFxFeatures);
constant uint _4005 = (2u & kEsiaFxFeatures);
constant uint _4139 = (128u & kEsiaFxFeatures);
constant uint _4221 = (2048u & kEsiaFxFeatures);
constant uint _4261 = (8u & kEsiaFxFeatures);
constant uint _4265 = (4u & kEsiaFxFeatures);

constant spvUnsafeArray<float3, 6> _1986 = spvUnsafeArray<float3, 6>({ float3(1.0, 0.4199999868869781494140625, 0.2199999988079071044921875), float3(1.0, 0.699999988079071044921875, 0.300000011920928955078125), float3(0.800000011920928955078125, 0.920000016689300537109375, 0.4000000059604644775390625), float3(0.3499999940395355224609375, 0.89999997615814208984375, 0.699999988079071044921875), float3(0.4000000059604644775390625, 0.62000000476837158203125, 1.0), float3(0.660000026226043701171875, 0.5, 1.0) });
constant spvUnsafeArray<float4, 4> _3603 = spvUnsafeArray<float4, 4>({ float4(-1.0, 1.0, 3.400000095367431640625, 2.599999904632568359375), float4(-0.550000011920928955078125, 0.800000011920928955078125, 2.0, 0.800000011920928955078125), float4(0.300000011920928955078125, 1.0, 1.2000000476837158203125, 1.2999999523162841796875), float4(0.62000000476837158203125, 0.800000011920928955078125, 1.60000002384185791015625, 0.449999988079071044921875) });
constant spvUnsafeArray<float2, 4> _3620 = spvUnsafeArray<float2, 4>({ float2(0.0), float2(0.100000001490116119384765625, 1.2999999523162841796875), float2(0.550000011920928955078125, 3.900000095367431640625), float2(0.20000000298023223876953125, 5.19999980926513671875) });

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

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _206 [[buffer(0)]], constant WgtDraw& _248 [[buffer(2)]], const device gFxData& gFxData_1 [[buffer(10)]], texture2d<float> gTex [[texture(3)]], texture2d<float> gBackdrop0 [[texture(4)]], texture2d<float> gBackdrop1 [[texture(5)]], texture2d<float> gBackdrop2 [[texture(6)]], texture2d<float> gBackdrop3 [[texture(7)]], texture2d<float> gBackdrop4 [[texture(8)]], texture2d<float> gBackdrop5 [[texture(9)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float2 _6020 = gl_FragCoord.xy + _206.gConv.yy;
    float2 _21219;
    if (_206.gConv.x > 0.5)
    {
        float2 _19774 = _6020;
        _19774.y = _206.gTarget.y - _6020.y;
        _21219 = _19774;
    }
    else
    {
        _21219 = _6020;
    }
    float _4621 = dfdx(in.i_local.x);
    float _4625 = dfdy(in.i_local.x);
    float _4628 = fast::max(abs(_4621) + abs(_4625), 9.9999997473787516355514526367188e-05);
    float4 _21224;
    float4 _21227;
    if ((in.i_flags.z == 2u) || ((in.i_flags.x & _2628) != 0u))
    {
        _21227 = gFxData_1._data[(in.i_instance * 24u) + 15u];
        _21224 = gFxData_1._data[(in.i_instance * 24u) + 16u];
    }
    else
    {
        _21227 = float4(0.0);
        _21224 = float4(0.0);
    }
    float4 _21220;
    float4 _21226;
    if ((in.i_flags.x & _2649) != 0u)
    {
        _21226 = gFxData_1._data[(in.i_instance * 24u) + 15u];
        _21220 = gFxData_1._data[(in.i_instance * 24u) + 18u];
    }
    else
    {
        _21226 = _21227;
        _21220 = float4(0.0);
    }
    float2 _6105 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
    float2 _6114 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
    float _21245;
    if ((in.i_flags.x & _1416) != 0u)
    {
        float _6220 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
        float4 _6254 = gTex.sample(gLinear, mix(_21220.xy, _21220.zw, (fast::clamp(in.i_local, _21226.xy, _21226.zw) - _21226.xy) / fast::max(_21226.zw - _21226.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
        float _6278 = (_6254.w - 0.5) * in.i_radii.y;
        float2 _6319 = abs(in.i_local - ((_21226.xy + _21226.zw) * 0.5)) - fast::max(((_21226.zw - _21226.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _6220), float2(0.0));
        _21245 = fast::max((-mix((fast::max(fast::min(_6254.x, _6254.y), fast::min(fast::max(_6254.x, _6254.y), _6254.z)) - 0.5) * in.i_radii.x, _6278, fast::clamp(abs(_6278) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _6220, length(fast::max(_6319, float2(0.0))) + fast::min(fast::max(_6319.x, _6319.y), 0.0)) - in.i_shape.w;
    }
    else
    {
        float _21246;
        if (in.i_flags.z == 1u)
        {
            float2 _6129 = in.i_local - _6105;
            float _21244;
            do
            {
                if (in.i_shape.w >= 6.282185077667236328125)
                {
                    _21244 = abs(length(_6129) - in.i_radii.x) - in.i_radii.y;
                    break;
                }
                float _6363 = in.i_shape.z + (in.i_shape.w * 0.5);
                float _6365 = cos(_6363);
                float _6367 = sin(_6363);
                float _6376 = dot(_6129, float2(-_6367, _6365));
                float _6379 = dot(_6129, float2(_6365, _6367));
                float2 _6380 = float2(_6376, _6379);
                float _6383 = abs(_6376);
                _6380.x = _6383;
                float _6386 = in.i_shape.w * 0.5;
                float _6388 = sin(_6386);
                float _6390 = cos(_6386);
                _21244 = (((_6390 * _6383) > (_6388 * _6379)) ? length(_6380 - (float2(_6388, _6390) * in.i_radii.x)) : abs(length(_6380) - in.i_radii.x)) - in.i_radii.y;
                break;
            } while(false);
            _21246 = _21244;
        }
        else
        {
            float _21247;
            if (in.i_flags.z == 2u)
            {
                float2 _6426 = in.i_local - _21226.xy;
                float2 _6429 = _21226.zw - _21226.xy;
                _21247 = length(_6426 - (_6429 * fast::clamp(dot(_6426, _6429) / fast::max(dot(_6429, _6429), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
            }
            else
            {
                float2 _6156 = in.i_local - _6105;
                float _6482 = fast::min(_6114.x, _6114.y);
                float _6485 = fast::min((_6156.x > 0.0) ? ((_6156.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_6156.y > 0.0) ? in.i_radii.w : in.i_radii.x), _6482);
                float _6491 = _6485 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                float _21229;
                float _21230;
                if (_6491 > _6482)
                {
                    float _6505 = in.i_shape.y * fast::clamp((_6482 - _6485) / fast::max(0.60000002384185791015625 * _6485, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _21230 = _6505;
                    _21229 = _6485 * (1.0 + (0.60000002384185791015625 * _6505));
                }
                else
                {
                    _21230 = in.i_shape.y;
                    _21229 = _6491;
                }
                float2 _6518 = (abs(_6156) - _6114) + float2(_21229);
                float2 _6520 = fast::max(_6518, float2(0.0));
                float _21231;
                if ((_6520.x > 0.0) && (_6520.y > 0.0))
                {
                    float _21232;
                    if ((_21230 > 0.001000000047497451305389404296875) && (_21229 > 9.9999997473787516355514526367188e-05))
                    {
                        float _6537 = 2.0 + (2.0 * _21230);
                        float2 _6542 = _6520 / float2(fast::max(_21229, 9.9999997473787516355514526367188e-05));
                        _21232 = pow(pow(_6542.x, _6537) + pow(_6542.y, _6537), 1.0 / _6537) * _21229;
                    }
                    else
                    {
                        _21232 = length(_6520);
                    }
                    _21231 = _21232;
                }
                else
                {
                    _21231 = fast::max(_6520.x, _6520.y);
                }
                float _6577 = (fast::min(fast::max(_6518.x, _6518.y), 0.0) + _21231) - _21229;
                float _21248;
                if ((in.i_flags.x & _1485) != 0u)
                {
                    float2 _6184 = fast::max((_21226.zw - _21226.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _6187 = in.i_local - ((_21226.xy + _21226.zw) * 0.5);
                    float _6613 = fast::min(_6184.x, _6184.y);
                    float _6616 = fast::min((_6187.x > 0.0) ? ((_6187.y > 0.0) ? _21224.x : _21224.x) : ((_6187.y > 0.0) ? _21224.x : _21224.x), _6613);
                    float _6622 = _6616 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _21235;
                    float _21236;
                    if (_6622 > _6613)
                    {
                        float _6636 = in.i_shape.y * fast::clamp((_6613 - _6616) / fast::max(0.60000002384185791015625 * _6616, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21236 = _6636;
                        _21235 = _6616 * (1.0 + (0.60000002384185791015625 * _6636));
                    }
                    else
                    {
                        _21236 = in.i_shape.y;
                        _21235 = _6622;
                    }
                    float2 _6649 = (abs(_6187) - _6184) + float2(_21235);
                    float2 _6651 = fast::max(_6649, float2(0.0));
                    float _21237;
                    if ((_6651.x > 0.0) && (_6651.y > 0.0))
                    {
                        float _21238;
                        if ((_21236 > 0.001000000047497451305389404296875) && (_21235 > 9.9999997473787516355514526367188e-05))
                        {
                            float _6668 = 2.0 + (2.0 * _21236);
                            float2 _6673 = _6651 / float2(fast::max(_21235, 9.9999997473787516355514526367188e-05));
                            _21238 = pow(pow(_6673.x, _6668) + pow(_6673.y, _6668), 1.0 / _6668) * _21235;
                        }
                        else
                        {
                            _21238 = length(_6651);
                        }
                        _21237 = _21238;
                    }
                    else
                    {
                        _21237 = fast::max(_6651.x, _6651.y);
                    }
                    float _6708 = (fast::min(fast::max(_6649.x, _6649.y), 0.0) + _21237) - _21235;
                    float _6713 = fast::max(_21224.y, 9.9999997473787516355514526367188e-05);
                    float _6722 = fast::max(_6713 - abs(_6577 - _6708), 0.0) / _6713;
                    _21248 = fast::min(_6577, _6708) - (((_6722 * _6722) * _6713) * 0.25);
                }
                else
                {
                    _21248 = _6577;
                }
                _21247 = _21248;
            }
            _21246 = _21247;
        }
        _21245 = _21246;
    }
    float _21415;
    float _24075;
    if ((in.i_flags.x & _2682) != 0u)
    {
        uint _6746 = (in.i_instance * 24u) + 6u;
        float _4688 = fast::clamp(ceil(length(gFxData_1._data[(in.i_instance * 24u) + 21u].xy) / (4.0 * _4628)), 1.0, 8.0);
        float _4693 = gFxData_1._data[_6746].x * gFxData_1._data[_6746].y;
        float _4699 = gFxData_1._data[_6746].x * (1.0 - gFxData_1._data[_6746].y);
        float2 _4703 = in.i_local + (gFxData_1._data[(in.i_instance * 24u) + 21u].xy * 0.5);
        float2 _6789 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _6798 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _21298;
        if ((in.i_flags.x & _1416) != 0u)
        {
            float _6904 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
            float4 _6938 = gTex.sample(gLinear, mix(_21220.xy, _21220.zw, (fast::clamp(_4703, _21226.xy, _21226.zw) - _21226.xy) / fast::max(_21226.zw - _21226.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
            float _6962 = (_6938.w - 0.5) * in.i_radii.y;
            float2 _7003 = abs(_4703 - ((_21226.xy + _21226.zw) * 0.5)) - fast::max(((_21226.zw - _21226.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _6904), float2(0.0));
            _21298 = fast::max((-mix((fast::max(fast::min(_6938.x, _6938.y), fast::min(fast::max(_6938.x, _6938.y), _6938.z)) - 0.5) * in.i_radii.x, _6962, fast::clamp(abs(_6962) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _6904, length(fast::max(_7003, float2(0.0))) + fast::min(fast::max(_7003.x, _7003.y), 0.0)) - in.i_shape.w;
        }
        else
        {
            float _21299;
            if (in.i_flags.z == 1u)
            {
                float2 _6813 = _4703 - _6789;
                float _21297;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _21297 = abs(length(_6813) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _7047 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _7049 = cos(_7047);
                    float _7051 = sin(_7047);
                    float _7060 = dot(_6813, float2(-_7051, _7049));
                    float _7063 = dot(_6813, float2(_7049, _7051));
                    float2 _7064 = float2(_7060, _7063);
                    float _7067 = abs(_7060);
                    _7064.x = _7067;
                    float _7070 = in.i_shape.w * 0.5;
                    float _7072 = sin(_7070);
                    float _7074 = cos(_7070);
                    _21297 = (((_7074 * _7067) > (_7072 * _7063)) ? length(_7064 - (float2(_7072, _7074) * in.i_radii.x)) : abs(length(_7064) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _21299 = _21297;
            }
            else
            {
                float _21300;
                if (in.i_flags.z == 2u)
                {
                    float2 _7110 = _4703 - _21226.xy;
                    float2 _7113 = _21226.zw - _21226.xy;
                    _21300 = length(_7110 - (_7113 * fast::clamp(dot(_7110, _7113) / fast::max(dot(_7113, _7113), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _6840 = _4703 - _6789;
                    float _7166 = fast::min(_6798.x, _6798.y);
                    float _7169 = fast::min((_6840.x > 0.0) ? ((_6840.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_6840.y > 0.0) ? in.i_radii.w : in.i_radii.x), _7166);
                    float _7175 = _7169 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _21282;
                    float _21283;
                    if (_7175 > _7166)
                    {
                        float _7189 = in.i_shape.y * fast::clamp((_7166 - _7169) / fast::max(0.60000002384185791015625 * _7169, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21283 = _7189;
                        _21282 = _7169 * (1.0 + (0.60000002384185791015625 * _7189));
                    }
                    else
                    {
                        _21283 = in.i_shape.y;
                        _21282 = _7175;
                    }
                    float2 _7202 = (abs(_6840) - _6798) + float2(_21282);
                    float2 _7204 = fast::max(_7202, float2(0.0));
                    float _21284;
                    if ((_7204.x > 0.0) && (_7204.y > 0.0))
                    {
                        float _21285;
                        if ((_21283 > 0.001000000047497451305389404296875) && (_21282 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7221 = 2.0 + (2.0 * _21283);
                            float2 _7226 = _7204 / float2(fast::max(_21282, 9.9999997473787516355514526367188e-05));
                            _21285 = pow(pow(_7226.x, _7221) + pow(_7226.y, _7221), 1.0 / _7221) * _21282;
                        }
                        else
                        {
                            _21285 = length(_7204);
                        }
                        _21284 = _21285;
                    }
                    else
                    {
                        _21284 = fast::max(_7204.x, _7204.y);
                    }
                    float _7261 = (fast::min(fast::max(_7202.x, _7202.y), 0.0) + _21284) - _21282;
                    float _21301;
                    if ((in.i_flags.x & _1485) != 0u)
                    {
                        float2 _6868 = fast::max((_21226.zw - _21226.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _6871 = _4703 - ((_21226.xy + _21226.zw) * 0.5);
                        float _7297 = fast::min(_6868.x, _6868.y);
                        float _7300 = fast::min((_6871.x > 0.0) ? ((_6871.y > 0.0) ? _21224.x : _21224.x) : ((_6871.y > 0.0) ? _21224.x : _21224.x), _7297);
                        float _7306 = _7300 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _21288;
                        float _21289;
                        if (_7306 > _7297)
                        {
                            float _7320 = in.i_shape.y * fast::clamp((_7297 - _7300) / fast::max(0.60000002384185791015625 * _7300, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21289 = _7320;
                            _21288 = _7300 * (1.0 + (0.60000002384185791015625 * _7320));
                        }
                        else
                        {
                            _21289 = in.i_shape.y;
                            _21288 = _7306;
                        }
                        float2 _7333 = (abs(_6871) - _6868) + float2(_21288);
                        float2 _7335 = fast::max(_7333, float2(0.0));
                        float _21290;
                        if ((_7335.x > 0.0) && (_7335.y > 0.0))
                        {
                            float _21291;
                            if ((_21289 > 0.001000000047497451305389404296875) && (_21288 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7352 = 2.0 + (2.0 * _21289);
                                float2 _7357 = _7335 / float2(fast::max(_21288, 9.9999997473787516355514526367188e-05));
                                _21291 = pow(pow(_7357.x, _7352) + pow(_7357.y, _7352), 1.0 / _7352) * _21288;
                            }
                            else
                            {
                                _21291 = length(_7335);
                            }
                            _21290 = _21291;
                        }
                        else
                        {
                            _21290 = fast::max(_7335.x, _7335.y);
                        }
                        float _7392 = (fast::min(fast::max(_7333.x, _7333.y), 0.0) + _21290) - _21288;
                        float _7397 = fast::max(_21224.y, 9.9999997473787516355514526367188e-05);
                        float _7406 = fast::max(_7397 - abs(_7261 - _7392), 0.0) / _7397;
                        _21301 = fast::min(_7261, _7392) - (((_7406 * _7406) * _7397) * 0.25);
                    }
                    else
                    {
                        _21301 = _7261;
                    }
                    _21300 = _21301;
                }
                _21299 = _21300;
            }
            _21298 = _21299;
        }
        float _21372;
        float _21384;
        _21384 = 0.0;
        _21372 = 0.0;
        float _4738;
        float _4767;
        float _21355;
        int _21302 = 1;
        float _21360 = _21298;
        for (; _21302 <= 8; _21384 = _4767, _21372 = _4738, _21360 = _21355, _21302++)
        {
            if (float(_21302) > _4688)
            {
                break;
            }
            float2 _4725 = in.i_local + (gFxData_1._data[(in.i_instance * 24u) + 21u].xy * (0.5 - (float(_21302) / _4688)));
            float2 _7457 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _7466 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            if ((in.i_flags.x & _1416) != 0u)
            {
                float _7572 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                float4 _7606 = gTex.sample(gLinear, mix(_21220.xy, _21220.zw, (fast::clamp(_4725, _21226.xy, _21226.zw) - _21226.xy) / fast::max(_21226.zw - _21226.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                float _7630 = (_7606.w - 0.5) * in.i_radii.y;
                float2 _7671 = abs(_4725 - ((_21226.xy + _21226.zw) * 0.5)) - fast::max(((_21226.zw - _21226.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _7572), float2(0.0));
                _21355 = fast::max((-mix((fast::max(fast::min(_7606.x, _7606.y), fast::min(fast::max(_7606.x, _7606.y), _7606.z)) - 0.5) * in.i_radii.x, _7630, fast::clamp(abs(_7630) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _7572, length(fast::max(_7671, float2(0.0))) + fast::min(fast::max(_7671.x, _7671.y), 0.0)) - in.i_shape.w;
            }
            else
            {
                float _21356;
                if (in.i_flags.z == 1u)
                {
                    float2 _7481 = _4725 - _7457;
                    float _21354;
                    do
                    {
                        if (in.i_shape.w >= 6.282185077667236328125)
                        {
                            _21354 = abs(length(_7481) - in.i_radii.x) - in.i_radii.y;
                            break;
                        }
                        float _7715 = in.i_shape.z + (in.i_shape.w * 0.5);
                        float _7717 = cos(_7715);
                        float _7719 = sin(_7715);
                        float _7728 = dot(_7481, float2(-_7719, _7717));
                        float _7731 = dot(_7481, float2(_7717, _7719));
                        float2 _7732 = float2(_7728, _7731);
                        float _7735 = abs(_7728);
                        _7732.x = _7735;
                        float _7738 = in.i_shape.w * 0.5;
                        float _7740 = sin(_7738);
                        float _7742 = cos(_7738);
                        _21354 = (((_7742 * _7735) > (_7740 * _7731)) ? length(_7732 - (float2(_7740, _7742) * in.i_radii.x)) : abs(length(_7732) - in.i_radii.x)) - in.i_radii.y;
                        break;
                    } while(false);
                    _21356 = _21354;
                }
                else
                {
                    float _21357;
                    if (in.i_flags.z == 2u)
                    {
                        float2 _7778 = _4725 - _21226.xy;
                        float2 _7781 = _21226.zw - _21226.xy;
                        _21357 = length(_7778 - (_7781 * fast::clamp(dot(_7778, _7781) / fast::max(dot(_7781, _7781), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                    }
                    else
                    {
                        float2 _7508 = _4725 - _7457;
                        float _7834 = fast::min(_7466.x, _7466.y);
                        float _7837 = fast::min((_7508.x > 0.0) ? ((_7508.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_7508.y > 0.0) ? in.i_radii.w : in.i_radii.x), _7834);
                        float _7843 = _7837 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _21339;
                        float _21340;
                        if (_7843 > _7834)
                        {
                            float _7857 = in.i_shape.y * fast::clamp((_7834 - _7837) / fast::max(0.60000002384185791015625 * _7837, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21340 = _7857;
                            _21339 = _7837 * (1.0 + (0.60000002384185791015625 * _7857));
                        }
                        else
                        {
                            _21340 = in.i_shape.y;
                            _21339 = _7843;
                        }
                        float2 _7870 = (abs(_7508) - _7466) + float2(_21339);
                        float2 _7872 = fast::max(_7870, float2(0.0));
                        float _21341;
                        if ((_7872.x > 0.0) && (_7872.y > 0.0))
                        {
                            float _21342;
                            if ((_21340 > 0.001000000047497451305389404296875) && (_21339 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7889 = 2.0 + (2.0 * _21340);
                                float2 _7894 = _7872 / float2(fast::max(_21339, 9.9999997473787516355514526367188e-05));
                                _21342 = pow(pow(_7894.x, _7889) + pow(_7894.y, _7889), 1.0 / _7889) * _21339;
                            }
                            else
                            {
                                _21342 = length(_7872);
                            }
                            _21341 = _21342;
                        }
                        else
                        {
                            _21341 = fast::max(_7872.x, _7872.y);
                        }
                        float _7929 = (fast::min(fast::max(_7870.x, _7870.y), 0.0) + _21341) - _21339;
                        float _21358;
                        if ((in.i_flags.x & _1485) != 0u)
                        {
                            float2 _7536 = fast::max((_21226.zw - _21226.xy) * 0.5, float2(0.001000000047497451305389404296875));
                            float2 _7539 = _4725 - ((_21226.xy + _21226.zw) * 0.5);
                            float _7965 = fast::min(_7536.x, _7536.y);
                            float _7968 = fast::min((_7539.x > 0.0) ? ((_7539.y > 0.0) ? _21224.x : _21224.x) : ((_7539.y > 0.0) ? _21224.x : _21224.x), _7965);
                            float _7974 = _7968 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _21345;
                            float _21346;
                            if (_7974 > _7965)
                            {
                                float _7988 = in.i_shape.y * fast::clamp((_7965 - _7968) / fast::max(0.60000002384185791015625 * _7968, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _21346 = _7988;
                                _21345 = _7968 * (1.0 + (0.60000002384185791015625 * _7988));
                            }
                            else
                            {
                                _21346 = in.i_shape.y;
                                _21345 = _7974;
                            }
                            float2 _8001 = (abs(_7539) - _7536) + float2(_21345);
                            float2 _8003 = fast::max(_8001, float2(0.0));
                            float _21347;
                            if ((_8003.x > 0.0) && (_8003.y > 0.0))
                            {
                                float _21348;
                                if ((_21346 > 0.001000000047497451305389404296875) && (_21345 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _8020 = 2.0 + (2.0 * _21346);
                                    float2 _8025 = _8003 / float2(fast::max(_21345, 9.9999997473787516355514526367188e-05));
                                    _21348 = pow(pow(_8025.x, _8020) + pow(_8025.y, _8020), 1.0 / _8020) * _21345;
                                }
                                else
                                {
                                    _21348 = length(_8003);
                                }
                                _21347 = _21348;
                            }
                            else
                            {
                                _21347 = fast::max(_8003.x, _8003.y);
                            }
                            float _8060 = (fast::min(fast::max(_8001.x, _8001.y), 0.0) + _21347) - _21345;
                            float _8065 = fast::max(_21224.y, 9.9999997473787516355514526367188e-05);
                            float _8074 = fast::max(_8065 - abs(_7929 - _8060), 0.0) / _8065;
                            _21358 = fast::min(_7929, _8060) - (((_8074 * _8074) * _8065) * 0.25);
                        }
                        else
                        {
                            _21358 = _7929;
                        }
                        _21357 = _21358;
                    }
                    _21356 = _21357;
                }
                _21355 = _21356;
            }
            float _4730 = _21360 / _4628;
            float _4731 = 0.5 - _4730;
            float _4734 = _21355 / _4628;
            float _4735 = 0.5 - _4734;
            float _8092 = _4735 - _4731;
            _4738 = _21372 + ((abs(_8092) < 0.001000000047497451305389404296875) ? fast::clamp(0.5 * (_4731 + _4735), 0.0, 1.0) : ((((_4735 <= 0.0) ? 0.0 : ((_4735 >= 1.0) ? (-_4734) : ((0.5 * _4735) * _4735))) - ((_4731 <= 0.0) ? 0.0 : ((_4731 >= 1.0) ? (-_4730) : ((0.5 * _4731) * _4731)))) / _8092));
            float _4743 = (_21360 - _4693) / _4628;
            float _4744 = 0.5 - _4743;
            float _4749 = (_21355 - _4693) / _4628;
            float _4750 = 0.5 - _4749;
            float _8144 = _4750 - _4744;
            float _4756 = (_21360 + _4699) / _4628;
            float _4757 = 0.5 - _4756;
            float _4762 = (_21355 + _4699) / _4628;
            float _4763 = 0.5 - _4762;
            float _8196 = _4763 - _4757;
            _4767 = _21384 + (((abs(_8144) < 0.001000000047497451305389404296875) ? fast::clamp(0.5 * (_4744 + _4750), 0.0, 1.0) : ((((_4750 <= 0.0) ? 0.0 : ((_4750 >= 1.0) ? (-_4749) : ((0.5 * _4750) * _4750))) - ((_4744 <= 0.0) ? 0.0 : ((_4744 >= 1.0) ? (-_4743) : ((0.5 * _4744) * _4744)))) / _8144)) - ((abs(_8196) < 0.001000000047497451305389404296875) ? fast::clamp(0.5 * (_4757 + _4763), 0.0, 1.0) : ((((_4763 <= 0.0) ? 0.0 : ((_4763 >= 1.0) ? (-_4762) : ((0.5 * _4763) * _4763))) - ((_4757 <= 0.0) ? 0.0 : ((_4757 >= 1.0) ? (-_4756) : ((0.5 * _4757) * _4757)))) / _8196)));
        }
        _24075 = ((in.i_flags.x & _2818) != 0u) ? (_21384 / _4688) : (-1.0);
        _21415 = _21372 / _4688;
    }
    else
    {
        _24075 = -1.0;
        _21415 = fast::clamp(0.5 - (_21245 / _4628), 0.0, 1.0);
    }
    float _24306;
    if ((in.i_flags.x & _2828) != 0u)
    {
        uint _8246 = (in.i_instance * 24u) + 19u;
        uint _8254 = (in.i_instance * 24u) + 20u;
        float2 _4812 = fast::max((gFxData_1._data[_8246].zw - gFxData_1._data[_8246].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float2 _4815 = in.i_local - ((gFxData_1._data[_8246].xy + gFxData_1._data[_8246].zw) * 0.5);
        float _8292 = fast::min(_4812.x, _4812.y);
        float _8295 = fast::min((_4815.x > 0.0) ? ((_4815.y > 0.0) ? gFxData_1._data[_8254].x : gFxData_1._data[_8254].x) : ((_4815.y > 0.0) ? gFxData_1._data[_8254].x : gFxData_1._data[_8254].x), _8292);
        float _8301 = _8295 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_8254].y));
        float _21408;
        float _21409;
        if (_8301 > _8292)
        {
            float _8315 = gFxData_1._data[_8254].y * fast::clamp((_8292 - _8295) / fast::max(0.60000002384185791015625 * _8295, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _21409 = _8315;
            _21408 = _8295 * (1.0 + (0.60000002384185791015625 * _8315));
        }
        else
        {
            _21409 = gFxData_1._data[_8254].y;
            _21408 = _8301;
        }
        float2 _8328 = (abs(_4815) - _4812) + float2(_21408);
        float2 _8330 = fast::max(_8328, float2(0.0));
        float _21410;
        if ((_8330.x > 0.0) && (_8330.y > 0.0))
        {
            float _21411;
            if ((_21409 > 0.001000000047497451305389404296875) && (_21408 > 9.9999997473787516355514526367188e-05))
            {
                float _8347 = 2.0 + (2.0 * _21409);
                float2 _8352 = _8330 / float2(fast::max(_21408, 9.9999997473787516355514526367188e-05));
                _21411 = pow(pow(_8352.x, _8347) + pow(_8352.y, _8347), 1.0 / _8347) * _21408;
            }
            else
            {
                _21411 = length(_8330);
            }
            _21410 = _21411;
        }
        else
        {
            _21410 = fast::max(_8330.x, _8330.y);
        }
        float _4827 = fast::clamp(0.5 - (((fast::min(fast::max(_8328.x, _8328.y), 0.0) + _21410) - _21408) / _4628), 0.0, 1.0);
        if (_4827 <= 0.0)
        {
            discard_fragment();
        }
        _24306 = _4827;
    }
    else
    {
        _24306 = 1.0;
    }
    bool _4838 = ((in.i_flags.x & _2895) != 0u) && (_21415 >= 0.999000012874603271484375);
    float4 _21499;
    if ((((in.i_flags.x & _2903) != 0u) && (!((in.i_flags.x & _2908) != 0u))) && (!_4838))
    {
        uint _8393 = (in.i_instance * 24u) + 7u;
        uint _8401 = (in.i_instance * 24u) + 8u;
        float2 _8448 = select(_248.gDrawInfo.yz, _248.gDrawInfo.yz - ((in.i_rect.xy + in.i_rect.zw) * 0.5), bool2(_248.gDrawInfo.w > 1.0));
        float2 _4868 = in.i_local - select(gFxData_1._data[_8401].zw, (-(_8448 / float2(fast::max(length(_8448), 9.9999997473787516355514526367188e-05)))) * length(gFxData_1._data[_8401].zw), bool2(_248.gDrawInfo.w > 0.5));
        float2 _8495 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _8504 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _21453;
        if ((in.i_flags.x & _1416) != 0u)
        {
            float _8610 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
            float4 _8644 = gTex.sample(gLinear, mix(_21220.xy, _21220.zw, (fast::clamp(_4868, _21226.xy, _21226.zw) - _21226.xy) / fast::max(_21226.zw - _21226.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
            float _8668 = (_8644.w - 0.5) * in.i_radii.y;
            float2 _8709 = abs(_4868 - ((_21226.xy + _21226.zw) * 0.5)) - fast::max(((_21226.zw - _21226.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _8610), float2(0.0));
            _21453 = fast::max((-mix((fast::max(fast::min(_8644.x, _8644.y), fast::min(fast::max(_8644.x, _8644.y), _8644.z)) - 0.5) * in.i_radii.x, _8668, fast::clamp(abs(_8668) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _8610, length(fast::max(_8709, float2(0.0))) + fast::min(fast::max(_8709.x, _8709.y), 0.0)) - in.i_shape.w;
        }
        else
        {
            float _21454;
            if (in.i_flags.z == 1u)
            {
                float2 _8519 = _4868 - _8495;
                float _21452;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _21452 = abs(length(_8519) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _8753 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _8755 = cos(_8753);
                    float _8757 = sin(_8753);
                    float _8766 = dot(_8519, float2(-_8757, _8755));
                    float _8769 = dot(_8519, float2(_8755, _8757));
                    float2 _8770 = float2(_8766, _8769);
                    float _8773 = abs(_8766);
                    _8770.x = _8773;
                    float _8776 = in.i_shape.w * 0.5;
                    float _8778 = sin(_8776);
                    float _8780 = cos(_8776);
                    _21452 = (((_8780 * _8773) > (_8778 * _8769)) ? length(_8770 - (float2(_8778, _8780) * in.i_radii.x)) : abs(length(_8770) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _21454 = _21452;
            }
            else
            {
                float _21455;
                if (in.i_flags.z == 2u)
                {
                    float2 _8816 = _4868 - _21226.xy;
                    float2 _8819 = _21226.zw - _21226.xy;
                    _21455 = length(_8816 - (_8819 * fast::clamp(dot(_8816, _8819) / fast::max(dot(_8819, _8819), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _8546 = _4868 - _8495;
                    float _8872 = fast::min(_8504.x, _8504.y);
                    float _8875 = fast::min((_8546.x > 0.0) ? ((_8546.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_8546.y > 0.0) ? in.i_radii.w : in.i_radii.x), _8872);
                    float _8881 = _8875 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _21437;
                    float _21438;
                    if (_8881 > _8872)
                    {
                        float _8895 = in.i_shape.y * fast::clamp((_8872 - _8875) / fast::max(0.60000002384185791015625 * _8875, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _21438 = _8895;
                        _21437 = _8875 * (1.0 + (0.60000002384185791015625 * _8895));
                    }
                    else
                    {
                        _21438 = in.i_shape.y;
                        _21437 = _8881;
                    }
                    float2 _8908 = (abs(_8546) - _8504) + float2(_21437);
                    float2 _8910 = fast::max(_8908, float2(0.0));
                    float _21439;
                    if ((_8910.x > 0.0) && (_8910.y > 0.0))
                    {
                        float _21440;
                        if ((_21438 > 0.001000000047497451305389404296875) && (_21437 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8927 = 2.0 + (2.0 * _21438);
                            float2 _8932 = _8910 / float2(fast::max(_21437, 9.9999997473787516355514526367188e-05));
                            _21440 = pow(pow(_8932.x, _8927) + pow(_8932.y, _8927), 1.0 / _8927) * _21437;
                        }
                        else
                        {
                            _21440 = length(_8910);
                        }
                        _21439 = _21440;
                    }
                    else
                    {
                        _21439 = fast::max(_8910.x, _8910.y);
                    }
                    float _8967 = (fast::min(fast::max(_8908.x, _8908.y), 0.0) + _21439) - _21437;
                    float _21456;
                    if ((in.i_flags.x & _1485) != 0u)
                    {
                        float2 _8574 = fast::max((_21226.zw - _21226.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _8577 = _4868 - ((_21226.xy + _21226.zw) * 0.5);
                        float _9003 = fast::min(_8574.x, _8574.y);
                        float _9006 = fast::min((_8577.x > 0.0) ? ((_8577.y > 0.0) ? _21224.x : _21224.x) : ((_8577.y > 0.0) ? _21224.x : _21224.x), _9003);
                        float _9012 = _9006 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _21443;
                        float _21444;
                        if (_9012 > _9003)
                        {
                            float _9026 = in.i_shape.y * fast::clamp((_9003 - _9006) / fast::max(0.60000002384185791015625 * _9006, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21444 = _9026;
                            _21443 = _9006 * (1.0 + (0.60000002384185791015625 * _9026));
                        }
                        else
                        {
                            _21444 = in.i_shape.y;
                            _21443 = _9012;
                        }
                        float2 _9039 = (abs(_8577) - _8574) + float2(_21443);
                        float2 _9041 = fast::max(_9039, float2(0.0));
                        float _21445;
                        if ((_9041.x > 0.0) && (_9041.y > 0.0))
                        {
                            float _21446;
                            if ((_21444 > 0.001000000047497451305389404296875) && (_21443 > 9.9999997473787516355514526367188e-05))
                            {
                                float _9058 = 2.0 + (2.0 * _21444);
                                float2 _9063 = _9041 / float2(fast::max(_21443, 9.9999997473787516355514526367188e-05));
                                _21446 = pow(pow(_9063.x, _9058) + pow(_9063.y, _9058), 1.0 / _9058) * _21443;
                            }
                            else
                            {
                                _21446 = length(_9041);
                            }
                            _21445 = _21446;
                        }
                        else
                        {
                            _21445 = fast::max(_9041.x, _9041.y);
                        }
                        float _9098 = (fast::min(fast::max(_9039.x, _9039.y), 0.0) + _21445) - _21443;
                        float _9103 = fast::max(_21224.y, 9.9999997473787516355514526367188e-05);
                        float _9112 = fast::max(_9103 - abs(_8967 - _9098), 0.0) / _9103;
                        _21456 = fast::min(_8967, _9098) - (((_9112 * _9112) * _9103) * 0.25);
                    }
                    else
                    {
                        _21456 = _8967;
                    }
                    _21455 = _21456;
                }
                _21454 = _21455;
            }
            _21453 = _21454;
        }
        float _4877 = (_21453 - gFxData_1._data[_8401].y) / (fast::max(gFxData_1._data[_8401].x * 0.5, _4628 * 0.5) * 1.41421353816986083984375);
        float _9129 = sign(_4877);
        float _9131 = abs(_4877);
        float _9142 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_9131 * _9131))) * _9131)) * _9131);
        float _9145 = _9142 * _9142;
        float _9160 = fast::clamp(gFxData_1._data[_8393].w * (0.5 - (0.5 * (_9129 - (_9129 / (_9145 * _9145))))), 0.0, 1.0);
        _21499 = float4(gFxData_1._data[_8393].xyz * _9160, _9160);
    }
    else
    {
        _21499 = float4(0.0);
    }
    float4 _21599;
    if (((in.i_flags.x & _2973) != 0u) && (!_4838))
    {
        uint _9184 = (in.i_instance * 24u) + 9u;
        uint _9192 = (in.i_instance * 24u) + 10u;
        float _4905 = fast::max(gFxData_1._data[_9192].x, 0.001000000047497451305389404296875);
        float _21492;
        float _21495;
        if ((in.i_flags.x & _3002) != 0u)
        {
            uint _9200 = (in.i_instance * 24u) + 22u;
            float2 _4921 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _4930 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float2 _4937 = in.i_rect.xy - gFxData_1._data[_9200].xy;
            float2 _4944 = gFxData_1._data[_9200].zw - in.i_rect.zw;
            float2 _4974 = fast::clamp(float2((in.i_local.x < _4921.x) ? _4937.x : _4944.x, (in.i_local.y < _4921.y) ? _4937.y : _4944.y) * float2(0.58823525905609130859375), float2(fast::min(_4905, 1.5)), float2(_4905));
            float _4979 = fast::min(_4930.x, _4930.y);
            float _21490;
            if (in.i_flags.z == 0u)
            {
                _21490 = fast::min(((in.i_local.x > _4921.x) ? ((in.i_local.y > _4921.y) ? in.i_radii.z : in.i_radii.y) : ((in.i_local.y > _4921.y) ? in.i_radii.w : in.i_radii.x)) * (1.0 + (0.60000002384185791015625 * in.i_shape.y)), _4979);
            }
            else
            {
                _21490 = _4979;
            }
            float2 _5028 = fast::max(abs(in.i_local - _4921) - (_4930 - float2(_21490)), float2(0.0));
            float _5030 = length(_5028);
            float2 _5038 = select(float2(0.707099974155426025390625), _5028 / float2(_5030), bool2(_5030 > 9.9999997473787516355514526367188e-05));
            float2 _5053 = in.i_local - gFxData_1._data[_9200].xy;
            float2 _5058 = gFxData_1._data[_9200].zw - in.i_local;
            _21495 = fast::clamp(fast::min(fast::min(_5053.x, _5053.y), fast::min(_5058.x, _5058.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _21492 = rsqrt(dot(_5038 * _5038, float2(1.0) / (_4974 * _4974)));
        }
        else
        {
            _21495 = 1.0;
            _21492 = _4905;
        }
        float _5076 = fast::max(_21245, 0.0) / _21492;
        float _9210 = fast::clamp(gFxData_1._data[_9184].w * fast::clamp((exp(((-_5076) * _5076) * 2.2000000476837158203125) * gFxData_1._data[_9192].y) * _21495, 0.0, 1.0), 0.0, 1.0);
        _21599 = float4(gFxData_1._data[_9184].xyz * _9210, _9210) + (_21499 * (1.0 - _9210));
    }
    else
    {
        _21599 = _21499;
    }
    float4 _22787;
    if ((in.i_flags.x & _3219) != 0u)
    {
        float2 _5107 = _21219 * _206.gTarget.zw;
        float _9243 = fast::clamp(log2(fast::max(fast::max(in.i_radii.w / _4628, 4.0), 1.0)) - 1.0, 0.0, 5.0);
        int _9246 = int(floor(_9243));
        float _9250 = _9243 - float(_9246);
        bool _9255 = (_9250 > 0.0199999995529651641845703125) && (_9246 < 5);
        float3 _21571;
        _21571 = float3(0.0);
        float3 _9284;
        for (int _21501 = 0; _21501 < 2; _21571 = _9284, _21501++)
        {
            if ((_21501 > 0) && (!_9255))
            {
                break;
            }
            int _9270 = _9246 + _21501;
            int _9308 = clamp(_9270, 1, 5);
            float2 _9377 = (_5107 * _206.gLevel[_9308].xy) - float2(0.5);
            float2 _9379 = floor(_9377);
            float2 _9382 = _9377 - _9379;
            float2 _9385 = _9382 * _9382;
            float2 _9388 = _9385 * _9382;
            float2 _9407 = (((_9388 * 3.0) - (_9385 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
            float2 _9420 = _9388 * 0.16666667163372039794921875;
            float2 _9423 = (((((-_9388) + (_9385 * 3.0)) - (_9382 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9407;
            float2 _9427 = (((((_9388 * (-3.0)) + (_9385 * 3.0)) + (_9382 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _9420;
            float2 _9439 = ((_9379 - float2(0.5)) + (_9407 / _9423)) * _206.gLevel[_9308].zw;
            float2 _9451 = ((_9379 + float2(1.5)) + (_9420 / _9427)) * _206.gLevel[_9308].zw;
            float4 _21538;
            if (_9270 <= 0)
            {
                float2 _21537;
                if (_206.gConv.x > 0.5)
                {
                    float2 _20109 = _5107;
                    _20109.y = 1.0 - _5107.y;
                    _21537 = _20109;
                }
                else
                {
                    _21537 = _5107;
                }
                _21538 = gBackdrop0.sample(gLinear, _21537, level(0.0));
            }
            else
            {
                float4 _21539;
                if (_9270 == 1)
                {
                    float2 _21530;
                    if (_206.gConv.x > 0.5)
                    {
                        float2 _20114 = _9439;
                        _20114.y = 1.0 - _9439.y;
                        _21530 = _20114;
                    }
                    else
                    {
                        _21530 = _9439;
                    }
                    float _9496 = _9439.y;
                    float2 _9497 = float2(_9451.x, _9496);
                    float2 _21531;
                    if (_206.gConv.x > 0.5)
                    {
                        float2 _20121 = _9497;
                        _20121.y = 1.0 - _9496;
                        _21531 = _20121;
                    }
                    else
                    {
                        _21531 = _9497;
                    }
                    float _9514 = _9451.y;
                    float2 _9515 = float2(_9439.x, _9514);
                    float2 _21533;
                    if (_206.gConv.x > 0.5)
                    {
                        float2 _20128 = _9515;
                        _20128.y = 1.0 - _9514;
                        _21533 = _20128;
                    }
                    else
                    {
                        _21533 = _9515;
                    }
                    float2 _21535;
                    if (_206.gConv.x > 0.5)
                    {
                        float2 _20135 = _9451;
                        _20135.y = 1.0 - _9451.y;
                        _21535 = _20135;
                    }
                    else
                    {
                        _21535 = _9451;
                    }
                    _21539 = (((gBackdrop1.sample(gLinear, _21530, level(0.0)) * (_9423.x * _9423.y)) + (gBackdrop1.sample(gLinear, _21531, level(0.0)) * (_9427.x * _9423.y))) + (gBackdrop1.sample(gLinear, _21533, level(0.0)) * (_9423.x * _9427.y))) + (gBackdrop1.sample(gLinear, _21535, level(0.0)) * (_9427.x * _9427.y));
                }
                else
                {
                    float4 _21540;
                    if (_9270 == 2)
                    {
                        float2 _21523;
                        if (_206.gConv.x > 0.5)
                        {
                            float2 _20142 = _9439;
                            _20142.y = 1.0 - _9439.y;
                            _21523 = _20142;
                        }
                        else
                        {
                            _21523 = _9439;
                        }
                        float _9626 = _9439.y;
                        float2 _9627 = float2(_9451.x, _9626);
                        float2 _21524;
                        if (_206.gConv.x > 0.5)
                        {
                            float2 _20149 = _9627;
                            _20149.y = 1.0 - _9626;
                            _21524 = _20149;
                        }
                        else
                        {
                            _21524 = _9627;
                        }
                        float _9644 = _9451.y;
                        float2 _9645 = float2(_9439.x, _9644);
                        float2 _21526;
                        if (_206.gConv.x > 0.5)
                        {
                            float2 _20156 = _9645;
                            _20156.y = 1.0 - _9644;
                            _21526 = _20156;
                        }
                        else
                        {
                            _21526 = _9645;
                        }
                        float2 _21528;
                        if (_206.gConv.x > 0.5)
                        {
                            float2 _20163 = _9451;
                            _20163.y = 1.0 - _9451.y;
                            _21528 = _20163;
                        }
                        else
                        {
                            _21528 = _9451;
                        }
                        _21540 = (((gBackdrop2.sample(gLinear, _21523, level(0.0)) * (_9423.x * _9423.y)) + (gBackdrop2.sample(gLinear, _21524, level(0.0)) * (_9427.x * _9423.y))) + (gBackdrop2.sample(gLinear, _21526, level(0.0)) * (_9423.x * _9427.y))) + (gBackdrop2.sample(gLinear, _21528, level(0.0)) * (_9427.x * _9427.y));
                    }
                    else
                    {
                        float4 _21541;
                        if (_9270 == 3)
                        {
                            float2 _21516;
                            if (_206.gConv.x > 0.5)
                            {
                                float2 _20170 = _9439;
                                _20170.y = 1.0 - _9439.y;
                                _21516 = _20170;
                            }
                            else
                            {
                                _21516 = _9439;
                            }
                            float _9756 = _9439.y;
                            float2 _9757 = float2(_9451.x, _9756);
                            float2 _21517;
                            if (_206.gConv.x > 0.5)
                            {
                                float2 _20177 = _9757;
                                _20177.y = 1.0 - _9756;
                                _21517 = _20177;
                            }
                            else
                            {
                                _21517 = _9757;
                            }
                            float _9774 = _9451.y;
                            float2 _9775 = float2(_9439.x, _9774);
                            float2 _21519;
                            if (_206.gConv.x > 0.5)
                            {
                                float2 _20184 = _9775;
                                _20184.y = 1.0 - _9774;
                                _21519 = _20184;
                            }
                            else
                            {
                                _21519 = _9775;
                            }
                            float2 _21521;
                            if (_206.gConv.x > 0.5)
                            {
                                float2 _20191 = _9451;
                                _20191.y = 1.0 - _9451.y;
                                _21521 = _20191;
                            }
                            else
                            {
                                _21521 = _9451;
                            }
                            _21541 = (((gBackdrop3.sample(gLinear, _21516, level(0.0)) * (_9423.x * _9423.y)) + (gBackdrop3.sample(gLinear, _21517, level(0.0)) * (_9427.x * _9423.y))) + (gBackdrop3.sample(gLinear, _21519, level(0.0)) * (_9423.x * _9427.y))) + (gBackdrop3.sample(gLinear, _21521, level(0.0)) * (_9427.x * _9427.y));
                        }
                        else
                        {
                            float4 _21542;
                            if (_9270 == 4)
                            {
                                float2 _21509;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20198 = _9439;
                                    _20198.y = 1.0 - _9439.y;
                                    _21509 = _20198;
                                }
                                else
                                {
                                    _21509 = _9439;
                                }
                                float _9886 = _9439.y;
                                float2 _9887 = float2(_9451.x, _9886);
                                float2 _21510;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20205 = _9887;
                                    _20205.y = 1.0 - _9886;
                                    _21510 = _20205;
                                }
                                else
                                {
                                    _21510 = _9887;
                                }
                                float _9904 = _9451.y;
                                float2 _9905 = float2(_9439.x, _9904);
                                float2 _21512;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20212 = _9905;
                                    _20212.y = 1.0 - _9904;
                                    _21512 = _20212;
                                }
                                else
                                {
                                    _21512 = _9905;
                                }
                                float2 _21514;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20219 = _9451;
                                    _20219.y = 1.0 - _9451.y;
                                    _21514 = _20219;
                                }
                                else
                                {
                                    _21514 = _9451;
                                }
                                _21542 = (((gBackdrop4.sample(gLinear, _21509, level(0.0)) * (_9423.x * _9423.y)) + (gBackdrop4.sample(gLinear, _21510, level(0.0)) * (_9427.x * _9423.y))) + (gBackdrop4.sample(gLinear, _21512, level(0.0)) * (_9423.x * _9427.y))) + (gBackdrop4.sample(gLinear, _21514, level(0.0)) * (_9427.x * _9427.y));
                            }
                            else
                            {
                                float2 _21502;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20226 = _9439;
                                    _20226.y = 1.0 - _9439.y;
                                    _21502 = _20226;
                                }
                                else
                                {
                                    _21502 = _9439;
                                }
                                float _10016 = _9439.y;
                                float2 _10017 = float2(_9451.x, _10016);
                                float2 _21503;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20233 = _10017;
                                    _20233.y = 1.0 - _10016;
                                    _21503 = _20233;
                                }
                                else
                                {
                                    _21503 = _10017;
                                }
                                float _10034 = _9451.y;
                                float2 _10035 = float2(_9439.x, _10034);
                                float2 _21505;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20240 = _10035;
                                    _20240.y = 1.0 - _10034;
                                    _21505 = _20240;
                                }
                                else
                                {
                                    _21505 = _10035;
                                }
                                float2 _21507;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20247 = _9451;
                                    _20247.y = 1.0 - _9451.y;
                                    _21507 = _20247;
                                }
                                else
                                {
                                    _21507 = _9451;
                                }
                                _21542 = (((gBackdrop5.sample(gLinear, _21502, level(0.0)) * (_9423.x * _9423.y)) + (gBackdrop5.sample(gLinear, _21503, level(0.0)) * (_9427.x * _9423.y))) + (gBackdrop5.sample(gLinear, _21505, level(0.0)) * (_9423.x * _9427.y))) + (gBackdrop5.sample(gLinear, _21507, level(0.0)) * (_9427.x * _9427.y));
                            }
                            _21541 = _21542;
                        }
                        _21540 = _21541;
                    }
                    _21539 = _21540;
                }
                _21538 = _21539;
            }
            _9284 = _21571 + (_21538.xyz * (_9255 ? ((_21501 == 0) ? (1.0 - _9250) : _9250) : 1.0));
        }
        float _10120 = dot(in.i_fill0.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
        float _5137 = fast::max(_21245, 0.0) / fast::max(in.i_radii.w, 0.001000000047497451305389404296875);
        float _5140 = (_10120 > 0.5) ? 0.0 : 1.0;
        float _10132 = fast::clamp(0.85000002384185791015625 * ((fast::clamp((0.5 - abs(_10120 - dot(select(float3(0.5), _21571, bool3(_206.gTime.z > 0.5)), float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)))) * 4.0, 0.0, 1.0) * in.i_radii.z) * exp(((-_5137) * _5137) * 2.2000000476837158203125)), 0.0, 1.0);
        _22787 = float4(float4(_5140, _5140, _5140, 0.85000002384185791015625).xyz * _10132, _10132) + (_21599 * (1.0 - _10132));
    }
    else
    {
        _22787 = _21599;
    }
    float4 _23432;
    float4 _23608;
    float _24044;
    if (((in.i_flags.x & _3295) != 0u) && (_21415 > 0.0))
    {
        uint _10156 = (in.i_instance * 24u) + 11u;
        uint _10164 = (in.i_instance * 24u) + 12u;
        uint _10172 = (in.i_instance * 24u) + 13u;
        uint _10180 = (in.i_instance * 24u) + 16u;
        float _10253 = gFxData_1._data[_10156].x * _206.gDisplay.z;
        float2 _10264 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(1.0));
        float _10272 = fast::clamp(gFxData_1._data[_10156].z, 0.001000000047497451305389404296875, fast::min(_10264.x, _10264.y));
        float _10281 = fast::clamp(1.0 - (fast::max(-_21245, 0.0) / _10272), 0.0, 1.0);
        float _10287 = sqrt(fast::clamp(1.0 - (_10281 * _10281), 0.0, 1.0));
        float2 _21838;
        float3 _22415;
        if (_10281 > 0.0)
        {
            float2 _10699 = in.i_local + float2(0.5, 0.0);
            float2 _10769 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _10778 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _21774;
            if ((in.i_flags.x & _1416) != 0u)
            {
                float _10884 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                float4 _10918 = gTex.sample(gLinear, mix(_21220.xy, _21220.zw, (fast::clamp(_10699, _21226.xy, _21226.zw) - _21226.xy) / fast::max(_21226.zw - _21226.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                float _10942 = (_10918.w - 0.5) * in.i_radii.y;
                float2 _10983 = abs(_10699 - ((_21226.xy + _21226.zw) * 0.5)) - fast::max(((_21226.zw - _21226.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _10884), float2(0.0));
                _21774 = fast::max((-mix((fast::max(fast::min(_10918.x, _10918.y), fast::min(fast::max(_10918.x, _10918.y), _10918.z)) - 0.5) * in.i_radii.x, _10942, fast::clamp(abs(_10942) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _10884, length(fast::max(_10983, float2(0.0))) + fast::min(fast::max(_10983.x, _10983.y), 0.0)) - in.i_shape.w;
            }
            else
            {
                float _21775;
                if (in.i_flags.z == 1u)
                {
                    float2 _10793 = _10699 - _10769;
                    float _21773;
                    do
                    {
                        if (in.i_shape.w >= 6.282185077667236328125)
                        {
                            _21773 = abs(length(_10793) - in.i_radii.x) - in.i_radii.y;
                            break;
                        }
                        float _11027 = in.i_shape.z + (in.i_shape.w * 0.5);
                        float _11029 = cos(_11027);
                        float _11031 = sin(_11027);
                        float _11040 = dot(_10793, float2(-_11031, _11029));
                        float _11043 = dot(_10793, float2(_11029, _11031));
                        float2 _11044 = float2(_11040, _11043);
                        float _11047 = abs(_11040);
                        _11044.x = _11047;
                        float _11050 = in.i_shape.w * 0.5;
                        float _11052 = sin(_11050);
                        float _11054 = cos(_11050);
                        _21773 = (((_11054 * _11047) > (_11052 * _11043)) ? length(_11044 - (float2(_11052, _11054) * in.i_radii.x)) : abs(length(_11044) - in.i_radii.x)) - in.i_radii.y;
                        break;
                    } while(false);
                    _21775 = _21773;
                }
                else
                {
                    float _21776;
                    if (in.i_flags.z == 2u)
                    {
                        float2 _11090 = _10699 - _21226.xy;
                        float2 _11093 = _21226.zw - _21226.xy;
                        _21776 = length(_11090 - (_11093 * fast::clamp(dot(_11090, _11093) / fast::max(dot(_11093, _11093), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                    }
                    else
                    {
                        float2 _10820 = _10699 - _10769;
                        float _11146 = fast::min(_10778.x, _10778.y);
                        float _11149 = fast::min((_10820.x > 0.0) ? ((_10820.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_10820.y > 0.0) ? in.i_radii.w : in.i_radii.x), _11146);
                        float _11155 = _11149 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _21758;
                        float _21759;
                        if (_11155 > _11146)
                        {
                            float _11169 = in.i_shape.y * fast::clamp((_11146 - _11149) / fast::max(0.60000002384185791015625 * _11149, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21759 = _11169;
                            _21758 = _11149 * (1.0 + (0.60000002384185791015625 * _11169));
                        }
                        else
                        {
                            _21759 = in.i_shape.y;
                            _21758 = _11155;
                        }
                        float2 _11182 = (abs(_10820) - _10778) + float2(_21758);
                        float2 _11184 = fast::max(_11182, float2(0.0));
                        float _21760;
                        if ((_11184.x > 0.0) && (_11184.y > 0.0))
                        {
                            float _21761;
                            if ((_21759 > 0.001000000047497451305389404296875) && (_21758 > 9.9999997473787516355514526367188e-05))
                            {
                                float _11201 = 2.0 + (2.0 * _21759);
                                float2 _11206 = _11184 / float2(fast::max(_21758, 9.9999997473787516355514526367188e-05));
                                _21761 = pow(pow(_11206.x, _11201) + pow(_11206.y, _11201), 1.0 / _11201) * _21758;
                            }
                            else
                            {
                                _21761 = length(_11184);
                            }
                            _21760 = _21761;
                        }
                        else
                        {
                            _21760 = fast::max(_11184.x, _11184.y);
                        }
                        float _11241 = (fast::min(fast::max(_11182.x, _11182.y), 0.0) + _21760) - _21758;
                        float _21777;
                        if ((in.i_flags.x & _1485) != 0u)
                        {
                            float2 _10848 = fast::max((_21226.zw - _21226.xy) * 0.5, float2(0.001000000047497451305389404296875));
                            float2 _10851 = _10699 - ((_21226.xy + _21226.zw) * 0.5);
                            float _11277 = fast::min(_10848.x, _10848.y);
                            float _11280 = fast::min((_10851.x > 0.0) ? ((_10851.y > 0.0) ? gFxData_1._data[_10180].x : gFxData_1._data[_10180].x) : ((_10851.y > 0.0) ? gFxData_1._data[_10180].x : gFxData_1._data[_10180].x), _11277);
                            float _11286 = _11280 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _21764;
                            float _21765;
                            if (_11286 > _11277)
                            {
                                float _11300 = in.i_shape.y * fast::clamp((_11277 - _11280) / fast::max(0.60000002384185791015625 * _11280, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _21765 = _11300;
                                _21764 = _11280 * (1.0 + (0.60000002384185791015625 * _11300));
                            }
                            else
                            {
                                _21765 = in.i_shape.y;
                                _21764 = _11286;
                            }
                            float2 _11313 = (abs(_10851) - _10848) + float2(_21764);
                            float2 _11315 = fast::max(_11313, float2(0.0));
                            float _21766;
                            if ((_11315.x > 0.0) && (_11315.y > 0.0))
                            {
                                float _21767;
                                if ((_21765 > 0.001000000047497451305389404296875) && (_21764 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _11332 = 2.0 + (2.0 * _21765);
                                    float2 _11337 = _11315 / float2(fast::max(_21764, 9.9999997473787516355514526367188e-05));
                                    _21767 = pow(pow(_11337.x, _11332) + pow(_11337.y, _11332), 1.0 / _11332) * _21764;
                                }
                                else
                                {
                                    _21767 = length(_11315);
                                }
                                _21766 = _21767;
                            }
                            else
                            {
                                _21766 = fast::max(_11315.x, _11315.y);
                            }
                            float _11372 = (fast::min(fast::max(_11313.x, _11313.y), 0.0) + _21766) - _21764;
                            float _11377 = fast::max(gFxData_1._data[_10180].y, 9.9999997473787516355514526367188e-05);
                            float _11386 = fast::max(_11377 - abs(_11241 - _11372), 0.0) / _11377;
                            _21777 = fast::min(_11241, _11372) - (((_11386 * _11386) * _11377) * 0.25);
                        }
                        else
                        {
                            _21777 = _11241;
                        }
                        _21776 = _21777;
                    }
                    _21775 = _21776;
                }
                _21774 = _21775;
            }
            float2 _10703 = in.i_local - float2(0.5, 0.0);
            float2 _11437 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _11446 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _21794;
            if ((in.i_flags.x & _1416) != 0u)
            {
                float _11552 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                float4 _11586 = gTex.sample(gLinear, mix(_21220.xy, _21220.zw, (fast::clamp(_10703, _21226.xy, _21226.zw) - _21226.xy) / fast::max(_21226.zw - _21226.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                float _11610 = (_11586.w - 0.5) * in.i_radii.y;
                float2 _11651 = abs(_10703 - ((_21226.xy + _21226.zw) * 0.5)) - fast::max(((_21226.zw - _21226.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _11552), float2(0.0));
                _21794 = fast::max((-mix((fast::max(fast::min(_11586.x, _11586.y), fast::min(fast::max(_11586.x, _11586.y), _11586.z)) - 0.5) * in.i_radii.x, _11610, fast::clamp(abs(_11610) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _11552, length(fast::max(_11651, float2(0.0))) + fast::min(fast::max(_11651.x, _11651.y), 0.0)) - in.i_shape.w;
            }
            else
            {
                float _21795;
                if (in.i_flags.z == 1u)
                {
                    float2 _11461 = _10703 - _11437;
                    float _21793;
                    do
                    {
                        if (in.i_shape.w >= 6.282185077667236328125)
                        {
                            _21793 = abs(length(_11461) - in.i_radii.x) - in.i_radii.y;
                            break;
                        }
                        float _11695 = in.i_shape.z + (in.i_shape.w * 0.5);
                        float _11697 = cos(_11695);
                        float _11699 = sin(_11695);
                        float _11708 = dot(_11461, float2(-_11699, _11697));
                        float _11711 = dot(_11461, float2(_11697, _11699));
                        float2 _11712 = float2(_11708, _11711);
                        float _11715 = abs(_11708);
                        _11712.x = _11715;
                        float _11718 = in.i_shape.w * 0.5;
                        float _11720 = sin(_11718);
                        float _11722 = cos(_11718);
                        _21793 = (((_11722 * _11715) > (_11720 * _11711)) ? length(_11712 - (float2(_11720, _11722) * in.i_radii.x)) : abs(length(_11712) - in.i_radii.x)) - in.i_radii.y;
                        break;
                    } while(false);
                    _21795 = _21793;
                }
                else
                {
                    float _21796;
                    if (in.i_flags.z == 2u)
                    {
                        float2 _11758 = _10703 - _21226.xy;
                        float2 _11761 = _21226.zw - _21226.xy;
                        _21796 = length(_11758 - (_11761 * fast::clamp(dot(_11758, _11761) / fast::max(dot(_11761, _11761), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                    }
                    else
                    {
                        float2 _11488 = _10703 - _11437;
                        float _11814 = fast::min(_11446.x, _11446.y);
                        float _11817 = fast::min((_11488.x > 0.0) ? ((_11488.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_11488.y > 0.0) ? in.i_radii.w : in.i_radii.x), _11814);
                        float _11823 = _11817 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _21778;
                        float _21779;
                        if (_11823 > _11814)
                        {
                            float _11837 = in.i_shape.y * fast::clamp((_11814 - _11817) / fast::max(0.60000002384185791015625 * _11817, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21779 = _11837;
                            _21778 = _11817 * (1.0 + (0.60000002384185791015625 * _11837));
                        }
                        else
                        {
                            _21779 = in.i_shape.y;
                            _21778 = _11823;
                        }
                        float2 _11850 = (abs(_11488) - _11446) + float2(_21778);
                        float2 _11852 = fast::max(_11850, float2(0.0));
                        float _21780;
                        if ((_11852.x > 0.0) && (_11852.y > 0.0))
                        {
                            float _21781;
                            if ((_21779 > 0.001000000047497451305389404296875) && (_21778 > 9.9999997473787516355514526367188e-05))
                            {
                                float _11869 = 2.0 + (2.0 * _21779);
                                float2 _11874 = _11852 / float2(fast::max(_21778, 9.9999997473787516355514526367188e-05));
                                _21781 = pow(pow(_11874.x, _11869) + pow(_11874.y, _11869), 1.0 / _11869) * _21778;
                            }
                            else
                            {
                                _21781 = length(_11852);
                            }
                            _21780 = _21781;
                        }
                        else
                        {
                            _21780 = fast::max(_11852.x, _11852.y);
                        }
                        float _11909 = (fast::min(fast::max(_11850.x, _11850.y), 0.0) + _21780) - _21778;
                        float _21797;
                        if ((in.i_flags.x & _1485) != 0u)
                        {
                            float2 _11516 = fast::max((_21226.zw - _21226.xy) * 0.5, float2(0.001000000047497451305389404296875));
                            float2 _11519 = _10703 - ((_21226.xy + _21226.zw) * 0.5);
                            float _11945 = fast::min(_11516.x, _11516.y);
                            float _11948 = fast::min((_11519.x > 0.0) ? ((_11519.y > 0.0) ? gFxData_1._data[_10180].x : gFxData_1._data[_10180].x) : ((_11519.y > 0.0) ? gFxData_1._data[_10180].x : gFxData_1._data[_10180].x), _11945);
                            float _11954 = _11948 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _21784;
                            float _21785;
                            if (_11954 > _11945)
                            {
                                float _11968 = in.i_shape.y * fast::clamp((_11945 - _11948) / fast::max(0.60000002384185791015625 * _11948, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _21785 = _11968;
                                _21784 = _11948 * (1.0 + (0.60000002384185791015625 * _11968));
                            }
                            else
                            {
                                _21785 = in.i_shape.y;
                                _21784 = _11954;
                            }
                            float2 _11981 = (abs(_11519) - _11516) + float2(_21784);
                            float2 _11983 = fast::max(_11981, float2(0.0));
                            float _21786;
                            if ((_11983.x > 0.0) && (_11983.y > 0.0))
                            {
                                float _21787;
                                if ((_21785 > 0.001000000047497451305389404296875) && (_21784 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _12000 = 2.0 + (2.0 * _21785);
                                    float2 _12005 = _11983 / float2(fast::max(_21784, 9.9999997473787516355514526367188e-05));
                                    _21787 = pow(pow(_12005.x, _12000) + pow(_12005.y, _12000), 1.0 / _12000) * _21784;
                                }
                                else
                                {
                                    _21787 = length(_11983);
                                }
                                _21786 = _21787;
                            }
                            else
                            {
                                _21786 = fast::max(_11983.x, _11983.y);
                            }
                            float _12040 = (fast::min(fast::max(_11981.x, _11981.y), 0.0) + _21786) - _21784;
                            float _12045 = fast::max(gFxData_1._data[_10180].y, 9.9999997473787516355514526367188e-05);
                            float _12054 = fast::max(_12045 - abs(_11909 - _12040), 0.0) / _12045;
                            _21797 = fast::min(_11909, _12040) - (((_12054 * _12054) * _12045) * 0.25);
                        }
                        else
                        {
                            _21797 = _11909;
                        }
                        _21796 = _21797;
                    }
                    _21795 = _21796;
                }
                _21794 = _21795;
            }
            float2 _10708 = in.i_local + float2(0.0, 0.5);
            float2 _12105 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _12114 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _21814;
            if ((in.i_flags.x & _1416) != 0u)
            {
                float _12220 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                float4 _12254 = gTex.sample(gLinear, mix(_21220.xy, _21220.zw, (fast::clamp(_10708, _21226.xy, _21226.zw) - _21226.xy) / fast::max(_21226.zw - _21226.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                float _12278 = (_12254.w - 0.5) * in.i_radii.y;
                float2 _12319 = abs(_10708 - ((_21226.xy + _21226.zw) * 0.5)) - fast::max(((_21226.zw - _21226.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _12220), float2(0.0));
                _21814 = fast::max((-mix((fast::max(fast::min(_12254.x, _12254.y), fast::min(fast::max(_12254.x, _12254.y), _12254.z)) - 0.5) * in.i_radii.x, _12278, fast::clamp(abs(_12278) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _12220, length(fast::max(_12319, float2(0.0))) + fast::min(fast::max(_12319.x, _12319.y), 0.0)) - in.i_shape.w;
            }
            else
            {
                float _21815;
                if (in.i_flags.z == 1u)
                {
                    float2 _12129 = _10708 - _12105;
                    float _21813;
                    do
                    {
                        if (in.i_shape.w >= 6.282185077667236328125)
                        {
                            _21813 = abs(length(_12129) - in.i_radii.x) - in.i_radii.y;
                            break;
                        }
                        float _12363 = in.i_shape.z + (in.i_shape.w * 0.5);
                        float _12365 = cos(_12363);
                        float _12367 = sin(_12363);
                        float _12376 = dot(_12129, float2(-_12367, _12365));
                        float _12379 = dot(_12129, float2(_12365, _12367));
                        float2 _12380 = float2(_12376, _12379);
                        float _12383 = abs(_12376);
                        _12380.x = _12383;
                        float _12386 = in.i_shape.w * 0.5;
                        float _12388 = sin(_12386);
                        float _12390 = cos(_12386);
                        _21813 = (((_12390 * _12383) > (_12388 * _12379)) ? length(_12380 - (float2(_12388, _12390) * in.i_radii.x)) : abs(length(_12380) - in.i_radii.x)) - in.i_radii.y;
                        break;
                    } while(false);
                    _21815 = _21813;
                }
                else
                {
                    float _21816;
                    if (in.i_flags.z == 2u)
                    {
                        float2 _12426 = _10708 - _21226.xy;
                        float2 _12429 = _21226.zw - _21226.xy;
                        _21816 = length(_12426 - (_12429 * fast::clamp(dot(_12426, _12429) / fast::max(dot(_12429, _12429), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                    }
                    else
                    {
                        float2 _12156 = _10708 - _12105;
                        float _12482 = fast::min(_12114.x, _12114.y);
                        float _12485 = fast::min((_12156.x > 0.0) ? ((_12156.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_12156.y > 0.0) ? in.i_radii.w : in.i_radii.x), _12482);
                        float _12491 = _12485 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _21798;
                        float _21799;
                        if (_12491 > _12482)
                        {
                            float _12505 = in.i_shape.y * fast::clamp((_12482 - _12485) / fast::max(0.60000002384185791015625 * _12485, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21799 = _12505;
                            _21798 = _12485 * (1.0 + (0.60000002384185791015625 * _12505));
                        }
                        else
                        {
                            _21799 = in.i_shape.y;
                            _21798 = _12491;
                        }
                        float2 _12518 = (abs(_12156) - _12114) + float2(_21798);
                        float2 _12520 = fast::max(_12518, float2(0.0));
                        float _21800;
                        if ((_12520.x > 0.0) && (_12520.y > 0.0))
                        {
                            float _21801;
                            if ((_21799 > 0.001000000047497451305389404296875) && (_21798 > 9.9999997473787516355514526367188e-05))
                            {
                                float _12537 = 2.0 + (2.0 * _21799);
                                float2 _12542 = _12520 / float2(fast::max(_21798, 9.9999997473787516355514526367188e-05));
                                _21801 = pow(pow(_12542.x, _12537) + pow(_12542.y, _12537), 1.0 / _12537) * _21798;
                            }
                            else
                            {
                                _21801 = length(_12520);
                            }
                            _21800 = _21801;
                        }
                        else
                        {
                            _21800 = fast::max(_12520.x, _12520.y);
                        }
                        float _12577 = (fast::min(fast::max(_12518.x, _12518.y), 0.0) + _21800) - _21798;
                        float _21817;
                        if ((in.i_flags.x & _1485) != 0u)
                        {
                            float2 _12184 = fast::max((_21226.zw - _21226.xy) * 0.5, float2(0.001000000047497451305389404296875));
                            float2 _12187 = _10708 - ((_21226.xy + _21226.zw) * 0.5);
                            float _12613 = fast::min(_12184.x, _12184.y);
                            float _12616 = fast::min((_12187.x > 0.0) ? ((_12187.y > 0.0) ? gFxData_1._data[_10180].x : gFxData_1._data[_10180].x) : ((_12187.y > 0.0) ? gFxData_1._data[_10180].x : gFxData_1._data[_10180].x), _12613);
                            float _12622 = _12616 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _21804;
                            float _21805;
                            if (_12622 > _12613)
                            {
                                float _12636 = in.i_shape.y * fast::clamp((_12613 - _12616) / fast::max(0.60000002384185791015625 * _12616, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _21805 = _12636;
                                _21804 = _12616 * (1.0 + (0.60000002384185791015625 * _12636));
                            }
                            else
                            {
                                _21805 = in.i_shape.y;
                                _21804 = _12622;
                            }
                            float2 _12649 = (abs(_12187) - _12184) + float2(_21804);
                            float2 _12651 = fast::max(_12649, float2(0.0));
                            float _21806;
                            if ((_12651.x > 0.0) && (_12651.y > 0.0))
                            {
                                float _21807;
                                if ((_21805 > 0.001000000047497451305389404296875) && (_21804 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _12668 = 2.0 + (2.0 * _21805);
                                    float2 _12673 = _12651 / float2(fast::max(_21804, 9.9999997473787516355514526367188e-05));
                                    _21807 = pow(pow(_12673.x, _12668) + pow(_12673.y, _12668), 1.0 / _12668) * _21804;
                                }
                                else
                                {
                                    _21807 = length(_12651);
                                }
                                _21806 = _21807;
                            }
                            else
                            {
                                _21806 = fast::max(_12651.x, _12651.y);
                            }
                            float _12708 = (fast::min(fast::max(_12649.x, _12649.y), 0.0) + _21806) - _21804;
                            float _12713 = fast::max(gFxData_1._data[_10180].y, 9.9999997473787516355514526367188e-05);
                            float _12722 = fast::max(_12713 - abs(_12577 - _12708), 0.0) / _12713;
                            _21817 = fast::min(_12577, _12708) - (((_12722 * _12722) * _12713) * 0.25);
                        }
                        else
                        {
                            _21817 = _12577;
                        }
                        _21816 = _21817;
                    }
                    _21815 = _21816;
                }
                _21814 = _21815;
            }
            float2 _10712 = in.i_local - float2(0.0, 0.5);
            float2 _12773 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _12782 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float _21834;
            if ((in.i_flags.x & _1416) != 0u)
            {
                float _12888 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                float4 _12922 = gTex.sample(gLinear, mix(_21220.xy, _21220.zw, (fast::clamp(_10712, _21226.xy, _21226.zw) - _21226.xy) / fast::max(_21226.zw - _21226.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                float _12946 = (_12922.w - 0.5) * in.i_radii.y;
                float2 _12987 = abs(_10712 - ((_21226.xy + _21226.zw) * 0.5)) - fast::max(((_21226.zw - _21226.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _12888), float2(0.0));
                _21834 = fast::max((-mix((fast::max(fast::min(_12922.x, _12922.y), fast::min(fast::max(_12922.x, _12922.y), _12922.z)) - 0.5) * in.i_radii.x, _12946, fast::clamp(abs(_12946) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _12888, length(fast::max(_12987, float2(0.0))) + fast::min(fast::max(_12987.x, _12987.y), 0.0)) - in.i_shape.w;
            }
            else
            {
                float _21835;
                if (in.i_flags.z == 1u)
                {
                    float2 _12797 = _10712 - _12773;
                    float _21833;
                    do
                    {
                        if (in.i_shape.w >= 6.282185077667236328125)
                        {
                            _21833 = abs(length(_12797) - in.i_radii.x) - in.i_radii.y;
                            break;
                        }
                        float _13031 = in.i_shape.z + (in.i_shape.w * 0.5);
                        float _13033 = cos(_13031);
                        float _13035 = sin(_13031);
                        float _13044 = dot(_12797, float2(-_13035, _13033));
                        float _13047 = dot(_12797, float2(_13033, _13035));
                        float2 _13048 = float2(_13044, _13047);
                        float _13051 = abs(_13044);
                        _13048.x = _13051;
                        float _13054 = in.i_shape.w * 0.5;
                        float _13056 = sin(_13054);
                        float _13058 = cos(_13054);
                        _21833 = (((_13058 * _13051) > (_13056 * _13047)) ? length(_13048 - (float2(_13056, _13058) * in.i_radii.x)) : abs(length(_13048) - in.i_radii.x)) - in.i_radii.y;
                        break;
                    } while(false);
                    _21835 = _21833;
                }
                else
                {
                    float _21836;
                    if (in.i_flags.z == 2u)
                    {
                        float2 _13094 = _10712 - _21226.xy;
                        float2 _13097 = _21226.zw - _21226.xy;
                        _21836 = length(_13094 - (_13097 * fast::clamp(dot(_13094, _13097) / fast::max(dot(_13097, _13097), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                    }
                    else
                    {
                        float2 _12824 = _10712 - _12773;
                        float _13150 = fast::min(_12782.x, _12782.y);
                        float _13153 = fast::min((_12824.x > 0.0) ? ((_12824.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_12824.y > 0.0) ? in.i_radii.w : in.i_radii.x), _13150);
                        float _13159 = _13153 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _21818;
                        float _21819;
                        if (_13159 > _13150)
                        {
                            float _13173 = in.i_shape.y * fast::clamp((_13150 - _13153) / fast::max(0.60000002384185791015625 * _13153, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _21819 = _13173;
                            _21818 = _13153 * (1.0 + (0.60000002384185791015625 * _13173));
                        }
                        else
                        {
                            _21819 = in.i_shape.y;
                            _21818 = _13159;
                        }
                        float2 _13186 = (abs(_12824) - _12782) + float2(_21818);
                        float2 _13188 = fast::max(_13186, float2(0.0));
                        float _21820;
                        if ((_13188.x > 0.0) && (_13188.y > 0.0))
                        {
                            float _21821;
                            if ((_21819 > 0.001000000047497451305389404296875) && (_21818 > 9.9999997473787516355514526367188e-05))
                            {
                                float _13205 = 2.0 + (2.0 * _21819);
                                float2 _13210 = _13188 / float2(fast::max(_21818, 9.9999997473787516355514526367188e-05));
                                _21821 = pow(pow(_13210.x, _13205) + pow(_13210.y, _13205), 1.0 / _13205) * _21818;
                            }
                            else
                            {
                                _21821 = length(_13188);
                            }
                            _21820 = _21821;
                        }
                        else
                        {
                            _21820 = fast::max(_13188.x, _13188.y);
                        }
                        float _13245 = (fast::min(fast::max(_13186.x, _13186.y), 0.0) + _21820) - _21818;
                        float _21837;
                        if ((in.i_flags.x & _1485) != 0u)
                        {
                            float2 _12852 = fast::max((_21226.zw - _21226.xy) * 0.5, float2(0.001000000047497451305389404296875));
                            float2 _12855 = _10712 - ((_21226.xy + _21226.zw) * 0.5);
                            float _13281 = fast::min(_12852.x, _12852.y);
                            float _13284 = fast::min((_12855.x > 0.0) ? ((_12855.y > 0.0) ? gFxData_1._data[_10180].x : gFxData_1._data[_10180].x) : ((_12855.y > 0.0) ? gFxData_1._data[_10180].x : gFxData_1._data[_10180].x), _13281);
                            float _13290 = _13284 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _21824;
                            float _21825;
                            if (_13290 > _13281)
                            {
                                float _13304 = in.i_shape.y * fast::clamp((_13281 - _13284) / fast::max(0.60000002384185791015625 * _13284, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _21825 = _13304;
                                _21824 = _13284 * (1.0 + (0.60000002384185791015625 * _13304));
                            }
                            else
                            {
                                _21825 = in.i_shape.y;
                                _21824 = _13290;
                            }
                            float2 _13317 = (abs(_12855) - _12852) + float2(_21824);
                            float2 _13319 = fast::max(_13317, float2(0.0));
                            float _21826;
                            if ((_13319.x > 0.0) && (_13319.y > 0.0))
                            {
                                float _21827;
                                if ((_21825 > 0.001000000047497451305389404296875) && (_21824 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _13336 = 2.0 + (2.0 * _21825);
                                    float2 _13341 = _13319 / float2(fast::max(_21824, 9.9999997473787516355514526367188e-05));
                                    _21827 = pow(pow(_13341.x, _13336) + pow(_13341.y, _13336), 1.0 / _13336) * _21824;
                                }
                                else
                                {
                                    _21827 = length(_13319);
                                }
                                _21826 = _21827;
                            }
                            else
                            {
                                _21826 = fast::max(_13319.x, _13319.y);
                            }
                            float _13376 = (fast::min(fast::max(_13317.x, _13317.y), 0.0) + _21826) - _21824;
                            float _13381 = fast::max(gFxData_1._data[_10180].y, 9.9999997473787516355514526367188e-05);
                            float _13390 = fast::max(_13381 - abs(_13245 - _13376), 0.0) / _13381;
                            _21837 = fast::min(_13245, _13376) - (((_13390 * _13390) * _13381) * 0.25);
                        }
                        else
                        {
                            _21837 = _13245;
                        }
                        _21836 = _21837;
                    }
                    _21835 = _21836;
                }
                _21834 = _21835;
            }
            float2 _10718 = float2(_21774 - _21794, _21814 - _21834);
            float _10720 = length(_10718);
            float2 _10728 = select(float2(0.0, -1.0), _10718 / float2(_10720), bool2(_10720 > 9.9999997473787516355514526367188e-06));
            _22415 = fast::normalize(float3(_10728 * fast::min(_10281 / fast::max(_10287, 0.001000000047497451305389404296875), 8.0), 1.0));
            _21838 = _10728;
        }
        else
        {
            _22415 = float3(0.0, 0.0, 1.0);
            _21838 = float2(0.0, -1.0);
        }
        float2 _10313 = ((-_21838) * gFxData_1._data[_10156].y) * (1.0 - _10287);
        float2 _21839;
        if (gFxData_1._data[_10180].z > 0.0)
        {
            _21839 = (((in.i_rect.xy + in.i_rect.zw) * 0.5) - in.i_local) * (gFxData_1._data[_10180].z / (1.0 + gFxData_1._data[_10180].z));
        }
        else
        {
            _21839 = float2(0.0);
        }
        float2 _10339 = _21219 * _206.gTarget.zw;
        float2 _10346 = _206.gDisplay.zw * _206.gTarget.zw;
        float2 _10351 = (_10313 + _21839) * _10346;
        float2 _10354 = _10313 * _10346;
        float3 _22297;
        float _22315;
        float3 _22518;
        if (_206.gTime.z > 0.5)
        {
            float2 _10361 = _10339 + _10351;
            float _13415 = fast::clamp(log2(fast::max(_10253, 1.0)) - 1.0, 0.0, 5.0);
            int _13418 = int(floor(_13415));
            float _13422 = _13415 - float(_13418);
            bool _13427 = (_13422 > 0.0199999995529651641845703125) && (_13418 < 5);
            float3 _21910;
            _21910 = float3(0.0);
            float3 _13456;
            for (int _21840 = 0; _21840 < 2; _21910 = _13456, _21840++)
            {
                if ((_21840 > 0) && (!_13427))
                {
                    break;
                }
                int _13442 = _13418 + _21840;
                int _13480 = clamp(_13442, 1, 5);
                float2 _13549 = (_10361 * _206.gLevel[_13480].xy) - float2(0.5);
                float2 _13551 = floor(_13549);
                float2 _13554 = _13549 - _13551;
                float2 _13557 = _13554 * _13554;
                float2 _13560 = _13557 * _13554;
                float2 _13579 = (((_13560 * 3.0) - (_13557 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                float2 _13592 = _13560 * 0.16666667163372039794921875;
                float2 _13595 = (((((-_13560) + (_13557 * 3.0)) - (_13554 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13579;
                float2 _13599 = (((((_13560 * (-3.0)) + (_13557 * 3.0)) + (_13554 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _13592;
                float2 _13611 = ((_13551 - float2(0.5)) + (_13579 / _13595)) * _206.gLevel[_13480].zw;
                float2 _13623 = ((_13551 + float2(1.5)) + (_13592 / _13599)) * _206.gLevel[_13480].zw;
                float4 _21877;
                if (_13442 <= 0)
                {
                    float2 _21876;
                    if (_206.gConv.x > 0.5)
                    {
                        float2 _20535 = _10361;
                        _20535.y = 1.0 - _10361.y;
                        _21876 = _20535;
                    }
                    else
                    {
                        _21876 = _10361;
                    }
                    _21877 = gBackdrop0.sample(gLinear, _21876, level(0.0));
                }
                else
                {
                    float4 _21878;
                    if (_13442 == 1)
                    {
                        float2 _21869;
                        if (_206.gConv.x > 0.5)
                        {
                            float2 _20540 = _13611;
                            _20540.y = 1.0 - _13611.y;
                            _21869 = _20540;
                        }
                        else
                        {
                            _21869 = _13611;
                        }
                        float2 _13669 = float2(_13623.x, _13611.y);
                        float2 _21870;
                        if (_206.gConv.x > 0.5)
                        {
                            float2 _20547 = _13669;
                            _20547.y = 1.0 - _13611.y;
                            _21870 = _20547;
                        }
                        else
                        {
                            _21870 = _13669;
                        }
                        float2 _13687 = float2(_13611.x, _13623.y);
                        float2 _21872;
                        if (_206.gConv.x > 0.5)
                        {
                            float2 _20554 = _13687;
                            _20554.y = 1.0 - _13623.y;
                            _21872 = _20554;
                        }
                        else
                        {
                            _21872 = _13687;
                        }
                        float2 _21874;
                        if (_206.gConv.x > 0.5)
                        {
                            float2 _20561 = _13623;
                            _20561.y = 1.0 - _13623.y;
                            _21874 = _20561;
                        }
                        else
                        {
                            _21874 = _13623;
                        }
                        _21878 = (((gBackdrop1.sample(gLinear, _21869, level(0.0)) * (_13595.x * _13595.y)) + (gBackdrop1.sample(gLinear, _21870, level(0.0)) * (_13599.x * _13595.y))) + (gBackdrop1.sample(gLinear, _21872, level(0.0)) * (_13595.x * _13599.y))) + (gBackdrop1.sample(gLinear, _21874, level(0.0)) * (_13599.x * _13599.y));
                    }
                    else
                    {
                        float4 _21879;
                        if (_13442 == 2)
                        {
                            float2 _21862;
                            if (_206.gConv.x > 0.5)
                            {
                                float2 _20568 = _13611;
                                _20568.y = 1.0 - _13611.y;
                                _21862 = _20568;
                            }
                            else
                            {
                                _21862 = _13611;
                            }
                            float2 _13799 = float2(_13623.x, _13611.y);
                            float2 _21863;
                            if (_206.gConv.x > 0.5)
                            {
                                float2 _20575 = _13799;
                                _20575.y = 1.0 - _13611.y;
                                _21863 = _20575;
                            }
                            else
                            {
                                _21863 = _13799;
                            }
                            float2 _13817 = float2(_13611.x, _13623.y);
                            float2 _21865;
                            if (_206.gConv.x > 0.5)
                            {
                                float2 _20582 = _13817;
                                _20582.y = 1.0 - _13623.y;
                                _21865 = _20582;
                            }
                            else
                            {
                                _21865 = _13817;
                            }
                            float2 _21867;
                            if (_206.gConv.x > 0.5)
                            {
                                float2 _20589 = _13623;
                                _20589.y = 1.0 - _13623.y;
                                _21867 = _20589;
                            }
                            else
                            {
                                _21867 = _13623;
                            }
                            _21879 = (((gBackdrop2.sample(gLinear, _21862, level(0.0)) * (_13595.x * _13595.y)) + (gBackdrop2.sample(gLinear, _21863, level(0.0)) * (_13599.x * _13595.y))) + (gBackdrop2.sample(gLinear, _21865, level(0.0)) * (_13595.x * _13599.y))) + (gBackdrop2.sample(gLinear, _21867, level(0.0)) * (_13599.x * _13599.y));
                        }
                        else
                        {
                            float4 _21880;
                            if (_13442 == 3)
                            {
                                float2 _21855;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20596 = _13611;
                                    _20596.y = 1.0 - _13611.y;
                                    _21855 = _20596;
                                }
                                else
                                {
                                    _21855 = _13611;
                                }
                                float2 _13929 = float2(_13623.x, _13611.y);
                                float2 _21856;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20603 = _13929;
                                    _20603.y = 1.0 - _13611.y;
                                    _21856 = _20603;
                                }
                                else
                                {
                                    _21856 = _13929;
                                }
                                float2 _13947 = float2(_13611.x, _13623.y);
                                float2 _21858;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20610 = _13947;
                                    _20610.y = 1.0 - _13623.y;
                                    _21858 = _20610;
                                }
                                else
                                {
                                    _21858 = _13947;
                                }
                                float2 _21860;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20617 = _13623;
                                    _20617.y = 1.0 - _13623.y;
                                    _21860 = _20617;
                                }
                                else
                                {
                                    _21860 = _13623;
                                }
                                _21880 = (((gBackdrop3.sample(gLinear, _21855, level(0.0)) * (_13595.x * _13595.y)) + (gBackdrop3.sample(gLinear, _21856, level(0.0)) * (_13599.x * _13595.y))) + (gBackdrop3.sample(gLinear, _21858, level(0.0)) * (_13595.x * _13599.y))) + (gBackdrop3.sample(gLinear, _21860, level(0.0)) * (_13599.x * _13599.y));
                            }
                            else
                            {
                                float4 _21881;
                                if (_13442 == 4)
                                {
                                    float2 _21848;
                                    if (_206.gConv.x > 0.5)
                                    {
                                        float2 _20624 = _13611;
                                        _20624.y = 1.0 - _13611.y;
                                        _21848 = _20624;
                                    }
                                    else
                                    {
                                        _21848 = _13611;
                                    }
                                    float2 _14059 = float2(_13623.x, _13611.y);
                                    float2 _21849;
                                    if (_206.gConv.x > 0.5)
                                    {
                                        float2 _20631 = _14059;
                                        _20631.y = 1.0 - _13611.y;
                                        _21849 = _20631;
                                    }
                                    else
                                    {
                                        _21849 = _14059;
                                    }
                                    float2 _14077 = float2(_13611.x, _13623.y);
                                    float2 _21851;
                                    if (_206.gConv.x > 0.5)
                                    {
                                        float2 _20638 = _14077;
                                        _20638.y = 1.0 - _13623.y;
                                        _21851 = _20638;
                                    }
                                    else
                                    {
                                        _21851 = _14077;
                                    }
                                    float2 _21853;
                                    if (_206.gConv.x > 0.5)
                                    {
                                        float2 _20645 = _13623;
                                        _20645.y = 1.0 - _13623.y;
                                        _21853 = _20645;
                                    }
                                    else
                                    {
                                        _21853 = _13623;
                                    }
                                    _21881 = (((gBackdrop4.sample(gLinear, _21848, level(0.0)) * (_13595.x * _13595.y)) + (gBackdrop4.sample(gLinear, _21849, level(0.0)) * (_13599.x * _13595.y))) + (gBackdrop4.sample(gLinear, _21851, level(0.0)) * (_13595.x * _13599.y))) + (gBackdrop4.sample(gLinear, _21853, level(0.0)) * (_13599.x * _13599.y));
                                }
                                else
                                {
                                    float2 _21841;
                                    if (_206.gConv.x > 0.5)
                                    {
                                        float2 _20652 = _13611;
                                        _20652.y = 1.0 - _13611.y;
                                        _21841 = _20652;
                                    }
                                    else
                                    {
                                        _21841 = _13611;
                                    }
                                    float2 _14189 = float2(_13623.x, _13611.y);
                                    float2 _21842;
                                    if (_206.gConv.x > 0.5)
                                    {
                                        float2 _20659 = _14189;
                                        _20659.y = 1.0 - _13611.y;
                                        _21842 = _20659;
                                    }
                                    else
                                    {
                                        _21842 = _14189;
                                    }
                                    float2 _14207 = float2(_13611.x, _13623.y);
                                    float2 _21844;
                                    if (_206.gConv.x > 0.5)
                                    {
                                        float2 _20666 = _14207;
                                        _20666.y = 1.0 - _13623.y;
                                        _21844 = _20666;
                                    }
                                    else
                                    {
                                        _21844 = _14207;
                                    }
                                    float2 _21846;
                                    if (_206.gConv.x > 0.5)
                                    {
                                        float2 _20673 = _13623;
                                        _20673.y = 1.0 - _13623.y;
                                        _21846 = _20673;
                                    }
                                    else
                                    {
                                        _21846 = _13623;
                                    }
                                    _21881 = (((gBackdrop5.sample(gLinear, _21841, level(0.0)) * (_13595.x * _13595.y)) + (gBackdrop5.sample(gLinear, _21842, level(0.0)) * (_13599.x * _13595.y))) + (gBackdrop5.sample(gLinear, _21844, level(0.0)) * (_13595.x * _13599.y))) + (gBackdrop5.sample(gLinear, _21846, level(0.0)) * (_13599.x * _13599.y));
                                }
                                _21880 = _21881;
                            }
                            _21879 = _21880;
                        }
                        _21878 = _21879;
                    }
                    _21877 = _21878;
                }
                _13456 = _21910 + (_21877.xyz * (_13427 ? ((_21840 == 0) ? (1.0 - _13422) : _13422) : 1.0));
            }
            float3 _22300;
            if (((gFxData_1._data[_10156].w > 0.001000000047497451305389404296875) && (_10281 > 0.0)) && ((((gFxData_1._data[_10156].y * 0.300000011920928955078125) * gFxData_1._data[_10156].w) * _206.gDisplay.z) > (_10253 * 0.3499999940395355224609375)))
            {
                float _10381 = 0.300000011920928955078125 * gFxData_1._data[_10156].w;
                float2 _10388 = (_10339 + _10351) - (_10354 * _10381);
                float _14303 = fast::clamp(log2(fast::max(_10253, 1.0)) - 1.0, 0.0, 5.0);
                int _14306 = int(floor(_14303));
                float _14310 = _14303 - float(_14306);
                bool _14315 = (_14310 > 0.0199999995529651641845703125) && (_14306 < 5);
                float3 _22006;
                _22006 = float3(0.0);
                float3 _14344;
                for (int _21936 = 0; _21936 < 2; _22006 = _14344, _21936++)
                {
                    if ((_21936 > 0) && (!_14315))
                    {
                        break;
                    }
                    int _14330 = _14306 + _21936;
                    int _14368 = clamp(_14330, 1, 5);
                    float2 _14437 = (_10388 * _206.gLevel[_14368].xy) - float2(0.5);
                    float2 _14439 = floor(_14437);
                    float2 _14442 = _14437 - _14439;
                    float2 _14445 = _14442 * _14442;
                    float2 _14448 = _14445 * _14442;
                    float2 _14467 = (((_14448 * 3.0) - (_14445 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                    float2 _14480 = _14448 * 0.16666667163372039794921875;
                    float2 _14483 = (((((-_14448) + (_14445 * 3.0)) - (_14442 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14467;
                    float2 _14487 = (((((_14448 * (-3.0)) + (_14445 * 3.0)) + (_14442 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _14480;
                    float2 _14499 = ((_14439 - float2(0.5)) + (_14467 / _14483)) * _206.gLevel[_14368].zw;
                    float2 _14511 = ((_14439 + float2(1.5)) + (_14480 / _14487)) * _206.gLevel[_14368].zw;
                    float4 _21973;
                    if (_14330 <= 0)
                    {
                        float2 _21972;
                        if (_206.gConv.x > 0.5)
                        {
                            float2 _20678 = _10388;
                            _20678.y = 1.0 - _10388.y;
                            _21972 = _20678;
                        }
                        else
                        {
                            _21972 = _10388;
                        }
                        _21973 = gBackdrop0.sample(gLinear, _21972, level(0.0));
                    }
                    else
                    {
                        float4 _21974;
                        if (_14330 == 1)
                        {
                            float2 _21965;
                            if (_206.gConv.x > 0.5)
                            {
                                float2 _20683 = _14499;
                                _20683.y = 1.0 - _14499.y;
                                _21965 = _20683;
                            }
                            else
                            {
                                _21965 = _14499;
                            }
                            float _14556 = _14499.y;
                            float2 _14557 = float2(_14511.x, _14556);
                            float2 _21966;
                            if (_206.gConv.x > 0.5)
                            {
                                float2 _20690 = _14557;
                                _20690.y = 1.0 - _14556;
                                _21966 = _20690;
                            }
                            else
                            {
                                _21966 = _14557;
                            }
                            float _14574 = _14511.y;
                            float2 _14575 = float2(_14499.x, _14574);
                            float2 _21968;
                            if (_206.gConv.x > 0.5)
                            {
                                float2 _20697 = _14575;
                                _20697.y = 1.0 - _14574;
                                _21968 = _20697;
                            }
                            else
                            {
                                _21968 = _14575;
                            }
                            float2 _21970;
                            if (_206.gConv.x > 0.5)
                            {
                                float2 _20704 = _14511;
                                _20704.y = 1.0 - _14511.y;
                                _21970 = _20704;
                            }
                            else
                            {
                                _21970 = _14511;
                            }
                            _21974 = (((gBackdrop1.sample(gLinear, _21965, level(0.0)) * (_14483.x * _14483.y)) + (gBackdrop1.sample(gLinear, _21966, level(0.0)) * (_14487.x * _14483.y))) + (gBackdrop1.sample(gLinear, _21968, level(0.0)) * (_14483.x * _14487.y))) + (gBackdrop1.sample(gLinear, _21970, level(0.0)) * (_14487.x * _14487.y));
                        }
                        else
                        {
                            float4 _21975;
                            if (_14330 == 2)
                            {
                                float2 _21958;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20711 = _14499;
                                    _20711.y = 1.0 - _14499.y;
                                    _21958 = _20711;
                                }
                                else
                                {
                                    _21958 = _14499;
                                }
                                float _14686 = _14499.y;
                                float2 _14687 = float2(_14511.x, _14686);
                                float2 _21959;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20718 = _14687;
                                    _20718.y = 1.0 - _14686;
                                    _21959 = _20718;
                                }
                                else
                                {
                                    _21959 = _14687;
                                }
                                float _14704 = _14511.y;
                                float2 _14705 = float2(_14499.x, _14704);
                                float2 _21961;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20725 = _14705;
                                    _20725.y = 1.0 - _14704;
                                    _21961 = _20725;
                                }
                                else
                                {
                                    _21961 = _14705;
                                }
                                float2 _21963;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20732 = _14511;
                                    _20732.y = 1.0 - _14511.y;
                                    _21963 = _20732;
                                }
                                else
                                {
                                    _21963 = _14511;
                                }
                                _21975 = (((gBackdrop2.sample(gLinear, _21958, level(0.0)) * (_14483.x * _14483.y)) + (gBackdrop2.sample(gLinear, _21959, level(0.0)) * (_14487.x * _14483.y))) + (gBackdrop2.sample(gLinear, _21961, level(0.0)) * (_14483.x * _14487.y))) + (gBackdrop2.sample(gLinear, _21963, level(0.0)) * (_14487.x * _14487.y));
                            }
                            else
                            {
                                float4 _21976;
                                if (_14330 == 3)
                                {
                                    float2 _21951;
                                    if (_206.gConv.x > 0.5)
                                    {
                                        float2 _20739 = _14499;
                                        _20739.y = 1.0 - _14499.y;
                                        _21951 = _20739;
                                    }
                                    else
                                    {
                                        _21951 = _14499;
                                    }
                                    float _14816 = _14499.y;
                                    float2 _14817 = float2(_14511.x, _14816);
                                    float2 _21952;
                                    if (_206.gConv.x > 0.5)
                                    {
                                        float2 _20746 = _14817;
                                        _20746.y = 1.0 - _14816;
                                        _21952 = _20746;
                                    }
                                    else
                                    {
                                        _21952 = _14817;
                                    }
                                    float _14834 = _14511.y;
                                    float2 _14835 = float2(_14499.x, _14834);
                                    float2 _21954;
                                    if (_206.gConv.x > 0.5)
                                    {
                                        float2 _20753 = _14835;
                                        _20753.y = 1.0 - _14834;
                                        _21954 = _20753;
                                    }
                                    else
                                    {
                                        _21954 = _14835;
                                    }
                                    float2 _21956;
                                    if (_206.gConv.x > 0.5)
                                    {
                                        float2 _20760 = _14511;
                                        _20760.y = 1.0 - _14511.y;
                                        _21956 = _20760;
                                    }
                                    else
                                    {
                                        _21956 = _14511;
                                    }
                                    _21976 = (((gBackdrop3.sample(gLinear, _21951, level(0.0)) * (_14483.x * _14483.y)) + (gBackdrop3.sample(gLinear, _21952, level(0.0)) * (_14487.x * _14483.y))) + (gBackdrop3.sample(gLinear, _21954, level(0.0)) * (_14483.x * _14487.y))) + (gBackdrop3.sample(gLinear, _21956, level(0.0)) * (_14487.x * _14487.y));
                                }
                                else
                                {
                                    float4 _21977;
                                    if (_14330 == 4)
                                    {
                                        float2 _21944;
                                        if (_206.gConv.x > 0.5)
                                        {
                                            float2 _20767 = _14499;
                                            _20767.y = 1.0 - _14499.y;
                                            _21944 = _20767;
                                        }
                                        else
                                        {
                                            _21944 = _14499;
                                        }
                                        float _14946 = _14499.y;
                                        float2 _14947 = float2(_14511.x, _14946);
                                        float2 _21945;
                                        if (_206.gConv.x > 0.5)
                                        {
                                            float2 _20774 = _14947;
                                            _20774.y = 1.0 - _14946;
                                            _21945 = _20774;
                                        }
                                        else
                                        {
                                            _21945 = _14947;
                                        }
                                        float _14964 = _14511.y;
                                        float2 _14965 = float2(_14499.x, _14964);
                                        float2 _21947;
                                        if (_206.gConv.x > 0.5)
                                        {
                                            float2 _20781 = _14965;
                                            _20781.y = 1.0 - _14964;
                                            _21947 = _20781;
                                        }
                                        else
                                        {
                                            _21947 = _14965;
                                        }
                                        float2 _21949;
                                        if (_206.gConv.x > 0.5)
                                        {
                                            float2 _20788 = _14511;
                                            _20788.y = 1.0 - _14511.y;
                                            _21949 = _20788;
                                        }
                                        else
                                        {
                                            _21949 = _14511;
                                        }
                                        _21977 = (((gBackdrop4.sample(gLinear, _21944, level(0.0)) * (_14483.x * _14483.y)) + (gBackdrop4.sample(gLinear, _21945, level(0.0)) * (_14487.x * _14483.y))) + (gBackdrop4.sample(gLinear, _21947, level(0.0)) * (_14483.x * _14487.y))) + (gBackdrop4.sample(gLinear, _21949, level(0.0)) * (_14487.x * _14487.y));
                                    }
                                    else
                                    {
                                        float2 _21937;
                                        if (_206.gConv.x > 0.5)
                                        {
                                            float2 _20795 = _14499;
                                            _20795.y = 1.0 - _14499.y;
                                            _21937 = _20795;
                                        }
                                        else
                                        {
                                            _21937 = _14499;
                                        }
                                        float _15076 = _14499.y;
                                        float2 _15077 = float2(_14511.x, _15076);
                                        float2 _21938;
                                        if (_206.gConv.x > 0.5)
                                        {
                                            float2 _20802 = _15077;
                                            _20802.y = 1.0 - _15076;
                                            _21938 = _20802;
                                        }
                                        else
                                        {
                                            _21938 = _15077;
                                        }
                                        float _15094 = _14511.y;
                                        float2 _15095 = float2(_14499.x, _15094);
                                        float2 _21940;
                                        if (_206.gConv.x > 0.5)
                                        {
                                            float2 _20809 = _15095;
                                            _20809.y = 1.0 - _15094;
                                            _21940 = _20809;
                                        }
                                        else
                                        {
                                            _21940 = _15095;
                                        }
                                        float2 _21942;
                                        if (_206.gConv.x > 0.5)
                                        {
                                            float2 _20816 = _14511;
                                            _20816.y = 1.0 - _14511.y;
                                            _21942 = _20816;
                                        }
                                        else
                                        {
                                            _21942 = _14511;
                                        }
                                        _21977 = (((gBackdrop5.sample(gLinear, _21937, level(0.0)) * (_14483.x * _14483.y)) + (gBackdrop5.sample(gLinear, _21938, level(0.0)) * (_14487.x * _14483.y))) + (gBackdrop5.sample(gLinear, _21940, level(0.0)) * (_14483.x * _14487.y))) + (gBackdrop5.sample(gLinear, _21942, level(0.0)) * (_14487.x * _14487.y));
                                    }
                                    _21976 = _21977;
                                }
                                _21975 = _21976;
                            }
                            _21974 = _21975;
                        }
                        _21973 = _21974;
                    }
                    _14344 = _22006 + (_21973.xyz * (_14315 ? ((_21936 == 0) ? (1.0 - _14310) : _14310) : 1.0));
                }
                float3 _20820 = _21910;
                _20820.x = _22006.x;
                float2 _10399 = (_10339 + _10351) + (_10354 * _10381);
                float _15191 = fast::clamp(log2(fast::max(_10253, 1.0)) - 1.0, 0.0, 5.0);
                int _15194 = int(floor(_15191));
                float _15198 = _15191 - float(_15194);
                bool _15203 = (_15198 > 0.0199999995529651641845703125) && (_15194 < 5);
                float3 _22130;
                _22130 = float3(0.0);
                float3 _15232;
                for (int _22060 = 0; _22060 < 2; _22130 = _15232, _22060++)
                {
                    if ((_22060 > 0) && (!_15203))
                    {
                        break;
                    }
                    int _15218 = _15194 + _22060;
                    int _15256 = clamp(_15218, 1, 5);
                    float2 _15325 = (_10399 * _206.gLevel[_15256].xy) - float2(0.5);
                    float2 _15327 = floor(_15325);
                    float2 _15330 = _15325 - _15327;
                    float2 _15333 = _15330 * _15330;
                    float2 _15336 = _15333 * _15330;
                    float2 _15355 = (((_15336 * 3.0) - (_15333 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                    float2 _15368 = _15336 * 0.16666667163372039794921875;
                    float2 _15371 = (((((-_15336) + (_15333 * 3.0)) - (_15330 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15355;
                    float2 _15375 = (((((_15336 * (-3.0)) + (_15333 * 3.0)) + (_15330 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15368;
                    float2 _15387 = ((_15327 - float2(0.5)) + (_15355 / _15371)) * _206.gLevel[_15256].zw;
                    float2 _15399 = ((_15327 + float2(1.5)) + (_15368 / _15375)) * _206.gLevel[_15256].zw;
                    float4 _22097;
                    if (_15218 <= 0)
                    {
                        float2 _22096;
                        if (_206.gConv.x > 0.5)
                        {
                            float2 _20823 = _10399;
                            _20823.y = 1.0 - _10399.y;
                            _22096 = _20823;
                        }
                        else
                        {
                            _22096 = _10399;
                        }
                        _22097 = gBackdrop0.sample(gLinear, _22096, level(0.0));
                    }
                    else
                    {
                        float4 _22098;
                        if (_15218 == 1)
                        {
                            float2 _22089;
                            if (_206.gConv.x > 0.5)
                            {
                                float2 _20828 = _15387;
                                _20828.y = 1.0 - _15387.y;
                                _22089 = _20828;
                            }
                            else
                            {
                                _22089 = _15387;
                            }
                            float _15444 = _15387.y;
                            float2 _15445 = float2(_15399.x, _15444);
                            float2 _22090;
                            if (_206.gConv.x > 0.5)
                            {
                                float2 _20835 = _15445;
                                _20835.y = 1.0 - _15444;
                                _22090 = _20835;
                            }
                            else
                            {
                                _22090 = _15445;
                            }
                            float _15462 = _15399.y;
                            float2 _15463 = float2(_15387.x, _15462);
                            float2 _22092;
                            if (_206.gConv.x > 0.5)
                            {
                                float2 _20842 = _15463;
                                _20842.y = 1.0 - _15462;
                                _22092 = _20842;
                            }
                            else
                            {
                                _22092 = _15463;
                            }
                            float2 _22094;
                            if (_206.gConv.x > 0.5)
                            {
                                float2 _20849 = _15399;
                                _20849.y = 1.0 - _15399.y;
                                _22094 = _20849;
                            }
                            else
                            {
                                _22094 = _15399;
                            }
                            _22098 = (((gBackdrop1.sample(gLinear, _22089, level(0.0)) * (_15371.x * _15371.y)) + (gBackdrop1.sample(gLinear, _22090, level(0.0)) * (_15375.x * _15371.y))) + (gBackdrop1.sample(gLinear, _22092, level(0.0)) * (_15371.x * _15375.y))) + (gBackdrop1.sample(gLinear, _22094, level(0.0)) * (_15375.x * _15375.y));
                        }
                        else
                        {
                            float4 _22099;
                            if (_15218 == 2)
                            {
                                float2 _22082;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20856 = _15387;
                                    _20856.y = 1.0 - _15387.y;
                                    _22082 = _20856;
                                }
                                else
                                {
                                    _22082 = _15387;
                                }
                                float _15574 = _15387.y;
                                float2 _15575 = float2(_15399.x, _15574);
                                float2 _22083;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20863 = _15575;
                                    _20863.y = 1.0 - _15574;
                                    _22083 = _20863;
                                }
                                else
                                {
                                    _22083 = _15575;
                                }
                                float _15592 = _15399.y;
                                float2 _15593 = float2(_15387.x, _15592);
                                float2 _22085;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20870 = _15593;
                                    _20870.y = 1.0 - _15592;
                                    _22085 = _20870;
                                }
                                else
                                {
                                    _22085 = _15593;
                                }
                                float2 _22087;
                                if (_206.gConv.x > 0.5)
                                {
                                    float2 _20877 = _15399;
                                    _20877.y = 1.0 - _15399.y;
                                    _22087 = _20877;
                                }
                                else
                                {
                                    _22087 = _15399;
                                }
                                _22099 = (((gBackdrop2.sample(gLinear, _22082, level(0.0)) * (_15371.x * _15371.y)) + (gBackdrop2.sample(gLinear, _22083, level(0.0)) * (_15375.x * _15371.y))) + (gBackdrop2.sample(gLinear, _22085, level(0.0)) * (_15371.x * _15375.y))) + (gBackdrop2.sample(gLinear, _22087, level(0.0)) * (_15375.x * _15375.y));
                            }
                            else
                            {
                                float4 _22100;
                                if (_15218 == 3)
                                {
                                    float2 _22075;
                                    if (_206.gConv.x > 0.5)
                                    {
                                        float2 _20884 = _15387;
                                        _20884.y = 1.0 - _15387.y;
                                        _22075 = _20884;
                                    }
                                    else
                                    {
                                        _22075 = _15387;
                                    }
                                    float _15704 = _15387.y;
                                    float2 _15705 = float2(_15399.x, _15704);
                                    float2 _22076;
                                    if (_206.gConv.x > 0.5)
                                    {
                                        float2 _20891 = _15705;
                                        _20891.y = 1.0 - _15704;
                                        _22076 = _20891;
                                    }
                                    else
                                    {
                                        _22076 = _15705;
                                    }
                                    float _15722 = _15399.y;
                                    float2 _15723 = float2(_15387.x, _15722);
                                    float2 _22078;
                                    if (_206.gConv.x > 0.5)
                                    {
                                        float2 _20898 = _15723;
                                        _20898.y = 1.0 - _15722;
                                        _22078 = _20898;
                                    }
                                    else
                                    {
                                        _22078 = _15723;
                                    }
                                    float2 _22080;
                                    if (_206.gConv.x > 0.5)
                                    {
                                        float2 _20905 = _15399;
                                        _20905.y = 1.0 - _15399.y;
                                        _22080 = _20905;
                                    }
                                    else
                                    {
                                        _22080 = _15399;
                                    }
                                    _22100 = (((gBackdrop3.sample(gLinear, _22075, level(0.0)) * (_15371.x * _15371.y)) + (gBackdrop3.sample(gLinear, _22076, level(0.0)) * (_15375.x * _15371.y))) + (gBackdrop3.sample(gLinear, _22078, level(0.0)) * (_15371.x * _15375.y))) + (gBackdrop3.sample(gLinear, _22080, level(0.0)) * (_15375.x * _15375.y));
                                }
                                else
                                {
                                    float4 _22101;
                                    if (_15218 == 4)
                                    {
                                        float2 _22068;
                                        if (_206.gConv.x > 0.5)
                                        {
                                            float2 _20912 = _15387;
                                            _20912.y = 1.0 - _15387.y;
                                            _22068 = _20912;
                                        }
                                        else
                                        {
                                            _22068 = _15387;
                                        }
                                        float _15834 = _15387.y;
                                        float2 _15835 = float2(_15399.x, _15834);
                                        float2 _22069;
                                        if (_206.gConv.x > 0.5)
                                        {
                                            float2 _20919 = _15835;
                                            _20919.y = 1.0 - _15834;
                                            _22069 = _20919;
                                        }
                                        else
                                        {
                                            _22069 = _15835;
                                        }
                                        float _15852 = _15399.y;
                                        float2 _15853 = float2(_15387.x, _15852);
                                        float2 _22071;
                                        if (_206.gConv.x > 0.5)
                                        {
                                            float2 _20926 = _15853;
                                            _20926.y = 1.0 - _15852;
                                            _22071 = _20926;
                                        }
                                        else
                                        {
                                            _22071 = _15853;
                                        }
                                        float2 _22073;
                                        if (_206.gConv.x > 0.5)
                                        {
                                            float2 _20933 = _15399;
                                            _20933.y = 1.0 - _15399.y;
                                            _22073 = _20933;
                                        }
                                        else
                                        {
                                            _22073 = _15399;
                                        }
                                        _22101 = (((gBackdrop4.sample(gLinear, _22068, level(0.0)) * (_15371.x * _15371.y)) + (gBackdrop4.sample(gLinear, _22069, level(0.0)) * (_15375.x * _15371.y))) + (gBackdrop4.sample(gLinear, _22071, level(0.0)) * (_15371.x * _15375.y))) + (gBackdrop4.sample(gLinear, _22073, level(0.0)) * (_15375.x * _15375.y));
                                    }
                                    else
                                    {
                                        float2 _22061;
                                        if (_206.gConv.x > 0.5)
                                        {
                                            float2 _20940 = _15387;
                                            _20940.y = 1.0 - _15387.y;
                                            _22061 = _20940;
                                        }
                                        else
                                        {
                                            _22061 = _15387;
                                        }
                                        float _15964 = _15387.y;
                                        float2 _15965 = float2(_15399.x, _15964);
                                        float2 _22062;
                                        if (_206.gConv.x > 0.5)
                                        {
                                            float2 _20947 = _15965;
                                            _20947.y = 1.0 - _15964;
                                            _22062 = _20947;
                                        }
                                        else
                                        {
                                            _22062 = _15965;
                                        }
                                        float _15982 = _15399.y;
                                        float2 _15983 = float2(_15387.x, _15982);
                                        float2 _22064;
                                        if (_206.gConv.x > 0.5)
                                        {
                                            float2 _20954 = _15983;
                                            _20954.y = 1.0 - _15982;
                                            _22064 = _20954;
                                        }
                                        else
                                        {
                                            _22064 = _15983;
                                        }
                                        float2 _22066;
                                        if (_206.gConv.x > 0.5)
                                        {
                                            float2 _20961 = _15399;
                                            _20961.y = 1.0 - _15399.y;
                                            _22066 = _20961;
                                        }
                                        else
                                        {
                                            _22066 = _15399;
                                        }
                                        _22101 = (((gBackdrop5.sample(gLinear, _22061, level(0.0)) * (_15371.x * _15371.y)) + (gBackdrop5.sample(gLinear, _22062, level(0.0)) * (_15375.x * _15371.y))) + (gBackdrop5.sample(gLinear, _22064, level(0.0)) * (_15371.x * _15375.y))) + (gBackdrop5.sample(gLinear, _22066, level(0.0)) * (_15375.x * _15375.y));
                                    }
                                    _22100 = _22101;
                                }
                                _22099 = _22100;
                            }
                            _22098 = _22099;
                        }
                        _22097 = _22098;
                    }
                    _15232 = _22130 + (_22097.xyz * (_15203 ? ((_22060 == 0) ? (1.0 - _15198) : _15198) : 1.0));
                }
                _20820.z = _22130.z;
                _22300 = _20820;
            }
            else
            {
                _22300 = _21910;
            }
            float3 _22520;
            if (_10281 > 0.0)
            {
                float2 _10422 = _10339 + (((_21838 * fast::min(_10272 * 0.5, 16.0)) * _206.gDisplay.zw) * _206.gTarget.zw);
                float _16079 = fast::clamp(log2(fast::max(16.0 * _206.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _16082 = int(floor(_16079));
                float _16086 = _16079 - float(_16082);
                bool _16091 = (_16086 > 0.0199999995529651641845703125) && (_16082 < 5);
                float3 _22285;
                _22285 = float3(0.0);
                float3 _16120;
                for (int _22270 = 0; _22270 < 2; _22285 = _16120, _22270++)
                {
                    if ((_22270 > 0) && (!_16091))
                    {
                        break;
                    }
                    int _16106 = _16082 + _22270;
                    float2 _22271;
                    if (_206.gConv.x > 0.5)
                    {
                        float2 _20968 = _10422;
                        _20968.y = 1.0 - _10422.y;
                        _22271 = _20968;
                    }
                    else
                    {
                        _22271 = _10422;
                    }
                    float4 _22272;
                    if (_16106 <= 0)
                    {
                        _22272 = gBackdrop0.sample(gLinear, _22271, level(0.0));
                    }
                    else
                    {
                        float4 _22273;
                        if (_16106 == 1)
                        {
                            _22273 = gBackdrop1.sample(gLinear, _22271, level(0.0));
                        }
                        else
                        {
                            float4 _22274;
                            if (_16106 == 2)
                            {
                                _22274 = gBackdrop2.sample(gLinear, _22271, level(0.0));
                            }
                            else
                            {
                                float4 _22275;
                                if (_16106 == 3)
                                {
                                    _22275 = gBackdrop3.sample(gLinear, _22271, level(0.0));
                                }
                                else
                                {
                                    float4 _22276;
                                    if (_16106 == 4)
                                    {
                                        _22276 = gBackdrop4.sample(gLinear, _22271, level(0.0));
                                    }
                                    else
                                    {
                                        _22276 = gBackdrop5.sample(gLinear, _22271, level(0.0));
                                    }
                                    _22275 = _22276;
                                }
                                _22274 = _22275;
                            }
                            _22273 = _22274;
                        }
                        _22272 = _22273;
                    }
                    _16120 = _22285 + (_22272.xyz * (_16091 ? ((_22270 == 0) ? (1.0 - _16086) : _16086) : 1.0));
                }
                _22520 = _22285;
            }
            else
            {
                _22520 = float3(0.5);
            }
            float _22316;
            if (in.i_shape.x > 0.001000000047497451305389404296875)
            {
                int _16209 = clamp(int(rint(log2(36.0 * _206.gDisplay.z) - 1.0)), 1, 4);
                float2 _22291;
                if (_206.gConv.x > 0.5)
                {
                    float2 _20972 = _10339;
                    _20972.y = 1.0 - _10339.y;
                    _22291 = _20972;
                }
                else
                {
                    _22291 = _10339;
                }
                float4 _22292;
                if (_16209 <= 0)
                {
                    _22292 = gBackdrop0.sample(gLinear, _22291, level(0.0));
                }
                else
                {
                    float4 _22293;
                    if (_16209 == 1)
                    {
                        _22293 = gBackdrop1.sample(gLinear, _22291, level(0.0));
                    }
                    else
                    {
                        float4 _22294;
                        if (_16209 == 2)
                        {
                            _22294 = gBackdrop2.sample(gLinear, _22291, level(0.0));
                        }
                        else
                        {
                            float4 _22295;
                            if (_16209 == 3)
                            {
                                _22295 = gBackdrop3.sample(gLinear, _22291, level(0.0));
                            }
                            else
                            {
                                float4 _22296;
                                if (_16209 == 4)
                                {
                                    _22296 = gBackdrop4.sample(gLinear, _22291, level(0.0));
                                }
                                else
                                {
                                    _22296 = gBackdrop5.sample(gLinear, _22291, level(0.0));
                                }
                                _22295 = _22296;
                            }
                            _22294 = _22295;
                        }
                        _22293 = _22294;
                    }
                    _22292 = _22293;
                }
                _22316 = dot(_22292.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _22316 = 0.5;
            }
            _22518 = _22520;
            _22315 = _22316;
            _22297 = _22300;
        }
        else
        {
            _22518 = float3(0.5);
            _22315 = 0.5;
            _22297 = float3(0.5);
        }
        float3 _10450 = mix(float3(dot(_22297, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _22297, float3(gFxData_1._data[_10172].x)) + float3(gFxData_1._data[_10172].y);
        float3 _22411;
        if (in.i_shape.x > 0.001000000047497451305389404296875)
        {
            float _10460 = fast::clamp(fast::max(_22315 + gFxData_1._data[_10172].y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _10468 = mix(_10460, dot(gFxData_1._data[_10164].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), in.i_shape.x);
            _22411 = select(mix(_10450, float3(1.0), float3((_10468 - _10460) / fast::max(1.0 - _10460, 0.001000000047497451305389404296875))), _10450 * (_10468 / _10460), bool3(_10468 < _10460));
        }
        else
        {
            _22411 = _10450;
        }
        float2 _10506 = fast::clamp((in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875)), float2(0.0), float2(1.0));
        float2 _16321 = select(_248.gDrawInfo.yz, _248.gDrawInfo.yz - in.i_local, bool2(_248.gDrawInfo.w > 1.0));
        float2 _10559 = select(float2(cos(gFxData_1._data[_10172].w), sin(gFxData_1._data[_10172].w)), _16321 / float2(fast::max(length(_16321), 9.9999997473787516355514526367188e-05)), bool2(_248.gDrawInfo.w > 0.5));
        float _10562 = dot(_21838, _10559);
        float _10573 = -_21245;
        float _10580 = _4628 * ((0.5 * abs(_21838.x)) + 0.0199999995529651641845703125);
        float _10587 = _4628 * ((0.5 * abs(_21838.y)) + 0.0199999995529651641845703125);
        float _22630;
        if ((((_10573 + _10580) + _10587) > 0.0) && (((_10573 - _10580) - _10587) < (0.5 * _10272)))
        {
            float _16340 = fast::max((_10573 + _10580) + _10587, 0.0) / _10272;
            float _16344 = fast::clamp(sqrt(2.0 * _16340), 0.12399999797344207763671875, 1.0);
            float _16346 = 1.0 - _16344;
            float _16349 = _16346 * _16346;
            float _16403 = fast::max((_10573 + _10580) - _10587, 0.0) / _10272;
            float _16407 = fast::clamp(sqrt(2.0 * _16403), 0.12399999797344207763671875, 1.0);
            float _16409 = 1.0 - _16407;
            float _16412 = _16409 * _16409;
            float _16466 = fast::max((_10573 - _10580) + _10587, 0.0) / _10272;
            float _16470 = fast::clamp(sqrt(2.0 * _16466), 0.12399999797344207763671875, 1.0);
            float _16472 = 1.0 - _16470;
            float _16475 = _16472 * _16472;
            float _16529 = fast::max((_10573 - _10580) - _10587, 0.0) / _10272;
            float _16533 = fast::clamp(sqrt(2.0 * _16529), 0.12399999797344207763671875, 1.0);
            float _16535 = 1.0 - _16533;
            float _16538 = _16535 * _16535;
            float _10634 = ((((((_16340 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _16340) * _16340) : ((_16340 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _16344) * _16344)) + ((((_16349 * _16349) * _16349) * _16346) * (0.02380952425301074981689453125 + (_16346 * ((-0.0386904776096343994140625) + (_16346 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_16340 - 0.5))))) * _10272) * _10272) - ((((_16403 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _16403) * _16403) : ((_16403 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _16407) * _16407)) + ((((_16412 * _16412) * _16412) * _16409) * (0.02380952425301074981689453125 + (_16409 * ((-0.0386904776096343994140625) + (_16409 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_16403 - 0.5))))) * _10272) * _10272)) - ((((_16466 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _16466) * _16466) : ((_16466 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _16470) * _16470)) + ((((_16475 * _16475) * _16475) * _16472) * (0.02380952425301074981689453125 + (_16472 * ((-0.0386904776096343994140625) + (_16472 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_16466 - 0.5))))) * _10272) * _10272)) + ((((_16529 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _16529) * _16529) : ((_16529 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _16533) * _16533)) + ((((_16538 * _16538) * _16538) * _16535) * (0.02380952425301074981689453125 + (_16535 * ((-0.0386904776096343994140625) + (_16535 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_16529 - 0.5))))) * _10272) * _10272);
            _22630 = _10634 / ((4.0 * _10580) * _10587);
        }
        else
        {
            _22630 = 0.0;
        }
        float _10666 = fast::clamp(0.5 + (0.5 * dot((in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5)) / _10264, _10559)), 0.0, 1.0);
        _24044 = ((_22630 * (pow(fast::clamp(_10562, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(fast::clamp(-_10562, 0.0, 1.0), 1.5)))) * gFxData_1._data[_10172].z) * 1.60000002384185791015625;
        _23608 = gFxData_1._data[_10180];
        _23432 = float4((mix(mix(_22411, gFxData_1._data[_10164].xyz, float3(fast::clamp(gFxData_1._data[_10164].w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _10287))) + (0.3499999940395355224609375 * ((dot(gFxData_1._data[_10164].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _10506.y) : _10506.y))), 0.0, 1.0))), (_22518 * 1.10000002384185791015625) + float3(0.07999999821186065673828125), float3(pow(1.0 - _22415.z, 5.0) * 0.3499999940395355224609375)) + float3((((0.039999999105930328369140625 * _10666) * _10666) + (0.0500000007450580596923828125 * (1.0 - _10287))) * gFxData_1._data[_10172].z)) * _21415, _21415) + (_22787 * (1.0 - _21415));
    }
    else
    {
        _24044 = 0.0;
        _23608 = _21224;
        _23432 = _22787;
    }
    float4 _23444;
    float4 _23446;
    if ((in.i_flags.x & _3359) != 0u)
    {
        float4 _22933;
        float4 _23172;
        if (in.i_flags.y != 0u)
        {
            _23172 = gFxData_1._data[(in.i_instance * 24u) + 3u];
            _22933 = gFxData_1._data[(in.i_instance * 24u) + 4u];
        }
        else
        {
            _23172 = float4(0.0);
            _22933 = float4(0.0);
        }
        float4 _23419;
        do
        {
            if (in.i_flags.y == 0u)
            {
                _23419 = in.i_fill0;
                break;
            }
            float2 _16642 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float2 _16652 = fwidth(in.i_local);
            float _16654 = fast::max(length(_16652), 9.9999997473787516355514526367188e-05);
            float _23411;
            float _23415;
            if ((in.i_flags.y == 1u) || (in.i_flags.y == 4u))
            {
                float _16663 = cos(_22933.x);
                float _16666 = sin(_22933.x);
                float _16692 = ((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_16663, _16666)) / fast::max(0.5 * ((abs(_16663) * _16642.x) + (abs(_16666) * _16642.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (in.i_flags.y == 4u)
                {
                    float _16707 = fast::clamp((_16692 - _22933.y) / fast::max(_22933.z - _22933.y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _23419 = float4(fast::clamp(abs((fract(float3(_16707 * 0.800000011920928955078125) + float3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - float3(3.0)) - float3(1.0), float3(0.0), float3(1.0)), in.i_fill0.w * pow(fast::max(sin(_16707 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _23415 = -1.0;
                _23411 = _16692;
            }
            else
            {
                float _23412;
                float _23416;
                if (in.i_flags.y == 2u)
                {
                    _23416 = -1.0;
                    _23412 = length(in.i_local - (in.i_rect.xy + (_22933.xy * _16642))) / fast::max(_22933.z * fast::max(_16642.x, _16642.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    float2 _16775 = in.i_local - (in.i_rect.xy + (_22933.xy * _16642));
                    float _16786 = fract(((precise::atan2(_16775.y, _16775.x) - _22933.z) * 0.15915493667125701904296875) + 1.0);
                    float _23413;
                    float _23417;
                    if (_22933.w > 0.5)
                    {
                        _23417 = -1.0;
                        _23413 = 0.5 - (0.5 * cos(_16786 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _16806 = (((_16786 < 0.5) ? _16786 : (_16786 - 1.0)) * 6.283185482025146484375) * length(_16775);
                        float _23418;
                        if (abs(_16806) < _16654)
                        {
                            _23418 = fast::clamp(((_16806 / _16654) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _23418 = -1.0;
                        }
                        _23417 = _23418;
                        _23413 = _16786;
                    }
                    _23416 = _23417;
                    _23412 = _23413;
                }
                _23415 = _23416;
                _23411 = _23412;
            }
            float4 _16835 = float4(in.i_fill0.xyz * in.i_fill0.w, in.i_fill0.w);
            float4 _16847 = float4(_23172.xyz * _23172.w, _23172.w);
            float4 _16861 = select(mix(_16835, _16847, float4(fast::clamp(_23411, 0.0, 1.0))), mix(_16847, _16835, float4(_23415)), bool4(_23415 >= 0.0));
            _23419 = select(float4(0.0), float4(_16861.xyz / float3(_16861.w), _16861.w), bool4(_16861.w > 9.9999997473787516355514526367188e-06));
            break;
        } while(false);
        float4 _23420;
        float4 _23598;
        if ((in.i_flags.x & _3388) != 0u)
        {
            uint _16886 = (in.i_instance * 24u) + 18u;
            _23598 = gFxData_1._data[_16886];
            _23420 = _23419 * gTex.sample(gLinear, mix(gFxData_1._data[_16886].xy, gFxData_1._data[_16886].zw, (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875))));
        }
        else
        {
            _23598 = _21220;
            _23420 = _23419;
        }
        float _16896 = fast::clamp(_23420.w * _21415, 0.0, 1.0);
        _23446 = _23598;
        _23444 = float4(_23420.xyz * _16896, _16896) + (_23432 * (1.0 - _16896));
    }
    else
    {
        _23446 = _21220;
        _23444 = _23432;
    }
    float4 _24027;
    if ((in.i_flags.x & _3441) != 0u)
    {
        uint _16920 = (in.i_instance * 24u) + 21u;
        uint _16928 = (in.i_instance * 24u) + 3u;
        float2 _5281 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float2 _5288 = (in.i_local - in.i_rect.xy) / _5281;
        float _5299 = ((gFxData_1._data[_16928].x >= 0.0) ? gFxData_1._data[_16928].x : _206.gTime.x) * gFxData_1._data[_16920].z;
        float _5302 = _5288.x * 2.0;
        float _5303 = _5302 - 1.0;
        float _5308 = _5288.y * _5281.y;
        float _5311 = fast::max(gFxData_1._data[_16920].w, 0.001000000047497451305389404296875);
        float _5316 = fast::clamp(1.0 - (_5303 * _5303), 0.0, 1.0);
        float _5318 = pow(_5316, 1.2999999523162841796875);
        float _5320 = pow(_5316, 0.699999988079071044921875);
        float _5323 = fast::clamp(_5299 * 1.4285714626312255859375, 0.0, 1.0);
        float _5339 = (0.25 + (0.75 * ((_5323 * _5323) * (3.0 - (2.0 * _5323))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_5299 * 2.099999904632568359375)));
        float _5348 = ((gFxData_1._data[_16920].y * _5281.y) * _5339) * _5318;
        float _5374 = (_5281.y * (0.5 + ((gFxData_1._data[_16920].x * (0.5 - (_5303 * _5303))) * 0.5))) + (((0.14000000059604644775390625 * _5281.y) * _5318) * sin(((_5303 * 2.400000095367431640625) - (_5299 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _23438;
        float _23439;
        float3 _23440;
        _23440 = float3(0.0);
        _23439 = _5374;
        _23438 = _5374;
        float3 _5446;
        float _24596;
        float _24597;
        for (int _23437 = 0; _23437 < 4; _23440 = _5446, _23439 = _24597, _23438 = _24596, _23437++)
        {
            float _5398 = _5374 + ((_5348 * _3603[_23437].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_5299 * 1.7000000476837158203125) + _3620[_23437].y))));
            _24596 = (_23437 == 0) ? _5398 : _23438;
            _24597 = (_23437 == 2) ? _5398 : _23439;
            float _5413 = _5311 * _3603[_23437].y;
            float _5418 = (_5308 - _5398) / _5413;
            float _5423 = _5311 * _3603[_23437].z;
            float3 _24528;
            _24528 = float3(0.0);
            for (int _24527 = 0; _24527 < 6; )
            {
                float _16953 = ((_5308 - _5398) - (_5423 * ((float(_24527) * 0.4000000059604644775390625) - 1.0))) / _5413;
                _24528 += (_1986[_24527] * exp((-_16953) * _16953));
                _24527++;
                continue;
            }
            _5446 = _23440 + (mix(_24528 * float3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), float3(exp((-_5418) * _5418)), float3(_3620[_23437].x)) * (_3603[_23437].w * _5320));
        }
        float _5452 = _5311 * 1.5;
        float _5482 = fast::clamp((_5308 - _23438) / fast::max(_23439 - _23438, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _5510 = (_5308 - (_23439 - (_5311 * 3.0))) / (((_5281.y * 0.0900000035762786865234375) + (_5348 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _5513 = (_5302 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _5538 = ((_5308 - _23438) + (_5311 * 5.0)) / (_5311 * 7.0);
        float3 _5564 = float3(1.0) - exp((-((((_23440 + (mix(float3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), float3(1.0), float3(_5482)) * ((((1.0 / (1.0 + exp((-((_5308 - _23438) - (_5311 * 2.0))) / _5452))) / (1.0 + exp((-(_23439 - _5308)) / _5452))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_5482, 2.5)))) * _5320))) + (float3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_5510) * _5510) - (_5513 * _5513)) * (0.5 + (1.10000002384185791015625 * _5339))))) + (float3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_5538) * _5538) * _5318) * 0.100000001490116119384765625))) * mix(float3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), float3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), float3(0.5 + (0.5 * sin(_5299 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _5574 = (in.i_fill0.w * smoothstep(0.0, 0.119999997317790985107421875, _5288.y)) * smoothstep(1.0, 0.87999999523162841796875, _5288.y);
        float _5593 = (fast::clamp(fast::max(_5564.x, fast::max(_5564.y, _5564.z)), 0.0, 1.0) * _5574) * _21415;
        _24027 = float4((_5564 * _5574) * _21415, _5593) + (_23444 * (1.0 - _5593));
    }
    else
    {
        _24027 = _23444;
    }
    float4 _24039;
    if (((in.i_flags.x & _3872) != 0u) && ((in.i_flags.x & _3876) != 0u))
    {
        uint _16985 = (in.i_instance * 24u) + 7u;
        uint _16993 = (in.i_instance * 24u) + 8u;
        float2 _17040 = select(_248.gDrawInfo.yz, _248.gDrawInfo.yz - ((in.i_rect.xy + in.i_rect.zw) * 0.5), bool2(_248.gDrawInfo.w > 1.0));
        float2 _5626 = in.i_local - select(gFxData_1._data[_16993].zw, (-(_17040 / float2(fast::max(length(_17040), 9.9999997473787516355514526367188e-05)))) * length(gFxData_1._data[_16993].zw), bool2(_248.gDrawInfo.w > 0.5));
        float2 _17087 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _17096 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _23988;
        if ((in.i_flags.x & _1416) != 0u)
        {
            float _17202 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
            float4 _17236 = gTex.sample(gLinear, mix(_23446.xy, _23446.zw, (fast::clamp(_5626, _21226.xy, _21226.zw) - _21226.xy) / fast::max(_21226.zw - _21226.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
            float _17260 = (_17236.w - 0.5) * in.i_radii.y;
            float2 _17301 = abs(_5626 - ((_21226.xy + _21226.zw) * 0.5)) - fast::max(((_21226.zw - _21226.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _17202), float2(0.0));
            _23988 = fast::max((-mix((fast::max(fast::min(_17236.x, _17236.y), fast::min(fast::max(_17236.x, _17236.y), _17236.z)) - 0.5) * in.i_radii.x, _17260, fast::clamp(abs(_17260) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _17202, length(fast::max(_17301, float2(0.0))) + fast::min(fast::max(_17301.x, _17301.y), 0.0)) - in.i_shape.w;
        }
        else
        {
            float _23989;
            if (in.i_flags.z == 1u)
            {
                float2 _17111 = _5626 - _17087;
                float _23987;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _23987 = abs(length(_17111) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _17345 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _17347 = cos(_17345);
                    float _17349 = sin(_17345);
                    float _17358 = dot(_17111, float2(-_17349, _17347));
                    float _17361 = dot(_17111, float2(_17347, _17349));
                    float2 _17362 = float2(_17358, _17361);
                    float _17365 = abs(_17358);
                    _17362.x = _17365;
                    float _17368 = in.i_shape.w * 0.5;
                    float _17370 = sin(_17368);
                    float _17372 = cos(_17368);
                    _23987 = (((_17372 * _17365) > (_17370 * _17361)) ? length(_17362 - (float2(_17370, _17372) * in.i_radii.x)) : abs(length(_17362) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _23989 = _23987;
            }
            else
            {
                float _23990;
                if (in.i_flags.z == 2u)
                {
                    float2 _17408 = _5626 - _21226.xy;
                    float2 _17411 = _21226.zw - _21226.xy;
                    _23990 = length(_17408 - (_17411 * fast::clamp(dot(_17408, _17411) / fast::max(dot(_17411, _17411), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _17138 = _5626 - _17087;
                    float _17464 = fast::min(_17096.x, _17096.y);
                    float _17467 = fast::min((_17138.x > 0.0) ? ((_17138.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_17138.y > 0.0) ? in.i_radii.w : in.i_radii.x), _17464);
                    float _17473 = _17467 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _23972;
                    float _23973;
                    if (_17473 > _17464)
                    {
                        float _17487 = in.i_shape.y * fast::clamp((_17464 - _17467) / fast::max(0.60000002384185791015625 * _17467, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _23973 = _17487;
                        _23972 = _17467 * (1.0 + (0.60000002384185791015625 * _17487));
                    }
                    else
                    {
                        _23973 = in.i_shape.y;
                        _23972 = _17473;
                    }
                    float2 _17500 = (abs(_17138) - _17096) + float2(_23972);
                    float2 _17502 = fast::max(_17500, float2(0.0));
                    float _23974;
                    if ((_17502.x > 0.0) && (_17502.y > 0.0))
                    {
                        float _23975;
                        if ((_23973 > 0.001000000047497451305389404296875) && (_23972 > 9.9999997473787516355514526367188e-05))
                        {
                            float _17519 = 2.0 + (2.0 * _23973);
                            float2 _17524 = _17502 / float2(fast::max(_23972, 9.9999997473787516355514526367188e-05));
                            _23975 = pow(pow(_17524.x, _17519) + pow(_17524.y, _17519), 1.0 / _17519) * _23972;
                        }
                        else
                        {
                            _23975 = length(_17502);
                        }
                        _23974 = _23975;
                    }
                    else
                    {
                        _23974 = fast::max(_17502.x, _17502.y);
                    }
                    float _17559 = (fast::min(fast::max(_17500.x, _17500.y), 0.0) + _23974) - _23972;
                    float _23991;
                    if ((in.i_flags.x & _1485) != 0u)
                    {
                        float2 _17166 = fast::max((_21226.zw - _21226.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _17169 = _5626 - ((_21226.xy + _21226.zw) * 0.5);
                        float _17595 = fast::min(_17166.x, _17166.y);
                        float _17598 = fast::min((_17169.x > 0.0) ? ((_17169.y > 0.0) ? _23608.x : _23608.x) : ((_17169.y > 0.0) ? _23608.x : _23608.x), _17595);
                        float _17604 = _17598 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _23978;
                        float _23979;
                        if (_17604 > _17595)
                        {
                            float _17618 = in.i_shape.y * fast::clamp((_17595 - _17598) / fast::max(0.60000002384185791015625 * _17598, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _23979 = _17618;
                            _23978 = _17598 * (1.0 + (0.60000002384185791015625 * _17618));
                        }
                        else
                        {
                            _23979 = in.i_shape.y;
                            _23978 = _17604;
                        }
                        float2 _17631 = (abs(_17169) - _17166) + float2(_23978);
                        float2 _17633 = fast::max(_17631, float2(0.0));
                        float _23980;
                        if ((_17633.x > 0.0) && (_17633.y > 0.0))
                        {
                            float _23981;
                            if ((_23979 > 0.001000000047497451305389404296875) && (_23978 > 9.9999997473787516355514526367188e-05))
                            {
                                float _17650 = 2.0 + (2.0 * _23979);
                                float2 _17655 = _17633 / float2(fast::max(_23978, 9.9999997473787516355514526367188e-05));
                                _23981 = pow(pow(_17655.x, _17650) + pow(_17655.y, _17650), 1.0 / _17650) * _23978;
                            }
                            else
                            {
                                _23981 = length(_17633);
                            }
                            _23980 = _23981;
                        }
                        else
                        {
                            _23980 = fast::max(_17633.x, _17633.y);
                        }
                        float _17690 = (fast::min(fast::max(_17631.x, _17631.y), 0.0) + _23980) - _23978;
                        float _17695 = fast::max(_23608.y, 9.9999997473787516355514526367188e-05);
                        float _17704 = fast::max(_17695 - abs(_17559 - _17690), 0.0) / _17695;
                        _23991 = fast::min(_17559, _17690) - (((_17704 * _17704) * _17695) * 0.25);
                    }
                    else
                    {
                        _23991 = _17559;
                    }
                    _23990 = _23991;
                }
                _23989 = _23990;
            }
            _23988 = _23989;
        }
        float _5635 = (_23988 + gFxData_1._data[_16993].y) / (fast::max(gFxData_1._data[_16993].x * 0.5, _4628 * 0.5) * 1.41421353816986083984375);
        float _17721 = sign(_5635);
        float _17723 = abs(_5635);
        float _17734 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_17723 * _17723))) * _17723)) * _17723);
        float _17737 = _17734 * _17734;
        float _17752 = fast::clamp(gFxData_1._data[_16985].w * ((0.5 + (0.5 * (_17721 - (_17721 / (_17737 * _17737))))) * _21415), 0.0, 1.0);
        _24039 = float4(gFxData_1._data[_16985].xyz * _17752, _17752) + (_24027 * (1.0 - _17752));
    }
    else
    {
        _24039 = _24027;
    }
    float4 _24064;
    if ((in.i_flags.x & _3936) != 0u)
    {
        uint _17776 = (in.i_instance * 24u) + 9u;
        uint _17784 = (in.i_instance * 24u) + 10u;
        float _5667 = fast::max(-_21245, 0.0) / fast::max(gFxData_1._data[_17784].z, 0.001000000047497451305389404296875);
        float _17794 = fast::clamp(gFxData_1._data[_17776].w * fast::clamp((exp(((-_5667) * _5667) * 2.2000000476837158203125) * gFxData_1._data[_17784].w) * _21415, 0.0, 1.0), 0.0, 1.0);
        _24064 = float4(gFxData_1._data[_17776].xyz * _17794, _17794) + (_24039 * (1.0 - _17794));
    }
    else
    {
        _24064 = _24039;
    }
    float3 _5698 = _24064.xyz + float3(_24044 * fast::clamp(_24064.w / fast::max(_21415, 0.001000000047497451305389404296875), 0.0, 1.0));
    float4 _21115 = _24064;
    _21115.x = _5698.x;
    _21115.y = _5698.y;
    _21115.z = _5698.z;
    float4 _24293;
    if ((in.i_flags.x & _4005) != 0u)
    {
        uint _17818 = (in.i_instance * 24u) + 5u;
        float4 _17820 = gFxData_1._data[_17818];
        uint _17826 = (in.i_instance * 24u) + 6u;
        float4 _24289;
        if (gFxData_1._data[_17826].z < 0.999000012874603271484375)
        {
            float2 _5760 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float _5771 = cos(gFxData_1._data[_17826].w);
            float _5774 = sin(gFxData_1._data[_17826].w);
            float4 _21132 = _17820;
            _21132.w = _17820.w * mix(1.0, gFxData_1._data[_17826].z, fast::clamp(((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_5771, _5774)) / fast::max(0.5 * ((abs(_5771) * _5760.x) + (abs(_5774) * _5760.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _24289 = _21132;
        }
        else
        {
            _24289 = _17820;
        }
        float _17836 = fast::clamp(_24289.w * ((_24075 >= 0.0) ? _24075 : (fast::clamp(0.5 - ((_21245 - (gFxData_1._data[_17826].x * gFxData_1._data[_17826].y)) / _4628), 0.0, 1.0) - fast::clamp(0.5 - ((_21245 + (gFxData_1._data[_17826].x * (1.0 - gFxData_1._data[_17826].y))) / _4628), 0.0, 1.0))), 0.0, 1.0);
        _24293 = float4(_24289.xyz * _17836, _17836) + (_21115 * (1.0 - _17836));
    }
    else
    {
        _24293 = _21115;
    }
    float4 _24294;
    if ((in.i_flags.x & _4139) != 0u)
    {
        float2 _5835 = (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float _5857 = exp(-pow((((_5835.x * 0.85000002384185791015625) + (_5835.y * 0.1500000059604644775390625)) - ((fract(_206.gTime.x * in.i_misc.w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _24294 = float4(_24293.xyz + float3(((_5857 * in.i_misc.z) * _21415) * fast::max(_24293.w, 0.3499999940395355224609375)), fast::max(_24293.w, ((_5857 * in.i_misc.z) * _21415) * 0.5));
    }
    else
    {
        _24294 = _24293;
    }
    float4 _24522;
    if ((in.i_flags.x & _4221) != 0u)
    {
        float3 _17861 = fract(floor(_21219).xyx * 0.103100001811981201171875);
        float3 _17870 = _17861 + float3(dot(_17861, _17861.yzx + float3(33.3300018310546875)));
        float3 _5908 = _24294.xyz + float3(((fract((_17870.x + _17870.y) * _17870.z) - 0.5) * in.i_misc.y) * _24294.w);
        float4 _21156 = _24294;
        _21156.x = _5908.x;
        _21156.y = _5908.y;
        _21156.z = _5908.z;
        _24522 = _21156;
    }
    else
    {
        _24522 = _24294;
    }
    float _24518;
    if (_248.gFade.z > 0.0)
    {
        _24518 = smoothstep(0.0, 1.0, fast::clamp((_21219.y - _248.gFade.x) / _248.gFade.z, 0.0, 1.0));
    }
    else
    {
        _24518 = 1.0;
    }
    float _24519;
    if (_248.gFade.w > 0.0)
    {
        _24519 = _24518 * smoothstep(0.0, 1.0, fast::clamp((_248.gFade.y - _21219.y) / _248.gFade.w, 0.0, 1.0));
    }
    else
    {
        _24519 = _24518;
    }
    float4 _5925 = _24522 * ((in.i_misc.x * _24306) * _24519);
    float4 _24523;
    if (((in.i_flags.x & _4261) != 0u) || ((in.i_flags.x & _4265) != 0u))
    {
        float3 _17922 = fract((floor(_21219) + float2(17.0)).xyx * 0.103100001811981201171875);
        float3 _17931 = _17922 + float3(dot(_17922, _17922.yzx + float3(33.3300018310546875)));
        float3 _5949 = _5925.xyz + float3(((fract((_17931.x + _17931.y) * _17931.z) - 0.5) * 0.0039215688593685626983642578125) * fast::clamp(_5925.w * 8.0, 0.0, 1.0));
        float4 _21168 = _5925;
        _21168.x = _5949.x;
        _21168.y = _5949.y;
        _21168.z = _5949.z;
        _24523 = _21168;
    }
    else
    {
        _24523 = _5925;
    }
    float3 _5959 = fast::max(_24523.xyz, float3(0.0));
    float4 _21174 = _24523;
    _21174.x = _5959.x;
    _21174.y = _5959.y;
    _21174.z = _5959.z;
    float4 _24524;
    if ((_206.gTime.w > 0.5) && (_24523.w > 9.9999997473787516355514526367188e-06))
    {
        float3 _17975 = fast::clamp(_21174.xyz / float3(_24523.w), float3(0.0), float3(1.0));
        float3 _17961 = select(pow((_17975 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _17975 * float3(0.077399380505084991455078125), _17975 <= float3(0.040449999272823333740234375)) * _24523.w;
        float4 _21183 = _21174;
        _21183.x = _17961.x;
        _21183.y = _17961.y;
        _21183.z = _17961.z;
        _24524 = _21183;
    }
    else
    {
        _24524 = _21174;
    }
    out._entryPointOutput = _24524;
    return out;
}

