# Direct3D 10 backend (ESIA_BACKEND_D3D10): the rhi::Device on a host's ID3D10Device (10.0 / 10.1, shader model 4),
# and a headless device for the conformance suite (esia_conformance --backend d3d10).
add_library(esia_rhi_d3d10 STATIC d3d10_device.cpp)
target_include_directories(esia_rhi_d3d10 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(esia_rhi_d3d10 PUBLIC esia_rhi_d3d_common PRIVATE d3d10 dxgi dxguid)
esia_target_defaults(esia_rhi_d3d10)
esia_register_backend(d3d10 esia_rhi_d3d10)

if(BUILD_TESTING)
    esia_add_test(esia_rhi_d3d10_tests tests/test_d3d10_host.cpp)
    target_link_libraries(esia_rhi_d3d10_tests PRIVATE esia_rhi_d3d10 d3d10)
endif()
