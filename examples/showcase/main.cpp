// showcase - the widget layer (esia::ui) in a window: WGT's showcase, screen by screen as the widgets are ported.
//
//   showcase.exe [app options, see ../glass_window/app.hpp] [--dark] [--look theme|clear|frosted] [--tab 0|1|2|3]
//                [--page accent] [--menu]
#include "app.hpp"
#include "esia/render/painter.hpp"
#include "esia/ui/ui.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

namespace ui = esia::ui;
using esia::Color;
using esia::Paint;
using esia::Rect;
using esia::Style;
using esia::Vec2;

namespace
{
    const Color kAccents[] = {Color::Hex(0x0A84FF), Color::Hex(0x30D158), Color::Hex(0xFF9F0A), Color::Hex(0xFF375F), Color::Hex(0xBF5AF2)};

    struct Showcase
    {
        std::unique_ptr<ui::Ui> ui;
        bool dark = false;
        ui::GlassLook look = ui::GlassLook::Frosted;

        // state the widgets edit
        bool wifi = true, bluetooth = true, airplane = false, focus = false;
        bool hdr = true, autoLock = false, check1 = true, check2 = false;
        float volume = 0.6f, brightness = 0.8f, stepped = 48.0f;
        int count = 3, accent = 0;
        bool showComponents = true, showControl = true, showTelemetry = true, showSettings = true;
        // Settings
        bool vsync = true, specular = true, download = false;
        int textSize = 1, frameLimit = 0, glassLook = 2;
        float frost = 10.0f, lensing = 14.0f, dispersion = 0.30f;
        int presses = 0;
        int tab = 0, period = 1;
        bool previewOn = true;
        float previewValue = 0.6f;
        std::string openPage;   // --page: pushed on the first frame
        bool openMenu = false;  // --menu: the Inputs tab's menu opens on the first frame
        int size = 1, resolution = 2;
        float frameTimes[120] = {};
        int frameCursor = 0;
        bool pinned = true;
        std::string name = "Esia", query, password = "secret", settingsQuery;
        std::string scripts = "Hello · 你好 · مرحبا · こんにちは · café";
        std::string renderer;   // the API and the adapter

        void Wallpaper(esia::Context& ctx)
        {
            // WGT's showcase background: a deep gradient with soft color blobs that drift, so glass has something to
            // refract and blur
            const Vec2 d = ctx.DisplaySize();
            const float t = (float)ui::Time();
            esia::PainterEnv env;
            env.pixelScale = ctx.FramebufferScale().x;
            esia::Painter p(ctx.BackgroundDrawList(), env);
            const Rect all(0, 0, d.x, d.y);
            if (dark)
                p.Rect(all, Style().Fill(Paint::Linear(Color::Hex(0x0B1026), Color::Hex(0x2A0F3A), 120)));
            else
                p.Rect(all, Style().Fill(Paint::Linear(Color::Hex(0xBFD7FF), Color::Hex(0xFFD6E8), 120)));
            const struct
            {
                float x, y, r;
                std::uint32_t color;
                float speed;
            } blobs[] = {{0.18f, 0.25f, 0.30f, 0xFF9F0A, 0.21f}, {0.78f, 0.30f, 0.26f, 0x0A84FF, 0.17f}, {0.55f, 0.78f, 0.32f, 0xFF375F, 0.13f},
                         {0.12f, 0.80f, 0.22f, 0x30D158, 0.19f}, {0.90f, 0.85f, 0.20f, 0xBF5AF2, 0.23f}};
            for (const auto& b : blobs)
            {
                const Vec2 c(d.x * (b.x + 0.04f * std::sin(t * b.speed * 3.0f)), d.y * (b.y + 0.04f * std::cos(t * b.speed * 2.0f)));
                const float r = std::min(d.x, d.y) * b.r;
                p.Circle(c, r, Style().Fill(Paint::Radial(Color::Hex(b.color, dark ? 0.55f : 0.75f), Color::Hex(b.color, 0.0f))));
            }
        }

        void Components()
        {
            ui::WindowOptions wo;
            wo.size = Vec2(430, 700);
            wo.pos = Vec2(40, 40);
            wo.subtitle = "Buttons, switches, sliders, progress";
            if (!ui::BeginWindow("Components", &showComponents, wo))
                return;
            if (tab == 0)
            {
                ui::Headline("Buttons");
                ui::BeginFlow("buttons");
                ui::Button("Filled");
                ui::Button("Tinted", {.kind = ui::ButtonKind::Tinted});
                ui::Button("Gray", {.kind = ui::ButtonKind::Gray});
                ui::Button("Plain", {.kind = ui::ButtonKind::Plain});
                ui::Button("Glass", {.kind = ui::ButtonKind::Glass});
                ui::Button("Prominent", {.kind = ui::ButtonKind::GlassProminent});
                ui::Button("Delete", {.kind = ui::ButtonKind::Destructive, .icon = ui::icons::Delete});
                ui::IconButton("add", ui::icons::Add, {.kind = ui::ButtonKind::Tinted});
                ui::IconButton("play", ui::icons::Play, {.kind = ui::ButtonKind::Glass});
                ui::Next().Tint(Color::Hex(0x30D158));
                ui::Button("Glow", {.glow = true});
                ui::EndFlow();
                ui::Button("Continue", {.size = ui::ControlSize::Large, .width = -1});

                ui::Spacer();
                ui::Headline("Switches");
                ui::BeginFlow("switches", {.spacing = 16});
                ui::Toggle("hdr", &hdr);
                ui::Next().Tint(Color::Hex(0xFF9F0A));
                ui::Toggle("lock", &autoLock);
                ui::Checkbox("Notifications", &check1);
                ui::Checkbox("Sounds", &check2);
                ui::EndFlow();

                ui::Spacer();
                ui::Headline("Sliders");
                ui::Slider("volume", &volume, 0.0f, 1.0f, {.minIcon = ui::icons::Mute, .maxIcon = ui::icons::Volume});
                ui::Slider("stepped", &stepped, 0.0f, 128.0f, {.step = 16.0f});

                ui::Spacer();
                ui::Headline("Segmented");
                ui::Segmented("period", &period, {"Day", "Week", "Month", "Year"});
                ui::BeginHStack("stepper", {.align = ui::Align::Center});
                ui::Stepper("count", &count, 0, 10);
                ui::Text(ui::TextStyle::Body, "Count: %d", count);
                ui::EndStack();
            }
            else if (tab == 1)
            {
                ui::Headline("Text fields");
                ui::TextField("name", &name, "Your name");
                ui::SearchField("search", &query);
                ui::TextField("password", &password, "Password", {.password = true});
                ui::Spacer();
                ui::Headline("Any script");
                ui::TextField("scripts", &scripts, "Type in any language", {.icon = ui::icons::Globe});
                ui::TextSecondary("Graphemes, words (Ctrl), selection, clipboard, undo, IME in place.");

                ui::Spacer();
                ui::Headline("Pickers and menus");
                bool openActions = openMenu;
                openMenu = false;
                ui::BeginHStack("pickers", {.spacing = 10, .align = ui::Align::Center});
                ui::Picker("size", &size, {"Small", "Medium", "Large", "Extra Large"});
                ui::FlexSpacer();
                openActions |= ui::IconButton("more", ui::icons::More, {.kind = ui::ButtonKind::Gray});
                const Rect more = ui->GetContext().LastItemStatus().rect;
                ui::Tooltip("More actions");
                ui::EndStack();
                if (openActions)
                    ui::OpenMenu("actions");
                if (ui::BeginMenu("actions", {.anchor = Vec2(more.max.x, more.max.y + ui::S(6)), .pivot = Vec2(1, 0)}))
                {
                    ui::MenuItem("Duplicate", ui::icons::Copy);
                    ui::MenuItem("Rename", ui::icons::Edit);
                    ui::MenuItem("Share", ui::icons::Share);
                    if (ui::MenuItem("Pinned", ui::icons::Pin, pinned))
                        pinned = !pinned;
                    ui::MenuItem("Delete", ui::icons::Delete);
                    ui::EndMenu();
                }
            }
            else if (tab == 2)
            {
                ui::Headline("Progress");
                ui::ProgressBar(volume);
                ui::BeginFlow("rings", {.spacing = 18});
                ui::ProgressRing(0.25f);
                ui::ProgressRing(volume, {.tint = Color::Hex(0x30D158), .glow = true});
                ui::ProgressRing(0.8f, {.tint = Color::Hex(0xFF375F), .diameter = 34, .thickness = 6});
                ui::ActivityIndicator();
                ui::EndFlow();
                ui::Spacer();
                ui::Headline("Badges");
                ui::BeginFlow("badges");
                ui::Badge("3");
                ui::Badge("New", Color::Hex(0x0A84FF));
                ui::Badge("Beta", Color::Hex(0xBF5AF2));
                ui::Badge("Live", Color::Hex(0xFF375F));
                ui::EndFlow();
            }
            else
            {
                ui::Headline("Appearance");
                ui::BeginHStack("dark", {.align = ui::Align::Center});
                ui::Text(ui::TextStyle::Body, "Dark mode");
                ui::FlexSpacer();
                if (ui::Toggle("darkmode", &dark))
                    ApplyGlass();
                ui::EndStack();
                if (ui::ColorSwatches("accent", &accent, kAccents, 5))
                    ui->SetAccent(kAccents[accent]);
                ui::Spacer();
                ui::Headline("Glass");
                if (ui::Segmented("look", &glassLook, {"Theme", "Clear", "Frosted"}))
                    ui->SetGlassLook((ui::GlassLook)glassLook);
            }
            ui::TabBar("tabs", &tab, {{ui::icons::Apps, "Controls"}, {ui::icons::Edit, "Inputs"}, {ui::icons::Diagnostic, "Status"}, {ui::icons::Palette, "Style"}});
            ui::EndWindow();
        }

        void ControlCenter()
        {
            ui::WindowOptions wo;
            wo.size = Vec2(400, 360);
            wo.pos = Vec2(500, 40);
            wo.flags = ui::WindowFlags_NoScroll;
            if (!ui::BeginWindow("Control Center", &showControl, wo))
                return;
            ui::BeginGrid("tiles", {.minColumnWidth = 150, .maxColumns = 4, .spacing = 12});
            ui::BeginCard("connect", {}, {.glass = true, .radius = 22});
            ui::BeginGrid("toggles", {.columns = 2, .spacing = 10, .align = ui::Align::Center});
            ui::ToggleButton("wifi", &wifi, ui::icons::Wifi, {.diameter = 52});
            ui::ToggleButton("bt", &bluetooth, ui::icons::Bluetooth, {.diameter = 52});
            ui::ToggleButton("air", &airplane, ui::icons::Airplane, {.tint = Color::Hex(0xFF9F0A), .diameter = 52});
            ui::ToggleButton("focus", &focus, ui::icons::Moon, {.tint = Color::Hex(0x5E5CE6), .diameter = 52});
            ui::EndGrid();
            ui::EndCard();
            ui::BeginCard("now", {}, {.glass = true, .radius = 22});
            ui::Text(ui::TextStyle::Headline, "Now Playing");
            ui::TextSecondary("Liquid Glass - Esia");
            ui::Spacer(6);
            ui::BeginHStack("transport", {.justify = ui::Align::Center});
            ui::IconButton("prev", ui::icons::Previous, {.kind = ui::ButtonKind::Plain, .size = ui::ControlSize::Small});
            ui::IconButton("pp", ui::icons::Pause, {.kind = ui::ButtonKind::Plain, .size = ui::ControlSize::Small});
            ui::IconButton("next", ui::icons::Next, {.kind = ui::ButtonKind::Plain, .size = ui::ControlSize::Small});
            ui::EndStack();
            ui::EndCard();
            ui::LayoutSpan(2);
            ui::BeginCard("brightness", {}, {.glass = true, .radius = 22});
            ui::Slider("bright", &brightness, 0.0f, 1.0f, {.minIcon = ui::icons::Brightness, .maxIcon = ui::icons::Brightness});
            ui::EndCard();
            ui::EndGrid();
            ui::EndWindow();
        }

        void Telemetry()
        {
            ui::WindowOptions wo;
            wo.size = Vec2(400, 300);
            wo.pos = Vec2(500, 420);
            if (!ui::BeginWindow("Telemetry", &showTelemetry, wo))
                return;
            const float t = (float)ui::Time();
            const float cpu = 0.5f + 0.35f * std::sin(t * 0.7f), gpu = 0.45f + 0.4f * std::sin(t * 0.43f + 1.0f);
            ui::BeginFlow("gauges", {.spacing = 28});
            for (int i = 0; i < 2; ++i)
            {
                ui::BeginHStack(i == 0 ? "cpu" : "gpu", {.spacing = 12, .align = ui::Align::Center});
                ui::ProgressRing(i == 0 ? cpu : gpu, {.tint = i == 0 ? Color::Hex(0x30D158) : Color::Hex(0x0A84FF), .diameter = 64, .thickness = 8});
                ui::BeginVStack(i == 0 ? "cpu-t" : "gpu-t", {.spacing = 2, .align = ui::Align::Start});
                ui::Text(ui::TextStyle::Title2, "%.0f%%", (i == 0 ? cpu : gpu) * 100.0f);
                ui::TextSecondary(i == 0 ? "CPU" : "GPU");
                ui::EndStack();
                ui::EndStack();
            }
            ui::EndFlow();
            ui::Spacer();
            // frame time, a ring buffer of the last 120 frames (a made-up signal: this window has no GPU timings)
            frameTimes[(std::size_t)frameCursor] = 8.0f + 3.0f * std::sin(t * 1.3f) + 1.5f * std::sin(t * 7.1f) + (frameCursor % 17 == 0 ? 4.0f : 0.0f);
            frameCursor = (frameCursor + 1) % (int)std::size(frameTimes);
            ui::Text(ui::TextStyle::Headline, "Frame time");
            ui::LineChart("frametime", frameTimes, {.height = 70, .tint = Color::Hex(0x30D158), .glow = true, .offset = frameCursor});
            ui::Spacer();
            ui::BeginCard("log");
            ui::Text(ui::TextStyle::Headline, "Event log");
            ui::TextWrapped(ui::TextStyle::Footnote, ui->GetTheme().colors.secondaryLabel,
                            "Frames render on the RTX 4080 SUPER; glass captures stay within budget. 中文也可以正常换行显示。");
            ui::EndCard();
            ui::EndWindow();
        }

        // The live theme with the Liquid Glass section's edits (WGT's ApplyGlass)
        void ApplyGlass()
        {
            const ui::Theme base = dark ? ui::ThemeDark() : ui::ThemeLight();
            ui::Theme t = base;
            t.materials.window.blur = frost;
            t.materials.window.refraction = lensing;
            t.materials.window.dispersion = dispersion;
            const float spec = specular ? 1.0f : 0.0f;
            t.materials.window.specular = base.materials.window.specular * spec;
            t.materials.bar.specular = base.materials.bar.specular * spec;
            t.materials.control.specular = base.materials.control.specular * spec;
            ui->SetTheme(t, false);
        }

        // WGT's Settings screen: a navigation stack of inset grouped lists (its search bar is not ported yet)
        void Settings()
        {
            ui::WindowOptions wo;
            wo.size = Vec2(390, 700);
            wo.pos = Vec2(930, 40);
            wo.icon = ui::icons::Settings;
            wo.flags = ui::WindowFlags_NoScroll;   // the pages scroll
            if (!ui::BeginWindow("Settings", &showSettings, wo))
                return;
            ui::BeginNavigation("settings", "root");
            if (!openPage.empty())
            {
                ui::NavigationPush(openPage);
                openPage.clear();
            }
            if (ui::BeginPage("root", "Settings"))
            {
                RootPage();
                ui::EndPage();
            }
            if (ui::BeginPage("accent", "Accent Color"))
            {
                ui::BeginSection("Choose a tint", "The accent animates across every control.");
                Rect content;
                if (ui::BeginRow("swatches", 64, &content))
                {
                    ui->GetContext().SetCursorPos(Vec2(content.min.x, content.Center().y - ui::S(17)));
                    if (ui::ColorSwatches("acc", &accent, kAccents, 5, 30))
                        ui->SetAccent(kAccents[accent]);
                }
                ui::EndRow();
                ui::EndSection();
                ui::BeginSection("Preview");
                ui::RowToggle("Switch", &previewOn);
                ui::RowSlider("Slider", &previewValue, 0.0f, 1.0f, {}, nullptr);
                ui::RowButton("Button");
                ui::EndSection();
                ui::EndPage();
            }
            ui::EndNavigation();
            ui::EndWindow();
        }

        void RootPage()
        {
            const ui::Theme& t = ui->GetTheme();
            // a row of its own: the profile
            ui::BeginSection();
            Rect content;
            if (ui::BeginRow("profile", 76, &content))
            {
                esia::Painter p = ui::GetPainter();
                const Vec2 c(content.min.x + ui::S(28), content.Center().y);
                p.Circle(c, ui::S(28), Style().Fill(Paint::Linear(Color::Hex(0xBF5AF2), Color::Hex(0x0A84FF), 45)).Shadow(t.colors.shadow, ui::S(8), Vec2(0, ui::S(3))));
                p.TextBox(Rect::FromCenter(c, Vec2(ui::S(56), ui::S(56))), Vec2(0.5f, 0.5f), ui->Font(ui::TextStyle::Title2), Color::White(), "E");
                // the text ends before the chevron and is cut with an ellipsis: nothing overlaps at any width
                const float tx = content.min.x + ui::S(70), tr = content.max.x - ui::S(22);
                p.TextBox(Rect(tx, content.Center().y - ui::S(20), tr, content.Center().y), Vec2(0, 0), ui->Font(ui::TextStyle::Headline), t.colors.label,
                          "Esia Player", esia::text::TextFlags_Ellipsis);
                p.TextBox(Rect(tx, content.Center().y + ui::S(2), tr, content.Center().y + ui::S(20)), Vec2(0, 0), ui->Font(ui::TextStyle::Footnote),
                          t.colors.secondaryLabel, "Account, Cloud Saves & Achievements", esia::text::TextFlags_Ellipsis);
                p.Icon(Vec2(content.max.x - ui::S(6), content.Center().y), ui->IconFont(ui::S(11)), ui::icons::ChevronRight, t.colors.tertiaryLabel);
            }
            ui::EndRow();
            ui::EndSection();

            ui::BeginSection("Appearance");
            if (ui::RowToggle("Dark Mode", &dark, {ui::icons::Moon, Color::Hex(0x5E5CE6)}))
                ApplyGlass();
            if (ui::RowNavigation("Accent Color", {}, {ui::icons::Palette, kAccents[accent]}))
                ui::NavigationPush("accent");
            ui::RowSegmented("Text Size", &textSize, {"S", "M", "L"}, {ui::icons::ZoomIn, Color::Hex(0x0A84FF)});
            ui::EndSection();

            ui::BeginSection("Display", "The numbers are this window's; the frame limit is not wired to the host yet.");
            ui::RowToggle("VSync", &vsync, {ui::icons::FullScreen, Color::Hex(0x0A84FF)});
            ui::RowSegmented("Frame Limit", &frameLimit, {"Off", "60", "120", "144"}, {ui::icons::Lightning, Color::Hex(0xFF9F0A)});
            ui::RowPicker("Resolution", &resolution, {"1280 x 720", "1920 x 1080", "2560 x 1440", "3840 x 2160"}, {ui::icons::View, Color::Hex(0x5E5CE6)});
            char rate[64];
            const float dt = std::max(ui::DeltaTime(), 1e-4f);
            std::snprintf(rate, sizeof(rate), "%.0f fps  ·  %.2f ms", 1.0f / dt, dt * 1000.0f);
            ui::RowValue("Frame Rate", rate, {ui::icons::Diagnostic, Color::Hex(0x30D158)});
            ui::EndSection();

            ui::BeginSection("Liquid Glass", "Edits the live theme: every glass surface re-renders with the new material.");
            bool edited = false;
            edited |= ui::RowSlider("Frost", &frost, 0.0f, 48.0f, {ui::icons::Cloud, Color::Hex(0x64D2FF)}, "%.0f");
            edited |= ui::RowSlider("Lensing", &lensing, 0.0f, 40.0f, {ui::icons::View, Color::Hex(0x30D158)}, "%.0f");
            edited |= ui::RowSlider("Dispersion", &dispersion, 0.0f, 1.0f, {ui::icons::Brightness, Color::Hex(0xFF9F0A)}, "%.2f");
            edited |= ui::RowToggle("Specular Rim", &specular, {ui::icons::Lightbulb, Color::Hex(0xFFD60A)});
            if (ui::RowSegmented("Glass", &glassLook, {"Theme", "Clear", "Frosted"}, {ui::icons::View, Color::Hex(0x0A84FF)}))
                ui->SetGlassLook((ui::GlassLook)glassLook);
            if (edited)
                ApplyGlass();
            ui::EndSection();

            ui::BeginSection("Notifications");
            if (ui::RowButton("Send Test Notification"))
                ++presses;
            ui::RowToggle("Background Download", &download, {ui::icons::Download, Color::Hex(0x30D158)});
            ui::RowStepper("Badge Count", &count, 0, 10, {ui::icons::Bell, Color::Hex(0xFF375F)});
            ui::EndSection();

            ui::BeginSection("About");
            ui::RowValue("Version", "Esia UI (first part)", {ui::icons::Info, Color::Hex(0x8E8E93)});
            ui::RowValue("Renderer", renderer, {ui::icons::Game, Color::Hex(0xFF375F)});
            ui::RowButton("Reset All Settings", true);
            ui::EndSection();
            // the floating Liquid Glass search field: the list scrolls under it (iOS Settings)
            ui::SearchBar("settings_search", &settingsQuery);
        }
    };
}

int main(int argc, char** argv)
{
    Showcase s;
    glass::App app;
    app.name = "showcase";
    app.loadFonts = false;   // the Ui loads the platform's UI font, its weights, the CJK chain and the icon font
    app.option = [&](const std::string& o, const char* value, bool& usedValue) {
        if (o == "--dark")
            return s.dark = true;
        if (o == "--tab" && value)
        {
            usedValue = true;
            s.tab = std::clamp(std::atoi(value), 0, 3);
            return true;
        }
        if (o == "--menu")
            return s.openMenu = true;
        if (o == "--page" && value)
        {
            usedValue = true;
            s.openPage = value;
            return true;
        }
        if (o == "--look" && value)
        {
            usedValue = true;
            s.look = std::strcmp(value, "clear") == 0 ? ui::GlassLook::Clear : std::strcmp(value, "theme") == 0 ? ui::GlassLook::Theme : ui::GlassLook::Frosted;
            return true;
        }
        return false;
    };
    app.init = [&](esia::Context& ctx, esia::text::TextSystem* text, esia::text::FontId, const std::vector<std::string>& fonts) {
        ui::UiDesc d;
        d.text = text;
        if (!fonts.empty())
            d.fontFiles[0] = fonts[0];
        d.theme = s.dark ? ui::ThemeDark() : ui::ThemeLight();
        d.glassLook = s.look;
        s.glassLook = (int)s.look;
        s.ui = std::make_unique<ui::Ui>(ctx, d);
    };
    app.frame = [&](esia::Context& ctx, const glass::SceneInfo& info) {
        s.renderer = std::string(info.api) + "  ·  " + info.adapter;
        s.ui->NewFrame();
        s.Wallpaper(ctx);
        s.Components();
        s.ControlCenter();
        s.Telemetry();
        s.Settings();
        s.ui->EndFrame();
    };
    app.shutdown = [&] { s.ui.reset(); };
    return glass::RunApp(argc, argv, app);
}
