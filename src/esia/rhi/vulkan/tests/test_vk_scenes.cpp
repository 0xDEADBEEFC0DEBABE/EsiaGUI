// Vulkan backend: the conformance scenes rendered by the renderer on both render paths (dynamic rendering, render
// passes) must give identical pixels, with no validation message over a whole device lifetime (the messenger prints
// them; CTest fails on that output too). Several frames on one device exercise the frame slots: rings reused,
// fences waited for, deferred releases, timestamps read back.
#include "esia/render/renderer.hpp"
#include "esia/rhi/vulkan.hpp"
#include "esia_test.hpp"
#include "image.hpp"
#include "scenes.hpp"
#include <cstdio>
#include <filesystem>
#include <string>

using namespace esia;
using namespace esia::rhi;

namespace
{
    struct Rendered
    {
        bool available = false, ok = false;
        std::vector<std::uint8_t> pixels;
        std::uint32_t validation = 0;
        GpuProfile profile;
        render::RenderStats stats;
    };

    Rendered Render(const conformance::Scene& scene, bool dynamicRendering, int frames)
    {
        Rendered r;
        HeadlessDesc hd;
        hd.width = scene.width;
        hd.height = scene.height;
        hd.format = scene.format;
        hd.samples = scene.samples;
        vulkan::HeadlessOptions o;
        o.dynamicRendering = dynamicRendering;
        std::string error;
        HeadlessDevice h = vulkan::CreateHeadless(hd, o, error);
        if (!h.device)
        {
            std::printf("  (no Vulkan device: %s)\n", error.c_str());
            return r;
        }
        r.available = true;
        {
            render::Renderer renderer(*h.device);
            r.ok = true;
            for (int f = 0; f < frames && r.ok; ++f)
            {
                conformance::SceneFrame frame;   // a new frame each time: textures are created, used and released
                conformance::BuildScene(scene, frame);
                r.ok = renderer.Render(frame.data, &frame.textures, h.target);
            }
            r.ok = r.ok && h.device->ReadPixels(h.target, IRect{0, 0, scene.width, scene.height}, r.pixels);
            r.stats = renderer.Stats();
            h.device->ReadProfile(r.profile);
        }
        r.validation = vulkan::ValidationMessages(*h.device);
        return r;
    }

    void Save(const conformance::Scene& scene, const std::vector<std::uint8_t>& pixels, const char* suffix)
    {
        std::filesystem::create_directories(ESIA_VULKAN_TEST_OUT);
        testkit::Image image(scene.width, scene.height);
        image.rgba = pixels;
        testkit::WritePng(std::string(ESIA_VULKAN_TEST_OUT) + "/" + scene.name + suffix + ".png", image);
    }
}

ESIA_TEST(VulkanScenes, BothRenderPathsAgree)
{
    for (const conformance::Scene& scene : conformance::Scenes())
    {
        const Rendered a = Render(scene, true, 1);
        if (!a.available)
            return;
        const Rendered b = Render(scene, false, 1);
        ESIA_CHECK(a.ok && b.ok);
        ESIA_CHECK(a.validation == 0 && b.validation == 0);
        if (a.pixels != b.pixels)
        {
            std::printf("  %s: dynamic rendering and render passes differ\n", scene.name);
            Save(scene, a.pixels, "-dynamic");
            Save(scene, b.pixels, "-renderpass");
        }
        ESIA_CHECK(a.pixels == b.pixels);
    }
}

// Five frames on one device (two frames in flight) end in the same image as one frame, and the GPU times of the
// finished frames come back.
ESIA_TEST(VulkanScenes, FramesInFlight)
{
    for (const char* name : {"glass", "glow_layer", "windows"})
    {
        const conformance::Scene* scene = conformance::FindScene(name);
        const Rendered one = Render(*scene, true, 1);
        if (!one.available)
            return;
        const Rendered five = Render(*scene, true, 5);
        ESIA_CHECK(one.ok && five.ok);
        ESIA_CHECK(one.pixels == five.pixels);
        ESIA_CHECK(five.validation == 0);
        ESIA_CHECK(five.profile.valid && five.profile.totalMs > 0.0f);
        ESIA_CHECK(five.stats.gpu.valid);   // the renderer read it too (a frame or more behind)
        float sum = 0.0f;
        for (float ms : five.profile.categoryMs)
            sum += ms;
        const float* c = five.profile.categoryMs;
        std::printf("  %-10s GPU %.2f ms: capture %.2f, layer %.2f, fx %.2f, fx-glass %.2f, geometry %.2f\n", name, five.profile.totalMs, c[0], c[1], c[2],
                    c[3], c[4]);
        ESIA_CHECK(sum > 0.0f && sum <= five.profile.totalMs * 1.01f);
    }
}
