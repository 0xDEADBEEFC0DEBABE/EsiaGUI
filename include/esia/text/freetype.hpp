// Esia - the FreeType + HarfBuzz text system (target esia_text_ft, CMake option ESIA_TEXT_FREETYPE): fonts from
// files or memory on every platform, the only text system on Linux / Android / consoles.
//
//   * shaping with HarfBuzz in design units (kerning, ligatures, marks, complex scripts), scaled to the size in
//     float: unhinted, linear advances, the same layout at every pixel density;
//   * per-character fallback through the fonts given to AddFallback, in order;
//   * greedy line breaking at spaces, after hyphens and between CJK characters (no break before closing / after
//     opening punctuation), character breaks for words longer than the line, ellipsis trimming, alignment;
//   * a uniform line box from the requested font (ascent + descent + line gap, baseline at ascent + gap / 2), so
//     fallback fonts never change the line height - WGT's DirectWrite layout does the same;
//   * glyphs from the unhinted outlines (FreeType, design units) through the shared analytic rasterizer
//     (glyph_raster.hpp) at the exact physical size, 4 horizontal sub-pixel phases, baselines on whole pixels.
//
// Not yet (docs/REWRITE.md, phase 2): the Unicode bidi algorithm - right-to-left runs are shaped and drawn right
// to left, but the runs of a line are laid out left to right -, color glyphs (COLR / CBDT / sbix), system fonts.
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
    };

    // The text system keeps its glyph pages in `textures`, which must outlive it. Null if FreeType cannot start.
    ESIA_API std::unique_ptr<TextSystem> CreateFreeTypeTextSystem(TextureRegistry& textures, const FreeTypeDesc& desc = {});
}
