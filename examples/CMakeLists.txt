# WGT UI examples
#
# Built as part of the WGT tree, or on their own against an installed SDK:
#   cmake -S examples -B build-examples -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=<sdk root>
#   cmake --build build-examples --config Release
cmake_minimum_required(VERSION 3.24)

if(NOT TARGET wgt::wgt)
    project(wgt_examples LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    find_package(wgt 1.1 CONFIG REQUIRED)
    set(WGT_EXAMPLES_STANDALONE ON)
endif()

# Standalone builds copy wgt.dll next to the executables (in-tree builds already share one output folder).
function(wgt_copy_runtime target)
    if(WGT_EXAMPLES_STANDALONE)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:wgt::wgt> $<TARGET_FILE_DIR:${target}>)
    endif()
endfunction()

# --------------------------------------------------------------- minimal_d3d11 ----
# The smallest complete host: window, D3D11 swap chain, WGT context, one frame loop. Start here.
add_executable(wgt_minimal_d3d11 WIN32 minimal_d3d11/main.cpp)
target_link_libraries(wgt_minimal_d3d11 PRIVATE wgt::wgt d3d11 dxgi)
target_compile_definitions(wgt_minimal_d3d11 PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
target_compile_options(wgt_minimal_d3d11 PRIVATE /W3 /permissive-)
wgt_copy_runtime(wgt_minimal_d3d11)

# ------------------------------------------------------------------------ demo ----
add_executable(wgt_demo WIN32
    demo/main.cpp
    demo/host.hpp
    demo/host_d3d11.cpp
    demo/host_d3d12.cpp
    demo/image_io.hpp
    demo/image_io.cpp
    demo/showcase.hpp
    demo/showcase.cpp)
target_link_libraries(wgt_demo PRIVATE wgt::wgt d3d11 d3d12 dxgi windowscodecs ole32)
target_compile_definitions(wgt_demo PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
target_compile_options(wgt_demo PRIVATE /W3 /permissive-)
set_target_properties(wgt_demo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:wgt_demo>")
wgt_copy_runtime(wgt_demo)

# Example plugin: dropped into <exe dir>/plugins and loaded by the demo at startup.
add_library(wgt_plugin_hello SHARED plugin_hello/plugin_hello.cpp)
target_link_libraries(wgt_plugin_hello PRIVATE wgt::wgt)
target_compile_options(wgt_plugin_hello PRIVATE /W3 /permissive-)
set_target_properties(wgt_plugin_hello PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/plugins"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/plugins")
add_dependencies(wgt_demo wgt_plugin_hello)

# In-tree: the prebuilt demo and plugin go into the SDK package next to wgt.dll.
if(NOT WGT_EXAMPLES_STANDALONE)
    install(TARGETS wgt_demo wgt_minimal_d3d11 RUNTIME DESTINATION bin COMPONENT demo)
    install(FILES $<TARGET_FILE:wgt_plugin_hello> DESTINATION bin/plugins COMPONENT demo)
endif()
