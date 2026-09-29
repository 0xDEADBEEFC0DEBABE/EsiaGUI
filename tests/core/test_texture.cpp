#include "esia/core/texture.hpp"
#include "esia_test.hpp"
#include <thread>

using namespace esia;

ESIA_TEST(Textures, CreateUpdateCollect)
{
    TextureRegistry reg;
    const std::uint8_t px[4 * 4] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    const TextureId id = reg.Create({TextureFormat::Alpha8, 4, 4}, px);
    ESIA_CHECK(id != 0);
    const std::uint8_t sub[2] = {99, 98};
    ESIA_CHECK(reg.Update(id, 1, 2, 2, 1, sub));
    ESIA_CHECK(!reg.Update(id, 3, 3, 2, 1, sub));   // outside
    ESIA_CHECK(!reg.Update(12345, 0, 0, 1, 1, sub));
    std::vector<TextureChange> ch;
    reg.TakeChanges(ch);
    ESIA_CHECK(ch.size() == 2);
    ESIA_CHECK(ch[0].kind == TextureChange::Kind::Create && ch[0].pixels.size() == 16 && ch[0].pixels[15] == 16);
    ESIA_CHECK(ch[1].kind == TextureChange::Kind::Update && ch[1].x == 1 && ch[1].y == 2 && ch[1].pixels[1] == 98);
    reg.TakeChanges(ch);
    ESIA_CHECK(ch.empty());
}

ESIA_TEST(Textures, RowPitchAndZeroInit)
{
    TextureRegistry reg;
    const std::uint32_t rows[2 * 3] = {1, 2, 0xDEAD, 3, 4, 0xDEAD};   // 2 pixels per row, pitch 3 pixels
    const TextureId id = reg.Create({TextureFormat::RGBA8, 2, 2}, rows, 12);
    const TextureId z = reg.Create({TextureFormat::RGBA8, 2, 2});
    std::vector<TextureChange> ch;
    reg.TakeChanges(ch);
    ESIA_CHECK(ch.size() == 2 && ch[0].id == id && ch[1].id == z);
    ESIA_CHECK(ch[0].pixels.size() == 16 && ch[0].pixels[8] == 3);
    bool allZero = true;
    for (auto b : ch[1].pixels)
        allZero = allZero && b == 0;
    ESIA_CHECK(allZero);
}

ESIA_TEST(Textures, DestroyBeforeCollectDropsEverything)
{
    TextureRegistry reg;
    const TextureId id = reg.Create({TextureFormat::RGBA8, 1, 1});
    reg.Destroy(id);
    std::vector<TextureChange> ch;
    reg.TakeChanges(ch);
    ESIA_CHECK(ch.empty());
    ESIA_CHECK(reg.Count() == 0);
}

ESIA_TEST(Textures, DestroyAfterCollectIsQueued)
{
    TextureRegistry reg;
    const TextureId id = reg.Create({TextureFormat::RGBA8, 1, 1});
    std::vector<TextureChange> ch;
    reg.TakeChanges(ch);
    reg.Update(id, 0, 0, 1, 1, "abcd");
    reg.Destroy(id);
    reg.TakeChanges(ch);
    ESIA_CHECK(ch.size() == 1 && ch[0].kind == TextureChange::Kind::Destroy);
    TextureInfo info;
    ESIA_CHECK(!reg.Info(id, info));
}

ESIA_TEST(Textures, ThreadSafeCreation)
{
    TextureRegistry reg;
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t)
        threads.emplace_back([&] {
            for (int i = 0; i < 100; ++i)
                reg.Create({TextureFormat::Alpha8, 2, 2});
        });
    for (auto& t : threads)
        t.join();
    std::vector<TextureChange> ch;
    reg.TakeChanges(ch);
    ESIA_CHECK(ch.size() == 400);
    ESIA_CHECK(reg.Count() == 400);
}
