// WGT demo - Direct3D 12 host
#include "host.hpp"
#include "image_io.hpp"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <string>

using Microsoft::WRL::ComPtr;

namespace
{
    constexpr UINT kFrames = 3;

    class HostD3D12 final : public IHost
    {
    public:
        const char* Name() const override { return "Direct3D 12"; }

        ~HostD3D12() override
        {
            if (fenceEvent_)
                CloseHandle(fenceEvent_);
        }

        bool Init(HWND hwnd, int width, int height) override
        {
            if (HostDebugLayer())
            {
                ComPtr<ID3D12Debug> debug;
                if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
                    debug->EnableDebugLayer();
            }
            ComPtr<IDXGIFactory4> factory;
            if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
                return false;
            ComPtr<IDXGIAdapter1> adapter;
            adapter.Attach(HostPickAdapter());
            if (FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_))))
                return false;

            D3D12_COMMAND_QUEUE_DESC qd = {};
            qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            if (FAILED(device_->CreateCommandQueue(&qd, IID_PPV_ARGS(&queue_))))
                return false;

            DXGI_SWAP_CHAIN_DESC1 sd = {};
            sd.Width = (UINT)width;
            sd.Height = (UINT)height;
            sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.SampleDesc.Count = 1;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;   // lets WGT build frosted glass without a copy
            sd.BufferCount = kFrames;
            sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            tearing_ = HostTearingSupported();
            if (tearing_)
                sd.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
            ComPtr<IDXGISwapChain1> sc1;
            if (FAILED(factory->CreateSwapChainForHwnd(queue_.Get(), hwnd, &sd, nullptr, nullptr, &sc1)))
                return false;
            sc1.As(&swap_);
            factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

            D3D12_DESCRIPTOR_HEAP_DESC hd = {};
            hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            hd.NumDescriptors = kFrames;
            device_->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&rtvHeap_));
            rtvInc_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

            for (UINT i = 0; i < kFrames; ++i)
                device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frames_[i].allocator));
            device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frames_[0].allocator.Get(), nullptr, IID_PPV_ARGS(&list_));
            list_->Close();
            device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
            fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            width_ = width;
            height_ = height;
            CreateRtvs();
            return true;
        }

        void Describe(wgt::ContextDesc& d) override
        {
            d.backend = wgt::Backend::D3D12;
            d.d3d12Device = device_.Get();
            d.d3d12FramesInFlight = kFrames;
        }

        void Resize(int width, int height) override
        {
            if (width <= 0 || height <= 0 || (width == width_ && height == height_))
                return;
            WaitIdle();
            for (auto& b : backBuffers_)
                b.Reset();
            swap_->ResizeBuffers(0, (UINT)width, (UINT)height, DXGI_FORMAT_UNKNOWN, tearing_ ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);
            width_ = width;
            height_ = height;
            CreateRtvs();
        }

        void BeginFrame(const float clear[4]) override
        {
            backIndex_ = swap_->GetCurrentBackBufferIndex();
            Frame& f = frames_[backIndex_];
            WaitFor(f.fenceValue);
            f.allocator->Reset();
            list_->Reset(f.allocator.Get(), nullptr);
            Barrier(backBuffers_[backIndex_].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            const D3D12_CPU_DESCRIPTOR_HANDLE rtv = Rtv(backIndex_);
            list_->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
            list_->ClearRenderTargetView(rtv, clear, 0, nullptr);
        }

        wgt::RenderTarget Target() override
        {
            return wgt::RenderTarget::D3D12(list_.Get(), backBuffers_[backIndex_].Get(), Rtv(backIndex_).ptr, DXGI_FORMAT_R8G8B8A8_UNORM);
        }

        void EndFrame(bool vsync) override
        {
            ComPtr<ID3D12Resource> readback;
            UINT64 rowPitch = 0, readbackSize = 0;
            if (!capturePath_.empty())
            {
                // copy the back buffer into a readback buffer (recorded before present)
                D3D12_RESOURCE_DESC bd = backBuffers_[backIndex_]->GetDesc();
                D3D12_PLACED_SUBRESOURCE_FOOTPRINT fp;
                UINT64 total = 0;
                device_->GetCopyableFootprints(&bd, 0, 1, 0, &fp, nullptr, nullptr, &total);
                rowPitch = fp.Footprint.RowPitch;
                readbackSize = total;
                D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_READBACK};
                D3D12_RESOURCE_DESC rd = {};
                rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
                rd.Width = total;
                rd.Height = rd.DepthOrArraySize = rd.MipLevels = 1;
                rd.SampleDesc.Count = 1;
                rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                const HRESULT hr = device_->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readback));
                if (FAILED(hr))
                    HostLog("d3d12 capture: readback buffer creation failed (0x%08X, %llu bytes)", (unsigned)hr, (unsigned long long)total);
                Barrier(backBuffers_[backIndex_].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
                D3D12_TEXTURE_COPY_LOCATION dst = {}, src = {};
                dst.pResource = readback.Get();
                dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                dst.PlacedFootprint = fp;
                src.pResource = backBuffers_[backIndex_].Get();
                src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                list_->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
                Barrier(backBuffers_[backIndex_].Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PRESENT);
            }
            else
                Barrier(backBuffers_[backIndex_].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            list_->Close();
            ID3D12CommandList* lists[] = {list_.Get()};
            queue_->ExecuteCommandLists(1, lists);
            if (readback)
            {
                WaitIdle();
                void* data = nullptr;
                D3D12_RANGE range = {0, (SIZE_T)readbackSize};   // the last row is not padded to RowPitch
                const HRESULT hr = readback->Map(0, &range, &data);
                if (SUCCEEDED(hr))
                {
                    if (!SavePng(capturePath_, (const std::uint8_t*)data, width_, height_, (int)rowPitch, false))
                        HostLog("d3d12 capture: SavePng failed");
                    D3D12_RANGE none = {0, 0};
                    readback->Unmap(0, &none);
                }
                else
                    HostLog("d3d12 capture: Map failed (0x%08X), device removed reason 0x%08X", (unsigned)hr, (unsigned)device_->GetDeviceRemovedReason());
                capturePath_.clear();
            }
            swap_->Present(vsync ? 1 : 0, (!vsync && tearing_) ? DXGI_PRESENT_ALLOW_TEARING : 0);
            frames_[backIndex_].fenceValue = ++fenceValue_;
            queue_->Signal(fence_.Get(), fenceValue_);
        }

        bool CaptureNextFrame(const wchar_t* path) override
        {
            capturePath_ = path;
            return true;
        }

        int DumpDebugMessages(const wchar_t* path) override
        {
            ComPtr<ID3D12InfoQueue> q;
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
                auto* m = reinterpret_cast<D3D12_MESSAGE*>(buf.data());
                if (FAILED(q->GetMessage(i, m, &len)))
                    continue;
                if (m->Severity <= D3D12_MESSAGE_SEVERITY_WARNING)
                {
                    ++issues;
                    if (f)
                        fprintf(f, "[d3d12 %d] %.*s\n", (int)m->Severity, (int)m->DescriptionByteLength, m->pDescription);
                }
            }
            if (f)
            {
                fprintf(f, "(%llu messages stored in the d3d12 info queue)\n", (unsigned long long)n);
                fclose(f);
            }
            return issues;
        }

        void WaitIdle() override
        {
            const UINT64 v = ++fenceValue_;
            queue_->Signal(fence_.Get(), v);
            WaitFor(v);
        }

    private:
        struct Frame
        {
            ComPtr<ID3D12CommandAllocator> allocator;
            UINT64 fenceValue = 0;
        };

        D3D12_CPU_DESCRIPTOR_HANDLE Rtv(UINT i) const
        {
            D3D12_CPU_DESCRIPTOR_HANDLE h = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
            h.ptr += (SIZE_T)i * rtvInc_;
            return h;
        }

        void CreateRtvs()
        {
            for (UINT i = 0; i < kFrames; ++i)
            {
                swap_->GetBuffer(i, IID_PPV_ARGS(&backBuffers_[i]));
                device_->CreateRenderTargetView(backBuffers_[i].Get(), nullptr, Rtv(i));
            }
        }

        void WaitFor(UINT64 value)
        {
            if (value == 0 || fence_->GetCompletedValue() >= value)
                return;
            fence_->SetEventOnCompletion(value, fenceEvent_);
            WaitForSingleObject(fenceEvent_, INFINITE);
        }

        void Barrier(ID3D12Resource* r, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to)
        {
            D3D12_RESOURCE_BARRIER b = {};
            b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            b.Transition.pResource = r;
            b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            b.Transition.StateBefore = from;
            b.Transition.StateAfter = to;
            list_->ResourceBarrier(1, &b);
        }

        ComPtr<ID3D12Device> device_;
        ComPtr<ID3D12CommandQueue> queue_;
        bool tearing_ = false;
        ComPtr<IDXGISwapChain3> swap_;
        ComPtr<ID3D12DescriptorHeap> rtvHeap_;
        UINT rtvInc_ = 0;
        ComPtr<ID3D12Resource> backBuffers_[kFrames];
        Frame frames_[kFrames];
        ComPtr<ID3D12GraphicsCommandList> list_;
        ComPtr<ID3D12Fence> fence_;
        HANDLE fenceEvent_ = nullptr;
        UINT64 fenceValue_ = 0;
        UINT backIndex_ = 0;
        int width_ = 0, height_ = 0;
        std::wstring capturePath_;
    };
}

std::unique_ptr<IHost> CreateHostD3D12() { return std::make_unique<HostD3D12>(); }
