#include "esia/core/cpu.hpp"
#include <atomic>
#include <cstdlib>
#include <cstring>

#if defined(_M_X64) || defined(__x86_64__)
#define ESIA_CPU_X86 1
#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <cpuid.h>
#endif
#endif

namespace esia::cpu
{
    namespace
    {
#if ESIA_CPU_X86
        void CpuId(unsigned leaf, unsigned sub, unsigned r[4])
        {
#if defined(_MSC_VER)
            int v[4];
            __cpuidex(v, (int)leaf, (int)sub);
            for (int i = 0; i < 4; ++i)
                r[i] = (unsigned)v[i];
#else
            __cpuid_count(leaf, sub, r[0], r[1], r[2], r[3]);
#endif
        }

        // XCR0: which register files the operating system saves on a context switch
        unsigned long long SavedState()
        {
#if defined(_MSC_VER)
            return _xgetbv(0);
#else
            unsigned lo = 0, hi = 0;
            __asm__ volatile("xgetbv" : "=a"(lo), "=d"(hi) : "c"(0));
            return ((unsigned long long)hi << 32) | lo;
#endif
        }
#endif

        Simd Probe()
        {
#if ESIA_CPU_X86
            unsigned r[4];
            CpuId(0, 0, r);
            const unsigned leaves = r[0];
            CpuId(1, 0, r);
            const bool osxsave = (r[2] >> 27) & 1u, avx = (r[2] >> 28) & 1u;
            if (!osxsave || !avx || leaves < 7)
                return Simd::Sse2;
            const unsigned long long state = SavedState();
            if ((state & 0x6u) != 0x6u)   // XMM and YMM
                return Simd::Sse2;
            CpuId(7, 0, r);
            const bool avx2 = (r[1] >> 5) & 1u;
            const bool avx512 = ((r[1] >> 16) & 1u) && ((r[1] >> 17) & 1u) && ((r[1] >> 30) & 1u) && ((r[1] >> 31) & 1u);   // F DQ BW VL
            if (avx2 && avx512 && (state & 0xE6u) == 0xE6u)   // ... and the opmasks, ZMM_Hi256, Hi16_ZMM
                return Simd::Avx512;
            return avx2 ? Simd::Avx2 : Simd::Sse2;
#elif defined(__aarch64__) || defined(_M_ARM64)
            return Simd::Neon;
#else
            return Simd::Scalar;
#endif
        }

        Simd Parse(const char* s, Simd fallback)
        {
            const char* const names[] = {"scalar", "sse2", "avx2", "avx512", "neon"};
            for (int i = 0; i < 5; ++i)
                if (s && std::strcmp(s, names[i]) == 0 && Runs((Simd)i))
                    return (Simd)i;
            return fallback;
        }

        std::atomic<int> gForced{-1};
    }

    Simd Detected()
    {
        static const Simd level = Probe();
        return level;
    }

    bool Runs(Simd level)
    {
        const Simd best = Detected();
        if (level == Simd::Scalar || level == best)
            return true;
        if (best == Simd::Neon || level == Simd::Neon)
            return false;
        return (int)level <= (int)best;   // x86: SSE2 < AVX2 < AVX-512
    }

    Simd Active()
    {
        const int forced = gForced.load(std::memory_order_relaxed);
        if (forced >= 0)
            return (Simd)forced;
        static const Simd level = Parse(std::getenv("ESIA_SIMD"), Detected());
        return level;
    }

    void Force(Simd level) { gForced.store((int)(Runs(level) ? level : Detected()), std::memory_order_relaxed); }

    void Unforce() { gForced.store(-1, std::memory_order_relaxed); }

    const char* Name(Simd level)
    {
        switch (level)
        {
        case Simd::Sse2: return "sse2";
        case Simd::Avx2: return "avx2";
        case Simd::Avx512: return "avx512";
        case Simd::Neon: return "neon";
        default: return "scalar";
        }
    }
}
