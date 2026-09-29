// WGT minimal host - Direct3D 11, one thread. The integration reference (docs/INTEGRATION.md).
//
// What a game has to do, and nothing else:
//   1. per-monitor DPI awareness, a window, a D3D11 device + swap chain (the game's own),
//   2. wgt::Context::Create with that device,
//   3. forward window messages to Context::HandleWin32Message,
//   4. each frame: render the game, then NewFrame -> widgets -> Render into the back buffer, Present,
//   5. Destroy the context before the device goes away.
//
//   wgt_minimal_d3d11.exe [--frames N]      (N > 0: exit after N frames, for smoke tests)
#include <wgt/wgt.hpp>

#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <windows.h>
#include <shellapi.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <string>
#include <thread>

using Microsoft::WRL::ComPtr;

namespace
{
    wgt::Context* g_ui = nullptr;
    int g_resizeW = 0, g_resizeH = 0;

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        // 3. input first: true = the UI captured it (a click on a window, typing into a field ...)
        if (g_ui && g_ui->HandleWin32Message(hwnd, msg, (std::uint64_t)wParam, (std::int64_t)lParam))
            return msg == WM_SETCURSOR ? TRUE : 0;
        switch (msg)
        {
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
            {
                g_resizeW = LOWORD(lParam);
                g_resizeH = HIWORD(lParam);
            }
            return 0;
        case WM_DPICHANGED:
        {
            // take the size Windows suggests for the new monitor; WGT rescales itself
            const RECT* r = reinterpret_cast<const RECT*>(lParam);
            SetWindowPos(hwnd, nullptr, r->left, r->top, r->right - r->left, r->bottom - r->top, SWP_NOZORDER | SWP_NOACTIVATE);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    // Stand-in for the game's renderer: animated colored bars, drawn with plain D3D11 (no shaders), so the
    // liquid glass above has real content to blur and refract.
    void RenderGameScene(ID3D11DeviceContext* ctx, ID3D11DeviceContext1* ctx1, ID3D11RenderTargetView* rtv, int w, int h, float t)
    {
        const float sky[4] = {0.05f, 0.07f, 0.16f, 1.0f};
        ctx->ClearRenderTargetView(rtv, sky);
        if (!ctx1)
            return;   // ClearView needs the D3D11.1 runtime
        const float colors[5][4] = {{1.0f, 0.36f, 0.42f, 1}, {1.0f, 0.62f, 0.04f, 1}, {0.19f, 0.82f, 0.35f, 1}, {0.04f, 0.52f, 1.0f, 1}, {0.75f, 0.35f, 0.95f, 1}};
        for (int i = 0; i < 5; ++i)
        {
            const float x = (0.5f + 0.42f * std::sin(t * (0.35f + 0.11f * i) + i * 1.7f)) * w;
            const LONG bw = (LONG)(w * 0.09f);
            const D3D11_RECT r = {(LONG)x - bw / 2, (LONG)(h * (0.1f + 0.16f * i)), (LONG)x + bw / 2, (LONG)(h * (0.1f + 0.16f * i) + h * 0.22f)};
            ctx1->ClearView(rtv, colors[i], &r, 1);
        }
    }
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    int maxFrames = 0;
    {
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        for (int i = 1; i + 1 < argc; ++i)
            if (std::wstring(argv[i]) == L"--frames")
                maxFrames = _wtoi(argv[i + 1]);
        LocalFree(argv);
    }

    // 1. crisp text at any DPI: WGT follows the monitor of the window (Windows would bitmap-stretch otherwise)
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    WNDCLASSEXW wc = {sizeof(wc), CS_CLASSDC, WndProc, 0, 0, instance, nullptr, LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr, L"WgtMinimal", nullptr};
    RegisterClassExW(&wc);
    const float dpi = GetDpiForSystem() / 96.0f;
    RECT rc = {0, 0, (LONG)(1280 * dpi), (LONG)(800 * dpi)};
    AdjustWindowRectExForDpi(&rc, WS_OVERLAPPEDWINDOW, FALSE, 0, GetDpiForSystem());
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"WGT minimal (Direct3D 11)", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, instance, nullptr);

    // the game's device and swap chain (flip model, BGRA8)
    RECT client;
    GetClientRect(hwnd, &client);
    int width = client.right - client.left, height = client.bottom - client.top;
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = (UINT)width;
    sd.BufferDesc.Height = (UINT)height;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;   // lets WGT build frosted glass without a copy
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain> swapChain;
    const D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, levels, 2,
                                             D3D11_SDK_VERSION, &sd, &swapChain, &device, nullptr, &context)))
        return 1;
    ComPtr<ID3D11DeviceContext1> context1;
    context.As(&context1);
    ComPtr<ID3D11RenderTargetView> rtv;
    auto createRtv = [&] {
        ComPtr<ID3D11Texture2D> back;
        swapChain->GetBuffer(0, IID_PPV_ARGS(&back));
        device->CreateRenderTargetView(back.Get(), nullptr, &rtv);
    };
    createRtv();

    // 2. the UI context: the calling thread becomes the UI thread
    wgt::ContextDesc desc;
    desc.backend = wgt::Backend::D3D11;
    desc.hwnd = hwnd;
    desc.d3d11Device = device.Get();
    desc.d3d11Context = context.Get();
    desc.darkMode = true;
    desc.toggleKey = VK_F1;   // F1 shows / hides the whole UI
    desc.showDock = false;    // no panel launcher in this sample
    wgt::Context* ui = wgt::Context::Create(desc);
    ui->SetLogCallback([](int, const char* message) {
        OutputDebugStringA(message);
        OutputDebugStringA("\n");
    });
    g_ui = ui;
    ShowWindow(hwnd, SW_SHOWDEFAULT);

    // Game-side state shared with the UI. Property<T> is safe to read / write from any thread.
    wgt::Property<float> download(0.0f);
    std::atomic<bool> quit{false};
    std::thread worker([&] {
        // a background job (asset streaming, matchmaking ...) reporting to the UI from its own thread
        while (!quit && download.Get() < 1.0f)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            download.Update([](float& v) { v = std::min(v + 0.01f, 1.0f); });
        }
        if (!quit)
        {
            wgt::Notification n;
            n.title = "Download complete";
            n.message = "Texture pack installed";
            n.icon = wgt::icons::Download;
            ui->Notify(n);   // any thread
        }
    });

    bool open = true, vsync = true, hdr = false;
    float brightness = 70.0f;
    int quality = 1;
    const char* qualities[] = {"Low", "Medium", "High"};
    const auto start = std::chrono::steady_clock::now();
    int frame = 0;
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            continue;
        }
        if (g_resizeW > 0 && g_resizeH > 0)
        {
            // WGT holds no reference to the back buffer: resize exactly as without it
            rtv.Reset();
            context->ClearState();
            swapChain->ResizeBuffers(0, (UINT)g_resizeW, (UINT)g_resizeH, DXGI_FORMAT_UNKNOWN, 0);
            width = g_resizeW;
            height = g_resizeH;
            g_resizeW = g_resizeH = 0;
            createRtv();
        }

        // 4a. the game renders first ...
        const float t = std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();
        RenderGameScene(context.Get(), context1.Get(), rtv.Get(), width, height, t);
        // ... and only uses the input the UI did not take:
        //     if (!ui->WantsMouse()) game.OnMouse(...);   if (!ui->WantsKeyboard()) game.OnKeys(...);

        // 4b. the UI: widgets are immediate mode, called every frame between NewFrame and Render
        ui->NewFrame();
        if (open)
        {
            wgt::ui::WindowOptions wo;
            wo.size = wgt::Vec2(380, 520);
            wo.pos = wgt::Vec2(40, 40);
            wo.icon = wgt::icons::Settings;
            if (wgt::ui::BeginWindow("Settings", &open, wo))
            {
                if (wgt::ui::BeginSection("Display"))
                {
                    wgt::ui::RowToggle("VSync", &vsync, {wgt::icons::Refresh, wgt::Color::Hex(0x30D158)});
                    wgt::ui::RowToggle("HDR", &hdr, {wgt::icons::Lightbulb, wgt::Color::Hex(0xFF9F0A)});
                    wgt::ui::RowSlider("Brightness", &brightness, 0, 100, {wgt::icons::Brightness, wgt::Color::Hex(0x0A84FF)});
                    wgt::ui::RowSegmented("Quality", &quality, qualities, 3, {wgt::icons::Game, wgt::Color::Hex(0xBF5AF2)});
                    wgt::ui::EndSection();
                }
                if (wgt::ui::BeginSection("Downloads"))
                {
                    wgt::ui::RowValue("Texture pack", download.Get() < 1.0f ? "Downloading" : "Installed", {wgt::icons::Download});
                    wgt::ui::EndSection();
                }
                wgt::ui::ProgressBar(download.Get());
                wgt::ui::Spacer();

                if (wgt::ui::BeginFlow("actions"))   // buttons wrap like text when the window is narrow
                {
                    wgt::ui::ButtonOptions glass;
                    glass.kind = wgt::ui::ButtonKind::Glass;
                    glass.icon = wgt::icons::Bell;
                    if (wgt::ui::Button("Notify", glass))
                    {
                        wgt::Notification n;
                        n.title = "Hello from WGT";
                        n.message = "Notifications can be posted from any thread";
                        n.icon = wgt::icons::Info;
                        ui->Notify(n);
                    }
                    wgt::ui::ButtonOptions neon;
                    neon.glow = true;
                    neon.tint = wgt::Color::Hex(0xFF375F);
                    wgt::ui::Button("Play", neon);
                    wgt::ui::EndFlow();
                }
                wgt::ui::EndWindow();
            }
        }
        ui->Render(wgt::RenderTarget::D3D11(rtv.Get()));   // 4c. composited over the game's frame

        swapChain->Present(vsync ? 1 : 0, 0);
        if (maxFrames > 0 && ++frame >= maxFrames)
            DestroyWindow(hwnd);
    }

    // 5. shutdown: stop threads that talk to the UI, destroy the context, then release the device
    quit = true;
    worker.join();
    g_ui = nullptr;
    ui->Destroy();
    return 0;
}
