// esia::cpu: the vector level detected, the levels a processor runs, forcing one for a test and going back.
#include "esia/core/cpu.hpp"
#include "esia_test.hpp"
#include <cstring>

using namespace esia;

ESIA_TEST(Cpu, LevelsAndForcing)
{
    const cpu::Simd best = cpu::Detected();
    ESIA_CHECK(cpu::Runs(best) && cpu::Runs(cpu::Simd::Scalar));
#if defined(_M_X64) || defined(__x86_64__)
    ESIA_CHECK(best == cpu::Simd::Sse2 || best == cpu::Simd::Avx2 || best == cpu::Simd::Avx512);
    ESIA_CHECK(cpu::Runs(cpu::Simd::Sse2) && !cpu::Runs(cpu::Simd::Neon));
#elif defined(__aarch64__) || defined(_M_ARM64)
    ESIA_CHECK(best == cpu::Simd::Neon && !cpu::Runs(cpu::Simd::Sse2));
#endif
    cpu::Force(cpu::Simd::Scalar);
    ESIA_CHECK(cpu::Active() == cpu::Simd::Scalar);
    cpu::Force(best);
    ESIA_CHECK(cpu::Active() == best);
    cpu::Unforce();
    ESIA_CHECK(cpu::Runs(cpu::Active()));
    ESIA_CHECK(std::strcmp(cpu::Name(cpu::Simd::Avx512), "avx512") == 0 && std::strcmp(cpu::Name(cpu::Simd::Scalar), "scalar") == 0);
}
