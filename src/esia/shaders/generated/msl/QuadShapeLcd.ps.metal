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

constant float2 _2429 = {};

constant spvUnsafeArray<float4, 13> _490 = spvUnsafeArray<float4, 13>({ float4(0.0), float4(0.01659999974071979522705078125, -0.080700002610683441162109375, 0.22269999980926513671875, -0.075099997222423553466796875), float4(0.0350000001490116119384765625, -0.17599999904632568359375, 0.43250000476837158203125, -0.1369999945163726806640625), float4(0.0542999990284442901611328125, -0.2820999920368194580078125, 0.63020002841949462890625, -0.18760000169277191162109375), float4(0.07389999926090240478515625, -0.3962999880313873291015625, 0.816699981689453125, -0.22869999706745147705078125), float4(0.09329999983310699462890625, -0.516099989414215087890625, 0.992600023746490478515625, -0.2615999877452850341796875), float4(0.112099997699260711669921875, -0.63950002193450927734375, 1.15880000591278076171875, -0.287699997425079345703125), float4(0.12999999523162841796875, -0.764900028705596923828125, 1.31589996814727783203125, -0.3079999983310699462890625), float4(0.146899998188018798828125, -0.891099989414215087890625, 1.4644000530242919921875, -0.3233999907970428466796875), float4(0.162699997425079345703125, -1.0169999599456787109375, 1.60510003566741943359375, -0.33469998836517333984375), float4(0.1773000061511993408203125, -1.1419999599456787109375, 1.73849999904632568359375, -0.34259998798370361328125), float4(0.19079999625682830810546875, -1.26520001888275146484375, 1.8650000095367431640625, -0.3476000130176544189453125), float4(0.2030999958515167236328125, -1.3863999843597412109375, 1.9851000308990478515625, -0.3501000106334686279296875) });

struct esia_main_out
{
    float4 _entryPointOutput_color [[color(0)]];
    float4 _entryPointOutput_alpha [[color(0), index(1)]];
};

struct esia_main_in
{
    float4 i_col [[user(locn0)]];
    float2 i_uv [[user(locn1)]];
    float4 i_shapeRect [[user(locn2), flat]];
    float4 i_shape [[user(locn3), flat]];
    float2 i_kind [[user(locn4), flat]];
};

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _106 [[buffer(0)]], constant WgtDraw& _138 [[buffer(2)]], texture2d<float> gTex [[texture(3)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float4 _2427;
    float4 _2428;
    do
    {
        float4 _1074 = gTex.sample(gLinear, in.i_uv);
        if (in.i_kind.x > (-0.5))
        {
            float _1129 = floor(in.i_kind.x + 0.5);
            float _1132 = fast::max(in.i_kind.y, 9.9999997473787516355514526367188e-05);
            float _2421;
            if (_1129 > 4.5)
            {
                float2 _1253 = in.i_uv - in.i_shapeRect.xy;
                float _2420;
                if (_1129 < 5.5)
                {
                    _2420 = fast::clamp((in.i_shape.y - abs(dot(_1253, fast::normalize(float2(in.i_shapeRect.y - in.i_shapeRect.w, in.i_shapeRect.z - in.i_shapeRect.x))))) / fast::max(in.i_shape.y - in.i_shape.x, 9.9999999747524270787835121154785e-07), 0.0, 1.0);
                }
                else
                {
                    float _1284 = floor(in.i_shape.z + 0.5);
                    float _1286 = 3.1415927410125732421875 / _1284;
                    float2 _1290 = fast::normalize(in.i_shapeRect.xy - in.i_shapeRect.zw);
                    float2 _1296 = float2(-_1290.y, _1290.x);
                    float _1314 = (fast::clamp(floor(precise::atan2(fast::max(dot(_1253, _1290), 0.0), dot(_1253, _1296)) / _1286), 0.0, _1284 - 1.0) + 0.5) * _1286;
                    float _1317 = cos(1.57079637050628662109375 / _1284);
                    _2420 = (dot(_1253, _1253) < 9.9999999600419720025001879548654e-13) ? 1.0 : fast::clamp(((in.i_shape.y * _1317) - dot(_1253, (_1296 * cos(_1314)) + (_1290 * sin(_1314)))) / fast::max((in.i_shape.y - in.i_shape.x) * _1317, 9.9999999747524270787835121154785e-07), 0.0, 1.0);
                }
                _2421 = _2420;
            }
            else
            {
                float _2422;
                if (_1129 < 3.5)
                {
                    float _2419;
                    if (_1129 >= 1.5)
                    {
                        float2 _1355 = in.i_uv - in.i_shapeRect.xy;
                        float2 _1358 = in.i_shapeRect.zw - in.i_shapeRect.xy;
                        _2419 = length(_1355 - (_1358 * fast::clamp(dot(_1355, _1358) / fast::max(dot(_1358, _1358), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_shape.x;
                    }
                    else
                    {
                        float2 _1168 = in.i_uv - ((in.i_shapeRect.xy + in.i_shapeRect.zw) * 0.5);
                        float2 _1177 = fast::max((in.i_shapeRect.zw - in.i_shapeRect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float _1411 = fast::min(_1177.x, _1177.y);
                        float _1414 = fast::min((_1168.x > 0.0) ? ((_1168.y > 0.0) ? in.i_shape.x : in.i_shape.x) : ((_1168.y > 0.0) ? in.i_shape.x : in.i_shape.x), _1411);
                        float _1420 = _1414 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _2413;
                        float _2414;
                        if (_1420 > _1411)
                        {
                            float _1434 = in.i_shape.y * fast::clamp((_1411 - _1414) / fast::max(0.60000002384185791015625 * _1414, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _2414 = _1434;
                            _2413 = _1414 * (1.0 + (0.60000002384185791015625 * _1434));
                        }
                        else
                        {
                            _2414 = in.i_shape.y;
                            _2413 = _1420;
                        }
                        float2 _1447 = (abs(_1168) - _1177) + float2(_2413);
                        float2 _1449 = fast::max(_1447, float2(0.0));
                        float _2415;
                        if ((_1449.x > 0.0) && (_1449.y > 0.0))
                        {
                            float _2416;
                            if ((_2414 > 0.001000000047497451305389404296875) && (_2413 > 9.9999997473787516355514526367188e-05))
                            {
                                float _1466 = 2.0 + (2.0 * _2414);
                                float2 _1471 = _1449 / float2(fast::max(_2413, 9.9999997473787516355514526367188e-05));
                                _2416 = pow(pow(_1471.x, _1466) + pow(_1471.y, _1466), 1.0 / _1466) * _2413;
                            }
                            else
                            {
                                _2416 = length(_1449);
                            }
                            _2415 = _2416;
                        }
                        else
                        {
                            _2415 = fast::max(_1449.x, _1449.y);
                        }
                        _2419 = (fast::min(fast::max(_1447.x, _1447.y), 0.0) + _2415) - _2413;
                    }
                    _2422 = ((_1129 - (2.0 * floor(_1129 * 0.5))) > 0.5) ? (fast::clamp(0.5 - ((_2419 - (in.i_shape.z * in.i_shape.w)) / _1132), 0.0, 1.0) - fast::clamp(0.5 - ((_2419 + (in.i_shape.z * (1.0 - in.i_shape.w))) / _1132), 0.0, 1.0)) : fast::clamp(0.5 - (_2419 / _1132), 0.0, 1.0);
                }
                else
                {
                    _2422 = 1.0;
                }
                _2421 = _2422;
            }
            float2 _1515 = gl_FragCoord.xy + _106.gConv.yy;
            float2 _2423;
            if (_106.gConv.x > 0.5)
            {
                float2 _2341;
                _2341.y = _106.gTarget.y - _1515.y;
                _2423 = _2341;
            }
            else
            {
                _2423 = _1515;
            }
            float _2424;
            if (_138.gFade.z > 0.0)
            {
                _2424 = smoothstep(0.0, 1.0, fast::clamp((_2423.y - _138.gFade.x) / _138.gFade.z, 0.0, 1.0));
            }
            else
            {
                _2424 = 1.0;
            }
            float _2425;
            if (_138.gFade.w > 0.0)
            {
                _2425 = _2424 * smoothstep(0.0, 1.0, fast::clamp((_138.gFade.y - _2423.y) / _138.gFade.w, 0.0, 1.0));
            }
            else
            {
                _2425 = _2424;
            }
            float _1109 = (in.i_col.w * _2421) * _2425;
            float4 _1573 = float4(in.i_col.xyz, 1.0);
            float4 _2426;
            if (_106.gTime.w > 0.5)
            {
                float3 _1614 = fast::clamp(_1573.xyz, float3(0.0), float3(1.0));
                float3 _1626 = select(pow((_1614 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _1614 * float3(0.077399380505084991455078125), _1614 <= float3(0.040449999272823333740234375));
                float4 _2343 = _1573;
                _2343.x = _1626.x;
                _2343.y = _1626.y;
                _2343.z = _1626.z;
                _2426 = _2343;
            }
            else
            {
                _2426 = _1573;
            }
            _2428 = _2426;
            _2427 = float4(_1109, _1109, _1109, fast::max(_1109, fast::max(_1109, _1109)));
            break;
        }
        float3 _2408;
        if ((_1074.x == _1074.y) && (_1074.y == _1074.z))
        {
            float _1668 = _1074.x;
            float _1761 = _106.gText.y * fast::clamp(4.0 * (0.75 - dot(in.i_col.xyz, float3(0.300000011920928955078125, 0.589999973773956298828125, 0.10999999940395355224609375))), 0.0, 1.0);
            float _1745 = dot(in.i_col.xyz, float3(0.25, 0.5, 0.25));
            float _1772 = (_1668 * (_1761 + 1.0)) / ((_1668 * _1761) + 1.0);
            float _1813 = fast::clamp((_106.gText.x - 1.0) * 10.0, 0.0, 12.0);
            int _1816 = min(int(_1813), 11);
            float4 _1830 = mix(_490[_1816], _490[_1816 + 1], float4(_1813 - float(_1816))) * 0.25;
            _2408 = float3(fast::clamp(_1772 + ((_1772 * (1.0 - _1772)) * ((((_1830.x * _1745) + _1830.y) * _1772) + ((_1830.z * _1745) + _1830.w))), 0.0, 1.0));
        }
        else
        {
            float3 _1681 = mix(_1074.www, _1074.xyz, float3(_106.gText.w));
            float _1839 = _106.gText.z * fast::clamp(4.0 * (0.75 - dot(in.i_col.xyz, float3(0.300000011920928955078125, 0.589999973773956298828125, 0.10999999940395355224609375))), 0.0, 1.0);
            float _1688 = _1681.x;
            float _1850 = (_1688 * (_1839 + 1.0)) / ((_1688 * _1839) + 1.0);
            float _1891 = fast::clamp((_106.gText.x - 1.0) * 10.0, 0.0, 12.0);
            int _1894 = min(int(_1891), 11);
            float4 _1908 = mix(_490[_1894], _490[_1894 + 1], float4(_1891 - float(_1894))) * 0.25;
            float _1698 = _1681.y;
            float _1919 = (_1698 * (_1839 + 1.0)) / ((_1698 * _1839) + 1.0);
            float _1960 = fast::clamp((_106.gText.x - 1.0) * 10.0, 0.0, 12.0);
            int _1963 = min(int(_1960), 11);
            float4 _1977 = mix(_490[_1963], _490[_1963 + 1], float4(_1960 - float(_1963))) * 0.25;
            float _1708 = _1681.z;
            float _1988 = (_1708 * (_1839 + 1.0)) / ((_1708 * _1839) + 1.0);
            float _2029 = fast::clamp((_106.gText.x - 1.0) * 10.0, 0.0, 12.0);
            int _2032 = min(int(_2029), 11);
            float4 _2046 = mix(_490[_2032], _490[_2032 + 1], float4(_2029 - float(_2032))) * 0.25;
            _2408 = float3(fast::clamp(_1850 + ((_1850 * (1.0 - _1850)) * ((((_1908.x * in.i_col.x) + _1908.y) * _1850) + ((_1908.z * in.i_col.x) + _1908.w))), 0.0, 1.0), fast::clamp(_1919 + ((_1919 * (1.0 - _1919)) * ((((_1977.x * in.i_col.y) + _1977.y) * _1919) + ((_1977.z * in.i_col.y) + _1977.w))), 0.0, 1.0), fast::clamp(_1988 + ((_1988 * (1.0 - _1988)) * ((((_2046.x * in.i_col.z) + _2046.y) * _1988) + ((_2046.z * in.i_col.z) + _2046.w))), 0.0, 1.0));
        }
        float2 _2055 = gl_FragCoord.xy + _106.gConv.yy;
        float2 _2409;
        if (_106.gConv.x > 0.5)
        {
            float2 _2387;
            _2387.y = _106.gTarget.y - _2055.y;
            _2409 = _2387;
        }
        else
        {
            _2409 = _2055;
        }
        float _2410;
        if (_138.gFade.z > 0.0)
        {
            _2410 = smoothstep(0.0, 1.0, fast::clamp((_2409.y - _138.gFade.x) / _138.gFade.z, 0.0, 1.0));
        }
        else
        {
            _2410 = 1.0;
        }
        float _2411;
        if (_138.gFade.w > 0.0)
        {
            _2411 = _2410 * smoothstep(0.0, 1.0, fast::clamp((_138.gFade.y - _2409.y) / _138.gFade.w, 0.0, 1.0));
        }
        else
        {
            _2411 = _2410;
        }
        float3 _1726 = _2408 * (in.i_col.w * _2411);
        float4 _2113 = float4(in.i_col.xyz, 1.0);
        float4 _2412;
        if (_106.gTime.w > 0.5)
        {
            float3 _2154 = fast::clamp(_2113.xyz, float3(0.0), float3(1.0));
            float3 _2166 = select(pow((_2154 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _2154 * float3(0.077399380505084991455078125), _2154 <= float3(0.040449999272823333740234375));
            float4 _2389 = _2113;
            _2389.x = _2166.x;
            _2389.y = _2166.y;
            _2389.z = _2166.z;
            _2412 = _2389;
        }
        else
        {
            _2412 = _2113;
        }
        _2428 = _2412;
        _2427 = float4(_1726, fast::max(_1726.x, fast::max(_1726.y, _1726.z)));
        break;
    } while(false);
    out._entryPointOutput_color = _2428;
    out._entryPointOutput_alpha = _2427;
    return out;
}

