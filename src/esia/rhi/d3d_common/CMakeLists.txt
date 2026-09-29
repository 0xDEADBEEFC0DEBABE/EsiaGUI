# What the Direct3D 9 / 10 / 11 / 12 backends share (built when any ESIA_BACKEND_D3D* is ON): the host
# integration types (include/esia/rhi/d3d_common.hpp), a COM pointer, logging, the DXGI format tables, readback
# conversion, GPU timestamp accounting and runtime HLSL compilation (D3DCompile, loaded from d3dcompiler_47.dll).
if(NOT WIN32)
    message(FATAL_ERROR "The Direct3D backends (ESIA_BACKEND_D3D9/10/11/12) build for Windows only: use windows-clang-cl, "
                        "windows-cross, windows-mingw-cross or the MSVC presets")
endif()

add_library(esia_rhi_d3d_common STATIC
    d3d_util.cpp
    d3d_shader.cpp)
# the private headers (d3d_util.hpp, d3d_shader.hpp) are for the backends next door
target_include_directories(esia_rhi_d3d_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(esia_rhi_d3d_common PUBLIC esia_rhi esia_shaders)
esia_target_defaults(esia_rhi_d3d_common)

if(BUILD_TESTING)
    # SM4 / SM5: every program compiles with the machine's d3dcompiler_47 (the D3D9 backend tests SM3)
    esia_add_test(esia_rhi_d3d_shader_tests tests/test_d3d_shaders.cpp)
    target_link_libraries(esia_rhi_d3d_shader_tests PRIVATE esia_rhi_d3d_common)
endif()
