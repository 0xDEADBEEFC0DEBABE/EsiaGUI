// The review's interaction bugs (docs/UI_CORE.md, section 14): each probe scenario as a regression test, plus the
// hit testing, press ownership and key routing that replaced the old model.
#include "core_harness.hpp"
#include "esia_test.hpp"

using namespace esia;
using esia::test::Harness;

namespace
{
    // one window at (10, 10), 200 x 200, padding 12: a 40 x 20 button at its cursor (22, 22)
    ButtonResult OneButton(Harness& h, std::uint32_t buttonFlags = 0, std::uint32_t itemFlags = 0)
    {
        h.Frame();
        h.Win("W", {10, 10}, {200, 200});
        const ButtonResult r = h.Button("btn", Vec2(40, 20), buttonFlags, itemFlags);
        h.ctx.End();
        h.End();
        return r;
    }
}

// bug 1: losing window focus while holding a button must not click it
ESIA_TEST(Context, FocusLossDoesNotClick)
{
    Harness h;
    h.Move({30, 30});
    OneButton(h);
    OneButton(h);
    h.Down();
    ESIA_CHECK(OneButton(h).held);
    h.ctx.QueueInput(InputEvent::FocusEvent(false));
    const ButtonResult r = OneButton(h);
    ESIA_CHECK(!r.pressed && !r.held && !r.hovered);
    ESIA_CHECK(h.ctx.ActiveId() == 0);
    // even when the mouse comes back over the button in the same frame as the canceled release
    h.Move({30, 30});
    h.ctx.QueueInput(InputEvent::FocusEvent(true));
    ESIA_CHECK(!OneButton(h).pressed);
}

// bug 2: a drag that starts over the game stays the game's, even across a UI window
ESIA_TEST(Context, PressOwnershipGameDrag)
{
    Harness h;
    h.Move({500, 500});   // the game
    OneButton(h);
    OneButton(h);
    ESIA_CHECK(!h.ctx.Requests().wantCaptureMouse);
    h.Down();
    OneButton(h);
    ESIA_CHECK(!h.ctx.Requests().wantCaptureMouse);
    h.Move({30, 30});     // over the button of a UI window, button still held
    ButtonResult r = OneButton(h);
    ESIA_CHECK(!r.hovered && !r.held && h.ctx.ActiveId() == 0);
    ESIA_CHECK(!h.ctx.Requests().wantCaptureMouse);
    h.Up();
    r = OneButton(h);
    ESIA_CHECK(!r.pressed);
    ESIA_CHECK(!h.ctx.Requests().wantCaptureMouse);   // the release is the game's too
    r = OneButton(h);
    ESIA_CHECK(r.hovered && h.ctx.Requests().wantCaptureMouse);   // no press held: over the UI is the UI's
    ESIA_CHECK(h.ctx.FindWindowByName("W")->GetRect().min == Vec2(10, 10));
}

// bug 2: a press on the UI dragged out stays the UI's until it is released
ESIA_TEST(Context, PressOwnershipUiDragOut)
{
    Harness h;
    h.Move({30, 30});
    OneButton(h);
    OneButton(h);
    h.Down();
    OneButton(h);
    ESIA_CHECK(h.ctx.Requests().wantCaptureMouse);
    h.Move({600, 500});
    ButtonResult r = OneButton(h);
    ESIA_CHECK(r.held && !r.hovered && h.ctx.Requests().wantCaptureMouse);
    h.Up();
    r = OneButton(h);
    ESIA_CHECK(!r.pressed && h.ctx.Requests().wantCaptureMouse);   // the release frame is still the UI's
    OneButton(h);
    ESIA_CHECK(!h.ctx.Requests().wantCaptureMouse);
}

// bug 3: ContentRegionAvail().y is the visible content height at the top, whatever the scroll
ESIA_TEST(Context, ContentRegionAvailIgnoresScroll)
{
    Harness h;
    Vec2 atTop;
    auto frame = [&] {
        h.Frame();
        WindowOptions o;
        o.padding = {10, 10};
        h.Win("S", {0, 0}, {200, 100}, o);
        atTop = h.ctx.ContentRegionAvail();
        for (int i = 0; i < 20; ++i)
            h.ctx.ItemSize({50, 30});
        h.ctx.End();
        h.End();
    };
    h.Move({50, 50});
    frame();
    ESIA_CHECK(atTop == Vec2(180, 80));
    h.Wheel(0, -2);
    frame();
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("S")->Scroll().y == 96.0f);
    ESIA_CHECK(atTop == Vec2(180, 80));
}

// bug 4: a repeat button presses on the click, repeats after the delay, and not on the release
ESIA_TEST(Context, RepeatButton)
{
    Harness h;
    h.Move({30, 30});
    OneButton(h, ButtonFlags_Repeat);
    OneButton(h, ButtonFlags_Repeat);
    h.Down();
    ESIA_CHECK(OneButton(h, ButtonFlags_Repeat).pressed);   // the click
    int early = 0, repeats = 0;
    for (int i = 0; i < 12; ++i)   // 0.2 s: before the 0.275 s delay
        early += OneButton(h, ButtonFlags_Repeat).pressed ? 1 : 0;
    for (int i = 0; i < 48; ++i)   // 0.8 s more
        repeats += OneButton(h, ButtonFlags_Repeat).pressed ? 1 : 0;
    ESIA_CHECK(early == 0);
    ESIA_CHECK(repeats >= 14 && repeats <= 16);   // at 0.275 s, then every 0.05 s up to 1.0 s: 15
    h.Up();
    const ButtonResult r = OneButton(h, ButtonFlags_Repeat);
    ESIA_CHECK(!r.pressed && !r.held);
    // a quick click presses exactly once
    h.Down();
    int presses = OneButton(h, ButtonFlags_Repeat).pressed ? 1 : 0;
    h.Up();
    presses += OneButton(h, ButtonFlags_Repeat).pressed ? 1 : 0;
    ESIA_CHECK(presses == 1);
}

// bug 5: dragging a disabled item does not move its window, and it keeps the hover from what is below
ESIA_TEST(Context, DisabledItemBlocksWindowDrag)
{
    Harness h;
    h.Move({30, 30});
    OneButton(h, 0, ItemFlags_Disabled);
    OneButton(h, 0, ItemFlags_Disabled);
    h.Down();
    ButtonResult r = OneButton(h, 0, ItemFlags_Disabled);
    ESIA_CHECK(!r.hovered && !r.held && h.ctx.ActiveId() == 0);
    h.Move({80, 80});
    OneButton(h, 0, ItemFlags_Disabled);
    ESIA_CHECK(h.ctx.FindWindowByName("W")->GetRect().min == Vec2(10, 10));
    h.Up();
    ESIA_CHECK(!OneButton(h, 0, ItemFlags_Disabled).pressed);
}

ESIA_TEST(Context, DisabledItemClaimsHover)
{
    Harness h;
    bool below = false;
    ButtonResult above;
    auto frame = [&] {
        h.Frame();
        h.Win("W", {0, 0}, {300, 300});
        below = h.ctx.ItemHoverable(h.ctx.GetId("below"), Rect(20, 20, 200, 200));
        // ButtonBehavior takes the item flags given to ItemAdd
        const Id id = h.ctx.GetId("disabled");
        h.ctx.ItemAdd(id, Rect(50, 50, 100, 100), ItemFlags_Disabled);
        above = h.ctx.ButtonBehavior(id, Rect(50, 50, 100, 100));
        h.ctx.End();
        h.End();
    };
    h.Move({60, 60});
    frame();
    frame();
    ESIA_CHECK(!below && !above.hovered);
    h.Move({150, 150});
    frame();
    ESIA_CHECK(below);
}

ESIA_TEST(Context, DisabledWhileActiveIsDeactivated)
{
    Harness h;
    h.Move({30, 30});
    OneButton(h);
    OneButton(h);
    h.Down();
    ESIA_CHECK(OneButton(h).held && h.ctx.ActiveId() != 0);
    ButtonResult r = OneButton(h, 0, ItemFlags_Disabled);
    ESIA_CHECK(!r.held && h.ctx.ActiveId() == 0);
    h.Up();
    r = OneButton(h);   // enabled again, released over it: not a press (it was deactivated)
    ESIA_CHECK(!r.pressed);
}

// bug 6: a touch moves and presses in one frame: the item under the finger is hit, not last frame's hover
ESIA_TEST(Context, TouchTapHitsItemUnderIt)
{
    Harness h;
    ButtonResult big, small, other;
    auto frame = [&] {
        h.Frame();
        h.Win("T", {0, 0}, {400, 300});
        big = h.Button("big", Rect(20, 20, 200, 200));
        small = h.Button("small", Rect(50, 50, 80, 80));   // later: on top of "big"
        other = h.Button("other", Rect(250, 20, 350, 60));
        h.ctx.End();
        h.End();
    };
    h.Move({150, 150});
    frame();
    frame();
    ESIA_CHECK(big.hovered && !small.hovered);
    // the finger lands on "small": move and press arrive together
    h.Move({60, 60});
    h.Down();
    frame();
    ESIA_CHECK(small.held && !big.held);
    h.Up();
    frame();
    ESIA_CHECK(small.pressed && !big.pressed);
    // and from one item straight onto another
    h.Move({300, 40});
    h.Down();
    frame();
    h.Up();
    frame();
    ESIA_CHECK(other.pressed && !small.pressed && !big.pressed);
}

// bug 7: input deferred to the next frame is visible to an event-driven host
ESIA_TEST(Context, InputPending)
{
    Harness h;
    h.Move({30, 30});
    OneButton(h);
    ESIA_CHECK(!h.ctx.InputPending() && !h.ctx.Requests().inputPending);
    h.Down();
    h.Up();   // a click and its release between two frames
    OneButton(h);
    ESIA_CHECK(h.ctx.InputPending() && h.ctx.Requests().inputPending);
    ESIA_CHECK(OneButton(h).pressed);   // the release, one frame later
    ESIA_CHECK(!h.ctx.Requests().inputPending);
}

// bug 10: the text cursor only over the field, even while it has keyboard focus
ESIA_TEST(Context, TextCursorOnlyOverField)
{
    Harness h;
    auto frame = [&] {
        h.Frame();
        h.Win("F", {0, 0}, {300, 200});
        const Id id = h.ctx.GetId("field");
        const Rect bb = Rect::FromSize(h.ctx.CursorPos(), Vec2(200, 30));
        h.ctx.ItemSize(bb.Size());
        h.ctx.ItemAdd(id, bb, ItemFlags_Focusable);
        const ButtonResult r = h.ctx.ButtonBehavior(id, bb, ButtonFlags_FocusOnClick | ButtonFlags_PressOnClick);
        if (r.hovered)
            h.ctx.SetMouseCursor(MouseCursor::TextInput);
        if (h.ctx.KeyboardFocusId() == id)
            h.ctx.RequestTextInput(Rect::FromSize(bb.min, Vec2(1, 30)));
        h.ctx.End();
        h.End();
    };
    h.Move({30, 20});
    frame();
    frame();
    h.Down();
    frame();
    h.Up();
    frame();
    ESIA_CHECK(h.ctx.Requests().wantTextInput && h.ctx.Requests().cursor == MouseCursor::TextInput);
    h.Move({250, 150});
    frame();
    ESIA_CHECK(h.ctx.Requests().wantTextInput && h.ctx.Requests().cursor == MouseCursor::Arrow);
}

// bug 11: a drag continues past the left edge of the display (negative coordinates are positions)
ESIA_TEST(Context, DragPastLeftEdge)
{
    Harness h;
    h.Move({30, 30});
    OneButton(h);
    OneButton(h);
    h.Down();
    OneButton(h);
    h.Move({-40, 30});
    const ButtonResult r = OneButton(h);
    ESIA_CHECK(r.held && h.ctx.ActiveId() != 0);
    ESIA_CHECK(h.ctx.Input().MouseValid() && h.ctx.Input().MouseDragDelta(MouseButton::Left) == Vec2(-70, 0));
    ESIA_CHECK(h.ctx.Requests().wantCaptureMouse);
    h.Up();
    OneButton(h);
}

// Front to back: floating children and popups above content, background items below, whatever the order
ESIA_TEST(Context, HitTestLayers)
{
    Harness h;
    ButtonResult content, bg, bar;
    auto frame = [&] {
        h.Frame();
        h.Win("L", {0, 0}, {400, 400});
        content = h.Button("content", Rect(20, 20, 300, 300));
        // a card's press registered at its end, under the content drawn before it
        const Id bgId = h.ctx.GetId("card");
        h.ctx.ItemAdd(bgId, Rect(10, 10, 380, 380), ItemFlags_Background);
        bg = h.ctx.ButtonBehavior(bgId, Rect(10, 10, 380, 380));
        // a floating bar over the content, submitted first
        ChildOptions co;
        co.flags = ChildFlags_Floating;
        co.rect = Rect(20, 250, 380, 300);
        h.ctx.BeginChild("bar", co);
        bar = h.Button("tab", Rect(30, 255, 90, 295));
        h.ctx.EndChild();
        h.ctx.End();
        h.End();
    };
    h.Move({100, 100});
    frame();
    frame();
    ESIA_CHECK(content.hovered && !bg.hovered && !bar.hovered);
    h.Move({350, 350});
    frame();
    ESIA_CHECK(!content.hovered && bg.hovered);
    h.Move({50, 270});   // the tab on the floating bar
    frame();
    ESIA_CHECK(bar.hovered && !content.hovered);
    h.Move({200, 270});  // the bar's empty glass: nothing below it hovers, and a drag does not move the window
    frame();
    ESIA_CHECK(!bar.hovered && !content.hovered && !bg.hovered);
    h.Down();
    frame();
    h.Move({220, 290});
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("L")->GetRect().min == Vec2(0, 0));
    h.Up();
    frame();
}

ESIA_TEST(Context, ButtonClickCounts)
{
    Harness h;
    ButtonResult r;
    int doubles = 0;
    auto frame = [&] {
        h.Frame(0.05);
        h.Win("C", {0, 0}, {200, 200});
        r = h.Button("b", Rect(20, 20, 100, 60), ButtonFlags_PressOnDoubleClick);
        doubles += r.pressed ? 1 : 0;
        h.ctx.End();
        h.End();
    };
    h.Move({30, 30});
    frame();
    frame();
    int maxClicks = 0;
    for (int i = 0; i < 3; ++i)
    {
        h.Down();
        frame();
        maxClicks = std::max(maxClicks, r.clicks);
        h.Up();
        frame();
    }
    ESIA_CHECK(maxClicks == 3 && doubles == 1);
}

// key ownership: a claim holds for everything asked after it this frame and all of the next
ESIA_TEST(Context, KeyOwnership)
{
    Harness h;
    const Id dialog = 11, button = 22;
    bool buttonSaw = false, dialogSaw = false;
    auto frame = [&](bool claim) {
        h.Frame();
        buttonSaw = h.ctx.KeyPressed(Key::Enter, button, false);   // asked before the dialog claims
        if (claim)
            h.ctx.ClaimKey(Key::Enter, dialog);
        dialogSaw = h.ctx.KeyPressed(Key::Enter, dialog, false);
        h.End();
    };
    frame(true);
    h.KeyDown(Key::Enter);
    frame(true);
    ESIA_CHECK(!buttonSaw && dialogSaw);   // last frame's claim already holds for the button
    ESIA_CHECK(h.ctx.KeyOwner(Key::Enter) == dialog && h.ctx.Requests().wantCaptureKeyboard);
    h.KeyUp(Key::Enter);
    frame(false);
    frame(false);   // the claim was not renewed: it lapses
    h.KeyDown(Key::Enter);
    frame(false);
    ESIA_CHECK(buttonSaw && dialogSaw && h.ctx.KeyOwner(Key::Enter) == 0);
    // ClaimKeyboard takes every key
    h.KeyUp(Key::Enter);
    h.Frame();
    h.ctx.ClaimKeyboard(dialog);
    ESIA_CHECK(h.ctx.KeyOwner(Key::A) == dialog && h.ctx.KeyOwner(Key::Tab) == dialog);
    h.End();
}

// Tab navigation runs after the widgets: an editor that claims Tab keeps it
ESIA_TEST(Context, TabNavigationAfterWidgets)
{
    Harness h;
    Id ids[3] = {};
    bool editorKeepsTab = false;
    auto frame = [&] {
        h.Frame();
        h.Win("N", {0, 0}, {300, 300});
        for (int i = 0; i < 3; ++i)
        {
            ids[i] = h.ctx.GetId((std::int64_t)i);
            const Rect bb = Rect::FromSize(h.ctx.CursorPos(), Vec2(100, 20));
            h.ctx.ItemSize(bb.Size());
            h.ctx.ItemAdd(ids[i], bb, ItemFlags_Focusable);
            if (editorKeepsTab && h.ctx.KeyboardFocusId() == ids[i])
                h.ctx.ClaimKey(Key::Tab, ids[i]);
        }
        h.ctx.End();
        h.End();
    };
    frame();
    h.KeyDown(Key::Tab);
    frame();
    ESIA_CHECK(h.ctx.KeyboardFocusId() == ids[0]);
    h.KeyUp(Key::Tab);
    frame();
    h.KeyDown(Key::Tab);
    frame();
    ESIA_CHECK(h.ctx.KeyboardFocusId() == ids[1]);
    h.KeyUp(Key::Tab);
    editorKeepsTab = true;
    frame();
    h.KeyDown(Key::Tab);
    frame();
    ESIA_CHECK(h.ctx.KeyboardFocusId() == ids[1]);   // the focused editor kept its Tab
    h.KeyUp(Key::Tab);
    frame();
}

ESIA_TEST(Context, ItemStatus)
{
    Harness h;
    ItemStatus s;
    Id id = 0;
    auto frame = [&](std::uint32_t itemFlags) {
        h.Frame();
        h.Win("S", {0, 0}, {300, 300});
        id = h.ctx.GetId("b");
        h.ctx.ItemAdd(id, Rect(20, 20, 100, 60), itemFlags | ItemFlags_Focusable);
        h.ctx.ButtonBehavior(id, Rect(20, 20, 100, 60), ButtonFlags_FocusOnClick);
        s = h.ctx.LastItemStatus();
        h.ctx.ItemAdd(0, Rect(0, 0, 1, 1));
        const ItemStatus later = h.ctx.ItemStatusOf(id);   // looked up by id after other items
        ESIA_CHECK(later.hovered == s.hovered && later.active == s.active && later.rect == Rect(20, 20, 100, 60));
        h.ctx.End();
        h.End();
    };
    h.Move({30, 30});
    frame(0);
    frame(0);
    ESIA_CHECK(s.hovered && s.visible && !s.active && !s.disabled);
    h.Down();
    frame(0);
    ESIA_CHECK(s.active && s.focused);
    h.Up();
    frame(0);
    ESIA_CHECK(s.pressed);
    frame(ItemFlags_Disabled);
    ESIA_CHECK(s.disabled && !s.hovered);
}

// Two claimants of one key never both see it, whatever order they are submitted in; the keyboard's owner (an editor)
// outranks a per-key claim (the dialog around it)
ESIA_TEST(Context, KeyOwnershipConflicts)
{
    Harness h;
    const Id a = 1, b = 2, editor = 3, dialog = 4;
    int seenA = 0, seenB = 0;
    bool swap = false;
    auto frame = [&] {
        h.Frame();
        for (int i = 0; i < 2; ++i)
        {
            const Id who = (i == 0) != swap ? a : b;
            h.ctx.ClaimKey(Key::Enter, who);
            if (h.ctx.KeyPressed(Key::Enter, who, false))
                ++(who == a ? seenA : seenB);
        }
        h.End();
    };
    h.KeyDown(Key::Enter);
    frame();   // no owner before: the first claim of the frame wins
    ESIA_CHECK(seenA == 1 && seenB == 0);
    h.KeyUp(Key::Enter);
    frame();
    swap = true;   // b is submitted first now: a keeps the key it owned
    h.KeyDown(Key::Enter);
    frame();
    ESIA_CHECK(seenA == 2 && seenB == 0);
    h.KeyUp(Key::Enter);
    frame();

    bool editorSaw = false, dialogSaw = false;
    auto nested = [&](bool editorFirst) {
        h.Frame();
        for (int i = 0; i < 2; ++i)
        {
            if ((i == 0) == editorFirst)
            {
                h.ctx.ClaimKeyboard(editor);
                editorSaw = h.ctx.KeyPressed(Key::Enter, editor, false);
            }
            else
            {
                h.ctx.ClaimKey(Key::Enter, dialog);
                dialogSaw = h.ctx.KeyPressed(Key::Enter, dialog, false);
            }
        }
        h.End();
    };
    for (int order = 0; order < 2; ++order)
    {
        nested(order == 0);
        h.KeyDown(Key::Enter);
        nested(order == 0);
        ESIA_CHECK(editorSaw && !dialogSaw);
        h.KeyUp(Key::Enter);
        nested(order == 0);
    }
}

// Content that scrolls under a still mouse (the wheel, a glide) is hit where it is now, in the same frame
ESIA_TEST(Context, HoverFollowsScrolledContent)
{
    Harness h;
    ButtonResult rows[20];
    Rect rects[20];
    bool smooth = false;
    auto frame = [&] {
        h.Frame();
        WindowOptions o;
        o.padding = {0, 0};
        h.Win("W", {0, 0}, {300, 300}, o);
        ChildOptions c;
        c.size = {200, 200};
        c.flags = ChildFlags_ScrollY | (smooth ? ChildFlags_SmoothScroll : 0u);
        h.ctx.BeginChild("list", c);
        for (int i = 0; i < 20; ++i)
        {
            h.ctx.PushId(i);
            rows[i] = h.Button("row", Vec2(180, 40));   // rows every 48 units
            rects[i] = h.ctx.LastItemRect();
            h.ctx.PopId();
        }
        h.ctx.EndChild();
        h.ctx.End();
        h.End();
    };
    h.Move({50, 60});   // row 1 (48..88)
    frame();
    frame();
    ESIA_CHECK(rows[1].hovered);
    h.Wheel(0, -1);     // 48 down: row 2 is under the mouse now
    h.Down();
    frame();
    ESIA_CHECK(rows[2].hovered && rows[2].held && !rows[1].hovered);
    h.Up();
    frame();
    ESIA_CHECK(rows[2].pressed);
    // while a smooth scroll glides, every frame hovers the row that is under the mouse at that moment
    smooth = true;
    frame();
    h.Wheel(0, -4);
    bool alwaysRight = true, moved = false;
    for (int f = 0; f < 30; ++f)
    {
        frame();
        int hovered = -1, under = -1;
        for (int i = 0; i < 20; ++i)
        {
            hovered = rows[i].hovered ? i : hovered;
            under = rects[i].Contains(Vec2(50, 60)) ? i : under;   // where this frame's layout put the rows
        }
        alwaysRight = alwaysRight && hovered == under;
        moved = moved || under > 3;
    }
    ESIA_CHECK(moved);
    ESIA_CHECK(alwaysRight);
}

// an item made active by a key (not by a mouse press) is not "released" by ButtonBehavior
ESIA_TEST(Context, KeyboardActivatedItemStaysActive)
{
    Harness h;
    Id id = 0;
    ButtonResult r;
    auto frame = [&](bool activate) {
        h.Frame();
        h.Win("K", {0, 0}, {300, 300});
        id = h.ctx.GetId("b");
        if (activate)
            h.ctx.SetActiveId(id);
        r = h.Button("b", Rect(20, 20, 100, 60));
        h.ctx.End();
        h.End();
    };
    h.Move({30, 30});
    frame(false);
    frame(true);
    frame(false);
    ESIA_CHECK(h.ctx.ActiveId() == id && !r.pressed && !r.held);
    h.ctx.ClearActiveId();
}
