// WGT example plugin: a separate DLL that adds a panel and a custom GPU effect to any WGT host.
//
// Build it against wgt.lib, drop the DLL into the host's plugin directory and call
// Context::LoadPluginsFromDirectory(). Everything here uses only public headers.
#include <wgt/wgt.hpp>

using namespace wgt;

namespace
{
    // Holographic foil: iridescent bands that drift with time and refract the backdrop.
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

    class HelloPlugin final : public IPlugin
    {
    public:
        const char* Name() const override { return "Hello Plugin"; }

        void OnAttach(Context& ctx) override
        {
            holo_ = ctx.RegisterEffect("holographic", kHoloHlsl);
            PanelDesc d;
            d.id = "plugin.hello";
            d.title = "Plugin: Hello";
            d.icon = icons::Code;
            d.iconColor = Color::Hex(0x5E5CE6);
            d.size = Vec2(360, 380);
            ctx.AddPanel(d, [this](Context& c) { Draw(c); });
        }

        void OnDetach(Context& ctx) override { ctx.RemovePanel("plugin.hello"); }

    private:
        void Draw(Context& ctx)
        {
            const Theme& t = ctx.GetTheme();
            ui::Text(TextStyle::Headline, "Loaded from %s", "wgt_plugin_hello.dll");
            ui::TextSecondary("This panel, its shader and its state live in a plugin DLL.");
            ui::Spacer(8);

            Painter p;
            const Vec2 pos = ImGui::GetCursorScreenPos();
            const float w = ImGui::GetContentRegionAvail().x;
            const Rect card = Rect::FromSize(pos, Vec2(w, ui::S(150)));
            p.Rect(card, Style().Radius(ui::S(20)).Effect(holo_, 1.0f, 0.6f).Shadow(Color::Black(0.3f), ui::S(18), Vec2(0, ui::S(8))));
            p.Text(card.min + Vec2(ui::S(18), ui::S(16)), TextStyle::Title2, Color::White(), "Holo Card");
            p.Text(card.min + Vec2(ui::S(18), ui::S(46)), TextStyle::Footnote, Color::White(0.8f), "Custom HLSL via Context::RegisterEffect");
            ImGui::Dummy(Vec2(w, ui::S(150)));
            ui::Spacer(10);

            ui::ButtonOptions b;
            b.kind = ui::ButtonKind::GlassProminent;
            b.icon = icons::Add;
            if (ui::Button("Tap me", b))
                ++taps_;
            ImGui::SameLine(0, ui::S(12));
            ui::Text(TextStyle::Body, "%d taps", taps_);
            (void)t;
        }

        EffectId holo_ = 0;
        int taps_ = 0;
    };
}

WGT_DECLARE_PLUGIN(HelloPlugin)
