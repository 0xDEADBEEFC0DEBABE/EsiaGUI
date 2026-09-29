# Esia tests (CTest): unit tests of the core, renderer and RHI, and the backend conformance suite.
# esia_test_main and esia_add_test() come from src/esia/CMakeLists.txt (backends use them too).

esia_add_test(esia_core_tests
    core/test_hash.cpp
    core/test_draw_list.cpp
    core/test_texture.cpp
    core/test_input.cpp
    core/test_context.cpp
    core/test_interaction.cpp
    core/test_windows.cpp
    core/test_layout.cpp
    core/test_utf8.cpp
    core/test_public_headers.cpp)
target_link_libraries(esia_core_tests PRIVATE esia::core)

esia_add_test(esia_text_tests
    text/test_glyph_raster.cpp
    text/test_glyph_atlas.cpp
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

# test kit: PNG files and tolerant image comparison (the conformance suite's goldens)
add_library(esia_testkit STATIC support/image.cpp)
target_include_directories(esia_testkit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/support)
esia_target_defaults(esia_testkit)
esia_add_test(esia_testkit_tests support/test_image.cpp)
target_link_libraries(esia_testkit_tests PRIVATE esia_testkit)

# the FreeType + HarfBuzz text system with the fonts in tests/fonts (DroidSans, Karla, a Noto Sans SC subset) and the
# platform's fallback chain; its golden images (tests/text/golden) are CPU composites of the glyph quads, so they need
# no GPU
if(TARGET esia_text_ft)
    esia_add_test(esia_text_ft_tests text/test_ft_text.cpp text/test_ft_cjk.cpp)
    target_link_libraries(esia_text_ft_tests PRIVATE esia_text_ft esia_testkit)
    target_compile_definitions(esia_text_ft_tests PRIVATE
        ESIA_TEST_FONT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fonts"
        ESIA_TEXT_GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/text/golden"
        ESIA_TEXT_OUT_DIR="${CMAKE_BINARY_DIR}/text")
endif()

# conformance suite: every scene with every backend built into this binary (docs/backends/README.md). The scenes
# are a library of their own: backend tests that render them with their API's checks link it too.
add_library(esia_conformance_scenes STATIC conformance/scenes.cpp)
target_include_directories(esia_conformance_scenes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/conformance)
target_link_libraries(esia_conformance_scenes PUBLIC esia::render esia_testkit)
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
