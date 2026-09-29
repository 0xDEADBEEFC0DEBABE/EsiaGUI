// WGT UI - Painter: the render class behind every WGT widget.
//
// Painter draws signed-distance-field primitives with rich materials: gradients, strokes, soft
// shadows, inner shadows, outer/inner glow, liquid glass, image fills, shimmer, rounded masks and
// bloom layers. Everything is resolution independent and anti-aliased analytically on the GPU.
//
//   wgt::Painter p;                                  // current window draw list
//   p.Rect(r, wgt::Style().Radius(18).Glass(theme.materials.control).Shadow(Color::Black(0.25f), 24, {0, 8}));
//   p.Circle(c, 22, wgt::Style().Fill(Paint::Linear(a, b, 45)).Glow(a, 18, 0.8f));
#pragma once
#include "fx.hpp"
#include "theme.hpp"

namespace wgt
{
    struct Paint
    {
        fx::PaintKind kind = fx::PaintKind::Solid;
        Color a = Color::Clear();
        Color b = Color::Clear();
        float angle = kPi * 0.5f;       // linear: direction (radians). conic: start angle
        Vec2 center = Vec2(0.5f, 0.5f); // radial / conic center (0..1 of bounds)
        float radius = 0.5f;            // radial radius (fraction of the longest side)
        bool loop = false;              // conic: a -> b -> back to a around the circle (no seam)

        static Paint Solid(Color c) { Paint p; p.a = p.b = c; return p; }
        // angleDeg: 0 = left->right, 90 = top->bottom
        static Paint Linear(Color from, Color to, float angleDeg = 90.0f) { Paint p; p.kind = fx::PaintKind::Linear; p.a = from; p.b = to; p.angle = Radians(angleDeg); return p; }
        static Paint Radial(Color inner, Color outer, Vec2 center = Vec2(0.5f, 0.5f), float radius = 0.5f) { Paint p; p.kind = fx::PaintKind::Radial; p.a = inner; p.b = outer; p.center = center; p.radius = radius; return p; }
        // Sweep from `from` (at startDeg) to `to` all the way round: the colors meet at startDeg (progress rings,
        // spinner tails). For a full ring or disc without that seam use ConicLoop.
        static Paint Conic(Color from, Color to, float startDeg = -90.0f, Vec2 center = Vec2(0.5f, 0.5f)) { Paint p; p.kind = fx::PaintKind::Conic; p.a = from; p.b = to; p.angle = Radians(startDeg); p.center = center; return p; }
        // Rainbow (red .. violet) along a direction, fading out at both ends: caustics, iridescent streaks.
        // from / to: where it starts and ends across the shape's bounds (0..1), alpha: its strength.
        static Paint Spectrum(float alpha = 1.0f, float from = 0.0f, float to = 1.0f, float angleDeg = 0.0f) { Paint p; p.kind = fx::PaintKind::Spectrum; p.a = p.b = Color(1, 1, 1, alpha); p.angle = Radians(angleDeg); p.center = Vec2(from, to); return p; }
        // a at startDeg, b opposite, back to a: seamless all the way round.
        static Paint ConicLoop(Color a, Color b, float startDeg = -90.0f, Vec2 center = Vec2(0.5f, 0.5f)) { Paint p = Conic(a, b, startDeg, center); p.loop = true; return p; }
    };

    // Fluent description of a shape's material.
    struct Style
    {
        bool hasFill = false;
        Paint fill;
        float radii[4] = {0, 0, 0, 0};     // tl, tr, br, bl
        float smoothing = -1.0f;            // corner smoothing; < 0 = theme default

        float strokeWidth = 0.0f;
        Color strokeColor = Color::Clear();
        float strokeAlign = 0.0f;           // 0 inside, 0.5 centered, 1 outside
        float strokeFadeTo = 1.0f;          // alpha multiplier at the far end of the fade
        float strokeFadeAngle = kPi * 0.5f;

        Color shadowColor = Color::Clear();
        float shadowBlur = 0.0f;
        float shadowSpread = 0.0f;
        Vec2 shadowOffset = Vec2(0, 0);
        bool shadowInset = false;

        Color glowColor = Color::Clear();
        bool containGlow = true;            // fade the outer glow out before neighbouring items (and reserve room in layouts)
        float glowRadius = 0.0f;
        float glowIntensity = 0.0f;
        float innerGlowRadius = 0.0f;
        float innerGlowIntensity = 0.0f;

        bool hasGlass = false;
        GlassMaterial glass;

        ImTextureID image = ImTextureID_Invalid;
        Vec2 uv0 = Vec2(0, 0), uv1 = Vec2(1, 1);

        float opacity = 1.0f;
        float shimmer = 0.0f;
        float shimmerSpeed = 0.55f;
        float noise = -1.0f;                // < 0 = glass default

        EffectId effect = 0;
        float custom[4] = {0, 0, 0, 0};

        // ---- builder ----
        Style& Fill(Color c) { hasFill = true; fill = Paint::Solid(c); return *this; }
        Style& Fill(const Paint& p) { hasFill = true; fill = p; return *this; }
        Style& Radius(float r) { radii[0] = radii[1] = radii[2] = radii[3] = r; return *this; }
        Style& Radius(float tl, float tr, float br, float bl) { radii[0] = tl; radii[1] = tr; radii[2] = br; radii[3] = bl; return *this; }
        Style& Smoothing(float s) { smoothing = s; return *this; }
        Style& Stroke(float width, Color c, float align = 0.0f) { strokeWidth = width; strokeColor = c; strokeAlign = align; return *this; }
        Style& StrokeFade(float fadeTo, float angleDeg = 90.0f) { strokeFadeTo = fadeTo; strokeFadeAngle = Radians(angleDeg); return *this; }
        Style& Shadow(Color c, float blur, Vec2 offset = Vec2(0, 0), float spread = 0.0f) { shadowColor = c; shadowBlur = blur; shadowOffset = offset; shadowSpread = spread; shadowInset = false; return *this; }
        Style& InnerShadow(Color c, float blur, Vec2 offset = Vec2(0, 0), float spread = 0.0f) { shadowColor = c; shadowBlur = blur; shadowOffset = offset; shadowSpread = spread; shadowInset = true; return *this; }
        Style& Glow(Color c, float radius, float intensity = 1.0f) { glowColor = c; glowRadius = radius; glowIntensity = intensity; return *this; }
        Style& InnerGlow(float radius, float intensity = 1.0f) { innerGlowRadius = radius; innerGlowIntensity = intensity; return *this; }
        // false: the glow may spill over neighbouring items (decorative backdrops, hero art)
        Style& ContainGlow(bool contain) { containGlow = contain; return *this; }
        Style& Glass(const GlassMaterial& m) { hasGlass = true; glass = m; return *this; }
        Style& Image(ImTextureID tex, Vec2 uvMin = Vec2(0, 0), Vec2 uvMax = Vec2(1, 1)) { image = tex; uv0 = uvMin; uv1 = uvMax; if (!hasFill) Fill(Color::White()); return *this; }
        Style& Opacity(float o) { opacity = o; return *this; }
        Style& Shimmer(float intensity, float speed = 0.55f) { shimmer = intensity; shimmerSpeed = speed; return *this; }
        Style& Noise(float n) { noise = n; return *this; }
        Style& Effect(EffectId id, float p0 = 0, float p1 = 0, float p2 = 0, float p3 = 0) { effect = id; custom[0] = p0; custom[1] = p1; custom[2] = p2; custom[3] = p3; return *this; }
    };

    enum PolylineFlags_ : std::uint32_t
    {
        PolylineFlags_None = 0,
        // A smooth curve through the points instead of straight segments. When x keeps increasing (charts) it is
        // monotone: it never overshoots the data.
        PolylineFlags_Smooth = 1u << 0,
    };

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4251)   // header-only POD members (Rect / ImVec2): no DLL interface needed
#endif
    class WGT_API Painter
    {
    public:
        Painter();                          // current ImGui window draw list
        explicit Painter(ImDrawList* drawList);

        ImDrawList* DrawList() const { return dl_; }

        // ---------------------------------------------------------- SDF shapes
        void Rect(const wgt::Rect& r, const Style& s);
        void Capsule(const wgt::Rect& r, const Style& s);
        void Circle(Vec2 center, float radius, const Style& s);
        // Arc along a circle of `radius` (centerline) with round caps. Angles in radians, clockwise, 0 = +x.
        void Arc(Vec2 center, float radius, float thickness, float startRad, float sweepRad, const Style& s);
        void Ring(Vec2 center, float radius, float thickness, const Style& s);
        void Line(Vec2 a, Vec2 b, float thickness, const Style& s);
        // Liquid union of two rounded rects (smooth-min blend of the SDFs): metaball / morph effects.
        void Merge(const wgt::Rect& a, const wgt::Rect& b, float radius, float smoothness, const Style& s);

        // ---------------------------------------------------------------- text
        // WGT text engine (DirectWrite shaping and fallback, analytic anti-aliasing, physical-pixel snapping). `pos` = top-left.
        Vec2 Text(Vec2 pos, TextStyle style, Color color, const char* text, const char* textEnd = nullptr, float wrapWidth = 0.0f);
        Vec2 Text(Vec2 pos, FontRef font, Color color, const char* text, const char* textEnd = nullptr, float wrapWidth = 0.0f, std::uint32_t flags = 0);
        // Text laid out inside `r`: align (0,0) = top-left, (0.5,0.5) = centered ... TextFlags_Ellipsis trims to the width.
        Vec2 TextBox(const wgt::Rect& r, Vec2 align, FontRef font, Color color, const char* text, const char* textEnd = nullptr, std::uint32_t flags = 0);
        Vec2 TextAligned(const wgt::Rect& r, Vec2 align, TextStyle style, Color color, const char* text, const char* textEnd = nullptr);
        static Vec2 MeasureText(TextStyle style, const char* text, const char* textEnd = nullptr, float wrapWidth = 0.0f);
        static Vec2 MeasureText(FontRef font, const char* text, const char* textEnd = nullptr, float wrapWidth = 0.0f);

        // Icon glyph (see wgt/icons.hpp) optically centered on `center`, `size` = em size in UI units.
        void Icon(Vec2 center, wgt::Icon icon, float size, Color color);
        // Snaps a UI-unit coordinate to the physical pixel grid of the render target.
        static float SnapToPixel(float v);

        // Rounded image (SDF-masked, anti-aliased).
        void Image(ImTextureID tex, const wgt::Rect& r, float radius, Color tint = Color::White(), Vec2 uv0 = Vec2(0, 0), Vec2 uv1 = Vec2(1, 1));

        // ---------------------------------------------------- cheap primitives
        void FillRect(const wgt::Rect& r, Color c, float rounding = 0.0f);
        void HLine(float x0, float x1, float y, Color c, float thickness = 1.0f);
        // One connected stroke through `points` (charts, paths): mitered joins (clipped on sharp turns, no
        // spikes), round caps, anti-aliased to the physical pixel. Color: s.fill's color (s.opacity applies).
        // It glows only if `s` has a glow, and then as one even halo around the whole line (not a halo per
        // segment stacking up where they meet).
        void Polyline(const Vec2* points, int count, float thickness, const Style& s, std::uint32_t flags = PolylineFlags_None);
        // Fills between the same path and the horizontal line y = `baseline` (area charts). Solid or linear paint
        // (a vertical Paint::Linear gives the usual fade towards the baseline).
        void Area(const Vec2* points, int count, float baseline, const Paint& paint, std::uint32_t flags = PolylineFlags_None);

        // ------------------------------------------------------------- state
        // Rounded mask applied to subsequent SDF shapes (max depth 8, intersects nothing - innermost wins).
        void PushMask(const wgt::Rect& r, float radius);
        void PopMask();
        void PushClip(const wgt::Rect& r, bool intersect = true);
        void PopClip();
        // Uniform scale around `origin` applied to shapes and text emitted until PopScale().
        void PushScale(Vec2 origin, float scale);
        void PopScale();
        void SetAlpha(float a) { alpha_ = a; }
        float Alpha() const { return alpha_; }

        // Light across `r` like the Siri orb's: a few thin lines of light, softened just enough, held together at
        // both ends and opening up in the middle. The top line disperses into thin warm, green and lavender lines;
        // a white line runs along the bottom edge with a white bloom above it, a pale sheet of light in between.
        // The lines part and gather as the light breathes; brightness saturates softly instead of clipping.
        // Draw clear glass over it to have it refracted. thickness = line softness (UI units); smile = how much the
        // whole light bows, open = how far the lines open in the middle (both fractions of r's height); speed 1 =
        // Siri's pace; time = the animation's own clock in seconds (0 = closed, open about 0.7 s later), < 0 runs
        // it on the frame clock.
        void LightStreak(const wgt::Rect& r, float intensity, float thickness, float speed = 1.0f, float smile = 0.0f, float open = 0.30f, float time = -1.0f);

        // Everything drawn between Begin/EndGlowLayer gets a GPU bloom halo (neon text, icons, lines...).
        // tint.a = how much the halo is tinted towards tint.rgb (0 keeps the content colors).
        void BeginGlowLayer(Color tint, float radius, float intensity = 1.0f, float contentOpacity = 1.0f);
        void EndGlowLayer();

        // What this draw list draws until EndEdgeFade fades out towards the top / bottom edge of `region`, over
        // `top` / `bottom` UI units (0 = that edge stays hard). Scroll views use it so content dissolves at their
        // edges instead of being cut; the fade is real transparency, no color is added.
        void BeginEdgeFade(const wgt::Rect& region, float top, float bottom);
        void EndEdgeFade();

        // Low-level: emit a raw instance (advanced / custom backends & effects).
        void Emit(const fx::Instance& inst, EffectId effect = 0, ImTextureID texture = ImTextureID_Invalid);

    private:
        void EmitShape(fx::ShapeKind kind, const wgt::Rect& bounds, const Style& s, const float* extra);
        void ApplyScale(fx::Instance& inst) const;
        void ExcludeFromScale(int vtxStart);

        ImDrawList* dl_ = nullptr;
        float alpha_ = 1.0f;
        int maskDepth_ = 0;
        wgt::Rect masks_[8];
        float maskRadius_[8] = {};
        int scaleDepth_ = 0;
        Vec2 scaleOrigin_[8];
        float scaleValue_[8] = {};
        int scaleVtxStart_[8] = {};
        float mergeRadius_ = 0.0f;
        float mergeSmooth_ = -1.0f;
        // vertex ranges drawn at their final scaled geometry (text / icons): PopScale leaves them alone
        static constexpr int kMaxExcluded = 64;
        int excludedStart_[kMaxExcluded] = {};
        int excludedEnd_[kMaxExcluded] = {};
        int excludedCount_ = 0;
    };
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
}
