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
constant uint _2192 = (32768u & kEsiaFxFeatures);
constant uint _2207 = (262144u & kEsiaFxFeatures);
constant uint _2278 = (512u & kEsiaFxFeatures);
constant uint _2330 = (262144u & kEsiaFxFeatures);
constant uint _2350 = (32768u & kEsiaFxFeatures);
constant uint _2357 = (512u & kEsiaFxFeatures);
constant uint _3542 = (512u & kEsiaFxFeatures);
constant uint _3562 = (32768u & kEsiaFxFeatures);
constant uint _3595 = (131072u & kEsiaFxFeatures);
constant uint _3730 = (2u & kEsiaFxFeatures);
constant uint _3740 = (1024u & kEsiaFxFeatures);
constant uint _3805 = (32u & kEsiaFxFeatures);
constant uint _3813 = (4u & kEsiaFxFeatures);
constant uint _3818 = (256u & kEsiaFxFeatures);
constant uint _3882 = (8u & kEsiaFxFeatures);
constant uint _3911 = (8192u & kEsiaFxFeatures);
constant uint _4127 = (65536u & kEsiaFxFeatures);
constant uint _4203 = (32u & kEsiaFxFeatures);
constant uint _4267 = (1u & kEsiaFxFeatures);
constant uint _4296 = (64u & kEsiaFxFeatures);
constant uint _4349 = (16384u & kEsiaFxFeatures);
constant uint _4780 = (4u & kEsiaFxFeatures);
constant uint _4784 = (256u & kEsiaFxFeatures);
constant uint _4844 = (16u & kEsiaFxFeatures);
constant uint _4913 = (2u & kEsiaFxFeatures);
constant uint _5046 = (128u & kEsiaFxFeatures);
constant uint _5128 = (2048u & kEsiaFxFeatures);
constant uint _5168 = (8u & kEsiaFxFeatures);
constant uint _5172 = (4u & kEsiaFxFeatures);

constant spvUnsafeArray<float3, 6> _2899 = spvUnsafeArray<float3, 6>({ float3(1.0, 0.4199999868869781494140625, 0.2199999988079071044921875), float3(1.0, 0.699999988079071044921875, 0.300000011920928955078125), float3(0.800000011920928955078125, 0.920000016689300537109375, 0.4000000059604644775390625), float3(0.3499999940395355224609375, 0.89999997615814208984375, 0.699999988079071044921875), float3(0.4000000059604644775390625, 0.62000000476837158203125, 1.0), float3(0.660000026226043701171875, 0.5, 1.0) });
constant spvUnsafeArray<float4, 4> _4511 = spvUnsafeArray<float4, 4>({ float4(-1.0, 1.0, 3.400000095367431640625, 2.599999904632568359375), float4(-0.550000011920928955078125, 0.800000011920928955078125, 2.0, 0.800000011920928955078125), float4(0.300000011920928955078125, 1.0, 1.2000000476837158203125, 1.2999999523162841796875), float4(0.62000000476837158203125, 0.800000011920928955078125, 1.60000002384185791015625, 0.449999988079071044921875) });
constant spvUnsafeArray<float2, 4> _4528 = spvUnsafeArray<float2, 4>({ float2(0.0), float2(0.100000001490116119384765625, 1.2999999523162841796875), float2(0.550000011920928955078125, 3.900000095367431640625), float2(0.20000000298023223876953125, 5.19999980926513671875) });

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

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _288 [[buffer(0)]], constant WgtDraw& _330 [[buffer(2)]], const device gFxData& gFxData_1 [[buffer(10)]], texture2d<float> gTex [[texture(3)]], texture2d<float> gBackdrop0 [[texture(4)]], texture2d<float> gBackdrop1 [[texture(5)]], texture2d<float> gBackdrop2 [[texture(6)]], texture2d<float> gBackdrop3 [[texture(7)]], texture2d<float> gBackdrop4 [[texture(8)]], texture2d<float> gBackdrop5 [[texture(9)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float2 _6937 = gl_FragCoord.xy + _288.gConv.yy;
    float2 _36240;
    if (_288.gConv.x > 0.5)
    {
        float2 _34417 = _6937;
        _34417.y = _288.gTarget.y - _6937.y;
        _36240 = _34417;
    }
    else
    {
        _36240 = _6937;
    }
    float _5538 = dfdx(in.i_local.x);
    float _5542 = dfdy(in.i_local.x);
    float _5545 = fast::max(abs(_5538) + abs(_5542), 9.9999997473787516355514526367188e-05);
    float4 _36245;
    float4 _36248;
    if ((in.i_flags.z == 2u) || ((in.i_flags.x & _3542) != 0u))
    {
        _36248 = gFxData_1._data[(in.i_instance * 24u) + 15u];
        _36245 = gFxData_1._data[(in.i_instance * 24u) + 16u];
    }
    else
    {
        _36248 = float4(0.0);
        _36245 = float4(0.0);
    }
    float4 _36241;
    float4 _36247;
    if ((in.i_flags.x & _3562) != 0u)
    {
        _36247 = gFxData_1._data[(in.i_instance * 24u) + 15u];
        _36241 = gFxData_1._data[(in.i_instance * 24u) + 18u];
    }
    else
    {
        _36247 = _36248;
        _36241 = float4(0.0);
    }
    float2 _7026 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
    float2 _7035 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
    float _36352;
    if ((in.i_flags.x & _2192) != 0u)
    {
        float _7152 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
        float4 _7186 = gTex.sample(gLinear, mix(_36241.xy, _36241.zw, (fast::clamp(in.i_local, _36247.xy, _36247.zw) - _36247.xy) / fast::max(_36247.zw - _36247.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
        float _7210 = (_7186.w - 0.5) * in.i_radii.y;
        float2 _7251 = abs(in.i_local - ((_36247.xy + _36247.zw) * 0.5)) - fast::max(((_36247.zw - _36247.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _7152), float2(0.0));
        _36352 = fast::max((-mix((fast::max(fast::min(_7186.x, _7186.y), fast::min(fast::max(_7186.x, _7186.y), _7186.z)) - 0.5) * in.i_radii.x, _7210, fast::clamp(abs(_7210) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _7152, length(fast::max(_7251, float2(0.0))) + fast::min(fast::max(_7251.x, _7251.y), 0.0)) - in.i_shape.w;
    }
    else
    {
        float _36353;
        if ((in.i_flags.x & _2207) != 0u)
        {
            float _36304;
            do
            {
                bool _7317 = in.i_radii.w > 0.5;
                float _7321 = floor((in.i_radii.z + 1.0) * 0.5);
                float _7325 = floor((_7321 + 127.0) * 0.0078125);
                float _36297;
                float _36299;
                if (_7317)
                {
                    uint _7501 = uint(in.i_radii.z);
                    uint _7504 = (_7501 + 31u) >> 5u;
                    uint _7587 = (in.i_instance + 1u) * 24u;
                    uint _36289;
                    uint _36290;
                    _36290 = 0u;
                    _36289 = _7587;
                    uint _7614;
                    float _43865;
                    uint _43866;
                    uint _43867;
                    uint _36288 = 0u;
                    uint _36348 = _7587;
                    float _36349 = 3.0000000054977557577780399428115e+38;
                    for (; _36288 < _7504; _36349 = _43865, _36348 = _7614, _36290 = _43866, _36289 = _43867, _36288++)
                    {
                        uint _7592 = _36348 + 20u;
                        float2 _7607 = fast::max(fast::max(gFxData_1._data[_7592].xy - in.i_local, in.i_local - gFxData_1._data[_7592].zw), float2(0.0));
                        float _7610 = dot(_7607, _7607);
                        bool _7522 = _7610 < _36349;
                        _43865 = _7522 ? _7610 : _36349;
                        _43866 = _7522 ? _36288 : _36290;
                        _43867 = _7522 ? _36348 : _36289;
                        _7614 = _36348 + 24u;
                    }
                    uint _7652 = _36290 * 32u;
                    uint _7656 = min((_7501 - _7652), 32u);
                    uint _7760 = _36289 + 21u;
                    uint _7767 = _36289 + 22u;
                    float _36293;
                    uint _36294;
                    _36294 = 4294967295u;
                    _36293 = 1000000015047466219876688855040.0;
                    float _42984;
                    uint _42985;
                    for (uint _36292 = 0u; (_36292 * 8u) < _7656; _36294 = _42985, _36293 = _42984, _36292++)
                    {
                        uint _7774 = _36289 + (16u + _36292);
                        float _7794 = ((sqrt(_36293) * 1.000003814697265625) + 0.03125) + ((abs(in.i_local.x) + abs(in.i_local.y)) * 3.814697265625e-06);
                        float2 _7814 = fast::max(fast::max(gFxData_1._data[_7774].xy - in.i_local, in.i_local - gFxData_1._data[_7774].zw), float2(0.0));
                        if (dot(_7814, _7814) > (_7794 * _7794))
                        {
                            _42985 = _36294;
                            _42984 = _36293;
                            continue;
                        }
                        float4 _7682 = select(gFxData_1._data[_7767], gFxData_1._data[_7760], bool4(_36292 < 2u));
                        uint _7696 = min(_7656, ((_36292 * 8u) + 8u));
                        uint _7698 = _36292 * 8u;
                        float2 _36333;
                        float _36334;
                        uint _36335;
                        _36335 = _36294;
                        _36334 = _36293;
                        _36333 = select(_7682.xy, _7682.zw, bool2((_36292 & 1u) != 0u));
                        float2 _43028;
                        float _43029;
                        uint _43030;
                        for (uint _36332 = _7698; _36332 < _7696; _36335 = _43030, _36334 = _43029, _36333 = _43028, _36332 += 2u)
                        {
                            uint _7822 = _36289 + (_36332 >> 1u);
                            uint _7711 = _7652 + _36332;
                            float2 _7832 = _36333 - gFxData_1._data[_7822].xy;
                            float2 _7835 = in.i_local - gFxData_1._data[_7822].xy;
                            float2 _7848 = _7835 - (_7832 * fast::clamp(dot(_7835, _7832) / fast::max(dot(_7832, _7832), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                            float _7851 = dot(_7848, _7848);
                            bool _7862 = (_7851 < _36334) || ((_7851 == _36334) && (_7711 < _36335));
                            uint _43868 = _7862 ? _7711 : _36335;
                            float _43869 = _7862 ? _7851 : _36334;
                            if ((_36332 + 1u) < _7696)
                            {
                                uint _7733 = (_7652 + _36332) + 1u;
                                float2 _7875 = gFxData_1._data[_7822].xy - gFxData_1._data[_7822].zw;
                                float2 _7878 = in.i_local - gFxData_1._data[_7822].zw;
                                float2 _7891 = _7878 - (_7875 * fast::clamp(dot(_7878, _7875) / fast::max(dot(_7875, _7875), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _7894 = dot(_7891, _7891);
                                bool _7905 = (_7894 < _43869) || ((_7894 == _43869) && (_7733 < _43868));
                                _43030 = _7905 ? _7733 : _43868;
                                _43029 = _7905 ? _7894 : _43869;
                                _43028 = gFxData_1._data[_7822].zw;
                            }
                            else
                            {
                                _43030 = _43868;
                                _43029 = _43869;
                                _43028 = gFxData_1._data[_7822].xy;
                            }
                        }
                        _42985 = _36335;
                        _42984 = _36334;
                    }
                    uint _7914 = (in.i_instance + 1u) * 24u;
                    float _36296;
                    _36296 = _36293;
                    uint _8262;
                    float _42986;
                    uint _43012;
                    for (uint _36295 = 0u, _36307 = _7914, _36308 = _36294; _36295 < _7504; _36308 = _43012, _36307 = _8262, _36296 = _42986, _36295++)
                    {
                        uint _7919 = _36307 + 20u;
                        float _7939 = ((sqrt(_36296) * 1.000003814697265625) + 0.03125) + ((abs(in.i_local.x) + abs(in.i_local.y)) * 3.814697265625e-06);
                        float2 _7959 = fast::max(fast::max(gFxData_1._data[_7919].xy - in.i_local, in.i_local - gFxData_1._data[_7919].zw), float2(0.0));
                        if ((_36295 != _36290) && (!(dot(_7959, _7959) > (_7939 * _7939))))
                        {
                            uint _8000 = _36295 * 32u;
                            uint _8004 = min((_7501 - _8000), 32u);
                            uint _8108 = _36307 + 21u;
                            uint _8115 = _36307 + 22u;
                            float _36310;
                            uint _36311;
                            _36311 = _36308;
                            _36310 = _36296;
                            float _43013;
                            uint _43014;
                            for (uint _36309 = 0u; (_36309 * 8u) < _8004; _36311 = _43014, _36310 = _43013, _36309++)
                            {
                                uint _8122 = _36307 + (16u + _36309);
                                float _8142 = ((sqrt(_36310) * 1.000003814697265625) + 0.03125) + ((abs(in.i_local.x) + abs(in.i_local.y)) * 3.814697265625e-06);
                                float2 _8162 = fast::max(fast::max(gFxData_1._data[_8122].xy - in.i_local, in.i_local - gFxData_1._data[_8122].zw), float2(0.0));
                                if (dot(_8162, _8162) > (_8142 * _8142))
                                {
                                    _43014 = _36311;
                                    _43013 = _36310;
                                    continue;
                                }
                                float4 _8030 = select(gFxData_1._data[_8115], gFxData_1._data[_8108], bool4(_36309 < 2u));
                                uint _8044 = min(_8004, ((_36309 * 8u) + 8u));
                                uint _8046 = _36309 * 8u;
                                float2 _36317;
                                float _36318;
                                uint _36319;
                                _36319 = _36311;
                                _36318 = _36310;
                                _36317 = select(_8030.xy, _8030.zw, bool2((_36309 & 1u) != 0u));
                                float2 _43025;
                                float _43026;
                                uint _43027;
                                for (uint _36316 = _8046; _36316 < _8044; _36319 = _43027, _36318 = _43026, _36317 = _43025, _36316 += 2u)
                                {
                                    uint _8170 = _36307 + (_36316 >> 1u);
                                    uint _8059 = _8000 + _36316;
                                    float2 _8180 = _36317 - gFxData_1._data[_8170].xy;
                                    float2 _8183 = in.i_local - gFxData_1._data[_8170].xy;
                                    float2 _8196 = _8183 - (_8180 * fast::clamp(dot(_8183, _8180) / fast::max(dot(_8180, _8180), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _8199 = dot(_8196, _8196);
                                    bool _8210 = (_8199 < _36318) || ((_8199 == _36318) && (_8059 < _36319));
                                    uint _43872 = _8210 ? _8059 : _36319;
                                    float _43873 = _8210 ? _8199 : _36318;
                                    if ((_36316 + 1u) < _8044)
                                    {
                                        uint _8081 = (_8000 + _36316) + 1u;
                                        float2 _8223 = gFxData_1._data[_8170].xy - gFxData_1._data[_8170].zw;
                                        float2 _8226 = in.i_local - gFxData_1._data[_8170].zw;
                                        float2 _8239 = _8226 - (_8223 * fast::clamp(dot(_8226, _8223) / fast::max(dot(_8223, _8223), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _8242 = dot(_8239, _8239);
                                        bool _8253 = (_8242 < _43873) || ((_8242 == _43873) && (_8081 < _43872));
                                        _43027 = _8253 ? _8081 : _43872;
                                        _43026 = _8253 ? _8242 : _43873;
                                        _43025 = gFxData_1._data[_8170].zw;
                                    }
                                    else
                                    {
                                        _43027 = _43872;
                                        _43026 = _43873;
                                        _43025 = gFxData_1._data[_8170].xy;
                                    }
                                }
                                _43014 = _36319;
                                _43013 = _36318;
                            }
                            _43012 = _36311;
                            _42986 = _36310;
                        }
                        else
                        {
                            _43012 = _36308;
                            _42986 = _36296;
                        }
                        _8262 = _36307 + 24u;
                    }
                    _36299 = 1.0;
                    _36297 = _36296;
                }
                else
                {
                    float _7339 = floor((in.i_radii.z - 1.0) * 0.5);
                    float _8270 = floor(_7339 * 0.0625);
                    uint _8287 = (((in.i_instance + 1u) + uint(_8270)) * 24u) + uint(_7339 - (_8270 * 16.0));
                    float2 _36267;
                    if (_7317)
                    {
                        float _8297 = floor(0.0);
                        _36267 = gFxData_1._data[(((in.i_instance + 1u) + uint(_8297)) * 24u) + uint(_8297 * (-16.0))].xy;
                    }
                    else
                    {
                        _36267 = select(gFxData_1._data[_8287].zw, gFxData_1._data[_8287].xy, bool2(((in.i_radii.z - 1.0) - (_7339 * 2.0)) < 0.5));
                    }
                    float2 _36266;
                    float _36269;
                    float _36272;
                    _36272 = 1.0;
                    _36269 = 1000000015047466219876688855040.0;
                    _36266 = _36267;
                    float2 _36265;
                    float _36268;
                    float _36271;
                    for (float _36262 = 0.0; _36262 < 16.0; _36272 = _36271, _36269 = _36268, _36266 = _36265, _36262 += 1.0)
                    {
                        if (_36262 >= _7325)
                        {
                            break;
                        }
                        _36271 = _36272;
                        _36268 = _36269;
                        _36265 = _36266;
                        float2 _42976;
                        float _42978;
                        float _42980;
                        for (float _36263 = 0.0; _36263 < 128.0; _36271 = _42980, _36268 = _42978, _36265 = _42976, _36263 += 1.0)
                        {
                            float _7381 = (_36262 * 128.0) + _36263;
                            if (_7381 >= _7321)
                            {
                                break;
                            }
                            float _8324 = floor(_7381 * 0.0625);
                            uint _8341 = (((in.i_instance + 1u) + uint(_8324)) * 24u) + uint(_7381 - (_8324 * 16.0));
                            float2 _8354 = _36265 - gFxData_1._data[_8341].xy;
                            float2 _8357 = in.i_local - gFxData_1._data[_8341].xy;
                            float2 _8370 = _8357 - (_8354 * fast::clamp(dot(_8357, _8354) / fast::max(dot(_8354, _8354), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                            float _8373 = dot(_8370, _8370);
                            float _43876 = (_8373 < _36268) ? _8373 : _36268;
                            bool _8385 = in.i_local.y >= gFxData_1._data[_8341].y;
                            bool _8390 = in.i_local.y < _36265.y;
                            bool _8401 = (_8354.x * _8357.y) > (_8354.y * _8357.x);
                            float _36277;
                            if (((_8385 && _8390) && _8401) || (((!_8385) && (!_8390)) && (!_8401)))
                            {
                                _36277 = -_36271;
                            }
                            else
                            {
                                _36277 = _36271;
                            }
                            if (((2.0 * _7381) + 1.0) < in.i_radii.z)
                            {
                                float2 _8430 = gFxData_1._data[_8341].xy - gFxData_1._data[_8341].zw;
                                float2 _8433 = in.i_local - gFxData_1._data[_8341].zw;
                                float2 _8446 = _8433 - (_8430 * fast::clamp(dot(_8433, _8430) / fast::max(dot(_8430, _8430), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _8449 = dot(_8446, _8446);
                                bool _8461 = in.i_local.y >= gFxData_1._data[_8341].w;
                                bool _8466 = in.i_local.y < gFxData_1._data[_8341].y;
                                bool _8477 = (_8430.x * _8433.y) > (_8430.y * _8433.x);
                                float _36281;
                                if (((_8461 && _8466) && _8477) || (((!_8461) && (!_8466)) && (!_8477)))
                                {
                                    _36281 = -_36277;
                                }
                                else
                                {
                                    _36281 = _36277;
                                }
                                _42980 = _36281;
                                _42978 = (_8449 < _43876) ? _8449 : _43876;
                                _42976 = gFxData_1._data[_8341].zw;
                            }
                            else
                            {
                                _42980 = _36277;
                                _42978 = _43876;
                                _42976 = gFxData_1._data[_8341].xy;
                            }
                        }
                    }
                    _36299 = _36272;
                    _36297 = _36269;
                }
                float _7434 = sqrt(_36297);
                if (_7317)
                {
                    _36304 = _7434 - in.i_radii.x;
                    break;
                }
                _36304 = (_36299 * _7434) - in.i_radii.x;
                break;
            } while(false);
            _36353 = _36304;
        }
        else
        {
            float _36354;
            if (in.i_flags.z == 1u)
            {
                float2 _7060 = in.i_local - _7026;
                float _36261;
                do
                {
                    if (in.i_shape.w >= 6.282185077667236328125)
                    {
                        _36261 = abs(length(_7060) - in.i_radii.x) - in.i_radii.y;
                        break;
                    }
                    float _8520 = in.i_shape.z + (in.i_shape.w * 0.5);
                    float _8522 = cos(_8520);
                    float _8524 = sin(_8520);
                    float _8533 = dot(_7060, float2(-_8524, _8522));
                    float _8536 = dot(_7060, float2(_8522, _8524));
                    float2 _8537 = float2(_8533, _8536);
                    float _8540 = abs(_8533);
                    _8537.x = _8540;
                    float _8543 = in.i_shape.w * 0.5;
                    float _8545 = sin(_8543);
                    float _8547 = cos(_8543);
                    _36261 = (((_8547 * _8540) > (_8545 * _8536)) ? length(_8537 - (float2(_8545, _8547) * in.i_radii.x)) : abs(length(_8537) - in.i_radii.x)) - in.i_radii.y;
                    break;
                } while(false);
                _36354 = _36261;
            }
            else
            {
                float _36355;
                if (in.i_flags.z == 2u)
                {
                    float2 _8583 = in.i_local - _36247.xy;
                    float2 _8586 = _36247.zw - _36247.xy;
                    _36355 = length(_8583 - (_8586 * fast::clamp(dot(_8583, _8586) / fast::max(dot(_8586, _8586), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                }
                else
                {
                    float2 _7087 = in.i_local - _7026;
                    float _8651 = fast::min(_7035.x, _7035.y);
                    float _8654 = fast::min((_7087.x > 0.0) ? ((_7087.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_7087.y > 0.0) ? in.i_radii.w : in.i_radii.x), _8651);
                    float _8660 = _8654 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _36250;
                    float _36251;
                    if (_8660 > _8651)
                    {
                        float _8674 = in.i_shape.y * fast::clamp((_8651 - _8654) / fast::max(0.60000002384185791015625 * _8654, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _36251 = _8674;
                        _36250 = _8654 * (1.0 + (0.60000002384185791015625 * _8674));
                    }
                    else
                    {
                        _36251 = in.i_shape.y;
                        _36250 = _8660;
                    }
                    float2 _8703 = (abs(_7087) - _7035) + float2(_36250);
                    float2 _8705 = fast::max(_8703, float2(0.0));
                    float _36252;
                    if ((_8705.x > 0.0) && (_8705.y > 0.0))
                    {
                        float _36253;
                        if ((_36251 > 0.001000000047497451305389404296875) && (_36250 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8722 = 2.0 + (2.0 * _36251);
                            float2 _8727 = _8705 / float2(fast::max(_36250, 9.9999997473787516355514526367188e-05));
                            _36253 = pow(pow(_8727.x, _8722) + pow(_8727.y, _8722), 1.0 / _8722) * _36250;
                        }
                        else
                        {
                            _36253 = length(_8705);
                        }
                        _36252 = _36253;
                    }
                    else
                    {
                        _36252 = fast::max(_8705.x, _8705.y);
                    }
                    float _8762 = (fast::min(fast::max(_8703.x, _8703.y), 0.0) + _36252) - _36250;
                    float _36356;
                    if ((in.i_flags.x & _2278) != 0u)
                    {
                        float2 _7115 = fast::max((_36247.zw - _36247.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float2 _7118 = in.i_local - ((_36247.xy + _36247.zw) * 0.5);
                        float _8810 = fast::min(_7115.x, _7115.y);
                        float _8813 = fast::min((_7118.x > 0.0) ? ((_7118.y > 0.0) ? _36245.x : _36245.x) : ((_7118.y > 0.0) ? _36245.x : _36245.x), _8810);
                        float _8819 = _8813 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _36254;
                        float _36255;
                        if (_8819 > _8810)
                        {
                            float _8833 = in.i_shape.y * fast::clamp((_8810 - _8813) / fast::max(0.60000002384185791015625 * _8813, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _36255 = _8833;
                            _36254 = _8813 * (1.0 + (0.60000002384185791015625 * _8833));
                        }
                        else
                        {
                            _36255 = in.i_shape.y;
                            _36254 = _8819;
                        }
                        float2 _8862 = (abs(_7118) - _7115) + float2(_36254);
                        float2 _8864 = fast::max(_8862, float2(0.0));
                        float _36256;
                        if ((_8864.x > 0.0) && (_8864.y > 0.0))
                        {
                            float _36257;
                            if ((_36255 > 0.001000000047497451305389404296875) && (_36254 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8881 = 2.0 + (2.0 * _36255);
                                float2 _8886 = _8864 / float2(fast::max(_36254, 9.9999997473787516355514526367188e-05));
                                _36257 = pow(pow(_8886.x, _8881) + pow(_8886.y, _8881), 1.0 / _8881) * _36254;
                            }
                            else
                            {
                                _36257 = length(_8864);
                            }
                            _36256 = _36257;
                        }
                        else
                        {
                            _36256 = fast::max(_8864.x, _8864.y);
                        }
                        float _8921 = (fast::min(fast::max(_8862.x, _8862.y), 0.0) + _36256) - _36254;
                        float _8926 = fast::max(_36245.y, 9.9999997473787516355514526367188e-05);
                        float _8935 = fast::max(_8926 - abs(_8762 - _8921), 0.0) / _8926;
                        _36356 = fast::min(_8762, _8921) - (((_8935 * _8935) * _8926) * 0.25);
                    }
                    else
                    {
                        _36356 = _8762;
                    }
                    _36355 = _36356;
                }
                _36354 = _36355;
            }
            _36353 = _36354;
        }
        _36352 = _36353;
    }
    float _36985;
    float _42052;
    if ((in.i_flags.x & _3595) != 0u)
    {
        uint _8959 = (in.i_instance * 24u) + 6u;
        float _5605 = fast::clamp(ceil(length(gFxData_1._data[(in.i_instance * 24u) + 21u].xy) / (4.0 * _5545)), 1.0, 8.0);
        float _5610 = gFxData_1._data[_8959].x * gFxData_1._data[_8959].y;
        float _5616 = gFxData_1._data[_8959].x * (1.0 - gFxData_1._data[_8959].y);
        float2 _5620 = in.i_local + (gFxData_1._data[(in.i_instance * 24u) + 21u].xy * 0.5);
        float2 _9006 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _9015 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _36579;
        if ((in.i_flags.x & _2192) != 0u)
        {
            float _9132 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
            float4 _9166 = gTex.sample(gLinear, mix(_36241.xy, _36241.zw, (fast::clamp(_5620, _36247.xy, _36247.zw) - _36247.xy) / fast::max(_36247.zw - _36247.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
            float _9190 = (_9166.w - 0.5) * in.i_radii.y;
            float2 _9231 = abs(_5620 - ((_36247.xy + _36247.zw) * 0.5)) - fast::max(((_36247.zw - _36247.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _9132), float2(0.0));
            _36579 = fast::max((-mix((fast::max(fast::min(_9166.x, _9166.y), fast::min(fast::max(_9166.x, _9166.y), _9166.z)) - 0.5) * in.i_radii.x, _9190, fast::clamp(abs(_9190) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _9132, length(fast::max(_9231, float2(0.0))) + fast::min(fast::max(_9231.x, _9231.y), 0.0)) - in.i_shape.w;
        }
        else
        {
            float _36580;
            if ((in.i_flags.x & _2207) != 0u)
            {
                float _36531;
                do
                {
                    bool _9297 = in.i_radii.w > 0.5;
                    float _9301 = floor((in.i_radii.z + 1.0) * 0.5);
                    float _9305 = floor((_9301 + 127.0) * 0.0078125);
                    float _36524;
                    float _36526;
                    if (_9297)
                    {
                        uint _9481 = uint(in.i_radii.z);
                        uint _9484 = (_9481 + 31u) >> 5u;
                        uint _9567 = (in.i_instance + 1u) * 24u;
                        uint _36516;
                        uint _36517;
                        _36517 = 0u;
                        _36516 = _9567;
                        uint _9594;
                        float _43878;
                        uint _43879;
                        uint _43880;
                        uint _36515 = 0u;
                        uint _36575 = _9567;
                        float _36576 = 3.0000000054977557577780399428115e+38;
                        for (; _36515 < _9484; _36576 = _43878, _36575 = _9594, _36517 = _43879, _36516 = _43880, _36515++)
                        {
                            uint _9572 = _36575 + 20u;
                            float2 _9587 = fast::max(fast::max(gFxData_1._data[_9572].xy - _5620, _5620 - gFxData_1._data[_9572].zw), float2(0.0));
                            float _9590 = dot(_9587, _9587);
                            bool _9502 = _9590 < _36576;
                            _43878 = _9502 ? _9590 : _36576;
                            _43879 = _9502 ? _36515 : _36517;
                            _43880 = _9502 ? _36575 : _36516;
                            _9594 = _36575 + 24u;
                        }
                        uint _9632 = _36517 * 32u;
                        uint _9636 = min((_9481 - _9632), 32u);
                        uint _9740 = _36516 + 21u;
                        uint _9747 = _36516 + 22u;
                        float _36520;
                        uint _36521;
                        _36521 = 4294967295u;
                        _36520 = 1000000015047466219876688855040.0;
                        float _43046;
                        uint _43047;
                        for (uint _36519 = 0u; (_36519 * 8u) < _9636; _36521 = _43047, _36520 = _43046, _36519++)
                        {
                            uint _9754 = _36516 + (16u + _36519);
                            float _9774 = ((sqrt(_36520) * 1.000003814697265625) + 0.03125) + ((abs(_5620.x) + abs(_5620.y)) * 3.814697265625e-06);
                            float2 _9794 = fast::max(fast::max(gFxData_1._data[_9754].xy - _5620, _5620 - gFxData_1._data[_9754].zw), float2(0.0));
                            if (dot(_9794, _9794) > (_9774 * _9774))
                            {
                                _43047 = _36521;
                                _43046 = _36520;
                                continue;
                            }
                            float4 _9662 = select(gFxData_1._data[_9747], gFxData_1._data[_9740], bool4(_36519 < 2u));
                            uint _9676 = min(_9636, ((_36519 * 8u) + 8u));
                            uint _9678 = _36519 * 8u;
                            float2 _36560;
                            float _36561;
                            uint _36562;
                            _36562 = _36521;
                            _36561 = _36520;
                            _36560 = select(_9662.xy, _9662.zw, bool2((_36519 & 1u) != 0u));
                            float2 _43090;
                            float _43091;
                            uint _43092;
                            for (uint _36559 = _9678; _36559 < _9676; _36562 = _43092, _36561 = _43091, _36560 = _43090, _36559 += 2u)
                            {
                                uint _9802 = _36516 + (_36559 >> 1u);
                                uint _9691 = _9632 + _36559;
                                float2 _9812 = _36560 - gFxData_1._data[_9802].xy;
                                float2 _9815 = _5620 - gFxData_1._data[_9802].xy;
                                float2 _9828 = _9815 - (_9812 * fast::clamp(dot(_9815, _9812) / fast::max(dot(_9812, _9812), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _9831 = dot(_9828, _9828);
                                bool _9842 = (_9831 < _36561) || ((_9831 == _36561) && (_9691 < _36562));
                                uint _43881 = _9842 ? _9691 : _36562;
                                float _43882 = _9842 ? _9831 : _36561;
                                if ((_36559 + 1u) < _9676)
                                {
                                    uint _9713 = (_9632 + _36559) + 1u;
                                    float2 _9855 = gFxData_1._data[_9802].xy - gFxData_1._data[_9802].zw;
                                    float2 _9858 = _5620 - gFxData_1._data[_9802].zw;
                                    float2 _9871 = _9858 - (_9855 * fast::clamp(dot(_9858, _9855) / fast::max(dot(_9855, _9855), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _9874 = dot(_9871, _9871);
                                    bool _9885 = (_9874 < _43882) || ((_9874 == _43882) && (_9713 < _43881));
                                    _43092 = _9885 ? _9713 : _43881;
                                    _43091 = _9885 ? _9874 : _43882;
                                    _43090 = gFxData_1._data[_9802].zw;
                                }
                                else
                                {
                                    _43092 = _43881;
                                    _43091 = _43882;
                                    _43090 = gFxData_1._data[_9802].xy;
                                }
                            }
                            _43047 = _36562;
                            _43046 = _36561;
                        }
                        uint _9894 = (in.i_instance + 1u) * 24u;
                        float _36523;
                        _36523 = _36520;
                        uint _10242;
                        float _43048;
                        uint _43074;
                        for (uint _36522 = 0u, _36534 = _9894, _36535 = _36521; _36522 < _9484; _36535 = _43074, _36534 = _10242, _36523 = _43048, _36522++)
                        {
                            uint _9899 = _36534 + 20u;
                            float _9919 = ((sqrt(_36523) * 1.000003814697265625) + 0.03125) + ((abs(_5620.x) + abs(_5620.y)) * 3.814697265625e-06);
                            float2 _9939 = fast::max(fast::max(gFxData_1._data[_9899].xy - _5620, _5620 - gFxData_1._data[_9899].zw), float2(0.0));
                            if ((_36522 != _36517) && (!(dot(_9939, _9939) > (_9919 * _9919))))
                            {
                                uint _9980 = _36522 * 32u;
                                uint _9984 = min((_9481 - _9980), 32u);
                                uint _10088 = _36534 + 21u;
                                uint _10095 = _36534 + 22u;
                                float _36537;
                                uint _36538;
                                _36538 = _36535;
                                _36537 = _36523;
                                float _43075;
                                uint _43076;
                                for (uint _36536 = 0u; (_36536 * 8u) < _9984; _36538 = _43076, _36537 = _43075, _36536++)
                                {
                                    uint _10102 = _36534 + (16u + _36536);
                                    float _10122 = ((sqrt(_36537) * 1.000003814697265625) + 0.03125) + ((abs(_5620.x) + abs(_5620.y)) * 3.814697265625e-06);
                                    float2 _10142 = fast::max(fast::max(gFxData_1._data[_10102].xy - _5620, _5620 - gFxData_1._data[_10102].zw), float2(0.0));
                                    if (dot(_10142, _10142) > (_10122 * _10122))
                                    {
                                        _43076 = _36538;
                                        _43075 = _36537;
                                        continue;
                                    }
                                    float4 _10010 = select(gFxData_1._data[_10095], gFxData_1._data[_10088], bool4(_36536 < 2u));
                                    uint _10024 = min(_9984, ((_36536 * 8u) + 8u));
                                    uint _10026 = _36536 * 8u;
                                    float2 _36544;
                                    float _36545;
                                    uint _36546;
                                    _36546 = _36538;
                                    _36545 = _36537;
                                    _36544 = select(_10010.xy, _10010.zw, bool2((_36536 & 1u) != 0u));
                                    float2 _43087;
                                    float _43088;
                                    uint _43089;
                                    for (uint _36543 = _10026; _36543 < _10024; _36546 = _43089, _36545 = _43088, _36544 = _43087, _36543 += 2u)
                                    {
                                        uint _10150 = _36534 + (_36543 >> 1u);
                                        uint _10039 = _9980 + _36543;
                                        float2 _10160 = _36544 - gFxData_1._data[_10150].xy;
                                        float2 _10163 = _5620 - gFxData_1._data[_10150].xy;
                                        float2 _10176 = _10163 - (_10160 * fast::clamp(dot(_10163, _10160) / fast::max(dot(_10160, _10160), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _10179 = dot(_10176, _10176);
                                        bool _10190 = (_10179 < _36545) || ((_10179 == _36545) && (_10039 < _36546));
                                        uint _43885 = _10190 ? _10039 : _36546;
                                        float _43886 = _10190 ? _10179 : _36545;
                                        if ((_36543 + 1u) < _10024)
                                        {
                                            uint _10061 = (_9980 + _36543) + 1u;
                                            float2 _10203 = gFxData_1._data[_10150].xy - gFxData_1._data[_10150].zw;
                                            float2 _10206 = _5620 - gFxData_1._data[_10150].zw;
                                            float2 _10219 = _10206 - (_10203 * fast::clamp(dot(_10206, _10203) / fast::max(dot(_10203, _10203), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _10222 = dot(_10219, _10219);
                                            bool _10233 = (_10222 < _43886) || ((_10222 == _43886) && (_10061 < _43885));
                                            _43089 = _10233 ? _10061 : _43885;
                                            _43088 = _10233 ? _10222 : _43886;
                                            _43087 = gFxData_1._data[_10150].zw;
                                        }
                                        else
                                        {
                                            _43089 = _43885;
                                            _43088 = _43886;
                                            _43087 = gFxData_1._data[_10150].xy;
                                        }
                                    }
                                    _43076 = _36546;
                                    _43075 = _36545;
                                }
                                _43074 = _36538;
                                _43048 = _36537;
                            }
                            else
                            {
                                _43074 = _36535;
                                _43048 = _36523;
                            }
                            _10242 = _36534 + 24u;
                        }
                        _36526 = 1.0;
                        _36524 = _36523;
                    }
                    else
                    {
                        float _9319 = floor((in.i_radii.z - 1.0) * 0.5);
                        float _10250 = floor(_9319 * 0.0625);
                        uint _10267 = (((in.i_instance + 1u) + uint(_10250)) * 24u) + uint(_9319 - (_10250 * 16.0));
                        float2 _36494;
                        if (_9297)
                        {
                            float _10277 = floor(0.0);
                            _36494 = gFxData_1._data[(((in.i_instance + 1u) + uint(_10277)) * 24u) + uint(_10277 * (-16.0))].xy;
                        }
                        else
                        {
                            _36494 = select(gFxData_1._data[_10267].zw, gFxData_1._data[_10267].xy, bool2(((in.i_radii.z - 1.0) - (_9319 * 2.0)) < 0.5));
                        }
                        float2 _36493;
                        float _36496;
                        float _36499;
                        _36499 = 1.0;
                        _36496 = 1000000015047466219876688855040.0;
                        _36493 = _36494;
                        float2 _36492;
                        float _36495;
                        float _36498;
                        for (float _36489 = 0.0; _36489 < 16.0; _36499 = _36498, _36496 = _36495, _36493 = _36492, _36489 += 1.0)
                        {
                            if (_36489 >= _9305)
                            {
                                break;
                            }
                            _36498 = _36499;
                            _36495 = _36496;
                            _36492 = _36493;
                            float2 _43038;
                            float _43040;
                            float _43042;
                            for (float _36490 = 0.0; _36490 < 128.0; _36498 = _43042, _36495 = _43040, _36492 = _43038, _36490 += 1.0)
                            {
                                float _9361 = (_36489 * 128.0) + _36490;
                                if (_9361 >= _9301)
                                {
                                    break;
                                }
                                float _10304 = floor(_9361 * 0.0625);
                                uint _10321 = (((in.i_instance + 1u) + uint(_10304)) * 24u) + uint(_9361 - (_10304 * 16.0));
                                float2 _10334 = _36492 - gFxData_1._data[_10321].xy;
                                float2 _10337 = _5620 - gFxData_1._data[_10321].xy;
                                float2 _10350 = _10337 - (_10334 * fast::clamp(dot(_10337, _10334) / fast::max(dot(_10334, _10334), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _10353 = dot(_10350, _10350);
                                float _43889 = (_10353 < _36495) ? _10353 : _36495;
                                bool _10365 = _5620.y >= gFxData_1._data[_10321].y;
                                bool _10370 = _5620.y < _36492.y;
                                bool _10381 = (_10334.x * _10337.y) > (_10334.y * _10337.x);
                                float _36504;
                                if (((_10365 && _10370) && _10381) || (((!_10365) && (!_10370)) && (!_10381)))
                                {
                                    _36504 = -_36498;
                                }
                                else
                                {
                                    _36504 = _36498;
                                }
                                if (((2.0 * _9361) + 1.0) < in.i_radii.z)
                                {
                                    float2 _10410 = gFxData_1._data[_10321].xy - gFxData_1._data[_10321].zw;
                                    float2 _10413 = _5620 - gFxData_1._data[_10321].zw;
                                    float2 _10426 = _10413 - (_10410 * fast::clamp(dot(_10413, _10410) / fast::max(dot(_10410, _10410), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _10429 = dot(_10426, _10426);
                                    bool _10441 = _5620.y >= gFxData_1._data[_10321].w;
                                    bool _10446 = _5620.y < gFxData_1._data[_10321].y;
                                    bool _10457 = (_10410.x * _10413.y) > (_10410.y * _10413.x);
                                    float _36508;
                                    if (((_10441 && _10446) && _10457) || (((!_10441) && (!_10446)) && (!_10457)))
                                    {
                                        _36508 = -_36504;
                                    }
                                    else
                                    {
                                        _36508 = _36504;
                                    }
                                    _43042 = _36508;
                                    _43040 = (_10429 < _43889) ? _10429 : _43889;
                                    _43038 = gFxData_1._data[_10321].zw;
                                }
                                else
                                {
                                    _43042 = _36504;
                                    _43040 = _43889;
                                    _43038 = gFxData_1._data[_10321].xy;
                                }
                            }
                        }
                        _36526 = _36499;
                        _36524 = _36496;
                    }
                    float _9414 = sqrt(_36524);
                    if (_9297)
                    {
                        _36531 = _9414 - in.i_radii.x;
                        break;
                    }
                    _36531 = (_36526 * _9414) - in.i_radii.x;
                    break;
                } while(false);
                _36580 = _36531;
            }
            else
            {
                float _36581;
                if (in.i_flags.z == 1u)
                {
                    float2 _9040 = _5620 - _9006;
                    float _36488;
                    do
                    {
                        if (in.i_shape.w >= 6.282185077667236328125)
                        {
                            _36488 = abs(length(_9040) - in.i_radii.x) - in.i_radii.y;
                            break;
                        }
                        float _10500 = in.i_shape.z + (in.i_shape.w * 0.5);
                        float _10502 = cos(_10500);
                        float _10504 = sin(_10500);
                        float _10513 = dot(_9040, float2(-_10504, _10502));
                        float _10516 = dot(_9040, float2(_10502, _10504));
                        float2 _10517 = float2(_10513, _10516);
                        float _10520 = abs(_10513);
                        _10517.x = _10520;
                        float _10523 = in.i_shape.w * 0.5;
                        float _10525 = sin(_10523);
                        float _10527 = cos(_10523);
                        _36488 = (((_10527 * _10520) > (_10525 * _10516)) ? length(_10517 - (float2(_10525, _10527) * in.i_radii.x)) : abs(length(_10517) - in.i_radii.x)) - in.i_radii.y;
                        break;
                    } while(false);
                    _36581 = _36488;
                }
                else
                {
                    float _36582;
                    if (in.i_flags.z == 2u)
                    {
                        float2 _10563 = _5620 - _36247.xy;
                        float2 _10566 = _36247.zw - _36247.xy;
                        _36582 = length(_10563 - (_10566 * fast::clamp(dot(_10563, _10566) / fast::max(dot(_10566, _10566), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                    }
                    else
                    {
                        float2 _9067 = _5620 - _9006;
                        float _10631 = fast::min(_9015.x, _9015.y);
                        float _10634 = fast::min((_9067.x > 0.0) ? ((_9067.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_9067.y > 0.0) ? in.i_radii.w : in.i_radii.x), _10631);
                        float _10640 = _10634 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _36477;
                        float _36478;
                        if (_10640 > _10631)
                        {
                            float _10654 = in.i_shape.y * fast::clamp((_10631 - _10634) / fast::max(0.60000002384185791015625 * _10634, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _36478 = _10654;
                            _36477 = _10634 * (1.0 + (0.60000002384185791015625 * _10654));
                        }
                        else
                        {
                            _36478 = in.i_shape.y;
                            _36477 = _10640;
                        }
                        float2 _10683 = (abs(_9067) - _9015) + float2(_36477);
                        float2 _10685 = fast::max(_10683, float2(0.0));
                        float _36479;
                        if ((_10685.x > 0.0) && (_10685.y > 0.0))
                        {
                            float _36480;
                            if ((_36478 > 0.001000000047497451305389404296875) && (_36477 > 9.9999997473787516355514526367188e-05))
                            {
                                float _10702 = 2.0 + (2.0 * _36478);
                                float2 _10707 = _10685 / float2(fast::max(_36477, 9.9999997473787516355514526367188e-05));
                                _36480 = pow(pow(_10707.x, _10702) + pow(_10707.y, _10702), 1.0 / _10702) * _36477;
                            }
                            else
                            {
                                _36480 = length(_10685);
                            }
                            _36479 = _36480;
                        }
                        else
                        {
                            _36479 = fast::max(_10685.x, _10685.y);
                        }
                        float _10742 = (fast::min(fast::max(_10683.x, _10683.y), 0.0) + _36479) - _36477;
                        float _36583;
                        if ((in.i_flags.x & _2278) != 0u)
                        {
                            float2 _9095 = fast::max((_36247.zw - _36247.xy) * 0.5, float2(0.001000000047497451305389404296875));
                            float2 _9098 = _5620 - ((_36247.xy + _36247.zw) * 0.5);
                            float _10790 = fast::min(_9095.x, _9095.y);
                            float _10793 = fast::min((_9098.x > 0.0) ? ((_9098.y > 0.0) ? _36245.x : _36245.x) : ((_9098.y > 0.0) ? _36245.x : _36245.x), _10790);
                            float _10799 = _10793 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _36481;
                            float _36482;
                            if (_10799 > _10790)
                            {
                                float _10813 = in.i_shape.y * fast::clamp((_10790 - _10793) / fast::max(0.60000002384185791015625 * _10793, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _36482 = _10813;
                                _36481 = _10793 * (1.0 + (0.60000002384185791015625 * _10813));
                            }
                            else
                            {
                                _36482 = in.i_shape.y;
                                _36481 = _10799;
                            }
                            float2 _10842 = (abs(_9098) - _9095) + float2(_36481);
                            float2 _10844 = fast::max(_10842, float2(0.0));
                            float _36483;
                            if ((_10844.x > 0.0) && (_10844.y > 0.0))
                            {
                                float _36484;
                                if ((_36482 > 0.001000000047497451305389404296875) && (_36481 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _10861 = 2.0 + (2.0 * _36482);
                                    float2 _10866 = _10844 / float2(fast::max(_36481, 9.9999997473787516355514526367188e-05));
                                    _36484 = pow(pow(_10866.x, _10861) + pow(_10866.y, _10861), 1.0 / _10861) * _36481;
                                }
                                else
                                {
                                    _36484 = length(_10844);
                                }
                                _36483 = _36484;
                            }
                            else
                            {
                                _36483 = fast::max(_10844.x, _10844.y);
                            }
                            float _10901 = (fast::min(fast::max(_10842.x, _10842.y), 0.0) + _36483) - _36481;
                            float _10906 = fast::max(_36245.y, 9.9999997473787516355514526367188e-05);
                            float _10915 = fast::max(_10906 - abs(_10742 - _10901), 0.0) / _10906;
                            _36583 = fast::min(_10742, _10901) - (((_10915 * _10915) * _10906) * 0.25);
                        }
                        else
                        {
                            _36583 = _10742;
                        }
                        _36582 = _36583;
                    }
                    _36581 = _36582;
                }
                _36580 = _36581;
            }
            _36579 = _36580;
        }
        float _36857;
        float _36898;
        _36898 = 0.0;
        _36857 = 0.0;
        float _5655;
        float _5684;
        float _36810;
        int _36584 = 1;
        float _36816 = _36579;
        for (; _36584 <= 8; _36898 = _5684, _36857 = _5655, _36816 = _36810, _36584++)
        {
            if (float(_36584) > _5605)
            {
                break;
            }
            float2 _5642 = in.i_local + (gFxData_1._data[(in.i_instance * 24u) + 21u].xy * (0.5 - (float(_36584) / _5605)));
            float2 _10970 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _10979 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            if ((in.i_flags.x & _2192) != 0u)
            {
                float _11096 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                float4 _11130 = gTex.sample(gLinear, mix(_36241.xy, _36241.zw, (fast::clamp(_5642, _36247.xy, _36247.zw) - _36247.xy) / fast::max(_36247.zw - _36247.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                float _11154 = (_11130.w - 0.5) * in.i_radii.y;
                float2 _11195 = abs(_5642 - ((_36247.xy + _36247.zw) * 0.5)) - fast::max(((_36247.zw - _36247.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _11096), float2(0.0));
                _36810 = fast::max((-mix((fast::max(fast::min(_11130.x, _11130.y), fast::min(fast::max(_11130.x, _11130.y), _11130.z)) - 0.5) * in.i_radii.x, _11154, fast::clamp(abs(_11154) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _11096, length(fast::max(_11195, float2(0.0))) + fast::min(fast::max(_11195.x, _11195.y), 0.0)) - in.i_shape.w;
            }
            else
            {
                float _36811;
                if ((in.i_flags.x & _2207) != 0u)
                {
                    float _36762;
                    do
                    {
                        bool _11261 = in.i_radii.w > 0.5;
                        float _11265 = floor((in.i_radii.z + 1.0) * 0.5);
                        float _11269 = floor((_11265 + 127.0) * 0.0078125);
                        float _36755;
                        float _36757;
                        if (_11261)
                        {
                            uint _11445 = uint(in.i_radii.z);
                            uint _11448 = (_11445 + 31u) >> 5u;
                            uint _11531 = (in.i_instance + 1u) * 24u;
                            uint _36747;
                            uint _36748;
                            _36748 = 0u;
                            _36747 = _11531;
                            uint _11558;
                            float _43891;
                            uint _43892;
                            uint _43893;
                            uint _36746 = 0u;
                            uint _36806 = _11531;
                            float _36807 = 3.0000000054977557577780399428115e+38;
                            for (; _36746 < _11448; _36807 = _43891, _36806 = _11558, _36748 = _43892, _36747 = _43893, _36746++)
                            {
                                uint _11536 = _36806 + 20u;
                                float2 _11551 = fast::max(fast::max(gFxData_1._data[_11536].xy - _5642, _5642 - gFxData_1._data[_11536].zw), float2(0.0));
                                float _11554 = dot(_11551, _11551);
                                bool _11466 = _11554 < _36807;
                                _43891 = _11466 ? _11554 : _36807;
                                _43892 = _11466 ? _36746 : _36748;
                                _43893 = _11466 ? _36806 : _36747;
                                _11558 = _36806 + 24u;
                            }
                            uint _11596 = _36748 * 32u;
                            uint _11600 = min((_11445 - _11596), 32u);
                            uint _11704 = _36747 + 21u;
                            uint _11711 = _36747 + 22u;
                            float _36751;
                            uint _36752;
                            _36752 = 4294967295u;
                            _36751 = 1000000015047466219876688855040.0;
                            float _43228;
                            uint _43229;
                            for (uint _36750 = 0u; (_36750 * 8u) < _11600; _36752 = _43229, _36751 = _43228, _36750++)
                            {
                                uint _11718 = _36747 + (16u + _36750);
                                float _11738 = ((sqrt(_36751) * 1.000003814697265625) + 0.03125) + ((abs(_5642.x) + abs(_5642.y)) * 3.814697265625e-06);
                                float2 _11758 = fast::max(fast::max(gFxData_1._data[_11718].xy - _5642, _5642 - gFxData_1._data[_11718].zw), float2(0.0));
                                if (dot(_11758, _11758) > (_11738 * _11738))
                                {
                                    _43229 = _36752;
                                    _43228 = _36751;
                                    continue;
                                }
                                float4 _11626 = select(gFxData_1._data[_11711], gFxData_1._data[_11704], bool4(_36750 < 2u));
                                uint _11640 = min(_11600, ((_36750 * 8u) + 8u));
                                uint _11642 = _36750 * 8u;
                                float2 _36791;
                                float _36792;
                                uint _36793;
                                _36793 = _36752;
                                _36792 = _36751;
                                _36791 = select(_11626.xy, _11626.zw, bool2((_36750 & 1u) != 0u));
                                float2 _43272;
                                float _43273;
                                uint _43274;
                                for (uint _36790 = _11642; _36790 < _11640; _36793 = _43274, _36792 = _43273, _36791 = _43272, _36790 += 2u)
                                {
                                    uint _11766 = _36747 + (_36790 >> 1u);
                                    uint _11655 = _11596 + _36790;
                                    float2 _11776 = _36791 - gFxData_1._data[_11766].xy;
                                    float2 _11779 = _5642 - gFxData_1._data[_11766].xy;
                                    float2 _11792 = _11779 - (_11776 * fast::clamp(dot(_11779, _11776) / fast::max(dot(_11776, _11776), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _11795 = dot(_11792, _11792);
                                    bool _11806 = (_11795 < _36792) || ((_11795 == _36792) && (_11655 < _36793));
                                    uint _43894 = _11806 ? _11655 : _36793;
                                    float _43895 = _11806 ? _11795 : _36792;
                                    if ((_36790 + 1u) < _11640)
                                    {
                                        uint _11677 = (_11596 + _36790) + 1u;
                                        float2 _11819 = gFxData_1._data[_11766].xy - gFxData_1._data[_11766].zw;
                                        float2 _11822 = _5642 - gFxData_1._data[_11766].zw;
                                        float2 _11835 = _11822 - (_11819 * fast::clamp(dot(_11822, _11819) / fast::max(dot(_11819, _11819), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _11838 = dot(_11835, _11835);
                                        bool _11849 = (_11838 < _43895) || ((_11838 == _43895) && (_11677 < _43894));
                                        _43274 = _11849 ? _11677 : _43894;
                                        _43273 = _11849 ? _11838 : _43895;
                                        _43272 = gFxData_1._data[_11766].zw;
                                    }
                                    else
                                    {
                                        _43274 = _43894;
                                        _43273 = _43895;
                                        _43272 = gFxData_1._data[_11766].xy;
                                    }
                                }
                                _43229 = _36793;
                                _43228 = _36792;
                            }
                            uint _11858 = (in.i_instance + 1u) * 24u;
                            float _36754;
                            _36754 = _36751;
                            uint _12206;
                            float _43230;
                            uint _43256;
                            for (uint _36753 = 0u, _36765 = _11858, _36766 = _36752; _36753 < _11448; _36766 = _43256, _36765 = _12206, _36754 = _43230, _36753++)
                            {
                                uint _11863 = _36765 + 20u;
                                float _11883 = ((sqrt(_36754) * 1.000003814697265625) + 0.03125) + ((abs(_5642.x) + abs(_5642.y)) * 3.814697265625e-06);
                                float2 _11903 = fast::max(fast::max(gFxData_1._data[_11863].xy - _5642, _5642 - gFxData_1._data[_11863].zw), float2(0.0));
                                if ((_36753 != _36748) && (!(dot(_11903, _11903) > (_11883 * _11883))))
                                {
                                    uint _11944 = _36753 * 32u;
                                    uint _11948 = min((_11445 - _11944), 32u);
                                    uint _12052 = _36765 + 21u;
                                    uint _12059 = _36765 + 22u;
                                    float _36768;
                                    uint _36769;
                                    _36769 = _36766;
                                    _36768 = _36754;
                                    float _43257;
                                    uint _43258;
                                    for (uint _36767 = 0u; (_36767 * 8u) < _11948; _36769 = _43258, _36768 = _43257, _36767++)
                                    {
                                        uint _12066 = _36765 + (16u + _36767);
                                        float _12086 = ((sqrt(_36768) * 1.000003814697265625) + 0.03125) + ((abs(_5642.x) + abs(_5642.y)) * 3.814697265625e-06);
                                        float2 _12106 = fast::max(fast::max(gFxData_1._data[_12066].xy - _5642, _5642 - gFxData_1._data[_12066].zw), float2(0.0));
                                        if (dot(_12106, _12106) > (_12086 * _12086))
                                        {
                                            _43258 = _36769;
                                            _43257 = _36768;
                                            continue;
                                        }
                                        float4 _11974 = select(gFxData_1._data[_12059], gFxData_1._data[_12052], bool4(_36767 < 2u));
                                        uint _11988 = min(_11948, ((_36767 * 8u) + 8u));
                                        uint _11990 = _36767 * 8u;
                                        float2 _36775;
                                        float _36776;
                                        uint _36777;
                                        _36777 = _36769;
                                        _36776 = _36768;
                                        _36775 = select(_11974.xy, _11974.zw, bool2((_36767 & 1u) != 0u));
                                        float2 _43269;
                                        float _43270;
                                        uint _43271;
                                        for (uint _36774 = _11990; _36774 < _11988; _36777 = _43271, _36776 = _43270, _36775 = _43269, _36774 += 2u)
                                        {
                                            uint _12114 = _36765 + (_36774 >> 1u);
                                            uint _12003 = _11944 + _36774;
                                            float2 _12124 = _36775 - gFxData_1._data[_12114].xy;
                                            float2 _12127 = _5642 - gFxData_1._data[_12114].xy;
                                            float2 _12140 = _12127 - (_12124 * fast::clamp(dot(_12127, _12124) / fast::max(dot(_12124, _12124), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _12143 = dot(_12140, _12140);
                                            bool _12154 = (_12143 < _36776) || ((_12143 == _36776) && (_12003 < _36777));
                                            uint _43898 = _12154 ? _12003 : _36777;
                                            float _43899 = _12154 ? _12143 : _36776;
                                            if ((_36774 + 1u) < _11988)
                                            {
                                                uint _12025 = (_11944 + _36774) + 1u;
                                                float2 _12167 = gFxData_1._data[_12114].xy - gFxData_1._data[_12114].zw;
                                                float2 _12170 = _5642 - gFxData_1._data[_12114].zw;
                                                float2 _12183 = _12170 - (_12167 * fast::clamp(dot(_12170, _12167) / fast::max(dot(_12167, _12167), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _12186 = dot(_12183, _12183);
                                                bool _12197 = (_12186 < _43899) || ((_12186 == _43899) && (_12025 < _43898));
                                                _43271 = _12197 ? _12025 : _43898;
                                                _43270 = _12197 ? _12186 : _43899;
                                                _43269 = gFxData_1._data[_12114].zw;
                                            }
                                            else
                                            {
                                                _43271 = _43898;
                                                _43270 = _43899;
                                                _43269 = gFxData_1._data[_12114].xy;
                                            }
                                        }
                                        _43258 = _36777;
                                        _43257 = _36776;
                                    }
                                    _43256 = _36769;
                                    _43230 = _36768;
                                }
                                else
                                {
                                    _43256 = _36766;
                                    _43230 = _36754;
                                }
                                _12206 = _36765 + 24u;
                            }
                            _36757 = 1.0;
                            _36755 = _36754;
                        }
                        else
                        {
                            float _11283 = floor((in.i_radii.z - 1.0) * 0.5);
                            float _12214 = floor(_11283 * 0.0625);
                            uint _12231 = (((in.i_instance + 1u) + uint(_12214)) * 24u) + uint(_11283 - (_12214 * 16.0));
                            float2 _36725;
                            if (_11261)
                            {
                                float _12241 = floor(0.0);
                                _36725 = gFxData_1._data[(((in.i_instance + 1u) + uint(_12241)) * 24u) + uint(_12241 * (-16.0))].xy;
                            }
                            else
                            {
                                _36725 = select(gFxData_1._data[_12231].zw, gFxData_1._data[_12231].xy, bool2(((in.i_radii.z - 1.0) - (_11283 * 2.0)) < 0.5));
                            }
                            float2 _36724;
                            float _36727;
                            float _36730;
                            _36730 = 1.0;
                            _36727 = 1000000015047466219876688855040.0;
                            _36724 = _36725;
                            float2 _36723;
                            float _36726;
                            float _36729;
                            for (float _36720 = 0.0; _36720 < 16.0; _36730 = _36729, _36727 = _36726, _36724 = _36723, _36720 += 1.0)
                            {
                                if (_36720 >= _11269)
                                {
                                    break;
                                }
                                _36729 = _36730;
                                _36726 = _36727;
                                _36723 = _36724;
                                float2 _43220;
                                float _43222;
                                float _43224;
                                for (float _36721 = 0.0; _36721 < 128.0; _36729 = _43224, _36726 = _43222, _36723 = _43220, _36721 += 1.0)
                                {
                                    float _11325 = (_36720 * 128.0) + _36721;
                                    if (_11325 >= _11265)
                                    {
                                        break;
                                    }
                                    float _12268 = floor(_11325 * 0.0625);
                                    uint _12285 = (((in.i_instance + 1u) + uint(_12268)) * 24u) + uint(_11325 - (_12268 * 16.0));
                                    float2 _12298 = _36723 - gFxData_1._data[_12285].xy;
                                    float2 _12301 = _5642 - gFxData_1._data[_12285].xy;
                                    float2 _12314 = _12301 - (_12298 * fast::clamp(dot(_12301, _12298) / fast::max(dot(_12298, _12298), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _12317 = dot(_12314, _12314);
                                    float _43902 = (_12317 < _36726) ? _12317 : _36726;
                                    bool _12329 = _5642.y >= gFxData_1._data[_12285].y;
                                    bool _12334 = _5642.y < _36723.y;
                                    bool _12345 = (_12298.x * _12301.y) > (_12298.y * _12301.x);
                                    float _36735;
                                    if (((_12329 && _12334) && _12345) || (((!_12329) && (!_12334)) && (!_12345)))
                                    {
                                        _36735 = -_36729;
                                    }
                                    else
                                    {
                                        _36735 = _36729;
                                    }
                                    if (((2.0 * _11325) + 1.0) < in.i_radii.z)
                                    {
                                        float2 _12374 = gFxData_1._data[_12285].xy - gFxData_1._data[_12285].zw;
                                        float2 _12377 = _5642 - gFxData_1._data[_12285].zw;
                                        float2 _12390 = _12377 - (_12374 * fast::clamp(dot(_12377, _12374) / fast::max(dot(_12374, _12374), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _12393 = dot(_12390, _12390);
                                        bool _12405 = _5642.y >= gFxData_1._data[_12285].w;
                                        bool _12410 = _5642.y < gFxData_1._data[_12285].y;
                                        bool _12421 = (_12374.x * _12377.y) > (_12374.y * _12377.x);
                                        float _36739;
                                        if (((_12405 && _12410) && _12421) || (((!_12405) && (!_12410)) && (!_12421)))
                                        {
                                            _36739 = -_36735;
                                        }
                                        else
                                        {
                                            _36739 = _36735;
                                        }
                                        _43224 = _36739;
                                        _43222 = (_12393 < _43902) ? _12393 : _43902;
                                        _43220 = gFxData_1._data[_12285].zw;
                                    }
                                    else
                                    {
                                        _43224 = _36735;
                                        _43222 = _43902;
                                        _43220 = gFxData_1._data[_12285].xy;
                                    }
                                }
                            }
                            _36757 = _36730;
                            _36755 = _36727;
                        }
                        float _11378 = sqrt(_36755);
                        if (_11261)
                        {
                            _36762 = _11378 - in.i_radii.x;
                            break;
                        }
                        _36762 = (_36757 * _11378) - in.i_radii.x;
                        break;
                    } while(false);
                    _36811 = _36762;
                }
                else
                {
                    float _36812;
                    if (in.i_flags.z == 1u)
                    {
                        float2 _11004 = _5642 - _10970;
                        float _36719;
                        do
                        {
                            if (in.i_shape.w >= 6.282185077667236328125)
                            {
                                _36719 = abs(length(_11004) - in.i_radii.x) - in.i_radii.y;
                                break;
                            }
                            float _12464 = in.i_shape.z + (in.i_shape.w * 0.5);
                            float _12466 = cos(_12464);
                            float _12468 = sin(_12464);
                            float _12477 = dot(_11004, float2(-_12468, _12466));
                            float _12480 = dot(_11004, float2(_12466, _12468));
                            float2 _12481 = float2(_12477, _12480);
                            float _12484 = abs(_12477);
                            _12481.x = _12484;
                            float _12487 = in.i_shape.w * 0.5;
                            float _12489 = sin(_12487);
                            float _12491 = cos(_12487);
                            _36719 = (((_12491 * _12484) > (_12489 * _12480)) ? length(_12481 - (float2(_12489, _12491) * in.i_radii.x)) : abs(length(_12481) - in.i_radii.x)) - in.i_radii.y;
                            break;
                        } while(false);
                        _36812 = _36719;
                    }
                    else
                    {
                        float _36813;
                        if (in.i_flags.z == 2u)
                        {
                            float2 _12527 = _5642 - _36247.xy;
                            float2 _12530 = _36247.zw - _36247.xy;
                            _36813 = length(_12527 - (_12530 * fast::clamp(dot(_12527, _12530) / fast::max(dot(_12530, _12530), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                        }
                        else
                        {
                            float2 _11031 = _5642 - _10970;
                            float _12595 = fast::min(_10979.x, _10979.y);
                            float _12598 = fast::min((_11031.x > 0.0) ? ((_11031.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_11031.y > 0.0) ? in.i_radii.w : in.i_radii.x), _12595);
                            float _12604 = _12598 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _36708;
                            float _36709;
                            if (_12604 > _12595)
                            {
                                float _12618 = in.i_shape.y * fast::clamp((_12595 - _12598) / fast::max(0.60000002384185791015625 * _12598, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _36709 = _12618;
                                _36708 = _12598 * (1.0 + (0.60000002384185791015625 * _12618));
                            }
                            else
                            {
                                _36709 = in.i_shape.y;
                                _36708 = _12604;
                            }
                            float2 _12647 = (abs(_11031) - _10979) + float2(_36708);
                            float2 _12649 = fast::max(_12647, float2(0.0));
                            float _36710;
                            if ((_12649.x > 0.0) && (_12649.y > 0.0))
                            {
                                float _36711;
                                if ((_36709 > 0.001000000047497451305389404296875) && (_36708 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _12666 = 2.0 + (2.0 * _36709);
                                    float2 _12671 = _12649 / float2(fast::max(_36708, 9.9999997473787516355514526367188e-05));
                                    _36711 = pow(pow(_12671.x, _12666) + pow(_12671.y, _12666), 1.0 / _12666) * _36708;
                                }
                                else
                                {
                                    _36711 = length(_12649);
                                }
                                _36710 = _36711;
                            }
                            else
                            {
                                _36710 = fast::max(_12649.x, _12649.y);
                            }
                            float _12706 = (fast::min(fast::max(_12647.x, _12647.y), 0.0) + _36710) - _36708;
                            float _36814;
                            if ((in.i_flags.x & _2278) != 0u)
                            {
                                float2 _11059 = fast::max((_36247.zw - _36247.xy) * 0.5, float2(0.001000000047497451305389404296875));
                                float2 _11062 = _5642 - ((_36247.xy + _36247.zw) * 0.5);
                                float _12754 = fast::min(_11059.x, _11059.y);
                                float _12757 = fast::min((_11062.x > 0.0) ? ((_11062.y > 0.0) ? _36245.x : _36245.x) : ((_11062.y > 0.0) ? _36245.x : _36245.x), _12754);
                                float _12763 = _12757 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                float _36712;
                                float _36713;
                                if (_12763 > _12754)
                                {
                                    float _12777 = in.i_shape.y * fast::clamp((_12754 - _12757) / fast::max(0.60000002384185791015625 * _12757, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                    _36713 = _12777;
                                    _36712 = _12757 * (1.0 + (0.60000002384185791015625 * _12777));
                                }
                                else
                                {
                                    _36713 = in.i_shape.y;
                                    _36712 = _12763;
                                }
                                float2 _12806 = (abs(_11062) - _11059) + float2(_36712);
                                float2 _12808 = fast::max(_12806, float2(0.0));
                                float _36714;
                                if ((_12808.x > 0.0) && (_12808.y > 0.0))
                                {
                                    float _36715;
                                    if ((_36713 > 0.001000000047497451305389404296875) && (_36712 > 9.9999997473787516355514526367188e-05))
                                    {
                                        float _12825 = 2.0 + (2.0 * _36713);
                                        float2 _12830 = _12808 / float2(fast::max(_36712, 9.9999997473787516355514526367188e-05));
                                        _36715 = pow(pow(_12830.x, _12825) + pow(_12830.y, _12825), 1.0 / _12825) * _36712;
                                    }
                                    else
                                    {
                                        _36715 = length(_12808);
                                    }
                                    _36714 = _36715;
                                }
                                else
                                {
                                    _36714 = fast::max(_12808.x, _12808.y);
                                }
                                float _12865 = (fast::min(fast::max(_12806.x, _12806.y), 0.0) + _36714) - _36712;
                                float _12870 = fast::max(_36245.y, 9.9999997473787516355514526367188e-05);
                                float _12879 = fast::max(_12870 - abs(_12706 - _12865), 0.0) / _12870;
                                _36814 = fast::min(_12706, _12865) - (((_12879 * _12879) * _12870) * 0.25);
                            }
                            else
                            {
                                _36814 = _12706;
                            }
                            _36813 = _36814;
                        }
                        _36812 = _36813;
                    }
                    _36811 = _36812;
                }
                _36810 = _36811;
            }
            float _5647 = _36816 / _5545;
            float _5648 = 0.5 - _5647;
            float _5651 = _36810 / _5545;
            float _5652 = 0.5 - _5651;
            float _12897 = _5652 - _5648;
            _5655 = _36857 + ((abs(_12897) < 0.001000000047497451305389404296875) ? fast::clamp(0.5 * (_5648 + _5652), 0.0, 1.0) : ((((_5652 <= 0.0) ? 0.0 : ((_5652 >= 1.0) ? (-_5651) : ((0.5 * _5652) * _5652))) - ((_5648 <= 0.0) ? 0.0 : ((_5648 >= 1.0) ? (-_5647) : ((0.5 * _5648) * _5648)))) / _12897));
            float _5660 = (_36816 - _5610) / _5545;
            float _5661 = 0.5 - _5660;
            float _5666 = (_36810 - _5610) / _5545;
            float _5667 = 0.5 - _5666;
            float _12949 = _5667 - _5661;
            float _5673 = (_36816 + _5616) / _5545;
            float _5674 = 0.5 - _5673;
            float _5679 = (_36810 + _5616) / _5545;
            float _5680 = 0.5 - _5679;
            float _13001 = _5680 - _5674;
            _5684 = _36898 + (((abs(_12949) < 0.001000000047497451305389404296875) ? fast::clamp(0.5 * (_5661 + _5667), 0.0, 1.0) : ((((_5667 <= 0.0) ? 0.0 : ((_5667 >= 1.0) ? (-_5666) : ((0.5 * _5667) * _5667))) - ((_5661 <= 0.0) ? 0.0 : ((_5661 >= 1.0) ? (-_5660) : ((0.5 * _5661) * _5661)))) / _12949)) - ((abs(_13001) < 0.001000000047497451305389404296875) ? fast::clamp(0.5 * (_5674 + _5680), 0.0, 1.0) : ((((_5680 <= 0.0) ? 0.0 : ((_5680 >= 1.0) ? (-_5679) : ((0.5 * _5680) * _5680))) - ((_5674 <= 0.0) ? 0.0 : ((_5674 >= 1.0) ? (-_5673) : ((0.5 * _5674) * _5674)))) / _13001)));
        }
        _42052 = ((in.i_flags.x & _3730) != 0u) ? (_36898 / _5605) : (-1.0);
        _36985 = _36857 / _5605;
    }
    else
    {
        _42052 = -1.0;
        _36985 = fast::clamp(0.5 - (_36352 / _5545), 0.0, 1.0);
    }
    float _42486;
    if ((in.i_flags.x & _3740) != 0u)
    {
        uint _13051 = (in.i_instance * 24u) + 19u;
        uint _13059 = (in.i_instance * 24u) + 20u;
        float2 _5729 = fast::max((gFxData_1._data[_13051].zw - gFxData_1._data[_13051].xy) * 0.5, float2(0.001000000047497451305389404296875));
        float2 _5732 = in.i_local - ((gFxData_1._data[_13051].xy + gFxData_1._data[_13051].zw) * 0.5);
        float _13109 = fast::min(_5729.x, _5729.y);
        float _13112 = fast::min((_5732.x > 0.0) ? ((_5732.y > 0.0) ? gFxData_1._data[_13059].x : gFxData_1._data[_13059].x) : ((_5732.y > 0.0) ? gFxData_1._data[_13059].x : gFxData_1._data[_13059].x), _13109);
        float _13118 = _13112 * (1.0 + (0.60000002384185791015625 * gFxData_1._data[_13059].y));
        float _36980;
        float _36981;
        if (_13118 > _13109)
        {
            float _13132 = gFxData_1._data[_13059].y * fast::clamp((_13109 - _13112) / fast::max(0.60000002384185791015625 * _13112, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _36981 = _13132;
            _36980 = _13112 * (1.0 + (0.60000002384185791015625 * _13132));
        }
        else
        {
            _36981 = gFxData_1._data[_13059].y;
            _36980 = _13118;
        }
        float2 _13161 = (abs(_5732) - _5729) + float2(_36980);
        float2 _13163 = fast::max(_13161, float2(0.0));
        float _36982;
        if ((_13163.x > 0.0) && (_13163.y > 0.0))
        {
            float _36983;
            if ((_36981 > 0.001000000047497451305389404296875) && (_36980 > 9.9999997473787516355514526367188e-05))
            {
                float _13180 = 2.0 + (2.0 * _36981);
                float2 _13185 = _13163 / float2(fast::max(_36980, 9.9999997473787516355514526367188e-05));
                _36983 = pow(pow(_13185.x, _13180) + pow(_13185.y, _13180), 1.0 / _13180) * _36980;
            }
            else
            {
                _36983 = length(_13163);
            }
            _36982 = _36983;
        }
        else
        {
            _36982 = fast::max(_13163.x, _13163.y);
        }
        float _5744 = fast::clamp(0.5 - (((fast::min(fast::max(_13161.x, _13161.y), 0.0) + _36982) - _36980) / _5545), 0.0, 1.0);
        if (_5744 <= 0.0)
        {
            discard_fragment();
        }
        _42486 = _5744;
    }
    else
    {
        _42486 = 1.0;
    }
    bool _5755 = ((in.i_flags.x & _3805) != 0u) && (_36985 >= 0.999000012874603271484375);
    float4 _37250;
    if ((((in.i_flags.x & _3813) != 0u) && (!((in.i_flags.x & _3818) != 0u))) && (!_5755))
    {
        uint _13226 = (in.i_instance * 24u) + 7u;
        uint _13234 = (in.i_instance * 24u) + 8u;
        float2 _36989;
        if (_330.gDrawInfo.w > 0.5)
        {
            float2 _13283 = select(_330.gDrawInfo.yz, _330.gDrawInfo.yz - ((in.i_rect.xy + in.i_rect.zw) * 0.5), bool2(_330.gDrawInfo.w > 1.0));
            _36989 = (-(_13283 / float2(fast::max(length(_13283), 9.9999997473787516355514526367188e-05)))) * length(gFxData_1._data[_13234].zw);
        }
        else
        {
            _36989 = gFxData_1._data[_13234].zw;
        }
        float2 _5785 = in.i_local - _36989;
        float2 _13334 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _13343 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _37113;
        if ((in.i_flags.x & _2192) != 0u)
        {
            float _13460 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
            float4 _13494 = gTex.sample(gLinear, mix(_36241.xy, _36241.zw, (fast::clamp(_5785, _36247.xy, _36247.zw) - _36247.xy) / fast::max(_36247.zw - _36247.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
            float _13518 = (_13494.w - 0.5) * in.i_radii.y;
            float2 _13559 = abs(_5785 - ((_36247.xy + _36247.zw) * 0.5)) - fast::max(((_36247.zw - _36247.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _13460), float2(0.0));
            _37113 = fast::max((-mix((fast::max(fast::min(_13494.x, _13494.y), fast::min(fast::max(_13494.x, _13494.y), _13494.z)) - 0.5) * in.i_radii.x, _13518, fast::clamp(abs(_13518) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _13460, length(fast::max(_13559, float2(0.0))) + fast::min(fast::max(_13559.x, _13559.y), 0.0)) - in.i_shape.w;
        }
        else
        {
            float _37114;
            if ((in.i_flags.x & _2207) != 0u)
            {
                float _37065;
                do
                {
                    bool _13625 = in.i_radii.w > 0.5;
                    float _13629 = floor((in.i_radii.z + 1.0) * 0.5);
                    float _13633 = floor((_13629 + 127.0) * 0.0078125);
                    float _37058;
                    float _37060;
                    if (_13625)
                    {
                        uint _13809 = uint(in.i_radii.z);
                        uint _13812 = (_13809 + 31u) >> 5u;
                        uint _13895 = (in.i_instance + 1u) * 24u;
                        uint _37050;
                        uint _37051;
                        _37051 = 0u;
                        _37050 = _13895;
                        uint _13922;
                        float _43904;
                        uint _43905;
                        uint _43906;
                        uint _37049 = 0u;
                        uint _37109 = _13895;
                        float _37110 = 3.0000000054977557577780399428115e+38;
                        for (; _37049 < _13812; _37110 = _43904, _37109 = _13922, _37051 = _43905, _37050 = _43906, _37049++)
                        {
                            uint _13900 = _37109 + 20u;
                            float2 _13915 = fast::max(fast::max(gFxData_1._data[_13900].xy - _5785, _5785 - gFxData_1._data[_13900].zw), float2(0.0));
                            float _13918 = dot(_13915, _13915);
                            bool _13830 = _13918 < _37110;
                            _43904 = _13830 ? _13918 : _37110;
                            _43905 = _13830 ? _37049 : _37051;
                            _43906 = _13830 ? _37109 : _37050;
                            _13922 = _37109 + 24u;
                        }
                        uint _13960 = _37051 * 32u;
                        uint _13964 = min((_13809 - _13960), 32u);
                        uint _14068 = _37050 + 21u;
                        uint _14075 = _37050 + 22u;
                        float _37054;
                        uint _37055;
                        _37055 = 4294967295u;
                        _37054 = 1000000015047466219876688855040.0;
                        float _43290;
                        uint _43291;
                        for (uint _37053 = 0u; (_37053 * 8u) < _13964; _37055 = _43291, _37054 = _43290, _37053++)
                        {
                            uint _14082 = _37050 + (16u + _37053);
                            float _14102 = ((sqrt(_37054) * 1.000003814697265625) + 0.03125) + ((abs(_5785.x) + abs(_5785.y)) * 3.814697265625e-06);
                            float2 _14122 = fast::max(fast::max(gFxData_1._data[_14082].xy - _5785, _5785 - gFxData_1._data[_14082].zw), float2(0.0));
                            if (dot(_14122, _14122) > (_14102 * _14102))
                            {
                                _43291 = _37055;
                                _43290 = _37054;
                                continue;
                            }
                            float4 _13990 = select(gFxData_1._data[_14075], gFxData_1._data[_14068], bool4(_37053 < 2u));
                            uint _14004 = min(_13964, ((_37053 * 8u) + 8u));
                            uint _14006 = _37053 * 8u;
                            float2 _37094;
                            float _37095;
                            uint _37096;
                            _37096 = _37055;
                            _37095 = _37054;
                            _37094 = select(_13990.xy, _13990.zw, bool2((_37053 & 1u) != 0u));
                            float2 _43334;
                            float _43335;
                            uint _43336;
                            for (uint _37093 = _14006; _37093 < _14004; _37096 = _43336, _37095 = _43335, _37094 = _43334, _37093 += 2u)
                            {
                                uint _14130 = _37050 + (_37093 >> 1u);
                                uint _14019 = _13960 + _37093;
                                float2 _14140 = _37094 - gFxData_1._data[_14130].xy;
                                float2 _14143 = _5785 - gFxData_1._data[_14130].xy;
                                float2 _14156 = _14143 - (_14140 * fast::clamp(dot(_14143, _14140) / fast::max(dot(_14140, _14140), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _14159 = dot(_14156, _14156);
                                bool _14170 = (_14159 < _37095) || ((_14159 == _37095) && (_14019 < _37096));
                                uint _43907 = _14170 ? _14019 : _37096;
                                float _43908 = _14170 ? _14159 : _37095;
                                if ((_37093 + 1u) < _14004)
                                {
                                    uint _14041 = (_13960 + _37093) + 1u;
                                    float2 _14183 = gFxData_1._data[_14130].xy - gFxData_1._data[_14130].zw;
                                    float2 _14186 = _5785 - gFxData_1._data[_14130].zw;
                                    float2 _14199 = _14186 - (_14183 * fast::clamp(dot(_14186, _14183) / fast::max(dot(_14183, _14183), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _14202 = dot(_14199, _14199);
                                    bool _14213 = (_14202 < _43908) || ((_14202 == _43908) && (_14041 < _43907));
                                    _43336 = _14213 ? _14041 : _43907;
                                    _43335 = _14213 ? _14202 : _43908;
                                    _43334 = gFxData_1._data[_14130].zw;
                                }
                                else
                                {
                                    _43336 = _43907;
                                    _43335 = _43908;
                                    _43334 = gFxData_1._data[_14130].xy;
                                }
                            }
                            _43291 = _37096;
                            _43290 = _37095;
                        }
                        uint _14222 = (in.i_instance + 1u) * 24u;
                        float _37057;
                        _37057 = _37054;
                        uint _14570;
                        float _43292;
                        uint _43318;
                        for (uint _37056 = 0u, _37068 = _14222, _37069 = _37055; _37056 < _13812; _37069 = _43318, _37068 = _14570, _37057 = _43292, _37056++)
                        {
                            uint _14227 = _37068 + 20u;
                            float _14247 = ((sqrt(_37057) * 1.000003814697265625) + 0.03125) + ((abs(_5785.x) + abs(_5785.y)) * 3.814697265625e-06);
                            float2 _14267 = fast::max(fast::max(gFxData_1._data[_14227].xy - _5785, _5785 - gFxData_1._data[_14227].zw), float2(0.0));
                            if ((_37056 != _37051) && (!(dot(_14267, _14267) > (_14247 * _14247))))
                            {
                                uint _14308 = _37056 * 32u;
                                uint _14312 = min((_13809 - _14308), 32u);
                                uint _14416 = _37068 + 21u;
                                uint _14423 = _37068 + 22u;
                                float _37071;
                                uint _37072;
                                _37072 = _37069;
                                _37071 = _37057;
                                float _43319;
                                uint _43320;
                                for (uint _37070 = 0u; (_37070 * 8u) < _14312; _37072 = _43320, _37071 = _43319, _37070++)
                                {
                                    uint _14430 = _37068 + (16u + _37070);
                                    float _14450 = ((sqrt(_37071) * 1.000003814697265625) + 0.03125) + ((abs(_5785.x) + abs(_5785.y)) * 3.814697265625e-06);
                                    float2 _14470 = fast::max(fast::max(gFxData_1._data[_14430].xy - _5785, _5785 - gFxData_1._data[_14430].zw), float2(0.0));
                                    if (dot(_14470, _14470) > (_14450 * _14450))
                                    {
                                        _43320 = _37072;
                                        _43319 = _37071;
                                        continue;
                                    }
                                    float4 _14338 = select(gFxData_1._data[_14423], gFxData_1._data[_14416], bool4(_37070 < 2u));
                                    uint _14352 = min(_14312, ((_37070 * 8u) + 8u));
                                    uint _14354 = _37070 * 8u;
                                    float2 _37078;
                                    float _37079;
                                    uint _37080;
                                    _37080 = _37072;
                                    _37079 = _37071;
                                    _37078 = select(_14338.xy, _14338.zw, bool2((_37070 & 1u) != 0u));
                                    float2 _43331;
                                    float _43332;
                                    uint _43333;
                                    for (uint _37077 = _14354; _37077 < _14352; _37080 = _43333, _37079 = _43332, _37078 = _43331, _37077 += 2u)
                                    {
                                        uint _14478 = _37068 + (_37077 >> 1u);
                                        uint _14367 = _14308 + _37077;
                                        float2 _14488 = _37078 - gFxData_1._data[_14478].xy;
                                        float2 _14491 = _5785 - gFxData_1._data[_14478].xy;
                                        float2 _14504 = _14491 - (_14488 * fast::clamp(dot(_14491, _14488) / fast::max(dot(_14488, _14488), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _14507 = dot(_14504, _14504);
                                        bool _14518 = (_14507 < _37079) || ((_14507 == _37079) && (_14367 < _37080));
                                        uint _43911 = _14518 ? _14367 : _37080;
                                        float _43912 = _14518 ? _14507 : _37079;
                                        if ((_37077 + 1u) < _14352)
                                        {
                                            uint _14389 = (_14308 + _37077) + 1u;
                                            float2 _14531 = gFxData_1._data[_14478].xy - gFxData_1._data[_14478].zw;
                                            float2 _14534 = _5785 - gFxData_1._data[_14478].zw;
                                            float2 _14547 = _14534 - (_14531 * fast::clamp(dot(_14534, _14531) / fast::max(dot(_14531, _14531), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _14550 = dot(_14547, _14547);
                                            bool _14561 = (_14550 < _43912) || ((_14550 == _43912) && (_14389 < _43911));
                                            _43333 = _14561 ? _14389 : _43911;
                                            _43332 = _14561 ? _14550 : _43912;
                                            _43331 = gFxData_1._data[_14478].zw;
                                        }
                                        else
                                        {
                                            _43333 = _43911;
                                            _43332 = _43912;
                                            _43331 = gFxData_1._data[_14478].xy;
                                        }
                                    }
                                    _43320 = _37080;
                                    _43319 = _37079;
                                }
                                _43318 = _37072;
                                _43292 = _37071;
                            }
                            else
                            {
                                _43318 = _37069;
                                _43292 = _37057;
                            }
                            _14570 = _37068 + 24u;
                        }
                        _37060 = 1.0;
                        _37058 = _37057;
                    }
                    else
                    {
                        float _13647 = floor((in.i_radii.z - 1.0) * 0.5);
                        float _14578 = floor(_13647 * 0.0625);
                        uint _14595 = (((in.i_instance + 1u) + uint(_14578)) * 24u) + uint(_13647 - (_14578 * 16.0));
                        float2 _37028;
                        if (_13625)
                        {
                            float _14605 = floor(0.0);
                            _37028 = gFxData_1._data[(((in.i_instance + 1u) + uint(_14605)) * 24u) + uint(_14605 * (-16.0))].xy;
                        }
                        else
                        {
                            _37028 = select(gFxData_1._data[_14595].zw, gFxData_1._data[_14595].xy, bool2(((in.i_radii.z - 1.0) - (_13647 * 2.0)) < 0.5));
                        }
                        float2 _37027;
                        float _37030;
                        float _37033;
                        _37033 = 1.0;
                        _37030 = 1000000015047466219876688855040.0;
                        _37027 = _37028;
                        float2 _37026;
                        float _37029;
                        float _37032;
                        for (float _37023 = 0.0; _37023 < 16.0; _37033 = _37032, _37030 = _37029, _37027 = _37026, _37023 += 1.0)
                        {
                            if (_37023 >= _13633)
                            {
                                break;
                            }
                            _37032 = _37033;
                            _37029 = _37030;
                            _37026 = _37027;
                            float2 _43282;
                            float _43284;
                            float _43286;
                            for (float _37024 = 0.0; _37024 < 128.0; _37032 = _43286, _37029 = _43284, _37026 = _43282, _37024 += 1.0)
                            {
                                float _13689 = (_37023 * 128.0) + _37024;
                                if (_13689 >= _13629)
                                {
                                    break;
                                }
                                float _14632 = floor(_13689 * 0.0625);
                                uint _14649 = (((in.i_instance + 1u) + uint(_14632)) * 24u) + uint(_13689 - (_14632 * 16.0));
                                float2 _14662 = _37026 - gFxData_1._data[_14649].xy;
                                float2 _14665 = _5785 - gFxData_1._data[_14649].xy;
                                float2 _14678 = _14665 - (_14662 * fast::clamp(dot(_14665, _14662) / fast::max(dot(_14662, _14662), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _14681 = dot(_14678, _14678);
                                float _43915 = (_14681 < _37029) ? _14681 : _37029;
                                bool _14693 = _5785.y >= gFxData_1._data[_14649].y;
                                bool _14698 = _5785.y < _37026.y;
                                bool _14709 = (_14662.x * _14665.y) > (_14662.y * _14665.x);
                                float _37038;
                                if (((_14693 && _14698) && _14709) || (((!_14693) && (!_14698)) && (!_14709)))
                                {
                                    _37038 = -_37032;
                                }
                                else
                                {
                                    _37038 = _37032;
                                }
                                if (((2.0 * _13689) + 1.0) < in.i_radii.z)
                                {
                                    float2 _14738 = gFxData_1._data[_14649].xy - gFxData_1._data[_14649].zw;
                                    float2 _14741 = _5785 - gFxData_1._data[_14649].zw;
                                    float2 _14754 = _14741 - (_14738 * fast::clamp(dot(_14741, _14738) / fast::max(dot(_14738, _14738), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _14757 = dot(_14754, _14754);
                                    bool _14769 = _5785.y >= gFxData_1._data[_14649].w;
                                    bool _14774 = _5785.y < gFxData_1._data[_14649].y;
                                    bool _14785 = (_14738.x * _14741.y) > (_14738.y * _14741.x);
                                    float _37042;
                                    if (((_14769 && _14774) && _14785) || (((!_14769) && (!_14774)) && (!_14785)))
                                    {
                                        _37042 = -_37038;
                                    }
                                    else
                                    {
                                        _37042 = _37038;
                                    }
                                    _43286 = _37042;
                                    _43284 = (_14757 < _43915) ? _14757 : _43915;
                                    _43282 = gFxData_1._data[_14649].zw;
                                }
                                else
                                {
                                    _43286 = _37038;
                                    _43284 = _43915;
                                    _43282 = gFxData_1._data[_14649].xy;
                                }
                            }
                        }
                        _37060 = _37033;
                        _37058 = _37030;
                    }
                    float _13742 = sqrt(_37058);
                    if (_13625)
                    {
                        _37065 = _13742 - in.i_radii.x;
                        break;
                    }
                    _37065 = (_37060 * _13742) - in.i_radii.x;
                    break;
                } while(false);
                _37114 = _37065;
            }
            else
            {
                float _37115;
                if (in.i_flags.z == 1u)
                {
                    float2 _13368 = _5785 - _13334;
                    float _37022;
                    do
                    {
                        if (in.i_shape.w >= 6.282185077667236328125)
                        {
                            _37022 = abs(length(_13368) - in.i_radii.x) - in.i_radii.y;
                            break;
                        }
                        float _14828 = in.i_shape.z + (in.i_shape.w * 0.5);
                        float _14830 = cos(_14828);
                        float _14832 = sin(_14828);
                        float _14841 = dot(_13368, float2(-_14832, _14830));
                        float _14844 = dot(_13368, float2(_14830, _14832));
                        float2 _14845 = float2(_14841, _14844);
                        float _14848 = abs(_14841);
                        _14845.x = _14848;
                        float _14851 = in.i_shape.w * 0.5;
                        float _14853 = sin(_14851);
                        float _14855 = cos(_14851);
                        _37022 = (((_14855 * _14848) > (_14853 * _14844)) ? length(_14845 - (float2(_14853, _14855) * in.i_radii.x)) : abs(length(_14845) - in.i_radii.x)) - in.i_radii.y;
                        break;
                    } while(false);
                    _37115 = _37022;
                }
                else
                {
                    float _37116;
                    if (in.i_flags.z == 2u)
                    {
                        float2 _14891 = _5785 - _36247.xy;
                        float2 _14894 = _36247.zw - _36247.xy;
                        _37116 = length(_14891 - (_14894 * fast::clamp(dot(_14891, _14894) / fast::max(dot(_14894, _14894), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                    }
                    else
                    {
                        float2 _13395 = _5785 - _13334;
                        float _14959 = fast::min(_13343.x, _13343.y);
                        float _14962 = fast::min((_13395.x > 0.0) ? ((_13395.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_13395.y > 0.0) ? in.i_radii.w : in.i_radii.x), _14959);
                        float _14968 = _14962 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _37011;
                        float _37012;
                        if (_14968 > _14959)
                        {
                            float _14982 = in.i_shape.y * fast::clamp((_14959 - _14962) / fast::max(0.60000002384185791015625 * _14962, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _37012 = _14982;
                            _37011 = _14962 * (1.0 + (0.60000002384185791015625 * _14982));
                        }
                        else
                        {
                            _37012 = in.i_shape.y;
                            _37011 = _14968;
                        }
                        float2 _15011 = (abs(_13395) - _13343) + float2(_37011);
                        float2 _15013 = fast::max(_15011, float2(0.0));
                        float _37013;
                        if ((_15013.x > 0.0) && (_15013.y > 0.0))
                        {
                            float _37014;
                            if ((_37012 > 0.001000000047497451305389404296875) && (_37011 > 9.9999997473787516355514526367188e-05))
                            {
                                float _15030 = 2.0 + (2.0 * _37012);
                                float2 _15035 = _15013 / float2(fast::max(_37011, 9.9999997473787516355514526367188e-05));
                                _37014 = pow(pow(_15035.x, _15030) + pow(_15035.y, _15030), 1.0 / _15030) * _37011;
                            }
                            else
                            {
                                _37014 = length(_15013);
                            }
                            _37013 = _37014;
                        }
                        else
                        {
                            _37013 = fast::max(_15013.x, _15013.y);
                        }
                        float _15070 = (fast::min(fast::max(_15011.x, _15011.y), 0.0) + _37013) - _37011;
                        float _37117;
                        if ((in.i_flags.x & _2278) != 0u)
                        {
                            float2 _13423 = fast::max((_36247.zw - _36247.xy) * 0.5, float2(0.001000000047497451305389404296875));
                            float2 _13426 = _5785 - ((_36247.xy + _36247.zw) * 0.5);
                            float _15118 = fast::min(_13423.x, _13423.y);
                            float _15121 = fast::min((_13426.x > 0.0) ? ((_13426.y > 0.0) ? _36245.x : _36245.x) : ((_13426.y > 0.0) ? _36245.x : _36245.x), _15118);
                            float _15127 = _15121 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _37015;
                            float _37016;
                            if (_15127 > _15118)
                            {
                                float _15141 = in.i_shape.y * fast::clamp((_15118 - _15121) / fast::max(0.60000002384185791015625 * _15121, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _37016 = _15141;
                                _37015 = _15121 * (1.0 + (0.60000002384185791015625 * _15141));
                            }
                            else
                            {
                                _37016 = in.i_shape.y;
                                _37015 = _15127;
                            }
                            float2 _15170 = (abs(_13426) - _13423) + float2(_37015);
                            float2 _15172 = fast::max(_15170, float2(0.0));
                            float _37017;
                            if ((_15172.x > 0.0) && (_15172.y > 0.0))
                            {
                                float _37018;
                                if ((_37016 > 0.001000000047497451305389404296875) && (_37015 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _15189 = 2.0 + (2.0 * _37016);
                                    float2 _15194 = _15172 / float2(fast::max(_37015, 9.9999997473787516355514526367188e-05));
                                    _37018 = pow(pow(_15194.x, _15189) + pow(_15194.y, _15189), 1.0 / _15189) * _37015;
                                }
                                else
                                {
                                    _37018 = length(_15172);
                                }
                                _37017 = _37018;
                            }
                            else
                            {
                                _37017 = fast::max(_15172.x, _15172.y);
                            }
                            float _15229 = (fast::min(fast::max(_15170.x, _15170.y), 0.0) + _37017) - _37015;
                            float _15234 = fast::max(_36245.y, 9.9999997473787516355514526367188e-05);
                            float _15243 = fast::max(_15234 - abs(_15070 - _15229), 0.0) / _15234;
                            _37117 = fast::min(_15070, _15229) - (((_15243 * _15243) * _15234) * 0.25);
                        }
                        else
                        {
                            _37117 = _15070;
                        }
                        _37116 = _37117;
                    }
                    _37115 = _37116;
                }
                _37114 = _37115;
            }
            _37113 = _37114;
        }
        float _5794 = (_37113 - gFxData_1._data[_13234].y) / (fast::max(gFxData_1._data[_13234].x * 0.5, _5545 * 0.5) * 1.41421353816986083984375);
        float _15260 = sign(_5794);
        float _15262 = abs(_5794);
        float _15273 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_15262 * _15262))) * _15262)) * _15262);
        float _15276 = _15273 * _15273;
        float _15291 = fast::clamp(gFxData_1._data[_13226].w * (0.5 - (0.5 * (_15260 - (_15260 / (_15276 * _15276))))), 0.0, 1.0);
        _37250 = float4(gFxData_1._data[_13226].xyz * _15291, _15291);
    }
    else
    {
        _37250 = float4(0.0);
    }
    float4 _37350;
    if (((in.i_flags.x & _3882) != 0u) && (!_5755))
    {
        uint _15315 = (in.i_instance * 24u) + 9u;
        uint _15323 = (in.i_instance * 24u) + 10u;
        float _5822 = fast::max(gFxData_1._data[_15323].x, 0.001000000047497451305389404296875);
        float _37243;
        float _37246;
        if ((in.i_flags.x & _3911) != 0u)
        {
            uint _15331 = (in.i_instance * 24u) + 22u;
            float2 _5838 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
            float2 _5847 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
            float2 _5854 = in.i_rect.xy - gFxData_1._data[_15331].xy;
            float2 _5861 = gFxData_1._data[_15331].zw - in.i_rect.zw;
            float2 _5891 = fast::clamp(float2((in.i_local.x < _5838.x) ? _5854.x : _5861.x, (in.i_local.y < _5838.y) ? _5854.y : _5861.y) * float2(0.58823525905609130859375), float2(fast::min(_5822, 1.5)), float2(_5822));
            float _5896 = fast::min(_5847.x, _5847.y);
            float _37241;
            if (in.i_flags.z == 0u)
            {
                _37241 = fast::min(((in.i_local.x > _5838.x) ? ((in.i_local.y > _5838.y) ? in.i_radii.z : in.i_radii.y) : ((in.i_local.y > _5838.y) ? in.i_radii.w : in.i_radii.x)) * (1.0 + (0.60000002384185791015625 * in.i_shape.y)), _5896);
            }
            else
            {
                _37241 = _5896;
            }
            float2 _5945 = fast::max(abs(in.i_local - _5838) - (_5847 - float2(_37241)), float2(0.0));
            float _5947 = length(_5945);
            float2 _5955 = select(float2(0.707099974155426025390625), _5945 / float2(_5947), bool2(_5947 > 9.9999997473787516355514526367188e-05));
            float2 _5970 = in.i_local - gFxData_1._data[_15331].xy;
            float2 _5975 = gFxData_1._data[_15331].zw - in.i_local;
            _37246 = fast::clamp(fast::min(fast::min(_5970.x, _5970.y), fast::min(_5975.x, _5975.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _37243 = rsqrt(dot(_5955 * _5955, float2(1.0) / (_5891 * _5891)));
        }
        else
        {
            _37246 = 1.0;
            _37243 = _5822;
        }
        float _5993 = fast::max(_36352, 0.0) / _37243;
        float _15341 = fast::clamp(gFxData_1._data[_15315].w * fast::clamp((exp(((-_5993) * _5993) * 2.2000000476837158203125) * gFxData_1._data[_15323].y) * _37246, 0.0, 1.0), 0.0, 1.0);
        _37350 = float4(gFxData_1._data[_15315].xyz * _15341, _15341) + (_37250 * (1.0 - _15341));
    }
    else
    {
        _37350 = _37250;
    }
    float4 _39403;
    if ((in.i_flags.x & _4127) != 0u)
    {
        float2 _6024 = _36240 * _288.gTarget.zw;
        float _15374 = fast::clamp(log2(fast::max(fast::max(in.i_radii.w / _5545, 4.0), 1.0)) - 1.0, 0.0, 5.0);
        int _15377 = int(floor(_15374));
        float _15381 = _15374 - float(_15377);
        bool _15386 = (_15381 > 0.0199999995529651641845703125) && (_15377 < 5);
        float3 _37322;
        _37322 = float3(0.0);
        float3 _15415;
        for (int _37252 = 0; _37252 < 2; _37322 = _15415, _37252++)
        {
            if ((_37252 > 0) && (!_15386))
            {
                break;
            }
            int _15401 = _15377 + _37252;
            int _15439 = clamp(_15401, 1, 5);
            float2 _15508 = (_6024 * _288.gLevel[_15439].xy) - float2(0.5);
            float2 _15510 = floor(_15508);
            float2 _15513 = _15508 - _15510;
            float2 _15516 = _15513 * _15513;
            float2 _15519 = _15516 * _15513;
            float2 _15538 = (((_15519 * 3.0) - (_15516 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
            float2 _15551 = _15519 * 0.16666667163372039794921875;
            float2 _15554 = (((((-_15519) + (_15516 * 3.0)) - (_15513 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15538;
            float2 _15558 = (((((_15519 * (-3.0)) + (_15516 * 3.0)) + (_15513 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _15551;
            float2 _15570 = ((_15510 - float2(0.5)) + (_15538 / _15554)) * _288.gLevel[_15439].zw;
            float2 _15582 = ((_15510 + float2(1.5)) + (_15551 / _15558)) * _288.gLevel[_15439].zw;
            float4 _37289;
            if (_15401 <= 0)
            {
                float2 _37288;
                if (_288.gConv.x > 0.5)
                {
                    float2 _34882 = _6024;
                    _34882.y = 1.0 - _6024.y;
                    _37288 = _34882;
                }
                else
                {
                    _37288 = _6024;
                }
                _37289 = gBackdrop0.sample(gLinear, _37288, level(0.0));
            }
            else
            {
                float4 _37290;
                if (_15401 == 1)
                {
                    float2 _37281;
                    if (_288.gConv.x > 0.5)
                    {
                        float2 _34887 = _15570;
                        _34887.y = 1.0 - _15570.y;
                        _37281 = _34887;
                    }
                    else
                    {
                        _37281 = _15570;
                    }
                    float _15627 = _15570.y;
                    float2 _15628 = float2(_15582.x, _15627);
                    float2 _37282;
                    if (_288.gConv.x > 0.5)
                    {
                        float2 _34894 = _15628;
                        _34894.y = 1.0 - _15627;
                        _37282 = _34894;
                    }
                    else
                    {
                        _37282 = _15628;
                    }
                    float _15645 = _15582.y;
                    float2 _15646 = float2(_15570.x, _15645);
                    float2 _37284;
                    if (_288.gConv.x > 0.5)
                    {
                        float2 _34901 = _15646;
                        _34901.y = 1.0 - _15645;
                        _37284 = _34901;
                    }
                    else
                    {
                        _37284 = _15646;
                    }
                    float2 _37286;
                    if (_288.gConv.x > 0.5)
                    {
                        float2 _34908 = _15582;
                        _34908.y = 1.0 - _15582.y;
                        _37286 = _34908;
                    }
                    else
                    {
                        _37286 = _15582;
                    }
                    _37290 = (((gBackdrop1.sample(gLinear, _37281, level(0.0)) * (_15554.x * _15554.y)) + (gBackdrop1.sample(gLinear, _37282, level(0.0)) * (_15558.x * _15554.y))) + (gBackdrop1.sample(gLinear, _37284, level(0.0)) * (_15554.x * _15558.y))) + (gBackdrop1.sample(gLinear, _37286, level(0.0)) * (_15558.x * _15558.y));
                }
                else
                {
                    float4 _37291;
                    if (_15401 == 2)
                    {
                        float2 _37274;
                        if (_288.gConv.x > 0.5)
                        {
                            float2 _34915 = _15570;
                            _34915.y = 1.0 - _15570.y;
                            _37274 = _34915;
                        }
                        else
                        {
                            _37274 = _15570;
                        }
                        float _15757 = _15570.y;
                        float2 _15758 = float2(_15582.x, _15757);
                        float2 _37275;
                        if (_288.gConv.x > 0.5)
                        {
                            float2 _34922 = _15758;
                            _34922.y = 1.0 - _15757;
                            _37275 = _34922;
                        }
                        else
                        {
                            _37275 = _15758;
                        }
                        float _15775 = _15582.y;
                        float2 _15776 = float2(_15570.x, _15775);
                        float2 _37277;
                        if (_288.gConv.x > 0.5)
                        {
                            float2 _34929 = _15776;
                            _34929.y = 1.0 - _15775;
                            _37277 = _34929;
                        }
                        else
                        {
                            _37277 = _15776;
                        }
                        float2 _37279;
                        if (_288.gConv.x > 0.5)
                        {
                            float2 _34936 = _15582;
                            _34936.y = 1.0 - _15582.y;
                            _37279 = _34936;
                        }
                        else
                        {
                            _37279 = _15582;
                        }
                        _37291 = (((gBackdrop2.sample(gLinear, _37274, level(0.0)) * (_15554.x * _15554.y)) + (gBackdrop2.sample(gLinear, _37275, level(0.0)) * (_15558.x * _15554.y))) + (gBackdrop2.sample(gLinear, _37277, level(0.0)) * (_15554.x * _15558.y))) + (gBackdrop2.sample(gLinear, _37279, level(0.0)) * (_15558.x * _15558.y));
                    }
                    else
                    {
                        float4 _37292;
                        if (_15401 == 3)
                        {
                            float2 _37267;
                            if (_288.gConv.x > 0.5)
                            {
                                float2 _34943 = _15570;
                                _34943.y = 1.0 - _15570.y;
                                _37267 = _34943;
                            }
                            else
                            {
                                _37267 = _15570;
                            }
                            float _15887 = _15570.y;
                            float2 _15888 = float2(_15582.x, _15887);
                            float2 _37268;
                            if (_288.gConv.x > 0.5)
                            {
                                float2 _34950 = _15888;
                                _34950.y = 1.0 - _15887;
                                _37268 = _34950;
                            }
                            else
                            {
                                _37268 = _15888;
                            }
                            float _15905 = _15582.y;
                            float2 _15906 = float2(_15570.x, _15905);
                            float2 _37270;
                            if (_288.gConv.x > 0.5)
                            {
                                float2 _34957 = _15906;
                                _34957.y = 1.0 - _15905;
                                _37270 = _34957;
                            }
                            else
                            {
                                _37270 = _15906;
                            }
                            float2 _37272;
                            if (_288.gConv.x > 0.5)
                            {
                                float2 _34964 = _15582;
                                _34964.y = 1.0 - _15582.y;
                                _37272 = _34964;
                            }
                            else
                            {
                                _37272 = _15582;
                            }
                            _37292 = (((gBackdrop3.sample(gLinear, _37267, level(0.0)) * (_15554.x * _15554.y)) + (gBackdrop3.sample(gLinear, _37268, level(0.0)) * (_15558.x * _15554.y))) + (gBackdrop3.sample(gLinear, _37270, level(0.0)) * (_15554.x * _15558.y))) + (gBackdrop3.sample(gLinear, _37272, level(0.0)) * (_15558.x * _15558.y));
                        }
                        else
                        {
                            float4 _37293;
                            if (_15401 == 4)
                            {
                                float2 _37260;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _34971 = _15570;
                                    _34971.y = 1.0 - _15570.y;
                                    _37260 = _34971;
                                }
                                else
                                {
                                    _37260 = _15570;
                                }
                                float _16017 = _15570.y;
                                float2 _16018 = float2(_15582.x, _16017);
                                float2 _37261;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _34978 = _16018;
                                    _34978.y = 1.0 - _16017;
                                    _37261 = _34978;
                                }
                                else
                                {
                                    _37261 = _16018;
                                }
                                float _16035 = _15582.y;
                                float2 _16036 = float2(_15570.x, _16035);
                                float2 _37263;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _34985 = _16036;
                                    _34985.y = 1.0 - _16035;
                                    _37263 = _34985;
                                }
                                else
                                {
                                    _37263 = _16036;
                                }
                                float2 _37265;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _34992 = _15582;
                                    _34992.y = 1.0 - _15582.y;
                                    _37265 = _34992;
                                }
                                else
                                {
                                    _37265 = _15582;
                                }
                                _37293 = (((gBackdrop4.sample(gLinear, _37260, level(0.0)) * (_15554.x * _15554.y)) + (gBackdrop4.sample(gLinear, _37261, level(0.0)) * (_15558.x * _15554.y))) + (gBackdrop4.sample(gLinear, _37263, level(0.0)) * (_15554.x * _15558.y))) + (gBackdrop4.sample(gLinear, _37265, level(0.0)) * (_15558.x * _15558.y));
                            }
                            else
                            {
                                float2 _37253;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _34999 = _15570;
                                    _34999.y = 1.0 - _15570.y;
                                    _37253 = _34999;
                                }
                                else
                                {
                                    _37253 = _15570;
                                }
                                float _16147 = _15570.y;
                                float2 _16148 = float2(_15582.x, _16147);
                                float2 _37254;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _35006 = _16148;
                                    _35006.y = 1.0 - _16147;
                                    _37254 = _35006;
                                }
                                else
                                {
                                    _37254 = _16148;
                                }
                                float _16165 = _15582.y;
                                float2 _16166 = float2(_15570.x, _16165);
                                float2 _37256;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _35013 = _16166;
                                    _35013.y = 1.0 - _16165;
                                    _37256 = _35013;
                                }
                                else
                                {
                                    _37256 = _16166;
                                }
                                float2 _37258;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _35020 = _15582;
                                    _35020.y = 1.0 - _15582.y;
                                    _37258 = _35020;
                                }
                                else
                                {
                                    _37258 = _15582;
                                }
                                _37293 = (((gBackdrop5.sample(gLinear, _37253, level(0.0)) * (_15554.x * _15554.y)) + (gBackdrop5.sample(gLinear, _37254, level(0.0)) * (_15558.x * _15554.y))) + (gBackdrop5.sample(gLinear, _37256, level(0.0)) * (_15554.x * _15558.y))) + (gBackdrop5.sample(gLinear, _37258, level(0.0)) * (_15558.x * _15558.y));
                            }
                            _37292 = _37293;
                        }
                        _37291 = _37292;
                    }
                    _37290 = _37291;
                }
                _37289 = _37290;
            }
            _15415 = _37322 + (_37289.xyz * (_15386 ? ((_37252 == 0) ? (1.0 - _15381) : _15381) : 1.0));
        }
        float _16251 = dot(in.i_fill0.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
        float _6054 = fast::max(_36352, 0.0) / fast::max(in.i_radii.w, 0.001000000047497451305389404296875);
        float _6057 = (_16251 > 0.5) ? 0.0 : 1.0;
        float _16263 = fast::clamp(0.85000002384185791015625 * ((fast::clamp((0.5 - abs(_16251 - dot(select(float3(0.5), _37322, bool3(_288.gTime.z > 0.5)), float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)))) * 4.0, 0.0, 1.0) * in.i_radii.z) * exp(((-_6054) * _6054) * 2.2000000476837158203125)), 0.0, 1.0);
        _39403 = float4(float4(_6057, _6057, _6057, 0.85000002384185791015625).xyz * _16263, _16263) + (_37350 * (1.0 - _16263));
    }
    else
    {
        _39403 = _37350;
    }
    float4 _40710;
    float4 _41032;
    float _41991;
    if (((in.i_flags.x & _4203) != 0u) && (_36985 > 0.0))
    {
        uint _16287 = (in.i_instance * 24u) + 11u;
        uint _16295 = (in.i_instance * 24u) + 12u;
        uint _16303 = (in.i_instance * 24u) + 13u;
        uint _16311 = (in.i_instance * 24u) + 16u;
        float _16384 = gFxData_1._data[_16287].x * _288.gDisplay.z;
        float2 _16395 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(1.0));
        float _16403 = fast::clamp(gFxData_1._data[_16287].z, 0.001000000047497451305389404296875, fast::min(_16395.x, _16395.y));
        float _16412 = fast::clamp(1.0 - (fast::max(-_36352, 0.0) / _16403), 0.0, 1.0);
        float _16418 = sqrt(fast::clamp(1.0 - (_16412 * _16412), 0.0, 1.0));
        float2 _38306;
        float3 _38883;
        if (_16412 > 0.0)
        {
            float2 _38305;
            do
            {
                if ((in.i_flags.x & _2330) != 0u)
                {
                    float2 _38304;
                    do
                    {
                        bool _17032 = in.i_radii.w > 0.5;
                        float _17036 = floor((in.i_radii.z + 1.0) * 0.5);
                        float _17040 = floor((_17036 + 127.0) * 0.0078125);
                        float _38295;
                        float2 _38297;
                        float _38299;
                        if (_17032)
                        {
                            uint _17216 = uint(in.i_radii.z);
                            uint _17219 = (_17216 + 31u) >> 5u;
                            uint _17302 = (in.i_instance + 1u) * 24u;
                            uint _38284;
                            uint _38285;
                            _38285 = 0u;
                            _38284 = _17302;
                            uint _17329;
                            float _43917;
                            uint _43918;
                            uint _43919;
                            uint _38283 = 0u;
                            uint _42966 = _17302;
                            float _42967 = 3.0000000054977557577780399428115e+38;
                            for (; _38283 < _17219; _42967 = _43917, _42966 = _17329, _38285 = _43918, _38284 = _43919, _38283++)
                            {
                                uint _17307 = _42966 + 20u;
                                float2 _17322 = fast::max(fast::max(gFxData_1._data[_17307].xy - in.i_local, in.i_local - gFxData_1._data[_17307].zw), float2(0.0));
                                float _17325 = dot(_17322, _17322);
                                bool _17237 = _17325 < _42967;
                                _43917 = _17237 ? _17325 : _42967;
                                _43918 = _17237 ? _38283 : _38285;
                                _43919 = _17237 ? _42966 : _38284;
                                _17329 = _42966 + 24u;
                            }
                            uint _17367 = _38285 * 32u;
                            uint _17371 = min((_17216 - _17367), 32u);
                            uint _17475 = _38284 + 21u;
                            uint _17482 = _38284 + 22u;
                            float _38289;
                            float2 _38290;
                            uint _38291;
                            _38291 = 4294967295u;
                            _38290 = float2(0.0, -1.0);
                            _38289 = 1000000015047466219876688855040.0;
                            float _43603;
                            float2 _43604;
                            uint _43605;
                            for (uint _38288 = 0u; (_38288 * 8u) < _17371; _38291 = _43605, _38290 = _43604, _38289 = _43603, _38288++)
                            {
                                uint _17489 = _38284 + (16u + _38288);
                                float _17509 = ((sqrt(_38289) * 1.000003814697265625) + 0.03125) + ((abs(in.i_local.x) + abs(in.i_local.y)) * 3.814697265625e-06);
                                float2 _17529 = fast::max(fast::max(gFxData_1._data[_17489].xy - in.i_local, in.i_local - gFxData_1._data[_17489].zw), float2(0.0));
                                if (dot(_17529, _17529) > (_17509 * _17509))
                                {
                                    _43605 = _38291;
                                    _43604 = _38290;
                                    _43603 = _38289;
                                    continue;
                                }
                                float4 _17397 = select(gFxData_1._data[_17482], gFxData_1._data[_17475], bool4(_38288 < 2u));
                                uint _17411 = min(_17371, ((_38288 * 8u) + 8u));
                                uint _17413 = _38288 * 8u;
                                float2 _42948;
                                float _42949;
                                float2 _42950;
                                uint _42951;
                                _42951 = _38291;
                                _42950 = _38290;
                                _42949 = _38289;
                                _42948 = select(_17397.xy, _17397.zw, bool2((_38288 & 1u) != 0u));
                                float2 _43860;
                                float _43861;
                                float2 _43862;
                                uint _43863;
                                for (uint _42947 = _17413; _42947 < _17411; _42951 = _43863, _42950 = _43862, _42949 = _43861, _42948 = _43860, _42947 += 2u)
                                {
                                    uint _17537 = _38284 + (_42947 >> 1u);
                                    uint _17426 = _17367 + _42947;
                                    float2 _17547 = _42948 - gFxData_1._data[_17537].xy;
                                    float2 _17550 = in.i_local - gFxData_1._data[_17537].xy;
                                    float2 _17563 = _17550 - (_17547 * fast::clamp(dot(_17550, _17547) / fast::max(dot(_17547, _17547), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _17566 = dot(_17563, _17563);
                                    bool _17577 = (_17566 < _42949) || ((_17566 == _42949) && (_17426 < _42951));
                                    uint _43920 = _17577 ? _17426 : _42951;
                                    float2 _43922 = select(_42950, _17563, bool2(_17577));
                                    float _43923 = _17577 ? _17566 : _42949;
                                    if ((_42947 + 1u) < _17411)
                                    {
                                        uint _17448 = (_17367 + _42947) + 1u;
                                        float2 _17590 = gFxData_1._data[_17537].xy - gFxData_1._data[_17537].zw;
                                        float2 _17593 = in.i_local - gFxData_1._data[_17537].zw;
                                        float2 _17606 = _17593 - (_17590 * fast::clamp(dot(_17593, _17590) / fast::max(dot(_17590, _17590), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _17609 = dot(_17606, _17606);
                                        bool _17620 = (_17609 < _43923) || ((_17609 == _43923) && (_17448 < _43920));
                                        _43863 = _17620 ? _17448 : _43920;
                                        _43862 = select(_43922, _17606, bool2(_17620));
                                        _43861 = _17620 ? _17609 : _43923;
                                        _43860 = gFxData_1._data[_17537].zw;
                                    }
                                    else
                                    {
                                        _43863 = _43920;
                                        _43862 = _43922;
                                        _43861 = _43923;
                                        _43860 = gFxData_1._data[_17537].xy;
                                    }
                                }
                                _43605 = _42951;
                                _43604 = _42950;
                                _43603 = _42949;
                            }
                            uint _17629 = (in.i_instance + 1u) * 24u;
                            float _38293;
                            float2 _38294;
                            _38294 = _38290;
                            _38293 = _38289;
                            uint _17977;
                            float _43606;
                            float2 _43607;
                            uint _43842;
                            for (uint _38292 = 0u, _42918 = _17629, _42919 = _38291; _38292 < _17219; _42919 = _43842, _42918 = _17977, _38294 = _43607, _38293 = _43606, _38292++)
                            {
                                uint _17634 = _42918 + 20u;
                                float _17654 = ((sqrt(_38293) * 1.000003814697265625) + 0.03125) + ((abs(in.i_local.x) + abs(in.i_local.y)) * 3.814697265625e-06);
                                float2 _17674 = fast::max(fast::max(gFxData_1._data[_17634].xy - in.i_local, in.i_local - gFxData_1._data[_17634].zw), float2(0.0));
                                if ((_38292 != _38285) && (!(dot(_17674, _17674) > (_17654 * _17654))))
                                {
                                    uint _17715 = _38292 * 32u;
                                    uint _17719 = min((_17216 - _17715), 32u);
                                    uint _17823 = _42918 + 21u;
                                    uint _17830 = _42918 + 22u;
                                    float _42921;
                                    float2 _42922;
                                    uint _42923;
                                    _42923 = _42919;
                                    _42922 = _38294;
                                    _42921 = _38293;
                                    float _43843;
                                    float2 _43844;
                                    uint _43845;
                                    for (uint _42920 = 0u; (_42920 * 8u) < _17719; _42923 = _43845, _42922 = _43844, _42921 = _43843, _42920++)
                                    {
                                        uint _17837 = _42918 + (16u + _42920);
                                        float _17857 = ((sqrt(_42921) * 1.000003814697265625) + 0.03125) + ((abs(in.i_local.x) + abs(in.i_local.y)) * 3.814697265625e-06);
                                        float2 _17877 = fast::max(fast::max(gFxData_1._data[_17837].xy - in.i_local, in.i_local - gFxData_1._data[_17837].zw), float2(0.0));
                                        if (dot(_17877, _17877) > (_17857 * _17857))
                                        {
                                            _43845 = _42923;
                                            _43844 = _42922;
                                            _43843 = _42921;
                                            continue;
                                        }
                                        float4 _17745 = select(gFxData_1._data[_17830], gFxData_1._data[_17823], bool4(_42920 < 2u));
                                        uint _17759 = min(_17719, ((_42920 * 8u) + 8u));
                                        uint _17761 = _42920 * 8u;
                                        float2 _42929;
                                        float _42930;
                                        float2 _42931;
                                        uint _42932;
                                        _42932 = _42923;
                                        _42931 = _42922;
                                        _42930 = _42921;
                                        _42929 = select(_17745.xy, _17745.zw, bool2((_42920 & 1u) != 0u));
                                        float2 _43856;
                                        float _43857;
                                        float2 _43858;
                                        uint _43859;
                                        for (uint _42928 = _17761; _42928 < _17759; _42932 = _43859, _42931 = _43858, _42930 = _43857, _42929 = _43856, _42928 += 2u)
                                        {
                                            uint _17885 = _42918 + (_42928 >> 1u);
                                            uint _17774 = _17715 + _42928;
                                            float2 _17895 = _42929 - gFxData_1._data[_17885].xy;
                                            float2 _17898 = in.i_local - gFxData_1._data[_17885].xy;
                                            float2 _17911 = _17898 - (_17895 * fast::clamp(dot(_17898, _17895) / fast::max(dot(_17895, _17895), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _17914 = dot(_17911, _17911);
                                            bool _17925 = (_17914 < _42930) || ((_17914 == _42930) && (_17774 < _42932));
                                            uint _43928 = _17925 ? _17774 : _42932;
                                            float2 _43930 = select(_42931, _17911, bool2(_17925));
                                            float _43931 = _17925 ? _17914 : _42930;
                                            if ((_42928 + 1u) < _17759)
                                            {
                                                uint _17796 = (_17715 + _42928) + 1u;
                                                float2 _17938 = gFxData_1._data[_17885].xy - gFxData_1._data[_17885].zw;
                                                float2 _17941 = in.i_local - gFxData_1._data[_17885].zw;
                                                float2 _17954 = _17941 - (_17938 * fast::clamp(dot(_17941, _17938) / fast::max(dot(_17938, _17938), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _17957 = dot(_17954, _17954);
                                                bool _17968 = (_17957 < _43931) || ((_17957 == _43931) && (_17796 < _43928));
                                                _43859 = _17968 ? _17796 : _43928;
                                                _43858 = select(_43930, _17954, bool2(_17968));
                                                _43857 = _17968 ? _17957 : _43931;
                                                _43856 = gFxData_1._data[_17885].zw;
                                            }
                                            else
                                            {
                                                _43859 = _43928;
                                                _43858 = _43930;
                                                _43857 = _43931;
                                                _43856 = gFxData_1._data[_17885].xy;
                                            }
                                        }
                                        _43845 = _42932;
                                        _43844 = _42931;
                                        _43843 = _42930;
                                    }
                                    _43842 = _42923;
                                    _43607 = _42922;
                                    _43606 = _42921;
                                }
                                else
                                {
                                    _43842 = _42919;
                                    _43607 = _38294;
                                    _43606 = _38293;
                                }
                                _17977 = _42918 + 24u;
                            }
                            _38299 = 1.0;
                            _38297 = _38294;
                            _38295 = _38293;
                        }
                        else
                        {
                            float _17054 = floor((in.i_radii.z - 1.0) * 0.5);
                            float _17985 = floor(_17054 * 0.0625);
                            uint _18002 = (((in.i_instance + 1u) + uint(_17985)) * 24u) + uint(_17054 - (_17985 * 16.0));
                            float2 _38255;
                            if (_17032)
                            {
                                float _18012 = floor(0.0);
                                _38255 = gFxData_1._data[(((in.i_instance + 1u) + uint(_18012)) * 24u) + uint(_18012 * (-16.0))].xy;
                            }
                            else
                            {
                                _38255 = select(gFxData_1._data[_18002].zw, gFxData_1._data[_18002].xy, bool2(((in.i_radii.z - 1.0) - (_17054 * 2.0)) < 0.5));
                            }
                            float2 _38254;
                            float _38257;
                            float2 _38260;
                            float _38263;
                            _38263 = 1.0;
                            _38260 = float2(0.0, -1.0);
                            _38257 = 1000000015047466219876688855040.0;
                            _38254 = _38255;
                            float2 _38253;
                            float _38256;
                            float2 _38259;
                            float _38262;
                            for (float _38250 = 0.0; _38250 < 16.0; _38263 = _38262, _38260 = _38259, _38257 = _38256, _38254 = _38253, _38250 += 1.0)
                            {
                                if (_38250 >= _17040)
                                {
                                    break;
                                }
                                _38262 = _38263;
                                _38259 = _38260;
                                _38256 = _38257;
                                _38253 = _38254;
                                float2 _43592;
                                float _43594;
                                float2 _43596;
                                float _43598;
                                for (float _38251 = 0.0; _38251 < 128.0; _38262 = _43598, _38259 = _43596, _38256 = _43594, _38253 = _43592, _38251 += 1.0)
                                {
                                    float _17096 = (_38250 * 128.0) + _38251;
                                    if (_17096 >= _17036)
                                    {
                                        break;
                                    }
                                    float _18039 = floor(_17096 * 0.0625);
                                    uint _18056 = (((in.i_instance + 1u) + uint(_18039)) * 24u) + uint(_17096 - (_18039 * 16.0));
                                    float2 _18069 = _38253 - gFxData_1._data[_18056].xy;
                                    float2 _18072 = in.i_local - gFxData_1._data[_18056].xy;
                                    float2 _18085 = _18072 - (_18069 * fast::clamp(dot(_18072, _18069) / fast::max(dot(_18069, _18069), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _18088 = dot(_18085, _18085);
                                    bool _18091 = _18088 < _38256;
                                    float2 _43937 = select(_38259, _18085, bool2(_18091));
                                    float _43938 = _18091 ? _18088 : _38256;
                                    bool _18100 = in.i_local.y >= gFxData_1._data[_18056].y;
                                    bool _18105 = in.i_local.y < _38253.y;
                                    bool _18116 = (_18069.x * _18072.y) > (_18069.y * _18072.x);
                                    float _38270;
                                    if (((_18100 && _18105) && _18116) || (((!_18100) && (!_18105)) && (!_18116)))
                                    {
                                        _38270 = -_38262;
                                    }
                                    else
                                    {
                                        _38270 = _38262;
                                    }
                                    if (((2.0 * _17096) + 1.0) < in.i_radii.z)
                                    {
                                        float2 _18145 = gFxData_1._data[_18056].xy - gFxData_1._data[_18056].zw;
                                        float2 _18148 = in.i_local - gFxData_1._data[_18056].zw;
                                        float2 _18161 = _18148 - (_18145 * fast::clamp(dot(_18148, _18145) / fast::max(dot(_18145, _18145), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _18164 = dot(_18161, _18161);
                                        bool _18167 = _18164 < _43938;
                                        bool _18176 = in.i_local.y >= gFxData_1._data[_18056].w;
                                        bool _18181 = in.i_local.y < gFxData_1._data[_18056].y;
                                        bool _18192 = (_18145.x * _18148.y) > (_18145.y * _18148.x);
                                        float _38276;
                                        if (((_18176 && _18181) && _18192) || (((!_18176) && (!_18181)) && (!_18192)))
                                        {
                                            _38276 = -_38270;
                                        }
                                        else
                                        {
                                            _38276 = _38270;
                                        }
                                        _43598 = _38276;
                                        _43596 = select(_43937, _18161, bool2(_18167));
                                        _43594 = _18167 ? _18164 : _43938;
                                        _43592 = gFxData_1._data[_18056].zw;
                                    }
                                    else
                                    {
                                        _43598 = _38270;
                                        _43596 = _43937;
                                        _43594 = _43938;
                                        _43592 = gFxData_1._data[_18056].xy;
                                    }
                                }
                            }
                            _38299 = _38263;
                            _38297 = _38260;
                            _38295 = _38257;
                        }
                        float _17149 = sqrt(_38295);
                        float2 _17157 = select(float2(0.0, -1.0), _38297 / float2(_17149), bool2(_17149 > 9.9999999747524270787835121154785e-07));
                        if (_17032)
                        {
                            _38304 = _17157;
                            break;
                        }
                        _38304 = _17157 * _38299;
                        break;
                    } while(false);
                    _38305 = _38304;
                    break;
                }
                float2 _38249;
                if ((((in.i_flags.z == 0u) && (!((in.i_flags.x & _2350) != 0u))) && (!((in.i_flags.x & _2357) != 0u))) && all(in.i_radii.xyz == in.i_radii.yzw))
                {
                    float2 _16900 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
                    float2 _16909 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float2 _16912 = in.i_local - _16900;
                    float _18241 = fast::min(_16909.x, _16909.y);
                    float _18244 = fast::min((_16912.x > 0.0) ? ((_16912.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_16912.y > 0.0) ? in.i_radii.w : in.i_radii.x), _18241);
                    float _18250 = _18244 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                    float _38025;
                    float _38026;
                    if (_18250 > _18241)
                    {
                        float _18264 = in.i_shape.y * fast::clamp((_18241 - _18244) / fast::max(0.60000002384185791015625 * _18244, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _38026 = _18264;
                        _38025 = _18244 * (1.0 + (0.60000002384185791015625 * _18264));
                    }
                    else
                    {
                        _38026 = in.i_shape.y;
                        _38025 = _18250;
                    }
                    float2 _18293 = (abs((in.i_local + float2(0.5, 0.0)) - _16900) - _16909) + float2(_38025);
                    float2 _18295 = fast::max(_18293, float2(0.0));
                    float _38027;
                    if ((_18295.x > 0.0) && (_18295.y > 0.0))
                    {
                        float _38028;
                        if ((_38026 > 0.001000000047497451305389404296875) && (_38025 > 9.9999997473787516355514526367188e-05))
                        {
                            float _18312 = 2.0 + (2.0 * _38026);
                            float2 _18317 = _18295 / float2(fast::max(_38025, 9.9999997473787516355514526367188e-05));
                            _38028 = pow(pow(_18317.x, _18312) + pow(_18317.y, _18312), 1.0 / _18312) * _38025;
                        }
                        else
                        {
                            _38028 = length(_18295);
                        }
                        _38027 = _38028;
                    }
                    else
                    {
                        _38027 = fast::max(_18295.x, _18295.y);
                    }
                    float2 _18372 = (abs((in.i_local - float2(0.5, 0.0)) - _16900) - _16909) + float2(_38025);
                    float2 _18374 = fast::max(_18372, float2(0.0));
                    float _38029;
                    if ((_18374.x > 0.0) && (_18374.y > 0.0))
                    {
                        float _38030;
                        if ((_38026 > 0.001000000047497451305389404296875) && (_38025 > 9.9999997473787516355514526367188e-05))
                        {
                            float _18391 = 2.0 + (2.0 * _38026);
                            float2 _18396 = _18374 / float2(fast::max(_38025, 9.9999997473787516355514526367188e-05));
                            _38030 = pow(pow(_18396.x, _18391) + pow(_18396.y, _18391), 1.0 / _18391) * _38025;
                        }
                        else
                        {
                            _38030 = length(_18374);
                        }
                        _38029 = _38030;
                    }
                    else
                    {
                        _38029 = fast::max(_18374.x, _18374.y);
                    }
                    float2 _18451 = (abs((in.i_local + float2(0.0, 0.5)) - _16900) - _16909) + float2(_38025);
                    float2 _18453 = fast::max(_18451, float2(0.0));
                    float _38241;
                    if ((_18453.x > 0.0) && (_18453.y > 0.0))
                    {
                        float _38242;
                        if ((_38026 > 0.001000000047497451305389404296875) && (_38025 > 9.9999997473787516355514526367188e-05))
                        {
                            float _18470 = 2.0 + (2.0 * _38026);
                            float2 _18475 = _18453 / float2(fast::max(_38025, 9.9999997473787516355514526367188e-05));
                            _38242 = pow(pow(_18475.x, _18470) + pow(_18475.y, _18470), 1.0 / _18470) * _38025;
                        }
                        else
                        {
                            _38242 = length(_18453);
                        }
                        _38241 = _38242;
                    }
                    else
                    {
                        _38241 = fast::max(_18453.x, _18453.y);
                    }
                    float2 _18530 = (abs((in.i_local - float2(0.0, 0.5)) - _16900) - _16909) + float2(_38025);
                    float2 _18532 = fast::max(_18530, float2(0.0));
                    float _38243;
                    if ((_18532.x > 0.0) && (_18532.y > 0.0))
                    {
                        float _38244;
                        if ((_38026 > 0.001000000047497451305389404296875) && (_38025 > 9.9999997473787516355514526367188e-05))
                        {
                            float _18549 = 2.0 + (2.0 * _38026);
                            float2 _18554 = _18532 / float2(fast::max(_38025, 9.9999997473787516355514526367188e-05));
                            _38244 = pow(pow(_18554.x, _18549) + pow(_18554.y, _18549), 1.0 / _18549) * _38025;
                        }
                        else
                        {
                            _38244 = length(_18532);
                        }
                        _38243 = _38244;
                    }
                    else
                    {
                        _38243 = fast::max(_18532.x, _18532.y);
                    }
                    _38249 = float2(((fast::min(fast::max(_18293.x, _18293.y), 0.0) + _38027) - _38025) - ((fast::min(fast::max(_18372.x, _18372.y), 0.0) + _38029) - _38025), ((fast::min(fast::max(_18451.x, _18451.y), 0.0) + _38241) - _38025) - ((fast::min(fast::max(_18530.x, _18530.y), 0.0) + _38243) - _38025));
                }
                else
                {
                    float2 _16953 = in.i_local + float2(0.5, 0.0);
                    float2 _18634 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
                    float2 _18643 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float _37699;
                    if ((in.i_flags.x & _2192) != 0u)
                    {
                        float _18760 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                        float4 _18794 = gTex.sample(gLinear, mix(_36241.xy, _36241.zw, (fast::clamp(_16953, _36247.xy, _36247.zw) - _36247.xy) / fast::max(_36247.zw - _36247.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                        float _18818 = (_18794.w - 0.5) * in.i_radii.y;
                        float2 _18859 = abs(_16953 - ((_36247.xy + _36247.zw) * 0.5)) - fast::max(((_36247.zw - _36247.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _18760), float2(0.0));
                        _37699 = fast::max((-mix((fast::max(fast::min(_18794.x, _18794.y), fast::min(fast::max(_18794.x, _18794.y), _18794.z)) - 0.5) * in.i_radii.x, _18818, fast::clamp(abs(_18818) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _18760, length(fast::max(_18859, float2(0.0))) + fast::min(fast::max(_18859.x, _18859.y), 0.0)) - in.i_shape.w;
                    }
                    else
                    {
                        float _37700;
                        if ((in.i_flags.x & _2207) != 0u)
                        {
                            float _37651;
                            do
                            {
                                bool _18925 = in.i_radii.w > 0.5;
                                float _18929 = floor((in.i_radii.z + 1.0) * 0.5);
                                float _18933 = floor((_18929 + 127.0) * 0.0078125);
                                float _37644;
                                float _37646;
                                if (_18925)
                                {
                                    uint _19109 = uint(in.i_radii.z);
                                    uint _19112 = (_19109 + 31u) >> 5u;
                                    uint _19195 = (in.i_instance + 1u) * 24u;
                                    uint _37636;
                                    uint _37637;
                                    _37637 = 0u;
                                    _37636 = _19195;
                                    uint _19222;
                                    float _43942;
                                    uint _43943;
                                    uint _43944;
                                    uint _37635 = 0u;
                                    uint _37695 = _19195;
                                    float _37696 = 3.0000000054977557577780399428115e+38;
                                    for (; _37635 < _19112; _37696 = _43942, _37695 = _19222, _37637 = _43943, _37636 = _43944, _37635++)
                                    {
                                        uint _19200 = _37695 + 20u;
                                        float2 _19215 = fast::max(fast::max(gFxData_1._data[_19200].xy - _16953, _16953 - gFxData_1._data[_19200].zw), float2(0.0));
                                        float _19218 = dot(_19215, _19215);
                                        bool _19130 = _19218 < _37696;
                                        _43942 = _19130 ? _19218 : _37696;
                                        _43943 = _19130 ? _37635 : _37637;
                                        _43944 = _19130 ? _37695 : _37636;
                                        _19222 = _37695 + 24u;
                                    }
                                    uint _19260 = _37637 * 32u;
                                    uint _19264 = min((_19109 - _19260), 32u);
                                    uint _19368 = _37636 + 21u;
                                    uint _19375 = _37636 + 22u;
                                    float _37640;
                                    uint _37641;
                                    _37641 = 4294967295u;
                                    _37640 = 1000000015047466219876688855040.0;
                                    float _43352;
                                    uint _43353;
                                    for (uint _37639 = 0u; (_37639 * 8u) < _19264; _37641 = _43353, _37640 = _43352, _37639++)
                                    {
                                        uint _19382 = _37636 + (16u + _37639);
                                        float _19402 = ((sqrt(_37640) * 1.000003814697265625) + 0.03125) + ((abs(_16953.x) + abs(_16953.y)) * 3.814697265625e-06);
                                        float2 _19422 = fast::max(fast::max(gFxData_1._data[_19382].xy - _16953, _16953 - gFxData_1._data[_19382].zw), float2(0.0));
                                        if (dot(_19422, _19422) > (_19402 * _19402))
                                        {
                                            _43353 = _37641;
                                            _43352 = _37640;
                                            continue;
                                        }
                                        float4 _19290 = select(gFxData_1._data[_19375], gFxData_1._data[_19368], bool4(_37639 < 2u));
                                        uint _19304 = min(_19264, ((_37639 * 8u) + 8u));
                                        uint _19306 = _37639 * 8u;
                                        float2 _37680;
                                        float _37681;
                                        uint _37682;
                                        _37682 = _37641;
                                        _37681 = _37640;
                                        _37680 = select(_19290.xy, _19290.zw, bool2((_37639 & 1u) != 0u));
                                        float2 _43396;
                                        float _43397;
                                        uint _43398;
                                        for (uint _37679 = _19306; _37679 < _19304; _37682 = _43398, _37681 = _43397, _37680 = _43396, _37679 += 2u)
                                        {
                                            uint _19430 = _37636 + (_37679 >> 1u);
                                            uint _19319 = _19260 + _37679;
                                            float2 _19440 = _37680 - gFxData_1._data[_19430].xy;
                                            float2 _19443 = _16953 - gFxData_1._data[_19430].xy;
                                            float2 _19456 = _19443 - (_19440 * fast::clamp(dot(_19443, _19440) / fast::max(dot(_19440, _19440), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _19459 = dot(_19456, _19456);
                                            bool _19470 = (_19459 < _37681) || ((_19459 == _37681) && (_19319 < _37682));
                                            uint _43945 = _19470 ? _19319 : _37682;
                                            float _43946 = _19470 ? _19459 : _37681;
                                            if ((_37679 + 1u) < _19304)
                                            {
                                                uint _19341 = (_19260 + _37679) + 1u;
                                                float2 _19483 = gFxData_1._data[_19430].xy - gFxData_1._data[_19430].zw;
                                                float2 _19486 = _16953 - gFxData_1._data[_19430].zw;
                                                float2 _19499 = _19486 - (_19483 * fast::clamp(dot(_19486, _19483) / fast::max(dot(_19483, _19483), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _19502 = dot(_19499, _19499);
                                                bool _19513 = (_19502 < _43946) || ((_19502 == _43946) && (_19341 < _43945));
                                                _43398 = _19513 ? _19341 : _43945;
                                                _43397 = _19513 ? _19502 : _43946;
                                                _43396 = gFxData_1._data[_19430].zw;
                                            }
                                            else
                                            {
                                                _43398 = _43945;
                                                _43397 = _43946;
                                                _43396 = gFxData_1._data[_19430].xy;
                                            }
                                        }
                                        _43353 = _37682;
                                        _43352 = _37681;
                                    }
                                    uint _19522 = (in.i_instance + 1u) * 24u;
                                    float _37643;
                                    _37643 = _37640;
                                    uint _19870;
                                    float _43354;
                                    uint _43380;
                                    for (uint _37642 = 0u, _37654 = _19522, _37655 = _37641; _37642 < _19112; _37655 = _43380, _37654 = _19870, _37643 = _43354, _37642++)
                                    {
                                        uint _19527 = _37654 + 20u;
                                        float _19547 = ((sqrt(_37643) * 1.000003814697265625) + 0.03125) + ((abs(_16953.x) + abs(_16953.y)) * 3.814697265625e-06);
                                        float2 _19567 = fast::max(fast::max(gFxData_1._data[_19527].xy - _16953, _16953 - gFxData_1._data[_19527].zw), float2(0.0));
                                        if ((_37642 != _37637) && (!(dot(_19567, _19567) > (_19547 * _19547))))
                                        {
                                            uint _19608 = _37642 * 32u;
                                            uint _19612 = min((_19109 - _19608), 32u);
                                            uint _19716 = _37654 + 21u;
                                            uint _19723 = _37654 + 22u;
                                            float _37657;
                                            uint _37658;
                                            _37658 = _37655;
                                            _37657 = _37643;
                                            float _43381;
                                            uint _43382;
                                            for (uint _37656 = 0u; (_37656 * 8u) < _19612; _37658 = _43382, _37657 = _43381, _37656++)
                                            {
                                                uint _19730 = _37654 + (16u + _37656);
                                                float _19750 = ((sqrt(_37657) * 1.000003814697265625) + 0.03125) + ((abs(_16953.x) + abs(_16953.y)) * 3.814697265625e-06);
                                                float2 _19770 = fast::max(fast::max(gFxData_1._data[_19730].xy - _16953, _16953 - gFxData_1._data[_19730].zw), float2(0.0));
                                                if (dot(_19770, _19770) > (_19750 * _19750))
                                                {
                                                    _43382 = _37658;
                                                    _43381 = _37657;
                                                    continue;
                                                }
                                                float4 _19638 = select(gFxData_1._data[_19723], gFxData_1._data[_19716], bool4(_37656 < 2u));
                                                uint _19652 = min(_19612, ((_37656 * 8u) + 8u));
                                                uint _19654 = _37656 * 8u;
                                                float2 _37664;
                                                float _37665;
                                                uint _37666;
                                                _37666 = _37658;
                                                _37665 = _37657;
                                                _37664 = select(_19638.xy, _19638.zw, bool2((_37656 & 1u) != 0u));
                                                float2 _43393;
                                                float _43394;
                                                uint _43395;
                                                for (uint _37663 = _19654; _37663 < _19652; _37666 = _43395, _37665 = _43394, _37664 = _43393, _37663 += 2u)
                                                {
                                                    uint _19778 = _37654 + (_37663 >> 1u);
                                                    uint _19667 = _19608 + _37663;
                                                    float2 _19788 = _37664 - gFxData_1._data[_19778].xy;
                                                    float2 _19791 = _16953 - gFxData_1._data[_19778].xy;
                                                    float2 _19804 = _19791 - (_19788 * fast::clamp(dot(_19791, _19788) / fast::max(dot(_19788, _19788), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                    float _19807 = dot(_19804, _19804);
                                                    bool _19818 = (_19807 < _37665) || ((_19807 == _37665) && (_19667 < _37666));
                                                    uint _43949 = _19818 ? _19667 : _37666;
                                                    float _43950 = _19818 ? _19807 : _37665;
                                                    if ((_37663 + 1u) < _19652)
                                                    {
                                                        uint _19689 = (_19608 + _37663) + 1u;
                                                        float2 _19831 = gFxData_1._data[_19778].xy - gFxData_1._data[_19778].zw;
                                                        float2 _19834 = _16953 - gFxData_1._data[_19778].zw;
                                                        float2 _19847 = _19834 - (_19831 * fast::clamp(dot(_19834, _19831) / fast::max(dot(_19831, _19831), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                        float _19850 = dot(_19847, _19847);
                                                        bool _19861 = (_19850 < _43950) || ((_19850 == _43950) && (_19689 < _43949));
                                                        _43395 = _19861 ? _19689 : _43949;
                                                        _43394 = _19861 ? _19850 : _43950;
                                                        _43393 = gFxData_1._data[_19778].zw;
                                                    }
                                                    else
                                                    {
                                                        _43395 = _43949;
                                                        _43394 = _43950;
                                                        _43393 = gFxData_1._data[_19778].xy;
                                                    }
                                                }
                                                _43382 = _37666;
                                                _43381 = _37665;
                                            }
                                            _43380 = _37658;
                                            _43354 = _37657;
                                        }
                                        else
                                        {
                                            _43380 = _37655;
                                            _43354 = _37643;
                                        }
                                        _19870 = _37654 + 24u;
                                    }
                                    _37646 = 1.0;
                                    _37644 = _37643;
                                }
                                else
                                {
                                    float _18947 = floor((in.i_radii.z - 1.0) * 0.5);
                                    float _19878 = floor(_18947 * 0.0625);
                                    uint _19895 = (((in.i_instance + 1u) + uint(_19878)) * 24u) + uint(_18947 - (_19878 * 16.0));
                                    float2 _37614;
                                    if (_18925)
                                    {
                                        float _19905 = floor(0.0);
                                        _37614 = gFxData_1._data[(((in.i_instance + 1u) + uint(_19905)) * 24u) + uint(_19905 * (-16.0))].xy;
                                    }
                                    else
                                    {
                                        _37614 = select(gFxData_1._data[_19895].zw, gFxData_1._data[_19895].xy, bool2(((in.i_radii.z - 1.0) - (_18947 * 2.0)) < 0.5));
                                    }
                                    float2 _37613;
                                    float _37616;
                                    float _37619;
                                    _37619 = 1.0;
                                    _37616 = 1000000015047466219876688855040.0;
                                    _37613 = _37614;
                                    float2 _37612;
                                    float _37615;
                                    float _37618;
                                    for (float _37609 = 0.0; _37609 < 16.0; _37619 = _37618, _37616 = _37615, _37613 = _37612, _37609 += 1.0)
                                    {
                                        if (_37609 >= _18933)
                                        {
                                            break;
                                        }
                                        _37618 = _37619;
                                        _37615 = _37616;
                                        _37612 = _37613;
                                        float2 _43344;
                                        float _43346;
                                        float _43348;
                                        for (float _37610 = 0.0; _37610 < 128.0; _37618 = _43348, _37615 = _43346, _37612 = _43344, _37610 += 1.0)
                                        {
                                            float _18989 = (_37609 * 128.0) + _37610;
                                            if (_18989 >= _18929)
                                            {
                                                break;
                                            }
                                            float _19932 = floor(_18989 * 0.0625);
                                            uint _19949 = (((in.i_instance + 1u) + uint(_19932)) * 24u) + uint(_18989 - (_19932 * 16.0));
                                            float2 _19962 = _37612 - gFxData_1._data[_19949].xy;
                                            float2 _19965 = _16953 - gFxData_1._data[_19949].xy;
                                            float2 _19978 = _19965 - (_19962 * fast::clamp(dot(_19965, _19962) / fast::max(dot(_19962, _19962), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _19981 = dot(_19978, _19978);
                                            float _43953 = (_19981 < _37615) ? _19981 : _37615;
                                            bool _19993 = _16953.y >= gFxData_1._data[_19949].y;
                                            bool _19998 = _16953.y < _37612.y;
                                            bool _20009 = (_19962.x * _19965.y) > (_19962.y * _19965.x);
                                            float _37624;
                                            if (((_19993 && _19998) && _20009) || (((!_19993) && (!_19998)) && (!_20009)))
                                            {
                                                _37624 = -_37618;
                                            }
                                            else
                                            {
                                                _37624 = _37618;
                                            }
                                            if (((2.0 * _18989) + 1.0) < in.i_radii.z)
                                            {
                                                float2 _20038 = gFxData_1._data[_19949].xy - gFxData_1._data[_19949].zw;
                                                float2 _20041 = _16953 - gFxData_1._data[_19949].zw;
                                                float2 _20054 = _20041 - (_20038 * fast::clamp(dot(_20041, _20038) / fast::max(dot(_20038, _20038), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _20057 = dot(_20054, _20054);
                                                bool _20069 = _16953.y >= gFxData_1._data[_19949].w;
                                                bool _20074 = _16953.y < gFxData_1._data[_19949].y;
                                                bool _20085 = (_20038.x * _20041.y) > (_20038.y * _20041.x);
                                                float _37628;
                                                if (((_20069 && _20074) && _20085) || (((!_20069) && (!_20074)) && (!_20085)))
                                                {
                                                    _37628 = -_37624;
                                                }
                                                else
                                                {
                                                    _37628 = _37624;
                                                }
                                                _43348 = _37628;
                                                _43346 = (_20057 < _43953) ? _20057 : _43953;
                                                _43344 = gFxData_1._data[_19949].zw;
                                            }
                                            else
                                            {
                                                _43348 = _37624;
                                                _43346 = _43953;
                                                _43344 = gFxData_1._data[_19949].xy;
                                            }
                                        }
                                    }
                                    _37646 = _37619;
                                    _37644 = _37616;
                                }
                                float _19042 = sqrt(_37644);
                                if (_18925)
                                {
                                    _37651 = _19042 - in.i_radii.x;
                                    break;
                                }
                                _37651 = (_37646 * _19042) - in.i_radii.x;
                                break;
                            } while(false);
                            _37700 = _37651;
                        }
                        else
                        {
                            float _37701;
                            if (in.i_flags.z == 1u)
                            {
                                float2 _18668 = _16953 - _18634;
                                float _37608;
                                do
                                {
                                    if (in.i_shape.w >= 6.282185077667236328125)
                                    {
                                        _37608 = abs(length(_18668) - in.i_radii.x) - in.i_radii.y;
                                        break;
                                    }
                                    float _20128 = in.i_shape.z + (in.i_shape.w * 0.5);
                                    float _20130 = cos(_20128);
                                    float _20132 = sin(_20128);
                                    float _20141 = dot(_18668, float2(-_20132, _20130));
                                    float _20144 = dot(_18668, float2(_20130, _20132));
                                    float2 _20145 = float2(_20141, _20144);
                                    float _20148 = abs(_20141);
                                    _20145.x = _20148;
                                    float _20151 = in.i_shape.w * 0.5;
                                    float _20153 = sin(_20151);
                                    float _20155 = cos(_20151);
                                    _37608 = (((_20155 * _20148) > (_20153 * _20144)) ? length(_20145 - (float2(_20153, _20155) * in.i_radii.x)) : abs(length(_20145) - in.i_radii.x)) - in.i_radii.y;
                                    break;
                                } while(false);
                                _37701 = _37608;
                            }
                            else
                            {
                                float _37702;
                                if (in.i_flags.z == 2u)
                                {
                                    float2 _20191 = _16953 - _36247.xy;
                                    float2 _20194 = _36247.zw - _36247.xy;
                                    _37702 = length(_20191 - (_20194 * fast::clamp(dot(_20191, _20194) / fast::max(dot(_20194, _20194), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                                }
                                else
                                {
                                    float2 _18695 = _16953 - _18634;
                                    float _20259 = fast::min(_18643.x, _18643.y);
                                    float _20262 = fast::min((_18695.x > 0.0) ? ((_18695.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_18695.y > 0.0) ? in.i_radii.w : in.i_radii.x), _20259);
                                    float _20268 = _20262 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                    float _37597;
                                    float _37598;
                                    if (_20268 > _20259)
                                    {
                                        float _20282 = in.i_shape.y * fast::clamp((_20259 - _20262) / fast::max(0.60000002384185791015625 * _20262, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                        _37598 = _20282;
                                        _37597 = _20262 * (1.0 + (0.60000002384185791015625 * _20282));
                                    }
                                    else
                                    {
                                        _37598 = in.i_shape.y;
                                        _37597 = _20268;
                                    }
                                    float2 _20311 = (abs(_18695) - _18643) + float2(_37597);
                                    float2 _20313 = fast::max(_20311, float2(0.0));
                                    float _37599;
                                    if ((_20313.x > 0.0) && (_20313.y > 0.0))
                                    {
                                        float _37600;
                                        if ((_37598 > 0.001000000047497451305389404296875) && (_37597 > 9.9999997473787516355514526367188e-05))
                                        {
                                            float _20330 = 2.0 + (2.0 * _37598);
                                            float2 _20335 = _20313 / float2(fast::max(_37597, 9.9999997473787516355514526367188e-05));
                                            _37600 = pow(pow(_20335.x, _20330) + pow(_20335.y, _20330), 1.0 / _20330) * _37597;
                                        }
                                        else
                                        {
                                            _37600 = length(_20313);
                                        }
                                        _37599 = _37600;
                                    }
                                    else
                                    {
                                        _37599 = fast::max(_20313.x, _20313.y);
                                    }
                                    float _20370 = (fast::min(fast::max(_20311.x, _20311.y), 0.0) + _37599) - _37597;
                                    float _37703;
                                    if ((in.i_flags.x & _2278) != 0u)
                                    {
                                        float2 _18723 = fast::max((_36247.zw - _36247.xy) * 0.5, float2(0.001000000047497451305389404296875));
                                        float2 _18726 = _16953 - ((_36247.xy + _36247.zw) * 0.5);
                                        float _20418 = fast::min(_18723.x, _18723.y);
                                        float _20421 = fast::min((_18726.x > 0.0) ? ((_18726.y > 0.0) ? gFxData_1._data[_16311].x : gFxData_1._data[_16311].x) : ((_18726.y > 0.0) ? gFxData_1._data[_16311].x : gFxData_1._data[_16311].x), _20418);
                                        float _20427 = _20421 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                        float _37601;
                                        float _37602;
                                        if (_20427 > _20418)
                                        {
                                            float _20441 = in.i_shape.y * fast::clamp((_20418 - _20421) / fast::max(0.60000002384185791015625 * _20421, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                            _37602 = _20441;
                                            _37601 = _20421 * (1.0 + (0.60000002384185791015625 * _20441));
                                        }
                                        else
                                        {
                                            _37602 = in.i_shape.y;
                                            _37601 = _20427;
                                        }
                                        float2 _20470 = (abs(_18726) - _18723) + float2(_37601);
                                        float2 _20472 = fast::max(_20470, float2(0.0));
                                        float _37603;
                                        if ((_20472.x > 0.0) && (_20472.y > 0.0))
                                        {
                                            float _37604;
                                            if ((_37602 > 0.001000000047497451305389404296875) && (_37601 > 9.9999997473787516355514526367188e-05))
                                            {
                                                float _20489 = 2.0 + (2.0 * _37602);
                                                float2 _20494 = _20472 / float2(fast::max(_37601, 9.9999997473787516355514526367188e-05));
                                                _37604 = pow(pow(_20494.x, _20489) + pow(_20494.y, _20489), 1.0 / _20489) * _37601;
                                            }
                                            else
                                            {
                                                _37604 = length(_20472);
                                            }
                                            _37603 = _37604;
                                        }
                                        else
                                        {
                                            _37603 = fast::max(_20472.x, _20472.y);
                                        }
                                        float _20529 = (fast::min(fast::max(_20470.x, _20470.y), 0.0) + _37603) - _37601;
                                        float _20534 = fast::max(gFxData_1._data[_16311].y, 9.9999997473787516355514526367188e-05);
                                        float _20543 = fast::max(_20534 - abs(_20370 - _20529), 0.0) / _20534;
                                        _37703 = fast::min(_20370, _20529) - (((_20543 * _20543) * _20534) * 0.25);
                                    }
                                    else
                                    {
                                        _37703 = _20370;
                                    }
                                    _37702 = _37703;
                                }
                                _37701 = _37702;
                            }
                            _37700 = _37701;
                        }
                        _37699 = _37700;
                    }
                    float2 _16957 = in.i_local - float2(0.5, 0.0);
                    float2 _20598 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
                    float2 _20607 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float _37806;
                    if ((in.i_flags.x & _2192) != 0u)
                    {
                        float _20724 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                        float4 _20758 = gTex.sample(gLinear, mix(_36241.xy, _36241.zw, (fast::clamp(_16957, _36247.xy, _36247.zw) - _36247.xy) / fast::max(_36247.zw - _36247.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                        float _20782 = (_20758.w - 0.5) * in.i_radii.y;
                        float2 _20823 = abs(_16957 - ((_36247.xy + _36247.zw) * 0.5)) - fast::max(((_36247.zw - _36247.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _20724), float2(0.0));
                        _37806 = fast::max((-mix((fast::max(fast::min(_20758.x, _20758.y), fast::min(fast::max(_20758.x, _20758.y), _20758.z)) - 0.5) * in.i_radii.x, _20782, fast::clamp(abs(_20782) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _20724, length(fast::max(_20823, float2(0.0))) + fast::min(fast::max(_20823.x, _20823.y), 0.0)) - in.i_shape.w;
                    }
                    else
                    {
                        float _37807;
                        if ((in.i_flags.x & _2207) != 0u)
                        {
                            float _37758;
                            do
                            {
                                bool _20889 = in.i_radii.w > 0.5;
                                float _20893 = floor((in.i_radii.z + 1.0) * 0.5);
                                float _20897 = floor((_20893 + 127.0) * 0.0078125);
                                float _37751;
                                float _37753;
                                if (_20889)
                                {
                                    uint _21073 = uint(in.i_radii.z);
                                    uint _21076 = (_21073 + 31u) >> 5u;
                                    uint _21159 = (in.i_instance + 1u) * 24u;
                                    uint _37743;
                                    uint _37744;
                                    _37744 = 0u;
                                    _37743 = _21159;
                                    uint _21186;
                                    float _43955;
                                    uint _43956;
                                    uint _43957;
                                    uint _37742 = 0u;
                                    uint _37802 = _21159;
                                    float _37803 = 3.0000000054977557577780399428115e+38;
                                    for (; _37742 < _21076; _37803 = _43955, _37802 = _21186, _37744 = _43956, _37743 = _43957, _37742++)
                                    {
                                        uint _21164 = _37802 + 20u;
                                        float2 _21179 = fast::max(fast::max(gFxData_1._data[_21164].xy - _16957, _16957 - gFxData_1._data[_21164].zw), float2(0.0));
                                        float _21182 = dot(_21179, _21179);
                                        bool _21094 = _21182 < _37803;
                                        _43955 = _21094 ? _21182 : _37803;
                                        _43956 = _21094 ? _37742 : _37744;
                                        _43957 = _21094 ? _37802 : _37743;
                                        _21186 = _37802 + 24u;
                                    }
                                    uint _21224 = _37744 * 32u;
                                    uint _21228 = min((_21073 - _21224), 32u);
                                    uint _21332 = _37743 + 21u;
                                    uint _21339 = _37743 + 22u;
                                    float _37747;
                                    uint _37748;
                                    _37748 = 4294967295u;
                                    _37747 = 1000000015047466219876688855040.0;
                                    float _43414;
                                    uint _43415;
                                    for (uint _37746 = 0u; (_37746 * 8u) < _21228; _37748 = _43415, _37747 = _43414, _37746++)
                                    {
                                        uint _21346 = _37743 + (16u + _37746);
                                        float _21366 = ((sqrt(_37747) * 1.000003814697265625) + 0.03125) + ((abs(_16957.x) + abs(_16957.y)) * 3.814697265625e-06);
                                        float2 _21386 = fast::max(fast::max(gFxData_1._data[_21346].xy - _16957, _16957 - gFxData_1._data[_21346].zw), float2(0.0));
                                        if (dot(_21386, _21386) > (_21366 * _21366))
                                        {
                                            _43415 = _37748;
                                            _43414 = _37747;
                                            continue;
                                        }
                                        float4 _21254 = select(gFxData_1._data[_21339], gFxData_1._data[_21332], bool4(_37746 < 2u));
                                        uint _21268 = min(_21228, ((_37746 * 8u) + 8u));
                                        uint _21270 = _37746 * 8u;
                                        float2 _37787;
                                        float _37788;
                                        uint _37789;
                                        _37789 = _37748;
                                        _37788 = _37747;
                                        _37787 = select(_21254.xy, _21254.zw, bool2((_37746 & 1u) != 0u));
                                        float2 _43458;
                                        float _43459;
                                        uint _43460;
                                        for (uint _37786 = _21270; _37786 < _21268; _37789 = _43460, _37788 = _43459, _37787 = _43458, _37786 += 2u)
                                        {
                                            uint _21394 = _37743 + (_37786 >> 1u);
                                            uint _21283 = _21224 + _37786;
                                            float2 _21404 = _37787 - gFxData_1._data[_21394].xy;
                                            float2 _21407 = _16957 - gFxData_1._data[_21394].xy;
                                            float2 _21420 = _21407 - (_21404 * fast::clamp(dot(_21407, _21404) / fast::max(dot(_21404, _21404), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _21423 = dot(_21420, _21420);
                                            bool _21434 = (_21423 < _37788) || ((_21423 == _37788) && (_21283 < _37789));
                                            uint _43958 = _21434 ? _21283 : _37789;
                                            float _43959 = _21434 ? _21423 : _37788;
                                            if ((_37786 + 1u) < _21268)
                                            {
                                                uint _21305 = (_21224 + _37786) + 1u;
                                                float2 _21447 = gFxData_1._data[_21394].xy - gFxData_1._data[_21394].zw;
                                                float2 _21450 = _16957 - gFxData_1._data[_21394].zw;
                                                float2 _21463 = _21450 - (_21447 * fast::clamp(dot(_21450, _21447) / fast::max(dot(_21447, _21447), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _21466 = dot(_21463, _21463);
                                                bool _21477 = (_21466 < _43959) || ((_21466 == _43959) && (_21305 < _43958));
                                                _43460 = _21477 ? _21305 : _43958;
                                                _43459 = _21477 ? _21466 : _43959;
                                                _43458 = gFxData_1._data[_21394].zw;
                                            }
                                            else
                                            {
                                                _43460 = _43958;
                                                _43459 = _43959;
                                                _43458 = gFxData_1._data[_21394].xy;
                                            }
                                        }
                                        _43415 = _37789;
                                        _43414 = _37788;
                                    }
                                    uint _21486 = (in.i_instance + 1u) * 24u;
                                    float _37750;
                                    _37750 = _37747;
                                    uint _21834;
                                    float _43416;
                                    uint _43442;
                                    for (uint _37749 = 0u, _37761 = _21486, _37762 = _37748; _37749 < _21076; _37762 = _43442, _37761 = _21834, _37750 = _43416, _37749++)
                                    {
                                        uint _21491 = _37761 + 20u;
                                        float _21511 = ((sqrt(_37750) * 1.000003814697265625) + 0.03125) + ((abs(_16957.x) + abs(_16957.y)) * 3.814697265625e-06);
                                        float2 _21531 = fast::max(fast::max(gFxData_1._data[_21491].xy - _16957, _16957 - gFxData_1._data[_21491].zw), float2(0.0));
                                        if ((_37749 != _37744) && (!(dot(_21531, _21531) > (_21511 * _21511))))
                                        {
                                            uint _21572 = _37749 * 32u;
                                            uint _21576 = min((_21073 - _21572), 32u);
                                            uint _21680 = _37761 + 21u;
                                            uint _21687 = _37761 + 22u;
                                            float _37764;
                                            uint _37765;
                                            _37765 = _37762;
                                            _37764 = _37750;
                                            float _43443;
                                            uint _43444;
                                            for (uint _37763 = 0u; (_37763 * 8u) < _21576; _37765 = _43444, _37764 = _43443, _37763++)
                                            {
                                                uint _21694 = _37761 + (16u + _37763);
                                                float _21714 = ((sqrt(_37764) * 1.000003814697265625) + 0.03125) + ((abs(_16957.x) + abs(_16957.y)) * 3.814697265625e-06);
                                                float2 _21734 = fast::max(fast::max(gFxData_1._data[_21694].xy - _16957, _16957 - gFxData_1._data[_21694].zw), float2(0.0));
                                                if (dot(_21734, _21734) > (_21714 * _21714))
                                                {
                                                    _43444 = _37765;
                                                    _43443 = _37764;
                                                    continue;
                                                }
                                                float4 _21602 = select(gFxData_1._data[_21687], gFxData_1._data[_21680], bool4(_37763 < 2u));
                                                uint _21616 = min(_21576, ((_37763 * 8u) + 8u));
                                                uint _21618 = _37763 * 8u;
                                                float2 _37771;
                                                float _37772;
                                                uint _37773;
                                                _37773 = _37765;
                                                _37772 = _37764;
                                                _37771 = select(_21602.xy, _21602.zw, bool2((_37763 & 1u) != 0u));
                                                float2 _43455;
                                                float _43456;
                                                uint _43457;
                                                for (uint _37770 = _21618; _37770 < _21616; _37773 = _43457, _37772 = _43456, _37771 = _43455, _37770 += 2u)
                                                {
                                                    uint _21742 = _37761 + (_37770 >> 1u);
                                                    uint _21631 = _21572 + _37770;
                                                    float2 _21752 = _37771 - gFxData_1._data[_21742].xy;
                                                    float2 _21755 = _16957 - gFxData_1._data[_21742].xy;
                                                    float2 _21768 = _21755 - (_21752 * fast::clamp(dot(_21755, _21752) / fast::max(dot(_21752, _21752), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                    float _21771 = dot(_21768, _21768);
                                                    bool _21782 = (_21771 < _37772) || ((_21771 == _37772) && (_21631 < _37773));
                                                    uint _43962 = _21782 ? _21631 : _37773;
                                                    float _43963 = _21782 ? _21771 : _37772;
                                                    if ((_37770 + 1u) < _21616)
                                                    {
                                                        uint _21653 = (_21572 + _37770) + 1u;
                                                        float2 _21795 = gFxData_1._data[_21742].xy - gFxData_1._data[_21742].zw;
                                                        float2 _21798 = _16957 - gFxData_1._data[_21742].zw;
                                                        float2 _21811 = _21798 - (_21795 * fast::clamp(dot(_21798, _21795) / fast::max(dot(_21795, _21795), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                        float _21814 = dot(_21811, _21811);
                                                        bool _21825 = (_21814 < _43963) || ((_21814 == _43963) && (_21653 < _43962));
                                                        _43457 = _21825 ? _21653 : _43962;
                                                        _43456 = _21825 ? _21814 : _43963;
                                                        _43455 = gFxData_1._data[_21742].zw;
                                                    }
                                                    else
                                                    {
                                                        _43457 = _43962;
                                                        _43456 = _43963;
                                                        _43455 = gFxData_1._data[_21742].xy;
                                                    }
                                                }
                                                _43444 = _37773;
                                                _43443 = _37772;
                                            }
                                            _43442 = _37765;
                                            _43416 = _37764;
                                        }
                                        else
                                        {
                                            _43442 = _37762;
                                            _43416 = _37750;
                                        }
                                        _21834 = _37761 + 24u;
                                    }
                                    _37753 = 1.0;
                                    _37751 = _37750;
                                }
                                else
                                {
                                    float _20911 = floor((in.i_radii.z - 1.0) * 0.5);
                                    float _21842 = floor(_20911 * 0.0625);
                                    uint _21859 = (((in.i_instance + 1u) + uint(_21842)) * 24u) + uint(_20911 - (_21842 * 16.0));
                                    float2 _37721;
                                    if (_20889)
                                    {
                                        float _21869 = floor(0.0);
                                        _37721 = gFxData_1._data[(((in.i_instance + 1u) + uint(_21869)) * 24u) + uint(_21869 * (-16.0))].xy;
                                    }
                                    else
                                    {
                                        _37721 = select(gFxData_1._data[_21859].zw, gFxData_1._data[_21859].xy, bool2(((in.i_radii.z - 1.0) - (_20911 * 2.0)) < 0.5));
                                    }
                                    float2 _37720;
                                    float _37723;
                                    float _37726;
                                    _37726 = 1.0;
                                    _37723 = 1000000015047466219876688855040.0;
                                    _37720 = _37721;
                                    float2 _37719;
                                    float _37722;
                                    float _37725;
                                    for (float _37716 = 0.0; _37716 < 16.0; _37726 = _37725, _37723 = _37722, _37720 = _37719, _37716 += 1.0)
                                    {
                                        if (_37716 >= _20897)
                                        {
                                            break;
                                        }
                                        _37725 = _37726;
                                        _37722 = _37723;
                                        _37719 = _37720;
                                        float2 _43406;
                                        float _43408;
                                        float _43410;
                                        for (float _37717 = 0.0; _37717 < 128.0; _37725 = _43410, _37722 = _43408, _37719 = _43406, _37717 += 1.0)
                                        {
                                            float _20953 = (_37716 * 128.0) + _37717;
                                            if (_20953 >= _20893)
                                            {
                                                break;
                                            }
                                            float _21896 = floor(_20953 * 0.0625);
                                            uint _21913 = (((in.i_instance + 1u) + uint(_21896)) * 24u) + uint(_20953 - (_21896 * 16.0));
                                            float2 _21926 = _37719 - gFxData_1._data[_21913].xy;
                                            float2 _21929 = _16957 - gFxData_1._data[_21913].xy;
                                            float2 _21942 = _21929 - (_21926 * fast::clamp(dot(_21929, _21926) / fast::max(dot(_21926, _21926), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _21945 = dot(_21942, _21942);
                                            float _43966 = (_21945 < _37722) ? _21945 : _37722;
                                            bool _21957 = _16957.y >= gFxData_1._data[_21913].y;
                                            bool _21962 = _16957.y < _37719.y;
                                            bool _21973 = (_21926.x * _21929.y) > (_21926.y * _21929.x);
                                            float _37731;
                                            if (((_21957 && _21962) && _21973) || (((!_21957) && (!_21962)) && (!_21973)))
                                            {
                                                _37731 = -_37725;
                                            }
                                            else
                                            {
                                                _37731 = _37725;
                                            }
                                            if (((2.0 * _20953) + 1.0) < in.i_radii.z)
                                            {
                                                float2 _22002 = gFxData_1._data[_21913].xy - gFxData_1._data[_21913].zw;
                                                float2 _22005 = _16957 - gFxData_1._data[_21913].zw;
                                                float2 _22018 = _22005 - (_22002 * fast::clamp(dot(_22005, _22002) / fast::max(dot(_22002, _22002), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _22021 = dot(_22018, _22018);
                                                bool _22033 = _16957.y >= gFxData_1._data[_21913].w;
                                                bool _22038 = _16957.y < gFxData_1._data[_21913].y;
                                                bool _22049 = (_22002.x * _22005.y) > (_22002.y * _22005.x);
                                                float _37735;
                                                if (((_22033 && _22038) && _22049) || (((!_22033) && (!_22038)) && (!_22049)))
                                                {
                                                    _37735 = -_37731;
                                                }
                                                else
                                                {
                                                    _37735 = _37731;
                                                }
                                                _43410 = _37735;
                                                _43408 = (_22021 < _43966) ? _22021 : _43966;
                                                _43406 = gFxData_1._data[_21913].zw;
                                            }
                                            else
                                            {
                                                _43410 = _37731;
                                                _43408 = _43966;
                                                _43406 = gFxData_1._data[_21913].xy;
                                            }
                                        }
                                    }
                                    _37753 = _37726;
                                    _37751 = _37723;
                                }
                                float _21006 = sqrt(_37751);
                                if (_20889)
                                {
                                    _37758 = _21006 - in.i_radii.x;
                                    break;
                                }
                                _37758 = (_37753 * _21006) - in.i_radii.x;
                                break;
                            } while(false);
                            _37807 = _37758;
                        }
                        else
                        {
                            float _37808;
                            if (in.i_flags.z == 1u)
                            {
                                float2 _20632 = _16957 - _20598;
                                float _37715;
                                do
                                {
                                    if (in.i_shape.w >= 6.282185077667236328125)
                                    {
                                        _37715 = abs(length(_20632) - in.i_radii.x) - in.i_radii.y;
                                        break;
                                    }
                                    float _22092 = in.i_shape.z + (in.i_shape.w * 0.5);
                                    float _22094 = cos(_22092);
                                    float _22096 = sin(_22092);
                                    float _22105 = dot(_20632, float2(-_22096, _22094));
                                    float _22108 = dot(_20632, float2(_22094, _22096));
                                    float2 _22109 = float2(_22105, _22108);
                                    float _22112 = abs(_22105);
                                    _22109.x = _22112;
                                    float _22115 = in.i_shape.w * 0.5;
                                    float _22117 = sin(_22115);
                                    float _22119 = cos(_22115);
                                    _37715 = (((_22119 * _22112) > (_22117 * _22108)) ? length(_22109 - (float2(_22117, _22119) * in.i_radii.x)) : abs(length(_22109) - in.i_radii.x)) - in.i_radii.y;
                                    break;
                                } while(false);
                                _37808 = _37715;
                            }
                            else
                            {
                                float _37809;
                                if (in.i_flags.z == 2u)
                                {
                                    float2 _22155 = _16957 - _36247.xy;
                                    float2 _22158 = _36247.zw - _36247.xy;
                                    _37809 = length(_22155 - (_22158 * fast::clamp(dot(_22155, _22158) / fast::max(dot(_22158, _22158), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                                }
                                else
                                {
                                    float2 _20659 = _16957 - _20598;
                                    float _22223 = fast::min(_20607.x, _20607.y);
                                    float _22226 = fast::min((_20659.x > 0.0) ? ((_20659.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_20659.y > 0.0) ? in.i_radii.w : in.i_radii.x), _22223);
                                    float _22232 = _22226 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                    float _37704;
                                    float _37705;
                                    if (_22232 > _22223)
                                    {
                                        float _22246 = in.i_shape.y * fast::clamp((_22223 - _22226) / fast::max(0.60000002384185791015625 * _22226, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                        _37705 = _22246;
                                        _37704 = _22226 * (1.0 + (0.60000002384185791015625 * _22246));
                                    }
                                    else
                                    {
                                        _37705 = in.i_shape.y;
                                        _37704 = _22232;
                                    }
                                    float2 _22275 = (abs(_20659) - _20607) + float2(_37704);
                                    float2 _22277 = fast::max(_22275, float2(0.0));
                                    float _37706;
                                    if ((_22277.x > 0.0) && (_22277.y > 0.0))
                                    {
                                        float _37707;
                                        if ((_37705 > 0.001000000047497451305389404296875) && (_37704 > 9.9999997473787516355514526367188e-05))
                                        {
                                            float _22294 = 2.0 + (2.0 * _37705);
                                            float2 _22299 = _22277 / float2(fast::max(_37704, 9.9999997473787516355514526367188e-05));
                                            _37707 = pow(pow(_22299.x, _22294) + pow(_22299.y, _22294), 1.0 / _22294) * _37704;
                                        }
                                        else
                                        {
                                            _37707 = length(_22277);
                                        }
                                        _37706 = _37707;
                                    }
                                    else
                                    {
                                        _37706 = fast::max(_22277.x, _22277.y);
                                    }
                                    float _22334 = (fast::min(fast::max(_22275.x, _22275.y), 0.0) + _37706) - _37704;
                                    float _37810;
                                    if ((in.i_flags.x & _2278) != 0u)
                                    {
                                        float2 _20687 = fast::max((_36247.zw - _36247.xy) * 0.5, float2(0.001000000047497451305389404296875));
                                        float2 _20690 = _16957 - ((_36247.xy + _36247.zw) * 0.5);
                                        float _22382 = fast::min(_20687.x, _20687.y);
                                        float _22385 = fast::min((_20690.x > 0.0) ? ((_20690.y > 0.0) ? gFxData_1._data[_16311].x : gFxData_1._data[_16311].x) : ((_20690.y > 0.0) ? gFxData_1._data[_16311].x : gFxData_1._data[_16311].x), _22382);
                                        float _22391 = _22385 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                        float _37708;
                                        float _37709;
                                        if (_22391 > _22382)
                                        {
                                            float _22405 = in.i_shape.y * fast::clamp((_22382 - _22385) / fast::max(0.60000002384185791015625 * _22385, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                            _37709 = _22405;
                                            _37708 = _22385 * (1.0 + (0.60000002384185791015625 * _22405));
                                        }
                                        else
                                        {
                                            _37709 = in.i_shape.y;
                                            _37708 = _22391;
                                        }
                                        float2 _22434 = (abs(_20690) - _20687) + float2(_37708);
                                        float2 _22436 = fast::max(_22434, float2(0.0));
                                        float _37710;
                                        if ((_22436.x > 0.0) && (_22436.y > 0.0))
                                        {
                                            float _37711;
                                            if ((_37709 > 0.001000000047497451305389404296875) && (_37708 > 9.9999997473787516355514526367188e-05))
                                            {
                                                float _22453 = 2.0 + (2.0 * _37709);
                                                float2 _22458 = _22436 / float2(fast::max(_37708, 9.9999997473787516355514526367188e-05));
                                                _37711 = pow(pow(_22458.x, _22453) + pow(_22458.y, _22453), 1.0 / _22453) * _37708;
                                            }
                                            else
                                            {
                                                _37711 = length(_22436);
                                            }
                                            _37710 = _37711;
                                        }
                                        else
                                        {
                                            _37710 = fast::max(_22436.x, _22436.y);
                                        }
                                        float _22493 = (fast::min(fast::max(_22434.x, _22434.y), 0.0) + _37710) - _37708;
                                        float _22498 = fast::max(gFxData_1._data[_16311].y, 9.9999997473787516355514526367188e-05);
                                        float _22507 = fast::max(_22498 - abs(_22334 - _22493), 0.0) / _22498;
                                        _37810 = fast::min(_22334, _22493) - (((_22507 * _22507) * _22498) * 0.25);
                                    }
                                    else
                                    {
                                        _37810 = _22334;
                                    }
                                    _37809 = _37810;
                                }
                                _37808 = _37809;
                            }
                            _37807 = _37808;
                        }
                        _37806 = _37807;
                    }
                    float2 _16962 = in.i_local + float2(0.0, 0.5);
                    float2 _22562 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
                    float2 _22571 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float _37913;
                    if ((in.i_flags.x & _2192) != 0u)
                    {
                        float _22688 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                        float4 _22722 = gTex.sample(gLinear, mix(_36241.xy, _36241.zw, (fast::clamp(_16962, _36247.xy, _36247.zw) - _36247.xy) / fast::max(_36247.zw - _36247.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                        float _22746 = (_22722.w - 0.5) * in.i_radii.y;
                        float2 _22787 = abs(_16962 - ((_36247.xy + _36247.zw) * 0.5)) - fast::max(((_36247.zw - _36247.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _22688), float2(0.0));
                        _37913 = fast::max((-mix((fast::max(fast::min(_22722.x, _22722.y), fast::min(fast::max(_22722.x, _22722.y), _22722.z)) - 0.5) * in.i_radii.x, _22746, fast::clamp(abs(_22746) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _22688, length(fast::max(_22787, float2(0.0))) + fast::min(fast::max(_22787.x, _22787.y), 0.0)) - in.i_shape.w;
                    }
                    else
                    {
                        float _37914;
                        if ((in.i_flags.x & _2207) != 0u)
                        {
                            float _37865;
                            do
                            {
                                bool _22853 = in.i_radii.w > 0.5;
                                float _22857 = floor((in.i_radii.z + 1.0) * 0.5);
                                float _22861 = floor((_22857 + 127.0) * 0.0078125);
                                float _37858;
                                float _37860;
                                if (_22853)
                                {
                                    uint _23037 = uint(in.i_radii.z);
                                    uint _23040 = (_23037 + 31u) >> 5u;
                                    uint _23123 = (in.i_instance + 1u) * 24u;
                                    uint _37850;
                                    uint _37851;
                                    _37851 = 0u;
                                    _37850 = _23123;
                                    uint _23150;
                                    float _43968;
                                    uint _43969;
                                    uint _43970;
                                    uint _37849 = 0u;
                                    uint _37909 = _23123;
                                    float _37910 = 3.0000000054977557577780399428115e+38;
                                    for (; _37849 < _23040; _37910 = _43968, _37909 = _23150, _37851 = _43969, _37850 = _43970, _37849++)
                                    {
                                        uint _23128 = _37909 + 20u;
                                        float2 _23143 = fast::max(fast::max(gFxData_1._data[_23128].xy - _16962, _16962 - gFxData_1._data[_23128].zw), float2(0.0));
                                        float _23146 = dot(_23143, _23143);
                                        bool _23058 = _23146 < _37910;
                                        _43968 = _23058 ? _23146 : _37910;
                                        _43969 = _23058 ? _37849 : _37851;
                                        _43970 = _23058 ? _37909 : _37850;
                                        _23150 = _37909 + 24u;
                                    }
                                    uint _23188 = _37851 * 32u;
                                    uint _23192 = min((_23037 - _23188), 32u);
                                    uint _23296 = _37850 + 21u;
                                    uint _23303 = _37850 + 22u;
                                    float _37854;
                                    uint _37855;
                                    _37855 = 4294967295u;
                                    _37854 = 1000000015047466219876688855040.0;
                                    float _43476;
                                    uint _43477;
                                    for (uint _37853 = 0u; (_37853 * 8u) < _23192; _37855 = _43477, _37854 = _43476, _37853++)
                                    {
                                        uint _23310 = _37850 + (16u + _37853);
                                        float _23330 = ((sqrt(_37854) * 1.000003814697265625) + 0.03125) + ((abs(_16962.x) + abs(_16962.y)) * 3.814697265625e-06);
                                        float2 _23350 = fast::max(fast::max(gFxData_1._data[_23310].xy - _16962, _16962 - gFxData_1._data[_23310].zw), float2(0.0));
                                        if (dot(_23350, _23350) > (_23330 * _23330))
                                        {
                                            _43477 = _37855;
                                            _43476 = _37854;
                                            continue;
                                        }
                                        float4 _23218 = select(gFxData_1._data[_23303], gFxData_1._data[_23296], bool4(_37853 < 2u));
                                        uint _23232 = min(_23192, ((_37853 * 8u) + 8u));
                                        uint _23234 = _37853 * 8u;
                                        float2 _37894;
                                        float _37895;
                                        uint _37896;
                                        _37896 = _37855;
                                        _37895 = _37854;
                                        _37894 = select(_23218.xy, _23218.zw, bool2((_37853 & 1u) != 0u));
                                        float2 _43520;
                                        float _43521;
                                        uint _43522;
                                        for (uint _37893 = _23234; _37893 < _23232; _37896 = _43522, _37895 = _43521, _37894 = _43520, _37893 += 2u)
                                        {
                                            uint _23358 = _37850 + (_37893 >> 1u);
                                            uint _23247 = _23188 + _37893;
                                            float2 _23368 = _37894 - gFxData_1._data[_23358].xy;
                                            float2 _23371 = _16962 - gFxData_1._data[_23358].xy;
                                            float2 _23384 = _23371 - (_23368 * fast::clamp(dot(_23371, _23368) / fast::max(dot(_23368, _23368), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _23387 = dot(_23384, _23384);
                                            bool _23398 = (_23387 < _37895) || ((_23387 == _37895) && (_23247 < _37896));
                                            uint _43971 = _23398 ? _23247 : _37896;
                                            float _43972 = _23398 ? _23387 : _37895;
                                            if ((_37893 + 1u) < _23232)
                                            {
                                                uint _23269 = (_23188 + _37893) + 1u;
                                                float2 _23411 = gFxData_1._data[_23358].xy - gFxData_1._data[_23358].zw;
                                                float2 _23414 = _16962 - gFxData_1._data[_23358].zw;
                                                float2 _23427 = _23414 - (_23411 * fast::clamp(dot(_23414, _23411) / fast::max(dot(_23411, _23411), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _23430 = dot(_23427, _23427);
                                                bool _23441 = (_23430 < _43972) || ((_23430 == _43972) && (_23269 < _43971));
                                                _43522 = _23441 ? _23269 : _43971;
                                                _43521 = _23441 ? _23430 : _43972;
                                                _43520 = gFxData_1._data[_23358].zw;
                                            }
                                            else
                                            {
                                                _43522 = _43971;
                                                _43521 = _43972;
                                                _43520 = gFxData_1._data[_23358].xy;
                                            }
                                        }
                                        _43477 = _37896;
                                        _43476 = _37895;
                                    }
                                    uint _23450 = (in.i_instance + 1u) * 24u;
                                    float _37857;
                                    _37857 = _37854;
                                    uint _23798;
                                    float _43478;
                                    uint _43504;
                                    for (uint _37856 = 0u, _37868 = _23450, _37869 = _37855; _37856 < _23040; _37869 = _43504, _37868 = _23798, _37857 = _43478, _37856++)
                                    {
                                        uint _23455 = _37868 + 20u;
                                        float _23475 = ((sqrt(_37857) * 1.000003814697265625) + 0.03125) + ((abs(_16962.x) + abs(_16962.y)) * 3.814697265625e-06);
                                        float2 _23495 = fast::max(fast::max(gFxData_1._data[_23455].xy - _16962, _16962 - gFxData_1._data[_23455].zw), float2(0.0));
                                        if ((_37856 != _37851) && (!(dot(_23495, _23495) > (_23475 * _23475))))
                                        {
                                            uint _23536 = _37856 * 32u;
                                            uint _23540 = min((_23037 - _23536), 32u);
                                            uint _23644 = _37868 + 21u;
                                            uint _23651 = _37868 + 22u;
                                            float _37871;
                                            uint _37872;
                                            _37872 = _37869;
                                            _37871 = _37857;
                                            float _43505;
                                            uint _43506;
                                            for (uint _37870 = 0u; (_37870 * 8u) < _23540; _37872 = _43506, _37871 = _43505, _37870++)
                                            {
                                                uint _23658 = _37868 + (16u + _37870);
                                                float _23678 = ((sqrt(_37871) * 1.000003814697265625) + 0.03125) + ((abs(_16962.x) + abs(_16962.y)) * 3.814697265625e-06);
                                                float2 _23698 = fast::max(fast::max(gFxData_1._data[_23658].xy - _16962, _16962 - gFxData_1._data[_23658].zw), float2(0.0));
                                                if (dot(_23698, _23698) > (_23678 * _23678))
                                                {
                                                    _43506 = _37872;
                                                    _43505 = _37871;
                                                    continue;
                                                }
                                                float4 _23566 = select(gFxData_1._data[_23651], gFxData_1._data[_23644], bool4(_37870 < 2u));
                                                uint _23580 = min(_23540, ((_37870 * 8u) + 8u));
                                                uint _23582 = _37870 * 8u;
                                                float2 _37878;
                                                float _37879;
                                                uint _37880;
                                                _37880 = _37872;
                                                _37879 = _37871;
                                                _37878 = select(_23566.xy, _23566.zw, bool2((_37870 & 1u) != 0u));
                                                float2 _43517;
                                                float _43518;
                                                uint _43519;
                                                for (uint _37877 = _23582; _37877 < _23580; _37880 = _43519, _37879 = _43518, _37878 = _43517, _37877 += 2u)
                                                {
                                                    uint _23706 = _37868 + (_37877 >> 1u);
                                                    uint _23595 = _23536 + _37877;
                                                    float2 _23716 = _37878 - gFxData_1._data[_23706].xy;
                                                    float2 _23719 = _16962 - gFxData_1._data[_23706].xy;
                                                    float2 _23732 = _23719 - (_23716 * fast::clamp(dot(_23719, _23716) / fast::max(dot(_23716, _23716), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                    float _23735 = dot(_23732, _23732);
                                                    bool _23746 = (_23735 < _37879) || ((_23735 == _37879) && (_23595 < _37880));
                                                    uint _43975 = _23746 ? _23595 : _37880;
                                                    float _43976 = _23746 ? _23735 : _37879;
                                                    if ((_37877 + 1u) < _23580)
                                                    {
                                                        uint _23617 = (_23536 + _37877) + 1u;
                                                        float2 _23759 = gFxData_1._data[_23706].xy - gFxData_1._data[_23706].zw;
                                                        float2 _23762 = _16962 - gFxData_1._data[_23706].zw;
                                                        float2 _23775 = _23762 - (_23759 * fast::clamp(dot(_23762, _23759) / fast::max(dot(_23759, _23759), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                        float _23778 = dot(_23775, _23775);
                                                        bool _23789 = (_23778 < _43976) || ((_23778 == _43976) && (_23617 < _43975));
                                                        _43519 = _23789 ? _23617 : _43975;
                                                        _43518 = _23789 ? _23778 : _43976;
                                                        _43517 = gFxData_1._data[_23706].zw;
                                                    }
                                                    else
                                                    {
                                                        _43519 = _43975;
                                                        _43518 = _43976;
                                                        _43517 = gFxData_1._data[_23706].xy;
                                                    }
                                                }
                                                _43506 = _37880;
                                                _43505 = _37879;
                                            }
                                            _43504 = _37872;
                                            _43478 = _37871;
                                        }
                                        else
                                        {
                                            _43504 = _37869;
                                            _43478 = _37857;
                                        }
                                        _23798 = _37868 + 24u;
                                    }
                                    _37860 = 1.0;
                                    _37858 = _37857;
                                }
                                else
                                {
                                    float _22875 = floor((in.i_radii.z - 1.0) * 0.5);
                                    float _23806 = floor(_22875 * 0.0625);
                                    uint _23823 = (((in.i_instance + 1u) + uint(_23806)) * 24u) + uint(_22875 - (_23806 * 16.0));
                                    float2 _37828;
                                    if (_22853)
                                    {
                                        float _23833 = floor(0.0);
                                        _37828 = gFxData_1._data[(((in.i_instance + 1u) + uint(_23833)) * 24u) + uint(_23833 * (-16.0))].xy;
                                    }
                                    else
                                    {
                                        _37828 = select(gFxData_1._data[_23823].zw, gFxData_1._data[_23823].xy, bool2(((in.i_radii.z - 1.0) - (_22875 * 2.0)) < 0.5));
                                    }
                                    float2 _37827;
                                    float _37830;
                                    float _37833;
                                    _37833 = 1.0;
                                    _37830 = 1000000015047466219876688855040.0;
                                    _37827 = _37828;
                                    float2 _37826;
                                    float _37829;
                                    float _37832;
                                    for (float _37823 = 0.0; _37823 < 16.0; _37833 = _37832, _37830 = _37829, _37827 = _37826, _37823 += 1.0)
                                    {
                                        if (_37823 >= _22861)
                                        {
                                            break;
                                        }
                                        _37832 = _37833;
                                        _37829 = _37830;
                                        _37826 = _37827;
                                        float2 _43468;
                                        float _43470;
                                        float _43472;
                                        for (float _37824 = 0.0; _37824 < 128.0; _37832 = _43472, _37829 = _43470, _37826 = _43468, _37824 += 1.0)
                                        {
                                            float _22917 = (_37823 * 128.0) + _37824;
                                            if (_22917 >= _22857)
                                            {
                                                break;
                                            }
                                            float _23860 = floor(_22917 * 0.0625);
                                            uint _23877 = (((in.i_instance + 1u) + uint(_23860)) * 24u) + uint(_22917 - (_23860 * 16.0));
                                            float2 _23890 = _37826 - gFxData_1._data[_23877].xy;
                                            float2 _23893 = _16962 - gFxData_1._data[_23877].xy;
                                            float2 _23906 = _23893 - (_23890 * fast::clamp(dot(_23893, _23890) / fast::max(dot(_23890, _23890), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _23909 = dot(_23906, _23906);
                                            float _43979 = (_23909 < _37829) ? _23909 : _37829;
                                            bool _23921 = _16962.y >= gFxData_1._data[_23877].y;
                                            bool _23926 = _16962.y < _37826.y;
                                            bool _23937 = (_23890.x * _23893.y) > (_23890.y * _23893.x);
                                            float _37838;
                                            if (((_23921 && _23926) && _23937) || (((!_23921) && (!_23926)) && (!_23937)))
                                            {
                                                _37838 = -_37832;
                                            }
                                            else
                                            {
                                                _37838 = _37832;
                                            }
                                            if (((2.0 * _22917) + 1.0) < in.i_radii.z)
                                            {
                                                float2 _23966 = gFxData_1._data[_23877].xy - gFxData_1._data[_23877].zw;
                                                float2 _23969 = _16962 - gFxData_1._data[_23877].zw;
                                                float2 _23982 = _23969 - (_23966 * fast::clamp(dot(_23969, _23966) / fast::max(dot(_23966, _23966), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _23985 = dot(_23982, _23982);
                                                bool _23997 = _16962.y >= gFxData_1._data[_23877].w;
                                                bool _24002 = _16962.y < gFxData_1._data[_23877].y;
                                                bool _24013 = (_23966.x * _23969.y) > (_23966.y * _23969.x);
                                                float _37842;
                                                if (((_23997 && _24002) && _24013) || (((!_23997) && (!_24002)) && (!_24013)))
                                                {
                                                    _37842 = -_37838;
                                                }
                                                else
                                                {
                                                    _37842 = _37838;
                                                }
                                                _43472 = _37842;
                                                _43470 = (_23985 < _43979) ? _23985 : _43979;
                                                _43468 = gFxData_1._data[_23877].zw;
                                            }
                                            else
                                            {
                                                _43472 = _37838;
                                                _43470 = _43979;
                                                _43468 = gFxData_1._data[_23877].xy;
                                            }
                                        }
                                    }
                                    _37860 = _37833;
                                    _37858 = _37830;
                                }
                                float _22970 = sqrt(_37858);
                                if (_22853)
                                {
                                    _37865 = _22970 - in.i_radii.x;
                                    break;
                                }
                                _37865 = (_37860 * _22970) - in.i_radii.x;
                                break;
                            } while(false);
                            _37914 = _37865;
                        }
                        else
                        {
                            float _37915;
                            if (in.i_flags.z == 1u)
                            {
                                float2 _22596 = _16962 - _22562;
                                float _37822;
                                do
                                {
                                    if (in.i_shape.w >= 6.282185077667236328125)
                                    {
                                        _37822 = abs(length(_22596) - in.i_radii.x) - in.i_radii.y;
                                        break;
                                    }
                                    float _24056 = in.i_shape.z + (in.i_shape.w * 0.5);
                                    float _24058 = cos(_24056);
                                    float _24060 = sin(_24056);
                                    float _24069 = dot(_22596, float2(-_24060, _24058));
                                    float _24072 = dot(_22596, float2(_24058, _24060));
                                    float2 _24073 = float2(_24069, _24072);
                                    float _24076 = abs(_24069);
                                    _24073.x = _24076;
                                    float _24079 = in.i_shape.w * 0.5;
                                    float _24081 = sin(_24079);
                                    float _24083 = cos(_24079);
                                    _37822 = (((_24083 * _24076) > (_24081 * _24072)) ? length(_24073 - (float2(_24081, _24083) * in.i_radii.x)) : abs(length(_24073) - in.i_radii.x)) - in.i_radii.y;
                                    break;
                                } while(false);
                                _37915 = _37822;
                            }
                            else
                            {
                                float _37916;
                                if (in.i_flags.z == 2u)
                                {
                                    float2 _24119 = _16962 - _36247.xy;
                                    float2 _24122 = _36247.zw - _36247.xy;
                                    _37916 = length(_24119 - (_24122 * fast::clamp(dot(_24119, _24122) / fast::max(dot(_24122, _24122), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                                }
                                else
                                {
                                    float2 _22623 = _16962 - _22562;
                                    float _24187 = fast::min(_22571.x, _22571.y);
                                    float _24190 = fast::min((_22623.x > 0.0) ? ((_22623.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_22623.y > 0.0) ? in.i_radii.w : in.i_radii.x), _24187);
                                    float _24196 = _24190 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                    float _37811;
                                    float _37812;
                                    if (_24196 > _24187)
                                    {
                                        float _24210 = in.i_shape.y * fast::clamp((_24187 - _24190) / fast::max(0.60000002384185791015625 * _24190, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                        _37812 = _24210;
                                        _37811 = _24190 * (1.0 + (0.60000002384185791015625 * _24210));
                                    }
                                    else
                                    {
                                        _37812 = in.i_shape.y;
                                        _37811 = _24196;
                                    }
                                    float2 _24239 = (abs(_22623) - _22571) + float2(_37811);
                                    float2 _24241 = fast::max(_24239, float2(0.0));
                                    float _37813;
                                    if ((_24241.x > 0.0) && (_24241.y > 0.0))
                                    {
                                        float _37814;
                                        if ((_37812 > 0.001000000047497451305389404296875) && (_37811 > 9.9999997473787516355514526367188e-05))
                                        {
                                            float _24258 = 2.0 + (2.0 * _37812);
                                            float2 _24263 = _24241 / float2(fast::max(_37811, 9.9999997473787516355514526367188e-05));
                                            _37814 = pow(pow(_24263.x, _24258) + pow(_24263.y, _24258), 1.0 / _24258) * _37811;
                                        }
                                        else
                                        {
                                            _37814 = length(_24241);
                                        }
                                        _37813 = _37814;
                                    }
                                    else
                                    {
                                        _37813 = fast::max(_24241.x, _24241.y);
                                    }
                                    float _24298 = (fast::min(fast::max(_24239.x, _24239.y), 0.0) + _37813) - _37811;
                                    float _37917;
                                    if ((in.i_flags.x & _2278) != 0u)
                                    {
                                        float2 _22651 = fast::max((_36247.zw - _36247.xy) * 0.5, float2(0.001000000047497451305389404296875));
                                        float2 _22654 = _16962 - ((_36247.xy + _36247.zw) * 0.5);
                                        float _24346 = fast::min(_22651.x, _22651.y);
                                        float _24349 = fast::min((_22654.x > 0.0) ? ((_22654.y > 0.0) ? gFxData_1._data[_16311].x : gFxData_1._data[_16311].x) : ((_22654.y > 0.0) ? gFxData_1._data[_16311].x : gFxData_1._data[_16311].x), _24346);
                                        float _24355 = _24349 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                        float _37815;
                                        float _37816;
                                        if (_24355 > _24346)
                                        {
                                            float _24369 = in.i_shape.y * fast::clamp((_24346 - _24349) / fast::max(0.60000002384185791015625 * _24349, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                            _37816 = _24369;
                                            _37815 = _24349 * (1.0 + (0.60000002384185791015625 * _24369));
                                        }
                                        else
                                        {
                                            _37816 = in.i_shape.y;
                                            _37815 = _24355;
                                        }
                                        float2 _24398 = (abs(_22654) - _22651) + float2(_37815);
                                        float2 _24400 = fast::max(_24398, float2(0.0));
                                        float _37817;
                                        if ((_24400.x > 0.0) && (_24400.y > 0.0))
                                        {
                                            float _37818;
                                            if ((_37816 > 0.001000000047497451305389404296875) && (_37815 > 9.9999997473787516355514526367188e-05))
                                            {
                                                float _24417 = 2.0 + (2.0 * _37816);
                                                float2 _24422 = _24400 / float2(fast::max(_37815, 9.9999997473787516355514526367188e-05));
                                                _37818 = pow(pow(_24422.x, _24417) + pow(_24422.y, _24417), 1.0 / _24417) * _37815;
                                            }
                                            else
                                            {
                                                _37818 = length(_24400);
                                            }
                                            _37817 = _37818;
                                        }
                                        else
                                        {
                                            _37817 = fast::max(_24400.x, _24400.y);
                                        }
                                        float _24457 = (fast::min(fast::max(_24398.x, _24398.y), 0.0) + _37817) - _37815;
                                        float _24462 = fast::max(gFxData_1._data[_16311].y, 9.9999997473787516355514526367188e-05);
                                        float _24471 = fast::max(_24462 - abs(_24298 - _24457), 0.0) / _24462;
                                        _37917 = fast::min(_24298, _24457) - (((_24471 * _24471) * _24462) * 0.25);
                                    }
                                    else
                                    {
                                        _37917 = _24298;
                                    }
                                    _37916 = _37917;
                                }
                                _37915 = _37916;
                            }
                            _37914 = _37915;
                        }
                        _37913 = _37914;
                    }
                    float2 _16966 = in.i_local - float2(0.0, 0.5);
                    float2 _24526 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
                    float2 _24535 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                    float _38020;
                    if ((in.i_flags.x & _2192) != 0u)
                    {
                        float _24652 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
                        float4 _24686 = gTex.sample(gLinear, mix(_36241.xy, _36241.zw, (fast::clamp(_16966, _36247.xy, _36247.zw) - _36247.xy) / fast::max(_36247.zw - _36247.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
                        float _24710 = (_24686.w - 0.5) * in.i_radii.y;
                        float2 _24751 = abs(_16966 - ((_36247.xy + _36247.zw) * 0.5)) - fast::max(((_36247.zw - _36247.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _24652), float2(0.0));
                        _38020 = fast::max((-mix((fast::max(fast::min(_24686.x, _24686.y), fast::min(fast::max(_24686.x, _24686.y), _24686.z)) - 0.5) * in.i_radii.x, _24710, fast::clamp(abs(_24710) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _24652, length(fast::max(_24751, float2(0.0))) + fast::min(fast::max(_24751.x, _24751.y), 0.0)) - in.i_shape.w;
                    }
                    else
                    {
                        float _38021;
                        if ((in.i_flags.x & _2207) != 0u)
                        {
                            float _37972;
                            do
                            {
                                bool _24817 = in.i_radii.w > 0.5;
                                float _24821 = floor((in.i_radii.z + 1.0) * 0.5);
                                float _24825 = floor((_24821 + 127.0) * 0.0078125);
                                float _37965;
                                float _37967;
                                if (_24817)
                                {
                                    uint _25001 = uint(in.i_radii.z);
                                    uint _25004 = (_25001 + 31u) >> 5u;
                                    uint _25087 = (in.i_instance + 1u) * 24u;
                                    uint _37957;
                                    uint _37958;
                                    _37958 = 0u;
                                    _37957 = _25087;
                                    uint _25114;
                                    float _43981;
                                    uint _43982;
                                    uint _43983;
                                    uint _37956 = 0u;
                                    uint _38016 = _25087;
                                    float _38017 = 3.0000000054977557577780399428115e+38;
                                    for (; _37956 < _25004; _38017 = _43981, _38016 = _25114, _37958 = _43982, _37957 = _43983, _37956++)
                                    {
                                        uint _25092 = _38016 + 20u;
                                        float2 _25107 = fast::max(fast::max(gFxData_1._data[_25092].xy - _16966, _16966 - gFxData_1._data[_25092].zw), float2(0.0));
                                        float _25110 = dot(_25107, _25107);
                                        bool _25022 = _25110 < _38017;
                                        _43981 = _25022 ? _25110 : _38017;
                                        _43982 = _25022 ? _37956 : _37958;
                                        _43983 = _25022 ? _38016 : _37957;
                                        _25114 = _38016 + 24u;
                                    }
                                    uint _25152 = _37958 * 32u;
                                    uint _25156 = min((_25001 - _25152), 32u);
                                    uint _25260 = _37957 + 21u;
                                    uint _25267 = _37957 + 22u;
                                    float _37961;
                                    uint _37962;
                                    _37962 = 4294967295u;
                                    _37961 = 1000000015047466219876688855040.0;
                                    float _43538;
                                    uint _43539;
                                    for (uint _37960 = 0u; (_37960 * 8u) < _25156; _37962 = _43539, _37961 = _43538, _37960++)
                                    {
                                        uint _25274 = _37957 + (16u + _37960);
                                        float _25294 = ((sqrt(_37961) * 1.000003814697265625) + 0.03125) + ((abs(_16966.x) + abs(_16966.y)) * 3.814697265625e-06);
                                        float2 _25314 = fast::max(fast::max(gFxData_1._data[_25274].xy - _16966, _16966 - gFxData_1._data[_25274].zw), float2(0.0));
                                        if (dot(_25314, _25314) > (_25294 * _25294))
                                        {
                                            _43539 = _37962;
                                            _43538 = _37961;
                                            continue;
                                        }
                                        float4 _25182 = select(gFxData_1._data[_25267], gFxData_1._data[_25260], bool4(_37960 < 2u));
                                        uint _25196 = min(_25156, ((_37960 * 8u) + 8u));
                                        uint _25198 = _37960 * 8u;
                                        float2 _38001;
                                        float _38002;
                                        uint _38003;
                                        _38003 = _37962;
                                        _38002 = _37961;
                                        _38001 = select(_25182.xy, _25182.zw, bool2((_37960 & 1u) != 0u));
                                        float2 _43582;
                                        float _43583;
                                        uint _43584;
                                        for (uint _38000 = _25198; _38000 < _25196; _38003 = _43584, _38002 = _43583, _38001 = _43582, _38000 += 2u)
                                        {
                                            uint _25322 = _37957 + (_38000 >> 1u);
                                            uint _25211 = _25152 + _38000;
                                            float2 _25332 = _38001 - gFxData_1._data[_25322].xy;
                                            float2 _25335 = _16966 - gFxData_1._data[_25322].xy;
                                            float2 _25348 = _25335 - (_25332 * fast::clamp(dot(_25335, _25332) / fast::max(dot(_25332, _25332), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _25351 = dot(_25348, _25348);
                                            bool _25362 = (_25351 < _38002) || ((_25351 == _38002) && (_25211 < _38003));
                                            uint _43984 = _25362 ? _25211 : _38003;
                                            float _43985 = _25362 ? _25351 : _38002;
                                            if ((_38000 + 1u) < _25196)
                                            {
                                                uint _25233 = (_25152 + _38000) + 1u;
                                                float2 _25375 = gFxData_1._data[_25322].xy - gFxData_1._data[_25322].zw;
                                                float2 _25378 = _16966 - gFxData_1._data[_25322].zw;
                                                float2 _25391 = _25378 - (_25375 * fast::clamp(dot(_25378, _25375) / fast::max(dot(_25375, _25375), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _25394 = dot(_25391, _25391);
                                                bool _25405 = (_25394 < _43985) || ((_25394 == _43985) && (_25233 < _43984));
                                                _43584 = _25405 ? _25233 : _43984;
                                                _43583 = _25405 ? _25394 : _43985;
                                                _43582 = gFxData_1._data[_25322].zw;
                                            }
                                            else
                                            {
                                                _43584 = _43984;
                                                _43583 = _43985;
                                                _43582 = gFxData_1._data[_25322].xy;
                                            }
                                        }
                                        _43539 = _38003;
                                        _43538 = _38002;
                                    }
                                    uint _25414 = (in.i_instance + 1u) * 24u;
                                    float _37964;
                                    _37964 = _37961;
                                    uint _25762;
                                    float _43540;
                                    uint _43566;
                                    for (uint _37963 = 0u, _37975 = _25414, _37976 = _37962; _37963 < _25004; _37976 = _43566, _37975 = _25762, _37964 = _43540, _37963++)
                                    {
                                        uint _25419 = _37975 + 20u;
                                        float _25439 = ((sqrt(_37964) * 1.000003814697265625) + 0.03125) + ((abs(_16966.x) + abs(_16966.y)) * 3.814697265625e-06);
                                        float2 _25459 = fast::max(fast::max(gFxData_1._data[_25419].xy - _16966, _16966 - gFxData_1._data[_25419].zw), float2(0.0));
                                        if ((_37963 != _37958) && (!(dot(_25459, _25459) > (_25439 * _25439))))
                                        {
                                            uint _25500 = _37963 * 32u;
                                            uint _25504 = min((_25001 - _25500), 32u);
                                            uint _25608 = _37975 + 21u;
                                            uint _25615 = _37975 + 22u;
                                            float _37978;
                                            uint _37979;
                                            _37979 = _37976;
                                            _37978 = _37964;
                                            float _43567;
                                            uint _43568;
                                            for (uint _37977 = 0u; (_37977 * 8u) < _25504; _37979 = _43568, _37978 = _43567, _37977++)
                                            {
                                                uint _25622 = _37975 + (16u + _37977);
                                                float _25642 = ((sqrt(_37978) * 1.000003814697265625) + 0.03125) + ((abs(_16966.x) + abs(_16966.y)) * 3.814697265625e-06);
                                                float2 _25662 = fast::max(fast::max(gFxData_1._data[_25622].xy - _16966, _16966 - gFxData_1._data[_25622].zw), float2(0.0));
                                                if (dot(_25662, _25662) > (_25642 * _25642))
                                                {
                                                    _43568 = _37979;
                                                    _43567 = _37978;
                                                    continue;
                                                }
                                                float4 _25530 = select(gFxData_1._data[_25615], gFxData_1._data[_25608], bool4(_37977 < 2u));
                                                uint _25544 = min(_25504, ((_37977 * 8u) + 8u));
                                                uint _25546 = _37977 * 8u;
                                                float2 _37985;
                                                float _37986;
                                                uint _37987;
                                                _37987 = _37979;
                                                _37986 = _37978;
                                                _37985 = select(_25530.xy, _25530.zw, bool2((_37977 & 1u) != 0u));
                                                float2 _43579;
                                                float _43580;
                                                uint _43581;
                                                for (uint _37984 = _25546; _37984 < _25544; _37987 = _43581, _37986 = _43580, _37985 = _43579, _37984 += 2u)
                                                {
                                                    uint _25670 = _37975 + (_37984 >> 1u);
                                                    uint _25559 = _25500 + _37984;
                                                    float2 _25680 = _37985 - gFxData_1._data[_25670].xy;
                                                    float2 _25683 = _16966 - gFxData_1._data[_25670].xy;
                                                    float2 _25696 = _25683 - (_25680 * fast::clamp(dot(_25683, _25680) / fast::max(dot(_25680, _25680), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                    float _25699 = dot(_25696, _25696);
                                                    bool _25710 = (_25699 < _37986) || ((_25699 == _37986) && (_25559 < _37987));
                                                    uint _43988 = _25710 ? _25559 : _37987;
                                                    float _43989 = _25710 ? _25699 : _37986;
                                                    if ((_37984 + 1u) < _25544)
                                                    {
                                                        uint _25581 = (_25500 + _37984) + 1u;
                                                        float2 _25723 = gFxData_1._data[_25670].xy - gFxData_1._data[_25670].zw;
                                                        float2 _25726 = _16966 - gFxData_1._data[_25670].zw;
                                                        float2 _25739 = _25726 - (_25723 * fast::clamp(dot(_25726, _25723) / fast::max(dot(_25723, _25723), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                        float _25742 = dot(_25739, _25739);
                                                        bool _25753 = (_25742 < _43989) || ((_25742 == _43989) && (_25581 < _43988));
                                                        _43581 = _25753 ? _25581 : _43988;
                                                        _43580 = _25753 ? _25742 : _43989;
                                                        _43579 = gFxData_1._data[_25670].zw;
                                                    }
                                                    else
                                                    {
                                                        _43581 = _43988;
                                                        _43580 = _43989;
                                                        _43579 = gFxData_1._data[_25670].xy;
                                                    }
                                                }
                                                _43568 = _37987;
                                                _43567 = _37986;
                                            }
                                            _43566 = _37979;
                                            _43540 = _37978;
                                        }
                                        else
                                        {
                                            _43566 = _37976;
                                            _43540 = _37964;
                                        }
                                        _25762 = _37975 + 24u;
                                    }
                                    _37967 = 1.0;
                                    _37965 = _37964;
                                }
                                else
                                {
                                    float _24839 = floor((in.i_radii.z - 1.0) * 0.5);
                                    float _25770 = floor(_24839 * 0.0625);
                                    uint _25787 = (((in.i_instance + 1u) + uint(_25770)) * 24u) + uint(_24839 - (_25770 * 16.0));
                                    float2 _37935;
                                    if (_24817)
                                    {
                                        float _25797 = floor(0.0);
                                        _37935 = gFxData_1._data[(((in.i_instance + 1u) + uint(_25797)) * 24u) + uint(_25797 * (-16.0))].xy;
                                    }
                                    else
                                    {
                                        _37935 = select(gFxData_1._data[_25787].zw, gFxData_1._data[_25787].xy, bool2(((in.i_radii.z - 1.0) - (_24839 * 2.0)) < 0.5));
                                    }
                                    float2 _37934;
                                    float _37937;
                                    float _37940;
                                    _37940 = 1.0;
                                    _37937 = 1000000015047466219876688855040.0;
                                    _37934 = _37935;
                                    float2 _37933;
                                    float _37936;
                                    float _37939;
                                    for (float _37930 = 0.0; _37930 < 16.0; _37940 = _37939, _37937 = _37936, _37934 = _37933, _37930 += 1.0)
                                    {
                                        if (_37930 >= _24825)
                                        {
                                            break;
                                        }
                                        _37939 = _37940;
                                        _37936 = _37937;
                                        _37933 = _37934;
                                        float2 _43530;
                                        float _43532;
                                        float _43534;
                                        for (float _37931 = 0.0; _37931 < 128.0; _37939 = _43534, _37936 = _43532, _37933 = _43530, _37931 += 1.0)
                                        {
                                            float _24881 = (_37930 * 128.0) + _37931;
                                            if (_24881 >= _24821)
                                            {
                                                break;
                                            }
                                            float _25824 = floor(_24881 * 0.0625);
                                            uint _25841 = (((in.i_instance + 1u) + uint(_25824)) * 24u) + uint(_24881 - (_25824 * 16.0));
                                            float2 _25854 = _37933 - gFxData_1._data[_25841].xy;
                                            float2 _25857 = _16966 - gFxData_1._data[_25841].xy;
                                            float2 _25870 = _25857 - (_25854 * fast::clamp(dot(_25857, _25854) / fast::max(dot(_25854, _25854), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _25873 = dot(_25870, _25870);
                                            float _43992 = (_25873 < _37936) ? _25873 : _37936;
                                            bool _25885 = _16966.y >= gFxData_1._data[_25841].y;
                                            bool _25890 = _16966.y < _37933.y;
                                            bool _25901 = (_25854.x * _25857.y) > (_25854.y * _25857.x);
                                            float _37945;
                                            if (((_25885 && _25890) && _25901) || (((!_25885) && (!_25890)) && (!_25901)))
                                            {
                                                _37945 = -_37939;
                                            }
                                            else
                                            {
                                                _37945 = _37939;
                                            }
                                            if (((2.0 * _24881) + 1.0) < in.i_radii.z)
                                            {
                                                float2 _25930 = gFxData_1._data[_25841].xy - gFxData_1._data[_25841].zw;
                                                float2 _25933 = _16966 - gFxData_1._data[_25841].zw;
                                                float2 _25946 = _25933 - (_25930 * fast::clamp(dot(_25933, _25930) / fast::max(dot(_25930, _25930), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _25949 = dot(_25946, _25946);
                                                bool _25961 = _16966.y >= gFxData_1._data[_25841].w;
                                                bool _25966 = _16966.y < gFxData_1._data[_25841].y;
                                                bool _25977 = (_25930.x * _25933.y) > (_25930.y * _25933.x);
                                                float _37949;
                                                if (((_25961 && _25966) && _25977) || (((!_25961) && (!_25966)) && (!_25977)))
                                                {
                                                    _37949 = -_37945;
                                                }
                                                else
                                                {
                                                    _37949 = _37945;
                                                }
                                                _43534 = _37949;
                                                _43532 = (_25949 < _43992) ? _25949 : _43992;
                                                _43530 = gFxData_1._data[_25841].zw;
                                            }
                                            else
                                            {
                                                _43534 = _37945;
                                                _43532 = _43992;
                                                _43530 = gFxData_1._data[_25841].xy;
                                            }
                                        }
                                    }
                                    _37967 = _37940;
                                    _37965 = _37937;
                                }
                                float _24934 = sqrt(_37965);
                                if (_24817)
                                {
                                    _37972 = _24934 - in.i_radii.x;
                                    break;
                                }
                                _37972 = (_37967 * _24934) - in.i_radii.x;
                                break;
                            } while(false);
                            _38021 = _37972;
                        }
                        else
                        {
                            float _38022;
                            if (in.i_flags.z == 1u)
                            {
                                float2 _24560 = _16966 - _24526;
                                float _37929;
                                do
                                {
                                    if (in.i_shape.w >= 6.282185077667236328125)
                                    {
                                        _37929 = abs(length(_24560) - in.i_radii.x) - in.i_radii.y;
                                        break;
                                    }
                                    float _26020 = in.i_shape.z + (in.i_shape.w * 0.5);
                                    float _26022 = cos(_26020);
                                    float _26024 = sin(_26020);
                                    float _26033 = dot(_24560, float2(-_26024, _26022));
                                    float _26036 = dot(_24560, float2(_26022, _26024));
                                    float2 _26037 = float2(_26033, _26036);
                                    float _26040 = abs(_26033);
                                    _26037.x = _26040;
                                    float _26043 = in.i_shape.w * 0.5;
                                    float _26045 = sin(_26043);
                                    float _26047 = cos(_26043);
                                    _37929 = (((_26047 * _26040) > (_26045 * _26036)) ? length(_26037 - (float2(_26045, _26047) * in.i_radii.x)) : abs(length(_26037) - in.i_radii.x)) - in.i_radii.y;
                                    break;
                                } while(false);
                                _38022 = _37929;
                            }
                            else
                            {
                                float _38023;
                                if (in.i_flags.z == 2u)
                                {
                                    float2 _26083 = _16966 - _36247.xy;
                                    float2 _26086 = _36247.zw - _36247.xy;
                                    _38023 = length(_26083 - (_26086 * fast::clamp(dot(_26083, _26086) / fast::max(dot(_26086, _26086), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                                }
                                else
                                {
                                    float2 _24587 = _16966 - _24526;
                                    float _26151 = fast::min(_24535.x, _24535.y);
                                    float _26154 = fast::min((_24587.x > 0.0) ? ((_24587.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_24587.y > 0.0) ? in.i_radii.w : in.i_radii.x), _26151);
                                    float _26160 = _26154 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                    float _37918;
                                    float _37919;
                                    if (_26160 > _26151)
                                    {
                                        float _26174 = in.i_shape.y * fast::clamp((_26151 - _26154) / fast::max(0.60000002384185791015625 * _26154, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                        _37919 = _26174;
                                        _37918 = _26154 * (1.0 + (0.60000002384185791015625 * _26174));
                                    }
                                    else
                                    {
                                        _37919 = in.i_shape.y;
                                        _37918 = _26160;
                                    }
                                    float2 _26203 = (abs(_24587) - _24535) + float2(_37918);
                                    float2 _26205 = fast::max(_26203, float2(0.0));
                                    float _37920;
                                    if ((_26205.x > 0.0) && (_26205.y > 0.0))
                                    {
                                        float _37921;
                                        if ((_37919 > 0.001000000047497451305389404296875) && (_37918 > 9.9999997473787516355514526367188e-05))
                                        {
                                            float _26222 = 2.0 + (2.0 * _37919);
                                            float2 _26227 = _26205 / float2(fast::max(_37918, 9.9999997473787516355514526367188e-05));
                                            _37921 = pow(pow(_26227.x, _26222) + pow(_26227.y, _26222), 1.0 / _26222) * _37918;
                                        }
                                        else
                                        {
                                            _37921 = length(_26205);
                                        }
                                        _37920 = _37921;
                                    }
                                    else
                                    {
                                        _37920 = fast::max(_26205.x, _26205.y);
                                    }
                                    float _26262 = (fast::min(fast::max(_26203.x, _26203.y), 0.0) + _37920) - _37918;
                                    float _38024;
                                    if ((in.i_flags.x & _2278) != 0u)
                                    {
                                        float2 _24615 = fast::max((_36247.zw - _36247.xy) * 0.5, float2(0.001000000047497451305389404296875));
                                        float2 _24618 = _16966 - ((_36247.xy + _36247.zw) * 0.5);
                                        float _26310 = fast::min(_24615.x, _24615.y);
                                        float _26313 = fast::min((_24618.x > 0.0) ? ((_24618.y > 0.0) ? gFxData_1._data[_16311].x : gFxData_1._data[_16311].x) : ((_24618.y > 0.0) ? gFxData_1._data[_16311].x : gFxData_1._data[_16311].x), _26310);
                                        float _26319 = _26313 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                                        float _37922;
                                        float _37923;
                                        if (_26319 > _26310)
                                        {
                                            float _26333 = in.i_shape.y * fast::clamp((_26310 - _26313) / fast::max(0.60000002384185791015625 * _26313, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                            _37923 = _26333;
                                            _37922 = _26313 * (1.0 + (0.60000002384185791015625 * _26333));
                                        }
                                        else
                                        {
                                            _37923 = in.i_shape.y;
                                            _37922 = _26319;
                                        }
                                        float2 _26362 = (abs(_24618) - _24615) + float2(_37922);
                                        float2 _26364 = fast::max(_26362, float2(0.0));
                                        float _37924;
                                        if ((_26364.x > 0.0) && (_26364.y > 0.0))
                                        {
                                            float _37925;
                                            if ((_37923 > 0.001000000047497451305389404296875) && (_37922 > 9.9999997473787516355514526367188e-05))
                                            {
                                                float _26381 = 2.0 + (2.0 * _37923);
                                                float2 _26386 = _26364 / float2(fast::max(_37922, 9.9999997473787516355514526367188e-05));
                                                _37925 = pow(pow(_26386.x, _26381) + pow(_26386.y, _26381), 1.0 / _26381) * _37922;
                                            }
                                            else
                                            {
                                                _37925 = length(_26364);
                                            }
                                            _37924 = _37925;
                                        }
                                        else
                                        {
                                            _37924 = fast::max(_26364.x, _26364.y);
                                        }
                                        float _26421 = (fast::min(fast::max(_26362.x, _26362.y), 0.0) + _37924) - _37922;
                                        float _26426 = fast::max(gFxData_1._data[_16311].y, 9.9999997473787516355514526367188e-05);
                                        float _26435 = fast::max(_26426 - abs(_26262 - _26421), 0.0) / _26426;
                                        _38024 = fast::min(_26262, _26421) - (((_26435 * _26435) * _26426) * 0.25);
                                    }
                                    else
                                    {
                                        _38024 = _26262;
                                    }
                                    _38023 = _38024;
                                }
                                _38022 = _38023;
                            }
                            _38021 = _38022;
                        }
                        _38020 = _38021;
                    }
                    _38249 = float2(_37699 - _37806, _37913 - _38020);
                }
                float _16975 = length(_38249);
                _38305 = select(float2(0.0, -1.0), _38249 / float2(_16975), bool2(_16975 > 9.9999997473787516355514526367188e-06));
                break;
            } while(false);
            _38883 = fast::normalize(float3(_38305 * fast::min(_16412 / fast::max(_16418, 0.001000000047497451305389404296875), 8.0), 1.0));
            _38306 = _38305;
        }
        else
        {
            _38883 = float3(0.0, 0.0, 1.0);
            _38306 = float2(0.0, -1.0);
        }
        float2 _16444 = ((-_38306) * gFxData_1._data[_16287].y) * (1.0 - _16418);
        float2 _38307;
        if (gFxData_1._data[_16311].z > 0.0)
        {
            _38307 = (((in.i_rect.xy + in.i_rect.zw) * 0.5) - in.i_local) * (gFxData_1._data[_16311].z / (1.0 + gFxData_1._data[_16311].z));
        }
        else
        {
            _38307 = float2(0.0);
        }
        float2 _16470 = _36240 * _288.gTarget.zw;
        float2 _16477 = _288.gDisplay.zw * _288.gTarget.zw;
        float2 _16482 = (_16444 + _38307) * _16477;
        float2 _16485 = _16444 * _16477;
        float3 _38765;
        float _38783;
        float3 _38986;
        if (_288.gTime.z > 0.5)
        {
            float2 _16492 = _16470 + _16482;
            float _26460 = fast::clamp(log2(fast::max(_16384, 1.0)) - 1.0, 0.0, 5.0);
            int _26463 = int(floor(_26460));
            float _26467 = _26460 - float(_26463);
            bool _26472 = (_26467 > 0.0199999995529651641845703125) && (_26463 < 5);
            float3 _38378;
            _38378 = float3(0.0);
            float3 _26501;
            for (int _38308 = 0; _38308 < 2; _38378 = _26501, _38308++)
            {
                if ((_38308 > 0) && (!_26472))
                {
                    break;
                }
                int _26487 = _26463 + _38308;
                int _26525 = clamp(_26487, 1, 5);
                float2 _26594 = (_16492 * _288.gLevel[_26525].xy) - float2(0.5);
                float2 _26596 = floor(_26594);
                float2 _26599 = _26594 - _26596;
                float2 _26602 = _26599 * _26599;
                float2 _26605 = _26602 * _26599;
                float2 _26624 = (((_26605 * 3.0) - (_26602 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                float2 _26637 = _26605 * 0.16666667163372039794921875;
                float2 _26640 = (((((-_26605) + (_26602 * 3.0)) - (_26599 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _26624;
                float2 _26644 = (((((_26605 * (-3.0)) + (_26602 * 3.0)) + (_26599 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _26637;
                float2 _26656 = ((_26596 - float2(0.5)) + (_26624 / _26640)) * _288.gLevel[_26525].zw;
                float2 _26668 = ((_26596 + float2(1.5)) + (_26637 / _26644)) * _288.gLevel[_26525].zw;
                float4 _38345;
                if (_26487 <= 0)
                {
                    float2 _38344;
                    if (_288.gConv.x > 0.5)
                    {
                        float2 _35521 = _16492;
                        _35521.y = 1.0 - _16492.y;
                        _38344 = _35521;
                    }
                    else
                    {
                        _38344 = _16492;
                    }
                    _38345 = gBackdrop0.sample(gLinear, _38344, level(0.0));
                }
                else
                {
                    float4 _38346;
                    if (_26487 == 1)
                    {
                        float2 _38337;
                        if (_288.gConv.x > 0.5)
                        {
                            float2 _35526 = _26656;
                            _35526.y = 1.0 - _26656.y;
                            _38337 = _35526;
                        }
                        else
                        {
                            _38337 = _26656;
                        }
                        float2 _26714 = float2(_26668.x, _26656.y);
                        float2 _38338;
                        if (_288.gConv.x > 0.5)
                        {
                            float2 _35533 = _26714;
                            _35533.y = 1.0 - _26656.y;
                            _38338 = _35533;
                        }
                        else
                        {
                            _38338 = _26714;
                        }
                        float2 _26732 = float2(_26656.x, _26668.y);
                        float2 _38340;
                        if (_288.gConv.x > 0.5)
                        {
                            float2 _35540 = _26732;
                            _35540.y = 1.0 - _26668.y;
                            _38340 = _35540;
                        }
                        else
                        {
                            _38340 = _26732;
                        }
                        float2 _38342;
                        if (_288.gConv.x > 0.5)
                        {
                            float2 _35547 = _26668;
                            _35547.y = 1.0 - _26668.y;
                            _38342 = _35547;
                        }
                        else
                        {
                            _38342 = _26668;
                        }
                        _38346 = (((gBackdrop1.sample(gLinear, _38337, level(0.0)) * (_26640.x * _26640.y)) + (gBackdrop1.sample(gLinear, _38338, level(0.0)) * (_26644.x * _26640.y))) + (gBackdrop1.sample(gLinear, _38340, level(0.0)) * (_26640.x * _26644.y))) + (gBackdrop1.sample(gLinear, _38342, level(0.0)) * (_26644.x * _26644.y));
                    }
                    else
                    {
                        float4 _38347;
                        if (_26487 == 2)
                        {
                            float2 _38330;
                            if (_288.gConv.x > 0.5)
                            {
                                float2 _35554 = _26656;
                                _35554.y = 1.0 - _26656.y;
                                _38330 = _35554;
                            }
                            else
                            {
                                _38330 = _26656;
                            }
                            float2 _26844 = float2(_26668.x, _26656.y);
                            float2 _38331;
                            if (_288.gConv.x > 0.5)
                            {
                                float2 _35561 = _26844;
                                _35561.y = 1.0 - _26656.y;
                                _38331 = _35561;
                            }
                            else
                            {
                                _38331 = _26844;
                            }
                            float2 _26862 = float2(_26656.x, _26668.y);
                            float2 _38333;
                            if (_288.gConv.x > 0.5)
                            {
                                float2 _35568 = _26862;
                                _35568.y = 1.0 - _26668.y;
                                _38333 = _35568;
                            }
                            else
                            {
                                _38333 = _26862;
                            }
                            float2 _38335;
                            if (_288.gConv.x > 0.5)
                            {
                                float2 _35575 = _26668;
                                _35575.y = 1.0 - _26668.y;
                                _38335 = _35575;
                            }
                            else
                            {
                                _38335 = _26668;
                            }
                            _38347 = (((gBackdrop2.sample(gLinear, _38330, level(0.0)) * (_26640.x * _26640.y)) + (gBackdrop2.sample(gLinear, _38331, level(0.0)) * (_26644.x * _26640.y))) + (gBackdrop2.sample(gLinear, _38333, level(0.0)) * (_26640.x * _26644.y))) + (gBackdrop2.sample(gLinear, _38335, level(0.0)) * (_26644.x * _26644.y));
                        }
                        else
                        {
                            float4 _38348;
                            if (_26487 == 3)
                            {
                                float2 _38323;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _35582 = _26656;
                                    _35582.y = 1.0 - _26656.y;
                                    _38323 = _35582;
                                }
                                else
                                {
                                    _38323 = _26656;
                                }
                                float2 _26974 = float2(_26668.x, _26656.y);
                                float2 _38324;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _35589 = _26974;
                                    _35589.y = 1.0 - _26656.y;
                                    _38324 = _35589;
                                }
                                else
                                {
                                    _38324 = _26974;
                                }
                                float2 _26992 = float2(_26656.x, _26668.y);
                                float2 _38326;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _35596 = _26992;
                                    _35596.y = 1.0 - _26668.y;
                                    _38326 = _35596;
                                }
                                else
                                {
                                    _38326 = _26992;
                                }
                                float2 _38328;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _35603 = _26668;
                                    _35603.y = 1.0 - _26668.y;
                                    _38328 = _35603;
                                }
                                else
                                {
                                    _38328 = _26668;
                                }
                                _38348 = (((gBackdrop3.sample(gLinear, _38323, level(0.0)) * (_26640.x * _26640.y)) + (gBackdrop3.sample(gLinear, _38324, level(0.0)) * (_26644.x * _26640.y))) + (gBackdrop3.sample(gLinear, _38326, level(0.0)) * (_26640.x * _26644.y))) + (gBackdrop3.sample(gLinear, _38328, level(0.0)) * (_26644.x * _26644.y));
                            }
                            else
                            {
                                float4 _38349;
                                if (_26487 == 4)
                                {
                                    float2 _38316;
                                    if (_288.gConv.x > 0.5)
                                    {
                                        float2 _35610 = _26656;
                                        _35610.y = 1.0 - _26656.y;
                                        _38316 = _35610;
                                    }
                                    else
                                    {
                                        _38316 = _26656;
                                    }
                                    float2 _27104 = float2(_26668.x, _26656.y);
                                    float2 _38317;
                                    if (_288.gConv.x > 0.5)
                                    {
                                        float2 _35617 = _27104;
                                        _35617.y = 1.0 - _26656.y;
                                        _38317 = _35617;
                                    }
                                    else
                                    {
                                        _38317 = _27104;
                                    }
                                    float2 _27122 = float2(_26656.x, _26668.y);
                                    float2 _38319;
                                    if (_288.gConv.x > 0.5)
                                    {
                                        float2 _35624 = _27122;
                                        _35624.y = 1.0 - _26668.y;
                                        _38319 = _35624;
                                    }
                                    else
                                    {
                                        _38319 = _27122;
                                    }
                                    float2 _38321;
                                    if (_288.gConv.x > 0.5)
                                    {
                                        float2 _35631 = _26668;
                                        _35631.y = 1.0 - _26668.y;
                                        _38321 = _35631;
                                    }
                                    else
                                    {
                                        _38321 = _26668;
                                    }
                                    _38349 = (((gBackdrop4.sample(gLinear, _38316, level(0.0)) * (_26640.x * _26640.y)) + (gBackdrop4.sample(gLinear, _38317, level(0.0)) * (_26644.x * _26640.y))) + (gBackdrop4.sample(gLinear, _38319, level(0.0)) * (_26640.x * _26644.y))) + (gBackdrop4.sample(gLinear, _38321, level(0.0)) * (_26644.x * _26644.y));
                                }
                                else
                                {
                                    float2 _38309;
                                    if (_288.gConv.x > 0.5)
                                    {
                                        float2 _35638 = _26656;
                                        _35638.y = 1.0 - _26656.y;
                                        _38309 = _35638;
                                    }
                                    else
                                    {
                                        _38309 = _26656;
                                    }
                                    float2 _27234 = float2(_26668.x, _26656.y);
                                    float2 _38310;
                                    if (_288.gConv.x > 0.5)
                                    {
                                        float2 _35645 = _27234;
                                        _35645.y = 1.0 - _26656.y;
                                        _38310 = _35645;
                                    }
                                    else
                                    {
                                        _38310 = _27234;
                                    }
                                    float2 _27252 = float2(_26656.x, _26668.y);
                                    float2 _38312;
                                    if (_288.gConv.x > 0.5)
                                    {
                                        float2 _35652 = _27252;
                                        _35652.y = 1.0 - _26668.y;
                                        _38312 = _35652;
                                    }
                                    else
                                    {
                                        _38312 = _27252;
                                    }
                                    float2 _38314;
                                    if (_288.gConv.x > 0.5)
                                    {
                                        float2 _35659 = _26668;
                                        _35659.y = 1.0 - _26668.y;
                                        _38314 = _35659;
                                    }
                                    else
                                    {
                                        _38314 = _26668;
                                    }
                                    _38349 = (((gBackdrop5.sample(gLinear, _38309, level(0.0)) * (_26640.x * _26640.y)) + (gBackdrop5.sample(gLinear, _38310, level(0.0)) * (_26644.x * _26640.y))) + (gBackdrop5.sample(gLinear, _38312, level(0.0)) * (_26640.x * _26644.y))) + (gBackdrop5.sample(gLinear, _38314, level(0.0)) * (_26644.x * _26644.y));
                                }
                                _38348 = _38349;
                            }
                            _38347 = _38348;
                        }
                        _38346 = _38347;
                    }
                    _38345 = _38346;
                }
                _26501 = _38378 + (_38345.xyz * (_26472 ? ((_38308 == 0) ? (1.0 - _26467) : _26467) : 1.0));
            }
            float3 _38768;
            if (((gFxData_1._data[_16287].w > 0.001000000047497451305389404296875) && (_16412 > 0.0)) && ((((gFxData_1._data[_16287].y * 0.300000011920928955078125) * gFxData_1._data[_16287].w) * _288.gDisplay.z) > (_16384 * 0.3499999940395355224609375)))
            {
                float _16512 = 0.300000011920928955078125 * gFxData_1._data[_16287].w;
                float2 _16519 = (_16470 + _16482) - (_16485 * _16512);
                float _27348 = fast::clamp(log2(fast::max(_16384, 1.0)) - 1.0, 0.0, 5.0);
                int _27351 = int(floor(_27348));
                float _27355 = _27348 - float(_27351);
                bool _27360 = (_27355 > 0.0199999995529651641845703125) && (_27351 < 5);
                float3 _38474;
                _38474 = float3(0.0);
                float3 _27389;
                for (int _38404 = 0; _38404 < 2; _38474 = _27389, _38404++)
                {
                    if ((_38404 > 0) && (!_27360))
                    {
                        break;
                    }
                    int _27375 = _27351 + _38404;
                    int _27413 = clamp(_27375, 1, 5);
                    float2 _27482 = (_16519 * _288.gLevel[_27413].xy) - float2(0.5);
                    float2 _27484 = floor(_27482);
                    float2 _27487 = _27482 - _27484;
                    float2 _27490 = _27487 * _27487;
                    float2 _27493 = _27490 * _27487;
                    float2 _27512 = (((_27493 * 3.0) - (_27490 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                    float2 _27525 = _27493 * 0.16666667163372039794921875;
                    float2 _27528 = (((((-_27493) + (_27490 * 3.0)) - (_27487 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _27512;
                    float2 _27532 = (((((_27493 * (-3.0)) + (_27490 * 3.0)) + (_27487 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _27525;
                    float2 _27544 = ((_27484 - float2(0.5)) + (_27512 / _27528)) * _288.gLevel[_27413].zw;
                    float2 _27556 = ((_27484 + float2(1.5)) + (_27525 / _27532)) * _288.gLevel[_27413].zw;
                    float4 _38441;
                    if (_27375 <= 0)
                    {
                        float2 _38440;
                        if (_288.gConv.x > 0.5)
                        {
                            float2 _35664 = _16519;
                            _35664.y = 1.0 - _16519.y;
                            _38440 = _35664;
                        }
                        else
                        {
                            _38440 = _16519;
                        }
                        _38441 = gBackdrop0.sample(gLinear, _38440, level(0.0));
                    }
                    else
                    {
                        float4 _38442;
                        if (_27375 == 1)
                        {
                            float2 _38433;
                            if (_288.gConv.x > 0.5)
                            {
                                float2 _35669 = _27544;
                                _35669.y = 1.0 - _27544.y;
                                _38433 = _35669;
                            }
                            else
                            {
                                _38433 = _27544;
                            }
                            float _27601 = _27544.y;
                            float2 _27602 = float2(_27556.x, _27601);
                            float2 _38434;
                            if (_288.gConv.x > 0.5)
                            {
                                float2 _35676 = _27602;
                                _35676.y = 1.0 - _27601;
                                _38434 = _35676;
                            }
                            else
                            {
                                _38434 = _27602;
                            }
                            float _27619 = _27556.y;
                            float2 _27620 = float2(_27544.x, _27619);
                            float2 _38436;
                            if (_288.gConv.x > 0.5)
                            {
                                float2 _35683 = _27620;
                                _35683.y = 1.0 - _27619;
                                _38436 = _35683;
                            }
                            else
                            {
                                _38436 = _27620;
                            }
                            float2 _38438;
                            if (_288.gConv.x > 0.5)
                            {
                                float2 _35690 = _27556;
                                _35690.y = 1.0 - _27556.y;
                                _38438 = _35690;
                            }
                            else
                            {
                                _38438 = _27556;
                            }
                            _38442 = (((gBackdrop1.sample(gLinear, _38433, level(0.0)) * (_27528.x * _27528.y)) + (gBackdrop1.sample(gLinear, _38434, level(0.0)) * (_27532.x * _27528.y))) + (gBackdrop1.sample(gLinear, _38436, level(0.0)) * (_27528.x * _27532.y))) + (gBackdrop1.sample(gLinear, _38438, level(0.0)) * (_27532.x * _27532.y));
                        }
                        else
                        {
                            float4 _38443;
                            if (_27375 == 2)
                            {
                                float2 _38426;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _35697 = _27544;
                                    _35697.y = 1.0 - _27544.y;
                                    _38426 = _35697;
                                }
                                else
                                {
                                    _38426 = _27544;
                                }
                                float _27731 = _27544.y;
                                float2 _27732 = float2(_27556.x, _27731);
                                float2 _38427;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _35704 = _27732;
                                    _35704.y = 1.0 - _27731;
                                    _38427 = _35704;
                                }
                                else
                                {
                                    _38427 = _27732;
                                }
                                float _27749 = _27556.y;
                                float2 _27750 = float2(_27544.x, _27749);
                                float2 _38429;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _35711 = _27750;
                                    _35711.y = 1.0 - _27749;
                                    _38429 = _35711;
                                }
                                else
                                {
                                    _38429 = _27750;
                                }
                                float2 _38431;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _35718 = _27556;
                                    _35718.y = 1.0 - _27556.y;
                                    _38431 = _35718;
                                }
                                else
                                {
                                    _38431 = _27556;
                                }
                                _38443 = (((gBackdrop2.sample(gLinear, _38426, level(0.0)) * (_27528.x * _27528.y)) + (gBackdrop2.sample(gLinear, _38427, level(0.0)) * (_27532.x * _27528.y))) + (gBackdrop2.sample(gLinear, _38429, level(0.0)) * (_27528.x * _27532.y))) + (gBackdrop2.sample(gLinear, _38431, level(0.0)) * (_27532.x * _27532.y));
                            }
                            else
                            {
                                float4 _38444;
                                if (_27375 == 3)
                                {
                                    float2 _38419;
                                    if (_288.gConv.x > 0.5)
                                    {
                                        float2 _35725 = _27544;
                                        _35725.y = 1.0 - _27544.y;
                                        _38419 = _35725;
                                    }
                                    else
                                    {
                                        _38419 = _27544;
                                    }
                                    float _27861 = _27544.y;
                                    float2 _27862 = float2(_27556.x, _27861);
                                    float2 _38420;
                                    if (_288.gConv.x > 0.5)
                                    {
                                        float2 _35732 = _27862;
                                        _35732.y = 1.0 - _27861;
                                        _38420 = _35732;
                                    }
                                    else
                                    {
                                        _38420 = _27862;
                                    }
                                    float _27879 = _27556.y;
                                    float2 _27880 = float2(_27544.x, _27879);
                                    float2 _38422;
                                    if (_288.gConv.x > 0.5)
                                    {
                                        float2 _35739 = _27880;
                                        _35739.y = 1.0 - _27879;
                                        _38422 = _35739;
                                    }
                                    else
                                    {
                                        _38422 = _27880;
                                    }
                                    float2 _38424;
                                    if (_288.gConv.x > 0.5)
                                    {
                                        float2 _35746 = _27556;
                                        _35746.y = 1.0 - _27556.y;
                                        _38424 = _35746;
                                    }
                                    else
                                    {
                                        _38424 = _27556;
                                    }
                                    _38444 = (((gBackdrop3.sample(gLinear, _38419, level(0.0)) * (_27528.x * _27528.y)) + (gBackdrop3.sample(gLinear, _38420, level(0.0)) * (_27532.x * _27528.y))) + (gBackdrop3.sample(gLinear, _38422, level(0.0)) * (_27528.x * _27532.y))) + (gBackdrop3.sample(gLinear, _38424, level(0.0)) * (_27532.x * _27532.y));
                                }
                                else
                                {
                                    float4 _38445;
                                    if (_27375 == 4)
                                    {
                                        float2 _38412;
                                        if (_288.gConv.x > 0.5)
                                        {
                                            float2 _35753 = _27544;
                                            _35753.y = 1.0 - _27544.y;
                                            _38412 = _35753;
                                        }
                                        else
                                        {
                                            _38412 = _27544;
                                        }
                                        float _27991 = _27544.y;
                                        float2 _27992 = float2(_27556.x, _27991);
                                        float2 _38413;
                                        if (_288.gConv.x > 0.5)
                                        {
                                            float2 _35760 = _27992;
                                            _35760.y = 1.0 - _27991;
                                            _38413 = _35760;
                                        }
                                        else
                                        {
                                            _38413 = _27992;
                                        }
                                        float _28009 = _27556.y;
                                        float2 _28010 = float2(_27544.x, _28009);
                                        float2 _38415;
                                        if (_288.gConv.x > 0.5)
                                        {
                                            float2 _35767 = _28010;
                                            _35767.y = 1.0 - _28009;
                                            _38415 = _35767;
                                        }
                                        else
                                        {
                                            _38415 = _28010;
                                        }
                                        float2 _38417;
                                        if (_288.gConv.x > 0.5)
                                        {
                                            float2 _35774 = _27556;
                                            _35774.y = 1.0 - _27556.y;
                                            _38417 = _35774;
                                        }
                                        else
                                        {
                                            _38417 = _27556;
                                        }
                                        _38445 = (((gBackdrop4.sample(gLinear, _38412, level(0.0)) * (_27528.x * _27528.y)) + (gBackdrop4.sample(gLinear, _38413, level(0.0)) * (_27532.x * _27528.y))) + (gBackdrop4.sample(gLinear, _38415, level(0.0)) * (_27528.x * _27532.y))) + (gBackdrop4.sample(gLinear, _38417, level(0.0)) * (_27532.x * _27532.y));
                                    }
                                    else
                                    {
                                        float2 _38405;
                                        if (_288.gConv.x > 0.5)
                                        {
                                            float2 _35781 = _27544;
                                            _35781.y = 1.0 - _27544.y;
                                            _38405 = _35781;
                                        }
                                        else
                                        {
                                            _38405 = _27544;
                                        }
                                        float _28121 = _27544.y;
                                        float2 _28122 = float2(_27556.x, _28121);
                                        float2 _38406;
                                        if (_288.gConv.x > 0.5)
                                        {
                                            float2 _35788 = _28122;
                                            _35788.y = 1.0 - _28121;
                                            _38406 = _35788;
                                        }
                                        else
                                        {
                                            _38406 = _28122;
                                        }
                                        float _28139 = _27556.y;
                                        float2 _28140 = float2(_27544.x, _28139);
                                        float2 _38408;
                                        if (_288.gConv.x > 0.5)
                                        {
                                            float2 _35795 = _28140;
                                            _35795.y = 1.0 - _28139;
                                            _38408 = _35795;
                                        }
                                        else
                                        {
                                            _38408 = _28140;
                                        }
                                        float2 _38410;
                                        if (_288.gConv.x > 0.5)
                                        {
                                            float2 _35802 = _27556;
                                            _35802.y = 1.0 - _27556.y;
                                            _38410 = _35802;
                                        }
                                        else
                                        {
                                            _38410 = _27556;
                                        }
                                        _38445 = (((gBackdrop5.sample(gLinear, _38405, level(0.0)) * (_27528.x * _27528.y)) + (gBackdrop5.sample(gLinear, _38406, level(0.0)) * (_27532.x * _27528.y))) + (gBackdrop5.sample(gLinear, _38408, level(0.0)) * (_27528.x * _27532.y))) + (gBackdrop5.sample(gLinear, _38410, level(0.0)) * (_27532.x * _27532.y));
                                    }
                                    _38444 = _38445;
                                }
                                _38443 = _38444;
                            }
                            _38442 = _38443;
                        }
                        _38441 = _38442;
                    }
                    _27389 = _38474 + (_38441.xyz * (_27360 ? ((_38404 == 0) ? (1.0 - _27355) : _27355) : 1.0));
                }
                float3 _35806 = _38378;
                _35806.x = _38474.x;
                float2 _16530 = (_16470 + _16482) + (_16485 * _16512);
                float _28236 = fast::clamp(log2(fast::max(_16384, 1.0)) - 1.0, 0.0, 5.0);
                int _28239 = int(floor(_28236));
                float _28243 = _28236 - float(_28239);
                bool _28248 = (_28243 > 0.0199999995529651641845703125) && (_28239 < 5);
                float3 _38598;
                _38598 = float3(0.0);
                float3 _28277;
                for (int _38528 = 0; _38528 < 2; _38598 = _28277, _38528++)
                {
                    if ((_38528 > 0) && (!_28248))
                    {
                        break;
                    }
                    int _28263 = _28239 + _38528;
                    int _28301 = clamp(_28263, 1, 5);
                    float2 _28370 = (_16530 * _288.gLevel[_28301].xy) - float2(0.5);
                    float2 _28372 = floor(_28370);
                    float2 _28375 = _28370 - _28372;
                    float2 _28378 = _28375 * _28375;
                    float2 _28381 = _28378 * _28375;
                    float2 _28400 = (((_28381 * 3.0) - (_28378 * 6.0)) + float2(4.0)) * 0.16666667163372039794921875;
                    float2 _28413 = _28381 * 0.16666667163372039794921875;
                    float2 _28416 = (((((-_28381) + (_28378 * 3.0)) - (_28375 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _28400;
                    float2 _28420 = (((((_28381 * (-3.0)) + (_28378 * 3.0)) + (_28375 * 3.0)) + float2(1.0)) * 0.16666667163372039794921875) + _28413;
                    float2 _28432 = ((_28372 - float2(0.5)) + (_28400 / _28416)) * _288.gLevel[_28301].zw;
                    float2 _28444 = ((_28372 + float2(1.5)) + (_28413 / _28420)) * _288.gLevel[_28301].zw;
                    float4 _38565;
                    if (_28263 <= 0)
                    {
                        float2 _38564;
                        if (_288.gConv.x > 0.5)
                        {
                            float2 _35809 = _16530;
                            _35809.y = 1.0 - _16530.y;
                            _38564 = _35809;
                        }
                        else
                        {
                            _38564 = _16530;
                        }
                        _38565 = gBackdrop0.sample(gLinear, _38564, level(0.0));
                    }
                    else
                    {
                        float4 _38566;
                        if (_28263 == 1)
                        {
                            float2 _38557;
                            if (_288.gConv.x > 0.5)
                            {
                                float2 _35814 = _28432;
                                _35814.y = 1.0 - _28432.y;
                                _38557 = _35814;
                            }
                            else
                            {
                                _38557 = _28432;
                            }
                            float _28489 = _28432.y;
                            float2 _28490 = float2(_28444.x, _28489);
                            float2 _38558;
                            if (_288.gConv.x > 0.5)
                            {
                                float2 _35821 = _28490;
                                _35821.y = 1.0 - _28489;
                                _38558 = _35821;
                            }
                            else
                            {
                                _38558 = _28490;
                            }
                            float _28507 = _28444.y;
                            float2 _28508 = float2(_28432.x, _28507);
                            float2 _38560;
                            if (_288.gConv.x > 0.5)
                            {
                                float2 _35828 = _28508;
                                _35828.y = 1.0 - _28507;
                                _38560 = _35828;
                            }
                            else
                            {
                                _38560 = _28508;
                            }
                            float2 _38562;
                            if (_288.gConv.x > 0.5)
                            {
                                float2 _35835 = _28444;
                                _35835.y = 1.0 - _28444.y;
                                _38562 = _35835;
                            }
                            else
                            {
                                _38562 = _28444;
                            }
                            _38566 = (((gBackdrop1.sample(gLinear, _38557, level(0.0)) * (_28416.x * _28416.y)) + (gBackdrop1.sample(gLinear, _38558, level(0.0)) * (_28420.x * _28416.y))) + (gBackdrop1.sample(gLinear, _38560, level(0.0)) * (_28416.x * _28420.y))) + (gBackdrop1.sample(gLinear, _38562, level(0.0)) * (_28420.x * _28420.y));
                        }
                        else
                        {
                            float4 _38567;
                            if (_28263 == 2)
                            {
                                float2 _38550;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _35842 = _28432;
                                    _35842.y = 1.0 - _28432.y;
                                    _38550 = _35842;
                                }
                                else
                                {
                                    _38550 = _28432;
                                }
                                float _28619 = _28432.y;
                                float2 _28620 = float2(_28444.x, _28619);
                                float2 _38551;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _35849 = _28620;
                                    _35849.y = 1.0 - _28619;
                                    _38551 = _35849;
                                }
                                else
                                {
                                    _38551 = _28620;
                                }
                                float _28637 = _28444.y;
                                float2 _28638 = float2(_28432.x, _28637);
                                float2 _38553;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _35856 = _28638;
                                    _35856.y = 1.0 - _28637;
                                    _38553 = _35856;
                                }
                                else
                                {
                                    _38553 = _28638;
                                }
                                float2 _38555;
                                if (_288.gConv.x > 0.5)
                                {
                                    float2 _35863 = _28444;
                                    _35863.y = 1.0 - _28444.y;
                                    _38555 = _35863;
                                }
                                else
                                {
                                    _38555 = _28444;
                                }
                                _38567 = (((gBackdrop2.sample(gLinear, _38550, level(0.0)) * (_28416.x * _28416.y)) + (gBackdrop2.sample(gLinear, _38551, level(0.0)) * (_28420.x * _28416.y))) + (gBackdrop2.sample(gLinear, _38553, level(0.0)) * (_28416.x * _28420.y))) + (gBackdrop2.sample(gLinear, _38555, level(0.0)) * (_28420.x * _28420.y));
                            }
                            else
                            {
                                float4 _38568;
                                if (_28263 == 3)
                                {
                                    float2 _38543;
                                    if (_288.gConv.x > 0.5)
                                    {
                                        float2 _35870 = _28432;
                                        _35870.y = 1.0 - _28432.y;
                                        _38543 = _35870;
                                    }
                                    else
                                    {
                                        _38543 = _28432;
                                    }
                                    float _28749 = _28432.y;
                                    float2 _28750 = float2(_28444.x, _28749);
                                    float2 _38544;
                                    if (_288.gConv.x > 0.5)
                                    {
                                        float2 _35877 = _28750;
                                        _35877.y = 1.0 - _28749;
                                        _38544 = _35877;
                                    }
                                    else
                                    {
                                        _38544 = _28750;
                                    }
                                    float _28767 = _28444.y;
                                    float2 _28768 = float2(_28432.x, _28767);
                                    float2 _38546;
                                    if (_288.gConv.x > 0.5)
                                    {
                                        float2 _35884 = _28768;
                                        _35884.y = 1.0 - _28767;
                                        _38546 = _35884;
                                    }
                                    else
                                    {
                                        _38546 = _28768;
                                    }
                                    float2 _38548;
                                    if (_288.gConv.x > 0.5)
                                    {
                                        float2 _35891 = _28444;
                                        _35891.y = 1.0 - _28444.y;
                                        _38548 = _35891;
                                    }
                                    else
                                    {
                                        _38548 = _28444;
                                    }
                                    _38568 = (((gBackdrop3.sample(gLinear, _38543, level(0.0)) * (_28416.x * _28416.y)) + (gBackdrop3.sample(gLinear, _38544, level(0.0)) * (_28420.x * _28416.y))) + (gBackdrop3.sample(gLinear, _38546, level(0.0)) * (_28416.x * _28420.y))) + (gBackdrop3.sample(gLinear, _38548, level(0.0)) * (_28420.x * _28420.y));
                                }
                                else
                                {
                                    float4 _38569;
                                    if (_28263 == 4)
                                    {
                                        float2 _38536;
                                        if (_288.gConv.x > 0.5)
                                        {
                                            float2 _35898 = _28432;
                                            _35898.y = 1.0 - _28432.y;
                                            _38536 = _35898;
                                        }
                                        else
                                        {
                                            _38536 = _28432;
                                        }
                                        float _28879 = _28432.y;
                                        float2 _28880 = float2(_28444.x, _28879);
                                        float2 _38537;
                                        if (_288.gConv.x > 0.5)
                                        {
                                            float2 _35905 = _28880;
                                            _35905.y = 1.0 - _28879;
                                            _38537 = _35905;
                                        }
                                        else
                                        {
                                            _38537 = _28880;
                                        }
                                        float _28897 = _28444.y;
                                        float2 _28898 = float2(_28432.x, _28897);
                                        float2 _38539;
                                        if (_288.gConv.x > 0.5)
                                        {
                                            float2 _35912 = _28898;
                                            _35912.y = 1.0 - _28897;
                                            _38539 = _35912;
                                        }
                                        else
                                        {
                                            _38539 = _28898;
                                        }
                                        float2 _38541;
                                        if (_288.gConv.x > 0.5)
                                        {
                                            float2 _35919 = _28444;
                                            _35919.y = 1.0 - _28444.y;
                                            _38541 = _35919;
                                        }
                                        else
                                        {
                                            _38541 = _28444;
                                        }
                                        _38569 = (((gBackdrop4.sample(gLinear, _38536, level(0.0)) * (_28416.x * _28416.y)) + (gBackdrop4.sample(gLinear, _38537, level(0.0)) * (_28420.x * _28416.y))) + (gBackdrop4.sample(gLinear, _38539, level(0.0)) * (_28416.x * _28420.y))) + (gBackdrop4.sample(gLinear, _38541, level(0.0)) * (_28420.x * _28420.y));
                                    }
                                    else
                                    {
                                        float2 _38529;
                                        if (_288.gConv.x > 0.5)
                                        {
                                            float2 _35926 = _28432;
                                            _35926.y = 1.0 - _28432.y;
                                            _38529 = _35926;
                                        }
                                        else
                                        {
                                            _38529 = _28432;
                                        }
                                        float _29009 = _28432.y;
                                        float2 _29010 = float2(_28444.x, _29009);
                                        float2 _38530;
                                        if (_288.gConv.x > 0.5)
                                        {
                                            float2 _35933 = _29010;
                                            _35933.y = 1.0 - _29009;
                                            _38530 = _35933;
                                        }
                                        else
                                        {
                                            _38530 = _29010;
                                        }
                                        float _29027 = _28444.y;
                                        float2 _29028 = float2(_28432.x, _29027);
                                        float2 _38532;
                                        if (_288.gConv.x > 0.5)
                                        {
                                            float2 _35940 = _29028;
                                            _35940.y = 1.0 - _29027;
                                            _38532 = _35940;
                                        }
                                        else
                                        {
                                            _38532 = _29028;
                                        }
                                        float2 _38534;
                                        if (_288.gConv.x > 0.5)
                                        {
                                            float2 _35947 = _28444;
                                            _35947.y = 1.0 - _28444.y;
                                            _38534 = _35947;
                                        }
                                        else
                                        {
                                            _38534 = _28444;
                                        }
                                        _38569 = (((gBackdrop5.sample(gLinear, _38529, level(0.0)) * (_28416.x * _28416.y)) + (gBackdrop5.sample(gLinear, _38530, level(0.0)) * (_28420.x * _28416.y))) + (gBackdrop5.sample(gLinear, _38532, level(0.0)) * (_28416.x * _28420.y))) + (gBackdrop5.sample(gLinear, _38534, level(0.0)) * (_28420.x * _28420.y));
                                    }
                                    _38568 = _38569;
                                }
                                _38567 = _38568;
                            }
                            _38566 = _38567;
                        }
                        _38565 = _38566;
                    }
                    _28277 = _38598 + (_38565.xyz * (_28248 ? ((_38528 == 0) ? (1.0 - _28243) : _28243) : 1.0));
                }
                _35806.z = _38598.z;
                _38768 = _35806;
            }
            else
            {
                _38768 = _38378;
            }
            float3 _38988;
            if (_16412 > 0.0)
            {
                float2 _16553 = _16470 + (((_38306 * fast::min(_16403 * 0.5, 16.0)) * _288.gDisplay.zw) * _288.gTarget.zw);
                float _29124 = fast::clamp(log2(fast::max(16.0 * _288.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _29127 = int(floor(_29124));
                float _29131 = _29124 - float(_29127);
                bool _29136 = (_29131 > 0.0199999995529651641845703125) && (_29127 < 5);
                float3 _38753;
                _38753 = float3(0.0);
                float3 _29165;
                for (int _38738 = 0; _38738 < 2; _38753 = _29165, _38738++)
                {
                    if ((_38738 > 0) && (!_29136))
                    {
                        break;
                    }
                    int _29151 = _29127 + _38738;
                    float2 _38739;
                    if (_288.gConv.x > 0.5)
                    {
                        float2 _35954 = _16553;
                        _35954.y = 1.0 - _16553.y;
                        _38739 = _35954;
                    }
                    else
                    {
                        _38739 = _16553;
                    }
                    float4 _38740;
                    if (_29151 <= 0)
                    {
                        _38740 = gBackdrop0.sample(gLinear, _38739, level(0.0));
                    }
                    else
                    {
                        float4 _38741;
                        if (_29151 == 1)
                        {
                            _38741 = gBackdrop1.sample(gLinear, _38739, level(0.0));
                        }
                        else
                        {
                            float4 _38742;
                            if (_29151 == 2)
                            {
                                _38742 = gBackdrop2.sample(gLinear, _38739, level(0.0));
                            }
                            else
                            {
                                float4 _38743;
                                if (_29151 == 3)
                                {
                                    _38743 = gBackdrop3.sample(gLinear, _38739, level(0.0));
                                }
                                else
                                {
                                    float4 _38744;
                                    if (_29151 == 4)
                                    {
                                        _38744 = gBackdrop4.sample(gLinear, _38739, level(0.0));
                                    }
                                    else
                                    {
                                        _38744 = gBackdrop5.sample(gLinear, _38739, level(0.0));
                                    }
                                    _38743 = _38744;
                                }
                                _38742 = _38743;
                            }
                            _38741 = _38742;
                        }
                        _38740 = _38741;
                    }
                    _29165 = _38753 + (_38740.xyz * (_29136 ? ((_38738 == 0) ? (1.0 - _29131) : _29131) : 1.0));
                }
                _38988 = _38753;
            }
            else
            {
                _38988 = float3(0.5);
            }
            float _38784;
            if (in.i_shape.x > 0.001000000047497451305389404296875)
            {
                int _29254 = clamp(int(rint(log2(36.0 * _288.gDisplay.z) - 1.0)), 1, 4);
                float2 _38759;
                if (_288.gConv.x > 0.5)
                {
                    float2 _35958 = _16470;
                    _35958.y = 1.0 - _16470.y;
                    _38759 = _35958;
                }
                else
                {
                    _38759 = _16470;
                }
                float4 _38760;
                if (_29254 <= 0)
                {
                    _38760 = gBackdrop0.sample(gLinear, _38759, level(0.0));
                }
                else
                {
                    float4 _38761;
                    if (_29254 == 1)
                    {
                        _38761 = gBackdrop1.sample(gLinear, _38759, level(0.0));
                    }
                    else
                    {
                        float4 _38762;
                        if (_29254 == 2)
                        {
                            _38762 = gBackdrop2.sample(gLinear, _38759, level(0.0));
                        }
                        else
                        {
                            float4 _38763;
                            if (_29254 == 3)
                            {
                                _38763 = gBackdrop3.sample(gLinear, _38759, level(0.0));
                            }
                            else
                            {
                                float4 _38764;
                                if (_29254 == 4)
                                {
                                    _38764 = gBackdrop4.sample(gLinear, _38759, level(0.0));
                                }
                                else
                                {
                                    _38764 = gBackdrop5.sample(gLinear, _38759, level(0.0));
                                }
                                _38763 = _38764;
                            }
                            _38762 = _38763;
                        }
                        _38761 = _38762;
                    }
                    _38760 = _38761;
                }
                _38784 = dot(_38760.xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _38784 = 0.5;
            }
            _38986 = _38988;
            _38783 = _38784;
            _38765 = _38768;
        }
        else
        {
            _38986 = float3(0.5);
            _38783 = 0.5;
            _38765 = float3(0.5);
        }
        float3 _16581 = mix(float3(dot(_38765, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _38765, float3(gFxData_1._data[_16303].x)) + float3(gFxData_1._data[_16303].y);
        float3 _38879;
        if (in.i_shape.x > 0.001000000047497451305389404296875)
        {
            float _16591 = fast::clamp(fast::max(_38783 + gFxData_1._data[_16303].y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _16599 = mix(_16591, dot(gFxData_1._data[_16295].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), in.i_shape.x);
            _38879 = select(mix(_16581, float3(1.0), float3((_16599 - _16591) / fast::max(1.0 - _16591, 0.001000000047497451305389404296875))), _16581 * (_16599 / _16591), bool3(_16599 < _16591));
        }
        else
        {
            _38879 = _16581;
        }
        float2 _16637 = fast::clamp((in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875)), float2(0.0), float2(1.0));
        float2 _39099;
        if (_330.gDrawInfo.w > 0.5)
        {
            float2 _29366 = select(_330.gDrawInfo.yz, _330.gDrawInfo.yz - in.i_local, bool2(_330.gDrawInfo.w > 1.0));
            _39099 = _29366 / float2(fast::max(length(_29366), 9.9999997473787516355514526367188e-05));
        }
        else
        {
            _39099 = float2(cos(gFxData_1._data[_16303].w), sin(gFxData_1._data[_16303].w));
        }
        float _16693 = -_36352;
        float _16700 = _5545 * ((0.5 * abs(_38306.x)) + 0.0199999995529651641845703125);
        float _16707 = _5545 * ((0.5 * abs(_38306.y)) + 0.0199999995529651641845703125);
        float _39103;
        if ((((_16693 + _16700) + _16707) > 0.0) && (((_16693 - _16700) - _16707) < (0.5 * _16403)))
        {
            float _29385 = fast::max((_16693 + _16700) + _16707, 0.0) / _16403;
            float _29389 = fast::clamp(sqrt(2.0 * _29385), 0.12399999797344207763671875, 1.0);
            float _29391 = 1.0 - _29389;
            float _29394 = _29391 * _29391;
            float _29448 = fast::max((_16693 + _16700) - _16707, 0.0) / _16403;
            float _29452 = fast::clamp(sqrt(2.0 * _29448), 0.12399999797344207763671875, 1.0);
            float _29454 = 1.0 - _29452;
            float _29457 = _29454 * _29454;
            float _29511 = fast::max((_16693 - _16700) + _16707, 0.0) / _16403;
            float _29515 = fast::clamp(sqrt(2.0 * _29511), 0.12399999797344207763671875, 1.0);
            float _29517 = 1.0 - _29515;
            float _29520 = _29517 * _29517;
            float _29574 = fast::max((_16693 - _16700) - _16707, 0.0) / _16403;
            float _29578 = fast::clamp(sqrt(2.0 * _29574), 0.12399999797344207763671875, 1.0);
            float _29580 = 1.0 - _29578;
            float _29583 = _29580 * _29580;
            float _16755 = ((((((_29385 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _29385) * _29385) : ((_29385 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _29389) * _29389)) + ((((_29394 * _29394) * _29394) * _29391) * (0.02380952425301074981689453125 + (_29391 * ((-0.0386904776096343994140625) + (_29391 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_29385 - 0.5))))) * _16403) * _16403) - ((((_29448 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _29448) * _29448) : ((_29448 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _29452) * _29452)) + ((((_29457 * _29457) * _29457) * _29454) * (0.02380952425301074981689453125 + (_29454 * ((-0.0386904776096343994140625) + (_29454 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_29448 - 0.5))))) * _16403) * _16403)) - ((((_29511 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _29511) * _29511) : ((_29511 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _29515) * _29515)) + ((((_29520 * _29520) * _29520) * _29517) * (0.02380952425301074981689453125 + (_29517 * ((-0.0386904776096343994140625) + (_29517 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_29511 - 0.5))))) * _16403) * _16403)) + ((((_29574 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _29574) * _29574) : ((_29574 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _29578) * _29578)) + ((((_29583 * _29583) * _29583) * _29580) * (0.02380952425301074981689453125 + (_29580 * ((-0.0386904776096343994140625) + (_29580 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_29574 - 0.5))))) * _16403) * _16403);
            float _16763 = dot(_38306, _39099);
            _39103 = (((_16755 / ((4.0 * _16700) * _16707)) * (pow(fast::clamp(_16763, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(fast::clamp(-_16763, 0.0, 1.0), 1.5)))) * gFxData_1._data[_16303].z) * 1.60000002384185791015625;
        }
        else
        {
            _39103 = 0.0;
        }
        float _16799 = fast::clamp(0.5 + (0.5 * dot((in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5)) / _16395, _39099)), 0.0, 1.0);
        _41991 = _39103;
        _41032 = gFxData_1._data[_16311];
        _40710 = float4((mix(mix(_38879, gFxData_1._data[_16295].xyz, float3(fast::clamp(gFxData_1._data[_16295].w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _16418))) + (0.3499999940395355224609375 * ((dot(gFxData_1._data[_16295].xyz, float3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _16637.y) : _16637.y))), 0.0, 1.0))), (_38986 * 1.10000002384185791015625) + float3(0.07999999821186065673828125), float3(pow(1.0 - _38883.z, 5.0) * 0.3499999940395355224609375)) + float3((((0.039999999105930328369140625 * _16799) * _16799) + (0.0500000007450580596923828125 * (1.0 - _16418))) * gFxData_1._data[_16303].z)) * _36985, _36985) + (_39403 * (1.0 - _36985));
    }
    else
    {
        _41991 = 0.0;
        _41032 = _36245;
        _40710 = _39403;
    }
    float4 _40722;
    float4 _40726;
    if ((in.i_flags.x & _4267) != 0u)
    {
        float4 _39691;
        float4 _40190;
        if (in.i_flags.y != 0u)
        {
            _40190 = gFxData_1._data[(in.i_instance * 24u) + 3u];
            _39691 = gFxData_1._data[(in.i_instance * 24u) + 4u];
        }
        else
        {
            _40190 = float4(0.0);
            _39691 = float4(0.0);
        }
        float4 _40697;
        do
        {
            if (in.i_flags.y == 0u)
            {
                _40697 = in.i_fill0;
                break;
            }
            float2 _29687 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float2 _29697 = fwidth(in.i_local);
            float _29699 = fast::max(length(_29697), 9.9999997473787516355514526367188e-05);
            float _40689;
            float _40693;
            if ((in.i_flags.y == 1u) || (in.i_flags.y == 4u))
            {
                float _29708 = cos(_39691.x);
                float _29711 = sin(_39691.x);
                float _29737 = ((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_29708, _29711)) / fast::max(0.5 * ((abs(_29708) * _29687.x) + (abs(_29711) * _29687.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (in.i_flags.y == 4u)
                {
                    float _29752 = fast::clamp((_29737 - _39691.y) / fast::max(_39691.z - _39691.y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _40697 = float4(fast::clamp(abs((fract(float3(_29752 * 0.800000011920928955078125) + float3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - float3(3.0)) - float3(1.0), float3(0.0), float3(1.0)), in.i_fill0.w * pow(fast::max(sin(_29752 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _40693 = -1.0;
                _40689 = _29737;
            }
            else
            {
                float _40690;
                float _40694;
                if (in.i_flags.y == 2u)
                {
                    _40694 = -1.0;
                    _40690 = length(in.i_local - (in.i_rect.xy + (_39691.xy * _29687))) / fast::max(_39691.z * fast::max(_29687.x, _29687.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    float2 _29820 = in.i_local - (in.i_rect.xy + (_39691.xy * _29687));
                    float _29831 = fract(((precise::atan2(_29820.y, _29820.x) - _39691.z) * 0.15915493667125701904296875) + 1.0);
                    float _40691;
                    float _40695;
                    if (_39691.w > 0.5)
                    {
                        _40695 = -1.0;
                        _40691 = 0.5 - (0.5 * cos(_29831 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _29851 = (((_29831 < 0.5) ? _29831 : (_29831 - 1.0)) * 6.283185482025146484375) * length(_29820);
                        float _40696;
                        if (abs(_29851) < _29699)
                        {
                            _40696 = fast::clamp(((_29851 / _29699) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _40696 = -1.0;
                        }
                        _40695 = _40696;
                        _40691 = _29831;
                    }
                    _40694 = _40695;
                    _40690 = _40691;
                }
                _40693 = _40694;
                _40689 = _40690;
            }
            float4 _29880 = float4(in.i_fill0.xyz * in.i_fill0.w, in.i_fill0.w);
            float4 _29892 = float4(_40190.xyz * _40190.w, _40190.w);
            float4 _29906 = select(mix(_29880, _29892, float4(fast::clamp(_40689, 0.0, 1.0))), mix(_29892, _29880, float4(_40693)), bool4(_40693 >= 0.0));
            _40697 = select(float4(0.0), float4(_29906.xyz / float3(_29906.w), _29906.w), bool4(_29906.w > 9.9999997473787516355514526367188e-06));
            break;
        } while(false);
        float4 _40698;
        float4 _41021;
        if ((in.i_flags.x & _4296) != 0u)
        {
            uint _29931 = (in.i_instance * 24u) + 18u;
            _41021 = gFxData_1._data[_29931];
            _40698 = _40697 * gTex.sample(gLinear, mix(gFxData_1._data[_29931].xy, gFxData_1._data[_29931].zw, (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875))));
        }
        else
        {
            _41021 = _36241;
            _40698 = _40697;
        }
        float _29941 = fast::clamp(_40698.w * _36985, 0.0, 1.0);
        _40726 = _41021;
        _40722 = float4(_40698.xyz * _29941, _29941) + (_40710 * (1.0 - _29941));
    }
    else
    {
        _40726 = _36241;
        _40722 = _40710;
    }
    float4 _41945;
    if ((in.i_flags.x & _4349) != 0u)
    {
        uint _29965 = (in.i_instance * 24u) + 21u;
        uint _29973 = (in.i_instance * 24u) + 3u;
        float2 _6198 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float2 _6205 = (in.i_local - in.i_rect.xy) / _6198;
        float _6216 = ((gFxData_1._data[_29973].x >= 0.0) ? gFxData_1._data[_29973].x : _288.gTime.x) * gFxData_1._data[_29965].z;
        float _6219 = _6205.x * 2.0;
        float _6220 = _6219 - 1.0;
        float _6225 = _6205.y * _6198.y;
        float _6228 = fast::max(gFxData_1._data[_29965].w, 0.001000000047497451305389404296875);
        float _6233 = fast::clamp(1.0 - (_6220 * _6220), 0.0, 1.0);
        float _6235 = pow(_6233, 1.2999999523162841796875);
        float _6237 = pow(_6233, 0.699999988079071044921875);
        float _6240 = fast::clamp(_6216 * 1.4285714626312255859375, 0.0, 1.0);
        float _6256 = (0.25 + (0.75 * ((_6240 * _6240) * (3.0 - (2.0 * _6240))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_6216 * 2.099999904632568359375)));
        float _6265 = ((gFxData_1._data[_29965].y * _6198.y) * _6256) * _6235;
        float _6291 = (_6198.y * (0.5 + ((gFxData_1._data[_29965].x * (0.5 - (_6220 * _6220))) * 0.5))) + (((0.14000000059604644775390625 * _6198.y) * _6235) * sin(((_6220 * 2.400000095367431640625) - (_6216 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _40716;
        float _40717;
        float3 _40718;
        _40718 = float3(0.0);
        _40717 = _6291;
        _40716 = _6291;
        float3 _6363;
        float _43994;
        float _43995;
        for (int _40715 = 0; _40715 < 4; _40718 = _6363, _40717 = _43995, _40716 = _43994, _40715++)
        {
            float _6315 = _6291 + ((_6265 * _4511[_40715].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_6216 * 1.7000000476837158203125) + _4528[_40715].y))));
            _43994 = (_40715 == 0) ? _6315 : _40716;
            _43995 = (_40715 == 2) ? _6315 : _40717;
            float _6330 = _6228 * _4511[_40715].y;
            float _6335 = (_6225 - _6315) / _6330;
            float _6340 = _6228 * _4511[_40715].z;
            float3 _42911;
            _42911 = float3(0.0);
            for (int _42910 = 0; _42910 < 6; )
            {
                float _29998 = ((_6225 - _6315) - (_6340 * ((float(_42910) * 0.4000000059604644775390625) - 1.0))) / _6330;
                _42911 += (_2899[_42910] * exp((-_29998) * _29998));
                _42910++;
                continue;
            }
            _6363 = _40718 + (mix(_42911 * float3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), float3(exp((-_6335) * _6335)), float3(_4528[_40715].x)) * (_4511[_40715].w * _6237));
        }
        float _6369 = _6228 * 1.5;
        float _6399 = fast::clamp((_6225 - _40716) / fast::max(_40717 - _40716, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _6427 = (_6225 - (_40717 - (_6228 * 3.0))) / (((_6198.y * 0.0900000035762786865234375) + (_6265 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _6430 = (_6219 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _6455 = ((_6225 - _40716) + (_6228 * 5.0)) / (_6228 * 7.0);
        float3 _6481 = float3(1.0) - exp((-((((_40718 + (mix(float3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), float3(1.0), float3(_6399)) * ((((1.0 / (1.0 + exp((-((_6225 - _40716) - (_6228 * 2.0))) / _6369))) / (1.0 + exp((-(_40717 - _6225)) / _6369))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_6399, 2.5)))) * _6237))) + (float3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_6427) * _6427) - (_6430 * _6430)) * (0.5 + (1.10000002384185791015625 * _6256))))) + (float3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_6455) * _6455) * _6235) * 0.100000001490116119384765625))) * mix(float3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), float3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), float3(0.5 + (0.5 * sin(_6216 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _6491 = (in.i_fill0.w * smoothstep(0.0, 0.119999997317790985107421875, _6205.y)) * smoothstep(1.0, 0.87999999523162841796875, _6205.y);
        float _6510 = (fast::clamp(fast::max(_6481.x, fast::max(_6481.y, _6481.z)), 0.0, 1.0) * _6491) * _36985;
        _41945 = float4((_6481 * _6491) * _36985, _6510) + (_40722 * (1.0 - _6510));
    }
    else
    {
        _41945 = _40722;
    }
    float4 _41986;
    if (((in.i_flags.x & _4780) != 0u) && ((in.i_flags.x & _4784) != 0u))
    {
        uint _30030 = (in.i_instance * 24u) + 7u;
        uint _30038 = (in.i_instance * 24u) + 8u;
        float2 _40723;
        if (_330.gDrawInfo.w > 0.5)
        {
            float2 _30087 = select(_330.gDrawInfo.yz, _330.gDrawInfo.yz - ((in.i_rect.xy + in.i_rect.zw) * 0.5), bool2(_330.gDrawInfo.w > 1.0));
            _40723 = (-(_30087 / float2(fast::max(length(_30087), 9.9999997473787516355514526367188e-05)))) * length(gFxData_1._data[_30038].zw);
        }
        else
        {
            _40723 = gFxData_1._data[_30038].zw;
        }
        float2 _6543 = in.i_local - _40723;
        float2 _30138 = (in.i_rect.xy + in.i_rect.zw) * 0.5;
        float2 _30147 = fast::max((in.i_rect.zw - in.i_rect.xy) * 0.5, float2(0.001000000047497451305389404296875));
        float _41814;
        if ((in.i_flags.x & _2192) != 0u)
        {
            float _30264 = 1.0 / fast::max(in.i_shape.z, 9.9999999747524270787835121154785e-07);
            float4 _30298 = gTex.sample(gLinear, mix(_40726.xy, _40726.zw, (fast::clamp(_6543, _36247.xy, _36247.zw) - _36247.xy) / fast::max(_36247.zw - _36247.xy, float2(9.9999999747524270787835121154785e-07))), level(0.0));
            float _30322 = (_30298.w - 0.5) * in.i_radii.y;
            float2 _30363 = abs(_6543 - ((_36247.xy + _36247.zw) * 0.5)) - fast::max(((_36247.zw - _36247.xy) * 0.5) - float2(((0.5 * in.i_radii.y) + 1.0) * _30264), float2(0.0));
            _41814 = fast::max((-mix((fast::max(fast::min(_30298.x, _30298.y), fast::min(fast::max(_30298.x, _30298.y), _30298.z)) - 0.5) * in.i_radii.x, _30322, fast::clamp(abs(_30322) - ((0.5 * in.i_radii.x) - 1.5), 0.0, 1.0))) * _30264, length(fast::max(_30363, float2(0.0))) + fast::min(fast::max(_30363.x, _30363.y), 0.0)) - in.i_shape.w;
        }
        else
        {
            float _41815;
            if ((in.i_flags.x & _2207) != 0u)
            {
                float _41766;
                do
                {
                    bool _30429 = in.i_radii.w > 0.5;
                    float _30433 = floor((in.i_radii.z + 1.0) * 0.5);
                    float _30437 = floor((_30433 + 127.0) * 0.0078125);
                    float _41759;
                    float _41761;
                    if (_30429)
                    {
                        uint _30613 = uint(in.i_radii.z);
                        uint _30616 = (_30613 + 31u) >> 5u;
                        uint _30699 = (in.i_instance + 1u) * 24u;
                        uint _41751;
                        uint _41752;
                        _41752 = 0u;
                        _41751 = _30699;
                        uint _30726;
                        float _43996;
                        uint _43997;
                        uint _43998;
                        uint _41750 = 0u;
                        uint _41810 = _30699;
                        float _41811 = 3.0000000054977557577780399428115e+38;
                        for (; _41750 < _30616; _41811 = _43996, _41810 = _30726, _41752 = _43997, _41751 = _43998, _41750++)
                        {
                            uint _30704 = _41810 + 20u;
                            float2 _30719 = fast::max(fast::max(gFxData_1._data[_30704].xy - _6543, _6543 - gFxData_1._data[_30704].zw), float2(0.0));
                            float _30722 = dot(_30719, _30719);
                            bool _30634 = _30722 < _41811;
                            _43996 = _30634 ? _30722 : _41811;
                            _43997 = _30634 ? _41750 : _41752;
                            _43998 = _30634 ? _41810 : _41751;
                            _30726 = _41810 + 24u;
                        }
                        uint _30764 = _41752 * 32u;
                        uint _30768 = min((_30613 - _30764), 32u);
                        uint _30872 = _41751 + 21u;
                        uint _30879 = _41751 + 22u;
                        float _41755;
                        uint _41756;
                        _41756 = 4294967295u;
                        _41755 = 1000000015047466219876688855040.0;
                        float _43747;
                        uint _43748;
                        for (uint _41754 = 0u; (_41754 * 8u) < _30768; _41756 = _43748, _41755 = _43747, _41754++)
                        {
                            uint _30886 = _41751 + (16u + _41754);
                            float _30906 = ((sqrt(_41755) * 1.000003814697265625) + 0.03125) + ((abs(_6543.x) + abs(_6543.y)) * 3.814697265625e-06);
                            float2 _30926 = fast::max(fast::max(gFxData_1._data[_30886].xy - _6543, _6543 - gFxData_1._data[_30886].zw), float2(0.0));
                            if (dot(_30926, _30926) > (_30906 * _30906))
                            {
                                _43748 = _41756;
                                _43747 = _41755;
                                continue;
                            }
                            float4 _30794 = select(gFxData_1._data[_30879], gFxData_1._data[_30872], bool4(_41754 < 2u));
                            uint _30808 = min(_30768, ((_41754 * 8u) + 8u));
                            uint _30810 = _41754 * 8u;
                            float2 _41795;
                            float _41796;
                            uint _41797;
                            _41797 = _41756;
                            _41796 = _41755;
                            _41795 = select(_30794.xy, _30794.zw, bool2((_41754 & 1u) != 0u));
                            float2 _43791;
                            float _43792;
                            uint _43793;
                            for (uint _41794 = _30810; _41794 < _30808; _41797 = _43793, _41796 = _43792, _41795 = _43791, _41794 += 2u)
                            {
                                uint _30934 = _41751 + (_41794 >> 1u);
                                uint _30823 = _30764 + _41794;
                                float2 _30944 = _41795 - gFxData_1._data[_30934].xy;
                                float2 _30947 = _6543 - gFxData_1._data[_30934].xy;
                                float2 _30960 = _30947 - (_30944 * fast::clamp(dot(_30947, _30944) / fast::max(dot(_30944, _30944), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _30963 = dot(_30960, _30960);
                                bool _30974 = (_30963 < _41796) || ((_30963 == _41796) && (_30823 < _41797));
                                uint _43999 = _30974 ? _30823 : _41797;
                                float _44000 = _30974 ? _30963 : _41796;
                                if ((_41794 + 1u) < _30808)
                                {
                                    uint _30845 = (_30764 + _41794) + 1u;
                                    float2 _30987 = gFxData_1._data[_30934].xy - gFxData_1._data[_30934].zw;
                                    float2 _30990 = _6543 - gFxData_1._data[_30934].zw;
                                    float2 _31003 = _30990 - (_30987 * fast::clamp(dot(_30990, _30987) / fast::max(dot(_30987, _30987), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _31006 = dot(_31003, _31003);
                                    bool _31017 = (_31006 < _44000) || ((_31006 == _44000) && (_30845 < _43999));
                                    _43793 = _31017 ? _30845 : _43999;
                                    _43792 = _31017 ? _31006 : _44000;
                                    _43791 = gFxData_1._data[_30934].zw;
                                }
                                else
                                {
                                    _43793 = _43999;
                                    _43792 = _44000;
                                    _43791 = gFxData_1._data[_30934].xy;
                                }
                            }
                            _43748 = _41797;
                            _43747 = _41796;
                        }
                        uint _31026 = (in.i_instance + 1u) * 24u;
                        float _41758;
                        _41758 = _41755;
                        uint _31374;
                        float _43749;
                        uint _43775;
                        for (uint _41757 = 0u, _41769 = _31026, _41770 = _41756; _41757 < _30616; _41770 = _43775, _41769 = _31374, _41758 = _43749, _41757++)
                        {
                            uint _31031 = _41769 + 20u;
                            float _31051 = ((sqrt(_41758) * 1.000003814697265625) + 0.03125) + ((abs(_6543.x) + abs(_6543.y)) * 3.814697265625e-06);
                            float2 _31071 = fast::max(fast::max(gFxData_1._data[_31031].xy - _6543, _6543 - gFxData_1._data[_31031].zw), float2(0.0));
                            if ((_41757 != _41752) && (!(dot(_31071, _31071) > (_31051 * _31051))))
                            {
                                uint _31112 = _41757 * 32u;
                                uint _31116 = min((_30613 - _31112), 32u);
                                uint _31220 = _41769 + 21u;
                                uint _31227 = _41769 + 22u;
                                float _41772;
                                uint _41773;
                                _41773 = _41770;
                                _41772 = _41758;
                                float _43776;
                                uint _43777;
                                for (uint _41771 = 0u; (_41771 * 8u) < _31116; _41773 = _43777, _41772 = _43776, _41771++)
                                {
                                    uint _31234 = _41769 + (16u + _41771);
                                    float _31254 = ((sqrt(_41772) * 1.000003814697265625) + 0.03125) + ((abs(_6543.x) + abs(_6543.y)) * 3.814697265625e-06);
                                    float2 _31274 = fast::max(fast::max(gFxData_1._data[_31234].xy - _6543, _6543 - gFxData_1._data[_31234].zw), float2(0.0));
                                    if (dot(_31274, _31274) > (_31254 * _31254))
                                    {
                                        _43777 = _41773;
                                        _43776 = _41772;
                                        continue;
                                    }
                                    float4 _31142 = select(gFxData_1._data[_31227], gFxData_1._data[_31220], bool4(_41771 < 2u));
                                    uint _31156 = min(_31116, ((_41771 * 8u) + 8u));
                                    uint _31158 = _41771 * 8u;
                                    float2 _41779;
                                    float _41780;
                                    uint _41781;
                                    _41781 = _41773;
                                    _41780 = _41772;
                                    _41779 = select(_31142.xy, _31142.zw, bool2((_41771 & 1u) != 0u));
                                    float2 _43788;
                                    float _43789;
                                    uint _43790;
                                    for (uint _41778 = _31158; _41778 < _31156; _41781 = _43790, _41780 = _43789, _41779 = _43788, _41778 += 2u)
                                    {
                                        uint _31282 = _41769 + (_41778 >> 1u);
                                        uint _31171 = _31112 + _41778;
                                        float2 _31292 = _41779 - gFxData_1._data[_31282].xy;
                                        float2 _31295 = _6543 - gFxData_1._data[_31282].xy;
                                        float2 _31308 = _31295 - (_31292 * fast::clamp(dot(_31295, _31292) / fast::max(dot(_31292, _31292), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _31311 = dot(_31308, _31308);
                                        bool _31322 = (_31311 < _41780) || ((_31311 == _41780) && (_31171 < _41781));
                                        uint _44003 = _31322 ? _31171 : _41781;
                                        float _44004 = _31322 ? _31311 : _41780;
                                        if ((_41778 + 1u) < _31156)
                                        {
                                            uint _31193 = (_31112 + _41778) + 1u;
                                            float2 _31335 = gFxData_1._data[_31282].xy - gFxData_1._data[_31282].zw;
                                            float2 _31338 = _6543 - gFxData_1._data[_31282].zw;
                                            float2 _31351 = _31338 - (_31335 * fast::clamp(dot(_31338, _31335) / fast::max(dot(_31335, _31335), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _31354 = dot(_31351, _31351);
                                            bool _31365 = (_31354 < _44004) || ((_31354 == _44004) && (_31193 < _44003));
                                            _43790 = _31365 ? _31193 : _44003;
                                            _43789 = _31365 ? _31354 : _44004;
                                            _43788 = gFxData_1._data[_31282].zw;
                                        }
                                        else
                                        {
                                            _43790 = _44003;
                                            _43789 = _44004;
                                            _43788 = gFxData_1._data[_31282].xy;
                                        }
                                    }
                                    _43777 = _41781;
                                    _43776 = _41780;
                                }
                                _43775 = _41773;
                                _43749 = _41772;
                            }
                            else
                            {
                                _43775 = _41770;
                                _43749 = _41758;
                            }
                            _31374 = _41769 + 24u;
                        }
                        _41761 = 1.0;
                        _41759 = _41758;
                    }
                    else
                    {
                        float _30451 = floor((in.i_radii.z - 1.0) * 0.5);
                        float _31382 = floor(_30451 * 0.0625);
                        uint _31399 = (((in.i_instance + 1u) + uint(_31382)) * 24u) + uint(_30451 - (_31382 * 16.0));
                        float2 _41729;
                        if (_30429)
                        {
                            float _31409 = floor(0.0);
                            _41729 = gFxData_1._data[(((in.i_instance + 1u) + uint(_31409)) * 24u) + uint(_31409 * (-16.0))].xy;
                        }
                        else
                        {
                            _41729 = select(gFxData_1._data[_31399].zw, gFxData_1._data[_31399].xy, bool2(((in.i_radii.z - 1.0) - (_30451 * 2.0)) < 0.5));
                        }
                        float2 _41728;
                        float _41731;
                        float _41734;
                        _41734 = 1.0;
                        _41731 = 1000000015047466219876688855040.0;
                        _41728 = _41729;
                        float2 _41727;
                        float _41730;
                        float _41733;
                        for (float _41724 = 0.0; _41724 < 16.0; _41734 = _41733, _41731 = _41730, _41728 = _41727, _41724 += 1.0)
                        {
                            if (_41724 >= _30437)
                            {
                                break;
                            }
                            _41733 = _41734;
                            _41730 = _41731;
                            _41727 = _41728;
                            float2 _43739;
                            float _43741;
                            float _43743;
                            for (float _41725 = 0.0; _41725 < 128.0; _41733 = _43743, _41730 = _43741, _41727 = _43739, _41725 += 1.0)
                            {
                                float _30493 = (_41724 * 128.0) + _41725;
                                if (_30493 >= _30433)
                                {
                                    break;
                                }
                                float _31436 = floor(_30493 * 0.0625);
                                uint _31453 = (((in.i_instance + 1u) + uint(_31436)) * 24u) + uint(_30493 - (_31436 * 16.0));
                                float2 _31466 = _41727 - gFxData_1._data[_31453].xy;
                                float2 _31469 = _6543 - gFxData_1._data[_31453].xy;
                                float2 _31482 = _31469 - (_31466 * fast::clamp(dot(_31469, _31466) / fast::max(dot(_31466, _31466), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _31485 = dot(_31482, _31482);
                                float _44007 = (_31485 < _41730) ? _31485 : _41730;
                                bool _31497 = _6543.y >= gFxData_1._data[_31453].y;
                                bool _31502 = _6543.y < _41727.y;
                                bool _31513 = (_31466.x * _31469.y) > (_31466.y * _31469.x);
                                float _41739;
                                if (((_31497 && _31502) && _31513) || (((!_31497) && (!_31502)) && (!_31513)))
                                {
                                    _41739 = -_41733;
                                }
                                else
                                {
                                    _41739 = _41733;
                                }
                                if (((2.0 * _30493) + 1.0) < in.i_radii.z)
                                {
                                    float2 _31542 = gFxData_1._data[_31453].xy - gFxData_1._data[_31453].zw;
                                    float2 _31545 = _6543 - gFxData_1._data[_31453].zw;
                                    float2 _31558 = _31545 - (_31542 * fast::clamp(dot(_31545, _31542) / fast::max(dot(_31542, _31542), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _31561 = dot(_31558, _31558);
                                    bool _31573 = _6543.y >= gFxData_1._data[_31453].w;
                                    bool _31578 = _6543.y < gFxData_1._data[_31453].y;
                                    bool _31589 = (_31542.x * _31545.y) > (_31542.y * _31545.x);
                                    float _41743;
                                    if (((_31573 && _31578) && _31589) || (((!_31573) && (!_31578)) && (!_31589)))
                                    {
                                        _41743 = -_41739;
                                    }
                                    else
                                    {
                                        _41743 = _41739;
                                    }
                                    _43743 = _41743;
                                    _43741 = (_31561 < _44007) ? _31561 : _44007;
                                    _43739 = gFxData_1._data[_31453].zw;
                                }
                                else
                                {
                                    _43743 = _41739;
                                    _43741 = _44007;
                                    _43739 = gFxData_1._data[_31453].xy;
                                }
                            }
                        }
                        _41761 = _41734;
                        _41759 = _41731;
                    }
                    float _30546 = sqrt(_41759);
                    if (_30429)
                    {
                        _41766 = _30546 - in.i_radii.x;
                        break;
                    }
                    _41766 = (_41761 * _30546) - in.i_radii.x;
                    break;
                } while(false);
                _41815 = _41766;
            }
            else
            {
                float _41816;
                if (in.i_flags.z == 1u)
                {
                    float2 _30172 = _6543 - _30138;
                    float _41723;
                    do
                    {
                        if (in.i_shape.w >= 6.282185077667236328125)
                        {
                            _41723 = abs(length(_30172) - in.i_radii.x) - in.i_radii.y;
                            break;
                        }
                        float _31632 = in.i_shape.z + (in.i_shape.w * 0.5);
                        float _31634 = cos(_31632);
                        float _31636 = sin(_31632);
                        float _31645 = dot(_30172, float2(-_31636, _31634));
                        float _31648 = dot(_30172, float2(_31634, _31636));
                        float2 _31649 = float2(_31645, _31648);
                        float _31652 = abs(_31645);
                        _31649.x = _31652;
                        float _31655 = in.i_shape.w * 0.5;
                        float _31657 = sin(_31655);
                        float _31659 = cos(_31655);
                        _41723 = (((_31659 * _31652) > (_31657 * _31648)) ? length(_31649 - (float2(_31657, _31659) * in.i_radii.x)) : abs(length(_31649) - in.i_radii.x)) - in.i_radii.y;
                        break;
                    } while(false);
                    _41816 = _41723;
                }
                else
                {
                    float _41817;
                    if (in.i_flags.z == 2u)
                    {
                        float2 _31695 = _6543 - _36247.xy;
                        float2 _31698 = _36247.zw - _36247.xy;
                        _41817 = length(_31695 - (_31698 * fast::clamp(dot(_31695, _31698) / fast::max(dot(_31698, _31698), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_radii.x;
                    }
                    else
                    {
                        float2 _30199 = _6543 - _30138;
                        float _31763 = fast::min(_30147.x, _30147.y);
                        float _31766 = fast::min((_30199.x > 0.0) ? ((_30199.y > 0.0) ? in.i_radii.z : in.i_radii.y) : ((_30199.y > 0.0) ? in.i_radii.w : in.i_radii.x), _31763);
                        float _31772 = _31766 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _41712;
                        float _41713;
                        if (_31772 > _31763)
                        {
                            float _31786 = in.i_shape.y * fast::clamp((_31763 - _31766) / fast::max(0.60000002384185791015625 * _31766, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _41713 = _31786;
                            _41712 = _31766 * (1.0 + (0.60000002384185791015625 * _31786));
                        }
                        else
                        {
                            _41713 = in.i_shape.y;
                            _41712 = _31772;
                        }
                        float2 _31815 = (abs(_30199) - _30147) + float2(_41712);
                        float2 _31817 = fast::max(_31815, float2(0.0));
                        float _41714;
                        if ((_31817.x > 0.0) && (_31817.y > 0.0))
                        {
                            float _41715;
                            if ((_41713 > 0.001000000047497451305389404296875) && (_41712 > 9.9999997473787516355514526367188e-05))
                            {
                                float _31834 = 2.0 + (2.0 * _41713);
                                float2 _31839 = _31817 / float2(fast::max(_41712, 9.9999997473787516355514526367188e-05));
                                _41715 = pow(pow(_31839.x, _31834) + pow(_31839.y, _31834), 1.0 / _31834) * _41712;
                            }
                            else
                            {
                                _41715 = length(_31817);
                            }
                            _41714 = _41715;
                        }
                        else
                        {
                            _41714 = fast::max(_31817.x, _31817.y);
                        }
                        float _31874 = (fast::min(fast::max(_31815.x, _31815.y), 0.0) + _41714) - _41712;
                        float _41818;
                        if ((in.i_flags.x & _2278) != 0u)
                        {
                            float2 _30227 = fast::max((_36247.zw - _36247.xy) * 0.5, float2(0.001000000047497451305389404296875));
                            float2 _30230 = _6543 - ((_36247.xy + _36247.zw) * 0.5);
                            float _31922 = fast::min(_30227.x, _30227.y);
                            float _31925 = fast::min((_30230.x > 0.0) ? ((_30230.y > 0.0) ? _41032.x : _41032.x) : ((_30230.y > 0.0) ? _41032.x : _41032.x), _31922);
                            float _31931 = _31925 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                            float _41716;
                            float _41717;
                            if (_31931 > _31922)
                            {
                                float _31945 = in.i_shape.y * fast::clamp((_31922 - _31925) / fast::max(0.60000002384185791015625 * _31925, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _41717 = _31945;
                                _41716 = _31925 * (1.0 + (0.60000002384185791015625 * _31945));
                            }
                            else
                            {
                                _41717 = in.i_shape.y;
                                _41716 = _31931;
                            }
                            float2 _31974 = (abs(_30230) - _30227) + float2(_41716);
                            float2 _31976 = fast::max(_31974, float2(0.0));
                            float _41718;
                            if ((_31976.x > 0.0) && (_31976.y > 0.0))
                            {
                                float _41719;
                                if ((_41717 > 0.001000000047497451305389404296875) && (_41716 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _31993 = 2.0 + (2.0 * _41717);
                                    float2 _31998 = _31976 / float2(fast::max(_41716, 9.9999997473787516355514526367188e-05));
                                    _41719 = pow(pow(_31998.x, _31993) + pow(_31998.y, _31993), 1.0 / _31993) * _41716;
                                }
                                else
                                {
                                    _41719 = length(_31976);
                                }
                                _41718 = _41719;
                            }
                            else
                            {
                                _41718 = fast::max(_31976.x, _31976.y);
                            }
                            float _32033 = (fast::min(fast::max(_31974.x, _31974.y), 0.0) + _41718) - _41716;
                            float _32038 = fast::max(_41032.y, 9.9999997473787516355514526367188e-05);
                            float _32047 = fast::max(_32038 - abs(_31874 - _32033), 0.0) / _32038;
                            _41818 = fast::min(_31874, _32033) - (((_32047 * _32047) * _32038) * 0.25);
                        }
                        else
                        {
                            _41818 = _31874;
                        }
                        _41817 = _41818;
                    }
                    _41816 = _41817;
                }
                _41815 = _41816;
            }
            _41814 = _41815;
        }
        float _6552 = (_41814 + gFxData_1._data[_30038].y) / (fast::max(gFxData_1._data[_30038].x * 0.5, _5545 * 0.5) * 1.41421353816986083984375);
        float _32064 = sign(_6552);
        float _32066 = abs(_6552);
        float _32077 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_32066 * _32066))) * _32066)) * _32066);
        float _32080 = _32077 * _32077;
        float _32095 = fast::clamp(gFxData_1._data[_30030].w * ((0.5 + (0.5 * (_32064 - (_32064 / (_32080 * _32080))))) * _36985), 0.0, 1.0);
        _41986 = float4(gFxData_1._data[_30030].xyz * _32095, _32095) + (_41945 * (1.0 - _32095));
    }
    else
    {
        _41986 = _41945;
    }
    float4 _42041;
    if ((in.i_flags.x & _4844) != 0u)
    {
        uint _32119 = (in.i_instance * 24u) + 9u;
        uint _32127 = (in.i_instance * 24u) + 10u;
        float _6584 = fast::max(-_36352, 0.0) / fast::max(gFxData_1._data[_32127].z, 0.001000000047497451305389404296875);
        float _32137 = fast::clamp(gFxData_1._data[_32119].w * fast::clamp((exp(((-_6584) * _6584) * 2.2000000476837158203125) * gFxData_1._data[_32127].w) * _36985, 0.0, 1.0), 0.0, 1.0);
        _42041 = float4(gFxData_1._data[_32119].xyz * _32137, _32137) + (_41986 * (1.0 - _32137));
    }
    else
    {
        _42041 = _41986;
    }
    float3 _6615 = _42041.xyz + float3(_41991 * fast::clamp(_42041.w / fast::max(_36985, 0.001000000047497451305389404296875), 0.0, 1.0));
    float4 _36133 = _42041;
    _36133.x = _6615.x;
    _36133.y = _6615.y;
    _36133.z = _6615.z;
    float4 _42473;
    if ((in.i_flags.x & _4913) != 0u)
    {
        uint _32161 = (in.i_instance * 24u) + 5u;
        float4 _32163 = gFxData_1._data[_32161];
        uint _32169 = (in.i_instance * 24u) + 6u;
        float4 _42469;
        if (gFxData_1._data[_32169].z < 0.999000012874603271484375)
        {
            float2 _6677 = fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
            float _6688 = cos(gFxData_1._data[_32169].w);
            float _6691 = sin(gFxData_1._data[_32169].w);
            float4 _36150 = _32163;
            _36150.w = _32163.w * mix(1.0, gFxData_1._data[_32169].z, fast::clamp(((dot(in.i_local - ((in.i_rect.xy + in.i_rect.zw) * 0.5), float2(_6688, _6691)) / fast::max(0.5 * ((abs(_6688) * _6677.x) + (abs(_6691) * _6677.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _42469 = _36150;
        }
        else
        {
            _42469 = _32163;
        }
        float _32179 = fast::clamp(_42469.w * ((_42052 >= 0.0) ? _42052 : (fast::clamp(0.5 - ((_36352 - (gFxData_1._data[_32169].x * gFxData_1._data[_32169].y)) / _5545), 0.0, 1.0) - fast::clamp(0.5 - ((_36352 + (gFxData_1._data[_32169].x * (1.0 - gFxData_1._data[_32169].y))) / _5545), 0.0, 1.0))), 0.0, 1.0);
        _42473 = float4(_42469.xyz * _32179, _32179) + (_36133 * (1.0 - _32179));
    }
    else
    {
        _42473 = _36133;
    }
    float4 _42474;
    if ((in.i_flags.x & _5046) != 0u)
    {
        float2 _6752 = (in.i_local - in.i_rect.xy) / fast::max(in.i_rect.zw - in.i_rect.xy, float2(0.001000000047497451305389404296875));
        float _6774 = exp(-pow((((_6752.x * 0.85000002384185791015625) + (_6752.y * 0.1500000059604644775390625)) - ((fract(_288.gTime.x * in.i_misc.w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _42474 = float4(_42473.xyz + float3(((_6774 * in.i_misc.z) * _36985) * fast::max(_42473.w, 0.3499999940395355224609375)), fast::max(_42473.w, ((_6774 * in.i_misc.z) * _36985) * 0.5));
    }
    else
    {
        _42474 = _42473;
    }
    float4 _42905;
    if ((in.i_flags.x & _5128) != 0u)
    {
        float3 _32204 = fract(floor(_36240).xyx * 0.103100001811981201171875);
        float3 _32213 = _32204 + float3(dot(_32204, _32204.yzx + float3(33.3300018310546875)));
        float3 _6825 = _42474.xyz + float3(((fract((_32213.x + _32213.y) * _32213.z) - 0.5) * in.i_misc.y) * _42474.w);
        float4 _36174 = _42474;
        _36174.x = _6825.x;
        _36174.y = _6825.y;
        _36174.z = _6825.z;
        _42905 = _36174;
    }
    else
    {
        _42905 = _42474;
    }
    float _42901;
    if (_330.gFade.z > 0.0)
    {
        _42901 = smoothstep(0.0, 1.0, fast::clamp((_36240.y - _330.gFade.x) / _330.gFade.z, 0.0, 1.0));
    }
    else
    {
        _42901 = 1.0;
    }
    float _42902;
    if (_330.gFade.w > 0.0)
    {
        _42902 = _42901 * smoothstep(0.0, 1.0, fast::clamp((_330.gFade.y - _36240.y) / _330.gFade.w, 0.0, 1.0));
    }
    else
    {
        _42902 = _42901;
    }
    float4 _6842 = _42905 * ((in.i_misc.x * _42486) * _42902);
    float4 _42906;
    if (((in.i_flags.x & _5168) != 0u) || ((in.i_flags.x & _5172) != 0u))
    {
        float3 _32265 = fract((floor(_36240) + float2(17.0)).xyx * 0.103100001811981201171875);
        float3 _32274 = _32265 + float3(dot(_32265, _32265.yzx + float3(33.3300018310546875)));
        float3 _6866 = _6842.xyz + float3(((fract((_32274.x + _32274.y) * _32274.z) - 0.5) * 0.0039215688593685626983642578125) * fast::clamp(_6842.w * 8.0, 0.0, 1.0));
        float4 _36186 = _6842;
        _36186.x = _6866.x;
        _36186.y = _6866.y;
        _36186.z = _6866.z;
        _42906 = _36186;
    }
    else
    {
        _42906 = _6842;
    }
    float3 _6876 = fast::max(_42906.xyz, float3(0.0));
    float4 _36192 = _42906;
    _36192.x = _6876.x;
    _36192.y = _6876.y;
    _36192.z = _6876.z;
    float4 _42907;
    if ((_288.gTime.w > 0.5) && (_42906.w > 9.9999997473787516355514526367188e-06))
    {
        float3 _32318 = fast::clamp(_36192.xyz / float3(_42906.w), float3(0.0), float3(1.0));
        float3 _32304 = select(pow((_32318 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _32318 * float3(0.077399380505084991455078125), _32318 <= float3(0.040449999272823333740234375)) * _42906.w;
        float4 _36201 = _36192;
        _36201.x = _32304.x;
        _36201.y = _32304.y;
        _36201.z = _32304.z;
        _42907 = _36201;
    }
    else
    {
        _42907 = _36192;
    }
    out._entryPointOutput = _42907;
    return out;
}

