// Esia - Metal backend: frame bookkeeping without Apple headers - which device frames the GPU has finished, and
// the resources that must outlive the frames that used them (versioned buffers, deferred releases, staging).
//
// The Metal backend never blocks a frame on the GPU: a host may record several device frames into one command
// buffer before it commits (docs/backends/README.md: "count device frames for rings"), so waiting for a frame in
// BeginFrame could wait for a command buffer that is not committed yet. Instead every resource the CPU rewrites
// is versioned and a version is reused only once the frames that read it completed.
//
// Plain C++, unit-tested on every host (tests/test_metal_frames.cpp). UNVERIFIED: needs macOS for the real
// completion handlers that feed FrameTracker.
#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <set>
#include <vector>

namespace esia::rhi::metal
{
    // Device frames are numbered 1, 2, ... in BeginFrame order. Command buffers complete (addCompletedHandler) on a
    // Metal thread, possibly out of order when a host uses several queues; CompletedPrefix() is the newest serial
    // up to which every frame completed - what "the GPU is done with it" means for a resource last used then.
    class FrameTracker
    {
    public:
        std::uint64_t Begin();                    // render thread
        void Complete(std::uint64_t serial);      // any thread
        std::uint64_t CompletedPrefix() const;    // any thread
        std::uint64_t LastBegun() const;

    private:
        mutable std::mutex mutex_;
        std::uint64_t begun_ = 0, prefix_ = 0;
        std::set<std::uint64_t> early_;   // completed, but an older frame has not
    };

    // Several copies of a CPU-written GPU object (a buffer the renderer updates every frame). Acquire returns the
    // version to write: the current one when no frame in flight reads it, else a free older one, else a new one.
    template <class T>
    class VersionRing
    {
    public:
        struct Version
        {
            T object{};
            std::uint64_t lastUse = 0;   // serial of the last frame that read it (0 = never)
        };

        // `make` creates a new version's object; returns the index of the version to write, now current.
        int Acquire(std::uint64_t completed, const std::function<T()>& make)
        {
            if (current_ >= 0 && versions_[(std::size_t)current_].lastUse <= completed)
                return current_;
            for (std::size_t i = 0; i < versions_.size(); ++i)
                if ((int)i != current_ && versions_[i].lastUse <= completed)
                    return current_ = (int)i;
            versions_.push_back({make(), 0});
            return current_ = (int)versions_.size() - 1;
        }
        void MarkUsed(std::uint64_t serial)
        {
            if (current_ >= 0)
                versions_[(std::size_t)current_].lastUse = serial;
        }
        int CurrentIndex() const { return current_; }
        bool HasCurrent() const { return current_ >= 0; }
        const Version& Current() const { return versions_[(std::size_t)current_]; }
        const Version& At(int i) const { return versions_[(std::size_t)i]; }
        const std::vector<Version>& All() const { return versions_; }

    private:
        std::vector<Version> versions_;
        int current_ = -1;
    };

    // Objects destroyed while frames in flight may still read them: released once those frames completed.
    template <class T>
    class DeferredReleases
    {
    public:
        void Push(T object, std::uint64_t lastSerial) { items_.push_back({object, lastSerial}); }
        // Calls `release` for every object whose frames completed; `all` releases everything (device teardown).
        void Collect(std::uint64_t completed, const std::function<void(T)>& release, bool all = false)
        {
            std::size_t kept = 0;
            for (std::size_t i = 0; i < items_.size(); ++i)
            {
                if (all || items_[i].serial <= completed)
                    release(items_[i].object);
                else
                    items_[kept++] = items_[i];
            }
            items_.resize(kept);
        }
        std::size_t Size() const { return items_.size(); }

    private:
        struct Item
        {
            T object;
            std::uint64_t serial;
        };
        std::vector<Item> items_;
    };

    // Bump allocator over CPU-visible staging buffers (texture uploads): a chunk used by frame S is rewritten only
    // after S completed. Chunks are created through callbacks, so the same code runs on Metal and on the tests'
    // fake GPU.
    class StagingArena
    {
    public:
        struct Allocation
        {
            std::uint32_t buffer = 0;   // Gpu buffer id
            std::size_t offset = 0;
            void* cpu = nullptr;        // contents + offset
        };
        struct Callbacks
        {
            std::function<std::uint32_t(std::size_t bytes)> create;   // 0 on failure
            std::function<void*(std::uint32_t buffer)> contents;
            std::function<void(std::uint32_t buffer)> release;
        };
        static constexpr std::size_t kMinChunk = 1u << 20;

        explicit StagingArena(Callbacks cb) : cb_(std::move(cb)) {}
        ~StagingArena() { ReleaseAll(); }
        StagingArena(const StagingArena&) = delete;
        StagingArena& operator=(const StagingArena&) = delete;

        // `serial` is the frame whose command buffer will read the allocation.
        bool Allocate(std::size_t bytes, std::size_t alignment, std::uint64_t serial, std::uint64_t completed, Allocation& out);
        // Releases chunks no frame needs any more beyond `keep` idle ones (after a burst of uploads).
        void Trim(std::uint64_t completed, std::size_t keep);
        void ReleaseAll();
        std::size_t ChunkCount() const { return chunks_.size(); }

    private:
        struct Chunk
        {
            std::uint32_t buffer = 0;
            std::size_t size = 0, used = 0;
            std::uint64_t serial = 0;   // last frame that allocated from it
        };
        Callbacks cb_;
        std::vector<Chunk> chunks_;
        int open_ = -1;                 // the chunk the current frame allocates from
    };
}
