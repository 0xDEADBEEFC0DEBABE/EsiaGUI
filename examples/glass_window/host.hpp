// glass_window - one graphics API's side of the example: its device, and the swap chain or surface of the window.
//
// A host runs on the render thread only. Per frame: BeginFrame (wait for a free buffer, wrap it as the RHI target),
// Renderer::Render into Target() with Frame() as RenderParams::frame, then EndFrame (submit, optionally read the image
// back, present).
// The scene paints every pixel (its wallpaper is opaque), so no host clears its target.
#pragma once
#include "esia/rhi/rhi.hpp"
#if defined(_WIN32)
#include <windows.h>
#endif
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace glass
{
    // The window a host presents to: a Win32 window, or on Linux an X11 window (app_linux.cpp).
    struct NativeWindow
    {
#if defined(_WIN32)
        HWND hwnd = nullptr;
#else
        void* display = nullptr;    // Display*
        unsigned long window = 0;   // Window
#endif
    };

    struct HostOptions
    {
        bool vsync = true;
        bool debug = false;   // the API's debug / validation layer, messages to stderr
    };

    class Host
    {
    public:
        virtual ~Host() = default;
        virtual const char* Name() const = 0;
        const std::string& Adapter() const { return adapter_; }   // the GPU, when the API names it
        // `width` x `height`: the client area in pixels.
        virtual bool Init(const NativeWindow& window, int width, int height, const HostOptions& options, std::string& error) = 0;
        virtual esia::rhi::Device& Device() = 0;
        // The window's client area changed size (pixels, never 0).
        virtual void Resize(int width, int height) = 0;
        // Waits for a free buffer and wraps it; false: skip this frame (swap chain being recreated).
        virtual bool BeginFrame() = 0;
        esia::rhi::Texture Target() const { return target_; }
        virtual esia::rhi::FrameDesc Frame() { return {}; }
        // Submits the frame's GPU work; `capture` (optional) receives the image as RGBA8 rows, then it is presented.
        bool EndFrame(std::vector<std::uint8_t>* capture)
        {
            Submit();
            bool ok = true;
            if (capture)
            {
                const esia::rhi::TextureDesc d = Device().GetTextureDesc(target_);
                ok = Device().ReadPixels(target_, {0, 0, d.width, d.height}, *capture);
            }
            Present();
            return ok;
        }
        // Messages of the API's debug layer counted by the backend or the host.
        virtual std::uint32_t ValidationMessages() { return Device().ValidationErrors(); }

    protected:
        virtual void Submit() {}
        virtual void Present() = 0;
        esia::rhi::Texture target_;
        bool vsync_ = true;
        std::string adapter_;
    };

    // Null when the API was not built into the example (ESIA_BACKEND_<API>).
    std::unique_ptr<Host> CreateHost(const std::string& api);
    // The --api names built in, e.g. "directx, d3d11, d3d12, d3d10, d3d9, opengl"; the default is the first.
    std::string BuiltApis();
    const char* DefaultApi();
    // "directx" stands for the Direct3D versions built in, in the order it tries them: 11 (every GPU of the last
    // fifteen years, the most used), 12, 10, 9 (the oldest GPUs). Empty without DirectX.
    std::vector<std::string> DirectXApis();

    std::unique_ptr<Host> CreateHostD3D9();
    std::unique_ptr<Host> CreateHostD3D10();
    std::unique_ptr<Host> CreateHostD3D11();
    std::unique_ptr<Host> CreateHostD3D12();
    std::unique_ptr<Host> CreateHostOpenGL();
    std::unique_ptr<Host> CreateHostVulkan();
}
