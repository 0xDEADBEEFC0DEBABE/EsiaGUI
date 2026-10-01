// Esia - hashing for ids and caches.
//
// Ids are 32-bit FNV-1a hashes chained through the id stack (the seed is the parent id), so "OK" inside two
// different windows gets two different ids. Labels follow the usual immediate-mode conventions:
//   "Save##toolbar"   -> displayed "Save", hashed "Save##toolbar"
//   "Save###save"     -> displayed "Save", hashed "###save" only (the label may change without changing the id)
#pragma once
#include "config.hpp"
#include <cstddef>
#include <string_view>

namespace esia
{
    constexpr std::uint32_t kFnvOffset = 2166136261u;
    constexpr std::uint32_t kFnvPrime = 16777619u;

    constexpr std::uint32_t HashBytes(const void* data, std::size_t size, std::uint32_t seed)
    {
        // the seed is folded in first so equal payloads under different parents never collide trivially
        std::uint32_t h = kFnvOffset ^ seed;
        const unsigned char* p = static_cast<const unsigned char*>(data);
        for (std::size_t i = 0; i < size; ++i)
            h = (h ^ p[i]) * kFnvPrime;
        return h;
    }

    constexpr std::uint32_t HashString(std::string_view s, std::uint32_t seed)
    {
        std::uint32_t h = kFnvOffset ^ seed;
        for (char c : s)
            h = (h ^ (unsigned char)c) * kFnvPrime;
        return h;
    }

    // Id of a label under `seed`, with the "##" / "###" conventions above. Never returns 0.
    constexpr Id HashLabel(std::string_view label, Id seed)
    {
        const std::size_t triple = label.find("###");
        if (triple != std::string_view::npos)
            label = label.substr(triple);
        const Id h = HashString(label, seed);
        return h != 0 ? h : 1u;
    }

    constexpr Id HashInt(std::int64_t v, Id seed)
    {
        unsigned char b[8] = {};
        for (int i = 0; i < 8; ++i)
            b[i] = (unsigned char)((std::uint64_t)v >> (i * 8));
        const Id h = HashBytes(b, 8, seed);
        return h != 0 ? h : 1u;
    }

    // The part of a label that is displayed (everything before "##").
    constexpr std::string_view LabelText(std::string_view label)
    {
        const std::size_t p = label.find("##");
        return p == std::string_view::npos ? label : label.substr(0, p);
    }
}
