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

    // DXGI_SWAP_EFFECT_FLIP_DISCARD, B8G8R8A8_UNORM, buffers usable as render targets and shader inputs (frosted glass
    // then reads the back buffer without a copy), tearing allowed when the display path supports it (vsync off).
    class FlipSwapChain
    {
    public:
        // `device`: the ID3D10Device / ID3D11Device, or the ID3D12CommandQueue that presents.
        bool Create(IDXGIFactory2* factory, IUnknown* device, HWND hwnd, int width, int height, UINT buffers, std::string& error);
        // Every reference to the buffers must be released first (wrapped RHI targets included).
        bool Resize(int width, int height);
        void Present(bool vsync);
        IDXGISwapChain1* Get() const { return swap_.Get(); }

    private:
        ComPtr<IDXGISwapChain1> swap_;
        bool tearing_ = false;
    };

    std::string HrText(HRESULT hr);
}
