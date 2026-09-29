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

constant float2 _1002 = {};

constant spvUnsafeArray<float4, 13> _284 = spvUnsafeArray<float4, 13>({ float4(0.0), float4(0.01659999974071979522705078125, -0.080700002610683441162109375, 0.22269999980926513671875, -0.075099997222423553466796875), float4(0.0350000001490116119384765625, -0.17599999904632568359375, 0.43250000476837158203125, -0.1369999945163726806640625), float4(0.0542999990284442901611328125, -0.2820999920368194580078125, 0.63020002841949462890625, -0.18760000169277191162109375), float4(0.07389999926090240478515625, -0.3962999880313873291015625, 0.816699981689453125, -0.22869999706745147705078125), float4(0.09329999983310699462890625, -0.516099989414215087890625, 0.992600023746490478515625, -0.2615999877452850341796875), float4(0.112099997699260711669921875, -0.63950002193450927734375, 1.15880000591278076171875, -0.287699997425079345703125), float4(0.12999999523162841796875, -0.764900028705596923828125, 1.31589996814727783203125, -0.3079999983310699462890625), float4(0.146899998188018798828125, -0.891099989414215087890625, 1.4644000530242919921875, -0.3233999907970428466796875), float4(0.162699997425079345703125, -1.0169999599456787109375, 1.60510003566741943359375, -0.33469998836517333984375), float4(0.1773000061511993408203125, -1.1419999599456787109375, 1.73849999904632568359375, -0.34259998798370361328125), float4(0.19079999625682830810546875, -1.26520001888275146484375, 1.8650000095367431640625, -0.3476000130176544189453125), float4(0.2030999958515167236328125, -1.3863999843597412109375, 1.9851000308990478515625, -0.3501000106334686279296875) });

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

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _65 [[buffer(0)]], constant WgtDraw& _97 [[buffer(2)]], texture2d<float> gTex [[texture(3)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float4 _525 = gTex.sample(gLinear, in.i_uv);
    float3 _526 = _525.xyz;
    float3 _534 = mix(float3(dot(_526, float3(0.3333333432674407958984375))), _526, float3(_65.gText.w));
    float _613 = _65.gText.z * fast::clamp(4.0 * (0.75 - dot(in.i_col.xyz, float3(0.300000011920928955078125, 0.589999973773956298828125, 0.10999999940395355224609375))), 0.0, 1.0);
    float _542 = _534.x;
    float _624 = (_542 * (_613 + 1.0)) / ((_542 * _613) + 1.0);
    float _665 = fast::clamp((_65.gText.x - 1.0) * 10.0, 0.0, 12.0);
    int _668 = min(int(_665), 11);
    float4 _682 = mix(_284[_668], _284[_668 + 1], float4(_665 - float(_668))) * 0.25;
    float _552 = _534.y;
    float _693 = (_552 * (_613 + 1.0)) / ((_552 * _613) + 1.0);
    float _734 = fast::clamp((_65.gText.x - 1.0) * 10.0, 0.0, 12.0);
    int _737 = min(int(_734), 11);
    float4 _751 = mix(_284[_737], _284[_737 + 1], float4(_734 - float(_737))) * 0.25;
    float _562 = _534.z;
    float _762 = (_562 * (_613 + 1.0)) / ((_562 * _613) + 1.0);
    float _803 = fast::clamp((_65.gText.x - 1.0) * 10.0, 0.0, 12.0);
    int _806 = min(int(_803), 11);
    float4 _820 = mix(_284[_806], _284[_806 + 1], float4(_803 - float(_806))) * 0.25;
    float2 _829 = gl_FragCoord.xy + _65.gConv.yy;
    float2 _994;
    if (_65.gConv.x > 0.5)
    {
        float2 _974;
        _974.y = _65.gTarget.y - _829.y;
        _994 = _974;
    }
    else
    {
        _994 = _829;
    }
    float _995;
    if (_97.gFade.z > 0.0)
    {
        _995 = smoothstep(0.0, 1.0, fast::clamp((_994.y - _97.gFade.x) / _97.gFade.z, 0.0, 1.0));
    }
    else
    {
        _995 = 1.0;
    }
    float _996;
    if (_97.gFade.w > 0.0)
    {
        _996 = _995 * smoothstep(0.0, 1.0, fast::clamp((_97.gFade.y - _994.y) / _97.gFade.w, 0.0, 1.0));
    }
    else
    {
        _996 = _995;
    }
    float3 _580 = float3(fast::clamp(_624 + ((_624 * (1.0 - _624)) * ((((_682.x * in.i_col.x) + _682.y) * _624) + ((_682.z * in.i_col.x) + _682.w))), 0.0, 1.0), fast::clamp(_693 + ((_693 * (1.0 - _693)) * ((((_751.x * in.i_col.y) + _751.y) * _693) + ((_751.z * in.i_col.y) + _751.w))), 0.0, 1.0), fast::clamp(_762 + ((_762 * (1.0 - _762)) * ((((_820.x * in.i_col.z) + _820.y) * _762) + ((_820.z * in.i_col.z) + _820.w))), 0.0, 1.0)) * (in.i_col.w * _996);
    float4 _587 = float4(in.i_col.xyz, 1.0);
    float4 _1000;
    if (_65.gTime.w > 0.5)
    {
        float3 _901 = fast::clamp(_587.xyz, float3(0.0), float3(1.0));
        float3 _913 = select(pow((_901 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _901 * float3(0.077399380505084991455078125), _901 <= float3(0.040449999272823333740234375));
        float4 _976 = _587;
        _976.x = _913.x;
        _976.y = _913.y;
        _976.z = _913.z;
        _1000 = _976;
    }
    else
    {
        _1000 = _587;
    }
    out._entryPointOutput_color = _1000;
    out._entryPointOutput_alpha = float4(_580, fast::max(_580.x, fast::max(_580.y, _580.z)));
    return out;
}

