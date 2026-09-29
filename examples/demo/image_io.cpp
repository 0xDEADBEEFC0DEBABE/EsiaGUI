// WGT demo - WIC image I/O
#include "image_io.hpp"
#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <algorithm>

using Microsoft::WRL::ComPtr;

namespace
{
    ComPtr<IWICImagingFactory> Factory()
    {
        static ComPtr<IWICImagingFactory> factory;
        if (!factory)
        {
            CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
        }
        return factory;
    }
}

bool LoadImageRGBA(const std::wstring& path, ImageRGBA& out, int maxDimension)
{
    ComPtr<IWICImagingFactory> f = Factory();
    if (!f)
        return false;
    ComPtr<IWICBitmapDecoder> decoder;
    if (FAILED(f->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)))
        return false;
    ComPtr<IWICBitmapFrameDecode> frame;
    if (FAILED(decoder->GetFrame(0, &frame)))
        return false;
    UINT w = 0, h = 0;
    frame->GetSize(&w, &h);
    ComPtr<IWICBitmapSource> source = frame;
    if (maxDimension > 0 && (int)std::max(w, h) > maxDimension)
    {
        const double k = (double)maxDimension / (double)std::max(w, h);
        ComPtr<IWICBitmapScaler> scaler;
        f->CreateBitmapScaler(&scaler);
        const UINT nw = (UINT)(w * k), nh = (UINT)(h * k);
        if (scaler && SUCCEEDED(scaler->Initialize(frame.Get(), nw, nh, WICBitmapInterpolationModeHighQualityCubic)))
        {
            source = scaler;
            w = nw;
            h = nh;
        }
    }
    ComPtr<IWICFormatConverter> conv;
    f->CreateFormatConverter(&conv);
    if (!conv || FAILED(conv->Initialize(source.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom)))
        return false;
    out.width = (int)w;
    out.height = (int)h;
    out.pixels.resize((size_t)w * h * 4);
    return SUCCEEDED(conv->CopyPixels(nullptr, w * 4, (UINT)out.pixels.size(), out.pixels.data()));
}

bool SavePng(const std::wstring& path, const std::uint8_t* pixels, int width, int height, int rowPitch, bool bgra)
{
    ComPtr<IWICImagingFactory> f = Factory();
    if (!f)
        return false;
    ComPtr<IWICStream> stream;
    if (FAILED(f->CreateStream(&stream)) || FAILED(stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE)))
        return false;
    ComPtr<IWICBitmapEncoder> encoder;
    if (FAILED(f->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder)) || FAILED(encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache)))
        return false;
    ComPtr<IWICBitmapFrameEncode> frame;
    if (FAILED(encoder->CreateNewFrame(&frame, nullptr)) || FAILED(frame->Initialize(nullptr)))
        return false;
    frame->SetSize((UINT)width, (UINT)height);
    WICPixelFormatGUID fmt = GUID_WICPixelFormat32bppBGRA;
    frame->SetPixelFormat(&fmt);
    std::vector<std::uint8_t> row((size_t)width * 4 * height);
    for (int y = 0; y < height; ++y)
    {
        const std::uint8_t* src = pixels + (size_t)y * rowPitch;
        std::uint8_t* dst = row.data() + (size_t)y * width * 4;
        for (int x = 0; x < width; ++x)
        {
            const std::uint8_t* s = src + x * 4;
            std::uint8_t* d = dst + x * 4;
            d[0] = bgra ? s[0] : s[2];
            d[1] = s[1];
            d[2] = bgra ? s[2] : s[0];
            d[3] = 255;
        }
    }
    if (FAILED(frame->WritePixels((UINT)height, (UINT)width * 4, (UINT)row.size(), row.data())))
        return false;
    return SUCCEEDED(frame->Commit()) && SUCCEEDED(encoder->Commit());
}
