// Esia - the processor's vector instructions, for the loops written for them (a polyline's joints and pieces): detected
// once at run time and chosen per call, with plain C++ where there are none. Every level computes the same numbers - the
// same IEEE operations in the same order, exact square roots and divisions, no fused multiply-adds - so what is drawn
// never depends on the level (the render tests compare them bit for bit).
#pragma once

#include <cstdint>

namespace esia::cpu
{
    enum class Simd : std::uint8_t
    {
        Scalar,   // plain C++
        Sse2,     // x86-64's baseline: 4 floats a step
        Avx2,     // 8 floats (Intel Haswell, AMD Zen and later)
        Avx512,   // 16 floats; F, VL, BW and DQ (Intel Skylake-X / Ice Lake, AMD Zen 4 and later)
        Neon,     // AArch64: 4 floats
    };

    // The best level this processor and its operating system run (on x86 cpuid and the register state the OS saves).
    Simd Detected();
    // What the loops use: Detected(), or a lower level named by the environment variable ESIA_SIMD (scalar, sse2, avx2,
    // avx512, neon) or by Force. A level the processor cannot run gives Detected() instead.
    Simd Active();
    // Tests and benchmarks: run `level` (as ESIA_SIMD does) until Unforce. Not for use while other threads draw.
    void Force(Simd level);
    void Unforce();
    // Whether this processor runs `level` (Scalar always).
    bool Runs(Simd level);
    const char* Name(Simd level);
}
