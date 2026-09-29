// WGT UI - runtime HLSL compilation of user FX effects (shared by the DX11 and DX12 backends).
#pragma once
#include <string>
#include <vector>

namespace wgt
{
    struct EffectBytecode
    {
        std::vector<unsigned char> code;
        std::string error;
    };

    // Compiles the FX pixel shader with the user's `WgtEffect()` injected (ps_5_0, DXBC).
    // Loads d3dcompiler_47.dll lazily (ships with Windows 10+).
    bool CompileUserEffect(const std::string& name, const std::string& userSource, EffectBytecode& out);
}
