// WGT UI - Context implementation: lifecycle, frame loop, thread-safe services.
#include "core/context_impl.hpp"
#include "backends/backends.hpp"
#include "imgui_impl_win32.h"

#include <cstdarg>
#include <cstdio>
#include <imm.h>

#pragma comment(lib, "imm32.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Thread-local ImGui context (see imconfig_wgt.h)
thread_local ImGuiContext* WgtImGuiTLS = nullptr;

namespace wgt
{
    namespace
    {
        thread_local Context* tlsContext = nullptr;

        bool IsQueuedInputMessage(UINT msg)
        {
            switch (msg)
            {
            case WM_MOUSEMOVE: case WM_NCMOUSEMOVE: case WM_MOUSELEAVE: case WM_NCMOUSELEAVE:
            case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK: case WM_LBUTTONUP:
            case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK: case WM_RBUTTONUP:
            case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK: case WM_MBUTTONUP:
            case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK: case WM_XBUTTONUP:
            case WM_MOUSEWHEEL: case WM_MOUSEHWHEEL:
            case WM_KEYDOWN: case WM_KEYUP: case WM_SYSKEYDOWN: case WM_SYSKEYUP:
            case WM_SETFOCUS: case WM_KILLFOCUS: case WM_CHAR: case WM_INPUTLANGCHANGE: case WM_DEVICECHANGE:
                return true;
            default:
                return false;
            }
        }

        bool IsMouseMessage(UINT msg)
        {
            return (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST) || msg == WM_NCMOUSEMOVE;
        }

        double Seconds(LARGE_INTEGER a, LARGE_INTEGER b, LARGE_INTEGER freq)
        {
            return double(b.QuadPart - a.QuadPart) / double(freq.QuadPart);
        }
    }

    Context* GetCurrentContext() { return tlsContext; }

    void SetCurrentContext(Context* ctx)
    {
        tlsContext = ctx;
        ImGui::SetCurrentContext(ctx ? ctx->GetImpl()->imgui : nullptr);
    }

    Context::Impl* CurrentImpl() { return tlsContext ? tlsContext->GetImpl() : nullptr; }

    Context::Impl& RequireImpl()
    {
        Context::Impl* impl = CurrentImpl();
        IM_ASSERT(impl != nullptr && "No current WGT context on this thread: call Context::NewFrame() first.");
        return *impl;
    }

    const char* GetVersionString() { return WGT_VERSION_STRING; }
    std::uint32_t GetAbiVersion() { return WGT_ABI_VERSION; }

    const Theme& CurrentTheme() { return RequireImpl().theme.current; }

    // ================================================================ Impl
    void Context::Impl::Log(int level, const char* fmt, ...)
    {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        std::vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        std::lock_guard lock(logMutex);
        if (log)
            log(level, buf);
        else
        {
            OutputDebugStringA(buf);
            OutputDebugStringA("\n");
        }
    }

    // ================================================================= IME
    void Context::Impl::ReadImeComposition(HWND hwnd, LPARAM flags)
    {
        HIMC himc = ImmGetContext(hwnd);
        if (!himc)
            return;
        auto read = [&](DWORD kind) {
            const LONG bytes = ImmGetCompositionStringW(himc, kind, nullptr, 0);
            std::wstring s(bytes > 0 ? (size_t)bytes / sizeof(wchar_t) : 0, L'\0');
            if (bytes > 0)
                ImmGetCompositionStringW(himc, kind, s.data(), (DWORD)bytes);
            return s;
        };
        {
            std::lock_guard lock(imeMutex);
            if (flags & GCS_RESULTSTR)
                imeCommitted += read(GCS_RESULTSTR);
            if (flags & GCS_COMPSTR)
            {
                imeComposition = read(GCS_COMPSTR);
                imeCursor = (flags & GCS_CURSORPOS) ? (int)LOWORD(ImmGetCompositionStringW(himc, GCS_CURSORPOS, nullptr, 0)) : (int)imeComposition.size();
            }
            else if (flags & GCS_RESULTSTR)
            {
                imeComposition.clear();
                imeCursor = 0;
            }
        }
        ImmReleaseContext(hwnd, himc);
    }

    void Context::Impl::PlaceImeWindows(HWND hwnd)
    {
        HIMC himc = ImmGetContext(hwnd);
        if (!himc)
            return;
        const LONG x = imeCaretX.load(), y = imeCaretY.load(), h = imeCaretH.load();
        COMPOSITIONFORM cf = {};
        cf.dwStyle = CFS_FORCE_POSITION;
        cf.ptCurrentPos = {x, y};
        ImmSetCompositionWindow(himc, &cf);
        CANDIDATEFORM cand = {};
        cand.dwIndex = 0;
        cand.dwStyle = CFS_EXCLUDE;   // candidates open next to the caret line, never on top of it
        cand.ptCurrentPos = {x, y + h};
        cand.rcArea = {x - 1, y, x + 1, y + h};
        ImmSetCandidateWindow(himc, &cand);
        ImmReleaseContext(hwnd, himc);
    }

    void Context::Impl::SetImeFocus(ImGuiID field, Vec2 caret, float lineHeight)
    {
        imeOwner = field;
        imeCaretX = (int)std::floor(caret.x);
        imeCaretY = (int)std::floor(caret.y);
        imeCaretH = (int)std::ceil(lineHeight);
        imeField = true;
    }

    void Context::Impl::ReleaseIme()
    {
        imeOwner = 0;
        imeField = false;
        std::lock_guard lock(imeMutex);
        imeComposition.clear();
        imeCommitted.clear();
        imeCursor = 0;
    }

    std::wstring Context::Impl::TakeImeCommitted()
    {
        std::lock_guard lock(imeMutex);
        std::wstring s;
        s.swap(imeCommitted);
        return s;
    }

    void Context::Impl::GetImeComposition(std::wstring& text, int& cursor)
    {
        std::lock_guard lock(imeMutex);
        text = imeComposition;
        cursor = std::clamp(imeCursor, 0, (int)imeComposition.size());
    }

    void Context::Impl::ProcessInput()
    {
        {
            std::lock_guard lock(inputMutex);
            inputScratch.swap(inputQueue);
            inputKeysScratch.swap(inputKeys);
            inputKeys.clear();
            frameKeys = latestKeys;
        }
        const bool isVisible = visible.load();
        for (const Win32Message& m : inputScratch)
        {
            // the key state as it was on the window thread when this message arrived
            ImGui_ImplWin32_SetKeyState(m.keys >= 0 ? inputKeysScratch[(size_t)m.keys].data() : frameKeys.data());
            if (!isVisible)
            {
                // Hidden: only release keys / focus so ImGui never keeps stale "held" state.
                if (!(m.msg == WM_KEYUP || m.msg == WM_SYSKEYUP || m.msg == WM_KILLFOCUS || m.msg == WM_LBUTTONUP ||
                      m.msg == WM_RBUTTONUP || m.msg == WM_MBUTTONUP))
                    continue;
            }
            ImGui_ImplWin32_WndProcHandler(m.hwnd, m.msg, m.wparam, m.lparam);
        }
        ImGui_ImplWin32_SetKeyState(frameKeys.data());   // until the backend's NewFrame (key release workarounds)
        inputScratch.clear();
        inputKeysScratch.clear();
    }

    void Context::Impl::RunTasks()
    {
        {
            std::lock_guard lock(taskMutex);
            taskScratch.swap(tasks);
        }
        for (Task& t : taskScratch)
            if (t)
                t();
        taskScratch.clear();
    }

    void Context::Impl::AttachPendingPlugins()
    {
        std::vector<IPlugin*> toAttach;
        {
            std::lock_guard lock(pluginMutex);
            for (PluginEntry& p : plugins)
                if (!p.attached)
                {
                    p.attached = true;
                    toAttach.push_back(p.plugin);
                }
        }
        for (IPlugin* p : toAttach)
        {
            Log(0, "WGT: attaching plugin '%s'", p->Name());
            p->OnAttach(*self);
        }
    }

    void Context::Impl::ApplyScaleAndTheme()
    {
        {
            std::lock_guard lock(themeMutex);
            if (pendingThemeValid)
            {
                theme.Set(pendingTheme, pendingThemeAnimate);
                pendingThemeValid = false;
            }
        }
        // Per-monitor DPI: refreshed immediately on WM_DPICHANGED, polled as a fallback (window moved
        // between monitors without us seeing the message, e.g. when the host owns the window procedure).
        if (hwnd && (dpiDirty.exchange(false) || (frameIndex % 60) == 1))
            dpiScale = ImGui_ImplWin32_GetDpiScaleForHwnd(hwnd);
        const float dpi = dpiOverride.load() > 0.0f ? dpiOverride.load() : dpiScale;
        const float user = std::clamp(userScale.load(), 0.25f, 4.0f);
        const float scale = std::round(dpi * user * 1000.0f) / 1000.0f;

        // Keep window layout proportional when the scale changes at runtime (DPI change / user zoom).
        if (appliedScale > 0.0f && std::fabs(scale - appliedScale) > 1e-4f)
        {
            ImGuiViewportP* vp = (ImGuiViewportP*)ImGui::GetMainViewport();
            ImGui::ScaleWindowsInViewport(vp, scale / appliedScale);
            Log(0, "WGT: UI scale %.2f -> %.2f", appliedScale, scale);
        }
        theme.from.metrics.scale = scale;
        theme.to.metrics.scale = scale;
        theme.current.metrics.scale = scale;
        theme.Step(dt);
        theme.current.metrics.scale = scale;
        appliedScale = scale;
        ApplyThemeToImGuiStyle(theme.current, ImGui::GetStyle());

        std::lock_guard lock(scaleInfoMutex);
        scaleInfo.dpi = dpi;
        scaleInfo.user = user;
        scaleInfo.metrics = scale;
    }

    void Context::Impl::ApplyRenderScale()
    {
        // Render scale = render-target pixels per UI unit. Dear ImGui uses it (io.DisplayFramebufferScale)
        // as the font rasterizer density, so glyphs are baked at the resolution they are displayed at:
        // a 4K target behind a 1080p window gets 2x-density text, a 720p target gets 0.5x (no wasted atlas).
        ImGuiIO& io = ImGui::GetIO();
        const int tw = targetW.load(), th = targetH.load();
        float render = renderOverride.load();
        if (render <= 0.0f)
        {
            render = 1.0f;
            if (hwnd && tw > 0 && th > 0 && io.DisplaySize.x > 0.0f && io.DisplaySize.y > 0.0f)
            {
                const float rx = (float)tw / io.DisplaySize.x;
                const float ry = (float)th / io.DisplaySize.y;
                render = std::sqrt(rx * ry);
                if (std::fabs(render - 1.0f) < 0.02f)   // rounding of odd window sizes
                    render = 1.0f;
            }
        }
        render = std::clamp(render, 0.25f, 8.0f);
        if (!hwnd && tw > 0 && th > 0)
            io.DisplaySize = Vec2((float)tw / render, (float)th / render);
        io.DisplayFramebufferScale = Vec2(render, render);
        renderScale = render;
        std::lock_guard lock(scaleInfoMutex);
        scaleInfo.render = render;
    }

    void Context::Impl::ApplyInjectedInput()
    {
        std::vector<InjectedEvent> events;
        {
            std::lock_guard lock(injectMutex);
            events.swap(injected);
        }
        ImGuiIO& io = ImGui::GetIO();
        bool mouseInjected = injectActive;
        for (const InjectedEvent& e : events)
            mouseInjected |= e.kind == InjectedEvent::Pos || e.kind == InjectedEvent::Button || e.kind == InjectedEvent::Wheel;
        if (mouseInjected)
        {
            // While the mouse is injected, the OS cursor the Win32 backend just polled must not compete with the
            // injected one (event trickling would otherwise apply a wheel / click at the real cursor position).
            ImGuiContext& g = *ImGui::GetCurrentContext();
            for (int i = g.InputEventsQueue.Size - 1; i >= 0; --i)
                if (g.InputEventsQueue[i].Type == ImGuiInputEventType_MousePos)
                    g.InputEventsQueue.erase(g.InputEventsQueue.Data + i);
        }
        bool posSent = false;
        for (const InjectedEvent& e : events)
        {
            switch (e.kind)
            {
            case InjectedEvent::Pos:
                injectActive = true;
                injectPos = Vec2(e.x, e.y);
                io.AddMousePosEvent(e.x, e.y);
                posSent = true;
                break;
            case InjectedEvent::Button:
                injectActive = true;
                if (!posSent)
                {
                    io.AddMousePosEvent(injectPos.x, injectPos.y);
                    posSent = true;
                }
                io.AddMouseButtonEvent(e.button, e.down);
                break;
            case InjectedEvent::Wheel: io.AddMouseWheelEvent(e.x, e.y); break;
            case InjectedEvent::Key: io.AddKeyEvent(e.key, e.down); break;
            case InjectedEvent::Text: io.AddInputCharactersUTF8(e.text.c_str()); break;
            }
        }
        // keep overriding the OS cursor (the Win32 backend polls it when the window is focused)
        if (injectActive && !posSent)
            io.AddMousePosEvent(injectPos.x, injectPos.y);
    }

    void Context::Impl::SyncFontDensity()
    {
        // PushFont() updates the density of the pushed font only. Fonts that are drawn directly through
        // ImDrawList::AddText() (Painter text, icons) must follow the current density too.
        const float density = ImGui::GetFontRasterizerDensity();
        for (ImFont* f : ImGui::GetIO().Fonts->Fonts)
            f->CurrentRasterizerDensity = density;
    }

    void Context::Impl::DrawPanels()
    {
        std::vector<std::shared_ptr<PanelEntry>> snapshot;
        {
            std::shared_lock lock(panelMutex);
            snapshot = panels;
        }
        for (auto& p : snapshot)
        {
            bool open = p->open.load();
            if (!open && !p->wasOpen)
                continue;
            ui::WindowOptions wo;
            wo.size = p->desc.size;
            wo.pos = p->desc.pos;
            wo.icon = p->desc.icon;
            if (p->desc.flags & PanelFlags_NoClose) wo.flags |= ui::WindowFlags_NoClose;
            if (p->desc.flags & PanelFlags_NoResize) wo.flags |= ui::WindowFlags_NoResize;
            if (p->desc.flags & PanelFlags_Solid) wo.flags |= ui::WindowFlags_Solid;
            if (p->desc.flags & PanelFlags_ClearGlass) wo.flags |= ui::WindowFlags_ClearGlass;
            if (p->desc.flags & PanelFlags_NoScroll) wo.flags |= ui::WindowFlags_NoScroll;
            if (p->focusRequest.exchange(false))
                ImGui::SetNextWindowFocus();
            bool keepOpen = open;
            ImGui::PushID(p->id.c_str());
            if (ui::BeginWindow(p->title.c_str(), &keepOpen, wo))
            {
                if (p->draw)
                    p->draw(*self);
                ui::EndWindow();
            }
            ImGui::PopID();
            if (!keepOpen && open)
                p->open.store(false);
            p->wasOpen = keepOpen;
        }
    }

    // ============================================================ Context
    Context::Context() = default;
    Context::~Context() = default;

    Context* Context::Create(const ContextDesc& desc)
    {
        Context* ctx = new Context();
        Impl* m = new Impl();
        ctx->impl_ = m;
        m->self = ctx;
        m->desc = desc;
        m->debugLayout = desc.debugLayout;
        m->textAntialiasing = (int)desc.textAntialiasing;
        m->hwnd = static_cast<HWND>(desc.hwnd);
        m->owner = std::this_thread::get_id();
        m->iniFilename = desc.iniFilename ? desc.iniFilename : "";
        m->darkMode = desc.darkMode;
        m->accentOverride = desc.accent;
        m->dockVisible = desc.showDock;
        m->userScale = desc.uiScale > 0.0f ? desc.uiScale : 1.0f;
        m->dpiOverride = desc.dpiScale;
        m->renderOverride = desc.renderScale;
        QueryPerformanceFrequency(&m->qpcFreq);
        QueryPerformanceCounter(&m->qpcLast);

        IMGUI_CHECKVERSION();
        m->imgui = ImGui::CreateContext();
        SetCurrentContext(ctx);

        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = m->iniFilename.empty() ? nullptr : m->iniFilename.c_str();
        io.LogFilename = nullptr;
        io.ConfigWindowsMoveFromTitleBarOnly = true;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigDragClickToInputText = true;

        if (m->hwnd)
        {
            ImGui_ImplWin32_Init(m->hwnd);
            m->dpiScale = ImGui_ImplWin32_GetDpiScaleForHwnd(m->hwnd);
        }

        switch (desc.backend)
        {
        case Backend::D3D11:
            m->ownedBackend = CreateD3D11Backend(desc.d3d11Device, desc.d3d11Context, desc.d3d11RestoreState);
            break;
        case Backend::D3D12:
            m->ownedBackend = CreateD3D12Backend(desc.d3d12Device, desc.d3d12FramesInFlight);
            break;
        case Backend::Custom:
            m->backend = desc.customBackend;
            break;
        default:
            break;
        }
        if (m->ownedBackend)
            m->backend = m->ownedBackend.get();
        m->backendKind = desc.backend;
        if (m->backend)
        {
            m->backend->SetLogCallback([](void* user, int level, const char* msg) { static_cast<Impl*>(user)->Log(level, "%s", msg); }, m);
            if (!m->backend->Init(io))
            {
                m->Log(2, "WGT: render backend '%s' failed to initialize", m->backend->Name());
                m->backend = nullptr;
                m->ownedBackend.reset();
            }
        }
        // No renderer (Backend::None or a failed init): the UI still runs - layout, input, text measurement -
        // and draws nothing. Fonts need ImGui's dynamic texture path, which a renderer would otherwise enable.
        if (!m->backend)
            io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

        LoadFonts(*m);

        Theme base = desc.darkMode ? ThemeDark() : ThemeLight();
        base = ThemeWithAccent(base, desc.accent);
        m->theme.Set(base, false);
        const float dpi = desc.dpiScale > 0.0f ? desc.dpiScale : m->dpiScale;
        m->theme.current.metrics.scale = dpi * m->userScale.load();
        ApplyThemeToImGuiStyle(m->theme.current, ImGui::GetStyle());
        return ctx;
    }

    void Context::Destroy()
    {
        Impl* m = impl_;
        SetCurrentContext(this);

        // 1. let plugins clean up while everything is alive
        std::vector<PluginEntry> plugins;
        {
            std::lock_guard lock(m->pluginMutex);
            plugins.swap(m->plugins);
        }
        for (PluginEntry& p : plugins)
            if (p.attached)
                p.plugin->OnDetach(*this);

        // 2. drop every callback that may point into plugin modules
        {
            std::unique_lock lock(m->panelMutex);
            m->panels.clear();
        }
        {
            std::lock_guard lock(m->taskMutex);
            m->tasks.clear();
        }
        {
            std::lock_guard lock(m->logMutex);
            m->log.Reset();
        }

        // 3. GPU + platform
        if (m->backend)
            m->backend->Shutdown();
        m->ownedBackend.reset();
        m->backend = nullptr;
        if (m->hwnd)
            ImGui_ImplWin32_Shutdown();
        m->state.Clear();
        // glyph atlas pages: GPU copies were released by the backend above
        if (m->fonts.engine)
            m->fonts.engine->Shutdown();
        ImGui::DestroyContext(m->imgui);
        m->fonts.engine.reset();
        m->imgui = nullptr;

        // 4. unload plugin modules last
        for (PluginEntry& p : plugins)
        {
            if (p.destroy)
                p.destroy(p.plugin);
            if (p.module)
                FreeLibrary(p.module);
        }

        if (tlsContext == this)
            tlsContext = nullptr;
        WgtImGuiTLS = nullptr;
        delete m;
        delete this;
    }

    void Context::NewFrame()
    {
        Impl& m = *impl_;
        SetCurrentContext(this);
        IM_ASSERT(!m.inFrame && "NewFrame() called twice without Render()");

        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        m.frameStart = now;
        m.dt = m.desc.fixedDeltaTime > 0.0f ? m.desc.fixedDeltaTime : (float)std::clamp(Seconds(m.qpcLast, now, m.qpcFreq), 1e-4, 0.1);
        m.qpcLast = now;
        m.time += m.dt;
        ++m.frameIndex;
        m.state.BeginFrame(m.frameIndex);
        LayoutNewFrame(m);
        m.ui.nextStyle = ui::ItemStyle();   // a ui::Next() style no component took does not carry over

        m.ProcessInput();
        m.RunTasks();
        m.AttachPendingPlugins();
        m.ApplyScaleAndTheme();

        ImGuiIO& io = ImGui::GetIO();
        if (m.hwnd)
            ImGui_ImplWin32_NewFrame();
        ImGui_ImplWin32_SetKeyState(nullptr);
        m.ApplyInjectedInput();
        m.ApplyRenderScale();
        io.DeltaTime = m.dt;
        ImGui::NewFrame();
        m.SyncFontDensity();
        const Theme& th = m.theme.current;
        if (m.fonts.engine)
        {
            m.fonts.engine->BeginFrame(m.frameIndex, th.metrics.scale, m.renderScale);
            // sub-pixel text only when UI pixels are display pixels (a scaled render target would smear the stripes)
            const TextAntialiasing aa = (TextAntialiasing)m.textAntialiasing.load();
            const bool oneToOne = std::fabs(m.renderScale - 1.0f) < 0.01f;
            const bool lcd = aa == TextAntialiasing::Subpixel || (aa == TextAntialiasing::Auto && m.fonts.engine->RenderParams().systemClearType && oneToOne);
            m.fonts.engine->SetSubpixel(lcd);
            m.subpixelActive = lcd;
        }
        ImGui::PushFont(m.fonts.compat[(int)FontWeight::Regular], th.type.size[(int)TextStyle::Body] * th.metrics.scale);
        m.inFrame = true;

        std::vector<IPlugin*> active;
        {
            std::lock_guard lock(m.pluginMutex);
            for (PluginEntry& p : m.plugins)
                if (p.attached)
                    active.push_back(p.plugin);
        }
        for (IPlugin* p : active)
            p->OnFrame(*this);
    }

    void Context::Render(const RenderTarget& target)
    {
        Impl& m = *impl_;
        SetCurrentContext(this);
        IM_ASSERT(m.inFrame && "Render() without NewFrame()");

        const bool isVisible = m.visible.load();
        if (isVisible)
        {
            m.DrawPanels();
            if (m.dockVisible.load())
                DockFrame(m);
        }
        IslandFrame(m);
        if (m.imeOwner != 0 && ImGui::GetActiveID() != m.imeOwner)
            m.ReleaseIme();
        LayoutInspect(m);
        ImGui::PopFont();
        ImGui::Render();
        m.inFrame = false;

        LARGE_INTEGER built;
        QueryPerformanceCounter(&built);
        const float uiMs = (float)(Seconds(m.frameStart, built, m.qpcFreq) * 1000.0);

        ImDrawData* dd = ImGui::GetDrawData();
        ImDrawData hiddenData;
        if (!isVisible)
        {
            // Only the foreground layer (notifications island) is presented while the UI is hidden.
            hiddenData.Valid = true;
            hiddenData.DisplayPos = dd->DisplayPos;
            hiddenData.DisplaySize = dd->DisplaySize;
            hiddenData.FramebufferScale = dd->FramebufferScale;
            hiddenData.OwnerViewport = dd->OwnerViewport;
            hiddenData.Textures = dd->Textures;
            ImDrawList* fg = ImGui::GetForegroundDrawList();
            if (fg->CmdBuffer.Size > 0 && !(fg->CmdBuffer.Size == 1 && fg->CmdBuffer[0].ElemCount == 0 && fg->CmdBuffer[0].UserCallback == nullptr))
                hiddenData.AddDrawList(fg);
            dd = &hiddenData;
        }

        // remember the target size: next frame's automatic render scale (font density) derives from it
        int tw = 0, th = 0;
        if (m.backend && m.backend->QueryTargetSize(target, tw, th))
        {
            m.targetW = tw;
            m.targetH = th;
        }

        if (m.backend)
        {
            BackendFrameInfo info;
            info.time = m.time;
            info.deltaTime = m.dt;
            info.maxBackdropCaptures = m.desc.maxBackdropCaptures;
            if (m.fonts.engine)
            {
                const TextRenderParams& tp = m.fonts.engine->RenderParams();
                info.textGamma = m.desc.textGamma > 0.0f ? std::clamp(m.desc.textGamma, 1.0f, 2.2f) : tp.gamma;
                // grayscale has no horizontal sub-samples: heavy stems make its stair-steps more visible, so it
                // runs at half the system stem weight (ClearType follows the system exactly, like Edge / DirectWrite)
                info.textGrayscaleContrast = m.desc.textContrast >= 0.0f ? m.desc.textContrast : tp.grayscaleContrast * 0.5f;
                info.textClearTypeContrast = m.desc.textContrast >= 0.0f ? m.desc.textContrast : tp.clearTypeContrast;
                info.textClearTypeLevel = tp.clearTypeLevel;
            }
            m.backend->RenderDrawData(dd, target, info);
        }

        ImGuiIO& io = ImGui::GetIO();
        m.wantMouse = isVisible && io.WantCaptureMouse;
        m.wantKeyboard = isVisible && io.WantCaptureKeyboard;
        m.wantText = isVisible && io.WantTextInput;
        m.desiredCursor = ImGui::GetMouseCursor();

        {
            std::lock_guard lock(m.statsMutex);
            FrameStats s;
            if (m.backend)
                m.backend->CollectStats(s);
            m.fpsAccum += m.dt;
            ++m.fpsFrames;
            if (m.fpsAccum >= 0.5f)
            {
                m.stats.fps = m.fpsFrames / m.fpsAccum;
                m.stats.frameMs = 1000.0f * m.fpsAccum / m.fpsFrames;
                m.fpsAccum = 0.0f;
                m.fpsFrames = 0;
            }
            s.fps = m.stats.fps;
            s.frameMs = m.stats.frameMs;
            s.cpuUiMs = Lerp(m.stats.cpuUiMs, uiMs, 0.1f);
            m.stats = s;
        }

        if ((m.frameIndex % 60) == 0)
            m.state.Collect(900);
    }

    void Context::InvalidateDeviceObjects()
    {
        if (impl_->backend)
            impl_->backend->InvalidateDeviceObjects();
    }

    // -------------------------------------------------------------- input
    bool Context::HandleWin32Message(void* hwndPtr, std::uint32_t msg, std::uint64_t wparam, std::int64_t lparam)
    {
        Impl& m = *impl_;
        HWND hwnd = static_cast<HWND>(hwndPtr);

        if (m.desc.toggleKey != 0 && (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) && wparam == m.desc.toggleKey)
        {
            if ((lparam & (1ll << 30)) == 0)
                ToggleVisible();
            return true;
        }

        if (msg == WM_DPICHANGED)
            m.dpiDirty = true;

        // A WGT text field has focus: WGT draws the IME composition inline and receives the result itself.
        if (hwnd && m.imeField.load())
        {
            switch (msg)
            {
            case WM_IME_SETCONTEXT:
                DefWindowProcW(hwnd, msg, (WPARAM)wparam, (LPARAM)(lparam & ~(std::int64_t)ISC_SHOWUICOMPOSITIONWINDOW));
                return true;
            case WM_IME_STARTCOMPOSITION:
                m.PlaceImeWindows(hwnd);
                return true;
            case WM_IME_COMPOSITION:
                m.ReadImeComposition(hwnd, (LPARAM)lparam);
                m.PlaceImeWindows(hwnd);
                return true;
            case WM_IME_ENDCOMPOSITION:
            {
                std::lock_guard lock(m.imeMutex);
                m.imeComposition.clear();
                m.imeCursor = 0;
                return true;
            }
            default:
                break;
            }
        }

        const bool isVisible = m.visible.load();
        if (msg == WM_SETCURSOR && isVisible && m.wantMouse.load() && LOWORD(lparam) == HTCLIENT)
        {
            LPTSTR cursor = IDC_ARROW;
            switch (m.desiredCursor.load())
            {
            case ImGuiMouseCursor_None: ::SetCursor(nullptr); return true;
            case ImGuiMouseCursor_TextInput: cursor = IDC_IBEAM; break;
            case ImGuiMouseCursor_ResizeAll: cursor = IDC_SIZEALL; break;
            case ImGuiMouseCursor_ResizeEW: cursor = IDC_SIZEWE; break;
            case ImGuiMouseCursor_ResizeNS: cursor = IDC_SIZENS; break;
            case ImGuiMouseCursor_ResizeNESW: cursor = IDC_SIZENESW; break;
            case ImGuiMouseCursor_ResizeNWSE: cursor = IDC_SIZENWSE; break;
            case ImGuiMouseCursor_Hand: cursor = IDC_HAND; break;
            case ImGuiMouseCursor_Wait: cursor = IDC_WAIT; break;
            case ImGuiMouseCursor_Progress: cursor = IDC_APPSTARTING; break;
            case ImGuiMouseCursor_NotAllowed: cursor = IDC_NO; break;
            default: break;
            }
            ::SetCursor(::LoadCursor(nullptr, cursor));
            return true;
        }

        // Mouse capture and leave tracking only work on the window's own thread: done here, not in the replay
        // (which may run on a separate render thread).
        if (hwnd)
        {
            switch (msg)
            {
            case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN: case WM_XBUTTONDOWN:
            case WM_LBUTTONDBLCLK: case WM_RBUTTONDBLCLK: case WM_MBUTTONDBLCLK: case WM_XBUTTONDBLCLK:
                if (::GetCapture() == nullptr)
                    ::SetCapture(hwnd);
                break;
            case WM_LBUTTONUP: case WM_RBUTTONUP: case WM_MBUTTONUP: case WM_XBUTTONUP:
            {
                const auto down = [](int vk) { return (::GetKeyState(vk) & 0x8000) != 0; };
                if (::GetCapture() == hwnd && !down(VK_LBUTTON) && !down(VK_RBUTTON) && !down(VK_MBUTTON) && !down(VK_XBUTTON1) && !down(VK_XBUTTON2))
                    ::ReleaseCapture();
                break;
            }
            case WM_MOUSEMOVE:
                if (!m.mouseTracked)
                {
                    TRACKMOUSEEVENT tme = {sizeof(tme), TME_LEAVE, hwnd, 0};
                    m.mouseTracked = ::TrackMouseEvent(&tme) != FALSE;
                }
                break;
            case WM_MOUSELEAVE:
                m.mouseTracked = false;
                break;
            default:
                break;
            }
        }

        if (IsQueuedInputMessage(msg))
        {
            // keyboard state belongs to this (window) thread: capture it with the message
            const bool keyish = (msg >= WM_KEYFIRST && msg <= WM_KEYLAST) || msg == WM_SETFOCUS || msg == WM_KILLFOCUS;
            KeyState keys{};
            if (keyish)
                ::GetKeyboardState(keys.data());
            std::lock_guard lock(m.inputMutex);
            Win32Message qm{hwnd, (UINT)msg, (WPARAM)wparam, (LPARAM)lparam};
            if (keyish)
            {
                qm.keys = (int)m.inputKeys.size();
                m.inputKeys.push_back(keys);
                m.latestKeys = keys;
            }
            m.inputQueue.push_back(qm);
        }

        if (!isVisible)
            return false;
        if (IsMouseMessage(msg))
            return m.wantMouse.load();
        switch (msg)
        {
        case WM_KEYDOWN: case WM_KEYUP: case WM_SYSKEYDOWN: case WM_SYSKEYUP:
            return m.wantKeyboard.load();
        case WM_CHAR:
            return m.wantText.load() || m.wantKeyboard.load();
        default:
            return false;
        }
    }

    void Context::SetDebugLayout(bool enabled) { impl_->debugLayout = enabled; }
    void Context::SetTextAntialiasing(TextAntialiasing mode) { impl_->textAntialiasing = (int)mode; }
    TextAntialiasing Context::GetTextAntialiasing() const { return (TextAntialiasing)impl_->textAntialiasing.load(); }
    bool Context::IsSubpixelTextActive() const { return impl_->subpixelActive.load(); }

    Context::TextRenderingInfo Context::GetTextRenderingInfo() const
    {
        TextRenderingInfo r{};
        if (const TextEngine* e = impl_->fonts.engine.get())
        {
            const TextRenderParams& p = e->RenderParams();
            r = {p.gamma, p.grayscaleContrast, p.clearTypeContrast, p.clearTypeLevel, p.systemClearType, p.bgr, impl_->subpixelActive.load()};
        }
        return r;
    }

    bool Context::WantsMouse() const { return impl_->wantMouse.load(); }
    bool Context::WantsKeyboard() const { return impl_->wantKeyboard.load(); }
    void Context::SetVisible(bool v) { impl_->visible.store(v); }
    bool Context::IsVisible() const { return impl_->visible.load(); }
    void Context::ToggleVisible() { impl_->visible.store(!impl_->visible.load()); }

    // ---------------------------------------------------------- services
    void Context::Post(Task task)
    {
        std::lock_guard lock(impl_->taskMutex);
        impl_->tasks.push_back(std::move(task));
    }

    void Context::SetTheme(const Theme& theme, bool animate)
    {
        std::lock_guard lock(impl_->themeMutex);
        impl_->pendingTheme = theme;
        impl_->pendingThemeAnimate = animate;
        impl_->pendingThemeValid = true;
        impl_->darkMode = theme.dark;
    }

    void Context::SetDarkMode(bool dark, bool animate)
    {
        Color accent;
        {
            std::lock_guard lock(impl_->themeMutex);
            accent = impl_->accentOverride;
        }
        SetTheme(ThemeWithAccent(dark ? ThemeDark() : ThemeLight(), accent), animate);
    }

    bool Context::IsDarkMode() const { return impl_->darkMode.load(); }

    void Context::SetAccent(Color accent, bool animate)
    {
        {
            std::lock_guard lock(impl_->themeMutex);
            impl_->accentOverride = accent;
        }
        SetDarkMode(impl_->darkMode.load(), animate);
    }

    void Context::SetUiScale(float multiplier) { impl_->userScale.store(multiplier > 0.0f ? multiplier : 1.0f); }
    void Context::SetDpiScale(float scale) { impl_->dpiOverride.store(std::max(scale, 0.0f)); impl_->dpiDirty = true; }
    void Context::SetRenderScale(float scale) { impl_->renderOverride.store(std::max(scale, 0.0f)); }

    ScaleInfo Context::GetScaleInfo() const
    {
        std::lock_guard lock(impl_->scaleInfoMutex);
        return impl_->scaleInfo;
    }

    const Theme& Context::GetTheme() const { return impl_->theme.current; }

    void Context::Notify(const Notification& n)
    {
        IslandItem item;
        item.title = n.title ? n.title : "";
        item.message = n.message ? n.message : "";
        item.icon = n.icon;
        item.tint = n.tint;
        item.duration = n.duration;
        std::lock_guard lock(impl_->islandMutex);
        impl_->islandQueue.push_back(std::move(item));
    }

    void Context::SetActivity(const char* id, const char* title, float progress, Icon icon, Color tint)
    {
        if (!id)
            return;
        std::lock_guard lock(impl_->islandMutex);
        for (IslandActivity& a : impl_->activities)
            if (a.id == id)
            {
                if (title)
                    a.title = title;
                a.progress = progress;
                a.icon = icon ? icon : a.icon;
                if (tint.a > 0)
                    a.tint = tint;
                return;
            }
        IslandActivity a;
        a.id = id;
        a.title = title ? title : "";
        a.progress = progress;
        a.icon = icon;
        a.tint = tint;
        impl_->activities.push_back(std::move(a));
    }

    void Context::ClearActivity(const char* id)
    {
        if (!id)
            return;
        std::lock_guard lock(impl_->islandMutex);
        auto& v = impl_->activities;
        v.erase(std::remove_if(v.begin(), v.end(), [&](const IslandActivity& a) { return a.id == id; }), v.end());
    }

    bool Context::AddPanel(const PanelDesc& desc, PanelFn draw)
    {
        if (!desc.id || !*desc.id)
            return false;
        auto e = std::make_shared<PanelEntry>();
        e->id = desc.id;
        e->title = desc.title ? desc.title : desc.id;
        e->desc = desc;
        e->desc.id = e->id.c_str();
        e->desc.title = e->title.c_str();
        e->draw = std::move(draw);
        e->open = desc.open;
        std::unique_lock lock(impl_->panelMutex);
        for (auto& p : impl_->panels)
            if (p->id == e->id)
                return false;
        impl_->panels.push_back(std::move(e));
        return true;
    }

    void Context::RemovePanel(const char* id)
    {
        if (!id)
            return;
        std::shared_ptr<PanelEntry> removed;   // destroyed outside the lock
        std::unique_lock lock(impl_->panelMutex);
        auto& v = impl_->panels;
        for (auto it = v.begin(); it != v.end(); ++it)
            if ((*it)->id == id)
            {
                removed = *it;
                v.erase(it);
                break;
            }
        lock.unlock();
    }

    void Context::SetPanelOpen(const char* id, bool open)
    {
        std::shared_lock lock(impl_->panelMutex);
        for (auto& p : impl_->panels)
            if (p->id == id)
            {
                p->open = open;
                if (open)
                    p->focusRequest = true;
            }
    }

    bool Context::IsPanelOpen(const char* id) const
    {
        std::shared_lock lock(impl_->panelMutex);
        for (auto& p : impl_->panels)
            if (p->id == id)
                return p->open.load();
        return false;
    }

    void Context::SetDockVisible(bool v) { impl_->dockVisible = v; }

    EffectId Context::RegisterEffect(const char* name, const char* hlslSource)
    {
        if (!hlslSource || !impl_->backend)
            return 0;
        EffectId id = ++impl_->nextEffectId;
        impl_->backend->SetEffectSource(id, name ? name : "effect", hlslSource);
        return id;
    }

    ImTextureID Context::CreateTexture(const void* rgba8, int width, int height)
    {
        if (!impl_->backend || !rgba8 || width <= 0 || height <= 0)
            return ImTextureID_Invalid;
        return impl_->backend->CreateTexture(rgba8, width, height);
    }

    void Context::DestroyTexture(ImTextureID texture)
    {
        if (impl_->backend && texture != ImTextureID_Invalid)
            impl_->backend->DestroyTexture(texture);
    }

    bool Context::LoadPlugin(const wchar_t* dllPath)
    {
        HMODULE mod = LoadLibraryW(dllPath);
        if (!mod)
        {
            impl_->Log(2, "WGT: cannot load plugin (error %lu)", GetLastError());
            return false;
        }
        using CreateFn = IPlugin* (*)(std::uint32_t);
        using DestroyFn = void (*)(IPlugin*);
        auto create = reinterpret_cast<CreateFn>(GetProcAddress(mod, "WgtCreatePlugin"));
        auto destroy = reinterpret_cast<DestroyFn>(GetProcAddress(mod, "WgtDestroyPlugin"));
        IPlugin* plugin = create ? create(WGT_ABI_VERSION) : nullptr;
        if (!plugin)
        {
            impl_->Log(2, "WGT: plugin has no compatible WgtCreatePlugin export (ABI %u)", WGT_ABI_VERSION);
            FreeLibrary(mod);
            return false;
        }
        PluginEntry e;
        e.plugin = plugin;
        e.module = mod;
        e.destroy = destroy;
        std::lock_guard lock(impl_->pluginMutex);
        impl_->plugins.push_back(e);
        return true;
    }

    int Context::LoadPluginsFromDirectory(const wchar_t* directory)
    {
        if (!directory)
            return 0;
        std::wstring pattern = std::wstring(directory) + L"\\*.dll";
        WIN32_FIND_DATAW fd;
        HANDLE h = FindFirstFileW(pattern.c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE)
            return 0;
        int count = 0;
        do
        {
            std::wstring path = std::wstring(directory) + L"\\" + fd.cFileName;
            if (LoadPlugin(path.c_str()))
                ++count;
        } while (FindNextFileW(h, &fd));
        FindClose(h);
        return count;
    }

    void Context::AddPlugin(IPlugin* plugin)
    {
        if (!plugin)
            return;
        PluginEntry e;
        e.plugin = plugin;
        std::lock_guard lock(impl_->pluginMutex);
        impl_->plugins.push_back(e);
    }

    void Context::SetLogCallback(LogFn fn)
    {
        std::lock_guard lock(impl_->logMutex);
        impl_->log = std::move(fn);
    }

    namespace
    {
        void PushInjected(Context::Impl* m, Context::Impl::InjectedEvent e)
        {
            std::lock_guard lock(m->injectMutex);
            m->injected.push_back(std::move(e));
        }
    }

    void Context::InjectMousePos(float x, float y)
    {
        Impl::InjectedEvent e{Impl::InjectedEvent::Pos};
        e.x = x;
        e.y = y;
        PushInjected(impl_, std::move(e));
    }

    void Context::InjectMouseButton(int button, bool down)
    {
        Impl::InjectedEvent e{Impl::InjectedEvent::Button};
        e.button = button;
        e.down = down;
        PushInjected(impl_, std::move(e));
    }

    void Context::InjectMouseWheel(float wheelY, float wheelX)
    {
        Impl::InjectedEvent e{Impl::InjectedEvent::Wheel};
        e.x = wheelX;
        e.y = wheelY;
        PushInjected(impl_, std::move(e));
    }

    void Context::InjectKey(ImGuiKey key, bool down)
    {
        Impl::InjectedEvent e{Impl::InjectedEvent::Key};
        e.key = key;
        e.down = down;
        PushInjected(impl_, std::move(e));
    }

    void Context::InjectText(const char* utf8)
    {
        Impl::InjectedEvent e{Impl::InjectedEvent::Text};
        e.text = utf8 ? utf8 : "";
        PushInjected(impl_, std::move(e));
    }

    void Context::EndInputInjection()
    {
        Post([this]() { impl_->injectActive = false; });
    }

    FrameStats Context::GetStats() const
    {
        std::lock_guard lock(impl_->statsMutex);
        return impl_->stats;
    }

    Backend Context::GetBackend() const { return impl_->backendKind; }
    ImGuiContext* Context::GetImGuiContext() const { return impl_->imgui; }
}
