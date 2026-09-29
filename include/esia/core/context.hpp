// Esia - UI core context: the immediate-mode machinery that WGT took from Dear ImGui, rewritten.
//
// What it does, per frame, on the UI thread:
//   * applies queued input (InputState), updates window moves / resizes and wheel scrolling;
//   * windows: z-order, focus, move / resize, per-window clip rect, scroll, draw list;
//   * ids: the id stack and label hashing (esia/base/hash.hpp);
//   * items: registration (ItemAdd / ItemSize), hover / active / keyboard-focus logic (ButtonBehavior), the
//     layout cursor (SameLine, groups, indent);
//   * output: DrawData (draw lists back to front) and PlatformRequests (cursor, IME rect, text input).
//
// The core draws nothing itself: window backgrounds, title bars and widgets are drawn by the widget layer with
// Painter into Window::DrawList(). It has no platform headers and no global state: several contexts may live in
// one process (one per thread). QueueInput and the texture registry are thread-safe, everything else is UI thread.
#pragma once
#include "esia/base/hash.hpp"
#include "esia/core/draw_list.hpp"
#include "esia/core/input.hpp"
#include "esia/core/texture.hpp"
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

namespace esia
{
    enum WindowFlags_ : std::uint32_t
    {
        WindowFlags_None = 0,
        WindowFlags_NoMove = 1u << 0,          // dragging its empty area does not move it
        WindowFlags_NoResize = 1u << 1,        // no resize borders
        WindowFlags_NoInputs = 1u << 2,        // never hovered: the mouse goes to what is below
        WindowFlags_NoScroll = 1u << 3,        // the wheel does not scroll it
        WindowFlags_NoBringToFront = 1u << 4,  // focusing it keeps its place in the z-order
        WindowFlags_NoFocus = 1u << 5,         // clicking it does not focus it
    };

    // Draw order: every window of a higher layer is above every window of a lower one.
    enum class WindowLayer : std::uint8_t { Background = 0, Normal = 1, Overlay = 2, Tooltip = 3 };

    enum class Cond : std::uint8_t { Always, FirstUse };

    struct WindowOptions
    {
        std::uint32_t flags = WindowFlags_None;
        WindowLayer layer = WindowLayer::Normal;
        Vec2 minSize{32, 32};
        Vec2 maxSize{1e6f, 1e6f};
        Vec2 padding{-1, -1};        // < 0 = LayoutMetrics::windowPadding
    };

    enum ItemFlags_ : std::uint32_t
    {
        ItemFlags_None = 0,
        ItemFlags_AllowOverlap = 1u << 0,   // a later overlapping item may take the hover (then this one loses it)
        ItemFlags_Disabled = 1u << 1,       // never hovered / active
        ItemFlags_Focusable = 1u << 2,      // takes part in Tab navigation of keyboard focus
    };

    enum ButtonFlags_ : std::uint32_t
    {
        ButtonFlags_None = 0,
        ButtonFlags_MouseLeft = 1u << 0,        // default when no mouse button is given
        ButtonFlags_MouseRight = 1u << 1,
        ButtonFlags_MouseMiddle = 1u << 2,
        ButtonFlags_PressOnClick = 1u << 3,     // pressed on mouse down (default: released over the item)
        ButtonFlags_PressOnDoubleClick = 1u << 4,
        ButtonFlags_Repeat = 1u << 5,           // pressed repeatedly while held (key-repeat timing)
        ButtonFlags_AllowOverlap = 1u << 6,
        ButtonFlags_FocusOnClick = 1u << 7,     // a click gives the item keyboard focus
    };

    struct ButtonResult
    {
        bool hovered = false;
        bool held = false;
        bool pressed = false;
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
    };

    struct LayoutMetrics
    {
        Vec2 windowPadding{12, 12};
        Vec2 itemSpacing{8, 8};
        float indent = 16.0f;
        float resizeBorder = 5.0f;          // UI units on each side of a window edge
        float scrollStep = 48.0f;           // UI units per wheel notch
    };

    struct ContextDesc
    {
        InputConfig input;
        LayoutMetrics layout;
        // Clipboard of the platform (optional). Called on the UI thread.
        std::function<std::string()> getClipboard;
        std::function<void(const std::string&)> setClipboard;
    };

    struct FrameParams
    {
        Vec2 displaySize{1280, 720};        // UI units
        Vec2 framebufferScale{1, 1};        // render-target pixels per UI unit
        double time = 0.0;                  // seconds, monotonic
    };

    class Context;

    class ESIA_API Window
    {
    public:
        Id GetId() const { return id_; }
        const std::string& Name() const { return name_; }
        const Rect& GetRect() const { return rect_; }
        Rect ContentRect() const { return rect_.Expanded(-padding_.x, -padding_.y); }
        Vec2 Scroll() const { return scroll_; }
        Vec2 ScrollMax() const { return scrollMax_; }
        Vec2 ContentSize() const { return contentSize_; }
        WindowLayer Layer() const { return layer_; }
        std::uint32_t Flags() const { return flags_; }
        bool Appearing() const { return appearing_; }   // first frame after it was not submitted
        DrawList& GetDrawList() { return drawList_; }
        const DrawList& GetDrawList() const { return drawList_; }

    private:
        friend class Context;
        struct Group
        {
            Vec2 cursor, cursorMax, lineStart;
            float indent, lineHeight;
        };

        Id id_ = 0;
        std::string name_;
        Rect rect_;
        Vec2 padding_;
        Vec2 minSize_, maxSize_;
        Vec2 scroll_, scrollMax_, contentSize_;
        std::uint32_t flags_ = 0;
        WindowLayer layer_ = WindowLayer::Normal;
        bool active_ = false, wasActive_ = false, appearing_ = true;
        std::uint64_t lastFrame_ = 0;
        DrawList drawList_;
        std::vector<Id> idStack_;
        // layout cursor
        Vec2 cursor_, cursorStart_, cursorMax_, prevLineEnd_;
        float lineHeight_ = 0.0f, prevLineHeight_ = 0.0f, indent_ = 0.0f;
        bool sameLine_ = false;
        std::vector<Group> groups_;
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
        const LayoutMetrics& Metrics() const { return desc_.layout; }
        LayoutMetrics& Metrics() { return desc_.layout; }
        Vec2 DisplaySize() const { return params_.displaySize; }
        Vec2 FramebufferScale() const { return params_.framebufferScale; }

        // ---- windows
        void SetNextWindowPos(Vec2 pos, Cond cond = Cond::Always);
        void SetNextWindowSize(Vec2 size, Cond cond = Cond::Always);
        // Returns false when the window is fully clipped / collapsed to nothing (End must be called anyway).
        bool Begin(std::string_view name, const WindowOptions& options = {});
        void End();
        Window* CurrentWindow() const { return stack_.empty() ? nullptr : stack_.back(); }
        Window* FindWindow(std::string_view name) const;
        Window* HoveredWindow() const { return hoveredWindow_; }
        Window* FocusedWindow() const { return focusedWindow_; }
        void FocusWindow(Window* w);
        // Windows in draw order (back to front) that were submitted last frame.
        std::vector<Window*> WindowsInDrawOrder() const;
        DrawList& ForegroundDrawList() { return foreground_; }
        DrawList& BackgroundDrawList() { return background_; }
        DrawList& WindowDrawList() { return CurrentWindow()->drawList_; }

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
        // Advances the layout cursor past an item of `size` placed at the cursor.
        void ItemSize(Vec2 size);
        bool ItemHoverable(Id id, const Rect& bb, std::uint32_t itemFlags = ItemFlags_None);
        ButtonResult ButtonBehavior(Id id, const Rect& bb, std::uint32_t buttonFlags = ButtonFlags_None);
        Id LastItemId() const { return lastItemId_; }
        const Rect& LastItemRect() const { return lastItemRect_; }
        bool LastItemHovered() const { return lastItemHovered_; }

        Id HoveredId() const { return hoveredId_; }
        Id ActiveId() const { return activeId_; }
        void SetActiveId(Id id);
        void ClearActiveId() { SetActiveId(0); }
        // The active item must call this every frame it is submitted without ItemAdd, or it is deactivated.
        void KeepAliveId(Id id);
        Id KeyboardFocusId() const { return focusId_; }
        void SetKeyboardFocusId(Id id);
        // The item with keyboard focus edits text this frame: the platform shows the IME at `caret`.
        void RequestTextInput(const Rect& caret);
        void SetMouseCursor(MouseCursor c) { requests_.cursor = c; }

        // ---- layout cursor (current window, UI units, absolute)
        Vec2 CursorPos() const;
        void SetCursorPos(Vec2 pos);
        void SameLine(float offsetFromStartX = 0.0f, float spacing = -1.0f);
        void NewLine();
        void Spacing();
        void Indent(float width = 0.0f);
        void Unindent(float width = 0.0f);
        Vec2 ContentRegionAvail() const;
        void BeginGroup();
        void EndGroup();

        // ---- clipping (current window draw list and item visibility)
        void PushClipRect(const Rect& r, bool intersect = true);
        void PopClipRect();

        // ---- scroll (current window)
        void SetScrollY(float y);
        void SetScrollX(float x);

        // ---- clipboard (the platform callbacks of ContextDesc)
        std::string GetClipboardText() const { return desc_.getClipboard ? desc_.getClipboard() : std::string(); }
        void SetClipboardText(const std::string& s) const { if (desc_.setClipboard) desc_.setClipboard(s); }

    private:
        Window* CreateWindow(std::string_view name, Id id);
        void UpdateHoveredWindow();
        void UpdateMoveResize();
        void StartResize();
        void UpdateScroll();
        void UpdateFocusNavigation();
        int ResizeEdgesAt(const Window& w, Vec2 p) const;
        Id MoveId(const Window& w) const { return HashString("#move", w.id_); }
        Id ResizeId(const Window& w) const { return HashString("#resize", w.id_); }

        ContextDesc desc_;
        FrameParams params_;
        std::uint64_t frame_ = 0;
        bool inFrame_ = false;

        std::mutex inputMutex_;
        std::vector<InputEvent> queued_;
        InputState input_;
        TextureRegistry textures_;

        std::vector<std::unique_ptr<Window>> windows_;   // creation order
        std::vector<Window*> order_;                     // z-order, back to front (per layer when drawn)
        std::vector<Window*> stack_;                     // Begin / End nesting
        Window* root_ = nullptr;                         // implicit full-display background window
        Window* hoveredWindow_ = nullptr;
        Window* focusedWindow_ = nullptr;
        Vec2 nextPos_, nextSize_;
        Cond nextPosCond_ = Cond::Always, nextSizeCond_ = Cond::Always;
        bool hasNextPos_ = false, hasNextSize_ = false;

        Id hoveredId_ = 0, hoveredIdPrev_ = 0;
        bool hoveredAllowOverlap_ = false;
        Id activeId_ = 0;
        bool activeAlive_ = false, activeSetThisFrame_ = false;
        MouseButton activeButton_ = MouseButton::Left;
        Id focusId_ = 0;
        bool focusAlive_ = false, focusClaimed_ = false, textInputRequested_ = false;
        std::vector<Id> focusOrder_, focusOrderPrev_;
        Id lastItemId_ = 0;
        Rect lastItemRect_;
        bool lastItemHovered_ = false;

        // window move / resize in progress
        Window* dragWindow_ = nullptr;
        Vec2 dragOffset_;
        int resizeEdges_ = 0;   // 1 left, 2 right, 4 top, 8 bottom

        DrawList background_, foreground_;
        DrawData drawData_;
        PlatformRequests requests_;
    };
}
