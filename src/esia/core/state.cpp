// Esia - per-id state storage (see esia/core/state.hpp)
#include "esia/core/state.hpp"

namespace esia
{
    StateStorage::Slot& StateStorage::Insert(Id id, const void* type)
    {
        if ((count_ + 1) * 4 > slots_.size() * 3)
            Rehash(slots_.empty() ? 64 : slots_.size() * 2);
        const std::size_t mask = slots_.size() - 1;
        std::size_t i = Home(id, type) & mask;
        while (slots_[i].type)
            i = (i + 1) & mask;
        Slot& s = slots_[i];
        s.id = id;
        s.type = type;
        ++count_;
        return s;
    }

    void StateStorage::Rehash(std::size_t size)
    {
        std::vector<Slot> old(size);
        old.swap(slots_);
        const std::size_t mask = size - 1;
        for (const Slot& s : old)
            if (s.type)
            {
                std::size_t i = Home(s.id, s.type) & mask;
                while (slots_[i].type)
                    i = (i + 1) & mask;
                slots_[i] = s;
            }
    }

    // Backward-shift deletion: the entries after the freed slot that probed past it move up, so every probe chain
    // stays unbroken (no tombstones).
    void StateStorage::Erase(std::size_t i)
    {
        const std::size_t mask = slots_.size() - 1;
        for (std::size_t j = (i + 1) & mask; slots_[j].type; j = (j + 1) & mask)
        {
            const std::size_t home = Home(slots_[j].id, slots_[j].type) & mask;
            // slot j may move to i unless its home lies cyclically in (i, j]
            if (((j - home) & mask) >= ((j - i) & mask))
            {
                slots_[i] = slots_[j];
                i = j;
            }
        }
        slots_[i] = Slot{};
        --count_;
    }

    void StateStorage::Collect(std::uint64_t frame, std::uint64_t retain)
    {
        // an erase may move a later entry (not yet looked at) into slot i: look at it again
        for (std::size_t i = 0; i < slots_.size(); ++i)
            while (slots_[i].type && frame - slots_[i].lastFrame > retain)
            {
                slots_[i].destroy(slots_[i].data);
                Erase(i);
            }
    }

    void StateStorage::Clear()
    {
        for (Slot& s : slots_)
            if (s.type)
                s.destroy(s.data);
        slots_.clear();
        count_ = 0;
    }
}
