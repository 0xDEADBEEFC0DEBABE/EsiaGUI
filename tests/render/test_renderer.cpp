// Renderer on the null RHI backend: the command stream it records for each technique (captures, direct reads,
// pyramids, glow layers, text programs, instance storage, callbacks, profiling), with the null device validating
// every call against the RHI contract.
#include "esia/render/painter.hpp"
#include "esia/render/renderer.hpp"
#include "esia/rhi/null_device.hpp"
#include "esia_test.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <sstream>
#include <string>

using namespace esia;
using namespace esia::render;
using namespace esia::rhi;

// Heap allocations while g_countAllocations is set (Renderer.SteadyFramesAllocateNothing). The other forms of new and
// delete forward to these.
namespace
{
    bool g_countAllocations = false;
    long g_allocations = 0;
}

void* operator new(std::size_t size)
{
    if (g_countAllocations)
        ++g_allocations;
    if (void* p = std::malloc(size ? size : 1))
        return p;
    throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }

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

ESIA_TEST(Renderer, OnlyTheClearGlassRegionIsCopied)
{
    // a frosted card and a clear control beside it share one capture: the pyramid comes straight from the target
    // and only the control's region is copied for level 0
    Setup s;
    DrawList dl = MakeList();
    Painter p(dl);
    p.Rect(Rect(0, 0, 400, 300), Style().Fill(Paint::Linear(Color::Hex(0x203050), Color::Hex(0x805030))));
    p.Rect(Rect(20, 20, 250, 280), Style().Radius(24).Glass(Glass(12)));
    p.Circle(Vec2(330, 150), 16, Style().Glass(Glass(0)));
    Renderer r(s.dev);
    r.Render(Data({&dl}), nullptr, s.target);
    ESIA_CHECK(NoErrors(s.dev));
    ESIA_CHECK(r.Stats().backdropCaptures == 1 && r.Stats().directCaptures == 0);
    ESIA_CHECK(Lines(s.dev, "copy #1 -> ") == 1 && Lines(s.dev, "texture t0 #1") >= 1);   // the first downsample reads the target
    int copied = 0;
    for (const std::string& l : s.dev.Log())
        if (l.rfind("copy #1 -> ", 0) == 0)
        {
            int x = 0, y = 0, w = 0, h = 0;
            ESIA_CHECK(std::sscanf(l.c_str() + l.find('['), "[%d,%d %dx%d]", &x, &y, &w, &h) == 4);
            copied = w * h;
        }
    ESIA_CHECK(copied > 0 && copied < 400 * 300 / 4);   // the control's neighbourhood, not the card
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

ESIA_TEST(Renderer, TextDrawsWithTheGrayscaleProgram)
{
    // on the target and inside glow layers (RGBA16F) alike
    auto run = [](bool inLayer) {
        Setup s;
        TextureRegistry reg;
        const TextureId page = reg.Create({TextureFormat::Alpha8, 64, 64});
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
        return Lines(s.dev, inLayer ? "TextGray list straight RGBA16_FLOAT" : "TextGray list straight RGBA8_UNORM");
    };
    ESIA_CHECK(run(false) == 1);
    ESIA_CHECK(run(true) == 1);
}

ESIA_TEST(Renderer, TextureRegistryReachesTheDevice)
{
    Setup s;
    TextureRegistry reg;
    const std::uint8_t pixels[4 * 4] = {};
    const TextureId glyphs = reg.Create({TextureFormat::Alpha8, 4, 4}, pixels);
    const TextureId image = reg.Create({TextureFormat::RGBA8, 2, 2}, pixels);
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
    ESIA_CHECK(Lines(s.dev, "draw indexed 12 from 0") == 1);   // same clip and texture: one draw for both lists
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

ESIA_TEST(Renderer, UserEffectsStartCompilingAtTheNextFrame)
{
    // the first frame after SetEffectSource asks for the effect's pipeline before any shape uses it (a device compiles
    // it in hundreds of milliseconds), and a shape that does later gets that one
    Caps caps;
    caps.runtimeEffects = true;
    caps.firstDrawCompiles = true;
    Setup s(caps);
    Renderer r(s.dev);
    r.SetEffectSource(3, "aurora", "float4 WgtEffect(WgtFx fx) { return fx.fill; }");
    DrawList plain = MakeList();
    Painter pp(plain);
    pp.Rect(Rect(10, 10, 100, 100), Style().Fill(Color::White()));
    r.Render(Data({&plain}), nullptr, s.target);
    ESIA_CHECK(NoErrors(s.dev));
    ESIA_CHECK(Lines(s.dev, "Fx strip premul RGBA8_UNORM effect=3") == 1);
    // once ready it draws one instance where no pixel is written (drivers that compile at the first draw do it now),
    // then the shape's own batch draws with its own pipeline and scissor
    ESIA_CHECK(Lines(s.dev, "scissor [0,0 0x0]") == 1 && Lines(s.dev, "draw instanced 4 x 1") == 2);
    DrawList dl = MakeList();
    Painter p(dl);
    p.Rect(Rect(10, 10, 100, 100), Style().Fill(Color::White()).Effect(3, 1.0f));
    r.Render(Data({&dl}), nullptr, s.target);
    ESIA_CHECK(NoErrors(s.dev));
    ESIA_CHECK(Lines(s.dev, "Fx strip premul RGBA8_UNORM effect=3") == 1);
    ESIA_CHECK(Lines(s.dev, "scissor [0,0 0x0]") == 1);   // once

    // a device whose pipelines are complete when created: the prewarm, no warm-up draw
    Caps complete;
    complete.runtimeEffects = true;
    Setup s2(complete);
    Renderer r2(s2.dev);
    r2.SetEffectSource(3, "aurora", "float4 WgtEffect(WgtFx fx) { return fx.fill; }");
    r2.Render(Data({&plain}), nullptr, s2.target);
    ESIA_CHECK(NoErrors(s2.dev));
    ESIA_CHECK(Lines(s2.dev, "Fx strip premul RGBA8_UNORM effect=3") == 1);
    ESIA_CHECK(Lines(s2.dev, "scissor [0,0 0x0]") == 0 && Lines(s2.dev, "draw instanced 4 x 1") == 1);
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

namespace
{
    // three glass cards over content: three planned captures
    void ThreeGlassCards(DrawList& dl)
    {
        Painter p(dl);
        for (int i = 0; i < 3; ++i)
        {
            const float x = 20.0f + 120.0f * (float)i;
            p.Rect(Rect(x, 20, x + 100, 280), Style().Fill(Color::Hex(0x336699)));
            p.Rect(Rect(x + 10, 40, x + 90, 120), Style().Radius(16).Glass(Glass(10)));
        }
    }

    // the id after "#" in the first log line that contains `needle` (0 if none)
    std::uint32_t IdIn(const NullDevice& d, const std::string& needle)
    {
        for (const std::string& l : d.Log())
        {
            const std::size_t at = l.find(needle);
            if (at == std::string::npos)
                continue;
            const std::size_t hash = l.find('#');
            return hash == std::string::npos ? 0u : (std::uint32_t)std::stoul(l.substr(hash + 1));
        }
        return 0;
    }
}

ESIA_TEST(Renderer, PyramidLevelsKeepTheirContentWhenGlassReusesACapture)
{
    Setup s;
    DrawList dl = MakeList();
    ThreeGlassCards(dl);
    Renderer r(s.dev);
    RenderParams over;
    over.maxBackdropCaptures = 1;
    // past the budget, glass reuses the last capture and reads what the levels hold outside its region: new levels
    // are cleared, then loaded - never DontCare (undefined on tilers)
    r.Render(Data({&dl}), nullptr, s.target, over);
    ESIA_CHECK(NoErrors(s.dev) && r.Stats().overBudget);
    ESIA_CHECK(Lines(s.dev, "clear pyramid") == 4 && Lines(s.dev, "load pyramid") == 0 && Lines(s.dev, "dont-care pyramid") == 0);
    s.dev.ClearLog();
    r.Render(Data({&dl}), nullptr, s.target, over);
    ESIA_CHECK(Lines(s.dev, "load pyramid") == 4 && Lines(s.dev, "clear pyramid") == 0);
    // within the budget every read stays in a refreshed region: DontCare, which leaves the levels undefined ...
    s.dev.ClearLog();
    r.Render(Data({&dl}), nullptr, s.target);
    ESIA_CHECK(!r.Stats().overBudget && Lines(s.dev, "dont-care pyramid") == 12 && Lines(s.dev, "load pyramid") == 0);
    // ... so the next frame that keeps them clears them first
    s.dev.ClearLog();
    r.Render(Data({&dl}), nullptr, s.target, over);
    ESIA_CHECK(Lines(s.dev, "clear pyramid") == 4 && Lines(s.dev, "load pyramid") == 0);
    ESIA_CHECK(NoErrors(s.dev));

    // a user effect may sample the backdrop outside its margin: the same
    Caps caps;
    caps.runtimeEffects = true;
    Setup e(caps);
    DrawList fx = MakeList();
    GlassScene(fx, 10);
    Painter(fx).Rect(Rect(10, 10, 100, 100), Style().Fill(Color::White()).Effect(3, 1.0f));
    Renderer re(e.dev);
    re.SetEffectSource(3, "aurora", "float4 WgtEffect(WgtFx fx) { return float4(WgtBackdrop(fx.screenUV, 4.0), 1.0); }");
    re.Render(Data({&fx}), nullptr, e.target);
    ESIA_CHECK(NoErrors(e.dev));
    ESIA_CHECK(Lines(e.dev, "clear pyramid") >= 4 && Lines(e.dev, "dont-care pyramid") == 0);   // the effect reads every level
}

ESIA_TEST(Renderer, RefusedOrFailedVariantsFallBackToTheFullShader)
{
    Caps caps;
    caps.fxFeatureVariants = true;
    DrawList dl = MakeList();
    Painter p(dl);
    p.Rect(Rect(10, 10, 50, 50), Style().Fill(Color::White()).Shadow(Color::Black(0.5f), 8));
    p.Image(7, Rect(110, 10, 150, 50), 4);
    {
        NullOptions o{caps};
        o.refuseFxVariants = true;   // e.g. an SM3 variant over the instruction or register limits
        NullDevice dev(o);
        const Texture target = dev.CreateHostTarget(400, 300, Format::RGBA8_UNORM, true);
        Renderer r(dev);
        r.Render(Data({&dl}), nullptr, target);
        ESIA_CHECK(NoErrors(dev));
        ESIA_CHECK(Lines(dev, "refused: fx variant") == 2 && Lines(dev, "create pipeline #") == 1);   // the full shader, once
        ESIA_CHECK(Lines(dev, "draw instanced") == 2 && r.Stats().fxFallbacks == 2);   // drawn, not dropped
        dev.ClearLog();
        r.Render(Data({&dl}), nullptr, target);
        ESIA_CHECK(Lines(dev, "create pipeline") == 0 && r.Stats().fxFallbacks == 2);   // a refusal is final
    }
    {
        caps.asyncPipelines = true;
        NullOptions o{caps};
        o.failBackground = true;     // compiled in the background, and failed
        NullDevice dev(o);
        const Texture target = dev.CreateHostTarget(400, 300, Format::RGBA8_UNORM, true);
        Renderer r(dev);
        r.Render(Data({&dl}), nullptr, target);
        ESIA_CHECK(NoErrors(dev));   // a failed pipeline is never bound
        ESIA_CHECK(Lines(dev, "background") == 2 && Lines(dev, "draw instanced") == 2 && r.Stats().fxFallbacks == 2);
    }
}

ESIA_TEST(Renderer, VariantsCompilingInTheBackgroundDrawWithAReadyPipeline)
{
    Caps caps;
    caps.fxFeatureVariants = true;
    caps.asyncPipelines = true;
    NullOptions o{caps};
    o.pendingFrames = 2;
    NullDevice dev(o);
    const Texture target = dev.CreateHostTarget(400, 300, Format::RGBA8_UNORM, true);
    DrawList shadowed = MakeList();
    Painter(shadowed).Rect(Rect(10, 10, 50, 50), Style().Fill(Color::White()).Shadow(Color::Black(0.5f), 8));   // features 0x5
    Renderer r(dev);
    // frames 1 and 2: the variant compiles, the full shader draws (asked for synchronously, once)
    r.Render(Data({&shadowed}), nullptr, target);
    ESIA_CHECK(NoErrors(dev) && r.Stats().fxPendingVariants == 1 && Lines(dev, "draw instanced") == 1);
    const std::uint32_t variant = IdIn(dev, "features=0x5 background");
    ESIA_CHECK(variant != 0 && Lines(dev, "pipeline #" + std::to_string(variant)) == 1);   // created, not bound
    r.Render(Data({&shadowed}), nullptr, target);
    ESIA_CHECK(NoErrors(dev) && r.Stats().fxPendingVariants == 1);
    // frame 3: ready, bound
    dev.ClearLog();
    r.Render(Data({&shadowed}), nullptr, target);
    ESIA_CHECK(NoErrors(dev) && r.Stats().fxPendingVariants == 0 && Lines(dev, "pipeline #" + std::to_string(variant)) == 1);
    // a shape that only fills (0x1): its variant is pending, the ready 0x5 variant covers it - not the full shader
    DrawList filled = MakeList();
    Painter(filled).Rect(Rect(10, 10, 50, 50), Style().Fill(Color::White()));
    dev.ClearLog();
    r.Render(Data({&filled}), nullptr, target);
    ESIA_CHECK(NoErrors(dev) && r.Stats().fxPendingVariants == 1);
    ESIA_CHECK(Lines(dev, "features=0x1 background") == 1 && Lines(dev, "pipeline #" + std::to_string(variant)) == 1);
}

ESIA_TEST(Renderer, TargetsThatCannotBeCopied)
{
    {
        // sampleable, not copyable (a swap-chain image without TRANSFER_SRC): clear glass wants level 0, gets a
        // direct read and level 1 in its place
        Setup s;
        TextureDesc d;
        d.width = 400;
        d.height = 300;
        d.usage = TextureUsage_RenderTarget | TextureUsage_Sampled;
        const Texture target = s.dev.CreateTexture(d, nullptr, 0);
        DrawList dl = MakeList();
        GlassScene(dl, 0);
        Renderer r(s.dev);
        r.Render(Data({&dl}), nullptr, target);
        ESIA_CHECK(NoErrors(s.dev));
        ESIA_CHECK(r.Stats().backdropCaptures == 1 && r.Stats().directCaptures == 1);
        ESIA_CHECK(Lines(s.dev, "copy ") == 0 && Lines(s.dev, "backdrop-copy") == 0 && Lines(s.dev, "pyramid") >= 1);
        const std::uint32_t level1 = IdIn(s.dev, "pyramid-1");
        ESIA_CHECK(level1 != 0 && Lines(s.dev, "texture t1 #" + std::to_string(level1)) >= 1);
        const std::vector<float> fc = FrameConstantsAfter(s.dev, "load ui");
        ESIA_CHECK(fc.size() == 48 && fc[14] == 1.0f);   // backdrop valid
    }
    {
        // neither copyable nor sampleable (a framebufferOnly drawable): no backdrop at all
        Setup s;
        TextureDesc d;
        d.width = 400;
        d.height = 300;
        d.usage = TextureUsage_RenderTarget;
        const Texture target = s.dev.CreateTexture(d, nullptr, 0);
        DrawList dl = MakeList();
        GlassScene(dl, 10);
        Renderer r(s.dev);
        r.Render(Data({&dl}), nullptr, target);
        ESIA_CHECK(NoErrors(s.dev));
        ESIA_CHECK(r.Stats().backdropCaptures == 0 && Lines(s.dev, "copy ") == 0 && Lines(s.dev, "pyramid") == 0);
        const std::vector<float> fc = FrameConstantsAfter(s.dev, "load ui");
        ESIA_CHECK(fc.size() == 48 && fc[14] == 0.0f);   // gTime.z: no backdrop
    }
}

ESIA_TEST(Renderer, ProfileScopesAreBoundedOrOff)
{
    Caps caps;
    caps.timestampQueries = true;
    Setup s(caps);
    DrawList dl = MakeList();
    Painter p(dl);
    for (int i = 0; i < 40; ++i)   // geometry and FX alternating on the same pixels (no batch joins another): 80 runs
    {
        dl.AddRectFilled(Rect(5, 5, 9, 9), 0xFFFFFFFFu);
        p.Rect(Rect(5, 5, 9, 9), Style().Fill(Color::White()));
    }
    Renderer r(s.dev);
    r.Render(Data({&dl}), nullptr, s.target);
    ESIA_CHECK(NoErrors(s.dev) && Lines(s.dev, "profile ") == 32 && Lines(s.dev, "end profile") == 32);
    RenderParams params;
    params.profile = false;   // the frame total only (the backend times it by itself)
    s.dev.ClearLog();
    r.Render(Data({&dl}), nullptr, s.target, params);
    ESIA_CHECK(NoErrors(s.dev) && Lines(s.dev, "profile ") == 0);
}

ESIA_TEST(Renderer, SmallFixes)
{
    {
        // a host callback whose clip lies outside the target does not run (it would draw unclipped)
        Setup s;
        DrawList dl = MakeList();
        static int calls = 0;
        calls = 0;
        dl.PushClipRect(Rect(500, 500, 600, 600));
        dl.AddCallback([](const DrawList&, const DrawCmd&, void*) { ++calls; }, nullptr);
        dl.PopClipRect();
        Renderer r(s.dev);
        r.Render(Data({&dl}), nullptr, s.target);
        ESIA_CHECK(NoErrors(s.dev) && calls == 0 && Lines(s.dev, "native render state") == 0);
    }
    {
        // a backend without the downsample program: the pyramid stops, nothing is drawn without a pipeline
        NullOptions o;
        o.refusePrograms = 1u << (unsigned)ShaderProgram::Downsample;
        NullDevice dev(o);
        const Texture target = dev.CreateHostTarget(400, 300, Format::RGBA8_UNORM, true);
        DrawList dl = MakeList();
        GlassScene(dl, 10);
        Renderer r(dev);
        r.Render(Data({&dl}), nullptr, target);
        ESIA_CHECK(NoErrors(dev) && Lines(dev, "refused: Downsample") == 1 && Lines(dev, "dont-care pyramid") == 1);
    }
    {
        // the instance texture: whole rows, then only the used part of the last one; the row width can be capped
        Caps caps;
        caps.fxStorage = FxStorage::Texture;
        Setup s(caps, Format::RGBA8_UNORM, true, 1, true);
        DrawList dl = MakeList();
        Painter p(dl);
        for (int i = 0; i < 5; ++i)
            p.Rect(Rect(10.0f + 30.0f * (float)i, 10, 30.0f + 30.0f * (float)i, 30), Style().Fill(Color::White()));
        Renderer r(s.dev);
        RenderParams params;
        params.maxFxInstancesPerRow = 2;
        r.Render(Data({&dl}), nullptr, s.target, params);
        ESIA_CHECK(NoErrors(s.dev));
        ESIA_CHECK(Lines(s.dev, "48x4 RGBA32_FLOAT usage=9 fx-instances") == 1 || Lines(s.dev, "48x3 RGBA32_FLOAT usage=9 fx-instances") == 1);
        const std::uint32_t tex = IdIn(s.dev, "fx-instances");
        ESIA_CHECK(Lines(s.dev, "update texture #" + std::to_string(tex) + " [0,0 48x2]") == 1);
        ESIA_CHECK(Lines(s.dev, "update texture #" + std::to_string(tex) + " [0,2 24x1]") == 1);
        const std::vector<float> fc = FrameConstantsAfter(s.dev, "load ui");
        ESIA_CHECK(fc.size() == 48 && fc[46] == 2.0f);
    }
    {
        // the host's frame number reaches the backend
        Setup s;
        DrawList dl = MakeList();
        dl.AddRectFilled(Rect(0, 0, 5, 5), 0xFFFFFFFFu);
        Renderer r(s.dev);
        RenderParams params;
        params.frame.hostFrame = 7;
        r.Render(Data({&dl}), nullptr, s.target, params);
        ESIA_CHECK(NoErrors(s.dev) && Lines(s.dev, "begin frame 1 host 7") == 1);
    }
}

// Once a scene has been drawn, drawing it again allocates nothing on the heap: the frame plan, its capture planning
// and the renderer keep their storage from frame to frame.
ESIA_TEST(Renderer, SteadyFramesAllocateNothing)
{
    NullDevice dev(NullOptions{Caps{}, false});   // no log: the null device's own text lines would allocate
    const Texture target = dev.CreateHostTarget(400, 300, Format::RGBA8_UNORM, true, 1);
    DrawList dl = MakeList(), over = MakeList();
    {
        Painter p(dl);
        GlassScene(dl, 12.0f);
        for (int i = 0; i < 6; ++i)   // glass controls on the card, content between them: several captures
        {
            const float x = 70.0f + 44.0f * (float)i;
            p.Rect(Rect(x, 150, x + 36, 170), Style().Fill(Color::Hex(0x336699)));
            p.Rect(Rect(x, 80 + 10.0f * (float)i, x + 36, 116 + 10.0f * (float)i), Style().Radius(12).Glass(Glass(6)));
        }
        p.BeginEdgeFade(Rect(60, 60, 340, 240), 12.0f, 16.0f);
        p.Rect(Rect(80, 200, 320, 230), Style().Radius(6).Fill(Color::White(0.6f)));
        p.EndEdgeFade();
        p.BeginGlowLayer(Color::Hex(0x00FFFF, 0.5f), 12.0f, 1.5f);
        p.Rect(Rect(100, 250, 200, 280), Style().Fill(Color::White()).Radius(8));
        p.EndGlowLayer();
        Painter q(over);
        q.Rect(Rect(250, 20, 390, 120), Style().Radius(20).Glass(Glass(20)).Shadow(Color::Black(0.3f), 12, Vec2(0, 4)));
    }
    Renderer r(dev);
    const DrawData dd = Data({&dl, &over});
    for (int i = 0; i < 3; ++i)
        r.Render(dd, nullptr, target);
    ESIA_CHECK(r.Stats().backdropCaptures >= 2 && r.Stats().glowLayers == 1);
    g_allocations = 0;
    g_countAllocations = true;
    for (int i = 0; i < 5; ++i)
        r.Render(dd, nullptr, target);
    g_countAllocations = false;
    ESIA_CHECK(g_allocations == 0);
    if (g_allocations != 0)
        std::fprintf(stderr, "    %ld allocations in 5 frames\n", g_allocations);
    ESIA_CHECK(NoErrors(dev));
}
