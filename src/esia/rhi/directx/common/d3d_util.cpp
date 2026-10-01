// Esia - private helpers of the Direct3D backends (see d3d_util.hpp)
#include "d3d_util.hpp"
#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dxgi1_6.h>

namespace esia::rhi::d3d
{
    // ------------------------------------------------------------------ logging
    void Logger::Log(LogLevel level, const std::string& message) const
    {
        if (level != LogLevel::Info)
            ++*problems_;
        if (desc_.log)
        {
            desc_.log(desc_.logUser, level, message.c_str());
            return;
        }
        static const char* kLevels[] = {"error", "warning", "info"};
        const std::string line = std::string("esia d3d ") + kLevels[(int)level] + ": " + message + "\n";
        OutputDebugStringA(line.c_str());
        std::fputs(line.c_str(), stderr);
    }

    void Logger::Printf(LogLevel level, const char* fmt, ...) const
    {
        char buf[1024];
        va_list ap;
        va_start(ap, fmt);
        std::vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        Log(level, buf);
    }

    bool Logger::Check(HRESULT hr, const char* what) const
    {
        if (SUCCEEDED(hr))
            return true;
        Printf(LogLevel::Error, "%s failed: 0x%08lX (%s)", what, (unsigned long)hr, HResultName(hr));
        return false;
    }

    const char* HResultName(HRESULT hr)
    {
        switch ((unsigned long)hr)
        {
        case 0x80004001ul: return "E_NOTIMPL";
        case 0x80004002ul: return "E_NOINTERFACE";
        case 0x80004003ul: return "E_POINTER";
        case 0x80004005ul: return "E_FAIL";
        case 0x8007000Eul: return "E_OUTOFMEMORY";
        case 0x80070057ul: return "E_INVALIDARG";
        case 0x887A0001ul: return "DXGI_ERROR_INVALID_CALL";
        case 0x887A0004ul: return "DXGI_ERROR_UNSUPPORTED";
        case 0x887A0005ul: return "DXGI_ERROR_DEVICE_REMOVED";
        case 0x887A0006ul: return "DXGI_ERROR_DEVICE_HUNG";
        case 0x887A0007ul: return "DXGI_ERROR_DEVICE_RESET";
        case 0x887A0020ul: return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
        case 0x887A002Dul: return "DXGI_ERROR_SDK_COMPONENT_MISSING";
        case 0x8876086Cul: return "D3DERR_INVALIDCALL";
        case 0x8876017Cul: return "D3DERR_OUTOFVIDEOMEMORY";
        case 0x88760868ul: return "D3DERR_DEVICELOST";
        case 0x8876086Aul: return "D3DERR_NOTAVAILABLE";
        case 0x88760B59ul: return "D3DCompile: syntax error";
        }
        return "unknown";
    }

    int DebugLevelFromEnvironment()
    {
        const char* v = std::getenv("ESIA_D3D_DEBUG");
        return v ? std::atoi(v) : 0;
    }

    bool UseWarpFromEnvironment()
    {
        const char* v = std::getenv("ESIA_D3D_DRIVER");
        return v && (std::strcmp(v, "warp") == 0 || std::strcmp(v, "WARP") == 0);
    }

    ComPtr<IDXGIAdapter> AdapterFromEnvironment()
    {
        const char* v = std::getenv("ESIA_D3D_ADAPTER");
        if (!v || (std::strcmp(v, "high-performance") != 0 && std::strcmp(v, "minimum-power") != 0))
            return {};
        const DXGI_GPU_PREFERENCE preference =
            std::strcmp(v, "high-performance") == 0 ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_MINIMUM_POWER;
        ComPtr<IDXGIFactory6> factory;   // Windows 10 1803 and later
        ComPtr<IDXGIAdapter> adapter;
        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))) || FAILED(factory->EnumAdapterByGpuPreference(0, preference, IID_PPV_ARGS(&adapter))))
            return {};
        return adapter;
    }

    namespace
    {
        std::string Description(IDXGIAdapter* adapter)
        {
            DXGI_ADAPTER_DESC d = {};
            if (!adapter || FAILED(adapter->GetDesc(&d)))
                return {};
            char name[sizeof(d.Description) * 2] = {};
            WideCharToMultiByte(CP_UTF8, 0, d.Description, -1, name, (int)sizeof(name), nullptr, nullptr);
            return name;
        }
    }

    std::string AdapterName(IUnknown* dxgiDevice)
    {
        ComPtr<IDXGIDevice> device;
        ComPtr<IDXGIAdapter> adapter;
        if (!dxgiDevice || FAILED(dxgiDevice->QueryInterface(IID_PPV_ARGS(&device))) || FAILED(device->GetAdapter(&adapter)))
            return {};
        return Description(adapter.Get());
    }

    std::string AdapterName(LUID adapterLuid)
    {
        ComPtr<IDXGIFactory4> factory;
        ComPtr<IDXGIAdapter> adapter;
        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))) || FAILED(factory->EnumAdapterByLuid(adapterLuid, IID_PPV_ARGS(&adapter))))
            return {};
        return Description(adapter.Get());
    }

    // ------------------------------------------------------------------ formats
    DxgiFormats DxgiFormatsOf(Format f)
    {
        switch (f)
        {
        case Format::RGBA8_UNORM: return {DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM};
        case Format::RGBA8_SRGB: return {DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB};
        case Format::BGRA8_UNORM: return {DXGI_FORMAT_B8G8R8A8_TYPELESS, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM};
        case Format::BGRA8_SRGB: return {DXGI_FORMAT_B8G8R8A8_TYPELESS, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB};
        case Format::RGB10A2_UNORM: return {DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_FORMAT_R10G10B10A2_UNORM};
        case Format::RGBA16_FLOAT: return {DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT};
        case Format::RGBA32_FLOAT: return {DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT};
        case Format::R8_UNORM: return {DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM};
        case Format::Unknown: break;
        }
        return {};
    }

    Format FormatFromDxgi(DXGI_FORMAT f)
    {
        switch (f)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM: return Format::RGBA8_UNORM;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return Format::RGBA8_SRGB;
        case DXGI_FORMAT_B8G8R8A8_UNORM: return Format::BGRA8_UNORM;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return Format::BGRA8_SRGB;
        case DXGI_FORMAT_R10G10B10A2_UNORM: return Format::RGB10A2_UNORM;
        case DXGI_FORMAT_R16G16B16A16_FLOAT: return Format::RGBA16_FLOAT;
        case DXGI_FORMAT_R32G32B32A32_FLOAT: return Format::RGBA32_FLOAT;
        case DXGI_FORMAT_R8_UNORM: return Format::R8_UNORM;
        default: return Format::Unknown;
        }
    }

    DXGI_FORMAT DxgiTypeless(DXGI_FORMAT f)
    {
        switch (f)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_TYPELESS;
        default: return f;
        }
    }

    DXGI_FORMAT DxgiRaw(DXGI_FORMAT f)
    {
        switch (f)
        {
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM;
        case DXGI_FORMAT_R10G10B10A2_TYPELESS: return DXGI_FORMAT_R10G10B10A2_UNORM;
        case DXGI_FORMAT_R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case DXGI_FORMAT_R32G32B32A32_TYPELESS: return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case DXGI_FORMAT_R8_TYPELESS: return DXGI_FORMAT_R8_UNORM;
        default: return f;
        }
    }

    bool DxgiIsSrgb(DXGI_FORMAT f) { return f == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB || f == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; }

    // ------------------------------------------------------------------ readback
    namespace
    {
        std::uint8_t ToUnorm8(float v)
        {
            if (!(v > 0.0f))   // NaN too
                return 0;
            return v >= 1.0f ? 255 : (std::uint8_t)std::lround(v * 255.0f);
        }

        float HalfToFloat(std::uint16_t h)
        {
            const int sign = (h >> 15) & 1, exp = (h >> 10) & 0x1F, mant = h & 0x3FF;
            float v;
            if (exp == 0)
                v = std::ldexp((float)mant, -24);
            else if (exp == 31)
                v = mant ? NAN : INFINITY;
            else
                v = std::ldexp((float)(mant | 0x400), exp - 25);
            return sign ? -v : v;
        }
    }

    void ConvertToRgba8(Format layout, const void* src, std::size_t rowPitch, int width, int height, std::vector<std::uint8_t>& out)
    {
        out.resize((std::size_t)width * (std::size_t)height * 4);
        for (int y = 0; y < height; ++y)
        {
            const std::uint8_t* row = static_cast<const std::uint8_t*>(src) + rowPitch * (std::size_t)y;
            std::uint8_t* o = out.data() + (std::size_t)y * (std::size_t)width * 4;
            for (int x = 0; x < width; ++x, o += 4)
            {
                switch (layout)
                {
                case Format::RGBA8_UNORM:
                case Format::RGBA8_SRGB: std::memcpy(o, row + x * 4, 4); break;
                case Format::BGRA8_UNORM:
                case Format::BGRA8_SRGB:
                    o[0] = row[x * 4 + 2];
                    o[1] = row[x * 4 + 1];
                    o[2] = row[x * 4 + 0];
                    o[3] = row[x * 4 + 3];
                    break;
                case Format::RGB10A2_UNORM:
                {
                    std::uint32_t v;
                    std::memcpy(&v, row + x * 4, 4);
                    o[0] = ToUnorm8((float)(v & 0x3FF) / 1023.0f);
                    o[1] = ToUnorm8((float)((v >> 10) & 0x3FF) / 1023.0f);
                    o[2] = ToUnorm8((float)((v >> 20) & 0x3FF) / 1023.0f);
                    o[3] = ToUnorm8((float)(v >> 30) / 3.0f);
                    break;
                }
                case Format::RGBA16_FLOAT:
                    for (int c = 0; c < 4; ++c)
                    {
                        std::uint16_t h;
                        std::memcpy(&h, row + x * 8 + c * 2, 2);
                        o[c] = ToUnorm8(HalfToFloat(h));
                    }
                    break;
                case Format::RGBA32_FLOAT:
                    for (int c = 0; c < 4; ++c)
                    {
                        float f;
                        std::memcpy(&f, row + x * 16 + c * 4, 4);
                        o[c] = ToUnorm8(f);
                    }
                    break;
                case Format::R8_UNORM:
                    o[0] = row[x];
                    o[1] = o[2] = 0;
                    o[3] = 255;
                    break;
                case Format::Unknown: std::memset(o, 0, 4); break;
                }
            }
        }
    }

    void ClearValueFor(Format f, const float in[4], float out[4])
    {
        for (int c = 0; c < 4; ++c)
            out[c] = in[c];
        if (!IsSrgb(f))
            return;
        for (int c = 0; c < 3; ++c)
        {
            const float v = std::clamp(in[c], 0.0f, 1.0f);
            out[c] = v <= 0.04045f ? v / 12.92f : std::pow((v + 0.055f) / 1.055f, 2.4f);
        }
    }

    // ------------------------------------------------------------------ GPU timestamps
    GpuProfile ProfileFrame::Resolve(const std::uint64_t* ticks, int count, double frequency) const
    {
        GpuProfile p;
        if (frameStart < 0 || frameEnd < 0 || frameEnd >= count || frequency <= 0.0)
            return p;
        const double ms = 1000.0 / frequency;
        auto span = [&](int a, int b) { return ticks[b] >= ticks[a] ? (double)(ticks[b] - ticks[a]) * ms : 0.0; };
        double sums[(int)ProfileCategory::Count] = {};
        for (const Interval& iv : intervals)
            if (iv.start >= 0 && iv.end < count)
                sums[(int)iv.category] += span(iv.start, iv.end);
        p.valid = true;
        p.frame = frame;
        p.totalMs = (float)span(frameStart, frameEnd);
        for (int i = 0; i < (int)ProfileCategory::Count; ++i)
            p.categoryMs[i] = (float)sums[i];
        return p;
    }
}
