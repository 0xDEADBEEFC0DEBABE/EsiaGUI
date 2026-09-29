# Esia

A liquid-glass (iOS 26 style) UI toolkit for games and tools, in C++20, with its own UI core, renderer and render
hardware interface (RHI). It replaces WGT UI, the Dear ImGui based `wgt.dll`; WGT's last version is tag
`wgt-1.1-final` (branch `archive/wgt`), and screenshots of it are on branch `reference/wgt-1.1`.

## Status

The renderer and its backends work; widgets, windowing and a demo are the next phases (see "Roadmap").

| Part | State |
| --- | --- |
| UI core (`src/esia/core`) | ids, input queue, windows, items, layout cursor, draw lists, texture registry; to be reworked before the widgets (phase 3) |
| Renderer (`src/esia/render`) | Painter, frame planner, liquid glass (backdrop captures, blur pyramid, refraction), glow layers, edge fades, GPU profiling |
| Text (`src/esia/text`) | analytic glyph rasterizer, glyph atlas, FreeType + HarfBuzz text system; grayscale antialiasing |
| Backends (`src/esia/rhi/<name>`) | Direct3D 9 / 10 / 11 / 12, OpenGL 3.3 / OpenGL ES 3.0, Vulkan 1.1+, Metal (below) |
| Widgets, platform layer, demo | not yet: WGT's widgets are ported in phase 3, window / input / IME layers in phase 4 |

| Backend | CMake option | Verified on |
| --- | --- | --- |
| Direct3D 11, 12, 10, 9 | `ESIA_BACKEND_D3D11` / `D3D12` / `D3D10` / `D3D9` | Windows 11, NVIDIA RTX 4080 SUPER, debug layers (D3D12 GPU-based validation); Wine |
| OpenGL, OpenGL ES | `ESIA_BACKEND_OPENGL` | NVIDIA (WGL), Mesa llvmpipe (EGL) |
| Vulkan | `ESIA_BACKEND_VULKAN` | NVIDIA and Mesa lavapipe, Khronos validation layer |
| Metal | `ESIA_BACKEND_METAL` | its portable part on a fake GPU only; the Metal code needs a Mac |

Every backend passes the conformance suite (17 scenes against golden images, single frame and across frames, with
the API's validation counted); details and numbers in `docs/REWRITE_STATUS.md` and each backend's `STATUS.md`.

## Building

The toolchain is LLVM (clang / clang-cl with lld); MSVC works too.

```
# Windows, from an "x64 Native Tools" prompt with LLVM on PATH (Vulkan: set VULKAN_SDK)
cmake --preset windows-clang-cl -DESIA_BACKEND_D3D11=ON -DESIA_BACKEND_OPENGL=ON
cmake --build --preset windows-clang-cl
ctest --preset windows-clang-cl

# Windows with MSVC (Visual Studio 2022)
cmake --preset windows-msvc -DESIA_BACKEND_D3D11=ON
cmake --build --preset windows-msvc-release

# Linux (clang + lld); macOS: macos-clang
cmake --preset linux-clang -DESIA_BACKEND_OPENGL=ON -DESIA_BACKEND_VULKAN=ON
cmake --build --preset linux-clang && ctest --preset linux-clang
```

`-DESIA_WERROR=ON` turns warnings into errors (every preset builds without warnings). The Windows backends can be
cross-compiled from Linux with `windows-mingw-cross` (or `windows-cross` with xwin).
`python3 tools/shaders/build_shaders.py` regenerates the shader library after a change to `src/esia/shaders`.

## Documents

* `docs/REWRITE.md`: the architecture (layers, renderer, RHI, shaders, text, threading, phases).
* `docs/REWRITE_STATUS.md`: what is done and verified, known issues, next steps.
* `docs/backends/README.md`: how to write and test an RHI backend.

## Roadmap

1. UI core, second version: the review's input fixes; layout, hit testing, input routing, scroll areas, popups.
2. Platform layers: Win32 first (windows, input, IME, DPI, clipboard, swap chains), then Cocoa and SDL / X11 /
   Wayland.
3. Text: FreeType + HarfBuzz bundled on every platform, system font lookup.
4. Widgets: WGT's iOS-style controls, themes, per-component styles, animations and auto layout on the new core,
   compared with the WGT reference screenshots; then the demo.
5. Packaging (`find_package(esia)`), API reference, Metal verified on a Mac, CI.

## Third-party

The text tests use DroidSans (Apache License 2.0) and Karla (SIL Open Font License 1.1) in `tests/fonts`.
FreeType and HarfBuzz are found on the system when `ESIA_TEXT_FREETYPE` is on.
