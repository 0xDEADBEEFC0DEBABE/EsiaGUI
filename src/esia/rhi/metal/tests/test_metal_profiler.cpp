// Metal backend: timestamp calibration, sample assignment and the per-category split.
// UNVERIFIED: needs macOS (passes on Linux and under Wine).
#include "esia_test.hpp"
#include "metal_profiler.hpp"
#include "metal_tables.hpp"
#include <map>
#include <vector>

using namespace esia::rhi;
using namespace esia::rhi::metal;

ESIA_TEST(MetalProfiler, ClockCalibration)
{
    TimestampClock c;
    c.AddSample(1000000000, 5000);
    ESIA_CHECK(!c.Ready());
    c.AddSample(1000000500, 5010);   // too short a baseline
    ESIA_CHECK(!c.Ready());
    c.AddSample(1002000000, 5000 + 48000);   // 2 ms over 48000 ticks (a 24 MHz timebase)
    ESIA_CHECK(c.Ready());
    ESIA_CHECK_NEAR(c.NsPerTick(), 2000000.0 / 48000.0, 1e-9);
}

ESIA_TEST(MetalProfiler, ComputeProfile)
{
    std::vector<EncoderTiming> e(3);
    e[0] = {0, 1e6, {}};          // 1 ms capture (blit + pyramid)
    e[0].weight[(int)ProfileCategory::Capture] = 2;
    e[1] = {0.5e6, 3.5e6, {}};    // 3 ms "ui" pass overlapping it: 2 glass batches, 1 text draw
    e[1].weight[(int)ProfileCategory::FxGlass] = 2;
    e[1].weight[(int)ProfileCategory::Geometry] = 1;
    e[2] = {5e6, 5.5e6, {}};      // uploads: no scope
    const GpuProfile p = ComputeProfile(7, e);
    ESIA_CHECK(p.valid && p.frame == 7);
    ESIA_CHECK_NEAR(p.totalMs, 3.5 + 0.5, 1e-4);   // union, not the sum
    ESIA_CHECK_NEAR(p.categoryMs[(int)ProfileCategory::Capture], 1.0, 1e-4);
    ESIA_CHECK_NEAR(p.categoryMs[(int)ProfileCategory::FxGlass], 2.0, 1e-4);
    ESIA_CHECK_NEAR(p.categoryMs[(int)ProfileCategory::Geometry], 1.0, 1e-4);
    ESIA_CHECK_NEAR(p.categoryMs[(int)ProfileCategory::Layer], 0.0, 0);
}

ESIA_TEST(MetalProfiler, RecorderLifecycle)
{
    ProfileRecorder r;
    TimestampClock clock;
    clock.AddSample(0, 0);
    clock.AddSample(10000000, 10000000);   // 1 ns per tick

    // frame 1: an upload blit, then a render pass with a Fx and a Geometry draw
    r.BeginFrame(1, 1, 5);
    EncoderSamples s;
    ESIA_CHECK(r.NextEncoder(false, s) && s.buffer == 5 && s.count == 2 && s.index[0] == 0 && s.index[1] == 1 && s.index[2] == ~0ull);
    r.Work();   // outside a scope
    ESIA_CHECK(r.NextEncoder(true, s) && s.count == 4 && s.index[0] == 2 && s.index[3] == 5);
    r.Scope(ProfileCategory::Fx);
    r.Work();
    r.Scope(ProfileCategory::Geometry);
    r.Work();
    r.EndScope();
    r.EndFrame();
    ESIA_CHECK(r.PendingFrames() == 1);

    // frame 2 without a counter buffer: not profiled
    r.BeginFrame(2, 2, 0);
    ESIA_CHECK(!r.NextEncoder(true, s) && s.buffer == 0);
    r.EndFrame();
    ESIA_CHECK(r.PendingFrames() == 1);

    // timestamps: blit 100..200, render vertex 150..300 / fragment 250..1100 (ticks = ns)
    const std::uint64_t ticks[] = {1100, 1200, 1150, 1300, 1250, 2100};
    std::vector<std::uint32_t> released;
    auto resolve = [&](std::uint32_t buf, std::uint32_t first, std::uint32_t count, std::uint64_t* out) {
        ESIA_CHECK(buf == 5 && first == 0 && count == 6);
        for (std::uint32_t i = 0; i < count; ++i)
            out[i] = ticks[first + i];
        return true;
    };
    auto release = [&](std::uint32_t b) { released.push_back(b); };
    GpuProfile p;
    r.Poll(0, clock, resolve, release);   // not completed yet
    ESIA_CHECK(!r.Latest(p) && released.empty());
    r.Poll(1, clock, resolve, release);
    ESIA_CHECK(r.Latest(p) && p.frame == 1 && released.size() == 1 && released[0] == 5 && r.PendingFrames() == 0);
    ESIA_CHECK_NEAR(p.totalMs, 1000e-6, 1e-7);                                  // 1100 .. 2100
    ESIA_CHECK_NEAR(p.categoryMs[(int)ProfileCategory::Fx], 475e-6, 1e-7);        // render 1150 .. 2100, half each
    ESIA_CHECK_NEAR(p.categoryMs[(int)ProfileCategory::Geometry], 475e-6, 1e-7);

    // a counter the GPU could not sample is skipped, not read as time 0
    r.BeginFrame(3, 3, 6);
    r.NextEncoder(true, s);
    r.Scope(ProfileCategory::Layer);
    r.Work();
    r.EndFrame();
    const std::uint64_t bad[] = {mtl::kCounterErrorValue, mtl::kCounterErrorValue, 3000, 3500};
    r.Poll(3, clock,
           [&](std::uint32_t, std::uint32_t first, std::uint32_t count, std::uint64_t* out) {
               for (std::uint32_t i = 0; i < count; ++i)
                   out[i] = bad[first + i];
               return true;
           },
           release);
    ESIA_CHECK(r.Latest(p) && p.frame == 3);
    ESIA_CHECK_NEAR(p.categoryMs[(int)ProfileCategory::Layer], 500e-6, 1e-7);

    // a frame with more encoders than samples reports nothing (a partial total would lie)
    r.BeginFrame(4, 4, 7);
    int encoders = 0;
    while (r.NextEncoder(true, s))
        ++encoders;
    ESIA_CHECK(encoders == (int)ProfileRecorder::kSamplesPerFrame / 4);
    r.EndFrame();
    r.Poll(4, clock, resolve, release);
    ESIA_CHECK(r.Latest(p) && p.frame == 3 && released.back() == 7);

    // teardown hands back the buffers of frames still in flight
    r.BeginFrame(5, 5, 8);
    r.EndFrame();
    r.Drop(release);
    ESIA_CHECK(released.back() == 8 && r.PendingFrames() == 0);
}
