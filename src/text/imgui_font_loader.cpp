// WGT UI - Dear ImGui font loader backed by the WGT DirectWrite engine.
//
// Installed as the atlas-level loader before any font is added, so stb_truetype is never used and no
// font file is ever read by Dear ImGui. Stock ImGui widgets (InputText, tables, the demo window...) get
// hinted DirectWrite glyphs, rasterized at the current render density, with per-character system fallback.
#include "text/imgui_font_loader.hpp"

namespace wgt
{
    namespace
    {
        CompatFontSource* Source(ImFontConfig* src) { return static_cast<CompatFontSource*>(src->FontLoaderData); }

        bool SrcInit(ImFontAtlas*, ImFontConfig* src)
        {
            // FontData carries our descriptor (not owned by the atlas)
            src->FontLoaderData = src->FontData;
            return src->FontLoaderData != nullptr;
        }

        void SrcDestroy(ImFontAtlas*, ImFontConfig* src) { src->FontLoaderData = nullptr; }

        bool SrcContainsGlyph(ImFontAtlas*, ImFontConfig* src, ImWchar codepoint)
        {
            CompatFontSource* s = Source(src);
            TextEngine::ResolvedGlyph g;
            return s && s->engine->ResolveCodepoint(s->font, 16.0f, codepoint, g);
        }

        bool BakedInit(ImFontAtlas*, ImFontConfig* src, ImFontBaked* baked, void*)
        {
            CompatFontSource* s = Source(src);
            if (!s)
                return false;
            if (!src->MergeMode)
            {
                float ascent = 0.0f, descent = 0.0f;
                s->engine->FontVerticalMetrics(s->font, baked->Size, ascent, descent);
                baked->Ascent = std::ceil(ascent);
                baked->Descent = -std::ceil(descent);
            }
            return true;
        }

        void BakedDestroy(ImFontAtlas*, ImFontConfig*, ImFontBaked*, void*) {}

        bool BakedLoadGlyph(ImFontAtlas* atlas, ImFontConfig* src, ImFontBaked* baked, void*, ImWchar codepoint, ImFontGlyph* out, float* outAdvance)
        {
            CompatFontSource* s = Source(src);
            if (!s)
                return false;
            TextEngine::ResolvedGlyph g;
            if (!s->engine->ResolveCodepoint(s->font, baked->Size, codepoint, g))
                return false;
            const float density = src->RasterizerDensity * baked->RasterizerDensity;
            const float advance = s->engine->GlyphAdvance(g.faceSlot, g.index, baked->Size);
            if (outAdvance)
            {
                *outAdvance = advance;
                return true;
            }
            GlyphBitmap bmp;
            if (!s->engine->RasterizeGlyph(g.faceSlot, g.index, baked->Size * density, 0, bmp))
                return false;
            out->Codepoint = codepoint;
            out->AdvanceX = advance;
            if (bmp.width > 0 && bmp.height > 0)
            {
                const ImFontAtlasRectId id = ImFontAtlasPackAddRect(atlas, bmp.width, bmp.height);
                if (id == ImFontAtlasRectId_Invalid)
                    return false;
                ImTextureRect* r = ImFontAtlasPackGetRect(atlas, id);
                const float inv = 1.0f / density;
                out->X0 = bmp.left * inv;
                out->Y0 = bmp.top * inv + baked->Ascent;
                out->X1 = (bmp.left + bmp.width) * inv;
                out->Y1 = (bmp.top + bmp.height) * inv + baked->Ascent;
                out->Visible = true;
                out->PackId = id;
                ImFontAtlasBakedSetFontGlyphBitmap(atlas, baked, src, out, r, bmp.pixels.data(), ImTextureFormat_Alpha8, bmp.width);
            }
            return true;
        }

        ImFontLoader MakeLoader()
        {
            ImFontLoader l;
            l.Name = "wgt_directwrite";
            l.FontSrcInit = SrcInit;
            l.FontSrcDestroy = SrcDestroy;
            l.FontSrcContainsGlyph = SrcContainsGlyph;
            l.FontBakedInit = BakedInit;
            l.FontBakedDestroy = BakedDestroy;
            l.FontBakedLoadGlyph = BakedLoadGlyph;
            l.FontBakedSrcLoaderDataSize = 0;
            return l;
        }
    }

    const ImFontLoader* GetDirectWriteFontLoader()
    {
        static const ImFontLoader loader = MakeLoader();
        return &loader;
    }

    ImFont* AddCompatFont(ImFontAtlas* atlas, CompatFontSource* source, const char* name)
    {
        ImFontConfig cfg;
        cfg.FontData = source;
        cfg.FontDataSize = (int)sizeof(CompatFontSource);
        cfg.FontDataOwnedByAtlas = false;
        cfg.OversampleH = cfg.OversampleV = 1;   // DirectWrite rasterizes at the exact density already
        std::snprintf(cfg.Name, sizeof(cfg.Name), "%s", name);
        return atlas->AddFont(&cfg);
    }
}
