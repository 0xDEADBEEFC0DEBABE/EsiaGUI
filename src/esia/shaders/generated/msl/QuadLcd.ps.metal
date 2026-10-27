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

constant float2 _1191 = {};

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
    float4 _582 = gTex.sample(gLinear, in.i_uv);
    float3 _1188;
    if ((_582.x == _582.y) && (_582.y == _582.z))
    {
        float _596 = _582.x;
        float _714 = _69.gText.y * fast::clamp(4.0 * (0.75 - dot(in.i_col.xyz, float3(0.300000011920928955078125, 0.589999973773956298828125, 0.10999999940395355224609375))), 0.0, 1.0);
        float _698 = dot(in.i_col.xyz, float3(0.25, 0.5, 0.25));
        float _725 = (_596 * (_714 + 1.0)) / ((_596 * _714) + 1.0);
        float _766 = fast::clamp((_69.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _769 = min(int(_766), 11);
        float4 _783 = mix(_288[_769], _288[_769 + 1], float4(_766 - float(_769))) * 0.25;
        _1188 = float3(fast::clamp(_725 + ((_725 * (1.0 - _725)) * ((((_783.x * _698) + _783.y) * _725) + ((_783.z * _698) + _783.w))), 0.0, 1.0));
    }
    else
    {
        float3 _610 = mix(_582.www, _582.xyz, float3(_69.gText.w));
        float _792 = _69.gText.z * fast::clamp(4.0 * (0.75 - dot(in.i_col.xyz, float3(0.300000011920928955078125, 0.589999973773956298828125, 0.10999999940395355224609375))), 0.0, 1.0);
        float _618 = _610.x;
        float _803 = (_618 * (_792 + 1.0)) / ((_618 * _792) + 1.0);
        float _844 = fast::clamp((_69.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _847 = min(int(_844), 11);
        float4 _861 = mix(_288[_847], _288[_847 + 1], float4(_844 - float(_847))) * 0.25;
        float _628 = _610.y;
        float _872 = (_628 * (_792 + 1.0)) / ((_628 * _792) + 1.0);
        float _913 = fast::clamp((_69.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _916 = min(int(_913), 11);
        float4 _930 = mix(_288[_916], _288[_916 + 1], float4(_913 - float(_916))) * 0.25;
        float _638 = _610.z;
        float _941 = (_638 * (_792 + 1.0)) / ((_638 * _792) + 1.0);
        float _982 = fast::clamp((_69.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _985 = min(int(_982), 11);
        float4 _999 = mix(_288[_985], _288[_985 + 1], float4(_982 - float(_985))) * 0.25;
        _1188 = float3(fast::clamp(_803 + ((_803 * (1.0 - _803)) * ((((_861.x * in.i_col.x) + _861.y) * _803) + ((_861.z * in.i_col.x) + _861.w))), 0.0, 1.0), fast::clamp(_872 + ((_872 * (1.0 - _872)) * ((((_930.x * in.i_col.y) + _930.y) * _872) + ((_930.z * in.i_col.y) + _930.w))), 0.0, 1.0), fast::clamp(_941 + ((_941 * (1.0 - _941)) * ((((_999.x * in.i_col.z) + _999.y) * _941) + ((_999.z * in.i_col.z) + _999.w))), 0.0, 1.0));
    }
    float2 _1008 = gl_FragCoord.xy + _69.gConv.yy;
    float2 _1182;
    if (_69.gConv.x > 0.5)
    {
        float2 _1162;
        _1162.y = _69.gTarget.y - _1008.y;
        _1182 = _1162;
    }
    else
    {
        _1182 = _1008;
    }
    float _1183;
    if (_101.gFade.z > 0.0)
    {
        _1183 = smoothstep(0.0, 1.0, fast::clamp((_1182.y - _101.gFade.x) / _101.gFade.z, 0.0, 1.0));
    }
    else
    {
        _1183 = 1.0;
    }
    float _1184;
    if (_101.gFade.w > 0.0)
    {
        _1184 = _1183 * smoothstep(0.0, 1.0, fast::clamp((_101.gFade.y - _1182.y) / _101.gFade.w, 0.0, 1.0));
    }
    else
    {
        _1184 = _1183;
    }
    float3 _657 = _1188 * (in.i_col.w * _1184);
    float4 _664 = float4(in.i_col.xyz, 1.0);
    float4 _1189;
    if (_69.gTime.w > 0.5)
    {
        float3 _1080 = fast::clamp(_664.xyz, float3(0.0), float3(1.0));
        float3 _1092 = select(pow((_1080 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _1080 * float3(0.077399380505084991455078125), _1080 <= float3(0.040449999272823333740234375));
        float4 _1164 = _664;
        _1164.x = _1092.x;
        _1164.y = _1092.y;
        _1164.z = _1092.z;
        _1189 = _1164;
    }
    else
    {
        _1189 = _664;
    }
    out._entryPointOutput_color = _1189;
    out._entryPointOutput_alpha = float4(_657, fast::max(_657.x, fast::max(_657.y, _657.z)));
    return out;
}

