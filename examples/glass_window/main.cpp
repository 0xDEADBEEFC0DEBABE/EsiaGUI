// glass_window - Esia in a real window: the Win32 platform layer, one of the backends built in, liquid glass, and the
// core's windows. A smoke test of the whole path (messages -> input -> core -> renderer -> RHI -> swap chain), not
// the demo. Options and threads: app.hpp.
#include "app.hpp"
#include "scene.hpp"
#include <memory>

int main(int argc, char** argv)
{
    std::unique_ptr<glass::Scene> scene;
    glass::App app;
    app.name = "glass_window";
    app.init = [&](esia::Context&, esia::text::TextSystem* text, esia::text::FontId font, const std::vector<std::string>&) {
        scene = std::make_unique<glass::Scene>(text, font);
    };
    app.frame = [&](esia::Context& ctx, const glass::SceneInfo& info) { scene->Frame(ctx, info); };
    return glass::RunApp(argc, argv, app);
}
