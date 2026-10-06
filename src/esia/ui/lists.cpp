// Esia UI - inset grouped lists (iOS Settings): sections and rows (WGT's lists.cpp, on the core).
//
// A section is one item of its parent (header, card, footer), so it is one child of an auto-layout container. Its
// rows sit in a column without gaps (the core's StackLayout); the card behind them is drawn at EndSection, when its
// height is known, and moved under the rows (DrawList::Mark / MoveCommands), as cards are.
#include "ui_internal.hpp"
#include <algorithm>
#include <cstdio>

namespace esia::ui
{
    using namespace detail;
    using Section = Ui::Impl::SectionEntry;

    namespace
    {
        struct SectionPersist
        {
            Rect lastCard;
            bool valid = false;
        };

        struct RowLayout
        {
            Id id = 0;
            Rect rect;
            Rect accessory;      // where the accessory goes (never overlaps the label)
            Rect labelLine;      // the label's line, and the label's own width on it
            float labelWidth = 0.0f;
            bool stacked = false;
            bool visible = false;
            Interaction it;
        };

        // What a row's accessory needs. The label gets whatever is left; when even a short label and the accessory's
        // minimum do not fit side by side, the row stacks them (label on top, control below).
        struct Accessory
        {
            float natural = 0.0f;   // preferred width
            float min = 0.0f;       // narrowest usable width
            float height = 0.0f;    // height of the control (stacked layout)
            bool flexible = false;  // takes all the room the label leaves (sliders)
        };

        Section* CurrentSection()
        {
            Ui::Impl& m = M();
            ESIA_ASSERT(!m.sections.empty() && "a row outside BeginSection / EndSection");
            return m.sections.empty() ? nullptr : &m.sections.back();
        }

        // Lays out a row at the cursor. `reserve`: take its room in the section now (BeginRow leaves that to the
        // container its content goes in).
        RowLayout RowStart(std::string_view label, RowIcon icon, const Accessory& acc, float height, bool interactive, Color labelColor = Color::Clear(),
                           std::string_view idOverride = {}, bool reserve = true)
        {
            RowLayout L;
            Section* s = CurrentSection();
            if (!s)
                return L;
            Context& c = Ctx();
            const Theme& t = T();
            const Palette& pc = t.colors;
            const float pad = Sc(16), gap = Sc(12);
            const float iconW = icon.icon ? Sc(t.metrics.iconTile) + Sc(12) : 0.0f;
            const text::FontRef f = Font(TextStyle::Body);
            const std::string_view shown = VisibleLabel(label);
            const Vec2 ts = MeasureText(f, shown);

            // ---- measure: label vs accessory
            const float rowW = s->x1 - s->x0;
            const float avail = std::max(0.0f, rowW - pad * 2.0f - iconW);
            const float labelMin = std::min(ts.x, Sc(72));
            float accW = 0.0f, labelW = ts.x;
            if (acc.natural > 0.0f)
            {
                if (acc.flexible)
                {
                    // flexible controls (sliders) start on a shared column so a section lines up, but never before
                    // the end of their label
                    const float column = std::max(ts.x + gap, rowW * 0.38f - pad - iconW);
                    accW = std::max(acc.min, avail - column);
                    labelW = std::min(ts.x, avail - gap - accW);
                }
                else
                {
                    accW = ts.x + gap + acc.natural <= avail ? acc.natural : std::max(acc.min, avail - gap - ts.x);
                    labelW = std::min(ts.x, avail - gap - accW);
                }
                L.stacked = labelW < labelMin - 0.5f;
            }
            else
                labelW = std::min(ts.x, avail);

            float h = height > 0.0f ? Sc(height) : Sc(t.metrics.rowHeight);
            const float vPad = Sc(11), lineGap = Sc(8);
            if (L.stacked)
                h = std::max(h, vPad + ts.y + lineGap + acc.height + vPad);
            const Vec2 pos(s->x0, c.CursorPos().y);
            if (reserve)
                c.ItemSize(Vec2(rowW, h));
            L.rect = Rect(s->x0, pos.y, s->x1, pos.y + h);
            L.id = c.GetId(idOverride.empty() ? label : idOverride);

            Painter p = GetPainter();
            if (s->rows > 0)
                p.HLine(s->x0 + pad + iconW, s->x1, L.rect.min.y, pc.separator, std::max(1.0f, Sc(t.metrics.hairline) * 0.75f));
            s->rows++;

            if (interactive)
            {
                L.it = InteractImpl(L.id, L.rect, InteractFlags_None);
                L.visible = L.it.visible;
                const float hl = std::max(L.it.hover * 0.55f, L.it.press);
                if (L.visible && hl > 0.01f)
                {
                    p.PushMask(s->mask, ItemRadius(Sc(t.metrics.cardRadius)));
                    p.Rect(L.rect, Style().Fill(pc.highlight.Fade(hl * 1.3f)));
                    p.PopMask();
                }
            }
            else
                L.visible = c.ItemAdd(L.id, L.rect);

            // ---- place
            float x = L.rect.min.x + pad;
            const float labelY = L.stacked ? L.rect.min.y + vPad : std::floor(L.rect.Center().y - ts.y * 0.5f + 0.5f);
            if (icon.icon && L.visible)
            {
                const float tile = Sc(t.metrics.iconTile);
                const float iconCy = L.stacked ? labelY + ts.y * 0.5f : L.rect.Center().y;
                IconTile(p, Rect::FromCenter(Vec2(x + tile * 0.5f, iconCy), Vec2(tile, tile)), icon.icon, icon.color);
            }
            x += iconW;
            const float labelRoom = L.stacked ? avail : std::max(labelW, 0.0f);
            L.labelLine = Rect(x, labelY, x + avail, labelY + ts.y);
            L.labelWidth = std::min(ts.x, labelRoom);
            if (L.visible && labelRoom > 0.0f && !shown.empty())
                p.TextBox(Rect(x, labelY, x + labelRoom, labelY + ts.y), Vec2(0.0f, 0.0f), f, labelColor.a > 0 ? labelColor : LabelOr(pc.label), shown,
                          text::TextFlags_Ellipsis);
            if (L.stacked)
            {
                const float y2 = labelY + ts.y + lineGap;
                const float w2 = acc.flexible ? avail : std::min(acc.natural, avail);
                L.accessory = Rect(x, y2, x + w2, y2 + acc.height);
            }
            else
            {
                const float right = L.rect.max.x - pad;
                L.accessory = Rect(right - accW, L.rect.min.y, right, L.rect.max.y);
            }
            return L;
        }

        float BodyWidth(std::string_view text) { return text.empty() ? 0.0f : MeasureText(Font(TextStyle::Body), text).x; }

        // Right-aligned text inside the accessory (ellipsized to it).
        void RightText(const RowLayout& L, std::string_view text, Color color, float rightInset = 0.0f)
        {
            const Rect r(L.accessory.min.x, L.accessory.min.y, L.accessory.max.x - rightInset, L.accessory.max.y);
            if (r.Width() <= 1.0f || text.empty())
                return;
            Painter p = GetPainter();
            p.TextBox(r, Vec2(1.0f, 0.5f), Font(TextStyle::Body), color, text, text::TextFlags_Ellipsis);
        }

        // Footnote text inset like the rows' labels, wrapping inside that inset.
        void SectionNote(Id id, std::string_view text)
        {
            Context& c = Ctx();
            ContainerOptions co;
            co.fillWidth = true;
            co.padding = Vec2(Sc(16), 0.0f);
            c.BeginContainer(id, co);
            TextImpl(Font(TextStyle::Footnote), C().secondaryLabel, text, true);
            c.EndContainer();
        }

        StackLayout& Column(Id id)
        {
            StackLayout& l = Ctx().State<StackLayout>(id);
            l.horizontal = false;
            l.spacing = 0.0f;
            l.align = esia::Align::Start;
            return l;
        }
    }

    bool BeginSection(std::string_view header, std::string_view footer)
    {
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const bool stylePushed = TakeNextStyle();   // the section and everything in it, until EndSection
        // The section's id scope: its header. Sections without one, or with the same one, in one scope are told
        // apart by their order: their rows' ids and the section's own state (its card) stay their own.
        const std::string_view scopeLabel = header.empty() ? std::string_view("##section") : header;
        const Id scope = c.GetId(scopeLabel);
        int seen = 0;
        auto it = std::find_if(m.sectionScopes.begin(), m.sectionScopes.end(), [scope](const std::pair<Id, int>& e) { return e.first == scope; });
        if (it != m.sectionScopes.end())
            seen = ++it->second;
        else
            m.sectionScopes.emplace_back(scope, 0);
        c.PushId(scopeLabel);
        if (seen > 0)
            c.PushId(seen);
        const Id id = c.GetId("##sec");

        // header + card + footer: one item (one child inside auto-layout containers), as wide as it is offered
        MarkFill();
        ContainerOptions outer;
        outer.layout = &Column(Salt(id, 1));
        outer.fillWidth = true;
        c.BeginContainer(Salt(id, 1), outer);
        if (!header.empty())
        {
            SectionNote(Salt(id, 2), header);
            // WGT's gap: a spacer of 2 between two item spacings
            c.ItemSize(Vec2(0.0f, Sc(2) + 2.0f * c.Metrics().itemSpacing.y));
        }

        Section s;
        s.id = id;
        // the footer is copied (it may be a temporary) into a string kept per depth: no allocation once it is long enough
        if (m.sectionFooters.size() <= m.sections.size())
            m.sectionFooters.resize(m.sections.size() + 1);
        m.sectionFooters[m.sections.size()].assign(footer);
        s.stylePushed = stylePushed;
        s.counted = seen > 0;
        s.mark = c.WindowDrawList().Mark();   // the card goes under the rows, drawn at EndSection
        ContainerOptions card;
        card.layout = &Column(Salt(id, 3));
        card.fillWidth = true;
        c.BeginContainer(Salt(id, 3), card);
        const Vec2 pos = c.CursorPos();
        s.x0 = pos.x;
        s.x1 = pos.x + c.ContentRegionAvail().x;
        const SectionPersist& persist = c.State<SectionPersist>(id);
        s.mask = Rect(s.x0, pos.y, s.x1, pos.y + (persist.valid ? persist.lastCard.Height() : 100000.0f));
        m.sections.push_back(std::move(s));
        return true;
    }

    void EndSection()
    {
        Ui::Impl& m = M();
        ESIA_ASSERT(!m.sections.empty() && "EndSection without BeginSection");
        if (m.sections.empty())
            return;
        const Section s = m.sections.back();
        m.sections.pop_back();
        const std::string& footer = m.sectionFooters[m.sections.size()];
        Context& c = *m.ctx;
        const Theme& t = T();
        const Rect card = c.EndContainer();

        if (s.rows > 0)
        {
            DrawList& dl = c.WindowDrawList();
            const std::size_t from = dl.Mark();
            {
                Painter p = GetPainter();
                Style cs = Style().Radius(ItemRadius(Sc(t.metrics.cardRadius)));
                p.Rect(card, SurfaceFill(cs, FillOr(t.colors.cardSurface)));
            }
            MoveCommands(dl, from, s.mark);
        }
        SectionPersist& persist = c.State<SectionPersist>(s.id);
        persist.lastCard = card;
        persist.valid = true;

        const float spacing = c.Metrics().itemSpacing.y;
        if (!footer.empty())
        {
            c.ItemSize(Vec2(0.0f, Sc(6) + spacing));
            SectionNote(Salt(s.id, 4), footer);
            c.ItemSize(Vec2(0.0f, spacing));
        }
        c.ItemSize(Vec2(0.0f, Sc(t.metrics.sectionSpacing)));
        c.EndContainer();
        if (s.counted)
            c.PopId();
        c.PopId();
        if (s.stylePushed)
            PopStyle();
    }

    bool RowNavigation(std::string_view label, std::string_view detail, RowIcon icon)
    {
        ItemScope scope;
        const float chevronW = Sc(12);
        const float detailW = BodyWidth(detail);
        Accessory acc;
        acc.natural = chevronW + (detailW > 0.0f ? detailW + Sc(8) : 0.0f);
        acc.min = chevronW + (detailW > 0.0f ? std::min(detailW, Sc(48)) + Sc(8) : 0.0f);
        acc.height = Sc(22);
        const RowLayout L = RowStart(label, icon, acc, 0.0f, true);
        if (!L.visible)
            return false;
        const Palette& pc = C();
        Painter p = GetPainter();
        const float cy = L.stacked ? L.accessory.Center().y : L.rect.Center().y;
        DrawIcon(p, Vec2(L.accessory.max.x - chevronW * 0.5f, cy), icons::ChevronRight, Sc(11), pc.tertiaryLabel);
        if (detailW > 0.0f)
            RightText(L, detail, pc.secondaryLabel, chevronW + Sc(8));
        return L.it.pressed;
    }

    bool RowToggle(std::string_view label, bool* value, RowIcon icon)
    {
        ItemScope scope;
        const Vec2 sz = ToggleSize();
        Accessory acc;
        acc.natural = acc.min = sz.x;
        acc.height = sz.y;
        const RowLayout L = RowStart(label, icon, acc, 0.0f, false);
        if (!L.visible)
            return false;
        const Rect r = Rect::FromSize(Vec2(L.accessory.max.x - sz.x, L.accessory.Center().y - sz.y * 0.5f), sz);
        return ToggleAt(Salt(L.id, 1), r, value);
    }

    bool RowSlider(std::string_view label, float* value, float mn, float mx, RowIcon icon, const char* format)
    {
        ItemScope scope;
        // The slider can only show its range: a value outside it sits at the nearest end, and the number reads the
        // same (the data is left alone until the slider is dragged). The value column fits the widest value of the
        // range, so the slider keeps its length while the value changes and the sliders of a section line up.
        const float lo = std::min(mn, mx), hi = std::max(mn, mx);
        float valueW = 0.0f;
        if (format && *format)
        {
            char a[64], b[64];
            std::snprintf(a, sizeof(a), format, (double)mn);
            std::snprintf(b, sizeof(b), format, (double)mx);
            valueW = std::max({BodyWidth(a), BodyWidth(b), Sc(34)});
        }
        const float minSlider = Sc(96), gap = Sc(12);
        Accessory acc;
        acc.flexible = true;
        acc.min = minSlider + (valueW > 0.0f ? valueW + gap : 0.0f);
        acc.natural = acc.min;
        acc.height = Sc(32);
        const RowLayout L = RowStart(label, icon, acc, 0.0f, false);
        if (!L.visible)
            return false;
        // the slider keeps its minimum length inside the row; the value takes what is left. In a stacked row without
        // room for it beside the slider, the value goes up on the label's line and the slider gets the whole line
        float slotW = valueW > 0.0f ? std::min(valueW, std::max(0.0f, L.accessory.Width() - minSlider - gap)) : 0.0f;
        const bool valueUp = L.stacked && valueW > 0.0f && slotW < valueW - 0.5f;
        if (valueUp)
            slotW = 0.0f;
        const float right = std::max(L.accessory.max.x - (slotW > 0.0f ? slotW + gap : 0.0f), L.accessory.min.x);
        const Rect sr(L.accessory.min.x, L.accessory.Center().y - Sc(16), right, L.accessory.Center().y + Sc(16));
        const bool changed = SliderAt(Salt(L.id, 2), sr, value, mn, mx, SliderOptions{});
        Rect valueRect;
        if (valueUp)
            valueRect = Rect(std::min(L.labelLine.min.x + L.labelWidth + gap, L.labelLine.max.x), L.labelLine.min.y, L.labelLine.max.x, L.labelLine.max.y);
        else if (slotW > 0.0f)
            valueRect = Rect(L.accessory.max.x - slotW, L.accessory.min.y, L.accessory.max.x, L.accessory.max.y);
        if (valueRect.Width() > 1.0f)
        {
            char buf[64];
            std::snprintf(buf, sizeof(buf), format, (double)Clamp(*value, lo, hi));
            Painter p = GetPainter();
            p.TextBox(valueRect, Vec2(1.0f, 0.5f), Font(TextStyle::Body), LabelOr(C().secondaryLabel), buf, text::TextFlags_Ellipsis);
        }
        return changed;
    }

    bool RowStepper(std::string_view label, int* value, int mn, int mx, RowIcon icon)
    {
        ItemScope scope;
        const Vec2 sz = StepperSize();
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%d", *value);
        Accessory acc;
        acc.natural = acc.min = sz.x + Sc(14) + std::max(BodyWidth(buf), Sc(20));
        acc.height = sz.y;
        const RowLayout L = RowStart(label, icon, acc, 0.0f, false);
        if (!L.visible)
            return false;
        const Rect r = Rect::FromSize(Vec2(L.accessory.max.x - sz.x, L.accessory.Center().y - sz.y * 0.5f), sz);
        const bool changed = StepperAt(Salt(L.id, 3), r, value, mn, mx, 1);
        std::snprintf(buf, sizeof(buf), "%d", *value);
        RightText(L, buf, C().secondaryLabel, sz.x + Sc(14));
        return changed;
    }

    bool RowSegmented(std::string_view label, int* selected, std::span<const std::string_view> items, RowIcon icon)
    {
        ItemScope scope;
        float widest = 0.0f;
        for (const std::string_view item : items)
            widest = std::max(widest, MeasureText(Font(FontWeight::Semibold, 13.5f), VisibleLabel(item)).x);
        const float count = (float)items.size();
        Accessory acc;
        acc.min = count * (widest + Sc(20));   // every label still fits its segment
        acc.natural = std::max(acc.min, count * Sc(74));
        acc.height = Sc(30);
        const RowLayout L = RowStart(label, icon, acc, 0.0f, false);
        if (!L.visible)
            return false;
        const Rect r(L.accessory.min.x, L.accessory.Center().y - Sc(15), L.accessory.max.x, L.accessory.Center().y + Sc(15));
        return SegmentedAt(Salt(L.id, 4), r, selected, items);
    }

    bool RowPicker(std::string_view label, int* selected, std::span<const std::string_view> items, RowIcon icon)
    {
        ItemScope scope;
        const int count = (int)items.size();
        const int sel = count > 0 ? std::clamp(*selected, 0, count - 1) : 0;
        Accessory acc;
        acc.natural = (count > 0 ? BodyWidth(VisibleLabel(items[(std::size_t)sel])) : 0.0f) + Sc(30);
        acc.min = std::min(acc.natural, Sc(80));
        acc.height = Sc(30);
        const RowLayout L = RowStart(label, icon, acc, 0.0f, false);
        if (!L.visible)
            return false;
        const Rect r(L.accessory.min.x, L.accessory.Center().y - Sc(15), L.accessory.max.x, L.accessory.Center().y + Sc(15));
        return PickerAt(Salt(L.id, 5), r, selected, items, true);   // at most 10 choices in view
    }

    void RowValue(std::string_view label, std::string_view value, RowIcon icon)
    {
        ItemScope scope;
        Accessory acc;
        acc.natural = BodyWidth(value);
        acc.min = std::min(acc.natural, Sc(56));
        acc.height = Sc(22);
        const RowLayout L = RowStart(label, icon, acc, 0.0f, false);
        if (L.visible)
            RightText(L, value, C().secondaryLabel);
    }

    bool RowButton(std::string_view label, bool destructive, RowIcon icon)
    {
        ItemScope scope;
        const RowLayout L = RowStart(label, icon, Accessory{}, 0.0f, true, destructive ? AccentOr(C().red) : Accent());
        return L.visible && L.it.pressed;
    }

    bool BeginRow(std::string_view id, float height, Rect* outContent)
    {
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        m.rowStyles.push_back(TakeNextStyle() ? 1 : 0);   // the row and everything in it, until EndRow
        const RowLayout L = RowStart({}, RowIcon{}, Accessory{}, height, false, Color::Clear(), id, false);
        const float pad = Sc(16);
        if (outContent)
            *outContent = Rect(L.rect.min.x + pad, L.rect.min.y, L.rect.max.x - pad, L.rect.max.y);
        // the content goes in a container of the row's size: widgets that fill the available width (sliders, fields,
        // width < 0 buttons ...) stay inside the row, and the section takes the row's height whatever it holds
        ContainerOptions co;
        co.size = L.rect.Size();
        co.padding = Vec2(pad, 0.0f);
        c.BeginContainer(Salt(L.id, 7), co);
        c.PushId(id);
        return L.visible;
    }

    void EndRow()
    {
        Ui::Impl& m = M();
        ESIA_ASSERT(!m.rowStyles.empty() && "EndRow without BeginRow");
        Context& c = *m.ctx;
        c.PopId();
        c.EndContainer();
        if (!m.rowStyles.empty())
        {
            if (m.rowStyles.back())
                PopStyle();
            m.rowStyles.pop_back();
        }
    }
}
