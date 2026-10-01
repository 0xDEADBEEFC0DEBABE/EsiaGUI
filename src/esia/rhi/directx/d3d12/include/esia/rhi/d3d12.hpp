// Esia - Direct3D 12 backend: host integration.
//
//   esia::rhi::d3d12::Desc d;
//   d.device = device;  d.queue = queue;  d.framesInFlight = 2;
//   auto dev = esia::rhi::d3d12::CreateDevice(d);
//   esia::render::Renderer renderer(*dev);
//   // every frame, recording into the host's open command list:
//   esia::rhi::Texture target = esia::rhi::d3d12::WrapRenderTarget(*dev, backBuffer, rtvHandle, DXGI_FORMAT_R8G8B8A8_UNORM,
//                                                                    D3D12_RESOURCE_STATE_RENDER_TARGET);
//   esia::render::RenderParams params;  params.frame.nativeContext = commandList;   // ID3D12GraphicsCommandList*
//   renderer.Render(drawData, &textures, target, params);
//
// * Two ways to run a frame:
//   - FrameDesc::nativeContext = the host's ID3D12GraphicsCommandList (open, recording): the frame is recorded into
//     it and the host executes it. Per-frame memory (constants, vertices, instances, uploads, descriptor tables) is
//     a ring of `framesInFlight` slots, and a slot is reused framesInFlight frames later: the host must have waited
//     for that frame's GPU work by then. One Renderer::Render is one device frame; a host that renders several targets
//     per frame passes its frame number in FrameDesc::hostFrame (RenderParams::frame), and the device frames of one
//     host frame share a slot, so framesInFlight counts host frames (their lists executed in recording order).
//   - nativeContext = null with Desc::queue: the device records into its own command list, executes it on `queue`
//     at EndFrame and fences its ring slots itself (what the headless conformance device does).
// * ReadPixels (tests, screenshots) needs Desc::queue: it records a copy on the device's own list, executes it and
//   waits. With host command lists, call it after the host executed the frame.
// * WrapRenderTarget: the resource is in `state` when a frame starts and is returned to it at EndFrame; it is cached
//   per resource (cheap every frame) and holds a reference: DestroyTexture it before IDXGISwapChain::ResizeBuffers.
//   It is sampleable (the no-copy capture path) when it has one sample, allows shader resources, and its storage is
//   not a typed sRGB format.
// * Shader model 5.0 bytecode (D3DCompile from d3dcompiler_47.dll, compiled at runtime and cached per process).
// * The host keeps the GPU idle-safe for destruction: destroy the device after its last frame completed (a device
//   running its own list waits for it).
// * On a host command list the frame leaves its own descriptor heap, root signature, PSO and render target bound:
//   the host sets its own again after Renderer::Render.
// * NativeRenderState() (host draw callbacks) returns the ID3D12GraphicsCommandList* being recorded.
#pragma once
#include "esia/rhi/d3d_common.hpp"
#include <d3d12.h>
#include <memory>
#include <string>

namespace esia::rhi::d3d12
{
    struct Desc
    {
        ID3D12Device* device = nullptr;
        ID3D12CommandQueue* queue = nullptr;   // direct queue: own-list frames and ReadPixels (optional otherwise)
        int framesInFlight = 2;
        d3d::DebugDesc debug;
    };

    std::unique_ptr<Device> CreateDevice(const Desc& desc, std::string* error = nullptr);

    // `rtv` is the host's descriptor of a render target view of the resource in `format` (e.g. an _SRGB view of a
    // UNORM swap-chain buffer); the resource is in `stateOnEntryAndExit` whenever no Esia frame is recording.
    Texture WrapRenderTarget(Device& device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE rtv, DXGI_FORMAT format,
                             D3D12_RESOURCE_STATES stateOnEntryAndExit);
}
