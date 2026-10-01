// Esia - Direct3D 9 backend: host integration.
//
//   auto dev = esia::rhi::d3d9::CreateDevice(d3d9ExDevice);
//   esia::render::Renderer renderer(*dev);
//   // every frame, between the host's own work (the backend opens and closes its own scenes):
//   esia::rhi::Texture target = esia::rhi::d3d9::WrapRenderTarget(*dev, backBufferSurface);
//   renderer.Render(drawData, &textures, target);
//
// * Direct3D 9Ex is recommended (IDirect3DDevice9Ex): every resource lives in D3DPOOL_DEFAULT, which a plain
//   IDirect3DDevice9 loses on Reset - there is no device-loss protocol yet (docs/REWRITE_STATUS.md, known issue 6).
// * Needs shader model 3 hardware: vs_3_0 / ps_3_0 with vertex texture fetch of A32B32G32R32F (the FX instances),
//   stream-frequency instancing, 32-bit indices, D3DDECLTYPE_UBYTE4N. The shaders are compiled at runtime with
//   d3dcompiler_47.dll with the backend's SM3 prelude (esia_sm3_prelude.hlsli) and cached per process; the FX
//   shader is compiled per batch feature mask (Caps::fxFeatureVariants): the whole shader does not fit SM3.
// * No dual-source blending: sub-pixel glyph pages draw with their grayscale coverage.
// * Pixel centers: the backend reports Caps::halfPixelOffset and moves every vertex by half a pixel of the pass
//   target in the vertex shaders, so the output matches the other APIs'.
// * D3D9 has no sRGB formats, only a render state: WrapRenderTarget(..., srgb = true) makes the target BGRA8_SRGB
//   (D3DRS_SRGBWRITEENABLE while drawing into it). Sampling never decodes (D3DSAMP_SRGBTEXTURE off).
// * WrapRenderTarget returns the same texture for the same surface and holds a reference to it (DestroyTexture it
//   before IDirect3DDevice9Ex::ResetEx). Pass the texture that owns the surface to make it sampleable (the no-copy
//   capture path); a multisampled surface is resolved with StretchRect when captured.
// * The device's state (render states, samplers, shaders, streams, viewport, scissor, render target 0, depth
//   stencil surface) is saved at BeginFrame and restored at EndFrame (restoreHostState).
// * FrameDesc::nativeContext stays null; NativeRenderState() (host draw callbacks) returns the IDirect3DDevice9*.
#pragma once
#include "esia/rhi/d3d_common.hpp"
#include <memory>
#include <string>

struct IDirect3DDevice9;
struct IDirect3DSurface9;
struct IDirect3DTexture9;

namespace esia::rhi::d3d9
{
    struct Desc
    {
        IDirect3DDevice9* device = nullptr;   // an IDirect3DDevice9Ex preferably
        bool restoreHostState = true;
        d3d::DebugDesc debug;
    };

    // Null (and the reason in `error`) without shader model 3 or the capabilities listed above.
    std::unique_ptr<Device> CreateDevice(const Desc& desc, std::string* error = nullptr);

    inline std::unique_ptr<Device> CreateDevice(IDirect3DDevice9* device, bool restoreHostState = true)
    {
        Desc d;
        d.device = device;
        d.restoreHostState = restoreHostState;
        return CreateDevice(d);
    }

    // A texture for a host render target surface (A8R8G8B8 / X8R8G8B8 / A2B10G10R10 / A16B16G16R16F); `texture`: the
    // texture whose level 0 it is (makes it sampleable); `srgb`: the host presents it as sRGB.
    Texture WrapRenderTarget(Device& device, IDirect3DSurface9* surface, IDirect3DTexture9* texture = nullptr, bool srgb = false);
}
