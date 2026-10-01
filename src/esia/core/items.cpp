// Esia - UI core items: ids, registration, hover from the hit test, button behavior, keyboard focus, key ownership,
// item status and scope data (see esia/core/context.hpp and docs/UI_CORE.md, sections 4, 5 and 10).
#include "esia/core/context.hpp"
#include <algorithm>
#include <cmath>

namespace esia
{
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
    void Context::ScrollOffsets(const Window& w, const std::vector<Window::ChildRecord>& records, int index, Vec2& total, Vec2& own) const
    {
        // the scroll offsets that move content in child record `index`: its scroll areas out to the window, stopping at
        // a floating region (it floats over its parents' content, their scroll does not move it)
        total = Vec2(0, 0);
        bool haveOwn = false;
        for (int i = index; i >= 0 && (std::size_t)i < records.size(); i = records[(std::size_t)i].parent)
        {
            const Window::ChildRecord& c = records[(std::size_t)i];
            if (c.flags & (ChildFlags_ScrollX | ChildFlags_ScrollY))
            {
                const auto it = w.childStates_.find(c.id);
                const Vec2 v = it != w.childStates_.end() ? it->second.scroll.scroll : Vec2(0, 0);
                total += v;
                if (!haveOwn)
                    own = v;
                haveOwn = true;
            }
            if (c.flags & ChildFlags_Floating)
            {
                if (!haveOwn)
                    own = Vec2(0, 0);
                return;
            }
        }
        total += w.scroll_.scroll;
        if (!haveOwn)
            own = w.scroll_.scroll;
    }

    void Context::RecordHit(Window& w, Id id, const Rect& bb, std::uint32_t itemFlags, bool fixed)
    {
        // what the next frame's hit test sees: the item inside the clip, on its layer (floating children above
        // content, background items below the rest of their layer), with the scroll offsets it was laid out with
        const Rect clip = w.drawList_.ClipRect();
        if (bb.Intersect(clip).Empty())
            return;
        Window::HitRecord h;
        h.id = id;
        h.rect = bb;
        h.clip = clip;
        h.flags = itemFlags;
        h.layer = w.floating_ * 2 + ((itemFlags & ItemFlags_Background) ? 0 : 1);
        h.child = w.childStack_.empty() ? -1 : w.childStack_.back();
        h.fixed = fixed;
        ScrollOffsets(w, w.children_, h.child, h.total, h.own);
        if (!w.hits_.empty() && w.hits_.back().id == id)
            w.hits_.back() = h;
        else
            w.hits_.push_back(h);
    }

    bool Context::ItemAdd(Id id, const Rect& bb, std::uint32_t itemFlags)
    {
        Window* w = CurrentWindow();
        ESIA_ASSERT(w);
        lastItem_ = LastItem();
        lastItem_.id = id;
        lastItem_.rect = bb;
        lastItem_.flags = itemFlags;
        const bool disabled = (itemFlags & ItemFlags_Disabled) != 0;
        if (id != 0)
        {
            if (id == activeId_)
            {
                // an item disabled while it is active (held, dragged, editing) lets go
                if (disabled)
                    ClearActiveId();
                else
                    activeAlive_ = true;
            }
            if (id == focusId_)
                focusAlive_ = true;
            if ((itemFlags & ItemFlags_Focusable) && !disabled)
                focusOrder_.push_back(id);
            // a disabled item claims the hover even when its widget asks nothing more
            if (disabled)
                RecordHit(*w, id, bb, itemFlags);
        }
        lastItem_.visible = bb.Overlaps(w->drawList_.ClipRect());
        return lastItem_.visible;
    }

    bool Context::ItemHoverable(Id id, const Rect& bb, std::uint32_t itemFlags)
    {
        Window* w = CurrentWindow();
        if (!w || id == 0)
            return false;
        if (id == lastItem_.id)
            itemFlags |= lastItem_.flags;
        RecordHit(*w, id, bb, itemFlags);
        if (hoveredWindow_ != w || !input_.MouseValid() || PressBlocked())
            return false;
        const Vec2 p = input_.MousePos();
        if (!bb.Intersect(w->drawList_.ClipRect()).Contains(p))
            return false;
        if (activeId_ != 0 && activeId_ != id)
            return false;
        // The front-most of last frame's items under the mouse takes the hover (the active item keeps it while the
        // mouse is over it). Where last frame had no item, the first new item here takes it: an item appearing under
        // a still mouse is not a frame late.
        bool claimed;
        if (activeId_ == id)
            claimed = true;
        else if (hitId_ != 0)
            claimed = hitId_ == id;
        else
        {
            if (newItemClaim_ == 0)
                newItemClaim_ = id;
            claimed = newItemClaim_ == id;
        }
        if (!claimed || (itemFlags & ItemFlags_Disabled))
            return false;
        hoveredId_ = id;
        if (id == lastItem_.id)
            lastItem_.hovered = true;
        return true;
    }

    ButtonResult Context::ButtonBehavior(Id id, const Rect& bb, std::uint32_t flags, std::uint32_t itemFlags)
    {
        ButtonResult r;
        if (id == lastItem_.id)
            itemFlags |= lastItem_.flags;
        if (itemFlags & ItemFlags_Disabled)
        {
            // claims the hover (ItemHoverable records it), never hovers or activates
            if (id != 0 && activeId_ == id)
                ClearActiveId();
            ItemHoverable(id, bb, itemFlags);
            return r;
        }
        const std::uint32_t buttons = (flags & (ButtonFlags_MouseLeft | ButtonFlags_MouseRight | ButtonFlags_MouseMiddle)) ? flags : (flags | ButtonFlags_MouseLeft);
        const bool repeat = (flags & ButtonFlags_Repeat) != 0;
        r.hovered = ItemHoverable(id, bb, itemFlags);

        bool clickedNow = false;
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
                activeByMouse_ = true;
                clickedNow = true;
                r.clicks = input_.MouseClickCount(mb);
                // a finger's press may yet be a scroll (ActiveIdYieldsToScroll): an item that acts on the press gets
                // it once it is not (below)
                activeYields_ = input_.MouseTouch(mb);
                activeDeferred_ = activeYields_ && ((flags & (ButtonFlags_PressOnClick | ButtonFlags_PressOnDoubleClick)) || repeat);
                if (activeDeferred_)
                    break;
                if (flags & ButtonFlags_FocusOnClick)
                    SetKeyboardFocusId(id);
                if ((flags & ButtonFlags_PressOnClick) || repeat)
                    r.pressed = true;
                if ((flags & ButtonFlags_PressOnDoubleClick) && r.clicks == 2)
                    r.pressed = true;
                break;
            }
        }

        // an item made active another way (a key, a field that stays active) is not held by a mouse button: no
        // "release" presses or deactivates it
        if (activeId_ == id && id != 0 && !activeByMouse_)
            activeAlive_ = true;
        else if (activeId_ == id && id != 0 && activeDeferred_)
        {
            // A finger's press on an item that acts on the press (a slider, a segmented control, a field, a repeat
            // button), held back while it may be a scroll: the item gets it when the finger lifts (a tap), rests
            // (touchDelay), or moves past the slop across - or along, a frame after a scroll area could have taken
            // it (the areas end after their items). Until then the item is hovered, not held.
            activeAlive_ = true;
            const bool down = input_.MouseDown(activeButton_);
            const Vec2 d = input_.MousePos() - input_.MouseClickedPos(activeButton_);
            const bool past = std::max(std::fabs(d.x), std::fabs(d.y)) > input_.config.touchSlop;
            const bool give = !down || (!past && input_.MouseDownDuration(activeButton_) >= input_.config.touchDelay) ||
                              (past && (std::fabs(d.x) >= std::fabs(d.y) || activeSlopSeen_));
            activeSlopSeen_ = activeSlopSeen_ || past;
            if (give)
            {
                activeDeferred_ = activeYields_ = false;
                r.clicks = input_.MouseClickCount(activeButton_);
                if (!(down || (r.hovered && !input_.MouseCanceled(activeButton_))))
                    ClearActiveId();   // lifted elsewhere, or by focus loss: nothing
                else
                {
                    // the press lands now (lifted: for this frame, held where the tap was)
                    if (flags & ButtonFlags_FocusOnClick)
                        SetKeyboardFocusId(id);
                    r.pressed = (flags & ButtonFlags_PressOnClick) || repeat || ((flags & ButtonFlags_PressOnDoubleClick) && r.clicks == 2);
                    r.held = true;
                    if (!down)
                        ClearActiveId();
                }
            }
        }
        else if (activeId_ == id && id != 0)
        {
            activeAlive_ = true;
            if (input_.MouseDown(activeButton_))
            {
                r.held = true;
                r.clicks = input_.MouseClickCount(activeButton_);
                if (repeat && r.hovered && !clickedNow)
                {
                    // the click pressed; then key-repeat timing: after the delay, once per rate period
                    const float t = input_.MouseDownDuration(activeButton_) - input_.config.keyRepeatDelay;
                    const float t0 = t - input_.DeltaTime();
                    const float rate = input_.config.keyRepeatRate;
                    if (t >= 0.0f && (t0 < 0.0f || std::floor(t / rate) != std::floor(t0 / rate)))
                        r.pressed = true;
                }
            }
            else
            {
                // released over the item presses, unless the item pressed on the click already or the release came
                // from focus loss
                const bool onRelease = !(flags & (ButtonFlags_PressOnClick | ButtonFlags_PressOnDoubleClick)) && !repeat;
                if (r.hovered && onRelease && !input_.MouseCanceled(activeButton_))
                    r.pressed = true;
                ClearActiveId();
            }
        }
        if (id != 0 && id == lastItem_.id)
        {
            lastItem_.hovered = r.hovered;
            lastItem_.pressed = r.pressed;
        }
        return r;
    }

    ItemStatus Context::LastItemStatus() const
    {
        ItemStatus s;
        s.hovered = lastItem_.hovered;
        s.active = lastItem_.id != 0 && activeId_ == lastItem_.id;
        s.focused = lastItem_.id != 0 && focusId_ == lastItem_.id;
        s.disabled = (lastItem_.flags & ItemFlags_Disabled) != 0;
        s.pressed = lastItem_.pressed;
        s.visible = lastItem_.visible;
        s.rect = lastItem_.rect;
        return s;
    }

    ItemStatus Context::ItemStatusOf(Id id) const
    {
        if (id != 0 && id == lastItem_.id)
            return LastItemStatus();
        ItemStatus s;
        if (id == 0)
            return s;
        s.hovered = hoveredId_ == id;
        s.active = activeId_ == id;
        s.focused = focusId_ == id;
        // rect and flags from this frame's records, else last frame's
        if (const Window* w = CurrentWindow())
            for (const auto* list : {&w->hits_, &w->hitsPrev_})
            {
                const auto it = std::find_if(list->rbegin(), list->rend(), [id](const Window::HitRecord& h) { return h.id == id; });
                if (it != list->rend())
                {
                    s.rect = it->rect.Intersect(it->clip);
                    s.disabled = (it->flags & ItemFlags_Disabled) != 0;
                    s.visible = list == &w->hits_;
                    break;
                }
            }
        return s;
    }

    void Context::SetActiveId(Id id)
    {
        activeId_ = id;
        activeByMouse_ = false;
        activeYields_ = activeDeferred_ = activeSlopSeen_ = false;
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
    }

    // ------------------------------------------------------------------ keys
    // Ownership never depends on the order widgets are submitted in: a claim is refused when another id claimed the
    // key earlier this frame, or owned it last frame (it keeps it while it renews the claim; a newcomer gets it the
    // frame after the old owner stops). So two claimants never both see a key.
    void Context::ClaimKey(Key key, Id owner)
    {
        if ((std::size_t)key >= keyOwner_.size() || owner == 0)
            return;
        const std::size_t k = (std::size_t)key;
        anyKeyClaim_ = true;
        if (keyOwner_[k] == 0 && (keyOwnerPrev_[k] == 0 || keyOwnerPrev_[k] == owner))
            keyOwner_[k] = owner;
    }

    void Context::ClaimKeyboard(Id owner)
    {
        if (owner == 0)
            return;
        anyKeyClaim_ = true;
        if (keyboardOwner_ == 0 && (keyboardOwnerPrev_ == 0 || keyboardOwnerPrev_ == owner))
            keyboardOwner_ = owner;
    }

    Id Context::KeyOwner(Key key) const
    {
        // The keyboard's owner (the editor that edits) outranks per-key claims: a dialog's Enter does not take the
        // Enter of the multiline field inside it. This frame's claims, then last frame's (they still hold for the
        // widgets submitted before their owner renews them).
        if ((std::size_t)key >= keyOwner_.size())
            return 0;
        const std::size_t k = (std::size_t)key;
        if (keyboardOwner_ != 0)
            return keyboardOwner_;
        if (keyboardOwnerPrev_ != 0)
            return keyboardOwnerPrev_;
        if (keyOwner_[k] != 0)
            return keyOwner_[k];
        return keyOwnerPrev_[k];
    }

    bool Context::KeyPressed(Key key, Id asker, bool repeat) const
    {
        const Id owner = KeyOwner(key);
        if (owner != 0 && owner != asker)
            return false;
        return input_.KeyPressed(key, repeat);
    }

    bool Context::KeyDown(Key key, Id asker) const
    {
        const Id owner = KeyOwner(key);
        if (owner != 0 && owner != asker)
            return false;
        return input_.KeyDown(key);
    }

    void Context::UpdateTabNavigation()
    {
        // after the widgets: an editor that keeps Tab claimed it, and this frame's focusable items are all known
        if (!input_.KeyPressed(Key::Tab) || KeyOwner(Key::Tab) != 0 || focusOrder_.empty() || activeId_ != 0)
            return;
        const bool back = (input_.Mods() & Mod_Shift) != 0;
        const auto it = std::find(focusOrder_.begin(), focusOrder_.end(), focusId_);
        const int n = (int)focusOrder_.size();
        int i = it == focusOrder_.end() ? (back ? n - 1 : 0) : (int)(it - focusOrder_.begin()) + (back ? -1 : 1);
        i = (i % n + n) % n;
        focusId_ = focusOrder_[(std::size_t)i];
        focusAlive_ = true;   // alive until the item is submitted next frame
        focusClaimed_ = true;
    }

    // ------------------------------------------------------------------ scope data
    void Context::SetScopeData(const void* key, const void* value)
    {
        auto& scope = CurFrame().scope;
        for (auto& e : scope)
            if (e.first == key)
            {
                e.second = value;
                return;
            }
        scope.emplace_back(key, value);
    }

    const void* Context::FindScopeData(const void* key) const
    {
        // the innermost container first, out to the window, then the windows it was begun in (a popup inherits)
        for (auto w = stack_.rbegin(); w != stack_.rend(); ++w)
            for (auto f = (*w)->frames_.rbegin(); f != (*w)->frames_.rend(); ++f)
                for (const auto& e : f->scope)
                    if (e.first == key)
                        return e.second;
        return nullptr;
    }
}
