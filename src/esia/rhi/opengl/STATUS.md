# Esia OpenGL / GLES backend - status of `esia-opengl`

The OpenGL 3.3 core + OpenGL ES 3.0 RHI backend (`src/esia/rhi/opengl`, `ESIA_BACKEND_OPENGL`, OFF by default),
end of the backend session (2026-09-29). Built on `esia-core` at `3810b97`; the guide it follows is
[docs/backends/README.md](../../../../docs/backends/README.md).

**In one paragraph.** One static target, `esia_rhi_opengl`, registered as the backends `opengl` (GLSL 330 on a
3.3+ core context) and `gles` (ESSL 300 on an ES 3.0+ context). It loads its own GL functions, links no GL or EGL
library, and runs headless through EGL loaded at runtime (Mesa's surfaceless platform on llvmpipe here). Both
backends pass all 14 conformance scenes with `--strict` against the image goldens, which this session produced on
llvmpipe, reviewed image by image, and committed on their own (`77e936b`). Every scene renders with zero GL errors
on both APIs, both with every extension llvmpipe offers and on the bare GL 3.3 / GLES 3.0 minimum. Everything builds
warning-free with clang 18 (`-DESIA_WERROR=ON`, debug and release) and with the mingw-w64 Windows cross preset.
**Nothing ran on a real GPU, on Windows, on macOS or on Android** (section 4).

* [1. What is implemented](#1-what-is-implemented)
* [2. Conformance](#2-conformance)
* [3. Verified here](#3-verified-here)
* [4. Not verified](#4-not-verified)
* [5. Known issues and limits](#5-known-issues-and-limits)
* [6. Building and testing on the real platforms](#6-building-and-testing-on-the-real-platforms)
* [7. Core change requests](#7-core-change-requests)

## 1. What is implemented

### Files

| File | What |
| --- | --- |
| `CMakeLists.txt` | `esia_rhi_opengl` (links `esia_rhi`, `esia_shaders`, `${CMAKE_DL_LIBS}`), `esia_register_backend(opengl ...)` and `(gles ...)`, the test `esia_rhi_opengl_tests` |
| `include/esia/rhi/opengl.hpp` | host integration: `Desc` (`es`, `getProcAddress`, `debug`, `restoreHostState`, `coreOnly`), `CreateDevice`, `WrapFramebuffer`, `NativeTexture`, `ErrorCount`; no GL header needed |
| `gl_api.hpp` / `.cpp` | GL types, constants and the function table (X-macro lists: what GL 3.3 core and GLES 3.0 share, plus optional entry points loaded by exact name per version / extension, because Mesa's `eglGetProcAddress` returns stubs for any name); the platform default lookup (EGL / GLX, WGL + `opengl32.dll`, the process image on macOS) |
| `gl_device.hpp` / `.cpp` | `GlDevice`, the `rhi::Device` |
| `gl_headless.hpp` / `.cpp` | EGL loaded with `dlopen` / `LoadLibrary`, surfaceless display, headless devices, the two `EsiaRegisterBackend_*` functions; `ESIA_GL_CORE_ONLY=1` makes the registered backends run on the core minimum |
| `tests/test_gl_device.cpp` | 11 device tests on the machine's driver (section 3) |

### The RHI on GL

| RHI | GL |
| --- | --- |
| textures | `GL_TEXTURE_2D`, one level (`MAX_LEVEL 0`, non-mip filters), `GL_RGBA8` / `GL_SRGB8_ALPHA8` / `GL_RGB10_A2` / `GL_RGBA16F` / `GL_RGBA32F` / `GL_R8`; BGRA8 formats are stored as RGBA (no BGRA internal format in GL 3.3 / GLES 3.0) and swizzled on upload; odd row pitches repacked on the CPU |
| render targets | an FBO each, created with the texture (`glCheckFramebufferStatus`: an unrenderable format returns `{}`); multisampled targets are renderbuffers (`glRenderbufferStorageMultisample`, GLES 3.0 has no multisample textures; the RHI never samples them) |
| copy destinations, readback sources | an FBO on first use |
| origin | GL's bottom-left: scissors, blits, draw copies, uploads into render targets and readback convert `y = h - y1`; each texture knows whether its rows are GL's (rendered, or copied from a render target) or top-first (uploaded); `Caps::framebufferOriginBottomLeft` |
| buffers | `GL_DYNAMIC_DRAW` buffers written through `GL_COPY_WRITE_BUFFER` (no other binding disturbed); `glBufferSubData` is ordered after the draws of earlier frames, so the driver versions it |
| constants | one 256 KB uniform-buffer ring, `glBindBufferRange` at binding points 0 / 1 / 2 with `GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT`; when full it is orphaned and the current values of all three slots are written again (bindings made before would point into the new storage) |
| textures and samplers | unit n = RHI slot n (`glUniform1i` per `ShaderBlob::textures` after linking, both stages merged by name); `SetPipeline` binds each used unit's sampler object (linear / point clamp); unit 8 is the backend's own for uploads, so slot bindings survive them |
| pipelines | one GL program per `ShaderProgram`, compiled at the first `CreatePipeline` and shared; a pipeline is program + blend + layout + topology; `effect` / `fxFeatures` / dual source without support return `{}` |
| vertex input | a UI VAO (attributes 0 / 1 / 2, 20-byte `esia::Vertex`, color `GL_UNSIGNED_BYTE` normalized), an empty VAO for id-only programs |
| draws | `glDrawElements(GL_TRIANGLES, n, GL_UNSIGNED_INT, first * 4)`, `glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, n)`, `glDrawArrays` |
| blend | `glBlendFuncSeparate`: straight, premultiplied, opaque (blending off), dual source `GL_SRC1_COLOR` / `GL_SRC1_ALPHA` |
| passes | bind the FBO, full viewport and scissor, `GL_FRAMEBUFFER_SRGB` on for sRGB targets (desktop; GLES always encodes); `Clear` = `glClear` with the scissor off (the clear color is linear and encoded into sRGB targets, as on the other APIs); `DontCare` = `glInvalidateFramebuffer` where it exists; stale unit bindings of the target are dropped (feedback loops) |
| `CopyTexture` | `glBlitFramebuffer` between formats of the same encoding (`GL_FRAMEBUFFER_SRGB` on for sRGB -> sRGB, so the decode / encode of a blit cancels); **sRGB -> raw copies (the backdrop copy of an sRGB target) are a draw** with the backend's own tiny program, because a blit linearizes an sRGB source on GLES (and, by the spec, on desktop GL before 4.4): the point sampler skips the decode (`EXT_texture_sRGB_decode`) or the shader re-encodes what it decoded (bit-exact for all 256 levels, tested); a multisampled source is resolved by the blit, or first into a same-format temporary when GLES's rules forbid it (different format, or a destination rectangle that is not the source rectangle) |
| direct render-target read | the headless target and wrapped host framebuffers with a color texture report `Sampled` (sRGB only with `EXT_texture_sRGB_decode`: the sampler objects skip the decode) |
| `ReadPixels` | `glReadPixels` with what GLES allows per format class (`RGBA`/`UNSIGNED_BYTE`, `UNSIGNED_INT_2_10_10_10_REV`, `FLOAT`), converted to RGBA8, rows flipped for GL-order textures, multisampled targets resolved first, sRGB as stored |
| profiling | `glQueryCounter(GL_TIMESTAMP)` (GL 3.3 core; GLES `EXT_disjoint_timer_query`) at frame start / end and around every profile scope, 4 frame slots, read without waiting (`GL_QUERY_RESULT_AVAILABLE`), GLES disjoint periods discard what is in flight; `ReadProfile` returns the newest complete frame |
| host callbacks | `NativeRenderState()` returns null (the current context is the state); the device forgets every cached binding, and the next call restores the fixed state, the pass's FBO, viewport, scissor and sRGB write |
| host state | `Desc::restoreHostState` (default on): everything the device changes (framebuffers, program, VAO, buffer bindings incl. indexed uniform buffers, texture units 0-8 and their samplers, viewport, scissor, enables, blend, color mask, clear color, polygon mode, pixel store) is saved at `BeginFrame` and around resource calls outside frames, and restored after |
| debug | `Desc::debug` (default: debug builds): `KHR_debug` synchronous output, errors and high / medium messages to stderr, errors counted (`ErrorCount`); without `KHR_debug`, `glGetError` after every call group; object labels from `debugName`; headless contexts are created with `EGL_CONTEXT_OPENGL_DEBUG` in debug builds |

### Caps

| Cap | `opengl` | `gles` | on llvmpipe (both) | `ESIA_GL_CORE_ONLY=1` |
| --- | --- | --- | --- | --- |
| `fxStorage` / `shaderFormat` | Texture / Glsl330 | Texture / Essl300 | | |
| `framebufferOriginBottomLeft` | yes | yes | | |
| `dualSourceBlend` | yes (core 3.3) | `EXT_blend_func_extended` | yes | gles: no |
| `floatRenderTargets` | yes (probed) | `EXT_color_buffer_half_float` or `_float` (probed) | yes | gles: no |
| `timestampQueries` | yes (core 3.3) | `EXT_disjoint_timer_query` | yes | gles: no |
| raw sampling of sRGB targets (reported as `Sampled`) | `EXT_texture_sRGB_decode` | same | yes | no |
| `sampleRenderTarget`, `readback` | yes | yes | | |
| `runtimeEffects`, `fxFeatureVariants`, `clipSpaceYDown`, `halfPixelOffset` | no | no | | |
| `maxTextureSize` / `maxFxDataWidth` | `GL_MAX_TEXTURE_SIZE` / min(that, 4096) | same | 16384 / 4096 | |

`maxFxDataWidth` is 4096, not `GL_MAX_TEXTURE_SIZE`: the renderer uploads whole rows of the instance texture, and a
16384-wide row is 256 KB per frame even for one instance.

## 2. Conformance

`esia_conformance --backend <b> --golden tests/conformance/golden --strict`, Debug and Release builds:

| Scene | `opengl` | `gles` | `opengl`, core only | `gles`, core only (RGBA8 pyramids and layers) |
| --- | --- | --- | --- | --- |
| shapes, gradients, shadows, text, edge_fade, clipping | PASS, max delta 0 | PASS, 0 | identical to `opengl` | identical |
| glass, windows, hidpi, light_streak | PASS, 0 | PASS, 0 | identical | PASS, max delta <= 2 |
| glow_layer | PASS, 0 | PASS, 0 | identical | PASS, max delta 6 (8 % of pixels over 2, none over the tolerance of 10) |
| text_lcd | PASS, 0 | PASS, 0 | identical | **SKIP**: no dual-source blending (no `EXT_blend_func_extended` on the GLES 3.0 minimum) |
| srgb_target | PASS, 0 | PASS, 0 | identical (copy by shader re-encode) | PASS, max delta 2 |
| msaa_target (vs. glass.png) | PASS, max delta 1 | PASS, 1 | identical | PASS, max delta 2 |

The `gles` images are pixel-identical to the `opengl` ones. The core-only columns were compared with the goldens
(`ESIA_GL_CORE_ONLY=1 esia_conformance --backend ... --out ...`, then a per-pixel diff).

### The image goldens (`tests/conformance/golden/*.png`, commit `77e936b`)

Produced with `esia_conformance --backend opengl --golden tests/conformance/golden --update` on Mesa 25.2.8
llvmpipe (a 4.5 core context; 3.3 requested). Every image was opened and checked against its scene in
`tests/conformance/scenes.cpp` (viewed at 2x; the committed files are pixel-identical to the viewed renders):

* **shapes**: square, circular (radius 14) and continuous corners side by side, per-corner radii 0 / 8 / 16 / 24
  with the square corner top-left (orientation), capsule, circle, the arc with its gap in the top-left quadrant
  (start -pi/2, sweep 1.5 pi), ring, 2 px and 8 px segments in their directions, liquid merge, inside / centered /
  outside strokes of growing extent, stroke-only circle, the capsule stroke fading downwards, the 50 % bar.
* **gradients**: 0 deg left to right, 45 deg towards the bottom right, 90 deg top to bottom (into transparent),
  radial highlight at (0.35, 0.35), conic seam at the top, seamless conic loop ring, spectrum capsule, the area
  chart's vertical fade under a smooth line, the mitered zigzag.
* **shadows**: soft drop shadow below the first card, blue offset + spread shadow right / below the second, inner
  shadow darker at the top, pink glow, inner glow with a teal outer glow, the green glow cut to its containment
  band vertically, shimmer highlights near the right ends at t = 1.25, noise grain.
* **glass**: the wallpaper (35 deg blue to magenta, orange and teal disks, 8 translucent stripes); the frosted card
  blurs disk and stripes and casts its shadow; the clear capsule on it shows the card below (glass on glass); the
  lens top right refracts a stripe into a curve with chromatic fringes and magnifies the teal disk's rim; the dark
  tinted and the blue-filled capsules.
* **glow_layer**: tinted teal bloom around the stroked rect and its glyphs (bar, ring, dot, triangle pointing up),
  untinted bloom of the pink disk and the yellow line, the glowing smooth polyline.
* **text**: the eight test glyphs in order at 32 / 20 / 12 px on dark and light, the diagonal as "/" and the E facing
  right (uploaded textures are not flipped), black at 60 % on a fractional position.
* **text_lcd**: sub-pixel coverage with blue fringes on left edges and red on right edges (RGB stripe order), the
  grayscale fallback with a pink bloom inside the glow layer.
* **edge_fade**: the left column cut hard at the clip, the right one fading over 28 px at the top and 44 px at the
  bottom, rows, glyphs and hairlines alike.
* **clipping**: nested clip intersection (disk and white bar cut at x = 150, y = 40..110), the replacing clip with the
  purple rect's rounded corners outside it, the rounded mask with the teal disk in its corner, the SDF image fill,
  the uv sub-rect, plain and mirrored textured quads (the test image's glow top-left, stripe along x = y).
* **windows**: Library over Settings (z-order), both glass surfaces blurring the wallpaper, rows clipped by the
  window (the overflowing last row leaves a sliver), the foreground capsule on top.
* **hidpi**: 2x scale, glass with its blur and reach in pixels, snapped glyphs, a 0.5 px and a 1 px hairline, the
  polyline and the glowing disk.
* **light_streak**: the caustic streak masked by the island, refracted and dispersed by the clear dome.
* **srgb_target**: the glass scene with translucent content blended in linear light (stripes brighter: the known
  behaviour, REWRITE_STATUS.md section 4 item 2); opaque content and what the clear lens shows through its copy
  match `glass.png` within 1 (so the backdrop copy holds the stored bits, not decoded ones).
* **msaa_target** (no golden of its own): within 1 / 255 of `glass.png`.

## 3. Verified here

Machine: Ubuntu 24.04 container, **no GPU**. clang / clang++ 18.1.3, ld.lld 18.1.3, CMake 3.28.3, Ninja 1.11.1,
Mesa 25.2.8 (llvmpipe, LLVM 20.1.2) through EGL (`libegl-mesa0`), mingw-w64 GCC 13 (posix) headers and libraries
for the cross build.

| Command | Result |
| --- | --- |
| `cmake --preset linux-clang -DESIA_WERROR=ON -DESIA_BACKEND_OPENGL=ON && cmake --build --preset linux-clang` | 0 warnings |
| `ctest --preset linux-clang` | 11 / 11 pass (the 8 of `esia-core`, `esia_rhi_opengl_tests`, `esia_conformance_opengl`, `esia_conformance_gles`) |
| `build/linux-clang/bin/esia_conformance --backend opengl --golden tests/conformance/golden --strict` | 14 / 14 PASS, `max delta 0` except msaa_target (1) |
| the same with `--backend gles` | 14 / 14 PASS, the same numbers |
| `ESIA_GL_CORE_ONLY=1` + both backends, `--out`, diffed with the goldens | section 2 (all pass; `gles` text_lcd SKIP) |
| `cmake --preset linux-clang-release -DESIA_WERROR=ON -DESIA_BACKEND_OPENGL=ON`, build, `ctest --preset linux-clang-release`, both strict runs | 0 warnings, 11 / 11, 14 / 14 twice |
| `cmake --preset windows-mingw-cross -DESIA_WERROR=ON -DESIA_BACKEND_OPENGL=ON && cmake --build --preset windows-mingw-cross` | builds every target incl. `esia_rhi_opengl_tests.exe`, 0 warnings (the WGL loader and `LoadLibrary` EGL paths compile); **not run** (no Wine here) |
| `git diff --stat origin/esia-core..HEAD` | only `src/esia/rhi/opengl/**` and the 13 goldens |

`esia_rhi_opengl_tests` (Debug and Release; `Desc::debug` forced on so errors count in Release too), each on
`opengl` and `gles`:

* `ConformanceScenesWithoutGlErrors`: all 14 scenes, two frames each, with all extensions and core-only: **0 GL
  errors** (KHR_debug) on every one - the task's "0 GL errors on GL 3.3 core and ES 3.0" check.
* `DebugOutputCountsErrors`: a deliberate `glEnable(0xFFFF)` is reported and counted (the zero above is real).
* `Caps`: the table in section 1, with and without `coreOnly`.
* `UploadAndReadBackFormats`: RGBA8 (whole, sub-rect read, sub-rect update with a row pitch), BGRA8 swizzle, R8,
  RGB10A2 conversion, RGBA32F clamping.
* `ScissorsAndReadbackAreTopLeft`: RGBA8, RGBA16F and sRGB targets, scissored draws in two corners, sub-rect reads.
* `CopiesKeepTheBits`: all 256 values per channel, sRGB -> raw exact with and without `EXT_texture_sRGB_decode`,
  copies at an offset, multisampled resolves at an offset (this found the GLES same-rectangle rule), sRGB clear
  colors encoded (0.25 / 0.5 / 0.75 -> 137 / 188 / 225).
* `HostStateIsRestored`: host FBO, texture unit 3, VAO, buffers, indexed uniform buffer, viewport, scissor, blend,
  color mask, cull face, clear color, pixel store survive a frame, `ReadPixels`, `CreateTexture`, `UpdateTexture`,
  `DestroyTexture` - and the host's odd state does not change the device's image.
* `WrappedFramebuffers`: a host FBO with its texture (frost reads it directly: 1 direct capture) and without (copy
  path), re-wrapping returns the same handle, both images equal the headless target's, the host's texture outlives
  the wrapper.
* `HostCallbacksChangeNothingAfterThem`: a callback that unbinds everything and sets hostile state, followed by a
  glass capture: the image equals the one without the callback (this found the fixed-state bug fixed in `e77a3e8`).
* `GpuTimes`: after 8 frames of the glass scene `RenderStats::gpu` is valid with total, capture and glass times
  (llvmpipe: ~8-12 ms per frame; its "GPU" time is CPU rasterization).
* `RendererKeepsTheContractWithGlCaps`: every scene through the null device configured with this backend's caps (both
  APIs, both modes): no RHI contract violation (checklist item 4).

Measured on llvmpipe: creating a device and compiling the programs a scene uses takes ~1.9 s with Mesa's shader
cache disabled (`MESA_SHADER_CACHE_DISABLE=true`, mostly the 257 KB FX fragment shader), ~0.1 s with it.

### Windows, NVIDIA (the local Windows session, after the cloud session)

Windows 11, NVIDIA GeForce RTX 4080 SUPER, driver 610.88; clang-cl 22.1.8 + lld-link, `windows-clang-cl` preset
with `-DESIA_WERROR=ON -DESIA_BACKEND_OPENGL=ON` (Debug): 0 warnings. GPU drivers on Windows ship no EGL, so every
headless test was a SKIP (and still "passed", REWRITE_STATUS known issue); the headless path now creates its
context through WGL first (`gl_headless.cpp`: a hidden window lends the pixel format; GLES through
`WGL_EXT_create_context_es2_profile`), EGL remaining the fallback. On that driver:

| Run | Result |
| --- | --- |
| `esia_rhi_opengl_tests` | 11 / 11 pass, 0 GL errors (the one reported error is `DebugOutputCountsErrors`' deliberate one); GPU times of the glass scene: 0.485 ms (`opengl`), 0.480 ms (`gles`) |
| `esia_conformance --backend opengl --golden tests/conformance/golden --strict` | 14 / 14 PASS against the llvmpipe goldens: max delta 1 - 6 per scene (shadows: 20 on 0.003 % of the pixels), mean delta <= 0.09 |
| the same with `--backend gles` (NVIDIA's ES 3.2 profile) | 14 / 14 PASS, the same numbers |

Three things only the real driver showed, fixed on this branch: NVIDIA's ES profile lists
`EXT_disjoint_timer_query` but rejects `GL_GPU_DISJOINT_EXT` (now probed once at creation); its timestamp queries
of a headless frame never complete after a `glFlush` alone (headless frames now end with `glFinish`; hosts have
their SwapBuffers); and the conformance scene table read a freed tolerance (fixed on `esia-core`, `3a8fe0a`).

## 4. Not verified

* **Other drivers.** Mesa llvmpipe (Linux) and NVIDIA (Windows) only. AMD, Intel, Apple, Adreno, Mali, PowerVR and
  ANGLE were not tried; driver-specific behaviour (blit sRGB semantics before GL 4.4, precision of the sRGB
  re-encode) is reasoned from the specs on the others.
* **Platforms.** Linux / EGL and Windows / WGL ran. The `libEGL.dll` fallback on Windows (ANGLE) was not run; macOS
  (`dlsym` default loader, `libEGL.dylib`) and Android (`libEGL.so`) were not compiled. The GLX branch of the Linux
  default loader was not exercised (the tests pass EGL's lookup).
* **`CreateDevice` on a host context** is exercised only indirectly (the headless path is `GlDevice` on an EGL
  context, and the tests act as the host with wrapped FBOs, host state and callbacks); no windowed application
  (GLFW / SDL, swap chains, the default framebuffer 0 as a target) was run.
* **Performance**: no comparison with WGT's D3D11 numbers yet (the NVIDIA GPU times above are of a 320 x 240 test
  scene).
* **Desktop contexts older than 4.5**: Mesa gives a 4.5 core context when 3.3 is requested, so "3.3" is checked by
  restricting what the backend uses (`coreOnly`), not by a real 3.3 driver.

## 5. Known issues and limits

1. **`Desc::debug` takes over the context's debug output** (`glDebugMessageCallback`), and the destructor clears it
   rather than restoring a host's callback. Hosts with their own callback pass `debug = false`.
2. **sRGB multisampled resolves average in linear light** (the blit's decode / encode); a raw average would differ
   slightly. D3D resolves sRGB formats in linear light too; no conformance scene has an sRGB multisampled target.
3. **Host state save / restore costs about 70 `glGet*` per frame** (and per resource call outside frames).
   `restoreHostState = false` skips it.
4. **Timestamps are dropped, not waited for,** when a frame's queries are still pending 4 frames later.
5. **No program binary cache** (`glGetProgramBinary`): every device compiles its programs; llvmpipe's first FX
   compile is ~1.5 s without Mesa's disk cache.
6. **Vertex / index / FX uploads rely on the driver versioning** `glBufferSubData` / `glTexSubImage2D` of objects
   the previous frame still reads (correct by GL's ordering rules, possibly a stall on some drivers); a
   per-frame-in-flight ring would avoid that.
7. **The EGL display of the headless path is never terminated** (it is shared by every headless device of the
   process).
8. `glInvalidateFramebuffer` is used only where it exists (GL 4.3 / `ARB_invalidate_subdata`, GLES 3.0); `DontCare`
   is a no-op on older desktop GL.

## 6. Building and testing on the real platforms

* **Linux (any Mesa, NVIDIA, AMD)**: `cmake --preset linux-clang -DESIA_BACKEND_OPENGL=ON && cmake --build --preset
  linux-clang && ctest --preset linux-clang`. The headless tests need `libEGL.so.1` (glvnd or Mesa) with a GPU or
  Mesa's surfaceless platform; `EGL_KHR_surfaceless_context` or a pbuffer config is required. Set
  `ESIA_GL_CORE_ONLY=1` to run the conformance suite on the GL 3.3 / GLES 3.0 minimum.
* **Windows**: `cmake --preset windows-clang-cl -DESIA_BACKEND_OPENGL=ON` (or `vs2022` with the option). Hosts
  create the device on their WGL context (`Desc::getProcAddress` may stay null: `wglGetProcAddress` +
  `opengl32.dll`). The headless tests use the GPU driver through WGL (`gles` needs
  `WGL_EXT_create_context_es2_profile`: NVIDIA, AMD); where WGL cannot create a context they fall back to ANGLE's
  `libEGL.dll` / `libGLESv2.dll` next to the executables (GLES only).
* **macOS**: desktop GL 4.1 core through CGL / NSOpenGL (`macos-clang` preset + the option); the headless tests need
  ANGLE's `libEGL.dylib` (GLES) on the library path, else SKIP.
* **Android / embedded Linux**: GLES 3.0+ through the platform's EGL (`libEGL.so`); pass `eglGetProcAddress` as
  `Desc::getProcAddress`.
* **Host integration**: see `include/esia/rhi/opengl.hpp` (the context must be current on the renderer's thread for
  every call; wrap the swap chain's framebuffer each frame; `FrameDesc::nativeContext` is null).

## 7. Core change requests

Not made on this branch (the rules forbid it); each would be a change on `esia-core`.

1. **Let the conformance suite fail on API validation errors.** Why: "0 GL errors" is enforced only by this
   backend's own test; the suite itself cannot see KHR_debug errors, and the D3D debug layer / Vulkan validation
   have the same need. Smallest change: `virtual std::uint32_t ValidationErrors() const { return 0; }` on
   `rhi::Device` (this backend would return `ErrorCount`), and `conformance.cpp` failing a scene when it is non-zero
   after `ReadPixels`.
2. **Fix the GL recipe in `docs/backends/README.md`, section 5**: it says `CopyTexture` = `glBlitFramebuffer` with
   `GL_FRAMEBUFFER_SRGB` off. That is not a raw copy for an sRGB source on GLES (a blit always linearizes an sRGB read
   buffer there) nor, by the letter of the spec, on desktop GL before 4.4; the srgb_target scene came out dark on
   `gles` until the draw copy (section 1). Also worth a line: GLES resolves a multisampled source only into the
   identical rectangle and format.
3. **Conformance coverage the scenes lack** (bugs this session found only with its own tests): a target that is
   *not* sampleable (`HeadlessDesc::sampleable = false`, e.g. a `glass_copy` scene sharing `glass.png`: frost then
   takes the copy path instead of the direct read), an sRGB multisampled target, `CopyTexture` to an offset other
   than the source position, and a host callback followed by a capture.
4. **`docs/REWRITE_STATUS.md` section 3** still says "Conformance image goldens: none yet": once `esia-opengl` is
   merged they exist (`77e936b`).
