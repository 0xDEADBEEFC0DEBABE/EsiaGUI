// Direct3D 12 host integration: frames recorded into the host's command list (FrameDesc::nativeContext) for a
// wrapped host render target that is handed back in its entry state; the host executes and waits; readback; the
// submission contracts (on trust, with the host's fence).
#include "esia/rhi/backend_registry.hpp"
#include "esia/rhi/d3d12.hpp"
#include "d3d_util.hpp"
#include "esia_test.hpp"
#include "scenes.hpp"
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

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

namespace
{
    // The host's side of the fence contract (FrameDesc::nativeFence): a device and its queue, one command list with an
    // allocator per frame, the fence signaled after each frame's list, a gate fence the queue can be made to wait on
    // (the GPU held back until the test opens it), and render targets of the conformance scenes' size.
    struct FencedHost
    {
        static constexpr int kTargets = 3;
        ComPtr<ID3D12Device> device;
        ComPtr<ID3D12CommandQueue> queue;
        ComPtr<ID3D12CommandAllocator> allocs[8];
        ComPtr<ID3D12GraphicsCommandList> list;
        ComPtr<ID3D12Fence> fence, gate;
        ComPtr<ID3D12DescriptorHeap> rtvHeap;
        ComPtr<ID3D12Resource> res[kTargets];
        std::unique_ptr<rhi::Device> dev;
        rhi::Texture targets[kTargets];

        bool Init(int framesInFlight, int maxFramesInFlight, int hostFenceWaitMs)
        {
            if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))))
                return false;
            D3D12_COMMAND_QUEUE_DESC qd = {};
            qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            ESIA_CHECK(SUCCEEDED(device->CreateCommandQueue(&qd, IID_PPV_ARGS(&queue))));
            for (auto& a : allocs)
                ESIA_CHECK(SUCCEEDED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&a))));
            ESIA_CHECK(SUCCEEDED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocs[0].Get(), nullptr, IID_PPV_ARGS(&list))));
            list->Close();
            ESIA_CHECK(SUCCEEDED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))));
            ESIA_CHECK(SUCCEEDED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&gate))));
            rhi::d3d12::Desc desc;
            desc.device = device.Get();
            desc.queue = queue.Get();
            desc.framesInFlight = framesInFlight;
            desc.maxFramesInFlight = maxFramesInFlight;
            desc.hostFenceWaitMs = hostFenceWaitMs;
            desc.debug.debugLayer = rhi::d3d::DebugLevelFromEnvironment() > 0;
            dev = rhi::d3d12::CreateDevice(desc);
            if (!dev)
                return false;
            D3D12_RESOURCE_DESC rd = {};
            rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            rd.Width = 320;
            rd.Height = 240;
            rd.DepthOrArraySize = rd.MipLevels = 1;
            rd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            rd.SampleDesc.Count = 1;
            rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            const D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0};
            D3D12_DESCRIPTOR_HEAP_DESC hd = {};
            hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            hd.NumDescriptors = kTargets;
            ESIA_CHECK(SUCCEEDED(device->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&rtvHeap))));
            const UINT rtvSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            for (int i = 0; i < kTargets; ++i)
            {
                ESIA_CHECK(SUCCEEDED(device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&res[i]))));
                D3D12_CPU_DESCRIPTOR_HANDLE rtv = rtvHeap->GetCPUDescriptorHandleForHeapStart();
                rtv.ptr += (SIZE_T)i * rtvSize;
                device->CreateRenderTargetView(res[i].Get(), nullptr, rtv);
                targets[i] = rhi::d3d12::WrapRenderTarget(*dev, res[i].Get(), rtv, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_STATE_COMMON);
                ESIA_CHECK((bool)targets[i]);
            }
            return true;
        }

        // Host frame `value`: `scene` recorded into target i with the fence and `value`, then (unless told not to) the
        // list executed and the fence signaled to `value` behind it. False when the renderer did not draw the frame.
        bool Frame(int i, std::uint64_t value, render::Renderer& renderer, conformance::SceneFrame& frame, const conformance::Scene& scene,
                   bool execute = true)
        {
            ID3D12CommandAllocator* alloc = allocs[value % 8].Get();   // a test's frames never come round to one again
            alloc->Reset();
            list->Reset(alloc, nullptr);
            render::RenderParams params = conformance::RenderParamsOf(scene);
            params.frame.nativeContext = list.Get();
            params.frame.hostFrame = value;
            params.frame.nativeFence = fence.Get();
            params.frame.nativeFenceValue = value;
            const bool drawn = renderer.Render(frame.data, &frame.textures, targets[i], params);
            list->Close();
            if (execute)
            {
                ID3D12CommandList* lists[] = {list.Get()};
                queue->ExecuteCommandLists(1, lists);
                queue->Signal(fence.Get(), value);
            }
            return drawn;
        }

        void WaitFor(std::uint64_t value) const
        {
            while (fence->GetCompletedValue() < value)
                Sleep(1);
        }

        bool Matches(int i, const testkit::Image& want, const conformance::Scene& scene) const
        {
            testkit::Image got(scene.width, scene.height);
            return dev->ReadPixels(targets[i], rhi::IRect{0, 0, scene.width, scene.height}, got.rgba) && testkit::Compare(got, want, scene.tolerance).pass;
        }

        ~FencedHost()
        {
            if (dev)
                for (const rhi::Texture t : targets)
                    dev->DestroyTexture(t);
        }
    };

    // The headless device's rendering of a scene: what a host target must match.
    bool Reference(const conformance::Scene& scene, testkit::Image& want)
    {
        const rhi::BackendInfo* backend = rhi::FindBackend("d3d12");
        std::string error;
        rhi::HeadlessDevice ref = backend ? backend->createHeadless(conformance::HeadlessDescOf(scene), error) : rhi::HeadlessDevice{};
        if (!ref.device)
        {
            std::printf("  skipped: no D3D12 device (%s)\n", error.c_str());
            return false;
        }
        render::Renderer renderer(*ref.device);
        conformance::SceneFrame frame;
        conformance::BuildScene(scene, frame);
        want = testkit::Image(scene.width, scene.height);
        return renderer.Render(frame.data, &frame.textures, ref.target, conformance::RenderParamsOf(scene)) &&
               ref.device->ReadPixels(ref.target, rhi::IRect{0, 0, scene.width, scene.height}, want.rgba);
    }
}

// The fence contract: frames the GPU has not run yet keep their slots. A gate on the queue holds back three frames of
// different scenes, recorded with one slot to start with; the device makes a slot per frame instead of writing over the
// first frame's memory, and once the gate opens each target matches the headless device's rendering. The slots come back
// when their fences pass, and so do the frames' timestamps.
ESIA_TEST(D3D12Host, FencedFramesGrowTheSlots)
{
    EsiaRegisterBackend_d3d12();
    const char* const names[FencedHost::kTargets] = {"shapes", "glass", "text_fields"};
    const conformance::Scene* scenes[FencedHost::kTargets] = {};
    testkit::Image want[FencedHost::kTargets];
    for (int i = 0; i < FencedHost::kTargets; ++i)
    {
        scenes[i] = conformance::FindScene(names[i]);
        ESIA_CHECK(scenes[i] && scenes[i]->width == 320 && scenes[i]->height == 240 && scenes[i]->format == rhi::Format::RGBA8_UNORM);
        if (!scenes[i] || !Reference(*scenes[i], want[i]))
            return;
    }
    FencedHost host;
    if (!host.Init(1, 4, 10000))
    {
        std::printf("  skipped: no D3D12 device\n");
        return;
    }
    host.queue->Wait(host.gate.Get(), 1);   // the GPU runs nothing until the gate opens
    conformance::SceneFrame frames[FencedHost::kTargets];
    std::vector<std::unique_ptr<render::Renderer>> renderers;
    for (int i = 0; i < FencedHost::kTargets; ++i)
    {
        conformance::BuildScene(*scenes[i], frames[i]);
        renderers.push_back(std::make_unique<render::Renderer>(*host.dev));
        ESIA_CHECK(host.Frame(i, (std::uint64_t)i + 1, *renderers.back(), frames[i], *scenes[i]));
    }
    ESIA_CHECK(rhi::d3d12::FrameSlotCount(*host.dev) == 3);   // three frames in flight, none run
    host.gate->Signal(1);
    host.WaitFor(3);
    for (int i = 0; i < FencedHost::kTargets; ++i)
        ESIA_CHECK(host.Matches(i, want[i], *scenes[i]));
    rhi::GpuProfile profile;
    ESIA_CHECK(host.dev->ReadProfile(profile) && profile.valid);   // read once the fence passed, no later frame needed
    ESIA_CHECK(host.Frame(0, 4, *renderers[0], frames[0], *scenes[0]));
    host.WaitFor(4);
    ESIA_CHECK(rhi::d3d12::FrameSlotCount(*host.dev) == 3);   // a slot whose fence passed came back
    ESIA_CHECK(host.Matches(0, want[0], *scenes[0]));
    ESIA_CHECK(host.dev->ValidationErrors() == 0);
}

// At Desc::maxFramesInFlight the device waits for the oldest frame's fence: two slots, the gate closed, the third frame
// waits until a thread opens the gate, takes the first frame's slot, and every frame still draws what it was given.
ESIA_TEST(D3D12Host, FencedFramesWaitAtTheCap)
{
    EsiaRegisterBackend_d3d12();
    const conformance::Scene& scene = *conformance::FindScene("shapes");
    testkit::Image want;
    if (!Reference(scene, want))
        return;
    FencedHost host;
    if (!host.Init(1, 2, 10000))
    {
        std::printf("  skipped: no D3D12 device\n");
        return;
    }
    host.queue->Wait(host.gate.Get(), 1);
    conformance::SceneFrame frame;
    conformance::BuildScene(scene, frame);
    render::Renderer renderer(*host.dev);
    ESIA_CHECK(host.Frame(0, 1, renderer, frame, scene));
    ESIA_CHECK(host.Frame(1, 2, renderer, frame, scene));
    ESIA_CHECK(rhi::d3d12::FrameSlotCount(*host.dev) == 2);
    std::thread opener([&] {
        Sleep(200);
        host.gate->Signal(1);
    });
    const auto t0 = std::chrono::steady_clock::now();
    ESIA_CHECK(host.Frame(2, 3, renderer, frame, scene));   // waits for frame 1's fence
    const double waited = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    opener.join();
    ESIA_CHECK(waited >= 100.0);
    ESIA_CHECK(rhi::d3d12::FrameSlotCount(*host.dev) == 2);
    host.WaitFor(3);
    for (int i = 0; i < 3; ++i)
        ESIA_CHECK(host.Matches(i, want, scene));
    ESIA_CHECK(host.dev->ValidationErrors() == 0);
}

// A frame whose list is never executed never reaches its fence: with every slot waiting on it, the next BeginFrame gives
// up after Desc::hostFenceWaitMs with an error, and the renderer draws nothing rather than hanging.
ESIA_TEST(D3D12Host, AFenceNeverReachedFailsTheFrame)
{
    EsiaRegisterBackend_d3d12();
    const conformance::Scene& scene = *conformance::FindScene("shapes");
    FencedHost host;
    if (!host.Init(1, 1, 50))
    {
        std::printf("  skipped: no D3D12 device\n");
        return;
    }
    conformance::SceneFrame frame;
    conformance::BuildScene(scene, frame);
    render::Renderer renderer(*host.dev);
    ESIA_CHECK(host.Frame(0, 1, renderer, frame, scene, false));   // recorded, never executed
    const std::uint32_t before = host.dev->ValidationErrors();
    ESIA_CHECK(!host.Frame(1, 2, renderer, frame, scene, false));
    ESIA_CHECK(host.dev->ValidationErrors() > before);
    host.fence->Signal(2);   // the host gives the frames up: the device can go without waiting
}
