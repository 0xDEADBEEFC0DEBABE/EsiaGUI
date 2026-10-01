#include "esia/core/context.hpp"
#include "esia_test.hpp"

using namespace esia;

namespace
{
    // Drives a context one frame at a time, like a platform layer would.
    struct Harness
    {
        Context ctx;
        double t = 1.0;
        void Frame() { t += 1.0 / 60.0; ctx.NewFrame({Vec2(800, 600), Vec2(1, 1), t}); }
        void Move(Vec2 p) { ctx.QueueInput(InputEvent::MouseMove(p)); }
        void Down() { ctx.QueueInput(InputEvent::Button(MouseButton::Left, true)); }
        void Up() { ctx.QueueInput(InputEvent::Button(MouseButton::Left, false)); }
        // one window with one button at its layout cursor
        ButtonResult ButtonFrame(const char* win, Vec2 pos, Vec2 size = {100, 100})
        {
            Frame();
            ctx.SetNextWindowPos(pos, Cond::FirstUse);
            ctx.SetNextWindowSize(size, Cond::FirstUse);
            ctx.Begin(win);
            const Id id = ctx.GetId("btn");
            const Rect bb = Rect::FromSize(ctx.CursorPos(), Vec2(40, 20));
            ctx.ItemSize(bb.Size());
            ctx.ItemAdd(id, bb);
            const ButtonResult r = ctx.ButtonBehavior(id, bb);
            ctx.End();
            ctx.EndFrame();
            return r;
        }
    };
}

ESIA_TEST(Context, IdsDependOnWindowAndStack)
{
    Context ctx;
    ctx.NewFrame({});
    ctx.Begin("A");
    const Id a = ctx.GetId("x");
    ctx.PushId("row");
    const Id ar = ctx.GetId("x");
    ctx.PushId(3);
    const Id ar3 = ctx.GetId("x");
    ctx.PopId();
    ctx.PopId();
    ESIA_CHECK(ctx.GetId("x") == a);
    ctx.End();
    ctx.Begin("B");
    const Id b = ctx.GetId("x");
    ctx.End();
    ctx.EndFrame();
    ESIA_CHECK(a != ar && ar != ar3 && a != b);
}

ESIA_TEST(Context, ButtonClickOnRelease)
{
    Harness h;
    h.Move({30, 30});
    ButtonResult r = h.ButtonFrame("W", {10, 10});   // window cursor at 22,22 (padding 12): button 22..62 x 22..42
    ESIA_CHECK(!r.hovered);                          // no window hovered on the first frame (none existed)
    r = h.ButtonFrame("W", {10, 10});
    ESIA_CHECK(r.hovered && !r.held && !r.pressed);
    h.Down();
    r = h.ButtonFrame("W", {10, 10});
    ESIA_CHECK(r.held && !r.pressed);
    ESIA_CHECK(h.ctx.ActiveId() != 0);
    h.Up();
    r = h.ButtonFrame("W", {10, 10});
    ESIA_CHECK(r.pressed && !r.held);
    ESIA_CHECK(h.ctx.ActiveId() == 0);
}

ESIA_TEST(Context, ReleaseOutsideDoesNotPress)
{
    Harness h;
    h.Move({30, 30});
    h.ButtonFrame("W", {10, 10});
    h.ButtonFrame("W", {10, 10});
    h.Down();
    h.ButtonFrame("W", {10, 10});
    h.Move({90, 90});
    ButtonResult r = h.ButtonFrame("W", {10, 10});
    ESIA_CHECK(r.held && !r.hovered);   // still held while dragged away
    h.Up();
    r = h.ButtonFrame("W", {10, 10});
    ESIA_CHECK(!r.pressed);
}

ESIA_TEST(Context, TopWindowTakesHoverAndClickFocuses)
{
    Harness h;
    auto frame = [&](bool* hitA, bool* hitB) {
        h.Frame();
        h.ctx.SetNextWindowPos({0, 0}, Cond::FirstUse);
        h.ctx.SetNextWindowSize({200, 200}, Cond::FirstUse);
        h.ctx.Begin("A");
        *hitA = h.ctx.ItemHoverable(h.ctx.GetId("a"), h.ctx.CurrentWindow()->GetRect());
        h.ctx.End();
        h.ctx.SetNextWindowPos({100, 100}, Cond::FirstUse);
        h.ctx.SetNextWindowSize({200, 200}, Cond::FirstUse);
        h.ctx.Begin("B");   // created later: on top
        *hitB = h.ctx.ItemHoverable(h.ctx.GetId("b"), h.ctx.CurrentWindow()->GetRect());
        h.ctx.End();
        h.ctx.EndFrame();
    };
    bool a = false, b = false;
    h.Move({150, 150});
    frame(&a, &b);
    frame(&a, &b);
    ESIA_CHECK(!a && b);
    auto order = h.ctx.WindowsInDrawOrder();
    ESIA_CHECK(order.size() == 3 && order[0]->Name() == "##root" && order[2]->Name() == "B");
    // click into A's visible part: A comes to the front
    h.Move({50, 50});
    h.Down();
    frame(&a, &b);
    h.Up();
    frame(&a, &b);
    order = h.ctx.WindowsInDrawOrder();
    ESIA_CHECK(order[2]->Name() == "A");
    ESIA_CHECK(h.ctx.FocusedWindow() && h.ctx.FocusedWindow()->Name() == "A");
    h.Move({150, 150});
    frame(&a, &b);
    frame(&a, &b);
    ESIA_CHECK(a && !b);
    // draw data follows the z-order
    const DrawData& dd = h.ctx.GetDrawData();
    ESIA_CHECK(dd.displaySize == Vec2(800, 600));
}

ESIA_TEST(Context, DragEmptyAreaMovesWindow)
{
    Harness h;
    auto frame = [&] {
        h.Frame();
        h.ctx.SetNextWindowPos({100, 100}, Cond::FirstUse);
        h.ctx.SetNextWindowSize({200, 150}, Cond::FirstUse);
        h.ctx.Begin("Mover");
        h.ctx.End();
        h.ctx.EndFrame();
    };
    h.Move({150, 150});
    frame();
    frame();
    h.Down();
    frame();
    ESIA_CHECK(h.ctx.ActiveId() != 0);
    h.Move({180, 170});
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("Mover")->GetRect().min == Vec2(130, 120));
    ESIA_CHECK(h.ctx.Requests().wantCaptureMouse);
    h.Up();
    frame();
    h.Move({300, 300});
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("Mover")->GetRect().min == Vec2(130, 120));
    ESIA_CHECK(h.ctx.ActiveId() == 0);
}

ESIA_TEST(Context, DragReleasedInTheFrameOfItsMovesStillMoves)
{
    // a slow frame (pipelines compiling, a hitch): the drag's moves and its release arrive together
    Harness h;
    auto frame = [&] {
        h.Frame();
        h.ctx.SetNextWindowPos({100, 100}, Cond::FirstUse);
        h.ctx.SetNextWindowSize({200, 150}, Cond::FirstUse);
        h.ctx.Begin("Quick");
        h.ctx.End();
        h.ctx.EndFrame();
    };
    h.Move({150, 150});
    frame();
    frame();
    h.Down();
    frame();
    h.Move({170, 160});
    h.Move({200, 190});
    h.Up();
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("Quick")->GetRect().min == Vec2(150, 140));
    ESIA_CHECK(h.ctx.ActiveId() == 0);
    h.Move({400, 400});
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("Quick")->GetRect().min == Vec2(150, 140));
}

ESIA_TEST(Context, ResizeFromCornerWithMinSize)
{
    Harness h;
    auto frame = [&] {
        h.Frame();
        h.ctx.SetNextWindowPos({100, 100}, Cond::FirstUse);
        h.ctx.SetNextWindowSize({200, 150}, Cond::FirstUse);
        WindowOptions o;
        o.minSize = {80, 60};
        h.ctx.Begin("Sizer", o);
        h.ctx.End();
        h.ctx.EndFrame();
    };
    h.Move({299, 249});   // bottom-right corner band
    frame();
    frame();
    ESIA_CHECK(h.ctx.Requests().cursor == MouseCursor::ResizeNWSE);
    h.Down();
    frame();
    h.Move({350, 300});
    frame();
    // grabbed 1 unit inside the corner: the corner keeps that distance to the pointer
    ESIA_CHECK(h.ctx.FindWindowByName("Sizer")->GetRect() == Rect(100, 100, 351, 301));
    h.Move({0, 0});
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("Sizer")->GetRect() == Rect(100, 100, 180, 160));   // clamped to the min size
    h.Up();
    frame();
}

ESIA_TEST(Context, LayoutCursorSameLineGroups)
{
    Context ctx;
    ctx.NewFrame({});
    ctx.SetNextWindowPos({0, 0});
    ctx.SetNextWindowSize({400, 400});
    WindowOptions o;
    o.padding = {10, 10};
    ctx.Begin("L", o);
    ESIA_CHECK(ctx.CursorPos() == Vec2(10, 10));
    ctx.ItemSize({50, 20});
    ESIA_CHECK(ctx.CursorPos() == Vec2(10, 38));   // 20 + spacing 8
    ctx.ItemSize({30, 10});
    ctx.SameLine();
    ESIA_CHECK(ctx.CursorPos() == Vec2(48, 38));   // after the 30 wide item + 8
    ctx.ItemSize({20, 30});                        // taller: line height becomes 30
    ESIA_CHECK(ctx.CursorPos() == Vec2(10, 76));
    ctx.BeginGroup();
    ctx.ItemSize({100, 10});
    ctx.ItemSize({60, 10});
    ctx.EndGroup();
    ESIA_CHECK(ctx.LastItemRect() == Rect(10, 76, 110, 104));
    ctx.SameLine();
    ESIA_CHECK(ctx.CursorPos() == Vec2(118, 76));
    ctx.ItemSize({10, 10});
    ctx.Indent();
    ESIA_CHECK(ctx.CursorPos().x == 26.0f);
    ctx.Unindent();
    ESIA_CHECK(ctx.ContentRegionAvail().x == 380.0f);
    ctx.End();
    ctx.EndFrame();
}

ESIA_TEST(Context, ScrollAndClipVisibility)
{
    Harness h;
    auto frame = [&](bool* lastVisible) {
        h.Frame();
        h.ctx.SetNextWindowPos({0, 0}, Cond::FirstUse);
        h.ctx.SetNextWindowSize({200, 100}, Cond::FirstUse);
        h.ctx.Begin("S");
        for (int i = 0; i < 10; ++i)
        {
            const Rect bb = Rect::FromSize(h.ctx.CursorPos(), Vec2(100, 32));
            h.ctx.ItemSize(bb.Size());
            *lastVisible = h.ctx.ItemAdd(h.ctx.GetId((std::int64_t)i), bb);
        }
        h.ctx.End();
        h.ctx.EndFrame();
    };
    bool visible = true;
    h.Move({50, 50});
    frame(&visible);
    ESIA_CHECK(!visible);
    const Window* w = h.ctx.FindWindowByName("S");
    ESIA_CHECK(w->ContentSize().y == 392.0f);             // 10 * 32 + 9 * 8 (last spacing not counted)
    ESIA_CHECK(w->ScrollMax().y == 392.0f - 76.0f);
    for (int i = 0; i < 20; ++i)
    {
        h.ctx.QueueInput(InputEvent::Wheel(0, -1));
        frame(&visible);
    }
    ESIA_CHECK(w->Scroll().y == w->ScrollMax().y);
    frame(&visible);
    ESIA_CHECK(visible);
}

ESIA_TEST(Context, ActiveItemNotSubmittedIsCleared)
{
    Harness h;
    h.Move({30, 30});
    h.ButtonFrame("W", {10, 10});
    h.ButtonFrame("W", {10, 10});
    h.Down();
    h.ButtonFrame("W", {10, 10});
    ESIA_CHECK(h.ctx.ActiveId() != 0);
    h.Frame();   // the button disappears
    h.ctx.EndFrame();
    h.Frame();
    h.ctx.EndFrame();
    ESIA_CHECK(h.ctx.ActiveId() == 0);
}

ESIA_TEST(Context, KeyboardFocusTabAndClickAway)
{
    Harness h;
    Id ids[3] = {};
    auto frame = [&] {
        h.Frame();
        h.ctx.SetNextWindowPos({0, 0}, Cond::FirstUse);
        h.ctx.SetNextWindowSize({300, 300}, Cond::FirstUse);
        h.ctx.Begin("F");
        for (int i = 0; i < 3; ++i)
        {
            ids[i] = h.ctx.GetId((std::int64_t)i);
            const Rect bb = Rect::FromSize(h.ctx.CursorPos(), Vec2(100, 20));
            h.ctx.ItemSize(bb.Size());
            h.ctx.ItemAdd(ids[i], bb, ItemFlags_Focusable);
            h.ctx.ButtonBehavior(ids[i], bb, ButtonFlags_FocusOnClick);
            if (h.ctx.KeyboardFocusId() == ids[i])
                h.ctx.RequestTextInput(Rect::FromSize(bb.min, Vec2(1, 20)));
        }
        h.ctx.End();
        h.ctx.EndFrame();
    };
    h.Move({20, 20});
    frame();
    frame();
    h.Down();
    frame();
    h.Up();
    frame();
    ESIA_CHECK(h.ctx.KeyboardFocusId() == ids[0]);
    ESIA_CHECK(h.ctx.Requests().wantTextInput && h.ctx.Requests().wantCaptureKeyboard);
    h.ctx.QueueInput(InputEvent::KeyEvent(Key::Tab, true));
    frame();
    ESIA_CHECK(h.ctx.KeyboardFocusId() == ids[1]);
    h.ctx.QueueInput(InputEvent::KeyEvent(Key::Tab, false));
    h.ctx.QueueInput(InputEvent::KeyEvent(Key::LeftShift, true, Mod_Shift));
    frame();
    h.ctx.QueueInput(InputEvent::KeyEvent(Key::Tab, true, Mod_Shift));
    frame();
    ESIA_CHECK(h.ctx.KeyboardFocusId() == ids[0]);
    // click on empty space: focus goes away
    h.Move({250, 250});
    frame();
    h.Down();
    frame();
    ESIA_CHECK(h.ctx.KeyboardFocusId() == 0);
    h.Up();
    frame();
}

ESIA_TEST(Context, LaterItemIsHitOnTop)
{
    Harness h;
    bool first = false, second = false;
    auto frame = [&] {
        h.Frame();
        h.ctx.SetNextWindowPos({0, 0}, Cond::FirstUse);
        h.ctx.SetNextWindowSize({300, 300}, Cond::FirstUse);
        h.ctx.Begin("O");
        const Rect big(20, 20, 200, 200), small(50, 50, 80, 80);
        first = h.ctx.ItemHoverable(h.ctx.GetId("big"), big);
        second = h.ctx.ItemHoverable(h.ctx.GetId("small"), small);
        h.ctx.End();
        h.ctx.EndFrame();
    };
    h.Move({60, 60});
    frame();
    frame();
    frame();
    ESIA_CHECK(!first && second);
    h.Move({150, 150});
    frame();
    frame();
    ESIA_CHECK(first && !second);
}

ESIA_TEST(Context, ClipboardCallbacks)
{
    std::string board;
    ContextDesc d;
    d.setClipboard = [&](const std::string& s) { board = s; };
    d.getClipboard = [&] { return board; };
    Context ctx(d);
    ctx.SetClipboardText("hello");
    ESIA_CHECK(ctx.GetClipboardText() == "hello");
}

ESIA_TEST(Context, SafeAreaIsTheDisplayUnlessGiven)
{
    Context ctx;
    ctx.NewFrame({Vec2(440, 956), Vec2(3, 3), 1.0});
    ESIA_CHECK(ctx.SafeArea().min == Vec2(0, 0) && ctx.SafeArea().max == Vec2(440, 956));   // a desktop's: all of it
    ctx.EndFrame();
    // a phone held upright: under the camera housing, above the home indicator
    ctx.NewFrame({Vec2(440, 956), Vec2(3, 3), 2.0, Rect(0, 62, 440, 922)});
    ESIA_CHECK(ctx.SafeArea().min == Vec2(0, 62) && ctx.SafeArea().max == Vec2(440, 922));
    ctx.EndFrame();
    // never past the display; a degenerate one is the whole display
    ctx.NewFrame({Vec2(440, 956), Vec2(3, 3), 3.0, Rect(-10, 62, 500, 990)});
    ESIA_CHECK(ctx.SafeArea().min == Vec2(0, 62) && ctx.SafeArea().max == Vec2(440, 956));
    ctx.EndFrame();
    ctx.NewFrame({Vec2(440, 956), Vec2(3, 3), 4.0, Rect(0, 900, 440, 100)});
    ESIA_CHECK(ctx.SafeArea().min == Vec2(0, 0) && ctx.SafeArea().max == Vec2(440, 956));
    ctx.EndFrame();
}
