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

constant float2 _1230 = {};

constant spvUnsafeArray<float4, 13> _299 = spvUnsafeArray<float4, 13>({ float4(0.0), float4(0.01659999974071979522705078125, -0.080700002610683441162109375, 0.22269999980926513671875, -0.075099997222423553466796875), float4(0.0350000001490116119384765625, -0.17599999904632568359375, 0.43250000476837158203125, -0.1369999945163726806640625), float4(0.0542999990284442901611328125, -0.2820999920368194580078125, 0.63020002841949462890625, -0.18760000169277191162109375), float4(0.07389999926090240478515625, -0.3962999880313873291015625, 0.816699981689453125, -0.22869999706745147705078125), float4(0.09329999983310699462890625, -0.516099989414215087890625, 0.992600023746490478515625, -0.2615999877452850341796875), float4(0.112099997699260711669921875, -0.63950002193450927734375, 1.15880000591278076171875, -0.287699997425079345703125), float4(0.12999999523162841796875, -0.764900028705596923828125, 1.31589996814727783203125, -0.3079999983310699462890625), float4(0.146899998188018798828125, -0.891099989414215087890625, 1.4644000530242919921875, -0.3233999907970428466796875), float4(0.162699997425079345703125, -1.0169999599456787109375, 1.60510003566741943359375, -0.33469998836517333984375), float4(0.1773000061511993408203125, -1.1419999599456787109375, 1.73849999904632568359375, -0.34259998798370361328125), float4(0.19079999625682830810546875, -1.26520001888275146484375, 1.8650000095367431640625, -0.3476000130176544189453125), float4(0.2030999958515167236328125, -1.3863999843597412109375, 1.9851000308990478515625, -0.3501000106334686279296875) });

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

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _80 [[buffer(0)]], constant WgtDraw& _112 [[buffer(2)]], texture2d<float> gTex [[texture(3)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float4 _580 = gTex.sample(gLinear, in.i_uv);
    float3 _1225;
    if ((_580.x == _580.y) && (_580.y == _580.z))
    {
        float _627 = _580.x;
        float _720 = _80.gText.y * fast::clamp(4.0 * (0.75 - dot(in.i_col.xyz, float3(0.300000011920928955078125, 0.589999973773956298828125, 0.10999999940395355224609375))), 0.0, 1.0);
        float _704 = dot(in.i_col.xyz, float3(0.25, 0.5, 0.25));
        float _731 = (_627 * (_720 + 1.0)) / ((_627 * _720) + 1.0);
        float _772 = fast::clamp((_80.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _775 = min(int(_772), 11);
        float4 _789 = mix(_299[_775], _299[_775 + 1], float4(_772 - float(_775))) * 0.25;
        _1225 = float3(fast::clamp(_731 + ((_731 * (1.0 - _731)) * ((((_789.x * _704) + _789.y) * _731) + ((_789.z * _704) + _789.w))), 0.0, 1.0));
    }
    else
    {
        float3 _640 = mix(_580.www, _580.xyz, float3(_80.gText.w));
        float _798 = _80.gText.z * fast::clamp(4.0 * (0.75 - dot(in.i_col.xyz, float3(0.300000011920928955078125, 0.589999973773956298828125, 0.10999999940395355224609375))), 0.0, 1.0);
        float _647 = _640.x;
        float _809 = (_647 * (_798 + 1.0)) / ((_647 * _798) + 1.0);
        float _850 = fast::clamp((_80.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _853 = min(int(_850), 11);
        float4 _867 = mix(_299[_853], _299[_853 + 1], float4(_850 - float(_853))) * 0.25;
        float _657 = _640.y;
        float _878 = (_657 * (_798 + 1.0)) / ((_657 * _798) + 1.0);
        float _919 = fast::clamp((_80.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _922 = min(int(_919), 11);
        float4 _936 = mix(_299[_922], _299[_922 + 1], float4(_919 - float(_922))) * 0.25;
        float _667 = _640.z;
        float _947 = (_667 * (_798 + 1.0)) / ((_667 * _798) + 1.0);
        float _988 = fast::clamp((_80.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _991 = min(int(_988), 11);
        float4 _1005 = mix(_299[_991], _299[_991 + 1], float4(_988 - float(_991))) * 0.25;
        _1225 = float3(fast::clamp(_809 + ((_809 * (1.0 - _809)) * ((((_867.x * in.i_col.x) + _867.y) * _809) + ((_867.z * in.i_col.x) + _867.w))), 0.0, 1.0), fast::clamp(_878 + ((_878 * (1.0 - _878)) * ((((_936.x * in.i_col.y) + _936.y) * _878) + ((_936.z * in.i_col.y) + _936.w))), 0.0, 1.0), fast::clamp(_947 + ((_947 * (1.0 - _947)) * ((((_1005.x * in.i_col.z) + _1005.y) * _947) + ((_1005.z * in.i_col.z) + _1005.w))), 0.0, 1.0));
    }
    float2 _1014 = gl_FragCoord.xy + _80.gConv.yy;
    float2 _1226;
    if (_80.gConv.x > 0.5)
    {
        float2 _1205;
        _1205.y = _80.gTarget.y - _1014.y;
        _1226 = _1205;
    }
    else
    {
        _1226 = _1014;
    }
    float _1227;
    if (_112.gFade.z > 0.0)
    {
        _1227 = smoothstep(0.0, 1.0, fast::clamp((_1226.y - _112.gFade.x) / _112.gFade.z, 0.0, 1.0));
    }
    else
    {
        _1227 = 1.0;
    }
    float _1228;
    if (_112.gFade.w > 0.0)
    {
        _1228 = _1227 * smoothstep(0.0, 1.0, fast::clamp((_112.gFade.y - _1226.y) / _112.gFade.w, 0.0, 1.0));
    }
    else
    {
        _1228 = _1227;
    }
    float3 _685 = _1225 * (in.i_col.w * _1228);
    float4 _1072 = float4(in.i_col.xyz, 1.0);
    float4 _1229;
    if (_80.gTime.w > 0.5)
    {
        float3 _1113 = fast::clamp(_1072.xyz, float3(0.0), float3(1.0));
        float3 _1125 = select(pow((_1113 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _1113 * float3(0.077399380505084991455078125), _1113 <= float3(0.040449999272823333740234375));
        float4 _1207 = _1072;
        _1207.x = _1125.x;
        _1207.y = _1125.y;
        _1207.z = _1125.z;
        _1229 = _1207;
    }
    else
    {
        _1229 = _1072;
    }
    out._entryPointOutput_color = _1229;
    out._entryPointOutput_alpha = float4(_685, fast::max(_685.x, fast::max(_685.y, _685.z)));
    return out;
}

