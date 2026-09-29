// WGT UI - inset grouped lists (iOS Settings style): sections and rows.
#include "ui/ui_internal.hpp"

namespace wgt::ui
{
    using namespace detail;

    namespace
    {
        struct SectionPersist
        {
            Rect lastCard;
            bool valid = false;
        };

        struct RowBottom
        {
            float y = 0.0f;
        };

        struct RowLayout
        {
            ImGuiID id = 0;
            Rect rect;
            Rect accessory;      // where the accessory goes (never overlaps the label)
            Rect labelLine;      // the label's line, and the label's own width on it
            float labelWidth = 0.0f;
            bool stacked = false;
            bool visible = false;
            Interaction it;
        };

        // What a row's accessory needs. The label gets whatever is left; when even a short label and the
        // accessory's minimum do not fit side by side, the row stacks them (label on top, control below).
        struct Accessory
        {
            float natural = 0.0f;   // preferred width
            float min = 0.0f;       // narrowest usable width
            float height = 0.0f;    // height of the control (stacked layout)
            bool flexible = false;  // takes all the room the label leaves (sliders)
        };

        SectionFrame* CurrentSection()
        {
            Context::Impl& impl = Ctx();
            IM_ASSERT(!impl.ui.sections.empty() && "Row outside BeginSection()/EndSection()");
            return impl.ui.sections.empty() ? nullptr : impl.ui.sections.back().get();
        }

        RowLayout RowStart(const char* label, RowIcon icon, const Accessory& acc, float height, bool interactive, Color labelColor = Color::Clear(),
                           const char* idOverride = nullptr)
        {
            RowLayout L;
            ImGuiWindow* w = ImGui::GetCurrentWindow();
            SectionFrame* s = CurrentSection();
            if (!s || w->SkipItems)
                return L;
            const Theme& t = T();
            const Palette& c = t.colors;
            const float pad = Sc(16), gap = Sc(12);
            const float iconW = icon.icon ? Sc(t.metrics.iconTile) + Sc(12) : 0.0f;
            const FontRef f = GetFont(TextStyle::Body);
            const char* end = ImGui::FindRenderedTextEnd(label);
            const Vec2 ts = Painter::MeasureText(f, label, end);

            // ---- measure: label vs accessory
            const float rowW = s->x1 - s->x0;
            const float avail = std::max(0.0f, rowW - pad * 2.0f - iconW);
            const float labelMin = std::min(ts.x, Sc(72));
            float accW = 0.0f, labelW = ts.x;
            if (acc.natural > 0.0f)
            {
                if (acc.flexible)
                {
                    // flexible controls (sliders) start on a shared column so a section lines up, but never
                    // before the end of their label
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
            const Vec2 pos(s->x0, w->DC.CursorPos.y);
            ImGui::SetCursorScreenPos(pos);
            ImGui::ItemSize(Vec2(rowW, h));
            L.rect = Rect(s->x0, pos.y, s->x1, pos.y + h);
            L.id = w->GetID(idOverride ? idOverride : label);

            Painter p(s->drawList);
            if (s->rows > 0)
                p.HLine(s->x0 + pad + iconW, s->x1, L.rect.min.y, c.separator, std::max(1.0f, Sc(t.metrics.hairline) * 0.75f));
            s->rows++;

            if (interactive)
            {
                L.it = InteractImpl(L.id, L.rect, InteractFlags_AllowOverlap);
                L.visible = L.it.visible;
                const float hl = std::max(L.it.hover * 0.55f, L.it.press);
                if (hl > 0.01f)
                {
                    p.PushMask(s->maskRect, ItemRadius(Sc(t.metrics.cardRadius)));
                    p.Rect(L.rect, Style().Fill(c.highlight.Fade(hl * 1.3f)));
                    p.PopMask();
                }
            }
            else
                L.visible = ImGui::ItemAdd(ImRect(L.rect.min, L.rect.max), 0);

            // ---- place
            float x = L.rect.min.x + pad;
            const float labelY = L.stacked ? L.rect.min.y + vPad : std::floor(L.rect.Center().y - ts.y * 0.5f + 0.5f);
            if (icon.icon)
            {
                const float tile = Sc(t.metrics.iconTile);
                const float iconCy = L.stacked ? labelY + ts.y * 0.5f : L.rect.Center().y;
                IconTile(p, Rect::FromCenter(Vec2(x + tile * 0.5f, iconCy), Vec2(tile, tile)), icon.icon, icon.color);
                x += iconW;
            }
            const float labelRoom = L.stacked ? avail : std::max(labelW, 0.0f);
            L.labelLine = Rect(x, labelY, x + avail, labelY + ts.y);
            L.labelWidth = std::min(ts.x, labelRoom);
            if (labelRoom > 0.0f)
                p.TextBox(Rect(x, labelY, x + labelRoom, labelY + ts.y), Vec2(0.0f, 0.0f), f, labelColor.a > 0 ? labelColor : LabelOr(c.label), label, end, TextFlags_Ellipsis);
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

        float BodyWidth(const char* text) { return text && *text ? Painter::MeasureText(GetFont(TextStyle::Body), text).x : 0.0f; }

        // Right-aligned text inside the accessory (ellipsized to it).
        void RightText(const RowLayout& L, const char* text, Color color, float rightInset = 0.0f)
        {
            const FontRef f = GetFont(TextStyle::Body);
            const Rect r(L.accessory.min.x, L.accessory.min.y, L.accessory.max.x - rightInset, L.accessory.max.y);
            if (r.Width() <= 1.0f)
                return;
            Painter p(CurrentSection()->drawList);
            p.TextBox(r, Vec2(1.0f, 0.5f), f, color, text, nullptr, TextFlags_Ellipsis);
        }
    }

    bool BeginSection(const char* header, const char* footer)
    {
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        const bool look = TakeNextStyle();   // the section and everything in it, until EndSection
        if (w->SkipItems)
        {
            if (look)
                PopGlassLook();
            return false;
        }
        Context::Impl& impl = Ctx();
        const Theme& t = T();
        ImGui::PushID(header ? header : "##section");
        ImGui::BeginGroup();   // header + card + footer = one item (one child inside auto-layout containers)
        const ImGuiID id = ImGui::GetID("##sec");

        if (header && *header)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + Sc(16));
            TextImpl(GetFont(TextStyle::Footnote), t.colors.secondaryLabel, header, nullptr, false);
            ImGui::Dummy(Vec2(0, Sc(2)));
        }

        auto frame = std::make_unique<SectionFrame>();
        frame->id = id;
        frame->drawList = w->DrawList;
        frame->x0 = w->DC.CursorPos.x;
        frame->x1 = frame->x0 + AvailWidth();
        frame->y0 = w->DC.CursorPos.y;
        frame->footer = footer ? footer : "";
        const SectionPersist& persist = impl.state.Get<SectionPersist>(id);
        frame->maskRect = persist.valid ? Rect(frame->x0, frame->y0, frame->x1, frame->y0 + persist.lastCard.Height())
                                        : Rect(frame->x0, frame->y0, frame->x1, frame->y0 + 100000.0f);
        frame->splitter.Split(w->DrawList, 2);
        frame->splitter.SetCurrentChannel(w->DrawList, 1);
        frame->lookPushed = look;
        impl.ui.sections.push_back(std::move(frame));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Vec2(ImGui::GetStyle().ItemSpacing.x, 0.0f));
        return true;
    }

    void EndSection()
    {
        Context::Impl& impl = Ctx();
        IM_ASSERT(!impl.ui.sections.empty());
        std::unique_ptr<SectionFrame> s = std::move(impl.ui.sections.back());
        impl.ui.sections.pop_back();
        ImGui::PopStyleVar();
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        const Theme& t = T();
        const float y1 = w->DC.CursorPos.y;
        const Rect card(s->x0, s->y0, s->x1, std::max(y1, s->y0));

        s->splitter.SetCurrentChannel(s->drawList, 0);
        if (s->rows > 0)
        {
            Painter p(s->drawList);
            Style cs = Style().Radius(ItemRadius(Sc(t.metrics.cardRadius)));
            p.Rect(card, SurfaceFill(cs, FillOr(t.colors.cardSurface)));
        }
        s->splitter.Merge(s->drawList);
        if (s->lookPushed)
            PopGlassLook();

        SectionPersist& persist = impl.state.Get<SectionPersist>(s->id);
        persist.lastCard = card;
        persist.valid = true;

        if (!s->footer.empty())
        {
            ImGui::Dummy(Vec2(0, Sc(6)));
            const float x = ImGui::GetCursorPosX();
            ImGui::SetCursorPosX(x + Sc(16));
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + (s->x1 - s->x0) - Sc(32));
            TextImpl(GetFont(TextStyle::Footnote), t.colors.secondaryLabel, s->footer.c_str(), nullptr, false);
            ImGui::PopTextWrapPos();
        }
        ImGui::Dummy(Vec2(0, Sc(t.metrics.sectionSpacing)));
        ImGui::EndGroup();
        ImGui::PopID();
    }

    bool RowNavigation(const char* label, const char* detail, RowIcon icon)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        const float chevronW = Sc(12);
        const float detailW = BodyWidth(detail);
        Accessory acc;
        acc.natural = chevronW + (detailW > 0.0f ? detailW + Sc(8) : 0.0f);
        acc.min = chevronW + (detailW > 0.0f ? std::min(detailW, Sc(48)) + Sc(8) : 0.0f);
        acc.height = Sc(22);
        RowLayout L = RowStart(label, icon, acc, 0.0f, true);
        if (!L.visible)
            return false;
        const Palette& c = C();
        Painter p(CurrentSection()->drawList);
        const float cy = L.stacked ? L.accessory.Center().y : L.rect.Center().y;
        p.Icon(Vec2(L.accessory.max.x - chevronW * 0.5f, cy), icons::ChevronRight, Sc(11), c.tertiaryLabel);
        if (detailW > 0.0f)
            RightText(L, detail, c.secondaryLabel, chevronW + Sc(8));
        return L.it.pressed;
    }

    bool RowToggle(const char* label, bool* value, RowIcon icon)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        const Vec2 sz = ToggleSize();
        Accessory acc;
        acc.natural = acc.min = sz.x;
        acc.height = sz.y;
        RowLayout L = RowStart(label, icon, acc, 0.0f, false);
        if (!L.visible)
            return false;
        const Rect r = Rect::FromSize(Vec2(L.accessory.max.x - sz.x, L.accessory.Center().y - sz.y * 0.5f), sz);
        return ToggleAt(Salt(L.id, 1), r, value);
    }

    bool RowSlider(const char* label, float* value, float mn, float mx, RowIcon icon, const char* format)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        // The slider can only show its range: a value outside it sits at the nearest end, and the number reads
        // the same (the data is left alone until the slider is dragged). The value column fits the widest value
        // of the range, so the slider keeps its length while the value changes and sliders of a section line up.
        const float lo = std::min(mn, mx), hi = std::max(mn, mx);
        char buf[64] = {};
        float valueW = 0.0f;
        if (format && *format)
        {
            char a[64], b[64];
            std::snprintf(a, sizeof(a), format, mn);
            std::snprintf(b, sizeof(b), format, mx);
            valueW = std::max({BodyWidth(a), BodyWidth(b), Sc(34)});
        }
        const float minSlider = Sc(96), gap = Sc(12);
        Accessory acc;
        acc.flexible = true;
        acc.min = minSlider + (valueW > 0.0f ? valueW + gap : 0.0f);
        acc.natural = acc.min;
        acc.height = Sc(32);
        RowLayout L = RowStart(label, icon, acc, 0.0f, false);
        if (!L.visible)
            return false;
        // the slider keeps its minimum length inside the row; the value takes what is left. In a stacked row
        // without room for it beside the slider, the value goes up on the label's line and the slider gets the
        // whole line (it is ellipsized only if even that is too narrow)
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
            std::snprintf(buf, sizeof(buf), format, Clamp(*value, lo, hi));
            Painter p(CurrentSection()->drawList);
            p.TextBox(valueRect, Vec2(1.0f, 0.5f), GetFont(TextStyle::Body), LabelOr(C().secondaryLabel), buf, nullptr, TextFlags_Ellipsis);
        }
        return changed;
    }

    bool RowStepper(const char* label, int* value, int mn, int mx, RowIcon icon)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        const Vec2 sz = StepperSize();
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%d", *value);
        Accessory acc;
        acc.natural = acc.min = sz.x + Sc(14) + std::max(BodyWidth(buf), Sc(20));
        acc.height = sz.y;
        RowLayout L = RowStart(label, icon, acc, 0.0f, false);
        if (!L.visible)
            return false;
        const Rect r = Rect::FromSize(Vec2(L.accessory.max.x - sz.x, L.accessory.Center().y - sz.y * 0.5f), sz);
        const bool changed = StepperAt(Salt(L.id, 3), r, value, mn, mx, 1);
        std::snprintf(buf, sizeof(buf), "%d", *value);
        RightText(L, buf, C().secondaryLabel, sz.x + Sc(14));
        return changed;
    }

    bool RowSegmented(const char* label, int* selected, const char* const* items, int count, RowIcon icon)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        float widest = 0.0f;
        for (int i = 0; i < count; ++i)
            widest = std::max(widest, Painter::MeasureText(Font(FontWeight::Semibold, 13.5f), items[i]).x);
        Accessory acc;
        acc.min = count * (widest + Sc(20));                     // every label still fits its segment
        acc.natural = std::max(acc.min, count * Sc(74));
        acc.height = Sc(30);
        RowLayout L = RowStart(label, icon, acc, 0.0f, false);
        if (!L.visible)
            return false;
        const Rect r(L.accessory.min.x, L.accessory.Center().y - Sc(15), L.accessory.max.x, L.accessory.Center().y + Sc(15));
        return SegmentedAt(Salt(L.id, 4), r, selected, items, count);
    }

    bool RowPicker(const char* label, int* selected, const char* const* items, int count, RowIcon icon)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        const int sel = count > 0 ? std::clamp(*selected, 0, count - 1) : 0;
        Accessory acc;
        acc.natural = (count > 0 ? BodyWidth(items[sel]) : 0.0f) + Sc(30);
        acc.min = std::min(acc.natural, Sc(80));
        acc.height = Sc(30);
        RowLayout L = RowStart(label, icon, acc, 0.0f, false);
        if (!L.visible)
            return false;
        const Rect r(L.accessory.min.x, L.accessory.Center().y - Sc(15), L.accessory.max.x, L.accessory.Center().y + Sc(15));
        return PickerAt(Salt(L.id, 5), r, selected, items, count, true);
    }

    void RowValue(const char* label, const char* value, RowIcon icon)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        Accessory acc;
        acc.natural = BodyWidth(value);
        acc.min = std::min(acc.natural, Sc(56));
        acc.height = Sc(22);
        RowLayout L = RowStart(label, icon, acc, 0.0f, false);
        if (!L.visible || !value)
            return;
        RightText(L, value, C().secondaryLabel);
    }

    bool RowButton(const char* label, bool destructive, RowIcon icon)
    {
        ItemScope itemScope;   // takes a ui::Next() style
        const Palette& c = C();
        RowLayout L = RowStart(label, icon, Accessory{}, 0.0f, true, destructive ? AccentOr(c.red) : Accent());
        return L.visible && L.it.pressed;
    }

    bool BeginRow(const char* id, float height, Rect* outContent)
    {
        Ctx().ui.rowLooks.push_back(TakeNextStyle() ? 1 : 0);   // the row and everything in it, until EndRow
        RowLayout L = RowStart("", RowIcon{}, Accessory{}, height, false, Color::Clear(), id);
        const float pad = Sc(16);
        const Rect content(L.rect.min.x + pad, L.rect.min.y, L.rect.max.x - pad, L.rect.max.y);
        if (outContent)
            *outContent = content;
        ImGui::PushID(id);
        Ctx().state.Get<RowBottom>(ImGui::GetID("##rowbottom")).y = L.rect.max.y;
        ImGui::SetCursorScreenPos(content.min);
        // widgets that fill the available width (sliders, fields, width = -1 buttons ...) stay inside the row
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        Ctx().ui.rowRegions.push_back(Vec2(w->WorkRect.Max.x, w->ContentRegionRect.Max.x));
        w->WorkRect.Max.x = std::min(w->WorkRect.Max.x, content.max.x);
        w->ContentRegionRect.Max.x = std::min(w->ContentRegionRect.Max.x, content.max.x);
        ImGui::BeginGroup();
        return L.visible;
    }

    void EndRow()
    {
        ImGui::EndGroup();
        std::vector<Vec2>& regions = Ctx().ui.rowRegions;
        if (!regions.empty())
        {
            ImGuiWindow* w = ImGui::GetCurrentWindow();
            w->WorkRect.Max.x = regions.back().x;
            w->ContentRegionRect.Max.x = regions.back().y;
            regions.pop_back();
        }
        std::vector<std::uint8_t>& looks = Ctx().ui.rowLooks;
        IM_ASSERT(!looks.empty() && "EndRow() without BeginRow()");
        if (!looks.empty())
        {
            if (looks.back())
                PopGlassLook();
            looks.pop_back();
        }
        const float y = Ctx().state.Get<RowBottom>(ImGui::GetID("##rowbottom")).y;
        ImGui::PopID();
        SectionFrame* s = CurrentSection();
        ImGui::SetCursorScreenPos(Vec2(s ? s->x0 : ImGui::GetCursorScreenPos().x, y));
    }
}
