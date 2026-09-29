// WGT UI - text setup and the public font API.
//
// No font file is ever read here: families are resolved by name through DirectWrite (system collection +
// files registered with ContextDesc::fontFiles / Context::AddFontFile). Dear ImGui gets DirectWrite-backed
// compatibility fonts through a custom ImFontLoader, and its atlas is Alpha8 (rendered by the text shader).
#include "core/context_impl.hpp"

namespace wgt
{
    namespace
    {
        std::string Str(const char* s) { return s ? std::string(s) : std::string(); }
    }

    bool LoadFonts(Context::Impl& impl)
    {
        const ContextDesc& d = impl.desc;
        TextEngineDesc td;
        td.fontFamily = Str(d.fontFamily);
        td.fontFamilyDisplay = Str(d.fontFamilyDisplay);
        td.monoFamily = Str(d.monoFamily);
        td.iconFamily = Str(d.iconFamily);
        td.locale = Str(d.locale);
        for (int i = 0; i < d.fontFileCount && d.fontFiles; ++i)
            if (d.fontFiles[i])
                td.fontFiles.push_back(d.fontFiles[i]);

        impl.fonts.engine = std::make_unique<TextEngine>();
        if (!impl.fonts.engine->Init(td))
        {
            impl.Log(2, "WGT: DirectWrite text engine failed to initialize");
            impl.fonts.engine.reset();
            return false;
        }

        // Dear ImGui side: our loader, Alpha8 atlas, one compat font per weight.
        ImFontAtlas* atlas = ImGui::GetIO().Fonts;
        atlas->TexDesiredFormat = ImTextureFormat_Alpha8;
        atlas->SetFontLoader(GetDirectWriteFontLoader());
        static const char* kNames[] = {"WGT Regular", "WGT Semibold", "WGT Bold", "WGT Mono"};
        for (int w = 0; w < (int)FontWeight::Count; ++w)
        {
            CompatFontSource& src = impl.fonts.compatSources[w];
            src.engine = impl.fonts.engine.get();
            src.font = (FontId)(w + 1);
            impl.fonts.compat[w] = AddCompatFont(atlas, &src, kNames[w]);
        }
        ImGui::GetIO().FontDefault = impl.fonts.compat[(int)FontWeight::Regular];
        return true;
    }

    // ============================================================ public API
    FontRef GetFont(TextStyle style)
    {
        const Theme& th = RequireImpl().theme.current;
        FontRef r;
        r.id = (FontId)th.type.weight[(int)style] + 1;
        r.size = std::round(th.type.size[(int)style] * th.metrics.scale * 4.0f) * 0.25f;
        return r;
    }

    FontRef GetFont(FontWeight weight, float unscaledSize)
    {
        const Theme& th = RequireImpl().theme.current;
        FontRef r;
        r.id = (FontId)weight + 1;
        r.size = std::round(unscaledSize * th.metrics.scale * 4.0f) * 0.25f;
        return r;
    }

    FontId GetFontId(FontWeight weight) { return (FontId)weight + 1; }

    ImFont* GetImGuiFont(FontWeight weight)
    {
        Context::Impl& impl = RequireImpl();
        ImFont* f = impl.fonts.compat[(int)weight];
        return f ? f : ImGui::GetFont();
    }

    FontId RegisterFont(const char* family, int weight, bool italic)
    {
        Context::Impl& impl = RequireImpl();
        if (!impl.fonts.engine || !family)
            return GetFontId(FontWeight::Regular);
        return impl.fonts.engine->RegisterFont(family, weight, italic);
    }

    TextMetrics MeasureText(FontRef font, const char* text, const char* textEnd, float wrapWidth, std::uint32_t flags)
    {
        Context::Impl& impl = RequireImpl();
        if (!impl.fonts.engine || !text)
            return {};
        const ShapedText* s = impl.fonts.engine->Shape(font, text, textEnd, wrapWidth, flags);
        return s ? s->metrics : TextMetrics{};
    }

    void Context::AddFontFile(const wchar_t* path)
    {
        if (path && impl_->fonts.engine)
            impl_->fonts.engine->QueueFontFile(path);
    }
}
