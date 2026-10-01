// Esia - input state (see esia/core/input.hpp)
#include "esia/core/input.hpp"
#include "esia/base/utf8.hpp"
#include <algorithm>
#include <cmath>

namespace esia
{
    const InputState::ButtonState InputState::kUp{};
    const InputState::KeyState InputState::kKeyUp{};

    void InputState::NewFrame(double time, std::vector<InputEvent>& events)
    {
        // the first frame has no previous one (its time may well be 0); a clock going backwards gives 0, not < 0
        deltaTime_ = started_ ? (float)std::max(time - time_, 0.0) : 0.0f;
        started_ = true;
        time_ = time;
        for (ButtonState& b : mouse_)
            b.clicked = b.released = b.canceled = b.changed = false;
        for (KeyState& k : keys_)
            k.pressed = k.released = k.changed = false;
        wheel_ = Vec2(0, 0);
        text_.clear();

        std::size_t used = 0;
        while (used < events.size() && Apply(events[used]))
            ++used;
        events.erase(events.begin(), events.begin() + (std::ptrdiff_t)used);

        const bool prevValid = mousePrev_.x != kNoMousePos && mousePrev_.y != kNoMousePos;
        mouseDelta_ = (MouseValid() && prevValid) ? mousePos_ - mousePrev_ : Vec2(0, 0);
        mousePrev_ = mousePos_;
        for (ButtonState& b : mouse_)
            if (b.down && MouseValid() && b.clickPos.x != kNoMousePos)
                b.maxDistSq = std::max(b.maxDistSq, LengthSq(mousePos_ - b.clickPos));
    }

    bool InputState::Apply(const InputEvent& e)
    {
        switch (e.type)
        {
        case InputEvent::Type::MousePos:
            // a move after a button changed this frame belongs to the next frame: the click lands where it happened
            for (const ButtonState& b : mouse_)
                if (b.changed)
                    return false;
            mousePos_ = e.pos;
            return true;
        case InputEvent::Type::MouseButton:
        {
            if ((std::size_t)e.button >= mouse_.size())
                return true;
            ButtonState& b = mouse_[(std::size_t)e.button];
            if (b.down == e.down)
                return true;
            if (b.changed)
                return false;
            b.changed = true;
            b.down = e.down;
            if (e.down)
            {
                b.clicked = true;
                b.touch = e.touch;
                b.downTime = time_;
                b.clickPos = mousePos_;
                b.maxDistSq = 0.0f;
                // a click close in time and place to the previous one continues its count (double, triple ...)
                const float d = config.doubleClickDistance;
                const bool near = MouseValid() && LengthSq(mousePos_ - b.lastClickPos) <= d * d;
                b.clickCount = (time_ - b.lastClickTime <= config.doubleClickTime && near) ? b.clickCount + 1 : 1;
                b.lastClickTime = time_;
                b.lastClickPos = mousePos_;
            }
            else
                b.released = true;
            return true;
        }
        case InputEvent::Type::MouseWheel:
            wheel_ += e.wheel;
            return true;
        case InputEvent::Type::Key:
        {
            if (e.key == Key::None || e.key >= Key::Count)
                return true;
            KeyState& k = keys_[(std::size_t)e.key];
            if (k.down == e.down)
            {
                mods_ = e.mods;
                return true;
            }
            if (k.changed)
                return false;
            k.changed = true;
            k.down = e.down;
            if (e.down)
            {
                k.pressed = true;
                k.downTime = time_;
                k.pressMods = e.mods;
            }
            else
                k.released = true;
            mods_ = e.mods;
            return true;
        }
        case InputEvent::Type::Text:
            for (std::size_t i = 0; i < e.text.size();)
                text_.push_back(DecodeUtf8(e.text, i));
            return true;
        case InputEvent::Type::ImeComposition:
            composition_ = e.text;
            compositionCursor_ = e.imeCursor;
            {
                // a range inside the string, or none
                const int size = (int)composition_.size();
                const int begin = std::clamp(e.imeTargetBegin, 0, size), end = std::clamp(e.imeTargetEnd, begin, size);
                compositionTarget_[0] = begin;
                compositionTarget_[1] = end;
            }
            return true;
        case InputEvent::Type::Focus:
            focused_ = e.down;
            if (!focused_)
            {
                // Nothing stays stuck down when the window loses focus mid-press. The releases are canceled and the
                // position is invalidated: a button under the last known position must not see "released while
                // hovered" and press. (A second change of the button in this frame waits for the next one.)
                for (ButtonState& b : mouse_)
                    if (b.down)
                    {
                        b.down = false;
                        b.released = b.canceled = b.changed = true;
                        b.clickCount = 0;
                        b.lastClickTime = -1e9;
                    }
                mousePos_ = Vec2(kNoMousePos, kNoMousePos);
                for (KeyState& k : keys_)
                    if (k.down)
                    {
                        k.down = false;
                        k.released = true;
                    }
                mods_ = 0;
            }
            return true;
        }
        return true;
    }

    Vec2 InputState::MouseDragDelta(MouseButton b, float threshold) const
    {
        if (!MouseDragging(b, threshold))
            return Vec2(0, 0);
        return mousePos_ - Button(b).clickPos;
    }

    bool InputState::MouseDragging(MouseButton b, float threshold) const
    {
        const ButtonState& s = Button(b);
        if (!s.down)
            return false;
        if (threshold < 0.0f)
            threshold = config.dragThreshold;
        return s.maxDistSq >= threshold * threshold;
    }

    bool InputState::KeyPressed(Key k, bool repeat) const
    {
        const KeyState& s = KeyAt(k);
        if (s.pressed)
            return true;
        if (!repeat || !s.down)
            return false;
        // repeats fire at every multiple of the rate after the delay that falls inside this frame
        const double t = time_ - s.downTime - config.keyRepeatDelay, t0 = t - deltaTime_;
        if (t < 0.0)
            return false;
        const double rate = config.keyRepeatRate;
        return std::floor(t / rate) != std::floor(t0 / rate) || t0 < 0.0;
    }
}
