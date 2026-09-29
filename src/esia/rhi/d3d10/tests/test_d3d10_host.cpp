// Direct3D 11 host integration: a host render target view wrapped (an sRGB view of typeless storage: sampleable
// through its raw view), cached per view, drawn into, read back; the host's pipeline state restored after a frame.
#include "esia/rhi/d3d10.hpp"
#include "d3d_util.hpp"
#include "esia_test.hpp"
#include <d3d10.h>

using namespace esia;
using esia::rhi::d3d::ComPtr;

ESIA_TEST(D3D10Host, WrapDrawRestore)
{
    ComPtr<ID3D10Device> device;
    if (FAILED(D3D10CreateDevice(nullptr, D3D10_DRIVER_TYPE_HARDWARE, nullptr, 0, D3D10_SDK_VERSION, &device)))
    {
        std::printf("  skipped: no D3D10 device\n");
        return;
    }
    auto dev = rhi::d3d10::CreateDevice(device.Get(), true);
    ESIA_CHECK(dev != nullptr);
    if (!dev)
        return;

    D3D10_TEXTURE2D_DESC td = {};
    td.Width = 32;
    td.Height = 16;
    td.MipLevels = td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
    td.SampleDesc.Count = 1;
    td.Usage = D3D10_USAGE_DEFAULT;
    td.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
    ComPtr<ID3D10Texture2D> tex;
    ESIA_CHECK(SUCCEEDED(device->CreateTexture2D(&td, nullptr, &tex)));
    D3D10_RENDER_TARGET_VIEW_DESC rv = {};
    rv.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rv.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
    ComPtr<ID3D10RenderTargetView> rtv;
    ESIA_CHECK(SUCCEEDED(device->CreateRenderTargetView(tex.Get(), &rv, &rtv)));

    const rhi::Texture t = rhi::d3d10::WrapRenderTarget(*dev, rtv.Get());
    ESIA_CHECK((bool)t);
    ESIA_CHECK(rhi::d3d10::WrapRenderTarget(*dev, rtv.Get()) == t);   // cached per view
    const rhi::TextureDesc d = dev->GetTextureDesc(t);
    ESIA_CHECK(d.width == 32 && d.height == 16 && d.format == rhi::Format::RGBA8_SRGB);
    ESIA_CHECK((d.usage & rhi::TextureUsage_Sampled) && (d.usage & rhi::TextureUsage_RenderTarget));

    // host state that must survive the frame
    const D3D10_VIEWPORT hostVp = {3, 4, 5, 6, 0.0f, 1.0f};
    device->RSSetViewports(1, &hostVp);
    device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

    rhi::FrameDesc fd;
    ESIA_CHECK(dev->BeginFrame(fd));
    rhi::PassDesc p;
    p.target = t;
    p.load = rhi::LoadOp::Clear;
    p.clearColor[0] = 0.5f;   // stored value: 128 through the sRGB view
    p.clearColor[3] = 1.0f;
    dev->BeginPass(p);
    ESIA_CHECK(dev->NativeRenderState() == device.Get());
    dev->EndPass();
    dev->EndFrame();

    UINT n = 1;
    D3D10_VIEWPORT vp = {};
    device->RSGetViewports(&n, &vp);
    ESIA_CHECK(n == 1 && vp.TopLeftX == 3 && vp.Width == 5);
    D3D10_PRIMITIVE_TOPOLOGY topo;
    device->IAGetPrimitiveTopology(&topo);
    ESIA_CHECK(topo == D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

    std::vector<std::uint8_t> px;
    ESIA_CHECK(dev->ReadPixels(t, rhi::IRect{31, 15, 32, 16}, px) && px.size() == 4);
    if (px.size() == 4)
        ESIA_CHECK(std::abs(px[0] - 128) <= 1 && px[1] == 0 && px[3] == 255);
    dev->DestroyTexture(t);
}
