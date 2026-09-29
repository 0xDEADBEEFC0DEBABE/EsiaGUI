# Esia rewrite - status of `esia-core`

Where the ImGui-free rewrite stands after core round 3 (2026-09-29). The plan is [REWRITE.md](REWRITE.md); how to
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
3. **SM3 size.** The full FX pixel shader is ~5.4k `ps_3_0` instruction slots, a plain fill ~450; SM3 guarantees
   512. On a device limited to that, most variants cannot be created, and neither can the full shader - the fallback
   of refused and pending variants - so those batches are dropped (counted in `RenderStats::fxFallbacks`).
4. **Glass over the capture budget** reuses the last capture: the pyramid levels are now loaded rather than
   undefined, and the backdrop copy holds the previous frames' content outside this frame's captures (stale but
   defined).
5. **Text.** No Unicode bidi algorithm, no color glyphs, no system fonts, no caret / grapheme query yet; glyphs are
   rasterized before the clip test. The text tests use the fonts in `third_party/imgui/misc/fonts`: move them before
   `third_party/imgui` is removed (phase 6).
6. **Zero-filled uploads.** `TextureRegistry::Create(info, nullptr)` queues a zero-filled CPU copy of the whole
   texture until the renderer consumes it (4 MB for a 2048 x 2048 glyph page).
7. **No device-loss protocol.** After a lost device (D3D TDR, a lost GL context) images must be supplied again by
   their owners.

## 7. Building and testing

### Linux (what the cloud sessions use)

```bash
sudo apt-get install -y clang lld cmake ninja-build python3 \
    glslang-tools spirv-cross spirv-tools \
    libegl-dev libgles-dev libgl-dev mesa-utils \
    libfreetype-dev libharfbuzz-dev
cmake --preset linux-clang && cmake --build --preset linux-clang && ctest --preset linux-clang
# release: linux-clang-release; warnings as errors: -DESIA_WERROR=ON; a backend: -DESIA_BACKEND_OPENGL=ON
python3 tools/shaders/build_shaders.py          # after a shader change: regenerate the library (commit the output)
python3 tools/shaders/build_shaders.py --check  # CI: the checked-in library matches the sources
build/linux-clang/bin/esia_conformance --backend null --golden tests/conformance/golden --strict
```

Without `libegl-dev` the GLSL link test is left out; without FreeType / HarfBuzz the text system and its tests are.
Vulkan on the CPU: `apt-get install mesa-vulkan-drivers libvulkan-dev vulkan-validationlayers` (lavapipe). The D3D
backends under Wine: `apt-get install wine64 xvfb`, build `windows-mingw-cross`, put Microsoft's
`d3dcompiler_47.dll` next to the executables, then `WINEDLLOVERRIDES=d3dcompiler_47=n xvfb-run -a wine
build/windows-mingw-cross/bin/esia_conformance.exe --backend d3d11 --golden tests/conformance/golden --strict`.

### Windows with LLVM

* **On Windows**: the LLVM installer (`clang-cl`, `lld-link` on `PATH`) plus the MSVC build tools and Windows SDK for
  the STL, CRT and headers; from an "x64 Native Tools Command Prompt":
  `cmake --preset windows-clang-cl && cmake --build --preset windows-clang-cl && ctest --preset windows-clang-cl`.
  For the text system, point `CMAKE_PREFIX_PATH` at a FreeType + HarfBuzz installation (e.g. vcpkg's installed tree);
  without it `esia_text_ft` is skipped. The local session used clang-cl 22.1.8 and MSVC 19.44.
* **From Linux, MSVC ABI** (`windows-cross`): `cargo install xwin --locked`, then
  `xwin --accept-license --arch x86_64 splat --output ~/.xwin` (needs download.visualstudio.microsoft.com),
  `cmake --preset windows-cross -DXWIN_DIR=$HOME/.xwin && cmake --build --preset windows-cross` (tests off: they
  cannot run on Linux).
* **From Linux, offline** (`windows-mingw-cross`, GNU ABI - a compile / link check of the same code, see the guide
  for its `type_info::operator==` trap): `sudo apt-get install g++-mingw-w64-x86-64-posix mingw-w64-x86-64-dev
  wine64`, then `cmake --preset windows-mingw-cross && cmake --build --preset windows-mingw-cross` and, optionally,
  `wine build/windows-mingw-cross/bin/esia_core_tests.exe` (and the other test executables).
* **Today's `wgt.dll`**, unchanged: `cmake --preset vs2022 && cmake --build --preset release` with MSVC.

### macOS

`brew install llvm lld cmake ninja glslang spirv-cross spirv-tools freetype harfbuzz`, then
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
| Toolchains | `CMakePresets.json`, `cmake/toolchains` | `linux-clang`, `linux-clang-release`, `macos-clang`, `windows-clang-cl`, `windows-cross` (clang-cl + lld-link + xwin), `windows-mingw-cross` (clang + mingw-w64); the MSVC presets `vs2022` / `release` / `debug` unchanged |

Round-1 commits (oldest first): `51d88dc` presets, toolchains, UI core - `4f36cef` RHI, null backend, shader library -
`55e04ae` delta time from a clock at 0 - `c8a3372` pass and color contract, host callbacks - `c845be3` Painter, planner,
renderer - `8ebb8df` SDF fix - `2283d72` conformance suite - `94d89bf` D3D9 hooks, FX feature variants - `1f1443a`
mingw-w64 cross build - `653dd4f` shader library and GLSL / ESSL link tests - `9d7fd4b` ESSL at highp - `4c9af5c`
runtime shader sources - `4c35054` clip-position hook, threading rules - `871a86c` REWRITE.md and the guide - `d3768bb`
rasterizer, atlas - `09f4b27` FreeType + HarfBuzz - `331bddc` the null goldens tracked - `79c12ef` `[cloud-done]`.

## 9. Next

1. **Backends**: merge `esia-core` into each backend branch and delete the dual-source code (section 0, "Backend
   follow-ups"); then merge the backends (OpenGL first), each passing `--strict` and `--frames 3` with
   `ValidationErrors` implemented.
2. **Measure** Esia against WGT's legacy D3D11 backend on the same UI (needs the widget port or a scene in WGT's API);
   run Metal on a Mac.
3. **Core follow-ups**: device loss (section 6, item 7), empty-pixel creates (item 6), splitting heavy SM3 glass
   batches, DXBC generated and checked in.
4. **Phase 2, text**: the bidi algorithm, color glyphs, a caret / grapheme query; DirectWrite and Core Text behind
   the interface, feeding the shared rasterizer.
5. **Phases 3 - 6**: widgets on the core, platform layers and services, the `wgt::` compatibility layer and
   `wgt.dll` 2.0 without Dear ImGui, then removing `third_party/imgui` (REWRITE.md section 15).
