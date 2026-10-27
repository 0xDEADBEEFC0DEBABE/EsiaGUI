// Esia - the renderer on fixed workloads: what a frame costs the CPU to build (Painter and text into a draw list) and
// to submit (Renderer::Render: plan, upload, commands), and what it costs the GPU where the device has timestamps.
//
//   esia_bench [--backend d3d11] [--scene all|text|widgets|widgets-flat|shapes|glass|...] [--frames 600] [--warmup 60]
//              [--size 1600x1000] [--hdr]
//
// The glass scenes: as they are, with RenderParams::targetStatic (unchanged captures reused), with a background that
// scrolls under the glass ("live"), and that with captures reused for two frames (RenderParams::backdropReuseFrames).
//
// Every frame draws the same scene at a fixed clock. The GPU finishes each frame before the next one starts (a
// one-pixel readback outside the timed parts), so a queue that fills up does not skew the CPU times. Printed per
// scene: the median and the 99th percentile of each part over the frames after the warm-up, the draw calls and what
// a frame uploads. Compare two builds by alternating their runs on the same machine, not by absolute numbers.
#include "esia/core/texture.hpp"
#include "esia/render/painter.hpp"
#include "esia/render/renderer.hpp"
#include "esia/rhi/backend_registry.hpp"
#include "esia/text/freetype.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using namespace esia;

namespace
{
    using Clock = std::chrono::steady_clock;

    double Ms(Clock::time_point a, Clock::time_point b) { return std::chrono::duration<double, std::milli>(b - a).count(); }

    // a part's times over the measured frames
    struct Series
    {
        std::vector<double> v;
        double At(double q)
        {
            if (v.empty())
                return 0.0;
            std::sort(v.begin(), v.end());
            const std::size_t i = std::min(v.size() - 1, (std::size_t)(q * (double)(v.size() - 1) + 0.5));
            return v[i];
        }
    };

    struct Bench
    {
        text::TextSystem* text = nullptr;
        text::FontId font = 0;
        float width = 0, height = 0;
        double time = 0.0;
        bool live = false;   // the glass scene's background scrolls
    };

    // Lines of the text scene: Latin, Chinese (the characters of tests/fonts' subset) and both mixed.
    const char* const kLines[] = {
        "The quick brown fox jumps over the lazy dog - 0123456789",
        "中文与英文混排的版面，测试字体与字号。",
        "Esia draws text, shapes and glass on every backend.",
        "这是一段很长的汉字文本，可以看出排版的宽度。",
        "Kerning AV To Ty, ligatures fi fl ffi, (brackets) [and] {braces}",
        "玻璃界面的文本：中文、英文与符号（括号）对齐。",
        "Mixed 中文 and English 文本 in one line, 2026",
        "日月年时间之首尾，在现行版本中可省略。",
    };

    // A page of text: two columns of 14-unit lines, headings at 24 units.
    void TextScene(Painter& p, const Bench& b)
    {
        const float lineH = 18.0f;
        const int rows = (int)((b.height - 40.0f) / lineH);
        int n = 0;
        for (int col = 0; col < 2; ++col)
            for (int row = 0; row < rows; ++row, ++n)
            {
                const Vec2 pos(20.0f + (float)col * (b.width * 0.5f), 20.0f + (float)row * lineH);
                if (row % 16 == 0)
                    p.Text(pos, text::FontRef{b.font, 24.0f}, Color::Hex(0xF2F2F7), kLines[(n / 16) % 8]);
                else
                    p.Text(pos, text::FontRef{b.font, 14.0f}, row % 3 == 0 ? Color::Hex(0x8E8E93) : Color::Hex(0xE5E5EA), kLines[n % 8]);
            }
    }

    // Controls: a capsule with a label each, in a grid - the shape of a frame of plain widgets.
    void WidgetScene(Painter& p, const Bench& b)
    {
        const char* const labels[] = {"Apply", "Cancel", "Settings", "测试", "Open file", "版本", "Volume", "OK"};
        const float w = 120.0f, h = 28.0f, gap = 8.0f;
        const int cols = (int)((b.width - gap) / (w + gap)), rows = (int)((b.height - gap) / (h + gap));
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c)
            {
                const esia::Rect box = esia::Rect::FromSize(Vec2(gap + (float)c * (w + gap), gap + (float)r * (h + gap)), Vec2(w, h));
                const bool on = (r + c) % 5 == 0;
                p.Capsule(box, Style().Fill(on ? Color::Hex(0x0A84FF) : Color::Hex(0x2C2C2E)).Stroke(1.0f, Color::White(0.12f)));
                p.TextBox(box, Vec2(0.5f, 0.5f), text::FontRef{b.font, 13.0f}, on ? Color::White() : Color::Hex(0xE5E5EA), labels[(r * cols + c) % 8]);
            }
    }

    // Ten thousand small rounded rectangles with a stroke (FX instances).
    void ShapeScene(Painter& p, const Bench& b)
    {
        const int cols = 125, rows = 80;
        const float cw = b.width / (float)cols, ch = b.height / (float)rows;
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c)
            {
                const esia::Rect box = esia::Rect::FromSize(Vec2((float)c * cw + 1.0f, (float)r * ch + 1.0f), Vec2(cw - 2.0f, ch - 2.0f));
                p.Rect(box, Style().Fill(Color::Hsv((float)((r * cols + c) % 360) / 360.0f, 0.6f, 0.9f)).Radius(3.0f).Stroke(1.0f, Color::Black(0.3f)));
            }
    }

    // Twenty frosted glass panels with a title each over colored stripes (scrolling when live).
    void GlassScene(Painter& p, const Bench& b)
    {
        const float band = b.height / 16.0f, shift = b.live ? (float)std::fmod(b.time * 90.0, (double)band) : 0.0f;
        for (int i = -1; i < 16; ++i)
            p.Rect(esia::Rect(0, band * (float)i + shift, b.width, band * (float)(i + 1) + shift), Style().Fill(Color::Hsv((float)((i + 16) % 16) / 16.0f, 0.7f, 0.8f)));
        GlassMaterial g;
        g.blur = 10.0f;
        const float w = (b.width - 60.0f) / 5.0f, h = (b.height - 50.0f) / 4.0f;
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 5; ++c)
            {
                const esia::Rect box = esia::Rect::FromSize(Vec2(10.0f + (float)c * (w + 10.0f), 10.0f + (float)r * (h + 10.0f)), Vec2(w, h));
                p.Rect(box, Style().Radius(24.0f).Glass(g));
                p.Text(box.min + Vec2(18.0f, 16.0f), text::FontRef{b.font, 17.0f}, Color::White(), kLines[(r * 5 + c) % 8]);
            }
    }

    struct SceneDef
    {
        const char* name;
        void (*draw)(Painter&, const Bench&);
        bool flat = false;
        bool live = false;
        bool targetStatic = false;   // RenderParams::targetStatic
        int reuseFrames = 0;         // RenderParams::backdropReuseFrames
    };
    const SceneDef kScenes[] = {
        {"text", &TextScene},
        {"widgets", &WidgetScene},
        {"widgets-flat", &WidgetScene, true},
        {"shapes", &ShapeScene},
        {"glass", &GlassScene},
        {"glass-static", &GlassScene, false, false, true},
        {"glass-live", &GlassScene, false, true},
        {"glass-live-2", &GlassScene, false, true, false, 2},
    };

    const char* DefaultBackend()
    {
#if defined(_WIN32)
        return "d3d11";
#elif defined(__APPLE__)
        return "metal";
#else
        return "vulkan";
#endif
    }
}

int main(int argc, char** argv)
{
    std::string backend = DefaultBackend(), scene = "all", fonts = ESIA_BENCH_FONT_DIR;
    int frames = 600, warmup = 60, width = 1600, height = 1000;
    bool hdr = false;   // an RGBA16_FLOAT target, the frame encoded as scRGB (RenderParams::hdr)
    for (int i = 1; i < argc; ++i)
    {
        const std::string a = argv[i];
        const bool more = i + 1 < argc;
        if (a == "--backend" && more)
            backend = argv[++i];
        else if (a == "--scene" && more)
            scene = argv[++i];
        else if (a == "--frames" && more)
            frames = std::max(1, std::atoi(argv[++i]));
        else if (a == "--warmup" && more)
            warmup = std::max(0, std::atoi(argv[++i]));
        else if (a == "--size" && more && std::sscanf(argv[++i], "%dx%d", &width, &height) == 2)
            ;
        else if (a == "--fonts" && more)
            fonts = argv[++i];
        else if (a == "--hdr")
            hdr = true;
        else
        {
            std::fprintf(stderr, "usage: esia_bench [--backend name] [--scene all|text|widgets|widgets-flat|shapes|glass] [--frames n] "
                                 "[--warmup n] [--size WxH] [--fonts dir] [--hdr]\n");
            return 2;
        }
    }

    // the driver's time, not the validation layer's: the headless Vulkan device turns it on (with synchronization
    // validation) unless told otherwise - set ESIA_VULKAN_VALIDATION=1 to measure with it
    if (!std::getenv("ESIA_VULKAN_VALIDATION"))
    {
#if defined(_WIN32)
        _putenv("ESIA_VULKAN_VALIDATION=0");   // the CRT keeps the string: a literal
#else
        setenv("ESIA_VULKAN_VALIDATION", "0", 1);
#endif
    }
    rhi::RegisterBuiltinBackends();
    const rhi::BackendInfo* info = rhi::FindBackend(backend);
    if (!info)
    {
        std::fprintf(stderr, "esia_bench: no backend '%s' in this build\n", backend.c_str());
        return 2;
    }
    rhi::HeadlessDesc hd;
    hd.width = width;
    hd.height = height;
    hd.format = hdr ? rhi::Format::RGBA16_FLOAT : rhi::Format::RGBA8_UNORM;
    std::string error;
    rhi::HeadlessDevice hdev = info->createHeadless(hd, error);
    if (!hdev.device)
    {
        std::fprintf(stderr, "esia_bench: %s: %s\n", backend.c_str(), error.c_str());
        return 77;
    }
    TextureRegistry textures;
    const std::unique_ptr<text::TextSystem> ts = text::CreateFreeTypeTextSystem(textures);
    if (!ts)
    {
        std::fprintf(stderr, "esia_bench: FreeType did not start\n");
        return 1;
    }
    Bench b;
    b.text = ts.get();
    b.font = ts->AddFontFile((fonts + "/DroidSans.ttf").c_str());
    if (const text::FontId cjk = ts->AddFontFile((fonts + "/NotoSansSC-Subset.otf").c_str()))
        ts->AddFallback(cjk);
    if (!b.font)
    {
        std::fprintf(stderr, "esia_bench: no DroidSans.ttf in %s\n", fonts.c_str());
        return 1;
    }
    b.width = (float)width;
    b.height = (float)height;

    const bool readback = hdev.device->GetCaps().readback;
    std::printf("esia_bench: %s on %s, %d x %d, %d frames after %d of warm-up%s\n", backend.c_str(), hdev.adapter.empty() ? "?" : hdev.adapter.c_str(),
                width, height, frames, warmup, readback ? "" : " (no readback: the GPU is not waited for)");
    std::printf("%-13s %-17s %-17s %-17s %6s %9s %9s %7s %7s %10s %9s\n", "scene", "build ms (p50/99)", "submit ms", "gpu ms", "draws", "vertices",
                "indices", "quads", "fx", "upload KB", "captures");
    for (const SceneDef& sd : kScenes)
    {
        if (scene != "all" && scene != sd.name)
            continue;
        render::Renderer renderer(*hdev.device);
        PainterEnv env;
        env.text = ts.get();
        env.flat = sd.flat;
        b.live = sd.live;
        render::RenderParams params;
        params.targetStatic = sd.targetStatic;
        params.backdropReuseFrames = sd.reuseFrames;
        if (hdr)
            params.hdr.encoding = render::HdrEncoding::ScRgb;
        int captures = 0, reused = 0, restored = 0;
        DrawList dl;
        Series build, submit, gpu;
        std::uint64_t lastGpuFrame = 0;
        std::vector<std::uint8_t> pixel;
        render::RenderStats last;
        for (int f = 0; f < warmup + frames; ++f)
        {
            const double t = (double)f / 60.0;
            b.time = t;
            const Clock::time_point t0 = Clock::now();
            ts->NewFrame({1.0f, t});
            dl.Reset(esia::Rect(0, 0, b.width, b.height));
            {
                Painter p(dl, env);
                sd.draw(p, b);
            }
            const Clock::time_point t1 = Clock::now();
            DrawData dd;
            dd.lists.push_back(&dl);
            dd.displaySize = Vec2(b.width, b.height);
            dd.time = t;
            dd.deltaTime = 1.0f / 60.0f;
            if (!renderer.Render(dd, &textures, hdev.target, params))
            {
                std::fprintf(stderr, "esia_bench: %s: the device refused frame %d\n", sd.name, f);
                return 1;
            }
            const Clock::time_point t2 = Clock::now();
            if (readback)
                hdev.device->ReadPixels(hdev.target, rhi::IRect{0, 0, 1, 1}, pixel);   // the GPU finishes the frame
            last = renderer.Stats();
            if (f < warmup)
                continue;
            build.v.push_back(Ms(t0, t1));
            submit.v.push_back(Ms(t1, t2));
            captures += last.backdropCaptures;
            reused += last.reusedCaptures;
            restored += last.restoredCaptures;
            if (last.gpu.valid && last.gpu.frame != lastGpuFrame)
            {
                gpu.v.push_back(last.gpu.totalMs);
                lastGpuFrame = last.gpu.frame;
            }
        }
        char cols[3][32];
        Series* parts[3] = {&build, &submit, &gpu};
        for (int i = 0; i < 3; ++i)
        {
            if (parts[i]->v.empty())
                std::snprintf(cols[i], sizeof(cols[i]), "-");
            else
                std::snprintf(cols[i], sizeof(cols[i]), "%.3f / %.3f", parts[i]->At(0.5), parts[i]->At(0.99));
        }
        const double kb = ((double)last.vertices * sizeof(Vertex) + (double)last.indices * 4.0 + (double)last.quads * sizeof(QuadInstance) +
                           (double)last.fxInstances * sizeof(fx::Instance)) /
                          1024.0;
        char caps[48];   // per frame: taken + reused (of them restored from their copy aside)
        std::snprintf(caps, sizeof(caps), "%.1f+%.1f(%.1f)", (double)captures / (double)frames, (double)reused / (double)frames,
                      (double)restored / (double)frames);
        std::printf("%-13s %-17s %-17s %-17s %6d %9d %9d %7d %7d %10.1f %9s\n", sd.name, cols[0], cols[1], cols[2], last.drawCalls, last.vertices,
                    last.indices, last.quads, last.fxInstances, kb, caps);
    }
    return 0;
}
