// WGT UI - built-in backend factories
#pragma once
#include "wgt/backend.hpp"
#include <memory>

namespace wgt
{
    std::unique_ptr<IRenderBackend> CreateD3D11Backend(ID3D11Device* device, ID3D11DeviceContext* context, bool restoreState);
    std::unique_ptr<IRenderBackend> CreateD3D12Backend(ID3D12Device* device, std::uint32_t framesInFlight);
}
