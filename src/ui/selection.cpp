// WGT UI - liquid selection: the moving selection of segmented controls and tab bars (iOS 26+).
//
// Tap an item and the selection travels there; press on it and drag and it follows the pointer, then lands
// on the nearest item. While it is held or travelling it is a clear lens that magnifies what it passes over;
// it swells out of the resting pill with a pop and melts back into it where it lands. Position and lens are
// springs, so every transition (press, drag, release, retarget mid-flight) is continuous.
#include "ui/ui_internal.hpp"

namespace wgt::ui::detail
{
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

    LiquidSelection LiquidSelect(ImGuiID id, const Rect& area, int count, int* selected)
    {
        LiquidSelection out;
        if (count <= 0 || !selected)
            return out;
        Context::Impl& m = Ctx();
        SelectState& s = m.state.Get<SelectState>(id);
        const int sel = std::clamp(*selected, 0, count - 1);
        const float itemW = area.Width() / (float)count;
        if (!s.init)
        {
            s.pos.value = s.target = (float)sel;
            s.init = true;
        }

        const Interaction it = InteractImpl(Salt(id, 0x5E1), area, InteractFlags_PressOnClick);
        const float mx = ImGui::GetIO().MousePos.x;
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
            s.target = (float)sel;   // set from code / keyboard

        static const Spring kFollow{0.14f, 0.95f};   // under the pointer, a hair behind it
        static const Spring kLand{0.42f, 0.68f};     // lands with a liquid overshoot
        s.pos.Step(s.target, s.dragging ? kFollow : kLand, m.dt);
        const bool travelling = std::fabs(s.pos.value - s.target) > 0.03f || std::fabs(s.pos.velocity) > 0.5f;
        const bool lensOn = s.dragging || travelling;
        static const Spring kSwell{0.24f, 0.58f};    // pops out of the pill
        static const Spring kSettle{0.34f, 0.82f};   // melts back into it
        s.lens.Step(lensOn ? 1.0f : 0.0f, lensOn ? kSwell : kSettle, m.dt);

        out.pos = s.pos.value;
        out.velocity = s.pos.velocity * itemW;
        out.lens = std::max(s.lens.value, 0.0f);
        out.held = s.dragging;
        return out;
    }

    Rect LiquidSelectionRect(const LiquidSelection& s, const Rect& area, int count, float inset)
    {
        const float itemW = area.Width() / (float)std::max(count, 1);
        // liquid: stretches along its motion and thins a little across it
        const float stretch = std::min(std::fabs(s.velocity) * 0.035f, itemW * 0.45f);
        const float x = area.min.x + s.pos * itemW;
        Rect r(x + inset - stretch * 0.5f, area.min.y + inset, x + itemW - inset + stretch * 0.5f, area.max.y - inset);
        r = r.Expanded(0.0f, -std::min(stretch * 0.06f, inset));
        return ContainLiquid(r, area, inset * 0.6f);
    }

    void DrawSelectionLens(Painter& p, const Rect& pill, const LiquidSelection& s, float rise, Color tint)
    {
        const float k = Saturate(s.lens);
        if (k <= 0.01f)
            return;
        // grows out of the pill: wider, and taller than the track; the swell spring overshoots (a pop)
        const Rect lr = pill.Expanded(Sc(5) * s.lens, rise * s.lens);
        GlassMaterial m = LensMaterial();
        m.magnify *= k;
        m.refraction *= k;
        m.dispersion *= k;
        ScopedUnclip unclip(p.DrawList(), lr, ShadowExtent(Sc(14), Vec2(0, Sc(4))));
        Style ls = Style().Radius(lr.Height() * 0.5f).Glass(m).Shadow(Color::Black(0.20f * k), Sc(14), Vec2(0, Sc(4))).Opacity(SmoothStep(0.0f, 0.35f, k));
        if (tint.a > 0.0f)
            ls.Fill(tint);   // ItemStyle::MovingFill: the lens carries a color while it travels
        p.Rect(lr, ls);
    }
}
