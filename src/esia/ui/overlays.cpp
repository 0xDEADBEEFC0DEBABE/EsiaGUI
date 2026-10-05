// Esia UI - the overlays of an app: the dock of its panels and the island of notifications and live activities
// (WGT's overlay.cpp). The dock is a widget the app calls every frame; the island draws itself at Ui::EndFrame from
// what Notify / SetActivity (any thread) queued.
#include "ui_internal.hpp"
#include <algorithm>
#include <cstdio>

namespace esia::ui
{
    using namespace detail;

    // ================================================================= dock
    int Dock(std::span<const DockItem> items)
    {
        const int n = (int)items.size();
        if (n == 0)
            return -1;
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const Theme& t = T();
        const Palette& pc = t.colors;
        const InputState& in = c.Input();
        const float tile = Sc(46), gap = Sc(12), pad = Sc(11);
        const Vec2 size((float)n * tile + (float)(n - 1) * gap + pad * 2.0f, tile + pad * 2.0f);
        const Vec2 display = c.DisplaySize();
        const Rect dock = Rect::FromSize(Vec2(std::floor((display.x - size.x) * 0.5f), std::floor(display.y - size.y - Sc(16))), size);

        // a window of the dock's own size in the overlay layer: nothing around the dock takes the mouse; magnified
        // tiles and labels draw past it
        c.SetNextWindowPos(dock.min);
        c.SetNextWindowSize(dock.Size());
        esia::WindowOptions wo;
        wo.flags = esia::WindowFlags_NoMove | esia::WindowFlags_NoResize | esia::WindowFlags_NoScroll | esia::WindowFlags_NoFocus | esia::WindowFlags_NoBringToFront;
        wo.layer = WindowLayer::Overlay;
        wo.padding = Vec2(0, 0);
        wo.minSize = Vec2(0, 0);
        c.Begin("##esia_dock", wo);
        int clicked = -1;
        {
            Painter p = GetPainter();
            ScopedUnclip unclip(dock.Expanded(Sc(24), Sc(60)), ShadowExtent(Sc(26), Vec2(0, Sc(10))));
            p.Capsule(dock, Style().Glass(LookMaterial(t.materials.bar)).Shadow(pc.shadow.Fade(0.6f), Sc(26), Vec2(0, Sc(10))));   // the material carries its veil

            const bool near = in.MouseValid() && in.MousePos().y > dock.min.y - Sc(40) && in.MousePos().y < dock.max.y + Sc(10) &&
                              in.MousePos().x > dock.min.x - Sc(30) && in.MousePos().x < dock.max.x + Sc(30);
            for (int i = 0; i < n; ++i)
            {
                const DockItem& item = items[(std::size_t)i];
                const Vec2 center(dock.min.x + pad + (float)i * (tile + gap) + tile * 0.5f, dock.Center().y);
                // tiles magnify toward the mouse and grow upward from the dock's floor
                const float magTarget = near ? Saturate(1.0f - std::fabs(in.MousePos().x - center.x) / (tile * 2.4f)) : 0.0f;
                c.PushId(i);
                const Id id = c.GetId("##tile");
                c.PopId();
                const float mag = Anim(id, 0xD0, magTarget, t.motion.fast);
                const float s = 1.0f + 0.32f * mag;
                const Rect r = Rect::FromCenter(Vec2(center.x, dock.max.y - pad - tile * 0.5f * s), Vec2(tile * s, tile * s));
                const Interaction it = InteractImpl(id, Rect::FromCenter(center, Vec2(tile + gap, tile + pad)), InteractFlags_None);
                const bool isOpen = item.open && *item.open;
                if (it.pressed)
                {
                    clicked = i;
                    if (item.open)
                        *item.open = !isOpen;
                }
                p.PushScale(r.Center(), 1.0f - 0.08f * it.press);
                const Color base = item.color.a > 0.0f ? item.color : pc.accent;
                p.Rect(r, Style()
                              .Radius(r.Height() * 0.26f)
                              .Fill(Paint::Linear(base.Lighter(0.18f), base.Darker(0.12f), 90))
                              .Shadow(Color::Black(0.25f), Sc(8), Vec2(0, Sc(3)))
                              .Stroke(1.0f, Color::White(0.25f))
                              .StrokeFade(0.0f, 90));
                DrawIcon(p, r.Center(), item.icon ? item.icon : icons::Apps, r.Height() * 0.5f, Color::White());
                p.PopScale();
                const float dot = Anim(id, 0xD1, isOpen ? 1.0f : 0.0f, t.motion.standard);
                if (dot > 0.01f)
                    p.Circle(Vec2(center.x, dock.max.y - Sc(4)), Sc(2.2f) * dot, Style().Fill(pc.label.Fade(0.7f * dot)));
                if (it.hovered && !item.label.empty())
                {
                    // the label above the tile, on the popover glass
                    const text::FontRef f = Font(TextStyle::Footnote);
                    const Vec2 ts = MeasureText(f, item.label);
                    const Rect tip = Rect::FromCenter(Vec2(center.x, r.min.y - Sc(18)), Vec2(ts.x + Sc(18), ts.y + Sc(10)));
                    p.Capsule(tip, Style().Glass(LookMaterial(t.materials.popover)).Shadow(pc.shadow, Sc(10), Vec2(0, Sc(3))));
                    p.Text(Vec2(std::floor(tip.Center().x - ts.x * 0.5f), std::floor(tip.Center().y - ts.y * 0.5f)), f, pc.label, item.label);
                }
            }
        }
        c.End();
        return clicked;
    }

    // =============================================================== island
    // Notifications arrive like the Siri orb: the island opens into a card large enough for the whole message and the
    // full effect - a black body clearing into a lens drop that magnifies what lies below, with a flowing streak of
    // light where black turns into glass. Then it settles into a plain Dynamic Island pill (icon, title, time left)
    // until the notification ends. Live activities live in the plain pill; a newly started one gets the same short
    // arrival. On a display with a camera housing at the top (its safe area starts well below the island: a phone's
    // Dynamic Island or notch) the island grows out of the housing as the system's does: the card's content goes
    // below it, and the pill shows only what fits beside the camera - the icon and the time left, no title.
    void Ui::Notify(const Notification& n)
    {
        std::lock_guard lock(impl_->islandMutex);
        impl_->islandQueue.push_back(n);
    }

    void Ui::SetActivity(std::string_view id, std::string_view title, float progress, Icon icon, Color tint)
    {
        std::lock_guard lock(impl_->islandMutex);
        auto& acts = impl_->activities;
        auto it = std::find_if(acts.begin(), acts.end(), [&](const Impl::Activity& a) { return a.id == id; });
        if (it == acts.end())
            it = acts.insert(acts.end(), Impl::Activity{std::string(id)});
        it->title.assign(title);
        it->progress = progress;
        it->icon = icon;
        it->tint = tint;
    }

    void Ui::ClearActivity(std::string_view id)
    {
        std::lock_guard lock(impl_->islandMutex);
        std::erase_if(impl_->activities, [&](const Impl::Activity& a) { return a.id == id; });
    }

    void detail::IslandFrame()
    {
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        Ui::Impl::IslandState& rt = m.island;
        std::vector<Ui::Impl::Activity>& acts = rt.acts;   // assigned in place: its strings keep their storage
        {
            std::lock_guard lock(m.islandMutex);
            if (rt.hasItem && m.time - rt.start > rt.item.duration)
                rt.hasItem = false;
            if (!rt.hasItem && !m.islandQueue.empty())
            {
                rt.item = std::move(m.islandQueue.front());
                m.islandQueue.erase(m.islandQueue.begin());
                rt.hasItem = true;
                rt.start = m.time;
            }
            acts = m.activities;
        }
        for (const Ui::Impl::Activity& a : acts)
            if (std::find(rt.knownActs.begin(), rt.knownActs.end(), a.id) == rt.knownActs.end())
                rt.actStart = m.time;   // a new live activity: it arrives like a notification
        rt.knownActs.resize(acts.size());
        for (std::size_t i = 0; i < acts.size(); ++i)
            rt.knownActs[i] = acts[i].id;

        const Theme& t = T();
        const Id id = 0x151A4Eu;
        const bool notif = rt.hasItem;
        const bool activity = !notif && !acts.empty();
        const bool shown = notif || activity;
        const double since = notif ? m.time - rt.start : m.time - rt.actStart;
        const float burstFor = notif ? std::min(1.35f, rt.item.duration * 0.45f) : 1.1f;
        const bool burst = shown && since < burstFor;
        if (shown)
            m.animating = true;   // the time left, the streak

        const float safeTop = c.SafeArea().min.y;
        const bool housing = safeTop > Sc(30);
        const float below = housing ? std::max(0.0f, safeTop - Sc(20)) : 0.0f;   // the card's content starts under it

        const text::FontRef tf = Font(FontWeight::Semibold, 15.0f);
        const text::FontRef mf = Font(FontWeight::Regular, 13.5f);
        const text::FontRef cf = Font(FontWeight::Semibold, 13.0f);
        const std::string_view label = notif ? std::string_view(rt.item.title) : (activity ? std::string_view(acts[0].title) : std::string_view());
        // the pill's width (resident) and the card's (arrival): each content shows once the shape fits it
        const float pillW = !shown ? Sc(126) : housing ? Sc(210) : Clamp(MeasureText(cf, label).x + Sc(notif ? 100.0f : 112.0f), Sc(210), Sc(430));
        float cardW = Sc(126);
        if (notif)
            cardW = Clamp(std::max(MeasureText(tf, rt.item.title).x, MeasureText(mf, rt.item.message).x) + Sc(116), Sc(320), Sc(540));
        else if (activity)
            cardW = Clamp(MeasureText(cf, label).x + Sc(124), Sc(290), Sc(460));
        float body = Sc(34), drop = 0.0f, width = Sc(126);
        if (burst)
        {
            body = (notif ? Sc(68) : Sc(50)) + below;
            width = cardW;
            drop = notif ? Sc(46) : Sc(40);   // the clear lens below: room for the full light
        }
        else if (shown)
        {
            body = Sc(38);
            width = pillW;
        }
        static constexpr Spring kMorph{0.46f, 0.70f};
        const float wv = Anim(id, 1, width, kMorph, Sc(126));
        const float hb = Anim(id, 5, body, kMorph, Sc(34));
        const float hd = std::max(Anim(id, 2, drop, kMorph, 0.0f), 0.0f);
        const float glow = Saturate(Anim(id, 6, burst ? 1.0f : 0.0f, burst ? t.motion.standard : t.motion.fast, 0.0f));   // the streak
        const float big = Saturate(Anim(id, 7, burst && notif ? 1.0f : 0.0f, kMorph, 0.0f));   // the card's content vs the pill's
        const float alpha = Anim(id, 3, shown ? 1.0f : 0.0f, shown ? t.motion.standard : t.motion.fast, 0.0f);
        const float under = std::max(Anim(id, 8, burst ? below : 0.0f, kMorph, 0.0f), 0.0f);   // the content's offset
        if (alpha < 0.01f)
            return;

        const float hv = hb + hd;
        const Vec2 display = c.DisplaySize();
        const Rect r = Rect::FromCenter(Vec2(display.x * 0.5f, Sc(10) + hv * 0.5f), Vec2(wv, hv));
        const float rad = std::min(hv * 0.5f, Sc(34));
        PainterEnv env;
        env.metricsScale = t.metrics.scale;
        env.cornerSmoothing = t.metrics.cornerSmoothing;
        env.pixelScale = c.FramebufferScale().x;
        env.text = m.text;
        Painter p(c.ForegroundDrawList(), env);
        p.SetAlpha(alpha);
        const Color tint = notif ? (rt.item.tint.a > 0 ? rt.item.tint : t.colors.accent) : (activity && acts[0].tint.a > 0 ? acts[0].tint : t.colors.accent);
        const Color ink(0.012f, 0.012f, 0.016f, 1.0f);

        if (hd > 0.5f || glow > 0.01f)
        {
            // inside the orb, drawn first so the glass on top transforms them: the black body clearing into the drop,
            // the strands of light where black turns into glass, a soft glow deeper in the drop
            p.PushMask(r, rad);
            const float fadeTop = r.min.y + hb - Sc(10);
            const float fadeBot = r.min.y + hb + hd * 0.55f;
            p.Rect(Rect(r.min.x, r.min.y, r.max.x, fadeTop + 1.0f), Style().Fill(ink));
            p.Rect(Rect(r.min.x, fadeTop, r.max.x, std::max(fadeBot, fadeTop + 1.0f)), Style().Fill(Paint::Linear(ink, ink.Fade(0.0f), 90)));
            if (glow > 0.01f)
            {
                // held together well inside the rim (the lens bends the ends, not the whole length), opening in the
                // middle; its own clock from the arrival: a closed line that opens up
                const float cy = r.min.y + hb + hd * 0.12f;
                const Rect sr(r.Center().x - r.Width() * 0.36f, cy - Sc(28), r.Center().x + r.Width() * 0.36f, cy + Sc(28));
                p.LightStreak(sr, glow, Sc(0.85f), 1.0f, 0.0f, 0.28f, (float)std::max(since, 0.0));
            }
            p.Rect(Rect(r.Center().x - r.Width() * 0.3f, fadeBot - Sc(6), r.Center().x + r.Width() * 0.3f, r.max.y),
                   Style().Fill(Paint::Radial(Color::White(0.08f + 0.06f * glow), Color::White(0.0f), Vec2(0.5f, 0.35f), 0.6f)));
            p.PopMask();

            // the orb itself: clear glass, a strong dome lens - it bends the strands, magnifies what lies below the
            // drop and splits colors at its rim
            GlassMaterial lens = t.materials.clear;
            lens.blur = 0.0f;
            lens.tint = Color::Clear();
            lens.legibility = 0.0f;
            lens.saturation = 1.0f;
            lens.refraction = 9.0f;
            lens.bezel = 80.0f;
            lens.dispersion = 0.35f;
            lens.magnify = 0.22f * Saturate(hd / Sc(36));
            lens.specular = 1.0f;
            lens.rim = Color(1, 1, 1, 0.22f);
            p.Rect(r, Style().Radius(rad).Glass(lens).Shadow(Color::Black(0.30f), Sc(20), Vec2(0, Sc(8))));
        }
        else
        {
            // the plain Dynamic Island
            p.Rect(r, Style().Radius(rad).Fill(ink).Stroke(1.0f, Color::White(0.08f)).Shadow(Color::Black(0.30f), Sc(18), Vec2(0, Sc(6))));
        }

        // content fades in once the shape is (almost) wide enough for it: the card's while arriving, the pill's as
        // soon as the card has shrunk into it (no empty black shape in between)
        const float fitCard = Saturate((wv - cardW * 0.75f) / std::max(cardW * 0.25f, 1.0f));
        const float fitPill = Saturate((wv - pillW * 0.75f) / std::max(pillW * 0.25f, 1.0f));
        const Rect content(r.min.x, r.min.y + under, r.max.x, r.min.y + hb);
        const float cyc = content.Center().y;
        if (notif)
        {
            const float left = 1.0f - Saturate((float)((m.time - rt.start) / std::max(rt.item.duration, 0.1f)));
            // the full card (arrival)
            const float aBig = SmoothStep(0.55f, 1.0f, big) * fitCard * alpha;
            if (aBig > 0.02f)
            {
                p.SetAlpha(aBig);
                const float tileS = Sc(44);
                const Rect tileR = Rect::FromCenter(Vec2(r.min.x + Sc(16) + tileS * 0.5f, cyc), Vec2(tileS, tileS));
                p.Rect(tileR, Style().Radius(tileS * 0.3f).Fill(Paint::Linear(tint.Lighter(0.2f), tint.Darker(0.1f), 90)));
                DrawIcon(p, tileR.Center(), rt.item.icon ? rt.item.icon : icons::Bell, tileS * 0.5f, Color::White());
                const float tx = tileR.max.x + Sc(14);
                p.TextBox(Rect(tx, std::floor(cyc - Sc(19)), r.max.x - Sc(18), std::floor(cyc)), Vec2(0, 0), tf, Color::White(), rt.item.title, text::TextFlags_Ellipsis);
                p.TextBox(Rect(tx, std::floor(cyc + Sc(1)), r.max.x - Sc(18), std::floor(cyc + Sc(20))), Vec2(0, 0), mf, Color::White(0.68f), rt.item.message,
                          text::TextFlags_Ellipsis);
            }
            // the plain pill (resident): icon, title, time left
            const float aPill = SmoothStep(0.55f, 1.0f, 1.0f - big) * fitPill * alpha;
            if (aPill > 0.02f)
            {
                p.SetAlpha(aPill);
                const float tileS = Sc(24);
                const Rect tileR = Rect::FromCenter(Vec2(r.min.x + Sc(9) + tileS * 0.5f, cyc), Vec2(tileS, tileS));
                p.Rect(tileR, Style().Radius(tileS * 0.5f).Fill(tint));
                DrawIcon(p, tileR.Center(), rt.item.icon ? rt.item.icon : icons::Bell, tileS * 0.52f, Color::White());
                const Vec2 ts = MeasureText(cf, label);
                if (!housing)
                    p.TextBox(Rect(tileR.max.x + Sc(10), std::floor(cyc - ts.y * 0.5f), r.max.x - Sc(40), std::floor(cyc + ts.y * 0.5f + 1.0f)), Vec2(0, 0), cf,
                              Color::White(), label, text::TextFlags_Ellipsis);
                const Vec2 rc(r.max.x - Sc(21), cyc);
                p.Ring(rc, Sc(8), Sc(2.5f), Style().Fill(tint.Fade(0.25f)));
                p.Arc(rc, Sc(8), Sc(2.5f), -kPi * 0.5f, kTau * left, Style().Fill(tint));
            }
        }
        else if (activity)
        {
            if (fitPill * alpha < 0.02f)
                return;
            p.SetAlpha(fitPill * alpha);
            const Ui::Impl::Activity& a = acts[0];
            DrawIcon(p, Vec2(r.min.x + Sc(22), cyc), a.icon ? a.icon : icons::Sync, Sc(15), tint);
            const Vec2 ts = MeasureText(cf, a.title);
            if (!housing)
                p.TextBox(Rect(r.min.x + Sc(40), std::floor(cyc - ts.y * 0.5f), r.max.x - Sc(44), std::floor(cyc + ts.y * 0.5f + 1.0f)), Vec2(0, 0), cf,
                          Color::White(), a.title, text::TextFlags_Ellipsis);
            const Vec2 rc(r.max.x - Sc(22), cyc);
            if (a.progress >= 0.0f)
            {
                const float done = Anim(id, 4, Saturate(a.progress), t.motion.standard);
                p.Ring(rc, Sc(9), Sc(3), Style().Fill(tint.Fade(0.25f)));
                p.Arc(rc, Sc(9), Sc(3), -kPi * 0.5f, kTau * done, Style().Fill(tint));
            }
            else
            {
                const float ang = (float)std::fmod(m.time * 5.0, (double)kTau);
                p.Arc(rc, Sc(9), Sc(3), ang, kPi * 1.3f, Style().Fill(Paint::Conic(tint.Fade(0.0f), tint, Degrees(ang))));
            }
            if (acts.size() > 1 && !housing)   // (beside a camera there is room for the icon and the ring only)
            {
                char buf[16];
                std::snprintf(buf, sizeof(buf), "+%d", (int)acts.size() - 1);
                const text::FontRef bf = Font(FontWeight::Semibold, 11.0f);
                const Vec2 bs = MeasureText(bf, buf);
                p.Text(Vec2(rc.x - Sc(20) - bs.x, std::floor(cyc - bs.y * 0.5f)), bf, Color::White(0.6f), buf);
            }
        }
    }
}
