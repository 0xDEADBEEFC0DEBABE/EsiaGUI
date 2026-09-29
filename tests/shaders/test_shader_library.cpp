// Shader library: every format generated on Linux is complete, text blobs are null-terminated, lookups work.
#include "esia/render/shader_library.hpp"
#include "esia_test.hpp"
#include <cstring>

using namespace esia;

ESIA_TEST(ShaderLibrary, EveryFormatHasItsPrograms)
{
    // the formats built on Linux must be complete; the Direct3D ones appear when fxc / DXC ran on Windows
    for (shaders::Format f : {shaders::Format::SpirV, shaders::Format::Glsl330, shaders::Format::Essl300, shaders::Format::Msl})
    {
        ESIA_CHECK(shaders::Available(f));
        for (int p = 0; p < (int)rhi::ShaderProgram::Count; ++p)
            for (shaders::Stage s : {shaders::Stage::Vertex, shaders::Stage::Pixel})
            {
                const shaders::ShaderBlob* b = shaders::Find(f, (rhi::ShaderProgram)p, s);
                ESIA_CHECK(b && b->size > 0);
                if (b && b->text)
                    ESIA_CHECK(std::strlen(reinterpret_cast<const char*>(b->data)) == b->size);   // null-terminated text
            }
    }
    // ESSL: nothing at reduced precision (mediump is FP16 on mobile GPUs, far too coarse for pixel coordinates)
    for (int p = 0; p < (int)rhi::ShaderProgram::Count; ++p)
        for (shaders::Stage s : {shaders::Stage::Vertex, shaders::Stage::Pixel})
            if (const shaders::ShaderBlob* b = shaders::Find(shaders::Format::Essl300, (rhi::ShaderProgram)p, s))
            {
                const char* text = reinterpret_cast<const char*>(b->data);
                ESIA_CHECK(std::strstr(text, "mediump") == nullptr && std::strstr(text, "lowp") == nullptr);
            }
    ESIA_CHECK(shaders::FxStorageOf(shaders::Format::Glsl330) == rhi::FxStorage::Texture);
    ESIA_CHECK(shaders::FxStorageOf(shaders::Format::SpirV) == rhi::FxStorage::Buffer);
    ESIA_CHECK(shaders::FindSource("esia_fx.hlsl") != nullptr && shaders::FindSource("nope.hlsl") == nullptr);
    // the runtime-compilation table agrees with the generated library (both come from build_shaders.py's PROGRAMS)
    for (int p = 0; p < (int)rhi::ShaderProgram::Count; ++p)
    {
        const shaders::ProgramSource src = shaders::SourceOf((rhi::ShaderProgram)p);
        const shaders::ShaderBlob* vs = shaders::Find(shaders::Format::SpirV, (rhi::ShaderProgram)p, shaders::Stage::Vertex);
        const shaders::ShaderBlob* ps = shaders::Find(shaders::Format::SpirV, (rhi::ShaderProgram)p, shaders::Stage::Pixel);
        ESIA_CHECK(src.file && shaders::FindSource(src.file) && vs && ps);
        if (src.file && vs && ps)
            ESIA_CHECK(std::strcmp(vs->entry, src.vertexEntry) == 0 && std::strcmp(ps->entry, src.pixelEntry) == 0);
    }
}
