// Windows (lifecycle, focus, garbage collection, on-screen rules, DPI), scrolling, popups and tooltips, state and
// scope data (docs/UI_CORE.md, sections 7 - 10 and 14).
#include "core_harness.hpp"
#include "esia_test.hpp"
#include <cmath>
#include <string>
#include <vector>

using namespace esia;
using esia::test::Harness;

// bug 8: an item in the padding (under a title bar, scrolled out of the content) is neither visible nor hovered
ESIA_TEST(Context, ContentClipVisibility)
{
    Harness h;
    bool inPadding = true, straddling = false, hovered = false;
    auto frame = [&] {
        h.Frame();
        h.Win("C", {0, 0}, {200, 100});   // padding 12: content 12..188 x 12..88
        inPadding = h.ctx.ItemAdd(h.ctx.GetId("a"), Rect(20, 90, 100, 98));
        straddling = h.ctx.ItemAdd(h.ctx.GetId("b"), Rect(20, 80, 100, 95));
        hovered = h.ctx.ItemHoverable(h.ctx.GetId("b"), Rect(20, 80, 100, 95));
        ESIA_CHECK(h.ctx.CurrentWindow()->GetDrawList().ClipRect() == Rect(12, 12, 188, 88));
        h.ctx.End();
        h.End();
    };
    h.Move({50, 92});   // over "b", but in the padding
    frame();
    frame();
    ESIA_CHECK(!inPadding && straddling && !hovered);
    h.Move({50, 85});
    frame();
    ESIA_CHECK(hovered);
}

// bug 8: SetScrollY clamps to the range measured this frame, not last frame's
ESIA_TEST(Context, SetScrollClampsToThisFrame)
{
    Harness h;
    int rows = 1;
    bool request = false;
    auto frame = [&] {
        h.Frame();
        h.Win("S", {0, 0}, {200, 100});
        if (request)
            h.ctx.SetScrollY(600.0f);   // before the content that makes it reachable
        for (int i = 0; i < rows; ++i)
            h.ctx.ItemSize({50, 40});
        h.ctx.End();
        h.End();
    };
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("S")->ScrollMax().y == 0.0f);
    rows = 30;
    request = true;
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("S")->Scroll().y == 600.0f);
    request = false;
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("S")->Scroll().y == 600.0f);
    rows = 1;   // the content shrinks: the offset follows the range
    frame();
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("S")->Scroll().y == 0.0f);
}

// bug 8: deferred scroll targets
ESIA_TEST(Context, ScrollTargets)
{
    Harness h;
    int mode = 0;
    auto frame = [&] {
        h.Frame();
        WindowOptions o;
        o.padding = {0, 0};
        if (mode == 4)
            h.ctx.SetNextScroll({-1, 96});
        h.Win("T", {0, 0}, {200, 100}, o);   // rows of 40 + 8 spacing: row i at y = 48 i
        for (int i = 0; i < 20; ++i)
        {
            if (mode == 5 && i == 0)
                h.ctx.ScrollToRect(Rect(0, 500 - 96, 10, 540 - 96), Vec2(-1, 1.0f));
            if (mode == 1 && i == 10)
                h.ctx.SetScrollHereY(0.0f);   // row 10 at the top
            if (mode == 2 && i == 10)
                h.ctx.SetScrollHereY(0.5f);   // row 10's top at the center
            const Rect bb = Rect::FromSize(h.ctx.CursorPos(), Vec2(50, 40));
            h.ctx.ItemSize(bb.Size());
            h.ctx.ItemAdd(h.ctx.GetId((std::int64_t)i), bb);
            if (mode == 3 && i == 5)
                h.ctx.ScrollToItem();          // the smallest scroll that shows row 5
        }
        mode = 0;
        h.ctx.End();
        h.End();
    };
    const Window* w = nullptr;
    frame();
    w = h.ctx.FindWindowByName("T");
    mode = 1;
    frame();
    ESIA_CHECK(w->Scroll().y == 480.0f);
    mode = 2;
    frame();
    ESIA_CHECK(w->Scroll().y == 430.0f);
    // from 430, row 5 (240..280) is above the view: it comes to the top
    mode = 3;
    frame();
    ESIA_CHECK(w->Scroll().y == 240.0f);
    mode = 4;
    frame();
    ESIA_CHECK(w->Scroll().y == 96.0f);
    // ScrollToRect with an alignment: the rect's bottom at the view's bottom (absolute: it is scrolled by 96)
    mode = 5;
    frame();
    ESIA_CHECK(w->Scroll().y == 440.0f);
}

// bug 9: a window that is submitted again comes to the front
ESIA_TEST(Context, ReopenedWindowComesToFront)
{
    Harness h;
    bool showA = true;
    auto frame = [&] {
        h.Frame();
        if (showA)
        {
            h.Win("A", {0, 0}, {200, 200});
            h.ctx.End();
        }
        h.Win("B", {100, 100}, {200, 200});
        h.ctx.End();
        h.End();
    };
    frame();
    frame();
    ESIA_CHECK(h.ctx.WindowsInDrawOrder().back()->Name() == "B");
    showA = false;
    frame();
    showA = true;
    frame();
    ESIA_CHECK(h.ctx.WindowsInDrawOrder().back()->Name() == "A");
    ESIA_CHECK(h.ctx.FocusedWindow() && h.ctx.FocusedWindow()->Name() == "A");
    ESIA_CHECK(h.ctx.FindWindowByName("A")->Appearing());
}

// bug 9: focus, hover and drag let go of a window that is no longer submitted
ESIA_TEST(Context, FocusAndDragClearedWhenWindowGone)
{
    Harness h;
    bool show = true;
    auto frame = [&] {
        h.Frame();
        if (show)
        {
            h.Win("D", {100, 100}, {200, 150});
            h.ctx.End();
        }
        h.End();
    };
    h.Move({150, 150});
    frame();
    frame();
    h.Down();
    frame();   // click on the empty area: focus, and a move starts
    ESIA_CHECK(h.ctx.FocusedWindow() && h.ctx.ActiveId() != 0);
    h.Move({160, 160});
    frame();
    show = false;
    frame();
    frame();
    ESIA_CHECK(h.ctx.FocusedWindow() == nullptr);
    ESIA_CHECK(h.ctx.HoveredWindow() && h.ctx.HoveredWindow()->Name() == "##root");
    ESIA_CHECK(h.ctx.ActiveId() == 0);
    h.Up();
    frame();
    show = true;
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("D")->GetRect().min == Vec2(110, 110));   // it moved while dragged, no further
}

// bug 9: dynamically named windows do not pile up
ESIA_TEST(Context, WindowGc)
{
    ContextDesc d;
    d.retainFrames = 5;
    Harness h(d);
    for (int i = 0; i < 20; ++i)
    {
        h.Frame();
        const std::string name = "doc " + std::to_string(i);
        h.Win(name.c_str(), {0, 0}, {100, 100});
        h.ctx.End();
        h.End();
    }
    int alive = 0;
    for (int i = 0; i < 20; ++i)
        alive += h.ctx.FindWindowByName("doc " + std::to_string(i)) ? 1 : 0;
    ESIA_CHECK(alive <= 7);
    ESIA_CHECK(h.ctx.FindWindowByName("doc 19") && !h.ctx.FindWindowByName("doc 0"));

    // a child region's scroll state goes the same way
    float scroll = -1.0f;
    auto frame = [&](bool child, bool request) {
        h.Frame();
        h.Win("host", {0, 0}, {300, 300});
        if (child)
        {
            ChildOptions c;
            c.size = {100, 100};
            c.flags = ChildFlags_ScrollY;
            h.ctx.BeginChild("list", c);
            if (request)
                h.ctx.SetScrollY(50);
            for (int i = 0; i < 10; ++i)
                h.ctx.ItemSize({50, 30});
            scroll = h.ctx.Scroll().y;
            h.ctx.EndChild();
        }
        h.ctx.End();
        h.End();
    };
    frame(true, true);
    frame(true, false);
    ESIA_CHECK(scroll == 50.0f);
    for (int i = 0; i < 3; ++i)
        frame(false, false);
    frame(true, false);
    ESIA_CHECK(scroll == 50.0f);   // kept while recently used
    for (int i = 0; i < 10; ++i)
        frame(false, false);
    frame(true, false);
    ESIA_CHECK(scroll == 0.0f);    // freed, starts over
}

// bug 12: the first frame of a resize does not snap the edge to the pointer
ESIA_TEST(Context, ResizeKeepsGrabOffset)
{
    Harness h;
    auto frame = [&] {
        h.Frame();
        h.Win("R", {100, 100}, {200, 150});
        h.ctx.End();
        h.End();
    };
    h.Move({303, 200});   // right edge band, 3 units outside
    frame();
    frame();
    ESIA_CHECK(h.ctx.Requests().cursor == MouseCursor::ResizeEW);
    h.Down();
    frame();
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("R")->GetRect() == Rect(100, 100, 300, 250));
    h.Move({313, 200});
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("R")->GetRect() == Rect(100, 100, 310, 250));
    h.Up();
    frame();
}

// bug 12: windows stay on screen after a drag and after the display shrinks
ESIA_TEST(Context, KeepOnScreen)
{
    Harness h;
    auto frame = [&] {
        h.Frame();
        h.Win("K", {100, 100}, {200, 150});
        h.ctx.End();
        h.End();
    };
    h.Move({150, 150});
    frame();
    frame();
    h.Down();
    frame();
    h.Move({1500, -300});   // far past the right and the top
    frame();
    const Rect r = h.ctx.FindWindowByName("K")->GetRect();
    ESIA_CHECK(r.min.x == 800.0f - 24.0f && r.min.y == 0.0f);
    h.Up();
    frame();
    h.display = Vec2(400, 300);   // the display shrinks (a resolution change)
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("K")->GetRect().min.x == 400.0f - 24.0f);
    ESIA_CHECK(h.ctx.FindWindowByName("K")->GetRect().Size() == Vec2(200, 150));
}

// DPI per window: the monitor under the window gives its scale; moving to another keeps the physical size
ESIA_TEST(Context, DpiScalePerWindow)
{
    Harness h;
    h.ctx.SetMonitors({Monitor{Rect(0, 0, 400, 600), 1.0f}, Monitor{Rect(400, 0, 800, 600), 2.0f}});
    Vec2 pos(450, 100);
    float scale = 0.0f;
    bool changed = false;
    auto frame = [&] {
        h.Frame();
        h.ctx.SetNextWindowPos(pos);
        h.ctx.SetNextWindowSize({100, 100}, Cond::FirstUse);
        h.ctx.Begin("M");
        scale = h.ctx.Scale();
        changed = h.ctx.CurrentWindow()->ScaleChanged();
        h.ctx.End();
        h.End();
    };
    frame();
    ESIA_CHECK(scale == 2.0f && !changed);
    pos = Vec2(100, 100);
    frame();
    ESIA_CHECK(scale == 1.0f && changed);
    ESIA_CHECK(h.ctx.FindWindowByName("M")->GetRect().Size() == Vec2(50, 50));
    frame();
    ESIA_CHECK(!changed);
}

ESIA_TEST(Context, AutoSizeWindow)
{
    Harness h;
    auto frame = [&] {
        h.Frame();
        WindowOptions o;
        o.flags = WindowFlags_AutoSize;
        o.minSize = {0, 0};
        h.ctx.SetNextWindowPos({10, 10});
        h.ctx.Begin("A", o);
        h.ctx.ItemSize({120, 30});
        h.ctx.ItemSize({80, 30});
        h.ctx.WindowDrawList().AddRectFilled(Rect(10, 10, 20, 20), 0xFFFFFFFFu);
        h.ctx.End();
        h.End();
    };
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("A")->Hidden());
    ESIA_CHECK(h.ctx.GetDrawData().lists.empty());   // not drawn at a size it does not have yet
    frame();
    const Window* w = h.ctx.FindWindowByName("A");
    ESIA_CHECK(!w->Hidden() && w->GetRect() == Rect(10, 10, 10 + 120 + 24, 10 + 68 + 24));
    ESIA_CHECK(h.ctx.GetDrawData().lists.size() == 1);
}

// popups: open, measured and shown, click-away consumed, Escape
ESIA_TEST(Context, PopupClickAway)
{
    Harness h;
    const Id menu = 0x51;
    bool open = false, row = false;
    ButtonResult under;
    bool consume = true;
    auto frame = [&] {
        h.Frame();
        h.Win("W", {0, 0}, {400, 400});
        under = h.Button("under", Rect(200, 200, 300, 250));
        h.ctx.End();
        PopupOptions po;
        po.pos = {20, 20};
        po.consumeClickAway = consume;
        open = h.ctx.BeginPopup(menu, po);
        if (open)
        {
            row = h.Button("row", Vec2(100, 20)).hovered;
            h.ctx.EndPopup();
        }
        h.End();
    };
    frame();
    ESIA_CHECK(!open && !h.ctx.IsPopupOpen(menu));
    h.ctx.OpenPopup(menu);
    frame();
    ESIA_CHECK(open && h.ctx.FindWindowByName("##popup") == nullptr);   // popups are found by id, not by name
    frame();
    h.Move({40, 40});
    frame();
    ESIA_CHECK(row);   // over the popup's row: the popup is on top of the window
    // a click away closes it and does nothing else
    h.Move({250, 220});
    frame();
    h.Down();
    frame();
    ESIA_CHECK(!h.ctx.IsPopupOpen(menu) && !under.held && h.ctx.Requests().wantCaptureMouse);
    h.Up();
    frame();
    ESIA_CHECK(!under.pressed);
    // with consumeClickAway off the click goes through
    consume = false;
    h.ctx.OpenPopup(menu);
    frame();
    frame();
    h.Down();
    frame();
    ESIA_CHECK(!h.ctx.IsPopupOpen(menu) && under.held);
    h.Up();
    frame();
    ESIA_CHECK(under.pressed);
    // Escape closes the top popup unless something claimed it
    h.ctx.OpenPopup(menu);
    frame();
    h.KeyDown(Key::Escape);
    frame();
    ESIA_CHECK(!h.ctx.IsPopupOpen(menu));
    h.KeyUp(Key::Escape);
    frame();
}

ESIA_TEST(Context, PopupStackAndLifetime)
{
    Harness h;
    const Id menu = 1, sub1 = 2, sub2 = 3, other = 4;
    int openSub = 0;
    bool submitMenu = true;
    auto frame = [&] {
        h.Frame();
        PopupOptions po;
        po.pos = {700, 550};   // near the corner: kept on screen
        if (submitMenu && h.ctx.BeginPopup(menu, po))
        {
            h.ctx.ItemSize({150, 100});
            if (openSub)
                h.ctx.OpenPopup(openSub == 1 ? sub1 : sub2);
            openSub = 0;
            if (h.ctx.BeginPopup(sub1))
                h.ctx.EndPopup();
            if (h.ctx.BeginPopup(sub2))
                h.ctx.EndPopup();
            h.ctx.EndPopup();
        }
        h.End();
    };
    h.ctx.OpenPopup(menu);
    frame();
    frame();
    frame();
    const Window* w = h.ctx.WindowsInDrawOrder().back();
    ESIA_CHECK(w->GetRect().max.x <= 800.0f && w->GetRect().max.y <= 600.0f && w->Layer() == WindowLayer::Overlay);
    openSub = 1;
    frame();
    ESIA_CHECK(h.ctx.IsPopupOpen(menu) && h.ctx.IsPopupOpen(sub1));
    openSub = 2;   // a sibling submenu replaces the first
    frame();
    ESIA_CHECK(h.ctx.IsPopupOpen(menu) && !h.ctx.IsPopupOpen(sub1) && h.ctx.IsPopupOpen(sub2));
    h.ctx.OpenPopup(other);   // from outside any popup: replaces them all
    ESIA_CHECK(!h.ctx.IsPopupOpen(menu) && !h.ctx.IsPopupOpen(sub2) && h.ctx.IsPopupOpen(other));
    frame();   // opened last frame: still open although not submitted yet
    frame();
    ESIA_CHECK(!h.ctx.IsPopupOpen(other));   // not submitted: closed
    h.ctx.OpenPopup(menu);
    frame();
    submitMenu = false;
    frame();
    frame();
    ESIA_CHECK(!h.ctx.IsPopupOpen(menu));
}

ESIA_TEST(Context, Tooltip)
{
    Harness h;
    bool hoveredBelow = false;
    auto frame = [&] {
        h.Frame();
        h.Win("W", {0, 0}, {400, 400});
        hoveredBelow = h.Button("b", Rect(20, 20, 380, 380)).hovered;
        h.ctx.End();
        h.ctx.BeginTooltip();
        h.ctx.ItemSize({60, 20});
        h.ctx.EndTooltip();
        h.ctx.BeginTooltip();   // a second part in the same frame goes to the same tooltip
        h.ctx.ItemSize({60, 20});
        h.ctx.EndTooltip();
        h.End();
    };
    h.Move({100, 100});
    frame();
    frame();
    frame();
    const Window* tip = h.ctx.FindWindowByName("##tooltip");
    ESIA_CHECK(tip && !tip->Hidden() && tip->Layer() == WindowLayer::Tooltip);
    ESIA_CHECK(tip->GetRect().min == Vec2(116, 116) && tip->GetRect().Height() == 20 + 8 + 20 + 24);
    ESIA_CHECK(hoveredBelow);   // a tooltip never takes the hover
    h.Move({790, 590});
    frame();
    frame();
    ESIA_CHECK(tip->GetRect().max.x <= 800.0f && tip->GetRect().max.y <= 600.0f);
}

namespace
{
    struct Spring
    {
        float value = 0.0f;
        int frames = 0;
    };
}

ESIA_TEST(Context, StateStorage)
{
    ContextDesc d;
    d.retainFrames = 10;
    Harness h(d);
    h.Frame();
    h.ctx.State<Spring>(7).value = 3.0f;
    h.ctx.State<int>(7) = 42;   // the same id holds one entry per type
    h.End();
    for (int i = 0; i < 5; ++i)
    {
        h.Frame();
        ++h.ctx.State<Spring>(7).frames;
        h.End();
    }
    h.Frame();
    ESIA_CHECK(h.ctx.State<Spring>(7).value == 3.0f && h.ctx.State<Spring>(7).frames == 5);
    h.End();
    for (int i = 0; i < 200; ++i)
    {
        h.Frame();
        h.End();
    }
    h.Frame();
    ESIA_CHECK(h.ctx.State<Spring>(7).value == 0.0f && h.ctx.State<int>(7) == 0);   // unused: collected
    h.End();
}

// The table directly: thousands of entries of two types, every third one left unused and collected (deletions in
// the middle of probe chains), the others found again at the same address with their values.
ESIA_TEST(Context, StateStorageTable)
{
    StateStorage st;
    constexpr int kCount = 3000;
    std::vector<int*> ints(kCount);
    std::vector<Spring*> springs(kCount);
    for (int i = 0; i < kCount; ++i)
    {
        const Id id = (Id)(i * 7919u + 1u);
        ints[(std::size_t)i] = &st.Get<int>(id, 1);
        *ints[(std::size_t)i] = i;
        springs[(std::size_t)i] = &st.Get<Spring>(id, 1);
        springs[(std::size_t)i]->frames = -i;
    }
    ESIA_CHECK(st.Size() == 2u * kCount);
    for (int i = 0; i < kCount; ++i)
        if (i % 3 != 0)
        {
            const Id id = (Id)(i * 7919u + 1u);
            ESIA_CHECK(&st.Get<int>(id, 50) == ints[(std::size_t)i]);   // the entries never move
            st.Get<Spring>(id, 50);
        }
    st.Collect(60, 20);   // drops what was last used at frame 1
    ESIA_CHECK(st.Size() == 2u * (kCount - (kCount + 2) / 3));
    bool same = true;
    for (int i = 0; i < kCount; ++i)
        if (i % 3 != 0)
        {
            const Id id = (Id)(i * 7919u + 1u);
            same = same && &st.Get<int>(id, 60) == ints[(std::size_t)i] && st.Get<int>(id, 60) == i;
            same = same && &st.Get<Spring>(id, 60) == springs[(std::size_t)i] && st.Get<Spring>(id, 60).frames == -i;
        }
    ESIA_CHECK(same);
    ESIA_CHECK(st.Size() == 2u * (kCount - (kCount + 2) / 3));   // found, not added again
    ESIA_CHECK(st.Get<int>((Id)1u, 60) == 0);   // collected (i = 0): a new, default entry
    st.Clear();
    ESIA_CHECK(st.Size() == 0u);
}

ESIA_TEST(Context, ScopeData)
{
    Harness h;
    static const char kStyle = 0;
    const int windowStyle = 1, cardStyle = 2;
    h.Frame();
    h.Win("S", {0, 0}, {300, 300});
    ESIA_CHECK(h.ctx.FindScopeData(&kStyle) == nullptr);
    h.ctx.SetScopeData(&kStyle, &windowStyle);
    h.ctx.BeginContainer(h.ctx.GetId("card"));
    ESIA_CHECK(h.ctx.FindScopeData(&kStyle) == &windowStyle);   // inherited
    h.ctx.SetScopeData(&kStyle, &cardStyle);
    h.ctx.BeginGroup();
    ESIA_CHECK(h.ctx.FindScopeData(&kStyle) == &cardStyle);
    h.ctx.EndGroup();
    h.ctx.EndContainer();
    ESIA_CHECK(h.ctx.FindScopeData(&kStyle) == &windowStyle);   // ended with its container: nothing to pop
    h.ctx.BeginTooltip();
    ESIA_CHECK(h.ctx.FindScopeData(&kStyle) == &windowStyle);   // begun inside the window: inherits
    h.ctx.EndTooltip();
    h.ctx.End();
    ESIA_CHECK(h.ctx.FindScopeData(&kStyle) == nullptr);
    h.End();
}

// review: a window at a monitor boundary does not flip between two scales every frame
ESIA_TEST(Context, DpiBoundaryHysteresis)
{
    Harness h;
    h.display = Vec2(2000, 1000);
    h.ctx.SetMonitors({Monitor{Rect(0, 0, 1000, 1000), 2.0f}, Monitor{Rect(1000, 0, 2000, 1000), 1.0f}});
    float x = 100.0f;
    int changes = 0;
    auto frame = [&] {
        h.Frame();
        h.ctx.SetNextWindowPos(Vec2(x, 100));
        h.ctx.SetNextWindowSize({900, 400}, Cond::FirstUse);
        h.ctx.Begin("M");
        changes += h.ctx.CurrentWindow()->ScaleChanged() ? 1 : 0;
        h.ctx.End();
        h.End();
    };
    frame();
    x = 600.0f;   // center at 1050, on the scale-1 monitor; halved (450) it would be back on the scale-2 one
    for (int i = 0; i < 6; ++i)
        frame();
    ESIA_CHECK(changes == 0 && h.ctx.FindWindowByName("M")->Scale() == 2.0f && h.ctx.FindWindowByName("M")->GetRect().Width() == 900.0f);
    x = 1200.0f;   // well onto it: switches once
    for (int i = 0; i < 6; ++i)
        frame();
    ESIA_CHECK(changes == 1 && h.ctx.FindWindowByName("M")->Scale() == 1.0f && h.ctx.FindWindowByName("M")->GetRect().Width() == 450.0f);
}

// review: SetNextScroll on an area's first frame, past last frame's (empty) range
ESIA_TEST(Context, SetNextScrollOnNewArea)
{
    Harness h;
    float scroll = -1.0f;
    auto frame = [&](bool set) {
        h.Frame();
        h.Win("W", {0, 0}, {300, 300});
        if (set)
            h.ctx.SetNextScroll({-1, 200});
        ChildOptions c;
        c.size = {200, 100};
        c.flags = ChildFlags_ScrollY;
        h.ctx.BeginChild("list", c);
        for (int i = 0; i < 20; ++i)
            h.ctx.ItemSize({50, 22});   // 592 tall: 492 to scroll
        scroll = h.ctx.Scroll().y;
        h.ctx.EndChild();
        h.ctx.End();
        h.End();
    };
    frame(true);
    ESIA_CHECK(scroll == 200.0f);
    frame(false);
    ESIA_CHECK(scroll == 200.0f);   // kept: the end of the first frame clamped it to the range it measured

    // a window appended to in the same frame: its layout already started, the request lands at its End (and does
    // not leak into a later child)
    h.Frame();
    WindowOptions o;
    o.padding = {0, 0};
    h.Win("L", {0, 0}, {200, 100}, o);
    h.ctx.End();
    h.ctx.SetNextScroll({-1, 150});
    h.ctx.Begin("L", o);
    for (int i = 0; i < 20; ++i)
        h.ctx.ItemSize({50, 22});
    ChildOptions c;
    c.size = {100, 50};
    c.flags = ChildFlags_ScrollY;
    h.ctx.BeginChild("c", c);
    h.ctx.ItemSize({10, 200});
    const float childScroll = h.ctx.Scroll().y;
    h.ctx.EndChild();
    h.ctx.End();
    h.End();
    ESIA_CHECK(childScroll == 0.0f && h.ctx.FindWindowByName("L")->Scroll().y == 150.0f);
}

// review: a fractional smooth-scroll target ends on a whole pixel, and the glide stops there
ESIA_TEST(Child, SmoothScrollEndsOnPixel)
{
    Harness h;
    float scroll = 0.0f;
    bool request = false;
    auto frame = [&] {
        h.Frame();
        h.Win("W", {0, 0}, {300, 300});
        ChildOptions c;
        c.size = {200, 100};
        c.flags = ChildFlags_ScrollY | ChildFlags_SmoothScroll;
        h.ctx.BeginChild("area", c);
        if (request)
            h.ctx.SetScrollY(40.3f);
        request = false;
        for (int i = 0; i < 20; ++i)
            h.ctx.ItemSize({50, 22});
        scroll = h.ctx.Scroll().y;
        h.ctx.EndChild();
        h.ctx.End();
        h.End();
    };
    frame();
    request = true;
    frame();
    for (int i = 0; i < 60 && h.ctx.Requests().animating; ++i)
        frame();
    ESIA_CHECK(!h.ctx.Requests().animating && scroll == 40.0f);
}

namespace
{
    int gThrow = 1;
    struct Fragile
    {
        int v = 5;
        Fragile()
        {
            if (gThrow > 0)
            {
                --gThrow;
                throw 1;
            }
        }
    };
}

// review: a constructor that throws leaves no half-made entry behind (collecting it would call a null destroy)
ESIA_TEST(Context, StateStorageThrowingConstructor)
{
    StateStorage st;
    bool threw = false;
    try
    {
        st.Get<Fragile>(1, 1);
    }
    catch (int)
    {
        threw = true;
    }
    ESIA_CHECK(threw && st.Size() == 0);
    ESIA_CHECK(st.Get<Fragile>(1, 2).v == 5 && st.Size() == 1);
    st.Collect(1000, 10);
    ESIA_CHECK(st.Size() == 0);
}

// An auto-sized popup anchored at its right edge between two pixels: it lands on whole pixels and stays put (its
// measured size used to depend on where it started between pixels, and its place on that size: it crept and jumped)
ESIA_TEST(Context, AutoSizedPopupAtItsRightEdgeStaysPut)
{
    Harness h;
    h.fbScale = {1.25f, 1.25f};
    const Id popup = h.ctx.GetId("menu");
    StackLayout rows;
    rows.spacing = 0.0f;
    std::vector<Rect> seen;
    auto frame = [&] {
        h.Frame();
        PopupOptions po;
        po.pos = {300.5f, 40.0f};
        po.pivot = {1.0f, 0.0f};
        po.padding = {6.0f, 6.0f};
        if (h.ctx.BeginPopup(popup, po))
        {
            ContainerOptions co;
            co.layout = &rows;
            h.ctx.BeginContainer(1, co);
            h.ctx.ItemSize({120.7f, 34.0f});
            h.ctx.ItemSize({96.0f, 34.0f});
            h.ctx.EndContainer();
            seen.push_back(h.ctx.CurrentWindow()->GetRect());
            h.ctx.EndPopup();
        }
        h.End();
    };
    h.Frame();
    h.ctx.OpenPopup(popup);
    h.End();
    for (int i = 0; i < 12; ++i)
        frame();
    ESIA_CHECK(seen.size() == 12);
    for (std::size_t i = 3; i < seen.size(); ++i)
        ESIA_CHECK(seen[i] == seen[2]);
    const Rect& r = seen.back();
    const auto whole = [](float v) { return std::fabs(v * 1.25f - std::round(v * 1.25f)) < 1e-3f; };
    ESIA_CHECK(whole(r.min.x) && whole(r.min.y) && whole(r.Width()) && whole(r.Height()));
    ESIA_CHECK(r.max.x <= 300.5f + 1e-3f && r.max.x > 300.5f - 0.8f);   // its right edge on the anchor's pixel
}

// The resize grip (WindowOptions::resizeGrip): the square at the bottom-right corner resizes the window both ways,
// above the item there; away from it the item keeps the mouse
ESIA_TEST(Context, ResizeGripIsAboveTheItemsAtTheCorner)
{
    Harness h;
    ButtonResult b;
    int presses = 0;
    auto frame = [&] {
        h.Frame();
        WindowOptions o;
        o.padding = {0, 0};
        o.resizeGrip = 26.0f;
        h.Win("R", {100, 100}, {200, 150}, o);
        b = h.Button("corner", Rect(220, 170, 300, 250));   // reaches into the corner
        presses += b.pressed ? 1 : 0;
        h.ctx.End();
        h.End();
    };
    h.Move({290, 240});   // in the grip, 10 in from the corner: outside the edges' 5-unit bands
    frame();
    frame();
    ESIA_CHECK(h.ctx.Requests().cursor == MouseCursor::ResizeNWSE && !b.hovered);
    h.Down();
    frame();
    h.Move({320, 260});
    frame();
    ESIA_CHECK(h.ctx.FindWindowByName("R")->GetRect() == Rect(100, 100, 330, 270));
    h.Up();
    frame();
    ESIA_CHECK(presses == 0);
    h.Move({240, 190});   // the item, away from the grip
    frame();
    frame();
    ESIA_CHECK(b.hovered && h.ctx.Requests().cursor == MouseCursor::Arrow);
}
