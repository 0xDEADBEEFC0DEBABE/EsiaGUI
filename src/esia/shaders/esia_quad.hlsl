// Esia - textured quads drawn instanced (esia::QuadInstance: glyphs, the Painter's flat tiles). QuadVS makes each
// instance's quad from its rect and packed rows for the pixel shaders of esia_ui.hlsl. Programs QuadImage, QuadGray,
// QuadLcd, QuadLcdGray (UiPS, TextGrayPS, TextLcdPS, TextLcdGrayPS).
#include "esia_ui.hlsl"

#ifdef ESIA_FX_STORAGE_TEXTURE
// Where the instance data is a texture (OpenGL, Direct3D 9 / 10) the quads come as instance-rate attributes
// (rhi::Caps::quadAttributes, VertexLayout::QuadInstance): uploading them into the texture and fetching two texels a
// vertex cost more than the vertices they replaced did.
struct QuadVSIn
{
    ESIA_VERTEX_ID(vid);
    ESIA_LOCATION(0) float4 rect : TEXCOORD4;
    ESIA_LOCATION(1) float4 packed : TEXCOORD5;
};
#else
// The quads follow the FX instances in the instance buffer, from gConv.w on.
ESIA_BINDING(10) StructuredBuffer<float4> gFxData : register(t7);
float4 QuadFetch(uint quad, uint row) { return gFxData[(uint)gConv.w + quad * 2u + row]; }

struct QuadVSIn
{
    ESIA_VERTEX_ID(vid);
    ESIA_INSTANCE_ID(iid);
#ifdef ESIA_INSTANCE_ATTRIBUTE
    uint index : INSTANCEINDEX;   // instance-rate: counts the draw's first instance (rhi::Caps::drawFirstInstance)
#endif
};
#endif

UiPSIn QuadVS(QuadVSIn v)
{
#ifdef ESIA_FX_STORAGE_TEXTURE
    const float4 rect = v.rect;
    const float4 packed = v.packed;
#else
#ifdef ESIA_INSTANCE_ATTRIBUTE
    const uint quad = v.index;
#else
    const uint quad = v.iid + (uint)gDrawInfo.x;
#endif
    const float4 rect = QuadFetch(quad, 0u);
    const float4 packed = QuadFetch(quad, 1u);
#endif
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
