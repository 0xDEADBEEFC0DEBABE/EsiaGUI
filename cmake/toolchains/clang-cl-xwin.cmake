# Cross-compiling the Windows (x64, DirectX) build from Linux: clang-cl + lld-link against a Windows SDK and the
# MSVC CRT / STL fetched with xwin (https://github.com/Jake-Shadle/xwin). See docs/backends/README.md, "LLVM":
#
#   cargo install xwin --locked
#   xwin --accept-license --arch x86_64 splat --output ~/.xwin      # creates crt/ and sdk/ (lower-case symlinks)
#   cmake --preset windows-cross -DXWIN_DIR=$HOME/.xwin
#
# XWIN_DIR may also come from the environment. Nothing here runs Windows code: the result is a PE build that is
# checked for compile and link errors only (run it on Windows or under Wine).
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 10.0)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

set(ESIA_LLVM_SUFFIX "" CACHE STRING "Suffix of the LLVM executables (e.g. -18)")
if(NOT XWIN_DIR)
    if(DEFINED ENV{XWIN_DIR})
        set(XWIN_DIR "$ENV{XWIN_DIR}")
    else()
        set(XWIN_DIR "$ENV{HOME}/.xwin")
    endif()
endif()
set(XWIN_DIR "${XWIN_DIR}" CACHE PATH "Output directory of 'xwin splat'")
if(NOT EXISTS "${XWIN_DIR}/crt/include" OR NOT EXISTS "${XWIN_DIR}/sdk/include/um")
    message(FATAL_ERROR "XWIN_DIR='${XWIN_DIR}' does not hold an 'xwin splat' output (crt/include, sdk/include/um). "
                        "Run: xwin --accept-license --arch x86_64 splat --output ${XWIN_DIR}")
endif()
# try_compile projects get the same settings
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES XWIN_DIR ESIA_LLVM_SUFFIX)

set(CMAKE_C_COMPILER   clang-cl${ESIA_LLVM_SUFFIX})
set(CMAKE_CXX_COMPILER clang-cl${ESIA_LLVM_SUFFIX})
set(CMAKE_LINKER       lld-link${ESIA_LLVM_SUFFIX})
set(CMAKE_AR           llvm-lib${ESIA_LLVM_SUFFIX})
set(CMAKE_RC_COMPILER  llvm-rc${ESIA_LLVM_SUFFIX})
set(CMAKE_MT           llvm-mt${ESIA_LLVM_SUFFIX})
set(CMAKE_C_COMPILER_TARGET   x86_64-pc-windows-msvc)
set(CMAKE_CXX_COMPILER_TARGET x86_64-pc-windows-msvc)

# /imsvc: system headers (no warnings from them); the case-insensitive Windows headers are handled by the
# lower-case symlinks xwin creates.
set(_xwin_includes
    "/imsvc${XWIN_DIR}/crt/include"
    "/imsvc${XWIN_DIR}/sdk/include/ucrt"
    "/imsvc${XWIN_DIR}/sdk/include/um"
    "/imsvc${XWIN_DIR}/sdk/include/shared"
    "/imsvc${XWIN_DIR}/sdk/include/winrt")
string(JOIN " " _xwin_flags ${_xwin_includes})
set(CMAKE_C_FLAGS_INIT   "${_xwin_flags}")
set(CMAKE_CXX_FLAGS_INIT "${_xwin_flags} /EHsc")
set(CMAKE_RC_FLAGS_INIT  "-I${XWIN_DIR}/sdk/include/um -I${XWIN_DIR}/sdk/include/shared")

set(_xwin_libs
    "/libpath:${XWIN_DIR}/crt/lib/x86_64"
    "/libpath:${XWIN_DIR}/sdk/lib/um/x86_64"
    "/libpath:${XWIN_DIR}/sdk/lib/ucrt/x86_64")
string(JOIN " " _xwin_link ${_xwin_libs})
set(CMAKE_EXE_LINKER_FLAGS_INIT    "${_xwin_link}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${_xwin_link}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${_xwin_link}")

set(CMAKE_FIND_ROOT_PATH "${XWIN_DIR}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
