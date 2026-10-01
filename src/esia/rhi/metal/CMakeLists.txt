# Esia - the Metal RHI backend (macOS 11+ / iOS 14+, Apple silicon and Intel). ESIA_BACKEND_METAL, OFF by default.
#
#   esia_rhi_metal_core   the backend's logic in plain C++ (tables, planning, frames, profiling, the rhi::Device on
#                         the Gpu layer): built and unit-tested on every host
#   esia_rhi_metal        on Apple platforms: + metal_device.mm (the Gpu layer on Metal, host integration,
#                         registration); elsewhere: a registration whose headless creator says why the conformance
#                         suite skips Metal
#
# UNVERIFIED: needs macOS. The Apple part was written on Linux and never compiled (see STATUS.md).

add_library(esia_rhi_metal_core STATIC
    metal_tables.cpp
    metal_planning.cpp
    metal_frames.cpp
    metal_profiler.cpp
    metal_device_core.cpp)
target_include_directories(esia_rhi_metal_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(esia_rhi_metal_core PUBLIC esia_rhi PRIVATE esia_shaders)
esia_target_defaults(esia_rhi_metal_core)

if(APPLE)
    # Objective-C++ with ARC for metal_device.mm; the C++ compiler of the toolchain compiles it (Homebrew LLVM's
    # clang or Apple's), so both languages agree on the standard library.
    if(NOT CMAKE_OBJCXX_COMPILER)
        set(CMAKE_OBJCXX_COMPILER ${CMAKE_CXX_COMPILER})
    endif()
    enable_language(OBJCXX)
    # The backend's minimum (counter sampling, blit pass descriptors, sampleTimestamps): macOS 11 / iOS 14
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set(_esia_metal_min 14.0)
    else()
        set(_esia_metal_min 11.0)
    endif()
    if(CMAKE_OSX_DEPLOYMENT_TARGET AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS _esia_metal_min)
        message(FATAL_ERROR "The Metal backend needs CMAKE_OSX_DEPLOYMENT_TARGET ${_esia_metal_min} or later")
    endif()
    add_library(esia_rhi_metal STATIC metal_device.mm)
    set(_esia_metal_objc_options -fobjc-arc)
    # clang 23 calls class methods through objc_msgSendClass$<sel>$<class> stubs, which ld64.lld 23 cannot
    # synthesize (undefined symbols at link time); plain objc_msgSend$<sel> stubs link with both linkers
    include(CheckCompilerFlag)
    check_compiler_flag(OBJCXX -fno-objc-msgsend-class-selector-stubs ESIA_OBJC_NO_CLASS_STUBS)
    if(ESIA_OBJC_NO_CLASS_STUBS)
        list(APPEND _esia_metal_objc_options -fno-objc-msgsend-class-selector-stubs)
    endif()
    set(ESIA_METAL_OBJC_OPTIONS ${_esia_metal_objc_options} CACHE INTERNAL "Objective-C++ options of the Metal backend's sources")
    set_source_files_properties(metal_device.mm PROPERTIES COMPILE_OPTIONS "${_esia_metal_objc_options}")
    # Executables that link the backend keep C++ as their linker language (CXX outranks OBJCXX), so the
    # Objective-C runtime (objc_msgSend, the ARC entry points) is linked explicitly: Foundation does not re-export it
    target_link_libraries(esia_rhi_metal PUBLIC esia_rhi_metal_core PRIVATE esia_shaders "-framework Metal" "-framework Foundation" objc)
else()
    add_library(esia_rhi_metal STATIC metal_unavailable.cpp)
    target_link_libraries(esia_rhi_metal PUBLIC esia_rhi_metal_core)
endif()
esia_target_defaults(esia_rhi_metal)
esia_register_backend(metal esia_rhi_metal)

if(BUILD_TESTING)
    # The backend's logic through the real renderer and the conformance scenes, on a fake Gpu that enforces the
    # Metal rules the backend relies on (and parses the generated MSL for each program's resources).
    esia_add_test(esia_rhi_metal_tests
        tests/test_metal_tables.cpp
        tests/test_metal_planning.cpp
        tests/test_metal_frames.cpp
        tests/test_metal_profiler.cpp
        tests/test_metal_msl.cpp
        tests/test_metal_device.cpp
        tests/msl_interface.cpp
        tests/fake_metal_gpu.cpp)
    target_include_directories(esia_rhi_metal_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(esia_rhi_metal_tests PRIVATE esia_rhi_metal_core esia_shaders esia_conformance_scenes)
    target_compile_definitions(esia_rhi_metal_tests PRIVATE ESIA_METAL_NULL_GOLDEN_DIR="${ESIA_ROOT}/tests/conformance/golden/null")

    # The generated MSL (src/esia/shaders/generated/msl): on macOS compiled by Apple's compiler; elsewhere
    # type-checked by clang against a mock of the Metal standard library (tests/msl_check, see its README).
    if(APPLE)
        find_program(ESIA_XCRUN xcrun)
        if(ESIA_XCRUN)
            add_test(NAME esia_rhi_metal_msl_compile
                     COMMAND ${CMAKE_COMMAND} -DMODE=xcrun -DXCRUN=${ESIA_XCRUN} -DMSL_DIR=${ESIA_ROOT}/src/esia/shaders/generated/msl
                             -DOUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/msl_check -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/msl_check/check_msl.cmake)
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
        # (not with clang-cl: the checks pass GCC-style options, which clang-cl ignores with a warning)
        # metal_device.mm parsed as Objective-C++ with ARC against mock Foundation / Metal declarations
        # (tests/objc_check): its syntax, types, bridging and warnings - not Apple's API itself
        add_test(NAME esia_rhi_metal_objc_mock_check
                 COMMAND ${CMAKE_CXX_COMPILER} -x objective-c++ -std=c++20 -fobjc-arc -fobjc-runtime=macosx-11.0 -fblocks -fsyntax-only
                         -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wno-missing-field-initializers -Werror
                         -I${CMAKE_CURRENT_SOURCE_DIR}/tests/objc_check -I${CMAKE_CURRENT_SOURCE_DIR} -I${CMAKE_CURRENT_SOURCE_DIR}/include
                         -I${ESIA_ROOT}/include ${CMAKE_CURRENT_SOURCE_DIR}/metal_device.mm)
        add_test(NAME esia_rhi_metal_msl_mock_check
                 COMMAND ${CMAKE_COMMAND} -DMODE=mock -DCLANG=${CMAKE_CXX_COMPILER} -DMSL_DIR=${ESIA_ROOT}/src/esia/shaders/generated/msl
                         -DMOCK_DIR=${CMAKE_CURRENT_SOURCE_DIR}/tests/msl_check -DOUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/msl_check
                         -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/msl_check/check_msl.cmake)
    endif()
endif()
