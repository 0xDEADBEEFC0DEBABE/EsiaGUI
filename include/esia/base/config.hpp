// Esia - build configuration shared by every module (no platform headers, no Dear ImGui).
#pragma once
#include <cassert>
#include <cstdint>

// Esia is built as static libraries for now; a shared build defines ESIA_SHARED and ESIA_BUILD_DLL.
#if defined(ESIA_SHARED)
#  if defined(_WIN32)
#    if defined(ESIA_BUILD_DLL)
#      define ESIA_API __declspec(dllexport)
#    else
#      define ESIA_API __declspec(dllimport)
#    endif
#  else
#    define ESIA_API __attribute__((visibility("default")))
#  endif
#else
#  define ESIA_API
#endif

#define ESIA_ASSERT(expr) assert(expr)

#if defined(__clang__) || defined(__GNUC__)
#  define ESIA_PRINTF(fmtIndex, argIndex) __attribute__((format(printf, fmtIndex, argIndex)))
#else
#  define ESIA_PRINTF(fmtIndex, argIndex)
#endif

namespace esia
{
    constexpr int kVersionMajor = 0;
    constexpr int kVersionMinor = 1;

    // Identifier of an item / window: a hash of labels and the id stack (see core/id.hpp). 0 = none.
    using Id = std::uint32_t;
    // Handle of a texture in the TextureRegistry (core/texture.hpp). 0 = none (the renderer binds white).
    using TextureId = std::uint64_t;
    // Registered custom FX effect (0 = built-in shader).
    using EffectId = std::uint32_t;
}
