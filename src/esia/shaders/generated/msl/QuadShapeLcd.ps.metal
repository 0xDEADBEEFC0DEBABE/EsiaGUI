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

constant float2 _2821 = {};
constant float _2822 = {};

constant spvUnsafeArray<float4, 13> _561 = spvUnsafeArray<float4, 13>({ float4(0.0), float4(0.01659999974071979522705078125, -0.080700002610683441162109375, 0.22269999980926513671875, -0.075099997222423553466796875), float4(0.0350000001490116119384765625, -0.17599999904632568359375, 0.43250000476837158203125, -0.1369999945163726806640625), float4(0.0542999990284442901611328125, -0.2820999920368194580078125, 0.63020002841949462890625, -0.18760000169277191162109375), float4(0.07389999926090240478515625, -0.3962999880313873291015625, 0.816699981689453125, -0.22869999706745147705078125), float4(0.09329999983310699462890625, -0.516099989414215087890625, 0.992600023746490478515625, -0.2615999877452850341796875), float4(0.112099997699260711669921875, -0.63950002193450927734375, 1.15880000591278076171875, -0.287699997425079345703125), float4(0.12999999523162841796875, -0.764900028705596923828125, 1.31589996814727783203125, -0.3079999983310699462890625), float4(0.146899998188018798828125, -0.891099989414215087890625, 1.4644000530242919921875, -0.3233999907970428466796875), float4(0.162699997425079345703125, -1.0169999599456787109375, 1.60510003566741943359375, -0.33469998836517333984375), float4(0.1773000061511993408203125, -1.1419999599456787109375, 1.73849999904632568359375, -0.34259998798370361328125), float4(0.19079999625682830810546875, -1.26520001888275146484375, 1.8650000095367431640625, -0.3476000130176544189453125), float4(0.2030999958515167236328125, -1.3863999843597412109375, 1.9851000308990478515625, -0.3501000106334686279296875) });

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

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _130 [[buffer(0)]], constant WgtDraw& _162 [[buffer(2)]], texture2d<float> gTex [[texture(3)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float4 _2818;
    float4 _2819;
    do
    {
        float4 _1180 = gTex.sample(gLinear, in.i_uv);
        if (in.i_kind.x > (-0.5))
        {
            float _1248 = floor(in.i_kind.x + 0.5);
            float _1251 = fast::max(in.i_kind.y, 9.9999997473787516355514526367188e-05);
            float _2812;
            if (_1248 > 4.5)
            {
                float2 _1372 = in.i_uv - in.i_shapeRect.xy;
                float _2811;
                if (_1248 < 5.5)
                {
                    _2811 = fast::clamp((in.i_shape.y - abs(dot(_1372, fast::normalize(float2(in.i_shapeRect.y - in.i_shapeRect.w, in.i_shapeRect.z - in.i_shapeRect.x))))) / fast::max(in.i_shape.y - in.i_shape.x, 9.9999999747524270787835121154785e-07), 0.0, 1.0);
                }
                else
                {
                    float _1403 = floor(in.i_shape.z + 0.5);
                    float _1405 = 3.1415927410125732421875 / _1403;
                    float2 _1409 = fast::normalize(in.i_shapeRect.xy - in.i_shapeRect.zw);
                    float2 _1415 = float2(-_1409.y, _1409.x);
                    float _1433 = (fast::clamp(floor(precise::atan2(fast::max(dot(_1372, _1409), 0.0), dot(_1372, _1415)) / _1405), 0.0, _1403 - 1.0) + 0.5) * _1405;
                    float _1436 = cos(1.57079637050628662109375 / _1403);
                    _2811 = (dot(_1372, _1372) < 9.9999999600419720025001879548654e-13) ? 1.0 : fast::clamp(((in.i_shape.y * _1436) - dot(_1372, (_1415 * cos(_1433)) + (_1409 * sin(_1433)))) / fast::max((in.i_shape.y - in.i_shape.x) * _1436, 9.9999999747524270787835121154785e-07), 0.0, 1.0);
                }
                _2812 = _2811;
            }
            else
            {
                float _2813;
                if (_1248 < 3.5)
                {
                    float _2810;
                    if (_1248 >= 1.5)
                    {
                        float2 _1474 = in.i_uv - in.i_shapeRect.xy;
                        float2 _1477 = in.i_shapeRect.zw - in.i_shapeRect.xy;
                        _2810 = length(_1474 - (_1477 * fast::clamp(dot(_1474, _1477) / fast::max(dot(_1477, _1477), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_shape.x;
                    }
                    else
                    {
                        float2 _1287 = in.i_uv - ((in.i_shapeRect.xy + in.i_shapeRect.zw) * 0.5);
                        float2 _1296 = fast::max((in.i_shapeRect.zw - in.i_shapeRect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float _1542 = fast::min(_1296.x, _1296.y);
                        float _1545 = fast::min((_1287.x > 0.0) ? ((_1287.y > 0.0) ? in.i_shape.x : in.i_shape.x) : ((_1287.y > 0.0) ? in.i_shape.x : in.i_shape.x), _1542);
                        float _1551 = _1545 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _2806;
                        float _2807;
                        if (_1551 > _1542)
                        {
                            float _1565 = in.i_shape.y * fast::clamp((_1542 - _1545) / fast::max(0.60000002384185791015625 * _1545, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _2807 = _1565;
                            _2806 = _1545 * (1.0 + (0.60000002384185791015625 * _1565));
                        }
                        else
                        {
                            _2807 = in.i_shape.y;
                            _2806 = _1551;
                        }
                        float2 _1594 = (abs(_1287) - _1296) + float2(_2806);
                        float2 _1596 = fast::max(_1594, float2(0.0));
                        float _2808;
                        if ((_1596.x > 0.0) && (_1596.y > 0.0))
                        {
                            float _2809;
                            if ((_2807 > 0.001000000047497451305389404296875) && (_2806 > 9.9999997473787516355514526367188e-05))
                            {
                                float _1613 = 2.0 + (2.0 * _2807);
                                float2 _1618 = _1596 / float2(fast::max(_2806, 9.9999997473787516355514526367188e-05));
                                _2809 = pow(pow(_1618.x, _1613) + pow(_1618.y, _1613), 1.0 / _1613) * _2806;
                            }
                            else
                            {
                                _2809 = length(_1596);
                            }
                            _2808 = _2809;
                        }
                        else
                        {
                            _2808 = fast::max(_1596.x, _1596.y);
                        }
                        _2810 = (fast::min(fast::max(_1594.x, _1594.y), 0.0) + _2808) - _2806;
                    }
                    _2813 = ((_1248 - (2.0 * floor(_1248 * 0.5))) > 0.5) ? (fast::clamp(0.5 - ((_2810 - (in.i_shape.z * in.i_shape.w)) / _1251), 0.0, 1.0) - fast::clamp(0.5 - ((_2810 + (in.i_shape.z * (1.0 - in.i_shape.w))) / _1251), 0.0, 1.0)) : fast::clamp(0.5 - (_2810 / _1251), 0.0, 1.0);
                }
                else
                {
                    _2813 = 1.0;
                }
                _2812 = _2813;
            }
            float2 _1662 = gl_FragCoord.xy + _130.gConv.yy;
            float2 _2814;
            if (_130.gConv.x > 0.5)
            {
                float2 _2703;
                _2703.y = _130.gTarget.y - _1662.y;
                _2814 = _2703;
            }
            else
            {
                _2814 = _1662;
            }
            float _2815;
            if (_162.gFade.z > 0.0)
            {
                _2815 = smoothstep(0.0, 1.0, fast::clamp((_2814.y - _162.gFade.x) / _162.gFade.z, 0.0, 1.0));
            }
            else
            {
                _2815 = 1.0;
            }
            float _2816;
            if (_162.gFade.w > 0.0)
            {
                _2816 = _2815 * smoothstep(0.0, 1.0, fast::clamp((_162.gFade.y - _2814.y) / _162.gFade.w, 0.0, 1.0));
            }
            else
            {
                _2816 = _2815;
            }
            float _1228 = (in.i_col.w * _2812) * _2816;
            float4 _1720 = float4(in.i_col.xyz, 1.0);
            float4 _2817;
            if (_130.gTime.w > 0.5)
            {
                float3 _1761 = fast::clamp(_1720.xyz, float3(0.0), float3(1.0));
                float3 _1773 = select(pow((_1761 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _1761 * float3(0.077399380505084991455078125), _1761 <= float3(0.040449999272823333740234375));
                float4 _2705 = _1720;
                _2705.x = _1773.x;
                _2705.y = _1773.y;
                _2705.z = _1773.z;
                _2817 = _2705;
            }
            else
            {
                _2817 = _1720;
            }
            _2819 = _2817;
            _2818 = float4(_1228, _1228, _1228, fast::max(_1228, fast::max(_1228, _1228)));
            break;
        }
        if (in.i_kind.x < (-1.5))
        {
            float4 _1794 = in.i_col * float4(1.0, 1.0, 1.0, _1180.w);
            float2 _1813 = gl_FragCoord.xy + _130.gConv.yy;
            float2 _2798;
            if (_130.gConv.x > 0.5)
            {
                float2 _2717;
                _2717.y = _130.gTarget.y - _1813.y;
                _2798 = _2717;
            }
            else
            {
                _2798 = _1813;
            }
            float _2799;
            if (_162.gFade.z > 0.0)
            {
                _2799 = smoothstep(0.0, 1.0, fast::clamp((_2798.y - _162.gFade.x) / _162.gFade.z, 0.0, 1.0));
            }
            else
            {
                _2799 = 1.0;
            }
            float _2800;
            if (_162.gFade.w > 0.0)
            {
                _2800 = _2799 * smoothstep(0.0, 1.0, fast::clamp((_162.gFade.y - _2798.y) / _162.gFade.w, 0.0, 1.0));
            }
            else
            {
                _2800 = _2799;
            }
            float _1801 = _1794.w * _2800;
            float4 _2720 = _1794;
            _2720.w = _1801;
            float4 _2804;
            if (_130.gTime.w > 0.5)
            {
                _2804 = float4(_2822, _2822, _2822, _1801);
            }
            else
            {
                _2804 = _2720;
            }
            float4 _1907 = float4(in.i_col.xyz, 1.0);
            float4 _2805;
            if (_130.gTime.w > 0.5)
            {
                float3 _1948 = fast::clamp(_1907.xyz, float3(0.0), float3(1.0));
                float3 _1960 = select(pow((_1948 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _1948 * float3(0.077399380505084991455078125), _1948 <= float3(0.040449999272823333740234375));
                float4 _2728 = _1907;
                _2728.x = _1960.x;
                _2728.y = _1960.y;
                _2728.z = _1960.z;
                _2805 = _2728;
            }
            else
            {
                _2805 = _1907;
            }
            _2819 = _2805;
            _2818 = float4(_2804.www, fast::max(_2804.w, fast::max(_2804.w, _2804.w)));
            break;
        }
        float3 _2793;
        if ((_1180.x == _1180.y) && (_1180.y == _1180.z))
        {
            float _2002 = _1180.x;
            float _2095 = _130.gText.y * fast::clamp(4.0 * (0.75 - dot(in.i_col.xyz, float3(0.300000011920928955078125, 0.589999973773956298828125, 0.10999999940395355224609375))), 0.0, 1.0);
            float _2079 = dot(in.i_col.xyz, float3(0.25, 0.5, 0.25));
            float _2106 = (_2002 * (_2095 + 1.0)) / ((_2002 * _2095) + 1.0);
            float _2147 = fast::clamp((_130.gText.x - 1.0) * 10.0, 0.0, 12.0);
            int _2150 = min(int(_2147), 11);
            float4 _2164 = mix(_561[_2150], _561[_2150 + 1], float4(_2147 - float(_2150))) * 0.25;
            _2793 = float3(fast::clamp(_2106 + ((_2106 * (1.0 - _2106)) * ((((_2164.x * _2079) + _2164.y) * _2106) + ((_2164.z * _2079) + _2164.w))), 0.0, 1.0));
        }
        else
        {
            float3 _2015 = mix(_1180.www, _1180.xyz, float3(_130.gText.w));
            float _2173 = _130.gText.z * fast::clamp(4.0 * (0.75 - dot(in.i_col.xyz, float3(0.300000011920928955078125, 0.589999973773956298828125, 0.10999999940395355224609375))), 0.0, 1.0);
            float _2022 = _2015.x;
            float _2184 = (_2022 * (_2173 + 1.0)) / ((_2022 * _2173) + 1.0);
            float _2225 = fast::clamp((_130.gText.x - 1.0) * 10.0, 0.0, 12.0);
            int _2228 = min(int(_2225), 11);
            float4 _2242 = mix(_561[_2228], _561[_2228 + 1], float4(_2225 - float(_2228))) * 0.25;
            float _2032 = _2015.y;
            float _2253 = (_2032 * (_2173 + 1.0)) / ((_2032 * _2173) + 1.0);
            float _2294 = fast::clamp((_130.gText.x - 1.0) * 10.0, 0.0, 12.0);
            int _2297 = min(int(_2294), 11);
            float4 _2311 = mix(_561[_2297], _561[_2297 + 1], float4(_2294 - float(_2297))) * 0.25;
            float _2042 = _2015.z;
            float _2322 = (_2042 * (_2173 + 1.0)) / ((_2042 * _2173) + 1.0);
            float _2363 = fast::clamp((_130.gText.x - 1.0) * 10.0, 0.0, 12.0);
            int _2366 = min(int(_2363), 11);
            float4 _2380 = mix(_561[_2366], _561[_2366 + 1], float4(_2363 - float(_2366))) * 0.25;
            _2793 = float3(fast::clamp(_2184 + ((_2184 * (1.0 - _2184)) * ((((_2242.x * in.i_col.x) + _2242.y) * _2184) + ((_2242.z * in.i_col.x) + _2242.w))), 0.0, 1.0), fast::clamp(_2253 + ((_2253 * (1.0 - _2253)) * ((((_2311.x * in.i_col.y) + _2311.y) * _2253) + ((_2311.z * in.i_col.y) + _2311.w))), 0.0, 1.0), fast::clamp(_2322 + ((_2322 * (1.0 - _2322)) * ((((_2380.x * in.i_col.z) + _2380.y) * _2322) + ((_2380.z * in.i_col.z) + _2380.w))), 0.0, 1.0));
        }
        float2 _2389 = gl_FragCoord.xy + _130.gConv.yy;
        float2 _2794;
        if (_130.gConv.x > 0.5)
        {
            float2 _2772;
            _2772.y = _130.gTarget.y - _2389.y;
            _2794 = _2772;
        }
        else
        {
            _2794 = _2389;
        }
        float _2795;
        if (_162.gFade.z > 0.0)
        {
            _2795 = smoothstep(0.0, 1.0, fast::clamp((_2794.y - _162.gFade.x) / _162.gFade.z, 0.0, 1.0));
        }
        else
        {
            _2795 = 1.0;
        }
        float _2796;
        if (_162.gFade.w > 0.0)
        {
            _2796 = _2795 * smoothstep(0.0, 1.0, fast::clamp((_162.gFade.y - _2794.y) / _162.gFade.w, 0.0, 1.0));
        }
        else
        {
            _2796 = _2795;
        }
        float3 _2060 = _2793 * (in.i_col.w * _2796);
        float4 _2447 = float4(in.i_col.xyz, 1.0);
        float4 _2797;
        if (_130.gTime.w > 0.5)
        {
            float3 _2488 = fast::clamp(_2447.xyz, float3(0.0), float3(1.0));
            float3 _2500 = select(pow((_2488 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _2488 * float3(0.077399380505084991455078125), _2488 <= float3(0.040449999272823333740234375));
            float4 _2774 = _2447;
            _2774.x = _2500.x;
            _2774.y = _2500.y;
            _2774.z = _2500.z;
            _2797 = _2774;
        }
        else
        {
            _2797 = _2447;
        }
        _2819 = _2797;
        _2818 = float4(_2060, fast::max(_2060.x, fast::max(_2060.y, _2060.z)));
        break;
    } while(false);
    out._entryPointOutput_color = _2819;
    out._entryPointOutput_alpha = _2818;
    return out;
}

