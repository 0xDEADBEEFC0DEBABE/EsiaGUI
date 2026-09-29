// glass_window - Esia in a real window: the Win32 platform layer, one of the backends built in, liquid glass, and the
// core's windows. A smoke test of the whole path (messages -> input -> core -> renderer -> RHI -> swap chain), not
// the demo.
//
//   glass_window.exe [--api d3d9|d3d10|d3d11|d3d12|opengl|vulkan] [--size 1280x800] [--vsync on|off] [--debug]
//                    [--fixed-dt 0.016667] [--frames N] [--screenshot out.png] [--font file.ttf]...
//
//   --size          client area in UI units (pixels at 100 % scale; the window grows with the monitor's scale)
//   --frames N      quit after N frames; with --screenshot, the last one is read back from the swap chain image
//   --fixed-dt s    the UI clock advances s seconds per frame (deterministic screenshots)
//   --debug         the API's debug / validation layer; its message count is printed at exit (exit code 3 if any)
//   --font          font files, the first the main one, the others fallbacks (default: Segoe UI + Microsoft YaHei)
//
// Threads, as a game has them: the main thread creates the window and pumps its messages (window thread); a render
// thread owns the device, the Context and the frame loop. Input crosses over through Context::QueueInput, so moving
// or resizing the window never stalls rendering (esia/platform/win32.hpp, "Threading rules").
#include "host.hpp"
#include "scene.hpp"
#include "image.hpp"
#include "esia/platform/win32.hpp"
#include "esia/render/renderer.hpp"
#if defined(GLASS_TEXT)
#include "esia/text/freetype.hpp"
#endif
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

namespace
{
    namespace pw = esia::platform::win32;

    struct Options
    {
        std::string api;
        int width = 1280, height = 800;
        bool vsync = true, debug = false;
        double fixedDt = 0.0;
        int frames = 0;
        std::string screenshot;
        std::vector<std::string> fonts;
    };

    int Usage(const char* problem)
    {
        std::fprintf(stderr,
                     "glass_window: %s\n"
                     "usage: glass_window [--api %s] [--size WxH] [--vsync on|off] [--debug] [--fixed-dt seconds]\n"
                     "                    [--frames N] [--screenshot out.png] [--font file]...\n",
                     problem, glass::BuiltApis().c_str());
        return 2;
    }

#if defined(GLASS_TEXT)
    // The main font and its fallbacks (--font, or the Windows fonts that cover Latin and Chinese).
    esia::text::FontId LoadFonts(esia::text::TextSystem& text, const std::vector<std::string>& files)
    {
        std::vector<std::string> paths = files;
        if (paths.empty())
        {
            const char* windir = std::getenv("WINDIR");
            const std::string dir = std::string(windir ? windir : "C:\\Windows") + "\\Fonts\\";
            for (const char* f : {"segoeui.ttf", "msyh.ttc", "simsun.ttc"})
                paths.push_back(dir + f);
        }
        esia::text::FontId main = 0;
        for (const std::string& path : paths)
            if (const esia::text::FontId id = text.AddFontFile(path.c_str()))
            {
                if (!main)
                    main = id;
                else
                    text.AddFallback(id);
            }
        return main;
    }
#endif

    // The render thread: device, context, frame loop. Returns the exit code.
    int Render(pw::Platform& platform, const Options& opt)
    {
        std::unique_ptr<glass::Host> host = glass::CreateHost(opt.api);
        pw::FrameInfo fi = platform.Frame();
        std::string error;
        if (!host->Init(static_cast<HWND>(platform.Hwnd()), fi.width, fi.height, {opt.vsync, opt.debug}, error))
        {
            std::fprintf(stderr, "glass_window: %s: %s\n", opt.api.c_str(), error.c_str());
            return 1;
        }
        std::printf("glass_window: %s on %s, %d x %d px, scale %.2f\n", host->Name(), host->Adapter().c_str(), fi.width, fi.height, fi.scale);
        std::fflush(stdout);

        esia::ContextDesc cd;
        platform.Configure(cd);
        esia::Context ctx(cd);
        esia::text::TextSystem* text = nullptr;
        esia::text::FontId font = 0;
#if defined(GLASS_TEXT)
        std::unique_ptr<esia::text::TextSystem> textSystem = esia::text::CreateFreeTypeTextSystem(ctx.Textures());
        if (textSystem && (font = LoadFonts(*textSystem, opt.fonts)) != 0)
            text = textSystem.get();
        else
            std::fprintf(stderr, "glass_window: no font loaded (--font): labels are left out\n");
#endif
        esia::render::Renderer renderer(host->Device());
        glass::Scene scene(text, font);
        platform.SetContext(&ctx);   // input flows from here on

        using Clock = std::chrono::steady_clock;
        const Clock::time_point start = Clock::now();
        Clock::time_point fpsStart = start;
        int fpsFrames = 0, frame = 0, result = 0;
        float fps = 0.0f;
        int width = fi.width, height = fi.height;
        while (!platform.CloseRequested())
        {
            fi = platform.Frame();
            if (fi.minimized || fi.width <= 0 || fi.height <= 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));   // nothing to show
                continue;
            }
            if (fi.width != width || fi.height != height)
            {
                width = fi.width;
                height = fi.height;
                host->Resize(width, height);
            }
            if (!host->BeginFrame())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            const double now = std::chrono::duration<double>(Clock::now() - start).count();
            ctx.NewFrame(fi.Params(opt.fixedDt > 0.0 ? frame * opt.fixedDt : now));
            if (text)
                text->NewFrame({fi.scale});
            scene.Frame(ctx, {host->Name(), host->Adapter(), width, height, fi.scale, fps});
            ctx.EndFrame();
            esia::render::RenderParams rp;
            rp.frame = host->FrameParams();
            if (!renderer.Render(ctx.GetDrawData(), &ctx.Textures(), host->Target(), rp))
                std::fprintf(stderr, "glass_window: the device refused frame %d\n", frame);
            platform.ApplyRequests(ctx.Requests());

            ++frame;
            const bool last = opt.frames > 0 && frame >= opt.frames;
            std::vector<std::uint8_t> pixels;
            const bool capture = last && !opt.screenshot.empty();
            if (!host->EndFrame(capture ? &pixels : nullptr))
            {
                std::fprintf(stderr, "glass_window: reading the swap chain image back failed\n");
                result = 1;
            }
            else if (capture)
            {
                esia::testkit::Image image(width, height);
                image.rgba = std::move(pixels);
                for (std::size_t i = 3; i < image.rgba.size(); i += 4)
                    image.rgba[i] = 255;   // a window has no alpha: whatever the swap chain holds there is not shown
                if (!esia::testkit::WritePng(opt.screenshot, image, &error))
                {
                    std::fprintf(stderr, "glass_window: %s\n", error.c_str());
                    result = 1;
                }
                else
                    std::printf("glass_window: frame %d written to %s\n", frame, opt.screenshot.c_str());
            }
            ++fpsFrames;
            const double since = std::chrono::duration<double>(Clock::now() - fpsStart).count();
            if (since >= 0.5)
            {
                fps = (float)(fpsFrames / since);
                fpsFrames = 0;
                fpsStart = Clock::now();
            }
            if (last)
                break;
        }
        platform.SetContext(nullptr);   // before ctx goes away

        const std::uint32_t messages = host->ValidationMessages();
        std::printf("glass_window: %d frames, %u validation / debug-layer messages%s\n", frame, messages, opt.debug ? "" : " (--debug to enable the layer)");
        return (opt.debug && messages > 0 && result == 0) ? 3 : result;
    }

    bool Parse(int argc, char** argv, Options& o, std::string& problem)
    {
        for (int i = 1; i < argc; ++i)
        {
            const std::string a = argv[i];
            const char* value = i + 1 < argc ? argv[i + 1] : nullptr;
            const auto needs = [&]() {
                if (!value)
                    problem = a + " needs a value";
                else
                    ++i;
                return value != nullptr;
            };
            if (a == "--debug")
                o.debug = true;
            else if (a == "--api" && needs())
                o.api = value;
            else if (a == "--vsync" && needs())
                o.vsync = std::strcmp(value, "off") != 0;
            else if (a == "--size" && needs())
            {
                if (std::sscanf(value, "%dx%d", &o.width, &o.height) != 2 || o.width <= 0 || o.height <= 0)
                    problem = "--size wants WxH";
            }
            else if (a == "--fixed-dt" && needs())
                o.fixedDt = std::atof(value);
            else if (a == "--frames" && needs())
                o.frames = std::atoi(value);
            else if (a == "--screenshot" && needs())
                o.screenshot = value;
            else if (a == "--font" && needs())
                o.fonts.push_back(value);
            else if (problem.empty())
                problem = "unknown option " + a;
            if (!problem.empty())
                return false;
        }
        if (o.api.empty())
            o.api = glass::DefaultApi();
        if (!glass::CreateHost(o.api))
            problem = "--api " + o.api + " is not built in (built: " + glass::BuiltApis() + ")";
        else if (!o.screenshot.empty() && o.frames <= 0)
            o.frames = 60;
        return problem.empty();
    }
}

int main(int argc, char** argv)
{
    ::SetConsoleOutputCP(CP_UTF8);   // the text field prints what was typed (Chinese included)
    Options opt;
    std::string problem;
    if (!Parse(argc, argv, opt, problem))
        return Usage(problem.c_str());

    pw::Platform platform;
    std::string error;
    const std::unique_ptr<glass::Host> probe = glass::CreateHost(opt.api);
    if (!pw::CreateAppWindow(platform, {"Esia glass_window - " + std::string(probe->Name()), opt.width, opt.height}, &error))
    {
        std::fprintf(stderr, "glass_window: %s\n", error.c_str());
        return 1;
    }
    int result = 0;
    std::thread render([&] {
        result = Render(platform, opt);
        pw::DestroyAppWindow(platform);   // everything that rendered into the window is gone: now the window
    });
    pw::RunMessageLoop();
    render.join();
    return result;
}
