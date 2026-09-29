// WGT UI - thread-safe data plumbing between game/worker threads and the UI thread.
//
//   Property<T>  observable value, readable/writable from any thread, with change versioning.
//                Lock-free for small trivially-copyable types (bool, int, float, enums, Vec2 ...).
//   Channel<T>   multi-producer / single-consumer queue (log lines, events, commands).
//   Latest<T>    lock-free triple buffer: producer publishes snapshots, UI always reads the newest.
//
// All of these are header-only and never cross the DLL boundary by reference, so they are safe to use
// in any module regardless of CRT/STL configuration.
#pragma once
#include <atomic>
#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <type_traits>
#include <utility>
#include <vector>

namespace wgt
{
    namespace detail
    {
        template <class T, bool = std::is_trivially_copyable_v<T> && (sizeof(T) <= 8)>
        struct AtomicFriendly : std::false_type {};
        template <class T>
        struct AtomicFriendly<T, true> : std::bool_constant<std::atomic<T>::is_always_lock_free> {};
        template <class T>
        constexpr bool kAtomicFriendly = AtomicFriendly<T>::value;
    }

    template <class T>
    class Property
    {
    public:
        Property() = default;
        explicit Property(T initial) { StoreImpl(std::move(initial)); }
        Property(const Property&) = delete;
        Property& operator=(const Property&) = delete;

        T Get() const
        {
            if constexpr (detail::kAtomicFriendly<T>)
                return atomic_.load(std::memory_order_acquire);
            else
            {
                std::shared_lock lock(mutex_);
                return value_;
            }
        }

        void Set(T v)
        {
            StoreImpl(std::move(v));
            version_.fetch_add(1, std::memory_order_acq_rel);
        }

        // Atomically read-modify-write: prop.Update([](T& v){ v += 1; });
        template <class F>
        void Update(F&& fn)
        {
            if constexpr (detail::kAtomicFriendly<T>)
            {
                T expected = atomic_.load(std::memory_order_relaxed);
                T desired;
                do
                {
                    desired = expected;
                    fn(desired);
                } while (!atomic_.compare_exchange_weak(expected, desired, std::memory_order_acq_rel));
            }
            else
            {
                std::unique_lock lock(mutex_);
                fn(value_);
            }
            version_.fetch_add(1, std::memory_order_acq_rel);
        }

        std::uint64_t Version() const { return version_.load(std::memory_order_acquire); }

        // Returns true once per change observed through `seen` (per-observer cursor).
        bool Changed(std::uint64_t& seen) const
        {
            std::uint64_t v = Version();
            if (v == seen)
                return false;
            seen = v;
            return true;
        }

        operator T() const { return Get(); }
        Property& operator=(T v)
        {
            Set(std::move(v));
            return *this;
        }

    private:
        void StoreImpl(T v)
        {
            if constexpr (detail::kAtomicFriendly<T>)
                atomic_.store(v, std::memory_order_release);
            else
            {
                std::unique_lock lock(mutex_);
                value_ = std::move(v);
            }
        }

        struct Empty {};
        using AtomicStorage = std::conditional_t<detail::kAtomicFriendly<T>, std::atomic<T>, Empty>;
        using LockedStorage = std::conditional_t<detail::kAtomicFriendly<T>, Empty, T>;
        using MutexStorage = std::conditional_t<detail::kAtomicFriendly<T>, Empty, std::shared_mutex>;

        [[no_unique_address]] AtomicStorage atomic_{};
        [[no_unique_address]] LockedStorage value_{};
        [[no_unique_address]] mutable MutexStorage mutex_{};
        std::atomic<std::uint64_t> version_{0};
    };

    template <class T>
    class Channel
    {
    public:
        void Push(T v)
        {
            std::lock_guard lock(mutex_);
            items_.push_back(std::move(v));
        }

        template <class... Args>
        void Emplace(Args&&... args)
        {
            std::lock_guard lock(mutex_);
            items_.emplace_back(std::forward<Args>(args)...);
        }

        // Consumer side: hands every pending item to fn, in push order. Returns the count.
        template <class F>
        std::size_t Drain(F&& fn)
        {
            {
                std::lock_guard lock(mutex_);
                scratch_.swap(items_);
            }
            std::size_t n = scratch_.size();
            for (auto& it : scratch_)
                fn(std::move(it));
            scratch_.clear();
            return n;
        }

        bool Empty() const
        {
            std::lock_guard lock(mutex_);
            return items_.empty();
        }

    private:
        mutable std::mutex mutex_;
        std::vector<T> items_;
        std::vector<T> scratch_;
    };

    template <class T>
    class Latest
    {
    public:
        // Producer thread.
        void Publish(const T& v)
        {
            buffers_[write_] = v;
            std::uint8_t prev = middle_.exchange(static_cast<std::uint8_t>(write_ | kDirty), std::memory_order_acq_rel);
            write_ = prev & kIndexMask;
        }

        // Consumer thread. Returns true when a newer value than the previous Fetch was published.
        bool Fetch(T& out)
        {
            bool fresh = false;
            if (middle_.load(std::memory_order_relaxed) & kDirty)
            {
                std::uint8_t prev = middle_.exchange(read_, std::memory_order_acq_rel);
                read_ = prev & kIndexMask;
                fresh = true;
            }
            out = buffers_[read_];
            return fresh;
        }

    private:
        static constexpr std::uint8_t kDirty = 4;
        static constexpr std::uint8_t kIndexMask = 3;
        T buffers_[3]{};
        std::atomic<std::uint8_t> middle_{1};
        std::uint8_t write_ = 0;
        std::uint8_t read_ = 2;
    };
}
