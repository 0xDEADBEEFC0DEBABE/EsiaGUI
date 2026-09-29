# Esia on Win32 - the platform layer and `glass_window`

Branch `feat/platform-win32`, written in a Linux cloud session (2026-09-29) without Windows or a GPU. Everything
below that "ran" ran under **Wine 9.0 on Xvfb with Mesa's software renderers** (wined3d on llvmpipe for Direct3D
9 / 10 / 11, Wine's WGL on llvmpipe for OpenGL, winevulkan on llvmpipe for Vulkan), built with the
`windows-mingw-cross` preset. That exercises the code paths; it is **not proof that anything works on real drivers,
with a real IME or across real monitors**. Section 6 is the plan for the local Windows session.

* [1. Design](#1-design)
* [2. API](#2-api)
* [3. Threading rules](#3-threading-rules)
* [4. The example: `glass_window`](#4-the-example-glass_window)
* [5. What was verified, and how](#5-what-was-verified-and-how)
* [6. Test plan for the Windows session](#6-test-plan-for-the-windows-session)
* [7. Core requests](#7-core-requests)
* [8. Known issues](#8-known-issues)
* [9. Files](#9-files)

## 1. Design

`esia_platform_win32` (CMake option `ESIA_PLATFORM_WIN32`, on by default on Windows, absent elsewhere) is the
platform layer of `docs/REWRITE.md` section 10 for Win32. It depends on `esia_core` only: it turns one window's
messages into `InputEvent`s (`Context::QueueInput`) and applies the core's `PlatformRequests` to that window. It owns
no UI state and knows nothing of the renderer or the backends.

It serves either a window it creates (`CreateAppWindow`: an app) or the host's own window (`Attach` + forwarding
every message to `HandleMessage`: a game). One `Platform` per window.

### Messages -> input

| Win32 | Core | Notes |
| --- | --- | --- |
| `WM_MOUSEMOVE` | `MouseMove(px / scale)` | UI units = client pixels / scale, converted on the window thread with the scale of the moment; unchanged positions are not queued |
| `WM_[LRMX]BUTTONDOWN/DBLCLK/UP` | the button's position, then `Button` | `SetCapture` on the first button down, `ReleaseCapture` after the last up; a release without a press in the window is dropped; the core detects double clicks itself |
| `WM_CAPTURECHANGED` | `Button(up)` for held buttons | only when the lost capture was the platform's (a dialog took it); a host's own capture is the host's |
| `WM_MOUSELEAVE` (`TrackMouseEvent`) | `MouseMove(-1, -1)` | held back while captured; after the release, the platform checks whether the cursor is outside |
| `WM_MOUSEWHEEL` / `WM_MOUSEHWHEEL` | `Wheel(0, delta / 120)` / `Wheel(-delta / 120, 0)` | fractional notches (touchpads); x > 0 scrolls toward the left, like y > 0 toward the top (the core's `UpdateScroll`) |
| `WM_(SYS)KEYDOWN/UP` | `KeyEvent(key, down, mods)` | left / right from the scan code or extended bit; mods from `GetKeyState` (the keyboard as of that message); auto-repeat dropped (the core repeats); `VK_PROCESSKEY` dropped (the IME's key) and its release too; both shifts released on one key-up |
| `WM_CHAR` | `TextEvent(utf8)` | UTF-16 surrogate pairs joined; control characters (< 0x20, DEL) are keys, not text |
| `WM_SETFOCUS` / `WM_KILLFOCUS` | `FocusEvent` | on focus loss the window thread also forgets its held keys / buttons, releases its capture and ends a composition |
| `WM_IME_COMPOSITION` | `TextEvent(result)`, then `Composition(string, caret in UTF-8 bytes)` | `GCS_RESULTSTR`, `GCS_COMPSTR`, `GCS_CURSORPOS` |
| `WM_IME_ENDCOMPOSITION` | `Composition("", 0)` | |
| `WM_SIZE`, `WM_DPICHANGED` | `Platform::Frame()` | the UI thread builds `FrameParams` from them (below) |

### Requests -> window

`ApplyRequests` (UI thread, after `EndFrame`) publishes the capture flags at once and, when the cursor, the text
input or the IME rect changed, posts **one** registered message; the window thread then applies the latest requests:

| Request | Window thread |
| --- | --- |
| `cursor` | `WM_SETCURSOR` over the client area answers with the shape (`Desc::cursor`: always, only while the UI wants the mouse, or never); a change under a still mouse is applied at once |
| `wantTextInput` | the window's input context is associated (`ImmAssociateContextEx(IACE_DEFAULT)`) only while it is true (`Desc::imeOnlyForTextInput`); turning it off cancels a composition in progress (`CPS_CANCEL`: its text would reach the next frame, when no field has the focus) |
| `imeRect` | the caret, in UI units -> client pixels: `ImmSetCompositionWindow` (`CFS_FORCE_POSITION`), `ImmSetCandidateWindow` (`CFS_EXCLUDE` of the caret line, so the candidates open below it, never over it), a hidden system caret (`CreateCaret` / `SetCaretPos`, for IMEs and accessibility tools that follow `GetCaretPos`), and `WM_IME_REQUEST` / `IMR_QUERYCHARPOSITION` (TSF IMEs, Microsoft Pinyin included, ask where the text is) |
| `wantCaptureMouse / Keyboard / TextInput` | atomics any thread reads (`WantCaptureMouse()` ...): a game skips the input the UI has |

### IME

Chinese through Microsoft Pinyin is the case that matters. With `Desc::inlineComposition` (default) the UI draws the
composition itself: `WM_IME_SETCONTEXT` goes to `DefWindowProc` without `ISC_SHOWUICOMPOSITIONWINDOW`,
`WM_IME_STARTCOMPOSITION` / `COMPOSITION` / `ENDCOMPOSITION` are answered by the platform (so `DefWindowProc` neither
opens the IME's composition window nor turns the result into `WM_IME_CHAR` -> `WM_CHAR` a second time), and the IME
shows only its candidate list at the caret. The keys the IME takes arrive as `VK_PROCESSKEY` and never reach the UI,
so Backspace, Enter, Space and digits inside a composition edit the composition, not the field. Without inline
composition the IME draws its own composition window at the caret (with a font as tall as the caret line) and the
result arrives as `WM_CHAR`. The IME messages are the UI's only while it takes text (or finishes a composition it
started): with `imeOnlyForTextInput` off, a game's own IME use is left alone.

### DPI

`CreateAppWindow` makes the process per-monitor-v2 DPI aware (`EnableDpiAwareness`; a manifest that already did is
fine) and sizes the window in UI units for its monitor. `scale` = monitor DPI / 96 x `Desc::uiScale` (a user zoom,
`SetUiScale`) is **pixels per UI unit**; `FrameInfo::Params(time)` gives `displaySize` = client pixels / scale and
`framebufferScale` = scale, so the UI renders at the native resolution and keeps its size in UI units on every
monitor. On `WM_DPICHANGED` an app window takes the rectangle Windows suggests; the size and the scale are
published together. A host window keeps its size (the host decides; `HandleMessage` returns false for it).

### Touch and pen

Not handled: `WM_POINTER*` falls through to Windows' mouse promotion (touch and pen arrive as the left button, press
and hold as the right one). The core has no pointer events (id, type, pressure, several contacts), so taking
`WM_POINTER` over would only re-derive what the promotion does, without hardware here to test it (section 7).

## 2. API

`include/esia/platform/win32.hpp`, namespace `esia::platform::win32`:

| | Thread | |
| --- | --- | --- |
| `Platform(const Desc&)` | any | `inlineComposition`, `imeOnlyForTextInput`, `cursor` (`CursorMode::Always / WhenCapturing / Never`), `uiScale` |
| `bool Attach(void* hwnd)` / `void Detach()` | window | serve / stop serving a window (a destroyed window detaches itself) |
| `bool HandleMessage(hwnd, msg, wparam, lparam, intptr_t* result)` | window | first in the window procedure; true = return `*result`, skip `DefWindowProc` |
| `void SetContext(Context*)` | UI | where input goes; `SetContext(nullptr)` returns once nothing is being queued into the old one |
| `FrameInfo Frame()` | any | client pixels, `dpiScale`, `scale`, `minimized`, `focused`; `Params(time)` -> `FrameParams` |
| `void ApplyRequests(const PlatformRequests&)` | UI | after `EndFrame` |
| `void Configure(ContextDesc&)` | before the context | clipboard callbacks (UTF-8 <-> UTF-16, `\n` <-> `\r\n`) and the user's double-click time and key repeat (Control Panel) |
| `void SetUiScale(float)` | UI | |
| `WantCaptureMouse / WantCaptureKeyboard / WantTextInput / CloseRequested / Hwnd` | any | |
| `void* CreateAppWindow(Platform&, const WindowDesc&, std::string* error)` | window | title (UTF-8), client size in UI units, resizable, dark title bar; attached and shown |
| `void DestroyAppWindow(Platform&)` | any | posted: the window thread destroys it, which ends `RunMessageLoop` |
| `int RunMessageLoop()` | window | `GetMessage` / `TranslateMessage` / `DispatchMessage` until `WM_QUIT` |
| `bool EnableDpiAwareness()` | any, early | per-monitor v2 unless already set |

An app window's `HandleMessage` also answers `WM_CLOSE` (`CloseRequested()` becomes true; the render thread
releases its device, then calls `DestroyAppWindow`), `WM_DESTROY` (`PostQuitMessage`), `WM_ERASEBKGND` (no flash of
a brush on resize) and the Alt key menu (`SC_KEYMENU`, except Alt+Space: there is no menu bar, and menu mode would
swallow the next keys).

A game:

```cpp
esia::platform::win32::Platform platform({.cursor = esia::platform::win32::CursorMode::WhenCapturing});
platform.Attach(hwnd);                                      // on the window thread
LRESULT CALLBACK GameWndProc(HWND w, UINT m, WPARAM wp, LPARAM lp)
{
    std::intptr_t r;
    if (platform.HandleMessage(w, m, wp, lp, &r))
        return (LRESULT)r;
    if (m >= WM_MOUSEFIRST && m <= WM_MOUSELAST && platform.WantCaptureMouse())
        return DefWindowProcW(w, m, wp, lp);                // the UI has the mouse
    ... the game's own handling ...
}
// UI thread, per frame: ctx.NewFrame(platform.Frame().Params(t)); ...; ctx.EndFrame(); render;
//                       platform.ApplyRequests(ctx.Requests());
```

The game's message loop must call `TranslateMessage` (it feeds the IME and makes `WM_CHAR`).

## 3. Threading rules

The header states them; in short, as WGT's demo had them:

1. **Window thread** = the thread that created the `HWND`. `Attach`, `Detach`, `HandleMessage`, `CreateAppWindow`
   and `RunMessageLoop` run there, and so does everything that must touch the window: capture, leave tracking,
   `SetCursor`, input contexts, the system caret, `SetWindowPos` on DPI changes. **It never waits for the render
   thread**, so the modal loop of a move or resize (inside `DefWindowProc`) never stalls rendering.
2. **UI thread** = the thread of the `Context` (and, in the example, the render thread that owns the device):
   `SetContext`, `ApplyRequests`, the clipboard callbacks. It never *sends* a message to the window (a window thread
   inside a modal loop would make it wait); requests cross as one posted, registered message (not `WM_APP`: a host
   window may use those).
3. Input crosses through `Context::QueueInput` (thread-safe); the platform holds a mutex while queueing, which is what
   lets `SetContext(nullptr)` guarantee that the context can be destroyed right after it.
4. `Frame`, the capture flags and `CloseRequested` are atomics / a small mutex: any thread.
5. Shutdown of an app window: the render thread leaves its loop on `CloseRequested`, calls `SetContext(nullptr)`,
   releases the renderer, the device and the swap chain / surface, and only then `DestroyAppWindow`; the window
   thread's `RunMessageLoop` returns and joins the render thread.
6. Swap chains are created and presented on the render thread with the window thread's `HWND`. The Direct3D hosts
   keep DXGI and D3D9 from hooking the window (`DXGI_MWA_NO_ALT_ENTER`, `D3DCREATE_NOWINDOWCHANGES`); OpenGL uses the
   window's own DC (`CS_OWNDC`) from the render thread.

## 4. The example: `glass_window`

`examples/glass_window` (root option `ESIA_BUILD_EXAMPLES`, on when Esia is the top-level project; built when
`esia_platform_win32` and at least one window-capable backend are). A smoke test of the whole path, not the demo:

```
glass_window.exe [--api d3d9|d3d10|d3d11|d3d12|opengl|vulkan] [--size 1280x800] [--vsync on|off] [--debug]
                 [--fixed-dt 0.016667] [--frames N] [--screenshot out.png] [--font file.ttf]...
```

* `--api`: the backends built in (default: the first of d3d11, d3d12, d3d10, d3d9, opengl, vulkan).
* `--size`: the client area in UI units (pixels at 100 %; the window grows with the monitor's scale).
* `--frames N`: quit after N frames; `--screenshot`: the last frame's swap-chain image read back through
  `Device::ReadPixels` into a PNG (default N = 60).
* `--debug`: the API's debug / validation layer; the number of messages is printed at exit and makes the exit code 3.
* `--fixed-dt`: the UI clock advances by that much per frame (reproducible screenshots).
* `--font`: the first file is the main font, the others fallbacks; default `%WINDIR%\Fonts\segoeui.ttf`, `msyh.ttc`,
  `simsun.ttc`. Text needs `esia_text_ft` (FreeType + HarfBuzz); without it the labels are left out and the text
  field shows one dot per character (rings for the composition).

It draws a wallpaper (gradient, moving color blobs, stripes) and three drifting glass cards (clear, frosted, tinted)
into the background list, and two core windows with a glass surface: "Controls" (a button showing its normal /
hovered / pressed state and a click count) and "Text input (IME)" (a single-line field: typing, Backspace, the inline
composition underlined with its caret, Ctrl+C / Ctrl+V, Enter prints the text to the console, Esc leaves). Both are
moved by dragging their empty area and resized from their edges. A status line shows the API, the GPU, the size in
pixels, the scale and the frame rate.

Threads: `main` creates the window and runs `RunMessageLoop` (window thread); a render thread creates the host,
`Context`, text system, `Renderer` and runs the frame loop (resize when `Frame()` changes, skip frames while
minimized, `BeginFrame` / NewFrame / scene / EndFrame / `Render` / `ApplyRequests` / `EndFrame`).

| Host | Swap chain / surface | Frames | Resize |
| --- | --- | --- | --- |
| `host_d3d11.cpp` | DXGI flip model (`FLIP_DISCARD`, 2 x BGRA8, `RENDER_TARGET_OUTPUT | SHADER_INPUT`), tearing when vsync is off and allowed | the immediate context | the wrapped target and RTV released, `ResizeBuffers` |
| `host_d3d10.cpp` | same, `D3D10CreateDevice` on the same adapter | the device | same |
| `host_d3d12.cpp` | same on the direct queue, 2 buffers, RTV heap; buffers wrapped in `PRESENT` (Esia moves them to `RENDER_TARGET` and back) | host command list, 2 frames in flight (allocator + fence value per slot), `hostFrame` | GPU idle, wrappers destroyed, `ResizeBuffers` |
| `host_d3d9.cpp` | `IDirect3DDevice9Ex`, windowed, `D3DSWAPEFFECT_DISCARD`, 1 back buffer, `Present` | the device | wrapper destroyed, `ResetEx` |
| `host_opengl.cpp` | WGL: RGBA8 double-buffered pixel format, 3.3 core context (debug context with `--debug`), `wglSwapIntervalEXT`, default framebuffer | the context | the wrapper learns the size |
| `host_vulkan.cpp` | `vulkan-1.dll` loaded at run time, `VkSurfaceKHR` + swapchain (B8G8R8A8 / R8G8B8A8 UNORM, `TRANSFER_SRC` and `SAMPLED` where allowed, FIFO or mailbox / immediate), dynamic rendering on 1.3 | 2 frames in flight: fence, acquire semaphore and command buffer per slot, a present semaphore per image; Esia records into the frame's command buffer | GPU idle, wrappers destroyed, swapchain recreated from the old one (also on `OUT_OF_DATE` / `SUBOPTIMAL`) |

The Direct3D hosts pick the high-performance adapter (`EnumAdapterByGpuPreference`). Every host renders to an 8-bit
UNORM target, so the six APIs give the same image.

## 5. What was verified, and how

Machine: Ubuntu 24.04 container, clang 18.1.3 + lld, mingw-w64 GCC 13 (posix) headers and libstdc++, Wine 9.0,
Xvfb, Mesa 25.2.8 (llvmpipe, LLVM 20.1.2), Microsoft's `d3dcompiler_47.dll` next to the executables
(`WINEDLLOVERRIDES=d3dcompiler_47=n`; taken from the `cef.redist.x64` 120.2.7 NuGet package), Vulkan headers 1.3.275
(a copy passed as `ESIA_VULKAN_INCLUDE_DIR`), FreeType 2.13.3 and HarfBuzz 10.1.0 built from source into the mingw
sysroot for the cross build (scratch, not committed).

| Check | Result |
| --- | --- |
| `cmake --preset windows-mingw-cross -DESIA_WERROR=ON` with D3D9 / 10 / 11 / 12, OpenGL, Vulkan on; clean build | platform library, its tests and `glass_window`: **0 compiler warnings**. The link of `glass_window` prints one `ld.lld` warning (duplicate `std::type_info::operator==`, section 8); the D3D backend tests and `esia_conformance.exe` **fail to link on `main` too** with this toolchain (section 8) |
| `cmake --preset linux-clang -DESIA_WERROR=ON`, build, `ctest` | 0 warnings, 6 / 6 pass; `ESIA_PLATFORM_WIN32` is not offered, `glass_window` says it is not built |
| `xvfb-run wine esia_platform_win32_tests.exe` | **17 / 17 pass**: `MouseIsInUiUnits`, `ButtonsCaptureTheMouse`, `LosingTheCaptureReleasesTheButtons`, `WheelNotches`, `KeysAndModifiers`, `KeysTheImeTakesAreNotTheUis`, `BothShiftKeysRelease`, `TextWithSurrogatePairs`, `FocusLossWhilePressing`, `SizeAndDpi`, `RequestsReachTheWindow` (cursor, IME context on / off, caret, `IMR_QUERYCHARPOSITION`), `ImeLeftToTheHost`, `Clipboard` (CRLF, Chinese), `SystemInputSettings`, `NoContextNoInput`, `AppWindowLifecycle` (title in UTF-8, DPI rect, close / destroy / loop exit), `WindowThreadAndUiThread` |
| `glass_window --api <api> --size 1100x700 --frames 30 --fixed-dt 0.016667 --screenshot ...` | d3d11, d3d10, d3d9, opengl, vulkan: image written, identical content (wallpaper, glass cards, windows, Chinese fallback font). **d3d12: does not start under Wine** - Esia's own `CreateRootSignature` fails with `E_INVALIDARG` on vkd3d, as in the DirectX session; the D3D12 host has **not run anywhere** |
| Real input end to end: `xdotool` on Xvfb -> Wine -> `WM_*` -> platform -> UI (click the button 3 times; drag the Controls window; resize the Text window from its right edge; click the field, type `Hello Esiax`, Backspace, `!`, Enter, Ctrl+C, Ctrl+V; hover the button), final frame read back | d3d11, d3d10, d3d9, opengl, vulkan: button count 3, window moved, window resized, `text field: Hello Esia!` printed, `Hello Esia!Hello Esia!` after the paste, button drawn hovered. Two findings from it are in section 7 (a drag whose moves and release fall into one frame is lost - seen when input arrives while the first frames compile; `Mods()` vs a Ctrl released in the same frame - the example now works around it) |
| The same at 144 DPI (Wine `LogPixels` = 144), opengl | window 1650 x 1050 px, scale 1.50, crisp text; every interaction above lands where it should |
| Live resize: `xdotool windowsize` six times while rendering | d3d11, d3d10, d3d9, opengl, vulkan: last frame read back at the new size (1040 x 650), no refused frame |
| `--debug` | vulkan: Wine's loader does not see the host's validation layer (says so, 0 messages); d3d10 / 11: Wine has no info queue, the backend reports that one warning (exit code 3, as designed); d3d9, opengl: 0 |

The screenshots are in `examples/glass_window/wine_screenshots/` (JPEG, for comparing by eye): `apis.jpg` (the five
APIs, 30 frames at a fixed step), `interaction_d3d11.jpg` (the last frame of the xdotool run: 3 clicks, moved and
resized windows, pasted text, hovered button), `scale150_vulkan.jpg` (144 DPI).

Not verified here: anything on Windows or a real GPU, the IME (Wine has no Microsoft Pinyin), real per-monitor DPI
changes (only a fixed Wine DPI and synthetic `WM_DPICHANGED` in the tests), D3D12, the debug layers, `clang-cl`
(`windows-clang-cl`) and MSVC builds, touch and pen, vsync / tearing behavior.

### 5.1 On Windows (the local session)

Windows 11 Pro, RTX 4080 SUPER (driver 32.0.16.1088) next to an AMD Radeon iGPU that drives the monitor (2560 x 1440 at
150 %), clang-cl 22.1.8, MSVC 19.44, Vulkan SDK 1.4.357. Branch merged with `main` (text everywhere, UI core v2) and
adapted to the core's new input API (`InputEvent::MouseLeave`, `EncodeUtf8`). Automated steps of section 6 only (6.1 -
6.2, step 3); the interactive steps (6.3 - 6.8, the IME) are the owner's.

| Check | Result |
| --- | --- |
| `windows-clang-cl` and `windows-msvc` (Debug), every backend, `-DESIA_WERROR=ON`, clean | 0 warnings once two Windows-only faults were fixed (below); `ctest` 35 / 35 on both, `esia_platform_win32_tests` 17 / 17 |
| `glass_window --api <api> --debug --size 1100x700 --frames 30 --fixed-dt 0.016667 --screenshot <api>.png` | all six APIs: exit 0, `0 validation / debug-layer messages`, 1650 x 1050 px at scale 1.50. Direct3D 10 / 11 / 12 and Vulkan run on the RTX 4080 (high-performance preference); **Direct3D 9Ex and OpenGL run on the AMD iGPU**, the adapter of the window's monitor (D3D9 only enumerates adapters with outputs, WGL takes the ICD of the window's display). **D3D12's host ran for the first time.** The six images are identical but for the status line (API, adapter, fps): mean delta 0.007 (D3D10 / 12) to 0.42 (OpenGL) against D3D11 |

Fixed on Windows:

* **`GL/wglext.h`**: the Windows SDK has none (mingw-w64 has), so `host_opengl.cpp` did not compile with clang-cl or
  MSVC; the host now defines the WGL constants it needs, as the backend's `gl_headless.cpp` does.
* **Direct3D 10 swap chain**: `CreateSwapChainForHwnd` answered `E_INVALIDARG`. A Direct3D 10.0 device is refused any
  `B8G8R8A8_UNORM` swap chain on Windows (flip or blt model, either factory call, NVIDIA and AMD alike; Wine accepts it);
  `R8G8B8A8_UNORM` works. `FlipSwapChain::Create` takes the format; D3D10 passes `R8G8B8A8_UNORM`.

## 6. Test plan for the Windows session

Windows 11, RTX 4080 SUPER, Microsoft Pinyin. Two monitors at different scales if available (e.g. 100 % and 150 %),
else change the scale in Settings while the example runs.

### 6.1 Build

```
cmake --preset windows-clang-cl -DESIA_WERROR=ON -DESIA_BACKEND_D3D9=ON -DESIA_BACKEND_D3D10=ON -DESIA_BACKEND_D3D11=ON ^
      -DESIA_BACKEND_D3D12=ON -DESIA_BACKEND_OPENGL=ON -DESIA_BACKEND_VULKAN=ON -DCMAKE_PREFIX_PATH=<vcpkg>/installed/x64-windows
cmake --build --preset windows-clang-cl
ctest --preset windows-clang-cl -R platform
```

1. The build has **no warnings** in `esia_platform_win32`, its tests and `glass_window` (clang-cl was not tried in
   the cloud). FreeType + HarfBuzz must be found (`Esia text: FreeType ... + HarfBuzz`), otherwise the IME tests
   below only show dots. Repeat with `windows-msvc` if time allows.
2. `esia_platform_win32_tests` passes (17 tests; it needs the interactive desktop).

### 6.2 Each API

For `api` in d3d11, d3d12, d3d10, d3d9, opengl, vulkan:

```
glass_window --api <api> --debug --size 1100x700 --frames 30 --fixed-dt 0.016667 --screenshot <api>.png
```

(on a 100 % monitor, for the same pixels as the cloud's screenshots)

3. Exit code 0 and `0 validation / debug-layer messages` (Graphics Tools installed for D3D; the Vulkan SDK's layer
   for Vulkan; D3D12's debug layer is on with `--debug`). The six PNGs look the same (compare with the cloud's, sec. 5).
4. Run each without `--frames` for a minute: status line fps close to the refresh rate with vsync; `--vsync off`:
   uncapped (tearing on D3D10 / 11 / 12 when the display allows, mailbox or immediate on Vulkan). No stutter while the
   cards drift. The PNGs of step 3 match `wine_screenshots/apis.jpg` in content (the fps in the status line differs).

### 6.3 Resize and move (per API, at least d3d11, d3d12, opengl, vulkan)

5. Drag the window's bottom-right corner around continuously for 10 s: the cards keep drifting during the drag (the
   render thread is not blocked by the modal size loop), the content follows the new size within a frame or two,
   no black flashes, no stretched frame that stays, no debug-layer message (`--debug`).
6. Move the window by its title bar the same way: rendering never pauses.
7. Minimize, restore, maximize, restore, Win+Left / Win+Right snaps: the right size each time, no error, no busy CPU
   while minimized.

### 6.4 DPI across monitors

8. Start on the 100 % monitor, drag the window to the 150 % one: it snaps to the size Windows suggests (the same size
   in UI units), the status line says `scale 1.50` and the new pixel size, text and glass rims are crisp, not blurry.
9. Hover and click right at the edges of the core windows (resize cursors, 5 UI units): hit testing matches what is
   drawn on both monitors. Back to 100 %: the same.
10. With the window across both monitors, and while changing the display scale in Settings: no crash, the scale
    follows the monitor that holds most of the window.

### 6.5 Mouse capture

11. Press the button, move out of the window while holding, release outside: no click counted; the button returns to
    normal.
12. Press the button, move out and back in, release over it: one click.
13. Drag a core window by its empty area past the client edge and release outside the window: the drag follows the
    whole time (right and bottom). Past the left / top edge it goes on moving, but hover is lost (section 7).
14. Right, middle, X1 / X2 buttons: no stuck buttons (hover a button afterwards; it must react normally).

### 6.6 Focus loss while pressing

15. Hold the left button on "Press me" (it shows "Pressed"), press Alt+Tab to another app, release there, come
    back: the button is back to "Press me", no click counted, nothing stuck.
16. In the text field, hold Backspace (it repeats), Alt+Tab away: the repeat stops; back: no key still held.
17. The same with the Windows key (the Start menu takes the focus) and with a notification / UAC prompt if possible.

### 6.7 IME: Microsoft Pinyin (the core of the plan)

Run `glass_window --api d3d11` (then repeat 18 - 27 on opengl and vulkan). Switch to Microsoft Pinyin (Win+Space).

18. Hover the button: typing letters does **not** open a candidate window (the IME is off outside the field).
19. Click the text field: the caret blinks; type `nihao`: the composition (pinyin or the converted characters) is
    drawn **inline** in the field, underlined, with its caret; no separate IME composition window appears.
20. The candidate window opens **just below the caret line**, left edge at the caret, and follows the caret while
    typing more syllables (`nihaoshijie`). Near the bottom of the screen it flips above the line without covering it.
21. Space commits the first candidate: `你好` appears as committed text, the composition disappears; `1`-`9` pick a
    candidate; Enter commits the raw letters; Esc cancels the composition (the field keeps its text, the focus stays).
22. Backspace inside a composition edits the composition, never the committed text; Backspace after it deletes one
    character (a whole Chinese character, not half of it).
23. Shift toggles 中 / 英: English mode types Latin letters directly; Chinese punctuation (`，。？！“”`) appears as
    typed; full-width / half-width toggle.
24. Click elsewhere (the button) during a composition: the composition is cancelled, nothing is inserted, and the
    candidate window closes. Alt+Tab during a composition: no stuck candidate window, no text lost from the field.
25. Enter (no composition) prints `text field: <text>` in the console with the Chinese correct (UTF-8 console).
    Ctrl+C in the field, paste into Notepad: the same text; copy Chinese from Notepad, Ctrl+V in the field: the same.
26. Move the window to the 150 % monitor (or change the scale): the candidate window is still at the caret.
27. Drag the Text window elsewhere while the field has the focus: the next composition's candidates are at the new
    caret position.
28. Repeat 19 - 22 with "Use previous version of Microsoft IME" on (Settings > Time & language > Chinese > Microsoft
    Pinyin > Compatibility), and with another TSF IME if one is installed (Sogou, Microsoft Japanese IME).
29. Report where the candidate window appears in each case (a screenshot is best); if it is wrong, try
    `Desc::inlineComposition = false` in `main.cpp` to see whether the IME's own composition window is placed right.

### 6.8 Keyboard, wheel, cursor

30. Tab / Shift+Tab move the keyboard focus between the button and the field (the button gets a white ring); Enter on
    the focused button does nothing (the core has no activation key yet).
31. Cursor shapes: arrow over the wallpaper, hand over the button, I-beam over the field, resize arrows at window
    edges and corners, move cursor while dragging a window. After a drag ends with the mouse still, the shape
    changes without moving the mouse.
32. Alt alone and F10 do not freeze keyboard input (no menu mode); Alt+Space opens the system menu; Alt+F4 closes
    the example cleanly (exit code 0).

## 7. Core requests

For the UI core rework on `feat/ui-core-v2` (none of these were changed here):

**After the merge with UI core v2** (the local session): 1 (`kNoMousePos`, `InputEvent::MouseLeave`), 5 (`RequestTextInput`
leaves the cursor alone) and 9 (`Context::InputPending`, `PlatformRequests::inputPending`) are in the core. The platform
and the example use the first two; the example renders every frame, so it needs no `InputPending`. 2, 3, 4, 6, 7, 8 and
10 are still open.

1. **Mouse leave is a negative position.** While captured, positions left of or above the window are legitimately
   negative; the core reads them as "outside" (`MouseValid` false: hover lost, `MouseDelta` zero, drag distance not
   updated). Wanted: an explicit leave event (or flag) so any coordinate is a position.
2. **A drag loses its last move when the release comes in the same frame.** `UpdateMoveResize` ends the drag on
   `!MouseDown` before applying the frame's position; when the moves and the release of a quick drag are applied in
   one frame (slow frames: the first frames compiling pipelines, a hitch), the window does not move at all (seen under
   Wine on D3D9's first frames). Apply the position, then end the drag - or keep a release that follows moves for the
   next frame, as `InputState` already does for a move that follows a button change.
3. **`Mods()` is the state after the frame's last key event.** A Ctrl released in the same frame as the V it held
   makes Ctrl+V read as V (seen with fast synthetic input; a fast typist at a low frame rate can do the same). Wanted:
   the modifiers of each press (`KeyPressed(Key::V, Mod_Ctrl)`), or a modifier change that follows a press in the same
   frame kept for the next frame.
4. **`Context::FindWindow` collides with `<windows.h>`**: its `FindWindow` macro renames the member to `FindWindowW`
   in any translation unit that includes `windows.h` first (a link error, or a silent mismatch). Seen in this branch.
   Rename it (`FindWindowByName`), and add it to the public-headers test that includes `windows.h` first.
5. **`RequestTextInput` turns the whole UI's cursor into an I-beam** while a field has the keyboard (it replaces
   `Arrow` whatever is hovered). The example restores the frame's shape when the field is not hovered; the I-beam
   should come from hovering the field only.
6. **Keys**: `esia::Key` has no keypad keys, no punctuation / OEM keys (minus, equals, brackets, semicolon, quote,
   comma, period, slash, backslash, grave), no Caps Lock, Print Screen, Pause, Menu key or F13 - F24; the platform
   drops them. Needed for shortcuts and games.
7. **Pointer events** (id, type mouse / touch / pen, pressure, tilt, several contacts): with them the platform can take
   `WM_POINTER` over (no promotion delay, no press-and-hold right click, multi-touch).
8. **IME clause attributes**: `ImeComposition` carries the string and the caret only. Pinyin marks the clause being
   converted (`GCS_COMPATTR` / `GCS_COMPCLAUSE`); a composition event with attribute ranges would let the editor
   draw it as Windows editors do.
9. **Input pending**: after `NewFrame`, events can stay queued for the next frame (a second change of the same button
   or key). A host that renders only when something happens (a tool) cannot know it must render another frame.
   Wanted: `bool Context::InputPending() const`, or `NewFrame` returning it.
10. **Horizontal wheel sign** is not documented in `input.hpp` (only "y > 0 = away from the user"). The platform follows
    `UpdateScroll`: x > 0 scrolls toward the left. Please document it.

## 8. Known issues

1. **mingw-w64 link, pre-existing on `main`**: `src/esia/rhi/d3d_common/d3d_util.hpp` uses `std::make_shared`, which
   with clang + mingw-w64's libstdc++ 13 defines `std::type_info::operator==` next to libstdc++'s own
   (`docs/backends/README.md` section 6). Every executable linking a D3D backend fails to link in
   `windows-mingw-cross` (`esia_conformance`, the D3D and OpenGL backend tests) - reproduced on `origin/main`.
   `glass_window` links with `-Wl,--allow-multiple-definition` (MinGW only, when a D3D backend is built), which
   leaves one linker warning. The fix belongs to the backend (`std::shared_ptr<T>(new T)`) or to the toolchain
   (`-D__GXX_TYPEINFO_EQUALITY_INLINE=0` in `clang-mingw.cmake`); once it is in, drop the option.
2. **D3D12 under Wine**: Esia's root signature is refused (vkd3d). On Windows the host runs (section 5.1).
3. The IME behavior is designed from the documentation, Chromium's and WGT's code, and checked only with synthetic
   messages: section 6.7 decides.
4. The core behaviors of section 7 that are still open (2, 3, 4, 6, 7, 8, 10).
5. The content of `glass_window`'s windows never overflows, so the wheel has nothing to scroll there: it is queued and
   unit-tested, not visible in the example.

## 9. Files

| File | |
| --- | --- |
| `include/esia/platform/win32.hpp` | the API and the threading rules |
| `src/esia/platform/win32/win32_platform.hpp / .cpp` | `Platform::Impl`: messages -> input, requests, IME, DPI |
| `src/esia/platform/win32/win32_window.cpp` | app window, message loop, DPI awareness, clipboard, `Configure` |
| `src/esia/platform/win32/tests/test_win32_platform.cpp` | 17 tests on a hidden window |
| `src/esia/platform/win32/CMakeLists.txt`, `src/esia/CMakeLists.txt` | `esia_platform_win32`, `ESIA_PLATFORM_WIN32` |
| `examples/glass_window/main.cpp` | options, window thread, render thread, frame loop, screenshot |
| `examples/glass_window/scene.hpp / .cpp` | wallpaper, glass cards, the two core windows, button, text field |
| `examples/glass_window/host.hpp / .cpp` | the host interface and the table of built-in APIs |
| `examples/glass_window/host_d3d9 / d3d10 / d3d11 / d3d12 / opengl / vulkan.cpp`, `dxgi_swap_chain.*` | one host per API |
| `examples/glass_window/wine_screenshots/*.jpg` | what the cloud session saw under Wine (section 5) |
| `CMakeLists.txt` | `ESIA_BUILD_EXAMPLES` |
