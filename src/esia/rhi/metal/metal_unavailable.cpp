// Esia - the Metal backend's registration on hosts without Metal (Linux, Windows): the backend's logic
// (esia_rhi_metal_core) is built and tested there, but it can only draw through metal_device.mm on Apple
// platforms. The conformance suite then reports SKIP with the reason instead of failing.
#include "esia/rhi/backend_registry.hpp"

namespace
{
    esia::rhi::HeadlessDevice CreateHeadlessMetal(const esia::rhi::HeadlessDesc&, std::string& error)
    {
        error = "Metal runs on macOS / iOS only (this build has the backend's portable parts, tested by esia_rhi_metal_tests)";
        return {};
    }
}

void EsiaRegisterBackend_metal()
{
    esia::rhi::BackendInfo info;
    info.name = "metal";
    info.createHeadless = &CreateHeadlessMetal;
    esia::rhi::RegisterBackend(info);
}
