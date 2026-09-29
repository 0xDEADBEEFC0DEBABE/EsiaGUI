# Shader tooling for WGT UI
#  - wgt_compile_shader : HLSL -> C header (DXBC byte array) through fxc.exe at build time
#  - wgt_embed_files    : text files -> C header (byte arrays) for runtime compilation of user effects

# ---- locate fxc.exe inside the Windows 10/11 SDK ------------------------------
if(NOT WGT_FXC)
    set(_wgt_kits_root "")
    get_filename_component(_wgt_kits_root
        "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE)
    set(_wgt_fxc_hints)
    if(DEFINED ENV{WindowsSdkVerBinPath})
        list(APPEND _wgt_fxc_hints "$ENV{WindowsSdkVerBinPath}/x64")
    endif()
    if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        list(APPEND _wgt_fxc_hints "${_wgt_kits_root}/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/x64")
    endif()
    file(GLOB _wgt_sdk_bins LIST_DIRECTORIES true "${_wgt_kits_root}/bin/10.*")
    list(SORT _wgt_sdk_bins COMPARE NATURAL ORDER DESCENDING)
    foreach(_dir ${_wgt_sdk_bins})
        list(APPEND _wgt_fxc_hints "${_dir}/x64")
    endforeach()
    find_program(WGT_FXC NAMES fxc HINTS ${_wgt_fxc_hints} NO_DEFAULT_PATH)
    find_program(WGT_FXC NAMES fxc)
endif()

if(NOT WGT_FXC)
    message(FATAL_ERROR "fxc.exe not found. Install the Windows 10/11 SDK or pass -DWGT_FXC=<path to fxc.exe>.")
endif()
message(STATUS "WGT: using fxc at ${WGT_FXC}")

# wgt_compile_shader(<out list var> <source.hlsl> <entry> <profile> <C variable name>)
function(wgt_compile_shader OUT_LIST SRC ENTRY PROFILE VARNAME)
    set(_out "${CMAKE_CURRENT_BINARY_DIR}/generated/shaders/${VARNAME}.h")
    get_filename_component(_src_dir "${SRC}" DIRECTORY)
    add_custom_command(
        OUTPUT  "${_out}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated/shaders"
        COMMAND "${WGT_FXC}" /nologo /O3 /Ges /T ${PROFILE} /E ${ENTRY} /Vn ${VARNAME} /I "${_src_dir}" /Fh "${_out}" "${SRC}"
        DEPENDS "${SRC}" ${WGT_SHADER_DEPS}
        COMMENT "fxc ${ENTRY} (${PROFILE}) -> ${VARNAME}.h"
        VERBATIM)
    set(${OUT_LIST} ${${OUT_LIST}} "${_out}" PARENT_SCOPE)
endfunction()

# wgt_embed_files(<out var> <header path> <symbol> <file> [<symbol> <file> ...])
function(wgt_embed_files OUT_VAR HEADER)
    set(_args ${ARGN})
    set(_deps)
    set(_pairs)
    list(LENGTH _args _n)
    math(EXPR _last "${_n} - 1")
    foreach(_i RANGE 0 ${_last} 2)
        math(EXPR _j "${_i} + 1")
        list(GET _args ${_i} _sym)
        list(GET _args ${_j} _file)
        list(APPEND _deps "${_file}")
        list(APPEND _pairs "${_sym}=${_file}")
    endforeach()
    string(REPLACE ";" "|" _pairs_str "${_pairs}")
    add_custom_command(
        OUTPUT  "${HEADER}"
        COMMAND "${CMAKE_COMMAND}" "-DOUT=${HEADER}" "-DPAIRS=${_pairs_str}" -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/WgtEmbed.cmake"
        DEPENDS ${_deps} "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/WgtEmbed.cmake"
        COMMENT "Embedding HLSL sources"
        VERBATIM)
    set(${OUT_VAR} "${HEADER}" PARENT_SCOPE)
endfunction()
