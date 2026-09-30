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
        // the very top or end.
        bool BeginScrollEdgeFade()
        {
            Context& c = Ctx();
            const Vec2 scroll = c.Scroll(), max = c.ScrollMax();
            const float top = std::min(std::max(scroll.y, 0.0f), Sc(22));
            const float bottom = std::min(std::max(max.y - scroll.y, 0.0f), Sc(30));
            if (top <= 0.5f && bottom <= 0.5f)
                return false;
            Painter p = GetPainter();
            p.BeginEdgeFade(c.WindowDrawList().ClipRect(), top, bottom);
            return true;
        }

        void EndScrollEdgeFade(bool started)
        {
            if (!started)
                return;
            Painter p = GetPainter();
            p.EndEdgeFade();
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
        c.BeginChild(id, co);
        Entry e;
        e.kind = Entry::Kind::Scroll;
        e.edgeFade = BeginScrollEdgeFade();
        m.containers.push_back(e);
        return true;
    }

    void EndScrollArea()
    {
        const Entry e = PopEntry(Entry::Kind::Scroll);
        EndScrollEdgeFade(e.edgeFade);
        Ctx().EndChild();
    }

    // ============================================================== windows
    bool BeginWindow(std::string_view title, bool* open, const WindowOptions& o)
    {
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const Theme& t = m.theme.current;
        const Palette& pc = t.colors;
        const Id id = c.GetId(title);

        struct WindowState
        {
            bool closing = false;   // the close button was pressed: fade out, then *open = false
            bool seen = false;
        };
        WindowState& st = c.State<WindowState>(id);
        const bool stylePushed = TakeNextStyle();   // the window and everything in it, until EndWindow

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

        c.SetNextWindowSize(o.size * t.metrics.scale, Cond::FirstUse);
        if (o.pos.x >= 0.0f)
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
        if (!c.Begin(title, wo))
        {
            c.End();
            if (stylePushed)
                PopStyle();
            return false;
        }
        Window* w = c.CurrentWindow();
        const Rect wr = w->GetRect();
        const float alpha = Saturate(vis);
        PushStyle(ItemStyle().Opacity(alpha));   // everything the window draws fades with it

        Entry e;
        e.kind = Entry::Kind::Window;
        e.id = id;
        e.stylePushed = stylePushed;
        e.windowFlags = o.flags;
        const float radius = ItemRadius(Sc(t.metrics.windowRadius));

        // ---------------------------------------------------------- surface
        const bool focused = c.FocusedWindow() == w;
        const float focusT = Anim(id, 0x78, focused ? 1.0f : 0.0f, t.motion.gentle);
        {
            Painter bg = GetPainter();
            Style ws;
            ws.Radius(radius);
            if (!(o.flags & WindowFlags_NoShadow))
                ws.Shadow(pc.shadow.Fade(0.9f + 0.5f * focusT), Sc(34 + 14 * focusT), Vec2(0, Sc(14 + 6 * focusT)));
            if ((o.flags & WindowFlags_Solid) && !LookClear())
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
        if (!(o.flags & WindowFlags_NoHeader))
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
                const float d = Sc(30);
                closeR = Rect::FromCenter(Vec2(wr.max.x - pad - d * 0.5f + Sc(4), cy), Vec2(d, d));
                const Interaction ci = InteractImpl(Salt(id, 0xC105E), closeR, InteractFlags_None);
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
                IconTile(p, Rect::FromCenter(Vec2(tx + Sc(13), cy), Vec2(Sc(26), Sc(26))), o.icon, Accent());
                tx += Sc(36);
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
        c.SetCursorPos(Vec2(wr.min.x, wr.min.y + headerH));
        ChildOptions co;
        co.size = Vec2(wr.Width(), std::max(1.0f, wr.Height() - headerH));
        co.padding = Vec2(padX, padY);
        if (!(o.flags & WindowFlags_NoScroll))
            co.flags = ChildFlags_ScrollY | ChildFlags_SmoothScroll;
        // the clip is measured from the content rect (inside the padding): out to half the padding at the sides, to
        // the header (or the corner cut) at the top and to the corner cut at the bottom
        co.clipInset = Rect(-(padX - side), headerH > cut ? -padY : cut - padY, -(padX - side), cut - padY);
        c.BeginChild("##content", co);
        e.edgeFade = !(o.flags & WindowFlags_NoScroll) && BeginScrollEdgeFade();
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
        c.EndChild();

        // resize affordance at the bottom-right corner, shown while the mouse is near it
        if (!(e.windowFlags & WindowFlags_NoResize))
        {
            const Rect wr = c.CurrentWindow()->GetRect();
            const float zone = Sc(26);
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
