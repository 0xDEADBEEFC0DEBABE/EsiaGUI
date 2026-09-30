// Esia - input: events from the platform layer, per-frame state for widgets.
//
// The platform layer (Win32, Cocoa, X11 / Wayland, SDL, a game engine ...) translates its native events into
// InputEvents and queues them on the Context from any thread. Once per frame the context applies the queue
// to an InputState, which widgets query: positions and buttons, clicks, double clicks, drags, keys with
// repeat, text and IME composition.
//
// Events are applied in order. A button or key that changes twice in one frame (a quick click between two
// frames) keeps the second change for the next frame, so the press is still seen as a click.
#pragma once
#include "esia/base/math.hpp"
#include <array>
#include <cfloat>
#include <string>
#include <vector>

namespace esia
{
    enum class Key : std::uint16_t
    {
        None = 0,
        Tab, Left, Right, Up, Down, PageUp, PageDown, Home, End, Insert, Delete, Backspace, Space, Enter, Escape,
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24,
        LeftCtrl, RightCtrl, LeftShift, RightShift, LeftAlt, RightAlt, LeftSuper, RightSuper,
        // the keypad (its Enter is Enter; with Num Lock off its keys arrive as Home, Left ... and Delete)
        Keypad0, Keypad1, Keypad2, Keypad3, Keypad4, Keypad5, Keypad6, Keypad7, Keypad8, Keypad9,
        KeypadDecimal, KeypadDivide, KeypadMultiply, KeypadSubtract, KeypadAdd,
        // punctuation keys by their position on a US keyboard (what they type depends on the layout: use Text())
        Apostrophe, Comma, Minus, Period, Slash, Semicolon, Equal, LeftBracket, Backslash, RightBracket, GraveAccent,
        CapsLock, ScrollLock, NumLock, PrintScreen, Pause, Menu,
        Count
    };

    enum class MouseButton : std::uint8_t { Left = 0, Right, Middle, X1, X2, Count };

    enum Modifiers_ : std::uint32_t
    {
        Mod_None = 0,
        Mod_Ctrl = 1u << 0,
        Mod_Shift = 1u << 1,
        Mod_Alt = 1u << 2,
        Mod_Super = 1u << 3,
    };

    // The mouse position when there is no mouse (it left the window, or focus was lost). Every other value,
    // negative ones included, is a position: drags past the left / top edge and monitors left of or above the
    // primary one are ordinary coordinates.
    constexpr float kNoMousePos = -FLT_MAX;

    struct InputEvent
    {
        enum class Type : std::uint8_t
        {
            MousePos,        // pos (UI units); kNoMousePos = the mouse left the window
            MouseButton,     // button, down
            MouseWheel,      // wheel (x, y: notches; y > 0 = away from the user, scrolls toward the top; x > 0 scrolls
                             // toward the left, i.e. a tilt / swipe to the left)
            Key,             // key, down, mods
            Text,            // text (UTF-8, committed characters)
            ImeComposition,  // text = composition string (empty = composition ended), imeCursor = caret (bytes),
                             // [imeTargetBegin, imeTargetEnd) = the clause being converted (bytes; empty: none)
            Focus,           // down = the window gained focus
        };
        Type type = Type::MousePos;
        Vec2 pos;
        Vec2 wheel;
        MouseButton button = MouseButton::Left;
        Key key = Key::None;
        bool down = false;
        std::uint32_t mods = 0;
        std::string text;
        int imeCursor = 0;
        int imeTargetBegin = 0, imeTargetEnd = 0;

        static InputEvent MouseMove(Vec2 p) { InputEvent e; e.type = Type::MousePos; e.pos = p; return e; }
        static InputEvent MouseLeave() { return MouseMove(Vec2(kNoMousePos, kNoMousePos)); }
        static InputEvent Button(MouseButton b, bool isDown) { InputEvent e; e.type = Type::MouseButton; e.button = b; e.down = isDown; return e; }
        static InputEvent Wheel(float x, float y) { InputEvent e; e.type = Type::MouseWheel; e.wheel = Vec2(x, y); return e; }
        static InputEvent KeyEvent(Key k, bool isDown, std::uint32_t m = 0) { InputEvent e; e.type = Type::Key; e.key = k; e.down = isDown; e.mods = m; return e; }
        static InputEvent TextEvent(std::string utf8) { InputEvent e; e.type = Type::Text; e.text = std::move(utf8); return e; }
        static InputEvent Composition(std::string utf8, int cursor, int targetBegin = 0, int targetEnd = 0)
        {
            InputEvent e;
            e.type = Type::ImeComposition;
            e.text = std::move(utf8);
            e.imeCursor = cursor;
            e.imeTargetBegin = targetBegin;
            e.imeTargetEnd = targetEnd;
            return e;
        }
        static InputEvent FocusEvent(bool focused) { InputEvent e; e.type = Type::Focus; e.down = focused; return e; }
    };

    struct InputConfig
    {
        float doubleClickTime = 0.30f;       // seconds
        float doubleClickDistance = 6.0f;    // UI units
        float dragThreshold = 6.0f;          // UI units before a press counts as a drag
        float keyRepeatDelay = 0.275f;
        float keyRepeatRate = 0.050f;
    };

    class ESIA_API InputState
    {
    public:
        InputConfig config;

        // Applies `events` for a frame at `time` (seconds). Events that must wait for the next frame (a second
        // change of the same button / key) are left in `events`; the others are removed.
        void NewFrame(double time, std::vector<InputEvent>& events);

        // ---- mouse (a button outside MouseButton reads as up)
        Vec2 MousePos() const { return mousePos_; }
        Vec2 MouseDelta() const { return mouseDelta_; }
        bool MouseValid() const { return mousePos_.x != kNoMousePos && mousePos_.y != kNoMousePos; }
        bool MouseDown(MouseButton b) const { return Button(b).down; }
        bool MouseClicked(MouseButton b) const { return Button(b).clicked; }
        bool MouseReleased(MouseButton b) const { return Button(b).released; }
        // The release this frame came from focus loss, not from the user: nothing may treat it as a click.
        bool MouseCanceled(MouseButton b) const { return Button(b).canceled; }
        // Consecutive clicks of this press (1, 2 = double click, 3 = triple click ...), within doubleClickTime and
        // doubleClickDistance of the previous one. Kept while the button is held; 0 before the first click.
        int MouseClickCount(MouseButton b) const { return Button(b).clickCount; }
        bool MouseDoubleClicked(MouseButton b) const { return Button(b).clicked && Button(b).clickCount == 2; }
        float MouseDownDuration(MouseButton b) const { return Button(b).down ? (float)(time_ - Button(b).downTime) : -1.0f; }
        Vec2 MouseClickedPos(MouseButton b) const { return Button(b).clickPos; }
        // Movement since the press (zero until it passed the drag threshold, unless threshold < 0).
        Vec2 MouseDragDelta(MouseButton b, float threshold = -1.0f) const;
        bool MouseDragging(MouseButton b, float threshold = -1.0f) const;
        Vec2 Wheel() const { return wheel_; }

        // ---- keyboard
        bool KeyDown(Key k) const { return KeyAt(k).down; }
        bool KeyPressed(Key k, bool repeat = true) const;
        bool KeyReleased(Key k) const { return KeyAt(k).released; }
        // The modifiers after the frame's last key event: for modifier state (Shift held while dragging).
        std::uint32_t Mods() const { return mods_; }
        // The modifiers held when `k` was last pressed: for shortcuts. A fast Ctrl+V whose Ctrl is released in the
        // same frame reads Mods() == 0 but KeyMods(Key::V) == Mod_Ctrl.
        std::uint32_t KeyMods(Key k) const { return KeyAt(k).pressMods; }

        // ---- text
        const std::u32string& Text() const { return text_; }   // characters typed this frame
        const std::string& Composition() const { return composition_; }
        int CompositionCursor() const { return compositionCursor_; }
        // The clause of the composition the IME is converting (bytes, [begin, end); begin == end: none). Editors
        // draw it with a thicker underline, as Windows editors do.
        int CompositionTargetBegin() const { return compositionTarget_[0]; }
        int CompositionTargetEnd() const { return compositionTarget_[1]; }
        bool Focused() const { return focused_; }
        double Time() const { return time_; }
        float DeltaTime() const { return deltaTime_; }

    private:
        struct ButtonState
        {
            bool down = false, clicked = false, released = false, canceled = false, changed = false;
            int clickCount = 0;
            double downTime = 0.0, lastClickTime = -1e9;
            Vec2 clickPos, lastClickPos;
            float maxDistSq = 0.0f;
        };
        struct KeyState
        {
            bool down = false, pressed = false, released = false, changed = false;
            double downTime = 0.0;
            std::uint32_t pressMods = 0;
        };
        // out-of-range values (a cast from a platform code) read the always-up state instead of past the arrays
        const ButtonState& Button(MouseButton b) const { return (std::size_t)b < mouse_.size() ? mouse_[(std::size_t)b] : kUp; }
        const KeyState& KeyAt(Key k) const { return (std::size_t)k < keys_.size() ? keys_[(std::size_t)k] : kKeyUp; }
        static const ButtonState kUp;
        static const KeyState kKeyUp;
        bool Apply(const InputEvent& e);   // false: must wait for the next frame

        double time_ = 0.0;
        float deltaTime_ = 0.0f;
        bool started_ = false;
        Vec2 mousePos_{kNoMousePos, kNoMousePos}, mousePrev_{kNoMousePos, kNoMousePos}, mouseDelta_;
        Vec2 wheel_;
        std::array<ButtonState, (int)MouseButton::Count> mouse_{};
        std::array<KeyState, (int)Key::Count> keys_{};
        std::uint32_t mods_ = 0;
        std::u32string text_;
        std::string composition_;
        int compositionCursor_ = 0;
        int compositionTarget_[2] = {0, 0};
        bool focused_ = true;
    };
}
