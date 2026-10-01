# Esia

<p align="center">
  <img src="docs/images/showcase-dark.jpg" width="100%"
       alt="The showcase in the dark theme: Settings, Effects Lab, Components, Languages, Control Center, Telemetry and a plugin panel as liquid-glass windows over the wallpaper, the island at the top, the dock at the bottom">
</p>
<p align="center">
  <img src="docs/images/showcase-light.jpg" width="32%" alt="The same panels in the light theme">
  <img src="docs/images/glass-closeup.jpg" width="32%" alt="Effects Lab: clear, regular, thick and tinted glass over moving content">
  <img src="docs/images/control-center-closeup.jpg" width="32%" alt="Control Center: glass toggles and the media card">
</p>
<p align="center"><sub>The showcase on Windows 11 with Direct3D 11: every panel open, dark and light theme, 2808 x 2100 pixels
at UI scale 1.5 (<code>showcase --spread --size 1872x1400 --scale 1.5</code>). The background is the system's wallpaper.</sub></p>

A liquid-glass (iOS 26 style) UI toolkit for games and tools, in C++20: its own UI core, widgets, renderer and render
hardware interface (RHI), with backends for Direct3D 9 to 12, OpenGL / OpenGL ES, Vulkan and Metal, on Windows, Linux,
macOS, iOS and Android. It replaces WGT UI, the Dear ImGui based `wgt.dll`; WGT's last version is tag
`wgt-1.1-final`, in this repository's history.

* [Esia and Dear ImGui](#esia-and-dear-imgui) · [Widgets for tools](#widgets-for-tools-the-workbench) ·
  [Status](#status) · [Getting it](#getting-it) · [Building](#building) · [Running the examples](#running-the-examples)
* Per system: [Windows](#windows) · [Linux](#linux) · [macOS](#macos) · [iOS](#ios) · [Android](#android)
* [Changing the shaders](#changing-the-shaders) · [Documents](#documents) · [Roadmap](#roadmap) · [Third-party](#third-party)

## Esia and Dear ImGui

Esia began as WGT, a liquid-glass layer on a modified Dear ImGui, and then replaced ImGui with a core of its own. It
keeps ImGui's model, so its code reads much like ImGui code:

* it is immediate mode: a widget is a function call each frame and returns true when it changed something;
* ids come from labels, with the same `##` convention;
* windows are `BeginWindow` / `EndWindow`.

What differs is what it draws, how it draws it, and what that costs. There is no compatibility layer: moving a panel
from ImGui to Esia means writing it again with `esia::ui`.

| | Dear ImGui | Esia |
| --- | --- | --- |
| Made for | tools: debug panels, editors, dense data | UI that users see, in the iOS 26 liquid-glass style |
| Look | flat panels and one global style (`ImGuiStyle`, with push / pop of colors and variables) | liquid glass (backdrop blur, refraction, dispersion, specular rims), glow and shadows; a theme whose light and dark modes cross-fade; styles per widget or per block; three glass looks |
| Motion | none: a change shows on the next frame | springs on every control: the switch's knob, the segmented control's lens, momentum scrolling, panels, the island |
| Rendering | hands the host triangles (`ImDrawData`), drawn in one pass by any renderer that can draw textured triangles with a scissor | its own renderer, several passes per frame (backdrop captures, a blur pyramid, glow layers), through its RHI on Direct3D 9 - 12, OpenGL / ES, Vulkan or Metal |
| GPU cost | a fraction of a millisecond | real, because the glass reads back and blurs what is behind it. The showcase with seven panels at 2808 x 2100 takes 0.9 ms on an RTX 4080 SUPER and runs at about 170 fps at 3024 x 1890 on an M3 Pro; each backdrop capture breaks the render pass, which tile-based GPUs feel most |
| Text | its font atlas (stb_truetype, or FreeType). Text is not shaped, so scripts that need shaping (Arabic, Devanagari and the other Indic scripts) do not come out right, and there is no right-to-left | FreeType + HarfBuzz shaping, the system's fonts with a fallback chain for every major script, editing by grapheme, IME composition in the field (Windows, iOS), color emoji (bitmap and COLR fonts: Apple Color Emoji, Segoe UI Emoji, Noto Color Emoji). A line in one direction is right; mixed directions wait for the bidi algorithm |
| Widgets | a large set: tables, trees, number inputs and drags, color pickers, plots, multi-line text, docking, multiple viewports; extensions add more (ImPlot, node editors) | iOS-style: buttons, switches, sliders, steppers, segmented controls, inset grouped lists, navigation stacks, tab and search bars, menus, pickers, tooltips, text fields, a line chart, the island and the dock. For tools: number and vector fields (scrub, type an expression), a color picker, tables (only the rows in view are drawn: a million rows; sort, resize, select), trees, a multi-line editor, docking. No plots beyond the line chart, no node editors, no multiple viewports |
| Integration | copy a few files; platform and renderer backends exist for almost everything (Win32, GLFW, SDL, Android, Emscripten ...) | a CMake project that needs FreeType and HarfBuzz (found, or downloaded and built by the configure step). The host creates an Esia device on its own device or context and passes a render target each frame. Windows, Linux (X11) and Android have platform layers; on macOS and iOS the host queues the input events itself, as the examples' frames do |
| Touch | touch arrives as the mouse; dragging content does not scroll it | a phone's behavior: a finger scrolls a list from anywhere, a row or a slider included, without pressing them; a tap presses, a sideways drag moves a slider. The showcase lays itself out for a phone |
| Platforms | anywhere that can draw triangles: desktop, mobile, web, consoles | Windows, Linux, macOS, iOS, Android |
| Threads | a context is used from one thread at a time | one UI thread per context, no global state; the input queue and the texture registry are thread-safe; worker threads post to the UI (the island's notifications) |
| Maturity | more than ten years old, used in a great many games and engines, with extensions (ImPlot, node editors), bindings for many languages and a stable API | new in 2026: the API can still change, there is no package yet (`find_package` is on the roadmap), and one team works on it |

**Strengths of Esia:**

* how the UI looks and moves;
* correct text in every major script, with the system's fonts;
* styling per component rather than one global style;
* one renderer for Direct3D, OpenGL, Vulkan and Metal, tested against the same golden images on each;
* the same UI on a desktop and a phone, touch included.

**Weaknesses of Esia:**

* GPU cost;
* a smaller widget set: the widgets for tools are new, and there are no plots (beyond a line chart), node editors or
  multiple viewports;
* heavier integration: FreeType and HarfBuzz, and no platform layer library on macOS and iOS yet;
* no web and no consoles;
* youth.

**Use Esia** for the UI users see: a game's menus and overlays, a launcher, an application's settings, a tool that
should look like a product. It fits where the glass, the motion and the text matter and about a millisecond of GPU
time is affordable.

**Use Dear ImGui** for internal tools and debuggers that live on ImGui's ecosystem (plots, node editors, multiple
viewports), the smallest integration and GPU cost, or a platform Esia does not run on (the web, consoles).

## Widgets for tools: the workbench

<p align="center">
  <img src="docs/images/workbench.jpg" width="49%"
       alt="The workbench: an outline tree, a scene view, an inspector with number, vector and color fields, and a table of 100 000 assets, docked side by side">
  <img src="docs/images/workbench-floating.jpg" width="49%"
       alt="The workbench with the inspector dragged out as a floating glass window over the docked panels, the script editor in front">
</p>
<p align="center">
  <img src="docs/images/inspector-dark.jpg" width="24%"
       alt="The inspector in the dark theme: vector fields for position, rotation and scale, an opacity field, a stepper, a switch, the color picker's spectrum, hue and opacity bars, the hex value and swatches">
  <img src="docs/images/inspector-light.jpg" width="24%" alt="The same inspector in the light theme">
</p>
<p align="center"><sub>The workbench on Windows 11 with Direct3D 11 at UI scale 1.5: docked; with the inspector
floating over the dock space and the script editor's tab in front; the inspector's number, vector and color fields in
the dark and the light theme (<code>workbench --size 1280x800 --scale 1.5 [--float Inspector --show Script] [--light]</code>).</sub></p>

The data widgets, in the same glass style as the rest (`esia::ui`, [UI_WIDGETS.md](docs/UI_WIDGETS.md) section 12):

* **Docking.** Drag a window by its header over a dock space: glass targets show where it would go (into a node's
  tabs, or beside it) and it docks when let go. Drag a tab out and the window floats again, still under the pointer;
  the lines between the panels resize them. Docked windows are ordinary `BeginWindow` code; the layout saves and
  loads as text.
* **Tables** with a million rows cost what the rows in view cost: sortable and resizable columns, rows selected with
  a click, Ctrl and Shift, icons in cells, any widget in a cell.
* **Trees**: an arrow that turns, children that slide open with a guide line, icons and details.
* **Number and vector fields**: drag sideways to scrub, click to type a value or an expression (`2*(3+4)`), Up and
  Down to step; a field with limits fills in proportion to its value.
* **Color picker**: a spectrum, hue and opacity bars, the hex value and swatches, or a round color well that opens it.
* **Multi-line editor**: wrapping, line numbers, the monospace font, Tab, undo, the clipboard and IME composition.

```
workbench                                  # the docked layout (glass_window's options apply too)
workbench --float Inspector --show Script  # a window floating, the editor's tab in front
workbench --rows 1000000 --light           # a million rows, light theme
```

## Status

| Part | State |
| --- | --- |
| UI core (`src/esia/core`) | second version ([UI_CORE.md](docs/UI_CORE.md)): ids, input (touch included), windows with per-window DPI, front-to-back hit testing, press and key ownership, containers and layout providers, child scroll regions, popups and tooltips, per-id state, draw lists, texture registry |
| Widgets (`src/esia/ui`, `esia::ui`) | WGT's liquid-glass widgets on the core ([UI_WIDGETS.md](docs/UI_WIDGETS.md)): themes, per-widget styles, springs, auto layout, controls, lists, navigation, popups, text fields, charts, the island; and widgets for tools: number, vector and color fields, tables, trees, a multi-line editor, docking |
| Renderer (`src/esia/render`) | Painter, frame planner, liquid glass (backdrop captures, blur pyramid, refraction), glow layers, edge fades, GPU profiling |
| Text (`src/esia/text`) | analytic glyph rasterizer, glyph atlas, FreeType + HarfBuzz text system (bundled or the system's), system font lookup with fallback chains for every major script, color emoji from bitmap strikes (sbix, CBDT) and COLR fonts (gradients, transforms, composite modes), grayscale antialiasing |
| Backends (`src/esia/rhi/<name>`) | DirectX (Direct3D 9 / 10 / 11 / 12, in `rhi/directx`), OpenGL 3.3 / OpenGL ES 3.0, Vulkan 1.1+, Metal |
| Platform | layers as libraries for Win32 (`src/esia/platform/win32`, [PLATFORM_WIN32.md](docs/PLATFORM_WIN32.md)), X11 (`src/esia/platform/x11`) and Android (`src/esia/platform/android`); the examples' own frames on AppKit (macOS) and UIKit (iOS) |
| Examples | `showcase` (WGT's demo on Esia), `workbench` (a tool's layout on the data widgets) and `glass_window` (a smoke test of the whole path) |

| Backend | CMake option | Systems | Verified on |
| --- | --- | --- | --- |
| DirectX: Direct3D 11, 12, 10, 9 | `ESIA_BACKEND_DIRECTX` | Windows | Windows 11, NVIDIA RTX 4080 SUPER, debug layers (D3D12 GPU-based validation); Wine |
| OpenGL, OpenGL ES | `ESIA_BACKEND_OPENGL` | Windows, Linux, Android | NVIDIA (WGL), Mesa llvmpipe (EGL), VMware SVGA3D (EGL on X11), the Android 15 emulator (OpenGL ES) |
| Vulkan | `ESIA_BACKEND_VULKAN` | Windows, Linux, Android | NVIDIA and Mesa lavapipe, Khronos validation layer; the Android 15 emulator |
| Metal | `ESIA_BACKEND_METAL` | macOS, iOS | MacBook Pro (M3 Pro, macOS 27), GitHub's arm64 runner (macOS 15), iPhone 18 Pro Max (A20 Pro, iOS 27); Metal API and shader validation |

Every backend passes the conformance suite (17 scenes against golden images, single frame and across frames, with
the API's validation counted); details and numbers in [REWRITE_STATUS.md](docs/REWRITE_STATUS.md) and each backend's
`STATUS.md`.

| System | Library and tests | Example window |
| --- | --- | --- |
| Windows 10 / 11 | yes | yes: DirectX, OpenGL, Vulkan |
| Linux | yes | yes: OpenGL, Vulkan (X11, XWayland on Wayland) |
| macOS 11+ | yes | yes: Metal |
| iOS 16+ | yes | yes: Metal |
| Android 9+ (API 28) | yes (the emulator) | yes: OpenGL ES, Vulkan (APKs) |

## Getting it

Clone the branch of your graphics API: it holds that backend and what builds it on every system it runs on (the
example frames, the tests, the presets), and its README has the commands.

| Branch | Backend | Systems |
| --- | --- | --- |
| `directx` | Direct3D 11, 12, 10, 9 (the first that works on the machine, or `--api d3d12` ...) | Windows |
| `opengl` | OpenGL 3.3 / OpenGL ES 3.0 | Windows, Linux, Android |
| `vulkan` | Vulkan 1.1+ (the Vulkan SDK / headers) | Windows, Linux, Android |
| `metal` | Metal | macOS, iOS |

```
git clone -b directx https://github.com/0xDEADBEEFC0DEBABE/EsiaGUI.git
```

The branches are made from `main` after every push to it (`tools/branches/backend_branches.py`, the `Backend branches`
workflow): do not commit to them. `main` has every backend, the development history and the documents; it builds the
same way.

## Building

Every system builds with a CMake preset, and the backends of the system are on by default: nothing to choose first.
The commands and what to install are in each system's section: [Windows](#windows), [Linux](#linux), [macOS](#macos),
[iOS](#ios), [Android](#android).

| System | Presets | Backends built by default |
| --- | --- | --- |
| Windows | `windows-msvc`, `windows-clang-cl` | DirectX (Direct3D 9, 10, 11, 12) and OpenGL; Vulkan too when the Vulkan SDK is installed |
| Linux | `linux-clang`, `linux-clang-release`; `windows-mingw-cross`, `windows-cross` build the Windows backends | OpenGL; Vulkan too when the Vulkan headers are installed |
| macOS | `macos-clang` | Metal |
| iOS | `ios`, `ios-simulator` | Metal |
| Android (from any of them) | `android` | OpenGL ES and Vulkan |

* Configuring prints the list (`-- Esia backends: ...`). `-DESIA_BACKEND_<NAME>=ON` or `OFF` adds or drops one
  (`DIRECTX`, `OPENGL`, `VULKAN`, `METAL`); with DirectX on, the advanced `ESIA_BACKEND_D3D9` / `D3D10` / `D3D11` /
  `D3D12` leave out single Direct3D versions.
* `-DESIA_WERROR=ON` turns warnings into errors (every preset builds without warnings).
* Text: with `ESIA_TEXT_FREETYPE` on (the default), FreeType and HarfBuzz come from the system or are downloaded and
  built at configure time (`ESIA_TEXT_DEPS=auto|bundled|system`).
* Every preset puts the examples in `build/<preset>/bin` (`showcase`, `workbench`, `glass_window`; Android: APKs in
  `build/android/apk`); `ctest --preset <preset>` runs the tests, the conformance suite on the machine's GPU included.

## Running the examples

```
showcase                                   # Settings, Effects Lab and Control Center over a wallpaper, the dock, the island
showcase --open-all --light                # every panel, light theme
showcase --vsync off                       # uncapped frame rate
workbench                                  # the widgets for tools in a dock space
glass_window                               # the smoke test: glass, the core's windows, text and input
```

These options work on every system with an example window; each system's section lists its own.

| Option | Meaning |
| --- | --- |
| `--api name` | the backend, where more than one is built: see [Windows](#windows) and [Linux](#linux) |
| `--size WxH`, `--scale s` | client area in UI units, and pixels per UI unit (default: the monitor's); `--size 1512x945 --scale 2` is 3024 x 1890 pixels |
| `--vsync on \| off` | off: no frame cap |
| `--frames N`, `--screenshot out.png`, `--fixed-dt s` | quit after N frames, write the last one, advance the UI clock by `s` per frame (deterministic captures) |
| `--debug` | the API's validation layer; the message count is printed at exit |
| `--font file` | font files instead of the system's (the first is the main one) |
| showcase: `--open list`, `--open-all`, `--spread`, `--light`, `--look theme \| clear \| frosted`, `--tab n`, `--page id` | panels to open (`settings,effects,control,components,languages,telemetry,plugin`), every panel side by side (in a window of 1872 x 1400 UI units), theme, glass look, the Components tab, the Settings page |
| workbench: `--float window`, `--show window`, `--rows N`, `--light` | a window floating instead of docked (`Inspector`, `Outline` ...), the tab in front, the table's row count, theme |

## Windows

**Build.** With Visual Studio 2022 and nothing else (or open the folder in Visual Studio: it lists the presets):

```
cmake --preset windows-msvc
cmake --build --preset windows-msvc-release
```

The examples are in `build\windows-msvc\bin\Release`. With LLVM instead (clang-cl and lld on PATH, from an "x64 Native
Tools" prompt): `cmake --preset windows-clang-cl`, `cmake --build --preset windows-clang-cl`,
`ctest --preset windows-clang-cl`.

**Things to know on Windows**

* **Path length.** Visual Studio's build fails on paths over 260 characters: keep the clone's path under about 140
  characters (`C:\Users\<you>\source\repos\EsiaGUI` is fine), or enable long paths in Windows.
* **Choosing the API.** `--api directx` (the default) starts the first Direct3D version that works on the machine: 11,
  then 12, 10, 9. `--api d3d11 | d3d12 | d3d10 | d3d9 | opengl | vulkan` picks one. The first line printed (and the
  showcase's status bar) names the API and the GPU that started.
* **Vulkan** needs the LunarG Vulkan SDK (https://vulkan.lunarg.com; its installer sets `VULKAN_SDK`) at configure
  time; without it the build has no Vulkan backend and says so.
* **Debug layers.** `--debug` turns on the Direct3D debug layer (D3D12 with GPU-based validation), which comes with
  Windows' optional feature "Graphics Tools"; for Vulkan the SDK's Khronos validation layer.
* **Custom HLSL effects** (`Renderer::SetEffectSource`, the showcase's "HLSL effect" and "Holo Card") are compiled at
  run time by `d3dcompiler_47.dll`, part of Windows: only the Direct3D backends run them; OpenGL and Vulkan draw those
  shapes with the built-in shader.
* **Fonts and icons.** The UI uses Segoe UI with the system's fallback chain (Microsoft YaHei, Yu Gothic UI, Malgun
  Gothic, Nirmala UI ...), Segoe Fluent Icons (Windows 11) or Segoe MDL2 Assets (Windows 10) for the icons; the showcase
  draws Windows' default wallpaper. Color emoji come from Segoe UI Emoji, a COLR font painted with its gradients.
* **Touch screens.** A finger scrolls lists from anywhere, rows and sliders included, as on a phone (Windows marks
  the mouse messages a touch makes); the mouse keeps the desktop's behavior.
* **Wine.** The Direct3D backends run under Wine (screenshots in `examples/glass_window/wine_screenshots`).
* **Cross-compiling from Linux:** `cmake --preset windows-mingw-cross` (MinGW-w64), or `windows-cross` (clang-cl with
  the Windows SDK from xwin).

## Linux

<p align="center"><img src="docs/images/showcase-ubuntu.jpg" width="80%" alt="The showcase on Ubuntu 24.04 over Ubuntu's wallpaper, with the desktop's symbolic icons"></p>
<p align="center"><sub>The showcase on Ubuntu 24.04 (GNOME, XWayland), OpenGL.</sub></p>

**Build.** clang, lld, Ninja and CMake; X11 for the examples' window, EGL for OpenGL, the Vulkan headers for Vulkan
(Ubuntu 24.04):

```
sudo apt install clang lld ninja-build cmake libfreetype-dev libharfbuzz-dev libx11-dev libegl-dev libvulkan-dev libjpeg-dev
cmake --preset linux-clang
cmake --build --preset linux-clang && ctest --preset linux-clang
```

The examples are `build/linux-clang/bin/showcase` and `glass_window` (`linux-clang-release` for an optimized build).

**Things to know on Linux**

* **The window** is an X11 window (the X11 platform layer `esia_platform_x11`, used by
  `examples/glass_window/app_linux.cpp`); on a Wayland desktop (Ubuntu's and Fedora's default) it runs through
  XWayland. `--api opengl` (the default: EGL on the window) or `--api vulkan` (`VK_KHR_xlib_surface`). The UI scale
  follows the desktop's scaling through `Xft.dpi`, or `--scale`.
* **Input.** Mouse, wheel, keyboard and the clipboard (`CLIPBOARD`). Text comes through the input method (IBus, Fcitx
  over XIM): what it commits arrives, but the composition is not shown inline yet.
* **Icons.** esia::ui's icons are drawn from the desktop's symbolic icon theme (Adwaita, else Yaru), rasterized by
  librsvg, which every GNOME desktop has; it is loaded at run time, so nothing is needed to build. Without it the icons
  are left out.
* **Fonts.** The text system finds fonts through fontconfig when it is installed at configure time
  (`libfontconfig-dev`; `-DESIA_TEXT_FONTCONFIG=OFF` to not use it), otherwise by scanning the standard font
  directories. The UI font is Noto Sans (else DejaVu Sans), with Noto Sans CJK, WenQuanYi Zen Hei or Droid Sans
  Fallback for Chinese, Japanese and Korean, a Noto font for every other script (Arabic, Hebrew, the Indic scripts,
  Thai ...) and Noto Color Emoji: install `fonts-noto-core fonts-noto-cjk fonts-noto-color-emoji` for all of them.
* **Wallpaper.** The showcase draws Ubuntu's default wallpaper, else GNOME's (`/usr/share/backgrounds`); JPEG ones
  need `libjpeg-dev` at build time.
* **Headless.** The tests need no window system: OpenGL runs through EGL without one (Mesa's llvmpipe, or the GPU's
  driver), Vulkan without a surface (lavapipe or the GPU's driver), so they run on servers and in CI. `--debug` and the
  tests use the Khronos validation layer when it is installed (`vulkan-validationlayers`).
* **Virtual machines.** In VMware, OpenGL needs 3D acceleration and virtual hardware 13 or later: SVGA3D then offers
  OpenGL 4.3 core; with older hardware versions it offers 2.1 and the OpenGL host refuses it. VMware has no Vulkan
  driver: `--api vulkan` runs on lavapipe, on the CPU.
* **Custom HLSL effects** need Direct3D: on Linux those shapes draw with the built-in shader.

## macOS

<p align="center"><img src="docs/images/showcase-macos.jpg" width="80%" alt="The showcase on macOS over the system's wallpaper: Settings, Effects Lab and Control Center as liquid-glass windows, the island at the top, the dock at the bottom, SF Symbols for the icons"></p>
<p align="center"><sub>The showcase on a MacBook Pro (M3 Pro, macOS 27), Metal: 2400 x 1500 pixels at UI scale 1.5
(<code>showcase --size 1600x1000 --scale 1.5</code>).</sub></p>

**Build.** Apple silicon or Intel, macOS 11 or later; Xcode (the SDK and the Metal compiler) and Homebrew:

```
brew install llvm lld ninja cmake freetype harfbuzz
xcodebuild -downloadComponent MetalToolchain     # only if `xcrun metal` says the Metal toolchain is missing
cmake --preset macos-clang
cmake --build --preset macos-clang && ctest --preset macos-clang
```

The examples are `build/macos-clang/bin/showcase` and `glass_window`.

**Things to know on macOS**

* **Validation.** `ctest` runs the Metal conformance suite on the Mac's GPU; set
  `MTL_DEBUG_LAYER=1 MTL_SHADER_VALIDATION=1` for Metal's API and shader validation (CI does), or pass `--debug` to the
  examples.
* **Frame rate.** A window cannot present faster than the display: with `--vsync off` frames render into offscreen
  targets as fast as they can and the newest is presented each refresh.
* **Options of their own:** `--stats` prints once a second the frame rate, UI / encode / wait times, the GPU time of the
  frame's command buffer, passes, backdrop captures, frames shown and dropped. `--fullscreen`, `--fullscreen-at N`,
  `--hide-at N` start in full screen, enter it after frame N, or hide the app after frame N (tests: the frame loop
  keeps its speed hidden, it declares itself latency critical so macOS does not nap it).
* **Fonts and icons.** The examples use the system fonts (SF, the fallback chain of `system_fonts.hpp`, Apple Color
  Emoji), SF Symbols for esia::ui's icons (Windows' icon fonts are not there) and a system wallpaper. The fonts
  FreeType cannot read (PingFang's `hvgl` glyphs) go through Core Text.
* **Performance** (showcase, MacBook Pro M3 Pro, 3024 x 1890 pixels at UI scale 2, `--vsync off`): about 170 fps (the
  Fx shader is specialized per batch through Metal function constants), about 180 fps in the default 2560 x 1600
  window. The GPU's clock follows its temperature and load: a hot laptop or another app drawing can halve these for a
  while. The time goes to the glass panels' pixels, the drop shadows around them, and about 15 backdrop captures per
  frame, each of which ends and resumes the render pass. Measure with `--stats` or Instruments' Metal System Trace.
* **Limitations.** Custom HLSL effects need Direct3D: Metal draws those shapes with the built-in shader. SF is a
  variable font and the FreeType text system loads its default instance: the UI's bold weights draw as regular.

## iOS

<p align="center">
  <img src="docs/images/ios-settings.jpg" width="30%" alt="The showcase's Settings panel on an iPhone, under the Dynamic Island, the status bar and the dock below it">
  <img src="docs/images/ios-control-center.jpg" width="30%" alt="The Control Center on an iPhone: glass toggles, the media card, sliders and the Focus module">
  <img src="docs/images/ios-languages.jpg" width="30%" alt="The Languages panel on an iPhone: Chinese in PingFang, Japanese, Korean, Arabic, Hebrew, Hindi, Thai and color emoji">
</p>
<p align="center"><sub>The showcase on an iPhone 18 Pro Max (iOS 27), Metal: Settings, Control Center, Languages, screenshots
taken on the phone.</sub></p>

**Build** (iPhones and iPads, iOS 16 or later; on a Mac with Xcode). The examples become app bundles, signed after the
link with a provisioning profile Xcode has for them (`tools/ios/codesign.py`: sign in to Xcode with your Apple ID; a
development or ad hoc profile that lists the device). With the iPhone connected and unlocked:

```
cmake --preset ios && cmake --build --preset ios
xcrun devicectl list devices
xcrun devicectl device install app --device "<name>" build/ios/bin/showcase.app
xcrun devicectl device process launch --console --device "<name>" org.esia.showcase --stats
```

The simulator needs no signing: `cmake --preset ios-simulator && cmake --build --preset ios-simulator`, then
`xcrun simctl install booted build/ios-simulator/bin/showcase.app`.

**Things to know on iOS**

* **Signing.** `-DESIA_IOS_PROVISIONING_PROFILE=<file>` picks the profile, `-DESIA_IOS_BUNDLE_PREFIX=` the bundle
  identifiers (default `org.esia.showcase`, `org.esia.glass-window`).
* **Options** are launch arguments (`devicectl device process launch ... org.esia.showcase --vsync off`). The app takes
  the whole screen; `--scale` defaults to the screen's (UI units are points).
* **Input.** The first finger is the mouse: a tap clicks, a drag along a scroll area scrolls it from anywhere (a row or
  a slider included, which then do not press), a sideways drag moves a slider; two fingers scroll anywhere. The
  keyboard comes up for text fields with the system's input methods (pinyin and the like compose inline); a hardware
  keyboard works.
* **Layout.** The examples keep to the safe area the frame passes on (`FrameParams::safeArea`: clear of the camera
  housing and the home indicator), and lay themselves out for a phone: the showcase shows one panel at a time, as large
  as fits, and the dock switches between them; the workbench docks its six panels as tabs of two (one above the other,
  side by side in landscape), with the asset table's type column left out; glass_window stacks its windows. The island
  grows out of the Dynamic Island: a notification's card shows below the camera, the resident pill only its icon and
  progress beside it.
* **Fonts and wallpaper.** As on macOS; the system's emoji (`emjc` strikes) go through Core Text, and the wallpaper is
  one of the building Mac's, copied into the app.
* **Performance.** On an iPhone 18 Pro Max (1320 x 2868 pixels) the showcase keeps the display's 120 Hz with about 6 ms
  of GPU time per frame.
* **Limitations.** As on macOS.

## Android

<p align="center"><img src="docs/images/showcase-android.jpg" width="80%" alt="Three screens of the showcase on Android: Settings, the Control Center and the Languages panel with color emoji and flags"></p>
<p align="center"><sub>The showcase on the Android 15 emulator, OpenGL ES: Settings, Control Center, Languages.</sub></p>

**Build** (from Windows, Linux or macOS). The Android NDK and SDK (a platform, build-tools and platform-tools; Android
Studio's SDK manager installs them), a JDK for the APK signing tools, CMake and Ninja:

```
export ANDROID_NDK_HOME=<the NDK> ANDROID_HOME=<the SDK>     # set ... in a Windows prompt
cmake --preset android
cmake --build --preset android
adb install build/android/apk/showcase.apk
adb shell "am start -n org.esia.showcase/android.app.NativeActivity -e args '--api vulkan'"
```

The examples become APKs in `build/android/apk` (`showcase`, `workbench`, `glass_window`), built for arm64 phones;
`-DANDROID_ABI=x86_64` builds them for the emulator. No Gradle and no Java code: each example is a shared library
behind Android's `NativeActivity`, packed and signed with a debug key by `tools/android/package.py`.

**Things to know on Android**

* **Options** come from the launch intent's `args` extra (above: the whole command in one string, or the device's
  shell splits the options): `--api opengl` (OpenGL ES 3.0, the default) or `--api vulkan`, and the options of
  [Running the examples](#running-the-examples). stdout and stderr go to the log (`adb logcat -s esia`); a
  `--screenshot` lands in the app's files (`adb exec-out run-as org.esia.showcase cat files/shot.png > shot.png`).
* **Touch.** The first finger is the mouse: a tap clicks, a drag along a list scrolls it from anywhere (a row or a
  slider included, which then do not press), a sideways drag moves a slider; two fingers scroll anywhere. UI units are
  dp: the scale is the screen's density. The soft keyboard comes up for text fields; the clipboard is Android's.
* **Layout.** As on an iPhone, the showcase shows one panel at a time, as large as fits; the dock switches between them.
* **The app's lifetime.** Sent to the background, the app drops its window surface and keeps its device, textures and
  UI; brought back, it carries on where it was.
* **Fonts and icons.** Roboto, then a Noto font per script, Noto Color Emoji (a COLR font) and its flags font. Android
  has no icon font for apps: the examples bring Google's Material Icons (Outlined, Apache License 2.0), downloaded when
  configuring (`ESIA_ANDROID_ICON_FONT` names a local copy for offline builds) and packed into each APK.
* **Validation.** The Vulkan validation layer is not in the NDK, and `tools/android/package.py` does not pack it:
  `--debug` on Vulkan says it is missing (Android's loader would find Khronos' `libVkLayer_khronos_validation.so`
  among the APK's libraries). OpenGL ES debug output works where the driver has `KHR_debug`.
* **Limitations.** Verified on the emulator only (Android 15 on an RTX 4080 SUPER); input methods that compose
  (pinyin, kana) do not reach a NativeActivity, so only directly typed text arrives; custom HLSL effects need Direct3D.

## Changing the shaders

`src/esia/shaders/generated` (SPIR-V, GLSL, ESSL, MSL) is generated from the HLSL in `src/esia/shaders` by
`tools/shaders/build_shaders.py`, with the glslang and SPIRV-Cross of Ubuntu 24.04 (CI checks the files byte for byte).
Without those packages, push the shader change to a branch named `shaders/<anything>`: the `Shaders` workflow
regenerates the library and commits it to that branch ([CI.md](docs/CI.md), section 5). Take the generated files into
the commit for `main` and delete the branch.

## Documents

* [CI.md](docs/CI.md): the GitHub Actions jobs (Linux, Windows, macOS), what they prove, how to reproduce them.
* [REWRITE.md](docs/REWRITE.md): the architecture (layers, renderer, RHI, shaders, text, threading, phases).
* [UI_CORE.md](docs/UI_CORE.md): the UI core (input, hit testing, layout, windows, scrolling, popups) and how widgets
  use it.
* [UI_WIDGETS.md](docs/UI_WIDGETS.md): the widget layer, its theme and styles, and what of WGT is ported.
* [PLATFORM_WIN32.md](docs/PLATFORM_WIN32.md): the Win32 platform layer and `glass_window`.
* [REWRITE_STATUS.md](docs/REWRITE_STATUS.md): what is done and verified, known issues, next steps.
* [backends/README.md](docs/backends/README.md): how to write and test an RHI backend.

## Roadmap

1. Platform layers: Cocoa and UIKit as libraries (the examples' AppKit and UIKit frames are the start), then Wayland,
   SDL, and inline IME composition on Linux and Android.
2. Text: the bidi algorithm, variable-font instances (SF's weights).
3. Performance: fewer render-pass breaks per backdrop capture on tile-based GPUs; FX feature variants on Vulkan and
   OpenGL (the specialization constant is already in the shader library).
4. Packaging (`find_package(esia)`), API reference, the iOS frame as a platform layer.

## Third-party

The text tests use DroidSans (Apache License 2.0), Karla and a subset of Noto Sans SC (SIL Open Font License 1.1) in
`tests/fonts`. With `ESIA_TEXT_FREETYPE` on (the default), FreeType (FreeType License) and HarfBuzz (MIT) come from the
system or are downloaded and built from source at configure time: `ESIA_TEXT_DEPS=auto|bundled|system`. The Android
examples download Google's Material Icons (Outlined, Apache License 2.0) for their icons at configure time and pack the
font into their APKs.
