# Esia - the Vulkan RHI backend (ESIA_BACKEND_VULKAN). Needs only the Vulkan headers to build: the loader is opened
# at runtime (vk_loader.hpp), so binaries start - and skip the backend - on machines without Vulkan.
#   Linux: apt-get install libvulkan-dev (headers), mesa-vulkan-drivers (lavapipe) and vulkan-validationlayers to run
#   Windows / macOS: the LunarG Vulkan SDK (VULKAN_SDK), or any Vulkan-Headers on the include path
find_path(ESIA_VULKAN_INCLUDE_DIR vulkan/vulkan.h HINTS $ENV{VULKAN_SDK}/include $ENV{VULKAN_SDK}/Include)
if(NOT ESIA_VULKAN_INCLUDE_DIR)
    message(FATAL_ERROR "ESIA_BACKEND_VULKAN: vulkan/vulkan.h not found (install the Vulkan headers or set VULKAN_SDK)")
endif()

add_library(esia_rhi_vulkan STATIC
    vk_loader.cpp
    vk_formats.cpp
    vk_device.cpp
    vk_headless.cpp)
target_include_directories(esia_rhi_vulkan PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${ESIA_VULKAN_INCLUDE_DIR})
target_link_libraries(esia_rhi_vulkan PUBLIC esia_rhi PRIVATE esia_shaders ${CMAKE_DL_LIBS})
esia_target_defaults(esia_rhi_vulkan)
esia_register_backend(vulkan esia_rhi_vulkan)

if(BUILD_TESTING)
    # unit tests; they render the conformance scenes too, to check them on both render paths and to count the
    # validation messages of a whole device lifetime (the suite's own CTest cannot see those)
    esia_add_test(esia_rhi_vulkan_tests
        tests/test_vk_formats.cpp
        tests/test_vk_device.cpp
        tests/test_vk_scenes.cpp
        tests/test_vk_host.cpp)
    target_include_directories(esia_rhi_vulkan_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(esia_rhi_vulkan_tests PRIVATE esia_rhi_vulkan esia_conformance_scenes)
    target_compile_definitions(esia_rhi_vulkan_tests PRIVATE ESIA_VULKAN_TEST_OUT="${CMAKE_BINARY_DIR}/vulkan")
    # a validation message printed during device destruction has no scene left to fail: the output fails the test
    set_tests_properties(esia_rhi_vulkan_tests PROPERTIES FAIL_REGULAR_EXPRESSION "esia vulkan \\[validation")

    # the conformance suite again with render passes instead of dynamic rendering (the path of Vulkan 1.1 / 1.2
    # devices without VK_KHR_dynamic_rendering); --strict, exit code 77 and a --frames 3 twin as the core's tests
    foreach(_frames 1 3)
        set(_name esia_conformance_vulkan_renderpass)
        if(_frames GREATER 1)
            set(_name esia_conformance_vulkan_renderpass_frames)
        endif()
        add_test(NAME ${_name}
                 COMMAND $<TARGET_FILE:esia_conformance> --backend vulkan --golden ${ESIA_ROOT}/tests/conformance/golden
                         --out ${CMAKE_BINARY_DIR}/conformance-renderpass --frames ${_frames} --strict)
        set_tests_properties(${_name} PROPERTIES ENVIRONMENT "ESIA_VULKAN_RENDER_PASS=1" SKIP_RETURN_CODE 77
                             FAIL_REGULAR_EXPRESSION "esia vulkan \\[validation")
    endforeach()
endif()
