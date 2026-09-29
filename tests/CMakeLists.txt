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
