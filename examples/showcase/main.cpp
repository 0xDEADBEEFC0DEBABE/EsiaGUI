// showcase - WGT's demo on Esia: its panels in liquid-glass windows over the wallpaper, the dock that opens them, the
// island a worker thread posts notifications and a download to, and the status bar. The panels are WGT's
// (examples/demo/showcase.cpp at tag wgt-1.1-final), widget for widget, so captures compare with WGT's reference
// screenshots (1600 x 1000 pixels at UI scale 1 and 1.5, the same clock; docs/UI_WIDGETS.md, section 11):
//
//   showcase.exe --api d3d11 --size 1600x1000 --scale 1 --open <panel> [--light] --fixed-dt 0.016667
//                --frames 120 --screenshot <panel>_<dark|light>_x1.png
//   showcase.exe ... --size 1066.6667x666.6667 --scale 1.5 ...       (x1.5: the same pixels, UI units x 1.5)
//
//   --open list     settings,effects,control,components,languages,telemetry,plugin (default: the first three),
//                   none, or --open-all
//   --open-later p@n  panel p opens at frame n, as a click on its dock tile would (what a first appearance costs)
//   --light         the light theme (the demo starts dark)
//   --look l        theme | clear | frosted (default)
//   --tab n         the Components panel's tab
//   --page id       the Settings page shown first (accent, perf)
//   --menu          the Components panel opens its menu (checks the popups)
// On a display smaller than the desktop layout (a phone) one panel shows at a time, as large as fits over the status bar
// (then centered over the dock); the dock switches panels as between apps.
// The app options (API, size, scale, frames, screenshot ...) are glass_window's (../glass_window/app.hpp).
#include "app.hpp"
#include "esia/render/painter.hpp"
#include "esia/render/renderer.hpp"
#include "esia/ui/ui.hpp"
#include "image_file.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <string>
#include <thread>
#include <vector>
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

namespace ui = esia::ui;
namespace icon = esia::ui::icons;
using esia::Color;
using esia::Paint;
using esia::Rect;
using esia::Style;
using esia::Vec2;

namespace
{
    constexpr esia::EffectId kAurora = 1, kHolo = 2;

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

    // holographic foil: iridescent bands that drift with time and refract the backdrop (WGT's plugin example)
    const char* kHoloHlsl = R"(
float4 WgtEffect(WgtFx fx)
{
    float t = fx.time * fx.params.y;
    float band = sin((fx.uv.x * 7.0 + fx.uv.y * 3.5) - t * 2.0) * 0.5 + 0.5;
    float3 foil = 0.55 + 0.45 * cos(6.28318 * (band + float3(0.00, 0.33, 0.67)));
    float3 behind = WgtBackdrop(fx.screenUV + (band - 0.5) * 0.01, 6.0);
    float3 col = lerp(behind, foil, 0.55);
    float a = fx.coverage * fx.params.x;
    return float4(col * a, a);
}
)";

    const Color kAccents[] = {Color::Hex(0x0A84FF), Color::Hex(0xBF5AF2), Color::Hex(0xFF375F), Color::Hex(0xFF9F0A),
                              Color::Hex(0x30D158), Color::Hex(0x64D2FF), Color::Hex(0xFFD60A)};

    enum Panel : int { Settings, Effects, Control, Components, Languages, Telemetry, Plugin, PanelCount };
    struct PanelInfo
    {
        const char* id;
        const char* title;
        esia::ui::Icon icon;
        std::uint32_t color;
        Vec2 size, pos;
        bool open;
    };
    const PanelInfo kPanels[PanelCount] = {
        {"settings", "Settings", icon::Settings, 0x8E8E93, {390, 700}, {40, 64}, true},
        {"effects", "Effects Lab", icon::Brush, 0xFF375F, {470, 660}, {452, 64}, true},
        {"control", "Control Center", icon::Equalizer, 0xFF9F0A, {392, 440}, {944, 64}, true},
        {"components", "Components", icon::Apps, 0x0A84FF, {430, 800}, {452, 64}, false},
        {"languages", "Languages", icon::Globe, 0x64D2FF, {430, 760}, {944, 64}, false},
        {"telemetry", "Telemetry", icon::Diagnostic, 0x30D158, {430, 460}, {470, 120}, false},
        {"plugin", "Plugin: Hello", icon::Code, 0x5E5CE6, {360, 380}, {-1, -1}, false},
    };
    // --spread: every panel open, side by side, none over another or under the island and the dock, in a window of
    // 1872 x 1400 UI units (the README's picture)
    const Vec2 kSpread[PanelCount] = {{40, 140}, {454, 140}, {40, 864}, {948, 140}, {1402, 140}, {474, 824}, {1437, 924}};

    float S(float v) { return ui::S(v); }
    Rect Bottom(const Rect& r, float h) { return Rect(r.min.x, r.max.y - h, r.max.x, r.max.y); }

    struct Demo
    {
        std::unique_ptr<ui::Ui> ui;
        esia::Context* ctx = nullptr;
        bool open[PanelCount] = {};
        int openLater = -1, openLaterFrame = 0, frameCount = 0;   // --open-later panel@frame

        // appearance
        bool darkMode = true;
        int accent = 0, textSize = 1, glassLook = 2;
        float frost = 10.0f, refraction = 14.0f, dispersion = 0.30f;
        bool glassEdited = false, specular = true, searchBase = true;

        // control center
        bool airplane = false, cellular = true, wifi = true, bluetooth = true, focus = false, rotation = true, mirror = false;
        float brightness = 0.72f, volume = 0.45f;
        bool playing = true;

        // components
        bool toggleA = true, toggleB = false, checkA = true, checkB = false;
        float slider = 0.42f, sliderB = 64.0f;
        int stepper = 3, segment = 1, picker = 1, tab = 0, swatch = 0;
        std::string name = "Waffle", search, settingsSearch, langInput, password = "secret";
        bool clearComponents = false, styledToggle = true;
        float styledSlider = 0.6f;
        int styledSegment = 1;
        bool openMenu = false;
        bool spread = false;   // --spread: panels at kSpread
        std::string openPage;

        // settings
        bool vsync = true;
        int frameLimit = 0, textAa = 1;
        float fpsHistory[120] = {};
        int fpsCursor = 0;
        bool chartGlow = false;

        // plugin
        int taps = 0;

        // the frame
        glass::SceneInfo info;
        esia::TextureId wallpaper = 0;
        int wallpaperWidth = 0, wallpaperHeight = 0;

        // telemetry: written by the worker thread
        std::atomic<float> cpu{0.3f}, gpu{0.5f}, download{0.0f};
        std::atomic<int> players{12};
        std::mutex logMutex;
        std::vector<std::string> logIn, log;
        std::atomic<bool> running{false}, activity{true};
        std::thread worker;

        // ------------------------------------------------------------ theme
        // the theme from the settings: dark or light, the text size, the Liquid Glass edits
        void ApplyTheme(bool animate)
        {
            static const float kScales[] = {0.87f, 1.0f, 1.18f};
            const ui::Theme base = darkMode ? ui::ThemeDark() : ui::ThemeLight();
            ui::Theme t = base;
            t.metrics.scale = kScales[std::clamp(textSize, 0, 2)];
            if (glassEdited)
            {
                t.materials.window.blur = frost;
                t.materials.window.refraction = refraction;
                t.materials.window.dispersion = dispersion;
                const float spec = specular ? 1.0f : 0.0f;
                t.materials.window.specular = base.materials.window.specular * spec;
                t.materials.bar.specular = base.materials.bar.specular * spec;
                t.materials.control.specular = base.materials.control.specular * spec;
            }
            ui->SetTheme(t, animate);
        }

        // ----------------------------------------------------------- worker
        void WorkerMain()
        {
            std::mt19937 rng(1234);
            std::uniform_real_distribution<float> n01(0.0f, 1.0f);
            const char* messages[][2] = {
                {"Match found", "Ranked \xE2\x80\xA2 Summoner's Rift is ready"},
                {"Achievement unlocked", "Liquid Glass Enthusiast"},
                {"Friend online", "Gamerdoc just came online"},
                {"Shaders compiled", "1,248 pipelines cached in 2.3 s"},
            };
            const ui::Icon msgIcons[] = {icon::Game, icon::StarFill, icon::People, icon::Lightning};
            const Color msgTints[] = {Color::Hex(0x30D158), Color::Hex(0xFFD60A), Color::Hex(0x0A84FF), Color::Hex(0xBF5AF2)};
            int tick = 0, msg = 0;
            float dl = 0.0f;
            const auto logLine = [&](std::string s) {
                std::lock_guard lock(logMutex);
                logIn.push_back(std::move(s));
            };
            while (running.load())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                ++tick;
                cpu = esia::Clamp(cpu.load() + (n01(rng) - 0.5f) * 0.06f, 0.08f, 0.95f);
                gpu = esia::Clamp(gpu.load() + (n01(rng) - 0.48f) * 0.05f, 0.15f, 0.98f);
                if (tick % 40 == 0)
                    players = std::max(1, players.load() + (int)(n01(rng) * 5.0f) - 2);
                if (activity.load())
                {
                    dl += 0.004f + n01(rng) * 0.004f;
                    if (dl >= 1.0f)
                    {
                        ui->ClearActivity("download");
                        ui::Notification n;
                        n.title = "Download complete";
                        n.message = "HD texture pack \xE2\x80\xA2 2.4 GB installed";
                        n.icon = icon::Download;
                        n.tint = Color::Hex(0x30D158);
                        ui->Notify(n);
                        logLine("[worker] download finished");
                        dl = -1.5f;   // a pause before the next download
                    }
                    else if (dl >= 0.0f)
                        ui->SetActivity("download", "Downloading texture pack", dl, icon::Download, Color::Hex(0x30D158));
                    download = std::max(dl, 0.0f);
                }
                if (tick % 180 == 90)
                {
                    ui::Notification n;
                    n.title = messages[msg % 4][0];
                    n.message = messages[msg % 4][1];
                    n.icon = msgIcons[msg % 4];
                    n.tint = msgTints[msg % 4];
                    ui->Notify(n);
                    logLine(std::string("[worker] notify: ") + n.title);
                    ++msg;
                }
                if (tick % 20 == 0)
                {
                    char buf[96];
                    std::snprintf(buf, sizeof(buf), "[worker] cpu %.0f%%  gpu %.0f%%", cpu.load() * 100.0f, gpu.load() * 100.0f);
                    logLine(buf);
                }
            }
        }

        // ---------------------------------------------------------- widgets
        // A custom widget built only from public pieces: ui::Interact + Painter (the iOS Control Center slider). Its
        // width is whatever the layout offers (ui::AvailableWidth), so it works in stacks and grids.
        bool TallSlider(const char* id, float* value, ui::Icon ic, float height)
        {
            const ui::Interaction it = ui::Interact(id, Vec2(ui::AvailableWidth(), height), ui::InteractFlags_PressOnClick);
            if (!it.visible)
                return false;
            bool changed = false;
            const esia::Id aid = ui::Salt(it.id, 1);
            float shown;
            if (it.held)
            {
                const float v = esia::Saturate(1.0f - (ctx->Input().MousePos().y - it.rect.min.y) / it.rect.Height());
                changed = v != *value;
                *value = v;
                ui::AnimSet(aid, v);
                shown = v;
            }
            else
                shown = ui::Anim(aid, *value, ui->GetTheme().motion.standard);
            const ui::Theme& t = ui->GetTheme();
            esia::Painter p = ui::GetPainter();
            const Rect r = it.rect;
            const float radius = S(22);
            p.PushScale(r.Center(), 1.0f + 0.035f * it.press);
            p.Rect(r, Style().Radius(radius).Glass(ui::LookMaterial(t.materials.control)).Shadow(t.colors.shadow, S(16), Vec2(0, S(6))));
            p.PushMask(r, radius);
            const float fillTop = r.max.y - r.Height() * shown;
            p.Rect(Rect(r.min.x, fillTop, r.max.x, r.max.y), Style().Fill(Color::White(0.92f)));
            p.PopMask();
            const bool covered = fillTop < r.max.y - S(34);
            p.Icon(Vec2(r.Center().x, r.max.y - S(24)), ui->IconFont(S(20)), ic, covered ? Color::Hex(0x3A3A3C) : t.colors.label);
            p.PopScale();
            return changed;
        }

        void GlassTile(const Rect& r, float radius = 22.0f)
        {
            const ui::Theme& t = ui->GetTheme();
            esia::Painter p = ui::GetPainter();
            Style s = Style().Radius(S(radius)).Glass(ui::LookMaterial(t.materials.control)).Shadow(t.colors.shadow, S(14), Vec2(0, S(5)));
            if (ui::CurrentGlassLook() == ui::GlassLook::Theme)
                s.Fill(t.colors.windowSurface.Fade(0.4f));
            p.Rect(r, s);
        }

        // A Control Center module: a glass tile as wide as its layout slot (height < 0 = square). The group makes the
        // module one child of the surrounding grid / stack; its contents are placed inside the tile's rect.
        Rect BeginModule(float height = -1.0f)
        {
            ctx->BeginGroup();
            const float w = ui::AvailableWidth();
            const Rect r = Rect::FromSize(ctx->CursorPos(), Vec2(w, height < 0.0f ? w : height));
            GlassTile(r);
            return r;
        }

        void EndModule(const Rect& r)
        {
            ctx->SetCursorPos(r.min);
            ctx->ItemSize(r.Size());
            ctx->EndGroup();
        }

        // a round toggle centered on `center`, `diameter` in UI units
        void ModuleToggle(const char* id, bool* on, ui::Icon ic, Color tint, Vec2 center, float diameter)
        {
            ctx->SetCursorPos(center - Vec2(diameter * 0.5f, diameter * 0.5f));
            ui::ToggleButton(id, on, ic, {.tint = tint, .diameter = diameter / S(1.0f)});
        }

        // ---------------------------------------------------------- panels
        void PanelSettings()
        {
            const ui::Theme& t = ui->GetTheme();
            ui::BeginNavigation("settings_nav", "root");
            if (!openPage.empty())
            {
                ui::NavigationPush(openPage);
                openPage.clear();
            }
            if (ui::BeginPage("root", "Settings"))
            {
                // the profile row (a row of its own)
                ui::BeginSection();
                Rect content;
                if (ui::BeginRow("profile", 76, &content))
                {
                    esia::Painter p = ui::GetPainter();
                    const Vec2 c(content.min.x + S(28), content.Center().y);
                    p.Circle(c, S(28), Style().Fill(Paint::Linear(Color::Hex(0xBF5AF2), Color::Hex(0x0A84FF), 45)).Shadow(t.colors.shadow, S(8), Vec2(0, S(3))));
                    p.TextBox(Rect::FromCenter(c, Vec2(S(56), S(56))), Vec2(0.5f, 0.5f), ui->Font(ui::TextStyle::Title2), Color::White(), "W");
                    // the text ends before the chevron and is cut with an ellipsis: nothing overlaps at any width
                    const float tx = content.min.x + S(70), tr = content.max.x - S(22);
                    p.TextBox(Rect(tx, content.Center().y - S(20), tr, content.Center().y), Vec2(0, 0), ui->Font(ui::TextStyle::Headline), t.colors.label,
                              "Waffle Player", esia::text::TextFlags_Ellipsis);
                    p.TextBox(Rect(tx, content.Center().y + S(2), tr, content.Center().y + S(20)), Vec2(0, 0), ui->Font(ui::TextStyle::Footnote),
                              t.colors.secondaryLabel, "Account, Cloud Saves & Achievements", esia::text::TextFlags_Ellipsis);
                    p.Icon(Vec2(content.max.x - S(6), content.Center().y), ui->IconFont(S(11)), icon::ChevronRight, t.colors.tertiaryLabel);
                }
                ui::EndRow();
                ui::EndSection();

                ui::BeginSection("Appearance");
                if (ui::RowToggle("Dark Mode", &darkMode, {icon::Moon, Color::Hex(0x5E5CE6)}))
                    ApplyTheme(true);
                if (ui::RowNavigation("Accent Color", {}, {icon::Palette, kAccents[accent]}))
                    ui::NavigationPush("accent");
                if (ui::RowSegmented("Text Size", &textSize, {"S", "M", "L"}, {icon::ZoomIn, Color::Hex(0x0A84FF)}))
                    ApplyTheme(true);
                ui::EndSection();

                ui::BeginSection("Display", "The frame limit paces the render thread; Esia's text is grayscale.");
                ui::RowToggle("VSync", &vsync, {icon::FullScreen, Color::Hex(0x0A84FF)});
                ui::RowSegmented("Frame Limit", &frameLimit, {"Off", "60", "120", "144"}, {icon::Lightning, Color::Hex(0xFF9F0A)});
                ui::RowSegmented("Text", &textAa, {"Auto", "Gray", "LCD"}, {icon::Edit, Color::Hex(0x64D2FF)});
                char rate[64];
                std::snprintf(rate, sizeof(rate), "%.0f fps  \xC2\xB7  %.2f ms", info.fps, info.fps > 0.0f ? 1000.0f / info.fps : 0.0f);
                ui::RowValue("Frame Rate", rate, {icon::Diagnostic, Color::Hex(0x30D158)});
                ui::RowValue("Threads", "Window + render thread", {icon::Sync, Color::Hex(0x5E5CE6)});
                ui::EndSection();

                ui::BeginSection("Liquid Glass", "Edits the live theme: every glass surface re-renders with the new material.");
                bool edited = false;
                edited |= ui::RowSlider("Frost", &frost, 0.0f, 48.0f, {icon::Cloud, Color::Hex(0x64D2FF)}, "%.0f");
                edited |= ui::RowSlider("Lensing", &refraction, 0.0f, 40.0f, {icon::View, Color::Hex(0x30D158)}, "%.0f");
                edited |= ui::RowSlider("Dispersion", &dispersion, 0.0f, 1.0f, {icon::Brightness, Color::Hex(0xFF9F0A)}, "%.2f");
                edited |= ui::RowToggle("Specular Rim", &specular, {icon::Lightbulb, Color::Hex(0xFFD60A)});
                if (ui::RowSegmented("Glass", &glassLook, {"Theme", "Clear", "Frosted"}, {icon::View, Color::Hex(0x0A84FF)}))
                    ui->SetGlassLook((ui::GlassLook)glassLook);
                ui::RowToggle("Search Bar Base", &searchBase, {icon::Search, Color::Hex(0x8E8E93)});
                if (edited)
                {
                    glassEdited = true;
                    ApplyTheme(false);
                }
                ui::EndSection();

                ui::BeginSection("Notifications");
                if (ui::RowButton("Send Test Notification"))
                {
                    ui::Notification n;
                    n.title = "Hello from the UI thread";
                    n.message = "Notifications can be posted from any thread";
                    n.icon = icon::Bell;
                    ui->Notify(n);
                }
                bool act = activity.load();
                if (ui::RowToggle("Background Download", &act, {icon::Download, Color::Hex(0x30D158)}))
                {
                    activity = act;
                    if (!act)
                        ui->ClearActivity("download");
                }
                ui::EndSection();

                ui::BeginSection("About");
                ui::RowValue("Version", "Esia UI", {icon::Info, Color::Hex(0x8E8E93)});
                ui::RowValue("Renderer", info.api, {icon::Game, Color::Hex(0xFF375F)});
                char scaleBuf[96];
                std::snprintf(scaleBuf, sizeof(scaleBuf), "%.0f%% scale  \xC2\xB7  %d x %d px", info.scale * 100.0f, info.width, info.height);
                ui::RowValue("Display", scaleBuf, {icon::FullScreen, Color::Hex(0x5E5CE6)});
                ui::RowValue("Text", "Grayscale  \xC2\xB7  FreeType + HarfBuzz", {icon::Edit, Color::Hex(0x30D158)});
                if (ui::RowNavigation("Performance", {}, {icon::Diagnostic, Color::Hex(0x30D158)}))
                    ui::NavigationPush("perf");
                ui::EndSection();
                // the floating Liquid Glass search field: the list scrolls under it (iOS Settings)
                ui::SearchBar("settings_search", &settingsSearch, {.base = searchBase});
                ui::EndPage();
            }

            if (ui::BeginPage("accent", "Accent Color"))
            {
                ui::BeginSection("Choose a tint", "The accent animates across every control.");
                Rect content;
                if (ui::BeginRow("swatches", 64, &content))
                {
                    ctx->SetCursorPos(Vec2(content.min.x, content.Center().y - S(17)));
                    if (ui::ColorSwatches("acc", &accent, kAccents, 7, 30))
                        ui->SetAccent(kAccents[accent]);
                }
                ui::EndRow();
                ui::EndSection();
                ui::BeginSection("Preview");
                static bool dummy = true;
                ui::RowToggle("Switch", &dummy);
                static float v = 0.6f;
                ui::RowSlider("Slider", &v, 0, 1, {}, nullptr);
                ui::RowButton("Button");
                ui::EndSection();
                ui::EndPage();
            }

            if (ui::BeginPage("perf", "Performance"))
            {
                const esia::render::RenderStats* s = info.stats;
                char buf[64];
                ui::BeginSection("Frame");
                std::snprintf(buf, sizeof(buf), "%.0f fps", info.fps);
                ui::RowValue("Frame rate", buf, {icon::Lightning, Color::Hex(0xFF9F0A)});
                std::snprintf(buf, sizeof(buf), "%.2f ms", info.cpuMs);
                ui::RowValue("UI build (CPU)", buf, {icon::Code, Color::Hex(0x0A84FF)});
                std::snprintf(buf, sizeof(buf), "%d", s ? s->drawCalls : 0);
                ui::RowValue("Draw calls", buf, {icon::Apps, Color::Hex(0x5E5CE6)});
                std::snprintf(buf, sizeof(buf), "%d", s ? s->fxInstances : 0);
                ui::RowValue("SDF instances", buf, {icon::Brush, Color::Hex(0xFF375F)});
                std::snprintf(buf, sizeof(buf), "%d", s ? s->backdropCaptures : 0);
                ui::RowValue("Glass captures", buf, {icon::View, Color::Hex(0x64D2FF)});
                std::snprintf(buf, sizeof(buf), "%d", s ? s->vertices : 0);
                ui::RowValue("Vertices", buf, {icon::Diagnostic, Color::Hex(0x30D158)});
                ui::EndSection();
                ui::BeginSection("History");
                Rect content;
                if (ui::BeginRow("graph", 90, &content))
                    ui::LineChart("fps", fpsHistory, {.rect = content.Shrunk(S(8)), .glow = chartGlow, .offset = fpsCursor});   // the oldest sample first
                ui::EndRow();
                ui::RowToggle("Glow", &chartGlow, {icon::Lightbulb, Color::Hex(0xFFD60A)});
                ui::EndSection();
                ui::EndPage();
            }
            ui::EndNavigation();
        }

        void PanelComponents()
        {
            const ui::Theme& t = ui->GetTheme();
            // any component can be made transparent: here a scope over the whole tab (ui::Next().Look does the same for
            // one component, see the "Clear" button)
            ui::Checkbox("Transparent components", &clearComponents);
            ui::Spacer(6);
            std::optional<ui::StyleScope> clearScope;
            if (clearComponents)
                clearScope.emplace(ui::ItemStyle().Look(ui::GlassLook::Clear));

            if (tab == 0)
            {
                ui::Text(ui::TextStyle::Title3, "Buttons");
                ui::Spacer(4);
                // a flow wraps the buttons with the panel width; the glowing button gets room for its halo, and its
                // glow fades out before it reaches a neighbour
                ui::BeginFlow("buttons");
                ui::Button("Filled");
                ui::Button("Tinted", {.kind = ui::ButtonKind::Tinted});
                ui::Button("Gray", {.kind = ui::ButtonKind::Gray});
                ui::Button("Plain", {.kind = ui::ButtonKind::Plain});
                ui::Button("Glass", {.kind = ui::ButtonKind::Glass, .icon = icon::Play});
                ui::Button("Prominent", {.kind = ui::ButtonKind::GlassProminent, .icon = icon::StarFill});
                ui::Button("Delete", {.kind = ui::ButtonKind::Destructive, .icon = icon::Delete});
                ui::Next().Look(ui::GlassLook::Clear);   // just this one: a Filled button made transparent
                ui::Button("Clear", {.icon = icon::View});
                ui::IconButton("i1", icon::Share, {.kind = ui::ButtonKind::Glass});
                ui::IconButton("i2", icon::HeartFill, {.kind = ui::ButtonKind::Glass});
                ui::IconButton("i3", icon::Add, {.kind = ui::ButtonKind::Filled});
                const bool more = ui::IconButton("i4", icon::More, {.kind = ui::ButtonKind::Tinted});
                const Rect moreRect = ctx->LastItemStatus().rect;
                ui::Button("Neon Glow", {.tint = Color::Hex(0xFF375F), .glow = true});
                if (ui::Button("Launch Game", {.size = ui::ControlSize::Large, .icon = icon::Game, .width = -1}))
                {
                    ui::Notification n;
                    n.title = "Launching";
                    n.message = "Starting the game client...";
                    n.icon = icon::Game;
                    ui->Notify(n);
                }
                ui::Tooltip("Buttons, tooltips and notifications are all Esia widgets");
                ui::EndFlow();
                if (more || openMenu)
                {
                    ui::OpenMenu("more");
                    openMenu = false;
                }
                if (ui::BeginMenu("more", {.anchor = Vec2(moreRect.max.x, moreRect.max.y + S(6)), .pivot = Vec2(1, 0)}))
                {
                    ui::MenuItem("Duplicate", icon::Copy);
                    ui::MenuItem("Rename", icon::Edit);
                    ui::MenuItem("Share", icon::Share);
                    ui::MenuItem("Delete", icon::Delete);
                    ui::EndMenu();
                }

                // per-component style: one line before a component changes just that component (no theme edits, no
                // push / pop pairs); only what is set changes
                ui::Spacer(8);
                ui::Text(ui::TextStyle::Title3, "Per-component style");
                ui::Spacer(4);
                ui::BeginFlow("styled");
                ui::Next().Tint(Color::Hex(0x30D158)).Radius(8);
                ui::Button("Tint + radius");
                ui::Next().Look(ui::GlassLook::Frosted).Blur(24).Fill(Color::Hex(0xBF5AF2));
                ui::Button("Frosted", {.icon = icon::StarFill});
                ui::Next().Look(ui::GlassLook::Clear).Refraction(20).Dispersion(0.9f).Magnify(0.15f);
                ui::Button("Lens");
                ui::Next().Opacity(0.45f);
                ui::Button("45%");
                ui::Next().Tint(Color::Hex(0xFF9F0A));
                ui::Toggle("styledToggle", &styledToggle);
                ui::EndFlow();
                ui::Next().Tint(Color::Hex(0xFF375F)).Radius(3);
                ui::Slider("styledSlider", &styledSlider, 0.0f, 1.0f);
                // track, selection after a switch, the lens while it moves, and the texts all have their own color
                ui::Next()
                    .Look(ui::GlassLook::Frosted)
                    .Fill(Color::Hex(0x0A84FF))
                    .Radius(10)
                    .SelectedFill(Color::White(0.92f))
                    .SelectedLabel(Color::Hex(0x0A64D8))
                    .MovingFill(Color::White(0.25f))
                    .Label(Color::White());
                ui::Segmented("styledSegment", &styledSegment, {"Day", "Week", "Month"});

                ui::Spacer(8);
                ui::Text(ui::TextStyle::Title3, "Switches");
                ui::Spacer(2);
                ui::BeginFlow("switches", {.spacing = 18});
                ui::Toggle("tA", &toggleA);
                ui::Toggle("tB", &toggleB);
                ui::Checkbox("Subtitles", &checkA);
                ui::Checkbox("HDR", &checkB);
                ui::EndFlow();

                ui::Spacer(8);
                ui::Text(ui::TextStyle::Title3, "Sliders");
                ui::Slider("s1", &slider, 0.0f, 1.0f, {.minIcon = icon::Mute, .maxIcon = icon::Volume});
                ui::Slider("s2", &sliderB, 0.0f, 128.0f, {.tint = Color::Hex(0xFF9F0A), .step = 8.0f});

                ui::Spacer(8);
                ui::Text(ui::TextStyle::Title3, "Segmented & Stepper");
                ui::Segmented("seg", &segment, {"Low", "Medium", "High", "Ultra"});
                ui::Spacer(4);
                ui::Stepper("step", &stepper, 0, 10);
                ctx->SameLine(0, S(12));
                ui::Text(ui::TextStyle::Body, "Squad size: %d", stepper);
            }
            else if (tab == 1)
            {
                ui::Text(ui::TextStyle::Title3, "Text");
                ui::Spacer(2);
                ui::TextField("name", &name, "Player name");
                ui::Spacer(4);
                ui::SearchField("search", &search, "Search servers");
                ui::Spacer(4);
                ui::TextField("pw", &password, "Password", {.icon = icon::Lock, .password = true});
                ui::Spacer(8);
                ui::Text(ui::TextStyle::Title3, "Menus");
                ui::Picker("region", &picker, {"Europe West", "North America", "Asia Pacific", "Oceania"});
                ui::Spacer(8);
                ui::Text(ui::TextStyle::Title3, "Swatches");
                ui::ColorSwatches("sw", &swatch, kAccents, 7);
                ui::Spacer(8);
                ui::Text(ui::TextStyle::Title3, "Stock ImGui (auto-themed)");
                ui::TextSecondary("Not in Esia: every widget here is Esia's own.");
            }
            else
            {
                ui::Text(ui::TextStyle::Title3, "Progress");
                ui::Spacer(4);
                ui::ProgressBar(download.load());
                ui::Spacer(8);
                ui::ProgressRing(cpu.load(), {.tint = Color::Hex(0x30D158), .diameter = 54});
                ctx->SameLine(0, S(14));
                ui::ProgressRing(gpu.load(), {.tint = Color::Hex(0xFF9F0A), .diameter = 54});
                ctx->SameLine(0, S(14));
                ui::ProgressRing(download.load(), {.diameter = 54});
                ctx->SameLine(0, S(18));
                ctx->SetCursorPos(ctx->CursorPos() + Vec2(0, S(15)));
                ui::ActivityIndicator();
                ui::Spacer(8);
                ui::Text(ui::TextStyle::Title3, "Charts");
                ui::Spacer(4);
                static float series[48];
                static bool seeded = false;
                if (!seeded)
                {
                    for (int i = 0; i < 48; ++i)
                        series[i] = 60.0f + 18.0f * std::sin((float)i * 0.23f) + 9.0f * std::sin((float)i * 0.71f + 1.0f) + (i > 30 ? 14.0f : 0.0f);
                    seeded = true;
                }
                ui::LineChart("smooth", series, {.height = 70.0f});   // smooth, soft fill, no glow
                ui::Spacer(6);
                ui::LineChart("neon", series, {.height = 70.0f, .tint = Color::Hex(0xFF375F), .smooth = false, .fill = false, .glow = true});
                ui::Spacer(8);
                ui::Text(ui::TextStyle::Title3, "Badges");
                ui::Badge("3");
                ctx->SameLine();
                ui::Badge("NEW", Color::Hex(0x0A84FF));
                ctx->SameLine();
                ui::Badge("LIVE", Color::Hex(0x30D158));
                ctx->SameLine();
                ui::Badge("BETA", Color::Hex(0xBF5AF2));
                ui::Spacer(8);
                ui::Text(ui::TextStyle::Title3, "Skeleton loading");
                esia::Painter p = ui::GetPainter();
                const Vec2 pos = ctx->CursorPos();
                const float w = ctx->ContentRegionAvail().x;
                const Style sk = Style().Radius(S(8)).Fill(t.colors.fill).Shimmer(0.35f, 0.6f);
                p.Circle(pos + Vec2(S(22), S(22)), S(22), Style().Fill(t.colors.fill).Shimmer(0.35f, 0.6f));
                p.Rect(Rect::FromSize(pos + Vec2(S(56), S(4)), Vec2(w * 0.55f, S(14))), sk);
                p.Rect(Rect::FromSize(pos + Vec2(S(56), S(26)), Vec2(w * 0.35f, S(12))), sk);
                p.Rect(Rect::FromSize(pos + Vec2(0, S(56)), Vec2(w, S(64))), Style(sk).Radius(S(14)));
                ctx->ItemSize(Vec2(w, S(128)));
            }
            ui::TabBar("tabs", &tab, {{icon::Apps, "Controls"}, {icon::Edit, "Inputs"}, {icon::Diagnostic, "Status"}});
        }

        void PanelEffects()
        {
            const ui::Theme& t = ui->GetTheme();
            const float time = (float)ui::Time();
            const float w = ctx->ContentRegionAvail().x;
            esia::Painter p = ui::GetPainter();

            // ---- the glass stage
            ui::Text(ui::TextStyle::Title3, "Liquid Glass");
            ui::TextColored(ui::TextStyle::Footnote, t.colors.secondaryLabel, "Backdrop blur, lensing, dispersion and specular rims over live content");
            ui::Spacer(4);
            Vec2 pos = ctx->CursorPos();
            const float stageH = S(190);
            const Rect stage = Rect::FromSize(pos, Vec2(w, stageH));
            p.Rect(stage, Style().Radius(S(18)).Fill(Paint::Linear(Color::Hex(0x5E2BFF), Color::Hex(0xFF6A3D), 20)));
            p.PushMask(stage, S(18));
            for (int i = 0; i < 9; ++i)   // stripes: sharp detail to see refraction
                p.Rect(Rect::FromSize(Vec2(stage.min.x + S(14) + (float)i * (w / 9.0f), stage.min.y), Vec2(S(5), stageH)), Style().Fill(Color::White(0.55f)));
            for (int i = 0; i < 5; ++i)
            {
                const float a = time * 0.6f + (float)i * 1.3f;
                const Vec2 c(stage.min.x + w * (0.12f + 0.19f * (float)i) + std::sin(a) * S(18), stage.Center().y + std::cos(a * 1.3f) * S(46));
                const Color cols[] = {Color::Hex(0x30D158), Color::Hex(0xFFD60A), Color::Hex(0x64D2FF), Color::Hex(0xFF375F), Color::Hex(0xFFFFFF)};
                p.Circle(c, S(22), Style().Fill(cols[i]));
            }
            p.PopMask();
            p.Text(stage.min + Vec2(S(16), S(12)), ui->Font(ui::FontWeight::Bold, 30), Color::White(), "Waffle");

            struct Sample
            {
                const char* label;
                esia::GlassMaterial m;
            };
            esia::GlassMaterial regular = t.materials.control;
            regular.blur = 14.0f;
            esia::GlassMaterial tinted = t.materials.control;
            tinted.tint = Color::Hex(0x0A84FF, 0.35f);
            tinted.blur = 8.0f;
            const Sample samples[] = {{"Clear", t.materials.clear}, {"Regular", regular}, {"Thick", t.materials.popover}, {"Tinted", tinted}};
            const float cellW = (w - S(24)) / 4.0f;
            for (int i = 0; i < 4; ++i)
            {
                const float bob = std::sin(time * 1.1f + (float)i * 0.9f) * S(6);
                const Rect r = Rect::FromCenter(Vec2(stage.min.x + S(12) + cellW * ((float)i + 0.5f), stage.Center().y + S(22) + bob), Vec2(cellW - S(14), S(82)));
                p.Rect(r, Style().Radius(S(24)).Glass(samples[i].m).Shadow(Color::Black(0.25f), S(18), Vec2(0, S(8))));
                p.TextBox(Bottom(r, S(26)), Vec2(0.5f, 0.5f), ui->Font(ui::TextStyle::Footnote), Color::White(0.95f), samples[i].label);
            }
            ctx->ItemSize(Vec2(w, stageH));

            // ---- liquid merge and a custom effect
            ui::Spacer(10);
            ui::Text(ui::TextStyle::Title3, "Liquid Morph & Custom Shader");
            ui::Spacer(4);
            pos = ctx->CursorPos();
            const float rowH = S(120);
            const Rect left = Rect::FromSize(pos, Vec2(w * 0.5f - S(6), rowH));
            const Rect right = Rect::FromSize(Vec2(pos.x + w * 0.5f + S(6), pos.y), Vec2(w * 0.5f - S(6), rowH));
            p.Rect(left, Style().Radius(S(18)).Fill(t.colors.cardSurface));
            const float sep = std::sin(time * 1.2f) * 0.5f + 0.5f;
            const Vec2 c1(left.Center().x - S(34) - sep * S(26), left.Center().y), c2(left.Center().x + S(34) + sep * S(26), left.Center().y);
            esia::GlassMaterial blob = t.materials.clear;
            blob.tint = Color::Hex(0x64D2FF, 0.25f);
            p.Merge(Rect::FromCenter(c1, Vec2(S(64), S(64))), Rect::FromCenter(c2, Vec2(S(52), S(52))), S(32), S(34),
                    Style().Glass(blob).Fill(Paint::Radial(Color::Hex(0x64D2FF, 0.35f), Color::Hex(0x5E5CE6, 0.55f))).Glow(Color::Hex(0x64D2FF), S(14), 0.5f));
            p.Rect(right, Style().Radius(S(18)).Effect(kAurora, 1.0f).Shadow(Color::Black(0.3f), S(16), Vec2(0, S(6))));
            p.TextBox(right, Vec2(0.5f, 0.5f), ui->Font(ui::TextStyle::Headline), Color::White(), "HLSL effect");
            ctx->ItemSize(Vec2(w, rowH));

            // ---- a glow layer (bloom on anything)
            ui::Spacer(10);
            ui::Text(ui::TextStyle::Title3, "Bloom Layers & Glow");
            ui::Spacer(4);
            pos = ctx->CursorPos();
            const Rect neon = Rect::FromSize(pos, Vec2(w, S(96)));
            p.Rect(neon, Style().Radius(S(18)).Fill(Color::Hex(0x06060A, 0.92f)));
            p.BeginGlowLayer(Color::Clear(), S(16), 1.35f, 1.0f);
            const esia::text::FontRef big = ui->Font(ui::FontWeight::Bold, 40);
            const Vec2 ts = p.MeasureText(big, "NEON");
            const float pulse = 0.88f + 0.12f * std::sin(time * 3.0f);
            // a neon tube: a near-white core, the cyan comes from the bloom around it
            p.Text(Vec2(neon.min.x + S(24), neon.Center().y - ts.y * 0.5f), big, Color::Hex(0xB4F2FF).Fade(pulse), "NEON");
            p.Ring(Vec2(neon.max.x - S(56), neon.Center().y), S(26), S(5), Style().Fill(Paint::ConicLoop(Color::Hex(0xFF375F), Color::Hex(0xBF5AF2), esia::Degrees(time * 2.0f))));
            p.Line(Vec2(neon.min.x + S(150), neon.Center().y + S(18)), Vec2(neon.max.x - S(110), neon.Center().y - S(18)), S(4), Style().Fill(Color::Hex(0x30D158)));
            p.EndGlowLayer();
            ctx->ItemSize(Vec2(w, S(96)));

            // ---- shapes
            ui::Spacer(10);
            ui::Text(ui::TextStyle::Title3, "Shapes, Gradients & Shadows");
            ui::Spacer(4);
            pos = ctx->CursorPos();
            const float s = (w - S(36)) / 4.0f;
            const Style base = Style().Radius(S(20));
            p.Rect(Rect::FromSize(pos, Vec2(s, s)), Style(base).Fill(Paint::Linear(Color::Hex(0xFF9F0A), Color::Hex(0xFF375F), 45)).Shadow(Color::Hex(0xFF375F, 0.45f), S(18), Vec2(0, S(8))));
            p.Rect(Rect::FromSize(pos + Vec2(s + S(12), 0), Vec2(s, s)), Style(base).Fill(Paint::Radial(Color::Hex(0x64D2FF), Color::Hex(0x0A2A6A), Vec2(0.3f, 0.25f), 0.9f)));
            p.Rect(Rect::FromSize(pos + Vec2((s + S(12)) * 2, 0), Vec2(s, s)), Style(base).Fill(Paint::ConicLoop(Color::Hex(0x30D158), Color::Hex(0xBF5AF2), esia::Degrees(time))));
            p.Rect(Rect::FromSize(pos + Vec2((s + S(12)) * 3, 0), Vec2(s, s)), Style(base).Fill(t.colors.cardSurface).InnerShadow(Color::Black(0.35f), S(14), Vec2(0, S(4))));
            ctx->ItemSize(Vec2(w, s));
            ui::Spacer(6);
            pos = ctx->CursorPos();
            for (int i = 0; i < 4; ++i)
            {
                const Vec2 c(pos.x + s * 0.5f + (float)i * (s + S(12)), pos.y + S(34));
                const float prog = std::fmod(time * 0.15f + (float)i * 0.23f, 1.0f);
                const Color col = kAccents[i + 1];
                p.Ring(c, S(26), S(7), Style().Fill(col.Fade(0.18f)));
                p.Arc(c, S(26), S(7), -esia::kPi * 0.5f, esia::kTau * prog, Style().Fill(col).Glow(col, S(6), 0.5f));
                char buf[8];
                std::snprintf(buf, sizeof(buf), "%d", (int)(prog * 100));
                p.TextBox(Rect::FromCenter(c, Vec2(S(40), S(20))), Vec2(0.5f, 0.5f), ui->Font(ui::TextStyle::Footnote), t.colors.label, buf);
            }
            ctx->ItemSize(Vec2(w, S(70)));
        }

        void PanelControlCenter()
        {
            const ui::Theme& t = ui->GetTheme();
            // modules flow through an adaptive grid: 2 columns at the default size, all 4 in a row when the panel is
            // wide, one column when it is narrow; every module sizes itself from its cell
            ui::BeginGrid("cc", {.minColumnWidth = 150, .maxColumns = 4, .spacing = 12});

            // connectivity: 2 x 2 round toggles
            {
                const Rect r = BeginModule();
                const float w = r.Width(), d = w * 0.34f;
                const float a = w * 0.29f, b = w * 0.71f;
                ModuleToggle("air", &airplane, icon::Airplane, Color::Hex(0xFF9F0A), r.min + Vec2(a, a), d);
                ModuleToggle("cell", &cellular, icon::Phone, Color::Hex(0x30D158), r.min + Vec2(b, a), d);
                ModuleToggle("wifi", &wifi, icon::Wifi, Color::Hex(0x0A84FF), r.min + Vec2(a, b), d);
                ModuleToggle("bt", &bluetooth, icon::Bluetooth, Color::Hex(0x0A84FF), r.min + Vec2(b, b), d);
                EndModule(r);
            }

            // now playing
            {
                const Rect r = BeginModule();
                const float w = r.Width(), pad = w * 0.094f;
                esia::Painter p = ui::GetPainter();
                const Rect art = Rect::FromSize(r.min + Vec2(pad, pad), Vec2(w * 0.33f, w * 0.33f));
                if (wallpaper)
                    p.Image(wallpaper, art, S(12), Color::White(), Vec2(0.35f, 0.3f), Vec2(0.6f, 0.7f));
                else
                    p.Rect(art, Style().Radius(S(12)).Fill(Paint::Linear(Color::Hex(0xFF375F), Color::Hex(0x5E5CE6), 45)));
                p.Text(Vec2(r.min.x + pad, art.max.y + w * 0.06f), ui->Font(ui::TextStyle::Headline), t.colors.label, "Glass Hearts");
                p.Text(Vec2(r.min.x + pad, art.max.y + w * 0.06f + S(20)), ui->Font(ui::TextStyle::Footnote), t.colors.secondaryLabel, "The Wafflers");
                ctx->SetCursorPos(Vec2(r.min.x + pad * 1.2f, r.max.y - pad - S(28)));
                const ui::ButtonOptions bo{.kind = ui::ButtonKind::Plain, .size = ui::ControlSize::Small};
                ui::IconButton("prev", icon::Back, bo);
                ctx->SameLine(0, S(12));
                if (ui::IconButton("play", playing ? icon::Pause : icon::Play, bo))
                    playing = !playing;
                ctx->SameLine(0, S(12));
                ui::IconButton("next", icon::Forward, bo);
                EndModule(r);
            }

            // brightness and volume: a stack in one cell, each slider half of it
            {
                const float h = ui::AvailableWidth();
                ui::BeginHStack("sliders", {.spacing = 12});
                TallSlider("bright", &brightness, icon::Brightness, h);
                TallSlider("vol", &volume, icon::Volume, h);
                ui::EndStack();
            }

            // focus and two small toggles
            {
                const float cw = ui::AvailableWidth();
                const float half = (cw - S(12)) * 0.5f;
                ui::BeginVStack("small", {.spacing = 12, .align = ui::Align::Start});
                {
                    const Rect r = BeginModule(half);
                    const float d = half * 0.70f;
                    ModuleToggle("focus_t", &focus, icon::Moon, Color::Hex(0x5E5CE6), Vec2(r.min.x + half * 0.5f, r.Center().y), d);
                    esia::Painter p = ui::GetPainter();
                    const float tx = r.min.x + half * 0.5f + d * 0.5f + S(12);
                    p.Text(Vec2(tx, r.Center().y - S(19)), ui->Font(ui::TextStyle::Headline), t.colors.label, "Focus");
                    p.Text(Vec2(tx, r.Center().y + S(1)), ui->Font(ui::TextStyle::Footnote), t.colors.secondaryLabel, focus ? "Gaming" : "Off");
                    EndModule(r);
                }
                ui::BeginHStack("pair", {.spacing = 12});
                {
                    const Rect r = BeginModule(half);
                    ModuleToggle("rot_t", &rotation, icon::Lock, Color::Hex(0xFF375F), r.Center(), half * 0.70f);
                    EndModule(r);
                }
                {
                    const Rect r = BeginModule(half);
                    ModuleToggle("mir_t", &mirror, icon::Connect, Color::Hex(0x64D2FF), r.Center(), half * 0.70f);
                    EndModule(r);
                }
                ui::EndStack();
                ui::EndStack();
            }
            ui::EndGrid();
        }

        void PanelLanguages()
        {
            const ui::Theme& t = ui->GetTheme();
            // nothing is configured for any of these: shaping, line breaking and fallback fonts come from the text system
            static const char* kSamples[][2] = {
                {"English", "The quick brown fox jumps over the lazy dog"},
                {"\xE7\xAE\x80\xE4\xBD\x93\xE4\xB8\xAD\xE6\x96\x87", "\xE6\xB6\xB2\xE6\x80\x81\xE7\x8E\xBB\xE7\x92\x83\xE7\x95\x8C\xE9\x9D\xA2\xEF\xBC\x8C\xE5\x8E\x9F\xE7\x94\x9F\xE6\x94\xAF\xE6\x8C\x81\xE5\xA4\x9A\xE8\xAF\xAD\xE8\xA8\x80\xE6\x98\xBE\xE7\xA4\xBA\xE4\xB8\x8E\xE8\xBE\x93\xE5\x85\xA5"},
                {"\xE7\xB9\x81\xE9\xAB\x94\xE4\xB8\xAD\xE6\x96\x87", "\xE6\xB6\xB2\xE6\x85\x8B\xE7\x8E\xBB\xE7\x92\x83\xE4\xBB\x8B\xE9\x9D\xA2\xEF\xBC\x8C\xE5\x8E\x9F\xE7\x94\x9F\xE6\x94\xAF\xE6\x8F\xB4\xE5\xA4\x9A\xE8\xAA\x9E\xE8\xA8\x80\xE9\xA1\xAF\xE7\xA4\xBA\xE8\x88\x87\xE8\xBC\xB8\xE5\x85\xA5"},
                {"\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E", "\xE3\x83\xAA\xE3\x82\xAD\xE3\x83\x83\xE3\x83\x89\xE3\x82\xB0\xE3\x83\xA9\xE3\x82\xB9\xE3\x81\xAE UI\xE3\x80\x81\xE3\x83\x8D\xE3\x82\xA4\xE3\x83\x86\xE3\x82\xA3\xE3\x83\x96\xE3\x81\xAA\xE5\xA4\x9A\xE8\xA8\x80\xE8\xAA\x9E\xE5\xAF\xBE\xE5\xBF\x9C"},
                {"\xED\x95\x9C\xEA\xB5\xAD\xEC\x96\xB4", "\xEB\xA6\xAC\xED\x80\xB4\xEB\x93\x9C \xEA\xB8\x80\xEB\x9E\x98\xEC\x8A\xA4 UI, \xEA\xB8\xB0\xEB\xB3\xB8 \xEB\x8B\xA4\xEA\xB5\xAD\xEC\x96\xB4 \xEC\xA7\x80\xEC\x9B\x90"},
                {"\xD8\xA7\xD9\x84\xD8\xB9\xD8\xB1\xD8\xA8\xD9\x8A\xD8\xA9", "\xD9\x88\xD8\xA7\xD8\xAC\xD9\x87\xD8\xA9 \xD8\xB2\xD8\xAC\xD8\xA7\xD8\xAC\xD9\x8A\xD8\xA9 \xD8\xB3\xD8\xA7\xD8\xA6\xD9\x84\xD8\xA9 \xD8\xA8\xD8\xAF\xD8\xB9\xD9\x85 \xD8\xA3\xD8\xB5\xD9\x8A\xD9\x84 \xD9\x84\xD9\x83\xD9\x84 \xD8\xA7\xD9\x84\xD9\x84\xD8\xBA\xD8\xA7\xD8\xAA"},
                {"\xD7\xA2\xD7\x91\xD7\xA8\xD7\x99\xD7\xAA", "\xD7\x9E\xD7\x9E\xD7\xA9\xD7\xA7 \xD7\x96\xD7\x9B\xD7\x95\xD7\x9B\xD7\x99\xD7\xAA \xD7\xA0\xD7\x95\xD7\x96\xD7\x9C\xD7\x99\xD7\xAA \xD7\xA2\xD7\x9D \xD7\xAA\xD7\x9E\xD7\x99\xD7\x9B\xD7\x94 \xD7\x9E\xD7\x9C\xD7\x90\xD7\x94 \xD7\x91\xD7\xA9\xD7\xA4\xD7\x95\xD7\xAA"},
                {"\xE0\xA4\xB9\xE0\xA4\xBF\xE0\xA4\xA8\xE0\xA5\x8D\xE0\xA4\xA6\xE0\xA5\x80", "\xE0\xA4\xA4\xE0\xA4\xB0\xE0\xA4\xB2 \xE0\xA4\x97\xE0\xA5\x8D\xE0\xA4\xB2\xE0\xA4\xBE\xE0\xA4\xB8 \xE0\xA4\x87\xE0\xA4\x82\xE0\xA4\x9F\xE0\xA4\xB0\xE0\xA4\xAB\xE0\xA4\xBC\xE0\xA5\x87\xE0\xA4\xB8, \xE0\xA4\xAE\xE0\xA5\x82\xE0\xA4\xB2 \xE0\xA4\xAC\xE0\xA4\xB9\xE0\xA5\x81\xE0\xA4\xAD\xE0\xA4\xBE\xE0\xA4\xB7\xE0\xA5\x80 \xE0\xA4\xB8\xE0\xA4\xAE\xE0\xA4\xB0\xE0\xA5\x8D\xE0\xA4\xA5\xE0\xA4\xA8"},
                {"\xE0\xB9\x84\xE0\xB8\x97\xE0\xB8\xA2", "\xE0\xB8\xAD\xE0\xB8\xB4\xE0\xB8\x99\xE0\xB9\x80\xE0\xB8\x97\xE0\xB8\xAD\xE0\xB8\xA3\xE0\xB9\x8C\xE0\xB9\x80\xE0\xB8\x9F\xE0\xB8\x8B\xE0\xB8\x81\xE0\xB8\xA3\xE0\xB8\xB0\xE0\xB8\x88\xE0\xB8\x81\xE0\xB9\x80\xE0\xB8\xAB\xE0\xB8\xA5\xE0\xB8\xA7 \xE0\xB8\xA3\xE0\xB8\xAD\xE0\xB8\x87\xE0\xB8\xA3\xE0\xB8\xB1\xE0\xB8\x9A\xE0\xB8\xAB\xE0\xB8\xA5\xE0\xB8\xB2\xE0\xB8\xA2\xE0\xB8\xA0\xE0\xB8\xB2\xE0\xB8\xA9\xE0\xB8\xB2\xE0\xB9\x83\xE0\xB8\x99\xE0\xB8\x95\xE0\xB8\xB1\xE0\xB8\xA7"},
                {"\xD0\xA0\xD1\x83\xD1\x81\xD1\x81\xD0\xBA\xD0\xB8\xD0\xB9", "\xD0\x98\xD0\xBD\xD1\x82\xD0\xB5\xD1\x80\xD1\x84\xD0\xB5\xD0\xB9\xD1\x81 \xD0\xB8\xD0\xB7 \xD0\xB6\xD0\xB8\xD0\xB4\xD0\xBA\xD0\xBE\xD0\xB3\xD0\xBE \xD1\x81\xD1\x82\xD0\xB5\xD0\xBA\xD0\xBB\xD0\xB0, \xD0\xB2\xD1\x81\xD0\xB5 \xD1\x8F\xD0\xB7\xD1\x8B\xD0\xBA\xD0\xB8 \xD1\x81\xD1\x80\xD0\xB0\xD0\xB7\xD1\x83"},
                {"\xCE\x95\xCE\xBB\xCE\xBB\xCE\xB7\xCE\xBD\xCE\xB9\xCE\xBA\xCE\xAC", "\xCE\x94\xCE\xB9\xCE\xB5\xCF\x80\xCE\xB1\xCF\x86\xCE\xAE \xCF\x85\xCE\xB3\xCF\x81\xCE\xBF\xCF\x8D \xCE\xB3\xCF\x85\xCE\xB1\xCE\xBB\xCE\xB9\xCE\xBF\xCF\x8D \xCE\xB3\xCE\xB9\xCE\xB1 \xCE\xBA\xCE\xAC\xCE\xB8\xCE\xB5 \xCE\xB3\xCE\xBB\xCF\x8E\xCF\x83\xCF\x83\xCE\xB1"},
                {"Ti\xE1\xBA\xBFng Vi\xE1\xBB\x87t", "Giao di\xE1\xBB\x87n k\xC3\xADnh l\xE1\xBB\x8Fng, h\xE1\xBB\x97 tr\xE1\xBB\xA3 \xC4\x91\x61 ng\xC3\xB4n ng\xE1\xBB\xAF"},
                {"Emoji", "\xF0\x9F\x8E\xAE \xF0\x9F\x9A\x80 \xE2\x9C\xA8 \xF0\x9F\xA7\x87 \xF0\x9F\x91\x8D\xF0\x9F\x8F\xBD \xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7 \xF0\x9F\x87\xA8\xF0\x9F\x87\xB3 \xF0\x9F\x87\xAF\xF0\x9F\x87\xB5 \xF0\x9F\x87\xB0\xF0\x9F\x87\xB7"},
            };
            ui::TextColored(ui::TextStyle::Footnote, t.colors.secondaryLabel, "No fonts or glyph ranges were loaded for any of these.");
            ui::Spacer(6);
            ui::BeginVStack("samples", {.spacing = 10, .align = ui::Align::Start});
            for (const auto& sample : kSamples)
            {
                ctx->BeginGroup();
                ui::TextColored(ui::TextStyle::Caption1, t.colors.secondaryLabel, "%s", sample[0]);
                ui::TextWrapped(ui::TextStyle::Body, t.colors.label, sample[1]);
                ctx->EndGroup();
            }
            ui::EndStack();
            ui::Spacer(10);
            ui::Text(ui::TextStyle::Headline, "Type in any language");
            ui::Spacer(4);
            ui::TextField("lang_input", &langInput,
                          "\xE4\xB8\xAD\xE6\x96\x87 / \xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E / \xED\x95\x9C\xEA\xB5\xAD\xEC\x96\xB4 / \xD8\xA7\xD9\x84\xD8\xB9\xD8\xB1\xD8\xA8\xD9\x8A\xD8\xA9 ...",
                          {.icon = icon::Globe});
            ui::Spacer(4);
            ui::TextColored(ui::TextStyle::Footnote, t.colors.secondaryLabel, "IME composition is drawn inline; the caret moves by grapheme clusters.");
        }

        void PanelTelemetry()
        {
            const ui::Theme& t = ui->GetTheme();
            {
                std::lock_guard lock(logMutex);
                for (std::string& s : logIn)
                {
                    log.push_back(std::move(s));
                    if (log.size() > 200)
                        log.erase(log.begin());
                }
                logIn.clear();
            }
            ui::TextColored(ui::TextStyle::Footnote, t.colors.secondaryLabel, "Values below are written by a worker thread (atomics and a locked queue).");
            ui::Spacer(6);
            const float c = cpu.load(), g = gpu.load();
            // a gauge: a ring and its caption, one item of the flow
            const auto gauge = [&](float value, Color tint, const char* title, const char* detail) {
                ctx->BeginGroup();
                ui::ProgressRing(value, {.tint = tint, .diameter = 64, .thickness = 8});
                ctx->SameLine(0, S(16));
                ctx->BeginGroup();
                ui::Text(ui::TextStyle::Headline, "%s", title);
                ui::TextSecondary("%s", detail);
                ctx->EndGroup();
                ctx->EndGroup();
            };
            // side by side while they fit; the second one goes to its own line in a narrow window (never clipped)
            ui::BeginFlow("gauges", {.spacing = 28});
            char title[32], detail[48];
            std::snprintf(title, sizeof(title), "CPU %.0f%%", c * 100.0f);
            gauge(c, Color::Hex(0x30D158), title, "Game thread load");
            std::snprintf(title, sizeof(title), "GPU %.0f%%", g * 100.0f);
            std::snprintf(detail, sizeof(detail), "%d players online", players.load());
            gauge(g, Color::Hex(0xFF9F0A), title, detail);
            ui::EndFlow();
            ui::Spacer(10);
            ui::BeginSection("Event log");
            const int n = (int)log.size();
            for (int i = std::max(0, n - 8); i < n; ++i)
            {
                ctx->PushId(i);
                ui::RowValue(log[(std::size_t)i], "");
                ctx->PopId();
            }
            if (n == 0)
                ui::RowValue("Waiting for events...", "");
            ui::EndSection();
        }

        void PanelPlugin()
        {
            // WGT loaded this panel from a plugin DLL; Esia has no plugin host yet, so the showcase draws it
            ui::Text(ui::TextStyle::Headline, "WGT's plugin panel");
            ui::TextSecondary("In WGT, this panel, its shader and its state lived in a plugin DLL.");
            ui::Spacer(8);
            esia::Painter p = ui::GetPainter();
            const Vec2 pos = ctx->CursorPos();
            const float w = ctx->ContentRegionAvail().x;
            const Rect card = Rect::FromSize(pos, Vec2(w, S(150)));
            p.Rect(card, Style().Radius(S(20)).Effect(kHolo, 1.0f, 0.6f).Shadow(Color::Black(0.3f), S(18), Vec2(0, S(8))));
            p.Text(card.min + Vec2(S(18), S(16)), ui->Font(ui::TextStyle::Title2), Color::White(), "Holo Card");
            p.Text(card.min + Vec2(S(18), S(46)), ui->Font(ui::TextStyle::Footnote), Color::White(0.8f), "Custom HLSL via Renderer::SetEffectSource");
            ctx->ItemSize(Vec2(w, S(150)));
            ui::Spacer(10);
            if (ui::Button("Tap me", {.kind = ui::ButtonKind::GlassProminent, .icon = icon::Add}))
                ++taps;
            ctx->SameLine(0, S(12));
            ui::Text(ui::TextStyle::Body, "%d taps", taps);
        }

        // ---------------------------------------------------- the desktop
        // A display smaller than the desktop layout: a phone (in either orientation), not a laptop's window.
        bool Compact() const
        {
            const Vec2 d = ctx->DisplaySize();
            return d.x < S(720) || d.y < S(600);
        }

        // UI units: the part of the display the system draws nothing over
        Rect Safe() const
        {
            const Vec2 d = ctx->DisplaySize();
            return info.safeArea.Width() > 0.0f ? info.safeArea : Rect(0, 0, d.x, d.y);
        }

        // The status bar's top on a compact display: over the dock (ui::Dock: 46 tiles, 11 padding, 16 from the
        // bottom). Not at the top, where the island opens.
        float CompactStatusTop() const { return ctx->DisplaySize().y - S(46 + 22 + 16) - S(10) - S(30); }

        // A panel on a compact display: centered in the safe area, over the status bar, as large as fits.
        void FitCompact(ui::WindowOptions& wo) const
        {
            const Rect safe = Safe();
            const float m = S(10);
            const float top = safe.min.y + m;
            const float bottom = CompactStatusTop() - m;
            const Vec2 size(std::min(S(wo.size.x), safe.Width() - 2.0f * m), std::min(S(wo.size.y), bottom - top));
            wo.size = size / S(1);
            wo.pos = Vec2(safe.Center().x - size.x * 0.5f, top) / S(1);
        }

        void Background()
        {
            const Vec2 d = ctx->DisplaySize();
            const Rect screen(0, 0, d.x, d.y);
            const float time = (float)ui::Time();
            esia::PainterEnv env;
            env.pixelScale = ctx->FramebufferScale().x;
            esia::Painter p(ctx->BackgroundDrawList(), env);
            if (wallpaper && wallpaperWidth > 0)
            {
                // cover-fit
                const float sa = screen.Width() / screen.Height();
                const float ia = (float)wallpaperWidth / (float)wallpaperHeight;
                Vec2 uv0(0, 0), uv1(1, 1);
                if (ia > sa)
                {
                    uv0.x = (1.0f - sa / ia) * 0.5f;
                    uv1.x = 1.0f - uv0.x;
                }
                else
                {
                    uv0.y = (1.0f - ia / sa) * 0.5f;
                    uv1.y = 1.0f - uv0.y;
                }
                p.Image(wallpaper, screen, 0.0f, Color::White(), uv0, uv1);
            }
            else
                p.Rect(screen, Style().Fill(Paint::Linear(Color::Hex(0x1B1464), Color::Hex(0xC2185B), 35)));

            // drifting light blobs: something alive for the glass to refract
            const Color blobs[] = {Color::Hex(0xFF375F, 0.55f), Color::Hex(0x64D2FF, 0.50f), Color::Hex(0xFFD60A, 0.45f), Color::Hex(0xBF5AF2, 0.50f)};
            for (int i = 0; i < 4; ++i)
            {
                const float a = time * (0.13f + 0.04f * (float)i) + (float)i * 1.7f;
                const Vec2 c(screen.min.x + screen.Width() * (0.5f + 0.38f * std::sin(a)), screen.min.y + screen.Height() * (0.5f + 0.34f * std::cos(a * 1.31f)));
                const float r = screen.Height() * (0.12f + 0.03f * (float)i);
                p.Circle(c, r, Style().Fill(Paint::Radial(blobs[i], blobs[i].Fade(0.0f), Vec2(0.5f, 0.5f), 0.5f)));
            }
        }

        // the status bar (bottom left): immediate-mode drawing outside any window
        void StatusBar()
        {
            fpsHistory[fpsCursor] = info.fps;
            fpsCursor = (fpsCursor + 1) % 120;
            const ui::Theme& t = ui->GetTheme();
            const esia::render::RenderStats* s = info.stats;
            char buf[256];
            const bool compact = Compact();
            if (compact)
                std::snprintf(buf, sizeof(buf), "%.0f fps  |  CPU %.2f ms  GPU %.2f ms  |  %d glass passes", info.fps, info.cpuMs,
                              s && s->gpu.valid ? s->gpu.totalMs : 0.0f, s ? s->backdropCaptures : 0);
            else
                std::snprintf(buf, sizeof(buf), "%s  \xC2\xB7  %s  |  %.0f fps  |  UI: CPU %.2f ms  GPU %.2f ms  |  %d draws  |  %d shapes  |  %d glass passes", info.api,
                              info.adapter.c_str(), info.fps, info.cpuMs, s && s->gpu.valid ? s->gpu.totalMs : 0.0f, s ? s->drawCalls : 0, s ? s->fxInstances : 0,
                              s ? s->backdropCaptures : 0);
            esia::PainterEnv env;
            env.pixelScale = ctx->FramebufferScale().x;
            env.text = ui->Text();
            esia::Painter p(ctx->BackgroundDrawList(), env);
            const esia::text::FontRef f = ui->Font(ui::TextStyle::Footnote);
            const Vec2 ts = p.MeasureText(f, buf);
            // bottom left; on a compact display the dock is there: centered over it
            const float w = ts.x + S(28);
            const Vec2 at = compact ? Vec2(std::floor(Safe().Center().x - w * 0.5f), CompactStatusTop()) : Vec2(S(14), ctx->DisplaySize().y - S(44));
            const Rect r = Rect::FromSize(at, Vec2(w, S(30)));
            Style hs = Style().Glass(ui::LookMaterial(t.materials.bar));
            if (ui::CurrentGlassLook() == ui::GlassLook::Theme)
                hs.Fill(t.colors.windowSurface.Fade(0.5f));
            p.Capsule(r, hs);
            p.Text(Vec2(r.min.x + S(14), std::floor(r.Center().y - ts.y * 0.5f)), f, t.colors.label, buf);
        }

        void Frame()
        {
            if (++frameCount == openLaterFrame && openLater >= 0)
                open[openLater] = true;
            ui->NewFrame();
            const bool compact = Compact();
            if (compact && frameCount == 1)
            {
                // one panel at a time: the first of those asked for
                int kept = -1;
                for (int i = 0; i < PanelCount; ++i)
                    if (open[i] && kept >= 0)
                        open[i] = false;
                    else if (open[i])
                        kept = i;
            }
            Background();
            StatusBar();
            for (int i = 0; i < PanelCount; ++i)
            {
                if (!open[i])
                    continue;
                const PanelInfo& pi = kPanels[i];
                ui::WindowOptions wo;
                wo.size = pi.size;
                wo.pos = spread ? kSpread[i] : pi.pos;
                if (compact)
                    FitCompact(wo);
                wo.icon = pi.icon;
                if (i == Settings)
                    wo.flags = ui::WindowFlags_NoScroll;   // the navigation pages scroll
                if (!ui::BeginWindow(pi.title, &open[i], wo))
                    continue;
                switch (i)
                {
                case Settings: PanelSettings(); break;
                case Effects: PanelEffects(); break;
                case Control: PanelControlCenter(); break;
                case Components: PanelComponents(); break;
                case Languages: PanelLanguages(); break;
                case Telemetry: PanelTelemetry(); break;
                case Plugin: PanelPlugin(); break;
                default: break;
                }
                ui::EndWindow();
            }
            ui::DockItem items[PanelCount];
            for (int i = 0; i < PanelCount; ++i)
                items[i] = {kPanels[i].title, kPanels[i].icon, Color::Hex(kPanels[i].color), &open[i]};
            const int clicked = ui::Dock(items);
            if (compact && clicked >= 0 && open[clicked])   // the panel opened replaces the one before
                for (int i = 0; i < PanelCount; ++i)
                    if (i != clicked)
                        open[i] = false;
            ui->EndFrame();
        }
    };
}

int main(int argc, char** argv)
{
    Demo d;
    for (int i = 0; i < PanelCount; ++i)
        d.open[i] = kPanels[i].open;
    ui::GlassLook look = ui::GlassLook::Frosted;
    glass::App app;
    app.name = "showcase";
    app.loadFonts = false;   // the Ui loads the platform's UI font, its weights, the CJK chain and the icon font
    app.option = [&](const std::string& o, const char* value, bool& usedValue) {
        if (o == "--light")
            return !(d.darkMode = false);
        if (o == "--dark")
            return d.darkMode = true;
        if (o == "--open-all")
        {
            for (bool& b : d.open)
                b = true;
            return true;
        }
        if (o == "--spread")
        {
            for (bool& b : d.open)
                b = true;
            return d.spread = true;
        }
        if (o == "--menu")
            return d.openMenu = true;
        if (!value)
            return false;
        usedValue = true;
        if (o == "--open")
        {
            const std::string list = std::string(",") + value + ",";
            for (int i = 0; i < PanelCount; ++i)
                d.open[i] = list.find(std::string(",") + kPanels[i].id + ",") != std::string::npos;
            return true;
        }
        if (o == "--open-later")
        {
            // panel@frame: the panel opens at that frame, as a click on its dock tile would
            const std::string v = value;
            const std::size_t at = v.find('@');
            for (int i = 0; i < PanelCount && at != std::string::npos; ++i)
                if (v.compare(0, at, kPanels[i].id) == 0)
                {
                    d.openLater = i;
                    d.openLaterFrame = std::atoi(v.c_str() + at + 1);
                }
            return d.openLater >= 0;
        }
        if (o == "--look")
        {
            look = std::strcmp(value, "clear") == 0 ? ui::GlassLook::Clear : std::strcmp(value, "theme") == 0 ? ui::GlassLook::Theme : ui::GlassLook::Frosted;
            return true;
        }
        if (o == "--tab")
        {
            d.tab = std::clamp(std::atoi(value), 0, 2);
            return true;
        }
        if (o == "--page")
        {
            d.openPage = value;
            return true;
        }
        usedValue = false;
        return false;
    };
    app.init = [&](esia::Context& ctx, esia::text::TextSystem* text, esia::text::FontId, const std::vector<std::string>& fonts) {
        d.ctx = &ctx;
        ui::UiDesc desc;
        desc.text = text;
        if (!fonts.empty())
            desc.fontFiles[0] = fonts[0];
        desc.theme = d.darkMode ? ui::ThemeDark() : ui::ThemeLight();
        desc.glassLook = look;
#if !defined(_WIN32)
        desc.iconFontFile = glass::kSystemSymbolsFont;   // SF Symbols, or the Linux desktop's icon theme, for the icons
#endif
        d.glassLook = (int)look;
        d.ui = std::make_unique<ui::Ui>(ctx, desc);
        // the wallpaper WGT's demo used, scaled down to 2560 on its longer side
        showcase::ImageFile wall;
        // (on macOS one of the system's: WGT's is a Windows file; on iOS the app bundle's, from the Mac that built it; on
        // Linux Ubuntu's, else GNOME's)
#if defined(__APPLE__) && TARGET_OS_IPHONE
        for (const wchar_t* path : {L"wallpaper.jpg"})
#elif defined(__APPLE__)
        for (const wchar_t* path : {L"/System/Library/Desktop Pictures/Sonoma.heic", L"/System/Library/Desktop Pictures/Mac Blue.heic"})
#elif defined(_WIN32)
        for (const wchar_t* path : {L"C:\\Windows\\Web\\Wallpaper\\Windows\\img0.jpg", L"C:\\Windows\\Web\\4K\\Wallpaper\\Windows\\img0_1920x1200.jpg"})
#else
        for (const wchar_t* path : {L"/usr/share/backgrounds/warty-final-ubuntu.png", L"/usr/share/backgrounds/gnome/adwaita-l.jpg"})
#endif
            if (showcase::LoadImageFile(path, wall, 2560))
                break;
        if (!wall.rgba.empty())
        {
            d.wallpaper = ctx.Textures().Create({esia::TextureFormat::RGBA8, wall.width, wall.height}, wall.rgba.data());
            d.wallpaperWidth = wall.width;
            d.wallpaperHeight = wall.height;
        }
        d.running = true;
        d.worker = std::thread([&] { d.WorkerMain(); });
    };
    app.renderer = [](esia::render::Renderer& r) {
        r.SetEffectSource(kAurora, "aurora", kAuroraHlsl);
        r.SetEffectSource(kHolo, "holographic", kHoloHlsl);
    };
    app.frame = [&](esia::Context&, const glass::SceneInfo& info) {
        d.info = info;
        d.Frame();
    };
    app.shutdown = [&] {
        d.running = false;
        if (d.worker.joinable())
            d.worker.join();
        d.ui.reset();
    };
    return glass::RunApp(argc, argv, app);
}
