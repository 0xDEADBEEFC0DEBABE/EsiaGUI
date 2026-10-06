# An application of its own, built against an installed Esia (cmake --install, then find_package): what the
# esia_package test (tests/CMakeLists.txt, run_package_test.cmake) configures, builds and runs.
cmake_minimum_required(VERSION 3.24)
project(EsiaPackageUser LANGUAGES CXX)

find_package(Esia 0.1 CONFIG REQUIRED)

add_executable(esia_package_user main.cpp)
target_link_libraries(esia_package_user PRIVATE esia::ui esia::backends)
if(TARGET esia::text_ft)
    target_link_libraries(esia_package_user PRIVATE esia::text_ft)
    target_compile_definitions(esia_package_user PRIVATE ESIA_PACKAGE_TEXT_FT)
endif()
