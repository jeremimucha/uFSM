#pragma once

#include <iostream>
#include "traits.hpp"
#include "transition_table.hpp"
#include "dispatch_event.hpp"
#include "logging.hpp"


/* FsmState */
/* --------------------------------------------------------------------------------------------- */
template<size_type Idx, typename State>
class FsmState
{
private:
    State value_{};
public:
    template<typename, typename...> friend class Fsm_impl;
    template<typename, typename...> friend class Fsm;
    using type = State;

    constexpr FsmState() noexcept = default;
    constexpr FsmState(FsmState&) = default;
    constexpr FsmState(FsmState const&) = default;
    constexpr FsmState(FsmState&&) noexcept = default;
    constexpr FsmState& operator=(FsmState const&) = default;
    constexpr FsmState& operator=(FsmState&&) noexcept = default;
    ~FsmState() noexcept = default;

    template<typename U
        /* , typename = std::enable_if_t<!std::is_same_v<FsmState<...>,std::decay_t<U>>> */>
    constexpr FsmState(U&& fvalue) noexcept(std::is_nothrow_constructible_v<State,U>)
        : value_{std::forward<U>(fvalue)} { }

    constexpr State& get() & noexcept { return value_; }
    constexpr State const& get() const& noexcept { return value_; }
    constexpr State&& get() && noexcept { return std::move(value_); }
    constexpr State const&& get() const&& noexcept { return std::move(value_); }
};

template<size_type Idx, typename State>
using FsmState_t = typename FsmState<Idx,State>::type;
/* --------------------------------------------------------------------------------------------- */

template<size_type Index, typename StateT>
inline constexpr FsmState_t<Index,StateT>& Get_impl(FsmState<Index,StateT>& state) noexcept;
template<size_type Index, typename StateT>
inline constexpr FsmState_t<Index,StateT> const& Get_impl(FsmState<Index,StateT> const& state) noexcept;
template<size_type Index, typename StateT>
inline constexpr FsmState_t<Index,StateT>&& Get_impl(FsmState<Index,StateT>&& state) noexcept;
template<size_type Index, typename StateT>
inline constexpr FsmState_t<Index,StateT> const&& Get_impl(FsmState<Index,StateT> const&& state) noexcept;

/* Fsm */
/* --------------------------------------------------------------------------------------------- */
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


template<typename Derived, typename... States>
class Fsm : public Fsm_impl<Make_index_sequence<sizeof...(States)>, States...>
{
    using Indices = Make_index_sequence<sizeof...(States)>;
    using Base = Fsm_impl<Indices, States...>;
public:
    constexpr Fsm() noexcept = default;

    using Base::Base;

    using Base::set_initial_state;

    template<typename FsmT, typename Event, size_type Idx, size_type... Idxs>
    friend constexpr inline void
    dispatch_event(FsmT&& fsm, Event&& event, Index_sequence<Idx,Idxs...>) noexcept;

    template<typename Event>
    constexpr inline void dispatch_event(Event&& event) noexcept
    {
        ::dispatch_event(*this, std::forward<Event>(event), Indices{});
    }

    constexpr decltype(auto) transition_table() const noexcept { return derived().transition_table(); }
    constexpr decltype(auto) transition_table() noexcept { return derived().transition_table(); }
    constexpr decltype(auto) self() & noexcept { return derived(); }
    constexpr decltype(auto) self() const& noexcept { return derived(); }
    constexpr decltype(auto) self() && noexcept { return std::move(*this).derived(); }
    constexpr decltype(auto) self() const&& noexcept { return std::move(*this).derived(); }
private:
    constexpr Derived& derived() & noexcept { return *static_cast<Derived*>(this); }
    constexpr Derived const& derived() const& noexcept { return *static_cast<Derived const*>(this); }
    constexpr Derived&& derived() && noexcept { return *static_cast<Derived*>(this); }
};


/* Get */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
template<size_type I, typename U>
inline constexpr FsmState_t<I,U>& Get_impl(FsmState<I,U>& st) noexcept
{
    return st.get();
}
template<size_type I, typename U>
inline constexpr FsmState_t<I,U> const& Get_impl(FsmState<I,U> const& st) noexcept
{
    return st.get();
}
template<size_type I, typename U>
inline constexpr FsmState_t<I,U>&& Get_impl(FsmState<I,U>&& st) noexcept
{
    return std::move(st).get();
}
template<size_type I, typename U>
inline constexpr FsmState_t<I,U>&& Get_impl(FsmState<I,U> const&& st) noexcept
{
    return std::move(st).get();
}

template<size_type Idx, typename FsmT>
inline constexpr decltype(auto) Get(FsmT&& fsm) noexcept
{
    return Get_impl<Idx>(std::forward<FsmT>(fsm));
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



/* --------------------------------------------------------------------------------------------- */
