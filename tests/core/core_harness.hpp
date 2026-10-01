// Esia - a context driven one frame at a time, like a platform layer would (the UI core tests).
#pragma once
#include "esia/core/context.hpp"

namespace esia::test
{
    struct Harness
    {
        Context ctx;
        double t = 1.0;
        Vec2 display{800, 600};
        Vec2 fbScale{1, 1};

        explicit Harness(const ContextDesc& desc = {}) : ctx(desc) {}

        void Frame(double dt = 1.0 / 60.0)
        {
            t += dt;
            ctx.NewFrame({display, fbScale, t});
        }
        void End() { ctx.EndFrame(); }
        void Move(Vec2 p) { ctx.QueueInput(InputEvent::MouseMove(p)); }
        void Down(MouseButton b = MouseButton::Left) { ctx.QueueInput(InputEvent::Button(b, true)); }
        void Up(MouseButton b = MouseButton::Left) { ctx.QueueInput(InputEvent::Button(b, false)); }
        // a finger: the left button, marked as a touch
        void TouchDown() { ctx.QueueInput(InputEvent::Button(MouseButton::Left, true, true)); }
        void TouchUp() { ctx.QueueInput(InputEvent::Button(MouseButton::Left, false, true)); }
        void KeyDown(Key k, std::uint32_t mods = 0) { ctx.QueueInput(InputEvent::KeyEvent(k, true, mods)); }
        void KeyUp(Key k, std::uint32_t mods = 0) { ctx.QueueInput(InputEvent::KeyEvent(k, false, mods)); }
        void Wheel(float x, float y) { ctx.QueueInput(InputEvent::Wheel(x, y)); }

        // a window placed on first use
        bool Win(const char* name, Vec2 pos, Vec2 size, const WindowOptions& o = {})
        {
            ctx.SetNextWindowPos(pos, Cond::FirstUse);
            ctx.SetNextWindowSize(size, Cond::FirstUse);
            return ctx.Begin(name, o);
        }
        // a button at an explicit rect
        ButtonResult Button(const char* label, const Rect& bb, std::uint32_t buttonFlags = 0, std::uint32_t itemFlags = 0)
        {
            const Id id = ctx.GetId(label);
            ctx.ItemAdd(id, bb, itemFlags);
            return ctx.ButtonBehavior(id, bb, buttonFlags);
        }
        // a button of `size` at the layout cursor
        ButtonResult Button(const char* label, Vec2 size, std::uint32_t buttonFlags = 0, std::uint32_t itemFlags = 0)
        {
            const Rect bb = Rect::FromSize(ctx.CursorPos(), size);
            ctx.ItemSize(size);
            return Button(label, bb, buttonFlags, itemFlags);
        }
    };
}
