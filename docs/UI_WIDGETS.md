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
* [7. Navigation and the tab bar](#7-navigation-and-the-tab-bar)
* [8. Auto layout](#8-auto-layout)
* [9. Custom widgets and animation](#9-custom-widgets-and-animation)
* [10. The showcase](#10-the-showcase)
* [11. Status: what of WGT is ported](#11-status-what-of-wgt-is-ported)

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
textSystem.NewFrame(...);   // the host, as without the Ui
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

## 2. The theme

A `Theme` (`theme.hpp`) is a plain struct holding every design token. Widgets never hard-code a color or a size.

| Part | Holds |
| --- | --- |
| `Palette colors` | accent, labels (4 levels), backgrounds, fills, separators, the 13 system colors, and surfaces: window tint, card, knob, shadow, highlight |
| `Materials materials` | glass materials: `window`, `bar`, `control`, `popover`, `clear` |
| `Metrics metrics` | `scale` (every size goes through `ui::S`), radii, paddings, spacing, control heights |
| `Typography type` | a size and weight per `TextStyle` (Apple's type ramp, LargeTitle .. Caption2, Mono) |
| `Motion motion` | the springs widgets use (hover, press, toggle, layout ...) |

* **Built-in themes.** `ThemeLight()` and `ThemeDark()` hold WGT's values. `ThemeWithAccent(theme, color)` changes
  the accent and what derives from it.
* **Changing the theme.** `Ui::SetTheme(theme, animate)`, `SetDarkMode(dark)` (keeps the accent) and
  `SetAccent(color)` cross-fade over the theme's transition time; an accent set with `SetAccent` stays across
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
| `Segmented(id, &i, {"A", "B", "C"}, width)` | the iOS 26 segmented control: tap a segment, or grab the selection and drag it; it travels as a clear lens that magnifies what it passes, then lands on the nearest segment as a solid pill. `*i` changes on release |

* **Ids.** Labels may carry an id suffix: `"OK##dialog"` shows "OK", and `"##x"` shows nothing.
* **Return values.** The functions that change a value return true in the frame they change it.

## 5. Windows, cards, scroll areas

* **`BeginWindow(title, &open, {size, pos, flags, subtitle, icon})`** is a liquid-glass window: a glass surface
  with a shadow, a header and a body that scrolls smoothly.
  * The header has the close button (when `open` is given), an icon tile, and the title and subtitle; the
    subtitle is cut with an ellipsis.
  * The body fades at its edges and is clipped to the window's rounded corners.
  * When closed, the window fades out before `*open` turns false.
  * The core moves and resizes it; the resize corner shows as an arc.
  * First-use positions cascade when `pos` is negative.
  * Flags: `NoClose`, `NoResize`, `NoMove`, `NoHeader`, `NoScroll`, `Solid`, `LargeTitle`, `NoShadow`, `NoPadding`,
    `ClearGlass`.
* **`BeginCard(id, size, {glass, radius, fill, shadow, padding})`** is an inset card.
  * Its background is drawn at `EndCard`, once its height is known, and moved under the content
    (`DrawList::Mark` / `MoveCommands`, [UI_CORE.md](UI_CORE.md) section 11). So there is no one-frame lag and no
    measuring frame.
  * With `size.x == 0` it fills the available width.
* **`BeginScrollArea(id, size)`** is a child region with the core's smooth scrolling and an edge fade.

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

## 7. Navigation and the tab bar

```cpp
ui::BeginNavigation("settings", "root");   // takes the room left in its area
if (ui::BeginPage("root", "Settings")) {
    if (ui::RowNavigation("Accent Color")) ui::NavigationPush("accent");
    ui::EndPage();
}
if (ui::BeginPage("accent", "Accent Color")) { ...; ui::EndPage(); }
ui::EndNavigation();

ui::TabBar("tabs", &tab, {{ui::icons::Apps, "Controls"}, {ui::icons::Palette, "Style"}});   // last in its area
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

## 10. The showcase

`examples/showcase` shows the widgets in a window over a drifting wallpaper. It uses the same frame as
`glass_window`: the `glass_app` library, with the Win32 platform layer, every backend, and a render thread.

* **Components**: three tabs under a tab bar: every button kind, switches, checkboxes, sliders, a segmented
  control and the stepper; progress bars and rings and badges; dark mode, the accent swatches and the glass look.
* **Control Center**: a grid of glass cards with round toggles, Now Playing, and a brightness slider spanning two
  columns.
* **Telemetry**: two gauges in a flow, and a card with wrapped mixed Latin / Chinese text.
* **Settings**: WGT's Settings screen without its search bar: a navigation stack whose root page has a custom
  profile row and sections of every row kind (the Liquid Glass section edits the live theme); Accent Color opens a
  second page.

```
showcase.exe --api d3d12 --dark --look frosted --size 1360x780 --frames 90 --fixed-dt 0.016667 --screenshot shot.png
```

`--tab 0|1|2` picks the Components tab and `--page accent` opens the Accent Color page. The options of
`glass_window` (`app.hpp`) apply: `--api`, `--size`, `--frames`, `--fixed-dt`, `--screenshot` and
`--debug`.

## 11. Status: what of WGT is ported

Ported, in the order the parts depend on each other:

* the theme, springs and easing;
* style resolution (WGT's `look.cpp`: `Next`, scopes, glass looks, surfaces, state fills);
* the interaction core;
* text;
* every control in section 4, with the liquid selection of segmented controls (WGT's `selection.cpp`);
* cards, scroll areas and windows;
* sections and rows (WGT's `lists.cpp`), except `RowPicker`, which waits for the picker;
* navigation and the tab bar (WGT's `navigation.cpp`), except the search bar, which waits for the text field;
* stacks, adaptive stacks, grids and flows.

Two WGT bugs were fixed on the way:

* WGT keyed the progress widgets' animations by their screen position, so they restarted when the page scrolled.
  They now take a per-frame serial under the widget's id.
* A stack inside a flow took the full width. It is now as wide as its content unless it has something to fill with
  (section 8).

Not ported yet, in the planned order:

1. TextField (WGT's `text_edit`) and the search bar; pickers and popups (and `RowPicker`), tooltips;
2. the scroll indicator and drag-to-scroll, the glow halo;
3. the island, the dock, notifications;
4. LineChart;
5. WGT's demo screens: Settings, Components, Effects, Control Center, Languages, Telemetry.

Each is checked against WGT's screenshots (branch `reference/wgt-1.1`).

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
* springs stepping once per frame.

What renders is checked in the showcase's screenshots on every backend.
