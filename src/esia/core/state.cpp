// Esia - per-id state storage (see esia/core/state.hpp)
#include "esia/core/state.hpp"

namespace esia
{
    void StateStorage::Collect(std::uint64_t frame, std::uint64_t retain)
    {
        for (auto it = slots_.begin(); it != slots_.end();)
        {
            if (frame - it->second.lastFrame > retain)
            {
                it->second.destroy(it->second.data);
                it = slots_.erase(it);
            }
            else
                ++it;
        }
    }

    void StateStorage::Clear()
    {
        for (auto& [key, slot] : slots_)
            slot.destroy(slot.data);
        slots_.clear();
    }
}
