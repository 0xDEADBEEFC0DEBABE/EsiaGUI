// Esia UI - the navigation stack (pages pushed and popped with iOS transitions) and the floating liquid-glass tab
// bar (WGT's navigation.cpp, on the core: pages are child regions, the tab bar a floating one).
#include "ui_internal.hpp"
#include <algorithm>
#include <unordered_map>

namespace esia::ui
{
    using namespace detail;

    namespace detail
    {
        struct NavState
        {
            std::vector<std::string> stack;   // page ids, the root first
            std::string outgoing;             // the page leaving during a transition
            int dir = 0;                      // +1 push, -1 pop
            SpringState progress;
            bool transitioning = false;
            std::unordered_map<std::string, std::string> titles;   // for the back buttons
        };
    }

    namespace
    {
        NavState* CurrentNav()
        {
            Ui::Impl& m = M();
            ESIA_ASSERT(!m.navs.empty() && "a navigation call outside BeginNavigation / EndNavigation");
            return m.navs.empty() ? nullptr : m.navs.back().state;
        }

        void StartTransition(NavState& ns, int dir)
        {
            ns.dir = dir;
            ns.progress.value = 0.0f;
            ns.progress.velocity = 0.0f;
            ns.transitioning = true;
        }
    }

    bool BeginNavigation(std::string_view id, std::string_view rootPage)
    {
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const Id nid = c.GetId(id);
        NavState& ns = c.State<NavState>(nid);
        if (ns.stack.empty())
            ns.stack.emplace_back(rootPage);
        const Vec2 size(std::max(AvailableWidth(), 1.0f), std::max(c.ContentRegionAvail().y, 1.0f));
        if (ns.transitioning)
        {
            // critically damped: no overshoot; done once the moving page is within half a pixel of its place
            ns.progress.Step(1.0f, Spring{0.36f, 1.0f}, m.dt);
            m.animating = true;
            if ((1.0f - ns.progress.value) * size.x < 0.5f)
            {
                ns.transitioning = false;
                ns.outgoing.clear();
            }
        }
        Ui::Impl::NavEntry e;
        e.state = &ns;
        e.origin = c.CursorPos();
        e.size = size;
        ChildOptions co;
        co.size = size;
        c.BeginChild(id, co);
        m.navs.push_back(e);
        return true;
    }

    void EndNavigation()
    {
        Ui::Impl& m = M();
        ESIA_ASSERT(!m.navs.empty() && "EndNavigation without BeginNavigation");
        if (m.navs.empty())
            return;
        m.navs.pop_back();
        m.ctx->EndChild();
    }

    bool BeginPage(std::string_view pageId, std::string_view title)
    {
        Ui::Impl& m = M();
        NavState* ns = CurrentNav();
        if (!ns)
            return false;
        Context& c = *m.ctx;
        const Ui::Impl::NavEntry nav = m.navs.back();
        const std::string key(pageId);
        const std::string& shownTitle = ns->titles[key] = title.empty() ? key : std::string(title);
        const bool isCurrent = ns->stack.back() == key;
        const bool isOutgoing = ns->transitioning && ns->outgoing == key;
        if (!isCurrent && !isOutgoing)
            return false;

        const float W = nav.size.x, H = nav.size.y;
        const float prog = ns->transitioning ? Saturate(ns->progress.value) : 1.0f;
        // Push: the new page slides in from the right over the current one. Pop: the top page slides out to the right
        // and uncovers the one below. The top page acts as opaque (as on iOS): the page below is drawn only in the
        // strip left of the top page's edge, while it drifts left (parallax) and dims. Pages are glass, so without
        // the clip both pages' text would show through each other.
        float x = 0.0f, alpha = 1.0f;
        bool below = false;
        float edge = W;   // left edge of the top page, from the navigation's left
        if (ns->transitioning)
        {
            const float covered = ns->dir > 0 ? prog : 1.0f - prog;   // how far the top page covers the one below
            edge = std::floor(ns->dir > 0 ? (1.0f - prog) * W : prog * W);
            below = (ns->dir > 0) != isCurrent;
            if (below)
            {
                if (edge < 1.0f)
                    return false;   // fully covered: not drawn at all
                x = -0.3f * covered * W;
                alpha = 1.0f - 0.6f * covered;
            }
            else
                x = edge;
        }

        if (below)
            c.PushClipRect(Rect(nav.origin, Vec2(nav.origin.x + edge, nav.origin.y + H)));
        PushStyle(ItemStyle().Opacity(alpha));
        const Rect pr = Rect::FromSize(Vec2(nav.origin.x + std::floor(x), nav.origin.y), Vec2(W, H));
        c.SetCursorPos(pr.min);
        ChildOptions pco;
        pco.size = pr.Size();
        c.BeginChild(pageId, pco);
        const Palette& pc = C();
        const float barH = Sc(44);

        // the navigation bar: the title, and a back button titled after the page below
        Painter p = GetPainter();
        const text::FontRef tf = Font(TextStyle::Headline);
        const Vec2 ts = MeasureText(tf, shownTitle);
        p.Text(Vec2(std::floor(pr.Center().x - ts.x * 0.5f), std::floor(pr.min.y + barH * 0.5f - ts.y * 0.5f)), tf, pc.label, shownTitle);

        const bool canGoBack = ns->stack.size() > 1 && isCurrent && ns->stack.front() != key;
        const bool showBack = canGoBack || (isOutgoing && ns->dir < 0) || (isOutgoing && ns->dir > 0 && ns->stack.size() > 2);
        if (showBack)
        {
            const std::string* prevId = nullptr;
            for (std::size_t i = 1; i < ns->stack.size(); ++i)
                if (ns->stack[i] == key)
                    prevId = &ns->stack[i - 1];
            if (!prevId && isOutgoing && ns->dir < 0)
                prevId = &ns->stack.back();   // popped: the page it uncovers
            std::string_view backTitle = "Back";
            if (prevId)
            {
                const auto it = ns->titles.find(*prevId);
                if (it != ns->titles.end())
                    backTitle = it->second;
            }
            const text::FontRef bf = Font(TextStyle::Body);
            const Vec2 bs = MeasureText(bf, backTitle);
            const float maxW = pr.Width() * 0.5f - ts.x * 0.5f - Sc(28);
            const Rect br(pr.min.x, pr.min.y, pr.min.x + std::min(bs.x, std::max(maxW, 0.0f)) + Sc(30), pr.min.y + barH);
            const Interaction it = InteractImpl(c.GetId("##back"), br, InteractFlags_None);
            const Color accent = Accent().Fade(1.0f - 0.4f * it.press);
            DrawIcon(p, Vec2(br.min.x + Sc(8), br.Center().y), icons::ChevronLeft, Sc(15), accent);
            p.TextBox(Rect(br.min.x + Sc(22), br.min.y, br.max.x, br.max.y), Vec2(0.0f, 0.5f), bf, accent, backTitle, text::TextFlags_Ellipsis);
            if (it.pressed && isCurrent && !ns->transitioning)
                NavigationPop();
        }

        // the page's content scrolls under the bar
        c.SetCursorPos(Vec2(pr.min.x, pr.min.y + barH));
        ChildOptions sco;
        sco.size = Vec2(W, std::max(1.0f, H - barH));
        sco.padding = Vec2(0.0f, Sc(4));
        sco.flags = ChildFlags_ScrollY | ChildFlags_SmoothScroll;
        Ui::Impl::PageEntry pe;
        pe.child = c.GetId("##page");
        ScrollBegin(pe.child);
        c.BeginChild("##page", sco);
        pe.clipped = below;
        pe.edgeFade = BeginScrollEdgeFade();
        if (below)
        {
            // the top page's edge casts a soft shadow onto the page below; it fades out as the move settles
            const float sx = nav.origin.x + edge;
            pe.edgeShadow = Rect(sx - Sc(14), nav.origin.y, sx, nav.origin.y + H);
            pe.shadowAlpha = Saturate(std::min(edge, W - edge) / Sc(40));
        }
        m.pages.push_back(pe);
        return true;
    }

    void EndPage()
    {
        Ui::Impl& m = M();
        ESIA_ASSERT(!m.pages.empty() && "EndPage without BeginPage");
        if (m.pages.empty())
            return;
        const Ui::Impl::PageEntry pe = m.pages.back();
        m.pages.pop_back();
        Context& c = *m.ctx;
        c.ItemSize(Vec2(0.0f, Sc(12)));
        EndScrollEdgeFade(pe.edgeFade);
        ScrollEnd(pe.child, WindowLane(c.ViewRect()));
        c.EndChild();   // the scrolling content
        if (pe.clipped && pe.shadowAlpha > 0.0f)
        {
            Painter p = GetPainter();
            const Color s = C().shadow;
            p.Rect(pe.edgeShadow, Style().Fill(Paint::Linear(s.Fade(0.0f), s.Fade(0.5f * pe.shadowAlpha), 0.0f)));
        }
        c.EndChild();   // the page
        PopStyle();
        if (pe.clipped)
            c.PopClipRect();
    }

    void NavigationPush(std::string_view pageId)
    {
        NavState* ns = CurrentNav();
        if (!ns || ns->stack.back() == pageId)
            return;
        ns->outgoing = ns->stack.back();
        ns->stack.emplace_back(pageId);
        StartTransition(*ns, +1);
    }

    void NavigationPop()
    {
        NavState* ns = CurrentNav();
        if (!ns || ns->stack.size() <= 1)
            return;
        ns->outgoing = ns->stack.back();
        ns->stack.pop_back();
        StartTransition(*ns, -1);
    }

    // =============================================================== tab bar
    bool TabBar(std::string_view id, int* selected, std::span<const TabItem> items)
    {
        ItemScope scope;
        const int count = (int)items.size();
        if (count <= 0)
            return false;
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const Theme& t = T();
        const Palette& pc = t.colors;
        const Id tid = c.GetId(id);

        // the bar floats over the bottom of the area it is submitted in (iOS): the content scrolls under the glass
        // and shows through it, and the bar takes the clicks over its area
        const Rect vis = c.ViewRect();
        const float barH = Sc(60);
        const float barW = std::max(std::min(vis.Width() - Sc(28), (float)count * Sc(88)), 1.0f);
        const Rect bar = Rect::FromCenter(Vec2(vis.Center().x, vis.max.y - Sc(14) - barH * 0.5f), Vec2(barW, barH));
        ChildOptions co;
        co.flags = ChildFlags_Floating;
        co.rect = bar;
        DrawList& dl = c.WindowDrawList();
        const std::size_t from = dl.Mark();
        c.BeginChild(id, co);
        Painter p = GetPainter();
        {
            ScopedUnclip unclip(bar, ShadowExtent(Sc(24), Vec2(0, Sc(8))));
            DrawPill(p, bar, Style().Glass(LookMaterial(t.materials.bar)).Shadow(pc.shadow.Fade(0.6f), Sc(24), Vec2(0, Sc(8))));   // the material carries its veil
        }

        const float itemW = bar.Width() / (float)count;
        // tap a tab or drag the selection: it travels as a clear lens over the items and lands as a pill
        const LiquidSelection sel = LiquidSelect(tid, bar, count, selected);
        const Rect pill = LiquidSelectionRect(sel, bar, count, Sc(5));
        const float solid = 1.0f - Saturate(sel.lens);
        const ItemStyle& st = ResolvedStyle();
        if (solid > 0.01f)
        {
            const Color selFill = st.Has(ItemStyle::kSelectedFill) ? st.selectedFill : pc.label.Fade(t.dark ? 0.14f : 0.08f);
            p.Rect(pill, Style().Radius(pill.Height() * 0.5f).Fill(selFill.Fade(solid)));
        }
        for (int i = 0; i < count; ++i)
        {
            const TabItem& item = items[(std::size_t)i];
            const Rect ir(bar.min.x + itemW * (float)i, bar.min.y, bar.min.x + itemW * (float)(i + 1), bar.max.y);
            // the highlight follows the sliding pill (and its overshoot), not the selected index
            const float on = SmoothStep(0.2f, 0.8f, Saturate(1.0f - std::fabs(sel.pos - (float)i)));
            const float hov = Anim(tid, 400u + (std::uint32_t)i, i == sel.hovered ? 1.0f : 0.0f, SpringFast());
            const Color col = Lerp(LabelOr(pc.label).Fade(0.85f + 0.15f * hov), st.Has(ItemStyle::kSelectedLabel) ? st.selectedLabel : Accent(), on);
            const std::string_view label = VisibleLabel(item.label);
            const float iconY = label.empty() ? ir.Center().y : ir.Center().y - Sc(8);
            if (item.icon)
                DrawIcon(p, Vec2(ir.Center().x, iconY), item.icon, Sc(19) * (1.0f + 0.06f * on), col);
            if (!label.empty())
            {
                const text::FontRef f = Font(on > 0.5f ? FontWeight::Semibold : FontWeight::Regular, 10.5f);
                const Vec2 ts = MeasureText(f, label);
                p.Text(Vec2(std::floor(ir.Center().x - ts.x * 0.5f), std::floor(ir.Center().y + Sc(7))), f, col, label);
            }
        }
        DrawSelectionLens(p, pill, sel, Sc(8), st.Has(ItemStyle::kMovingFill) ? st.movingFill : Color::Clear());
        c.EndChild();
        m.floats.push_back({&dl, from, dl.Mark()});   // drawn over the content at EndFrame
        // room below the content, so its end can scroll out from under the bar
        c.ItemSize(Vec2(0.0f, barH + Sc(22)));
        return sel.changed;
    }

    // ============================================================ search bar
    TextFieldResult SearchBar(std::string_view id, std::string* text, const SearchBarOptions& so)
    {
        ItemScope scope;
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const Theme& t = T();
        const Palette& pc = t.colors;
        // floats over the bottom of its area like the tab bar: the list scrolls under the glass
        const Rect vis = c.ViewRect();
        const float barH = Sc(46);
        const Rect bar(vis.min.x + Sc(14), vis.max.y - Sc(14) - barH, std::max(vis.max.x - Sc(14), vis.min.x + Sc(15)), vis.max.y - Sc(14));
        DrawList& dl = c.WindowDrawList();
        const std::size_t from = dl.Mark();
        ChildOptions co;
        co.flags = ChildFlags_Floating;
        co.rect = bar;
        c.BeginChild(id, co);
        Painter p = GetPainter();
        {
            // clear glass by default (no frost, no tint) with a soft base inside; a Clear look from the style makes it
            // fully transparent (no base), another look from the style overrides so.look
            const bool clear = LookClear();
            const ItemStyle& st = ResolvedStyle();
            GlassMaterial mat = st.Has(ItemStyle::kLook) || clear ? StyledMaterial(t.materials.bar) : GlassFieldsOver(ApplyLook(so.look, t.materials.bar));
            Style bs = Style().Glass(mat).Shadow(pc.shadow.Fade(0.25f), Sc(16), Vec2(0, Sc(4)));
            if ((so.base && !clear) || st.Has(ItemStyle::kFill))
                bs.Fill(so.fill.a > 0.0f ? so.fill : FillOr(t.dark ? Color(0.11f, 0.11f, 0.13f, 0.55f) : Color::White(0.6f)));
            ScopedUnclip unclip(bar, ShadowExtent(Sc(18), Vec2(0, Sc(6))));
            DrawPill(p, bar, bs);
        }
        const float fieldH = Sc(38);
        c.SetCursorPos(Vec2(bar.min.x + Sc(6), std::floor(bar.Center().y - fieldH * 0.5f)));
        TextFieldOptions o;
        o.icon = icons::Search;
        o.background = false;   // the glass is the field
        o.width = (bar.Width() - Sc(12)) / Sc(1.0f);
        const TextFieldResult r = TextField("##field", text, so.placeholder, o);
        c.EndChild();
        m.floats.push_back({&dl, from, dl.Mark()});   // drawn over the content at EndFrame
        c.ItemSize(Vec2(0.0f, barH + Sc(22)));
        return r;
    }
}
