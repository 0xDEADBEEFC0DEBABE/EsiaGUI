// The Direct3D backends built into this binary, below the renderer: texture formats and readback, region updates,
// copies (raw bits between a format and its RawFormat, multisampled resolves), clears of sRGB targets (stored
// values), scissored draws, and GPU timestamps. A backend whose headless device cannot be created here is skipped.
#include "esia/rhi/backend_registry.hpp"
#include "esia_test.hpp"
#include <chrono>
#include <cstring>
#include <string>
#include <thread>

using namespace esia;
using namespace esia::rhi;

namespace
{
    template <class F>
    void ForEachD3D(F&& f)
    {
        RegisterBuiltinBackends();
        for (const BackendInfo& b : Backends())
        {
            if (std::strncmp(b.name, "d3d", 3) != 0)
                continue;
            HeadlessDesc hd;
            hd.width = 64;
            hd.height = 32;
            std::string error;
            HeadlessDevice h = b.createHeadless(hd, error);
            if (!h.device)
            {
                std::printf("  %s skipped: %s\n", b.name, error.c_str());
                continue;
            }
            std::printf("  %s\n", b.name);
            f(*h.device, h.target);
        }
    }

    Texture Make(Device& dev, int w, int h, Format f, std::uint32_t usage, const void* data = nullptr, int samples = 1)
    {
        TextureDesc d;
        d.width = w;
        d.height = h;
        d.format = f;
        d.usage = usage;
        d.samples = samples;
        return dev.CreateTexture(d, data, 0);
    }

    // A pass that clears `t` to `c` (and draws nothing).
    void ClearPass(Device& dev, Texture t, float r, float g, float b, float a)
    {
        PassDesc p;
        p.target = t;
        p.load = LoadOp::Clear;
        p.clearColor[0] = r;
        p.clearColor[1] = g;
        p.clearColor[2] = b;
        p.clearColor[3] = a;
        dev.BeginPass(p);
        dev.EndPass();
    }

    bool Pixel(Device& dev, Texture t, int x, int y, std::uint8_t out[4])
    {
        std::vector<std::uint8_t> px;
        if (!dev.ReadPixels(t, IRect{x, y, x + 1, y + 1}, px) || px.size() != 4)
            return false;
        std::memcpy(out, px.data(), 4);
        return true;
    }

    bool Near(const std::uint8_t* p, int r, int g, int b, int a, int tol = 1)
    {
        return std::abs(p[0] - r) <= tol && std::abs(p[1] - g) <= tol && std::abs(p[2] - b) <= tol && std::abs(p[3] - a) <= tol;
    }
}

ESIA_TEST(D3DDevices, UploadAndReadback)
{
    ForEachD3D([](Device& dev, Texture) {
        const std::uint32_t use = TextureUsage_Sampled | TextureUsage_CopyDst | TextureUsage_RenderTarget | TextureUsage_CopySrc;
        // 2 x 2 of each format, rows top first: (10,20,30,40) (50,60,70,80) / (90,...)
        const std::uint8_t rgba[16] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160};
        for (Format f : {Format::RGBA8_UNORM, Format::BGRA8_UNORM, Format::RGBA8_SRGB})
        {
            const Texture t = Make(dev, 2, 2, f, use, rgba);
            ESIA_CHECK((bool)t);
            std::vector<std::uint8_t> px;
            ESIA_CHECK(dev.ReadPixels(t, IRect{0, 0, 2, 2}, px) && px.size() == 16);
            // uploads are bytes in the format's own order (B first for BGRA8); readback always returns RGBA
            if (px.size() == 16)
                ESIA_CHECK(f == Format::BGRA8_UNORM ? (px[0] == 30 && px[2] == 10 && px[15] == 160) : std::memcmp(px.data(), rgba, 16) == 0);
            dev.DestroyTexture(t);
        }
        const std::uint8_t r8[4] = {0, 64, 128, 255};
        const Texture g = Make(dev, 2, 2, Format::R8_UNORM, TextureUsage_Sampled | TextureUsage_CopyDst | TextureUsage_CopySrc, r8);
        std::uint8_t p[4];
        ESIA_CHECK(g && Pixel(dev, g, 1, 1, p) && p[0] == 255);
        const float f32[4] = {0.25f, 0.5f, 0.75f, 1.0f};
        const Texture ft = Make(dev, 1, 1, Format::RGBA32_FLOAT, TextureUsage_Sampled | TextureUsage_CopyDst | TextureUsage_CopySrc, f32);
        ESIA_CHECK(ft && Pixel(dev, ft, 0, 0, p) && Near(p, 64, 128, 191, 255));

        // UpdateTexture replaces a rectangle only
        const std::uint8_t blue[4] = {0, 0, 255, 255};
        const Texture u = Make(dev, 4, 4, Format::RGBA8_UNORM, use);
        ESIA_CHECK((bool)u);
        FrameDesc fd;
        ESIA_CHECK(dev.BeginFrame(fd));
        ClearPass(dev, u, 1, 0, 0, 1);
        dev.EndFrame();
        dev.UpdateTexture(u, IRect{2, 1, 3, 2}, blue, 0);
        ESIA_CHECK(Pixel(dev, u, 2, 1, p) && Near(p, 0, 0, 255, 255, 0));
        ESIA_CHECK(Pixel(dev, u, 1, 1, p) && Near(p, 255, 0, 0, 255, 0));
        for (Texture t : {g, ft, u})
            dev.DestroyTexture(t);
    });
}

ESIA_TEST(D3DDevices, ClearsCopiesAndResolves)
{
    ForEachD3D([](Device& dev, Texture) {
        const std::uint32_t rt = TextureUsage_RenderTarget | TextureUsage_CopySrc | TextureUsage_Sampled;
        // an sRGB target stores the clear color as given (stored values, not linear ones)
        const Texture s = Make(dev, 16, 16, Format::RGBA8_SRGB, rt);
        const Texture copy = Make(dev, 16, 16, Format::RGBA8_UNORM, TextureUsage_Sampled | TextureUsage_CopyDst | TextureUsage_CopySrc);
        const Texture half = Make(dev, 16, 16, Format::RGBA16_FLOAT, rt);
        ESIA_CHECK(s && copy && half);
        FrameDesc fd;
        ESIA_CHECK(dev.BeginFrame(fd));
        ClearPass(dev, s, 0.5f, 0.25f, 1.0f, 1.0f);
        ClearPass(dev, half, 0.5f, 0.0f, 0.25f, 0.5f);
        // copy the raw bits of a region of the sRGB target into its RawFormat copy
        dev.CopyTexture(copy, 4, 2, s, IRect{0, 0, 8, 8});
        dev.EndFrame();
        std::uint8_t p[4];
        ESIA_CHECK(Pixel(dev, s, 3, 3, p) && Near(p, 128, 64, 255, 255));
        ESIA_CHECK(Pixel(dev, copy, 5, 3, p) && Near(p, 128, 64, 255, 255));
        ESIA_CHECK(Pixel(dev, half, 0, 15, p) && Near(p, 128, 0, 64, 128));

        // a multisampled target is resolved by CopyTexture and ReadPixels
        const Texture ms = Make(dev, 16, 16, Format::RGBA8_UNORM, TextureUsage_RenderTarget | TextureUsage_CopySrc, nullptr, 4);
        if (ms)
        {
            ESIA_CHECK(!(dev.GetTextureDesc(ms).usage & TextureUsage_Sampled) && dev.GetTextureDesc(ms).samples == 4);
            ESIA_CHECK(dev.BeginFrame(fd));
            ClearPass(dev, ms, 0.0f, 1.0f, 0.0f, 1.0f);
            dev.CopyTexture(copy, 0, 0, ms, IRect{0, 0, 16, 16});
            dev.EndFrame();
            ESIA_CHECK(Pixel(dev, ms, 7, 7, p) && Near(p, 0, 255, 0, 255));
            ESIA_CHECK(Pixel(dev, copy, 15, 15, p) && Near(p, 0, 255, 0, 255));
            dev.DestroyTexture(ms);
        }
        for (Texture t : {s, copy, half})
            dev.DestroyTexture(t);
    });
}

ESIA_TEST(D3DDevices, ScissoredDrawAndTimestamps)
{
    ForEachD3D([](Device& dev, Texture target) {
        PipelineDesc pd;
        pd.program = ShaderProgram::Clear;
        pd.layout = VertexLayout::None;
        pd.blend = BlendMode::Opaque;
        pd.targetFormat = dev.GetTextureDesc(target).format;
        const Pipeline clear = dev.CreatePipeline(pd);
        ESIA_CHECK((bool)clear);
        const float frame[48] = {};
        const bool timed = dev.GetCaps().timestampQueries;
        GpuProfile profile;
        bool gotProfile = false;
        // timestamps arrive a few frames later: keep rendering (with pauses: the GPU may lag the loop) until they do
        for (int i = 0; i < 300 && !gotProfile; ++i)
        {
            if (i >= 4)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            FrameDesc fd;
            ESIA_CHECK(dev.BeginFrame(fd));
            PassDesc p;
            p.target = target;
            p.load = LoadOp::Clear;
            p.clearColor[0] = p.clearColor[3] = 1.0f;
            dev.BeginProfile(ProfileCategory::Layer);
            dev.BeginPass(p);
            dev.SetPipeline(clear);
            dev.SetScissor(IRect{8, 4, 16, 12});
            dev.SetConstants(ConstantSlot::Frame, frame, sizeof(frame));
            dev.Draw(3, 0);
            dev.EndPass();
            dev.EndProfile();
            dev.EndFrame();
            gotProfile = dev.ReadProfile(profile);
        }
        std::uint8_t px[4];
        ESIA_CHECK(Pixel(dev, target, 8, 4, px) && Near(px, 0, 0, 0, 0, 0));      // inside the scissor: cleared by the draw
        ESIA_CHECK(Pixel(dev, target, 16, 4, px) && Near(px, 255, 0, 0, 255, 0));  // outside: the pass clear
        ESIA_CHECK(Pixel(dev, target, 8, 12, px) && Near(px, 255, 0, 0, 255, 0));
        if (timed)
        {
            ESIA_CHECK(gotProfile && profile.valid && profile.totalMs >= 0.0f);
            ESIA_CHECK(profile.categoryMs[(int)ProfileCategory::Layer] <= profile.totalMs + 1e-3f);
        }
        dev.DestroyPipeline(clear);
    });
}

ESIA_TEST(D3DDevices, UserEffectPipeline)
{
    ForEachD3D([](Device& dev, Texture target) {
        ESIA_CHECK(dev.GetCaps().runtimeEffects);
        PipelineDesc pd;
        pd.program = ShaderProgram::Fx;
        pd.layout = VertexLayout::None;
        pd.topology = Topology::TriangleStrip;
        pd.blend = BlendMode::Premultiplied;
        pd.targetFormat = dev.GetTextureDesc(target).format;
        pd.effect = 1;
        pd.effectSource = "float4 WgtEffect(WgtFx fx) { return float4(fx.uv, WgtTexture(fx.uv).b, 1.0) * fx.coverage; }";
        // D3D9 builds FX variants: fill + custom effect
        pd.fxFeatures = dev.GetCaps().fxFeatureVariants ? 0x1001u : 0u;
        // compiled on a worker: {} until it is ready (the renderer asks again on a later frame)
        Pipeline p;
        for (int i = 0; i < 1200 && !p; ++i)
        {
            p = dev.CreatePipeline(pd);
            if (!p)
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        ESIA_CHECK((bool)p);
        if (p)
            dev.DestroyPipeline(p);
        // a source that does not compile never yields a pipeline, and never blocks
        pd.effect = 2;
        pd.effectSource = "float4 WgtEffect(WgtFx fx) { return undefined_thing; }";
        for (int i = 0; i < 100; ++i)
        {
            ESIA_CHECK(!dev.CreatePipeline(pd));
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    });
}
