// WGT demo - Direct3D 11 host
#include "host.hpp"
#include "image_io.hpp"
#include <d3d11.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <cstdarg>
#include <cstdio>
#include <string>

using Microsoft::WRL::ComPtr;

static bool g_debugLayer = false;
static bool g_lowPower = false;
static char g_adapterName[128] = "default adapter";
void SetHostDebugLayer(bool enabled) { g_debugLayer = enabled; }
bool HostDebugLayer() { return g_debugLayer; }
void SetHostLowPower(bool lowPower) { g_lowPower = lowPower; }
const char* HostAdapterName() { return g_adapterName; }

void HostLog(const char* fmt, ...)
{
    char buf[512];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    FILE* f = nullptr;
    if (fopen_s(&f, "wgt_demo.log", "a") == 0 && f)
    {
        std::fprintf(f, "%s\n", buf);
        std::fclose(f);
    }
}

bool HostTearingSupported()
{
    ComPtr<IDXGIFactory5> f5;
    BOOL allow = FALSE;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&f5))))
        f5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow, sizeof(allow));
    return allow != FALSE;
}

IDXGIAdapter1* HostPickAdapter()
{
    ComPtr<IDXGIFactory6> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
        return nullptr;
    ComPtr<IDXGIAdapter1> adapter;
    const DXGI_GPU_PREFERENCE pref = g_lowPower ? DXGI_GPU_PREFERENCE_MINIMUM_POWER : DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
    for (UINT i = 0; SUCCEEDED(factory->EnumAdapterByGpuPreference(i, pref, IID_PPV_ARGS(&adapter))); ++i)
    {
        DXGI_ADAPTER_DESC1 d;
        adapter->GetDesc1(&d);
        if (d.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue;
        WideCharToMultiByte(CP_UTF8, 0, d.Description, -1, g_adapterName, sizeof(g_adapterName), nullptr, nullptr);
        return adapter.Detach();
    }
    return nullptr;
}

namespace
{
    class HostD3D11 final : public IHost
    {
    public:
        const char* Name() const override { return "Direct3D 11"; }

        bool Init(HWND hwnd, int width, int height) override
        {
            DXGI_SWAP_CHAIN_DESC sd = {};
            sd.BufferCount = 2;
            sd.BufferDesc.Width = (UINT)width;
            sd.BufferDesc.Height = (UINT)height;
            sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;   // lets WGT build frosted glass without a copy
            sd.OutputWindow = hwnd;
            sd.SampleDesc.Count = 1;
            sd.Windowed = TRUE;
            sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            tearing_ = HostTearingSupported();
            if (tearing_)
                sd.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
            UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
            if (g_debugLayer)
                flags |= D3D11_CREATE_DEVICE_DEBUG;
            const D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
            ComPtr<IDXGIAdapter1> adapter;
            adapter.Attach(HostPickAdapter());
            const D3D_DRIVER_TYPE driver = adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE;
            HRESULT hr = D3D11CreateDeviceAndSwapChain(adapter.Get(), driver, nullptr, flags, levels, 2, D3D11_SDK_VERSION,
                                                       &sd, &swap_, &device_, nullptr, &ctx_);
            if (FAILED(hr) && (flags & D3D11_CREATE_DEVICE_DEBUG))
            {
                flags &= ~D3D11_CREATE_DEVICE_DEBUG;
                hr = D3D11CreateDeviceAndSwapChain(adapter.Get(), driver, nullptr, flags, levels, 2, D3D11_SDK_VERSION,
                                                   &sd, &swap_, &device_, nullptr, &ctx_);
            }
            if (FAILED(hr))
                return false;
            width_ = width;
            height_ = height;
            return CreateRtv();
        }

        void Describe(wgt::ContextDesc& d) override
        {
            d.backend = wgt::Backend::D3D11;
            d.d3d11Device = device_.Get();
            d.d3d11Context = ctx_.Get();
        }

        void Resize(int width, int height) override
        {
            if (width <= 0 || height <= 0 || (width == width_ && height == height_))
                return;
            rtv_.Reset();
            ctx_->ClearState();
            ctx_->Flush();
            swap_->ResizeBuffers(0, (UINT)width, (UINT)height, DXGI_FORMAT_UNKNOWN, tearing_ ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);
            width_ = width;
            height_ = height;
            CreateRtv();
        }

        void BeginFrame(const float clear[4]) override
        {
            ID3D11RenderTargetView* rtv = rtv_.Get();
            ctx_->OMSetRenderTargets(1, &rtv, nullptr);
            ctx_->ClearRenderTargetView(rtv, clear);
        }

        wgt::RenderTarget Target() override { return wgt::RenderTarget::D3D11(rtv_.Get()); }

        void EndFrame(bool vsync) override
        {
            if (!capturePath_.empty())
            {
                Capture(capturePath_);
                capturePath_.clear();
            }
            // VSync off really means uncapped (tearing / VRR) when the display path allows it
            swap_->Present(vsync ? 1 : 0, (!vsync && tearing_) ? DXGI_PRESENT_ALLOW_TEARING : 0);
        }

        bool CaptureNextFrame(const wchar_t* path) override
        {
            capturePath_ = path;
            return true;
        }

        void WaitIdle() override
        {
            ctx_->ClearState();
            ctx_->Flush();
        }

        int DumpDebugMessages(const wchar_t* path) override
        {
            ComPtr<ID3D11InfoQueue> q;
            if (FAILED(device_.As(&q)))
                return 0;
            FILE* f = nullptr;
            _wfopen_s(&f, path, L"a");
            int issues = 0;
            const UINT64 n = q->GetNumStoredMessages();
            for (UINT64 i = 0; i < n; ++i)
            {
                SIZE_T len = 0;
                q->GetMessage(i, nullptr, &len);
                std::string buf(len, '\0');
                auto* m = reinterpret_cast<D3D11_MESSAGE*>(buf.data());
                if (FAILED(q->GetMessage(i, m, &len)))
                    continue;
                if (m->Severity <= D3D11_MESSAGE_SEVERITY_WARNING)
                {
                    ++issues;
                    if (f)
                        fprintf(f, "[d3d11 %d] %.*s\n", (int)m->Severity, (int)m->DescriptionByteLength, m->pDescription);
                }
            }
            if (f)
            {
                fprintf(f, "(%llu messages stored in the d3d11 info queue)\n", (unsigned long long)n);
                fclose(f);
            }
            return issues;
        }

    private:
        bool CreateRtv()
        {
            ComPtr<ID3D11Texture2D> back;
            if (FAILED(swap_->GetBuffer(0, IID_PPV_ARGS(&back))))
                return false;
            return SUCCEEDED(device_->CreateRenderTargetView(back.Get(), nullptr, &rtv_));
        }

        void Capture(const std::wstring& path)
        {
            ComPtr<ID3D11Texture2D> back;
            swap_->GetBuffer(0, IID_PPV_ARGS(&back));
            D3D11_TEXTURE2D_DESC d;
            back->GetDesc(&d);
            d.Usage = D3D11_USAGE_STAGING;
            d.BindFlags = 0;
            d.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            d.MiscFlags = 0;
            ComPtr<ID3D11Texture2D> staging;
            if (FAILED(device_->CreateTexture2D(&d, nullptr, &staging)))
                return;
            ctx_->CopyResource(staging.Get(), back.Get());
            D3D11_MAPPED_SUBRESOURCE m;
            if (SUCCEEDED(ctx_->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &m)))
            {
                SavePng(path, (const std::uint8_t*)m.pData, (int)d.Width, (int)d.Height, (int)m.RowPitch, d.Format == DXGI_FORMAT_B8G8R8A8_UNORM);
                ctx_->Unmap(staging.Get(), 0);
            }
        }

        ComPtr<ID3D11Device> device_;
        ComPtr<ID3D11DeviceContext> ctx_;
        bool tearing_ = false;
        ComPtr<IDXGISwapChain> swap_;
        ComPtr<ID3D11RenderTargetView> rtv_;
        int width_ = 0, height_ = 0;
        std::wstring capturePath_;
    };
}

std::unique_ptr<IHost> CreateHostD3D11() { return std::make_unique<HostD3D11>(); }
