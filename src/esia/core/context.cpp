// Esia - UI core context (see esia/core/context.hpp)
#include "esia/core/context.hpp"
#include <algorithm>

namespace esia
{
    namespace
    {
        constexpr std::string_view kRootName = "##root";
        constexpr int kEdgeLeft = 1, kEdgeRight = 2, kEdgeTop = 4, kEdgeBottom = 8;
    }

    Context::Context(const ContextDesc& desc) : desc_(desc) { input_.config = desc.input; }

    Context::~Context() = default;

    void Context::QueueInput(InputEvent e)
    {
        std::lock_guard lock(inputMutex_);
        queued_.push_back(std::move(e));
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
        }

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
        hoveredIdPrev_ = hoveredId_;
        hoveredId_ = 0;
        hoveredAllowOverlap_ = false;
        focusOrderPrev_.swap(focusOrder_);
        focusOrder_.clear();
        lastItemId_ = 0;
        lastItemRect_ = Rect();
        lastItemHovered_ = false;
        requests_ = PlatformRequests();

        for (auto& w : windows_)
        {
            w->wasActive_ = w->active_;
            w->active_ = false;
        }

        UpdateMoveResize();
        UpdateHoveredWindow();
        StartResize();
        UpdateScroll();
        UpdateFocusNavigation();

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

        // a click that no item took: focus the window under the mouse, and move it by its empty area
        if (input_.MouseClicked(MouseButton::Left))
        {
            Window* w = hoveredWindow_;
            if (w && w != root_)
            {
                FocusWindow(w);
                if (hoveredId_ == 0 && activeId_ == 0 && !(w->flags_ & WindowFlags_NoMove))
                {
                    dragWindow_ = w;
                    dragOffset_ = input_.MousePos() - w->rect_.min;
                    resizeEdges_ = 0;
                    SetActiveId(MoveId(*w));
                }
            }
            else if (!w || w == root_)
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
            if (!w->drawList_.Empty())
                drawData_.lists.push_back(&w->drawList_);
        if (!foreground_.Empty())
            drawData_.lists.push_back(&foreground_);

        requests_.wantCaptureMouse = activeId_ != 0 || (hoveredWindow_ && hoveredWindow_ != root_) || hoveredId_ != 0;
        requests_.wantCaptureKeyboard = focusId_ != 0 || activeId_ != 0;
        requests_.wantTextInput = textInputRequested_ && focusId_ != 0;
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
    Window* Context::FindWindow(std::string_view name) const
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

    void Context::SetNextWindowPos(Vec2 pos, Cond cond)
    {
        nextPos_ = pos;
        nextPosCond_ = cond;
        hasNextPos_ = true;
    }

    void Context::SetNextWindowSize(Vec2 size, Cond cond)
    {
        nextSize_ = size;
        nextSizeCond_ = cond;
        hasNextSize_ = true;
    }

    bool Context::Begin(std::string_view name, const WindowOptions& options)
    {
        ESIA_ASSERT(inFrame_ && "Begin outside NewFrame / EndFrame");
        const Id id = HashLabel(name, 0);
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
            if (hasNextPos_ && (nextPosCond_ == Cond::Always || created))
                w->rect_ = Rect::FromSize(nextPos_, w->rect_.Size());
            if (hasNextSize_ && (nextSizeCond_ == Cond::Always || created))
                w->rect_.max = w->rect_.min + nextSize_;
            const Vec2 sz(Clamp(w->rect_.Width(), w->minSize_.x, w->maxSize_.x), Clamp(w->rect_.Height(), w->minSize_.y, w->maxSize_.y));
            w->rect_.max = w->rect_.min + sz;
            w->active_ = true;
            w->lastFrame_ = frame_;
            if (created && !(w->flags_ & WindowFlags_NoFocus))
                focusedWindow_ = w;

            // clamp the scroll to what the content measured last frame
            w->scroll_.x = Clamp(w->scroll_.x, 0.0f, w->scrollMax_.x);
            w->scroll_.y = Clamp(w->scroll_.y, 0.0f, w->scrollMax_.y);
            const Rect content = w->ContentRect();
            w->drawList_.Reset(w->rect_);
            w->drawList_.PushClipRect(w->rect_);
            w->idStack_.assign(1, id);
            w->cursorStart_ = content.min - w->scroll_;
            w->cursor_ = w->cursorStart_;
            w->cursorMax_ = w->cursorStart_;
            w->prevLineEnd_ = w->cursorStart_;
            w->lineHeight_ = w->prevLineHeight_ = 0.0f;
            w->indent_ = 0.0f;
            w->sameLine_ = false;
            w->groups_.clear();
        }
        hasNextPos_ = hasNextSize_ = false;
        stack_.push_back(w);
        return !w->rect_.Empty();
    }

    void Context::End()
    {
        ESIA_ASSERT(!stack_.empty() && "End without Begin");
        if (stack_.empty())
            return;
        Window* w = stack_.back();
        // what the content measured (scroll range for the next frame)
        const Rect content = w->ContentRect();
        w->contentSize_ = w->cursorMax_ - w->cursorStart_;
        w->scrollMax_ = Vec2(std::max(0.0f, w->contentSize_.x - content.Width()), std::max(0.0f, w->contentSize_.y - content.Height()));
        stack_.pop_back();
    }

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
                if ((int)w->layer_ == layer && w->wasActive_)
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
        if (!w || dragWindow_ || activeId_ != 0)
            return;
        const int edges = ResizeEdgesAt(*w, input_.MousePos());
        if (edges == 0)
            return;
        const bool diag = (edges & (kEdgeLeft | kEdgeRight)) && (edges & (kEdgeTop | kEdgeBottom));
        if (diag)
            requests_.cursor = (edges == (kEdgeLeft | kEdgeTop) || edges == (kEdgeRight | kEdgeBottom)) ? MouseCursor::ResizeNWSE : MouseCursor::ResizeNESW;
        else
            requests_.cursor = (edges & (kEdgeLeft | kEdgeRight)) ? MouseCursor::ResizeEW : MouseCursor::ResizeNS;
        if (input_.MouseClicked(MouseButton::Left))
        {
            dragWindow_ = w;
            resizeEdges_ = edges;
            SetActiveId(ResizeId(*w));
            FocusWindow(w);
        }
    }

    void Context::UpdateMoveResize()
    {
        if (!dragWindow_)
            return;
        Window& w = *dragWindow_;
        const Id expected = resizeEdges_ ? ResizeId(w) : MoveId(w);
        if (activeId_ != expected || !input_.MouseDown(MouseButton::Left))
        {
            if (activeId_ == expected)
                activeId_ = 0;
            dragWindow_ = nullptr;
            resizeEdges_ = 0;
            return;
        }
        activeAlive_ = true;
        const Vec2 p = input_.MousePos();
        if (resizeEdges_ == 0)
        {
            w.rect_ = Rect::FromSize(p - dragOffset_, w.rect_.Size());
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
        requests_.cursor = (resizeEdges_ & (kEdgeLeft | kEdgeRight)) && (resizeEdges_ & (kEdgeTop | kEdgeBottom))
                               ? ((resizeEdges_ == (kEdgeLeft | kEdgeTop) || resizeEdges_ == (kEdgeRight | kEdgeBottom)) ? MouseCursor::ResizeNWSE : MouseCursor::ResizeNESW)
                               : ((resizeEdges_ & (kEdgeLeft | kEdgeRight)) ? MouseCursor::ResizeEW : MouseCursor::ResizeNS);
    }

    void Context::UpdateScroll()
    {
        const Vec2 wheel = input_.Wheel();
        if ((wheel.x == 0.0f && wheel.y == 0.0f) || !hoveredWindow_ || (hoveredWindow_->flags_ & WindowFlags_NoScroll) || activeId_ != 0)
            return;
        Window& w = *hoveredWindow_;
        const float step = desc_.layout.scrollStep;
        // shift + wheel scrolls horizontally, as on every desktop platform
        const bool shift = (input_.Mods() & Mod_Shift) != 0;
        const float dx = shift ? wheel.y : wheel.x, dy = shift ? 0.0f : wheel.y;
        w.scroll_.x = Clamp(w.scroll_.x - dx * step, 0.0f, w.scrollMax_.x);
        w.scroll_.y = Clamp(w.scroll_.y - dy * step, 0.0f, w.scrollMax_.y);
    }

    void Context::UpdateFocusNavigation()
    {
        if (!input_.KeyPressed(Key::Tab) || focusOrderPrev_.empty() || activeId_ != 0)
            return;
        const bool back = (input_.Mods() & Mod_Shift) != 0;
        const auto it = std::find(focusOrderPrev_.begin(), focusOrderPrev_.end(), focusId_);
        const int n = (int)focusOrderPrev_.size();
        int i = it == focusOrderPrev_.end() ? (back ? n - 1 : 0) : (int)(it - focusOrderPrev_.begin()) + (back ? -1 : 1);
        i = (i % n + n) % n;
        focusId_ = focusOrderPrev_[(std::size_t)i];
        focusAlive_ = true;   // alive until the item is submitted this frame
        focusClaimed_ = true;
    }

    // ------------------------------------------------------------------ ids
    Id Context::IdSeed() const
    {
        const Window* w = CurrentWindow();
        return w && !w->idStack_.empty() ? w->idStack_.back() : 0;
    }

    void Context::PushId(std::string_view label)
    {
        Window* w = CurrentWindow();
        ESIA_ASSERT(w);
        w->idStack_.push_back(HashLabel(label, IdSeed()));
    }

    void Context::PushId(std::int64_t value)
    {
        Window* w = CurrentWindow();
        ESIA_ASSERT(w);
        w->idStack_.push_back(HashInt(value, IdSeed()));
    }

    void Context::PopId()
    {
        Window* w = CurrentWindow();
        ESIA_ASSERT(w && w->idStack_.size() > 1 && "PopId without PushId");
        if (w && w->idStack_.size() > 1)
            w->idStack_.pop_back();
    }

    Id Context::GetId(std::string_view label) const { return HashLabel(label, IdSeed()); }
    Id Context::GetId(std::int64_t value) const { return HashInt(value, IdSeed()); }

    // ------------------------------------------------------------------ items
    bool Context::ItemAdd(Id id, const Rect& bb, std::uint32_t itemFlags)
    {
        Window* w = CurrentWindow();
        ESIA_ASSERT(w);
        lastItemId_ = id;
        lastItemRect_ = bb;
        lastItemHovered_ = false;
        if (id != 0)
        {
            if (id == activeId_)
                activeAlive_ = true;
            if (id == focusId_)
                focusAlive_ = true;
            if ((itemFlags & ItemFlags_Focusable) && !(itemFlags & ItemFlags_Disabled))
                focusOrder_.push_back(id);
        }
        const Rect& clip = w->drawList_.ClipRect();
        return bb.Overlaps(clip);
    }

    void Context::ItemSize(Vec2 size)
    {
        Window* w = CurrentWindow();
        ESIA_ASSERT(w);
        const float lineHeight = std::max(w->lineHeight_, size.y);
        w->prevLineEnd_ = Vec2(w->cursor_.x + size.x, w->cursor_.y);
        w->prevLineHeight_ = lineHeight;
        w->cursorMax_.x = std::max(w->cursorMax_.x, w->cursor_.x + size.x);
        w->cursorMax_.y = std::max(w->cursorMax_.y, w->cursor_.y + lineHeight);
        w->cursor_ = Vec2(w->cursorStart_.x + w->indent_, w->cursor_.y + lineHeight + desc_.layout.itemSpacing.y);
        w->lineHeight_ = 0.0f;
        w->sameLine_ = false;
    }

    bool Context::ItemHoverable(Id id, const Rect& bb, std::uint32_t itemFlags)
    {
        Window* w = CurrentWindow();
        if (!w || hoveredWindow_ != w || (itemFlags & ItemFlags_Disabled) || !input_.MouseValid())
            return false;
        const Vec2 p = input_.MousePos();
        if (!bb.Intersect(w->drawList_.ClipRect()).Contains(p))
            return false;
        if (activeId_ != 0 && activeId_ != id)
            return false;
        if (hoveredId_ != 0 && hoveredId_ != id && !hoveredAllowOverlap_)
            return false;
        // an overlappable item yields to the item that took the hover last frame
        if ((itemFlags & ItemFlags_AllowOverlap) && hoveredIdPrev_ != 0 && hoveredIdPrev_ != id && activeId_ != id)
        {
            return false;
        }
        hoveredId_ = id;
        hoveredAllowOverlap_ = (itemFlags & ItemFlags_AllowOverlap) != 0;
        if (id == lastItemId_)
            lastItemHovered_ = true;
        return true;
    }

    ButtonResult Context::ButtonBehavior(Id id, const Rect& bb, std::uint32_t flags)
    {
        ButtonResult r;
        const std::uint32_t buttons = (flags & (ButtonFlags_MouseLeft | ButtonFlags_MouseRight | ButtonFlags_MouseMiddle)) ? flags : (flags | ButtonFlags_MouseLeft);
        r.hovered = ItemHoverable(id, bb, (flags & ButtonFlags_AllowOverlap) ? ItemFlags_AllowOverlap : ItemFlags_None);

        if (r.hovered)
        {
            for (int b = 0; b < 3; ++b)
            {
                if (!(buttons & (ButtonFlags_MouseLeft << b)))
                    continue;
                const MouseButton mb = (MouseButton)b;
                if (!input_.MouseClicked(mb))
                    continue;
                SetActiveId(id);
                activeButton_ = mb;
                if (flags & ButtonFlags_FocusOnClick)
                    SetKeyboardFocusId(id);
                if (flags & ButtonFlags_PressOnClick)
                    r.pressed = true;
                if ((flags & ButtonFlags_PressOnDoubleClick) && input_.MouseDoubleClicked(mb))
                    r.pressed = true;
                break;
            }
        }

        if (activeId_ == id && id != 0)
        {
            activeAlive_ = true;
            if (input_.MouseDown(activeButton_))
            {
                r.held = true;
                if ((flags & ButtonFlags_Repeat) && r.hovered && !input_.MouseClicked(activeButton_))
                {
                    // same timing as key repeat
                    const float t = input_.MouseDownDuration(activeButton_) - input_.config.keyRepeatDelay;
                    const float t0 = t - input_.DeltaTime();
                    const float rate = input_.config.keyRepeatRate;
                    if (t >= 0.0f && (t0 < 0.0f || (int)(t / rate) != (int)(t0 / rate)))
                        r.pressed = true;
                }
            }
            else
            {
                if (r.hovered && !(flags & (ButtonFlags_PressOnClick | ButtonFlags_PressOnDoubleClick)))
                    r.pressed = true;
                ClearActiveId();
            }
        }
        if (id != 0 && id == lastItemId_)
            lastItemHovered_ = r.hovered;
        return r;
    }

    void Context::SetActiveId(Id id)
    {
        activeId_ = id;
        activeSetThisFrame_ = id != 0;
        activeAlive_ = id != 0;
    }

    void Context::KeepAliveId(Id id)
    {
        if (id != 0 && id == activeId_)
            activeAlive_ = true;
        if (id != 0 && id == focusId_)
            focusAlive_ = true;
    }

    void Context::SetKeyboardFocusId(Id id)
    {
        focusId_ = id;
        focusAlive_ = id != 0;
        focusClaimed_ = id != 0;
    }

    void Context::RequestTextInput(const Rect& caret)
    {
        textInputRequested_ = true;
        requests_.imeRect = caret;
        requests_.cursor = requests_.cursor == MouseCursor::Arrow ? MouseCursor::TextInput : requests_.cursor;
    }

    // ------------------------------------------------------------------ layout cursor
    Vec2 Context::CursorPos() const { return CurrentWindow()->cursor_; }

    void Context::SetCursorPos(Vec2 pos)
    {
        Window* w = CurrentWindow();
        w->cursor_ = pos;
        w->cursorMax_.x = std::max(w->cursorMax_.x, pos.x);
        w->cursorMax_.y = std::max(w->cursorMax_.y, pos.y);
    }

    void Context::SameLine(float offsetFromStartX, float spacing)
    {
        Window* w = CurrentWindow();
        const float sp = spacing < 0.0f ? desc_.layout.itemSpacing.x : spacing;
        if (offsetFromStartX != 0.0f)
            w->cursor_ = Vec2(w->cursorStart_.x + offsetFromStartX + (spacing < 0.0f ? 0.0f : spacing), w->prevLineEnd_.y);
        else
            w->cursor_ = Vec2(w->prevLineEnd_.x + sp, w->prevLineEnd_.y);
        w->lineHeight_ = w->prevLineHeight_;
        w->sameLine_ = true;
    }

    void Context::NewLine()
    {
        Window* w = CurrentWindow();
        if (w->lineHeight_ > 0.0f)
            ItemSize(Vec2(0, 0));
        else
            ItemSize(Vec2(0, desc_.layout.itemSpacing.y));
    }

    void Context::Spacing() { ItemSize(Vec2(0, 0)); }

    void Context::Indent(float width)
    {
        Window* w = CurrentWindow();
        const float d = width != 0.0f ? width : desc_.layout.indent;
        w->indent_ += d;
        w->cursor_.x += d;
    }

    void Context::Unindent(float width)
    {
        Window* w = CurrentWindow();
        const float d = width != 0.0f ? width : desc_.layout.indent;
        w->indent_ -= d;
        w->cursor_.x -= d;
    }

    Vec2 Context::ContentRegionAvail() const
    {
        const Window* w = CurrentWindow();
        const Rect content = w->ContentRect();
        // measured against the unscrolled content: the width stays put while scrolling horizontally
        const float maxX = content.max.x - w->scroll_.x;
        return Vec2(std::max(0.0f, maxX - w->cursor_.x), std::max(0.0f, content.max.y - w->cursor_.y));
    }

    void Context::BeginGroup()
    {
        Window* w = CurrentWindow();
        w->groups_.push_back({w->cursor_, w->cursorMax_, w->cursorStart_, w->indent_, w->lineHeight_});
        // the group's content lines start where the group starts
        w->indent_ = w->cursor_.x - w->cursorStart_.x;
        w->cursorMax_ = w->cursor_;
        w->lineHeight_ = 0.0f;
    }

    void Context::EndGroup()
    {
        Window* w = CurrentWindow();
        ESIA_ASSERT(!w->groups_.empty() && "EndGroup without BeginGroup");
        if (w->groups_.empty())
            return;
        const Window::Group g = w->groups_.back();
        w->groups_.pop_back();
        const Rect bb(g.cursor, Vec2(std::max(w->cursorMax_.x, g.cursor.x), std::max(w->cursorMax_.y, g.cursor.y)));
        w->cursor_ = g.cursor;
        w->cursorMax_ = Vec2(std::max(g.cursorMax.x, bb.max.x), std::max(g.cursorMax.y, bb.max.y));
        w->indent_ = g.indent;
        w->lineHeight_ = g.lineHeight;
        ItemSize(bb.Size());
        ItemAdd(0, bb);
    }

    // ------------------------------------------------------------------ clip / scroll
    void Context::PushClipRect(const Rect& r, bool intersect) { CurrentWindow()->drawList_.PushClipRect(r, intersect); }
    void Context::PopClipRect() { CurrentWindow()->drawList_.PopClipRect(); }

    void Context::SetScrollY(float y)
    {
        Window* w = CurrentWindow();
        w->scroll_.y = Clamp(y, 0.0f, w->scrollMax_.y);
    }

    void Context::SetScrollX(float x)
    {
        Window* w = CurrentWindow();
        w->scroll_.x = Clamp(x, 0.0f, w->scrollMax_.x);
    }
}
