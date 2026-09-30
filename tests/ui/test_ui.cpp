// The widget layer without a GPU: springs, themes, styles, auto layout and interaction, on a Context driven frame by
// frame (no text system: text measures as empty, the layout and the logic are what is tested).
#include "esia/ui/ui.hpp"
#include "esia_test.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

using namespace esia;

namespace
{
    struct UiHarness
    {
        Context ctx;
        ui::Ui ui{ctx, {}};
        double t = 1.0;

        // One frame: `body` runs inside a window at (0, 0) of 600 x 400 without padding.
        template <class F>
        void Frame(F&& body, double dt = 1.0 / 60.0)
        {
            t += dt;
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
    };

    // An item of `size` whose rect is recorded (Interaction::rect)
    Rect Box(const char* id, Vec2 size) { return ui::Interact(id, size).rect; }
}

ESIA_TEST(UiSpring, SettlesAndIsFrameRateIndependent)
{
    const ui::Spring s{0.35f, 0.86f};
    ui::SpringState a, b;
    for (int i = 0; i < 60; ++i)
        a.Step(100.0f, s, 1.0f / 60.0f);
    for (int i = 0; i < 240; ++i)
        b.Step(100.0f, s, 1.0f / 240.0f);
    ESIA_CHECK(std::fabs(a.value - b.value) < 0.01f);   // analytic: the same curve at any frame rate
    for (int i = 0; i < 600; ++i)
        a.Step(100.0f, s, 1.0f / 60.0f);
    ESIA_CHECK(a.value == 100.0f && a.velocity == 0.0f);   // comes to rest exactly
    // an under-damped spring overshoots, a critically damped one does not
    ui::SpringState bouncy, smooth;
    float maxB = 0.0f, maxS = 0.0f;
    for (int i = 0; i < 120; ++i)
    {
        bouncy.Step(1.0f, ui::Spring::Bouncy(), 1.0f / 60.0f);
        smooth.Step(1.0f, ui::Spring::Smooth(), 1.0f / 60.0f);
        maxB = std::max(maxB, bouncy.value);
        maxS = std::max(maxS, smooth.value);
    }
    ESIA_CHECK(maxB > 1.02f && maxS <= 1.0f);
}

ESIA_TEST(UiTheme, LerpAndAnimator)
{
    const ui::Theme light = ui::ThemeLight(), dark = ui::ThemeDark();
    ESIA_CHECK(ui::LerpTheme(light, dark, 0.0f).colors.label == light.colors.label);
    ESIA_CHECK(ui::LerpTheme(light, dark, 1.0f).colors.label == dark.colors.label);
    const ui::Theme mid = ui::LerpTheme(light, dark, 0.5f);
    ESIA_CHECK(std::fabs(mid.colors.label.r - 0.5f) < 1e-4f && mid.dark);   // `dark` switches at the middle
    ESIA_CHECK(std::fabs(mid.materials.window.blur - light.materials.window.blur) < 1e-4f);
    ESIA_CHECK(ui::ThemeWithAccent(light, Color::Hex(0x30D158)).colors.accent == Color::Hex(0x30D158));
    ESIA_CHECK(ui::ThemeWithAccent(light, Color::Clear()).colors.accent == light.colors.accent);

    ui::ThemeAnimator a;
    a.Set(light, false);
    a.Set(dark, true);
    ESIA_CHECK(a.Animating() && a.current.colors.label == light.colors.label);
    a.Step(0.1f);
    ESIA_CHECK(a.current.colors.label.r > 0.0f && a.current.colors.label.r < 1.0f);
    a.Step(1.0f);
    ESIA_CHECK(!a.Animating() && a.current.colors.label == dark.colors.label);
}

ESIA_TEST(UiStyle, NextStylesOneWidgetScopesNest)
{
    UiHarness h;
    Color seen[3];
    h.Frame([&] {
        ui::Next().Tint(Color::Hex(0xFF0000));
        const ui::Interaction it = ui::Interact("a", {10, 10});
        seen[0] = it.style.Has(ui::ItemStyle::kTint) ? it.style.tint : Color::Clear();
        seen[1] = ui::AccentColor();   // Next() was taken: back to the theme's
        {
            ui::StyleScope outer(ui::ItemStyle().Tint(Color::Hex(0x00FF00)).Opacity(0.5f));
            ui::StyleScope inner(ui::ItemStyle().Radius(4));
            seen[2] = ui::AccentColor();   // the outer tint shows through the inner scope
            ESIA_CHECK(ui::CurrentItemStyle().Has(ui::ItemStyle::kRadius));
        }
        ESIA_CHECK(!ui::CurrentItemStyle().Has(ui::ItemStyle::kTint));
    });
    ESIA_CHECK(seen[0] == Color::Hex(0xFF0000));
    ESIA_CHECK(seen[1] == ui::ThemeLight().colors.accent);
    ESIA_CHECK(seen[2] == Color::Hex(0x00FF00));
}

ESIA_TEST(UiLayout, StacksPlaceTheirChildren)
{
    UiHarness h;
    Rect a, b, c;
    auto frame = [&] {
        h.Frame([&] {
            ui::BeginHStack("row", {.spacing = 10, .align = ui::Align::Center});
            a = Box("a", {50, 20});
            b = Box("b", {30, 40});
            ui::EndStack();
            ui::BeginVStack("col", {.spacing = 5, .align = ui::Align::Start});
            c = Box("c", {70, 10});
            ui::EndStack();
        });
    };
    for (int i = 0; i < 3; ++i)   // the first frame measures, the next places
        frame();
    ESIA_CHECK(a.min.x == 0.0f && b.min.x == 60.0f);
    ESIA_CHECK(a.min.y == 10.0f && b.min.y == 0.0f);   // centered across the row: (40 - 20) / 2
    ESIA_CHECK(c.min.y >= 40.0f && c.min.x == 0.0f);
}

ESIA_TEST(UiLayout, FlexSpacerTakesTheFreeRoom)
{
    UiHarness h;
    Rect a, b;
    for (int i = 0; i < 3; ++i)
        h.Frame([&] {
            ui::BeginHStack("row", {.spacing = 0});
            a = Box("a", {100, 20});
            ui::FlexSpacer();
            b = Box("b", {100, 20});
            ui::EndStack();
        });
    ESIA_CHECK(a.min.x == 0.0f && b.max.x == 600.0f);
}

ESIA_TEST(UiLayout, FlowWrapsAndGridColumns)
{
    UiHarness h;
    std::vector<Rect> flow(5), grid(5);
    for (int i = 0; i < 3; ++i)
        h.Frame([&] {
            ui::BeginFlow("flow", {.spacing = 20, .lineSpacing = 10});
            for (int k = 0; k < 5; ++k)
                flow[(std::size_t)k] = Box(k == 0 ? "f0" : k == 1 ? "f1" : k == 2 ? "f2" : k == 3 ? "f3" : "f4", {180, 30});
            ui::EndFlow();
            ui::BeginGrid("grid", {.minColumnWidth = 150, .spacing = 10});
            for (int k = 0; k < 5; ++k)
                grid[(std::size_t)k] = Box(k == 0 ? "g0" : k == 1 ? "g1" : k == 2 ? "g2" : k == 3 ? "g3" : "g4", {100, 20});
            ui::EndGrid();
        });
    // 600 wide: three 180-wide items and two gaps of 20 fit (580), the fourth wraps
    ESIA_CHECK(flow[2].min.y == flow[0].min.y && flow[3].min.y == flow[0].min.y + 40.0f && flow[3].min.x == 0.0f);
    // columns: floor((600 + 10) / (150 + 10)) = 3, cells (600 - 20) / 3 = 193.3 wide; items land on whole pixels
    ESIA_CHECK(grid[1].min.x - grid[0].min.x == 203.0f && grid[2].min.x - grid[0].min.x == 407.0f);
    ESIA_CHECK(grid[3].min.x == grid[0].min.x && grid[3].min.y > grid[0].min.y);
}

ESIA_TEST(UiControls, ButtonClicksAndToggleFlips)
{
    UiHarness h;
    bool on = false;
    int clicks = 0;
    auto frame = [&] {
        h.Frame([&] {
            if (ui::Interact("button", {100, 40}).pressed)
                ++clicks;
            ui::Toggle("switch", &on);
        });
    };
    frame();
    h.ctx.QueueInput(InputEvent::MouseMove({50, 20}));
    frame();
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, true));
    frame();
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false));
    frame();
    ESIA_CHECK(clicks == 1);
    // the toggle is below the button (the window's layout cursor)
    h.ctx.QueueInput(InputEvent::MouseMove({25, 40 + 8 + 15}));
    frame();
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, true));
    frame();
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false));
    frame();
    ESIA_CHECK(on);
}

ESIA_TEST(UiAnim, SpringsStepOncePerFrameAndReportMotion)
{
    UiHarness h;
    float v1 = 0.0f, v2 = 0.0f;
    bool moving = false;
    h.Frame([&] { v1 = ui::Anim(42, 0.0f, ui::Spring::Smooth()); });
    h.Frame([&] {
        v1 = ui::Anim(42, 1.0f, ui::Spring::Smooth());
        v2 = ui::Anim(42, 1.0f, ui::Spring::Smooth());   // the same frame: no second step
        moving = h.ui.Animating();
    });
    ESIA_CHECK(v1 > 0.0f && v1 < 1.0f && v1 == v2 && moving);
    for (int i = 0; i < 200; ++i)
        h.Frame([&] { v1 = ui::Anim(42, 1.0f, ui::Spring::Smooth()); });
    ESIA_CHECK(v1 == 1.0f && !h.ui.Animating());
}

ESIA_TEST(UiLists, RowsAbutInTheirSectionAndSectionsStack)
{
    UiHarness h;
    std::vector<Rect> rows;
    h.ctx.SetItemObserver([&](const LaidOutItem& it) {
        if (it.depth == 2 && it.rect.Height() == 44.0f)   // in the section's card (not header / footer text)
            rows.push_back(it.rect);
    });
    bool a = false, b = true;
    float v = 0.5f;
    for (int i = 0; i < 2; ++i)
    {
        rows.clear();
        h.Frame([&] {
            ui::BeginSection();
            ui::RowToggle("A", &a);
            ui::RowValue("B", "value");
            ui::RowSlider("C", &v, 0.0f, 1.0f);
            ui::EndSection();
            ui::BeginSection("Header", "Footer");
            ui::RowToggle("D", &b);
            ui::EndSection();
        });
    }
    ESIA_CHECK(rows.size() == 4);
    if (rows.size() == 4)
    {
        ESIA_CHECK(rows[0].Width() == 600.0f && rows[0].Height() == 44.0f);   // the window's width, the theme's row height
        ESIA_CHECK(rows[0].min.y == 0.0f && rows[1].min.y == rows[0].max.y && rows[2].min.y == rows[1].max.y);   // no gaps
        ESIA_CHECK(rows[3].min.y > rows[2].max.y + 22.0f);   // the section spacing, the header, then the next card
    }
}

ESIA_TEST(UiLists, RowControlsReact)
{
    UiHarness h;
    bool on = false;
    int pressed = 0;
    auto frame = [&] {
        h.Frame([&] {
            ui::BeginSection();
            if (ui::RowButton("Button"))   // y 0 .. 44
                ++pressed;
            ui::RowToggle("Toggle", &on);  // y 44 .. 88, the switch at the right: 600 - 16 - 50
            ui::EndSection();
        });
    };
    auto click = [&](Vec2 at) {
        h.ctx.QueueInput(InputEvent::MouseMove(at));
        frame();
        h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, true));
        frame();
        h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false));
        frame();
    };
    frame();
    click({300, 22});
    ESIA_CHECK(pressed == 1 && !on);
    click({300, 66});   // the row, not its switch
    ESIA_CHECK(!on);
    click({559, 66});
    ESIA_CHECK(on && pressed == 1);
}

ESIA_TEST(UiSegmented, TapAndDragLandOnASegment)
{
    UiHarness h;
    int sel = 0, changes = 0;
    auto frame = [&] {
        h.Frame([&] {
            if (ui::Segmented("seg", &sel, {"One", "Two", "Three"}, 300))   // (0, 0) .. (300, 32), 100 per segment
                ++changes;
        });
    };
    frame();
    h.ctx.QueueInput(InputEvent::MouseMove({250, 16}));
    frame();
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, true));
    frame();
    ESIA_CHECK(sel == 0);   // it lands on release
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false));
    frame();
    ESIA_CHECK(sel == 2 && changes == 1);

    // grab the selection and drag it to the first segment
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, true));
    frame();
    for (float x = 240.0f; x >= 40.0f; x -= 40.0f)
    {
        h.ctx.QueueInput(InputEvent::MouseMove({x, 16}));
        frame();
    }
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false));
    frame();
    ESIA_CHECK(sel == 0 && changes == 2);
    for (int i = 0; i < 120; ++i)
        frame();
    ESIA_CHECK(!h.ui.Animating());   // the pill has landed and the lens melted back
}

ESIA_TEST(UiNavigation, PushSlidesInAndTheBackButtonPops)
{
    UiHarness h;
    bool rootShown = false, detailShown = false;
    bool push = false;
    auto frame = [&] {
        h.Frame([&] {
            rootShown = detailShown = false;
            ui::BeginNavigation("nav", "root");
            if (ui::BeginPage("root", "Root"))
            {
                rootShown = true;
                if (push)
                    ui::NavigationPush("detail");
                ui::EndPage();
            }
            if (ui::BeginPage("detail", "Detail"))
            {
                detailShown = true;
                ui::EndPage();
            }
            ui::EndNavigation();
        });
    };
    frame();
    ESIA_CHECK(rootShown && !detailShown);
    push = true;
    frame();
    push = false;
    frame();
    ESIA_CHECK(rootShown && detailShown);   // mid-transition: both
    for (int i = 0; i < 90; ++i)
        frame();
    ESIA_CHECK(!rootShown && detailShown && !h.ui.Animating());

    // the back button: the top-left of the navigation bar (44 high), titled after the page below
    h.ctx.QueueInput(InputEvent::MouseMove({20, 22}));
    frame();
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, true));
    frame();
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false));
    frame();
    for (int i = 0; i < 90; ++i)
        frame();
    ESIA_CHECK(rootShown && !detailShown);
}

ESIA_TEST(UiTabBar, FloatsAtTheBottomOfItsAreaAndSelects)
{
    UiHarness h;
    int tab = 0, changes = 0;
    Rect after;
    auto frame = [&] {
        h.Frame([&] {
            // 3 tabs of 88: the bar is 264 wide, centered, 60 high and 14 above the bottom of the 600 x 400 window
            if (ui::TabBar("tabs", &tab, {{ui::icons::Home, "Home"}, {ui::icons::Search, "Search"}, {ui::icons::Settings, "Settings"}}))
                ++changes;
            after = Box("after", {10, 10});
        });
    };
    frame();
    ESIA_CHECK(after.min.y == 60.0f + 22.0f + 8.0f);   // the bar reserves room below the content (plus the item spacing)
    h.ctx.QueueInput(InputEvent::MouseMove({300 + 88, 356}));   // the third tab's center
    frame();
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, true));
    frame();
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false));
    frame();
    ESIA_CHECK(tab == 2 && changes == 1);
}

ESIA_TEST(UiTabBar, DrawsOverWhatIsSubmittedAfterIt)
{
    for (int inCard = 0; inCard < 2; ++inCard)
    {
        UiHarness h;
        int tab = 0;
        for (int i = 0; i < 2; ++i)
            h.Frame([&] {
                if (inCard)
                    ui::BeginCard("card");   // its background is moved under its content at EndCard
                ui::TabBar("tabs", &tab, {{ui::icons::Home, "Home"}, {ui::icons::Search, "Search"}});
                if (inCard)
                    ui::EndCard();
                ui::BeginCard("after", {200, 100});   // drawn after the bar, at the top of the window
                ui::EndCard();
            });
        const std::vector<DrawCmd>& cmds = h.ctx.FindWindowByName("W")->GetDrawList().Commands();
        ESIA_CHECK(!cmds.empty());
        // the last commands are the bar's, clipped to it at the bottom of the window (not the card at the top)
        if (!cmds.empty())
            ESIA_CHECK(cmds.back().clip.min.y > 300.0f);
    }
}
