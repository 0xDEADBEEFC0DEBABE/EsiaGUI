// Direct3D 12 host integration: frames recorded into the host's command list (FrameDesc::nativeContext) for a
// wrapped host render target that is handed back in its entry state; the host executes and waits; readback.
#include "esia/rhi/backend_registry.hpp"
#include "esia/rhi/d3d12.hpp"
#include "d3d_util.hpp"
#include "esia_test.hpp"
#include "scenes.hpp"

void EsiaRegisterBackend_d3d12();

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

// A host that renders two targets per frame (FrameDesc::hostFrame) into its command list, two frames in flight, for
// five frames: the device frames of one host frame share a ring slot, so the host's fence per frame is all the
// backend needs. Both targets match the headless device's rendering of the scene.
ESIA_TEST(D3D12Host, TwoTargetsPerHostFrame)
{
    const conformance::Scene& scene = *conformance::FindScene("glass");
    EsiaRegisterBackend_d3d12();
    const rhi::BackendInfo* backend = rhi::FindBackend("d3d12");
    std::string error;
    rhi::HeadlessDevice reference = backend ? backend->createHeadless(conformance::HeadlessDescOf(scene), error) : rhi::HeadlessDevice{};
    if (!reference.device)
    {
        std::printf("  skipped: no D3D12 device (%s)\n", error.c_str());
        return;
    }
    const rhi::IRect all{0, 0, scene.width, scene.height};
    testkit::Image want(scene.width, scene.height);
    {
        render::Renderer renderer(*reference.device);
        conformance::SceneFrame frame;
        conformance::BuildScene(scene, frame);
        ESIA_CHECK(renderer.Render(frame.data, &frame.textures, reference.target, conformance::RenderParamsOf(scene)));
        ESIA_CHECK(reference.device->ReadPixels(reference.target, all, want.rgba));
    }

    // the host: a device (the headless one enabled the debug layer when ESIA_D3D_DEBUG is set), a list, an allocator
    // and a fence value per frame in flight, two render targets
    constexpr int kFrames = 2;
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12CommandQueue> queue;
    ComPtr<ID3D12CommandAllocator> allocs[kFrames];
    ComPtr<ID3D12GraphicsCommandList> list;
    ComPtr<ID3D12Fence> fence;
    D3D12_COMMAND_QUEUE_DESC qd = {};
    qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ESIA_CHECK(SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))));
    ESIA_CHECK(SUCCEEDED(device->CreateCommandQueue(&qd, IID_PPV_ARGS(&queue))));
    for (auto& a : allocs)
        ESIA_CHECK(SUCCEEDED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&a))));
    ESIA_CHECK(SUCCEEDED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocs[0].Get(), nullptr, IID_PPV_ARGS(&list))));
    ESIA_CHECK(SUCCEEDED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))));
    list->Close();

    rhi::d3d12::Desc desc;
    desc.device = device.Get();
    desc.queue = queue.Get();
    desc.framesInFlight = kFrames;
    desc.debug.debugLayer = rhi::d3d::DebugLevelFromEnvironment() > 0;
    auto dev = rhi::d3d12::CreateDevice(desc);
    ESIA_CHECK(dev != nullptr);
    if (!dev)
        return;

    D3D12_RESOURCE_DESC rd = {};
    rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    rd.Width = (UINT64)scene.width;
    rd.Height = (UINT)scene.height;
    rd.DepthOrArraySize = rd.MipLevels = 1;
    rd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rd.SampleDesc.Count = 1;
    rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    const D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0};
    D3D12_DESCRIPTOR_HEAP_DESC hd = {};
    hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    hd.NumDescriptors = 2;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ESIA_CHECK(SUCCEEDED(device->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&rtvHeap))));
    const UINT rtvSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    ComPtr<ID3D12Resource> res[2];
    rhi::Texture targets[2];
    for (int i = 0; i < 2; ++i)
    {
        ESIA_CHECK(SUCCEEDED(device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&res[i]))));
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        rtv.ptr += (SIZE_T)i * rtvSize;
        device->CreateRenderTargetView(res[i].Get(), nullptr, rtv);
        targets[i] = rhi::d3d12::WrapRenderTarget(*dev, res[i].Get(), rtv, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_STATE_COMMON);
        ESIA_CHECK((bool)targets[i]);
    }

    render::Renderer renderers[2] = {render::Renderer(*dev), render::Renderer(*dev)};
    constexpr UINT64 kHostFrames = 5;
    for (UINT64 f = 1; f <= kHostFrames; ++f)
    {
        // the host's side of the contract: frame f - kFrames is done before frame f is recorded
        while (f > kFrames && fence->GetCompletedValue() < f - kFrames)
            Sleep(1);
        ID3D12CommandAllocator* alloc = allocs[f % kFrames].Get();
        alloc->Reset();
        list->Reset(alloc, nullptr);
        for (int i = 0; i < 2; ++i)
        {
            conformance::SceneFrame frame;
            conformance::BuildScene(scene, frame);
            render::RenderParams params = conformance::RenderParamsOf(scene);
            params.frame.nativeContext = list.Get();
            params.frame.hostFrame = f;
            ESIA_CHECK(renderers[i].Render(frame.data, &frame.textures, targets[i], params));
        }
        ESIA_CHECK(SUCCEEDED(list->Close()));
        ID3D12CommandList* lists[] = {list.Get()};
        queue->ExecuteCommandLists(1, lists);
        queue->Signal(fence.Get(), f);
    }
    while (fence->GetCompletedValue() < kHostFrames)
        Sleep(1);
    for (const rhi::Texture t : targets)
    {
        testkit::Image got(scene.width, scene.height);
        ESIA_CHECK(dev->ReadPixels(t, all, got.rgba));
        ESIA_CHECK(testkit::Compare(got, want, scene.tolerance).pass);
        dev->DestroyTexture(t);
    }
    ESIA_CHECK(dev->ValidationErrors() == 0);
}
