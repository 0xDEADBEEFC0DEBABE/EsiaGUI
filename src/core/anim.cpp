// WGT UI - analytic spring solver and keyed animation store.
#include "core/context_impl.hpp"

namespace wgt
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

        if (zeta < 0.999f)
        {
            const float wd = omega * std::sqrt(1.0f - zeta * zeta);
            const float a = zeta * omega;
            const float e = std::exp(-a * dt);
            const float c = std::cos(wd * dt), s = std::sin(wd * dt);
            x = e * (x0 * c + ((v0 + a * x0) / wd) * s);
            v = e * (v0 * c - ((a * v0 + omega * omega * x0) / wd) * s);
        }
        else if (zeta <= 1.001f)
        {
            const float e = std::exp(-omega * dt);
            x = e * (x0 + (v0 + omega * x0) * dt);
            v = e * (v0 - omega * dt * (v0 + omega * x0));
        }
        else
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
        const float scale = std::max(1.0f, std::fabs(target));
        if (std::fabs(x) < 1e-4f * scale && std::fabs(v) < 1e-3f * scale)
        {
            value = target;
            velocity = 0.0f;
        }
    }

    namespace
    {
        struct FloatAnim
        {
            SpringState s;
            std::uint64_t lastFrame = 0;
        };
        struct TimerAnim
        {
            float t = 1.0f;
        };

        ImGuiID Salt(ImGuiID id, ImGuiID salt) { return id * 16777619u ^ salt; }
    }

    namespace anim
    {
        float Float(ImGuiID id, float target, const Spring* spring, float initial)
        {
            Context::Impl& impl = RequireImpl();
            bool created = false;
            FloatAnim& a = impl.state.Get<FloatAnim>(id, &created);
            if (created)
                a.s.value = std::isnan(initial) ? target : initial;
            if (a.lastFrame != impl.frameIndex)
            {
                a.lastFrame = impl.frameIndex;
                const Spring& sp = spring ? *spring : impl.theme.current.motion.standard;
                a.s.Step(target, sp, impl.dt);
            }
            return a.s.value;
        }

        float Velocity(ImGuiID id)
        {
            Context::Impl& impl = RequireImpl();
            FloatAnim* a = impl.state.Find<FloatAnim>(id);
            return a ? a->s.velocity : 0.0f;
        }

        void Set(ImGuiID id, float value, float velocity)
        {
            Context::Impl& impl = RequireImpl();
            FloatAnim& a = impl.state.Get<FloatAnim>(id);
            a.s.value = value;
            a.s.velocity = velocity;
            a.lastFrame = 0;
        }

        void Kick(ImGuiID id, float velocity)
        {
            Context::Impl& impl = RequireImpl();
            if (FloatAnim* a = impl.state.Find<FloatAnim>(id))
                a->s.velocity += velocity;
        }

        Vec2 Vector(ImGuiID id, Vec2 target, const Spring* spring)
        {
            return Vec2(Float(Salt(id, 0x51A7u), target.x, spring), Float(Salt(id, 0x7E11u), target.y, spring));
        }

        Color Colour(ImGuiID id, Color target, const Spring* spring)
        {
            return Color(Float(Salt(id, 0x1001u), target.r, spring), Float(Salt(id, 0x1002u), target.g, spring),
                         Float(Salt(id, 0x1003u), target.b, spring), Float(Salt(id, 0x1004u), target.a, spring));
        }

        float Timer(ImGuiID id, float duration, bool restart)
        {
            Context::Impl& impl = RequireImpl();
            TimerAnim& t = impl.state.Get<TimerAnim>(id);
            if (restart)
                t.t = 0.0f;
            else if (t.t < 1.0f)
                t.t = std::min(1.0f, t.t + impl.dt / std::max(duration, 1e-3f));
            return t.t;
        }

        double Time() { return RequireImpl().time; }
        float DeltaTime() { return RequireImpl().dt; }
    }
}
