// Esia UI - containers: cards, smooth-scrolling areas and liquid-glass windows (WGT's windows.cpp, on the core).
#include "ui_internal.hpp"
#include <algorithm>

namespace esia::ui
{
    using namespace detail;
    using Entry = Ui::Impl::ContainerEntry;

    namespace
    {
        Entry PopEntry(Entry::Kind kind)
        {
            Ui::Impl& m = M();
            ESIA_ASSERT(!m.containers.empty() && m.containers.back().kind == kind && "mismatched End of a ui container");
            (void)kind;
            Entry e = m.containers.back();
            m.containers.pop_back();
            return e;
        }
    }

    namespace detail
    {
        // iOS's scroll edge effect, adding no color. The fades grow with the distance left to scroll: nothing fades at
        // the very top or end. They are at the edges of what is visible: a window reaching past the display fades
        // at the display's edge (as WGT's, whose Dear ImGui clip rects stayed on the display).
        bool BeginScrollEdgeFade(bool allowed)
        {
            Ui::Impl& m = M();
            Context& c = *m.ctx;
            DrawList& dl = c.WindowDrawList();
            m.edgeFades.push_back({&dl, -1});
            if (!allowed)
                return false;
            const Vec2 scroll = c.Scroll(), max = c.ScrollMax();
            const float top = std::min(std::max(scroll.y, 0.0f), Sc(22));
            const float bottom = std::min(std::max(max.y - scroll.y, 0.0f), Sc(30));
            if (top <= 0.5f && bottom <= 0.5f)
                return false;
            Painter p = GetPainter();
            p.BeginEdgeFade(dl.ClipRect().Intersect(Rect(Vec2(0.0f, 0.0f), c.DisplaySize())), top, bottom);
            m.edgeFades.back().fade = (int)dl.Fades().size() - 1;
            return true;
        }

        void EndScrollEdgeFade(bool started)
        {
            Ui::Impl& m = M();
            ESIA_ASSERT(!m.edgeFades.empty() && "EndScrollEdgeFade without BeginScrollEdgeFade");
            if (!m.edgeFades.empty())
                m.edgeFades.pop_back();
            if (!started)
                return;
            Painter p = GetPainter();
            p.EndEdgeFade();
        }

        // iOS's scroll edge effect under a floating bar. The content under the bar showed through its glass at full
        // strength over the upper half of it (the area's own fade is 30 units, the bar and its margin twice that):
        // rows ran into the bar's own text (the search field's placeholder). Now the area's fade ends at the bar's
        // middle and starts a little above the bar - as much as there is left to scroll past it: at the end the
        // room below the content keeps the content clear of the bar, and nothing fades.
        void FadeUnderBar(const Rect& bar, float room)
        {
            Ui::Impl& m = M();
            if (m.edgeFades.empty() || m.edgeFades.back().fade < 0)
                return;
            Context& c = *m.ctx;
            const float left = std::max(c.ScrollMax().y - c.Scroll().y, 0.0f);
            const float s = Saturate(left / std::max(room, 1.0f));
            fx::FadeParams& f = m.edgeFades.back().list->Fades()[(std::size_t)m.edgeFades.back().fade];
            const float edge = bar.Center().y;
            if (s <= 0.0f || edge >= f.y1 || edge <= f.y0)
                return;   // nothing left to scroll under it, or the bar is not over the area's visible bottom
            const float width = edge - (bar.min.y - Sc(16));
            f.y1 = Lerp(f.y1, edge, s);
            f.bottom = Lerp(f.bottom, width, s);
        }

        namespace
        {
            struct ScrollState
            {
                bool dragging = false;       // drag-to-scroll
                float startMouse = 0.0f, startScroll = 0.0f, lastMouse = 0.0f, velocity = 0.0f;
                bool follow = false;         // this frame's offset is followY (a drag of the content or the indicator)
                float followY = 0.0f;
                bool indicatorDrag = false;
                float indicatorGrab = 0.0f;
                float lastScroll = 0.0f;
                double lastActivity = -10.0;
                bool scrolled = false;       // it had something to scroll last frame
            };

            float IndicatorLane() { return Sc(T().metrics.padding * 0.75f); }   // in a window's right padding
        }

        void ScrollBegin(Id child)
        {
            ScrollState& s = Ctx().State<ScrollState>(Salt(child, 0x5C));
            if (s.follow)
                Ctx().SetNextScroll(Vec2(-1.0f, s.followY));   // no glide: the content stays under the pointer
        }

        float WindowLane(const Rect& view, float* cornerRadius)
        {
            const Ui::Impl& m = M();
            if (cornerRadius)
                *cornerRadius = 0.0f;
            for (auto it = m.containers.rbegin(); it != m.containers.rend(); ++it)
                if (it->kind == Ui::Impl::ContainerEntry::Kind::Window)
                {
                    if (!(std::fabs(view.max.x - (it->view.max.x - it->padX)) < 1.0f && it->padX >= IndicatorLane() - 0.5f))
                        return -1.0f;
                    if (cornerRadius)
                        *cornerRadius = it->radius;
                    return it->view.max.x - it->padX * 0.5f;
                }
            return -1.0f;
        }

        float ResizeGripSize() { return Sc(26); }

        void ScrollEnd(Id child, float laneX, float cornerRadius)
        {
            Ui::Impl& m = M();
            Context& c = *m.ctx;
            const InputState& in = c.Input();
            ScrollState& s = c.State<ScrollState>(Salt(child, 0x5C));
            const Rect view = c.ViewRect();
            const float scroll = c.Scroll().y, max = std::max(c.ScrollMax().y, 0.0f);
            if (std::fabs(scroll - s.lastScroll) > 0.5f)
                s.lastActivity = m.time;   // the wheel, a glide: the indicator shows
            s.lastScroll = scroll;
            s.follow = false;
            const bool scrolls = max > 0.5f;
            if (scrolls && !s.scrolled)
                s.lastActivity = m.time;   // it appears with more than it shows (or grows past it): it flashes, as on iOS
            s.scrolled = scrolls;

            // the indicator (auto-hiding, wider under the mouse, draggable) in its lane, over the content without one;
            // submitted before a drag of the content is decided, so a press on it is its own
            const Theme& t = T();
            const float lane = IndicatorLane();
            const float x = laneX >= 0.0f ? laneX : view.max.x - lane * 0.5f;
            const float trackPad = Sc(6);
            // In its window's lane the track ends above the window's rounded bottom corner: the indicator stays inside
            // the window's shape (it ran down into the corner's curve) and off the grip that resizes the window.
            float trackEnd = view.max.y - trackPad;
            if (cornerRadius > 0.0f)
                trackEnd = std::min(trackEnd, c.CurrentWindow()->GetRect().max.y - std::max(cornerRadius, trackPad));
            const float trackH = std::max(trackEnd - (view.min.y + trackPad), 1.0f);
            const float thumbH = std::min(std::max(Sc(36), trackH * view.Height() / (view.Height() + max)), trackH);
            const float thumbY = view.min.y + trackPad + (trackH - thumbH) * (max > 0.0f ? Saturate(scroll / max) : 0.0f);
            const Rect hit(x - lane * 0.5f, view.min.y, x + lane * 0.5f, view.max.y);
            InteractState it;
            // The lane may lie in a parent's padding (outside its clip), but not past what shows of the area: one
            // its window scrolls half out of view shows its indicator, and is hit there, only in the window.
            const Rect shown = c.VisibleViewRect();
            c.PushClipRect(Rect(hit.min.x, std::max(hit.min.y, shown.min.y), hit.max.x, std::min(hit.max.y, shown.max.y)).Intersect(c.CurrentWindow()->GetRect()), false);
            if (scrolls)
                it = InteractImpl(Salt(child, 0x1D1), Rect(hit.min.x, thumbY, hit.max.x, thumbY + thumbH), InteractFlags_PressOnClick);
            if (it.pressed)
            {
                s.indicatorDrag = true;
                s.indicatorGrab = in.MousePos().y - thumbY;
            }
            if (s.indicatorDrag)
            {
                if (it.held)
                {
                    s.follow = true;
                    s.followY = max * Saturate((in.MousePos().y - s.indicatorGrab - view.min.y - trackPad) / std::max(trackH - thumbH, 1.0f));
                    s.lastActivity = m.time;
                }
                else
                    s.indicatorDrag = false;
            }

            // drag-to-scroll: pressed on the area's empty space, the content follows the finger (a mouse too with
            // InputConfig::mouseDragScrolls); let go, it glides on
            const Id dragId = Salt(child, 0xD2A6);
            if (!s.dragging && scrolls && in.MouseClicked(MouseButton::Left) && (in.MouseTouch(MouseButton::Left) || in.config.mouseDragScrolls) &&
                c.HoveredChild() == child && c.HoveredId() == 0 && c.ActiveId() == 0)
            {
                s.dragging = true;
                s.startMouse = s.lastMouse = in.MousePos().y;
                s.startScroll = scroll;
                s.velocity = 0.0f;
                c.SetActiveId(dragId);
            }
            // a finger that pressed an item in the area (a row, a button) and then moves along it scrolls instead, past
            // a slop and more along than across: the item lets go without pressing (Context::ActiveIdYieldsToScroll).
            // An inner area that scrolls has taken the touch already (it ends first).
            const Vec2 pressPos = in.MouseClickedPos(MouseButton::Left);
            if (!s.dragging && scrolls && c.ActiveIdYieldsToScroll() && in.MouseDown(MouseButton::Left) && view.Contains(pressPos))
            {
                const Vec2 d = in.MousePos() - pressPos;
                if (std::fabs(d.y) > in.config.touchSlop && std::fabs(d.y) > std::fabs(d.x))
                {
                    s.dragging = true;
                    s.startMouse = s.lastMouse = in.MousePos().y;
                    s.startScroll = scroll;
                    s.velocity = 0.0f;
                    c.SetActiveId(dragId);
                }
            }
            if (s.dragging)
            {
                if (c.ActiveId() == dragId && in.MouseDown(MouseButton::Left))
                {
                    c.KeepAliveId(dragId);
                    const float dy = in.MousePos().y - s.lastMouse;
                    s.lastMouse = in.MousePos().y;
                    s.velocity = Lerp(s.velocity, -dy / std::max(m.dt, 1e-3f), 0.35f);
                    s.follow = true;
                    s.followY = Clamp(s.startScroll - (in.MousePos().y - s.startMouse), 0.0f, max);
                    s.lastActivity = m.time;
                }
                else
                {
                    s.dragging = false;
                    if (c.ActiveId() == dragId)
                        c.ClearActiveId();
                    c.SetScrollY(Clamp(scroll + s.velocity * 0.28f, 0.0f, max));   // momentum: the smooth scroll glides there
                }
            }

            if (scrolls)
            {
                if (in.MouseValid() && hit.Contains(in.MousePos()) && c.HoveredWindow() == c.CurrentWindow())
                    s.lastActivity = std::max(s.lastActivity, m.time - 0.5);
                const bool active = (m.time - s.lastActivity) < 0.9 || it.held;
                if (active)
                    m.animating = true;   // a frame when it is time to hide
                // at rest it hides (iOS), or stays, dimmed on a faint track, as the theme asks (desktop apps) - while a
                // mouse is in use: after a finger's press it hides at rest, as on a phone
                const float always = in.MouseTouch(MouseButton::Left) ? 0.0f : Saturate(t.metrics.scrollIndicatorAlways);
                const float show = ui::Anim(Salt(child, 0x1D2), active ? 1.0f : 0.0f, active ? SpringFast() : t.motion.gentle);
                const float wide = ui::Anim(Salt(child, 0x1D3), (it.hovered || it.held) ? 1.0f : 0.0f, SpringFast());
                const float alpha = std::max(show, 0.55f * always);
                if (alpha > 0.01f)
                {
                    const float w = Sc(t.metrics.scrollIndicator) + Sc(3) * wide;
                    Painter p = GetPainter();
                    if (always > 0.0f)
                        p.Capsule(Rect(x - w * 0.5f, view.min.y + trackPad, x + w * 0.5f, view.min.y + trackPad + trackH), Style().Fill(C().label.Fade(0.07f * always)));
                    p.Capsule(Rect(x - w * 0.5f, thumbY, x + w * 0.5f, thumbY + thumbH), Style().Fill(C().label.Fade((0.28f + 0.2f * wide) * alpha)));
                }
            }
            c.PopClipRect();
        }
    }

    // ================================================================ cards
    bool BeginCard(std::string_view id, Vec2 size, const CardOptions& o)
    {
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const bool stylePushed = TakeNextStyle();   // the card and everything in it, until EndCard
        const float pad = o.padding >= 0.0f ? Sc(o.padding) : Sc(T().metrics.padding);
        ContainerOptions co;
        co.padding = Vec2(pad, pad);
        if (size.x > 0.0f)
            co.size.x = Sc(size.x);
        else
        {
            co.fillWidth = true;
            co.size.x = size.x < 0.0f ? Sc(size.x) : 0.0f;
            MarkFill();
        }
        if (size.y > 0.0f)
            co.size.y = Sc(size.y);
        Entry e;
        e.kind = Entry::Kind::Card;
        e.id = c.GetId(id);
        e.stylePushed = stylePushed;
        e.card = o;
        // the background is drawn at EndCard, when the height is known, and moved under the content
        e.mark = c.WindowDrawList().Mark();
        c.BeginContainer(e.id, co);
        m.containers.push_back(e);
        return true;
    }

    void EndCard()
    {
        const Entry e = PopEntry(Entry::Kind::Card);
        Context& c = Ctx();
        const Theme& t = T();
        const Rect card = c.EndContainer();
        const CardOptions& o = e.card;

        DrawList& dl = c.WindowDrawList();
        const std::size_t from = dl.Mark();
        {
            Painter p = GetPainter();
            Style s;
            s.Radius(o.radius >= 0.0f ? Sc(o.radius) : ItemRadius(Sc(t.metrics.cardRadius)));
            if (GlassSurface() && !o.glass)
                SurfaceFill(s, o.fill.a > 0 ? o.fill : FillOr(t.colors.cardSurface));   // the style turns it into glass
            else if (LookClear())
                s.Glass(StyledMaterial(t.materials.control));   // transparent: clear glass, no fill
            else
            {
                if (o.glass)
                    s.Glass(SurfaceMaterial(t.materials.control, o.fill.a > 0 ? Color::Clear() : t.colors.windowSurface.Fade(0.5f)));
                if (o.fill.a > 0 || !o.glass || ResolvedStyle().Has(ItemStyle::kFill))
                    s.Fill(o.fill.a > 0 ? o.fill : FillOr(t.colors.cardSurface));
            }
            if (o.shadow)
                s.Shadow(t.colors.shadow.Fade(0.6f), Sc(18), Vec2(0, Sc(6)));
            ScopedUnclip unclip(card, ShadowExtent(Sc(18), Vec2(0, Sc(6))));
            p.Rect(card, s);
        }
        MoveCommands(dl, from, e.mark);
        if (e.stylePushed)
            PopStyle();
    }

    // ============================================================ scrolling
    bool BeginScrollArea(std::string_view id, Vec2 size)
    {
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        ChildOptions co;
        co.size = Vec2(Sc(size.x), Sc(size.y));
        co.flags = ChildFlags_ScrollY | ChildFlags_SmoothScroll;
        if (size.x <= 0.0f)
            MarkFill();
        Entry e;
        e.kind = Entry::Kind::Scroll;
        e.child = c.GetId(id);
        ScrollBegin(e.child);
        c.BeginChild(id, co);
        e.edgeFade = BeginScrollEdgeFade();
        m.containers.push_back(e);
        return true;
    }

    void EndScrollArea()
    {
        const Entry e = PopEntry(Entry::Kind::Scroll);
        EndScrollEdgeFade(e.edgeFade);
        float radius = 0.0f;
        const float lane = WindowLane(Ctx().ViewRect(), &radius);
        ScrollEnd(e.child, lane, radius);
        Ctx().EndChild();
    }

    // ============================================================== windows
    bool BeginWindow(std::string_view title, bool* open, const WindowOptions& o)
    {
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const Theme& t = T();
        const Palette& pc = t.colors;
        const Id id = c.GetId(title);

        struct WindowState
        {
            bool closing = false;   // the close button was pressed: fade out, then *open = false
            bool seen = false;
        };
        WindowState& st = c.State<WindowState>(id);
        const bool stylePushed = TakeNextStyle();   // the window and everything in it, until EndWindow

        // docked (dock.cpp): it fills its node when its tab is the active one, and is not drawn otherwise
        DockPlacement dock;
        const bool docked = DockedPlacement(title, open && !(o.flags & WindowFlags_NoClose), o.icon, dock);
        if (docked)
        {
            if (dock.closeRequested && open)
                *open = false;
            if (!dock.shown || (open && !*open))
            {
                if (stylePushed)
                    PopStyle();
                return false;
            }
            st.closing = false;
            st.seen = true;
            AnimSet(Salt(id, 0x77), 1.0f);
        }

        const bool wantOpen = (open ? *open : true) && !st.closing;
        const Id visId = Salt(id, 0x77);
        const bool firstShow = !st.seen && wantOpen;
        if (firstShow)
        {
            AnimSet(visId, 0.0f);
            st.seen = true;
        }
        const float vis = ui::Anim(visId, wantOpen ? 1.0f : 0.0f, wantOpen ? t.motion.standard : t.motion.fast);
        if (!wantOpen && vis < 0.02f)
        {
            if (st.closing && open)
                *open = false;
            st.closing = false;
            st.seen = false;
            if (stylePushed)
                PopStyle();
            return false;
        }

        if (docked)
        {
            c.SetNextWindowPos(dock.rect.min, Cond::Always);
            c.SetNextWindowSize(dock.rect.Size(), Cond::Always);
        }
        else if (dock.pendingMove)
        {
            // just pulled out of a dock node by its tab: its own size, under the mouse
            c.SetNextWindowSize(o.size * t.metrics.scale, Cond::Always);
            c.SetNextWindowPos(c.Input().MousePos() - dock.grab, Cond::Always);
        }
        else
            c.SetNextWindowSize(o.size * t.metrics.scale, Cond::FirstUse);
        if (docked || dock.pendingMove)
        {
        }
        else if (o.pos.x >= 0.0f)
            c.SetNextWindowPos(o.pos * t.metrics.scale, Cond::FirstUse);
        else
        {
            // centered, each new window a step further down and right
            if (firstShow && !c.FindWindowByName(title))
                ++m.windowCascade;
            const float k = (float)(m.windowCascade % 6) * Sc(28);
            c.SetNextWindowPos(c.DisplaySize() * 0.5f + Vec2(k, k), Cond::FirstUse, Vec2(0.5f, 0.5f));
        }
        esia::WindowOptions wo;
        if (o.flags & WindowFlags_NoResize)
            wo.flags |= esia::WindowFlags_NoResize;
        if (o.flags & WindowFlags_NoMove)
            wo.flags |= esia::WindowFlags_NoMove;
        if (!wantOpen)
            wo.flags |= esia::WindowFlags_NoInputs;
        wo.padding = Vec2(0, 0);
        wo.minSize = Vec2(Sc(220), Sc(160));
        if (!(o.flags & WindowFlags_NoResize))
            wo.resizeGrip = ResizeGripSize();   // where the grip shows (EndWindow): its corner is well inside the edges
        if (docked)
        {
            // behind the floating windows, where its node puts it
            wo.flags |= esia::WindowFlags_NoMove | esia::WindowFlags_NoResize;
            wo.layer = WindowLayer::Background;
            wo.minSize = Vec2(1, 1);
        }
        if (!c.Begin(title, wo))
        {
            c.End();
            if (stylePushed)
                PopStyle();
            return false;
        }
        Window* w = c.CurrentWindow();
        if (dock.pendingMove)
            c.StartWindowMove(w, dock.grab);
        const Rect wr = w->GetRect();
        const float alpha = Saturate(vis);
        PushStyle(ItemStyle().Opacity(alpha));   // everything the window draws fades with it

        Entry e;
        e.kind = Entry::Kind::Window;
        e.id = id;
        e.stylePushed = stylePushed;
        e.windowFlags = o.flags | (docked ? (WindowFlags_NoResize | WindowFlags_NoShadow) : 0u);
        const std::uint32_t flags = e.windowFlags;
        const float radius = docked ? ItemRadius(Sc(18)) : ItemRadius(Sc(t.metrics.windowRadius));

        // ---------------------------------------------------------- surface
        const bool focused = c.FocusedWindow() == w;
        const float focusT = Anim(id, 0x78, focused ? 1.0f : 0.0f, t.motion.gentle);
        {
            Painter bg = GetPainter();
            Style ws;
            ws.Radius(radius);
            if (!(flags & WindowFlags_NoShadow))
                ws.Shadow(pc.shadow.Fade(0.9f + 0.5f * focusT), Sc(34 + 14 * focusT), Vec2(0, Sc(14 + 6 * focusT)));
            if (((o.flags & WindowFlags_Solid) && !LookClear()) || m.flat)   // flat: every window is solid
                ws.Fill(FillOr(t.dark ? Color::Hex(0x1C1C1E) : Color::Hex(0xF2F2F7))).Stroke(1.0f, pc.separator.Fade(0.6f));
            else
            {
                if (o.flags & WindowFlags_ClearGlass)
                    PushStyle(ItemStyle().Look(GlassLook::Clear));
                GlassMaterial gm = SurfaceMaterial(t.materials.window, pc.windowSurface);
                if (o.flags & WindowFlags_ClearGlass)
                    PopStyle();
                // "materialize": frost and lensing ramp in with the appear animation
                gm.blur *= 0.35f + 0.65f * alpha;
                gm.refraction *= alpha;
                ws.Glass(gm);
            }
            // the shadow lives outside the window rectangle: the window's clip must not cut it square
            ScopedUnclip unclip(wr, ShadowExtent(Sc(48), Vec2(0, Sc(20))));
            bg.Rect(wr, ws);
        }

        // ----------------------------------------------------------- header
        float headerH = 0.0f;
        if (docked)
            headerH = DockTabBar(title, Rect(wr.min.x, wr.min.y, wr.max.x, wr.min.y + Sc(Sizes().dockTabs)));   // its node's tabs
        else if (!(o.flags & WindowFlags_NoHeader))
        {
            headerH = Sc(t.metrics.headerHeight) + (!o.subtitle.empty() ? Sc(12) : 0.0f);
            const float pad = Sc(t.metrics.padding + 4);
            const bool closable = open && !(o.flags & WindowFlags_NoClose);
            const float cy = wr.min.y + Sc(t.metrics.headerHeight) * 0.5f;
            Painter p = GetPainter();

            // close button: a glass circle with an x (the rest of the header is empty window area: dragging it moves
            // the window)
            Rect closeR;
            if (closable)
            {
                const float d = Sc(Sizes().closeButton);
                closeR = Rect::FromCenter(Vec2(wr.max.x - pad - d * 0.5f + Sc(4), cy), Vec2(d, d));
                const InteractState ci = InteractImpl(Salt(id, 0xC105E), closeR, InteractFlags_None);
                p.PushScale(closeR.Center(), 1.0f + 0.08f * ci.press);
                p.Circle(closeR.Center(), d * 0.5f, Surface(pc.fill.Fade(0.8f + 0.6f * ci.hover)));
                DrawIcon(p, closeR.Center(), icons::Close, Sc(11), pc.secondaryLabel.Fade(1.0f + 0.4f * ci.hover));
                p.PopScale();
                if (ci.pressed)
                    st.closing = true;
            }

            float tx = wr.min.x + pad;
            if (o.icon)
            {
                const float tile = Sc(Sizes().windowIcon);
                IconTile(p, Rect::FromCenter(Vec2(tx + tile * 0.5f, cy), Vec2(tile, tile)), o.icon, Accent());
                tx += Sc(Sizes().windowIcon + 10.0f);
            }
            const std::string_view shown = VisibleLabel(title);
            const text::FontRef tf = Font(TextStyle::Title3);
            const Vec2 ts = MeasureText(tf, shown);
            const float ty = !o.subtitle.empty() ? cy - ts.y * 0.5f - Sc(4) : cy - ts.y * 0.5f;
            // title and subtitle end before the close button (ellipsized), never under it
            const float tr = closable ? closeR.min.x - Sc(8) : wr.max.x - pad;
            if (tr > tx)
            {
                p.TextBox(Rect(std::floor(tx), std::floor(ty), tr, std::floor(ty) + ts.y), Vec2(0, 0), tf, LabelOr(pc.label), shown, text::TextFlags_Ellipsis);
                if (!o.subtitle.empty())
                {
                    const text::FontRef sf = Font(TextStyle::Footnote);
                    const float sy = std::floor(ty + ts.y + Sc(1));
                    p.TextBox(Rect(std::floor(tx), sy, tr, sy + sf.size * 1.4f), Vec2(0, 0), sf, pc.secondaryLabel, o.subtitle, text::TextFlags_Ellipsis);
                }
            }
        }

        // ---------------------------------------------------------- content
        const float padX = (o.flags & WindowFlags_NoPadding) ? 0.0f : Sc(t.metrics.padding);
        const float padY = (o.flags & WindowFlags_NoPadding) ? 0.0f : (headerH > 0.0f ? Sc(4) : Sc(t.metrics.padding));
        // The content is clipped by a rectangle, the window is rounded: keep it out of the corners, so nothing scrolled
        // to an edge pokes out past them. At the clip's side (half the padding in) the corner curve reaches `cut`
        // into the window; the bottom (and a headerless top) move in by that much.
        const float side = std::floor(padX * 0.5f);
        const float cut = side < radius ? radius - std::sqrt(std::max(radius * radius - (radius - side) * (radius - side), 0.0f)) + 1.0f : 0.0f;
        e.cornerCut = cut;
        e.radius = radius;
        c.SetCursorPos(Vec2(wr.min.x, wr.min.y + headerH));
        ChildOptions co;
        co.size = Vec2(wr.Width(), std::max(1.0f, wr.Height() - headerH));
        co.padding = Vec2(padX, padY);
        if (!(o.flags & WindowFlags_NoScroll))
            co.flags = ChildFlags_ScrollY | ChildFlags_SmoothScroll;
        // the clip is measured from the content rect (inside the padding): out to half the padding at the sides, to
        // the header (or the corner cut) at the top and to the corner cut at the bottom
        co.clipInset = Rect(-(padX - side), headerH > cut ? -padY : cut - padY, -(padX - side), cut - padY);
        e.child = c.GetId("##content");
        if (!(o.flags & WindowFlags_NoScroll))
            ScrollBegin(e.child);
        c.BeginChild("##content", co);
        e.view = c.ViewRect();
        e.padX = padX;
        e.edgeFade = BeginScrollEdgeFade(!(o.flags & WindowFlags_NoScroll));
        m.containers.push_back(e);

        if (o.flags & WindowFlags_LargeTitle)
        {
            LargeTitle(VisibleLabel(title));
            Spacer(6);
        }
        return true;
    }

    void EndWindow()
    {
        const Entry e = PopEntry(Entry::Kind::Window);
        Context& c = Ctx();
        // breathing room at the bottom, so the last row never sticks to the rounded edge (and clears the corner clip)
        c.ItemSize(Vec2(0, std::max(Sc(T().metrics.padding), e.cornerCut)));
        EndScrollEdgeFade(e.edgeFade);
        if (!(e.windowFlags & WindowFlags_NoScroll))
            ScrollEnd(e.child, e.padX >= Sc(12) - 0.5f ? e.view.max.x - e.padX * 0.5f : -1.0f, e.padX >= Sc(12) - 0.5f ? e.radius : 0.0f);
        c.EndChild();

        // resize affordance at the bottom-right corner, shown while the mouse is near it
        if (!(e.windowFlags & WindowFlags_NoResize))
        {
            const Rect wr = c.CurrentWindow()->GetRect();
            const float zone = ResizeGripSize();
            const InputState& in = c.Input();
            const bool nearCorner = c.HoveredWindow() == c.CurrentWindow() && in.MouseValid() &&
                                    Rect(wr.max - Vec2(zone, zone), wr.max + Vec2(4, 4)).Contains(in.MousePos());
            const float show = Anim(e.id, 0x2E5, nearCorner ? 1.0f : 0.0f, SpringFast());
            if (show > 0.01f)
            {
                const float r = Sc(T().metrics.windowRadius);
                Painter p = GetPainter();
                p.Arc(wr.max - Vec2(r, r), r - Sc(5), Sc(4), kPi * 0.12f, kPi * 0.26f, Style().Fill(C().label.Fade(0.35f * show)));
            }
        }
        PopStyle();   // the window's opacity
        c.End();
        if (e.stylePushed)
            PopStyle();
    }
}
