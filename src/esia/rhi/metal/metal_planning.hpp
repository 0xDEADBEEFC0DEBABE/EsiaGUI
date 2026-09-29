// Esia - Metal backend: decisions that need no Apple header - pipeline keys, upload / readback layouts, pixel
// conversion for ReadPixels, and how a CopyTexture maps onto blits, views and resolves.
//
// Plain C++, unit-tested on every host (tests/test_metal_planning.cpp). UNVERIFIED on macOS: the alignment rules
// below are the conservative reading of Apple's blit documentation, not measured on a device.
#pragma once
#include "esia/rhi/rhi.hpp"
#include <cstddef>
#include <cstdint>
#include <string>

namespace esia::rhi::metal
{
    // ------------------------------------------------------------------ pipelines
    // What an MTLRenderPipelineState depends on. The topology is not part of it (Metal takes the primitive type at
    // draw time) and the vertex layout follows from the program, so two PipelineDescs that differ only there share
    // a pipeline state.
    struct PipelineKey
    {
        ShaderProgram program = ShaderProgram::UiGeometry;
        BlendMode blend = BlendMode::Straight;
        Format format = Format::RGBA8_UNORM;
        std::uint8_t samples = 1;
        bool operator==(const PipelineKey&) const = default;
    };

    std::uint32_t PackPipelineKey(const PipelineKey& k);   // unique per key (8 bits per field)
    struct PipelineKeyHash
    {
        std::size_t operator()(const PipelineKey& k) const;
    };

    // The programs whose vertex shader reads the UI vertex (stage_in attributes 0..2).
    bool ProgramUsesVertices(ShaderProgram p);

    // Checks a PipelineDesc against what the Metal backend builds and fills the key. False (with the reason) when
    // the RHI says to return an invalid pipeline: user effects (no runtime HLSL on Metal), dual-source blending
    // without the cap, a layout that does not fit the program, a format that is not a render target here.
    bool MakePipelineKey(const PipelineDesc& desc, const Caps& caps, PipelineKey& key, std::string& why);

    // ------------------------------------------------------------------ uploads and readback
    // Blits between buffers and textures: offsets and row pitches aligned to 256 bytes. Apple documents multiples
    // of the pixel size (and 4 bytes on some macOS GPUs); 256 satisfies every rule and costs a little staging
    // memory on narrow updates.
    constexpr std::size_t kBlitAlignment = 256;
    constexpr std::size_t AlignUp(std::size_t v, std::size_t a) { return (v + a - 1) / a * a; }

    struct BufferImageLayout
    {
        std::size_t rowBytes = 0;     // tightly packed row (width * bytes per pixel)
        std::size_t bytesPerRow = 0;  // pitch in the buffer
        std::size_t bytes = 0;        // bytesPerRow * height
    };
    BufferImageLayout LayoutOf(Format format, int width, int height);

    // Copies `rows` rows of `rowBytes` between two pitches (the staging copy of an upload, the readback rows).
    void CopyRows(void* dst, std::size_t dstPitch, const void* src, std::size_t srcPitch, std::size_t rowBytes, int rows);

    // ReadPixels: converts rows of `format` to RGBA8, top row first. sRGB is returned as stored (the bits), BGRA is
    // swizzled, floats are clamped to [0, 1] and rounded, R8 reads as (r, 0, 0, 255) like glReadPixels. False for
    // Format::Unknown.
    bool ConvertToRgba8(Format format, const void* src, std::size_t srcPitch, int width, int height, std::uint8_t* dst);
    float HalfToFloat(std::uint16_t h);

    // ------------------------------------------------------------------ texture copies
    // CopyTexture copies bits: dst has src's format or its RawFormat. A blit (copyFromTexture) wants identical pixel
    // formats, so when the twins differ one side is aliased through a view of the other's format - on whichever
    // texture has MTLTextureUsagePixelFormatView (the backend gives it to every texture it creates in such a
    // format; a host texture may lack it). A multisampled source is first resolved into a scratch texture of its
    // own format (a render pass with StoreAndMultisampleResolve), which the backend creates with PixelFormatView.
    struct CopySide
    {
        Format format = Format::Unknown;
        int samples = 1;
        bool viewable = false;   // has MTLTextureUsagePixelFormatView
        bool copyable = true;    // not framebufferOnly
    };

    enum class CopyView : std::uint8_t { None, Source, Destination };

    struct CopyPlan
    {
        bool valid = false;
        bool resolve = false;           // resolve the source into a scratch texture first (then blit from it)
        CopyView view = CopyView::None; // which side is aliased
        Format viewFormat = Format::Unknown;
        std::string why;                // when !valid
    };
    CopyPlan PlanCopy(const CopySide& src, const CopySide& dst);

    // True when a copy / readback of `rect` stays inside a width x height texture.
    bool RectInside(const IRect& r, int width, int height);
}
