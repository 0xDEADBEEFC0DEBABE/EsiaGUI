# Cross-compiling Windows x64 from Linux WITHOUT the Microsoft SDK: clang + ld.lld against the mingw-w64 headers and
# import libraries (Direct3D 9 / 10 / 11 / 12, DXGI, d3dcompiler included). The fallback when xwin cannot download
# the Windows SDK (download.visualstudio.microsoft.com unreachable, as in the cloud sessions): it checks that the
# DirectX backends compile and link. The binaries use the MinGW ABI (libstdc++, not the MSVC STL) and are not what
# ships; the release build stays clang-cl + lld-link (clang-cl-windows.cmake / clang-cl-xwin.cmake).
#
#   sudo apt-get install g++-mingw-w64-x86-64-posix mingw-w64-x86-64-dev
#   cmake --preset windows-mingw-cross && cmake --build --preset windows-mingw-cross
#
# The .exe files can run under Wine if it is installed (tests stay off: CTest cannot run them here).
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 10.0)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

set(ESIA_LLVM_SUFFIX "" CACHE STRING "Suffix of the LLVM executables (e.g. -18)")
set(ESIA_MINGW_TRIPLE x86_64-w64-mingw32 CACHE STRING "Target triple of the installed mingw-w64 toolchain")
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES ESIA_LLVM_SUFFIX ESIA_MINGW_TRIPLE)

# clang finds the mingw-w64 sysroot (headers, CRT, libstdc++) through the triple's gcc installation
set(CMAKE_C_COMPILER   clang${ESIA_LLVM_SUFFIX})
set(CMAKE_CXX_COMPILER clang++${ESIA_LLVM_SUFFIX})
set(CMAKE_C_COMPILER_TARGET   ${ESIA_MINGW_TRIPLE})
set(CMAKE_CXX_COMPILER_TARGET ${ESIA_MINGW_TRIPLE})
set(CMAKE_RC_COMPILER llvm-windres${ESIA_LLVM_SUFFIX})
set(CMAKE_AR     llvm-ar${ESIA_LLVM_SUFFIX}     CACHE FILEPATH "")
set(CMAKE_RANLIB llvm-ranlib${ESIA_LLVM_SUFFIX} CACHE FILEPATH "")

# self-contained executables: no libgcc / libstdc++ / winpthread DLLs next to them
set(CMAKE_EXE_LINKER_FLAGS_INIT    "-fuse-ld=lld -static")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld -static-libgcc -static-libstdc++")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld -static-libgcc -static-libstdc++")

set(CMAKE_FIND_ROOT_PATH /usr/${ESIA_MINGW_TRIPLE})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
