#if !defined(TRY_SET_INITIAL_STATE_HPP_)
#define TRY_SET_INITIAL_STATE_HPP_

#include <type_traits>
#include "traits.hpp"
#include "dispatch_event.hpp"

namespace ufsm
{

struct InitialTransitionEvent { };

namespace back
{
namespace detail
{

template<typename State, bool = IsFsm<std::decay_t<State>>>
struct trySetInitialState {
    template<typename Event>
    constexpr inline void operator()(State const&, Event const&) const noexcept { /* nop */ }
};

template<typename State>
struct trySetInitialState<State, true> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&& fsm, Event&& event) const noexcept
    {
        std::forward<FsmT>(fsm).set_initial_state(
            initial_state_v<typename std::decay_t<FsmT>::InitialState>
        );
        // An optimization - dispatch directly into the InitialState
        using fsm_t = std::decay_t<FsmT>;
        constexpr auto InitialStateIndex = StateIndex<GetStateList<fsm_t>, fsm_t::InitialState>;
        handle_dispatch_event(
            Get<InitialStateIndex>(std::forward<FsmT>(fsm)),
            std::forward<FsmT>(fsm),
            std::forward<Event>(event)
        );
    }
};

} // namespace detail
} // namespace back
} // namespace ufsm

#endif // TRY_SET_INITIAL_STATE_HPP_
