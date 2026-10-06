// Layout: containers, providers, laid-out item reports, baselines, work rects, pixel snapping; child regions and
// their scrolling (docs/UI_CORE.md, sections 6 and 8).
#include "core_harness.hpp"
#include "esia_test.hpp"
#include <cmath>
#include <vector>

using namespace esia;
using esia::test::Harness;

ESIA_TEST(Layout, ProviderSlotsAndReports)
{
    Harness h;
    std::vector<LaidOutItem> seen;
    h.ctx.SetItemObserver([&](const LaidOutItem& it) { seen.push_back(it); });
    Rect row;
    Vec2 availInSlot;
    auto frame = [&] {
        seen.clear();
        h.Frame();
        WindowOptions o;
        o.padding = {10, 10};
        h.Win("P", {0, 0}, {400, 300}, o);
        StackLayout& L = h.ctx.State<StackLayout>(h.ctx.GetId("row"));
        L.horizontal = true;
        L.spacing = 6;
        ContainerOptions co;
        co.layout = &L;
        h.ctx.BeginContainer(h.ctx.GetId("row"), co);
        h.ctx.ItemSize({50, 20});
        availInSlot = h.ctx.ContentRegionAvail();   // the slot's region: from the second child to the right edge
        h.ctx.BeginGroup();                          // a nested container is one child
        h.ctx.ItemSize({30, 10});
        h.ctx.ItemSize({30, 10});
        h.ctx.EndGroup();
        h.ctx.ItemSize({40, 30});
        row = h.ctx.EndContainer();
        h.ctx.ItemSize({10, 10});                    // after the container, below it
        h.ctx.End();
        h.End();
    };
    frame();
    ESIA_CHECK(row == Rect(10, 10, 10 + 50 + 6 + 30 + 6 + 40, 40));
    ESIA_CHECK(availInSlot == Vec2(390 - 66, 290 - 10));
    // reports: every item with its depth and container (the row's children at 1, the group's at 2)
    const Id rowId = seen[0].container;
    ESIA_CHECK(seen.size() == 7 && rowId != 0);
    ESIA_CHECK(seen[0].rect == Rect(10, 10, 60, 30) && seen[0].depth == 1);
    ESIA_CHECK(seen[1].rect == Rect(66, 10, 96, 20) && seen[1].depth == 2 && seen[1].container == 0);
    ESIA_CHECK(seen[2].rect == Rect(66, 28, 96, 38) && seen[2].depth == 2);
    ESIA_CHECK(seen[3].rect == Rect(66, 10, 96, 38) && seen[3].depth == 1 && seen[3].container == rowId);   // the group
    ESIA_CHECK(seen[4].rect == Rect(102, 10, 142, 40) && seen[4].depth == 1);
    ESIA_CHECK(seen[5].rect == row && seen[5].depth == 0 && seen[5].container == 0);   // the row, in the window's content
    ESIA_CHECK(seen[6].rect == Rect(10, 48, 20, 58));
    // the window keeps last frame's list
    frame();
    const auto& items = h.ctx.FindWindowByName("P")->LaidOutItems();
    ESIA_CHECK(items.size() == 7 && items[5].rect == row);
}

ESIA_TEST(Layout, BaselineAlignment)
{
    Harness h;
    std::vector<Rect> placed;
    auto frame = [&] {
        placed.clear();
        h.Frame();
        WindowOptions o;
        o.padding = {0, 0};
        h.Win("B", {0, 0}, {400, 300}, o);
        // a row of a tall field (baseline 20) and a label (baseline 12): aligned on the second frame
        StackLayout& L = h.ctx.State<StackLayout>(1);
        L.horizontal = true;
        L.spacing = 4;
        L.align = Align::Baseline;
        ContainerOptions co;
        co.layout = &L;
        h.ctx.BeginContainer(1, co);
        placed.push_back(Rect::FromSize(h.ctx.CursorPos(), Vec2(100, 32)));
        h.ctx.ItemSize({100, 32}, 20);
        placed.push_back(Rect::FromSize(h.ctx.CursorPos(), Vec2(60, 16)));
        h.ctx.ItemSize({60, 16}, 12);
        const Rect r = h.ctx.EndContainer();
        ESIA_CHECK(h.ctx.LastItemRect() == r);
        // the layout cursor: a label after the field on the same line
        h.ctx.ItemSize({100, 32}, 20);
        h.ctx.SameLine();
        ESIA_CHECK(h.ctx.LineBaseline() == 20.0f);
        const float moved = h.ctx.AlignToLineBaseline(12);
        placed.push_back(Rect::FromSize(h.ctx.CursorPos(), Vec2(60, 16)));
        h.ctx.ItemSize({60, 16}, 12);
        ESIA_CHECK(moved == 8.0f);
        placed.push_back(Rect::FromSize(h.ctx.CursorPos(), Vec2(10, 10)));   // the next line starts below the field
        h.ctx.End();
        h.End();
    };
    frame();
    frame();
    ESIA_CHECK(placed[0].min.y + 20 == placed[1].min.y + 12);   // baselines line up
    ESIA_CHECK(placed[1].min.x == 104.0f);
    ESIA_CHECK(placed[2].min == Vec2(108, 40 + 8) && placed[2].min.y + 12 == 40 + 20);
    ESIA_CHECK(placed[3].min.y == 40 + 32 + 8);
}

ESIA_TEST(Layout, ContainerWorkRect)
{
    Harness h;
    h.Frame();
    WindowOptions o;
    o.padding = {10, 10};
    h.Win("W", {0, 0}, {400, 300}, o);
    ESIA_CHECK(h.ctx.WorkRect() == Rect(10, 10, 390, 290) && h.ctx.CurrentDepth() == 0);
    // a card: the available width, padded; what fills its width stays inside it (WGT narrowed WorkRect for this)
    ContainerOptions card;
    card.fillWidth = true;
    card.size = {-20, 0};
    card.padding = {16, 12};
    h.ctx.BeginContainer(h.ctx.GetId("card"), card);
    ESIA_CHECK(h.ctx.CurrentDepth() == 1);
    ESIA_CHECK(h.ctx.CursorPos() == Vec2(26, 22));
    ESIA_CHECK(h.ctx.WorkRect().max.x == 10 + 360 - 16);
    ESIA_CHECK(h.ctx.ContentRegionAvail().x == 360 - 32);
    h.ctx.ItemSize({h.ctx.ContentRegionAvail().x, 30});
    h.ctx.ItemSize({50, 30});
    const Rect r = h.ctx.EndContainer();
    ESIA_CHECK(r == Rect(10, 10, 370, 10 + 12 + 30 + 8 + 30 + 12));
    ESIA_CHECK(h.ctx.WorkRect() == Rect(10, 10, 390, 290) && h.ctx.CursorPos() == Vec2(10, r.max.y + 8));
    // a fixed-size container reports its size whatever its content measures
    ContainerOptions fixed;
    fixed.size = {100, 50};
    h.ctx.BeginContainer(0, fixed);
    h.ctx.ItemSize({300, 300});
    ESIA_CHECK(h.ctx.EndContainer().Size() == Vec2(100, 50));
    h.ctx.End();
    h.End();
}

// the layout cursor lands on physical pixels at a fractional scale
ESIA_TEST(Layout, PixelSnapping)
{
    Harness h;
    h.fbScale = Vec2(1.25f, 1.25f);
    h.Frame();
    WindowOptions o;
    o.padding = {0, 0};
    h.Win("S", {0, 0}, {400, 300}, o);
    ESIA_CHECK(h.ctx.Scale() == 1.25f);
    for (int i = 0; i < 6; ++i)
    {
        h.ctx.ItemSize({10.3f, 10.3f});
        const Vec2 c = h.ctx.CursorPos() * 1.25f;
        ESIA_CHECK(std::fabs(c.y - std::round(c.y)) < 1e-3f && std::fabs(c.x - std::round(c.x)) < 1e-3f);
    }
    h.ctx.SameLine();
    const float px = h.ctx.CursorPos().x * 1.25f;
    ESIA_CHECK(std::fabs(px - std::round(px)) < 1e-3f);
    h.ctx.End();
    h.End();
}

// child regions: their own clip, scroll and ids; the wheel goes to the innermost area that can still move
ESIA_TEST(Child, NestedScrollAreasAndWheel)
{
    Harness h;
    Vec2 innerScroll, outerScroll;
    Id innerButton = 0, outerButton = 0;
    auto frame = [&](double dt = 1.0 / 60.0) {
        h.Frame(dt);
        WindowOptions o;
        o.padding = {0, 0};
        h.Win("W", {0, 0}, {400, 400}, o);
        ChildOptions outer;
        outer.size = {300, 200};
        outer.flags = ChildFlags_ScrollY;
        h.ctx.BeginChild("outer", outer);
        outerButton = h.ctx.GetId("b");
        ChildOptions inner;
        inner.size = {0, 100};   // the available width
        inner.flags = ChildFlags_ScrollY;
        h.ctx.BeginChild("inner", inner);
        innerButton = h.ctx.GetId("b");
        ESIA_CHECK(h.ctx.ContentRegionAvail().x == 300.0f);
        for (int i = 0; i < 10; ++i)
            h.ctx.ItemSize({50, 22});   // 10 * 30 - 8 = 292: 192 to scroll
        innerScroll = h.ctx.Scroll();
        h.ctx.EndChild();
        for (int i = 0; i < 10; ++i)
            h.ctx.ItemSize({50, 22});   // 108 + 300 - 8 = 400: 200 to scroll
        outerScroll = h.ctx.Scroll();
        h.ctx.EndChild();
        h.ctx.End();
        h.End();
    };
    h.Move({50, 50});   // over the inner area
    frame();
    frame();
    ESIA_CHECK(innerButton != outerButton);   // ids inside a child are its own
    h.Wheel(0, -2);   // 96 down: the inner area takes it
    frame();
    frame();
    ESIA_CHECK(innerScroll.y == 96.0f && outerScroll.y == 0.0f);
    h.Wheel(0, -3);   // 144 more: the inner area stops at 192
    frame();
    frame();
    ESIA_CHECK(innerScroll.y == 192.0f && outerScroll.y == 0.0f);
    h.Wheel(0, -1);   // at its end while the wheel turns on: it keeps the wheel (the outer area does not move)
    frame();
    frame();
    ESIA_CHECK(innerScroll.y == 192.0f && outerScroll.y == 0.0f);
    frame(0.5);       // a pause: the next notch goes to the outer area
    h.Wheel(0, -1);
    frame();
    frame();
    ESIA_CHECK(innerScroll.y == 192.0f && outerScroll.y == 48.0f);
    h.Move({50, 150});   // over the outer area's own content (the inner one scrolled up by 48)
    h.Wheel(0, 1);
    frame();
    frame();
    ESIA_CHECK(outerScroll.y == 0.0f && innerScroll.y == 192.0f);
    ESIA_CHECK(h.ctx.FindWindowByName("W")->Scroll().y == 0.0f);
}

ESIA_TEST(Child, SmoothScroll)
{
    Harness h;
    float scroll = 0.0f;
    auto frame = [&] {
        h.Frame();
        WindowOptions o;
        o.padding = {0, 0};
        h.Win("W", {0, 0}, {400, 400}, o);
        ChildOptions c;
        c.size = {200, 100};
        c.flags = ChildFlags_ScrollY | ChildFlags_SmoothScroll;
        h.ctx.BeginChild("area", c);
        for (int i = 0; i < 20; ++i)
            h.ctx.ItemSize({50, 22});
        scroll = h.ctx.Scroll().y;
        h.ctx.EndChild();
        h.ctx.End();
        h.End();
    };
    h.Move({50, 50});
    frame();
    frame();
    h.Wheel(0, -2);   // a target 96 down: the offset glides there
    frame();
    frame();
    const float first = scroll;
    ESIA_CHECK(first > 0.0f && first < 96.0f && h.ctx.Requests().animating);
    ESIA_CHECK(scroll == std::round(scroll));   // on whole pixels at scale 1
    for (int i = 0; i < 60 && h.ctx.Requests().animating; ++i)
        frame();
    ESIA_CHECK(scroll == 96.0f && !h.ctx.Requests().animating);
}

ESIA_TEST(Child, FloatingAndClipInset)
{
    Harness h;
    Vec2 after;
    bool underCut = true, visible = false;
    std::vector<LaidOutItem> seen;
    h.ctx.SetItemObserver([&](const LaidOutItem& it) { seen.push_back(it); });
    h.Frame();
    WindowOptions o;
    o.padding = {0, 0};
    h.Win("W", {0, 0}, {400, 400}, o);
    h.ctx.ItemSize({100, 20});
    // a floating bar over the content: the layout does not move, its report is flagged
    ChildOptions bar;
    bar.flags = ChildFlags_Floating;
    bar.rect = Rect(0, 340, 400, 400);
    h.ctx.BeginChild("bar", bar);
    h.ctx.ItemSize({60, 40});
    h.ctx.EndChild();
    after = h.ctx.CursorPos();
    // a rounded area: its clip moves in by 10 at the bottom
    ChildOptions area;
    area.size = {200, 100};
    area.clipInset = Rect(0, 0, 0, 10);
    h.ctx.BeginChild("area", area);
    underCut = h.ctx.ItemAdd(0, Rect(0, 120, 50, 128));      // inside the area, in the cut
    visible = h.ctx.ItemAdd(0, Rect(0, 100, 50, 110));
    h.ctx.EndChild();
    h.ctx.End();
    h.End();
    ESIA_CHECK(after == Vec2(0, 28));
    ESIA_CHECK(!underCut && visible);
    ESIA_CHECK(seen.size() == 4);
    ESIA_CHECK(seen[1].floating && seen[1].depth == 1);           // inside the bar
    ESIA_CHECK(seen[2].floating && seen[2].depth == -1 && seen[2].rect == Rect(0, 340, 400, 400));   // the bar
    ESIA_CHECK(!seen[3].floating && seen[3].rect == Rect(0, 28, 200, 128));
}

// review: a fitted vertical stack centers its children against the widest one, and stays as wide as its content
ESIA_TEST(Layout, StackCenterFitted)
{
    Harness h;
    Rect r, second;
    auto frame = [&](bool inRegion) {
        h.Frame();
        WindowOptions o;
        o.padding = {0, 0};
        h.Win("S", {0, 0}, {500, 300}, o);
        StackLayout& L = h.ctx.State<StackLayout>(inRegion ? 2 : 1);
        L.spacing = 0;
        L.align = Align::Center;
        L.alignInRegion = inRegion;
        ContainerOptions co;
        co.layout = &L;
        co.fillWidth = inRegion;
        h.ctx.BeginContainer(inRegion ? 2 : 1, co);
        h.ctx.ItemSize({100, 20});
        second = Rect::FromSize(h.ctx.CursorPos(), Vec2(50, 20));
        h.ctx.ItemSize({50, 20});
        r = h.ctx.EndContainer();
        h.ctx.End();
        h.End();
    };
    frame(false);
    frame(false);
    ESIA_CHECK(r == Rect(0, 0, 100, 40) && second.min.x == 25.0f);
    frame(true);
    frame(true);
    ESIA_CHECK(r.Width() == 500.0f && second.min.x == 225.0f);
}

// review: a floating region's padding hides what is below it too, and the wheel over a scroll child's padding
// scrolls that child
ESIA_TEST(Child, PaddingIsPartOfTheRegion)
{
    Harness h;
    ButtonResult content;
    float listScroll = 0.0f;
    auto frame = [&] {
        h.Frame();
        WindowOptions o;
        o.padding = {0, 0};
        h.Win("W", {0, 0}, {400, 400}, o);
        content = h.Button("content", Rect(0, 0, 300, 200));
        ChildOptions bar;
        bar.flags = ChildFlags_Floating;
        bar.rect = Rect(0, 0, 300, 60);
        bar.padding = {10, 10};
        h.ctx.BeginChild("bar", bar);
        h.ctx.EndChild();
        h.ctx.SetCursorPos({0, 220});
        ChildOptions list;
        list.size = {200, 100};
        list.padding = {10, 10};
        list.flags = ChildFlags_ScrollY;
        h.ctx.BeginChild("list", list);
        for (int i = 0; i < 20; ++i)
            h.ctx.ItemSize({50, 22});
        listScroll = h.ctx.Scroll().y;
        h.ctx.EndChild();
        h.ctx.End();
        h.End();
    };
    h.Move({150, 5});   // in the bar's padding
    frame();
    frame();
    ESIA_CHECK(!content.hovered);
    h.Move({150, 100});
    frame();
    ESIA_CHECK(content.hovered);
    h.Move({100, 225});   // the list's top padding
    frame();
    h.Wheel(0, -1);
    frame();
    frame();
    ESIA_CHECK(listScroll == 48.0f && h.ctx.FindWindowByName("W")->Scroll().y == 0.0f);
}

// The wheel stays with the area it scrolls while it keeps turning: the window scrolling an inner area in under the
// mouse does not hand it the rest of the turn; after a pause the area under the mouse takes it.
ESIA_TEST(Child, WheelStaysWithTheAreaItScrolls)
{
    Harness h;
    Vec2 innerScroll, outerScroll;
    auto frame = [&](double dt = 1.0 / 60.0) {
        h.Frame(dt);
        WindowOptions o;
        o.padding = {0, 0};
        h.Win("W", {0, 0}, {400, 400}, o);
        ChildOptions outer;
        outer.size = {300, 200};
        outer.flags = ChildFlags_ScrollY;
        h.ctx.BeginChild("outer", outer);
        h.ctx.ItemSize({50, 142});   // the inner area starts at 150
        ChildOptions inner;
        inner.size = {0, 100};
        inner.flags = ChildFlags_ScrollY;
        h.ctx.BeginChild("inner", inner);
        for (int i = 0; i < 10; ++i)
            h.ctx.ItemSize({50, 22});   // 192 to scroll
        innerScroll = h.ctx.Scroll();
        h.ctx.EndChild();
        for (int i = 0; i < 10; ++i)
            h.ctx.ItemSize({50, 22});   // 550 in all: 350 to scroll
        outerScroll = h.ctx.Scroll();
        h.ctx.EndChild();
        h.ctx.End();
        h.End();
    };
    h.Move({50, 50});   // over the outer area, above the inner one
    frame();
    frame();
    for (int notch = 1; notch <= 4; ++notch)   // after the third the inner area (at 6 .. 106) is under the mouse
    {
        h.Wheel(0, -1);
        frame();
        frame();
        ESIA_CHECK(outerScroll.y == 48.0f * (float)notch && innerScroll.y == 0.0f);
    }
    frame(0.5);
    h.Wheel(0, -1);
    frame();
    frame();
    ESIA_CHECK(outerScroll.y == 192.0f && innerScroll.y == 48.0f);
    // moving the mouse ends it at once: over the outer area's own content again (below the inner one)
    h.Wheel(0, -1);
    frame();
    h.Move({50, 150});
    h.Wheel(0, -1);
    frame();
    frame();
    ESIA_CHECK(outerScroll.y == 240.0f && innerScroll.y == 96.0f);
}

// An area half out of its parent: VisibleViewRect is the part still in it (where its scroll indicator may show).
ESIA_TEST(Child, VisibleViewRectStopsAtTheParent)
{
    Harness h;
    h.Frame();
    WindowOptions o;
    o.padding = {0, 0};
    h.Win("W", {0, 0}, {400, 300}, o);
    h.ctx.ItemSize({10, 242});   // the area starts at 250
    ChildOptions c;
    c.size = {200, 100};
    c.flags = ChildFlags_ScrollY;
    h.ctx.BeginChild("area", c);
    ESIA_CHECK(h.ctx.ViewRect() == Rect(0, 250, 200, 350));
    ESIA_CHECK(h.ctx.VisibleViewRect() == Rect(0, 250, 200, 300));
    h.ctx.EndChild();
    ESIA_CHECK(h.ctx.VisibleViewRect() == Rect(0, 0, 400, 300));   // the window: its rect
    h.ctx.End();
    h.End();
}

// LineRoom: an item asking at the start of a line learns how far the SameLine items after it reached last frame
ESIA_TEST(Layout, LineRoomOfSameLineItems)
{
    Harness h;
    float room[4] = {};
    float lead = 0.0f;
    auto frame = [&] {
        h.Frame();
        WindowOptions o;
        o.padding = {0, 0};
        h.Win("W", {0, 0}, {400, 300}, o);
        h.ctx.ItemSize({100, 20});           // a line of its own
        room[0] = h.ctx.LineRoom();          // a line of three: something filling, then two of 60 and 40
        lead = 400.0f - room[0];
        h.ctx.ItemSize({lead, 20});
        h.ctx.SameLine();
        room[1] = h.ctx.LineRoom();
        h.ctx.ItemSize({60, 20});
        h.ctx.SameLine();
        room[2] = h.ctx.LineRoom();
        h.ctx.ItemSize({40, 20});
        room[3] = h.ctx.LineRoom();          // the next line: nothing after it
        h.ctx.ItemSize({100, 20});
        h.ctx.End();
        h.End();
    };
    frame();
    ESIA_CHECK(room[0] == 0.0f);   // the first frame knows nothing yet
    frame();
    ESIA_CHECK(room[0] == 116.0f && room[1] == 48.0f && room[2] == 0.0f && room[3] == 0.0f);   // 8 + 60 + 8 + 40
    frame();
    ESIA_CHECK(room[0] == 116.0f && lead == 284.0f);   // the same every frame: the line ends at the window's edge
}

// Groups in an auto-layout container, whose children are on no line of the cursor, keep their lines' room to
// themselves: a group starting a line does not get the room of another group's SameLine items, nor of the row it
// places elsewhere with SetCursorPos (the showcase's Control Center modules shrank by their media buttons)
ESIA_TEST(Layout, LineRoomKeepsToItsGroup)
{
    Harness h;
    StackLayout stack;   // a column
    float second = -1.0f, first = -1.0f, third = -1.0f;
    auto frame = [&] {
        h.Frame();
        h.Win("W", {0, 0}, {400, 300});
        ContainerOptions co;
        co.layout = &stack;
        h.ctx.BeginContainer(h.ctx.GetId("stack"), co);
        h.ctx.BeginGroup();
        first = h.ctx.LineRoom();
        h.ctx.ItemSize({100, 20});
        h.ctx.SameLine();
        h.ctx.ItemSize({60, 20});
        h.ctx.EndGroup();
        h.ctx.BeginGroup();
        second = h.ctx.LineRoom();
        h.ctx.ItemSize({100, 20});
        h.ctx.EndGroup();
        h.ctx.BeginGroup();   // a module: sized with what is available, its buttons placed in a row elsewhere
        third = h.ctx.LineRoom();
        h.ctx.SetCursorPos(h.ctx.CursorPos() + Vec2(10, 40));
        h.ctx.ItemSize({30, 20});
        h.ctx.SameLine();
        h.ctx.ItemSize({30, 20});
        h.ctx.EndGroup();
        h.ctx.EndContainer();
        h.ctx.End();
        h.End();
    };
    frame();
    frame();
    ESIA_CHECK(first == 68.0f && second == 0.0f && third == 0.0f);   // 8 + 60 after the first group's first item
}
