# Esia OpenGL backend (ESIA_BACKEND_OPENGL): OpenGL 3.3 core and OpenGL ES 3.0 in one target, registered as the
# backends "opengl" and "gles". GL and EGL are loaded at runtime: nothing but the platform's dynamic loader is linked.
add_library(esia_rhi_opengl STATIC gl_api.cpp gl_device.cpp gl_headless.cpp)
target_include_directories(esia_rhi_opengl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(esia_rhi_opengl PUBLIC esia_rhi PRIVATE esia_shaders ${CMAKE_DL_LIBS})
if(WIN32)
    target_link_libraries(esia_rhi_opengl PRIVATE user32 gdi32)   # the hidden window and pixel format of WGL headless contexts
endif()
esia_target_defaults(esia_rhi_opengl)
add_library(esia::rhi_opengl ALIAS esia_rhi_opengl)
esia_register_backend(opengl esia_rhi_opengl)
esia_register_backend(gles esia_rhi_opengl)

# Device tests on the machine's driver (Mesa llvmpipe through EGL in CI, the GPU driver through WGL on Windows; skipped
# without either): formats, copies,
# readback, host state, timing, and every conformance scene with zero GL errors in both APIs.
if(BUILD_TESTING)
    esia_add_test(esia_rhi_opengl_tests tests/test_gl_device.cpp ${ESIA_ROOT}/tests/conformance/scenes.cpp)
    target_include_directories(esia_rhi_opengl_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${ESIA_ROOT}/tests/conformance ${ESIA_ROOT}/tests/support)
    target_link_libraries(esia_rhi_opengl_tests PRIVATE esia_backends esia_rhi_opengl esia_render)
endif()
