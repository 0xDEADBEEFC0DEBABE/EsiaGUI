# Esia: rewriting WGT without Dear ImGui

> **WGT is gone from this tree.** Esia replaced it: WGT's sources (`src/render`, `src/backends`, `src/ui`,
> `src/text`, `src/shaders`, `src/core`, `include/wgt`, the demo, the modified Dear ImGui) are in tag
> `wgt-1.1-final`, in `main`'s history (the branches `archive/wgt` and `reference/wgt-1.1`, with screenshots of its
> demo, were deleted on 2026-10-02). Where this document cites a WGT file, read it there. With WGT gone, the `wgt::` compatibility layer of phase 5 is dropped:
> Esia's widgets get their own API.

WGT ("WGT UI", `wgt.dll`) renders everything you see itself - the SDF Painter, the FX pipeline with liquid glass,
the DirectWrite text engine, the widgets - but it still stands on a modified Dear ImGui 1.92 for ids, input,
windows, draw lists, clipping and text editing, and on two hand-written Direct3D backends. **Esia** is the rewrite:
the same toolkit on its own base, with its own render class, running on Direct3D 9 / 10 / 11 / 12, OpenGL 3.3+ /
OpenGL ES 3, Vulkan and Metal. The owner's goal, translated:

> Leave Dear ImGui completely: rewrite everything on our own base, with our own render class, supporting OpenGL,
> Metal, Vulkan, Direct3D 9 / 10 / 11 / 12 and other real-world APIs. It must stay a differentiated, modern toolkit:
> no ImGui limitations (e.g. cumbersome styling), per-component styling, liquid glass, thread-safe services, C++20,
> extensible.

This document is the plan: the layers and their boundaries, how every FX feature maps onto every API, the shader
strategy, how the public API migrates, and the phases. What exists today is summarized in
[REWRITE_STATUS.md](REWRITE_STATUS.md); how to write a backend is in [backends/README.md](backends/README.md).

* [1. Principles](#1-principles)
* [2. Layers and modules](#2-layers-and-modules)
* [3. The frame, end to end](#3-the-frame-end-to-end)
* [4. The UI core (replacing Dear ImGui)](#4-the-ui-core-replacing-dear-imgui)
* [5. The renderer](#5-the-renderer)
* [6. The render hardware interface](#6-the-render-hardware-interface-rhi)
* [7. FX features on every API](#7-fx-features-on-every-api)
* [8. Shader strategy](#8-shader-strategy)
* [9. Text](#9-text)
* [10. Platform layer](#10-platform-layer)
* [11. Threading](#11-threading)
* [12. Public API and migration](#12-public-api-and-migration)
* [13. Toolchain](#13-toolchain)
* [14. Testing](#14-testing)
* [15. Phases](#15-phases)
* [16. Risks and open questions](#16-risks-and-open-questions)

---

## 1. Principles

* **Keep what is proven.** The Painter's instance encoding (`fx::Instance`, 24 float4), the SDF / material /
  glass math of `wgt_fx.hlsl`, the frame planner's batching and capture planning, the backdrop pyramid and the
  glow layers are WGT's and they work: Esia ports them byte for byte and changes only what ImGui or Direct3D
  forced on them (transport, bindings, coordinate conventions).
* **One implementation of every technique.** The capture logic lives once, in the renderer, above a small render
  hardware interface (RHI). A backend translates RHI calls; it never decides what to capture, batch or blend.
* **No platform in the core.** `esia_core`, `esia_render` and the text interface include no OS or graphics API
  header. Windows, input, IME, clipboard and DPI come through a platform layer; GPUs through the RHI.
* **Tested without a GPU.** The null backend records and validates every RHI call, so the core, the renderer and
  the conformance scenes are tested on any machine; drawing backends are tested with the same scenes against
  golden images.
* **C++20, static libraries, no exceptions required, no global state** (several contexts per process, one per
  thread). Warning-free with clang (`-Wall -Wextra -Wpedantic -Wshadow`).

## 2. Layers and modules

```
 app / game / tool      widgets, custom widgets, plugins
        |                      |
        v                      v
 +-------------+   +------------------------------------------+   +-------------------+
 | platform    |-->| esia_ui (phase 3): widgets, layout,       |   | services          |
 | Win32 Cocoa |   | ItemStyle, Theme, anim, text editing       |   | Post Notify panels|
 | X11/Wayland |   +------------------------------------------+   | Property Channel  |
 | SDL, engine |          |                     |                  | plugins, pacing   |
 +-------------+          v                     v                  +-------------------+
   InputEvents   +-------------------+   +------------------+
   Requests      | esia_core         |   | esia_text        |
                 | ids, input,       |<--| interface + FT / |
                 | windows, items,   |   | DWrite / CoreText|
                 | layout cursor,    |   +------------------+
                 | draw lists,       |          | glyph pages (TextureRegistry)
                 | texture registry  |          v
                 +-------------------+   DrawList geometry
                          | DrawData
                          v
                 +-------------------------------------------+
                 | esia_render: Painter, FramePlan, Renderer |
                 +-------------------------------------------+
                          | rhi::Device calls only
                          v
                 +-------------------------------------------+       +------------------+
                 | esia_rhi: the interface, caps, null device|<------| esia_shaders     |
                 +-------------------------------------------+       | one HLSL source, |
                   |      |      |      |      |      |      |       | every format     |
                 d3d9  d3d10  d3d11  d3d12  opengl vulkan  metal     +------------------+
```

| Module (CMake target) | Directory | Depends on | Contents | State |
| --- | --- | --- | --- | --- |
| `esia_core` | `include/esia/{base,core}`, `src/esia/core` | STL | ids and hashing, input state, context (windows, z-order, focus, move / resize, items, hover / active / focus, layout cursor, groups, clipping, scroll), draw lists + FX stream, texture registry | done |
| `esia_render` | `include/esia/render`, `src/esia/render` | core, rhi | `Painter` (+ `Style`, `Paint`, `GlassMaterial`, `PainterEnv`), `FramePlan`, `Renderer` | done |
| `esia_rhi` | `include/esia/rhi`, `src/esia/rhi` | - | `rhi::Device` interface, caps, formats, backend registry, null device | done |
| `esia_shaders` | `src/esia/shaders` | rhi | HLSL sources + generated SPIR-V / GLSL / ESSL / MSL (+ DXBC / DXIL once built on Windows), lookup table | done |
| `esia_rhi_<api>` | `src/esia/rhi/<api>` | rhi, shaders | one backend each; the DirectX branch shares `src/esia/rhi/directx/common` | backend sessions |
| `esia_text`, `esia_text_ft` | `include/esia/text`, `src/esia/text` | core; FreeType + HarfBuzz for `esia_text_ft` | `text::TextSystem` interface, WGT's analytic glyph rasterizer, the glyph atlas; the FreeType + HarfBuzz text system | done (no right-to-left paragraphs yet) |
| `esia_ui` | `include/esia/ui`, `src/esia/ui` | core, render, text | port of `src/ui` (controls, lists, windows, navigation, overlay, selection, text edit, auto layout), `Theme`, `ItemStyle`, `anim` | phase 3 |
| `esia_platform_<os>` | `src/esia/platform/<os>` | core | window, input / IME translation, clipboard, DPI, cursor, frame pacing | phase 4 |
| `wgt` (compat) | `src/compat` | everything | the `wgt::` API on Esia, so existing hosts recompile | phase 5 |

Rules at the boundaries:

* The core never draws: widgets draw with Painter into `Window::GetDrawList()`. The core's only output is
  `DrawData` (draw lists back to front) and `PlatformRequests` (cursor, IME rect, text input, capture flags).
* The renderer sees only `DrawData`, `TextureRegistry` changes and an `rhi::Device`. It does not know windows,
  widgets or fonts.
* Textures are created through the `TextureRegistry` (any thread) and become GPU textures when the renderer
  applies the pending changes at the start of a frame. Nothing above the renderer touches the GPU.
* Backends implement `rhi::Device` plus a small backend-specific header to wrap the host's objects (device,
  render target, command list). They never see draw lists.

## 3. The frame, end to end

1. **Platform** (window thread or UI thread): native events become `InputEvent`s and are queued with
   `Context::QueueInput` (thread-safe).
2. **UI thread** `Context::NewFrame(FrameParams)`: applies the queued input (a button that changes twice waits for
   the next frame, so fast clicks are never lost), window move / resize, the hit test of last frame's items, press
   ownership, popup dismissal, wheel scrolling; starts the implicit root window.
3. **Widgets** register items (`ItemAdd` / `ItemSize` / `ButtonBehavior`), keep springs, and draw with `Painter`
   into the current window's `DrawList`: geometry (text, lines, images) and FX instances in one ordered command
   list with clip rects, glow-layer and edge-fade brackets, host callbacks.
4. `Context::EndFrame()`: unclaimed keys (Tab navigation, Escape), click-to-focus, window dragging, the `DrawData` (background list, windows by layer and
   z-order, foreground list) and `PlatformRequests`.
5. **Renderer** `Render(DrawData, TextureRegistry*, target, params)`:
   1. `Device::BeginFrame`; pending texture creates / updates / destroys go to the device;
   2. `FramePlan::Build`: merged vertex / index / instance arrays, batches, capture plan (section 5);
   3. one upload of vertices, indices and FX instances; frame constants;
   4. the ops run in passes: geometry and text, instanced FX batches, backdrop captures (end the pass, copy or read
      directly, pyramid passes), glow layers, edge fades, host callbacks;
   5. `Device::EndFrame`; GPU times per category arrive a few frames later.
6. The platform applies the requests (cursor shape, IME candidate window at the caret, whether the game gets the
   mouse / keyboard) and presents.

## 4. The UI core (replacing Dear ImGui)

`esia::Context` (`include/esia/core/context.hpp`) is the immediate-mode machinery WGT took from ImGui, rewritten
around what WGT uses. Its second version (the widget port's base) is designed in [UI_CORE.md](UI_CORE.md); in
short:

| ImGui today | Esia |
| --- | --- |
| `ImGuiID`, id stack, `###` / `##` labels | `esia::Id` (32-bit FNV-1a chained through the window's id stack), `HashLabel`, same label conventions |
| `ImGuiIO` input, `AddMouseButtonEvent` ... | `InputEvent` queue (thread-safe) -> `InputState`: clicks and click counts, drag thresholds, key repeat, text (UTF-32), IME composition, focus loss cancels presses and invalidates the mouse |
| `ImGui::Begin` / `End`, z-order, focus, move / resize, `WindowRounding` ... | `Context::Begin` / `End` with `WindowOptions` (flags, layer, min / max size, padding): z-order per layer (background, normal, overlay, tooltip), focus and bring-to-front, drag-to-move on empty areas, resize from edges and corners with cursors, per-window content clip and scroll, per-window DPI scale, auto-size, kept on screen, popups and tooltips |
| `ItemSize`, `ItemAdd`, `ItemHoverable`, `ButtonBehavior` | the same four, with hover from a front-to-back hit test of last frame's item rects, press ownership per mouse button, disabled items that claim the hover, active id with keep-alive, press on click / release / double click, repeat; `ItemStatusOf` |
| `SameLine`, `Indent`, `BeginGroup` / `EndGroup`, `GetContentRegionAvail` | containers with the layout cursor (`SameLine`, `NewLine`, `Spacing`, `Indent`) or a `LayoutProvider`, laid-out item reports, baselines, work rects, pixel snapping; child regions with clip and (smooth) scroll |
| keyboard nav (partial) | keyboard focus: `SetKeyboardFocusId`, Tab / Shift+Tab over focusable items after the widgets, click-away clears it; key ownership (`ClaimKey`) |
| `ImDrawList` + WGT's FX callbacks | `esia::DrawList`: 32-bit indices (no base vertex needed), geometry + FX instances + layers + fades + callbacks, clip and texture stacks, command merging, `Mark` / `MoveCommands` for backgrounds drawn after their content |
| `ImTextureData` protocol | `TextureRegistry`: create / update / destroy from any thread, collected by the renderer once per frame |
| `ImGuiPlatformIO` | `PlatformRequests` (cursor, capture flags, text input, IME rect) + clipboard callbacks in `ContextDesc` |

What ImGui did that Esia deliberately does not take over: ImGui's style (`ImGuiStyle` - Esia's styling is the
`Theme` + `ItemStyle` of the widget layer), ImGui's widgets (every WGT widget is already WGT's own), `.ini`
persistence (a service in phase 4), docking and multi-viewports (not needed by WGT).

Child regions are regions of their window's draw list (not nested windows), popups are `WindowLayer::Overlay`
windows, and the per-window item map WGT's auto layout keeps is `Window::LaidOutItems()`. The text editor stays
the widget layer's (WGT's `ui/text_edit.cpp`, ported onto `InputState::Text` / `Composition`, `MouseClickCount`,
`ClaimKeyboard` and `RequestTextInput`).

## 5. The renderer

`esia::Painter` (`include/esia/render/painter.hpp`) is `wgt::Painter` on `esia::DrawList`: the same shapes,
`Style`, `Paint`, masks, scale, glow layers, edge fades, light streak, polylines and areas, emitting the same
`fx::Instance` bytes. What changed: it is constructed on a draw list with a `PainterEnv` (metrics scale, corner
smoothing, pixel density, alpha, `text::TextSystem*`, `GlowContainment*`) instead of reading the global
context; textures are `TextureId`; text is `std::string_view`; rounded `FillRect` is an SDF instance.

`render::FramePlan` is WGT's planner on `DrawData`, unchanged in its decisions:

* batches of FX instances with the same clip, texture and effect, split when a glass shape would have to see what
  the batch drew under it;
* dirty tracking as a list of rectangles, content vs. glass tracked apart (neighbouring glass shares a backdrop,
  glass sees other glass only where it sits on it), the visible *core* of shadows and glows (> 4 %);
* look-ahead merging: one capture serves the following glass batches that nothing draws under, while the merged
  region stays mostly useful (area <= 1.6 x the parts);
* per capture: the region, the pyramid levels the largest blur needs, and whether anything reads level 0;
* the merged vertex / index / instance arrays (indices rebased; backends never need base-vertex draws).

`render::Renderer` executes the plan through `rhi::Device` (`src/esia/render/renderer.cpp`), with the techniques of
the current Direct3D backends:

* **region-limited captures**: the capture region (plus the B-spline margin, aligned to 32 px) is copied, and only
  the pyramid levels the batch reads are rebuilt inside it; the 13-tap downsample clamps its taps to the refreshed
  region, so stale texels never bleed in;
* **the direct render-target read**: when no glass of the capture reads the full-resolution level (frosted
  glass) and the target can be sampled, the first downsample reads the target itself - no copy;
* **B-spline frost sampling** across two pyramid levels, bilinear single taps for the rim reflection and the
  legibility exposure (in the shader, unchanged);
* **glow layers**: an RGBA16F layer cleared only in its region, a bloom pyramid, content + bloom composited with a
  soft shoulder;
* **GPU profile categories**: runs of capture / layer / FX / FX-with-glass / geometry are timed without nesting
  (`rhi::ProfileCategory`, `RenderStats::gpu`);
* passes start lazily (a capture or a layer ends the current pass, and nothing is loaded / stored again until
  something draws: an empty pass costs a full-target load and store on tile-based GPUs);
* FX instances go to the GPU as float4 rows, in a structured / storage buffer or an RGBA32F texture
  (`Caps::fxStorage`); the batch's first instance is a per-draw constant (no API needs base-instance support).
  The vertex shader hands the rows every pixel reads to the pixel shader as flat varyings; the others are fetched
  only by the branches of the features that use them (section 16).

## 6. The render hardware interface (RHI)

`include/esia/rhi/rhi.hpp` is what the renderer needs, not a general graphics API. Its contract:

* **Resources**: textures (sampled, render target, copy source / destination, multisampled), buffers (vertices,
  32-bit indices, FX instances), pipelines (one of eight shader programs + vertex layout + topology + blend mode +
  target format and samples + optional user effect / FX feature mask). Handles are plain ids; creation and updates
  happen before the frame's first pass.
* **Frames and passes**: `BeginFrame(nativeContext)` .. `EndFrame`; `BeginPass(target, load op)` .. `EndPass`. The
  viewport is the whole target. **Every pass starts with nothing bound**: the renderer binds pipeline, textures,
  buffers and constants again after each `BeginPass` (Metal encoders and Vulkan render passes carry no state).
* **Fixed binding model**: constants Frame (b0) / Pass (b1) / Draw (b2); textures t0 (main), t1..t6 (backdrop
  pyramid levels 0..5), t7 (FX instance texture); FX instance buffer at t7; samplers s0 linear clamp, s1 point
  clamp. The same slots in every shader and every API (Vulkan bindings and Metal indices in section 8).
* **Draws**: indexed geometry (`DrawIndexed`), instanced FX quads (`DrawInstanced(4, n)`, triangle strip, no vertex
  buffer), full-screen passes (`Draw(3)`, no vertex buffer).
* **Copies**: `CopyTexture(dst, x, y, src, rect)` outside passes; resolves a multisampled source.
* **Coordinates**: render-target pixels, top-left origin, y down, for scissors, copies and readback on every API;
  a bottom-left API converts and sets `Caps::framebufferOriginBottomLeft` so the shaders flip what they read.
* **Color**: the shaders work on the stored, gamma-encoded values and encode their output for `*_SRGB` targets
  (as WGT does). Sampling never decodes sRGB: a render target that is sRGB is sampled only through a raw view, and
  `CopyTexture` copies bits - the backdrop copy of an sRGB target is its `RawFormat`.
* **Capabilities** (`rhi::Caps`) instead of API versions: FX instance storage, bottom-left origin, y-down clip
  space, D3D9's half-pixel offset, float render targets, sampling render targets, timestamps, readback, runtime
  effects, FX feature variants (and building them in the background), texture and instance-texture limits.
* **Host callbacks**: `NativeRenderState()` returns the object host code records into (device context, command
  list / buffer, encoder, device); the backend forgets its bindings, the renderer binds again.
* **Validation**: the null device (`include/esia/rhi/null_device.hpp`) checks every rule above - draws outside
  passes or without what the program's shaders declare, updates after the first pass, copies inside a pass or
  between incompatible formats, sampling the bound target, scissors outside the target, pipelines for the wrong
  format or sample count - and records the command stream as text.

Everything a backend implements, call by call, is in [backends/README.md](backends/README.md).

## 7. FX features on every API

What the renderer asks for, and how each API provides it. "Caps" names the `rhi::Caps` value the backend reports.

### 7.1 Feature map

| Feature | Direct3D 9 (SM3) | Direct3D 10 (SM4) | Direct3D 11 | Direct3D 12 | OpenGL 3.3 / 4.x | OpenGL ES 3.0 | Vulkan | Metal |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Instanced SDF quads | `SetStreamSourceFreq` instancing (SM3 hardware, indexed draw of a 4-vertex quad); vertex / instance ids from static streams | `DrawInstanced`, `SV_InstanceID` | `DrawInstanced` | `DrawInstanced` | `glDrawArraysInstanced`, `gl_InstanceID` | same as GL 3.3 | `vkCmdDraw(4, n)` | `drawPrimitives(.triangleStrip, 0, 4, n)` |
| FX instance data | RGBA32F texture, vertex texture fetch (`tex2Dlod`) - `Caps::fxStorage = Texture` | RGBA32F texture (no structured buffers in SM4) | `StructuredBuffer<float4>` | structured buffer (SRV in the table / root) | GL 3.3: RGBA32F texture + `texelFetch`; GL 4.3+ could use an SSBO variant | RGBA32F texture (non-filterable, `texelFetch`) | storage buffer, binding 10 | device buffer, index 10 |
| First instance of a draw | Draw constants (`gDrawInfo.x`) on every API: no base-instance support is needed (GL 3.3, GLES 3, D3D9 have none that reaches the shader) | | | | | | | |
| Per-draw constants (edge fade, first instance) | `Set*ShaderConstantF` (c registers from the constant table) | CB update | dynamic CB (`MAP_WRITE_DISCARD`) or D3D11.1 `*SetConstantBuffers1` offsets | root constants or a CBV ring | UBO ring (`glBindBufferRange`, `GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT`) | same | dynamic uniform-buffer offsets into a per-frame ring (the SPIR-V declares `WgtDraw` as a uniform buffer) | `setVertexBytes` / `setFragmentBytes` |
| Backdrop copy | `StretchRect` into a render-target texture (also resolves MSAA) | `CopySubresourceRegion` / `ResolveSubresource` | same (typeless storage for raw sRGB) | `CopyTextureRegion` / `ResolveSubresourceRegion` + barriers | `glBlitFramebuffer` (region; resolves MSAA), `GL_FRAMEBUFFER_SRGB` off while copying | `glBlitFramebuffer` | `vkCmdCopyImage` / `vkCmdResolveImage` + layout transitions | blit encoder `copyFromTexture` (same pixel format: alias the copy through a texture view for sRGB) |
| Direct read of the target (frost, no copy) | if the host target is a texture | SRV on the target (typeless if sRGB) | SRV on the target (typeless if sRGB) | SRV + `RENDER_TARGET -> PIXEL_SHADER_RESOURCE` barrier | the host FBO's color texture (not a renderbuffer / window); sRGB raw via `EXT_texture_sRGB_decode` or a texture view (4.3) | color texture; sRGB raw via `EXT_texture_sRGB_decode` | image view with `SAMPLED` usage (+ `MUTABLE_FORMAT` UNORM view for sRGB), layout transition | host texture with `ShaderRead` usage (+ `PixelFormatView` for sRGB) |
| Blur pyramid (levels 1..5, 13-tap downsample) | render-target textures, `A16B16G16R16F` if `D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING`, else A8R8G8B8 | RGBA16F RTs | RGBA16F RTs | RGBA16F RTs | RGBA16F FBOs | RGBA16F only with `EXT_color_buffer_half_float` - else RGBA8 (`Caps::floatRenderTargets = false`) | RGBA16F attachments | `rgba16Float` |
| Glow layer + region clear | Clear program draw (scissored) | same | same (D3D11.1 `ClearView` would also do) | same | same | same | same (or `vkCmdClearAttachments`) | Clear program (Metal has no partial clear) |
| Blend: straight / premultiplied / opaque | render states | blend states | blend states | PSO blend | `glBlendFuncSeparate` | same | pipeline blend | pipeline blend |
| Scissor | `D3DRS_SCISSORTESTENABLE` + `SetScissorRect` | RS scissor | RS scissor | `RSSetScissorRects` | `glScissor` (y flipped: bottom-left) | same | dynamic scissor | `setScissorRect` (within the attachment) |
| sRGB targets | `D3DRS_SRGBWRITEENABLE`; sampling with `D3DSAMP_SRGBTEXTURE` off is raw | sRGB RTV, typeless resource for raw views | same | same | `GL_FRAMEBUFFER_SRGB` enabled for sRGB attachments | sRGB attachments always encode | `*_SRGB` formats | `*_sRGB` pixel formats |
| Multisampled host targets | `StretchRect` resolve | resolve | resolve | resolve | blit resolve | blit resolve | `vkCmdResolveImage` | resolve texture / `copyFromTexture` after a resolve pass |
| Pixel position in shaders | VPOS = integer centers: `halfPixelOffset` (renderer sets `gConv.y = 0.5`; the SM3 prelude shifts positions by half a pixel of the current target in `ESIA_CLIP_POSITION`) | `SV_Position` | `SV_Position` | `SV_Position` | `gl_FragCoord`, bottom-left: `framebufferOriginBottomLeft` (unless `glClipControl(GL_UPPER_LEFT)` on 4.5) | bottom-left | top-left; clip +y down: `clipSpaceYDown` or a negative viewport height | top-left, clip +y up |
| GPU timestamps | `D3DQUERYTYPE_TIMESTAMP` (+ `TIMESTAMPDISJOINT`, `TIMESTAMPFREQ`) | timestamp queries | same | timestamp query heap + resolve | `GL_TIMESTAMP` queries (core 3.3) | `EXT_disjoint_timer_query` (optional) | `vkCmdWriteTimestamp`, `timestampPeriod` | counter sample buffers (Apple GPUs: at encoder boundaries only) |
| User HLSL effects at runtime | `D3DCompile` (d3dcompiler_47) | `D3DCompile` | `D3DCompile` (WGT does this today) | `D3DCompile` (SM5.1) or DXC | not in phase 1 (would need glslang + SPIRV-Cross at runtime) | no | not in phase 1 | not in phase 1 |

### 7.2 Limits per API family

* **Direct3D 9 / SM3.** No integer operations (the FX shader tests feature bits: the SM3 prelude defines `ESIA_HAS`
  with float arithmetic), no `SV_VertexID` / `SV_InstanceID` (static id streams), no texture objects (DX9 samplers,
  `tex2Dlod`), no flat interpolation (the instance index is interpolated and rounded), constants in `c` registers
  (the backend reads the constant table from the bytecode), 512 pixel-shader instruction slots guaranteed (32768
  on current hardware). The whole FX shader may not fit a given profile: `Caps::fxFeatureVariants` lets the backend
  compile the FX shader per batch feature mask (`ESIA_FX_FEATURES`), so a batch of plain rounded rects never carries
  the glass code. Use Direct3D 9Ex (no lost `D3DPOOL_DEFAULT` resources). `StretchRect` only writes render-target
  surfaces, so the backdrop copy is a render-target texture.
* **Direct3D 10 / SM4.** Instancing, `SV_InstanceID` and typed resources exist; structured
  buffers do not, so FX instances come from a texture (`dxbc_sm4` is compiled with `ESIA_FX_STORAGE_TEXTURE`).
* **Direct3D 11 / 12.** Everything; D3D12 manages barriers (render target <-> copy source / pixel-shader resource,
  pyramid levels between render target and shader resource), descriptor heaps, an upload ring per frame in flight
  and deferred releases, as WGT's D3D12 backend does today.
* **OpenGL 3.3 vs 4.x.** 3.3 has what the renderer needs (UBOs, instancing, `texelFetch`, sampler objects, timer
  queries, `glBlitFramebuffer`). 4.x only makes it faster or simpler: SSBOs for
  instances (a GLSL 430 variant), `glClipControl` (4.5) for a top-left origin and no flips, persistent mapping
  (4.4), DSA (4.5), texture views (4.3) for raw sRGB. The bottom-left origin is handled once, in the shaders
  (`WgtPixelPos`, `WgtRtUv`) and in the backend's rectangle conversion.
* **OpenGL ES 3.0 / WebGL 2.** Like GL 3.3 minus: float render targets need `EXT_color_buffer_half_float` /
  `EXT_color_buffer_float` (else RGBA8 pyramids and layers, some banding), timestamps need
  `EXT_disjoint_timer_query`; `highp` everywhere (the generated ESSL declares it).
* **Vulkan.** A pass is a render pass (or dynamic rendering, 1.3); every texture has a tracked layout and the
  backend inserts the barriers the RHI implies (copy source / destination around `CopyTexture`, color attachment
  vs. shader read between a pyramid level's pass and the next level). Descriptor set 0 matches the binding model;
  pipelines are created from `PipelineDesc` (cache them, creation is slow); resources are versioned per frame in
  flight; the host's `VkCommandBuffer` comes in `FrameDesc::nativeContext`. Clip space is y-down: report
  `clipSpaceYDown` or use a negative viewport height. `dualSrcBlend` is a device feature.
* **Metal.** A pass is a `MTLRenderCommandEncoder`, copies need a blit encoder between passes, nothing carries over
  (the RHI's "every pass starts empty" rule is Metal's), no partial clears (the Clear program), buffers / textures /
  samplers by index (section 8), sRGB raw reads through `newTextureViewWithPixelFormat`, timestamps only at encoder
  boundaries on Apple GPUs. Metal code cannot be compiled on Linux; the MSL is generated and checked in.

## 8. Shader strategy

**Decision: one HLSL source; SPIR-V, GLSL 330, ESSL 300 and MSL generated on Linux with glslang + SPIRV-Cross and
checked in; DXBC (SM3 / SM4 / SM5) built with Microsoft's fxc on Windows and checked in, with runtime
`D3DCompile` of the embedded HLSL as the fallback until then; DXIL optional with DXC.**

```
src/esia/shaders/*.hlsl, esia_common.hlsli
  |-- glslangValidator (HLSL front end) -> SPIR-V (Vulkan 1.0; spirv-val)                -> generated/spirv
  |                                         |-- SPIRV-Cross -> GLSL 330 (validated by glslang) -> generated/glsl330
  |                                         |-- SPIRV-Cross -> ESSL 300 (validated by glslang) -> generated/essl300
  |                                         '-- SPIRV-Cross -> MSL 2.0 (entry point esia_main) -> generated/msl
  |-- fxc (Windows) -> DXBC SM5 (D3D11/12), SM4 (D3D10, texture storage), SM3 (D3D9, with its prelude)
  '-- DXC (optional) -> DXIL SM6 (D3D12)
tools/shaders/build_shaders.py writes generated/<format>/<Program>.<vs|ps>.<ext> + esia_shader_table.cpp
CMake embeds the files (cmake/EsiaEmbed.cmake) into esia_shaders: shaders::Find(format, program, stage)
```

Why this, and not the alternatives:

* **HLSL as the one source.** The math is WGT's HLSL (`wgt_fx.hlsl`, 700 lines of SDFs, materials and glass): the
  D3D backends compile it natively and it stays the reference; nothing is translated by hand.
* **glslang, not DXC, for SPIR-V.** glslang ships in every Linux distribution, Homebrew and the Vulkan SDK; its
  HLSL front end handles this code. DXC's SPIR-V is equally valid, but DXC is not packaged for Ubuntu and its
  GitHub releases are unreachable from the cloud sessions (egress policy), so it cannot be the required tool.
* **SPIRV-Cross for GLSL / ESSL / MSL.** One IR, one cross-compiler: the three text formats come out of the same
  SPIR-V the Vulkan backend runs, with combined samplers named predictably (`SPIRV_Cross_Combined<texture><sampler>`)
  and the varyings renamed `esia_v<location>` so GL 3.3's name-based linking works. Every GLSL / ESSL program is
  compiled and linked on Mesa in CTest (`esia_glsl_link_tests`).
* **Checked-in outputs.** No backend session needs a shader tool to build; a shader change is regenerated with one
  command and reviewed as a diff (`--check` in CI). The generated GLSL of the FX shader is large (~250 KB): it
  compiles once per device.
* **DXBC needs fxc.** fxc is Windows-only. The options for Linux were checked here: *fxc under Wine* needs
  Microsoft's `fxc.exe` / `d3dcompiler_47.dll` from a Windows SDK (not something xwin fetches, and the SDK download
  host is unreachable from the cloud sessions); *Wine's own `d3dcompiler_47`* (vkd3d-shader, as in Ubuntu 24.04's
  Wine 9.0) compiles `esia_ui.hlsl` and `esia_post.hlsl` but not `esia_fx.hlsl` (parser gaps, no `atan2`) - not
  usable; *vkd3d-shader's own compiler* on Ubuntu 24.04 is 1.2, without an HLSL front end; *DXC* makes DXIL only
  (D3D12), not DXBC. So: the Windows session runs `python tools/shaders/build_shaders.py --fxc fxc` (and `--dxc
  dxc`) and commits the DXBC; until the DXBC exists, the D3D backends compile `shaders::FindSource()` at device
  creation with `D3DCompile` from `d3dcompiler_47.dll` (part of Windows 10 / 11), which is exactly what WGT does
  for user effects today. D3D12 can use SM5.0 DXBC (no DXC needed).
* **Direct3D 9 without a second source.** Everything SM3 lacks goes through macros in `esia_common.hlsli`
  (`ESIA_CBUFFER`, `ESIA_TEXTURE`, `ESIA_SAMPLE[_LEVEL]`, `ESIA_LOAD`, `ESIA_SAMPLER`, `ESIA_HAS`, `ESIA_VERTEX_ID`,
  `ESIA_INSTANCE_ID`, `ESIA_FLAT`, `ESIA_FLAT_UINT`, `ESIA_CLIP_POSITION`) whose defaults are the SM4+ code; the
  D3D9 backend compiles the same files with `ESIA_SHADER_PRELUDE` pointing at its own prelude that defines them
  for SM3. The refactoring was verified to be a no-op: SPIR-V and MSL are byte-identical, and all conformance
  scenes render pixel-identically on llvmpipe before and after.
* **Variants.** `ESIA_FX_FEATURES` compiles the FX shader for a subset of the feature bits; with
  `Caps::fxFeatureVariants` the renderer asks for one pipeline per batch feature mask. D3D9 needs it to fit SM3;
  any backend that compiles at runtime can use it for speed.

Binding numbers, identical everywhere:

| Resource | HLSL register | SPIR-V (set 0) binding | Metal index | GLSL |
| --- | --- | --- | --- | --- |
| `WgtFrame` / `WgtPass` / `WgtDraw` | b0 / b1 / b2 | 0 / 1 / 2 | buffer 0 / 1 / 2 | uniform blocks by name -> binding points 0 / 1 / 2 |
| `gTex` | t0 | 3 | texture 3 | combined samplers: `esia_shader_table.cpp` gives each sampler uniform its RHI slot and sampler |
| `gBackdrop0..5` | t1..t6 | 4..9 | texture 4..9 | (same) |
| `gFxData` | t7 | 10 (storage buffer / sampled image) | buffer 10 (device) | (texture) |
| `gLinear` / `gPoint` | s0 / s1 | 11 / 12 | sampler 11 / 12 | (same) |
| UI vertex (pos, uv, color) | `POSITION`, `TEXCOORD0`, `COLOR0` | locations 0 / 1 / 2 | attributes 0 / 1 / 2 (stage_in; put the vertex buffer at an index not listed above) | locations 0 / 1 / 2 |

## 9. Text

`include/esia/text/text.hpp` is the interface Painter and widgets use: fonts (files, memory, fallback chain),
`NewFrame(RasterParams)` (pixel density), `Measure`, `Draw` (glyph quads into a `DrawList`, rasterized at the
scaled size under `PushScale`), `DrawGlyph` (icon fonts). A text system owns its glyph pages in the
`TextureRegistry`: Alpha8 coverage, which the renderer recognizes from the page's format and draws with its text
program (`TextGray`), so text needs no special commands; the gamma / contrast composition of WGT's grayscale text
shader (`TextComposition`, DirectWrite's model) applies to every backend.

**No sub-pixel text.** By the owner's decision, Esia antialiases text in grayscale everywhere: there is no LCD /
ClearType text - no per-stripe RGB coverage or stripe order (RGB / BGR, OLED layouts), no dual-source blending, no
ClearType level or contrast. It was removed in core round 3 (the RHI's `TextLcd` / `TextLcdGray` programs,
`BlendMode::DualSourceLcd` and `Caps::dualSourceBlend` with it). What stays is sub-pixel *positioning* - the
quarter-pixel pen phases of the rasterizer and the atlas - and the grayscale gamma / contrast composition.

Implementations:

* **FreeType + HarfBuzz** (`esia_text_ft`, `include/esia/text/freetype.hpp`; done): every platform, the only one
  on Linux / Android / consoles. HarfBuzz shapes in design units (kerning, ligatures, marks, complex scripts),
  scaled to the size in float - unhinted, linear layout at every pixel density; per-character fallback through
  the added fonts; greedy line breaking (spaces, hyphens, CJK with a small kinsoku set, character breaks for
  overlong words), ellipsis trimming, alignment; WGT's uniform line box. FreeType only reads the unhinted outlines;
  they are covered by WGT's analytic rasterizer, now platform-free in `include/esia/text/glyph_raster.hpp`
  (exact area coverage, 4 horizontal sub-pixel phases) and packed by `GlyphAtlas` into the
  `TextureRegistry`. FreeType 2.14.3 and HarfBuzz 14.5.0 are built from pinned sources where the system has
  neither (`ESIA_TEXT_DEPS=auto|bundled|system`, `cmake/EsiaTextDeps.cmake`), so the same text system runs on
  Windows too. Right-to-left words in left-to-right text are laid out right to left, and the white space and
  punctuation around them stay where they were typed (a small part of UAX #9: neutrals between two right-to-left
  characters are right to left, elsewhere left to right; digits are left to right). Not yet: the rest of the bidi
  algorithm (right-to-left paragraphs, whose runs would be reordered - fribidi / ICU). Color glyphs: bitmap strikes
  (sbix, CBDT; their PNGs decoded by Esia when FreeType has no libpng) are scaled, COLR fonts (version 0 layers and
  version 1 paints: gradients, transforms, composite modes - Segoe UI Emoji, Android's Noto Color Emoji) are painted
  from their outlines by `colr.cpp`.
* **System fonts** (`include/esia/text/system_fonts.hpp`, in `esia_text`; done): `FindSystemFont(family, weight,
  style)` locates an installed face (file path + face index) - DirectWrite's system font collection on Windows, Core
  Text descriptors on macOS, fontconfig or the standard font directories on Linux - and each platform has a default
  fallback chain covering Latin, Chinese (Simplified first, then Traditional), Japanese, Korean and symbols, loaded
  into any `TextSystem` with `AddFallbackFonts`. The lookup only finds files: the text system reads and rasterizes
  them. The chains end with the system's color emoji font (Segoe UI Emoji, Apple Color Emoji, Noto Color Emoji); on
  Linux and Android a Noto font per script fills in what the UI font lacks.
* **DirectWrite** (Windows): WGT's `src/text` (system font collection, per-character fallback, COLR emoji, the
  analytic rasterizer, the user's text parameters) moved behind the interface, without its ClearType-style
  sub-pixel filtering.
* **Core Text** (Apple): system fonts and fallback, the same rasterizer.

Every implementation feeds its outlines to the same rasterizer and atlas, so a glyph looks the same whichever
library read the font.

Editing (grapheme-cluster caret movement, selection, undo, IME composition drawn inline) is WGT's
`ui/text_edit.cpp`, ported onto the interface (`ui::TextField`, [UI_WIDGETS.md](UI_WIDGETS.md) section 4) with one
query added for it: `CaretStops(font, text)`, every grapheme boundary of a line with the x where the caret is drawn
(right-to-left runs and ligatures included). Its default implementation stops at every code point and measures the
text before it; the FreeType system takes the boundaries from HarfBuzz's clusters and splits a ligature between its
graphemes (UAX #29's rules for marks, ZWJ sequences, regional indicators, Hangul and conjuncts).

## 10. Platform layer

A platform layer turns native events into `InputEvent`s and applies `PlatformRequests`. It owns no UI state.

| Concern | Input to the core | Output from the core |
| --- | --- | --- |
| mouse, wheel, keys, modifiers, focus | `InputEvent::MouseMove / Button / Wheel / KeyEvent / FocusEvent` | `wantCaptureMouse / Keyboard`, cursor shape |
| touch | the first finger as the left button, marked `InputEvent::touch` (scroll areas may take its drag over) | - |
| text and IME | `TextEvent` (committed UTF-8), `Composition` (string + caret) | `wantTextInput`, `imeRect` (candidate window at the caret) |
| clipboard | - | `ContextDesc::getClipboard / setClipboard` |
| DPI, display size, render scale | `FrameParams::displaySize / framebufferScale` + the widget layer's metrics scale | - |

Implementations: Win32 first (WGT's `HandleWin32Message` logic, including window-thread / render-thread splits and
IME); X11 (`esia_platform_x11`: XIM text, the `CLIPBOARD` selection, `Xft.dpi`) and Android (`esia_platform_android`:
a NativeActivity's touches, keys and soft keyboard, the clipboard through JNI, dp as UI units) followed. Still to come:
Cocoa and UIKit as libraries (the examples' frames do their work today), Wayland (through SDL3 or directly), and an
"engine" adapter for hosts that already own a window and an input system. Hosts may also feed events themselves
(automated UI tests do: WGT's input injection maps to `QueueInput`).

## 11. Threading

* One UI thread per `Context`: `NewFrame` .. widgets .. `EndFrame`, and `Renderer::Render` on the same thread (as in
  WGT). `Context::QueueInput` and the `TextureRegistry` are thread-safe (a window thread and loader threads use
  them).
* WGT's thread-safe services carry over unchanged: `Property<T>`, `Channel<T>`, `Latest<T>` (`wgt/sync.hpp` is
  already ImGui-free), `Post(Task)`, notifications, panel registration, theme changes - queued and applied at the
  next `NewFrame`.
* A separate render thread is possible later without changing the renderer: `DrawData` holds pointers to the draw
  lists; a host that renders on another thread double-buffers the lists (swap after `EndFrame`).
* Backends: `rhi::Device` is used from one thread at a time (the renderer's). A backend with frames in flight
  defers releases until the GPU is done.

## 12. Public API and migration

The `wgt::` API keeps its shape; what changes is what ImGui and Direct3D put into it.

| Today (`include/wgt`) | Esia | Migration |
| --- | --- | --- |
| `wgt::Painter` (`ImDrawList*`, `ImTextureID`, `const char*` text) | `esia::Painter` (any `DrawList&`, `TextureId`, `std::string_view`, `PainterEnv`) | the compat `wgt::Painter` wraps the current window's list and the context's env: existing widget code compiles unchanged |
| `wgt::Style`, `Paint`, `GlassMaterial`, `fx::Instance` | identical fields (`TextureId` for images) | none |
| `wgt::ui::*` widgets, `ItemStyle` / `ui::Next()`, auto layout | same names and options in `esia::ui` (phase 3) | `ImGuiID` -> `esia::Id`; `InteractRect(ImGuiID, ...)` takes `Id` |
| `wgt::anim::*` keyed by `ImGuiID` | same functions keyed by `esia::Id` | recompile |
| `wgt::Theme`, `Palette`, `Metrics`, `Motion` | same | `ApplyThemeToImGuiStyle` removed (no stock ImGui widgets) |
| `wgt::Vec2 = ImVec2`, `Rect`, `Color` | `esia::Vec2` / `Rect` / `Color` (same members) | `ToVec4()` / `ToU32()` / `ImVec4` conversions removed; `ToRgba8()` |
| `Context::Create(ContextDesc{backend, hwnd, d3d11Device ...})` | `esia::Context` + a platform object + an `rhi::Device` from the backend (`esia::rhi::d3d11::CreateDevice(device, context)` ...) | the compat `wgt::Context::Create` builds the three from the old `ContextDesc` |
| `Context::HandleWin32Message` | the Win32 platform layer | compat forwards to it |
| `Context::Render(RenderTarget::D3D11(rtv))` | `Renderer::Render(drawData, textures, rhi::Texture)` with the backend's `WrapRenderTarget` | compat wraps the target per frame |
| `IRenderBackend`, `fx::ReadShape` on `ImDrawData` | implement `rhi::Device` (a custom engine RHI is a backend) | custom backends are rewritten against the RHI (much smaller: no batching, no capture logic) |
| `Context::GetImGuiContext`, `GetImGuiFont`, stock `ImGui::` widgets next to WGT's | removed | hosts that mix stock ImGui widgets keep WGT 1.x, or move those panels to Esia widgets |
| `RegisterEffect(name, hlsl)` | `Renderer::SetEffectSource`; runtime compilation on Direct3D first | same HLSL (`WgtEffect(WgtFx)`) |
| plugins (`WGT_DECLARE_PLUGIN`, ABI 8) | same mechanism, new ABI major | plugins recompile |

The version becomes 2.0 (ABI 9): source compatible through the compat layer for code that does not touch ImGui
directly, binary incompatible.

## 13. Toolchain

LLVM everywhere for the new code: clang / clang++ with lld on Linux and macOS, clang-cl with lld-link on Windows
(`CMakePresets.json`: `linux-clang`, `linux-clang-release`, `macos-clang`, `windows-clang-cl`, `windows-cross`,
`windows-mingw-cross`, and `windows-msvc` for MSVC; toolchain files in `cmake/toolchains`).

Cross-compiling the DirectX backends from Linux:

* **`windows-cross`**: clang-cl + lld-link against the MSVC CRT / STL and the Windows SDK fetched by `xwin`
  (exact steps in [backends/README.md](backends/README.md#6-llvm-toolchain)). The ABI is MSVC's, as shipped.
  `xwin` downloads from `download.visualstudio.microsoft.com`, which the cloud sessions cannot reach.
* **`windows-mingw-cross`**: clang + ld.lld against mingw-w64's headers and import libraries (`apt-get install
  g++-mingw-w64-x86-64-posix mingw-w64-x86-64-dev`): Direct3D 9 / 10 / 11 / 12, DXGI and d3dcompiler headers,
  offline. A compile / link check (GNU ABI, libstdc++), verified in this session including running every test
  executable under Wine.

## 14. Testing

* **Unit tests** (CTest, no GPU): core (ids, input, windows, items, layout, draw lists, textures), renderer (planner
  decisions, Painter encoding, the command streams of every technique on the null device), test kit (PNG, image
  comparison), shader library (completeness; GLSL / ESSL compiled and linked on the system driver through EGL),
  text (the rasterizer's exact coverage, the atlas, FreeType layout with the fonts in `tests/fonts`,
  and golden images of the glyph quads composited on the CPU - no GPU needed).
* **Conformance suite** (`tests/conformance`, the scenes also a library for backend tests): 17 scenes built
  through the public API, rendered by every backend built into the binary. Drawing backends are read back and
  compared with golden PNGs within per-scene tolerances (a share of differing pixels, a maximum and a mean channel
  difference); the null backend's command streams are compared with golden logs, and any RHI contract violation
  fails. API validation messages (`Device::ValidationErrors`) fail a scene too. The image goldens come from OpenGL on
  Mesa llvmpipe (`esia_conformance --backend opengl --golden tests/conformance/golden --update`, reviewed); every
  backend compares against them with `--strict`, once on a fresh device and once after two poison frames
  (`--frames 3`: stale or undefined surfaces show). A backend that cannot run on the machine reports every scene
  skipped (exit code 77), which CTest shows as skipped, not passed.

## 15. Phases

| Phase | Content | Exit criteria |
| --- | --- | --- |
| 0 - core (this branch, `esia-core`) | toolchains, UI core, RHI + null device, shader pipeline and library, renderer (Painter, planner, captures, layers), conformance suite, text interface, docs | CTest green on Linux; Windows cross builds; guide complete for the backend sessions |
| 1 - backends (parallel: `esia-opengl`, `esia-vulkan`, `esia-metal`, `esia-directx`) | one `rhi::Device` per API + host wrap functions; OpenGL produces the image goldens | conformance suite passes per backend where it can run (GL / GLES / Vulkan on Mesa here; D3D and Metal on their OS) |
| 2 - text | FreeType + HarfBuzz complete (fallback, bidi, emoji layers); DirectWrite and Core Text behind the interface | WGT's language demo renders on every platform |
| 3 - widgets | `src/ui` + auto layout + `Theme` / `ItemStyle` / `anim` + text editing ported to `esia_ui` on the core | the WGT showcase panels run on Esia with the same screenshots (within tolerance) |
| 4 - platforms and services | Win32 (parity with WGT: IME, DPI, thread split), Cocoa, SDL3 / X11 / Wayland; `Post`, notifications, panels, plugins, pacing | the demo runs on Windows, macOS and Linux |
| 5 - compatibility | `wgt::` on Esia; `wgt.dll` 2.0 without Dear ImGui; performance parity (GPU profile categories vs. today's D3D11 numbers) | existing hosts recompile; demo scripts produce matching screenshots |
| 6 - cleanup | `third_party/imgui` removed; legacy sources retired | done ahead of the other phases: WGT was retired, tag `wgt-1.1-final` |

## 16. Risks and open questions

* **SM3 instruction budget.** The whole FX shader may exceed what a D3D9 profile accepts; the variant mechanism
  exists for that. Measured with Microsoft's compiler: since the hot rows arrive as varyings even the mask with all
  15 features compiles for `ps_3_0` (5.4k slots; before, glass with seven other features - `0x1FF` - ran out of
  temporary registers), within the 32k slots of current GPUs but far over the 512 SM3 guarantees - D3D9 on older
  hardware stays a risk.
* **FX instance data.** WGT passed the whole instance to the pixel shader as 24 flat interpolants (D3D11 has 32);
  GL 3.3 / GLES 3 / D3D10 guarantee 15 - 16 and SM3 10. The first Esia shader fetched all 24 rows at the start of
  every pixel - an über-shader keeps every load its branches might need, so "unused loads are dropped" did not
  hold, and a plain rounded rect paid 24 fetches per pixel. Now the vertex shader fetches the six hot rows (rect,
  radii, fill color, shape parameters, opacity, flags) and passes them as flat varyings (8 interpolators with the
  position and instance index), and each branch fetches the cold rows it reads: a solid shape reads nothing per
  pixel. `ESIA_FX_FETCH_ALL` restores the old path for A / B timing with the GPU profile categories
  (`tools/shaders/build_shaders.py --define ESIA_FX_FETCH_ALL=1`, or the define in a backend's runtime
  compilation); both paths render the same pixels.
* **Metal from a Linux session.** It cannot be compiled or run there; the Metal backend needs a macOS machine
  (`macos-clang` preset, `xcrun metal` to validate the MSL) before it is trusted.
* **D3D shaders from a Linux session.** DXBC comes from a Windows run (or `D3DCompile` at runtime). On Linux the
  DirectX backends are cross-compiled; D3D9 / 10 / 11 also run the conformance suite under Wine (wined3d over
  llvmpipe) with Microsoft's `d3dcompiler_47.dll`, which tests the code paths but not a real driver; D3D12 does not
  start there. Real drivers: the local Windows run (NVIDIA, round 2).
* **Text parity.** DirectWrite's system fallback and color emoji are Windows features; the
  FreeType path must match WGT's rasterizer closely enough that screenshots stay within tolerance.
* **sRGB targets blend in linear light.** Translucent content on an `*_SRGB` target looks different from a UNORM
  target (it does in WGT's D3D11 backend too); the conformance suite has a separate golden for it.
