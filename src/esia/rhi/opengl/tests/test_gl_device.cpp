// Esia OpenGL backend - device tests on the machine's driver (Mesa llvmpipe through EGL in CI). Every test runs
// on both backends ("opengl": GL 3.3 core, "gles": GLES 3.0) and is skipped when EGL offers no such context.
#include "esia/render/renderer.hpp"
#include "esia/rhi/backend_registry.hpp"
#include "esia/rhi/opengl.hpp"
#include "esia_test.hpp"
#include "scenes.hpp"
#include <cstdio>

using namespace esia;

namespace
{
    const char* const kBackends[] = {"opengl", "gles"};

    rhi::HeadlessDevice Headless(const char* backend, const rhi::HeadlessDesc& desc)
    {
        rhi::RegisterBuiltinBackends();
        std::string error;
        rhi::HeadlessDevice h = rhi::FindBackend(backend)->createHeadless(desc, error);
        if (!h.device)
            std::printf("  %s: skipped (%s)\n", backend, error.c_str());
        return h;
    }
}

// Every conformance scene renders and reads back without a single GL error (KHR_debug in debug builds), two frames
// each so the second one runs on the device's caches.
ESIA_TEST(GlDevice, ConformanceScenesWithoutGlErrors)
{
    for (const char* backend : kBackends)
        for (const conformance::Scene& scene : conformance::Scenes())
        {
            rhi::HeadlessDesc hd;
            hd.width = scene.width;
            hd.height = scene.height;
            hd.format = scene.format;
            hd.samples = scene.samples;
            rhi::HeadlessDevice h = Headless(backend, hd);
            if (!h.device)
                continue;
            if (scene.needsDualSource && !h.device->GetCaps().dualSourceBlend)
                continue;
            render::Renderer renderer(*h.device);
            for (int frame = 0; frame < 2; ++frame)
            {
                conformance::SceneFrame f;
                conformance::BuildScene(scene, f);
                ESIA_CHECK(renderer.Render(f.data, &f.textures, h.target));
            }
            std::vector<std::uint8_t> px;
            ESIA_CHECK(h.device->ReadPixels(h.target, rhi::IRect{0, 0, scene.width, scene.height}, px));
            const std::uint32_t errors = rhi::opengl::ErrorCount(*h.device);
            if (errors)
                std::printf("  %s %s: %u GL errors\n", backend, scene.name, errors);
            ESIA_CHECK(errors == 0);
        }
}
