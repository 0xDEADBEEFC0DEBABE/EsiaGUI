# Esia tests (CTest): unit tests of the core, renderer and RHI, and the backend conformance suite.
add_library(esia_test_main STATIC esia_test.cpp)
target_include_directories(esia_test_main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
esia_target_defaults(esia_test_main)

function(esia_add_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE esia_test_main)
    esia_target_defaults(${name})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

esia_add_test(esia_core_tests
    core/test_hash.cpp
    core/test_draw_list.cpp
    core/test_texture.cpp
    core/test_input.cpp
    core/test_context.cpp)
target_link_libraries(esia_core_tests PRIVATE esia::core)

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
