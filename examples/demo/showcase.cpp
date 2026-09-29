// WGT demo - showcase panels. Everything here uses only the public WGT API (like a game or plugin would).
#include "showcase.hpp"
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace wgt;
namespace icon = wgt::icons;

namespace
{
    // ------------------------------------------------------------ demo state
    struct Demo
    {
        Context* ctx = nullptr;
        ShowcaseOptions opt;

        // appearance
        bool darkMode = true;
        int accent = 0;
        GlassMaterial windowGlass;
        bool glassEdited = false;
        float frost = 10.0f, refraction = 14.0f, dispersion = 0.30f;
        bool specular = true;
        bool searchBase = true;   // base color inside the floating search bar (off: nothing but clear glass)
        int glassLook = 2;        // GlassLook of the whole UI (Frosted)

        // control center
        bool airplane = false, cellular = true, wifi = true, bluetooth = true, focus = false, rotation = true;
        float brightness = 0.72f, volume = 0.45f;
        bool playing = true;

        // components
        bool toggleA = true, toggleB = false;
        bool checkA = true, checkB = false;
        float slider = 0.42f, sliderB = 64.0f;
        int stepper = 3, segment = 1, picker = 1, tab = 0, swatch = 0;
        char name[64] = "Waffle";
        char search[64] = "";
        char settingsSearch[64] = "";
        char langInput[256] = "";
        char password[64] = "secret";

        // effects
        EffectId aurora = 0;
        float shimmerLoad = 0.0f;

        // telemetry (written by the worker thread)
        Property<float> cpu{0.3f}, gpu{0.5f}, download{0.0f};
        Property<int> players{12};
        Channel<std::string> logIn;
        std::vector<std::string> log;
        std::atomic<bool> running{false};
        std::atomic<bool> activity{true};
        std::thread worker;
        float fpsHistory[120] = {};
        int fpsCursor = 0;
        bool chartGlow = false;
        bool clearComponents = false;   // Components panel: every component in the tab fully transparent
        bool styledToggle = true;       // per-component style examples
        float styledSlider = 0.6f;
        int styledSegment = 1;
    };
    Demo* D = nullptr;

    const Color kAccents[] = {Color::Hex(0x0A84FF), Color::Hex(0xBF5AF2), Color::Hex(0xFF375F), Color::Hex(0xFF9F0A),
                              Color::Hex(0x30D158), Color::Hex(0x64D2FF), Color::Hex(0xFFD60A)};

    const char* kAuroraHlsl = R"(
float4 WgtEffect(WgtFx fx)
{
    float t = fx.time * 0.55;
    float2 uv = fx.uv;
    float w1 = sin(uv.x * 5.0 + t * 1.3 + sin(uv.y * 3.0 + t)) * 0.5 + 0.5;
    float w2 = sin(uv.y * 4.0 - t * 1.1 + cos(uv.x * 2.5 - t * 0.7)) * 0.5 + 0.5;
    float3 col = lerp(lerp(float3(0.10, 0.95, 0.75), float3(0.45, 0.25, 1.00), w1), float3(1.0, 0.35, 0.65), w2 * 0.65);
    float3 behind = WgtBackdrop(fx.screenUV, 18.0);
    col = lerp(behind, col, 0.70);
    col += pow(saturate(1.0 - abs(fx.sd) / 3.0), 3.0) * 0.35;   // bright rim
    float a = fx.coverage * fx.params.x;
    return float4(col * a, a);
}
)";

    // -------------------------------------------------------------- helpers
    float S(float v) { return ui::S(v); }

    void ApplyGlass()
    {
        Theme t = D->ctx->GetTheme();
        t.materials.window.blur = D->frost;
        t.materials.window.refraction = D->refraction;
        t.materials.window.dispersion = D->dispersion;
        const float spec = D->specular ? 1.0f : 0.0f;
        const Theme base = D->darkMode ? ThemeDark() : ThemeLight();
        t.materials.window.specular = base.materials.window.specular * spec;
        t.materials.bar.specular = base.materials.bar.specular * spec;
        t.materials.control.specular = base.materials.control.specular * spec;
        D->ctx->SetTheme(t, false);
    }

    // =============================================================== worker
    void WorkerMain()
    {
        std::mt19937 rng(1234);
        std::uniform_real_distribution<float> n01(0.0f, 1.0f);
        const char* messages[][2] = {
            {"Match found", "Ranked • Summoner's Rift is ready"},
            {"Achievement unlocked", "Liquid Glass Enthusiast"},
            {"Friend online", "Gamerdoc just came online"},
            {"Shaders compiled", "1,248 pipelines cached in 2.3 s"},
        };
        const Icon msgIcons[] = {icon::Game, icon::StarFill, icon::People, icon::Lightning};
        const Color msgTints[] = {Color::Hex(0x30D158), Color::Hex(0xFFD60A), Color::Hex(0x0A84FF), Color::Hex(0xBF5AF2)};
        int tick = 0, msg = 0;
        float dl = 0.0f;
        while (D->running.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            ++tick;
            // telemetry - lock-free Property<float> writes from this thread
            D->cpu.Update([&](float& v) { v = Clamp(v + (n01(rng) - 0.5f) * 0.06f, 0.08f, 0.95f); });
            D->gpu.Update([&](float& v) { v = Clamp(v + (n01(rng) - 0.48f) * 0.05f, 0.15f, 0.98f); });
            if (tick % 40 == 0)
                D->players.Update([&](int& p) { p = std::max(1, p + (int)(n01(rng) * 5.0f) - 2); });

            if (D->activity.load())
            {
                dl += 0.004f + n01(rng) * 0.004f;
                if (dl >= 1.0f)
                {
                    D->ctx->ClearActivity("download");
                    Notification n;
                    n.title = "Download complete";
                    n.message = "HD texture pack • 2.4 GB installed";
                    n.icon = icon::Download;
                    n.tint = Color::Hex(0x30D158);
                    D->ctx->Notify(n);
                    D->logIn.Push("[worker] download finished");
                    dl = -1.5f;   // pause before the next download
                }
                else if (dl >= 0.0f)
                    D->ctx->SetActivity("download", "Downloading texture pack", dl, icon::Download, Color::Hex(0x30D158));
                D->download.Set(std::max(dl, 0.0f));
            }
            if (tick % 180 == 90)
            {
                Notification n;
                n.title = messages[msg % 4][0];
                n.message = messages[msg % 4][1];
                n.icon = msgIcons[msg % 4];
                n.tint = msgTints[msg % 4];
                D->ctx->Notify(n);
                char buf[128];
                std::snprintf(buf, sizeof(buf), "[worker] notify: %s", n.title);
                D->logIn.Push(buf);
                ++msg;
            }
            if (tick % 20 == 0)
            {
                char buf[96];
                std::snprintf(buf, sizeof(buf), "[worker] cpu %.0f%%  gpu %.0f%%", D->cpu.Get() * 100.0f, D->gpu.Get() * 100.0f);
                D->logIn.Push(buf);
            }
        }
    }

    // ============================================================== widgets
    // A custom widget built only from public pieces: ui::Interact + Painter (iOS Control Center slider).
    // Its width is whatever the layout offers (ui::AvailableWidth), so it works in stacks and grids.
    bool TallSlider(const char* id, float* value, Icon ic, float height)
    {
        ui::Interaction it = ui::Interact(id, Vec2(ui::AvailableWidth(), height), ui::InteractFlags_PressOnClick);
        if (!it.visible)
            return false;
        bool changed = false;
        if (it.held)
        {
            const float v = Saturate(1.0f - (ImGui::GetIO().MousePos.y - it.rect.min.y) / it.rect.Height());
            changed = v != *value;
            *value = v;
        }
        const Theme& t = CurrentTheme();
        const float shown = anim::Float(it.id, *value, it.held ? nullptr : &t.motion.standard);
        Painter p;
        const Rect r = it.rect;
        const float radius = S(22);
        p.PushScale(r.Center(), 1.0f + 0.035f * it.press);
        p.Rect(r, Style().Radius(radius).Glass(ui::LookMaterial(t.materials.control)).Shadow(t.colors.shadow, S(16), Vec2(0, S(6))));
        p.PushMask(r, radius);
        const float fillTop = r.max.y - r.Height() * shown;
        p.Rect(Rect(r.min.x, fillTop, r.max.x, r.max.y), Style().Fill(Color::White(0.92f)));
        p.PopMask();
        const bool covered = fillTop < r.max.y - S(34);
        p.Icon(Vec2(r.Center().x, r.max.y - S(24)), ic, S(20), covered ? Color::Hex(0x3A3A3C) : t.colors.label);
        p.PopScale();
        return changed;
    }

    void GlassTile(const Rect& r, float radius = 22.0f)
    {
        const Theme& t = CurrentTheme();
        Painter p;
        Style s = Style().Radius(S(radius)).Glass(ui::LookMaterial(t.materials.control)).Shadow(t.colors.shadow, S(14), Vec2(0, S(5)));
        if (ui::CurrentGlassLook() == GlassLook::Theme)
            s.Fill(t.colors.windowSurface.Fade(0.4f));
        p.Rect(r, s);
    }

    // A Control Center module: a glass tile as wide as its layout slot (height < 0 = square). The group makes
    // the module one child of the surrounding grid / stack; its contents are placed inside the tile rect.
    Rect BeginModule(float height = -1.0f)
    {
        ImGui::BeginGroup();
        const float w = ui::AvailableWidth();
        const Rect r = Rect::FromSize(ImGui::GetCursorScreenPos(), Vec2(w, height < 0.0f ? w : height));
        GlassTile(r);
        return r;
    }

    void EndModule(const Rect& r)
    {
        ImGui::SetCursorScreenPos(r.min);
        ImGui::Dummy(r.Size());
        ImGui::EndGroup();
    }

    // Round toggle centered on `center`, `diameter` in UI units.
    void ModuleToggle(const char* id, bool* on, Icon ic, Color tint, Vec2 center, float diameter)
    {
        ImGui::SetCursorScreenPos(center - Vec2(diameter * 0.5f, diameter * 0.5f));
        ui::ToggleButtonOptions o;
        o.tint = tint;
        o.diameter = diameter / S(1.0f);
        ui::ToggleButton(id, on, ic, o);
    }

    // =============================================================== panels
    void PanelSettings(Context& ctx)
    {
        const Theme& t = ctx.GetTheme();
        if (!ui::BeginNavigation("settings_nav", "root"))
            return;

        if (ui::BeginPage("root", "Settings"))
        {
            // profile row (custom row content)
            if (ui::BeginSection())
            {
                Rect content;
                if (ui::BeginRow("profile", 76, &content))
                {
                    Painter p;
                    const Vec2 c(content.min.x + S(28), content.Center().y);
                    p.Circle(c, S(28), Style().Fill(Paint::Linear(Color::Hex(0xBF5AF2), Color::Hex(0x0A84FF), 45)).Shadow(t.colors.shadow, S(8), Vec2(0, S(3))));
                    p.TextAligned(Rect::FromCenter(c, Vec2(S(56), S(56))), Vec2(0.5f, 0.5f), TextStyle::Title2, Color::White(), "W");
                    // text boxes end before the chevron and ellipsize: nothing overlaps at any width
                    const float tx = content.min.x + S(70), tr = content.max.x - S(22);
                    p.TextBox(Rect(tx, content.Center().y - S(20), tr, content.Center().y), Vec2(0, 0), GetFont(TextStyle::Headline), t.colors.label,
                              "Waffle Player", nullptr, TextFlags_Ellipsis);
                    p.TextBox(Rect(tx, content.Center().y + S(2), tr, content.Center().y + S(20)), Vec2(0, 0), GetFont(TextStyle::Footnote),
                              t.colors.secondaryLabel, "Account, Cloud Saves & Achievements", nullptr, TextFlags_Ellipsis);
                    p.Icon(Vec2(content.max.x - S(6), content.Center().y), icon::ChevronRight, S(11), t.colors.tertiaryLabel);
                }
                ui::EndRow();
                ui::EndSection();
            }

            if (ui::BeginSection("Appearance"))
            {
                if (ui::RowToggle("Dark Mode", &D->darkMode, {icon::Moon, Color::Hex(0x5E5CE6)}))
                {
                    ctx.SetDarkMode(D->darkMode);
                    D->glassEdited = false;
                }
                if (ui::RowNavigation("Accent Color", nullptr, {icon::Palette, kAccents[D->accent]}))
                    ui::NavigationPush("accent");
                static const char* sizes[] = {"S", "M", "L"};
                static int sizeSel = 1;
                if (ui::RowSegmented("Text Size", &sizeSel, sizes, 3, {icon::ZoomIn, Color::Hex(0x0A84FF)}))
                    ctx.SetUiScale(sizeSel == 0 ? 0.87f : sizeSel == 1 ? 1.0f : 1.18f);   // multiplier on top of the monitor DPI
                ui::EndSection();
            }

            if (ui::BeginSection("Display", "The frame limit uses wgt::FramePacer (high-resolution timer, no busy wait)."))
            {
                ShowcaseDisplay& dsp = ShowcaseDisplaySettings();
                bool vs = dsp.vsync.load();
                if (ui::RowToggle("VSync", &vs, {icon::FullScreen, Color::Hex(0x0A84FF)}))
                    dsp.vsync = vs;
                static const char* limits[] = {"Off", "60", "120", "144"};
                static const int limitValues[] = {0, 60, 120, 144};
                int sel = 0;
                for (int i = 0; i < 4; ++i)
                    if (limitValues[i] == dsp.fpsLimit.load())
                        sel = i;
                if (ui::RowSegmented("Frame Limit", &sel, limits, 4, {icon::Lightning, Color::Hex(0xFF9F0A)}))
                    dsp.fpsLimit = limitValues[sel];
                static const char* smoothing[] = {"Auto", "Gray", "LCD"};
                int aa = (int)ctx.GetTextAntialiasing();
                if (ui::RowSegmented("Text", &aa, smoothing, 3, {icon::Edit, Color::Hex(0x64D2FF)}))
                    ctx.SetTextAntialiasing((TextAntialiasing)aa);
                char rate[64];
                std::snprintf(rate, sizeof(rate), "%.0f fps  ·  %.2f ms", dsp.fps.load(), dsp.frameMs.load());
                ui::RowValue("Frame Rate", rate, {icon::Diagnostic, Color::Hex(0x30D158)});
                ui::RowValue("Threads", dsp.threading, {icon::Sync, Color::Hex(0x5E5CE6)});
                ui::EndSection();
            }

            if (ui::BeginSection("Liquid Glass", "Edits the live theme: every glass surface re-renders with the new material."))
            {
                bool edited = false;
                edited |= ui::RowSlider("Frost", &D->frost, 0.0f, 48.0f, {icon::Cloud, Color::Hex(0x64D2FF)}, "%.0f");
                edited |= ui::RowSlider("Lensing", &D->refraction, 0.0f, 40.0f, {icon::View, Color::Hex(0x30D158)}, "%.0f");
                edited |= ui::RowSlider("Dispersion", &D->dispersion, 0.0f, 1.0f, {icon::Brightness, Color::Hex(0xFF9F0A)}, "%.2f");
                edited |= ui::RowToggle("Specular Rim", &D->specular, {icon::Lightbulb, Color::Hex(0xFFD60A)});
                static const char* kLooks[] = {"Theme", "Clear", "Frosted"};
                if (ui::RowSegmented("Glass", &D->glassLook, kLooks, 3, {icon::View, Color::Hex(0x0A84FF)}))
                    ctx.SetGlassLook((GlassLook)D->glassLook);
                ui::RowToggle("Search Bar Base", &D->searchBase, {icon::Search, Color::Hex(0x8E8E93)});
                if (edited)
                    ApplyGlass();
                ui::EndSection();
            }

            if (ui::BeginSection("Notifications"))
            {
                if (ui::RowButton("Send Test Notification"))
                {
                    Notification n;
                    n.title = "Hello from the UI thread";
                    n.message = "Notifications can be posted from any thread";
                    n.icon = icon::Bell;
                    ctx.Notify(n);
                }
                bool act = D->activity.load();
                if (ui::RowToggle("Background Download", &act, {icon::Download, Color::Hex(0x30D158)}))
                {
                    D->activity = act;
                    if (!act)
                        ctx.ClearActivity("download");
                }
                ui::EndSection();
            }

            if (ui::BeginSection("About"))
            {
                ui::RowValue("Version", WGT_VERSION_STRING, {icon::Info, Color::Hex(0x8E8E93)});
                ui::RowValue("Renderer", D->opt.backendName, {icon::Game, Color::Hex(0xFF375F)});
                const ScaleInfo si = ctx.GetScaleInfo();
                char scaleBuf[96];
                std::snprintf(scaleBuf, sizeof(scaleBuf), "%.0f%% DPI  ·  %.2fx render", si.dpi * 100.0f, si.render);
                ui::RowValue("Display", scaleBuf, {icon::FullScreen, Color::Hex(0x5E5CE6)});
                const Context::TextRenderingInfo ti = ctx.GetTextRenderingInfo();
                char textBuf[96];
                std::snprintf(textBuf, sizeof(textBuf), "%s  ·  γ %.1f  ·  contrast %.2f / %.2f", ti.subpixel ? (ti.bgr ? "ClearType BGR" : "ClearType RGB") : "Grayscale",
                              ti.gamma, ti.grayscaleContrast, ti.clearTypeContrast);
                ui::RowValue("Text", textBuf, {icon::Edit, Color::Hex(0x30D158)});
                if (ui::RowNavigation("Performance", nullptr, {icon::Diagnostic, Color::Hex(0x30D158)}))
                    ui::NavigationPush("perf");
                ui::EndSection();
            }
            // floating Liquid Glass search field, the list scrolls under it (iOS Settings)
            ui::SearchBarOptions sbo;   // clear glass with a soft base color inside (base = false: nothing but the glass)
            sbo.base = D->searchBase;
            ui::SearchBar("settings_search", D->settingsSearch, sizeof(D->settingsSearch), sbo);
            ui::EndPage();
        }

        if (ui::BeginPage("accent", "Accent Color"))
        {
            if (ui::BeginSection("Choose a tint", "The accent animates across every control."))
            {
                Rect content;
                if (ui::BeginRow("swatches", 64, &content))
                {
                    ImGui::SetCursorScreenPos(Vec2(content.min.x, content.Center().y - S(17)));
                    if (ui::ColorSwatches("acc", &D->accent, kAccents, 7, 30))
                        ctx.SetAccent(kAccents[D->accent]);
                }
                ui::EndRow();
                ui::EndSection();
            }
            if (ui::BeginSection("Preview"))
            {
                bool dummy = true;
                ui::RowToggle("Switch", &dummy);
                static float v = 0.6f;
                ui::RowSlider("Slider", &v, 0, 1, {}, nullptr);
                ui::RowButton("Button");
                ui::EndSection();
            }
            ui::EndPage();
        }

        if (ui::BeginPage("perf", "Performance"))
        {
            const FrameStats s = ctx.GetStats();
            char buf[64];
            if (ui::BeginSection("Frame"))
            {
                std::snprintf(buf, sizeof(buf), "%.0f fps", s.fps);
                ui::RowValue("Frame rate", buf, {icon::Lightning, Color::Hex(0xFF9F0A)});
                std::snprintf(buf, sizeof(buf), "%.2f ms", s.cpuUiMs);
                ui::RowValue("UI build (CPU)", buf, {icon::Code, Color::Hex(0x0A84FF)});
                std::snprintf(buf, sizeof(buf), "%d", s.drawCalls);
                ui::RowValue("Draw calls", buf, {icon::Apps, Color::Hex(0x5E5CE6)});
                std::snprintf(buf, sizeof(buf), "%d", s.fxInstances);
                ui::RowValue("SDF instances", buf, {icon::Brush, Color::Hex(0xFF375F)});
                std::snprintf(buf, sizeof(buf), "%d", s.backdropCaptures);
                ui::RowValue("Glass captures", buf, {icon::View, Color::Hex(0x64D2FF)});
                std::snprintf(buf, sizeof(buf), "%d", s.vertices);
                ui::RowValue("Vertices", buf, {icon::Diagnostic, Color::Hex(0x30D158)});
                ui::EndSection();
            }
            if (ui::BeginSection("History"))
            {
                Rect content;
                if (ui::BeginRow("graph", 90, &content))
                {
                    ui::LineChartOptions co;
                    co.rect = content.Shrunk(S(8));
                    co.offset = D->fpsCursor;   // oldest sample of the ring buffer
                    co.glow = D->chartGlow;     // glow is opt-in
                    ui::LineChart("fps", D->fpsHistory, 120, co);
                }
                ui::EndRow();
                ui::RowToggle("Glow", &D->chartGlow, {icon::Lightbulb, Color::Hex(0xFFD60A)});
                ui::EndSection();
            }
            ui::EndPage();
        }
        ui::EndNavigation();
    }

    void PanelComponents(Context& ctx)
    {
        (void)ctx;
        const Theme& t = CurrentTheme();
        static const ui::TabItem tabs[] = {{"Controls", icon::Apps}, {"Inputs", icon::Edit}, {"Status", icon::Diagnostic}};

        // any component can be made transparent: here a scope over the whole tab (ui::SetNextItemLook does
        // the same for a single component, see the "Clear" button)
        ui::Checkbox("Transparent components", &D->clearComponents);
        ui::Spacer(6);
        if (D->clearComponents)
            ui::PushGlassLook(GlassLook::Clear);

        if (D->tab == 0)
        {
            ui::Text(TextStyle::Title3, "Buttons");
            ui::Spacer(4);
            // A flow wraps the buttons with the panel width. The glowing button automatically gets room
            // for its halo, and its glow fades out before it reaches a neighbour.
            if (ui::BeginFlow("buttons"))
            {
                ui::ButtonOptions b;
                ui::Button("Filled", b);
                b.kind = ui::ButtonKind::Tinted;
                ui::Button("Tinted", b);
                b.kind = ui::ButtonKind::Gray;
                ui::Button("Gray", b);
                b.kind = ui::ButtonKind::Plain;
                ui::Button("Plain", b);

                b = {};
                b.kind = ui::ButtonKind::Glass;
                b.icon = icon::Play;
                ui::Button("Glass", b);
                b.kind = ui::ButtonKind::GlassProminent;
                b.icon = icon::StarFill;
                ui::Button("Prominent", b);
                b = {};
                b.kind = ui::ButtonKind::Destructive;
                b.icon = icon::Delete;
                ui::Button("Delete", b);
                ui::Next().Look(GlassLook::Clear);   // just this one: a Filled button made transparent
                ui::Button("Clear", {.icon = icon::View});

                b = {};
                b.kind = ui::ButtonKind::Glass;
                ui::IconButton("i1", icon::Share, b);
                ui::IconButton("i2", icon::HeartFill, b);
                b.kind = ui::ButtonKind::Filled;
                ui::IconButton("i3", icon::Add, b);
                b.kind = ui::ButtonKind::Tinted;
                ui::IconButton("i4", icon::More, b);
                b = {};
                b.glow = true;
                b.tint = Color::Hex(0xFF375F);
                ui::Button("Neon Glow", b);

                b = {};
                b.size = ui::ControlSize::Large;
                b.width = -1;   // fills its line
                b.icon = icon::Game;
                if (ui::Button("Launch Game", b))
                {
                    Notification n;
                    n.title = "Launching";
                    n.message = "Starting the game client...";
                    n.icon = icon::Game;
                    D->ctx->Notify(n);
                }
                ui::Tooltip("Buttons, tooltips and notifications are all WGT widgets");
                ui::EndFlow();
            }

            // Per-component style: one line before any component changes just that component (no theme edits,
            // no Push/Pop pairs); only what is set changes
            ui::Spacer(8);
            ui::Text(TextStyle::Title3, "Per-component style");
            ui::Spacer(4);
            if (ui::BeginFlow("styled"))
            {
                ui::Next().Tint(Color::Hex(0x30D158)).Radius(8);
                ui::Button("Tint + radius");
                ui::Next().Look(GlassLook::Frosted).Blur(24).Fill(Color::Hex(0xBF5AF2));
                ui::Button("Frosted", {.icon = icon::StarFill});
                ui::Next().Look(GlassLook::Clear).Refraction(20).Dispersion(0.9f).Magnify(0.15f);
                ui::Button("Lens");
                ui::Next().Opacity(0.45f);
                ui::Button("45%");
                ui::Next().Tint(Color::Hex(0xFF9F0A));
                ui::Toggle("styledToggle", &D->styledToggle);
                ui::EndFlow();
            }
            ui::Next().Tint(Color::Hex(0xFF375F)).Radius(3);
            ui::Slider("styledSlider", &D->styledSlider, 0.0f, 1.0f);
            static const char* kStyledSeg[] = {"Day", "Week", "Month"};
            // track, selection after a switch, the lens while it moves, and the texts all have their own color
            ui::Next().Look(GlassLook::Frosted).Fill(Color::Hex(0x0A84FF)).Radius(10)
                .SelectedFill(Color::White(0.92f)).SelectedLabel(Color::Hex(0x0A64D8)).MovingFill(Color::White(0.25f)).Label(Color::White());
            ui::Segmented("styledSegment", &D->styledSegment, kStyledSeg, 3);

            ui::Spacer(8);
            ui::Text(TextStyle::Title3, "Switches");
            ui::Spacer(2);
            ui::FlowOptions sw;
            sw.spacing = 18;
            if (ui::BeginFlow("switches", sw))
            {
                ui::Toggle("tA", &D->toggleA);
                ui::Toggle("tB", &D->toggleB);
                ui::Checkbox("Subtitles", &D->checkA);
                ui::Checkbox("HDR", &D->checkB);
                ui::EndFlow();
            }

            ui::Spacer(8);
            ui::Text(TextStyle::Title3, "Sliders");
            ui::SliderOptions so;
            so.minIcon = icon::Mute;
            so.maxIcon = icon::Volume;
            ui::Slider("s1", &D->slider, 0.0f, 1.0f, so);
            so = {};
            so.tint = Color::Hex(0xFF9F0A);
            so.step = 8.0f;
            ui::Slider("s2", &D->sliderB, 0.0f, 128.0f, so);

            ui::Spacer(8);
            ui::Text(TextStyle::Title3, "Segmented & Stepper");
            static const char* segs[] = {"Low", "Medium", "High", "Ultra"};
            ui::Segmented("seg", &D->segment, segs, 4);
            ui::Spacer(4);
            ui::Stepper("step", &D->stepper, 0, 10);
            ImGui::SameLine(0, S(12));
            ui::Text(TextStyle::Body, "Squad size: %d", D->stepper);
        }
        else if (D->tab == 1)
        {
            ui::Text(TextStyle::Title3, "Text");
            ui::Spacer(2);
            ui::TextField("name", D->name, sizeof(D->name), "Player name");
            ui::Spacer(4);
            ui::SearchField("search", D->search, sizeof(D->search), "Search servers");
            ui::Spacer(4);
            ui::TextFieldOptions pw;
            pw.password = true;
            pw.icon = icon::Lock;
            ui::TextField("pw", D->password, sizeof(D->password), "Password", pw);
            ui::Spacer(8);
            ui::Text(TextStyle::Title3, "Menus");
            static const char* regions[] = {"Europe West", "North America", "Asia Pacific", "Oceania"};
            ui::Picker("region", &D->picker, regions, 4);
            ui::Spacer(8);
            ui::Text(TextStyle::Title3, "Swatches");
            ui::ColorSwatches("sw", &D->swatch, kAccents, 7);
            ui::Spacer(8);
            ui::Text(TextStyle::Title3, "Stock ImGui (auto-themed)");
            static float f = 0.5f;
            static int combo = 0;
            ImGui::SliderFloat("ImGui slider", &f, 0.0f, 1.0f);
            ImGui::Combo("ImGui combo", &combo, "One\0Two\0Three\0");
            ImGui::Button("ImGui::Button");
        }
        else
        {
            ui::Text(TextStyle::Title3, "Progress");
            ui::Spacer(4);
            ui::ProgressBar(D->download.Get());
            ui::Spacer(8);
            ui::ProgressRing(D->cpu.Get(), 54, 0, Color::Hex(0x30D158));
            ImGui::SameLine(0, S(14));
            ui::ProgressRing(D->gpu.Get(), 54, 0, Color::Hex(0xFF9F0A));
            ImGui::SameLine(0, S(14));
            ui::ProgressRing(D->download.Get(), 54, 0);
            ImGui::SameLine(0, S(18));
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + S(15));
            ui::ActivityIndicator();
            ui::Spacer(8);
            ui::Text(TextStyle::Title3, "Charts");
            ui::Spacer(4);
            static float series[48];
            static bool seeded = false;
            if (!seeded)
            {
                for (int i = 0; i < 48; ++i)
                    series[i] = 60.0f + 18.0f * std::sin(i * 0.23f) + 9.0f * std::sin(i * 0.71f + 1.0f) + (i > 30 ? 14.0f : 0.0f);
                seeded = true;
            }
            ui::LineChart("smooth", series, 48, {.height = 70.0f});   // smooth, soft fill, no glow
            ui::Spacer(6);
            ui::LineChart("neon", series, 48, {.height = 70.0f, .tint = Color::Hex(0xFF375F), .smooth = false, .fill = false, .glow = true});
            ui::Spacer(8);
            ui::Text(TextStyle::Title3, "Badges");
            ui::Badge("3");
            ImGui::SameLine();
            ui::Badge("NEW", Color::Hex(0x0A84FF));
            ImGui::SameLine();
            ui::Badge("LIVE", Color::Hex(0x30D158));
            ImGui::SameLine();
            ui::Badge("BETA", Color::Hex(0xBF5AF2));
            ui::Spacer(8);
            ui::Text(TextStyle::Title3, "Skeleton loading");
            Painter p;
            const Vec2 pos = ImGui::GetCursorScreenPos();
            const float w = ImGui::GetContentRegionAvail().x;
            const Style sk = Style().Radius(S(8)).Fill(t.colors.fill).Shimmer(0.35f, 0.6f);
            p.Circle(pos + Vec2(S(22), S(22)), S(22), Style().Fill(t.colors.fill).Shimmer(0.35f, 0.6f));
            p.Rect(Rect::FromSize(pos + Vec2(S(56), S(4)), Vec2(w * 0.55f, S(14))), sk);
            p.Rect(Rect::FromSize(pos + Vec2(S(56), S(26)), Vec2(w * 0.35f, S(12))), sk);
            p.Rect(Rect::FromSize(pos + Vec2(0, S(56)), Vec2(w, S(64))), Style(sk).Radius(S(14)));
            ImGui::Dummy(Vec2(w, S(128)));
        }
        ui::TabBar("tabs", &D->tab, tabs, 3);
        if (D->clearComponents)
            ui::PopGlassLook();
    }

    void PanelEffects(Context& ctx)
    {
        (void)ctx;
        const Theme& t = CurrentTheme();
        const float time = (float)anim::Time();
        const float w = ImGui::GetContentRegionAvail().x;
        Painter p;

        // ---- glass stage
        ui::Text(TextStyle::Title3, "Liquid Glass");
        ui::TextColored(TextStyle::Footnote, t.colors.secondaryLabel, "Backdrop blur, lensing, dispersion and specular rims over live content");
        ui::Spacer(4);
        Vec2 pos = ImGui::GetCursorScreenPos();
        const float stageH = S(190);
        const Rect stage = Rect::FromSize(pos, Vec2(w, stageH));
        p.Rect(stage, Style().Radius(S(18)).Fill(Paint::Linear(Color::Hex(0x5E2BFF), Color::Hex(0xFF6A3D), 20)));
        p.PushMask(stage, S(18));
        for (int i = 0; i < 9; ++i)   // stripes: sharp detail to see refraction
            p.Rect(Rect::FromSize(Vec2(stage.min.x + S(14) + i * (w / 9.0f), stage.min.y), Vec2(S(5), stageH)), Style().Fill(Color::White(0.55f)));
        for (int i = 0; i < 5; ++i)
        {
            const float a = time * 0.6f + i * 1.3f;
            const Vec2 c(stage.min.x + w * (0.12f + 0.19f * i) + std::sin(a) * S(18), stage.Center().y + std::cos(a * 1.3f) * S(46));
            const Color cols[] = {Color::Hex(0x30D158), Color::Hex(0xFFD60A), Color::Hex(0x64D2FF), Color::Hex(0xFF375F), Color::Hex(0xFFFFFF)};
            p.Circle(c, S(22), Style().Fill(cols[i]));
        }
        p.PopMask();
        p.Text(stage.min + Vec2(S(16), S(12)), GetFont(FontWeight::Bold, 30), Color::White(), "Waffle");

        struct Sample { const char* label; GlassMaterial m; Color fill; };
        GlassMaterial clear = t.materials.clear;
        GlassMaterial regular = t.materials.control;
        regular.blur = 14.0f;
        GlassMaterial thick = t.materials.popover;
        GlassMaterial tinted = t.materials.control;
        tinted.tint = Color::Hex(0x0A84FF, 0.35f);
        tinted.blur = 8.0f;
        const Sample samples[] = {{"Clear", clear, Color::Clear()}, {"Regular", regular, Color::Clear()}, {"Thick", thick, Color::Clear()}, {"Tinted", tinted, Color::Clear()}};
        const float cellW = (w - S(24)) / 4.0f;
        for (int i = 0; i < 4; ++i)
        {
            const float bob = std::sin(time * 1.1f + i * 0.9f) * S(6);
            const Rect r = Rect::FromCenter(Vec2(stage.min.x + S(12) + cellW * (i + 0.5f), stage.Center().y + S(22) + bob), Vec2(cellW - S(14), S(82)));
            p.Rect(r, Style().Radius(S(24)).Glass(samples[i].m).Shadow(Color::Black(0.25f), S(18), Vec2(0, S(8))));
            p.TextAligned(r.Bottom(S(26)), Vec2(0.5f, 0.5f), TextStyle::Footnote, Color::White(0.95f), samples[i].label);
        }
        ImGui::Dummy(Vec2(w, stageH));

        // ---- liquid merge + custom effect
        ui::Spacer(10);
        ui::Text(TextStyle::Title3, "Liquid Morph & Custom Shader");
        ui::Spacer(4);
        pos = ImGui::GetCursorScreenPos();
        const float rowH = S(120);
        const Rect left = Rect::FromSize(pos, Vec2(w * 0.5f - S(6), rowH));
        const Rect right = Rect::FromSize(Vec2(pos.x + w * 0.5f + S(6), pos.y), Vec2(w * 0.5f - S(6), rowH));
        p.Rect(left, Style().Radius(S(18)).Fill(t.colors.cardSurface));
        const float sep = (std::sin(time * 1.2f) * 0.5f + 0.5f);
        const Vec2 c1(left.Center().x - S(34) - sep * S(26), left.Center().y), c2(left.Center().x + S(34) + sep * S(26), left.Center().y);
        GlassMaterial blob = t.materials.clear;
        blob.tint = Color::Hex(0x64D2FF, 0.25f);
        p.Merge(Rect::FromCenter(c1, Vec2(S(64), S(64))), Rect::FromCenter(c2, Vec2(S(52), S(52))), S(32), S(34),
                Style().Glass(blob).Fill(Paint::Radial(Color::Hex(0x64D2FF, 0.35f), Color::Hex(0x5E5CE6, 0.55f))).Glow(Color::Hex(0x64D2FF), S(14), 0.5f));
        if (D->aurora)
            p.Rect(right, Style().Radius(S(18)).Effect(D->aurora, 1.0f).Shadow(Color::Black(0.3f), S(16), Vec2(0, S(6))));
        else
            p.Rect(right, Style().Radius(S(18)).Fill(t.colors.cardSurface));
        p.TextAligned(right, Vec2(0.5f, 0.5f), TextStyle::Headline, Color::White(), "HLSL effect");
        ImGui::Dummy(Vec2(w, rowH));

        // ---- glow layer (bloom on arbitrary content)
        ui::Spacer(10);
        ui::Text(TextStyle::Title3, "Bloom Layers & Glow");
        ui::Spacer(4);
        pos = ImGui::GetCursorScreenPos();
        const Rect neon = Rect::FromSize(pos, Vec2(w, S(96)));
        p.Rect(neon, Style().Radius(S(18)).Fill(Color::Hex(0x06060A, 0.92f)));
        p.BeginGlowLayer(Color::Clear(), S(16), 1.35f, 1.0f);
        const FontRef big = GetFont(FontWeight::Bold, 40);
        const Vec2 ts = Painter::MeasureText(big, "NEON");
        const float pulse = 0.88f + 0.12f * std::sin(time * 3.0f);
        // a neon tube: near-white core, the cyan comes from the bloom around it
        p.Text(Vec2(neon.min.x + S(24), neon.Center().y - ts.y * 0.5f), big, Color::Hex(0xB4F2FF).Fade(pulse), "NEON");
        p.Ring(Vec2(neon.max.x - S(56), neon.Center().y), S(26), S(5), Style().Fill(Paint::ConicLoop(Color::Hex(0xFF375F), Color::Hex(0xBF5AF2), Degrees(time * 2.0f))));
        p.Line(Vec2(neon.min.x + S(150), neon.Center().y + S(18)), Vec2(neon.max.x - S(110), neon.Center().y - S(18)), S(4), Style().Fill(Color::Hex(0x30D158)));
        p.EndGlowLayer();
        ImGui::Dummy(Vec2(w, S(96)));

        // ---- shapes
        ui::Spacer(10);
        ui::Text(TextStyle::Title3, "Shapes, Gradients & Shadows");
        ui::Spacer(4);
        pos = ImGui::GetCursorScreenPos();
        const float s = (w - S(36)) / 4.0f;
        const Style base = Style().Radius(S(20));
        p.Rect(Rect::FromSize(pos, Vec2(s, s)), Style(base).Fill(Paint::Linear(Color::Hex(0xFF9F0A), Color::Hex(0xFF375F), 45)).Shadow(Color::Hex(0xFF375F, 0.45f), S(18), Vec2(0, S(8))));
        p.Rect(Rect::FromSize(pos + Vec2(s + S(12), 0), Vec2(s, s)), Style(base).Fill(Paint::Radial(Color::Hex(0x64D2FF), Color::Hex(0x0A2A6A), Vec2(0.3f, 0.25f), 0.9f)));
        p.Rect(Rect::FromSize(pos + Vec2((s + S(12)) * 2, 0), Vec2(s, s)), Style(base).Fill(Paint::ConicLoop(Color::Hex(0x30D158), Color::Hex(0xBF5AF2), Degrees(time))));
        p.Rect(Rect::FromSize(pos + Vec2((s + S(12)) * 3, 0), Vec2(s, s)), Style(base).Fill(t.colors.cardSurface).InnerShadow(Color::Black(0.35f), S(14), Vec2(0, S(4))));
        ImGui::Dummy(Vec2(w, s));
        ui::Spacer(6);
        pos = ImGui::GetCursorScreenPos();
        for (int i = 0; i < 4; ++i)
        {
            const Vec2 c(pos.x + s * 0.5f + i * (s + S(12)), pos.y + S(34));
            const float prog = std::fmod(time * 0.15f + i * 0.23f, 1.0f);
            const Color col = kAccents[i + 1];
            p.Ring(c, S(26), S(7), Style().Fill(col.Fade(0.18f)));
            p.Arc(c, S(26), S(7), -kPi * 0.5f, kTau * prog, Style().Fill(col).Glow(col, S(6), 0.5f));
            char buf[8];
            std::snprintf(buf, sizeof(buf), "%d", (int)(prog * 100));
            p.TextAligned(Rect::FromCenter(c, Vec2(S(40), S(20))), Vec2(0.5f, 0.5f), TextStyle::Footnote, t.colors.label, buf);
        }
        ImGui::Dummy(Vec2(w, S(70)));
    }

    void PanelControlCenter(Context& ctx)
    {
        (void)ctx;
        const Theme& t = CurrentTheme();
        // Modules flow through an adaptive grid: 2 columns at the default size, all 4 in one row when the
        // panel is wide, a single column when it is narrow. Every module sizes itself from its cell.
        ui::GridOptions grid;
        grid.minColumnWidth = 150;
        grid.maxColumns = 4;
        grid.spacing = 12;
        if (!ui::BeginGrid("cc", grid))
            return;

        // connectivity: 2 x 2 round toggles
        {
            const Rect r = BeginModule();
            const float w = r.Width(), d = w * 0.34f;
            const float a = w * 0.29f, b = w * 0.71f;
            ModuleToggle("air", &D->airplane, icon::Airplane, Color::Hex(0xFF9F0A), r.min + Vec2(a, a), d);
            ModuleToggle("cell", &D->cellular, icon::Phone, Color::Hex(0x30D158), r.min + Vec2(b, a), d);
            ModuleToggle("wifi", &D->wifi, icon::Wifi, Color::Hex(0x0A84FF), r.min + Vec2(a, b), d);
            ModuleToggle("bt", &D->bluetooth, icon::Bluetooth, Color::Hex(0x0A84FF), r.min + Vec2(b, b), d);
            EndModule(r);
        }

        // now playing
        {
            const Rect r = BeginModule();
            const float w = r.Width(), pad = w * 0.094f;
            Painter p;
            const Rect art = Rect::FromSize(r.min + Vec2(pad, pad), Vec2(w * 0.33f, w * 0.33f));
            if (D->opt.wallpaper)
                p.Image(D->opt.wallpaper, art, S(12), Color::White(), Vec2(0.35f, 0.3f), Vec2(0.6f, 0.7f));
            else
                p.Rect(art, Style().Radius(S(12)).Fill(Paint::Linear(Color::Hex(0xFF375F), Color::Hex(0x5E5CE6), 45)));
            p.Text(Vec2(r.min.x + pad, art.max.y + w * 0.06f), TextStyle::Headline, t.colors.label, "Glass Hearts");
            p.Text(Vec2(r.min.x + pad, art.max.y + w * 0.06f + S(20)), TextStyle::Footnote, t.colors.secondaryLabel, "The Wafflers");
            ImGui::SetCursorScreenPos(Vec2(r.min.x + pad * 1.2f, r.max.y - pad - S(28)));
            ui::ButtonOptions bo;
            bo.kind = ui::ButtonKind::Plain;
            bo.size = ui::ControlSize::Small;
            ui::IconButton("prev", icon::Back, bo);
            ImGui::SameLine(0, S(12));
            if (ui::IconButton("play", D->playing ? icon::Pause : icon::Play, bo))
                D->playing = !D->playing;
            ImGui::SameLine(0, S(12));
            ui::IconButton("next", icon::Forward, bo);
            EndModule(r);
        }

        // brightness + volume: a stack inside one cell, each slider takes half of it
        {
            const float h = ui::AvailableWidth();
            ui::StackOptions row;
            row.spacing = 12;
            if (ui::BeginHStack("sliders", row))
            {
                TallSlider("bright", &D->brightness, icon::Brightness, h);
                TallSlider("vol", &D->volume, icon::Volume, h);
                ui::EndStack();
            }
        }

        // focus + two small toggles
        {
            const float cw = ui::AvailableWidth();
            const float half = (cw - S(12)) * 0.5f;
            ui::StackOptions col;
            col.spacing = 12;
            col.align = ui::Align::Start;
            if (ui::BeginVStack("small", col))
            {
                {
                    const Rect r = BeginModule(half);
                    const float d = half * 0.70f;
                    ModuleToggle("focus_t", &D->focus, icon::Moon, Color::Hex(0x5E5CE6), Vec2(r.min.x + half * 0.5f, r.Center().y), d);
                    Painter p;
                    const float tx = r.min.x + half * 0.5f + d * 0.5f + S(12);
                    p.Text(Vec2(tx, r.Center().y - S(19)), TextStyle::Headline, t.colors.label, "Focus");
                    p.Text(Vec2(tx, r.Center().y + S(1)), TextStyle::Footnote, t.colors.secondaryLabel, D->focus ? "Gaming" : "Off");
                    EndModule(r);
                }
                ui::StackOptions pair;
                pair.spacing = 12;
                if (ui::BeginHStack("pair", pair))
                {
                    static bool mirror = false;
                    {
                        const Rect r = BeginModule(half);
                        ModuleToggle("rot_t", &D->rotation, icon::Lock, Color::Hex(0xFF375F), r.Center(), half * 0.70f);
                        EndModule(r);
                    }
                    {
                        const Rect r = BeginModule(half);
                        ModuleToggle("mir_t", &mirror, icon::Connect, Color::Hex(0x64D2FF), r.Center(), half * 0.70f);
                        EndModule(r);
                    }
                    ui::EndStack();
                }
                ui::EndStack();
            }
        }
        ui::EndGrid();
    }

    void PanelLanguages(Context& ctx)
    {
        (void)ctx;
        const Theme& t = CurrentTheme();
        // Nothing is configured for any of these: shaping, bidi, line breaking and font fallback come from
        // the WGT text engine (DirectWrite), emoji are color-font layers. No glyph ranges, no font merging.
        static const char* kSamples[][2] = {
            {"English", "The quick brown fox jumps over the lazy dog"},
            {"简体中文", "液态玻璃界面，原生支持多语言显示与输入"},
            {"繁體中文", "液態玻璃介面，原生支援多語言顯示與輸入"},
            {"日本語", "リキッドグラスの UI、ネイティブな多言語対応"},
            {"한국어", "리퀴드 글래스 UI, 기본 다국어 지원"},
            {"العربية", "واجهة زجاجية سائلة بدعم أصيل لكل اللغات"},
            {"עברית", "ממשק זכוכית נוזלית עם תמיכה מלאה בשפות"},
            {"हिन्दी", "तरल ग्लास इंटरफ़ेस, मूल बहुभाषी समर्थन"},
            {"ไทย", "อินเทอร์เฟซกระจกเหลว รองรับหลายภาษาในตัว"},
            {"Русский", "Интерфейс из жидкого стекла, все языки сразу"},
            {"Ελληνικά", "Διεπαφή υγρού γυαλιού για κάθε γλώσσα"},
            {"Tiếng Việt", "Giao diện kính lỏng, hỗ trợ đa ngôn ngữ"},
            {"Emoji", "🎮 🚀 ✨ 🧇 👍🏽 👨‍👩‍👧 🇨🇳 🇯🇵 🇰🇷"},
        };
        ui::TextColored(TextStyle::Footnote, t.colors.secondaryLabel, "No fonts or glyph ranges were loaded for any of these.");
        ui::Spacer(6);
        ui::StackOptions col;
        col.spacing = 10;
        col.align = ui::Align::Start;
        if (ui::BeginVStack("samples", col))
        {
            for (const auto& sample : kSamples)
            {
                ImGui::BeginGroup();
                ui::TextColored(TextStyle::Caption1, t.colors.secondaryLabel, "%s", sample[0]);
                ui::TextWrapped(TextStyle::Body, t.colors.label, sample[1]);
                ImGui::EndGroup();
            }
            ui::EndStack();
        }
        ui::Spacer(10);
        ui::Text(TextStyle::Headline, "Type in any language");
        ui::Spacer(4);
        ui::TextFieldOptions o;
        o.icon = icon::Globe;
        ui::TextField("lang_input", D->langInput, sizeof(D->langInput), "中文 / 日本語 / 한국어 / العربية ...", o);
        ui::Spacer(4);
        ui::TextColored(TextStyle::Footnote, t.colors.secondaryLabel, "IME composition is drawn inline; the caret moves by grapheme clusters.");
    }

    void PanelTelemetry(Context& ctx)
    {
        (void)ctx;
        const Theme& t = CurrentTheme();
        D->logIn.Drain([](std::string&& s) {
            D->log.push_back(std::move(s));
            if (D->log.size() > 200)
                D->log.erase(D->log.begin());
        });

        ui::TextColored(TextStyle::Footnote, t.colors.secondaryLabel, "Values below are written by a worker thread through wgt::Property / wgt::Channel.");
        ui::Spacer(6);
        const float cpu = D->cpu.Get(), gpu = D->gpu.Get();
        // one gauge = ring + caption, a single item of the flow below
        auto gauge = [](float value, Color tint, const char* title, const char* detail) {
            ImGui::BeginGroup();
            ui::ProgressRing(value, 64, 8, tint);
            ImGui::SameLine(0, S(16));
            ImGui::BeginGroup();
            ui::Text(TextStyle::Headline, "%s", title);
            ui::TextSecondary("%s", detail);
            ImGui::EndGroup();
            ImGui::EndGroup();
        };
        // side by side while they fit, the second one moves to its own line in a narrow window (never clipped)
        ui::FlowOptions gf;
        gf.spacing = 28;
        if (ui::BeginFlow("gauges", gf))
        {
            char title[32], detail[48];
            std::snprintf(title, sizeof(title), "CPU %.0f%%", cpu * 100.0f);
            gauge(cpu, Color::Hex(0x30D158), title, "Game thread load");
            std::snprintf(title, sizeof(title), "GPU %.0f%%", gpu * 100.0f);
            std::snprintf(detail, sizeof(detail), "%d players online", D->players.Get());
            gauge(gpu, Color::Hex(0xFF9F0A), title, detail);
            ui::EndFlow();
        }
        ui::Spacer(10);
        if (ui::BeginSection("Event log"))
        {
            const int n = (int)D->log.size();
            for (int i = std::max(0, n - 8); i < n; ++i)
            {
                ImGui::PushID(i);
                ui::RowValue(D->log[i].c_str(), "");
                ImGui::PopID();
            }
            if (n == 0)
                ui::RowValue("Waiting for events...", "");
            ui::EndSection();
        }
    }

    bool ShouldOpen(const char* id, bool def)
    {
        if (D->opt.openAll)
            return true;
        if (!D->opt.openList)
            return def;
        const std::string list = std::string(",") + D->opt.openList + ",";
        return list.find(std::string(",") + id + ",") != std::string::npos;
    }
}

ShowcaseDisplay& ShowcaseDisplaySettings()
{
    static ShowcaseDisplay display;
    return display;
}

void ShowcaseInit(Context* ctx, const ShowcaseOptions& options)
{
    D = new Demo();
    D->ctx = ctx;
    D->opt = options;
    D->glassLook = (int)ctx->GetGlassLook();
    D->darkMode = ctx->IsDarkMode();
    D->aurora = ctx->RegisterEffect("aurora", kAuroraHlsl);

    PanelDesc d;
    d.id = "settings";
    d.title = "Settings";
    d.icon = icon::Settings;
    d.iconColor = Color::Hex(0x8E8E93);
    d.size = Vec2(390, 700);
    d.pos = Vec2(40, 64);
    d.flags = PanelFlags_NoScroll;
    d.open = ShouldOpen("settings", true);
    ctx->AddPanel(d, [](Context& c) { PanelSettings(c); });

    d = {};
    d.id = "effects";
    d.title = "Effects Lab";
    d.icon = icon::Brush;
    d.iconColor = Color::Hex(0xFF375F);
    d.size = Vec2(470, 660);
    d.pos = Vec2(452, 64);
    d.open = ShouldOpen("effects", true);
    ctx->AddPanel(d, [](Context& c) { PanelEffects(c); });

    d = {};
    d.id = "control";
    d.title = "Control Center";
    d.icon = icon::Equalizer;
    d.iconColor = Color::Hex(0xFF9F0A);
    d.size = Vec2(392, 440);
    d.pos = Vec2(944, 64);
    d.open = ShouldOpen("control", true);
    ctx->AddPanel(d, [](Context& c) { PanelControlCenter(c); });

    d = {};
    d.id = "components";
    d.title = "Components";
    d.icon = icon::Apps;
    d.iconColor = Color::Hex(0x0A84FF);
    d.size = Vec2(430, 800);
    d.pos = Vec2(452, 64);
    d.open = ShouldOpen("components", false);
    ctx->AddPanel(d, [](Context& c) { PanelComponents(c); });

    d = {};
    d.id = "languages";
    d.title = "Languages";
    d.icon = icon::Globe;
    d.iconColor = Color::Hex(0x64D2FF);
    d.size = Vec2(430, 760);
    d.pos = Vec2(944, 64);
    d.open = ShouldOpen("languages", false);
    ctx->AddPanel(d, [](Context& c) { PanelLanguages(c); });

    d = {};
    d.id = "telemetry";
    d.title = "Telemetry";
    d.icon = icon::Diagnostic;
    d.iconColor = Color::Hex(0x30D158);
    d.size = Vec2(430, 460);
    d.pos = Vec2(470, 120);
    d.open = ShouldOpen("telemetry", false);
    ctx->AddPanel(d, [](Context& c) { PanelTelemetry(c); });

    D->running = true;
    D->worker = std::thread(WorkerMain);
}

void ShowcaseBackground()
{
    ImDrawList* bg = ImGui::GetBackgroundDrawList();
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const Rect screen(vp->Pos, vp->Pos + vp->Size);
    const float time = (float)anim::Time();
    Painter p(bg);
    if (D->opt.wallpaper && D->opt.wallpaperWidth > 0)
    {
        // cover-fit
        const float sa = screen.Width() / screen.Height();
        const float ia = (float)D->opt.wallpaperWidth / (float)D->opt.wallpaperHeight;
        Vec2 uv0(0, 0), uv1(1, 1);
        if (ia > sa)
        {
            const float k = sa / ia;
            uv0.x = (1 - k) * 0.5f;
            uv1.x = 1 - uv0.x;
        }
        else
        {
            const float k = ia / sa;
            uv0.y = (1 - k) * 0.5f;
            uv1.y = 1 - uv0.y;
        }
        bg->AddImage(D->opt.wallpaper, screen.min, screen.max, uv0, uv1);
    }
    else
        p.Rect(screen, Style().Fill(Paint::Linear(Color::Hex(0x1B1464), Color::Hex(0xC2185B), 35)));

    // drifting light blobs: gives the glass something alive to refract
    const Color blobs[] = {Color::Hex(0xFF375F, 0.55f), Color::Hex(0x64D2FF, 0.50f), Color::Hex(0xFFD60A, 0.45f), Color::Hex(0xBF5AF2, 0.50f)};
    for (int i = 0; i < 4; ++i)
    {
        const float a = time * (0.13f + 0.04f * i) + i * 1.7f;
        const Vec2 c(screen.min.x + screen.Width() * (0.5f + 0.38f * std::sin(a)), screen.min.y + screen.Height() * (0.5f + 0.34f * std::cos(a * 1.31f)));
        const float r = screen.Height() * (0.12f + 0.03f * i);
        p.Circle(c, r, Style().Fill(Paint::Radial(blobs[i], blobs[i].Fade(0.0f), Vec2(0.5f, 0.5f), 0.5f)));
    }
}

void ShowcaseFrame()
{
    // HUD pill (top-left): regular immediate-mode drawing outside of any panel
    const FrameStats s = D->ctx->GetStats();
    D->fpsHistory[D->fpsCursor] = s.fps;
    D->fpsCursor = (D->fpsCursor + 1) % 120;
    if (!D->ctx->IsVisible())
        return;
    const Theme& t = CurrentTheme();
    Painter p(ImGui::GetBackgroundDrawList());
    char buf[256];
    std::snprintf(buf, sizeof(buf), "%s  ·  %s  |  %.0f fps  |  UI: CPU %.2f ms  GPU %.2f ms  |  %d draws  |  %d shapes  |  %d glass passes", D->opt.backendName,
                  D->opt.adapterName, s.fps, s.cpuUiMs, s.gpuMs, s.drawCalls, s.fxInstances, s.backdropCaptures);
    const FontRef f = GetFont(TextStyle::Footnote);
    const Vec2 ts = Painter::MeasureText(f, buf);
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const Rect r = Rect::FromSize(vp->Pos + Vec2(S(14), vp->Size.y - S(44)), Vec2(ts.x + S(28), S(30)));
    Style hs = Style().Glass(ui::LookMaterial(t.materials.bar));
    if (ui::CurrentGlassLook() == GlassLook::Theme)
        hs.Fill(t.colors.windowSurface.Fade(0.5f));
    p.Capsule(r, hs);
    p.Text(Vec2(r.min.x + S(14), std::floor(r.Center().y - ts.y * 0.5f)), f, t.colors.label, buf);
}

void ShowcaseShutdown()
{
    if (!D)
        return;
    D->running = false;
    if (D->worker.joinable())
        D->worker.join();
    delete D;
    D = nullptr;
}
