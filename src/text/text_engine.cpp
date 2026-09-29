// WGT UI - DirectWrite text engine implementation (see text_engine.hpp)
#include "text/text_engine.hpp"
#include "text/glyph_raster.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>

#pragma comment(lib, "dwrite.lib")

namespace wgt
{
    namespace
    {
        constexpr int kPageSize = 2048;
        constexpr int kMaxPages = 4;
        constexpr int kGlyphPadding = 1;

        std::wstring Widen(const char* s, const char* end = nullptr)
        {
            if (!s)
                return {};
            const int len = end ? (int)(end - s) : (int)std::strlen(s);
            if (len <= 0)
                return {};
            const int n = MultiByteToWideChar(CP_UTF8, 0, s, len, nullptr, 0);
            std::wstring w((size_t)n, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, s, len, w.data(), n);
            return w;
        }

        std::uint64_t Fnv1a(const void* data, size_t len, std::uint64_t h = 0xcbf29ce484222325ull)
        {
            const unsigned char* p = (const unsigned char*)data;
            for (size_t i = 0; i < len; ++i)
            {
                h ^= p[i];
                h *= 0x100000001b3ull;
            }
            return h;
        }

        std::uint32_t FloatBits(float f)
        {
            std::uint32_t u;
            std::memcpy(&u, &f, 4);
            return u;
        }

        // ---------------------------------------------------------------------------------------------
        // Collects the glyph runs of a laid-out IDWriteTextLayout (no drawing, just positions).
        class RunCollector final : public IDWriteTextRenderer
        {
        public:
            RunCollector(std::vector<ShapedGlyph>& out, std::function<std::uint16_t(IDWriteFontFace*)> faceSlot, IDWriteFactory2* factory)
                : out_(out), faceSlot_(std::move(faceSlot)), factory_(factory) {}

            // IUnknown (stack object: no ref counting)
            HRESULT __stdcall QueryInterface(REFIID riid, void** obj) override
            {
                if (riid == __uuidof(IDWriteTextRenderer) || riid == __uuidof(IDWritePixelSnapping) || riid == __uuidof(IUnknown))
                {
                    *obj = static_cast<IDWriteTextRenderer*>(this);
                    return S_OK;
                }
                *obj = nullptr;
                return E_NOINTERFACE;
            }
            ULONG __stdcall AddRef() override { return 1; }
            ULONG __stdcall Release() override { return 1; }

            // IDWritePixelSnapping: raw positions, WGT snaps to the physical grid itself at draw time
            HRESULT __stdcall IsPixelSnappingDisabled(void*, BOOL* disabled) override
            {
                *disabled = TRUE;
                return S_OK;
            }
            HRESULT __stdcall GetCurrentTransform(void*, DWRITE_MATRIX* m) override
            {
                *m = DWRITE_MATRIX{1, 0, 0, 1, 0, 0};
                return S_OK;
            }
            HRESULT __stdcall GetPixelsPerDip(void*, FLOAT* p) override
            {
                *p = 1.0f;
                return S_OK;
            }

            HRESULT __stdcall DrawGlyphRun(void*, FLOAT originX, FLOAT originY, DWRITE_MEASURING_MODE mode, const DWRITE_GLYPH_RUN* run,
                                           const DWRITE_GLYPH_RUN_DESCRIPTION* desc, IUnknown*) override
            {
                if (!run || !run->fontFace || run->glyphCount == 0)
                    return S_OK;
                // color fonts (emoji): COLR layers, each an ordinary outline glyph with a palette color
                ComPtr<IDWriteFontFace2> face2;
                if (factory_ && SUCCEEDED(run->fontFace->QueryInterface(IID_PPV_ARGS(&face2))) && face2->IsColorFont())
                {
                    ComPtr<IDWriteColorGlyphRunEnumerator> layers;
                    if (SUCCEEDED(factory_->TranslateColorGlyphRun(originX, originY, run, desc, mode, nullptr, 0, &layers)) && layers)
                    {
                        BOOL more = FALSE;
                        while (SUCCEEDED(layers->MoveNext(&more)) && more)
                        {
                            const DWRITE_COLOR_GLYPH_RUN* layer = nullptr;
                            if (FAILED(layers->GetCurrentRun(&layer)) || !layer)
                                break;
                            std::uint32_t color = 0;
                            if (layer->paletteIndex != 0xFFFF)
                            {
                                const auto ch = [](float v) { return (int)std::clamp(v * 255.0f + 0.5f, 0.0f, 255.0f); };
                                color = IM_COL32(ch(layer->runColor.r), ch(layer->runColor.g), ch(layer->runColor.b), std::max(1, ch(layer->runColor.a)));
                            }
                            Emit(layer->baselineOriginX, layer->baselineOriginY, &layer->glyphRun, color);
                        }
                        return S_OK;
                    }
                }
                Emit(originX, originY, run, 0);
                return S_OK;
            }

            void Emit(float originX, float originY, const DWRITE_GLYPH_RUN* run, std::uint32_t color)
            {
                const std::uint16_t slot = faceSlot_(run->fontFace);
                const bool rtl = (run->bidiLevel & 1) != 0;
                float pen = originX;
                for (UINT32 i = 0; i < run->glyphCount; ++i)
                {
                    const float adv = run->glyphAdvances ? run->glyphAdvances[i] : 0.0f;
                    const DWRITE_GLYPH_OFFSET off = run->glyphOffsets ? run->glyphOffsets[i] : DWRITE_GLYPH_OFFSET{0, 0};
                    ShapedGlyph g;
                    g.face = slot;
                    g.index = run->glyphIndices[i];
                    g.em = run->fontEmSize;
                    if (rtl)
                    {
                        pen -= adv;
                        g.x = pen - off.advanceOffset;
                    }
                    else
                    {
                        g.x = pen + off.advanceOffset;
                        pen += adv;
                    }
                    g.y = originY - off.ascenderOffset;
                    g.color = color;
                    out_.push_back(g);
                }
            }

            HRESULT __stdcall DrawUnderline(void*, FLOAT, FLOAT, const DWRITE_UNDERLINE*, IUnknown*) override { return S_OK; }
            HRESULT __stdcall DrawStrikethrough(void*, FLOAT, FLOAT, const DWRITE_STRIKETHROUGH*, IUnknown*) override { return S_OK; }
            HRESULT __stdcall DrawInlineObject(void* ctx, FLOAT x, FLOAT y, IDWriteInlineObject* obj, BOOL sideways, BOOL rtl, IUnknown* effect) override
            {
                // e.g. the ellipsis trimming sign: it draws itself through this renderer
                return obj ? obj->Draw(ctx, this, x, y, sideways, rtl, effect) : S_OK;
            }

        private:
            std::vector<ShapedGlyph>& out_;
            std::function<std::uint16_t(IDWriteFontFace*)> faceSlot_;
            IDWriteFactory2* factory_ = nullptr;
        };

        // Minimal analysis source for a single character (used for per-codepoint system fallback).
        class CharSource final : public IDWriteTextAnalysisSource
        {
        public:
            CharSource(const wchar_t* text, UINT32 len, const wchar_t* locale) : text_(text), len_(len), locale_(locale) {}
            HRESULT __stdcall QueryInterface(REFIID riid, void** obj) override
            {
                if (riid == __uuidof(IDWriteTextAnalysisSource) || riid == __uuidof(IUnknown))
                {
                    *obj = this;
                    return S_OK;
                }
                *obj = nullptr;
                return E_NOINTERFACE;
            }
            ULONG __stdcall AddRef() override { return 1; }
            ULONG __stdcall Release() override { return 1; }
            HRESULT __stdcall GetTextAtPosition(UINT32 pos, const WCHAR** text, UINT32* len) override
            {
                *text = pos < len_ ? text_ + pos : nullptr;
                *len = pos < len_ ? len_ - pos : 0;
                return S_OK;
            }
            HRESULT __stdcall GetTextBeforePosition(UINT32 pos, const WCHAR** text, UINT32* len) override
            {
                *text = (pos > 0 && pos <= len_) ? text_ : nullptr;
                *len = (pos > 0 && pos <= len_) ? pos : 0;
                return S_OK;
            }
            DWRITE_READING_DIRECTION __stdcall GetParagraphReadingDirection() override { return DWRITE_READING_DIRECTION_LEFT_TO_RIGHT; }
            HRESULT __stdcall GetLocaleName(UINT32, UINT32* len, const WCHAR** locale) override
            {
                *len = len_;
                *locale = locale_;
                return S_OK;
            }
            HRESULT __stdcall GetNumberSubstitution(UINT32, UINT32* len, IDWriteNumberSubstitution** sub) override
            {
                *len = len_;
                *sub = nullptr;
                return S_OK;
            }

        private:
            const wchar_t* text_;
            UINT32 len_;
            const wchar_t* locale_;
        };
    }

    TextEngine::TextEngine() = default;
    TextEngine::~TextEngine() = default;

    // ================================================================= setup
    bool TextEngine::Init(const TextEngineDesc& desc)
    {
        if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory3), reinterpret_cast<IUnknown**>(factory_.GetAddressOf()))))
            return false;

        if (!desc.locale.empty())
            locale_ = Widen(desc.locale.c_str());
        else
        {
            wchar_t name[LOCALE_NAME_MAX_LENGTH] = {};
            if (GetUserDefaultLocaleName(name, LOCALE_NAME_MAX_LENGTH) > 0)
                locale_ = name;
            else
                locale_ = L"en-US";
        }
        fontFiles_ = desc.fontFiles;
        if (!BuildCollection())
            return false;
        factory_->GetSystemFontFallback(&fallback_);
        factory_->CreateRenderingParams(&renderingParams_);
        // how Windows composes text for this user (ClearType Text Tuner): gamma, contrast, stripe order
        if (renderingParams_)
        {
            params_.gamma = std::clamp(renderingParams_->GetGamma(), 1.0f, 3.0f);
            params_.clearTypeContrast = std::clamp(renderingParams_->GetEnhancedContrast(), 0.0f, 2.0f);
            params_.clearTypeLevel = std::clamp(renderingParams_->GetClearTypeLevel(), 0.0f, 1.0f);
            params_.bgr = renderingParams_->GetPixelGeometry() == DWRITE_PIXEL_GEOMETRY_BGR;
            ComPtr<IDWriteRenderingParams1> rp1;
            if (SUCCEEDED(renderingParams_.As(&rp1)))
                params_.grayscaleContrast = std::clamp(rp1->GetGrayscaleEnhancedContrast(), 0.0f, 2.0f);
            BOOL smoothing = FALSE;
            UINT type = 0;
            SystemParametersInfoW(SPI_GETFONTSMOOTHING, 0, &smoothing, 0);
            SystemParametersInfoW(SPI_GETFONTSMOOTHINGTYPE, 0, &type, 0);
            params_.systemClearType = smoothing && type == FE_FONTSMOOTHINGCLEARTYPE &&
                                      renderingParams_->GetPixelGeometry() != DWRITE_PIXEL_GEOMETRY_FLAT;
        }

        // ---- built-in fonts (ids 1..5)
        FontDef text;
        if (!desc.fontFamily.empty() && FamilyExists(Widen(desc.fontFamily.c_str())))
        {
            text.families = {Widen(desc.fontFamily.c_str())};
            if (!desc.fontFamilyDisplay.empty() && FamilyExists(Widen(desc.fontFamilyDisplay.c_str())))
                text.displayFamily = Widen(desc.fontFamilyDisplay.c_str());
        }
        else if (FamilyExists(L"Segoe UI Variable Text"))
        {
            // Windows 11: optical sizes, the closest thing to SF Pro Text / Display on Windows
            text.families = {L"Segoe UI Variable Text"};
            if (FamilyExists(L"Segoe UI Variable Small"))
                text.smallFamily = L"Segoe UI Variable Small";
            if (FamilyExists(L"Segoe UI Variable Display"))
                text.displayFamily = L"Segoe UI Variable Display";
        }
        else
            text.families = {L"Segoe UI"};

        const DWRITE_FONT_WEIGHT weights[3] = {DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_WEIGHT_BOLD};
        for (DWRITE_FONT_WEIGHT w : weights)
        {
            FontDef d = text;
            d.weight = w;
            fonts_.push_back(d);
        }
        FontDef mono;
        if (!desc.monoFamily.empty() && FamilyExists(Widen(desc.monoFamily.c_str())))
            mono.families = {Widen(desc.monoFamily.c_str())};
        else
            mono.families = {FamilyExists(L"Cascadia Mono") ? L"Cascadia Mono" : L"Consolas"};
        fonts_.push_back(mono);
        FontDef icon;
        if (!desc.iconFamily.empty() && FamilyExists(Widen(desc.iconFamily.c_str())))
            icon.families = {Widen(desc.iconFamily.c_str())};
        else
            icon.families = {FamilyExists(L"Segoe Fluent Icons") ? L"Segoe Fluent Icons" : L"Segoe MDL2 Assets"};
        fonts_.push_back(icon);
        ready_ = true;
        return true;
    }

    bool TextEngine::BuildCollection()
    {
        collection_.Reset();
        if (fontFiles_.empty())
            return SUCCEEDED(factory_->GetSystemFontCollection(&collection_, FALSE));

        // system fonts + registered files in one collection
        ComPtr<IDWriteFactory5> f5;
        ComPtr<IDWriteFontSetBuilder1> builder;
        ComPtr<IDWriteFontSet> systemSet;
        if (FAILED(factory_.As(&f5)) || FAILED(f5->CreateFontSetBuilder(&builder)) || FAILED(factory_->GetSystemFontSet(&systemSet)))
            return SUCCEEDED(factory_->GetSystemFontCollection(&collection_, FALSE));
        for (const std::wstring& path : fontFiles_)
        {
            ComPtr<IDWriteFontFile> file;
            if (SUCCEEDED(f5->CreateFontFileReference(path.c_str(), nullptr, &file)))
                builder->AddFontFile(file.Get());
        }
        builder->AddFontSet(systemSet.Get());
        ComPtr<IDWriteFontSet> set;
        ComPtr<IDWriteFontCollection1> coll;
        if (FAILED(builder->CreateFontSet(&set)) || FAILED(f5->CreateFontCollectionFromFontSet(set.Get(), &coll)))
            return SUCCEEDED(factory_->GetSystemFontCollection(&collection_, FALSE));
        collection_ = coll;
        return true;
    }

    bool TextEngine::FamilyExists(const std::wstring& family) const
    {
        if (!collection_ || family.empty())
            return false;
        UINT32 index = 0;
        BOOL exists = FALSE;
        return SUCCEEDED(collection_->FindFamilyName(family.c_str(), &index, &exists)) && exists;
    }

    void TextEngine::QueueFontFile(const std::wstring& path)
    {
        std::lock_guard lock(pendingMutex_);
        pendingFiles_.push_back(path);
    }

    bool TextEngine::HasPendingFontFiles() const
    {
        std::lock_guard lock(pendingMutex_);
        return !pendingFiles_.empty();
    }

    void TextEngine::ApplyPendingFonts()
    {
        std::vector<std::wstring> files;
        {
            std::lock_guard lock(pendingMutex_);
            files.swap(pendingFiles_);
        }
        if (files.empty())
            return;
        for (auto& f : files)
            fontFiles_.push_back(f);
        BuildCollection();
        // fonts registered before their file was applied switch to it now (same FontId)
        for (FontDef& d : fonts_)
            if (!d.requested.empty() && FamilyExists(d.requested))
            {
                d.families = {d.requested};
                d.requested.clear();
            }
        // families may resolve differently now: drop everything derived from the old collection
        formats_.clear();
        primaryFaces_.clear();
        codepointCache_.clear();
        shapeCache_.clear();
    }

    void TextEngine::Shutdown()
    {
        for (std::vector<AtlasPage>* set : {&pages_, &lcdPages_})
        {
            for (AtlasPage& p : *set)
            {
                if (!p.tex)
                    continue;
                if (ImGui::GetCurrentContext())
                    ImGui::UnregisterUserTexture(p.tex);
                IM_DELETE(p.tex);
            }
            set->clear();
        }
        glyphs_.clear();
        shapeCache_.clear();
        formats_.clear();
        codepointCache_.clear();
        primaryFaces_.clear();
        faceIndex_.clear();
        faces_.clear();
        fallback_.Reset();
        collection_.Reset();
        renderingParams_.Reset();
        factory_.Reset();
        ready_ = false;
    }

    void TextEngine::BeginFrame(std::uint64_t frame, float metricsScale, float renderScale)
    {
        frame_ = frame;
        metricsScale_ = metricsScale > 0.0f ? metricsScale : 1.0f;
        renderScale_ = renderScale > 0.0f ? renderScale : 1.0f;
        ApplyPendingFonts();
        if ((frame % 120) == 0)
        {
            for (auto it = shapeCache_.begin(); it != shapeCache_.end();)
            {
                if (frame_ - it->second.shaped.lastFrame > 300)
                    it = shapeCache_.erase(it);
                else
                    ++it;
            }
        }
    }

    // ============================================================== fonts
    FontId TextEngine::RegisterFont(const std::string& family, int weight, bool italic)
    {
        FontDef d;
        const std::wstring w = Widen(family.c_str());
        // A family that is not in the collection yet (e.g. its file was just queued with AddFontFile) renders
        // with the UI family until the file is applied at the next frame, then switches to it.
        const bool available = FamilyExists(w) || HasPendingFontFiles();
        if (FamilyExists(w))
            d.families = {w};
        else
        {
            d.families = {fonts_[0].families[0]};
            if (available)
                d.requested = w;
        }
        d.weight = (DWRITE_FONT_WEIGHT)std::clamp(weight, 1, 999);
        d.style = italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
        for (size_t i = 0; i < fonts_.size(); ++i)
            if (fonts_[i].families == d.families && fonts_[i].requested == d.requested && fonts_[i].weight == d.weight && fonts_[i].style == d.style &&
                fonts_[i].smallFamily.empty() && fonts_[i].displayFamily.empty())
                return (FontId)(i + 1);
        fonts_.push_back(d);
        return (FontId)fonts_.size();
    }

    const std::wstring& TextEngine::FamilyFor(const FontDef& def, float size) const
    {
        // optical size is chosen on the unscaled (point-like) size so it follows the design, not the DPI
        const float optical = size / metricsScale_;
        if (!def.smallFamily.empty() && optical < 12.5f)
            return def.smallFamily;
        if (!def.displayFamily.empty() && optical >= 19.5f)
            return def.displayFamily;
        return def.families[0];
    }

    std::uint16_t TextEngine::FaceSlot(IDWriteFontFace* face)
    {
        auto it = faceIndex_.find(face);
        if (it != faceIndex_.end())
            return it->second;
        const std::uint16_t slot = (std::uint16_t)faces_.size();
        faces_.emplace_back(face);
        DWRITE_FONT_METRICS fm;
        face->GetMetrics(&fm);
        faceUnitsPerEm_.push_back(fm.designUnitsPerEm ? fm.designUnitsPerEm : 2048);
        faceIndex_[face] = slot;
        return slot;
    }

    IDWriteFontFace* TextEngine::PrimaryFace(FontId font, float size)
    {
        if (font == 0 || font > fonts_.size())
            font = 1;
        const FontDef& def = fonts_[font - 1];
        const std::wstring& family = FamilyFor(def, size);
        const std::uint64_t key = ((std::uint64_t)font << 32) ^ Fnv1a(family.data(), family.size() * 2);
        auto it = primaryFaces_.find(key);
        if (it != primaryFaces_.end())
            return faces_[it->second].Get();
        UINT32 index = 0;
        BOOL exists = FALSE;
        ComPtr<IDWriteFontFamily> fam;
        ComPtr<IDWriteFont> f;
        ComPtr<IDWriteFontFace> face;
        if (FAILED(collection_->FindFamilyName(family.c_str(), &index, &exists)) || !exists)
            collection_->FindFamilyName(L"Segoe UI", &index, &exists);
        if (FAILED(collection_->GetFontFamily(index, &fam)) || FAILED(fam->GetFirstMatchingFont(def.weight, DWRITE_FONT_STRETCH_NORMAL, def.style, &f)) ||
            FAILED(f->CreateFontFace(&face)))
            return nullptr;
        const std::uint16_t slot = FaceSlot(face.Get());
        primaryFaces_[key] = slot;
        return faces_[slot].Get();
    }

    IDWriteTextFormat* TextEngine::Format(FontId font, float size)
    {
        if (font == 0 || font > fonts_.size())
            font = 1;
        const std::uint64_t key = ((std::uint64_t)font << 32) | FloatBits(size);
        auto it = formats_.find(key);
        if (it != formats_.end())
            return it->second.Get();
        const FontDef& def = fonts_[font - 1];
        ComPtr<IDWriteTextFormat> fmt;
        if (FAILED(factory_->CreateTextFormat(FamilyFor(def, size).c_str(), collection_.Get(), def.weight, def.style, DWRITE_FONT_STRETCH_NORMAL,
                                              size, locale_.c_str(), &fmt)))
            return nullptr;
        // Uniform line box from the primary face, so fallback fonts (e.g. CJK with taller metrics) never
        // change the line height of mixed-script text.
        if (IDWriteFontFace* face = PrimaryFace(font, size))
        {
            DWRITE_FONT_METRICS fm;
            face->GetMetrics(&fm);
            const float k = size / (float)(fm.designUnitsPerEm ? fm.designUnitsPerEm : 2048);
            const float ascent = fm.ascent * k, descent = fm.descent * k, gap = fm.lineGap * k;
            fmt->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, ascent + descent + gap, ascent + gap * 0.5f);
        }
        formats_[key] = fmt;
        return fmt.Get();
    }

    // ============================================================== shaping
    const ShapedText* TextEngine::Shape(FontRef font, const char* text, const char* end, float wrapWidth, std::uint32_t flags)
    {
        if (!ready_ || !text)
            return nullptr;
        if (!end)
            end = text + std::strlen(text);
        const size_t len = (size_t)(end - text);
        std::uint64_t key = Fnv1a(text, len);
        key = Fnv1a(&font.id, sizeof(font.id), key);
        const std::uint32_t sizeBits = FloatBits(font.size), wrapBits = FloatBits(wrapWidth);
        key = Fnv1a(&sizeBits, 4, key);
        key = Fnv1a(&wrapBits, 4, key);
        key = Fnv1a(&flags, 4, key);
        const std::uint32_t optical = FloatBits(metricsScale_);
        key = Fnv1a(&optical, 4, key);

        auto it = shapeCache_.find(key);
        if (it != shapeCache_.end() && it->second.text.size() == len && std::memcmp(it->second.text.data(), text, len) == 0)
        {
            it->second.shaped.lastFrame = frame_;
            return &it->second.shaped;
        }

        CacheEntry entry;
        entry.text.assign(text, len);
        entry.shaped.lastFrame = frame_;
        const std::wstring w = Widen(text, end);
        IDWriteTextFormat* fmt = Format(font.id, font.size);
        if (!fmt)
            return nullptr;
        const bool ellipsis = (flags & TextFlags_Ellipsis) != 0 && wrapWidth > 0.0f;
        const float maxW = wrapWidth > 0.0f ? wrapWidth : 100000.0f;
        ComPtr<IDWriteTextLayout> layout;
        if (FAILED(factory_->CreateTextLayout(w.c_str(), (UINT32)w.size(), fmt, maxW, 100000.0f, &layout)))
            return nullptr;
        layout->SetWordWrapping((wrapWidth > 0.0f && !ellipsis) ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP);
        if (flags & TextFlags_AlignCenter)
            layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        else if (flags & TextFlags_AlignRight)
            layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
        if (ellipsis)
        {
            DWRITE_TRIMMING trim = {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
            ComPtr<IDWriteInlineObject> sign;
            factory_->CreateEllipsisTrimmingSign(layout.Get(), &sign);
            layout->SetTrimming(&trim, sign.Get());
        }

        RunCollector collector(entry.shaped.glyphs, [this](IDWriteFontFace* f) { return FaceSlot(f); }, factory_.Get());
        layout->Draw(nullptr, &collector, 0.0f, 0.0f);

        DWRITE_TEXT_METRICS m = {};
        layout->GetMetrics(&m);
        float width = m.widthIncludingTrailingWhitespace;
        if (wrapWidth > 0.0f)
            width = std::min(width, wrapWidth);
        if (!(flags & (TextFlags_AlignCenter | TextFlags_AlignRight)))
            width = std::max(width, m.left + m.width);
        entry.shaped.metrics.size = Vec2(width, m.height);
        entry.shaped.metrics.lines = (int)m.lineCount;
        DWRITE_LINE_METRICS lm;
        UINT32 lineCount = 0;
        if (SUCCEEDED(layout->GetLineMetrics(&lm, 1, &lineCount)) || lineCount > 0)
            entry.shaped.metrics.baseline = lm.baseline;

        auto res = shapeCache_.insert_or_assign(key, std::move(entry));
        return &res.first->second.shaped;
    }

    // ============================================================== editing
    bool TextEngine::CaretGeometry(FontRef font, const std::wstring& text, CaretMap& out)
    {
        out = {};
        if (!ready_)
            return false;
        IDWriteTextFormat* fmt = Format(font.id, font.size);
        if (!fmt)
            return false;
        ComPtr<IDWriteTextLayout> layout;
        if (FAILED(factory_->CreateTextLayout(text.c_str(), (UINT32)text.size(), fmt, 100000.0f, 100000.0f, &layout)))
            return false;
        layout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        UINT32 count = 0;
        layout->GetClusterMetrics(nullptr, 0, &count);
        std::vector<DWRITE_CLUSTER_METRICS> clusters(count);
        if (count > 0 && FAILED(layout->GetClusterMetrics(clusters.data(), count, &count)))
            return false;
        std::uint32_t pos = 0;
        for (const DWRITE_CLUSTER_METRICS& c : clusters)
        {
            out.stops.push_back(pos);
            out.space.push_back(c.isWhitespace ? 1 : 0);
            pos += c.length;
        }
        out.stops.push_back(pos);
        out.space.push_back(0);
        out.x.resize(out.stops.size(), 0.0f);
        for (size_t i = 0; i < out.stops.size(); ++i)
        {
            FLOAT x = 0.0f, y = 0.0f;
            DWRITE_HIT_TEST_METRICS hm = {};
            if (out.stops[i] < pos)
                layout->HitTestTextPosition(out.stops[i], FALSE, &x, &y, &hm);
            else if (pos > 0)
                layout->HitTestTextPosition(pos - 1, TRUE, &x, &y, &hm);
            out.x[i] = x;
        }
        DWRITE_TEXT_METRICS m = {};
        layout->GetMetrics(&m);
        out.width = m.widthIncludingTrailingWhitespace;
        return true;
    }

    // ======================================================== rasterization
    bool TextEngine::RasterizeGlyph(std::uint16_t faceSlot, std::uint16_t glyph, float emPixels, int subpixel, GlyphBitmap& out)
    {
        out = {};
        if (faceSlot >= faces_.size() || emPixels <= 0.0f)
            return false;
        // WGT's own rasterizer: unhinted outline, exact area coverage (glyph_raster.cpp)
        if (RasterizeGlyphOutline(faces_[faceSlot].Get(), glyph, emPixels, 0.25f * (float)subpixel, out))
            return true;
        // bitmap-only glyphs: DirectWrite's rasterizer
        DWRITE_GLYPH_RUN run = {};
        const FLOAT advance = 0.0f;
        run.fontFace = faces_[faceSlot].Get();
        run.fontEmSize = emPixels;
        run.glyphCount = 1;
        run.glyphIndices = &glyph;
        run.glyphAdvances = &advance;
        ComPtr<IDWriteGlyphRunAnalysis> analysis;
        // Symmetric (vertical + horizontal) grayscale AA with the font's hinting on the vertical axis:
        // crisp baselines / x-height at small sizes, natural shapes, independent of the background (glass!).
        if (FAILED(factory_->CreateGlyphRunAnalysis(&run, nullptr, DWRITE_RENDERING_MODE1_NATURAL_SYMMETRIC, DWRITE_MEASURING_MODE_NATURAL,
                                                    DWRITE_GRID_FIT_MODE_DEFAULT, DWRITE_TEXT_ANTIALIAS_MODE_GRAYSCALE, 0.25f * (float)subpixel,
                                                    0.0f, &analysis)))
            return false;
        RECT rc = {};
        if (FAILED(analysis->GetAlphaTextureBounds(DWRITE_TEXTURE_ALIASED_1x1, &rc)))
            return false;
        out.left = rc.left;
        out.top = rc.top;
        out.width = rc.right - rc.left;
        out.height = rc.bottom - rc.top;
        if (out.width <= 0 || out.height <= 0)
        {
            out.width = out.height = 0;
            return true;
        }
        out.pixels.resize((size_t)out.width * out.height);
        return SUCCEEDED(analysis->CreateAlphaTexture(DWRITE_TEXTURE_ALIASED_1x1, &rc, out.pixels.data(), (UINT32)out.pixels.size()));
    }

    void TextEngine::ResetAtlas()
    {
        glyphs_.clear();
        for (AtlasPage& p : pages_)
            p.shelfX = p.shelfY = p.shelfH = 0;
        for (AtlasPage& p : lcdPages_)
            p.shelfX = p.shelfY = p.shelfH = 0;
    }

    bool TextEngine::Pack(bool lcd, int w, int h, int& page, int& x, int& y)
    {
        if (w > kPageSize || h > kPageSize)
            return false;
        std::vector<AtlasPage>& pages = lcd ? lcdPages_ : pages_;
        for (int attempt = 0; attempt < 2; ++attempt)
        {
            for (int i = 0; i < (int)pages.size(); ++i)
            {
                AtlasPage& p = pages[i];
                if (p.shelfX + w > kPageSize)
                {
                    p.shelfY += p.shelfH;
                    p.shelfX = 0;
                    p.shelfH = 0;
                }
                if (p.shelfY + h <= kPageSize)
                {
                    page = i;
                    x = p.shelfX;
                    y = p.shelfY;
                    p.shelfX += w;
                    p.shelfH = std::max(p.shelfH, h);
                    return true;
                }
            }
            if ((int)pages.size() < kMaxPages)
            {
                AtlasPage p;
                p.tex = IM_NEW(ImTextureData)();
                p.tex->Create(lcd ? ImTextureFormat_RGBA32 : ImTextureFormat_Alpha8, kPageSize, kPageSize);
                p.tex->LcdCoverage = lcd;
                ImGui::RegisterUserTexture(p.tex);
                pages.push_back(p);
                --attempt;   // retry with the new page (does not count as the reset attempt)
                continue;
            }
            // every page is full: start over (glyphs in use this frame are re-rasterized on demand)
            ResetAtlas();
        }
        return false;
    }

    const TextEngine::GlyphSlot* TextEngine::Glyph(std::uint16_t faceSlot, std::uint16_t glyph, float emPixels, int subpixel)
    {
        const bool lcd = lcd_;
        const std::uint64_t q = (std::uint64_t)std::min(emPixels * 16.0f + 0.5f, 134217727.0f);
        const std::uint64_t key = ((std::uint64_t)faceSlot << 48) | ((std::uint64_t)glyph << 32) | ((std::uint64_t)lcd << 30) | (q << 2) |
                                  (std::uint64_t)(subpixel & 3);
        auto it = glyphs_.find(key);
        if (it != glyphs_.end())
            return &it->second;

        GlyphSlot slot;
        GlyphBitmap bmp;
        bool rasterized = false;
        if (lcd && faceSlot < faces_.size())
            rasterized = RasterizeGlyphOutlineLcd(faces_[faceSlot].Get(), glyph, (float)q / 16.0f, 0.25f * (float)subpixel, params_.bgr, bmp);
        slot.lcd = rasterized;
        if (!rasterized && !RasterizeGlyph(faceSlot, glyph, (float)q / 16.0f, subpixel, bmp))
            return nullptr;
        slot.left = bmp.left;
        slot.top = bmp.top;
        slot.width = bmp.width;
        slot.height = bmp.height;
        if (bmp.width > 0)
        {
            int page = 0, x = 0, y = 0;
            if (!Pack(slot.lcd, bmp.width + kGlyphPadding, bmp.height + kGlyphPadding, page, x, y))
                return nullptr;
            ImTextureData* tex = (slot.lcd ? lcdPages_ : pages_)[page].tex;
            const size_t bpp = slot.lcd ? 4 : 1;
            for (int row = 0; row < bmp.height; ++row)
                std::memcpy(tex->GetPixelsAt(x, y + row), bmp.pixels.data() + (size_t)row * bmp.width * bpp, (size_t)bmp.width * bpp);
            ImTextureDataQueueUpload(tex, x, y, bmp.width, bmp.height);
            slot.page = page;
            const float inv = 1.0f / (float)kPageSize;
            slot.u0 = x * inv;
            slot.v0 = y * inv;
            slot.u1 = (x + bmp.width) * inv;
            slot.v1 = (y + bmp.height) * inv;
        }
        auto res = glyphs_.emplace(key, slot);
        return &res.first->second;
    }

    // ================================================================= draw
    void TextEngine::Draw(ImDrawList* dl, const ShapedText& shaped, Vec2 origin, ImU32 color, float scale)
    {
        if (shaped.glyphs.empty() || (color & IM_COL32_A_MASK) == 0)
            return;
        const float rs = renderScale_;
        const float inv = 1.0f / rs;
        const ImVec4 clip = dl->_ClipRectStack.back();
        ImTextureData* bound = nullptr;
        for (const ShapedGlyph& g : shaped.glyphs)
        {
            // physical-pixel placement: integer baseline, quarter-pixel horizontal phase
            const float X = (origin.x + g.x * scale) * rs;
            const float Y = (origin.y + g.y * scale) * rs;
            float xi = std::floor(X);
            int phase = (int)std::floor((X - xi) * 4.0f + 0.5f);
            if (phase == 4)
            {
                phase = 0;
                xi += 1.0f;
            }
            const float yi = std::floor(Y + 0.5f);
            // animated scales are quantized to 1/4 px of em so an animation does not flood the atlas
            const float em = scale == 1.0f ? g.em * rs : std::floor(g.em * scale * rs * 4.0f + 0.5f) * 0.25f;
            const GlyphSlot* s = Glyph(g.face, g.index, em, phase);
            if (!s || s->page < 0)
                continue;
            const ImVec2 a((xi + s->left) * inv, (yi + s->top) * inv);
            const ImVec2 b(a.x + s->width * inv, a.y + s->height * inv);
            if (b.x < clip.x || b.y < clip.y || a.x > clip.z || a.y > clip.w)
                continue;
            ImTextureData* tex = PageTexture(*s);
            if (tex != bound)
            {
                if (bound)
                    dl->PopTexture();
                dl->PushTexture(tex->GetTexRef());
                bound = tex;
            }
            ImU32 col = color;
            if (g.color)
            {
                // palette layer: its own color, faded with the text
                const std::uint32_t alpha = ((g.color >> IM_COL32_A_SHIFT) & 0xFF) * ((color >> IM_COL32_A_SHIFT) & 0xFF) / 255;
                col = (g.color & ~IM_COL32_A_MASK) | (alpha << IM_COL32_A_SHIFT);
            }
            dl->PrimReserve(6, 4);
            dl->PrimRectUV(a, b, ImVec2(s->u0, s->v0), ImVec2(s->u1, s->v1), col);
        }
        if (bound)
            dl->PopTexture();
    }

    bool TextEngine::DrawIcon(ImDrawList* dl, std::uint32_t codepoint, float size, Vec2 center, ImU32 color)
    {
        IDWriteFontFace* face = PrimaryFace(kIconFont, size);
        if (!face)
            return false;
        UINT16 index = 0;
        const UINT32 cp = codepoint;
        if (FAILED(face->GetGlyphIndices(&cp, 1, &index)) || index == 0)
            return false;
        const float rs = renderScale_;
        // sizes animate (tab / toggle icons): quantized to 1/4 px so animations reuse atlas entries
        const GlyphSlot* s = Glyph(FaceSlot(face), index, std::floor(size * rs * 4.0f + 0.5f) * 0.25f, 0);
        if (!s || s->page < 0)
            return false;
        // optical centering on the rasterized ink box, snapped to physical pixels
        const float x0 = std::floor(center.x * rs - s->width * 0.5f + 0.5f);
        const float y0 = std::floor(center.y * rs - s->height * 0.5f + 0.5f);
        const float inv = 1.0f / rs;
        dl->PushTexture(PageTexture(*s)->GetTexRef());
        dl->PrimReserve(6, 4);
        dl->PrimRectUV(ImVec2(x0 * inv, y0 * inv), ImVec2((x0 + s->width) * inv, (y0 + s->height) * inv), ImVec2(s->u0, s->v0), ImVec2(s->u1, s->v1), color);
        dl->PopTexture();
        return true;
    }

    // ==================================================== ImGui compat support
    bool TextEngine::ResolveCodepoint(FontId font, float size, std::uint32_t codepoint, ResolvedGlyph& out)
    {
        IDWriteFontFace* face = PrimaryFace(font, size);
        if (!face)
            return false;
        UINT16 index = 0;
        if (SUCCEEDED(face->GetGlyphIndices(&codepoint, 1, &index)) && index != 0)
        {
            out.face = face;
            out.index = index;
            out.faceSlot = FaceSlot(face);
            return true;
        }
        const std::uint64_t key = ((std::uint64_t)font << 40) ^ ((std::uint64_t)FaceSlot(face) << 24) ^ codepoint;
        auto it = codepointCache_.find(key);
        if (it != codepointCache_.end())
        {
            out = it->second;
            return out.face != nullptr;
        }
        ResolvedGlyph r;
        if (fallback_)
        {
            wchar_t buf[2];
            UINT32 len = 1;
            if (codepoint >= 0x10000)
            {
                const std::uint32_t v = codepoint - 0x10000;
                buf[0] = (wchar_t)(0xD800 + (v >> 10));
                buf[1] = (wchar_t)(0xDC00 + (v & 0x3FF));
                len = 2;
            }
            else
                buf[0] = (wchar_t)codepoint;
            CharSource src(buf, len, locale_.c_str());
            const FontDef& def = fonts_[(font == 0 || font > fonts_.size()) ? 0 : font - 1];
            UINT32 mapped = 0;
            ComPtr<IDWriteFont> mappedFont;
            FLOAT scale = 1.0f;
            if (SUCCEEDED(fallback_->MapCharacters(&src, 0, len, collection_.Get(), FamilyFor(def, size).c_str(), def.weight, def.style,
                                                   DWRITE_FONT_STRETCH_NORMAL, &mapped, &mappedFont, &scale)) && mappedFont)
            {
                ComPtr<IDWriteFontFace> ff;
                if (SUCCEEDED(mappedFont->CreateFontFace(&ff)))
                {
                    UINT16 fi = 0;
                    if (SUCCEEDED(ff->GetGlyphIndices(&codepoint, 1, &fi)) && fi != 0)
                    {
                        r.faceSlot = FaceSlot(ff.Get());
                        r.face = faces_[r.faceSlot].Get();
                        r.index = fi;
                    }
                }
            }
        }
        codepointCache_[key] = r;
        out = r;
        return r.face != nullptr;
    }

    float TextEngine::GlyphAdvance(std::uint16_t faceSlot, std::uint16_t glyph, float em) const
    {
        if (faceSlot >= faces_.size())
            return 0.0f;
        DWRITE_GLYPH_METRICS gm = {};
        faces_[faceSlot]->GetDesignGlyphMetrics(&glyph, 1, &gm, FALSE);
        return (float)gm.advanceWidth * em / (float)faceUnitsPerEm_[faceSlot];
    }

    void TextEngine::FontVerticalMetrics(FontId font, float size, float& ascent, float& descent)
    {
        ascent = size * 0.9f;
        descent = size * 0.25f;
        if (IDWriteFontFace* face = PrimaryFace(font, size))
        {
            DWRITE_FONT_METRICS fm;
            face->GetMetrics(&fm);
            const float k = size / (float)(fm.designUnitsPerEm ? fm.designUnitsPerEm : 2048);
            ascent = fm.ascent * k;
            descent = fm.descent * k;
        }
    }
}
