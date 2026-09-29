# Esia Metal backend - status of `esia-metal`

The Metal RHI backend (macOS 11+ / iOS 14+, Apple silicon and Intel), as the backend guide
([docs/backends/README.md](../../../../docs/backends/README.md)) prescribes, written in a Linux cloud session on
2026-09-29. **UNVERIFIED: needs macOS.** Nothing in this directory was ever compiled against Apple's SDK or run on a
Mac. What *was* built and run is listed exactly in [section 3](#3-verified-here); the rest is in
[section 4](#4-not-verified).

**In one paragraph.** The backend is split so that almost all of it is plain C++ that builds and runs everywhere: the
translation tables, the planning (pipeline keys, blit layouts, copy plans, readback conversion), the frame bookkeeping
(completion tracking, buffer versions, deferred releases, staging) and the whole `rhi::Device` (`MetalDevice`) sit on a
thin `Gpu` interface with one virtual per Metal call. `metal_device.mm` implements that interface with Metal
(~600 lines of one-call-per-method Objective-C++). On Linux the same `MetalDevice` is driven by the real renderer
through all 14 conformance scenes on a fake `Gpu` that enforces the Metal rules the backend relies on (bindings
parsed from the generated MSL, encoder nesting, attachment / pipeline formats, blit formats and layouts, resource
lifetimes against command buffers in flight) and executes uploads, copies, resolves and readback on CPU copies: no
violation in any scene. The generated MSL was type-checked with clang against a mock of the Metal standard library,
and `metal_device.mm` was parsed as Objective-C++ with ARC against mock Foundation / Metal declarations - both only
prove what they can (section 3). The first macOS build will very likely need small fixes in `metal_device.mm`; the
test plan in [section 6](#6-building-and-testing-on-macos) says what to run and what to look for.

* [1. Implemented](#1-implemented)
* [2. Conformance scenes](#2-conformance-scenes)
* [3. Verified here](#3-verified-here)
* [4. Not verified](#4-not-verified)
* [5. Known issues and limitations](#5-known-issues-and-limitations)
* [6. Building and testing on macOS](#6-building-and-testing-on-macos)
* [7. Core change requests](#7-core-change-requests)
* [8. Guide checklist](#8-guide-checklist)

## 1. Implemented

### Files

| File | What | Built on |
| --- | --- | --- |
| `CMakeLists.txt` | `esia_rhi_metal_core` (portable), `esia_rhi_metal` (+ `.mm` on Apple, the SKIP registration elsewhere), `esia_register_backend(metal ...)`, tests; Objective-C++ with ARC, links Metal, Foundation and libobjc; refuses a deployment target below macOS 11 / iOS 14 | all |
| `include/esia/rhi/metal.hpp` | host integration: `CreateDevice(Desc)` / `CreateDevice(id<MTLDevice>, id<MTLCommandQueue>)`, `WrapTexture(device, id<MTLTexture>)`, what `nativeContext` and `NativeRenderState()` are, what a host texture must allow for each capture path | all (ObjC overloads under `__OBJC__`) |
| `metal_tables.hpp/.cpp` | RHI format <-> `MTLPixelFormat`, usage bits (created and host textures), blend states, UI vertex descriptor, binding indices, load actions, primitive types - as raw Apple enum values | all |
| `metal_planning.hpp/.cpp` | pipeline keys (program, blend, format, samples: the topology is a draw argument on Metal), blit layouts (256-byte aligned offsets / pitches), RGBA8 conversion for `ReadPixels` (BGRA swizzle, RGB10A2, half / float, R8), copy plans (sRGB-twin views on whichever side has `PixelFormatView`, MSAA resolve first) | all |
| `metal_frames.hpp/.cpp` | `FrameTracker` (completed prefix of device frames, thread-safe), `VersionRing` (buffer versions), `DeferredReleases`, `StagingArena` | all |
| `metal_profiler.hpp/.cpp` | timestamps at encoder boundaries, tick -> ns calibration, per-category split, resolution a few frames later | all |
| `metal_gpu.hpp` | the `Gpu` interface: one virtual per Metal call MetalDevice makes | all |
| `metal_device_core.hpp/.cpp` | `MetalDevice : rhi::Device` and `CreateHeadlessOn` (the conformance suite's headless device on any `Gpu`) | all |
| `metal_device.mm` | `MetalGpu : Gpu` on Metal, `CreateDevice`, `WrapTexture`, the headless creator, `EsiaRegisterBackend_metal`; `static_assert`s of every raw value the tables use | Apple only |
| `metal_unavailable.cpp` | `EsiaRegisterBackend_metal` on other hosts: the headless creator returns no device with the reason (the suite reports SKIP) | non-Apple |
| `tests/` | unit tests, the MSL interface parser, the fake Metal `Gpu`, the device tests (section 3) | all |
| `tests/msl_check/` | `check_msl.cmake` (mode `xcrun` on macOS, `mock` elsewhere) and the mock `metal_stdlib` | all |
| `tests/objc_check/` | mock `Foundation/Foundation.h`, `Metal/Metal.h` for parsing `metal_device.mm` on non-Apple hosts | non-Apple |

### How RHI calls map to Metal

| RHI | Metal |
| --- | --- |
| `BeginFrame(nativeContext)` | the host's `id<MTLCommandBuffer>` (still recording) or `[queue commandBuffer]`; `addCompletedHandler` feeds the frame tracker; staged uploads made outside a frame are blitted first. Never waits for the GPU. |
| `EndFrame` | ends the open encoder; commits the backend's own command buffer (a host's is committed by the host) |
| `BeginPass` / `EndPass` | `renderCommandEncoderWithDescriptor` (load action from `LoadOp`, store `Store`), viewport = target; every binding forgotten (the RHI rule, and a stale binding could name the new target) |
| `SetPipeline` | `MTLRenderPipelineState` cached per key: MSL 2.0 library per blob (`newLibraryWithSource`, entry `esia_main`, fast math off), `rasterSampleCount`, blend state, vertex descriptor (float2 / float2 / uchar4 normalized at buffer 30, stride 20) for the UI programs. User effects: refused (`runtimeEffects = false`). |
| `SetConstants` | cached, `setVertexBytes` + `setFragmentBytes` at 0 / 1 / 2 at the next draw |
| `SetTexture` t0..t6 | `setFragmentTexture` at 3..9 at the next draw; sRGB textures through their UNORM view; t7 is not a texture on Metal |
| `SetFxBuffer` | `setVertexBuffer` + `setFragmentBuffer` at 10 (the current version) |
| samplers | `setFragmentSamplerState` linear / point clamp at 11 / 12, per encoder |
| `SetVertexBuffer` / `SetIndexBuffer` | vertex buffer at 30 / `drawIndexedPrimitives` UInt32 with `first * 4` offset |
| `SetScissor` | `setScissorRect` at the next draw (top-left origin like the RHI); an empty rect is never sent (the RHI allows it when no draw follows) |
| `Draw` / `DrawInstanced` / `DrawIndexed` | `drawPrimitives` (triangle / strip from the pipeline's topology, instance count) / `drawIndexedPrimitives` |
| `CopyTexture` | blit `copyFromTexture`; between sRGB twins through a view; a multisampled source first resolved by a render pass (`Load` + `StoreAndMultisampleResolve`) into a scratch texture |
| `CreateTexture` / `UpdateTexture` | `MTLStorageModePrivate` textures; uploads through shared staging buffers and a blit in the frame's command buffer (so they never race a frame in flight); outside a frame kept as CPU copies until the next command buffer |
| `CreateBuffer` / `UpdateBuffer` | shared `MTLBuffer`s, one version per frame in flight (a version is rewritten only when the frames that read it completed; the untouched tail is carried over) |
| `DestroyTexture` / `DestroyBuffer` | released when every frame begun so far completed (safe with unretained host command buffers) |
| `ReadPixels` | waits for the frames in flight (fails instead of hanging if a host command buffer was not committed), blits (after a resolve if multisampled) into a shared buffer on the backend's queue, waits, converts to RGBA8 |
| `NativeRenderState` | the current `id<MTLRenderCommandEncoder>`; everything is bound again afterwards |
| `BeginProfile` ... `ReadProfile` | counter sample buffers at stage boundaries (Apple GPUs); see `metal_profiler.hpp` |

### Caps

`fxStorage = Buffer`, `shaderFormat = Msl`, `framebufferOriginBottomLeft = false`, `clipSpaceYDown = false`,
`halfPixelOffset = false`, `dualSourceBlend = true`, `floatRenderTargets = true`, `sampleRenderTarget = true`,
`readback = true`, `runtimeEffects = false`, `fxFeatureVariants = false`, `maxTextureSize` 16384 (Apple3+ / Mac2, else
8192), `timestampQueries` = the GPU supports `MTLCounterSamplingPointAtStageBoundary` with the timestamp counter set
(Apple silicon; Intel / AMD Macs only sample at draw / blit boundaries, not used) and `Desc::timestamps`.

## 2. Conformance scenes

**On a Mac: none has run.** Their expected status is unknown; until the OpenGL session's image goldens are merged
they report NO GOLDEN (a failure only with `--strict`), and the images must be inspected (section 6).

**On Linux** (this session): `esia_conformance --backend metal` reports SKIP for all 14 scenes with the reason "Metal
runs on macOS / iOS only (this build has the backend's portable parts, tested by esia_rhi_metal_tests)", and
`esia_conformance_metal` passes as a CTest (skips are not failures).

**Through the fake Metal GPU** (`MetalDevice.ConformanceScenes`, three frames per scene, with and without timestamps):
every scene renders with no RHI contract violation and no Metal rule broken, reads back, and releases everything. What
one frame exercises (from a throwaway counting run, first frame):

| scene | render passes | blit passes | MSAA resolves | texture copies | uploads | draws | direct / copy captures | pipelines |
|---|---|---|---|---|---|---|---|---|
| shapes | 1 | 1 | 0 | 0 | 1 | 1 | 0 / 0 | 1 |
| gradients | 1 | 1 | 0 | 0 | 1 | 2 | 0 / 0 | 2 |
| shadows | 1 | 1 | 0 | 0 | 1 | 1 | 0 / 0 | 1 |
| glass | 11 | 2 | 0 | 1 | 1 | 15 | 1 / 1 | 3 |
| glow_layer | 19 | 1 | 0 | 0 | 2 | 23 | 0 / 0 | 7 |
| text | 1 | 1 | 0 | 0 | 2 | 2 | 0 / 0 | 2 |
| text_lcd | 6 | 1 | 0 | 0 | 2 | 8 | 0 / 0 | 6 |
| edge_fade | 1 | 1 | 0 | 0 | 2 | 43 | 0 / 0 | 3 |
| clipping | 1 | 1 | 0 | 0 | 2 | 8 | 0 / 0 | 2 |
| windows | 11 | 1 | 0 | 0 | 2 | 33 | 2 / 0 | 4 |
| hidpi | 7 | 1 | 0 | 0 | 2 | 11 | 1 / 0 | 4 |
| light_streak | 6 | 2 | 0 | 1 | 1 | 6 | 0 / 1 | 2 |
| srgb_target | 11 | 2 | 0 | 1 | 1 | 15 | 1 / 1 | 3 |
| msaa_target | 13 | 3 | 2 | 2 | 1 | 15 | 0 / 2 | 3 |

(Uploads are texture uploads - the white texture, glyph and image pages; vertices, indices and FX instances go
through versioned buffers. The fake validates draws but does not rasterize: this says nothing about pixels.)

## 3. Verified here

Machine: Ubuntu 24.04 container without a GPU. Tools: clang / clang++ 18.1.3, ld.lld 18.1.3, CMake 3.28.3, Ninja
1.11.1, glslang 15.1.0, SPIRV-Cross 2021.01.15, mingw-w64 GCC 13-posix headers / libraries (used by clang), Wine 9.0,
Python 3.11. Branch `esia-metal`, based on `esia-core` at `3810b97` (`git merge origin/esia-core`: fast-forward; the core
did not move afterwards).

| Command | Result |
| --- | --- |
| `cmake --preset linux-clang -DESIA_WERROR=ON -DESIA_BACKEND_METAL=ON && cmake --build --preset linux-clang` | builds, 0 warnings |
| `ctest --preset linux-clang` | 12 / 12 pass: the core's 8 (incl. `esia_conformance_null`) + `esia_conformance_metal` (14 SKIP) + `esia_rhi_metal_tests` (25 tests) + `esia_rhi_metal_objc_mock_check` + `esia_rhi_metal_msl_mock_check` |
| the same with `linux-clang-release` (`-O3`) | 0 warnings, 12 / 12 pass |
| `linux-clang` with `ESIA_BACKEND_METAL=OFF` (the default) | 0 warnings, 8 / 8 pass, "Esia backends: null": nothing of this branch is built |
| `cmake --preset windows-mingw-cross -DESIA_WERROR=ON -DESIA_BACKEND_METAL=ON && cmake --build --preset windows-mingw-cross` | builds every target, 0 warnings (after `850fb03`, see section 5) |
| `wine build/windows-mingw-cross/bin/esia_rhi_metal_tests.exe` | 25 / 25 pass |
| `wine .../esia_conformance.exe --backend metal ...` / `--backend null --strict` | 14 SKIP / 14 pass |
| a fresh `git clone --branch esia-metal` at `1552d7f` (only what is committed): `linux-clang`, `ESIA_WERROR=ON`, `ESIA_BACKEND_METAL=ON`, build + `ctest` | 0 warnings, 12 / 12 pass |
| `python3 tools/shaders/build_shaders.py --check` | "generated shaders are up to date" (the MSL checked is the MSL of the HLSL) |

What `esia_rhi_metal_tests` covers (25 tests):

* **Tables / planning / frames / profiler**: every RHI format <-> pixel format both ways, bytes per pixel, sRGB twins;
  created and host texture usage (framebufferOnly, sRGB without `PixelFormatView`, MSAA); the four blend modes;
  vertex attribute offsets against `offsetof(esia::Vertex, ...)`; pipeline keys unique over every combination and the
  refusals (effects, dual source, layout, format); blit layouts; readback conversion of every format; copy plans;
  completed-prefix tracking (also from 4 threads); version rings; deferred releases; the staging arena; clock
  calibration, sample assignment, per-category split, invalid counters, overflow.
* **MSL interface** (`test_metal_msl.cpp`): the generated MSL of all 8 programs x 2 stages parsed - every
  `[[buffer]]`, `[[texture]]`, `[[sampler]]` index is the one the backend binds (constants 0 / 1 / 2, `gTex` 3,
  `gBackdrop0..5` 4..9, `gFxData` 10, `gLinear` / `gPoint` 11 / 12), nothing at buffer 30, vertex shaders sample no
  texture (so binding textures to the fragment stage only is enough), UI vertex shaders take `stage_in` attributes
  0 / 1 / 2 as float2 / float2 / float4 and the others use `vertex_id` (+ `instance_id` for Fx), every fragment input
  is written by the vertex shader with the same location and type, `TextLcd` alone has `color(0) index(1)`. (Integer
  varyings have no `[[flat]]`: SPIRV-Cross leaves it out because integers are always flat in MSL.)
* **Device on the fake GPU** (`test_metal_device.cpp`): the 14 scenes (above); `NullGoldensWithMetalCaps` - with the
  Metal caps the renderer's command stream is identical to `tests/conformance/golden/null/*.log` for all 14 scenes and
  the null device finds no violation; uploads and readback with row pitches, sub-rectangles, R8 / BGRA; in-frame
  staging and the refusal after the first pass; sRGB copies through a source view and through a destination view
  (host texture without `PixelFormatView`), the framebufferOnly refusal, wrapper caching; MSAA resolve + blit and MSAA
  readback, the headless SKIP for an unsupported sample count; four device frames recorded into uncommitted host
  command buffers with surfaces released and another frame begun in between (versions, deferred releases),
  `ReadPixels` refusing to wait for uncommitted work; bindings emptied at `NativeRenderState`, pipeline state sharing,
  empty scissors.
* **The fake catches what it claims** - checked by breaking the backend on purpose (then restoring it): not binding
  the samplers (hundreds of "gLinear not bound"), carrying bindings over into the next pass ("the pass target is
  bound as a texture" in glass / glow / windows / msaa), releasing destroyed objects immediately (the in-flight test
  fails: "a command buffer that uses it has not completed").

**MSL type check** (`esia_rhi_metal_msl_mock_check`, `tests/msl_check/check_msl.cmake` mode `mock`): each of the 16
`.metal` files is compiled with `clang++ -x c++ -std=c++17 -fsyntax-only` against `tests/msl_check/metal_stdlib`, a
mock of the Metal standard library (vectors as clang `ext_vector_type` with swizzles, constructor calls rewritten to
`make_floatN(...)` that `static_assert` the component count, `select` / `mix` / `clamp` / `min` / `max` / `pow`
checking operand widths, `texture2d<float>::sample` with and without `level`, SPIRV-Cross's `spvUnsafeArray` replaced,
float literals given MSL's float type). All 16 pass. Negative control: four deliberately broken copies (a float2 with 3
components, an unknown member, a float2 where a float3 is expected, wrong `sample` arguments) all fail. **What this
does not prove**: anything Metal-specific - address spaces are erased, attributes ignored, the functions do no math,
and Apple's compiler has rules C++ does not. The real check is `xcrun metal` (section 6).

**Objective-C++ parse** (`esia_rhi_metal_objc_mock_check`): `metal_device.mm` passes `clang++ -x objective-c++
-std=c++20 -fobjc-arc -fsyntax-only -Wall -Wextra -Wpedantic -Wshadow -Werror` against `tests/objc_check`'s mock
Foundation / Metal declarations (written from Apple's documentation). That checks the file's syntax, C++ / ObjC types,
ARC bridging, blocks capturing C++ objects, and the warnings the real build enables (its first run failed only on a
gap in the mock, `nil`; a C99 compound literal and cross-enum comparisons that `-Wpedantic` / `-Wenum-compare` reject
had been fixed by review before). **What it does not prove**: that Apple's selectors,
properties, types and availability are what the mock declares; its enum values are the backend's own beliefs, so the
`static_assert`s pass trivially there. Apple's SDK headers were deliberately not used on this Linux machine (their
license ties them to Apple-branded computers).

## 4. Not verified

* **`metal_device.mm` never compiled against the SDK and never ran.** Expect a first round of compile fixes (API
  spellings, availability annotations, deprecations under `-Werror` with a new SDK).
* **The generated MSL never went through Apple's compiler** (`xcrun metal`) - it is SPIRV-Cross 2021.01.15 output
  for MSL 2.0; only the mock type check above ran.
* **No pixel was ever produced by this backend.** Colors, blending (in particular dual-source text), sRGB handling,
  scissors, the resolve path, captures and pyramids are only checked structurally.
* **The fake's rules are Metal's as documented**, not compared with the Metal API validation layer.
* **Timestamps**: the tick -> ns calibration (`sampleTimestamps:gpuTimestamp:`, CPU side assumed to be nanoseconds),
  the error sentinel handling, and whether Apple GPUs fill all four stage-boundary samples of every encoder.
* **iOS**: nothing iOS-specific was tried (the code has no macOS-only path except the `@available` checks; storage is
  private / shared only, no managed resources).
* **Intel Macs**: `timestampQueries` is expected to be false there (no stage-boundary sampling); shared buffers on a
  discrete GPU are read over PCIe - fine for UI sizes, unmeasured.
* **Performance**: nothing measured. MSL is compiled at the first `CreatePipeline` of each program (a first-frame
  hitch; every headless conformance device compiles again).
* `macos-clang` + `enable_language(OBJCXX)` with Homebrew LLVM: written to work (the OBJCXX compiler is set to the C++
  compiler of the toolchain), not configured anywhere.

## 5. Known issues and limitations

1. **GPU times per category are approximate** on Apple GPUs: timestamps exist only at encoder boundaries, so a render
   pass shared by several categories (FX, glass FX, geometry, the layer composite) is split by draw count; captures and
   layer pyramids are exact (whole encoders). `GpuProfile::totalMs` is the union of the backend's encoder intervals.
   `ESIA_METAL_NO_TIMESTAMPS=1` turns timestamps off in the headless device (for the first runs); hosts use
   `Desc::timestamps = false`.
2. **No user effects** (`runtimeEffects = false`): Metal has no runtime HLSL; the renderer draws the built-in shader
   (as on GL and Vulkan, REWRITE.md section 7).
3. **Host targets**: CAMetalLayer's default `framebufferOnly = YES` allows no copy, so glass cannot capture (the copy is
   refused with an error, glass samples white); set `framebufferOnly = NO`. An sRGB drawable is only read directly
   with `MTLTextureUsagePixelFormatView` (else the backdrop is copied through a view of the backend's copy).
   Memoryless targets cannot work at all (the renderer reloads targets between passes); not detected.
4. **Wrappers keep the host texture alive** until `DestroyTexture` (documented in `metal.hpp`): wrap per frame and
   destroy, or keep one wrapper per drawable texture.
5. **A frame never waits**, so memory is the bound: buffer versions and staging chunks grow while the GPU is behind
   (idle staging chunks beyond two are trimmed). A host that never commits its command buffers grows without limit.
6. **mingw-w64 + clang + libstdc++ 13**: `std::type_info::operator==` is defined both inline and in `libstdc++.a`, so
   `std::make_shared` and `std::regex` fail to link in the `windows-mingw-cross` preset; this backend avoids both
   (`850fb03`). It is a trap for every session that builds with that preset (section 7, request 2).
7. The core's known issues apply unchanged (REWRITE_STATUS.md section 4): sRGB targets blend in linear light, no
   device-loss protocol, zero-filled glyph page uploads (4 MB staged per new page here).

## 6. Building and testing on macOS

Requirements: a Mac with macOS 11 or later (Apple silicon for timestamps), **Xcode** (not only the command line tools:
the `metal` compiler comes with Xcode; on recent Xcode versions the Metal toolchain may be a separate component -
`xcodebuild -downloadComponent MetalToolchain` if `xcrun metal` says it is missing).

```bash
brew install llvm lld cmake ninja glslang spirv-cross spirv-tools freetype harfbuzz

# 1. the generated MSL with Apple's compiler first (independent of the C++ build)
for f in src/esia/shaders/generated/msl/*.metal; do
    xcrun -sdk macosx metal -std=macos-metal2.0 -c "$f" -o "/tmp/$(basename "$f").air" || echo "FAILED $f"
done
# optional, iOS (needs the iOS platform installed in Xcode)
for f in src/esia/shaders/generated/msl/*.metal; do
    xcrun -sdk iphoneos metal -std=ios-metal2.0 -c "$f" -o "/tmp/$(basename "$f").ios.air" || echo "FAILED $f"
done

# 2. configure, build, unit tests (the ctest run includes esia_rhi_metal_msl_compile: the loop above as a test)
cmake --preset macos-clang -DESIA_WERROR=ON -DESIA_BACKEND_METAL=ON
cmake --build --preset macos-clang
ctest --preset macos-clang --output-on-failure
# also once at the minimum: -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 (availability warnings are errors with ESIA_WERROR)

# 3. the conformance suite with Metal's API and shader validation
export MTL_DEBUG_LAYER=1            # API validation (asserts on misuse)
export MTL_SHADER_VALIDATION=1      # out-of-bounds / nil resource access in shaders
build/macos-clang/bin/esia_conformance --list          # "backends: null metal"
build/macos-clang/bin/esia_conformance --backend metal --golden tests/conformance/golden --out /tmp/esia-metal
# the PNGs are in /tmp/esia-metal/metal/<scene>.png (and <scene>.diff.png on a mismatch once goldens exist)

# 4. if something fails at runtime: timestamps off first, then one scene at a time
ESIA_METAL_NO_TIMESTAMPS=1 build/macos-clang/bin/esia_conformance --backend metal --scene shapes --out /tmp/esia-metal
```

First build - likely spots, in this order: the `static_assert`s at the top of `metal_device.mm` (a failure means a raw
value in `metal_tables.hpp` is wrong: fix the table, the Linux tests follow it); availability / deprecation warnings
(`-Wunguarded-availability-new`, `fastMathEnabled`: guarded with a pragma and `@available`); property and selector
spellings (the mock in `tests/objc_check` then needs the same fix, so Linux keeps parsing the file); linking (libobjc,
Metal, Foundation are in the CMake file).

The scenes, what they exercise on Metal and what to look for (compare with the OpenGL session's images when both
exist; `glass.png` is also `msaa_target`'s golden):

| Scene | Exercises | Look for |
| --- | --- | --- |
| `shapes` | the Fx pipeline: FX buffer at 10 in both stages, `instance_id`, triangle strips, per-draw constants | crisp SDF shapes; nothing drawn = FX buffer / draw constants not reaching the vertex shader; shapes at wrong places = first-instance constant (`gDrawInfo.x`) |
| `gradients` | indexed geometry: the vertex descriptor (float2, float2, uchar4 normalized at buffer 30), 32-bit indices | polylines and area chart as on GL; red / blue swapped = color byte order; garbage triangles = attribute offsets / index offset |
| `shadows` | FX math (blur, noise) with fast math off | smooth shadows and glows, no NaN speckles |
| `glass` | one direct capture (frost read straight from the target) and one copy capture, 5 RGBA16F pyramid levels (DontCare loads, scissored), refraction / dispersion | frosted, refracted backdrop; black or white glass = pyramid not sampled / wrong texture index; blocky or noisy borders = region clamping; banding = pyramid not RGBA16F |
| `glow_layer` | 19 passes: glow layers cleared by the Clear program in DontCare passes, bloom pyramid, LayerComposite (t0 point, t1..t6 linear) | soft bloom around the content; garbage rectangles = the region clear; a hard-edged glow = the pyramid levels |
| `text` | R8 glyph page uploaded through the staging blit (256-byte pitches), TextGray | sharp glyphs; sheared or striped glyphs = upload pitch; missing text = the R8 upload or `gTex` |
| `text_lcd` | dual-source blending (`index(1)` + `Source1*` factors) and TextLcdGray inside layers | color-fringed sub-pixel text on the target; black boxes = blend factors; missing = pipeline creation failed (see stderr) |
| `edge_fade` | 43 draws with changing draw constants through `setVertexBytes` / `setFragmentBytes` | fades differ per clip rect; one fade everywhere = constants not re-sent |
| `clipping` | scissors (top-left, no flip), textured geometry, uv sub-rects | clips in the same places as GL; vertically mirrored clips = a coordinate flip that must not be there |
| `windows` | the UI core end to end, two direct captures | as on GL |
| `hidpi` | render scale 2, hairlines on pixel rows | 1-pixel lines stay 1 pixel |
| `light_streak` | copy capture, dispersion, rounded mask | the caustic streak under the dome |
| `srgb_target` | `RGBA8Unorm_sRGB` target: hardware encode on write, the UNORM view for the direct read, the source view for the backdrop copy | like `glass` (brighter translucency is expected: linear blending); a too dark or washed-out backdrop in the glass = sRGB decoded when sampling (a view missing) |
| `msaa_target` | 4x target: pipelines with `rasterSampleCount = 4`, captures by resolve render pass + blit, readback through a resolve | like `glass` within tolerance; black glass = the resolve pass; validation errors about sample counts = pipeline keys |

Then, in a host (see the example in `metal.hpp`): render a few frames into a `CAMetalLayer` drawable with
`framebufferOnly = NO` and print `renderer.Stats().gpu` - `valid` after two or three frames, `totalMs` close to what
Xcode's GPU frame capture or Instruments' Metal System Trace shows for the backend's encoders. If it is off by a
constant factor (around 41.7 on Apple silicon would mean the CPU side of `sampleTimestamps` is in mach ticks, not
nanoseconds), fix `TimestampClock` (`metal_profiler.cpp`).

## 7. Core change requests

None blocks this backend; all are small.

1. **`tests/conformance`: a scenes library.** Backend unit tests that want the scenes (this one does, to drive its
   logic through the renderer) compile `tests/conformance/scenes.cpp` into their own executable
   (`src/esia/rhi/metal/CMakeLists.txt`). Smallest change: in `tests/CMakeLists.txt`, `add_library(esia_conformance_scenes
   STATIC conformance/scenes.cpp)` with `tests/conformance` as a public include directory and `esia::render
   esia_testkit` as public dependencies, used by `esia_conformance`; backends then link it. Reason: a change of the
   scenes' dependencies would otherwise break backend test builds silently.
2. **Backend guide, section 6 (LLVM toolchain)**: note that with the `windows-mingw-cross` preset (clang + mingw-w64
   libstdc++ 13) anything that instantiates `std::type_info::operator==` fails to link with a duplicate symbol - seen
   here with `std::make_shared` and `std::regex`; `typeid(a) == typeid(b)` and `std::function::target` instantiate it
   too. Reason: every session that
   builds with that preset can hit it; this one lost a build to it.
3. **Backend guide, Metal recipe**: "textures 3..10" should read "textures 3..9, the FX buffer at 10" (the MSL uses
   buffer 10 for `gFxData`, `tests/test_metal_msl.cpp` checks it); and "timestamps: start with `timestampQueries =
   false`" can become "per encoder on Apple GPUs; categories inside a render pass are approximate" (section 5, item 1).
4. **`rhi::GpuProfile` comment** (`include/esia/rhi/rhi.hpp`): say that `categoryMs` may be an approximation on APIs
   that time whole passes (Metal on Apple GPUs), so tools do not over-trust the split. Comment only.
5. **Optional, `rhi::Format`: `BGR10A2_UNORM`**, the 10-bit format `CAMetalLayer` offers on iOS (macOS also has
   `RGB10A2`, which the RHI has). Smallest change: the enum value plus its cases in `rhi.cpp` (`BytesPerPixel` 4,
   `FormatName`); the Metal tables would add `MTLPixelFormatBGR10A2Unorm = 94`. Only needed for wide-color iOS hosts.

## 8. Guide checklist

| # | Item | State |
| --- | --- | --- |
| 1 | `ESIA_BACKEND_METAL=ON` builds warning-free with clang (`ESIA_WERROR=ON`) | on Linux and in the mingw cross build: yes (portable part, SKIP registration, the `.mm` parsed against mocks). On macOS: **not tried** |
| 2 | `EsiaRegisterBackend_metal()` registers a headless creator; `--list` shows the backend | yes (Linux: "backends: null metal"; the creator reports why it SKIPs). The macOS creator is unrun |
| 3 | every scene passes on the machine that runs the backend, or SKIPs for a stated reason | **not run on macOS**; on Linux all 14 SKIP with the reason; through the fake GPU all 14 run without violations |
| 4 | the null-device rules hold | yes: with Metal's caps the renderer's streams equal the null goldens for all 14 scenes, no violation (`MetalDevice.NullGoldensWithMetalCaps`) |
| 5 | the host header documents device creation, target wrapping, `nativeContext` | yes, `include/esia/rhi/metal.hpp` |
| 6 | GPU times in `RenderStats::gpu` where the API has timestamps | implemented for Apple GPUs (encoder boundaries); verified only on the fake (`st.gpu.valid` after 3 frames in every scene) |
| 7 | nothing outside `src/esia/rhi/metal/` changed | yes: `git diff --name-only origin/esia-core..esia-metal` lists only files under it |

Commits on `esia-metal` after `esia-core` (`3810b97`): `5e0ca4d` portable device logic, fake Metal GPU, tests -
`52b9ad6` the Metal Gpu layer, host integration, registration - `850fb03` the mingw-w64 cross build - `1552d7f`
libobjc, UNVERIFIED marks - and the `[cloud-done]` commit with this file.
