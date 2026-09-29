// Vulkan backend on the machine's driver (Mesa lavapipe in CI): the shader library's SPIR-V turned into pipelines for
// every program, clears / uploads / copies / resolves read back per format, with the validation layer watching.
// Without a Vulkan driver the tests only say so (the conformance suite skips the backend the same way).
#include "esia/rhi/vulkan.hpp"
#include "esia_test.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

using namespace esia::rhi;

namespace
{
    struct Headless
    {
        HeadlessDevice h;
        std::string error;
        Device* operator->() const { return h.device.get(); }
    };

    Headless Make(Format format = Format::RGBA8_UNORM, int samples = 1, bool dynamicRendering = true, int w = 64, int h = 48)
    {
        HeadlessDesc d;
        d.width = w;
        d.height = h;
        d.format = format;
        d.samples = samples;
        vulkan::HeadlessOptions o;
        o.dynamicRendering = dynamicRendering;
        Headless r;
        r.h = vulkan::CreateHeadless(d, o, r.error);
        return r;
    }

    bool Unavailable(const Headless& d)
    {
        if (d.h.device)
            return false;
        std::printf("  (no Vulkan device: %s)\n", d.error.c_str());
        return true;
    }

    void ClearPass(Device& dev, Texture target, const float color[4])
    {
        ESIA_CHECK(dev.BeginFrame(FrameDesc{}));
        PassDesc p;
        p.target = target;
        p.load = LoadOp::Clear;
        for (int i = 0; i < 4; ++i)
            p.clearColor[i] = color[i];
        dev.BeginPass(p);
        dev.EndPass();
        dev.EndFrame();
    }

    int Encode(float c, bool srgb)
    {
        if (srgb)
            c = c <= 0.0031308f ? 12.92f * c : 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f;
        return (int)std::lround(c * 255.0f);
    }

    PipelineDesc ProgramDesc(ShaderProgram p, Format format, int samples)
    {
        PipelineDesc d;
        d.program = p;
        const bool ui = p == ShaderProgram::UiGeometry || p == ShaderProgram::TextGray || p == ShaderProgram::TextLcd || p == ShaderProgram::TextLcdGray;
        d.layout = ui ? VertexLayout::UiVertex : VertexLayout::None;
        d.topology = p == ShaderProgram::Fx ? Topology::TriangleStrip : Topology::TriangleList;
        d.blend = p == ShaderProgram::TextLcd                                       ? BlendMode::DualSourceLcd
                  : p == ShaderProgram::Fx || p == ShaderProgram::LayerComposite    ? BlendMode::Premultiplied
                  : p == ShaderProgram::Downsample || p == ShaderProgram::Clear     ? BlendMode::Opaque
                                                                                    : BlendMode::Straight;
        d.targetFormat = format;
        d.samples = samples;
        return d;
    }
}

// The core's SPIR-V had never run on a driver: every program must become a pipeline, on both render paths, for
// the formats and sample counts the renderer uses.
ESIA_TEST(Vulkan, EveryProgramBuilds)
{
    for (bool dynamicRendering : {true, false})
    {
        Headless d = Make(Format::RGBA8_UNORM, 1, dynamicRendering);
        if (Unavailable(d))
            return;
        ESIA_CHECK(vulkan::UsesDynamicRendering(*d.h.device) == dynamicRendering || !dynamicRendering);
        const Caps& caps = d->GetCaps();
        ESIA_CHECK(caps.readback && caps.clipSpaceYDown && caps.fxStorage == FxStorage::Buffer && !caps.framebufferOriginBottomLeft);
        for (Format f : {Format::RGBA8_UNORM, Format::RGBA8_SRGB, Format::BGRA8_UNORM, Format::BGRA8_SRGB, Format::RGB10A2_UNORM, Format::RGBA16_FLOAT})
            for (int samples : {1, 4})
                for (int p = 0; p < (int)ShaderProgram::Count; ++p)
                {
                    const PipelineDesc pd = ProgramDesc((ShaderProgram)p, f, samples);
                    const Pipeline pipe = d->CreatePipeline(pd);
                    if (pd.blend == BlendMode::DualSourceLcd && !caps.dualSourceBlend)
                        ESIA_CHECK(!pipe);
                    else
                    {
                        if (!pipe)
                            std::printf("  %s for %s x%d failed\n", ShaderProgramName(pd.program), FormatName(f), samples);
                        ESIA_CHECK((bool)pipe);
                    }
                    if (pipe)
                        d->DestroyPipeline(pipe);
                }
        // what the backend cannot do, it refuses (the renderer falls back)
        PipelineDesc effect = ProgramDesc(ShaderProgram::Fx, Format::RGBA8_UNORM, 1);
        effect.effect = 7;
        effect.effectSource = "float4 WgtEffect(WgtFx fx) { return 1; }";
        ESIA_CHECK(!d->CreatePipeline(effect));
        ESIA_CHECK(vulkan::ValidationMessages(*d.h.device) == 0);
    }
}

// Clear values reach every render-target format and come back through ReadPixels: sRGB targets store encoded
// values (returned as stored), float targets are converted.
ESIA_TEST(Vulkan, ClearAndReadbackPerFormat)
{
    const float color[4] = {0.25f, 0.5f, 0.75f, 1.0f};
    for (Format f : {Format::RGBA8_UNORM, Format::RGBA8_SRGB, Format::BGRA8_UNORM, Format::BGRA8_SRGB, Format::RGB10A2_UNORM, Format::RGBA16_FLOAT,
                     Format::RGBA32_FLOAT})
    {
        Headless d = Make(f);
        if (Unavailable(d))
            return;
        ClearPass(*d.h.device, d.h.target, color);
        std::vector<std::uint8_t> px;
        ESIA_CHECK(d->ReadPixels(d.h.target, IRect{3, 5, 13, 9}, px));
        ESIA_CHECK(px.size() == 10 * 4 * 4);
        if (px.size() != 10 * 4 * 4)
            continue;
        for (int c = 0; c < 4; ++c)
        {
            const int want = Encode(color[c], IsSrgb(f) && c < 3);
            if (std::abs(px[(std::size_t)c] - want) > 1)
                std::printf("  %s channel %d: %d, want %d\n", FormatName(f), c, px[(std::size_t)c], want);
            ESIA_CHECK(std::abs(px[(std::size_t)c] - want) <= 1);
        }
        ESIA_CHECK(vulkan::ValidationMessages(*d.h.device) == 0);
    }
}

// Uploads (create with data, partial updates with a row pitch, updates outside and inside frames) and copies between
// textures, read back byte for byte.
ESIA_TEST(Vulkan, UploadUpdateCopy)
{
    Headless d = Make();
    if (Unavailable(d))
        return;
    const int w = 16, h = 8;
    std::vector<std::uint8_t> pixels((std::size_t)w * h * 4);
    for (std::size_t i = 0; i < pixels.size(); ++i)
        pixels[i] = (std::uint8_t)(i * 7 + 3);
    TextureDesc td;
    td.width = w;
    td.height = h;
    td.usage = TextureUsage_Sampled | TextureUsage_CopySrc | TextureUsage_CopyDst;
    const Texture a = d->CreateTexture(td, pixels.data(), 0);   // outside a frame: recorded by the next one
    ESIA_CHECK((bool)a);

    // a 3 x 2 patch at (5, 4) from rows with a pitch of 20 bytes
    std::vector<std::uint8_t> patch(20 * 2, 0xEE);
    for (int y = 0; y < 2; ++y)
        for (int x = 0; x < 12; ++x)
            patch[(std::size_t)(y * 20 + x)] = (std::uint8_t)(200 + y * 12 + x);
    d->UpdateTexture(a, IRect{5, 4, 8, 6}, patch.data(), 20);
    for (int y = 0; y < 2; ++y)
        for (int x = 0; x < 12; ++x)
            pixels[(std::size_t)(((4 + y) * w + 5) * 4 + x)] = (std::uint8_t)(200 + y * 12 + x);

    std::vector<std::uint8_t> px;
    ESIA_CHECK(d->ReadPixels(a, IRect{0, 0, w, h}, px));
    ESIA_CHECK(px == pixels);

    // inside a frame, before its first pass: an update, then a copy of a region into a new texture
    const Texture b = d->CreateTexture(td, nullptr, 0);
    ESIA_CHECK(d->BeginFrame(FrameDesc{}));
    const std::uint8_t one[4] = {1, 2, 3, 4};
    d->UpdateTexture(a, IRect{0, 0, 1, 1}, one, 0);
    d->CopyTexture(b, 2, 1, a, IRect{0, 0, 6, 5});
    d->EndFrame();
    std::copy(one, one + 4, pixels.begin());
    ESIA_CHECK(d->ReadPixels(b, IRect{2, 1, 8, 6}, px));
    bool same = px.size() == 6 * 5 * 4;
    for (int y = 0; same && y < 5; ++y)
        same = std::equal(px.begin() + y * 24, px.begin() + y * 24 + 24, pixels.begin() + y * w * 4);
    ESIA_CHECK(same);
    // new textures start as zeros
    ESIA_CHECK(d->ReadPixels(b, IRect{0, 0, 1, 1}, px) && px == std::vector<std::uint8_t>(4, 0));

    // buffers: created, updated outside and inside frames, destroyed in a frame (released later)
    BufferDesc bd;
    bd.kind = BufferKind::Vertex;
    bd.size = 256;
    const Buffer vb = d->CreateBuffer(bd);
    ESIA_CHECK((bool)vb);
    d->UpdateBuffer(vb, pixels.data(), 200);
    ESIA_CHECK(d->BeginFrame(FrameDesc{}));
    d->UpdateBuffer(vb, pixels.data(), 256);
    d->DestroyBuffer(vb);
    d->DestroyTexture(b);
    d->EndFrame();
    for (int i = 0; i < 3; ++i)   // the slots come round: the deferred releases happen
    {
        ESIA_CHECK(d->BeginFrame(FrameDesc{}));
        d->EndFrame();
    }
    d->DestroyTexture(a);
    ESIA_CHECK(vulkan::ValidationMessages(*d.h.device) == 0);
}

// The backdrop copy of an sRGB target is its RawFormat: the bits are copied, never converted.
ESIA_TEST(Vulkan, SrgbRawCopy)
{
    Headless d = Make(Format::RGBA8_SRGB);
    if (Unavailable(d))
        return;
    ESIA_CHECK((d->GetTextureDesc(d.h.target).usage & TextureUsage_Sampled) != 0);   // sampled through a UNORM view
    const float color[4] = {0.2f, 0.4f, 0.6f, 0.8f};
    ClearPass(*d.h.device, d.h.target, color);
    TextureDesc td;
    td.width = 64;
    td.height = 48;
    td.format = Format::RGBA8_UNORM;
    td.usage = TextureUsage_Sampled | TextureUsage_CopyDst;
    const Texture copy = d->CreateTexture(td, nullptr, 0);
    ESIA_CHECK(d->BeginFrame(FrameDesc{}));
    d->CopyTexture(copy, 0, 0, d.h.target, IRect{0, 0, 64, 48});
    d->EndFrame();
    std::vector<std::uint8_t> a, b;
    ESIA_CHECK(d->ReadPixels(d.h.target, IRect{10, 10, 20, 20}, a));
    ESIA_CHECK(d->ReadPixels(copy, IRect{10, 10, 20, 20}, b));
    ESIA_CHECK(!a.empty() && a == b);
    ESIA_CHECK(a.size() >= 4 && a[0] == Encode(0.2f, true) && a[3] == Encode(0.8f, false));
    ESIA_CHECK(vulkan::ValidationMessages(*d.h.device) == 0);
}

// Multisampled targets: not sampleable, resolved by CopyTexture (same format, and sRGB into its raw format through
// the scratch image) and by ReadPixels.
ESIA_TEST(Vulkan, MultisampledResolve)
{
    for (Format f : {Format::RGBA8_UNORM, Format::RGBA8_SRGB})
    {
        Headless d = Make(f, 4);
        if (Unavailable(d))
            return;
        const TextureDesc target = d->GetTextureDesc(d.h.target);
        ESIA_CHECK(target.samples == 4 && !(target.usage & TextureUsage_Sampled));
        const float color[4] = {1.0f, 0.5f, 0.0f, 1.0f};
        ClearPass(*d.h.device, d.h.target, color);
        TextureDesc td;
        td.width = 64;
        td.height = 48;
        td.format = RawFormat(f);
        td.usage = TextureUsage_Sampled | TextureUsage_CopyDst;
        const Texture copy = d->CreateTexture(td, nullptr, 0);
        ESIA_CHECK(d->BeginFrame(FrameDesc{}));
        d->CopyTexture(copy, 8, 8, d.h.target, IRect{0, 0, 32, 16});
        d->EndFrame();
        std::vector<std::uint8_t> a, b;
        ESIA_CHECK(d->ReadPixels(d.h.target, IRect{0, 0, 32, 16}, a));
        ESIA_CHECK(d->ReadPixels(copy, IRect{8, 8, 40, 24}, b));
        ESIA_CHECK(!a.empty() && a == b);
        ESIA_CHECK(a.size() >= 4 && a[0] == 255 && a[1] == Encode(0.5f, IsSrgb(f)) && a[2] == 0);
        ESIA_CHECK(vulkan::ValidationMessages(*d.h.device) == 0);
    }
}

// Formats and sample counts the device cannot render are refused, not faked.
ESIA_TEST(Vulkan, RefusesWhatItCannotDo)
{
    Headless d = Make();
    if (Unavailable(d))
        return;
    TextureDesc td;
    td.width = 0;
    td.height = 4;
    ESIA_CHECK(!d->CreateTexture(td, nullptr, 0));
    td.width = 4;
    td.samples = 3;
    td.usage = TextureUsage_RenderTarget;
    ESIA_CHECK(!d->CreateTexture(td, nullptr, 0));
    td.samples = 1;
    td.format = Format::Unknown;
    ESIA_CHECK(!d->CreateTexture(td, nullptr, 0));
    std::vector<std::uint8_t> px;
    ESIA_CHECK(!d->ReadPixels(d.h.target, IRect{0, 0, 65, 1}, px));
    ESIA_CHECK(vulkan::ValidationMessages(*d.h.device) == 0);
}
