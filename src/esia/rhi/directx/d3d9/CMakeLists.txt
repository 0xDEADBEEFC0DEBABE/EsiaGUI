# Direct3D 9 backend (ESIA_BACKEND_DIRECTX, ESIA_BACKEND_D3D9): the rhi::Device on a host's IDirect3DDevice9(Ex) with shader model 3, and a
# headless device for the conformance suite (esia_conformance --backend d3d9). esia_sm3_prelude.hlsli (the SM3
# portability hooks, also read by tools/shaders/build_shaders.py) is embedded for runtime compilation.
set(_prelude ${CMAKE_CURRENT_SOURCE_DIR}/esia_sm3_prelude.hlsli)
set(_embedded ${CMAKE_CURRENT_BINARY_DIR}/generated/esia_sm3_prelude.cpp)
add_custom_command(
    OUTPUT ${_embedded}
    COMMAND ${CMAKE_COMMAND} -DOUT=${_embedded} -DNAMESPACE=esia::rhi::d3d9::blobs "-DPAIRS=esia_sm3_prelude_hlsli=${_prelude}" -P ${ESIA_ROOT}/cmake/EsiaEmbed.cmake
    DEPENDS ${_prelude} ${ESIA_ROOT}/cmake/EsiaEmbed.cmake
    COMMENT "Embedding the SM3 prelude"
    VERBATIM)

add_library(esia_rhi_d3d9 STATIC d3d9_device.cpp ${_embedded})
target_include_directories(esia_rhi_d3d9 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(esia_rhi_d3d9 PUBLIC esia_rhi_d3d_common PRIVATE d3d9)
esia_target_defaults(esia_rhi_d3d9)
esia_register_backend(d3d9 esia_rhi_d3d9)

if(BUILD_TESTING)
    esia_add_test(esia_rhi_d3d9_tests tests/test_d3d9.cpp)
    target_link_libraries(esia_rhi_d3d9_tests PRIVATE esia_rhi_d3d9 d3d9)
endif()
