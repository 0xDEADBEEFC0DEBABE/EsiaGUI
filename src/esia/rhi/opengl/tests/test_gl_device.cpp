// Esia OpenGL backend - device tests on the machine's driver (Mesa llvmpipe through EGL in CI). Every test runs
// on both backends (OpenGL 3.3 core, GLES 3.0) and is skipped when EGL offers no such context.
#include "esia/render/painter.hpp"
#include "esia/render/renderer.hpp"
#include "esia/rhi/null_device.hpp"
#include "esia_test.hpp"
#include "gl_api.hpp"
#include "gl_headless.hpp"
#include "scenes.hpp"
#include <cstdio>
#include <cstring>

using namespace esia;
using namespace esia::rhi::opengl;

namespace
{
    // Desc::es of both backends
    const bool kApis[] = {false, true};

    const char* ApiName(bool es) { return es ? "gles" : "opengl"; }

    rhi::HeadlessDevice Headless(bool es, int w, int h, rhi::Format format = rhi::Format::RGBA8_UNORM, int samples = 1, bool coreOnly = false,
                                 bool sampleable = true)
    {
        Desc desc;
        desc.es = es;
        desc.coreOnly = coreOnly;
        desc.debug = true;   // count GL errors in release builds too
        rhi::HeadlessDesc hd;
        hd.width = w;
        hd.height = h;
        hd.format = format;
        hd.samples = samples;
        hd.sampleable = sampleable;
        std::string error;
        rhi::HeadlessDevice d = CreateHeadlessDevice(desc, hd, error);
        if (!d.device)
            std::printf("  %s: skipped (%s)\n", ApiName(es), error.c_str());
        return d;
    }

    // GL entry points of the current headless context, for tests that act as the host application
    GlApi LoadGl()
    {
        GlApi gl;
        const char* missing = nullptr;
        ESIA_CHECK(gl.Load(HeadlessGetProcAddress(), missing));
        return gl;
    }

    std::vector<std::uint8_t> Read(rhi::Device& dev, rhi::Texture t, const rhi::IRect& r)
    {
        std::vector<std::uint8_t> px;
        ESIA_CHECK(dev.ReadPixels(t, r, px));
        ESIA_CHECK(px.size() == (std::size_t)r.Width() * (std::size_t)r.Height() * 4);
        return px;
    }

    std::vector<std::uint8_t> RenderScene(rhi::Device& dev, rhi::Texture target, const conformance::Scene& scene, int frames = 1)
    {
        render::Renderer renderer(dev);
        for (int i = 0; i < frames; ++i)
        {
            conformance::SceneFrame f;
            conformance::BuildScene(scene, f);
            ESIA_CHECK(renderer.Render(f.data, &f.textures, target, conformance::RenderParamsOf(scene)));
        }
        return Read(dev, target, rhi::IRect{0, 0, scene.width, scene.height});
    }

    int MaxDelta(const std::vector<std::uint8_t>& a, const std::vector<std::uint8_t>& b)
    {
        if (a.size() != b.size())
            return 256;
        int m = 0;
        for (std::size_t i = 0; i < a.size(); ++i)
            m = std::max(m, std::abs((int)a[i] - (int)b[i]));
        return m;
    }

    rhi::TextureDesc TexDesc(int w, int h, rhi::Format f, std::uint32_t usage, int samples = 1)
    {
        rhi::TextureDesc d;
        d.width = w;
        d.height = h;
        d.format = f;
        d.usage = usage;
        d.samples = samples;
        return d;
    }

    // The frame constants the fullscreen programs read (gXform.y < 0: the renderer's y-up projection)
    void FrameConstants(rhi::Device& dev)
    {
        float c[48] = {};
        c[1] = -1.0f;
        dev.SetConstants(rhi::ConstantSlot::Frame, c, sizeof(c));
    }
}

// Every conformance scene renders and reads back without a single GL error (KHR_debug), two frames each so the
// second one runs on the device's caches - with everything the driver offers and on the core minimum.
ESIA_TEST(GlDevice, ConformanceScenesWithoutGlErrors)
{
    for (bool coreOnly : {false, true})
        for (bool es : kApis)
            for (const conformance::Scene& scene : conformance::Scenes())
            {
                rhi::HeadlessDevice h = Headless(es, scene.width, scene.height, scene.format, scene.samples, coreOnly, scene.sampleable);
                if (!h.device)
                    continue;
                RenderScene(*h.device, h.target, scene, 2);
                const std::uint32_t errors = ErrorCount(*h.device);
                if (errors)
                    std::printf("  %s%s %s: %u GL errors\n", ApiName(es), coreOnly ? " (core only)" : "", scene.name, errors);
                ESIA_CHECK(errors == 0);
            }
}

// The error count is real: a bad call through the host's context is reported and counted.
ESIA_TEST(GlDevice, DebugOutputCountsErrors)
{
    for (bool es : kApis)
    {
        rhi::HeadlessDevice h = Headless(es, 8, 8);
        if (!h.device)
            continue;
        const GlApi gl = LoadGl();
        std::printf("  %s: the GL_INVALID_ENUM reported next is deliberate\n", ApiName(es));
        gl.Enable(0xFFFF);
        ESIA_CHECK(ErrorCount(*h.device) == 1);
        while (gl.GetError() != GL_NO_ERROR) {}
    }
}

// What each API family reports, and what Desc::coreOnly leaves of it.
ESIA_TEST(GlDevice, Caps)
{
    for (bool coreOnly : {false, true})
        for (bool es : kApis)
        {
            rhi::HeadlessDevice h = Headless(es, 8, 8, rhi::Format::RGBA8_SRGB, 1, coreOnly);
            if (!h.device)
                continue;
            const rhi::Caps& c = h.device->GetCaps();
            ESIA_CHECK(std::strcmp(h.device->Name(), ApiName(es)) == 0);
            ESIA_CHECK(c.fxStorage == rhi::FxStorage::Texture);
            ESIA_CHECK(c.framebufferOriginBottomLeft && !c.clipSpaceYDown && !c.halfPixelOffset);
            ESIA_CHECK(c.readback && c.sampleRenderTarget && !c.runtimeEffects && !c.fxFeatureVariants);
            ESIA_CHECK(c.maxFxDataWidth >= 24 && c.maxFxDataWidth <= c.maxTextureSize);
            // GL 3.3 has these in core; GLES 3.0 only through extensions (llvmpipe has them all)
            ESIA_CHECK(c.floatRenderTargets == (!es || !coreOnly));
            ESIA_CHECK(c.timestampQueries == (!es || !coreOnly));
            // an sRGB target is sampled raw only with EXT_texture_sRGB_decode
            const bool sampled = (h.device->GetTextureDesc(h.target).usage & rhi::TextureUsage_Sampled) != 0;
            ESIA_CHECK(sampled == !coreOnly);
        }
}

// Uploaded textures keep their rows top first; readback converts every format to RGBA8; sub-rectangle updates
// with a row pitch land at their top-left position.
ESIA_TEST(GlDevice, UploadAndReadBackFormats)
{
    for (bool es : kApis)
    {
        rhi::HeadlessDevice h = Headless(es, 8, 8);
        if (!h.device)
            continue;
        rhi::Device& dev = *h.device;
        const std::uint32_t usage = rhi::TextureUsage_Sampled | rhi::TextureUsage_CopyDst;
        const int w = 4, hgt = 3;
        std::vector<std::uint8_t> rgba(w * hgt * 4), expect(w * hgt * 4);
        for (std::size_t i = 0; i < rgba.size(); ++i)
            rgba[i] = (std::uint8_t)(i * 7 + 3);

        rhi::Texture t = dev.CreateTexture(TexDesc(w, hgt, rhi::Format::RGBA8_UNORM, usage), rgba.data(), 0);
        ESIA_CHECK(Read(dev, t, {0, 0, w, hgt}) == rgba);
        ESIA_CHECK(Read(dev, t, {1, 1, 3, 3}) == std::vector<std::uint8_t>({rgba[20], rgba[21], rgba[22], rgba[23], rgba[24], rgba[25], rgba[26], rgba[27],
                                                                             rgba[36], rgba[37], rgba[38], rgba[39], rgba[40], rgba[41], rgba[42], rgba[43]}));
        // a 2 x 2 update at (2, 1) from rows 12 bytes apart
        const std::uint8_t patch[24] = {1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 9, 10, 11, 12, 13, 14, 15, 16, 0, 0, 0, 0};
        dev.UpdateTexture(t, {2, 1, 4, 3}, patch, 12);
        expect = rgba;
        std::memcpy(&expect[(1 * w + 2) * 4], patch, 8);
        std::memcpy(&expect[(2 * w + 2) * 4], patch + 12, 8);
        ESIA_CHECK(Read(dev, t, {0, 0, w, hgt}) == expect);
        dev.DestroyTexture(t);

        // BGRA8: bytes B, G, R, A in memory; read back as RGBA
        t = dev.CreateTexture(TexDesc(w, hgt, rhi::Format::BGRA8_UNORM, usage), rgba.data(), 0);
        expect = rgba;
        for (std::size_t i = 0; i < expect.size(); i += 4)
            std::swap(expect[i], expect[i + 2]);
        ESIA_CHECK(Read(dev, t, {0, 0, w, hgt}) == expect);
        dev.DestroyTexture(t);

        // R8: red only
        t = dev.CreateTexture(TexDesc(w, hgt, rhi::Format::R8_UNORM, usage), rgba.data(), 0);
        const std::vector<std::uint8_t> r8 = Read(dev, t, {0, 0, w, hgt});
        bool ok = true;
        for (int i = 0; i < w * hgt; ++i)
            ok &= r8[i * 4] == rgba[i] && r8[i * 4 + 1] == 0 && r8[i * 4 + 2] == 0 && r8[i * 4 + 3] == 255;
        ESIA_CHECK(ok);
        dev.DestroyTexture(t);

        // RGB10A2: 10-bit channels to 8 bits
        std::vector<std::uint32_t> packed(w * hgt);
        for (int i = 0; i < w * hgt; ++i)
            packed[i] = (std::uint32_t)(i * 85) | ((std::uint32_t)(1023 - i * 85) << 10) | (512u << 20) | ((std::uint32_t)(i % 4) << 30);
        t = dev.CreateTexture(TexDesc(w, hgt, rhi::Format::RGB10A2_UNORM, usage), packed.data(), 0);
        const std::vector<std::uint8_t> p10 = Read(dev, t, {0, 0, w, hgt});
        ok = true;
        for (int i = 0; i < w * hgt; ++i)
        {
            ok &= p10[i * 4] == (std::uint8_t)std::lround(i * 85 * 255.0 / 1023.0);
            ok &= p10[i * 4 + 1] == (std::uint8_t)std::lround((1023 - i * 85) * 255.0 / 1023.0);
            ok &= p10[i * 4 + 2] == 128 && p10[i * 4 + 3] == (std::uint8_t)((i % 4) * 85);
        }
        ESIA_CHECK(ok);
        dev.DestroyTexture(t);

        // RGBA32F (the FX instance texture): clamped and rounded
        const float f[4] = {0.5f, -1.0f, 2.0f, 0.25f};
        t = dev.CreateTexture(TexDesc(1, 1, rhi::Format::RGBA32_FLOAT, usage), f, 0);
        ESIA_CHECK(Read(dev, t, {0, 0, 1, 1}) == std::vector<std::uint8_t>({128, 0, 255, 64}));
        dev.DestroyTexture(t);
        ESIA_CHECK(ErrorCount(dev) == 0);
    }
}

// Rendered rows are GL's bottom-up rows: scissors, passes and readback convert them, so the RHI sees top-left
// rectangles only. Clears load in every format the renderer renders into.
ESIA_TEST(GlDevice, ScissorsAndReadbackAreTopLeft)
{
    for (bool es : kApis)
        for (rhi::Format format : {rhi::Format::RGBA8_UNORM, rhi::Format::RGBA16_FLOAT, rhi::Format::RGBA8_SRGB})
        {
            rhi::HeadlessDevice h = Headless(es, 64, 32, format);
            if (!h.device)
                continue;
            rhi::Device& dev = *h.device;
            rhi::PipelineDesc pd;
            pd.program = rhi::ShaderProgram::Clear;
            pd.layout = rhi::VertexLayout::None;
            pd.blend = rhi::BlendMode::Opaque;
            pd.targetFormat = format;
            const rhi::Pipeline clear = dev.CreatePipeline(pd);
            ESIA_CHECK((bool)clear);
            ESIA_CHECK(dev.BeginFrame({}));
            rhi::PassDesc pass;
            pass.target = h.target;
            pass.load = rhi::LoadOp::Clear;
            pass.clearColor[0] = pass.clearColor[3] = 1.0f;
            dev.BeginPass(pass);
            dev.SetPipeline(clear);
            FrameConstants(dev);
            dev.SetScissor({0, 0, 16, 8});     // top-left
            dev.Draw(3, 0);
            dev.SetScissor({48, 24, 64, 32});  // bottom-right
            dev.Draw(3, 0);
            dev.EndPass();
            dev.EndFrame();
            const std::vector<std::uint8_t> px = Read(dev, h.target, {0, 0, 64, 32});
            bool ok = true;
            for (int y = 0; y < 32; ++y)
                for (int x = 0; x < 64; ++x)
                {
                    const bool cleared = (x < 16 && y < 8) || (x >= 48 && y >= 24);
                    const std::uint8_t* p = &px[(std::size_t)(y * 64 + x) * 4];
                    ok &= cleared ? (p[0] == 0 && p[3] == 0) : (p[0] == 255 && p[1] == 0 && p[3] == 255);
                }
            ESIA_CHECK(ok);
            // a sub-rectangle across both regions keeps its rows top first
            const std::vector<std::uint8_t> sub = Read(dev, h.target, {8, 4, 24, 12});
            ESIA_CHECK(sub[0] == 0 && sub[(3 * 16 + 7) * 4] == 0 && sub[(4 * 16) * 4] == 255 && sub[(3 * 16 + 8) * 4] == 255);
            ESIA_CHECK(ErrorCount(dev) == 0);
        }
}

// CopyTexture copies bits: from a render target into a copy at an offset, from an sRGB target into its raw format
// (all 256 levels exact, with and without EXT_texture_sRGB_decode), and from multisampled targets (resolved).
ESIA_TEST(GlDevice, CopiesKeepTheBits)
{
    for (bool coreOnly : {false, true})
        for (bool es : kApis)
            for (rhi::Format format : {rhi::Format::RGBA8_UNORM, rhi::Format::RGBA8_SRGB})
            {
                rhi::HeadlessDevice h = Headless(es, 8, 8, rhi::Format::RGBA8_UNORM, 1, coreOnly);
                if (!h.device)
                    continue;
                rhi::Device& dev = *h.device;
                // a 256 x 4 render target with every 8-bit value in every channel
                std::vector<std::uint8_t> px(256 * 4 * 4);
                for (int y = 0; y < 4; ++y)
                    for (int x = 0; x < 256; ++x)
                    {
                        std::uint8_t* p = &px[(std::size_t)(y * 256 + x) * 4];
                        p[0] = (std::uint8_t)x;
                        p[1] = (std::uint8_t)(255 - x);
                        p[2] = (std::uint8_t)(x * 3 + y);
                        p[3] = (std::uint8_t)(x ^ 0x5A);
                    }
                const rhi::Texture src = dev.CreateTexture(TexDesc(256, 4, format, rhi::TextureUsage_RenderTarget | rhi::TextureUsage_CopySrc), px.data(), 0);
                const rhi::Texture dst = dev.CreateTexture(TexDesc(260, 8, rhi::RawFormat(format), rhi::TextureUsage_Sampled | rhi::TextureUsage_CopyDst), nullptr, 0);
                ESIA_CHECK(src && dst);
                ESIA_CHECK(Read(dev, src, {0, 0, 256, 4}) == px);   // stored values, top row first
                ESIA_CHECK(dev.BeginFrame({}));
                dev.CopyTexture(dst, 3, 2, src, {0, 1, 256, 4});
                dev.EndFrame();
                const std::vector<std::uint8_t> copied = Read(dev, dst, {3, 2, 259, 5});
                ESIA_CHECK(copied == std::vector<std::uint8_t>(px.begin() + 256 * 4, px.end()));

                // multisampled: resolved (a cleared color, so every sample agrees)
                const rhi::Texture ms = dev.CreateTexture(TexDesc(16, 16, format, rhi::TextureUsage_RenderTarget | rhi::TextureUsage_CopySrc, 4), nullptr, 0);
                ESIA_CHECK((bool)ms);
                ESIA_CHECK(dev.BeginFrame({}));
                rhi::PassDesc pass;
                pass.target = ms;
                pass.load = rhi::LoadOp::Clear;
                pass.clearColor[0] = 0.25f;
                pass.clearColor[1] = 0.5f;
                pass.clearColor[2] = 0.75f;
                pass.clearColor[3] = 1.0f;
                dev.BeginPass(pass);
                dev.EndPass();
                dev.CopyTexture(dst, 0, 0, ms, {4, 4, 12, 12});
                dev.EndFrame();
                const std::vector<std::uint8_t> stored = Read(dev, ms, {4, 4, 12, 12});
                ESIA_CHECK(Read(dev, dst, {0, 0, 8, 8}) == stored);
                // an sRGB clear color is linear and encoded (0.5 -> 188), as on every API
                const bool srgb = rhi::IsSrgb(format);
                ESIA_CHECK(std::abs((int)stored[0] - (srgb ? 137 : 64)) <= 1 && std::abs((int)stored[1] - (srgb ? 188 : 128)) <= 1 &&
                           std::abs((int)stored[2] - (srgb ? 225 : 191)) <= 1 && stored[3] == 255);
                ESIA_CHECK(ErrorCount(dev) == 0);
            }
}

// The host's GL state survives a frame and every call outside frames; the host's odd state does not leak into
// the device's rendering either.
ESIA_TEST(GlDevice, HostStateIsRestored)
{
    const conformance::Scene& scene = *conformance::FindScene("glass");
    for (bool es : kApis)
    {
        rhi::HeadlessDevice h = Headless(es, scene.width, scene.height);
        if (!h.device)
            continue;
        const std::vector<std::uint8_t> clean = RenderScene(*h.device, h.target, scene);

        const GlApi gl = LoadGl();
        GLuint tex = 0, fbo = 0, buf = 0, vao = 0;
        gl.GenTextures(1, &tex);
        gl.GenFramebuffers(1, &fbo);
        gl.GenBuffers(1, &buf);
        gl.GenVertexArrays(1, &vao);
        gl.ActiveTexture(GL_TEXTURE0 + 3);
        gl.BindTexture(GL_TEXTURE_2D, tex);
        gl.TexImage2D(GL_TEXTURE_2D, 0, (GLint)GL_RGBA8, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        gl.BindFramebuffer(GL_FRAMEBUFFER, fbo);
        gl.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
        gl.BindVertexArray(vao);
        gl.BindBuffer(GL_ARRAY_BUFFER, buf);
        gl.BindBufferBase(GL_UNIFORM_BUFFER, 1, buf);
        gl.Viewport(1, 2, 3, 4);
        gl.Scissor(5, 6, 7, 8);
        gl.Enable(GL_SCISSOR_TEST);
        gl.Enable(GL_BLEND);
        gl.BlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
        gl.ColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE);
        gl.Enable(GL_CULL_FACE);
        gl.ClearColor(0.25f, 0.5f, 0.75f, 1.0f);
        gl.PixelStorei(GL_UNPACK_ALIGNMENT, 8);
        gl.PixelStorei(GL_PACK_ALIGNMENT, 2);

        auto check = [&](const char* when) {
            GLint v[4] = {};
            bool ok = true;
            gl.GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, v);
            ok &= v[0] == (GLint)fbo;
            gl.GetIntegerv(GL_ACTIVE_TEXTURE, v);
            ok &= v[0] == (GLint)(GL_TEXTURE0 + 3);
            gl.GetIntegerv(GL_TEXTURE_BINDING_2D, v);
            ok &= v[0] == (GLint)tex;
            gl.GetIntegerv(GL_VERTEX_ARRAY_BINDING, v);
            ok &= v[0] == (GLint)vao;
            gl.GetIntegerv(GL_ARRAY_BUFFER_BINDING, v);
            ok &= v[0] == (GLint)buf;
            gl.GetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, 1, v);
            ok &= v[0] == (GLint)buf;
            gl.GetIntegerv(GL_VIEWPORT, v);
            ok &= v[0] == 1 && v[1] == 2 && v[2] == 3 && v[3] == 4;
            gl.GetIntegerv(GL_SCISSOR_BOX, v);
            ok &= v[0] == 5 && v[1] == 6 && v[2] == 7 && v[3] == 8;
            gl.GetIntegerv(GL_BLEND_SRC_RGB, v);
            ok &= v[0] == (GLint)GL_ZERO;
            gl.GetIntegerv(GL_UNPACK_ALIGNMENT, v);
            ok &= v[0] == 8;
            gl.GetIntegerv(GL_PACK_ALIGNMENT, v);
            ok &= v[0] == 2;
            GLboolean mask[4] = {};
            gl.GetBooleanv(GL_COLOR_WRITEMASK, mask);
            ok &= mask[0] && !mask[1] && mask[2] && !mask[3];
            GLfloat color[4] = {};
            gl.GetFloatv(GL_COLOR_CLEAR_VALUE, color);
            ok &= color[1] == 0.5f;
            ok &= gl.IsEnabled(GL_SCISSOR_TEST) && gl.IsEnabled(GL_BLEND) && gl.IsEnabled(GL_CULL_FACE);
            if (!ok)
                std::printf("  %s: host state changed by %s\n", ApiName(es), when);
            ESIA_CHECK(ok);
        };
        check("setup");
        const std::vector<std::uint8_t> hosted = RenderScene(*h.device, h.target, scene);   // a frame, then ReadPixels
        check("a frame and ReadPixels");
        ESIA_CHECK(hosted == clean);
        const rhi::Texture t = h.device->CreateTexture(TexDesc(8, 8, rhi::Format::RGBA16_FLOAT, rhi::TextureUsage_RenderTarget | rhi::TextureUsage_Sampled),
                                                       nullptr, 0);
        const std::uint8_t white[4] = {255, 255, 255, 255};
        h.device->UpdateTexture(t, {0, 0, 1, 1}, white, 0);
        h.device->DestroyTexture(t);
        check("resource calls");
        ESIA_CHECK(ErrorCount(*h.device) == 0);
        gl.BindFramebuffer(GL_FRAMEBUFFER, 0);
        gl.DeleteFramebuffers(1, &fbo);
        gl.DeleteTextures(1, &tex);
        gl.DeleteBuffers(1, &buf);
        gl.DeleteVertexArrays(1, &vao);
    }
}

// Host framebuffers: with their color texture (sampled directly by frosted glass) and without (copied), wrapping
// again returns the same handle, and the result is the headless target's.
ESIA_TEST(GlDevice, WrappedFramebuffers)
{
    const conformance::Scene& scene = *conformance::FindScene("glass");
    for (bool es : kApis)
    {
        rhi::HeadlessDevice h = Headless(es, scene.width, scene.height);
        if (!h.device)
            continue;
        rhi::Device& dev = *h.device;
        const std::vector<std::uint8_t> reference = RenderScene(dev, h.target, scene);
        const GlApi gl = LoadGl();
        GLuint tex = 0, fbo = 0;
        gl.GenTextures(1, &tex);
        gl.BindTexture(GL_TEXTURE_2D, tex);
        gl.TexImage2D(GL_TEXTURE_2D, 0, (GLint)GL_RGBA8, scene.width, scene.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        gl.GenFramebuffers(1, &fbo);
        gl.BindFramebuffer(GL_FRAMEBUFFER, fbo);
        gl.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
        for (bool withTexture : {true, false})
        {
            const rhi::Texture t = WrapFramebuffer(dev, fbo, scene.width, scene.height, rhi::Format::RGBA8_UNORM, withTexture ? tex : 0);
            ESIA_CHECK(WrapFramebuffer(dev, fbo, scene.width, scene.height, rhi::Format::RGBA8_UNORM, withTexture ? tex : 0) == t);
            const rhi::TextureDesc d = dev.GetTextureDesc(t);
            ESIA_CHECK(d.width == scene.width && (d.usage & rhi::TextureUsage_RenderTarget) && (d.usage & rhi::TextureUsage_CopySrc));
            ESIA_CHECK(((d.usage & rhi::TextureUsage_Sampled) != 0) == withTexture);
            ESIA_CHECK(NativeTexture(dev, t) == (withTexture ? tex : 0u));
            render::Renderer renderer(dev);
            conformance::SceneFrame f;
            conformance::BuildScene(scene, f);
            ESIA_CHECK(renderer.Render(f.data, &f.textures, t));
            ESIA_CHECK(renderer.Stats().directCaptures == (withTexture ? 1 : 0));
            ESIA_CHECK(MaxDelta(Read(dev, t, {0, 0, scene.width, scene.height}), reference) == 0);
            dev.DestroyTexture(t);
        }
        GLint alive = 0;
        gl.BindTexture(GL_TEXTURE_2D, tex);
        gl.GetIntegerv(GL_TEXTURE_BINDING_2D, &alive);
        ESIA_CHECK(alive == (GLint)tex);   // the host's objects outlive their wrappers
        ESIA_CHECK(ErrorCount(dev) == 0);
        gl.DeleteFramebuffers(1, &fbo);
        gl.DeleteTextures(1, &tex);
    }
}

// A host callback may change any state: what is drawn after it (in the same pass, or after a capture that ended
// the pass) is unaffected.
namespace
{
    const GlApi* gCallbackGl = nullptr;

    void MessUpState(const DrawList&, const DrawCmd&, void* state)
    {
        ESIA_CHECK(state == nullptr);   // GL: the current context is the render state
        const GlApi& gl = *gCallbackGl;
        gl.BindFramebuffer(GL_FRAMEBUFFER, 0);
        gl.UseProgram(0);
        gl.BindVertexArray(0);
        gl.Viewport(0, 0, 1, 1);
        gl.Disable(GL_SCISSOR_TEST);
        gl.BlendFuncSeparate(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);
        gl.ColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        gl.Enable(GL_CULL_FACE);
        for (GLenum u = 0; u < 9; ++u)
        {
            gl.ActiveTexture(GL_TEXTURE0 + u);
            gl.BindTexture(GL_TEXTURE_2D, 0);
            gl.BindSampler(u, 0);
        }
        for (GLuint b = 0; b < 3; ++b)
            gl.BindBufferBase(GL_UNIFORM_BUFFER, b, 0);
    }

    void DrawCallbackScene(DrawData& dd, std::unique_ptr<DrawList>& list, bool withCallback)
    {
        list = std::make_unique<DrawList>();
        list->Reset(Rect(0, 0, 64, 64));
        Painter p(*list);
        p.Rect(Rect(0, 0, 64, 64), Style().Fill(Color::Hex(0x203040)));
        p.Rect(Rect(4, 4, 30, 30), Style().Fill(Color::Hex(0xFF9F0A)).Radius(6));
        list->AddRectFilled(Rect(8, 40, 56, 44), Color::White().ToRgba8());
        if (withCallback)
            list->AddCallback(&MessUpState, nullptr);
        // glass right after the callback: its capture ends the pass, copies and starts the pyramid's passes
        GlassMaterial glass;
        glass.blur = 4.0f;
        p.Rect(Rect(2, 36, 62, 62), Style().Radius(8).Glass(glass));
        p.Rect(Rect(34, 4, 60, 30), Style().Fill(Color::Hex(0x30D158)).Radius(6).Shadow(Color::Black(0.5f), 4, Vec2(0, 2)));
        list->AddRectFilled(Rect(8, 50, 56, 54), Color::Hex(0x0A84FF).ToRgba8());
        dd = DrawData();
        dd.displaySize = Vec2(64, 64);
        dd.framebufferScale = Vec2(1, 1);
        dd.lists.push_back(list.get());
    }
}

ESIA_TEST(GlDevice, HostCallbacksChangeNothingAfterThem)
{
    for (bool es : kApis)
    {
        rhi::HeadlessDevice h = Headless(es, 64, 64);
        if (!h.device)
            continue;
        const GlApi gl = LoadGl();
        gCallbackGl = &gl;
        std::vector<std::uint8_t> result[2];
        for (int withCallback = 0; withCallback < 2; ++withCallback)
        {
            DrawData dd;
            std::unique_ptr<DrawList> list;
            DrawCallbackScene(dd, list, withCallback != 0);
            render::Renderer renderer(*h.device);
            ESIA_CHECK(renderer.Render(dd, nullptr, h.target));
            result[withCallback] = Read(*h.device, h.target, {0, 0, 64, 64});
        }
        ESIA_CHECK(result[0] == result[1]);
        ESIA_CHECK(result[0][(10 * 64 + 10) * 4] == 0xFF && result[0][(10 * 64 + 50) * 4 + 1] == 0xD1);   // both rects drawn
        ESIA_CHECK(ErrorCount(*h.device) == 0);
    }
}

// GPU times arrive a few frames later without stalling, per category (glass: captures and glass batches).
ESIA_TEST(GlDevice, GpuTimes)
{
    const conformance::Scene& scene = *conformance::FindScene("glass");
    for (bool es : kApis)
    {
        rhi::HeadlessDevice h = Headless(es, scene.width, scene.height);
        if (!h.device || !h.device->GetCaps().timestampQueries)
            continue;
        render::Renderer renderer(*h.device);
        for (int i = 0; i < 8; ++i)
        {
            conformance::SceneFrame f;
            conformance::BuildScene(scene, f);
            ESIA_CHECK(renderer.Render(f.data, &f.textures, h.target));
        }
        const rhi::GpuProfile& gpu = renderer.Stats().gpu;
        ESIA_CHECK(gpu.valid && gpu.frame >= 1 && gpu.frame <= 8);
        ESIA_CHECK(gpu.totalMs > 0.0f);
        ESIA_CHECK(gpu.categoryMs[(int)rhi::ProfileCategory::Capture] > 0.0f);
        ESIA_CHECK(gpu.categoryMs[(int)rhi::ProfileCategory::FxGlass] > 0.0f);
        float sum = 0.0f;
        for (float ms : gpu.categoryMs)
            sum += ms;
        ESIA_CHECK(sum <= gpu.totalMs * 1.001f);
        std::printf("  %s: frame %llu, %.3f ms (capture %.3f, glass %.3f)\n", ApiName(es), (unsigned long long)gpu.frame, (double)gpu.totalMs,
                    (double)gpu.categoryMs[(int)rhi::ProfileCategory::Capture], (double)gpu.categoryMs[(int)rhi::ProfileCategory::FxGlass]);
        ESIA_CHECK(ErrorCount(*h.device) == 0);
    }
}

// The renderer's command stream for this backend's caps keeps the RHI contract (the null device checks it).
ESIA_TEST(GlDevice, RendererKeepsTheContractWithGlCaps)
{
    for (bool coreOnly : {false, true})
        for (bool es : kApis)
        {
            rhi::HeadlessDevice h = Headless(es, 8, 8, rhi::Format::RGBA8_UNORM, 1, coreOnly);
            if (!h.device)
                continue;
            for (const conformance::Scene& scene : conformance::Scenes())
            {
                rhi::NullOptions o;
                o.caps = h.device->GetCaps();
                o.record = false;
                rhi::NullDevice null(o);
                const rhi::Texture target = null.CreateHostTarget(scene.width, scene.height, scene.format, !rhi::IsSrgb(scene.format) || !coreOnly, scene.samples);
                render::Renderer renderer(null);
                conformance::SceneFrame f;
                conformance::BuildScene(scene, f);
                ESIA_CHECK(renderer.Render(f.data, &f.textures, target));
                if (!null.Errors().empty())
                    std::printf("  %s %s: %s\n", ApiName(es), scene.name, null.Errors()[0].c_str());
                ESIA_CHECK(null.Errors().empty());
            }
        }
}
