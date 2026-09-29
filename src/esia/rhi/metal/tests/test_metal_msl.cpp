// Metal backend: the binding table (metal_tables.hpp) against the generated MSL of every program - the indices the
// backend binds at are the ones the shaders declare, and the shaders declare nothing the backend does not bind.
// UNVERIFIED: needs macOS (passes on Linux and under Wine).
#include "esia/render/shader_library.hpp"
#include "esia_test.hpp"
#include "metal_planning.hpp"
#include "metal_tables.hpp"
#include "msl_interface.hpp"
#include <string>

using namespace esia;
using namespace esia::rhi;
using namespace esia::rhi::metal;
using namespace esia::rhi::metal::test;

namespace
{
    MslInterface Parse(ShaderProgram p, shaders::Stage s)
    {
        const shaders::ShaderBlob* b = shaders::Find(shaders::Format::Msl, p, s);
        if (!b || !b->text)
            return {};
        return ParseMsl(std::string(reinterpret_cast<const char*>(b->data), b->size));
    }

    bool Contains(const std::string& s, const char* part) { return s.find(part) != std::string::npos; }
}

ESIA_TEST(MetalMsl, EveryProgramParses)
{
    for (int p = 0; p < (int)ShaderProgram::Count; ++p)
        for (shaders::Stage s : {shaders::Stage::Vertex, shaders::Stage::Pixel})
        {
            const MslInterface m = Parse((ShaderProgram)p, s);
            if (!m.ok)
                std::printf("  %s %s: %s\n", ShaderProgramName((ShaderProgram)p), s == shaders::Stage::Vertex ? "vs" : "ps", m.error.c_str());
            ESIA_CHECK(m.ok);
            ESIA_CHECK(m.vertex == (s == shaders::Stage::Vertex));
        }
}

ESIA_TEST(MetalMsl, ResourcesAtTheBackendsIndices)
{
    for (int pi = 0; pi < (int)ShaderProgram::Count; ++pi)
    {
        const ShaderProgram p = (ShaderProgram)pi;
        for (shaders::Stage s : {shaders::Stage::Vertex, shaders::Stage::Pixel})
        {
            const MslInterface m = Parse(p, s);
            for (const MslResource& r : m.resources)
            {
                switch (r.kind)
                {
                case MslResource::Buffer:
                    // constants and the FX data, never the vertex buffer's index
                    if (Contains(r.type, "WgtFrame"))
                        ESIA_CHECK(r.index == (int)binding::kFrameConstants && Contains(r.type, "constant"));
                    else if (Contains(r.type, "WgtPass"))
                        ESIA_CHECK(r.index == (int)binding::kPassConstants && Contains(r.type, "constant"));
                    else if (Contains(r.type, "WgtDraw"))
                        ESIA_CHECK(r.index == (int)binding::kDrawConstants && Contains(r.type, "constant"));
                    else if (Contains(r.type, "gFxData"))
                        ESIA_CHECK(r.index == (int)binding::kFxData && p == ShaderProgram::Fx);
                    else
                        ESIA_CHECK(!"unknown buffer");
                    ESIA_CHECK(r.index != (int)binding::kVertexBuffer);
                    break;
                case MslResource::Texture:
                    // the backend binds textures to the fragment stage only
                    ESIA_CHECK(s == shaders::Stage::Pixel);
                    ESIA_CHECK(r.type == "texture2d<float>");
                    if (r.name == "gTex")
                        ESIA_CHECK(r.index == (int)binding::TextureIndex(kSlotTexture));
                    else if (r.name.rfind("gBackdrop", 0) == 0)
                        ESIA_CHECK(r.index == (int)binding::TextureIndex(kSlotBackdrop0 + std::stoi(r.name.substr(9))));
                    else
                        ESIA_CHECK(!"unknown texture");
                    break;
                case MslResource::Sampler:
                    ESIA_CHECK(s == shaders::Stage::Pixel);
                    ESIA_CHECK((r.name == "gLinear" && r.index == (int)binding::kSamplerLinear) || (r.name == "gPoint" && r.index == (int)binding::kSamplerPoint));
                    break;
                }
            }
        }
    }
    // what each program reads, as docs/backends/README.md section 4 lists it
    const MslInterface fxPs = Parse(ShaderProgram::Fx, shaders::Stage::Pixel);
    for (int t = 0; t <= kBackdropLevels; ++t)
        ESIA_CHECK(fxPs.Find(MslResource::Texture, (int)binding::TextureIndex(t)) != nullptr);
    ESIA_CHECK(fxPs.Find(MslResource::Buffer, (int)binding::kFxData) && Parse(ShaderProgram::Fx, shaders::Stage::Vertex).Find(MslResource::Buffer, (int)binding::kFxData));
    const MslInterface composite = Parse(ShaderProgram::LayerComposite, shaders::Stage::Pixel);
    ESIA_CHECK(composite.FindByName("gPoint") && composite.Find(MslResource::Buffer, (int)binding::kPassConstants));
    ESIA_CHECK(Parse(ShaderProgram::Downsample, shaders::Stage::Pixel).Find(MslResource::Buffer, (int)binding::kPassConstants));
    ESIA_CHECK(Parse(ShaderProgram::Clear, shaders::Stage::Pixel).resources.empty());
}

ESIA_TEST(MetalMsl, VertexInputsAndOutputs)
{
    for (int pi = 0; pi < (int)ShaderProgram::Count; ++pi)
    {
        const ShaderProgram p = (ShaderProgram)pi;
        const MslInterface vs = Parse(p, shaders::Stage::Vertex);
        if (ProgramUsesVertices(p))
        {
            // stage_in attributes 0 / 1 / 2 as the vertex descriptor declares them (the color arrives as float4
            // from UChar4Normalized)
            ESIA_CHECK(vs.inputs.size() == 3);
            for (const MslVarying& v : vs.inputs)
            {
                if (v.attribute == "attribute(0)" || v.attribute == "attribute(1)")
                    ESIA_CHECK(v.type == "float2");
                else if (v.attribute == "attribute(2)")
                    ESIA_CHECK(v.type == "float4");
                else
                    ESIA_CHECK(!"unexpected vertex attribute");
            }
            ESIA_CHECK(!vs.usesVertexId && !vs.usesInstanceId);
        }
        else
        {
            ESIA_CHECK(vs.inputs.empty() && vs.usesVertexId);
            ESIA_CHECK(vs.usesInstanceId == (p == ShaderProgram::Fx));
        }
        // every varying the fragment shader reads is written by the vertex shader at the same location and type
        const MslInterface ps = Parse(p, shaders::Stage::Pixel);
        for (const MslVarying& in : ps.inputs)
        {
            bool found = false;
            for (const MslVarying& out : vs.outputs)
                found |= out.attribute == in.attribute && out.type == in.type;
            ESIA_CHECK(found);
            // integer varyings (the FX instance index): SPIRV-Cross leaves out [[flat]] because integers are
            // always flat in MSL
            if (in.type == "uint" || in.type == "int")
                ESIA_CHECK(in.attribute.rfind("user(", 0) == 0);
        }
        // one color output; TextLcd adds the second source for dual-source blending
        int color0 = 0, index1 = 0;
        for (const MslVarying& out : ps.outputs)
        {
            color0 += out.attribute.rfind("color(0)", 0) == 0;
            index1 += Contains(out.attribute, "index(1)");
        }
        ESIA_CHECK(color0 == (p == ShaderProgram::TextLcd ? 2 : 1));
        ESIA_CHECK(index1 == (p == ShaderProgram::TextLcd ? 1 : 0));
    }
}
