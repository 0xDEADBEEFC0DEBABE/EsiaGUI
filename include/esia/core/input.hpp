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
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        LeftCtrl, RightCtrl, LeftShift, RightShift, LeftAlt, RightAlt, LeftSuper, RightSuper,
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

    struct InputEvent
    {
        enum class Type : std::uint8_t
        {
            MousePos,        // pos (UI units); a negative pos = mouse left the window
            MouseButton,     // button, down
            MouseWheel,      // wheel (x, y: notches, y > 0 = away from the user)
            Key,             // key, down, mods
            Text,            // text (UTF-8, committed characters)
            ImeComposition,  // text = composition string (empty = composition ended), imeCursor = caret (bytes)
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

        static InputEvent MouseMove(Vec2 p) { InputEvent e; e.type = Type::MousePos; e.pos = p; return e; }
        static InputEvent Button(MouseButton b, bool isDown) { InputEvent e; e.type = Type::MouseButton; e.button = b; e.down = isDown; return e; }
        static InputEvent Wheel(float x, float y) { InputEvent e; e.type = Type::MouseWheel; e.wheel = Vec2(x, y); return e; }
        static InputEvent KeyEvent(Key k, bool isDown, std::uint32_t m = 0) { InputEvent e; e.type = Type::Key; e.key = k; e.down = isDown; e.mods = m; return e; }
        static InputEvent TextEvent(std::string utf8) { InputEvent e; e.type = Type::Text; e.text = std::move(utf8); return e; }
        static InputEvent Composition(std::string utf8, int cursor) { InputEvent e; e.type = Type::ImeComposition; e.text = std::move(utf8); e.imeCursor = cursor; return e; }
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

        // ---- mouse
        Vec2 MousePos() const { return mousePos_; }
        Vec2 MouseDelta() const { return mouseDelta_; }
        bool MouseValid() const { return mousePos_.x >= 0.0f && mousePos_.y >= 0.0f; }
        bool MouseDown(MouseButton b) const { return mouse_[Idx(b)].down; }
        bool MouseClicked(MouseButton b) const { return mouse_[Idx(b)].clicked; }
        bool MouseReleased(MouseButton b) const { return mouse_[Idx(b)].released; }
        bool MouseDoubleClicked(MouseButton b) const { return mouse_[Idx(b)].doubleClicked; }
        float MouseDownDuration(MouseButton b) const { return mouse_[Idx(b)].down ? (float)(time_ - mouse_[Idx(b)].downTime) : -1.0f; }
        Vec2 MouseClickedPos(MouseButton b) const { return mouse_[Idx(b)].clickPos; }
        // Movement since the press (zero until it passed the drag threshold, unless threshold < 0).
        Vec2 MouseDragDelta(MouseButton b, float threshold = -1.0f) const;
        bool MouseDragging(MouseButton b, float threshold = -1.0f) const;
        Vec2 Wheel() const { return wheel_; }

        // ---- keyboard
        bool KeyDown(Key k) const { return keys_[Idx(k)].down; }
        bool KeyPressed(Key k, bool repeat = true) const;
        bool KeyReleased(Key k) const { return keys_[Idx(k)].released; }
        std::uint32_t Mods() const { return mods_; }

        // ---- text
        const std::u32string& Text() const { return text_; }   // characters typed this frame
        const std::string& Composition() const { return composition_; }
        int CompositionCursor() const { return compositionCursor_; }
        bool Focused() const { return focused_; }
        double Time() const { return time_; }
        float DeltaTime() const { return deltaTime_; }

    private:
        struct ButtonState
        {
            bool down = false, clicked = false, released = false, doubleClicked = false, changed = false;
            double downTime = 0.0, lastClickTime = -1e9;
            Vec2 clickPos, lastClickPos;
            float maxDistSq = 0.0f;
        };
        struct KeyState
        {
            bool down = false, pressed = false, released = false, changed = false;
            double downTime = 0.0;
        };
        static int Idx(MouseButton b) { return (int)b; }
        static int Idx(Key k) { return (int)k; }
        bool Apply(const InputEvent& e);   // false: must wait for the next frame

        double time_ = 0.0;
        float deltaTime_ = 0.0f;
        bool started_ = false;
        Vec2 mousePos_{-1, -1}, mousePrev_{-1, -1}, mouseDelta_;
        Vec2 wheel_;
        std::array<ButtonState, (int)MouseButton::Count> mouse_{};
        std::array<KeyState, (int)Key::Count> keys_{};
        std::uint32_t mods_ = 0;
        std::u32string text_;
        std::string composition_;
        int compositionCursor_ = 0;
        bool focused_ = true;
    };

    // UTF-8 helpers used by the input and text code.
    ESIA_API void AppendUtf8(std::u32string& out, const std::string& utf8);
    ESIA_API void AppendUtf32(std::string& out, char32_t c);
}
