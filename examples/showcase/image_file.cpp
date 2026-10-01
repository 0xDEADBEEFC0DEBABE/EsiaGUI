// showcase - image files through the Windows Imaging Component.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "image_file.hpp"
#include <algorithm>
#include <cmath>
#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>

namespace showcase
{
    bool LoadImageFile(const std::wstring& path, ImageFile& out, int maxSide)
    {
        using Microsoft::WRL::ComPtr;
        const HRESULT com = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);   // this thread may have COM already
        const bool uninit = SUCCEEDED(com);
        bool ok = false;
        {
            ComPtr<IWICImagingFactory> factory;
            ComPtr<IWICBitmapDecoder> decoder;
            ComPtr<IWICBitmapFrameDecode> frame;
            ComPtr<IWICFormatConverter> converter;
            UINT w = 0, h = 0;
            if (SUCCEEDED(::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory))) &&
                SUCCEEDED(factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)) &&
                SUCCEEDED(decoder->GetFrame(0, &frame)) && SUCCEEDED(frame->GetSize(&w, &h)) && w > 0 && h > 0)
            {
                ComPtr<IWICBitmapSource> source = frame;
                if (maxSide > 0 && (int)std::max(w, h) > maxSide)
                {
                    const double k = (double)maxSide / (double)std::max(w, h);
                    const UINT sw = std::max(1u, (UINT)std::lround(w * k)), sh = std::max(1u, (UINT)std::lround(h * k));
                    ComPtr<IWICBitmapScaler> scaler;
                    if (SUCCEEDED(factory->CreateBitmapScaler(&scaler)) && SUCCEEDED(scaler->Initialize(frame.Get(), sw, sh, WICBitmapInterpolationModeFant)))
                    {
                        source = scaler;
                        w = sw;
                        h = sh;
                    }
                }
                if (SUCCEEDED(factory->CreateFormatConverter(&converter)) &&
                    SUCCEEDED(converter->Initialize(source.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom)))
                {
                    out.width = (int)w;
                    out.height = (int)h;
                    out.rgba.resize((std::size_t)w * h * 4);
                    ok = SUCCEEDED(converter->CopyPixels(nullptr, w * 4, (UINT)out.rgba.size(), out.rgba.data()));
                }
            }
        }
        if (uninit)
            ::CoUninitialize();
        return ok;
    }
}
