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
| `esia-directx` | `src/esia/rhi/d3d_common/` (shared: DXGI format tables, `D3DCompile` glue with the include handler, capture / resolve helpers, the SM3 prelude if shared), `src/esia/rhi/d3d9/`, `src/esia/rhi/d3d10/`, `src/esia/rhi/d3d11/`, `src/esia/rhi/d3d12/` | `ESIA_BACKEND_D3D9`, `ESIA_BACKEND_D3D10`, `ESIA_BACKEND_D3D11`, `ESIA_BACKEND_D3D12` |

The options already exist (`src/esia/CMakeLists.txt`, all `OFF` by default). Turning one on adds
`src/esia/rhi/<name>/CMakeLists.txt`; any `ESIA_BACKEND_D3D*` also adds `src/esia/rhi/d3d_common/CMakeLists.txt`
when it exists (before the four). Inside a backend directory:

```
src/esia/rhi/<name>/
  CMakeLists.txt
  include/esia/rhi/<name>.hpp     host integration: create the device from the host's API objects, wrap targets
  <name>_device.cpp ...           the rhi::Device implementation (private headers next to it)
  tests/                          optional unit tests of the backend (esia_add_test)
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
`error`: the conformance suite reports SKIP instead of failing. It also registers one CTest per backend name:
`esia_conformance_<name>`.

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
usage (`RenderTarget | CopySrc`, `Sampled` only if it can be sampled raw - see "Color" below); `DestroyTexture`
releases the wrapper, never the host's object. Swap-chain buffers change every frame: make wrapping cheap (cache by
the native pointer).

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
| `CreatePipeline(desc)` | Program (vertex + pixel shader of `ShaderProgram`), vertex layout, topology, blend, target format, samples, and for `Fx`: `effect` / `effectSource` (user effects, only with `runtimeEffects`) and `fxFeatures` (only with `fxFeatureVariants`). May be called inside a pass. Return `{}` if the combination is impossible (dual-source without support, an effect that failed): the renderer falls back or skips. A user effect may return `{}` while it compiles in the background; the renderer asks again on a later frame. |

### Frames, passes, state

| Call | Contract |
| --- | --- |
| `BeginFrame(desc)` / `EndFrame()` | One `Renderer::Render` = one device frame (a host may render several targets per host frame: count device frames for rings). `nativeContext` is the backend's recording object, if it records into the host's. Return false to skip the frame. |
| `BeginPass({target, load})` / `EndPass()` | `Load` keeps the contents, `Clear` clears to `clearColor`, `DontCare` means nothing outside what the pass draws is ever read (pyramid levels, fresh glow layers: tilers need not load them). Viewport = whole target. **Nothing is bound at the start of a pass**; scissor = whole target. |
| `SetPipeline(p)` | Its target format and samples match the pass target. |
| `SetScissor(rect)` | Top-left pixels, inside the target, may be empty only if no draw follows. Bottom-left APIs convert: `y = height - rect.y1`. |
| `SetConstants(slot, data, size)` | Frame (192 bytes), Pass (32), Draw (32); multiples of 16. Inline: each draw sees the latest values; version them (ring buffer, push / root constants, `setVertexBytes`). |
| `SetTexture(slot, tex)` | t0..t7. Never the current pass target. Bind with the sampler the shader expects (section 4). |
| `SetFxBuffer(buf)` | t7 as a structured / storage buffer (`FxStorage::Buffer`). |
| `SetVertexBuffer` / `SetIndexBuffer` | The frame's merged buffers; indices are 32-bit and already rebased (no base vertex). |
| `Draw(3, 0)` | Full-screen triangle, no vertex buffer: `SV_VertexID` 0..2 (Downsample, LayerComposite, Clear). |
| `DrawIndexed(count, first)` | Triangle list of `esia::Vertex` (UiGeometry, TextGray, TextLcd, TextLcdGray). |
| `DrawInstanced(4, n)` | Triangle strip, no vertex buffer, `SV_InstanceID` 0..n-1; the draw's first instance is in the Draw constants. |
| `CopyTexture(dst, x, y, src, rect)` | Outside passes; raw bits (`dst` is `src`'s format or `RawFormat(src)`); resolves a multisampled source. |
| `NativeRenderState()` | Inside a pass: the object host callbacks record with (table in section 1). Forget every binding afterwards. |
| `BeginProfile(category)` / `EndProfile()` / `ReadProfile(out)` | Non-nesting scopes, inside or outside passes. Resolve timestamps a few frames later without stalling; `ReadProfile` returns the latest complete frame (`GpuProfile::totalMs`, per-category ms). With `timestampQueries = false` these are no-ops. |
| `ReadPixels(tex, rect, rgba8)` | Tests and screenshots: waits for the GPU; RGBA8, top row first, whatever the format (float targets converted, sRGB returned as stored, multisampled resolved). |

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
| `fxStorage` | `Buffer`: FX instances in a buffer (`SetFxBuffer`); `Texture`: RGBA32F texture at t7, `perRow = maxFxDataWidth / 24` instances per row (`gConv.z`) | Texture | Texture | Buffer | Buffer | Texture | Texture | Buffer | Buffer |
| `shaderFormat` | informational: the `shaders::Format` the backend loads | DxbcSm3 | DxbcSm4 | DxbcSm5 | DxbcSm5 / Dxil | Glsl330 | Essl300 | SpirV | Msl |
| `framebufferOriginBottomLeft` | shaders flip pixel positions and render-target uv | no | no | no | no | yes | yes | no | no |
| `clipSpaceYDown` | the renderer flips its projection (`gXform.y > 0`; FullscreenVS follows) | no | no | no | no | no | no | yes (or a negative viewport height and no) | no |
| `halfPixelOffset` | renderer sets `gConv.y = 0.5`; the SM3 prelude shifts positions by half a pixel | yes | no | no | no | no | no | no | no |
| `dualSourceBlend` | sub-pixel text with per-channel alpha; else grayscale coverage from the page's alpha | no | yes | yes | yes | yes | `EXT_blend_func_extended` | `dualSrcBlend` feature | yes |
| `floatRenderTargets` | RGBA16F pyramid and layers (else RGBA8) | if RGBA16F blending | yes | yes | yes | yes | `EXT_color_buffer_half_float` | yes | yes |
| `sampleRenderTarget` | targets reported `Sampled` may be read right after their pass (the no-copy capture path) | yes | yes | yes | yes | yes | yes | yes | yes |
| `timestampQueries` | GPU time per category | yes | yes | yes | yes | yes | `EXT_disjoint_timer_query` | yes | not at first |
| `readback` | `ReadPixels` works: required by the conformance suite | yes | yes | yes | yes | yes | yes | yes | yes |
| `runtimeEffects` | user HLSL effects compiled at runtime | yes | yes | yes | yes | no | no | no | no |
| `fxFeatureVariants` | one FX pipeline per batch feature mask (`PipelineDesc::fxFeatures`) | yes | optional | optional | optional | no | no | no | no |
| `maxTextureSize`, `maxFxDataWidth` | limits (the FX texture is `perRow * 24` wide) | device caps | 8192 | 16384 | 16384 | `GL_MAX_TEXTURE_SIZE` | same | limits | 16384 |

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
| `TextLcd` | `UiVS` / `TextLcdPS` (two outputs) | `esia::Vertex` | list | DualSourceLcd | Frame, Draw, t0 (linear, `.rgb`) |
| `TextLcdGray` | `UiVS` / `TextLcdGrayPS` | `esia::Vertex` | list | Straight | Frame, Draw, t0 (linear, `.a`) |
| `Fx` | `FxVS` / `FxPS` (`esia_fx.hlsl`) | none (`SV_VertexID`, `SV_InstanceID`) | strip | Premultiplied | Frame, Draw, t0..t6 (linear), t7 FX data |
| `Downsample` | `FullscreenVS` / `DownsamplePS` (`esia_post.hlsl`) | none (`SV_VertexID`) | list | Opaque | Frame, Pass, t0 (linear) |
| `LayerComposite` | `FullscreenVS` / `LayerCompositePS` | none | list | Premultiplied | Frame, Pass, t0 (point), t1..t6 (linear) |
| `Clear` | `FullscreenVS` / `ClearPS` | none | list | Opaque | Frame |

Blend modes: `Opaque` = replace; `Straight` = rgb `src*srcA + dst*(1-srcA)`, alpha `src + dst*(1-srcA)`;
`Premultiplied` = rgb and alpha `src + dst*(1-srcA)`; `DualSourceLcd` = rgb `src0*src1 + dst*(1-src1)`, alpha
`src1.a + dst*(1-src1.a)`. No depth, no stencil, no culling. Samplers: s0 linear / clamp / no mips, s1 point /
clamp / no mips.

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
* TextLcd on GLES needs `GL_EXT_blend_func_extended` (the ESSL has the `#extension` line): create that pipeline only
  when the extension exists, and report `dualSourceBlend` accordingly.
* The ESSL is `highp` throughout (the build script enforces it).
* `esia_glsl_link_tests` compiles and links every program on the machine's driver; the GL backend's first test.

### 4.3 SPIR-V (Vulkan) and MSL (Metal)

* SPIR-V 1.0 (`--target-env vulkan1.0`); the entry point keeps the HLSL name, `ShaderBlob::entry` (`FxVS`,
  `FxPS` ...): pass it as `VkPipelineShaderStageCreateInfo::pName`.
  Separate images and samplers; one descriptor set layout for every program: bindings 0-2 `UNIFORM_BUFFER` (or
  `_DYNAMIC`), 3-9 `SAMPLED_IMAGE`, 10 `STORAGE_BUFFER`, 11-12 `SAMPLER`, all `VERTEX | FRAGMENT`. Only what a
  program uses must be valid when it draws (what the null device checks). Dual-source outputs are location 0 index 0
  / 1. Validated with `spirv-val` at build time.
* MSL 2.0, entry point `esia_main` in every source (one `MTLLibrary` per blob, `newLibraryWithSource`), indices as in
  the table. The FX data is a `device` buffer; constants may come through `setVertexBytes` / `setFragmentBytes` at
  0 / 1 / 2. MSL cannot be compiled on Linux: validate it on macOS (`xcrun -sdk macosx metal -c file.metal`).

### 4.4 Direct3D (DXBC / DXIL, runtime compilation, SM3)

* `shaders::Available(Format::DxbcSm5)` etc. is false until someone ran `python tools/shaders/build_shaders.py
  --fxc fxc` (and `--dxc dxc`) on Windows and committed the output. Until then, and for user effects and FX
  variants always, compile at device creation with `D3DCompile` (`d3dcompiler_47.dll`, part of Windows 10 / 11;
  mingw-w64 has the import library): source `shaders::FindSource(SourceOf(p).file)`, an `ID3DInclude` that serves
  `shaders::FindSource(name)`, entry points from `SourceOf`, profiles `vs_5_0` / `ps_5_0` (D3D11 / 12), `vs_4_0` /
  `ps_4_0` with `ESIA_FX_STORAGE_TEXTURE=1` (D3D10), `vs_3_0` / `ps_3_0` with `ESIA_FX_STORAGE_TEXTURE=1` and the
  SM3 prelude (D3D9). Cache the bytecode (the FX pixel shader takes about a second to compile).
* Registers: `cbuffer` b0-b2, textures t0-t7, samplers s0-s1; D3D12's root signature: 3 root CBVs (or root
  constants for Draw), one SRV table t0-t7, two static samplers.
* User effects: compile `esia_fx.hlsl` with `ESIA_CUSTOM_EFFECT=1`, the include handler returning the user's
  source for `esia_user_effect.hlsli` (it must define `float4 WgtEffect(WgtFx fx)`).
* **The SM3 prelude** (D3D9): `src/esia/rhi/d3d9/esia_sm3_prelude.hlsli`, passed as
  `ESIA_SHADER_PRELUDE="esia_sm3_prelude.hlsli"` (with `/I` or an include handler). It defines, for SM3, the
  macros whose SM4 defaults are in `esia_common.hlsli`:
  `ESIA_BINDING(n)` / `ESIA_LOCATION(n)` (empty), `ESIA_CBUFFER(name, reg, binding)` (e.g. a plain `cbuffer name`,
  then read the constant table from the bytecode for the `c` registers), `ESIA_TEXTURE(name, reg, binding)`
  (`sampler2D name : register(s<slot>)`), `ESIA_TEXTURE_ARG` (`sampler2D`), `ESIA_SAMPLE(tex, smp, uv)`
  (`tex2D`), `ESIA_SAMPLE_LEVEL(tex, smp, uv)` (`tex2Dlod(tex, float4(uv, 0, 0))`), `ESIA_LOAD(tex, texel)` (a
  `tex2Dlod` at the texel center, with the texture size in a constant of the backend's own), `ESIA_SAMPLER(...)`
  (empty), `ESIA_HAS(bits, flag)` (float arithmetic, e.g. `fmod(floor((bits) / (flag)), 2) >= 1`),
  `ESIA_VERTEX_ID(name)` / `ESIA_INSTANCE_ID(name)` (`float name : TEXCOORDn` from two vertex streams, instancing
  with `SetStreamSourceFreq`), `ESIA_FLAT` (empty), `ESIA_FLAT_UINT(v)` (round the interpolated index) and
  `ESIA_CLIP_POSITION(p)` (the half-pixel shift below; every vertex shader writes `SV_Position` through it).
  Check with fxc on Windows; `static const` arrays indexed dynamically (`kRatios` in `WgtGammaRatios`,
  `esia_ui.hlsl`) and the instruction count of the full FX shader are the known risks: `fxFeatureVariants` lets
  batches compile only their features.

## 5. API recipes

### OpenGL 3.3 core / OpenGL ES 3.0 (`esia-opengl`)

* Load functions yourself (glad-style loader in the backend, or the host's `getProcAddress`); no GLEW / GLFW
  dependency in the backend. Headless: EGL surfaceless (`EGL_MESA_platform_surfaceless`, config
  `EGL_PBUFFER_BIT`, context 3.3 core or ES 3.0) - this is how the tests run on Mesa llvmpipe (`EGL_PLATFORM` not
  needed; `libegl-dev libgles-dev` are installed in the cloud image).
* Textures: `GL_RGBA8`, `GL_R8`, `GL_RGBA16F`, `GL_RGBA32F`, `GL_SRGB8_ALPHA8`; `glPixelStorei(GL_UNPACK_ALIGNMENT, 1)`.
  Render targets get an FBO; a copy destination needs one too (blits write FBOs).
* Passes = `glBindFramebuffer` + `glViewport`; `LoadOp::Clear` = `glClear` with the scissor test off;
  `DontCare` = nothing (or `glInvalidateFramebuffer`). Toggle `GL_FRAMEBUFFER_SRGB` per target format (desktop GL).
* `CopyTexture` = `glBlitFramebuffer` with rectangles converted to bottom-left (`y = h - y1`), `GL_NEAREST`, the
  scissor test and `GL_FRAMEBUFFER_SRGB` off; it resolves multisampled renderbuffers.
* Constants: a UBO ring with `glBindBufferRange` (respect `GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT`); two VAOs (UI
  layout, empty for id-only programs).
* Draws: `glDrawElements(GL_TRIANGLES, n, GL_UNSIGNED_INT, first * 4)`, `glDrawArraysInstanced(GL_TRIANGLE_STRIP,
  0, 4, n)`, `glDrawArrays(GL_TRIANGLES, 0, 3)`. Blend with `glBlendFuncSeparate` (`GL_SRC1_COLOR` family for LCD).
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
* Constants with `setVertexBytes` / `setFragmentBytes` (index 0 / 1 / 2, under 4 KB), the FX buffer at index 10,
  textures 3..10, samplers 11 / 12, the vertex buffer at an unused index (30) with a vertex descriptor for attributes
  0 / 1 / 2.
* sRGB: a raw read of an `*_sRGB` target needs `MTLTextureUsagePixelFormatView` and a UNORM view; `copyFromTexture`
  wants identical pixel formats, so alias the backdrop copy through a view when the formats differ in sRGB-ness.
* Storage: `private` for render targets, `shared` (Apple silicon) / `managed` for uploads; `ReadPixels` blits into a
  shared buffer and waits.
* Timestamps: counter sample buffers are limited to encoder boundaries on Apple GPUs; start with
  `timestampQueries = false`.
* Build and test on macOS only (`macos-clang` preset); the Linux session can write the code but not compile it.

### Direct3D 11 (`esia-directx`)

WGT's `src/backends/d3d11_backend.cpp` is the model: state backup / restore around `Render` (host state), dynamic
buffers with `MAP_WRITE_DISCARD`, a structured buffer SRV for FX instances, typeless storage for the backdrop copy of
sRGB targets, `ResolveSubresource` for MSAA, the target's SRV for the direct read (only for non-sRGB or typeless
targets), `ID3D11DeviceContext1::ClearView` is not needed (the Clear program), timestamp queries with
`D3D11_ASYNC_GETDATA_DONOTFLUSH` a few frames later. Runtime effects via `D3DCompile` on a worker thread.

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
  samplers (`D3DVERTEXTEXTURESAMPLER0..3` = `s0..s3` of a `vs_3_0`): compile the vertex shaders with a define of
  the backend's own (e.g. `ESIA_SM3_VS`) that makes the prelude put `gFxData` at `s0` there. `gFxData` is the only
  texture read with `ESIA_LOAD`, so its size is the one constant that macro needs.
* Ids: a static vertex buffer of corner ids (0..3) with `D3DSTREAMSOURCE_INDEXEDDATA | n` and a static instance-id
  buffer with `D3DSTREAMSOURCE_INSTANCEDATA | 1` (+ an index buffer for the quad); the full-screen triangle from a
  static 3-vertex buffer of ids.
* Half-pixel offset: report `halfPixelOffset` (the renderer then sets `gConv.y = 0.5`, so `WgtPixelPos` returns pixel
  centers from `VPOS`) and define `ESIA_CLIP_POSITION(p)` in the prelude as
  `((p) + float4(gEsiaHalfPixel.xy * (p).w, 0, 0))`, with `gEsiaHalfPixel` a `c` register of the backend's own that
  it sets at `BeginPass` to `(-1 / width, +1 / height)` of the pass target (pyramid levels are smaller than the
  target). Checked with glslang that the three vertex shaders compile with such an override; not with fxc.
* Constants: `SetVertexShaderConstantF` / `SetPixelShaderConstantF` at the registers of the bytecode's constant table.
* sRGB: `D3DRS_SRGBWRITEENABLE` for sRGB targets, `D3DSAMP_SRGBTEXTURE` always off.
* No dual-source blending (`dualSourceBlend = false`), `fxFeatureVariants = true`, `runtimeEffects = true`.

## 6. LLVM toolchain

Presets (`CMakePresets.json`), toolchains in `cmake/toolchains`:

| Preset | Host | Compiler / linker | Notes |
| --- | --- | --- | --- |
| `linux-clang` / `linux-clang-release` | Linux | clang++ + lld | tests run; `-DESIA_LLVM_SUFFIX=-18` for versioned binaries |
| `macos-clang` | macOS | Homebrew LLVM (`brew install llvm lld`) or Apple clang + lld; SDK from `xcrun` | Metal |
| `windows-clang-cl` | Windows | clang-cl + lld-link (LLVM installer on PATH, a "x64 Native Tools" prompt for INCLUDE / LIB) | the release toolchain |
| `windows-cross` | Linux | clang-cl + lld-link + MSVC CRT / Windows SDK from `xwin` | MSVC ABI; tests off |
| `windows-mingw-cross` | Linux | clang + ld.lld + mingw-w64 | compile / link check when xwin cannot download |
| `vs2022` | Windows | MSVC | today's `wgt.dll`, unchanged |

```bash
# Linux (tests)
cmake --preset linux-clang && cmake --build --preset linux-clang && ctest --preset linux-clang
cmake --preset linux-clang -DESIA_BACKEND_OPENGL=ON        # a backend: its option on any preset

# Windows from Linux with the Microsoft SDK (xwin; needs download.visualstudio.microsoft.com)
cargo install xwin --locked
xwin --accept-license --arch x86_64 splat --output ~/.xwin   # crt/ and sdk/ with lower-case symlinks
cmake --preset windows-cross -DXWIN_DIR=$HOME/.xwin -DESIA_BACKEND_D3D11=ON
cmake --build --preset windows-cross

# Windows from Linux without network access to Microsoft (mingw-w64: D3D9-12 headers and import libraries)
sudo apt-get install g++-mingw-w64-x86-64-posix mingw-w64-x86-64-dev
cmake --preset windows-mingw-cross -DESIA_BACKEND_D3D11=ON
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

## 7. Conformance suite

```bash
build/linux-clang/bin/esia_conformance --list                                   # backends built in, scenes
build/linux-clang/bin/esia_conformance --backend opengl --golden tests/conformance/golden --out /tmp/out
ctest --preset linux-clang -R esia_conformance                                   # one test per backend
```

* Each scene (`tests/conformance/scenes.cpp`) is rendered into a headless target of its size, format and sample
  count (`HeadlessDesc`), read back (`ReadPixels`) and compared with `tests/conformance/golden/<scene>.png`: a pixel
  differs when a channel is more than `tolerance.channel` away; the scene passes when at most `tolerance.fraction`
  of the pixels differ and none by more than `maxDelta`. `--out` writes the image and, on failure,
  `<scene>.diff.png` (differing pixels red over the dimmed golden).
* Scenes a backend cannot run are SKIPped (the headless device refused the format / samples, or no dual-source
  blending for `text_lcd`). A missing golden is reported as NO GOLDEN (a failure only with `--strict`).
* **Goldens.** The image goldens are produced by the first drawing backend that runs in CI, OpenGL on Mesa
  llvmpipe: `esia_conformance --backend opengl --golden tests/conformance/golden --update`, look at every image,
  commit them on the OpenGL branch. To regenerate after an intended change: the same command, then review the diff
  images from a run before the update. `msaa_target` shares `glass.png`; everything else owns its golden. The null
  backend's goldens (`golden/null/*.log`, command streams) are regenerated with `--backend null --update` after a
  renderer change, and reviewed as text.
* Backends developed in parallel with OpenGL compare against its goldens once they are merged; until then run with
  `--out` and look at the images (and compare them with the OpenGL session's when both exist).

## 8. Checklist

1. `ESIA_BACKEND_<NAME>=ON` builds the target warning-free with clang (`ESIA_WERROR=ON` passes).
2. `EsiaRegisterBackend_<name>()` registers a headless creator; `esia_conformance --list` shows the backend.
3. Every conformance scene passes on the machine that can run the backend (llvmpipe / lavapipe here, the real OS
   for D3D and Metal) - or is SKIPped for a stated capability reason.
4. The null-device rules hold (run your own traces against `golden/null/*.log` when in doubt).
5. The host integration header documents device creation, target wrapping and `FrameDesc::nativeContext`.
6. GPU times appear in `RenderStats::gpu` where the API has timestamps.
7. Nothing outside `src/esia/rhi/<name>/` (and `d3d_common`) changed - except the image goldens on the OpenGL
   branch.
