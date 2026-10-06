// Esia - UI core debug checks: mistakes in UI code that otherwise only show as something not working - two items
// with one id, two child regions with one id, an item laid out where it cannot be seen (ContextDesc::debugChecks,
// esia/core/context.hpp and docs/UI_CORE.md, section 16).
#include "esia/core/context.hpp"
#include <algorithm>
#include <cstdio>

#if defined(_WIN32)
extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char* text);
#endif

namespace esia
{
    namespace
    {
        // An item counts as out of view once it has stayed there this long: what slides in and out (a page, a panel
        // moving on a spring) passes through.
        constexpr double kOutOfViewSeconds = 1.0;

        constexpr std::uint64_t Once(Diagnostic::Kind kind, Id window, Id key)
        {
            return ((std::uint64_t)kind << 62) ^ ((std::uint64_t)window << 32) ^ key;
        }

        std::string Hex(Id id)
        {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "0x%08X", (unsigned)id);
            return buf;
        }

        std::string WindowName(const std::string& name) { return "\"" + name + "\""; }
    }

    void Context::SetDebugChecks(bool on)
    {
        checks_ = on;
        if (!on)
        {
            idLabels_.clear();
            outOfView_.clear();
            outOfViewLast_ = -1;
            replacedHits_.clear();
        }
    }

    std::string Context::Describe(Id id) const
    {
        const auto it = idLabels_.find(id);
        return it != idLabels_.end() ? Hex(id) + " (\"" + it->second + "\")" : Hex(id);
    }

    void Context::Outline(const Rect& r)
    {
        // in the foreground list, over every window: red, as a failed check
        const std::uint32_t red = Color(1.0f, 0.23f, 0.19f, 1.0f).ToRgba8();
        const float t = 2.0f;
        foreground_.AddRectFilled(r, Color(1.0f, 0.23f, 0.19f, 0.18f).ToRgba8());
        foreground_.AddRectFilled(Rect(r.min.x - t, r.min.y - t, r.max.x + t, r.min.y), red);
        foreground_.AddRectFilled(Rect(r.min.x - t, r.max.y, r.max.x + t, r.max.y + t), red);
        foreground_.AddRectFilled(Rect(r.min.x - t, r.min.y, r.min.x, r.max.y), red);
        foreground_.AddRectFilled(Rect(r.max.x, r.min.y, r.max.x + t, r.max.y), red);
    }

    void Context::Report(Diagnostic d, std::uint64_t once)
    {
        const auto it = std::lower_bound(reported_.begin(), reported_.end(), once);
        if (it != reported_.end() && *it == once)
            return;
        reported_.insert(it, once);
        if (desc_.diagnostics)
        {
            desc_.diagnostics(d);
            return;
        }
        const std::string line = "Esia: " + d.message + "\n";
        std::fputs(line.c_str(), stderr);
#if defined(_WIN32)
        OutputDebugStringA(line.c_str());
#endif
    }

    // ------------------------------------------------------------------ an item out of view
    // An item laid out wholly past the edge of its layout region and of the clip, on an axis that nothing it is in
    // scrolls, is clipped away for good. LayOut asks before the item advances the cursor; the item's ItemAdd that
    // follows (the same rect) names it.
    void Context::CheckOutOfView(const Window& w, const Window::Frame& f, const Rect& r)
    {
        outOfViewLast_ = -1;
        if (w.hidden_ || w.floating_ > 0 || r.Width() <= 0.0f || r.Height() <= 0.0f)
            return;
        const Rect clip = w.drawList_.ClipRect();
        int edge = -1;
        if (r.min.x >= std::max(f.region.max.x, clip.max.x) - 0.5f)
            edge = 1;
        else if (r.max.x <= std::min(f.region.min.x, clip.min.x) + 0.5f)
            edge = 0;
        else if (r.min.y >= std::max(f.region.max.y, clip.max.y) - 0.5f)
            edge = 3;
        else if (r.max.y <= std::min(f.region.min.y, clip.min.y) + 0.5f)
            edge = 2;
        if (edge < 0)
            return;
        // content a scroll area moves may be scrolled to: the window's content scrolls down unless NoScroll
        bool scrollsX = false, scrollsY = !(w.flags_ & WindowFlags_NoScroll);
        for (const Window::Frame& g : w.frames_)
            if (g.childIndex >= 0)
            {
                const std::uint32_t flags = w.children_[(std::size_t)g.childIndex].flags;
                scrollsX = scrollsX || (flags & ChildFlags_ScrollX);
                scrollsY = scrollsY || (flags & ChildFlags_ScrollY);
            }
        if (edge < 2 ? scrollsX : scrollsY)
            return;

        const Id key = HashInt(f.items, f.seq);
        auto it = std::find_if(outOfView_.begin(), outOfView_.end(), [&](const OutOfViewItem& o) { return o.window == w.id_ && o.key == key; });
        if (it == outOfView_.end())
        {
            OutOfViewItem o;
            o.window = w.id_;
            o.key = key;
            o.since = params_.time;
            it = outOfView_.insert(outOfView_.end(), o);
        }
        it->rect = r;
        it->shown = clip;
        it->edge = edge;
        it->lastFrame = frame_;
        outOfViewLast_ = (int)(it - outOfView_.begin());
    }

    void Context::ChildIdConflict(const Window& w, Id id, const Rect& first, const Rect& second)
    {
        Outline(first);
        Outline(second);
        Diagnostic d;
        d.kind = Diagnostic::Kind::ChildIdConflict;
        d.window = w.id_;
        d.id = id;
        d.rect = second;
        d.other = first;
        d.message = "two child regions in window " + WindowName(w.name_) + " began with the same id " + Describe(id) +
                    " in one frame: they share one scroll offset. Put each in its own id scope (ui::IdScope, "
                    "Context::PushId), or give them different names.";
        Report(std::move(d), Once(Diagnostic::Kind::ChildIdConflict, w.id_, id));
    }

    // ------------------------------------------------------------------ at EndFrame
    void Context::RunDebugChecks()
    {
        // ids: two records of one id in a window's hit list (an item's own records run together, RecordHit)
        for (const auto& wp : windows_)
        {
            const Window& w = *wp;
            if (!w.active_ || w.hidden_ || w.hits_.empty())
                continue;
            idScratch_.clear();
            for (const Window::HitRecord& h : w.hits_)
                if (h.id != 0)
                    idScratch_.emplace_back(h.id, h.rect);
            for (const ReplacedHit& r : replacedHits_)
                if (r.window == w.id_)
                    idScratch_.emplace_back(r.id, r.rect);
            std::sort(idScratch_.begin(), idScratch_.end(), [](const std::pair<Id, Rect>& a, const std::pair<Id, Rect>& b) { return a.first < b.first; });
            for (std::size_t i = 0; i + 1 < idScratch_.size();)
            {
                std::size_t j = i + 1;
                while (j < idScratch_.size() && idScratch_[j].first == idScratch_[i].first)
                    ++j;
                if (j - i > 1)
                {
                    for (std::size_t k = i; k < j; ++k)
                        Outline(idScratch_[k].second);
                    Diagnostic d;
                    d.kind = Diagnostic::Kind::IdConflict;
                    d.window = w.id_;
                    d.id = idScratch_[i].first;
                    d.rect = idScratch_[i + 1].second;
                    d.other = idScratch_[i].second;
                    d.message = std::to_string(j - i) + " items in window " + WindowName(w.name_) + " have the same id " + Describe(d.id) +
                                ": they share the hover, presses and per-id state. Give each a label of its own "
                                "(\"Label##2\") or an id scope (ui::IdScope, Context::PushId), e.g. a loop's index.";
                    Report(std::move(d), Once(Diagnostic::Kind::IdConflict, w.id_, idScratch_[i].first));
                }
                i = j;
            }
        }
        replacedHits_.clear();

        // items out of view: those still out of view this frame, for a second now
        static const char* const kEdges[] = {"left", "right", "top", "bottom"};
        std::size_t kept = 0;
        for (std::size_t i = 0; i < outOfView_.size(); ++i)
        {
            const OutOfViewItem& o = outOfView_[i];
            if (o.lastFrame != frame_)
                continue;
            outOfView_[kept++] = o;
            if (params_.time - o.since < kOutOfViewSeconds)
                continue;
            Outline(o.rect);
            // where it went: a bar along the edge it is past
            const float t = 4.0f;
            const Rect& s = o.shown;
            if (o.edge < 2)
            {
                const float x = o.edge == 1 ? s.max.x - t : s.min.x;
                Outline(Rect(x, std::max(o.rect.min.y, s.min.y), x + t, std::max(std::min(o.rect.max.y, s.max.y), std::max(o.rect.min.y, s.min.y) + t)));
            }
            else
            {
                const float y = o.edge == 3 ? s.max.y - t : s.min.y;
                Outline(Rect(std::max(o.rect.min.x, s.min.x), y, std::max(std::min(o.rect.max.x, s.max.x), std::max(o.rect.min.x, s.min.x) + t), y + t));
            }
            const Window* w = nullptr;
            for (const auto& wp : windows_)
                if (wp->id_ == o.window)
                    w = wp.get();
            Diagnostic d;
            d.kind = Diagnostic::Kind::OutOfView;
            d.window = o.window;
            d.id = o.id;
            d.rect = o.rect;
            d.other = o.shown;
            char where[160];
            const bool across = o.edge < 2;
            std::snprintf(where, sizeof(where), "(%s %.0f to %.0f, shown %s %.0f)", across ? "x" : "y", across ? o.rect.min.x : o.rect.min.y,
                          across ? o.rect.max.x : o.rect.max.y, o.edge == 1 || o.edge == 3 ? "to" : "from",
                          o.edge == 0 ? s.min.x : o.edge == 1 ? s.max.x : o.edge == 2 ? s.min.y : s.max.y);
            d.message = "in window " + (w ? WindowName(w->name_) : Hex(o.window)) + ", an item" + (o.id ? " " + Describe(o.id) : std::string()) +
                        " has been laid out past the " + kEdges[o.edge] + " edge of what shows of its container for a second " + where +
                        ": it is clipped away. " +
                        (across ? "Its line is wider than the container: make room for it (a widget that fills the width "
                                  "leaves room for what follows it on its line), break the line, or let the area scroll sideways."
                                : "The container is too short for what it holds: let it scroll (no WindowFlags_NoScroll, "
                                  "ChildFlags_ScrollY) or make it taller.");
            Report(std::move(d), Once(Diagnostic::Kind::OutOfView, o.window, o.key));
        }
        outOfView_.resize(kept);
        outOfViewLast_ = -1;
    }
}
