#pragma once

#include <utility>
#include "traits.hpp"
#include "get.hpp"

namespace ufsm
{
namespace back
{

template<SizeT Idx, typename State>
class FsmState
{
private:
    State value_{};

protected:
    ~FsmState() noexcept = default;

public:
    template<typename, typename...> friend class Fsm_impl;
    template<typename, typename> friend class Fsm;
    using type = State;

    constexpr FsmState() noexcept = default;
    constexpr FsmState(FsmState&) = default;
    constexpr FsmState(FsmState const&) = default;
    constexpr FsmState(FsmState&&) noexcept = default;
    constexpr FsmState& operator=(FsmState const&) = default;
    constexpr FsmState& operator=(FsmState&&) noexcept = default;

    template<typename U
        /* , typename = std::enable_if_t<!std::is_same_v<FsmState<...>,std::decay_t<U>>> */>
    constexpr FsmState(U&& fvalue) noexcept(std::is_nothrow_constructible_v<State,U>)
        : value_{std::forward<U>(fvalue)} { }

    constexpr State& get() & noexcept { return value_; }
    constexpr State const& get() const& noexcept { return value_; }
    constexpr State&& get() && noexcept { return std::move(value_); }
    constexpr State const&& get() const&& noexcept { return std::move(value_); }
};

template<SizeT Idx, typename State>
using FsmState_t = typename FsmState<Idx,State>::type;

/* Get */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
template<SizeT I, typename U>
inline constexpr FsmState_t<I,U>& Get_impl(FsmState<I,U>& st) noexcept
{
    return st.get();
}
template<SizeT I, typename U>
inline constexpr FsmState_t<I,U> const& Get_impl(FsmState<I,U> const& st) noexcept
{
    return st.get();
}
template<SizeT I, typename U>
inline constexpr FsmState_t<I,U>&& Get_impl(FsmState<I,U>&& st) noexcept
{
    return std::move(st).get();
}
template<SizeT I, typename U>
inline constexpr FsmState_t<I,U>&& Get_impl(FsmState<I,U> const&& st) noexcept
{
    return std::move(st).get();
}

template<SizeT Idx, typename FsmT>
inline constexpr decltype(auto) Get(FsmT&& fsm) noexcept
{
    return Get_impl<Idx>(std::forward<FsmT>(fsm));
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* get */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
template<typename State, SizeT Idx> inline constexpr
FsmState_t<Idx,State>& get_state_impl(FsmState<Idx,State>& state) noexcept
{
    return state.get();
}
template<typename State, SizeT Idx> inline constexpr
FsmState_t<Idx,State> const& get_state_impl(FsmState<Idx,State> const& state) noexcept
{
    return state.get();
}
template<typename State, SizeT Idx> inline constexpr
FsmState_t<Idx,State>&& get_state_impl(FsmState<Idx,State>&& state) noexcept
{
    return std::move(state).get();
}
template<typename State, SizeT Idx> inline constexpr
FsmState_t<Idx,State> const&& get_state_impl(FsmState<Idx,State> const&& state) noexcept
{
    return std::move(state).get();
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

} // namespace back

/* get */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
template<typename State, typename FsmT>
inline constexpr decltype(auto) get_state(FsmT&& fsm) noexcept
{
    return back::get_state_impl<typename StateTraitsT<State>::state_type>(std::forward<FsmT>(fsm));
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

} // namespace ufsm
