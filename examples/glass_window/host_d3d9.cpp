// glass_window - Direct3D 9Ex host: a windowed device (its implicit swap chain, one back buffer) and Present.
#include "host.hpp"
#include "esia/rhi/d3d9.hpp"
#include <d3d9.h>
#include <wrl/client.h>
#include <cstdio>

namespace glass
{
    namespace
    {
        using Microsoft::WRL::ComPtr;

        class HostD3D9 final : public Host
        {
        public:
            const char* Name() const override { return "Direct3D 9Ex"; }

            bool Init(HWND hwnd, int width, int height, const HostOptions& o, std::string& error) override
            {
                vsync_ = o.vsync;
                HRESULT hr = ::Direct3DCreate9Ex(D3D_SDK_VERSION, &d3d_);
                if (FAILED(hr))
                {
                    error = "Direct3DCreate9Ex failed (Direct3D 9Ex needs Windows Vista or later)";
                    return false;
                }
                D3DADAPTER_IDENTIFIER9 id = {};
                if (SUCCEEDED(d3d_->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &id)))
                    adapter_ = id.Description;
                pp_.Windowed = TRUE;
                pp_.SwapEffect = D3DSWAPEFFECT_DISCARD;
                pp_.BackBufferFormat = D3DFMT_X8R8G8B8;
                pp_.BackBufferCount = 1;
                pp_.BackBufferWidth = (UINT)width;
                pp_.BackBufferHeight = (UINT)height;
                pp_.hDeviceWindow = hwnd;
                pp_.PresentationInterval = o.vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
                // NOWINDOWCHANGES: the window belongs to the window thread, D3D9 must not hook or restyle it;
                // FPU_PRESERVE: D3D9 would otherwise put the thread's x87 unit in single precision
                const DWORD flags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_NOWINDOWCHANGES | D3DCREATE_FPU_PRESERVE;
                hr = d3d_->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, flags, &pp_, nullptr, &device_);
                if (FAILED(hr))
                {
                    char buf[64];
                    std::snprintf(buf, sizeof(buf), "CreateDeviceEx failed: 0x%08lX", (unsigned long)hr);
                    error = buf;
                    return false;
                }
                esia::rhi::d3d9::Desc d;
                d.device = device_.Get();
                d.debug.debugLayer = o.debug;   // D3D9 has no info queue: failed calls only get more verbose
                esia_ = esia::rhi::d3d9::CreateDevice(d, &error);
                return esia_ && CreateTarget();
            }

            esia::rhi::Device& Device() override { return *esia_; }

            void Resize(int width, int height) override
            {
                // the wrapped target holds the back buffer; with 9Ex every other resource survives ResetEx
                esia_->DestroyTexture(target_);
                target_ = {};
                pp_.BackBufferWidth = (UINT)width;
                pp_.BackBufferHeight = (UINT)height;
                if (SUCCEEDED(device_->ResetEx(&pp_, nullptr)))
                    CreateTarget();
            }

            bool BeginFrame() override { return (bool)target_; }

        protected:
            void Present() override { device_->Present(nullptr, nullptr, nullptr, nullptr); }

        private:
            bool CreateTarget()
            {
                ComPtr<IDirect3DSurface9> back;
                if (FAILED(device_->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back)))
                    return false;
                target_ = esia::rhi::d3d9::WrapRenderTarget(*esia_, back.Get());
                return (bool)target_;
            }

            ComPtr<IDirect3D9Ex> d3d_;
            ComPtr<IDirect3DDevice9Ex> device_;
            D3DPRESENT_PARAMETERS pp_ = {};
            std::unique_ptr<esia::rhi::Device> esia_;
        };
    }

    std::unique_ptr<Host> CreateHostD3D9() { return std::make_unique<HostD3D9>(); }
}
