// WGT UI - frame pacer (see wgt/pacing.hpp)
#include "wgt/pacing.hpp"
#include <windows.h>
#include <timeapi.h>
#include <algorithm>

#pragma comment(lib, "winmm.lib")

#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

namespace wgt
{
    namespace
    {
        std::int64_t Now()
        {
            LARGE_INTEGER t;
            QueryPerformanceCounter(&t);
            return t.QuadPart;
        }
    }

    FramePacer::FramePacer()
    {
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        freq_ = f.QuadPart;
        // high-resolution timer (Windows 10 1803+): ~0.5 ms wake-up precision without timeBeginPeriod
        timer_ = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
        if (!timer_)
        {
            timer_ = CreateWaitableTimerExW(nullptr, nullptr, 0, TIMER_ALL_ACCESS);
            timeBeginPeriod(1);
        }
        last_ = next_ = Now();
    }

    FramePacer::~FramePacer()
    {
        if (timer_)
            CloseHandle(timer_);
    }

    void FramePacer::SetTargetFps(float fps)
    {
        targetFps_ = fps > 0.0f ? fps : 0.0f;
        next_ = Now();
    }

    void FramePacer::Wait()
    {
        std::int64_t now = Now();
        if (targetFps_ > 0.0f)
        {
            const std::int64_t period = (std::int64_t)((double)freq_ / (double)targetFps_);
            next_ += period;
            if (now - next_ > period)
                next_ = now;   // we are more than a frame late (hitch, breakpoint): re-anchor, do not rush
            std::int64_t remaining = next_ - now;
            const std::int64_t spinWindow = freq_ / 1000;   // last ~1 ms is spun for precision
            if (remaining > spinWindow && timer_)
            {
                LARGE_INTEGER due;
                due.QuadPart = -(LONGLONG)((remaining - spinWindow) * 10000000 / freq_);   // relative, 100 ns units
                if (SetWaitableTimerEx((HANDLE)timer_, &due, 0, nullptr, nullptr, nullptr, 0))
                    WaitForSingleObject((HANDLE)timer_, INFINITE);
            }
            while ((now = Now()) < next_)
                YieldProcessor();
        }
        frameMs_ = (float)((double)(now - last_) * 1000.0 / (double)freq_);
        last_ = now;
        if (frameMs_ > 0.0f)
        {
            const float fps = 1000.0f / frameMs_;
            avgFps_ = avgFps_ <= 0.0f ? fps : avgFps_ + (fps - avgFps_) * std::min(1.0f, frameMs_ / 500.0f);
        }
    }
}
