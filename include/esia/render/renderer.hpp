// Esia - the renderer: draws a frame's DrawData through an rhi::Device.
//
// Per frame (Render):
//   1. BeginFrame; the TextureRegistry's pending changes become RHI textures (glyph pages, images);
//   2. FramePlan::Build: batches, capture decisions, merged vertex / index / instance buffers, uploaded once;
//   3. the plan runs in passes on the target:
//      * geometry with the UI programs, glyphs and flat tiles as instanced quads (the program follows the texture's
//        coverage format; the quads' rows follow the FX instances in the instance data),
//      * FX batches as one instanced draw each (4-vertex strips; instance data in a buffer or RGBA32F texture),
//      * before a glass batch the planner marked: the backdrop capture (unless last frame's still serves it,
//        RenderParams::targetStatic) - end the pass, copy the capture region out
//        of the target (or read the target directly when no glass reads the full-resolution level and the target
//        can be sampled, or when it cannot be copied: pyramid level 1 then stands in for level 0), build only the
//        pyramid levels the batch needs (13-tap downsample, taps clamped to the refreshed region), resume the pass;
//        a target that can be neither copied nor sampled has no backdrop,
//      * glow layers: an offscreen RGBA16F layer (only the touched region cleared), its pyramid, and the content +
//        bloom composite back onto the target,
//      * edge fades as per-draw constants, host callbacks with the backend's native state;
//   4. GPU time per category (capture, layer, FX, FX with glass, geometry) when the device has timestamps;
//   5. EndFrame.
// These are the techniques of WGT's Direct3D 11 / 12 backends, written once for every API.
#pragma once
#include "esia/render/frame_plan.hpp"
#include "esia/rhi/rhi.hpp"
#include <cmath>
#include <memory>
#include <string>

namespace esia::render
{
    // How glyph coverage becomes alpha (the DirectWrite model; see TextGrayPS / TextLcdPS in esia_ui.hlsl).
    struct TextComposition
    {
        float gamma = 1.8f;               // display gamma of the alpha correction (1.0 .. 2.2)
        float grayscaleContrast = 0.5f;   // enhanced contrast (stem weight) of grayscale text, shapes and outlines
        float clearTypeContrast = 0.5f;   // ... of sub-pixel (LCD) text
        float clearTypeLevel = 1.0f;      // 0 = sub-pixel pages drawn with their grayscale coverage, 1 = full sub-pixel
    };

    // One light for the whole UI (RenderParams::light): glass rims catch it, the side of a slab facing it brightens,
    // drop shadows fall away from it (their offsets turned, their lengths kept) - a point above the screen, such as the
    // cursor, or a direction, such as a device's tilt. Off: each glass keeps its own light angle, each shadow its
    // offset.
    struct UiLight
    {
        bool enabled = false;
        Vec2 position;          // UI units: where the point light is - or, with height 0, the direction towards it
        float height = 0.0f;    // UI units above the screen; 0: a directional light
        static UiLight Point(Vec2 at, float height = 240.0f)
        {
            UiLight l;
            l.enabled = true;
            l.position = at;
            l.height = height;
            return l;
        }
        static UiLight Direction(Vec2 towards)
        {
            UiLight l;
            l.enabled = true;
            l.position = towards;
            return l;
        }
        // A light overhead seen on a screen held at this tilt (radians: pitch, the top edge away from the viewer; roll,
        // the right edge down): from the top left when held level, moving across the screen against the tilt, as the
        // glints of iOS's glass do (feed it the device's motion, e.g. CoreMotion's attitude).
        static UiLight FromTilt(float pitch, float roll)
        {
            return Direction(Vec2(-0.6f - 2.0f * std::sin(roll), -0.8f + 2.0f * std::sin(pitch)));
        }
    };

    // How the target's pixels encode light (RenderParams::hdr).
    enum class HdrEncoding : std::uint8_t
    {
        None,     // an SDR target: sRGB values (or linear light on an _SRGB format), drawn into as always
        ScRgb,    // linear BT.709, 1.0 = 80 nits, past 1 brighter, below 0 outside BT.709: an RGBA16_FLOAT target
                  // (Windows' HDR, DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709)
        Pq2020,   // SMPTE ST 2084 (PQ) code values, BT.2020 primaries: an RGB10A2_UNORM target (HDR10,
                  // DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
    };

    // HDR output. The UI is drawn as on an SDR target - sRGB colors, white at paper white, the same blending, glass and
    // text - into an RGBA16_FLOAT working target: the region the frame draws in or reads from comes into it from the
    // target as an SDR target would hold it ("extended sRGB": 1 = paper white, what lies past 1 and below 0 kept), and
    // goes back in the target's encoding afterwards. Needs Caps::floatRenderTargets and a single-sampled target that
    // can be sampled or copied, taken as opaque. Host draw callbacks draw into the working target.
    struct HdrOutput
    {
        HdrEncoding encoding = HdrEncoding::None;
        // The luminance of UI white (sRGB 1.0), nits: what the system shows SDR content at on that display (Windows:
        // the monitor's SDR white level), so the UI matches the desktop. 203 is ITU-R BT.2408's graphics white.
        float paperWhiteNits = 203.0f;
    };

    struct RenderParams
    {
        rhi::FrameDesc frame;             // the backend's recording context and the host's frame number, if it needs them
        TextComposition text;
        // Sub-pixel (LCD) glyph pages draw with per-channel alpha (dual-source blending), which is right on an opaque
        // target. Off for a target that is composited later through its alpha (a transparent window, a texture drawn
        // over other content): those pages then draw with their grayscale coverage, as inside glow layers.
        bool subpixelText = true;
        int maxBackdropCaptures = 64;     // per frame; past it, glass reuses the last capture (Stats().overBudget)
        // Temporal reuse of the backdrop (glass): the renderer keeps the last frame's captures and skips one whose
        // region, pyramid and the content drawn under it are the same now - its copy and blur are still in the
        // pyramid (Stats().reusedCaptures). Only when the target's own content under the glass is the same as well: set
        // this when the host gives the target the same content before every frame (it clears it to one color, or
        // draws the same background). Content that animates by itself (shimmer, user effects, host callbacks) is never
        // taken for unchanged.
        bool targetStatic = false;
        // ... and with a live background (a game under an overlay): a capture of the same region and pyramid serves for
        // up to this many frames after it was taken, whatever changed under it - the frost shows a backdrop that many
        // frames old, for a part of the capture cost (0 = never).
        int backdropReuseFrames = 0;
        // One light for every glass rim and drop shadow (UiLight); off by default.
        UiLight light;
        // How the target encodes light (HdrOutput); SDR by default.
        HdrOutput hdr;
        // GPU time per category (Caps::timestampQueries). Every switch between categories costs two timestamps, so
        // after maxProfileScopes of them the rest of the frame counts in totalMs only. Off: the frame total only.
        bool profile = true;
        int maxProfileScopes = 32;
        // FxStorage::Texture: at most this many instances per row of the instance texture (0 = what
        // Caps::maxFxDataWidth allows). The conformance suite uses it to run the multi-row path with a few shapes;
        // keep it a power of two (SM3 computes the row with a float modulo that is exact only then).
        int maxFxInstancesPerRow = 0;
    };

    struct RenderStats
    {
        int drawCalls = 0;                // every draw, pyramid passes and composites included
        int passes = 0;
        int fxInstances = 0;
        int fxBatches = 0;
        int backdropCaptures = 0;
        int directCaptures = 0;           // captures whose pyramid was built straight from the target (no copy)
        int reusedCaptures = 0;           // captures an earlier frame's served (RenderParams::targetStatic, backdropReuseFrames)
        int restoredCaptures = 0;         // ... of them brought back from the copy kept aside (something wrote over them)
        int glowLayers = 0;
        int vertices = 0;
        int indices = 0;
        int quads = 0;                    // instanced quads (QuadInstance: glyphs, flat tiles)
        bool overBudget = false;          // maxBackdropCaptures was reached this frame
        int fxFallbacks = 0;              // FX batches drawn with the full shader: their variant was refused or failed
        int fxPendingVariants = 0;        // FX batches drawn with a substitute while their variant compiles (asyncPipelines)
        rhi::GpuProfile gpu;              // the latest GPU times the device returned (a few frames old)
    };

    class ESIA_API Renderer
    {
    public:
        explicit Renderer(rhi::Device& device);
        ~Renderer();
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        // Renders `dd` into `target`: a render target of this device (a backend-wrapped host target, or a texture
        // created with TextureUsage_RenderTarget). The registry's pending texture changes are applied first
        // (`textures` may be null). Returns false when the device refused the frame.
        bool Render(const DrawData& dd, TextureRegistry* textures, rhi::Texture target, const RenderParams& params = {});

        // A user effect (HLSL defining `float4 WgtEffect(WgtFx fx)`, see docs/REWRITE.md). Shapes using it draw with
        // the built-in shader until the device compiled it (Caps::runtimeEffects), or for good when it cannot. The
        // compile starts at the next frame, before a shape uses the effect (not where the backend builds feature
        // variants: the one a shape needs is known when it draws).
        void SetEffectSource(EffectId id, const std::string& name, const std::string& source);

        // Releases the size-dependent surfaces (backdrop copy, pyramid, glow layer); they come back when needed.
        void ReleaseSurfaces();

        const RenderStats& Stats() const;
        const FramePlan& Plan() const;
        // The RHI texture behind a registry texture (0 before the frame that created it).
        rhi::Texture GpuTexture(TextureId id) const;

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
}
