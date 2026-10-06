# FreeType and HarfBuzz for the text system (esia_text_ft), and the optional system font libraries of esia_text.
# Included by src/esia/CMakeLists.txt; defines esia::freetype and esia::harfbuzz (and ESIA_TEXT_DEPS_FOUND).
#
#   ESIA_TEXT_DEPS=auto      the system's packages when both are found, else built from source (the default)
#   ESIA_TEXT_DEPS=bundled   always built from source: the pinned archives below, fetched at configure time
#   ESIA_TEXT_DEPS=system    the system's packages (find_package, CMAKE_PREFIX_PATH); an error when missing
#
# The bundled build compiles only what Esia uses: FreeType's OpenType / TrueType / CFF drivers without zlib, bzip2,
# libpng, Brotli or HarfBuzz, and HarfBuzz's amalgamated source without glib, ICU, FreeType, Graphite or the platform
# shapers. Both are static libraries built with their own warnings silenced, never Esia's flags, so -DESIA_WERROR=ON
# stays about Esia's code; their headers are system includes for the same reason.
#
# Offline: point FETCHCONTENT_SOURCE_DIR_ESIA_FREETYPE / FETCHCONTENT_SOURCE_DIR_ESIA_HARFBUZZ at the extracted
# archives (the versions below), or set FETCHCONTENT_FULLY_DISCONNECTED=ON after one configure has downloaded them.
include_guard(GLOBAL)

set(ESIA_TEXT_DEPS auto CACHE STRING "FreeType + HarfBuzz for esia_text_ft: auto, bundled (built from source) or system")
set_property(CACHE ESIA_TEXT_DEPS PROPERTY STRINGS auto bundled system)
if(NOT ESIA_TEXT_DEPS MATCHES "^(auto|bundled|system)$")
    message(FATAL_ERROR "ESIA_TEXT_DEPS must be auto, bundled or system (got '${ESIA_TEXT_DEPS}')")
endif()

set(ESIA_FREETYPE_VERSION 2.14.3)
set(ESIA_FREETYPE_SHA256 36bc4f1cc413335368ee656c42afca65c5a3987e8768cc28cf11ba775e785a5f)
set(ESIA_HARFBUZZ_VERSION 14.5.0)
set(ESIA_HARFBUZZ_SHA256 b7132e148358a45185c9feafd049dbaf243649d3c44414b3534d9c95d18592b9)

# ------------------------------------------------------------------ system packages
function(_esia_find_system_text_deps)
    # HarfBuzz first: vcpkg's harfbuzz config loads freetype's config (targets freetype and Freetype::Freetype),
    # which refuses to load after FindFreetype has defined Freetype::Freetype ("Some (but not all) targets in this
    # export set were already defined")
    find_package(harfbuzz CONFIG QUIET)
    if(NOT TARGET Freetype::Freetype)
        find_package(Freetype QUIET)
    endif()
    if(NOT TARGET harfbuzz::harfbuzz)
        find_path(ESIA_HARFBUZZ_INCLUDE_DIR hb.h PATH_SUFFIXES harfbuzz)
        find_library(ESIA_HARFBUZZ_LIBRARY harfbuzz)
        if(ESIA_HARFBUZZ_INCLUDE_DIR AND ESIA_HARFBUZZ_LIBRARY)
            add_library(harfbuzz::harfbuzz UNKNOWN IMPORTED)
            set_target_properties(harfbuzz::harfbuzz PROPERTIES
                IMPORTED_LOCATION ${ESIA_HARFBUZZ_LIBRARY}
                INTERFACE_INCLUDE_DIRECTORIES ${ESIA_HARFBUZZ_INCLUDE_DIR})
        endif()
    endif()
    if(TARGET Freetype::Freetype AND TARGET harfbuzz::harfbuzz)
        # A package's target may itself be an alias (vcpkg's harfbuzz::harfbuzz is one), and an alias can be neither
        # aliased again nor made global: work with the targets behind them. Imported targets made here are local to
        # this directory: global, so esia::freetype / esia::harfbuzz work everywhere.
        set(_esia_real)
        foreach(_dep Freetype::Freetype harfbuzz::harfbuzz)
            get_target_property(_target ${_dep} ALIASED_TARGET)
            if(NOT _target)
                set(_target ${_dep})
            endif()
            get_target_property(_imported ${_target} IMPORTED)
            if(_imported)
                set_target_properties(${_target} PROPERTIES IMPORTED_GLOBAL TRUE)
            endif()
            list(APPEND _esia_real ${_target})
        endforeach()
        list(GET _esia_real 0 _ft)
        list(GET _esia_real 1 _hb)
        add_library(esia::freetype ALIAS ${_ft})
        add_library(esia::harfbuzz ALIAS ${_hb})
        set(ESIA_TEXT_DEPS_FOUND system PARENT_SCOPE)
        set(ESIA_TEXT_DEPS_VERSIONS "FreeType ${FREETYPE_VERSION_STRING}${freetype_VERSION} + HarfBuzz ${harfbuzz_VERSION} (system)" PARENT_SCOPE)
    endif()
endfunction()

# ------------------------------------------------------------------ built from source
# (re)configuring must not touch unchanged headers: every FreeType source would rebuild
function(_esia_write_if_changed path content)
    if(EXISTS ${path})
        file(READ ${path} _old)
        if(_old STREQUAL content)
            return()
        endif()
    endif()
    file(WRITE ${path} "${content}")
endfunction()

# Third-party code keeps its own warnings to itself.
function(_esia_third_party_defaults target)
    set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W0)
        target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
    else()
        target_compile_options(${target} PRIVATE -w)
    endif()
endfunction()

function(_esia_build_text_deps)
    include(FetchContent)
    # SOURCE_SUBDIR names a directory without a CMakeLists.txt: MakeAvailable then only downloads and extracts, and
    # the targets are defined here instead of by the projects' own build scripts (which install, probe for optional
    # libraries and, for HarfBuzz, warn that its CMake build is unsupported)
    FetchContent_Declare(esia_freetype
        URL https://downloads.sourceforge.net/project/freetype/freetype2/${ESIA_FREETYPE_VERSION}/freetype-${ESIA_FREETYPE_VERSION}.tar.xz
            https://download.savannah.gnu.org/releases/freetype/freetype-${ESIA_FREETYPE_VERSION}.tar.xz
        URL_HASH SHA256=${ESIA_FREETYPE_SHA256}
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        SOURCE_SUBDIR esia-no-cmake)
    FetchContent_Declare(esia_harfbuzz
        URL https://github.com/harfbuzz/harfbuzz/releases/download/${ESIA_HARFBUZZ_VERSION}/harfbuzz-${ESIA_HARFBUZZ_VERSION}.tar.xz
        URL_HASH SHA256=${ESIA_HARFBUZZ_SHA256}
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        SOURCE_SUBDIR esia-no-cmake)
    message(STATUS "Esia text: FreeType ${ESIA_FREETYPE_VERSION} and HarfBuzz ${ESIA_HARFBUZZ_VERSION} from source")
    FetchContent_MakeAvailable(esia_freetype esia_harfbuzz)
    set(ft ${esia_freetype_SOURCE_DIR})
    set(hb ${esia_harfbuzz_SOURCE_DIR})
    if(NOT EXISTS ${ft}/src/base/ftbase.c OR NOT EXISTS ${hb}/src/harfbuzz.cc)
        message(FATAL_ERROR "Esia text: the FreeType / HarfBuzz sources are incomplete (${ft}, ${hb})")
    endif()

    # FreeType: its configuration headers are replaced through the include order (the generated directory comes
    # first). ftmodule.h lists the modules compiled below; ftoption.h is FreeType's own with the internal zlib and
    # LZW readers off (they only unpack WOFF 1 and compressed PCF: a .pcf.gz is unpacked before loading).
    set(cfg ${esia_freetype_BINARY_DIR}/esia-config)
    file(READ ${ft}/include/freetype/config/ftoption.h _options)
    string(REGEX REPLACE "\n#define +(FT_CONFIG_OPTION_USE_ZLIB|FT_CONFIG_OPTION_USE_LZW)\n" "\n/* #undef \\1 (Esia) */\n" _options "${_options}")
    _esia_write_if_changed(${cfg}/freetype/config/ftoption.h "${_options}")
    _esia_write_if_changed(${cfg}/freetype/config/ftmodule.h
"/* Esia: the FreeType modules compiled in (cmake/EsiaTextDeps.cmake) - TrueType and OpenType / CFF outlines, and the
   bitmap fonts: BDF, PCF and Windows FNT / FON */
FT_USE_MODULE( FT_Driver_ClassRec, tt_driver_class )
FT_USE_MODULE( FT_Driver_ClassRec, cff_driver_class )
FT_USE_MODULE( FT_Driver_ClassRec, bdf_driver_class )
FT_USE_MODULE( FT_Driver_ClassRec, pcf_driver_class )
FT_USE_MODULE( FT_Driver_ClassRec, winfnt_driver_class )
FT_USE_MODULE( FT_Module_Class, psaux_module_class )
FT_USE_MODULE( FT_Module_Class, psnames_module_class )
FT_USE_MODULE( FT_Module_Class, pshinter_module_class )
FT_USE_MODULE( FT_Module_Class, sfnt_module_class )
")
    # base: the portable stdio stream and debug files, and the two API files the drivers call into (bitmaps from
    # the SFNT loader, named instances of variable fonts from the TrueType one)
    add_library(esia_freetype STATIC
        ${ft}/src/base/ftsystem.c
        ${ft}/src/base/ftinit.c
        ${ft}/src/base/ftdebug.c
        ${ft}/src/base/ftbase.c
        ${ft}/src/base/ftbitmap.c
        ${ft}/src/base/ftmm.c
        ${ft}/src/sfnt/sfnt.c
        ${ft}/src/truetype/truetype.c
        ${ft}/src/cff/cff.c
        ${ft}/src/psaux/psaux.c
        ${ft}/src/psnames/psnames.c
        ${ft}/src/pshinter/pshinter.c
        ${ft}/src/bdf/bdf.c
        ${ft}/src/pcf/pcf.c
        ${ft}/src/winfonts/winfnt.c)
    target_include_directories(esia_freetype SYSTEM BEFORE PUBLIC ${cfg} ${ft}/include)
    target_compile_definitions(esia_freetype PRIVATE FT2_BUILD_LIBRARY)
    _esia_third_party_defaults(esia_freetype)
    add_library(esia::freetype ALIAS esia_freetype)

    # HarfBuzz: the amalgamated source (every shaper, no optional integration: the defaults of harfbuzz.cc)
    add_library(esia_harfbuzz STATIC ${hb}/src/harfbuzz.cc)
    target_include_directories(esia_harfbuzz SYSTEM PUBLIC ${hb}/src)
    target_compile_features(esia_harfbuzz PRIVATE cxx_std_11)
    if(MSVC)
        # as HarfBuzz's own builds: one translation unit of everything exceeds the default section count (C1128)
        target_compile_options(esia_harfbuzz PRIVATE /bigobj /utf-8)
    endif()
    if(NOT WIN32)
        find_package(Threads REQUIRED)
        target_compile_definitions(esia_harfbuzz PRIVATE HAVE_PTHREAD)
        target_link_libraries(esia_harfbuzz PUBLIC Threads::Threads)
    endif()
    _esia_third_party_defaults(esia_harfbuzz)
    add_library(esia::harfbuzz ALIAS esia_harfbuzz)

    set(ESIA_TEXT_DEPS_FOUND bundled PARENT_SCOPE)
    set(ESIA_TEXT_DEPS_VERSIONS "FreeType ${ESIA_FREETYPE_VERSION} + HarfBuzz ${ESIA_HARFBUZZ_VERSION} (bundled)" PARENT_SCOPE)
endfunction()

set(ESIA_TEXT_DEPS_FOUND "")
if(ESIA_TEXT_DEPS STREQUAL "system" OR ESIA_TEXT_DEPS STREQUAL "auto")
    _esia_find_system_text_deps()
    if(NOT ESIA_TEXT_DEPS_FOUND AND ESIA_TEXT_DEPS STREQUAL "system")
        message(FATAL_ERROR "ESIA_TEXT_DEPS=system: FreeType or HarfBuzz not found (install libfreetype-dev libharfbuzz-dev, "
                            "brew freetype harfbuzz, or set CMAKE_PREFIX_PATH); ESIA_TEXT_DEPS=bundled builds them from source")
    endif()
endif()
if(NOT ESIA_TEXT_DEPS_FOUND)
    enable_language(C)   # FreeType is C (file scope: enable_language cannot run inside a function)
    _esia_build_text_deps()
endif()
