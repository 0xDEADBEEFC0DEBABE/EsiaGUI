// Esia conformance suite - the test scenes. Each one is built only through the public API (Painter, DrawList,
// TextureRegistry, Context) and covers a slice of what every backend must render the same way; the harness
// (conformance.cpp) renders them with any registered backend and compares the result with the goldens.
#pragma once
#include "esia/core/context.hpp"
#include "esia/rhi/rhi.hpp"
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
        bool needsDualSource = false;          // sub-pixel text: skipped where Caps::dualSourceBlend is false
        const char* golden = nullptr;          // golden image name when shared with another scene (default: name)
        testkit::Tolerance tolerance;
    };

    const std::vector<Scene>& Scenes();
    const Scene* FindScene(const char* name);

    // Builds the scene's frame: display size / scale / clock set, draw lists filled.
    void BuildScene(const Scene& scene, SceneFrame& frame);
}
