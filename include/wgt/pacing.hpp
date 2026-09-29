// WGT UI - frame pacing for hosts (demo, tools, games without their own limiter).
//
//   wgt::FramePacer pacer;
//   pacer.SetTargetFps(120);                  // 0 = unlimited
//   while (running) { ...render...; swapChain->Present(vsync ? 1 : 0, flags); pacer.Wait(); }
//
// Wait() sleeps on a high-resolution waitable timer until shortly before the frame is due, then spins for
// the last fraction of a millisecond: exact frame times without burning a core. Deadlines advance by a
// fixed period (no drift); after a hitch the schedule re-anchors instead of rushing to catch up.
// VSync is the swap chain's job (Present sync interval); the pacer caps below or without it.
#pragma once
#include "config.hpp"
#include <cstdint>

namespace wgt
{
    class WGT_API FramePacer
    {
    public:
        FramePacer();
        ~FramePacer();
        FramePacer(const FramePacer&) = delete;
        FramePacer& operator=(const FramePacer&) = delete;

        void SetTargetFps(float fps);     // 0 (or less) = no cap
        float TargetFps() const { return targetFps_; }

        // Blocks until the next frame is due. Call once per frame, after presenting.
        void Wait();

        float FrameMs() const { return frameMs_; }      // duration of the last frame (ms)
        float AverageFps() const { return avgFps_; }    // smoothed over ~0.5 s

    private:
        void* timer_ = nullptr;
        std::int64_t freq_ = 0;
        std::int64_t next_ = 0;
        std::int64_t last_ = 0;
        float targetFps_ = 0.0f;
        float frameMs_ = 0.0f;
        float avgFps_ = 0.0f;
    };
}
