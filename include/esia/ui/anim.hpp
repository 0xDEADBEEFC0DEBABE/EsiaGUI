// Esia UI - animation primitives: an analytic spring and easing curves (WGT's, ported).
//
// Immediate-mode widgets animate by asking every frame for the value that moves toward a target:
// ui::Anim(id, target, spring) keeps a SpringState per id in the core's per-id state (Context::State) and steps it
// once per frame. SpringState is also usable on its own (layout providers keep theirs).
#pragma once
#include "esia/ui/theme.hpp"
#include <cmath>

namespace esia::ui
{
    // Damped harmonic oscillator stepped analytically: frame-rate independent and stable for any dt.
    struct SpringState
    {
        float value = 0.0f;
        float velocity = 0.0f;

        ESIA_API void Step(float target, const Spring& spring, float dt);
        bool Settled(float target, float eps = 1e-3f) const
        {
            return std::fabs(value - target) < eps && std::fabs(velocity) < eps * 10.0f;
        }
    };

    namespace ease
    {
        inline float InOutCubic(float t) { return t < 0.5f ? 4 * t * t * t : 1 - std::pow(-2 * t + 2, 3.0f) / 2; }
        inline float OutCubic(float t) { return 1 - std::pow(1 - t, 3.0f); }
        inline float OutBack(float t)
        {
            const float c1 = 1.70158f, c3 = c1 + 1;
            return 1 + c3 * std::pow(t - 1, 3.0f) + c1 * std::pow(t - 1, 2.0f);
        }
        inline float OutExpo(float t) { return t >= 1 ? 1 : 1 - std::pow(2.0f, -10 * t); }
        inline float Smooth(float t)
        {
            t = Saturate(t);
            return t * t * (3 - 2 * t);
        }
    }
}
