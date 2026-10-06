// A program built against an installed Esia (find_package): the core, the widgets, the FreeType text system and the
// backends link, and frames of a window with a button render on the null device.
#include "esia/core/context.hpp"
#include "esia/render/renderer.hpp"
#include "esia/rhi/backend_registry.hpp"
#include "esia/ui/ui.hpp"
#if defined(ESIA_PACKAGE_TEXT_FT)
#include "esia/text/freetype.hpp"
#endif
#include <cstdio>
#include <memory>
#include <string>

int main()
{
    namespace ui = esia::ui;
    esia::rhi::RegisterBuiltinBackends();
    const esia::rhi::BackendInfo* null = esia::rhi::FindBackend("null");
    std::string error;
    esia::rhi::HeadlessDevice hd = null ? null->createHeadless({}, error) : esia::rhi::HeadlessDevice{};
    if (!hd.device)
    {
        std::fprintf(stderr, "esia_package_user: no null device (%s)\n", error.c_str());
        return 1;
    }

    esia::Context ctx;
    std::unique_ptr<esia::text::TextSystem> text;
#if defined(ESIA_PACKAGE_TEXT_FT)
    text = esia::text::CreateFreeTypeTextSystem(ctx.Textures());
#endif
    ui::UiDesc desc;
    desc.text = text.get();
    desc.density = ui::Density::Compact;
    ui::Ui u(ctx, desc);
    esia::render::Renderer renderer(*hd.device);
    int pressed = 0;
    for (int frame = 0; frame < 3; ++frame)
    {
        ctx.NewFrame({{256, 256}, {1, 1}, 1.0 + frame / 60.0});
        u.NewFrame();
        if (ui::BeginWindow("Package"))
        {
            if (ui::Button("OK"))
                ++pressed;
            ui::EndWindow();
        }
        u.EndFrame();
        ctx.EndFrame();
        if (!renderer.Render(ctx.GetDrawData(), &ctx.Textures(), hd.target))
        {
            std::fprintf(stderr, "esia_package_user: the frame was refused\n");
            return 1;
        }
    }
    std::printf("esia_package_user: %d backends, 3 frames rendered (%d draws)\n", (int)esia::rhi::Backends().size(),
                renderer.Stats().drawCalls);
    return pressed == 0 ? 0 : 1;
}
