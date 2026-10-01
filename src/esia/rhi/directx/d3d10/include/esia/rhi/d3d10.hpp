// Esia - Direct3D 10 backend: host integration.
//
//   auto dev = esia::rhi::d3d10::CreateDevice(d3d10Device);
//   esia::render::Renderer renderer(*dev);
//   esia::rhi::Texture target = esia::rhi::d3d10::WrapRenderTarget(*dev, backBufferRtv);   // every frame
//   renderer.Render(drawData, &textures, target);
//
// * Direct3D 10.0 or 10.1 devices (ID3D10Device1 is an ID3D10Device). Shader model 4 (vs_4_0 / ps_4_0), compiled
//   at runtime with d3dcompiler_47.dll and cached per process. FX instances reach the shaders as an RGBA32F texture
//   (Caps::fxStorage = Texture): SM4 has no structured buffers.
// * The device draws with the host's device (FrameDesc::nativeContext stays null). With restoreHostState (the
//   default) the pipeline state is saved at BeginFrame and restored at EndFrame.
// * WrapRenderTarget returns the same texture for the same view; the texture holds a reference to the view:
//   DestroyTexture it before IDXGISwapChain::ResizeBuffers. It is sampleable (the no-copy capture path) when the
//   resource has D3D10_BIND_SHADER_RESOURCE, one sample, and storage that is not a typed sRGB format.
// * NativeRenderState() (host draw callbacks) returns the ID3D10Device*.
#pragma once
#include "esia/rhi/d3d_common.hpp"
#include <memory>
#include <string>

struct ID3D10Device;
struct ID3D10RenderTargetView;

namespace esia::rhi::d3d10
{
    struct Desc
    {
        ID3D10Device* device = nullptr;
        bool restoreHostState = true;
        d3d::DebugDesc debug;
    };

    // Null (and the reason in `error`) without d3dcompiler_47.dll.
    std::unique_ptr<Device> CreateDevice(const Desc& desc, std::string* error = nullptr);

    inline std::unique_ptr<Device> CreateDevice(ID3D10Device* device, bool restoreHostState = true)
    {
        Desc d;
        d.device = device;
        d.restoreHostState = restoreHostState;
        return CreateDevice(d);
    }

    // A texture for a host render target view (2D, mip 0); {} if the view's format cannot be a target.
    Texture WrapRenderTarget(Device& device, ID3D10RenderTargetView* rtv);
}
