// WGT UI - built-in overlays: the panel dock and the notification island.
#include "ui/ui_internal.hpp"
#include <algorithm>

namespace wgt
{
    using namespace ui::detail;

    // ================================================================= dock
    void DockFrame(Context::Impl& impl)
    {
        std::vector<std::shared_ptr<PanelEntry>> panels;
        {
            std::shared_lock lock(impl.panelMutex);
            for (auto& p : impl.panels)
                if (!(p->desc.flags & PanelFlags_HideFromDock))
                    panels.push_back(p);
        }
        if (panels.empty())
            return;

        const Theme& t = T();
        const Palette& c = t.colors;
        const ImGuiViewport* vp = ImGui::GetMainViewport();
        const float tile = Sc(46), gap = Sc(12), pad = Sc(11);
        const int n = (int)panels.size();
        const Vec2 size(n * tile + (n - 1) * gap + pad * 2.0f, tile + pad * 2.0f);
        const Vec2 pos(std::floor(vp->WorkPos.x + (vp->WorkSize.x - size.x) * 0.5f), std::floor(vp->WorkPos.y + vp->WorkSize.y - size.y - Sc(16)));
        // extra headroom above the dock for magnified tiles and tooltips
        const float headroom = Sc(30);
        ImGui::SetNextWindowPos(Vec2(pos.x - Sc(20), pos.y - headroom));
        ImGui::SetNextWindowSize(Vec2(size.x + Sc(40), size.y + headroom + Sc(10)));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        const ImGuiWindowFlags wf = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                                    ImGuiWindowFlags_NoScrollWithMouse;
        const bool open = ImGui::Begin("##wgt_dock", nullptr, wf);
        ImGui::PopStyleVar(2);
        if (!open)
        {
            ImGui::End();
            return;
        }
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        ImGui::BringWindowToDisplayFront(w);
        const Rect dock = Rect::FromSize(pos, size);
        Painter p(w->DrawList);
        ScopedUnclip unclip(w->DrawList, dock.Expanded(Sc(24)), ShadowExtent(Sc(26), Vec2(0, Sc(10))));   // shadow + magnified tiles
        p.Capsule(dock, Style().Radius(size.y * 0.5f).Glass(ui::LookMaterial(t.materials.bar)).Shadow(c.shadow.Fade(0.6f), Sc(26), Vec2(0, Sc(10))));   // the material carries its own veil

        const ImGuiIO& io = ImGui::GetIO();
        const bool mouseNear = io.MousePos.y > dock.min.y - Sc(40) && io.MousePos.y < dock.max.y + Sc(10) &&
                               io.MousePos.x > dock.min.x - Sc(30) && io.MousePos.x < dock.max.x + Sc(30);
        for (int i = 0; i < n; ++i)
        {
            PanelEntry& e = *panels[i];
            const Vec2 center(dock.min.x + pad + i * (tile + gap) + tile * 0.5f, dock.Center().y);
            const float dist = std::fabs(io.MousePos.x - center.x);
            const float magTarget = mouseNear ? Saturate(1.0f - dist / (tile * 2.4f)) : 0.0f;
            const ImGuiID id = w->GetID(e.id.c_str());
            const float mag = Anim(id, 0xD0, magTarget, t.motion.fast);
            const float s = 1.0f + 0.32f * mag;
            const Rect r = Rect::FromCenter(Vec2(center.x, dock.max.y - pad - tile * 0.5f * s), Vec2(tile * s, tile * s));
            ui::Interaction it = InteractImpl(id, Rect::FromCenter(center, Vec2(tile + gap, tile + pad)), ui::InteractFlags_None);
            const bool isOpen = e.open.load();
            if (it.pressed)
            {
                e.open = !isOpen;
                if (!isOpen)
                    e.focusRequest = true;
            }
            p.PushScale(r.Center(), 1.0f - 0.08f * it.press);
            const Color base = e.desc.iconColor.a > 0 ? e.desc.iconColor : c.accent;
            p.Rect(r, Style().Radius(r.Height() * 0.26f).Fill(Paint::Linear(base.Lighter(0.18f), base.Darker(0.12f), 90))
                          .Shadow(Color::Black(0.25f), Sc(8), Vec2(0, Sc(3))).Stroke(1.0f, Color::White(0.25f)).StrokeFade(0.0f, 90));
            p.Icon(r.Center(), e.desc.icon ? e.desc.icon : icons::Apps, r.Height() * 0.5f, Color::White());
            p.PopScale();
            const float dot = Anim(id, 0xD1, isOpen ? 1.0f : 0.0f, t.motion.standard);
            if (dot > 0.01f)
                p.Circle(Vec2(center.x, dock.max.y - Sc(4)), Sc(2.2f) * dot, Style().Fill(c.label.Fade(0.7f * dot)));
            if (it.hovered)
            {
                const FontRef f = GetFont(TextStyle::Footnote);
                const Vec2 ts = Painter::MeasureText(f, e.title.c_str());
                const Rect tip = Rect::FromCenter(Vec2(center.x, r.min.y - Sc(18)), Vec2(ts.x + Sc(18), ts.y + Sc(10)));
                p.Capsule(tip, Style().Glass(ui::LookMaterial(t.materials.popover)).Shadow(c.shadow, Sc(10), Vec2(0, Sc(3))));
                p.Text(Vec2(std::floor(tip.Center().x - ts.x * 0.5f), std::floor(tip.Center().y - ts.y * 0.5f)), f, c.label, e.title.c_str());
            }
        }
        ImGui::End();
    }

    // =============================================================== island
    // Notifications arrive like the Siri orb: the island opens into a card large enough for the whole message
    // and the full effect - a black body clearing into a lens drop that magnifies what lies below, with a
    // flowing streak of light where black turns into glass. Then it settles into a plain Dynamic Island pill
    // (icon, title, time left) until the notification ends. Live activities live in the plain pill; a newly
    // started one gets the same short arrival.
    namespace
    {
        struct IslandRuntime
        {
            bool hasItem = false;
            IslandItem item;
            double start = 0.0;
            std::vector<std::string> knownActs;   // activity ids already shown (a new one arrives once)
            double actStart = -100.0;
        };
    }

    void IslandFrame(Context::Impl& impl)
    {
        IslandRuntime& rt = impl.state.Get<IslandRuntime>(0x151A4Du);
        std::vector<IslandActivity> acts;
        {
            std::lock_guard lock(impl.islandMutex);
            if (rt.hasItem && impl.time - rt.start > rt.item.duration)
                rt.hasItem = false;
            if (!rt.hasItem && !impl.islandQueue.empty())
            {
                rt.item = std::move(impl.islandQueue.front());
                impl.islandQueue.erase(impl.islandQueue.begin());
                rt.hasItem = true;
                rt.start = impl.time;
            }
            acts = impl.activities;
        }
        {
            std::vector<std::string> ids;
            for (const IslandActivity& a : acts)
            {
                ids.push_back(a.id);
                if (std::find(rt.knownActs.begin(), rt.knownActs.end(), a.id) == rt.knownActs.end())
                    rt.actStart = impl.time;   // a new live activity: it arrives like a notification
            }
            rt.knownActs = std::move(ids);
        }

        const Theme& t = T();
        const ImGuiID id = 0x151A4Eu;
        const bool notif = rt.hasItem;
        const bool activity = !notif && !acts.empty();
        const bool shown = notif || activity;
        const double since = notif ? impl.time - rt.start : impl.time - rt.actStart;
        const float burstFor = notif ? std::min(1.35f, rt.item.duration * 0.45f) : 1.1f;
        const bool burst = shown && since < burstFor;

        const FontRef tf = Font(FontWeight::Semibold, 15.0f);
        const FontRef mf = Font(FontWeight::Regular, 13.5f);
        const FontRef cf = Font(FontWeight::Semibold, 13.0f);
        const char* label = notif ? rt.item.title.c_str() : (activity ? acts[0].title.c_str() : "");
        // the pill's width (resident) and the card's (arrival): each content shows once the shape fits it
        const float pillW = shown ? Clamp(Painter::MeasureText(cf, label).x + Sc(notif ? 100.0f : 112.0f), Sc(210), Sc(430)) : Sc(126);
        float cardW = Sc(126);
        if (notif)
            cardW = Clamp(std::max(Painter::MeasureText(tf, rt.item.title.c_str()).x, Painter::MeasureText(mf, rt.item.message.c_str()).x) + Sc(116), Sc(320), Sc(540));
        else if (activity)
            cardW = Clamp(Painter::MeasureText(cf, label).x + Sc(124), Sc(290), Sc(460));
        float body = Sc(34), drop = 0.0f, width = Sc(126);
        if (burst)
        {
            body = notif ? Sc(68) : Sc(50);
            width = cardW;
            drop = notif ? Sc(46) : Sc(40);   // the clear lens below: room for the full light
        }
        else if (shown)
        {
            body = Sc(38);
            width = pillW;
        }
        const Spring morph{0.46f, 0.70f};
        const float wv = Anim(id, 1, width, morph, Sc(126));
        const float hb = Anim(id, 5, body, morph, Sc(34));
        const float hd = std::max(Anim(id, 2, drop, morph, 0.0f), 0.0f);
        const float glow = Saturate(Anim(id, 6, burst ? 1.0f : 0.0f, burst ? t.motion.standard : t.motion.fast, 0.0f));   // the streak
        const float big = Saturate(Anim(id, 7, burst && notif ? 1.0f : 0.0f, morph, 0.0f));   // full card content vs pill content
        const float alpha = Anim(id, 3, shown ? 1.0f : 0.0f, shown ? t.motion.standard : t.motion.fast, 0.0f);
        if (alpha < 0.01f)
            return;

        const float hv = hb + hd;
        const ImGuiViewport* vp = ImGui::GetMainViewport();
        const Rect r = Rect::FromCenter(Vec2(vp->Pos.x + vp->Size.x * 0.5f, vp->Pos.y + Sc(10) + hv * 0.5f), Vec2(wv, hv));
        const float rad = std::min(hv * 0.5f, Sc(34));
        ImDrawList* dl = ImGui::GetForegroundDrawList();
        Painter p(dl);
        p.SetAlpha(alpha);
        const Color tint = notif ? (rt.item.tint.a > 0 ? rt.item.tint : t.colors.accent) : (activity && acts[0].tint.a > 0 ? acts[0].tint : t.colors.accent);
        const Color ink(0.012f, 0.012f, 0.016f, 1.0f);

        if (hd > 0.5f || glow > 0.01f)
        {
            // Inside the orb, drawn first so the glass on top transforms them: the black body clearing into the
            // drop, the strands of light where black turns into glass, a soft glow deeper in the drop
            p.PushMask(r, rad);
            const float fadeTop = r.min.y + hb - Sc(10);
            const float fadeBot = r.min.y + hb + hd * 0.55f;
            p.Rect(Rect(r.min.x, r.min.y, r.max.x, fadeTop + 1.0f), Style().Fill(ink));
            p.Rect(Rect(r.min.x, fadeTop, r.max.x, std::max(fadeBot, fadeTop + 1.0f)), Style().Fill(Paint::Linear(ink, ink.Fade(0.0f), 90)));
            if (glow > 0.01f)
            {
                // held together well inside the rim (so the lens bends the ends, not the whole length), opening in
                // the middle; its own clock from the arrival: a closed line that opens up
                const float cy = r.min.y + hb + hd * 0.12f;
                const Rect sr(r.Center().x - r.Width() * 0.36f, cy - Sc(28), r.Center().x + r.Width() * 0.36f, cy + Sc(28));
                p.LightStreak(sr, glow, Sc(0.85f), 1.0f, 0.0f, 0.28f, (float)std::max(since, 0.0));
            }
            p.Rect(Rect(r.Center().x - r.Width() * 0.3f, fadeBot - Sc(6), r.Center().x + r.Width() * 0.3f, r.max.y),
                   Style().Fill(Paint::Radial(Color::White(0.08f + 0.06f * glow), Color::White(0.0f), Vec2(0.5f, 0.35f), 0.6f)));
            p.PopMask();

            // the orb itself: clear glass, a strong dome lens - it bends the strands, magnifies what lies below
            // the drop and splits colors at its rim
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

        // content fades in once the shape is (almost) wide enough for it: the card's while arriving, the pill's
        // as soon as the card has shrunk into it (no empty black shape in between)
        const float fitCard = Saturate((wv - cardW * 0.75f) / std::max(cardW * 0.25f, 1.0f));
        const float fitPill = Saturate((wv - pillW * 0.75f) / std::max(pillW * 0.25f, 1.0f));
        const float ca = alpha;
        const Rect content(r.min.x, r.min.y, r.max.x, r.min.y + hb);
        const float cyc = content.Center().y;
        if (notif)
        {
            const float left = 1.0f - Saturate((float)((impl.time - rt.start) / std::max(rt.item.duration, 0.1f)));
            // full card (arrival)
            const float aBig = SmoothStep(0.55f, 1.0f, big) * fitCard * ca;
            if (aBig > 0.02f)
            {
                p.SetAlpha(aBig);
                const float tileS = Sc(44);
                const Rect tile = Rect::FromCenter(Vec2(r.min.x + Sc(16) + tileS * 0.5f, cyc), Vec2(tileS, tileS));
                p.Rect(tile, Style().Radius(tileS * 0.3f).Fill(Paint::Linear(tint.Lighter(0.2f), tint.Darker(0.1f), 90)));
                p.Icon(tile.Center(), rt.item.icon ? rt.item.icon : icons::Bell, tileS * 0.5f, Color::White());
                const float tx = tile.max.x + Sc(14);
                p.PushClip(Rect(tx, content.min.y, r.max.x - Sc(18), content.max.y));
                p.Text(Vec2(tx, std::floor(cyc - Sc(19))), tf, Color::White(), rt.item.title.c_str());
                p.Text(Vec2(tx, std::floor(cyc + Sc(1))), mf, Color::White(0.68f), rt.item.message.c_str());
                p.PopClip();
            }
            // plain pill (resident): icon, title, time left
            const float aPill = SmoothStep(0.55f, 1.0f, 1.0f - big) * fitPill * ca;
            if (aPill > 0.02f)
            {
                p.SetAlpha(aPill);
                const float tileS = Sc(24);
                const Rect tile = Rect::FromCenter(Vec2(r.min.x + Sc(9) + tileS * 0.5f, cyc), Vec2(tileS, tileS));
                p.Rect(tile, Style().Radius(tileS * 0.5f).Fill(tint));
                p.Icon(tile.Center(), rt.item.icon ? rt.item.icon : icons::Bell, tileS * 0.52f, Color::White());
                const Vec2 ts = Painter::MeasureText(cf, label);
                p.PushClip(Rect(tile.max.x + Sc(8), content.min.y, r.max.x - Sc(40), content.max.y));
                p.Text(Vec2(tile.max.x + Sc(10), std::floor(cyc - ts.y * 0.5f)), cf, Color::White(), label);
                p.PopClip();
                const Vec2 rc(r.max.x - Sc(21), cyc);
                p.Ring(rc, Sc(8), Sc(2.5f), Style().Fill(tint.Fade(0.25f)));
                p.Arc(rc, Sc(8), Sc(2.5f), -kPi * 0.5f, kTau * left, Style().Fill(tint));
            }
        }
        else if (activity)
        {
            if (fitPill * ca < 0.02f)
                return;
            p.SetAlpha(fitPill * ca);
            const IslandActivity& a = acts[0];
            p.Icon(Vec2(r.min.x + Sc(22), cyc), a.icon ? a.icon : icons::Sync, Sc(15), tint);
            const Vec2 ts = Painter::MeasureText(cf, a.title.c_str());
            p.PushClip(Rect(r.min.x + Sc(38), content.min.y, r.max.x - Sc(44), content.max.y));
            p.Text(Vec2(r.min.x + Sc(40), std::floor(cyc - ts.y * 0.5f)), cf, Color::White(), a.title.c_str());
            p.PopClip();
            const Vec2 rc(r.max.x - Sc(22), cyc);
            if (a.progress >= 0.0f)
            {
                const float done = Anim(id, 4, Saturate(a.progress), t.motion.standard);
                p.Ring(rc, Sc(9), Sc(3), Style().Fill(tint.Fade(0.25f)));
                p.Arc(rc, Sc(9), Sc(3), -kPi * 0.5f, kTau * done, Style().Fill(tint));
            }
            else
            {
                const float ang = (float)std::fmod(impl.time * 5.0, (double)kTau);
                p.Arc(rc, Sc(9), Sc(3), ang, kPi * 1.3f, Style().Fill(Paint::Conic(tint.Fade(0.0f), tint, Degrees(ang))));
            }
            if (acts.size() > 1)
            {
                char buf[16];
                std::snprintf(buf, sizeof(buf), "+%d", (int)acts.size() - 1);
                const FontRef bf = Font(FontWeight::Semibold, 11.0f);
                const Vec2 bs = Painter::MeasureText(bf, buf);
                p.Text(Vec2(rc.x - Sc(20) - bs.x, std::floor(cyc - bs.y * 0.5f)), bf, Color::White(0.6f), buf);
            }
        }
    }
}
