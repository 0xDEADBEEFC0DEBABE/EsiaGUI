// Esia - the FreeType + HarfBuzz text system (target esia_text_ft, CMake options ESIA_TEXT_FREETYPE and
// ESIA_TEXT_DEPS): fonts from files or memory on every platform, the only text system on Linux / Android / consoles.
//
//   * shaping with HarfBuzz in design units (kerning, ligatures, marks, complex scripts), scaled to the size in
//     float: linear advances, the same layout at every pixel density (hinted advances on whole pixels instead with
//     TextRendering::hintedAdvances);
//   * per-character fallback through the fonts given to AddFallback, in order;
//   * greedy line breaking at spaces, after hyphens and between CJK characters (no break before closing / after
//     opening punctuation), character breaks for words longer than the line, ellipsis trimming, alignment;
//   * a uniform line box from the requested font (ascent + descent + line gap, baseline at ascent + gap / 2), so
//     fallback fonts never change the line height - WGT's DirectWrite layout does the same;
//   * glyphs from the outlines at the exact physical size - the design's (the default), or hinted there by FreeType
//     (TextRendering: light, the fonts' own hints with either TrueType interpreter, the autohinter) - through the
//     shared analytic rasterizer (glyph_raster.hpp) or FreeType's, in grayscale or per LCD stripe (sub-pixel pages:
//     TextureFlags_LcdCoverage, shapes and outlines in them as gray); 4 horizontal sub-pixel phases (whole pixels when
//     the hinting fits vertical stems too - the classic interpreter, the autohinter - or the advances are hinted),
//     baselines on whole pixels.
//
// Installed fonts and the platform's CJK fallback chain come from system_fonts.hpp (FindSystemFont, AddFallbackFonts).
//
// Color glyphs: bitmap strikes (CBDT, sbix) and COLR fonts, painted with their gradients and composite modes.
//
// Bitmap fonts (BDF, PCF, Windows FNT / FON, bitmap-only sfnt): drawn from their strikes pixel for pixel - a size
// takes the strike and whole multiple of it nearest in physical pixels (repeated pixels, no smoothing), its glyphs on
// whole pixels; its metrics and advances are the strike's, so Measure gives that size's box. A .fon's sizes of one
// family and style are one font. The bundled FreeType reads no compressed PCF (.pcf.gz: unpack it first).
//
// Not yet (docs/REWRITE.md, phase 2): the Unicode bidi algorithm - right-to-left runs are shaped and drawn right
// to left, but the runs of a line are laid out left to right.
#pragma once
#include "esia/text/text.hpp"
#include <memory>

namespace esia
{
    class TextureRegistry;
}

namespace esia::text
{
    struct FreeTypeDesc
    {
        int atlasPageSize = 2048;
        int atlasMaxPages = 4;   // before the glyph atlas starts over
        // How glyphs become pixels (TextRendering: hinting, the rasterizer); SetRendering changes it later. The
        // ESIA_TEXT_RENDERING environment variable, when set, overrides it (ParseTextRendering: "crisp", "light",
        // "hinting=full,interpreter=35" ...), so any program can be compared under another rendering.
        TextRendering rendering;
        // Distance fields (DrawShapes, large text): the fields of a text's new glyphs are made at once by this many
        // worker threads and the UI thread together (FieldMakerDesc::workers): -1 one per processor but one, at most
        // 15; 0 the UI thread alone. The same fields either way.
        int fieldThreads = -1;
        // A directory where the distance fields are kept between runs (FieldMakerDesc::cacheDirectory, UTF-8): a
        // later run reads them instead of making them again. Empty: none.
        std::string fieldCacheDirectory;
    };

    // The text system keeps its glyph pages in `textures`, which must outlive it. Null if FreeType cannot start.
    ESIA_API std::unique_ptr<TextSystem> CreateFreeTypeTextSystem(TextureRegistry& textures, const FreeTypeDesc& desc = {});
}
