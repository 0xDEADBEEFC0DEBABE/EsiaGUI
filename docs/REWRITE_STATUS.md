# Esia rewrite - status of `esia-core`

Where the ImGui-free rewrite stands after core round 3 and the UI core's second version (2026-09-29). The plan is [REWRITE.md](REWRITE.md); how to
write a backend is [backends/README.md](backends/README.md).

**Round 3.** By the owner's decision, sub-pixel (LCD / ClearType) text was removed from Esia: text is antialiased
in grayscale everywhere (section 0). It came back on 2026-10-28, also at the owner's request ("Sub-pixel text"). The backends must delete their dual-source code after merging (section 0,
"Backend follow-ups"); the grayscale scenes render bit-identical.

**Before that, in one paragraph.** Round 1 built phase 0: the UI core without Dear ImGui, the RHI with a validating null
device, the shader pipeline, the renderer with WGT's techniques, the conformance suite, the text interface with
FreeType + HarfBuzz, the LLVM toolchains and the two documents (section 8). Four backend sessions then wrote OpenGL /
GLES, Vulkan, Metal and Direct3D 9 / 10 / 11 / 12 on their own branches, and a local Windows session ran them on an
NVIDIA RTX 4080 SUPER (section 3). Round 2 fixed on `esia-core` what those sessions and an independent review found: the
clang-cl build, the image goldens in core, a conformance suite that cannot mistake a skip for a pass and fails on API
validation messages, new scenes and a cross-frame mode, pyramid levels that stay defined when glass reuses a capture,
fallbacks for refused and still-compiling FX variants, targets that cannot be copied, the SM3 shader defects, and the FX
shader's per-pixel fetch of all 24 instance rows (section 1). Everything builds warning-free with clang 18 and every
core test passes; merged into scratch copies of the backend branches, the OpenGL, GLES and Vulkan suites (llvmpipe /
lavapipe) and the Direct3D 9 / 10 / 11 suites (under Wine) pass `--strict` in both modes (section 2). The backend
branches were not changed: what they had to adopt is in section 5 (the local session has done it, section 3).

**Text on every platform** (branch `feat/text-everywhere`, after round 3): FreeType and HarfBuzz are built from pinned
sources wherever the system has none, so the text system and its tests now build and run on Windows too; a system font
lookup and a fallback chain per platform cover Chinese, Japanese and Korean; CJK tests with a Noto Sans SC subset.

**UI core v2** (branch `feat/ui-core-v2`): the UI core reworked for the widget port; the design is
[UI_CORE.md](UI_CORE.md).

**Widget layer, first part** (branch `feat/ui-foundation`, 2026-09-30): `esia_ui` with WGT's theme, springs,
styles, text, controls, cards, windows, auto layout, inset grouped lists, navigation, the tab bar, text fields,
the search bar, menus, pickers, tooltips, scrolling and the line chart, and the `showcase` example. How to use it and what of WGT
is left: [UI_WIDGETS.md](UI_WIDGETS.md).

**Widget layer, second part** (branch `feat/ui-overlays`, 2026-10-01): glow halos, the island with notifications
and live activities, the dock, and WGT's demo ported panel for panel as the `showcase`, compared with WGT's
screenshots.

**Examples on Linux** (2026-10-02): `showcase` and `glass_window` run on an X11 window, with OpenGL and Vulkan.

**Data widgets, Android, color glyphs, touch** (2026-10-02): number, vector and color fields, tables, trees, a
multi-line editor and docking, with the `workbench` example; the X11 frame as a library; Android with OpenGL ES and
Vulkan; COLR color emoji; scrolling by touch from any item.

**Draw memos: widgets at rest draw nothing** (2026-10-30): a widget whose drawing has nothing new to draw from - its
rect, state, colors, label, the theme, the style - appends last frame's drawing again, byte for byte what drawing it
makes; texts measured again the same way are not looked up. Plain widgets at last under Dear ImGui by more than 20 %:
flat CPU 0.039 -> 0.025 ms against Dear ImGui's 0.032, regular 0.055 -> 0.041.

**Draw lists recorded and appended again** (2026-10-30, in Esia): a stretch of a draw list - what a widget drew -
kept as a `DrawRecord` and appended again where the list is in the state it began in, byte for byte what drawing it
again makes, with the text system's records of where texts were drawn (`DrawNote`) left as the draws would leave them:
the widget layer's draw memos stand on it.

**Glass shading cost** (2026-10-30, in Esia): what glass costs the GPU - the UI light nothing measurable (the shader paid
for it with or without it), a panel's backdrop capture 9 to 11 us, mostly its passes' fixed cost - and glass shaded 5 to
8 % faster on Direct3D 11 / 12, Vulkan and OpenGL with the same pixels: the light's direction and the rim's light computed
only where they are used, the bevel's normal from one corner.

**Vulkan frames in flight** (2026-10-30, in Esia): a frame's vertices, indices, instances and constants in rings per
frame in flight, in device memory the host maps where there is some (written in place; else copied from staging,
constants in host memory); upload barriers on what was written instead of one that waited for all earlier work;
`esia_bench --in-flight` measures frames the GPU is not waited for. RTX 4080 SUPER: GPU -22 % for glass (its
constants no longer read across PCIe), -46 % for 10 000 shapes, -50 % for a page of text; submit up to -12 %.

**SVG to textures** (2026-10-30, in Esia): SVG files parsed once and rasterized at the size drawn into texture pages, with
exact area coverage (overlaps, both fill rules and a stroke's pieces resolved in the sweep); monochrome icons as one mask
in any color; a 16-pixel icon in 7 us. Later that day: icons that last drawn from the text's pages in the labels' draw,
with the same pixels (200 rows of an icon and a label on row backgrounds: 394 draws -> 1), and of a large drawing only
what the clip rect shows rasterized, with the whole image's pixels.

**Polylines on the processor's vectors** (2026-10-30, in Esia): a polyline's loops on AVX-512, AVX2, SSE2 or NEON, chosen
at run time, every level with the same bits; long dense series in 0.039 ms of CPU on AVX-512 (plain C++ 0.150, Dear
ImGui 1.92.9b 0.084).

**Labels at many sub-pixel offsets** (2026-10-30, in Esia): a text drawn at many offsets is placed once per rounding class
of its glyphs and copied in after that; 3 000 labels 0.421 -> 0.280 ms of CPU on Direct3D 11, the same pixels.

**OpenGL: the host's texture units saved when first bound** (2026-10-30, in Esia): the host's state without a round trip
to a threaded driver for every unit; a small frame's submit 0.061 -> 0.039 ms.

**Render-class cost, round 2: much content on screen** (2026-10-30, in Esia): plain shapes - a solid fill or stroke
on a rounded rectangle, a circle, a capsule, a segment - and polylines are 32-byte quads drawn in the draws of the text
around them, with the pixels they had; shapes and labels overlapping one another are one draw (a thousand overlapping
items: 948 draws -> 1); a text can be drawn from the nearest whole pixel; solid lines and polylines from a Color,
without a Style. Measured against Dear ImGui 1.92.9b and 1.93.0. Polyline's round ends, drawn inside the line, are
beyond it.

**Glass on every shape** (2026-10-28, in Esia): polygons of any outline - slanted tabs, triangles, stars, arrows -
as one FX path each, with every material a rounded rectangle takes, glass included; lines, areas and ring sectors with
a material as paths too; glass on rings and lines one rounded rod.

**Render-class cost, round 1** (2026-10-28, in Esia): measured against Dear ImGui's renderer on the same device and
scenes - segments drawn as bands (a line across the screen shaded its bounding box: 1 000 of them took 3.4 ms of GPU
time, now 0.1), text placed without its color and at any sub-pixel offset without the atlas's map (a page of text
builds in 0.011 ms, Dear ImGui 0.026), Direct3D 12 writing a frame's data straight into its upload ring, and an
outside review's findings.

**One UI light, motion blur** (2026-10-28, in Esia): `RenderParams::light` lights every glass rim and turns every drop
shadow away from one point (the cursor) or direction (a device's tilt); `Style::MotionBlur` blurs a shape, its stroke
or text along the path it moves while the frame is shown.

**Temporal reuse of the backdrop** (2026-10-28, in Esia): glass captures reused from frame to frame when what lies
under them did not change (`RenderParams::targetStatic`), or for a few frames over a live background
(`backdropReuseFrames`); both opt-in.

**Instanced glyphs and tiles** (2026-10-28, in Esia): glyphs and flat tiles as 32-byte quad instances expanded on the
GPU (four new programs on every backend); a benchmark of fixed scenes (`esia_bench`). Text: build -43%, upload -69%,
GPU -60% on Direct3D 11.

**Smooth text motion, legible text** (2026-10-28, in Esia): moving text drawn unhinted at quarter pixels, settling
into its hinting when it stops; a halo of the other tone where the backdrop comes close to a glyph shape's color.

**Distance-field text** (2026-10-28, in Esia): Esia's own MTSDF generator (overlapping contours resolved, sharp
corners), glyphs as FX shapes with every Painter material (`Painter::Text` with a `Style`: outlines, glows,
shadows, gradients, glass, weight) on every backend, large text from fields by itself. Since 2026-10-30 the fields of a
text's new glyphs are made at once on worker threads (a paragraph of 200 new glyphs: 77 -> 8 ms on 16 cores, the same
bits) and can be kept on disk between runs.

**Sub-pixel text** (2026-10-28, in Esia): LCD antialiasing again, from either rasterizer with FreeType's filters,
dual-source blending on every backend but Direct3D 9, the grayscale fallback in glow layers; the ClearType setup of
the window's monitor on Windows.

**FreeType's rendering** (2026-10-28, in Esia): `TextRendering` chooses how the FreeType text system turns outlines
into pixels - FreeType's hinting (light, the fonts' own with the v35 or v40 interpreter, the autohinter), its
rasterizer, stem darkening, hinted advances; the design's outlines stay the default.

**Bitmap fonts** (2026-10-07): BDF, PCF and Windows FNT / FON draw pixel for pixel, sizes snapped to a strike's whole
multiples; a .fon's sizes are one font.

**Text outlines** (2026-10-07): `ItemStyle::TextOutline` / `Painter::SetTextOutline` - every glyph grown by exact
distance (round corners), cached in the atlas beside it and drawn under it in the same command.

**CPU of plain widgets** (2026-10-06, after flat drawing): springs and timers in a table of their own with fewer
look-ups, Painters that cost nothing to make, flat shapes written in place, a front for the text layout cache, a
Direct3D 11 host that owns its context: CPU 0.095 -> 0.079 ms regular and 0.072 flat, against Dear ImGui's 0.051. The
widgets' own work takes 0.039 ms with nothing drawn: 20% under Dear ImGui needs a lighter widget layer. Its second
round (2026-10-07): one animation entry per control, solid fills without a Style, capsule tiles found at any
length, hit records without a map look-up: flat 1.41 -> 1.23 times Dear ImGui's CPU, regular 1.55 -> 1.49.

**Flat drawing** (2026-10-06, before the CPU round): `UiDesc::flat` / `Ui::SetFlat` - no glass, shadows or glows, and rounded
shapes as nine-patches of coverage tiles in the text's texture, batched with the text: GPU time 0.054 -> 0.031 ms in
the plain-widget scene (Dear ImGui 0.025), 0.20 -> 0.11 ms in the workbench.

**Batching by what shadows show** (2026-10-06, after the plots): the planner lets a shape move to an earlier batch
past text its shadow's faint tail reaches, and keeps the pixels each batch draws as a few rects: the plain-widget
scene draws in 7 draws instead of 69 (submit: Vulkan -48%, Direct3D 12 -17%, OpenGL -24%; Direct3D 11 the same).

**Plots** (2026-10-06, last): lines, areas, scatter, bars and histograms over axes with a legend, a crosshair, pan
and zoom, and a donut chart, in the `workbench`'s new Graphs panel.

**Packaging** (2026-10-06, later still): `cmake --install` puts the libraries, their headers and a CMake package in a prefix;
`find_package(Esia CONFIG)` gives an application the targets `add_subdirectory` gives it (`esia::ui` ...).

**Density** (2026-10-06, later): `UiDesc::density` / `Ui::SetDensity` - Regular (iOS's sizes) or Compact, as WinUI's
compact sizing and Material's density: the text keeps its size, controls, rows, headers, padding and spacing get
smaller, and a control's shadows and glass with it, so the glass does not outweigh it. Under a floating bar (the tab
bar, the search bar) the content now fades out, as under iOS 26's bars.

**Debug checks, desktop defaults, the README audited** (2026-10-06): the core reports two items with one id, two
scroll areas with one id and items laid out out of view, and outlines them; a mouse no longer drags content and
desktops show scroll indicators at rest; `ui::IdScope`; sections without a header keep their state
apart, a table's `maxHeight` caps a `height`; the Control Center modules shrank since 2026-10-05 (`LineRoom`), fixed;
the README checked claim by claim against the code, the Windows pictures taken again.

**Text against Dear ImGui** (2026-10-05, later): frames of text as cheap as Dear ImGui's or cheaper - a text's glyph
quads kept and copied in, bounds kept as the lists are written, every list's vertices and indices uploaded as they
are (`Caps::baseVertex`, `Device::MapBuffer`), a formatter for the common label formats.

**Frame cost against Dear ImGui** (2026-10-05): the showcase against WGT's demo (Dear ImGui 1.92.9 with the same
glass) on the same scene, and plain widgets against a vanilla Dear ImGui; batches no longer split at clips that cut
nothing, a solid-area path in the FX shader, a flat per-id state table, glyph runs, fewer timestamp reads.

**Scrolling, menus and tables in use** (2026-10-05): what an application built on the widgets ran into - tables that
shared their scroll state, menus too long for the display or creeping by a fraction of a pixel, indicators past the
window, the wheel changing areas halfway, a field pushing the button after it out of view, a selection that stayed
on a row index.

* [Draw memos: widgets at rest draw nothing](#draw-memos-widgets-at-rest-draw-nothing)
* [Draw lists recorded and appended again](#draw-lists-recorded-and-appended-again)
* [Glass shading cost](#glass-shading-cost)
* [Vulkan frames in flight](#vulkan-frames-in-flight)
* [SVG to textures](#svg-to-textures)
* [Polylines on the processor's vectors](#polylines-on-the-processors-vectors)
* [Labels at many sub-pixel offsets](#labels-at-many-sub-pixel-offsets)
* [OpenGL: the host's texture units saved when first bound](#opengl-the-hosts-texture-units-saved-when-first-bound)
* [Render-class cost, round 2: much content on screen](#render-class-cost-round-2-much-content-on-screen)
* [Glass on every shape](#glass-on-every-shape)
* [Render-class cost, round 1](#render-class-cost-round-1)
* [One UI light, motion blur](#one-ui-light-motion-blur)
* [Temporal reuse of the backdrop](#temporal-reuse-of-the-backdrop)
* [Instanced glyphs and tiles](#instanced-glyphs-and-tiles)
* [Smooth motion and legible text](#smooth-motion-and-legible-text)
* [Distance-field text](#distance-field-text)
* [Sub-pixel text](#sub-pixel-text)
* [FreeType's rendering](#freetypes-rendering)
* [Bitmap fonts](#bitmap-fonts)
* [Text outlines](#text-outlines)
* [CPU of plain widgets](#cpu-of-plain-widgets)
* [Flat drawing](#flat-drawing)
* [Batching by what shadows show](#batching-by-what-shadows-show)
* [Plots](#plots)
* [Packaging](#packaging)
* [Density](#density)
* [Debug checks, desktop defaults, the README audited](#debug-checks-desktop-defaults-the-readme-audited)
* [Text against Dear ImGui](#text-against-dear-imgui)
* [Frame cost against Dear ImGui](#frame-cost-against-dear-imgui)
* [Scrolling, menus and tables in use](#scrolling-menus-and-tables-in-use)
* [Data widgets, Android, color glyphs, touch](#data-widgets-android-color-glyphs-touch)
* [Examples on Linux](#examples-on-linux)
* [Widget layer, second part](#widget-layer-second-part-featui-overlays)
* [Widget layer, first part](#widget-layer-first-part-featui-foundation)
* [UI core v2](#ui-core-v2)
* [Text on every platform](#text-on-every-platform-feattext-everywhere)
* [0. Round 3: sub-pixel text removed](#0-round-3-sub-pixel-text-removed)
* [1. Round 2: what changed](#1-round-2-what-changed)
* [2. Verified in round 2](#2-verified-in-round-2)
* [3. Results on Windows (the local session)](#3-results-on-windows-the-local-session)
* [4. Not verified](#4-not-verified)
* [5. Backend follow-ups](#5-backend-follow-ups)
* [6. Known issues](#6-known-issues)
* [7. Building and testing](#7-building-and-testing)
* [8. Round 1: the core](#8-round-1-the-core)
* [9. Next](#9-next)

## Draw memos: widgets at rest draw nothing

The goal of "CPU of plain widgets" (20 % under Dear ImGui) needed a lighter widget layer: drawing a widget again -
its Painter, its shapes' tiles looked up, its text placed or copied, every primitive merged - was a third of a frame of
plain widgets, and most widgets of most frames draw what they drew the frame before (2026-10-30).

### What changed (`ui.cpp`, `ui_internal.hpp`, `controls.cpp`, `containers.cpp`, `layout.cpp`)

* `detail::DrawMemo`: a widget declares one after what must happen every frame (interaction, springs), with its own
  inputs to its drawing (`MemoKey`: its rect, hover and press, its springs' values, colors, options, its label's layout
  - `TextMetrics::layout` -, word by word; floats by their bits). The memo adds what every drawing is drawn from: a
  serial of the theme, density, sizes, look, flat drawing, fonts and scales (compared byte for byte each frame), the
  style in scope (`StyleEntry::hash`, its fields and opacity) and the text system's `DrawGeneration`. When all of it is
  as it was and the draw list is in the state the drawing began in last frame (`DrawList::CanReplay`), the drawing is
  appended again (`DrawList::Replay`) with its texts' smooth-motion records (`TextSystem::ReplayDraws`: each at rest
  where it was) and the widget draws nothing; else its drawing is recorded for the next frame, and kept unless it
  cannot be appended again: a text in motion or settling, a glow fitted to its neighbours (`GlowContainment`'s calls),
  anything that reached into the list before it.
* Memos are kept per draw list in the order widgets draw (a lane, `Ui::Impl::MemoLane`): a widget finds its own by its
  place, nothing is looked up, and its inputs tell whether it is its own. A lane not drawn into for 600 frames goes.
* Button and IconButton, Toggle, Slider, Text (and TextWrapped, the headlines), a window's surface and its title keep
  memos; so do a list row's separator, highlight, icon and label (its interaction now comes before its drawing: it draws
  nothing), a row's right-aligned value, a navigation row's chevron, a slider row's value, a section's card (drawn after
  its rows and moved under them: the memo lies between the `Mark` and the move), a table's cells and a tree's rows (their
  springs first). A memo inside another's recording draws as part of it.
* Kept measures (`MeasureMetrics`): a text measured in a lane's same place with the same font, size, wrap width and
  bytes, the text system's text coming out as it did, takes the metrics it had (`MeasureSlot`) without the layout
  cache's look-up; the layouts found so stay in the cache's use (`TextSystem::KeepLayouts`, once a frame). A button
  measures its label once (its draw took the metrics again).
* `UiDesc::drawMemo` (on), `ESIA_UI_MEMO=off` / `verify`: every widget drawn and compared with what would have been
  appended (and every kept measure with the text system's), the differences counted in `Ui::MemoStats().mismatches`
  and reported on stderr.
* Also: a layout slot at rest steps no spring (`BoxLayout::Slot`); a widget's few quads are appended without the C
  runtime's memcpy.

### Verified

* `DrawMemo.TheSameDrawDataAsDrawingEveryWidget` / `TheSameDrawDataWithTextInMotion` (`esia_ui_memo_tests`, FreeType
  and the test fonts): 440 frames of a panel of labels, buttons, switches, sliders, an icon button, a paragraph, a
  section of rows (a value, a switch with an icon, a slider with its value, a navigation row, a button), a table and a
  tree - at rest, the mouse down across all of them with a click and a drag, a style on one button, the theme's transition to light and
  back, a second scale, content moving by fractions of a pixel, the flat look - with memos on and off, every frame's draw
  data byte for byte the same; "verify" finds no difference. With smooth motion the texts that moved are drawn while they
  settle (8 frames), the rest appended.
* showcase and workbench, 400 frames each with `ESIA_UI_MEMO=verify` (Direct3D 11): no difference.

### Measured (rcbench's widget scene, Direct3D 11, RTX 4080 SUPER, medians of 7 alternated runs)

Four windows of rows of a label, a button, a switch and a slider, and a paragraph; ids made outside the frame.

| ms per frame | build | submit | CPU |
| --- | --- | --- | --- |
| Dear ImGui 1.92.9b | 0.0265 | 0.0054 | 0.0317 |
| Esia flat, before | 0.0338 | 0.0049 | 0.0389 |
| Esia flat, memos | 0.0195 | 0.0050 | 0.0246 |
| Esia regular, before | 0.0350 | 0.0200 | 0.0550 |
| Esia regular, memos | 0.0210 | 0.0199 | 0.0408 |

The flat look takes 0.78 times Dear ImGui's CPU. With `ESIA_UI_MEMO=off` the same binary builds as before (0.0329 /
0.0341).

### Not done

* The regular look's submit: its widgets' shadows are FX instances between the labels' quads (66 draws against 5 flat).
* Other widgets (fields, pickers, segmented controls, steppers, badges, progress, plots, the dock's tabs ...) draw every
  frame; each needs its inputs named.
* What a widget draws from that the memo does not see would be drawn stale: a widget keeping a memo must name every
  input of its drawing (the verify mode and the memo test find what is missed), and draw nothing whose pixels change
  without its inputs (an SVG breaks recordings: `DrawList::BreakRecording`).

## Draw lists recorded and appended again

A widget whose inputs are those of the frame before draws what it drew then. With much content on screen, drawing it
again - its shapes' tiles looked up, its text's quads copied in, every merge of every primitive - is most of what a frame
of widgets costs (2026-10-30). The widget layer keeps such drawings (EsiaGUI: `UiDesc::drawMemo`); this is what it
keeps them with.

### What changed

* `DrawList::BeginRecord` / `EndRecord(DrawRecord&)`: what a stretch of the list added - its vertices, indices (from its
  first vertex), quads, FX instances and commands (ranges from its starts) - and what it did to the command before it:
  its primitives may join that command, so the count it added, the bounds of what it added (gathered apart while it
  records), its texture and kinds after them. A recording that reached into what lay before it is not kept (a `Mark`,
  moved commands, bounds refreshed there, a clip or texture popped from below its start, a layer, a fade or a callback,
  `BreakRecording`), nor one that left the clip or texture stacks changed; either way the list is what it would be
  unrecorded.
* `CanReplay` / `Replay`: appended where the list's last command is the same but for its ranges (and takes primitives the
  same way: the same kind, clip, texture, effect, contiguous or not, empty or not), after a `Mark` or not alike, with the
  same clip and texture in use. The arrays are appended and moved to where they go (indices, command ranges); the command
  before takes the count and bounds the stretch added to it.
* `BreakRecording`: what is drawn now depends on more than the drawing's inputs. `svg::Draw` calls it (a drawing is an
  image the first frame and a tile in the text's pages after).
* Text: `TextMetrics::layout`, a layout's identity (a serial given whenever a text is laid out; never reused). While
  `SetDrawNotes` has a list, `Draw` and `DrawOutline` with smooth motion append where they drew (`DrawNote`: the layout,
  the place, the scale, how much at rest). `ReplayDraws` leaves those records as the draws would - each one the next of
  its layout this frame, seen where it is, at rest - or changes nothing when one of them would be drawn otherwise (it
  moved, it is settling, its layout left the cache). `DrawGeneration` changes whenever a text drawn the same way could
  come out otherwise (pages started over, another rendering, density or fallback fonts). `KeepLayouts`: layouts a caller
  keeps using without measuring them again stay in the cache's use.

### Verified

* `DrawList.ReplayAppendsWhatDrawingWould`: a recorded stretch replayed after other content (glyphs on its label's page
  before it: it joins their command; more geometry: its indices move) against drawing it there, every array byte for
  byte and every command field for field; a recording leaves the list as it is unrecorded.
  `DrawList.ReplayNeedsTheStateItBeganIn`, `DrawList.RecordingsThatReachBackAreNotKept`.
* EsiaGUI's draw memos: 440 frames of a panel of labels, buttons, switches, sliders and a paragraph through hovers, clicks,
  a drag, a style, a theme's transition, a second scale and content moving by fractions of a pixel (smooth motion on and
  off): the draw data byte for byte what drawing every widget makes, every frame.

## Glass shading cost

What liquid glass costs the GPU - its shading with and without the UI light (RenderParams::light), and its backdrop
captures - and its shading made cheaper without a pixel changing (2026-10-30). RTX 4080 SUPER, Release clang-cl.

### Measured first

`esia_bench` prints two more medians: the GPU time of the backdrop captures (`capture`: the copy and the pyramid) and of
the FX batches with glass (`glass`), every category switch timed (`maxProfileScopes` 1024). `--light` puts a point
light over the middle of the target; the scene `glass-shadow` is the glass scene (20 frosted panels with titles) with a
drop shadow under each panel, as the widgets draw windows and bars; `shapes-fx` draws the 10 000 plain shapes as FX
instances. At 2808 x 2100 the glass scene takes one capture and its shading is most of the frame; at 1600 x 1000 every
panel captures again (20 captures).

* The UI light costs nothing measurable: glass 0.223 ms with it and without, on Direct3D 11 and 12. Not because it is
  free: FxPS computed the UI light's direction and the glass's own one in every pixel (a `?:` on the light, which fxc
  compiles as both sides and a `movc`), the drop shadow's offset likewise, so a frame without the light paid for it all
  the same. All of the light's terms together - the direction, the rim's two powers, the side of the slab facing it -
  were 5 % of the glass's time (taken out in a test build).
* The glass's shading in parts, each taken out in turn (Direct3D 11, 2808 x 2100): the backdrop as two B-spline levels
  instead of one bilinear read 25 %, the bevel's normal (four distances a pixel, over a quarter of a panel) 8.5 %,
  the light's terms 5 %, vibrancy and veil 3 %, the rim's reflection 2.5 %, the rim's Fresnel average 2 %; FxPS outside
  the glass body (the distance, coverage, the rim's hairline, grain, the output) about 35 %; the quads with an empty
  pixel shader 9 %.
* The captures: at 1600 x 1000 the 20 captures take 0.22 ms (Direct3D 11) and 0.18 ms (Direct3D 12), more than the
  panels' shading (0.14 / 0.13 ms) - 9 to 11 us each, most of it the fixed cost of its four downsample passes, each
  waiting for the one before (one capture of the whole 2808 x 2100 target: 0.04 ms). Not changed here (Not done).

### What changed (esia_fx.hlsl, esia_common.hlsli)

* The light's direction is a branch on the light: the UI light's (per pixel for a point light) or the glass's own
  angle's, not both and a select; the drop shadow's offset likewise (`ShadowOffset`).
* The rim's light - two powers - only where the rim has light: in the band along the outline where the pixel's Fresnel
  average is computed. Elsewhere the rim was 0 times them.
* The bevel's normal: a rounded rect with one radius on every corner (most glass) takes its four distances from one
  corner, without ShapeSD's dispatch over the shape kinds. `SdRoundRect` is two functions now - `RoundRectCorner`, the
  corner of the quadrant a point lies in (its reach and smoothing), and `SdRoundRectCorner`, the distance from it -
  and with one radius the four points' corners are one: the same operations on the same numbers. Not on SM3: the full
  ps_3_0 shader keeps one copy of the distances (3921 of NVIDIA's 4096 instruction slots; 4121 with a second).

Every change computes the same values in the same order where it computes them. Glass GPU ms, the build before and after
alternated, 10 rounds of 600 frames (1600 x 1000: 8; Vulkan, OpenGL: 6), medians of the rounds' medians - no two rounds
of a cell more than 0.002 ms apart:

| 2808 x 2100 | Direct3D 11 | Direct3D 12 | Vulkan | OpenGL |
| --- | --- | --- | --- | --- |
| glass | 0.223 -> 0.208 (-6.7 %) | 0.223 -> 0.208 (-6.7 %) | 0.238 -> 0.220 (-7.6 %) | 0.245 -> 0.228 (-6.7 %) |
| glass, UI light | 0.223 -> 0.209 (-6.3 %) | 0.223 -> 0.210 (-5.6 %) | 0.238 -> 0.222 (-6.7 %) | 0.245 -> 0.230 (-6.1 %) |
| glass-shadow | 0.263 -> 0.246 (-6.5 %) | 0.263 -> 0.247 (-6.3 %) | 0.281 -> 0.260 (-7.5 %) | 0.293 -> 0.275 (-6.1 %) |
| glass-shadow, UI light | 0.263 -> 0.249 (-5.3 %) | 0.263 -> 0.249 (-5.3 %) | | |

The frame -4 to -5 % on Direct3D 11 / 12. At 1600 x 1000 glass 0.139 -> 0.128 ms (Direct3D 11) and 0.134 -> 0.126
(Direct3D 12), glass-shadow 0.148 -> 0.137 and 0.152 -> 0.141, the captures as they were. WARP (Direct3D 11, a CPU
rasterizer: arithmetic weighs more there), 1600 x 1000, 5 rounds: 24.96 -> 23.34 ms (-6.5 %). `shapes-fx` (FX shapes
without glass, 14 rounds): 0.325 -> 0.324 ms (Direct3D 11), 0.333 -> 0.330 (Direct3D 12), the same. With the light the
glass now costs 0.001 - 0.003 ms more than without it: its direction per pixel, which glass without the light no longer
computes.

### Verified

* Every conformance scene rendered by the build before and after on Direct3D 9, 10, 11 and 12 (RTX 4080 SUPER and WARP),
  OpenGL, GLES and Vulkan (the last three with the shader library regenerated by the same local tools for both builds):
  identical, pixel for pixel; the goldens unchanged.
* ctest 42 / 42 (clang-cl Debug, `-DESIA_WERROR=ON`) with the library as it is and regenerated; Direct3D 10 / 11 / 12
  with the debug layers, 3 frames: no message; MSVC `/W4 /WX`; the GNU flags on esia_bench.
* fxc /O3: ps_5_0 FxPS 23 temporaries before and after, 3406 -> 3569 instruction slots (the second copy of the four
  distances: a glass pixel runs fewer instructions, the shader holds more), ps_4_0 3465 -> 3632; ps_3_0 31 registers,
  3910 -> 3921 slots; FxVS unchanged.

### Not done

* The glass body's four instance rows (glass, tint, its parameters, the loupe) as flat varyings: on Direct3D 11 those
  four structured-buffer reads a pixel are 6 % of the glass's time (Direct3D 12's root SRV: 1 %), but four more
  varyings cost every other FX shape 2.5 % (Direct3D 11) and 4.5 % (Direct3D 12) in `shapes-fx`; two of them in the
  solid area's varyings (a glass has none) cost it 4 - 5 %.
* FX feature variants on Direct3D 10 - 12 (`Caps::fxFeatureVariants`): the glass scene drawn with a variant of fill,
  stroke, shadow, glass, mask and grain (21 temporaries instead of 23, 2540 instruction slots instead of 3406) was 9 %
  faster; a variant is another compile, not bound to round as the full shader does, and is built at run time.
* The pyramid's last level: `LevelsForBlur` builds the level after a blur's own even where the shader reads that one
  alone (a radius at a power of two: the rim reflection's 16 units at scales 1 and 2). Not building a level no glass
  reads took 17 % off the captures (20 of them, 1600 x 1000, a test build); it changes the null device's command
  goldens.
* An integrated GPU: the Ryzen's is off in the BIOS. Its arithmetic is scarcer: what was cut here should weigh more
  there.

## Vulkan frames in flight

Rounds 1 and 2 left two things in Vulkan's uploads that make frames in flight wait for each other (2026-10-30): one
barrier before a frame's uploads waited for all earlier work at every stage - the frame before's too -, and every
buffer had one device copy, which each frame wrote while the frame before could still be reading it. No benchmark ran
frames in flight to see what that cost. Measuring it found a larger cost next to it: the GPU read every draw's
constants from host memory.

### What changed

* The frame's data in a ring per frame in flight: `MapBuffer` in a frame (the renderer's vertices, indices, FX instances
  and quads) hands out a range of the frame slot's new ring (vertex, index and storage usage; 1 MiB chunks that grow and
  merge like the staging ring's). Where the device has device memory the host maps (`DEVICE_LOCAL | HOST_VISIBLE |
  HOST_COHERENT`: the BAR of a discrete GPU - 256 MB without resizable BAR -, unified memory) the ring is there and the
  bytes are written in place: no copy, no barrier, read by the GPU at full speed. Without it (or without room in it) the
  ring is device memory, and the bytes go into the slot's staging ring - written once, not into a vector first - and are
  copied into the range at `UnmapBuffer`. The frame's draws read the range: vertex and index buffer offsets, the FX
  storage buffer's descriptor range. A slot's rings are reset only when the slot comes back - its last frame completed
  (its fence, or the host's promise) -, so nothing waits for the frame before and every frame in flight reads data of
  its own. Host frames that share a slot (`FrameDesc::hostFrame`) take ranges one after another.
* Constants: the uniform ring (one per slot already) in mapped device memory too where there is some, else in host
  memory as before. Read across PCIe, every draw's constants cost a glass frame a fifth of its GPU time on the RTX 4080
  SUPER.
* `vulkan::Desc::mappedDeviceMemory` (default on; `HeadlessOptions::mappedDeviceMemory`; the registered creator reads
  `ESIA_VULKAN_MAPPED_DEVICE_MEMORY=0`): off leaves that memory to the host - constants in host memory, the frame's data
  copied from staging. A few MB per frame in flight; an allocation that fails falls back chunk by chunk.
* A buffer's own copy keeps `UpdateBuffer`'s bytes (and `MapBuffer`'s outside a frame, recorded by the next one, as
  before) and is made at its first use: the buffers the renderer maps every frame take no device memory beside the
  ring.
* Barriers on what was written. No barrier opens a block of uploads (it was `ALL_COMMANDS` -> `TRANSFER`). An image
  upload waits through its transition from the layout it is in, as before, and an image written twice in a block waits
  on that image, not on every transfer; an `UpdateBuffer` into a copy that commands used waits on that buffer, for the
  stages that read its kind and for transfers. The block ends with the written buffer ranges made visible to the access
  and stages that read each kind (vertex attributes or indices at vertex input, shader reads of the FX buffer) and the
  uploaded images' transitions: no global memory barrier, no uniform or transfer reads of buffers nothing reads so.
* `esia_bench --in-flight`: the GPU is not waited for between frames, so the device keeps its frames in flight (Vulkan's
  headless device: 2, a fence per slot), as a window's loop with vsync off. Frame time is the period from a frame's
  start to the next one's - the GPU's time per frame where the GPU is the bottleneck, the CPU's where it is; submit
  includes the waits for a slot then, and a frame's GPU time runs from the end of the frame before. CTest runs the mode
  on the null device (`esia_bench_smoke_in_flight`).
* Tests (`esia_rhi_vulkan_tests`), each with mapped device memory and through staging:
  `Vulkan.BufferWritesWithFramesInFlight` writes vertex, index and FX buffers with `UpdateBuffer` and with `MapBuffer`,
  mixed from frame to frame and once outside a frame, while the frames that draw from them are in flight;
  synchronization validation checks every hazard (through staging, without the own copy's barrier, without its reader
  stages, or with one ring for every slot it reports one). `VulkanScenes.FrameDataPerFrame` renders five frames of
  different content, a target each, the GPU never waited for: every target has its frame's pixels rendered alone, and
  the two ways give the same pixels.

### Verified

* Every test on Windows (clang-cl Debug, `-DESIA_WERROR=ON`): 43 / 43, `esia_rhi_vulkan_tests` 17 / 17 and the four
  Vulkan conformance tests (`--strict`, dynamic rendering and render passes, one frame and three) with the validation
  layer and synchronization validation: no message. MSVC `/W4 /WX` and the GNU flags on the changed sources.
* Pixels: `esia_conformance --backend vulkan --out` of the build before and after (Release, RTX 4080 SUPER), dynamic
  rendering and render passes, one frame and three, with mapped device memory and through staging: 35 / 35 images
  identical bit for bit each time; the goldens are unchanged.
* `esia_bench --in-flight` with the validation layer and synchronization validation on (`ESIA_VULKAN_VALIDATION=1`),
  both ways: every scene, no message. `glass_window --api vulkan --debug` (a swap chain, the host's command buffers, two
  frames in flight): 600 frames, no message.

### Measured

`esia_bench`, Release clang-cl, Vulkan on the RTX 4080 SUPER, 1600 x 1000, 600 frames after 60 of warm-up; the build
before (9df4411 with this `esia_bench`) and after, alternated, 12 runs each in flight and 10 with the GPU waited for (a
run during which another benchmark ran on the machine done again); ms, medians of the runs' medians, before -> after
(after, through staging: `ESIA_VULKAN_MAPPED_DEVICE_MEMORY=0`):

| scene | frame, in flight | GPU, in flight | GPU, waited for | submit, waited for |
| --- | --- | --- | --- | --- |
| text | 0.036 -> 0.033 (0.035) | 0.014 -> 0.007 (0.014) | 0.016 -> 0.009 (0.016) | 0.036 -> 0.035 (0.034) |
| widgets | 0.051 -> 0.050 (0.050) | 0.025 -> 0.019 (0.025) | 0.026 -> 0.020 (0.026) | 0.036 -> 0.034 (0.033) |
| widgets-flat | 0.100 -> 0.099 (0.098) | 0.025 -> 0.018 (0.025) | 0.026 -> 0.019 (0.026) | 0.039 -> 0.035 (0.035) |
| shapes (10 000, 625 KB a frame) | 0.450 -> 0.444 (0.437) | 0.056 -> 0.030 (0.056) | 0.057 -> 0.031 (0.057) | 0.069 -> 0.061 (0.055) |
| glass | 0.530 -> 0.412 (0.527) | 0.512 -> 0.398 (0.513) | 0.514 -> 0.403 (0.515) | 0.207 -> 0.207 (0.204) |
| glass-static | 0.435 -> 0.407 (0.428) | 0.417 -> 0.394 (0.416) | 0.429 -> 0.402 (0.426) | 0.202 -> 0.202 (0.201) |
| glass-live | 0.536 -> 0.413 (0.530) | 0.512 -> 0.399 (0.514) | 0.538 -> 0.420 (0.540) | 0.251 -> 0.253 (0.246) |
| glass-live-2 | 0.597 -> 0.540 (0.589) | 0.427 -> 0.396 (0.434) | 0.458 -> 0.405 (0.434) | 0.201 -> 0.200 (0.198) |

* The GPU: a glass frame loses a fifth of its time (0.512 -> 0.398 ms in flight; the runs 0.507 - 0.730 -> 0.397 -
  0.400): its 121 draws read their constants from device memory instead of across PCIe. Frames that upload much lose
  the copy: the shapes 0.056 -> 0.030 ms, a page of text 0.014 -> 0.007. The glass scenes are bound by the GPU in
  flight, so their frame follows: 0.530 -> 0.412 ms.
* The CPU: submit with the GPU waited for 0.069 -> 0.061 ms for the shapes (runs 0.065 - 0.096 -> 0.059 - 0.063),
  4 - 10 % less for text and widgets, the glass scenes the same (constants written into device memory cost the CPU
  nothing measurable). In flight the text and widget scenes are bound by the CPU: text 0.036 -> 0.033 ms a frame.
  Through staging the shapes submit less still (0.055 ms), their 625 KB written into host memory rather than across
  PCIe, but the GPU copies them.
* Found on the way: the first version of this change wrote the frame's data through staging only, and the frames' GPU
  time stayed where it was - for the glass scenes in flight 3 to 4 us more (0.508 -> 0.512 ms, 12 runs, ranges
  0.502 - 0.516 -> 0.509 - 0.518). With the old barrier before the uploads put back (an experiment) it was 0.509: on
  this GPU the next frame's copy overlapping the frame before cost more than the overlap saved. The upload barrier was
  not what ordered the frames on the GPU; their target is (Not done). Written in place, nothing is copied.

### Not done

* Frames still meet at the images they share. A frame's first pass on its target waits for every earlier use of the
  stages the target's resting layout stands for, the frame before's included: vertex and fragment shaders for
  `SHADER_READ_ONLY`, where a sampleable target rests after each pass (a sampleable swap-chain image too, moved there at
  `BeginFrame`), color output for the others - only an image entered as `UNDEFINED` and not sampleable waits for
  nothing. And captures write the backdrop copy and the pyramid that the frame before's glass reads. A Vulkan barrier
  cannot wait for one image's last use: an image whose last use lies in a completed frame (or a swap-chain image entered
  as `UNDEFINED` and not read yet) could transition without waiting, and the frames would overlap there. The benchmark
  renders into one target, where they meet in any case.
* Textures (glyph pages, images) and `UpdateBuffer` still go through staging and a copy, also where device memory is
  mapped (`src/esia/rhi/vulkan/STATUS.md`, known issue 2); an `UpdateBuffer` into a buffer that frames in flight draw
  from waits for its kind's reading stages (the renderer maps its per-frame data; `UpdateBuffer` is for data that
  stays).
* The mapped device memory is not budgeted (`VK_EXT_memory_budget`): next to a host that fills the BAR the backend falls
  back when an allocation fails, not before.

## SVG to textures

SVG files - icons, illustrations - parsed once and rasterized at the size they are drawn, into texture pages that
Painter::Image draws (2026-10-30): `include/esia/svg/svg.hpp`, the library `esia::svg`.

### What it does

* `svg::Document::Parse` / `Load`: the XML (comments, DOCTYPE, CDATA, entities); CSS - style attributes and `<style>`
  sheets with type, class, id, universal, descendant and child selectors, specificity, `!important`, inheritance; path
  data (every command, numbers and flags without separators); the basic shapes; transforms, viewBox and
  preserveAspectRatio, nested `svg`, `use` and `symbol`; colors (hex, rgb / rgba, hsl / hsla, the named colors) and
  `currentColor`; linear and radial gradients (units, transforms, spread methods, focal points, `href` inheritance); fills
  with both rules; strokes (width, caps, joins, miter limit, dashes, paint-order); opacity; clip paths (both units,
  nested); `display`, `visibility`, `shape-rendering: crispEdges`. Not drawn: text, images, filters, masks, markers,
  patterns.
* `svg::Rasterize`: RGBA8, straight alpha in sRGB (the texture format of images); transparent texels next to the drawing
  take its colors, so bilinear filtering does not darken its edges. `RasterOptions::mask`: the alpha alone, in white,
  tinted when drawn. `RasterOptions::viewport`: the drawing fitted into any rectangle of the image, to fractions of a
  pixel, cut exactly at its sides.
* `svg::Atlas`: rasterized documents packed into RGBA8 pages (text::GlyphAtlas's packing and frame rules), keyed by
  document, size, viewport, mode and - where it paints something - currentColor. A new drawing goes into the recent
  pages and moves into the lasting ones when the next frame draws it too (from the pixels it was made with): the sizes
  an animation passes through stay among the recent pages, found again when it comes back to them, and filling those
  never makes what stays on screen be rasterized again. A drawing larger than a page gets a texture of its own while it
  is drawn. A monochrome icon that lasts goes into the text's pages instead (`Atlas::Tile`, below).
* `svg::Draw` rasterizes at the size the drawing has on the target and draws it with its texels on pixels one to one:
  outside a scale on the rectangle rounded to whole physical pixels; under `Painter::PushScale` exactly where the scale
  puts it (to 1/64 pixel), rasterized there - as text is under a scale - so every frame of a scale animation has the
  pixels of its own geometry. A monochrome document (every paint currentColor) is rasterized once, as a mask, and drawn
  in any color. Only what the clip rect shows is rasterized (below).

### Quality

* Exact area coverage: a pixel gets the area of the filled region in it. The sweep puts each row's monotone chains in
  order; where they keep apart, the winding count tells which chains bound the area, and only those leave their area in
  the cells; where they come close (turning points, overlaps, crossings), the row is cut into strips at every vertex and
  crossing. Overlapping contours, both fill rules and a stroke's overlapping pieces are resolved exactly - the usual
  |sum| clamped to 1 over-covers such pixels, by up to a quarter of a pixel.
* Strokes are made in the path's user units (a stroke scaled unevenly keeps its shape there) as pieces wound the same
  way whose union is the stroke: bands along runs of segments, wedges and caps. A closed convex outline whose inner side
  folds (a stroke wider than a small circle) is its outer loop less its inner offset, a half-plane intersection.
* Curves and arcs are flattened within 1/32 pixel, their points moved out by 2/3 of their chords' sagitta: the polygon
  keeps the curve's area, so curved edges do not come out thinner.
* Compositing as browsers composite SVG: premultiplied, in sRGB. Group opacity and clipped groups take a layer only where
  they need one (a group around one shape with one paint passes its opacity to it).

### Verified

* `esia_svg_tests` (29): the parser and CSS; coverage against dense point sampling (64 x 64 points a pixel) for
  overlapping squares, a pentagram and a bow tie under both rules and unions of groups with their own rules (within
  0.002); round strokes against "every point within half the width of the path" (within 0.008); strokes wider than their
  outline's curves (a disc, a ring, folded corners); dashes as SVG 2 places them; gradients, layers, clips, masks, the
  texels around the drawing, the atlas's cache, Draw on the pixel grid; a viewport at quarter and half pixels against
  the area of its box in every pixel, also for a square and a stroke reaching far beyond it (within half a level); Draw
  under scales of 1.5 and 1.25; a drawing that stays beside one at a new size every frame (rasterized once); drawings
  larger than a page. `esia_render_tests`: Painter::Scaled, ScaleFactor and UnscaledImage (a hundred images under one
  scale, with geometry between them and under a mask).
* Under a scale on a GPU: an icon drawn at scales 1.37, 1.5 and 2.25 on D3D9, D3D10, D3D11, D3D12, OpenGL and Vulkan
  (RTX 4080) has the pixels of svg::Rasterize of the same geometry on the CPU, within half a level (the rounding of
  the blend), and nothing around it. Drawn the first version's way - rasterized at its size in the layout and stretched
  by the scale - not one of its pixels reached full ink at those scales.
* Against Chrome, its rendering of the same files at the same size compared pixel by pixel in the page: 158 renderings
  of 80 files - the 63 SVG icons that come with Visual Studio, 16 feature files, two logos - at 16 to 128 pixels. Mean
  alpha difference 2.8 levels, along edges: Chrome's antialiasing there has 8 levels of coverage (multiples of 32) where
  Esia's is the area, its edges on whole pixels can sit 0.08 pixel off, and its dashes drift (half a pixel at the top of
  a dashed circle; Esia's fall where the dash pattern puts them). Two icons Chrome does not draw (attributes without
  white space between them) Esia does.
* clang-cl, MSVC `/W4 /WX`, the GNU flags on the new files.

### Measured

Release clang-cl, Ryzen 9 7950X, one thread, that corpus:

| | mean | max |
| --- | --- | --- |
| parse a file | 6.7 us | |
| rasterize at 16 x 16 | 7.3 us | 14 us |
| at 48 x 48 | 18 us | 30 us |
| at 128 x 128 | 88 us | 136 us |

At 512 x 512: an empty drawing 0.13 ms, a full opaque rectangle 0.42 ms, a full linear gradient 0.98 ms, a stroke wider
than its small circle 0.26 ms (8.7 ms as overlapping pieces). Only the drawing's rows and columns are visited; rows turn
into RGBA8 with SSE2 or NEON (a plain loop elsewhere); a group's layer only where it needs one.

The atlas over 600 frames: 80 icons that stay (40 documents at 24 and 48 pixels) and icons whose size changes every
frame in one-second cycles, the first version's atlas (one set of pages) against the recent and lasting pages, runs
taken in turn, the cost of the frame's lookups and rasterizations:

| | first version: mean / p99 / max | now: mean / p99 / max | icons that stay, rasterized again |
| --- | --- | --- | --- |
| 4 animated, 24 to 96 px | 7.4 / 155 / 165 us | 6.6 / 132 / 153 us | 0 / 0 |
| 20 animated, 24 to 120 px | 746 / 3290 / 3400 us | 576 / 930 / 1310 us | 1520 times in 19 frames / 0 |

With 20 animations the first version's pages filled 19 times, and each time the next frame rasterized the 80 icons that
stay as well. A lookup that finds its drawing costs what it did (6.3 to 6.8 ns in both).

### Fixed after the first version

* Under `Painter::PushScale` a drawing was rasterized at its size in the layout and the image stretched by the scale:
  soft at every scale but 1. It is now rasterized where the scale puts it. Painter has what that needs: `Scaled` and
  `ScaleFactor` (where the scales pushed put a point, how much they scale lengths) and `UnscaledImage` (an image already
  at its final place, which PopScale leaves alone); and PopScale's list of what it leaves alone joins ranges that follow
  one another, so labels and icons drawn one after another under a scale take one entry (it holds 64; past them, what
  came after was scaled twice).
* Drawings at new sizes every frame filled the one set of pages, and each time it started over, everything on screen was
  rasterized again: recent and lasting pages now.
* A drawing larger than an atlas page was not drawn at all (at the default 1024, anything over 1021 pixels - 510 UI
  units at a pixel scale of 2): a texture of its own now.

### Icons with the labels, only what the clip shows (later the same day)

An icon was an image of the atlas's RGBA8 pages, between labels drawn as glyph quads of the text's pages: another
texture and another program, a draw of its own wherever the planner could not move it - with a background under each
row, two draws a row. And a large drawing was rasterized whole, also where the clip rect hides it.

#### What changed

* `svg::Atlas::Tile`: a mask (a monochrome document, or `mask`) of up to 128 x 128 pixels that lasts - drawn in the frame
  before too, the lasting pages' rule - goes into the text system's pages as a coverage tile (`TextSystem::FindTile` /
  `AddTile`, keys of its own): its alpha taken from the pixels it was made with, or rasterized again where the recent
  pages served it. What is new in a frame stays in the recent pages, so the sizes and places of an animation never reach
  the text's pages, which start over only when they did before; then an icon is an image for a frame and a tile again.
* `svg::Draw` draws a tile with `Painter::UnscaledCoverage`: a quad of plain coverage (`QuadInstance::SetCoverage`: the
  alpha + 256; `DrawList::AddCoverageQuad`) in the command of the text on its page, at the image's place, in the
  painter's opacity, left alone by PopScale. Under a Painter mask an icon stays an image (its FX image takes the mask;
  `Painter::Masked`).
* The programs that tell quads apart (`QuadShapeVS`; `QuadGrayPS`, `QuadLcdGrayPS`, `QuadLcdPS` - a command with plain
  coverage draws with them, `DrawCmd::shapes`) give it UiColor's pixels: the color with the texel as its alpha, as an
  image of white texels with that alpha, none of the text's composition (gamma, enhanced contrast:
  `render::TextComposition`) - on Alpha8 and sub-pixel pages, with dual-source blending and without. The four programs'
  GLSL, ESSL, MSL and SPIR-V come from CI's Shaders workflow, as every program's.
* An image of part of a drawing (`detail::RasterizePart`) is made in the drawing's frame - the whole pixels around its
  viewport, the viewport moved by the part's corner - and holds what it covers: its pixels are the whole image's there,
  bit for bit. The rows above it are passed over as the whole image draws them (`Sweep::Pass`: each row's chains in their
  order across it, without its coverage - where chains tie across a row, their order is the one the rows before left),
  the columns beside it are not painted (a gradient is placed from its span's start, as before), and the border holds
  the drawing's pixels where it goes on. `Rasterize` and `RasterOptions::viewport` are as they were: the drawing in the
  image's pixels.
* `svg::Draw` does not rasterize a drawing outside the clip rect, and of a large one reaching beyond it only the part
  inside, when that is at most half of it - in steps of 256 pixels from its corner, so that the part is found again
  while the drawing moves less than that (`Atlas::GetPart`). The part of a drawing made last is kept while the drawing is
  drawn: a new part copies the pixels they share and rasterizes the bands around them.

#### Measured

200 rows of an icon (16 x 16 pixels, 12 documents) and a label: `list` four columns of 50 rows, `panels` the same on a
plain rounded background a row, `grid` icons over labels in 8 columns drawn row by row. 1600 x 1000, Release clang-cl,
the median of 9 alternated runs' medians (each 600 frames; the lowest and highest run in the report), ms; the old build
draws the icons as images:

| Direct3D 11 | build | submit | GPU | draws |
| --- | --- | --- | --- | --- |
| list | 0.0307 -> 0.0250 | 0.0161 -> 0.0060 | 0.0092 -> 0.0082 | 2 -> 1 |
| panels | 0.0405 -> 0.0363 | 0.0379 -> 0.0067 | 0.0367 -> 0.0184 | 394 -> 1 |
| grid | 0.0366 -> 0.0279 | 0.0162 -> 0.0056 | 0.0102 -> 0.0091 | 2 -> 1 |

Vulkan: submit -24% / -81% / -18%, GPU +5% / -55% / 0% (the text's quads in a frame of two draws now run the program that
tells quads apart); Direct3D 12: submit -22% / -56% / -20%, GPU 0% / -62% / -10%; build -8% to -15% on both. A second
Direct3D 11 run gave the list's build -1% (the machine was shared).

A large drawing, the CPU of its draws (a 1600 x 1000 view, 3 alternated runs):

| | before | now |
| --- | --- | --- |
| 3200 x 2400, its middle shown: the first frame | 77 - 81 ms, 7.7 M texels | 29 - 31 ms, 2.3 M texels |
| 800 x 600 zoomed to 4 times in 90 frames: a frame | 34 ms (worst 77) | 26 ms (worst 51 - 53) |
| 1600 x 4000 scrolled through it, 6 pixels a frame: a frame | 0.22 ms (the first 65) | 0.30 - 0.32 ms (the first 15 - 18) |

Scrolled, the drawing is made 15 times instead of once - every 256 pixels a part, from the part before and the band below
it - and uploads 27.5 M texels instead of 6.4 M.

#### Verified

* `esia_svg_tests` (35): images of parts against the image of all of it, texel for texel (two drawings with curves,
  dashes, crisp edges, gradients with a focus, layers and nested clips, masks and colors, three viewports, seven parts
  inside, at and beyond its edges); a part's viewport moved by whole pixels; a lasting icon as a tile in the labels'
  command, its coverage the mask's alpha, gone with the text's pages and back; animations - sizes, a scale, sizes in
  turn - never in the text's pages; no tiles for colored drawings, masks over 128 pixels, under a Painter mask or
  without tiles; tiles under a scale; only what the clip shows rasterized, the uploaded parts against the whole image
  while it scrolls.
  `esia_core_tests` `DrawList.CoverageQuadsJoinTheGlyphsOfTheirPage`, `esia_render_tests`
  `Painter.UnscaledCoverageIsAQuadWithTheText`.
* Over 2 922 SVG files (the custom feature files, Visual Studio's icons, two logos, Lucide's 1 924 icons, Steam's
  controller images, QDarkStyle's): 525 960 parts of 52 596 images, 3 billion texels - none differs from the whole
  image's. `Rasterize` against the version before, over the same files: 70 128 images (four sizes; no viewport, one at
  fractions of a pixel, one inside the image; color and mask) - every one byte for byte the same.
* Conformance `coverage` / `coverage_image` (plain coverage against images of it: light and dark ink, translucent, both
  kinds of page, an edge fade, a glow layer): the same pixels on Direct3D 9, 10, 11, 12, OpenGL and Vulkan (RTX 4080
  SUPER), and a temporary scene of `svg::Draw` - icons as tiles against images, under scales 1.25 / 1.5 / 2, the
  painter's opacity, an edge fade, sub-pixel pages; a part against the whole image under the same clip: no pixel
  differs. ctest 42 / 42 (clang-cl, every backend), MSVC `/W4 /WX`, the GNU flags on the changed files.

### Not done

* Text, `<image>`, filters, masks, markers, patterns, `vector-effect`.
* A stroke wider than a tight curve of an open path stays a set of overlapping pieces: exact, slower at large sizes.
* A row crossed thousands of times: past 256 cuts it goes in strips of 1/32 pixel (coverage within about 1/64 there).
* A large drawing is rasterized on the thread that draws it: what the clip shows (0.4 to 1 ms for 512 x 512 pixels of
  it); a zoom makes a part every frame, a scroll one every 256 pixels (a band of it rasterized). No async rasterizing.
* Icons under a Painter mask stay images: a draw of their own.
* The four quad programs' generated shaders (GLSL, ESSL, MSL, SPIR-V) are the Vulkan SDK's tools' output: CI's Shaders
  workflow must make them before a push.

## Labels at many sub-pixel offsets

Screens with much content on them draw the same short texts at many places a frame - names, values, units - at
offsets that are fractions of a pixel. Each such draw placed every glyph again (2026-10-30).

### What changed (`src/esia/text/ft/ft_text_system.cpp`)

* A glyph's quad depends only on the quarter pixel its pen rounds to across and the whole pixel its line rounds to
  down (a quarter pixel too for an outline in motion). Between the offsets where one of them changes - for a pen, where
  offset + pen = n + 1/8, 3/8, 5/8, 7/8; for a line, n + 1/2 - the whole placement is the same. `QuadSet` keeps those
  offsets (`xBreaks`, `yBreaks`) and a placement per class (`classes`, at most 256 a set, texts of up to 64 glyphs): a
  draw at another offset finds its class with two binary searches and copies the class's quads in, as a text drawn at
  the same offsets as last time is.
* `classScore` keeps classes only while a text's offsets come back (+4 a class found, -1 one placed): a text that moves
  on, or a new text every few frames, is placed as before (one pass straight into the list), and every 16th draw looks
  again.
* A moving text (smooth motion) keeps its glyphs' atlas slots per pen and line phase (`movingSlots`), as a still one
  did per pen phase: no atlas look-up per glyph in motion.
* A layout's key: its parameters' part kept while they repeat (most texts of a frame share a font, size and wrap
  width), the text's at a multiply per 8 bytes. A color glyph's color comes from the text's (one conversion, not two).
* A layout rebuilt for another text now forgets its moving sets too (their placements were the old text's).

### Verified

* The same pixels: rcbench's labels, text and widgets scenes and its scenes of many labels over boxes, drawn by the
  old and the new build on Direct3D 11, compared texel for texel (no difference); the text and render tests.

### Measured (rcbench, Direct3D 11, RTX 4080 SUPER, medians of 3 alternated runs, CPU = build + submit)

| ms | before | now | Dear ImGui 1.92.9b / 1.93.0 |
| --- | --- | --- | --- |
| 3 000 labels at their own offsets, 64 names | 0.421 | 0.280 | 0.252 / 0.256 |
| 1 000 items with a name and a distance each | 0.419 | 0.391 | 0.407 / 0.489 |

### Not done

* Labels at their own offsets still take 1.1 times Dear ImGui's CPU (0.92 on whole pixels).

## OpenGL: the host's texture units saved when first bound

### What changed (`src/esia/rhi/opengl/gl_device.cpp`)

Saving the host's state at BeginFrame asked every texture unit for its texture and sampler: an ActiveTexture before
each query, and on a threaded driver (NVIDIA's) a query after any call waits for the driver's thread. That was 80 % of a
small frame's submit time (`SaveHostState` under `BeginFrame` in the profile). The other queries now come with no call
between them (one wait), a unit's bindings are saved the first time the device binds a texture or a sampler to it
(`SaveUnit`), and only those units are put back (`HostState::savedUnits`).

### Verified

* `GlDevice.HostStateIsRestored` puts a texture of the host's on 16 units and samplers on some, and checks every one
  after a frame and after the calls outside frames; the OpenGL and GLES tests and conformance.

### Measured

* 100 items with a name and a distance (an rcbench scene), OpenGL on the RTX 4080 SUPER: submit p50 0.061 -> 0.039 ms
  (the profile's share of `SaveHostState` 80 -> 59 %: what is left is the first query's wait, which Dear ImGui's backend
  has too).

## Polylines on the processor's vectors

The loops that make a polyline's quads run on the widest vector instructions the processor has (2026-10-30): AVX-512,
AVX2 or SSE2 on x86-64, NEON on AArch64, plain C++ elsewhere - chosen at run time, not at build time, so one binary uses
what each machine runs.

### What changed

* `esia::cpu` (`include/esia/core/cpu.hpp`): the level a processor runs - cpuid and the register state the operating
  system saves (`xgetbv`) on x86, NEON on AArch64 - detected once; `ESIA_SIMD` (`scalar`, `sse2`, `avx2`, `avx512`, `neon`)
  or `cpu::Force` run a lower one (tests, benchmarks).
* `render/line_kernels.cpp`: Painter::LineQuads' loops - the segments' normals and the bounds, the joints' directions and
  the sharpest turn, the check that each piece clears its joints' lines, the pieces' quads - for every level. AVX2 and
  AVX-512 code is compiled with the compilers' target attributes in the same file (MSVC needs none), so the library is
  built for the baseline and still runs the wide code where it exists. A level hands what is left over to the next
  narrower one (AVX-512 to AVX2 to SSE2 to plain C++).
* Every level writes the same bits: the same IEEE operations in the same order, exact square roots and divisions (no
  reciprocal estimates, which differ between Intel and AMD and between SSE and AVX-512), no fused multiply-adds
  (contraction off for the file). `Painter.PolylineKernelsAgree` draws lines of every length (the vector loops' tails),
  turn and width at whole and fractional scales, and long dense series, on every level the processor runs, and compares
  quads, geometry and bounds with plain C++'s byte for byte.
* The pieces go straight into the draw list (`DrawList::AppendShapeQuads`). The check that a piece clears its joints'
  lines - which a long dense series (a point every pixel or two) ran piece by piece in plain C++ - is on the vectors too.
* Also: the quad shapes' antialiasing no longer takes derivatives - fxc had moved them into a branch, and WARP and Mesa
  gave one pixel of a thin line other values -, and the dense scene's pair allows 3 levels where its translucent lines
  meet and a pixel is blended a dozen times over (llvmpipe; 2 everywhere else).

### Verified

* Every test on Windows (clang-cl Debug, `-DESIA_WERROR=ON`): 41 / 41; MSVC `/W4 /WX` and the GNU flags on the new files;
  the kernels' test on SSE2, AVX2 and AVX-512 here (a Ryzen 9 7950X), NEON by the macOS CI. The polylines scenes on
  Direct3D 11 at every level: within 1 level of their geometry, as before. WARP (Direct3D 10 / 11 / 12): every scene.

### Measured

rcbench (round 1's benchmark), Direct3D 11, CPU ms (build + submit), medians of 5 alternated runs; `ESIA_SIMD` picks the
level in one binary:

| scene | plain C++ | SSE2 | AVX2 | AVX-512 | Dear ImGui 1.92.9b | 1.93.0 |
| --- | --- | --- | --- | --- | --- | --- |
| 200 polylines of 64 points, 2 px | 0.082 | 0.046 | 0.043 | 0.043 | 0.085 | 0.100 |
| 8 series of 1 601 points (a point every pixel or two), 2 px | 0.150 | 0.054 | 0.040 | 0.039 | 0.084 | 0.099 |

GPU time 0.028 and 0.021 ms (Dear ImGui 0.043 and 0.041). The round before (one loop a point, its reciprocal square roots
estimated) took 0.087 ms for the first scene. On a Zen 4 AVX-512 runs as two halves: it equals AVX2 here.

### Not done

* Text: labels at new sub-pixel offsets every frame still build in 1.7 times Dear ImGui's time unless drawn on whole
  pixels (round 2); their per-glyph work is lookups more than arithmetic.

## Render-class cost, round 2: much content on screen

Screens that show thousands of shapes, lines and labels at once - charts and maps, diagrams, dashboards, long lists
(2026-10-30). Round 1 left two costs there: every plain line, frame or dot was a 384-byte FX instance (most of what
such a frame uploads), and shapes and labels that overlap one another alternated between the FX program and the
text's, a draw each time (948 draws for a thousand overlapping items of a frame, a bar, a dot, two labels and a line
each, where Dear ImGui takes 2).

### What changed

* Plain shapes are quads (`PainterEnv::quadShapes`, on by default): a solid fill and / or a solid stroke on a rectangle
  with one radius on every corner (continuous corners included), a circle, a capsule or a segment - with no other
  material, outside masks and scales - is a `QuadInstance` of 32 bytes (`QuadShape::Fill`, `Stroke`, `SegmentFill`,
  `SegmentStroke`) instead of an FX instance of 384, drawn in the draws of the text around it. Its fields go where a
  glyph's uv and alpha are: its kind, smoothing (in 1/1023) and stroke align (in 1/64) as one negative whole number, its
  radius as it is, its stroke width (in 1/64 units) beside the alpha - exact in a float on every shader model, so glyphs
  keep their bytes and their shaders.
* The shape quad programs (`QuadShapeImage`, `QuadShapeGray`, `QuadShapeLcd`, `QuadShapeLcdGray`: `QuadShapeVS` and pixel
  shaders over the colors of esia_ui.hlsl's) measure a shape with the FX shader's own distance (`SdRoundRect`,
  `SdSegment`, moved to esia_common.hlsli) and antialiasing, and `QuadShapeVS` pads it by a pixel (a segment's quad turns
  with it), so it covers the same pixels. Batches that hold shapes (`DrawCmd::shapes`, `RenderOp::shapes`) draw with
  them, batches of text alone with the four quad programs as they were; a shape among sub-pixel text has the same alpha
  in every channel. A fill and a stroke fading together (opacity below 1) stay one FX instance: the FX shader puts the
  stroke over the fill and fades the two as one, two quads would blend each on its own.
* Polylines are quads too (`QuadShape::LinePiece`, `QuadShape::LineCap`): a piece per segment and one per round end, 32
  bytes each where a segment's geometry took 152 (four vertices, 18 indices), with the geometry's pixels. A piece's quad
  ends on the lines its geometry's triangles end on (the miters; their directions as diamond angles in 16 bits, no
  trigonometry to encode or decode them; the quad reaching along them as far as the sharpest miter does), and the two
  quads of a joint - or an end and its piece - compute those corners from the same numbers with the same arithmetic:
  they share the edge exactly, the rasterizer gives every pixel on it to one of them, as it does the triangles, on
  every API (Direct3D 9 too: no interpolated value decides it), and a translucent line is blended once. The pixel
  shaders give each pixel the alpha the triangles interpolate - flat inside, linear over the pixel at the edge, an end's
  polygonal fan the same way. A turn sharper than 120 degrees (the geometry clips its miter), a segment too short for
  the turns at its ends, a scale or a mask keep the line geometry.
* `FillRect` and `HLine` are `QuadShape::Solid` quads over the pixels their two triangles covered (no antialiasing, as
  before) instead of indexed geometry: no third program among the text.
* Solid lines without a Style: `Line(a, b, thickness, color)` and `Polyline(points, count, thickness, color, flags)` draw
  what `Style().Fill(color)` draws, quad for quad, without building a Style (a few hundred bytes of defaults) or going
  through the material checks - as `FillRound` does for rounded rectangles; a polyline's quads come from the points
  themselves when they need no smoothing and repeat none. A polyline's joints cost less: the segments' normals in a loop
  of their own (no chain from one segment to the next), reciprocal square roots from the processor's estimate and a
  Newton step on x86, a joint's code from the bisector before its length is known - the diamond angle around the whole
  turn modulo 2, one division and no branch (the joints of a line across a chart turn either way at random) -, and a
  piece longer than twice its quad's reach along x or y is clear of its joints' lines without being measured (a joint's
  line meets the segment at 30 degrees or more).
* A plain shape reads no texture: it joins the quads before it whatever their texture, and a run of shapes takes the
  texture of the first glyph after it (`DrawCmd::anyTexture`, `RenderOp::anyTexture`) - in the list and in the planner -
  so shapes and labels overlapping one another are one draw, in drawing order, whatever order the kinds come in.
* `TextFlags_PixelSnap`: a text from the nearest whole pixel. A label drawn at a new place every frame - one that moves
  with what it names - keeps its placed glyphs instead of placing them again at each new sub-pixel offset; it moves in
  whole pixels (smooth motion does not apply to it).
* Conformance: scenes that draw the same pixels two ways are also compared with each other on every backend
  (`Scene::sameAs`, no channel more than 2 apart): `shapes_fx` with `shapes`, `dense_fx` with `dense` (much content at
  scale 1.25, new), and the new `polylines_mesh` with `polylines`, `polylines_scaled_mesh` with `polylines_scaled`
  (scale 1.5): opaque and translucent lines on a checker turning by up to 115 degrees, steps on whole and half units
  (the joints' lines through pixel centers), ends on pixel centers, a hairline, a thick line, smooth curves, a line
  crossing itself, a short segment between turns, and lines left to geometry. The new scenes' goldens are Direct3D 11
  renders (NVIDIA), within the default tolerance of llvmpipe's.
* Fixed first, in a commit of its own: Polyline's round ends were turned inward - half discs inside the line, which
  ended flat and, translucent, showed them twice; they are beyond the ends now, as the code's comment always said.

### Verified

* Every test on Windows (clang-cl Debug, `-DESIA_WERROR=ON`): 41 / 41 - the conformance suite `--strict` on D3D9 / 10 /
  11 / 12, OpenGL and Vulkan (RTX 4080 SUPER), one frame and three, the pairs above included; the null device's command
  logs regenerated and read. MSVC `/W4 /WX` and the Linux CI's GNU flags (`-Wpedantic -Wshadow -Wnon-virtual-dtor`) on
  the changed sources: no warning.
* Pixels. The quads against their other way on each of D3D9, 10, 11, 12, OpenGL and Vulkan: polylines at most 1 level
  apart, plain shapes 1, the dense scene 2. Every scene against the build before, Direct3D 11: at most 2 levels, no
  pixel more (the round ends' own pixels left out); text, shadows, glows and glass identical.
* The shader library from the Shaders workflow.

### Measured

Against Dear ImGui (round 1's benchmark: both libraries on one device and an offscreen 1600 x 1000 target, RTX 4080
SUPER, medians of 3 alternated runs of 300 frames, CPU = build + submit). Dear ImGui 1.92.9b and 1.93.0, released on
2026-10-14 with a new ImDrawList: thin lines as one textured quad each (about half the CPU and GPU time of 1.92.9b's),
fractional rectangles antialiased (fills nearly 4 times its CPU), strokes inside closed shapes. Direct3D 11, ms, Esia
before -> after (Dear ImGui 1.92.9b / 1.93.0):

| scene | CPU | GPU | draws | upload KB |
| --- | --- | --- | --- | --- |
| 10 000 lines, 1.5 px | 0.568 -> 0.281 (0.338 / 0.176) | 0.231 -> 0.033 (0.136 / 0.061) | 1 (2 / 1) | 3 750 -> 312 (1 914 / 898) |
| 10 000 rect outlines, 1 px | 0.547 -> 0.304 (0.423 / 0.461) | 0.231 -> 0.032 (0.122 / 0.144) | 1 (2 / 2) | 3 750 -> 312 (2 031 / 2 421) |
| 10 000 filled rects | 0.170 -> 0.112 (0.115 / 0.441) | 0.066 -> 0.024 (0.058 / 0.124) | 1 (1 / 2) | 1 015 -> 312 (898 / 2 148) |
| 10 000 rounded rects with a stroke | 0.558 -> 0.369 (2.137 / 2.486) | 0.236 -> 0.053 (0.714 / 0.739) | 1 (8 / 8) | 3 750 -> 625 (12 773 / 13 164) |
| 5 000 circles with a stroke | 0.284 -> 0.185 (1.042 / 1.223) | 0.142 -> 0.048 (0.404 / 0.423) | 1 (5 / 5) | 1 875 -> 312 (7 204 / 7 508) |
| 1 000 overlapping items (a frame, a bar, a dot, two labels, a line) | 0.659 -> 0.428 (0.395 / 0.485) | 0.219 -> 0.095 (0.164 / 0.187) | 948 -> 1 (2 / 2) | 1 696 -> 524 (2 220 / 2 564) |
| ... labels on whole pixels (`TextFlags_PixelSnap`) | 0.305 | | | |
| 100 such items | 0.068 -> 0.040 (0.041 / 0.050) | 0.029 -> 0.015 (0.019 / 0.022) | 102 -> 1 (1 / 1) | 170 -> 52 (222 / 256) |
| 1 000 lines across the screen to one point | 0.059 -> 0.031 (0.033 / 0.021) | 0.113 -> 0.037 (0.032 / 0.035) | 1 (1 / 1) | 375 -> 31 (89 / 89) |
| 200 polylines of 64 points, 2 px | 0.824 -> 0.194 (0.082 / 0.099) | 0.123 -> 0.028 (0.044 / 0.043) | 1 (1 / 1) | 1 998 -> 406 (647 / 647) |
| 3 000 labels | 0.418 -> 0.422 (0.248 / 0.248) | 0.050 (0.133 / 0.133) | 1 (2 / 2) | 747 (2 150 / 2 150) |
| ... on whole pixels | 0.203 | | | |

Direct3D 12, Vulkan and OpenGL alike. In the tree's `esia_bench` (alternated, 5 runs), Direct3D 11: 10 000 plain shapes
build 0.363 -> 0.396 ms, submit 0.356 -> 0.026, GPU 0.236 -> 0.046, upload 3 750 -> 625 KB; plain widgets submit 0.025
-> 0.006, GPU 0.034 -> 0.025, 2 draws -> 1; text the same; glass GPU 0.409 -> 0.395. Cipher's demo (an application on
the widgets, 1920 x 1080, 5 alternated runs): with a thousand items of shapes and labels on screen the frame 0.891 ->
0.679 ms on Direct3D 11 and 1.042 -> 0.791 on Direct3D 12, GPU 0.233 -> 0.069 and 0.228 -> 0.102, 538 draws -> 1; its
ordinary screens within 1 %.

The solid lines and the cheaper joints came after these numbers; measured again on the same benchmark (4 builds
alternated, 3 runs each), Direct3D 11, ms, before -> after (Dear ImGui 1.92.9b / 1.93.0):

| scene | CPU | build | GPU |
| --- | --- | --- | --- |
| 10 000 lines, 1.5 px | 0.294 -> 0.137 (0.338 / 0.176) | 0.280 -> 0.122 | 0.033 (1.93.0: 0.063) |
| 1 000 lines across the screen to one point | 0.031 -> 0.015 (0.033 / 0.021) | 0.027 -> 0.011 | 0.037 (0.043) |
| 200 polylines of 64 points, 2 px | 0.189 -> 0.087 (0.079 / 0.100) | 0.173 -> 0.071 | 0.028 (0.046) |

Direct3D 12 and Vulkan alike (the polylines 0.88 and 0.94 times Dear ImGui 1.93.0's CPU); every other scene the same as
before.

### Not done

* Polylines against Dear ImGui 1.92.9b's CPU: 1.04 to 1.11 times (the joints' arithmetic one joint at a time; SIMD
  would take four). On OpenGL Esia's submission is most of the small scenes: 1 000 lines across the screen 0.046 ms
  against Dear ImGui 1.93.0's 0.037, the polylines 0.125 against 0.114 (their build the same as on the other APIs).
* Labels at new sub-pixel offsets every frame still build in 1.7 times Dear ImGui's time unless drawn on whole pixels
  (`TextFlags_PixelSnap`: 0.82 times).
* A plain shape between FX shapes that overlap it changes program twice (a draw each): scenes of glass and shadows with
  a plain rectangle among them take a draw more; scenes of text and plain shapes take fewer.
* Arcs and rings, per-corner radii, fading strokes and every other material stay FX instances; so do polylines with a
  material (FX paths) and, as geometry, the ones above.
* Vulkan: the uploads' barrier waits for all earlier work and every buffer has one device copy (round 1's). Done in
  [Vulkan frames in flight](#vulkan-frames-in-flight).

## Glass on every shape

At the owner's request: every shape Painter draws takes every material, glass included - any outline (a slanted tab, a
star) as a new FX shape kind, and Polyline / Area / Sector with a material.

### What changed

* `fx::ShapeKind::Path` with the feature `fx::kPath`: a polygon (any simple outline, convex or not, either winding;
  inside by the even-odd rule) or a polyline (its distance less half its thickness: round joins and caps), its points in
  `ShapeKind::PathData` instances right after it, 32 each (two to a row; the flags row marks them as data). The shader
  (`SdPath`) takes the exact distance to every edge, and the gradient with it - glass's normal in one pass over the
  points instead of four finite differences; a data instance draws an empty quad outside the target. The planner keeps
  the data behind their path - in its piece, its batch, out of sight with it (`PathData`) - so batching moves instances
  as before and the shader always finds a path's points after it. The row arithmetic is in floored floats: SM3 has no
  integers (its `uint` is a float, which divides without truncating), and its loops run 255 times at most (two nested
  loops of at most 16 x 128 rows).
* `kPath` is variant-only (`fx::kVariantOnlyFeatures`): the full shader and every other batch carry none of it. The
  ps_3_0 variant of a glass path is about 3250 instruction slots.
* Painter: `Polygon`, `Parallelogram` (CSS `skewX` around the center, the style's four radii its corners),
  `RegularPolygon`, `Star`; `Area` and `Sector` with a `Style`; `Polyline` with a material geometry cannot draw (glass, a
  shadow, an inner glow, an outline, a gradient, an image, shimmer, noise, an effect, motion blur) as an open path, its
  color still the polyline's. Rounded corners: the outline is inset by the smallest convex radius, which the shader
  takes off the distance again - every convex corner an exact circular arc; concave corners, and corners rounder than
  that, get fillets of points within a fortieth of a pixel; each corner is as round as half of each edge beside it
  allows (an edge inset to nothing is fine: a capsule's ends). An outline longer than 4096 points (`fx::kPathMaxPoints`)
  is thinned out.
* Glass on a thin shape: its bevel comes no deeper than the shape goes - half a ring's or a line's thickness, a path's
  depth (probed on the CPU, `OutlineDepth`) - so a thin shape is one rounded rod. It was half the bounds' shorter side,
  which a ring or a line never reaches: a crease along its middle. Painter clamps `glass.z`; done in the shader, the few
  more instructions broke the full ps_3_0 shader on NVIDIA's Direct3D 9 (32 temporaries, 3.9k of its 4096 slots: glass
  came out over-saturated).

### Verified

* Conformance scenes `paths` (slanted tabs of frosted, tinted and outlined glass with per-corner radii, a rounded
  triangle with a gradient and a shadow, a clear dispersive star, a frosted hexagon with an inner glow, an arrow whose
  concave corners are rounded, a glass ring and segment, a glass polyline, area and ring sector) and `path_rows` (the
  same with 2 FX instances per row of the instance texture): Direct3D 9, 10, 11 and 12, OpenGL, GLES and Vulkan against
  the Direct3D 11 golden within 3 levels; every other scene unchanged; ctest 41 / 41; MSVC /W4 /WX and the GNU flags
  clean.
* Unit tests: a path and its points in one command, both windings, two data instances for 40 points; corners inset and
  clamped, concave fillets; the parallelogram's slant and corners; bevels clamped on rings, lines and paths; lines,
  areas and sectors with a material as paths (a whole ring as the FX ring, a pie's slice from the center); the scale
  stack and culling; the planner keeping the points behind their path (a plain batch, a glass batch, a direct plan).

### Not done

* Holes - several contours in one path: a ring is `Ring`, an annular sector `Sector`.
* A path's cost grows with its points (every pixel of its quad visits every edge): keep an outline to the points it
  needs. A long chart line with glass is thousands of edge visits per pixel.
* Glyphs with glass keep the bevel of the text's box (a glyph's depth is not known).

## Render-class cost, round 1

The renderer against Dear ImGui's as a render class (2026-10-28): the same scenes drawn through ImDrawList and its
renderer backends (Dear ImGui 1.92.9b, unpatched) and through Esia's Painter and Renderer, on the same host device and
target - lines, rect outlines, filled rects, rounded rects with a stroke, circles, a page of text, short labels,
controls and markers over a scene (a frame, a bar, a circle, two labels and a line to each) - and an outside review's
findings (`64c1753`), each checked against the code.

### What changed

* A segment's FX quad lies along it (FxVS): a band of the thickness plus the padding to each side and past each end,
  turned with the segment, instead of its bounding box - a long diagonal line (a leader line across the screen) shaded
  every pixel of its box. A mask, a halo or motion blur keeps the box. The pixels are the same (the conformance scenes'
  diagonal and straight lines, every backend).
* Text: a text's cached quads carry no color - EmitQuads colors them as they go in, so a text that fades, changes its
  tint or cross-fades (smooth motion) is no longer placed again; every glyph's atlas slot is kept per pen phase, so a
  text drawn at other sub-pixel offsets (a label that moves with what it marks) is placed again without the atlas's map; a text
  placed again and again without being drawn twice at one offset goes straight into the list without being kept (every
  8th placement is, to see whether it settled); its quads keep their lines' extents, so a clip that cuts a long text
  looks at the lines it holds only, and reserves room for their quads only (a 10 000-line text in a 100-pixel view
  reserved 11 MiB); `GlyphSlot::quadUv` packs a slot's uv once; the layout cache is swept a few buckets a frame
  (256), not wholly every frame once it holds more than 1024 layouts, and a layout that leaves clears its own front
  slot only.
* Direct3D 12: `MapBuffer` in a frame writes into the frame's upload ring (no CPU copy, no copy at the bind) and as many
  bytes as were written - the buffer's length was the most it ever held, so one large frame made every later one copy
  that much; descriptor tables are reused per frame by their bindings (text, shapes and images alternating copied a
  table at every change, and 2 340 changes ran out of the frame's 16 384 descriptors). `Device::MapBuffer`'s contract
  says what Direct3D 11's `WRITE_DISCARD` already did: what the buffer held is gone, and its bytes may last for the
  frame only.
* The renderer uploads no geometry when no op draws any (a chart scrolled out of its clip still uploaded its vertices);
  captures are keyed by what was drawn under them only for `RenderParams::targetStatic` (`FramePlan::keyCaptureContent`):
  reuse for a number of frames compares a capture's region and pyramid alone.
* Painter: `Line` without a copy of the style; smooth paths reuse their scratch (a 1000-point curve allocated 20 times).

### Verified

* Every test on Windows (clang-cl, `-DESIA_WERROR=ON`): 41 / 41, the conformance suite `--strict` on D3D9 / 10 / 11 /
  12, OpenGL, GLES and Vulkan (render passes and dynamic rendering); D3D11 and D3D12 also with the debug layers and
  GPU-based validation (`ESIA_D3D_DEBUG=2`), one frame and three: no message. The shader library from the Shaders
  workflow (`Fx.vs` only changed).
* Against Dear ImGui (a scratch benchmark, not in the tree: both libraries on one D3D11 / D3D12 / Vulkan / OpenGL
  device and an offscreen 1600 x 1000 target, RTX 4080 SUPER, the GPU waited for after every frame, medians of 3
  alternated runs of 300 frames). Direct3D 11, ms, Esia before -> after (Dear ImGui):

  | scene | build | submit | GPU |
  | --- | --- | --- | --- |
  | a page of text (two columns, Latin and Chinese) | 0.037 -> 0.011 (0.026) | 0.007 -> 0.006 (0.011) | 0.010 (0.023) |
  | 3 000 labels at random positions | 0.615 -> 0.393 (0.181) | 0.029 (0.071) | 0.048 (0.133) |
  | 10 000 lines, 1.5 px | 0.323 -> 0.242 (0.280) | 0.345 -> 0.351 (0.062) | 0.315 -> 0.231 (0.134) |
  | 1 000 lines across the screen | 0.050 -> 0.025 (0.029) | 0.069 -> 0.037 (0.004) | 3.371 -> 0.095 (0.031) |
  | 1 000 markers | 0.844 -> 0.418 (0.328) | 0.379 -> 0.264 (0.072) | 3.944 -> 0.186 (0.163) |
  | controls (capsules with labels) | 0.040 -> 0.032 (0.108) | 0.027 -> 0.025 (0.031) | 0.035 (0.052) |
  | 10 000 rounded rects with a stroke | 0.242 -> 0.239 (1.747) | 0.337 -> 0.341 (0.471) | 0.230 (1.087) |

  Direct3D 12 submits less: 10 000 lines 0.477 -> 0.392 ms, the markers 0.531 -> 0.365 (one copy of the frame's data
  fewer). The markers are 948 draws (Dear ImGui: 2): they overlap, and shapes, geometry and text are
  three pipelines - see Not done.

### Not done

* One pipeline for plain shapes and text: solid rounded rects, circles, segments and fills as compact instances
  drawn with the glyph quads (the shapes' 384-byte FX instance is most of what 10 000 lines upload, and the markers'
  948 draws are kinds alternating, which on OpenGL cost 1.5 ms of GPU time). Done in
  [round 2](#render-class-cost-round-2-much-content-on-screen).
* Labels at new sub-pixel offsets every frame still build in twice Dear ImGui's time (0.39 against 0.18 ms for 3 000):
  every glyph is put on the pixel grid at a quarter-pixel phase, which Dear ImGui does not do. Round 2: such labels
  drawn on whole pixels (`TextFlags_PixelSnap`) build in 0.82 times its time.
* Vulkan: the uploads' barrier waits for all earlier work and every buffer has one device copy (frames in flight
  serialize); to be measured with frames in flight, which the benchmarks do not run. Done in
  [Vulkan frames in flight](#vulkan-frames-in-flight), measured with `esia_bench --in-flight`.

## HDR output

The colour contract for HDR targets (2026-10-28): the UI looks the same on an HDR display as on an SDR one at the same
brightness, and blends with an HDR scene under it.

### What changed

* `RenderParams::hdr` (`HdrOutput`): the target's encoding - `HdrEncoding::ScRgb` (linear BT.709, 1.0 = 80 nits, an
  RGBA16_FLOAT target: Windows' HDR), `Pq2020` (SMPTE ST 2084 code values with BT.2020 primaries, an RGB10A2_UNORM
  target: HDR10) or `None` (SDR, as before) - and `paperWhiteNits`, the luminance of UI white (default 203, ITU-R
  BT.2408's graphics white; a host passes its monitor's SDR white so the UI matches the desktop).
* The working target: the frame draws into an RGBA16_FLOAT texture holding what an SDR target would - sRGB values,
  1 = paper white - with what lies past them kept ("extended sRGB": above 1 brighter, below 0 outside BT.709). So
  blending, text gamma, glass and glow layers are those of SDR, unchanged. `HdrDecode` brings the region the frame
  covers (its ops' bounds, glass regions and captures) into it from the target first; `HdrEncode` takes the region
  back in the target's encoding afterwards (the sRGB curve continued past [0, 1] and mirrored below 0; PQ; the
  BT.2087 primaries matrices). Pixels outside the region are not touched; inside it a pixel the frame does not draw
  on goes through decode and encode unchanged. A target that cannot be sampled is copied first. Needs
  `Caps::floatRenderTargets` and a single-sampled target; Render returns false otherwise.
* Programs `HdrDecode` / `HdrEncode` (esia_post.hlsl, FullscreenVS; Direct3D 9 samples their t0 with points).
* Windows: `win32::MonitorHdr` / `FrameInfo::hdr` - whether the window's monitor is in HDR (advanced color) and its
  SDR white level in nits, from the display configuration. glass_window `--hdr`: an scRGB swap chain on Direct3D 11 /
  12 (`DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709`) and the UI at the monitor's SDR white.

### Verified

* `esia_hdr_tests` on every backend built in (D3D9 / 10 / 11 / 12, OpenGL, GLES, Vulkan; Metal on macOS CI), a ctest per
  backend (`esia_hdr_<backend>`; the Windows runner leaves out WARP's D3D9 and lavapipe, docs/CI.md): opaque
  sRGB grays 0 / 0.25 / 0.5 / 1 come out of an scRGB and a PQ target as the transfer functions computed on the CPU say
  (8-bit readback, within 2); a half-transparent white over the last frame's gray reads that gray back and blends to
  sRGB 0.75, and the pixels outside the second frame's region are the first frame's to the bit.
* Conformance scenes `hdr_scrgb` (paper white 80 nits: the target then holds linear sRGB) and `hdr_pq` (203 nits), the
  glass scene through the working target, against D3D11's goldens: every backend within 0-7 levels, 3 frames too.
  `hdr_scrgb` converted back to sRGB matches `glass`'s golden (mean 0.5 levels; the rest is 8-bit linear quantization).
* glass_window `--hdr --debug` on Direct3D 11 and 12 (RTX 4080 SUPER, a monitor in HDR at an SDR white of 80 nits):
  no debug-layer messages.
* Cost (`esia_bench --hdr`, 1600 x 1000, frames covering the whole target, GPU ms): widgets 0.036 -> 0.054 (D3D11),
  0.036 -> 0.055 (D3D12), 0.037 -> 0.062 (Vulkan) - the decode and the encode of the whole target; glass 0.41 ->
  0.45, 0.39 -> 0.43, 0.53 -> 0.57 (the FP16 working target besides). CPU unchanged. A UI over part of a scene
  pays for its region only.

### Not done

* Brighter-than-paper-white UI (HDR glows, highlights): the shaders still produce SDR values.
* Vulkan, OpenGL and Metal hosts with an HDR swap chain in the example (the renderer side works on all of them).

## Quads as instance attributes

On OpenGL and Direct3D 9 / 10 the instanced quads (glyphs, flat tiles) went into the RGBA32F instance texture with the
FX instances and were fetched two texels a vertex: text then cost OpenGL's GPU and Direct3D 9's submit more than the
vertices before it. `Caps::quadAttributes` takes them as instance-rate vertex attributes instead (`VertexLayout::
QuadInstance`, `BufferKind::QuadInstances`, `DrawInstancedFrom` from each run's first quad). Text, before the text work
/ with it / now: OpenGL GPU 0.016 / 0.026 / 0.012 ms, Direct3D 10 GPU 0.026 / 0.018 / 0.012 ms; Direct3D 9's submit
0.036 / 0.067 / 0.049 ms (NVIDIA's driver spends about 8 ns a quad on an instanced draw; building the frame went 0.072
-> 0.045). The showcase and the workbench on the three: within 4% of before the text work, most of them faster.

## One UI light, motion blur

The last two of the owner's picks: one light for the whole UI, which a cursor or a device's tilt can move, and motion
blur computed from the shapes themselves.

### What changed

* `RenderParams::light` (`UiLight`: `Point(at, height)`, `Direction(towards)`, `FromTilt(pitch, roll)`), in the Draw
  constants (`gDrawInfo.yzw`: the point or direction, and 0 none / 1 a direction / 1 + the height of a point; the
  frame constants keep their layout). With it, glass rims and the side of a slab facing it catch the light from it
  (per pixel: a point light near a panel lights the part of the rim nearest to it), and drop and inner shadows fall
  away from it - their offsets turned, their lengths kept; the planner then counts a shadow's offset as its length
  every way (`FramePlan::lightShadows`). Without it each glass keeps its light angle, each shadow its offset.
  glass_window: `--light` puts it at the cursor.
* `Style::MotionBlur(displacement)`, feature `fx::kMotion` (the displacement in the custom row; not with a user effect,
  not when drawing flat; scaled under `PushScale`): FxPS averages the pixel's coverage along the path - the distance
  taken at the ends of up to 8 segments at most 4 pixels long, the coverage ramp integrated exactly over each for a
  distance that changes linearly along it (`SegmentCoverage`) - for the fill and the stroke; FxVS and the planner
  cover the area swept. Text with a style moves the same way (every glyph shape, its outline pass too). A first
  attempt that blurred each edge along its own normal (one closed form per pixel) pinched curved ends and broke thin
  strokes at motions of tens of pixels; the path integral does not.
* `fx::kVariantOnlyFeatures` (`kGlyph | kLegible | kMotion`): the features the full FX shader leaves out on every
  backend that can specialize it (`fx::kFullShaderFeatures`; all but Metal without function constants); the renderer
  and the null device decide glyph and motion variants from it. Kept in, every FX pixel paid for them: ps_5_0 went
  from 23 temporaries and ~3400 instruction slots to 25 and ~4600, and a frame of 10,000 shapes from 0.236 to 0.260 ms
  of GPU on D3D11 (0.242 -> 0.262 D3D12, 0.209 -> 0.217 Vulkan); left out, 0.233 / 0.240 / 0.207 again.

### Verified

* Conformance scenes `motion_blur` (a capsule, a circle, a stroked card and outlined glyph shapes moving, beside the
  same at rest) and `ui_light` (glass and shadowed cards around a point light): D3D9, D3D10, D3D11, D3D12, OpenGL,
  GLES and Vulkan against the D3D11 goldens (OpenGL / GLES within 29 levels on 0.03 % of the motion pixels); every
  other scene unchanged; ctest 37 / 37.
* Unit tests: the displacement with its feature, scaled, left out with an effect or flat, on text; the swept area in
  the planner's bounds; the light in the Draw constants and the shadows' symmetric reach; moving shapes taking their
  variant where the full shader lacks motion blur.

### Not done

* No platform layer feeds a device's motion yet (`UiLight::FromTilt` takes the angles); EsiaGUI's springs could pass
  their velocity to `MotionBlur`.
* Shadows and glows are not blurred along the motion (they are soft already); images and coverage text (quads) are not
  motion-blurred - text with a style is.

## Temporal reuse of the backdrop

One of the owner's picks: the glass's backdrop capture (a copy and a blur pyramid) taken again only when it has to be.

### What changed

* Planner (`FramePlan::keyCaptures`, `KeyCaptures`): every capture gets a key - its region and pyramid mixed with
  every op before it that touches the region, in order (kind, state, data: FX instances, quads, the vertices of
  geometry); every op before it when one of those reads the backdrop itself or is a glow layer. 0 when it cannot be
  told: a host callback, content that animates on the shader's clock (shimmer, a light streak without its own clock,
  user effects), more than 256 KB of data in one op. `RenderOp::captureShape`: the region and pyramid alone.
* Renderer: `RenderParams::targetStatic` (the host gives the target the same content before every frame) reuses a
  capture whose key is last frame's; `backdropReuseFrames` reuses one of the same shape for that many frames whatever
  changed (a live background, the frost that many frames old). Captures share the copy and the pyramid, and the next
  capture writes over a capture's margins, so a kept capture whose levels were written over is brought back from a copy
  of them kept aside (made before the frame's passes for captures whose content stayed the same): a few
  `CopyTexture`s instead of a copy, a blur and its passes. The pyramid passes load instead of DontCare while captures
  are kept; the levels and the backdrop copy are copy sources and destinations. `RenderStats::reusedCaptures`,
  `restoredCaptures`.
* Conformance: `Scene::repeat` draws a frame several times on one renderer (the target cleared before each, with
  `targetStatic`) and checks the last; the scene `glass_reused` is the glass scene three times, against its golden.
  `esia_bench`: `glass-static`, `glass-live` (the stripes scroll), `glass-live-2` (reused for two frames); the bench
  turns the Vulkan validation layer off (`ESIA_VULKAN_VALIDATION=0` unless set).

### Measured (`esia_bench`, 20 glass panels with titles, 1600 x 1000, RTX 4080 SUPER, medians)

| backend | glass: submit / GPU ms | glass-static | glass-live-2 |
| --- | --- | --- | --- |
| Direct3D 11 | 0.042 / 0.412 | 0.039 / 0.369 | 0.044 / 0.373 |
| Direct3D 12 | 0.143 / 0.403 | 0.162 / 0.369 | |
| Vulkan | 0.198 / 0.550 | 0.179 / 0.457 | 0.190 / 0.463 |
| OpenGL | 1.095 / 0.887 | 0.997 / 0.692 | |

121 draw calls become 41. The panels lie close together, so every capture of `glass-static` comes back from its copy
aside (20 restores a frame); a lone glass window is reused without a copy. The glass's own shading, not its capture,
is most of the GPU time here: 6 to 22 % saved on these desktop backends. Tile-based GPUs (Apple, mobile), where each
capture ends and restarts a render pass, should gain more; not measured.

### Verified

* Unit tests (null device): an unchanged capture reused without a pass, taken again when something under it changes,
  without the promise, or under a shimmer; glass on glass restored from the copies aside from the third frame on, with
  no pyramid pass and no texture made; `backdropReuseFrames` over a changing background (taken, reused twice, taken
  ...), a moved capture taken at once.
* `glass_reused` on D3D9, D3D10, D3D11, D3D12, OpenGL, GLES and Vulkan within the glass golden's tolerance (the same
  deltas as `glass`); its null log shows the third frame's restores; ctest 37 / 37.

### Not done

* The examples promise `targetStatic` since 2026-10-30 (`App::opaqueBackground`: their wallpapers cover the target), but
  their backgrounds move, so no capture is reused there (glass_window, the showcase and the workbench: 0 of 1 200,
  6 652 and 1 206 in 600 frames; GPU time the same with `--no-reuse`). An app over a still background gains.
* Captures that write over each other are restored every frame (copies); separate storage per capture would keep
  them, at more memory.

## Instanced glyphs and tiles

One of the owner's picks (text GPU instancing), and the structural step the plain-widget measurements of 2026-10-07
pointed at: every glyph was four vertices and six indices written by the CPU (104 bytes), every flat capsule eight
vertices and eighteen indices.

### What changed

* `DrawList`: quads drawn instanced - `QuadInstance` (two float4 rows: the rect; the uv in 1/4096 and the color as
  whole numbers, exact in a float), `DrawCmdKind::Quads`, `BeginQuads` / `EndQuads` write them in place, `AddQuad`,
  `Quads()`, `RefreshBounds` for quads. Mirrored uv are not quads (images stay geometry).
* The FreeType text system keeps each layout's quads packed (pixel rects from the origin, uv and color packed once)
  and copies them in with the offset; single glyphs, outlines and icons are quads too. The Painter's flat tiles are
  their cells as quads (uv packed per tile), the plain middle of a large fill a quad of the white texture; `PopScale`
  scales quads as it scales vertices.
* Planner: quads are draws of their own kind (`RenderOp::quads`, joining only draws of quads), kept as runs in the
  lists (`FramePlan::quadRuns`). Renderer: the quads follow the FX instances in the instance data (`gConv.w`: a row
  of the buffer; on the instance texture 8 quads in the room of an FX instance, a power of two per row for SM3), and
  draw with `QuadImage` / `QuadGray` / `QuadLcd` / `QuadLcdGray` (`QuadVS` in `esia_quad.hlsl` with the UI pixel
  shaders): one instanced draw per batch, no index buffer.
* RHI: the four programs, `DrawsInstances` and `DualSourceProgram` (the backends' FX-only and TextLcd-only checks:
  Direct3D 11 / 12 instance-index attribute, Direct3D 9 vertex declaration, Metal planning, the null device).
  Direct3D 9: the instance texture at most 4096 texels wide - NVIDIA's driver failed the 16384-texel system-memory
  surface of a full-row upload (E_FAIL) and misbehaved after it.
* `esia_bench` (`tests/bench`): text, controls (regular and flat), shapes, glass through a headless device of any
  backend; median and 99th percentile of build, submit and GPU time, what a frame uploads. CTest runs it three frames
  on the null device.

### Measured (`esia_bench`, Release clang-cl, 1600 x 1000, RTX 4080 SUPER, medians of alternating runs)

| scene | build ms | submit ms | GPU ms | upload KB |
| --- | --- | --- | --- | --- |
| text (3418 glyphs), Direct3D 11 | 0.065 -> 0.037 | 0.014 -> 0.007 | 0.027 -> 0.010 | 347 -> 107 |
| controls (324 capsules, labels), Direct3D 11 | 0.052 -> 0.041 | 0.028 -> 0.024 | 0.047 -> 0.037 | 282 -> 171 |
| controls flat, Direct3D 11 | 0.100 -> 0.088 | 0.012 -> 0.008 | 0.035 -> 0.020 | 307 -> 110 |
| text, Direct3D 12 / Vulkan | 0.065 -> 0.039 / 0.066 -> 0.041 | 0.057 -> 0.044 / 0.224 -> 0.201 | 0.028 -> 0.013 / 0.027 -> 0.017 | |
| text, OpenGL | 0.075 -> 0.044 | 0.251 -> 0.174 | 0.018 -> 0.028 | |
| text, Direct3D 9 / 10 | 0.078 -> 0.044 / 0.064 -> 0.038 | 0.034 -> 0.064 / 0.031 -> 0.029 | 0.031 -> 0.034 / 0.026 -> 0.018 | |

Shapes and glass are unchanged (FX instances). Where the instance data is a texture (OpenGL, GLES, Direct3D 9 / 10)
the GPU fetches each quad by texel in the vertex shader: on OpenGL a little more GPU time, on Direct3D 9 more submit
time (the texture's upload through a system-memory surface), both less than the CPU time saved.

### Verified

* Every conformance scene on D3D9, D3D10, D3D11, D3D12, OpenGL, GLES and Vulkan against the goldens of the vertex
  path, unchanged (the scenes' glyphs are quads now); the new scene `quad_rows` (the text scene with 16 quads per row
  of the instance texture, after an FX row) on all of them; the null logs regenerated (one instance buffer, no vertex
  or index buffers in a frame of text); ctest 37 / 37.
* Unit tests: quad runs and single quads, the packing, quad bounds and their refresh, scaled quads, text drawn at its
  final geometry under a scale, flat tiles as quads batching with the text; the FreeType tests decode the quads as the
  shader does (golden images unchanged).

### Not done

* Instance-rate vertex attributes would spare the texture-storage backends their texel fetches (two rows a quad fit
  in attributes, unlike the 24 of an FX instance).
* Images are still geometry (any uv, mirrored, any texture size); FX instances still take 384 bytes each (the shapes
  scene uploads 3.75 MB for 10000 of them: the next structural step for shape-heavy frames).

## Smooth motion and legible text

Two of the text ideas the owner picked: hinted text that moves without jumping from pixel to pixel, and text that
stays legible over whatever lies behind it.

### What changed

* Smooth motion (`TextRendering::smoothMotion`; on in `Light`, `Crisp`, `ClearType` and `native`; parser
  `smooth-motion[=0|1]`): the FreeType text system follows each text from frame to frame by its layout and the order
  of its draws in the frame. A text whose position or scale changed is drawn from the design's outline, unhinted, at
  quarter pixels across and down (glyphs keyed apart in the atlas); once it stops, its moving and hinted quads are
  drawn together, their alphas crossfading over 0.12 s (`RasterParams::time`; 8 frames without a clock), then the
  hinted ones alone. Outlines (`DrawOutline`) follow the same rule. `RasterizeGray` / `RasterizeGrown` /
  `RasterizeLcd` take a 2D offset; the examples hand the frame's time to `NewFrame`.
* Legibility (`Style::Legible(strength, radius)`, feature `fx::kLegible`): `Painter::Text` with a fill asks for it
  (not when drawing flat); the text system puts it in the outer pass, under all faces, with its radius (by default a
  sixth of the em). The FX shader reads the captured backdrop at the pixel, as blurred as the halo is wide (4 px at
  least), and draws a halo of the other tone (black for light text, white for dark) as strong as the lightness
  contrast is missing: full within a quarter, none beyond a half. The planner treats it as a backdrop reader (a
  capture, the pyramid's levels for its radius, never level 0); Direct3D 9's full shader leaves it to the glyph
  variants.
* Painter: under `PushScale` a text's stroke, shadow, glows, glass and halo now scale with its glyphs (they kept their
  unscaled size).

### Verified

* The text system: moving text drawn from glyphs of its own, both renderings while it settles (alphas adding up to
  255), hinted again after 8 frames or 0.12 s; the ink moving by quarter pixels down (whole pixels without smooth
  motion); the halo under the faces with the default and a given radius. The Painter: the halo's strength and
  radius, none without a fill or when flat, scaled with the glyphs. The planner: a capture, a blur of 16 px or the
  halo's radius, no level 0.
* The conformance scene `text_legible` (white and black glyph shapes over a black-to-white gradient, a gray run on
  gray): D3D9, D3D10, D3D11, D3D12, OpenGL, GLES and Vulkan equal to the D3D11 golden; ctest 36 / 36.

### Not done

* The halo's tone is decided per pixel from the blurred backdrop: over detail finer than the halo it is even, not one
  per detail.
* Smooth motion follows texts by their order of drawing: a text drawn in another order next frame counts as moved
  (it settles again within 0.12 s).

## Distance-field text

The owner asked for native glows, outlines, shadows, glass and blur on text, all optimized, and for MSDF. Text can
now be drawn from multi-channel signed distance fields made by Esia, as FX shapes with any Painter material.

### What changed

* `GenerateMtsdf` (`include/esia/text/distance_field.hpp`, `GlyphShape`): curves flattened to 1/32 px, the filled
  area resolved (pieces of edges cut where contours cross, those inside the area dropped, the rest linked into loops
  with the inside on the left), loops cut at corners (turns over about 8 degrees, exact tangents at the curves' ends)
  and colored per run (Chlumsky's switching, a teardrop for one corner), per texel the pseudo-distance to the nearest
  run per channel (from a grid of the pieces within reach) and the true distance in A (nearest points handed on from
  texel to texel past the multi-channel reach), signs from the non-zero winding, clashes flattened to the median.
* The FreeType text system: `DrawShapes` (fields at 48 pixels per em, ranges 8 / 24, on a field atlas; one per glyph
  for every size), the two passes, color and bitmap glyphs as their bitmaps; `TextRendering::distanceFieldFrom`
  (parser: `distance-field=N`).
* FX: `ShapeKind::Glyph`, feature `kGlyph` (variants of other batches leave the glyph code out), `SdGlyph`: the median
  near the edge, the true distance farther, the ink box's signed distance as a lower bound past the reach. The quad
  and the planner's bounds come from `shape2`. The full FX shader leaves it out (`Caps::fullFxGlyphs` = false; on
  Direct3D 9 the full ps_3_0 shader has no registers left for it), so glyph batches build their variant at once.
* Painter: `Material()` out of `EmitShape` (shapes unchanged), `Text` / `TextBox` with a `Style`, `Style::Weight`.
  glass_window's text wrappers pass `DrawShapes` and `DrawOutline` on (outlines did not reach the inner system).
* Without the hitch (2026-10-30). A text that appears with glyphs the field atlas does not have had their fields made
  one by one on the UI thread, in its draw: 0.37 ms a Latin glyph, 0.58 a Chinese one (Release), so a paragraph of 200
  new glyphs held its frame for 75 to 120 ms. `FieldMaker` (`distance_field.hpp`) makes the fields of a batch together:
  worker threads (one per processor but one, at most 15: `FreeTypeDesc::fieldThreads`, 0 = the UI thread alone) start
  on a shape as soon as it is handed over, and the thread that waits makes the rest with them - `GenerateMtsdf` itself,
  the same bits on every thread. `DrawShapes` looks up every glyph's field once before its passes, reads the new
  glyphs' outlines on the UI thread (FreeType's faces stay there), makes them as one batch and packs them in the order
  the passes meet them: the fields, their places in the atlas and every instance are what the one-by-one way gave.
* A cache on disk, off by default (`FreeTypeDesc::fieldCacheDirectory`): each field a file named by what makes its
  bits - every point of the outline as the generator gets it, the ranges, `kDistanceFieldVersion` (raised when the
  generator's output changes), the kind of processor and compiler -, so another font file, face, variable instance or
  range never finds a field that is not its own (the outline is what a font file's content decides of a field). The file
  holds a check of that key and one of its texels: a damaged, cut or swapped file is made again and written anew. The
  workers read it; one of them writes the new fields afterwards, behind any batch, beside their place and renamed over.
* The generator's search, bit for bit the same: the pieces of each cell in one array, each piece's direction made once,
  the tie-breaking angle worked out only for the pieces close enough to need it - 11 % less time a field.
* `esia_field_bench` (`tests/bench/field_bench.cpp`): the draw that meets a paragraph of new glyphs against the same
  paragraph again - with a style, from a cache, as frames on a backend; `--digest` hashes every glyph's field of a font,
  to compare two builds' generators bit for bit.

### Verified

* Generator tests: the square's distances (median and A to 0.1 px), corners held to a twentieth of a pixel, either
  winding, holes, overlapping squares filled as their union (A exact to the union's outline), a cubic circle, a
  triangle, an L and a drop sampled at 6 x on the right side everywhere but 0.1 px of the outline.
* The text system (two passes, the text's box, one field per glyph for every size, large text from fields), the
  Painter (the material handed on, flat drawing), the renderer (glyph variants where the full shader lacks them).
* The conformance scene `text_fields` (made with the generator, no font): D3D9, D3D10, D3D11, D3D12, OpenGL, GLES and
  Vulkan within 2 levels of the D3D11 golden (D3D9 through its variant); ctest 36 / 36.
* glass_window with a title in white with a blue outline, a pink glow and a shadow (Latin and Chinese), a gradient
  across a word and glass text, on Direct3D 11.
* The hitch's work (2026-10-30): `FieldMaker.SameBitsAsMadeOneByOne` (corners, holes, overlaps, curves, a star, an
  empty shape and one too large; 0, 3 and 8 workers, batch after batch), `FieldMaker.CacheOnDiskKeepsTheSameFields` (a
  second maker reads every field, the same bits; another range, a point moved by one float step, a texel changed, a
  file cut short or grown, another field's file under its name: made again, and the file put right),
  `FreeTypeRendering.FieldsFromWorkersAndDiskAreTheSame` (three texts sharing and repeating glyphs, Latin and Chinese:
  instances, the draws' pages and the field page byte for byte from 0, 3 and 15 workers and from the cache). The
  generator's change: the fields of every glyph of 18 fonts - the test fonts, Segoe UI, Microsoft YaHei, SimSun,
  Malgun Gothic, the variable Noto Sans SC, Bahnschrift, Cascadia Code and Sitka, Segoe Fluent Icons, Segoe UI Symbol
  and Emoji, Georgia, Times New Roman, Consolas, Arial: 240 000 fields - hash as 9df4411's. Every test (clang-cl Debug,
  every backend, the conformance goldens with `text_fields` among them): 43 / 43; MSVC `/W4 /WX`; the GNU flags on the
  changed files.

### Measured

`esia_field_bench`, Release clang-cl, Ryzen 9 7950X (16 cores), against 9df4411 alternated run by run: 5 runs of 5 to 9
rounds, each a new text system. Medians of the runs' medians, in brackets the fastest and the slowest round; the draw
that meets the paragraph, once its text system has drawn fields before (the first fields also create the field page:
16 MB zeroed, 2 to 2.5 ms of that first draw, now as before):

| a paragraph of new glyphs, 48 px | before | now |
| --- | --- | --- |
| 200 Latin (DroidSans) | 76.5 ms (74.8 .. 84.9) | 7.7 ms (6.5 .. 9.9) |
| 200 Chinese (Microsoft YaHei) | 119.6 ms (117.3 .. 124.6) | 11.2 ms (10.3 .. 13.7) |
| 30 Latin with a style (fill, stroke outside, shadow) | 10.3 ms (9.3 .. 11.4) | 1.7 ms (1.5 .. 2.1) |
| 10 Latin with a style | 2.7 ms (2.5 .. 3.2) | 0.9 ms (0.7 .. 1.2) |
| 200 Latin, the UI thread alone (`fieldThreads` 0) | 77.2 ms (75.1 .. 83.1) | 67.7 ms (66.6 .. 72.3) |
| 200 Latin, read from the cache | | 3.9 ms (3.5 .. 5.0) |
| 200 Chinese, read from the cache | | 5.0 ms (4.6 .. 6.1) |

* The frame on Direct3D 11 (200 Latin with a style): draw 74.9 -> 7.2 ms, then 1.5 ms of submit, before and now (the
  new texels go up as a cell each).
* Drawn again, every field made: the same or less (one look-up a glyph for both passes): 0.077 -> 0.053 ms for 200
  glyphs with a fill.
* Where the time goes now, 200 Latin glyphs: their outlines on the UI thread 0.7 to 1 ms (overlapping the workers'
  start), the fields 5 to 6 ms on 16 threads (10 to 12 times one thread's), packing into the atlas 0.8 to 1 ms. 31
  workers instead of 15: 6.4 ms. A few glyphs wait for the slowest of them, 0.3 to 0.9 ms.
* The cache on Windows: a file read for the first time after it was written is scanned by the virus scanner
  (Defender), which makes the run after the one that wrote them no faster than making the fields; from the run after
  that on, the times above. A run that writes is about 1 ms slower per 200 new fields (looking for files that are not
  there).

### Not done

* Glows and shadows reach a quarter em from the field; farther they follow the glyph's box (use a glow layer).
* Per-glyph shadows of neighbouring glyphs add up where they overlap (no union of the whole text's shadow).
* Fields are not made ahead (a font's common glyphs in the background before they are drawn): a few new glyphs still
  wait for the slowest one's field (up to 0.9 ms), a paragraph of 200 for 5 to 8 ms.
* The generator on vectors (the texels of a cell share their pieces): it would have to give the same bits on every
  level, as the polylines' loops do.
* The field page is created as 16 MB of zeros on the CPU and uploaded whole (2 to 2.5 ms of the first field draw, more
  to upload); a new field's texels go up as a cell each (1.5 ms of submit for 200 on Direct3D 11).
* The cache keeps every field it is given (as the D3D shader cache, nothing is ever removed), one file each; on Windows
  each costs an open (66 us on one thread).

## Sub-pixel text

The owner asked for LCD (sub-pixel) text with FreeType's rendering, after it had been removed in core round 3
(section 0). Grayscale stays the default; `TextRendering::subpixel` (RGB / BGR) and `lcdFilter` turn it on.

### What changed

* Text: `RasterizeLcd` (glyph_raster.hpp) covers at three times the horizontal resolution and filters with FreeType's
  weights (default, light, none); A is the exact grayscale coverage. The FreeType rasterizer renders
  `FT_RENDER_MODE_LCD` with `FT_Library_SetLcdFilter`, A the stripes' mean. The atlas becomes sub-pixel pages
  (`GlyphAtlas::Reformat`, RGBA8 + `TextureFlags_LcdCoverage`), which take 8-bit coverage as gray (tiles, outlines,
  bitmap fonts); hinted loads use `FT_LOAD_TARGET_LCD`. Presets `ClearType()`; `native` on Windows reads the
  user's font smoothing (ClearType and its stripe order).
* Renderer and RHI: `ShaderProgram::TextLcd` / `TextLcdGray` (after `Clear`: the other programs keep their
  numbers), `BlendMode::DualSourceLcd`, `Caps::dualSourceBlend`, `RenderOp::lcd`, `TextComposition::clearTypeContrast`
  / `clearTypeLevel` (gText.zw), `RenderParams::subpixelText`. `TextLcdPS` composes gray texels as grayscale text
  and the others per channel; `TextLcdGrayPS` uses A.
* Backends: the dual-source code of round 3's removals is back - D3D10 / 11 / 12 blend states, GL 3.3 (core) and GLES
  (`EXT_blend_func_extended`), Vulkan (`dualSrcBlend`, `Desc::dualSrcBlend`), Metal (`Source1` factors, `index(1)`);
  D3D9 refuses it. `build_shaders.py` patches `SV_Target1` to location 0 index 1 in SPIR-V, adds the GLES extension
  and leaves `TextLcd` out of SM3.
* Conformance: the `text_lcd` scene is back with its golden from OpenGL on llvmpipe (all backends here are within
  8 of it, allowed 48) and its null log, byte for byte the old one; every other null log changes in the frame
  constants' gText.zw only (0 0 -> 0.5 1).
* Windows: `win32::MonitorText` / `FrameInfo::text` - DirectWrite's rendering parameters of the window's monitor,
  the user's font smoothing, and the monitor's orientation (`RotatedStripes`: reversed upside down, none sideways),
  re-read on monitor, display and setting changes; glass_window follows it under `--text-rendering native`.

### Verified

* Windows, clang-cl with `-Werror`, every backend (Metal's portable part included): ctest 36 / 36. Unit tests of the
  rasterizer (A = RasterizeGray's coverage, stripes by thirds, the filters keep the ink), the atlas, the text system
  (both rasterizers, RGB / BGR swap, switching kinds, the golden `ft_lcd`, the fallback = grayscale text), the frame
  planner, the renderer's program choice, the platform layer.
* `text_lcd` passes on D3D10, D3D11, D3D12, OpenGL, GLES and Vulkan (RTX 4080 SUPER); glass_window with
  `--text-rendering cleartype` on Direct3D 11: color-fringed stems on glass, blue at the left edge of light text.

### Not done

* Inside glow layers sub-pixel text stays grayscale (one alpha channel per pixel).
* Vertical stripes (VRGB / VBGR) and other panels' layouts (OLED) have no sub-pixel mode: they get grayscale.
* X11, macOS, Android: no per-monitor setup (Linux could read Xft.rgba / RandR's subpixel order).

## FreeType's rendering

`TextRendering` (`include/esia/text/text.hpp`) chooses how the FreeType text system turns outlines into pixels. The
default is unchanged - the design's outlines through the analytic rasterizer - and so is every golden image.

### What changed

* `Hinting` (None, Light, Full, Auto), `TrueTypeInterpreter` (V40, V35), `GlyphRasterizer` (Analytic, FreeType), stem
  darkening and hinted advances; the presets `Design`, `Light`, `Crisp` and `NativeTextRendering()`;
  `ParseTextRendering` for `--text-rendering` (glass_window, every platform) and `ESIA_TEXT_RENDERING` (any program).
  `FreeTypeDesc::rendering` sets it at creation, `TextSystem::SetRendering` from the next `NewFrame` on: the atlas
  starts over (`GlyphAtlas::StartOver`), and the layouts too when the advances change.
* Hinted or FreeType-rasterized glyphs load at their size; `FT_Set_Char_Size` runs (with a TrueType font's preparation
  program) only when the size changes. The interpreter and stem darkening are FreeType properties set when the
  rendering changes; the faces then request their sizes again.
* Hinted advances (`Full` / `Auto`): `FT_Get_Advance` at the frame's density, plus what HarfBuzz added (kerning,
  marks) rounded; the layout cache's key gains the density. Hinting across (v35, the autohinter) or hinted advances
  put the pens on whole pixels; otherwise hinted glyphs keep the quarter-pixel phases.
* The bundled FreeType turns on `FT_CONFIG_OPTION_SUBPIXEL_RENDERING` and adds the `autofit` and `smooth` modules.

### Verified

* `tests/text/test_ft_rendering.cpp`: the parser; the stem of an "l" in one pixel column with v35 and the autohinter
  at every phase, straddling two with v40 or unhinted; fewer partly covered pixels on horizontals (light 0.54, crisp
  0.29 times the design's with DroidSans); whole-pixel advances at densities 1, 1.5 and 2; FreeType's rasterizer
  within 2 % of the analytic one; v35, v40 and the autohinter differ; stem darkening thickens a CFF font; a new
  rendering waits for the next frame; the environment variable; the golden image `ft_crisp` (compared with the bundled
  FreeType only, `ESIA_TEST_BUNDLED_FREETYPE`).
* Windows, clang-cl with `-Werror`: 33 / 33 tests; the changed sources under the Linux CI's warnings.
* glass_window on Direct3D 11 at scale 1 with `design`, `light`, `crisp` and `native`.

### Notes

* Segoe UI's instructions were written for ClearType. Under v35 (`crisp`) they fit its stems for black-and-white
  pixels: an "m" gets narrower inside its advance and the spacing looks uneven, as GDI text without ClearType did.
  Fonts with classic hints (Tahoma, Verdana, DejaVu, Droid) suit `crisp`; Segoe UI suits `native` (v40).
* The autohinter's smooth mode gives stems the font's standard width and leans them into one pixel with a sliver
  beside it, where v35 makes them whole pixels.

## Bitmap fonts

Fonts without outlines - BDF, PCF, Windows FNT / FON (MS Sans Serif, the console fonts), bitmap-only sfnt - were
refused at load. They draw now, pixel for pixel.

### What changed

* The bundled FreeType compiles its BDF, PCF and Windows FNT drivers (`cmake/EsiaTextDeps.cmake`); zlib and LZW stay
  off, so a compressed PCF (.pcf.gz) is unpacked before loading. A system FreeType has them already.
* A bitmap face is a list of strikes; a Windows .fon's faces of one family and style (a face per size) join as one
  font's strikes, and its one character map (not marked Unicode: the font's code page) is selected.
* Sizes snap: a size takes the strike and whole multiple of it nearest in physical pixels (`Snapped`), and the face's
  metrics, units per em and HarfBuzz font become that strike's. HarfBuzz shapes it through callbacks into FreeType
  (characters, advances in the strike's pixels, its ascent and descent) on a face of its own without tables -
  HarfBuzz's empty face is inert and shapes nothing. Layouts are made again when the pixel density changes.
* Glyphs: the strike's bitmap as 8-bit coverage (monochrome: 0 or 255), each pixel repeated `times` x `times`, on
  whole pixels with no pen phases. An outline (`TextOutline`) is the pixels grown by whole pixels, a square around
  each: crisp as the font.

### Verified (2026-10-07)

* `FreeType.BitmapFontsDrawPixelForPixelAtTheirStrikes` (a BDF font made in the test: sizes 8, 11, 13, 16, the
  glyphs texel for pixel, twice the strike, an outline, twice the density),
  `FreeType.WindowsFonFilesJoinTheirSizes` (MS Sans Serif's sizes, whole pixels, the system's fallback chain beside
  it; Windows only); the showcase with `--font %WINDIR%\Fonts\sserife.fon`, with and without `--text-outline 1`,
  looked at.

## Text outlines

Outlined text and symbols, natively: `ui::ItemStyle::TextOutline(width, color)` (a widget, a container or a
`StyleScope`), `Painter::SetTextOutline` / `PainterEnv::textOutline` for custom drawing, `text::TextOutline` in the
text interface.

### What changed

* `RasterizeGrown` (`glyph_raster.hpp`): a glyph's coverage grown by a radius - a pixel is covered as much as the glyph
  covers it, or by how far its center lies within radius + 1/2 of the glyph's edge, from the exact distance to the
  flattened contours (each edge visits the pixels around it: the cost follows the perimeter). Round at the corners,
  zero borders as `RasterizeGray`'s.
* `TextSystem::DrawOutline` / `DrawGlyphOutline`: what `Draw` / `DrawGlyph` would draw, every glyph grown, in the
  outline's color - to be drawn just before them. The default draws none; the FreeType system keeps the grown glyphs
  in the gray atlas beside the glyphs (bit 31 of the key, the width in 1/8 px: up to 32 px) and their quads with the
  layout beside the text's, so an outlined label is copied in as a plain one is. Color glyphs (emoji) get none.
* `Painter::Text`, `TextBox` and `Icon` draw the outline first when the environment has one (under a scale too, at
  the scaled size); the glyphs share the outline's page, so the two are one draw command.
* `ItemStyle::TextOutline` (field `kTextOutline`): `GetPainter` gives every Painter of the scope the outline at the
  metrics scale, so every widget's labels and symbols get it.
* showcase: `--text-outline w`, `--outline-color RRGGBB[AA]`.

The text's box does not grow (as CSS's text shadows do not): the outline reaches past it by its width.

### Verified (2026-10-07)

* `Raster.GrownCoverageReachesTheRadiusRoundAtTheCorners`, `FreeType.OutlinesGrowEveryGlyphUnderTheText`,
  `Painter.TextOutlinesGoUnderTheirTextAndIcons`, `UiStyle.TextOutlineReachesTheLabelsOfItsScope`; the showcase with
  `--text-outline 1.5` (dark) and `--text-outline 2 --outline-color FFFFFF` (light), looked at.

## CPU of plain widgets

The owner's goal: plain widgets cheaper than Dear ImGui's. After flat drawing took the GPU time near Dear ImGui's,
this round cut the CPU time a frame of plain widgets takes, in both looks, without changing what is drawn.

### What changed

* Springs and timers (`ui.cpp`, `ui_internal.hpp`): `detail::AnimTable`, an open-addressing table in the Ui (at most
  half full), in place of two kinds of state in the context's per-id table. An interaction's hover and press share
  an entry (`AnimPair`), Toggle and Slider read a spring and its velocity in one look-up (`AnimWithVelocity`),
  LiquidPulse's timer and lens are one entry: the plain-widget scene looks up 267 a frame instead of 491. Entries
  not asked for in 600 frames go, checked every 64 frames.
* `InteractImpl` returns an `InteractState` (the id, the rect, the flags and the two springs) that the widgets use
  as it is; the public `InteractRect` makes an `Interaction` from it.
* `Painter`: its stacks are not initialized (`Stack<T, N>`; zeroing them was most of making a Painter, and every
  widget makes one); capsules and circles pass their radius to `EmitShape` instead of a copy of the style; flat
  drawing drops shadows, glows, glass, grain and shimmer with a flag instead of a copy of the style; `DrawPill`
  copies the style only when an item radius is set.
* Flat tiles: a trivially copyable entry, the key rounded without `lround`, its slot from four multiplies (every
  field is still compared); vertices and indices written straight into the list (`PrimReserve` with the texture,
  `PrimCommit` with the writer's command); the fill's tile is copied only when a stroke is drawn after it. Flat
  drawing pushes no clip around a shadow it does not draw.
* Layout: `Context::CurFrame` and `Scale` inline; `Snap` floors without the C runtime's call; a line's room is looked
  for among the line's own keys only.
* The FreeType text system: a 256-entry direct-mapped front for the layout cache, checked by text and parameters
  before the hash map, cleared when a layout leaves or a fallback font is added.
* Direct3D 11: `Desc::ownsContext` - a host that draws nothing else with the immediate context lets the device skip
  saving, clearing and restoring its state (about 0.004 ms a frame); the examples' Direct3D 11 host sets it.

### Measured (Direct3D 11, MSVC Release, RTX 4080 SUPER)

Esia regular, Esia flat and Dear ImGui alternating, the median of seven runs of 2100 frames each (the runs vary by
about 0.003 ms). Before: the table in "Flat drawing".

| ms per frame | build | submit | CPU | GPU |
| --- | --- | --- | --- | --- |
| plain widgets, regular | 0.061 -> 0.050 | 0.034 -> 0.029 | 0.095 -> 0.079 | 0.051 |
| plain widgets, flat | 0.070 -> 0.057 | 0.025 -> 0.015 | 0.095 -> 0.072 | 0.030 |
| Dear ImGui, the same scene | 0.040 | 0.011 | 0.051 | 0.025 |

Dear ImGui's Direct3D 11 backend always saves and restores the host's state; without `ownsContext` Esia's submit is
about 0.004 ms more.

### Not reached: 20% under Dear ImGui

(Reached on 2026-10-30: "Draw memos: widgets at rest draw nothing".)

20% under Dear ImGui's 0.051 ms is 0.041. With nothing drawn (the Painter's shapes and the text skipped: a benchmark
switch, not committed) the flat scene still takes 0.034 ms to build and 0.005 to submit: the widgets' own work -
ids, springs, styles, layout, hit records, the plan of an empty frame - leaves 0.002 ms for drawing. A flat capsule
takes about 50 ns to build against 23.5 for an FX instance (the call 14, the tile look-up 10, the grid 6, reserving
9.5, writing 9.5). Getting there needs a lighter widget layer, not more tuning: fewer look-ups per control (id,
state, springs), no styles merged in the hot path, layout and hit records in one pass.

### Verified (2026-10-06)

* `UiAnim.TableKeepsEntriesThroughGrowthAndForgetsUnused` (new), `UiAnim.SpringsStepOncePerFrameAndReportMotion`
  and the other UI tests unchanged; ctest 34 / 34 with clang-cl (every backend, `ESIA_WERROR`) and MSVC (a new tree,
  `ESIA_WERROR`); Ubuntu 24.04 (clang 18) Debug and Release, 20 / 20.

### Second round: the widgets' hot path (2026-10-07)

* A control's own springs and timers live in its interaction's table entry (`ControlSpring`, `ControlPulse`, beside
  the hover and press): a toggle looks up one entry instead of four, a slider one instead of three.
* `Painter::FillRound`: a solid rounded rectangle without a `Style` - flat drawing goes straight to its geometry.
  Toggles, sliders, buttons and pills use it wherever they draw nothing but a fill (in flat drawing the knobs too).
* Flat tiles of capsules are found by their short side and corners: their length does not change the tile (the
  corners cap at half the short side, the long axis always stretches), so a slider's fill - a length of its own on
  every slider - no longer misses the memo and asks the text system's map.
* Hit records take their region's scroll offsets from its child record (computed at `BeginChild`, fixed until the
  region ends) instead of walking its parents with a map look-up per item; `ItemAdd` and `LayOut` build their
  records once.
* The style helpers (`ResolvedStyle`, `Accent`, `FillOr`, `LookClear`, `GlassSurface` ...), `Font`, `VisibleLabel`
  and `Context::Snap` are inline; a spring at rest costs one test.

| ms per frame (alternating, median of seven) | build | submit | CPU | against Dear ImGui |
| --- | --- | --- | --- | --- |
| plain widgets, regular | 0.050 -> 0.048 | 0.029 -> 0.030 | 0.079 -> 0.077 | 1.55 -> 1.49 |
| plain widgets, flat | 0.057 -> 0.049 | 0.015 -> 0.015 | 0.072 -> 0.064 | 1.41 -> 1.23 |
| Dear ImGui, the same scene | 0.040 | 0.012 | 0.052 | |

Pixels: the workbench, regular and flat, renders the same image before and after (the showcase changes between any
two runs: its frame rate and clock-driven effects).

What is left (flat, share of the build): shapes 22% (a capsule is about 50 ns: 8 vertices and 18 indices written),
text 15% (four vertices and six indices a glyph), the core's items 12% (layout, hit records, ids), the windows 14%;
submit uploads 133 KB (32-bit indices) where Dear ImGui uploads 107 KB (16-bit). Each remaining item is a few
percent; 0.80 of Dear ImGui's CPU needs structural changes: shapes and glyphs expanded on the GPU from instances (a
few bytes each instead of 4 - 8 vertices and 6 - 18 indices, less to write and to upload) and 16-bit indices per
draw - the renderer and every backend.

### Verified (2026-10-07)

* ctest 34 / 34 with clang-cl (every backend, `ESIA_WERROR`); the image A/B above (MSVC Release, Direct3D 11).

## Flat drawing

The owner's goal: plain widgets cheaper than Dear ImGui's. Esia's draw a soft shadow under every button and knob and
every shape as a distance field; Dear ImGui's are flat triangles. Flat drawing is the option that drops what costs the
GPU, for tools where the GPU matters more than the glass.

### What changed

* `UiDesc::flat`, `Ui::SetFlat` / `GetFlat` (`ui.hpp`): every Painter the Ui makes gets `PainterEnv::flat` and
  `flatSurface` (the theme's `secondaryBackground`); windows are solid, as with `WindowFlags_Solid`.
* `Painter` (`painter.cpp`), when flat: shapes lose their shadow, glow, shimmer and grain; glass becomes a solid
  surface (`flatSurface` under its tint, the fill over it). A rounded rectangle, capsule or circle with a solid fill
  and stroke is geometry: the coverage of its corners rasterized once as a tile (`saturate(0.5 - d)` with
  `SdRoundRect`'s distance: the radius grown by the smoothing, the superellipse's exponent), stretched as a nine-patch
  along an axis with a pixel or more between its corners, whole along a shorter one; bilinear sampling fades its
  edges over a pixel wherever it lies. 1 to 9 quads a shape (a button 3, a knob 1, a window 9); the middle of a fill
  larger than 48 x 48 pixels is a quad of the white texture (the UI geometry program: the cheapest pixels), the rim
  stays on the tile. A stroke is a ring tile drawn the same way. Tiles are found again through a 128-entry table keyed
  by the shape's size, corners and stroke (in 1/64 pixel); a mask that cannot cut the shape (all of it inside, clear
  of the mask's corners) is ignored.
* `TextSystem::FindTile` / `AddTile` / `TileGeneration` (`text.hpp`): tiles in the glyph atlas under keys with the
  top bit set, gone when the pages start over (the generation changes, and differs between text systems); the
  FreeType system implements them. Shapes on the text's texture batch with the text: the plain-widget scene draws in
  8 draws (4 windows: the white middles, then the rims, shapes and text).
* `DrawList::PrimReserve` / `PrimCommit`: vertices and indices written in place.
* `examples/workbench`: `--flat`, Ctrl+Shift+F.

### Measured (Direct3D 11, MSVC Release, RTX 4080 SUPER, median of three)

| ms per frame | build | submit | GPU | draws |
| --- | --- | --- | --- | --- |
| plain widgets, regular | 0.061 | 0.034 | 0.054 | 7 |
| plain widgets, flat | 0.070 | 0.025 | 0.031 | 8 |
| Dear ImGui, the same scene | 0.042 | 0.012 | 0.025 | 8 |
| the workbench at 1600 x 1000, regular | 0.082 | 0.045 | 0.202 | 68 |
| the workbench, flat | 0.093 | 0.035 | 0.110 | 53 |

Along the way: the GPU time of flat geometry follows its vertex and index bytes (dynamic buffers are read across the
bus) - tessellated outlines with quarter-pixel arcs took 0.041 ms, one segment a corner 0.022 - which is why the
corners are tiles. The coverage program costs 0.002 ms more than the UI geometry program over 1.37 megapixels.

### Not reached

The CPU: a frame of plain widgets builds in 0.061 - 0.070 ms against Dear ImGui's 0.042, and a flat shape takes about
twice an FX instance to build (a tile look-up, a texture switch, 4 - 16 vertices). The rest of the build is the
widgets: per slider 6 spring look-ups, a Painter and three 300-byte Styles, a clip pushed around its shadow; per frame
text layout look-ups, hit records, layout. Each is a few percent: no single change halves it.

### Verified (2026-10-06)

* `Painter.FlatShapesAreGeometryThatBatchesWithText` (one command with the text, a tile per kind of shape, the white
  middle, FX for a gradient without shadow or glow, glass as its surface, tiles made again after the pages start
  over), `Painter.FlatTilesCoverAsTheShaderDoes`, `UiFlat.NoShadowsGlowsOrGlass`; ctest 34 / 34 (clang-cl, every
  backend), MSVC with `ESIA_WERROR`, Ubuntu 24.04 Debug and Release.
* The workbench flat, docked and with the Graphs panel floating, compared by eye with the regular one.

## Batching by what shadows show

The plain-widget scene (section "Frame cost against Dear ImGui": four windows of 8 rows of a label, a button, a switch
and a slider) drew in 69 draws: per row a batch of shapes and a batch of text. The planner moves a draw to an earlier
batch only across draws whose pixels it does not touch, and each button's shadow (blur 12, offset 4) reaches 25 px
around it - over the label beside it, drawn before it. So the button could not join the shapes above it, and its title
not the text above it.

### What changed (`src/esia/render/frame_plan.cpp`)

* **What a shape may not cross** (`InstanceJoinReach`): the shape, and its shadow or glow only as far as it is visible
  (> 4%, the measure capture planning uses: half the blur beyond the shape for a shadow), in the shadow's own box (its
  offset only the way it is drawn, not on every side). Past that a shadow's tail changes a pixel by a few levels; a
  label drawn before or after it shows no difference. Glass keeps its place as before.
* **What a batch touches** (`FramePlan::Region`): up to 16 rects - each draw's own, the next one along a row merged
  into the last while that adds little, merged pairwise in drawing order when full - instead of the union of the
  batch's bounds alone. A batch holding a button's title and the next row's label no longer claims the gap between
  them, where the next button lies.
* The glass-only work per FX instance (blur radius, level-0 reads) only for glass.

### Verified (2026-10-06)

* The plain-widget scene: 69 -> 7 draws. Submit time (MSVC Release, 1600 x 1000, the median of three): Direct3D 11
  0.036 ms both ways; Direct3D 12 0.054 -> 0.045; Vulkan 0.065 -> 0.034; OpenGL 0.085 -> 0.065. GPU time within the
  noise (Direct3D 12 0.051 -> 0.047).
* The showcase (`--spread` at 1.5) and the workbench, docked and with the Graphs panel: the same pixels but for the
  statistics line (214 draws instead of 226) and 2 pixels off by 1.
* `FramePlan.ShadowsBatchByWhatTheyShow`: a form's shapes batch past the labels their shadows' tails reach, a button
  right beside its label keeps its order; ctest 34 / 34 (clang-cl, every backend), MSVC with `ESIA_WERROR`, Ubuntu
  24.04 Debug and Release.

### Found on the way

* The Visual Studio generator's trees missed header changes: MSBuild's file tracker ignores files under `%TEMP%`, where
  the scratch worktree lives, so objects compiled against an old header linked with new ones (a crash in a Release
  benchmark, an unresolved symbol in another). Benchmarks now build with Ninja and MSVC's `cl` (Ninja reads
  `/showIncludes`); a Visual Studio tree under `%TEMP%` needs `--clean-first`.

### Where the plain-widget frame still goes (Direct3D 11)

Against Dear ImGui's 0.040 + 0.012 ms CPU and 0.025 ms GPU, Esia's 0.060 + 0.035 and 0.051:
* submit: the plan 0.016 ms (196 FX instances: extents, reach, joins), the upload 0.006, saving and restoring the
  host's Direct3D 11 state 0.004 (Dear ImGui's backend does the same), the draws 0.004;
* build (a sampling profile): text 15% (emitting quads, layout lookups), springs and timers 13% (a state lookup per
  spring), shapes 10% (384-byte instances), layout 8%;
* GPU: the window backgrounds through the FX shader cost 0.006 ms more than flat quads would, shadows 0.010 (their
  quads reach 1.6 x the blur plus the offset on every side), the controls' distance fields most of the rest.

## Plots

The README listed "no plots beyond a line chart" against Dear ImGui with ImPlot.

### What changed

* `src/esia/ui/plot.cpp`, declared in `ui.hpp` ([UI_WIDGETS.md](UI_WIDGETS.md) section 12): `BeginPlot` /
  `EndPlot` with `PlotLine` (an area under it, smooth or straight), `PlotScatter`, `PlotBars` (grouped, category
  labels), `PlotHistogram`; the view fitted to the data and eased, or fixed limits; ticks at 1, 2 or 5 times a power of
  ten; a legend under the axis whose entries hide their series; a crosshair and a readout of every series; drag to pan,
  the wheel to zoom around the mouse, a double click to fit again; `GetPlotLimits`. Dense sorted lines keep four points
  per pixel column. `PieChart`: a donut whose hovered segment grows and shows its value in the hole.
* The core: `ItemFlags_Wheel` - over such an item no scroll area takes the wheel, unless a turn is already latched
  to one (`UpdateWheel`).
* `Painter::Sector`: a ring's segment with flat ends and a gap of constant width, as anti-aliased geometry. No shader
  changed: the distance-field arc has round caps, which made short, thick segments look like beans.
* `examples/workbench`: a Graphs panel (a deterministic profiler capture: frame times, their spread, a frame budget,
  asset sizes by type, GPU time by draw calls) docked with the assets; `--hover X,Y` rests the mouse for pictures.
* Tests (`tests/ui/test_plot.cpp`): the ticks; the fitted view of lines, bars and histograms; fixed limits and empty or
  non-finite series; the legend hiding a series; pan, zoom and the double click; the wheel taken from the window over a
  plot and left to it with `PlotFlags_NoPanZoom`; the donut's room with any values. All under the strict debug checks.

### Verified (2026-10-06)

* Windows: clang-cl (8 backends, `ESIA_WERROR`), all 34 tests; MSVC (Visual Studio 17 2022, `ESIA_WERROR`) builds.
  Ubuntu 24.04 (clang 18): Debug and Release, 20 tests each.
* The workbench's Graphs panel on Direct3D 11, docked and floating, dark and light, with the mouse over a plot
  (`docs/images/workbench-graphs.jpg`).

### Not verified

* macOS, iOS and Android: CI builds only.

## Packaging

Esia could only be used from its source tree (`add_subdirectory`): a weakness the README listed ("no package for
`find_package`").

### What changed

* `cmake/EsiaInstall.cmake`, included last by the top-level CMakeLists.txt when `ESIA_INSTALL` is on (the default
  when Esia is the top-level project): every library target (the core, RHI, shaders, renderer, text, the FreeType
  text system, the widgets, the backends this build has, the platform layers, the bundled FreeType and HarfBuzz) is
  installed and exported as `esia::<name>` - the names of the aliases. Their include directories in the source and
  build trees are marked as the build's only there, so no backend's CMakeLists.txt changed, and every `include`
  directory among them is installed.
* `cmake/EsiaConfig.cmake.in`: the package finds again what the static libraries link against - Threads, fontconfig
  and X11 when this build used them, the system's FreeType and HarfBuzz the way `cmake/EsiaTextDeps.cmake` found
  them; a version file (same minor version: 0.x).
* `tests/package`: an application of its own (`find_package(Esia)`, a window with a button rendered on the null
  device, the FreeType text system, the compact density). The `esia_package` test installs the build into a
  prefix, configures it with the same generator and compilers (and what found the dependencies: vcpkg's toolchain and
  triplet in CI), builds and runs it.

### Verified (2026-10-06)

* `esia_package` on Windows with clang-cl (Ninja, the bundled FreeType and HarfBuzz) and with MSVC (Visual Studio
  17 2022, `ESIA_WERROR`), and on Ubuntu 24.04 (clang 18, the system's FreeType 2.13.2 and HarfBuzz): the
  application builds and renders, with 8 backends on Windows and 4 on Linux.

### Not verified

* macOS and the vcpkg packages of Windows CI: CI only.

### Found by CI (2026-10-06)

* `esia_package` failed in both Windows jobs: vcpkg's toolchain adds its own directories to `CMAKE_PREFIX_PATH`,
  the test forwarded them, and a second `-DCMAKE_PREFIX_PATH` replaced the one with the install prefix, so the
  application did not find Esia. `run_package_test.cmake` now passes one list, the prefix first; reproduced and
  checked locally with a build configured with two prefix paths.
* That fixed the clang-cl job; the MSVC one still failed: Visual Studio generates every configuration of the
  application's project, the test installs one (Debug), and vcpkg's toolchain maps MinSizeRel and RelWithDebInfo to
  Release and then to no configuration, which `esia::ui` does not have ("IMPORTED_LOCATION not set ... MinSizeRel"); a
  single-config generator (Ninja) generates only Debug. The application's project now gets
  `CMAKE_CONFIGURATION_TYPES` set to the configuration installed. Reproduced and checked locally with the CI job's
  configuration: Visual Studio 2022, vcpkg's FreeType and HarfBuzz (x64-windows), Debug.

## Density

The widgets' sizes are iOS's (44-unit rows, 34-unit controls, 15-unit text): roomy on a phone, sparse in a desktop
tool. `ThemeCompact` scaled everything to 0.87, text included - what `metrics.scale` already does, and barely denser.

### How others do it

* WinUI: `Compact.xaml`, a resource dictionary of smaller control sizes (`TextControlThemeMinHeight` 24 for 32,
  `ListViewItemMinHeight` 32 for 40, `TreeViewItemMinHeight` 24 for 28, `ComboBoxMinHeight` 24, smaller paddings);
  the font stays 14. One line puts it on a page.
* Material: a density scale per component (0 to -3), 4 units of height per step (a button 36 -> 24 at -3); the
  typography stays.
* Apple: control sizes (regular, small, mini; SwiftUI's `controlSize`): the control and its label shrink together.

Esia takes the first two's model: the text keeps its size, every control, row and gap gets a smaller size from a
table, not a factor. Apple's way was tried for the controls (their labels 13 for 15): beside 15-unit row labels the
smaller labels inside them looked patchy, so every text stays as it is. What the glass adds is taken down with the
control: its shadows, insets, the knob's swell and the lens's refraction (x 0.8).

### What changed

* `Density` (`Regular`, `Compact`), `UiDesc::density`, `Ui::SetDensity` / `GetDensity`; `Metrics::compact`, the
  Ui's: `SetTheme` and `SetDarkMode` keep it, a theme transition blends it (the switch animates).
* `detail::ControlSizes`: every built-in control's size in one table per density (buttons, fields, text fields,
  menu, table and tree rows, switches, check boxes, sliders, steppers, the color picker's bars, the bars, a window
  header's close button and icon), blended by `metrics.compact`. The Regular table is the sizes the code had, so
  the regular look is unchanged pixel for pixel.
* `DensityMetrics`: rows, headers, icon tiles, padding, spacing, section spacing and corners as fractions of the
  theme's; the scale and the type are left alone.
* A control's details (`Dt`: shadows, insets, the knob's swell) follow the table's `detail`, and its glass refracts in
  proportion (`materials.control`, the lens); bars keep their glass.
* The showcase and the workbench: `--compact`, and Ctrl+Shift+D (Cmd+Shift+D) switches the density while running.
* **Under a floating bar the content fades out** (iOS 26's scroll edge effect; the tab bar, the search bar): rows that
  scrolled under the bar showed through the upper half of its glass at full strength - the area's bottom fade is 30
  units, the bar and its margin twice that - and ran into the search field's placeholder (worst in the compact
  density, where a row lines up with the field). The bar moves its area's fade, begun earlier in the frame
  (`DrawList::Fades()` is writable until the frame renders): it ends at the bar's middle and starts 16 units above
  it, as much as there is left to scroll. The shaders are unchanged (the fade already reaches 0 past its edge).
  `BeginScrollEdgeFade` / `EndScrollEdgeFade` keep a stack of the areas being submitted for it.

### Verified (2026-10-06)

* ctest 33 / 33 in Debug (clang-cl 22); new: `UiSearchBar.TheContentFadesOutUnderIt` (the fade ends at the bar's
  middle while there is scrolling left, nothing fades at the end), `UiDensity.CompactKeepsTheTextAndShrinksTheControls` (the regular sizes
  exactly, the compact ones from the table, the text 15 in both),
  `UiTheme.DarkModeKeepsTheMetrics` (the density survives `SetTheme` and `SetDarkMode`).
* The workbench in the regular density, captured as the README's picture and encoded the same way: the same JPEG,
  byte for byte. The showcase's Components and Settings panels and the workbench in both densities, side by side:
  the compact ones show more (the Components panel two more of its sections), the controls in their regular
  proportions.
* The README's Windows showcase pictures taken again (the Settings and Components panels' bars).

### Not verified

* Linux, macOS, iOS and Android: CI only. The compact density on a phone (it is meant for a desktop tool).
* The iOS and Android pictures predate the fade under the bars (their Settings screens have the search bar).

## Debug checks, desktop defaults, the README audited

What an application built on the widgets ran into, again: a button that vanished (laid out past its window's edge),
two tables that shared their scroll state (an id conflict), defaults made for a phone, options whose rules were only
in the headers. The causes of the first two were fixed on 2026-10-05; nothing told the developer what was wrong.

### What changed

* **Debug checks** ([UI_CORE.md](UI_CORE.md) section 16; `ContextDesc::debugChecks`, `diagnostics`, `Diagnostic`):
  two items of a window with one id in a frame (from the hit lists; `HitRecord::item` tells an item's own records
  from the next item with its id, which `RecordHit` used to fold into one), two child regions begun with one id, and
  an item wholly past the edge of its layout region and of the clip, on an axis nothing scrolls, for a second. Each is
  reported once with the window, the id and its label (`GetId` keeps the frame's labels while the checks are on), to
  a callback or stderr and the debugger, and outlined in red in the foreground list while it lasts. `Auto` turns them
  on in debug builds; the release benchmarks run without them.
* **The UI tests run the checks in every build** and fail on any report (the widgets themselves trip none: every UI
  and data-widget test, the showcase, the workbench and glass_window at many sizes and layouts in a debug build).
* **`ui::IdScope`** for widgets made in a loop; `Context::PushId("literal")` hashes the label (a literal took the
  `const void*` overload and pushed its address); `PushId` / `GetId` take any integer type.
* **Sections keep apart**: a section is its header's id scope, and sections without a header or with the same one in
  a scope are told apart by their order. Two headerless sections shared their card's state (the mask of its rows).
* **Tables**: `maxHeight` with a `height` caps it (it was ignored); the header comment no longer says `TableVisible`
  is required (without it a table takes last frame's row count).
* **Desktop defaults.** A mouse press on a scroll area's empty space no longer drags the content (a finger's still
  does; `InputConfig::mouseDragScrolls` for the old way): on a desktop it moves the window, as in Dear ImGui. `metrics.scrollIndicatorAlways` is 1 in the built-in themes on Windows, Linux and macOS (0 on iOS and Android)
  and the indicators hide at rest after a finger's press; the showcase keeps WGT's hidden indicators.
  `metrics.controlHeight` is documented as what it is: for custom widgets (the built-in controls do not read it).
  `ThemeCompact`, added with these changes, scaled everything at once (as `metrics.scale` already does); the same
  day it became `Density` (below).
* **The Control Center modules shrank** (since `e8c4ff4`, 2026-10-05): the groups of an auto-layout container had one
  layout sequence (their parent's lines do not move under a provider), so one module's `SameLine` buttons gave every
  module their room, and a module's own buttons, placed with `SetCursorPos`, gave it theirs. A frame's sequence is now
  its place among its parent's items, and `SetCursorPos` starts a line. Found by comparing the README pictures taken
  again with the old ones.
* **The README audited** against the code and the other documents: the options (`--font`, `--spread`, the showcase's
  and workbench's lists), the bundle ids and packages, what each preset builds and tests, what ran under Wine, iOS
  tests, the threads row, measured numbers dated or measured again, a section on using Esia in a project. The Windows
  pictures were taken again (scroll indicators at rest in the workbench; the showcase at 0.74 ms of GPU time, was
  0.86). Stale statements in CI.md, REWRITE.md, `freetype.hpp` and the backends' STATUS files corrected or dated;
  the branch READMEs list the workbench.

### Verified (2026-10-06)

* ctest 33 / 33 in Debug (clang-cl 22, every backend); new: `DebugChecks.TwoItemsWithOneIdAreReportedOnce`,
  `TwoChildRegionsWithOneIdAreReported`, `AnItemLaidOutOutOfViewIsReported`, `OffChecksNothing`,
  `Context.PushIdTakesLiteralsAsLabels`, `Layout.LineRoomKeepsToItsGroup` (it failed before the fix),
  `UiLists.SectionsWithOneHeaderKeepTheirRowsApart`, `UiIds.IdScopeKeepsALoopsWidgetsApart`,
  `UiTable.MaxHeightCapsAHeight`; `UiScroll.DragTheContentOrTheIndicator` with a mouse, a finger and
  `mouseDragScrolls`.
* The examples in a debug build (Direct3D 11): showcase (every panel, spread, each Components tab, a Settings page, a
  phone's size), workbench (docked, floating, compact, narrow), glass_window, also at sizes too small for them: no
  report. Two buttons with one label put in the workbench for the test: reported once, both outlined. The default
  sink (stderr) checked by hand with a temporary test.
* The showcase and workbench captures against the README's old pictures: the same pixels but for the moving content,
  the status bar and the workbench's indicators, once the Control Center was fixed.

### Not verified

* Linux, macOS, iOS and Android after these changes: CI only.
* A real touch screen on Windows (the indicators hiding after a finger's press, the drag).

## Text against Dear ImGui

The same setup as below (section "Frame cost against Dear ImGui"), with two corrections to the plain-widget scene:
the Esia side drew its paragraph in a transparent color (it was left out), and Dear ImGui's Segoe UI was 15 px of
its height, which is an 11 px em - it is 20 px now, the 15 px em of Esia's body text. Both read their GPU timestamps
inside the submit. Text: four windows of 30 lines (`"Line %d: The quick brown fox jumps"`, about 3300 glyphs a frame),
with solid windows and without any window background.

| Direct3D 11, ms per frame | Esia build + submit | Dear ImGui build + submit | CPU | GPU Esia / Dear ImGui |
| --- | --- | --- | --- | --- |
| text only | 0.042 + 0.025 = 0.068 | 0.055 + 0.021 = 0.076 | -11% | 0.033 / 0.032 |
| text in solid windows | 0.049 + 0.028 = 0.077 | 0.055 + 0.022 = 0.077 | the same | 0.045 / 0.038 |
| plain widgets | 0.060 + 0.033 = 0.092 | 0.040 + 0.012 = 0.052 | +77% | 0.051 / 0.024 |

Before these changes text alone took Esia 0.083 + 0.047 ms (Dear ImGui 0.050 + 0.017). The glass scene against WGT's
demo now: Direct3D 11 CPU 0.199 ms against 0.367 (-46%), GPU 0.401 against 0.440 (-9%); Direct3D 12 CPU 0.325
against 0.477 (-32%), GPU 0.437 against 0.452 (-3%).

### What changed

* **A text's glyph quads are kept** (`ft_text_system.cpp`, `Layout::quads`): placed once in physical pixels from the
  whole pixel at the text's origin, by the origin's sub-pixel offsets, and copied in at each draw while the atlases,
  the em size, the scale, those offsets and the colors stay the same - the next frame, or the same string in another
  row or window. A text inside its clip goes in as a block through local cursors (stores through the writer's members
  kept the compiler from holding them in registers). Placing from the origin's whole pixel instead of the absolute
  position gives the same pixels at 100%, 125% and 150% (the text and plain-widget scenes compared).
* **Bounds kept as the lists are written** (`DrawCmd::vtxFirst`, `vtxEnd`, `vtxBounds`): every way of writing
  vertices updates its command's, `Painter::PopScale` refreshes them after rewriting positions
  (`DrawList::RefreshBounds`). The planner scanned every geometry command's vertices again (a third of the submit of
  a frame of text).
* **Lists without zeroing** (`UninitAllocator`, `VertexVector`, `IndexVector`): a run of quads is reserved and written
  in place, and `std::vector`'s resize zeroed it first.
* **Each list's geometry uploaded as it is** (`rhi::Caps::baseVertex`, `Device::DrawIndexedBase`, `Device::MapBuffer`
  / `UnmapBuffer`, [backends/README.md](backends/README.md) sections 2 and 3): a direct plan
  (`FramePlan::Build(..., direct)`) leaves the lists' vertices and indices where they are; the renderer writes them
  into the mapped buffers at each list's offsets and draws every run with its list's first vertex as base vertex. FX
  instances go from the lists into the mapped buffer too, the integer row converted on the way (they were copied
  three times). Direct3D 9 - 12 map their buffers; Direct3D 9 - 12, Vulkan and desktop OpenGL draw with a base
  vertex; GLES 3.0 and Metal keep the merged, rebased buffers (`MapBuffer`'s default stages for `UpdateBuffer`).
* **The common label formats written without vsnprintf** (`ui::detail::FormatV`): `%d %i %u %x %X %s %c %%` with
  `l`, `ll`, `z`, no flags, width or precision; anything else goes to vsnprintf. Microsoft's vsnprintf takes its locale
  lock first: a sixth of building a frame of labels.

### Verified (2026-10-05, later)

* ctest 33 / 33 in Debug (clang-cl 22, every backend); new: `DrawList.CommandsKeepTheirVertexBounds`,
  `FramePlan.DirectPlansDrawTheListsAsTheyAre` (the same indices and instances as the rebased plan's),
  `Renderer.BaseVertexUploadsTheListsAsTheyAre`, `UiText.FormattingIsVsnprintfs`.
* `esia_conformance --strict`, one frame and `--frames 3`, on Direct3D 9, 10, 11, 12, OpenGL, GLES, Vulkan and null:
  17 / 17 each; the null command logs are the goldens unchanged. The showcase with `--debug` on all six APIs: no
  validation or debug-layer messages.
* The text, text-in-windows and plain scenes render pixel for pixel as before on Direct3D 11 at 100%, 125% and 150%,
  and on Direct3D 12 the same with the base-vertex path on and off.
* The changed portable sources pass the Linux / macOS warning flags.

* Linux (the Ubuntu 24.04 VM with the CI's packages, clang 18, llvmpipe / lavapipe): `linux-clang` and
  `linux-clang-release` with `ESIA_WERROR`, ctest 22 / 22 each (the two Metal conformance runs skip). It found a
  fault of the previous batch, which failed the Linux CI there: `AddFallback` dropped the cached layouts but not the
  pointer to the last one looked up, so the next measure of that string read a freed layout (glibc left it as it
  was: the text measured as before the fallback; Microsoft's debug heap overwrites freed memory, so the Windows
  tests passed). `AddFallback` forgets it now.

### Not verified

* Metal (it keeps the merged buffers: `MapBuffer`'s default, no base vertex) and GLES 3.2's base vertex (unused).
* macOS, iOS and Android after these changes: CI only.

## Frame cost against Dear ImGui

Measured on Windows 11 with an RTX 4080 SUPER, MSVC 19.44 Release builds of both sides, 1600 x 1000 pixels, vsync off,
1800 frames after 300 of warm-up, the median of three alternating runs. Build: the UI from `NewFrame` to the draw
data; submit: the renderer (or `RenderDrawData`) up to the present; GPU: the frame's timestamps.

**The glass scene**: the showcase and WGT's demo with the same six panels open (`--open settings,effects,control,
components,languages,telemetry`), WGT's `--text gray`.

| ms per frame | WGT CPU | Esia CPU | | WGT GPU | Esia GPU | |
| --- | --- | --- | --- | --- | --- | --- |
| Direct3D 11 | 0.373 | 0.232 | -38% | 0.440 | 0.398 | -10% |
| Direct3D 12 | 0.453 | 0.338 | -25% | 0.410 | 0.406 | -1% |

The GPU time by category (Direct3D 11, every scope timed): backdrop captures WGT 0.229, Esia 0.166 (13 captures a
frame against 19: glass sees what lies under its shape, not the whole blur footprint); the glass shading 0.170 and
0.172 (the same shader); everything else 0.042 and 0.045. A capture's cost is mostly fixed (a copy and four pyramid
passes, each waiting for the one before): what is left to gain there means fewer captures or fewer levels, which the
glass would show, or the pyramid in one compute pass.

**Plain widgets**: four solid windows without shadows, 8 rows of a label, a button, a toggle and a slider, and a
wrapped paragraph each, against vanilla Dear ImGui 1.92.9 (WGT's patch reversed) drawing the same with its standard
widgets and Segoe UI at 15 px, on the same adapter, flip-model swap chain and timestamps.

| Direct3D 11, ms per frame | build | submit | GPU | draws |
| --- | --- | --- | --- | --- |
| Dear ImGui | 0.041 | 0.010 | 0.024 | 8 |
| Esia before the first six changes below | 0.069 | 0.046 | 0.057 | 172 |
| Esia now | 0.060 | 0.037 | 0.046 | 66 |

Esia draws more than Dear ImGui there: a soft shadow under every button and knob, antialiased distance-field shapes,
springs behind every control's states, text shaped by HarfBuzz; Dear ImGui draws flat rectangles. Its submit is a
memcpy of the draw lists and 8 draws; Esia's plans batches across the lists (15 us), copies its 384-byte FX
instances three times on their way to the GPU (6 us for the last two) and times the frame on the GPU (5 us of
Direct3D 11 query calls).

### What changed

* **Clips that cut nothing do not split batches** (`frame_plan.cpp`). Controls push a clip of their own around their
  shadow (`ScopedUnclip`) between labels clipped by the window's body, so every button, toggle knob and slider knob
  was a batch and a draw. A draw its clip does not cut (all of it inside) looks the same under any scissor that holds
  it whole: it joins batches of other clips, and such a batch keeps a free clip (`RenderOp::clipFree`) until a draw its
  clip does cut fixes the batch's scissor. Planned, a free batch takes the scissor before it when that holds it, else
  the whole target, so no scissor changes are added. The plain scene: 172 -> 66 draws. The 17 conformance scenes
  render identically on Direct3D 11 and 12 (compared pixel for pixel) and the null backend's command logs are the
  goldens unchanged.
* **The FX shader's solid area** (`esia_fx.hlsl`, [backends/README.md](backends/README.md) section 4): the interior
  of a solid rounded rectangle is its fill color from a few varyings, without the distance field - bit for bit what
  the full shader computes there. Direct3D 10 - 12 for now: the generated library keeps the previous shader until its
  next regeneration.
* **Per-id state in a flat table** (`StateStorage`, [UI_CORE.md](UI_CORE.md) section 10): one probe instead of a
  node-based hash map; a spring at rest returns without stepping (`SpringState::Step`: exp, sin and cos for nothing).
* **Glyph runs** (`DrawList::BeginQuads` / `EndQuads`): a text's glyphs are written in place after one reservation,
  the same vertices, indices and commands as one `AddRectFilledUV` each.
* **The planner's loops**: a geometry command's bounds from the range of its indices, then its vertices once each
  (a glyph's six indices name four vertices); rebased indices written in place.
* **Timestamp reads** (Direct3D 10 / 11): `ReadProfile` stops at the first frame not done yet and skips the frame
  just ended (not submitted yet); each look is a driver call.
* Also in this batch: Direct3D 11 draws FX batches with `DrawInstancedFrom` (`Caps::drawFirstInstance`,
  [backends/README.md](backends/README.md) section 3), so the Draw constants no longer change per batch; Direct3D 12
  skips unchanged root signatures, pipelines, topologies and descriptor tables and copies a table in one call; the
  end-of-frame flush of Direct3D 11 (0.18 ms of CPU) only when timestamps are still pending a full ring later;
  integer keys hashed by `esia::IntHash` instead of MSVC's byte-wise FNV; the text layout of the string just measured
  found without hashing, glyph slots reused while the atlas, size and sub-pixel phase stay; a draw list grows
  geometrically and writes a quad in place; `ui::detail::M()` is inline (a thread-local pointer); the straight-line
  backdrop sampling on shader models without SM3's slot limit (the compact forms cost the glass ~40% more on an RTX
  4080); glass content capture decided by the glass shape (+2 px), not its blur footprint (20 -> 13 captures in the
  showcase, nothing visible changed).

### Verified (2026-10-05)

* ctest 33 / 33 in Debug (clang-cl 22, every backend): `esia_core_tests` 107 / 107, `esia_render_tests` 50 / 50,
  `esia_ui_tests` 46 / 46; new: `Context.StateStorageTable` (3000 entries of two types, a third collected from the
  middle of probe chains, the rest found at their addresses), `DrawList.QuadRunsMatchSingleQuads`,
  `FramePlan.ClipsThatCutNothingDoNotSplitBatches` (and `FxInstancesBatchUntilTheStateChanges` with a clip that cuts).
* `esia_conformance --strict`, one frame and `--frames 3`, on Direct3D 9, 10, 11, 12, OpenGL, GLES, Vulkan and null:
  17 / 17 each. The showcase with `--debug` on all six APIs: no validation or debug-layer messages.
* The conformance renders on Direct3D 11 and 12 with the new planner and shader against the same build with both
  turned off: identical, all 17 scenes; the plain-widget scene identical before and after.
* `build_shaders.py` with the Vulkan SDK's tools on the sources before and after: the same library, byte for byte.
* The changed portable sources pass the Linux / macOS warning flags (`-Wpedantic -Wshadow -Wnon-virtual-dtor`).

### Not verified

* Linux, macOS, iOS and Android after these changes: CI only. The generated library is unchanged, so GL, GLES,
  Vulkan and Metal draw with the shaders they had.

## Scrolling, menus and tables in use

An application built on the widgets reported these, with minimal fixes of its own; they are fixed here at their
causes.

### What changed

* **Tables are id scopes** (`BeginTable` pushes the table's id until `EndTable`). Two tables with a `height` in one
  window had the same id for their rows' scroll area, so one scroll state: the table with few rows wrote a range of
  0, the wheel over the other could not move it and scrolled the window. Widgets in cells could share ids across
  tables too. The multi-line editor's lines had the same problem and the same fix.
* **Long menus scroll.** A glass popup's rows past `maxHeight` (by default what fits on the display) scroll inside it
  with the edge fades and the indicator. `Picker` shows `PickerOptions::maxRows` choices (10) and opens with the
  current one in the middle; `MenuOptions::maxHeight`. Fifty choices ran from the top of the screen to past its
  bottom.
* **Auto-sized windows on whole pixels** ([UI_CORE.md](UI_CORE.md) section 7). A menu anchored at its right edge
  between two pixels crept by a fraction of a pixel every frame and jumped back: what its content measured depended on
  where it started between pixels (layout positions snap down), and where it started on its size. Its size is now
  rounded up to whole physical pixels and its place down onto them. A menu row spans what its popup (or scroll area)
  offers now instead of last frame's popup width.
* **The scroll indicator stays where its area shows**: clipped to `Context::VisibleViewRect()`, the part of the area
  inside its parents' clips. A table its window scrolled half out of view drew its indicator, and took clicks on it,
  below the window.
* **`SetDarkMode` keeps the metrics**, the type and the motion: it replaced the whole theme, so the UI scale went back
  to 1 on a switch to dark or light.
* **Indicators that show they scroll**: `Theme::metrics.scrollIndicatorAlways` (a float, so theme transitions blend
  it; 1 = on) keeps the indicator of every area that scrolls, dimmed at rest, on a faint track - for desktop apps.
  By default an area that appears with more than it shows, or grows past it, flashes its indicator, as iOS does.
* **The wheel stays with the area it scrolls** while it turns (0.3 s, the mouse still, per axis), as in a browser:
  the window scrolling a table in under the mouse no longer hands the table the rest of the turn.
* **A field that fills the width leaves room for what follows on its line.** `NumberField` / `VectorField` (and every
  widget taking `AvailableWidth()`) took the whole width, so a `SameLine` button after it was laid out outside the
  clip, unseen and unclickable. The core now records how far each line reached past its items
  (`Context::LineRoom()`), and `AvailableWidth()` leaves that much.
* **Table selection by key**: `TableOptions::selectedKeys` with `rowKey(i)` keeps the selection as the rows' keys, so
  it follows the items when the rows are sorted, filtered or refreshed; Shift selects from where the anchor's row is
  now. Index selection (`selection`) is unchanged.
* **`TableOptions::maxHeight`**: as tall as the rows up to it, then they scroll. The height comes from the count given
  to `TableVisible`, in the same frame (a table of one row no longer keeps 330 units of empty rows).
* `MeasureText` on unchanged strings was reported as a cost: the text system already keeps laid-out text per font,
  size, wrap width, flags and string, so a repeated measure is a hash lookup without an allocation.

Then (the same day):

* **The resize grip** (`WindowOptions::resizeGrip`, set by `ui::BeginWindow` to the square where the arc shows): a
  window's bottom-right corner resizes it both ways and is above the items there. The arc is drawn along the rounded
  corner, well inside the 5-unit edge bands that resized it, so the arc could not be dragged; and a window scrolled to
  its end had its indicator in that corner, which took the press.
* **The window's indicator stops above its rounded corner**: in the window's lane the track ends a corner radius above
  the window's bottom (`ScrollEnd`'s `cornerRadius`). It ran down into the corner's curve, outside the window's shape.
* **No allocation per frame in the render submit.** Counted on the showcase (1600 x 1000, every panel, the island
  live), operator new per frame after 150 frames, before -> after:

  | | Direct3D 12 | Direct3D 11 | Vulkan | OpenGL |
  | --- | --- | --- | --- | --- |
  | `Renderer::Render` | 88 -> 0.08 | 0.01 | 207 -> 0.01 | 2 -> 0 |
  | building the UI (`Demo::Frame`, `EndFrame`) | 61 -> 5.4 | 4.8 | 5.3 | 5.9 |

  What allocated: the capture planner's dirty-rectangle lists (two per glass batch, now in place), a layer stack in
  `FramePlan::Build`, Vulkan's barrier batches (in place up to eight images), the timestamp read-backs of Vulkan and
  OpenGL and OpenGL's row repacking (kept buffers); in the UI the list of draw lists and of windows in draw order
  (`Context::EndFrame`), the auto-layout containers' child lists (moved away each frame, now swapped) and their
  grid / flow scratch, a section footer's copy, the island's copy of the activities. The rest is the text layouts of
  strings never seen before (a frame rate, a timer): a new layout reserves its glyphs at once, and once the cache is
  full, evicted layouts take new strings with their storage. What remains in `Render` is a glyph page upload.
  `Renderer.SteadyFramesAllocateNothing` counts it on the null device (which formats its log only when it records).
* A `-Werror` release build on Windows failed: `gl_headless.cpp` declared the WGL debug flag and used it only in debug
  builds.

### Verified (2026-10-05)

* Windows 11, clang-cl 22 with `ESIA_WERROR=ON`, every backend: ctest 33 / 33 (`esia_core_tests` 104 / 104,
  `esia_ui_tests` 45 / 45). New tests: `Context.AutoSizedPopupAtItsRightEdgeStaysPut`,
  `Child.WheelStaysWithTheAreaItScrolls`, `Child.VisibleViewRectStopsAtTheParent`, `Layout.LineRoomOfSameLineItems`,
  `UiTable.TwoTablesScrollOnTheirOwn`, `UiTable.MaxHeightFitsTheRows`, `UiTable.SelectionByKeyFollowsTheRows`,
  `UiTable.TheIndicatorStaysInsideTheWindow`, `UiLayout.AFieldThatFillsLeavesRoomForTheItemsAfterIt`,
  `UiPopups.ALongPickerScrollsOpenedAtTheChoice`, `UiTheme.DarkModeKeepsTheMetrics`; `Child.NestedScrollAreasAndWheel`
  now pauses before the parent takes the wheel. Each new test fails with its fix taken out (the popup one: its edge
  went through four places in turn).
* Then: ctest 33 / 33 in Debug, a RelWithDebInfo build of everything with `-Werror`; `esia_core_tests` 105 / 105,
  `esia_ui_tests` 46 / 46, `esia_render_tests` 49 / 49 (new: `Context.ResizeGripIsAboveTheItemsAtTheCorner`,
  `UiWindow.TheGripResizesAWindowScrolledToItsEnd`, `Renderer.SteadyFramesAllocateNothing`, each failing without its
  fix). The allocation counts above come from a temporary probe (operator new with stack traces) in the showcase.
* The changed sources pass the Linux / macOS warning flags (`-Wpedantic -Wshadow -Wnon-virtual-dtor`, clang 22).

### Not verified

* The application's own scenes with these changes (only the tests and the examples here).
* Linux, macOS, iOS and Android after these changes: CI only.

## Data widgets, Android, color glyphs, touch

### What changed

* **Data widgets** (`src/esia/ui/number.cpp`, `color.cpp`, `table.cpp`, `text_editor.cpp`, `dock.cpp`;
  [UI_WIDGETS.md](UI_WIDGETS.md) section 12): `NumberField` (scrub, type an expression, step), `VectorField`,
  `ColorPicker` and `ColorWell`, tables (only the rows in view submitted; sort, resize, select), `TreeNode`,
  `TextEditor` (paragraph layout cached by content, wrapping, line numbers), and docking (a node tree per dock space,
  docked windows behind floating ones under a glass tab bar, drag a header in, drag a tab out, splitters, the layout
  as text). The core gained `Context::StartWindowMove` (a tab dragged out keeps moving as a window) and
  `MovingWindow`. `examples/workbench` puts them in one dock space.
* **X11 as a library**: `esia_platform_x11` (`src/esia/platform/x11`, `include/esia/platform/x11.hpp`), taken out
  of the examples' frame, which now uses it.
* **Android**: `esia_platform_android` (`src/esia/platform/android`): a NativeActivity's touches (the first finger is
  the mouse, two fingers scroll), keys with the text they type (`KeyEvent.getUnicodeChar` through JNI), the soft
  keyboard while a field edits, the clipboard (`ClipboardManager`), the density as the UI scale. The examples' frame
  (`app_android.cpp`) runs the example's `main` as `glass_main` and keeps the device when the window goes (the app in
  the background). Hosts: `host_opengl_egl.cpp` with OpenGL ES 3.0, `host_vulkan.cpp` with `VK_KHR_android_surface`.
  `glass_add_app` builds each example as a shared library and an APK (`tools/android/package.py`: aapt2, zipalign,
  apksigner, no Gradle) with Material Icons Outlined for the icons (`icons_material.cpp`, `icon_font_text.hpp`; the
  font is downloaded at configure time, SHA-256 checked). Preset `android`; the `opengl` and `vulkan` branches carry it.
* **Color glyphs**: COLR fonts painted (`src/esia/text/ft/colr.cpp`: version 0 layers; version 1 paints with glyph
  clips, solid, linear, radial and sweep gradients, transforms and every composite mode). Windows' Segoe UI Emoji and
  Android's Noto Color Emoji are COLR version 1 and were drawn as outlines. PNG strikes FreeType cannot decode (the
  bundled FreeType has no libpng) are read by `png.cpp` (every color type and depth, palettes with tRNS) from
  HarfBuzz. Fonts without outlines take their units per em from `head` (FreeType reports 0: CBDT emoji were laid
  out in whole ems).
* **Fallback chain** on Linux and Android: the Noto fonts of Arabic, Hebrew, Armenian, Georgian, Bengali, Tamil,
  Telugu, Kannada, Malayalam, Gujarati, Gurmukhi, Sinhala, Khmer, Lao, Myanmar and Ethiopic, and Noto Color Emoji
  Flags (Android drew tofu for all of them).
* **Touch** ([UI_CORE.md](UI_CORE.md) section 5): presses marked as a finger's (`InputEvent::touch`) may become a
  scroll. A button pressing on release yields to a scroll area dragged along past `touchSlop`; an item acting on the
  press does not get a finger's press until it lifts, rests or moves across. The Win32 layer marks touch-screen
  mouse messages.
* **Scrolling**: layout offsets land on whole pixels (`Context::SnapScroll`). A drag held past the end of an area
  shook its content by a pixel every few frames: the offset between pixels changed the measured content and so the
  range that clamped it.
* **OpenGL ES devices**: the headless EGL context asks for debug output through `EGL_KHR_create_context` (EGL 1.4),
  `glGetError` errors beyond the debug callback's are counted; the OpenGL tests expect the caps of the extensions a
  device has, Vulkan's older-API test no longer needs dynamic rendering.
* `SymbolTextSystem` (macOS, iOS, Linux frames) forwards `CaretStops`: fields there stopped at every code point.
* **The safe area on Android** (after the core's `FrameParams::safeArea`, made for the iPhone): the platform layer
  reads the window's insets - `WindowInsets.Type.systemBars() | displayCutout()` (API 30), else the system window
  insets and the cutout's safe insets - every 30 frames and when the window or the configuration changes, and passes
  them on (`FrameInfo::safeLeft` ...). The island sits around the camera there as on an iPhone, the panels below it,
  the dock above the gesture bar. The showcase's wallpaper on Android is the building machine's, packed into the APK
  and decoded by `BitmapFactory` (`glass::Wallpaper`), else Android's built-in one (`WallpaperManager`).
* **Masks and scales reach `FillRect`**: a square `Painter::FillRect` was indexed geometry, which `PushMask` and
  `PushScale` do not reach, so the color picker's checkerboards and bar ends kept square corners outside their
  capsules, and a number field's fill outside its rounded well. Under a mask or a scale it is an SDF instance now.
* **The color picker's edges**: shapes stacked on one rounded edge each anti-aliased it, and the lower ones showed as
  a rim. The spectrum is one textured shape (a 2 x 2 texture - white, the hue; black, black - whose bilinear filtering
  is exactly the saturation / brightness plane, kept in the picker's state); the checkerboards are masked a physical
  pixel inside the edge the color draws, their cells and the hue bar's segments meet on whole pixels, and the
  spectrum, the bars and the swatch have no outline (it read as a light ring on dark backgrounds).

### Verified (2026-10-02)

* Windows 11, RTX 4080 SUPER, MSVC 19.44: `esia_core_tests` 99 / 99, `esia_ui_tests` 37 / 37, `esia_text_ft_tests`
  31 / 31 (Segoe UI Emoji's rocket: 262 colors in its RGBA page), `esia_render_tests` 48 / 48; after the `FillRect`
  change the conformance suite on Direct3D 11, Direct3D 12, OpenGL and Vulkan: 17 / 17 each, goldens unchanged.
  `workbench` and `showcase` on Direct3D 11 and 12; the color picker zoomed at UI scales 1 and 1.5 before and after.
  The touch tests and the still-at-the-end test fail with their fixes taken out.
* The README's pictures were taken again with these changes: Windows (Direct3D 11), Ubuntu (OpenGL), the Android
  emulator (OpenGL ES; as the iOS ones, one screenshot per screen taken on the device: the island around the camera,
  the panels below it, the building PC's wallpaper) and the workbench.
* Ubuntu 24.04 in VMware, clang 18, Debug with `ESIA_WERROR=ON`: ctest 19 / 19; `showcase` and `workbench` on the X11
  layer.
* Android 15 emulator (x86_64, the host's RTX 4080 SUPER through the emulator's OpenGL ES translator), NDK r27:
  * the library's tests and the conformance suite on OpenGL ES and Vulkan, single frame and 3 frames: 17 / 17 each;
  * `showcase`, `workbench` and `glass_window` as APKs on OpenGL ES (about 60 fps) and Vulkan (60 fps, 1.5 ms of GPU
    time); the app sent to the background and back twice keeps its device;
  * the Languages panel in every script, emoji and flags in color, Roboto as the UI font, Material icons;
  * touch: a swipe from a row or a segmented control scrolls without pressing, a tap selects, a sideways drag moves a
    slider; a drag held at the end recorded at 30 fps for 4 s: the content moves 0 pixels (before: a pixel every
    fourth frame).
* `cmake --preset android` (arm64-v8a) configures and builds the three APKs.

### Not verified

* A real Android phone (only the emulator ran), arm64 devices' GPUs (Adreno, Mali), Android versions before 15.
* Input methods that compose (pinyin and the like) on Android: a NativeActivity gets key events only, so composed
  text does not arrive.
* The Vulkan validation layer on Android: it is not in the NDK and not packed into the APKs.
* Touch on a Windows touch screen (the marking of its mouse messages is untested).
* macOS and iOS after these changes: CI only.

## Examples on Linux

### What changed

* `examples/glass_window/app_linux.cpp`: the examples' frame on X11 (XWayland on a Wayland desktop). Events and frames
  on one thread; mouse, wheel, keys by keysym with the server's key repeat, text through XIM (committed text only),
  focus, cursors, the `CLIPBOARD` selection, the UI scale from `Xft.dpi`.
* Hosts: `host_opengl_egl.cpp` (an EGL window surface, OpenGL 3.3 core, the config of the window's visual);
  `host_vulkan.cpp` also on Linux (`libvulkan.so.1`, `VK_KHR_xlib_surface` declared by hand: `vulkan_xlib.h` would bring
  Xlib's macros). `Host::Init` takes a `NativeWindow` (an `HWND`, or an X11 display and window).
* Icons: `symbol_text.hpp` is the Apple frame's symbol text system, moved out of `app_apple.mm` with the rasterizer as
  a parameter; on Linux `icons_linux.cpp` draws esia::ui's icons from the symbolic icon theme (Adwaita, else Yaru)
  through librsvg and cairo loaded at run time.
* `showcase`: PNG (the test kit's decoder) and JPEG (libjpeg, when found) wallpapers, Ubuntu's or GNOME's;
  `--spread` lays every panel out side by side (the README's picture).
* Text: an ellipsis no longer cuts a label whose box is exactly its measured width: `(x + w) - x` could come out a
  fraction of a pixel short ("Dark Mo..." on Linux).
* CI's Linux job installs `libx11-dev` and `libjpeg-dev`, so it builds the frame.

### Verified (2026-10-02)

* Ubuntu 24.04 (GNOME on Wayland, XWayland) in VMware Workstation 17, virtual hardware 21: `showcase` and
  `glass_window` on OpenGL (SVGA3D, OpenGL 4.3 core, the host's RTX 4080 SUPER; about 150 - 290 fps at 1280 x 800 to
  1600 x 1000) and on Vulkan (lavapipe, validation layer: 0 messages); `--debug` on OpenGL: 0 counted messages (Mesa
  prints one compiler warning about an uninitialized temporary in the generated GLSL). `linux-clang` with
  `ESIA_WERROR=ON` and `linux-clang-release`: ctest 19 / 19.
* Windows (MSVC, RTX 4080 SUPER), after the host interface change: ctest 33 / 33; `showcase --debug` on Direct3D 11,
  12, 10, 9Ex, OpenGL and Vulkan: 0 messages each.

### Not verified

* A Linux machine with a GPU driver (Mesa radeonsi / iris, NVIDIA's), other desktops (KDE, Xfce), a scaled (HiDPI)
  display, an input method's composition.
* `app_apple.mm` after the move of the symbol text system: compiled by the macOS job of CI only.

## Widget layer, second part (`feat/ui-overlays`)

Branch from `main` at `7a2f6e2`, written by the local session. The widgets and the comparison with WGT:
[UI_WIDGETS.md](UI_WIDGETS.md), sections 4 (glow halos), 10 (the island and the dock) and 11 (the showcase).

| Part | Files |
| --- | --- |
| glow halos | `src/esia/ui/layout.cpp`: the item map (`GlowContainment` from last frame's laid-out items, WGT's `ComputeGlowHalo`) and room for a glow in stacks, grids and flows |
| the island, notifications, the dock | `src/esia/ui/overlays.cpp` (new); `Ui::Notify` / `SetActivity` / `ClearActivity`, `UiDesc::island`, `ui::Dock` in `ui.hpp` |
| WGT's layout, pixel for pixel | `Context::Snap` snaps down as Dear ImGui did ([UI_CORE.md](UI_CORE.md) section 6); the Ui sets the core's item spacing from the theme as WGT did; `ViewRect()` includes the padding; section headers and footers take WGT's spacing; a group that fills the width inside a stack is a flexible child; scroll edge fades end at the display's edge when a window reaches past it (Dear ImGui's clip rects stayed on the display) |
| fonts | Windows' chain: Yu Gothic UI instead of Yu Gothic (DirectWrite's choice; proportional kana) and Nirmala UI, Leelawadee UI, Ebrima, Gadugi, Myanmar Text, Javanese Text, Segoe UI Historic and Segoe UI Emoji for the scripts Segoe UI leaves out; Linux's gained Noto Sans Devanagari and Thai. The FreeType text system gives kana and Hangul to a fallback whose OS/2 code pages name Japanese or Korean, Han and CJK punctuation to the paragraph's CJK font before them, and a space to the requested font when it has one (as CSS and DirectWrite) |
| Direct3D 9 (found running the showcase with `--debug`, which Windows put on the RTX 4080 while other programs' D3D9 ran on the AMD iGPU) | uploads to render targets and readback go through a system-memory texture: NVIDIA's driver has no L8 offscreen plain surface, so the glyph atlas never uploaded there and D3D9 drew no text on NVIDIA. A pixel shader over the device's instruction slots (the full FX shader, then ~5.5k, against NVIDIA's 4096) is logged as information, not as an error (`d3d::InstructionSlots`). Tests: an R8 target uploaded and read back, the slot count of a glass variant |
| the glass rim light (after the owner saw a dent in a round button) | `esia_fx.hlsl`: the rim light's Fresnel term peaks within a fraction of a pixel of the outline and was sampled at the pixel center, so the line flickered along curves (WGT's too). It was first filtered across the outline with a tent 1.6 px wide (the flicker of the rim's peak along the arc of the Control Center's Bluetooth button at 1.5x: 7.36 -> 4.53, dips 4 -> 0); the owner then saw a notch at the top of the button: the tent's five taps crossed the outline (where the term ends at once) one after the other, so along the flat top of a circle the line rose in steps and spread light a pixel above it. Now it is averaged over the pixel exactly: the pixel's square seen across the outline is a trapezoid, and the average over it is a second difference of the profile's second antiderivative, a polynomial (`RimG`). A model of the button (16 x 16 samples a pixel) matches it to 2 / 255, the tent was 48 off; the airplane button's top row now rises and falls smoothly (118 ... 194 ... 123, before 156 -> 192 ... 182 -> 147). The peak's flicker along the arc is that of exact coverage, 4.84 (the tent 3.50, the pixel center 7.36), no dips. D3D9: the full FX shader 3801 -> 3864 slots, the glass variant 2048 -> 2110. The five glass goldens carry each change as golden + (new - old) of D3D11 renders; the shader library was regenerated by the `Shaders` workflow (`docs/CI.md`, section 5) |
| D3D9: every FX variant within NVIDIA's 4096 `ps_3_0` slots | the rim filter pushed two glass variants over NVIDIA's limit (fill + stroke + shadow + glow + glass + noise + halo: 4142; with a mask: 4135), and one was over before it (fill + stroke + glow + glass + merge: 4176): those batches were not drawn on D3D9 with an NVIDIA GPU, nor the full shader (~5.5k) that stands in for variants still compiling. SM3 counts code, and the backdrop's level sampling was inlined ten times: the B-spline taps are now computed before the branch that picks a level's texture (`WgtBSplineTaps` / `WgtBSplineRead`), the two levels a blur blends are read in a loop (`WgtSampleBackdrop`, `WgtSampleBackdropSoft`), the glass reads its refracted backdrop once and dispersion adds red and blue, and `BevelFresnel` needs no `rsqrt` / `pow`. Full shader 5475 -> 3801 slots, glass 3723 -> 2048, the heaviest variant 4256 -> 2581; renders change by at most one level in a few pixels. The D3D9 test now fails when the full shader stops fitting in 4096 |
| D3D9Ex flip model | `glass_app`'s D3D9 host presents with `D3DSWAPEFFECT_FLIPEX` (two back buffers, wrapped per frame): with the copy model vsync held the showcase at 90 fps on a 240 Hz display driven by the AMD iGPU while the device was on the RTX 4080; now 240 (558 without vsync, was 267) |
| shader compile stalls | the D3D backends compile HLSL at runtime on the render thread's first use: the showcase's first frame took 0.65 s on D3D11 and 1.8 s on D3D9, a user effect (the plugin panel's hologram) showed up to seconds late, and D3D9 stalled 70 - 370 ms per new variant. Now: a disk cache of compiled shaders (`esia::rhi::d3d::SetShaderCacheDirectory`, keyed by the preprocessed source, profile, flags and compiler; glass_app keeps it in `%LOCALAPPDATA%\Esia\ShaderCache`, `--no-shader-cache` to measure a cold start): first frame 30 ms (D3D11), 45 ms (D3D12). User effects start compiling at the first frame after `SetEffectSource`, not when a shape first uses them, and on D3D9 (`Caps::firstDrawCompiles`) a ready one draws once where no pixel is written (NVIDIA's D3D9 driver compiles at the first draw: that 140 ms moved from opening the plugin panel to the second frame; D3D12's debug layer warns about that empty scissor, so only D3D9 reports it). D3D9 builds feature variants only where the full shader does not fit (below 4096 slots): the RTX 4080's startup stalls went from 0.85 s to 0.5 s. showcase `--open-later panel@frame` measures a panel's first appearance. Tests: the disk cache (a load gives the same bytecode, a damaged entry is rebuilt), the prewarm and warm-up draw |
| OpenGL constants (the showcase's GPU time was twice D3D11's on the RTX 4080 and barely grew with the window) | `gl_device.cpp`: one uniform buffer per constant slot, orphaned by every update, instead of `glBufferSubData` into a ring: NVIDIA's driver lets the draws that read a buffer finish before `glBufferSubData` writes any of it, a GPU drain at each of ~280 updates per frame. GPU time 2.1 -> 1.2 ms at 2400 x 1500 (D3D11: 0.9); the AMD iGPU is unchanged. The renderer's CPU time on NVIDIA's OpenGL (~3 ms with vsync off) is the wait for the previous frame's present, which its threaded driver hands to the frame's first `glGet`; D3D11 waits in `Present`, Vulkan in the acquire |
| batching across draws that do not overlap (the showcase alternated shapes and text: ~130 ops a frame, 97 of the boundaries between a text draw and an FX batch) | `frame_plan.cpp`: a draw joins the latest compatible batch (same clip, texture, effect or text program, edge fade) when every op after it touches other pixels, 64 ops back at most; instances and indices are laid out batch by batch at the end. Glass and user effects keep their place (they join only the last op, nothing moves across them), so the backdrop captures and the glass shader's batches stay as they were; layers and callbacks end the search. Geometry commands get vertex bounds (up to 16k indices; larger ones use their clip) and are dropped when nothing lies inside the clip. The showcase: 201 -> 162 draw calls a frame, the same captures (16 a frame), GPU and renderer CPU time a little lower or the same; the 17 conformance scenes render identically (D3D9, D3D11, Vulkan, OpenGL, compared pixel for pixel with the planner before), the null goldens were regenerated (clipping, edge_fade, windows). Tests: shapes and labels batching across each other, overlap and callbacks keeping the order, glass keeping its place |
| D3D9 uploads to render targets | `d3d9_device.cpp`: the system-memory textures they go through are kept (8, power-of-two sides, up to 4 MB each; larger uploads get a temporary one) and written again only 3 frames after their copy. The FX instance texture is a copy destination, so a render target on D3D9, and is uploaded every frame: a new texture each time cost 25 us an upload on the RTX 4080 (~50 us a frame), a kept one 3 us |
| the example frame | `glass_app`: `--scale` (pixels per UI unit whatever the monitor), fractional `--size`, a renderer hook (custom effects), the frame's CPU time and render stats |
| the showcase | `examples/showcase`: WGT's demo (`main.cpp`), the wallpaper loaded with WIC (`image_file.cpp`) |
| tests | `tests/ui/test_ui.cpp`: glow room, the island from another thread, dock tiles, a filling group, the edge fade at the display's edge; `tests/text/test_ft_cjk.cpp`: a space between fallback characters, kana from Yu Gothic UI (Windows), Devanagari and Thai in the chain |

### Verified on Windows (the local session)

* Clean builds of every backend with `ESIA_WERROR`, clang-cl and MSVC: every test passes with each.
* The showcase with every panel open and `--debug`, 240 frames on each API: no debug-layer or validation message
  on D3D9 (RTX 4080), D3D10, D3D11, D3D12, Vulkan and OpenGL.
* The showcase captured like WGT's reference screenshots (branch `reference/wgt-1.1`), each panel dark and light at
  UI scale 1 and 1.5, and compared pixel by pixel: [UI_WIDGETS.md](UI_WIDGETS.md) section 11 has the numbers and
  what still differs.

### Not verified

* Linux and macOS: as for the first part, the showcase needs the Win32 platform layer. (Since 2026-10-01 / 02 the
  macOS and X11 frames run it: section "Examples on Linux", and the README's macOS section.)
* By hand: the dock's magnification and the island's animations were looked at in captures and fixed-clock frames,
  not used at a real frame rate.

## Widget layer, first part (`feat/ui-foundation`)

Branch from `main` at `e43ea9f`, written by the local session. What is ported, how it maps onto the core and what
is left: [UI_WIDGETS.md](UI_WIDGETS.md), section 13.

| Part | Files |
| --- | --- |
| the widget layer | `include/esia/ui` (`theme.hpp`, `anim.hpp`, `icons.hpp`, `ui.hpp`), `src/esia/ui` (`theme`, `anim`, `ui` with the style resolution, `controls`, `layout`, `containers`, `selection`, `lists`, `navigation`, `text_field`, `popups`, `charts`) |
| `Color::Lighter` / `Darker` | `include/esia/base/math.hpp` |
| `Context::ViewRect()` | the core: what the innermost child region shows, where floating bars go ([UI_CORE.md](UI_CORE.md) section 15) |
| `TextSystem::CaretStops` | the text interface: grapheme boundaries with caret x (a default for any text system; the FreeType one's in `ft_text_system.cpp`, tested in `test_ft_text.cpp`) |
| the direction of neutrals | `ft_text_system.cpp`: white space and punctuation around a right-to-left word stay where they were typed ([REWRITE.md](REWRITE.md) section 9) |
| the example frame | `examples/glass_window`: options, the window and render threads and screenshots moved from `main.cpp` into the `glass_app` library (`app.hpp`, `app.cpp`), so other examples share them |
| the showcase | `examples/showcase` |
| tests | `tests/ui/test_ui.cpp` (`esia_ui_tests`) |

### Verified on Windows (the local session)

* Clean builds of every backend with `ESIA_WERROR`, clang-cl 22.1.8 and MSVC 19.44: 36 of 36 tests pass with each,
  `esia_ui_tests` included (Metal's two conformance tests skip on Windows, as before).
* `showcase` (the first one, before WGT's demo replaced it) with `--debug` on D3D11 (light) and D3D12 (dark): no
  debug-layer message, and every widget it showed renders.
* `glass_window` after its frame moved into `glass_app`: D3D11, Vulkan and OpenGL with `--debug`, no message.

### Not verified

* Linux and macOS: `esia_ui` has no platform code and builds with the core there, but `showcase` needs the Win32
  platform layer (`glass_app`), so nothing of it renders in CI. (Since 2026-10-01 / 02 the macOS and X11 frames run
  it; CI still renders none of it.)
* Use by hand: the tests drive input by script, and the screenshots are of fixed frames. Dragging windows and
  sliders, and the springs at a real frame rate, were only looked at, not checked.

## UI core v2

Branch `feat/ui-core-v2` (from `main` at `81ce5dc`): the UI core reworked for the widget port (phase 3). The design,
the whole API and how WGT's widgets map onto it are in [UI_CORE.md](UI_CORE.md); only `include/esia/{base,core}`,
`src/esia/core`, `tests/core`, one conformance golden log and docs changed. The renderer, RHI, backends and shaders
were not touched.

### What changed

| Commit | What |
| --- | --- |
| `17d9e25` | `docs/UI_CORE.md`: the design, written first |
| `3acdf63` | input: `kNoMousePos` sentinel (negative coordinates are positions), focus loss cancels presses and invalidates the mouse, click counts (`MouseClickCount`), out-of-range buttons / keys ignored, text decoded with `base/utf8.hpp` (`AppendUtf8` / `AppendUtf32` removed, `EncodeUtf8` added) |
| `14ce8e5` | draw lists: `Mark` / `MoveCommands` (a card's background drawn after its content, moved under it) |
| `4e77370` | the core: front-to-back hit testing of last frame's item rects; press ownership per mouse button; disabled items that claim the hover; repeat buttons; key ownership, Tab / Escape after the widgets; `InputPending`; containers with the layout cursor or a `LayoutProvider` (`StackLayout`), laid-out item reports, baselines, work rects, pixel snapping; child regions with clip and (smooth) scroll, deferred scroll targets, wheel routing; the content clip; window lifecycle (front on reappearing, dangling focus / drag cleared, garbage collection, kept on screen, resize grab offset, auto-size, DPI per window); popups and tooltips; `State<T>`, scope data, `ItemStatus` |
| `df5fac2` | the `windows` scene's null log (below); UI_CORE.md aligned with the code |
| `f7e4f0d` | unused child scroll states freed; README and REWRITE.md sections 3 - 4 point at UI_CORE.md |
| `fbc86aa` | fixes from an independent review of the implementation (UI_CORE.md, end of section 14): key-claim precedence, DPI hysteresis at monitor boundaries, floating padding, `SetNextScroll` on new areas, `StackLayout` centering, hit records that follow scrolled content, `State<T>` type tags and throwing constructors, keyboard-activated items, smooth scroll ending on a pixel |

Every review item of the task (the 12 bugs) and every new piece has a unit test: `tests/core/test_input.cpp`,
`test_interaction.cpp`, `test_windows.cpp`, `test_layout.cpp`, `test_draw_list.cpp` (the table in UI_CORE.md section
14 maps bugs to tests). `esia_core_tests` went from 39 to 93 tests.

**The `windows` conformance scene.** `Begin` now pushes the content clip (bug 8: visibility and hover clip to the
content rect). Each of the scene's two windows has a last row that lies entirely in the bottom padding (Settings: y
148 - 170 under a content rect ending at 140; Library: 216 - 238 under 208): it is culled, and the windows' scissors
are their content rects (`[28,28 152x112]`, `[140,96 152x112]`) instead of the window rects. Only
`golden/null/windows.log` was regenerated; the other 16 logs are byte-identical. The cloud session left the image
golden `golden/windows.png` alone (the task allowed only the null log): rendered with OpenGL / GLES on Mesa llvmpipe,
all 16 other scenes still matched their goldens at max delta 0, and `windows` differed exactly in the two culled
stripes (2.12 % of the pixels, max delta 194, over the 1 % / 48 tolerance).

**`golden/windows.png` updated by the local session.** On the RTX 4080 SUPER all seven drawing backends failed
`windows` identically: the same 1634 pixels over 10, in two boxes, `(28,142)-(192,156)` and `(140,216)-(303,219)`. In
the old golden those are the culled rows' blue bars, drawn into the bottom padding and, for Library, past the
window's rounded bottom edge; the new images clip them to the content rect. The new golden is the old llvmpipe image
with only those two boxes, grown by 2 pixels (4555 pixels), taken from the 4080's OpenGL rendering. The seven
backends agree within 4 inside the boxes, and the 4080 is within 6 of llvmpipe outside them. Every other pixel is
still llvmpipe's. Every drawing backend now passes `windows` at max delta 5 - 6. CI's llvmpipe job (`feat/ci`) is the
check that llvmpipe agrees inside the boxes too.

### Verified

Ubuntu 24.04 container without a GPU; clang 18.1.3 + lld, CMake 3.28.3, Ninja, mingw-w64 GCC 13 (posix), Wine 9.0,
Mesa 25.2.8 (llvmpipe through EGL). Fresh build directories, at `fbc86aa` (the code of the final commit):

| Command | Result |
| --- | --- |
| `cmake --preset linux-clang -DESIA_WERROR=ON && cmake --build --preset linux-clang && ctest --preset linux-clang` | 0 warnings; 7 / 7: `esia_core_tests` 93, `esia_text_tests` 9, `esia_render_tests` 42, `esia_testkit_tests` 5, `esia_conformance_null` (17 scenes, `--strict`), `esia_shader_tests` 1, `esia_glsl_link_tests` 1 (no FreeType here: `esia_text_ft_tests` not built) |
| the same with `linux-clang-release` | 0 warnings; 7 / 7, the same counts |
| `build/linux-clang{,-release}/bin/esia_conformance --backend null --golden tests/conformance/golden --strict` | 17 / 17 each |
| `cmake --preset windows-mingw-cross -DESIA_WERROR=ON && cmake --build --preset windows-mingw-cross` | 0 warnings |
| `wine build/windows-mingw-cross/bin/esia_core_tests.exe`; `wine .../esia_conformance.exe --backend null --golden tests/conformance/golden --strict` | 93 / 93; 17 / 17 |
| a scratch `-DESIA_BACKEND_OPENGL=ON` build (`build/gl`), `esia_conformance --backend opengl --backend gles --golden tests/conformance/golden --strict` | 16 / 17 each: every scene but `windows` at max delta 0 (`msaa_target` 1); `windows` as described above (GL and GLES render it identically) |
| the commit that adds only the input changes (`3acdf63`), built alone on `main` in a scratch worktree | `esia_core_tests` 44 / 44 |

`python3 tools/shaders/build_shaders.py --check` could not run (no glslangValidator / spirv-cross here); no shader
source or generated file was changed.

### Verified on Windows (the local session: Windows 11, RTX 4080 SUPER, clang-cl 22.1.8, MSVC 19.44)

On `feat/ui-core-v2` with `main` (text on every platform) merged in: clean build directories, every backend on,
`-DESIA_WERROR=ON`, `ESIA_D3D_DEBUG=1`.

| Command | Result |
| --- | --- |
| `windows-clang-cl`, build, `ctest` | 0 warnings; before the new `windows.png`: 18 / 34, every drawing backend's conformance runs failing `windows` only (above); after it **34 / 34** (Metal's conformance skipped as before) |
| `windows-msvc`, `windows-msvc-debug`, `ctest --preset windows-msvc` | 0 compiler warnings; 34 / 34 |
| `esia_conformance --backend <each> --scene windows --strict` | opengl / gles / d3d9 / d3d10 / d3d11 / d3d12 max delta 6, vulkan 5 |

### Not verified

* `windows-cross` (xwin) was not built.
* The API has not been exercised by real widgets yet: the WGT port (phase 3) is its first user, and UI_CORE.md
  section 12 shows how the widgets map, not ported code.

### Known limitations

* The hit test uses last frame's window rects and item rects (shifted for scrolling): a window the application moves
  this frame, or an item laid out somewhere new, is hit where it was until the next frame.
* A child region may be begun once per frame (a window may be appended to; a child region may not).
* No modal popups in the core: a dialog is an overlay window that claims its keys (Enter / Escape) and consumes
  clicks itself. No keyboard / gamepad navigation beyond Tab / Shift+Tab.
* Key ownership lags by a frame when it changes hands (last frame's owner keeps a key until it stops claiming it).
* `SetMonitors` has no platform layer to call it yet (phase 4); without monitors every window takes
  `FrameParams::framebufferScale.x`.

### What the widget port must do next

1. Port WGT's `src/ui` (tag `wgt-1.1-final`) along UI_CORE.md section 12: `InteractImpl` on `ItemAdd` /
   `ButtonBehavior` (no overlap flags: submit what is on top later, `ItemFlags_Background` for late backgrounds);
   WGT's `Arrange*` functions (stacks, adaptive stack, grid, flow) as `LayoutProvider`s kept in `State<T>`; the
   glow-halo item map and the layout inspector on `Window::LaidOutItems()` (`depth`, `floating`); cards and sections
   as containers with `DrawList::Mark` / `MoveCommands`; windows as `Begin` (padding 0) + a surface drawn with an
   explicit clip + a `ChildFlags_ScrollY | SmoothScroll` body, with drag-to-scroll, rubber banding and the
   indicator on `HoveredChild`, `ActiveId` and `SetScrollY`; pickers and menus on `BeginPopup`; the tab and search
   bars as floating children; the text editor on `ClaimKeyboard`, `MouseClickCount`, `Input().Text()`,
   `RequestTextInput` and its own `SetMouseCursor`.
2. Styling: `Theme`, `ItemStyle` and `ui::Next()` in `esia_ui`; scope styles through `SetScopeData` on containers,
   state colors from `ItemStatusOf`, metrics scaled by `Context::Scale()` (per window).
3. Add conformance scenes drawn by the ported widgets against the WGT reference screenshots (branch
   `reference/wgt-1.1`). (`golden/windows.png` has been updated, above.)
4. The platform layers (phase 4) feed `InputEvent::MouseLeave`, focus events and `SetMonitors`, and honor
   `PlatformRequests::inputPending` / `animating` in their event loops.

## Text on every platform (`feat/text-everywhere`)

Before this branch `esia_text_ft` was built only where FreeType and HarfBuzz were installed as system packages: on
Windows it was skipped, and its tests had never run there. The owner writes Chinese, so CJK text had to work.

### What changed

| Part | Where | What |
| --- | --- | --- |
| Bundled dependencies | `cmake/EsiaTextDeps.cmake`, `src/esia/CMakeLists.txt` | `ESIA_TEXT_DEPS=auto` (default: the system's packages when both are found, else built from source), `bundled` or `system` (an error when missing). FreeType 2.14.3 and HarfBuzz 14.5.0 are fetched with `FetchContent` (URL + SHA256; FreeType from SourceForge with savannah as the second URL, HarfBuzz from its GitHub release). Esia defines the targets itself (`SOURCE_SUBDIR` points to a directory without a `CMakeLists.txt`, so the projects' own scripts do not run: no install rules, no optional-library probes, no HarfBuzz "CMake is unsupported" warning): FreeType's TrueType, CFF, SFNT, PSAux, PSNames and PSHinter modules with FreeType's `ftoption.h` minus its internal zlib and LZW (WOFF 1, compressed PCF), no bzip2 / libpng / Brotli / HarfBuzz; HarfBuzz's amalgamated `harfbuzz.cc` with no glib / ICU / FreeType / Graphite / platform shaper (`/bigobj /utf-8` on MSVC, as its own builds). Both are static, compiled with `-w` / `/W0` and never Esia's flags; their headers are system includes, so `-DESIA_WERROR=ON` concerns Esia's code only. Offline: `FETCHCONTENT_SOURCE_DIR_ESIA_FREETYPE` / `..._ESIA_HARFBUZZ` point at extracted archives. `ESIA_TEXT_FREETYPE=OFF` still leaves the text system out. |
| System fonts | `include/esia/text/system_fonts.hpp` (new), `src/esia/text/font_file.*`, `system_fonts*.cpp` (in `esia_text`) | `FindSystemFont(family, weight, style)` returns the file path (UTF-8), face index, family, weight and style of an installed face, or nothing (a family is never substituted); `FindFontInDirectories` does the same in given directories; `DefaultFallbackFamilies()` / `FindDefaultFallbackFonts()` give the platform's chain, `AddSystemFont` / `AddFallbackFonts` load fonts into a `TextSystem`; `kSystemUiFamily` ("system-ui") names the UI font. Windows: DirectWrite's system font collection, only to locate files (`IDWriteLocalFontFileLoader`); macOS: Core Text descriptors with the family mandatory, their URLs, the collection face found by PostScript name; Linux: fontconfig when found (`ESIA_TEXT_FONTCONFIG`, optional; substitutes are rejected, generic names such as `sans-serif` allowed), else one scan of `$XDG_DATA_HOME/fonts`, `~/.fonts`, `$XDG_DATA_DIRS/fonts`, `/usr/share/fonts`, `/usr/local/share/fonts`, `/system/fonts`. Families match any localized name ASCII case-insensitively; the face is chosen by style, then by weight with the CSS rules. A small portable reader of the `name`, `OS/2` and `head` tables (collections included) serves the scans and the macOS face lookup. |
| Chains | `system_fonts_<platform>.cpp` | Windows: Segoe UI, Microsoft YaHei, Microsoft JhengHei, Yu Gothic, Malgun Gothic, Segoe UI Symbol. macOS: system-ui (SF), Helvetica Neue, Hiragino Sans GB, Heiti SC, Heiti TC, Hiragino Sans, Apple SD Gothic Neo, Apple Symbols (no PingFang: private to the system since macOS 12, see "Found by CI"). Linux: Noto Sans, Noto Sans CJK SC / TC / JP / KR, DejaVu Sans, Noto Sans Symbols, Noto Sans Symbols 2, WenQuanYi Zen Hei, Droid Sans Fallback (the last two for distributions without Noto CJK). Simplified Chinese comes first among the CJK fonts. |
| FreeType text system | `src/esia/text/ft/ft_text_system.cpp` | The faces of one collection file share one copy of it (the CJK fonts of a chain are 20 MB and more: Noto Sans CJK's SC / TC / JP / KR faces were four copies). No interface change. |
| Tests | `tests/text`, `tests/fonts`, the text part of `tests/CMakeLists.txt` | `NotoSansSC-Subset.otf`: 87 characters of Noto Sans SC (OFL, CFF outlines, 18 KB) made with `pyftsubset` (command and character list in `tests/fonts/README.md`). `test_ft_cjk.cpp`: CJK shaping (one full-width glyph per character, the ideographic space), fallback from DroidSans to Noto with DroidSans' line box, line breaks between ideographs with the kinsoku rules, CJK trimming, the golden image `ft_cjk.png` (Chinese mixed with Latin, wrapped, trimmed, centered; Traditional characters and kana), and the platform chain covering Latin, Simplified and Traditional Chinese, kana, Hangul and symbols. `test_system_fonts.cpp` (in `esia_text_tests`): lookups in `tests/fonts`, collections and the CSS weight / style choice on `.ttc` files the test assembles from the test fonts (`font_test_util.hpp`), and the platform lookup (no substitution for a missing family, every chain family found, localized names, Tahoma on Windows and Wine). A chain font or script that is missing fails on Windows and macOS, which ship them, and is reported as "skipped" on Linux and under Wine. `test_ft_text.cpp` gained a collection test; its draw-list and golden helpers moved to `ft_test_util.hpp`. |

The text interface (`text.hpp`, `freetype.hpp`) is unchanged; `system_fonts.hpp` is new and only builds on it.

### Verified here (Linux, clang 18.1.3, CMake 3.28.3)

The archives downloaded in this cloud environment (SourceForge and GitHub releases; savannah answered 403, the
second URL is a fallback only). SHA256 are those of the downloaded archives.

| Command | Result |
| --- | --- |
| `cmake --preset linux-clang -DESIA_WERROR=ON -DESIA_TEXT_DEPS=bundled`, `cmake --build --preset linux-clang`, `ctest --preset linux-clang` | configure without CMake warnings, 0 compiler warnings (Esia's and the dependencies'); 7 / 7: core 39, text 13, render 42, testkit 5, text_ft 19, `esia_conformance_null` 17 scenes, shader 1 (no `libegl-dev` here: the GLSL link test is left out) |
| the same with `-DESIA_TEXT_DEPS=system` (FreeType 2.13.2, HarfBuzz 8.3.0) | 0 warnings, 7 / 7; both goldens (`ft_gray`, `ft_cjk`) pass with either FreeType / HarfBuzz |
| `linux-clang-release`, both `bundled` and `system`, `-DESIA_WERROR=ON` | 0 warnings, 7 / 7 each |
| `cmake -S . -B <dir> -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang-linux.cmake -DESIA_WERROR=ON -DESIA_TEXT_FONTCONFIG=OFF -DESIA_TEXT_DEPS=system`, build `esia_text_tests esia_text_ft_tests`, run both | the directory scan finds what fontconfig finds (DejaVu Sans, WenQuanYi Zen Hei `wqy-zenhei.ttc` face 0, its Chinese name too); 13 + 19 pass |
| `cmake --preset windows-mingw-cross -DESIA_WERROR=ON -DESIA_TEXT_DEPS=bundled`, `cmake --build --preset windows-mingw-cross` | 0 warnings; `esia_text_ft_tests.exe` is built for Windows for the first time |
| `WINEDEBUG=-all wine build/windows-mingw-cross/bin/<test>.exe` (Wine 9.0) | core 39, text 13, render 42, testkit 5, shader 1, **text_ft 19** (both goldens), all pass; `esia_conformance.exe --backend null --strict` 17 / 17. DirectWrite under Wine finds Tahoma (`Z:\usr\share\wine\fonts\tahoma.ttf`) and the host's DejaVu Sans and WenQuanYi Zen Hei (also by "文泉驛正黑"); none of the Windows chain is installed there, so the chain tests report "skipped" |

On this Linux machine the chain found DejaVu Sans and WenQuanYi Zen Hei, which cover every script of the test (Noto
is not installed).

### Verified on Windows (the local session: Windows 11, RTX 4080 SUPER, clang-cl 22.1.8, MSVC 19.44)

Steps 1 - 4 of the Windows test plan below, on `0c5fad2`, clean build directories, every backend on, `ESIA_D3D_DEBUG=1`.

| Command | Result |
| --- | --- |
| `cmake --preset windows-clang-cl -DESIA_WERROR=ON -DESIA_TEXT_DEPS=bundled -DESIA_BACKEND_<all>=ON`, build, `ctest --preset windows-clang-cl` | configure downloads both archives, no CMake warning; 0 compiler warnings (Esia, FreeType, HarfBuzz; no `D9025`); **34 / 34** (Metal's two conformance runs skipped as before), `esia_text_ft_tests` built and run with clang-cl for the first time |
| the same with `--preset windows-msvc`, `windows-msvc-debug` and `windows-msvc-release`, `ctest` Debug and Release | 0 compiler warnings (only MSBuild's `MSB8029`, because the build directory was under `%TEMP%`); 34 / 34 in both configurations |
| `esia_text_tests.exe SystemFonts`, `esia_text_ft_tests.exe SystemFonts` | 4 + 1 pass, no "skipped" line. Chain: `SEGOEUI.TTF`, `MSYH.TTC` face 0, `MSJH.TTC` face 0, `YUGOTHR.TTC` face 0, `MALGUN.TTF`, `SEGUISYM.TTF`; "Microsoft YaHei = 微软雅黑" |
| a probe on `FindSystemFont` / `FindFontInDirectories` / `AddSystemFont` | YaHei 700 → `MSYHBD.TTC`, YaHei 300 → `MSYHL.TTC` (290); Segoe UI italic → `SEGOEUII.TTF`, bold italic → `SEGOEUIZ.TTF`, 600 → `SEGUISB.TTF`; `system-ui` → Segoe UI; `宋体` and `SimSun` → `SIMSUN.TTC`; Arial 900 → `ARIBLK.TTF`; "Comic Sans" (not a family) → nothing; fonts installed for the current user only (Source Sans 3 400 / 600 in `%LOCALAPPDATA%\Microsoft\Windows\Fonts`) found; a font in a directory named `字体 测试` found by `FindFontInDirectories` and loaded through its UTF-8 path (`你好，世界` at 20 → 100 x 28.96); the whole chain loaded (6 fonts) measures Latin, Simplified and Traditional Chinese, kana, Hangul and ★ together |
| `ft_cjk.png` written by the Windows runs | matches the golden; line breaks keep closing punctuation off the start of a line |

Found on Windows, then fixed by the local session: the weight DirectWrite reports was not always the weight of the face
that is loaded. `SimSun` at 700 returned `SIMSUN.TTC` face 0 with weight 700 (DirectWrite simulates the bold, the file
is regular); variable fonts (`Segoe UI Variable Text` → `SEGUIVAR.TTF`, `Noto Serif CJK SC` → `NotoSerifCJK-VF.ttf.ttc`
face 2) reported the named instance's weight for the same face index, while FreeType loads the default instance. The
Windows lookup now reports the stored face's own weight and style (read from the file's `OS/2` / `head` tables):
SimSun 700 → 400, Segoe UI Variable Text 700 → 400, Noto Serif CJK SC → 200 (its default instance is ExtraLight);
the test `SystemFonts.PlatformLookup` checks it with Tahoma's simulated oblique.

Found by CI on macOS 15 (the first run of `system_fonts_apple.cpp`, 2026-09-30): Core Text resolves "PingFang SC" to
`/System/Library/PrivateFrameworks/FontServices.framework/Resources/Reserved/PingFangUI.ttc`, the system UI's
private copy. Our table reader can read that file, but FreeType cannot load it (a format of its own, most likely).
So no font of the chain covered Simplified Chinese, and `SystemFonts.FallbackChainCoversEveryScript` failed.

The fix, in two parts:
* **The lookup skips the system's private fonts** (anything under `/PrivateFrameworks/`) and files the process cannot
  read. It takes the best remaining face of the family instead, so PingFang is not installed as far as Esia is
  concerned.
* **The macOS chain drops PingFang for Hiragino Sans GB and Heiti SC / TC** (STHeiti), ordinary files in
  `/System/Library/Fonts` on every macOS version. The second CI run found both.

### Not verified

* **macOS**: `system_fonts_apple.cpp` has never been compiled (no Apple SDK here); expect small fixes.
* **`windows-cross`** (clang-cl from Linux with xwin) with the bundled dependencies.
* **Windows, optional step 5**: vcpkg's FreeType / HarfBuzz with `-DESIA_TEXT_DEPS=system`, and the offline configure.

### Windows test plan (the local session: Windows 11, clang-cl 22 and MSVC 19.44)

1. `cmake --preset windows-clang-cl -DESIA_WERROR=ON -DESIA_TEXT_DEPS=bundled`, `cmake --build --preset windows-clang-cl`,
   `ctest --preset windows-clang-cl --output-on-failure`: configure downloads both archives, no warning from Esia's
   code, from the FreeType / HarfBuzz headers or from the dependencies' own compiles (no `D9025` about an
   overridden `/W` level); `esia_text_tests` and `esia_text_ft_tests` pass.
2. The same with MSVC: `cmake --preset windows-msvc -DESIA_WERROR=ON -DESIA_TEXT_DEPS=bundled`,
   `cmake --build --preset windows-msvc-debug` and `--preset windows-msvc-release`, `ctest --preset windows-msvc`.
3. `build\windows-clang-cl\bin\esia_text_tests.exe SystemFonts` prints the six chain families with their files
   (expected under `C:\Windows\Fonts`: `segoeui.ttf`, `msyh.ttc` face 0, `msjh.ttc` face 0, `YuGothR.ttc`,
   `malgun.ttf`, `seguisym.ttf`), "Microsoft YaHei = 微软雅黑" with the same file, and passes;
   `esia_text_ft_tests.exe SystemFonts` passes with no "skipped" line (on real Windows the chain must cover Latin,
   Simplified and Traditional Chinese, kana, Hangul and symbols: a missing script fails).
4. Spot checks worth a line in the report: `FindSystemFont("Microsoft YaHei", 700)` gives `msyhbd.ttc`;
   `FindSystemFont("Segoe UI", 400, FontStyle::Italic)` gives `segoeuii.ttf`; a font installed for the current user only
   (`%LOCALAPPDATA%\Microsoft\Windows\Fonts`) is found; a path with non-ASCII characters loads (the paths are
   UTF-8).
5. Optional: `-DESIA_TEXT_DEPS=system -DCMAKE_PREFIX_PATH=<vcpkg>/installed/x64-windows` with vcpkg's `freetype` and
   `harfbuzz`; and offline configure with `-DFETCHCONTENT_SOURCE_DIR_ESIA_FREETYPE=<dir>
   -DFETCHCONTENT_SOURCE_DIR_ESIA_HARFBUZZ=<dir>`.

### macOS test plan

`brew install llvm lld cmake ninja`, then `cmake --preset macos-clang -DESIA_WERROR=ON -DESIA_TEXT_DEPS=bundled`,
`cmake --build --preset macos-clang`, `ctest --preset macos-clang --output-on-failure`. `esia_text_tests SystemFonts`
should list system-ui (`SFNS.ttf`, face 0), Helvetica Neue (`HelveticaNeue.ttc`), Hiragino Sans GB, Heiti SC and Heiti TC (two faces of `STHeiti Light.ttc`), Hiragino Sans, Apple SD Gothic Neo and Apple Symbols; `esia_text_ft_tests
SystemFonts` must pass without "skipped" lines. Then the same with `-DESIA_TEXT_DEPS=system` after `brew install
freetype harfbuzz`.

### Known gaps

* **Color emoji** (COLR, CBDT, sbix, SVG) are out of scope: the rasterizer draws outlines, so no emoji font is in the
  chains.
* **Han unification**: one chain for every language, Simplified Chinese first; Japanese text gets Chinese glyph forms
  for shared ideographs where the chain has both. A per-language chain (from the text's language tag or the user's
  locale) is future work.
* **Fonts are found once**: the Windows collection and the Linux directory scan are taken at the first lookup; fonts
  installed later need a restart. `AddFallbackFonts` loads every chain font into memory up front (collection faces
  share one copy); loading a fallback only when a character needs it is future work.
* **Security**: font files are parsed by FreeType and HarfBuzz as before; the lookup's own table reader is bounded
  (faces, name table size, every read checked against the file size).
* **Variable fonts: the default instance only.** `SystemFont` reports the stored face as it is (above), so a request for
  a bold from a variable font gets the font's default instance and says so. Loading a named instance would need it in
  `SystemFont` (FreeType's face index `instance << 16 | face`); bold / oblique simulation is not done either.

## 0. Round 3: sub-pixel text removed

**By the owner's decision, Esia has no sub-pixel (LCD / ClearType) text any more: text is antialiased in grayscale
everywhere.** Gone: per-stripe RGB coverage and the RGB / BGR stripe order, dual-source blending, the ClearType level
and contrast (and with them anything about OLED stripe layouts). Kept: sub-pixel *positioning* (the quarter-pixel pen
phases of the rasterizer and the atlas) and the grayscale gamma / contrast composition. The legacy WGT code (`src/`,
`include/wgt`, `examples`, the root `CMakeLists.txt`'s legacy shaders) and its documents (`README.md`, `docs/API.md`,
`ARCHITECTURE.md`, `INTEGRATION.md`) still describe WGT's ClearType text: a separate task on another branch.

Commits: `3527cae` text - `fbc8b95` renderer, core textures, conformance - `c7eabc7` RHI and shaders - `273459e`
docs - and the `[cloud-done]` commit with this file.

| Where | Removed |
| --- | --- |
| RHI (`rhi.hpp`, `rhi.cpp`) | `BlendMode::DualSourceLcd`, `ShaderProgram::TextLcd` / `TextLcdGray` (the programs after them are renumbered: `Fx` 2, `Downsample` 3, `LayerComposite` 4, `Clear` 5 - the RHI makes no ABI promise yet), `Caps::dualSourceBlend` |
| Null device | its dual-source refusal and the TextLcd / dual-source pairing check |
| Shaders | `TextLcdPS`, `TextLcdGrayPS`, `TextLcdOut` (`esia_ui.hlsl`); `gText.zw` unused (0). The library regenerated: the 16 TextLcd / TextLcdGray files (SPIR-V, GLSL, ESSL, MSL) gone, the table shorter, every other generated file byte-identical. `build_shaders.py` lost the dual-source SPIR-V patch (Location 1 -> Index 1), the GLES `blend_func_extended` line and the D3D9 TextLcd exception |
| Renderer, frame planner | LCD batches (`RenderOp::lcd`), the LCD-inside-glow-layer fallback, dual-source pipeline requests - text always draws with `TextGray`; `TextComposition::clearTypeContrast` / `clearTypeLevel` |
| Core textures | `TextureFlags_LcdCoverage`, and with it `TextureInfo::flags` (it had no other flag); `Coverage()` means Alpha8 |
| Text | `RasterizeLcd` (the 5-tap LCD filter, the stripe order), `GlyphBitmap::lcd`, the atlas's sub-pixel pages (`PageCount()` has no argument), `text::Antialiasing` / `RasterParams::antialiasing`, `FreeTypeDesc::lcdBgr` |
| Tests | the `text_lcd` scene with `golden/text_lcd.png` and `golden/null/text_lcd.log` (and `Scene::needsDualSource`, the harness's dual-source skip), the LCD filter / stripe-order test, `tests/text/golden/ft_lcd.png`; the atlas, FreeType, frame-plan and renderer tests keep their grayscale halves |
| Docs | REWRITE.md (caps, API table, limits per API, bindings, text section, risks) and the backend guide (caps table, programs and blend modes, the GL / GLES / SPIR-V / D3D9 notes, the conformance SKIP rules) |

The null goldens changed only in the frame constants' `gText.zw` (ClearType contrast and level): `0.5 1` became
`0 0` on the `constants frame` lines of the 17 logs - checked line by line, nothing else changed in any command
stream. The image goldens are unchanged.

### Verified in round 3

Same container and tools as section 2; the core at `273459e` (the code of the final commit), fresh build directories.

| Command | Result |
| --- | --- |
| `cmake --preset linux-clang -DESIA_WERROR=ON`, build, `ctest --preset linux-clang` | 0 warnings; 8 / 8: core 39, text 9, render 42, testkit 5, text_ft 12, `esia_conformance_null` (17 scenes, `--strict`), shader 1, GLSL / ESSL link 1 (every program now links on GL and GLES) |
| the same with `linux-clang-release` | 0 warnings, 8 / 8 |
| `cmake --preset windows-mingw-cross -DESIA_WERROR=ON`, build; the test executables under Wine | 0 warnings; core 39, text 9, render 42, testkit 5, shader 1 pass; `esia_conformance.exe --backend null --strict` 17 / 17 |
| `python3 tools/shaders/build_shaders.py --check` | `generated shaders are up to date` |
| `esia_conformance --backend null --golden tests/conformance/golden --strict` | 17 / 17 (Debug and Release) |
| `esia_text_ft_tests` | the FreeType text golden `ft_gray.png` renders bit-identical |
| `esia-opengl` (`0093194`) with this `esia-core` merged in a scratch worktree (never pushed) and the backend's dual-source code deleted there (11 lines of follow-up 1 below; the unused `GL_SRC1_*` constants left) | `linux-clang -DESIA_WERROR=ON -DESIA_BACKEND_OPENGL=ON`: 0 warnings, ctest 13 / 13; `--strict` and `--frames 3` on llvmpipe: `opengl` and `gles` 17 / 17 each, their images identical to the round-2 renders (max delta 0 on all 17; the scenes with their own golden at max delta 0 against it); with `ESIA_GL_CORE_ONLY=1` 17 / 17 each too, no skip left (`text_lcd` was the only one) |
| `esia-vulkan` (`c31bb35`), the same way (follow-up 2's device and test parts; the `dualSrcBlend` feature request left in) | 0 warnings, ctest 13 / 13 on lavapipe with the validation layer; `--strict` and `--frames 3`, dynamic rendering and render passes: 17 / 17 each, max delta 4, 0 validation messages; the dynamic-rendering images identical to the round-2 renders (max delta 0) |

Not run in round 3: Metal and Direct3D (their dual-source code was not removed in a scratch copy), the Windows builds,
real GPUs.

### Backend follow-ups (round 3)

After merging this `esia-core`, each backend deletes its dual-source blend state and caps and its `TextLcd` /
`TextLcdGray` pipelines; it does not compile before that.

1. **OpenGL**: `caps_.dualSourceBlend` (`GL_EXT_blend_func_extended`), the `GL_SRC1_COLOR` blend case and the
   dual-source refusal (`gl_device.cpp`), the `GL_SRC1_*` constants (`gl_api.hpp`); in `test_gl_device.cpp` the
   `needsDualSource` skips and the `dualSourceBlend` caps check.
2. **Vulkan**: `caps_.dualSourceBlend`, the `DualSourceLcd` blend case and refusal (`vk_device.cpp`), the
   `dualSrcBlend` feature (`Desc::dualSrcBlend` in `vulkan.hpp`, `vk_headless.cpp`, `test_vk_host.cpp`); in
   `test_vk_device.cpp` the TextLcd programs of `ProgramDesc` and the dual-source expectation, and in
   `test_vk_scenes.cpp` `OlderApiVersions`, which asks for `text_lcd` (`FindScene` now returns null there).
3. **Metal**: the `[[color(0), index(1)]]` output: `dualSourceBlend` in `metal_gpu.hpp`, `metal_device.mm` and
   `metal_device_core.cpp`, the `DualSourceLcd` blend state (`metal_tables.cpp`), the TextLcd checks in
   `metal_planning.cpp`; tests: `test_metal_msl.cpp`'s second-output expectation, `test_metal_planning.cpp`,
   `test_metal_tables.cpp`, the fake GPU's `index(1)` handling.
4. **Direct3D 10 / 11 / 12**: `c.dualSourceBlend = true`, the dual-source blend states (`D3D1x_BLEND_SRC1_*`), and
   the `(desc.blend == DualSourceLcd) != (desc.program == TextLcd)` check in each `CreatePipeline`.
5. **Direct3D 9**: `caps_.dualSourceBlend = false`, its refusal of `TextLcd` / `DualSourceLcd` and the blend case; in
   `test_d3d9.cpp` the TextLcd exception of the program loop and the `!dualSourceBlend` check.
6. **Every backend**: tests, SKIP rules and STATUS text that mention `text_lcd` (17 scenes now, none skipped for a
   capability). `ShaderProgram` values after `TextGray` moved down by two: the backends name them symbolically (a
   search found no stored numbers), so rebuilding is enough.

## 1. Round 2: what changed

Commits on `esia-core` after round 1's `79c12ef`, oldest first: `3810b97` and `3a8fe0a` (from the local Windows
session: the legacy `wgt.dll` glob no longer picks up `src/esia`; `MakeScenes` no longer reads a `Scene&` after the
vector reallocated), then this round's `37303e3` goldens - `cb67f8c` build - `8eadc2b` SM3 shader fixes - `f7156e4`
FX hot rows - `f1d846e` RHI - `c141f2a` renderer - `6f5de3c` tolerance - `d4cc523` conformance - `4edfc32` docs -
and the `[cloud-done]` commit with this file.

| Item (round-2 task) | What changed | Commit |
| --- | --- | --- |
| 1. clang-cl and `ESIA_WERROR` | `esia_target_defaults` adds `-Wno-missing-field-initializers` for clang-cl (its `/W4` is `-Wall -Wextra`: `VkFooInfo i{sType}`, `D3D12_HEAP_PROPERTIES hp = {type}`) | `cb67f8c` |
| 1. `--check` on Windows checkouts | `.gitattributes`: everything under `src/esia/shaders/generated/` is `text eol=lf` (the SPIR-V / DXBC / DXIL blobs `binary`) | `cb67f8c` |
| 1. `build_shaders.py --fxc` | `EXT` has `dxbc_sm3`; every format is generated into a staging directory and installed only when all succeeded, the table last; the SM3 prelude reaches fxc as the fixed-name include; `--define NAME=VALUE` for A / B builds | `cb67f8c`, `8eadc2b`, `f7156e4` |
| 2.1 Image goldens | the 13 llvmpipe PNGs from `esia-opengl` (`77e936b`) cherry-picked; the docs no longer say none exist | `37303e3`, `4edfc32` |
| 2.2 Skips | `esia_conformance` exits 77 when every scene was skipped, 1 on a failure, else 0; the summary counts skips; every `esia_conformance_<backend>` CTest runs `--strict` with `SKIP_RETURN_CODE 77` | `d4cc523` |
| 2.3 Tolerance | default `maxDelta` 48 (was 128) and a `meanDelta` gate of 0.5 (why: `tests/support/image.hpp`) | `6f5de3c` |
| 2.4 Validation | `virtual std::uint32_t Device::ValidationErrors() const { return 0; }` (the null device returns its violations); a scene fails when it is non-zero after the readback | `f1d846e`, `d4cc523` |
| 2.5 New scenes | `glass_copy` (target not sampleable: every capture copies; shares `glass.png`), `srgb_msaa` (4x RGBA8_SRGB; shares `srgb_target.png`), `callback_capture` (a host callback before a capture, one with an empty clip that must not run; `glass.png`), `fx_rows` (below); `srgb_target`, `msaa_target`, `glass_copy`, `srgb_msaa` also `CopyTexture` half the target to an offset and compare the copy. No new image golden was needed (the four new scenes match the goldens they share on every backend run here); null goldens for the four, reviewed as text | `d4cc523` |
| 2.6 Scenes library | `esia_conformance_scenes` (public include `tests/conformance`, `esia::render esia_testkit`); `HeadlessDescOf` / `RenderParamsOf` give backend tests the harness's target and parameters | `d4cc523` |
| 2.7 Cross-frame | `--frames N`: N - 1 poison frames (glass and a glow layer over the whole target, 160 FX instances) on the same device, a cleared target, then the scene; CMake adds `esia_conformance_<backend>_frames` (`--frames 3 --strict`) for drawing backends | `d4cc523` |
| 2.8 FX texture rows | `RenderParams::maxFxInstancesPerRow` (a power of two); `fx_rows` = `shapes` at 2 instances per row (`shapes.png`) | `c141f2a`, `d4cc523` |
| 3.1 Pyramid levels | per frame: levels load (and are cleared once while undefined) when the planned captures exceed the budget or a user effect can sample the backdrop, else `DontCare` as before; the backdrop copy is created zero-filled | `c141f2a` |
| 3.2 Refused variants | a refused or failed FX variant draws with the full shader (`RenderStats::fxFallbacks`). The optional asynchronous protocol: `PipelineDesc::background`, `Caps::asyncPipelines`, `Device::GetPipelineStatus` (`Ready` / `Pending` / `Failed`); a batch whose variant is pending draws with the smallest ready superset variant, else the full shader (`RenderStats::fxPendingVariants`). Guide section 2, "Background pipelines" | `f1d846e`, `c141f2a` |
| 3.3 Uncopyable targets | without `CopySrc`, a sampleable target is read directly and pyramid level 1 stands in for level 0; neither: no backdrop (`time.z = 0`); null-device tests for both | `c141f2a` |
| 3.4 Host frames | `FrameDesc::hostFrame` (documented in `rhi.hpp` and the guide; the null log records it) | `f1d846e` |
| 3.5 Profiling | `RenderParams::profile` (off: frame total only) and `maxProfileScopes` (32 per frame) | `c141f2a` |
| 3.6 Small ones | a callback with an empty clip is skipped; a refused Downsample pipeline ends the pyramid; the FX texture uploads the used rows and the used part of the last one | `c141f2a` |
| 4.1 / 4.2 SM3 | the dither test uses `FX_HAS`; shape / paint kinds are plain literals (no X3203 signed / unsigned warning under the prelude's `uint` = `float`) | `8eadc2b` |
| 4.3 Prelude | `#ifdef ESIA_SHADER_PRELUDE` / `#include "esia_shader_prelude.hlsli"` (a fixed name); prepending keeps working | `8eadc2b` |
| 4.4 FX fetch | `FxVS` fetches the hot rows (rect, radii, fill0, shape, misc, flags) and passes them as flat varyings (8 in all); `FxPS` fetches every other row inside the branch that reads it; `ESIA_FX_FETCH_ALL` restores the old path; REWRITE.md section 16 corrected | `f7156e4` |
| 5. Docs | the backend guide (every item of the task's section 5, plus the new RHI calls, the async protocol and the conformance changes) and REWRITE.md | `4edfc32` |

## 2. Verified in round 2

Machine: the same Ubuntu 24.04 container **without a GPU**. Tools: clang 18.1.3 + lld, CMake 3.28.3, Ninja,
glslangValidator 15.1.0, SPIRV-Cross, spirv-val (SPIRV-Tools 2025.1), Mesa 25.2.8 (llvmpipe through EGL, lavapipe
for Vulkan), Khronos validation layer 1.3.275, Wine 9.0 with Xvfb, mingw-w64 (GCC 13 posix) for the cross build,
and Microsoft's `d3dcompiler_47.dll` 6.3.9600 (taken from the `PyQt5-Qt5` 5.15.2 win_amd64 wheel on PyPI) with a
140-line `D3DCompile` command-line wrapper written for this check (scratch, not committed): Wine's own d3dcompiler
cannot compile the FX shader.

**The core** (fresh build directories, at `4edfc32`, the code of the final commit):

| Command | Result |
| --- | --- |
| `cmake --preset linux-clang -DESIA_WERROR=ON`, build, `ctest --preset linux-clang` | 0 warnings; 8 / 8: `esia_core_tests` 38, `esia_text_tests` 10, `esia_render_tests` 42, `esia_testkit_tests` 5, `esia_text_ft_tests` 12, `esia_conformance_null` (18 scenes, `--strict`), `esia_shader_tests` 1, `esia_glsl_link_tests` 1 |
| the same with `linux-clang-release` | 0 warnings, 8 / 8 |
| `cmake --preset windows-mingw-cross -DESIA_WERROR=ON`, build | 0 warnings (`esia_text_ft` left out: no FreeType for mingw here) |
| `wine build/windows-mingw-cross/bin/<test>.exe` | core 38, text 10, render 42, testkit 5, shader 1: all pass; `esia_conformance.exe --backend null --strict`: 18 / 18 |
| `python3 tools/shaders/build_shaders.py --check` | `generated shaders are up to date` |
| `esia_conformance --backend null --golden tests/conformance/golden --strict` | 18 / 18 (Debug and Release) |
| `build_shaders.py --fxc "wine fxcw.exe"` in an `esia-directx` checkout (it has the D3D9 prelude), output to a scratch directory | runs through (6 min 40 s): `dxbc_sm3` 14 shaders (7 programs: no TextLcd without dual-source blending), `dxbc_sm4` / `dxbc_sm5` 16 each; GLSL, ESSL, MSL and SPIR-V identical to the checked-in library. Nothing of it was committed |

**The backends**: each branch's latest commit with `esia-core` merged in a scratch worktree (never pushed):

| Branch (commit) | Command | Result |
| --- | --- | --- |
| `esia-opengl` (`cc75e82`) | `linux-clang -DESIA_WERROR=ON -DESIA_BACKEND_OPENGL=ON`, build, `ctest` | 0 warnings, 13 / 13: `esia_rhi_opengl_tests` (its `ConformanceScenesWithoutGlErrors` renders the 18 scenes), `esia_conformance_opengl` / `_gles` and their `_frames` runs: 18 / 18 each, max delta <= 1, mean 0.000 |
| | `ESIA_GL_CORE_ONLY=1 esia_conformance --backend opengl --backend gles --strict` (and `--frames 3`) | 35 PASS + `text_lcd` SKIP on GLES 3.0 without `EXT_blend_func_extended`, both modes |
| `esia-vulkan` (`724df7a`) | `linux-clang -DESIA_WERROR=ON -DESIA_BACKEND_VULKAN=ON`, build, `ctest` | 0 warnings, 12 / 12: `esia_rhi_vulkan_tests`, `esia_conformance_vulkan`, `_frames` and the branch's own render-pass run: 18 / 18 each on lavapipe, max delta <= 4, mean <= 0.001, 0 validation messages (the loader inserts `VK_LAYER_KHRONOS_validation`) |
| | `ESIA_VULKAN_RENDER_PASS=1 esia_conformance --backend vulkan --strict --frames 3` | 18 / 18 |
| `esia-metal` (`eea8c0c`) | `linux-clang -DESIA_WERROR=ON -DESIA_BACKEND_METAL=ON`, build, `ctest` | 0 warnings; `esia_conformance_metal` and `_frames` **skipped** (exit 77: no Metal on Linux - before this round they "passed"); `esia_rhi_metal_tests` 23 / 25: the two failures are the follow-ups of section 5, item 5 (with those two changes, tried in the scratch copy: 25 / 25) |
| `esia-directx` (`2f150f9`) | `windows-mingw-cross -DESIA_WERROR=ON`, D3D9 / 10 / 11 / 12 on, build | 0 warnings |
| | under Wine + Xvfb (`WINEDLLOVERRIDES=d3dcompiler_47=n`): `esia_conformance.exe --backend <b> --strict`, and with `--frames 3` | d3d11, d3d10: 18 / 18 in both modes, max delta <= 2; d3d9: 17 PASS + `text_lcd` SKIP in both modes, max delta <= 2; d3d12: every scene SKIP ("creating the D3D12 device objects failed"), exit code 77 |

Wine's D3D runs through wined3d on llvmpipe (OpenGL): they exercise the backends' code paths and Microsoft's
shader compiler, not a real D3D driver. The d3d9 runs print two warnings of the backend's own log, which matter for
`ValidationErrors` (section 5, item 2).

**The shader restructure (4.4)**, checked before the renderer changes were layered on top: every scene rendered
by `opengl`, `gles`, `gles` with `ESIA_GL_CORE_ONLY=1` (FX data in a texture) and `vulkan` (validation on) is
pixel-identical to the renders before the change (max delta 0), and so is the `ESIA_FX_FETCH_ALL` build on GL; the
generated ESSL has no `texelFetch` before the first branch any more. Compiled with `d3dcompiler_47` (instruction
slots of `ps_3_0` FX variants, before -> after): `0x1` 451 -> 446, `0x822` 3509 -> 3519, `0xFF` 3990 -> 4101,
`0x1FF` and `0x3FFF` failed (out of temporary registers) -> 4298 and 5044, all features 5395; the SM3 pixel shader
reads 8 input registers. `ps_4_0` / `ps_5_0` FX: 3851 -> 3897 / 3831 -> 3832 instruction slots (the cold fetches
now sit in branches: more code, fewer fetches executed).

**The cross-frame mode catches what it should**: with the glow layer's region clear removed on purpose (in a
scratch copy), `text_lcd` still passes a single frame on llvmpipe (a fresh texture reads as zeros) and fails with
`--frames 3` (max delta 221).

## 3. Results on Windows (the local session)

**Which GPU (2026-09-30).** The local machine also has the AMD Radeon iGPU of its Ryzen, and at the time of this
check the monitor was connected to it. The headless devices do not all pick the same GPU:
- Vulkan takes the discrete GPU (the RTX 4080).
- D3D10 / 11 / 12 take the system's default adapter, D3D9 the adapter of the display, and WGL the ICD of the
  hidden window's display: all three are the GPU driving the monitor.

So results below that say "RTX 4080" for those backends are the 4080's only while the monitor was connected to it,
and that was not recorded. The conformance suite now prints each backend's adapter (`ADAPTER` lines), and
`ESIA_D3D_ADAPTER=high-performance` puts D3D10 / 11 / 12 on the 4080. On `main` after the step-3 merges, with the
debug layers on (`ESIA_D3D_DEBUG=1`), every backend passes 17 / 17 on both GPUs it can reach:
- OpenGL, GLES, D3D9, D3D10, D3D11 and D3D12 on the AMD Radeon iGPU;
- Vulkan, D3D10, D3D11 and D3D12 on the RTX 4080;
- D3D12 with GPU-based validation (`ESIA_D3D_DEBUG=2`) and `--frames 3` on both GPUs.

Windows 11, NVIDIA GeForce RTX 4080 SUPER (driver 610.88), clang-cl 22.1.8 + lld-link and MSVC 19.44, before this
round's core changes; reported in the backend branches' STATUS files and commits (`cc75e82`, `2f150f9`, `eea8c0c`),
not re-run here:

| Backend | Result |
| --- | --- |
| OpenGL, GLES (headless through WGL: Windows drivers ship no EGL) | `esia_rhi_opengl_tests` 11 / 11, 0 GL errors (KHR_debug); `esia_conformance --strict` 14 / 14 on both against the llvmpipe goldens, max delta 1 - 6 (shadows 20 on 0.003 % of the pixels), mean <= 0.09 |
| Direct3D 11, 12, 10 | 14 / 14 each, `ESIA_D3D_DEBUG=1`: no debug-layer warning or error; D3D12 GPU-based validation clean; clang-cl (`ESIA_WERROR=ON`) and MSVC (`/W4 /WX`) build all four with 0 warnings; ctest 16 / 16 |
| Direct3D 9 | 13 PASS + `text_lcd` SKIP (no dual-source blending); `msaa_target` max delta 25 on 0.018 % of the pixels; FX variants compile synchronously for up to 2.6 s each in the first frames (masks 0x822 / 0x823 / 0x826) |
| Metal (portable parts) | clang-cl 22.1.8, `ESIA_WERROR=ON`: 0 warnings, 8 / 8 tests (the fake-GPU tests drive all 14 scenes; the conformance run SKIPs Metal) |
| Vulkan | did not build with clang-cl and `ESIA_WERROR=ON` before `cb67f8c` (missing-field-initializer warnings); no Windows result recorded |

### After round 2 (the local session, same machine)

Each backend branch merged this `esia-core` and took its follow-ups (section 5); the results are in their STATUS
files and commits (`0093194` OpenGL, `eb8348c` Metal, `c31bb35` Vulkan, `60773c9` Direct3D). In short: every
backend passes all 18 scenes with `--strict`, single frame and `--frames 3`, with its API's validation counted
(`ValidationErrors`) - OpenGL and GLES (0 GL errors), Vulkan with dynamic rendering and with render passes (SDK
1.4.357 validation, 0 messages), D3D11 / 12 / 10 (debug layers, D3D12 GPU-based validation) - except D3D9's
`text_lcd` SKIP; largest delta 33 (D3D9 `srgb_msaa`, 0.016 % of the pixels). Found on the real machine: the Vulkan
loader's messages about other software's layer manifests counted as validation errors, the D3D9 backdrop copy lost
its zero upload at its first copy, and the public headers did not compile after `<windows.h>` without `NOMINMAX`
(`3a8d164`).

**FX fetch, measured** (section 1, 4.4): D3D11, conformance scenes scaled 8x to 2560 x 1920, medians of 400 frames,
three alternating runs of each build (`ESIA_FX_FETCH_ALL` for the old path), identical to 0.01 ms between runs:

| Scene | Fx, hot rows (ms) | Fx, every row (ms) | FxGlass, hot rows | FxGlass, every row |
| --- | --- | --- | --- | --- |
| shapes | 8.94 | 10.46 (-14.5 %) | - | - |
| gradients | 7.23 | 8.81 (-17.9 %) | - | - |
| shadows | 11.63 | 14.13 (-17.7 %) | - | - |
| glass | 7.70 | 8.99 (-14.3 %) | 10.29 - 10.57 | 11.74 - 11.80 (-11 %) |
| hidpi | 8.89 | 10.35 (-14.1 %) | 3.77 - 3.80 | 4.28 (-12 %) |

The capture category varies between runs (2.6 - 10 ms for the same frames) and is left out. Not measured: WGT's
legacy D3D11 backend on the same content (it takes WGT's own API), and the A / B on GL, GLES and Vulkan (their
libraries need `build_shaders.py --define ESIA_FX_FETCH_ALL=1`).

## 4. Not verified

* **No GPU here.** Nothing in this round ran on a hardware GPU. OpenGL / GLES and Vulkan ran on Mesa's CPU drivers,
  Direct3D 9 / 10 / 11 only under Wine (wined3d on llvmpipe), Direct3D 12 not at all (it does not start under
  Wine), Metal not at all (it cannot be compiled on Linux; its portable parts and mocks ran). No claim is made here
  that the D3D, Metal or Win32 code works on real drivers; section 3 is the local session's report.
* **Performance.** The FX fetch restructure (4.4) was timed on D3D11 only (section 3: FX batches 14 - 18 % faster,
  glass batches 11 - 12 %); GL, GLES and Vulkan were not A / B timed, and nothing was compared with WGT's legacy
  D3D11 backend.
* **Windows builds of this round.** The clang-cl flag (`cb67f8c`) was added without a Windows machine; the
  `windows-clang-cl`, `windows-cross` and MSVC builds of the round-2 core were not run here.
* **The new RHI features have no backend yet**: `asyncPipelines`, `hostFrame` and `ValidationErrors` are exercised
  only through the null device and its tests; the backends keep the defaults until section 5 is done.
* **The new scenes on Metal and D3D12** were not run (no machine), nor on any real GPU.

## 5. Backend follow-ups

The backend branches were not changed in this round. What they must (or may) adopt after merging `esia-core`:

1. **Direct3D 9: drop the `kPatches` entry** for `[branch] if (feat & (F_GLOW | F_SHADOW))` (`d3d9_device.cpp`):
   the shader line is fixed (`8eadc2b`), so the patch finds nothing. `d3d::Patched` (`d3d_shader.cpp`) skips a patch
   whose text is missing without a word: make it log an error (or fail the compile), so a patch never goes stale
   silently.
2. **All backends: implement `ValidationErrors()`.** OpenGL: its KHR_debug `ErrorCount`. Direct3D: the debug layer's
   messages and the D3D log's error / warning counts. Vulkan: the validation messages its messenger already counts
   (`ValidationLog::messages`). Watch Direct3D 9: under Wine the first device of a process logs "512 pixel shader
   instruction slots: liquid-glass pipelines may fail to build" (a capability notice), and the devices of the scenes
   with a backdrop copy log "a texture with uploaded contents became a copy destination: its contents were dropped"
   (7 times in a single-frame run, 17 with `--frames 3`) - the renderer now creates the backdrop copy with zero data
   (`c141f2a`), which the backend replaces with a cleared render target (the same zeros). Create `CopyDst` textures
   as render targets from the start, and keep capability notices out of the count, or those scenes fail.
3. **Direct3D 12 and Vulkan in host mode: adopt `FrameDesc::hostFrame`** for the per-frame slots they recycle
   `framesInFlight` frames later (same non-zero number = same slot; 0 = every `BeginFrame` its own frame).
4. **Metal, Vulkan (and OpenGL) unit tests: link `esia_conformance_scenes`** instead of compiling
   `tests/conformance/scenes.cpp` (`src/esia/rhi/<name>/CMakeLists.txt`), and create targets and parameters with
   `conformance::HeadlessDescOf(scene)` / `RenderParamsOf(scene)`.
5. **Metal: two tests fail after the merge** (run on Linux). `MetalMsl.VertexInputsAndOutputs`: the FX fragment
   inputs are now `[[user(locnN), flat]]`, the vertex outputs `[[user(locnN)]]` - compare the `user(locnN)` part.
   `MetalDevice.NullGoldensWithMetalCaps`: `glass_copy` needs a target that is not sampleable - create it with
   `scene.sampleable` and render with `RenderParamsOf(scene)`. Both one-line changes, checked in a scratch copy
   (25 / 25).
6. **Direct3D 9: the asynchronous variants** (guide section 2, "Background pipelines"): report `asyncPipelines`,
   compile variants on a worker (`d3d_common` already has `CompileShaderAsync` for user effects), build the full
   shader first. Keep `maxFxDataWidth` at 24 x a power of two.
7. **Vulkan: its extra CTest** `esia_conformance_vulkan_renderpass` should get `--strict`, `SKIP_RETURN_CODE 77`
   and a `--frames 3` twin, like the core's.
8. *Optional.* Direct3D 9 may serve its prelude as `esia_shader_prelude.hlsli` with `ESIA_SHADER_PRELUDE` defined
   instead of prepending it; DXBC could be generated on Linux (`build_shaders.py --fxc "wine <wrapper>.exe"` with
   Microsoft's `d3dcompiler_47.dll`, section 2), but `build_shaders.py` in core needs the SM3 prelude, which lives on
   `esia-directx`.

Still open from the backends' core requests: splitting heavy glass batches on SM3 when even their variant does not
fit (`esia-directx` request 5), a `BGR10A2_UNORM` format for iOS (`esia-metal` request 5, optional), and per-backend
CTest properties (`esia-vulkan` request 2, optional).

## 6. Known issues

1. **Legacy SDF speck.** `src/shaders/wgt_fx.hlsl` (`SdRoundRect`) keeps the zero-radius-corner defect fixed in
   Esia's copy (`8ebb8df`); the legacy shader is left untouched.
2. **sRGB targets blend in linear light** (as WGT's D3D11 backend does): translucent UI looks different on an
   `*_SRGB` target than on a UNORM one; `srgb_target` has its own golden. Multisampled sRGB captures resolve in
   linear light on GL and D3D; `srgb_msaa` stays within max delta 4 of `srgb_target` on every backend run here.
3. **SM3 size.** The full FX pixel shader is ~3.8k `ps_3_0` instruction slots (~5.5k until 2026-10-01, see the
   widget layer's second part), glass ~2k, a plain fill ~450; SM3 guarantees 512. NVIDIA's 4096 now take every
   variant and the full shader. On a device limited to 512, most variants cannot be created, and neither can the
   full shader - the fallback of refused and pending variants - so those batches are dropped (counted in
   `RenderStats::fxFallbacks`).
4. **Glass over the capture budget** reuses the last capture: the pyramid levels are now loaded rather than
   undefined, and the backdrop copy holds the previous frames' content outside this frame's captures (stale but
   defined).
5. **Text.** No Unicode bidi algorithm (right-to-left paragraphs); glyphs are rasterized before the clip test; one
   fallback chain for every language (Han unification, "Text on every platform"). Color glyphs (bitmap strikes and
   COLR fonts) and caret stops by grapheme cluster (`TextSystem::CaretStops`) exist since 2026-10-02. (The text tests'
   fonts moved to `tests/fonts` when WGT was removed.)
6. **Zero-filled uploads.** `TextureRegistry::Create(info, nullptr)` queues a zero-filled CPU copy of the whole
   texture until the renderer consumes it (4 MB for a 2048 x 2048 glyph page).
7. **No device-loss protocol.** After a lost device (D3D TDR, a lost GL context) images must be supplied again by
   their owners.
8. **Direct3D 12 after AMD's OpenGL driver in one process, with the debug layer** (found by the local session on
   the AMD Radeon iGPU of its Ryzen, driver 32.0.21045.5002). `esia_conformance --backend opengl --backend d3d12` (or
   `gles` first) with `ESIA_D3D_DEBUG=1` skips every D3D12 scene: `D3D12CreateDevice` returns
   `DXGI_ERROR_DEVICE_RESET`. A small program reproduces it without Esia. It happens only when a WGL context of
   AMD's driver (`atio6axx.dll`) exists and the D3D12 debug layer is enabled before creating a device on the AMD
   adapter. The same sequence gives a working device without the debug layer, or on the RTX 4080
   (`ESIA_D3D_ADAPTER=high-performance`); a D3D12 device made before enabling the layer is no problem either. A
   driver issue, not Esia's. CTest runs each backend in its own process, so the suite never meets it.
9. **PingFang through Core Text on macOS 15.** On GitHub's macos-15 runners, four ideographs in PingFang SC at 32 px
   (the `coretext:` path, `src/esia/text/ft/platform_face.cpp`) measure 130.69 px, against Core Text's own advances of
   128 px (2.1 % wider); on macOS 27 the two agree exactly. Not explained without a macOS 15 machine: the variation
   axes `CTFontCopyVariation` reports, as HarfBuzz applies them through HVAR, are the first suspect.
   `FreeType.SystemFontsCoreTextDraws` holds it within 3 % before macOS 26 and to 0.5 px from 26 on.

## 7. Building and testing

### Linux (what the cloud sessions use)

```bash
sudo apt-get install -y clang lld cmake ninja-build python3 \
    glslang-tools spirv-cross spirv-tools \
    libegl-dev libgles-dev libgl-dev mesa-utils \
    libfreetype-dev libharfbuzz-dev libfontconfig-dev   # optional: without them FreeType / HarfBuzz are built from source
cmake --preset linux-clang && cmake --build --preset linux-clang && ctest --preset linux-clang
# release: linux-clang-release; warnings as errors: -DESIA_WERROR=ON; a backend: -DESIA_BACKEND_OPENGL=ON
python3 tools/shaders/build_shaders.py          # after a shader change: regenerate the library (commit the output)
python3 tools/shaders/build_shaders.py --check  # CI: the checked-in library matches the sources
build/linux-clang/bin/esia_conformance --backend null --golden tests/conformance/golden --strict
```

Without `libegl-dev` the GLSL link test is left out. FreeType and HarfBuzz come from the system when both are found and
are built from source otherwise (`-DESIA_TEXT_DEPS=bundled` always, `system` never; the first configure downloads them).
Vulkan on the CPU: `apt-get install mesa-vulkan-drivers libvulkan-dev vulkan-validationlayers` (lavapipe). The D3D
backends under Wine: `apt-get install wine64 xvfb`, build `windows-mingw-cross`, put Microsoft's
`d3dcompiler_47.dll` next to the executables, then `WINEDLLOVERRIDES=d3dcompiler_47=n xvfb-run -a wine
build/windows-mingw-cross/bin/esia_conformance.exe --backend d3d11 --golden tests/conformance/golden --strict`.

### Windows with LLVM

* **On Windows**: the LLVM installer (`clang-cl`, `lld-link` on `PATH`) plus the MSVC build tools and Windows SDK for
  the STL, CRT and headers; from an "x64 Native Tools Command Prompt":
  `cmake --preset windows-clang-cl && cmake --build --preset windows-clang-cl && ctest --preset windows-clang-cl`.
  FreeType and HarfBuzz are built from source unless an installation of both is found. For vcpkg's
  (`vcpkg install freetype harfbuzz --triplet x64-windows`) use its toolchain file,
  `-DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake` (plus
  `-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=cmake/toolchains/clang-cl-windows.cmake` for clang-cl) and
  `-DESIA_TEXT_DEPS=system`: vcpkg's harfbuzz config breaks under a plain `CMAKE_PREFIX_PATH` (docs/CI.md). The local
  session used clang-cl 22.1.8 and MSVC 19.44.
* **From Linux, MSVC ABI** (`windows-cross`): `cargo install xwin --locked`, then
  `xwin --accept-license --arch x86_64 splat --output ~/.xwin` (needs download.visualstudio.microsoft.com),
  `cmake --preset windows-cross -DXWIN_DIR=$HOME/.xwin && cmake --build --preset windows-cross` (tests off: they
  cannot run on Linux).
* **From Linux, offline** (`windows-mingw-cross`, GNU ABI - a compile / link check of the same code, see the guide
  for its `type_info::operator==` trap): `sudo apt-get install g++-mingw-w64-x86-64-posix mingw-w64-x86-64-dev
  wine64`, then `cmake --preset windows-mingw-cross && cmake --build --preset windows-mingw-cross` and, optionally,
  `wine build/windows-mingw-cross/bin/esia_core_tests.exe` (and the other test executables).
* **MSVC**: `cmake --preset windows-msvc` (Visual Studio 2022), `cmake --build --preset windows-msvc-release`.

### macOS

`brew install llvm lld cmake ninja glslang spirv-cross spirv-tools` (`freetype harfbuzz` optional), then
`cmake --preset macos-clang && cmake --build --preset macos-clang && ctest --preset macos-clang` (not tried here).

## 8. Round 1: the core

| Scope item | Where | What |
| --- | --- | --- |
| 1. Architecture | `docs/REWRITE.md` | layers and boundaries, the frame end to end, the UI core, renderer, RHI contract, every FX feature per API with each family's limits, the shader strategy decision, text, platform, threading, migration of the `wgt::` API, toolchain, testing, phases, risks |
| 2. UI core without ImGui | `include/esia/{base,core}`, `src/esia/core` | ids (FNV-1a id stack, `##` / `###` labels), thread-safe input queue and input state, context (windows with layers and z-order, focus, move / resize, items with hover / active arbitration, layout cursor, groups, clipping, scrolling, keyboard focus), draw lists with the FX stream, the texture registry, UTF-8 decoding |
| 3. RHI | `include/esia/rhi`, `src/esia/rhi` | `rhi::Device` (resources, frames, passes that start with nothing bound, fixed binding model, copies, readback, profiling, host callbacks, validation counts, background pipelines), `Caps` instead of API versions, `RawFormat`, the backend registry, the null device that records the command stream and enforces the call-order and binding rules |
| 4. Renderer | `include/esia/render`, `src/esia/render`, `src/esia/shaders` | `Painter` (WGT's, byte-identical `fx::Instance`), `FramePlan` (batching, dirty rectangles, look-ahead capture merging, levels per capture), `Renderer` (region-limited captures, the direct render-target read, the pyramid, B-spline frost sampling, glow layers with region clears, lazy passes, FX feature variants, GPU profile categories); one HLSL source, SPIR-V / GLSL 330 / ESSL 300 / MSL generated by `tools/shaders/build_shaders.py` and checked in, the HLSL embedded for runtime compilation, the Direct3D 9 hooks |
| 5. Conformance suite | `tests/conformance`, `tests/support` | 17 scenes (14 in round 1, 18 in round 2; round 3 removed `text_lcd`) through the public API, image goldens and null command-stream goldens, a self-contained PNG codec and comparison |
| 6. Backend guide | `docs/backends/README.md` | files and CMake, host integration headers, the RHI call by call, caps per API, binding numbers, recipes for every API, the LLVM toolchains, the conformance suite, a checklist |
| 7. Text (stretch) | `include/esia/text`, `src/esia/text` | `text::TextSystem`; WGT's analytic rasterizer made platform-free; the glyph atlas on the texture registry; `esia_text_ft`: FreeType + HarfBuzz shaping, fallback, line breaking, trimming, alignment, with two golden images |
| Toolchains | `CMakePresets.json`, `cmake/toolchains` | `linux-clang`, `linux-clang-release`, `macos-clang`, `windows-clang-cl`, `windows-cross` (clang-cl + lld-link + xwin), `windows-mingw-cross` (clang + mingw-w64); `windows-msvc` since WGT was removed |

Round-1 commits (oldest first): `51d88dc` presets, toolchains, UI core - `4f36cef` RHI, null backend, shader library -
`55e04ae` delta time from a clock at 0 - `c8a3372` pass and color contract, host callbacks - `c845be3` Painter, planner,
renderer - `8ebb8df` SDF fix - `2283d72` conformance suite - `94d89bf` D3D9 hooks, FX feature variants - `1f1443a`
mingw-w64 cross build - `653dd4f` shader library and GLSL / ESSL link tests - `9d7fd4b` ESSL at highp - `4c9af5c`
runtime shader sources - `4c35054` clip-position hook, threading rules - `871a86c` REWRITE.md and the guide - `d3768bb`
rasterizer, atlas - `09f4b27` FreeType + HarfBuzz - `331bddc` the null goldens tracked - `79c12ef` `[cloud-done]`.

## 9. Next

The backends are merged into `main` and the widgets are ported (WGT and its `wgt::` compatibility layer were dropped;
tag `wgt-1.1-final`). What comes next:

1. **Platform layers**: Cocoa and UIKit as libraries (the examples' frames do their work today), Wayland, inline IME
   composition on Linux and on Android (a GameActivity, or an input connection of its own: a NativeActivity gets key
   events only).
2. **Text**: the bidi algorithm (right-to-left paragraphs), variable-font instances (SF's weights), per-language
   fallback and fallback fonts loaded on demand.
3. **Android on devices**: arm64 phones (Adreno, Mali), Android versions before 15, the Vulkan validation layer in
   debug APKs.
4. **Core follow-ups**: device loss (section 6, item 7), empty-pixel creates (item 6), splitting heavy SM3 glass
   batches, DXBC generated and checked in.
5. An API reference and vcpkg / Conan ports (the CMake package exists: section "Packaging"); a measurement against
   WGT's legacy D3D11 backend on the same UI.
