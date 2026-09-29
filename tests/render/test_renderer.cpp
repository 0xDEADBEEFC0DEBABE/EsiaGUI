// Renderer on the null RHI backend: the command stream it records for each technique (captures, direct reads,
// pyramids, glow layers, text programs, instance storage, callbacks, profiling), with the null device validating
// every call against the RHI contract.
#include "esia/render/painter.hpp"
#include "esia/render/renderer.hpp"
#include "esia/rhi/null_device.hpp"
#include "esia_test.hpp"
#include <cstring>
#include <sstream>
#include <string>

using namespace esia;
using namespace esia::render;
using namespace esia::rhi;

namespace
{
    struct Setup
    {
        NullDevice dev;
        Texture target;
        explicit Setup(Caps caps = {}, Format format = Format::RGBA8_UNORM, bool sampleable = true, int samples = 1, bool keepData = false)
            : dev(NullOptions{caps, true, keepData})
        {
            target = dev.CreateHostTarget(400, 300, format, sampleable, samples);
        }
    };

    DrawData Data(std::initializer_list<const DrawList*> lists)
    {
        DrawData dd;
        dd.lists = lists;
        dd.displaySize = Vec2(400, 300);
        dd.time = 2.0;
        return dd;
    }

    DrawList MakeList()
    {
        DrawList dl;
        dl.Reset(Rect(0, 0, 400, 300));
        return dl;
    }

    GlassMaterial Glass(float blur)
    {
        GlassMaterial g;
        g.blur = blur;
        return g;
    }

    int Lines(const NullDevice& d, const std::string& needle)
    {
        int n = 0;
        for (const std::string& l : d.Log())
            n += l.find(needle) != std::string::npos;
        return n;
    }

    bool NoErrors(const NullDevice& d)
    {
        for (const std::string& e : d.Errors())
            std::fprintf(stderr, "    null device: %s\n", e.c_str());
        return d.Errors().empty();
    }

    // The values of the first "constants frame" line recorded after the line containing `after`.
    std::vector<float> FrameConstantsAfter(const NullDevice& d, const std::string& after)
    {
        bool seen = false;
        for (const std::string& l : d.Log())
        {
            if (!seen)
            {
                seen = l.find(after) != std::string::npos;
                continue;
            }
            if (l.rfind("constants frame: ", 0) == 0)
            {
                std::string s = l.substr(17);
                for (char& c : s)
                    if (c == '|')
                        c = ' ';
                std::istringstream in(s);
                std::vector<float> v;
                float f;
                while (in >> f)
                    v.push_back(f);
                return v;
            }
        }
        return {};
    }

    // wallpaper + a glass card
    void GlassScene(DrawList& dl, float blur)
    {
        Painter p(dl);
        p.Rect(Rect(0, 0, 400, 300), Style().Fill(Paint::Linear(Color::Hex(0x203050), Color::Hex(0x805030))));
        p.Rect(Rect(60, 60, 340, 240), Style().Radius(24).Glass(Glass(blur)));
    }
}

ESIA_TEST(Renderer, GeometryAndFx)
{
    Setup s;
    DrawList dl = MakeList();
    Painter p(dl);
    p.Rect(Rect(10, 10, 110, 60), Style().Fill(Color::Hex(0x0A84FF)).Radius(12));
    dl.AddRectFilled(Rect(150, 10, 200, 60), 0xFF00FF00u);
    Renderer r(s.dev);
    ESIA_CHECK(r.Render(Data({&dl}), nullptr, s.target));
    ESIA_CHECK(NoErrors(s.dev));
    ESIA_CHECK(Lines(s.dev, "draw instanced 4 x 1") == 1);
    ESIA_CHECK(Lines(s.dev, "draw indexed 6 from 0") == 1);
    ESIA_CHECK(r.Stats().drawCalls == 2 && r.Stats().passes == 1 && r.Stats().fxInstances == 1 && r.Stats().backdropCaptures == 0);
    ESIA_CHECK(Lines(s.dev, "create texture") == 2);   // host target + white
    ESIA_CHECK(Lines(s.dev, "copy ") == 0 && Lines(s.dev, "pyramid") == 0);

    // the second frame reuses everything
    s.dev.ClearLog();
    ESIA_CHECK(r.Render(Data({&dl}), nullptr, s.target));
    ESIA_CHECK(NoErrors(s.dev));
    ESIA_CHECK(Lines(s.dev, "create ") == 0);
}

ESIA_TEST(Renderer, FrostedGlassReadsTheTargetDirectly)
{
    Setup s;
    DrawList dl = MakeList();
    GlassScene(dl, 10);
    Renderer r(s.dev);
    r.Render(Data({&dl}), nullptr, s.target);
    ESIA_CHECK(NoErrors(s.dev));
    ESIA_CHECK(r.Stats().backdropCaptures == 1 && r.Stats().directCaptures == 1);
    ESIA_CHECK(Lines(s.dev, "copy ") == 0);
    ESIA_CHECK(Lines(s.dev, "dont-care pyramid") == 4);   // env blur 16 px: levels 1..4
    ESIA_CHECK(r.Stats().passes == 6);                    // ui, 4 pyramid levels, ui again
    ESIA_CHECK(Lines(s.dev, "texture t0 #1") == 1);       // the first downsample reads the target itself
}

ESIA_TEST(Renderer, ClearGlassUnsampleableAndMsaaTargetsAreCopied)
{
    {
        Setup s;   // clear glass reads the full-resolution level: a copy
        DrawList dl = MakeList();
        GlassScene(dl, 0);
        Renderer r(s.dev);
        r.Render(Data({&dl}), nullptr, s.target);
        ESIA_CHECK(NoErrors(s.dev));
        ESIA_CHECK(r.Stats().backdropCaptures == 1 && r.Stats().directCaptures == 0);
        ESIA_CHECK(Lines(s.dev, "copy #1 -> ") == 1);
    }
    {
        Setup s(Caps{}, Format::RGBA8_UNORM, false);   // the host target cannot be sampled
        DrawList dl = MakeList();
        GlassScene(dl, 10);
        Renderer r(s.dev);
        r.Render(Data({&dl}), nullptr, s.target);
        ESIA_CHECK(NoErrors(s.dev));
        ESIA_CHECK(r.Stats().directCaptures == 0 && Lines(s.dev, "copy #1 -> ") == 1);
    }
    {
        Setup s(Caps{}, Format::RGBA8_UNORM, true, 4);   // multisampled: resolved by the copy
        DrawList dl = MakeList();
        GlassScene(dl, 10);
        Renderer r(s.dev);
        r.Render(Data({&dl}), nullptr, s.target);
        ESIA_CHECK(NoErrors(s.dev));
        ESIA_CHECK(r.Stats().directCaptures == 0 && Lines(s.dev, "copy #1 -> ") == 1);
    }
}

ESIA_TEST(Renderer, SrgbTargetsCopyRawAndEncodeTheirOutput)
{
    Setup s(Caps{}, Format::RGBA8_SRGB, false);
    DrawList dl = MakeList();
    Painter p(dl);
    p.BeginGlowLayer(Color::White(), 8);
    p.Rect(Rect(20, 20, 60, 60), Style().Fill(Color::White()));
    p.EndGlowLayer();
    GlassScene(dl, 10);
    Renderer r(s.dev);
    r.Render(Data({&dl}), nullptr, s.target);
    ESIA_CHECK(NoErrors(s.dev));
    ESIA_CHECK(Lines(s.dev, "RGBA8_UNORM usage=9 backdrop-copy") == 1);   // the raw sibling of the sRGB target
    const std::vector<float> main = FrameConstantsAfter(s.dev, "load ui");
    const std::vector<float> layer = FrameConstantsAfter(s.dev, "dont-care glow-layer");
    ESIA_CHECK(main.size() == 48 && layer.size() == 48);
    if (main.size() == 48 && layer.size() == 48)
    {
        ESIA_CHECK(main[15] == 1.0f);    // gTime.w: write linear
        ESIA_CHECK(layer[15] == 0.0f);   // layers are composed in gamma space
        ESIA_CHECK(main[12] == 2.0f && main[14] == 1.0f);   // time, backdrop valid
        ESIA_CHECK(main[4] == 400.0f && main[5] == 300.0f);
    }
}

ESIA_TEST(Renderer, CaptureBudget)
{
    Setup s;
    DrawList dl = MakeList();
    Painter p(dl);
    for (int i = 0; i < 3; ++i)
    {
        const float x = 20.0f + 120.0f * (float)i;
        p.Rect(Rect(x, 20, x + 100, 280), Style().Fill(Color::Hex(0x336699)));   // content under each glass
        p.Rect(Rect(x + 10, 40, x + 90, 120), Style().Radius(16).Glass(Glass(10)));
    }
    Renderer r(s.dev);
    RenderParams params;
    params.maxBackdropCaptures = 1;
    r.Render(Data({&dl}), nullptr, s.target, params);
    ESIA_CHECK(NoErrors(s.dev));
    ESIA_CHECK(r.Plan().plannedCaptures == 3);
    ESIA_CHECK(r.Stats().backdropCaptures == 1 && r.Stats().overBudget);
}

ESIA_TEST(Renderer, GlowLayer)
{
    Setup s;
    DrawList dl = MakeList();
    Painter p(dl);
    p.BeginGlowLayer(Color::Hex(0x00FFFF, 0.5f), 12.0f, 1.5f);
    p.Rect(Rect(100, 100, 200, 140), Style().Fill(Color::White()).Radius(8));
    p.EndGlowLayer();
    Renderer r(s.dev);
    r.Render(Data({&dl}), nullptr, s.target);
    ESIA_CHECK(NoErrors(s.dev));
    ESIA_CHECK(r.Stats().glowLayers == 1);
    ESIA_CHECK(Lines(s.dev, "dont-care glow-layer") == 1);
    ESIA_CHECK(Lines(s.dev, "Clear list opaque RGBA16_FLOAT") == 1);
    ESIA_CHECK(Lines(s.dev, "LayerComposite list premul RGBA8_UNORM") == 1);
    ESIA_CHECK(Lines(s.dev, "dont-care pyramid") == 4);   // bloom 12 px: level 2 + next + the wide tail
    ESIA_CHECK(Lines(s.dev, "constants pass: 0 1 1 0.5 | 1.5 12 1 0") == 1);
    ESIA_CHECK(Lines(s.dev, "backdrop-copy") == 0);        // no glass: no copy texture
}

ESIA_TEST(Renderer, SubpixelTextPrograms)
{
    auto run = [](bool dualSource, bool inLayer) {
        Caps caps;
        caps.dualSourceBlend = dualSource;
        Setup s(caps);
        TextureRegistry reg;
        const TextureId page = reg.Create({TextureFormat::RGBA8, 64, 64, TextureFlags_LcdCoverage});
        DrawList dl = MakeList();
        Painter p(dl);
        if (inLayer)
            p.BeginGlowLayer(Color::White(), 6);
        dl.AddImage(page, Rect(10, 10, 20, 20), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
        if (inLayer)
            p.EndGlowLayer();
        Renderer r(s.dev);
        r.Render(Data({&dl}), &reg, s.target);
        ESIA_CHECK(NoErrors(s.dev));
        return std::make_pair(Lines(s.dev, "TextLcd list dual-source"), Lines(s.dev, "TextLcdGray list straight"));
    };
    ESIA_CHECK(run(true, false) == std::make_pair(1, 0));
    ESIA_CHECK(run(false, false) == std::make_pair(0, 1));
    ESIA_CHECK(run(true, true) == std::make_pair(0, 1));   // alpha layers get the grayscale coverage
}

ESIA_TEST(Renderer, TextureRegistryReachesTheDevice)
{
    Setup s;
    TextureRegistry reg;
    const std::uint8_t pixels[4 * 4] = {};
    const TextureId glyphs = reg.Create({TextureFormat::Alpha8, 4, 4, 0}, pixels);
    const TextureId image = reg.Create({TextureFormat::RGBA8, 2, 2, 0}, pixels);
    DrawList dl = MakeList();
    dl.AddImage(glyphs, Rect(0, 0, 4, 4), Vec2(0, 0), Vec2(1, 1), 0xFFFFFFFFu);
    Renderer r(s.dev);
    r.Render(Data({&dl}), &reg, s.target);
    ESIA_CHECK(NoErrors(s.dev));
    ESIA_CHECK(Lines(s.dev, "4x4 R8_UNORM usage=9 glyphs") == 1 && Lines(s.dev, "2x2 RGBA8_UNORM usage=9 image") == 1);
    ESIA_CHECK(Lines(s.dev, "TextGray list straight") == 1);   // an Alpha8 page draws with the grayscale text program
    ESIA_CHECK((bool)r.GpuTexture(glyphs) && (bool)r.GpuTexture(image));
    reg.Update(glyphs, 1, 1, 2, 2, pixels);
    reg.Destroy(image);
    s.dev.ClearLog();
    r.Render(Data({&dl}), &reg, s.target);
    ESIA_CHECK(NoErrors(s.dev));
    ESIA_CHECK(Lines(s.dev, "update texture") == 1 && Lines(s.dev, "destroy texture") == 1);
    ESIA_CHECK(!r.GpuTexture(image));
}

ESIA_TEST(Renderer, InstanceDataAsBufferOrTexture)
{
    DrawList dl = MakeList();
    Painter p(dl);
    for (int i = 0; i < 5; ++i)
        p.Rect(Rect(10.0f + 30.0f * (float)i, 10, 30.0f + 30.0f * (float)i, 30), Style().Fill(Color::White()).Glow(Color::White(), 4));
    const std::uint32_t feat = dl.FxInstances()[0].flags[0];
    {
        Setup s(Caps{}, Format::RGBA8_UNORM, true, 1, true);
        Renderer r(s.dev);
        r.Render(Data({&dl}), nullptr, s.target);
        ESIA_CHECK(NoErrors(s.dev));
        ESIA_CHECK(Lines(s.dev, "fx buffer #") == 1 && Lines(s.dev, "draw instanced 4 x 5") == 1);
        // the integer row goes to the GPU as float values
        const std::vector<std::uint8_t>* data = s.dev.Data(Buffer{3});
        ESIA_CHECK(data && data->size() >= 5 * sizeof(fx::Instance));
        if (data && data->size() >= 5 * sizeof(fx::Instance))
        {
            float f[4];
            std::memcpy(f, data->data() + 23 * 16, 16);
            ESIA_CHECK(f[0] == (float)feat && f[1] == 0.0f && f[2] == 0.0f);
        }
    }
    {
        Caps caps;
        caps.fxStorage = FxStorage::Texture;
        caps.maxFxDataWidth = 48;   // two instances per row
        Setup s(caps, Format::RGBA8_UNORM, true, 1, true);
        Renderer r(s.dev);
        r.Render(Data({&dl}), nullptr, s.target);
        ESIA_CHECK(NoErrors(s.dev));
        ESIA_CHECK(Lines(s.dev, "48x3 RGBA32_FLOAT usage=9 fx-instances") == 1 && Lines(s.dev, "texture t7 #") == 1);
        const std::vector<float> fc = FrameConstantsAfter(s.dev, "load ui");
        ESIA_CHECK(fc.size() == 48 && fc[46] == 2.0f);   // gConv.z: instances per row
        const std::vector<std::uint8_t>* data = s.dev.Data(Texture{3});
        ESIA_CHECK(data && data->size() == 48u * 3u * 16u);
        if (data && data->size() == 48u * 3u * 16u)
        {
            float f[4];
            std::memcpy(f, data->data() + (48 + 23) * 16, 16);   // instance 2: row 1, first slot
            ESIA_CHECK(f[0] == (float)feat);
        }
    }
}

ESIA_TEST(Renderer, IndicesAreRebased)
{
    Setup s(Caps{}, Format::RGBA8_UNORM, true, 1, true);
    DrawList a = MakeList(), b = MakeList();
    a.AddRectFilled(Rect(0, 0, 10, 10), 0xFFFFFFFFu);
    b.AddRectFilled(Rect(20, 0, 30, 10), 0xFFFFFFFFu);
    Renderer r(s.dev);
    r.Render(Data({&a, &b}), nullptr, s.target);
    ESIA_CHECK(NoErrors(s.dev));
    const std::vector<std::uint8_t>* ib = s.dev.Data(Buffer{4});
    ESIA_CHECK(ib && ib->size() >= 12 * 4);
    if (ib && ib->size() >= 12 * 4)
    {
        std::uint32_t idx[12];
        std::memcpy(idx, ib->data(), sizeof(idx));
        ESIA_CHECK(idx[0] == 0 && idx[6] == 4 && idx[8] == 6);
    }
    ESIA_CHECK(Lines(s.dev, "draw indexed 6 from 6") == 1);
}

ESIA_TEST(Renderer, CallbacksGetTheNativeStateAndStateIsRebound)
{
    Setup s;
    DrawList dl = MakeList();
    Painter p(dl);
    static int calls = 0;
    calls = 0;
    p.Rect(Rect(10, 10, 50, 50), Style().Fill(Color::White()));
    dl.AddCallback([](const DrawList&, const DrawCmd&, void* state) { calls += state == nullptr ? 1 : 100; }, nullptr);
    p.Rect(Rect(60, 10, 100, 50), Style().Fill(Color::White()));
    Renderer r(s.dev);
    r.Render(Data({&dl}), nullptr, s.target);
    ESIA_CHECK(NoErrors(s.dev));   // the second draw had everything bound again
    ESIA_CHECK(calls == 1 && Lines(s.dev, "native render state") == 1);
}

ESIA_TEST(Renderer, ProfileRunsDoNotNest)
{
    Caps caps;
    caps.timestampQueries = true;
    Setup s(caps);
    DrawList dl = MakeList();
    GlassScene(dl, 10);
    dl.AddRectFilled(Rect(0, 0, 5, 5), 0xFFFFFFFFu);
    Painter p(dl);
    p.BeginGlowLayer(Color::White(), 6);
    p.Rect(Rect(10, 10, 20, 20), Style().Fill(Color::White()));
    p.EndGlowLayer();
    Renderer r(s.dev);
    r.Render(Data({&dl}), nullptr, s.target);
    ESIA_CHECK(NoErrors(s.dev));
    ESIA_CHECK(Lines(s.dev, "profile capture") == 1 && Lines(s.dev, "profile fx-glass") == 1 && Lines(s.dev, "profile geometry") == 1 &&
               Lines(s.dev, "profile layer") == 1 && Lines(s.dev, "profile fx") >= 3);
}

ESIA_TEST(Renderer, ProjectionFollowsTheApiConventions)
{
    Caps caps;
    caps.framebufferOriginBottomLeft = true;
    caps.clipSpaceYDown = true;
    caps.halfPixelOffset = true;
    Setup s(caps);
    DrawList dl = MakeList();
    dl.AddRectFilled(Rect(0, 0, 5, 5), 0xFFFFFFFFu);
    Renderer r(s.dev);
    DrawData dd = Data({&dl});
    dd.framebufferScale = Vec2(2, 2);
    dd.displaySize = Vec2(200, 150);
    r.Render(dd, nullptr, s.target);
    ESIA_CHECK(NoErrors(s.dev));
    const std::vector<float> fc = FrameConstantsAfter(s.dev, "load ui");
    ESIA_CHECK(fc.size() == 48);
    if (fc.size() == 48)
    {
        ESIA_CHECK_NEAR(fc[0], 0.01f, 1e-7f);    // 2 * framebuffer scale / target width
        ESIA_CHECK_NEAR(fc[1], 2.0f * 2.0f / 300.0f, 1e-7f);   // +y down in clip space
        ESIA_CHECK(fc[2] == -1.0f && fc[3] == -1.0f);
        ESIA_CHECK(fc[44] == 1.0f && fc[45] == 0.5f);   // gConv: bottom-left origin, half-pixel offset
    }
}

ESIA_TEST(Renderer, UserEffectsNeedRuntimeCompilation)
{
    DrawList dl = MakeList();
    Painter p(dl);
    p.Rect(Rect(10, 10, 100, 100), Style().Fill(Color::White()).Effect(3, 1.0f));
    {
        Setup s;
        Renderer r(s.dev);
        r.SetEffectSource(3, "aurora", "float4 WgtEffect(WgtFx fx) { return fx.fill; }");
        r.Render(Data({&dl}), nullptr, s.target);
        ESIA_CHECK(NoErrors(s.dev));
        ESIA_CHECK(Lines(s.dev, "effect=3") == 0 && Lines(s.dev, "create pipeline #") >= 1);
    }
    {
        Caps caps;
        caps.runtimeEffects = true;
        Setup s(caps);
        Renderer r(s.dev);
        r.SetEffectSource(3, "aurora", "float4 WgtEffect(WgtFx fx) { return fx.fill; }");
        r.Render(Data({&dl}), nullptr, s.target);
        ESIA_CHECK(NoErrors(s.dev));
        ESIA_CHECK(Lines(s.dev, "Fx strip premul RGBA8_UNORM effect=3") == 1);
    }
}

ESIA_TEST(Renderer, ReleasesEverythingAndSkipsEmptyFrames)
{
    Setup s;
    {
        DrawList dl = MakeList();
        GlassScene(dl, 10);
        Painter p(dl);
        p.BeginGlowLayer(Color::White(), 6);
        p.Rect(Rect(10, 10, 20, 20), Style().Fill(Color::White()));
        p.EndGlowLayer();
        Renderer r(s.dev);
        r.Render(Data({&dl}), nullptr, s.target);
        s.dev.ClearLog();
        r.Render(Data({}), nullptr, s.target);   // nothing to draw: no pass
        ESIA_CHECK(Lines(s.dev, "pass ") == 0 && Lines(s.dev, "begin frame") == 1 && Lines(s.dev, "end frame") == 1);
    }
    ESIA_CHECK(NoErrors(s.dev));
    ESIA_CHECK(s.dev.LiveTextures() == 1 && s.dev.LiveBuffers() == 0 && s.dev.LivePipelines() == 0);
}

ESIA_TEST(Renderer, FxShaderVariantsPerBatchFeatures)
{
    Caps caps;
    caps.fxFeatureVariants = true;
    Setup s(caps);
    DrawList dl = MakeList();
    Painter p(dl);
    p.Rect(Rect(10, 10, 50, 50), Style().Fill(Color::White()));
    p.Rect(Rect(60, 10, 100, 50), Style().Fill(Color::White()).Shadow(Color::Black(0.5f), 8));
    p.Image(7, Rect(110, 10, 150, 50), 4);   // another texture: a batch of its own
    Renderer r(s.dev);
    r.Render(Data({&dl}), nullptr, s.target);
    ESIA_CHECK(NoErrors(s.dev));
    ESIA_CHECK(r.Plan().ops[0].features == (fx::kFill | fx::kShadow) && r.Plan().ops[1].features == (fx::kFill | fx::kImage));
    ESIA_CHECK(Lines(s.dev, "Fx strip premul RGBA8_UNORM features=0x5") == 1 && Lines(s.dev, "Fx strip premul RGBA8_UNORM features=0x41") == 1);
    s.dev.ClearLog();
    r.Render(Data({&dl}), nullptr, s.target);
    ESIA_CHECK(Lines(s.dev, "create pipeline") == 0);   // variants are cached
}
