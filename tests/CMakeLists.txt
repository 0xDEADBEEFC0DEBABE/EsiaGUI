# Esia tests (CTest): unit tests of the core, renderer and RHI, and the backend conformance suite.
# esia_test_main and esia_add_test() come from src/esia/CMakeLists.txt (backends use them too).

esia_add_test(esia_core_tests
    core/test_hash.cpp
    core/test_draw_list.cpp
    core/test_texture.cpp
    core/test_input.cpp
    core/test_context.cpp
    core/test_utf8.cpp)
target_link_libraries(esia_core_tests PRIVATE esia::core)

esia_add_test(esia_text_tests
    text/test_glyph_raster.cpp
    text/test_glyph_atlas.cpp)
target_link_libraries(esia_text_tests PRIVATE esia::text)

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

# conformance suite: every scene with every backend built into this binary (docs/backends/README.md)
add_executable(esia_conformance conformance/conformance.cpp conformance/scenes.cpp)
target_include_directories(esia_conformance PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/conformance)
target_link_libraries(esia_conformance PRIVATE esia::render esia::backends esia_testkit)
esia_target_defaults(esia_conformance)
set(ESIA_GOLDEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/conformance/golden)
get_property(_esia_backends GLOBAL PROPERTY ESIA_BACKENDS)
foreach(_b ${_esia_backends})
    # the null backend's command-stream goldens live in core: a missing one is an error there
    set(_strict "")
    if(_b STREQUAL "null")
        set(_strict --strict)
    endif()
    add_test(NAME esia_conformance_${_b}
             COMMAND esia_conformance --backend ${_b} --golden ${ESIA_GOLDEN_DIR} --out ${CMAKE_BINARY_DIR}/conformance ${_strict})
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
