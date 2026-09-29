// glass_window - Direct3D 12 host: a flip-model swap chain, and Esia recording into the host's command list with
// two frames in flight (a fence value per frame slot).
#include "host.hpp"
#include "dxgi_swap_chain.hpp"
#include "esia/rhi/d3d12.hpp"
#include <d3d12sdklayers.h>

namespace glass
{
    namespace
    {
        constexpr UINT kBuffers = 2;
        constexpr int kFramesInFlight = 2;

        class HostD3D12 final : public Host
        {
        public:
            ~HostD3D12() override
            {
                if (fence_)
                    WaitIdle();   // the device and the buffers go only when the GPU is done with them
            }

            const char* Name() const override { return "Direct3D 12"; }

            bool Init(HWND hwnd, int width, int height, const HostOptions& o, std::string& error) override
            {
                vsync_ = o.vsync;
                if (o.debug)
                {
                    ComPtr<ID3D12Debug> debug;   // before the device: the layer applies to devices created afterwards
                    if (SUCCEEDED(::D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
                        debug->EnableDebugLayer();
                }
                DxgiAdapter a;
                if (!PickAdapter(a, error))
                    return false;
                adapter_ = a.name;
                HRESULT hr = ::D3D12CreateDevice(a.adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_));
                if (FAILED(hr))
                {
                    error = "D3D12CreateDevice failed: " + HrText(hr);
                    return false;
                }
                D3D12_COMMAND_QUEUE_DESC qd = {};
                qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                D3D12_DESCRIPTOR_HEAP_DESC hd = {};
                hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                hd.NumDescriptors = kBuffers;
                if (FAILED(hr = device_->CreateCommandQueue(&qd, IID_PPV_ARGS(&queue_))) ||
                    FAILED(hr = device_->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&rtvHeap_))) ||
                    FAILED(hr = device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_))))
                {
                    error = "creating the queue, descriptor heap or fence failed: " + HrText(hr);
                    return false;
                }
                for (auto& allocator : allocators_)
                    if (FAILED(hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))))
                    {
                        error = "CreateCommandAllocator failed: " + HrText(hr);
                        return false;
                    }
                if (FAILED(hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocators_[0].Get(), nullptr, IID_PPV_ARGS(&list_))))
                {
                    error = "CreateCommandList failed: " + HrText(hr);
                    return false;
                }
                list_->Close();   // BeginFrame resets it
                rtvSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
                if (!swap_.Create(a.factory.Get(), queue_.Get(), hwnd, width, height, kBuffers, error))
                    return false;
                if (FAILED(hr = swap_.Get()->QueryInterface(IID_PPV_ARGS(&swap3_))))   // GetCurrentBackBufferIndex
                {
                    error = "no IDXGISwapChain3: " + HrText(hr);
                    return false;
                }

                esia::rhi::d3d12::Desc d;
                d.device = device_.Get();
                d.queue = queue_.Get();   // ReadPixels (--screenshot)
                d.framesInFlight = kFramesInFlight;
                d.debug.debugLayer = o.debug;
                esia_ = esia::rhi::d3d12::CreateDevice(d, &error);
                return esia_ && CreateTargets();
            }

            esia::rhi::Device& Device() override { return *esia_; }

            void Resize(int width, int height) override
            {
                WaitIdle();
                ReleaseTargets();
                if (swap_.Resize(width, height))
                    CreateTargets();
            }

            bool BeginFrame() override
            {
                if (!wrapped_[0])
                    return false;
                const int slot = (int)(frame_ % kFramesInFlight);
                // the frame that used this slot kFramesInFlight frames ago must be done: Esia recycles its per-frame
                // memory on the same schedule (Desc::framesInFlight)
                Wait(slotFence_[slot]);
                allocators_[slot]->Reset();
                list_->Reset(allocators_[slot].Get(), nullptr);
                target_ = wrapped_[swap3_->GetCurrentBackBufferIndex()];
                return true;
            }

            esia::rhi::FrameDesc Frame() override
            {
                esia::rhi::FrameDesc f;
                f.nativeContext = list_.Get();
                f.hostFrame = frame_ + 1;
                return f;
            }

        protected:
            void Submit() override
            {
                list_->Close();
                ID3D12CommandList* lists[] = {list_.Get()};
                queue_->ExecuteCommandLists(1, lists);
            }

            void Present() override
            {
                swap_.Present(vsync_);
                queue_->Signal(fence_.Get(), ++fenceValue_);
                slotFence_[frame_ % kFramesInFlight] = fenceValue_;
                ++frame_;
            }

        private:
            bool CreateTargets()
            {
                D3D12_CPU_DESCRIPTOR_HANDLE rtv = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
                for (UINT i = 0; i < kBuffers; ++i, rtv.ptr += rtvSize_)
                {
                    if (FAILED(swap_.Get()->GetBuffer(i, IID_PPV_ARGS(&buffers_[i]))))
                        return false;
                    device_->CreateRenderTargetView(buffers_[i].Get(), nullptr, rtv);
                    // between frames the buffers rest in PRESENT: Esia moves them to RENDER_TARGET and back itself
                    wrapped_[i] = esia::rhi::d3d12::WrapRenderTarget(*esia_, buffers_[i].Get(), rtv, DXGI_FORMAT_B8G8R8A8_UNORM, D3D12_RESOURCE_STATE_PRESENT);
                    if (!wrapped_[i])
                        return false;
                }
                return true;
            }

            void ReleaseTargets()
            {
                for (UINT i = 0; i < kBuffers; ++i)
                {
                    esia_->DestroyTexture(wrapped_[i]);
                    wrapped_[i] = {};
                    buffers_[i].Reset();
                }
                target_ = {};
            }

            void Wait(UINT64 value)
            {
                if (fence_->GetCompletedValue() >= value)
                    return;
                fence_->SetEventOnCompletion(value, nullptr);   // null event: blocks until the value is reached
            }

            void WaitIdle()
            {
                queue_->Signal(fence_.Get(), ++fenceValue_);
                Wait(fenceValue_);
            }

            ComPtr<ID3D12Device> device_;
            ComPtr<ID3D12CommandQueue> queue_;
            ComPtr<ID3D12CommandAllocator> allocators_[kFramesInFlight];
            ComPtr<ID3D12GraphicsCommandList> list_;
            ComPtr<ID3D12Fence> fence_;
            UINT64 fenceValue_ = 0, slotFence_[kFramesInFlight] = {};
            std::uint64_t frame_ = 0;
            ComPtr<ID3D12DescriptorHeap> rtvHeap_;
            UINT rtvSize_ = 0;
            FlipSwapChain swap_;
            ComPtr<IDXGISwapChain3> swap3_;
            ComPtr<ID3D12Resource> buffers_[kBuffers];
            esia::rhi::Texture wrapped_[kBuffers];
            std::unique_ptr<esia::rhi::Device> esia_;
        };
    }

    std::unique_ptr<Host> CreateHostD3D12() { return std::make_unique<HostD3D12>(); }
}
