// Esia - a MOCK of the Metal declarations metal_device.mm uses, for the offline Objective-C++ check on hosts without
// Apple's SDK (the esia_rhi_metal_objc_mock_check test, ../../CMakeLists.txt). Not Apple's header: written from Apple's documentation, with the enum values the backend
// believes are Apple's - so the static_asserts of metal_device.mm prove nothing here; they are the real check on
// macOS. What this mock lets clang verify: syntax, C++ / Objective-C types, ARC bridging, warnings (-Wpedantic ...).
// What it cannot: that Apple's selectors, property names and types are the ones declared here.
// UNVERIFIED: needs macOS - a Linux-only stand-in for Apple's header.
#pragma once
#import <Foundation/Foundation.h>

typedef NS_ENUM(NSUInteger, MTLPixelFormat){
    MTLPixelFormatInvalid = 0, MTLPixelFormatR8Unorm = 10, MTLPixelFormatRGBA8Unorm = 70, MTLPixelFormatRGBA8Unorm_sRGB = 71,
    MTLPixelFormatBGRA8Unorm = 80, MTLPixelFormatBGRA8Unorm_sRGB = 81, MTLPixelFormatRGB10A2Unorm = 90, MTLPixelFormatRGBA16Float = 115,
    MTLPixelFormatRGBA32Float = 125,
};
typedef NS_OPTIONS(NSUInteger, MTLTextureUsage){
    MTLTextureUsageUnknown = 0, MTLTextureUsageShaderRead = 1, MTLTextureUsageShaderWrite = 2, MTLTextureUsageRenderTarget = 4,
    MTLTextureUsagePixelFormatView = 0x10,
};
typedef NS_ENUM(NSUInteger, MTLTextureType){MTLTextureType2D = 2, MTLTextureType2DMultisample = 4};
typedef NS_ENUM(NSUInteger, MTLStorageMode){MTLStorageModeShared = 0, MTLStorageModeManaged = 1, MTLStorageModePrivate = 2};
typedef NS_OPTIONS(NSUInteger, MTLResourceOptions){MTLResourceStorageModeShared = 0};
typedef NS_ENUM(NSUInteger, MTLSamplerMinMagFilter){MTLSamplerMinMagFilterNearest = 0, MTLSamplerMinMagFilterLinear = 1};
typedef NS_ENUM(NSUInteger, MTLSamplerMipFilter){MTLSamplerMipFilterNotMipmapped = 0};
typedef NS_ENUM(NSUInteger, MTLSamplerAddressMode){MTLSamplerAddressModeClampToEdge = 0};
typedef NS_ENUM(NSUInteger, MTLBlendFactor){
    MTLBlendFactorZero = 0, MTLBlendFactorOne = 1, MTLBlendFactorSourceAlpha = 4, MTLBlendFactorOneMinusSourceAlpha = 5,
    MTLBlendFactorSource1Color = 15, MTLBlendFactorOneMinusSource1Color = 16, MTLBlendFactorSource1Alpha = 17, MTLBlendFactorOneMinusSource1Alpha = 18,
};
typedef NS_ENUM(NSUInteger, MTLBlendOperation){MTLBlendOperationAdd = 0};
typedef NS_OPTIONS(NSUInteger, MTLColorWriteMask){MTLColorWriteMaskAll = 0xf};
typedef NS_ENUM(NSUInteger, MTLVertexFormat){MTLVertexFormatUChar4Normalized = 9, MTLVertexFormatFloat2 = 29};
typedef NS_ENUM(NSUInteger, MTLVertexStepFunction){MTLVertexStepFunctionPerVertex = 1};
typedef NS_ENUM(NSUInteger, MTLLoadAction){MTLLoadActionDontCare = 0, MTLLoadActionLoad = 1, MTLLoadActionClear = 2};
typedef NS_ENUM(NSUInteger, MTLStoreAction){
    MTLStoreActionDontCare = 0, MTLStoreActionStore = 1, MTLStoreActionMultisampleResolve = 2, MTLStoreActionStoreAndMultisampleResolve = 3,
};
typedef NS_ENUM(NSUInteger, MTLPrimitiveType){MTLPrimitiveTypeTriangle = 3, MTLPrimitiveTypeTriangleStrip = 4};
typedef NS_ENUM(NSUInteger, MTLIndexType){MTLIndexTypeUInt16 = 0, MTLIndexTypeUInt32 = 1};
typedef NS_ENUM(NSUInteger, MTLCommandBufferStatus){
    MTLCommandBufferStatusNotEnqueued = 0, MTLCommandBufferStatusEnqueued = 1, MTLCommandBufferStatusCommitted = 2,
    MTLCommandBufferStatusScheduled = 3, MTLCommandBufferStatusCompleted = 4, MTLCommandBufferStatusError = 5,
};
typedef NS_ENUM(NSUInteger, MTLLanguageVersion){MTLLanguageVersion2_0 = (2 << 16)};
typedef NS_ENUM(NSInteger, MTLGPUFamily){MTLGPUFamilyApple3 = 1003, MTLGPUFamilyMac2 = 2002};
typedef NS_ENUM(NSUInteger, MTLCounterSamplingPoint){MTLCounterSamplingPointAtStageBoundary = 0};
typedef NS_ENUM(NSUInteger, MTLDataType){MTLDataTypeUInt = 33};

typedef struct { NSUInteger x, y, z; } MTLOrigin;
typedef struct { NSUInteger width, height, depth; } MTLSize;
inline MTLOrigin MTLOriginMake(NSUInteger x, NSUInteger y, NSUInteger z) { return MTLOrigin{x, y, z}; }
inline MTLSize MTLSizeMake(NSUInteger w, NSUInteger h, NSUInteger d) { return MTLSize{w, h, d}; }
typedef struct { double red, green, blue, alpha; } MTLClearColor;
inline MTLClearColor MTLClearColorMake(double r, double g, double b, double a) { return MTLClearColor{r, g, b, a}; }
typedef struct { double originX, originY, width, height, znear, zfar; } MTLViewport;
typedef struct { NSUInteger x, y, width, height; } MTLScissorRect;
typedef uint64_t MTLTimestamp;
typedef struct { uint64_t timestamp; } MTLCounterResultTimestamp;
static const NSUInteger MTLCounterDontSample = (NSUInteger)-1;
static const uint64_t MTLCounterErrorValue = ~0ULL;
extern NSString* const MTLCommonCounterSetTimestamp;

@protocol MTLTexture, MTLBuffer, MTLFunction, MTLCounterSampleBuffer, MTLCommandBuffer;

@protocol MTLResource <NSObject>
@property (copy) NSString* label;
@end
@protocol MTLTexture <MTLResource>
@property (readonly) NSUInteger width;
@property (readonly) NSUInteger height;
@property (readonly) MTLPixelFormat pixelFormat;
@property (readonly) NSUInteger sampleCount;
@property (readonly) MTLTextureUsage usage;
@property (readonly) MTLTextureType textureType;
@property (readonly) NSUInteger arrayLength;
@property (readonly, getter=isFramebufferOnly) BOOL framebufferOnly;
- (id<MTLTexture>)newTextureViewWithPixelFormat:(MTLPixelFormat)pixelFormat;
@end
@protocol MTLBuffer <MTLResource>
- (void*)contents;
@property (readonly) NSUInteger length;
@end
@protocol MTLFunction <NSObject>
@end
@interface MTLFunctionConstantValues : NSObject
- (void)setConstantValue:(const void*)value type:(MTLDataType)type atIndex:(NSUInteger)index;
@end
@protocol MTLLibrary <NSObject>
- (id<MTLFunction>)newFunctionWithName:(NSString*)name;
- (id<MTLFunction>)newFunctionWithName:(NSString*)name constantValues:(MTLFunctionConstantValues*)constantValues error:(NSError**)error;
@end
@protocol MTLRenderPipelineState <NSObject>
@end
@protocol MTLSamplerState <NSObject>
@end
@protocol MTLCounterSet <NSObject>
@property (readonly) NSString* name;
@end
@protocol MTLCounterSampleBuffer <NSObject>
- (NSData*)resolveCounterRange:(NSRange)range;
@end

@interface MTLTextureDescriptor : NSObject
@property (nonatomic) MTLTextureType textureType;
@property (nonatomic) MTLPixelFormat pixelFormat;
@property (nonatomic) NSUInteger width;
@property (nonatomic) NSUInteger height;
@property (nonatomic) NSUInteger mipmapLevelCount;
@property (nonatomic) NSUInteger sampleCount;
@property (nonatomic) MTLTextureUsage usage;
@property (nonatomic) MTLStorageMode storageMode;
@end
@interface MTLSamplerDescriptor : NSObject
@property (nonatomic) MTLSamplerMinMagFilter minFilter;
@property (nonatomic) MTLSamplerMinMagFilter magFilter;
@property (nonatomic) MTLSamplerMipFilter mipFilter;
@property (nonatomic) MTLSamplerAddressMode sAddressMode;
@property (nonatomic) MTLSamplerAddressMode tAddressMode;
@end
@interface MTLCompileOptions : NSObject
@property (nonatomic) MTLLanguageVersion languageVersion;
@property (nonatomic) BOOL fastMathEnabled __attribute__((deprecated("Use mathMode instead")));
@end
@interface MTLRenderPipelineColorAttachmentDescriptor : NSObject
@property (nonatomic) MTLPixelFormat pixelFormat;
@property (nonatomic, getter=isBlendingEnabled) BOOL blendingEnabled;
@property (nonatomic) MTLBlendFactor sourceRGBBlendFactor;
@property (nonatomic) MTLBlendFactor destinationRGBBlendFactor;
@property (nonatomic) MTLBlendOperation rgbBlendOperation;
@property (nonatomic) MTLBlendFactor sourceAlphaBlendFactor;
@property (nonatomic) MTLBlendFactor destinationAlphaBlendFactor;
@property (nonatomic) MTLBlendOperation alphaBlendOperation;
@property (nonatomic) MTLColorWriteMask writeMask;
@end
@interface MTLRenderPipelineColorAttachmentDescriptorArray : NSObject
- (MTLRenderPipelineColorAttachmentDescriptor*)objectAtIndexedSubscript:(NSUInteger)i;
@end
@interface MTLVertexAttributeDescriptor : NSObject
@property (nonatomic) MTLVertexFormat format;
@property (nonatomic) NSUInteger offset;
@property (nonatomic) NSUInteger bufferIndex;
@end
@interface MTLVertexAttributeDescriptorArray : NSObject
- (MTLVertexAttributeDescriptor*)objectAtIndexedSubscript:(NSUInteger)i;
@end
@interface MTLVertexBufferLayoutDescriptor : NSObject
@property (nonatomic) NSUInteger stride;
@property (nonatomic) MTLVertexStepFunction stepFunction;
@property (nonatomic) NSUInteger stepRate;
@end
@interface MTLVertexBufferLayoutDescriptorArray : NSObject
- (MTLVertexBufferLayoutDescriptor*)objectAtIndexedSubscript:(NSUInteger)i;
@end
@interface MTLVertexDescriptor : NSObject
+ (MTLVertexDescriptor*)vertexDescriptor;
@property (readonly) MTLVertexAttributeDescriptorArray* attributes;
@property (readonly) MTLVertexBufferLayoutDescriptorArray* layouts;
@end
@interface MTLRenderPipelineDescriptor : NSObject
@property (copy, nonatomic) NSString* label;
@property (nonatomic, strong) id<MTLFunction> vertexFunction;
@property (nonatomic, strong) id<MTLFunction> fragmentFunction;
@property (nonatomic) NSUInteger rasterSampleCount;
@property (readonly) MTLRenderPipelineColorAttachmentDescriptorArray* colorAttachments;
@property (copy, nonatomic) MTLVertexDescriptor* vertexDescriptor;
@end
@interface MTLRenderPassColorAttachmentDescriptor : NSObject
@property (nonatomic, strong) id<MTLTexture> texture;
@property (nonatomic, strong) id<MTLTexture> resolveTexture;
@property (nonatomic) MTLLoadAction loadAction;
@property (nonatomic) MTLStoreAction storeAction;
@property (nonatomic) MTLClearColor clearColor;
@end
@interface MTLRenderPassColorAttachmentDescriptorArray : NSObject
- (MTLRenderPassColorAttachmentDescriptor*)objectAtIndexedSubscript:(NSUInteger)i;
@end
@interface MTLRenderPassSampleBufferAttachmentDescriptor : NSObject
@property (nonatomic, strong) id<MTLCounterSampleBuffer> sampleBuffer;
@property (nonatomic) NSUInteger startOfVertexSampleIndex;
@property (nonatomic) NSUInteger endOfVertexSampleIndex;
@property (nonatomic) NSUInteger startOfFragmentSampleIndex;
@property (nonatomic) NSUInteger endOfFragmentSampleIndex;
@end
@interface MTLRenderPassSampleBufferAttachmentDescriptorArray : NSObject
- (MTLRenderPassSampleBufferAttachmentDescriptor*)objectAtIndexedSubscript:(NSUInteger)i;
@end
@interface MTLRenderPassDescriptor : NSObject
+ (MTLRenderPassDescriptor*)renderPassDescriptor;
@property (readonly) MTLRenderPassColorAttachmentDescriptorArray* colorAttachments;
@property (readonly) MTLRenderPassSampleBufferAttachmentDescriptorArray* sampleBufferAttachments;
@end
@interface MTLBlitPassSampleBufferAttachmentDescriptor : NSObject
@property (nonatomic, strong) id<MTLCounterSampleBuffer> sampleBuffer;
@property (nonatomic) NSUInteger startOfEncoderSampleIndex;
@property (nonatomic) NSUInteger endOfEncoderSampleIndex;
@end
@interface MTLBlitPassSampleBufferAttachmentDescriptorArray : NSObject
- (MTLBlitPassSampleBufferAttachmentDescriptor*)objectAtIndexedSubscript:(NSUInteger)i;
@end
@interface MTLBlitPassDescriptor : NSObject
+ (MTLBlitPassDescriptor*)blitPassDescriptor;
@property (readonly) MTLBlitPassSampleBufferAttachmentDescriptorArray* sampleBufferAttachments;
@end
@interface MTLCounterSampleBufferDescriptor : NSObject
@property (nonatomic, strong) id<MTLCounterSet> counterSet;
@property (copy, nonatomic) NSString* label;
@property (nonatomic) MTLStorageMode storageMode;
@property (nonatomic) NSUInteger sampleCount;
@end

@protocol MTLCommandEncoder <NSObject>
@property (copy) NSString* label;
- (void)endEncoding;
@end
@protocol MTLRenderCommandEncoder <MTLCommandEncoder>
- (void)setRenderPipelineState:(id<MTLRenderPipelineState>)state;
- (void)setViewport:(MTLViewport)viewport;
- (void)setScissorRect:(MTLScissorRect)rect;
- (void)setVertexBytes:(const void*)bytes length:(NSUInteger)length atIndex:(NSUInteger)index;
- (void)setFragmentBytes:(const void*)bytes length:(NSUInteger)length atIndex:(NSUInteger)index;
- (void)setVertexBuffer:(id<MTLBuffer>)buffer offset:(NSUInteger)offset atIndex:(NSUInteger)index;
- (void)setFragmentBuffer:(id<MTLBuffer>)buffer offset:(NSUInteger)offset atIndex:(NSUInteger)index;
- (void)setFragmentTexture:(id<MTLTexture>)texture atIndex:(NSUInteger)index;
- (void)setFragmentSamplerState:(id<MTLSamplerState>)sampler atIndex:(NSUInteger)index;
- (void)drawPrimitives:(MTLPrimitiveType)type vertexStart:(NSUInteger)start vertexCount:(NSUInteger)count instanceCount:(NSUInteger)instances;
- (void)drawIndexedPrimitives:(MTLPrimitiveType)type indexCount:(NSUInteger)count indexType:(MTLIndexType)indexType indexBuffer:(id<MTLBuffer>)buffer indexBufferOffset:(NSUInteger)offset;
@end
@protocol MTLBlitCommandEncoder <MTLCommandEncoder>
- (void)copyFromTexture:(id<MTLTexture>)src sourceSlice:(NSUInteger)ss sourceLevel:(NSUInteger)sl sourceOrigin:(MTLOrigin)so sourceSize:(MTLSize)size
              toTexture:(id<MTLTexture>)dst destinationSlice:(NSUInteger)ds destinationLevel:(NSUInteger)dl destinationOrigin:(MTLOrigin)dorig;
- (void)copyFromBuffer:(id<MTLBuffer>)src sourceOffset:(NSUInteger)off sourceBytesPerRow:(NSUInteger)bpr sourceBytesPerImage:(NSUInteger)bpi sourceSize:(MTLSize)size
             toTexture:(id<MTLTexture>)dst destinationSlice:(NSUInteger)ds destinationLevel:(NSUInteger)dl destinationOrigin:(MTLOrigin)dorig;
- (void)copyFromTexture:(id<MTLTexture>)src sourceSlice:(NSUInteger)ss sourceLevel:(NSUInteger)sl sourceOrigin:(MTLOrigin)so sourceSize:(MTLSize)size
               toBuffer:(id<MTLBuffer>)dst destinationOffset:(NSUInteger)off destinationBytesPerRow:(NSUInteger)bpr destinationBytesPerImage:(NSUInteger)bpi;
@end
typedef void (^MTLCommandBufferHandler)(id<MTLCommandBuffer>);
@protocol MTLCommandBuffer <NSObject>
@property (readonly) MTLCommandBufferStatus status;
@property (readonly) NSError* error;
- (void)addCompletedHandler:(MTLCommandBufferHandler)block;
- (void)commit;
- (void)waitUntilCompleted;
- (id<MTLRenderCommandEncoder>)renderCommandEncoderWithDescriptor:(MTLRenderPassDescriptor*)desc;
- (id<MTLBlitCommandEncoder>)blitCommandEncoder;
- (id<MTLBlitCommandEncoder>)blitCommandEncoderWithDescriptor:(MTLBlitPassDescriptor*)desc;
@end
@protocol MTLCommandQueue <NSObject>
- (id<MTLCommandBuffer>)commandBuffer;
@end
@protocol MTLDevice <NSObject>
- (id<MTLCommandQueue>)newCommandQueue;
- (id<MTLTexture>)newTextureWithDescriptor:(MTLTextureDescriptor*)desc;
- (id<MTLBuffer>)newBufferWithLength:(NSUInteger)length options:(MTLResourceOptions)options;
- (id<MTLSamplerState>)newSamplerStateWithDescriptor:(MTLSamplerDescriptor*)desc;
- (id<MTLLibrary>)newLibraryWithSource:(NSString*)source options:(MTLCompileOptions*)options error:(NSError**)error;
- (id<MTLRenderPipelineState>)newRenderPipelineStateWithDescriptor:(MTLRenderPipelineDescriptor*)desc error:(NSError**)error;
- (BOOL)supportsFamily:(MTLGPUFamily)family;
- (BOOL)supportsTextureSampleCount:(NSUInteger)count;
- (BOOL)supportsCounterSampling:(MTLCounterSamplingPoint)point;
@property (readonly) NSArray<id<MTLCounterSet>>* counterSets;
- (id<MTLCounterSampleBuffer>)newCounterSampleBufferWithDescriptor:(MTLCounterSampleBufferDescriptor*)desc error:(NSError**)error;
- (void)sampleTimestamps:(MTLTimestamp*)cpuTimestamp gpuTimestamp:(MTLTimestamp*)gpuTimestamp;
@end
extern "C" id<MTLDevice> MTLCreateSystemDefaultDevice(void);
