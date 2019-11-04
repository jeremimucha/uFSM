#ifndef FSM_IMPL_HEADER_GUARD_HPP_
#define FSM_IMPL_HEADER_GUARD_HPP_

#include "traits.hpp"
#include "fsm_state.hpp"
#include "state_index.hpp"
#include "initial_state.hpp"
#include "state_traits.hpp"
#include "get.hpp"

namespace ufsm
{

namespace back
{

template<typename IndexSequence, typename... States>
class Fsm_impl;

template<SizeT... Indices, typename... States>
class Fsm_impl<IndexSequence<Indices...>, States...>
    : public FsmState<Indices, typename StateTraitsT<States>::state_type>...
{
public:
    constexpr Fsm_impl() noexcept = default;

    template<typename State>
    constexpr explicit Fsm_impl(initial_state_t<State>) noexcept
        : state_{StateIndex<Typelist<States...>,State>}
        {
        }

    template<typename State>
    constexpr void set_initial_state(initial_state_t<State>) noexcept
    {
        state_ = StateIndex<Typelist<States...>,State>;
    }

    constexpr inline SizeT state() const noexcept { return state_; }
    constexpr inline void state(SizeT new_state) noexcept { state_ = new_state; }
private:
    SizeT state_{};
};

} // namespace back
} // namespace ufsm

#endif /* FSM_IMPL_HEADER_GUARD_HPP_ */
