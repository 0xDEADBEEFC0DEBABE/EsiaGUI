cmake_minimum_required(VERSION 3.24)
project(WafflingGameToolkit VERSION 1.1.0 LANGUAGES CXX)

# ------------------------------------------------------------------------------
# WGT UI — Waffling Game Toolkit UI framework
#   wgt.dll             : the toolkit (Dear ImGui core + WGT core + DX11/DX12 renderers)
#   wgt_demo.exe        : showcase app (DX11 / DX12 switchable)
#   wgt_minimal_d3d11   : smallest possible host (integration reference)
#   wgt_plugin_*        : example plugin DLL
#
# SDK:  cmake --install build --config Release --prefix <dir>   ->  find_package(wgt CONFIG) / wgt::wgt
# Zip:  cpack --config build/CPackConfig.cmake -C Release         ->  WGT-<version>-sdk-win64.zip
#       cpack --config build/CPackSourceConfig.cmake              ->  WGT-<version>-src.zip
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Esia (the ImGui-free rewrite, docs/REWRITE.md) lives next to the current toolkit:
#   WGT_BUILD_LEGACY : wgt.dll and its examples, exactly as before (Windows + MSVC + fxc)
#   ESIA_BUILD_CORE  : the portable Esia libraries, tests and backends (src/esia, tests)
# With MSVC on Windows the defaults build only wgt.dll, so that build is unchanged. Everywhere else (clang,
# clang-cl, Linux, macOS) the defaults build only Esia.
# ------------------------------------------------------------------------------
if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(_wgt_legacy_default ON)
else()
    set(_wgt_legacy_default OFF)
endif()
option(WGT_BUILD_LEGACY "Build the current wgt.dll (Dear ImGui based; Windows, needs fxc)" ${_wgt_legacy_default})
if(_wgt_legacy_default)
    set(_esia_default OFF)
else()
    set(_esia_default ON)
endif()
option(ESIA_BUILD_CORE "Build the Esia core, renderer, RHI backends and tests" ${_esia_default})

if(WGT_BUILD_LEGACY AND NOT WIN32)
    message(FATAL_ERROR "WGT UI targets Windows (DirectX 11 / 12). Configure with -DWGT_BUILD_LEGACY=OFF to build Esia only.")
endif()
if(NOT WGT_BUILD_LEGACY AND NOT ESIA_BUILD_CORE)
    message(FATAL_ERROR "Nothing to build: enable WGT_BUILD_LEGACY and / or ESIA_BUILD_CORE.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(WGT_BUILD_EXAMPLES "Build the demo application and example plugin" ON)

if(ESIA_BUILD_CORE)
    include(CTest)   # before src/esia: backends register their own tests (esia_add_test)
    add_subdirectory(src/esia)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()

if(WGT_BUILD_LEGACY)

include(cmake/WgtShaders.cmake)

set(WGT_ROOT      ${CMAKE_CURRENT_SOURCE_DIR})
set(IMGUI_DIR     ${WGT_ROOT}/third_party/imgui)
set(WGT_SHADER_DIR ${WGT_ROOT}/src/shaders)
set(WGT_GEN_DIR   ${CMAKE_CURRENT_BINARY_DIR}/generated)

# ---------------------------------------------------------------- shaders ----
set(WGT_SHADER_DEPS
    ${WGT_SHADER_DIR}/wgt_common.hlsli
    ${WGT_SHADER_DIR}/wgt_fx.hlsl
    ${WGT_SHADER_DIR}/wgt_imgui.hlsl
    ${WGT_SHADER_DIR}/wgt_post.hlsl)

set(WGT_SHADER_HEADERS)
wgt_compile_shader(WGT_SHADER_HEADERS ${WGT_SHADER_DIR}/wgt_imgui.hlsl ImGuiVS          vs_5_0 kShaderImGuiVS)
wgt_compile_shader(WGT_SHADER_HEADERS ${WGT_SHADER_DIR}/wgt_imgui.hlsl ImGuiPS          ps_5_0 kShaderImGuiPS)
wgt_compile_shader(WGT_SHADER_HEADERS ${WGT_SHADER_DIR}/wgt_imgui.hlsl ImGuiTextPS      ps_5_0 kShaderImGuiTextPS)
wgt_compile_shader(WGT_SHADER_HEADERS ${WGT_SHADER_DIR}/wgt_imgui.hlsl ImGuiTextLcdPS   ps_5_0 kShaderImGuiTextLcdPS)
wgt_compile_shader(WGT_SHADER_HEADERS ${WGT_SHADER_DIR}/wgt_imgui.hlsl ImGuiTextLcdGrayPS ps_5_0 kShaderImGuiTextLcdGrayPS)
wgt_compile_shader(WGT_SHADER_HEADERS ${WGT_SHADER_DIR}/wgt_fx.hlsl    FxVS             vs_5_0 kShaderFxVS)
wgt_compile_shader(WGT_SHADER_HEADERS ${WGT_SHADER_DIR}/wgt_fx.hlsl    FxPS             ps_5_0 kShaderFxPS)
wgt_compile_shader(WGT_SHADER_HEADERS ${WGT_SHADER_DIR}/wgt_post.hlsl  FullscreenVS     vs_5_0 kShaderFullscreenVS)
wgt_compile_shader(WGT_SHADER_HEADERS ${WGT_SHADER_DIR}/wgt_post.hlsl  DownsamplePS     ps_5_0 kShaderDownsamplePS)
wgt_compile_shader(WGT_SHADER_HEADERS ${WGT_SHADER_DIR}/wgt_post.hlsl  LayerCompositePS ps_5_0 kShaderLayerCompositePS)

# HLSL sources are also embedded so user effects can be compiled at runtime.
wgt_embed_files(WGT_EMBED_HEADER ${WGT_GEN_DIR}/wgt_embedded_hlsl.h
    kHlslCommon ${WGT_SHADER_DIR}/wgt_common.hlsli
    kHlslFx     ${WGT_SHADER_DIR}/wgt_fx.hlsl)

add_custom_target(wgt_shaders DEPENDS ${WGT_SHADER_HEADERS} ${WGT_EMBED_HEADER})

# ------------------------------------------------------------------ wgt.dll ----
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp)

file(GLOB WGT_PUBLIC_HEADERS CONFIGURE_DEPENDS ${WGT_ROOT}/include/wgt/*.hpp ${WGT_ROOT}/include/wgt/*.h)
file(GLOB_RECURSE WGT_SOURCES CONFIGURE_DEPENDS
    ${WGT_ROOT}/src/*.cpp
    ${WGT_ROOT}/src/*.hpp)

add_library(wgt SHARED ${WGT_SOURCES} ${WGT_PUBLIC_HEADERS} ${IMGUI_SOURCES} ${WGT_SHADER_DEPS})
add_dependencies(wgt wgt_shaders)
set_source_files_properties(${WGT_SHADER_DEPS} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group("imgui" FILES ${IMGUI_SOURCES})
source_group("shaders" FILES ${WGT_SHADER_DEPS})

add_library(wgt::wgt ALIAS wgt)

target_include_directories(wgt
    PUBLIC  $<BUILD_INTERFACE:${WGT_ROOT}/include>
            $<BUILD_INTERFACE:${IMGUI_DIR}>
            $<INSTALL_INTERFACE:include>
            $<INSTALL_INTERFACE:include/imgui>
    PRIVATE ${WGT_ROOT}/src
            ${IMGUI_DIR}/backends
            ${WGT_GEN_DIR})

target_compile_definitions(wgt
    PUBLIC  "IMGUI_USER_CONFIG=\"wgt/imconfig_wgt.h\""
    PRIVATE WGT_BUILD_DLL
            UNICODE _UNICODE
            WIN32_LEAN_AND_MEAN NOMINMAX
            _CRT_SECURE_NO_WARNINGS)

target_compile_features(wgt PUBLIC cxx_std_20)
target_compile_options(wgt PRIVATE /W3 /permissive- /utf-8 /MP /Zc:__cplusplus)
target_compile_options(wgt INTERFACE /utf-8)
# Release keeps a PDB so crash stacks resolve inside wgt.dll (it is not needed at runtime).
target_compile_options(wgt PRIVATE $<$<CONFIG:Release>:/Zi>)
target_link_options(wgt PRIVATE $<$<CONFIG:Release>:/DEBUG> $<$<CONFIG:Release>:/OPT:REF> $<$<CONFIG:Release>:/OPT:ICF>)
target_link_libraries(wgt PRIVATE d3d11 d3d12 dxgi dxguid dwmapi)
set_target_properties(wgt PROPERTIES OUTPUT_NAME "wgt")

# ----------------------------------------------------------------- examples ----
if(WGT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ------------------------------------------------------------ install (SDK) ----
# <prefix>/bin            wgt.dll, wgt.pdb (+ wgt_demo.exe, plugins/ when examples are built)
# <prefix>/lib            wgt.lib, cmake/wgt (package config: find_package(wgt CONFIG) -> wgt::wgt)
# <prefix>/include        wgt/*.hpp, imgui/ (WGT's Dear ImGui headers)
# <prefix>/docs, examples, licenses
include(CMakePackageConfigHelpers)

install(TARGETS wgt EXPORT wgtTargets
    RUNTIME DESTINATION bin COMPONENT sdk
    LIBRARY DESTINATION lib COMPONENT sdk
    ARCHIVE DESTINATION lib COMPONENT sdk)
install(FILES $<TARGET_PDB_FILE:wgt> DESTINATION bin COMPONENT sdk OPTIONAL)
install(DIRECTORY include/wgt DESTINATION include COMPONENT sdk)
install(FILES ${IMGUI_DIR}/imgui.h ${IMGUI_DIR}/imconfig.h ${IMGUI_DIR}/imgui_internal.h
    DESTINATION include/imgui COMPONENT sdk)
install(EXPORT wgtTargets NAMESPACE wgt:: DESTINATION lib/cmake/wgt COMPONENT sdk)

configure_package_config_file(cmake/wgtConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/wgtConfig.cmake
    INSTALL_DESTINATION lib/cmake/wgt)
# the ABI may change between minor versions; the version file also rejects 32-bit consumers
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/wgtConfigVersion.cmake
    VERSION ${PROJECT_VERSION} COMPATIBILITY SameMinorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/wgtConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/wgtConfigVersion.cmake
    DESTINATION lib/cmake/wgt COMPONENT sdk)

install(FILES README.md cmake/wgt.props DESTINATION . COMPONENT sdk)   # wgt.props: Visual Studio projects without CMake
install(DIRECTORY docs/ DESTINATION docs COMPONENT sdk)
install(DIRECTORY examples/ DESTINATION examples COMPONENT sdk)   # sources; they build against the SDK too
install(FILES ${IMGUI_DIR}/LICENSE.txt DESTINATION licenses RENAME dear-imgui-LICENSE.txt COMPONENT sdk)

# ------------------------------------------------------------------ packages ----
set(CPACK_PACKAGE_NAME "WGT")
set(CPACK_PACKAGE_VENDOR "Waffling Game Toolkit")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "WGT UI - liquid-glass UI toolkit for Direct3D 11 / 12")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_FILE_NAME "WGT-${PROJECT_VERSION}-sdk-win64")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
set(CPACK_ARCHIVE_COMPONENT_INSTALL OFF)   # one zip: SDK + demo
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/packages)
set(CPACK_SOURCE_GENERATOR ZIP)
set(CPACK_SOURCE_PACKAGE_FILE_NAME "WGT-${PROJECT_VERSION}-src")
set(CPACK_SOURCE_IGNORE_FILES
    "^${PROJECT_SOURCE_DIR}/build/"
    "^${PROJECT_SOURCE_DIR}/out/"
    "/\\.vs/"
    "/\\.git/"
    "\\.user$"
    "\\.log$")
include(CPack)
endif()   # WGT_BUILD_LEGACY
