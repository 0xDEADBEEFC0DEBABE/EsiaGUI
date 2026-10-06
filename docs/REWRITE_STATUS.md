# Esia rewrite - status of `esia-core`

Where the ImGui-free rewrite stands after core round 3 and the UI core's second version (2026-09-29). The plan is [REWRITE.md](REWRITE.md); how to
write a backend is [backends/README.md](backends/README.md).

**Round 3.** By the owner's decision, sub-pixel (LCD / ClearType) text was removed from Esia: text is antialiased
in grayscale everywhere (section 0). The backends must delete their dual-source code after merging (section 0,
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
