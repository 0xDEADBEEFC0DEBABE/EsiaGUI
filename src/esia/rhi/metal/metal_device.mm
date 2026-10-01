// Esia - Metal backend: the Gpu layer (metal_gpu.hpp) on Metal, the host integration (esia/rhi/metal.hpp) and the
// backend's registration. Objective-C++ with ARC (-fobjc-arc).
//
// UNVERIFIED: needs macOS. This file was written on Linux, where neither the Apple SDK nor a Metal compiler exists;
// it has never been compiled or run. Everything it does is one Metal call per Gpu method - the decisions (what to
// bind, when to blit, which views, what to keep alive) are made by MetalDevice in plain C++, which is tested on
// Linux through the same Gpu interface (tests/fake_metal_gpu.cpp). Check list for the first macOS build: STATUS.md.
#include "esia/rhi/metal.hpp"
#include "esia/rhi/backend_registry.hpp"
#include "metal_device_core.hpp"
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <unordered_map>

// The raw values metal_tables.hpp uses on every host must be Apple's (compared as integers: they are different enum
// types). MTLCounterDontSample / MTLCounterErrorValue are checked at runtime (MetalGpu), they may not be constant
// expressions in every SDK.
namespace mtl = esia::rhi::metal::mtl;
#define ESIA_SAME(apple, ours) static_assert((std::uint64_t)(apple) == (std::uint64_t)(mtl::ours), #apple)
ESIA_SAME(MTLPixelFormatInvalid, PixelFormatInvalid);
ESIA_SAME(MTLPixelFormatR8Unorm, PixelFormatR8Unorm);
ESIA_SAME(MTLPixelFormatRGBA8Unorm, PixelFormatRGBA8Unorm);
ESIA_SAME(MTLPixelFormatRGBA8Unorm_sRGB, PixelFormatRGBA8Unorm_sRGB);
ESIA_SAME(MTLPixelFormatBGRA8Unorm, PixelFormatBGRA8Unorm);
ESIA_SAME(MTLPixelFormatBGRA8Unorm_sRGB, PixelFormatBGRA8Unorm_sRGB);
ESIA_SAME(MTLPixelFormatRGB10A2Unorm, PixelFormatRGB10A2Unorm);
ESIA_SAME(MTLPixelFormatRGBA16Float, PixelFormatRGBA16Float);
ESIA_SAME(MTLPixelFormatRGBA32Float, PixelFormatRGBA32Float);
ESIA_SAME(MTLTextureUsageShaderRead, TextureUsageShaderRead);
ESIA_SAME(MTLTextureUsageShaderWrite, TextureUsageShaderWrite);
ESIA_SAME(MTLTextureUsageRenderTarget, TextureUsageRenderTarget);
ESIA_SAME(MTLTextureUsagePixelFormatView, TextureUsagePixelFormatView);
ESIA_SAME(MTLBlendFactorZero, BlendFactorZero);
ESIA_SAME(MTLBlendFactorOne, BlendFactorOne);
ESIA_SAME(MTLBlendFactorSourceAlpha, BlendFactorSourceAlpha);
ESIA_SAME(MTLBlendFactorOneMinusSourceAlpha, BlendFactorOneMinusSourceAlpha);
ESIA_SAME(MTLBlendOperationAdd, BlendOperationAdd);
ESIA_SAME(MTLLoadActionDontCare, LoadActionDontCare);
ESIA_SAME(MTLLoadActionLoad, LoadActionLoad);
ESIA_SAME(MTLLoadActionClear, LoadActionClear);
ESIA_SAME(MTLStoreActionDontCare, StoreActionDontCare);
ESIA_SAME(MTLStoreActionStore, StoreActionStore);
ESIA_SAME(MTLStoreActionMultisampleResolve, StoreActionMultisampleResolve);
ESIA_SAME(MTLStoreActionStoreAndMultisampleResolve, StoreActionStoreAndMultisampleResolve);
ESIA_SAME(MTLPrimitiveTypeTriangle, PrimitiveTypeTriangle);
ESIA_SAME(MTLPrimitiveTypeTriangleStrip, PrimitiveTypeTriangleStrip);
ESIA_SAME(MTLVertexFormatUChar4Normalized, VertexFormatUChar4Normalized);
ESIA_SAME(MTLVertexFormatFloat2, VertexFormatFloat2);
ESIA_SAME(MTLVertexStepFunctionPerVertex, VertexStepFunctionPerVertex);
ESIA_SAME(MTLIndexTypeUInt32, IndexTypeUInt32);
#undef ESIA_SAME
static_assert(sizeof(MTLCounterResultTimestamp) == sizeof(std::uint64_t));

namespace esia::rhi::metal
{
    namespace
    {
        NSString* Label(const char* s) { return s ? [NSString stringWithUTF8String:s] : nil; }

        MTLOrigin Origin(int x, int y) { return MTLOriginMake((NSUInteger)x, (NSUInteger)y, 0); }
        MTLSize Size(const IRect& r) { return MTLSizeMake((NSUInteger)r.Width(), (NSUInteger)r.Height(), 1); }

        class MetalGpu final : public Gpu
        {
        public:
            MetalGpu(id<MTLDevice> device, id<MTLCommandQueue> queue, std::function<void(const std::string&)> log)
                : device_(device), queue_(queue ? queue : [device newCommandQueue]), log_(std::move(log))
            {
                // s0 / s1 of the binding model: linear and point, clamped, no mips (every texture has one level)
                MTLSamplerDescriptor* s = [MTLSamplerDescriptor new];
                s.minFilter = s.magFilter = MTLSamplerMinMagFilterLinear;
                s.mipFilter = MTLSamplerMipFilterNotMipmapped;
                s.sAddressMode = s.tAddressMode = MTLSamplerAddressModeClampToEdge;
                linear_ = [device_ newSamplerStateWithDescriptor:s];
                s.minFilter = s.magFilter = MTLSamplerMinMagFilterNearest;
                point_ = [device_ newSamplerStateWithDescriptor:s];

                // MTLGPUFamilyMac2 is deprecated in macOS 27 (no Intel Macs there) but still what Intel Macs report before it
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                caps_.maxTextureSize = ([device_ supportsFamily:MTLGPUFamilyApple3] || [device_ supportsFamily:MTLGPUFamilyMac2]) ? 16384 : 8192;
#pragma clang diagnostic pop
                if (@available(macOS 11.0, iOS 14.0, *))
                {
                    // the core writes ~0 for "no sample" and reads ~0 as "not sampled"
                    const bool sentinels = (std::uint64_t)MTLCounterDontSample == mtl::kCounterDontSample && (std::uint64_t)MTLCounterErrorValue == mtl::kCounterErrorValue;
                    if (sentinels && [device_ supportsCounterSampling:MTLCounterSamplingPointAtStageBoundary])
                        for (id<MTLCounterSet> set in device_.counterSets)
                            if ([set.name isEqualToString:MTLCommonCounterSetTimestamp])
                                timestampSet_ = set;
                }
                caps_.timestamps = timestampSet_ != nil;
            }

            GpuCaps Caps() const override { return caps_; }
            bool SupportsSampleCount(int samples) const override { return samples >= 1 && [device_ supportsTextureSampleCount:(NSUInteger)samples]; }

            // ---- objects
            std::uint32_t CreateTexture(const GpuTextureDesc& d) override
            {
                @autoreleasepool
                {
                    MTLTextureDescriptor* td = [MTLTextureDescriptor new];
                    td.textureType = d.samples > 1 ? MTLTextureType2DMultisample : MTLTextureType2D;
                    td.pixelFormat = (MTLPixelFormat)d.pixelFormat;
                    td.width = (NSUInteger)d.width;
                    td.height = (NSUInteger)d.height;
                    td.mipmapLevelCount = 1;
                    td.sampleCount = (NSUInteger)d.samples;
                    td.usage = (MTLTextureUsage)d.usage;
                    td.storageMode = MTLStorageModePrivate;   // written by render passes and blits only
                    id<MTLTexture> t = [device_ newTextureWithDescriptor:td];
                    if (!t)
                        return 0;
                    t.label = Label(d.label);
                    return Keep(textures_, t);
                }
            }

            std::uint32_t CreateTextureView(std::uint32_t texture, std::uint32_t pixelFormat) override
            {
                id<MTLTexture> v = [Tex(texture) newTextureViewWithPixelFormat:(MTLPixelFormat)pixelFormat];
                return v ? Keep(textures_, v) : 0;
            }

            std::uint32_t AdoptTexture(void* native, HostTextureInfo& info) override
            {
                id<MTLTexture> t = (__bridge id<MTLTexture>)native;
                if (!t)
                    return 0;
                info.width = (int)t.width;
                info.height = (int)t.height;
                info.pixelFormat = (std::uint32_t)t.pixelFormat;
                info.samples = (int)t.sampleCount;
                info.usage = (std::uint32_t)t.usage;
                info.framebufferOnly = t.framebufferOnly;
                info.is2D = (t.textureType == MTLTextureType2D || t.textureType == MTLTextureType2DMultisample) && t.arrayLength == 1;
                return Keep(textures_, t);
            }

            void ReleaseTexture(std::uint32_t texture) override { textures_.erase(texture); }

            std::uint32_t CreateBuffer(std::size_t bytes, const char* label) override
            {
                // shared: the CPU writes vertices, indices, FX instances and staging data, and reads readbacks; on
                // Intel Macs too (shared buffers are coherent there once the command buffer completed)
                id<MTLBuffer> b = [device_ newBufferWithLength:bytes options:MTLResourceStorageModeShared];
                if (!b)
                    return 0;
                b.label = Label(label);
                return Keep(buffers_, b);
            }

            void* BufferContents(std::uint32_t buffer) override
            {
                auto it = buffers_.find(buffer);
                return it != buffers_.end() ? it->second.contents : nullptr;
            }

            void ReleaseBuffer(std::uint32_t buffer) override { buffers_.erase(buffer); }

            std::uint32_t CreatePipeline(const GpuPipelineDesc& d, std::string& error) override
            {
                @autoreleasepool
                {
                    id<MTLFunction> vs = Function(d.vertexSource, error);
                    id<MTLFunction> fs = vs ? Function(d.fragmentSource, error) : nil;
                    if (!vs || !fs)
                        return 0;
                    MTLRenderPipelineDescriptor* pd = [MTLRenderPipelineDescriptor new];
                    pd.label = [NSString stringWithUTF8String:ShaderProgramName(d.program)];
                    pd.vertexFunction = vs;
                    pd.fragmentFunction = fs;
                    pd.rasterSampleCount = (NSUInteger)d.samples;
                    MTLRenderPipelineColorAttachmentDescriptor* c = pd.colorAttachments[0];
                    c.pixelFormat = (MTLPixelFormat)d.pixelFormat;
                    c.writeMask = MTLColorWriteMaskAll;
                    c.blendingEnabled = d.blend.enabled;
                    c.sourceRGBBlendFactor = (MTLBlendFactor)d.blend.rgbSrc;
                    c.destinationRGBBlendFactor = (MTLBlendFactor)d.blend.rgbDst;
                    c.rgbBlendOperation = (MTLBlendOperation)d.blend.rgbOp;
                    c.sourceAlphaBlendFactor = (MTLBlendFactor)d.blend.alphaSrc;
                    c.destinationAlphaBlendFactor = (MTLBlendFactor)d.blend.alphaDst;
                    c.alphaBlendOperation = (MTLBlendOperation)d.blend.alphaOp;
                    if (d.uiVertexLayout)
                    {
                        MTLVertexDescriptor* vd = [MTLVertexDescriptor vertexDescriptor];
                        for (NSUInteger i = 0; i < 3; ++i)
                        {
                            vd.attributes[i].format = (MTLVertexFormat)kUiVertexAttributes[i].format;
                            vd.attributes[i].offset = kUiVertexAttributes[i].offset;
                            vd.attributes[i].bufferIndex = binding::kVertexBuffer;
                        }
                        vd.layouts[binding::kVertexBuffer].stride = kUiVertexStride;
                        vd.layouts[binding::kVertexBuffer].stepFunction = MTLVertexStepFunctionPerVertex;
                        vd.layouts[binding::kVertexBuffer].stepRate = 1;
                        pd.vertexDescriptor = vd;
                    }
                    NSError* err = nil;
                    id<MTLRenderPipelineState> pso = [device_ newRenderPipelineStateWithDescriptor:pd error:&err];
                    if (!pso)
                    {
                        error = err ? err.localizedDescription.UTF8String : "newRenderPipelineStateWithDescriptor failed";
                        return 0;
                    }
                    return Keep(psos_, pso);
                }
            }

            void ReleasePipeline(std::uint32_t pipeline) override { psos_.erase(pipeline); }

            // ---- command buffers
            bool BeginCommandBuffer(void* native, std::uint64_t serial, const std::shared_ptr<FrameTracker>& tracker) override
            {
                @autoreleasepool
                {
                    Prune();
                    id<MTLCommandBuffer> cb = native ? (__bridge id<MTLCommandBuffer>)native : [queue_ commandBuffer];
                    // a completed handler cannot be added after commit (Metal raises)
                    if (!cb || cb.status >= MTLCommandBufferStatusCommitted)
                        return false;
                    std::shared_ptr<FrameTracker> t = tracker;
                    [cb addCompletedHandler:^(id<MTLCommandBuffer>) {
                        t->Complete(serial);
                    }];
                    cb_ = cb;
                    own_ = native == nullptr;
                    inflight_[serial] = cb;
                    return true;
                }
            }

            void EndCommandBuffer() override
            {
                if (own_ && cb_)
                    [cb_ commit];
                cb_ = nil;
            }

            bool WaitForFrame(std::uint64_t serial) override
            {
                auto it = inflight_.find(serial);
                if (it == inflight_.end())
                    return true;
                id<MTLCommandBuffer> cb = it->second;
                if (cb.status < MTLCommandBufferStatusCommitted)
                    return false;
                [cb waitUntilCompleted];
                if (cb.status == MTLCommandBufferStatusError && log_)
                    log_(std::string("command buffer failed: ") + (cb.error ? cb.error.localizedDescription.UTF8String : "?"));
                inflight_.erase(it);
                return true;
            }

            // ---- encoders
            bool BeginRenderPass(const RenderPassDesc& d) override
            {
                @autoreleasepool
                {
                    MTLRenderPassDescriptor* rp = [MTLRenderPassDescriptor renderPassDescriptor];
                    MTLRenderPassColorAttachmentDescriptor* c = rp.colorAttachments[0];
                    c.texture = Tex(d.target);
                    c.loadAction = (MTLLoadAction)d.loadAction;
                    c.storeAction = (MTLStoreAction)d.storeAction;
                    c.clearColor = MTLClearColorMake(d.clearColor[0], d.clearColor[1], d.clearColor[2], d.clearColor[3]);
                    if (d.resolveTarget)
                        c.resolveTexture = Tex(d.resolveTarget);
                    if (d.samples.buffer)
                    {
                        if (@available(macOS 11.0, iOS 14.0, *))
                        {
                            MTLRenderPassSampleBufferAttachmentDescriptor* s = rp.sampleBufferAttachments[0];
                            s.sampleBuffer = Counters(d.samples.buffer);
                            s.startOfVertexSampleIndex = (NSUInteger)d.samples.index[0];
                            s.endOfVertexSampleIndex = (NSUInteger)d.samples.index[1];
                            s.startOfFragmentSampleIndex = (NSUInteger)d.samples.index[2];
                            s.endOfFragmentSampleIndex = (NSUInteger)d.samples.index[3];
                        }
                    }
                    render_ = [cb_ renderCommandEncoderWithDescriptor:rp];
                    if (!render_)
                        return false;
                    render_.label = Label(d.label);
                    return true;
                }
            }

            bool BeginBlitPass(const EncoderSamples& samples) override
            {
                @autoreleasepool
                {
                    if (samples.buffer)
                    {
                        if (@available(macOS 11.0, iOS 14.0, *))
                        {
                            MTLBlitPassDescriptor* bp = [MTLBlitPassDescriptor blitPassDescriptor];
                            MTLBlitPassSampleBufferAttachmentDescriptor* s = bp.sampleBufferAttachments[0];
                            s.sampleBuffer = Counters(samples.buffer);
                            s.startOfEncoderSampleIndex = (NSUInteger)samples.index[0];
                            s.endOfEncoderSampleIndex = (NSUInteger)samples.index[1];
                            blit_ = [cb_ blitCommandEncoderWithDescriptor:bp];
                        }
                    }
                    if (!blit_)
                        blit_ = [cb_ blitCommandEncoder];
                    return blit_ != nil;
                }
            }

            void EndEncoder() override
            {
                if (render_)
                    [render_ endEncoding];
                if (blit_)
                    [blit_ endEncoding];
                render_ = nil;
                blit_ = nil;
            }

            void* NativeRenderEncoder() override { return (__bridge void*)render_; }

            // ---- render encoder
            void SetPipeline(std::uint32_t pipeline) override { [render_ setRenderPipelineState:psos_[pipeline]]; }

            void SetViewport(int width, int height) override
            {
                const MTLViewport v = {0.0, 0.0, (double)width, (double)height, 0.0, 1.0};
                [render_ setViewport:v];
            }

            void SetScissor(const IRect& r) override
            {
                const MTLScissorRect s = {(NSUInteger)r.x0, (NSUInteger)r.y0, (NSUInteger)r.Width(), (NSUInteger)r.Height()};
                [render_ setScissorRect:s];
            }

            void SetBytes(std::uint32_t stages, std::uint32_t index, const void* data, std::uint32_t size) override
            {
                if (stages & StageVertex)
                    [render_ setVertexBytes:data length:size atIndex:index];
                if (stages & StageFragment)
                    [render_ setFragmentBytes:data length:size atIndex:index];
            }

            void SetBuffer(std::uint32_t stages, std::uint32_t index, std::uint32_t buffer, std::size_t offset) override
            {
                id<MTLBuffer> b = buffers_[buffer];
                if (stages & StageVertex)
                    [render_ setVertexBuffer:b offset:offset atIndex:index];
                if (stages & StageFragment)
                    [render_ setFragmentBuffer:b offset:offset atIndex:index];
            }

            void SetFragmentTexture(std::uint32_t index, std::uint32_t texture) override { [render_ setFragmentTexture:Tex(texture) atIndex:index]; }

            void SetFragmentSamplers() override
            {
                [render_ setFragmentSamplerState:linear_ atIndex:binding::kSamplerLinear];
                [render_ setFragmentSamplerState:point_ atIndex:binding::kSamplerPoint];
            }

            void Draw(std::uint32_t primitive, std::uint32_t first, std::uint32_t count, std::uint32_t instances) override
            {
                [render_ drawPrimitives:(MTLPrimitiveType)primitive vertexStart:first vertexCount:count instanceCount:instances];
            }

            void DrawIndexed(std::uint32_t count, std::uint32_t indexBuffer, std::size_t offset) override
            {
                [render_ drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                    indexCount:count
                                     indexType:MTLIndexTypeUInt32
                                   indexBuffer:buffers_[indexBuffer]
                             indexBufferOffset:offset];
            }

            // ---- blit encoder
            void CopyTexture(std::uint32_t src, const IRect& r, std::uint32_t dst, int dstX, int dstY) override
            {
                [blit_ copyFromTexture:Tex(src)
                           sourceSlice:0
                           sourceLevel:0
                          sourceOrigin:Origin(r.x0, r.y0)
                            sourceSize:Size(r)
                             toTexture:Tex(dst)
                      destinationSlice:0
                      destinationLevel:0
                     destinationOrigin:Origin(dstX, dstY)];
            }

            void CopyBufferToTexture(std::uint32_t buffer, std::size_t offset, std::size_t bytesPerRow, std::uint32_t dst, const IRect& r) override
            {
                [blit_ copyFromBuffer:buffers_[buffer]
                           sourceOffset:offset
                      sourceBytesPerRow:bytesPerRow
                    sourceBytesPerImage:bytesPerRow * (std::size_t)r.Height()
                             sourceSize:Size(r)
                              toTexture:Tex(dst)
                       destinationSlice:0
                       destinationLevel:0
                      destinationOrigin:Origin(r.x0, r.y0)];
            }

            void CopyTextureToBuffer(std::uint32_t src, const IRect& r, std::uint32_t buffer, std::size_t offset, std::size_t bytesPerRow) override
            {
                [blit_ copyFromTexture:Tex(src)
                                 sourceSlice:0
                                 sourceLevel:0
                                sourceOrigin:Origin(r.x0, r.y0)
                                  sourceSize:Size(r)
                                    toBuffer:buffers_[buffer]
                           destinationOffset:offset
                      destinationBytesPerRow:bytesPerRow
                    destinationBytesPerImage:bytesPerRow * (std::size_t)r.Height()];
            }

            // ---- timestamps
            std::uint32_t CreateCounterBuffer(std::uint32_t samples) override
            {
                if (@available(macOS 11.0, iOS 14.0, *))
                {
                    if (!timestampSet_)
                        return 0;
                    @autoreleasepool
                    {
                        MTLCounterSampleBufferDescriptor* d = [MTLCounterSampleBufferDescriptor new];
                        d.counterSet = timestampSet_;
                        d.storageMode = MTLStorageModeShared;
                        d.sampleCount = samples;
                        d.label = @"esia-timestamps";
                        NSError* err = nil;
                        id<MTLCounterSampleBuffer> b = [device_ newCounterSampleBufferWithDescriptor:d error:&err];
                        if (!b)
                        {
                            if (log_)
                                log_(std::string("counter sample buffer: ") + (err ? err.localizedDescription.UTF8String : "?"));
                            return 0;
                        }
                        return Keep(counters_, b);
                    }
                }
                return 0;
            }

            void ReleaseCounterBuffer(std::uint32_t buffer) override { counters_.erase(buffer); }

            bool ResolveCounters(std::uint32_t buffer, std::uint32_t first, std::uint32_t count, std::uint64_t* out) override
            {
                if (@available(macOS 11.0, iOS 14.0, *))
                {
                    @autoreleasepool
                    {
                        NSData* data = [Counters(buffer) resolveCounterRange:NSMakeRange(first, count)];
                        if (!data || data.length < count * sizeof(MTLCounterResultTimestamp))
                            return false;
                        std::memcpy(out, data.bytes, count * sizeof(MTLCounterResultTimestamp));
                        return true;
                    }
                }
                return false;
            }

            bool SampleClock(std::uint64_t& cpuNs, std::uint64_t& gpuTicks) override
            {
                MTLTimestamp cpu = 0, gpu = 0;
                [device_ sampleTimestamps:&cpu gpuTimestamp:&gpu];
                cpuNs = cpu;
                gpuTicks = gpu;
                return gpu != 0;
            }

        private:
            template <class Map, class T>
            std::uint32_t Keep(Map& map, T object)
            {
                const std::uint32_t id = next_++;
                map[id] = object;
                return id;
            }

            id<MTLTexture> Tex(std::uint32_t texture)
            {
                auto it = textures_.find(texture);
                return it != textures_.end() ? it->second : nil;
            }

            id<MTLCounterSampleBuffer> Counters(std::uint32_t buffer) API_AVAILABLE(macos(11.0), ios(14.0))
            {
                auto it = counters_.find(buffer);
                return it != counters_.end() ? it->second : nil;
            }

            // One library per MSL blob (the shader library's text lives for the program's lifetime): the fragment
            // and vertex stages of several programs share sources.
            id<MTLFunction> Function(const char* source, std::string& error)
            {
                id<MTLLibrary> lib = libraries_[source];
                if (!lib)
                {
                    MTLCompileOptions* o = [MTLCompileOptions new];
                    o.languageVersion = MTLLanguageVersion2_0;
                    // IEEE semantics like the other backends' compilers (the image goldens come from them)
#if (defined(__MAC_15_0) && __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_15_0) || (defined(__IPHONE_18_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_18_0)
                    if (@available(macOS 15.0, iOS 18.0, *))
                        o.mathMode = MTLMathModeSafe;
                    else
#endif
                    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                        o.fastMathEnabled = NO;
#pragma clang diagnostic pop
                    }
                    NSError* err = nil;
                    lib = [device_ newLibraryWithSource:[NSString stringWithUTF8String:source] options:o error:&err];
                    if (!lib)
                    {
                        error = err ? err.localizedDescription.UTF8String : "newLibraryWithSource failed";
                        return nil;
                    }
                    libraries_[source] = lib;
                }
                id<MTLFunction> f = [lib newFunctionWithName:@"esia_main"];
                if (!f)
                    error = "no esia_main in the MSL";
                return f;
            }

            // forget command buffers that finished (WaitForFrame then has nothing to wait for)
            void Prune()
            {
                for (auto it = inflight_.begin(); it != inflight_.end();)
                    it = it->second.status >= MTLCommandBufferStatusCompleted ? inflight_.erase(it) : std::next(it);
            }

            id<MTLDevice> device_;
            id<MTLCommandQueue> queue_;
            std::function<void(const std::string&)> log_;
            GpuCaps caps_;
            id<MTLSamplerState> linear_, point_;
            id<MTLCounterSet> timestampSet_ = nil;
            std::uint32_t next_ = 1;
            std::unordered_map<std::uint32_t, id<MTLTexture>> textures_;
            std::unordered_map<std::uint32_t, id<MTLBuffer>> buffers_;
            std::unordered_map<std::uint32_t, id<MTLRenderPipelineState>> psos_;
            std::unordered_map<std::uint32_t, id<MTLCounterSampleBuffer>> counters_;
            std::unordered_map<const char*, id<MTLLibrary>> libraries_;
            std::map<std::uint64_t, id<MTLCommandBuffer>> inflight_;
            id<MTLCommandBuffer> cb_;
            bool own_ = false;
            id<MTLRenderCommandEncoder> render_;
            id<MTLBlitCommandEncoder> blit_;
        };

        void LogToStderr(const std::string& what) { std::fprintf(stderr, "esia metal: %s\n", what.c_str()); }

        HeadlessDevice CreateHeadlessMetal(const HeadlessDesc& desc, std::string& error)
        {
            @autoreleasepool
            {
                id<MTLDevice> device = MTLCreateSystemDefaultDevice();
                if (!device)
                {
                    error = "no Metal device (MTLCreateSystemDefaultDevice returned nil)";
                    return {};
                }
                DeviceOptions o;
                // a switch for the first runs on a new GPU: timestamps are the least proven part (STATUS.md)
                o.timestamps = std::getenv("ESIA_METAL_NO_TIMESTAMPS") == nullptr;
                o.onError = &LogToStderr;
                return CreateHeadlessOn(std::make_unique<MetalGpu>(device, nil, &LogToStderr), desc, o, error);
            }
        }
    }

    std::unique_ptr<Device> CreateDevice(const Desc& desc)
    {
        if (!desc.device)
            return nullptr;
        @autoreleasepool
        {
            DeviceOptions o;
            o.timestamps = desc.timestamps;
            o.onError = desc.onError ? desc.onError : std::function<void(const std::string&)>(&LogToStderr);
            auto gpu = std::make_unique<MetalGpu>((__bridge id<MTLDevice>)desc.device, (__bridge id<MTLCommandQueue>)desc.commandQueue, o.onError);
            return std::make_unique<MetalDevice>(std::move(gpu), o);
        }
    }

    Texture WrapTexture(Device& device, void* texture)
    {
        auto* metal = dynamic_cast<MetalDevice*>(&device);
        return metal ? metal->WrapTexture(texture) : Texture{};
    }
}

void EsiaRegisterBackend_metal()
{
    esia::rhi::BackendInfo info;
    info.name = "metal";
    info.createHeadless = &esia::rhi::metal::CreateHeadlessMetal;
    esia::rhi::RegisterBackend(info);
}
