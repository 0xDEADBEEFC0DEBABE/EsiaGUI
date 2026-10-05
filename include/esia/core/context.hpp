// Esia - UI core context: the immediate-mode machinery that WGT took from Dear ImGui, rewritten.
//
// What it does, per frame, on the UI thread (docs/UI_CORE.md has the design, section by section):
//   * applies queued input (InputState), records who owns each mouse press, updates window moves / resizes;
//   * hit tests last frame's windows and items front to back: the hovered window and item;
//   * windows: z-order, focus, move / resize, per-window clip, scroll, draw list, DPI scale; popups and tooltips;
//   * ids: the id stack and label hashing (esia/base/hash.hpp);
//   * items: registration (ItemAdd / ItemSize), hover / active / keyboard focus (ButtonBehavior), key ownership;
//   * layout: containers with the layout cursor (SameLine, groups, indent) or a LayoutProvider, child regions with
//     their own clip and scroll;
//   * output: DrawData (draw lists back to front) and PlatformRequests (cursor, capture flags, IME rect, pending
//     input).
//
// The core draws nothing itself: window backgrounds, title bars and widgets are drawn by the widget layer with
// Painter into Window::GetDrawList(). It has no platform headers and no global state: several contexts may live in
// one process (one per thread). QueueInput and the texture registry are thread-safe, everything else is UI thread.
#pragma once
#include "esia/base/hash.hpp"
#include "esia/core/draw_list.hpp"
#include "esia/core/input.hpp"
#include "esia/core/layout.hpp"
#include "esia/core/state.hpp"
#include "esia/core/texture.hpp"
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace esia
{
    enum WindowFlags_ : std::uint32_t
    {
        WindowFlags_None = 0,
        WindowFlags_NoMove = 1u << 0,          // dragging its empty area does not move it (and it is not kept on screen)
        WindowFlags_NoResize = 1u << 1,        // no resize borders
        WindowFlags_NoInputs = 1u << 2,        // never hovered: the mouse goes to what is below
        WindowFlags_NoScroll = 1u << 3,        // the wheel does not scroll it
        WindowFlags_NoBringToFront = 1u << 4,  // focusing it keeps its place in the z-order
        WindowFlags_NoFocus = 1u << 5,         // clicking it does not focus it
        // sized to the content measured last frame (whole physical pixels, placed on them: what it measures does not
        // depend on where it is); hidden the frame it appears
        WindowFlags_AutoSize = 1u << 6,
    };

    // Draw order: every window of a higher layer is above every window of a lower one.
    enum class WindowLayer : std::uint8_t { Background = 0, Normal = 1, Overlay = 2, Tooltip = 3 };

    // When a SetNextWindow* value applies: every frame, when the window is created, or every time it appears.
    enum class Cond : std::uint8_t { Always, FirstUse, Appearing };

    struct WindowOptions
    {
        std::uint32_t flags = WindowFlags_None;
        WindowLayer layer = WindowLayer::Normal;
        Vec2 minSize{32, 32};
        Vec2 maxSize{1e6f, 1e6f};
        Vec2 padding{-1, -1};        // < 0 = LayoutMetrics::windowPadding
        // UI units: a square at the bottom-right corner that resizes the window both ways, above the items there (where
        // the widget layer draws a grip; a rounded corner lies well inside the edges' bands). 0 = the edges only.
        float resizeGrip = 0.0f;
    };

    enum ItemFlags_ : std::uint32_t
    {
        ItemFlags_None = 0,
        ItemFlags_Disabled = 1u << 0,     // claims the hover (nothing below gets it) but never hovers or activates
        ItemFlags_Focusable = 1u << 1,    // takes part in Tab navigation of keyboard focus
        ItemFlags_Background = 1u << 2,   // hit below the other items of its window / child, whenever it is submitted
    };

    enum ButtonFlags_ : std::uint32_t
    {
        ButtonFlags_None = 0,
        ButtonFlags_MouseLeft = 1u << 0,        // default when no mouse button is given
        ButtonFlags_MouseRight = 1u << 1,
        ButtonFlags_MouseMiddle = 1u << 2,
        ButtonFlags_PressOnClick = 1u << 3,     // pressed on mouse down (default: released over the item)
        ButtonFlags_PressOnDoubleClick = 1u << 4,
        ButtonFlags_Repeat = 1u << 5,           // pressed on the click, then repeatedly while held (key-repeat timing)
        ButtonFlags_FocusOnClick = 1u << 6,     // a click gives the item keyboard focus
    };

    struct ButtonResult
    {
        bool hovered = false;
        bool held = false;
        bool pressed = false;
        int clicks = 0;               // click count of the press that activated the item (2 = double click ...)
    };

    // What the core knows about an item this frame (for styling: a theme picks colors from it).
    struct ItemStatus
    {
        bool hovered = false;
        bool active = false;
        bool focused = false;
        bool disabled = false;
        bool pressed = false;         // ButtonBehavior pressed it this frame
        bool visible = false;         // ItemAdd found it inside the clip
        Rect rect;
    };

    enum class MouseCursor : std::uint8_t { Arrow, TextInput, Hand, ResizeEW, ResizeNS, ResizeNWSE, ResizeNESW, ResizeAll };

    // What the platform layer should do after a frame.
    struct PlatformRequests
    {
        MouseCursor cursor = MouseCursor::Arrow;
        bool wantCaptureMouse = false;      // the UI uses the mouse: do not pass it to the game
        bool wantCaptureKeyboard = false;
        bool wantTextInput = false;         // an editor has keyboard focus: show the IME / on-screen keyboard
        Rect imeRect;                       // caret rect for the IME candidate window (UI units)
        bool inputPending = false;          // input waits for the next frame: run one even without a new event
        bool animating = false;             // the core moves something (a smooth scroll): keep producing frames
    };

    struct LayoutMetrics
    {
        Vec2 windowPadding{12, 12};
        Vec2 itemSpacing{8, 8};
        float indent = 16.0f;
        float resizeBorder = 5.0f;          // UI units on each side of a window edge
        float scrollStep = 48.0f;           // UI units per wheel notch
        float scrollSmoothing = 0.08f;      // seconds: time constant of ChildFlags_SmoothScroll
        float keepOnScreen = 24.0f;         // UI units of a movable window that always stay inside the display
        Vec2 tooltipOffset{16, 16};         // from the mouse
    };

    struct ContextDesc
    {
        InputConfig input;
        LayoutMetrics layout;
        // Frames a window, a child region's scroll state or a State<T> entry is kept after its last use.
        std::uint32_t retainFrames = 600;
        // Clipboard of the platform (optional). Called on the UI thread.
        std::function<std::string()> getClipboard;
        std::function<void(const std::string&)> setClipboard;
    };

    struct FrameParams
    {
        Vec2 displaySize{1280, 720};        // UI units
        Vec2 framebufferScale{1, 1};        // render-target pixels per UI unit
        double time = 0.0;                  // seconds, monotonic
        // UI units: the part of the display the system draws nothing over - on phones not under the camera housing,
        // the rounded corners or the home indicator. Empty = the whole display (desktops).
        Rect safeArea;
    };

    // A monitor of the platform (Context::SetMonitors): windows take the scale of the monitor under their center.
    struct Monitor
    {
        Rect rect;                          // UI units, in the display's coordinates
        float scale = 1.0f;                 // physical pixels per UI unit
    };

    // Containers (docs/UI_CORE.md, section 6).
    struct ContainerOptions
    {
        LayoutProvider* layout = nullptr;   // null = the layout cursor
        Vec2 size;                          // > 0 fixed, else fitted to the content (per axis)
        bool fillWidth = false;             // width = what is available (minus |size.x| when size.x < 0)
        Vec2 padding;
    };

    enum ChildFlags_ : std::uint32_t
    {
        ChildFlags_None = 0,
        ChildFlags_ScrollX = 1u << 0,
        ChildFlags_ScrollY = 1u << 1,
        ChildFlags_NoWheel = 1u << 2,       // the wheel passes through to the parents
        ChildFlags_SmoothScroll = 1u << 3,  // the offset glides to its target
        ChildFlags_Floating = 1u << 4,      // placed at ChildOptions::rect over the content, hit above it
    };

    struct ChildOptions
    {
        Vec2 size;                          // > 0 fixed, 0 = what is available, < 0 = available + size (per axis)
        Vec2 padding;
        std::uint32_t flags = ChildFlags_None;
        Rect clipInset;                     // moves the clip in from the content rect (left, top, right, bottom); < 0 = out
        Rect rect;                          // ChildFlags_Floating: where it floats
    };

    struct PopupOptions
    {
        Vec2 pos{kNoMousePos, kNoMousePos}; // anchor; default: the mouse position when it was opened
        Vec2 pivot{0, 0};                   // which point of the popup sits on the anchor (0,0 top-left, 1,0 top-right)
        Vec2 minSize{0, 0};
        Vec2 maxSize{1e6f, 1e6f};
        Vec2 padding{-1, -1};               // < 0 = LayoutMetrics::windowPadding
        bool consumeClickAway = true;       // the click that dismisses it does nothing else
    };

    class Context;

    class ESIA_API Window
    {
    public:
        Id GetId() const { return id_; }
        const std::string& Name() const { return name_; }
        const Rect& GetRect() const { return rect_; }
        Rect ContentRect() const { return rect_.Expanded(-padding_.x, -padding_.y); }
        Vec2 Scroll() const { return scroll_.scroll; }
        Vec2 ScrollMax() const { return scroll_.max; }
        Vec2 ContentSize() const { return scroll_.content; }
        WindowLayer Layer() const { return layer_; }
        std::uint32_t Flags() const { return flags_; }
        bool Appearing() const { return appearing_; }     // first frame after it was not submitted
        bool Hidden() const { return hidden_; }           // submitted but not drawn or hit (AutoSize's first frame)
        float Scale() const { return scale_; }            // physical pixels per UI unit (its monitor's)
        bool ScaleChanged() const { return scaleChanged_; }
        // The items laid out in it in the previous frame (rects clipped to what was visible).
        const std::vector<LaidOutItem>& LaidOutItems() const { return laidOutPrev_; }
        DrawList& GetDrawList() { return drawList_; }
        const DrawList& GetDrawList() const { return drawList_; }

    private:
        friend class Context;

        // a scroll offset with its range and deferred target (a window's content, a scroll child)
        struct ScrollState
        {
            Vec2 scroll, max, content, view;
            Vec2 target;                  // smooth scrolling: where the offset glides
            bool pending[2] = {false, false};
            Vec2 request;                 // SetScroll* this frame: applied when the area ends
        };
        // one container of the layout stack
        struct Frame
        {
            Id id = 0;
            LayoutProvider* layout = nullptr;
            Vec2 origin;                  // outer top-left
            Vec2 contentOrigin;           // where the content starts (scrolled)
            Vec2 padding;
            Vec2 fixedSize;               // ContainerOptions / child size (0 = fitted)
            Rect region;                  // work rect of the current slot
            Vec2 cursor, cursorMax;
            float lineStartX = 0.0f, lineTop = 0.0f, lineHeight = 0.0f, lineBaseline = -1.0f;
            Vec2 prevLineEnd;
            float prevLineHeight = 0.0f, prevLineBaseline = -1.0f;
            float indent = 0.0f;
            float firstBaseline = -1.0f;  // absolute y of the first child's baseline
            bool anyItem = false;
            int childIndex = -1;          // a child region: its record this frame
            ScrollState* scroll = nullptr;
            bool smooth = false;
            std::vector<std::pair<const void*, const void*>> scope;
            // the layout cursor's lines (LineRoom): the frame among this frame's frames (the same from frame to frame),
            // the lines started in it, the items on the current line and the right edges of the first ones
            Id seq = 0;
            int line = -1, lineItems = 0;
            bool sameLine = false;        // SameLine: the next item continues the line
            float lineEdges[4] = {};
        };
        // An item as the next frame's hit test sees it. Content moves when a scroll area it is in scrolls (the wheel,
        // a glide, SetScroll*) before the next layout: the hit test shifts the record by how far its scroll areas
        // moved since it was recorded.
        struct HitRecord
        {
            Id id;
            Rect rect;                    // unclipped
            Rect clip;
            std::uint32_t flags;
            int layer;
            int child;                    // innermost child record it is in, -1 = the window's content
            bool fixed;                   // does not scroll (a floating region's own area)
            Vec2 total, own;              // scroll offsets when recorded: all its areas', its innermost area's
        };
        struct ChildRecord
        {
            Id id;
            int parent;                   // index of the enclosing child record, -1 = the window
            Rect clip;                    // its rect inside its parent's clip
            int layer;
            std::uint32_t flags;
            Vec2 outer;                   // its parents' scroll offsets when recorded
        };
        struct ChildState
        {
            ScrollState scroll;
            bool smooth = false;
            std::uint64_t lastFrame = 0;
        };

        Id id_ = 0;
        std::string name_;
        Rect rect_;
        Vec2 padding_;
        Vec2 minSize_, maxSize_;
        ScrollState scroll_;
        std::uint32_t flags_ = 0;
        WindowLayer layer_ = WindowLayer::Normal;
        bool active_ = false, wasActive_ = false, appearing_ = true, hidden_ = false, wasHidden_ = false;
        bool measured_ = false;           // AutoSize: its content was measured at least once
        float resizeGrip_ = 0.0f;         // WindowOptions::resizeGrip
        float scale_ = 1.0f;
        bool scaleKnown_ = false, scaleChanged_ = false;
        std::uint64_t lastFrame_ = 0;
        DrawList drawList_;
        std::vector<Id> idStack_;
        std::vector<Frame> frames_;       // the layout stack: [0] = the window's content
        int floating_ = 0;                // depth of floating children being submitted
        std::vector<HitRecord> hits_, hitsPrev_;
        std::vector<ChildRecord> children_, childrenPrev_;
        std::vector<int> childStack_;     // indices into children_ of the children being submitted
        std::vector<LaidOutItem> laidOut_, laidOutPrev_;
        // per item of a line followed by SameLine items: how far the line reached past it (LineRoom); last frame's
        // sorted by key
        std::vector<std::pair<Id, float>> lineRoom_, lineRoomPrev_;
        std::unordered_map<Id, ChildState, IntHash> childStates_;
    };

    class ESIA_API Context
    {
    public:
        explicit Context(const ContextDesc& desc = {});
        ~Context();
        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;

        // ---- any thread
        void QueueInput(InputEvent e);
        TextureRegistry& Textures() { return textures_; }

        // ---- frame (UI thread)
        void NewFrame(const FrameParams& params);
        void EndFrame();
        const DrawData& GetDrawData() const { return drawData_; }
        const PlatformRequests& Requests() const { return requests_; }
        std::uint64_t FrameCount() const { return frame_; }
        const InputState& Input() const { return input_; }
        // Events wait in the queue for the next frame (a click and its release in one frame): the host must run
        // another frame even if no new event arrives.
        bool InputPending() const { return inputPending_; }
        const LayoutMetrics& Metrics() const { return desc_.layout; }
        LayoutMetrics& Metrics() { return desc_.layout; }
        Vec2 DisplaySize() const { return params_.displaySize; }
        // FrameParams::safeArea, clamped to the display; the whole display when the platform gave none.
        Rect SafeArea() const;
        Vec2 FramebufferScale() const { return params_.framebufferScale; }
        void SetMonitors(std::vector<Monitor> monitors) { monitors_ = std::move(monitors); }
        // Physical pixels per UI unit of the current window (its monitor's): widget metrics and pixel snapping.
        float Scale() const;

        // ---- windows
        void SetNextWindowPos(Vec2 pos, Cond cond = Cond::Always, Vec2 pivot = Vec2(0, 0));
        void SetNextWindowSize(Vec2 size, Cond cond = Cond::Always);
        // Returns false when the window is fully clipped / collapsed to nothing (End must be called anyway).
        bool Begin(std::string_view name, const WindowOptions& options = {});
        void End();
        Window* CurrentWindow() const { return stack_.empty() ? nullptr : stack_.back(); }
        Window* FindWindowByName(std::string_view name) const;
        Window* HoveredWindow() const { return hoveredWindow_; }
        Window* FocusedWindow() const { return focusedWindow_; }
        void FocusWindow(Window* w);
        // Windows in draw order (back to front) that were submitted last frame.
        std::vector<Window*> WindowsInDrawOrder() const;
        DrawList& ForegroundDrawList() { return foreground_; }
        DrawList& BackgroundDrawList() { return background_; }
        DrawList& WindowDrawList() { return CurrentWindow()->drawList_; }
        // The window being moved by a drag (of its empty area, or StartWindowMove); null: none, or a resize.
        Window* MovingWindow() const { return dragWindow_ && resizeEdges_ == 0 ? dragWindow_ : nullptr; }
        // Moves `w` with the mouse from now on, as if it had been grabbed `grab` from its top-left, until the button is
        // let go (docking: a tab dragged out of its dock node). Needs the left button held.
        void StartWindowMove(Window* w, Vec2 grab);

        // ---- popups and tooltips (docs/UI_CORE.md, section 9)
        void OpenPopup(Id id);
        void ClosePopup(Id id);          // and the popups opened from it
        bool IsPopupOpen(Id id) const;
        bool BeginPopup(Id id, const PopupOptions& options = {});   // false = not open (no EndPopup then)
        void EndPopup();
        void CloseCurrentPopup();
        bool BeginTooltip();             // always true; EndTooltip after it
        void EndTooltip();

        // ---- ids
        void PushId(std::string_view label);
        void PushId(std::int64_t value);
        void PushId(const void* ptr) { PushId((std::int64_t)(std::intptr_t)ptr); }
        void PopId();
        Id GetId(std::string_view label) const;
        Id GetId(std::int64_t value) const;
        Id IdSeed() const;

        // ---- items
        // Registers an item; returns true when it is visible (inside the clip rect). id may be 0 (decoration).
        bool ItemAdd(Id id, const Rect& bb, std::uint32_t itemFlags = ItemFlags_None);
        // Advances the layout past an item of `size` placed at the cursor; `baseline` = its text baseline from the
        // top (< 0 = none).
        void ItemSize(Vec2 size, float baseline = -1.0f);
        bool ItemHoverable(Id id, const Rect& bb, std::uint32_t itemFlags = ItemFlags_None);
        // Mouse behavior of a button-like item. The item flags of the ItemAdd just before (same id) apply too.
        ButtonResult ButtonBehavior(Id id, const Rect& bb, std::uint32_t buttonFlags = ButtonFlags_None, std::uint32_t itemFlags = ItemFlags_None);
        Id LastItemId() const { return lastItem_.id; }
        const Rect& LastItemRect() const { return lastItem_.rect; }
        bool LastItemHovered() const { return lastItem_.hovered; }
        ItemStatus LastItemStatus() const;
        ItemStatus ItemStatusOf(Id id) const;

        Id HoveredId() const { return hoveredId_; }
        // The innermost child region under the mouse (last frame's), 0 = none.
        Id HoveredChild() const { return hoveredChild_; }
        Id ActiveId() const { return activeId_; }
        void SetActiveId(Id id);
        void ClearActiveId() { SetActiveId(0); }
        // The active item must call this every frame it is submitted without ItemAdd, or it is deactivated.
        void KeepAliveId(Id id);
        // The active item is a finger's press that has not acted yet: a button that presses on release, or an item
        // that acts on the press (a slider, a field) while ButtonBehavior holds the press back from it. A scroll area
        // the finger then drags along may take the touch over (SetActiveId to its own drag); the item lets go without
        // pressing, as the content of a phone's scroll views does. Mouse presses stay the item's.
        bool ActiveIdYieldsToScroll() const { return activeId_ != 0 && activeYields_; }
        Id KeyboardFocusId() const { return focusId_; }
        void SetKeyboardFocusId(Id id);
        // The item with keyboard focus edits text this frame: the platform shows the IME at `caret`. (The text
        // cursor shape is the widget's: SetMouseCursor while the field is hovered.)
        void RequestTextInput(const Rect& caret);
        void SetMouseCursor(MouseCursor c) { requests_.cursor = c; }

        // ---- keys (docs/UI_CORE.md, section 5)
        // Claims a key for `owner` this frame and the next: other ids asking KeyPressed / KeyDown get false.
        void ClaimKey(Key key, Id owner);
        void ClaimKeyboard(Id owner);    // every key
        Id KeyOwner(Key key) const;
        bool KeyPressed(Key key, Id asker = 0, bool repeat = true) const;
        bool KeyDown(Key key, Id asker = 0) const;

        // ---- layout (current window, UI units, absolute)
        Vec2 CursorPos() const;
        void SetCursorPos(Vec2 pos);
        void SameLine(float offsetFromStartX = 0.0f, float spacing = -1.0f);
        void NewLine();
        void Spacing();
        void Indent(float width = 0.0f);
        void Unindent(float width = 0.0f);
        Vec2 ContentRegionAvail() const;
        Rect WorkRect() const;
        // What the innermost child region being submitted shows (else the window): its rect where it is, padding
        // included, not moved by its own scroll. Bars that float over an area (tab bars, search bars) are placed in it.
        Rect ViewRect() const;
        // The part of ViewRect inside the clips of what it is in (an area scrolled half out of its window: the half
        // still in it). Scroll indicators, which may lie in a parent's padding, stay inside it.
        Rect VisibleViewRect() const;
        // The width the items after the next one on its line took last frame (SameLine), measured from the next
        // item's right edge: an item that fills what is available leaves them this room. 0 = nothing follows it, or
        // an auto-layout container (its provider places the items).
        float LineRoom() const;
        int CurrentDepth() const;
        // The current line's baseline from its top (< 0 = none) and moving the cursor down so an item with
        // `baseline` lines up with it; returns how far it moved.
        float LineBaseline() const;
        float AlignToLineBaseline(float baseline);
        void BeginGroup();
        void EndGroup();
        void BeginContainer(Id id, const ContainerOptions& options = {});
        Rect EndContainer();
        // Called for every laid-out item (null = none).
        void SetItemObserver(std::function<void(const LaidOutItem&)> observer) { observer_ = std::move(observer); }

        // ---- child regions (docs/UI_CORE.md, section 8)
        bool BeginChild(std::string_view id, const ChildOptions& options = {});
        void EndChild();

        // ---- clipping (current window draw list and item visibility)
        void PushClipRect(const Rect& r, bool intersect = true);
        void PopClipRect();

        // ---- scroll (the innermost scroll area being submitted: a scroll child, else the window)
        Vec2 Scroll() const;
        Vec2 ScrollMax() const;
        void SetScrollX(float x);        // applied when the area ends, clamped to this frame's range
        void SetScrollY(float y);
        void SetScrollHereX(float ratio = 0.5f);
        void SetScrollHereY(float ratio = 0.5f);
        // Shows `rect`: the smallest scroll when align < 0, else the rect at `align` of the view (0 start, 1 end).
        void ScrollToRect(const Rect& rect, Vec2 align = Vec2(-1, -1));
        void ScrollToItem(Vec2 align = Vec2(-1, -1)) { ScrollToRect(lastItem_.rect, align); }
        void SetNextScroll(Vec2 scroll);  // the next Begin / BeginChild; a negative component is left alone

        // ---- state and scopes (docs/UI_CORE.md, section 10)
        template <class T>
        T& State(Id id) { return state_.Get<T>(id, frame_); }
        void SetScopeData(const void* key, const void* value);
        const void* FindScopeData(const void* key) const;

        // ---- clipboard (the platform callbacks of ContextDesc)
        std::string GetClipboardText() const { return desc_.getClipboard ? desc_.getClipboard() : std::string(); }
        void SetClipboardText(const std::string& s) const { if (desc_.setClipboard) desc_.setClipboard(s); }

    private:
        enum class PressOwner : std::uint8_t { None, Host, Ui, Dismiss };
        struct PopupEntry
        {
            Id id = 0;
            Id window = 0;
            Vec2 openPos;
            std::uint64_t openFrame = 0, lastFrame = 0;
            bool consumeClickAway = true;
        };
        struct LastItem
        {
            Id id = 0;
            Rect rect;
            std::uint32_t flags = 0;
            bool hovered = false, visible = false, pressed = false;
        };

        Window* BeginWindow(Id id, std::string_view name, const WindowOptions& options, bool fullyOnScreen = false);
        Window* AddWindow(std::string_view name, Id id);
        void FreeUnusedWindows();
        void UpdateHoveredWindow();
        void HitTest();
        void UpdatePressOwners();
        void UpdatePopupsAtNewFrame();
        void UpdateMoveResize();
        void ApplyMoveResize(Window& w, Vec2 p);
        void EndMoveResize(Id expected);
        void StartResize();
        void UpdateWheel();
        void UpdateTabNavigation();
        void KeepOnScreen(Window& w) const;
        float MonitorScale(const Rect& r) const;
        int ResizeEdgesAt(const Window& w, Vec2 p) const;
        Id MoveId(const Window& w) const { return HashString("#move", w.id_); }
        Id ResizeId(const Window& w) const { return HashString("#resize", w.id_); }
        bool PressBlocked() const;       // a held press belongs to the host or to a popup dismissal
        void RecordHit(Window& w, Id id, const Rect& bb, std::uint32_t itemFlags, bool fixed = false);
        void ScrollOffsets(const Window& w, const std::vector<Window::ChildRecord>& records, int index, Vec2& total, Vec2& own) const;
        Rect ChildClipNow(const Window& w, std::size_t index) const;
        void StepSmoothScrolls();
        void ApplyNextScroll(Window::ScrollState& s, bool immediate);
        void ClosePopupsFrom(std::size_t index);
        int PopupIndex(Id id) const;

        // layout helpers (layout.cpp)
        Window::Frame& CurFrame();
        const Window::Frame& CurFrame() const;
        Window::Frame* ScrollFrame();
        const Window::Frame* ScrollFrame() const;
        void InitRootFrame(Window& w);
        void LayOut(Window& w, const Rect& r, float baseline, bool childReport);
        void AdvanceCursor(Window::Frame& f, const Rect& r, float baseline);
        float Snap(float v) const;
        void BeginScroll(Window::ScrollState& s, bool smooth);
        void SnapScroll(Window::ScrollState& s, float scale) const;
        void EndScroll(Window::ScrollState& s, bool smooth);
        void RequestScroll(int axis, float value);

        ContextDesc desc_;
        FrameParams params_;
        std::vector<Monitor> monitors_;
        std::uint64_t frame_ = 0;
        bool inFrame_ = false;
        // The wheel stays with the area it scrolled while it keeps turning (a browser's scroll latching): content
        // moving under the mouse does not take it over halfway. Per axis.
        struct WheelLatch
        {
            Id window = 0;
            Id child = 0;                  // 0 = the window's own scroll
            double time = -1e9;
            Vec2 mouse;
        };
        WheelLatch wheelLatch_[2];

        std::mutex inputMutex_;
        std::vector<InputEvent> queued_;
        InputState input_;
        bool inputPending_ = false;
        TextureRegistry textures_;
        StateStorage state_;

        std::vector<std::unique_ptr<Window>> windows_;   // creation order
        std::vector<Window*> order_;                     // z-order, back to front (per layer when drawn)
        std::vector<Window*> stack_;                     // Begin / End nesting
        Window* root_ = nullptr;                         // implicit full-display background window
        Window* hoveredWindow_ = nullptr;
        Window* focusedWindow_ = nullptr;
        Vec2 nextPos_, nextPivot_, nextSize_, nextScroll_{-1, -1};
        Cond nextPosCond_ = Cond::Always, nextSizeCond_ = Cond::Always;
        bool hasNextPos_ = false, hasNextSize_ = false, hasNextScroll_ = false;

        // hover, activity, focus
        Id hitId_ = 0;                     // front-most item under the mouse from last frame's records
        std::uint32_t hitFlags_ = 0;
        Id hoveredId_ = 0;                 // the enabled item that took the hover this frame
        Id newItemClaim_ = 0;              // an item not recorded last frame took the hover (hitId_ == 0)
        Id hoveredChild_ = 0;
        Id activeId_ = 0;
        bool activeAlive_ = false, activeSetThisFrame_ = false;
        bool activeByMouse_ = false;       // ButtonBehavior activated it with a mouse button (activeButton_)
        bool activeYields_ = false;        // ... by a finger, not yet the item's for good: ActiveIdYieldsToScroll
        bool activeDeferred_ = false;      // ... and acting on the press: held back from the item (ButtonBehavior)
        bool activeSlopSeen_ = false;      // the held-back finger moved past the slop (a scroll area could take it)
        MouseButton activeButton_ = MouseButton::Left;
        Id focusId_ = 0;
        bool focusAlive_ = false, focusClaimed_ = false, textInputRequested_ = false;
        std::vector<Id> focusOrder_;
        LastItem lastItem_;
        std::array<PressOwner, (int)MouseButton::Count> pressOwner_{};
        std::function<void(const LaidOutItem&)> observer_;

        // key ownership: this frame's claims and last frame's
        std::array<Id, (int)Key::Count> keyOwner_{}, keyOwnerPrev_{};
        Id keyboardOwner_ = 0, keyboardOwnerPrev_ = 0;
        bool anyKeyClaim_ = false;

        // popups: the open stack, and the popups being submitted
        std::vector<PopupEntry> popups_;
        std::vector<Id> popupStack_;

        // window move / resize in progress
        Window* dragWindow_ = nullptr;
        Vec2 dragOffset_;
        int resizeEdges_ = 0;   // 1 left, 2 right, 4 top, 8 bottom
        bool animating_ = false;

        DrawList background_, foreground_;
        DrawData drawData_;
        PlatformRequests requests_;
    };
}
