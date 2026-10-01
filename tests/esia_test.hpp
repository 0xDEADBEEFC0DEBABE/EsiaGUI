// Esia - a minimal test framework (no third-party dependency: the tests must build on every host the backends
// run on, offline).
//
//   ESIA_TEST(Suite, Name) { ESIA_CHECK(a == b); ESIA_CHECK_NEAR(x, 1.0f, 1e-5f); }
//
// Every executable runs all its tests (or those whose "Suite.Name" contains argv[1]) and returns non-zero when
// a check failed.
#pragma once
#include <cmath>
#include <cstdio>

namespace esia::test
{
    using TestFn = void (*)();
    struct Registrar
    {
        Registrar(const char* suite, const char* name, TestFn fn);
    };
    void Fail(const char* file, int line, const char* expr);
    int Failures();
}

#define ESIA_TEST(suite, name)                                                                           \
    static void EsiaTest_##suite##_##name();                                                             \
    static const ::esia::test::Registrar kEsiaReg_##suite##_##name(#suite, #name, &EsiaTest_##suite##_##name); \
    static void EsiaTest_##suite##_##name()

#define ESIA_CHECK(expr)                                                  \
    do                                                                    \
    {                                                                     \
        if (!(expr))                                                      \
            ::esia::test::Fail(__FILE__, __LINE__, #expr);                \
    } while (0)

#define ESIA_CHECK_NEAR(a, b, eps) ESIA_CHECK(std::fabs((double)(a) - (double)(b)) <= (double)(eps))
