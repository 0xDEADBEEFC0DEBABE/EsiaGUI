// WGT UI - small math vocabulary (Vec2 / Rect / Color)
#pragma once
#include "config.hpp"
#include <cmath>
#include <algorithm>

namespace wgt
{
    using Vec2 = ImVec2;

    inline float Clamp(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
    inline float Saturate(float v) { return Clamp(v, 0.0f, 1.0f); }
    inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }
    inline Vec2  Lerp(Vec2 a, Vec2 b, float t) { return Vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); }
    inline float Remap(float v, float a0, float a1, float b0, float b1) { return b0 + (v - a0) / (a1 - a0) * (b1 - b0); }
    inline float SmoothStep(float e0, float e1, float v) { const float t = Saturate((v - e0) / (e1 - e0)); return t * t * (3.0f - 2.0f * t); }
    inline float Length(Vec2 v) { return std::sqrt(v.x * v.x + v.y * v.y); }
    constexpr float kPi = 3.14159265358979f;
    constexpr float kTau = 6.28318530717959f;
    inline float Radians(float deg) { return deg * (kPi / 180.0f); }
    inline float Degrees(float rad) { return rad * (180.0f / kPi); }

    // Axis aligned rectangle (min inclusive, max exclusive), in ImGui coordinates.
    struct Rect
    {
        Vec2 min{0, 0};
        Vec2 max{0, 0};

        constexpr Rect() = default;
        constexpr Rect(Vec2 mn, Vec2 mx) : min(mn), max(mx) {}
        constexpr Rect(float x0, float y0, float x1, float y1) : min(x0, y0), max(x1, y1) {}
        static Rect FromSize(Vec2 pos, Vec2 size) { return Rect(pos, Vec2(pos.x + size.x, pos.y + size.y)); }
        static Rect FromCenter(Vec2 c, Vec2 size) { return Rect(c.x - size.x * 0.5f, c.y - size.y * 0.5f, c.x + size.x * 0.5f, c.y + size.y * 0.5f); }

        float Width() const  { return max.x - min.x; }
        float Height() const { return max.y - min.y; }
        Vec2  Size() const   { return Vec2(Width(), Height()); }
        Vec2  Center() const { return Vec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f); }
        bool  Contains(Vec2 p) const { return p.x >= min.x && p.y >= min.y && p.x < max.x && p.y < max.y; }
        bool  Overlaps(const Rect& r) const { return r.min.x < max.x && r.max.x > min.x && r.min.y < max.y && r.max.y > min.y; }
        bool  Empty() const { return max.x <= min.x || max.y <= min.y; }

        Rect Expanded(float a) const { return Rect(min.x - a, min.y - a, max.x + a, max.y + a); }
        Rect Expanded(float ax, float ay) const { return Rect(min.x - ax, min.y - ay, max.x + ax, max.y + ay); }
        Rect Shrunk(float a) const { return Expanded(-a); }
        Rect Translated(Vec2 d) const { return Rect(min.x + d.x, min.y + d.y, max.x + d.x, max.y + d.y); }
        Rect Scaled(float s) const { Vec2 c = Center(); Vec2 h(Width() * 0.5f * s, Height() * 0.5f * s); return Rect(c.x - h.x, c.y - h.y, c.x + h.x, c.y + h.y); }
        Rect Intersect(const Rect& r) const { return Rect(std::max(min.x, r.min.x), std::max(min.y, r.min.y), std::min(max.x, r.max.x), std::min(max.y, r.max.y)); }
        Rect Union(const Rect& r) const { return Rect(std::min(min.x, r.min.x), std::min(min.y, r.min.y), std::max(max.x, r.max.x), std::max(max.y, r.max.y)); }
        // Splits
        Rect Left(float w) const   { return Rect(min.x, min.y, min.x + w, max.y); }
        Rect Right(float w) const  { return Rect(max.x - w, min.y, max.x, max.y); }
        Rect Top(float h) const    { return Rect(min.x, min.y, max.x, min.y + h); }
        Rect Bottom(float h) const { return Rect(min.x, max.y - h, max.x, max.y); }
    };

    // Straight-alpha sRGB color with float components (0..1).
    struct Color
    {
        float r = 0, g = 0, b = 0, a = 1;

        constexpr Color() = default;
        constexpr Color(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}
        Color(const ImVec4& v) : r(v.x), g(v.y), b(v.z), a(v.w) {}

        // Color::Hex(0x007AFF) or Color::Hex(0x007AFF, 0.5f)
        static constexpr Color Hex(std::uint32_t rgb, float alpha = 1.0f)
        {
            return Color(((rgb >> 16) & 0xFF) / 255.0f, ((rgb >> 8) & 0xFF) / 255.0f, (rgb & 0xFF) / 255.0f, alpha);
        }
        static constexpr Color Rgba8(int r, int g, int b, int a = 255) { return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f); }
        static constexpr Color White(float a = 1.0f) { return Color(1, 1, 1, a); }
        static constexpr Color Black(float a = 1.0f) { return Color(0, 0, 0, a); }
        static constexpr Color Clear() { return Color(0, 0, 0, 0); }
        static Color Hsv(float h, float s, float v, float a = 1.0f)
        {
            float r, g, b;
            ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b);
            return Color(r, g, b, a);
        }

        constexpr Color WithAlpha(float na) const { return Color(r, g, b, na); }
        constexpr Color Fade(float k) const { return Color(r, g, b, a * k); }
        Color Lighter(float k) const { return Color(r + (1 - r) * k, g + (1 - g) * k, b + (1 - b) * k, a); }
        Color Darker(float k) const { return Color(r * (1 - k), g * (1 - k), b * (1 - k), a); }
        float Luminance() const { return 0.2126f * r + 0.7152f * g + 0.0722f * b; }
        bool  IsVisible() const { return a > 0.0005f; }

        ImVec4 ToVec4() const { return ImVec4(r, g, b, a); }
        ImU32  ToU32() const
        {
            auto c8 = [](float v) { return (ImU32)(Saturate(v) * 255.0f + 0.5f); };
            return (c8(a) << IM_COL32_A_SHIFT) | (c8(b) << IM_COL32_B_SHIFT) | (c8(g) << IM_COL32_G_SHIFT) | (c8(r) << IM_COL32_R_SHIFT);
        }
        operator ImVec4() const { return ToVec4(); }
    };

    inline Color Lerp(const Color& a, const Color& b, float t)
    {
        return Color(Lerp(a.r, b.r, t), Lerp(a.g, b.g, t), Lerp(a.b, b.b, t), Lerp(a.a, b.a, t));
    }
    inline bool operator==(const Color& a, const Color& b) { return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a; }
    inline bool operator!=(const Color& a, const Color& b) { return !(a == b); }
}
