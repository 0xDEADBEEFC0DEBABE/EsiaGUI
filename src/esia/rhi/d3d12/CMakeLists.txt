# Direct3D 12 backend (ESIA_BACKEND_D3D12): the rhi::Device recording into a host's command list (or its own, on a
# host queue), and a headless device for the conformance suite (esia_conformance --backend d3d12).
add_library(esia_rhi_d3d12 STATIC d3d12_device.cpp)
target_include_directories(esia_rhi_d3d12 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(esia_rhi_d3d12 PUBLIC esia_rhi_d3d_common PRIVATE d3d12 dxgi dxguid)
esia_target_defaults(esia_rhi_d3d12)
esia_register_backend(d3d12 esia_rhi_d3d12)

if(BUILD_TESTING)
    esia_add_test(esia_rhi_d3d12_tests tests/test_d3d12_host.cpp)
    target_link_libraries(esia_rhi_d3d12_tests PRIVATE esia_rhi_d3d12 esia_conformance_scenes d3d12)
endif()
