#if !defined(TRY_SET_INITIAL_STATE_HPP_)
#define TRY_SET_INITIAL_STATE_HPP_

#include <type_traits>

#include "traits.hpp"

namespace ufsm
{

struct InitialTransitionEvent { };

namespace back
{
namespace detail
{

template<typename State, bool = IsFsm<std::decay_t<State>>>
struct trySetInitialState {
    constexpr inline void operator()(State const&) const noexcept { /* nop */ }
};

template<typename State>
struct trySetInitialState<State, true> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&& fsm) const noexcept
    {
        std::forward<FsmT>(fsm).set_initial_state(
            initial_state_v<typename std::decay_t<FsmT>::InitialState>
        );
    }
};

} // namespace detail
} // namespace back
} // namespace ufsm

#endif // TRY_SET_INITIAL_STATE_HPP_
