// Esia - what the four Direct3D backends (d3d9, d3d10, d3d11, d3d12) share in their host integration headers:
// where their messages go and whether they forward the Direct3D debug layer's.
#pragma once
#include "esia/rhi/rhi.hpp"

namespace esia::rhi::d3d
{
    enum class LogLevel : int { Error = 0, Warning = 1, Info = 2 };

    // Receives the backend's messages: failed API calls, shader compiler errors and warnings, and - with
    // DebugDesc::debugLayer - every message of the debug layer's info queue. Called on the render thread (shader
    // warnings of user effects: on the thread that compiled them). Null = OutputDebugString + stderr.
    using LogFn = void (*)(void* user, LogLevel level, const char* message);

    struct DebugDesc
    {
        // Forward the debug layer's messages (ID3D10InfoQueue / ID3D11InfoQueue / ID3D12InfoQueue) to `log` after
        // every frame and every failed call. The layer itself is the host's: it creates its device with
        // D3D1x_CREATE_DEVICE_DEBUG (D3D12: ID3D12Debug::EnableDebugLayer first). Direct3D 9 has no info queue on
        // Windows 10 / 11: there the flag only makes failed calls more verbose. Headless devices (the conformance
        // suite) enable the layer themselves when the environment variable ESIA_D3D_DEBUG is 1 (or 2: D3D12 also
        // turns on GPU-based validation).
        bool debugLayer = false;
        LogFn log = nullptr;
        void* logUser = nullptr;
    };
}
