// WGT UI - DirectWrite text engine.
//
//   Font resolution  : family name -> IDWriteFontFace (system collection + registered files),
//                      optical-size families (Small / Text / Display), per-character system fallback.
//   Shaping & layout : IDWriteTextLayout (kerning, ligatures, bidi, complex scripts, CJK line breaking),
//                      results cached as positioned glyph lists keyed by (text, font, size, width, flags).
//   Rasterization    : WGT's analytic rasterizer on the unhinted outline (exact area coverage, glyph_raster.cpp),
//                      at the exact physical pixel size (UI size x render scale), 4 horizontal sub-pixel phases.
//   Atlas            : WGT-owned Alpha8 pages uploaded through Dear ImGui's texture protocol.
//   Emission         : glyph quads snapped to the physical pixel grid, straight into ImDrawList.
//
// UI thread only (like Dear ImGui). The DirectWrite factory itself is shared and free-threaded.
#pragma once
#include "wgt/text.hpp"
#include "imgui_internal.h"

#include <windows.h>
#include <dwrite_3.h>
#include <wrl/client.h>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace wgt
{
    using Microsoft::WRL::ComPtr;

    struct TextEngineDesc
    {
        std::string fontFamily;         // empty = Segoe UI Variable optical family
        std::string fontFamilyDisplay;  // optional display family
        std::string monoFamily;
        std::string iconFamily;
        std::vector<std::wstring> fontFiles;
        std::string locale;
    };

    struct ShapedGlyph
    {
        std::uint16_t face = 0;    // index into the engine face table
        std::uint16_t index = 0;   // glyph index
        float x = 0.0f;            // pen position (UI units) relative to the layout origin
        float y = 0.0f;            // baseline (UI units) relative to the layout top
        float em = 0.0f;           // em size (UI units)
        std::uint32_t color = 0;   // color-font layer (emoji): IM_COL32 palette color; 0 = the text color
    };

    struct ShapedText
    {
        std::vector<ShapedGlyph> glyphs;
        TextMetrics metrics;
        std::uint64_t lastFrame = 0;
    };

    // Caret geometry of one line of text, for editing (UTF-16 offsets).
    struct CaretMap
    {
        std::vector<std::uint32_t> stops;   // caret stops = grapheme cluster boundaries, ascending, 0 .. length
        std::vector<float> x;               // caret x of every stop (UI units from the text origin, bidi-correct)
        std::vector<std::uint8_t> space;    // 1 when the cluster that starts at stops[i] is whitespace
        float width = 0.0f;
    };

    struct GlyphBitmap
    {
        int left = 0, top = 0, width = 0, height = 0;   // relative to the (integer) baseline origin
        std::vector<unsigned char> pixels;               // Alpha8, width * height
    };

    // How text is composed (the system's DirectWrite rendering parameters, i.e. the ClearType Text Tuner).
    struct TextRenderParams
    {
        float gamma = 1.8f;
        float grayscaleContrast = 1.0f;
        float clearTypeContrast = 0.5f;
        float clearTypeLevel = 1.0f;
        bool systemClearType = false;   // ClearType is enabled in Windows
        bool bgr = false;               // LCD stripes are B-G-R
    };

    class TextEngine
    {
    public:
        TextEngine();
        ~TextEngine();

        bool Init(const TextEngineDesc& desc);
        // Releases atlas pages (after the render backend destroyed their GPU copies).
        void Shutdown();

        void BeginFrame(std::uint64_t frame, float metricsScale, float renderScale);

        // Built-in fonts: FontWeight::Regular .. Mono map to ids 1..4, icons = kIconFont.
        static constexpr FontId kIconFont = 5;
        FontId RegisterFont(const std::string& family, int weight, bool italic);

        const ShapedText* Shape(FontRef font, const char* text, const char* end, float wrapWidth, std::uint32_t flags);
        // Draws shaped text with its top-left at `origin` (UI units). `scale` != 1 (press / appear animations)
        // re-rasterizes the glyphs at the scaled size: crisp at every frame instead of a stretched bitmap.
        void Draw(ImDrawList* dl, const ShapedText& shaped, Vec2 origin, ImU32 color, float scale = 1.0f);
        // Draws a single codepoint of the icon font, optically centered on `center`.
        bool DrawIcon(ImDrawList* dl, std::uint32_t codepoint, float size, Vec2 center, ImU32 color);
        // Caret stops and positions of a single line laid out exactly like Shape() lays it out.
        bool CaretGeometry(FontRef font, const std::wstring& text, CaretMap& out);

        // Runtime font files (any thread). Applied at the next BeginFrame (caches are rebuilt).
        void QueueFontFile(const std::wstring& path);
        bool HasPendingFontFiles() const;

        // --- Dear ImGui compatibility loader support
        struct ResolvedGlyph
        {
            IDWriteFontFace* face = nullptr;
            std::uint16_t index = 0;
            std::uint16_t faceSlot = 0;
        };
        bool ResolveCodepoint(FontId font, float size, std::uint32_t codepoint, ResolvedGlyph& out);
        float GlyphAdvance(std::uint16_t faceSlot, std::uint16_t glyph, float em) const;
        void FontVerticalMetrics(FontId font, float size, float& ascent, float& descent);
        bool RasterizeGlyph(std::uint16_t faceSlot, std::uint16_t glyph, float emPixels, int subpixel, GlyphBitmap& out);

        float RenderScale() const { return renderScale_; }
        const TextRenderParams& RenderParams() const { return params_; }
        // Sub-pixel (ClearType-style) glyphs for WGT text from now on (UI thread; both kinds stay cached).
        void SetSubpixel(bool enabled) { lcd_ = enabled; }
        bool Subpixel() const { return lcd_; }

    private:
        struct FontDef
        {
            std::vector<std::wstring> families;   // [0] default, optional optical variants below
            std::wstring smallFamily, displayFamily;         // optical-size families (may be empty)
            std::wstring requested;               // RegisterFont family not available yet (file still pending)
            DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL;
            DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL;
        };

        struct AtlasPage
        {
            ImTextureData* tex = nullptr;
            int shelfX = 0, shelfY = 0, shelfH = 0;
        };

        struct GlyphSlot
        {
            int page = -1;
            bool lcd = false;   // page index into lcdPages_ (RGBA sub-pixel coverage)
            float u0 = 0, v0 = 0, u1 = 0, v1 = 0;
            int left = 0, top = 0, width = 0, height = 0;
        };

        bool BuildCollection();
        const std::wstring& FamilyFor(const FontDef& def, float size) const;
        IDWriteTextFormat* Format(FontId font, float size);
        std::uint16_t FaceSlot(IDWriteFontFace* face);
        IDWriteFontFace* PrimaryFace(FontId font, float size);
        const GlyphSlot* Glyph(std::uint16_t faceSlot, std::uint16_t glyph, float emPixels, int subpixel);
        bool Pack(bool lcd, int w, int h, int& page, int& x, int& y);
        ImTextureData* PageTexture(const GlyphSlot& s) const { return (s.lcd ? lcdPages_ : pages_)[s.page].tex; }
        void ResetAtlas();
        void ApplyPendingFonts();
        bool FamilyExists(const std::wstring& family) const;

        // DirectWrite
        ComPtr<IDWriteFactory3> factory_;
        ComPtr<IDWriteFontCollection> collection_;
        ComPtr<IDWriteFontFallback> fallback_;
        ComPtr<IDWriteRenderingParams> renderingParams_;
        ComPtr<IDWriteTextAnalyzer> analyzer_;
        std::wstring locale_;
        std::vector<std::wstring> fontFiles_;
        mutable std::mutex pendingMutex_;
        std::vector<std::wstring> pendingFiles_;

        // registry
        std::vector<FontDef> fonts_;   // index = FontId - 1
        std::unordered_map<std::uint64_t, ComPtr<IDWriteTextFormat>> formats_;
        std::vector<ComPtr<IDWriteFontFace>> faces_;
        std::unordered_map<IDWriteFontFace*, std::uint16_t> faceIndex_;
        std::vector<std::uint32_t> faceUnitsPerEm_;
        std::unordered_map<std::uint64_t, std::uint16_t> primaryFaces_;          // (font, optical) -> face slot
        std::unordered_map<std::uint64_t, ResolvedGlyph> codepointCache_;        // (font, optical, cp)

        // shaping cache
        struct CacheEntry
        {
            std::string text;
            ShapedText shaped;
        };
        std::unordered_map<std::uint64_t, CacheEntry> shapeCache_;

        // glyph cache + atlas
        std::unordered_map<std::uint64_t, GlyphSlot> glyphs_;
        std::vector<AtlasPage> pages_;      // Alpha8: grayscale coverage
        std::vector<AtlasPage> lcdPages_;   // RGBA32: sub-pixel coverage (RGB) + grayscale (A)
        TextRenderParams params_;
        bool lcd_ = false;

        std::uint64_t frame_ = 0;
        float metricsScale_ = 1.0f;
        float renderScale_ = 1.0f;
        bool ready_ = false;
    };
}
