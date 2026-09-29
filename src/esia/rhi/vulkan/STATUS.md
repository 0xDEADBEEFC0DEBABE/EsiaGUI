# Esia Vulkan backend - status of `esia-vulkan`

Where the Vulkan backend stands at the end of its cloud session (2026-09-29). The contract it implements is
`include/esia/rhi/rhi.hpp` and [docs/backends/README.md](../../../../docs/backends/README.md); the host API is
[include/esia/rhi/vulkan.hpp](include/esia/rhi/vulkan.hpp).

**In one paragraph.** The backend is complete for what the renderer needs: Vulkan 1.1+, dynamic rendering where the
device has it (core 1.3 or `VK_KHR_dynamic_rendering`) and render passes otherwise, every RHI call implemented.
On Mesa lavapipe it passes **all 14 conformance scenes against the OpenGL session's goldens with `--strict`** (max
channel delta 4, 0 % of pixels over tolerance), on both render paths, with the Khronos validation layer 1.3.275
and synchronization validation reporting **0 errors and 0 warnings**. The core's SPIR-V was tested first: every
program becomes a pipeline for every target format and 1 / 4 samples. Nothing was run on a real GPU, on Windows or
on macOS. Based on `esia-core` at `3810b97` (its latest commit).

* [1. Implemented](#1-implemented)
* [2. Conformance scenes](#2-conformance-scenes)
* [3. Verified here](#3-verified-here)
* [4. Not verified](#4-not-verified)
* [5. Known issues](#5-known-issues)
* [6. Building and testing on the real platforms](#6-building-and-testing-on-the-real-platforms)
* [7. Core change requests](#7-core-change-requests)
* [8. Checklist (backends/README.md section 8)](#8-checklist-backendsreadmemd-section-8)

## 1. Implemented

Files (nothing outside this directory changed):

| File | What |
| --- | --- |
| `CMakeLists.txt` | `esia_rhi_vulkan` (needs only the Vulkan headers), `esia_register_backend(vulkan ...)`, the unit tests, a second conformance CTest on the render-pass path |
| `include/esia/rhi/vulkan.hpp` | host integration: `CreateDevice(Desc)`, `WrapImage`, `UsesDynamicRendering` / `RenderPassFor` (host callbacks), `ToVkFormat` / `FromVkFormat`, `CreateHeadless` with `HeadlessOptions`, `ValidationMessages` |
| `vk_loader.hpp/.cpp` | the function table, loaded through `vkGetInstanceProcAddr` from the host or from the system loader opened at runtime (`libvulkan.so.1`, `vulkan-1.dll`, `libvulkan.1.dylib`): no link-time dependency, binaries start without Vulkan and the suite SKIPs |
| `vk_formats.hpp/.cpp` | RHI <-> `VkFormat` (RGB10A2 = `A2B10G10R10_UNORM_PACK32`), raw views, readback conversion to RGBA8 |
| `vk_device.hpp/.cpp` | the `rhi::Device` implementation |
| `vk_headless.cpp` | instance / device creation for tests (validation layer + sync validation + debug messenger), `CreateDevice`, registration |
| `tests/` | 14 unit tests (formats, device, scenes, host integration) |

Everything the task lists:

* **Instanced SDF / FX quads with per-draw constants**: `vkCmdDraw(4, n)` on a strip, FX instances in a storage
  buffer at binding 10 (`Caps::fxStorage = Buffer`), Frame / Pass / Draw constants as `UNIFORM_BUFFER_DYNAMIC`
  bindings 0-2 with offsets into a per-frame-slot uniform ring (inline: every draw sees the latest values).
* **Indexed geometry and text**: the 20-byte UI vertex layout, 32-bit indices; `TextLcd` with dual-source blending
  (`dualSrcBlend` feature), the gray programs otherwise.
* **Textures and render targets**: one-mip 2D images, device-local; uploads through a staging ring recorded before
  the frame's first pass (updates outside frames are kept on the CPU until the next frame / readback records them);
  new textures are zero-filled so they can be bound at once; sRGB images are `MUTABLE_FORMAT` with a UNORM view for
  sampling (never decoded) and an sRGB view for rendering (hardware encode).
* **Backdrop capture**: `vkCmdCopyImage` (sRGB -> UNORM raw), `vkCmdResolveImage` for multisampled sources (through
  a scratch image of the source's format when the destination is its raw format - resolves need identical formats);
  the **direct read** works because sampleable targets rest in `SHADER_READ_ONLY_OPTIMAL` after their pass;
  **pyramid** levels are ordinary render targets with `DONT_CARE` loads.
* **Glow layers and composite**: RGBA16F when the format has blending + linear filtering (`floatRenderTargets`),
  region clears with the Clear program.
* **Blend modes, scissor, sRGB, MSAA**: the four blend modes in the pipeline, dynamic viewport / scissor,
  `clipSpaceYDown = true` (the renderer flips its projection; no negative viewport), 1 / 2 / 4 / 8 / 16 samples
  where the device has them; multisampled targets are not reported `Sampled` (the renderer then copies / resolves).
* **Readback**: `ReadPixels` copies to a host-visible buffer through a one-time command buffer and waits; MSAA
  resolved, float / RGB10A2 / BGRA converted, sRGB returned as stored. Refused inside a frame.
* **GPU timing**: a timestamp query pool per frame slot (`BOTTOM_OF_PIPE`), frame total + one scope per profile
  run, read without stalling when the slot comes back (or polled in `ReadProfile` for self-submitted frames);
  `RenderStats::gpu` fills in.
* **Layouts**: one tracked layout per image; sampleable textures rest in `SHADER_READ_ONLY_OPTIMAL` and leave it only
  for their own pass (`COLOR_ATTACHMENT_OPTIMAL`), copies and uploads, always outside render passes; the barrier
  of a transition takes its source scope from the layout it leaves. Wrapped host images enter each frame in their
  entry layout (sampleable ones are moved to `SHADER_READ_ONLY` at `BeginFrame`: glass may read what the host drew
  before Esia's first pass) and leave in their exit layout at `EndFrame`.
* **Pipelines** cached by `PipelineDesc` (shared between handles, reference-counted), a `VkPipelineCache` (the
  host's or the backend's). One descriptor set layout for every program (immutable samplers at 11 / 12), sets from
  per-slot pools, rewritten only when a texture / buffer binding changes; unbound slots hold a 1x1 placeholder so
  every set is complete.
* **Frames in flight**: a slot per frame (staging + uniform rings, descriptor pools, queries, and when the backend
  submits itself a command pool and fence); destroyed resources are released when the last frame that could use
  them completed.
* **Host integration**: the frame is recorded into `FrameDesc::nativeContext` (the host's `VkCommandBuffer`) or,
  when null, into the backend's own command buffers submitted at `EndFrame`. `NativeRenderState()` returns the
  command buffer inside the current pass; everything is bound again afterwards.
* Not supported (reported in `Caps`, the renderer falls back): runtime effects, FX feature variants.

## 2. Conformance scenes

Against the OpenGL goldens of `origin/esia-opengl` at `77e936b` ("Conformance: image goldens from the OpenGL
backend (Mesa llvmpipe)"), extracted to a scratch directory (`git archive origin/esia-opengl
tests/conformance/golden | tar -x -C <dir>`; not committed here), `--strict`:

| Scene | Dynamic rendering | Render passes | Max delta | Pixels over tolerance |
| --- | --- | --- | --- | --- |
| shapes | PASS | PASS | 1 | 0.000 % |
| gradients | PASS | PASS | 1 | 0.000 % |
| shadows | PASS | PASS | 1 | 0.000 % |
| glass | PASS | PASS | 3 | 0.000 % |
| glow_layer | PASS | PASS | 1 | 0.000 % |
| text | PASS | PASS | 0 | 0.000 % |
| text_lcd | PASS | PASS | 1 | 0.000 % |
| edge_fade | PASS | PASS | 0 | 0.000 % |
| clipping | PASS | PASS | 0 | 0.000 % |
| windows | PASS | PASS | 3 | 0.000 % |
| hidpi | PASS | PASS | 4 | 0.000 % |
| light_streak | PASS | PASS | 3 | 0.000 % |
| srgb_target | PASS | PASS | 4 | 0.000 % |
| msaa_target | PASS | PASS | 3 | 0.000 % |

No scene SKIPs on lavapipe (it has dual-source blending, RGBA16F blending, 4x MSAA, sRGB). The two paths give
byte-identical images (unit test `VulkanScenes.BothRenderPathsAgree`, and the two runs above print identical
lines). Before the goldens existed, all 14 images of an `--out` run were inspected by eye (orientation, glass blur
and refraction, bloom, text, scissors, sRGB and MSAA variants); `msaa_target` was within 1 / 255 of `glass`.

In the repository's own CTest (`esia_conformance_vulkan`, `esia_conformance_vulkan_renderpass`) the scenes report
NO GOLDEN until the OpenGL goldens are merged into `esia-core`, which is not a failure without `--strict`.

## 3. Verified here

Machine: Ubuntu 24.04 container **without a GPU**. clang 18.1.3, CMake 3.28.3, Mesa 25.2.8 **lavapipe** ("llvmpipe
(LLVM 20.1.2, 256 bits)", device API 1.4.318, used as 1.3), Vulkan loader and headers 1.3.275, **Khronos validation
layer 1.3.275** (`vulkan-validationlayers 1.3.275.0-1`) with synchronization validation enabled
(`VK_EXT_validation_features`), mingw-w64 (GCC 13 posix) + Wine 9.0 for the Windows cross build.
Installed with `apt-get install mesa-vulkan-drivers libvulkan-dev vulkan-validationlayers vulkan-tools` on top of
the list in `docs/REWRITE_STATUS.md` section 5.

Run on a fresh `git clone --branch esia-vulkan` at `aad15b3` (the last code commit; only this file follows):

| Command | Result |
| --- | --- |
| `cmake --preset linux-clang -DESIA_WERROR=ON -DESIA_BACKEND_VULKAN=ON && cmake --build --preset linux-clang` | builds, 0 warnings |
| `ctest --preset linux-clang` | 11 / 11 pass: the 8 core tests, `esia_rhi_vulkan_tests` (14 tests), `esia_conformance_vulkan`, `esia_conformance_vulkan_renderpass` (14 scenes each, NO GOLDEN in-tree) |
| `esia_conformance --backend vulkan --golden <gl goldens> --strict`, and again with `ESIA_VULKAN_RENDER_PASS=1` | 14 / 14 PASS each (table above), 0 validation messages in the output |
| `cmake --preset linux-clang-release -DESIA_WERROR=ON -DESIA_BACKEND_VULKAN=ON`, build, `ctest --preset linux-clang-release` | 0 warnings, 11 / 11 pass |
| `VK_LOADER_DEBUG=layer esia_conformance --backend vulkan ...` | the loader inserts `VK_LAYER_KHRONOS_validation` (the layer really runs) |
| ASan + UBSan build (`-fsanitize=address,undefined`), `esia_rhi_vulkan_tests` and the Vulkan conformance run | nothing in the backend; one report in the core's `tests/conformance/scenes.cpp` (section 7) |
| `cmake --preset windows-mingw-cross -DESIA_WERROR=ON -DESIA_BACKEND_VULKAN=ON -DESIA_VULKAN_INCLUDE_DIR=<copy of /usr/include/{vulkan,vk_video}>`, build | builds, 0 warnings |
| `wine esia_rhi_vulkan_tests.exe`, `wine esia_conformance.exe --backend vulkan` | the format tests pass; `vulkan-1.dll` is found through `LoadLibraryA` but Wine's loader reports an instance version below 1.1, so the device tests and scenes SKIP cleanly (no crash) |

What the unit tests (`esia_rhi_vulkan_tests`, all on lavapipe with validation) establish:

* `Vulkan.EveryProgramBuilds`: the SPIR-V of all 8 programs becomes a pipeline for RGBA8 / RGBA8_SRGB / BGRA8 /
  BGRA8_SRGB / RGB10A2 / RGBA16F targets at 1 and 4 samples, with dynamic rendering and with render passes; user
  effects are refused.
* `ClearAndReadbackPerFormat` (7 formats, sRGB stored encoded), `UploadUpdateCopy` (create with data, row pitch,
  updates outside and inside frames, region copies, zero-filled new textures, deferred releases),
  `SrgbRawCopy` (bits identical), `MultisampledResolve` (RGBA8 and sRGB -> UNORM through the scratch image),
  `RefusesWhatItCannotDo`.
* `VulkanScenes.BothRenderPathsAgree`: every scene, identical pixels on both paths, 0 validation messages;
  `FramesInFlight`: 5 frames on one device (2 in flight) give the single frame's image, timestamps come back
  (lavapipe CPU times, e.g. glass 21-28 ms total); `OlderApiVersions`: capped at Vulkan 1.2
  (`VK_KHR_dynamic_rendering`) and 1.1 (plus its dependencies) the pixels equal 1.3's.
* `VulkanHost.HostDeviceImageAndCommandBuffers`: an application's own Vulkan **1.1** instance / device without
  dynamic rendering, its sRGB mutable-format image and its command buffers / fences (2 frames in flight, 4
  frames); `CreateDevice`, `WrapImage` every frame (same handle), a host callback that receives the host's command
  buffer, the host copying the image out of the exit layout: equal to the headless rendering, 0 validation
  messages. `GlassOverHostContent`: the host clears its image, Esia draws only glass over it (the capture reads the
  wrapped image before any Esia pass) - this found a real bug, fixed in `783938a`.

The validation layer found two real problems during the session, both fixed and covered: two uploads to one
resource without a barrier between them (sync validation, WRITE_AFTER_WRITE), and a wrapped image sampled in its
host entry layout (`783938a`).

### Windows, NVIDIA (the local Windows session, with `esia-core` round 2)

Windows 11, NVIDIA GeForce RTX 4080 SUPER (driver 610.88), LunarG Vulkan SDK 1.4.357 (its Khronos validation layer),
clang-cl 22.1.8, `windows-clang-cl` preset with `-DESIA_WERROR=ON -DESIA_BACKEND_VULKAN=ON`: 0 warnings;
`ctest` 11 / 11; `esia_conformance --strict` 18 / 18 with dynamic rendering, with `--frames 3`, and with render
passes (`ESIA_VULKAN_RENDER_PASS=1`) and `--frames 3`: max delta 20 (shadows), mean <= 0.06; 0 validation messages.

What the real machine showed, fixed on this branch:

* The loader reports through the debug messenger too (GENERAL messages): the layer manifests of other software
  (here RivaTuner's missing JSON, OBS / Epic overlays registered twice) counted as validation messages, failed every
  test and refused `ReadPixels`. They are now printed as `esia vulkan [loader]` and not counted.
* `Vulkan.MultisampledResolve` expected 128 for a 0.5 UNORM clear; NVIDIA gives 127 (half-way, both allowed).
* `VulkanHost.HostDeviceImageAndCommandBuffers` compared the application's device (Vulkan 1.1, render passes) with
  the headless one bit for bit; on NVIDIA they differ by up to 4 levels on 3 % of the channels, scattered over sRGB
  blends and frost (lavapipe: identical). It uses the scene's conformance tolerance now.

`FrameDesc::hostFrame` (core round 2) is adopted: in host mode the device frames of one host frame share a slot,
and slots rotate per host frame. `VulkanHost.TwoTargetsPerHostFrame` renders two targets per host frame for five
frames with two in flight; its first run caught the slot rotation still counting device frames (a descriptor pool
reset while in use, reported by the validation layer).

## 4. Not verified

* **Other GPUs.** lavapipe (Linux) and NVIDIA (Windows) only. Real drivers differ where these are lenient:
  `LOAD_OP_DONT_CARE` really discards on tilers (the renderer's contract says nothing outside the drawn region is
  read), memory types. AMD, Intel, Apple (MoltenVK) and mobile GPUs were not tried.
* **Windows builds** other than `windows-clang-cl`: `windows-cross` (xwin) and MSVC were not tried.
* **macOS / MoltenVK, Android**: not built. The headless creator enables `VK_KHR_portability_enumeration` /
  `VK_KHR_portability_subset` when present, untested.
* **Validation layer newer than 1.3.275** (the SDK's current layers check more; lavapipe's device is 1.4 but the
  backend caps it at 1.3).
* **Performance**: not measured (lavapipe's timestamps are CPU time); no GPU comparison with the D3D11 backend.
* **`LoadOp::Clear` on sRGB targets**: Vulkan treats the clear color as linear and encodes it; the renderer never
  clears a target with a load op (glow layers use `DontCare` + the Clear program), so this is untested against GL.

## 5. Known issues

1. **One `VkDeviceMemory` per image and buffer.** Fine for what Esia creates (a target's pyramid, layer, copy, a few
   images and glyph pages), but a host with many images would hit `maxMemoryAllocationCount` (4096 on some
   drivers) and waste allocation granularity; a sub-allocator (or VMA) is the fix.
2. **Uploads always go through staging**, even on unified-memory GPUs where device-local memory is host-visible.
3. **Wrapping contract.** A wrapped image is prepared for sampling at `BeginFrame` only when `WrapImage` was called
   for that frame (before `Renderer::Render`); a persistent host target wrapped once and never re-wrapped would be
   sampled in its entry layout if the frame's first operation is a capture of it. Documented in `vulkan.hpp`.
4. **Host mode timing and readback.** With `nativeContext`, timestamps are read only when the frame slot comes
   back (`framesInFlight` frames later), and `ReadPixels` submits to `Desc::queue` and waits: the host must have
   submitted its frame and must not use the queue from another thread meanwhile.
5. **Descriptor sets** are allocated per binding change from per-slot pools (256 sets per pool, more pools on
   demand); `VK_KHR_push_descriptor` would avoid the pools where available.
6. **No device-loss handling** (`VK_ERROR_DEVICE_LOST`): the core has no protocol yet (`docs/REWRITE_STATUS.md`,
   known issue 6).
7. **Zero-filled texture creates** (core known issue 5) cost a CPU copy of the zeros in the registry; the backend
   uploads what it is given.

## 6. Building and testing on the real platforms

The option is `ESIA_BACKEND_VULKAN` (OFF by default); building needs only the Vulkan headers, running needs a
Vulkan 1.1 driver, the validation layer is used when installed.

* **Linux (GPU or lavapipe)**: `apt-get install libvulkan-dev mesa-vulkan-drivers vulkan-validationlayers` (plus a
  GPU driver), then
  `cmake --preset linux-clang -DESIA_BACKEND_VULKAN=ON && cmake --build --preset linux-clang && ctest --preset linux-clang`.
  Pick lavapipe explicitly with `VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.json` (the headless creator
  prefers discrete > integrated > virtual > CPU devices).
* **Windows**: install the LunarG Vulkan SDK (sets `VULKAN_SDK`: headers and validation layers; the driver ships
  `vulkan-1.dll`), then from an "x64 Native Tools" prompt:
  `cmake --preset windows-clang-cl -DESIA_BACKEND_VULKAN=ON && cmake --build --preset windows-clang-cl && ctest --preset windows-clang-cl`
  (or the `vs2022` preset with `-DESIA_BUILD_CORE=ON -DESIA_BACKEND_VULKAN=ON`).
* **macOS**: the Vulkan SDK (MoltenVK) or `brew install vulkan-headers molten-vk vulkan-loader
  vulkan-validationlayers`, then the `macos-clang` preset with `-DESIA_BACKEND_VULKAN=ON` (untested).
* **Against the goldens**: `esia_conformance --backend vulkan --golden tests/conformance/golden --out out --strict`
  once the OpenGL goldens are in the tree; `ESIA_VULKAN_RENDER_PASS=1` forces render passes, `ESIA_VULKAN_VALIDATION=0`
  turns the layer off (e.g. for timing). Any validation message fails the scene's readback and is printed as
  `esia vulkan [validation ...]` (the backend's own CTests fail on that output too).

## 7. Core change requests

1. **Dangling reference in `tests/conformance/scenes.cpp` (`MakeScenes`).** `Scene& glass = add(...)` (and `glow`,
   `win` ...) refer into `std::vector<Scene> s`, and later `add` calls reallocate it; line 477
   (`srgb.tolerance = glass.tolerance;`) and 481 then read freed memory - ASan reports a heap-use-after-free. It works
   today by luck (the freed block still holds the old values). Smallest fix: `s.reserve(32);` at the top of
   `MakeScenes` (or copy `glass.tolerance` into a local before the later `add` calls).
2. *(optional)* **Test properties for backend CTests.** The suite's `esia_conformance_<name>` tests are created in
   `tests/CMakeLists.txt`, so a backend cannot give them a `FAIL_REGULAR_EXPRESSION` for messages printed after the
   readback (e.g. at device destruction). The Vulkan backend works around it (a validation message fails
   `ReadPixels`, and its own tests check whole device lifetimes); a global property such as
   `ESIA_BACKEND_TEST_FAIL_REGEX_<name>` read when the test is added would do it generally.

## 8. Checklist (backends/README.md section 8)

1. `ESIA_BACKEND_VULKAN=ON` builds warning-free with clang, `ESIA_WERROR=ON` (Debug and Release; also mingw-w64). **Yes.**
2. `EsiaRegisterBackend_vulkan()` registers a headless creator; `esia_conformance --list` shows `backends: null vulkan`. **Yes.**
3. Every conformance scene passes on lavapipe (against the OpenGL goldens, `--strict`), none SKIPped. **Yes.**
4. The null-device rules hold: the renderer's command streams are the ones the null goldens record; the backend
   additionally refuses what the rules forbid (copies inside passes, reads inside frames, draws without a
   pipeline), and the validation layer reported nothing over every scene. **Yes.**
5. The host integration header documents device creation, target wrapping, `FrameDesc::nativeContext`, frames in
   flight, host callbacks. **Yes**, and a test uses it the way an application would.
6. GPU times appear in `RenderStats::gpu` (checked by `VulkanScenes.FramesInFlight` and the host test). **Yes.**
7. Nothing outside `src/esia/rhi/vulkan/` changed (the OpenGL goldens were used from a scratch directory, not
   committed). **Yes.**
