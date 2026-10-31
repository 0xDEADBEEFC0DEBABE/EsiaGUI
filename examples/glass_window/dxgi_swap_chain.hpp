// glass_window - what the Direct3D 10 / 11 / 12 hosts share: the adapter and a flip-model swap chain.
#pragma once
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <string>

namespace glass
{
    using Microsoft::WRL::ComPtr;

    // The DXGI factory and the high-performance adapter (the discrete GPU of a laptop); null adapter: none found.
    struct DxgiAdapter
    {
        ComPtr<IDXGIFactory2> factory;
        ComPtr<IDXGIAdapter1> adapter;
        std::string name;
    };
    bool PickAdapter(DxgiAdapter& out, std::string& error);

    // DXGI_SWAP_EFFECT_FLIP_DISCARD, buffers usable as render targets and shader inputs (frosted glass then reads the
    // back buffer without a copy), tearing allowed when the display path supports it (vsync off).
    class FlipSwapChain
    {
    public:
        FlipSwapChain() = default;
        FlipSwapChain(const FlipSwapChain&) = delete;
        FlipSwapChain& operator=(const FlipSwapChain&) = delete;
        ~FlipSwapChain();
        // `device`: the ID3D10Device / ID3D11Device, or the ID3D12CommandQueue that presents. `format`: B8G8R8A8_UNORM
        // (what the desktop composes), except for a Direct3D 10.0 device, which Windows refuses a B8G8R8A8 swap chain
        // (E_INVALIDARG on every adapter; Wine accepts it): R8G8B8A8_UNORM there. `waitable`: a frame latency of one
        // and its waitable object (WaitForBuffer), where DXGI has them (Windows 8.1+).
        bool Create(IDXGIFactory2* factory, IUnknown* device, HWND hwnd, int width, int height, UINT buffers, DXGI_FORMAT format,
                    std::string& error, bool waitable = false);
        // Every reference to the buffers must be released first (wrapped RHI targets included).
        bool Resize(int width, int height);
        // An R16G16B16A16_FLOAT swap chain's colour space: scRGB (linear BT.709, 1.0 = 80 nits) - what the desktop
        // composes it as on an HDR monitor, and tone-maps to SDR elsewhere. False where DXGI refuses it.
        bool SetScRgb();
        // A waitable swap chain: blocks until the frame presented last has gone to the screen, so that the buffer to draw
        // into is free. Without it Direct3D 10 / 11 make the GPU wait for that buffer at the frame's first draw into it -
        // inside the frame's GPU timestamps, most of a refresh with vsync. Returns at once when not waitable.
        void WaitForBuffer();
        void Present(bool vsync);
        IDXGISwapChain1* Get() const { return swap_.Get(); }

    private:
        ComPtr<IDXGISwapChain1> swap_;
        HANDLE frameReady_ = nullptr;   // the frame latency waitable object
        UINT flags_ = 0;                // the creation flags, which ResizeBuffers wants again
        bool tearing_ = false;
    };

    std::string HrText(HRESULT hr);
}
