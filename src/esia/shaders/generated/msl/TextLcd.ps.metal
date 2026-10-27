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

constant float2 _1187 = {};

constant spvUnsafeArray<float4, 13> _288 = spvUnsafeArray<float4, 13>({ float4(0.0), float4(0.01659999974071979522705078125, -0.080700002610683441162109375, 0.22269999980926513671875, -0.075099997222423553466796875), float4(0.0350000001490116119384765625, -0.17599999904632568359375, 0.43250000476837158203125, -0.1369999945163726806640625), float4(0.0542999990284442901611328125, -0.2820999920368194580078125, 0.63020002841949462890625, -0.18760000169277191162109375), float4(0.07389999926090240478515625, -0.3962999880313873291015625, 0.816699981689453125, -0.22869999706745147705078125), float4(0.09329999983310699462890625, -0.516099989414215087890625, 0.992600023746490478515625, -0.2615999877452850341796875), float4(0.112099997699260711669921875, -0.63950002193450927734375, 1.15880000591278076171875, -0.287699997425079345703125), float4(0.12999999523162841796875, -0.764900028705596923828125, 1.31589996814727783203125, -0.3079999983310699462890625), float4(0.146899998188018798828125, -0.891099989414215087890625, 1.4644000530242919921875, -0.3233999907970428466796875), float4(0.162699997425079345703125, -1.0169999599456787109375, 1.60510003566741943359375, -0.33469998836517333984375), float4(0.1773000061511993408203125, -1.1419999599456787109375, 1.73849999904632568359375, -0.34259998798370361328125), float4(0.19079999625682830810546875, -1.26520001888275146484375, 1.8650000095367431640625, -0.3476000130176544189453125), float4(0.2030999958515167236328125, -1.3863999843597412109375, 1.9851000308990478515625, -0.3501000106334686279296875) });

struct esia_main_out
{
    float4 _entryPointOutput_color [[color(0)]];
    float4 _entryPointOutput_alpha [[color(0), index(1)]];
};

struct esia_main_in
{
    float4 i_col [[user(locn0)]];
    float2 i_uv [[user(locn1)]];
};

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _69 [[buffer(0)]], constant WgtDraw& _101 [[buffer(2)]], texture2d<float> gTex [[texture(3)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float4 _578 = gTex.sample(gLinear, in.i_uv);
    float3 _1184;
    if ((_578.x == _578.y) && (_578.y == _578.z))
    {
        float _592 = _578.x;
        float _710 = _69.gText.y * fast::clamp(4.0 * (0.75 - dot(in.i_col.xyz, float3(0.300000011920928955078125, 0.589999973773956298828125, 0.10999999940395355224609375))), 0.0, 1.0);
        float _694 = dot(in.i_col.xyz, float3(0.25, 0.5, 0.25));
        float _721 = (_592 * (_710 + 1.0)) / ((_592 * _710) + 1.0);
        float _762 = fast::clamp((_69.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _765 = min(int(_762), 11);
        float4 _779 = mix(_288[_765], _288[_765 + 1], float4(_762 - float(_765))) * 0.25;
        _1184 = float3(fast::clamp(_721 + ((_721 * (1.0 - _721)) * ((((_779.x * _694) + _779.y) * _721) + ((_779.z * _694) + _779.w))), 0.0, 1.0));
    }
    else
    {
        float3 _606 = mix(_578.www, _578.xyz, float3(_69.gText.w));
        float _788 = _69.gText.z * fast::clamp(4.0 * (0.75 - dot(in.i_col.xyz, float3(0.300000011920928955078125, 0.589999973773956298828125, 0.10999999940395355224609375))), 0.0, 1.0);
        float _614 = _606.x;
        float _799 = (_614 * (_788 + 1.0)) / ((_614 * _788) + 1.0);
        float _840 = fast::clamp((_69.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _843 = min(int(_840), 11);
        float4 _857 = mix(_288[_843], _288[_843 + 1], float4(_840 - float(_843))) * 0.25;
        float _624 = _606.y;
        float _868 = (_624 * (_788 + 1.0)) / ((_624 * _788) + 1.0);
        float _909 = fast::clamp((_69.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _912 = min(int(_909), 11);
        float4 _926 = mix(_288[_912], _288[_912 + 1], float4(_909 - float(_912))) * 0.25;
        float _634 = _606.z;
        float _937 = (_634 * (_788 + 1.0)) / ((_634 * _788) + 1.0);
        float _978 = fast::clamp((_69.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _981 = min(int(_978), 11);
        float4 _995 = mix(_288[_981], _288[_981 + 1], float4(_978 - float(_981))) * 0.25;
        _1184 = float3(fast::clamp(_799 + ((_799 * (1.0 - _799)) * ((((_857.x * in.i_col.x) + _857.y) * _799) + ((_857.z * in.i_col.x) + _857.w))), 0.0, 1.0), fast::clamp(_868 + ((_868 * (1.0 - _868)) * ((((_926.x * in.i_col.y) + _926.y) * _868) + ((_926.z * in.i_col.y) + _926.w))), 0.0, 1.0), fast::clamp(_937 + ((_937 * (1.0 - _937)) * ((((_995.x * in.i_col.z) + _995.y) * _937) + ((_995.z * in.i_col.z) + _995.w))), 0.0, 1.0));
    }
    float2 _1004 = gl_FragCoord.xy + _69.gConv.yy;
    float2 _1178;
    if (_69.gConv.x > 0.5)
    {
        float2 _1158;
        _1158.y = _69.gTarget.y - _1004.y;
        _1178 = _1158;
    }
    else
    {
        _1178 = _1004;
    }
    float _1179;
    if (_101.gFade.z > 0.0)
    {
        _1179 = smoothstep(0.0, 1.0, fast::clamp((_1178.y - _101.gFade.x) / _101.gFade.z, 0.0, 1.0));
    }
    else
    {
        _1179 = 1.0;
    }
    float _1180;
    if (_101.gFade.w > 0.0)
    {
        _1180 = _1179 * smoothstep(0.0, 1.0, fast::clamp((_101.gFade.y - _1178.y) / _101.gFade.w, 0.0, 1.0));
    }
    else
    {
        _1180 = _1179;
    }
    float3 _653 = _1184 * (in.i_col.w * _1180);
    float4 _660 = float4(in.i_col.xyz, 1.0);
    float4 _1185;
    if (_69.gTime.w > 0.5)
    {
        float3 _1076 = fast::clamp(_660.xyz, float3(0.0), float3(1.0));
        float3 _1088 = select(pow((_1076 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _1076 * float3(0.077399380505084991455078125), _1076 <= float3(0.040449999272823333740234375));
        float4 _1160 = _660;
        _1160.x = _1088.x;
        _1160.y = _1088.y;
        _1160.z = _1088.z;
        _1185 = _1160;
    }
    else
    {
        _1185 = _660;
    }
    out._entryPointOutput_color = _1185;
    out._entryPointOutput_alpha = float4(_653, fast::max(_653.x, fast::max(_653.y, _653.z)));
    return out;
}

