// WGT UI - render backend interface
//
// DX11 and DX12 backends ship with wgt.dll. Implement IRenderBackend to port WGT to another API
// (Vulkan, a proprietary engine renderer ...) and pass it through ContextDesc::customBackend.
//
// A backend must:
//   1. honor Dear ImGui's texture protocol (ImDrawData::Textures, ImGuiBackendFlags_RendererHasTextures),
//   2. render regular ImDrawCmd geometry,
//   3. interpret fx::Command callbacks (fx::CommandCallback()) - shapes, glow layers, edge fades - see wgt/fx.hpp.
//      A backend that ignores FX commands still renders all text and ImGui geometry.
#pragma once
#include "context.hpp"

namespace wgt
{
    struct BackendFrameInfo
    {
        double time = 0.0;
        float deltaTime = 0.0f;
        int maxBackdropCaptures = 8;
        // Text composition (the system's DirectWrite rendering parameters, see TextEngine)
        float textGamma = 1.8f;
        float textGrayscaleContrast = 1.0f;
        float textClearTypeContrast = 0.5f;
        float textClearTypeLevel = 1.0f;
    };

    class IRenderBackend
    {
    public:
        virtual ~IRenderBackend() = default;
        virtual const char* Name() const = 0;

        // UI thread. Called once after the ImGui context exists (set io.BackendFlags here).
        virtual bool Init(ImGuiIO& io) = 0;
        // UI thread. Releases every GPU object, including ImGui-owned textures.
        virtual void Shutdown() = 0;

        // UI thread. Renders one frame of draw data into `target`.
        virtual void RenderDrawData(ImDrawData* drawData, const RenderTarget& target, const BackendFrameInfo& info) = 0;

        // Any thread. RGBA8 textures for images.
        virtual ImTextureID CreateTexture(const void* rgba8, int width, int height) = 0;
        virtual void DestroyTexture(ImTextureID texture) = 0;

        // Any thread. Custom FX effect sources (see Context::RegisterEffect). Optional.
        virtual void SetEffectSource(EffectId id, const char* name, const char* hlslSource) { (void)id; (void)name; (void)hlslSource; }

        // Any thread. Size of the render target in pixels (used for automatic render-scale detection).
        virtual bool QueryTargetSize(const RenderTarget& target, int& width, int& height) { (void)target; (void)width; (void)height; return false; }

        virtual void InvalidateDeviceObjects() {}
        virtual void CollectStats(FrameStats& stats) const { (void)stats; }
        virtual void SetLogCallback(void (*log)(void* user, int level, const char* message), void* user) { (void)log; (void)user; }
    };
}
