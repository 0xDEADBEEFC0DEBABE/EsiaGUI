# Esia - checks the generated MSL (src/esia/shaders/generated/msl/*.metal).
#
#   cmake -DMODE=xcrun -DXCRUN=xcrun -DMSL_DIR=<dir> -DOUT_DIR=<dir> -P check_msl.cmake
#       macOS: compiles every file with Apple's Metal compiler for macOS (MSL 2.0, what metal_device.mm asks for at
#       runtime) and, when the iOS SDK is installed, for iOS - the real check.
#   cmake -DMODE=mock -DCLANG=clang++ -DMSL_DIR=<dir> -MOCK_DIR=<dir> -DOUT_DIR=<dir> -P check_msl.cmake
#       anywhere with clang: type-checks every file as C++ against the mock Metal standard library in this directory
#       (metal_stdlib explains what that does and does not prove). The rewrites below are the ones the mock needs:
#       SPIRV-Cross's spvUnsafeArray definition out (the mock has its own), vector constructors to make_<type>N
#       calls, float literals suffixed with f.
file(GLOB _files "${MSL_DIR}/*.metal")
list(SORT _files)
if(NOT _files)
    message(FATAL_ERROR "no .metal files in ${MSL_DIR}")
endif()
file(MAKE_DIRECTORY "${OUT_DIR}")
set(_failed "")
set(_count 0)

if(MODE STREQUAL "xcrun")
    execute_process(COMMAND ${XCRUN} -sdk iphoneos --show-sdk-path RESULT_VARIABLE _no_ios OUTPUT_QUIET ERROR_QUIET)
endif()

foreach(_f ${_files})
    get_filename_component(_name "${_f}" NAME)
    math(EXPR _count "${_count} + 1")
    if(MODE STREQUAL "xcrun")
        execute_process(COMMAND ${XCRUN} -sdk macosx metal -std=macos-metal2.0 -c "${_f}" -o "${OUT_DIR}/${_name}.macos.air"
                        RESULT_VARIABLE _r OUTPUT_VARIABLE _o ERROR_VARIABLE _e)
        if(NOT _r EQUAL 0)
            list(APPEND _failed "${_name} (macOS): ${_e}")
        elseif(_e)
            message(STATUS "${_name} (macOS) warnings:\n${_e}")
        endif()
        if(_no_ios EQUAL 0)
            execute_process(COMMAND ${XCRUN} -sdk iphoneos metal -std=ios-metal2.0 -c "${_f}" -o "${OUT_DIR}/${_name}.ios.air"
                            RESULT_VARIABLE _r OUTPUT_VARIABLE _o ERROR_VARIABLE _e)
            if(NOT _r EQUAL 0)
                list(APPEND _failed "${_name} (iOS): ${_e}")
            endif()
        endif()
    elseif(MODE STREQUAL "mock")
        file(READ "${_f}" _src)
        string(FIND "${_src}" "template<typename T, size_t Num>\nstruct spvUnsafeArray" _a)
        if(_a GREATER -1)
            string(SUBSTRING "${_src}" ${_a} -1 _tail)
            string(FIND "${_tail}" "\n};\n" _b)
            math(EXPR _b "${_b} + 4")
            string(SUBSTRING "${_src}" 0 ${_a} _head)
            string(SUBSTRING "${_tail}" ${_b} -1 _tail)
            set(_src "${_head}${_tail}")
        endif()
        string(REGEX REPLACE "([^A-Za-z0-9_])(float|int|uint|bool)([234])\\(" "\\1make_\\2\\3(" _src "${_src}")
        string(REGEX REPLACE "([0-9]+\\.[0-9]+(e[-+][0-9]+)?)" "\\1f" _src "${_src}")
        set(_cpp "${OUT_DIR}/${_name}.cpp")
        file(WRITE "${_cpp}" "${_src}")
        execute_process(COMMAND ${CLANG} -x c++ -std=c++17 -fsyntax-only -I "${MOCK_DIR}" -Wall -Wno-unknown-attributes
                                -Wno-unused-variable -Wno-unused-but-set-variable "${_cpp}"
                        RESULT_VARIABLE _r OUTPUT_VARIABLE _o ERROR_VARIABLE _e)
        if(NOT _r EQUAL 0 OR _e)
            list(APPEND _failed "${_name}: ${_e}")
        endif()
    else()
        message(FATAL_ERROR "MODE must be xcrun or mock")
    endif()
endforeach()

if(_failed)
    string(REPLACE ";" "\n" _msg "${_failed}")
    message(FATAL_ERROR "MSL check (${MODE}) failed:\n${_msg}")
endif()
message(STATUS "MSL check (${MODE}): ${_count} files OK")
