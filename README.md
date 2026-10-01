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
at UI scale 1.5. The background is the system's wallpaper.</sub></p>

A liquid-glass (iOS 26 style) UI toolkit for games and tools, in C++20: its own UI core, widgets, renderer and render
hardware interface (RHI), with backends for Direct3D 9 to 12, OpenGL / OpenGL ES, Vulkan and Metal. It replaces WGT UI,
the Dear ImGui based `wgt.dll`; WGT's last version is tag `wgt-1.1-final`, in this repository's history.

* [Status](#status) · [Getting it](#getting-it) · [Building](#building) · [Running the examples](#running-the-examples)
* Per system: [Windows](#windows) · [Linux](#linux) · [macOS](#macos) · [iOS](#ios)
* [Changing the shaders](#changing-the-shaders) · [Documents](#documents) · [Roadmap](#roadmap) · [Third-party](#third-party)

## Status

| Part | State |
| --- | --- |
| UI core (`src/esia/core`) | second version ([UI_CORE.md](docs/UI_CORE.md)): ids, input, windows with per-window DPI, front-to-back hit testing, press and key ownership, containers and layout providers, child scroll regions, popups and tooltips, per-id state, draw lists, texture registry |
| Widgets (`src/esia/ui`, `esia::ui`) | WGT's liquid-glass widgets on the core ([UI_WIDGETS.md](docs/UI_WIDGETS.md)): themes, per-widget styles, springs, auto layout, controls, lists, navigation, popups, text fields, charts, the island |
| Renderer (`src/esia/render`) | Painter, frame planner, liquid glass (backdrop captures, blur pyramid, refraction), glow layers, edge fades, GPU profiling |
| Text (`src/esia/text`) | analytic glyph rasterizer, glyph atlas, FreeType + HarfBuzz text system (bundled or the system's), system font lookup with fallback chains for every major script, color emoji from bitmap strikes (sbix, CBDT), grayscale antialiasing |
| Backends (`src/esia/rhi/<name>`) | DirectX (Direct3D 9 / 10 / 11 / 12, in `rhi/directx`), OpenGL 3.3 / OpenGL ES 3.0, Vulkan 1.1+, Metal |
| Platform | a Win32 layer as a library (`src/esia/platform/win32`, [PLATFORM_WIN32.md](docs/PLATFORM_WIN32.md)); the examples' own AppKit (macOS) and UIKit (iOS) frames |
| Examples | `showcase` (WGT's demo on Esia) and `glass_window` (a smoke test of the whole path) |

| Backend | CMake option | Systems | Verified on |
| --- | --- | --- | --- |
| DirectX: Direct3D 11, 12, 10, 9 | `ESIA_BACKEND_DIRECTX` | Windows | Windows 11, NVIDIA RTX 4080 SUPER, debug layers (D3D12 GPU-based validation); Wine |
| OpenGL, OpenGL ES | `ESIA_BACKEND_OPENGL` | Windows, Linux | NVIDIA (WGL), Mesa llvmpipe (EGL) |
| Vulkan | `ESIA_BACKEND_VULKAN` | Windows, Linux | NVIDIA and Mesa lavapipe, Khronos validation layer |
| Metal | `ESIA_BACKEND_METAL` | macOS, iOS | MacBook Pro (M3 Pro, macOS 27), GitHub's arm64 runner (macOS 15), iPhone 18 Pro Max (A20 Pro, iOS 27); Metal API and shader validation |

Every backend passes the conformance suite (17 scenes against golden images, single frame and across frames, with
the API's validation counted); details and numbers in [REWRITE_STATUS.md](docs/REWRITE_STATUS.md) and each backend's
`STATUS.md`.

| System | Library and tests | Example window |
| --- | --- | --- |
| Windows 10 / 11 | yes | yes: DirectX, OpenGL, Vulkan |
| Linux | yes | not yet |
| macOS 11+ | yes | yes: Metal |
| iOS 16+ | yes | yes: Metal |

## Getting it

Clone the branch of your graphics API: it holds that backend and what builds it on every system it runs on (the
example frames, the tests, the presets), and its README has the commands.

| Branch | Backend | Systems |
| --- | --- | --- |
| `directx` | Direct3D 11, 12, 10, 9 (the first that works on the machine, or `--api d3d12` ...) | Windows |
| `opengl` | OpenGL 3.3 / OpenGL ES 3.0 | Windows, Linux |
| `vulkan` | Vulkan 1.1+ (the Vulkan SDK / headers) | Windows, Linux |
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
[iOS](#ios).

| System | Presets | Backends built by default |
| --- | --- | --- |
| Windows | `windows-msvc`, `windows-clang-cl` | DirectX (Direct3D 9, 10, 11, 12) and OpenGL; Vulkan too when the Vulkan SDK is installed |
| Linux | `linux-clang`, `linux-clang-release`; `windows-mingw-cross`, `windows-cross` build the Windows backends | OpenGL; Vulkan too when the Vulkan headers are installed |
| macOS | `macos-clang` | Metal |
| iOS | `ios`, `ios-simulator` | Metal |

* Configuring prints the list (`-- Esia backends: ...`). `-DESIA_BACKEND_<NAME>=ON` or `OFF` adds or drops one
  (`DIRECTX`, `OPENGL`, `VULKAN`, `METAL`); with DirectX on, the advanced `ESIA_BACKEND_D3D9` / `D3D10` / `D3D11` /
  `D3D12` leave out single Direct3D versions.
* `-DESIA_WERROR=ON` turns warnings into errors (every preset builds without warnings).
* Text: with `ESIA_TEXT_FREETYPE` on (the default), FreeType and HarfBuzz come from the system or are downloaded and
  built at configure time (`ESIA_TEXT_DEPS=auto|bundled|system`).
* Every preset puts the examples in `build/<preset>/bin` (`showcase`, `glass_window`); `ctest --preset <preset>` runs
  the tests, the conformance suite on the machine's GPU included.

## Running the examples

```
showcase                                   # Settings, Effects Lab and Control Center over a wallpaper, the dock, the island
showcase --open-all --light                # every panel, light theme
showcase --vsync off                       # uncapped frame rate
glass_window                               # the smoke test: glass, the core's windows, text and input
```

These options work on every system with an example window; each system's section lists its own.

| Option | Meaning |
| --- | --- |
| `--api name` | the backend, where more than one is built: see [Windows](#windows) |
| `--size WxH`, `--scale s` | client area in UI units, and pixels per UI unit (default: the monitor's); `--size 1512x945 --scale 2` is 3024 x 1890 pixels |
| `--vsync on \| off` | off: no frame cap |
| `--frames N`, `--screenshot out.png`, `--fixed-dt s` | quit after N frames, write the last one, advance the UI clock by `s` per frame (deterministic captures) |
| `--debug` | the API's validation layer; the message count is printed at exit |
| `--font file` | font files instead of the system's (the first is the main one) |
| showcase: `--open list`, `--open-all`, `--light`, `--look theme \| clear \| frosted`, `--tab n`, `--page id` | panels to open (`settings,effects,control,components,languages,telemetry,plugin`), theme, glass look, the Components tab, the Settings page |

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
  draws Windows' default wallpaper. Color emoji: Segoe UI Emoji is a COLR font, drawn with its monochrome outlines.
* **Wine.** The Direct3D backends run under Wine (screenshots in `examples/glass_window/wine_screenshots`).
* **Cross-compiling from Linux:** `cmake --preset windows-mingw-cross` (MinGW-w64), or `windows-cross` (clang-cl with
  the Windows SDK from xwin).

## Linux

**Build.** clang, lld, Ninja and CMake; EGL for OpenGL, the Vulkan headers for Vulkan (Ubuntu 24.04):

```
sudo apt install clang lld ninja-build cmake libfreetype-dev libharfbuzz-dev libegl-dev libvulkan-dev
cmake --preset linux-clang
cmake --build --preset linux-clang && ctest --preset linux-clang
```

**Things to know on Linux**

* **No example window yet:** Linux builds the library, its tests and the conformance suite. OpenGL runs through EGL
  without a window system (Mesa's llvmpipe, or the GPU's driver), Vulkan without a surface (lavapipe or the GPU's
  driver), so the tests run on headless machines and in CI.
* **Vulkan** is built when `vulkan/vulkan.h` is found (`libvulkan-dev`, or the Vulkan SDK); `--debug` and the tests use
  the Khronos validation layer when it is installed (`vulkan-validationlayers`).
* **Fonts.** The text system finds fonts through fontconfig when it is installed at configure time
  (`libfontconfig-dev`; `-DESIA_TEXT_FONTCONFIG=OFF` to not use it), otherwise by scanning the standard font
  directories. The UI font is Noto Sans (else DejaVu Sans), with Noto Sans CJK, WenQuanYi Zen Hei or Droid Sans
  Fallback for Chinese, Japanese and Korean: install `fonts-noto-core fonts-noto-cjk` for every script. The Linux
  fallback chain has no emoji font yet.
* **Custom HLSL effects** need Direct3D: on Linux those shapes draw with the built-in shader.

## macOS

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
  variable font and the FreeType text system loads its default instance: the UI's bold weights draw as regular. COLR
  color fonts draw their monochrome outlines (Apple Color Emoji is a bitmap font and is in color).

## iOS

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
* **Input.** The first finger is the mouse (tap = click, a drag on a scroll area's empty space scrolls it), two fingers
  scroll anywhere. The keyboard comes up for text fields with the system's input methods (pinyin and the like compose
  inline); a hardware keyboard works.
* **Layout.** On a phone the showcase shows one panel at a time, as large as fits; the dock switches between them.
* **Fonts and wallpaper.** As on macOS; the system's emoji (`emjc` strikes) go through Core Text, and the wallpaper is
  one of the building Mac's, copied into the app.
* **Performance.** On an iPhone 18 Pro Max (1320 x 2868 pixels) the showcase keeps the display's 120 Hz with about 6 ms
  of GPU time per frame.
* **Limitations.** As on macOS; and on iPhones with a Dynamic Island, the island's notification card opens around the
  camera housing, which covers the middle of its first line (the UI does not know the safe area yet).

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

1. Platform layers: Cocoa as a library (the examples' AppKit frame is the start), then X11 / Wayland (an example window
   on Linux) and SDL.
2. Text: the bidi algorithm, variable-font instances (SF's weights), COLR color glyphs.
3. Performance: fewer render-pass breaks per backdrop capture on tile-based GPUs; FX feature variants on Vulkan and
   OpenGL (the specialization constant is already in the shader library).
4. Packaging (`find_package(esia)`), API reference, the iOS frame as a platform layer.

## Third-party

The text tests use DroidSans (Apache License 2.0), Karla and a subset of Noto Sans SC (SIL Open Font License 1.1) in
`tests/fonts`. With `ESIA_TEXT_FREETYPE` on (the default), FreeType (FreeType License) and HarfBuzz (MIT) come from the
system or are downloaded and built from source at configure time: `ESIA_TEXT_DEPS=auto|bundled|system`.
