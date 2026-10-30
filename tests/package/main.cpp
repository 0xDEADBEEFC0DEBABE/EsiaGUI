// A program built against an installed Esia (find_package): the core, the renderer, the FreeType text system, SVG, the
// backends and (where the package has it) the widget layer link, and frames of a window with a button render on the
// null device.
#include "esia/core/context.hpp"
#include "esia/render/renderer.hpp"
#include "esia/rhi/backend_registry.hpp"
#include "esia/svg/svg.hpp"
#if defined(ESIA_PACKAGE_UI)
#include "esia/ui/ui.hpp"
#endif
#if defined(ESIA_PACKAGE_TEXT_FT)
#include "esia/text/freetype.hpp"
#endif
#include <cstdio>
#include <memory>
#include <string>

int main()
{
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
#if defined(ESIA_PACKAGE_UI)
    namespace ui = esia::ui;
    ui::UiDesc desc;
    desc.text = text.get();
    desc.density = ui::Density::Compact;
    ui::Ui u(ctx, desc);
#endif
    esia::render::Renderer renderer(*hd.device);
    int pressed = 0;
    for (int frame = 0; frame < 3; ++frame)
    {
        ctx.NewFrame({{256, 256}, {1, 1}, 1.0 + frame / 60.0});
#if defined(ESIA_PACKAGE_UI)
        u.NewFrame();
        if (ui::BeginWindow("Package"))
        {
            if (ui::Button("OK"))
                ++pressed;
            ui::EndWindow();
        }
        u.EndFrame();
#else
        // the core alone: a window with a button drawn as a rectangle
        if (ctx.Begin("Package"))
        {
            const esia::Rect r = esia::Rect::FromSize(ctx.CursorPos(), esia::Vec2(80, 24));
            ctx.ItemSize(r.Size());
            const esia::Id id = ctx.GetId("OK");
            if (ctx.ItemAdd(id, r) && ctx.ButtonBehavior(id, r).pressed)
                ++pressed;
            ctx.WindowDrawList().AddRectFilled(r, 0xFF3A84FFu);
        }
        ctx.End();
#endif
        ctx.EndFrame();
        if (!renderer.Render(ctx.GetDrawData(), &ctx.Textures(), hd.target))
        {
            std::fprintf(stderr, "esia_package_user: the frame was refused\n");
            return 1;
        }
    }
    // SVG: a file parsed and rasterized
    const esia::svg::Document icon = esia::svg::Document::Parse("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 8 8'><rect width='4' height='8'/></svg>");
    esia::svg::Image image;
    if (!esia::svg::Rasterize(icon, 8, 8, {}, image) || image.pixels[3] != 255 || image.pixels[7 * 4 + 3] != 0)
    {
        std::fprintf(stderr, "esia_package_user: the SVG did not rasterize\n");
        return 1;
    }
    std::printf("esia_package_user: %d backends, 3 frames rendered (%d draws)\n", (int)esia::rhi::Backends().size(),
                renderer.Stats().drawCalls);
    return pressed == 0 ? 0 : 1;
}
