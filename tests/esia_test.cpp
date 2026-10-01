// Esia - minimal test runner (see esia_test.hpp)
#include "esia_test.hpp"
#include <cstring>
#include <string>
#include <vector>

namespace esia::test
{
    namespace
    {
        struct Entry
        {
            const char* suite;
            const char* name;
            TestFn fn;
        };
        std::vector<Entry>& Registry()
        {
            static std::vector<Entry> r;
            return r;
        }
        int gFailures = 0;
        int gCurrentFailures = 0;
    }

    Registrar::Registrar(const char* suite, const char* name, TestFn fn) { Registry().push_back({suite, name, fn}); }

    void Fail(const char* file, int line, const char* expr)
    {
        ++gFailures;
        ++gCurrentFailures;
        std::fprintf(stderr, "  %s:%d: check failed: %s\n", file, line, expr);
    }

    int Failures() { return gFailures; }
}

int main(int argc, char** argv)
{
    using namespace esia::test;
    const char* filter = argc > 1 ? argv[1] : nullptr;
    int run = 0, failed = 0;
    for (const Entry& e : Registry())
    {
        const std::string full = std::string(e.suite) + "." + e.name;
        if (filter && full.find(filter) == std::string::npos)
            continue;
        gCurrentFailures = 0;
        e.fn();
        ++run;
        if (gCurrentFailures)
        {
            ++failed;
            std::fprintf(stderr, "[FAIL] %s\n", full.c_str());
        }
        else
            std::printf("[ ok ] %s\n", full.c_str());
    }
    std::printf("%d test(s), %d failed\n", run, failed);
    return failed == 0 && run > 0 ? 0 : 1;
}
