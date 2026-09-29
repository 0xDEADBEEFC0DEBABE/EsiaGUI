// WGT UI - ABI-stable type-erased callable
//
// wgt::Callback<R(Args...)> is the only way callables cross the wgt.dll boundary. It is three raw
// pointers (invoke, destroy, state): no std::function, no CRT-heap ownership transfer. The closure is
// allocated *and* freed by code instantiated in the caller's module, so Debug hosts can safely talk
// to a Release wgt.dll and plugins built with other toolset versions keep working.
#pragma once
#include "config.hpp"
#include <new>
#include <type_traits>
#include <utility>

namespace wgt
{
    template <class Sig>
    class Callback;

    template <class R, class... A>
    class Callback<R(A...)>
    {
    public:
        using InvokeFn = R (*)(void* state, A... args);
        using DestroyFn = void (*)(void* state);

        Callback() = default;
        Callback(std::nullptr_t) {}

        // From any callable (lambda, functor, function pointer).
        template <class F, class D = std::decay_t<F>,
                  class = std::enable_if_t<!std::is_same_v<D, Callback> && std::is_invocable_r_v<R, D&, A...>>>
        Callback(F&& f)
        {
            if constexpr (std::is_pointer_v<D>)
            {
                if (f == nullptr)
                    return;
            }
            state_ = new D(std::forward<F>(f));
            invoke_ = [](void* s, A... args) -> R { return (*static_cast<D*>(s))(std::forward<A>(args)...); };
            destroy_ = [](void* s) { delete static_cast<D*>(s); };
        }

        // From a C-style function pointer + user data (not owned).
        static Callback FromRaw(InvokeFn fn, void* user, DestroyFn destroy = nullptr)
        {
            Callback c;
            c.invoke_ = fn;
            c.state_ = user;
            c.destroy_ = destroy;
            return c;
        }

        Callback(const Callback&) = delete;
        Callback& operator=(const Callback&) = delete;
        Callback(Callback&& o) noexcept : invoke_(o.invoke_), destroy_(o.destroy_), state_(o.state_)
        {
            o.invoke_ = nullptr;
            o.destroy_ = nullptr;
            o.state_ = nullptr;
        }
        Callback& operator=(Callback&& o) noexcept
        {
            if (this != &o)
            {
                Reset();
                invoke_ = o.invoke_;
                destroy_ = o.destroy_;
                state_ = o.state_;
                o.invoke_ = nullptr;
                o.destroy_ = nullptr;
                o.state_ = nullptr;
            }
            return *this;
        }
        ~Callback() { Reset(); }

        void Reset()
        {
            if (destroy_)
                destroy_(state_);
            invoke_ = nullptr;
            destroy_ = nullptr;
            state_ = nullptr;
        }

        explicit operator bool() const { return invoke_ != nullptr; }
        R operator()(A... args) const { return invoke_(state_, std::forward<A>(args)...); }

    private:
        InvokeFn invoke_ = nullptr;
        DestroyFn destroy_ = nullptr;
        void* state_ = nullptr;
    };

    using Task = Callback<void()>;
}
