// Direct3D shared code: every program compiles for SM4 (D3D10) and SM5 (D3D11 / 12) with the machine's
// d3dcompiler_47.dll, a user effect compiles on a worker thread, the disk cache, and the readback / format helpers.
#include "d3d_shader.hpp"
#include "esia/rhi/d3d_common.hpp"
#include "esia_test.hpp"
#include <chrono>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

using namespace esia;
using namespace esia::rhi::d3d;

namespace
{
    Logger Quiet()
    {
        DebugDesc d;
        d.log = [](void*, LogLevel level, const char* msg) {
            if (level == LogLevel::Error)
                std::fprintf(stderr, "%s\n", msg);
        };
        return Logger(d);
    }
}

ESIA_TEST(D3DShaders, EveryProgramCompiles)
{
    std::string error;
    ESIA_CHECK(ShaderCompilerAvailable(error));
    if (!error.empty())
        std::fprintf(stderr, "%s\n", error.c_str());
    const Logger log = Quiet();
    for (ShaderModel m : {ShaderModel::Sm4, ShaderModel::Sm5})
        for (int p = 0; p < (int)rhi::ShaderProgram::Count; ++p)
            for (shaders::Stage s : {shaders::Stage::Vertex, shaders::Stage::Pixel})
            {
                ShaderRequest r;
                r.program = (rhi::ShaderProgram)p;
                r.stage = s;
                r.model = m;
                const Bytecode code = CompileShader(r, log);
                ESIA_CHECK(code && code->size() > 16);
                // a second request is served from the cache: the same bytecode object
                ESIA_CHECK(CompileShader(r, log) == code);
            }
}

ESIA_TEST(D3DShaders, FeatureVariantAndUserEffect)
{
    const Logger log = Quiet();
    ShaderRequest r;
    r.program = rhi::ShaderProgram::Fx;
    r.stage = shaders::Stage::Pixel;
    r.model = ShaderModel::Sm5;
    r.fxFeatures = 1u;   // fill only
    const Bytecode fill = CompileShader(r, log);
    ESIA_CHECK(fill != nullptr);

    r.fxFeatures = 0;
    r.effectSource = "float4 WgtEffect(WgtFx fx) { return float4(fx.uv, 0.0, 1.0) * fx.coverage; }";
    Bytecode effect;
    CompileState st = CompileState::Pending;
    for (int i = 0; i < 600 && st == CompileState::Pending; ++i)
    {
        st = CompileShaderAsync(r, log, effect);
        if (st == CompileState::Pending)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    ESIA_CHECK(st == CompileState::Ready && effect);

    r.effectSource = "this is not HLSL";
    for (int i = 0; i < 600 && (st = CompileShaderAsync(r, log, effect)) == CompileState::Pending; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ESIA_CHECK(st == CompileState::Failed);
}

ESIA_TEST(D3DShaders, DiskCache)
{
    namespace fs = std::filesystem;
    const fs::path dir = fs::temp_directory_path() / ("esia_shader_cache_test_" + std::to_string(GetCurrentProcessId()));
    std::error_code ec;
    fs::remove_all(dir, ec);
    const auto files = [&] {
        std::vector<fs::path> out;
        for (const auto& e : fs::directory_iterator(dir, ec))
            out.push_back(e.path());
        return out;
    };
    // the log tells a compile from a load (Info messages need debugLayer)
    static std::vector<std::string> messages;
    DebugDesc d;
    d.debugLayer = true;
    d.log = [](void*, LogLevel, const char* msg) { messages.push_back(msg); };
    const Logger log(d);
    const auto loaded = [&] { return !messages.empty() && messages.back().find("from the shader cache") != std::string::npos; };

    SetShaderCacheDirectory(reinterpret_cast<const char*>(dir.u8string().c_str()));
    ShaderRequest r;
    r.program = rhi::ShaderProgram::Fx;
    r.stage = shaders::Stage::Pixel;
    r.model = ShaderModel::Sm4;
    r.fxFeatures = 0x13u;   // no other test of this process compiles it
    const Bytecode compiled = CompileShader(r, log);
    ESIA_CHECK(compiled && !loaded());
    ESIA_CHECK(files().size() == 1);   // written, and no temporary file left

    // a new process (here: the process's shaders forgotten) reads it back, the same bytecode
    ForgetCompiledShaders();
    const Bytecode again = CompileShader(r, log);
    ESIA_CHECK(again && loaded() && *again == *compiled);

    // another shader is another entry; a damaged entry is compiled again and rewritten
    r.fxFeatures = 0x15u;
    ESIA_CHECK(CompileShader(r, log) && !loaded() && files().size() == 2);
    for (const fs::path& f : files())
        fs::resize_file(f, 10, ec);
    ForgetCompiledShaders();
    r.fxFeatures = 0x13u;
    const Bytecode rebuilt = CompileShader(r, log);
    ESIA_CHECK(rebuilt && !loaded() && *rebuilt == *compiled);
    ForgetCompiledShaders();
    ESIA_CHECK(CompileShader(r, log) && loaded());

    SetShaderCacheDirectory("");
    ForgetCompiledShaders();
    ESIA_CHECK(CompileShader(r, log) && !loaded());   // no cache: compiled
    fs::remove_all(dir, ec);
}

ESIA_TEST(D3DCommon, FormatsAndReadback)
{
    ESIA_CHECK(DxgiFormatsOf(rhi::Format::RGBA8_SRGB).storage == DXGI_FORMAT_R8G8B8A8_TYPELESS);
    ESIA_CHECK(DxgiFormatsOf(rhi::Format::RGBA8_SRGB).srv == DXGI_FORMAT_R8G8B8A8_UNORM);
    ESIA_CHECK(DxgiFormatsOf(rhi::Format::BGRA8_SRGB).rtv == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
    ESIA_CHECK(FormatFromDxgi(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB) == rhi::Format::BGRA8_SRGB);
    ESIA_CHECK(DxgiRaw(DXGI_FORMAT_R8G8B8A8_TYPELESS) == DXGI_FORMAT_R8G8B8A8_UNORM);

    std::vector<std::uint8_t> out;
    const std::uint8_t bgra[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    ConvertToRgba8(rhi::Format::BGRA8_UNORM, bgra, 8, 2, 1, out);
    ESIA_CHECK(out.size() == 8 && out[0] == 3 && out[2] == 1 && out[3] == 4 && out[4] == 7);
    const std::uint16_t half[4] = {0x3C00, 0x3800, 0x0000, 0xBC00};   // 1, 0.5, 0, -1
    ConvertToRgba8(rhi::Format::RGBA16_FLOAT, half, 8, 1, 1, out);
    ESIA_CHECK(out[0] == 255 && out[1] == 128 && out[2] == 0 && out[3] == 0);
    const float f[4] = {0.25f, 2.0f, 0.0f, 1.0f};
    ConvertToRgba8(rhi::Format::RGBA32_FLOAT, f, 16, 1, 1, out);
    ESIA_CHECK(out[0] == 64 && out[1] == 255 && out[3] == 255);
    const std::uint32_t rgb10 = 1023u | (0u << 10) | (512u << 20) | (3u << 30);
    ConvertToRgba8(rhi::Format::RGB10A2_UNORM, &rgb10, 4, 1, 1, out);
    ESIA_CHECK(out[0] == 255 && out[1] == 0 && out[2] == 128 && out[3] == 255);

    const float in[4] = {0.5f, 0.0f, 1.0f, 0.5f};
    float v[4];
    ClearValueFor(rhi::Format::RGBA8_SRGB, in, v);
    ESIA_CHECK_NEAR(v[0], 0.214f, 1e-3f);
    ESIA_CHECK(v[3] == 0.5f);
}

ESIA_TEST(D3DCommon, ProfileIntervals)
{
    ProfileFrame f;
    f.Reset(7);
    f.frameStart = 0;
    f.intervals.push_back({1, 2, rhi::ProfileCategory::Capture});
    f.intervals.push_back({3, 4, rhi::ProfileCategory::Capture});
    f.intervals.push_back({4, 5, rhi::ProfileCategory::Fx});
    f.frameEnd = 6;
    const std::uint64_t ticks[7] = {1000, 1100, 1300, 1300, 1400, 1450, 3000};
    const rhi::GpuProfile p = f.Resolve(ticks, 7, 1e6);   // 1 tick = 1 us
    ESIA_CHECK(p.valid && p.frame == 7);
    ESIA_CHECK_NEAR(p.totalMs, 2.0f, 1e-5f);
    ESIA_CHECK_NEAR(p.categoryMs[(int)rhi::ProfileCategory::Capture], 0.3f, 1e-5f);
    ESIA_CHECK_NEAR(p.categoryMs[(int)rhi::ProfileCategory::Fx], 0.05f, 1e-5f);
}
