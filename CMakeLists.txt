cmake_minimum_required(VERSION 3.24)
project(Esia VERSION 0.1.0 LANGUAGES CXX)

# ------------------------------------------------------------------------------
# Esia - a liquid-glass (iOS 26 style) UI toolkit for games and tools: its own UI core, renderer and render hardware
# interface with backends for Direct3D 9 / 10 / 11 / 12, OpenGL / OpenGL ES, Vulkan and Metal (docs/REWRITE.md).
#
#   cmake --preset windows-clang-cl -DESIA_BACKEND_D3D11=ON     (see CMakePresets.json and docs/backends/README.md)
#   cmake --build --preset windows-clang-cl
#   ctest --preset windows-clang-cl
#
# It replaces WGT UI (the Dear ImGui based wgt.dll), whose last version is tag wgt-1.1-final.
# ------------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(CTest)   # before src/esia: backends register their own tests (esia_add_test)
add_subdirectory(src/esia)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
