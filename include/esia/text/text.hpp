// Esia - the text interface: what Painter and widgets need from a text stack.
//
// One interface, one implementation per platform family (docs/REWRITE.md, "Text"):
//   * FreeType + HarfBuzz (esia_text_ft, every platform; the only one on Linux / Android / consoles),
//   * DirectWrite on Windows and Core Text on Apple (later: system fallback fonts and the platform's rendering).
// A TextSystem shapes UTF-8 text, rasterizes glyphs into atlas pages it owns in the TextureRegistry and emits
// glyph quads into a DrawList (Geometry commands on those pages). Glyph pages are Alpha8 coverage, which the renderer
// recognizes from the page's TextureInfo and draws with its text pipeline, so text needs no special draw commands.
// How outlines become pixels - hinting, the rasterizer, grayscale or sub-pixel (LCD) antialiasing - is a text system's
// TextRendering (the design's outlines in grayscale unless asked otherwise); sub-pixel glyphs go to RGBA8 pages flagged
// TextureFlags_LcdCoverage, drawn with dual-source blending. Bitmap fonts draw their own pixels (freetype.hpp).
//
// Threading: a TextSystem belongs to one UI thread (like the Context that uses it).
#pragma once
#include "esia/base/math.hpp"
#include "esia/base/utf8.hpp"
#include "esia/core/fx.hpp"
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace esia
{
    class DrawList;
}

namespace esia::text
{
    struct GlyphSlot;
    struct GlyphBitmap;

    // Handle of a loaded font face (a file / memory blob + face index, or a system family). 0 = none.
    using FontId = std::uint32_t;

    // A font at a size in UI units (already multiplied by the metrics scale).
    struct FontRef
    {
        FontId id = 0;
        float size = 0.0f;
    };

    enum TextFlags_ : std::uint32_t
    {
        TextFlags_None = 0,
        TextFlags_Ellipsis = 1u << 0,      // trim with "…" when wider than the wrap width (single line)
        TextFlags_AlignCenter = 1u << 1,   // paragraph alignment of wrapped lines
        TextFlags_AlignRight = 1u << 2,
        // The text's origin on the pixel grid (the nearest pixel): a text drawn at a new place every frame (a label that
        // moves with what it names) reuses its placed glyphs instead of placing them again at each new sub-pixel offset,
        // and moves in whole pixels.
        TextFlags_PixelSnap = 1u << 3,
    };

    struct TextMetrics
    {
        Vec2 size;               // box of the laid-out text (UI units)
        float baseline = 0.0f;   // first baseline, from the top of the box
        int lines = 0;
        // The layout's identity in its text system: another text, font, size, wrap width or flags - or the same laid
        // out again - another one (0: none). What a caller keeps drawn from a text may stand for it.
        std::uint64_t layout = 0;
    };

    // What a text draw leaves in its text system besides its quads: smooth motion's record of where the text was drawn
    // (TextRendering::smoothMotion), which tells the next draw whether it moved. Kept while a list of them is set
    // (TextSystem::SetDrawNotes), so that a draw appended again from a record (DrawList::Replay) leaves it too
    // (TextSystem::ReplayDraws).
    struct DrawNote
    {
        std::uint64_t layout = 0;    // TextMetrics::layout
        Vec2 pos;
        float scale = 1.0f;
        float rest = 1.0f;           // how much it was drawn as at rest (below 1: moving, or settling after it moved)
        std::uint32_t outline = 0;   // 1: DrawOutline's
    };

    // A place a caret can stand in a line of text: a grapheme cluster boundary.
    struct CaretStop
    {
        std::uint32_t offset = 0;   // bytes into the UTF-8 text
        float x = 0.0f;             // where the caret is drawn, from the start of the line (UI units)
    };

    // An outline around text and icons (Painter::SetTextOutline, ui::ItemStyle::TextOutline): `width` UI units of
    // `color` all around every glyph, under it - the glyph's coverage grown by the width, round at its corners (up to
    // 32 physical pixels). The text's box does not grow: the outline reaches past it by the width. Color glyphs
    // (emoji) get none.
    struct TextOutline
    {
        float width = 0.0f;
        Color color = Color::Black();
        constexpr bool Visible() const { return width > 0.0f && color.a > 0.0f; }
    };

    // How glyph outlines become pixels (TextSystem::SetRendering; the FreeType text system also takes it from
    // FreeTypeDesc::rendering and the ESIA_TEXT_RENDERING environment variable). The default is the design's outline as
    // it is: no hinting, exact area coverage (Esia's analytic rasterizer), the pen at quarter pixels - shapes, weights
    // and spacing true to the font at every size and density, and one layout at every density. Hinting first fits the
    // outline to the pixel grid: crisper small text on a low-density display, at the cost of the design's exact shapes
    // (and, with hinted advances, of a layout independent of the density). Fonts drawn by the platform (Apple's Core
    // Text faces), color glyphs and bitmap fonts are not hinted.
    enum class Hinting : std::uint8_t
    {
        None,    // the design's outline
        Light,   // vertical only: baselines, x-heights and horizontal stems on whole pixels; widths and spacing unchanged
        Full,    // the font's own hints: its TrueType instructions (as `TrueTypeInterpreter` runs them), its CFF hints
        Auto,    // FreeType's autohinter, the font's own hints left aside (fonts with poor hints or none)
    };

    enum class TrueTypeInterpreter : std::uint8_t
    {
        V40,   // FreeType's default: the instructions' vertical part only, as fonts made for ClearType expect it
        V35,   // the classic interpreter, both directions: the shapes of black-and-white and grayscale GDI text
    };

    enum class GlyphRasterizer : std::uint8_t
    {
        Analytic,   // Esia's (glyph_raster.hpp): exact area coverage, the same whichever library read the font
        FreeType,   // FreeType's anti-aliasing rasterizer (the FreeType text system; others draw Analytic)
    };

    // Sub-pixel (LCD) antialiasing: each of an LCD pixel's three color stripes covered on its own, a third of a pixel's
    // resolution across - ClearType's look. It needs a panel with those stripes, text that is not rotated, and an
    // opaque target: the renderer draws sub-pixel pages with dual-source blending (Caps::dualSourceBlend) on the target,
    // and with their grayscale coverage inside glow layers, on targets RenderParams says are not opaque and on devices
    // without dual-source blending.
    enum class SubpixelLayout : std::uint8_t
    {
        None,   // grayscale antialiasing (OLED and high-density panels, rotated displays)
        Rgb,    // stripes red, green, blue from the left (most LCD monitors)
        Bgr,    // blue, green, red
    };

    // The filter across the stripes of sub-pixel coverage: less color fringing for a little blur (FreeType's filters;
    // the analytic rasterizer applies the same weights, glyph_raster.hpp).
    enum class LcdFilter : std::uint8_t
    {
        Default,   // FreeType's default five taps (8, 77, 86, 77, 8) / 256
        Light,     // three taps (85, 86, 85) / 256: sharper, a little more fringing
        None,      // each stripe its own coverage: the sharpest, strong fringes
    };

    struct TextRendering
    {
        Hinting hinting = Hinting::None;
        TrueTypeInterpreter interpreter = TrueTypeInterpreter::V40;   // Full hinting of TrueType fonts
        GlyphRasterizer rasterizer = GlyphRasterizer::Analytic;
        bool stemDarkening = false;    // FreeType's darkening of thin stems at small sizes (CFF fonts, the autohinter)
        // Full or Auto hinting: the layout takes the hinted advance widths, on whole pixels, instead of the design's -
        // as GDI-era text did. A text's width then depends on the density (Measure gives the width at the density of
        // the last NewFrame), and the pen stays on whole pixels.
        bool hintedAdvances = false;
        SubpixelLayout subpixel = SubpixelLayout::None;   // sub-pixel (LCD) antialiasing, for panels of that layout
        LcdFilter lcdFilter = LcdFilter::Default;
        // Text of this many physical pixels per em and more draws from distance fields (DrawShapes with its color):
        // one field per glyph for every size - large and animated text without a bitmap per size, sharp under any
        // scale. 0: never (small text is better hinted and covered per pixel).
        float distanceFieldFrom = 0.0f;
        // Text in motion - its position or scale changed since the last frame - is drawn from the design's outlines at
        // quarter pixels both ways, smooth however slowly it moves; when it comes to rest it fades into this rendering
        // (hinted, its baseline and maybe its pens on whole pixels) within 0.12 s (RasterParams::time; else 8 frames).
        // Hinting keeps text at rest crisp; in motion it would make it jump from pixel to pixel. A text is followed
        // from frame to frame by its layout and the order of its draws in the frame.
        bool smoothMotion = false;

        bool operator==(const TextRendering&) const = default;

        // The design's outlines (the default).
        static constexpr TextRendering Design() { return {}; }
        // Light hinting: sharper horizontals, the design's widths; smooth motion.
        static constexpr TextRendering Light()
        {
            TextRendering r;
            r.hinting = Hinting::Light;
            r.smoothMotion = true;
            return r;
        }
        // GDI-era sharpness: the fonts' own hints in both directions (the classic interpreter), FreeType's rasterizer,
        // hinted advances on whole pixels; smooth motion.
        static constexpr TextRendering Crisp()
        {
            TextRendering r;
            r.hinting = Hinting::Full;
            r.interpreter = TrueTypeInterpreter::V35;
            r.rasterizer = GlyphRasterizer::FreeType;
            r.hintedAdvances = true;
            r.smoothMotion = true;
            return r;
        }
        // ClearType as DirectWrite draws it: the fonts' own hints run vertically (v40), FreeType's rasterizer, sub-pixel
        // antialiasing for red-green-blue stripes with FreeType's default filter, the design's widths; smooth motion.
        static constexpr TextRendering ClearType()
        {
            TextRendering r;
            r.hinting = Hinting::Full;
            r.rasterizer = GlyphRasterizer::FreeType;
            r.subpixel = SubpixelLayout::Rgb;
            r.smoothMotion = true;
            return r;
        }
    };

    // What the platform's own text looks like: Windows - the fonts' hints with FreeType's default interpreter and
    // rasterizer, sub-pixel antialiasing where the user's font smoothing is ClearType (its stripe order); Linux - light
    // hinting, FreeType's rasterizer and stem darkening (the desktops' defaults); Apple and Android - the design's
    // outlines in grayscale.
    ESIA_API TextRendering NativeTextRendering();

    // A rendering from text, for command lines and environment variables: comma-separated presets and settings, later
    // ones overriding earlier ones -
    //   design | light | crisp | cleartype | native
    //   hinting=none|light|full|auto   interpreter=35|40   rasterizer=analytic|freetype
    //   stem-darkening[=0|1]   hinted-advances[=0|1]   subpixel=none|rgb|bgr   lcd-filter=default|light|none
    //   distance-field=<pixels per em, 0 = never>   smooth-motion[=0|1]
    // e.g. "crisp", "native,stem-darkening=0", "hinting=full,interpreter=35", "cleartype,subpixel=bgr". False (and `out`
    // unchanged) on a word it does not know, which `error` then names.
    ESIA_API bool ParseTextRendering(std::string_view spec, TextRendering& out, std::string* error = nullptr);

    // How glyphs are rasterized this frame.
    struct RasterParams
    {
        float pixelsPerUnit = 1.0f;   // render-target pixels per UI unit (metrics x render scale is already in the sizes)
        double time = -1.0;           // the frame clock, seconds (TextRendering::smoothMotion fades by it); < 0: by frames
    };

    class TextSystem
    {
    public:
        virtual ~TextSystem() = default;

        // ---- fonts
        // Loads a font file (TTF / OTF / TTC face `faceIndex`); 0 on failure. FreeType's convention: the face of a
        // collection in the low 16 bits of `faceIndex`, a variable font's named instance + 1 in the high 16 bits
        // ((3 << 16) | 0: the third named instance of the file's only face; 0 there: the default instance).
        virtual FontId AddFontFile(const char* path, int faceIndex = 0) = 0;
        // The data must stay alive as long as the TextSystem (bundled fonts in the executable).
        virtual FontId AddFontMemory(const void* data, std::size_t size, int faceIndex = 0) = 0;
        // Characters a font lacks are looked up in the fallback chain, in the order the fonts were added here.
        virtual void AddFallback(FontId font) = 0;

        // ---- frame
        // Call once per frame before drawing: sets the pixel density and grows / trims the glyph pages.
        virtual void NewFrame(const RasterParams& params) = 0;
        // How glyphs become pixels from the next NewFrame on (TextRendering); the glyph pages start over when it changes.
        // A text system without the choice keeps its own: this default ignores it.
        virtual void SetRendering(const TextRendering& rendering) { (void)rendering; }
        virtual TextRendering Rendering() const { return {}; }

        // ---- layout and drawing (UTF-8)
        virtual TextMetrics Measure(FontRef font, std::string_view text, float wrapWidth = 0.0f, std::uint32_t flags = 0) = 0;
        // Draws `text` with its box's top-left at `pos` and returns the box size at scale 1 (what Measure returns).
        // `scale` draws it scaled around `pos` (press / pop animations) by rasterizing at the scaled size instead
        // of stretching the bitmaps.
        virtual Vec2 Draw(DrawList& dl, FontRef font, Vec2 pos, Color color, std::string_view text, float wrapWidth = 0.0f,
                          std::uint32_t flags = 0, float scale = 1.0f) = 0;
        // One glyph (icon fonts) optically centered on `center`; `size` = em size in UI units.
        virtual void DrawGlyph(DrawList& dl, FontRef font, char32_t codepoint, Vec2 center, Color color) = 0;
        // The outline of what Draw / DrawGlyph with the same arguments draws (TextOutline), to be drawn just before it:
        // the same layout and placement, every glyph grown by the outline's width in its color. This default draws
        // none (a text system without outlines); the FreeType system draws them.
        virtual void DrawOutline(DrawList& dl, FontRef font, Vec2 pos, const TextOutline& outline, std::string_view text, float wrapWidth = 0.0f,
                                 std::uint32_t flags = 0, float scale = 1.0f)
        {
            (void)dl, (void)font, (void)pos, (void)outline, (void)text, (void)wrapWidth, (void)flags, (void)scale;
        }
        virtual void DrawGlyphOutline(DrawList& dl, FontRef font, char32_t codepoint, Vec2 center, const TextOutline& outline)
        {
            (void)dl, (void)font, (void)codepoint, (void)center, (void)outline;
        }
        // `text` as glyph shapes for the FX pipeline (fx::ShapeKind::Glyph): every glyph an instance made from
        // `material` - its material rows (paint, stroke, shadow, glow, glass, misc, mask, custom) and features
        // (flags[0]); the text system puts in the glyph's distance field (rect, page, uv, ranges), its kind, and
        // `weight` (UI units every glyph grows by, < 0 thins). The outer layers of all glyphs - drop shadow, outer glow,
        // a stroke outside - go first, then the faces of all glyphs, so no glyph's outer layer covers another's face.
        // Glyphs without outlines (color, bitmap fonts) draw as Draw draws them, in the fill's first color. Same
        // layout and result as Draw. This default (no distance fields) draws the fill's first color with Draw.
        virtual Vec2 DrawShapes(DrawList& dl, FontRef font, Vec2 pos, const fx::Instance& material, EffectId effect, float weight,
                                std::string_view text, float wrapWidth = 0.0f, std::uint32_t flags = 0, float scale = 1.0f)
        {
            (void)effect, (void)weight;
            const Color fill(material.fill0[0], material.fill0[1], material.fill0[2], material.fill0[3] * material.misc[0]);
            return Draw(dl, font, pos, fill, text, wrapWidth, flags, scale);
        }
        // Coverage tiles other than glyphs, in the texture text is drawn with (the corners of Painter's flat shapes, the
        // masks of SVG icons that last - drawn as images of it, DrawList::AddCoverageQuad - so they batch with the text)
        // and kept apart from the glyphs, under the caller's keys: the tile cached under `key`, else null; AddTile packs
        // one the caller rasterized (8-bit coverage). Gone when the glyph pages start over. Null from both: no tiles.
        virtual const GlyphSlot* FindTile(std::uint64_t key)
        {
            (void)key;
            return nullptr;
        }
        virtual const GlyphSlot* AddTile(std::uint64_t key, const GlyphBitmap& bitmap)
        {
            (void)key;
            (void)bitmap;
            return nullptr;
        }
        // Changes when the tiles found so far are gone (the pages started over), and differs between text systems (one
        // made where another was): a caller may keep their slots while it stays. 0: a text system without tiles.
        virtual std::uint64_t TileGeneration() { return 0; }

        // ---- draws done again (the widget layer's draw memo)
        // While `notes` is set (null: none), every draw that leaves a record behind appends its DrawNote there.
        virtual void SetDrawNotes(std::vector<DrawNote>* notes) { (void)notes; }
        // Leaves the records of `notes` as their draws would leave them now, when every one of them would be drawn as it
        // was - its layout still there, at rest, where it was: true. False (nothing changed): one of them would not.
        virtual bool ReplayDraws(std::span<const DrawNote> notes) { return notes.empty(); }
        // Whether ReplayDraws would (nothing changed)
        virtual bool CanReplayDraws(std::span<const DrawNote> notes) { return notes.empty(); }
        // The layouts (TextMetrics::layout) a caller measured the frame before and keeps using without measuring them
        // again: in use this frame, as if measured (a cache of unused layouts does not drop them).
        virtual void KeepLayouts(std::span<const std::uint64_t> layouts) { (void)layouts; }
        // Changes whenever a text drawn again the same way could come out otherwise: the glyph pages started over,
        // another rendering, another density, other fonts. 0: draws are not noted (they cannot be done again).
        virtual std::uint64_t DrawGeneration() { return 0; }

        // ---- editing
        // The caret stops of `text` laid out as one line (line breaks are not honored: single-line fields), in logical
        // order: the first at offset 0, the last at text.size(). x is where Draw puts the caret, so it decreases
        // through a right-to-left run; a ligature of several characters is split evenly. This default stops at every
        // code point and measures the text before it; the FreeType system stops at grapheme cluster boundaries.
        virtual void CaretStops(FontRef font, std::string_view text, std::vector<CaretStop>& out)
        {
            out.clear();
            for (std::size_t i = 0;;)
            {
                out.push_back({(std::uint32_t)i, i > 0 ? Measure(font, text.substr(0, i)).size.x : 0.0f});
                if (i >= text.size())
                    break;
                DecodeUtf8(text, i);
            }
        }
    };
}
