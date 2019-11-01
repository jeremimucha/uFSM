#pragma once

#include <utility>
#include "traits.hpp"
#include "fsm_state.hpp"
#include "state_traits.hpp"

namespace ufsm
{

template<typename State, typename FsmT>
inline constexpr decltype(auto) get(FsmT&& fsm) noexcept;

template<SizeT Idx, typename FsmT>
using StateAt = decltype(get<Idx>(std::declval<FsmT>()));

namespace back
{

template<SizeT Idx, typename FsmT>
inline constexpr decltype(auto) get(FsmT&& fsm) noexcept;

template<SizeT I, typename U>
inline constexpr FsmStateT<I,U>& get_impl(FsmState<I,U>& st) noexcept
{
    return st.get();
}
template<SizeT I, typename U>
inline constexpr FsmStateT<I,U> const& get_impl(FsmState<I,U> const& st) noexcept
{
    return st.get();
}
template<SizeT I, typename U>
inline constexpr FsmStateT<I,U>&& get_impl(FsmState<I,U>&& st) noexcept
{
    return std::move(st).get();
}
template<SizeT I, typename U>
inline constexpr FsmStateT<I,U> const&& get_impl(FsmState<I,U> const&& st) noexcept
{
    return std::move(st).get();
}

template<typename State, SizeT Idx> inline constexpr
FsmStateT<Idx,State>& get_impl(FsmState<Idx,State>& state) noexcept
{
    return state.get();
}
template<typename State, SizeT Idx> inline constexpr
FsmStateT<Idx,State> const& get_impl(FsmState<Idx,State> const& state) noexcept
{
    return state.get();
}
template<typename State, SizeT Idx> inline constexpr
FsmStateT<Idx,State>&& get_impl(FsmState<Idx,State>&& state) noexcept
{
    return std::move(state).get();
}
template<typename State, SizeT Idx> inline constexpr
FsmStateT<Idx,State> const&& get_impl(FsmState<Idx,State> const&& state) noexcept
{
    return std::move(state).get();
}

template<SizeT Idx, typename FsmT>
inline constexpr decltype(auto) get(FsmT&& fsm) noexcept
{
    return back::get_impl<Idx>(std::forward<FsmT>(fsm));
}

} // namespace back

template<typename State, typename FsmT>
inline constexpr decltype(auto) get(FsmT&& fsm) noexcept
{
    return back::get_impl<typename StateTraitsT<State>::state_type>(std::forward<FsmT>(fsm));
}

} // namespace ufsm
