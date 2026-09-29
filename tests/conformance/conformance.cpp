// Esia conformance suite - renders every scene (scenes.cpp) with the RHI backends built into this binary and
// compares the result with the goldens (docs/backends/README.md, "Conformance suite").
//
//   esia_conformance [--backend NAME]... [--scene NAME]... [--golden DIR] [--out DIR] [--update] [--strict] [--list]
//
// Backends with readback (Caps::readback) render into a headless target, read it back and compare it with
// <golden>/<scene>.png within the scene's tolerance. The null backend cannot draw: its recorded command stream
// is compared with <golden>/null/<scene>.log line by line, and every RHI contract violation it detected fails the
// scene - that is how the renderer and the scenes are tested without a GPU.
//
// --update writes the goldens from this run instead of comparing (images: only for scenes that own theirs).
// A missing golden is reported as NO GOLDEN and fails only with --strict: the image goldens are produced by the
// first backend that runs here (OpenGL on Mesa llvmpipe), backends developed in parallel compare later.
// --out writes every rendered image (and, on a mismatch, <scene>.diff.png) or command log for inspection.
#include "esia/render/renderer.hpp"
#include "esia/rhi/backend_registry.hpp"
#include "esia/rhi/null_device.hpp"
#include "image.hpp"
#include "scenes.hpp"
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace esia;
using namespace esia::conformance;

namespace
{
    struct Options
    {
        std::vector<std::string> backends, scenes;
        std::string golden, out;
        bool update = false, strict = false, list = false;
    };

    enum class Result { Pass, Fail, Skip, NoGolden, Updated };

    const char* ResultName(Result r)
    {
        switch (r)
        {
        case Result::Pass: return "PASS";
        case Result::Fail: return "FAIL";
        case Result::Skip: return "SKIP";
        case Result::NoGolden: return "NO GOLDEN";
        case Result::Updated: return "UPDATED";
        }
        return "?";
    }

    bool ReadLines(const fs::path& path, std::vector<std::string>& lines)
    {
        std::ifstream in(path);
        if (!in)
            return false;
        lines.clear();
        std::string l;
        while (std::getline(in, l))
            lines.push_back(l);
        return true;
    }

    bool WriteLines(const fs::path& path, const std::vector<std::string>& lines)
    {
        fs::create_directories(path.parent_path());
        std::ofstream out(path, std::ios::binary);
        for (const std::string& l : lines)
            out << l << '\n';
        return (bool)out;
    }

    // The null backend: command stream against its golden log, and no contract violation.
    Result CheckLog(const Options& o, const Scene& scene, const rhi::NullDevice& dev, std::string& detail)
    {
        const std::vector<std::string>& log = dev.Log();
        if (!o.out.empty())
            WriteLines(fs::path(o.out) / "null" / (std::string(scene.name) + ".log"), log);
        if (!dev.Errors().empty())
        {
            detail = std::to_string(dev.Errors().size()) + " RHI contract violation(s), first: " + dev.Errors()[0];
            return Result::Fail;
        }
        const fs::path golden = fs::path(o.golden) / "null" / (std::string(scene.name) + ".log");
        if (o.update)
            return WriteLines(golden, log) ? Result::Updated : Result::Fail;
        std::vector<std::string> want;
        if (!ReadLines(golden, want))
        {
            detail = golden.string();
            return Result::NoGolden;
        }
        for (std::size_t i = 0; i < std::max(want.size(), log.size()); ++i)
        {
            const std::string a = i < log.size() ? log[i] : "<end>", b = i < want.size() ? want[i] : "<end>";
            if (a != b)
            {
                detail = "line " + std::to_string(i + 1) + ": got '" + a + "', golden '" + b + "'";
                return Result::Fail;
            }
        }
        detail = std::to_string(log.size()) + " commands";
        return Result::Pass;
    }

    // A drawing backend: pixels against the golden image, within the scene's tolerance.
    Result CheckImage(const Options& o, const std::string& backend, const Scene& scene, rhi::Device& dev, rhi::Texture target, std::string& detail)
    {
        std::vector<std::uint8_t> pixels;
        if (!dev.ReadPixels(target, rhi::IRect{0, 0, scene.width, scene.height}, pixels) ||
            pixels.size() != (std::size_t)scene.width * (std::size_t)scene.height * 4)
        {
            detail = "ReadPixels failed";
            return Result::Fail;
        }
        testkit::Image image(scene.width, scene.height);
        image.rgba = std::move(pixels);
        const fs::path outDir = o.out.empty() ? fs::path() : fs::path(o.out) / backend;
        if (!o.out.empty())
        {
            fs::create_directories(outDir);
            testkit::WritePng((outDir / (std::string(scene.name) + ".png")).string(), image);
        }
        const std::string goldenName = scene.golden ? scene.golden : scene.name;
        const fs::path golden = fs::path(o.golden) / (goldenName + ".png");
        if (o.update && !scene.golden)
        {
            fs::create_directories(golden.parent_path());
            std::string error;
            if (!testkit::WritePng(golden.string(), image, &error))
            {
                detail = error;
                return Result::Fail;
            }
            return Result::Updated;
        }
        testkit::Image want;
        std::string error;
        if (!testkit::ReadPng(golden.string(), want, &error))
        {
            detail = golden.string() + ": " + error;
            return Result::NoGolden;
        }
        testkit::Image diff;
        const testkit::CompareResult r = testkit::Compare(image, want, scene.tolerance, &diff);
        if (!r.sizeMatches)
        {
            detail = "golden is " + std::to_string(want.width) + "x" + std::to_string(want.height);
            return Result::Fail;
        }
        char buf[160];
        std::snprintf(buf, sizeof(buf), "max delta %d, %.3f%% pixels over %d (allowed %.2f%%), mean delta %.3f", r.maxDelta, r.fraction * 100.0,
                      scene.tolerance.channel, scene.tolerance.fraction * 100.0, r.meanDelta);
        detail = buf;
        if (!r.pass && !o.out.empty())
            testkit::WritePng((outDir / (std::string(scene.name) + ".diff.png")).string(), diff);
        return r.pass ? Result::Pass : Result::Fail;
    }

    Result RunScene(const Options& o, const rhi::BackendInfo& backend, const Scene& scene, std::string& detail)
    {
        rhi::HeadlessDesc hd;
        hd.width = scene.width;
        hd.height = scene.height;
        hd.format = scene.format;
        hd.samples = scene.samples;
        hd.sampleable = true;
        std::string error;
        rhi::HeadlessDevice h = backend.createHeadless(hd, error);
        if (!h.device || !h.target)
        {
            detail = error.empty() ? "no headless device" : error;
            return Result::Skip;
        }
        const rhi::Caps& caps = h.device->GetCaps();
        if (scene.needsDualSource && !caps.dualSourceBlend)
        {
            detail = "no dual-source blending";
            return Result::Skip;
        }
        auto* null = dynamic_cast<rhi::NullDevice*>(h.device.get());
        if (!caps.readback && !null)
        {
            detail = "the backend has no readback (Caps::readback)";
            return Result::Fail;
        }
        SceneFrame frame;
        BuildScene(scene, frame);
        render::Renderer renderer(*h.device);
        if (!renderer.Render(frame.data, &frame.textures, h.target))
        {
            detail = "Renderer::Render refused the frame";
            return Result::Fail;
        }
        return null ? CheckLog(o, scene, *null, detail) : CheckImage(o, backend.name, scene, *h.device, h.target, detail);
    }

    bool Selected(const std::vector<std::string>& list, const char* name)
    {
        if (list.empty())
            return true;
        for (const std::string& s : list)
            if (s == name)
                return true;
        return false;
    }
}

int main(int argc, char** argv)
{
    Options o;
    o.golden = "golden";
    for (int i = 1; i < argc; ++i)
    {
        const std::string a = argv[i];
        auto value = [&]() -> std::string {
            if (i + 1 >= argc)
            {
                std::fprintf(stderr, "%s needs a value\n", a.c_str());
                std::exit(2);
            }
            return argv[++i];
        };
        if (a == "--backend")
            o.backends.push_back(value());
        else if (a == "--scene")
            o.scenes.push_back(value());
        else if (a == "--golden")
            o.golden = value();
        else if (a == "--out")
            o.out = value();
        else if (a == "--update")
            o.update = true;
        else if (a == "--strict")
            o.strict = true;
        else if (a == "--list")
            o.list = true;
        else
        {
            std::fprintf(stderr, "usage: esia_conformance [--backend NAME]... [--scene NAME]... [--golden DIR] [--out DIR] [--update] [--strict] [--list]\n");
            return 2;
        }
    }

    rhi::RegisterBuiltinBackends();
    if (o.list)
    {
        std::printf("backends:");
        for (const rhi::BackendInfo& b : rhi::Backends())
            std::printf(" %s", b.name);
        std::printf("\nscenes:\n");
        for (const Scene& s : Scenes())
            std::printf("  %-13s %dx%d %s%s  %s\n", s.name, s.width, s.height, rhi::FormatName(s.format), s.samples > 1 ? " msaa" : "", s.covers);
        return 0;
    }
    for (const std::string& s : o.scenes)
        if (!FindScene(s.c_str()))
        {
            std::fprintf(stderr, "unknown scene '%s' (--list)\n", s.c_str());
            return 2;
        }

    int failed = 0, run = 0;
    for (const rhi::BackendInfo& backend : rhi::Backends())
    {
        if (!Selected(o.backends, backend.name))
            continue;
        for (const Scene& scene : Scenes())
        {
            if (!Selected(o.scenes, scene.name))
                continue;
            std::string detail;
            const Result r = RunScene(o, backend, scene, detail);
            ++run;
            if (r == Result::Fail || (r == Result::NoGolden && o.strict))
                ++failed;
            std::printf("%-9s %-8s %-13s %s\n", ResultName(r), backend.name, scene.name, detail.c_str());
        }
    }
    for (const std::string& b : o.backends)
        if (!rhi::FindBackend(b))
        {
            std::fprintf(stderr, "backend '%s' is not built into this binary (ESIA_BACKEND_<NAME>)\n", b.c_str());
            ++failed;
        }
    std::printf("%d scene run(s), %d failed\n", run, failed);
    return failed == 0 && run > 0 ? 0 : 1;
}
