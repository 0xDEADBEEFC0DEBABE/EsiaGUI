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
            Slot& s = slots_[Key{id, &Tag<T>::tag}];
            if (!s.data)
            {
                s.data = new T();
                s.destroy = [](void* p) { delete static_cast<T*>(p); };
            }
            s.lastFrame = frame;
            return *static_cast<T*>(s.data);
        }

        // Destroys the entries not used since `frame - retain`.
        void Collect(std::uint64_t frame, std::uint64_t retain);
        void Clear();
        std::size_t Size() const { return slots_.size(); }

    private:
        // one address per type: identifies the type of an entry without RTTI
        template <class T>
        struct Tag
        {
            static constexpr char tag = 0;
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
