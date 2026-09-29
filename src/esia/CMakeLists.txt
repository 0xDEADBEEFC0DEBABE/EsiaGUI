# Esia - the portable core, renderer and RHI backends (docs/REWRITE.md).
#
#   esia_core      UI core: ids, input, windows, items, layout, draw lists, textures      (no platform headers)
#   esia_rhi       render hardware interface + the recording null backend
#   esia_shaders   precompiled shader library (src/esia/shaders/generated, see tools/shaders)
#   esia_render    Painter, frame planner and renderer on the RHI
#   esia_text      text interface, the analytic glyph rasterizer and the glyph atlas
#   esia_text_ft   FreeType + HarfBuzz text system (ESIA_TEXT_FREETYPE, when both libraries are found)
#   esia_rhi_<n>   one target per backend, from src/esia/rhi/<n>/ (ESIA_BACKEND_<N>)
cmake_minimum_required(VERSION 3.24)

option(ESIA_WERROR "Treat compiler warnings as errors in Esia targets" OFF)
option(ESIA_TEXT_FREETYPE "Build the FreeType + HarfBuzz text system (skipped when they are not found)" ON)
option(ESIA_SHADERS_REGENERATE "Recompile the shader library at build time (needs glslangValidator, spirv-cross)" OFF)

set(ESIA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
cmake_path(NORMAL_PATH ESIA_ROOT)
set(ESIA_ROOT ${ESIA_ROOT} CACHE INTERNAL "")

# Common settings of every Esia target (backends call it too).
function(esia_target_defaults target)
    target_compile_features(${target} PUBLIC cxx_std_20)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    if(MSVC)   # cl and clang-cl
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8 /Zc:__cplusplus)
        target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            # clang-cl's /W4 is -Wall -Wextra: the graphics APIs' partial initializers (VkFooInfo i{sType},
            # D3D12_HEAP_PROPERTIES hp = {type}) would warn, as on the GNU side below
            target_compile_options(${target} PRIVATE -Wno-missing-field-initializers)
        endif()
        if(ESIA_WERROR)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wno-missing-field-initializers)
        if(ESIA_WERROR)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

# ------------------------------------------------------------------ tests
# The test runner (tests/esia_test.hpp) and esia_add_test(<name> <sources>...): defined here, before the backends,
# so a backend's directory can add unit tests of its own next to the conformance suite (tests/).
if(BUILD_TESTING)
    add_library(esia_test_main STATIC ${ESIA_ROOT}/tests/esia_test.cpp)
    target_include_directories(esia_test_main PUBLIC ${ESIA_ROOT}/tests)
    esia_target_defaults(esia_test_main)
    # the static MinGW libstdc++ (posix threads model) needs winpthreads in every executable
    find_package(Threads REQUIRED)
    target_link_libraries(esia_test_main PUBLIC Threads::Threads)
endif()

function(esia_add_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE esia_test_main)
    esia_target_defaults(${name})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

# ------------------------------------------------------------------ core
add_library(esia_core STATIC
    core/context.cpp
    core/items.cpp
    core/layout.cpp
    core/state.cpp
    core/draw_list.cpp
    core/input.cpp
    core/texture.cpp)
target_include_directories(esia_core PUBLIC ${ESIA_ROOT}/include)
esia_target_defaults(esia_core)
find_package(Threads REQUIRED)
target_link_libraries(esia_core PUBLIC Threads::Threads)
add_library(esia::core ALIAS esia_core)

# ------------------------------------------------------------------ renderer
add_library(esia_render STATIC
    render/painter.cpp
    render/frame_plan.cpp
    render/renderer.cpp)
target_link_libraries(esia_render PUBLIC esia_core esia_rhi)
esia_target_defaults(esia_render)
add_library(esia::render ALIAS esia_render)

# ------------------------------------------------------------------ text
add_library(esia_text STATIC
    text/glyph_raster.cpp
    text/glyph_atlas.cpp)
target_link_libraries(esia_text PUBLIC esia_core)
esia_target_defaults(esia_text)
add_library(esia::text ALIAS esia_text)

# FreeType + HarfBuzz: system packages on Linux / macOS (libfreetype-dev libharfbuzz-dev, brew freetype harfbuzz),
# vcpkg or the like on Windows. Cross builds usually have neither: the text system is then left out, not an error.
if(ESIA_TEXT_FREETYPE)
    find_package(Freetype QUIET)
    find_package(harfbuzz CONFIG QUIET)
    if(NOT TARGET harfbuzz::harfbuzz)
        find_path(ESIA_HARFBUZZ_INCLUDE_DIR hb.h PATH_SUFFIXES harfbuzz)
        find_library(ESIA_HARFBUZZ_LIBRARY harfbuzz)
        if(ESIA_HARFBUZZ_INCLUDE_DIR AND ESIA_HARFBUZZ_LIBRARY)
            add_library(harfbuzz::harfbuzz UNKNOWN IMPORTED)
            set_target_properties(harfbuzz::harfbuzz PROPERTIES
                IMPORTED_LOCATION ${ESIA_HARFBUZZ_LIBRARY}
                INTERFACE_INCLUDE_DIRECTORIES ${ESIA_HARFBUZZ_INCLUDE_DIR})
        endif()
    endif()
    if(TARGET Freetype::Freetype AND TARGET harfbuzz::harfbuzz)
        add_library(esia_text_ft STATIC text/ft/ft_text_system.cpp)
        target_link_libraries(esia_text_ft PUBLIC esia_text PRIVATE Freetype::Freetype harfbuzz::harfbuzz)
        esia_target_defaults(esia_text_ft)
        add_library(esia::text_ft ALIAS esia_text_ft)
        message(STATUS "Esia text: FreeType ${FREETYPE_VERSION_STRING} + HarfBuzz")
    else()
        message(STATUS "Esia text: FreeType or HarfBuzz not found - esia_text_ft is not built")
    endif()
endif()

# ------------------------------------------------------------------ RHI + backends
add_library(esia_rhi STATIC
    rhi/rhi.cpp
    rhi/null/null_device.cpp)
target_include_directories(esia_rhi PUBLIC ${ESIA_ROOT}/include)
esia_target_defaults(esia_rhi)
add_library(esia::rhi ALIAS esia_rhi)

# esia_register_backend(<name> <target>): called by every backend's CMakeLists.txt. The target must define
# `void EsiaRegisterBackend_<name>()` (global namespace) which calls esia::rhi::RegisterBackend.
function(esia_register_backend name target)
    set_property(GLOBAL APPEND PROPERTY ESIA_BACKENDS ${name})
    set_property(GLOBAL APPEND PROPERTY ESIA_BACKEND_TARGETS ${target})
endfunction()
esia_register_backend(null esia_rhi)

# One option and one directory per backend. The DirectX branch shares code between its four backends in
# rhi/d3d_common (built when any ESIA_BACKEND_D3D* is ON).
set(ESIA_KNOWN_BACKENDS opengl vulkan metal d3d9 d3d10 d3d11 d3d12)
foreach(_b ${ESIA_KNOWN_BACKENDS})
    string(TOUPPER ${_b} _B)
    option(ESIA_BACKEND_${_B} "Build the ${_b} RHI backend (src/esia/rhi/${_b})" OFF)
endforeach()
if(ESIA_BACKEND_D3D9 OR ESIA_BACKEND_D3D10 OR ESIA_BACKEND_D3D11 OR ESIA_BACKEND_D3D12)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/rhi/d3d_common/CMakeLists.txt)
        add_subdirectory(rhi/d3d_common)
    endif()
endif()
foreach(_b ${ESIA_KNOWN_BACKENDS})
    string(TOUPPER ${_b} _B)
    if(ESIA_BACKEND_${_B})
        if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/rhi/${_b}/CMakeLists.txt)
            message(FATAL_ERROR "ESIA_BACKEND_${_B} is ON but src/esia/rhi/${_b}/CMakeLists.txt does not exist on this branch")
        endif()
        add_subdirectory(rhi/${_b})
    endif()
endforeach()

# esia_backends: every registered backend + RegisterBuiltinBackends() (generated)
get_property(_esia_backends GLOBAL PROPERTY ESIA_BACKENDS)
get_property(_esia_backend_targets GLOBAL PROPERTY ESIA_BACKEND_TARGETS)
set(_decls "")
set(_calls "")
foreach(_b ${_esia_backends})
    string(APPEND _decls "void EsiaRegisterBackend_${_b}();\n")
    string(APPEND _calls "        EsiaRegisterBackend_${_b}();\n")
endforeach()
set(_gen ${CMAKE_CURRENT_BINARY_DIR}/generated/esia_backends.cpp)
file(CONFIGURE OUTPUT ${_gen} CONTENT "// Generated by src/esia/CMakeLists.txt: the backends built into this binary.
#include \"esia/rhi/backend_registry.hpp\"

${_decls}
void esia::rhi::RegisterBuiltinBackends()
{
    static bool done = false;
    if (!done)
    {
        done = true;
${_calls}    }
}
" @ONLY)
add_library(esia_backends STATIC ${_gen})
target_link_libraries(esia_backends PUBLIC esia_rhi ${_esia_backend_targets})
esia_target_defaults(esia_backends)
add_library(esia::backends ALIAS esia_backends)
message(STATUS "Esia backends: ${_esia_backends}")

# ------------------------------------------------------------------ shader library
# src/esia/shaders/generated holds every backend format, produced by tools/shaders/build_shaders.py and checked
# in; here the files become C++ arrays (one translation unit per format) at build time.
set(ESIA_SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(ESIA_SHADER_GEN ${ESIA_SHADER_DIR}/generated)
set(_esia_shader_srcs
    ${ESIA_SHADER_DIR}/esia_common.hlsli
    ${ESIA_SHADER_DIR}/esia_ui.hlsl
    ${ESIA_SHADER_DIR}/esia_fx.hlsl
    ${ESIA_SHADER_DIR}/esia_post.hlsl)

if(ESIA_SHADERS_REGENERATE)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_custom_command(
        OUTPUT ${ESIA_SHADER_GEN}/esia_shader_table.cpp
        COMMAND Python3::Interpreter ${ESIA_ROOT}/tools/shaders/build_shaders.py
        DEPENDS ${_esia_shader_srcs} ${ESIA_ROOT}/tools/shaders/build_shaders.py
        COMMENT "Compiling Esia shaders (tools/shaders/build_shaders.py)"
        VERBATIM)
endif()

set(_esia_embedded)
function(_esia_embed out_var name)   # ARGN: sym=file pairs
    set(_out ${CMAKE_CURRENT_BINARY_DIR}/generated/esia_embed_${name}.cpp)
    set(_deps)
    foreach(_p ${ARGN})
        string(REGEX REPLACE "^[^=]*=" "" _f "${_p}")
        list(APPEND _deps ${_f})
    endforeach()
    string(REPLACE ";" "|" _pairs "${ARGN}")
    add_custom_command(
        OUTPUT ${_out}
        COMMAND ${CMAKE_COMMAND} -DOUT=${_out} -DNAMESPACE=esia::shaders::blobs "-DPAIRS=${_pairs}" -P ${ESIA_ROOT}/cmake/EsiaEmbed.cmake
        DEPENDS ${_deps} ${ESIA_ROOT}/cmake/EsiaEmbed.cmake
        COMMENT "Embedding Esia shaders: ${name}"
        VERBATIM)
    set(${out_var} ${${out_var}} ${_out} PARENT_SCOPE)
endfunction()

set(_pairs)
foreach(_f ${_esia_shader_srcs})
    get_filename_component(_n ${_f} NAME)
    string(REGEX REPLACE "[^A-Za-z0-9_]" "_" _sym "source_${_n}")
    list(APPEND _pairs "${_sym}=${_f}")
endforeach()
_esia_embed(_esia_embedded sources ${_pairs})
foreach(_fmt spirv glsl330 essl300 msl dxbc_sm5 dxbc_sm4 dxil dxbc_sm3)
    file(GLOB _files CONFIGURE_DEPENDS ${ESIA_SHADER_GEN}/${_fmt}/*)
    list(SORT _files)
    set(_pairs)
    foreach(_f ${_files})
        get_filename_component(_n ${_f} NAME)
        string(REGEX REPLACE "[^A-Za-z0-9_]" "_" _sym "${_fmt}_${_n}")
        list(APPEND _pairs "${_sym}=${_f}")
    endforeach()
    if(_pairs)
        _esia_embed(_esia_embedded ${_fmt} ${_pairs})
    endif()
endforeach()

add_library(esia_shaders STATIC
    shaders/shader_library.cpp
    ${ESIA_SHADER_GEN}/esia_shader_table.cpp
    ${_esia_embedded})
target_link_libraries(esia_shaders PUBLIC esia_rhi)
esia_target_defaults(esia_shaders)
add_library(esia::shaders ALIAS esia_shaders)
