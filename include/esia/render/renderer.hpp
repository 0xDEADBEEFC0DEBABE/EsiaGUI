// Esia - the renderer: draws a frame's DrawData through an rhi::Device.
//
// Per frame (Render):
//   1. BeginFrame; the TextureRegistry's pending changes become RHI textures (glyph pages, images);
//   2. FramePlan::Build: batches, capture decisions, merged vertex / index / instance buffers, uploaded once;
//   3. the plan runs in passes on the target:
//      * geometry and text with the UI programs (the text program follows the texture's coverage format),
//      * FX batches as one instanced draw each (4-vertex strips; instance data in a buffer or RGBA32F texture),
//      * before a glass batch the planner marked: the backdrop capture - end the pass, copy the capture region out
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
#include <memory>
#include <string>

namespace esia::render
{
    // How glyph coverage becomes alpha (the DirectWrite model; see TextGrayPS / TextLcdPS in esia_ui.hlsl).
    struct TextComposition
    {
        float gamma = 1.8f;               // display gamma of the alpha correction (1.0 .. 2.2)
        float grayscaleContrast = 0.5f;   // enhanced contrast (stem weight) of grayscale text
        float clearTypeContrast = 0.5f;   // ... of sub-pixel text
        float clearTypeLevel = 1.0f;      // 0 = sub-pixel pages drawn as grayscale, 1 = full sub-pixel
    };

    struct RenderParams
    {
        rhi::FrameDesc frame;             // the backend's recording context and the host's frame number, if it needs them
        TextComposition text;
        int maxBackdropCaptures = 64;     // per frame; past it, glass reuses the last capture (Stats().overBudget)
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
        int glowLayers = 0;
        int vertices = 0;
        int indices = 0;
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
        // the built-in shader until the device compiled it (Caps::runtimeEffects), or for good when it cannot.
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
