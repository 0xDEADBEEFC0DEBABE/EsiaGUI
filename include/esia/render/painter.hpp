// Esia - Painter: the render class behind every widget (a port of wgt::Painter onto esia::DrawList).
//
// Painter draws signed-distance-field primitives with rich materials: gradients, strokes, soft shadows, inner
// shadows, outer / inner glow, liquid glass, image fills, shimmer, rounded masks and bloom layers. Every shape is
// ONE fx::Instance (24 float4) in the draw list's FX stream, drawn as one instanced quad: resolution independent,
// anti-aliased analytically on the GPU, and its cost does not grow with its size or its number of effects.
//
//   esia::Painter p(window.GetDrawList(), env);
//   p.Rect(r, esia::Style().Radius(18).Glass(material).Shadow(Color::Black(0.25f), 24, {0, 8}));
//   p.Circle(c, 22, esia::Style().Fill(Paint::Linear(a, b, 45)).Glow(a, 18, 0.8f));
//
// Differences from wgt::Painter (docs/REWRITE.md, "Migration"): it draws into any esia::DrawList given to it (no
// "current window"), the design-system values it needs (metrics scale, corner smoothing, pixel density, text,
// glow containment) come in a PainterEnv instead of the global context, textures are esia::TextureId and text is
// std::string_view. The instance encoding is byte for byte WGT's, so the shaders and pixels are the same.
#pragma once
#include "esia/core/draw_list.hpp"
#include "esia/text/text.hpp"

namespace esia
{
    // Liquid-glass material (all distances in UI units at scale 1, multiplied by PainterEnv::metricsScale). The
    // glass is a clear slab with a rounded edge: the interior shows the content behind (frosted by `blur`), the
    // edge refracts it like a lens.
    struct GlassMaterial
    {
        float blur = 2.0f;         // frost: backdrop blur radius (0 = crystal clear)
        float refraction = 8.0f;   // lensing: how far the content is pulled in (magnified) at the rim
        float bezel = 40.0f;       // radius of the curved edge; >= half the shape's size = one rounded rod / dome
        float dispersion = 0.35f;  // chromatic separation in the lensing band (0..1)
        float saturation = 1.30f;  // vibrancy: backdrop saturation
        float brightness = 0.03f;  // vibrancy: added light
        float specular = 0.85f;    // rim highlight strength (Fresnel-weighted, on the edge facing the light)
        float lightAngle = -2.2f;  // light direction (radians, screen space; default = from top-left)
        float noise = 0.012f;      // film grain against banding
        float legibility = 0.0f;   // 0..1: exposes the backdrop (its wide-area brightness) toward the tint's
        float magnify = 0.0f;      // loupe: the content behind is enlarged by 1 + magnify around the center
        Color tint = Color(1, 1, 1, 0.18f);   // rgb tint, a = amount
        Color rim = Color(1, 1, 1, 0.35f);    // hairline rim stroke
    };

    struct Paint
    {
        fx::PaintKind kind = fx::PaintKind::Solid;
        Color a = Color::Clear();
        Color b = Color::Clear();
        float angle = kPi * 0.5f;       // linear: direction (radians). conic: start angle
        Vec2 center = Vec2(0.5f, 0.5f); // radial / conic center (0..1 of bounds); spectrum: from / to
        float radius = 0.5f;            // radial radius (fraction of the longest side)
        bool loop = false;              // conic: a -> b -> back to a around the circle (no seam)

        static Paint Solid(Color c) { Paint p; p.a = p.b = c; return p; }
        // angleDeg: 0 = left->right, 90 = top->bottom
        static Paint Linear(Color from, Color to, float angleDeg = 90.0f) { Paint p; p.kind = fx::PaintKind::Linear; p.a = from; p.b = to; p.angle = Radians(angleDeg); return p; }
        static Paint Radial(Color inner, Color outer, Vec2 center = Vec2(0.5f, 0.5f), float radius = 0.5f) { Paint p; p.kind = fx::PaintKind::Radial; p.a = inner; p.b = outer; p.center = center; p.radius = radius; return p; }
        // Sweep from `from` (at startDeg) to `to` all the way round: the colors meet at startDeg (progress rings).
        static Paint Conic(Color from, Color to, float startDeg = -90.0f, Vec2 center = Vec2(0.5f, 0.5f)) { Paint p; p.kind = fx::PaintKind::Conic; p.a = from; p.b = to; p.angle = Radians(startDeg); p.center = center; return p; }
        // a at startDeg, b opposite, back to a: seamless all the way round.
        static Paint ConicLoop(Color a, Color b, float startDeg = -90.0f, Vec2 center = Vec2(0.5f, 0.5f)) { Paint p = Conic(a, b, startDeg, center); p.loop = true; return p; }
        // Rainbow along a direction, fading out at both ends: caustics, iridescent streaks. from / to: where it
        // starts and ends across the shape's bounds (0..1), alpha: its strength.
        static Paint Spectrum(float alpha = 1.0f, float from = 0.0f, float to = 1.0f, float angleDeg = 0.0f) { Paint p; p.kind = fx::PaintKind::Spectrum; p.a = p.b = Color(1, 1, 1, alpha); p.angle = Radians(angleDeg); p.center = Vec2(from, to); return p; }
    };

    // Fluent description of a shape's material.
    struct Style
    {
        bool hasFill = false;
        Paint fill;
        float radii[4] = {0, 0, 0, 0};     // tl, tr, br, bl
        float smoothing = -1.0f;            // corner smoothing; < 0 = PainterEnv::cornerSmoothing

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
        bool containGlow = true;            // fade the outer glow out before neighbouring items (PainterEnv::glow)
        float glowRadius = 0.0f;
        float glowIntensity = 0.0f;
        float innerGlowRadius = 0.0f;
        float innerGlowIntensity = 0.0f;

        bool hasGlass = false;
        GlassMaterial glass;

        TextureId image = 0;
        Vec2 uv0 = Vec2(0, 0), uv1 = Vec2(1, 1);

        float opacity = 1.0f;
        float shimmer = 0.0f;
        float shimmerSpeed = 0.55f;
        float noise = -1.0f;                // < 0 = glass default

        EffectId effect = 0;
        float custom[4] = {0, 0, 0, 0};

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
        Style& Image(TextureId tex, Vec2 uvMin = Vec2(0, 0), Vec2 uvMax = Vec2(1, 1)) { image = tex; uv0 = uvMin; uv1 = uvMax; if (!hasFill) Fill(Color::White()); return *this; }
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

    // Keeps glows off neighbouring items: implemented by the layout layer, which knows the items of a window
    // (WGT's item map). Painter works without it: glows then spill freely.
    class GlowContainment
    {
    public:
        virtual ~GlowContainment() = default;
        // A shape at rest (without press / pop scale animations) glows `reach` UI units beyond `restShape`:
        // containers reserve that room around the child that drew it.
        virtual void ReportGlowReach(const DrawList& dl, const Rect& restShape, float reach) = 0;
        // Bounds the outer glow of `shape` (reaching `extent`) may cover without touching a neighbouring item, and
        // the width of the fade before them. False: nothing to avoid.
        virtual bool GlowBounds(const DrawList& dl, const Rect& shape, float extent, Rect& bounds, float& fadeWidth) = 0;
    };

    // What Painter needs from the design system and the frame.
    struct PainterEnv
    {
        float metricsScale = 1.0f;      // DPI x user scale: GlassMaterial distances are multiplied by it
        float cornerSmoothing = 0.6f;   // continuous-corner smoothing for Style::smoothing < 0 (0 circular, 1 squircle)
        float pixelScale = 1.0f;        // render-target pixels per UI unit (pixel snapping, polyline anti-aliasing)
        float alpha = 1.0f;             // opacity every shape and text starts from
        text::TextSystem* text = nullptr;
        GlowContainment* glow = nullptr;
        // Flat drawing (ui::UiDesc::flat): shapes lose their shadows, glows and glass - glass becomes a solid surface,
        // `flatSurface` under the glass's tint - and rounded rectangles, capsules and circles with a solid fill and
        // stroke become a few quads on the text's texture: coverage tiles of their corners (TextSystem::FindTile,
        // made once per kind of shape) stretched as nine-patches, the middle of a large one from the white texture.
        // They batch with the text around them: a frame of plain widgets is a few draws of flat geometry.
        bool flat = false;
        Color flatSurface = Color(0.11f, 0.11f, 0.12f, 1.0f);
    };

    class ESIA_API Painter
    {
    public:
        explicit Painter(DrawList& drawList, const PainterEnv& env = {});

        DrawList& GetDrawList() const { return *dl_; }
        const PainterEnv& Env() const { return env_; }

        // ---------------------------------------------------------- SDF shapes
        void Rect(const esia::Rect& r, const Style& s);
        void Capsule(const esia::Rect& r, const Style& s);
        void Circle(Vec2 center, float radius, const Style& s);
        // Arc along a circle of `radius` (centerline) with round caps. Angles in radians, clockwise, 0 = +x.
        void Arc(Vec2 center, float radius, float thickness, float startRad, float sweepRad, const Style& s);
        void Ring(Vec2 center, float radius, float thickness, const Style& s);
        void Line(Vec2 a, Vec2 b, float thickness, const Style& s);
        // Liquid union of two rounded rects (smooth-min blend of the SDFs): metaball / morph effects.
        void Merge(const esia::Rect& a, const esia::Rect& b, float radius, float smoothness, const Style& s);
        // Rounded image (SDF-masked, anti-aliased).
        void Image(TextureId tex, const esia::Rect& r, float radius, Color tint = Color::White(), Vec2 uv0 = Vec2(0, 0), Vec2 uv1 = Vec2(1, 1));

        // ---------------------------------------------------------------- text (PainterEnv::text)
        // `pos` = top-left of the text box. Returns its size.
        Vec2 Text(Vec2 pos, text::FontRef font, Color color, std::string_view text, float wrapWidth = 0.0f, std::uint32_t flags = 0);
        // Text laid out inside `r`: align (0,0) = top-left, (0.5,0.5) = centered ... TextFlags_Ellipsis trims to the width.
        Vec2 TextBox(const esia::Rect& r, Vec2 align, text::FontRef font, Color color, std::string_view text, std::uint32_t flags = 0);
        Vec2 MeasureText(text::FontRef font, std::string_view text, float wrapWidth = 0.0f) const;
        // One glyph of an icon font optically centered on `center`, `font.size` = em size.
        void Icon(Vec2 center, text::FontRef font, char32_t icon, Color color);
        // Snaps a UI-unit coordinate to the physical pixel grid of the render target.
        float SnapToPixel(float v) const;

        // ---------------------------------------------------- cheap primitives (indexed geometry)
        void FillRect(const esia::Rect& r, Color c, float rounding = 0.0f);
        // A hairline on physical pixel rows (crisp at any DPI / render scale).
        void HLine(float x0, float x1, float y, Color c, float thickness = 1.0f);
        // One connected stroke through `points`: mitered joins (clipped on sharp turns), round caps, anti-aliased to
        // the physical pixel. Color: s.fill's color (s.opacity applies). Glows only if `s` has a glow, as one halo.
        void Polyline(const Vec2* points, int count, float thickness, const Style& s, std::uint32_t flags = PolylineFlags_None);
        // Fills between the same path and the horizontal line y = `baseline` (area charts). Solid or linear paint.
        void Area(const Vec2* points, int count, float baseline, const Paint& paint, std::uint32_t flags = PolylineFlags_None);
        // A segment of a ring (a donut chart's) between two radii: flat ends, each moved `inset` in along the ring, so
        // neighbours keep a gap of the same width from the hole to the rim. Angles as Arc's. Solid, anti-aliased.
        void Sector(Vec2 center, float innerRadius, float outerRadius, float startRad, float sweepRad, Color color, float inset = 0.0f);

        // ------------------------------------------------------------- state
        // Rounded mask applied to subsequent SDF shapes (max depth 8, innermost wins).
        void PushMask(const esia::Rect& r, float radius);
        void PopMask();
        void PushClip(const esia::Rect& r, bool intersect = true);
        void PopClip();
        // Uniform scale around `origin` applied to shapes and text emitted until PopScale().
        void PushScale(Vec2 origin, float scale);
        void PopScale();
        void SetAlpha(float a) { alpha_ = a; }
        float Alpha() const { return alpha_; }

        // Siri-orb light across `r`: a few thin lines held together at both ends and opening in the middle, the
        // top one dispersed into warm / green / lavender lines, a white line and bloom along the bottom. thickness =
        // line softness (UI units); smile / open = fractions of r's height; speed 1 = Siri's pace; time = the
        // animation's own clock (s), < 0 = the frame clock. Draw clear glass over it to have it refracted.
        void LightStreak(const esia::Rect& r, float intensity, float thickness, float speed = 1.0f, float smile = 0.0f, float open = 0.30f, float time = -1.0f);

        // Everything drawn between Begin/EndGlowLayer gets a GPU bloom halo (neon text, icons, lines).
        // tint.a = how much the halo is tinted towards tint.rgb (0 keeps the content colors). radius: UI units at scale 1.
        void BeginGlowLayer(Color tint, float radius, float intensity = 1.0f, float contentOpacity = 1.0f);
        void EndGlowLayer();

        // What this draw list draws until EndEdgeFade fades out towards the top / bottom edge of `region`, over
        // `top` / `bottom` UI units (0 = that edge stays hard). Real transparency, no color is added.
        void BeginEdgeFade(const esia::Rect& region, float top, float bottom);
        void EndEdgeFade();

        // Low level: emits a raw instance (custom shapes, tests).
        void Emit(const fx::Instance& inst, EffectId effect = 0, TextureId texture = 0);

    private:
        // `radius` >= 0: every corner's instead of the style's; `smoothing` >= 0: instead of the style's (capsules and
        // circles without a copy of the style)
        void EmitShape(fx::ShapeKind kind, const esia::Rect& bounds, const Style& s, const float* extra, float radius = -1.0f,
                       float smoothing = -1.0f);
        // Flat drawing: the shape as geometry when it can be, filled with `fill` (true: drawn, or nothing to draw).
        bool FlatGeometry(const esia::Rect& bounds, const Style& s, Color fill, const float* radii, float smoothing);
        void ApplyScale(fx::Instance& inst) const;
        void ExcludeFromScale(std::size_t vtxStart);
        float Pixel() const { return env_.pixelScale > 0.0f ? env_.pixelScale : 1.0f; }

        // A stack read only below its depth: not initialized (a Painter is made for every widget; zeroing these
        // was most of making one)
        template <class T, int N>
        struct Stack
        {
            union
            {
                T v[N];
            };
            Stack() {}
            T& operator[](int i) { return v[i]; }
            const T& operator[](int i) const { return v[i]; }
        };

        DrawList* dl_;
        PainterEnv env_;
        float alpha_ = 1.0f;
        int maskDepth_ = 0;
        Stack<esia::Rect, 8> masks_;
        Stack<float, 8> maskRadius_;
        int scaleDepth_ = 0;
        Stack<Vec2, 8> scaleOrigin_;
        Stack<float, 8> scaleValue_;
        Stack<std::size_t, 8> scaleVtxStart_;
        float mergeRadius_ = 0.0f;
        float mergeSmooth_ = -1.0f;
        std::uint64_t tileGeneration_ = 0;   // flat drawing: env_.text->TileGeneration(), asked once (0: not yet)
        // vertex ranges drawn at their final scaled geometry (text / icons): PopScale leaves them alone
        static constexpr int kMaxExcluded = 64;
        Stack<std::size_t, kMaxExcluded> excludedStart_, excludedEnd_;
        int excludedCount_ = 0;
    };
}
