// Esia UI - glass popups on the core's popups: menus, the picker (a pop-up button with a menu of choices) and
// tooltips (WGT's BeginGlassPopup / PopupRow / Picker / Tooltip in controls.cpp).
#include "ui_internal.hpp"
#include <algorithm>

namespace esia::ui
{
    using namespace detail;

    namespace
    {
        Rect Scaled(const Rect& r, float k)
        {
            const Vec2 c = r.Center(), h = r.Size() * (0.5f * k);
            return Rect(c - h, c + h);
        }

        struct GlassPopupState
        {
            Vec2 rows;                   // what the rows measured last frame
            bool measured = false;       // ... since it opened
            bool scrolled = false;       // its scroll area was opened since it opened (centerOn is applied once)
        };

        // a glass popup being submitted: its rows may be in a scroll area
        struct GlassPopupFrame
        {
            Id id = 0;
            bool scrolls = false;
            Id child = 0;
            bool edgeFade = false;
        };
        thread_local std::vector<GlassPopupFrame> g_glassPopups;
    }

    // ================================================================ popups
    bool detail::BeginGlassPopup(Id id, PopupOptions options, float minWidth, float maxHeight, float centerOn)
    {
        Context& c = Ctx();
        const float pad = Sc(6);
        GlassPopupState& st = c.State<GlassPopupState>(Salt(id, 0x52));
        // the rows' height at most: maxHeight, and what fits on the display (a menu never runs off it)
        float rowsMax = std::max(c.SafeArea().Height() - Sc(24) - pad * 2.0f, Sc(68));
        if (maxHeight > 0.0f)
            rowsMax = std::min(rowsMax, Sc(maxHeight));
        options.minSize = Vec2(std::max(options.minSize.x, minWidth), options.minSize.y);
        options.maxSize.y = std::min(options.maxSize.y, rowsMax + pad * 2.0f);   // also the frame it measured them unscrolled
        options.padding = Vec2(pad, pad);
        if (!c.BeginPopup(id, options))
        {
            st.measured = st.scrolled = false;
            return false;
        }
        const Window& w = *c.CurrentWindow();
        if (w.Appearing())
            st.measured = st.scrolled = false;
        // it fades in and grows from 96 % (a spring), on glass
        const Id aid = Salt(id, 0x50);
        if (w.Appearing())
            AnimSet(aid, 0.0f);
        const float a = ui::Anim(aid, 1.0f, SpringStd(), 0.0f);
        PushStyle(ItemStyle().Opacity(Saturate(a)));
        Painter p = GetPainter();
        const Rect wr = w.GetRect();
        {
            ScopedUnclip unclip(wr, ShadowExtent(Sc(28), Vec2(0, Sc(10))));
            p.Rect(Scaled(wr, 0.96f + 0.04f * a),
                   Style().Radius(Sc(14)).Glass(LookMaterial(T().materials.popover)).Shadow(C().shadow.Fade(1.2f), Sc(28), Vec2(0, Sc(10))));
        }
        // More rows than fit scroll inside the height they may take, in an area as wide as the widest (a fixed
        // size: an area sized by the popup would feed its size back to it). The frame a popup opens measures them
        // unscrolled (it is hidden then).
        GlassPopupFrame g;
        g.id = id;
        g.scrolls = st.measured && st.rows.y > rowsMax + 0.5f;
        if (g.scrolls)
        {
            if (!st.scrolled && centerOn >= 0.0f)
                c.SetNextScroll(Vec2(-1.0f, std::clamp(centerOn - rowsMax * 0.5f, 0.0f, st.rows.y - rowsMax)));
            st.scrolled = true;
            ChildOptions ch;
            ch.size = Vec2(std::max(st.rows.x, options.minSize.x - pad * 2.0f), rowsMax);
            ch.flags = ChildFlags_ScrollY | ChildFlags_SmoothScroll;
            g.child = c.GetId("##rows");
            ScrollBegin(g.child);
            c.BeginChild("##rows", ch);
            g.edgeFade = BeginScrollEdgeFade();
        }
        g_glassPopups.push_back(g);

        // its rows follow each other without gaps
        StackLayout& rows = c.State<StackLayout>(Salt(id, 0x51));
        rows.horizontal = false;
        rows.spacing = 0.0f;
        rows.align = esia::Align::Start;
        ContainerOptions co;
        co.layout = &rows;
        c.BeginContainer(Salt(id, 0x51), co);
        return true;
    }

    void detail::EndGlassPopup()
    {
        Context& c = Ctx();
        ESIA_ASSERT(!g_glassPopups.empty() && "EndGlassPopup without BeginGlassPopup");
        const GlassPopupFrame g = g_glassPopups.back();
        g_glassPopups.pop_back();
        GlassPopupState& st = c.State<GlassPopupState>(Salt(g.id, 0x52));
        st.rows = c.EndContainer().Size();
        st.measured = true;
        if (g.scrolls)
        {
            EndScrollEdgeFade(g.edgeFade);
            ScrollEnd(g.child, -1.0f);
            c.EndChild();
        }
        PopStyle();
        c.EndPopup();
    }

    bool detail::PopupRow(std::string_view label, bool selected, Icon icon)
    {
        Context& c = Ctx();
        const text::FontRef f = Font(TextStyle::Body);
        const std::string_view shown = VisibleLabel(label);
        const Vec2 ts = MeasureText(f, shown);
        // The popup sizes itself to its widest row (what each measures: ItemSize); every row then spans the popup
        // (or the scroll area its rows are in) as wide as it is now.
        const float natural = ts.x + Sc(64), h = Sc(34);
        const Vec2 pos = c.CursorPos();
        const float width = std::max(natural, c.WorkRect().max.x - pos.x);
        c.ItemSize(Vec2(natural, h));
        const Interaction it = InteractImpl(c.GetId(label), Rect::FromSize(pos, Vec2(width, h)), InteractFlags_None);
        if (!it.visible)
            return false;
        const Palette& pc = C();
        Painter p = GetPainter();
        if (it.hover > 0.01f)
            p.Rect(it.rect, Style().Radius(Sc(9)).Fill(pc.highlight.Fade(it.hover * 1.4f)));
        float x = it.rect.min.x + Sc(10);
        if (selected)
            DrawIcon(p, Vec2(x + Sc(7), it.rect.Center().y), icons::Checkmark, Sc(13), pc.label);
        x += Sc(24);
        if (icon)
        {
            DrawIcon(p, Vec2(x + Sc(8), it.rect.Center().y), icon, Sc(14), pc.label);
            x += Sc(24);
        }
        p.Text(Vec2(x, std::floor(it.rect.Center().y - ts.y * 0.5f)), f, pc.label, shown);
        return it.pressed;
    }

    // ================================================================= menus
    void OpenMenu(std::string_view id)
    {
        Context& c = Ctx();
        c.OpenPopup(c.GetId(id));
    }

    bool BeginMenu(std::string_view id, const MenuOptions& o)
    {
        Context& c = Ctx();
        PopupOptions po;
        if (o.anchor.x >= 0.0f && o.anchor.y >= 0.0f)
            po.pos = o.anchor;
        po.pivot = o.pivot;
        return BeginGlassPopup(c.GetId(id), po, Sc(o.minWidth), o.maxHeight);
    }

    bool MenuItem(std::string_view label, Icon icon, bool checked)
    {
        if (!PopupRow(label, checked, icon))
            return false;
        Ctx().CloseCurrentPopup();
        return true;
    }

    void EndMenu() { EndGlassPopup(); }

    // ================================================================ picker
    bool detail::PickerAt(Id id, const Rect& r, int* selected, std::span<const std::string_view> items, bool plain, int maxRows)
    {
        Context& c = Ctx();
        const Palette& pc = C();
        const int count = (int)items.size();
        const int sel = count > 0 ? std::clamp(*selected, 0, count - 1) : -1;
        const std::string_view current = sel >= 0 ? VisibleLabel(items[(std::size_t)sel]) : std::string_view();
        const Interaction it = InteractImpl(id, r, InteractFlags_None);
        Painter p = GetPainter();
        const text::FontRef f = Font(TextStyle::Body);
        const Vec2 ts = MeasureText(f, current);
        const float chevron = Sc(10);
        if (it.visible)
        {
            if (!plain)
            {
                Style well = Style().Radius(ItemRadius(Sc(10)));
                p.Rect(r, SurfaceFill(well, FillOr(pc.tertiaryFill).Fade(1.0f + 0.5f * it.hover)));
            }
            // plain (in a list row): the choice and the chevron at the right, in the secondary color
            const float right = r.max.x - (plain ? Sc(2) : Sc(10));
            const Color fg = LabelOr(plain ? pc.secondaryLabel : pc.label);
            DrawIcon(p, Vec2(right - chevron * 0.5f, r.Center().y), icons::ChevronDown, chevron, fg.Fade(0.9f));
            const float tx = plain ? std::max(right - chevron - Sc(6) - ts.x, r.min.x) : r.min.x + Sc(10);
            p.TextBox(Rect(std::floor(tx), r.min.y, right - chevron - Sc(4), r.max.y), Vec2(0.0f, 0.5f), f, fg.Fade(1.0f - 0.3f * it.press), current,
                      text::TextFlags_Ellipsis);
        }

        // the menu opens under the button, right-aligned with it; a long list scrolls inside it, at the choice
        const Id popupId = Salt(id, 0x60);
        if (it.pressed)
            c.OpenPopup(popupId);
        bool changed = false;
        PopupOptions po;
        po.pos = Vec2(r.max.x, r.max.y + Sc(6));
        po.pivot = Vec2(1.0f, 0.0f);
        const float rowH = Sc(34);   // PopupRow's
        if (BeginGlassPopup(popupId, po, std::max(r.Width(), Sc(180)), maxRows > 0 ? 34.0f * (float)maxRows : 0.0f, sel >= 0 ? ((float)sel + 0.5f) * rowH : -1.0f))
        {
            for (int i = 0; i < count; ++i)
            {
                c.PushId(i);
                if (PopupRow(items[(std::size_t)i], i == sel))
                {
                    *selected = i;
                    changed = true;
                    c.CloseCurrentPopup();
                }
                c.PopId();
            }
            EndGlassPopup();
        }
        return changed;
    }

    bool Picker(std::string_view id, int* selected, std::span<const std::string_view> items, const PickerOptions& o)
    {
        ItemScope scope;
        Context& c = Ctx();
        const float width = o.width;
        float w = width > 0.0f ? Sc(width) : 0.0f;
        if (width < 0.0f)
            w = AvailableWidth();
        if (w == 0.0f)
        {
            // as wide as the widest choice
            const text::FontRef f = Font(TextStyle::Body);
            for (const std::string_view item : items)
                w = std::max(w, MeasureText(f, VisibleLabel(item)).x);
            w += Sc(44);
        }
        const Vec2 size(w, Sc(34));
        const Vec2 pos = c.CursorPos();
        c.ItemSize(size);
        return PickerAt(c.GetId(id), Rect::FromSize(pos, size), selected, items, false, o.maxRows);
    }

    // ============================================================== tooltips
    void Tooltip(std::string_view text)
    {
        ItemScope scope;
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const ItemStatus st = c.LastItemStatus();
        if (!st.hovered)
        {
            if (st.rect == m.tooltipRect)
                m.tooltipItem = 0;   // left: the next hover waits again
            return;
        }
        // it appears after a short hover over the same item
        if (m.tooltipItem != c.HoveredId())
        {
            m.tooltipItem = c.HoveredId();
            m.tooltipRect = st.rect;
            m.tooltipSince = m.time;
        }
        if (m.time - m.tooltipSince < 0.5)
        {
            m.animating = true;   // a frame when the wait is over
            return;
        }
        c.BeginTooltip();
        const Rect tr = c.CurrentWindow()->GetRect();
        {
            Painter p = GetPainter();
            ScopedUnclip unclip(tr, ShadowExtent(Sc(16), Vec2(0, Sc(6))));
            p.Rect(tr, Style().Radius(Sc(10)).Glass(LookMaterial(T().materials.popover)).Shadow(C().shadow, Sc(16), Vec2(0, Sc(6))));
        }
        TextImpl(Font(TextStyle::Footnote), C().label, text, false);
        c.EndTooltip();
    }
}
