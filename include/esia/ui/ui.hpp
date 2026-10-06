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
#include <functional>
#include <initializer_list>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace esia::ui
{
    // ============================================================== the Ui
    // A notification the island shows (Ui::Notify).
    struct Notification
    {
        std::string title;
        std::string message;
        Icon icon = 0;                 // 0 = a bell
        Color tint = Color::Clear();   // Clear = accent
        float duration = 3.5f;         // seconds on screen
    };

    // How much room the widgets take (Ui::SetDensity), as WinUI's compact sizing and Material's density: the text
    // keeps its size; Compact makes the controls shorter (buttons 36 -> 28, fields 32 -> 26, text fields 38 -> 28,
    // menu and table rows 34 -> 28), list rows 44 -> 32, window headers 54 -> 40, padding 16 -> 12, spacing 10 -> 6.
    enum class Density : std::uint8_t { Regular, Compact };

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
        Density density = Density::Regular;   // Compact: a desktop tool's sizes (one line: .density = Density::Compact)
        GlassLook glassLook = GlassLook::Frosted;
        // Flat: no glass, shadows or glows - solid windows and surfaces - and the shapes drawn as geometry that
        // batches with the text (PainterEnv::flat). The cheapest frames: for a tool on a weak GPU, or where the UI
        // is not what users look at.
        bool flat = false;
        // Draw the island (notifications, live activities) at the top of the display at EndFrame.
        bool island = true;
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

        const Theme& GetTheme() const;   // the one in effect (mid-transition: the blend); its sizes before Density
        // Keeps the density (metrics.compact is the Ui's).
        void SetTheme(const Theme& theme, bool animate = true);
        // The built-in light / dark colors and materials, with the accent, metrics (scale), type and motion kept.
        void SetDarkMode(bool dark, bool animate = true);
        // Regular or Compact sizes for every widget; kept across SetTheme and SetDarkMode, animated like them.
        void SetDensity(Density density, bool animate = true);
        Density GetDensity() const;
        void SetAccent(Color accent);                       // Clear: the theme's own
        void SetGlassLook(GlassLook look);
        GlassLook GetGlassLook() const;
        void SetFlat(bool flat);   // UiDesc::flat
        bool GetFlat() const;

        text::FontRef Font(TextStyle style) const;
        text::FontRef Font(FontWeight weight, float size) const;   // size in UI units at metrics scale 1
        text::FontRef IconFont(float size) const;                  // id 0 when no icon font was found

        // Something moved this frame (springs, the theme): an event-driven host renders another frame.
        bool Animating() const;

        // ---- the island (any thread): a notification opens it for its duration, one after the other; live
        // activities (a download ...) stay in it until cleared. progress < 0: indeterminate.
        void Notify(const Notification& notification);
        void SetActivity(std::string_view id, std::string_view title, float progress, Icon icon = 0, Color tint = Color::Clear());
        void ClearActivity(std::string_view id);

        struct Impl;
        Impl& GetImpl() const { return *impl_; }

    private:
        std::unique_ptr<Impl> impl_;
    };

    // The current Ui (between NewFrame and EndFrame), or null.
    ESIA_API Ui* Current();

    // An id scope for the widgets submitted while it lives (inside one window): the same label in two scopes is two
    // widgets. Widgets take their ids from their labels, so widgets made in a loop need one - the loop's index, an
    // object's address, a name. Two items with one id share the hover, presses and state; a debug build reports
    // them (ContextDesc::debugChecks).
    //
    //   for (int i = 0; i < (int)items.size(); ++i) {
    //       ui::IdScope scope(i);
    //       if (ui::Button("Set value")) items[i].Set();
    //   }
    class ESIA_API IdScope
    {
    public:
        explicit IdScope(std::string_view key);
        explicit IdScope(const char* key) : IdScope(std::string_view(key)) {}
        explicit IdScope(std::int64_t key);
        template <class T>
            requires std::is_integral_v<T>
        explicit IdScope(T key) : IdScope((std::int64_t)key) {}
        explicit IdScope(const void* key);
        ~IdScope();
        IdScope(const IdScope&) = delete;
        IdScope& operator=(const IdScope&) = delete;
    };

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

    // Alignment in containers and table columns.
    enum class Align : std::uint8_t
    {
        Start,
        Center,
        End,
        Stretch,   // cross axis: fill; main axis (justify): space between
    };

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

    // ================================================================ dock
    struct DockItem
    {
        std::string_view label;
        Icon icon = 0;
        Color color = Color::Clear();  // the tile's color (Clear = accent)
        bool* open = nullptr;          // a click toggles it; a dot under the tile while it is true
    };
    // A glass dock along the bottom of the display, above the windows (macOS): its tiles magnify under the mouse and
    // show their label. Returns the index of the tile clicked this frame, else -1.
    ESIA_API int Dock(std::span<const DockItem> items);
    inline int Dock(std::initializer_list<DockItem> items) { return Dock(std::span<const DockItem>(items.begin(), items.size())); }

    // ================================================ menus, pickers, tips
    // A glass menu (iOS pull-down / context menu). OpenMenu when it should open (a click), BeginMenu every frame: it is
    // true while the menu is open. Choosing an item, a click outside or Escape closes it.
    //
    //   if (ui::IconButton("more", ui::icons::More)) ui::OpenMenu("actions");
    //   if (ui::BeginMenu("actions")) {
    //       if (ui::MenuItem("Rename", ui::icons::Edit)) ...;
    //       if (ui::MenuItem("Delete", ui::icons::Delete)) ...;
    //       ui::EndMenu();
    //   }
    struct MenuOptions
    {
        Vec2 anchor = Vec2(-1, -1);    // where it opens (< 0: where the mouse was at OpenMenu)
        Vec2 pivot = Vec2(0, 0);       // the point of the menu on the anchor (0,0 top-left, 1,0 top-right)
        float minWidth = 180.0f;
        float maxHeight = 0.0f;        // > 0: items past this height scroll inside it; 0 = what fits on the display
    };
    ESIA_API void OpenMenu(std::string_view id);
    ESIA_API bool BeginMenu(std::string_view id, const MenuOptions& options = {});
    ESIA_API bool MenuItem(std::string_view label, Icon icon = 0, bool checked = false);   // chosen (closes the menu)
    ESIA_API void EndMenu();                                                               // when BeginMenu returned true

    // A pop-up button showing the choice, with a menu of the choices (iOS). True when the choice changed. A long list
    // scrolls inside the menu, opened at the current choice.
    struct PickerOptions
    {
        float width = 0.0f;            // > 0 fixed, < 0 the available width, 0 = the widest choice
        int maxRows = 10;              // the choices the menu shows at most (more scroll); 0 = what fits on the display
    };
    ESIA_API bool Picker(std::string_view id, int* selected, std::span<const std::string_view> items, const PickerOptions& options);
    inline bool Picker(std::string_view id, int* selected, std::span<const std::string_view> items, float width = 0.0f)
    {
        return Picker(id, selected, items, PickerOptions{.width = width});
    }
    inline bool Picker(std::string_view id, int* selected, std::initializer_list<std::string_view> items, float width = 0.0f)
    {
        return Picker(id, selected, std::span<const std::string_view>(items.begin(), items.size()), width);
    }

    // A glass tip for the widget before it, after half a second of hover.
    ESIA_API void Tooltip(std::string_view text);

    // ========================================================= text fields
    struct TextFieldOptions
    {
        float width = 0.0f;            // > 0 fixed, else the available width
        Icon icon = 0;                 // a symbol before the text (SearchField: Search)
        bool password = false;         // bullets; nothing is copied, no IME
        bool clearButton = true;       // an x while there is text
        bool background = true;        // false: no fill (the field sits on glass or a surface of its own)
        std::size_t maxBytes = 0;      // > 0: the longest text it takes (UTF-8 bytes)
        bool selectOnFocus = false;    // taking the keyboard selects all the text (number fields)
    };
    struct TextFieldResult
    {
        bool changed = false;          // the text changed this frame
        bool submitted = false;        // Enter was pressed (the field let go of the keyboard)
        explicit operator bool() const { return changed; }
    };
    // A single-line editor: any script the text system shapes; the caret moves by graphemes, by words with Ctrl
    // (Option on macOS); selection by drag, double click (a word), triple click (all) and Shift; clipboard; undo and
    // redo; the IME's composition in place, its candidate window at the caret. A click or Tab gives it the keyboard,
    // Enter or Escape (or a click elsewhere) takes it away. Every change is written to *text at once.
    //
    //   if (ui::TextField("name", &name, "Your name")) ...;
    ESIA_API TextFieldResult TextField(std::string_view id, std::string* text, std::string_view placeholder = {}, const TextFieldOptions& options = {});
    ESIA_API TextFieldResult SearchField(std::string_view id, std::string* text, std::string_view placeholder = "Search");

    // A search field in a glass bar floating over the bottom of the area it is submitted in, as the tab bar (iOS
    // Settings): the content scrolls under it. Submit it last in its area.
    struct SearchBarOptions
    {
        std::string_view placeholder = "Search";
        GlassLook look = GlassLook::Clear;   // the bar's glass: Clear (no frost, no tint), Frosted, or Theme (the bar material)
        bool base = true;                    // a soft base color inside the glass (false: nothing but the glass)
        Color fill = Color::Clear();         // the base color (Clear = the theme's: light white / dark gray)
    };
    ESIA_API TextFieldResult SearchBar(std::string_view id, std::string* text, const SearchBarOptions& options = {});

    // A multi-line editor: everything the text field does, over lines - Enter starts a new one, Up / Down and Page Up /
    // Down move by visual line, Home / End go to the line's ends (Ctrl: the text's), long lines wrap at the width (or
    // scroll sideways). It scrolls inside its height and draws only the lines in view.
    //
    //   ui::TextEditor("notes", &notes, {.size = {0, 240}, .lineNumbers = true});
    struct TextEditorOptions
    {
        Vec2 size = Vec2(0, 200);      // x: > 0 fixed, else the available width; y: the height (it scrolls inside)
        bool wrap = true;              // long lines wrap at the width (false: the view scrolls sideways)
        bool readOnly = false;         // selectable and copyable, not editable
        bool lineNumbers = false;      // a gutter with the line numbers
        bool monospace = false;        // the monospace font (code, logs)
        bool tabInput = false;         // Tab types a tab (otherwise it moves the keyboard focus)
        std::size_t maxBytes = 0;      // > 0: the longest text it takes (UTF-8 bytes)
    };
    ESIA_API TextFieldResult TextEditor(std::string_view id, std::string* text, const TextEditorOptions& options = {});

    // ======================================================= number fields
    // A number in a field: drag it sideways to scrub the value, click it to type one (an expression such as 2*(3+4)
    // works; Enter or a click elsewhere takes it, Escape keeps the old value), Up / Down step while typing. With both
    // limits finite the field fills in proportion to the value. True when the value changed.
    //
    //   ui::NumberField("speed", &speed, {.min = 0, .max = 10, .step = 0.1, .format = "%.1f m/s"});
    struct NumberOptions
    {
        double min = -INFINITY, max = INFINITY;
        double step = 0.0;             // Up / Down and the buttons; 0 = 1 for integers, else a hundredth of a finite range, else 0.1
        double speed = 0.0;            // value per UI unit dragged; 0 = from the range (or the step)
        const char* format = nullptr;  // printf of the value (units may follow it); null = "%d" / "%.3f" trimmed
        float width = 0.0f;            // > 0 fixed, else the available width
        std::string_view label;        // a short label inside the field, on its left ("X")
        Color labelColor = Color::Clear();   // its color (Clear = the secondary label color)
        bool buttons = false;          // - and + at the field's ends
    };
    ESIA_API bool NumberField(std::string_view id, float* value, const NumberOptions& options = {});
    ESIA_API bool NumberField(std::string_view id, double* value, const NumberOptions& options = {});
    ESIA_API bool NumberField(std::string_view id, int* value, const NumberOptions& options = {});
    // 2 - 4 numbers side by side (a position, a size, a color), labeled X Y Z W in red, green, blue and gray.
    ESIA_API bool VectorField(std::string_view id, float* values, int count, const NumberOptions& options = {});

    // ============================================================= colors
    struct ColorPickerOptions
    {
        bool alpha = true;             // an opacity bar, and the alpha in the hex text
        bool hex = true;               // the hex field (#RRGGBB, #RRGGBBAA)
        std::span<const Color> swatches;   // colors picked with one click (empty: none)
        float width = 0.0f;            // > 0 fixed, else the available width (at most 340)
    };
    // A spectrum of saturation and brightness over a hue bar, an opacity bar, the hex value and swatches (iOS's color
    // picker). True when the color changed.
    ESIA_API bool ColorPicker(std::string_view id, Color* color, const ColorPickerOptions& options = {});
    // A round color well (UIColorWell): a click opens the picker in a glass popover.
    ESIA_API bool ColorWell(std::string_view id, Color* color, const ColorPickerOptions& options = {}, float diameter = 0.0f);

    // ====================================================== tables and trees
    enum TableFlags_ : std::uint32_t
    {
        TableFlags_None = 0,
        TableFlags_Sortable = 1u << 0,      // a click on a header sorts by it (TableSortSpec)
        TableFlags_Resizable = 1u << 1,     // the lines between the headers drag
        TableFlags_Selectable = 1u << 2,    // a click selects a row (TableOptions::selection); Ctrl adds, Shift extends
        TableFlags_Striped = 1u << 3,       // every other row tinted
        TableFlags_NoHeader = 1u << 4,
        TableFlags_ColumnLines = 1u << 5,   // hairlines between the columns
    };
    struct TableColumn
    {
        std::string_view label;
        float width = 0.0f;            // > 0 fixed (UI units); 0 = a share of what the fixed columns leave
        float weight = 1.0f;           // that share
        Align align = Align::Start;    // the header's and TableCell's alignment
        bool sortable = true;
    };
    struct TableOptions
    {
        std::uint32_t flags = TableFlags_None;
        float height = 0.0f;           // > 0: rows scroll inside this height under the header; 0 = as tall as the rows
        // > 0: as tall as the rows up to this height (the header included), then they scroll inside it; with a
        // height too, the smaller of the two. Its row count comes from TableVisible (without it: last frame's rows).
        float maxHeight = 0.0f;
        float rowHeight = 0.0f;        // 0 = 34
        std::vector<int>* selection = nullptr;   // Selectable: the selected row indices, sorted
        // Selectable, for rows that move (sorted, filtered, a list refreshed under the table): the selection kept as
        // the rows' keys (sorted) instead of their indices, rowKey(i) giving row i's key now (an id from your data).
        // A selected item stays selected wherever it moves; Shift selects the rows between in their current order.
        std::vector<std::uint64_t>* selectedKeys = nullptr;
        std::function<std::uint64_t(int)> rowKey;
    };
    struct TableSort
    {
        int column = -1;               // -1 = in the data's order
        bool ascending = true;
        bool changed = false;          // this frame: sort again
    };
    struct TableRange
    {
        int first = 0, last = 0;       // [first, last): the rows in view
    };
    struct TableRowResult
    {
        bool selected = false;
        bool clicked = false;
        bool doubleClicked = false;
    };
    // Rows go between BeginTable and EndTable. Only the rows in view are submitted (TableVisible), so a table of a
    // million rows costs what the visible ones cost; without TableVisible every row is submitted and the table
    // sizes itself from last frame's. In a row, each TableCell or TableNextColumn moves to the next cell; widgets
    // submitted after TableNextColumn go in that cell. A table is an id scope: two tables' rows never share ids.
    //
    //   if (ui::BeginTable("files", {{"Name"}, {"Size", 90, 0, ui::Align::End}}, {.flags = ui::TableFlags_Sortable})) {
    //       if (ui::TableSortSpec().changed) Sort(files, ui::TableSortSpec());
    //       const ui::TableRange rows = ui::TableVisible((int)files.size());
    //       for (int i = rows.first; i < rows.last; ++i) {
    //           ui::TableRow(i);
    //           ui::TableCell(files[i].name);
    //           ui::TableCell(files[i].size);
    //       }
    //       ui::EndTable();
    //   }
    ESIA_API bool BeginTable(std::string_view id, std::span<const TableColumn> columns, const TableOptions& options = {});
    inline bool BeginTable(std::string_view id, std::initializer_list<TableColumn> columns, const TableOptions& options = {})
    {
        return BeginTable(id, std::span<const TableColumn>(columns.begin(), columns.size()), options);
    }
    ESIA_API void EndTable();
    ESIA_API TableSort TableSortSpec();
    ESIA_API TableRange TableVisible(int rowCount);
    ESIA_API TableRowResult TableRow(int index);
    ESIA_API void TableNextColumn();
    ESIA_API void TableCell(std::string_view text, Color color = Color::Clear());
    ESIA_API void TableCellIcon(Icon icon, std::string_view text, Color iconColor = Color::Clear());

    enum TreeFlags_ : std::uint32_t
    {
        TreeFlags_None = 0,
        TreeFlags_DefaultOpen = 1u << 0,
        TreeFlags_Leaf = 1u << 1,        // no children: no arrow, never open (no TreePop)
        TreeFlags_Selected = 1u << 2,    // drawn selected (the host keeps the selection)
        TreeFlags_OpenOnArrow = 1u << 3, // only the arrow opens it; a click on the row only selects
    };
    struct TreeNodeOptions
    {
        std::uint32_t flags = TreeFlags_None;
        Icon icon = 0;
        Color iconColor = Color::Clear();   // Clear = the secondary label color
        std::string_view detail;            // secondary text on the right
    };
    struct TreeNodeResult
    {
        bool open = false;               // submit the children, then TreePop (also while they slide closed)
        bool clicked = false;            // the row was clicked this frame
        bool doubleClicked = false;
        bool toggled = false;            // it opened or closed this frame
        explicit operator bool() const { return open; }
    };
    // A row of an outline: an arrow that turns as it opens, an optional icon, the label. The children submitted before
    // TreePop slide open and closed under it, with a guide line on their left.
    //
    //   if (ui::TreeNode("Scene", {.icon = ui::icons::Folder})) {
    //       ui::TreeNode("Camera", {.flags = ui::TreeFlags_Leaf, .icon = ui::icons::Camera});
    //       ui::TreePop();
    //   }
    ESIA_API TreeNodeResult TreeNode(std::string_view label, const TreeNodeOptions& options = {});
    ESIA_API void TreePop();
    // Opens or closes a node the next time it is submitted (expand all, reveal a selection).
    ESIA_API void SetNextTreeNodeOpen(bool open);

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
    ESIA_API bool RowPicker(std::string_view label, int* selected, std::span<const std::string_view> items, RowIcon icon = {});
    inline bool RowPicker(std::string_view label, int* selected, std::initializer_list<std::string_view> items, RowIcon icon = {})
    {
        return RowPicker(label, selected, std::span<const std::string_view>(items.begin(), items.size()), icon);
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

    // A line chart of `values`: a monotone curve (it never overshoots the data) with a gradient under it and a dot on
    // the latest value. Without a fixed range it fits the data and eases to a new range instead of jumping.
    struct LineChartOptions
    {
        float height = 90.0f;          // UI units
        float width = -1.0f;           // < 0 = the available width
        Rect rect = Rect();            // not empty: draw into this rect (a row's, a card's) instead of taking a place
        Color tint = Color::Clear();   // Clear = accent
        float thickness = 2.0f;
        bool smooth = true;            // false: straight segments
        bool fill = true;              // the gradient under the line
        bool glow = false;             // a soft halo around the line
        bool lastPoint = true;         // a dot on the latest value
        float min = 0.0f, max = 0.0f;  // the value range; min == max: fitted to the data (from 0 for positive data)
        int offset = 0;                // a ring buffer's oldest value
    };
    ESIA_API void LineChart(std::string_view id, std::span<const float> values, const LineChartOptions& options = {});

    // ============================================================== plots
    // Plots for tools (what ImPlot gives Dear ImGui, in the glass style): lines, areas, scatter, bars and histograms
    // over axes with ticks and a grid. The legend's entries hide and show their series; under the mouse a crosshair
    // reads the values. A drag pans, the wheel zooms around the mouse (it takes the wheel from the scroll areas
    // around), a double click fits the data again.
    //
    //   if (ui::BeginPlot("frame times", {.height = 200, .yFormat = "%.1f ms"})) {
    //       ui::PlotLine("CPU", cpu);                         // x = the index
    //       ui::PlotLine("GPU", gpu, {.fill = true});
    //       ui::EndPlot();
    //   }
    enum PlotFlags_ : std::uint32_t
    {
        PlotFlags_None = 0,
        PlotFlags_NoLegend = 1u << 0,
        PlotFlags_NoGrid = 1u << 1,
        PlotFlags_NoHover = 1u << 2,     // no crosshair and no readout
        PlotFlags_NoPanZoom = 1u << 3,   // the view stays fitted (or at the given limits)
    };
    struct PlotOptions
    {
        float height = 220.0f;           // UI units
        float width = -1.0f;             // < 0 = the available width
        std::uint32_t flags = PlotFlags_None;
        float xMin = 0.0f, xMax = 0.0f;  // min == max: fitted to the data
        float yMin = 0.0f, yMax = 0.0f;
        const char* xFormat = "%g";      // the tick labels and the readout (printf, one float)
        const char* yFormat = "%g";
        std::span<const std::string_view> categories;   // labels under the x axis at 0, 1, 2 ... (PlotBars' bars)
    };
    struct PlotStyle
    {
        Color color = Color::Clear();    // Clear: the plot's next color
        float thickness = 2.0f;          // a line's
        float size = 3.5f;               // a scatter point's radius
        bool fill = false;               // a line: the area under it, fading down
        bool smooth = false;             // a line: a curve through the points
        float xStart = 0.0f, xStep = 1.0f;   // a line without xs: x = xStart + i * xStep
    };
    ESIA_API bool BeginPlot(std::string_view id, const PlotOptions& options = {});   // true: series, then EndPlot
    ESIA_API void PlotLine(std::string_view label, std::span<const float> ys, const PlotStyle& style = {});
    ESIA_API void PlotLine(std::string_view label, std::span<const float> xs, std::span<const float> ys, const PlotStyle& style = {});
    ESIA_API void PlotScatter(std::string_view label, std::span<const float> xs, std::span<const float> ys, const PlotStyle& style = {});
    // Bar i of every PlotBars series at x = i (PlotOptions::categories names them); the series side by side.
    ESIA_API void PlotBars(std::string_view label, std::span<const float> values, const PlotStyle& style = {});
    // The samples counted into `bins` bars between their smallest and largest (0: about the square root of their count).
    ESIA_API void PlotHistogram(std::string_view label, std::span<const float> samples, int bins = 0, const PlotStyle& style = {});
    ESIA_API void EndPlot();
    struct PlotLimits
    {
        float xMin = 0.0f, xMax = 1.0f, yMin = 0.0f, yMax = 1.0f;
    };
    // Between BeginPlot and EndPlot: the view the plot showed last frame - fitted, panned or zoomed (the first frame:
    // the given limits, else 0..1). To submit only what is in view.
    ESIA_API PlotLimits GetPlotLimits();

    // A donut of shares (iOS's activity rings, cut into segments): the value under the mouse shows in the hole.
    struct PieChartOptions
    {
        float size = 170.0f;             // the donut's diameter, UI units
        float thickness = 0.36f;         // the ring's width, of the radius (0.15 .. 0.7)
        bool legend = true;              // the labels and their shares beside it
        std::string_view center;         // the hole's text when nothing is hovered (a total ...)
        const char* format = "%.0f";     // the values in the hole
    };
    ESIA_API void PieChart(std::string_view id, std::span<const float> values, std::span<const std::string_view> labels = {},
                           const PieChartOptions& options = {});

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

    // ============================================================= docking
    // An area windows dock into. A window dragged by its header over it shows where it would go - into a node's tabs
    // (the middle) or beside it (an edge) - and docks there when let go; a tab dragged out of its node floats the
    // window again. The splitters between nodes resize them. Docked windows fill their node under a glass tab bar and
    // stay behind floating windows.
    //
    //   ui::DockSpace("main");                                  // every frame, before the windows
    //   if (first) {                                            // an initial layout
    //       ui::DockWindow("Scene", "main");
    //       ui::DockWindow("Outline", "main", ui::DockSide::Left, 0.22f);
    //       ui::DockWindow("Console", "main", ui::DockSide::Bottom, 0.28f, "Scene");
    //   }
    //   if (ui::BeginWindow("Scene")) { ...; ui::EndWindow(); }
    enum class DockSide : std::uint8_t { Center, Left, Right, Top, Bottom };
    // `rect`: UI units; empty = the whole display (less the safe area).
    ESIA_API void DockSpace(std::string_view id, Rect rect = Rect());
    // Docks `window` (a BeginWindow title) into `dockSpace`: into the tabs of the node holding `relativeTo` (Center) or
    // beside it with `ratio` of its room; relativeTo empty: the whole dock space. A window already docked moves.
    ESIA_API void DockWindow(std::string_view window, std::string_view dockSpace, DockSide side = DockSide::Center, float ratio = 0.25f,
                             std::string_view relativeTo = {});
    ESIA_API void UndockWindow(std::string_view window);
    ESIA_API bool IsWindowDocked(std::string_view window);
    // Brings a docked window's tab to the front of its node ("show the console").
    ESIA_API void FocusDockedWindow(std::string_view window);
    // The layout as text, to keep with the application's settings, and back (false: not a layout of this version).
    ESIA_API std::string SaveDockLayout(std::string_view dockSpace);
    ESIA_API bool LoadDockLayout(std::string_view dockSpace, std::string_view layout);

    // ========================================================= auto layout
    // Containers that size and place their children from the room they are given, so pages re-flow with the window.
    // Every widget (or nested container) submitted directly inside is one child; children are measured every frame,
    // placement uses the latest measurements and moves glide on a spring. Widgets that fill the available width
    // (sliders, width < 0 buttons, wrapped text ...) are flexible children.
    //
    //   ui::BeginFlow("buttons"); ui::Button("Play"); ui::Button("Settings"); ui::EndFlow();
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
    // The width available at the cursor, less what the items after it on its line (SameLine) took last frame. In an
    // auto-layout container, asking for it makes the item a flexible child.
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
