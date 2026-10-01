// Esia - small math vocabulary: Vec2, Rect, Color. Header only, constexpr where it helps.
#pragma once
#include "config.hpp"
#include <algorithm>
#include <cmath>

namespace esia
{
    constexpr float kPi = 3.14159265358979f;
    constexpr float kTau = 6.28318530717959f;

    struct Vec2
    {
        float x = 0.0f, y = 0.0f;
        constexpr Vec2() = default;
        constexpr Vec2(float x_, float y_) : x(x_), y(y_) {}

        constexpr Vec2 operator+(Vec2 o) const { return {x + o.x, y + o.y}; }
        constexpr Vec2 operator-(Vec2 o) const { return {x - o.x, y - o.y}; }
        constexpr Vec2 operator*(float k) const { return {x * k, y * k}; }
        constexpr Vec2 operator*(Vec2 o) const { return {x * o.x, y * o.y}; }
        constexpr Vec2 operator/(float k) const { return {x / k, y / k}; }
        constexpr Vec2 operator-() const { return {-x, -y}; }
        constexpr Vec2& operator+=(Vec2 o) { x += o.x; y += o.y; return *this; }
        constexpr Vec2& operator-=(Vec2 o) { x -= o.x; y -= o.y; return *this; }
        constexpr Vec2& operator*=(float k) { x *= k; y *= k; return *this; }
        constexpr bool operator==(const Vec2&) const = default;
    };

    constexpr float Clamp(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
    constexpr float Saturate(float v) { return Clamp(v, 0.0f, 1.0f); }
    constexpr float Lerp(float a, float b, float t) { return a + (b - a) * t; }
    constexpr Vec2 Lerp(Vec2 a, Vec2 b, float t) { return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t}; }
    constexpr float Dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }
    constexpr float LengthSq(Vec2 v) { return v.x * v.x + v.y * v.y; }
    inline float Length(Vec2 v) { return std::sqrt(LengthSq(v)); }
    constexpr float SmoothStep(float e0, float e1, float v)
    {
        const float t = Saturate((v - e0) / (e1 - e0));
        return t * t * (3.0f - 2.0f * t);
    }
    constexpr float Radians(float deg) { return deg * (kPi / 180.0f); }
    constexpr float Degrees(float rad) { return rad * (180.0f / kPi); }

    // Axis aligned rectangle (min inclusive, max exclusive) in UI units, y down.
    struct Rect
    {
        Vec2 min, max;

        constexpr Rect() = default;
        constexpr Rect(Vec2 mn, Vec2 mx) : min{mn}, max{mx} {}
        constexpr Rect(float x0, float y0, float x1, float y1) : min{x0, y0}, max{x1, y1} {}
        static constexpr Rect FromSize(Vec2 pos, Vec2 size) { return {pos, pos + size}; }
        static constexpr Rect FromCenter(Vec2 c, Vec2 size) { return {c - size * 0.5f, c + size * 0.5f}; }

        constexpr float Width() const { return max.x - min.x; }
        constexpr float Height() const { return max.y - min.y; }
        constexpr Vec2 Size() const { return {Width(), Height()}; }
        constexpr Vec2 Center() const { return {(min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f}; }
        constexpr bool Contains(Vec2 p) const { return p.x >= min.x && p.y >= min.y && p.x < max.x && p.y < max.y; }
        constexpr bool Contains(const Rect& r) const { return r.min.x >= min.x && r.min.y >= min.y && r.max.x <= max.x && r.max.y <= max.y; }
        constexpr bool Overlaps(const Rect& r) const { return r.min.x < max.x && r.max.x > min.x && r.min.y < max.y && r.max.y > min.y; }
        constexpr bool Empty() const { return max.x <= min.x || max.y <= min.y; }

        constexpr Rect Expanded(float a) const { return {min.x - a, min.y - a, max.x + a, max.y + a}; }
        constexpr Rect Expanded(float ax, float ay) const { return {min.x - ax, min.y - ay, max.x + ax, max.y + ay}; }
        constexpr Rect Shrunk(float a) const { return Expanded(-a); }
        constexpr Rect Translated(Vec2 d) const { return {min + d, max + d}; }
        constexpr Rect Intersect(const Rect& r) const
        {
            return {(std::max)(min.x, r.min.x), (std::max)(min.y, r.min.y), (std::min)(max.x, r.max.x), (std::min)(max.y, r.max.y)};
        }
        constexpr Rect Union(const Rect& r) const
        {
            return {(std::min)(min.x, r.min.x), (std::min)(min.y, r.min.y), (std::max)(max.x, r.max.x), (std::max)(max.y, r.max.y)};
        }
        constexpr bool operator==(const Rect&) const = default;
    };

    // Straight-alpha sRGB color, float components 0..1.
    struct Color
    {
        float r = 0, g = 0, b = 0, a = 1;

        constexpr Color() = default;
        constexpr Color(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}

        // Color::Hex(0x007AFF) or Color::Hex(0x007AFF, 0.5f)
        static constexpr Color Hex(std::uint32_t rgb, float alpha = 1.0f)
        {
            return {((rgb >> 16) & 0xFF) / 255.0f, ((rgb >> 8) & 0xFF) / 255.0f, (rgb & 0xFF) / 255.0f, alpha};
        }
        static constexpr Color Rgba8(int r, int g, int b, int a = 255) { return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f}; }
        static constexpr Color White(float a = 1.0f) { return {1, 1, 1, a}; }
        static constexpr Color Black(float a = 1.0f) { return {0, 0, 0, a}; }
        static constexpr Color Clear() { return {0, 0, 0, 0}; }
        static Color Hsv(float h, float s, float v, float a = 1.0f);

        constexpr Color WithAlpha(float na) const { return {r, g, b, na}; }
        constexpr Color Fade(float k) const { return {r, g, b, a * k}; }
        // toward white / black by `k` (0..1), alpha kept
        constexpr Color Lighter(float k) const { return {r + (1 - r) * k, g + (1 - g) * k, b + (1 - b) * k, a}; }
        constexpr Color Darker(float k) const { return {r * (1 - k), g * (1 - k), b * (1 - k), a}; }
        constexpr float Luminance() const { return 0.2126f * r + 0.7152f * g + 0.0722f * b; }
        constexpr bool IsVisible() const { return a > 0.0005f; }
        // Packed for vertices: R in the lowest byte (memory order R, G, B, A = the RGBA8_UNORM vertex format).
        constexpr std::uint32_t ToRgba8() const
        {
            auto c8 = [](float v) { return (std::uint32_t)(Saturate(v) * 255.0f + 0.5f); };
            return c8(r) | (c8(g) << 8) | (c8(b) << 16) | (c8(a) << 24);
        }
        static constexpr Color FromRgba8(std::uint32_t c)
        {
            return {(c & 0xFF) / 255.0f, ((c >> 8) & 0xFF) / 255.0f, ((c >> 16) & 0xFF) / 255.0f, (c >> 24) / 255.0f};
        }
        constexpr bool operator==(const Color&) const = default;
    };

    constexpr Color Lerp(const Color& a, const Color& b, float t)
    {
        return {Lerp(a.r, b.r, t), Lerp(a.g, b.g, t), Lerp(a.b, b.b, t), Lerp(a.a, b.a, t)};
    }

    inline Color Color::Hsv(float h, float s, float v, float a)
    {
        if (s <= 0.0f)
            return {v, v, v, a};
        h = std::fmod(h, 1.0f) / (60.0f / 360.0f);
        if (h < 0.0f)
            h += 6.0f;
        const int i = (int)h;
        const float f = h - (float)i;
        const float p = v * (1.0f - s), q = v * (1.0f - s * f), t = v * (1.0f - s * (1.0f - f));
        switch (i)
        {
        case 0: return {v, t, p, a};
        case 1: return {q, v, p, a};
        case 2: return {p, v, t, a};
        case 3: return {p, q, v, a};
        case 4: return {t, p, v, a};
        default: return {v, p, q, a};
        }
    }
}
