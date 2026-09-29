// WGT UI - Painter implementation: encodes SDF shapes into fx::Command draw-list callbacks.
#include "core/context_impl.hpp"

namespace wgt
{
    namespace
    {
        void FxCommandMarker(const ImDrawList*, const ImDrawCmd*) {}

        inline void Set4(float* d, float a, float b, float c, float e) { d[0] = a; d[1] = b; d[2] = c; d[3] = e; }
        inline void SetColor(float* d, const Color& c) { Set4(d, c.r, c.g, c.b, c.a); }
        inline void SetRect(float* d, const Rect& r) { Set4(d, r.min.x, r.min.y, r.max.x, r.max.y); }

        inline Vec2 ScalePoint(Vec2 p, Vec2 o, float s) { return Vec2(o.x + (p.x - o.x) * s, o.y + (p.y - o.y) * s); }

        // The path Polyline / Area follow: the points (minus repeats), or a smooth curve through them sampled
        // every few pixels. Monotone cubic (Fritsch-Carlson) when x keeps increasing, so a chart never
        // overshoots its data; Catmull-Rom otherwise.
        void BuildPath(const Vec2* pts, int n, std::uint32_t flags, ImVector<ImVec2>& out)
        {
            out.resize(0);
            for (int i = 0; i < n; ++i)
                if (out.Size == 0 || ImLengthSqr(ImVec2(pts[i].x - out.back().x, pts[i].y - out.back().y)) > 1e-8f)
                    out.push_back(pts[i]);
            if (!(flags & PolylineFlags_Smooth) || out.Size < 3)
                return;
            ImVector<ImVec2> src;
            src.swap(out);
            const int m = src.Size;
            bool monotone = true;
            for (int i = 1; i < m && monotone; ++i)
                monotone = src[i].x > src[i - 1].x;
            ImVector<float> tan;
            if (monotone)
            {
                ImVector<float> d;
                d.resize(m - 1);
                for (int i = 0; i < m - 1; ++i)
                    d[i] = (src[i + 1].y - src[i].y) / (src[i + 1].x - src[i].x);
                tan.resize(m);
                tan[0] = d[0];
                tan[m - 1] = d[m - 2];
                for (int i = 1; i < m - 1; ++i)
                    tan[i] = d[i - 1] * d[i] <= 0.0f ? 0.0f : (d[i - 1] + d[i]) * 0.5f;
                for (int i = 0; i < m - 1; ++i)
                {
                    if (d[i] == 0.0f)
                    {
                        tan[i] = tan[i + 1] = 0.0f;
                        continue;
                    }
                    const float a = tan[i] / d[i], b = tan[i + 1] / d[i];
                    const float h = a * a + b * b;
                    if (h > 9.0f)
                    {
                        const float k = 3.0f / std::sqrt(h);
                        tan[i] = k * a * d[i];
                        tan[i + 1] = k * b * d[i];
                    }
                }
            }
            const float rs = ImGui::GetIO().DisplayFramebufferScale.x > 0.0f ? ImGui::GetIO().DisplayFramebufferScale.x : 1.0f;
            out.push_back(src[0]);
            for (int i = 0; i < m - 1 && out.Size < 16000; ++i)
            {
                const ImVec2 p0 = src[i > 0 ? i - 1 : 0], p1 = src[i], p2 = src[i + 1], p3 = src[i + 2 < m ? i + 2 : m - 1];
                const float len = std::sqrt(ImLengthSqr(ImVec2(p2.x - p1.x, p2.y - p1.y))) * rs;
                const int steps = ImClamp((int)std::ceil(len / 3.0f), 1, 24);
                for (int k = 1; k <= steps; ++k)
                {
                    const float t = (float)k / (float)steps, t2 = t * t, t3 = t2 * t;
                    if (monotone)
                    {
                        const float h = p2.x - p1.x;
                        const float y = (2 * t3 - 3 * t2 + 1) * p1.y + (t3 - 2 * t2 + t) * h * tan[i] + (-2 * t3 + 3 * t2) * p2.y + (t3 - t2) * h * tan[i + 1];
                        out.push_back(ImVec2(p1.x + h * t, y));
                    }
                    else
                    {
                        auto cr = [&](float a, float b, float c, float e) { return 0.5f * (2 * b + (-a + c) * t + (2 * a - 5 * b + 4 * c - e) * t2 + (-a + 3 * b - 3 * c + e) * t3); };
                        out.push_back(ImVec2(cr(p0.x, p1.x, p2.x, p3.x), cr(p0.y, p1.y, p2.y, p3.y)));
                    }
                }
            }
        }

        thread_local ImVector<ImVec2> tPath;
    }

    ImDrawCallback fx::CommandCallback() { return &FxCommandMarker; }

    Painter::Painter() : Painter(ImGui::GetWindowDrawList()) {}

    Painter::Painter(ImDrawList* drawList) : dl_(drawList)
    {
        alpha_ = ImGui::GetCurrentContext() ? ImGui::GetStyle().Alpha : 1.0f;
    }

    // ------------------------------------------------------------ shapes
    void Painter::Rect(const wgt::Rect& r, const Style& s) { EmitShape(fx::ShapeKind::RoundRect, r, s, nullptr); }

    void Painter::Capsule(const wgt::Rect& r, const Style& s)
    {
        Style c = s;
        c.Radius(std::min(r.Width(), r.Height()) * 0.5f);
        EmitShape(fx::ShapeKind::RoundRect, r, c, nullptr);
    }

    void Painter::Circle(Vec2 center, float radius, const Style& s)
    {
        Style c = s;
        c.Radius(radius);
        c.smoothing = 0.0f;
        EmitShape(fx::ShapeKind::RoundRect, wgt::Rect(center.x - radius, center.y - radius, center.x + radius, center.y + radius), c, nullptr);
    }

    void Painter::Arc(Vec2 center, float radius, float thickness, float startRad, float sweepRad, const Style& s)
    {
        if (std::fabs(sweepRad) < 1e-4f)
            return;
        if (sweepRad < 0.0f)
        {
            startRad += sweepRad;
            sweepRad = -sweepRad;
        }
        const float ext = radius + thickness * 0.5f;
        float extra[4] = {radius, thickness * 0.5f, startRad, std::min(sweepRad, kTau)};
        EmitShape(fx::ShapeKind::Arc, wgt::Rect(center.x - ext, center.y - ext, center.x + ext, center.y + ext), s, extra);
    }

    void Painter::Ring(Vec2 center, float radius, float thickness, const Style& s) { Arc(center, radius, thickness, 0.0f, kTau, s); }

    void Painter::Line(Vec2 a, Vec2 b, float thickness, const Style& s)
    {
        const float h = thickness * 0.5f;
        wgt::Rect bounds(std::min(a.x, b.x) - h, std::min(a.y, b.y) - h, std::max(a.x, b.x) + h, std::max(a.y, b.y) + h);
        float extra[4] = {a.x, a.y, b.x, b.y};
        Style c = s;
        c.radii[0] = h;
        EmitShape(fx::ShapeKind::Segment, bounds, c, extra);
    }

    void Painter::Merge(const wgt::Rect& a, const wgt::Rect& b, float radius, float smoothness, const Style& s)
    {
        Style c = s;
        c.Radius(radius);
        float extra[4] = {b.min.x, b.min.y, b.max.x, b.max.y};
        // smoothness is carried through Style-independent params
        mergeRadius_ = radius;
        mergeSmooth_ = smoothness;
        EmitShape(fx::ShapeKind::RoundRect, a, c, extra);
        mergeSmooth_ = -1.0f;
    }

    void Painter::EmitShape(fx::ShapeKind kind, const wgt::Rect& bounds, const Style& s, const float* extra)
    {
        if (!dl_ || (bounds.Empty() && kind == fx::ShapeKind::RoundRect))
            return;
        Context::Impl* impl = CurrentImpl();
        const Theme* th = impl ? &impl->theme.current : nullptr;
        const float scale = th ? th->metrics.scale : 1.0f;

        fx::Instance inst;
        std::memset(&inst, 0, sizeof(inst));
        std::uint32_t feat = 0;

        SetRect(inst.rect, bounds);
        if (kind == fx::ShapeKind::Arc)
        {
            Set4(inst.radii, extra[0], extra[1], 0, 0);
            Set4(inst.shape, 0, 0, extra[2], extra[3]);
        }
        else if (kind == fx::ShapeKind::Segment)
        {
            Set4(inst.radii, s.radii[0], 0, 0, 0);
            Set4(inst.shape2, extra[0], extra[1], extra[2], extra[3]);
        }
        else
        {
            Set4(inst.radii, s.radii[0], s.radii[1], s.radii[2], s.radii[3]);
            const float smoothing = s.smoothing >= 0.0f ? s.smoothing : (th ? th->metrics.cornerSmoothing : 0.6f);
            inst.shape[1] = smoothing;
            if (extra && mergeSmooth_ >= 0.0f)
            {
                feat |= fx::kMerge;
                Set4(inst.shape2, extra[0], extra[1], extra[2], extra[3]);
                Set4(inst.shape2Params, mergeRadius_, std::max(mergeSmooth_, 0.01f), 0, 0);
            }
        }

        // paint
        if (s.hasFill && (s.fill.a.a > 0.0f || s.fill.b.a > 0.0f || s.image != ImTextureID_Invalid))
        {
            feat |= fx::kFill;
            SetColor(inst.fill0, s.fill.a);
            SetColor(inst.fill1, s.fill.b);
            inst.flags[1] = (std::uint32_t)s.fill.kind;
            switch (s.fill.kind)
            {
            case fx::PaintKind::Linear: Set4(inst.fillParams, s.fill.angle, 0, 0, 0); break;
            case fx::PaintKind::Radial: Set4(inst.fillParams, s.fill.center.x, s.fill.center.y, s.fill.radius, 0); break;
            case fx::PaintKind::Conic: Set4(inst.fillParams, s.fill.center.x, s.fill.center.y, s.fill.angle, s.fill.loop ? 1.0f : 0.0f); break;
            case fx::PaintKind::Spectrum: Set4(inst.fillParams, s.fill.angle, s.fill.center.x, s.fill.center.y, 0); break;
            default: break;
            }
        }
        if (s.image != ImTextureID_Invalid)
        {
            feat |= fx::kImage | fx::kFill;
            Set4(inst.uvRect, s.uv0.x, s.uv0.y, s.uv1.x, s.uv1.y);
        }

        // stroke (glass gets an automatic specular hairline rim)
        float strokeWidth = s.strokeWidth;
        Color strokeColor = s.strokeColor;
        float strokeAlign = s.strokeAlign, fadeTo = s.strokeFadeTo, fadeAngle = s.strokeFadeAngle;
        if (s.hasGlass && strokeWidth <= 0.0f && s.glass.rim.a > 0.0f)
        {
            strokeWidth = std::max(1.0f, 1.0f * scale);
            strokeColor = s.glass.rim;
            strokeAlign = 0.0f;
            // a light rim is a highlight (strongest facing the light); a dark one is the edge's shade, strongest
            // away from it and never gone (the outline that defines light glass on a light background)
            const bool shade = s.glass.rim.r + s.glass.rim.g + s.glass.rim.b < 1.5f;
            fadeTo = shade ? 0.55f : 0.25f;
            fadeAngle = shade ? s.glass.lightAngle : s.glass.lightAngle + kPi;
        }
        if (strokeWidth > 0.0f && strokeColor.a > 0.0f)
        {
            feat |= fx::kStroke;
            SetColor(inst.stroke, strokeColor);
            float dir = fadeAngle;
            // strokeFadeAngle points from "bright" to "faded": convert to the gradient direction used by the shader
            Set4(inst.strokeParams, strokeWidth, strokeAlign, fadeTo, dir);
        }

        // shadow
        if (s.shadowColor.a > 0.0f && (s.shadowBlur > 0.0f || s.shadowSpread != 0.0f || s.shadowOffset.x != 0.0f || s.shadowOffset.y != 0.0f))
        {
            feat |= fx::kShadow;
            if (s.shadowInset)
                feat |= fx::kInnerShadow;
            SetColor(inst.shadow, s.shadowColor);
            Set4(inst.shadowParams, s.shadowBlur, s.shadowSpread, s.shadowOffset.x, s.shadowOffset.y);
        }

        // glow
        if (s.glowColor.a > 0.0f)
        {
            SetColor(inst.glow, s.glowColor);
            if (s.glowRadius > 0.0f && s.glowIntensity > 0.0f)
                feat |= fx::kGlow;
            if (s.innerGlowRadius > 0.0f && s.innerGlowIntensity > 0.0f)
                feat |= fx::kInnerGlow;
            Set4(inst.glowParams, s.glowRadius, s.glowIntensity, s.innerGlowRadius, s.innerGlowIntensity);
        }

        // glass
        if (s.hasGlass)
        {
            feat |= fx::kGlass;
            const GlassMaterial& g = s.glass;
            Set4(inst.glass, g.blur * scale, g.refraction * scale, g.bezel * scale, g.dispersion);
            SetColor(inst.glassTint, g.tint);
            Set4(inst.glassParams, g.saturation, g.brightness, g.specular, g.lightAngle);
            inst.shape[0] = g.legibility;
            inst.shape2Params[2] = std::max(g.magnify, 0.0f);
        }
        const float noise = s.noise >= 0.0f ? s.noise : (s.hasGlass ? s.glass.noise : 0.0f);
        if (noise > 0.0f)
            feat |= fx::kNoise;

        if (s.shimmer > 0.0f)
            feat |= fx::kShimmer;
        Set4(inst.misc, Saturate(s.opacity * alpha_), noise, s.shimmer, s.shimmerSpeed);

        if (s.effect != 0)
        {
            feat |= fx::kCustom;
            Set4(inst.custom, s.custom[0], s.custom[1], s.custom[2], s.custom[3]);
        }

        if (maskDepth_ > 0)
        {
            feat |= fx::kMask;
            SetRect(inst.mask, masks_[maskDepth_ - 1]);
            const float smoothing = th ? th->metrics.cornerSmoothing : 0.6f;
            Set4(inst.maskParams, maskRadius_[maskDepth_ - 1], smoothing, 0, 0);
        }

        if ((feat & (fx::kFill | fx::kStroke | fx::kShadow | fx::kGlow | fx::kInnerGlow | fx::kGlass | fx::kCustom)) == 0)
            return;
        if (inst.misc[0] <= 0.0f)
            return;

        inst.flags[0] = feat;
        inst.flags[2] = (std::uint32_t)kind;
        // layouts reserve glow room from the resting geometry: press / pop scale animations must not re-flow
        wgt::Rect restShape(inst.rect[0], inst.rect[1], inst.rect[2], inst.rect[3]);
        if (feat & fx::kMerge)
            restShape = restShape.Union(wgt::Rect(inst.shape2[0], inst.shape2[1], inst.shape2[2], inst.shape2[3]));
        const float restGlow = inst.glowParams[0];
        ApplyScale(inst);

        // glows stay off neighbouring items: layouts reserve room for them, and whatever still reaches a
        // neighbour fades out before its edge (halo bounds from the window's item map)
        if ((feat & fx::kGlow) && s.containGlow && impl)
        {
            wgt::Rect shape(inst.rect[0], inst.rect[1], inst.rect[2], inst.rect[3]);
            if (feat & fx::kMerge)
                shape = shape.Union(wgt::Rect(inst.shape2[0], inst.shape2[1], inst.shape2[2], inst.shape2[3]));
            const float r = inst.glowParams[0];
            LayoutReportOverflow(*impl, dl_, restShape, restGlow * 0.75f);
            wgt::Rect bound;
            float fade = 0.0f;
            if (ComputeGlowHalo(*impl, dl_, shape, r * 1.8f, bound, fade))
            {
                feat |= fx::kHalo;
                inst.flags[0] = feat;
                SetRect(inst.halo, bound);
                inst.maskParams[2] = fade;
            }
        }

        // coarse CPU culling against the current clip rect
        float pad = 2.0f;
        if (feat & fx::kShadow)
            pad = std::max(pad, inst.shadowParams[0] * 1.6f + std::max(inst.shadowParams[1], 0.0f) + std::max(std::fabs(inst.shadowParams[2]), std::fabs(inst.shadowParams[3])));
        if (feat & fx::kGlow)
            pad = std::max(pad, inst.glowParams[0] * 1.8f);
        wgt::Rect ext(inst.rect[0] - pad, inst.rect[1] - pad, inst.rect[2] + pad, inst.rect[3] + pad);
        if (feat & fx::kMerge)
            ext = ext.Union(wgt::Rect(inst.shape2[0] - pad, inst.shape2[1] - pad, inst.shape2[2] + pad, inst.shape2[3] + pad));
        const ImVec4& clip = dl_->_ClipRectStack.back();
        if (ext.max.x < clip.x || ext.max.y < clip.y || ext.min.x > clip.z || ext.min.y > clip.w)
            return;

        Emit(inst, s.effect, s.image);
    }

    void Painter::ApplyScale(fx::Instance& inst) const
    {
        for (int i = scaleDepth_ - 1; i >= 0; --i)
        {
            const Vec2 o = scaleOrigin_[i];
            const float k = scaleValue_[i];
            if (k == 1.0f)
                continue;
            Vec2 a = ScalePoint(Vec2(inst.rect[0], inst.rect[1]), o, k), b = ScalePoint(Vec2(inst.rect[2], inst.rect[3]), o, k);
            Set4(inst.rect, a.x, a.y, b.x, b.y);
            const std::uint32_t kind = inst.flags[2];
            if (kind == (std::uint32_t)fx::ShapeKind::Segment || (inst.flags[0] & fx::kMerge))
            {
                Vec2 c = ScalePoint(Vec2(inst.shape2[0], inst.shape2[1]), o, k), d = ScalePoint(Vec2(inst.shape2[2], inst.shape2[3]), o, k);
                Set4(inst.shape2, c.x, c.y, d.x, d.y);
                inst.shape2Params[0] *= k;
                inst.shape2Params[1] *= k;
            }
            for (float& r : inst.radii)
                r *= k;
            inst.strokeParams[0] *= k;
            inst.shadowParams[0] *= k;
            inst.shadowParams[1] *= k;
            inst.shadowParams[2] *= k;
            inst.shadowParams[3] *= k;
            inst.glowParams[0] *= k;
            inst.glowParams[2] *= k;
            inst.glass[1] *= k;
            inst.glass[2] *= k;
        }
    }

    void Painter::LightStreak(const wgt::Rect& r, float intensity, float thickness, float speed, float smile, float open, float time)
    {
        if (!dl_ || r.Empty() || intensity <= 0.0f || alpha_ <= 0.0f)
            return;
        fx::Instance inst;
        std::memset(&inst, 0, sizeof(inst));
        SetRect(inst.rect, r);
        inst.fill0[3] = intensity;
        inst.fill1[0] = time;
        Set4(inst.custom, smile, open, speed, thickness);
        std::uint32_t feat = fx::kCaustic;
        Set4(inst.misc, Saturate(alpha_), 0, 0, 0);
        if (maskDepth_ > 0)
        {
            feat |= fx::kMask;
            SetRect(inst.mask, masks_[maskDepth_ - 1]);
            Context::Impl* impl = CurrentImpl();
            Set4(inst.maskParams, maskRadius_[maskDepth_ - 1], impl ? impl->theme.current.metrics.cornerSmoothing : 0.6f, 0, 0);
        }
        inst.flags[0] = feat;
        inst.flags[2] = (std::uint32_t)fx::ShapeKind::RoundRect;
        ApplyScale(inst);
        Emit(inst);
    }

    void Painter::Emit(const fx::Instance& inst, EffectId effect, ImTextureID texture)
    {
        fx::ShapeCommand cmd;
        std::memset(&cmd, 0, sizeof(cmd));
        cmd.header.magic = fx::kMagic;
        cmd.header.kind = fx::CommandKind::Shape;
        cmd.header.effect = effect;
        cmd.header.texture = texture;
        cmd.instance = inst;
        dl_->AddCallback(fx::CommandCallback(), &cmd, sizeof(cmd));
    }

    // -------------------------------------------------------------- text
    float Painter::SnapToPixel(float v)
    {
        const float rs = ImGui::GetCurrentContext() ? ImGui::GetIO().DisplayFramebufferScale.x : 1.0f;
        return std::floor(v * rs + 0.5f) / rs;
    }

    Vec2 Painter::MeasureText(FontRef f, const char* text, const char* textEnd, float wrapWidth)
    {
        return wgt::MeasureText(f, text, textEnd, wrapWidth, 0).size;
    }

    Vec2 Painter::MeasureText(TextStyle style, const char* text, const char* textEnd, float wrapWidth)
    {
        return MeasureText(GetFont(style), text, textEnd, wrapWidth);
    }

    Vec2 Painter::Text(Vec2 pos, FontRef f, Color color, const char* text, const char* textEnd, float wrapWidth, std::uint32_t flags)
    {
        Context::Impl* impl = CurrentImpl();
        if (!impl || !impl->fonts.engine || !text)
            return Vec2(0, 0);
        TextEngine& engine = *impl->fonts.engine;
        const ShapedText* shaped = engine.Shape(f, text, textEnd, wrapWidth, flags);
        if (!shaped)
            return Vec2(0, 0);
        color.a *= alpha_;
        if (color.a > 0.0f)
        {
            if (scaleDepth_ > 0)
            {
                // under PushScale: draw at the final (scaled) geometry and keep PopScale off these vertices
                float k = 1.0f;
                Vec2 at = pos;
                for (int i = scaleDepth_ - 1; i >= 0; --i)
                {
                    at = ScalePoint(at, scaleOrigin_[i], scaleValue_[i]);
                    k *= scaleValue_[i];
                }
                const int start = dl_->VtxBuffer.Size;
                engine.Draw(dl_, *shaped, at, color.ToU32(), k);
                ExcludeFromScale(start);
            }
            else
                engine.Draw(dl_, *shaped, pos, color.ToU32());
        }
        return shaped->metrics.size;
    }

    Vec2 Painter::Text(Vec2 pos, TextStyle style, Color color, const char* text, const char* textEnd, float wrapWidth)
    {
        return Text(pos, GetFont(style), color, text, textEnd, wrapWidth, 0);
    }

    Vec2 Painter::TextBox(const wgt::Rect& r, Vec2 align, FontRef f, Color color, const char* text, const char* textEnd, std::uint32_t flags)
    {
        const bool trim = (flags & TextFlags_Ellipsis) != 0;
        const TextMetrics m = wgt::MeasureText(f, text, textEnd, trim ? std::max(r.Width(), 1.0f) : 0.0f, flags);
        const Vec2 pos(r.min.x + (r.Width() - m.size.x) * align.x, r.min.y + (r.Height() - m.size.y) * align.y);
        Text(pos, f, color, text, textEnd, trim ? std::max(r.Width(), 1.0f) : 0.0f, flags);
        return m.size;
    }

    Vec2 Painter::TextAligned(const wgt::Rect& r, Vec2 align, TextStyle style, Color color, const char* text, const char* textEnd)
    {
        return TextBox(r, align, GetFont(style), color, text, textEnd, 0);
    }

    void Painter::Icon(Vec2 center, wgt::Icon icon, float size, Color color)
    {
        if (icon == 0)
            return;
        color.a *= alpha_;
        Context::Impl* impl = CurrentImpl();
        if (color.a <= 0.0f || !impl || !impl->fonts.engine)
            return;
        if (scaleDepth_ > 0)
        {
            float k = 1.0f;
            Vec2 at = center;
            for (int i = scaleDepth_ - 1; i >= 0; --i)
            {
                at = ScalePoint(at, scaleOrigin_[i], scaleValue_[i]);
                k *= scaleValue_[i];
            }
            const int start = dl_->VtxBuffer.Size;
            impl->fonts.engine->DrawIcon(dl_, icon, size * k, at, color.ToU32());
            ExcludeFromScale(start);
            return;
        }
        impl->fonts.engine->DrawIcon(dl_, icon, size, center, color.ToU32());
    }

    void Painter::ExcludeFromScale(int vtxStart)
    {
        if (dl_->VtxBuffer.Size > vtxStart && excludedCount_ < kMaxExcluded)
        {
            excludedStart_[excludedCount_] = vtxStart;
            excludedEnd_[excludedCount_] = dl_->VtxBuffer.Size;
            ++excludedCount_;
        }
    }

    void Painter::Image(ImTextureID tex, const wgt::Rect& r, float radius, Color tint, Vec2 uv0, Vec2 uv1)
    {
        Style s;
        s.Radius(radius).Fill(tint).Image(tex, uv0, uv1);
        Rect(r, s);
    }

    void Painter::FillRect(const wgt::Rect& r, Color c, float rounding)
    {
        c.a *= alpha_;
        if (c.a > 0.0f)
            dl_->AddRectFilled(r.min, r.max, c.ToU32(), rounding);
    }

    void Painter::HLine(float x0, float x1, float y, Color c, float thickness)
    {
        c.a *= alpha_;
        if (c.a <= 0.0f)
            return;
        // hairlines land exactly on physical pixel rows (crisp at any DPI / render scale)
        const float rs = ImGui::GetIO().DisplayFramebufferScale.x;
        const float y0 = std::floor(y * rs) / rs;
        const float h = std::max(1.0f, std::floor(thickness * rs + 0.5f)) / rs;
        dl_->AddRectFilled(Vec2(x0, y0), Vec2(x1, y0 + h), c.ToU32());
    }

    void Painter::Polyline(const Vec2* points, int count, float thickness, const Style& s, std::uint32_t flags)
    {
        if (!dl_ || !points || count < 2 || thickness <= 0.0f)
            return;
        Color col = s.hasFill ? s.fill.a : (s.strokeColor.a > 0.0f ? s.strokeColor : Color::White());
        col.a *= Saturate(s.opacity) * alpha_;
        if (col.a <= 0.0f)
            return;
        ImVector<ImVec2>& path = tPath;
        BuildPath(points, count, flags, path);
        const int n = path.Size;
        if (n < 2)
            return;

        // one glow for the whole line: the stroke goes through a glow layer. The layer blurs the line's light
        // over the glow radius, so a thin line gets proportionally more of it; a glow in the line's own color
        // keeps the line's colors instead of re-tinting them
        const bool glow = s.glowRadius > 0.0f && s.glowIntensity > 0.0f && s.glowColor.a > 0.0f;
        if (glow)
        {
            const float scale = CurrentImpl() ? CurrentImpl()->theme.current.metrics.scale : 1.0f;
            const float thin = ImClamp(s.glowRadius / std::max(thickness, 1e-3f), 1.0f, 4.0f);
            const float dc = std::fabs(s.glowColor.r - col.r) + std::fabs(s.glowColor.g - col.g) + std::fabs(s.glowColor.b - col.b);
            BeginGlowLayer(Color(s.glowColor.r, s.glowColor.g, s.glowColor.b, dc < 0.05f ? 0.0f : 1.0f), s.glowRadius / std::max(scale, 1e-3f),
                           s.glowIntensity * s.glowColor.a * thin);
        }

        // cross-sections: outer fringe, solid core, solid core, outer fringe (1 physical pixel of anti-aliasing)
        const float rs = ImGui::GetIO().DisplayFramebufferScale.x > 0.0f ? ImGui::GetIO().DisplayFramebufferScale.x : 1.0f;
        const float aa = 1.0f / rs;
        float hw = thickness * 0.5f;
        if (thickness < aa)
        {
            col.a *= thickness / aa;   // hairline thinner than a pixel: fainter, not thinner
            hw = aa * 0.5f;
        }
        const float inner = std::max(hw - aa * 0.5f, 0.0f), outer = hw + aa * 0.5f;
        const ImU32 cIn = col.ToU32(), cOut = Color(col.r, col.g, col.b, 0.0f).ToU32();
        const ImVec2 uv = dl_->_Data->TexUvWhitePixel;
        const int capSeg = ImClamp((int)std::ceil(hw * rs * 1.2f), 3, 12);
        const int vtxCount = n * 4 + 2 * (1 + 2 * (capSeg + 1));
        const int idxCount = (n - 1) * 18 + 2 * (capSeg * 9);
        dl_->PrimReserve(idxCount, vtxCount);
        ImDrawVert* vw = dl_->_VtxWritePtr;
        ImDrawIdx* iw = dl_->_IdxWritePtr;
        const unsigned int base = dl_->_VtxCurrentIdx;
        auto vert = [&](ImVec2 p, ImU32 c) { vw->pos = p; vw->uv = uv; vw->col = c; ++vw; };
        auto tri = [&](unsigned int a, unsigned int b, unsigned int c) { *iw++ = (ImDrawIdx)a; *iw++ = (ImDrawIdx)b; *iw++ = (ImDrawIdx)c; };
        auto segNormal = [&](int i) {
            const ImVec2 d(path[i + 1].x - path[i].x, path[i + 1].y - path[i].y);
            const float l = std::sqrt(ImLengthSqr(d));
            return l > 0.0f ? ImVec2(-d.y / l, d.x / l) : ImVec2(0, 1);
        };
        ImVec2 nFirst(0, 1), nLast(0, 1);
        for (int i = 0; i < n; ++i)
        {
            ImVec2 m;
            float k = 1.0f;
            if (i == 0)
                m = nFirst = segNormal(0);
            else if (i == n - 1)
                m = nLast = segNormal(n - 2);
            else
            {
                const ImVec2 a = segNormal(i - 1), b = segNormal(i);
                m = ImVec2(a.x + b.x, a.y + b.y);
                const float l = std::sqrt(ImLengthSqr(m));
                m = l > 1e-4f ? ImVec2(m.x / l, m.y / l) : a;
                k = 1.0f / std::max(m.x * b.x + m.y * b.y, 0.5f);   // miter, clipped at 2x on sharp turns
            }
            const ImVec2 p = path[i];
            vert(ImVec2(p.x + m.x * outer * k, p.y + m.y * outer * k), cOut);
            vert(ImVec2(p.x + m.x * inner * k, p.y + m.y * inner * k), cIn);
            vert(ImVec2(p.x - m.x * inner * k, p.y - m.y * inner * k), cIn);
            vert(ImVec2(p.x - m.x * outer * k, p.y - m.y * outer * k), cOut);
            if (i > 0)
            {
                const unsigned int a = base + (i - 1) * 4, b = base + i * 4;
                for (unsigned int j = 0; j < 3; ++j)
                {
                    tri(a + j, a + j + 1, b + j + 1);
                    tri(a + j, b + j + 1, b + j);
                }
            }
        }
        // round caps: a half disc beyond each end, its rim anti-aliased
        unsigned int next = base + n * 4;
        for (int e = 0; e < 2; ++e)
        {
            const ImVec2 p = e == 0 ? path[0] : path[n - 1];
            const ImVec2 nm = e == 0 ? nFirst : nLast;
            const ImVec2 away = e == 0 ? ImVec2(nm.y, -nm.x) : ImVec2(-nm.y, nm.x);
            const unsigned int c0 = next;
            vert(p, cIn);
            for (int k = 0; k <= capSeg; ++k)
            {
                const float a = kPi * (float)k / (float)capSeg;
                const float ca = std::cos(a), sa = std::sin(a);
                const ImVec2 dir(nm.x * ca + away.x * sa, nm.y * ca + away.y * sa);
                vert(ImVec2(p.x + dir.x * inner, p.y + dir.y * inner), cIn);
                vert(ImVec2(p.x + dir.x * outer, p.y + dir.y * outer), cOut);
            }
            for (int k = 0; k < capSeg; ++k)
            {
                const unsigned int i0 = c0 + 1 + k * 2, i1 = i0 + 2;
                tri(c0, i0, i1);
                tri(i0, i0 + 1, i1 + 1);
                tri(i0, i1 + 1, i1);
            }
            next = c0 + 1 + 2 * (capSeg + 1);
        }
        dl_->_VtxWritePtr = vw;
        dl_->_IdxWritePtr = iw;
        dl_->_VtxCurrentIdx += (unsigned int)vtxCount;

        if (glow)
            EndGlowLayer();
    }

    void Painter::Area(const Vec2* points, int count, float baseline, const Paint& paint, std::uint32_t flags)
    {
        if (!dl_ || !points || count < 2)
            return;
        ImVector<ImVec2>& path = tPath;
        BuildPath(points, count, flags, path);
        const int n = path.Size;
        if (n < 2)
            return;
        // linear paint across the area's bounds (solid: both ends the same color)
        float x0 = path[0].x, x1 = x0, y0 = std::min(path[0].y, baseline), y1 = std::max(path[0].y, baseline);
        for (const ImVec2& p : path)
        {
            x0 = std::min(x0, p.x);
            x1 = std::max(x1, p.x);
            y0 = std::min(y0, p.y);
            y1 = std::max(y1, p.y);
        }
        const bool linear = paint.kind == fx::PaintKind::Linear;
        const ImVec2 dir(std::cos(paint.angle), std::sin(paint.angle));
        float lo = 1e30f, hi = -1e30f;
        const ImVec2 corners[4] = {ImVec2(x0, y0), ImVec2(x1, y0), ImVec2(x0, y1), ImVec2(x1, y1)};
        for (const ImVec2& c : corners)
        {
            const float d = c.x * dir.x + c.y * dir.y;
            lo = std::min(lo, d);
            hi = std::max(hi, d);
        }
        auto colorAt = [&](ImVec2 p) {
            Color c = paint.a;
            if (linear)
                c = Lerp(paint.a, paint.b, Saturate((p.x * dir.x + p.y * dir.y - lo) / std::max(hi - lo, 1e-4f)));
            c.a *= alpha_;
            return c.ToU32();
        };
        const ImVec2 uv = dl_->_Data->TexUvWhitePixel;
        dl_->PrimReserve((n - 1) * 6, n * 2);
        const unsigned int base = dl_->_VtxCurrentIdx;
        for (int i = 0; i < n; ++i)
        {
            const ImVec2 top = path[i], bot(path[i].x, baseline);
            dl_->PrimWriteVtx(top, uv, colorAt(top));
            dl_->PrimWriteVtx(bot, uv, colorAt(bot));
            if (i > 0)
            {
                const unsigned int a = base + (i - 1) * 2, b = base + i * 2;
                dl_->PrimWriteIdx((ImDrawIdx)a);
                dl_->PrimWriteIdx((ImDrawIdx)b);
                dl_->PrimWriteIdx((ImDrawIdx)(b + 1));
                dl_->PrimWriteIdx((ImDrawIdx)a);
                dl_->PrimWriteIdx((ImDrawIdx)(b + 1));
                dl_->PrimWriteIdx((ImDrawIdx)(a + 1));
            }
        }
    }

    // ------------------------------------------------------------- state
    void Painter::PushMask(const wgt::Rect& r, float radius)
    {
        IM_ASSERT(maskDepth_ < 8);
        if (maskDepth_ >= 8)
            return;
        masks_[maskDepth_] = r;
        maskRadius_[maskDepth_] = radius;
        ++maskDepth_;
    }

    void Painter::PopMask()
    {
        IM_ASSERT(maskDepth_ > 0);
        if (maskDepth_ > 0)
            --maskDepth_;
    }

    void Painter::PushClip(const wgt::Rect& r, bool intersect) { dl_->PushClipRect(r.min, r.max, intersect); }
    void Painter::PopClip() { dl_->PopClipRect(); }

    void Painter::PushScale(Vec2 origin, float scale)
    {
        IM_ASSERT(scaleDepth_ < 8);
        if (scaleDepth_ >= 8)
            return;
        scaleOrigin_[scaleDepth_] = origin;
        scaleValue_[scaleDepth_] = scale;
        scaleVtxStart_[scaleDepth_] = dl_->VtxBuffer.Size;
        ++scaleDepth_;
    }

    void Painter::PopScale()
    {
        IM_ASSERT(scaleDepth_ > 0);
        if (scaleDepth_ <= 0)
            return;
        --scaleDepth_;
        const float k = scaleValue_[scaleDepth_];
        const Vec2 o = scaleOrigin_[scaleDepth_];
        if (k != 1.0f)
        {
            int ex = 0;
            for (int i = scaleVtxStart_[scaleDepth_]; i < dl_->VtxBuffer.Size; ++i)
            {
                // text / icons were drawn at their final geometry (re-rasterized): skip their vertices
                while (ex < excludedCount_ && i >= excludedEnd_[ex])
                    ++ex;
                if (ex < excludedCount_ && i >= excludedStart_[ex])
                {
                    i = excludedEnd_[ex] - 1;
                    continue;
                }
                ImDrawVert& v = dl_->VtxBuffer[i];
                v.pos = ScalePoint(v.pos, o, k);
            }
        }
        if (scaleDepth_ == 0)
            excludedCount_ = 0;
    }

    void Painter::BeginGlowLayer(Color tint, float radius, float intensity, float contentOpacity)
    {
        fx::LayerCommand cmd;
        std::memset(&cmd, 0, sizeof(cmd));
        cmd.header.magic = fx::kMagic;
        cmd.header.kind = fx::CommandKind::LayerBegin;
        SetColor(cmd.params.color, tint);
        const float scale = CurrentImpl() ? CurrentImpl()->theme.current.metrics.scale : 1.0f;
        cmd.params.intensity = intensity * alpha_;
        cmd.params.radius = radius * scale;
        cmd.params.opacity = contentOpacity;
        dl_->AddCallback(fx::CommandCallback(), &cmd, sizeof(cmd));
    }

    void Painter::BeginEdgeFade(const wgt::Rect& region, float top, float bottom)
    {
        fx::FadeCommand cmd;
        std::memset(&cmd, 0, sizeof(cmd));
        cmd.header.magic = fx::kMagic;
        cmd.header.kind = fx::CommandKind::FadeBegin;
        cmd.params = {region.min.y, region.max.y, std::max(top, 0.0f), std::max(bottom, 0.0f)};
        dl_->AddCallback(fx::CommandCallback(), &cmd, sizeof(cmd));
    }

    void Painter::EndEdgeFade()
    {
        fx::CommandHeader h;
        std::memset(&h, 0, sizeof(h));
        h.magic = fx::kMagic;
        h.kind = fx::CommandKind::FadeEnd;
        dl_->AddCallback(fx::CommandCallback(), &h, sizeof(h));
    }

    void Painter::EndGlowLayer()
    {
        fx::CommandHeader h;
        std::memset(&h, 0, sizeof(h));
        h.magic = fx::kMagic;
        h.kind = fx::CommandKind::LayerEnd;
        dl_->AddCallback(fx::CommandCallback(), &h, sizeof(h));
    }
}
