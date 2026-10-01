// glass_window - the frame the examples run in: options, the window thread, the render thread with its device,
// Context, text system and frame loop, screenshots. An example gives what it draws per frame (App).
//
//   <example>.exe [--api directx|d3d11|d3d12|d3d10|d3d9|opengl|vulkan] [--size 1280x800] [--scale 1.5] [--vsync on|off]
//                 [--debug] [--fixed-dt 0.016667] [--frames N] [--screenshot out.png] [--font file.ttf]...
//                 <the example's own>
//
//   --api           the backend; directx (the default where it is built) starts the first Direct3D version that works
//                   on this machine: 11, 12, 10, 9
//   --size          client area in UI units (pixels at 100 % scale; the window grows with the monitor's scale)
//   --scale s       pixels per UI unit instead of the monitor's (the client area is size x s pixels, which may be
//                   larger than the screen): captures at a given scale on any monitor. --size takes fractions, so a
//                   pixel size at a scale is `--size 1066.667x666.667 --scale 1.5` (1600 x 1000 pixels)
//   --frames N      quit after N frames; with --screenshot, the last one is read back from the swap chain image
//   --fixed-dt s    the UI clock advances s seconds per frame (deterministic screenshots)
//   --debug         the API's debug / validation layer; its message count is printed at exit (exit code 3 if any)
//   --no-shader-cache  the D3D backends compile every shader (by default a shader compiled once is kept in
//                   %LOCALAPPDATA%\Esia\ShaderCache and read back in later runs: esia::rhi::d3d::SetShaderCacheDirectory)
//   --font          font files, the first the main one, the others fallbacks (default: Segoe UI + Microsoft YaHei);
//                   for examples that load their own fonts, passed on to them
//
// Threads, as a game has them: the main thread creates the window and pumps its messages (window thread); a render
// thread owns the device, the Context and the frame loop. Input crosses over through Context::QueueInput, so moving
// or resizing the window never stalls rendering (esia/platform/win32.hpp, "Threading rules").
#pragma once
#include "esia/core/context.hpp"
#include "esia/text/text.hpp"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace esia::render
{
    class Renderer;
    struct RenderStats;
}

namespace glass
{
    struct SceneInfo
    {
        const char* api = "";
        std::string adapter;
        int width = 0, height = 0;   // pixels
        float scale = 1.0f;          // pixels per UI unit
        float fps = 0.0f;
        float cpuMs = 0.0f;          // the last frame's UI build (App::frame), milliseconds
        const esia::render::RenderStats* stats = nullptr;   // the last frame the renderer drew
    };

    struct App
    {
        const char* name = "glass_window";
        // true: the frame loads the fonts (--font, else Segoe UI + Microsoft YaHei) and calls TextSystem::NewFrame;
        // false: the example does both (esia::ui::Ui does)
        bool loadFonts = true;
        // Options the frame does not know ("--demo name"): true when consumed (then `value`, if not null, was too).
        std::function<bool(const std::string& option, const char* value, bool& usedValue)> option;
        // On the render thread, once the Context and the text system exist (text is null when built without it or
        // when no font loaded). `font` is the main font the frame loaded (0 with loadFonts = false); `fonts` are the
        // --font files.
        std::function<void(esia::Context& ctx, esia::text::TextSystem* text, esia::text::FontId font, const std::vector<std::string>& fonts)> init;
        // Once, after init, with the renderer (user effects: Renderer::SetEffectSource).
        std::function<void(esia::render::Renderer& renderer)> renderer;
        // Every frame, between Context::NewFrame and EndFrame.
        std::function<void(esia::Context& ctx, const SceneInfo& info)> frame;
        // Before the Context goes away.
        std::function<void()> shutdown;
    };

    int RunApp(int argc, char** argv, App& app);

#if defined(__ANDROID__)
    // Android: a wallpaper as straight-alpha RGBA8, at most `maxSide` pixels on its longer side - the APK's own
    // (assets/wallpaper.jpg, .png or .heic: the building machine's, as iOS's app bundle has the Mac's), else the system's
    // built-in one (WallpaperManager.getBuiltInDrawable: an app may not read the user's without a permission). False
    // when there is none. On the frame's thread, after RunApp started.
    bool Wallpaper(int maxSide, int& width, int& height, std::vector<std::uint8_t>& rgba);
#endif

    // macOS, iOS and Linux: a font path the frame's text system draws with the system's symbols - SF Symbols, the
    // desktop's icon theme - for the code points of esia/ui/icons.hpp (UiDesc::iconFontFile; Windows' icon fonts do not
    // exist there; symbol_text.hpp). On Windows a path that does not load.
    inline constexpr const char* kSystemSymbolsFont = "glass:sf-symbols";
}
