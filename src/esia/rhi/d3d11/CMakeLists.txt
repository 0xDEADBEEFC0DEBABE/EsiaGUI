# Direct3D 11 backend (ESIA_BACKEND_D3D11): the rhi::Device on a host's ID3D11Device + immediate context, and a
# headless device for the conformance suite (esia_conformance --backend d3d11).
add_library(esia_rhi_d3d11 STATIC d3d11_device.cpp)
target_include_directories(esia_rhi_d3d11 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(esia_rhi_d3d11 PUBLIC esia_rhi_d3d_common PRIVATE d3d11 dxgi dxguid)
esia_target_defaults(esia_rhi_d3d11)
esia_register_backend(d3d11 esia_rhi_d3d11)

if(BUILD_TESTING)
    esia_add_test(esia_rhi_d3d11_tests tests/test_d3d11_host.cpp)
    target_link_libraries(esia_rhi_d3d11_tests PRIVATE esia_rhi_d3d11 d3d11)
endif()
