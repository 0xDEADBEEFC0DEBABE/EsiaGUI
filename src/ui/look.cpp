// WGT UI - per-component style: any component (or everything in a scope) gets its own glass look and material,
// tint, fill, corner radius and opacity, without touching the theme. Only the fields that are set change.
#include "ui/ui_internal.hpp"

namespace wgt
{
    void Context::SetGlassLook(GlassLook look) { impl_->glassLook = (int)look; }
    GlassLook Context::GetGlassLook() const { return (GlassLook)impl_->glassLook.load(); }
}

namespace wgt::ui
{
    using namespace detail;

    ItemStyle& Next() { return Ctx().ui.nextStyle; }
    void SetNextItemStyle(const ItemStyle& style) { Ctx().ui.nextStyle = style; }
    void SetNextItemLook(GlassLook look) { Next().Look(look); }

    void PushItemStyle(const ItemStyle& style)
    {
        Context::Impl& m = Ctx();
        UiStacks::StyleEntry e;
        e.style = style;
        e.resolved = m.ui.itemStyles.empty() ? ItemStyle() : m.ui.itemStyles.back().resolved;
        MergeStyle(e.resolved, style);
        if (style.Has(ItemStyle::kOpacity) && ImGui::GetCurrentContext())
        {
            // everything a component draws goes through the style alpha (Painter, text, stock ImGui widgets)
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * Saturate(style.opacity));
            e.alpha = true;
        }
        m.ui.itemStyles.push_back(e);
    }

    void PopItemStyle()
    {
        Context::Impl& m = Ctx();
        IM_ASSERT(!m.ui.itemStyles.empty() && "PopItemStyle() without PushItemStyle()");
        if (m.ui.itemStyles.empty())
            return;
        if (m.ui.itemStyles.back().alpha)
            ImGui::PopStyleVar();
        m.ui.itemStyles.pop_back();
    }

    void PushGlassLook(GlassLook look) { PushItemStyle(ItemStyle().Look(look)); }
    void PopGlassLook() { PopItemStyle(); }

    ItemStyle CurrentItemStyle() { return ResolvedStyle(); }

    GlassLook CurrentGlassLook()
    {
        const ItemStyle& s = ResolvedStyle();
        return s.Has(ItemStyle::kLook) ? s.look : (GlassLook)Ctx().glassLook.load();
    }

    GlassMaterial LookMaterial(const GlassMaterial& themed) { return StyledMaterial(themed); }
    Color AccentColor() { return Accent(); }
}

namespace wgt::ui::detail
{
    namespace
    {
        // `s` over `into`, field by field
        void Merge(ItemStyle& into, const ItemStyle& s)
        {
            const std::uint32_t f = s.set;
            if (f & ItemStyle::kLook) into.look = s.look;
            if (f & ItemStyle::kBlur) into.glass.blur = s.glass.blur;
            if (f & ItemStyle::kRefraction) into.glass.refraction = s.glass.refraction;
            if (f & ItemStyle::kBezel) into.glass.bezel = s.glass.bezel;
            if (f & ItemStyle::kDispersion) into.glass.dispersion = s.glass.dispersion;
            if (f & ItemStyle::kSaturation) into.glass.saturation = s.glass.saturation;
            if (f & ItemStyle::kBrightness) into.glass.brightness = s.glass.brightness;
            if (f & ItemStyle::kSpecular) into.glass.specular = s.glass.specular;
            if (f & ItemStyle::kLegibility) into.glass.legibility = s.glass.legibility;
            if (f & ItemStyle::kMagnify) into.glass.magnify = s.glass.magnify;
            if (f & ItemStyle::kGlassTint) into.glass.tint = s.glass.tint;
            if (f & ItemStyle::kRim) into.glass.rim = s.glass.rim;
            if (f & ItemStyle::kTint) into.tint = s.tint;
            if (f & ItemStyle::kFill) into.fill = s.fill;
            if (f & ItemStyle::kRadius) into.radius = s.radius;
            if (f & ItemStyle::kOpacity) into.opacity = s.opacity;
            if (f & ItemStyle::kLabel) into.label = s.label;
            if (f & ItemStyle::kSelectedFill) into.selectedFill = s.selectedFill;
            if (f & ItemStyle::kMovingFill) into.movingFill = s.movingFill;
            if (f & ItemStyle::kSelectedLabel) into.selectedLabel = s.selectedLabel;
            into.set |= f;
        }

        // the set glass fields of `s` over a material
        GlassMaterial Override(GlassMaterial m, const ItemStyle& s)
        {
            if (!(s.set & ItemStyle::kGlassFields))
                return m;
            ItemStyle base;
            base.glass = m;
            Merge(base, s);
            base.glass.lightAngle = m.lightAngle;
            base.glass.noise = m.noise;
            return base.glass;
        }

        GlassLook LookOf(const ItemStyle& s) { return s.Has(ItemStyle::kLook) ? s.look : (GlassLook)Ctx().glassLook.load(); }

        // the surface joins the tint (which the shader lays in layers) instead of a flat coat on top
        GlassMaterial FoldSurface(GlassMaterial m, Color surface)
        {
            if (surface.a <= 0.0f)
                return m;
            const float a = 1.0f - (1.0f - m.tint.a) * (1.0f - surface.a);
            const float k = a > 1e-4f ? 1.0f / a : 0.0f;
            m.tint = Color((surface.r * surface.a + m.tint.r * m.tint.a * (1.0f - surface.a)) * k,
                           (surface.g * surface.a + m.tint.g * m.tint.a * (1.0f - surface.a)) * k,
                           (surface.b * surface.a + m.tint.b * m.tint.a * (1.0f - surface.a)) * k, a);
            return m;
        }
    }

    void MergeStyle(ItemStyle& into, const ItemStyle& s) { Merge(into, s); }

    const ItemStyle& ResolvedStyle()
    {
        static const ItemStyle kNone;
        const std::vector<UiStacks::StyleEntry>& st = Ctx().ui.itemStyles;
        return st.empty() ? kNone : st.back().resolved;   // merged once, when the scope was pushed
    }

    GlassMaterial ApplyLook(GlassLook look, GlassMaterial m)
    {
        switch (look)
        {
        case GlassLook::Clear:
            // nothing but the glass itself: its lens, rim light and hairline
            m.blur = 0.0f;
            m.tint.a = 0.0f;
            m.legibility = 0.0f;
            m.saturation = std::min(m.saturation, 1.12f);
            m.brightness = std::min(m.brightness, 0.02f);
            break;
        case GlassLook::Frosted:
            // iOS frost: a soft blur and a light, layered veil (the shader lays it heavier on the bevel)
            m.blur = std::max(m.blur, 14.0f);
            m.tint.a = Clamp(m.tint.a, 0.08f, 0.14f);
            m.saturation = std::max(m.saturation, 1.4f);
            m.legibility = Clamp(m.legibility, 0.2f, 0.35f);   // only a gentle exposure: the backdrop's color shows
            break;
        default:
            break;
        }
        return m;
    }

    GlassMaterial StyledMaterial(const GlassMaterial& themed)
    {
        const ItemStyle& s = ResolvedStyle();
        return Override(ApplyLook(LookOf(s), themed), s);
    }

    GlassMaterial SurfaceMaterial(const GlassMaterial& themed, Color surface)
    {
        const ItemStyle& s = ResolvedStyle();
        const GlassLook look = LookOf(s);
        GlassMaterial m = ApplyLook(look, themed);
        if (look == GlassLook::Theme || s.Has(ItemStyle::kFill))
            m = FoldSurface(m, s.Has(ItemStyle::kFill) ? s.fill : surface);   // clear / frosted: no surface of their own
        return Override(m, s);
    }

    bool LookClear() { return ui::CurrentGlassLook() == GlassLook::Clear; }

    bool GlassSurface()
    {
        const ItemStyle& s = ResolvedStyle();
        if (s.Has(ItemStyle::kLook))
            return s.look != GlassLook::Theme;
        if (s.set & ItemStyle::kGlassFields)
            return true;
        return (GlassLook)Ctx().glassLook.load() == GlassLook::Clear;
    }

    Color Accent()
    {
        const ItemStyle& s = ResolvedStyle();
        return s.Has(ItemStyle::kTint) ? s.tint : C().accent;
    }

    Color AccentOr(Color themed)
    {
        const ItemStyle& s = ResolvedStyle();
        return s.Has(ItemStyle::kTint) ? s.tint : themed;
    }

    Color FillOr(Color themed)
    {
        const ItemStyle& s = ResolvedStyle();
        return s.Has(ItemStyle::kFill) ? s.fill : themed;
    }

    bool HasItemRadius() { return ResolvedStyle().Has(ItemStyle::kRadius); }

    float ItemRadius(float themed)
    {
        const ItemStyle& s = ResolvedStyle();
        return s.Has(ItemStyle::kRadius) ? Sc(s.radius) : themed;
    }

    bool TakeNextStyle()
    {
        Context::Impl& m = Ctx();
        if (m.ui.nextStyle.set == 0)
            return false;
        const ItemStyle s = m.ui.nextStyle;
        m.ui.nextStyle = ItemStyle();
        ui::PushItemStyle(s);
        return true;
    }

    GlassMaterial SurfaceGlass(Color fill)
    {
        // clear: nothing but the glass; frosted / custom glass: the surface color tints it, see-through
        const GlassMaterial m = StyledMaterial(T().materials.control);
        if (LookClear())
            return m;
        return FoldSurface(m, fill.Fade(std::min(1.0f, 0.55f / std::max(fill.a, 1e-3f))));
    }

    Style& SurfaceFill(Style& s, Color fill)
    {
        if (!GlassSurface())
            return s.Fill(fill);
        s.Glass(SurfaceGlass(fill));
        if (LookClear() && ResolvedStyle().Has(ItemStyle::kFill))
            s.Fill(fill);   // an explicit fill still shows on clear glass
        return s;
    }

    Style Surface(Color fill)
    {
        Style s;
        SurfaceFill(s, fill);
        return s;
    }

    Color StateFill(Color c) { return LookClear() ? c.Fade(0.55f) : c; }

    Color LabelOr(Color themed)
    {
        const ItemStyle& s = ResolvedStyle();
        return s.Has(ItemStyle::kLabel) ? s.label : themed;
    }
}

namespace wgt::ui::detail
{
    GlassMaterial GlassFieldsOver(GlassMaterial m) { return Override(m, ResolvedStyle()); }

    void DrawPill(Painter& p, const Rect& r, Style s)
    {
        if (HasItemRadius())
            p.Rect(r, s.Radius(std::min(ItemRadius(0.0f), std::min(r.Width(), r.Height()) * 0.5f)));
        else
            p.Capsule(r, s);
    }
}
