// Esia UI - charts: the line chart (WGT's LineChart), drawn with Painter's polyline and area.
#include "ui_internal.hpp"
#include <algorithm>
#include <vector>

namespace esia::ui
{
    using namespace detail;

    void LineChart(std::string_view id, std::span<const float> values, const LineChartOptions& o)
    {
        ItemScope scope;
        Context& c = Ctx();
        const Id gid = c.GetId(id);
        Rect r = o.rect;
        if (r.Empty())
        {
            const Vec2 size(o.width > 0.0f ? Sc(o.width) : AvailableWidth(), Sc(o.height));
            const Vec2 pos = c.CursorPos();
            c.ItemSize(size);
            r = Rect::FromSize(pos, size);
            if (!c.ItemAdd(0, r))
                return;
        }
        const int count = (int)values.size();
        if (count < 2)
            return;

        // the value range: fixed, or fitted to the data (from 0 for positive data) and eased, so it never jumps
        float lo = o.min, hi = o.max;
        if (!(hi > lo))
        {
            const auto [dmin, dmax] = std::minmax_element(values.begin(), values.end());
            lo = std::min(*dmin, 0.0f);
            hi = *dmax + (*dmax - lo) * 0.1f;
            if (!(hi > lo))
                hi = lo + 1.0f;
            lo = Anim(gid, 0x80, lo, SpringStd(), lo);
            hi = Anim(gid, 0x81, hi, SpringStd(), hi);
        }

        const float th = Sc(o.thickness);
        const float dot = o.lastPoint ? th * 1.6f : 0.0f;
        const float pad = th * 0.5f + dot + Sc(1);
        const Rect plot(r.min.x + th * 0.5f, r.min.y + pad, r.max.x - std::max(th * 0.5f, dot + Sc(1)), r.max.y - th * 0.5f);
        static thread_local std::vector<Vec2> pts;
        pts.resize((std::size_t)count);
        const int first = ((o.offset % count) + count) % count;   // a ring buffer's oldest value
        for (int i = 0; i < count; ++i)
        {
            const float v = values[(std::size_t)((first + i) % count)];
            const float k = Saturate((v - lo) / (hi - lo));
            pts[(std::size_t)i] = Vec2(plot.min.x + plot.Width() * (float)i / (float)(count - 1), plot.max.y - plot.Height() * k);
        }
        const Color t = Tint(o.tint);
        const std::uint32_t flags = o.smooth ? PolylineFlags_Smooth : PolylineFlags_None;
        Painter p = GetPainter();
        if (o.fill)
            p.Area(pts.data(), count, r.max.y, Paint::Linear(t.Fade(0.30f), t.Fade(0.0f), 90), flags);
        Style s = Style().Fill(t);
        if (o.glow)
            s.Glow(t, Sc(10), 0.9f);
        p.Polyline(pts.data(), count, th, s, flags);
        if (o.lastPoint)
        {
            const Vec2 last = pts.back();
            p.Circle(last, dot * 1.9f, Style().Fill(t.Fade(0.22f)));
            p.Circle(last, dot, Style().Fill(t));
        }
    }
}
