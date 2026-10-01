// Esia conformance suite - the test scenes. Each one is built only through the public API (Painter, DrawList,
// TextureRegistry, Context) and covers a slice of what every backend must render the same way; the harness
// (conformance.cpp) renders them with any registered backend and compares the result with the goldens.
//
// Backend tests that render the scenes themselves (to check their API's validation, both render paths ...) link the
// esia_conformance_scenes library and create the target and the render parameters with HeadlessDescOf and
// RenderParamsOf, as the harness does, so that their results can be compared with the goldens.
#pragma once
#include "esia/core/context.hpp"
#include "esia/render/renderer.hpp"
#include "esia/rhi/backend_registry.hpp"
#include "image.hpp"
#include <memory>
#include <vector>

namespace esia::conformance
{
    // Everything a scene creates for one frame; kept alive until the frame was rendered.
    struct SceneFrame
    {
        TextureRegistry textures;
        std::vector<std::unique_ptr<DrawList>> lists;
        std::unique_ptr<Context> ui;          // scenes that go through the UI core
        DrawData data;
        int callbacks = 0;                     // host callbacks of the frame that ran

        DrawList& NewList();                   // appended to `data.lists`, clip = the whole display
    };

    struct Scene
    {
        const char* name;
        const char* covers;                    // what it exercises (--list)
        void (*build)(SceneFrame& frame);
        int width = 320, height = 240;         // render target (pixels)
        float scale = 1.0f;                    // render-target pixels per UI unit
        rhi::Format format = rhi::Format::RGBA8_UNORM;
        int samples = 1;
        bool sampleable = true;                // HeadlessDesc::sampleable; false: every backdrop capture copies
        int fxInstancesPerRow = 0;             // RenderParams::maxFxInstancesPerRow (FxStorage::Texture on several rows)
        int callbacks = 0;                     // host callbacks the frame must run (SceneFrame::callbacks)
        // Drawing backends: after the image check, CopyTexture a region of the target to another position of a second
        // texture (resolving a multisampled target) and compare it with the target's pixels.
        bool checkCopy = false;
        const char* golden = nullptr;          // golden image name when shared with another scene (default: name)
        testkit::Tolerance tolerance;
    };

    const std::vector<Scene>& Scenes();
    const Scene* FindScene(const char* name);

    // Builds the scene's frame: display size / scale / clock set, draw lists filled.
    void BuildScene(const Scene& scene, SceneFrame& frame);

    // A frame of loud content at the scene's size that leaves every surface the renderer keeps between frames dirty
    // (backdrop copy and pyramid, glow layer, FX instance data, vertex / index buffers), and uses no registry
    // texture. The cross-frame mode renders it before the scene's frame: a frame that reads what it did not write
    // then differs from its golden, even where the first frame on a fresh device reads zeros.
    void BuildPoisonFrame(const Scene& scene, SceneFrame& frame);

    rhi::HeadlessDesc HeadlessDescOf(const Scene& scene);
    render::RenderParams RenderParamsOf(const Scene& scene);
}
