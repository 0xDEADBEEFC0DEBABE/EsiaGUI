// Esia - runtime HLSL compilation for the Direct3D backends: D3DCompile from d3dcompiler_47.dll (part of Windows
// 10 / 11) on the embedded shader sources (shaders::FindSource), with an include handler that serves them, the
// user effects and the SM3 prelude. The shader library has no DXBC until someone runs tools/shaders with fxc on
// Windows, and user effects and FX feature variants always compile at runtime (docs/backends/README.md, 4.4).
//
// Bytecode is cached per process (program stage, shader model, defines, effect source): the FX pixel shader takes
// seconds to compile, and the conformance suite creates a device per scene.
#pragma once
#include "d3d_util.hpp"
#include "esia/render/shader_library.hpp"
#include <memory>

namespace esia::rhi::d3d
{
    enum class ShaderModel : std::uint8_t
    {
        Sm3,   // vs_3_0 / ps_3_0, FX instances in a texture, the D3D9 backend's prelude and source fix-ups
        Sm4,   // vs_4_0 / ps_4_0, FX instances in a texture (D3D10)
        Sm5,   // vs_5_0 / ps_5_0, FX instances in a structured buffer (D3D11, D3D12)
    };

    using Bytecode = std::shared_ptr<const std::vector<std::uint8_t>>;

    // A textual replacement applied to every source file before compilation (SM3 fix-ups of the shared sources).
    struct SourcePatch
    {
        const char* file;
        const char* from;
        const char* to;
    };

    struct ShaderRequest
    {
        ShaderProgram program = ShaderProgram::UiGeometry;
        shaders::Stage stage = shaders::Stage::Vertex;
        ShaderModel model = ShaderModel::Sm5;
        std::uint32_t fxFeatures = 0;          // Fx: ESIA_FX_FEATURES (0 = every feature)
        const char* effectSource = nullptr;    // Fx pixel shader: the user effect (ESIA_CUSTOM_EFFECT)
        // Sm3: the prelude, prepended to the program's source: fxc's preprocessor cannot `#include` a macro, so
        // esia_common.hlsli's `#include ESIA_SHADER_PRELUDE` is not usable with D3DCompile (STATUS.md, core requests)
        const char* prelude = nullptr;
        const SourcePatch* patches = nullptr;
        int patchCount = 0;
    };

    // Loads d3dcompiler_47.dll (once); false with the reason when it is missing.
    bool ShaderCompilerAvailable(std::string& error);

    // Compiles now (or finds in the cache). Null on failure; errors and warnings go to `log`.
    Bytecode CompileShader(const ShaderRequest& request, const Logger& log);

    // User effects compile on a worker thread so that a new effect never stalls a frame: Pending until the
    // bytecode is ready (the renderer asks again on a later frame), Failed for good once the source was rejected.
    // The worker never logs (the host's callback may be gone by then): the first call that sees the result logs
    // the compiler's message through `log`, on the caller's thread.
    enum class CompileState { Pending, Ready, Failed };
    CompileState CompileShaderAsync(const ShaderRequest& request, const Logger& log, Bytecode& out);

    // The profile string ("ps_5_0" ...) of a request.
    const char* ShaderProfile(ShaderModel model, shaders::Stage stage);
}
