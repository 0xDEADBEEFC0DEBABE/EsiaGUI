// WGT demo - entry point.
//
//   wgt_demo.exe [--dx11 | --dx12] [--light] [--scale 1.25] [--render-scale 2] [--size 1600x1000]
//                [--open settings,effects,control,components,languages,telemetry | --open-all]
//                [--screenshot out.png --frames 120] [--novsync] [--fps 120] [--stats] [--debug-layer]
//                [--low-power] [--debug-layout] [--text auto|gray|lcd] [--glass theme|clear|frosted] [--fixed-dt 0.016667]
//                [--script "30:move 373 329;31:down;33:up;34:text 你好;35:shot a.png"]
//
// Threads, like a game: this (window) thread creates the window and pumps its messages; a render thread owns
// the device, the WGT context and the frame loop. Input crosses over through Context::HandleWin32Message
// (thread-safe), so moving / resizing the window never stalls the UI.
//
// F1 toggles the whole UI (like a game overlay). --script + --fixed-dt give deterministic frame-by-frame
// captures (animation review, UI regression tests).
#include "host.hpp"
#include "image_io.hpp"
#include "showcase.hpp"
#include <dwmapi.h>
#include <shellapi.h>
#include <algorithm>
#include <atomic>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#pragma comment(lib, "dwmapi.lib")

namespace
{
    constexpr UINT WM_APP_RENDER_DONE = WM_APP + 1;   // render thread finished and released everything

    std::atomic<wgt::Context*> g_ui{nullptr};
    std::atomic<bool> g_quit{false};
    std::atomic<int> g_pendingW{0}, g_pendingH{0};

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (wgt::Context* ui = g_ui.load())
            if (ui->HandleWin32Message(hwnd, msg, (std::uint64_t)wParam, (std::int64_t)lParam))
                return msg == WM_SETCURSOR ? TRUE : 0;
        switch (msg)
        {
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
            {
                g_pendingH = HIWORD(lParam);
                g_pendingW = LOWORD(lParam);
            }
            return 0;
        case WM_DPICHANGED:
        {
            // per-monitor DPI: take the size Windows suggests for the new monitor (WGT rescales itself)
            const RECT* r = reinterpret_cast<const RECT*>(lParam);
            SetWindowPos(hwnd, nullptr, r->left, r->top, r->right - r->left, r->bottom - r->top, SWP_NOZORDER | SWP_NOACTIVATE);
            return 0;
        }
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU)
                return 0;
            break;
        case WM_CLOSE:
            g_quit = true;   // the render thread finishes, releases the device, then posts WM_APP_RENDER_DONE
            return 0;
        case WM_APP_RENDER_DONE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    std::wstring ExeDir()
    {
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        std::wstring s(path);
        return s.substr(0, s.find_last_of(L"\\/"));
    }

    std::string Narrow(const std::wstring& w)
    {
        int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string s(n > 0 ? n - 1 : 0, '\0');
        if (n > 0)
            WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, s.data(), n, nullptr, nullptr);
        return s;
    }

    // --script "30:move 373 329;32:down;34:up;35:shot a.png;40:shot b.png"
    // Drives the UI through Context::Inject*() (UI units = client pixels) and captures frames: used to
    // check animations frame by frame without touching the real mouse.
    struct ScriptAction
    {
        int frame = 0;
        std::wstring cmd;
        float x = 0, y = 0;
        std::wstring arg;
    };

    std::vector<ScriptAction> ParseScript(const std::wstring& s)
    {
        std::vector<ScriptAction> out;
        std::wstringstream all(s);
        std::wstring item;
        while (std::getline(all, item, L';'))
        {
            const size_t colon = item.find(L':');
            if (colon == std::wstring::npos)
                continue;
            ScriptAction a;
            a.frame = _wtoi(item.substr(0, colon).c_str());
            std::wstringstream rest(item.substr(colon + 1));
            rest >> a.cmd;
            if (a.cmd == L"move" || a.cmd == L"wheel")
                rest >> a.x >> a.y;
            else
                rest >> a.arg;
            out.push_back(a);
        }
        return out;
    }

    struct Options
    {
        bool dx12 = false, light = false, openAll = false, stats = false, debugLayout = false;
        wgt::TextAntialiasing text = wgt::TextAntialiasing::Auto;
        wgt::GlassLook glass = wgt::GlassLook::Frosted;
        float textGamma = 0.0f, textContrast = -1.0f;
        float scale = 0.0f, renderScale = 1.0f, fixedDt = 0.0f;
        int frames = 0;
        std::wstring screenshot;
        std::string openList;
        std::wstring script;
    };

    // ---------------------------------------------------------------- render thread
    void RenderMain(HWND hwnd, const Options& opt)
    {
        std::unique_ptr<IHost> host = opt.dx12 ? CreateHostD3D12() : CreateHostD3D11();
        RECT client;
        GetClientRect(hwnd, &client);
        // --render-scale N: the "game" renders at N x the window resolution (super/under-sampling); WGT detects it
        if (!host->Init(hwnd, (int)((client.right - client.left) * opt.renderScale), (int)((client.bottom - client.top) * opt.renderScale)))
        {
            MessageBoxW(hwnd, L"Failed to initialize the graphics device.", L"WGT demo", MB_ICONERROR);
            PostMessageW(hwnd, WM_APP_RENDER_DONE, 0, 0);
            return;
        }

        // The context is created on this thread: it becomes the UI thread.
        wgt::ContextDesc desc;
        host->Describe(desc);
        desc.hwnd = hwnd;
        desc.darkMode = !opt.light;
        desc.dpiScale = opt.scale;   // 0 = follow the monitor DPI
        desc.toggleKey = VK_F1;
        desc.fixedDeltaTime = opt.fixedDt;
        desc.debugLayout = opt.debugLayout;
        desc.textAntialiasing = opt.text;
        desc.textGamma = opt.textGamma;
        desc.textContrast = opt.textContrast;
        wgt::Context* ui = wgt::Context::Create(desc);
        ui->SetGlassLook(opt.glass);
        ui->SetLogCallback([](int level, const char* message) {
            OutputDebugStringA(message);
            OutputDebugStringA("\n");
            if (level >= 2)
            {
                FILE* f = nullptr;
                if (fopen_s(&f, "wgt_demo.log", "a") == 0 && f)
                {
                    std::fprintf(f, "%s\n", message);
                    std::fclose(f);
                }
            }
        });

        ShowcaseOptions so;
        so.backendName = host->Name();
        so.adapterName = HostAdapterName();
        so.openAll = opt.openAll;
        so.openList = opt.openList.empty() ? nullptr : opt.openList.c_str();
        ImageRGBA wall;
        const wchar_t* wallpapers[] = {L"C:\\Windows\\Web\\Wallpaper\\Windows\\img0.jpg", L"C:\\Windows\\Web\\4K\\Wallpaper\\Windows\\img0_1920x1200.jpg"};
        for (const wchar_t* w : wallpapers)
            if (LoadImageRGBA(w, wall, 2560))
                break;
        if (!wall.pixels.empty())
        {
            so.wallpaper = ui->CreateTexture(wall.pixels.data(), wall.width, wall.height);
            so.wallpaperWidth = wall.width;
            so.wallpaperHeight = wall.height;
        }
        ui->LoadPluginsFromDirectory((ExeDir() + L"\\plugins").c_str());
        ShowcaseInit(ui, so);
        g_ui = ui;   // the window thread starts forwarding input

        ShowcaseDisplay& display = ShowcaseDisplaySettings();
        display.tearing = HostTearingSupported();
        display.threading = "Window + render thread";
        wgt::FramePacer pacer;

        const std::vector<ScriptAction> scriptActions = ParseScript(opt.script);
        int frame = 0;
        int statFrames = 0, statDraws = 0, statFx = 0, statCaptures = 0;
        double statCpu = 0.0, statGpu = 0.0, statGlass = 0.0, statLayer = 0.0;
        float statFrameMs = 0.0f;
        while (!g_quit.load())
        {
            const int pw = g_pendingW.exchange(0), ph = g_pendingH.exchange(0);
            if (pw > 0 && ph > 0)
                host->Resize((int)(pw * opt.renderScale), (int)(ph * opt.renderScale));
            if (IsIconic(hwnd))
            {
                Sleep(16);
                continue;
            }

            // scripted input for this frame (applied by the next NewFrame)
            std::wstring shotThisFrame;
            for (const ScriptAction& a : scriptActions)
            {
                if (a.frame != frame + 1)
                    continue;
                if (a.cmd == L"move") ui->InjectMousePos(a.x, a.y);
                else if (a.cmd == L"down") ui->InjectMouseButton(0, true);
                else if (a.cmd == L"up") ui->InjectMouseButton(0, false);
                else if (a.cmd == L"wheel") ui->InjectMouseWheel(a.x);
                else if (a.cmd == L"shot") shotThisFrame = a.arg;
                else if (a.cmd == L"text") ui->InjectText(Narrow(a.arg).c_str());
            }

            // wait for a free back buffer first: input is sampled after the wait (lower latency) and the
            // wait never counts as UI time
            const float clear[4] = {0.03f, 0.03f, 0.05f, 1.0f};
            host->BeginFrame(clear);

            ui->NewFrame();
            ShowcaseBackground();
            ShowcaseFrame();
            ui->Render(host->Target());
            ++frame;
            if (!opt.screenshot.empty() && frame == opt.frames)
                host->CaptureNextFrame(opt.screenshot.c_str());
            if (!shotThisFrame.empty())
                host->CaptureNextFrame(shotThisFrame.c_str());
            host->EndFrame(display.vsync.load());

            // pacing: frame limit on top of (or instead of) VSync
            const float limit = (float)display.fpsLimit.load();
            if (limit != pacer.TargetFps())
                pacer.SetTargetFps(limit);   // re-anchors the schedule: only when the limit changes
            pacer.Wait();
            display.fps = pacer.AverageFps();
            display.frameMs = pacer.AverageFps() > 0.0f ? 1000.0f / pacer.AverageFps() : 0.0f;

            if (opt.stats && frame > 60)
            {
                const wgt::FrameStats s = ui->GetStats();
                statFrames++;
                statCpu += s.cpuUiMs;
                statFrameMs = s.frameMs;
                statDraws = s.drawCalls;
                statFx = s.fxInstances;
                statCaptures = s.backdropCaptures;
                statGpu += s.gpuMs;
                statGlass += s.gpuGlassMs;
                statLayer += s.gpuLayerMs;
            }
            if (opt.frames > 0 && frame >= opt.frames)
                break;
        }

        if (opt.stats && statFrames > 0)
        {
            FILE* f = nullptr;
            if (_wfopen_s(&f, (ExeDir() + L"\\wgt_stats.txt").c_str(), L"a") == 0 && f)
            {
                RECT cr;
                GetClientRect(hwnd, &cr);
                std::fprintf(f, "%s [%s] %ldx%ld render x%.2f vsync=%d limit=%d | frame %.2f ms (pacer %.1f fps) | UI CPU %.3f ms | UI GPU %.3f ms (glass %.3f, layers %.3f) | draws %d | shapes %d | glass captures %d\n",
                             host->Name(), HostAdapterName(), cr.right - cr.left, cr.bottom - cr.top, opt.renderScale, display.vsync.load() ? 1 : 0,
                             display.fpsLimit.load(), statFrameMs, pacer.AverageFps(), statCpu / statFrames, statGpu / statFrames, statGlass / statFrames,
                             statLayer / statFrames, statDraws, statFx, statCaptures);
                std::fclose(f);
            }
        }
        g_ui = nullptr;   // stop forwarding input before the context goes away
        ShowcaseShutdown();
        host->WaitIdle();
        if (HostDebugLayer())
        {
            const std::wstring logPath = ExeDir() + L"\\wgt_debug_layer.txt";
            const int issues = host->DumpDebugMessages(logPath.c_str());
            FILE* f = nullptr;
            if (_wfopen_s(&f, logPath.c_str(), L"a") == 0 && f)
            {
                std::fprintf(f, "== %d debug-layer warnings/errors ==\n", issues);
                std::fclose(f);
            }
        }
        ui->Destroy();
        host.reset();
        PostMessageW(hwnd, WM_APP_RENDER_DONE, 0, 0);
    }
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    // ------------------------------------------------------------- options
    Options opt;
    bool vsync = true;
    int fpsLimit = 0;
    int width = 0, height = 0;
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i = 1; i < argc; ++i)
    {
        std::wstring a = argv[i];
        auto next = [&]() -> std::wstring { return i + 1 < argc ? std::wstring(argv[++i]) : std::wstring(); };
        if (a == L"--dx12") opt.dx12 = true;
        else if (a == L"--dx11") opt.dx12 = false;
        else if (a == L"--light") opt.light = true;
        else if (a == L"--novsync") vsync = false;
        else if (a == L"--fps") fpsLimit = std::stoi(next());
        else if (a == L"--stats") opt.stats = true;
        else if (a == L"--debug-layer") SetHostDebugLayer(true);
        else if (a == L"--low-power") SetHostLowPower(true);
        else if (a == L"--open-all") opt.openAll = true;
        else if (a == L"--open") opt.openList = Narrow(next());
        else if (a == L"--script") opt.script = next();
        else if (a == L"--fixed-dt") opt.fixedDt = std::stof(next());
        else if (a == L"--debug-layout") opt.debugLayout = true;
        else if (a == L"--text-gamma") opt.textGamma = std::stof(next());
        else if (a == L"--text-contrast") opt.textContrast = std::stof(next());
        else if (a == L"--glass") { const std::wstring v = next(); opt.glass = v == L"clear" ? wgt::GlassLook::Clear : v == L"theme" ? wgt::GlassLook::Theme : wgt::GlassLook::Frosted; }
        else if (a == L"--text") { const std::wstring v = next(); opt.text = v == L"gray" ? wgt::TextAntialiasing::Grayscale : v == L"lcd" ? wgt::TextAntialiasing::Subpixel : wgt::TextAntialiasing::Auto; }
        else if (a == L"--scale") opt.scale = std::stof(next());
        else if (a == L"--render-scale") opt.renderScale = std::stof(next());
        else if (a == L"--frames") opt.frames = std::stoi(next());
        else if (a == L"--screenshot") opt.screenshot = next();
        else if (a == L"--size") { std::wstring v = next(); swscanf_s(v.c_str(), L"%dx%d", &width, &height); }
    }
    LocalFree(argv);
    if (!opt.screenshot.empty() && opt.frames <= 0)
        opt.frames = 120;
    ShowcaseDisplaySettings().vsync = vsync;
    ShowcaseDisplaySettings().fpsLimit = fpsLimit;

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // -------------------------------------------------------------- window
    WNDCLASSEXW wc = {sizeof(wc), CS_CLASSDC, WndProc, 0, 0, instance, nullptr, LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr, L"WgtDemo", nullptr};
    RegisterClassExW(&wc);
    // Default client size: 1600x1000 design units at the monitor DPI, clamped to the work area.
    POINT origin = {0, 0};
    HMONITOR monitor = MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi = {sizeof(mi)};
    GetMonitorInfoW(monitor, &mi);
    const UINT systemDpi = GetDpiForSystem();
    const float dpi = opt.scale > 0.0f ? opt.scale : systemDpi / 96.0f;
    if (width <= 0 || height <= 0)
    {
        const int workW = mi.rcWork.right - mi.rcWork.left, workH = mi.rcWork.bottom - mi.rcWork.top;
        width = std::min((int)(1600 * dpi), (int)(workW * 0.94f));
        height = std::min((int)(1000 * dpi), (int)(workH * 0.90f));
    }
    RECT rc = {0, 0, width, height};
    AdjustWindowRectExForDpi(&rc, WS_OVERLAPPEDWINDOW, FALSE, 0, systemDpi);
    const std::wstring title = std::wstring(L"Waffling Game Toolkit - WGT UI Showcase (") + (opt.dx12 ? L"Direct3D 12" : L"Direct3D 11") + L")";
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, instance, nullptr);
    BOOL darkTitle = opt.light ? FALSE : TRUE;
    DwmSetWindowAttribute(hwnd, 20 /*DWMWA_USE_IMMERSIVE_DARK_MODE*/, &darkTitle, sizeof(darkTitle));
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // ----------------------------------------------------- threads
    std::thread render(RenderMain, hwnd, std::cref(opt));
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    g_quit = true;
    render.join();
    UnregisterClassW(wc.lpszClassName, instance);
    return 0;
}
