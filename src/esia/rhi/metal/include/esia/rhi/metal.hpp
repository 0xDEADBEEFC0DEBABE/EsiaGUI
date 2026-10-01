// Esia - the Metal RHI backend's host integration (macOS 11+ / iOS 14+, Apple silicon and Intel).
//
// UNVERIFIED: needs macOS. Written on Linux against Apple's documentation; never compiled with the Apple SDK.
//
//   // once: the device, from the host's MTLDevice and (recommended) the queue its command buffers come from
//   esia::rhi::metal::Desc desc;
//   desc.device = (__bridge void*)mtlDevice;
//   desc.commandQueue = (__bridge void*)queue;
//   std::unique_ptr<esia::rhi::Device> device = esia::rhi::metal::CreateDevice(desc);
//   esia::render::Renderer renderer(*device);
//
//   // every frame: wrap the target, record into the host's command buffer, commit it afterwards
//   id<CAMetalDrawable> drawable = [layer nextDrawable];
//   esia::rhi::Texture target = esia::rhi::metal::WrapTexture(*device, (__bridge void*)drawable.texture);
//   esia::render::RenderParams params;
//   params.frame.nativeContext = (__bridge void*)commandBuffer;   // not committed yet
//   renderer.Render(drawData, &textures, target, params);
//   device->DestroyTexture(target);                                // the wrapper only (or keep one per drawable)
//   [commandBuffer presentDrawable:drawable];
//   [commandBuffer commit];
//
// * FrameDesc::nativeContext: an id<MTLCommandBuffer> that is still recording; the backend encodes its passes and
//   blits into it and adds a completed handler. Null: the backend records into a command buffer of its own queue
//   and commits it in EndFrame. Several device frames may go into one command buffer (a host rendering several
//   targets): the backend never waits for a frame, it versions what the CPU rewrites.
// * Targets: WrapTexture(device, id<MTLTexture>) - 2D or 2D multisample, one of the RHI's formats (RGBA8 / BGRA8
//   UNORM or sRGB, RGB10A2, RGBA16Float). The same texture gives the same handle while the wrapper lives; wrapping is
//   cheap (a map lookup; one view for a sampleable sRGB texture). What the renderer can do with it follows from the
//   texture:
//     - CAMetalLayer.framebufferOnly = YES (the default): render only. Glass needs a copy of the target:
//       set framebufferOnly = NO;
//     - MTLTextureUsageShaderRead (framebufferOnly = NO): frosted glass reads the target directly, no copy;
//     - an *_sRGB texture is sampled through a UNORM view: that needs MTLTextureUsagePixelFormatView as well;
//     - a multisampled target is resolved by the backend (a render pass with StoreAndMultisampleResolve) for the
//       backdrop copy; the host resolves it for presentation as before.
// * Host callbacks (DrawCmdKind::Callback): NativeRenderState() is the current id<MTLRenderCommandEncoder>. Change
//   anything on it; the backend binds everything again afterwards. Do not end it.
// * ReadPixels (tests, screenshots) waits for the frames that rendered the texture: their command buffers must have
//   been committed (a host's still recording one makes it fail instead of hanging).
// * Caps: FX instances in a buffer, MSL from the shader library (compiled at runtime, MSL 2.0), RGBA16F pyramids, direct target reads, readback; GPU times per category on GPUs that sample
//   timestamps at encoder boundaries (Apple silicon; see metal_profiler.hpp for the approximation), none on the
//   others. No user HLSL effects (Caps::runtimeEffects = false).
#pragma once
#include "esia/rhi/rhi.hpp"
#include <functional>
#include <memory>
#include <string>
#ifdef __OBJC__
#import <Metal/Metal.h>
#endif

namespace esia::rhi::metal
{
    struct Desc
    {
        void* device = nullptr;         // id<MTLDevice> (required)
        // id<MTLCommandQueue> for the backend's own command buffers (frames without nativeContext, readback).
        // Give the queue the host's command buffers come from, so that everything stays in one submission order;
        // null: the backend creates a queue.
        void* commandQueue = nullptr;
        bool timestamps = true;         // GPU times where the GPU samples at encoder boundaries
        // Contract violations and Metal failures (pipeline compile errors, command buffer errors). Default: stderr.
        std::function<void(const std::string&)> onError;
    };

    // Null when `device` is null. Shader libraries are compiled when the pipelines are first created.
    std::unique_ptr<Device> CreateDevice(const Desc& desc);

    // A host texture (id<MTLTexture>) as a render target / copy source; an invalid handle when `device` is not a
    // Metal device or the texture cannot be used (see above).
    Texture WrapTexture(Device& device, void* texture);

#ifdef __OBJC__
    inline std::unique_ptr<Device> CreateDevice(id<MTLDevice> device, id<MTLCommandQueue> queue = nil)
    {
        Desc d;
        d.device = (__bridge void*)device;
        d.commandQueue = (__bridge void*)queue;
        return CreateDevice(d);
    }
    inline Texture WrapTexture(Device& device, id<MTLTexture> texture) { return WrapTexture(device, (__bridge void*)texture); }
#endif
}
