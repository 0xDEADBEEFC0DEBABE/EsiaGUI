// The widget layer without a GPU: springs, themes, styles, auto layout and interaction, on a Context driven frame by
// frame (no text system: text measures as empty, the layout and the logic are what is tested).
#include "esia/ui/ui.hpp"
#include "esia_test.hpp"
#include "ui_internal.hpp"
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cmath>
#include <functional>
#include <string>
#include <thread>
#include <vector>

using namespace esia;

namespace
{
    // The UI tests run the debug checks in every build, failing on any report: the widgets themselves never share
    // an id or lay an item out of view. A test that wants the reports passes its own `diagnostics`.
    ContextDesc Strict(ContextDesc d)
    {
        d.debugChecks = DebugChecks::On;
        if (!d.diagnostics)
            d.diagnostics = [](const Diagnostic& x) {
                std::fprintf(stderr, "  %s\n", x.message.c_str());
                ::esia::test::Fail(__FILE__, __LINE__, "a debug check reported something");
            };
        return d;
    }

    struct UiHarness
    {
        Context ctx;
        ui::Ui ui{ctx, {}};
        double t = 1.0;
        Vec2 display{800, 600};
        Vec2 scale{1, 1};   // physical pixels per UI unit

        explicit UiHarness(const ContextDesc& desc = {}) : ctx(Strict(desc)) {}

        // One frame: `body` runs inside a window at (0, 0) of 600 x 400 without padding.
        template <class F>
        void Frame(F&& body, double dt = 1.0 / 60.0)
        {
            t += dt;
            ctx.NewFrame({display, scale, t});
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

ESIA_TEST(UiStyle, TextOutlineReachesTheLabelsOfItsScope)
{
    // a text system that records which texts got an outline, and how wide
    struct Recorder final : text::TextSystem
    {
        std::vector<std::pair<std::string, float>> outlined;
        std::vector<std::string> drawn;
        text::FontId AddFontFile(const char*, int) override { return 1; }
        text::FontId AddFontMemory(const void*, std::size_t, int) override { return 1; }
        void AddFallback(text::FontId) override {}
        void NewFrame(const text::RasterParams&) override {}
        text::TextMetrics Measure(text::FontRef f, std::string_view s, float, std::uint32_t) override
        {
            return {Vec2(6.0f * (float)s.size(), f.size), f.size * 0.8f, 1};
        }
        Vec2 Draw(DrawList&, text::FontRef f, Vec2, Color, std::string_view s, float, std::uint32_t, float) override
        {
            drawn.emplace_back(s);
            return Measure(f, s, 0, 0).size;
        }
        void DrawGlyph(DrawList&, text::FontRef, char32_t, Vec2, Color) override {}
        void DrawOutline(DrawList&, text::FontRef, Vec2, const text::TextOutline& o, std::string_view s, float, std::uint32_t, float) override
        {
            outlined.emplace_back(std::string(s), o.width);
        }
    };
    Recorder rec;
    Context ctx(Strict({}));
    ui::UiDesc desc;
    desc.text = &rec;
    desc.fallbackChain = false;
    ui::Ui u(ctx, desc);
    ctx.NewFrame({Vec2(800, 600), Vec2(1, 1), 1.0});
    u.NewFrame();
    ctx.SetNextWindowPos({0, 0}, Cond::FirstUse);
    ctx.SetNextWindowSize({600, 400}, Cond::FirstUse);
    ctx.Begin("W", WindowOptions{});
    ui::Text(ui::TextStyle::Body, "plain");
    {
        ui::StyleScope scope(ui::ItemStyle().TextOutline(2.0f, Color::Black()));
        ui::Text(ui::TextStyle::Body, "scoped");
        ui::Button("Scoped button");
    }
    ui::Next().TextOutline(1.0f, Color::White());
    ui::Button("Next button");
    ui::Text(ui::TextStyle::Body, "after");
    ctx.End();
    u.EndFrame();
    ctx.EndFrame();
    const auto has = [&](const std::string& s) { return std::find(rec.drawn.begin(), rec.drawn.end(), s) != rec.drawn.end(); };
    ESIA_CHECK(has("plain") && has("scoped") && has("Scoped button") && has("Next button") && has("after"));
    ESIA_CHECK(rec.outlined.size() == 3);
    if (rec.outlined.size() == 3)
    {
        ESIA_CHECK(rec.outlined[0] == std::make_pair(std::string("scoped"), 2.0f));
        ESIA_CHECK(rec.outlined[1] == std::make_pair(std::string("Scoped button"), 2.0f));
        ESIA_CHECK(rec.outlined[2] == std::make_pair(std::string("Next button"), 1.0f));
    }
}

ESIA_TEST(UiAnim, TableKeepsEntriesThroughGrowthAndForgetsUnused)
{
    using Table = ui::detail::AnimTable;
    Table t;
    bool created = false, allCreated = true, allFound = true;
    // a spring and a timer for each of 2500 ids: their own entries, through every growth of the table
    for (Id id = 1; id <= 2500; ++id)
    {
        t.Get(id, Table::kSpring, 1, created).value = (float)id;
        allCreated = allCreated && created;
        t.Get(id, Table::kTimer, 1, created).value = -(float)id;
        allCreated = allCreated && created;
    }
    ESIA_CHECK(allCreated && t.count == 5000 && t.slots.size() >= 10000);
    for (Id id = 1; id <= 2500; ++id)
    {
        allFound = allFound && t.Get(id, Table::kSpring, 2, created).value == (float)id && !created;
        allFound = allFound && t.Get(id, Table::kTimer, 2, created).value == -(float)id && !created;
    }
    ESIA_CHECK(allFound);
    // entries not asked for in `retain` frames go; the others are still found
    for (Id id = 1; id <= 100; ++id)
        t.Get(id, Table::kSpring, 700, created);
    t.Collect(700, 600);
    ESIA_CHECK(t.count == 100);
    allFound = true;
    for (Id id = 1; id <= 100; ++id)
        allFound = allFound && t.Get(id, Table::kSpring, 701, created).value == (float)id && !created;
    ESIA_CHECK(allFound);
    ESIA_CHECK(t.Get(101, Table::kSpring, 701, created).value == 0.0f && created);
    ESIA_CHECK(t.Get(1, Table::kTimer, 701, created).value == 0.0f && created);
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

namespace
{
    // A field at (0, 0) of 600 x 38 in the harness window; `second` below it (y 46 .. 84)
    struct FieldHarness : UiHarness
    {
        std::string a, b;
        ui::TextFieldResult ra, rb;
        ui::TextFieldOptions oa;
        std::string clipboard;

        FieldHarness() : UiHarness(Desc(clipboard)) {}
        static ContextDesc Desc(std::string& clip)
        {
            ContextDesc d;
            d.getClipboard = [&clip] { return clip; };
            d.setClipboard = [&clip](const std::string& v) { clip = v; };
            return d;
        }
        void Step()
        {
            Frame([&] {
                ra = ui::TextField("a", &a, "First", oa);
                rb = ui::TextField("b", &b, "Second");
            });
        }
        void Click(Vec2 at)
        {
            ctx.QueueInput(InputEvent::MouseMove(at));
            Step();
            ctx.QueueInput(InputEvent::Button(MouseButton::Left, true));
            Step();
            ctx.QueueInput(InputEvent::Button(MouseButton::Left, false));
            Step();
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

#if defined(__APPLE__)
    constexpr std::uint32_t kShortcut = Mod_Super;
#else
    constexpr std::uint32_t kShortcut = Mod_Ctrl;
#endif
}

ESIA_TEST(UiTextField, TypesEditsAndUndoes)
{
    FieldHarness h;
    h.Step();
    h.Type("ignored");   // nobody has the keyboard
    ESIA_CHECK(h.a.empty());
    h.Click({300, 19});
    ESIA_CHECK(h.ctx.KeyboardFocusId() != 0);
    h.ctx.QueueInput(InputEvent::TextEvent("h\xC3\xA9llo"));   // h, U+00E9, l, l, o
    h.Step();
    ESIA_CHECK(h.a == "h\xC3\xA9llo" && h.ra.changed);
    h.Key(Key::Backspace);
    ESIA_CHECK(h.a == "h\xC3\xA9ll");
    // two to the left, then select one more and replace it
    h.Key(Key::Left);
    h.Key(Key::Left);
    h.Key(Key::Left, Mod_Shift);
    h.Type("E");
    ESIA_CHECK(h.a == "hEll");   // the two-byte character went as one
    // select all, delete, undo, redo
    h.Key(Key::A, kShortcut);
    h.Key(Key::Delete);
    ESIA_CHECK(h.a.empty());
    h.Key(Key::Z, kShortcut);
    ESIA_CHECK(h.a == "hEll");
    h.Key(Key::Y, kShortcut);
    ESIA_CHECK(h.a.empty());
}

ESIA_TEST(UiTextField, ClipboardEnterAndTab)
{
    FieldHarness h;
    h.Step();
    h.Click({300, 19});
    h.Type("copy me");
    h.Key(Key::A, kShortcut);
    h.Key(Key::C, kShortcut);
    ESIA_CHECK(h.clipboard == "copy me");
    h.Key(Key::End);
    h.clipboard = "\r\nand this\t";
    h.Key(Key::V, kShortcut);
    ESIA_CHECK(h.a == "copy meand this");   // pasted on one line

    // Tab gives the keyboard to the next field
    h.Key(Key::Tab);
    h.Type("second");
    ESIA_CHECK(h.b == "second" && h.a == "copy meand this");

    // Enter submits and lets go of the keyboard
    h.ctx.QueueInput(InputEvent::KeyEvent(Key::Enter, true));
    h.Step();
    ESIA_CHECK(h.rb.submitted && h.ctx.KeyboardFocusId() == 0);
    h.ctx.QueueInput(InputEvent::KeyEvent(Key::Enter, false));
    h.Step();
    h.Type("x");
    ESIA_CHECK(h.b == "second");
}

ESIA_TEST(UiTextField, CompositionPasswordAndLimit)
{
    FieldHarness h;
    h.oa.password = true;
    h.oa.maxBytes = 4;
    h.Step();
    h.Click({300, 19});
    h.Type("abc");
    h.Type("de");   // "abcde" is longer than 4 bytes: refused
    ESIA_CHECK(h.a == "abc");
    h.Key(Key::A, kShortcut);
    h.Key(Key::C, kShortcut);
    ESIA_CHECK(h.clipboard.empty());   // a password is not copied

    // the IME's composition is shown, not written; what it commits is typed text
    h.Click({300, 46 + 19});
    h.ctx.QueueInput(InputEvent::Composition("ni", 2));
    h.Step();
    ESIA_CHECK(h.b.empty() && !h.rb.changed);
    h.ctx.QueueInput(InputEvent::Composition("", 0));
    h.ctx.QueueInput(InputEvent::TextEvent("\xE4\xBD\xA0"));   // U+4F60
    h.Step();
    ESIA_CHECK(h.b == "\xE4\xBD\xA0" && h.rb.changed);
}

namespace
{
    void ClickAt(UiHarness& h, Vec2 at, const std::function<void()>& frame)
    {
        h.ctx.QueueInput(InputEvent::MouseMove(at));
        frame();
        h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, true));
        frame();
        h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false));
        frame();
    }
}

ESIA_TEST(UiPopups, PickerOpensItsMenuAndTakesAChoice)
{
    UiHarness h;
    int choice = 0, changes = 0, behind = 0;
    auto frame = [&] {
        h.Frame([&] {
            // 200 x 34 at (0, 0): its menu opens under it, right-aligned: 200 wide (at least the picker), rows of 34
            // from y 40 + 6
            if (ui::Picker("pick", &choice, {"One", "Two", "Three"}, 200))
                ++changes;
            h.ctx.SetCursorPos({0, 300});
            if (ui::Interact("behind", {600, 100}).pressed)   // under the click that dismisses the menu
                ++behind;
        });
    };
    frame();
    ClickAt(h, {100, 17}, frame);
    for (int i = 0; i < 3; ++i)
        frame();   // shown (an auto-sized popup is measured in its first frame)
    ClickAt(h, {100, 46 + 34 + 17}, frame);   // the second row
    ESIA_CHECK(choice == 1 && changes == 1);

    // a click outside closes it, and belongs to it: the item under it does not react
    ClickAt(h, {100, 17}, frame);
    for (int i = 0; i < 3; ++i)
        frame();
    ClickAt(h, {300, 350}, frame);
    ESIA_CHECK(behind == 0 && choice == 1);
    ClickAt(h, {300, 350}, frame);
    ESIA_CHECK(behind == 1);   // closed: the next click reaches it
}

// A long list scrolls inside a menu of ten rows, opened at the choice; the menu, anchored between two pixels at its
// right edge, lands on whole pixels and stays put
ESIA_TEST(UiPopups, ALongPickerScrollsOpenedAtTheChoice)
{
    UiHarness h;
    h.scale = {1.25f, 1.25f};
    std::vector<std::string> names;
    for (int i = 0; i < 50; ++i)
        names.push_back("Item " + std::to_string(i));
    const std::vector<std::string_view> items(names.begin(), names.end());
    int choice = 40;
    auto frame = [&] {
        h.Frame([&] {
            h.ctx.SetCursorPos({100.5f, 0.0f});   // its right edge (the menu's anchor) at 300.5
            ui::Picker("pick", &choice, std::span<const std::string_view>(items), 200);
        });
    };
    const auto menu = [&] {
        for (Window* w : h.ctx.WindowsInDrawOrder())
            if (w->Layer() == WindowLayer::Overlay)
                return w->GetRect();
        return Rect();
    };
    frame();
    ClickAt(h, {200, 17}, frame);
    std::vector<Rect> seen;
    for (int i = 0; i < 8; ++i)
    {
        frame();
        seen.push_back(menu());
    }
    const Rect m = seen.back();
    for (std::size_t i = 2; i < seen.size(); ++i)
        ESIA_CHECK(seen[i] == m);
    ESIA_CHECK(m.Height() == 10 * 34.0f + 12.0f && m.max.x <= 300.5f && m.max.x > 299.5f);
    const auto whole = [](float v) { return std::fabs(v * 1.25f - std::round(v * 1.25f)) < 1e-3f; };
    ESIA_CHECK(whole(m.min.x) && whole(m.min.y) && whole(m.Width()));
    // the choice is in the middle: the row under it is the next one
    ClickAt(h, {m.Center().x, m.min.y + 6.0f + 170.0f + 17.0f}, frame);
    ESIA_CHECK(choice == 41);
}

// SetDarkMode swaps the colors and keeps what SetTheme set: the scale, the indicators
ESIA_TEST(UiTheme, DarkModeKeepsTheMetrics)
{
    UiHarness h;
    ui::Theme t = ui::ThemeLight();
    t.metrics.scale = 1.5f;
    const float system = t.metrics.scrollIndicatorAlways;   // 1 on desktops, 0 on phones
    t.metrics.scrollIndicatorAlways = 1.0f - system;
    h.ui.SetTheme(t, false);
    h.ui.SetDarkMode(true, false);
    h.Frame([] {});
    const ui::Theme& now = h.ui.GetTheme();
    ESIA_CHECK(now.dark && now.metrics.scale == 1.5f && now.metrics.scrollIndicatorAlways == 1.0f - system);
    ESIA_CHECK(now.colors.label == ui::ThemeDark().colors.label);
    ESIA_CHECK(ui::LerpTheme(ui::ThemeLight(), t, 0.5f).metrics.scrollIndicatorAlways == 0.5f);   // it blends
#if defined(_WIN32) || (defined(__linux__) && !defined(__ANDROID__))
    ESIA_CHECK(system == 1.0f && ui::ThemeDark().metrics.scrollIndicatorAlways == 1.0f);   // a desktop's
#endif
    // the density is the Ui's: SetTheme and SetDarkMode keep it
    h.ui.SetDensity(ui::Density::Compact, false);
    h.ui.SetTheme(ui::ThemeLight(), false);
    h.ui.SetDarkMode(true, false);
    h.Frame([] {});
    ESIA_CHECK(h.ui.GetDensity() == ui::Density::Compact && h.ui.GetTheme().metrics.compact == 1.0f && h.ui.GetTheme().dark);
    h.ui.SetDensity(ui::Density::Regular, false);
    ESIA_CHECK(h.ui.GetDensity() == ui::Density::Regular && h.ui.GetTheme().metrics.compact == 0.0f);
}

ESIA_TEST(UiPopups, MenuItemsCloseTheMenu)
{
    UiHarness h;
    int open = 0, renamed = 0;
    auto frame = [&] {
        h.Frame([&] {
            if (ui::Interact("more", {40, 40}).pressed)
                ui::OpenMenu("actions");
            if (ui::BeginMenu("actions", {.anchor = {0, 50}}))
            {
                ++open;
                if (ui::MenuItem("Rename", ui::icons::Edit))
                    ++renamed;
                ui::MenuItem("Delete", ui::icons::Delete);
                ui::EndMenu();
            }
        });
    };
    frame();
    ESIA_CHECK(open == 0);
    ClickAt(h, {20, 20}, frame);
    for (int i = 0; i < 3; ++i)
        frame();
    ESIA_CHECK(open > 0);
    ClickAt(h, {90, 50 + 6 + 17}, frame);   // Rename
    ESIA_CHECK(renamed == 1);
    open = 0;
    frame();
    frame();
    ESIA_CHECK(open == 0);
}

ESIA_TEST(UiPopups, TooltipAfterAShortHover)
{
    UiHarness h;
    auto frame = [&](double dt) {
        h.Frame(
            [&] {
                ui::Interact("button", {100, 40});
                ui::Tooltip("What it does");
            },
            dt);
    };
    const auto lists = [&] { return h.ctx.GetDrawData().lists.size(); };
    frame(0.1);
    const std::size_t alone = lists();
    h.ctx.QueueInput(InputEvent::MouseMove({50, 20}));
    frame(0.1);
    frame(0.1);
    ESIA_CHECK(lists() == alone && h.ui.Animating());   // waiting: frames keep coming
    for (int i = 0; i < 5; ++i)
        frame(0.1);
    ESIA_CHECK(lists() == alone + 1);   // the tooltip's window
    h.ctx.QueueInput(InputEvent::MouseMove({300, 300}));
    frame(0.1);
    frame(0.1);
    ESIA_CHECK(lists() == alone);
    h.ctx.QueueInput(InputEvent::MouseMove({50, 20}));
    frame(0.1);
    frame(0.1);
    ESIA_CHECK(lists() == alone);   // back over it: it waits again
}

// A finger drags a scroll area's content from its empty space and lets it glide on; a mouse does that only with
// InputConfig::mouseDragScrolls (it has the wheel and the indicator). The indicator drags with the mouse.
ESIA_TEST(UiScroll, DragTheContentOrTheIndicator)
{
  for (int mode = 0; mode < 3; ++mode)   // a mouse, a finger, a mouse with mouseDragScrolls
  {
    ContextDesc desc;
    desc.input.mouseDragScrolls = mode == 2;
    UiHarness h(desc);
    const bool finger = mode == 1;
    float scroll = 0.0f, max = 0.0f;
    auto frame = [&] {
        h.Frame([&] {
            ui::BeginScrollArea("area", {600, 200});
            ui::Spacer(1000);   // content without items: empty space to drag
            scroll = h.ctx.Scroll().y;
            max = h.ctx.ScrollMax().y;
            ui::EndScrollArea();
        });
    };
    frame();
    frame();
    ESIA_CHECK(max > 700.0f);

    // drag the content up by 100: it follows the finger (the mouse with mouseDragScrolls)
    h.ctx.QueueInput(InputEvent::MouseMove({300, 150}));
    frame();
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, true, finger));
    frame();
    for (int i = 1; i <= 10; ++i)
    {
        h.ctx.QueueInput(InputEvent::MouseMove({300, 150 - 10.0f * (float)i}));
        frame();
    }
    frame();
    if (mode == 0)
    {
        ESIA_CHECK(scroll == 0.0f);   // a desktop's mouse: the content stays
        h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false));
        frame();
        continue;
    }
    ESIA_CHECK(std::fabs(scroll - 100.0f) < 1.0f);
    // let go: it glides on
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false, finger));
    for (int i = 0; i < 60; ++i)
        frame();
    ESIA_CHECK(scroll > 110.0f && scroll <= max);

    // the indicator: at the right edge (no window padding to be its lane here), 6 in from the ends of the view;
    // dragged to the bottom, the content goes to its end
    const float trackH = 200.0f - 12.0f;
    const float thumbH = std::max(36.0f, trackH * 200.0f / (200.0f + max));
    const float thumbMid = 6.0f + (trackH - thumbH) * scroll / max + thumbH * 0.5f;
    h.ctx.QueueInput(InputEvent::MouseMove({594, thumbMid}));
    frame();
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, true));
    frame();
    for (float y = thumbMid; y < 199.0f; y += 20.0f)
    {
        h.ctx.QueueInput(InputEvent::MouseMove({594, std::min(y + 20.0f, 199.0f)}));
        frame();
    }
    frame();
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false));
    frame();
    ESIA_CHECK(std::fabs(scroll - max) < 1.0f);
  }
}

// A window scrolled to its end: its indicator stops above the rounded bottom corner (inside the window's shape), and
// the grip there resizes the window (the indicator used to sit on it and take the press)
ESIA_TEST(UiWindow, TheGripResizesAWindowScrolledToItsEnd)
{
    UiHarness h;
    const Id thumb = ui::Salt(HashLabel("##content", HashLabel("Glass", 0)), 0x1D1);   // ScrollEnd's indicator
    Rect thumbRect, window;
    bool scrollToEnd = true;
    auto frame = [&] {
        h.Frame([&] {
            if (ui::BeginWindow("Glass", nullptr, {.size = {300, 300}, .pos = {100, 50}}))
            {
                ui::Spacer(1000);
                if (scrollToEnd)
                    h.ctx.SetScrollY(1e6f);
                thumbRect = h.ctx.ItemStatusOf(thumb).rect;
                window = h.ctx.CurrentWindow()->GetRect();
                ui::EndWindow();
            }
        });
    };
    for (int i = 0; i < 30; ++i)
        frame();
    scrollToEnd = false;
    frame();
    ESIA_CHECK(!thumbRect.Empty() && thumbRect.max.y <= window.max.y - 28.0f + 0.01f);
    const Vec2 grab = window.max - Vec2(10, 10);   // where the indicator used to end
    h.ctx.QueueInput(InputEvent::MouseMove(grab));
    frame();
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, true));
    frame();
    h.ctx.QueueInput(InputEvent::MouseMove(grab + Vec2(20, 30)));
    frame();
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false));
    frame();
    ESIA_CHECK(window.Width() == 320.0f && window.Height() == 330.0f);
}

// A finger that presses a row and then moves along the area scrolls it, past a slop, and the row does not press; a
// tap still presses, a sideways move does not scroll, and the mouse keeps the row (it drags only empty space).
ESIA_TEST(UiScroll, TouchDragFromARowScrolls)
{
    UiHarness h;
    float scroll = 0.0f;
    int presses = 0;
    bool held = false;
    auto frame = [&] {
        h.Frame([&] {
            ui::BeginScrollArea("area", {600, 200});
            for (int i = 0; i < 20; ++i)
            {
                const ui::Interaction it = ui::Interact(("row" + std::to_string(i)).c_str(), {600, 50});
                presses += it.pressed;
                held = held || it.held;
            }
            scroll = h.ctx.Scroll().y;
            ui::EndScrollArea();
        });
    };
    const auto gesture = [&](bool touch, Vec2 from, Vec2 step, int steps) {
        presses = 0;
        h.ctx.QueueInput(InputEvent::MouseMove(from));
        frame();
        h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, true, touch));
        frame();
        for (int i = 1; i <= steps; ++i)
        {
            h.ctx.QueueInput(InputEvent::MouseMove(from + step * (float)i));
            held = false;
            frame();
        }
        h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false, touch));
        frame();
    };
    frame();
    frame();

    // up by 10 a frame from a row: the first move passes the slop and hands the touch to the area, which follows the
    // rest (90); the row lets go and does not press
    gesture(true, {300, 160}, {0, -10}, 10);
    ESIA_CHECK(presses == 0);
    ESIA_CHECK(!held);
    ESIA_CHECK(scroll > 85.0f && scroll < 130.0f);   // then gliding on
    for (int i = 0; i < 90; ++i)
        frame();
    const float rested = scroll;

    // a tap presses; a slight wobble under the slop too
    gesture(true, {300, 100}, {0, 0}, 0);
    ESIA_CHECK(presses == 1);
    gesture(true, {300, 100}, {1, -1.5f}, 4);
    ESIA_CHECK(presses == 1 && std::fabs(scroll - rested) < 0.5f);
    // sideways: no scroll (the row keeps the touch: released over it, it presses)
    gesture(true, {200, 100}, {10, 0}, 8);
    ESIA_CHECK(std::fabs(scroll - rested) < 0.5f);
    // the mouse: the row stays held, nothing scrolls, released over another row it does not press
    gesture(false, {300, 160}, {0, -10}, 10);
    ESIA_CHECK(presses == 0 && std::fabs(scroll - rested) < 0.5f);
}

// The same from an item that acts on the press (a slider, a segmented control): the finger's press is held back
// from it, so a swipe along the area scrolls and the item never acts; across, the item gets it (a slider drags).
ESIA_TEST(UiScroll, TouchDragFromAPressOnClickItemScrolls)
{
    UiHarness h;
    float scroll = 0.0f;
    int presses = 0;
    auto frame = [&] {
        h.Frame([&] {
            ui::BeginScrollArea("area", {600, 200});
            for (int i = 0; i < 20; ++i)
                presses += ui::Interact(("slider" + std::to_string(i)).c_str(), {600, 50}, ui::InteractFlags_PressOnClick).pressed;
            scroll = h.ctx.Scroll().y;
            ui::EndScrollArea();
        });
    };
    const auto swipe = [&](Vec2 from, Vec2 step, int steps) {
        presses = 0;
        h.ctx.QueueInput(InputEvent::MouseMove(from));
        frame();
        h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, true, true));
        frame();
        for (int i = 1; i <= steps; ++i)
        {
            h.ctx.QueueInput(InputEvent::MouseMove(from + step * (float)i));
            frame();
        }
        h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false, true));
        frame();
    };
    frame();
    frame();
    swipe({300, 160}, {0, -10}, 10);
    ESIA_CHECK(presses == 0);
    ESIA_CHECK(scroll > 85.0f && scroll < 130.0f);
    for (int i = 0; i < 90; ++i)
        frame();
    const float rested = scroll;
    swipe({200, 100}, {10, 0}, 8);
    ESIA_CHECK(presses == 1 && std::fabs(scroll - rested) < 0.5f);
}

// The offset the layout starts at lands on whole pixels: dragging on past the end at a fractional scale (content
// whose measured size depends on where between pixels it starts) keeps the content still instead of shaking it.
ESIA_TEST(UiScroll, DragPastTheEndStaysStill)
{
    UiHarness h;
    h.scale = {2.625f, 2.625f};   // a phone's density
    std::vector<float> offsets;
    auto frame = [&] {
        h.Frame([&] {
            ui::BeginScrollArea("area", {600, 200});
            for (int i = 0; i < 30; ++i)
                Box(("b" + std::to_string(i)).c_str(), {600, 33.37f});
            offsets.push_back(h.ctx.Scroll().y);
            ui::EndScrollArea();
        });
    };
    frame();
    frame();
    // a finger's drag from a row, up to the end and on
    h.ctx.QueueInput(InputEvent::MouseMove({300, 190}));
    frame();
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, true, true));
    frame();
    for (int i = 1; i <= 120; ++i)
    {
        h.ctx.QueueInput(InputEvent::MouseMove({300, 190 - 15.0f * (float)i}));
        frame();
    }
    const float last = offsets.back();
    const float pixel = 1.0f / 2.625f;
    ESIA_CHECK(last > 700.0f);
    for (std::size_t i = offsets.size() - 40; i < offsets.size(); ++i)
        ESIA_CHECK(offsets[i] == last);
    ESIA_CHECK(std::fabs(last / pixel - std::round(last / pixel)) < 1e-3f);   // on a pixel
    h.ctx.QueueInput(InputEvent::Button(MouseButton::Left, false, true));
    frame();
}

ESIA_TEST(UiScroll, ContentFadesAtTheDisplaysEdge)
{
    // a scroll area reaching past the bottom of the display: its content fades at the display's edge (y 300), where
    // it stops being visible, not at its own (400)
    UiHarness h;
    h.display = {800, 300};
    const DrawList* list = nullptr;
    for (int i = 0; i < 3; ++i)
        h.Frame([&] {
            ui::BeginScrollArea("area", {600, 400});
            ui::Spacer(1000);
            list = &h.ctx.WindowDrawList();
            ui::EndScrollArea();
        });
    ESIA_CHECK(list != nullptr && list->Fades().size() == 1);
    if (list != nullptr && list->Fades().size() == 1)
    {
        const fx::FadeParams& f = list->Fades()[0];
        ESIA_CHECK(f.y0 == 0.0f && f.y1 == 300.0f);
        ESIA_CHECK(f.top == 0.0f && f.bottom == 30.0f);   // at the top: only the bottom fades
    }
}

ESIA_TEST(UiCharts, LineChartTakesItsRoomOrDrawsIntoARect)
{
    UiHarness h;
    Rect after1, after2;
    const float values[] = {1.0f, 3.0f, 2.0f, 5.0f};
    std::size_t cmds = 0;
    h.Frame([&] {
        ui::LineChart("a", values, {.height = 50});
        after1 = Box("x", {10, 10});
        ui::LineChart("b", values, {.rect = Rect(0, 300, 200, 350)});
        after2 = Box("y", {10, 10});
        ui::LineChart("c", std::span<const float>(values, 1));   // one value: nothing to draw, only its room
        cmds = h.ctx.WindowDrawList().Commands().size();
    });
    ESIA_CHECK(after1.min.y == 50.0f + 8.0f);           // its height and the item spacing
    ESIA_CHECK(after2.min.y == after1.max.y + 8.0f);    // in a rect it takes no room
    ESIA_CHECK(cmds > 0);
}

ESIA_TEST(UiGlow, AGlowingChildGetsRoomForItsGlow)
{
    UiHarness h;
    Rect plain[2], glowing[2];
    auto frame = [&] {
        h.Frame([&] {
            // the same row twice: plain buttons, then the first one glowing
            ui::BeginHStack("plain", {.spacing = 8});
            ui::Button("A##p");
            plain[1] = Box("next##p", {40, 40});
            ui::EndStack();
            plain[0] = h.ctx.LastItemStatus().rect;
            ui::BeginHStack("glowing", {.spacing = 8});
            ui::Button("A##g", {.glow = true});
            glowing[1] = Box("next##g", {40, 40});
            ui::EndStack();
            glowing[0] = h.ctx.LastItemStatus().rect;
        });
    };
    for (int i = 0; i < 90; ++i)
        frame();   // measured, then moved there on the layout's spring
    // the stacks start at the same x: the glowing button's neighbour sits further away (a 16-unit glow reserves 12,
    // more than the spacing of 8)
    ESIA_CHECK(glowing[0].min.x == plain[0].min.x && glowing[1].min.x >= plain[1].min.x + 3.5f);
}

ESIA_TEST(UiOverlays, DockTilesToggleTheirPanels)
{
    UiHarness h;
    bool a = false, b = true;
    int clicked = -1;
    auto frame = [&] {
        h.Frame([&] {
            // three tiles of 46, gaps of 12, padding 11: 184 x 68, centered at the bottom of the 800 x 600 display, 16 up
            const int c = ui::Dock({{"A", ui::icons::Settings, Color::Clear(), &a}, {"B", ui::icons::Apps, Color::Clear(), &b}, {"C", ui::icons::Globe}});
            if (c >= 0)
                clicked = c;
        });
    };
    frame();
    ClickAt(h, {308 + 11 + 23, 516 + 34}, frame);   // the first tile
    ESIA_CHECK(a && b && clicked == 0);
    ClickAt(h, {308 + 11 + 46 + 12 + 23, 516 + 34}, frame);
    ESIA_CHECK(a && !b && clicked == 1);
    ClickAt(h, {308 + 11 + 2 * 58 + 23, 516 + 34}, frame);   // a tile without a flag: only reported
    ESIA_CHECK(a && !b && clicked == 2);
}

ESIA_TEST(UiOverlays, TheIslandShowsNotificationsAndActivities)
{
    UiHarness h;
    const auto drawn = [&] { return h.ctx.ForegroundDrawList().Commands().size(); };
    h.Frame([] {}, 0.1);
    const std::size_t idle = drawn();
    ui::Notification n;
    n.title = "Hello";
    n.message = "from a test";
    n.duration = 1.0f;
    h.ui.Notify(n);
    h.Frame([] {}, 0.1);
    h.Frame([] {}, 0.1);
    ESIA_CHECK(drawn() > idle && h.ui.Animating());
    for (int i = 0; i < 30; ++i)
        h.Frame([] {}, 0.1);   // its second is over, the island closed
    ESIA_CHECK(drawn() == idle);

    // a live activity, set from another thread, stays until it is cleared
    std::thread([&] { h.ui.SetActivity("download", "Downloading", 0.3f, ui::icons::Download); }).join();
    for (int i = 0; i < 30; ++i)
        h.Frame([] {}, 0.1);
    ESIA_CHECK(drawn() > idle);
    h.ui.ClearActivity("download");
    for (int i = 0; i < 30; ++i)
        h.Frame([] {}, 0.1);
    ESIA_CHECK(drawn() == idle);
}

ESIA_TEST(UiLayout, AGroupThatFillsIsAFlexibleChild)
{
    // two modules side by side (WGT's Control Center): each a group asking for the available width inside it
    UiHarness h;
    Rect a, b;
    for (int i = 0; i < 90; ++i)
        h.Frame([&] {
            ui::BeginHStack("pair", {.spacing = 12});
            for (Rect* r : {&a, &b})
            {
                h.ctx.BeginGroup();
                const float w = ui::AvailableWidth();
                *r = Rect::FromSize(h.ctx.CursorPos(), Vec2(w, 40));
                h.ctx.ItemSize(r->Size());
                h.ctx.EndGroup();
            }
            ui::EndStack();
        });
    ESIA_CHECK(std::fabs(a.Width() - 294.0f) < 1.0f && std::fabs(b.Width() - 294.0f) < 1.0f);   // (600 - 12) / 2 each
    ESIA_CHECK(std::fabs(b.min.x - (a.max.x + 12.0f)) < 1.0f);
}

namespace
{
    std::string Formatted(const char* fmt, ...)
    {
        char buf[64];
        va_list args;
        va_start(args, fmt);
        ui::detail::FormatV(buf, sizeof(buf), fmt, args);
        va_end(args);
        return buf;
    }

    std::string Vsnprintf(const char* fmt, ...)
    {
        char buf[64];
        va_list args;
        va_start(args, fmt);
        std::vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        return buf;
    }
}

// The labels' formatter writes what vsnprintf writes: the conversions it handles itself, the others through
// vsnprintf, cut at the same place.
ESIA_TEST(UiText, FormattingIsVsnprintfs)
{
#define ESIA_SAME_FORMAT(...) ESIA_CHECK(Formatted(__VA_ARGS__) == Vsnprintf(__VA_ARGS__))
    ESIA_SAME_FORMAT("Line %d: the quick brown fox", 42);
    ESIA_SAME_FORMAT("%d %i %u %x %X %%", -7, 2147483647, 4000000000u, 0xbeefu, 0xbeefu);
    ESIA_SAME_FORMAT("%ld %lld %zu %lu %zd", -5L, -9000000000LL, (std::size_t)123456789, 77UL, (std::ptrdiff_t)-3);
    ESIA_SAME_FORMAT("[%s] [%c] [%s]", "text", 'Q', "");
    ESIA_SAME_FORMAT("%5d|%-4s|%.2f|%08x", 12, "ab", 3.14159, 255u);   // flags, width, precision: vsnprintf's
    ESIA_SAME_FORMAT("no conversions at all");
    ESIA_SAME_FORMAT("%s", "a string longer than the sixty-three bytes of the buffer, cut where vsnprintf cuts it");
    ESIA_SAME_FORMAT("%d%s", 1234567, " and a tail that runs past the end of the sixty-four byte buffer for sure");
    ESIA_SAME_FORMAT("100%%");
#undef ESIA_SAME_FORMAT
}

// Sections without a header, or with one header, in one id scope keep their rows apart: the same row label in each
// is a row of its own (the strict harness fails on a shared id), and a click toggles that row only
ESIA_TEST(UiLists, SectionsWithOneHeaderKeepTheirRowsApart)
{
    UiHarness h;
    bool v[4] = {};
    Rect rows[4];
    auto frame = [&] {
        h.Frame([&] {
            for (int i = 0; i < 4; ++i)
            {
                ui::BeginSection(i < 2 ? std::string_view() : std::string_view("Network"));
                ui::RowToggle("Enabled", &v[i]);
                rows[i] = h.ctx.LastItemRect();
                ui::EndSection();
            }
        });
    };
    frame();
    frame();
    ClickAt(h, rows[3].Center(), frame);
    ClickAt(h, rows[1].Center(), frame);
    ESIA_CHECK(!v[0] && v[1] && !v[2] && v[3]);
}

// Widgets made in a loop: the same label in each pass is one id (the checks report it), each pass in an IdScope a
// widget of its own
ESIA_TEST(UiIds, IdScopeKeepsALoopsWidgetsApart)
{
    std::vector<Diagnostic> got;
    ContextDesc desc;
    desc.diagnostics = [&got](const Diagnostic& d) { got.push_back(d); };
    UiHarness h(desc);
    bool scoped = true;
    int pressed = -1;
    Rect rects[3];
    auto frame = [&] {
        h.Frame([&] {
            for (int i = 0; i < 3; ++i)
            {
                if (scoped)
                {
                    ui::IdScope scope(i);
                    if (ui::Button("Set value"))
                        pressed = i;
                }
                else if (ui::Button("Set value"))
                    pressed = i;
                rects[i] = h.ctx.LastItemRect();
            }
        });
    };
    frame();
    frame();
    ClickAt(h, rects[1].Center(), frame);
    ESIA_CHECK(got.empty() && pressed == 1);
    scoped = false;
    frame();
    ESIA_CHECK(got.size() == 1 && got[0].kind == Diagnostic::Kind::IdConflict && got[0].message.find("\"Set value\"") != std::string::npos);
}

// One line for a desktop tool's sizes (UiDesc::density or Ui::SetDensity), as WinUI's compact sizing and Material's
// density do it: the text keeps its size, the controls and rows get shorter; the regular sizes are the design's
// Flat (UiDesc::flat, Ui::SetFlat): no shadow, glow or glass on any shape (without a text system the shapes stay FX
// instances; with one, most become geometry: tests/render "Painter.Flat...")
ESIA_TEST(UiFlat, NoShadowsGlowsOrGlass)
{
    const auto count = [](bool flat) {
        UiHarness h;
        h.ui.SetFlat(flat);
        bool on = true;
        float v = 0.5f;
        for (int i = 0; i < 2; ++i)
            h.Frame([&] {
                ui::Button("Apply");
                ui::Toggle("t", &on);
                ui::Slider("s", &v, 0.0f, 1.0f);
            });
        int shapes = 0, effects = 0;
        for (const DrawList* dl : h.ctx.GetDrawData().lists)
            for (const fx::Instance& in : dl->FxInstances())
            {
                ++shapes;
                effects += (in.flags[0] & (fx::kShadow | fx::kGlow | fx::kGlass)) ? 1 : 0;
            }
        ESIA_CHECK(h.ui.GetFlat() == flat);
        return std::pair<int, int>(shapes, effects);
    };
    const std::pair<int, int> regular = count(false), flat = count(true);
    ESIA_CHECK(regular.second > 0);
    ESIA_CHECK(flat.first > 0 && flat.second == 0);
}

ESIA_TEST(UiDensity, CompactKeepsTheTextAndShrinksTheControls)
{
    struct Sizes
    {
        float button = 0, toggle = 0, field = 0, row = 0, table = 0, font = 0;
    };
    const auto measure = [](ui::Density density) {
        UiHarness h;
        h.ui.SetDensity(density, false);
        Sizes s;
        double value = 1.0;
        bool on = false;
        auto frame = [&] {
            h.Frame([&] {
                ui::Button("Apply");
                s.button = h.ctx.LastItemRect().Height();
                ui::NumberField("value", &value);
                s.field = h.ctx.LastItemRect().Height();
                ui::BeginSection();   // rows abut: the distance between two switches is a row
                ui::RowToggle("Row", &on);
                const float first = h.ctx.LastItemRect().min.y;
                ui::RowToggle("Row 2", &on);
                s.row = h.ctx.LastItemRect().min.y - first;
                s.toggle = h.ctx.LastItemRect().Height();
                ui::EndSection();
                if (ui::BeginTable("t", {{"Name"}}))
                {
                    const ui::TableRange r = ui::TableVisible(3);
                    for (int i = r.first; i < r.last; ++i)
                    {
                        ui::TableRow(i);
                        ui::TableCell("row");
                    }
                    ui::EndTable();
                    s.table = h.ctx.LastItemRect().Height();
                }
            });
        };
        frame();
        frame();
        s.font = h.ui.Font(ui::TextStyle::Body).size;
        return s;
    };
    const Sizes regular = measure(ui::Density::Regular), compact = measure(ui::Density::Compact);
    ESIA_CHECK(regular.button == 36.0f && regular.toggle == 30.0f && regular.field == 32.0f && regular.row == 44.0f && regular.table == 30.0f + 3 * 34.0f);
    ESIA_CHECK(compact.button == 30.0f && compact.toggle == 24.0f && compact.field == 26.0f && compact.table == 26.0f + 3 * 28.0f);
    ESIA_CHECK_NEAR(compact.row, 34.0f, 0.01f);
    ESIA_CHECK(compact.font == regular.font && regular.font == 15.0f);   // the text keeps its size
}

// A bar floating over the bottom of a scroll area (iOS's scroll edge effect): what scrolls under it fades out by the
// bar's middle, from a little above the bar - as much as there is left to scroll; at the end nothing fades
ESIA_TEST(UiSearchBar, TheContentFadesOutUnderIt)
{
    UiHarness h;
    std::string text;
    bool toEnd = false;
    auto frame = [&] {
        h.Frame([&] {
            ui::BeginScrollArea("area", {600, 300});
            ui::Spacer(1000);
            if (toEnd)
                h.ctx.SetScrollY(1e6f);
            ui::SearchBar("search", &text);
            ui::EndScrollArea();
        });
    };
    frame();
    frame();
    const std::vector<fx::FadeParams>& fades = h.ctx.FindWindowByName("W")->GetDrawList().Fades();
    ESIA_CHECK(fades.size() == 1);
    if (fades.size() == 1)
    {
        // the bar: 46 tall, 14 above the area's bottom (300): 240 .. 286, its middle 263
        ESIA_CHECK_NEAR(fades[0].y1, 263.0f, 0.01f);
        ESIA_CHECK_NEAR(fades[0].bottom, 263.0f - (240.0f - 16.0f), 0.01f);
    }
    toEnd = true;
    for (int i = 0; i < 90; ++i)
        frame();   // the smooth scroll glides to the end
    const std::vector<fx::FadeParams>& end = h.ctx.FindWindowByName("W")->GetDrawList().Fades();
    ESIA_CHECK(end.empty() || (end[0].bottom == 0.0f && end[0].y1 == 300.0f));   // at the end: nothing fades at the bottom
}
