// Esia UI - what the widget files share: the Ui's state, style resolution, animation and drawing helpers.
#pragma once
#include "esia/ui/ui.hpp"
#include <algorithm>
#include <cstdarg>
#include <cstddef>
#include <mutex>
#include <span>
#include <string>
#include <vector>

namespace esia::ui
{
    class BoxLayout;
    namespace detail
    {
        struct NavState;
    }

    namespace detail
    {
        // The built-in controls' sizes in UI units, one table per density (Ui::SetDensity). As in WinUI's compact
        // sizing every text keeps its size (one size of label everywhere: a smaller one inside the controls only
        // looked patchy); a shorter control has smaller details - shadows, insets, the knob's swell, the lens's
        // refraction - so its glass does not outweigh it.
        struct ControlSizes
        {
            float detail;                       // a control's shadows, swell, insets and glass refraction
            float button[3], buttonPad[3];      // ControlSize Small, Regular, Large: height, padding across
            float field;                        // number and vector fields, segmented controls, steppers, sliders
            float stepperWidth;
            float rowControl;                   // a segmented control or a picker in a list row
            float textField;                    // text fields, the search bar's field
            float row;                          // menu and picker rows, the picker button, the color picker's hex row
            float tableRow, tableHeader, tableCell, treeRow;
            float toggleWidth, toggleHeight, checkbox;
            float sliderTrack, knobWidth, knobHeight, heldKnobWidth, heldKnobHeight;
            float colorBar;                     // the color picker's hue and opacity bars
            float navigationBar, searchBar, tabBar, dockTabs;
            float closeButton, windowIcon;      // in a window's header
        };
        static_assert(sizeof(ControlSizes) % sizeof(float) == 0, "ControlSizes holds floats only (Refresh blends them)");
    }

    namespace detail
    {
        // Springs (Anim) and timers (Timer) by id, held in one open-addressing table: every widget steps a few of them
        // each frame, and a probe here finds the value in the slot itself (Context::State keeps an entry apart from
        // its slot, a second memory access).
        struct AnimTable
        {
            enum Kind : std::uint32_t
            {
                kFree = 0,
                kSpring = 1,
                kTimer = 2,
                kPair = 3,    // two springs (an interaction's hover and press)
                kPulse = 4,   // a timer (value) and a spring (value2, velocity2): LiquidPulse
            };
            struct Entry
            {
                Id id = 0;
                std::uint32_t kind = kFree;
                std::uint32_t used = 0;      // the frame it was last asked for (its low 32 bits)
                std::uint32_t stepped = 0;   // the frame it last stepped; 0: at the next ask
                float value = 0.0f;          // a timer: 0 .. 1
                float velocity = 0.0f;
                float value2 = 0.0f, velocity2 = 0.0f;   // kPair, kPulse
                float more[8] = {};          // kPair: the control's own springs and timers (ControlSpring, ControlPulse)
            };
            // The entry of `id` and `kind`, made when there is none (`created`: its value is the caller's to set).
            Entry& Get(Id id, std::uint32_t kind, std::uint32_t frame, bool& created);
            // Drops the entries not asked for in `retain` frames.
            void Collect(std::uint32_t frame, std::uint32_t retain);

            std::vector<Entry> slots;   // a power of two in size, at most half full
            std::size_t count = 0;

        private:
            static std::size_t Home(Id id, std::uint32_t kind, std::size_t mask)
            {
                return (std::size_t)((((std::uint64_t)id << 2 | kind) * 0x9E3779B97F4A7C15ull) >> 40) & mask;
            }
            void Rehash(std::size_t size);
        };
    }

    struct Ui::Impl
    {
        Context* ctx = nullptr;
        text::TextSystem* text = nullptr;
        text::FontId fonts[(int)FontWeight::Count] = {};
        text::FontId iconFont = 0;
        ThemeAnimator theme;
        Theme effective;   // what the widgets use (T()): theme.current with its density applied (Refresh)
        detail::ControlSizes sizes;   // the controls' sizes at that density (Sizes())
        Color accentOverride = Color::Clear();
        GlassLook look = GlassLook::Frosted;
        bool flat = false;   // UiDesc::flat
        detail::AnimTable anims;   // Anim, Timer

        void Refresh();   // effective and sizes from theme.current (ui.cpp)

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
        const ItemStyle noStyle{};   // ResolvedStyle outside every scope

        // auto-layout containers being submitted, with the core depth of their direct children
        struct LayoutEntry
        {
            BoxLayout* layout = nullptr;
            int depth = 0;
            bool hidden = false;        // measuring frame: its content is clipped away
            bool stylePushed = false;   // it took a Next() style for itself and its children
            const DrawList* list = nullptr;   // its window's draw list (glows drawn there reserve room in it)
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
            Id child = 0;                   // window / scroll area: its scrolling child region
            Rect view;                      // window: its content region (ViewRect: padding included)
            float padX = 0.0f;              // window: the content's side padding (the scroll indicator's lane)
            float radius = 0.0f;            // window: its corner radius
        };
        std::vector<ContainerEntry> containers;
        int windowCascade = 0;

        // inset grouped sections being submitted (lists.cpp)
        struct SectionEntry
        {
            Id id = 0;
            float x0 = 0.0f, x1 = 0.0f;   // the card's left and right
            int rows = 0;
            Rect mask;                     // last frame's card at this frame's top: row highlights stay inside
            std::size_t mark = 0;          // where the card's background goes in the draw list
            bool stylePushed = false;
            bool counted = false;          // a second id scope: the section's place among those with its header
        };
        std::vector<SectionEntry> sections;
        std::vector<std::pair<Id, int>> sectionScopes;   // this frame: per section scope, the sections that had it
        std::vector<std::string> sectionFooters;   // per depth of sections: copies kept from frame to frame
        std::vector<std::uint8_t> rowStyles;   // BeginRow: whether the row took a Next() style

        // navigation stacks and pages being submitted (navigation.cpp)
        struct NavEntry
        {
            detail::NavState* state = nullptr;
            Vec2 origin, size;             // the navigation's area
        };
        std::vector<NavEntry> navs;
        struct PageEntry
        {
            bool clipped = false;          // the page below during a transition: clipped to the top page's edge
            Id child = 0;                  // its scrolling content
            bool edgeFade = false;
            Rect edgeShadow;               // the top page's shadow on it
            float shadowAlpha = 0.0f;
        };
        std::vector<PageEntry> pages;

        // Floating bars (tab bars): their draw commands, moved to the end of their window's draw list at EndFrame. A
        // child region draws in submission order, inside the scroll edge fades of the areas around it; a bar floats
        // over all of that (WGT's were child windows, drawn after their parent).
        struct FloatBlock
        {
            DrawList* list = nullptr;
            std::size_t from = 0, to = 0;
        };
        std::vector<FloatBlock> floats;
        // the scroll areas being submitted, innermost last: their edge fade (an index in the list's Fades(), -1 none)
        struct EdgeFadeScope
        {
            DrawList* list;
            int fade;
        };
        std::vector<EdgeFadeScope> edgeFades;

        // the island (overlays.cpp): what Notify / SetActivity queued (any thread), and what it shows
        bool islandEnabled = true;
        std::mutex islandMutex;
        std::vector<Notification> islandQueue;
        struct Activity
        {
            std::string id, title;
            float progress = -1.0f;
            Icon icon = 0;
            Color tint = Color::Clear();
        };
        std::vector<Activity> activities;
        struct IslandState
        {
            bool hasItem = false;
            Notification item;
            double start = 0.0;
            std::vector<std::string> knownActs;   // activity ids already shown (a new one arrives once)
            double actStart = -100.0;
            std::vector<Activity> acts;           // this frame's copy of the activities (its storage kept)
        };
        IslandState island;

        // the item a tooltip waits on (popups.cpp)
        Id tooltipItem = 0;
        Rect tooltipRect;
        double tooltipSince = 0.0;
    };

    namespace detail
    {
        // The current Ui's state (asserts there is one): read by nearly every function here, so inline.
        extern thread_local Ui::Impl* g_impl;
        inline Ui::Impl& M()
        {
            ESIA_ASSERT(g_impl && "ui:: functions need a current Ui (between Ui::NewFrame and Ui::EndFrame)");
            return *g_impl;
        }
        inline Context& Ctx() { return *M().ctx; }
        inline const Theme& T() { return M().effective; }
        inline const Palette& C() { return M().effective.colors; }
        inline float Sc(float v) { return v * M().effective.metrics.scale; }
        // The built-in controls' sizes at the Ui's density (UI units before the scale: Sc them)
        inline const ControlSizes& Sizes() { return M().sizes; }
        // A detail of a control (a shadow's radius, an inset) at the density
        inline float Dt(float v) { return Sc(v * Sizes().detail); }

        inline const Spring& SpringFast() { return T().motion.fast; }
        inline const Spring& SpringStd() { return T().motion.standard; }
        inline const Spring& SpringBouncy() { return T().motion.bouncy; }
        inline float Anim(Id id, std::uint32_t salt, float target, const Spring& s, float initial = NAN)
        {
            return ui::Anim(Salt(id, salt), target, s, initial);
        }
        // A spring and its velocity in one look-up (Anim, then AnimVelocity).
        float AnimWithVelocity(Id id, float target, const Spring& s, float& velocity);

        // ---- text (inline: every label asks)
        inline text::FontRef Font(FontWeight w, float size)
        {
            const Ui::Impl& m = M();
            // quarter units (Ui::Font): sizes > 0, so adding a half and truncating rounds
            return {m.fonts[(int)w], (float)(int)(size * m.effective.metrics.scale * 4.0f + 0.5f) * 0.25f};
        }
        inline text::FontRef Font(TextStyle s)
        {
            const Typography& t = M().effective.type;
            return Font(t.weight[(int)s], t.size[(int)s]);
        }
        // the label before "##" (the rest is only its id)
        inline std::string_view VisibleLabel(std::string_view label)
        {
            const char* p = label.data();
            for (std::size_t i = 0; i + 1 < label.size(); ++i)
                if (p[i] == '#' && p[i + 1] == '#')
                    return label.substr(0, i);
            return label;
        }

        // ---- styles
        // every scope merged (inline: the widgets ask for it a dozen times each)
        inline const ItemStyle& ResolvedStyle()
        {
            const Ui::Impl& m = M();
            return m.styles.empty() ? m.noStyle : m.styles.back().resolved;
        }
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
        inline float StyleAlpha()
        {
            const Ui::Impl& m = M();
            return m.styles.empty() ? 1.0f : m.styles.back().alpha;
        }

        // the style's tint, or the theme accent
        inline Color Accent()
        {
            const ItemStyle& s = ResolvedStyle();
            return s.Has(ItemStyle::kTint) ? s.tint : C().accent;
        }
        inline Color Tint(Color c) { return c.a > 0.0f ? c : Accent(); }
        // the style's tint, else `themed` (a switch's green)
        inline Color AccentOr(Color themed)
        {
            const ItemStyle& s = ResolvedStyle();
            return s.Has(ItemStyle::kTint) ? s.tint : themed;
        }
        inline Color FillOr(Color themed)
        {
            const ItemStyle& s = ResolvedStyle();
            return s.Has(ItemStyle::kFill) ? s.fill : themed;
        }
        inline Color LabelOr(Color themed)
        {
            const ItemStyle& s = ResolvedStyle();
            return s.Has(ItemStyle::kLabel) ? s.label : themed;
        }
        inline bool HasItemRadius() { return ResolvedStyle().Has(ItemStyle::kRadius); }
        inline float ItemRadius(float themed)
        {
            const ItemStyle& s = ResolvedStyle();
            return s.Has(ItemStyle::kRadius) ? Sc(s.radius) : themed;
        }
        GlassMaterial ApplyLook(GlassLook look, GlassMaterial m);
        GlassMaterial StyledMaterial(const GlassMaterial& themed);                 // look + glass fields
        GlassMaterial GlassFieldsOver(const GlassMaterial& m);                     // the glass fields only
        GlassMaterial SurfaceMaterial(const GlassMaterial& themed, Color surface);  // a glass surface with a color of its own
        // the look of the current scope (CurrentGlassLook)
        inline GlassLook Look()
        {
            const ItemStyle& s = ResolvedStyle();
            return s.Has(ItemStyle::kLook) ? s.look : M().look;
        }
        inline bool LookClear() { return Look() == GlassLook::Clear; }
        // flat surfaces render as glass (Clear / Frosted / glass fields)
        inline bool GlassSurface()
        {
            const ItemStyle& s = ResolvedStyle();
            if (s.Has(ItemStyle::kLook))
                return s.look != GlassLook::Theme;
            if (s.set & ItemStyle::kGlassFields)
                return true;
            return M().look == GlassLook::Clear;
        }
        GlassMaterial SurfaceGlass(Color fill);
        Style& SurfaceFill(Style& s, Color fill);
        Style Surface(Color fill);
        inline Color StateFill(Color c) { return LookClear() ? c.Fade(0.55f) : c; }   // "on" / selection / progress colors: see-through under Clear
        void DrawPill(Painter& p, const Rect& r, const Style& s);   // a capsule, or a rounded rect with the style's radius
        // DrawPill(p, r, Style().Fill(fill)) without a Style
        inline void FillPill(Painter& p, const Rect& r, Color fill)
        {
            const float half = std::min(r.Width(), r.Height()) * 0.5f;
            p.FillRound(r, HasItemRadius() ? std::min(ItemRadius(0.0f), half) : half, fill);
        }

        // The clear lens a knob or selection becomes while pressed (iOS 26): no frost, no tint, magnified content.
        GlassMaterial LensMaterial();
        // Rises while held and blooms briefly after each activation, so a quick click gets the liquid feedback too.
        float LiquidPulse(Id id, bool held, bool activated, float bloomSeconds = 0.20f);
        // A pill that overshoots its track squashes against the wall instead of leaving it.
        Rect ContainLiquid(Rect pill, const Rect& track, float slack);
        // How far a drop shadow reaches beyond its shape.
        inline float ShadowExtent(float blur, Vec2 offset) { return blur * 1.6f + std::max(std::fabs(offset.x), std::fabs(offset.y)) + 4.0f; }
        // Widens the clip while it lives (shadows and glows reach past their window / container); flat drawing has
        // neither, and keeps the clip.
        struct ScopedUnclip
        {
            ScopedUnclip(const Rect& r, float extent);
            ~ScopedUnclip();
            ScopedUnclip(const ScopedUnclip&) = delete;
            ScopedUnclip& operator=(const ScopedUnclip&) = delete;

        private:
            bool pushed_;
        };

        // ---- text and icons (Font, VisibleLabel: above)
        Vec2 MeasureText(text::FontRef f, std::string_view text, float wrapWidth = 0.0f);
        void DrawIcon(Painter& p, Vec2 center, Icon icon, float size, Color color);
        void TextImpl(text::FontRef f, Color color, std::string_view text, bool wrap);
        // vsnprintf into `buf` (truncated to size - 1, terminated), the length written. The formats labels use most -
        // %d %i %u %x %X %s %c %% (with l, ll, z), no flags, width or precision - are written here: a C runtime's
        // vsnprintf takes its locale lock first (Microsoft's: a sixth of a frame of labels). Anything else goes to
        // vsnprintf. The output is the same.
        std::size_t FormatV(char* buf, std::size_t size, const char* fmt, va_list args);

        // DrawList::MoveCommands for the widget layer (a card's background under its content): the floating blocks it
        // shifts are kept track of.
        void MoveCommands(DrawList& dl, std::size_t from, std::size_t to);

        // ---- scrolling
        // Content dissolves toward an edge it can still scroll past (the innermost scroll area being submitted).
        // Every Begin has its End; `allowed` false: no fade (an area that does not scroll).
        bool BeginScrollEdgeFade(bool allowed = true);
        void EndScrollEdgeFade(bool started);
        // A bar floating over the bottom of the area being submitted (the tab bar, the search bar), `room` the space
        // left below the content for it: what scrolls under the bar fades out before it reaches the bar's middle.
        void FadeUnderBar(const Rect& bar, float room);
        // WGT's ScrollAreaBegin / End on the core's scrolling: ScrollBegin before the area's BeginChild (a drag in
        // progress sets the offset), ScrollEnd inside it before EndChild, after its edge fade: drag-to-scroll on empty
        // space with momentum, and the auto-hiding, draggable indicator in the lane at `laneX` (< 0: over the content
        // at the right edge). `child`: the id BeginChild gave the area. `cornerRadius` > 0: the lane is its window's,
        // whose corners have that radius - the track ends that far above the window's bottom, inside its shape and
        // clear of its resize grip.
        void ScrollBegin(Id child);
        void ScrollEnd(Id child, float laneX, float cornerRadius = 0.0f);
        // The lane of a scroll area flush with the right of the innermost window's content: in its padding (else -1);
        // `cornerRadius`: that window's corner radius (when the lane is its).
        float WindowLane(const Rect& view, float* cornerRadius = nullptr);
        // The square at a window's bottom-right corner that resizes it (WindowOptions::resizeGrip), where its grip shows.
        float ResizeGripSize();

        // ---- layout
        // In an auto-layout container, true while the item being submitted is one of its direct children.
        bool InLayoutContainer();
        void MarkFill();   // the item being submitted fills what it is offered
        // WGT's item map for Painter (PainterEnv::glow): an outer glow fades out before the neighbouring items of its
        // window (last frame's laid-out items), and the layout container of a glowing child keeps room for the glow.
        GlowContainment* ItemMapGlow();

        // ---- overlays
        void IslandFrame();   // Ui::EndFrame: the island on the foreground draw list

        // ---- interaction
        // What InteractImpl tells: an Interaction without its style (a widget takes its own; an ItemStyle made for
        // every control cost what the rest of this does)
        struct InteractState
        {
            Id id = 0;
            Rect rect;
            bool visible = false;
            bool hovered = false;
            bool held = false;
            bool pressed = false;
            float hover = 0.0f;
            float press = 0.0f;
            // the interaction's entry (visible items): its hover and press, and the control's own springs and timers
            AnimTable::Entry* anim = nullptr;
            bool animNew = false;    // made this call: its springs start at their targets
            bool animStep = false;   // not stepped yet this frame
        };
        InteractState InteractImpl(Id id, const Rect& r, std::uint32_t flags);
        // A control's own springs and timers, in its interaction's entry (Entry::more) beside the hover and press: one
        // look-up for all of them. `slot` indexes Entry::more. Call them right after InteractImpl, before anything else
        // animates (an entry made in between may move the table).
        float ControlSpring(const InteractState& it, int slot, float target, const Spring& s);   // more[slot], more[slot + 1]
        float ControlSpring(const InteractState& it, int slot, float target, const Spring& s, float& velocity);
        // LiquidPulse in more[slot .. slot + 2]
        float ControlPulse(const InteractState& it, int slot, bool held, bool activated, float bloomSeconds = 0.20f);
        // controls at explicit rects (list rows use them)
        Vec2 ToggleSize();
        Vec2 StepperSize();
        bool ToggleAt(Id id, const Rect& r, bool* value);
        bool SliderAt(Id id, const Rect& r, float* value, float mn, float mx, const SliderOptions& o);
        bool StepperAt(Id id, const Rect& r, int* value, int mn, int mx, int step);
        void IconTile(Painter& p, const Rect& r, Icon icon, Color color);
        bool SegmentedAt(Id id, const Rect& r, int* selected, std::span<const std::string_view> items);
        // plain: in a list row (no well, the choice in the secondary color)
        // maxRows: the choices the menu shows at most (more scroll inside it), 0 = what fits on the display.
        bool PickerAt(Id id, const Rect& r, int* selected, std::span<const std::string_view> items, bool plain, int maxRows = 10);
        // the text field at an explicit rect (number fields, the color picker's hex field)
        TextFieldResult TextFieldAt(Id id, const Rect& r, std::string* value, std::string_view placeholder, const TextFieldOptions& o);

        // ---- plots (plot.cpp)
        // The ticks of an axis from lo to hi: multiples of 1, 2 or 5 times a power of ten, at most maxTicks of them
        // (out, cleared first); the step between them (0: an empty range).
        float PlotTicks(float lo, float hi, int maxTicks, std::vector<float>& out);

        // ---- docking (dock.cpp)
        struct DockPlacement
        {
            Rect rect;                     // its node
            bool shown = false;            // its tab is the node's active one
            bool closeRequested = false;   // its tab's close button was pressed
            bool pendingMove = false;      // just undocked by a tab drag (floating): under the mouse, moving with it
            Vec2 grab;
        };
        // BeginWindow: true when `title` is docked in a dock space submitted this frame; `closable` and `icon` are
        // what its tab shows.
        bool DockedPlacement(std::string_view title, bool closable, Icon icon, DockPlacement& out);
        // A docked window's header: its node's tabs. Returns the header's height.
        float DockTabBar(std::string_view title, const Rect& header);

        // ---- glass popups (popups.cpp): a popup of the core on glass, its rows without gaps. Rows taller than
        // `maxHeight` (UI units; 0 = what fits on the display, which also bounds it) scroll inside that height;
        // `centerOn` >= 0: the y in the rows (from their top) brought to the middle when it opens (a picker's choice).
        bool BeginGlassPopup(Id id, PopupOptions options, float minWidth, float maxHeight = 0.0f, float centerOn = -1.0f);
        void EndGlassPopup();
        bool PopupRow(std::string_view label, bool selected, Icon icon = 0);

        // ---- the liquid selection of segmented controls and tab bars (selection.cpp)
        struct LiquidSelection
        {
            float pos = 0.0f;       // animated position, item units
            float velocity = 0.0f;  // px / s
            float lens = 0.0f;      // 0 = resting pill .. 1 (+overshoot) = clear lens
            int hovered = -1;
            bool held = false;
            bool changed = false;   // *selected changed (on release)
        };
        LiquidSelection LiquidSelect(Id id, const Rect& area, int count, int* selected);
        Rect LiquidSelectionRect(const LiquidSelection& s, const Rect& area, int count, float inset);
        // The lens over the items (draw it after them): `rise` = how far it grows past the track, top and bottom.
        void DrawSelectionLens(Painter& p, const Rect& pill, const LiquidSelection& s, float rise, Color tint = Color::Clear());
    }
}
