# Esia - the portable core, renderer and RHI backends (docs/REWRITE.md).
#
#   esia_core      UI core: ids, input, windows, items, layout, draw lists, textures      (no platform headers)
#   esia_rhi       render hardware interface + the recording null backend
#   esia_shaders   precompiled shader library (src/esia/shaders/generated, see tools/shaders)
#   esia_render    Painter, frame planner and renderer on the RHI
#   esia_text      text interface; FreeType + HarfBuzz implementation when ESIA_TEXT_FREETYPE is ON
#   esia_rhi_<n>   one target per backend, from src/esia/rhi/<n>/ (ESIA_BACKEND_<N>)
cmake_minimum_required(VERSION 3.24)

option(ESIA_WERROR "Treat compiler warnings as errors in Esia targets" OFF)
option(ESIA_TEXT_FREETYPE "Build the FreeType + HarfBuzz text backend" ON)
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

# ------------------------------------------------------------------ core
add_library(esia_core STATIC
    core/context.cpp
    core/draw_list.cpp
    core/input.cpp
    core/texture.cpp)
target_include_directories(esia_core PUBLIC ${ESIA_ROOT}/include)
esia_target_defaults(esia_core)
find_package(Threads REQUIRED)
target_link_libraries(esia_core PUBLIC Threads::Threads)
add_library(esia::core ALIAS esia_core)
