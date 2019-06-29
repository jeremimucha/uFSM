#ifndef FSM_IMPL_HEADER_GUARD_HPP_
#define FSM_IMPL_HEADER_GUARD_HPP_

#include "traits.hpp"
#include "fsm_state.hpp"
#include "get.hpp"

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
class Fsm_impl<Index_sequence<Indices...>, States...>
    : public FsmState<Indices, typename state_traits<States>::state_type>...
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
} // namespace ufsm

#endif /* FSM_IMPL_HEADER_GUARD_HPP_ */
