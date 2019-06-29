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
struct has_handle_event_impl : std::false_type { };
template<typename State, typename... Args>
struct has_handle_event_impl<State,
    void_t<decltype(std::declval<State>().handle_event(std::declval<Args>()...))>,
    Args...>
    : std::true_type
{
};
template<typename State, typename... Args>
struct has_handle_event : has_handle_event_impl<State, void, Args...> { };
template<typename State, typename... Args>
constexpr inline auto has_handle_event_v{has_handle_event<State,Args...>::value};

template<typename State> struct isFsmT : std::false_type { };
template<typename Impl, typename States>
struct isFsmT<Fsm<Impl, States>> : std::true_type { };

template<typename FsmT> struct baseFsmStateImpl;
template<typename Impl, typename States>
struct baseFsmStateImpl<ufsm::Fsm<Impl, States>> { using type = Impl; };
template<typename Impl, typename States>
struct baseFsmStateImpl<ufsm::Fsm<Impl, States> const> { using type = Impl const; };
template<typename Impl, typename States>
struct baseFsmStateImpl<ufsm::Fsm<Impl, States> const&> { using type = Impl const&; };
template<typename Impl, typename States>
struct baseFsmStateImpl<ufsm::Fsm<Impl, States> const&&> { using type = Impl const&&; };
template<typename Impl, typename States>
struct baseFsmStateImpl<ufsm::Fsm<Impl, States>&> { using type = Impl&; };
template<typename Impl, typename States>
struct baseFsmStateImpl<ufsm::Fsm<Impl, States>&&> { using type = Impl&&; };

template<typename State, bool = isFsmT<std::decay_t<State>>::value>
struct baseFsmState { using type = State; };
template<typename State>
struct baseFsmState<State, true> : baseFsmStateImpl<State> { };

template<typename State>
using baseFsmStateT = typename baseFsmState<State>::type;

template<typename FsmT>
constexpr decltype(auto) as_base_state(FsmT&& fsm) noexcept
{
    return static_cast<baseFsmStateT<FsmT>>(std::forward<FsmT>(fsm));
}

template<typename State, bool = isFsmT<std::decay_t<State>>::value>
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
// dispatch_event using `transition` for branching during event handling
template<typename FsmT, typename Event, size_type Idx, size_type... Idxs>
constexpr inline void
dispatch_event(FsmT&& fsm, Event&& event, Index_sequence<Idx,Idxs...>) noexcept;

template<typename FsmT, typename Event, size_type Idx, size_type... Idxs>
constexpr inline void
dispatch_event(FsmT&& fsm, Event&& event, Index_sequence<Idx,Idxs...>) noexcept
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
        using state_or_fsmstate_t = state_at<Idx, FsmT>;
        using state_t = detail::baseFsmStateT<state_or_fsmstate_t>;
        using event_t = std::decay_t<Event>;
        // auto&& state = Get<Idx>(fsm);
        decltype(auto) state_or_fsmstate = Get<Idx>(fsm);
        // try_dispatch_event(state, event) -> call state.dispatch_event(event) if state is Fsm
        detail::tryDispatch<state_or_fsmstate_t>{}(state_or_fsmstate, event);
        // down from here - cast to the actual State type (if state is Fsm)
        auto&& state = detail::as_base_state(state_or_fsmstate);
        logging::fsm_log_event(fsm, state, event);
        if constexpr (detail::has_handle_event_v<std::decay_t<state_t>, FsmT, Event>) {
            state.handle_event(fsm, std::forward<Event>(event));
        }
        // StateTransition<event_t, std::decay_t<FsmT>, state_t>{}(
        //     std::forward<FsmT>(fsm), std::forward<decltype(state)>(state));
        StateTransition<event_t, std::decay_t<FsmT>, state_t>{}(
            std::forward<FsmT>(fsm), std::forward<decltype(state)>(state));
    }
    else if constexpr (sizeof...(Idxs) != 0) {
        dispatch_event(
            std::forward<FsmT>(fsm), std::forward<Event>(event), Index_sequence<Idxs...>{});
    }
}

} // namespace back
} // namespace ufsm
