// Metal backend: frame completion tracking, buffer versions, deferred releases, the staging arena.
// UNVERIFIED: needs macOS (passes on Linux and under Wine).
#include "esia_test.hpp"
#include "metal_frames.hpp"
#include <thread>
#include <vector>

using namespace esia::rhi::metal;

ESIA_TEST(MetalFrames, CompletedPrefix)
{
    FrameTracker t;
    ESIA_CHECK(t.Begin() == 1 && t.Begin() == 2 && t.Begin() == 3);
    ESIA_CHECK(t.CompletedPrefix() == 0 && t.LastBegun() == 3);
    t.Complete(2);   // another queue finished first: frame 1 may still read what 2 released
    ESIA_CHECK(t.CompletedPrefix() == 0);
    t.Complete(1);
    ESIA_CHECK(t.CompletedPrefix() == 2);
    t.Complete(1);   // duplicates are harmless
    t.Complete(3);
    ESIA_CHECK(t.CompletedPrefix() == 3);

    // completion handlers run on Metal's threads
    FrameTracker m;
    for (int i = 0; i < 64; ++i)
        m.Begin();
    std::vector<std::thread> threads;
    for (int k = 0; k < 4; ++k)
        threads.emplace_back([&m, k] {
            for (std::uint64_t s = (std::uint64_t)k + 1; s <= 64; s += 4)
                m.Complete(s);
        });
    for (std::thread& th : threads)
        th.join();
    ESIA_CHECK(m.CompletedPrefix() == 64);
}

ESIA_TEST(MetalFrames, VersionRing)
{
    VersionRing<int> r;
    int made = 0;
    auto make = [&] { return ++made; };
    ESIA_CHECK(!r.HasCurrent());
    ESIA_CHECK(r.Acquire(0, make) == 0 && made == 1);
    ESIA_CHECK(r.Acquire(0, make) == 0);    // never read by a frame: rewritten in place
    r.MarkUsed(1);                           // frame 1 reads version 0
    ESIA_CHECK(r.Acquire(0, make) == 1 && made == 2);   // frame 1 still in flight
    r.MarkUsed(2);
    ESIA_CHECK(r.Acquire(0, make) == 2 && made == 3);   // frames 1 and 2 in flight
    r.MarkUsed(3);
    ESIA_CHECK(r.Acquire(1, make) == 0 && made == 3);   // frame 1 done: version 0 is free again
    ESIA_CHECK(r.Current().object == 1);
    r.MarkUsed(4);
    ESIA_CHECK(r.Acquire(4, make) == 0);    // everything done: the current version is reused
    ESIA_CHECK(r.All().size() == 3);
}

ESIA_TEST(MetalFrames, DeferredReleases)
{
    DeferredReleases<int> d;
    std::vector<int> released;
    auto rel = [&](int v) { released.push_back(v); };
    d.Push(10, 1);
    d.Push(20, 3);
    d.Push(30, 2);
    d.Collect(0, rel);
    ESIA_CHECK(released.empty() && d.Size() == 3);
    d.Collect(2, rel);
    ESIA_CHECK(released.size() == 2 && released[0] == 10 && released[1] == 30 && d.Size() == 1);
    d.Collect(0, rel, true);
    ESIA_CHECK(released.size() == 3 && released[2] == 20 && d.Size() == 0);
}

ESIA_TEST(MetalFrames, StagingArena)
{
    struct Fake
    {
        std::vector<std::vector<std::uint8_t>> buffers;
        std::vector<bool> live;
    } fake;
    StagingArena::Callbacks cb;
    cb.create = [&](std::size_t bytes) {
        fake.buffers.emplace_back(bytes);
        fake.live.push_back(true);
        return (std::uint32_t)fake.buffers.size();
    };
    cb.contents = [&](std::uint32_t b) { return (void*)fake.buffers[b - 1].data(); };
    cb.release = [&](std::uint32_t b) { fake.live[b - 1] = false; };
    {
        StagingArena a(cb);
        StagingArena::Allocation x, y, z, w;
        ESIA_CHECK(a.Allocate(100, 256, 1, 0, x) && x.buffer == 1 && x.offset == 0);
        ESIA_CHECK(a.Allocate(10, 256, 1, 0, y) && y.buffer == 1 && y.offset == 256);   // aligned bump
        ESIA_CHECK((std::uint8_t*)y.cpu - (std::uint8_t*)x.cpu == 256);
        // frame 2 while frame 1 is in flight: a new chunk
        ESIA_CHECK(a.Allocate(10, 256, 2, 0, z) && z.buffer == 2 && z.offset == 0);
        // frame 3 after frame 1 completed: chunk 1 is recycled from its start
        ESIA_CHECK(a.Allocate(10, 256, 3, 1, w) && w.buffer == 1 && w.offset == 0);
        // larger than a chunk: its own chunk
        StagingArena::Allocation big;
        ESIA_CHECK(a.Allocate(StagingArena::kMinChunk * 2 + 1, 256, 3, 1, big) && big.buffer == 3);
        ESIA_CHECK(fake.buffers[2].size() >= StagingArena::kMinChunk * 2 + 1);
        ESIA_CHECK(a.ChunkCount() == 3);
        a.Trim(3, 0);   // everything done; the open chunk stays
        ESIA_CHECK(a.ChunkCount() == 1 && !fake.live[0] && !fake.live[1] && fake.live[2]);
    }
    ESIA_CHECK(!fake.live[2]);   // the arena releases its chunks
}
