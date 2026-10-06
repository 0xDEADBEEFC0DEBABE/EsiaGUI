// Esia UI - plots (lines, areas, scatter, bars and histograms over axes; a legend, a crosshair, pan and zoom) and the
// donut chart. The series are collected between BeginPlot and EndPlot; EndPlot fits the view, draws the axes and the
// series and answers the mouse (ui.hpp, "plots"; docs/UI_WIDGETS.md section 12).
#include "ui_internal.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <string>
#include <vector>

namespace esia::ui
{
    using namespace detail;

    float detail::PlotTicks(float lo, float hi, int maxTicks, std::vector<float>& out)
    {
        out.clear();
        if (!(hi > lo) || maxTicks < 1 || !std::isfinite(lo) || !std::isfinite(hi))
            return 0.0f;
        // 1, 2 or 5 times a power of ten: the step of at most maxTicks ticks
        const double raw = ((double)hi - (double)lo) / (double)maxTicks;
        const double mag = std::pow(10.0, std::floor(std::log10(raw)));
        const double n = raw / mag;
        const double step = (n <= 1.0 ? 1.0 : n <= 2.0 ? 2.0 : n <= 5.0 ? 5.0 : 10.0) * mag;
        const double k0 = std::ceil((double)lo / step - 1e-9), k1 = std::floor((double)hi / step + 1e-9);
        for (double k = k0; k <= k1 && out.size() < 64; k += 1.0)
            out.push_back(k == 0.0 ? 0.0f : (float)(k * step));
        return (float)step;
    }

    namespace
    {
        enum class SeriesKind : std::uint8_t { Line, Scatter, Bars, Histogram };

        struct Series
        {
            SeriesKind kind = SeriesKind::Line;
            std::string label;
            Id key = 0;                  // the label's hash: the legend's hidden set
            Color color;
            PlotStyle style;
            std::vector<Vec2> pts;       // data space; bars: (category or bin center, value)
            float width = 0.0f;          // a histogram's bar width (data units)
            bool sorted = true;          // x never decreases: a binary search finds the point under the mouse,
                                         // a dense line keeps a few points per pixel column
            bool hidden = false;
        };

        struct PlotState
        {
            bool user = false;           // panned or zoomed: the view is the user's until a double click
            bool drawn = false;          // x0 .. y1 hold a view
            float x0 = 0.0f, x1 = 1.0f, y0 = 0.0f, y1 = 1.0f;   // the view drawn last
            bool dragging = false;
            Vec2 dragFrom;
            float d0[4] = {};            // the view when the drag started
            std::vector<Id> hidden;      // series the legend hid (their labels' hashes)
        };

        struct PlotFrame
        {
            Id id = 0;
            PlotOptions o;
            Rect outer;
            int count = 0;               // series of g_series used this frame (their storage stays)
            int colorIndex = 0;
            bool stylePushed = false;
        };
        thread_local PlotFrame g_plot;
        thread_local bool g_inPlot = false;
        thread_local std::vector<Series> g_series;
        thread_local std::vector<float> g_ticks;
        thread_local std::vector<Vec2> g_screen;
        thread_local char g_text[96];

        Color PlotColor(int i)
        {
            const Palette& c = C();
            const Color colors[] = {c.blue, c.orange, c.green, c.pink, c.purple, c.teal, c.yellow, c.indigo, c.red, c.mint};
            return colors[i % 10];
        }

        Series* NewSeries(SeriesKind kind, std::string_view label, const PlotStyle& style)
        {
            ESIA_ASSERT(g_inPlot && "a plot series outside BeginPlot / EndPlot");
            if (!g_inPlot)
                return nullptr;
            PlotFrame& f = g_plot;
            if ((int)g_series.size() <= f.count)
                g_series.emplace_back();
            Series& s = g_series[(std::size_t)f.count++];
            s.kind = kind;
            s.label.assign(label);
            s.key = HashString(label, 0x504C4F54u);
            s.style = style;
            // every series takes its color's turn, shown or not: hiding one leaves the others' colors as they were
            s.color = style.color.a > 0.0f ? style.color : PlotColor(f.colorIndex);
            ++f.colorIndex;
            s.pts.clear();
            s.width = 0.0f;
            s.sorted = true;
            const PlotState& st = Ctx().State<PlotState>(f.id);
            s.hidden = std::find(st.hidden.begin(), st.hidden.end(), s.key) != st.hidden.end();
            return &s;
        }

        void AddPoints(Series& s, std::span<const float> xs, std::span<const float> ys)
        {
            const std::size_t n = std::min(xs.size(), ys.size());
            s.pts.reserve(n);
            for (std::size_t i = 0; i < n; ++i)
                if (std::isfinite(xs[i]) && std::isfinite(ys[i]))
                {
                    if (!s.pts.empty() && xs[i] < s.pts.back().x)
                        s.sorted = false;
                    s.pts.emplace_back(xs[i], ys[i]);
                }
        }

        const char* Format(const char* fmt, float v)
        {
            std::snprintf(g_text, sizeof(g_text), fmt && *fmt ? fmt : "%g", (double)v);
            return g_text;
        }

        // the index of the point of a line nearest to x
        int Nearest(const Series& s, float x)
        {
            const int n = (int)s.pts.size();
            if (n == 0)
                return -1;
            if (s.sorted)
            {
                const auto it = std::lower_bound(s.pts.begin(), s.pts.end(), x, [](const Vec2& p, float v) { return p.x < v; });
                int i = (int)(it - s.pts.begin());
                if (i >= n)
                    return n - 1;
                if (i > 0 && x - s.pts[(std::size_t)i - 1].x < s.pts[(std::size_t)i].x - x)
                    --i;
                return i;
            }
            int best = 0;
            for (int i = 1; i < n; ++i)
                if (std::fabs(s.pts[(std::size_t)i].x - x) < std::fabs(s.pts[(std::size_t)best].x - x))
                    best = i;
            return best;
        }
    }

    // ============================================================== series
    bool BeginPlot(std::string_view id, const PlotOptions& o)
    {
        ESIA_ASSERT(!g_inPlot && "BeginPlot inside a plot");
        if (g_inPlot)
            return false;
        Context& c = Ctx();
        const bool stylePushed = TakeNextStyle();
        const Vec2 size(o.width > 0.0f ? Sc(o.width) : AvailableWidth(), Sc(o.height));
        const Rect outer = Rect::FromSize(c.CursorPos(), size);
        c.ItemSize(size);
        if (!outer.Overlaps(c.WindowDrawList().ClipRect()))
        {
            if (stylePushed)
                PopStyle();
            return false;   // out of view: nothing to collect (no EndPlot)
        }
        PlotFrame& f = g_plot;
        f.stylePushed = stylePushed;
        f.id = c.GetId(id);
        f.o = o;
        f.outer = outer;
        f.count = 0;
        f.colorIndex = 0;
        g_inPlot = true;
        return true;
    }

    PlotLimits GetPlotLimits()
    {
        ESIA_ASSERT(g_inPlot && "GetPlotLimits outside BeginPlot / EndPlot");
        const PlotOptions& o = g_plot.o;
        PlotLimits l;
        if (g_inPlot)
        {
            const PlotState& st = Ctx().State<PlotState>(g_plot.id);
            if (st.drawn)
                l = {st.x0, st.x1, st.y0, st.y1};
            else
            {
                if (o.xMax > o.xMin)
                    l.xMin = o.xMin, l.xMax = o.xMax;
                if (o.yMax > o.yMin)
                    l.yMin = o.yMin, l.yMax = o.yMax;
            }
        }
        return l;
    }

    void PlotLine(std::string_view label, std::span<const float> ys, const PlotStyle& style)
    {
        Series* s = NewSeries(SeriesKind::Line, label, style);
        if (!s)
            return;
        s->pts.reserve(ys.size());
        for (std::size_t i = 0; i < ys.size(); ++i)
            if (std::isfinite(ys[i]))
                s->pts.emplace_back(style.xStart + style.xStep * (float)i, ys[i]);
        s->sorted = style.xStep >= 0.0f;
    }

    void PlotLine(std::string_view label, std::span<const float> xs, std::span<const float> ys, const PlotStyle& style)
    {
        if (Series* s = NewSeries(SeriesKind::Line, label, style))
            AddPoints(*s, xs, ys);
    }

    void PlotScatter(std::string_view label, std::span<const float> xs, std::span<const float> ys, const PlotStyle& style)
    {
        if (Series* s = NewSeries(SeriesKind::Scatter, label, style))
            AddPoints(*s, xs, ys);
    }

    void PlotBars(std::string_view label, std::span<const float> values, const PlotStyle& style)
    {
        Series* s = NewSeries(SeriesKind::Bars, label, style);
        if (!s)
            return;
        s->pts.reserve(values.size());
        for (std::size_t i = 0; i < values.size(); ++i)
            s->pts.emplace_back((float)i, std::isfinite(values[i]) ? values[i] : 0.0f);
    }

    void PlotHistogram(std::string_view label, std::span<const float> samples, int bins, const PlotStyle& style)
    {
        Series* s = NewSeries(SeriesKind::Histogram, label, style);
        if (!s)
            return;
        float lo = std::numeric_limits<float>::max(), hi = -std::numeric_limits<float>::max();
        std::size_t n = 0;
        for (const float v : samples)
            if (std::isfinite(v))
            {
                lo = std::min(lo, v);
                hi = std::max(hi, v);
                ++n;
            }
        if (n == 0)
            return;
        if (bins <= 0)
            bins = std::clamp((int)std::lround(std::sqrt((double)n)), 1, 64);
        if (!(hi > lo))
        {
            lo -= 0.5f;
            hi += 0.5f;
            bins = 1;
        }
        const float w = (hi - lo) / (float)bins;
        s->pts.resize((std::size_t)bins);
        for (int b = 0; b < bins; ++b)
            s->pts[(std::size_t)b] = Vec2(lo + w * ((float)b + 0.5f), 0.0f);
        for (const float v : samples)
            if (std::isfinite(v))
                s->pts[(std::size_t)std::clamp((int)((v - lo) / w), 0, bins - 1)].y += 1.0f;
        s->width = w;
    }

    // ============================================================== the plot
    void EndPlot()
    {
        ESIA_ASSERT(g_inPlot && "EndPlot without BeginPlot");
        if (!g_inPlot)
            return;
        g_inPlot = false;
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const Theme& t = T();
        const Palette& pc = t.colors;
        const InputState& in = c.Input();
        PlotFrame& f = g_plot;
        const PlotOptions& o = f.o;
        PlotState& st = c.State<PlotState>(f.id);
        const std::span<Series> series(g_series.data(), (std::size_t)f.count);

        // ---- the data's range
        int barSeries = 0;
        float dx0 = std::numeric_limits<float>::max(), dx1 = -dx0, dy0 = dx0, dy1 = -dx0;
        bool any = false, zeroBased = false, scatter = false;
        for (const Series& s : series)
        {
            if (s.hidden || s.pts.empty())
                continue;
            any = true;
            const float half = s.kind == SeriesKind::Bars ? 0.5f : s.kind == SeriesKind::Histogram ? s.width * 0.5f : 0.0f;
            barSeries += s.kind == SeriesKind::Bars ? 1 : 0;
            zeroBased = zeroBased || s.kind == SeriesKind::Bars || s.kind == SeriesKind::Histogram || s.style.fill;
            scatter = scatter || s.kind == SeriesKind::Scatter;
            for (const Vec2& p : s.pts)
            {
                dx0 = std::min(dx0, p.x - half);
                dx1 = std::max(dx1, p.x + half);
                dy0 = std::min(dy0, p.y);
                dy1 = std::max(dy1, p.y);
            }
        }
        if (!any)
        {
            dx0 = dy0 = 0.0f;
            dx1 = dy1 = 1.0f;
        }
        if (zeroBased)
        {
            dy0 = std::min(dy0, 0.0f);
            dy1 = std::max(dy1, 0.0f);
        }
        if (!(dx1 > dx0))
        {
            dx0 -= 0.5f;
            dx1 += 0.5f;
        }
        if (!(dy1 > dy0))
        {
            const float d = std::max(std::fabs(dy0) * 0.1f, 1.0f);
            dy0 -= d;
            dy1 += d;
        }
        {
            const float py = (dy1 - dy0) * 0.06f;
            dy1 += py;
            if (!(zeroBased && dy0 == 0.0f))
                dy0 -= py;
            if (scatter)
            {
                const float px = (dx1 - dx0) * 0.04f;
                dx0 -= px;
                dx1 += px;
            }
        }

        // ---- the view: the given limits, the user's (panned, zoomed), or the data's, eased toward as it changes
        const bool fixX = o.xMax > o.xMin, fixY = o.yMax > o.yMin;
        const float fit[4] = {fixX ? o.xMin : dx0, fixX ? o.xMax : dx1, fixY ? o.yMin : dy0, fixY ? o.yMax : dy1};
        float view[4];
        for (int i = 0; i < 4; ++i)
        {
            const Id aid = Salt(f.id, 0x9100u + (std::uint32_t)i);
            const bool fixed = i < 2 ? fixX : fixY;
            if (st.user && !fixed)
            {
                view[i] = i == 0 ? st.x0 : i == 1 ? st.x1 : i == 2 ? st.y0 : st.y1;
                AnimSet(aid, view[i]);   // a double click eases from here to the data
            }
            else
                view[i] = fixed ? fit[i] : Anim(aid, fit[i], SpringStd(), fit[i]);
        }

        // ---- room for the tick labels and the legend under them (where Swift Charts has it: never over the data),
        // then the area the series fill
        const text::FontRef tf = Font(TextStyle::Caption1);
        const float labelH = std::max(MeasureText(tf, "0").y, Sc(10));
        const Rect& outer = f.outer;
        const bool legend = !(o.flags & PlotFlags_NoLegend) && f.count > 0;
        const float dot = Sc(8), dotGap = Sc(6), entryGap = Sc(16), legendRowH = labelH + Sc(8);
        const auto entryWidth = [&](const Series& s) { return dot + dotGap + MeasureText(tf, VisibleLabel(s.label)).x; };
        int legendRows = 0;
        if (legend)
        {
            float x = 0.0f;
            legendRows = 1;
            for (const Series& s : series)
            {
                const float w = entryWidth(s);
                if (x > 0.0f && x + w > outer.Width())
                {
                    ++legendRows;
                    x = 0.0f;
                }
                x += w + entryGap;
            }
        }
        const float bottom = labelH + Sc(8) + (legend ? Sc(6) + legendRowH * (float)legendRows : 0.0f);
        const int maxYTicks = std::max(2, (int)((outer.Height() - bottom) / Sc(34)));
        PlotTicks(view[2], view[3], maxYTicks, g_ticks);
        float labelW = 0.0f;
        for (const float v : g_ticks)
            labelW = std::max(labelW, MeasureText(tf, Format(o.yFormat, v)).x);
        const Rect area(outer.min.x + labelW + Sc(8), outer.min.y + Sc(2), outer.max.x - Sc(2), outer.max.y - bottom);
        if (area.Width() < 8.0f || area.Height() < 8.0f)
        {
            if (f.stylePushed)
                PopStyle();
            return;
        }

        // ---- the mouse: a drag pans, the wheel zooms around the mouse, a double click fits the data again
        const bool panZoom = !(o.flags & PlotFlags_NoPanZoom) && !(fixX && fixY);
        c.ItemAdd(f.id, area, panZoom ? ItemFlags_Wheel : ItemFlags_None);
        const ButtonResult b = c.ButtonBehavior(f.id, area, ButtonFlags_PressOnClick);
        if (panZoom)
        {
            if (b.pressed && b.clicks == 2)
            {
                st.user = false;
                st.dragging = false;
            }
            else if (b.pressed)
            {
                st.dragging = true;
                st.dragFrom = in.MousePos();
                std::copy(view, view + 4, st.d0);
            }
            if (st.dragging && b.held && in.MouseValid())
            {
                const Vec2 d = in.MousePos() - st.dragFrom;
                if (st.user || std::max(std::fabs(d.x), std::fabs(d.y)) > Sc(3))
                {
                    st.user = true;
                    const float kx = (st.d0[1] - st.d0[0]) / area.Width(), ky = (st.d0[3] - st.d0[2]) / area.Height();
                    if (!fixX)
                    {
                        view[0] = st.d0[0] - d.x * kx;
                        view[1] = st.d0[1] - d.x * kx;
                    }
                    if (!fixY)
                    {
                        view[2] = st.d0[2] + d.y * ky;
                        view[3] = st.d0[3] + d.y * ky;
                    }
                    c.SetMouseCursor(MouseCursor::ResizeAll);
                }
            }
            else if (!b.held)
                st.dragging = false;
            const float wheel = in.Wheel().y;
            if (b.hovered && wheel != 0.0f && in.MouseValid() && !st.dragging)
            {
                const float z = std::pow(0.85f, wheel);   // away from the user zooms in
                const Vec2 mp = in.MousePos();
                const float mx = view[0] + (mp.x - area.min.x) / area.Width() * (view[1] - view[0]);
                const float my = view[2] + (area.max.y - mp.y) / area.Height() * (view[3] - view[2]);
                if (!fixX)
                {
                    view[0] = mx + (view[0] - mx) * z;
                    view[1] = mx + (view[1] - mx) * z;
                }
                if (!fixY)
                {
                    view[2] = my + (view[2] - my) * z;
                    view[3] = my + (view[3] - my) * z;
                }
                st.user = true;
            }
        }
        st.drawn = true;
        st.x0 = view[0];
        st.x1 = view[1];
        st.y0 = view[2];
        st.y1 = view[3];
        const float vx0 = view[0], vy0 = view[2];
        const float sx = area.Width() / std::max(view[1] - view[0], 1e-20f), sy = area.Height() / std::max(view[3] - view[2], 1e-20f);
        const auto X = [&](float x) { return area.min.x + (x - vx0) * sx; };
        const auto Y = [&](float y) { return area.max.y - (y - vy0) * sy; };

        Painter p = GetPainter();
        const float px = 1.0f / std::max(c.Scale(), 1e-3f);   // one physical pixel
        p.Rect(area, Style().Radius(Sc(8)).Fill(pc.quaternaryFill));

        // ---- the grid and the tick labels
        PlotTicks(view[2], view[3], maxYTicks, g_ticks);
        for (const float v : g_ticks)
        {
            const float y = std::round(Y(v) / px) * px;
            if (!(o.flags & PlotFlags_NoGrid))
                p.HLine(area.min.x, area.max.x, y, pc.label.Fade(v == 0.0f ? 0.20f : 0.07f));
            const char* s = Format(o.yFormat, v);
            const Vec2 ts = MeasureText(tf, s);
            p.Text(Vec2(area.min.x - Sc(6) - ts.x, std::floor(y - ts.y * 0.5f)), tf, pc.secondaryLabel, s);
        }
        const bool categories = !o.categories.empty();
        if (categories)
        {
            // a label under each category in view; every n-th one when they would overlap
            float widest = 0.0f;
            for (const std::string_view l : o.categories)
                widest = std::max(widest, MeasureText(tf, l).x);
            const int every = std::max(1, (int)std::ceil((widest + Sc(8)) / std::max(sx, 1e-3f)));
            const int first = std::max(0, (int)std::ceil(view[0])), last = std::min((int)o.categories.size() - 1, (int)std::floor(view[1]));
            for (int i = first; i <= last; ++i)
            {
                if (i % every != 0)
                    continue;
                const std::string_view l = o.categories[(std::size_t)i];
                const Vec2 ts = MeasureText(tf, l);
                p.Text(Vec2(std::floor(X((float)i) - ts.x * 0.5f), area.max.y + Sc(5)), tf, pc.secondaryLabel, l);
            }
        }
        else
        {
            PlotTicks(view[0], view[1], std::max(2, (int)(area.Width() / Sc(80))), g_ticks);
            for (const float v : g_ticks)
            {
                const float x = std::round(X(v) / px) * px;
                if (!(o.flags & PlotFlags_NoGrid))
                    p.FillRect(Rect(x - px * 0.5f, area.min.y, x + px * 0.5f, area.max.y), pc.label.Fade(v == 0.0f ? 0.20f : 0.07f));
                const char* s = Format(o.xFormat, v);
                const Vec2 ts = MeasureText(tf, s);
                p.Text(Vec2(std::floor(x - ts.x * 0.5f), area.max.y + Sc(5)), tf, pc.secondaryLabel, s);
            }
        }

        // ---- what is under the mouse
        const bool hoverOn = b.hovered && !(o.flags & PlotFlags_NoHover) && !st.dragging && in.MouseValid() && area.Contains(in.MousePos());
        const float mouseX = hoverOn ? view[0] + (in.MousePos().x - area.min.x) / sx : 0.0f;
        const int hoverCategory = hoverOn ? (int)std::lround(mouseX) : -1;

        // ---- the series, clipped to the area
        p.PushClip(area);
        const float baseline = std::clamp(Y(0.0f), area.min.y, area.max.y);
        int barSlot = 0;
        for (Series& s : series)
        {
            if (s.hidden || s.pts.empty())
                continue;
            const Color col = s.color;
            if (s.kind == SeriesKind::Line)
            {
                // to the screen; past four points a pixel column, a sorted line keeps each column's first, lowest,
                // highest and last (its shape at any zoom, a million points cost what the width does)
                g_screen.clear();
                const bool decimate = s.sorted && (float)s.pts.size() > area.Width() / px * 4.0f;
                int column = std::numeric_limits<int>::min();
                Vec2 first, lo, hi, last;
                std::size_t inColumn = 0;
                const auto flush = [&] {
                    if (inColumn == 0)
                        return;
                    // in their order along x (a point twice in a row is dropped by the path)
                    g_screen.push_back(first);
                    g_screen.push_back(lo.x <= hi.x ? lo : hi);
                    g_screen.push_back(lo.x <= hi.x ? hi : lo);
                    g_screen.push_back(last);
                };
                for (const Vec2& d : s.pts)
                {
                    const Vec2 q(X(d.x), Y(d.y));
                    if (!decimate)
                    {
                        g_screen.push_back(q);
                        continue;
                    }
                    const int col0 = (int)std::floor(q.x / px);
                    if (col0 != column)
                    {
                        flush();
                        column = col0;
                        first = lo = hi = q;
                        inColumn = 0;
                    }
                    if (q.y > lo.y)
                        lo = q;
                    if (q.y < hi.y)
                        hi = q;
                    last = q;
                    ++inColumn;
                }
                flush();
                const int n = (int)g_screen.size();
                const std::uint32_t flags = s.style.smooth ? PolylineFlags_Smooth : PolylineFlags_None;
                if (s.style.fill && n >= 2)
                    p.Area(g_screen.data(), n, baseline, Paint::Linear(col.Fade(0.30f), col.Fade(0.0f), 90), flags);
                if (n >= 2)
                    p.Polyline(g_screen.data(), n, Sc(s.style.thickness), Style().Fill(col), flags);
                else if (n == 1)
                    p.Circle(g_screen[0], Sc(s.style.thickness) * 1.5f, Style().Fill(col));
            }
            else if (s.kind == SeriesKind::Scatter)
            {
                const float r = Sc(s.style.size);
                for (const Vec2& d : s.pts)
                {
                    const Vec2 q(X(d.x), Y(d.y));
                    if (q.x < area.min.x - r || q.x > area.max.x + r || q.y < area.min.y - r || q.y > area.max.y + r)
                        continue;
                    p.Circle(q, r, Style().Fill(col.Fade(0.85f)));
                }
            }
            else
            {
                // bars: the series side by side in each category; a histogram's bins as they are
                float w, offset;
                if (s.kind == SeriesKind::Bars)
                {
                    w = 0.8f / (float)std::max(barSeries, 1);
                    offset = -0.4f + w * ((float)barSlot + 0.5f);
                    ++barSlot;
                }
                else
                {
                    w = s.width;
                    offset = 0.0f;
                }
                const float inset = std::min(w * sx * 0.08f, Sc(2));
                for (const Vec2& d : s.pts)
                {
                    const float xc = X(d.x + offset), half = w * sx * 0.5f - inset;
                    if (xc + half < area.min.x || xc - half > area.max.x || half <= 0.0f)
                        continue;
                    const float top = Y(d.y);
                    Rect r(xc - half, std::min(top, baseline), xc + half, std::max(top, baseline));
                    if (r.Height() < px)
                        r.max.y = r.min.y + px;
                    const float rad = std::min(Sc(4), half);
                    const bool hot = s.kind == SeriesKind::Bars ? (int)std::lround(d.x) == hoverCategory
                                                                : hoverOn && std::fabs(mouseX - d.x) <= s.width * 0.5f;
                    Style bs = d.y >= 0.0f ? Style().Radius(rad, rad, 0, 0) : Style().Radius(0, 0, rad, rad);
                    p.Rect(r, bs.Fill(hot ? col.Lighter(0.18f) : col));
                }
            }
        }
        p.PopClip();

        // ---- the legend: a dot and the label per series, a click hides or shows it
        if (legend)
        {
            float x = outer.min.x, y = area.max.y + labelH + Sc(14);
            for (int i = 0; i < f.count; ++i)
            {
                Series& s = series[(std::size_t)i];
                const float w = entryWidth(s);
                if (x > outer.min.x && x + w > outer.max.x)
                {
                    x = outer.min.x;
                    y += legendRowH;
                }
                const Rect entry(std::max(x - Sc(4), outer.min.x), y, x + w + Sc(4), y + legendRowH);
                const Interaction it = InteractImpl(Salt(f.id, 0x4C00u + (std::uint32_t)i), entry, InteractFlags_None);
                if (it.pressed)
                {
                    auto h = std::find(st.hidden.begin(), st.hidden.end(), s.key);
                    if (h != st.hidden.end())
                        st.hidden.erase(h);
                    else
                        st.hidden.push_back(s.key);
                }
                const Vec2 dc(x + dot * 0.5f, entry.Center().y);
                if (s.hidden)
                    p.Circle(dc, dot * 0.5f - Sc(0.75f), Style().Fill(Color::Clear()).Stroke(Sc(1.5f), s.color.Fade(0.6f)));
                else
                    p.Circle(dc, dot * 0.5f, Style().Fill(s.color));
                const std::string_view label = VisibleLabel(s.label);
                const Vec2 ts = MeasureText(tf, label);
                const Color lc = Lerp(pc.secondaryLabel, pc.label, it.hover);
                p.Text(Vec2(x + dot + dotGap, std::floor(entry.Center().y - ts.y * 0.5f)), tf, s.hidden ? lc.Fade(0.5f) : lc, label);
                x += w + entryGap;
            }
        }

        // ---- the crosshair and the readout
        if (hoverOn && any)
        {
            const Vec2 mp = in.MousePos();
            const text::FontRef rf = Font(TextStyle::Footnote);
            const text::FontRef rb = Font(FontWeight::Semibold, T().type.size[(int)TextStyle::Footnote]);
            struct Row
            {
                Color color;
                const std::string* label;
                float value;
            };
            Row rows[16];
            int rowCount = 0;
            float snapX = mouseX;
            bool snapped = false, scatterHit = false;
            Vec2 scatterPoint;
            for (const Series& s : series)
            {
                if (s.hidden || s.pts.empty() || rowCount >= 16)
                    continue;
                if (s.kind == SeriesKind::Scatter)
                {
                    // the nearest point within reach of the mouse
                    float best = Sc(12) * Sc(12);
                    for (const Vec2& d : s.pts)
                    {
                        const Vec2 q(X(d.x), Y(d.y));
                        const float dd = (q.x - mp.x) * (q.x - mp.x) + (q.y - mp.y) * (q.y - mp.y);
                        if (dd < best)
                        {
                            best = dd;
                            scatterPoint = d;
                            scatterHit = true;
                        }
                    }
                    if (scatterHit)
                    {
                        p.Circle(Vec2(X(scatterPoint.x), Y(scatterPoint.y)), Sc(s.style.size) + Sc(3), Style().Fill(Color::Clear()).Stroke(Sc(1.5f), s.color));
                        rows[rowCount++] = {s.color, &s.label, scatterPoint.y};
                        snapX = scatterPoint.x;
                        snapped = true;
                    }
                    continue;
                }
                if (s.kind == SeriesKind::Bars)
                {
                    if (hoverCategory >= 0 && hoverCategory < (int)s.pts.size())
                    {
                        rows[rowCount++] = {s.color, &s.label, s.pts[(std::size_t)hoverCategory].y};
                        snapX = (float)hoverCategory;
                        snapped = true;
                    }
                    continue;
                }
                if (s.kind == SeriesKind::Histogram)
                {
                    for (const Vec2& d : s.pts)
                        if (std::fabs(mouseX - d.x) <= s.width * 0.5f)
                        {
                            rows[rowCount++] = {s.color, &s.label, d.y};
                            snapX = d.x;
                            snapped = true;
                        }
                    continue;
                }
                const int i = Nearest(s, snapped ? snapX : mouseX);
                if (i < 0)
                    continue;
                const Vec2 d = s.pts[(std::size_t)i];
                if (!snapped)
                {
                    snapX = d.x;
                    snapped = true;
                }
                rows[rowCount++] = {s.color, &s.label, d.y};
                const Vec2 q(X(d.x), Y(d.y));
                if (area.Contains(q))
                {
                    p.Circle(q, Sc(4.5f), Style().Fill(pc.background));
                    p.Circle(q, Sc(3), Style().Fill(s.color));
                }
            }
            if (rowCount > 0)
            {
                const float cx = std::round(X(snapX) / px) * px;
                if (cx >= area.min.x && cx <= area.max.x)
                    p.FillRect(Rect(cx - px * 0.5f, area.min.y, cx + px * 0.5f, area.max.y), pc.label.Fade(0.28f));
                // the header: the category or x (scatter: x, y); then a row per series
                const std::string* header = nullptr;
                std::string headerText;
                if (categories && hoverCategory >= 0 && hoverCategory < (int)o.categories.size())
                    headerText.assign(o.categories[(std::size_t)hoverCategory]);
                else
                    headerText.assign(Format(o.xFormat, snapX));
                header = &headerText;
                const float pad = Sc(8), gap = Sc(6);   // the legend's dots
                float w = MeasureText(rb, *header).x;
                const float lineH = std::max(MeasureText(rf, "0").y, Sc(14));
                for (int i = 0; i < rowCount; ++i)
                {
                    const std::string_view label = VisibleLabel(*rows[i].label);
                    const float lw = MeasureText(rf, label).x, vw = MeasureText(rb, Format(o.yFormat, rows[i].value)).x;
                    w = std::max(w, dot + gap + lw + Sc(14) + vw);
                }
                const Vec2 size(w + pad * 2.0f, lineH * (float)(rowCount + 1) + pad * 2.0f);
                Vec2 at(mp.x + Sc(14), mp.y + Sc(14));
                if (at.x + size.x > outer.max.x)
                    at.x = mp.x - Sc(14) - size.x;
                if (at.y + size.y > outer.max.y)
                    at.y = std::max(outer.min.y, mp.y - Sc(14) - size.y);
                const Rect box = Rect::FromSize(Vec2(std::floor(at.x), std::floor(at.y)), size);
                ScopedUnclip unclip(box, ShadowExtent(Sc(14), Vec2(0, Sc(4))));
                p.Rect(box, Style().Radius(Sc(10)).Fill(pc.secondaryBackground.Fade(0.94f)).Shadow(pc.shadow, Sc(14), Vec2(0, Sc(4))));
                float y = box.min.y + pad;
                p.Text(Vec2(box.min.x + pad, y), rb, pc.label, *header);
                for (int i = 0; i < rowCount; ++i)
                {
                    y += lineH;
                    p.Circle(Vec2(box.min.x + pad + dot * 0.5f, y + lineH * 0.5f), dot * 0.5f, Style().Fill(rows[i].color));
                    p.Text(Vec2(box.min.x + pad + dot + gap, y), rf, pc.secondaryLabel, VisibleLabel(*rows[i].label));
                    const char* v = Format(o.yFormat, rows[i].value);
                    const float vw = MeasureText(rb, v).x;
                    p.Text(Vec2(box.max.x - pad - vw, y), rb, pc.label, v);
                }
            }
        }

        if (f.stylePushed)
            PopStyle();
    }

    // ============================================================== the donut
    void PieChart(std::string_view id, std::span<const float> values, std::span<const std::string_view> labels, const PieChartOptions& o)
    {
        ItemScope scope;
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const Palette& pc = C();
        const Id pid = c.GetId(id);
        double total = 0.0;
        for (const float v : values)
            total += v > 0.0f && std::isfinite(v) ? v : 0.0f;

        const float d = Sc(o.size);
        const text::FontRef lf = Font(TextStyle::Footnote);
        const text::FontRef lb = Font(FontWeight::Semibold, T().type.size[(int)TextStyle::Footnote]);
        const bool legend = o.legend && !labels.empty();
        const float rowH = std::max(MeasureText(lf, "0").y, Sc(14)) + Sc(6), dot = Sc(8);
        float labelW = 0.0f;
        if (legend)
            for (const std::string_view l : labels)
                labelW = std::max(labelW, MeasureText(lf, l).x);
        const float pctW = MeasureText(lb, "100%").x;
        const float legendW = legend ? dot + Sc(8) + labelW + Sc(14) + pctW : 0.0f;
        const int rows = legend ? (int)std::min(labels.size(), values.size()) : 0;
        const Vec2 size(d + (legend ? Sc(22) + legendW : 0.0f), std::max(d, rowH * (float)rows));
        const Vec2 pos = c.CursorPos();
        c.ItemSize(size);
        const Rect r = Rect::FromSize(pos, size);
        const Rect ringRect = Rect::FromSize(Vec2(pos.x, pos.y + (size.y - d) * 0.5f), Vec2(d, d));
        if (!c.ItemAdd(pid, r))
            return;

        const Vec2 center = ringRect.Center();
        const float outerR = d * 0.5f - Sc(4);   // the room a hovered segment grows into
        const float ring = outerR * std::clamp(o.thickness, 0.15f, 0.7f);
        const float shown = Anim(pid, 0x70, 1.0f, SpringStd(), 0.0f);   // the ring grows round when it appears
        const float start = -kTau * 0.25f;   // the top, clockwise

        // the segment under the mouse (the ring, or its row in the legend)
        int hovered = -1;
        const bool hover = c.ItemHoverable(pid, r);
        if (hover && total > 0.0)
        {
            const Vec2 mp = c.Input().MousePos();
            const Vec2 v = mp - center;
            const float dist = std::sqrt(v.x * v.x + v.y * v.y);
            if (dist >= outerR - ring - Sc(2) && dist <= outerR + Sc(4))
            {
                float a = std::atan2(v.y, v.x) - start;
                while (a < 0.0f)
                    a += kTau;
                float acc = 0.0f;
                for (std::size_t i = 0; i < values.size(); ++i)
                {
                    const float share = values[i] > 0.0f && std::isfinite(values[i]) ? (float)(values[i] / total) : 0.0f;
                    if (a >= acc * kTau && a < (acc + share) * kTau)
                        hovered = (int)i;
                    acc += share;
                }
            }
            else if (legend && mp.x >= ringRect.max.x)
            {
                const float top = r.min.y + (size.y - rowH * (float)rows) * 0.5f;
                const int i = (int)std::floor((mp.y - top) / rowH);
                if (i >= 0 && i < rows)
                    hovered = i;
            }
        }

        Painter p = GetPainter();
        if (total <= 0.0)
            p.Sector(center, outerR - ring, outerR, 0.0f, kTau, pc.quaternaryFill);
        int positive = 0;
        for (const float v : values)
            positive += v > 0.0f && std::isfinite(v) ? 1 : 0;
        const float inset = positive > 1 ? Sc(1.5f) : 0.0f;   // a gap of the same width between the segments
        float acc = 0.0f;
        for (std::size_t i = 0; i < values.size() && total > 0.0; ++i)
        {
            const float share = values[i] > 0.0f && std::isfinite(values[i]) ? (float)(values[i] / total) : 0.0f;
            const float a0 = start + acc * kTau * shown, sweep = share * kTau * shown;
            acc += share;
            if (share <= 0.0f)
                continue;
            const float grow = Anim(pid, 0x200u + (std::uint32_t)i, (int)i == hovered ? 1.0f : 0.0f, SpringFast());
            p.Sector(center, outerR - ring, outerR + Sc(4) * grow, a0, sweep, PlotColor((int)i), inset);
        }

        // the hole: the hovered segment's label, value and share, else the given text
        const float hole = outerR - ring;
        if (hovered >= 0)
        {
            const std::string_view label = hovered < (int)labels.size() ? labels[(std::size_t)hovered] : std::string_view();
            char value[48], pct[16];
            std::snprintf(value, sizeof(value), o.format && *o.format ? o.format : "%.0f", (double)values[(std::size_t)hovered]);
            std::snprintf(pct, sizeof(pct), "%.0f%%", 100.0 * values[(std::size_t)hovered] / total);
            const text::FontRef big = Font(FontWeight::Semibold, T().type.size[(int)TextStyle::Title3]);
            const Vec2 vs = MeasureText(big, value), ls = MeasureText(lf, label), ps = MeasureText(lf, pct);
            const float h = vs.y + ls.y + ps.y;
            float y = center.y - h * 0.5f;
            p.TextBox(Rect(center.x - hole, y, center.x + hole, y + ls.y), Vec2(0.5f, 0.0f), lf, pc.secondaryLabel, label, text::TextFlags_Ellipsis);
            y += ls.y;
            p.Text(Vec2(std::floor(center.x - vs.x * 0.5f), y), big, pc.label, value);
            y += vs.y;
            p.Text(Vec2(std::floor(center.x - ps.x * 0.5f), y), lf, pc.secondaryLabel, pct);
        }
        else if (!o.center.empty())
        {
            const text::FontRef big = Font(FontWeight::Semibold, T().type.size[(int)TextStyle::Title3]);
            p.TextBox(Rect(center.x - hole, center.y - hole, center.x + hole, center.y + hole), Vec2(0.5f, 0.5f), big, pc.label, o.center, text::TextFlags_Ellipsis);
        }

        // the legend: a dot, the label, the share
        if (legend)
        {
            const float x0 = ringRect.max.x + Sc(22);
            float y = r.min.y + (size.y - rowH * (float)rows) * 0.5f;
            for (int i = 0; i < rows; ++i, y += rowH)
            {
                const float a = hovered < 0 || hovered == i ? 1.0f : 0.45f;
                p.Circle(Vec2(x0 + dot * 0.5f, y + rowH * 0.5f), dot * 0.5f, Style().Fill(PlotColor(i).Fade(a)));
                const std::string_view l = labels[(std::size_t)i];
                const Vec2 ls = MeasureText(lf, l);
                p.Text(Vec2(x0 + dot + Sc(8), std::floor(y + (rowH - ls.y) * 0.5f)), lf, pc.label.Fade(a), l);
                const float share = total > 0.0 && values[(std::size_t)i] > 0.0f ? (float)(100.0 * values[(std::size_t)i] / total) : 0.0f;
                char pct[16];
                std::snprintf(pct, sizeof(pct), "%.0f%%", (double)share);
                const Vec2 ps = MeasureText(lb, pct);
                p.Text(Vec2(r.max.x - ps.x, std::floor(y + (rowH - ps.y) * 0.5f)), lb, pc.secondaryLabel.Fade(a), pct);
            }
        }
    }
}
