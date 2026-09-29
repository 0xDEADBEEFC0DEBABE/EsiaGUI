// WGT UI - Context: lifecycle, frame loop and thread-safe services.
//
// Threading model (see docs/THREADING.md):
//   * One Context is driven by one "UI thread" = the thread calling NewFrame()/Render().
//     All wgt::ui widgets and ImGui calls happen on that thread between the two calls.
//   * Every method marked [any thread] can be called concurrently from game / worker threads.
//     Those calls are queued or lock-protected and take effect at the next NewFrame().
//   * Win32 input is thread-safe too: HandleWin32Message() may run on the window thread while the
//     UI is rendered from a separate render thread; messages are queued and replayed in order.
//   * ImGui's current-context pointer is thread-local, so several contexts can run on several
//     threads side by side.
#pragma once
#include "function.hpp"
#include "painter.hpp"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12Resource;

namespace wgt
{
    class IRenderBackend;

    enum class Backend : std::uint8_t
    {
        None,
        D3D11,
        D3D12,
        Custom,
    };

    enum class TextAntialiasing : std::uint8_t
    {
        Auto,        // sub-pixel when Windows ClearType is on and the UI maps 1:1 to display pixels, else grayscale
        Grayscale,   // macOS-style: exact coverage, gamma-correct (best for Retina-class densities and video)
        Subpixel,    // ClearType-style: one coverage per R/G/B stripe, 3x horizontal resolution on LCDs
    };

    struct ContextDesc
    {
        Backend backend = Backend::D3D11;
        void* hwnd = nullptr;                        // HWND used for input, DPI and cursor (optional)

        // --- Direct3D 11
        ID3D11Device* d3d11Device = nullptr;
        ID3D11DeviceContext* d3d11Context = nullptr; // immediate context used by Render()
        bool d3d11RestoreState = true;               // back up / restore the host pipeline state around Render()

        // --- Direct3D 12
        ID3D12Device* d3d12Device = nullptr;
        std::uint32_t d3d12FramesInFlight = 3;       // must be >= the host's frame latency

        // --- Custom backend (not owned, must outlive the context)
        IRenderBackend* customBackend = nullptr;

        // --- Look & feel
        bool darkMode = true;
        Color accent = Color::Clear();               // Clear = theme default (system blue)

        // --- Scaling (see "Scale model" in docs/EXTENDING.md)
        //   UI units      == window client pixels (the space of mouse input and ImGui layout)
        //   metric scale  == dpiScale * uiScale  (every size, radius and font size is multiplied by it)
        //   render scale  == render-target pixels per UI unit (fonts are rasterized at this density)
        float uiScale = 1.0f;                        // user preference multiplier (e.g. 0.85 / 1.0 / 1.25)
        float dpiScale = 0.0f;                       // 0 = follow the monitor DPI of `hwnd` (per-monitor aware), > 0 = fixed
        float renderScale = 0.0f;                    // 0 = automatic (render target size / window client size), > 0 = fixed
        // --- Text (DirectWrite; families are looked up by name, never loaded from TTF blobs)
        const char* fontFamily = nullptr;            // UI text family. null = "Segoe UI Variable" optical family (Small/Text/Display), "Segoe UI" on Windows 10
        const char* fontFamilyDisplay = nullptr;     // optional family for large sizes (e.g. "SF Pro Display" with fontFamily = "SF Pro Text")
        const char* monoFamily = nullptr;            // null = "Cascadia Mono", then "Consolas"
        const char* iconFamily = nullptr;            // null = "Segoe Fluent Icons", then "Segoe MDL2 Assets"
        const wchar_t* const* fontFiles = nullptr;   // extra .ttf/.otf/.ttc files registered into the WGT font collection
        int fontFileCount = 0;
        const char* locale = nullptr;                // BCP-47 tag driving fallback & CJK glyph variants (null = user default, e.g. "zh-CN")
        TextAntialiasing textAntialiasing = TextAntialiasing::Auto;
        float textGamma = 0.0f;                      // > 0 overrides the system text gamma (1.0 .. 2.2)
        float textContrast = -1.0f;                  // >= 0 overrides the system enhanced contrast (stem weight, 0 .. 2)

        // --- Behaviour
        const char* iniFilename = nullptr;           // ImGui window layout persistence (null = off)
        std::uint32_t toggleKey = 0;                 // Win32 virtual key that shows / hides the UI (0 = none)
        bool showDock = true;                        // panel launcher dock
        float fixedDeltaTime = 0.0f;                 // > 0: animations advance by this step every frame (tests, video capture)
        bool debugLayout = false;                    // layout inspector: outlines + logs items that overlap a sibling or are cut off by the window edge
        // Liquid-glass backdrop refreshes per frame. Every glass surface that sits on top of something drawn
        // earlier in the frame needs one (region-limited, cheap); past the budget glass falls back to the
        // last capture and a warning is logged once.
        int maxBackdropCaptures = 64;
    };

    struct RenderTarget
    {
        // Direct3D 11
        ID3D11RenderTargetView* d3d11Rtv = nullptr;
        // Direct3D 12 - the resource must be in D3D12_RESOURCE_STATE_RENDER_TARGET and is left in it.
        ID3D12GraphicsCommandList* d3d12CommandList = nullptr;
        ID3D12Resource* d3d12Resource = nullptr;
        std::uint64_t d3d12Rtv = 0;                  // D3D12_CPU_DESCRIPTOR_HANDLE::ptr
        std::uint32_t d3d12RtvFormat = 0;            // DXGI_FORMAT of the view (0 = resource format)

        static RenderTarget D3D11(ID3D11RenderTargetView* rtv)
        {
            RenderTarget t;
            t.d3d11Rtv = rtv;
            return t;
        }
        static RenderTarget D3D12(ID3D12GraphicsCommandList* list, ID3D12Resource* resource, std::uint64_t rtv, std::uint32_t rtvFormat = 0)
        {
            RenderTarget t;
            t.d3d12CommandList = list;
            t.d3d12Resource = resource;
            t.d3d12Rtv = rtv;
            t.d3d12RtvFormat = rtvFormat;
            return t;
        }
    };

    struct Notification
    {
        const char* title = nullptr;
        const char* message = nullptr;
        Icon icon = 0;
        Color tint = Color::Clear();   // Clear = accent
        float duration = 3.5f;         // seconds on screen
    };

    enum PanelFlags_ : std::uint32_t
    {
        PanelFlags_None = 0,
        PanelFlags_HideFromDock = 1u << 0,
        PanelFlags_NoClose = 1u << 1,
        PanelFlags_NoResize = 1u << 2,
        PanelFlags_Solid = 1u << 3,    // opaque surface instead of liquid glass
        PanelFlags_NoScroll = 1u << 4, // the panel manages its own scrolling (e.g. ui::BeginNavigation)
        PanelFlags_ClearGlass = 1u << 5, // fully transparent glass surface (no frost, no tint)
    };

    struct PanelDesc
    {
        const char* id = nullptr;       // unique, stable id
        const char* title = nullptr;    // window title (defaults to id)
        Icon icon = 0;                  // dock icon
        Color iconColor = Color::Clear();
        Vec2 size = Vec2(420, 540);
        Vec2 pos = Vec2(-1, -1);        // first-use position (-1 = auto)
        bool open = false;
        std::uint32_t flags = PanelFlags_None;
    };

    struct ScaleInfo
    {
        float dpi = 1.0f;          // monitor DPI scale (1.0 = 96 dpi, 1.5 = 144 dpi ...)
        float user = 1.0f;         // user multiplier (Context::SetUiScale)
        float metrics = 1.0f;      // dpi * user: applied to all sizes and font sizes
        float render = 1.0f;       // render-target pixels per UI unit (font rasterizer density)
    };

    struct FrameStats
    {
        float fps = 0;
        float frameMs = 0;
        float cpuUiMs = 0;           // NewFrame -> Render (UI build) cost
        int drawCalls = 0;
        int fxInstances = 0;
        int backdropCaptures = 0;
        int glowLayers = 0;
        int vertices = 0;
        // GPU time of WGT's rendering (timestamp queries, a few frames old; 0 when unavailable)
        float gpuMs = 0;
        float gpuGlassMs = 0;        // backdrop captures + blur pyramids
        float gpuLayerMs = 0;        // glow layers (bloom pyramid + composite)
    };

    // Plugins extend a context with panels, effects, themes and per-frame UI.
    class IPlugin
    {
    public:
        virtual ~IPlugin() = default;
        virtual const char* Name() const = 0;
        virtual void OnAttach(Context& ctx) { (void)ctx; }   // UI thread
        virtual void OnFrame(Context& ctx) { (void)ctx; }    // UI thread, inside the frame
        virtual void OnDetach(Context& ctx) { (void)ctx; }   // UI thread
    };

    using PanelFn = Callback<void(Context&)>;
    using LogFn = Callback<void(int level, const char* message)>;

    class WGT_API Context
    {
    public:
        // Creates a context and makes it current on the calling thread (which becomes the UI thread).
        static Context* Create(const ContextDesc& desc);
        void Destroy();

        // ------------------------------------------------------- UI thread
        void NewFrame();
        void Render(const RenderTarget& target);
        // Frees WGT's render-target-sized GPU surfaces now (recreated on demand), e.g. to reclaim memory. Not
        // needed on resize (surfaces follow the target size). A lost / replaced device needs a new Context.
        void InvalidateDeviceObjects();

        // ------------------------------------------------------ any thread
        // Win32 input. Returns true when the UI wants the message swallowed (capture).
        bool HandleWin32Message(void* hwnd, std::uint32_t msg, std::uint64_t wparam, std::int64_t lparam);
        bool WantsMouse() const;
        bool WantsKeyboard() const;

        void SetVisible(bool visible);
        bool IsVisible() const;
        void ToggleVisible();

        // Runs `task` on the UI thread at the start of the next frame.
        void Post(Task task);

        void SetTheme(const Theme& theme, bool animate = true);
        void SetDarkMode(bool dark, bool animate = true);
        bool IsDarkMode() const;
        void SetAccent(Color accent, bool animate = true);
        // Look of every glass surface (default Frosted). ui::PushGlassLook overrides it for a scope.
        void SetGlassLook(GlassLook look);
        GlassLook GetGlassLook() const;
        // Scale model: metrics = dpi * user, fonts rasterized at metrics * render density.
        void SetUiScale(float multiplier);    // user preference, 1 = default
        void SetDpiScale(float scale);        // 0 = follow the window's monitor
        void SetRenderScale(float scale);     // 0 = automatic from render target / window size
        ScaleInfo GetScaleInfo() const;

        void Notify(const Notification& notification);
        // Live activity shown in the island (progress < 0 = indeterminate).
        void SetActivity(const char* id, const char* title, float progress, Icon icon = 0, Color tint = Color::Clear());
        void ClearActivity(const char* id);

        bool AddPanel(const PanelDesc& desc, PanelFn draw);
        void RemovePanel(const char* id);
        void SetPanelOpen(const char* id, bool open);
        bool IsPanelOpen(const char* id) const;
        void SetDockVisible(bool visible);

        // Custom FX pixel effect (HLSL). The source must define `float4 WgtEffect(WgtFx fx)` returning a
        // premultiplied color. It starts compiling on a worker thread right away (about a second); until it is
        // ready, shapes using it render with the built-in shader. See docs/EXTENDING.md.
        EffectId RegisterEffect(const char* name, const char* hlslSource);

        // Registers a font file (.ttf/.otf/.ttc) into the WGT font collection (applied at the next frame).
        void AddFontFile(const wchar_t* path);

        // RGBA8 texture usable with ImGui::Image, Painter::Image or Style::Image.
        ImTextureID CreateTexture(const void* rgba8, int width, int height);
        void DestroyTexture(ImTextureID texture);

        // Plugins
        bool LoadPlugin(const wchar_t* dllPath);
        int LoadPluginsFromDirectory(const wchar_t* directory);
        void AddPlugin(IPlugin* plugin);   // in-process plugin, not owned

        void SetLogCallback(LogFn fn);

        // ------------------------------------------- automation / UI tests (any thread)
        // Injected input is applied at the next NewFrame, on top of OS input. While injecting, the last
        // injected mouse position overrides the real cursor until EndInputInjection().
        void InjectMousePos(float x, float y);                  // UI units (window client pixels)
        void InjectMouseButton(int button, bool down);          // 0 left, 1 right, 2 middle
        void InjectMouseWheel(float wheelY, float wheelX = 0.0f);
        void InjectKey(ImGuiKey key, bool down);
        void InjectText(const char* utf8);
        void EndInputInjection();

        // Layout inspector (any thread): overlapping sibling items are outlined in red, items cut off by their
        // window's right edge in orange; each is logged once.
        void SetDebugLayout(bool enabled);
        // Text anti-aliasing (any thread, applied at the next frame).
        void SetTextAntialiasing(TextAntialiasing mode);
        TextAntialiasing GetTextAntialiasing() const;
        bool IsSubpixelTextActive() const;   // what Auto resolved to last frame
        struct TextRenderingInfo
        {
            float gamma, grayscaleContrast, clearTypeContrast, clearTypeLevel;
            bool systemClearType, bgr, subpixel;
        };
        TextRenderingInfo GetTextRenderingInfo() const;   // UI thread

        FrameStats GetStats() const;
        Backend GetBackend() const;
        ImGuiContext* GetImGuiContext() const;

        // ---------------------------------------------- UI thread accessors
        const Theme& GetTheme() const;   // current (animated) theme

        struct Impl;
        Impl* GetImpl() const { return impl_; }

    private:
        Context();
        ~Context();
        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;
        Impl* impl_ = nullptr;
    };

    // Current context of the calling thread (set by Create/NewFrame, null elsewhere).
    WGT_API Context* GetCurrentContext();
    WGT_API void SetCurrentContext(Context* ctx);

    // Theme & fonts of the current context (UI thread).
    WGT_API const Theme& CurrentTheme();
    // Font of a theme text style, sized with the current metrics scale.
    WGT_API FontRef GetFont(TextStyle style);
    // Font of a weight at an unscaled (theme) size.
    WGT_API FontRef GetFont(FontWeight weight, float unscaledSize);
    // Dear ImGui compatibility font (DirectWrite-backed) for stock ImGui widgets, e.g. ImGui::PushFont(wgt::GetImGuiFont(...), size).
    WGT_API ImFont* GetImGuiFont(FontWeight weight);
}

// ------------------------------------------------------------------ plugin ABI
// A plugin DLL exports these two functions (use WGT_DECLARE_PLUGIN):
//   extern "C" wgt::IPlugin* WgtCreatePlugin(std::uint32_t abiVersion);
//   extern "C" void          WgtDestroyPlugin(wgt::IPlugin* plugin);
#define WGT_DECLARE_PLUGIN(PluginType)                                                             \
    extern "C" __declspec(dllexport) wgt::IPlugin* WgtCreatePlugin(std::uint32_t abiVersion)      \
    {                                                                                              \
        return abiVersion == WGT_ABI_VERSION ? new PluginType() : nullptr;                         \
    }                                                                                              \
    extern "C" __declspec(dllexport) void WgtDestroyPlugin(wgt::IPlugin* plugin) { delete plugin; }
