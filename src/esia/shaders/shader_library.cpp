// Esia - shader library lookup (see esia/render/shader_library.hpp). The tables are generated.
#include "esia/render/shader_library.hpp"
#include <cstring>

namespace esia::shaders
{
    extern const ShaderBlob kLibrary_spirv[];
    extern const ShaderBlob kLibrary_glsl330[];
    extern const ShaderBlob kLibrary_essl300[];
    extern const ShaderBlob kLibrary_msl[];
    extern const ShaderBlob kLibrary_dxbc_sm5[];
    extern const ShaderBlob kLibrary_dxbc_sm4[];
    extern const ShaderBlob kLibrary_dxil[];
    extern const ShaderBlob kLibrary_dxbc_sm3[];
    namespace blobs
    {
        extern const unsigned char source_esia_common_hlsli[];
        extern const unsigned char source_esia_ui_hlsl[];
        extern const unsigned char source_esia_fx_hlsl[];
        extern const unsigned char source_esia_post_hlsl[];
    }

    namespace
    {
        const SourceFile kSources[] = {
            {"esia_common.hlsli", reinterpret_cast<const char*>(blobs::source_esia_common_hlsli)},
            {"esia_ui.hlsl", reinterpret_cast<const char*>(blobs::source_esia_ui_hlsl)},
            {"esia_fx.hlsl", reinterpret_cast<const char*>(blobs::source_esia_fx_hlsl)},
            {"esia_post.hlsl", reinterpret_cast<const char*>(blobs::source_esia_post_hlsl)},
            {nullptr, nullptr},
        };

        const ShaderBlob* Table(Format f)
        {
            switch (f)
            {
            case Format::SpirV: return kLibrary_spirv;
            case Format::Glsl330: return kLibrary_glsl330;
            case Format::Essl300: return kLibrary_essl300;
            case Format::Msl: return kLibrary_msl;
            case Format::DxbcSm5: return kLibrary_dxbc_sm5;
            case Format::DxbcSm4: return kLibrary_dxbc_sm4;
            case Format::Dxil: return kLibrary_dxil;
            case Format::DxbcSm3: return kLibrary_dxbc_sm3;
            case Format::Count: break;
            }
            return nullptr;
        }
    }

    const char* FormatName(Format f)
    {
        static const char* names[] = {"spirv", "glsl330", "essl300", "msl", "dxbc_sm5", "dxbc_sm4", "dxil", "dxbc_sm3"};
        return f < Format::Count ? names[(int)f] : "?";
    }

    rhi::FxStorage FxStorageOf(Format f)
    {
        return (f == Format::Glsl330 || f == Format::Essl300 || f == Format::DxbcSm4 || f == Format::DxbcSm3) ? rhi::FxStorage::Texture
                                                                                                            : rhi::FxStorage::Buffer;
    }

    const ShaderBlob* Find(Format format, rhi::ShaderProgram program, Stage stage)
    {
        for (const ShaderBlob* b = Table(format); b && b->program >= 0; ++b)
            if (b->program == (int)program && b->stage == (int)stage)
                return b;
        return nullptr;
    }

    bool Available(Format format)
    {
        const ShaderBlob* t = Table(format);
        return t && t->program >= 0;
    }

    ProgramSource SourceOf(rhi::ShaderProgram program)
    {
        // must match PROGRAMS in tools/shaders/build_shaders.py
        switch (program)
        {
        case rhi::ShaderProgram::UiGeometry: return {"esia_ui.hlsl", "UiVS", "UiPS"};
        case rhi::ShaderProgram::TextGray: return {"esia_ui.hlsl", "UiVS", "TextGrayPS"};
        case rhi::ShaderProgram::Fx: return {"esia_fx.hlsl", "FxVS", "FxPS"};
        case rhi::ShaderProgram::Downsample: return {"esia_post.hlsl", "FullscreenVS", "DownsamplePS"};
        case rhi::ShaderProgram::LayerComposite: return {"esia_post.hlsl", "FullscreenVS", "LayerCompositePS"};
        case rhi::ShaderProgram::Clear: return {"esia_post.hlsl", "FullscreenVS", "ClearPS"};
        case rhi::ShaderProgram::Count: break;
        }
        return {nullptr, nullptr, nullptr};
    }

    const char* FindSource(const char* name)
    {
        for (const SourceFile* s = kSources; s->name; ++s)
            if (std::strcmp(s->name, name) == 0)
                return s->text;
        return nullptr;
    }
}
