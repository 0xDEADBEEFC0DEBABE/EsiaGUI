// Esia - textured quads drawn instanced (esia::QuadInstance: glyphs, the Painter's flat tiles). QuadVS makes each
// instance's quad from its two rows of the instance data for the pixel shaders of esia_ui.hlsl. Programs QuadImage,
// QuadGray, QuadLcd, QuadLcdGray (UiPS, TextGrayPS, TextLcdPS, TextLcdGrayPS).
#include "esia_ui.hlsl"

// The quads follow the FX instances in the instance data, from gConv.w on: a row of the buffer, or of the texture
// (whose rows hold gConv.z FX instances of 24 texels; the quads take 16 of each instance's texels, 8 quads: a power of
// two per row, as SM3's float modulo needs)
#ifdef ESIA_FX_STORAGE_TEXTURE
ESIA_TEXTURE(gFxData, t7, 10);
float4 QuadFetch(uint quad, uint row)
{
    const uint perRow = max((uint)gConv.z, 1u) * 8u;
    return ESIA_LOAD(gFxData, int2((int)((quad % perRow) * 2u + row), (int)((uint)gConv.w + quad / perRow)));
}
#else
ESIA_BINDING(10) StructuredBuffer<float4> gFxData : register(t7);
float4 QuadFetch(uint quad, uint row) { return gFxData[(uint)gConv.w + quad * 2u + row]; }
#endif

struct QuadVSIn
{
    ESIA_VERTEX_ID(vid);
    ESIA_INSTANCE_ID(iid);
#ifdef ESIA_INSTANCE_ATTRIBUTE
    uint index : INSTANCEINDEX;   // instance-rate: counts the draw's first instance (rhi::Caps::drawFirstInstance)
#endif
};

UiPSIn QuadVS(QuadVSIn v)
{
#ifdef ESIA_INSTANCE_ATTRIBUTE
    const uint quad = v.index;
#else
    const uint quad = v.iid + (uint)gDrawInfo.x;
#endif
    const float4 rect = QuadFetch(quad, 0u);
    const float4 packed = QuadFetch(quad, 1u);
    // whole numbers below 2^24 (esia::QuadInstance): divided by powers of two and floored, they come apart exactly
    const float v0 = floor(packed.x / 4096.0), u0 = packed.x - v0 * 4096.0;
    const float dv = floor(packed.y / 4096.0), du = packed.y - dv * 4096.0;
    const float b = floor(packed.z / 65536.0), g = floor((packed.z - b * 65536.0) / 256.0);
    const float r = packed.z - b * 65536.0 - g * 256.0;
    const bool right = ESIA_HAS(v.vid, 1u), bottom = ESIA_HAS(v.vid, 2u);
    UiPSIn o;
    const float2 pos = float2(right ? rect.z : rect.x, bottom ? rect.w : rect.y);
    o.pos = ESIA_CLIP_POSITION(float4(pos * gXform.xy + gXform.zw, 0.0, 1.0));
    o.col = float4(r, g, b, packed.w) / 255.0;
    o.uv = float2(u0 + (right ? du : 0.0), v0 + (bottom ? dv : 0.0)) / 4096.0;
    return o;
}
