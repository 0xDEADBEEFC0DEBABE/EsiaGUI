#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

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

struct WgtPass
{
    float4 gPass0;
    float4 gPass1;
};

struct esia_main_out
{
    float4 _entryPointOutput [[color(0)]];
};

struct esia_main_in
{
    float2 i_uv [[user(locn0)]];
};

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _29 [[buffer(0)]], constant WgtPass& _64 [[buffer(1)]], texture2d<float> gTex [[texture(3)]], sampler gLinear [[sampler(11)]])
{
    esia_main_out out = {};
    float2 _432 = fast::clamp(in.i_uv + (_64.gPass0.xy * float2(-2.0)), _64.gPass1.xy, _64.gPass1.zw);
    float2 _843;
    if (_29.gConv.x > 0.5)
    {
        float2 _806 = _432;
        _806.y = 1.0 - _432.y;
        _843 = _806;
    }
    else
    {
        _843 = _432;
    }
    float2 _461 = fast::clamp(in.i_uv + (_64.gPass0.xy * float2(0.0, -2.0)), _64.gPass1.xy, _64.gPass1.zw);
    float2 _844;
    if (_29.gConv.x > 0.5)
    {
        float2 _809 = _461;
        _809.y = 1.0 - _461.y;
        _844 = _809;
    }
    else
    {
        _844 = _461;
    }
    float4 _463 = gTex.sample(gLinear, _844, level(0.0));
    float2 _490 = fast::clamp(in.i_uv + (_64.gPass0.xy * float2(2.0, -2.0)), _64.gPass1.xy, _64.gPass1.zw);
    float2 _845;
    if (_29.gConv.x > 0.5)
    {
        float2 _812 = _490;
        _812.y = 1.0 - _490.y;
        _845 = _812;
    }
    else
    {
        _845 = _490;
    }
    float2 _519 = fast::clamp(in.i_uv + (_64.gPass0.xy * float2(-1.0)), _64.gPass1.xy, _64.gPass1.zw);
    float2 _846;
    if (_29.gConv.x > 0.5)
    {
        float2 _815 = _519;
        _815.y = 1.0 - _519.y;
        _846 = _815;
    }
    else
    {
        _846 = _519;
    }
    float2 _548 = fast::clamp(in.i_uv + (_64.gPass0.xy * float2(1.0, -1.0)), _64.gPass1.xy, _64.gPass1.zw);
    float2 _847;
    if (_29.gConv.x > 0.5)
    {
        float2 _818 = _548;
        _818.y = 1.0 - _548.y;
        _847 = _818;
    }
    else
    {
        _847 = _548;
    }
    float2 _577 = fast::clamp(in.i_uv + (_64.gPass0.xy * float2(-2.0, 0.0)), _64.gPass1.xy, _64.gPass1.zw);
    float2 _848;
    if (_29.gConv.x > 0.5)
    {
        float2 _821 = _577;
        _821.y = 1.0 - _577.y;
        _848 = _821;
    }
    else
    {
        _848 = _577;
    }
    float4 _579 = gTex.sample(gLinear, _848, level(0.0));
    float2 _606 = fast::clamp(in.i_uv, _64.gPass1.xy, _64.gPass1.zw);
    float2 _849;
    if (_29.gConv.x > 0.5)
    {
        float2 _824 = _606;
        _824.y = 1.0 - _606.y;
        _849 = _824;
    }
    else
    {
        _849 = _606;
    }
    float4 _608 = gTex.sample(gLinear, _849, level(0.0));
    float2 _635 = fast::clamp(in.i_uv + (_64.gPass0.xy * float2(2.0, 0.0)), _64.gPass1.xy, _64.gPass1.zw);
    float2 _850;
    if (_29.gConv.x > 0.5)
    {
        float2 _827 = _635;
        _827.y = 1.0 - _635.y;
        _850 = _827;
    }
    else
    {
        _850 = _635;
    }
    float4 _637 = gTex.sample(gLinear, _850, level(0.0));
    float2 _664 = fast::clamp(in.i_uv + (_64.gPass0.xy * float2(-1.0, 1.0)), _64.gPass1.xy, _64.gPass1.zw);
    float2 _851;
    if (_29.gConv.x > 0.5)
    {
        float2 _830 = _664;
        _830.y = 1.0 - _664.y;
        _851 = _830;
    }
    else
    {
        _851 = _664;
    }
    float2 _693 = fast::clamp(in.i_uv + _64.gPass0.xy, _64.gPass1.xy, _64.gPass1.zw);
    float2 _852;
    if (_29.gConv.x > 0.5)
    {
        float2 _833 = _693;
        _833.y = 1.0 - _693.y;
        _852 = _833;
    }
    else
    {
        _852 = _693;
    }
    float2 _722 = fast::clamp(in.i_uv + (_64.gPass0.xy * float2(-2.0, 2.0)), _64.gPass1.xy, _64.gPass1.zw);
    float2 _853;
    if (_29.gConv.x > 0.5)
    {
        float2 _836 = _722;
        _836.y = 1.0 - _722.y;
        _853 = _836;
    }
    else
    {
        _853 = _722;
    }
    float2 _751 = fast::clamp(in.i_uv + (_64.gPass0.xy * float2(0.0, 2.0)), _64.gPass1.xy, _64.gPass1.zw);
    float2 _854;
    if (_29.gConv.x > 0.5)
    {
        float2 _839 = _751;
        _839.y = 1.0 - _751.y;
        _854 = _839;
    }
    else
    {
        _854 = _751;
    }
    float4 _753 = gTex.sample(gLinear, _854, level(0.0));
    float2 _780 = fast::clamp(in.i_uv + (_64.gPass0.xy * float2(2.0)), _64.gPass1.xy, _64.gPass1.zw);
    float2 _855;
    if (_29.gConv.x > 0.5)
    {
        float2 _842 = _780;
        _842.y = 1.0 - _780.y;
        _855 = _842;
    }
    else
    {
        _855 = _780;
    }
    out._entryPointOutput = (((((((gTex.sample(gLinear, _846, level(0.0)) + gTex.sample(gLinear, _847, level(0.0))) + gTex.sample(gLinear, _851, level(0.0))) + gTex.sample(gLinear, _852, level(0.0))) * 0.125) + ((((gTex.sample(gLinear, _843, level(0.0)) + _463) + _579) + _608) * 0.03125)) + ((((_463 + gTex.sample(gLinear, _845, level(0.0))) + _608) + _637) * 0.03125)) + ((((_579 + _608) + gTex.sample(gLinear, _853, level(0.0))) + _753) * 0.03125)) + ((((_608 + _637) + _753) + gTex.sample(gLinear, _855, level(0.0))) * 0.03125);
    return out;
}

