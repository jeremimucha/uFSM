#pragma once

#include "dispatch_event.hpp"
#include "initial_state.hpp"
#include "state_index.hpp"
#include "traits.hpp"
#include <type_traits>

namespace ufsm {

struct InitialTransitionEvent { };

namespace back {
namespace detail {

template<typename State, bool = IsFsm<std::decay_t<State>>>
struct trySetInitialState {
    template<typename Event>
    constexpr inline void operator()(State const&, Event const&) const noexcept
    { /* nop */
    }
};

template<typename State>
struct trySetInitialState<State, true> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&& fsm, Event&& event) const noexcept
    {
        std::forward<FsmT>(fsm).set_initial_state(initial_state_v<typename std::decay_t<FsmT>::InitialState>);
        // An optimization - dispatch directly into the InitialState
        using fsm_t = std::decay_t<FsmT>;
        constexpr auto InitialStateIndex = StateIndex<GetStateList<fsm_t>, typename fsm_t::InitialState>;
        handle_dispatch_event(
          get<InitialStateIndex>(std::forward<FsmT>(fsm)), std::forward<FsmT>(fsm), std::forward<Event>(event));
    }
};

}  // namespace detail
}  // namespace back
}  // namespace ufsm
