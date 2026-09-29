// Esia - Metal backend: GPU timing from counter sample buffers, without Apple headers.
//
// Apple GPUs sample timestamps only at encoder (stage) boundaries (MTLCounterSamplingPointAtStageBoundary), not
// between draws, while the renderer's profile scopes (rhi::ProfileCategory) change inside a render pass: glass
// batches, plain FX batches and text share the "ui" pass. So:
//   * every encoder of a profiled frame gets timestamp samples (render: start / end of the vertex and the fragment
//     stage; blit: start / end of the encoder) and its interval is [earliest start, latest end];
//   * GpuProfile::totalMs is the union of those intervals (encoders overlap on tile-based GPUs);
//   * each encoder's time is split between the categories of the draws and copies it executed, in proportion to
//     their number - an approximation for mixed passes, exact for captures and layer pyramids (encoders of one
//     category). Work outside every scope (uploads) counts in the total only.
// Results are read a few frames later, when the frame's command buffer completed; nothing waits for the GPU.
//
// Plain C++, unit-tested on every host (tests/test_metal_profiler.cpp). UNVERIFIED: needs macOS - in particular
// the tick -> nanosecond calibration (TimestampClock) follows Apple's "Converting GPU timestamps into CPU time" and
// was never compared with a real GPU capture.
#pragma once
#include "esia/rhi/rhi.hpp"
#include <cstdint>
#include <functional>
#include <vector>

namespace esia::rhi::metal
{
    // GPU timestamp ticks -> nanoseconds, from pairs of (CPU ns, GPU ticks) taken with
    // -[MTLDevice sampleTimestamps:gpuTimestamp:] at different times.
    class TimestampClock
    {
    public:
        void AddSample(std::uint64_t cpuNs, std::uint64_t gpuTicks);
        bool Ready() const { return ratio_ > 0.0; }
        double NsPerTick() const { return ratio_; }

    private:
        bool first_ = true;
        std::uint64_t cpu0_ = 0, gpu0_ = 0;
        double ratio_ = 0.0;
    };

    struct EncoderSamples
    {
        std::uint32_t buffer = 0;   // counter sample buffer id, 0 = not sampled
        int count = 0;              // 4 for a render encoder, 2 for a blit encoder
        std::uint64_t index[4] = {~0ull, ~0ull, ~0ull, ~0ull};   // mtl::kCounterDontSample when unused
    };

    class ProfileRecorder
    {
    public:
        static constexpr std::uint32_t kSamplesPerFrame = 2048;   // 8 bytes each: 16 KB per counter buffer

        // `counterBuffer` 0 = this frame is not profiled (no buffer free, or timestamps off).
        void BeginFrame(std::uint64_t serial, std::uint64_t frame, std::uint32_t counterBuffer);
        // Sample indices for the next encoder; false when the frame is not profiled or its buffer is full (the
        // frame then reports no result: a partial total would be wrong).
        bool NextEncoder(bool render, EncoderSamples& out);
        void Scope(ProfileCategory c);
        void EndScope();
        void Work();   // one draw or copy in the current encoder
        void EndFrame();
        bool FrameOpen() const { return open_; }

        // Resolves the frames whose command buffers completed (serial <= completed): `resolve` reads `count`
        // timestamps from `first` of a counter buffer (false on failure), `release` hands the buffer back.
        using ResolveFn = std::function<bool(std::uint32_t buffer, std::uint32_t first, std::uint32_t count, std::uint64_t* out)>;
        using ReleaseFn = std::function<void(std::uint32_t buffer)>;
        void Poll(std::uint64_t completed, const TimestampClock& clock, const ResolveFn& resolve, const ReleaseFn& release);
        // Hands back every buffer of unresolved frames (device teardown).
        void Drop(const ReleaseFn& release);
        bool Latest(GpuProfile& out) const;
        std::size_t PendingFrames() const { return pending_.size(); }

    private:
        struct Encoder
        {
            std::uint32_t first = 0, count = 0;
            float weight[(int)ProfileCategory::Count] = {};
        };
        struct Frame
        {
            std::uint64_t serial = 0, frame = 0;
            std::uint32_t buffer = 0, used = 0;
            bool overflow = false;
            std::vector<Encoder> encoders;
        };
        bool open_ = false;
        Frame cur_;
        int scope_ = -1;
        std::vector<Frame> pending_;
        GpuProfile latest_;
    };

    // The per-category split of one resolved frame (exposed for the tests): intervals in ns.
    struct EncoderTiming
    {
        double start = 0, end = 0;
        float weight[(int)ProfileCategory::Count] = {};
    };
    GpuProfile ComputeProfile(std::uint64_t frame, const std::vector<EncoderTiming>& encoders);
}
