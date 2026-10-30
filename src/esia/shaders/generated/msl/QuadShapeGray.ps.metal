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

constant float2 _1887 = {};

constant spvUnsafeArray<float4, 13> _484 = spvUnsafeArray<float4, 13>({ float4(0.0), float4(0.01659999974071979522705078125, -0.080700002610683441162109375, 0.22269999980926513671875, -0.075099997222423553466796875), float4(0.0350000001490116119384765625, -0.17599999904632568359375, 0.43250000476837158203125, -0.1369999945163726806640625), float4(0.0542999990284442901611328125, -0.2820999920368194580078125, 0.63020002841949462890625, -0.18760000169277191162109375), float4(0.07389999926090240478515625, -0.3962999880313873291015625, 0.816699981689453125, -0.22869999706745147705078125), float4(0.09329999983310699462890625, -0.516099989414215087890625, 0.992600023746490478515625, -0.2615999877452850341796875), float4(0.112099997699260711669921875, -0.63950002193450927734375, 1.15880000591278076171875, -0.287699997425079345703125), float4(0.12999999523162841796875, -0.764900028705596923828125, 1.31589996814727783203125, -0.3079999983310699462890625), float4(0.146899998188018798828125, -0.891099989414215087890625, 1.4644000530242919921875, -0.3233999907970428466796875), float4(0.162699997425079345703125, -1.0169999599456787109375, 1.60510003566741943359375, -0.33469998836517333984375), float4(0.1773000061511993408203125, -1.1419999599456787109375, 1.73849999904632568359375, -0.34259998798370361328125), float4(0.19079999625682830810546875, -1.26520001888275146484375, 1.8650000095367431640625, -0.3476000130176544189453125), float4(0.2030999958515167236328125, -1.3863999843597412109375, 1.9851000308990478515625, -0.3501000106334686279296875) });

struct esia_main_out
{
    float4 _entryPointOutput [[color(0)]];
};

struct esia_main_in
{
    float4 i_col [[user(locn0)]];
    float2 i_uv [[user(locn1)]];
    float4 i_shapeRect [[user(locn2), flat]];
    float4 i_shape [[user(locn3), flat]];
    float2 i_kind [[user(locn4), flat]];
};

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _100 [[buffer(0)]], constant WgtDraw& _132 [[buffer(2)]], texture2d<float> gTex [[texture(3)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float4 _1886;
    do
    {
        float4 _961 = gTex.sample(gLinear, in.i_uv);
        if (in.i_kind.x > (-0.5))
        {
            float _1021 = floor(in.i_kind.x + 0.5);
            float _1024 = fast::max(in.i_kind.y, 9.9999997473787516355514526367188e-05);
            float _1880;
            if (_1021 > 4.5)
            {
                float2 _1145 = in.i_uv - in.i_shapeRect.xy;
                float _1879;
                if (_1021 < 5.5)
                {
                    _1879 = fast::clamp((in.i_shape.y - abs(dot(_1145, fast::normalize(float2(in.i_shapeRect.y - in.i_shapeRect.w, in.i_shapeRect.z - in.i_shapeRect.x))))) / fast::max(in.i_shape.y - in.i_shape.x, 9.9999999747524270787835121154785e-07), 0.0, 1.0);
                }
                else
                {
                    float _1176 = floor(in.i_shape.z + 0.5);
                    float _1178 = 3.1415927410125732421875 / _1176;
                    float2 _1182 = fast::normalize(in.i_shapeRect.xy - in.i_shapeRect.zw);
                    float2 _1188 = float2(-_1182.y, _1182.x);
                    float _1206 = (fast::clamp(floor(precise::atan2(fast::max(dot(_1145, _1182), 0.0), dot(_1145, _1188)) / _1178), 0.0, _1176 - 1.0) + 0.5) * _1178;
                    float _1209 = cos(1.57079637050628662109375 / _1176);
                    _1879 = (dot(_1145, _1145) < 9.9999999600419720025001879548654e-13) ? 1.0 : fast::clamp(((in.i_shape.y * _1209) - dot(_1145, (_1188 * cos(_1206)) + (_1182 * sin(_1206)))) / fast::max((in.i_shape.y - in.i_shape.x) * _1209, 9.9999999747524270787835121154785e-07), 0.0, 1.0);
                }
                _1880 = _1879;
            }
            else
            {
                float _1881;
                if (_1021 < 3.5)
                {
                    float _1878;
                    if (_1021 >= 1.5)
                    {
                        float2 _1247 = in.i_uv - in.i_shapeRect.xy;
                        float2 _1250 = in.i_shapeRect.zw - in.i_shapeRect.xy;
                        _1878 = length(_1247 - (_1250 * fast::clamp(dot(_1247, _1250) / fast::max(dot(_1250, _1250), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_shape.x;
                    }
                    else
                    {
                        float2 _1060 = in.i_uv - ((in.i_shapeRect.xy + in.i_shapeRect.zw) * 0.5);
                        float2 _1069 = fast::max((in.i_shapeRect.zw - in.i_shapeRect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float _1303 = fast::min(_1069.x, _1069.y);
                        float _1306 = fast::min((_1060.x > 0.0) ? ((_1060.y > 0.0) ? in.i_shape.x : in.i_shape.x) : ((_1060.y > 0.0) ? in.i_shape.x : in.i_shape.x), _1303);
                        float _1312 = _1306 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _1872;
                        float _1873;
                        if (_1312 > _1303)
                        {
                            float _1326 = in.i_shape.y * fast::clamp((_1303 - _1306) / fast::max(0.60000002384185791015625 * _1306, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _1873 = _1326;
                            _1872 = _1306 * (1.0 + (0.60000002384185791015625 * _1326));
                        }
                        else
                        {
                            _1873 = in.i_shape.y;
                            _1872 = _1312;
                        }
                        float2 _1339 = (abs(_1060) - _1069) + float2(_1872);
                        float2 _1341 = fast::max(_1339, float2(0.0));
                        float _1874;
                        if ((_1341.x > 0.0) && (_1341.y > 0.0))
                        {
                            float _1875;
                            if ((_1873 > 0.001000000047497451305389404296875) && (_1872 > 9.9999997473787516355514526367188e-05))
                            {
                                float _1358 = 2.0 + (2.0 * _1873);
                                float2 _1363 = _1341 / float2(fast::max(_1872, 9.9999997473787516355514526367188e-05));
                                _1875 = pow(pow(_1363.x, _1358) + pow(_1363.y, _1358), 1.0 / _1358) * _1872;
                            }
                            else
                            {
                                _1875 = length(_1341);
                            }
                            _1874 = _1875;
                        }
                        else
                        {
                            _1874 = fast::max(_1341.x, _1341.y);
                        }
                        _1878 = (fast::min(fast::max(_1339.x, _1339.y), 0.0) + _1874) - _1872;
                    }
                    _1881 = ((_1021 - (2.0 * floor(_1021 * 0.5))) > 0.5) ? (fast::clamp(0.5 - ((_1878 - (in.i_shape.z * in.i_shape.w)) / _1024), 0.0, 1.0) - fast::clamp(0.5 - ((_1878 + (in.i_shape.z * (1.0 - in.i_shape.w))) / _1024), 0.0, 1.0)) : fast::clamp(0.5 - (_1878 / _1024), 0.0, 1.0);
                }
                else
                {
                    _1881 = 1.0;
                }
                _1880 = _1881;
            }
            float2 _1407 = gl_FragCoord.xy + _100.gConv.yy;
            float2 _1882;
            if (_100.gConv.x > 0.5)
            {
                float2 _1836;
                _1836.y = _100.gTarget.y - _1407.y;
                _1882 = _1836;
            }
            else
            {
                _1882 = _1407;
            }
            float _1883;
            if (_132.gFade.z > 0.0)
            {
                _1883 = smoothstep(0.0, 1.0, fast::clamp((_1882.y - _132.gFade.x) / _132.gFade.z, 0.0, 1.0));
            }
            else
            {
                _1883 = 1.0;
            }
            float _1884;
            if (_132.gFade.w > 0.0)
            {
                _1884 = _1883 * smoothstep(0.0, 1.0, fast::clamp((_132.gFade.y - _1882.y) / _132.gFade.w, 0.0, 1.0));
            }
            else
            {
                _1884 = _1883;
            }
            float4 _974 = float4(in.i_col.xyz, (in.i_col.w * _1880) * _1884);
            float4 _1885;
            if (_100.gTime.w > 0.5)
            {
                float3 _1479 = fast::clamp(_974.xyz, float3(0.0), float3(1.0));
                float3 _1491 = select(pow((_1479 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _1479 * float3(0.077399380505084991455078125), _1479 <= float3(0.040449999272823333740234375));
                float4 _1838 = _974;
                _1838.x = _1491.x;
                _1838.y = _1491.y;
                _1838.z = _1491.z;
                _1885 = _1838;
            }
            else
            {
                _1885 = _974;
            }
            _1886 = _1885;
            break;
        }
        float _980 = _961.x;
        float _1550 = _100.gText.y * fast::clamp(4.0 * (0.75 - dot(in.i_col.xyz, float3(0.300000011920928955078125, 0.589999973773956298828125, 0.10999999940395355224609375))), 0.0, 1.0);
        float _1534 = dot(in.i_col.xyz, float3(0.25, 0.5, 0.25));
        float _1561 = (_980 * (_1550 + 1.0)) / ((_980 * _1550) + 1.0);
        float _1602 = fast::clamp((_100.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _1605 = min(int(_1602), 11);
        float4 _1619 = mix(_484[_1605], _484[_1605 + 1], float4(_1602 - float(_1605))) * 0.25;
        float2 _1628 = gl_FragCoord.xy + _100.gConv.yy;
        float2 _1868;
        if (_100.gConv.x > 0.5)
        {
            float2 _1851;
            _1851.y = _100.gTarget.y - _1628.y;
            _1868 = _1851;
        }
        else
        {
            _1868 = _1628;
        }
        float _1869;
        if (_132.gFade.z > 0.0)
        {
            _1869 = smoothstep(0.0, 1.0, fast::clamp((_1868.y - _132.gFade.x) / _132.gFade.z, 0.0, 1.0));
        }
        else
        {
            _1869 = 1.0;
        }
        float _1870;
        if (_132.gFade.w > 0.0)
        {
            _1870 = _1869 * smoothstep(0.0, 1.0, fast::clamp((_132.gFade.y - _1868.y) / _132.gFade.w, 0.0, 1.0));
        }
        else
        {
            _1870 = _1869;
        }
        float4 _1516 = float4(in.i_col.xyz, (in.i_col.w * fast::clamp(_1561 + ((_1561 * (1.0 - _1561)) * ((((_1619.x * _1534) + _1619.y) * _1561) + ((_1619.z * _1534) + _1619.w))), 0.0, 1.0)) * _1870);
        float4 _1871;
        if (_100.gTime.w > 0.5)
        {
            float3 _1700 = fast::clamp(_1516.xyz, float3(0.0), float3(1.0));
            float3 _1712 = select(pow((_1700 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _1700 * float3(0.077399380505084991455078125), _1700 <= float3(0.040449999272823333740234375));
            float4 _1853 = _1516;
            _1853.x = _1712.x;
            _1853.y = _1712.y;
            _1853.z = _1712.z;
            _1871 = _1853;
        }
        else
        {
            _1871 = _1516;
        }
        _1886 = _1871;
        break;
    } while(false);
    out._entryPointOutput = _1886;
    return out;
}

