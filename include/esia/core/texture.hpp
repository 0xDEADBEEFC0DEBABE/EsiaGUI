// Esia - texture registry: the one place textures are created, updated and destroyed.
//
// Anything that needs pixels on the GPU (glyph atlases, images, icons) asks the registry for a TextureId and
// writes CPU pixels. The renderer collects the pending changes once per frame (TakeChanges) and applies them
// through the RHI, so nothing outside the renderer touches the GPU. Thread-safe: textures may be created or
// updated from any thread; a change becomes visible in the next frame that collects it.
#pragma once
#include "esia/base/math.hpp"
#include <mutex>
#include <unordered_map>
#include <vector>

namespace esia
{
    enum class TextureFormat : std::uint8_t
    {
        RGBA8,    // straight-alpha sRGB color (images)
        Alpha8,   // grayscale coverage (glyph atlas)
    };

    enum TextureFlags_ : std::uint32_t
    {
        TextureFlags_None = 0,
        // RGBA8 page of sub-pixel (LCD) coverage: one coverage per R/G/B stripe, grayscale coverage in A.
        // Drawn with the dual-source text pipeline (or its grayscale fallback).
        TextureFlags_LcdCoverage = 1u << 0,
    };

    struct TextureInfo
    {
        TextureFormat format = TextureFormat::RGBA8;
        int width = 0, height = 0;
        std::uint32_t flags = 0;
        bool Coverage() const { return format == TextureFormat::Alpha8 || (flags & TextureFlags_LcdCoverage); }
    };

    // One pending change, in the order it was requested.
    struct TextureChange
    {
        enum class Kind : std::uint8_t { Create, Update, Destroy };
        Kind kind = Kind::Create;
        TextureId id = 0;
        TextureInfo info;                       // Create
        int x = 0, y = 0, width = 0, height = 0;   // Create: whole texture; Update: the rectangle
        std::vector<std::uint8_t> pixels;       // tightly packed rows (width * bytes per pixel); empty = zeroed
    };

    inline int BytesPerPixel(TextureFormat f) { return f == TextureFormat::Alpha8 ? 1 : 4; }

    class ESIA_API TextureRegistry
    {
    public:
        // `pixels` may be null (zero-initialized); rowPitch 0 = tightly packed.
        TextureId Create(const TextureInfo& info, const void* pixels = nullptr, int rowPitch = 0);
        // Copies a rectangle of new pixels. Returns false for an unknown id or a rectangle outside the texture.
        bool Update(TextureId id, int x, int y, int width, int height, const void* pixels, int rowPitch = 0);
        void Destroy(TextureId id);

        bool Info(TextureId id, TextureInfo& out) const;
        std::size_t Count() const;

        // Renderer: moves the pending changes out (oldest first). Changes for a texture destroyed before it was
        // collected are dropped: the renderer never sees a texture that already went away.
        void TakeChanges(std::vector<TextureChange>& out);

    private:
        mutable std::mutex mutex_;
        TextureId next_ = 1;
        std::unordered_map<TextureId, TextureInfo> live_;
        std::vector<TextureChange> pending_;
    };
}
