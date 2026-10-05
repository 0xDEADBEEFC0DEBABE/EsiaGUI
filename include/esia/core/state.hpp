// Esia - per-id state of the UI core (Context::State<T>, docs/UI_CORE.md section 10).
//
// Immediate-mode widgets keep what must survive a frame (springs, an editor's text and caret, scroll physics, a
// layout provider's measurements) keyed by their Id. An entry is created default-constructed on first use, kept
// while it is used and destroyed after it went unused for a number of frames. The same id may hold one entry per
// type.
#pragma once
#include "esia/base/config.hpp"
#include "esia/base/hash.hpp"
#include <cstddef>
#include <memory>
#include <vector>

namespace esia
{
    class ESIA_API StateStorage
    {
    public:
        StateStorage() = default;
        StateStorage(const StateStorage&) = delete;
        StateStorage& operator=(const StateStorage&) = delete;
        ~StateStorage() { Clear(); }

        // Every widget looks its springs and state up here every frame: one probe of a flat table (the entries
        // themselves live apart and never move, so a reference stays valid while the entry exists).
        template <class T>
        T& Get(Id id, std::uint64_t frame)
        {
            const void* type = &Tag<T>::tag;
            if (!slots_.empty())
            {
                const std::size_t mask = slots_.size() - 1;
                for (std::size_t i = Home(id, type) & mask;; i = (i + 1) & mask)
                {
                    Slot& s = slots_[i];
                    if (s.type == type && s.id == id)
                    {
                        s.lastFrame = frame;
                        return *static_cast<T*>(s.data);
                    }
                    if (!s.type)
                        break;
                }
            }
            // constructed before it is stored: a constructor that throws leaves nothing behind
            auto data = std::make_unique<T>();
            Slot& s = Insert(id, type);
            s.destroy = [](void* p) { delete static_cast<T*>(p); };
            s.data = data.release();
            s.lastFrame = frame;
            return *static_cast<T*>(s.data);
        }

        // Destroys the entries not used since `frame - retain`.
        void Collect(std::uint64_t frame, std::uint64_t retain);
        void Clear();
        std::size_t Size() const { return count_; }

    private:
        // one address per type: identifies the type of an entry without RTTI. Writable on purpose: identical
        // read-only constants may be folded into one address by the linker (/OPT:ICF), mutable data is not.
        template <class T>
        struct Tag
        {
            static inline char tag = 0;
        };
        struct Slot
        {
            Id id = 0;
            const void* type = nullptr;   // null: a free slot
            void* data = nullptr;
            void (*destroy)(void*) = nullptr;
            std::uint64_t lastFrame = 0;
        };
        static std::size_t Home(Id id, const void* type) { return IntHash()(((std::uint64_t)reinterpret_cast<std::uintptr_t>(type) << 7) ^ id); }
        Slot& Insert(Id id, const void* type);   // a free slot for a new entry (grows the table)
        void Rehash(std::size_t size);
        void Erase(std::size_t i);

        // open addressing with linear probing: a power of two in size, at most 3/4 full
        std::vector<Slot> slots_;
        std::size_t count_ = 0;
    };
}
