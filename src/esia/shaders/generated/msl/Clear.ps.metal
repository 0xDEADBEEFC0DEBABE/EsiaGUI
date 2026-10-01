#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct esia_main_out
{
    float4 _entryPointOutput [[color(0)]];
};

fragment esia_main_out esia_main()
{
    esia_main_out out = {};
    out._entryPointOutput = float4(0.0);
    return out;
}

