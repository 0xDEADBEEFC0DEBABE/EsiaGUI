# Architecture

WGT is one DLL that contains Dear ImGui 1.92 plus everything WGT adds on top. Dear ImGui provides
immediate-mode ids, the basic cursor layout, window management, input routing and text editing. WGT replaces everything
that decides how the UI looks and flows:

* the widgets and responsive layout (`wgt::ui`)
* the style (`wgt::Theme`)
* the renderer (`wgt::Painter` → FX instances → WGT backends)
* text (DirectWrite `TextEngine`)
* motion (springs)

```
 game / tool code            wgt::ui widgets, custom widgets, plugins
        │                          │  (ui::Interact + Painter + anim)
        ▼                          ▼
 ┌──────────────┐    ┌────────────────────────────────────────────────┐
 │ wgt::Context │───▶│ Dear ImGui (ids, layout, windows, input)        │
 │ frame loop,  │    │ ImDrawList = ordinary triangles  +  FX commands │
 │ services     │    └────────────────────────────────────────────────┘
 └──────────────┘                          │ ImDrawData
                                           ▼
                             FramePlan (backend agnostic)
                  batches · backdrop-capture decisions · glow layers
                                           │
                        ┌──────────────────┴──────────────────┐
                        ▼                                     ▼
                 D3D11 backend                         D3D12 backend
```

## Frame flow

1. **`Context::NewFrame()`** (UI thread):
   * makes the context current (thread-local ImGui context);
   * replays queued Win32 input, then injected input;
   * runs tasks posted with `Post()` and attaches pending plugins;
   * applies the scale model (DPI × user scale, render scale) and steps the theme animation;
   * starts the ImGui frame and draws the registered panels, the dock and the island.
2. **Widgets** call `ImGui` for ids/layout/interaction and `wgt::Painter` for pixels. Painter never makes
   triangles for shapes. Each shape becomes one `fx::Instance`: 24 × float4 describing geometry and the
   full material. It goes into the window's `ImDrawList` as a callback command, so it stays correctly
   ordered and clipped with everything ImGui draws.
3. **`Context::Render(target)`**:
   * finishes the ImGui frame;
   * lets the text engine upload new glyph pages;
   * hands `ImDrawData` to the backend.
4. **`FramePlan::Build`** walks the draw data once and produces a flat list of `RenderOp`s:
   * ImGui draws, with pixel bounds for dirty tracking. Draws that sample a glyph atlas page (text) are
     flagged *coverage*, and *lcd* for sub-pixel pages.
   * **FX batches**: runs of FX commands with the same clip, texture and effect become *one* instanced
     draw. A batch is split whenever a glass shape would need to see something drawn earlier in the same
     batch.
   * glow-layer begin/end markers;
   * foreign user callbacks, which run untouched.
5. **The backend executes the plan.** Both backends share the plan, the constants in `gpu_common.hpp` and
   the HLSL, so the two APIs render the same pixels.

## FX shapes (`src/shaders/wgt_fx.hlsl`)

Every primitive is a quad, sized to cover its shape plus its shadow and glow, with the instance's fields
passed as `nointerpolation` attributes. The pixel shader computes the following:

* **Signed distance**:
  * rounded rect with per-corner radii and *continuous* corners (a superellipse evaluated only in the
    corner quadrants);
  * arc, segment;
  * smooth-min union of two rects (liquid merge);
  * rounded mask.
* **Paint**: solid, or linear / radial / conic gradient, or image, then stroke (inside / centered /
  outside, with a directional fade).
* **Shadows**: analytic erf-based drop shadows and inner shadows; outer and inner glow.
* **Liquid glass** (`EvalGlass`): a clear slab whose edge is rounded with radius `bezel`.
  * Surface: a circle profile from the SDF (flat top inside the bezel band), so the normal is straight up on
    the flat top and tilts to vertical at the rim. The bezel is clamped to half the shape's size: a bar or a
    control is one rounded rod / dome, like iOS glass.
  * Refraction: the content behind is pulled towards the inside by the sag of that curve, up to `refraction`
    at the rim. The flat top shows the content undistorted, the curve magnifies it towards the edge, and
    where the pull outgrows the distance to the rim the image folds into a thin mirrored band (the edge of
    real glass). Chromatic dispersion separates R / G / B only in the curved band. `magnify` adds a loupe:
    the content is enlarged around the shape's center (the clear lens a tab selection, slider or switch knob
    becomes while pressed or moving); the fringes stay at the rim.
  * Frost: the backdrop is read from a blur pyramid (B-spline filtered, radius in UI units × render scale).
    Clear and control glass barely frost, windows a little.
  * Vibrancy: saturation and brightness. *Legibility* exposes the backdrop toward the tint's brightness,
    using the brightness of a wide neighbourhood rather than per pixel, so detail and color stay visible.
  * Fresnel: the steep outer rim mirrors the blurred surroundings just outside the edge, and a highlight lit
    from `lightAngle` sits on the rim facing the light (weaker on the opposite side).
  * Then the tint, laid in layers rather than as a flat coat: heavier on the curved bevel, a light veil
    gathering towards the lit top and a dark one towards the bottom. A light term brightens the side facing
    the light and the bevel, on clear glass too. Film-grain noise against banding comes last.
  * Glass looks (`Theme` / `Clear` / `Frosted`, see EXTENDING.md) transform the material on the CPU before
    it is emitted; windows and cards fold their surface color into the tint so it is layered too.
* **Notification island** (`ui/overlay.cpp`). A notification arrives like the Siri orb: the island opens
  into a card big enough for the whole message and the full effect. Inside it, a black body fades into a
  clear drop at the bottom, and light sits where black turns into glass (`Painter::LightStreak`, `kCaustic`):
  a few thin lines of light, softened just enough, held together at both ends and opening up in the middle.
  The top one is dispersed into thin warm, green and lavender lines; a white line runs along the bottom with a
  white bloom above it. The light opens on the notification's own clock and then breathes, and brightness
  saturates softly instead of clipping. A clear dome lens (magnify,
  dispersion) is drawn over all of it, so the liquid glass transforms the light as it does in the orb: the
  strands curve and split into color at the rim, and the drop magnifies what lies below. After about a second
  it morphs into a plain Dynamic Island pill (icon, title, time-left ring) until the notification ends. Live
  activities stay in the plain pill; a newly started one gets the same short arrival. Size, body, drop,
  streak and content are separate springs, and each content fades in once the shape fits it.
* **Optional**: shimmer (skeleton loading), then the custom effect (see EXTENDING.md).
* **Halo**: an outer glow can carry halo bounds (`fx::kHalo`). The glow keeps its gaussian profile, but its
  radius becomes elliptical: along each axis it shrinks to the room left on that side. The blend follows
  the shape's outward normal, measured from the shape's core (the rect minus the corner radius), so the glow
  flows around rounded corners and capsule ends without seams. A 1.5 px fade guarantees nothing lands past
  the bounds, and the vertex shader shrinks the quad to them.
* Glow and shadow falloffs get ±½ LSB of dithering, so wide faint gradients do not band.

Anti-aliasing is analytic: coverage comes from the distance and the pixel footprint. Shapes stay sharp at
any scale because nothing is tessellated.

## Auto layout and the item map (`src/ui/layout.cpp`)

The Dear ImGui in `third_party/imgui` is WGT's own copy and is changed wherever WGT needs it to behave
differently:

1. `ImGui::ItemSize()` reports every laid-out item (window and rect) to `WgtOnItemLaidOut()`, which is
   declared in `imgui_internal.h`.
2. `EndGroup()` lays a group out at its parent's depth, so a group is a single child of the container that
   holds it.

The hook feeds two things:

* **Layout containers**:
  * `Begin*()` computes every child's slot from the sizes, flexibility and glow margins measured on the
    previous frame, and from the width available now.
  * Each item submitted at the container's group depth is a child. The hook records its size, then moves
    the cursor and the content region (`WorkRect` / `ContentRegionRect`) to the next slot, so widgets that
    fill the available width fill exactly their slot.
  * `End*()` restores the region and reports the arranged size as one group item to the parent.
  * Slot positions are spring-animated relative to the container origin.
  * A container's first frame is clipped away while it measures.
* **The item map**:
  * the item rects of every window, for the current and the previous frame;
  * the painter reads it for a glowing shape and turns the nearest neighbour on each side into halo
    bounds, with a fade width taken from the smallest gap;
  * the same draw reports its glow reach (from the resting geometry, so press / pop scale animations do
    not re-flow) to the enclosing container. Only the part that sticks out of the child it belongs to
    counts: a glowing button gets its full margin, a glowing toggle inside a tile usually none. The
    container spaces that child further from its neighbours on the next frame.
  * Slot targets are whole pixels, so a settled spring rounds to the same pixel every frame.

## Backdrop capture (liquid glass)

Glass needs to see what is behind it. It must be the *current* frame including everything drawn before
the glass (game scene, windows below, a panel's own background), not last frame's picture.

Captures are planned for the whole frame in `FramePlan::PlanCaptures()` before anything executes:

* **Dirty tracking** is a list of rectangles (not one growing box), so a draw elsewhere never invalidates a
  capture. Each op records its *core*: the shape plus the part of its shadow / glow that is still visible
  (above 4 %). The faint outer tails vanish through blurred glass, so they never force a capture.
* **Glass vs. glass**: liquid glass shapes are tracked apart from other content, as in SwiftUI's glass effect
  container. A glass batch needs a new capture when ordinary content was drawn inside its blur footprint,
  but other glass only counts where it actually sits under it (a button on a glass card). Neighbouring
  glass controls (the Control Center toggles, a row of glass buttons) share one backdrop.
* **Look-ahead merging**: a capture is stretched over the following glass batches that nothing draws under
  in between, as long as the merged region stays mostly useful (area ≤ 1.6 × the sum of the parts). One
  capture then serves a window and every glass control on it. The scan stops at the first glass batch that
  must capture anyway, and at the end of a glow layer (its bloom pyramid replaces the backdrop pyramid).
* The backend just executes the plan: `RenderOp::captureRegion` / `captureLevels` on a glass batch mean
  "capture this first"; empty means reuse.
* A capture is region-limited:
  * copy the region (plus a 64 px margin for blur footprints) out of the render target;
  * build only the pyramid levels the batch's largest blur needs, using a 13-tap downsample into RGBA16F
    with taps clamped to the valid region (`PyramidStep`), so stale texels never bleed in.
* Nested glass works: a control on a glass window re-captures after the window background was drawn, so
  it refracts the window rather than the wallpaper.
* `ContextDesc::maxBackdropCaptures` (default 64) bounds the work per frame. Past it, glass reuses the last
  capture and a warning is logged once.

* **Cost**: about nine tenths of WGT's GPU time is liquid glass, half in captures (a copy plus the blur pyramid,
  bandwidth bound: the cost follows the captured area) and half in shading the glass. Captures are kept small:
  a glass surface asks for its frost footprint plus the rim reflection just outside its edge (`GlassEnvReach`,
  at most 16 UI units), and only the pyramid levels it reads. The legibility exposure reads one level
  (`AmbientLevel`, bilinear), so no capture builds level 5 for it.
* **No copy for frost**: a capture whose glass never reads the full-resolution level (frosted glass; clear glass
  and user effects do read it, `RenderOp::readsLevel0`) builds its first pyramid level straight from the render
  target when the host created it as a shader resource. The values are the same as through the copy; the copy's
  bandwidth is saved. Otherwise, and for MSAA or typed sRGB targets, it copies as before.
* **Shading**: the frost is reconstructed with a B-spline across two levels (smooth under any scale). The wide,
  low-contrast reads use single bilinear taps: the rim reflection and the legibility exposure. Rainbow fringes
  narrower than the frost are invisible, so frosted glass skips the extra dispersion reads.
* `WGT_GPU_PROFILE=1` (D3D11) logs the per-category GPU time and one frame's capture regions every 240 frames.

## Profiling

`Context::GetStats()` reports the UI thread's CPU time (`cpuUiMs`: NewFrame to draw data, host waits
excluded when the host waits before `NewFrame`) and GPU timestamps from both backends: `gpuMs` for all of
WGT's rendering, `gpuGlassMs` for backdrop captures and blur pyramids, `gpuLayerMs` for glow layers. The
queries never stall: D3D11 reads them a few frames later with `DONOTFLUSH`, and D3D12 resolves them into a
readback slot per frame in flight. Measured with the demo on an RTX 4080 SUPER at 2400×1231:

| scene | glass captures | UI GPU |
|---|---|---|
| wallpaper + dock | 3 | 0.11 ms |
| Settings | 4 | 0.18 ms |
| Settings + Effects + Control Center | 12 | 0.49–0.51 ms |
| all panels | 14 | 0.64–0.68 ms |

## Glow layers

`Painter::BeginGlowLayer()` / `EndGlowLayer()` redirect everything drawn in between into an offscreen
RGBA16F layer (scissored clear of the touched area only). At the end:

1. the backend builds a pyramid of the layer;
2. it composites *content + bloom* back into the render target (`LayerCompositePS`);
3. the tint, intensity, radius and content opacity come from the layer parameters.

This is how neon text, glowing lines and icons are made.

## Edge fades (scroll views)

`Painter::BeginEdgeFade` / `EndEdgeFade` put `FadeBegin` / `FadeEnd` commands into a draw list. The frame
plan tags every draw and FX batch in between with the fade (top / bottom edge and widths, in render-target
pixels), and the backend passes it as per-draw constants (`b2`: a small constant buffer in D3D11, four root
constants in D3D12). The ImGui and FX pixel shaders multiply their alpha by `WgtEdgeFade`, a smooth ramp to
0 at the edge. Scroll areas open a fade over their visible rect whose widths follow the distance left to
scroll, so content dissolves under a header or the window's rounded bottom instead of being cut.

## Text (`src/text/`)

WGT does not use `stb_truetype` or ImGui's font atlas for its own text. `TextEngine` does the following:

* **Fonts:** resolves family names through DirectWrite (system collection + files registered with
  `ContextDesc::fontFiles` / `Context::AddFontFile`). It handles optical-size families (Segoe UI Variable
  Small/Text/Display) and falls back per character (`IDWriteFontFallback`), with the fallback shaped by
  `ContextDesc::locale`, which also picks CJK glyph variants.
* **Shaping:** shapes with `IDWriteTextLayout`, covering kerning, ligatures, bidi, complex scripts and CJK
  line breaking. Layouts are cached.
* **Rasterizing** (`glyph_raster.cpp`), the way Core Text does it on macOS:
  * WGT rasterizes each glyph itself from its *unhinted* outline (`GetGlyphRunOutline`, Béziers flattened
    to 1/12 px) with exact area coverage: a signed-area accumulation per scanline gives each pixel the
    exact fraction of it that is covered.
  * Without grid fitting, shapes and spacing stay true to the design.
  * With exact coverage, edges get a full 8-bit ramp instead of a few quantized levels.
  * Glyphs are rasterized at the *exact physical pixel size* (UI size × render scale), with 4 horizontal
    sub-pixel phases.
  * They go into WGT-owned atlas pages (Alpha8 for grayscale, RGBA for sub-pixel), uploaded through ImGui's
    texture protocol.
* **Color emoji:** color fonts are translated into COLR layers (`TranslateColorGlyphRun`). Each layer is an
  ordinary outline glyph drawn with its palette color, so emoji need no RGBA atlas.
* **Anti-aliasing** (`ContextDesc::textAntialiasing`):
  * **Sub-pixel** (ClearType-style) is used automatically when Windows ClearType is on and the UI maps 1:1
    to display pixels. Glyphs are rasterized at 3× horizontal resolution and filtered with FreeType's 5-tap
    LCD filter. That gives one coverage per R/G/B stripe, stored in RGBA atlas pages, and the text is
    drawn with dual-source blending (a per-channel alpha).
  * Inside glow layers, which are alpha offscreen targets, the grayscale coverage kept in A is used.
  * **Grayscale** is the fallback, for Retina-class densities or OLED stripe layouts.
* **Composition:** coverage becomes alpha the way DirectWrite composes it, using the user's system
  parameters (ClearType Text Tuner):
  * enhanced contrast (stem weight), fading out for light ink;
  * then DirectWrite's gamma alpha correction.

  `textGamma` / `textContrast` override them. Grayscale runs at half the system stem weight: it has no
  horizontal sub-samples, and heavy stems make its stair-steps visible.
* **Motion:**
  * glyph quads are snapped to the physical pixel grid;
  * scroll offsets are snapped too, so text and surfaces move together without judder;
  * text under `Painter::PushScale` (press / pop animations) is re-rasterized at the scaled size every
    frame instead of stretching a bitmap.
* **Editing** (`ui/text_edit.cpp`): `ui::TextField` is WGT's own editor on the same engine, with no
  `ImGui::InputText`:
  * the caret moves by grapheme clusters, with bidi-correct caret positions (`CaretGeometry`);
  * selection, clipboard, undo / redo;
  * IME composition drawn inline. The context handles `WM_IME_*` on the window thread, reads the
    composition and result strings, and places the candidate window at the caret.

Nothing needs to be configured for any language: there are no glyph ranges and no fonts to merge.

Stock ImGui widgets (`ImGui::InputText`, `ImGui::SliderFloat`, …) still need `ImFont`s. A small
`ImFontLoader` shim (`imgui_font_loader.cpp`) feeds them glyphs from the same DirectWrite faces, so even
stock widgets get the WGT font, fallback and density. Use `wgt::GetImGuiFont(weight)` to push them.

## Scale model

| Quantity | Meaning | Source |
| --- | --- | --- |
| UI unit | window client pixel: mouse input and ImGui layout space | — |
| DPI scale | monitor DPI / 96 | `WM_DPICHANGED` + polling, or `SetDpiScale` |
| user scale | user preference ("Text Size") | `SetUiScale` |
| **metrics scale** | DPI × user: every size, radius, blur and font size | automatic |
| **render scale** | render-target pixels per UI unit | target size ÷ window size, or `SetRenderScale` |

Text is rasterized at metrics × render density, and every SDF parameter measured in pixels is converted
with the render scale in the shader. A game rendering at 4K behind a 1080p window gets 2× glyphs, and a
720p target does not waste atlas space. Changing the metrics scale at runtime rescales the positions and
sizes of open windows, so layouts stay proportional.

## Animation

`anim::Float/Vector/Colour` are analytic damped springs keyed by `ImGuiID`, parameterised like SwiftUI:
*response* (period, seconds) and *damping* (1 = critical). Widgets derive "liquid" motion from them:

* velocity-driven stretch and squash;
* `LiquidPulse`, which turns a knob or selection pill into a glass lens while pressed and briefly after
  every activation;
* labels that follow the moving selection rather than the selected index;
* pills that squash against the end of their track instead of leaving it (`ContainLiquid`).

The theme itself is animated too: dark/light and accent changes interpolate every color and material.

## Backends

|  | Direct3D 11 | Direct3D 12 |
| --- | --- | --- |
| host state | backed up and restored around `Render()` (`d3d11RestoreState`) | records into the host's command list; sets its own heaps and root signature |
| pipelines | created at init; effect shaders compiled in the background from registration | PSO cache keyed by kind / effect / format / sample count |
| per-frame data | dynamic buffers | upload ring per frame in flight, deferred release |
| resources | backdrop pyramid, glow layer, glyph / user textures | same, plus a shader-visible descriptor heap allocator |

`IRenderBackend` (`wgt/backend.hpp`) is public. A custom backend (Vulkan, an engine RHI) receives the
`ImDrawData` and recognises WGT commands with `fx::ReadHeader` / `fx::ReadShape` / `fx::ReadLayer`
(`wgt/fx.hpp`). Those commands are the contract between Painter and every renderer. A stock ImGui backend
that knows nothing about WGT just calls the no-op callback, so WGT draw lists degrade gracefully instead of
crashing.
