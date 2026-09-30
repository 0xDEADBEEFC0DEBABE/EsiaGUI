// showcase - the widget layer (esia::ui) in a window: WGT's showcase, screen by screen as the widgets are ported.
//
//   showcase.exe [app options, see ../glass_window/app.hpp] [--dark] [--look theme|clear|frosted]
#include "app.hpp"
#include "esia/render/painter.hpp"
#include "esia/ui/ui.hpp"
#include <cmath>
#include <cstring>
#include <memory>

namespace ui = esia::ui;
using esia::Color;
using esia::Paint;
using esia::Rect;
using esia::Style;
using esia::Vec2;

namespace
{
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
        bool showComponents = true, showControl = true, showTelemetry = true;

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
            ui::BeginHStack("stepper", {.align = ui::Align::Center});
            ui::Stepper("count", &count, 0, 10);
            ui::Text(ui::TextStyle::Body, "Count: %d", count);
            ui::EndStack();

            ui::Spacer();
            ui::Headline("Progress");
            ui::ProgressBar(volume);
            ui::BeginFlow("rings", {.spacing = 18});
            ui::ProgressRing(0.25f);
            ui::ProgressRing(volume, {.tint = Color::Hex(0x30D158), .glow = true});
            ui::ProgressRing(0.8f, {.tint = Color::Hex(0xFF375F), .diameter = 34, .thickness = 6});
            ui::ActivityIndicator();
            ui::Badge("3");
            ui::Badge("New", Color::Hex(0x0A84FF));
            ui::Badge("Beta", Color::Hex(0xBF5AF2));
            ui::EndFlow();

            ui::Spacer();
            ui::Headline("Appearance");
            ui::BeginHStack("dark", {.align = ui::Align::Center});
            ui::Text(ui::TextStyle::Body, "Dark mode");
            ui::FlexSpacer();
            if (ui::Toggle("darkmode", &dark))
                ui->SetDarkMode(dark);
            ui::EndStack();
            static const Color kAccents[] = {Color::Hex(0x0A84FF), Color::Hex(0x30D158), Color::Hex(0xFF9F0A), Color::Hex(0xFF375F), Color::Hex(0xBF5AF2)};
            if (ui::ColorSwatches("accent", &accent, kAccents, 5))
                ui->SetAccent(kAccents[accent]);
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
            ui::BeginCard("log");
            ui::Text(ui::TextStyle::Headline, "Event log");
            ui::TextWrapped(ui::TextStyle::Footnote, ui->GetTheme().colors.secondaryLabel,
                            "Frames render on the RTX 4080 SUPER; glass captures stay within budget. 中文也可以正常换行显示。");
            ui::EndCard();
            ui::EndWindow();
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
        s.ui = std::make_unique<ui::Ui>(ctx, d);
    };
    app.frame = [&](esia::Context& ctx, const glass::SceneInfo&) {
        s.ui->NewFrame();
        s.Wallpaper(ctx);
        s.Components();
        s.ControlCenter();
        s.Telemetry();
        s.ui->EndFrame();
    };
    app.shutdown = [&] { s.ui.reset(); };
    return glass::RunApp(argc, argv, app);
}
