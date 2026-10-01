// Esia - UI core context: frames, windows, hit testing, press ownership, popups (see esia/core/context.hpp and
// docs/UI_CORE.md). Items and keys are in items.cpp, layout, child regions and scrolling in layout.cpp.
#include "esia/core/context.hpp"
#include <algorithm>
#include <cmath>

namespace esia
{
    namespace
    {
        constexpr std::string_view kRootName = "##root";
        constexpr std::string_view kTooltipName = "##tooltip";
        constexpr int kEdgeLeft = 1, kEdgeRight = 2, kEdgeTop = 4, kEdgeBottom = 8;

        MouseCursor ResizeCursor(int edges)
        {
            const bool diag = (edges & (kEdgeLeft | kEdgeRight)) && (edges & (kEdgeTop | kEdgeBottom));
            if (diag)
                return (edges == (kEdgeLeft | kEdgeTop) || edges == (kEdgeRight | kEdgeBottom)) ? MouseCursor::ResizeNWSE : MouseCursor::ResizeNESW;
            return (edges & (kEdgeLeft | kEdgeRight)) ? MouseCursor::ResizeEW : MouseCursor::ResizeNS;
        }
    }

    Context::Context(const ContextDesc& desc) : desc_(desc) { input_.config = desc.input; }

    Context::~Context() = default;

    void Context::QueueInput(InputEvent e)
    {
        std::lock_guard lock(inputMutex_);
        queued_.push_back(std::move(e));
    }

    float Context::Scale() const
    {
        const Window* w = CurrentWindow();
        return w ? w->scale_ : params_.framebufferScale.x;
    }

    // ------------------------------------------------------------------ frame
    void Context::NewFrame(const FrameParams& params)
    {
        ESIA_ASSERT(!inFrame_ && "NewFrame called twice without EndFrame");
        params_ = params;
        ++frame_;
        inFrame_ = true;
        {
            std::lock_guard lock(inputMutex_);
            input_.NewFrame(params.time, queued_);
            inputPending_ = !queued_.empty();
        }

        // last frame's records become the ones hit testing and LaidOutItems() read; a window that was not
        // submitted has none
        for (auto& w : windows_)
        {
            w->wasActive_ = w->active_;
            w->wasHidden_ = w->hidden_;
            w->active_ = false;
            w->hitsPrev_.clear();
            w->childrenPrev_.clear();
            w->laidOutPrev_.clear();
            if (w->wasActive_)
            {
                w->hitsPrev_.swap(w->hits_);
                w->childrenPrev_.swap(w->children_);
                w->laidOutPrev_.swap(w->laidOut_);
            }
            w->hits_.clear();
            w->children_.clear();
            w->laidOut_.clear();
        }
        // what refers to a window that was not submitted lets go of it (it may be freed below)
        if (focusedWindow_ && !focusedWindow_->wasActive_)
            focusedWindow_ = nullptr;
        if (dragWindow_ && !dragWindow_->wasActive_)
        {
            if (activeId_ == MoveId(*dragWindow_) || activeId_ == ResizeId(*dragWindow_))
                activeId_ = 0;
            dragWindow_ = nullptr;
            resizeEdges_ = 0;
        }
        hoveredWindow_ = nullptr;
        FreeUnusedWindows();
        if (frame_ % 64 == 0)
            state_.Collect(frame_, desc_.retainFrames);

        // an active item that was not submitted last frame is gone
        if (activeId_ != 0 && !activeAlive_ && !activeSetThisFrame_)
            activeId_ = 0;
        activeAlive_ = false;
        activeSetThisFrame_ = false;
        if (focusId_ != 0 && !focusAlive_)
            focusId_ = 0;
        focusAlive_ = false;
        focusClaimed_ = false;
        textInputRequested_ = false;
        hoveredId_ = 0;
        newItemClaim_ = 0;
        focusOrder_.clear();
        lastItem_ = LastItem();
        keyOwnerPrev_ = keyOwner_;
        keyOwner_.fill(0);
        keyboardOwnerPrev_ = keyboardOwner_;
        keyboardOwner_ = 0;
        anyKeyClaim_ = false;
        animating_ = false;
        requests_ = PlatformRequests();

        UpdatePopupsAtNewFrame();
        UpdateMoveResize();
        UpdateHoveredWindow();
        // content moves before the hit test: it sees where last frame's items are now
        StepSmoothScrolls();
        UpdateWheel();
        HitTest();
        UpdatePressOwners();
        StartResize();

        const Rect display(Vec2(0, 0), params.displaySize);
        background_.Reset(display);
        foreground_.Reset(display);

        // the implicit root window: items submitted outside any Begin / End land here
        SetNextWindowPos(Vec2(0, 0));
        SetNextWindowSize(params.displaySize);
        WindowOptions ro;
        ro.flags = WindowFlags_NoMove | WindowFlags_NoResize | WindowFlags_NoBringToFront | WindowFlags_NoFocus;
        ro.layer = WindowLayer::Background;
        ro.minSize = Vec2(0, 0);
        ro.padding = Vec2(0, 0);
        Begin(kRootName, ro);
        root_ = CurrentWindow();
    }

    void Context::EndFrame()
    {
        ESIA_ASSERT(inFrame_ && "EndFrame without NewFrame");
        ESIA_ASSERT(stack_.size() == 1 && "Begin / End mismatch");
        while (stack_.size() > 1)
            End();
        End();   // root
        inFrame_ = false;
        popupStack_.clear();

        // keys nobody claimed act now, after every widget had its chance
        UpdateTabNavigation();
        if (!popups_.empty() && KeyOwner(Key::Escape) == 0 && input_.KeyPressed(Key::Escape, false))
            ClosePopupsFrom(popups_.size() - 1);

        // a click that no item took: focus the window under the mouse, and move it by its empty area
        const PressOwner leftOwner = pressOwner_[(int)MouseButton::Left];
        if (input_.MouseClicked(MouseButton::Left) && leftOwner != PressOwner::Dismiss)
        {
            Window* w = hoveredWindow_;
            if (w && w != root_)
            {
                FocusWindow(w);
                if (hitId_ == 0 && hoveredId_ == 0 && newItemClaim_ == 0 && activeId_ == 0 && !(w->flags_ & WindowFlags_NoMove))
                {
                    dragWindow_ = w;
                    dragOffset_ = input_.MousePos() - w->rect_.min;
                    resizeEdges_ = 0;
                    SetActiveId(MoveId(*w));
                }
            }
            else
                FocusWindow(nullptr);
            // clicking anything but the focused item takes keyboard focus away
            if (!focusClaimed_ && hoveredId_ != focusId_)
                focusId_ = 0;
        }

        // output
        drawData_ = DrawData();
        drawData_.displaySize = params_.displaySize;
        drawData_.framebufferScale = params_.framebufferScale;
        drawData_.time = params_.time;
        drawData_.deltaTime = input_.DeltaTime();
        if (!background_.Empty())
            drawData_.lists.push_back(&background_);
        for (Window* w : WindowsInDrawOrder())
            if (!w->hidden_ && !w->drawList_.Empty())
                drawData_.lists.push_back(&w->drawList_);
        if (!foreground_.Empty())
            drawData_.lists.push_back(&foreground_);

        // The mouse is the UI's while a press it owns is held (or released this frame); with no press held, while
        // it is over a UI window or item. A press that started over the game stays the game's.
        bool uiPress = false, anyPress = false;
        for (int b = 0; b < (int)MouseButton::Count; ++b)
        {
            const MouseButton mb = (MouseButton)b;
            if (!input_.MouseDown(mb) && !input_.MouseReleased(mb))
                continue;
            anyPress = true;   // held, or released this frame: the release goes to the press's owner
            uiPress = uiPress || pressOwner_[b] == PressOwner::Ui || pressOwner_[b] == PressOwner::Dismiss;
        }
        const bool overUi = (hoveredWindow_ && hoveredWindow_ != root_) || hoveredId_ != 0 || hitId_ != 0;
        requests_.wantCaptureMouse = uiPress || activeId_ != 0 || (!anyPress && overUi);
        requests_.wantCaptureKeyboard = focusId_ != 0 || activeId_ != 0 || anyKeyClaim_;
        requests_.wantTextInput = textInputRequested_ && focusId_ != 0;
        requests_.inputPending = inputPending_;
        requests_.animating = animating_;
        if (dragWindow_ && resizeEdges_ == 0)
            requests_.cursor = MouseCursor::ResizeAll;
    }

    std::vector<Window*> Context::WindowsInDrawOrder() const
    {
        std::vector<Window*> out;
        for (int layer = 0; layer <= (int)WindowLayer::Tooltip; ++layer)
            for (Window* w : order_)
                if ((int)w->layer_ == layer && w->active_)
                    out.push_back(w);
        return out;
    }

    // ------------------------------------------------------------------ windows
    Window* Context::FindWindowByName(std::string_view name) const
    {
        const Id id = HashLabel(name, 0);
        for (const auto& w : windows_)
            if (w->id_ == id)
                return w.get();
        return nullptr;
    }

    Window* Context::AddWindow(std::string_view name, Id id)
    {
        auto w = std::make_unique<Window>();
        w->id_ = id;
        w->name_ = std::string(name);
        w->rect_ = Rect(Vec2(60, 60), Vec2(460, 360));
        Window* p = w.get();
        windows_.push_back(std::move(w));
        order_.push_back(p);   // new windows appear on top of their layer
        return p;
    }

    void Context::FreeUnusedWindows()
    {
        // Dynamically named windows (a popup per id, a window per document) must not pile up: a window that was not
        // submitted for retainFrames frames goes, with its draw list and records. Nothing points at it any more:
        // focus, hover and drag let go of it the first frame it was missing.
        for (auto it = windows_.begin(); it != windows_.end();)
        {
            Window* w = it->get();
            if (w != root_ && !w->wasActive_ && frame_ - w->lastFrame_ > desc_.retainFrames)
            {
                order_.erase(std::remove(order_.begin(), order_.end(), w), order_.end());
                it = windows_.erase(it);
                continue;
            }
            // the same for child regions (their scroll state): no frame refers to them before the first Begin
            for (auto c = w->childStates_.begin(); c != w->childStates_.end();)
                c = frame_ - c->second.lastFrame > desc_.retainFrames ? w->childStates_.erase(c) : std::next(c);
            ++it;
        }
    }

    void Context::FocusWindow(Window* w)
    {
        if (w && (w->flags_ & WindowFlags_NoFocus))
            return;
        focusedWindow_ = w;
        if (w && !(w->flags_ & WindowFlags_NoBringToFront))
        {
            auto it = std::find(order_.begin(), order_.end(), w);
            if (it != order_.end())
            {
                order_.erase(it);
                order_.push_back(w);
            }
        }
    }

    void Context::SetNextWindowPos(Vec2 pos, Cond cond, Vec2 pivot)
    {
        nextPos_ = pos;
        nextPivot_ = pivot;
        nextPosCond_ = cond;
        hasNextPos_ = true;
    }

    void Context::SetNextWindowSize(Vec2 size, Cond cond)
    {
        nextSize_ = size;
        nextSizeCond_ = cond;
        hasNextSize_ = true;
    }

    void Context::SetNextScroll(Vec2 scroll)
    {
        nextScroll_ = scroll;
        hasNextScroll_ = true;
    }

    float Context::MonitorScale(const Rect& r) const
    {
        if (monitors_.empty())
            return params_.framebufferScale.x;
        // the monitor under the center; off every monitor, the one it overlaps most (or the first)
        const Vec2 c = r.Center();
        const Monitor* best = &monitors_[0];
        float bestArea = -1.0f;
        for (const Monitor& m : monitors_)
        {
            if (m.rect.Contains(c))
                return m.scale;
            const Rect o = m.rect.Intersect(r);
            const float area = o.Empty() ? 0.0f : o.Width() * o.Height();
            if (area > bestArea)
            {
                bestArea = area;
                best = &m;
            }
        }
        return best->scale;
    }

    void Context::KeepOnScreen(Window& w) const
    {
        // at least `keep` units stay inside the display horizontally, and the top edge (where a header is dragged)
        // stays reachable
        const Vec2 d = params_.displaySize;
        const float keep = desc_.layout.keepOnScreen;
        const Vec2 size = w.rect_.Size();
        const float kx = std::min(keep, size.x), ky = std::min(keep, size.y);
        const float x = std::max(std::min(w.rect_.min.x, d.x - kx), kx - size.x);
        const float y = std::max(std::min(w.rect_.min.y, d.y - ky), 0.0f);
        w.rect_ = Rect::FromSize(Vec2(x, y), size);
    }

    bool Context::Begin(std::string_view name, const WindowOptions& options)
    {
        Window* w = BeginWindow(HashLabel(name, 0), name, options);
        return !w->rect_.Empty();
    }

    Window* Context::BeginWindow(Id id, std::string_view name, const WindowOptions& options, bool fullyOnScreen)
    {
        ESIA_ASSERT(inFrame_ && "Begin outside NewFrame / EndFrame");
        Window* w = nullptr;
        for (const auto& x : windows_)
            if (x->id_ == id)
                w = x.get();
        const bool created = w == nullptr;
        if (created)
            w = AddWindow(name, id);

        const bool firstThisFrame = !w->active_;
        if (firstThisFrame)
        {
            w->appearing_ = created || !w->wasActive_;
            w->flags_ = options.flags;
            w->layer_ = options.layer;
            w->minSize_ = options.minSize;
            w->maxSize_ = options.maxSize;
            w->padding_ = options.padding.x >= 0.0f ? options.padding : desc_.layout.windowPadding;
            const auto applies = [&](Cond c) { return c == Cond::Always || (c == Cond::FirstUse && created) || (c == Cond::Appearing && w->appearing_); };
            if (hasNextSize_ && applies(nextSizeCond_))
                w->rect_.max = w->rect_.min + nextSize_;
            if (w->flags_ & WindowFlags_AutoSize)
            {
                // the content measured last frame plus padding; not measured yet (or appearing): measured now, shown
                // from the next frame
                if (w->measured_)
                    w->rect_.max = w->rect_.min + w->scroll_.content + w->padding_ * 2.0f;
                w->hidden_ = !w->measured_ || w->appearing_;
            }
            else
                w->hidden_ = false;
            Vec2 sz(Clamp(w->rect_.Width(), w->minSize_.x, w->maxSize_.x), Clamp(w->rect_.Height(), w->minSize_.y, w->maxSize_.y));
            if (hasNextPos_ && applies(nextPosCond_))
                w->rect_ = Rect::FromSize(nextPos_ - sz * nextPivot_, sz);
            w->rect_.max = w->rect_.min + sz;

            // DPI: the scale of the monitor under the window; when it changes, the window keeps its physical size
            float scale = MonitorScale(w->rect_);
            if (w->scaleKnown_ && scale != w->scale_ && w != root_ && !(w->flags_ & WindowFlags_AutoSize))
            {
                sz = w->rect_.Size() * (scale / w->scale_);
                sz = Vec2(Clamp(sz.x, w->minSize_.x, w->maxSize_.x), Clamp(sz.y, w->minSize_.y, w->maxSize_.y));
                // hysteresis: the rescaled window must still be on the new monitor, or it would flip back and forth
                // at the boundary (it is anchored at its top-left, where drags and SetNextWindowPos put it)
                const Rect rescaled = Rect::FromSize(w->rect_.min, sz);
                if (MonitorScale(rescaled) == scale)
                    w->rect_ = rescaled;
                else
                    scale = w->scale_;
            }
            w->scaleChanged_ = w->scaleKnown_ && scale != w->scale_;
            w->scale_ = scale;
            w->scaleKnown_ = true;
            if (fullyOnScreen)
            {
                // popups and tooltips lie entirely on screen
                const Vec2 d = params_.displaySize, size = w->rect_.Size();
                w->rect_ = Rect::FromSize(Vec2(std::max(0.0f, std::min(w->rect_.min.x, d.x - size.x)), std::max(0.0f, std::min(w->rect_.min.y, d.y - size.y))), size);
            }
            else if (!(w->flags_ & WindowFlags_NoMove) && w->layer_ != WindowLayer::Background)
                KeepOnScreen(*w);

            w->active_ = true;
            w->lastFrame_ = frame_;
            // a new window, and one that appears again, comes to the front
            if (w->appearing_ && !(w->flags_ & WindowFlags_NoFocus))
                FocusWindow(w);

            BeginScroll(w->scroll_, false);
            ApplyNextScroll(w->scroll_, true);
            w->drawList_.Reset(w->rect_);
            // items are visible and hoverable inside the content rect only; decorations draw with a wider clip
            w->drawList_.PushClipRect(w->ContentRect());
            w->idStack_.assign(1, id);
            w->floating_ = 0;
            w->childStack_.clear();
            InitRootFrame(*w);
        }
        else
            ApplyNextScroll(w->scroll_, false);   // appended to: its layout already started, the request lands at End
        hasNextPos_ = hasNextSize_ = false;
        stack_.push_back(w);
        return w;
    }

    void Context::End()
    {
        ESIA_ASSERT(!stack_.empty() && "End without Begin");
        if (stack_.empty())
            return;
        Window* w = stack_.back();
        ESIA_ASSERT(w->frames_.size() == 1 && "a container or child region was not closed before End");
        while (w->frames_.size() > 1)
        {
            if (w->frames_.back().childIndex >= 0)
                EndChild();
            else
                EndContainer();
        }
        // what the content measured (scroll range for the next frame, AutoSize)
        Window::Frame& f = w->frames_[0];
        f.cursorMax = Vec2(std::max(f.cursorMax.x, f.contentOrigin.x), std::max(f.cursorMax.y, f.contentOrigin.y));
        w->scroll_.content = f.cursorMax - f.contentOrigin;
        w->scroll_.view = w->ContentRect().Size();
        EndScroll(w->scroll_, false);
        w->measured_ = true;
        stack_.pop_back();
    }

    // ------------------------------------------------------------------ hit testing
    void Context::UpdateHoveredWindow()
    {
        hoveredWindow_ = nullptr;
        if (dragWindow_)
        {
            hoveredWindow_ = dragWindow_;
            return;
        }
        if (!input_.MouseValid())
            return;
        const Vec2 p = input_.MousePos();
        // last frame's windows, front to back
        std::vector<Window*> order;
        for (int layer = 0; layer <= (int)WindowLayer::Tooltip; ++layer)
            for (Window* w : order_)
                if ((int)w->layer_ == layer && w->wasActive_ && !w->wasHidden_)
                    order.push_back(w);
        for (auto it = order.rbegin(); it != order.rend(); ++it)
        {
            Window* w = *it;
            if (w->flags_ & WindowFlags_NoInputs)
                continue;
            const float border = (w->flags_ & WindowFlags_NoResize) ? 0.0f : desc_.layout.resizeBorder;
            if (w->rect_.Expanded(border).Contains(p))
            {
                hoveredWindow_ = w;
                return;
            }
        }
    }

    Rect Context::ChildClipNow(const Window& w, std::size_t index) const
    {
        // a child region moves with its parents' scroll (a floating one stays where it floats)
        const Window::ChildRecord& c = w.childrenPrev_[index];
        if (c.flags & ChildFlags_Floating)
            return c.clip;
        Vec2 total, own;
        ScrollOffsets(w, w.childrenPrev_, c.parent, total, own);
        return c.clip.Translated(c.outer - total);
    }

    void Context::HitTest()
    {
        // The front-most of last frame's items under the mouse, in the hovered window: a higher layer first
        // (floating children, then content, background items last), then the item submitted (drawn) last. Each
        // record is where its item is now: shifted by how far its scroll areas moved since it was laid out (its clip
        // by how far the areas around its own one moved).
        hitId_ = 0;
        hitFlags_ = 0;
        hoveredChild_ = 0;
        Window* w = hoveredWindow_;
        if (!w || !input_.MouseValid())
            return;
        const Vec2 p = input_.MousePos();
        const Window::HitRecord* best = nullptr;
        for (const Window::HitRecord& h : w->hitsPrev_)
        {
            if (best && h.layer < best->layer)
                continue;
            Rect r = h.rect.Intersect(h.clip);
            if (!h.fixed)
            {
                Vec2 total, own;
                ScrollOffsets(*w, w->childrenPrev_, h.child, total, own);
                const Vec2 moved = total - h.total, movedOwn = own - h.own;
                r = h.rect.Translated(-moved).Intersect(h.clip.Translated(movedOwn - moved));
            }
            if (r.Contains(p))
                best = &h;
        }
        if (best)
        {
            hitId_ = best->id;
            hitFlags_ = best->flags;
        }
        int child = -1;
        for (std::size_t i = 0; i < w->childrenPrev_.size(); ++i)
            if ((child < 0 || w->childrenPrev_[i].layer >= w->childrenPrev_[(std::size_t)child].layer) && ChildClipNow(*w, i).Contains(p))
                child = (int)i;
        if (child >= 0)
            hoveredChild_ = w->childrenPrev_[(std::size_t)child].id;
    }

    void Context::UpdatePressOwners()
    {
        for (int b = 0; b < (int)MouseButton::Count; ++b)
        {
            const MouseButton mb = (MouseButton)b;
            if (input_.MouseClicked(mb))
                pressOwner_[b] = ((hoveredWindow_ && hoveredWindow_ != root_) || hitId_ != 0) ? PressOwner::Ui : PressOwner::Host;
            else if (!input_.MouseDown(mb) && !input_.MouseReleased(mb))
                pressOwner_[b] = PressOwner::None;
        }

        // A click outside the open popups closes the ones above the window it landed on (all of them when it is not
        // a popup). The dismissing press may belong to the popup: then it does nothing else.
        bool clicked = false;
        for (int b = 0; b < (int)MouseButton::Count; ++b)
            clicked = clicked || input_.MouseClicked((MouseButton)b);
        if (!clicked || popups_.empty())
            return;
        std::size_t keep = 0;
        for (std::size_t i = 0; i < popups_.size(); ++i)
            if (hoveredWindow_ && hoveredWindow_->id_ == popups_[i].window)
                keep = i + 1;
        if (keep == popups_.size())
            return;
        const bool consume = popups_[keep].consumeClickAway;
        ClosePopupsFrom(keep);
        if (consume)
            for (int b = 0; b < (int)MouseButton::Count; ++b)
                if (input_.MouseClicked((MouseButton)b))
                    pressOwner_[b] = PressOwner::Dismiss;
    }

    bool Context::PressBlocked() const
    {
        for (int b = 0; b < (int)MouseButton::Count; ++b)
            if (input_.MouseDown((MouseButton)b) && (pressOwner_[b] == PressOwner::Host || pressOwner_[b] == PressOwner::Dismiss))
                return true;
        return false;
    }

    // ------------------------------------------------------------------ move / resize
    int Context::ResizeEdgesAt(const Window& w, Vec2 p) const
    {
        if (w.flags_ & WindowFlags_NoResize)
            return 0;
        const float b = desc_.layout.resizeBorder;
        const Rect& r = w.rect_;
        if (!r.Expanded(b).Contains(p))
            return 0;
        int e = 0;
        if (p.x < r.min.x + b)
            e |= kEdgeLeft;
        else if (p.x >= r.max.x - b)
            e |= kEdgeRight;
        if (p.y < r.min.y + b)
            e |= kEdgeTop;
        else if (p.y >= r.max.y - b)
            e |= kEdgeBottom;
        return e;
    }

    void Context::StartResize()
    {
        Window* w = hoveredWindow_;
        if (!w || dragWindow_ || activeId_ != 0 || PressBlocked())
            return;
        const Vec2 p = input_.MousePos();
        // an item at the edge keeps the mouse; outside the window the border band is the resize handle's
        if (hitId_ != 0 && w->rect_.Contains(p))
            return;
        const int edges = ResizeEdgesAt(*w, p);
        if (edges == 0)
            return;
        requests_.cursor = ResizeCursor(edges);
        if (input_.MouseClicked(MouseButton::Left) && pressOwner_[(int)MouseButton::Left] == PressOwner::Ui)
        {
            dragWindow_ = w;
            resizeEdges_ = edges;
            // where on the edge it was grabbed: the edge keeps that distance to the pointer (no jump on the first frame)
            const Rect& r = w->rect_;
            dragOffset_ = Vec2((edges & kEdgeLeft) ? p.x - r.min.x : p.x - r.max.x, (edges & kEdgeTop) ? p.y - r.min.y : p.y - r.max.y);
            SetActiveId(ResizeId(*w));
            FocusWindow(w);
        }
    }

    void Context::StartWindowMove(Window* w, Vec2 grab)
    {
        if (!w || !input_.MouseDown(MouseButton::Left))
            return;
        dragWindow_ = w;
        resizeEdges_ = 0;
        dragOffset_ = grab;
        SetActiveId(MoveId(*w));
        FocusWindow(w);
    }

    void Context::UpdateMoveResize()
    {
        if (!dragWindow_)
            return;
        Window& w = *dragWindow_;
        const Id expected = resizeEdges_ ? ResizeId(w) : MoveId(w);
        const bool held = input_.MouseDown(MouseButton::Left);
        // A quick drag's moves and its release can arrive in one frame (a slow frame): the position of the release
        // is where the window goes, then the drag ends. A canceled release (focus lost) keeps the last position.
        const bool released = !held && input_.MouseReleased(MouseButton::Left) && !input_.MouseCanceled(MouseButton::Left);
        if (activeId_ != expected || !(held || released) || !input_.MouseValid())
        {
            EndMoveResize(expected);
            return;
        }
        activeAlive_ = true;
        ApplyMoveResize(w, input_.MousePos() - dragOffset_);
        if (released)
            EndMoveResize(expected);
    }

    void Context::EndMoveResize(Id expected)
    {
        if (activeId_ == expected)
            activeId_ = 0;
        dragWindow_ = nullptr;
        resizeEdges_ = 0;
    }

    void Context::ApplyMoveResize(Window& w, Vec2 p)
    {
        if (resizeEdges_ == 0)
        {
            w.rect_ = Rect::FromSize(p, w.rect_.Size());
            return;
        }
        Rect r = w.rect_;
        if (resizeEdges_ & kEdgeLeft)
            r.min.x = Clamp(p.x, r.max.x - w.maxSize_.x, r.max.x - w.minSize_.x);
        if (resizeEdges_ & kEdgeRight)
            r.max.x = Clamp(p.x, r.min.x + w.minSize_.x, r.min.x + w.maxSize_.x);
        if (resizeEdges_ & kEdgeTop)
            r.min.y = Clamp(p.y, r.max.y - w.maxSize_.y, r.max.y - w.minSize_.y);
        if (resizeEdges_ & kEdgeBottom)
            r.max.y = Clamp(p.y, r.min.y + w.minSize_.y, r.min.y + w.maxSize_.y);
        w.rect_ = r;
        requests_.cursor = ResizeCursor(resizeEdges_);
    }

    // ------------------------------------------------------------------ popups and tooltips
    int Context::PopupIndex(Id id) const
    {
        for (std::size_t i = 0; i < popups_.size(); ++i)
            if (popups_[i].id == id)
                return (int)i;
        return -1;
    }

    void Context::ClosePopupsFrom(std::size_t index)
    {
        if (index < popups_.size())
            popups_.erase(popups_.begin() + (std::ptrdiff_t)index, popups_.end());
    }

    void Context::UpdatePopupsAtNewFrame()
    {
        // a popup that was not submitted last frame (and was not opened then) is closed, with the ones above it
        for (std::size_t i = 0; i < popups_.size(); ++i)
            if (popups_[i].lastFrame + 1 < frame_ && popups_[i].openFrame + 1 < frame_)
            {
                ClosePopupsFrom(i);
                break;
            }
    }

    void Context::OpenPopup(Id id)
    {
        // opened from inside a popup: it stays with the popups up to that one (a submenu replaces its sibling);
        // from anywhere else it replaces every open popup
        std::size_t keep = 0;
        if (!popupStack_.empty())
        {
            const int parent = PopupIndex(popupStack_.back());
            keep = parent < 0 ? 0 : (std::size_t)parent + 1;
        }
        if (keep < popups_.size() && popups_[keep].id == id)
        {
            ClosePopupsFrom(keep + 1);   // already open at this level: keep it where it is
            return;
        }
        ClosePopupsFrom(keep);
        PopupEntry e;
        e.id = id;
        e.window = HashString("##popup", id);
        e.openPos = input_.MouseValid() ? input_.MousePos() : Vec2(0, 0);
        e.openFrame = frame_;
        popups_.push_back(e);
    }

    void Context::ClosePopup(Id id)
    {
        const int i = PopupIndex(id);
        if (i >= 0)
            ClosePopupsFrom((std::size_t)i);
    }

    bool Context::IsPopupOpen(Id id) const { return PopupIndex(id) >= 0; }

    bool Context::BeginPopup(Id id, const PopupOptions& options)
    {
        const int index = PopupIndex(id);
        if (index < 0)
            return false;
        PopupEntry& e = popups_[(std::size_t)index];
        e.lastFrame = frame_;
        e.consumeClickAway = options.consumeClickAway;
        WindowOptions wo;
        wo.flags = WindowFlags_NoMove | WindowFlags_NoResize | WindowFlags_AutoSize;
        wo.layer = WindowLayer::Overlay;
        wo.minSize = options.minSize;
        wo.padding = options.padding;
        const Vec2 anchor = options.pos.x != kNoMousePos ? options.pos : e.openPos;
        SetNextWindowPos(anchor, Cond::Always, options.pivot);
        Window* w = BeginWindow(e.window, "##popup", wo, true);
        if (e.openFrame == frame_ || w->appearing_)
            FocusWindow(w);
        popupStack_.push_back(id);
        return true;
    }

    void Context::EndPopup()
    {
        ESIA_ASSERT(!popupStack_.empty() && "EndPopup without BeginPopup");
        if (!popupStack_.empty())
            popupStack_.pop_back();
        End();
    }

    void Context::CloseCurrentPopup()
    {
        if (!popupStack_.empty())
            ClosePopup(popupStack_.back());
    }

    bool Context::BeginTooltip()
    {
        WindowOptions wo;
        wo.flags = WindowFlags_NoInputs | WindowFlags_NoMove | WindowFlags_NoResize | WindowFlags_NoFocus | WindowFlags_NoBringToFront |
                   WindowFlags_AutoSize;
        wo.layer = WindowLayer::Tooltip;
        wo.minSize = Vec2(0, 0);
        Window* w = FindWindowByName(kTooltipName);
        if (!w || !w->active_)
        {
            const Vec2 at = input_.MouseValid() ? input_.MousePos() + desc_.layout.tooltipOffset : Vec2(0, 0);
            SetNextWindowPos(at);
        }
        BeginWindow(HashLabel(kTooltipName, 0), kTooltipName, wo, true);
        return true;
    }

    void Context::EndTooltip() { End(); }
}
