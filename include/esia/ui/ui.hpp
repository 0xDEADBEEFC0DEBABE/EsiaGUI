// Esia UI - the widget layer: liquid-glass widgets drawn with Painter on the UI core (WGT's widgets, ported).
//
// Immediate mode, as WGT: a widget is a function call per frame, and what must survive a frame (springs, layout
// measurements, scroll physics) lives in the core's per-id state.
//
//   esia::Context ctx;
//   esia::ui::Ui ui(ctx, {.text = textSystem});
//   // every frame:
//   ctx.NewFrame(params);
//   ui.NewFrame();
//   if (ui::BeginWindow("Settings")) {
//       ui::Text(ui::TextStyle::Headline, "Display");
//       if (ui::Button("Apply")) ...;
//       ui::Next().Tint(Color::Hex(0x30D158));   // the next widget only: no push / pop
//       ui::Toggle("hdr", &hdr);
//       ui::EndWindow();
//   }
//   ui.EndFrame();
//   ctx.EndFrame();
//
// Styling (docs/UI_WIDGETS.md): the Theme gives every color, material and size; ui::Next() styles one widget; a
// container (window, card, stack ...) takes the Next() style for itself and everything inside it; ui::StyleScope
// styles a block. Styles merge field by field, the innermost wins.
//
// Threading: one Ui per Context, on the Context's thread. The ui:: functions act on the Ui between its NewFrame and
// EndFrame (the "current" Ui of the thread).
#pragma once
#include "esia/core/context.hpp"
#include "esia/render/painter.hpp"
#include "esia/text/text.hpp"
#include "esia/ui/anim.hpp"
#include "esia/ui/icons.hpp"
#include "esia/ui/theme.hpp"
#include <cmath>
#include <initializer_list>
#include <memory>
#include <span>
#include <string>
#include <string_view>

namespace esia::ui
{
    // ============================================================== the Ui
    struct UiDesc
    {
        // Required for text. The Ui loads its fonts into it (below); the host renders with it (PainterEnv).
        text::TextSystem* text = nullptr;
        // Font files per FontWeight (UTF-8 paths). Empty: the platform's UI font at that weight (system_fonts.hpp);
        // Mono: the platform's monospace font.
        std::string fontFiles[(int)FontWeight::Count];
        // The icon font. Empty: "Segoe Fluent Icons", else "Segoe MDL2 Assets" when installed.
        std::string iconFontFile;
        // Add the platform's fallback chain (CJK, symbols) after the UI font.
        bool fallbackChain = true;
        Theme theme = ThemeLight();
        GlassLook glassLook = GlassLook::Frosted;
    };

    class ESIA_API Ui
    {
    public:
        Ui(Context& context, const UiDesc& desc);
        ~Ui();
        Ui(const Ui&) = delete;
        Ui& operator=(const Ui&) = delete;

        // After Context::NewFrame, before the first widget: steps the theme transition and makes this the current
        // Ui of the thread. EndFrame: before Context::EndFrame.
        void NewFrame();
        void EndFrame();

        Context& GetContext() const;
        text::TextSystem* Text() const;

        const Theme& GetTheme() const;   // the one in effect (mid-transition: the blend)
        void SetTheme(const Theme& theme, bool animate = true);
        void SetDarkMode(bool dark, bool animate = true);   // the built-in light / dark theme with the accent kept
        void SetAccent(Color accent);                       // Clear: the theme's own
        void SetGlassLook(GlassLook look);
        GlassLook GetGlassLook() const;

        text::FontRef Font(TextStyle style) const;
        text::FontRef Font(FontWeight weight, float size) const;   // size in UI units at metrics scale 1
        text::FontRef IconFont(float size) const;                  // id 0 when no icon font was found

        // Something moved this frame (springs, the theme): an event-driven host renders another frame.
        bool Animating() const;

        struct Impl;
        Impl& GetImpl() const { return *impl_; }

    private:
        std::unique_ptr<Impl> impl_;
    };

    // The current Ui (between NewFrame and EndFrame), or null.
    ESIA_API Ui* Current();

    // ====================================================== per-widget style
    // Only the fields that are set change; everything else follows the theme and the enclosing scopes.
    //
    //   ui::Next().Tint(Color::Hex(0x30D158)).Radius(8);   // the next widget only
    //   ui::Next().Look(GlassLook::Frosted).Blur(24);       // a flat widget (filled button, switch track, field ...)
    //                                                       // becomes that glass
    //   ui::Next().Fill(blue).SelectedFill(white);          // surfaces and selections
    //   { ui::StyleScope s(ui::ItemStyle().Tint(red).Opacity(0.8f)); ... }   // a block
    //
    // Before a container (window, card, stack ...) it styles the container and everything inside it.
    struct ItemStyle
    {
        enum Field : std::uint32_t
        {
            kLook = 1u << 0,
            kBlur = 1u << 1, kRefraction = 1u << 2, kBezel = 1u << 3, kDispersion = 1u << 4, kSaturation = 1u << 5,
            kBrightness = 1u << 6, kSpecular = 1u << 7, kLegibility = 1u << 8, kMagnify = 1u << 9,
            kGlassTint = 1u << 10, kRim = 1u << 11,
            kGlassFields = 0x0FFEu,   // blur .. rim
            kTint = 1u << 16, kFill = 1u << 17, kRadius = 1u << 18, kOpacity = 1u << 19,
            kLabel = 1u << 20, kSelectedFill = 1u << 21, kMovingFill = 1u << 22, kSelectedLabel = 1u << 23,
        };
        std::uint32_t set = 0;       // which fields are set
        GlassLook look = GlassLook::Theme;
        GlassMaterial glass;         // the glass fields that are set (see `set`)
        Color tint, fill, label, selectedFill, movingFill, selectedLabel;
        float radius = 0.0f, opacity = 1.0f;

        // ---- glass. On a flat widget, Clear, Frosted or any glass field turns its surface into that glass (its fill
        // color tints the glass).
        ItemStyle& Look(GlassLook l) { look = l; set |= kLook; return *this; }
        ItemStyle& Glass(const GlassMaterial& m) { glass = m; set |= kGlassFields; return *this; }
        ItemStyle& Blur(float v) { glass.blur = v; set |= kBlur; return *this; }
        ItemStyle& Refraction(float v) { glass.refraction = v; set |= kRefraction; return *this; }
        ItemStyle& Bezel(float v) { glass.bezel = v; set |= kBezel; return *this; }
        ItemStyle& Dispersion(float v) { glass.dispersion = v; set |= kDispersion; return *this; }
        ItemStyle& Saturation(float v) { glass.saturation = v; set |= kSaturation; return *this; }
        ItemStyle& Brightness(float v) { glass.brightness = v; set |= kBrightness; return *this; }
        ItemStyle& Specular(float v) { glass.specular = v; set |= kSpecular; return *this; }
        ItemStyle& Legibility(float v) { glass.legibility = v; set |= kLegibility; return *this; }
        ItemStyle& Magnify(float v) { glass.magnify = v; set |= kMagnify; return *this; }
        ItemStyle& GlassTint(Color c) { glass.tint = c; set |= kGlassTint; return *this; }
        ItemStyle& Rim(Color c) { glass.rim = c; set |= kRim; return *this; }
        // ---- the widget
        ItemStyle& Tint(Color c) { tint = c; set |= kTint; return *this; }        // accent: filled button, switch on, slider, progress, check
        ItemStyle& Fill(Color c) { fill = c; set |= kFill; return *this; }        // its surface: button / field / track / card / window
        ItemStyle& Radius(float r) { radius = r; set |= kRadius; return *this; }  // corner radius (UI units)
        ItemStyle& Opacity(float o) { opacity = o; set |= kOpacity; return *this; }
        ItemStyle& Label(Color c) { label = c; set |= kLabel; return *this; }    // its text and symbols
        // ---- selections (segmented controls, tab bars): the selected item's background, the moving lens, its text
        ItemStyle& SelectedFill(Color c) { selectedFill = c; set |= kSelectedFill; return *this; }
        ItemStyle& MovingFill(Color c) { movingFill = c; set |= kMovingFill; return *this; }
        ItemStyle& SelectedLabel(Color c) { selectedLabel = c; set |= kSelectedLabel; return *this; }

        bool Has(std::uint32_t fields) const { return (set & fields) != 0; }
    };

    // The style of the next widget or container, edited in place.
    ESIA_API ItemStyle& Next();
    ESIA_API void SetNextItemStyle(const ItemStyle& style);

    // A block styled until the end of the scope (scopes nest, the innermost wins field by field).
    class ESIA_API StyleScope
    {
    public:
        explicit StyleScope(const ItemStyle& style);
        ~StyleScope();
        StyleScope(const StyleScope&) = delete;
        StyleScope& operator=(const StyleScope&) = delete;
    };

    ESIA_API ItemStyle CurrentItemStyle();                               // every scope merged (custom widgets)
    ESIA_API Color AccentColor();                                        // the style's tint, or the theme accent
    ESIA_API GlassLook CurrentGlassLook();
    ESIA_API GlassMaterial LookMaterial(const GlassMaterial& themed);    // a themed material as the style renders it

    // ================================================================ text
    // Text never runs past its container: wider than the room left on its line, it wraps at the container's edge.
    // Inside an auto-layout container the container sizes it.
    ESIA_API void Text(TextStyle style, const char* fmt, ...) ESIA_PRINTF(2, 3);
    ESIA_API void TextColored(TextStyle style, Color color, const char* fmt, ...) ESIA_PRINTF(3, 4);
    ESIA_API void TextSecondary(const char* fmt, ...) ESIA_PRINTF(1, 2);
    ESIA_API void TextWrapped(TextStyle style, Color color, std::string_view text);   // always wraps at the container's width
    ESIA_API void TextUnformatted(TextStyle style, Color color, std::string_view text);
    ESIA_API void LargeTitle(std::string_view text);
    ESIA_API void Headline(std::string_view text);
    ESIA_API void Spacer(float height = -1.0f);   // < 0 = the theme's spacing
    ESIA_API void Divider();

    // ============================================================ controls
    enum class ButtonKind : std::uint8_t
    {
        Filled,          // accent background, white label
        Tinted,          // translucent accent background, accent label
        Gray,            // neutral fill
        Plain,           // label only
        Glass,           // liquid glass
        GlassProminent,  // liquid glass tinted with the accent
        Destructive,     // red
    };
    enum class ControlSize : std::uint8_t { Small, Regular, Large };

    struct ButtonOptions
    {
        ButtonKind kind = ButtonKind::Filled;
        ControlSize size = ControlSize::Regular;
        Icon icon = 0;
        Color tint = Color::Clear();   // Clear = accent (red for Destructive)
        float width = 0.0f;            // 0 = fit the content, < 0 = fill the available width
        bool capsule = true;           // capsule, else a rounded rectangle
        bool glow = false;             // neon glow halo
    };
    struct SliderOptions
    {
        float width = -1.0f;           // < 0 = fill
        Icon minIcon = 0;
        Icon maxIcon = 0;
        Color tint = Color::Clear();
        float step = 0.0f;             // snap step (0 = continuous)
    };
    struct ToggleButtonOptions         // round Control Center toggle
    {
        Color tint = Color::Clear();   // "on" color (Clear = accent)
        float diameter = 58.0f;
        bool glow = true;
    };
    struct ProgressOptions
    {
        Color tint = Color::Clear();   // Clear = accent
        bool glow = false;
        float width = -1.0f;           // bar: < 0 = fill the available width
        float diameter = 0.0f;         // ring: 0 = 44
        float thickness = 0.0f;        // ring: 0 = in proportion to the diameter
    };

    // Labels may carry an id suffix: "OK##dialog" shows "OK", "##hidden" shows nothing.
    ESIA_API bool Button(std::string_view label, const ButtonOptions& options = {});
    ESIA_API bool IconButton(std::string_view id, Icon icon, const ButtonOptions& options = {});
    ESIA_API bool Toggle(std::string_view id, bool* value);
    ESIA_API bool Checkbox(std::string_view label, bool* value);
    ESIA_API bool ToggleButton(std::string_view id, bool* value, Icon icon, const ToggleButtonOptions& options = {});
    ESIA_API bool Slider(std::string_view id, float* value, float min, float max, const SliderOptions& options = {});
    ESIA_API bool Stepper(std::string_view id, int* value, int min, int max, int step = 1);
    ESIA_API void ProgressBar(float fraction, const ProgressOptions& options = {});
    ESIA_API void ProgressRing(float fraction, const ProgressOptions& options = {});
    ESIA_API void ActivityIndicator(float diameter = 0.0f, Color tint = Color::Clear());
    ESIA_API void Badge(std::string_view text, Color tint = Color::Clear());
    ESIA_API bool ColorSwatches(std::string_view id, int* selected, const Color* colors, int count, float diameter = 0.0f);
    ESIA_API void Image(TextureId texture, Vec2 size, float radius = -1.0f, Vec2 uv0 = Vec2(0, 0), Vec2 uv1 = Vec2(1, 1));

    // iOS segmented control: tap a segment or drag the selection, which travels as a clear lens and lands as a pill.
    // width: > 0 fixed, else the available width.
    //
    //   ui::Segmented("scale", &scale, {"Small", "Default", "Large"});
    ESIA_API bool Segmented(std::string_view id, int* selected, std::span<const std::string_view> items, float width = -1.0f);
    inline bool Segmented(std::string_view id, int* selected, std::initializer_list<std::string_view> items, float width = -1.0f)
    {
        return Segmented(id, selected, std::span<const std::string_view>(items.begin(), items.size()), width);
    }

    // ================================================= inset grouped lists
    // iOS Settings: sections of rows on a card, with an optional header and footer. A row's label takes what its
    // accessory (switch, value, slider ...) leaves; when even a short label does not fit beside it, the row stacks
    // them. Rows go between BeginSection and EndSection; a section is one item of its parent.
    //
    //   ui::BeginSection("Display", "HDR needs a display that supports it.");
    //   ui::RowToggle("HDR", &hdr, {ui::icons::Brightness, Color::Hex(0xFF9F0A)});
    //   ui::RowValue("Resolution", "2560 x 1440");
    //   if (ui::RowNavigation("Advanced")) ...;
    //   ui::EndSection();
    struct RowIcon
    {
        Icon icon = 0;
        Color color = Color::Clear();  // the tile's color (Clear = accent)
    };
    ESIA_API bool BeginSection(std::string_view header = {}, std::string_view footer = {});   // always true
    ESIA_API void EndSection();
    ESIA_API bool RowNavigation(std::string_view label, std::string_view detail = {}, RowIcon icon = {});   // pressed
    ESIA_API bool RowToggle(std::string_view label, bool* value, RowIcon icon = {});
    // format: printf for the value column (empty or null: none)
    ESIA_API bool RowSlider(std::string_view label, float* value, float min, float max, RowIcon icon = {}, const char* format = "%.0f");
    ESIA_API bool RowStepper(std::string_view label, int* value, int min, int max, RowIcon icon = {});
    ESIA_API bool RowSegmented(std::string_view label, int* selected, std::span<const std::string_view> items, RowIcon icon = {});
    inline bool RowSegmented(std::string_view label, int* selected, std::initializer_list<std::string_view> items, RowIcon icon = {})
    {
        return RowSegmented(label, selected, std::span<const std::string_view>(items.begin(), items.size()), icon);
    }
    ESIA_API void RowValue(std::string_view label, std::string_view value, RowIcon icon = {});
    ESIA_API bool RowButton(std::string_view label, bool destructive = false, RowIcon icon = {});   // pressed
    // A row of your own: `height` (0 = the theme's row height) and the content rect, inset like the labels; widgets
    // submitted before EndRow go in it. Call EndRow whatever BeginRow returns (false: the row is not visible).
    ESIA_API bool BeginRow(std::string_view id, float height = 0.0f, Rect* content = nullptr);
    ESIA_API void EndRow();

    // ========================================================== navigation
    // A navigation stack (iOS): a pushed page slides in from the right over the current one, a popped page slides out
    // and uncovers the one below. Each page has a title bar with a back button and scrolls on its own. Submit every
    // page every frame: BeginPage returns true for the page shown (and, during a transition, the one leaving).
    //
    //   ui::BeginNavigation("settings", "root");   // takes the room left in its area
    //   if (ui::BeginPage("root", "Settings")) {
    //       if (ui::RowNavigation("Accent Color")) ui::NavigationPush("accent");
    //       ui::EndPage();
    //   }
    //   if (ui::BeginPage("accent", "Accent Color")) { ...; ui::EndPage(); }
    //   ui::EndNavigation();
    ESIA_API bool BeginNavigation(std::string_view id, std::string_view rootPage);   // always true
    ESIA_API void EndNavigation();
    ESIA_API bool BeginPage(std::string_view pageId, std::string_view title);
    ESIA_API void EndPage();                                  // when BeginPage returned true
    ESIA_API void NavigationPush(std::string_view pageId);   // between BeginNavigation and EndNavigation
    ESIA_API void NavigationPop();

    // A liquid-glass tab bar floating over the bottom of the area it is submitted in (a window's content, a page): the
    // content scrolls under it. Tap a tab or drag the selection, as on a segmented control.
    struct TabItem
    {
        Icon icon = 0;
        std::string_view label;
    };
    ESIA_API bool TabBar(std::string_view id, int* selected, std::span<const TabItem> items);
    inline bool TabBar(std::string_view id, int* selected, std::initializer_list<TabItem> items)
    {
        return TabBar(id, selected, std::span<const TabItem>(items.begin(), items.size()));
    }

    // ============================================================= windows
    enum WindowFlags_ : std::uint32_t
    {
        WindowFlags_None = 0,
        WindowFlags_NoClose = 1u << 0,
        WindowFlags_NoResize = 1u << 1,
        WindowFlags_NoMove = 1u << 2,
        WindowFlags_NoHeader = 1u << 3,
        WindowFlags_NoScroll = 1u << 4,
        WindowFlags_Solid = 1u << 5,       // an opaque surface instead of glass
        WindowFlags_LargeTitle = 1u << 6,  // a big title in the content (iOS large title)
        WindowFlags_NoShadow = 1u << 7,
        WindowFlags_NoPadding = 1u << 8,
        WindowFlags_ClearGlass = 1u << 9,  // a fully transparent glass surface
    };
    struct WindowOptions
    {
        Vec2 size = Vec2(420, 540);
        Vec2 pos = Vec2(-1, -1);          // first-use position (< 0 = centered, cascading)
        std::uint32_t flags = WindowFlags_None;
        std::string_view subtitle;
        Icon icon = 0;
    };
    // A liquid-glass window with a header and a smooth-scrolling body. `open` (optional) gets a close button; the
    // window fades out before *open turns false. EndWindow only when BeginWindow returned true.
    ESIA_API bool BeginWindow(std::string_view title, bool* open = nullptr, const WindowOptions& options = {});
    ESIA_API void EndWindow();

    // A smooth-scrolling region with an auto-hiding indicator. size: > 0 fixed, 0 = what is available.
    ESIA_API bool BeginScrollArea(std::string_view id, Vec2 size = Vec2(0, 0));
    ESIA_API void EndScrollArea();

    struct CardOptions
    {
        bool glass = false;
        float radius = -1.0f;          // < 0 = the theme's card radius
        Color fill = Color::Clear();   // Clear = the theme's card surface
        bool shadow = true;
        float padding = -1.0f;         // < 0 = the theme's padding
    };
    // size: > 0 fixed, 0 = the available width and the content's height, x < 0 = available width + x.
    ESIA_API bool BeginCard(std::string_view id, Vec2 size = Vec2(0, 0), const CardOptions& options = {});
    ESIA_API void EndCard();

    // ========================================================= auto layout
    // Containers that size and place their children from the room they are given, so pages re-flow with the window.
    // Every widget (or nested container) submitted directly inside is one child; children are measured every frame,
    // placement uses the latest measurements and moves glide on a spring. Widgets that fill the available width
    // (sliders, width < 0 buttons, wrapped text ...) are flexible children.
    //
    //   ui::BeginFlow("buttons"); ui::Button("Play"); ui::Button("Settings"); ui::EndFlow();
    enum class Align : std::uint8_t
    {
        Start,
        Center,
        End,
        Stretch,   // cross axis: fill; main axis (justify): space between
    };
    struct StackOptions
    {
        float spacing = -1.0f;          // < 0 = the theme's spacing
        Align align = Align::Center;    // cross-axis alignment of the children
        Align justify = Align::Start;   // main-axis distribution when no child is flexible
        float minFillWidth = 120.0f;    // adaptive stack: the least width of a flexible child in a row
        bool animate = true;            // spring re-flow
    };
    struct GridOptions
    {
        float minColumnWidth = 160.0f;  // adaptive columns: as many as fit (cells stretch to fill)
        int columns = 0;                // > 0 = a fixed column count
        int maxColumns = 0;             // 0 = unlimited
        float spacing = -1.0f;          // column gap (< 0 = the theme's spacing)
        float rowSpacing = -1.0f;       // row gap (< 0 = spacing)
        float cellHeight = 0.0f;        // > 0 = a fixed row height; 0 = the tallest cell of the row
        float aspect = 0.0f;            // > 0 = cell width / height
        Align align = Align::Stretch;   // children smaller than their cell
        bool animate = true;
    };
    struct FlowOptions
    {
        float spacing = -1.0f;          // gap between items (< 0 = the theme's spacing)
        float lineSpacing = -1.0f;      // gap between lines (< 0 = spacing)
        Align justify = Align::Start;   // per line
        Align align = Align::Center;    // vertical alignment in a line
        bool animate = true;
    };
    ESIA_API void BeginVStack(std::string_view id, const StackOptions& options = {});
    ESIA_API void BeginHStack(std::string_view id, const StackOptions& options = {});
    // A row while the children fit side by side, a column otherwise (SwiftUI's ViewThatFits).
    ESIA_API void BeginAdaptiveStack(std::string_view id, const StackOptions& options = {});
    ESIA_API void EndStack();
    ESIA_API void BeginGrid(std::string_view id, const GridOptions& options = {});
    ESIA_API void EndGrid();
    ESIA_API void BeginFlow(std::string_view id, const FlowOptions& options = {});
    ESIA_API void EndFlow();
    // For the next child of the current container.
    ESIA_API void LayoutFlex(float flex);    // share of the free main-axis space (stacks)
    ESIA_API void LayoutSpan(int columns);   // grid column span
    ESIA_API void FlexSpacer();              // flexible empty space (pushes siblings apart in an HStack)

    enum class SizeClass : std::uint8_t
    {
        Compact,    // < 420: phone-like, one column
        Regular,    // < 760
        Expanded,   // wide panels, full screen
    };
    // The size class of a width (< 0: the width available at the cursor).
    ESIA_API SizeClass GetSizeClass(float width = -1.0f);

    // ====================================================== custom widgets
    struct Interaction
    {
        Id id = 0;
        Rect rect;
        bool visible = false;
        bool hovered = false;
        bool held = false;
        bool pressed = false;    // activated this frame
        float hover = 0.0f;      // spring-animated 0..1
        float press = 0.0f;      // spring-animated 0..1
        // The ui::Next() style this widget took: draw inside ui::StyleScope s(it.style) so AccentColor and
        // LookMaterial follow it.
        ItemStyle style;
    };
    enum InteractFlags_ : std::uint32_t
    {
        InteractFlags_None = 0,
        InteractFlags_PressOnClick = 1u << 0,   // fires on mouse down instead of release
        InteractFlags_Repeat = 1u << 1,         // repeats while held
        InteractFlags_Disabled = 1u << 2,
    };
    // Takes layout room of `size` at the cursor and runs press / hover logic with spring-animated states.
    ESIA_API Interaction Interact(std::string_view id, Vec2 size, std::uint32_t flags = InteractFlags_None);
    // The same at an explicit rect (no layout).
    ESIA_API Interaction InteractRect(Id id, const Rect& rect, std::uint32_t flags = InteractFlags_None);

    // A Painter for the current window, with the theme's corner smoothing, the window's pixel density, the style's
    // opacity and the Ui's text system.
    ESIA_API Painter GetPainter();
    // Theme-scaled value: S(12) == 12 * theme.metrics.scale.
    ESIA_API float S(float value);
    // The width available at the cursor. In an auto-layout container, asking for it makes the item a flexible child.
    ESIA_API float AvailableWidth();

    // Animation, keyed by id (Context::State): the value that springs toward `target`. `initial` seeds the first
    // frame (NaN: the target, no pop-in).
    ESIA_API float Anim(Id id, float target, const Spring& spring, float initial = NAN);
    ESIA_API float AnimVelocity(Id id);
    ESIA_API void AnimSet(Id id, float value, float velocity = 0.0f);   // jumps there (replay an appear animation)
    ESIA_API void AnimKick(Id id, float velocity);                     // an impulse (a pop on click)
    ESIA_API float Timer(Id id, float duration, bool restart);          // linear 0..1, restarted by `restart`
    ESIA_API double Time();                                            // seconds (the frame clock)
    ESIA_API float DeltaTime();
    // A sub-id of `id` for a widget's parts and animations.
    inline Id Salt(Id id, std::uint32_t salt) { return (id ^ (salt * 0x9E3779B9u)) * 16777619u; }
}
