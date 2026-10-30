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

constant float2 _2180 = {};

constant spvUnsafeArray<float4, 13> _555 = spvUnsafeArray<float4, 13>({ float4(0.0), float4(0.01659999974071979522705078125, -0.080700002610683441162109375, 0.22269999980926513671875, -0.075099997222423553466796875), float4(0.0350000001490116119384765625, -0.17599999904632568359375, 0.43250000476837158203125, -0.1369999945163726806640625), float4(0.0542999990284442901611328125, -0.2820999920368194580078125, 0.63020002841949462890625, -0.18760000169277191162109375), float4(0.07389999926090240478515625, -0.3962999880313873291015625, 0.816699981689453125, -0.22869999706745147705078125), float4(0.09329999983310699462890625, -0.516099989414215087890625, 0.992600023746490478515625, -0.2615999877452850341796875), float4(0.112099997699260711669921875, -0.63950002193450927734375, 1.15880000591278076171875, -0.287699997425079345703125), float4(0.12999999523162841796875, -0.764900028705596923828125, 1.31589996814727783203125, -0.3079999983310699462890625), float4(0.146899998188018798828125, -0.891099989414215087890625, 1.4644000530242919921875, -0.3233999907970428466796875), float4(0.162699997425079345703125, -1.0169999599456787109375, 1.60510003566741943359375, -0.33469998836517333984375), float4(0.1773000061511993408203125, -1.1419999599456787109375, 1.73849999904632568359375, -0.34259998798370361328125), float4(0.19079999625682830810546875, -1.26520001888275146484375, 1.8650000095367431640625, -0.3476000130176544189453125), float4(0.2030999958515167236328125, -1.3863999843597412109375, 1.9851000308990478515625, -0.3501000106334686279296875) });

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

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _124 [[buffer(0)]], constant WgtDraw& _156 [[buffer(2)]], texture2d<float> gTex [[texture(3)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float4 _2178;
    do
    {
        float4 _1059 = gTex.sample(gLinear, in.i_uv);
        if (in.i_kind.x > (-0.5))
        {
            float _1128 = floor(in.i_kind.x + 0.5);
            float _1131 = fast::max(in.i_kind.y, 9.9999997473787516355514526367188e-05);
            float _2172;
            if (_1128 > 4.5)
            {
                float2 _1252 = in.i_uv - in.i_shapeRect.xy;
                float _2171;
                if (_1128 < 5.5)
                {
                    _2171 = fast::clamp((in.i_shape.y - abs(dot(_1252, fast::normalize(float2(in.i_shapeRect.y - in.i_shapeRect.w, in.i_shapeRect.z - in.i_shapeRect.x))))) / fast::max(in.i_shape.y - in.i_shape.x, 9.9999999747524270787835121154785e-07), 0.0, 1.0);
                }
                else
                {
                    float _1283 = floor(in.i_shape.z + 0.5);
                    float _1285 = 3.1415927410125732421875 / _1283;
                    float2 _1289 = fast::normalize(in.i_shapeRect.xy - in.i_shapeRect.zw);
                    float2 _1295 = float2(-_1289.y, _1289.x);
                    float _1313 = (fast::clamp(floor(precise::atan2(fast::max(dot(_1252, _1289), 0.0), dot(_1252, _1295)) / _1285), 0.0, _1283 - 1.0) + 0.5) * _1285;
                    float _1316 = cos(1.57079637050628662109375 / _1283);
                    _2171 = (dot(_1252, _1252) < 9.9999999600419720025001879548654e-13) ? 1.0 : fast::clamp(((in.i_shape.y * _1316) - dot(_1252, (_1295 * cos(_1313)) + (_1289 * sin(_1313)))) / fast::max((in.i_shape.y - in.i_shape.x) * _1316, 9.9999999747524270787835121154785e-07), 0.0, 1.0);
                }
                _2172 = _2171;
            }
            else
            {
                float _2173;
                if (_1128 < 3.5)
                {
                    float _2170;
                    if (_1128 >= 1.5)
                    {
                        float2 _1354 = in.i_uv - in.i_shapeRect.xy;
                        float2 _1357 = in.i_shapeRect.zw - in.i_shapeRect.xy;
                        _2170 = length(_1354 - (_1357 * fast::clamp(dot(_1354, _1357) / fast::max(dot(_1357, _1357), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_shape.x;
                    }
                    else
                    {
                        float2 _1167 = in.i_uv - ((in.i_shapeRect.xy + in.i_shapeRect.zw) * 0.5);
                        float2 _1176 = fast::max((in.i_shapeRect.zw - in.i_shapeRect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float _1422 = fast::min(_1176.x, _1176.y);
                        float _1425 = fast::min((_1167.x > 0.0) ? ((_1167.y > 0.0) ? in.i_shape.x : in.i_shape.x) : ((_1167.y > 0.0) ? in.i_shape.x : in.i_shape.x), _1422);
                        float _1431 = _1425 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _2166;
                        float _2167;
                        if (_1431 > _1422)
                        {
                            float _1445 = in.i_shape.y * fast::clamp((_1422 - _1425) / fast::max(0.60000002384185791015625 * _1425, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _2167 = _1445;
                            _2166 = _1425 * (1.0 + (0.60000002384185791015625 * _1445));
                        }
                        else
                        {
                            _2167 = in.i_shape.y;
                            _2166 = _1431;
                        }
                        float2 _1474 = (abs(_1167) - _1176) + float2(_2166);
                        float2 _1476 = fast::max(_1474, float2(0.0));
                        float _2168;
                        if ((_1476.x > 0.0) && (_1476.y > 0.0))
                        {
                            float _2169;
                            if ((_2167 > 0.001000000047497451305389404296875) && (_2166 > 9.9999997473787516355514526367188e-05))
                            {
                                float _1493 = 2.0 + (2.0 * _2167);
                                float2 _1498 = _1476 / float2(fast::max(_2166, 9.9999997473787516355514526367188e-05));
                                _2169 = pow(pow(_1498.x, _1493) + pow(_1498.y, _1493), 1.0 / _1493) * _2166;
                            }
                            else
                            {
                                _2169 = length(_1476);
                            }
                            _2168 = _2169;
                        }
                        else
                        {
                            _2168 = fast::max(_1476.x, _1476.y);
                        }
                        _2170 = (fast::min(fast::max(_1474.x, _1474.y), 0.0) + _2168) - _2166;
                    }
                    _2173 = ((_1128 - (2.0 * floor(_1128 * 0.5))) > 0.5) ? (fast::clamp(0.5 - ((_2170 - (in.i_shape.z * in.i_shape.w)) / _1131), 0.0, 1.0) - fast::clamp(0.5 - ((_2170 + (in.i_shape.z * (1.0 - in.i_shape.w))) / _1131), 0.0, 1.0)) : fast::clamp(0.5 - (_2170 / _1131), 0.0, 1.0);
                }
                else
                {
                    _2173 = 1.0;
                }
                _2172 = _2173;
            }
            float2 _1542 = gl_FragCoord.xy + _124.gConv.yy;
            float2 _2174;
            if (_124.gConv.x > 0.5)
            {
                float2 _2109;
                _2109.y = _124.gTarget.y - _1542.y;
                _2174 = _2109;
            }
            else
            {
                _2174 = _1542;
            }
            float _2175;
            if (_156.gFade.z > 0.0)
            {
                _2175 = smoothstep(0.0, 1.0, fast::clamp((_2174.y - _156.gFade.x) / _156.gFade.z, 0.0, 1.0));
            }
            else
            {
                _2175 = 1.0;
            }
            float _2176;
            if (_156.gFade.w > 0.0)
            {
                _2176 = _2175 * smoothstep(0.0, 1.0, fast::clamp((_156.gFade.y - _2174.y) / _156.gFade.w, 0.0, 1.0));
            }
            else
            {
                _2176 = _2175;
            }
            float4 _1072 = float4(in.i_col.xyz, (in.i_col.w * _2172) * _2176);
            float4 _2177;
            if (_124.gTime.w > 0.5)
            {
                float3 _1614 = fast::clamp(_1072.xyz, float3(0.0), float3(1.0));
                float3 _1626 = select(pow((_1614 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _1614 * float3(0.077399380505084991455078125), _1614 <= float3(0.040449999272823333740234375));
                float4 _2111 = _1072;
                _2111.x = _1626.x;
                _2111.y = _1626.y;
                _2111.z = _1626.z;
                _2177 = _2111;
            }
            else
            {
                _2177 = _1072;
            }
            _2178 = _2177;
            break;
        }
        if (in.i_kind.x < (-1.5))
        {
            float4 _1647 = in.i_col * float4(1.0, 1.0, 1.0, _1059.w);
            float2 _1666 = gl_FragCoord.xy + _124.gConv.yy;
            float2 _2159;
            if (_124.gConv.x > 0.5)
            {
                float2 _2120;
                _2120.y = _124.gTarget.y - _1666.y;
                _2159 = _2120;
            }
            else
            {
                _2159 = _1666;
            }
            float _2160;
            if (_156.gFade.z > 0.0)
            {
                _2160 = smoothstep(0.0, 1.0, fast::clamp((_2159.y - _156.gFade.x) / _156.gFade.z, 0.0, 1.0));
            }
            else
            {
                _2160 = 1.0;
            }
            float _2161;
            if (_156.gFade.w > 0.0)
            {
                _2161 = _2160 * smoothstep(0.0, 1.0, fast::clamp((_156.gFade.y - _2159.y) / _156.gFade.w, 0.0, 1.0));
            }
            else
            {
                _2161 = _2160;
            }
            float _1654 = _1647.w * _2161;
            float4 _2123 = _1647;
            _2123.w = _1654;
            float4 _2165;
            if (_124.gTime.w > 0.5)
            {
                float3 _1738 = fast::clamp(_2123.xyz, float3(0.0), float3(1.0));
                _2165 = float4(select(pow((_1738 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _1738 * float3(0.077399380505084991455078125), _1738 <= float3(0.040449999272823333740234375)), _1654);
            }
            else
            {
                _2165 = _2123;
            }
            _2178 = _2165;
            break;
        }
        float _1087 = _1059.w;
        float _1809 = _124.gText.y * fast::clamp(4.0 * (0.75 - dot(in.i_col.xyz, float3(0.300000011920928955078125, 0.589999973773956298828125, 0.10999999940395355224609375))), 0.0, 1.0);
        float _1793 = dot(in.i_col.xyz, float3(0.25, 0.5, 0.25));
        float _1820 = (_1087 * (_1809 + 1.0)) / ((_1087 * _1809) + 1.0);
        float _1861 = fast::clamp((_124.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _1864 = min(int(_1861), 11);
        float4 _1878 = mix(_555[_1864], _555[_1864 + 1], float4(_1861 - float(_1864))) * 0.25;
        float2 _1887 = gl_FragCoord.xy + _124.gConv.yy;
        float2 _2155;
        if (_124.gConv.x > 0.5)
        {
            float2 _2138;
            _2138.y = _124.gTarget.y - _1887.y;
            _2155 = _2138;
        }
        else
        {
            _2155 = _1887;
        }
        float _2156;
        if (_156.gFade.z > 0.0)
        {
            _2156 = smoothstep(0.0, 1.0, fast::clamp((_2155.y - _156.gFade.x) / _156.gFade.z, 0.0, 1.0));
        }
        else
        {
            _2156 = 1.0;
        }
        float _2157;
        if (_156.gFade.w > 0.0)
        {
            _2157 = _2156 * smoothstep(0.0, 1.0, fast::clamp((_156.gFade.y - _2155.y) / _156.gFade.w, 0.0, 1.0));
        }
        else
        {
            _2157 = _2156;
        }
        float4 _1775 = float4(in.i_col.xyz, (in.i_col.w * fast::clamp(_1820 + ((_1820 * (1.0 - _1820)) * ((((_1878.x * _1793) + _1878.y) * _1820) + ((_1878.z * _1793) + _1878.w))), 0.0, 1.0)) * _2157);
        float4 _2158;
        if (_124.gTime.w > 0.5)
        {
            float3 _1959 = fast::clamp(_1775.xyz, float3(0.0), float3(1.0));
            float3 _1971 = select(pow((_1959 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _1959 * float3(0.077399380505084991455078125), _1959 <= float3(0.040449999272823333740234375));
            float4 _2140 = _1775;
            _2140.x = _1971.x;
            _2140.y = _1971.y;
            _2140.z = _1971.z;
            _2158 = _2140;
        }
        else
        {
            _2158 = _1775;
        }
        _2178 = _2158;
        break;
    } while(false);
    out._entryPointOutput = _2178;
    return out;
}

