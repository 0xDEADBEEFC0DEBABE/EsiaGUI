// WGT UI - runtime effect compiler (see effect_compiler.hpp)
#include "render/effect_compiler.hpp"
#include <windows.h>
#include <d3dcompiler.h>
#include <cstring>
#include <mutex>

#include "wgt_embedded_hlsl.h"

namespace wgt
{
    namespace
    {
        class EmbeddedInclude final : public ID3DInclude
        {
        public:
            explicit EmbeddedInclude(const std::string& user) : user_(user) {}

            HRESULT __stdcall Open(D3D_INCLUDE_TYPE, LPCSTR name, LPCVOID, LPCVOID* data, UINT* bytes) override
            {
                if (std::strcmp(name, "wgt_common.hlsli") == 0)
                {
                    *data = kHlslCommon;
                    *bytes = (UINT)kHlslCommonSize;
                    return S_OK;
                }
                if (std::strcmp(name, "wgt_fx.hlsl") == 0)
                {
                    *data = kHlslFx;
                    *bytes = (UINT)kHlslFxSize;
                    return S_OK;
                }
                if (std::strcmp(name, "wgt_user_effect.hlsli") == 0)
                {
                    *data = user_.data();
                    *bytes = (UINT)user_.size();
                    return S_OK;
                }
                return E_FAIL;
            }
            HRESULT __stdcall Close(LPCVOID) override { return S_OK; }

        private:
            const std::string& user_;
        };

        pD3DCompile LoadCompiler()
        {
            static std::once_flag once;
            static pD3DCompile fn = nullptr;
            std::call_once(once, [] {
                HMODULE mod = LoadLibraryW(L"d3dcompiler_47.dll");
                if (mod)
                    fn = reinterpret_cast<pD3DCompile>(GetProcAddress(mod, "D3DCompile"));
            });
            return fn;
        }
    }

    bool CompileUserEffect(const std::string& name, const std::string& userSource, EffectBytecode& out)
    {
        out.code.clear();
        out.error.clear();
        pD3DCompile compile = LoadCompiler();
        if (!compile)
        {
            out.error = "d3dcompiler_47.dll not available";
            return false;
        }
        static const char kMain[] = "#define WGT_CUSTOM_EFFECT 1\n#include \"wgt_fx.hlsl\"\n";
        EmbeddedInclude includes(userSource);
        ID3DBlob* code = nullptr;
        ID3DBlob* errors = nullptr;
        HRESULT hr = compile(kMain, sizeof(kMain) - 1, name.c_str(), nullptr, &includes, "FxPS", "ps_5_0",
                             D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &code, &errors);
        if (errors)
        {
            out.error.assign((const char*)errors->GetBufferPointer(), errors->GetBufferSize());
            errors->Release();
        }
        if (FAILED(hr) || !code)
        {
            if (code)
                code->Release();
            if (out.error.empty())
                out.error = "D3DCompile failed";
            return false;
        }
        const unsigned char* p = (const unsigned char*)code->GetBufferPointer();
        out.code.assign(p, p + code->GetBufferSize());
        code->Release();
        return true;
    }
}
