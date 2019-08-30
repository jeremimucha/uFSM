#pragma once

#include <type_traits>
#include <utility>
#include "traits.hpp"
#include "state_transition.hpp"
#include "logging.hpp"
#include "fsmfwd.hpp"


namespace ufsm
{
namespace back
{
namespace detail
{

template<typename State, typename Void, typename... Args>
struct HasHandleEventTImpl : std::false_type { };
template<typename State, typename... Args>
struct HasHandleEventTImpl<State,
    void_t<decltype(std::declval<State>().handle_event(std::declval<Args>()...))>,
    Args...>
    : std::true_type
{
};
template<typename State, typename... Args>
struct HasHandleEventT : HasHandleEventTImpl<State, void, Args...> { };
template<typename State, typename... Args>
constexpr inline auto HasHandleEvent{HasHandleEventT<State,Args...>::value};

template<typename State, bool = IsFsm<std::decay_t<State>>>
struct tryDispatch {
    template<typename Event>
    constexpr inline void operator()(State const&, Event) const noexcept { /* nop */ }
};

template<typename State>
struct tryDispatch<State, true> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&& fsm, Event&& event) const noexcept
    {
        std::forward<FsmT>(fsm).dispatch_event(std::forward<Event>(event));
    }
};

} // namespace detail

// 1. Evaluate the guard condition associated with the transition and perform the following steps
//    only if the guard evaluates to TRUE.
// 2. Exit the source state configuration.
// 3. Execute the actions associated with the transition.
// 4. Enter the target state configuration.
// dispatch_event using `transition` for branching during event handling~
// TODO: Potential dispatch_event optimization
// Statically build a list of states that handle the given Event, iterate only over those
// states (i.e. pass and index_sequence of only the approptiate indices). This check would need
// to be recursive - all composite states need to be walked all the way down - if there's a nested
// composite state which can handle the given event the parent state of the composite handling
// the event needs to be included in the potential handlers list.
template<typename FsmT, typename Event, size_type Idx, size_type... Idxs>
constexpr inline void
dispatch_event(FsmT&& fsm, Event&& event, IndexSequence<Idx,Idxs...>) noexcept;

template<typename FsmT, typename Event, size_type Idx, size_type... Idxs>
constexpr inline void
dispatch_event(FsmT&& fsm, Event&& event, IndexSequence<Idx,Idxs...>) noexcept
{
    if (Idx == fsm.state()) {
        // This may be ufsm::Fsm<State>, rather than the State itself
        // decay into the underlying type for now
        // to correctly handle hierarhical fsm's we'll need to provide overrides for
        // states which are Fsm's themselves
        // We can't determine statically what's the current active state of the nested Fsm
        // -> we'll need to dispatch the event to the nested Fsm and check at runtime
        // if the event was handled, if it wasn't -> let the parent (current) state handle
        // the event.
        // Alternatively just dispatch the event to the nested Fsm and than handle it
        // in the parent (current) state anyway?
        // Optimization (?) -> check statically if any of the nested states can handle the
        // event, don't dispatch if it can't - general optimization of dispatch_event,
        // To handle the event in the parent (current) state we'll need to cast the nested Fsm
        // to the type that's actually in the Statelist of the current Fsm
        using state_or_fsmstate_t = StateAt<Idx, FsmT>;
        using state_t = detail::BaseFsmState<state_or_fsmstate_t>;
        using event_t = std::decay_t<Event>;
        // auto&& state = Get<Idx>(fsm);
        decltype(auto) state_or_fsmstate = Get<Idx>(fsm);
        detail::tryDispatch<state_or_fsmstate_t>{}(state_or_fsmstate, event);
        // down from here - cast to the actual State type (if state is Fsm)
        auto&& state = detail::asBaseState(state_or_fsmstate);
        logging::fsm_log_event(fsm, state, event);
        if constexpr (detail::HasHandleEvent<state_t, FsmT, Event>) {
            state_or_fsmstate.handle_event(fsm, std::forward<Event>(event));
        }
        // stateTransition<event_t, std::decay_t<FsmT>, state_t>{}(
        //     std::forward<FsmT>(fsm), std::forward<decltype(state)>(state));
        stateTransition<event_t, std::decay_t<FsmT>, state_or_fsmstate_t>{}(
            std::forward<FsmT>(fsm), std::forward<state_or_fsmstate_t>(state_or_fsmstate),
            std::forward<Event>(event)
            );
        // dispatch the event to the underlying state here - after state transition,
        // to give it a change to react to the state change?
        // this is probably wrong - sould tryDispatch to the nested states first
        // detail::tryDispatch<state_or_fsmstate_t>{}(state_or_fsmstate, event);
    }
    else if constexpr (sizeof...(Idxs) != 0) {
        dispatch_event(
            std::forward<FsmT>(fsm), std::forward<Event>(event), IndexSequence<Idxs...>{});
    }
}

} // namespace back
} // namespace ufsm
