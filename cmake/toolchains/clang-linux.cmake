# LLVM toolchain for Linux: clang / clang++ with lld.
#   cmake --preset linux-clang        (or -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang-linux.cmake)
# Versioned compilers (clang++-18 ...) can be picked with -DESIA_LLVM_SUFFIX=-18.
set(ESIA_LLVM_SUFFIX "" CACHE STRING "Suffix of the LLVM executables (e.g. -18)")
set(CMAKE_C_COMPILER   clang${ESIA_LLVM_SUFFIX})
set(CMAKE_CXX_COMPILER clang++${ESIA_LLVM_SUFFIX})
set(CMAKE_AR     llvm-ar${ESIA_LLVM_SUFFIX}     CACHE FILEPATH "")
set(CMAKE_RANLIB llvm-ranlib${ESIA_LLVM_SUFFIX} CACHE FILEPATH "")
set(CMAKE_EXE_LINKER_FLAGS_INIT    "-fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld")
