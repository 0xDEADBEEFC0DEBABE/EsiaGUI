// Metal backend: MetalDevice (the whole rhi::Device logic) on the fake, rule-enforcing Gpu - the conformance
// scenes through the real renderer, frames in flight, uploads / copies / resolves / readback executed on the
// fake's CPU copies, host textures, host command buffers, timestamps.
// UNVERIFIED: needs macOS - passes on Linux and under Wine against the fake Gpu, never against Metal.
#include "esia/render/renderer.hpp"
#include "esia/rhi/null_device.hpp"
#include "esia_test.hpp"
#include "fake_metal_gpu.hpp"
#include "metal_device_core.hpp"
#include "scenes.hpp"
#include <cstdio>
#include <fstream>
#include <string>

using namespace esia;
using namespace esia::rhi;
using namespace esia::rhi::metal;
using namespace esia::rhi::metal::test;

namespace
{
    struct Rig
    {
        std::shared_ptr<FakeMetalGpu::Shared> fake;
        FakeMetalGpu* gpu = nullptr;   // owned by the device
        std::unique_ptr<MetalDevice> dev;
        Texture target;
    };

    Rig MakeRig(const HeadlessDesc& hd, FakeMetalGpu::Options o = FakeMetalGpu::Options(), DeviceOptions d = DeviceOptions())
    {
        Rig r;
        auto gpu = std::make_unique<FakeMetalGpu>(o);
        r.fake = gpu->State();
        r.gpu = gpu.get();
        std::string error;
        HeadlessDevice h = CreateHeadlessOn(std::move(gpu), hd, d, error);
        r.dev.reset(static_cast<MetalDevice*>(h.device.release()));
        r.target = h.target;
        return r;
    }

    // Both sides' complaints (the device's contract checks and the fake's Metal rules), printed for the log.
    bool Clean(const Rig& r, const char* what)
    {
        bool clean = true;
        if (r.dev)
            for (const std::string& e : r.dev->Errors())
            {
                std::printf("  %s: device: %s\n", what, e.c_str());
                clean = false;
            }
        for (const std::string& e : r.fake->errors)
        {
            std::printf("  %s: metal: %s\n", what, e.c_str());
            clean = false;
        }
        return clean;
    }

    std::vector<std::string> ReadLines(const std::string& path)
    {
        std::vector<std::string> lines;
        std::ifstream in(path);
        for (std::string l; std::getline(in, l);)
            lines.push_back(l);
        return lines;
    }
}

// Every conformance scene, three frames each, through the real renderer: no contract violation, no Metal rule broken,
// readback of the right size, everything released at the end.
ESIA_TEST(MetalDevice, ConformanceScenes)
{
    for (bool timestamps : {false, true})
        for (const conformance::Scene& scene : conformance::Scenes())
        {
            HeadlessDesc hd;
            hd.width = scene.width;
            hd.height = scene.height;
            hd.format = scene.format;
            hd.samples = scene.samples;
            FakeMetalGpu::Options o;
            o.caps.timestamps = timestamps;
            Rig r = MakeRig(hd, o);
            ESIA_CHECK(r.dev && r.target);
            if (!r.dev)
                continue;
            ESIA_CHECK(r.dev->GetCaps().timestampQueries == timestamps);
            {
                conformance::SceneFrame frame;
                conformance::BuildScene(scene, frame);
                render::Renderer renderer(*r.dev);
                for (int f = 0; f < 3; ++f)
                    ESIA_CHECK(renderer.Render(frame.data, &frame.textures, r.target));
                std::vector<std::uint8_t> px;
                ESIA_CHECK(r.dev->ReadPixels(r.target, IRect{0, 0, scene.width, scene.height}, px));
                ESIA_CHECK(px.size() == (std::size_t)scene.width * (std::size_t)scene.height * 4);
                const render::RenderStats& st = renderer.Stats();
                if (scene.samples > 1 && st.backdropCaptures > 0)
                    ESIA_CHECK(r.fake->stats.resolves > 0);
                if (timestamps)
                    ESIA_CHECK(st.gpu.valid && st.gpu.totalMs > 0.0f);   // frames 1 and 2 completed by frame 3
                r.dev->DestroyTexture(r.target);
            }
            ESIA_CHECK(Clean(r, scene.name));
            r.dev.reset();
            ESIA_CHECK(r.fake->stats.liveTextures == 0 && r.fake->stats.liveBuffers == 0);
            ESIA_CHECK(Clean(r, scene.name));
        }
}

// With Metal's caps (timestamps off) the renderer sends exactly the command streams of the null goldens: the
// renderer takes no Metal-specific path, and the null device finds no contract violation (checklist item 4).
ESIA_TEST(MetalDevice, NullGoldensWithMetalCaps)
{
    HeadlessDesc hd;
    FakeMetalGpu::Options o;
    o.caps.timestamps = false;
    Rig r = MakeRig(hd, o);
    Caps caps = r.dev->GetCaps();
    r.dev->DestroyTexture(r.target);
    for (const conformance::Scene& scene : conformance::Scenes())
    {
        NullOptions no;
        no.caps = caps;
        NullDevice null(no);
        const Texture t = null.CreateHostTarget(scene.width, scene.height, scene.format, true, scene.samples);
        {
            conformance::SceneFrame frame;
            conformance::BuildScene(scene, frame);
            render::Renderer renderer(null);
            ESIA_CHECK(renderer.Render(frame.data, &frame.textures, t));
        }
        ESIA_CHECK(null.Errors().empty());
        const std::vector<std::string> golden = ReadLines(std::string(ESIA_METAL_NULL_GOLDEN_DIR) + "/" + scene.name + ".log");
        ESIA_CHECK(!golden.empty());
        // the golden stops where the conformance harness stopped recording: after Render
        const std::vector<std::string>& log = null.Log();
        bool same = log.size() >= golden.size();
        for (std::size_t i = 0; same && i < golden.size(); ++i)
            same = log[i] == golden[i];
        if (!same)
            std::printf("  %s: the stream differs from golden/null\n", scene.name);
        ESIA_CHECK(same);
    }
    ESIA_CHECK(Clean(r, "caps"));
}

ESIA_TEST(MetalDevice, UploadsAndReadback)
{
    HeadlessDesc hd;
    hd.width = hd.height = 8;
    Rig r = MakeRig(hd);
    // created with data outside a frame: staged into the next command buffer (here: the readback's)
    std::vector<std::uint8_t> img(4 * 3 * 4);
    for (std::size_t i = 0; i < img.size(); ++i)
        img[i] = (std::uint8_t)i;
    TextureDesc d;
    d.width = 4;
    d.height = 3;
    d.usage = TextureUsage_Sampled | TextureUsage_CopyDst | TextureUsage_CopySrc;
    const Texture t = r.dev->CreateTexture(d, img.data(), 0);
    std::vector<std::uint8_t> px;
    ESIA_CHECK(r.dev->ReadPixels(t, IRect{0, 0, 4, 3}, px) && px == img);
    // a region update with a row pitch, then a sub-rect readback
    const std::uint8_t patch[] = {200, 201, 202, 203, 204, 205, 206, 207, 99, 99, 210, 211, 212, 213, 214, 215, 216, 217};
    r.dev->UpdateTexture(t, IRect{1, 1, 3, 2}, patch, 0);
    r.dev->UpdateTexture(t, IRect{2, 2, 4, 3}, patch + 10, 0);
    ESIA_CHECK(r.dev->ReadPixels(t, IRect{1, 1, 4, 3}, px) && px.size() == 3 * 2 * 4);
    ESIA_CHECK(px[0] == 200 && px[7] == 207 && px[8] == img[(1 * 4 + 3) * 4] && px[12 + 4] == 210 && px[12 + 11] == 217);
    // R8 and BGRA read back as RGBA8
    d.format = Format::R8_UNORM;
    d.width = 2;
    d.height = 1;
    const std::uint8_t r8[] = {10, 20};
    const Texture g = r.dev->CreateTexture(d, r8, 0);
    ESIA_CHECK(r.dev->ReadPixels(g, IRect{0, 0, 2, 1}, px) && px[0] == 10 && px[3] == 255 && px[4] == 20);
    d.format = Format::BGRA8_UNORM;
    const std::uint8_t bgra[] = {1, 2, 3, 4, 5, 6, 7, 8};
    const Texture b = r.dev->CreateTexture(d, bgra, 0);
    ESIA_CHECK(r.dev->ReadPixels(b, IRect{0, 0, 2, 1}, px) && px[0] == 3 && px[2] == 1 && px[4] == 7 && px[7] == 8);
    // updates inside a frame are staged in the frame's command buffer; after the first pass they are refused
    ESIA_CHECK(r.dev->BeginFrame(FrameDesc()));
    r.dev->UpdateTexture(g, IRect{0, 0, 1, 1}, r8 + 1, 0);
    r.dev->BeginPass(PassDesc{r.target});
    r.dev->EndPass();
    ESIA_CHECK(r.dev->Errors().empty());
    r.dev->UpdateTexture(g, IRect{1, 0, 2, 1}, r8, 0);
    ESIA_CHECK(r.dev->Errors().size() == 1);
    r.dev->EndFrame();
    ESIA_CHECK(r.dev->ReadPixels(g, IRect{0, 0, 2, 1}, px) && px[0] == 20 && px[4] == 20);
    for (Texture x : {t, g, b, r.target})
        r.dev->DestroyTexture(x);
    ESIA_CHECK(r.fake->errors.empty());
}

ESIA_TEST(MetalDevice, CopiesViewsAndResolves)
{
    // an sRGB target the backend created: the backdrop copy reads it through a UNORM view
    HeadlessDesc hd;
    hd.width = hd.height = 4;
    hd.format = Format::RGBA8_SRGB;
    Rig r = MakeRig(hd);
    TextureDesc cd;
    cd.width = cd.height = 4;
    cd.format = RawFormat(hd.format);
    cd.usage = TextureUsage_Sampled | TextureUsage_CopyDst;
    const Texture copy = r.dev->CreateTexture(cd, nullptr, 0);
    ESIA_CHECK(r.dev->BeginFrame(FrameDesc()));
    PassDesc clear{r.target, LoadOp::Clear};
    r.dev->BeginPass(clear);
    r.dev->EndPass();
    r.dev->CopyTexture(copy, 1, 1, r.target, IRect{0, 0, 2, 2});
    r.dev->EndFrame();
    ESIA_CHECK(r.fake->stats.copies == 1 && r.fake->stats.views == 1);   // the target's raw view serves sampling and the copy
    // a host sRGB drawable without PixelFormatView: the backend's copy is aliased as sRGB instead
    HostTextureInfo host;
    host.width = host.height = 4;
    host.pixelFormat = mtl::PixelFormatBGRA8Unorm_sRGB;
    host.usage = mtl::TextureUsageRenderTarget;
    void* native = r.gpu->MakeHostTexture(host);
    const Texture wrapped = r.dev->WrapTexture(native);
    ESIA_CHECK(wrapped && r.dev->WrapTexture(native) == wrapped);   // cached by the native pointer
    ESIA_CHECK(r.dev->GetTextureDesc(wrapped).usage == (TextureUsage_RenderTarget | TextureUsage_CopySrc | TextureUsage_CopyDst));
    cd.format = Format::BGRA8_UNORM;
    const Texture copy2 = r.dev->CreateTexture(cd, nullptr, 0);
    ESIA_CHECK(r.dev->BeginFrame(FrameDesc()));
    r.dev->CopyTexture(copy2, 0, 0, wrapped, IRect{0, 0, 4, 4});
    r.dev->EndFrame();
    ESIA_CHECK(r.fake->stats.copies == 2);
    // CAMetalLayer's default (framebufferOnly): no copy possible, reported, nothing recorded
    host.pixelFormat = mtl::PixelFormatBGRA8Unorm;
    host.framebufferOnly = true;
    const Texture fbo = r.dev->WrapTexture(r.gpu->MakeHostTexture(host));
    ESIA_CHECK(r.dev->GetTextureDesc(fbo).usage == TextureUsage_RenderTarget);
    ESIA_CHECK(r.dev->BeginFrame(FrameDesc()));
    r.dev->CopyTexture(copy2, 0, 0, fbo, IRect{0, 0, 4, 4});
    r.dev->EndFrame();
    ESIA_CHECK(r.fake->stats.copies == 2 && r.dev->Errors().size() == 1);
    for (Texture x : {copy, copy2, wrapped, fbo, r.target})
        r.dev->DestroyTexture(x);
    ESIA_CHECK(r.fake->errors.empty());

    // multisampled: resolved by a render pass, then blitted; readback resolves too
    hd.format = Format::RGBA8_UNORM;
    hd.samples = 4;
    Rig m = MakeRig(hd);
    ESIA_CHECK(m.dev && !(m.dev->GetTextureDesc(m.target).usage & TextureUsage_Sampled));
    cd.format = Format::RGBA8_UNORM;
    const Texture mc = m.dev->CreateTexture(cd, nullptr, 0);
    ESIA_CHECK(m.dev->BeginFrame(FrameDesc()));
    PassDesc red{m.target, LoadOp::Clear, {1, 0, 0, 1}};
    m.dev->BeginPass(red);
    m.dev->EndPass();
    m.dev->CopyTexture(mc, 0, 0, m.target, IRect{0, 0, 4, 4});
    m.dev->EndFrame();
    std::vector<std::uint8_t> px;
    ESIA_CHECK(m.dev->ReadPixels(m.target, IRect{0, 0, 4, 4}, px) && px[0] == 255 && px[1] == 0 && px[3] == 255);
    ESIA_CHECK(m.fake->stats.resolves == 2 && m.fake->stats.copies == 1);
    ESIA_CHECK(m.dev->ReadPixels(mc, IRect{3, 3, 4, 4}, px) && px[0] == 255);
    // no 16x MSAA on this GPU: the headless creator says so (the suite SKIPs)
    hd.samples = 16;
    std::string error;
    ESIA_CHECK(!CreateHeadlessOn(std::make_unique<FakeMetalGpu>(), hd, DeviceOptions(), error).device && !error.empty());
    m.dev->DestroyTexture(mc);
    m.dev->DestroyTexture(m.target);
    ESIA_CHECK(Clean(m, "msaa"));
}

// Frames recorded into host command buffers the host commits late: buffer versions, deferred releases and uploads
// must never touch what an uncommitted or running command buffer uses.
ESIA_TEST(MetalDevice, HostCommandBuffersInFlight)
{
    HeadlessDesc hd;
    FakeMetalGpu::Options o;
    Rig r = MakeRig(hd, o);
    int hostCb = 0;
    FrameDesc fd;
    fd.nativeContext = &hostCb;
    const conformance::Scene* scene = conformance::FindScene("glass");
    ESIA_CHECK(scene != nullptr);
    {
        conformance::SceneFrame frame;
        conformance::BuildScene(*scene, frame);
        render::RenderParams params;
        params.frame = fd;
        render::Renderer renderer(*r.dev);
        // four device frames before the host commits anything (a host rendering several targets per frame)
        for (int f = 0; f < 4; ++f)
            ESIA_CHECK(renderer.Render(frame.data, &frame.textures, r.target, params));
        // the renderer's size-dependent surfaces are released while those frames still run, and the next frame
        // (whose BeginFrame collects finished releases) comes before the host committed anything
        renderer.ReleaseSurfaces();
        ESIA_CHECK(renderer.Render(frame.data, &frame.textures, r.target, params));
        std::vector<std::uint8_t> px;
        ESIA_CHECK(!r.dev->ReadPixels(r.target, IRect{0, 0, 8, 8}, px));   // cannot wait for uncommitted work
        ESIA_CHECK(r.dev->Errors().size() == 1);
        for (std::uint64_t s = 1; s <= 5; ++s)
            r.gpu->CommitHost(s);
        ESIA_CHECK(r.dev->ReadPixels(r.target, IRect{0, 0, 8, 8}, px));
        ESIA_CHECK(renderer.Render(frame.data, &frame.textures, r.target, params));
        r.gpu->CommitHost(7);   // serial 6 was the readback's own command buffer
        r.gpu->CompleteAll();
    }
    r.dev->DestroyTexture(r.target);
    ESIA_CHECK(r.fake->errors.empty());
    r.dev.reset();
    ESIA_CHECK(r.fake->errors.empty() && r.fake->stats.liveTextures == 0 && r.fake->stats.liveBuffers == 0);
}

ESIA_TEST(MetalDevice, BindingsStartEmpty)
{
    HeadlessDesc hd;
    hd.width = hd.height = 16;
    Rig r = MakeRig(hd);
    PipelineDesc pd;
    pd.program = ShaderProgram::Clear;
    pd.layout = VertexLayout::None;
    pd.blend = BlendMode::Opaque;
    const Pipeline clear = r.dev->CreatePipeline(pd);
    ESIA_CHECK(clear);
    // same state object for a second handle, and user effects are refused (no runtime HLSL on Metal)
    ESIA_CHECK(r.dev->CreatePipeline(pd));
    pd.program = ShaderProgram::Fx;
    pd.effect = 3;
    ESIA_CHECK(!r.dev->CreatePipeline(pd));
    ESIA_CHECK(r.fake->stats.pipelines == 1);
    float frame[48] = {};
    ESIA_CHECK(r.dev->BeginFrame(FrameDesc()));
    r.dev->BeginPass(PassDesc{r.target});
    r.dev->SetPipeline(clear);
    r.dev->SetConstants(ConstantSlot::Frame, frame, sizeof(frame));
    r.dev->SetScissor(IRect{2, 2, 6, 6});
    r.dev->Draw(3, 0);
    ESIA_CHECK(r.fake->stats.draws == 1);
    // a host callback may change anything: the backend forgets its bindings, the renderer binds again
    ESIA_CHECK(r.dev->NativeRenderState() == r.gpu);
    r.dev->Draw(3, 0);   // nothing bound: skipped, reported
    ESIA_CHECK(r.fake->stats.draws == 1 && r.dev->Errors().size() == 1);
    r.dev->SetPipeline(clear);
    r.dev->SetConstants(ConstantSlot::Frame, frame, sizeof(frame));
    r.dev->Draw(3, 0);
    // an empty scissor: allowed by the RHI when no draw follows; Metal never sees it
    r.dev->SetScissor(IRect{4, 4, 4, 9});
    r.dev->EndPass();
    r.dev->EndFrame();
    ESIA_CHECK(r.fake->stats.draws == 2);
    r.dev->DestroyTexture(r.target);
    ESIA_CHECK(r.fake->errors.empty());
}
