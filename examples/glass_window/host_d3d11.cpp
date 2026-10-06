// glass_window - Direct3D 11 host: a device on the high-performance adapter and a flip-model swap chain.
#include "host.hpp"
#include "dxgi_swap_chain.hpp"
#include "esia/rhi/d3d11.hpp"
#include <d3d11.h>

namespace glass
{
    namespace
    {
        class HostD3D11 final : public Host
        {
        public:
            const char* Name() const override { return "Direct3D 11"; }

            bool Init(const NativeWindow& window, int width, int height, const HostOptions& o, std::string& error) override
            {
                const HWND hwnd = window.hwnd;
                vsync_ = o.vsync;
                DxgiAdapter a;
                if (!PickAdapter(a, error))
                    return false;
                adapter_ = a.name;
                const D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
                const UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | (o.debug ? D3D11_CREATE_DEVICE_DEBUG : 0);
                HRESULT hr = ::D3D11CreateDevice(a.adapter.Get(), a.adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
                                                 levels, 2, D3D11_SDK_VERSION, &device_, nullptr, &context_);
                if (FAILED(hr))
                {
                    error = "D3D11CreateDevice failed: " + HrText(hr) + (o.debug ? " (is the debug layer installed? Graphics Tools)" : "");
                    return false;
                }
                if (!swap_.Create(a.factory.Get(), device_.Get(), hwnd, width, height, 2, DXGI_FORMAT_B8G8R8A8_UNORM, error))
                    return false;
                esia::rhi::d3d11::Desc d;
                d.device = device_.Get();
                d.context = context_.Get();
                d.ownsContext = true;   // Esia draws everything here: no state of the host's to save, clear or put back
                d.debug.debugLayer = o.debug;
                esia_ = esia::rhi::d3d11::CreateDevice(d, &error);
                return esia_ && CreateTarget();
            }

            esia::rhi::Device& Device() override { return *esia_; }

            void Resize(int width, int height) override
            {
                // the wrapped target and the view hold the buffer: both go before ResizeBuffers
                esia_->DestroyTexture(target_);
                target_ = {};
                rtv_.Reset();
                context_->ClearState();
                context_->Flush();
                if (swap_.Resize(width, height))
                    CreateTarget();
            }

            bool BeginFrame() override { return (bool)target_; }

        protected:
            void Present() override { swap_.Present(vsync_); }

        private:
            bool CreateTarget()
            {
                // flip model: buffer 0 is always the one to draw into
                ComPtr<ID3D11Texture2D> buffer;
                if (FAILED(swap_.Get()->GetBuffer(0, IID_PPV_ARGS(&buffer))) || FAILED(device_->CreateRenderTargetView(buffer.Get(), nullptr, &rtv_)))
                    return false;
                target_ = esia::rhi::d3d11::WrapRenderTarget(*esia_, rtv_.Get());
                return (bool)target_;
            }

            ComPtr<ID3D11Device> device_;
            ComPtr<ID3D11DeviceContext> context_;
            FlipSwapChain swap_;
            ComPtr<ID3D11RenderTargetView> rtv_;
            std::unique_ptr<esia::rhi::Device> esia_;
        };
    }

    std::unique_ptr<Host> CreateHostD3D11() { return std::make_unique<HostD3D11>(); }
}
