# WGT UI — Waffling Game Toolkit

A liquid-glass UI toolkit for Direct3D 11 / Direct3D 12 games and tools, shipped as one DLL (`wgt.dll`).
Dear ImGui (1.92) is used for what it is good at — immediate-mode ids, input, windows, text editing — while
everything you *see* is WGT's own: an SDF shape renderer with materials, a DirectWrite text engine, an
iOS-style design system and spring-driven motion.

* **Effects stock ImGui cannot do** — liquid glass (backdrop blur, lensing/refraction, chromatic dispersion,
  specular rim, adaptive legibility), outer/inner glow, GPU bloom layers, soft & inner shadows, gradients
  (linear / radial / conic), continuous "squircle" corners, liquid merges (metaballs), shimmer, rounded
  masks, image fills and **custom HLSL effects compiled at runtime**.
* **Style any component on its own** — `ui::Next().Tint(green).Radius(8)` before a widget changes just that
  widget: its glass look (frosted by default, fully clear, or the theme's) and any glass parameter, plus its
  tint, fill, corner radius and opacity. There's no theme editing and no Push/Pop pair; the same modifiers
  work on a section, card or window, on a scope, or on the whole UI.
* **Charts and lines** — `ui::LineChart` (smooth monotone line, gradient fill, eased range) and
  `Painter::Polyline` / `Painter::Area` for custom charts. Glow is opt-in everywhere.
* **Its own design** — every widget is drawn by `wgt::Painter`; light/dark themes, materials, typography
  and motion are data (`wgt::Theme`) and animate when changed. Controls move like liquid: lens knobs,
  stretch and squash, Control Center toggles that flood with color from the press point.
* **Auto layout** — responsive containers (`BeginFlow`, adaptive `BeginGrid`, `BeginHStack` /
  `BeginVStack` / `BeginAdaptiveStack`) that measure their children and re-flow with the page size. Items
  glide to their new slots with a spring. Glowing items get room automatically, and any glow that would
  still reach a neighbour fades out before its edge.
* **Every language, no setup** — DirectWrite shaping, bidi and per-character font fallback (CJK, Arabic,
  Hebrew, Devanagari, Thai …) and color emoji, with no glyph ranges or font merging. WGT's own text editor
  moves by grapheme cluster and composes IME input inline. The text system is independent of Dear
  ImGui's font code.
* **Smooth, true-to-design text** — an analytic rasterizer draws the unhinted outline with exact coverage
  at the physical pixel density (per-monitor DPI × user scale × render scale). ClearType-style sub-pixel
  anti-aliasing (3× horizontal resolution) follows the system setting, and the gamma / contrast are
  composed like DirectWrite. Text stays crisp while it animates.
* **Nothing overlaps** — text wraps at its container's edge instead of running out of the window, rows
  ellipsize or stack, scroll indicators get their own lane, floating glass bars
  (tab bar, search bar) let content scroll under them but always clear of them, glows fade before
  neighbours, and a layout inspector (`debugLayout`) flags overlapping and cut-off items.
* **Thread-safe** — one UI thread renders; every service (`Notify`, `Post`, panels, themes, input,
  `Property<T>`, `Channel<T>`, `Latest<T>`) is callable from any thread. Window thread and render thread
  can be separate: key state, mouse capture and IME are handled on the right thread. Custom shaders
  compile in the background.
* **Frame pacing** — `wgt::FramePacer` gives precise frame caps with no busy wait, and works with VSync.
* **Extensible** — custom widgets from public pieces (`ui::Interact` + `Painter` + `anim`), themes,
  effects, plugin DLLs, custom render backends, input injection for automated UI tests.

## Build

Requirements: Windows 10/11, Visual Studio 2022 (C++20), CMake ≥ 3.24, Windows SDK (d3dcompiler / fxc).

```bash
cmake --preset vs2022
```

```bash
cmake --build --preset release
```

Output lands in `build/bin/Release` (`wgt.dll`, `wgt_demo.exe`, `wgt_minimal_d3d11.exe`,
`plugins/wgt_plugin_hello.dll`) and `build/lib/Release` (`wgt.lib`). Set `-DWGT_BUILD_EXAMPLES=OFF` to build
only the library.

### SDK and packages

```bash
cmake --install build --config Release --prefix <sdk dir>
```

```bash
cpack --config build/CPackConfig.cmake -C Release
```

```bash
cpack --config build/CPackSourceConfig.cmake
```

The install tree is a self-contained SDK: `bin/` (`wgt.dll`, `wgt.pdb`, demo), `lib/` (`wgt.lib`, CMake
package), `include/wgt` + `include/imgui`, `wgt.props` for Visual Studio projects, docs, examples. CPack
writes `build/packages/WGT-1.1.0-sdk-win64.zip` and `WGT-1.1.0-src.zip`. Consumers link it with:

```cmake
find_package(wgt 1.1 CONFIG REQUIRED)
target_link_libraries(my_game PRIVATE wgt::wgt)
```

## Quick start (Direct3D 11)

```cpp
#include <wgt/wgt.hpp>

wgt::ContextDesc desc;
desc.backend = wgt::Backend::D3D11;
desc.hwnd = hwnd;
desc.d3d11Device = device;
desc.d3d11Context = immediateContext;
wgt::Context* ui = wgt::Context::Create(desc);   // the calling thread becomes the UI thread

// WndProc (may run on another thread than rendering)
if (ui->HandleWin32Message(hwnd, msg, wParam, lParam))
    return msg == WM_SETCURSOR ? TRUE : 0;

// every frame, after the game has rendered into the back buffer
ui->NewFrame();
static bool open = true;
if (wgt::ui::BeginWindow("Hello", &open))
{
    static bool hdr = true;
    wgt::ui::RowToggle("HDR", &hdr, {wgt::icons::Brightness, wgt::Color::Hex(0xFF9F0A)});
    wgt::ui::ButtonOptions b;
    b.kind = wgt::ui::ButtonKind::Glass;
    wgt::ui::Button("Liquid Glass", b);
    wgt::ui::EndWindow();
}
ui->Render(wgt::RenderTarget::D3D11(backBufferRtv));

// shutdown
ui->Destroy();
```

### Direct3D 12

```cpp
desc.backend = wgt::Backend::D3D12;
desc.d3d12Device = device;
desc.d3d12FramesInFlight = 3;              // >= the number of frames your renderer keeps in flight
...
// back buffer in D3D12_RESOURCE_STATE_RENDER_TARGET; WGT records into your list and leaves the state as is
ui->Render(wgt::RenderTarget::D3D12(commandList, backBuffer, rtvHandle.ptr, DXGI_FORMAT_R8G8B8A8_UNORM));
```

WGT sets its own descriptor heaps and root signature on the list; rebind yours if you record more work after
`Render()`. Window resizes need nothing from WGT. If the device is lost or replaced, destroy the context and
create a new one with the new device.

The complete host is [examples/minimal_d3d11/main.cpp](examples/minimal_d3d11/main.cpp); the integration
guide ([docs/INTEGRATION.md](docs/INTEGRATION.md), Chinese) covers project setup, input, threading, D3D12,
render-target formats, diagnostics and a checklist.

## The demo

`wgt_demo.exe` is a showcase and a test harness:

| Flag | Meaning |
| --- | --- |
| `--dx11` / `--dx12` | backend |
| `--light` | light appearance |
| `--scale 1.5` | fixed DPI scale (default: follow the monitor) |
| `--render-scale 2` | render at 2× the window resolution (super-sampled game); text stays crisp |
| `--open settings,effects,control,components,languages,telemetry` / `--open-all` | panels to open |
| `--glass theme\|clear\|frosted` | glass look of the whole UI (`Context::SetGlassLook`, default frosted) |
| `--novsync`, `--fps 120` | VSync off (tearing-capable swap chain), frame cap (`wgt::FramePacer`) |
| `--debug-layout` | layout inspector: outline and log overlapping items |
| `--screenshot out.png --frames 120` | capture frame N and exit |
| `--fixed-dt 0.016667` | deterministic animation clock |
| `--script "30:move 373 329;31:down;33:up;34:text 你好;35:shot a.png"` | injected input + captures per frame |
| `--stats`, `--debug-layer`, `--low-power` | diagnostics |

The demo runs like a game: the main thread pumps window messages and a render thread owns the device,
the WGT context and the frame loop. Settings → Display switches VSync and the frame limit live.

F1 toggles the whole UI, like a game overlay.

## Documentation

* [docs/INTEGRATION.md](docs/INTEGRATION.md) — 对接文档: integrating WGT into a game or tool (Chinese).
* [docs/API.md](docs/API.md) — API 参考: every public type, function and option, header by header (Chinese).
* [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — how a frame flows from widgets to pixels.
* [docs/EXTENDING.md](docs/EXTENDING.md) — custom widgets, styles, themes, HLSL effects, plugins, fonts,
  the scale model, custom backends, UI tests.
* [docs/THREADING.md](docs/THREADING.md) — what may be called from where.

## Layout

```
include/wgt/     public API (header-only helpers + WGT_API exports)
src/core/        context, frame loop, themes, fonts, animation store
src/text/        DirectWrite text engine + Dear ImGui font-loader shim
src/render/      Painter, frame planning, effect compiler, shared GPU helpers
src/backends/    Direct3D 11 / Direct3D 12 renderers
src/ui/          widgets (controls, lists, windows, navigation, overlay) and auto layout
src/shaders/     HLSL (compiled at build time; wgt_fx.hlsl also embedded for runtime effects)
examples/        minimal D3D11 host, demo host (D3D11 + D3D12), example plugin DLL
cmake/           shader tooling, package config, wgt.props (Visual Studio property sheet)
third_party/     Dear ImGui 1.92 (WGT's fork, compiled into wgt.dll)
```
