// glass_window - Direct3D 10 host: a 10.0 device on the high-performance adapter and a flip-model swap chain.
#include "host.hpp"
#include "dxgi_swap_chain.hpp"
#include "esia/rhi/d3d10.hpp"
#include <d3d10.h>

namespace glass
{
    namespace
    {
        class HostD3D10 final : public Host
        {
        public:
            const char* Name() const override { return "Direct3D 10"; }

            bool Init(HWND hwnd, int width, int height, const HostOptions& o, std::string& error) override
            {
                vsync_ = o.vsync;
                DxgiAdapter a;
                if (!PickAdapter(a, error))
                    return false;
                adapter_ = a.name;
                const UINT flags = D3D10_CREATE_DEVICE_BGRA_SUPPORT | (o.debug ? D3D10_CREATE_DEVICE_DEBUG : 0);
                HRESULT hr = ::D3D10CreateDevice(a.adapter.Get(), D3D10_DRIVER_TYPE_HARDWARE, nullptr, flags, D3D10_SDK_VERSION, &device_);
                if (FAILED(hr))
                {
                    error = "D3D10CreateDevice failed: " + HrText(hr) + (o.debug ? " (is the debug layer installed? Graphics Tools)" : "");
                    return false;
                }
                if (!swap_.Create(a.factory.Get(), device_.Get(), hwnd, width, height, 2, DXGI_FORMAT_R8G8B8A8_UNORM, error))
                    return false;
                esia::rhi::d3d10::Desc d;
                d.device = device_.Get();
                d.debug.debugLayer = o.debug;
                esia_ = esia::rhi::d3d10::CreateDevice(d, &error);
                return esia_ && CreateTarget();
            }

            esia::rhi::Device& Device() override { return *esia_; }

            void Resize(int width, int height) override
            {
                esia_->DestroyTexture(target_);   // it holds the view, the view holds the buffer
                target_ = {};
                rtv_.Reset();
                device_->ClearState();
                device_->Flush();
                if (swap_.Resize(width, height))
                    CreateTarget();
            }

            bool BeginFrame() override { return (bool)target_; }

        protected:
            void Present() override { swap_.Present(vsync_); }

        private:
            bool CreateTarget()
            {
                ComPtr<ID3D10Texture2D> buffer;   // flip model: buffer 0 is always the one to draw into
                if (FAILED(swap_.Get()->GetBuffer(0, IID_PPV_ARGS(&buffer))) || FAILED(device_->CreateRenderTargetView(buffer.Get(), nullptr, &rtv_)))
                    return false;
                target_ = esia::rhi::d3d10::WrapRenderTarget(*esia_, rtv_.Get());
                return (bool)target_;
            }

            ComPtr<ID3D10Device> device_;
            FlipSwapChain swap_;
            ComPtr<ID3D10RenderTargetView> rtv_;
            std::unique_ptr<esia::rhi::Device> esia_;
        };
    }

    std::unique_ptr<Host> CreateHostD3D10() { return std::make_unique<HostD3D10>(); }
}
