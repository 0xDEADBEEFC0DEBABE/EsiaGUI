// Esia UI - colors: iOS's color picker (a saturation / brightness spectrum over a hue bar, an opacity bar, the hex
// value, swatches) and the round color well that opens it in a glass popover.
#include "ui_internal.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>

namespace esia::ui
{
    using namespace detail;

    namespace
    {
        struct Hsv
        {
            float h = 0.0f, s = 0.0f, v = 0.0f;
        };

        Hsv ToHsv(const Color& c)
        {
            const float mx = std::max({c.r, c.g, c.b}), mn = std::min({c.r, c.g, c.b});
            const float d = mx - mn;
            Hsv o;
            o.v = mx;
            o.s = mx > 0.0f ? d / mx : 0.0f;
            if (d > 0.0f)
            {
                float h;
                if (mx == c.r)
                    h = (c.g - c.b) / d;
                else if (mx == c.g)
                    h = 2.0f + (c.b - c.r) / d;
                else
                    h = 4.0f + (c.r - c.g) / d;
                h /= 6.0f;
                o.h = h < 0.0f ? h + 1.0f : h;
            }
            return o;
        }

        std::string ToHex(const Color& c, bool alpha)
        {
            const auto b = [](float v) { return (int)std::lround(Saturate(v) * 255.0f); };
            char buf[16];
            if (alpha && c.a < 0.999f)
                std::snprintf(buf, sizeof(buf), "#%02X%02X%02X%02X", b(c.r), b(c.g), b(c.b), b(c.a));
            else
                std::snprintf(buf, sizeof(buf), "#%02X%02X%02X", b(c.r), b(c.g), b(c.b));
            return buf;
        }

        // #RGB, #RRGGBB, #RRGGBBAA (the # optional)
        bool FromHex(std::string_view text, Color& out, bool alpha)
        {
            std::string h;
            for (char ch : text)
                if (std::isxdigit((unsigned char)ch))
                    h += ch;
                else if (ch != '#' && ch != ' ')
                    return false;
            const auto nib = [](char ch) { return std::isdigit((unsigned char)ch) ? ch - '0' : (std::tolower((unsigned char)ch) - 'a' + 10); };
            const auto byte = [&](std::size_t i) { return (float)(nib(h[i]) * 16 + nib(h[i + 1])) / 255.0f; };
            if (h.size() == 3)
            {
                out = Color((float)nib(h[0]) / 15.0f, (float)nib(h[1]) / 15.0f, (float)nib(h[2]) / 15.0f, out.a);
                return true;
            }
            if (h.size() == 6 || (h.size() == 8 && alpha))
            {
                out = Color(byte(0), byte(2), byte(4), h.size() == 8 ? byte(6) : out.a);
                return true;
            }
            return false;
        }

        // The spectrum as one texture: 2 x 2 texels (white, the hue / black, black) whose bilinear filtering is exactly
        // the saturation / brightness plane, so the spectrum is one shape with one anti-aliased edge. (Three gradients
        // stacked on the same rounded rect blended each edge pixel three times: a fringe around it.) Made on first use,
        // rewritten when the hue changes, destroyed with the picker's state.
        struct SpectrumTexture
        {
            TextureRegistry* registry = nullptr;
            TextureId id = 0;
            float hue = -1.0f;
            SpectrumTexture() = default;
            SpectrumTexture(const SpectrumTexture&) = delete;
            SpectrumTexture& operator=(const SpectrumTexture&) = delete;
            ~SpectrumTexture()
            {
                if (registry && id)
                    registry->Destroy(id);
            }
            TextureId For(TextureRegistry& textures, float h)
            {
                if (!id)
                {
                    registry = &textures;
                    id = textures.Create(TextureInfo{TextureFormat::RGBA8, 2, 2});
                }
                if (id && h != hue)
                {
                    const std::uint32_t texels[4] = {Color::White().ToRgba8(), Color::Hsv(h, 1.0f, 1.0f).ToRgba8(), Color::Black().ToRgba8(),
                                                     Color::Black().ToRgba8()};
                    textures.Update(id, 0, 0, 2, 2, texels);
                    hue = h;
                }
                return id;
            }
        };

        struct PickerState
        {
            Hsv hsv;
            Color last = Color(-1, -1, -1, -1);   // the color the hsv belongs to (a change from outside re-reads it)
            std::string hex;
            bool hexEditing = false;
            SpectrumTexture spectrum;
        };

        // What lies under a translucent color (a checkerboard) is masked one physical pixel inside the shape's edge:
        // the color on top then draws the edge alone. Two layers anti-aliased along the same edge let the lower one
        // show through as a light rim.
        float OnePixel() { return 1.0f / std::max(Ctx().Scale(), 1e-3f); }

        // A checkerboard under translucent colors. The cells' edges lie on physical pixels: between pixels each cell's
        // anti-aliased edge blended into its neighbours as a faint seam (the mask still smooths the outline).
        void Checker(Painter& p, const Rect& r, float cell)
        {
            p.FillRect(r, Color::White());
            const int nx = (int)std::ceil(r.Width() / cell), ny = (int)std::ceil(r.Height() / cell);
            const auto edge = [&](float from, float to, int i) { return i <= 0 ? from : p.SnapToPixel(std::min(from + (float)i * cell, to)); };
            for (int y = 0; y < ny; ++y)
                for (int x = (y & 1); x < nx; x += 2)
                    p.FillRect(Rect(edge(r.min.x, r.max.x, x), edge(r.min.y, r.max.y, y), edge(r.min.x, r.max.x, x + 1), edge(r.min.y, r.max.y, y + 1)),
                               Color::Hex(0xC8C8C8));
        }

        // The round handle of the spectrum and the bars: white ring, the color inside; it swells into a lens while held.
        void Handle(Painter& p, Vec2 center, float radius, Color fill, float press)
        {
            const float r = radius * (1.0f + 0.25f * press);
            Style s;
            s.Shadow(Color::Black(0.25f + 0.1f * press), Sc(6 + 6 * press), Vec2(0, Sc(2)));
            s.Stroke(Sc(3), Color::White(), 1.0f);
            if (press > 0.02f)
                s.Glass(LensMaterial()).Fill(fill.WithAlpha(1.0f - 0.6f * Saturate(press)));
            else
                s.Fill(fill.WithAlpha(1.0f));
            ScopedUnclip unclip(Rect::FromCenter(center, Vec2(r * 2, r * 2)), Sc(16));
            p.Circle(center, r, s);
        }

        // The picker's height at a width: the spectrum, the bars, the hex row and the swatches (PickerAt's layout).
        float PickerHeight(float w, const ColorPickerOptions& o)
        {
            const float gap = Sc(14), barH = Sc(Sizes().colorBar), rowH = Sc(Sizes().row);
            float h = std::round(w * 0.62f) + gap + barH + gap * 0.75f;
            if (o.alpha)
                h += barH + gap * 0.75f;
            h += rowH;
            if (!o.swatches.empty())
            {
                const float d = Sc(26), sg = Sc(10);
                const int perRow = std::max(1, (int)((w + sg) / (d + sg)));
                const int rows = ((int)o.swatches.size() + perRow - 1) / perRow;
                h += gap * 0.75f + (float)rows * (d + sg) - sg;
            }
            return h;
        }

        bool PickerAt(Id id, const Rect& area, Color* color, const ColorPickerOptions& o)
        {
            Ui::Impl& m = M();
            Context& c = *m.ctx;
            const InputState& in = c.Input();
            const Palette& pc = C();
            PickerState& st = c.State<PickerState>(id);
            if (!(*color == st.last))
            {
                // a new color from outside: its hue, kept when it is gray (no hue of its own)
                const Hsv n = ToHsv(*color);
                st.hsv.v = n.v;
                st.hsv.s = n.s;
                if (n.s > 0.0f && n.v > 0.0f)
                    st.hsv.h = n.h;
                st.last = *color;
                if (!st.hexEditing)
                    st.hex = ToHex(*color, o.alpha);
            }
            Hsv& h = st.hsv;
            bool changed = false;
            const auto commit = [&] {
                const Color nc = Color::Hsv(h.h, h.s, h.v, color->a);
                *color = nc;
                st.last = nc;
                if (!st.hexEditing)
                    st.hex = ToHex(nc, o.alpha);
                changed = true;
            };

            const float w = area.Width();
            const float gap = Sc(14);
            float y = area.min.y;
            Painter p = GetPainter();

            // ---- the spectrum: saturation left to right, brightness top to bottom
            const Rect spec = Rect::FromSize(Vec2(area.min.x, y), Vec2(w, std::round(w * 0.62f)));
            const float sr = Sc(12);
            const InteractState si = InteractImpl(Salt(id, 1), spec, InteractFlags_PressOnClick);
            if (si.held && in.MouseValid())
            {
                h.s = Saturate((in.MousePos().x - spec.min.x) / spec.Width());
                h.v = 1.0f - Saturate((in.MousePos().y - spec.min.y) / spec.Height());
                commit();
            }
            // texel centers at a quarter and three quarters: the corners' colors at the corners. No outline here, on the
            // bars or on the swatch (iOS's picker has none): a 1-unit stroke read as a light rim on dark backgrounds.
            p.Image(st.spectrum.For(c.Textures(), h.h), spec, sr, Color::White(), Vec2(0.25f, 0.25f), Vec2(0.75f, 0.75f));
            const float sPress = LiquidPulse(Salt(id, 1), si.held, si.pressed);
            Handle(p, Vec2(spec.min.x + h.s * spec.Width(), spec.min.y + (1.0f - h.v) * spec.Height()), Sc(11), Color::Hsv(h.h, h.s, h.v), sPress);
            if (si.hovered || si.held)
                c.SetMouseCursor(MouseCursor::Hand);
            y = spec.max.y + gap;

            // ---- the hue bar: six gradients between the primaries and secondaries, in a capsule
            const float barH = Sc(Sizes().colorBar);
            const Rect hue = Rect::FromSize(Vec2(area.min.x, y), Vec2(w, barH));
            const float inset = barH * 0.5f;   // the handle's center stays inside the capsule's ends
            const InteractState hi = InteractImpl(Salt(id, 2), hue, InteractFlags_PressOnClick);
            if (hi.held && in.MouseValid())
            {
                h.h = std::min(Saturate((in.MousePos().x - hue.min.x - inset) / std::max(hue.Width() - inset * 2.0f, 1.0f)), 0.9999f);
                commit();
            }
            p.PushMask(hue, barH * 0.5f);
            for (int i = 0; i < 6; ++i)
            {
                // the segments meet on physical pixels: no overlap, no seam
                const float x0 = p.SnapToPixel(hue.min.x + inset + (hue.Width() - inset * 2.0f) * (float)i / 6.0f);
                const float x1 = p.SnapToPixel(hue.min.x + inset + (hue.Width() - inset * 2.0f) * (float)(i + 1) / 6.0f);
                // the ends keep their pure color under the capsule's round caps
                if (i == 0)
                    p.FillRect(Rect(hue.min.x, hue.min.y, x0, hue.max.y), Color::Hsv(0.0f, 1.0f, 1.0f));
                p.Rect(Rect(x0, hue.min.y, x1, hue.max.y), Style().Fill(Paint::Linear(Color::Hsv((float)i / 6.0f, 1, 1), Color::Hsv((float)(i + 1) / 6.0f, 1, 1), 0.0f)));
                if (i == 5)
                    p.FillRect(Rect(x1, hue.min.y, hue.max.x, hue.max.y), Color::Hsv(0.0f, 1.0f, 1.0f));
            }
            p.PopMask();
            Handle(p, Vec2(hue.min.x + inset + h.h * (hue.Width() - inset * 2.0f), hue.Center().y), Sc(10), Color::Hsv(h.h, 1, 1),
                   LiquidPulse(Salt(id, 2), hi.held, hi.pressed));
            y = hue.max.y + gap * 0.75f;

            // ---- the opacity bar: the color from clear to opaque over a checkerboard
            if (o.alpha)
            {
                const Rect bar = Rect::FromSize(Vec2(area.min.x, y), Vec2(w, barH));
                const InteractState ai = InteractImpl(Salt(id, 3), bar, InteractFlags_PressOnClick);
                if (ai.held && in.MouseValid())
                {
                    color->a = Saturate((in.MousePos().x - bar.min.x - inset) / std::max(bar.Width() - inset * 2.0f, 1.0f));
                    commit();
                }
                const float px = OnePixel();
                p.PushMask(bar.Expanded(-px), barH * 0.5f - px);
                Checker(p, bar, Sc(6));
                p.PopMask();
                p.PushMask(bar, barH * 0.5f);
                const Color opaque = Color::Hsv(h.h, h.s, h.v);
                const float g0 = p.SnapToPixel(bar.min.x + inset), g1 = p.SnapToPixel(bar.max.x - inset);   // clear left of g0
                p.Rect(Rect(g0, bar.min.y, g1, bar.max.y), Style().Fill(Paint::Linear(opaque.WithAlpha(0.0f), opaque, 0.0f)));
                p.FillRect(Rect(g1, bar.min.y, bar.max.x, bar.max.y), opaque);
                p.PopMask();
                Handle(p, Vec2(bar.min.x + inset + color->a * (bar.Width() - inset * 2.0f), bar.Center().y), Sc(10), opaque,
                       LiquidPulse(Salt(id, 3), ai.held, ai.pressed));
                y = bar.max.y + gap * 0.75f;
            }

            // ---- the result beside the hex value
            const float rowH = Sc(Sizes().row);
            const Rect swatch = Rect::FromSize(Vec2(area.min.x, y), Vec2(Sc(54), rowH));
            {
                const float px = OnePixel();
                p.PushMask(swatch.Expanded(-px), Sc(9) - px);
                Checker(p, swatch, Sc(6));
                p.PopMask();
                p.PushMask(swatch, Sc(9));
                p.FillRect(swatch, *color);
                p.PopMask();
            }
            if (o.hex)
            {
                const Rect field(swatch.max.x + Sc(10), y, area.max.x, y + rowH);
                TextFieldOptions to;
                to.clearButton = false;
                to.selectOnFocus = true;
                const Id fid = Salt(id, 4);
                const bool wasEditing = st.hexEditing;
                const TextFieldResult tr = TextFieldAt(fid, field, &st.hex, "#RRGGBB", to);
                st.hexEditing = c.KeyboardFocusId() == fid;
                Color parsed = *color;
                if ((tr.changed || (wasEditing && !st.hexEditing)) && FromHex(st.hex, parsed, o.alpha))
                {
                    if (!(parsed == *color))
                    {
                        *color = parsed;
                        st.last = Color(-1, -1, -1, -1);   // re-read the hsv next frame
                        changed = true;
                    }
                }
                if (wasEditing && !st.hexEditing)
                    st.hex = ToHex(*color, o.alpha);
            }
            y += rowH;

            // ---- swatches
            if (!o.swatches.empty())
            {
                y += gap * 0.75f;
                const float d = Sc(26), sg = Sc(10);
                const int perRow = std::max(1, (int)((w + sg) / (d + sg)));
                for (std::size_t i = 0; i < o.swatches.size(); ++i)
                {
                    const int col = (int)i % perRow, row = (int)i / perRow;
                    const Vec2 cc(area.min.x + d * 0.5f + (float)col * (d + sg), y + d * 0.5f + (float)row * (d + sg));
                    const InteractState it = InteractImpl(Salt(id, 0x100 + (std::uint32_t)i), Rect::FromCenter(cc, Vec2(d, d)), InteractFlags_None);
                    const Color sc = o.swatches[i];
                    const bool current = ToHex(sc, true) == ToHex(*color, true);
                    p.PushScale(cc, 1.0f + 0.1f * it.hover - 0.08f * it.press);
                    p.Circle(cc, d * 0.5f, Style().Fill(sc).Stroke(1.0f, pc.separator));
                    if (current)
                        p.Ring(cc, d * 0.5f + Sc(3.5f), Sc(2), Style().Fill(pc.label.Fade(0.85f)));
                    p.PopScale();
                    if (it.pressed)
                    {
                        *color = sc;
                        changed = true;
                    }
                }
            }
            return changed;
        }
    }

    bool ColorPicker(std::string_view id, Color* color, const ColorPickerOptions& o)
    {
        ItemScope scope;
        Context& c = Ctx();
        const float width = o.width > 0.0f ? Sc(o.width) : std::min(AvailableWidth(), Sc(340));
        const float height = PickerHeight(width, o);
        const Vec2 pos = c.CursorPos();
        c.ItemSize(Vec2(width, height));
        return color && PickerAt(c.GetId(id), Rect::FromSize(pos, Vec2(width, height)), color, o);
    }

    bool ColorWell(std::string_view id, Color* color, const ColorPickerOptions& o, float diameter)
    {
        ItemScope scope;
        Context& c = Ctx();
        const float d = diameter > 0.0f ? Sc(diameter) : Sc(30);
        const Vec2 pos = c.CursorPos();
        c.ItemSize(Vec2(d, d));
        const Id wid = c.GetId(id);
        const Rect r = Rect::FromSize(pos, Vec2(d, d));
        const InteractState it = InteractImpl(wid, r, InteractFlags_None);
        if (!color)
            return false;
        Painter p = GetPainter();
        const Vec2 cc = r.Center();
        p.PushScale(cc, 1.0f + 0.06f * it.hover - 0.08f * it.press);
        // the spectrum ring of UIColorWell, then the color inside it
        const int segments = 36;
        const float ringW = d * 0.13f;
        for (int i = 0; i < segments; ++i)
            p.Arc(cc, d * 0.5f - ringW * 0.5f, ringW, kPi * 2.0f * (float)i / (float)segments - kPi * 0.5f, kPi * 2.0f / (float)segments + 0.02f,
                  Style().Fill(Color::Hsv((float)i / (float)segments, 0.85f, 1.0f)));
        const float inner = d * 0.5f - ringW - Sc(2);
        if (color->a < 0.999f)
        {
            const float px = OnePixel();
            const Rect ir = Rect::FromCenter(cc, Vec2(inner * 2 - px * 2, inner * 2 - px * 2));
            p.PushMask(ir, inner - px);
            Checker(p, ir, Sc(4));
            p.PopMask();
        }
        p.Circle(cc, inner, Style().Fill(*color).Stroke(1.0f, C().separator));
        p.PopScale();

        const Id popId = Salt(wid, 0xC011);
        if (it.pressed)
            c.OpenPopup(popId);
        bool changed = false;
        PopupOptions po;
        po.pos = Vec2(r.Center().x, r.max.y + Sc(8));
        po.pivot = Vec2(0.5f, 0.0f);
        const float width = o.width > 0.0f ? Sc(o.width) : Sc(300);
        if (BeginGlassPopup(popId, po, width + Sc(12)))
        {
            ColorPickerOptions popupOptions = o;
            popupOptions.width = width / T().metrics.scale;
            changed = ColorPicker("##picker", color, popupOptions);
            EndGlassPopup();
        }
        return changed;
    }
}
