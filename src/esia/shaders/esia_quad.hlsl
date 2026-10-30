// Esia - quads drawn instanced (esia::QuadInstance): textured quads (glyphs, the Painter's flat tiles) and plain
// shapes (esia::QuadShape: the fill or the stroke of a rounded rectangle or of a segment, a solid rectangle), in the
// same draws. QuadVS makes a textured quad from its rows for the pixel shaders of esia_ui.hlsl (programs QuadImage,
// QuadGray, QuadLcd, QuadLcdGray: batches of textured quads alone); QuadShapeVS makes either, for pixel shaders that
// also take a shape's coverage - the FX shader's fill or stroke of the same distance (SdRoundRect / SdSegment, the
// same antialiasing), so a plain shape covers the same pixels as its FX instance would - or a polyline's piece or end,
// with its geometry's pixels and coverage (programs QuadShapeImage .. QuadShapeLcdGray: QuadImagePS, QuadGrayPS,
// QuadLcdPS, QuadLcdGrayPS). They also tell plain coverage in a glyph page (an SVG icon's alpha) from the glyphs: drawn
// as an image of it, without text's composition.
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

// The quad's color: whole numbers below 2^24 (esia::QuadInstance), divided by powers of two and floored, come apart
// exactly
float4 QuadColor(float4 packed)
{
    const float b = floor(packed.z / 65536.0), g = floor((packed.z - b * 65536.0) / 256.0);
    const float r = packed.z - b * 65536.0 - g * 256.0;
    return float4(r, g, b, packed.w) / 255.0;
}

// ... and its uv at the corner
float2 QuadUv(float4 packed, bool right, bool bottom)
{
    const float v0 = floor(packed.x / 4096.0), u0 = packed.x - v0 * 4096.0;
    const float dv = floor(packed.y / 4096.0), du = packed.y - dv * 4096.0;
    return float2(u0 + (right ? du : 0.0), v0 + (bottom ? dv : 0.0)) / 4096.0;
}

// Textured quads alone: their two rows, for esia_ui.hlsl's pixel shaders
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
    const bool right = ESIA_HAS(v.vid, 1u), bottom = ESIA_HAS(v.vid, 2u);
    UiPSIn o;
    const float2 pos = float2(right ? rect.z : rect.x, bottom ? rect.w : rect.y);
    o.pos = ESIA_CLIP_POSITION(float4(pos * gXform.xy + gXform.zw, 0.0, 1.0));
    o.col = QuadColor(packed);
    o.uv = QuadUv(packed, right, bottom);
    return o;
}

struct QuadPSIn
{
    float4 pos : SV_Position;
    float4 col : COLOR0;
    float2 uv : TEXCOORD0;                    // a textured quad's uv; a shape's position (UI units)
    ESIA_FLAT float4 shapeRect : TEXCOORD1;   // a shape: its rect, or the segment's end points
    ESIA_FLAT float4 shape : TEXCOORD2;       // ... radius (a segment's half thickness), smoothing, stroke width and align;
                                              // a polyline's inside and edge (half widths), an end's sides
    ESIA_FLAT float2 kind : TEXCOORD3;        // x: its esia::QuadShape (-1: a textured quad), y: UI units per pixel
};

// A polyline's quad corner (esia::QuadShape::LinePiece / LineCap, Painter::LineQuads). A piece's quad ends on the lines
// of its joints - the rungs of the line's geometry, their directions as diamond angles (QuadInstance::LineDirection) -
// `reach` along them either side: the two pieces of a joint, and an end and its piece, compute those corners alike from
// the same numbers, so their quads share the edge exactly and the rasterizer gives each pixel on it to one of them. An
// end's quad goes `outer` on from its joint's line, away from the point before.
float2 QuadLineCorner(float4 rect, bool cap, bool right, bool bottom, float jointA, float jointB, float reach, float outer)
{
    const bool atB = right && !cap;
    const float d = (atB ? jointB : jointA) / 32768.0;   // 0 .. 2 for half a turn
    float2 along = normalize(d <= 1.0 ? float2(1.0 - d, d) : float2(1.0 - d, 2.0 - d));
    along = dot(along, float2(rect.y - rect.w, rect.z - rect.x)) < 0.0 ? -along : along;   // bottom: left of rect.xy -> zw
    const float2 corner = (atB ? rect.zw : rect.xy) + along * (bottom ? reach : -reach);
    if (!(cap && right))
        return corner;
    const float2 away = float2(along.y, -along.x);
    return corner + (dot(away, rect.xy - rect.zw) < 0.0 ? -away : away) * outer;
}

QuadPSIn QuadShapeVS(QuadVSIn v)
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
    const bool right = ESIA_HAS(v.vid, 1u), bottom = ESIA_HAS(v.vid, 2u);
    QuadPSIn o;
    o.col = QuadColor(packed);
    o.shapeRect = rect;
    o.shape = float4(0.0, 0.0, 0.0, 0.0);
    float2 pos;
    [branch] if (packed.x < 0.0)
    {
        // a shape (esia::QuadInstance::SetShape): its kind, smoothing and stroke align from packed.x, its radius in
        // packed.y, its stroke width beside the alpha in packed.w - whole numbers below 2^24, divided by powers of two
        // and floored. Its rect - or the band along a segment, turned with it (as FxVS's) - and a pixel around it for
        // the antialiasing, past the part of a stroke outside the edge; a solid rect's own (its pixels are the ones the
        // rect covers). A polyline's piece or end: its joint's angle and its reach in packed.x, half its thickness in
        // packed.y, the piece's other joint or the end's sides beside the alpha; the line's edge half a pixel past half
        // its thickness, its inside half a pixel short of it (the geometry's).
        const float code = -packed.x - 1.0;
        const float rest = floor(code / 8.0), high = floor(packed.w / 256.0);
        const float kind = code - rest * 8.0;
        o.col.a = (packed.w - high * 256.0) / 255.0;
        const float unit = max(2.0 * gTarget.z / max(abs(gXform.x), 1e-6), 2.0 * gTarget.w / max(abs(gXform.y), 1e-6));   // UI units per pixel
        [branch] if (kind > 4.5)
        {
            const float joint = floor(rest / 32.0);
            const float inner = max(packed.y - unit * 0.5, 0.0), outer = packed.y + unit * 0.5;
            o.shape = float4(inner, outer, high, 0.0);
            pos = QuadLineCorner(rect, kind > 5.5, right, bottom, joint, high, outer * (1.0 + (rest - joint * 32.0) / 16.0), outer);
        }
        else
        {
            const float aligned = floor(rest / 1024.0);
            const float4 shape = float4(packed.y, (rest - aligned * 1024.0) / 1023.0, high / 64.0, aligned / 64.0);
            o.shape = shape;
            const float stroke = kind - 2.0 * floor(kind * 0.5);
            const float pad = kind > 3.5 ? 0.0 : unit + stroke * max(shape.z * shape.w, 0.0);
            if (kind >= 1.5 && kind < 3.5)
            {
                const float2 ab = rect.zw - rect.xy;
                const float len = length(ab);
                const float2 d = len > 1e-4 ? ab / len : float2(1.0, 0.0);
                const float2 n = float2(-d.y, d.x);
                const float w = shape.x + pad;
                pos = (right ? rect.zw + d * w : rect.xy - d * w) + n * (bottom ? w : -w);
            }
            else
                pos = float2(right ? rect.z + pad : rect.x - pad, bottom ? rect.w + pad : rect.y - pad);
        }
        o.kind = float2(kind, unit);
        o.uv = pos;
    }
    else
    {
        // a textured quad: a glyph or a tile (kind -1), or plain coverage (-2: its alpha + 256, QuadInstance::SetCoverage)
        pos = float2(right ? rect.z : rect.x, bottom ? rect.w : rect.y);
        o.kind = float2(-1.0, 0.0);
        [branch] if (packed.w > 255.5)
        {
            o.col.a = (packed.w - 256.0) / 255.0;
            o.kind.x = -2.0;
        }
        o.uv = QuadUv(packed, right, bottom);
    }
    o.pos = ESIA_CLIP_POSITION(float4(pos * gXform.xy + gXform.zw, 0.0, 1.0));
    return o;
}

// A polyline's coverage of the pixel as its geometry's triangles give it (Painter::Polyline): the color's alpha inside
// `inner`, falling linearly to 0 at `outer`. A piece measures across its segment; an end is half a polygon of `sides`
// sides round its point (the geometry's fan), measured across the side the pixel is in (at the point itself, inside).
float QuadLineCoverage(QuadPSIn i, float kind)
{
    const float inner = i.shape.x, outer = i.shape.y;
    const float2 a = i.shapeRect.xy, b = i.shapeRect.zw, q = i.uv - a;
    float cov;
    [branch] if (kind < 5.5)
        cov = saturate((outer - abs(dot(q, normalize(float2(a.y - b.y, b.x - a.x))))) / max(outer - inner, 1e-6));
    else
    {
        const float sides = floor(i.shape.z + 0.5), step = WGT_PI / sides;
        const float2 away = normalize(a - b), side = float2(-away.y, away.x);
        const float k = clamp(floor(atan2(max(dot(q, away), 0.0), dot(q, side)) / step), 0.0, sides - 1.0);
        const float mid = (k + 0.5) * step, c = cos(0.5 * step);
        cov = dot(q, q) < 1e-12 ? 1.0 : saturate((outer * c - dot(q, side * cos(mid) + away * sin(mid))) / max((outer - inner) * c, 1e-6));
    }
    return cov;
}

// A shape's coverage of the pixel, as FxPS takes its fill's (cov) or its stroke's (sc); a solid rect covers its pixels
// whole; a polyline's piece or end as its geometry. Its units per pixel come from QuadShapeVS, not from derivatives: in
// the branch they would be undefined (a software rasterizer gave a pixel of a thin line other ones). `kind` is rounded:
// without flat interpolation (SM3) it comes a little off its whole number.
float QuadShapeCoverage(QuadPSIn i)
{
    const float kind = floor(i.kind.x + 0.5), px = max(i.kind.y, 1e-4);
    float cov = 1.0;
    [branch] if (kind > 4.5)
        cov = QuadLineCoverage(i, kind);
    else if (kind < 3.5)
    {
        float d;
        [branch] if (kind >= 1.5)
            d = SdSegment(i.uv, i.shapeRect.xy, i.shapeRect.zw, i.shape.x);
        else
            d = SdRoundRect(i.uv - (i.shapeRect.xy + i.shapeRect.zw) * 0.5, max((i.shapeRect.zw - i.shapeRect.xy) * 0.5, 1e-3), i.shape.xxxx,
                            i.shape.y);
        cov = kind - 2.0 * floor(kind * 0.5) > 0.5
                  ? saturate(0.5 - (d - i.shape.z * i.shape.w) / px) - saturate(0.5 - (d + i.shape.z * (1.0 - i.shape.w)) / px)
                  : saturate(0.5 - d / px);
    }
    return cov;
}

// The vertex color's alpha, the shape's coverage and the edge fade
float QuadShapeAlpha(QuadPSIn i)
{
    return i.col.a * QuadShapeCoverage(i) * WgtEdgeFade(WgtPixelPos(i.pos).y);
}

// Plain coverage (kind -2, esia::QuadInstance::SetCoverage: an image's alpha kept in a coverage page): the color with
// the coverage as its alpha - UiColor's for an image of white texels with that alpha - none of text's composition
float4 QuadCoverageColor(QuadPSIn i, float cov)
{
    return UiColor(i.col, float4(1.0, 1.0, 1.0, cov), i.pos);
}

// The texel is read before the branch (a shape's reads the texture somewhere and drops it).
float4 QuadImagePS(QuadPSIn i) : SV_Target
{
    const float4 t = ESIA_SAMPLE(gTex, gLinear, i.uv);
    [branch] if (i.kind.x > -0.5)
        return WgtOutputStraight(float4(i.col.rgb, QuadShapeAlpha(i)));
    return UiColor(i.col, t, i.pos);
}

float4 QuadGrayPS(QuadPSIn i) : SV_Target
{
    const float4 t = ESIA_SAMPLE(gTex, gLinear, i.uv);
    [branch] if (i.kind.x > -0.5)
        return WgtOutputStraight(float4(i.col.rgb, QuadShapeAlpha(i)));
    [branch] if (i.kind.x < -1.5)
        return QuadCoverageColor(i, t.r);
    return TextGrayColor(i.col, t.r, i.pos);
}

float4 QuadLcdGrayPS(QuadPSIn i) : SV_Target
{
    const float4 t = ESIA_SAMPLE(gTex, gLinear, i.uv);
    [branch] if (i.kind.x > -0.5)
        return WgtOutputStraight(float4(i.col.rgb, QuadShapeAlpha(i)));
    [branch] if (i.kind.x < -1.5)
        return QuadCoverageColor(i, t.a);
    return TextGrayColor(i.col, t.a, i.pos);
}

// a shape among sub-pixel text, or plain coverage in its page (gray, R = G = B = A): the same alpha in every channel
// (dual-source blending then blends as straight alpha)
TextLcdOut QuadLcdPS(QuadPSIn i)
{
    const float4 t = ESIA_SAMPLE(gTex, gLinear, i.uv);
    [branch] if (i.kind.x > -0.5)
        return TextLcdOutput(i.col, QuadShapeAlpha(i).xxx);
    [branch] if (i.kind.x < -1.5)
        return TextLcdOutput(i.col, QuadCoverageColor(i, t.a).aaa);
    return TextLcdColor(i.col, t, i.pos);
}
