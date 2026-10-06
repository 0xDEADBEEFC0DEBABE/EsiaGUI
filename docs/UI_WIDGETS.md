# Esia UI widgets

`esia_ui` (`include/esia/ui`, `src/esia/ui`, target `esia::ui`) is the widget layer: WGT's liquid-glass widgets,
ported onto the UI core ([UI_CORE.md](UI_CORE.md)) and drawn with the Painter (`esia/render/painter.hpp`). It is
immediate mode like WGT: a widget is a function call per frame, and what outlives a frame (springs, layout
measurements, scroll physics) is per-id state in the core. This document covers how to use it, how it is built on
the core, and what of WGT is ported so far.

* [1. Setting up](#1-setting-up)
* [2. The theme](#2-the-theme)
* [3. Styling one widget or a block](#3-styling-one-widget-or-a-block)
* [4. Text and controls](#4-text-and-controls)
* [5. Windows, cards, scroll areas](#5-windows-cards-scroll-areas)
* [6. Lists: sections and rows](#6-lists-sections-and-rows)
* [7. Navigation, the tab bar and the search bar](#7-navigation-the-tab-bar-and-the-search-bar)
* [8. Auto layout](#8-auto-layout)
* [9. Custom widgets and animation](#9-custom-widgets-and-animation)
* [10. The island, notifications and the dock](#10-the-island-notifications-and-the-dock)
* [11. The showcase: WGT's demo](#11-the-showcase-wgts-demo)
* [12. Data widgets: numbers, colors, tables, trees, the editor, docking](#12-data-widgets-numbers-colors-tables-trees-the-editor-docking)
* [13. Status: what of WGT is ported](#13-status-what-of-wgt-is-ported)
* [14. Pitfalls](#14-pitfalls)

## 1. Setting up

```cpp
esia::Context ctx;
esia::ui::Ui ui(ctx, {.text = &textSystem, .theme = ui::ThemeDark()});

// every frame, on the Context's thread
ctx.NewFrame(params);
ui.NewFrame();
if (ui::BeginWindow("Settings", &open)) {
    ui::Headline("Display");
    ui::Toggle("hdr", &hdr);
    ui::EndWindow();
}
ui.EndFrame();
ctx.EndFrame();
// the host renders ctx.GetDrawData() (the Ui steps the text system in its NewFrame)
```

* **Fonts.** The `Ui` loads its fonts into the text system it is given (`UiDesc::text`). By default:
  * the platform's UI font at 400, 600 and 700 (`FindSystemFont(kSystemUiFamily, weight)`), one `FontWeight` each;
  * a monospace font (Cascadia Mono, Consolas ...);
  * the platform's fallback chain for CJK and symbols (`AddFallbackFonts`);
  * Segoe Fluent Icons, else Segoe MDL2 Assets, as the icon font.

  `UiDesc::fontFiles` / `iconFontFile` override any of them. Without a text system the widgets still lay out and
  react (text measures as empty), which is how `tests/ui` runs.
* **The current Ui.** Between `NewFrame` and `EndFrame` the `Ui` is the thread's current one (`ui::Current()`); the
  free `ui::` functions act on it. One `Ui` per `Context`.
* **Event-driven hosts.** `Ui::Animating()` is true when something moved this frame (a spring, the theme
  cross-fade): render another frame.
* **Debug checks.** In a debug build the core reports two widgets with one id and a widget laid out where it cannot
  be seen, and outlines them in red ([UI_CORE.md](UI_CORE.md) section 16). What else trips people up: section 14.

## 2. The theme

A `Theme` (`theme.hpp`) is a plain struct holding every design token. Widgets never hard-code a color or a size.

| Part | Holds |
| --- | --- |
| `Palette colors` | accent, labels (4 levels), backgrounds, fills, separators, the 13 system colors, and surfaces: window tint, card, knob, shadow, highlight |
| `Materials materials` | glass materials: `window`, `bar`, `control`, `popover`, `clear` |
| `Metrics metrics` | `scale` (every size goes through `ui::S`), radii, paddings, spacing, row and header heights, the scroll indicator (the built-in controls are 32 - 34 tall whatever `controlHeight` says: it is for custom widgets) |
| `Typography type` | a size and weight per `TextStyle` (Apple's type ramp, LargeTitle .. Caption2, Mono) |
| `Motion motion` | the springs widgets use (hover, press, toggle, layout ...) |

* **Built-in themes.** `ThemeLight()` and `ThemeDark()` hold WGT's values. `ThemeWithAccent(theme, color)` changes
  the accent and what derives from it.
* **A desktop tool's density.** The sizes are iOS's (44-unit rows, 34-unit controls, 15-unit body text).
  `ThemeCompact(theme)` sets everything at 0.87 of its size (body text 13, controls about 29), with shorter rows
  and headers, less padding and spacing and smaller corners: `workbench --compact`. `SetDarkMode` keeps it.
* **Scroll indicators.** `metrics.scrollIndicatorAlways` is 1 in the built-in themes on Windows, Linux and macOS
  (an area that scrolls shows its indicator, dimmed, at rest) and 0 on iOS and Android (hidden at rest); after a
  finger's press they hide at rest anyway (section 5).
* **Spacing.** Every frame the Ui sets the core's item spacing (`Context::Metrics().itemSpacing`) from the theme:
  `metrics.spacing` across and 0.8 of it down, times the scale, as WGT set Dear ImGui's `ItemSpacing`.
* **Changing the theme.** `Ui::SetTheme(theme, animate)`, `SetDarkMode(dark)` and `SetAccent(color)` cross-fade over
  the theme's transition time. `SetDarkMode` changes only the colors and materials: the accent, the metrics (the
  scale, `scrollIndicatorAlways`), the type and the motion stay as they were set; an accent set with `SetAccent` stays across
  theme changes (`Color::Clear()` returns to the theme's own). `LerpTheme` blends every color, material and
  metric; `dark` switches at the middle.
* **Glass look.** `Ui::SetGlassLook` sets how every glass surface renders:
  * `Theme`: each surface uses its material;
  * `Clear`: only the lens, the rim and the hairline;
  * `Frosted`: iOS frost everywhere.

  `ui::Next().Look(...)` overrides it for one widget or a container.

## 3. Styling one widget or a block

There is no push / pop for styles. An `ItemStyle` sets only the fields it names; everything else follows the
enclosing scopes and the theme.

```cpp
ui::Next().Tint(Color::Hex(0x30D158)).Radius(8);   // the next widget only
ui::Toggle("hdr", &hdr);

ui::Next().Look(ui::GlassLook::Clear);              // a container: itself and everything inside
ui::BeginCard("player");
...
ui::EndCard();

{
    ui::StyleScope s(ui::ItemStyle().Tint(red).Opacity(0.8f));   // a block, until the end of the scope
    ...
}
```

* **Fields.**
  * Glass: `Look`, `Blur`, `Refraction`, `Bezel`, `Dispersion`, `Saturation`, `Brightness`, `Specular`, `Legibility`,
    `Magnify`, `GlassTint`, `Rim`.
  * The widget: `Tint` (its accent), `Fill` (its surface), `Radius`, `Opacity`, `Label`.
  * Selections: `SelectedFill`, `MovingFill`, `SelectedLabel`.
* **Merging.** Scopes merge field by field and the innermost wins. Opacity multiplies down the scopes.
* **Taking the style.** A widget takes `Next()` when it starts, so it styles exactly one widget. A container takes
  it for itself and keeps it as a scope for its children until its `End`.
* **Glass on flat widgets.** A glass field or a `Clear` / `Frosted` look on a flat widget (a filled button, a switch
  track, a field) turns its surface into that glass, tinted by its fill.
* **Custom widgets** read the merged style with `CurrentItemStyle()`, `AccentColor()`, `CurrentGlassLook()` and
  `LookMaterial(themed)`.

## 4. Text and controls

Text never runs past its container: when it is wider than the room left on its line, it wraps at the container's
edge. `TextWrapped` always wraps at the container's width. Inside an auto-layout container, the container sizes it.

| Function | What |
| --- | --- |
| `Text`, `TextColored`, `TextSecondary`, `TextUnformatted`, `TextWrapped` | text in a `TextStyle` (printf-style or a string) |
| `LargeTitle`, `Headline`, `Spacer`, `Divider` | page structure |
| `Button(label, {kind, size, icon, tint, width, capsule, glow})` | the 7 `ButtonKind`s (Filled, Tinted, Gray, Plain, Glass, GlassProminent, Destructive) in 3 sizes; `width < 0` fills |
| `IconButton(id, icon, options)` | a round button with one symbol |
| `Toggle(id, &on)` | the iOS 26 switch: the knob overshoots and squashes at the end of its travel, stretches with speed, and swells into a clear lens while held or just flipped |
| `Checkbox(label, &on)` | a round check with its label |
| `ToggleButton(id, &on, icon, {tint, diameter, glow})` | the round Control Center toggle; the tint floods in from the press point |
| `Slider(id, &v, min, max, {width, minIcon, maxIcon, tint, step})` | a capsule track whose knob swells into a lens when grabbed and stretches with fast motion; optional end icons and snapping |
| `Stepper(id, &n, min, max, step)` | - / + with auto-repeat |
| `ProgressBar`, `ProgressRing`, `ActivityIndicator` | determinate (springs toward the value) and indeterminate progress |
| `Badge(text, tint)`, `ColorSwatches(id, &i, colors, n)`, `Image(texture, size, radius)` | small parts |
| `LineChart(id, values, {height, rect, tint, smooth, fill, glow, min, max, offset})` | a monotone curve (never overshooting the data) with a gradient under it and a dot on the latest value; fits the range to the data and eases it; a ring buffer through `offset`; `rect` draws into a row or card without taking room |
| `Segmented(id, &i, {"A", "B", "C"}, width)` | the iOS 26 segmented control: tap a segment, or grab the selection and drag it; it travels as a clear lens that magnifies what it passes, then lands on the nearest segment as a solid pill. `*i` changes on release |

* **Ids.** Labels may carry an id suffix: `"OK##dialog"` shows "OK", and `"##x"` shows nothing.

**Menus, pickers and tooltips** are glass popups on the core's popups ([UI_CORE.md](UI_CORE.md) section 9):

```cpp
if (ui::IconButton("more", ui::icons::More)) ui::OpenMenu("actions");
if (ui::BeginMenu("actions", {.anchor = {r.max.x, r.max.y + 6}, .pivot = {1, 0}})) {   // under the button
    if (ui::MenuItem("Rename", ui::icons::Edit)) ...;
    if (ui::MenuItem("Pinned", ui::icons::Pin, pinned)) pinned = !pinned;           // with a check mark
    ui::EndMenu();
}
ui::Picker("size", &size, {"Small", "Medium", "Large"});   // shows the choice, opens a menu of the choices
ui::Picker("item", &item, names, {.width = 220, .maxRows = 12});   // a long list scrolls inside its menu
ui::Tooltip("What it does");                                // for the widget before it
```

* A menu opens with `OpenMenu` (at the mouse, or at `MenuOptions::anchor` with `pivot`) and is submitted with
  `BeginMenu` every frame. It fades in and grows from 96 % on the popover glass, sized to its widest item (at least
  `minWidth`). Choosing an item, a click outside it or Escape closes it. The click outside belongs to the menu:
  the widget under it does not react (iOS). Items past `maxHeight` (by default: what fits on the display) scroll
  inside the menu, with the edge fades and the indicator of a scroll area.
* `Picker` is a pop-up button: its menu opens under it, right-aligned, with a check at the current choice. It shows
  `PickerOptions::maxRows` choices at most (10); a longer list scrolls inside it and opens with the current choice
  in the middle. `RowPicker` is the same in a list row.
* Menus are on whole physical pixels: anchored at their right edge between two pixels, they no longer creep and
  jump ([UI_CORE.md](UI_CORE.md) section 7).
* `Tooltip` shows a glass tip after half a second over the widget before it, following the mouse. It waits again
  when the mouse comes back.

**Text fields** (WGT's single-line editor):

```cpp
if (ui::TextField("name", &name, "Your name"))           // true when the text changed
    ...;
ui::TextField("pin", &pin, "PIN", {.password = true, .maxBytes = 8});
ui::SearchField("search", &query);                        // a field with the search symbol
```

* The field edits a `std::string` and writes every change back at once. The result also tells whether Enter was
  pressed (`.submitted`).
* A click or Tab gives it the keyboard; Enter, Escape or a click elsewhere takes it away. It claims only the keys it
  uses, so Tab still moves the focus.
* The caret moves by graphemes, as the text system reports them (`TextSystem::CaretStops`). So it steps over a
  letter with its accents, an emoji sequence or a flag as one, and through a right-to-left word in the direction
  it is drawn. Ctrl moves and deletes by words (Option on macOS).
* Selection: drag, Shift with the keys, double click (a word), triple click (all). Clipboard: Ctrl+A / C / X / V
  (Cmd on macOS); a paste becomes one line. Undo and redo keep 200 steps; typing within a second is one step.
* The IME's composition is drawn in the text, underlined, with the clause it converts underlined thicker. Its
  candidate window opens at the caret (`Context::RequestTextInput`).
* Options: `width`, `icon`, `password` (bullets per grapheme, nothing copied), `clearButton`, `background` (off
  on glass), `maxBytes`.
* **Return values.** The functions that change a value return true in the frame they change it.

**Glow halos** (WGT's item map). A widget with `glow` (a button, a toggle button, the chart's dot) lights the area
around it, and the light stops short of its neighbors:

* The Painter asks the Ui how far a glow may reach (`PainterEnv::glow`, a `GlowContainment`). The Ui answers from
  the window's items of last frame (`Window::LaidOutItems()`, WGT's `ComputeGlowHalo`): on each side the halo ends
  at the first item in its way (not its own, nor the containers it is in), and it fades out over the gap before it.
* Inside a stack, grid or flow the glowing child keeps room for its glow: from the next frame, the gaps next to it
  grow to the glow's reach, as if it had a margin. A glow drawn inside a nested container counts when that
  container is laid out; a glow that stays inside its child (a toggle in a tile) takes no room.

## 5. Windows, cards, scroll areas

* **`BeginWindow(title, &open, {size, pos, flags, subtitle, icon})`** is a liquid-glass window: a glass surface
  with a shadow, a header and a body that scrolls smoothly.
  * The header has the close button (when `open` is given), an icon tile, and the title and subtitle; the
    subtitle is cut with an ellipsis.
  * The body fades at its edges and is clipped to the window's rounded corners.
  * When closed, the window fades out before `*open` turns false.
  * The core moves and resizes it; the resize corner shows as an arc, and that whole corner (26 units square)
    resizes the window, above what is under it. The scroll indicator ends above the rounded corner.
  * First-use positions cascade when `pos` is negative.
  * Flags: `NoClose`, `NoResize`, `NoMove`, `NoHeader`, `NoScroll`, `Solid`, `LargeTitle`, `NoShadow`, `NoPadding`,
    `ClearGlass`.
* **`BeginCard(id, size, {glass, radius, fill, shadow, padding})`** is an inset card.
  * Its background is drawn at `EndCard`, once its height is known, and moved under the content
    (`DrawList::Mark` / `MoveCommands`, [UI_CORE.md](UI_CORE.md) section 11). So there is no one-frame lag and no
    measuring frame.
  * With `size.x == 0` it fills the available width.
* **`BeginScrollArea(id, size)`** is a child region with the core's smooth scrolling and an edge fade.
* **Edge fades** (iOS's scroll edge effect): content dissolves toward an edge it can still scroll past, over up to
  22 units at the top and 30 at the bottom, growing with the distance left to scroll. The edge is where the content
  stops being visible: a window reaching past the display fades at the display's edge.
* **Scrolling** is the same in a window's body, a scroll area and a navigation page:
  * the wheel and the core's scroll calls glide (`ChildFlags_SmoothScroll`);
  * pressed on empty space by a finger, the content follows it, and let go it glides on with the drag's speed (no
    rubber band past the ends: the core keeps the offset in range - WGT's rubber band did nothing either). A mouse
    does that only with `InputConfig::mouseDragScrolls` (as WGT's did): on a desktop the wheel and the indicator
    scroll, and a mouse drag on empty space moves the window;
  * the indicator shows while the content moves and for 0.9 s after, and when the mouse comes near. It also flashes
    when an area appears with more than it shows, or grows past it (iOS's `flashScrollIndicators`). It widens under
    the mouse and can be dragged. It runs in a lane in the window's right padding (also for a page or scroll area
    flush with the window's content), else over the content at the right edge. It shows, and takes clicks, only
    where its area shows: a table scrolled half out of its window has its indicator in the window;
  * `Theme::metrics.scrollIndicatorAlways = 1` keeps the indicator of every area that scrolls, dimmed at rest, on a
    faint track: for desktop apps, where users look for a scroll bar - the built-in themes' value on Windows, Linux
    and macOS. 0 hides it at rest, as iOS (the themes' value on iOS and Android; the showcase sets it everywhere, as
    WGT's demo). After a finger's press it hides at rest whatever the theme says, until a mouse presses again;
  * the wheel stays with the area it scrolls while it turns: a table the window scrolls in under the mouse does not
    take the rest of the turn ([UI_CORE.md](UI_CORE.md) section 8);
  * a finger's drag on empty space scrolls instead of moving the window; the header, and content that cannot
    scroll, still move it (the core moves a window only when no widget took the click);
  * a finger scrolls from anywhere, as on a phone (presses marked `InputEvent::touch`: Android, iOS, Windows touch
    screens). From a row or a button, the content follows once the finger has moved 8 units along the area
    (`InputConfig::touchSlop`), and the row does not press. A control that acts on the press (a slider, a segmented
    control, a field, a stepper's repeat) does not get a finger's press until it is not a scroll: the finger lifts,
    rests for 0.15 s, or moves across. A swipe over a list of sliders scrolls it; a sideways drag moves the slider.
    The mouse keeps the desktop's behavior ([UI_CORE.md](UI_CORE.md) section 5);
  * held at an end, the content stays still: the offset the layout starts at lands on whole pixels.

## 6. Lists: sections and rows

Inset grouped lists, as in iOS Settings:

```cpp
ui::BeginSection("Display", "HDR needs a display that supports it.");   // header and footer are optional
ui::RowToggle("HDR", &hdr, {ui::icons::Brightness, Color::Hex(0xFF9F0A)});   // an icon tile and its color
ui::RowSegmented("Frame Limit", &limit, {"Off", "60", "120"});
ui::RowValue("Resolution", "2560 x 1440");
if (ui::RowNavigation("Advanced", "On")) ...;
ui::EndSection();
```

* **A section** is one item of its parent: the header, the card with its rows, and the footer. Inside an auto-layout
  container it is one child, and it takes the width it is offered.
  * It is an id scope: its header's. Sections without a header, or with the same header, in one scope are told
    apart by their order, so the same row label in each is a row of its own.
  * The rows sit in the core's `StackLayout` without gaps, so they need no measuring frame.
  * The card is drawn at `EndSection` and moved under the rows, as a card's background is.
  * A row's hover highlight is masked by the card's rounded corners (last frame's height).
* **Rows**: `RowNavigation` (a chevron and a detail text; returns true when pressed), `RowToggle`, `RowSlider`
  (a value column formatted with printf), `RowStepper`, `RowSegmented`, `RowValue`, and `RowButton` (accent, or red
  when destructive).
* **Label and accessory.** A row's label gets the room its accessory leaves.
  * A switch, value, stepper or segmented control keeps its natural width while it fits, down to a minimum.
  * Sliders start on a column shared by the section (38 % of the row), so a section's sliders line up.
  * When even a short label does not fit beside the accessory's minimum, the row stacks them: label on top,
    control below.
  * Labels and values are cut with an ellipsis; nothing overlaps at any width.
* **Rows of your own.** `BeginRow(id, height, &content)` / `EndRow` give a row of that height. Its content rect is
  inset like the labels. Widgets submitted in between go inside it, and widgets that fill the available width stay
  in the row.

## 7. Navigation, the tab bar and the search bar

```cpp
ui::BeginNavigation("settings", "root");   // takes the room left in its area
if (ui::BeginPage("root", "Settings")) {
    if (ui::RowNavigation("Accent Color")) ui::NavigationPush("accent");
    ui::EndPage();
}
if (ui::BeginPage("accent", "Accent Color")) { ...; ui::EndPage(); }
ui::EndNavigation();

ui::TabBar("tabs", &tab, {{ui::icons::Apps, "Controls"}, {ui::icons::Palette, "Style"}});   // last in its area
ui::SearchBar("search", &query);                                                           // or this
```

* **Navigation** is a stack of pages, as in iOS.
  * Every page is submitted every frame. `BeginPage` returns true for the page shown and, during a transition, for
    the page leaving.
  * A pushed page slides in from the right over the current one. A popped page slides out and uncovers the one
    below, which drifts in from the left and brightens.
  * The top page counts as opaque: the page below is clipped to the strip left of its edge, which casts a soft
    shadow. The pages are glass, and without the clip their text would show through each other.
  * Each page has a title bar and a back button. The button is titled after the page below and pops when pressed.
  * Each page scrolls on its own (a child region with smooth scrolling and edge fades).
  * `NavigationPush` and `NavigationPop` work anywhere between `BeginNavigation` and `EndNavigation`.
* **The tab bar** floats over the bottom of the area it is submitted in, a window's content or a page
  (`Context::ViewRect`). The content scrolls under the glass, and the bar reserves room below the content so its
  end can scroll out from under it.
  * Tap a tab or drag the selection, as on a segmented control.
  * A child region draws in submission order, inside the edge fades of the scroll areas around it. So a floating
    bar's draw commands are moved to the end of its window's draw list at `Ui::EndFrame`: it draws over all the
    window's content and is not faded. (WGT's bars were ImGui child windows, drawn after their parent.) Card and
    section backgrounds, which are moved under their content, go through the same bookkeeping.
* **The search bar** floats the same way: a text field with the search symbol in a glass pill across the bottom of
  its area (iOS Settings). Its glass is clear by default with a soft base inside (`SearchBarOptions::look`, `base`,
  `fill`); a Clear look from the style makes it fully transparent.

## 8. Auto layout

Stacks, grids and flows are `LayoutProvider`s of the core ([UI_CORE.md](UI_CORE.md) section 6). Every widget or
nested container submitted directly inside one is one child.

| Container | Places its children |
| --- | --- |
| `BeginVStack` / `BeginHStack` (`EndStack`) | in a column / row; `align` on the cross axis, `justify` on the main axis when no child is flexible |
| `BeginAdaptiveStack` | in a row while the children fit side by side, else in a column (SwiftUI's `ViewThatFits`) |
| `BeginGrid` (`EndGrid`) | in columns: as many as fit `minColumnWidth` (or `columns`, capped by `maxColumns`), cells stretched to the width; `cellHeight`, `aspect`, `LayoutSpan(n)` |
| `BeginFlow` (`EndFlow`) | left to right, wrapping into lines; `justify` per line, `align` within a line |

* **Measure, then place.** Each frame a container places its children with the sizes they had in the previous frame,
  and records their sizes this frame.
  * In a container's first frame nothing is known yet. It measures under an empty clip, so nothing is seen, and
    shows its children from the second frame on.
  * Moves glide on a spring when `animate` is set.
  * Positions land on whole pixels.
* **Flexible children.** Widgets that ask for the available width are flexible children: sliders, `width < 0`
  buttons, wrapped text, `FlexSpacer()`. In a stack they share the free main-axis room, by `LayoutFlex(f)` shares.
* **Width.** A grid or a flow always takes the width it is offered. So does a stack with a flexible child, or with a
  non-`Start` `justify` (rows) or `align` (columns).

  Any other stack is as wide as its content. For example, two gauge stacks in a flow sit side by side.
* **Size classes.** `GetSizeClass(width)` returns `Compact` (< 420), `Regular` (< 760) or `Expanded`, for pages that
  change their structure with the width.
* **Outside a container, with `SameLine`.** A widget that fills the available width (`AvailableWidth()`: a number,
  vector or text field, a picker of width < 0, a table) leaves the items after it on its line the room they took last
  frame: `NumberField` then `SameLine()` and a button keeps the button in view (`Context::LineRoom`).

## 9. Custom widgets and animation

```cpp
const ui::Interaction it = ui::Interact("knob", {S(44), S(44)});   // layout room + press / hover logic
ui::StyleScope s(it.style);                                         // the Next() style it took
esia::Painter p = ui::GetPainter();
p.Circle(it.rect.Center(), S(22) * (1.0f - 0.08f * it.press), Style().Fill(ui::AccentColor()));
if (it.pressed) ...;
```

* **Interaction.**
  * `Interact` takes layout room at the cursor. `InteractRect` works on an explicit rect.
  * Both return hover and held state, whether the widget was pressed this frame, and spring-animated `hover` /
    `press` values.
  * Flags: `PressOnClick`, `Repeat`, `Disabled`.
* **Drawing.** `GetPainter()` is a Painter for the current window. It carries the theme's corner smoothing, the
  window's pixel density, the style's opacity and the Ui's text system. `S(v)` scales a size by the theme.
* **Animation.** `Anim(id, target, spring)` springs toward `target`. It is keyed by id, stepped once per frame however
  often it is read, and reported by `Animating()`.
  * `AnimVelocity`, `AnimSet` and `AnimKick` read, jump and push a spring.
  * `Timer` is a restartable 0..1 ramp. `Time()` and `DeltaTime()` give the frame clock.
  * `Salt(id, n)` derives ids for a widget's parts.
* **Springs** (`anim.hpp`) are analytic: the same curve at any frame rate, coming to rest exactly on the target. The
  presets are `Spring::Snappy`, `Smooth`, `Bouncy` and `Gentle`, as in SwiftUI.

## 10. The island, notifications and the dock

```cpp
ui.Notify({.title = "Download complete", .message = "HD texture pack", .icon = icons::Download});   // any thread
ui.SetActivity("download", "Downloading texture pack", 0.4f, icons::Download);                     // any thread
ui.ClearActivity("download");

bool settingsOpen = true, effectsOpen = false;
ui::Dock({{"Settings", icons::Settings, Color::Hex(0x8E8E93), &settingsOpen},
          {"Effects Lab", icons::Brush, Color::Hex(0xFF375F), &effectsOpen}});                      // every frame
```

* **The island** (WGT's Dynamic Island) draws itself at the top center of the display at `Ui::EndFrame`, over
  everything (`UiDesc::island = false` turns it off).
  * A notification arrives like the Siri orb: the island opens into a card big enough for the message, a black body
    clearing into a lens that magnifies what lies below, with a streak of light where black turns into glass. Then
    it settles into a pill (icon, title, time left) until its `duration` ends. Notifications queue: one after the
    other.
  * A live activity (`SetActivity`, `progress` 0..1, < 0 indeterminate) stays in the pill with a progress ring until
    `ClearActivity`; a new one gets the same short arrival.
  * `Notify`, `SetActivity` and `ClearActivity` take a lock: a worker thread calls them directly.
* **The dock** (macOS) is a glass shelf of tiles along the bottom of the display, above the windows. A tile magnifies
  under the mouse and shows its label; a click toggles its `open` flag, and a dot under the tile shows it is set.
  `Dock` returns the tile clicked this frame, else -1. It is a window of its own size in the overlay layer, so
  nothing around it takes the mouse.

## 11. The showcase: WGT's demo

`examples/showcase` is WGT's demo (`examples/demo/showcase.cpp` at tag `wgt-1.1-final`) on Esia, panel for panel:
liquid-glass windows over the wallpaper (cover-fitted, with drifting color blobs), the dock that opens them, the
status bar, and a worker thread that posts notifications, a download activity and telemetry. It uses the same frame
as `glass_window`: the `glass_app` library, with the Win32 platform layer, every backend, and a render thread.

| Panel | What |
| --- | --- |
| Settings | a navigation stack: the root page (profile row, every row kind, the floating search bar), Accent Color, Performance |
| Effects Lab | the glass materials over a moving stage, liquid morphing, a custom HLSL effect (an aurora), a bloom layer, gradients, shadows and glowing arcs |
| Control Center | modules in groups: round toggles, tall sliders, Now Playing |
| Components | three tabs under a tab bar: Controls, Inputs (text fields, picker, menu), Status (progress) |
| Languages | one line in each of 13: English, Simplified and Traditional Chinese, Japanese, Korean, Arabic, Hebrew, Hindi, Thai, Russian, Greek, Vietnamese, emoji |
| Telemetry | values a worker thread writes (gauges, a log) |
| Plugin: Hello | WGT's plugin panel with its own effect (a hologram), drawn by the showcase (no plugin host yet) |

```
showcase.exe --api d3d11 --size 1600x1000 --scale 1 --open languages --fixed-dt 0.016667 --frames 120 --screenshot shot.png
```

* `--open list` opens those panels (`settings,effects,control,components,languages,telemetry,plugin`, `none`, or
  `--open-all`); the default is the first three. `--open-later panel@frame` opens one at a frame, as a click on its
  dock tile would (what a panel's first appearance costs).
* `--light` starts light (the demo starts dark), `--look` sets the glass look, `--tab n` the Components tab,
  `--page accent|perf` the Settings page, `--menu` opens the Components menu.
* The options of `glass_window` (`app.hpp`) apply: `--api`, `--size`, `--scale`, `--frames`, `--fixed-dt`,
  `--screenshot` and `--debug`. `--scale s` renders at s pixels per UI unit whatever the monitor, and `--size` takes
  fractions: `--size 1066.6667x666.6667 --scale 1.5` is 1600 x 1000 pixels at UI scale 1.5.

**Compared with WGT's screenshots.** Branch `reference/wgt-1.1` held WGT's captures (deleted on 2026-10-02; the
numbers below were measured against them) of each panel, dark and light,
at UI scale 1 and 1.5 (1600 x 1000 pixels each, the same clock). The showcase takes the same captures and each
panel's window is compared pixel by pixel, the island masked (its timing differs). Mean difference per pixel
(0 - 255), dark / light:

| Panel | x1 | x1.5 |
| --- | --- | --- |
| Settings | 0.38 / 0.41 | 0.74 / 1.41 |
| Effects Lab | 0.41 / 0.41 | 0.93 / 1.57 |
| Control Center | 0.81 / 0.81 | 0.49 / 0.40 |
| Components | 0.85 / 0.88 | 1.97 / 1.63 |
| Languages | 1.24 / 1.09 | 0.54 / 0.76 |
| Telemetry | 0.7 - 1.3 / 0.76 | 1.3 - 1.8 / 1.2 - 1.8 (from run to run: live values) |

What still differs, and why:

* Text: FreeType's rasterization is not DirectWrite's, so glyph edges differ by a few levels everywhere. This is
  most of what is left in every panel.
* Glass edges, on purpose: WGT's rim light flickered along curves (thinner than a pixel, sampled at the pixel
  center: a round button looked dented). Esia averages it over each pixel exactly, so glass edges differ by a few
  levels; most of Control Center's difference at 1x (0.50 / 0.58 before).
* Chinese: WGT asked DirectWrite's fallback for each character with the user's locale; under en-US that gives
  Yu Gothic UI for all CJK text, so WGT drew Chinese with Japanese glyph forms. Esia keeps Microsoft YaHei for Chinese
  (the same widths; the Japanese and Korean lines now match WGT).
* Emoji are drawn as outlines (color glyphs are not drawn yet); the emoji line is below the fold in the captures.
* Live values: the Telemetry log, the gauges and the island depend on the worker thread's timing.
* At 1.5: a few buttons sit a pixel apart horizontally.
* Texts reworded for Esia (the Telemetry note, the Settings footer, the version).

## 12. Data widgets: numbers, colors, tables, trees, the editor, docking

What tools need beyond WGT's set, in the same glass style. The `workbench` example puts all of them in one dock space
(an outline, a scene, an inspector, a table of 100 000 assets, a script editor):

```
workbench                                  # the docked layout
workbench --float Inspector --show Script  # one window floating, the editor's tab in front
workbench --rows 1000000 --light           # a million rows, light theme
```

```cpp
ui::NumberField("speed", &speed, {.min = 0, .max = 10, .step = 0.1, .format = "%.1f m/s"});
ui::VectorField("position", pos, 3);
ui::ColorPicker("tint", &tint, {.swatches = kSwatches});
ui::TextEditor("script", &source, {.size = {0, 240}, .lineNumbers = true, .monospace = true, .tabInput = true});

if (ui::BeginTable("assets", {{"Name"}, {"Size", 90, 0, ui::Align::End}},
                   {.flags = ui::TableFlags_Sortable | ui::TableFlags_Selectable, .height = 300, .selection = &selected})) {
    if (ui::TableSortSpec().changed) Sort(assets, ui::TableSortSpec());
    const ui::TableRange rows = ui::TableVisible((int)assets.size());
    for (int i = rows.first; i < rows.last; ++i) {
        ui::TableRow(i);
        ui::TableCellIcon(assets[i].icon, assets[i].name);
        ui::TableCell(assets[i].size);
    }
    ui::EndTable();
}

if (ui::TreeNode("Characters", {.icon = icons::People})) {
    ui::TreeNode("Player", {.flags = ui::TreeFlags_Leaf | (sel ? ui::TreeFlags_Selected : 0)});
    ui::TreePop();
}

ui::DockSpace("main");                                   // every frame, before the windows
if (first) {
    ui::DockWindow("Scene", "main");
    ui::DockWindow("Outline", "main", ui::DockSide::Left, 0.22f);
    ui::DockWindow("Assets", "main", ui::DockSide::Bottom, 0.35f, "Scene");
}
```

* **`NumberField`** (`float`, `double`, `int`): drag it sideways to scrub the value (Shift: a tenth of the speed);
  click it to type, also an expression (`2*(3+4)`); Enter or a click elsewhere takes it, Escape keeps the old value,
  Up / Down step. With both limits finite the field fills in proportion to the value; `buttons` adds - and +, `label`
  a short label inside. **`VectorField`** puts 2 - 4 of them side by side, labeled X Y Z W in red, green, blue and
  gray.
* **`ColorPicker`** is iOS's: a saturation / brightness spectrum over a hue bar, an opacity bar, the hex value
  (`#RRGGBB`, `#RRGGBBAA`) and swatches. **`ColorWell`** is the round well that opens it in a glass popover.
* **Tables** submit only the rows in view (`TableVisible`): a million rows cost what the visible ones cost. Headers
  sort (`TableSortSpec`: the application sorts its data when `changed`), the lines between them resize, rows select
  (click, Ctrl adds, Shift extends), stripes and column lines are flags. A cell holds text (`TableCell`), an icon and
  text, or any widget after `TableNextColumn`. Row offsets are kept in doubles, so the last of a million rows lands on
  its pixel.
  * Height: `height` fixes it (the rows scroll inside); `maxHeight` makes it as tall as its rows up to that height,
    then they scroll. It takes the height from the count given to `TableVisible`, in the same frame (without
    `TableVisible`, from the rows submitted last frame); both given, the smaller; 0 of both: as tall as all the rows.
  * `TableVisible` is what keeps a big table cheap: it tells the rows in view, and only those are submitted. Without
    it every row is submitted and laid out (fine for a few dozen).
  * Selection: `selection` holds row indices. Rows that move (sorted, filtered, a list refreshed under the table)
    keep the selection as keys instead: `selectedKeys` with `rowKey(i)`, the key of row i now (an id of your data).
    A selected item stays selected wherever its row goes, and Shift selects the rows between in their order now.
  * A table is an id scope: tables in one window scroll on their own, and widgets in their cells never share an id
    with another table's. The multi-line editor is one too.

  ```cpp
  ui::TableOptions o{.flags = ui::TableFlags_Selectable, .maxHeight = 330, .selectedKeys = &picked};
  o.rowKey = [&](int i) { return items[i].id; };
  ```
* **Trees** (`TreeNode` / `TreePop`): an arrow that turns as the node opens, an icon, the label and a detail on the
  right; children slide open and closed under it with a guide line. `TreeFlags_OpenOnArrow` keeps clicks on the row
  for selecting; `SetNextTreeNodeOpen` expands or reveals.
* **`TextEditor`** is the multi-line field: the text field's editing (graphemes, selection, undo, the clipboard, IME
  composition) over paragraphs, wrapped at the width or scrolled sideways, Up / Down by visual line, a line-number
  gutter, the monospace font, Tab as a character. Only the lines in view are laid out, so long files stay cheap.
* **Docking.** `DockSpace` is an area windows dock into. A window dragged by its header over it shows glass targets -
  the middle of a node (its tabs) or an edge (beside it) - and docks there when let go; a tab dragged out of its node
  floats the window again, still under the pointer. The splitters between nodes resize them. Docked windows fill their
  node under a glass tab bar, stay behind floating windows, and are ordinary `BeginWindow` code. `SaveDockLayout` /
  `LoadDockLayout` keep the layout as text with the application's settings.

## 13. Status: what of WGT is ported

Ported, in the order the parts depend on each other:

* the theme, springs and easing;
* style resolution (WGT's `look.cpp`: `Next`, scopes, glass looks, surfaces, state fills);
* the interaction core;
* text;
* every control in section 4, with the liquid selection of segmented controls (WGT's `selection.cpp`);
* cards, scroll areas and windows;
* sections and rows (WGT's `lists.cpp`), except `RowPicker`, which waits for the picker;
* navigation, the tab bar and the search bar (WGT's `navigation.cpp`);
* the text field (WGT's `text_edit.cpp`);
* menus, the picker and `RowPicker`, tooltips (WGT's glass popups);
* drag-to-scroll and the scroll indicator (WGT's `ScrollAreaEnd`);
* the line chart;
* stacks, adaptive stacks, grids and flows;
* glow halos (WGT's item map, section 4);
* the island, notifications, live activities and the dock (WGT's `overlay.cpp`, section 10);
* WGT's demo: Settings, Effects Lab, Control Center, Components, Languages, Telemetry and the plugin panel, compared
  with WGT's screenshots (section 11).

WGT's layout matches pixel for pixel because Esia now does what Dear ImGui did under WGT: layout positions snap
down to the pixel (`Context::Snap`, [UI_CORE.md](UI_CORE.md) section 6), and the item spacing is WGT's (section 2).

Two WGT bugs were fixed on the way:

* WGT keyed the progress widgets' animations by their screen position, so they restarted when the page scrolled.
  They now take a per-frame serial under the widget's id.
* A stack inside a flow took the full width. It is now as wide as its content unless it has something to fill with
  (section 8).

Not ported: WGT's plugin loading (the plugin panel is drawn by the showcase itself). Color emoji, which WGT had from
DirectWrite, come from the FreeType text system now: bitmap strikes and COLR fonts (Segoe UI Emoji, Noto Color
Emoji, Apple Color Emoji).

Beyond WGT: the data widgets of section 12, and scrolling by touch from anywhere (section 5).

**Tests** (`tests/ui/test_ui.cpp`, `esia_ui_tests`) run without a GPU, on a `Context` driven frame by frame:

* springs (frame-rate independence, rest, overshoot);
* theme blending and the animator;
* style merging (`Next` taken by one widget, nested scopes);
* stack, flex, flow and grid placement;
* a button click and a switch flip through queued input;
* rows without gaps in their section, sections one after another;
* a row button, a row's switch (and not the rest of its row);
* the segmented control: a tap lands on release, a drag lands on the nearest segment, and it comes to rest;
* navigation: a push shows both pages while it moves and then only the new one; the back button pops;
* the tab bar: its place at the bottom of its area, the room it reserves, a tap; its draw commands last in the
  window's list, also inside a card;
* the text field: typing, graphemes, selection, select all, delete, undo, redo; the clipboard, a paste on one
  line; Tab to the next field; Enter; a password (not copied) and the length limit; the IME composition shown but
  not written, its committed text written;
* popups: a picker's menu opens and takes a choice; the click that closes a menu does not reach the widget under
  it; a menu item closes its menu; a tooltip waits half a second, goes when the mouse leaves and waits again;
* scrolling: the content follows a drag and glides on after it; the indicator dragged to the bottom takes the
  content to its end; content past the display fades at the display's edge; a finger's swipe from a row, and from a
  control that acts on the press, scrolls without pressing it, a tap still presses and a sideways drag stays the
  control's; a drag held past the end at a phone's density keeps the content on the same pixel;
* the data widgets (`tests/ui/test_data_widgets.cpp`): a number field scrubbed, typed as an expression and stepped; a
  table sorted, its rows selected with Ctrl and Shift, only the rows in view asked for; tree nodes opened by their arrow and
  their children submitted while they slide; the editor's lines, newlines and paste; the color picker's hex field;
  dock layouts saved and loaded, a window dragged into a dock space, a tab dragged out;
* the line chart: the room it takes, none when it draws into a rect;
* glow halos: a glowing child gets room for its glow in a stack;
* the island and the dock: a notification and an activity from another thread open the island; a dock tile click
  toggles its panel;
* a group that fills the width is a flexible child (Control Center's modules side by side);
* springs stepping once per frame.

What renders is checked in the showcase's screenshots on every backend.

## 14. Pitfalls

What applications built on the widgets ran into, and what now catches it.

* **Ids come from labels.** Two widgets with one label in one id scope are one widget to the core: they share the
  hover, their presses and their state (an animation, a field's text, a scroll offset). Buttons made in a loop are
  the usual case. Give each pass a scope, or the label a hidden suffix:

  ```cpp
  for (int i = 0; i < (int)items.size(); ++i) {
      ui::IdScope scope(i);                       // or an object's address, a name
      if (ui::Button("Set value")) items[i].Set();
  }
  ui::Button("Apply##left");  ui::Button("Apply##right");   // "##": the rest is the id, not shown
  ```

  Windows, tables, the multi-line editor and sections are scopes of their own. A debug build reports two items with
  one id, with the window, the id and its label, and outlines both in red ([UI_CORE.md](UI_CORE.md) section 16).
  `ui::IdScope` lives inside one window.
* **A widget pushed out of view is not drawn, and nothing says so.** An item laid out past its window's right edge
  is clipped away. A widget that fills the width (`AvailableWidth()`) leaves the items after it on its line (`SameLine`)
  the room they took last frame, so a field no longer pushes its button out; a row that is simply too wide still does.
  A debug build reports an item that stays out of view for a second, on an axis nothing scrolls.
* **Tables.** Call `TableVisible(rowCount)` before the rows: it returns the rows in view, and with it a million rows
  cost what the visible ones cost. `maxHeight` fits the table to its rows up to that height; with `height` too, the
  smaller wins.
* **Phone defaults on a desktop.** The sizes are iOS's: `ThemeCompact` for a denser tool (section 2). Scroll
  indicators show at rest on desktops and hide on phones (`metrics.scrollIndicatorAlways`); a mouse scrolls with the
  wheel and the indicator, a finger drags the content (`InputConfig::mouseDragScrolls` lets a mouse drag it too).
* **The frame.** `Ui::NewFrame` after `Context::NewFrame`, `Ui::EndFrame` before `Context::EndFrame`; the `ui::`
  functions act on the thread's current `Ui` in between. An event-driven host renders again while
  `Ui::Animating()` or `Context::InputPending()` is true.
* **Auto-sized windows** (`WindowFlags_AutoSize`: menus, tooltips) measure their content in the frame they appear and
  show from the next one.
