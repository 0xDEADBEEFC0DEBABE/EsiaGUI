// The data widgets without a GPU: number fields, tables, trees, the multi-line editor, the color picker's hex field and
// docking, on a Context driven frame by frame with queued input (no text system: text measures as empty).
#include "esia/ui/ui.hpp"
#include "esia_test.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

using namespace esia;

namespace
{
#if defined(__APPLE__)
    constexpr std::uint32_t kShortcut = Mod_Super;
#else
    constexpr std::uint32_t kShortcut = Mod_Ctrl;
#endif

    // Frames of `body` inside a window at (0, 0) of 600 x 400 without padding; input helpers between them.
    struct Harness
    {
        std::string clipboard;
        Context ctx;
        ui::Ui ui{ctx, {}};
        double t = 1.0;
        std::function<void()> body;

        Harness() : ctx(Desc(clipboard)) {}
        static ContextDesc Desc(std::string& clip)
        {
            ContextDesc d;
            d.getClipboard = [&clip] { return clip; };
            d.setClipboard = [&clip](const std::string& v) { clip = v; };
            return d;
        }
        void Step()
        {
            t += 1.0 / 60.0;
            ctx.NewFrame({{800, 600}, {1, 1}, t});
            ui.NewFrame();
            ctx.SetNextWindowPos({0, 0}, Cond::FirstUse);
            ctx.SetNextWindowSize({600, 400}, Cond::FirstUse);
            WindowOptions wo;
            wo.padding = {0, 0};
            ctx.Begin("W", wo);
            body();
            ctx.End();
            ui.EndFrame();
            ctx.EndFrame();
        }
        void Move(Vec2 at)
        {
            ctx.QueueInput(InputEvent::MouseMove(at));
            Step();
        }
        void Down(std::uint32_t mods = 0)
        {
            if (mods)
                ctx.QueueInput(InputEvent::KeyEvent(mods & Mod_Shift ? Key::LeftShift : Key::LeftCtrl, true, mods));
            ctx.QueueInput(InputEvent::Button(MouseButton::Left, true));
            Step();
        }
        void Up(std::uint32_t mods = 0)
        {
            ctx.QueueInput(InputEvent::Button(MouseButton::Left, false));
            Step();
            if (mods)
            {
                ctx.QueueInput(InputEvent::KeyEvent(mods & Mod_Shift ? Key::LeftShift : Key::LeftCtrl, false, 0));
                Step();
            }
        }
        void Click(Vec2 at, std::uint32_t mods = 0)
        {
            Move(at);
            Down(mods);
            Up(mods);
        }
        void Key(esia::Key k, std::uint32_t mods = 0)
        {
            ctx.QueueInput(InputEvent::KeyEvent(k, true, mods));
            Step();
            ctx.QueueInput(InputEvent::KeyEvent(k, false, mods));
            Step();
        }
        void Type(const char* text)
        {
            ctx.QueueInput(InputEvent::TextEvent(text));
            Step();
        }
    };
}

ESIA_TEST(UiNumberField, DragScrubsAndClickTypesAnExpression)
{
    Harness h;
    double v = 1.0;
    int n = 3;
    bool changed = false;
    h.body = [&] {
        changed = ui::NumberField("v", &v, {.max = 20.0}) || changed;   // (0, 0) 600 x 32
        ui::NumberField("n", &n, {.min = 0, .max = 5, .buttons = true});   // (0, 32 + spacing) 600 x 32
    };
    h.Step();
    // a drag to the right raises the value
    h.Move({300, 16});
    h.Down();
    h.Move({360, 16});
    h.Up();
    ESIA_CHECK(changed && v > 1.5);

    // a click without a drag types: an expression, taken with Enter
    h.Click({300, 16});
    h.Type("2*(3+4)");
    h.Key(Key::Enter);
    ESIA_CHECK(v == 14.0);
    // past the limit: clamped; Escape keeps the old value
    h.Click({300, 16});
    h.Type("99");
    h.Key(Key::Enter);
    ESIA_CHECK(v == 20.0);
    h.Click({300, 16});
    h.Type("5");
    h.Key(Key::Escape);
    ESIA_CHECK(v == 20.0);

    // the + and - buttons step an integer within its range
    const float y = 32.0f + h.ctx.Metrics().itemSpacing.y + 16.0f;
    h.Click({600 - 16, y});
    h.Click({600 - 16, y});
    h.Click({600 - 16, y});
    ESIA_CHECK(n == 5);
    h.Click({16, y});
    ESIA_CHECK(n == 4);
}

ESIA_TEST(UiTable, VisibleRowsSelectionAndSort)
{
    Harness h;
    std::vector<int> selection;
    ui::TableRange range;
    ui::TableSort sort;
    int clicked = -1;
    h.body = [&] {
        ui::TableOptions o;
        o.flags = ui::TableFlags_Sortable | ui::TableFlags_Selectable;
        o.height = 300.0f;      // header 30, rows of 34 under it
        o.selection = &selection;
        if (ui::BeginTable("t", {{"Name"}, {"Size", 100}}, o))
        {
            sort = ui::TableSortSpec();
            range = ui::TableVisible(100000);
            for (int i = range.first; i < range.last; ++i)
            {
                if (ui::TableRow(i).clicked)
                    clicked = i;
                ui::TableCell("name");
                ui::TableCell("size");
            }
            ui::EndTable();
        }
    };
    h.Step();
    h.Step();
    // only the rows in view are asked for: 270 / 34 of them, and one more
    ESIA_CHECK(range.first == 0 && range.last >= 8 && range.last <= 10);

    // a click selects a row, Shift extends to another, Ctrl adds and removes
    h.Click({200, 30 + 34 * 2 + 10});
    ESIA_CHECK(clicked == 2 && selection == std::vector<int>({2}));
    h.Click({200, 30 + 34 * 5 + 10}, Mod_Shift);
    ESIA_CHECK(selection == std::vector<int>({2, 3, 4, 5}));
    h.Click({200, 30 + 34 * 3 + 10}, Mod_Ctrl);
    ESIA_CHECK(selection == std::vector<int>({2, 4, 5}));

    // a header click sorts by it, another reverses it
    h.Click({100, 15});
    ESIA_CHECK(sort.column == 0 && sort.ascending);
    h.Click({100, 15});
    ESIA_CHECK(sort.column == 0 && !sort.ascending);
}

// Two tables that scroll in one window scroll on their own: the wheel over the first moves its rows (their scroll
// state was shared, and the second table's few rows left nothing to scroll: the window took the wheel)
ESIA_TEST(UiTable, TwoTablesScrollOnTheirOwn)
{
    Harness h;
    ui::TableRange a, b;
    h.body = [&] {
        if (ui::BeginTable("a", {{"Name"}}, {.height = 150.0f}))
        {
            a = ui::TableVisible(40);
            for (int i = a.first; i < a.last; ++i)
            {
                ui::TableRow(i);
                ui::TableCell("row");
            }
            ui::EndTable();
        }
        if (ui::BeginTable("b", {{"Name"}}, {.height = 150.0f}))
        {
            b = ui::TableVisible(2);
            for (int i = b.first; i < b.last; ++i)
            {
                ui::TableRow(i);
                ui::TableCell("row");
            }
            ui::EndTable();
        }
        h.ctx.ItemSize({10, 400});   // the window scrolls too
    };
    h.Step();
    h.Move({200, 90});   // over the first table's rows
    h.ctx.QueueInput(InputEvent::Wheel(0, -2));
    for (int i = 0; i < 40; ++i)
        h.Step();
    ESIA_CHECK(a.first >= 2 && b.first == 0);
    ESIA_CHECK(h.ctx.FindWindowByName("W")->Scroll().y == 0.0f);
}

// maxHeight: as tall as the rows, up to it (then they scroll), in the frame the row count changes
ESIA_TEST(UiTable, MaxHeightFitsTheRows)
{
    Harness h;
    int rows = 2;
    Rect table;
    float below = 0.0f;
    h.body = [&] {
        if (ui::BeginTable("t", {{"Name"}}, {.maxHeight = 200.0f}))
        {
            const ui::TableRange r = ui::TableVisible(rows);
            for (int i = r.first; i < r.last; ++i)
            {
                ui::TableRow(i);
                ui::TableCell("row");
            }
            ui::EndTable();
            table = h.ctx.LastItemRect();
        }
        below = h.ctx.CursorPos().y;
    };
    h.Step();
    ESIA_CHECK(table.Height() == 30.0f + 2 * 34.0f && below == table.max.y + h.ctx.Metrics().itemSpacing.y);
    rows = 40;
    h.Step();
    ESIA_CHECK(table.Height() == 200.0f);
    rows = 0;
    h.Step();
    ESIA_CHECK(table.Height() == 30.0f);   // the header
}

// The selection kept as keys follows its rows when they move; Shift runs from where the anchor's row is now
ESIA_TEST(UiTable, SelectionByKeyFollowsTheRows)
{
    Harness h;
    std::vector<int> data = {10, 20, 30, 40, 50};
    std::vector<std::uint64_t> keys;
    bool selected[5] = {};
    h.body = [&] {
        ui::TableOptions o;
        o.flags = ui::TableFlags_Selectable;
        o.height = 300.0f;
        o.selectedKeys = &keys;
        o.rowKey = [&](int i) { return (std::uint64_t)data[(std::size_t)i]; };
        if (ui::BeginTable("t", {{"Name"}}, o))
        {
            const ui::TableRange r = ui::TableVisible((int)data.size());
            for (int i = r.first; i < r.last; ++i)
            {
                selected[i] = ui::TableRow(i).selected;
                ui::TableCell("row");
            }
            ui::EndTable();
        }
    };
    h.Step();
    h.Click({200, 30 + 34 * 1 + 10});   // row 1: 20
    ESIA_CHECK(keys == std::vector<std::uint64_t>({20}));
    std::reverse(data.begin(), data.end());   // re-sorted: 20 is row 3 now
    h.Step();
    ESIA_CHECK(selected[3] && !selected[1]);
    h.Click({200, 30 + 10}, Mod_Shift);       // to row 0: rows 0 .. 3 in their order now
    ESIA_CHECK(keys == std::vector<std::uint64_t>({20, 30, 40, 50}));
    h.Click({200, 30 + 34 * 1 + 10}, Mod_Ctrl);   // row 1 (40) out
    ESIA_CHECK(keys == std::vector<std::uint64_t>({20, 30, 50}));
    ESIA_CHECK(selected[0] && !selected[1] && selected[2] && selected[3] && !selected[4]);
}

// A table its window scrolled half out of view shows (and hits) its scroll indicator only inside the window
ESIA_TEST(UiTable, TheIndicatorStaysInsideTheWindow)
{
    Harness h;
    h.body = [&] {
        h.ctx.ItemSize({10, 300});   // the table at 308 .. 508: the window ends at 400
        if (ui::BeginTable("t", {{"Name"}}, {.height = 200.0f}))
        {
            const ui::TableRange r = ui::TableVisible(40);
            for (int i = r.first; i < r.last; ++i)
            {
                ui::TableRow(i);
                ui::TableCell("row");
            }
            ui::EndTable();
        }
    };
    for (int i = 0; i < 4; ++i)
        h.Step();   // it appears with rows to scroll: its indicator flashes
    int lanes = 0;
    for (const DrawCmd& cmd : h.ctx.FindWindowByName("W")->GetDrawList().Commands())
        if (std::fabs(cmd.clip.Width() - 12.0f) < 0.01f)
        {
            ++lanes;
            ESIA_CHECK(cmd.clip.min.y >= 338.0f && cmd.clip.max.y <= 400.0f);
        }
    ESIA_CHECK(lanes > 0);
}

// A field that fills the width leaves the items after it on its line (SameLine) their room
ESIA_TEST(UiLayout, AFieldThatFillsLeavesRoomForTheItemsAfterIt)
{
    Harness h;
    double v = 1.0;
    float vec[3] = {};
    Rect set, apply;
    h.body = [&] {
        ui::NumberField("v", &v);
        h.ctx.SameLine();
        set = ui::Interact("set", {80, 32}).rect;
        ui::VectorField("p", vec, 3);
        h.ctx.SameLine();
        ui::Interact("use", {60, 32});
        h.ctx.SameLine();
        apply = ui::Interact("apply", {70, 32}).rect;
    };
    h.Step();
    h.Step();
    ESIA_CHECK(set.max.x == 600.0f && set.min.x == 520.0f);
    ESIA_CHECK(apply.max.x == 600.0f);
    h.Step();
    ESIA_CHECK(set.max.x == 600.0f && apply.max.x == 600.0f);   // and it stays so
}

ESIA_TEST(UiTree, OpensClosesAndLeaves)
{
    Harness h;
    bool open = false, leafOpen = true, childSeen = false;
    ui::TreeNodeResult arrowOnly;
    h.body = [&] {
        childSeen = false;
        const ui::TreeNodeResult r = ui::TreeNode("parent");                         // (0, 0) 600 x 30
        open = r.open;
        if (r.open)
        {
            ui::TreeNode("child", {.flags = ui::TreeFlags_Leaf});
            childSeen = true;
            ui::TreePop();
        }
        leafOpen = ui::TreeNode("leaf", {.flags = ui::TreeFlags_Leaf}).open;
        arrowOnly = ui::TreeNode("arrow", {.flags = ui::TreeFlags_OpenOnArrow});
        if (arrowOnly.open)
            ui::TreePop();
    };
    h.Step();
    ESIA_CHECK(!open && !childSeen && !leafOpen);
    h.Click({300, 15});   // the row: opens it
    for (int i = 0; i < 30; ++i)
        h.Step();
    ESIA_CHECK(open && childSeen);
    h.Click({300, 15});   // closes it: the children slide away, then are no longer submitted
    for (int i = 0; i < 60; ++i)
        h.Step();
    ESIA_CHECK(!open && !childSeen);
    // OpenOnArrow: a click on the row only reports the click
    const float y = 2 * (30.0f + h.ctx.Metrics().itemSpacing.y) + 15.0f;
    h.Click({300, y});
    ESIA_CHECK(arrowOnly.clicked || !arrowOnly.open);
    for (int i = 0; i < 30; ++i)
        h.Step();
    ESIA_CHECK(!arrowOnly.open);
    h.Click({11, y});   // the arrow
    for (int i = 0; i < 30; ++i)
        h.Step();
    ESIA_CHECK(arrowOnly.open);
}

ESIA_TEST(UiTextEditor, LinesNewlinesAndPaste)
{
    Harness h;
    std::string text;
    h.body = [&] { ui::TextEditor("e", &text, {.size = Vec2(500, 200)}); };
    h.Step();
    h.Click({200, 100});
    h.Type("ab");
    h.Key(Key::Enter);
    h.Type("cd");
    ESIA_CHECK(text == "ab\ncd");
    // Up goes to the first line, Home to its start; Backspace at the second line's start joins the lines
    h.Key(Key::Up);
    h.Key(Key::Home);
    h.Type("X");
    ESIA_CHECK(text == "Xab\ncd");
    h.Key(Key::Down);
    h.Key(Key::Home);
    h.Key(Key::Backspace);
    ESIA_CHECK(text == "Xabcd");
    // a paste keeps its lines (CR LF becomes LF); undo takes it back
    h.clipboard = "1\r\n2";
    h.Key(Key::End);
    h.Key(Key::V, kShortcut);
    ESIA_CHECK(text == "Xabcd1\n2");
    h.Key(Key::Z, kShortcut);
    ESIA_CHECK(text == "Xabcd");
}

ESIA_TEST(UiColorPicker, HexFieldSetsTheColor)
{
    Harness h;
    Color c = Color::Hex(0x0A84FF);
    bool changed = false;
    h.body = [&] { changed = ui::ColorPicker("c", &c, {.alpha = false, .width = 300}) || changed; };
    h.Step();
    // the hex field: right of the 54-wide swatch, under the spectrum (300 x 186), the hue bar and their gaps
    const float y = 186.0f + 14.0f + 26.0f + 14.0f * 0.75f + 17.0f;
    h.Click({200, y});
    h.Type("#ff3b30");
    h.Key(Key::Enter);
    ESIA_CHECK(changed && c == Color::Hex(0xFF3B30));
}

namespace
{
    // Frames with a dock space over the display and windows submitted after it.
    struct DockHarness
    {
        Context ctx;
        ui::Ui ui{ctx, {}};
        double t = 1.0;
        std::vector<std::string> windows;
        std::vector<std::string> shown;
        std::vector<std::string> docked;   // IsWindowDocked at the end of the last frame (it needs a current Ui)

        void Step()
        {
            t += 1.0 / 60.0;
            ctx.NewFrame({{1000, 700}, {1, 1}, t});
            ui.NewFrame();
            ui::DockSpace("space");
            shown.clear();
            for (const std::string& w : windows)
                if (ui::BeginWindow(w))
                {
                    shown.push_back(w);
                    ui::EndWindow();
                }
            docked.clear();
            for (const std::string& w : windows)
                if (ui::IsWindowDocked(w))
                    docked.push_back(w);
            ui.EndFrame();
            ctx.EndFrame();
        }
    };
}

ESIA_TEST(UiDock, LayoutTabsAndRoundTrip)
{
    DockHarness h;
    h.windows = {"A", "B", "C", "D", "Free"};
    h.Step();
    ui::Ui* saved = ui::Current();
    (void)saved;
    // the layout is built between frames of the Ui: inside one, as an application does on its first frame
    h.t += 1.0 / 60.0;
    h.ctx.NewFrame({{1000, 700}, {1, 1}, h.t});
    h.ui.NewFrame();
    ui::DockWindow("A", "space");
    ui::DockWindow("B", "space", ui::DockSide::Left, 0.25f, "A");
    ui::DockWindow("C", "space", ui::DockSide::Bottom, 0.3f, "A");
    ui::DockWindow("D", "space", ui::DockSide::Center, 0.5f, "C");   // a tab beside C, and the one shown
    ESIA_CHECK(ui::IsWindowDocked("A") && ui::IsWindowDocked("D") && !ui::IsWindowDocked("Free"));
    const std::string layout = ui::SaveDockLayout("space");
    h.ui.EndFrame();
    h.ctx.EndFrame();

    h.Step();
    // C and D share a node: D's tab is in front
    ESIA_CHECK(std::find(h.shown.begin(), h.shown.end(), "D") != h.shown.end());
    ESIA_CHECK(std::find(h.shown.begin(), h.shown.end(), "C") == h.shown.end());
    ESIA_CHECK(std::find(h.shown.begin(), h.shown.end(), "Free") != h.shown.end());
    // B on the left: its window is left of A's, and A's above C / D's
    const Window* a = h.ctx.FindWindowByName("A");
    const Window* b = h.ctx.FindWindowByName("B");
    const Window* d = h.ctx.FindWindowByName("D");
    ESIA_CHECK(a && b && d && b->GetRect().max.x <= a->GetRect().min.x && a->GetRect().max.y <= d->GetRect().min.y);
    ESIA_CHECK(b->GetRect().Width() > 200.0f && b->GetRect().Width() < 300.0f);   // a quarter of 1000, less the gap

    // undocked, a window floats; the layout reads back as it was saved
    h.t += 1.0 / 60.0;
    h.ctx.NewFrame({{1000, 700}, {1, 1}, h.t});
    h.ui.NewFrame();
    ui::UndockWindow("B");
    ESIA_CHECK(!ui::IsWindowDocked("B"));
    ESIA_CHECK(ui::SaveDockLayout("space") != layout);
    ESIA_CHECK(ui::LoadDockLayout("space", layout));
    ESIA_CHECK(ui::SaveDockLayout("space") == layout && ui::IsWindowDocked("B"));
    ESIA_CHECK(!ui::LoadDockLayout("space", "not a layout"));
    h.ui.EndFrame();
    h.ctx.EndFrame();
}

ESIA_TEST(UiDock, ASpaceWithoutARectIsTheSafeArea)
{
    // a phone's display: the dock space keeps clear of the camera housing and the home indicator
    Context ctx;
    ui::Ui u{ctx, {}};
    const Rect safe(0, 62, 440, 922);
    for (int frame = 0; frame < 3; ++frame)
    {
        ctx.NewFrame({{440, 956}, {3, 3}, 1.0 + frame / 60.0, safe});
        u.NewFrame();
        ui::DockSpace("phone");
        if (frame == 0)
            ui::DockWindow("Panel", "phone");
        if (ui::BeginWindow("Panel"))
            ui::EndWindow();
        u.EndFrame();
        ctx.EndFrame();
    }
    const Window* w = ctx.FindWindowByName("Panel");
    ESIA_CHECK(w && w->GetRect().min.y >= safe.min.y && w->GetRect().max.y <= safe.max.y && w->GetRect().Height() > 700.0f);
}

ESIA_TEST(UiDock, DragToDockAndTabOut)
{
    DockHarness h;
    h.windows = {"A", "B", "Free"};
    h.Step();
    h.t += 1.0 / 60.0;
    h.ctx.NewFrame({{1000, 700}, {1, 1}, h.t});
    h.ui.NewFrame();
    ui::DockWindow("A", "space");
    ui::DockWindow("B", "space", ui::DockSide::Right, 0.4f, "A");
    h.ui.EndFrame();
    h.ctx.EndFrame();
    h.Step();
    h.Step();
    const Rect free = h.ctx.FindWindowByName("Free")->GetRect();
    const Rect a = h.ctx.FindWindowByName("A")->GetRect();
    const auto move = [&](Vec2 p) {
        h.ctx.QueueInput(InputEvent::MouseMove(p));
        h.Step();
    };

    // the floating window by its header, over the middle of A's node, let go: a tab beside A, and the one shown
    move(free.min + Vec2(80, 18));
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, true));
    h.Step();
    for (int i = 1; i <= 8; ++i)
        move(Lerp(free.min + Vec2(80, 18), a.Center(), (float)i / 8.0f));
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false));
    h.Step();
    h.Step();
    h.Step();
    ESIA_CHECK(std::find(h.docked.begin(), h.docked.end(), "Free") != h.docked.end());
    ESIA_CHECK(std::find(h.shown.begin(), h.shown.end(), "Free") != h.shown.end());
    ESIA_CHECK(std::find(h.shown.begin(), h.shown.end(), "A") == h.shown.end());
    const Rect docked = h.ctx.FindWindowByName("Free")->GetRect();
    ESIA_CHECK(std::fabs(docked.min.x - a.min.x) < 1.0f && std::fabs(docked.Width() - a.Width()) < 1.0f);

    // its tab (the second: tabs of 190 from 8 in, 4 apart) dragged down out of the bar: it floats and goes on moving
    // with the mouse
    const Vec2 tab = docked.min + Vec2(8 + 190 + 4 + 60, 26);
    move(tab);
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, true));
    h.Step();
    for (int i = 1; i <= 6; ++i)
        move(tab + Vec2(10.0f * (float)i, 25.0f * (float)i));
    h.Step();
    ESIA_CHECK(std::find(h.docked.begin(), h.docked.end(), "Free") == h.docked.end());
    move(tab + Vec2(200, 300));
    const Rect floating = h.ctx.FindWindowByName("Free")->GetRect();
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false));
    h.Step();
    ESIA_CHECK(floating.Contains(tab + Vec2(200, 300)));
    ESIA_CHECK(std::find(h.shown.begin(), h.shown.end(), "A") != h.shown.end());   // A's tab is in front again
}
