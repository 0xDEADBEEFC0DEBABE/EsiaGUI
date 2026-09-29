// Direct3D 12 host integration: frames recorded into the host's command list (FrameDesc::nativeContext) for a
// wrapped host render target that is handed back in its entry state; the host executes and waits; readback.
#include "esia/rhi/d3d12.hpp"
#include "d3d_util.hpp"
#include "esia_test.hpp"

using namespace esia;
using esia::rhi::d3d::ComPtr;

ESIA_TEST(D3D12Host, HostCommandList)
{
    ComPtr<ID3D12Device> device;
    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))))
    {
        std::printf("  skipped: no D3D12 device\n");
        return;
    }
    ComPtr<ID3D12CommandQueue> queue;
    D3D12_COMMAND_QUEUE_DESC qd = {};
    qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ComPtr<ID3D12CommandAllocator> alloc;
    ComPtr<ID3D12GraphicsCommandList> list;
    ComPtr<ID3D12Fence> fence;
    ESIA_CHECK(SUCCEEDED(device->CreateCommandQueue(&qd, IID_PPV_ARGS(&queue))));
    ESIA_CHECK(SUCCEEDED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&alloc))));
    ESIA_CHECK(SUCCEEDED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc.Get(), nullptr, IID_PPV_ARGS(&list))));
    ESIA_CHECK(SUCCEEDED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))));
    list->Close();

    rhi::d3d12::Desc desc;
    desc.device = device.Get();
    desc.queue = queue.Get();
    desc.framesInFlight = 1;   // the test waits for every frame
    auto dev = rhi::d3d12::CreateDevice(desc);
    ESIA_CHECK(dev != nullptr);
    if (!dev)
        return;

    // the host's target: a UNORM buffer, drawn through a UNORM view, living in COMMON between frames
    D3D12_RESOURCE_DESC rd = {};
    rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    rd.Width = 32;
    rd.Height = 16;
    rd.DepthOrArraySize = rd.MipLevels = 1;
    rd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    rd.SampleDesc.Count = 1;
    rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    const D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0};
    ComPtr<ID3D12Resource> res;
    ESIA_CHECK(SUCCEEDED(device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&res))));
    D3D12_DESCRIPTOR_HEAP_DESC hd = {};
    hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    hd.NumDescriptors = 1;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ESIA_CHECK(SUCCEEDED(device->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&rtvHeap))));
    const D3D12_CPU_DESCRIPTOR_HANDLE rtv = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    device->CreateRenderTargetView(res.Get(), nullptr, rtv);

    const rhi::Texture t = rhi::d3d12::WrapRenderTarget(*dev, res.Get(), rtv, DXGI_FORMAT_B8G8R8A8_UNORM, D3D12_RESOURCE_STATE_COMMON);
    ESIA_CHECK((bool)t);
    ESIA_CHECK(rhi::d3d12::WrapRenderTarget(*dev, res.Get(), rtv, DXGI_FORMAT_B8G8R8A8_UNORM, D3D12_RESOURCE_STATE_COMMON) == t);
    const rhi::TextureDesc d = dev->GetTextureDesc(t);
    ESIA_CHECK(d.format == rhi::Format::BGRA8_UNORM && (d.usage & rhi::TextureUsage_Sampled));

    for (int frame = 0; frame < 3; ++frame)
    {
        alloc->Reset();
        list->Reset(alloc.Get(), nullptr);
        rhi::FrameDesc fd;
        fd.nativeContext = list.Get();
        ESIA_CHECK(dev->BeginFrame(fd));
        rhi::PassDesc p;
        p.target = t;
        p.load = rhi::LoadOp::Clear;
        p.clearColor[1] = 0.25f * (float)(frame + 1);   // green 64, 128, 191
        p.clearColor[3] = 1.0f;
        dev->BeginPass(p);
        ESIA_CHECK(dev->NativeRenderState() == list.Get());
        dev->EndPass();
        dev->EndFrame();
        ESIA_CHECK(SUCCEEDED(list->Close()));
        ID3D12CommandList* lists[] = {list.Get()};
        queue->ExecuteCommandLists(1, lists);
        queue->Signal(fence.Get(), (UINT64)frame + 1);
        while (fence->GetCompletedValue() < (UINT64)frame + 1)
            Sleep(1);
    }
    std::vector<std::uint8_t> px;
    ESIA_CHECK(dev->ReadPixels(t, rhi::IRect{0, 0, 32, 16}, px) && px.size() == 32 * 16 * 4);
    if (px.size() == 32 * 16 * 4)
        ESIA_CHECK(px[0] == 0 && px[1] == 191 && px[2] == 0 && px[3] == 255);   // the last frame: green 0.75
    dev->DestroyTexture(t);
}
