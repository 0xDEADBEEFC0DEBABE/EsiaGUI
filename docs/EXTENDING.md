# Extending WGT

Everything the built-in widgets do is available through the public headers. This guide goes from the
smallest extension (a custom widget) to the largest (a render backend).

* [Custom widgets](#custom-widgets)
* [Auto layout](#auto-layout)
* [Painter & Style](#painter--style)
* [Motion](#motion)
* [Themes & materials](#themes--materials)
* [Custom HLSL effects](#custom-hlsl-effects)
* [Panels](#panels)
* [Plugins](#plugins)
* [Text & fonts](#text--fonts)
* [Scale model](#scale-model)
* [Automated UI tests](#automated-ui-tests)
* [Custom render backends](#custom-render-backends)

---

## Custom widgets

A widget is three things:
* **interaction**: `ui::Interact`, a hit-tested, id-stable item in the ImGui layout;
* **motion**: `anim::*`, springs keyed by the item id;
* **drawing**: `Painter`.

A round Control Center toggle is a good example. The library version is `ui::ToggleButton`; this
simplified one uses only public API:

```cpp
bool CircleToggle(const char* id, bool* on, wgt::Icon icon, wgt::Color tint, float diameter)
{
    using namespace wgt;
    ui::Interaction it = ui::Interact(id, Vec2(ui::S(diameter), ui::S(diameter)));   // S() = metrics scale
    if (!it.visible)
        return false;

    const ImGuiID popId = it.id ^ 0x9E3779B9u;
    static const Spring kInk{0.36f, 0.86f};   // response (s), damping (1 = critical)
    static const Spring kPop{0.32f, 0.42f};   // bouncy
    if (it.pressed)
    {
        *on = !*on;
        anim::Kick(popId, 30.0f);             // impulse -> springy bulge
    }
    const Theme& t = CurrentTheme();
    const float k = anim::Float(it.id, *on ? 1.0f : 0.0f, &kInk);
    const float pop = anim::Float(popId, 0.0f, &kPop);

    Painter p;
    const Vec2 c = it.rect.Center();
    const float r = ui::S(diameter) * 0.5f;
    p.PushScale(c, 1.0f - 0.08f * it.press + 0.07f * pop);   // it.press: animated 0..1 while held
    p.Circle(c, r, Style().Glass(t.materials.control).Fill(t.colors.fill));
    if (k > 0.002f)
        p.Circle(c, r * (0.35f + 0.65f * std::min(k, 1.0f)), Style().Fill(tint).Glow(tint, ui::S(10), 0.45f * k));
    p.Icon(c, icon, r * 0.84f, Lerp(t.colors.label, Color::White(), std::min(k, 1.0f)));
    p.PopScale();
    return it.pressed;
}
```

`ui::Interaction` gives:
* `hovered`, `held`, `pressed`;
* animated `hover` / `press` (0..1);
* the item `rect` and `id`.

Flags:
* `InteractFlags_PressOnClick`: fire on mouse-down instead of release;
* `InteractFlags_Repeat`;
* `InteractFlags_AllowOverlap`.

`ui::InteractRect(id, rect)` hit-tests an explicit rect without advancing the layout, e.g. for
sub-parts of a widget.

Stock `ImGui::` calls work next to WGT widgets and pick up the WGT theme, fonts and colors.

## Auto layout

Dear ImGui lays items out with a cursor: each item goes below the previous one, or beside it after
`SameLine()`. WGT adds containers that *measure* their children and place them from the space they are
given, so a page re-flows with the window size:

```cpp
// wraps like text: as many per line as fit
if (ui::BeginFlow("buttons"))
{
    ui::Button("Play");
    ui::Button("Settings");
    ui::Button("Quit");
    ui::EndFlow();
}

// as many columns as fit (>= 150 wide each, at most 4); cells stretch to fill the row
ui::GridOptions g;
g.minColumnWidth = 150;
g.maxColumns = 4;
if (ui::BeginGrid("tiles", g))
{
    for (Item& item : items)
        DrawTile(item);          // one child per tile
    ui::EndGrid();
}

// a row whose flexible children share the free width
if (ui::BeginHStack("toolbar"))
{
    ui::IconButton("back", icons::Back);
    ui::SearchField("q", query, sizeof(query));   // asks for the available width -> flexible
    ui::FlexSpacer();                             // or: ui::LayoutFlex(2) before a child
    ui::IconButton("more", icons::More);
    ui::EndStack();
}
```

| Container | Places children |
| --- | --- |
| `BeginVStack` | top to bottom; `align` puts narrower children at the start, center or end |
| `BeginHStack` | left to right; flexible children share the free width, `justify` distributes the rest |
| `BeginAdaptiveStack` | a row while the children fit side by side, a column otherwise (like SwiftUI `ViewThatFits`) |
| `BeginGrid` | adaptive (`minColumnWidth`) or fixed (`columns`) columns, `LayoutSpan(n)` for wider cells, row height = tallest cell, `cellHeight` or `aspect` |
| `BeginFlow` | in lines that wrap; each line can be justified |

Rules that make it automatic:
* **Children are detected, not declared.** Every item submitted directly inside a container is one child:
  * a widget;
  * an `ImGui::BeginGroup/EndGroup` block;
  * a nested container.

  Group a multi-part cell so it counts once.
* **Flexible children are detected too.** A widget that asks for the available width
  (`ui::AvailableWidth()`, sliders, fields, `width = -1` buttons, wrapping text) stretches to the space it
  is offered. `ui::LayoutFlex(n)` sets an explicit share.
* **Measure, then place.** Positions come from the sizes measured on the previous frame, so they are
  exact without guessing. A container's very first frame is measured invisibly, so nothing ever flashes
  in the wrong place.
* **Re-flows animate.** Children glide to their new slots on a spring (`animate = false` to snap).
  Positions are relative to the container, so dragging or scrolling a window never lags.
* **Room for glows.** Glowing children automatically get extra spacing so their glow does not cover a
  neighbour. Only the glow that reaches outside the child counts: a glow drawn inside a tile or card that
  contains it changes nothing.

`ui::GetSizeClass()` returns `Compact` / `Regular` / `Expanded` for the available width in design units.
Use it to switch page structure, e.g. a sidebar only when `Expanded`.

The Control Center panel in the demo is an adaptive grid of modules:
* each module is a group sized from `ui::AvailableWidth()`;
* the sliders and the small toggles are nested stacks;
* resizing the panel moves it between 1, 2 and 4 columns.

## Liquid selection

Segmented controls (`Segmented`, `RowSegmented`) and the tab bar share one selection model
(`src/ui/selection.cpp`):
* tap an item and the selection travels there; press on it and drag and it follows the pointer (the grab
  point stays under it), then lands on the nearest item when released;
* while held or travelling it is a clear lens (`magnify`, rim dispersion) that enlarges the items it passes
  over and rises past the track; it swells out of the resting pill with a pop and melts back into it where
  it lands;
* position and lens amount are springs, so pressing, dragging, releasing and retargeting mid-flight are
  continuous.

## Painter & Style

`Painter` draws into the current window (or any `ImDrawList*`). Coordinates are UI units (ImGui screen
space). Each shape is one GPU instance, so cost does not grow with size or effect count.

| Shapes | |
| --- | --- |
| `Rect(r, style)` | rounded rect; radii from `Style::Radius` (per corner), continuous corners |
| `Capsule(r, style)` | fully rounded rect |
| `Circle(c, radius, style)` / `Ring(c, radius, thickness, style)` | |
| `Arc(c, radius, thickness, start, sweep, style)` | round caps, radians, clockwise from +x |
| `Line(a, b, thickness, style)` | round-capped segment |
| `Polyline(points, count, thickness, style, flags)` | one connected stroke (mitered joins, round caps, pixel AA); glows only if `style` has a glow, as one even halo. `PolylineFlags_Smooth`: monotone curve through the points |
| `Area(points, count, baseline, paint, flags)` | fills between the same path and `y = baseline` (area charts, vertical `Paint::Linear` fade) |
| `Merge(a, b, radius, smoothness, style)` | liquid union of two rects (metaballs, morphing pills) |
| `Image(tex, r, radius)` | SDF-masked image |
| `Text` / `TextBox` / `TextAligned` / `Icon` | WGT text engine; see [Text & fonts](#text--fonts) |

| State | |
| --- | --- |
| `PushMask(r, radius)` / `PopMask()` | rounded mask for following shapes |
| `PushClip` / `PopClip` | rectangular clip |
| `PushScale(origin, s)` / `PopScale()` | scale shapes and text (press / pop animations) |
| `SetAlpha(a)` | global opacity |
| `BeginGlowLayer(tint, radius, intensity, contentOpacity)` / `EndGlowLayer()` | GPU bloom around everything in between |
| `LightStreak(rect, intensity, thickness, speed, smile, open, time)` | Siri's light: a few thin, softened lines held together at both ends and opening in the middle (thin warm/green/lavender lines on top, a white line and bloom below); animated on the frame clock or on `time`. Draw clear glass over it to have it refracted |
| `BeginEdgeFade(region, top, bottom)` / `EndEdgeFade()` | what the draw list draws in between fades out towards the region's top / bottom edge (real transparency, no color). Scroll areas use it for their edges. |

`Style` is a fluent material description:

```cpp
Style()
    .Radius(ui::S(18))                                   // or Radius(tl, tr, br, bl); Smoothing(0..1)
    .Fill(Paint::Linear(Color::Hex(0x5E5CE6), Color::Hex(0xBF5AF2), 45))   // Solid / Linear / Radial / Conic / ConicLoop
    .Stroke(ui::S(1), Color::White(0.4f), 0.5f)          // align: 0 inside, 0.5 centered, 1 outside
    .StrokeFade(0.2f, 90)                                // fade the stroke along a direction
    .Shadow(Color::Black(0.3f), ui::S(24), Vec2(0, ui::S(8)))   // or InnerShadow(...)
    .Glow(accent, ui::S(16), 0.8f)                       // outer glow; InnerGlow(radius, intensity)
    .Glass(theme.materials.control)                      // liquid glass (see materials)
    .Image(texture, uv0, uv1)                            // image fill (tinted by the fill color)
    .Shimmer(0.35f, 0.6f)                                // skeleton-loading sweep
    .Noise(0.01f)                                        // film grain
    .Opacity(0.9f)
    .Effect(myEffect, p0, p1, p2, p3);                   // custom HLSL (see below)
```

`Paint::Spectrum(alpha, from, to, angle)` is a rainbow along a direction, fading out at both ends (caustics,
iridescent streaks). `Paint::Conic(a, b, start)` sweeps from `a` to `b` once around, so the two colors meet at `start` (right for
progress rings and spinner tails; the seam is anti-aliased). For a full ring or disc use
`Paint::ConicLoop(a, b, start)`: `a` → `b` → back to `a`, with no seam.


Shadows and glows extend beyond the shape. When you draw a background whose shadow must leave the ImGui
window (a floating card or popup), widen the draw-list clip for that draw, otherwise the shadow is cut
into a hard rectangle. The built-in windows, popups, tab bar and dock do this for you.

Outer glows are *contained* by default:
* layout containers reserve room for the part of the glow that reaches outside its child;
* whatever still reaches a neighbouring item fades out smoothly before that item's edge.

The neighbours come from the window's item map of the previous frame. Use `Style::ContainGlow(false)` for
decorative glows that should spill freely, e.g. hero art or backdrops.

Glow is always the caller's choice. A shape glows only when its `Style` has `Glow(...)`, and the built-in
widgets glow only when their options ask for it: `ButtonOptions::glow`, `ToggleButtonOptions::glow`,
`ProgressOptions::glow` and `LineChartOptions::glow`. Everything except the Control Center toggle is off by
default.

### Lines and charts

`Painter::Polyline` draws a path as one connected stroke: mitered joins, clipped on sharp turns so they don't
spike, round caps, and anti-aliasing to the physical pixel. Use it rather than a chain of `Line` segments,
whose caps overlap at every joint. Stacked glows and translucent colors would show those overlaps.
`PolylineFlags_Smooth` passes a curve through the points. When x keeps increasing, the curve is monotone and
never overshoots the data. `Painter::Area` fills between the same path and a baseline.

`ui::LineChart(id, values, count, LineChartOptions)` is the ready-made widget (iOS Stocks / Health):
* a smooth line over a soft gradient, with a dot on the latest value;
* the value range fits the data and eases when it changes, or you can fix it with `min` / `max`;
* `offset` reads a ring buffer from its oldest sample;
* `rect` draws the chart into a row or card instead of giving it its own place in the layout.

```cpp
ui::LineChartOptions o;
o.offset = cursor;      // ring buffer: index of the oldest sample
o.smooth = true;        // false: straight segments
o.fill = true;          // gradient under the line
o.glow = false;         // opt-in: one even halo around the whole line
ui::LineChart("fps", history, 120, o);
```

## Motion

| API | Use |
| --- | --- |
| `anim::Float(id, target, &spring)` | spring toward `target` (also `Vector`, `Colour`) |
| `anim::Velocity(id)` | velocity of a `Float` spring, for stretch/squash |
| `anim::Kick(id, velocity)` | impulse: pops, shakes, rubber-banding |
| `anim::Set(id, value)` | jump without motion (replay an appear animation) |
| `anim::Timer(id, seconds, restart)` | linear 0..1 timer |
| `anim::Time()`, `anim::DeltaTime()` | UI clock |
| `ease::*` | OutCubic, OutBack, OutExpo, InOutCubic, Smooth (for timers) |

`Spring{response, damping}` follows SwiftUI:
* `response` is the period in seconds;
* `damping` of 1 is critically damped and below 1 is bouncy.

Presets: `Spring::Snappy()`, `Smooth()`, `Bouncy()`, `Gentle()`, and the theme's `motion.fast` /
`motion.standard` / `motion.bouncy`. Keys are `ImGuiID`s: salt the widget id per animated property.

Liquid-motion recipes used by the built-in controls (`src/ui/controls.cpp`):
* **Stretch along the motion:** `stretch = min(|velocity| * 0.05, max)`. Widen the shape by it, thin it a
  little.
* **Lens while pressed:** swap the solid fill for a clear `GlassMaterial` while held and for ~0.2 s after
  each activation.
* **Labels follow the pill:** highlight by coverage `1 - |animatedIndex - i|`, not by the selected index.
* **Squash at the wall:** clamp the pill to its track and let the height bulge by the overshoot.

## Themes & materials

A `Theme` is plain data:
* `colors` (`Palette`): the iOS semantic colors — labels, fills, backgrounds, system colors, and surfaces;
* `materials`: `window`, `bar`, `control`, `popover`, `clear`;
* `metrics`: radii, spacing, hairline, and the current scale;
* `type`: text styles;
* `motion`: springs and theme transition time.

```cpp
wgt::Theme t = wgt::ThemeDark();                 // or ThemeLight(), ThemeWithAccent(base, color)
t.colors.accent = wgt::Color::Hex(0xFF9F0A);
t.materials.window.blur = 20.0f;                  // frostier windows
t.materials.window.legibility = 0.6f;             // stronger contrast protection
ctx->SetTheme(t);                                 // any thread; animates by default
```

`SetDarkMode` and `SetAccent` are shortcuts, and the change animates as well. `ctx->GetTheme()` /
`wgt::CurrentTheme()` return the *current, animated* theme on the UI thread.

### Per-component style

Any single component, or everything in a scope, can be styled without touching the theme. `ui::ItemStyle`
is a set of modifiers in the style of SwiftUI: only the fields you set change, and everything else follows the
theme. `ui::Next()` styles the next component with one line, with no Push/Pop pair:

```cpp
ui::Next().Tint(Color::Hex(0x30D158)).Radius(8);                 // accent + corner radius
ui::Button("Play");

ui::Next().Look(GlassLook::Frosted).Blur(24).Fill(purple);       // a filled button becomes purple frosted glass
ui::Button("Frosted");

ui::Next().Look(GlassLook::Clear).Refraction(20).Dispersion(0.9f); // fully transparent, strong lens
ui::Slider("vol", &vol, 0, 1);

ui::Next().Opacity(0.5f);
ui::Toggle("wifi", &wifi);
```

| Modifier | Changes |
| --- | --- |
| `Look(GlassLook)` | `Theme`, `Clear` (fully transparent) or `Frosted` (iOS frost) |
| `Blur` `Refraction` `Bezel` `Dispersion` `Saturation` `Brightness` `Specular` `Legibility` `Magnify` `GlassTint` `Rim` | single glass-material fields |
| `Glass(GlassMaterial)` | the whole material |
| `Tint(Color)` | the component's accent: filled button, switch "on", slider, progress, checkmark, caret, selected tab, row link |
| `Fill(Color)` | its surface: button, field, track, segmented control, section / card / window background, badge |
| `Radius(float)` | its corner radius (UI units) |
| `Opacity(float)` | the whole component (including text and stock ImGui widgets inside) |
| `Label(Color)` | its text and symbols |
| `SelectedFill(Color)` | segmented control / tab bar: the selected item's background once a switch has landed |
| `MovingFill(Color)` | segmented control / tab bar: the selection while it travels (the liquid lens); `a` = how strongly it is tinted |
| `SelectedLabel(Color)` | segmented control / tab bar: the selected item's text (it takes the color as the pill lands) |

A segmented control therefore has a color for each part: the track (`Fill`), the selection while it moves
(`MovingFill`), the selection once it lands (`SelectedFill`), and the texts (`Label` / `SelectedLabel`). On a
custom track with no `SelectedFill`, the selection defaults to a light, see-through pill that sits on any color.

```cpp
ui::Next().Fill(blue).SelectedFill(Color::White(0.92f)).MovingFill(Color::White(0.25f))
          .Label(Color::White()).SelectedLabel(blue);
ui::Segmented("range", &range, items, 3);
```

A flat component (filled button, switch track, field, card ...) turns into glass when its style asks for
glass. That happens with `Look(Clear)`, `Look(Frosted)` or any glass field, and its fill color then tints the
glass. Under `Clear` every surface becomes clear glass (lens, rim light, hairline). State colors ("on",
selection, progress) stay but become see-through, and labels that sat on a solid color take the label color,
so they stay legible over anything. `Theme` keeps flat surfaces flat.

Scopes: before `BeginWindow` / `BeginSection` / `BeginCard` / `BeginRow`, `ui::Next()` styles that container
and everything inside it. `ui::PushItemStyle` / `PopItemStyle`, or the RAII `ui::StyleScope`, cover any range.
Scopes nest, and the innermost wins field by field. The shorthands `SetNextItemLook` / `PushGlassLook` /
`PopGlassLook` set only the look. `Context::SetGlassLook` sets the default for the whole UI (`Frosted`).
Explicit widget options (`ButtonOptions::tint`, `CardOptions::radius`, ...) still win over the style.

Panels created with `Context::AddPanel` take `PanelFlags_ClearGlass` for a clear window surface, and
`WindowFlags_ClearGlass` does the same for `ui::BeginWindow`. The search bar has its own `look` (default
`Clear`) and a soft `base` color inside (on by default). A `Clear` style makes the whole bar transparent,
base included.

Custom widgets follow along. `ui::Interact` / `InteractRect` take the pending `ui::Next()` style and return it
in `Interaction::style`. Draw inside `ui::StyleScope s(it.style);` and read `ui::CurrentItemStyle()`,
`ui::AccentColor()` and `ui::LookMaterial(theme.materials.control)`, which already include the style.

### Glass looks

* `GlassLook::Theme`: the theme's material for that kind of surface (window, bar, control, popover);
* `GlassLook::Clear`: fully transparent. Only the lens, the rim light and the hairline remain; no frost, tint
  or surface fill;
* `GlassLook::Frosted`: iOS frost, the default. A soft blur under a light veil that the shader lays in layers
  (heavier on the curved bevel, lighter in the body, lit from the top), with only a gentle exposure for
  legibility.

`GlassMaterial` fields (distances in UI units at scale 1):

| Field | Effect |
| --- | --- |
| `blur` | frost: backdrop blur radius (0 = crystal clear) |
| `refraction` | lensing: how far the content behind is pulled in (magnified) at the rim |
| `bezel` | radius of the curved edge, i.e. the width of the lensing band; the interior beyond it is undistorted. At half the shape's size or more, the whole shape is one rounded rod / dome (bars and controls). |
| `dispersion` | chromatic separation in the lensing band (0..1) |
| `magnify` | loupe: the content behind is enlarged by `1 + magnify` around the center (selection lenses, dragged knobs) |
| `saturation`, `brightness` | vibrancy of the backdrop |
| `legibility` | 0..1: exposes the backdrop toward the tint's brightness, judged over a wide neighbourhood. Dark glass stays dark over a bright wallpaper and light glass stays light over a dark scene, while hue and detail survive. |
| `tint` | rgb + amount mixed over the result |
| `specular`, `lightAngle` | rim highlight strength and direction |
| `rim` | hairline edge stroke |
| `noise` | film grain against banding |

## Custom HLSL effects

Register an effect once (any thread). It starts compiling in the background right away, for each backend:

```cpp
const char* kAurora = R"(
float4 WgtEffect(WgtFx fx)
{
    float t = fx.time * 0.55;
    float w = sin(fx.uv.x * 5.0 + t) * 0.5 + 0.5;
    float3 col = lerp(float3(0.1, 0.95, 0.75), float3(0.45, 0.25, 1.0), w);
    float3 behind = WgtBackdrop(fx.screenUV, 18.0);   // blurred backdrop, blur in pixels
    col = lerp(behind, col, 0.7);
    float a = fx.coverage * fx.params.x;
    return float4(col * a, a);                        // premultiplied
}
)";
wgt::EffectId aurora = ctx->RegisterEffect("aurora", kAurora);
...
p.Rect(r, wgt::Style().Radius(ui::S(20)).Effect(aurora, /*params.x*/ 1.0f));
```

`WgtFx` fields:

| Field | Meaning |
| --- | --- |
| `pos` | position in UI units |
| `uv` | 0..1 inside the shape bounds |
| `size` | shape bounds size |
| `screenUV` | 0..1 in the render target |
| `sd` | signed distance to the shape edge (negative inside) |
| `coverage` | anti-aliased coverage — multiply your alpha by it |
| `px` | UI units per pixel |
| `time` | seconds |
| `params` | the 4 floats from `Style::Effect(id, p0, p1, p2, p3)` |
| `fill` | the evaluated fill / gradient (straight alpha) |

Helpers:
* `WgtBackdrop(screenUV, blurPx)`: the live backdrop, blurred. This makes the shape a glass shape, so the
  backdrop is captured for it.
* `WgtTexture(uv)`: the shape's image texture, from `Style::Image`.

Compile errors go to the log callback, and the shape falls back to the built-in shader. Effects can also
come from plugins (`examples/plugin_hello`).

## Panels

Panels are dock-launched windows owned by the context. Register them from any thread:

```cpp
wgt::PanelDesc d;
d.id = "inventory";            // stable id (also the ImGui window id)
d.title = "Inventory";
d.icon = wgt::icons::Apps;
d.iconColor = wgt::Color::Hex(0x30D158);
d.size = wgt::Vec2(420, 600);  // UI units at scale 1
d.flags = wgt::PanelFlags_None;   // HideFromDock, NoClose, NoResize, Solid, NoScroll
ctx->AddPanel(d, [](wgt::Context& c) { /* widgets, UI thread */ });
ctx->SetPanelOpen("inventory", true);
```

For free-standing windows, use `ui::BeginWindow` / `ui::EndWindow` in your own frame code. Inside windows
you can use:
* grouped lists: `BeginSection` and the `Row*` family;
* cards: `BeginCard`;
* navigation stacks: `BeginNavigation` / `BeginPage` / `NavigationPush`;
* tab bars, scroll areas, pickers (glass menus) and tooltips.

## Plugins

A plugin is a DLL that exports two C functions. `WGT_DECLARE_PLUGIN` writes them:

```cpp
#include <wgt/wgt.hpp>

class MyPlugin final : public wgt::IPlugin
{
public:
    const char* Name() const override { return "My Plugin"; }
    void OnAttach(wgt::Context& ctx) override { /* AddPanel, RegisterEffect, SetTheme ... */ }
    void OnFrame(wgt::Context& ctx) override { /* per-frame UI, inside the frame */ }
    void OnDetach(wgt::Context& ctx) override { ctx.RemovePanel("my.panel"); }
};
WGT_DECLARE_PLUGIN(MyPlugin)
```

The host calls `ctx->LoadPlugin(path)` or `ctx->LoadPluginsFromDirectory(dir)`. In-process plugins use
`ctx->AddPlugin(&instance)`.

ABI rules:
* The creation function receives `WGT_ABI_VERSION` and returns null on mismatch, so an incompatible
  plugin is skipped instead of crashing.
* Callables cross the DLL boundary only as `wgt::Callback<>`, which is made of raw pointers (no STL
  types).
* The thread-safe containers in `sync.hpp` are header-only.

As a result, plugins may be built with a different CRT or STL configuration than the host.

## Text & fonts

* Text styles follow iOS Dynamic Type: `LargeTitle`, `Title1..3`, `Headline`, `Body`, `Callout`,
  `Subheadline`, `Footnote`, `Caption1..2`, `Mono`. Use `ui::Text(TextStyle::Body, "fmt", ...)`,
  `Painter::Text(pos, style, color, text)`, or `GetFont(style)` for a `FontRef`, which is already scaled.
* Other sizes and weights: `GetFont(FontWeight::Semibold, 13.5f)`. The size is unscaled; the metrics scale
  is applied for you.
* Other families: `FontId id = wgt::RegisterFont("Inter", 600);` then `FontRef{id, ui::S(15)}`. Families
  come from the system or from files registered with `ContextDesc::fontFiles` /
  `Context::AddFontFile(L"C:/game/fonts/MyFont.otf")`.
* Defaults are set in `ContextDesc`: `fontFamily`, `fontFamilyDisplay`, `monoFamily`, `iconFamily` and
  `locale`. `locale` drives fallback and CJK glyph variants, e.g. `"zh-CN"` vs `"ja-JP"`.
* Layout helpers:
  * `Painter::TextBox(rect, align, font, color, text, end, TextFlags_Ellipsis)`;
  * `TextFlags_AlignCenter` / `TextFlags_AlignRight` for paragraphs;
  * `MeasureText(font, text, end, wrapWidth)`, which returns size, first baseline and line count.
* Stock ImGui text editing uses a DirectWrite-backed `ImFont`:
  `ImGui::PushFont(wgt::GetImGuiFont(FontWeight::Regular), size)`.

Never bake pixel sizes. Fonts are rasterized at the exact physical size, so the same code is crisp at
100 %, 150 % and 200 % DPI and at any render scale.

## Scale model

| | |
| --- | --- |
| **UI units** | window client pixels. Mouse input, ImGui layout and every Painter coordinate. |
| **metrics scale** | DPI × user scale. Multiply every design size by it: `ui::S(12)`, or `theme.S(12)`. |
| **render scale** | render-target pixels per UI unit. Automatic (target size ÷ window size) or `SetRenderScale`. |

* Design at scale 1 (points, like iOS) and wrap every length in `ui::S()`. Built-in widgets do; custom
  widgets should too.
* `SetUiScale(1.18f)` enlarges the whole UI (the demo's Text Size S/M/L). Open windows keep their
  proportions.
* DPI is followed per monitor (`WM_DPICHANGED`). `SetDpiScale(x)` pins it.
* If the game renders at a different resolution than the window (dynamic resolution, super-sampling),
  nothing is needed: WGT reads the target size every frame. Text and SDF edges use the physical pixel
  density, and blur radii are converted in the shader.
* `ctx->GetScaleInfo()` reports the current DPI, user, metrics and render scales.

## Text input

`ui::TextField` / `ui::SearchField` are WGT's own editor, not `ImGui::InputText`. They work in every script
with no setup:

* the caret moves by grapheme clusters (emoji sequences, combining marks, Indic clusters), and Ctrl moves
  by words;
* selection by drag / double-click / triple-click / Shift, clipboard, undo / redo;
* IME (Chinese / Japanese / Korean) composes inline, with the candidate window at the caret;
* the buffer is UTF-8. Edits that would not fit `bufferSize` are rejected, never truncated mid-character.

For IME, the host must forward `WM_IME_*` messages to `HandleWin32Message`. Forwarding every message, as
the quick start does, covers this.

## Layout inspector

`ContextDesc::debugLayout = true` (or `Context::SetDebugLayout(true)`; `--debug-layout` in the demo)
checks every frame for two kinds of problem:
* items at the same nesting level that partially overlap, outlined in red;
* items cut off by their window's right edge (content nobody can see), outlined in orange. An item only
  passing through, such as a page slide or a container's measuring frame, isn't reported; it has to stay
  there for half a second. Windows with a horizontal scrollbar, and windows that are only partly on screen,
  aren't reported either.

Each problem is logged once.

The built-in widgets are designed so this never fires:
* text never runs past its container. `ui::Text` / `TextColored` / `TextSecondary` / `Headline` /
  `LargeTitle` wrap at the container's edge when the line is wider than the room left, and that includes
  text after `SameLine` items. Inside auto-layout containers the container sizes the text instead;
* side-by-side groups that must not be cut off belong in an auto-layout container (`BeginFlow`,
  `BeginAdaptiveStack`), which moves them onto a new line when they don't fit. Raw `ImGui::SameLine`
  places an item even past the edge;
* rows measure their accessory, ellipsize the label and stack when too narrow;
* the scroll indicator lives in a padding lane or a reserved gutter;
* window content is clipped short of the rounded corners (by exactly how far the corner curve reaches in at
  the content's side inset), so nothing scrolled to an edge pokes out of the window;
* floating glass bars (`TabBar`, `SearchBar`) float over the content like iOS: it scrolls under the glass and
  shows through it, the bar takes the clicks over its area, and scroll room below keeps every row reachable;
* scroll areas fade their content out towards an edge it can still scroll past, instead of cutting it (the
  fade width grows with the distance left to scroll). The search bar is clear glass by default;
  `SearchBarOptions::fill` gives it an optional base color inside the field;
* titles end before the close button.

## Automated UI tests

* `Context::InjectMousePos / InjectMouseButton / InjectMouseWheel / InjectKey / InjectText` feed input
  from any thread. It is applied at the next `NewFrame`, on top of OS input; `EndInputInjection()` hands
  the cursor back.
* `ContextDesc::fixedDeltaTime` makes animations deterministic, so frame N always looks the same.
* The demo combines both:

```bash
wgt_demo.exe --open settings --fixed-dt 0.016667 --script "30:move 373 660;31:down;35:up;36:shot a.png;40:shot b.png"
```

This workflow was used to tune the liquid toggle, slider and segmented animations frame by frame.

## Custom render backends

Implement `wgt::IRenderBackend` (`wgt/backend.hpp`) and pass it as `ContextDesc::customBackend` with
`Backend::Custom`:
* `RenderDrawData` receives Dear ImGui's `ImDrawData`.
* WGT commands are `ImDrawCmd`s whose `UserCallback == fx::CommandCallback()`. Decode them with
  `fx::ReadShape` / `fx::ReadLayer` / `fx::ReadFade`. `FadeBegin` / `FadeEnd` scope an edge fade over the
  following draws of the same draw list: multiply their alpha by the ramp in `WgtEdgeFade`
  (`wgt_common.hlsli`); ignoring it only leaves scroll edges hard. The `fx::Instance` layout is documented field by field in
  `wgt/fx.hpp` and `src/shaders/wgt_fx.hlsl`.
* The HLSL in `src/shaders/` is backend-neutral. Use it as the reference for the SDF, material and glass
  math.
* A backend that ignores FX commands still renders all regular ImGui geometry.
