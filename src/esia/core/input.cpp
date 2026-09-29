// Esia - input state (see esia/core/input.hpp)
#include "esia/core/input.hpp"
#include <cmath>

namespace esia
{
    void InputState::NewFrame(double time, std::vector<InputEvent>& events)
    {
        deltaTime_ = time_ > 0.0 ? (float)(time - time_) : 0.0f;
        time_ = time;
        for (ButtonState& b : mouse_)
            b.clicked = b.released = b.doubleClicked = b.changed = false;
        for (KeyState& k : keys_)
            k.pressed = k.released = k.changed = false;
        wheel_ = Vec2(0, 0);
        text_.clear();

        std::size_t used = 0;
        while (used < events.size() && Apply(events[used]))
            ++used;
        events.erase(events.begin(), events.begin() + (std::ptrdiff_t)used);

        mouseDelta_ = (MouseValid() && mousePrev_.x >= 0.0f && mousePrev_.y >= 0.0f) ? mousePos_ - mousePrev_ : Vec2(0, 0);
        mousePrev_ = mousePos_;
        for (ButtonState& b : mouse_)
            if (b.down && MouseValid())
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
            ButtonState& b = mouse_[Idx(e.button)];
            if (b.down == e.down)
                return true;
            if (b.changed)
                return false;
            b.changed = true;
            b.down = e.down;
            if (e.down)
            {
                b.clicked = true;
                b.downTime = time_;
                b.clickPos = mousePos_;
                b.maxDistSq = 0.0f;
                const float d = config.doubleClickDistance;
                if (time_ - b.lastClickTime <= config.doubleClickTime && LengthSq(mousePos_ - b.lastClickPos) <= d * d)
                {
                    b.doubleClicked = true;
                    b.lastClickTime = -1e9;   // a third click starts a new pair
                }
                else
                {
                    b.lastClickTime = time_;
                    b.lastClickPos = mousePos_;
                }
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
            KeyState& k = keys_[Idx(e.key)];
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
            }
            else
                k.released = true;
            mods_ = e.mods;
            return true;
        }
        case InputEvent::Type::Text:
            AppendUtf8(text_, e.text);
            return true;
        case InputEvent::Type::ImeComposition:
            composition_ = e.text;
            compositionCursor_ = e.imeCursor;
            return true;
        case InputEvent::Type::Focus:
            focused_ = e.down;
            if (!focused_)
            {
                // nothing stays stuck down when the window loses focus mid-press
                for (ButtonState& b : mouse_)
                    if (b.down)
                    {
                        b.down = false;
                        b.released = true;
                    }
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
        return mousePos_ - mouse_[Idx(b)].clickPos;
    }

    bool InputState::MouseDragging(MouseButton b, float threshold) const
    {
        const ButtonState& s = mouse_[Idx(b)];
        if (!s.down)
            return false;
        if (threshold < 0.0f)
            threshold = config.dragThreshold;
        return s.maxDistSq >= threshold * threshold;
    }

    bool InputState::KeyPressed(Key k, bool repeat) const
    {
        const KeyState& s = keys_[Idx(k)];
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

    void AppendUtf8(std::u32string& out, const std::string& s)
    {
        std::size_t i = 0;
        while (i < s.size())
        {
            const unsigned char c = (unsigned char)s[i];
            char32_t cp = 0xFFFD;
            int n = 1;
            if (c < 0x80)
                cp = c;
            else if ((c >> 5) == 0x6 && i + 1 < s.size())
            {
                cp = ((c & 0x1Fu) << 6) | ((unsigned char)s[i + 1] & 0x3Fu);
                n = 2;
            }
            else if ((c >> 4) == 0xE && i + 2 < s.size())
            {
                cp = ((c & 0x0Fu) << 12) | (((unsigned char)s[i + 1] & 0x3Fu) << 6) | ((unsigned char)s[i + 2] & 0x3Fu);
                n = 3;
            }
            else if ((c >> 3) == 0x1E && i + 3 < s.size())
            {
                cp = ((c & 0x07u) << 18) | (((unsigned char)s[i + 1] & 0x3Fu) << 12) | (((unsigned char)s[i + 2] & 0x3Fu) << 6) |
                     ((unsigned char)s[i + 3] & 0x3Fu);
                n = 4;
            }
            out.push_back(cp);
            i += (std::size_t)n;
        }
    }

    void AppendUtf32(std::string& out, char32_t c)
    {
        if (c < 0x80)
            out.push_back((char)c);
        else if (c < 0x800)
        {
            out.push_back((char)(0xC0 | (c >> 6)));
            out.push_back((char)(0x80 | (c & 0x3F)));
        }
        else if (c < 0x10000)
        {
            out.push_back((char)(0xE0 | (c >> 12)));
            out.push_back((char)(0x80 | ((c >> 6) & 0x3F)));
            out.push_back((char)(0x80 | (c & 0x3F)));
        }
        else
        {
            out.push_back((char)(0xF0 | (c >> 18)));
            out.push_back((char)(0x80 | ((c >> 12) & 0x3F)));
            out.push_back((char)(0x80 | ((c >> 6) & 0x3F)));
            out.push_back((char)(0x80 | (c & 0x3F)));
        }
    }
}
