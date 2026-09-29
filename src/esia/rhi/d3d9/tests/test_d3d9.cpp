// Direct3D 9: every program compiles for SM3 with the prelude (and the FX shader for the feature masks batches use,
// glass included); a host render target surface wrapped, drawn into and read back; the host's state restored;
// FX variants built in the background.
#include "esia/rhi/backend_registry.hpp"
#include "esia/rhi/d3d9.hpp"
#include "d3d_shader.hpp"
#include "esia_test.hpp"
#include <d3d9.h>

void EsiaRegisterBackend_d3d9();

using namespace esia;
using esia::rhi::d3d::ComPtr;

namespace esia::rhi::d3d9::blobs
{
    extern const unsigned char esia_sm3_prelude_hlsli[];
}

ESIA_TEST(D3D9Shaders, Sm3ProgramsAndVariants)
{
    rhi::d3d::DebugDesc dd;
    dd.log = [](void*, rhi::d3d::LogLevel level, const char* msg) {
        if (level == rhi::d3d::LogLevel::Error)
            std::fprintf(stderr, "%s\n", msg);
    };
    const rhi::d3d::Logger log(dd);
    rhi::d3d::ShaderRequest r;
    r.model = rhi::d3d::ShaderModel::Sm3;
    r.prelude = reinterpret_cast<const char*>(rhi::d3d9::blobs::esia_sm3_prelude_hlsli);
    for (int p = 0; p < (int)rhi::ShaderProgram::Count; ++p)
    {
        if ((rhi::ShaderProgram)p == rhi::ShaderProgram::Fx)
            continue;   // the FX shader: per feature mask, below
        for (shaders::Stage s : {shaders::Stage::Vertex, shaders::Stage::Pixel})
        {
            r.program = (rhi::ShaderProgram)p;
            r.stage = s;
            ESIA_CHECK(rhi::d3d::CompileShader(r, log) != nullptr);
        }
    }
    // fill, fill + stroke + merge, shadow + glow, glass (+ stroke), image, noise + shimmer, mask + halo, caustic
    r.program = rhi::ShaderProgram::Fx;
    for (std::uint32_t mask : {0x1u, 0x203u, 0x0Du, 0x21u, 0x23u, 0x41u, 0x881u, 0x2401u, 0x4001u})
        for (shaders::Stage s : {shaders::Stage::Vertex, shaders::Stage::Pixel})
        {
            r.fxFeatures = mask;
            r.stage = s;
            const rhi::d3d::Bytecode code = rhi::d3d::CompileShader(r, log);
            ESIA_CHECK(code != nullptr);
            if (!code)
                std::fprintf(stderr, "  FX mask 0x%x failed\n", mask);
        }
}

ESIA_TEST(D3D9Host, WrapDrawRestore)
{
    ComPtr<IDirect3D9Ex> d3d;
    if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &d3d)))
    {
        std::printf("  skipped: no Direct3D 9Ex\n");
        return;
    }
    WNDCLASSW wc = {};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"EsiaD3D9Test";
    RegisterClassW(&wc);
    const HWND hwnd = CreateWindowW(L"EsiaD3D9Test", L"test", WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, wc.hInstance, nullptr);
    D3DPRESENT_PARAMETERS pp = {};
    pp.BackBufferWidth = pp.BackBufferHeight = 1;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = hwnd;
    pp.Windowed = TRUE;
    ComPtr<IDirect3DDevice9Ex> device;
    if (FAILED(d3d->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, nullptr, &device)))
    {
        std::printf("  skipped: no D3D9 device\n");
        return;
    }
    auto dev = rhi::d3d9::CreateDevice(device.Get(), true);
    ESIA_CHECK(dev != nullptr);
    if (!dev)
        return;
    ESIA_CHECK(dev->GetCaps().halfPixelOffset && dev->GetCaps().fxFeatureVariants);
    // perRow = maxFxDataWidth / 24 is a power of two (exact float modulo in FxFetch)
    const int perRow = dev->GetCaps().maxFxDataWidth / 24;
    ESIA_CHECK(perRow > 0 && (perRow & (perRow - 1)) == 0 && perRow * 24 == dev->GetCaps().maxFxDataWidth);

    ComPtr<IDirect3DTexture9> tex;
    ESIA_CHECK(SUCCEEDED(device->CreateTexture(32, 16, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &tex, nullptr)));
    ComPtr<IDirect3DSurface9> surface;
    tex->GetSurfaceLevel(0, &surface);
    const rhi::Texture t = rhi::d3d9::WrapRenderTarget(*dev, surface.Get(), tex.Get(), true);
    ESIA_CHECK((bool)t);
    ESIA_CHECK(rhi::d3d9::WrapRenderTarget(*dev, surface.Get(), tex.Get(), true) == t);
    const rhi::TextureDesc d = dev->GetTextureDesc(t);
    ESIA_CHECK(d.format == rhi::Format::BGRA8_SRGB && (d.usage & rhi::TextureUsage_Sampled) && d.width == 32);

    // host state that must survive the frame
    const D3DVIEWPORT9 hostVp = {1, 2, 3, 4, 0.0f, 1.0f};
    device->SetViewport(&hostVp);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    ComPtr<IDirect3DSurface9> hostRt;
    device->GetRenderTarget(0, &hostRt);

    rhi::FrameDesc fd;
    ESIA_CHECK(dev->BeginFrame(fd));
    rhi::PassDesc p;
    p.target = t;
    p.load = rhi::LoadOp::Clear;
    p.clearColor[2] = 0.5f;   // stored value: 128 on the sRGB target
    p.clearColor[3] = 1.0f;
    dev->BeginPass(p);
    ESIA_CHECK(dev->NativeRenderState() == device.Get());
    dev->EndPass();
    dev->EndFrame();

    D3DVIEWPORT9 vp = {};
    device->GetViewport(&vp);
    ESIA_CHECK(vp.X == 1 && vp.Width == 3);
    DWORD cull = 0;
    device->GetRenderState(D3DRS_CULLMODE, &cull);
    ESIA_CHECK(cull == D3DCULL_CW);
    ComPtr<IDirect3DSurface9> rt;
    device->GetRenderTarget(0, &rt);
    ESIA_CHECK(rt.Get() == hostRt.Get());

    std::vector<std::uint8_t> px;
    ESIA_CHECK(dev->ReadPixels(t, rhi::IRect{0, 0, 32, 16}, px) && px.size() == 32 * 16 * 4);
    if (px.size() == 32 * 16 * 4)
        ESIA_CHECK(px[0] == 0 && px[1] == 0 && std::abs(px[2] - 128) <= 1 && px[3] == 255);
    dev->DestroyTexture(t);
    DestroyWindow(hwnd);
}

// Background FX variants (Caps::asyncPipelines): CreatePipeline returns at once with a Pending variant that compiles
// on a worker; GetPipelineStatus makes its shaders on the render thread when it is done; it is never bound before.
ESIA_TEST(D3D9Pipelines, BackgroundVariants)
{
    EsiaRegisterBackend_d3d9();
    const rhi::BackendInfo* backend = rhi::FindBackend("d3d9");
    rhi::HeadlessDesc hd;
    hd.width = hd.height = 16;
    std::string error;
    rhi::HeadlessDevice h = backend ? backend->createHeadless(hd, error) : rhi::HeadlessDevice{};
    if (!h.device)
    {
        std::printf("  skipped: no D3D9 device (%s)\n", error.c_str());
        return;
    }
    ESIA_CHECK(h.device->GetCaps().asyncPipelines);
    rhi::PipelineDesc d;
    d.program = rhi::ShaderProgram::Fx;
    d.layout = rhi::VertexLayout::None;
    d.topology = rhi::Topology::TriangleStrip;
    d.blend = rhi::BlendMode::Premultiplied;
    d.fxFeatures = 0x105;   // a mask no other test of this process compiles: not in the compile cache
    d.background = true;
    const rhi::Pipeline p = h.device->CreatePipeline(d);
    ESIA_CHECK((bool)p);
    ESIA_CHECK(h.device->GetPipelineStatus(p) == rhi::PipelineStatus::Pending);   // an SM3 compile takes far longer
    rhi::PipelineStatus status = rhi::PipelineStatus::Pending;
    for (int i = 0; i < 6000 && status == rhi::PipelineStatus::Pending; ++i)
    {
        Sleep(10);
        status = h.device->GetPipelineStatus(p);
    }
    ESIA_CHECK(status == rhi::PipelineStatus::Ready);
    d.background = false;   // the full shader and every non-background pipeline: built in CreatePipeline
    d.fxFeatures = 0;
    const rhi::Pipeline full = h.device->CreatePipeline(d);
    ESIA_CHECK(full && h.device->GetPipelineStatus(full) == rhi::PipelineStatus::Ready);
    ESIA_CHECK(h.device->ValidationErrors() == 0);
    h.device->DestroyPipeline(p);
    h.device->DestroyPipeline(full);
}
