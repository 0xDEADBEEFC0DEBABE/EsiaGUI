# The esia_package test: installs this build of Esia into a prefix of its own, then configures, builds and runs
# tests/package (an application using find_package) with the same generator and compilers.
#   cmake -DBUILD=<Esia's build> -DSOURCE=<tests/package> -DWORK=<scratch dir> -DCONFIG=<config> -DGENERATOR=<generator>
#         [-DTOOLCHAIN=<file>] [-DPLATFORM=<generator platform>] [-DCXX=<compiler>] [-DMAKE=<make program>]
#         [-DFORWARD=-Dvar=value|...] -P run_package_test.cmake
function(_run)
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE _rc OUTPUT_VARIABLE _out ERROR_VARIABLE _err)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "esia_package: failed (${_rc}): ${ARGN}\n${_out}\n${_err}")
    endif()
    message(STATUS "${_out}")
endfunction()

set(_prefix ${WORK}/prefix)
set(_build ${WORK}/build)
file(REMOVE_RECURSE ${WORK})
if(NOT CONFIG)
    set(CONFIG Release)
endif()
_run(${CMAKE_COMMAND} --install ${BUILD} --prefix ${_prefix} --config ${CONFIG})

set(_args -S ${SOURCE} -B ${_build} -G ${GENERATOR} -DCMAKE_BUILD_TYPE=${CONFIG})
if(PLATFORM)
    list(APPEND _args -A ${PLATFORM})
endif()
if(TOOLCHAIN)
    list(APPEND _args -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN})
endif()
# the prefix first, then the paths that found Esia's dependencies (vcpkg's toolchain puts its own there): one list -
# a second -DCMAKE_PREFIX_PATH would replace the first
set(_prefix_path ${_prefix})
if(FORWARD)
    string(REPLACE "|" ";" _forward "${FORWARD}")   # the settings that found Esia's dependencies
    foreach(_a ${_forward})
        if(_a MATCHES "^-DCMAKE_PREFIX_PATH=(.*)$")
            string(REPLACE "@SEMICOLON@" ";" _paths "${CMAKE_MATCH_1}")
            list(APPEND _prefix_path ${_paths})
        else()
            string(REPLACE "@SEMICOLON@" "\\;" _a "${_a}")
            list(APPEND _args "${_a}")
        endif()
    endforeach()
endif()
string(REPLACE ";" "\\;" _prefix_path "${_prefix_path}")
list(APPEND _args "-DCMAKE_PREFIX_PATH=${_prefix_path}")
if(NOT GENERATOR MATCHES "^Visual Studio")   # Visual Studio picks its own compiler and MSBuild
    if(CXX)
        list(APPEND _args -DCMAKE_CXX_COMPILER=${CXX})
    endif()
    if(MAKE)
        list(APPEND _args -DCMAKE_MAKE_PROGRAM=${MAKE})
    endif()
endif()
_run(${CMAKE_COMMAND} ${_args})
_run(${CMAKE_COMMAND} --build ${_build} --config ${CONFIG})

find_program(_exe esia_package_user PATHS ${_build} ${_build}/${CONFIG} NO_DEFAULT_PATH NO_CACHE)
if(NOT _exe)
    message(FATAL_ERROR "esia_package: esia_package_user was not built")
endif()
_run(${_exe})
