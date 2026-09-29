// UTF-8 decoding: every well-formed length, and every kind of malformed input decodes as U+FFFD while advancing.
#include "esia/base/utf8.hpp"
#include "esia_test.hpp"
#include <utility>

using namespace esia;

namespace
{
    constexpr char32_t First(std::string_view s)
    {
        std::size_t i = 0;
        return DecodeUtf8(s, i);
    }
    static_assert(First("A") == U'A' && First("\xC3\xA9") == 0xE9, "usable in constant expressions");

    // (code point, bytes consumed) of the first code point
    std::pair<char32_t, std::size_t> Decode(std::string_view s)
    {
        std::size_t i = 0;
        const char32_t c = DecodeUtf8(s, i);
        return {c, i};
    }
}

ESIA_TEST(Utf8, WellFormedSequences)
{
    ESIA_CHECK(Decode("A") == std::make_pair(char32_t(U'A'), std::size_t(1)));
    ESIA_CHECK(Decode("\xC3\xA9") == std::make_pair(char32_t(0xE9), std::size_t(2)));             // é
    ESIA_CHECK(Decode("\xE2\x82\xAC") == std::make_pair(char32_t(0x20AC), std::size_t(3)));       // €
    ESIA_CHECK(Decode("\xF0\x9F\x98\x80") == std::make_pair(char32_t(0x1F600), std::size_t(4)));  // emoji
    ESIA_CHECK(Decode("\xF4\x8F\xBF\xBF") == std::make_pair(char32_t(0x10FFFF), std::size_t(4))); // the last code point

    // a whole string, code point by code point
    const std::string_view s = "a\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80z";
    const char32_t want[] = {U'a', 0xE9, 0x20AC, 0x1F600, U'z'};
    std::size_t i = 0, n = 0;
    while (i < s.size() && n < 5)
        ESIA_CHECK(DecodeUtf8(s, i) == want[n++]);
    ESIA_CHECK(n == 5 && i == s.size());
}

ESIA_TEST(Utf8, MalformedInputDecodesAsReplacementAndAdvances)
{
    // stray continuation byte, invalid lead bytes: one byte
    ESIA_CHECK(Decode("\x80z") == std::make_pair(kReplacementCharacter, std::size_t(1)));
    ESIA_CHECK(Decode("\xFFz") == std::make_pair(kReplacementCharacter, std::size_t(1)));
    // truncated at the end, or broken off by a non-continuation byte: the lead byte only (resynchronize)
    ESIA_CHECK(Decode("\xE2\x82") == std::make_pair(kReplacementCharacter, std::size_t(1)));
    ESIA_CHECK(Decode("\xE2z\xAC") == std::make_pair(kReplacementCharacter, std::size_t(1)));
    // complete but invalid: overlong, surrogate, beyond U+10FFFF - the whole sequence
    ESIA_CHECK(Decode("\xC0\xAF") == std::make_pair(kReplacementCharacter, std::size_t(2)));
    ESIA_CHECK(Decode("\xE0\x80\xAF") == std::make_pair(kReplacementCharacter, std::size_t(3)));
    ESIA_CHECK(Decode("\xED\xA0\x80") == std::make_pair(kReplacementCharacter, std::size_t(3)));
    ESIA_CHECK(Decode("\xF4\x90\x80\x80") == std::make_pair(kReplacementCharacter, std::size_t(4)));

    // decoding garbage always terminates and yields one code point per step
    const std::string_view junk = "\xF0\x9F\x98\xE2\x82\xC3";
    std::size_t i = 0, steps = 0;
    while (i < junk.size() && steps < 100)
    {
        DecodeUtf8(junk, i);
        ++steps;
    }
    ESIA_CHECK(i == junk.size() && steps <= junk.size());
}
