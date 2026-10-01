// glass_window - the examples' frame on Linux (app.hpp): the X11 platform layer's window (esia/platform/x11.hpp; on a
// Wayland desktop through XWayland), its events and the frame loop on one thread, OpenGL through EGL
// (host_opengl_egl.cpp) or Vulkan through VK_KHR_xlib_surface (host_vulkan.cpp). esia::ui's icons come from the
// desktop's icon theme (icons_linux.cpp).
#include "app.hpp"
#include "host.hpp"
#include "icons_linux.hpp"
#include "image.hpp"
#include "symbol_text.hpp"
#include "esia/platform/x11.hpp"
#include "esia/render/renderer.hpp"
#if defined(GLASS_TEXT)
#include "esia/text/freetype.hpp"
#include "esia/text/system_fonts.hpp"
#endif
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <thread>

namespace glass
{
    namespace
    {
        struct Options
        {
            std::string api;
            float width = 1280.0f, height = 800.0f;   // UI units (fractions: a pixel size at a --scale)
            float scale = 0.0f;   // pixels per UI unit (0 = the desktop's)
            bool vsync = true, debug = false;
            double fixedDt = 0.0;
            int frames = 0;
            std::string screenshot;
            std::vector<std::string> fonts;
        };

        int Usage(const App& app, const char* problem)
        {
            std::fprintf(stderr,
                         "%s: %s\n"
                         "usage: %s [--api %s] [--size WxH] [--scale s] [--vsync on|off] [--debug] [--fixed-dt seconds]\n"
                         "       [--frames N] [--screenshot out.png] [--font file]...\n",
                         app.name, problem, app.name, BuiltApis().c_str());
            return 2;
        }

        bool Parse(App& app, int argc, char** argv, Options& o, std::string& problem)
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
                bool usedValue = false;
                if (a == "--debug")
                    o.debug = true;
                else if (a == "--api" && needs())
                    o.api = value;
                else if (a == "--vsync" && needs())
                    o.vsync = std::strcmp(value, "off") != 0;
                else if (a == "--size" && needs())
                {
                    if (std::sscanf(value, "%fx%f", &o.width, &o.height) != 2 || !(o.width > 0.0f) || !(o.height > 0.0f))
                        problem = "--size wants WxH";
                }
                else if (a == "--scale" && needs())
                {
                    o.scale = (float)std::atof(value);
                    if (!(o.scale > 0.0f))
                        problem = "--scale wants a number above 0";
                }
                else if (a == "--fixed-dt" && needs())
                    o.fixedDt = std::atof(value);
                else if (a == "--frames" && needs())
                    o.frames = std::atoi(value);
                else if (a == "--screenshot" && needs())
                    o.screenshot = value;
                else if (a == "--font" && needs())
                    o.fonts.push_back(value);
                else if (app.option && app.option(a, value, usedValue))
                    i += usedValue ? 1 : 0;
                else if (problem.empty())
                    problem = "unknown option " + a;
                if (!problem.empty())
                    return false;
            }
            if (o.api.empty())
                o.api = DefaultApi();
            if (!CreateHost(o.api))
                problem = "--api " + o.api + " is not built in (built: " + BuiltApis() + ")";
            else if (!o.screenshot.empty() && o.frames <= 0)
                o.frames = 60;
            return problem.empty();
        }

#if defined(GLASS_TEXT)
        // The main font and its fallbacks: --font, or the system's chain (Noto Sans, Noto Sans CJK, DejaVu Sans ...).
        esia::text::FontId LoadFonts(esia::text::TextSystem& text, const std::vector<std::string>& files)
        {
            if (files.empty())
            {
                const std::vector<esia::text::FontId> chain = esia::text::AddFallbackFonts(text, esia::text::FindDefaultFallbackFonts());
                return chain.empty() ? 0 : chain.front();
            }
            esia::text::FontId main = 0;
            for (const std::string& path : files)
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


        int Run(App& app, esia::platform::x11::Platform& window, const Options& opt)
        {
            std::string error;
            std::unique_ptr<Host> host = CreateHost(opt.api);
            NativeWindow native;
            native.display = window.Display();
            native.window = window.Window();
            esia::platform::x11::FrameInfo fi = window.Frame();
            int width = fi.width, height = fi.height;
            if (!host || !host->Init(native, width, height, {opt.vsync, opt.debug}, error))
            {
                std::fprintf(stderr, "%s: %s: %s\n", app.name, opt.api.c_str(), error.c_str());
                return 1;
            }
            std::printf("%s: %s on %s, %d x %d px, scale %.2f\n", app.name, host->Name(), host->Adapter().c_str(), width, height, fi.scale);
            std::fflush(stdout);

            esia::ContextDesc cd;
            window.Configure(cd);
            esia::Context ctx(cd);
            esia::text::TextSystem* text = nullptr;
            esia::text::FontId font = 0;
#if defined(GLASS_TEXT)
            std::unique_ptr<esia::text::TextSystem> textSystem;   // FreeType, and the icon theme for kSystemSymbolsFont
            if (std::unique_ptr<esia::text::TextSystem> ft = esia::text::CreateFreeTypeTextSystem(ctx.Textures()))
                textSystem = std::make_unique<SymbolTextSystem>(std::move(ft), ctx.Textures(), &linux_icons::Rasterize);
            if (textSystem && !app.loadFonts)
                text = textSystem.get();
            else if (textSystem && (font = LoadFonts(*textSystem, opt.fonts)) != 0)
                text = textSystem.get();
            else
                std::fprintf(stderr, "%s: no font loaded (--font): labels are left out\n", app.name);
#endif
            esia::render::Renderer renderer(host->Device());
            if (app.init)
                app.init(ctx, text, font, opt.fonts);
            if (app.renderer)
                app.renderer(renderer);
            window.SetContext(&ctx);   // input flows from here on

            using Clock = std::chrono::steady_clock;
            const Clock::time_point start = Clock::now();
            Clock::time_point fpsStart = start;
            int fpsFrames = 0, frame = 0, result = 0;
            float fps = 0.0f, cpuMs = 0.0f;
            const float scale = fi.scale;
            while (window.Pump(fi.minimized))   // minimized: wait for the next event
            {
                fi = window.Frame();
                if (fi.minimized)
                    continue;
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
                // fixed steps: frame N (from 1) is at N x dt (captures at the same moment)
                ctx.NewFrame(fi.Params(opt.fixedDt > 0.0 ? (frame + 1) * opt.fixedDt : now));
                if (text && app.loadFonts)
                    text->NewFrame({scale});
                const Clock::time_point uiStart = Clock::now();
                if (app.frame)
                    app.frame(ctx, {host->Name(), host->Adapter(), width, height, scale, fps, cpuMs, &renderer.Stats()});
                ctx.EndFrame();
                cpuMs = std::chrono::duration<float, std::milli>(Clock::now() - uiStart).count();
                esia::render::RenderParams rp;
                rp.frame = host->Frame();
                if (!renderer.Render(ctx.GetDrawData(), &ctx.Textures(), host->Target(), rp))
                    std::fprintf(stderr, "%s: the device refused frame %d\n", app.name, frame);
                window.ApplyRequests(ctx.Requests());

                ++frame;
                const bool last = opt.frames > 0 && frame >= opt.frames;
                std::vector<std::uint8_t> pixels;
                const bool capture = last && !opt.screenshot.empty();
                if (!host->EndFrame(capture ? &pixels : nullptr))
                {
                    std::fprintf(stderr, "%s: reading the window's image back failed\n", app.name);
                    result = 1;
                }
                else if (capture)
                {
                    esia::testkit::Image image(width, height);
                    image.rgba = std::move(pixels);
                    for (std::size_t i = 3; i < image.rgba.size(); i += 4)
                        image.rgba[i] = 255;   // a window has no alpha
                    if (!esia::testkit::WritePng(opt.screenshot, image, &error))
                    {
                        std::fprintf(stderr, "%s: %s\n", app.name, error.c_str());
                        result = 1;
                    }
                    else
                        std::printf("%s: frame %d written to %s\n", app.name, frame, opt.screenshot.c_str());
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
            window.SetContext(nullptr);   // before ctx goes away
            if (app.shutdown)
                app.shutdown();

            const std::uint32_t messages = host->ValidationMessages();
            std::printf("%s: %d frames, %u validation / debug-layer messages%s\n", app.name, frame, messages, opt.debug ? "" : " (--debug to enable the layer)");
            return (opt.debug && messages > 0 && result == 0) ? 3 : result;
        }
    }

    int RunApp(int argc, char** argv, App& app)
    {
        Options opt;
        std::string problem;
        if (!Parse(app, argc, argv, opt, problem))
            return Usage(app, problem.c_str());
        esia::platform::x11::Platform window;
        std::string error;
        esia::platform::x11::WindowDesc wd;
        wd.title = "Esia " + std::string(app.name) + " - " + CreateHost(opt.api)->Name();
        wd.width = opt.width;
        wd.height = opt.height;
        wd.scale = opt.scale;
        wd.appClass = app.name;
        if (!window.CreateAppWindow(wd, &error))
        {
            std::fprintf(stderr, "%s: %s\n", app.name, error.c_str());
            return 1;
        }
        return Run(app, window, opt);   // the host goes before the window
    }
}
