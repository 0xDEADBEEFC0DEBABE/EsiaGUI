// WGT UI - internal context state
#pragma once
#include "wgt/wgt.hpp"
#include "imgui_internal.h"
#include "core/state_store.hpp"
#include "text/imgui_font_loader.hpp"

#include <windows.h>
#include <array>
#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace wgt
{
    struct Win32Message
    {
        HWND hwnd;
        UINT msg;
        WPARAM wparam;
        LPARAM lparam;
        int keys = -1;   // index of the keyboard state captured with it on the window thread (key / focus messages)
    };
    using KeyState = std::array<unsigned char, 256>;

    struct PanelEntry
    {
        std::string id;
        std::string title;
        PanelDesc desc;          // string pointers inside are re-pointed to the members above
        PanelFn draw;
        std::atomic<bool> open{false};
        std::atomic<bool> focusRequest{false};
        bool wasOpen = false;
    };

    struct IslandItem
    {
        std::string title;
        std::string message;
        Icon icon = 0;
        Color tint = Color::Clear();
        float duration = 3.5f;
    };

    struct IslandActivity
    {
        std::string id;
        std::string title;
        float progress = 0.0f;
        Icon icon = 0;
        Color tint = Color::Clear();
    };

    struct PluginEntry
    {
        IPlugin* plugin = nullptr;
        HMODULE module = nullptr;
        void (*destroy)(IPlugin*) = nullptr;
        bool attached = false;
    };

    // Per-context stacks used by container widgets (UI thread only).
    struct SectionFrame
    {
        ImGuiID id = 0;
        ImDrawList* drawList = nullptr;
        ImDrawListSplitter splitter;
        float x0 = 0, x1 = 0, y0 = 0;
        int rows = 0;
        bool lookPushed = false;   // took a ui::Next() style (End* pops it)
        Rect maskRect;          // card rect from the previous frame (row highlight masks)
        std::string footer;
    };

    struct WindowFrame
    {
        ImGuiID id = 0;
        std::uint32_t flags = 0;
        bool hasContent = false;
        int styleVars = 0;
        float cornerCut = 0.0f;   // content clip inset keeping it inside the rounded corners (pushed on the content)
        bool lookPushed = false;  // took a ui::Next() style (EndWindow pops it)
    };

    struct NavFrame
    {
        ImGuiID id = 0;
        void* state = nullptr;   // NavState*
        float width = 0, height = 0;
    };

    struct PageFrame
    {
        int styleVars = 0;
        bool clipped = false;   // the page below a transition: clipped to the strip the top page has not covered
        Rect edgeShadow;        // ... with the top page's shadow falling on it (screen space)
        float shadowAlpha = 0.0f;
    };

    // ------------------------------------------------------------ auto layout (ui/layout.cpp)
    enum class LayoutKind : std::uint8_t { VStack, HStack, Adaptive, Grid, Flow };

    struct LayoutChild
    {
        Vec2 size{0, 0};         // measured size (UI units)
        Vec2 pos{0, 0};          // where it was placed this frame
        float flex = 0.0f;       // explicit share of the free main-axis space (ui::LayoutFlex)
        int span = 1;            // grid columns (ui::LayoutSpan)
        float overflow = 0.0f;   // visual overflow margin (glow) reported while it was drawn
        bool fill = false;       // queried the available width -> stretches to what it is offered
    };

    struct LayoutCache
    {
        std::vector<LayoutChild> children;
        std::uint64_t lastFrame = 0;
        bool measured = false;
        bool horizontal = true;  // adaptive stack: current axis
    };

    struct LayoutFrame
    {
        ImGuiID id = 0;
        LayoutKind kind = LayoutKind::VStack;
        ImGuiWindow* window = nullptr;
        int depth = 0;                       // g.GroupStack.Size of direct children
        Vec2 origin{0, 0};
        float width = 0.0f;
        // options (UI units, scaled)
        float spacing = 0.0f, lineSpacing = 0.0f;
        std::uint8_t align = 0, justify = 0;
        bool animate = true;
        float minColumn = 0.0f, cellHeight = 0.0f, aspect = 0.0f, minFill = 0.0f;
        int columns = 0, maxColumns = 0;
        bool horizontal = true;
        // state
        LayoutCache* cache = nullptr;
        bool hidden = false;                 // first frame: measure only, draw nothing
        std::vector<Rect> regions;           // offered rect per child (from last frame's measurements)
        std::vector<Vec2> places;            // aligned top-left per child
        std::vector<LayoutChild> now;        // measured this frame
        Rect region;                         // region of the child being submitted (animated)
        float pendingFlex = 0.0f, pendingOverflow = 0.0f;
        Rect pendingGlow;                    // glow reach drawn inside the child still being submitted
        bool hasPendingGlow = false;
        int pendingSpan = 1;
        bool pendingFill = false;
        float backupWorkMaxX = 0.0f, backupContentMaxX = 0.0f;
    };

    struct WindowItems
    {
        std::vector<Rect> prev, cur;         // item rects of the previous / current frame
        std::vector<int> curDepth;           // group depth of every current item (layout inspector)
        std::uint64_t frame = 0;
        ImGuiWindow* window = nullptr;
    };

    struct UiStacks
    {
        std::vector<std::unique_ptr<SectionFrame>> sections;
        std::vector<WindowFrame> windows;
        std::vector<NavFrame> navs;
        std::vector<PageFrame> pages;
        std::vector<ImGuiID> scrolls;
        std::vector<std::uint8_t> scrollEdges;   // per open scroll area: 1 = clip pushed in, 2 = edge fade open
        struct StyleEntry
        {
            ui::ItemStyle style;
            ui::ItemStyle resolved;              // this scope merged over every enclosing one (lookups read it directly)
            bool alpha = false;                  // pushed an ImGui style alpha (Opacity)
        };
        std::vector<StyleEntry> itemStyles;      // ui::PushItemStyle scopes (PushGlassLook is one)
        ui::ItemStyle nextStyle;                 // ui::Next(), taken by the next component
        bool floatingItem = false;               // the next laid-out item floats over the content (tab bar, search bar)
        std::vector<std::uint8_t> rowLooks;      // per BeginRow: 1 = it took a ui::Next() style (EndRow pops it)
        std::vector<Vec2> rowRegions;            // per BeginRow: the window's work / content right edges EndRow restores
        std::vector<ImGuiID> cards;
        std::vector<std::unique_ptr<LayoutFrame>> layouts;
        std::unordered_map<ImGuiID, LayoutCache> layoutCache;
        std::unordered_map<ImGuiID, WindowItems> items;   // per window (ItemSize hook)
        int windowCascade = 0;
    };

    struct FontSet
    {
        std::unique_ptr<TextEngine> engine;                       // WGT text (DirectWrite)
        CompatFontSource compatSources[(int)FontWeight::Count];  // descriptors for Dear ImGui compat fonts
        ImFont* compat[(int)FontWeight::Count] = {};             // Dear ImGui fonts (stock widgets, InputText)
    };

    struct ThemeAnimator
    {
        Theme from;
        Theme to;
        Theme current;
        float t = 1.0f;
        float duration = 0.45f;

        void Set(const Theme& target, bool animate);
        void Step(float dt);
    };

    struct Context::Impl
    {
        Context* self = nullptr;
        ContextDesc desc;
        ImGuiContext* imgui = nullptr;
        HWND hwnd = nullptr;
        std::thread::id owner;

        // backend
        std::unique_ptr<IRenderBackend> ownedBackend;
        IRenderBackend* backend = nullptr;
        Backend backendKind = Backend::None;

        // fonts / theme / animation
        FontSet fonts;
        ThemeAnimator theme;
        std::mutex themeMutex;               // guards pendingTheme*
        bool pendingThemeValid = false;
        Theme pendingTheme;
        bool pendingThemeAnimate = true;
        std::atomic<bool> darkMode{true};
        Color accentOverride = Color::Clear();

        // scale model (see ContextDesc)
        std::atomic<float> userScale{1.0f};
        std::atomic<float> dpiOverride{0.0f};
        std::atomic<float> renderOverride{0.0f};
        std::atomic<bool> dpiDirty{true};
        float dpiScale = 1.0f;          // monitor DPI (auto)
        float appliedScale = 0.0f;      // metrics scale applied last frame
        float renderScale = 1.0f;       // framebuffer pixels per UI unit applied this frame
        std::atomic<int> targetW{0};
        std::atomic<int> targetH{0};
        mutable std::mutex scaleInfoMutex;
        ScaleInfo scaleInfo;
        StateStore state;
        UiStacks ui;

        // clock
        double time = 0.0;
        float dt = 1.0f / 60.0f;
        LARGE_INTEGER qpcFreq{};
        LARGE_INTEGER qpcLast{};
        LARGE_INTEGER frameStart{};
        std::uint64_t frameIndex = 0;

        // input (thread-safe)
        std::mutex inputMutex;
        std::vector<Win32Message> inputQueue;
        std::vector<Win32Message> inputScratch;
        std::vector<KeyState> inputKeys, inputKeysScratch;   // keyboard snapshots referenced by queued messages
        KeyState latestKeys{};                               // newest snapshot (guarded by inputMutex)
        KeyState frameKeys{};                                // UI thread copy used during NewFrame
        std::atomic<bool> wantMouse{false};
        std::atomic<bool> wantKeyboard{false};
        std::atomic<bool> wantText{false};
        std::atomic<int> desiredCursor{ImGuiMouseCursor_Arrow};
        std::atomic<bool> visible{true};
        bool mouseTracked = false;           // window thread: TrackMouseEvent armed
        std::atomic<bool> debugLayout{false};
        std::atomic<int> glassLook{(int)GlassLook::Frosted};   // GlassLook, Context::SetGlassLook (default: iOS frost)
        std::atomic<int> textAntialiasing{0};
        std::atomic<bool> subpixelActive{false};
        std::unordered_map<std::uint64_t, bool> reportedOverlaps;
        // layout inspector: items that run past a window's right edge (key: window + rect)
        struct CutOffItem
        {
            int frames = 0;
            std::uint64_t lastFrame = 0;
            bool reported = false;
        };
        std::unordered_map<std::uint64_t, CutOffItem> cutOffItems;

        // injected input (automation / tests)
        struct InjectedEvent
        {
            enum Kind { Pos, Button, Wheel, Key, Text } kind;
            float x = 0, y = 0;
            int button = 0;
            bool down = false;
            ImGuiKey key = ImGuiKey_None;
            std::string text;
        };
        std::mutex injectMutex;
        std::vector<InjectedEvent> injected;
        bool injectActive = false;
        Vec2 injectPos;
        void ApplyInjectedInput();

        // IME: inline composition for WGT text fields. Window thread (HandleWin32Message) <-> UI thread.
        std::mutex imeMutex;
        std::wstring imeComposition;          // text being composed (shown inline by the focused field)
        int imeCursor = 0;                    // caret inside the composition (UTF-16)
        std::wstring imeCommitted;            // committed text not yet consumed by the field
        std::atomic<bool> imeField{false};    // a WGT text field has keyboard focus
        std::atomic<int> imeCaretX{0}, imeCaretY{0}, imeCaretH{0};   // caret line, client pixels
        ImGuiID imeOwner = 0;                 // UI thread: the field that owns the IME
        void ReadImeComposition(HWND hwnd, LPARAM flags);   // window thread
        void PlaceImeWindows(HWND hwnd);                    // window thread
        void SetImeFocus(ImGuiID field, Vec2 caret, float lineHeight);   // UI thread, every frame while focused
        void ReleaseIme();                                               // UI thread
        std::wstring TakeImeCommitted();
        void GetImeComposition(std::wstring& text, int& cursor);

        // tasks (thread-safe)
        std::mutex taskMutex;
        std::vector<Task> tasks;
        std::vector<Task> taskScratch;

        // panels (thread-safe registry; the UI thread iterates a shared_ptr snapshot without holding the lock)
        mutable std::shared_mutex panelMutex;
        std::vector<std::shared_ptr<PanelEntry>> panels;
        std::atomic<bool> dockVisible{true};
        std::string iniFilename;
        std::atomic<std::uint32_t> nextEffectId{0};

        // island (thread-safe queue)
        std::mutex islandMutex;
        std::vector<IslandItem> islandQueue;
        std::vector<IslandActivity> activities;

        // plugins
        std::mutex pluginMutex;
        std::vector<PluginEntry> plugins;

        // effects
        std::mutex effectMutex;
        std::vector<std::string> effectNames;

        // logging
        std::mutex logMutex;
        LogFn log;

        // stats
        mutable std::mutex statsMutex;
        FrameStats stats;
        float fpsAccum = 0.0f;
        int fpsFrames = 0;

        bool inFrame = false;

        void Log(int level, const char* fmt, ...);
        void ApplyScaleAndTheme();
        void ApplyRenderScale();
        void SyncFontDensity();
        void ProcessInput();
        void RunTasks();
        void DrawPanels();
        void DrawDock();
        void DrawIsland();
        void AttachPendingPlugins();
    };

    // TLS current context
    Context::Impl* CurrentImpl();
    Context::Impl& RequireImpl();

    // fonts.cpp
    bool LoadFonts(Context::Impl& impl);

    // ui/layout.cpp
    void LayoutNewFrame(Context::Impl& impl);
    // Layout inspector: checks this frame's item map for overlapping siblings (UI thread, before ImGui::Render).
    void LayoutInspect(Context::Impl& impl);
    // Halo for an outer glow drawn into `dl`: the rect the glow must fade out before (neighbouring items of
    // the same window, previous frame) and the fade width. False when nothing is in reach.
    bool ComputeGlowHalo(Context::Impl& impl, const ImDrawList* dl, const Rect& shape, float extent, Rect& bound, float& fade);
    // A shape with visual overflow (glow) was drawn: auto-layout containers reserve room for it next frame.
    void LayoutReportOverflow(Context::Impl& impl, const ImDrawList* dl, const Rect& shape, float margin);

    // island.cpp / dock.cpp
    void IslandFrame(Context::Impl& impl);
    void DockFrame(Context::Impl& impl);
}
