// Esia - the shader model 3 prelude of the Direct3D 9 backend: esia_common.hlsli's portability hooks for vs_3_0 /
// ps_3_0 (docs/backends/README.md, 4.4). Compiled before the shared sources, as fxc /Gec (D3DCOMPILE_ENABLE_BACKWARDS
// _COMPATIBILITY) with ESIA_FX_STORAGE_TEXTURE=1. The backend prepends it to the source text: fxc cannot #include
// a macro, so the ESIA_SHADER_PRELUDE route of esia_common.hlsli does not work with it (STATUS.md, core requests).
//
// Registers are left to the compiler and read back from the bytecode's constant table (CTAB) by name: the same
// source then serves both stages (gFxData is vertex sampler 0 in FxVS, some pixel sampler in FxPS) without a
// stage define, and constants the compiler dropped cost nothing.

// SM3 counts code against the pixel shader's instruction slots: esia_common.hlsli's compact forms of the backdrop
// sampling (each level's code once, the two levels of a blur in a loop)
#define ESIA_COMPACT 1

// no binding annotations; cbuffers are plain groups of globals (float4 c registers)
#define ESIA_BINDING(n)
#define ESIA_LOCATION(n)
#define ESIA_CBUFFER(name, reg, binding) cbuffer name

// textures are samplers (sampler states come from the backend: linear, or point for t7 and LayerComposite's t0).
// Every texture has one mip level, so an explicit level 0 is what tex2D would read - and tex2Dlod is also allowed
// under the dynamic branches of the FX shader, where SM3 has no gradients.
#define ESIA_TEXTURE(name, reg, binding) sampler2D name
#define ESIA_TEXTURE_ARG sampler2D
#define ESIA_SAMPLE(tex, smp, uv) tex2Dlod(tex, float4(uv, 0.0, 0.0))
#define ESIA_SAMPLE_LEVEL(tex, smp, uv) tex2Dlod(tex, float4(uv, 0.0, 0.0))
// only the FX instance texture is fetched by texel: its size (xy) and 1 / size (zw) come in gEsiaFxSize
#define ESIA_LOAD(tex, texel) tex2Dlod(tex, float4(((float2)(texel) + 0.5) * gEsiaFxSize.zw, 0.0, 0.0))
#define ESIA_SAMPLER(name, reg, binding) static const float name##Unused = 0.0

// No integers in SM3: feature bits are tested with float arithmetic. A flag that ESIA_FX_FEATURES masked out is the
// constant 0, which folds the test (and the feature's code) away; the divisor never reaches 0.
#define ESIA_HAS(bits, flag) ((flag) != 0 && fmod(floor((float)(bits) / max((float)(flag), 1.0)), 2.0) >= 1.0)

// Vertex and instance ids come from two vertex streams of floats (TEXCOORD6: 0..3 per quad corner, TEXCOORD7: the
// instance, stepped once per instance with SetStreamSourceFreq).
#define ESIA_VERTEX_ID(name) float name : TEXCOORD6
#define ESIA_INSTANCE_ID(name) float name : TEXCOORD7

// No flat interpolation: the instance index is interpolated (constant over the quad) and rounded.
#define ESIA_FLAT
#define ESIA_FLAT_UINT(v) floor((v) + 0.5)

// Direct3D 9 puts pixel centers on integer coordinates: move every vertex by half a pixel of the current target
// (gEsiaHalfPixel = (-1 / width, +1 / height), set by the backend at each pass) so rasterization matches D3D10+.
// The renderer sets gConv.y = 0.5 (Caps::halfPixelOffset): VPOS then yields pixel centers too.
#define ESIA_CLIP_POSITION(p) ((p) + float4(gEsiaHalfPixel.xy * (p).w, 0.0, 0.0))

// uint is emulated with floats in SM3 anyway, and fxc rejects uints it cannot prove positive ("X3548"): the shared
// sources' instance indices, feature words and shape kinds are exact float integers far below 2^24.
#define uint float
#define uint2 float2
#define uint3 float3
#define uint4 float4

float4 gEsiaHalfPixel;
float4 gEsiaFxSize;
