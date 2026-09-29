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

struct esia_main_out
{
    float4 _entryPointOutput_col [[user(locn0)]];
    float2 _entryPointOutput_uv [[user(locn1)]];
    float4 gl_Position [[position]];
};

struct esia_main_in
{
    float2 v_pos [[attribute(0)]];
    float2 v_uv [[attribute(1)]];
    float4 v_col [[attribute(2)]];
};

vertex esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _28 [[buffer(0)]])
{
    esia_main_out out = {};
    out.gl_Position = float4((in.v_pos * _28.gXform.xy) + _28.gXform.zw, 0.0, 1.0);
    out._entryPointOutput_col = in.v_col;
    out._entryPointOutput_uv = in.v_uv;
    return out;
}

