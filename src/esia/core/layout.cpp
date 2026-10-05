// Esia - UI core layout: the container stack with the layout cursor or a LayoutProvider, child regions, scrolling
// and the StackLayout provider (see esia/core/context.hpp, esia/core/layout.hpp and docs/UI_CORE.md, sections 6
// and 8).
#include "esia/core/context.hpp"
#include <algorithm>
#include <cmath>

namespace esia
{
    namespace
    {
        Vec2 Max(Vec2 a, Vec2 b) { return Vec2(std::max(a.x, b.x), std::max(a.y, b.y)); }

        // A scroll offset may move `delta` from where it is (or glides to)?
        bool CanMove(float at, float max, float delta) { return (delta < 0.0f && at > 0.0f) || (delta > 0.0f && at < max); }

        // How long the wheel stays with the area it scrolled after it stopped turning (WheelLatch).
        constexpr double kWheelLatchSeconds = 0.3;

        // ---- the layout cursor's lines (Context::LineRoom)
        constexpr int kLineEdges = 4;   // the items of a line that know what follows them

        // A frame among this frame's frames: from its parent's, the place it has in its parent's lines and its id.
        // Nothing positional: it stays the same while things move (scrolling, an animation above it).
        Id FrameSeq(Id parentSeq, int line, int lineItems, bool sameLine, Id id)
        {
            return HashInt(((std::int64_t)line << 24) ^ ((std::int64_t)lineItems << 1) ^ (sameLine ? 1 : 0), HashInt(id, parentSeq));
        }

        Id LineKey(Id seq, int line, int item) { return HashInt(((std::int64_t)line << 8) | item, seq); }
    }

    // ------------------------------------------------------------------ frames
    Window::Frame& Context::CurFrame()
    {
        Window* w = CurrentWindow();
        ESIA_ASSERT(w && !w->frames_.empty());
        return w->frames_.back();
    }

    const Window::Frame& Context::CurFrame() const
    {
        const Window* w = CurrentWindow();
        ESIA_ASSERT(w && !w->frames_.empty());
        return w->frames_.back();
    }

    Window::Frame* Context::ScrollFrame()
    {
        Window* w = CurrentWindow();
        for (auto it = w->frames_.rbegin(); it != w->frames_.rend(); ++it)
            if (it->scroll)
                return &*it;
        return nullptr;
    }

    const Window::Frame* Context::ScrollFrame() const { return const_cast<Context*>(this)->ScrollFrame(); }

    float Context::Snap(float v) const
    {
        // layout positions land on physical pixels: text and hairlines stay crisp at fractional scales. Down, as
        // Dear ImGui truncates its cursor (WGT's layouts, pixel for pixel); the epsilon keeps a whole pixel whole
        const float s = Scale();
        return s > 0.0f ? std::floor(v * s + 1e-3f) / s : v;
    }

    void Context::InitRootFrame(Window& w)
    {
        w.frames_.clear();
        Window::Frame f;
        const Rect content = w.ContentRect();
        const Vec2 scroll = w.scroll_.scroll;
        f.origin = w.rect_.min;
        f.padding = w.padding_;
        f.contentOrigin = content.min - scroll;
        // the work rect moves with the scroll on both axes: what is available does not grow while scrolling
        f.region = Rect(content.min - scroll, Max(content.max, content.min) - scroll);
        f.cursor = f.cursorMax = f.prevLineEnd = f.contentOrigin;
        f.lineStartX = f.cursor.x;
        f.lineTop = f.cursor.y;
        f.seq = w.id_;
        f.scroll = &w.scroll_;
        w.scroll_.view = Max(content.Size(), Vec2(0, 0));
        w.frames_.push_back(f);
    }

    int Context::CurrentDepth() const { return (int)CurrentWindow()->frames_.size() - 1; }

    void Context::LayOut(Window& w, const Rect& r, float baseline, bool childReport)
    {
        const bool floating = w.floating_ > 0 || childReport;
        LaidOutItem item;
        item.rect = r;
        item.baseline = baseline;
        item.depth = childReport ? -1 : (int)w.frames_.size() - 1;
        item.window = w.id_;
        item.container = w.frames_.back().id;
        item.floating = floating;
        if (observer_)
            observer_(item);
        // the window's item map keeps what was visible of it (glow halos, the layout inspector)
        LaidOutItem visible = item;
        visible.rect = r.Intersect(w.drawList_.ClipRect());
        if (visible.rect.Width() >= 1.0f && visible.rect.Height() >= 1.0f)
            w.laidOut_.push_back(visible);
        if (!childReport)
            AdvanceCursor(w.frames_.back(), r, baseline);
    }

    void Context::AdvanceCursor(Window::Frame& f, const Rect& r, float baseline)
    {
        if (baseline >= 0.0f && f.firstBaseline < 0.0f)
            f.firstBaseline = r.min.y + baseline;
        f.anyItem = true;
        if (!f.layout)
        {
            // the line: SameLine continues it, any other item starts the next one. Each item continuing it tells
            // the items before it how far the line now reaches past them (LineRoom, next frame).
            if (!f.sameLine)
            {
                ++f.line;
                f.lineItems = 0;
            }
            f.sameLine = false;
            const int k = f.lineItems++;
            std::vector<std::pair<Id, float>>& rooms = CurrentWindow()->lineRoom_;
            for (int i = 0; i < std::min(k, kLineEdges); ++i)
            {
                const Id key = LineKey(f.seq, f.line, i);
                const float room = r.max.x - f.lineEdges[i];
                auto it = std::find_if(rooms.rbegin(), rooms.rend(), [key](const std::pair<Id, float>& e) { return e.first == key; });
                if (it != rooms.rend())
                    it->second = std::max(it->second, room);
                else
                    rooms.emplace_back(key, room);
            }
            if (k < kLineEdges)
                f.lineEdges[k] = r.max.x;
        }
        if (f.layout)
        {
            // the provider places the next child
            f.cursorMax = Max(f.cursorMax, r.max);
            LaidOutItem child;
            child.rect = r;
            child.baseline = baseline;
            child.depth = (int)CurrentWindow()->frames_.size() - 1;
            child.window = CurrentWindow()->id_;
            child.container = f.id;
            const LayoutSlot slot = f.layout->Next(child);
            f.cursor = Vec2(Snap(slot.pos.x), Snap(slot.pos.y));
            f.region = slot.region;
            f.lineTop = f.cursor.y;
            return;
        }
        // the layout cursor: the item ends its line; SameLine brings the cursor back to it
        const float h = (r.min.y - f.lineTop) + r.Height();
        f.lineHeight = std::max(f.lineHeight, h);
        if (baseline >= 0.0f)
            f.lineBaseline = std::max(f.lineBaseline, r.min.y - f.lineTop + baseline);
        f.prevLineEnd = Vec2(r.max.x, f.lineTop);
        f.prevLineHeight = f.lineHeight;
        f.prevLineBaseline = f.lineBaseline;
        f.cursorMax.x = std::max(f.cursorMax.x, r.max.x);
        f.cursorMax.y = std::max(f.cursorMax.y, f.lineTop + f.lineHeight);
        f.cursor = Vec2(Snap(f.lineStartX + f.indent), Snap(f.lineTop + f.lineHeight + desc_.layout.itemSpacing.y));
        f.lineTop = f.cursor.y;
        f.lineHeight = 0.0f;
        f.lineBaseline = -1.0f;
    }

    void Context::ItemSize(Vec2 size, float baseline)
    {
        Window* w = CurrentWindow();
        ESIA_ASSERT(w);
        LayOut(*w, Rect::FromSize(CurFrame().cursor, size), baseline, false);
    }

    // ------------------------------------------------------------------ layout cursor
    Vec2 Context::CursorPos() const { return CurFrame().cursor; }

    void Context::SetCursorPos(Vec2 pos)
    {
        Window::Frame& f = CurFrame();
        f.cursor = pos;
        f.lineTop = pos.y;
        f.cursorMax = Max(f.cursorMax, pos);
        f.sameLine = false;
    }

    void Context::SameLine(float offsetFromStartX, float spacing)
    {
        Window::Frame& f = CurFrame();
        f.sameLine = true;
        const float sp = spacing < 0.0f ? desc_.layout.itemSpacing.x : spacing;
        if (offsetFromStartX != 0.0f)
            f.cursor = Vec2(f.lineStartX + offsetFromStartX + (spacing < 0.0f ? 0.0f : spacing), f.prevLineEnd.y);
        else
            f.cursor = Vec2(Snap(f.prevLineEnd.x + sp), f.prevLineEnd.y);
        f.lineTop = f.prevLineEnd.y;
        f.lineHeight = f.prevLineHeight;
        f.lineBaseline = f.prevLineBaseline;
    }

    void Context::NewLine()
    {
        if (CurFrame().lineHeight > 0.0f)
            ItemSize(Vec2(0, 0));
        else
            ItemSize(Vec2(0, desc_.layout.itemSpacing.y));
    }

    void Context::Spacing() { ItemSize(Vec2(0, 0)); }

    void Context::Indent(float width)
    {
        Window::Frame& f = CurFrame();
        const float d = width != 0.0f ? width : desc_.layout.indent;
        f.indent += d;
        f.cursor.x += d;
    }

    void Context::Unindent(float width)
    {
        Window::Frame& f = CurFrame();
        const float d = width != 0.0f ? width : desc_.layout.indent;
        f.indent -= d;
        f.cursor.x -= d;
    }

    Vec2 Context::ContentRegionAvail() const
    {
        const Window::Frame& f = CurFrame();
        return Max(f.region.max - f.cursor, Vec2(0, 0));
    }

    Rect Context::WorkRect() const { return CurFrame().region; }

    Rect Context::ViewRect() const
    {
        const Window* w = CurrentWindow();
        if (!w)
            return Rect();
        for (auto it = w->frames_.rbegin(); it != w->frames_.rend(); ++it)
            if (it->childIndex >= 0)
                return Rect(it->origin, it->origin + it->fixedSize);
        return w->GetRect();
    }

    Rect Context::VisibleViewRect() const
    {
        const Window* w = CurrentWindow();
        if (!w)
            return Rect();
        for (auto it = w->frames_.rbegin(); it != w->frames_.rend(); ++it)
            if (it->childIndex >= 0)
                return w->children_[(std::size_t)it->childIndex].clip;   // its rect inside its parent's clip
        return w->GetRect();
    }

    float Context::LineRoom() const
    {
        const Window* w = CurrentWindow();
        if (!w || w->frames_.empty())
            return 0.0f;
        const Window::Frame& f = w->frames_.back();
        if (f.layout)
            return 0.0f;
        const int line = f.sameLine ? f.line : f.line + 1;
        const int item = f.sameLine ? f.lineItems : 0;
        if (item >= kLineEdges)
            return 0.0f;
        const Id key = LineKey(f.seq, line, item);
        const auto& rooms = w->lineRoomPrev_;
        const auto it = std::lower_bound(rooms.begin(), rooms.end(), key, [](const std::pair<Id, float>& e, Id k) { return e.first < k; });
        return it != rooms.end() && it->first == key ? std::max(it->second, 0.0f) : 0.0f;
    }

    float Context::LineBaseline() const { return CurFrame().lineBaseline; }

    float Context::AlignToLineBaseline(float baseline)
    {
        Window::Frame& f = CurFrame();
        if (baseline < 0.0f || f.lineBaseline < 0.0f)
            return 0.0f;
        const float target = f.lineTop + f.lineBaseline - baseline;
        if (target <= f.cursor.y)
            return 0.0f;
        const float d = target - f.cursor.y;
        f.cursor.y = target;
        return d;
    }

    // ------------------------------------------------------------------ containers
    void Context::BeginContainer(Id id, const ContainerOptions& options)
    {
        Window* w = CurrentWindow();
        ESIA_ASSERT(w);
        const Window::Frame& parent = w->frames_.back();
        Window::Frame f;
        f.id = id;
        f.seq = FrameSeq(parent.seq, parent.line, parent.lineItems, parent.sameLine, id);
        f.layout = options.layout;
        f.origin = parent.cursor;
        f.padding = options.padding;
        const Vec2 avail = Max(parent.region.max - f.origin, Vec2(0, 0));
        f.fixedSize = Vec2(options.size.x > 0.0f ? options.size.x : (options.fillWidth ? std::max(0.0f, avail.x + std::min(options.size.x, 0.0f)) : 0.0f),
                           options.size.y > 0.0f ? options.size.y : 0.0f);
        f.contentOrigin = f.origin + f.padding;
        // a fitted container offers its children what its parent offers it
        const Vec2 outerMax(f.fixedSize.x > 0.0f ? f.origin.x + f.fixedSize.x : parent.region.max.x,
                            f.fixedSize.y > 0.0f ? f.origin.y + f.fixedSize.y : parent.region.max.y);
        f.region = Rect(f.contentOrigin, Max(outerMax - f.padding, f.contentOrigin));
        f.cursor = f.contentOrigin;
        if (f.layout)
        {
            const LayoutSlot slot = f.layout->Begin(f.contentOrigin, f.region);
            f.cursor = Vec2(Snap(slot.pos.x), Snap(slot.pos.y));
            f.region = slot.region;
        }
        f.cursorMax = f.prevLineEnd = f.contentOrigin;
        f.lineStartX = f.contentOrigin.x;
        f.lineTop = f.cursor.y;
        w->frames_.push_back(std::move(f));
    }

    Rect Context::EndContainer()
    {
        Window* w = CurrentWindow();
        ESIA_ASSERT(w && w->frames_.size() > 1 && w->frames_.back().childIndex < 0 && "EndContainer without BeginContainer");
        if (!w || w->frames_.size() <= 1 || w->frames_.back().childIndex >= 0)
            return Rect();
        const Window::Frame f = std::move(w->frames_.back());
        w->frames_.pop_back();
        Vec2 content;
        float baseline = -1.0f;
        if (f.layout)
            content = f.layout->End(&baseline);
        else
        {
            content = Max(f.cursorMax - f.contentOrigin, Vec2(0, 0));
            baseline = f.firstBaseline >= 0.0f ? f.firstBaseline - f.contentOrigin.y : -1.0f;
        }
        const Vec2 size(f.fixedSize.x > 0.0f ? f.fixedSize.x : content.x + f.padding.x * 2.0f,
                        f.fixedSize.y > 0.0f ? f.fixedSize.y : content.y + f.padding.y * 2.0f);
        const Rect rect = Rect::FromSize(f.origin, size);
        // one item of the parent (the parent's cursor is still at the container's origin)
        ItemSize(size, baseline >= 0.0f ? baseline + f.padding.y : -1.0f);
        ItemAdd(f.id, rect);
        return rect;
    }

    void Context::BeginGroup() { BeginContainer(0); }
    void Context::EndGroup() { EndContainer(); }

    // ------------------------------------------------------------------ child regions
    bool Context::BeginChild(std::string_view name, const ChildOptions& options)
    {
        Window* w = CurrentWindow();
        ESIA_ASSERT(w);
        const Window::Frame& parent = w->frames_.back();
        const Id id = GetId(name);
        const bool floating = (options.flags & ChildFlags_Floating) != 0;
        Rect rect;
        if (floating)
            rect = options.rect;
        else
        {
            const Vec2 avail = Max(parent.region.max - parent.cursor, Vec2(0, 0));
            const auto axis = [](float want, float have) { return std::max(0.0f, want > 0.0f ? want : have + want); };
            rect = Rect::FromSize(parent.cursor, Vec2(axis(options.size.x, avail.x), axis(options.size.y, avail.y)));
        }

        Window::ChildState& cs = w->childStates_[id];
        cs.lastFrame = frame_;
        const bool scrolls = (options.flags & (ChildFlags_ScrollX | ChildFlags_ScrollY)) != 0;
        const bool smooth = scrolls && (options.flags & ChildFlags_SmoothScroll);
        cs.smooth = smooth;
        if (scrolls)
        {
            BeginScroll(cs.scroll, smooth);
            ApplyNextScroll(cs.scroll, true);
            SnapScroll(cs.scroll, Scale());
        }
        else
        {
            cs.scroll = Window::ScrollState();
            hasNextScroll_ = false;
        }
        const Vec2 scroll = cs.scroll.scroll;

        // the region as a whole, padding included, inside its parent's clip: what the wheel and HoveredChild see,
        // and for a floating region what hides the content below it (hit below its own items, never scrolled)
        const Rect parentClip = w->drawList_.ClipRect();
        const int parentIndex = w->childStack_.empty() ? -1 : w->childStack_.back();
        Vec2 outer, outerOwn;
        ScrollOffsets(*w, w->children_, parentIndex, outer, outerOwn);
        if (floating)
        {
            ++w->floating_;
            RecordHit(*w, id, rect, ItemFlags_Background, true);
        }
        w->children_.push_back({id, parentIndex, rect.Intersect(parentClip), w->floating_, options.flags, outer});
        const int index = (int)w->children_.size() - 1;
        w->childStack_.push_back(index);

        const Rect content = rect.Expanded(-options.padding.x, -options.padding.y);
        const Rect& in = options.clipInset;
        w->drawList_.PushClipRect(Rect(content.min.x + in.min.x, content.min.y + in.min.y, content.max.x - in.max.x, content.max.y - in.max.y));
        const Rect clip = w->drawList_.ClipRect();

        Window::Frame f;
        f.id = id;
        f.seq = FrameSeq(parent.seq, parent.line, parent.lineItems, parent.sameLine, id);
        f.origin = rect.min;
        f.padding = options.padding;
        f.fixedSize = rect.Size();
        f.contentOrigin = content.min - scroll;
        f.region = Rect(content.min - scroll, Max(content.max, content.min) - scroll);
        f.cursor = f.cursorMax = f.prevLineEnd = f.contentOrigin;
        f.lineStartX = f.cursor.x;
        f.lineTop = f.cursor.y;
        f.childIndex = index;
        f.scroll = scrolls ? &cs.scroll : nullptr;
        f.smooth = smooth;
        cs.scroll.view = Max(content.Size(), Vec2(0, 0));
        w->frames_.push_back(std::move(f));
        w->idStack_.push_back(id);   // ids inside are the child's own
        return !clip.Empty();
    }

    void Context::EndChild()
    {
        Window* w = CurrentWindow();
        ESIA_ASSERT(w && w->frames_.size() > 1 && w->frames_.back().childIndex >= 0 && "EndChild without BeginChild");
        if (!w || w->frames_.size() <= 1 || w->frames_.back().childIndex < 0)
            return;
        const Window::Frame f = std::move(w->frames_.back());
        w->frames_.pop_back();
        const std::uint32_t flags = w->children_[(std::size_t)f.childIndex].flags;
        if (f.scroll)
        {
            Window::ScrollState& s = *f.scroll;
            s.content = Max(f.cursorMax - f.contentOrigin, Vec2(0, 0));
            EndScroll(s, f.smooth);
            // an axis the child does not scroll stays at 0 (its content is clipped)
            if (!(flags & ChildFlags_ScrollX))
                s.max.x = s.scroll.x = s.target.x = 0.0f;
            if (!(flags & ChildFlags_ScrollY))
                s.max.y = s.scroll.y = s.target.y = 0.0f;
        }
        w->drawList_.PopClipRect();
        w->childStack_.pop_back();
        if (w->idStack_.size() > 1)
            w->idStack_.pop_back();
        const Rect rect = Rect::FromSize(f.origin, f.fixedSize);
        if (flags & ChildFlags_Floating)
        {
            // over the content: reported (flagged), the parent's layout does not move
            LayOut(*w, rect, -1.0f, true);
            --w->floating_;
            return;
        }
        ItemSize(rect.Size());
        ItemAdd(f.id, rect);
    }

    // ------------------------------------------------------------------ clip
    void Context::PushClipRect(const Rect& r, bool intersect) { CurrentWindow()->drawList_.PushClipRect(r, intersect); }
    void Context::PopClipRect() { CurrentWindow()->drawList_.PopClipRect(); }

    // ------------------------------------------------------------------ scrolling
    void Context::BeginScroll(Window::ScrollState& s, bool smooth)
    {
        // the range measured last frame bounds the offset; a smooth area already glided in StepSmoothScrolls
        s.pending[0] = s.pending[1] = false;
        s.scroll = Vec2(Clamp(s.scroll.x, 0.0f, s.max.x), Clamp(s.scroll.y, 0.0f, s.max.y));
        if (smooth)
            s.target = Vec2(Clamp(s.target.x, 0.0f, s.max.x), Clamp(s.target.y, 0.0f, s.max.y));
        else
            s.target = s.scroll;
    }

    void Context::SnapScroll(Window::ScrollState& s, float scale) const
    {
        // The offset the layout starts at lands on a physical pixel, as a glide's does (StepSmoothScrolls). Layout
        // positions snap to pixels (Snap), so between pixels the offset would change what the content measures, and
        // with it the range that clamps the offset: a drag held at the end of an area (each frame's offset the end)
        // shook the content by a pixel every few frames.
        const float pixel = 1.0f / std::max(scale, 1e-3f);
        s.scroll = Vec2(std::round(s.scroll.x / pixel) * pixel, std::round(s.scroll.y / pixel) * pixel);
    }

    void Context::StepSmoothScrolls()
    {
        // Every smooth area used last frame glides to its target now, before the hit test (it sees where the content
        // is): exponentially, frame-rate independent, on whole physical pixels of its window.
        const float tau = std::max(desc_.layout.scrollSmoothing, 1e-4f);
        const float k = 1.0f - std::exp(-input_.DeltaTime() / tau);
        for (auto& w : windows_)
        {
            if (!w->wasActive_)
                continue;
            const float pixel = 1.0f / std::max(w->scale_, 1e-3f);
            for (auto& [id, cs] : w->childStates_)
            {
                if (!cs.smooth || cs.lastFrame + 1 != frame_)
                    continue;
                Window::ScrollState& s = cs.scroll;
                for (int a = 0; a < 2; ++a)
                {
                    float& v = a == 0 ? s.scroll.x : s.scroll.y;
                    const float t = Clamp(a == 0 ? s.target.x : s.target.y, 0.0f, a == 0 ? s.max.x : s.max.y);
                    if (v == t)
                        continue;
                    // at least one pixel per frame: rounding would otherwise stall the tail short of the target
                    float step = (t - v) * k;
                    if (std::fabs(step) < pixel)
                        step = std::copysign(std::min(pixel, std::fabs(t - v)), t - v);
                    v = std::round((v + step) / pixel) * pixel;
                    if (std::fabs(t - v) < pixel)
                        v = std::round(t / pixel) * pixel;   // the pixel nearest the target: done
                    else
                        animating_ = true;
                }
            }
        }
    }

    void Context::EndScroll(Window::ScrollState& s, bool smooth)
    {
        // requests of this frame land now, against the range its content just measured
        s.max = Max(s.content - s.view, Vec2(0, 0));
        for (int a = 0; a < 2; ++a)
        {
            if (!s.pending[a])
                continue;
            const float v = Clamp(a == 0 ? s.request.x : s.request.y, 0.0f, a == 0 ? s.max.x : s.max.y);
            (a == 0 ? s.target.x : s.target.y) = v;
            if (!smooth)
                (a == 0 ? s.scroll.x : s.scroll.y) = v;
            s.pending[a] = false;
        }
        if (smooth)
        {
            s.target = Vec2(Clamp(s.target.x, 0.0f, s.max.x), Clamp(s.target.y, 0.0f, s.max.y));
            const float pixel = 1.0f / std::max(Scale(), 1e-3f);
            if (std::fabs(s.target.x - s.scroll.x) >= pixel || std::fabs(s.target.y - s.scroll.y) >= pixel)
                animating_ = true;
        }
    }

    void Context::ApplyNextScroll(Window::ScrollState& s, bool immediate)
    {
        // SetNextScroll: the offset this frame's layout starts at (when it has not started yet) and a request, so
        // that the end of the area clamps it to the range its content measures now - also on its first frame
        if (!hasNextScroll_)
            return;
        hasNextScroll_ = false;
        for (int a = 0; a < 2; ++a)
        {
            const float v = a == 0 ? nextScroll_.x : nextScroll_.y;
            if (v < 0.0f)
                continue;
            if (immediate)
                (a == 0 ? s.scroll.x : s.scroll.y) = (a == 0 ? s.target.x : s.target.y) = v;
            s.pending[a] = true;
            (a == 0 ? s.request.x : s.request.y) = v;
        }
    }

    void Context::RequestScroll(int axis, float value)
    {
        Window::Frame* f = ScrollFrame();
        if (!f)
            return;
        f->scroll->pending[axis] = true;
        (axis == 0 ? f->scroll->request.x : f->scroll->request.y) = value;
    }

    Vec2 Context::Scroll() const
    {
        const Window::Frame* f = ScrollFrame();
        return f ? f->scroll->scroll : Vec2(0, 0);
    }

    Vec2 Context::ScrollMax() const
    {
        const Window::Frame* f = ScrollFrame();
        return f ? f->scroll->max : Vec2(0, 0);
    }

    void Context::SetScrollX(float x) { RequestScroll(0, x); }
    void Context::SetScrollY(float y) { RequestScroll(1, y); }

    void Context::SetScrollHereX(float ratio)
    {
        const Window::Frame* f = ScrollFrame();
        if (f)
            RequestScroll(0, CurFrame().cursor.x - f->contentOrigin.x - ratio * f->scroll->view.x);
    }

    void Context::SetScrollHereY(float ratio)
    {
        const Window::Frame* f = ScrollFrame();
        if (f)
            RequestScroll(1, CurFrame().cursor.y - f->contentOrigin.y - ratio * f->scroll->view.y);
    }

    void Context::ScrollToRect(const Rect& rect, Vec2 align)
    {
        const Window::Frame* f = ScrollFrame();
        if (!f)
            return;
        const Window::ScrollState& s = *f->scroll;
        for (int a = 0; a < 2; ++a)
        {
            const float lo = (a == 0 ? rect.min.x - f->contentOrigin.x : rect.min.y - f->contentOrigin.y);
            const float hi = (a == 0 ? rect.max.x - f->contentOrigin.x : rect.max.y - f->contentOrigin.y);
            const float view = a == 0 ? s.view.x : s.view.y;
            const float at = s.pending[a] ? (a == 0 ? s.request.x : s.request.y) : (a == 0 ? s.target.x : s.target.y);
            const float k = a == 0 ? align.x : align.y;
            if (k >= 0.0f)
                RequestScroll(a, lo - k * (view - (hi - lo)));
            else if (lo < at)
                RequestScroll(a, lo);
            else if (hi > at + view)
                RequestScroll(a, hi - view);
        }
    }

    void Context::UpdateWheel()
    {
        const Vec2 wheel = input_.Wheel();
        Window* w = hoveredWindow_;
        if ((wheel.x == 0.0f && wheel.y == 0.0f) || !w || activeId_ != 0 || PressBlocked())
            return;
        const float step = desc_.layout.scrollStep;
        // shift + wheel scrolls horizontally, as on every desktop platform
        const bool shift = (input_.Mods() & Mod_Shift) != 0;
        const Vec2 delta(-(shift ? wheel.y : wheel.x) * step, shift ? 0.0f : -wheel.y * step);

        // the innermost scroll child under the mouse that can move that way takes each axis, else its parents,
        // else the window
        int innermost = -1;
        if (input_.MouseValid())
            for (int i = 0; i < (int)w->childrenPrev_.size(); ++i)
                if ((innermost < 0 || w->childrenPrev_[(std::size_t)i].layer >= w->childrenPrev_[(std::size_t)innermost].layer) &&
                    ChildClipNow(*w, (std::size_t)i).Contains(input_.MousePos()))
                    innermost = i;
        const double now = params_.time;
        for (int a = 0; a < 2; ++a)
        {
            const float d = a == 0 ? delta.x : delta.y;
            if (d == 0.0f)
                continue;
            const std::uint32_t axisFlag = a == 0 ? ChildFlags_ScrollX : ChildFlags_ScrollY;
            const auto state = [&](int i) -> Window::ChildState* {
                const Window::ChildRecord& c = w->childrenPrev_[(std::size_t)i];
                if (!(c.flags & axisFlag) || (c.flags & ChildFlags_NoWheel))
                    return nullptr;
                auto it = w->childStates_.find(c.id);
                return it != w->childStates_.end() ? &it->second : nullptr;
            };
            const auto canMove = [&](int i) {
                const Window::ScrollState& s = w->childStates_.find(w->childrenPrev_[(std::size_t)i].id)->second.scroll;
                const bool smooth = (w->childrenPrev_[(std::size_t)i].flags & ChildFlags_SmoothScroll) != 0;
                return CanMove(smooth ? (a == 0 ? s.target.x : s.target.y) : (a == 0 ? s.scroll.x : s.scroll.y), a == 0 ? s.max.x : s.max.y, d);
            };

            // While the wheel keeps turning (and the mouse stays put) it stays with the area it scrolled, as in a
            // browser: the window scrolling a table in under the mouse does not hand the rest of the turn to the
            // table, and an area at its end does not pass it on to its parent halfway. -1 = the window.
            WheelLatch& latch = wheelLatch_[a];
            const bool latched = latch.window == w->id_ && now - latch.time < kWheelLatchSeconds && input_.MouseValid() &&
                                 std::fabs(input_.MousePos().x - latch.mouse.x) < 8.0f && std::fabs(input_.MousePos().y - latch.mouse.y) < 8.0f;
            int target = -2;
            if (latched && latch.child == 0)
                target = -1;
            else if (latched)
                for (int i = 0; i < (int)w->childrenPrev_.size(); ++i)
                    if (w->childrenPrev_[(std::size_t)i].id == latch.child && state(i))
                        target = i;
            if (target == -2)
            {
                for (int i = innermost; i >= 0 && target == -2; i = w->childrenPrev_[(std::size_t)i].parent)
                    if (state(i) && canMove(i))
                        target = i;
                if (target == -2 && !(w->flags_ & WindowFlags_NoScroll))
                    target = -1;
            }
            if (target == -2)
            {
                latch.window = 0;
                continue;
            }
            latch.window = w->id_;
            latch.child = target >= 0 ? w->childrenPrev_[(std::size_t)target].id : 0;
            latch.time = now;
            latch.mouse = input_.MousePos();

            if (target >= 0)
            {
                Window::ScrollState& s = state(target)->scroll;
                const bool smooth = (w->childrenPrev_[(std::size_t)target].flags & ChildFlags_SmoothScroll) != 0;
                float& at = smooth ? (a == 0 ? s.target.x : s.target.y) : (a == 0 ? s.scroll.x : s.scroll.y);
                at = Clamp(at + d, 0.0f, a == 0 ? s.max.x : s.max.y);
                if (!smooth)
                    (a == 0 ? s.target.x : s.target.y) = at;
            }
            else if (!(w->flags_ & WindowFlags_NoScroll))
            {
                Window::ScrollState& s = w->scroll_;
                float& at = a == 0 ? s.scroll.x : s.scroll.y;
                at = Clamp(at + d, 0.0f, a == 0 ? s.max.x : s.max.y);
                (a == 0 ? s.target.x : s.target.y) = at;
            }
        }
    }

    // ------------------------------------------------------------------ StackLayout
    LayoutSlot StackLayout::Slot() const
    {
        const std::size_t i = now_.size();
        const Child* last = i < last_.size() ? &last_[i] : nullptr;
        if (horizontal)
        {
            // across a row: from what this child measured last frame against the row's height / baseline
            float off = 0.0f;
            if (last)
            {
                switch (align)
                {
                case Align::Center: off = ((alignInRegion ? region_.Height() : lastCross_) - last->size.y) * 0.5f; break;
                case Align::End: off = (alignInRegion ? region_.Height() : lastCross_) - last->size.y; break;
                case Align::Baseline: off = (last->baseline >= 0.0f && lastBaseline_ >= 0.0f) ? lastBaseline_ - last->baseline : 0.0f; break;
                default: break;
                }
            }
            const Vec2 pos(origin_.x + main_, origin_.y + std::max(off, 0.0f));
            return {pos, Rect(Vec2(pos.x, origin_.y), Vec2(std::max(region_.max.x, pos.x), std::max(region_.max.y, origin_.y)))};
        }
        float off = 0.0f;
        if (last)
        {
            // against the widest child (a fitted stack stays as wide as its content), or the whole region
            const float room = (alignInRegion ? region_.Width() : lastCross_) - last->size.x;
            if (align == Align::Center)
                off = room * 0.5f;
            else if (align == Align::End)
                off = room;
        }
        const Vec2 pos(origin_.x + std::max(off, 0.0f), origin_.y + main_);
        return {pos, Rect(Vec2(origin_.x, pos.y), Vec2(std::max(region_.max.x, origin_.x), std::max(region_.max.y, pos.y)))};
    }

    LayoutSlot StackLayout::Begin(Vec2 origin, const Rect& region)
    {
        origin_ = origin;
        region_ = region;
        main_ = 0.0f;
        now_.clear();
        lastCross_ = 0.0f;
        lastBaseline_ = -1.0f;
        for (const Child& c : last_)
        {
            lastCross_ = std::max(lastCross_, horizontal ? c.size.y : c.size.x);
            lastBaseline_ = std::max(lastBaseline_, c.baseline);
        }
        crossExtent_ = 0.0f;
        baseline_ = -1.0f;
        return Slot();
    }

    LayoutSlot StackLayout::Next(const LaidOutItem& child)
    {
        now_.push_back({child.rect.Size(), child.baseline});
        main_ += (horizontal ? child.rect.Width() : child.rect.Height()) + spacing;
        if (horizontal)
        {
            crossExtent_ = std::max(crossExtent_, child.rect.max.y - origin_.y);
            if (child.baseline >= 0.0f)
                baseline_ = std::max(baseline_, child.rect.min.y - origin_.y + child.baseline);
        }
        else
        {
            crossExtent_ = std::max(crossExtent_, child.rect.max.x - origin_.x);
            if (child.baseline >= 0.0f && baseline_ < 0.0f)
                baseline_ = child.rect.min.y - origin_.y + child.baseline;
        }
        return Slot();
    }

    Vec2 StackLayout::End(float* baseline)
    {
        const float main = now_.empty() ? 0.0f : main_ - spacing;
        last_ = now_;
        if (baseline)
            *baseline = baseline_;
        return horizontal ? Vec2(main, crossExtent_) : Vec2(crossExtent_, main);
    }
}
