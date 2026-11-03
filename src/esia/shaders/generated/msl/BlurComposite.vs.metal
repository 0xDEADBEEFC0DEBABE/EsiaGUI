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
    float2 _entryPointOutput_uv [[user(locn0)]];
    float4 gl_Position [[position]];
};

vertex esia_main_out esia_main(constant WgtFrame& _45 [[buffer(0)]], uint gl_VertexIndex [[vertex_id]])
{
    esia_main_out out = {};
    float2 _119 = float2(((gl_VertexIndex & 1u) != 0u) ? 2.0 : 0.0, ((gl_VertexIndex & 2u) != 0u) ? 2.0 : 0.0);
    out.gl_Position = float4((_119 * float2(2.0, (_45.gXform.y < 0.0) ? (-2.0) : 2.0)) + float2(-1.0, (_45.gXform.y < 0.0) ? 1.0 : (-1.0)), 0.0, 1.0);
    out._entryPointOutput_uv = _119;
    return out;
}

