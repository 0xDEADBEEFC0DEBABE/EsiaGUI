# Esia

A liquid-glass (iOS 26 style) UI toolkit for games and tools, in C++20, with its own UI core, renderer and render
hardware interface (RHI). It replaces WGT UI, the Dear ImGui based `wgt.dll`; WGT's last version is tag
`wgt-1.1-final`, in this repository's history. `main` is the only branch.

## Status

| Part | State |
| --- | --- |
| UI core (`src/esia/core`) | second version ([UI_CORE.md](docs/UI_CORE.md)): ids, input, windows with per-window DPI, front-to-back hit testing, press and key ownership, containers and layout providers, child scroll regions, popups and tooltips, per-id state, draw lists, texture registry |
| Widgets (`src/esia/ui`, `esia::ui`) | WGT's liquid-glass widgets on the core ([UI_WIDGETS.md](docs/UI_WIDGETS.md)): themes, per-widget styles, springs, auto layout, controls, lists, navigation, popups, text fields, charts, the island |
| Renderer (`src/esia/render`) | Painter, frame planner, liquid glass (backdrop captures, blur pyramid, refraction), glow layers, edge fades, GPU profiling |
| Text (`src/esia/text`) | analytic glyph rasterizer, glyph atlas, FreeType + HarfBuzz text system (bundled or the system's), system font lookup with fallback chains for every major script, color emoji from bitmap strikes (sbix, CBDT); on Apple systems the fonts FreeType cannot read (PingFang's `hvgl` glyphs, iOS' `emjc` emoji) through Core Text; grayscale antialiasing |
| Backends (`src/esia/rhi/<name>`) | DirectX (Direct3D 9 / 10 / 11 / 12, in `rhi/directx`), OpenGL 3.3 / OpenGL ES 3.0, Vulkan 1.1+, Metal |
| Platform | Win32 layer (`src/esia/platform/win32`, [PLATFORM_WIN32.md](docs/PLATFORM_WIN32.md)); the examples' frame is AppKit + Metal on macOS and UIKit + Metal on iOS (`examples/glass_window/app_macos.mm`, `app_ios.mm`, `app_apple.mm`) |
| Examples | `showcase` (WGT's demo on Esia) and `glass_window` (a smoke test of the whole path), on Windows, macOS and iOS |

| Backend | CMake option | Verified on |
| --- | --- | --- |
| DirectX: Direct3D 11, 12, 10, 9 | `ESIA_BACKEND_DIRECTX` | Windows 11, NVIDIA RTX 4080 SUPER, debug layers (D3D12 GPU-based validation); Wine |
| OpenGL, OpenGL ES | `ESIA_BACKEND_OPENGL` | NVIDIA (WGL), Mesa llvmpipe (EGL) |
| Vulkan | `ESIA_BACKEND_VULKAN` | NVIDIA and Mesa lavapipe, Khronos validation layer |
| Metal | `ESIA_BACKEND_METAL` | MacBook Pro, Apple M3 Pro, macOS 27 (Metal API and shader validation); macOS 15 on GitHub's arm64 runner; iPhone 18 Pro Max (A20 Pro), iOS 27 |

Every backend passes the conformance suite (17 scenes against golden images, single frame and across frames, with
the API's validation counted); details and numbers in `docs/REWRITE_STATUS.md` and each backend's `STATUS.md`.

## Getting it

Clone the branch of your platform: it holds what that platform builds and nothing else, and its README has the two
commands that build it.

| Branch | For | Backends |
| --- | --- | --- |
| `windows` | Windows (Visual Studio 2022, or LLVM) | DirectX (Direct3D 11, 12, 10 or 9: the first that works), OpenGL, Vulkan |
| `apple` | macOS and iOS | Metal |
| `linux` | Linux (no example frame yet) | OpenGL, Vulkan |

```
git clone -b windows https://github.com/0xDEADBEEFC0DEBABE/EsiaGUI.git
```

They are made from `main` after every push to it (`tools/branches/platform_branches.py`, the `Platform branches`
workflow): do not commit to them. `main` has every platform, the development history and the documents; it builds the
same way.

## Building

Clone `main` (or a platform branch) and build: the backends of your platform are on by default, nothing to choose
first.

| Platform | Backends built by default |
| --- | --- |
| Windows | DirectX (Direct3D 9, 10, 11, 12) and OpenGL; Vulkan too when the Vulkan SDK is installed (`VULKAN_SDK`) |
| Linux | OpenGL; Vulkan too when the Vulkan headers are installed |
| macOS, iOS | Metal |

Configuring prints the list (`-- Esia backends: ...`). `-DESIA_BACKEND_<NAME>=ON` or `OFF` adds or drops one
(`DIRECTX`, `OPENGL`, `VULKAN`, `METAL`); with DirectX on, the advanced `ESIA_BACKEND_D3D9` / `D3D10` / `D3D11` / `D3D12`
leave out single Direct3D versions. On Windows the examples start DirectX: the first Direct3D version that works on
the machine (11, then 12, 10, 9); `--api` picks a backend or a version.
`-DESIA_WERROR=ON` turns warnings into errors (every preset builds without warnings).

**Windows**, with Visual Studio 2022 and nothing else (or open the folder in Visual Studio: it lists the presets):

```
cmake --preset windows-msvc
cmake --build --preset windows-msvc-release
```

With LLVM (clang-cl and lld on PATH, from an "x64 Native Tools" prompt): `cmake --preset windows-clang-cl`,
`cmake --build --preset windows-clang-cl`, `ctest --preset windows-clang-cl`. The Windows backends can be
cross-compiled from Linux with `windows-mingw-cross` (or `windows-cross` with xwin).

**Linux** (clang + lld; `libvulkan-dev` or the Vulkan SDK for Vulkan):

```
cmake --preset linux-clang
cmake --build --preset linux-clang && ctest --preset linux-clang
```

**macOS** (Apple silicon or Intel, macOS 11 or later; Xcode for the SDK and the Metal compiler):

```
brew install llvm lld ninja cmake freetype harfbuzz
xcodebuild -downloadComponent MetalToolchain     # only if `xcrun metal` says the Metal toolchain is missing
cmake --preset macos-clang
cmake --build --preset macos-clang && ctest --preset macos-clang
```

`ctest` runs the Metal conformance suite on the Mac's GPU; set `MTL_DEBUG_LAYER=1 MTL_SHADER_VALIDATION=1` for
Metal's API and shader validation (CI does).

Every preset puts the examples in `build/<preset>/bin` (`showcase`, `glass_window`; with Visual Studio in
`build/windows-msvc/bin/Release`). Visual Studio's build fails on paths over 260 characters: keep the clone's path
under about 140 characters (`C:\Users\<you>\source\repos\EsiaGUI` is fine), or enable long paths in Windows.

**iOS** (iPhones and iPads, iOS 16 or later; on a Mac with Xcode). The examples become app bundles, signed after the
link with a provisioning profile Xcode has for them (`tools/ios/codesign.py`: sign in to Xcode with your Apple ID; a
development or ad hoc profile that lists the device). `-DESIA_IOS_PROVISIONING_PROFILE=<file>` picks the profile,
`-DESIA_IOS_BUNDLE_PREFIX=` the bundle identifiers (default `org.esia.showcase`, `org.esia.glass-window`). With the
iPhone connected and unlocked:

```
cmake --preset ios && cmake --build --preset ios
xcrun devicectl list devices
xcrun devicectl device install app --device "<name>" build/ios/bin/showcase.app
xcrun devicectl device process launch --console --device "<name>" org.esia.showcase --stats
```

The simulator needs no signing: `cmake --preset ios-simulator && cmake --build --preset ios-simulator`, then
`xcrun simctl install booted build/ios-simulator/bin/showcase.app`.

**Shader library.** `src/esia/shaders/generated` (SPIR-V, GLSL, ESSL, MSL) is generated from the HLSL in
`src/esia/shaders` by `tools/shaders/build_shaders.py`, with the glslang and SPIRV-Cross of Ubuntu 24.04 (CI checks the
files byte for byte). Without those packages, push the shader change to a branch named `shaders/<anything>`: the
`Shaders` workflow regenerates the library and commits it to that branch ([CI.md](docs/CI.md), section 5). Take the
generated files into the commit for `main` and delete the branch.

## Running the examples

```
showcase                                   # Settings, Effects Lab and Control Center over a wallpaper, the dock, the island
showcase --open-all --light                # every panel, light theme
showcase --vsync off --stats               # uncapped frame rate, timings once a second (macOS)
glass_window                               # the smoke test: glass, the core's windows, text and input
```

| Option | Meaning |
| --- | --- |
| `--api directx \| d3d11 \| d3d12 \| d3d10 \| d3d9 \| opengl \| vulkan` | Windows: the backend; `directx` (the default) starts the first Direct3D version that works, 11, 12, 10, 9; macOS: Metal only |
| `--size WxH`, `--scale s` | client area in UI units, and pixels per UI unit (default: the monitor's); `--size 1512x945 --scale 2` is 3024 x 1890 pixels |
| `--vsync on \| off` | off: no frame cap. On macOS a window cannot present faster than the display, so frames render into offscreen targets as fast as they can and the newest is presented each refresh |
| `--stats` | macOS: once a second the frame rate, UI / encode / wait times, the GPU time of the frame's command buffer, passes, backdrop captures, frames shown and dropped |
| `--fullscreen`, `--fullscreen-at N`, `--hide-at N` | macOS: start in full screen, enter it after frame N, or hide the app after frame N (tests: the frame loop keeps its speed hidden, it declares itself latency critical so macOS does not nap it) |
| `--frames N`, `--screenshot out.png`, `--fixed-dt s` | quit after N frames, write the last one, advance the UI clock by `s` per frame (deterministic captures) |
| `--debug` | the API's validation layer (Metal: `MTL_DEBUG_LAYER`); the message count is printed at exit |
| `--font file` | font files instead of the system's (the first is the main one) |
| showcase: `--open list`, `--open-all`, `--light`, `--look theme \| clear \| frosted`, `--tab n`, `--page id` | panels to open (`settings,effects,control,components,languages,telemetry,plugin`), theme, glass look, the Components tab, the Settings page |

On macOS and iOS the examples use the system fonts (SF, the fallback chain of `system_fonts.hpp`, Apple Color Emoji),
SF Symbols for esia::ui's icons (Windows' icon fonts are not there) and a system wallpaper (iOS: one of the building
Mac's, copied into the app).

**On iOS** the options are launch arguments (`devicectl device process launch ... org.esia.showcase --vsync off`);
the app takes the whole screen, `--scale` defaults to the screen's (UI units are points). The first finger is the
mouse (tap = click, a drag on a scroll area's empty space scrolls it), two fingers scroll anywhere, the keyboard
comes up for text fields with the system's input methods (pinyin and the like compose inline), a hardware keyboard
works. On a phone the showcase shows one panel at a time, as large as
fits; the dock switches between them.

**Performance on Metal** (showcase, MacBook Pro M3 Pro, 3024 x 1890 pixels at UI scale 2, `--vsync off`): about 140
fps with the shader library generated before the FX feature variants, about 170 fps with them (the Fx shader is
specialized per batch through Metal function constants); about 180 fps in the default 2560 x 1600 window. The GPU's
clock follows its temperature and load: a hot laptop or another app drawing can halve these for a while. Where the time goes: the glass panels' pixels, the drop
shadows around them, and about 15 backdrop captures per frame, each of which ends and resumes the render pass. Measure
with `--stats` or Instruments' Metal System Trace. On an iPhone 18 Pro Max (1320 x 2868 pixels) the showcase keeps the
display's 120 Hz with about 6 ms of GPU time per frame.

### Known limitations on macOS and iOS

* Custom HLSL effects (`Renderer::SetEffectSource`, the showcase's "HLSL effect" card) need runtime HLSL compilation,
  which only the Direct3D backends have: Metal draws those shapes with the built-in shader.
* SF is a variable font and the FreeType text system loads its default instance: the UI's bold weights draw as
  regular.
* Color emoji come from bitmap strikes (Apple Color Emoji, Noto Color Emoji); COLR fonts (Segoe UI Emoji on Windows)
  draw their monochrome outlines.
* On iPhones with a Dynamic Island, the island's notification card opens around the camera housing, which covers the
  middle of its first line (the UI does not know the safe area yet).

## Documents

* `docs/CI.md`: the GitHub Actions jobs (Linux, Windows, macOS), what they prove, how to reproduce them.
* `docs/REWRITE.md`: the architecture (layers, renderer, RHI, shaders, text, threading, phases).
* `docs/UI_CORE.md`: the UI core (input, hit testing, layout, windows, scrolling, popups) and how widgets use it.
* `docs/UI_WIDGETS.md`: the widget layer, its theme and styles, and what of WGT is ported.
* `docs/PLATFORM_WIN32.md`: the Win32 platform layer and `glass_window`.
* `docs/REWRITE_STATUS.md`: what is done and verified, known issues, next steps.
* `docs/backends/README.md`: how to write and test an RHI backend.

## Roadmap

1. Platform layers: Cocoa as a library (the examples' AppKit frame is the start), then SDL / X11 / Wayland.
2. Text: the bidi algorithm, variable-font instances (SF's weights), COLR color glyphs.
3. Performance: fewer render-pass breaks per backdrop capture on tile-based GPUs; FX feature variants on Vulkan and
   OpenGL (the specialization constant is already in the shader library).
4. Packaging (`find_package(esia)`), API reference, the iOS frame as a platform layer.

## Third-party

The text tests use DroidSans (Apache License 2.0), Karla and a subset of Noto Sans SC (SIL Open Font License 1.1) in
`tests/fonts`. With `ESIA_TEXT_FREETYPE` on (the default), FreeType (FreeType License) and HarfBuzz (MIT) come from the
system or are downloaded and built from source at configure time: `ESIA_TEXT_DEPS=auto|bundled|system`.
