# Esia tests (CTest): unit tests of the core, renderer and RHI, and the backend conformance suite.
# esia_test_main and esia_add_test() come from src/esia/CMakeLists.txt (backends use them too).

esia_add_test(esia_core_tests
    core/test_hash.cpp
    core/test_cpu.cpp
    core/test_draw_list.cpp
    core/test_texture.cpp
    core/test_input.cpp
    core/test_context.cpp
    core/test_interaction.cpp
    core/test_windows.cpp
    core/test_layout.cpp
    core/test_debug_checks.cpp
    core/test_utf8.cpp
    core/test_public_headers.cpp)
target_link_libraries(esia_core_tests PRIVATE esia::core)

esia_add_test(esia_text_tests
    text/test_glyph_raster.cpp
    text/test_glyph_atlas.cpp
    text/test_distance_field.cpp
    text/test_system_fonts.cpp)
target_link_libraries(esia_text_tests PRIVATE esia::text)
target_compile_definitions(esia_text_tests PRIVATE
    ESIA_TEST_FONT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fonts"
    ESIA_TEXT_OUT_DIR="${CMAKE_BINARY_DIR}/text")

esia_add_test(esia_render_tests
    render/test_frame_plan.cpp
    render/test_painter.cpp
    render/test_renderer.cpp)
target_link_libraries(esia_render_tests PRIVATE esia::render)

# SVG: the parser, exact coverage (against dense point sampling), strokes, paints and the atlas
esia_add_test(esia_svg_tests svg/test_svg_parse.cpp svg/test_svg_raster.cpp)
target_link_libraries(esia_svg_tests PRIVATE esia::svg)
target_include_directories(esia_svg_tests PRIVATE ${ESIA_ROOT}/src/esia/svg)   # the internals: path data, the sweep

# the widget layer's tests, where the tree has the layer (tests/ui/CMakeLists.txt)
if(TARGET esia_ui AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ui/CMakeLists.txt)
    add_subdirectory(ui)
endif()

# test kit: PNG files and tolerant image comparison (the conformance suite's goldens)
add_library(esia_testkit STATIC support/image.cpp)
target_include_directories(esia_testkit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/support)
esia_target_defaults(esia_testkit)
esia_add_test(esia_testkit_tests support/test_image.cpp)
target_link_libraries(esia_testkit_tests PRIVATE esia_testkit)

# the FreeType + HarfBuzz text system with the fonts in tests/fonts (DroidSans, Karla, a Noto Sans SC subset) and the
# platform's fallback chain; its golden images (tests/text/golden) are CPU composites of the glyph quads, so they need
# no GPU. Images of FreeType's own pixels (its hinting, its rasterizer) are compared only with the bundled FreeType they
# were made with; the tests run without ESIA_TEXT_RENDERING (a developer's own choice would change every image).
if(TARGET esia_text_ft)
    esia_add_test(esia_text_ft_tests text/test_ft_text.cpp text/test_ft_cjk.cpp text/test_ft_rendering.cpp text/test_platform_face.cpp
                  text/test_color_glyphs.cpp)
    set_tests_properties(esia_text_ft_tests PROPERTIES ENVIRONMENT "ESIA_TEXT_RENDERING=")
    get_property(_esia_text_deps GLOBAL PROPERTY ESIA_TEXT_DEPS_FOUND)
    if(_esia_text_deps STREQUAL "bundled")
        target_compile_definitions(esia_text_ft_tests PRIVATE ESIA_TEST_BUNDLED_FREETYPE)
    endif()
    target_link_libraries(esia_text_ft_tests PRIVATE esia_text_ft esia_testkit esia::harfbuzz)
    target_include_directories(esia_text_ft_tests PRIVATE ${ESIA_ROOT}/src/esia/text/ft)   # platform_face.hpp, png.hpp
    if(APPLE)   # Core Text's own widths (coretext_util.cpp: its headers' Rect clashes with esia's in the tests)
        target_sources(esia_text_ft_tests PRIVATE text/coretext_util.cpp)
        target_link_libraries(esia_text_ft_tests PRIVATE "-framework CoreText" "-framework CoreFoundation")
    endif()
    target_compile_definitions(esia_text_ft_tests PRIVATE
        ESIA_TEST_FONT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fonts"
        ESIA_TEXT_GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/text/golden"
        ESIA_TEXT_OUT_DIR="${CMAKE_BINARY_DIR}/text")
endif()

# conformance suite: every scene with every backend built into this binary (docs/backends/README.md). The scenes
# are a library of their own: backend tests that render them with their API's checks link it too.
add_library(esia_conformance_scenes STATIC conformance/scenes.cpp)
target_include_directories(esia_conformance_scenes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/conformance)
target_link_libraries(esia_conformance_scenes PUBLIC esia::render esia_testkit PRIVATE esia::text)   # text: the glyph fields
esia_target_defaults(esia_conformance_scenes)
add_executable(esia_conformance conformance/conformance.cpp)
target_link_libraries(esia_conformance PRIVATE esia_conformance_scenes esia::backends)
esia_target_defaults(esia_conformance)
set(ESIA_GOLDEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/conformance/golden)
get_property(_esia_backends GLOBAL PROPERTY ESIA_BACKENDS)
foreach(_b ${_esia_backends})
    # --strict: every golden exists (the images from OpenGL on llvmpipe, the null logs from core), so a missing one
    # fails. Exit code 77 = every scene skipped (no device for the backend here): CTest shows it as skipped.
    add_test(NAME esia_conformance_${_b}
             COMMAND esia_conformance --backend ${_b} --golden ${ESIA_GOLDEN_DIR} --out ${CMAKE_BINARY_DIR}/conformance --strict)
    set_tests_properties(esia_conformance_${_b} PROPERTIES SKIP_RETURN_CODE 77)
    if(NOT _b STREQUAL "null")
        # cross-frame: two poison frames on the same device first (the null backend's goldens are single frames)
        add_test(NAME esia_conformance_${_b}_frames
                 COMMAND esia_conformance --backend ${_b} --golden ${ESIA_GOLDEN_DIR} --out ${CMAKE_BINARY_DIR}/conformance-frames --frames 3 --strict)
        set_tests_properties(esia_conformance_${_b}_frames PROPERTIES SKIP_RETURN_CODE 77)
    endif()
endforeach()

# HDR output (render::HdrOutput): the encodings' numbers and the decode - a test per backend (ESIA_HDR_BACKEND), as
# the conformance suite has, so a runner can leave out a driver it cannot run by its name (docs/CI.md)
add_executable(esia_hdr_tests render/test_hdr.cpp)
target_link_libraries(esia_hdr_tests PRIVATE esia_test_main esia::render esia::backends)
esia_target_defaults(esia_hdr_tests)
foreach(_b ${_esia_backends})
    if(NOT _b STREQUAL "null")
        add_test(NAME esia_hdr_${_b} COMMAND esia_hdr_tests)
        set_tests_properties(esia_hdr_${_b} PROPERTIES ENVIRONMENT "ESIA_HDR_BACKEND=${_b}"
                             SKIP_REGULAR_EXPRESSION "esia_hdr: no device ran")
    endif()
endforeach()

# the renderer on fixed workloads (tests/bench/esia_bench.cpp): build, submit and GPU time per frame, median and 99th
# percentile. CTest only runs it a few frames on the null device, so that it keeps working.
if(TARGET esia_text_ft)
    add_executable(esia_bench bench/esia_bench.cpp)
    target_link_libraries(esia_bench PRIVATE esia::render esia_text_ft esia::backends)
    target_compile_definitions(esia_bench PRIVATE ESIA_BENCH_FONT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fonts")
    esia_target_defaults(esia_bench)
    add_test(NAME esia_bench_smoke COMMAND esia_bench --backend null --frames 3 --warmup 1 --size 640x400)
endif()

esia_add_test(esia_shader_tests shaders/test_shader_library.cpp)
target_link_libraries(esia_shader_tests PRIVATE esia_shaders)

# the generated GLSL / ESSL compiled and linked by the system's driver (Mesa llvmpipe in CI) through EGL
# (only EGL is linked: the test loads its few GL functions through eglGetProcAddress)
find_path(ESIA_EGL_INCLUDE_DIR EGL/egl.h)
find_library(ESIA_EGL_LIBRARY EGL)
if(ESIA_EGL_INCLUDE_DIR AND ESIA_EGL_LIBRARY)
    esia_add_test(esia_glsl_link_tests shaders/test_glsl_link.cpp)
    target_include_directories(esia_glsl_link_tests PRIVATE ${ESIA_EGL_INCLUDE_DIR})
    target_link_libraries(esia_glsl_link_tests PRIVATE esia_shaders ${ESIA_EGL_LIBRARY})
endif()

# find_package: this build installed into a prefix of its own, and an application of its own (tests/package) built
# against it with the same generator and compilers, then run (a frame on the null device)
if(ESIA_INSTALL AND NOT CMAKE_CROSSCOMPILING)
    # what this build was configured with that finds the dependencies (vcpkg's toolchain and triplet in CI)
    set(_esia_package_forward)
    foreach(_v CMAKE_PREFIX_PATH VCPKG_CHAINLOAD_TOOLCHAIN_FILE VCPKG_TARGET_TRIPLET CMAKE_OSX_DEPLOYMENT_TARGET CMAKE_OSX_ARCHITECTURES)
        if(DEFINED ${_v} AND NOT "${${_v}}" STREQUAL "")
            string(REPLACE ";" "@SEMICOLON@" _value "${${_v}}")   # a list (CMAKE_PREFIX_PATH) stays one argument
            list(APPEND _esia_package_forward "-D${_v}=${_value}")
        endif()
    endforeach()
    string(REPLACE ";" "|" _esia_package_forward "${_esia_package_forward}")
    add_test(NAME esia_package
             COMMAND ${CMAKE_COMMAND} -DBUILD=${CMAKE_BINARY_DIR} -DSOURCE=${ESIA_ROOT}/tests/package
                     -DWORK=${CMAKE_BINARY_DIR}/package-test -DCONFIG=$<CONFIG> "-DGENERATOR=${CMAKE_GENERATOR}"
                     "-DPLATFORM=${CMAKE_GENERATOR_PLATFORM}" "-DTOOLCHAIN=${CMAKE_TOOLCHAIN_FILE}"
                     "-DCXX=${CMAKE_CXX_COMPILER}" "-DMAKE=${CMAKE_MAKE_PROGRAM}" "-DFORWARD=${_esia_package_forward}"
                     -P ${ESIA_ROOT}/tests/package/run_package_test.cmake)
    set_tests_properties(esia_package PROPERTIES TIMEOUT 900)
endif()
