// Esia UI - what the widget files share: the Ui's state, style resolution, animation and drawing helpers.
#pragma once
#include "esia/ui/ui.hpp"
#include <vector>

namespace esia::ui
{
    class BoxLayout;

    struct Ui::Impl
    {
        Context* ctx = nullptr;
        text::TextSystem* text = nullptr;
        text::FontId fonts[(int)FontWeight::Count] = {};
        text::FontId iconFont = 0;
        ThemeAnimator theme;
        Color accentOverride = Color::Clear();
        GlassLook look = GlassLook::Frosted;

        double time = 0.0;
        float dt = 0.0f;
        bool animating = false;   // a spring moved this frame
        bool inFrame = false;
        std::uint32_t decorationSerial = 0;   // widgets without an id this frame (progress bars ...), in order

        // styles: Next() waits for the next widget; scopes hold the merged style and opacity of every enclosing
        // container / StyleScope / widget (innermost last)
        ItemStyle next;
        struct StyleEntry
        {
            ItemStyle resolved;
            float alpha = 1.0f;
        };
        std::vector<StyleEntry> styles;

        // auto-layout containers being submitted, with the core depth of their direct children
        struct LayoutEntry
        {
            BoxLayout* layout = nullptr;
            int depth = 0;
            bool hidden = false;        // measuring frame: its content is clipped away
            bool stylePushed = false;   // it took a Next() style for itself and its children
        };
        std::vector<LayoutEntry> layouts;

        // widget containers being submitted (card / window / scroll area): what their End needs
        struct ContainerEntry
        {
            enum class Kind : std::uint8_t { Card, Window, Scroll } kind = Kind::Card;
            Id id = 0;
            bool stylePushed = false;
            std::size_t mark = 0;           // card: the draw list position its background goes to
            Vec2 origin;
            CardOptions card;
            std::uint32_t windowFlags = 0;
            float cornerCut = 0.0f;
            bool edgeFade = false;
        };
        std::vector<ContainerEntry> containers;
        int windowCascade = 0;
    };

    namespace detail
    {
        Ui::Impl& M();   // the current Ui's state (asserts there is one)
        inline Context& Ctx() { return *M().ctx; }
        inline const Theme& T() { return M().theme.current; }
        inline const Palette& C() { return M().theme.current.colors; }
        inline float Sc(float v) { return v * M().theme.current.metrics.scale; }

        inline const Spring& SpringFast() { return T().motion.fast; }
        inline const Spring& SpringStd() { return T().motion.standard; }
        inline const Spring& SpringBouncy() { return T().motion.bouncy; }
        inline float Anim(Id id, std::uint32_t salt, float target, const Spring& s, float initial = NAN)
        {
            return ui::Anim(Salt(id, salt), target, s, initial);
        }

        // ---- styles
        const ItemStyle& ResolvedStyle();   // every scope merged
        void MergeStyle(ItemStyle& into, const ItemStyle& s);
        void PushStyle(const ItemStyle& s);
        void PopStyle();
        // Pushes a pending Next() style (true: the caller pops it). Leaf widgets hold an ItemScope for their duration,
        // containers keep the style until their End.
        bool TakeNextStyle();
        struct ItemScope
        {
            bool pushed;
            ItemScope() : pushed(TakeNextStyle()) {}
            ~ItemScope()
            {
                if (pushed)
                    PopStyle();
            }
            ItemScope(const ItemScope&) = delete;
            ItemScope& operator=(const ItemScope&) = delete;
        };
        float StyleAlpha();

        Color Accent();                      // the style's tint, or the theme accent
        inline Color Tint(Color c) { return c.a > 0.0f ? c : Accent(); }
        Color AccentOr(Color themed);        // the style's tint, else `themed` (a switch's green)
        Color FillOr(Color themed);
        Color LabelOr(Color themed);
        bool HasItemRadius();
        float ItemRadius(float themed);
        GlassMaterial ApplyLook(GlassLook look, GlassMaterial m);
        GlassMaterial StyledMaterial(const GlassMaterial& themed);                 // look + glass fields
        GlassMaterial SurfaceMaterial(const GlassMaterial& themed, Color surface);  // a glass surface with a color of its own
        bool LookClear();
        bool GlassSurface();                 // flat surfaces render as glass (Clear / Frosted / glass fields)
        GlassMaterial SurfaceGlass(Color fill);
        Style& SurfaceFill(Style& s, Color fill);
        Style Surface(Color fill);
        Color StateFill(Color c);            // "on" / selection / progress colors: see-through under Clear
        void DrawPill(Painter& p, const Rect& r, Style s);   // a capsule, or a rounded rect with the style's radius

        // The clear lens a knob or selection becomes while pressed (iOS 26): no frost, no tint, magnified content.
        GlassMaterial LensMaterial();
        // Rises while held and blooms briefly after each activation, so a quick click gets the liquid feedback too.
        float LiquidPulse(Id id, bool held, bool activated, float bloomSeconds = 0.20f);
        // A pill that overshoots its track squashes against the wall instead of leaving it.
        Rect ContainLiquid(Rect pill, const Rect& track, float slack);
        // How far a drop shadow reaches beyond its shape.
        inline float ShadowExtent(float blur, Vec2 offset) { return blur * 1.6f + std::max(std::fabs(offset.x), std::fabs(offset.y)) + 4.0f; }
        // Widens the clip while it lives (shadows and glows reach past their window / container).
        struct ScopedUnclip
        {
            ScopedUnclip(const Rect& r, float extent);
            ~ScopedUnclip();
            ScopedUnclip(const ScopedUnclip&) = delete;
            ScopedUnclip& operator=(const ScopedUnclip&) = delete;
        };

        // ---- text and icons
        text::FontRef Font(FontWeight w, float size);
        text::FontRef Font(TextStyle s);
        Vec2 MeasureText(text::FontRef f, std::string_view text, float wrapWidth = 0.0f);
        void DrawIcon(Painter& p, Vec2 center, Icon icon, float size, Color color);
        // A label without its "##id" suffix.
        std::string_view VisibleLabel(std::string_view label);
        void TextImpl(text::FontRef f, Color color, std::string_view text, bool wrap);

        // ---- layout
        // In an auto-layout container, true while the item being submitted is one of its direct children.
        bool InLayoutContainer();
        void MarkFill();   // the item being submitted fills what it is offered

        // ---- interaction
        Interaction InteractImpl(Id id, const Rect& r, std::uint32_t flags);
        // controls at explicit rects (list rows use them)
        Vec2 ToggleSize();
        Vec2 StepperSize();
        bool ToggleAt(Id id, const Rect& r, bool* value);
        bool SliderAt(Id id, const Rect& r, float* value, float mn, float mx, const SliderOptions& o);
        bool StepperAt(Id id, const Rect& r, int* value, int mn, int mx, int step);
        void IconTile(Painter& p, const Rect& r, Icon icon, Color color);
    }
}
