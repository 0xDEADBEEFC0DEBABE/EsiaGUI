# Esia Direct3D backends - status of `esia-directx`

The Direct3D 9, 10, 11 and 12 backends of the Esia RHI (`docs/backends/README.md`), written in a Linux cloud
session without a GPU or Windows (2026-09-29). They build warning-free in every option combination with the
`windows-mingw-cross` toolchain, and **everything below that says "ran" ran under Wine 9.0 on Mesa's software
renderers** (wined3d on llvmpipe for D3D9 / 10 / 11, vkd3d on lavapipe for D3D12) with Microsoft's
`d3dcompiler_47.dll`. There, all four backends pass every conformance scene against the OpenGL session's image
goldens (D3D9 SKIPs `text_lcd`: no dual-source blending). **Nothing has run on a real Windows driver or with the
Direct3D debug layers**: that is the local session's job (section 7).

* [1. Implemented](#1-implemented)
* [2. Conformance](#2-conformance)
* [3. Verified here](#3-verified-here)
* [4. Not verified](#4-not-verified)
* [5. Known issues](#5-known-issues)
* [6. Core change requests](#6-core-change-requests)
* [7. Windows test plan](#7-windows-test-plan)
* [8. Files](#8-files)

## 1. Implemented

| | Direct3D 9 (9Ex) | Direct3D 10 / 10.1 | Direct3D 11 | Direct3D 12 |
| --- | --- | --- | --- | --- |
| CMake option / target | `ESIA_BACKEND_D3D9` / `esia_rhi_d3d9` | `ESIA_BACKEND_D3D10` / `esia_rhi_d3d10` | `ESIA_BACKEND_D3D11` / `esia_rhi_d3d11` | `ESIA_BACKEND_D3D12` / `esia_rhi_d3d12` |
| Host header | `esia/rhi/d3d9.hpp` | `esia/rhi/d3d10.hpp` | `esia/rhi/d3d11.hpp` | `esia/rhi/d3d12.hpp` |
| Device creation | `CreateDevice(IDirect3DDevice9*)` | `CreateDevice(ID3D10Device*)` | `CreateDevice(ID3D11Device*, ID3D11DeviceContext*)` (FL 11_0) | `CreateDevice({device, queue, framesInFlight})` |
| Target wrapping | `WrapRenderTarget(dev, surface, texture = null, srgb = false)` | `WrapRenderTarget(dev, rtv)` | `WrapRenderTarget(dev, rtv)` | `WrapRenderTarget(dev, resource, rtv, format, stateOnEntryAndExit)` |
| `FrameDesc::nativeContext` | null | null | null | the host's `ID3D12GraphicsCommandList*`, or null to record on the device's own list (needs `queue`) |
| Host state | state block + RT0 + depth surface saved / restored | saved / restored | saved / restored | the host rebinds its heaps / root signature |
| Shaders | runtime `D3DCompile`, `vs/ps_3_0` + SM3 prelude, FX per feature mask | `vs/ps_4_0`, FX texture storage | `vs/ps_5_0` | `vs/ps_5_0` DXBC |
| FX instances | dynamic `A32B32G32R32F` texture, vertex texture fetch | RGBA32F texture at t7 (both stages) | structured buffer SRV (both stages) | root SRV (structured buffer in the upload ring) |
| Constants | `Set*ShaderConstantF` at the registers of each shader's CTAB | dynamic CBs, `WRITE_DISCARD` per `SetConstants` | same | root CBVs into the per-frame upload ring |
| Instanced quads | `SetStreamSourceFreq` over corner / instance id streams | `DrawInstanced` | `DrawInstanced` | `DrawInstanced` |
| Backdrop copy | `StretchRect` (copy destination promoted to a render-target texture) | `CopySubresourceRegion` / `ResolveSubresource`, typeless 8-bit storage | same | `CopyTextureRegion` / `ResolveSubresource` + barriers |
| Direct read (no copy) | host target given with its texture | SRV of the target (raw view of sRGB) | same | same + `RENDER_TARGET -> PIXEL_SHADER_RESOURCE` |
| sRGB targets | `D3DRS_SRGBWRITEENABLE` | sRGB RTV on typeless storage | same | same |
| MSAA targets | multisampled render-target surface, `StretchRect` resolve | resolve | resolve | resolve |
| Dual-source (sub-pixel text) | no (grayscale coverage) | yes | yes | yes |
| Float render targets | if `A16B16G16R16F` blends and filters | yes | yes | yes |
| Half-pixel offset | yes (`gEsiaHalfPixel` per pass target) | no | no | no |
| GPU timestamps | `TIMESTAMP` / `DISJOINT` / `FREQ` queries | timestamp queries | timestamp queries (`DONOTFLUSH`) | query heap resolved per frame slot |
| Readback | `GetRenderTargetData` / `LockRect` | staging copy | staging copy | readback buffer on the device's queue |
| User effects | yes (worker thread) | yes | yes | yes |
| `fxFeatureVariants` | yes (required) | no | no | no |
| Debug messages | failed calls only (no info queue on Windows 10 / 11) | `ID3D10InfoQueue` | `ID3D11InfoQueue` | `ID3D12InfoQueue` |

Shared (`src/esia/rhi/d3d_common`): a COM pointer, logging (`d3d::DebugDesc`: a log callback and the
debug-layer switch), handle tables, the DXGI format tables (8-bit RGBA / BGRA stored typeless so a format and
its `RawFormat` share bits), readback conversion to RGBA8, timestamp accounting, and runtime HLSL compilation:
`D3DCompile` loaded from `d3dcompiler_47.dll` at runtime (a missing DLL is a SKIP, not a failed start), an
include handler serving `shaders::FindSource` and user effects, a process-wide bytecode cache (the SM4 / SM5 FX
pixel shader takes 2.3 - 2.6 s to compile), user effects compiled on worker threads (`CreatePipeline` returns
`{}` until they are ready; a rejected source never yields a pipeline and never blocks).

Every checklist item of `docs/backends/README.md` section 8: 1 (warning-free, all combinations), 2
(`esia_conformance --list` shows `null d3d9 d3d10 d3d11 d3d12`), 3 (every scene passes or SKIPs for a capability
reason - under Wine, see 2), 4 (the backends follow the null device's rules; the renderer drives them unchanged),
5 (host headers), 6 (timestamps on all four; `ReadProfile` returns frames under Wine), 7 (only files under
`src/esia/rhi/d3d*` were added, plus a merge of `esia-core`).

Headless devices (the conformance suite): D3D9Ex on a hidden window; D3D10.1 (then 10.0); D3D11 (11_1, then
11_0); D3D12 (11_0) with its own queue. `ESIA_D3D_DEBUG=1` enables the debug layer (falls back without it when
the Graphics Tools are not installed), `ESIA_D3D_DEBUG=2` also D3D12's GPU-based validation, and
`ESIA_D3D_DRIVER=warp` uses WARP for D3D10 / 11 / 12.

## 2. Conformance

Run under Wine 9.0 + Xvfb against the OpenGL session's goldens (`origin/esia-opengl` at `cc75e82`, extracted to
`/tmp/gl`, not committed), with `--strict`:

| Scene | d3d9 | d3d10 | d3d11 | d3d12 |
| --- | --- | --- | --- | --- |
| shapes | PASS (max delta 1) | PASS (1) | PASS (1) | PASS (1) |
| gradients | PASS (1) | PASS (1) | PASS (1) | PASS (1) |
| shadows | PASS (1) | PASS (1) | PASS (1) | PASS (1) |
| glass | PASS (1) | PASS (1) | PASS (1) | PASS (3) |
| glow_layer | PASS (1) | PASS (1) | PASS (1) | PASS (1) |
| text | PASS (0) | PASS (0) | PASS (0) | PASS (0) |
| text_lcd | SKIP: no dual-source blending | PASS (1) | PASS (1) | PASS (1) |
| edge_fade | PASS (0) | PASS (0) | PASS (0) | PASS (0) |
| clipping | PASS (0) | PASS (0) | PASS (0) | PASS (0) |
| windows | PASS (1) | PASS (1) | PASS (1) | PASS (3) |
| hidpi | PASS (1) | PASS (1) | PASS (1) | PASS (4) |
| light_streak | PASS (2) | PASS (2) | PASS (2) | PASS (3) |
| srgb_target | PASS (1) | PASS (2) | PASS (2) | PASS (4) |
| msaa_target | PASS (1) | PASS (1) | PASS (1) | PASS (3) |

"0.000 % pixels over the tolerance" everywhere; the numbers are the largest channel difference (of 255). The
images were also inspected by eye (contact sheets of every scene per backend). Before the goldens existed the
backends were compared with each other: D3D10 is pixel-identical to D3D11 (so the texture-storage FX path
matches the structured buffer), D3D12 within 4, D3D9 within 2.

These are results on software renderers through translation layers - evidence that the backends drive the RHI
and the shaders correctly, not proof of correct behaviour on real drivers.

## 3. Verified here

Machine: Ubuntu 24.04 container, no GPU. clang 18.1.3, lld 18, CMake 3.28, mingw-w64 11 (GCC 13 posix
headers / import libraries), Wine 9.0 (wined3d, vkd3d), Mesa 25.2.8 (llvmpipe, lavapipe), Xvfb.
Microsoft's `d3dcompiler_47.dll` (6.3.9600.16384) was taken from Qt's `PyQt5_Qt5-5.15.2-py3-none-win_amd64.whl`
on PyPI (Qt redistributes it) and placed next to the test executables with `WINEDLLOVERRIDES=d3dcompiler_47=n`
- never committed. Wine's own `d3dcompiler_47` cannot compile `esia_fx.hlsl` (`docs/REWRITE.md` section 8).

| Command | Result |
| --- | --- |
| `cmake --preset linux-clang -DESIA_WERROR=ON && cmake --build --preset linux-clang && ctest --preset linux-clang` | 0 warnings, 8 / 8 pass (the D3D options are off; nothing of this branch is built on Linux - turning one on there stops with a message) |
| the 15 combinations of `ESIA_BACKEND_D3D9/10/11/12` with `cmake/toolchains/clang-mingw.cmake`, `-DESIA_WERROR=ON`, Debug | all build, 0 warnings; `Esia backends:` lists exactly the enabled ones |
| all four ON, Release | builds, 0 warnings |
| `wine esia_rhi_d3d_shader_tests.exe` | 4 / 4: every program compiles for `vs/ps_4_0` and `vs/ps_5_0`, cache hits, a feature variant, an async user effect, a rejected effect; format and readback helpers; profile accounting |
| `wine esia_rhi_d3d_device_tests.exe` (Xvfb) | 4 / 4 on d3d9, d3d10, d3d11, d3d12: every format uploaded and read back, region updates, raw copies sRGB -> RawFormat, MSAA resolves (copy and readback), sRGB clears store the given value, a scissored `Clear`-program draw, timestamps (`ReadProfile` returns a frame), user-effect pipelines |
| `wine esia_rhi_d3d9_tests.exe` | 2 / 2: every SM3 program and nine FX masks compile (glass, caustic, image, noise, halo ...); a host surface wrapped (sRGB flag), cleared, read back; host viewport / render state / render target restored |
| `wine esia_rhi_d3d10_tests.exe`, `esia_rhi_d3d11_tests.exe` | 1 / 1 each: an sRGB view of typeless storage wrapped (cached, sampleable), cleared, read back; host viewport and topology restored |
| `VKD3D_CONFIG=virtual_heaps wine esia_rhi_d3d12_tests.exe` | 1 / 1: three frames recorded into a host command list for a wrapped BGRA target (returned to COMMON), executed by the host, read back |
| `wine esia_conformance.exe --backend <b> --golden /tmp/gl/tests/conformance/golden --strict` | section 2 |
| `wine esia_conformance.exe --backend null --golden tests/conformance/golden --strict`, the core test executables | 14 scenes pass; 38 + 10 + 36 + 5 + 1 tests pass |
| `ESIA_D3D_DEBUG=1 wine esia_conformance.exe --scene glass` | runs; compile warnings are logged (Wine has no debug layer to report) |

Under Wine, D3D12 needs `VKD3D_CONFIG=virtual_heaps`: Wine 9.0's vkd3d otherwise wants more descriptor sets for
the root signature than lavapipe's 8 (`vkd3d_validate_descriptor_set_count`). Real D3D12 drivers are not
affected.

Tried and blocked: the `windows-cross` preset (clang-cl + xwin) - the egress proxy answers 403 for
`download.visualstudio.microsoft.com` and `aka.ms`, so xwin cannot fetch the MSVC CRT / Windows SDK (the core
session hit the same). The MSVC-ABI build (clang-cl, MSVC) is therefore unverified.

## 4. Not verified

* **Any real Windows driver** (the RTX 4080 SUPER run is the local session's), and the D3D debug layers:
  D3D12 barrier / state tracking, descriptor lifetimes and the host-command-list mode were exercised by vkd3d
  only; wined3d is far more permissive than real D3D9 / 10 / 11 drivers.
* **clang-cl and MSVC builds** (Windows SDK headers instead of mingw-w64's; MSVC `/W4` warnings such as C4458 /
  C4244 that clang does not have).
* **D3D9 on real hardware**: wined3d reports 512 pixel-shader instruction slots but does not enforce them; the
  glass variants of the FX shader need about 3.6k - 4k (NVIDIA / AMD SM3+ parts report 32768). Vertex texture
  fetch of `A32B32G32R32F`, `StretchRect` from `X8R8G8B8` back buffers into `A8R8G8B8`, `UpdateSurface` into
  render-target textures, and `D3DRS_SRGBWRITEENABLE` behaviour are driver territory.
* **Performance**: nothing was timed on a GPU; the timestamps only proved to arrive.
* The `D3D10_1` device path with a real 10.0-only GPU; WARP (`ESIA_D3D_DRIVER=warp`) was never run.

## 5. Known issues

1. **SM3 cannot hold every FX feature combination.** Measured with Microsoft's compiler: every mask without
   glass fits `ps_3_0` (all 14 other features together: 1.7k slots); glass fits with up to about seven other
   features (`0xFF`: 4k slots), but e.g. glass + stroke + shadow + glow + inner glow + image + shimmer + inner
   shadow (`0x1FF`) exceeds the 32 temporary registers (X4505; `D3DCOMPILE_PREFER_FLOW_CONTROL`,
   `SKIP_OPTIMIZATION`, `IEEE_STRICTNESS` do not help). Such a batch gets no pipeline and is not drawn (the
   renderer skips batches without a pipeline). See core request 5.
2. **Two core defects worked around** in the backends: fxc cannot `#include` a macro (the SM3 prelude is
   prepended to the source), and one feature test in `esia_fx.hlsl` uses integer bit operations (patched in the
   source text for SM3 only, a no-op once fixed). Core requests 1 and 2.
3. **SM3 float arithmetic in `FxFetch`**: `instance % perRow` is a float modulo on SM3 and fxc's is inexact
   (6 % 682 = 5.9999, truncated to the neighbour's texel: two shapes of the `shapes` scene vanished). The D3D9
   backend reports `maxFxDataWidth = 24 * 2^k`, making `perRow` a power of two and the arithmetic exact.
4. **Multisampled `CopyTexture`**: D3D10 / 11 / 12 resolve the whole surface when the destination has the
   source's size and position (the backdrop copy); texels outside the region then get the same, newer source
   texels, which nothing reads. Other destinations go through a temporary resolve.
5. **D3D9 copy destinations** are created dynamic and become render-target textures at their first
   `CopyTexture` (`StretchRect` only writes render targets); contents uploaded before that are dropped (logged).
   The renderer never uploads into its backdrop copy.
6. **D3D12 host command lists**: the ring slot of a device frame is reused `framesInFlight` device frames later
   without a fence (the host's frame pacing is trusted, as in WGT's backend); count one device frame per
   `Renderer::Render`. `ReadPixels` needs `Desc::queue` and must follow the host's execution of the frame.
7. **D3D10 / 11 flush at `EndFrame`** when timestamps are on, so their queries complete without a `Present`.
8. **Wrapped host targets hold a reference** (view / resource / surface): `DestroyTexture` them before
   `ResizeBuffers` / `ResetEx`.
9. **Shader compile time**: the FX pixel shader compiles in about 2.4 s (SM4 / SM5) or 0.3 - 6 s per variant
   (SM3), once per process (no disk cache); D3D9 compiles variants lazily, the first frame that meets a new mask
   stalls. DXBC generated with fxc on Windows and checked in (core follow-up) removes the SM4 / SM5 cost.
10. **User-effect workers are detached threads**: a process exiting during a compile ends them (the cache is
    never destroyed, so nothing dangles).
11. **Direct3D 9 has no debug layer on Windows 10 / 11**: `DebugDesc::debugLayer` only adds compile logs there.
12. `X3203 signed/unsigned mismatch` warnings of the SM3 compiles appear in the log with `ESIA_D3D_DEBUG` (the
    shared sources compare float-emulated uints with unsigned literals); harmless.
13. No device-loss handling (core known issue 6): a D3D9 device must be 9Ex in practice, a removed D3D10 / 11 /
    12 device fails every call.

## 6. Core change requests

None of these were made on this branch (the rules forbid it); each is the smallest change that would do.

1. **`esia_common.hlsli`: `#include ESIA_SHADER_PRELUDE` does not work with fxc / `D3DCompile`** - their
   preprocessor rejects a macro after `#include` (`error X1500: syntax error: unexpected token`, reproduced with
   `d3dcompiler_47` 6.3.9600). `tools/shaders/build_shaders.py --fxc` would fail the same way for `dxbc_sm3`.
   Smallest change: `#ifdef ESIA_SHADER_PRELUDE` / `#include "esia_shader_prelude.hlsli"` / `#endif` (a fixed
   name, found through `/I` or the include handler), and the D3D9 prelude renamed or served under that name;
   or document that the prelude is prepended (what this backend does) and make `build_shaders.py` compile a
   temporary file that concatenates prelude and source.
2. **`esia_fx.hlsl` line 719**: `[branch] if (feat & (F_GLOW | F_SHADOW))` uses integer bit operations (SM3 has
   none) and ignores `ESIA_FX_FEATURES`. Change to `[branch] if (FX_HAS(feat, F_GLOW) || FX_HAS(feat, F_SHADOW))`.
   The D3D9 backend patches exactly this text before compiling (`kPatches` in `d3d9_device.cpp`).
3. **`docs/backends/README.md`, "Direct3D 9"**: the guide suggests a vertex-shader define (`ESIA_SM3_VS`) to put
   `gFxData` at `s0`. Not needed and not what `build_shaders.py` does (it passes no stage define): the prelude
   leaves all registers to the compiler and the backend binds by name from the CTAB. Also worth a sentence:
   `maxFxDataWidth` must be 24 times a power of two on SM3 (known issue 3), and `uint` must be defined as `float`
   in the prelude (fxc X3548 on the shared sources' uints).
4. **DXBC on Linux** (suggestion): `d3dcompiler_47.dll` from Qt's Windows wheels on PyPI runs under Wine and
   compiles every program (SM3 / SM4 / SM5), unlike Wine's own. `build_shaders.py` could generate the DXBC in
   the cloud with a small `D3DCompile` wrapper executable (the equivalent of `fxc /O3 /Ges|/Gec`), removing the
   runtime compile cost (known issue 9).
5. **Heavy glass batches on SM3** (known issue 1): when an `Fx` pipeline with `fxFeatures` cannot be created, the
   renderer could split the batch by feature mask (e.g. draw its glass instances and its other instances as two
   batches), or the FX shader could reuse temporaries between the glass and the inner-shadow / shimmer paths.
   Only D3D9 needs it.

## 7. Windows test plan

For the local session on Windows 11 (NVIDIA RTX 4080 SUPER). Merge `esia-directx` (and `esia-opengl` for the
image goldens) or check out `esia-directx` and extract the goldens: `git archive origin/esia-opengl
tests/conformance/golden | tar -x -C %TEMP%\gl`.

**Debug layers** (install once): Settings > System > Optional features > "Graphics Tools" (or
`dism /online /add-capability /capabilityname:Tools.Graphics.DirectX~~~~0.0.1.0`). Then, for every command below:
`set ESIA_D3D_DEBUG=1` (D3D10 / 11 / 12 debug layers, messages printed as `esia d3d warning/error: D3D1x debug
layer: ...`); `set ESIA_D3D_DEBUG=2` adds D3D12 GPU-based validation (slow). D3D9 has no debug runtime on
Windows 10 / 11 (only failed calls are logged); PIX or RenderDoc captures are the tools there. Optional:
`set ESIA_D3D_DRIVER=warp` runs D3D10 / 11 / 12 on WARP for a second reference.

**Build with clang-cl** (x64 Native Tools prompt, LLVM on PATH):

```bat
cmake --preset windows-clang-cl -DESIA_WERROR=ON -DESIA_BACKEND_D3D9=ON -DESIA_BACKEND_D3D10=ON -DESIA_BACKEND_D3D11=ON -DESIA_BACKEND_D3D12=ON
cmake --build --preset windows-clang-cl
ctest --preset windows-clang-cl --output-on-failure
```

**Build with MSVC** (a separate directory: the `vs2022` preset's `build/` is the legacy `wgt.dll`):

```bat
cmake -S . -B build/msvc-esia -G "Visual Studio 17 2022" -A x64 -DWGT_BUILD_LEGACY=OFF -DESIA_BUILD_CORE=ON -DESIA_WERROR=ON ^
      -DESIA_BACKEND_D3D9=ON -DESIA_BACKEND_D3D10=ON -DESIA_BACKEND_D3D11=ON -DESIA_BACKEND_D3D12=ON
cmake --build build/msvc-esia --config Debug
ctest --test-dir build/msvc-esia -C Debug --output-on-failure
```

Also build a few single options (`-DESIA_BACKEND_D3D9=ON` alone, `-DESIA_BACKEND_D3D12=ON` alone) and check that
the legacy build is unchanged (`cmake --preset vs2022 && cmake --build --preset release`).

**Tests** (`bin` for Ninja, `bin\Debug` for MSVC), each with `ESIA_D3D_DEBUG=1`, expecting no debug-layer
message at warning or error level:

```bat
bin\esia_rhi_d3d_shader_tests.exe
bin\esia_rhi_d3d_device_tests.exe
bin\esia_rhi_d3d9_tests.exe
bin\esia_rhi_d3d10_tests.exe
bin\esia_rhi_d3d11_tests.exe
bin\esia_rhi_d3d12_tests.exe
bin\esia_conformance.exe --list
bin\esia_conformance.exe --backend d3d11 --golden %TEMP%\gl\tests\conformance\golden --out out --strict
bin\esia_conformance.exe --backend d3d12 --golden %TEMP%\gl\tests\conformance\golden --out out --strict
bin\esia_conformance.exe --backend d3d10 --golden %TEMP%\gl\tests\conformance\golden --out out --strict
bin\esia_conformance.exe --backend d3d9  --golden %TEMP%\gl\tests\conformance\golden --out out --strict
```

(`ctest` runs `esia_conformance_d3d9 ... _d3d12` too, against `tests/conformance/golden`, without `--strict`.)
Expected: every scene PASS, `text_lcd` SKIP on d3d9 only. On a failure, look at `out\<backend>\<scene>.png` and
`.diff.png`.

**What to watch in particular**

* D3D12 debug layer: resource-state errors (copies, resolves, `ReadPixels`), descriptor heap and root signature
  messages, and the host-command-list test (`esia_rhi_d3d12_tests`); GPU-based validation once.
* D3D11 / 10: SRV / RTV hazard warnings between pyramid passes, `ResolveSubresource` format errors on the
  `msaa_target` and `srgb_target` scenes.
* D3D9: that `glass` / `windows` / `hidpi` / `light_streak` draw their glass (the variants must fit the GPU's
  instruction slots), the half-pixel alignment (compare `shapes` and `text` at max delta <= 1), and
  `esia_rhi_d3d9_tests` (host state restore).
* Timings: `esia_rhi_d3d_device_tests` checks that timestamps arrive; a real UI frame's `RenderStats::gpu` per
  category (capture / layer / fx / fx-glass / geometry) is worth a look in a host.
* Compile times of the first frame (`ESIA_D3D_DEBUG=1` logs every compile with its duration).

## 8. Files

```
src/esia/rhi/d3d_common/  CMakeLists.txt, include/esia/rhi/d3d_common.hpp (host types), d3d_util.hpp/.cpp,
                          d3d_shader.hpp/.cpp, tests/test_d3d_shaders.cpp, tests/test_d3d_devices.cpp, STATUS.md
src/esia/rhi/d3d9/        CMakeLists.txt, include/esia/rhi/d3d9.hpp, d3d9_device.cpp, esia_sm3_prelude.hlsli,
                          tests/test_d3d9.cpp
src/esia/rhi/d3d10/       CMakeLists.txt, include/esia/rhi/d3d10.hpp, d3d10_device.cpp, tests/test_d3d10_host.cpp
src/esia/rhi/d3d11/       CMakeLists.txt, include/esia/rhi/d3d11.hpp, d3d11_device.cpp, tests/test_d3d11_host.cpp
src/esia/rhi/d3d12/       CMakeLists.txt, include/esia/rhi/d3d12.hpp, d3d12_device.cpp, tests/test_d3d12_host.cpp
```
