// WGT UI - widget internals shared across ui/*.cpp
#pragma once
#include "core/context_impl.hpp"

namespace wgt::ui::detail
{
    inline Context::Impl& Ctx() { return RequireImpl(); }
    inline const Theme& T() { return RequireImpl().theme.current; }
    inline const Palette& C() { return RequireImpl().theme.current.colors; }
    inline float Sc(float v) { return v * RequireImpl().theme.current.metrics.scale; }
    Color Accent();   // the component's tint (ui::Next().Tint / a scope) or the theme accent
    inline Color Tint(Color c) { return c.a > 0.0f ? c : Accent(); }
    inline ImGuiID Salt(ImGuiID id, std::uint32_t salt) { return (id ^ (salt * 0x9E3779B9u)) * 16777619u; }

    inline FontRef Font(FontWeight w, float unscaledSize) { return GetFont(w, unscaledSize); }

    inline float Anim(ImGuiID id, std::uint32_t salt, float target, const Spring& s, float initial = NAN)
    {
        return anim::Float(Salt(id, salt), target, &s, initial);
    }

    // Spring presets for controls
    inline const Spring& SpringFast() { return T().motion.fast; }
    inline const Spring& SpringStd() { return T().motion.standard; }
    inline const Spring& SpringBouncy() { return T().motion.bouncy; }

    // "Liquid lens" amount (0..~1.15): rises while the control is held and also blooms briefly after every
    // activation, so a quick click gets the same liquid feedback as a long press. Springy, slightly bouncy.
    inline float LiquidPulse(ImGuiID id, bool held, bool activated, float bloomSeconds = 0.20f)
    {
        const float since = anim::Timer(Salt(id, 0xB100), bloomSeconds, activated);
        const bool on = held || since < 1.0f;
        static const Spring kLens{0.34f, 0.62f};
        return Clamp(anim::Float(Salt(id, 0xB101), on ? 1.0f : 0.0f, &kLens), 0.0f, 1.15f);
    }

    // Knob glass: clear, strongly refractive lens used by pressed knobs (iOS 26 liquid controls).
    // The clear lens a selection / knob becomes while it is pressed or moving (iOS): no frost, no tint, the
    // content under it enlarged, a strong lens at the rim with rainbow dispersion.
    inline GlassMaterial LensMaterial()
    {
        GlassMaterial m = T().materials.clear;
        m.blur = 0.0f;
        m.refraction = 10.0f;
        m.bezel = 40.0f;   // one rounded rod / dome
        m.dispersion = 1.0f;
        m.magnify = 0.28f;
        m.saturation = 1.1f;
        m.brightness = 0.0f;
        m.specular = 1.0f;
        m.legibility = 0.0f;
        m.tint = Color(1, 1, 1, 0.0f);
        m.rim = T().dark ? Color(0, 0, 0, 0.55f) : Color(0, 0, 0, 0.20f);
        return m;
    }

    Interaction InteractImpl(ImGuiID id, const Rect& r, std::uint32_t flags);

    // Soft shadows / glows extend beyond the ImGui window that owns the shape. Draw such backgrounds with
    // the draw-list clip widened by the effect extent, otherwise the shadow is cut into a hard rectangle.
    struct ScopedUnclip
    {
        ImDrawList* dl;
        ScopedUnclip(ImDrawList* drawList, const Rect& r, float extent) : dl(drawList)
        {
            dl->PushClipRect(Vec2(r.min.x - extent, r.min.y - extent), Vec2(r.max.x + extent, r.max.y + extent), false);
        }
        ~ScopedUnclip() { dl->PopClipRect(); }
        ScopedUnclip(const ScopedUnclip&) = delete;
        ScopedUnclip& operator=(const ScopedUnclip&) = delete;
    };
    // Extent of a drop shadow (blur, offset) as rendered by the FX shader.
    inline float ShadowExtent(float blur, Vec2 offset) { return blur * 1.6f + std::max(std::fabs(offset.x), std::fabs(offset.y)) + 4.0f; }

    // A sliding pill that overshoots its track squashes against the wall instead of leaving it:
    // it shortens along the motion and bulges a little across it (volume is roughly kept).
    inline Rect ContainLiquid(Rect pill, const Rect& track, float slack)
    {
        const float overR = std::max(0.0f, pill.max.x - (track.max.x + slack));
        const float overL = std::max(0.0f, (track.min.x - slack) - pill.min.x);
        pill.max.x -= overR;
        pill.min.x += overL;
        const float bulge = std::min((overR + overL) * 0.18f, pill.Height() * 0.10f);
        return pill.Expanded(0.0f, bulge);
    }

    // glass looks (look.cpp)
    GlassMaterial ApplyLook(GlassLook look, GlassMaterial m);
    // A glass surface that also carries a flat surface color (windows, cards): the color joins the (layered)
    // tint, and the current look applies; a clear look drops it.
    GlassMaterial SurfaceMaterial(const GlassMaterial& themed, Color surface);
    bool LookClear();
    // Per-component style (look.cpp). TakeNextStyle pushes a pending ui::Next() style (true = the caller pops
    // it): leaf widgets hold an ItemScope for their duration, containers keep it until their End*().
    bool TakeNextStyle();
    struct ItemScope
    {
        bool pushed;
        ItemScope() : pushed(TakeNextStyle()) {}
        ~ItemScope()
        {
            if (pushed)
                ui::PopItemStyle();
        }
        ItemScope(const ItemScope&) = delete;
        ItemScope& operator=(const ItemScope&) = delete;
    };
    const ItemStyle& ResolvedStyle();                        // all scopes merged (cached per scope)
    void MergeStyle(ItemStyle& into, const ItemStyle& s);    // `s` over `into`, field by field
    Color LabelOr(Color themed);                             // the style label color, else `themed`
    GlassMaterial StyledMaterial(const GlassMaterial& themed);   // look + glass fields
    bool GlassSurface();                                     // flat surfaces render as glass (Clear / Frosted / glass fields)
    Color AccentOr(Color themed);                            // the style tint, else `themed` (e.g. a switch's green)
    Color FillOr(Color themed);                              // the style fill, else `themed`
    bool HasItemRadius();
    float ItemRadius(float themed);                          // the style radius (scaled), else `themed`
    GlassMaterial SurfaceGlass(Color fill);                  // a flat surface as glass (its color tints it)
    GlassMaterial GlassFieldsOver(GlassMaterial m);          // only the style's glass fields over `m`
    void DrawPill(Painter& p, const Rect& r, Style s);       // a capsule, or a rounded rect with the style radius
    // A component's flat surface (background, track, field): its fill, or glass when the style asks for it (clear
    // glass under Clear; frosted / custom glass tinted by the fill otherwise).
    Style& SurfaceFill(Style& s, Color fill);
    Style Surface(Color fill);
    // A colored state fill ("on", selection, progress, icon tiles): kept, but see-through under the Clear look.
    Color StateFill(Color c);

    // liquid selection (selection.cpp): segmented controls and tab bars
    struct LiquidSelection
    {
        float pos = 0.0f;       // animated position, item units
        float velocity = 0.0f;  // px / s
        float lens = 0.0f;      // 0 = resting pill .. 1 (+overshoot) = clear lens
        int hovered = -1;
        bool held = false;
        bool changed = false;   // *selected changed (on release)
    };
    LiquidSelection LiquidSelect(ImGuiID id, const Rect& area, int count, int* selected);
    Rect LiquidSelectionRect(const LiquidSelection& s, const Rect& area, int count, float inset);
    // The lens over the items (draw it after them): `rise` = how far it grows past the track, top and bottom.
    void DrawSelectionLens(Painter& p, const Rect& pill, const LiquidSelection& s, float rise, Color tint = Color::Clear());

    // auto layout (layout.cpp)
    void OnItemSize(Context::Impl& m, ImGuiWindow* window, const Rect& r);
    // Width available at the cursor; inside a layout container it marks the item as a flexible child.
    float AvailWidth();
    // True while the item being submitted belongs to an auto-layout container (the container sizes and
    // places it).
    bool InLayoutContainer();

    // text
    void TextImpl(FontRef font, Color color, const char* text, const char* end, bool wrap);

    // controls with explicit rects (used by list rows)
    Vec2 ToggleSize();
    Vec2 StepperSize();
    bool ToggleAt(ImGuiID id, const Rect& r, bool* value);
    bool SliderAt(ImGuiID id, const Rect& r, float* value, float mn, float mx, const SliderOptions& o);
    bool StepperAt(ImGuiID id, const Rect& r, int* value, int mn, int mx, int step);
    bool SegmentedAt(ImGuiID id, const Rect& r, int* selected, const char* const* items, int count);
    bool PickerAt(ImGuiID id, const Rect& r, int* selected, const char* const* items, int count, bool plain);
    void IconTile(Painter& p, const Rect& r, Icon icon, Color color);

    // glass popups (menus)
    bool BeginGlassPopup(ImGuiID id, float minWidth);
    void EndGlassPopup();
    bool PopupRow(const char* label, bool selected, Icon icon = 0);

    // scroll areas. topCut / bottomCut move the content clip in from those edges (rounded window corners).
    // Content fades out towards the edges it can still scroll past (Painter::BeginEdgeFade).
    void ScrollAreaBegin(ImGuiID id, const char* name, Vec2 size, Vec2 padding, bool allowScroll, float topCut = 0.0f, float bottomCut = 0.0f);
    void ScrollAreaEnd();
}
