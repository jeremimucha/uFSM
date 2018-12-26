#pragma once

#include <iostream>
#include "traits.hpp"
#include "fsm_state.hpp"
#include "transition_table.hpp"
#include "dispatch_event.hpp"
#include "get.hpp"
#include "logging.hpp"


namespace ufsm
{

namespace back
{

template<typename IndexSequence, typename... States>
class Fsm_impl;

template<size_type Idx, typename FsmT>
inline constexpr decltype(auto) Get(FsmT&& fsm) noexcept;
// Inheriting publicly because clang insists that declaring the `Get` template a friend makes the
// calls ambiguous. Inheriting publicly and not declaring `Get` a friend for now as a workaound.
template<size_type... Indices, typename... States>
class Fsm_impl<Index_sequence<Indices...>, States...> : public FsmState<Indices, States>...
{
public:
    constexpr Fsm_impl() noexcept = default;

    template<typename State>
    constexpr explicit Fsm_impl(initial_state<State>) noexcept
        : state_{state_index_v<typelist<States...>,State>} { }

    template<typename... Ts>
    constexpr Fsm_impl(Ts&&... states) noexcept((... && std::is_nothrow_constructible_v<States, Ts>))
        : FsmState<Indices,States>{std::forward<Ts>(states)}...
        { }

    template<typename T>
    constexpr void set_initial_state(initial_state<T>) noexcept
    {
        state_ = state_index_v<typelist<States...>,T>;
    }

    constexpr inline size_type state() const noexcept { return state_; }
    constexpr inline void state(size_type new_state) noexcept { state_ = new_state; }

    template<size_type Idx, typename State> friend inline constexpr
    FsmState_t<Idx,State>& Get_impl(FsmState<Idx,State>& state) noexcept;
    template<size_type Idx, typename State> friend inline constexpr
    FsmState_t<Idx,State> const& Get_impl(FsmState<Idx,State> const& state) noexcept;
    template<size_type Idx, typename State> friend inline constexpr
    FsmState_t<Idx,State>&& Get_impl(FsmState<Idx,State>&& state) noexcept;
    template<size_type Idx, typename State> friend inline constexpr
    FsmState_t<Idx,State> const&& Get_impl(FsmState<Idx,State> const&& state) noexcept;

    // clang insists that this friend declaration makes the calls to Get ambiguous
    // template<size_type Idx, typename FsmT>
    // friend constexpr decltype(auto) Get(FsmT&& fsm) noexcept;
private:
    size_type state_{};
};

} // namespace back

template<typename Impl, typename Statelist = get_fsm_state_list_t<Impl>> class Fsm;
template<typename Impl, typename... States>
class Fsm<Impl, typelist<States...>>
    : public Impl, public back::Fsm_impl<Make_index_sequence<sizeof...(States)>, States...>
{
    using Indices = Make_index_sequence<sizeof...(States)>;
    using Base = back::Fsm_impl<Indices, States...>;
public:
    // using Statelist = typelist<States...>;
    constexpr Fsm() noexcept = default;

    using Base::Base;
    using Base::set_initial_state;

    template<typename FsmT, typename Event, size_type Idx, size_type... Idxs>
    friend constexpr inline void
    ::ufsm::back::dispatch_event(FsmT&& fsm, Event&& event, Index_sequence<Idx,Idxs...>) noexcept;

    template<typename Event>
    constexpr inline void dispatch_event(Event&& event) noexcept
    {
        ::ufsm::back::dispatch_event(*this, std::forward<Event>(event), Indices{});
    }

    using Impl::transition_table;
    // constexpr decltype(auto) transition_table() const noexcept { return derived().transition_table(); }
    // constexpr decltype(auto) transition_table() noexcept { return derived().transition_table(); }
    constexpr decltype(auto) self() & noexcept { return *this; }
    constexpr decltype(auto) self() const& noexcept { return *this; }
    constexpr decltype(auto) self() && noexcept { return std::move(*this); }
    constexpr decltype(auto) self() const&& noexcept { return std::move(*this); }
// private:
    // constexpr Derived& derived() & noexcept { return *static_cast<Derived*>(this); }
    // constexpr Derived const& derived() const& noexcept { return *static_cast<Derived const*>(this); }
    // constexpr Derived&& derived() && noexcept { return *static_cast<Derived*>(this); }
};


} // namespace ufsm
