#pragma once

#include "logging.hpp"
#include "traits.hpp"
#include <iostream>
#include <type_traits>
#include <utility>

namespace ufsm {
namespace back {
namespace detail {

template<typename State, typename = void_t<>, typename... Args>
struct HasExitT : std::false_type { };

template<typename State, typename... Args>
using state_exit_call = decltype(std::declval<State>().exit(std::declval<Args>()...));

template<typename State, typename... Args>
struct HasExitT<State, void_t<state_exit_call<State, Args...>>, Args...> : std::true_type { };

template<typename State, typename... Args>
constexpr inline auto HasExit{HasExitT<State, void, Args...>::value};

struct NoExitAction { };
struct ExitActionNoArgs { };
struct ExitActionFsm { };
struct ExitActionFsmEvent { };

template<typename State, typename = void_t<>>
struct SelectExitActionT {
    using type = NoExitAction;
};

template<typename State>
struct SelectExitActionT<State, void_t<state_exit_call<State>>> {
    using type = ExitActionNoArgs;
};

template<typename State, typename FsmT, typename = void_t<>>
struct SelectExitActionFsmT : SelectExitActionT<State> { };

template<typename State, typename FsmT>
struct SelectExitActionFsmT<State, FsmT, void_t<state_exit_call<State, FsmT>>> {
    using type = ExitActionFsm;
};

template<typename State, typename FsmT, typename Event, typename = void_t<>>
struct SelectExitActionFsmEventT : SelectExitActionFsmT<State, FsmT> { };

template<typename State, typename FsmT, typename Event>
struct SelectExitActionFsmEventT<State, FsmT, Event, void_t<state_exit_call<State, FsmT, Event>>> {
    using type = ExitActionFsmEvent;
};

template<typename State, typename FsmT, typename Event>
using SelectExitActionSignature = typename SelectExitActionFsmEventT<State, FsmT, Event>::type;

template<typename State, bool = IsFsm<std::decay_t<State>>>
struct tryExit;

}  // namespace detail

// Intentionally do not decay the types here - HasExit should decide if FsmT has an exit()
// member callable with the given state including the qualifiers
template<typename State_,
         typename FsmT_,
         typename Event_,
         typename = detail::SelectExitActionSignature<State_, FsmT_, Event_>>
struct fsmExit {
    template<typename State, typename FsmT, typename Event>
    constexpr inline void operator()(State&& state, FsmT&&, Event&& event) const noexcept
    {
        detail::tryExit<State>{}(std::forward<State>(state), std::forward<Event>(event));
    }
};

template<typename State_, typename FsmT_, typename Event_>
struct fsmExit<State_, FsmT_, Event_, detail::ExitActionNoArgs> {
    template<typename State, typename FsmT, typename Event>
    constexpr inline void operator()(State&& state, FsmT&& fsm, Event&& event) const noexcept
    {
        detail::tryExit<State>{}(std::forward<State>(state), std::forward<Event>(event));
        logging::fsm_log_exit(fsm, detail::asBaseState(state));
        std::forward<State>(state).exit();
    }
};

template<typename State_, typename FsmT_, typename Event_>
struct fsmExit<State_, FsmT_, Event_, detail::ExitActionFsm> {
    template<typename State, typename FsmT, typename Event>
    constexpr inline void operator()(State&& state, FsmT&& fsm, Event&& event) const noexcept
    {
        detail::tryExit<State>{}(std::forward<State>(state), std::forward<Event>(event));
        logging::fsm_log_exit(fsm, detail::asBaseState(state));
        std::forward<State>(state).exit(std::forward<FsmT>(fsm));
    }
};

template<typename State_, typename FsmT_, typename Event_>
struct fsmExit<State_, FsmT_, Event_, detail::ExitActionFsmEvent> {
    template<typename State, typename FsmT, typename Event>
    constexpr inline void operator()(State&& state, FsmT&& fsm, Event&& event) const noexcept
    {
        detail::tryExit<State>{}(std::forward<State>(state), std::forward<Event>(event));
        logging::fsm_log_exit(fsm, detail::asBaseState(state));
        std::forward<State>(state).exit(std::forward<FsmT>(fsm), std::forward<Event>(event));
    }
};

namespace detail {
// TODO: optimization - narrow down the Indices to only those that refer to states
// which actually do have an exit action
template<typename Indices>
struct exitCurrentState;
template<>
struct exitCurrentState<IndexSequence<>> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&&, Event&&) const noexcept
    { /* nop */
    }
};
template<SizeT I, SizeT... Is>
struct exitCurrentState<IndexSequence<I, Is...>> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&& fsm, Event&& event) const noexcept
    {
        if (I == fsm.state())
        {
            fsmExit<StateAt<I, FsmT>, FsmT, Event>{}(
              get<I>(std::forward<FsmT>(fsm)), std::forward<FsmT>(fsm), std::forward<Event>(event));
            return;
        }
        else
        {
            exitCurrentState<IndexSequence<Is...>>{}(std::forward<FsmT>(fsm), std::forward<Event>(event));
        }
    }
};

template<typename State, bool>
struct tryExit {
    template<typename Event>
    constexpr inline void operator()(State const&, Event const&) const noexcept
    { /* nop */
    }
};

template<typename State>
struct tryExit<State, true> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&& fsm, Event&& event) const noexcept
    {
        exitCurrentState<GetIndices<FsmT>>{}(std::forward<FsmT>(fsm), std::forward<Event>(event));
    }
};

}  // namespace detail


}  // namespace back
}  // namespace ufsm
