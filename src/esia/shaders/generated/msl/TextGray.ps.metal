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

constant float2 _729 = {};

constant spvUnsafeArray<float4, 13> _293 = spvUnsafeArray<float4, 13>({ float4(0.0), float4(0.01659999974071979522705078125, -0.080700002610683441162109375, 0.22269999980926513671875, -0.075099997222423553466796875), float4(0.0350000001490116119384765625, -0.17599999904632568359375, 0.43250000476837158203125, -0.1369999945163726806640625), float4(0.0542999990284442901611328125, -0.2820999920368194580078125, 0.63020002841949462890625, -0.18760000169277191162109375), float4(0.07389999926090240478515625, -0.3962999880313873291015625, 0.816699981689453125, -0.22869999706745147705078125), float4(0.09329999983310699462890625, -0.516099989414215087890625, 0.992600023746490478515625, -0.2615999877452850341796875), float4(0.112099997699260711669921875, -0.63950002193450927734375, 1.15880000591278076171875, -0.287699997425079345703125), float4(0.12999999523162841796875, -0.764900028705596923828125, 1.31589996814727783203125, -0.3079999983310699462890625), float4(0.146899998188018798828125, -0.891099989414215087890625, 1.4644000530242919921875, -0.3233999907970428466796875), float4(0.162699997425079345703125, -1.0169999599456787109375, 1.60510003566741943359375, -0.33469998836517333984375), float4(0.1773000061511993408203125, -1.1419999599456787109375, 1.73849999904632568359375, -0.34259998798370361328125), float4(0.19079999625682830810546875, -1.26520001888275146484375, 1.8650000095367431640625, -0.3476000130176544189453125), float4(0.2030999958515167236328125, -1.3863999843597412109375, 1.9851000308990478515625, -0.3501000106334686279296875) });

struct esia_main_out
{
    float4 _entryPointOutput [[color(0)]];
};

struct esia_main_in
{
    float4 i_col [[user(locn0)]];
    float2 i_uv [[user(locn1)]];
};

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _74 [[buffer(0)]], constant WgtDraw& _106 [[buffer(2)]], texture2d<float> gTex [[texture(3)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float4 _461 = gTex.sample(gLinear, in.i_uv);
    float _464 = _461.x;
    float _526 = _74.gText.y * fast::clamp(4.0 * (0.75 - dot(in.i_col.xyz, float3(0.300000011920928955078125, 0.589999973773956298828125, 0.10999999940395355224609375))), 0.0, 1.0);
    float _510 = dot(in.i_col.xyz, float3(0.25, 0.5, 0.25));
    float _537 = (_464 * (_526 + 1.0)) / ((_464 * _526) + 1.0);
    float _578 = fast::clamp((_74.gText.x - 1.0) * 10.0, 0.0, 12.0);
    int _581 = min(int(_578), 11);
    float4 _595 = mix(_293[_581], _293[_581 + 1], float4(_578 - float(_581))) * 0.25;
    float2 _604 = gl_FragCoord.xy + _74.gConv.yy;
    float2 _725;
    if (_74.gConv.x > 0.5)
    {
        float2 _709;
        _709.y = _74.gTarget.y - _604.y;
        _725 = _709;
    }
    else
    {
        _725 = _604;
    }
    float _726;
    if (_106.gFade.z > 0.0)
    {
        _726 = smoothstep(0.0, 1.0, fast::clamp((_725.y - _106.gFade.x) / _106.gFade.z, 0.0, 1.0));
    }
    else
    {
        _726 = 1.0;
    }
    float _727;
    if (_106.gFade.w > 0.0)
    {
        _727 = _726 * smoothstep(0.0, 1.0, fast::clamp((_106.gFade.y - _725.y) / _106.gFade.w, 0.0, 1.0));
    }
    else
    {
        _727 = _726;
    }
    float4 _492 = float4(in.i_col.xyz, (in.i_col.w * fast::clamp(_537 + ((_537 * (1.0 - _537)) * ((((_595.x * _510) + _595.y) * _537) + ((_595.z * _510) + _595.w))), 0.0, 1.0)) * _727);
    float4 _728;
    if (_74.gTime.w > 0.5)
    {
        float3 _676 = fast::clamp(_492.xyz, float3(0.0), float3(1.0));
        float3 _688 = select(pow((_676 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _676 * float3(0.077399380505084991455078125), _676 <= float3(0.040449999272823333740234375));
        float4 _711 = _492;
        _711.x = _688.x;
        _711.y = _688.y;
        _711.z = _688.z;
        _728 = _711;
    }
    else
    {
        _728 = _492;
    }
    out._entryPointOutput = _728;
    return out;
}

