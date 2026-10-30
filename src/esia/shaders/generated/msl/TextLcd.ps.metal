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

constant float2 _1226 = {};

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
    float4 _576 = gTex.sample(gLinear, in.i_uv);
    float3 _1221;
    if ((_576.x == _576.y) && (_576.y == _576.z))
    {
        float _623 = _576.x;
        float _716 = _80.gText.y * fast::clamp(4.0 * (0.75 - dot(in.i_col.xyz, float3(0.300000011920928955078125, 0.589999973773956298828125, 0.10999999940395355224609375))), 0.0, 1.0);
        float _700 = dot(in.i_col.xyz, float3(0.25, 0.5, 0.25));
        float _727 = (_623 * (_716 + 1.0)) / ((_623 * _716) + 1.0);
        float _768 = fast::clamp((_80.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _771 = min(int(_768), 11);
        float4 _785 = mix(_299[_771], _299[_771 + 1], float4(_768 - float(_771))) * 0.25;
        _1221 = float3(fast::clamp(_727 + ((_727 * (1.0 - _727)) * ((((_785.x * _700) + _785.y) * _727) + ((_785.z * _700) + _785.w))), 0.0, 1.0));
    }
    else
    {
        float3 _636 = mix(_576.www, _576.xyz, float3(_80.gText.w));
        float _794 = _80.gText.z * fast::clamp(4.0 * (0.75 - dot(in.i_col.xyz, float3(0.300000011920928955078125, 0.589999973773956298828125, 0.10999999940395355224609375))), 0.0, 1.0);
        float _643 = _636.x;
        float _805 = (_643 * (_794 + 1.0)) / ((_643 * _794) + 1.0);
        float _846 = fast::clamp((_80.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _849 = min(int(_846), 11);
        float4 _863 = mix(_299[_849], _299[_849 + 1], float4(_846 - float(_849))) * 0.25;
        float _653 = _636.y;
        float _874 = (_653 * (_794 + 1.0)) / ((_653 * _794) + 1.0);
        float _915 = fast::clamp((_80.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _918 = min(int(_915), 11);
        float4 _932 = mix(_299[_918], _299[_918 + 1], float4(_915 - float(_918))) * 0.25;
        float _663 = _636.z;
        float _943 = (_663 * (_794 + 1.0)) / ((_663 * _794) + 1.0);
        float _984 = fast::clamp((_80.gText.x - 1.0) * 10.0, 0.0, 12.0);
        int _987 = min(int(_984), 11);
        float4 _1001 = mix(_299[_987], _299[_987 + 1], float4(_984 - float(_987))) * 0.25;
        _1221 = float3(fast::clamp(_805 + ((_805 * (1.0 - _805)) * ((((_863.x * in.i_col.x) + _863.y) * _805) + ((_863.z * in.i_col.x) + _863.w))), 0.0, 1.0), fast::clamp(_874 + ((_874 * (1.0 - _874)) * ((((_932.x * in.i_col.y) + _932.y) * _874) + ((_932.z * in.i_col.y) + _932.w))), 0.0, 1.0), fast::clamp(_943 + ((_943 * (1.0 - _943)) * ((((_1001.x * in.i_col.z) + _1001.y) * _943) + ((_1001.z * in.i_col.z) + _1001.w))), 0.0, 1.0));
    }
    float2 _1010 = gl_FragCoord.xy + _80.gConv.yy;
    float2 _1222;
    if (_80.gConv.x > 0.5)
    {
        float2 _1201;
        _1201.y = _80.gTarget.y - _1010.y;
        _1222 = _1201;
    }
    else
    {
        _1222 = _1010;
    }
    float _1223;
    if (_112.gFade.z > 0.0)
    {
        _1223 = smoothstep(0.0, 1.0, fast::clamp((_1222.y - _112.gFade.x) / _112.gFade.z, 0.0, 1.0));
    }
    else
    {
        _1223 = 1.0;
    }
    float _1224;
    if (_112.gFade.w > 0.0)
    {
        _1224 = _1223 * smoothstep(0.0, 1.0, fast::clamp((_112.gFade.y - _1222.y) / _112.gFade.w, 0.0, 1.0));
    }
    else
    {
        _1224 = _1223;
    }
    float3 _681 = _1221 * (in.i_col.w * _1224);
    float4 _1068 = float4(in.i_col.xyz, 1.0);
    float4 _1225;
    if (_80.gTime.w > 0.5)
    {
        float3 _1109 = fast::clamp(_1068.xyz, float3(0.0), float3(1.0));
        float3 _1121 = select(pow((_1109 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _1109 * float3(0.077399380505084991455078125), _1109 <= float3(0.040449999272823333740234375));
        float4 _1203 = _1068;
        _1203.x = _1121.x;
        _1203.y = _1121.y;
        _1203.z = _1121.z;
        _1225 = _1203;
    }
    else
    {
        _1225 = _1068;
    }
    out._entryPointOutput_color = _1225;
    out._entryPointOutput_alpha = float4(_681, fast::max(_681.x, fast::max(_681.y, _681.z)));
    return out;
}

