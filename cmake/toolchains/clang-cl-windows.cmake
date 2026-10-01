# LLVM toolchain for Windows (native): clang-cl with lld-link, llvm-lib, llvm-rc, llvm-mt.
# Run from a "x64 Native Tools" prompt (or after vcvars64.bat) so the MSVC STL / CRT and the Windows SDK are
# on INCLUDE / LIB; the LLVM installer's bin directory must be on PATH.
#   cmake --preset windows-clang-cl
set(CMAKE_C_COMPILER   clang-cl)
set(CMAKE_CXX_COMPILER clang-cl)
set(CMAKE_LINKER       lld-link)
set(CMAKE_AR           llvm-lib)
set(CMAKE_RC_COMPILER  llvm-rc)
set(CMAKE_MT           llvm-mt)
