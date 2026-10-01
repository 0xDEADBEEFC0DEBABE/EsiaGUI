// glass_window - DXGI adapter and flip-model swap chain (dxgi_swap_chain.hpp).
#include "dxgi_swap_chain.hpp"
#include <cstdio>

namespace glass
{
    std::string HrText(HRESULT hr)
    {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "0x%08lX", (unsigned long)hr);
        return buf;
    }

    bool PickAdapter(DxgiAdapter& out, std::string& error)
    {
        HRESULT hr = ::CreateDXGIFactory1(IID_PPV_ARGS(&out.factory));
        if (FAILED(hr))
        {
            error = "CreateDXGIFactory1 failed: " + HrText(hr);
            return false;
        }
        ComPtr<IDXGIFactory6> f6;
        const bool byPreference = SUCCEEDED(out.factory.As(&f6));   // Windows 10 1803+
        for (UINT i = 0;; ++i)
        {
            ComPtr<IDXGIAdapter1> a;
            hr = byPreference ? f6->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&a))
                              : out.factory->EnumAdapters1(i, &a);
            if (FAILED(hr))
                break;
            DXGI_ADAPTER_DESC1 d = {};
            a->GetDesc1(&d);
            if (d.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                continue;
            char name[128] = {};
            ::WideCharToMultiByte(CP_UTF8, 0, d.Description, -1, name, sizeof(name), nullptr, nullptr);
            out.adapter = a;
            out.name = name;
            return true;
        }
        return true;   // no hardware adapter: the device's own default (WARP, Wine's) is tried
    }

    bool FlipSwapChain::Create(IDXGIFactory2* factory, IUnknown* device, HWND hwnd, int width, int height, UINT buffers, DXGI_FORMAT format,
                               std::string& error)
    {
        ComPtr<IDXGIFactory5> f5;
        BOOL allowTearing = FALSE;
        if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&f5))))
            tearing_ = SUCCEEDED(f5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))) && allowTearing;

        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.Width = (UINT)width;
        sd.Height = (UINT)height;
        sd.Format = format;
        sd.SampleDesc.Count = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
        sd.BufferCount = buffers;
        sd.Scaling = DXGI_SCALING_STRETCH;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        sd.Flags = tearing_ ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        const HRESULT hr = factory->CreateSwapChainForHwnd(device, hwnd, &sd, nullptr, nullptr, &swap_);
        if (FAILED(hr))
        {
            error = "CreateSwapChainForHwnd (flip model) failed: " + HrText(hr);
            return false;
        }
        // DXGI's Alt+Enter would switch to exclusive fullscreen through the window thread behind the render thread's back
        factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
        return true;
    }

    bool FlipSwapChain::Resize(int width, int height)
    {
        return SUCCEEDED(swap_->ResizeBuffers(0, (UINT)width, (UINT)height, DXGI_FORMAT_UNKNOWN, tearing_ ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0));
    }

    void FlipSwapChain::Present(bool vsync)
    {
        // vsync off means uncapped: tearing (variable refresh) where the display path allows it
        swap_->Present(vsync ? 1 : 0, (!vsync && tearing_) ? DXGI_PRESENT_ALLOW_TEARING : 0);
    }
}
