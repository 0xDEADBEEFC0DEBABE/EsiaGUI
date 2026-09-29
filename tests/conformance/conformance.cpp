// Esia conformance suite - renders every scene (scenes.cpp) with the RHI backends built into this binary and
// compares the result with the goldens (docs/backends/README.md, "Conformance suite").
//
//   esia_conformance [--backend NAME]... [--scene NAME]... [--golden DIR] [--out DIR] [--frames N] [--update] [--strict] [--list]
//
// Backends with readback (Caps::readback) render into a headless target, read it back and compare it with
// <golden>/<scene>.png within the scene's tolerance; scenes with Scene::checkCopy then also copy a region of the
// target to another position and compare the copy. The null backend cannot draw: its recorded command stream is
// compared with <golden>/null/<scene>.log line by line, and every RHI contract violation it detected fails the
// scene - that is how the renderer and the scenes are tested without a GPU. A scene also fails when the device
// reports API validation messages (Device::ValidationErrors) after all that, or when its host callbacks did not run
// as often as the scene says.
//
// --frames N (drawing backends): N - 1 frames of loud content (BuildPoisonFrame) on the same device and renderer
// first, then a cleared target and the scene's frame. Undefined or stale surfaces that a fresh device happens to
// hold as zeros (llvmpipe) then show. The null backend's goldens are single frames: it ignores --frames.
// --update writes the goldens from this run instead of comparing (images: only for scenes that own theirs).
// A missing golden is reported as NO GOLDEN and fails only with --strict.
// --out writes every rendered image (and, on a mismatch, <scene>.diff.png) or command log for inspection.
//
// Exit code: 1 when a scene failed (or nothing ran), else 77 when every scene was skipped - no device for the backend
// on this machine; CTest reports that as skipped (SKIP_RETURN_CODE), not passed - else 0.
#include "esia/render/renderer.hpp"
#include "esia/rhi/backend_registry.hpp"
#include "esia/rhi/null_device.hpp"
#include "image.hpp"
#include "scenes.hpp"
#include <cstdio>
#include <cstdlib>
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
        int frames = 1;
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
    Result CheckImage(const Options& o, const std::string& backend, const Scene& scene, const testkit::Image& image, std::string& detail)
    {
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
        char buf[192];
        std::snprintf(buf, sizeof(buf), "max delta %d (allowed %d), %.3f%% pixels over %d (allowed %.2f%%), mean delta %.3f (allowed %.2f)", r.maxDelta,
                      scene.tolerance.maxDelta, r.fraction * 100.0, scene.tolerance.channel, scene.tolerance.fraction * 100.0, r.meanDelta,
                      scene.tolerance.meanDelta);
        detail = buf;
        if (!r.pass && !o.out.empty())
            testkit::WritePng((outDir / (std::string(scene.name) + ".diff.png")).string(), diff);
        return r.pass ? Result::Pass : Result::Fail;
    }

    // CopyTexture to another position: half of the target into a second texture (the backdrop copy's usage and raw
    // format) at an offset, a multisampled target resolved on the way, read back and compared with `image`, the
    // target's own pixels. The renderer copies each capture to its own position, so only this check moves one.
    Result CheckCopy(const Scene& scene, rhi::Device& dev, rhi::Texture target, const testkit::Image& image, std::string& detail)
    {
        const rhi::IRect src{13, 19, 13 + scene.width / 2, 19 + scene.height / 2};
        const int dstX = scene.width - src.Width() - 3, dstY = scene.height - src.Height() - 11;
        rhi::TextureDesc d;
        d.width = scene.width;
        d.height = scene.height;
        d.format = rhi::RawFormat(scene.format);
        d.usage = rhi::TextureUsage_Sampled | rhi::TextureUsage_CopyDst;
        d.debugName = "copy-check";
        const rhi::Texture copy = dev.CreateTexture(d);
        bool ok = copy && dev.BeginFrame(rhi::FrameDesc());
        if (ok)
        {
            dev.CopyTexture(copy, dstX, dstY, target, src);
            dev.EndFrame();
        }
        std::vector<std::uint8_t> pixels;
        ok = ok && dev.ReadPixels(copy, rhi::IRect{dstX, dstY, dstX + src.Width(), dstY + src.Height()}, pixels) &&
             pixels.size() == (std::size_t)src.Width() * (std::size_t)src.Height() * 4;
        if (copy)
            dev.DestroyTexture(copy);
        if (!ok)
        {
            detail = "copy check: CopyTexture to an offset could not be created, recorded or read back";
            return Result::Fail;
        }
        testkit::Image got(src.Width(), src.Height()), want(src.Width(), src.Height());
        got.rgba = std::move(pixels);
        for (int y = 0; y < src.Height(); ++y)
            std::memcpy(want.At(0, y), image.At(src.x0, src.y0 + y), (std::size_t)src.Width() * 4);
        // the bits are copied; only resolves (in the copy and in ReadPixels) may round differently where a pixel's
        // samples differ
        testkit::Tolerance t;
        t.channel = t.maxDelta = scene.samples > 1 ? 2 : 0;
        t.fraction = 0.0;
        const testkit::CompareResult r = testkit::Compare(got, want, t);
        if (!r.pass)
        {
            char buf[160];
            std::snprintf(buf, sizeof(buf), "copy check: (%d, %d) %dx%d copied to (%d, %d) differs from the target: max delta %d, %d pixels", src.x0, src.y0,
                          src.Width(), src.Height(), dstX, dstY, r.maxDelta, (int)r.differing);
            detail = buf;
            return Result::Fail;
        }
        detail += ", offset copy matches";
        return Result::Pass;
    }

    // An empty frame that clears the target (the cross-frame mode's scene frame starts from what a fresh one holds).
    bool ClearTarget(rhi::Device& dev, rhi::Texture target)
    {
        if (!dev.BeginFrame(rhi::FrameDesc()))
            return false;
        rhi::PassDesc pass;
        pass.target = target;
        pass.load = rhi::LoadOp::Clear;
        pass.debugName = "clear";
        dev.BeginPass(pass);
        dev.EndPass();
        dev.EndFrame();
        return true;
    }

    Result RenderAndCheck(const Options& o, const rhi::BackendInfo& backend, const Scene& scene, rhi::HeadlessDevice& h, std::string& detail)
    {
        rhi::Device& dev = *h.device;
        auto* null = dynamic_cast<rhi::NullDevice*>(&dev);
        if (!dev.GetCaps().readback && !null)
        {
            detail = "the backend has no readback (Caps::readback)";
            return Result::Fail;
        }
        render::Renderer renderer(dev);
        const render::RenderParams params = RenderParamsOf(scene);
        const int poison = null ? 0 : o.frames - 1;
        for (int i = 0; i < poison; ++i)
        {
            SceneFrame frame;
            BuildPoisonFrame(scene, frame);
            if (!renderer.Render(frame.data, nullptr, h.target, params))
            {
                detail = "Renderer::Render refused poison frame " + std::to_string(i + 1);
                return Result::Fail;
            }
        }
        if (poison > 0 && !ClearTarget(dev, h.target))
        {
            detail = "BeginFrame failed (clearing the target)";
            return Result::Fail;
        }
        SceneFrame frame;
        BuildScene(scene, frame);
        if (!renderer.Render(frame.data, &frame.textures, h.target, params))
        {
            detail = "Renderer::Render refused the frame";
            return Result::Fail;
        }
        if (frame.callbacks != scene.callbacks)
        {
            detail = std::to_string(frame.callbacks) + " host callback(s) ran, " + std::to_string(scene.callbacks) + " expected";
            return Result::Fail;
        }
        if (null)
            return CheckLog(o, scene, *null, detail);
        std::vector<std::uint8_t> pixels;
        if (!dev.ReadPixels(h.target, rhi::IRect{0, 0, scene.width, scene.height}, pixels) ||
            pixels.size() != (std::size_t)scene.width * (std::size_t)scene.height * 4)
        {
            detail = "ReadPixels failed";
            return Result::Fail;
        }
        testkit::Image image(scene.width, scene.height);
        image.rgba = std::move(pixels);
        const Result r = CheckImage(o, backend.name, scene, image, detail);
        return r == Result::Pass && scene.checkCopy ? CheckCopy(scene, dev, h.target, image, detail) : r;
    }

    Result RunScene(const Options& o, const rhi::BackendInfo& backend, const Scene& scene, std::string& detail)
    {
        std::string error;
        rhi::HeadlessDevice h = backend.createHeadless(HeadlessDescOf(scene), error);
        if (!h.device || !h.target)
        {
            detail = error.empty() ? "no headless device" : error;
            return Result::Skip;
        }
        const Result r = RenderAndCheck(o, backend, scene, h, detail);
        // counted after the readback: reading back is checked too
        if (const std::uint32_t messages = h.device->ValidationErrors())
        {
            detail = std::to_string(messages) + " API validation message(s) (Device::ValidationErrors); " + detail;
            return Result::Fail;
        }
        return r;
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
        else if (a == "--frames")
        {
            o.frames = std::atoi(value().c_str());
            if (o.frames < 1)
            {
                std::fprintf(stderr, "--frames needs a number >= 1\n");
                return 2;
            }
        }
        else if (a == "--update")
            o.update = true;
        else if (a == "--strict")
            o.strict = true;
        else if (a == "--list")
            o.list = true;
        else
        {
            std::fprintf(stderr, "usage: esia_conformance [--backend NAME]... [--scene NAME]... [--golden DIR] [--out DIR] [--frames N] [--update] [--strict] "
                                 "[--list]\n");
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
            std::printf("  %-16s %dx%d %s%s%s  %s\n", s.name, s.width, s.height, rhi::FormatName(s.format), s.samples > 1 ? " msaa" : "",
                        s.sampleable ? "" : " unsampled", s.covers);
        return 0;
    }
    for (const std::string& s : o.scenes)
        if (!FindScene(s.c_str()))
        {
            std::fprintf(stderr, "unknown scene '%s' (--list)\n", s.c_str());
            return 2;
        }

    int failed = 0, run = 0, skipped = 0;
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
            if (r == Result::Skip)
                ++skipped;
            std::printf("%-9s %-8s %-16s %s\n", ResultName(r), backend.name, scene.name, detail.c_str());
        }
    }
    for (const std::string& b : o.backends)
        if (!rhi::FindBackend(b))
        {
            std::fprintf(stderr, "backend '%s' is not built into this binary (ESIA_BACKEND_<NAME>)\n", b.c_str());
            ++failed;
        }
    std::printf("%d scene run(s), %d failed, %d skipped\n", run, failed, skipped);
    if (failed > 0 || run == 0)
        return 1;
    return skipped == run ? 77 : 0;
}
