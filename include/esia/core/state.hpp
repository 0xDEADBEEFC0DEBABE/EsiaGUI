// Esia - per-id state of the UI core (Context::State<T>, docs/UI_CORE.md section 10).
//
// Immediate-mode widgets keep what must survive a frame (springs, an editor's text and caret, scroll physics, a
// layout provider's measurements) keyed by their Id. An entry is created default-constructed on first use, kept
// while it is used and destroyed after it went unused for a number of frames. The same id may hold one entry per
// type.
#pragma once
#include "esia/base/config.hpp"
#include <cstddef>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>

namespace esia
{
    class ESIA_API StateStorage
    {
    public:
        StateStorage() = default;
        StateStorage(const StateStorage&) = delete;
        StateStorage& operator=(const StateStorage&) = delete;
        ~StateStorage() { Clear(); }

        template <class T>
        T& Get(Id id, std::uint64_t frame)
        {
            const Key key{id, &Tag<T>::tag};
            auto it = slots_.find(key);
            if (it == slots_.end())
            {
                // constructed before it is stored: a constructor that throws leaves nothing behind
                auto data = std::make_unique<T>();
                Slot slot;
                slot.destroy = [](void* p) { delete static_cast<T*>(p); };
                slot.data = data.get();
                it = slots_.emplace(key, slot).first;
                data.release();
            }
            it->second.lastFrame = frame;
            return *static_cast<T*>(it->second.data);
        }

        // Destroys the entries not used since `frame - retain`.
        void Collect(std::uint64_t frame, std::uint64_t retain);
        void Clear();
        std::size_t Size() const { return slots_.size(); }

    private:
        // one address per type: identifies the type of an entry without RTTI. Writable on purpose: identical
        // read-only constants may be folded into one address by the linker (/OPT:ICF), mutable data is not.
        template <class T>
        struct Tag
        {
            static inline char tag = 0;
        };
        struct Key
        {
            Id id;
            const void* type;
            bool operator==(const Key&) const = default;
        };
        struct KeyHash
        {
            std::size_t operator()(const Key& k) const { return std::hash<const void*>()(k.type) ^ ((std::size_t)k.id * 0x9E3779B97F4A7C15ull); }
        };
        struct Slot
        {
            void* data = nullptr;
            void (*destroy)(void*) = nullptr;
            std::uint64_t lastFrame = 0;
        };
        std::unordered_map<Key, Slot, KeyHash> slots_;
    };
}
