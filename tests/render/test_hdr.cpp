// HDR output (RenderParams::hdr) on every backend built in: the numbers of the encodings - opaque sRGB grays drawn
// into an scRGB and a PQ BT.2020 target come out as the transfer functions say - and the decode: a translucent rect
// over what the last frame left blends with it as on an SDR target, and nothing outside the region the frame covers
// changes.
#include "esia/render/painter.hpp"
#include "esia/render/renderer.hpp"
#include "esia/rhi/backend_registry.hpp"
#include "esia_test.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace esia;

namespace
{
    double SrgbToLinear(double v) { return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); }

    double NitsToPq(double nits)
    {
        const double m1 = 0.1593017578125, m2 = 78.84375, c1 = 0.8359375, c2 = 18.8515625, c3 = 18.6875;
        const double y = std::pow(std::clamp(nits / 10000.0, 0.0, 1.0), m1);
        return std::pow((c1 + c2 * y) / (1.0 + c3 * y), m2);
    }

    // The 8 bits ReadPixels gives for an opaque sRGB gray under an encoding (gray stays gray from BT.709 to BT.2020).
    int Expected(double gray, render::HdrEncoding e, double paperWhite)
    {
        const double nits = SrgbToLinear(gray) * paperWhite;
        const double v = e == render::HdrEncoding::ScRgb ? nits / 80.0 : std::round(NitsToPq(nits) * 1023.0) / 1023.0;
        return (int)std::lround(std::clamp(v, 0.0, 1.0) * 255.0);
    }

    struct Case
    {
        const char* name;
        rhi::Format format;
        render::HdrEncoding encoding;
    };
    const Case kCases[] = {{"scRGB", rhi::Format::RGBA16_FLOAT, render::HdrEncoding::ScRgb}, {"PQ", rhi::Format::RGB10A2_UNORM, render::HdrEncoding::Pq2020}};
    constexpr float kPaperWhite = 203.0f;
    constexpr int kW = 64, kH = 16;

    DrawData Frame(const DrawList& dl)
    {
        DrawData dd;
        dd.lists.push_back(&dl);
        dd.displaySize = Vec2((float)kW, (float)kH);
        return dd;
    }

    int Channel(const std::vector<std::uint8_t>& rgba, int x, int y, int c) { return rgba[((std::size_t)y * kW + (std::size_t)x) * 4 + (std::size_t)c]; }

    bool Near(int got, int want, int tolerance = 2) { return std::abs(got - want) <= tolerance; }

    // The devices the tests ran on in this process. With none (no device of the backend on this machine) the line at
    // exit makes CTest report the test as skipped (SKIP_REGULAR_EXPRESSION), not passed, as the conformance suite's
    // exit code 77 does.
    struct DevicesRun
    {
        int count = 0;
        ~DevicesRun()
        {
            if (count == 0)
                std::printf("esia_hdr: no device ran\n");
        }
    } gDevicesRun;

    // The devices to run on: every backend built in (or the one ESIA_HDR_BACKEND names: CTest runs a test per backend)
    // that makes a headless device of the format, reads back and has float render targets.
    template <class F>
    void ForEachDevice(rhi::Format format, const char* what, F&& f)
    {
        rhi::RegisterBuiltinBackends();
        const char* only = std::getenv("ESIA_HDR_BACKEND");
        for (const rhi::BackendInfo& b : rhi::Backends())
        {
            if (std::string(b.name) == "null" || !b.createHeadless || (only && *only && std::string(b.name) != only))
                continue;
            rhi::HeadlessDesc hd;
            hd.width = kW;
            hd.height = kH;
            hd.format = format;
            std::string error;
            rhi::HeadlessDevice dev = b.createHeadless(hd, error);
            if (!dev.device || !dev.device->GetCaps().readback || !dev.device->GetCaps().floatRenderTargets)
            {
                std::printf("  %s %s: skipped (%s)\n", b.name, what, dev.device ? "no readback or float targets" : error.c_str());
                continue;
            }
            ++gDevicesRun.count;
            f(b.name, dev);
        }
    }
}

// Four opaque grays (sRGB 0, 0.25, 0.5, 1) side by side: in scRGB 1.0 is 80 nits, so paper white at 203 nits takes
// sRGB 0.5 to 0.543 and white past 1 (read back as 255); in PQ the code values of their nits.
ESIA_TEST(Hdr, OpaqueGraysEncodeAsTheTransferFunctionsSay)
{
    const double grays[4] = {0.0, 0.25, 0.5, 1.0};
    DrawList dl;
    dl.Reset(Rect(0, 0, (float)kW, (float)kH));
    {
        Painter p(dl);
        for (int i = 0; i < 4; ++i)
            p.Rect(Rect(16.0f * (float)i, 0, 16.0f * (float)(i + 1), (float)kH), Style().Fill(Color((float)grays[i], (float)grays[i], (float)grays[i], 1.0f)));
    }
    for (const Case& c : kCases)
        ForEachDevice(c.format, c.name, [&](const std::string& backend, rhi::HeadlessDevice& dev) {
            render::Renderer r(*dev.device);
            render::RenderParams params;
            params.hdr.encoding = c.encoding;
            params.hdr.paperWhiteNits = kPaperWhite;
            std::vector<std::uint8_t> px;
            ESIA_CHECK(r.Render(Frame(dl), nullptr, dev.target, params));
            ESIA_CHECK(dev.device->ReadPixels(dev.target, rhi::IRect{0, 0, kW, kH}, px) && px.size() == (std::size_t)kW * kH * 4);
            if (px.size() != (std::size_t)kW * kH * 4)
                return;
            for (int i = 0; i < 4; ++i)
            {
                const int want = Expected(grays[i], c.encoding, kPaperWhite);
                for (int ch = 0; ch < 3; ++ch)
                {
                    const int got = Channel(px, 16 * i + 8, kH / 2, ch);
                    if (!Near(got, want))
                        std::printf("  %s %s: gray %.2f channel %d = %d, want %d\n", backend.c_str(), c.name, grays[i], ch, got, want);
                    ESIA_CHECK(Near(got, want));
                }
            }
        });
}

// The decode: a frame of sRGB 0.5 gray, then one with half-transparent white over the left half - in the working
// target that is 0.5 * 0.5 + 0.5 = 0.75, as on an SDR target, read from what the first frame left in the target's
// encoding. The right half is outside the second frame's region: what the first frame left, to the bit.
ESIA_TEST(Hdr, TheTargetsPixelsComeBackForTheUiToBlendWith)
{
    DrawList base, over;
    base.Reset(Rect(0, 0, (float)kW, (float)kH));
    over.Reset(Rect(0, 0, (float)kW, (float)kH));
    {
        Painter p(base);
        p.Rect(Rect(0, 0, (float)kW, (float)kH), Style().Fill(Color(0.5f, 0.5f, 0.5f, 1.0f)));
    }
    {
        Painter p(over);
        p.Rect(Rect(0, 0, (float)kW / 2.0f, (float)kH), Style().Fill(Color(1.0f, 1.0f, 1.0f, 0.5f)));
    }
    for (const Case& c : kCases)
        ForEachDevice(c.format, c.name, [&](const std::string& backend, rhi::HeadlessDevice& dev) {
            render::Renderer r(*dev.device);
            render::RenderParams params;
            params.hdr.encoding = c.encoding;
            params.hdr.paperWhiteNits = kPaperWhite;
            std::vector<std::uint8_t> first, second;
            ESIA_CHECK(r.Render(Frame(base), nullptr, dev.target, params));
            ESIA_CHECK(dev.device->ReadPixels(dev.target, rhi::IRect{0, 0, kW, kH}, first));
            ESIA_CHECK(r.Render(Frame(over), nullptr, dev.target, params));
            ESIA_CHECK(dev.device->ReadPixels(dev.target, rhi::IRect{0, 0, kW, kH}, second));
            if (first.size() != (std::size_t)kW * kH * 4 || second.size() != first.size())
                return;
            const int blended = Expected(0.75, c.encoding, kPaperWhite);
            for (int ch = 0; ch < 3; ++ch)
            {
                const int left = Channel(second, kW / 4, kH / 2, ch), right = Channel(second, kW * 3 / 4, kH / 2, ch);
                if (!Near(left, blended) || right != Channel(first, kW * 3 / 4, kH / 2, ch))
                    std::printf("  %s %s: channel %d left %d (want %d), right %d (was %d)\n", backend.c_str(), c.name, ch, left, blended, right,
                                Channel(first, kW * 3 / 4, kH / 2, ch));
                ESIA_CHECK(Near(left, blended));
                ESIA_CHECK(right == Channel(first, kW * 3 / 4, kH / 2, ch));
            }
        });
}
