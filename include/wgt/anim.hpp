// WGT UI - animation primitives
//
// Immediate-mode friendly: state is keyed by ImGuiID and lives in the context; call anim::Float() (Vector,
// Colour) every frame with the *target* and you get the current, physically-animated value back.
#pragma once
#include "theme.hpp"

namespace wgt
{
    // Analytic damped-harmonic-oscillator step. Frame-rate independent and stable for any dt.
    struct SpringState
    {
        float value = 0.0f;
        float velocity = 0.0f;

        WGT_API void Step(float target, const Spring& spring, float dt);
        bool Settled(float target, float eps = 1e-3f) const
        {
            return std::fabs(value - target) < eps && std::fabs(velocity) < eps * 10.0f;
        }
    };

    namespace ease
    {
        inline float InOutCubic(float t) { return t < 0.5f ? 4 * t * t * t : 1 - std::pow(-2 * t + 2, 3.0f) / 2; }
        inline float OutCubic(float t) { return 1 - std::pow(1 - t, 3.0f); }
        inline float OutBack(float t) { const float c1 = 1.70158f, c3 = c1 + 1; return 1 + c3 * std::pow(t - 1, 3.0f) + c1 * std::pow(t - 1, 2.0f); }
        inline float OutExpo(float t) { return t >= 1 ? 1 : 1 - std::pow(2.0f, -10 * t); }
        inline float Smooth(float t) { t = Saturate(t); return t * t * (3 - 2 * t); }
    }

    namespace anim
    {
        // Spring-animates a scalar identified by `id` towards `target`. Uses the theme's standard spring
        // when `spring` is null. `initial` seeds the value on first use (default: target, i.e. no pop-in).
        WGT_API float Float(ImGuiID id, float target, const Spring* spring = nullptr, float initial = NAN);
        WGT_API Vec2  Vector(ImGuiID id, Vec2 target, const Spring* spring = nullptr);
        WGT_API Color Colour(ImGuiID id, Color target, const Spring* spring = nullptr);
        // Velocity of a Float() animation (useful for "liquid" stretch effects).
        WGT_API float Velocity(ImGuiID id);
        // Jumps a Float() animation to `value` (no motion), e.g. to replay an appear animation.
        WGT_API void Set(ImGuiID id, float value, float velocity = 0.0f);
        // Adds an impulse to a Float() animation (e.g. a "pop" on click).
        WGT_API void Kick(ImGuiID id, float velocity);
        // Linear timer 0..1 that restarts whenever `restart` is true.
        WGT_API float Timer(ImGuiID id, float duration, bool restart);
        // Seconds since the context was created (UI thread clock).
        WGT_API double Time();
        WGT_API float  DeltaTime();

        // Convenience: string ids are hashed within the current ImGui ID stack.
        inline float Float(const char* key, float target, const Spring* spring = nullptr) { return Float(ImGui::GetID(key), target, spring); }
    }
}
