// WGT demo - showcase panels
#pragma once
#include <wgt/wgt.hpp>
#include <atomic>

struct ShowcaseOptions
{
    ImTextureID wallpaper = ImTextureID_Invalid;
    int wallpaperWidth = 0;
    int wallpaperHeight = 0;
    const char* backendName = "";
    const char* adapterName = "";
    bool openAll = false;
    const char* openList = nullptr;   // comma separated panel ids to open (nullptr = defaults)
};

// Display settings edited in Settings > Display and applied by the render loop. Atomic: the render loop runs
// on its own thread, separate from the thread that pumps window messages.
struct ShowcaseDisplay
{
    std::atomic<bool> vsync{true};
    std::atomic<int> fpsLimit{0};        // 0 = unlimited
    std::atomic<float> fps{0.0f};        // measured by the render loop
    std::atomic<float> frameMs{0.0f};
    bool tearing = false;                // the display path allows uncapped presents with VSync off
    const char* threading = "";
};
ShowcaseDisplay& ShowcaseDisplaySettings();

void ShowcaseInit(wgt::Context* ctx, const ShowcaseOptions& options);
void ShowcaseBackground();   // the "game scene" behind the UI
void ShowcaseFrame();        // immediate-mode UI outside panels (HUD pill)
void ShowcaseShutdown();
