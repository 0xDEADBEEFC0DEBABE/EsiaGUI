// Esia UI - the analytic spring (WGT's solver).
#include "esia/ui/anim.hpp"
#include <algorithm>

namespace esia::ui
{
    void SpringState::Step(float target, const Spring& spring, float dt)
    {
        if (dt <= 0.0f)
            return;
        const float omega = kTau / std::max(spring.response, 1e-3f);
        const float zeta = std::max(spring.damping, 0.0f);
        const float x0 = value - target;
        const float v0 = velocity;
        float x, v;
        if (zeta < 0.999f)   // under-damped: a decaying oscillation
        {
            const float wd = omega * std::sqrt(1.0f - zeta * zeta);
            const float a = zeta * omega;
            const float e = std::exp(-a * dt);
            const float c = std::cos(wd * dt), s = std::sin(wd * dt);
            x = e * (x0 * c + ((v0 + a * x0) / wd) * s);
            v = e * (v0 * c - ((a * v0 + omega * omega * x0) / wd) * s);
        }
        else if (zeta <= 1.001f)   // critically damped
        {
            const float e = std::exp(-omega * dt);
            x = e * (x0 + (v0 + omega * x0) * dt);
            v = e * (v0 - omega * dt * (v0 + omega * x0));
        }
        else   // over-damped: two decaying exponentials
        {
            const float sq = std::sqrt(zeta * zeta - 1.0f);
            const float r1 = -omega * (zeta - sq);
            const float r2 = -omega * (zeta + sq);
            const float c2 = (v0 - r1 * x0) / (r2 - r1);
            const float c1 = x0 - c2;
            const float e1 = std::exp(r1 * dt), e2 = std::exp(r2 * dt);
            x = c1 * e1 + c2 * e2;
            v = c1 * r1 * e1 + c2 * r2 * e2;
        }
        value = target + x;
        velocity = v;
        // come to rest exactly: a value a hair off its target keeps a pixel flickering and the frame loop awake
        const float scale = std::max(1.0f, std::fabs(target));
        if (std::fabs(x) < 1e-4f * scale && std::fabs(v) < 1e-3f * scale)
        {
            value = target;
            velocity = 0.0f;
        }
    }
}
