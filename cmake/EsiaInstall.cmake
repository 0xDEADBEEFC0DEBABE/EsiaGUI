# Esia - `cmake --install`: the libraries, their headers and a CMake package. An application then finds them with
#
#   find_package(Esia CONFIG REQUIRED)   # CMAKE_PREFIX_PATH: the install prefix
#   target_link_libraries(app PRIVATE esia::ui esia::text_ft esia::backends esia::platform_win32)
#
# - the names add_subdirectory gives them (the aliases). Included at the end of the top-level CMakeLists.txt, when every
# target exists, so the CMakeLists.txt of the libraries and backends need not know about it: the include directories in
# the source and build trees are marked as the build's only, and the `include` directories among them are installed.
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(_esia_install_targets)
set(_esia_header_dirs)
string(REGEX REPLACE "/+$" "" _esia_src "${ESIA_ROOT}")   # ESIA_ROOT may end in a slash
string(REGEX REPLACE "/+$" "" _esia_bin "${CMAKE_BINARY_DIR}")
foreach(_t esia_core esia_rhi esia_shaders esia_render esia_text esia_text_ft esia_ui esia_backends
           esia_rhi_d3d_common esia_rhi_d3d9 esia_rhi_d3d10 esia_rhi_d3d11 esia_rhi_d3d12 esia_rhi_opengl
           esia_rhi_vulkan esia_rhi_metal_core esia_rhi_metal
           esia_platform_win32 esia_platform_x11 esia_platform_android
           esia_freetype esia_harfbuzz)   # the last two: the bundled FreeType and HarfBuzz (ESIA_TEXT_DEPS)
    if(NOT TARGET ${_t})
        continue()
    endif()
    list(APPEND _esia_install_targets ${_t})
    string(REGEX REPLACE "^esia_" "" _name ${_t})
    set_target_properties(${_t} PROPERTIES EXPORT_NAME ${_name})
    foreach(_prop INTERFACE_INCLUDE_DIRECTORIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES)
        get_target_property(_dirs ${_t} ${_prop})
        if(NOT _dirs)
            continue()
        endif()
        set(_out)
        foreach(_d ${_dirs})
            string(REPLACE "//" "/" _p "${_d}/")
            string(FIND "${_p}" "${_esia_src}/" _in_source)
            string(FIND "${_p}" "${_esia_bin}/" _in_build)
            if(_d MATCHES "^\\$<" OR NOT (_in_source EQUAL 0 OR _in_build EQUAL 0))
                list(APPEND _out "${_d}")   # a generator expression, or outside both trees (the Vulkan headers)
            else()
                list(APPEND _out "$<BUILD_INTERFACE:${_d}>")
                if(_in_source EQUAL 0 AND _d MATCHES "/include$")
                    list(APPEND _esia_header_dirs ${_d})
                endif()
            endif()
        endforeach()
        set_target_properties(${_t} PROPERTIES ${_prop} "${_out}")
    endforeach()
endforeach()
list(REMOVE_DUPLICATES _esia_header_dirs)

install(TARGETS ${_esia_install_targets} EXPORT EsiaTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
foreach(_d ${_esia_header_dirs})
    install(DIRECTORY ${_d}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endforeach()
install(EXPORT EsiaTargets NAMESPACE esia:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Esia)

# what the package's config must find again: the libraries Esia's static libraries link against
function(_esia_links target pattern out)
    set(${out} OFF PARENT_SCOPE)
    if(TARGET ${target})
        get_target_property(_libs ${target} INTERFACE_LINK_LIBRARIES)
        if(_libs MATCHES "${pattern}")
            set(${out} ON PARENT_SCOPE)
        endif()
    endif()
endfunction()
_esia_links(esia_text "Fontconfig::Fontconfig" ESIA_PACKAGE_FONTCONFIG)
_esia_links(esia_platform_x11 "X11::X11" ESIA_PACKAGE_X11)
set(ESIA_PACKAGE_SYSTEM_TEXT_DEPS OFF)
if(TARGET esia_text_ft AND NOT TARGET esia_freetype)
    set(ESIA_PACKAGE_SYSTEM_TEXT_DEPS ON)   # ESIA_TEXT_DEPS found the system's FreeType and HarfBuzz
endif()
set(_esia_cmake_dir ${CMAKE_INSTALL_LIBDIR}/cmake/Esia)
configure_package_config_file(${ESIA_ROOT}/cmake/EsiaConfig.cmake.in ${CMAKE_BINARY_DIR}/EsiaConfig.cmake
                              INSTALL_DESTINATION ${_esia_cmake_dir})
write_basic_package_version_file(${CMAKE_BINARY_DIR}/EsiaConfigVersion.cmake VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMinorVersion)   # 0.x: a minor version may change the API
install(FILES ${CMAKE_BINARY_DIR}/EsiaConfig.cmake ${CMAKE_BINARY_DIR}/EsiaConfigVersion.cmake
        DESTINATION ${_esia_cmake_dir})
