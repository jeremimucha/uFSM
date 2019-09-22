#pragma once

#include <type_traits>
#include <utility>
#include "traits.hpp"
#include "state_transition.hpp"
#include "try_dispatch.hpp"
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

// TODO: Use this function if it's necessary to execute the transition explicitly
// once the state reference is already available, to avoid iteration.
template<typename State, typename FsmT, typename Event>
constexpr inline void handle_dispatch_event(State&& state, FsmT&& fsm, Event&& event) noexcept
{
    using state_t = detail::BaseFsmState<State>;
    using event_t = std::decay_t<Event>;
    detail::tryDispatch<State>{}(state, event);
    // down from here - cast to the actual State type (if state is Fsm)
    // auto&& state = detail::asBaseState(state_fsm);
    using detail::asBaseState;
    logging::fsm_log_event(fsm, asBaseState(state), event);
    if constexpr (detail::HasHandleEvent<state_t, FsmT, Event>) {
        state.handle_event(fsm, std::forward<Event>(event));
    }
    stateTransition<event_t, std::decay_t<FsmT>, State>{}(
        std::forward<FsmT>(fsm),
        std::forward<State>(state),
        std::forward<Event>(event)
        );
}

template<typename Indices> struct dispatchEvent;

template<> struct dispatchEvent<IndexSequence<>> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&&, Event&&) const noexcept
    {
        /* nop */
    }
};

template<size_type I, size_type... Is> struct dispatchEvent<IndexSequence<I, Is...>> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&& fsm, Event&& event) const noexcept
    {
        if (I == fsm.state()) {
            // using state_fsm_t = StateAt<I, FsmT>;
            // using state_t = detail::BaseFsmState<state_fsm_t>;
            // using event_t = std::decay_t<Event>;
            // decltype(auto) state_fsm = Get<I>(fsm);
            // detail::tryDispatch<state_fsm_t>{}(state_fsm, event);
            // // down from here - cast to the actual State type (if state is Fsm)
            // // auto&& state = detail::asBaseState(state_fsm);
            // using detail::asBaseState;
            // logging::fsm_log_event(fsm, asBaseState(state_fsm), event);
            // if constexpr (detail::HasHandleEvent<state_t, FsmT, Event>) {
            //     state_fsm.handle_event(fsm, std::forward<Event>(event));
            // }
            // stateTransition<event_t, std::decay_t<FsmT>, state_fsm_t>{}(
            //     std::forward<FsmT>(fsm),
            //     std::forward<state_fsm_t>(state_fsm),
            //     std::forward<Event>(event)
            //     );
            // return;
            handle_dispatch_event(
                Get<I>(std::forward<FsmT>(fsm)),
                std::forward<FsmT>(fsm),
                std::forward<Event>(event)
            );
        }
        else {
            dispatchEvent<IndexSequence<Is...>>{}(
                std::forward<FsmT>(fsm), std::forward<Event>(event)
            );
        }
    }
};

template<typename FsmT, typename Event, size_type Idx, size_type... Idxs>
constexpr inline void
dispatch_event(FsmT&& fsm, Event&& event, IndexSequence<Idx,Idxs...>) noexcept;

template<typename FsmT, typename Event, size_type Idx, size_type... Idxs>
constexpr inline void
dispatch_event(FsmT&& fsm, Event&& event, IndexSequence<Idx,Idxs...>) noexcept
{
    if (Idx == fsm.state()) {
        // This may be ufsm::Fsm<State, ...>, rather than the State itself
        // We can't determine statically what's the current active state of the nested Fsm
        // -> dispatch the event to the nested Fsm and check at runtime
        // if the event was handled, if it wasn't -> let the parent (current) state handle
        // the event.
        // Alternatively just dispatch the event to the nested Fsm and than handle it
        // in the parent (current) state anyway?
        // Optimization (?) -> check statically if any of the nested states can handle the
        // event, don't dispatch if it can't - general optimization of dispatch_event,
        // To handle the event in the parent (current) state, cast the nested Fsm
        // to the type that's actually in the Statelist of the current Fsm
        using state_fsm_t = StateAt<Idx, FsmT>;
        using state_t = detail::BaseFsmState<state_fsm_t>;
        using event_t = std::decay_t<Event>;
        decltype(auto) state_fsm = Get<Idx>(fsm);
        detail::tryDispatch<state_fsm_t>{}(state_fsm, event);
        // down from here - cast to the actual State type (if state is Fsm)
        // auto&& state = detail::asBaseState(state_fsm);
        using detail::asBaseState;
        logging::fsm_log_event(fsm, asBaseState(state_fsm), event);
        if constexpr (detail::HasHandleEvent<state_t, FsmT, Event>) {
            state_fsm.handle_event(fsm, std::forward<Event>(event));
        }
        stateTransition<event_t, std::decay_t<FsmT>, state_fsm_t>{}(
            std::forward<FsmT>(fsm),
            std::forward<state_fsm_t>(state_fsm),
            std::forward<Event>(event)
            );
    }
    else if constexpr (sizeof...(Idxs) != 0) {
        dispatch_event(
            std::forward<FsmT>(fsm), std::forward<Event>(event), IndexSequence<Idxs...>{});
    }
}

} // namespace back
} // namespace ufsm
