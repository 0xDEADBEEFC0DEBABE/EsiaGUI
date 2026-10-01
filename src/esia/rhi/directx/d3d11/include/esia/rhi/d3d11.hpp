// Esia - Direct3D 11 backend: host integration.
//
//   auto dev = esia::rhi::d3d11::CreateDevice(d3dDevice, immediateContext);
//   esia::render::Renderer renderer(*dev);
//   // every frame, with the swap chain's current back buffer view:
//   esia::rhi::Texture target = esia::rhi::d3d11::WrapRenderTarget(*dev, backBufferRtv);
//   renderer.Render(drawData, &textures, target);
//
// * The device draws with the host's immediate context (FrameDesc::nativeContext stays null). With
//   restoreHostState (the default) the context's pipeline state is saved at BeginFrame and restored at EndFrame, as
//   WGT's D3D11 backend did; without it the host must not rely on any state after Renderer::Render.
// * Needs feature level 11_0 (shader model 5) and d3dcompiler_47.dll: the shaders are compiled at runtime from the
//   embedded HLSL (cached per process) until DXBC is generated on Windows.
// * WrapRenderTarget returns the same texture for the same view (swap-chain buffers are wrapped every frame at no
//   cost). The texture holds a reference to the view: DestroyTexture it before IDXGISwapChain::ResizeBuffers.
//   Its format is the view's; it is sampleable (the no-copy capture path) when the resource has
//   D3D11_BIND_SHADER_RESOURCE, one sample, and storage that is not a typed sRGB format (a typeless resource, or the
//   UNORM buffer of a flip-model swap chain seen through an sRGB view: the shaders then read it raw).
// * NativeRenderState() (host draw callbacks) returns the ID3D11DeviceContext*.
#pragma once
#include "esia/rhi/d3d_common.hpp"
#include <memory>
#include <string>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;

namespace esia::rhi::d3d11
{
    struct Desc
    {
        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* context = nullptr;   // the immediate context
        bool restoreHostState = true;
        d3d::DebugDesc debug;
    };

    // Null (and the reason in `error`) without feature level 11_0 or d3dcompiler_47.dll.
    std::unique_ptr<Device> CreateDevice(const Desc& desc, std::string* error = nullptr);

    inline std::unique_ptr<Device> CreateDevice(ID3D11Device* device, ID3D11DeviceContext* context, bool restoreHostState = true)
    {
        Desc d;
        d.device = device;
        d.context = context;
        d.restoreHostState = restoreHostState;
        return CreateDevice(d);
    }

    // A texture for a host render target view (2D, mip 0); {} if the view's format cannot be a target.
    Texture WrapRenderTarget(Device& device, ID3D11RenderTargetView* rtv);
}
