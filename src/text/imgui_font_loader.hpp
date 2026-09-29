// WGT UI - Dear ImGui font loader backed by the WGT DirectWrite engine
#pragma once
#include "text/text_engine.hpp"
#include <cmath>
#include <cstdio>

namespace wgt
{
    struct CompatFontSource
    {
        TextEngine* engine = nullptr;
        FontId font = 0;
    };

    // Loader to install with ImFontAtlas::SetFontLoader() before adding fonts.
    const ImFontLoader* GetDirectWriteFontLoader();
    // Adds an ImFont whose glyphs come from `source` (the descriptor must outlive the atlas).
    ImFont* AddCompatFont(ImFontAtlas* atlas, CompatFontSource* source, const char* name);
}
