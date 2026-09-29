// WGT UI - navigation stack (push / pop transitions) and the floating liquid-glass tab bar.
#include "ui/ui_internal.hpp"

namespace wgt::ui
{
    using namespace detail;

    namespace
    {
        struct NavState
        {
            std::vector<std::string> stack;
            std::string outgoing;
            int dir = 0;
            SpringState progress;
            bool transitioning = false;
            std::unordered_map<std::string, std::string> titles;
        };

        NavState* CurrentNav()
        {
            Context::Impl& impl = Ctx();
            IM_ASSERT(!impl.ui.navs.empty() && "Navigation call outside BeginNavigation()/EndNavigation()");
            return impl.ui.navs.empty() ? nullptr : static_cast<NavState*>(impl.ui.navs.back().state);
        }

        void StartTransition(NavState& ns, int dir)
        {
            ns.dir = dir;
            ns.progress.value = 0.0f;
            ns.progress.velocity = 0.0f;
            ns.transitioning = true;
        }
    }

    bool BeginNavigation(const char* id, const char* rootPage)
    {
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return false;
        Context::Impl& impl = Ctx();
        const ImGuiID nid = w->GetID(id);
        NavState& ns = impl.state.Get<NavState>(nid);
        if (ns.stack.empty())
            ns.stack.push_back(rootPage);
        const Vec2 size(AvailWidth(), ImGui::GetContentRegionAvail().y);
        if (ns.transitioning)
        {
            // critically damped: no overshoot; done once the moving page is within half a pixel of its place
            ns.progress.Step(1.0f, Spring{0.36f, 1.0f}, impl.dt);
            if ((1.0f - ns.progress.value) * size.x < 0.5f)
            {
                ns.transitioning = false;
                ns.outgoing.clear();
            }
        }
        ImGui::BeginChild(id, size, ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        NavFrame f;
        f.id = nid;
        f.state = &ns;
        f.width = size.x;
        f.height = size.y;
        impl.ui.navs.push_back(f);
        return true;
    }

    void EndNavigation()
    {
        Context::Impl& impl = Ctx();
        IM_ASSERT(!impl.ui.navs.empty());
        impl.ui.navs.pop_back();
        ImGui::EndChild();
    }

    bool BeginPage(const char* pageId, const char* title)
    {
        Context::Impl& impl = Ctx();
        NavState* ns = CurrentNav();
        if (!ns)
            return false;
        const NavFrame& nav = impl.ui.navs.back();
        ns->titles[pageId] = title ? title : pageId;
        const bool isCurrent = ns->stack.back() == pageId;
        const bool isOutgoing = ns->transitioning && ns->outgoing == pageId;
        if (!isCurrent && !isOutgoing)
            return false;

        const float W = nav.width;
        const float prog = ns->transitioning ? Saturate(ns->progress.value) : 1.0f;
        // Push: the new page slides in from the right over the current one. Pop: the top page slides out to the
        // right and uncovers the one below. The top page acts as opaque (like iOS): the page below is drawn only
        // in the strip left of the top page's edge, while it drifts left (parallax) and dims. Pages are glass,
        // so without the clip both pages' text would show through each other.
        float x = 0.0f, alpha = 1.0f;
        bool below = false;
        float edge = W;   // left edge of the top page, relative to the navigation area
        if (ns->transitioning)
        {
            const float covered = ns->dir > 0 ? prog : 1.0f - prog;   // how far the top page covers the one below
            edge = std::floor(ns->dir > 0 ? (1.0f - prog) * W : prog * W);
            below = (ns->dir > 0) != isCurrent;
            if (below)
            {
                x = -0.3f * covered * W;
                alpha = 1.0f - 0.6f * covered;
                if (edge < 1.0f)
                    return false;   // fully covered: not drawn at all
            }
            else
                x = edge;
        }

        ImGuiWindow* navWindow = ImGui::GetCurrentWindow();
        if (below)
            ImGui::PushClipRect(navWindow->Pos, Vec2(navWindow->Pos.x + edge, navWindow->Pos.y + nav.height), true);
        ImGui::SetCursorPos(Vec2(std::floor(x), 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * alpha);
        ImGui::BeginChild(pageId, Vec2(W, nav.height), ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGuiWindow* pw = ImGui::GetCurrentWindow();
        const Palette& c = C();
        const Rect pr(pw->Pos, pw->Pos + pw->Size);
        const float barH = Sc(44);

        // navigation bar
        Painter p(pw->DrawList);
        const FontRef tf = GetFont(TextStyle::Headline);
        const Vec2 ts = Painter::MeasureText(tf, title);
        p.Text(Vec2(std::floor(pr.Center().x - ts.x * 0.5f), std::floor(pr.min.y + barH * 0.5f - ts.y * 0.5f)), tf, c.label, title);

        const bool canGoBack = ns->stack.size() > 1 && isCurrent && ns->stack.front() != pageId;
        const bool showBack = canGoBack || (isOutgoing && ns->dir < 0) || (isOutgoing && ns->dir > 0 && ns->stack.size() > 2);
        if (showBack)
        {
            // title of the page below this one
            const std::string* prevId = nullptr;
            for (size_t i = 0; i < ns->stack.size(); ++i)
                if (ns->stack[i] == pageId && i > 0)
                    prevId = &ns->stack[i - 1];
            if (!prevId && isOutgoing && ns->dir < 0)
                prevId = &ns->stack.back();
            const char* backTitle = "Back";
            if (prevId)
            {
                auto it = ns->titles.find(*prevId);
                if (it != ns->titles.end())
                    backTitle = it->second.c_str();
            }
            const FontRef bf = GetFont(TextStyle::Body);
            const Vec2 bs = Painter::MeasureText(bf, backTitle);
            const float maxW = pr.Width() * 0.5f - ts.x * 0.5f - Sc(28);
            const Rect br(pr.min.x, pr.min.y, pr.min.x + std::min(bs.x, std::max(maxW, 0.0f)) + Sc(30), pr.min.y + barH);
            Interaction it = InteractImpl(pw->GetID("##back"), br, InteractFlags_None);
            const Color accent = Accent().Fade(1.0f - 0.4f * it.press);
            p.Icon(Vec2(br.min.x + Sc(8), br.Center().y), icons::ChevronLeft, Sc(15), accent);
            p.PushClip(Rect(br.min.x, br.min.y, br.max.x, br.max.y));
            p.Text(Vec2(br.min.x + Sc(22), std::floor(br.Center().y - bs.y * 0.5f)), bf, accent, backTitle);
            p.PopClip();
            if (it.pressed && isCurrent && !ns->transitioning)
                NavigationPop();
        }

        ImGui::SetCursorPos(Vec2(0.0f, barH));
        ScrollAreaBegin(Salt(pw->GetID("##pagescroll"), 1), "##page", Vec2(W, std::max(1.0f, nav.height - barH)), Vec2(0, Sc(4)), true);
        PageFrame pf;
        pf.styleVars = 1;
        pf.clipped = below;
        if (below)
        {
            // the top page's edge casts a soft shadow onto the page below; it fades out as the move settles
            const float sx = navWindow->Pos.x + edge;
            pf.edgeShadow = Rect(sx - Sc(14), navWindow->Pos.y, sx, navWindow->Pos.y + nav.height);
            pf.shadowAlpha = Saturate(std::min(edge, W - edge) / Sc(40));
        }
        impl.ui.pages.push_back(pf);
        return true;
    }

    void EndPage()
    {
        Context::Impl& impl = Ctx();
        IM_ASSERT(!impl.ui.pages.empty());
        const PageFrame pf = impl.ui.pages.back();
        impl.ui.pages.pop_back();
        ImGui::Dummy(Vec2(0, Sc(12)));
        ScrollAreaEnd();
        if (pf.clipped && pf.shadowAlpha > 0.0f)
        {
            Painter p;
            const Color s = C().shadow;
            p.Rect(pf.edgeShadow, Style().Fill(Paint::Linear(s.Fade(0.0f), s.Fade(0.5f * pf.shadowAlpha), 0.0f)));
        }
        ImGui::EndChild();
        ImGui::PopStyleVar(pf.styleVars);
        if (pf.clipped)
            ImGui::PopClipRect();
    }

    void NavigationPush(const char* pageId)
    {
        NavState* ns = CurrentNav();
        if (!ns || ns->stack.back() == pageId)
            return;
        ns->outgoing = ns->stack.back();
        ns->stack.push_back(pageId);
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
    bool TabBar(const char* id, int* selected, const TabItem* items, int count)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems || count <= 0)
            return false;
        const Theme& t = T();
        const Palette& c = t.colors;
        const ImGuiID tid = w->GetID(id);

        const Rect vis(w->InnerRect.Min, w->InnerRect.Max);
        const float barH = Sc(60);
        const float barW = std::min(vis.Width() - Sc(28), count * Sc(88));
        const Rect bar = Rect::FromCenter(Vec2(vis.Center().x, vis.max.y - Sc(14) - barH * 0.5f), Vec2(barW, barH));

        const ImVec2 backupPos = w->DC.CursorPos, backupMax = w->DC.CursorMaxPos;
        const ImVec2 backupPrevPos = w->DC.CursorPosPrevLine;
        const float backupPrevLine = w->DC.PrevLineSize.y, backupCurrLine = w->DC.CurrLineSize.y;
        // the bar floats over the content (iOS): content scrolls under the glass and shows through it; the
        // scroll room added below keeps the last row reachable, and the bar takes the clicks over its area
        ImGui::SetCursorScreenPos(bar.min);
        ImGui::BeginChild(id, bar.Size(), ImGuiChildFlags_None,
                          ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings);
        ImGuiWindow* cw = ImGui::GetCurrentWindow();
        Painter p(cw->DrawList);
        {
            ScopedUnclip unclip(cw->DrawList, bar, ShadowExtent(Sc(24), Vec2(0, Sc(8))));
            DrawPill(p, bar, Style().Glass(LookMaterial(t.materials.bar)).Shadow(c.shadow.Fade(0.6f), Sc(24), Vec2(0, Sc(8))));   // the material carries its own veil
        }

        const float itemW = bar.Width() / count;
        // tap a tab or drag the selection: it travels as a clear lens over the items and lands as a pill
        const LiquidSelection sel = LiquidSelect(tid, bar, count, selected);
        const bool changed = sel.changed;
        const int hoveredIdx = sel.hovered;
        const float sx = sel.pos * itemW;
        const Rect pill = LiquidSelectionRect(sel, bar, count, Sc(5));
        const float solid = 1.0f - Saturate(sel.lens);
        if (solid > 0.01f)
        {
            const ItemStyle& st = ResolvedStyle();
            const Color selFill = st.Has(ItemStyle::kSelectedFill) ? st.selectedFill : c.label.Fade(t.dark ? 0.14f : 0.08f);
            p.Rect(pill, Style().Radius(pill.Height() * 0.5f).Fill(selFill.Fade(solid)));
        }

        for (int i = 0; i < count; ++i)
        {
            const Rect ir(bar.min.x + itemW * i, bar.min.y, bar.min.x + itemW * (i + 1), bar.max.y);
            // highlight follows the sliding pill (and its overshoot), not the selected index
            const float on = SmoothStep(0.2f, 0.8f, Saturate(1.0f - std::fabs(sx / itemW - (float)i)));
            const float hov = Anim(tid, 400 + i, i == hoveredIdx ? 1.0f : 0.0f, SpringFast());
            const ItemStyle& st = ResolvedStyle();
            const Color col = Lerp(LabelOr(c.label).Fade(0.85f + 0.15f * hov), st.Has(ItemStyle::kSelectedLabel) ? st.selectedLabel : Accent(), on);
            const bool hasLabel = items[i].label && *items[i].label;
            const float iconY = hasLabel ? ir.Center().y - Sc(8) : ir.Center().y;
            if (items[i].icon)
                p.Icon(Vec2(ir.Center().x, iconY), items[i].icon, Sc(19) * (1.0f + 0.06f * on), col);
            if (hasLabel)
            {
                const FontRef f = Font(on > 0.5f ? FontWeight::Semibold : FontWeight::Regular, 10.5f);
                const Vec2 ts = Painter::MeasureText(f, items[i].label);
                p.Text(Vec2(std::floor(ir.Center().x - ts.x * 0.5f), std::floor(ir.Center().y + Sc(7))), f, col, items[i].label);
            }
        }
        DrawSelectionLens(p, pill, sel, Sc(8), ResolvedStyle().Has(ItemStyle::kMovingFill) ? ResolvedStyle().movingFill : Color::Clear());
        Ctx().ui.floatingItem = true;   // the child's item in the parent floats over the content
        ImGui::EndChild();

        // the bar floats: restore the parent layout, then reserve scroll room below the content
        w->DC.CursorPos = backupPos;
        w->DC.CursorMaxPos = backupMax;
        w->DC.CursorPosPrevLine = backupPrevPos;
        w->DC.PrevLineSize.y = backupPrevLine;
        w->DC.CurrLineSize.y = backupCurrLine;
        ImGui::Dummy(Vec2(0, barH + Sc(22)));
        return changed;
    }

    // ============================================================ search bar
    bool SearchBar(const char* id, char* buffer, std::size_t bufferSize, const SearchBarOptions& so)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems)
            return false;
        const Theme& t = T();
        const Palette& c = t.colors;
        const Rect vis(w->InnerRect.Min, w->InnerRect.Max);
        const float barH = Sc(46);
        const Rect bar(vis.min.x + Sc(14), vis.max.y - Sc(14) - barH, vis.max.x - Sc(14), vis.max.y - Sc(14));

        const ImVec2 backupPos = w->DC.CursorPos, backupMax = w->DC.CursorMaxPos;
        const ImVec2 backupPrevPos = w->DC.CursorPosPrevLine;
        const float backupPrevLine = w->DC.PrevLineSize.y, backupCurrLine = w->DC.CurrLineSize.y;
        // floats over the content like the tab bar: the list scrolls under the glass
        ImGui::SetCursorScreenPos(bar.min);
        ImGui::BeginChild(id, bar.Size(), ImGuiChildFlags_None,
                          ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings);
        ImGuiWindow* cw = ImGui::GetCurrentWindow();
        Painter p(cw->DrawList);
        {
            // clear glass by default (no frost, no tint) with a soft base inside; a Clear look from
            // SetNextItemLook or a scope makes it fully transparent (no base), another look it sets overrides so.look
            const bool clear = LookClear();
            const ItemStyle st = ResolvedStyle();
            GlassMaterial m = st.Has(ItemStyle::kLook) || clear ? StyledMaterial(t.materials.bar) : ApplyLook(so.look, t.materials.bar);
            if (!st.Has(ItemStyle::kLook) && !clear)
                m = GlassFieldsOver(m);   // glass fields over the bar's own look
            Style bs = Style().Glass(m).Shadow(c.shadow.Fade(0.25f), Sc(16), Vec2(0, Sc(4)));
            if ((so.base && !clear) || st.Has(ItemStyle::kFill))
                bs.Fill(so.fill.a > 0.0f ? so.fill : FillOr(t.dark ? Color(0.11f, 0.11f, 0.13f, 0.55f) : Color::White(0.6f)));
            ScopedUnclip unclip(cw->DrawList, bar, ShadowExtent(Sc(18), Vec2(0, Sc(6))));
            DrawPill(p, bar, bs);
        }
        const float fieldH = Sc(38);
        ImGui::SetCursorScreenPos(Vec2(bar.min.x + Sc(6), std::floor(bar.Center().y - fieldH * 0.5f)));
        TextFieldOptions o;
        o.icon = icons::Search;
        o.background = false;   // the glass is the field
        o.width = (bar.Width() - Sc(12)) / Sc(1.0f);
        const bool changed = TextField("##field", buffer, bufferSize, so.placeholder ? so.placeholder : "Search", o);
        Ctx().ui.floatingItem = true;   // the child's item in the parent floats over the content
        ImGui::EndChild();

        w->DC.CursorPos = backupPos;
        w->DC.CursorMaxPos = backupMax;
        w->DC.CursorPosPrevLine = backupPrevPos;
        w->DC.PrevLineSize.y = backupPrevLine;
        w->DC.CurrLineSize.y = backupCurrLine;
        ImGui::Dummy(Vec2(0, barH + Sc(22)));
        return changed;
    }
}
