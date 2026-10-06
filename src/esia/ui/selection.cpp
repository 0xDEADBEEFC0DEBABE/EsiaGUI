// Esia UI - the liquid selection of segmented controls and tab bars (iOS 26), and the segmented control (WGT's).
//
// Tap an item and the selection travels there; press on it and drag and it follows the pointer, then lands on the
// nearest item. While it is held or travelling it is a clear lens that magnifies what it passes over; it swells out
// of the resting pill with a pop and melts back into it where it lands. Position and lens are springs, so every
// transition (press, drag, release, retarget mid-flight) is continuous.
#include "ui_internal.hpp"
#include <algorithm>

namespace esia::ui
{
    using namespace detail;

    namespace
    {
        struct SelectState
        {
            SpringState pos;          // item units
            SpringState lens;         // 0 = resting pill .. 1 = lens
            float target = 0.0f;      // where it is heading (fractional while dragged)
            float grab = 0.0f;        // pointer offset from the selection's center at press (item units)
            bool dragging = false;
            bool init = false;
        };
    }

    LiquidSelection detail::LiquidSelect(Id id, const Rect& area, int count, int* selected)
    {
        LiquidSelection out;
        if (count <= 0 || !selected)
            return out;
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        SelectState& s = c.State<SelectState>(id);
        const int sel = std::clamp(*selected, 0, count - 1);
        const float itemW = area.Width() / (float)count;
        if (!s.init)
        {
            s.pos.value = s.target = (float)sel;
            s.init = true;
        }

        const InteractState it = InteractImpl(Salt(id, 0x5E1), area, InteractFlags_PressOnClick);
        const float mx = c.Input().MousePos().x;
        const float under = Clamp((mx - area.min.x) / itemW - 0.5f, 0.0f, (float)(count - 1));   // pointer, item units
        if (it.hovered || it.held)
            out.hovered = std::clamp((int)std::floor((mx - area.min.x) / itemW), 0, count - 1);
        if (it.pressed)
        {
            // grabbing the selection keeps the grab point under the pointer; pressed elsewhere it comes over
            s.grab = std::fabs(under - s.pos.value) < 0.5f ? s.pos.value - under : 0.0f;
            s.dragging = true;
        }
        if (s.dragging && it.held)
            s.target = Clamp(under + s.grab, 0.0f, (float)(count - 1));
        else if (s.dragging)
        {
            // released: land on the nearest item
            s.dragging = false;
            const int landed = std::clamp((int)std::floor(s.target + 0.5f), 0, count - 1);
            if (landed != *selected)
            {
                *selected = landed;
                out.changed = true;
            }
            s.target = (float)landed;
        }
        else
            s.target = (float)sel;   // set from code

        static constexpr Spring kFollow{0.14f, 0.95f};   // under the pointer, a hair behind it
        static constexpr Spring kLand{0.42f, 0.68f};     // lands with a liquid overshoot
        s.pos.Step(s.target, s.dragging ? kFollow : kLand, m.dt);
        const bool travelling = std::fabs(s.pos.value - s.target) > 0.03f || std::fabs(s.pos.velocity) > 0.5f;
        const bool lensOn = s.dragging || travelling;
        static constexpr Spring kSwell{0.24f, 0.58f};    // pops out of the pill
        static constexpr Spring kSettle{0.34f, 0.82f};   // melts back into it
        s.lens.Step(lensOn ? 1.0f : 0.0f, lensOn ? kSwell : kSettle, m.dt);
        if (!s.pos.Settled(s.target) || !s.lens.Settled(lensOn ? 1.0f : 0.0f))
            m.animating = true;

        out.pos = s.pos.value;
        out.velocity = s.pos.velocity * itemW;
        out.lens = std::max(s.lens.value, 0.0f);
        out.held = s.dragging;
        return out;
    }

    Rect detail::LiquidSelectionRect(const LiquidSelection& s, const Rect& area, int count, float inset)
    {
        const float itemW = area.Width() / (float)std::max(count, 1);
        // liquid: stretches along its motion and thins a little across it
        const float stretch = std::min(std::fabs(s.velocity) * 0.035f, itemW * 0.45f);
        const float x = area.min.x + s.pos * itemW;
        Rect r(x + inset - stretch * 0.5f, area.min.y + inset, x + itemW - inset + stretch * 0.5f, area.max.y - inset);
        r = r.Expanded(0.0f, -std::min(stretch * 0.06f, inset));
        return ContainLiquid(r, area, inset * 0.6f);
    }

    void detail::DrawSelectionLens(Painter& p, const Rect& pill, const LiquidSelection& s, float rise, Color tint)
    {
        const float k = Saturate(s.lens);
        if (k <= 0.01f)
            return;
        // grows out of the pill: wider, and taller than the track; the swell spring overshoots (a pop)
        const Rect lr = pill.Expanded(Sc(5) * s.lens, rise * s.lens);
        GlassMaterial mat = LensMaterial();
        mat.magnify *= k;
        mat.refraction *= k;
        mat.dispersion *= k;
        ScopedUnclip unclip(lr, ShadowExtent(Sc(14), Vec2(0, Sc(4))));
        Style ls = Style().Radius(lr.Height() * 0.5f).Glass(mat).Shadow(Color::Black(0.20f * k), Sc(14), Vec2(0, Sc(4))).Opacity(SmoothStep(0.0f, 0.35f, k));
        if (tint.a > 0.0f)
            ls.Fill(tint);   // ItemStyle::MovingFill: the lens carries a color while it travels
        p.Rect(lr, ls);
    }

    // ============================================================= segmented
    bool detail::SegmentedAt(Id id, const Rect& r, int* selected, std::span<const std::string_view> items)
    {
        const int count = (int)items.size();
        if (count <= 0)
            return false;
        const Palette& c = C();
        const Theme& t = T();
        Painter p = GetPainter();
        const float trackR = std::min(ItemRadius(r.Height() * 0.5f), r.Height() * 0.5f);
        DrawPill(p, r, Surface(FillOr(c.tertiaryFill)));
        const float segW = r.Width() / (float)count;
        // tap a segment or drag the selection: it travels as a clear lens and lands as a solid pill
        const LiquidSelection sel = LiquidSelect(id, r, count, selected);
        const Rect pill = LiquidSelectionRect(sel, r, count, Dt(2.5f));
        const float solid = 1.0f - Saturate(sel.lens);
        // the selected segment's background: the style's, else the tint, else - on a custom track - a light,
        // see-through pill that sits on any color, else the iOS gray / white
        const ItemStyle& st = ResolvedStyle();
        Color selFill = t.dark ? Color::Hex(0x636366) : Color::White();
        if (st.Has(ItemStyle::kFill))
            selFill = Color::White(t.dark ? 0.26f : 0.90f);
        if (st.Has(ItemStyle::kTint))
            selFill = st.tint;
        if (st.Has(ItemStyle::kSelectedFill))
            selFill = st.selectedFill;
        const Color labelColor = LabelOr(c.label);
        const Color selLabel = st.Has(ItemStyle::kSelectedLabel) ? st.selectedLabel : labelColor;
        if (solid > 0.01f)
        {
            Style ps = Style().Radius(std::min(pill.Height() * 0.5f, std::max(trackR - (pill.min.y - r.min.y), 0.0f))).Fill(StateFill(selFill).Fade(solid));
            if (!LookClear())
                ps.Shadow(Color::Black((t.dark ? 0.3f : 0.12f) * solid), Dt(8), Vec2(0, Dt(2)));
            p.Rect(pill, ps);
        }

        // labels follow the pill, not the index: a segment turns semibold as the pill slides over it
        for (int i = 0; i < count; ++i)
        {
            const Rect sr(r.min.x + segW * (float)i, r.min.y, r.min.x + segW * (float)(i + 1), r.max.y);
            const float cover = Saturate(1.0f - std::fabs(sel.pos - (float)i));
            const std::string_view shown = VisibleLabel(items[(std::size_t)i]);
            auto label = [&](FontWeight weight, float alpha) {
                if (alpha <= 0.01f)
                    return;
                const text::FontRef f = Font(weight, 13.5f);
                const Vec2 ts = MeasureText(f, shown);
                // the selected color once the pill lands
                p.Text(Vec2(std::floor(sr.Center().x - ts.x * 0.5f + 0.5f), std::floor(sr.Center().y - ts.y * 0.5f + 0.5f)), f,
                       Lerp(labelColor, selLabel, cover * solid).Fade(alpha), shown);
            };
            const float bold = SmoothStep(0.25f, 0.75f, cover);
            label(FontWeight::Regular, 1.0f - bold);
            label(FontWeight::Semibold, bold);
        }
        DrawSelectionLens(p, pill, sel, Dt(4), st.Has(ItemStyle::kMovingFill) ? st.movingFill : Color::Clear());
        return sel.changed;
    }

    bool Segmented(std::string_view id, int* selected, std::span<const std::string_view> items, float width)
    {
        ItemScope scope;
        Context& c = Ctx();
        const float w = width > 0.0f ? Sc(width) : AvailableWidth();
        const Vec2 size(w, Sc(Sizes().field));
        const Vec2 pos = c.CursorPos();
        c.ItemSize(size);
        return SegmentedAt(c.GetId(id), Rect::FromSize(pos, size), selected, items);
    }
}
