// WGT UI - typed per-widget state keyed by ImGuiID, with automatic garbage collection.
#pragma once
#include "imgui.h"
#include <cstdint>
#include <unordered_map>

namespace wgt
{
    class StateStore
    {
    public:
        StateStore() = default;
        StateStore(const StateStore&) = delete;
        StateStore& operator=(const StateStore&) = delete;
        ~StateStore() { Clear(); }

        void BeginFrame(std::uint64_t frame) { frame_ = frame; }

        // Returns the state of type T for `id`, default-constructing it on first use.
        template <class T>
        T& Get(ImGuiID id, bool* created = nullptr)
        {
            Key key{id, TypeKey<T>()};
            auto it = map_.find(key);
            if (it == map_.end())
            {
                Slot slot;
                slot.ptr = new T();
                slot.destroy = [](void* p) { delete static_cast<T*>(p); };
                it = map_.emplace(key, slot).first;
                if (created)
                    *created = true;
            }
            else if (created)
                *created = false;
            it->second.lastFrame = frame_;
            return *static_cast<T*>(it->second.ptr);
        }

        template <class T>
        T* Find(ImGuiID id)
        {
            auto it = map_.find(Key{id, TypeKey<T>()});
            if (it == map_.end())
                return nullptr;
            it->second.lastFrame = frame_;
            return static_cast<T*>(it->second.ptr);
        }

        // Frees states untouched for more than `maxAge` frames.
        void Collect(std::uint64_t maxAge)
        {
            for (auto it = map_.begin(); it != map_.end();)
            {
                if (frame_ - it->second.lastFrame > maxAge)
                {
                    it->second.destroy(it->second.ptr);
                    it = map_.erase(it);
                }
                else
                    ++it;
            }
        }

        void Clear()
        {
            for (auto& kv : map_)
                kv.second.destroy(kv.second.ptr);
            map_.clear();
        }

        std::size_t Size() const { return map_.size(); }

    private:
        template <class T>
        static const void* TypeKey()
        {
            static const char tag = 0;
            return &tag;
        }

        struct Key
        {
            ImGuiID id;
            const void* type;
            bool operator==(const Key& o) const { return id == o.id && type == o.type; }
        };
        struct KeyHash
        {
            std::size_t operator()(const Key& k) const
            {
                std::size_t h = (std::size_t)k.id * 0x9E3779B97F4A7C15ull;
                return h ^ ((std::size_t)k.type >> 4);
            }
        };
        struct Slot
        {
            void* ptr = nullptr;
            void (*destroy)(void*) = nullptr;
            std::uint64_t lastFrame = 0;
        };

        std::unordered_map<Key, Slot, KeyHash> map_;
        std::uint64_t frame_ = 0;
    };
}
