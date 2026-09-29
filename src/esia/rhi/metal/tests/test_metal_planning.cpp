// Metal backend: pipeline keys, blit layouts, readback conversion, copy plans.
// UNVERIFIED: needs macOS (passes on Linux and under Wine).
#include "esia_test.hpp"
#include "metal_planning.hpp"
#include <cstring>
#include <set>
#include <unordered_set>

using namespace esia::rhi;
using namespace esia::rhi::metal;

ESIA_TEST(MetalPlanning, PipelineKeys)
{
    Caps caps;
    caps.dualSourceBlend = true;
    // every combination the renderer can ask for gets its own key and packed value
    std::set<std::uint32_t> packed;
    std::unordered_set<PipelineKey, PipelineKeyHash> keys;
    int n = 0;
    for (int p = 0; p < (int)ShaderProgram::Count; ++p)
        for (BlendMode b : {BlendMode::Opaque, BlendMode::Straight, BlendMode::Premultiplied, BlendMode::DualSourceLcd})
            for (Format f : {Format::RGBA8_UNORM, Format::RGBA8_SRGB, Format::BGRA8_UNORM, Format::BGRA8_SRGB, Format::RGB10A2_UNORM, Format::RGBA16_FLOAT})
                for (int s : {1, 4})
                {
                    PipelineDesc d;
                    d.program = (ShaderProgram)p;
                    d.layout = ProgramUsesVertices(d.program) ? VertexLayout::UiVertex : VertexLayout::None;
                    d.topology = d.program == ShaderProgram::Fx ? Topology::TriangleStrip : Topology::TriangleList;
                    d.blend = b;
                    d.targetFormat = f;
                    d.samples = s;
                    PipelineKey k;
                    std::string why;
                    const bool ok = MakePipelineKey(d, caps, k, why);
                    // dual-source blending belongs to TextLcd and TextLcd needs it
                    ESIA_CHECK(ok == ((d.program == ShaderProgram::TextLcd) == (b == BlendMode::DualSourceLcd)));
                    if (!ok)
                        continue;
                    ++n;
                    packed.insert(PackPipelineKey(k));
                    keys.insert(k);
                }
    ESIA_CHECK(n > 0 && (int)packed.size() == n && (int)keys.size() == n);

    PipelineDesc d;
    PipelineKey k, k2;
    std::string why;
    ESIA_CHECK(MakePipelineKey(d, caps, k, why));
    d.topology = Topology::TriangleStrip;   // the topology is a draw-time argument on Metal: same pipeline state
    ESIA_CHECK(MakePipelineKey(d, caps, k2, why) && k == k2);
    d.effect = 7;                            // no runtime HLSL
    ESIA_CHECK(!MakePipelineKey(d, caps, k, why) && !why.empty());
    d = PipelineDesc();
    d.layout = VertexLayout::None;           // UiGeometry reads vertices
    ESIA_CHECK(!MakePipelineKey(d, caps, k, why));
    d = PipelineDesc();
    d.targetFormat = Format::RGBA32_FLOAT;
    ESIA_CHECK(!MakePipelineKey(d, caps, k, why));
    d = PipelineDesc();
    d.program = ShaderProgram::TextLcd;
    d.blend = BlendMode::DualSourceLcd;
    caps.dualSourceBlend = false;
    ESIA_CHECK(!MakePipelineKey(d, caps, k, why));
}

ESIA_TEST(MetalPlanning, BlitLayouts)
{
    const BufferImageLayout a = LayoutOf(Format::R8_UNORM, 3, 5);
    ESIA_CHECK(a.rowBytes == 3 && a.bytesPerRow == 256 && a.bytes == 256 * 5);
    const BufferImageLayout b = LayoutOf(Format::RGBA16_FLOAT, 100, 2);
    ESIA_CHECK(b.rowBytes == 800 && b.bytesPerRow == 1024 && b.bytes == 2048);
    const BufferImageLayout c = LayoutOf(Format::RGBA8_UNORM, 64, 1);
    ESIA_CHECK(c.rowBytes == 256 && c.bytesPerRow == 256);
    ESIA_CHECK(LayoutOf(Format::RGBA8_UNORM, 0, 4).bytes == 0);
    // every pitch is a multiple of every pixel size
    for (Format f : {Format::R8_UNORM, Format::RGBA8_UNORM, Format::RGBA16_FLOAT, Format::RGBA32_FLOAT})
        for (int w : {1, 7, 63, 300})
            ESIA_CHECK(LayoutOf(f, w, 1).bytesPerRow % (std::size_t)BytesPerPixel(f) == 0);

    const std::uint8_t src[] = {1, 2, 3, 9, 4, 5, 6, 9};
    std::uint8_t dst[12] = {};
    CopyRows(dst, 6, src, 4, 3, 2);
    const std::uint8_t want[12] = {1, 2, 3, 0, 0, 0, 4, 5, 6, 0, 0, 0};
    ESIA_CHECK(std::memcmp(dst, want, sizeof(want)) == 0);
}

ESIA_TEST(MetalPlanning, ReadbackConversion)
{
    std::uint8_t out[8];
    // RGBA8 and sRGB: the stored bits
    const std::uint8_t rgba[] = {10, 20, 30, 40, 50, 60, 70, 80};
    ESIA_CHECK(ConvertToRgba8(Format::RGBA8_SRGB, rgba, 8, 2, 1, out) && std::memcmp(out, rgba, 8) == 0);
    // BGRA swizzled
    const std::uint8_t bgra[] = {30, 20, 10, 40};
    ESIA_CHECK(ConvertToRgba8(Format::BGRA8_UNORM, bgra, 4, 1, 1, out) && out[0] == 10 && out[1] == 20 && out[2] == 30 && out[3] == 40);
    // RGB10A2: R in the low bits
    const std::uint32_t packed = 1023u | (512u << 10) | (0u << 20) | (3u << 30);
    ESIA_CHECK(ConvertToRgba8(Format::RGB10A2_UNORM, &packed, 4, 1, 1, out) && out[0] == 255 && out[1] == 128 && out[2] == 0 && out[3] == 255);
    // half floats: 1.0, 0.5, 0, 2.0 (clamped)
    const std::uint16_t half[] = {0x3C00, 0x3800, 0x0000, 0x4000};
    ESIA_CHECK(ConvertToRgba8(Format::RGBA16_FLOAT, half, 8, 1, 1, out) && out[0] == 255 && out[1] == 128 && out[2] == 0 && out[3] == 255);
    const float f32[] = {-1.0f, 0.25f, 1.5f, 1.0f};
    ESIA_CHECK(ConvertToRgba8(Format::RGBA32_FLOAT, f32, 16, 1, 1, out) && out[0] == 0 && out[1] == 64 && out[2] == 255 && out[3] == 255);
    const std::uint8_t r8[] = {77, 0, 0, 0, 99};
    ESIA_CHECK(ConvertToRgba8(Format::R8_UNORM, r8, 4, 1, 2, out) && out[0] == 77 && out[3] == 255 && out[4] == 99 && out[5] == 0);
    ESIA_CHECK(!ConvertToRgba8(Format::Unknown, rgba, 4, 1, 1, out));

    ESIA_CHECK_NEAR(HalfToFloat(0x3C00), 1.0, 0);
    ESIA_CHECK_NEAR(HalfToFloat(0xC000), -2.0, 0);
    ESIA_CHECK_NEAR(HalfToFloat(0x7BFF), 65504.0, 0);
    ESIA_CHECK_NEAR(HalfToFloat(0x0001), 5.9604644775390625e-8, 1e-15);
}

ESIA_TEST(MetalPlanning, CopyPlans)
{
    // same format: a plain blit
    CopyPlan p = PlanCopy({Format::RGBA8_UNORM, 1, true, true}, {Format::RGBA8_UNORM, 1, true, true});
    ESIA_CHECK(p.valid && !p.resolve && p.view == CopyView::None);
    // the backdrop copy of an sRGB target: the target read through a UNORM view
    p = PlanCopy({Format::BGRA8_SRGB, 1, true, true}, {Format::BGRA8_UNORM, 1, true, true});
    ESIA_CHECK(p.valid && p.view == CopyView::Source && p.viewFormat == Format::BGRA8_UNORM);
    // a host sRGB target without PixelFormatView: the backend's copy is aliased as sRGB instead
    p = PlanCopy({Format::BGRA8_SRGB, 1, false, true}, {Format::BGRA8_UNORM, 1, true, true});
    ESIA_CHECK(p.valid && p.view == CopyView::Destination && p.viewFormat == Format::BGRA8_SRGB);
    p = PlanCopy({Format::BGRA8_SRGB, 1, false, true}, {Format::BGRA8_UNORM, 1, false, true});
    ESIA_CHECK(!p.valid);
    // multisampled: resolved into the backend's scratch texture (viewable), then blitted
    p = PlanCopy({Format::RGBA8_SRGB, 4, false, true}, {Format::RGBA8_UNORM, 1, false, true});
    ESIA_CHECK(p.valid && p.resolve && p.view == CopyView::Source && p.viewFormat == Format::RGBA8_UNORM);
    p = PlanCopy({Format::RGBA8_UNORM, 4, false, true}, {Format::RGBA8_UNORM, 1, false, true});
    ESIA_CHECK(p.valid && p.resolve && p.view == CopyView::None);
    // what CopyTexture never does
    ESIA_CHECK(!PlanCopy({Format::RGBA8_UNORM, 1, true, true}, {Format::RGBA16_FLOAT, 1, true, true}).valid);
    ESIA_CHECK(!PlanCopy({Format::RGBA8_UNORM, 1, true, true}, {Format::RGBA8_SRGB, 1, true, true}).valid);   // dst must be src or RawFormat(src)
    ESIA_CHECK(!PlanCopy({Format::RGBA8_UNORM, 1, true, true}, {Format::RGBA8_UNORM, 4, true, true}).valid);
    ESIA_CHECK(!PlanCopy({Format::BGRA8_UNORM, 1, true, false}, {Format::BGRA8_UNORM, 1, true, true}).valid);   // framebufferOnly

    ESIA_CHECK(RectInside(IRect{0, 0, 4, 4}, 4, 4));
    ESIA_CHECK(!RectInside(IRect{0, 0, 5, 4}, 4, 4) && !RectInside(IRect{2, 2, 2, 3}, 4, 4) && !RectInside(IRect{-1, 0, 1, 1}, 4, 4));
}
