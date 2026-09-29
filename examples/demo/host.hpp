// WGT demo - minimal D3D11 / D3D12 hosts (window swap chain + frame loop) standing in for a game.
#pragma once
#include <wgt/wgt.hpp>
#include <windows.h>
#include <memory>

class IHost
{
public:
    virtual ~IHost() = default;
    virtual const char* Name() const = 0;
    virtual bool Init(HWND hwnd, int width, int height) = 0;
    // Fills the backend part of the context description.
    virtual void Describe(wgt::ContextDesc& desc) = 0;
    virtual void Resize(int width, int height) = 0;
    // Starts a frame: clears the back buffer (the "game scene").
    virtual void BeginFrame(const float clear[4]) = 0;
    virtual wgt::RenderTarget Target() = 0;
    virtual void EndFrame(bool vsync) = 0;
    // Saves the current back buffer (after EndFrame's rendering, before present) as PNG.
    virtual bool CaptureNextFrame(const wchar_t* path) = 0;
    // Blocks until the GPU is idle (before destroying the UI context).
    virtual void WaitIdle() = 0;
    // Appends debug-layer messages (if enabled) to `path`. Returns the number of warnings/errors.
    virtual int DumpDebugMessages(const wchar_t* path) = 0;
};

// Enables the D3D debug layer (must be set before Init).
void SetHostDebugLayer(bool enabled);
bool HostDebugLayer();
// GPU selection like a game: high-performance adapter by default, integrated GPU with lowPower.
void SetHostLowPower(bool lowPower);
struct IDXGIAdapter1;
IDXGIAdapter1* HostPickAdapter();   // AddRef'd, may be null (= system default)
const char* HostAdapterName();
// True when the display path supports tearing (variable refresh / uncapped presents with VSync off).
bool HostTearingSupported();
// Appends a line to wgt_demo.log (next to the executable's working directory).
void HostLog(const char* fmt, ...);

std::unique_ptr<IHost> CreateHostD3D11();
std::unique_ptr<IHost> CreateHostD3D12();
