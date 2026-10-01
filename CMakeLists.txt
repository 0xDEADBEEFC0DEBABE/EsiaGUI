cmake_minimum_required(VERSION 3.24)
project(Esia VERSION 0.1.0 LANGUAGES CXX)

# ------------------------------------------------------------------------------
# Esia - a liquid-glass (iOS 26 style) UI toolkit for games and tools: its own UI core, renderer and render hardware
# interface with backends for Direct3D 9 / 10 / 11 / 12, OpenGL / OpenGL ES, Vulkan and Metal (docs/REWRITE.md).
#
#   cmake --preset windows-msvc                 (or windows-clang-cl, linux-clang, macos-clang, ios: CMakePresets.json)
#   cmake --build --preset windows-msvc-release
#
# Every backend of the platform is built by default (src/esia/CMakeLists.txt: ESIA_BACKEND_<NAME>).
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
option(ESIA_BUILD_EXAMPLES "Build the examples (examples/)" ${PROJECT_IS_TOP_LEVEL})
if(ESIA_BUILD_EXAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
    add_subdirectory(examples/glass_window)
    add_subdirectory(examples/showcase)
    add_subdirectory(examples/workbench)
endif()
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
