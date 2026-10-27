// Esia - Direct3D 12 backend: host integration.
//
//   esia::rhi::d3d12::Desc d;
//   d.device = device;  d.queue = queue;  d.framesInFlight = 2;
//   auto dev = esia::rhi::d3d12::CreateDevice(d);
//   esia::render::Renderer renderer(*dev);
//   // every frame, recording into the host's open command list:
//   esia::rhi::Texture target = esia::rhi::d3d12::WrapRenderTarget(*dev, backBuffer, rtvHandle, DXGI_FORMAT_R8G8B8A8_UNORM,
//                                                                    D3D12_RESOURCE_STATE_RENDER_TARGET);
//   esia::render::RenderParams params;
//   params.frame.nativeContext = commandList;                                 // ID3D12GraphicsCommandList*
//   params.frame.nativeFence = fence;  params.frame.nativeFenceValue = value;   // signaled after the list executed
//   renderer.Render(drawData, &textures, target, params);
//
// * A frame's memory - constants, vertices, instances, uploads, descriptor tables, timestamps, the releases it
//   deferred - is a slot, recycled once the GPU is done with the frame. How the device knows that is the submission
//   contract, one of three per frame:
//   - Host list with a fence (what a host should do): FrameDesc::nativeContext = the host's ID3D12GraphicsCommandList
//     (open, recording), FrameDesc::nativeFence = the ID3D12Fence the host signals after executing that list and
//     nativeFenceValue = the value it signals. A slot is reused once its fence reached its value; while no slot has,
//     the device makes another (Desc::framesInFlight at the start, Desc::maxFramesInFlight at most) and past that waits
//     for the oldest - at most Desc::hostFenceWaitMs, then BeginFrame fails with an error (a list never executed).
//   - Host list on trust: no fence. A slot is reused framesInFlight frames later and the host must have waited for that
//     frame's GPU work by then; nothing checks it - a host that runs further ahead has its frames' memory overwritten
//     while the GPU still reads it.
//   - Own list: nativeContext = null with Desc::queue - the device records into its own command list, executes it on
//     `queue` at EndFrame and fences its slots itself (what the headless conformance device does).
//   One Renderer::Render is one device frame. A host that renders several targets per frame passes its frame number in
//   FrameDesc::hostFrame (RenderParams::frame): the device frames of one host frame share a slot, so framesInFlight
//   counts host frames. Frames complete in the order they were recorded (one queue, increasing fence values).
// * ReadPixels (tests, screenshots) needs Desc::queue: it records a copy on the device's own list, executes it and
//   waits. With host command lists, call it after the host executed the frame.
// * WrapRenderTarget: the resource is in `state` when a frame starts and is returned to it at EndFrame; it is cached
//   per resource (cheap every frame) and holds a reference: DestroyTexture it before IDXGISwapChain::ResizeBuffers.
//   It is sampleable (the no-copy capture path) when it has one sample, allows shader resources, and its storage is
//   not a typed sRGB format.
// * Shader model 5.0 bytecode (D3DCompile from d3dcompiler_47.dll, compiled at runtime and cached per process).
// * The host keeps the GPU idle-safe for destruction: destroy the device after its last frame completed (a device
//   running its own list waits for it, and for the fences of host lists it was given, at most hostFenceWaitMs).
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
        int framesInFlight = 2;      // slots made at the start; on trust a slot comes back this many host frames later
        int maxFramesInFlight = 8;   // host lists with a fence: the most slots made before BeginFrame waits for one
        int hostFenceWaitMs = 10000; // how long BeginFrame waits for a host fence before it fails
        d3d::DebugDesc debug;
    };

    std::unique_ptr<Device> CreateDevice(const Desc& desc, std::string* error = nullptr);

    // Diagnostics: the frame slots the device has made (more than framesInFlight when host fences lagged behind).
    int FrameSlotCount(Device& device);

    // `rtv` is the host's descriptor of a render target view of the resource in `format` (e.g. an _SRGB view of a
    // UNORM swap-chain buffer); the resource is in `stateOnEntryAndExit` whenever no Esia frame is recording.
    Texture WrapRenderTarget(Device& device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE rtv, DXGI_FORMAT format,
                             D3D12_RESOURCE_STATES stateOnEntryAndExit);
}
