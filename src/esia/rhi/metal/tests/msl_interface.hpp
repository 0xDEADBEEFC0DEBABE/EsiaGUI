// Metal backend tests: the resource interface of a generated MSL shader (esia_main's parameters and its stage_in /
// output structs), parsed from the text SPIRV-Cross emits. It lets the tests check the backend's binding table
// against the real shaders, and lets the fake GPU know what each draw must have bound.
#pragma once
#include <string>
#include <vector>

namespace esia::rhi::metal::test
{
    struct MslResource
    {
        enum Kind { Buffer, Texture, Sampler } kind;
        int index;
        std::string type;   // "constant WgtFrame&", "texture2d<float>", "sampler", "const device gFxData&"
        std::string name;
    };

    struct MslVarying
    {
        std::string type;
        std::string name;
        std::string attribute;   // "attribute(0)", "user(locn1)", "color(0), index(1)", "position" ...
    };

    struct MslInterface
    {
        bool ok = false;
        std::string error;
        bool vertex = false;     // `vertex` or `fragment` function
        std::vector<MslResource> resources;
        std::vector<MslVarying> inputs;    // members of esia_main_in (stage_in)
        std::vector<MslVarying> outputs;   // members of esia_main_out
        bool usesVertexId = false, usesInstanceId = false;

        const MslResource* Find(MslResource::Kind kind, int index) const;
        const MslResource* FindByName(const std::string& name) const;
    };

    MslInterface ParseMsl(const std::string& source);
}
