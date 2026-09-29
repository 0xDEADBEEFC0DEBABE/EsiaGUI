// WGT UI - liquid-glass windows, smooth scroll areas and cards.
#include "ui/ui_internal.hpp"

namespace wgt::ui
{
    using namespace detail;

    namespace
    {
        struct WindowState
        {
            bool closing = false;
            bool seen = false;
        };

        struct ScrollState
        {
            ImGuiWindow* window = nullptr;
            float target = 0.0f;
            SpringState display;
            bool dragging = false;
            float dragStartMouse = 0.0f;
            float dragStartScroll = 0.0f;
            float dragVelocity = 0.0f;
            float lastMouse = 0.0f;
            double lastActivity = -10.0;
            bool indicatorDrag = false;
            float indicatorGrab = 0.0f;
            float gutter = 0.0f;       // content inset reserved for the indicator when no padding lane exists
            float lastX = 0.0f;        // area x last frame (a moving area, e.g. a sliding page, keeps its decision)
        };

        // Width of the lane the scroll indicator lives in. Nothing else may be drawn there.
        float IndicatorLane() { return Sc(12); }

        struct CardStyleStore
        {
            CardOptions options;
            float savedWorkMaxX = 0.0f;
            float savedContentMaxX = 0.0f;
        };
    }

    // ============================================================ scrolling
    void detail::ScrollAreaBegin(ImGuiID id, const char* name, Vec2 size, Vec2 padding, bool allowScroll, float topCut, float bottomCut)
    {
        Context::Impl& impl = Ctx();
        ScrollState& s = impl.state.Get<ScrollState>(id);
        const float dt = impl.dt;
        if (s.window && allowScroll)
        {
            ImGuiWindow* w = s.window;
            // Absorb scroll requests ImGui queued since last frame (mouse wheel, keyboard nav, SetScrollY...)
            if (w->ScrollTarget.y < FLT_MAX)
            {
                float requested = w->ScrollTarget.y - w->ScrollTargetCenterRatio.y * (w->InnerRect.GetHeight());
                if (w->ScrollTargetCenterRatio.y == 0.0f)
                    s.target = s.target + (requested - w->Scroll.y);
                else
                    s.target = requested;
                w->ScrollTarget.y = FLT_MAX;
                s.lastActivity = impl.time;
            }
            s.target = Clamp(s.target, 0.0f, std::max(w->ScrollMax.y, 0.0f));
            if (s.dragging)
                s.display.value = s.target, s.display.velocity = 0.0f;
            else
                s.display.Step(s.target, Spring{0.30f, 1.0f}, dt);
            if (std::fabs(s.display.value - s.target) > 0.5f)
                s.lastActivity = impl.time;
            // whole physical pixels: text (pixel-snapped glyphs) and SDF surfaces move together, no judder
            ImGui::SetNextWindowScroll(Vec2(-1.0f, std::max(0.0f, Painter::SnapToPixel(s.display.value))));
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
        ImGuiWindowFlags wf = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
        if (!allowScroll)
            wf |= ImGuiWindowFlags_NoScrollWithMouse;
        ImGui::BeginChild(name, size, ImGuiChildFlags_AlwaysUseWindowPadding, wf);
        ImGui::PopStyleVar();
        s.window = ImGui::GetCurrentWindow();
        if (allowScroll && s.gutter > 0.0f)
        {
            // no padding can host the scroll indicator: keep a gutter on the right free of content
            s.window->ContentRegionRect.Max.x -= s.gutter;
            s.window->WorkRect.Max.x -= s.gutter;
        }
        ImGuiWindow* cw = s.window;
        std::uint8_t edges = 0;
        if (topCut > 0.0f || bottomCut > 0.0f)
        {
            ImGui::PushClipRect(Vec2(cw->ClipRect.Min.x, cw->InnerRect.Min.y + topCut), Vec2(cw->ClipRect.Max.x, cw->InnerRect.Max.y - bottomCut), true);
            edges |= 1;
        }
        // Content dissolves towards an edge it can still scroll past (iOS scroll edge effect), with no color
        // added. The fade widths grow with the distance left to scroll: nothing fades at the very top / end.
        if (allowScroll)
        {
            const float top = std::min(std::max(cw->Scroll.y, 0.0f), Sc(22));
            const float bottom = std::min(std::max(cw->ScrollMax.y - cw->Scroll.y, 0.0f), Sc(30));
            if (top > 0.5f || bottom > 0.5f)
            {
                Painter(cw->DrawList).BeginEdgeFade(Rect(cw->ClipRect.Min, cw->ClipRect.Max), top, bottom);
                edges |= 2;
            }
        }
        impl.ui.scrollEdges.push_back(edges);
        impl.ui.scrolls.push_back(allowScroll ? id : 0);
    }

    void detail::ScrollAreaEnd()
    {
        Context::Impl& impl = Ctx();
        IM_ASSERT(!impl.ui.scrolls.empty());
        const ImGuiID id = impl.ui.scrolls.back();
        impl.ui.scrolls.pop_back();
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        ImGuiContext& g = *ImGui::GetCurrentContext();
        ImGuiIO& io = ImGui::GetIO();
        // content ends here: close its edge fade and corner clip before the indicator is drawn
        const std::uint8_t edges = impl.ui.scrollEdges.empty() ? 0 : impl.ui.scrollEdges.back();
        if (!impl.ui.scrollEdges.empty())
            impl.ui.scrollEdges.pop_back();
        if (edges & 2)
            Painter(w->DrawList).EndEdgeFade();
        if (edges & 1)
            ImGui::PopClipRect();

        if (id != 0)
        {
            ScrollState& s = impl.state.Get<ScrollState>(id);
            // Drag-to-scroll on empty space, with momentum.
            const ImGuiID dragId = Salt(id, 0xD2A6);
            if (!s.dragging && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && g.ActiveId == 0 && g.HoveredId == 0 && w->ScrollMax.y > 0.0f)
            {
                s.dragging = true;
                s.dragStartMouse = io.MousePos.y;
                s.dragStartScroll = s.display.value;
                s.lastMouse = io.MousePos.y;
                s.dragVelocity = 0.0f;
                ImGui::SetActiveID(dragId, w);
                ImGui::FocusWindow(w);
            }
            if (s.dragging)
            {
                if (g.ActiveId == dragId && io.MouseDown[0])
                {
                    ImGui::KeepAliveID(dragId);
                    const float dy = io.MousePos.y - s.lastMouse;
                    s.lastMouse = io.MousePos.y;
                    s.dragVelocity = Lerp(s.dragVelocity, -dy / std::max(impl.dt, 1e-3f), 0.35f);
                    // rubber band beyond the edges
                    float t = s.dragStartScroll - (io.MousePos.y - s.dragStartMouse);
                    const float mx = std::max(w->ScrollMax.y, 0.0f);
                    if (t < 0.0f) t = -std::sqrt(-t) * 2.0f;
                    if (t > mx) t = mx + std::sqrt(t - mx) * 2.0f;
                    s.target = Clamp(t, 0.0f, mx);
                    s.lastActivity = impl.time;
                }
                else
                {
                    s.dragging = false;
                    if (g.ActiveId == dragId)
                        ImGui::ClearActiveID();
                    s.target = Clamp(s.target + s.dragVelocity * 0.28f, 0.0f, std::max(w->ScrollMax.y, 0.0f));
                }
            }
        }

        const Rect area(w->Pos, w->Pos + w->Size);
        const float scrollMax = w->ScrollMax.y;
        const float scrollY = w->Scroll.y;

        // The indicator's lane: the right padding of this area, or of the parents it is flush with (a page in
        // a navigation stack uses its window's padding). Without one, a gutter is reserved in the area itself.
        float laneX = -1.0f;
        if (id != 0)
        {
            ScrollState& s = impl.state.Get<ScrollState>(id);
            const float lane = IndicatorLane();
            bool viaParent = false;
            for (ImGuiWindow* cw = w; cw;)
            {
                const float right = cw->Pos.x + cw->Size.x;
                const float pad = right - cw->ContentRegionRect.Max.x;
                if (pad >= lane - 0.5f)
                {
                    laneX = cw->ContentRegionRect.Max.x + pad * 0.5f;
                    break;
                }
                ImGuiWindow* parent = cw->ParentWindow;
                if (!(cw->Flags & ImGuiWindowFlags_ChildWindow) || !parent || right < parent->ContentRegionRect.Max.x - 1.0f)
                    break;
                cw = parent;
                viaParent = true;
            }
            const bool moving = std::fabs(w->Pos.x - s.lastX) > 0.5f;
            s.lastX = w->Pos.x;
            if (!moving)
            {
                if (laneX >= 0.0f && viaParent)
                    s.gutter = 0.0f;
                else if (laneX < 0.0f)
                    s.gutter = scrollMax > 0.5f ? lane : 0.0f;   // reserved from the next frame on
            }
        }
        ImGui::EndChild();

        if (id == 0 || scrollMax <= 0.5f || laneX < 0.0f)
            return;

        // Scroll indicator (auto-hiding, draggable), drawn in its lane - never over content.
        ScrollState& s = impl.state.Get<ScrollState>(id);
        const Theme& t = T();
        const float trackPad = Sc(6);
        const float trackH = area.Height() - trackPad * 2.0f;
        const float visible = area.Height();
        const float thumbH = std::max(Sc(36), trackH * visible / (visible + scrollMax));
        const float thumbY = area.min.y + trackPad + (trackH - thumbH) * Saturate(scrollY / scrollMax);
        const float laneHalf = IndicatorLane() * 0.5f;
        const Rect hit(laneX - laneHalf, area.min.y, laneX + laneHalf, area.max.y);
        const ImGuiID indId = Salt(id, 0x1D1);
        ImGui::PushClipRect(hit.min, hit.max, false);   // the lane may lie in a parent's padding
        ImGui::PushItemFlag(ImGuiItemFlags_NoWindowHoverableCheck, true);
        Interaction it = InteractImpl(indId, Rect(hit.min.x, thumbY, hit.max.x, thumbY + thumbH), InteractFlags_PressOnClick | InteractFlags_AllowOverlap);
        ImGui::PopItemFlag();
        if (it.pressed)
        {
            s.indicatorDrag = true;
            s.indicatorGrab = io.MousePos.y - thumbY;
        }
        if (s.indicatorDrag)
        {
            if (it.held)
            {
                const float f = Saturate((io.MousePos.y - s.indicatorGrab - area.min.y - trackPad) / std::max(trackH - thumbH, 1.0f));
                s.target = f * scrollMax;
                s.lastActivity = impl.time;
            }
            else
                s.indicatorDrag = false;
        }
        const bool nearEdge = ImGui::IsMouseHoveringRect(hit.min, hit.max, false) && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
        if (nearEdge)
            s.lastActivity = std::max(s.lastActivity, impl.time - 0.5);
        const bool active = (impl.time - s.lastActivity) < 0.9 || it.held;
        const float show = Anim(id, 0x1D2, active ? 1.0f : 0.0f, active ? SpringFast() : t.motion.gentle);
        const float wide = Anim(id, 0x1D3, (it.hovered || it.held) ? 1.0f : 0.0f, SpringFast());
        if (show > 0.01f)
        {
            const float width = Sc(t.metrics.scrollIndicator) + Sc(3) * wide;
            Painter p;
            const Rect thumb(laneX - width * 0.5f, thumbY, laneX + width * 0.5f, thumbY + thumbH);
            p.Capsule(thumb, Style().Fill(C().label.Fade((0.28f + 0.2f * wide) * show)));
        }
        ImGui::PopClipRect();
    }

    bool BeginScrollArea(const char* id, Vec2 size)
    {
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return false;
        const ImGuiID sid = w->GetID(id);
        ScrollAreaBegin(sid, id, Vec2(Sc(size.x), Sc(size.y)), Vec2(0, 0), true);
        return true;
    }

    void EndScrollArea() { ScrollAreaEnd(); }

    // ============================================================== windows
    bool BeginWindow(const char* title, bool* open, const WindowOptions& o)
    {
        Context::Impl& impl = Ctx();
        const Theme& t = T();
        const Palette& c = t.colors;
        const ImGuiID id = ImGui::GetID(title);
        WindowState& st = impl.state.Get<WindowState>(id);
        const bool look = TakeNextStyle();   // the window and everything in it, until EndWindow

        const bool wantOpen = (open ? *open : true) && !st.closing;
        const ImGuiID visId = Salt(id, 0x77);
        const bool firstShow = !st.seen && wantOpen;
        if (firstShow)
        {
            anim::Set(visId, 0.0f);
            st.seen = true;
        }
        const float vis = anim::Float(visId, wantOpen ? 1.0f : 0.0f, wantOpen ? &t.motion.standard : &t.motion.fast);
        if (!wantOpen && vis < 0.02f)
        {
            if (st.closing && open)
                *open = false;
            st.closing = false;
            st.seen = false;
            if (look)
                PopGlassLook();
            return false;
        }

        const float scale = t.metrics.scale;
        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowSize(Vec2(o.size.x * scale, o.size.y * scale), ImGuiCond_FirstUseEver);
        if (o.pos.x >= 0.0f)
            ImGui::SetNextWindowPos(Vec2(o.pos.x * scale, o.pos.y * scale), ImGuiCond_FirstUseEver);
        else
        {
            if (firstShow && !ImGui::FindWindowByName(title))
                ++impl.ui.windowCascade;
            const float k = (float)(impl.ui.windowCascade % 6) * Sc(28);
            ImGui::SetNextWindowPos(Vec2(vp->WorkPos.x + vp->WorkSize.x * 0.5f + k, vp->WorkPos.y + vp->WorkSize.y * 0.5f + k), ImGuiCond_FirstUseEver, Vec2(0.5f, 0.5f));
        }

        ImGuiWindowFlags wf = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar |
                              ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;
        if (o.flags & WindowFlags_NoResize)
            wf |= ImGuiWindowFlags_NoResize;
        if (o.flags & WindowFlags_NoMove)
            wf |= ImGuiWindowFlags_NoMove;
        if (!wantOpen)
            wf |= ImGuiWindowFlags_NoInputs;

        const float radius = ItemRadius(Sc(t.metrics.windowRadius));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, radius);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, Vec2(Sc(220), Sc(160)));
        const bool visible = ImGui::Begin(title, nullptr, wf);
        ImGui::PopStyleVar(4);
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (!visible)
        {
            ImGui::End();
            if (look)
                PopGlassLook();
            return false;
        }

        const float alpha = Saturate(vis);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * alpha);
        WindowFrame frame;
        frame.id = id;
        frame.flags = o.flags;
        frame.styleVars = 1;
        frame.lookPushed = look;

        // ---------------------------------------------------------- surface
        const Rect wr(window->Pos, window->Pos + window->Size);
        const bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        const float focusT = Anim(id, 0x78, focused ? 1.0f : 0.0f, t.motion.gentle);
        Painter bg(window->DrawList);
        Style ws;
        ws.Radius(radius);
        if (!(o.flags & WindowFlags_NoShadow))
            ws.Shadow(c.shadow.Fade(0.9f + 0.5f * focusT), Sc(34 + 14 * focusT), Vec2(0, Sc(14 + 6 * focusT)));
        if ((o.flags & WindowFlags_Solid) && !LookClear())
            ws.Fill(FillOr(t.dark ? Color::Hex(0x1C1C1E) : Color::Hex(0xF2F2F7))).Stroke(1.0f, c.separator.Fade(0.6f));
        else
        {
            if (o.flags & WindowFlags_ClearGlass)
                PushGlassLook(GlassLook::Clear);
            GlassMaterial m = SurfaceMaterial(t.materials.window, c.windowSurface);
            if (o.flags & WindowFlags_ClearGlass)
                PopGlassLook();
            // "materialize": frost and lensing ramp in with the appear animation
            m.blur *= 0.35f + 0.65f * alpha;
            m.refraction *= alpha;
            ws.Glass(m);
        }
        {
            // the shadow lives outside the window rectangle: never let the window clip cut it square
            ScopedUnclip unclip(window->DrawList, wr, ShadowExtent(Sc(48), Vec2(0, Sc(20))));
            bg.Rect(wr, ws);
        }

        // ----------------------------------------------------------- header
        float headerH = 0.0f;
        if (!(o.flags & WindowFlags_NoHeader))
        {
            headerH = Sc(t.metrics.headerHeight) + (o.subtitle ? Sc(12) : 0.0f);
            const Rect header(wr.min, Vec2(wr.max.x, wr.min.y + headerH));
            const float pad = Sc(t.metrics.padding + 4);
            const bool closable = open && !(o.flags & WindowFlags_NoClose);

            // close button (glass circle with xmark)
            Rect closeR;
            if (closable)
            {
                const float d = Sc(30);
                closeR = Rect::FromCenter(Vec2(header.max.x - pad - d * 0.5f + Sc(4), header.min.y + Sc(t.metrics.headerHeight) * 0.5f), Vec2(d, d));
                Interaction ci = InteractImpl(Salt(id, 0xC105E), closeR, InteractFlags_None);
                Painter p(window->DrawList);
                p.PushScale(closeR.Center(), 1.0f + 0.08f * ci.press);
                p.Circle(closeR.Center(), d * 0.5f, Surface(c.fill.Fade(0.8f + 0.6f * ci.hover)));
                p.Icon(closeR.Center(), icons::Close, Sc(11), c.secondaryLabel.Fade(1.0f + 0.4f * ci.hover));
                p.PopScale();
                if (ci.pressed)
                    st.closing = true;
            }

            // drag to move (anywhere on the header except the close button)
            if (!(o.flags & WindowFlags_NoMove))
            {
                Rect dragR = header;
                if (closable)
                    dragR.max.x = closeR.min.x - Sc(4);
                Interaction hi = InteractImpl(Salt(id, 0x4EAD), dragR, InteractFlags_PressOnClick | InteractFlags_AllowOverlap);
                if (hi.pressed)
                    ImGui::StartMouseMovingWindow(window);
            }

            Painter p(window->DrawList);
            float tx = wr.min.x + pad;
            const float cy = header.min.y + Sc(t.metrics.headerHeight) * 0.5f;
            if (o.icon)
            {
                const Rect tile = Rect::FromCenter(Vec2(tx + Sc(13), cy), Vec2(Sc(26), Sc(26)));
                IconTile(p, tile, o.icon, Accent());
                tx += Sc(36);
            }
            const char* titleEnd = ImGui::FindRenderedTextEnd(title);
            const FontRef tf = GetFont(TextStyle::Title3);
            const Vec2 ts = Painter::MeasureText(tf, title, titleEnd);
            const float ty = o.subtitle ? cy - ts.y * 0.5f - Sc(4) : cy - ts.y * 0.5f;
            // title / subtitle end before the close button (ellipsized), never under it
            const float tr = closable ? closeR.min.x - Sc(8) : wr.max.x - pad;
            if (tr > tx)
            {
                p.TextBox(Rect(std::floor(tx), std::floor(ty), tr, std::floor(ty) + ts.y), Vec2(0, 0), tf, c.label, title, titleEnd, TextFlags_Ellipsis);
                if (o.subtitle)
                {
                    const FontRef sf = GetFont(TextStyle::Footnote);
                    const float sy = std::floor(ty + ts.y + Sc(1));
                    p.TextBox(Rect(std::floor(tx), sy, tr, sy + sf.size * 1.4f), Vec2(0, 0), sf, c.secondaryLabel, o.subtitle, nullptr, TextFlags_Ellipsis);
                }
            }
        }

        // ---------------------------------------------------------- content
        ImGui::SetCursorScreenPos(Vec2(wr.min.x, wr.min.y + headerH));
        const Vec2 contentSize(wr.Width(), std::max(1.0f, wr.Height() - headerH));
        const float padX = (o.flags & WindowFlags_NoPadding) ? 0.0f : Sc(t.metrics.padding);
        const float padY = (o.flags & WindowFlags_NoPadding) ? 0.0f : (headerH > 0.0f ? Sc(4) : Sc(t.metrics.padding));
        // Content is clipped by a rectangle, the window is rounded: keep it out of the corner zones, so nothing
        // scrolled to an edge pokes out past the corners. At the clip's side inset (ImGui clips child content half
        // the padding in) the corner curve reaches `cut` into the window; the bottom (and a headerless top) move in.
        const float side = std::floor(padX * 0.5f);
        const float cut = side < radius ? radius - std::sqrt(std::max(radius * radius - (radius - side) * (radius - side), 0.0f)) + 1.0f : 0.0f;
        frame.cornerCut = cut;
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, radius);
        ScrollAreaBegin(Salt(id, 0xC0A7), "##content", contentSize, Vec2(padX, padY), !(o.flags & WindowFlags_NoScroll), headerH > cut ? 0.0f : cut, cut);
        ImGui::PopStyleVar();
        frame.hasContent = true;
        impl.ui.windows.push_back(frame);

        if (o.flags & WindowFlags_LargeTitle)
        {
            LargeTitle(title);
            Spacer(6);
        }
        return true;
    }

    void EndWindow()
    {
        Context::Impl& impl = Ctx();
        IM_ASSERT(!impl.ui.windows.empty() && "EndWindow() without BeginWindow()");
        const WindowFrame frame = impl.ui.windows.back();
        impl.ui.windows.pop_back();
        if (frame.hasContent)
        {
            // bottom breathing room so the last row never sticks to the rounded edge (and clears the corner clip)
            ImGui::Dummy(Vec2(0, std::max(Sc(T().metrics.padding), frame.cornerCut)));
            ScrollAreaEnd();
        }

        // resize affordance (bottom-right), only visible while hovering the corner
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (!(frame.flags & WindowFlags_NoResize))
        {
            const Rect wr(window->Pos, window->Pos + window->Size);
            const Vec2 corner = wr.max;
            const float zone = Sc(26);
            const bool nearCorner = ImGui::IsMouseHoveringRect(corner - Vec2(zone, zone), corner + Vec2(4, 4), false) &&
                              (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem));
            const float show = Anim(frame.id, 0x2E5, nearCorner ? 1.0f : 0.0f, SpringFast());
            if (show > 0.01f)
            {
                const float r = Sc(T().metrics.windowRadius);
                Painter p(window->DrawList);
                const Vec2 center = corner - Vec2(r, r);
                p.Arc(center, r - Sc(5), Sc(4), kPi * 0.12f, kPi * 0.26f, Style().Fill(C().label.Fade(0.35f * show)));
            }
        }
        ImGui::PopStyleVar(frame.styleVars);
        ImGui::End();
        if (frame.lookPushed)
            PopGlassLook();
    }

    // ================================================================ cards
    bool BeginCard(const char* id, Vec2 size, const CardOptions& o)
    {
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        const bool look = TakeNextStyle();   // the card and everything in it, until EndCard
        if (w->SkipItems)
        {
            if (look)
                PopGlassLook();
            return false;
        }
        Context::Impl& impl = Ctx();
        const Theme& t = T();
        const ImGuiID cid = w->GetID(id);
        const float pad = o.padding >= 0.0f ? Sc(o.padding) : Sc(t.metrics.padding);
        const float width = size.x > 0.0f ? Sc(size.x) : (size.x < 0.0f ? AvailWidth() + Sc(size.x) : AvailWidth());
        const float height = size.y > 0.0f ? Sc(size.y) : 0.0f;
        const Vec2 pos = w->DC.CursorPos;

        // draw the card background on a lower channel once the content height is known
        auto frame = std::make_unique<SectionFrame>();
        frame->id = cid;
        frame->drawList = w->DrawList;
        frame->x0 = pos.x;
        frame->x1 = pos.x + width;
        frame->y0 = pos.y;
        frame->rows = height > 0.0f ? (int)height : -1;   // fixed height marker
        frame->lookPushed = look;
        frame->splitter.Split(w->DrawList, 2);
        frame->splitter.SetCurrentChannel(w->DrawList, 1);
        impl.ui.sections.push_back(std::move(frame));
        impl.ui.cards.push_back(cid);

        CardStyleStore& store = impl.state.Get<CardStyleStore>(cid);
        store.options = o;
        store.savedWorkMaxX = w->WorkRect.Max.x;
        store.savedContentMaxX = w->ContentRegionRect.Max.x;

        ImGui::SetCursorScreenPos(Vec2(pos.x + pad, pos.y + pad));
        ImGui::BeginGroup();
        ImGui::PushTextWrapPos(pos.x + width - pad - w->Pos.x + w->Scroll.x);   // window-local wrap position
        ImGui::PushItemWidth(width - pad * 2.0f);
        // constrain "fill available width" widgets to the card (the content region is what they measure)
        w->WorkRect.Max.x = std::min(w->WorkRect.Max.x, pos.x + width - pad);
        w->ContentRegionRect.Max.x = std::min(w->ContentRegionRect.Max.x, pos.x + width - pad);
        return true;
    }

    void EndCard()
    {
        Context::Impl& impl = Ctx();
        IM_ASSERT(!impl.ui.cards.empty());
        const ImGuiID cid = impl.ui.cards.back();
        impl.ui.cards.pop_back();
        std::unique_ptr<SectionFrame> frame = std::move(impl.ui.sections.back());
        impl.ui.sections.pop_back();
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        const Theme& t = T();
        const CardStyleStore& store = impl.state.Get<CardStyleStore>(cid);
        const CardOptions o = store.options;
        const float pad = o.padding >= 0.0f ? Sc(o.padding) : Sc(t.metrics.padding);

        ImGui::PopItemWidth();
        ImGui::PopTextWrapPos();
        ImGui::EndGroup();
        w->WorkRect.Max.x = store.savedWorkMaxX;
        w->ContentRegionRect.Max.x = store.savedContentMaxX;
        float y1 = ImGui::GetItemRectMax().y + pad;
        if (frame->rows > 0)
            y1 = frame->y0 + (float)frame->rows;
        const Rect card(frame->x0, frame->y0, frame->x1, y1);

        frame->splitter.SetCurrentChannel(frame->drawList, 0);
        Painter p(frame->drawList);
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
        p.Rect(card, s);
        frame->splitter.Merge(frame->drawList);
        if (frame->lookPushed)
            PopGlassLook();

        ImGui::SetCursorScreenPos(Vec2(frame->x0, frame->y0));
        ImGui::ItemSize(card.Size());
    }
}
