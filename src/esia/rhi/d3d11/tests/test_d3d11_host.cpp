// Direct3D 11 host integration: a host render target view wrapped (an sRGB view of typeless storage: sampleable
// through its raw view), cached per view, drawn into, read back; the host's pipeline state restored after a frame.
#include "esia/rhi/d3d11.hpp"
#include "d3d_util.hpp"
#include "esia_test.hpp"
#include <d3d11.h>

using namespace esia;
using esia::rhi::d3d::ComPtr;

ESIA_TEST(D3D11Host, WrapDrawRestore)
{
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> ctx;
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &device, nullptr, &ctx)))
    {
        std::printf("  skipped: no D3D11 device\n");
        return;
    }
    auto dev = rhi::d3d11::CreateDevice(device.Get(), ctx.Get(), true);
    ESIA_CHECK(dev != nullptr);
    if (!dev)
        return;

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = 32;
    td.Height = 16;
    td.MipLevels = td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
    td.SampleDesc.Count = 1;
    td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    ComPtr<ID3D11Texture2D> tex;
    ESIA_CHECK(SUCCEEDED(device->CreateTexture2D(&td, nullptr, &tex)));
    D3D11_RENDER_TARGET_VIEW_DESC rv = {};
    rv.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rv.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    ComPtr<ID3D11RenderTargetView> rtv;
    ESIA_CHECK(SUCCEEDED(device->CreateRenderTargetView(tex.Get(), &rv, &rtv)));

    const rhi::Texture t = rhi::d3d11::WrapRenderTarget(*dev, rtv.Get());
    ESIA_CHECK((bool)t);
    ESIA_CHECK(rhi::d3d11::WrapRenderTarget(*dev, rtv.Get()) == t);   // cached per view
    const rhi::TextureDesc d = dev->GetTextureDesc(t);
    ESIA_CHECK(d.width == 32 && d.height == 16 && d.format == rhi::Format::RGBA8_SRGB);
    ESIA_CHECK((d.usage & rhi::TextureUsage_Sampled) && (d.usage & rhi::TextureUsage_RenderTarget));

    // host state that must survive the frame
    const D3D11_VIEWPORT hostVp = {3.0f, 4.0f, 5.0f, 6.0f, 0.0f, 1.0f};
    ctx->RSSetViewports(1, &hostVp);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    rhi::FrameDesc fd;
    ESIA_CHECK(dev->BeginFrame(fd));
    rhi::PassDesc p;
    p.target = t;
    p.load = rhi::LoadOp::Clear;
    p.clearColor[0] = 0.5f;   // stored value: 128 through the sRGB view
    p.clearColor[3] = 1.0f;
    dev->BeginPass(p);
    ESIA_CHECK(dev->NativeRenderState() == ctx.Get());
    dev->EndPass();
    dev->EndFrame();

    UINT n = 1;
    D3D11_VIEWPORT vp = {};
    ctx->RSGetViewports(&n, &vp);
    ESIA_CHECK(n == 1 && vp.TopLeftX == 3.0f && vp.Width == 5.0f);
    D3D11_PRIMITIVE_TOPOLOGY topo;
    ctx->IAGetPrimitiveTopology(&topo);
    ESIA_CHECK(topo == D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    std::vector<std::uint8_t> px;
    ESIA_CHECK(dev->ReadPixels(t, rhi::IRect{31, 15, 32, 16}, px) && px.size() == 4);
    if (px.size() == 4)
        ESIA_CHECK(std::abs(px[0] - 128) <= 1 && px[1] == 0 && px[3] == 255);
    dev->DestroyTexture(t);
}
