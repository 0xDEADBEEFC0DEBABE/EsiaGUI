// Esia - SVG to textures: the SVG that icons and illustrations are made of, parsed once and rasterized at the size it is
// drawn, kept in texture pages for Painter::Image.
//
//   esia::svg::Document icon = esia::svg::Document::Parse(text);   // once: the drawing, ready to rasterize
//   esia::svg::Atlas atlas(context.Textures());                     // once: pages in the TextureRegistry
//   atlas.BeginFrame();                                              // every frame, before drawing
//   esia::svg::Draw(painter, atlas, icon, rect, color);              // at the rectangle's size in pixels, cached
//
// Drawn: paths and the basic shapes (rect, circle, ellipse, line, polyline, polygon); fills with the non-zero and
// even-odd rules; strokes (width, caps, joins, miter limit, dashes); solid colors, currentColor, linear and radial
// gradients (units, transforms, spread methods, focal points); opacity (a group's in a layer of its own); clip paths;
// transforms, viewBox and preserveAspectRatio, nested svg, use and symbol; CSS in style attributes and in <style> sheets
// (type, class, id, descendant and child selectors).
// Not drawn: text, images, filters, masks, markers, patterns, animation, scripts and anything outside the file.
//
// Rasterizing: every pixel gets the exact area the drawing covers - where contours overlap, under either fill rule and
// where a stroke's pieces meet too: the sweep finds the outline of their union, nothing is approximated per pixel.
// Curves are flattened within 1/32 pixel; colors are composited the way browsers composite SVG (premultiplied, in sRGB).
#pragma once
#include "esia/base/hash.hpp"
#include "esia/base/math.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace esia
{
    class Painter;
    class TextureRegistry;
}

namespace esia::text
{
    class GlyphAtlas;
    struct GlyphSlot;
}

namespace esia::svg
{
    namespace detail
    {
        struct Tree;
    }

    // A parsed SVG file: immutable, cheap to copy (copies share the drawing), safe to rasterize from any thread.
    class ESIA_API Document
    {
    public:
        Document() = default;
        // SVG text (UTF-8). An empty document when it is not SVG (XML that does not parse, no <svg> root): `error` says
        // why. Parts it cannot draw are left out; a path's data is drawn up to an error in it, as browsers do.
        static Document Parse(std::string_view text, std::string* error = nullptr);
        static Document Load(const std::string& path, std::string* error = nullptr);

        bool Empty() const;
        // The size the file asks for in pixels (width / height; the viewBox's where they are missing or relative).
        Vec2 Size() const;
        // Every paint is currentColor (or none): rasterized once as a mask and tinted when drawn, in any color.
        bool Monochrome() const;
        // Whether currentColor paints anything (a rasterization depends on RasterOptions::currentColor).
        bool UsesCurrentColor() const;
        // The same for copies of one parse, different for every parse.
        std::uint64_t Id() const;

        const detail::Tree* Data() const { return tree_.get(); }

    private:
        std::shared_ptr<const detail::Tree> tree_;
    };

    struct RasterOptions
    {
        Color currentColor = Color::Black();   // what currentColor paints (straight alpha)
        bool mask = false;   // the drawing's alpha only, in white pixels (tinted when drawn): an icon in any color
        int border = 0;      // transparent texels around the drawing, colored like the texels next to them
        // Where the drawing goes, in pixels from the top-left corner of the width x height (inside the border): any
        // fraction of a pixel, nothing drawn outside it. Empty: all of width x height.
        Rect viewport;
    };

    // RGBA8 pixels: straight alpha, sRGB (TextureFormat::RGBA8), rows top first. Fully transparent texels next to the
    // drawing carry their neighbours' color, so bilinear filtering does not darken its edges.
    struct Image
    {
        int width = 0, height = 0;
        std::vector<std::uint8_t> pixels;
    };

    // `doc` in width x height pixels (and RasterOptions::border more on every side): its viewBox fitted by its
    // preserveAspectRatio into RasterOptions::viewport, or into all of them. false: an empty document, a size of 0 or
    // beyond 16384, or a viewport that is not finite.
    ESIA_API bool Rasterize(const Document& doc, int width, int height, const RasterOptions& options, Image& out);

    // Rasterized documents in RGBA8 textures of the TextureRegistry, cached by document, size, viewport, mode and (where
    // it paints something) currentColor.
    // A new drawing goes into the recent pages, and into the lasting ones when the next frame draws it too: the sizes and
    // places of an animation stay among the recent (drawn again when an animation comes back to them), and filling
    // those never makes what stays on screen be rasterized again. Each set has up to `maxPages` pages, made as they are
    // needed, recycled as text::GlyphAtlas's - when they are full the frame still gets more, and the next BeginFrame
    // starts over: the lasting pages only when what is drawn frame after frame no longer fits in them, the recent ones
    // whenever animations fill them. A drawing larger than a page gets a texture of its own, kept while it is drawn.
    // One per UI thread.
    class ESIA_API Atlas
    {
    public:
        explicit Atlas(TextureRegistry& textures, int pageSize = 1024, int maxPages = 4);
        ~Atlas();
        Atlas(const Atlas&) = delete;
        Atlas& operator=(const Atlas&) = delete;

        // Once per frame, before drawing.
        void BeginFrame();

        struct Entry
        {
            TextureId texture = 0;
            Vec2 uv0, uv1;   // the drawing's width x height texels
        };
        // `doc` at width x height pixels: from the cache, or rasterized now. Null for an empty document or what
        // Rasterize refuses. Valid until the next Get.
        const Entry* Get(const Document& doc, int width, int height, const RasterOptions& options = {});

        int Rasterized() const { return rasterized_; }   // how many times Get rasterized (the rest came from the cache)

    private:
        struct Own   // a drawing larger than a page
        {
            TextureId texture = 0;
            int width = 0, height = 0;   // with the border
            std::uint64_t frame = 0;     // the last frame it was drawn in
        };

        // a key the lasting pages do not have: from the last frame's pixels, the recent pages or rasterized now
        const Entry* Make(const Document& doc, int width, int height, const RasterOptions& options, std::uint64_t key);
        const Entry* Placed(const text::GlyphSlot& slot);
        void Recycle(Image& img);

        TextureRegistry& textures_;
        std::unique_ptr<text::GlyphAtlas> lasting_, recent_;   // drawings drawn frame after frame; the others
        std::unordered_map<std::uint64_t, Image, IntHash> made_, madeBefore_;   // the pixels of this / the last frame's new ones
        std::vector<std::vector<std::uint8_t>> spare_;   // pixel buffers to rasterize into again
        std::unordered_map<std::uint64_t, Own, IntHash> own_;
        int pageSize_ = 0;
        int rasterized_ = 0;
        std::uint64_t frame_ = 0;
        Entry entry_;
    };

    // Draws `doc` into `r` (UI units): the drawing rasterized at the size it is drawn (once: `atlas` keeps it) and drawn
    // as an image, its texels on the target's pixels. Outside a scale, `r` is rounded to whole physical pixels. Under
    // Painter::PushScale, the drawing goes exactly where the scale puts it (to 1/64 pixel), rasterized there - every
    // frame of a scale animation has the pixels of its own geometry, never a stretched image. `color` is currentColor,
    // and its alpha fades the whole drawing. A monochrome document (Document::Monochrome), or any with `mask`, is
    // rasterized once as a mask and drawn in `color`.
    ESIA_API void Draw(Painter& painter, Atlas& atlas, const Document& doc, const Rect& r, Color color = Color::Black(),
                       bool mask = false);
}
