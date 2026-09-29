// WGT UI - widgets
//
// Immediate-mode, like Dear ImGui, but every widget is drawn by WGT's Painter with the current
// Theme: liquid-glass windows and bars, inset-grouped lists, iOS switches, sliders with liquid
// knobs, segmented controls with morphing selection, navigation stacks, floating tab bars ...
//
// All functions must be called on the UI thread between Context::NewFrame() and Context::Render().
// Regular ImGui calls can be mixed freely (they are restyled by the theme as well).
#pragma once
#include "context.hpp"
#include "icons.hpp"
#include "anim.hpp"
#include "sync.hpp"

namespace wgt::ui
{
    // ================================================================ basics
    enum class ButtonKind : std::uint8_t
    {
        Filled,          // accent background, white label
        Tinted,          // translucent accent background, accent label
        Gray,            // neutral fill
        Plain,           // label only
        Glass,           // liquid glass
        GlassProminent,  // liquid glass tinted with the accent color
        Destructive,     // red
    };

    enum class ControlSize : std::uint8_t { Small, Regular, Large };

    struct ButtonOptions
    {
        ButtonKind kind = ButtonKind::Filled;
        ControlSize size = ControlSize::Regular;
        Icon icon = 0;
        Color tint = Color::Clear();   // Clear = accent (or red for Destructive)
        float width = 0.0f;            // 0 = fit content, < 0 = fill available width
        bool capsule = true;           // capsule vs rounded rectangle
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

    // Round Control Center toggle.
    struct ToggleButtonOptions
    {
        Color tint = Color::Clear();   // "on" color (Clear = accent)
        float diameter = 58.0f;        // UI units at scale 1
        bool glow = true;              // soft colored halo while on (kept off neighbours automatically)
    };

    // Progress bar / ring. No glow unless asked for.
    struct ProgressOptions
    {
        Color tint = Color::Clear();   // Clear = accent
        bool glow = false;             // soft halo in the tint color
        float width = -1.0f;           // bar: < 0 = fill the available width
        float diameter = 0.0f;         // ring: 0 = 44
        float thickness = 0.0f;        // ring: 0 = in proportion to the diameter
    };

    // Line chart (iOS Stocks / Health): a smooth line through the values over a soft gradient.
    struct LineChartOptions
    {
        float height = 90.0f;          // UI units
        float width = -1.0f;           // < 0 = fill the available width
        Rect rect = Rect();            // non-empty: draw into this rect (a row, a card) instead of taking a place in the layout
        Color tint = Color::Clear();   // Clear = accent
        float thickness = 2.0f;
        bool smooth = true;            // monotone curve through the values (never overshoots); false = straight segments
        bool fill = true;              // gradient area under the line
        bool glow = false;             // soft halo around the line
        bool lastPoint = true;         // dot on the latest value
        float min = 0.0f, max = 0.0f;  // value range; min == max = fit the data (starting at 0 for positive data), eased
        int offset = 0;                // ring buffers: index of the oldest value
    };

    struct TextFieldOptions
    {
        Icon icon = 0;
        float width = -1.0f;
        bool password = false;
        bool clearButton = true;
        bool background = true;        // false: no field fill (the field sits on glass or a custom surface)
    };

    struct RowIcon
    {
        Icon icon = 0;
        Color color = Color::Clear();  // tile color (Clear = accent)
    };

    struct TabItem
    {
        const char* label = nullptr;
        Icon icon = 0;
    };

    // ======================================================= per-component style
    // Style any single component, or everything in a scope, without touching the theme. Only the fields you set
    // change; everything else follows the theme (and any enclosing scope - innermost wins, field by field).
    //
    //   ui::Next().Tint(Color::Hex(0x30D158)).Radius(8);    // the next component only: one line, no Push/Pop
    //   ui::Button("Play");
    //   ui::Next().Look(GlassLook::Frosted).Blur(24);        // a flat control (filled button, switch track,
    //   ui::Toggle("wifi", &wifi);                           //   field ...) becomes that glass
    //   ui::Next().Look(GlassLook::Clear);                   // fully transparent
    //   ui::Slider("vol", &vol, 0, 1);
    //   ui::Next().Fill(blue).SelectedFill(white).MovingFill(white.Fade(0.3f)).SelectedLabel(blue);
    //   ui::Segmented("range", &range, items, 3);            // track, selection, lens and text colors
    //   { ui::StyleScope s(ui::ItemStyle().Tint(red).Opacity(0.8f)); ... }   // a scope (RAII)
    //
    // Before a window / section / card / BeginRow it styles that container and everything inside it.
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

        // ---- glass. Look: Theme / Clear (fully transparent) / Frosted (iOS frost). On a flat component, Clear,
        // Frosted or any glass setting turns its surface into that glass (its fill color tints the glass).
        ItemStyle& Look(GlassLook l) { look = l; set |= kLook; return *this; }
        ItemStyle& Glass(const GlassMaterial& m) { glass = m; set |= kGlassFields; return *this; }   // the whole material
        ItemStyle& Blur(float v) { glass.blur = v; set |= kBlur; return *this; }                     // frost radius
        ItemStyle& Refraction(float v) { glass.refraction = v; set |= kRefraction; return *this; }   // lensing strength
        ItemStyle& Bezel(float v) { glass.bezel = v; set |= kBezel; return *this; }                  // width of the curved edge
        ItemStyle& Dispersion(float v) { glass.dispersion = v; set |= kDispersion; return *this; }   // rainbow at the rim (0..1)
        ItemStyle& Saturation(float v) { glass.saturation = v; set |= kSaturation; return *this; }
        ItemStyle& Brightness(float v) { glass.brightness = v; set |= kBrightness; return *this; }
        ItemStyle& Specular(float v) { glass.specular = v; set |= kSpecular; return *this; }         // rim highlight
        ItemStyle& Legibility(float v) { glass.legibility = v; set |= kLegibility; return *this; }
        ItemStyle& Magnify(float v) { glass.magnify = v; set |= kMagnify; return *this; }            // loupe
        ItemStyle& GlassTint(Color c) { glass.tint = c; set |= kGlassTint; return *this; }            // a = amount
        ItemStyle& Rim(Color c) { glass.rim = c; set |= kRim; return *this; }                        // hairline
        // ---- the component
        ItemStyle& Tint(Color c) { tint = c; set |= kTint; return *this; }        // accent: filled button, switch on, slider, progress, check, caret, selected tab
        ItemStyle& Fill(Color c) { fill = c; set |= kFill; return *this; }        // its surface: button / field / track / section / card / window background
        ItemStyle& Radius(float r) { radius = r; set |= kRadius; return *this; }  // corner radius (UI units)
        ItemStyle& Opacity(float o) { opacity = o; set |= kOpacity; return *this; }
        ItemStyle& Label(Color c) { label = c; set |= kLabel; return *this; }    // its text and symbols
        // ---- selections (segmented control, tab bar): the track is Fill; the selected item's background once a
        // switch has landed, the selection while it travels (the liquid lens; a = how strongly it is tinted), and
        // the selected item's text
        ItemStyle& SelectedFill(Color c) { selectedFill = c; set |= kSelectedFill; return *this; }
        ItemStyle& MovingFill(Color c) { movingFill = c; set |= kMovingFill; return *this; }
        ItemStyle& SelectedLabel(Color c) { selectedLabel = c; set |= kSelectedLabel; return *this; }
        bool Has(std::uint32_t fields) const { return (set & fields) != 0; }
    };

    // Custom-widget helper result.
    struct Interaction
    {
        ImGuiID id = 0;
        Rect rect;
        bool visible = false;
        bool hovered = false;
        bool held = false;
        bool pressed = false;   // clicked this frame
        float hover = 0.0f;     // spring-animated 0..1
        float press = 0.0f;     // spring-animated 0..1
        // The ui::Next() style this component took. Draw inside ui::StyleScope s(it.style) so LookMaterial,
        // AccentColor and CurrentItemStyle follow it.
        ItemStyle style;
    };

    enum InteractFlags_ : std::uint32_t
    {
        InteractFlags_None = 0,
        InteractFlags_NoLayout = 1u << 0,      // InteractRect only
        InteractFlags_PressOnClick = 1u << 1,  // fire on mouse down instead of release
        InteractFlags_AllowOverlap = 1u << 2,
        InteractFlags_Repeat = 1u << 3,        // repeat while held
    };

    // ================================================================= text
    // Text never runs past its container: a line wider than the room left (after SameLine items too) wraps at
    // the container's edge. Inside auto-layout containers the container sizes it. TextWrapped always wraps.
    WGT_API void Text(TextStyle style, const char* fmt, ...) IM_FMTARGS(2);
    WGT_API void TextColored(TextStyle style, Color color, const char* fmt, ...) IM_FMTARGS(3);
    WGT_API void TextSecondary(const char* fmt, ...) IM_FMTARGS(1);
    WGT_API void TextWrapped(TextStyle style, Color color, const char* text);
    WGT_API void LargeTitle(const char* text);
    WGT_API void Headline(const char* text);
    WGT_API void Spacer(float height = -1.0f);
    WGT_API void Divider();

    // ============================================================= controls
    WGT_API bool Button(const char* label, const ButtonOptions& options = {});
    WGT_API bool IconButton(const char* id, Icon icon, const ButtonOptions& options = {});
    WGT_API bool Toggle(const char* id, bool* value);
    WGT_API bool Checkbox(const char* label, bool* value);
    // Round glass toggle (iOS Control Center): press glow under the pointer, the tint floods in from the
    // press point, springy pop, bouncing symbol.
    WGT_API bool ToggleButton(const char* id, bool* value, Icon icon, const ToggleButtonOptions& options = {});
    WGT_API bool Slider(const char* id, float* value, float min, float max, const SliderOptions& options = {});
    WGT_API bool Stepper(const char* id, int* value, int min, int max, int step = 1);
    WGT_API bool Segmented(const char* id, int* selected, const char* const* items, int count, float width = -1.0f);
    WGT_API bool TextField(const char* id, char* buffer, std::size_t bufferSize, const char* placeholder = nullptr, const TextFieldOptions& options = {});
    WGT_API bool SearchField(const char* id, char* buffer, std::size_t bufferSize, const char* placeholder = "Search");
    WGT_API bool Picker(const char* id, int* selected, const char* const* items, int count, float width = 0.0f);
    WGT_API void ProgressBar(float fraction, float width = -1.0f, Color tint = Color::Clear());
    WGT_API void ProgressBar(float fraction, const ProgressOptions& options);
    WGT_API void ProgressRing(float fraction, float diameter = 0.0f, float thickness = 0.0f, Color tint = Color::Clear());
    WGT_API void ProgressRing(float fraction, const ProgressOptions& options);
    WGT_API void LineChart(const char* id, const float* values, int count, const LineChartOptions& options = {});
    WGT_API void ActivityIndicator(float diameter = 0.0f, Color tint = Color::Clear());
    WGT_API void Badge(const char* text, Color tint = Color::Clear());
    WGT_API void Image(ImTextureID texture, Vec2 size, float radius = -1.0f, Vec2 uv0 = Vec2(0, 0), Vec2 uv1 = Vec2(1, 1));
    WGT_API bool ColorSwatches(const char* id, int* selected, const Color* colors, int count, float diameter = 0.0f);
    // Tooltip for the previous item (glass bubble, appears after a short hover).
    WGT_API void Tooltip(const char* text);

    // Thread-safe Property<T> overloads (read -> edit -> write back if changed).
    inline bool Toggle(const char* id, Property<bool>& p) { bool v = p.Get(); if (Toggle(id, &v)) { p.Set(v); return true; } return false; }
    inline bool Slider(const char* id, Property<float>& p, float mn, float mx, const SliderOptions& o = {}) { float v = p.Get(); if (Slider(id, &v, mn, mx, o)) { p.Set(v); return true; } return false; }
    inline bool Stepper(const char* id, Property<int>& p, int mn, int mx, int step = 1) { int v = p.Get(); if (Stepper(id, &v, mn, mx, step)) { p.Set(v); return true; } return false; }

    // ============================================================== windows
    enum WindowFlags_ : std::uint32_t
    {
        WindowFlags_None = 0,
        WindowFlags_NoClose = 1u << 0,
        WindowFlags_NoResize = 1u << 1,
        WindowFlags_NoMove = 1u << 2,
        WindowFlags_NoHeader = 1u << 3,
        WindowFlags_NoScroll = 1u << 4,
        WindowFlags_Solid = 1u << 5,       // opaque surface instead of glass
        WindowFlags_LargeTitle = 1u << 6,  // big title in the content (iOS large title)
        WindowFlags_NoShadow = 1u << 7,
        WindowFlags_NoPadding = 1u << 8,
        WindowFlags_ClearGlass = 1u << 9,  // fully transparent glass surface (no frost, no tint)
    };

    struct WindowOptions
    {
        Vec2 size = Vec2(420, 540);
        Vec2 pos = Vec2(-1, -1);            // first-use position (-1 = centered cascade)
        std::uint32_t flags = WindowFlags_None;
        const char* subtitle = nullptr;
        Icon icon = 0;
    };

    // Liquid-glass window. Always call EndWindow() when BeginWindow() returned true.
    WGT_API bool BeginWindow(const char* title, bool* open = nullptr, const WindowOptions& options = {});
    WGT_API void EndWindow();

    // ===================================================== inset grouped lists
    WGT_API bool BeginSection(const char* header = nullptr, const char* footer = nullptr);
    WGT_API void EndSection();
    WGT_API bool RowNavigation(const char* label, const char* detail = nullptr, RowIcon icon = {});
    WGT_API bool RowToggle(const char* label, bool* value, RowIcon icon = {});
    WGT_API bool RowSlider(const char* label, float* value, float min, float max, RowIcon icon = {}, const char* format = "%.0f");
    WGT_API bool RowStepper(const char* label, int* value, int min, int max, RowIcon icon = {});
    WGT_API bool RowSegmented(const char* label, int* selected, const char* const* items, int count, RowIcon icon = {});
    WGT_API bool RowPicker(const char* label, int* selected, const char* const* items, int count, RowIcon icon = {});
    WGT_API void RowValue(const char* label, const char* value, RowIcon icon = {});
    WGT_API bool RowButton(const char* label, bool destructive = false, RowIcon icon = {});
    // Custom row: returns the content rect; draw inside, then EndRow().
    WGT_API bool BeginRow(const char* id, float height = 0.0f, Rect* outContent = nullptr);
    WGT_API void EndRow();

    inline bool RowToggle(const char* label, Property<bool>& p, RowIcon icon = {}) { bool v = p.Get(); if (RowToggle(label, &v, icon)) { p.Set(v); return true; } return false; }
    inline bool RowSlider(const char* label, Property<float>& p, float mn, float mx, RowIcon icon = {}, const char* fmt = "%.0f") { float v = p.Get(); if (RowSlider(label, &v, mn, mx, icon, fmt)) { p.Set(v); return true; } return false; }

    // ================================================================ cards
    struct CardOptions
    {
        bool glass = false;
        float radius = -1.0f;
        Color fill = Color::Clear();   // Clear = theme card surface
        bool shadow = true;
        float padding = -1.0f;
    };
    WGT_API bool BeginCard(const char* id, Vec2 size = Vec2(0, 0), const CardOptions& options = {});
    WGT_API void EndCard();

    // =========================================================== navigation
    // Stack navigation with iOS push / pop transitions and a back button.
    //   if (ui::BeginNavigation("settings", "root")) {
    //       if (ui::BeginPage("root", "Settings")) { if (ui::RowNavigation("General")) ui::NavigationPush("general"); ui::EndPage(); }
    //       if (ui::BeginPage("general", "General")) { ...; ui::EndPage(); }
    //       ui::EndNavigation();
    //   }
    WGT_API bool BeginNavigation(const char* id, const char* rootPage);
    WGT_API void EndNavigation();
    WGT_API bool BeginPage(const char* pageId, const char* title);
    WGT_API void EndPage();
    WGT_API void NavigationPush(const char* pageId);
    WGT_API void NavigationPop();

    // Floating liquid-glass tab bar (iOS 26+ style). Returns true when the selection changed.
    WGT_API bool TabBar(const char* id, int* selected, const TabItem* items, int count);
    // Floating search field pinned to the bottom of the current page / scroll area (iOS Settings): clear liquid
    // glass, no frost, no tint. Content scrolls under it. Returns true when the text changed.
    struct SearchBarOptions
    {
        const char* placeholder = "Search";
        GlassLook look = GlassLook::Clear;   // the bar's glass: Clear (no frost, no tint), Frosted, or Theme (the bar material)
        bool base = true;                    // a soft base color inside the field (false: nothing but the glass)
        Color fill = Color::Clear();         // the base color (Clear = the theme's: light white / dark gray)
        // A Clear look from ui::SetNextItemLook or a scope makes the whole bar transparent, base included.
    };
    WGT_API bool SearchBar(const char* id, char* buffer, std::size_t bufferSize, const SearchBarOptions& options);
    inline bool SearchBar(const char* id, char* buffer, std::size_t bufferSize, const char* placeholder = "Search")
    {
        SearchBarOptions o;
        o.placeholder = placeholder;
        return SearchBar(id, buffer, bufferSize, o);
    }

    // Smooth-scrolling region with an auto-hiding overlay indicator and drag-to-scroll.
    WGT_API bool BeginScrollArea(const char* id, Vec2 size = Vec2(0, 0));
    WGT_API void EndScrollArea();

    // ================================================== per-component style (see ItemStyle)
    // The style of the next component, edited in place: ui::Next().Tint(red).Radius(8); ui::Button("OK");
    WGT_API ItemStyle& Next();
    WGT_API void SetNextItemStyle(const ItemStyle& style);   // replaces it
    // Everything until PopItemStyle() (scopes nest; innermost wins field by field).
    WGT_API void PushItemStyle(const ItemStyle& style);
    WGT_API void PopItemStyle();
    struct StyleScope
    {
        explicit StyleScope(const ItemStyle& style) { PushItemStyle(style); }
        ~StyleScope() { PopItemStyle(); }
        StyleScope(const StyleScope&) = delete;
        StyleScope& operator=(const StyleScope&) = delete;
    };
    // The style in effect (all scopes merged) - for custom widgets.
    WGT_API ItemStyle CurrentItemStyle();
    // Its tint, or the theme accent.
    WGT_API Color AccentColor();
    // Shorthands for the look alone: Next().Look(look), PushItemStyle(ItemStyle().Look(look)), PopItemStyle().
    // Under GlassLook::Clear every component is fully transparent: its surfaces turn into clear glass (lens, rim
    // light, hairline; no frost, no fill) and state colors ("on", selection, progress) become see-through.
    // Default for the whole UI: Context::SetGlassLook (Frosted).
    WGT_API void SetNextItemLook(GlassLook look);
    WGT_API void PushGlassLook(GlassLook look);
    WGT_API void PopGlassLook();
    WGT_API GlassLook CurrentGlassLook();
    // A themed material as the current style renders it: look plus any glass fields set (custom widgets).
    WGT_API GlassMaterial LookMaterial(const GlassMaterial& themed);

    // ========================================================== auto layout
    // Containers that size and place their children from the space they are given (window, card, cell),
    // so pages re-flow with the window size. Children are detected automatically: every widget (or
    // ImGui::BeginGroup/EndGroup block, or nested container) submitted directly inside is one child.
    // Children are measured every frame; placement uses the latest measurements and re-flows glide with
    // a spring. Widgets that stretch to the available width (sliders, fields, width = -1 buttons ...)
    // are flexible children automatically. Glowing children get extra room so glows never cover a
    // neighbour.
    //
    //   ui::BeginFlow("buttons");                 // wraps like text
    //   ui::Button("Play"); ui::Button("Settings"); ui::Button("Quit");
    //   ui::EndFlow();
    //
    //   ui::GridOptions g; g.minColumnWidth = 150;  // as many columns as fit
    //   ui::BeginGrid("tiles", g); for (...) Tile(...); ui::EndGrid();
    enum class Align : std::uint8_t
    {
        Start,
        Center,
        End,
        Stretch,   // cross axis: fill / top-left; main axis (justify): space between
    };

    struct StackOptions
    {
        float spacing = -1.0f;          // < 0 = theme spacing
        Align align = Align::Center;    // cross-axis alignment of children
        Align justify = Align::Start;   // main-axis distribution when no child is flexible
        float minFillWidth = 120.0f;    // adaptive stack: minimum width of a flexible child in a row
        bool animate = true;            // spring re-flow
    };

    struct GridOptions
    {
        float minColumnWidth = 160.0f;  // adaptive columns: as many as fit (cells stretch to fill)
        int columns = 0;                // > 0 = fixed column count
        int maxColumns = 0;             // 0 = unlimited
        float spacing = -1.0f;          // column gap (< 0 = theme spacing)
        float rowSpacing = -1.0f;       // row gap (< 0 = spacing)
        float cellHeight = 0.0f;        // > 0 = fixed row height; 0 = tallest cell of the row
        float aspect = 0.0f;            // > 0 = cell width / height
        Align align = Align::Stretch;   // placement of children smaller than their cell
        bool animate = true;
    };

    struct FlowOptions
    {
        float spacing = -1.0f;          // gap between items (< 0 = theme spacing)
        float lineSpacing = -1.0f;      // gap between lines (< 0 = spacing)
        Align justify = Align::Start;   // per line
        Align align = Align::Center;    // vertical alignment inside a line
        bool animate = true;
    };

    WGT_API bool BeginVStack(const char* id, const StackOptions& options = {});
    WGT_API bool BeginHStack(const char* id, const StackOptions& options = {});
    // Horizontal while the children fit side by side, vertical otherwise (SwiftUI ViewThatFits).
    WGT_API bool BeginAdaptiveStack(const char* id, const StackOptions& options = {});
    WGT_API void EndStack();
    WGT_API bool BeginGrid(const char* id, const GridOptions& options = {});
    WGT_API void EndGrid();
    WGT_API bool BeginFlow(const char* id, const FlowOptions& options = {});
    WGT_API void EndFlow();
    // Options for the next child of the current container.
    WGT_API void LayoutFlex(float flex);    // share of the free main-axis space (stacks)
    WGT_API void LayoutSpan(int columns);   // grid column span
    // Flexible empty space (pushes siblings apart inside an HStack).
    WGT_API void FlexSpacer();

    // Size class of a width (default: the space available at the cursor), in design units.
    enum class SizeClass : std::uint8_t
    {
        Compact,    // < 420: phone-like, one column
        Regular,    // < 760
        Expanded,   // wide panels / full screen
    };
    WGT_API SizeClass GetSizeClass(float width = -1.0f);

    // ======================================================= custom widgets
    // Allocates layout space of `size` and runs press/hover logic with spring-animated states.
    WGT_API Interaction Interact(const char* id, Vec2 size, std::uint32_t flags = InteractFlags_None);
    // Same for an explicit rect (no layout).
    WGT_API Interaction InteractRect(ImGuiID id, const Rect& rect, std::uint32_t flags = InteractFlags_None);
    // Theme-scaled value: S(12) == 12 * theme.metrics.scale
    WGT_API float S(float value);
    // Content width available at the cursor. Inside auto-layout containers, asking for it makes the item a
    // flexible child (it fills the space the container offers).
    WGT_API float AvailableWidth();
}
