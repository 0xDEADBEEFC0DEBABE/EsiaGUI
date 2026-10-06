// Plots and the donut chart without a GPU: the ticks, the fitted view, the legend, pan and zoom and the wheel they take
// from the window, on a Context driven frame by frame with queued input (no text system: text measures as empty).
#include "esia/ui/ui.hpp"
#include "esia_test.hpp"
#include "ui_internal.hpp"
#include <cmath>
#include <cstdio>
#include <functional>
#include <vector>

using namespace esia;

namespace
{
    // Frames of `body` inside a window at (0, 0) of 600 x 400 without padding; input helpers between them.
    struct Harness
    {
        Context ctx;
        ui::Ui ui{ctx, {}};
        double t = 1.0;
        std::function<void()> body;

        Harness() : ctx(Desc()) {}
        static ContextDesc Desc()
        {
            ContextDesc d;
            // the debug checks in every build, failing on any report: a plot's parts never share an id
            d.debugChecks = DebugChecks::On;
            d.diagnostics = [](const Diagnostic& x) {
                std::fprintf(stderr, "  %s\n", x.message.c_str());
                ::esia::test::Fail(__FILE__, __LINE__, "a debug check reported something");
            };
            return d;
        }
        void Step(int frames = 1)
        {
            for (int i = 0; i < frames; ++i)
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
        }
        void Move(Vec2 at)
        {
            ctx.QueueInput(InputEvent::MouseMove(at));
            Step();
        }
        void Down()
        {
            ctx.QueueInput(InputEvent::Button(MouseButton::Left, true));
            Step();
        }
        void Up()
        {
            ctx.QueueInput(InputEvent::Button(MouseButton::Left, false));
            Step();
        }
        void Click(Vec2 at)
        {
            Move(at);
            Down();
            Up();
        }
        void Wheel(float y)
        {
            ctx.QueueInput(InputEvent::Wheel(0, y));
            Step();
        }
    };

    bool Near(float a, float b, float eps = 1e-3f) { return std::fabs(a - b) <= eps * std::max(1.0f, std::fabs(b)); }
}

ESIA_TEST(UiPlot, TicksAreOneTwoOrFiveTimesAPowerOfTen)
{
    std::vector<float> t;
    ESIA_CHECK(ui::detail::PlotTicks(0.0f, 10.0f, 5, t) == 2.0f);
    ESIA_CHECK((t == std::vector<float>{0, 2, 4, 6, 8, 10}));
    // a range across zero: zero is exactly a tick, the others multiples of the step (no drift)
    ESIA_CHECK(Near(ui::detail::PlotTicks(-0.37f, 0.81f, 6, t), 0.2f));
    ESIA_CHECK(t.size() == 6 && Near(t[0], -0.2f) && t[1] == 0.0f && Near(t[5], 0.8f));
    ESIA_CHECK(ui::detail::PlotTicks(0.0f, 1e6f, 4, t) == 5e5f && t.size() == 3);
    // an empty or broken range: no ticks
    ESIA_CHECK(ui::detail::PlotTicks(1.0f, 1.0f, 5, t) == 0.0f && t.empty());
    ESIA_CHECK(ui::detail::PlotTicks(0.0f, NAN, 5, t) == 0.0f && t.empty());
}

// The view fits the data: a line as it is (with room above and below), bars and histograms from zero
ESIA_TEST(UiPlot, TheViewFitsTheData)
{
    Harness h;
    const float ys[] = {10, 12, 20, 15, 11, 14, 16, 18, 13, 17};
    const float bars[] = {3, 5, 2};
    const float samples[] = {0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ui::PlotLimits line, bar, histogram;
    h.body = [&] {
        if (ui::BeginPlot("line", {.height = 100}))
        {
            line = ui::GetPlotLimits();
            ui::PlotLine("y", ys);
            ui::EndPlot();
        }
        if (ui::BeginPlot("bars", {.height = 100}))
        {
            bar = ui::GetPlotLimits();
            ui::PlotBars("b", bars);
            ui::EndPlot();
        }
        if (ui::BeginPlot("histogram", {.height = 100}))
        {
            histogram = ui::GetPlotLimits();
            ui::PlotHistogram("h", samples, 10);
            ui::EndPlot();
        }
    };
    h.Step(2);   // the first frame's view, read in the second
    ESIA_CHECK(line.xMin == 0.0f && line.xMax == 9.0f);
    ESIA_CHECK(Near(line.yMin, 9.4f) && Near(line.yMax, 20.6f));
    // bars: half a bar beyond the first and last, from zero (no room below it)
    ESIA_CHECK(bar.xMin == -0.5f && bar.xMax == 2.5f && bar.yMin == 0.0f && Near(bar.yMax, 5.3f));
    // 10 bins from 0 to 9: the first holds the three zeros
    ESIA_CHECK(Near(histogram.xMin, 0.0f) && Near(histogram.xMax, 9.0f) && histogram.yMin == 0.0f && Near(histogram.yMax, 3.18f));
}

// Given limits are kept; a series of non-finite values or none at all draws an empty plot
ESIA_TEST(UiPlot, LimitsAndEmptySeries)
{
    Harness h;
    const float ys[] = {NAN, INFINITY, 1.0f};
    ui::PlotLimits fixed, empty;
    h.body = [&] {
        if (ui::BeginPlot("fixed", {.height = 100, .xMin = -1, .xMax = 1, .yMin = 0, .yMax = 50}))
        {
            fixed = ui::GetPlotLimits();
            ui::PlotLine("y", ys);
            ui::EndPlot();
        }
        if (ui::BeginPlot("empty", {.height = 100}))
        {
            empty = ui::GetPlotLimits();
            ui::PlotLine("none", std::span<const float>());
            ui::PlotHistogram("none either", std::span<const float>());
            ui::EndPlot();
        }
    };
    h.Step();
    ESIA_CHECK(fixed.xMin == -1.0f && fixed.xMax == 1.0f && fixed.yMin == 0.0f && fixed.yMax == 50.0f);
    h.Step();
    ESIA_CHECK(fixed.xMin == -1.0f && fixed.xMax == 1.0f && fixed.yMin == 0.0f && fixed.yMax == 50.0f);
    ESIA_CHECK(empty.xMax > empty.xMin && empty.yMax > empty.yMin);
}

// A click on a legend entry hides its series: the view fits what is left; a second click shows it again
ESIA_TEST(UiPlot, TheLegendHidesASeries)
{
    Harness h;
    const float small[] = {0, 1, 0, 1};
    const float big[] = {0, 100, 50, 100};
    ui::PlotLimits l;
    h.body = [&] {
        if (ui::BeginPlot("p", {.height = 220}))
        {
            l = ui::GetPlotLimits();
            ui::PlotLine("small", small);
            ui::PlotLine("big", big);
            ui::EndPlot();
        }
    };
    h.Step(2);
    ESIA_CHECK(Near(l.yMax, 106.0f));
    // the legend: under the area (to y 220 - 42) and its x labels (10 + 14), a row of 18; the entries from the plot's
    // left, each a dot of 8, a gap of 6 and its label (no width here), 16 apart
    const Vec2 bigEntry(14 + 16 + 7, 178 + 24 + 9);
    h.Click(bigEntry);
    h.Step(90);   // the view eases to the data left
    ESIA_CHECK(Near(l.yMax, 1.06f, 1e-2f));
    h.Click(bigEntry);
    h.Step(90);
    ESIA_CHECK(Near(l.yMax, 106.0f, 1e-2f));
}

// A drag pans, the wheel zooms around the mouse and is the plot's (the window does not scroll), a double click fits the
// data again; elsewhere the wheel scrolls the window
ESIA_TEST(UiPlot, PanZoomAndTheWheel)
{
    Harness h;
    std::vector<float> ys(101);
    for (int i = 0; i <= 100; ++i)
        ys[(std::size_t)i] = std::sin((float)i * 0.1f);
    ui::PlotLimits l;
    h.body = [&] {
        if (ui::BeginPlot("p", {.height = 220, .flags = ui::PlotFlags_NoLegend}))
        {
            l = ui::GetPlotLimits();
            ui::PlotLine("sin", ys);
            ui::EndPlot();
        }
        h.ctx.ItemSize({10, 800});   // the window scrolls
    };
    h.Step(2);
    ESIA_CHECK(l.xMin == 0.0f && l.xMax == 100.0f);
    Window* w = h.ctx.FindWindowByName("W");

    // the wheel over the plot: zoomed in around the mouse, the window still
    h.Move({300, 100});
    h.Wheel(2);
    h.Step();
    ESIA_CHECK(l.xMax - l.xMin < 90.0f && l.xMin > 0.0f && l.xMax < 100.0f);
    ESIA_CHECK(w->Scroll().y == 0.0f);
    const float zoomed = l.xMax - l.xMin;

    // a drag to the right: earlier x comes into view, the zoom kept
    const float before = l.xMin;
    h.Down();
    h.Move({340, 100});
    h.Move({400, 100});
    h.Up();
    h.Step();
    ESIA_CHECK(l.xMin < before && Near(l.xMax - l.xMin, zoomed));

    // a double click: the data fitted again (eased)
    h.Click({300, 100});
    h.Click({300, 100});
    h.Step(90);
    ESIA_CHECK(Near(l.xMin, 0.0f, 1e-2f) && Near(l.xMax, 100.0f, 1e-2f));

    // below the plot the wheel scrolls the window
    h.Move({300, 300});
    h.Wheel(-2);
    h.Step(40);
    ESIA_CHECK(w->Scroll().y > 0.0f);
}

// NoPanZoom: the wheel is the window's, a drag changes nothing
ESIA_TEST(UiPlot, NoPanZoomLeavesTheWheelToTheWindow)
{
    Harness h;
    const float ys[] = {1, 3, 2, 5};
    ui::PlotLimits l;
    h.body = [&] {
        if (ui::BeginPlot("p", {.height = 220, .flags = ui::PlotFlags_NoPanZoom}))
        {
            l = ui::GetPlotLimits();
            ui::PlotLine("y", ys);
            ui::EndPlot();
        }
        h.ctx.ItemSize({10, 800});
    };
    h.Step(2);
    const ui::PlotLimits fitted = l;
    h.Move({300, 100});
    h.Down();
    h.Move({400, 150});
    h.Up();
    h.Wheel(-2);
    h.Step(40);
    ESIA_CHECK(l.xMin == fitted.xMin && l.xMax == fitted.xMax && l.yMin == fitted.yMin && l.yMax == fitted.yMax);
    ESIA_CHECK(h.ctx.FindWindowByName("W")->Scroll().y > 0.0f);
}

// The donut takes its room (the ring and the legend beside it); no values, zeros and non-finite values draw a gray ring
ESIA_TEST(UiPieChart, TakesItsRoomWithAnyValues)
{
    Harness h;
    const float values[] = {3, 0, NAN, 1};
    const std::string_view labels[] = {"a", "b", "c", "d"};
    Vec2 after[3];
    h.body = [&] {
        ui::PieChart("a", values, labels, {.size = 120});
        after[0] = h.ctx.CursorPos();
        ui::PieChart("b", std::span<const float>(), {}, {.size = 60, .center = "none"});
        after[1] = h.ctx.CursorPos();
        const float zeros[] = {0, 0};
        ui::PieChart("c", zeros, {}, {.size = 60});
        after[2] = h.ctx.CursorPos();
    };
    h.Step(2);
    const float spacing = h.ctx.Metrics().itemSpacing.y;
    ESIA_CHECK(after[0].y == 120.0f + spacing);
    ESIA_CHECK(after[1].y == after[0].y + 60.0f + spacing);
    ESIA_CHECK(after[2].y == after[1].y + 60.0f + spacing);
    h.Move({60, 10});   // over the first segment
    h.Step(30);
}
