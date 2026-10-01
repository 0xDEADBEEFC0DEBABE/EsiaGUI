// Esia - Metal backend: GPU timing (see metal_profiler.hpp). UNVERIFIED: needs macOS.
#include "metal_profiler.hpp"
#include "metal_tables.hpp"
#include <algorithm>

namespace esia::rhi::metal
{
    void TimestampClock::AddSample(std::uint64_t cpuNs, std::uint64_t gpuTicks)
    {
        if (first_)
        {
            first_ = false;
            cpu0_ = cpuNs;
            gpu0_ = gpuTicks;
            return;
        }
        // the longer the baseline the better the ratio; a millisecond is enough for a first estimate
        if (cpuNs > cpu0_ + 1000000 && gpuTicks > gpu0_)
            ratio_ = (double)(cpuNs - cpu0_) / (double)(gpuTicks - gpu0_);
    }

    void ProfileRecorder::BeginFrame(std::uint64_t serial, std::uint64_t frame, std::uint32_t counterBuffer)
    {
        cur_ = Frame();
        cur_.serial = serial;
        cur_.frame = frame;
        cur_.buffer = counterBuffer;
        scope_ = -1;
        open_ = true;
    }

    bool ProfileRecorder::NextEncoder(bool render, EncoderSamples& out)
    {
        out = EncoderSamples();
        if (!open_ || !cur_.buffer)
            return false;
        const std::uint32_t n = render ? 4 : 2;
        if (cur_.used + n > kSamplesPerFrame)
        {
            cur_.overflow = true;
            return false;
        }
        Encoder e;
        e.first = cur_.used;
        e.count = n;
        cur_.encoders.push_back(e);
        out.buffer = cur_.buffer;
        out.count = (int)n;
        for (std::uint32_t i = 0; i < n; ++i)
            out.index[i] = cur_.used + i;
        cur_.used += n;
        return true;
    }

    void ProfileRecorder::Scope(ProfileCategory c) { scope_ = (int)c; }
    void ProfileRecorder::EndScope() { scope_ = -1; }

    void ProfileRecorder::Work()
    {
        if (open_ && scope_ >= 0 && !cur_.encoders.empty() && !cur_.overflow)
            cur_.encoders.back().weight[scope_] += 1.0f;
    }

    void ProfileRecorder::EndFrame()
    {
        if (!open_)
            return;
        open_ = false;
        if (cur_.buffer)
            pending_.push_back(std::move(cur_));
        cur_ = Frame();
    }

    void ProfileRecorder::Poll(std::uint64_t completed, const TimestampClock& clock, const ResolveFn& resolve, const ReleaseFn& release)
    {
        std::size_t kept = 0;
        std::vector<std::uint64_t> ticks;
        for (std::size_t i = 0; i < pending_.size(); ++i)
        {
            Frame& f = pending_[i];
            if (f.serial > completed)
            {
                if (kept != i)   // a self-move would empty the encoders
                    pending_[kept] = std::move(f);
                ++kept;
                continue;
            }
            ticks.assign(f.used, mtl::kCounterErrorValue);
            const bool ok = !f.overflow && f.used > 0 && clock.Ready() && resolve(f.buffer, 0, f.used, ticks.data());
            release(f.buffer);
            if (!ok)
                continue;
            // intervals relative to the frame's earliest valid tick (doubles keep nanoseconds exact that way)
            std::uint64_t base = ~0ull;
            for (std::uint64_t t : ticks)
                if (t != mtl::kCounterErrorValue && t != 0)
                    base = std::min(base, t);
            std::vector<EncoderTiming> timings;
            for (const Encoder& e : f.encoders)
            {
                // render: [start vertex, end vertex, start fragment, end fragment]; blit: [start, end]
                std::uint64_t s = ~0ull, en = 0;
                for (std::uint32_t k = 0; k < e.count; ++k)
                {
                    const std::uint64_t t = ticks[e.first + k];
                    if (t == mtl::kCounterErrorValue || t == 0)
                        continue;
                    if (k % 2 == 0)
                        s = std::min(s, t);
                    else
                        en = std::max(en, t);
                }
                if (s == ~0ull || en <= s)
                    continue;
                EncoderTiming et;
                et.start = (double)(s - base) * clock.NsPerTick();
                et.end = (double)(en - base) * clock.NsPerTick();
                std::copy(std::begin(e.weight), std::end(e.weight), et.weight);
                timings.push_back(et);
            }
            if (!timings.empty() && f.frame >= latest_.frame)
                latest_ = ComputeProfile(f.frame, timings);
        }
        pending_.resize(kept);
    }

    void ProfileRecorder::Drop(const ReleaseFn& release)
    {
        for (const Frame& f : pending_)
            release(f.buffer);
        pending_.clear();
        if (open_ && cur_.buffer)
            release(cur_.buffer);
        cur_ = Frame();
        open_ = false;
    }

    bool ProfileRecorder::Latest(GpuProfile& out) const
    {
        out = latest_;
        return latest_.valid;
    }

    GpuProfile ComputeProfile(std::uint64_t frame, const std::vector<EncoderTiming>& encoders)
    {
        GpuProfile p;
        p.valid = true;
        p.frame = frame;
        // total: the union of the encoder intervals
        std::vector<std::pair<double, double>> iv;
        for (const EncoderTiming& e : encoders)
            iv.push_back({e.start, e.end});
        std::sort(iv.begin(), iv.end());
        double total = 0, s = 0, e = -1;
        for (const auto& [a, b] : iv)
        {
            if (a > e)
            {
                if (e > s)
                    total += e - s;
                s = a;
                e = b;
            }
            else
                e = std::max(e, b);
        }
        if (e > s)
            total += e - s;
        p.totalMs = (float)(total * 1e-6);
        for (const EncoderTiming& enc : encoders)
        {
            float w = 0;
            for (float x : enc.weight)
                w += x;
            if (w <= 0)
                continue;
            const double ms = (enc.end - enc.start) * 1e-6;
            for (int c = 0; c < (int)ProfileCategory::Count; ++c)
                p.categoryMs[c] += (float)(ms * enc.weight[c] / w);
        }
        return p;
    }
}
