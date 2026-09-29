// Esia - Metal backend: frame bookkeeping (see metal_frames.hpp). UNVERIFIED: needs macOS.
#include "metal_frames.hpp"
#include "metal_planning.hpp"
#include <algorithm>

namespace esia::rhi::metal
{
    std::uint64_t FrameTracker::Begin()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return ++begun_;
    }

    void FrameTracker::Complete(std::uint64_t serial)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (serial <= prefix_)
            return;
        early_.insert(serial);
        while (!early_.empty() && *early_.begin() == prefix_ + 1)
        {
            ++prefix_;
            early_.erase(early_.begin());
        }
    }

    std::uint64_t FrameTracker::CompletedPrefix() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return prefix_;
    }

    std::uint64_t FrameTracker::LastBegun() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return begun_;
    }

    bool StagingArena::Allocate(std::size_t bytes, std::size_t alignment, std::uint64_t serial, std::uint64_t completed, Allocation& out)
    {
        auto fits = [&](const Chunk& c) { return AlignUp(c.used, alignment) + bytes <= c.size; };
        if (open_ >= 0 && (chunks_[(std::size_t)open_].serial != serial || !fits(chunks_[(std::size_t)open_])))
            open_ = -1;
        if (open_ < 0)
        {
            for (std::size_t i = 0; i < chunks_.size(); ++i)
            {
                Chunk& c = chunks_[i];
                if (c.serial <= completed && c.size >= bytes)
                {
                    c.used = 0;
                    open_ = (int)i;
                    break;
                }
            }
        }
        if (open_ < 0)
        {
            Chunk c;
            c.size = std::max(kMinChunk, AlignUp(bytes, kMinChunk));
            c.buffer = cb_.create(c.size);
            if (!c.buffer)
                return false;
            chunks_.push_back(c);
            open_ = (int)chunks_.size() - 1;
        }
        Chunk& c = chunks_[(std::size_t)open_];
        c.serial = serial;
        out.buffer = c.buffer;
        out.offset = AlignUp(c.used, alignment);
        out.cpu = static_cast<std::uint8_t*>(cb_.contents(c.buffer)) + out.offset;
        c.used = out.offset + bytes;
        return true;
    }

    void StagingArena::Trim(std::uint64_t completed, std::size_t keep)
    {
        std::size_t idle = 0;
        for (std::size_t i = chunks_.size(); i-- > 0;)
        {
            if ((int)i == open_ || chunks_[i].serial > completed || idle++ < keep)
                continue;
            cb_.release(chunks_[i].buffer);
            chunks_.erase(chunks_.begin() + (std::ptrdiff_t)i);
            if (open_ > (int)i)
                --open_;
        }
    }

    void StagingArena::ReleaseAll()
    {
        for (const Chunk& c : chunks_)
            cb_.release(c.buffer);
        chunks_.clear();
        open_ = -1;
    }
}
