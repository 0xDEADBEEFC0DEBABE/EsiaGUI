# Writing an Esia RHI backend

This guide is for the four backend sessions (`esia-opengl`, `esia-vulkan`, `esia-metal`, `esia-directx`) and for
anyone porting Esia to another API or engine. Read [../REWRITE.md](../REWRITE.md) sections 5 - 8 first: the renderer
already does batching, capture planning, pyramids, glow layers and profiling; **a backend only translates
`rhi::Device` calls** (`include/esia/rhi/rhi.hpp`). Everything it needs is in `esia-core`; a backend branch adds
files under its own directory and changes nothing else.

* [1. Files and CMake](#1-files-and-cmake)
* [2. What to implement](#2-what-to-implement)
* [3. Capabilities](#3-capabilities)
* [4. Shaders and the binding model](#4-shaders-and-the-binding-model)
* [5. API recipes](#5-api-recipes)
* [6. LLVM toolchain](#6-llvm-toolchain)
* [7. Conformance suite](#7-conformance-suite)
* [8. Checklist](#8-checklist)

---

## 1. Files and CMake

### Layout (backend-specific files only)

| Branch | Directories it adds | CMake options |
| --- | --- | --- |
| `esia-opengl` | `src/esia/rhi/opengl/` (OpenGL 3.3 core and OpenGL ES 3.0 in one target, two registered backends `opengl` and `gles`) | `ESIA_BACKEND_OPENGL` |
| `esia-vulkan` | `src/esia/rhi/vulkan/` | `ESIA_BACKEND_VULKAN` |
| `esia-metal` | `src/esia/rhi/metal/` (Objective-C++ `.mm` allowed) | `ESIA_BACKEND_METAL` |
| `esia-directx` | `src/esia/rhi/directx/common/` (shared: DXGI format tables, `D3DCompile` glue with the include handler, capture / resolve helpers, the SM3 prelude if shared), `src/esia/rhi/directx/d3d9/`, `src/esia/rhi/directx/d3d10/`, `src/esia/rhi/directx/d3d11/`, `src/esia/rhi/directx/d3d12/` | `ESIA_BACKEND_DIRECTX` (`ESIA_BACKEND_D3D9` / `D3D10` / `D3D11` / `D3D12`, advanced, leave out single versions) |

The options exist in `src/esia/CMakeLists.txt`, on by default where the platform has the API (Windows: DirectX and
OpenGL, Vulkan with its headers; Apple: Metal; elsewhere: OpenGL, Vulkan with its headers). An option adds
`src/esia/rhi/<name>/CMakeLists.txt`; `ESIA_BACKEND_DIRECTX` adds `src/esia/rhi/directx/CMakeLists.txt`, which adds
`common/` and each Direct3D version left on. Inside a backend directory:

```
src/esia/rhi/<name>/
  CMakeLists.txt
  include/esia/rhi/<name>.hpp     host integration: create the device from the host's API objects, wrap targets
  <name>_device.cpp ...           the rhi::Device implementation (private headers next to it)
  tests/                          optional unit tests of the backend (esia_add_test; tests that render the
                                  conformance scenes link esia_conformance_scenes, section 7)
  esia_sm3_prelude.hlsli          d3d9 only: the SM3 prelude (section 4.4); tools/shaders looks for it here
```

### CMakeLists.txt template

```cmake
# src/esia/rhi/opengl/CMakeLists.txt
add_library(esia_rhi_opengl STATIC gl_device.cpp gl_loader.cpp)
target_include_directories(esia_rhi_opengl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(esia_rhi_opengl PUBLIC esia_rhi PRIVATE esia_shaders)
esia_target_defaults(esia_rhi_opengl)            # C++20, warnings (-Wall -Wextra -Wpedantic -Wshadow / /W4)
esia_register_backend(opengl esia_rhi_opengl)    # one call per backend name the target provides
esia_register_backend(gles esia_rhi_opengl)

if(BUILD_TESTING)                                # optional backend unit tests
    esia_add_test(esia_rhi_opengl_tests tests/test_gl_formats.cpp)
    target_link_libraries(esia_rhi_opengl_tests PRIVATE esia_rhi_opengl)
endif()
```

`esia_register_backend(<name> <target>)` links the target into `esia_backends` and makes the generated
`RegisterBuiltinBackends()` call **`void EsiaRegisterBackend_<name>()`** (global namespace, defined by the target):

```cpp
void EsiaRegisterBackend_opengl()
{
    esia::rhi::BackendInfo info;
    info.name = "opengl";
    info.createHeadless = &CreateHeadlessGl;   // HeadlessDevice (*)(const HeadlessDesc&, std::string& error)
    esia::rhi::RegisterBackend(info);
}
```

`createHeadless` returns a device plus an offscreen render target of `HeadlessDesc`'s size, format and sample count
(create it with `RenderTarget | CopySrc`, plus `Sampled` when `sampleable` and the backend can sample it raw). When
the machine cannot run the backend (no driver, no display, no GPU), return an empty `HeadlessDevice` and say why in
`error`: the conformance suite reports SKIP instead of failing. It also registers two CTests per backend name,
`esia_conformance_<name>` and `esia_conformance_<name>_frames` (section 7); when every scene is skipped they report
"skipped" (exit code 77), not "passed".

### Host integration header

Hosts create the device from their own API objects and hand their render target in every frame. Each backend
defines this in `include/esia/rhi/<name>.hpp` (namespace `esia::rhi::<name>`); recommended shapes:

| Backend | Device creation | Render target | `FrameDesc::nativeContext` |
| --- | --- | --- | --- |
| OpenGL / GLES | `CreateDevice(const Desc&)` on the current context (`Desc`: ES or core, optional `getProcAddress`) | `WrapFramebuffer(dev, GLuint fbo, int w, int h, Format, GLuint colorTexture = 0)` (0 = window / renderbuffer: not sampleable) | null |
| Vulkan | `CreateDevice(const Desc&)` (instance, physical device, device, queue family, frames in flight, `VkPipelineCache`) | `WrapImage(dev, VkImage, VkImageView, VkFormat, w, h, samples, VkImageLayout layoutOnEntry, VkImageLayout layoutOnExit, usage)` | `VkCommandBuffer` (recording, outside a render pass) |
| Metal | `CreateDevice(id<MTLDevice>)` | `WrapTexture(dev, id<MTLTexture>)` | `id<MTLCommandBuffer>` |
| Direct3D 9 | `CreateDevice(IDirect3DDevice9*)` (9Ex recommended) | `WrapRenderTarget(dev, IDirect3DSurface9*, IDirect3DTexture9* = nullptr)` | null (the device) |
| Direct3D 10 | `CreateDevice(ID3D10Device*)` | `WrapRenderTarget(dev, ID3D10RenderTargetView*)` | null |
| Direct3D 11 | `CreateDevice(ID3D11Device*, ID3D11DeviceContext*, bool restoreHostState = true)` | `WrapRenderTarget(dev, ID3D11RenderTargetView*)` | null |
| Direct3D 12 | `CreateDevice(ID3D12Device*, int framesInFlight)` | `WrapRenderTarget(dev, ID3D12Resource*, D3D12_CPU_DESCRIPTOR_HANDLE rtv, DXGI_FORMAT, D3D12_RESOURCE_STATES stateOnEntryAndExit)` | `ID3D12GraphicsCommandList*` |

A wrapped target is an `rhi::Texture` like any other: `GetTextureDesc` reports its size, format, sample count and
usage (`RenderTarget`; `CopySrc` when it can be copied from, `Sampled` only if it can be sampled raw - see "Color"
below); `DestroyTexture` releases the wrapper, never the host's object. Swap-chain buffers change every frame: make
wrapping cheap (cache by the native pointer). Report the usage honestly: glass on a target without `CopySrc` (a Metal
`framebufferOnly` drawable, a swap-chain image without `TRANSFER_SRC`) reads it directly, pyramid level 1 standing in
for the full-resolution level (blurrier, still valid); a target that is neither copyable nor sampleable gives glass
no backdrop.

## 2. What to implement

`rhi::Device`, call by call. The null device (`src/esia/rhi/null/null_device.cpp`) enforces every rule below and
is the executable reference: run the conformance suite with `--backend null --out dir` and read
`dir/null/<scene>.log` to see exactly what the renderer sends for each scene.

### Resources

| Call | Contract |
| --- | --- |
| `CreateTexture(desc, data, rowPitch)` | 2D, one mip level. `data` (tightly packed if `rowPitch == 0`, top row first) or null (contents undefined). Formats used by the renderer: `RGBA8_UNORM` (images, white), `R8_UNORM` (glyph pages; shaders read `.r`), `RGBA16_FLOAT` (pyramid, glow layer; `RGBA8_UNORM` when `!floatRenderTargets`), `RGBA32_FLOAT` (FX instance texture: `Sampled | CopyDst`, fetched by texel), `RawFormat(target)` (backdrop copy: `Sampled | CopyDst`). Host targets may also be `*_SRGB`, `BGRA8_*`, `RGB10A2_UNORM`. Return `{}` on failure. |
| `UpdateTexture(tex, rect, data, rowPitch)` | Replace `rect` (top-left origin). Before the frame's first pass. |
| `DestroyTexture(tex)` | Any time outside passes; defer the real release until the GPU is done with it. |
| `GetTextureDesc(tex)` | Size, format, usage, samples. The renderer reads it for the target every frame. |
| `CreateBuffer(desc)` / `UpdateBuffer(buf, data, size)` / `DestroyBuffer` | `Vertex` (20-byte `esia::Vertex`), `Index` (uint32), `FxInstances` (float4 rows, `FxStorage::Buffer` only). Updates replace the first `size` bytes, before the first pass; version them per frame in flight. |
| `MapBuffer(buf, size)` / `UnmapBuffer(buf)` | `UpdateBuffer` written in place: the renderer writes the frame's vertices, indices and FX instances straight from the draw lists, one buffer at a time. The default stages them for `UpdateBuffer`; a backend whose buffers map hands out their memory (Direct3D 9 - 11: lock / map with discard; Direct3D 12: the buffer's copy for the ring). |
| `CreatePipeline(desc)` | Program (vertex + pixel shader of `ShaderProgram`), vertex layout, topology, blend, target format, samples, and for `Fx`: `effect` / `effectSource` (user effects, only with `runtimeEffects`), `fxFeatures` (only with `fxFeatureVariants`) and `background` (only with `asyncPipelines`, below). May be called inside a pass. Return `{}` if the combination is impossible (a sample count the format cannot have, an effect that failed): the renderer falls back or skips - a refused FX variant is drawn with the full shader (`fxFeatures = 0`, counted in `RenderStats::fxFallbacks`). A user effect may return `{}` while it compiles in the background; the renderer asks again on a later frame. |
| `GetPipelineStatus(p)` | Only with `asyncPipelines`: `Pending` while a `background` pipeline compiles, `Failed` when it could not be built, else `Ready` (the default implementation). The renderer never binds a pipeline that is not `Ready`. |

### Frames, passes, state

| Call | Contract |
| --- | --- |
| `BeginFrame(desc)` / `EndFrame()` | One `Renderer::Render` = one device frame. `nativeContext` is the backend's recording object, if it records into the host's. A backend that records into the host's command list and cannot fence the host's work (D3D12, Vulkan with `nativeContext`) recycles its per-frame slots (upload ring, descriptors, queries) by `hostFrame`: device frames with the same non-zero number share a slot - a host rendering several targets per frame passes the same number to each `Render` - and 0 makes every `BeginFrame` a frame of its own. Backends that fence their own submissions count device frames. Return false to skip the frame. |
| `BeginPass({target, load})` / `EndPass()` | `Load` keeps the contents, `Clear` clears to `clearColor`, `DontCare` means nothing outside what the pass draws is ever read (pyramid levels, fresh glow layers: tilers need not load them). Viewport = whole target. **Nothing is bound at the start of a pass**; scissor = whole target. |
| `SetPipeline(p)` | Its target format and samples match the pass target. |
| `SetScissor(rect)` | Top-left pixels, inside the target, may be empty only if no draw follows. Bottom-left APIs convert: `y = height - rect.y1`. |
| `SetConstants(slot, data, size)` | Frame (192 bytes), Pass (32), Draw (32); multiples of 16. Inline: each draw sees the latest values; version them (ring buffer, push / root constants, `setVertexBytes`). |
| `SetTexture(slot, tex)` | t0..t7. Never the current pass target. Bind with the sampler the shader expects (section 4). |
| `SetFxBuffer(buf)` | t7 as a structured / storage buffer (`FxStorage::Buffer`). |
| `SetVertexBuffer` / `SetIndexBuffer` | The frame's buffers; indices are 32-bit. Without `baseVertex` they are rebased onto one merged vertex array; with it every draw list's are as the list wrote them (`DrawIndexedBase`). |
| `Draw(3, 0)` | Full-screen triangle, no vertex buffer: `SV_VertexID` 0..2 (Downsample, LayerComposite, Clear). |
| `DrawIndexed(count, first)` | Triangle list of `esia::Vertex` (UiGeometry, TextGray). |
| `DrawIndexedBase(count, first, base)` | Only with `baseVertex`: the same, `base` added to every index (a draw list's first vertex in the frame's vertex buffer). |
| `DrawInstanced(4, n)` | Triangle strip, no vertex buffer, `SV_InstanceID` 0..n-1; the draw's first instance is in the Draw constants. |
| `DrawInstancedFrom(4, n, first)` | Only with `drawFirstInstance`: the same, with the shaders' instance index starting at `first` (the Draw constants' first instance is 0). Direct3D 11 feeds it as an instance-rate attribute (`INSTANCEINDEX`, an immutable 0, 1, 2 ... buffer at slot 1, `ESIA_INSTANCE_ATTRIBUTE` in `FxVS`): `SV_InstanceID` does not count `StartInstanceLocation`. |
| `CopyTexture(dst, x, y, src, rect)` | Outside passes; raw bits (`dst` is `src`'s format or `RawFormat(src)`); resolves a multisampled source. `(x, y)` may differ from `rect`'s position (the renderer copies captures in place, the conformance suite checks an offset): where the API resolves only whole subresources or identical rectangles (D3D11 `ResolveSubresource`, GLES blits), resolve into a temporary first. |
| `NativeRenderState()` | Inside a pass: the object host callbacks record with (table in section 1). Forget every binding afterwards. |
| `BeginProfile(category)` / `EndProfile()` / `ReadProfile(out)` | Non-nesting scopes, inside or outside passes. Resolve timestamps a few frames later without stalling; `ReadProfile` returns the latest complete frame (`GpuProfile::totalMs`, per-category ms). With `timestampQueries = false` these are no-ops. |
| `ReadPixels(tex, rect, rgba8)` | Tests and screenshots: waits for the GPU; RGBA8, top row first, whatever the format (float targets converted, sRGB returned as stored, multisampled resolved). |
| `ValidationErrors()` | Messages at warning or error level the API's validation reported since the device was created: D3D debug layer / info queue (and the D3D backends' own error and warning log), Vulkan validation layer (debug messenger), GL `KHR_debug`; 0 when validation is off. The conformance suite fails a scene whose device reports any after its readback. |

### Background pipelines (optional: `Caps::asyncPipelines`)

For backends whose FX variants take long to build (Direct3D 9 compiles them with `D3DCompile`: up to seconds each on
a real driver in the first frames). The renderer requests every FX variant with `PipelineDesc::background = true`.
With `asyncPipelines`, `CreatePipeline` may return a handle at once and build it off the render thread:

* `GetPipelineStatus` reports `Pending` until the pipeline can be bound, then `Ready`, or `Failed` for good.
* While a batch's variant is `Pending`, the renderer draws the batch with the smallest `Ready` variant whose feature
  mask covers the batch's (same blend, format and samples), else with the full shader (`fxFeatures = 0`), and counts
  it in `RenderStats::fxPendingVariants`; a `Failed` variant is drawn with the full shader, like a refused one. No
  frame waits for a variant; the batch switches to it once it is ready.
* So build the full FX pipeline early (at device creation, or first on the worker): it is every batch's fallback.
* Every `rhi::Device` call still comes from the render thread: the worker only compiles (`D3DCompile` is
  thread-safe); create the API object (`CreatePixelShader` ...) on the render thread when the compile finished,
  for instance in `GetPipelineStatus`. `DestroyPipeline` of a `Pending` pipeline discards the result when it arrives.
* Without the cap `background` is ignored and every pipeline is built in `CreatePipeline`, as before.

### Color and coordinates

* Shaders compute in gamma space on stored values and convert their own output to linear for `*_SRGB` targets
  (`gTime.w`). So: **enable the hardware sRGB encode when rendering into an `*_SRGB` target** (GL:
  `GL_FRAMEBUFFER_SRGB`; D3D9: `D3DRS_SRGBWRITEENABLE`; elsewhere the format does it) and **never decode sRGB when
  sampling**: only report `Sampled` on an sRGB host target when you can sample it through a UNORM view (D3D:
  typeless resource + UNORM SRV; GL: `EXT_texture_sRGB_decode` / texture views; Vulkan: `MUTABLE_FORMAT` + UNORM
  view; Metal: `PixelFormatView` usage + `newTextureViewWithPixelFormat`).
* Every rectangle the RHI passes (scissor, copy, update, readback) is top-left based. A bottom-left API (OpenGL)
  converts rectangles on render targets and sets `framebufferOriginBottomLeft`: the shaders flip `SV_Position`
  (`WgtPixelPos`) and the uv of render-target textures (`WgtRtUv`). Uploaded textures (images, glyphs, the FX
  texture) are stored top row first on every API and are not flipped.

## 3. Capabilities

| `Caps` field | Meaning / what the renderer does | D3D9 | D3D10 | D3D11 | D3D12 | GL 3.3 | GLES 3.0 | Vulkan | Metal |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `fxStorage` | `Buffer`: FX instances in a buffer (`SetFxBuffer`); `Texture`: RGBA32F texture at t7, `perRow = maxFxDataWidth / 24` instances per row (`gConv.z`; fewer with `RenderParams::maxFxInstancesPerRow`) | Texture | Texture | Buffer | Buffer | Texture | Texture | Buffer | Buffer |
| `shaderFormat` | informational: the `shaders::Format` the backend loads | DxbcSm3 | DxbcSm4 | DxbcSm5 | DxbcSm5 / Dxil | Glsl330 | Essl300 | SpirV | Msl |
| `framebufferOriginBottomLeft` | shaders flip pixel positions and render-target uv | no | no | no | no | yes | yes | no | no |
| `clipSpaceYDown` | the renderer flips its projection (`gXform.y > 0`; FullscreenVS follows) | no | no | no | no | no | no | yes (or a negative viewport height and no) | no |
| `halfPixelOffset` | renderer sets `gConv.y = 0.5`; the SM3 prelude shifts positions by half a pixel | yes | no | no | no | no | no | no | no |
| `floatRenderTargets` | RGBA16F pyramid and layers (else RGBA8) | if RGBA16F blending | yes | yes | yes | yes | `EXT_color_buffer_half_float` | yes | yes |
| `sampleRenderTarget` | targets reported `Sampled` may be read right after their pass (the no-copy capture path) | yes | yes | yes | yes | yes | yes | yes | yes |
| `timestampQueries` | GPU time per category | yes | yes | yes | yes | yes | `EXT_disjoint_timer_query` | yes | not at first |
| `readback` | `ReadPixels` works: required by the conformance suite | yes | yes | yes | yes | yes | yes | yes | yes |
| `runtimeEffects` | user HLSL effects compiled at runtime | yes | yes | yes | yes | no | no | no | no |
| `fxFeatureVariants` | one FX pipeline per batch feature mask (`PipelineDesc::fxFeatures`) | yes | optional | optional | optional | no | no | no | no |
| `asyncPipelines` | variants build off the render thread; batches draw with a ready pipeline meanwhile (section 2) | recommended | optional | optional | optional | no | no | no | no |
| `firstDrawCompiles` | the driver finishes a shader at its first draw: the renderer draws a ready user effect once where no pixel is written (an empty scissor) | yes (NVIDIA) | no | no | no (its debug layer warns about the empty scissor) | no | no | no | no |
| `baseVertex` | `DrawIndexedBase` (above): every list's vertices and indices go to the GPU as they are, no merged copy and no rebasing | yes | yes | yes | yes | yes (3.2 core) | no | yes | no (not yet) |
| `drawFirstInstance` | `DrawInstancedFrom` (above): the Draw constants then change only with the edge fade - each change is a buffer update, one per FX batch was most of Direct3D 11's submit | no | no | yes | no | no | no | no | no |
| `maxTextureSize`, `maxFxDataWidth` | limits (the FX texture is `perRow * 24` wide) | device caps; `maxFxDataWidth` = 24 x a power of two | 8192 | 16384 | 16384 | `GL_MAX_TEXTURE_SIZE` | same | limits | 16384 |

## 4. Shaders and the binding model

The generated library (`include/esia/render/shader_library.hpp`, target `esia_shaders`) holds every program in
every format built so far:

```cpp
const shaders::ShaderBlob* vs = shaders::Find(shaders::Format::Glsl330, rhi::ShaderProgram::Fx, shaders::Stage::Vertex);
// vs->data / vs->size: GLSL / ESSL / MSL text (null-terminated) or SPIR-V / DXBC / DXIL bytes
// vs->textures / vs->textureCount: GLSL / ESSL only - sampler uniform name -> RHI slot + sampler
shaders::ProgramSource src = shaders::SourceOf(rhi::ShaderProgram::Fx);   // "esia_fx.hlsl", "FxVS", "FxPS"
const char* hlsl = shaders::FindSource("esia_common.hlsli");              // for D3DCompile's include handler
```

| Program | Entry points (file) | Vertex input | Topology | Blend the renderer uses | Reads |
| --- | --- | --- | --- | --- | --- |
| `UiGeometry` | `UiVS` / `UiPS` (`esia_ui.hlsl`) | `esia::Vertex` | list | Straight | Frame, Draw, t0 (linear) |
| `TextGray` | `UiVS` / `TextGrayPS` | `esia::Vertex` | list | Straight | Frame, Draw, t0 (linear, `.r`) |
| `Fx` | `FxVS` / `FxPS` (`esia_fx.hlsl`) | none (`SV_VertexID`, `SV_InstanceID`) | strip | Premultiplied | Frame, Draw, t0..t6 (linear), t7 FX data |
| `Downsample` | `FullscreenVS` / `DownsamplePS` (`esia_post.hlsl`) | none (`SV_VertexID`) | list | Opaque | Frame, Pass, t0 (linear) |
| `LayerComposite` | `FullscreenVS` / `LayerCompositePS` | none | list | Premultiplied | Frame, Pass, t0 (point), t1..t6 (linear) |
| `Clear` | `FullscreenVS` / `ClearPS` | none | list | Opaque | Frame |

**FX instance data.** An instance is 24 float4 rows (`fx::Instance`). `FxVS` fetches the rows every pixel reads
(rect, radii, fill0, shape, misc, flags) and passes them as flat varyings (`nointerpolation`: GLSL `flat`, MSL
`[[flat]]`; SM3 has no flat interpolation and gets the same values interpolated, the flags rounded); `FxPS` fetches
each other row inside the branch that uses it. The FX program therefore has 8 varyings (the quad's local position,
the instance index and the six rows): within GL 3.3 / GLES 3.0 / D3D10 (15-16 vec4) and SM3 (10 interpolators). The
define `ESIA_FX_FETCH_ALL=1` restores the old path (every row fetched at the start of `FxPS`, 2 varyings) for A / B
measurements: `tools/shaders/build_shaders.py --define ESIA_FX_FETCH_ALL=1` or `/DESIA_FX_FETCH_ALL=1` in
`D3DCompile`; the pixels are identical, only the cost differs (do not commit a library built that way).

**The solid area.** Most pixels of a window, a card or a large button lie inside a solid rounded rectangle, away
from its edge and its corners, where every layer of the FX shader but the fill is zero or under an opaque fill.
`FxVS` passes that area in two more varyings (`solid`, `solidCorner`: the center and half sizes less the edge band -
the stroke inside the edge and 2 pixels - and less the corners, 1.6 x the radius for continuous ones), and `FxPS`
returns the fill color there without the distance field, the same bits as the full path. On an RTX 4080 SUPER four
solid 380 x 900 windows took 0.026 ms of GPU time and take 0.016 ms. SM3 has neither the varyings nor the
instruction slots to spare: `ESIA_COMPACT` (the D3D9 prelude) leaves it out, as it keeps the compact forms of the
backdrop sampling (`esia_common.hlsli`).

**Until the next regeneration of the library**, the SPIR-V path is compiled as compact too (`esia_common.hlsli`
defines `ESIA_COMPACT` with `ESIA_SPIRV`): the generated SPIR-V, GLSL, ESSL and MSL are those of the sources before
the solid area and the straight-line backdrop sampling, byte for byte (`build_shaders.py` gives the same output for
both with the Vulkan SDK's tools). The Direct3D 10 - 12 backends compile the source at run time and have both. To
give them to OpenGL, GLES, Vulkan and Metal, drop that define and regenerate the library with the CI's tool versions
(docs/CI.md, the Shaders workflow).

Blend modes: `Opaque` = replace; `Straight` = rgb `src*srcA + dst*(1-srcA)`, alpha `src + dst*(1-srcA)`;
`Premultiplied` = rgb and alpha `src + dst*(1-srcA)`. No dual-source blending, no depth, no stencil, no culling.
Samplers: s0 linear / clamp / no mips, s1 point / clamp / no mips.

**Text is grayscale only.** By the owner's decision, Esia has no sub-pixel (LCD / ClearType) text: every glyph page
is Alpha8 coverage drawn with `TextGray` (core round 3 removed `TextLcd`, `TextLcdGray`, `BlendMode::DualSourceLcd`
and `Caps::dualSourceBlend`). A backend has no dual-source blend state, feature or extension to set up.

### 4.1 Binding numbers

| Resource | HLSL | SPIR-V set 0 binding | Metal index | GLSL / ESSL |
| --- | --- | --- | --- | --- |
| `WgtFrame`, `WgtPass`, `WgtDraw` | b0, b1, b2 | 0, 1, 2 (uniform buffers) | buffer 0, 1, 2 | uniform blocks named `WgtFrame` / `WgtPass` / `WgtDraw`: `glUniformBlockBinding` to 0 / 1 / 2 |
| `gTex` | t0 | 3 (sampled image) | texture 3 | see the sampler table |
| `gBackdrop0..5` | t1..t6 | 4..9 (sampled images) | texture 4..9 | |
| `gFxData` | t7 | 10 (storage buffer) | buffer 10 (`device const`) | texture (texture storage) |
| `gLinear`, `gPoint` | s0, s1 | 11, 12 (samplers) | sampler 11, 12 | |
| vertex `pos`, `uv`, `color` (R8G8B8A8 unorm) | `POSITION`, `TEXCOORD0`, `COLOR0` | locations 0, 1, 2 | attributes 0, 1, 2 (`stage_in`) - put the vertex buffer at an unused index, e.g. 30 | locations 0, 1, 2 |

### 4.2 GLSL 330 / ESSL 300

* Uniform blocks by name as above. Varyings are already named `esia_v<location>` in both stages (GL 3.3 links by
  name). Attributes have explicit locations.
* Samplers are combined: `ShaderBlob::textures` lists each `uniform sampler2D` with its RHI slot and sampler (0
  linear, 1 point). Give each its own texture unit (`glUniform1i` after linking), bind the texture of `slot` and a
  sampler object (`glBindSampler`) of the listed kind to that unit. Both stages of a program list their samplers;
  merge by name.
* `gFxData` is listed with the point sampler: keep it that way - RGBA32F is not filterable on GLES 3.0, and a linear
  filter makes the texture incomplete, which makes `texelFetch` return zeros. Every texture has one mip level
  (`GL_TEXTURE_MAX_LEVEL 0` or non-mipmap filters).
* The ESSL is `highp` throughout (the build script enforces it).
* `esia_glsl_link_tests` compiles and links every program on the machine's driver; the GL backend's first test.

### 4.3 SPIR-V (Vulkan) and MSL (Metal)

* SPIR-V 1.0 (`--target-env vulkan1.0`); the entry point keeps the HLSL name, `ShaderBlob::entry` (`FxVS`,
  `FxPS` ...): pass it as `VkPipelineShaderStageCreateInfo::pName`.
  Separate images and samplers; one descriptor set layout for every program: bindings 0-2 `UNIFORM_BUFFER` (or
  `_DYNAMIC`), 3-9 `SAMPLED_IMAGE`, 10 `STORAGE_BUFFER`, 11-12 `SAMPLER`, all `VERTEX | FRAGMENT`. Only what a
  program uses must be valid when it draws (what the null device checks). Validated with `spirv-val` at build
  time.
* MSL 2.0, entry point `esia_main` in every source (one `MTLLibrary` per blob, `newLibraryWithSource`), indices as in
  the table. The FX data is a `device` buffer; constants may come through `setVertexBytes` / `setFragmentBytes` at
  0 / 1 / 2. MSL cannot be compiled on Linux: validate it on macOS (`xcrun -sdk macosx metal -c file.metal`).

### 4.4 Direct3D (DXBC / DXIL, runtime compilation, SM3)

* `shaders::Available(Format::DxbcSm5)` etc. is false until someone ran `python tools/shaders/build_shaders.py
  --fxc fxc` (and `--dxc dxc`) and committed the output. `--fxc` also takes `"wine <fxc>.exe"`: on Linux,
  Microsoft's `d3dcompiler_47.dll` runs under Wine (Wine's own d3dcompiler cannot compile these shaders). Until
  then, and for user effects and FX variants always, compile at device creation with `D3DCompile`
  (`d3dcompiler_47.dll`, part of Windows 10 / 11; mingw-w64 has the import library): source
  `shaders::FindSource(SourceOf(p).file)`, an `ID3DInclude` that serves `shaders::FindSource(name)`, entry points
  from `SourceOf`, profiles `vs_5_0` / `ps_5_0` (D3D11 / 12), `vs_4_0` / `ps_4_0` with `ESIA_FX_STORAGE_TEXTURE=1`
  (D3D10), `vs_3_0` / `ps_3_0` with `ESIA_FX_STORAGE_TEXTURE=1` and the SM3 prelude (D3D9). Cache the bytecode (the
  FX pixel shader takes about a second to compile): `d3d_common` keeps it per process and, when the host calls
  `esia::rhi::d3d::SetShaderCacheDirectory` (`esia/rhi/d3d_common.hpp`; glass_app: `%LOCALAPPDATA%\Esia\ShaderCache`),
  on disk, keyed by the preprocessed source, profile, flags and compiler DLL. On the RTX 4080 the showcase's first
  frame went from 0.65 s (D3D11) and 1.8 s (D3D9) to 30 ms and 0.2 s once cached.
* Registers: `cbuffer` b0-b2, textures t0-t7, samplers s0-s1; D3D12's root signature: 3 root CBVs (or root
  constants for Draw), one SRV table t0-t7, two static samplers.
* User effects: compile `esia_fx.hlsl` with `ESIA_CUSTOM_EFFECT=1`, the include handler returning the user's
  source for `esia_user_effect.hlsli` (it must define `float4 WgtEffect(WgtFx fx)`).
* **The SM3 prelude** (D3D9): `src/esia/rhi/directx/d3d9/esia_sm3_prelude.hlsli`. fxc / `D3DCompile` cannot `#include` a
  macro (X1500), so either prepend it to the source (what the D3D9 backend does), or define `ESIA_SHADER_PRELUDE` and
  serve it as `esia_shader_prelude.hlsli` (through `/I` or the include handler): `esia_common.hlsli` then includes
  it first (`build_shaders.py --fxc` does this). It must `#define uint float` (X3548: the shared sources keep flags
  and indices in `uint`, SM3 has no integers) and defines, for SM3, the macros whose SM4 defaults are in
  `esia_common.hlsli`:
  `ESIA_BINDING(n)` / `ESIA_LOCATION(n)` (empty), `ESIA_CBUFFER(name, reg, binding)` (e.g. a plain `cbuffer name`,
  then read the constant table from the bytecode for the `c` registers), `ESIA_TEXTURE(name, reg, binding)`
  (`sampler2D name : register(s<slot>)`), `ESIA_TEXTURE_ARG` (`sampler2D`), `ESIA_SAMPLE(tex, smp, uv)`
  (`tex2D`), `ESIA_SAMPLE_LEVEL(tex, smp, uv)` (`tex2Dlod(tex, float4(uv, 0, 0))`), `ESIA_LOAD(tex, texel)` (a
  `tex2Dlod` at the texel center, with the texture size in a constant of the backend's own), `ESIA_SAMPLER(...)`
  (empty), `ESIA_HAS(bits, flag)` (float arithmetic, e.g. `fmod(floor((bits) / (flag)), 2) >= 1`; the FX shader
  tests feature and flag bits only through `FX_HAS` / `ESIA_HAS`),
  `ESIA_VERTEX_ID(name)` / `ESIA_INSTANCE_ID(name)` (`float name : TEXCOORDn` from two vertex streams, instancing
  with `SetStreamSourceFreq`), `ESIA_FLAT` (empty), `ESIA_FLAT_UINT(v)` (round the interpolated index) and
  `ESIA_CLIP_POSITION(p)` (the half-pixel shift below; every vertex shader writes `SV_Position` through it).
  With `d3dcompiler_47` every program compiles for `vs_3_0` / `ps_3_0` (checked under Wine); the risk is the FX
  pixel shader's size: a plain fill is ~450 instruction slots, glass ~2k, the full shader ~3.8k, beyond the 512
  that SM3 guarantees (`D3DCAPS9::MaxPixelShader30InstructionSlots` says what the device takes; NVIDIA's driver has
  4096, and the D3D9 test fails when the full shader stops fitting in it). SM3 counts code, so the backdrop
  sampling is written to be compiled once: the B-spline taps of a pyramid level are computed before the branch that
  picks the level's texture, and the two levels a blur blends are read in a loop (it was ~5.5k: the level
  selection was inlined ten times). Registers are the other limit (32 temporaries): the dispersion's three reads
  as a loop around that one did not compile for the full shader. `fxFeatureVariants` lets batches compile only
  their features; the full shader, the fallback of refused and pending variants, may not be creatable on a device
  limited to 512 slots. A shader over the device's slots (a large user effect) is refused by `CreatePixelShader`;
  the backend compares the disassembly's slot count (`d3d::InstructionSlots`) with the device's and logs that as
  information, not as an error.
  Render targets cannot be locked, so uploads to them and readback go through a level of a `D3DPOOL_SYSTEMMEM`
  texture, which every texture format has; offscreen plain surfaces do not (NVIDIA's driver has no L8 one, the glyph
  atlas's format, though it takes L8 render targets). Keep those textures for later uploads and write one again only
  a few frames after its copy: the FX instance texture (a copy destination, so a render target) is uploaded every
  frame, and a new texture each time cost 25 us an upload on the RTX 4080, a kept one 3 us.

## 5. API recipes

### OpenGL 3.3 core / OpenGL ES 3.0 (`esia-opengl`)

* Load functions yourself (glad-style loader in the backend, or the host's `getProcAddress`); no GLEW / GLFW
  dependency in the backend. Headless: EGL surfaceless (`EGL_MESA_platform_surfaceless`, config
  `EGL_PBUFFER_BIT`, context 3.3 core or ES 3.0) - this is how the tests run on Mesa llvmpipe (`EGL_PLATFORM` not
  needed; `libegl-dev libgles-dev` are installed in the cloud image). Windows GPU drivers ship no EGL: there the
  headless context comes from WGL (a hidden window lends the pixel format; GLES through
  `WGL_EXT_create_context_es2_profile`), with ANGLE's EGL as the fallback - what `esia-opengl` does.
* Textures: `GL_RGBA8`, `GL_R8`, `GL_RGBA16F`, `GL_RGBA32F`, `GL_SRGB8_ALPHA8`; `glPixelStorei(GL_UNPACK_ALIGNMENT, 1)`.
  Render targets get an FBO; a copy destination needs one too (blits write FBOs).
* Passes = `glBindFramebuffer` + `glViewport`; `LoadOp::Clear` = `glClear` with the scissor test off;
  `DontCare` = nothing (or `glInvalidateFramebuffer`). Toggle `GL_FRAMEBUFFER_SRGB` per target format (desktop GL).
* `CopyTexture` = `glBlitFramebuffer` with rectangles converted to bottom-left (`y = h - y1`), `GL_NEAREST`, the
  scissor test off; it resolves multisampled renderbuffers. A blit is a raw copy only between formats of the same
  encoding: on GLES, and by the spec on desktop GL before 4.4, a blit linearizes an sRGB read buffer, so the
  backdrop copy of an sRGB target (sRGB -> its raw format) must be a draw - a full-screen triangle that reads the
  source without decoding (`EXT_texture_sRGB_decode` on the sampler) or re-encodes what it decoded (bit-exact for
  all 256 levels); see `esia-opengl`'s STATUS. GLES resolves a multisampled source only into the identical rectangle
  and format: resolve into a temporary of the source's format and size first when the destination position or
  format differs.
* Constants: a UBO per slot, orphaned by each update (`glBufferData`). Not `glBufferSubData` into a shared ring:
  NVIDIA's driver waits for the draws reading a buffer before `glBufferSubData` writes any of it, a GPU drain per
  draw. Two VAOs (UI layout, empty for id-only programs).
* Draws: `glDrawElements(GL_TRIANGLES, n, GL_UNSIGNED_INT, first * 4)`, `glDrawArraysInstanced(GL_TRIANGLE_STRIP,
  0, 4, n)`, `glDrawArrays(GL_TRIANGLES, 0, 3)`. Blend with `glBlendFuncSeparate`.
* `ReadPixels`: `glReadPixels` of the rows `h - y1 .. h - y0`, flipped to top-first; multisampled targets blit to a
  temporary texture first.
* Timestamps: `GL_TIMESTAMP` with `glQueryCounter` (GLES: `EXT_disjoint_timer_query`), read a few frames later.
* A throwaway GL 3.3 implementation of exactly this rendered all 14 conformance scenes correctly on llvmpipe while
  the core was written (not committed: it is this branch's job); the pitfalls it hit are the ones listed here (EGL
  config needs `EGL_PBUFFER_BIT`; link `libEGL` + `libGL`, there is no `libOpenGL` in the image).

### Vulkan (`esia-vulkan`)

* Headless: an instance + device without surfaces; Mesa's lavapipe (`apt-get install mesa-vulkan-drivers`) runs the
  suite on the CPU.
* Pass = render pass (or dynamic rendering, 1.3) on the target; `Load` / `Clear` / `DontCare` map to load ops.
  Track every image's layout. Since a texture is never sampled in the pass that renders it, leave sampleable
  targets in `SHADER_READ_ONLY_OPTIMAL` at the end of their pass (final layout) and transition at `BeginPass`
  (to `COLOR_ATTACHMENT_OPTIMAL`), at `CopyTexture` (`TRANSFER_SRC` / `TRANSFER_DST`, then back to
  `SHADER_READ_ONLY`), after uploads, and at `EndFrame` (the host target back to its exit layout).
* Uploads go into a staging ring recorded before the first render pass (all updates happen before it).
* Constants: dynamic uniform-buffer offsets into a per-frame ring (32 bytes of Draw constants could be push
  constants, but the SPIR-V declares `WgtDraw` as a uniform buffer: keep the three as UBOs). Descriptor sets from a
  per-frame pool, or `VK_KHR_push_descriptor`.
* Pipelines from `PipelineDesc` (cache them; creation is slow): the UI vertex layout, strip / list, blend (dual
  source needs `dualSrcBlend`), the render pass / rendering formats of the target, the sample count.
* Clip space: report `clipSpaceYDown = true`, or use a negative viewport height (1.1 / `VK_KHR_maintenance1`) and
  report false. `gl_FragCoord` is top-left either way.
* `CopyTexture`: `vkCmdCopyImage` (formats of the same size class, sRGB <-> UNORM allowed) or `vkCmdResolveImage` for
  a multisampled source. `ReadPixels`: copy to a host-visible buffer, wait for the queue.
* Timestamps: `vkCmdWriteTimestamp` in a query pool per frame in flight, `timestampPeriod`.

### Metal (`esia-metal`)

* Pass = `MTLRenderCommandEncoder` from the frame's `MTLCommandBuffer` (`FrameDesc::nativeContext`); copies need a
  `MTLBlitCommandEncoder` between passes; end the current encoder before either.
* No partial clears: the renderer draws the `Clear` program. `LoadOp` maps to `MTLLoadAction`.
* Constants with `setVertexBytes` / `setFragmentBytes` (index 0 / 1 / 2, under 4 KB), textures 3..9, the FX buffer
  at index 10, samplers 11 / 12, the vertex buffer at an unused index (30) with a vertex descriptor for attributes
  0 / 1 / 2.
* sRGB: a raw read of an `*_sRGB` target needs `MTLTextureUsagePixelFormatView` and a UNORM view; `copyFromTexture`
  wants identical pixel formats, so alias the backdrop copy through a view when the formats differ in sRGB-ness.
* Storage: `private` for render targets, `shared` (Apple silicon) / `managed` for uploads; `ReadPixels` blits into a
  shared buffer and waits.
* Timestamps: Apple GPUs sample counters only at encoder boundaries, so time is measured per encoder: categories that
  share a render pass are split approximately (by their draw counts, as `esia-metal` does), `totalMs` is exact; the
  `rhi::GpuProfile` comment says so to tools.
* Build and test on macOS only (`macos-clang` preset); the Linux session can write the code but not compile it.

### Direct3D 11 (`esia-directx`)

WGT's `src/backends/d3d11_backend.cpp` is the model: state backup / restore around `Render` (host state), dynamic
buffers with `MAP_WRITE_DISCARD`, a structured buffer SRV for FX instances, typeless storage for the backdrop copy of
sRGB targets, `ResolveSubresource` for MSAA (whole subresources: a resolve to an offset goes through a temporary),
the target's SRV for the direct read (only for non-sRGB or typeless targets), `ID3D11DeviceContext1::ClearView` is
not needed (the Clear program), timestamp queries with `D3D11_ASYNC_GETDATA_DONOTFLUSH` a few frames later. Runtime
effects via `D3DCompile` on a worker thread.

### Direct3D 12 (`esia-directx`)

WGT's `src/backends/d3d12_backend.cpp` is the model: records into the host's command list
(`FrameDesc::nativeContext`), root signature (3 CBVs / root constants, one SRV table, two static samplers), a
shader-visible descriptor heap ring, upload ring per frame in flight, PSO cache keyed by `PipelineDesc`, barriers:
target `RENDER_TARGET <-> COPY_SOURCE / PIXEL_SHADER_RESOURCE`, pyramid levels and the layer `RENDER_TARGET <->
PIXEL_SHADER_RESOURCE`, copy destination `COPY_DEST <-> PIXEL_SHADER_RESOURCE`; timestamps resolved into a readback
buffer per frame. SM5.0 DXBC works in D3D12; DXIL is optional.

### Direct3D 10 (`esia-directx`)

As D3D11 with SM4: FX instances from an RGBA32F texture (`fxStorage = Texture`, `dxbc_sm4` / `vs_4_0` with
`ESIA_FX_STORAGE_TEXTURE`), no `ClearView`, no `D3D11_QUERY_TIMESTAMP` differences worth noting (D3D10 has timestamp
queries).

### Direct3D 9 (`esia-directx`)

* Direct3D 9Ex; `D3DPOOL_DEFAULT` render-target textures for the pyramid, the layer and the backdrop copy
  (`StretchRect` only writes render targets); `A16B16G16R16F` when
  `CheckDeviceFormat(D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING)` allows, else `floatRenderTargets = false`.
* FX instances: an `A32B32G32R32F` texture read in both stages, `fxStorage = Texture`. Vertex texture fetch has four
  samplers (`D3DVERTEXTEXTURESAMPLER0..3` = `s0..s3` of a `vs_3_0`); no stage define is needed: the prelude leaves
  every register to the compiler, and the backend binds samplers and constants by name from each shader's constant
  table (CTAB). `gFxData` is the only texture read with `ESIA_LOAD`, so its size is the one constant that macro
  needs. `maxFxDataWidth` must be 24 x a power of two: SM3 finds an instance's row and column with float `floor` /
  `fmod`, exact only when the instances per row are a power of two.
* Ids: a static vertex buffer of corner ids (0..3) with `D3DSTREAMSOURCE_INDEXEDDATA | n` and a static instance-id
  buffer with `D3DSTREAMSOURCE_INSTANCEDATA | 1` (+ an index buffer for the quad); the full-screen triangle from a
  static 3-vertex buffer of ids.
* Half-pixel offset: report `halfPixelOffset` (the renderer then sets `gConv.y = 0.5`, so `WgtPixelPos` returns pixel
  centers from `VPOS`) and define `ESIA_CLIP_POSITION(p)` in the prelude as
  `((p) + float4(gEsiaHalfPixel.xy * (p).w, 0, 0))`, with `gEsiaHalfPixel` a `c` register of the backend's own that
  it sets at `BeginPass` to `(-1 / width, +1 / height)` of the pass target (pyramid levels are smaller than the
  target). `esia-directx`'s prelude does exactly this.
* Constants: `SetVertexShaderConstantF` / `SetPixelShaderConstantF` at the registers of the bytecode's constant table.
* sRGB: `D3DRS_SRGBWRITEENABLE` for sRGB targets, `D3DSAMP_SRGBTEXTURE` always off.
* `runtimeEffects = true`; `fxFeatureVariants` only on a device whose `MaxPixelShader30InstructionSlots` cannot take
  the full FX shader (below 4096): NVIDIA's D3D9 driver compiles a shader again at its first draw, every run, so
  twenty variants stalled the showcase's first frames for 0.85 s on an RTX 4080 where the full shader runs as fast.
* Variants compile with `D3DCompile` in the first frames that use them: up to 2.6 s each on a real driver (masks
  0x822 / 0x823 / 0x826 on an RTX 4080). Report `asyncPipelines` and compile them on a worker thread (section 2,
  "Background pipelines"), with the full shader built first.
* A driver that compiles at the first draw would stall the frame a user effect first shows in (140 ms on the RTX
  4080): report `firstDrawCompiles`, and the renderer draws a ready effect pipeline once where no pixel is written,
  in the first frames.

## 6. LLVM toolchain

Presets (`CMakePresets.json`), toolchains in `cmake/toolchains`:

| Preset | Host | Compiler / linker | Notes |
| --- | --- | --- | --- |
| `linux-clang` / `linux-clang-release` | Linux | clang++ + lld | tests run; `-DESIA_LLVM_SUFFIX=-18` for versioned binaries |
| `macos-clang` | macOS | Homebrew LLVM (`brew install llvm lld`) or Apple clang + lld; SDK from `xcrun` | Metal |
| `windows-clang-cl` | Windows | clang-cl + lld-link (LLVM installer on PATH, a "x64 Native Tools" prompt for INCLUDE / LIB) | the release toolchain |
| `windows-cross` | Linux | clang-cl + lld-link + MSVC CRT / Windows SDK from `xwin` | MSVC ABI; tests off |
| `windows-mingw-cross` | Linux | clang + ld.lld + mingw-w64 | compile / link check when xwin cannot download |
| `windows-msvc` | Windows | MSVC (Visual Studio 2022 generator) | Debug and Release from one build directory |

```bash
# Linux (tests)
cmake --preset linux-clang && cmake --build --preset linux-clang && ctest --preset linux-clang
cmake --preset linux-clang -DESIA_BACKEND_VULKAN=OFF       # the platform's backends are on by default; drop or add one

# Windows from Linux with the Microsoft SDK (xwin; needs download.visualstudio.microsoft.com)
cargo install xwin --locked
xwin --accept-license --arch x86_64 splat --output ~/.xwin   # crt/ and sdk/ with lower-case symlinks
cmake --preset windows-cross -DXWIN_DIR=$HOME/.xwin       # Direct3D 9 - 12 and OpenGL by default (a Windows target)
cmake --build --preset windows-cross

# Windows from Linux without network access to Microsoft (mingw-w64: D3D9-12 headers and import libraries)
sudo apt-get install g++-mingw-w64-x86-64-posix mingw-w64-x86-64-dev
cmake --preset windows-mingw-cross
cmake --build --preset windows-mingw-cross
wine build/windows-mingw-cross/bin/esia_core_tests.exe      # optional: apt-get install wine64
```

What was verified in the core session: `linux-clang` builds warning-free and CTest passes; `windows-mingw-cross`
builds every library and test executable and they all pass under Wine 9.0 (including the null conformance goldens);
`windows-cross` could not be tried (xwin's download host is blocked in the cloud sessions); `macos-clang` and
`windows-clang-cl` could not be tried (no such machines). Write the DirectX backends so they build with both cross
presets: prefer standard C++ and the Windows SDK / mingw-w64 common subset of the D3D headers (`d3d9.h`, `d3d10.h`,
`d3d11.h`, `d3d12.h`, `dxgi1_6.h`, `d3dcompiler.h`), and `Microsoft::WRL::ComPtr` only if mingw-w64's `wrl` works for
you (a tiny intrusive `ComPtr` in `d3d_common` is safer).

With `windows-mingw-cross` (clang + mingw-w64's libstdc++ 13), anything that instantiates
`std::type_info::operator==` fails to link with a duplicate symbol: `std::make_shared` (use
`std::shared_ptr<T>(new T(...))`), `std::regex`, `typeid(a) == typeid(b)`, `std::function::target`. The native
Windows presets do not have the problem.

Round 2 added a local Windows run (Windows 11, NVIDIA RTX 4080 SUPER): `windows-clang-cl` with clang-cl 22.1.8 +
lld-link, and MSVC 19.44 (Visual Studio 2022), both with `ESIA_WERROR=ON`; clang-cl needed
`-Wno-missing-field-initializers` (its `/W4` is `-Wall -Wextra`: the graphics APIs' partial initializers), which
`esia_target_defaults` now passes.

## 7. Conformance suite

```bash
build/linux-clang/bin/esia_conformance --list                                   # backends built in, scenes
build/linux-clang/bin/esia_conformance --backend opengl --golden tests/conformance/golden --strict --out /tmp/out
build/linux-clang/bin/esia_conformance --backend opengl --golden tests/conformance/golden --strict --frames 3
ctest --preset linux-clang -R esia_conformance                                   # two tests per backend
```

* Each scene (`tests/conformance/scenes.cpp`) is rendered into a headless target of its size, format, sample count
  and sampleability (`HeadlessDescOf(scene)`), with its render parameters (`RenderParamsOf(scene)`), read back
  (`ReadPixels`) and compared with `tests/conformance/golden/<scene>.png`: a pixel differs when a channel is more
  than `tolerance.channel` away; the scene passes when at most `tolerance.fraction` of the pixels differ, none by
  more than `maxDelta` (48) and the mean channel difference is at most `meanDelta` (0.5). The fraction alone lets a
  missing hairline through (few pixels, far off: `maxDelta`) and a wrong blend or blur (most pixels, a little:
  `meanDelta`); NVIDIA's renderings on every API stay at max 25 and mean 0.09 against the llvmpipe goldens. `--out`
  writes the image and, on failure, `<scene>.diff.png` (differing pixels red over the dimmed golden).
* A scene also fails when its device reports validation messages after the readback (`Device::ValidationErrors`),
  when its host callbacks did not all run (`callback_capture`), or, for scenes with `checkCopy` (`srgb_target`,
  `msaa_target`, `glass_copy`, `srgb_msaa`), when half the target copied to another position of a second texture
  (`CopyTexture` at an offset, resolving multisampled targets) differs from the target.
* `--frames N`: N - 1 frames of loud content (`BuildPoisonFrame`: glass and a glow layer over the whole target, more
  FX instances than any scene) on the same device and renderer, then a cleared target and the scene's frame. A
  surface the frame reads but did not write - undefined after `DontCare`, stale from an earlier frame - then shows,
  where a fresh device reads zeros (llvmpipe does). The null backend's goldens are single frames: it ignores it.
* Scenes a backend cannot run are SKIPped with the reason (the headless device refused the format / samples, no
  device on this machine); no scene depends on an optional capability any more (`text_lcd` went with sub-pixel
  text). A missing golden is NO GOLDEN, a failure with
  `--strict`. Exit code: 1 on a failure, 77 when every scene was skipped, else 0. CMake adds, per backend,
  `esia_conformance_<name>` (`--strict`) and `esia_conformance_<name>_frames` (`--frames 3 --strict`; not for null),
  both with `SKIP_RETURN_CODE 77`: a machine without the backend shows them as skipped, never as passed.
* **Goldens.** The image goldens are in `tests/conformance/golden` (from `esia-opengl`, Mesa llvmpipe, reviewed);
  every backend compares against them. Scenes that must render like another one share its golden: `msaa_target`,
  `glass_copy`, `callback_capture` use `glass.png`, `srgb_msaa` uses `srgb_target.png`, `fx_rows` uses `shapes.png`.
  After an intended rendering change, regenerate them with OpenGL on llvmpipe: `esia_conformance --backend opengl
  --golden tests/conformance/golden --update` (only scenes that own their golden are written), after reviewing the
  diff images of a run before the update. The null backend's goldens (`golden/null/*.log`, command streams) are
  regenerated with `--backend null --update` after a renderer change and reviewed as text.
* Backend tests that render the scenes themselves (to check their API's validation, both render paths, their own
  traces against `golden/null`) link `esia_conformance_scenes` instead of compiling `scenes.cpp`, and use
  `HeadlessDescOf` / `RenderParamsOf` so that they render what the harness renders.

## 8. Checklist

1. `ESIA_BACKEND_<NAME>=ON` builds the target warning-free with clang (`ESIA_WERROR=ON` passes).
2. `EsiaRegisterBackend_<name>()` registers a headless creator; `esia_conformance --list` shows the backend.
3. Every conformance scene passes `--strict`, and with `--frames 3`, on the machine that can run the backend
   (llvmpipe / lavapipe here, the real OS for D3D and Metal) - or is SKIPped for a stated capability reason.
   `ValidationErrors()` reports the API's validation (debug layers, validation layer, `KHR_debug`) and is 0 on
   every scene.
4. The null-device rules hold (run your own traces against `golden/null/*.log` when in doubt).
5. The host integration header documents device creation, target wrapping and `FrameDesc::nativeContext`.
6. GPU times appear in `RenderStats::gpu` where the API has timestamps.
7. Nothing outside `src/esia/rhi/<name>/` (and `d3d_common`) changed.
