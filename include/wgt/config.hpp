// WGT UI - build configuration, export macros and version
#pragma once

#ifndef IMGUI_USER_CONFIG
#   define IMGUI_USER_CONFIG "wgt/imconfig_wgt.h"
#endif

#include <cstdint>
#include <cstddef>

#if defined(WGT_BUILD_DLL)
#   define WGT_API __declspec(dllexport)
#elif defined(WGT_STATIC)
#   define WGT_API
#else
#   define WGT_API __declspec(dllimport)
#endif

#define WGT_VERSION_MAJOR 1
#define WGT_VERSION_MINOR 1
#define WGT_VERSION_PATCH 0
#define WGT_VERSION_STRING "1.1.0"
// Bumped whenever an exported struct/interface layout changes (plugins check it on load).
#define WGT_ABI_VERSION 8u

#include "imgui.h"

#if !defined(WGT_IMCONFIG_INCLUDED)
#   error "imgui.h was included before wgt headers without IMGUI_USER_CONFIG=\"wgt/imconfig_wgt.h\". Include <wgt/wgt.hpp> first or add the define to your build."
#endif

namespace wgt
{
    class Context;
    class Painter;

    // Opaque handle of a user effect registered with Context::RegisterEffect(). 0 = none.
    using EffectId = std::uint32_t;
    // Unicode code point of an icon glyph (see wgt/icons.hpp).
    using Icon = std::uint32_t;

    WGT_API const char* GetVersionString();
    WGT_API std::uint32_t GetAbiVersion();
}
