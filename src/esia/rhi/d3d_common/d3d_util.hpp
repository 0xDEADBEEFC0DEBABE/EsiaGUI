// Esia - private helpers of the Direct3D backends: a COM pointer, logging, handle tables, the DXGI format tables,
// readback conversion to RGBA8 and GPU timestamp accounting. No D3D device header here (only DXGI's formats), so
// every backend can include it whatever else it builds with.
#pragma once
#include "esia/rhi/d3d_common.hpp"
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dxgiformat.h>

namespace esia::rhi::d3d
{
    // ------------------------------------------------------------------ COM
    // A small intrusive COM pointer: Microsoft::WRL::ComPtr is not in every mingw-w64 release, and this is all the
    // backends need.
    template <class T>
    class ComPtr
    {
    public:
        ComPtr() = default;
        ComPtr(std::nullptr_t) {}
        // Takes a new reference on `p` (like WRL's ComPtr); Attach adopts one.
        ComPtr(T* p) : p_(p) { AddRef(); }
        ComPtr(const ComPtr& o) : p_(o.p_) { AddRef(); }
        ComPtr(ComPtr&& o) noexcept : p_(std::exchange(o.p_, nullptr)) {}
        ~ComPtr() { Reset(); }
        ComPtr& operator=(const ComPtr& o)
        {
            ComPtr(o).Swap(*this);
            return *this;
        }
        ComPtr& operator=(ComPtr&& o) noexcept
        {
            ComPtr(std::move(o)).Swap(*this);
            return *this;
        }
        ComPtr& operator=(std::nullptr_t)
        {
            Reset();
            return *this;
        }

        T* Get() const { return p_; }
        T* operator->() const { return p_; }
        explicit operator bool() const { return p_ != nullptr; }
        // For out parameters (Create*(..., &ptr)): releases what it held.
        T** operator&()
        {
            Reset();
            return &p_;
        }
        T* const* GetAddressOf() const { return &p_; }
        void Reset()
        {
            if (p_)
                std::exchange(p_, nullptr)->Release();
        }
        void Attach(T* p)
        {
            Reset();
            p_ = p;
        }
        T* Detach() { return std::exchange(p_, nullptr); }
        void Swap(ComPtr& o) { std::swap(p_, o.p_); }
        template <class U>
        HRESULT As(ComPtr<U>& out) const
        {
            return p_ ? p_->QueryInterface(__uuidof(U), reinterpret_cast<void**>(&out)) : E_POINTER;
        }

    private:
        void AddRef()
        {
            if (p_)
                p_->AddRef();
        }
        T* p_ = nullptr;
    };

    // ------------------------------------------------------------------ logging
    class Logger
    {
    public:
        Logger() = default;
        explicit Logger(const DebugDesc& d) : desc_(d) {}
        const DebugDesc& Desc() const { return desc_; }
        bool DebugLayer() const { return desc_.debugLayer; }
        void Log(LogLevel level, const std::string& message) const;
        void Printf(LogLevel level, const char* fmt, ...) const ESIA_PRINTF(3, 4);
        // "<what> failed: 0x887A0005 (DXGI_ERROR_DEVICE_REMOVED)"; returns false for FAILED(hr)
        bool Check(HRESULT hr, const char* what) const;
        // Messages logged at warning or error level so far - the debug layers' and the backend's own (failed calls,
        // refused requests): the device's Device::ValidationErrors. Copies of a logger share the count.
        std::uint32_t Problems() const { return problems_->load(); }

    private:
        DebugDesc desc_;
        // not make_shared: its control block compares type_info, which clang with mingw-w64's libstdc++ 13 defines
        // twice (a duplicate symbol at link time in the windows-mingw-cross build)
        std::shared_ptr<std::atomic<std::uint32_t>> problems_{new std::atomic<std::uint32_t>(0u)};
    };

    const char* HResultName(HRESULT hr);

    // ESIA_D3D_DEBUG (0, 1 = debug layer, 2 = also GPU-based validation on D3D12): the headless devices' switch
    int DebugLevelFromEnvironment();
    // ESIA_D3D_DRIVER=warp: headless D3D10 / 11 / 12 devices on the WARP software rasterizer (else hardware)
    bool UseWarpFromEnvironment();

    // ------------------------------------------------------------------ handles
    // Handles are numbered from 1 in creation order and never reused (a stale handle finds nothing).
    template <class T>
    class HandleTable
    {
    public:
        std::uint32_t Add(T&& value)
        {
            const std::uint32_t id = next_++;
            items_.emplace(id, std::move(value));
            return id;
        }
        T* Find(std::uint32_t id)
        {
            auto it = items_.find(id);
            return it != items_.end() ? &it->second : nullptr;
        }
        const T* Find(std::uint32_t id) const
        {
            auto it = items_.find(id);
            return it != items_.end() ? &it->second : nullptr;
        }
        bool Remove(std::uint32_t id) { return items_.erase(id) != 0; }
        void Clear() { items_.clear(); }
        template <class F>
        void ForEach(F&& f)
        {
            for (auto& [id, v] : items_)
                f(id, v);
        }

    private:
        std::unordered_map<std::uint32_t, T> items_;
        std::uint32_t next_ = 1;
    };

    // ------------------------------------------------------------------ DXGI formats (D3D10 / 11 / 12)
    struct DxgiFormats
    {
        DXGI_FORMAT storage = DXGI_FORMAT_UNKNOWN;   // resource format (typeless for 8-bit RGBA / BGRA)
        DXGI_FORMAT srv = DXGI_FORMAT_UNKNOWN;       // shader resource view: never sRGB (sampling is raw, rhi.hpp)
        DXGI_FORMAT rtv = DXGI_FORMAT_UNKNOWN;       // render target view: sRGB for *_SRGB (the hardware encodes)
    };
    // 8-bit RGBA / BGRA textures are stored typeless so that a format and its RawFormat share bits: raw views of
    // sRGB targets, CopySubresourceRegion / ResolveSubresource between the target and its backdrop copy.
    DxgiFormats DxgiFormatsOf(Format f);
    // The rhi format of a host view (RTV) format; Unknown if the renderer cannot target it.
    Format FormatFromDxgi(DXGI_FORMAT f);
    // R8G8B8A8_UNORM_SRGB -> R8G8B8A8_TYPELESS ...; other formats map to themselves
    DXGI_FORMAT DxgiTypeless(DXGI_FORMAT f);
    // The non-sRGB typed format of a family (R8G8B8A8_TYPELESS / _UNORM_SRGB -> _UNORM ...)
    DXGI_FORMAT DxgiRaw(DXGI_FORMAT f);
    bool DxgiIsSrgb(DXGI_FORMAT f);

    // ------------------------------------------------------------------ readback
    // Converts `height` rows of `width` pixels laid out as `layout` (an rhi::Format describing the bytes in memory:
    // BGRA8_* for D3D9's A8R8G8B8, RGBA16_FLOAT, RGBA32_FLOAT, RGB10A2_UNORM, R8_UNORM ...) into RGBA8, top row
    // first. Float values are clamped to [0, 1]; sRGB values are returned as stored; R8 becomes (r, 0, 0, 255).
    void ConvertToRgba8(Format layout, const void* src, std::size_t rowPitch, int width, int height, std::vector<std::uint8_t>& out);

    // Clear colors are stored values (the shaders work on stored values, rhi.hpp): through an sRGB view the
    // hardware encodes the clear value, so it is decoded first.
    void ClearValueFor(Format f, const float in[4], float out[4]);

    // ------------------------------------------------------------------ GPU timestamps
    // The intervals of one frame (BeginProfile / EndProfile scopes as indices of its timestamps) and what they add
    // up to once the timestamps were read.
    struct ProfileFrame
    {
        struct Interval
        {
            int start, end;
            ProfileCategory category;
        };
        std::uint64_t frame = 0;
        int frameStart = -1, frameEnd = -1;
        int openStart = -1;
        ProfileCategory openCategory = ProfileCategory::Count;
        std::vector<Interval> intervals;

        void Reset(std::uint64_t f)
        {
            frame = f;
            frameStart = frameEnd = openStart = -1;
            intervals.clear();
        }
        // ticks: every timestamp of the frame; frequency: ticks per second
        GpuProfile Resolve(const std::uint64_t* ticks, int count, double frequency) const;
    };
}
