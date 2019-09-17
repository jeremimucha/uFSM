#pragma once

#include <iostream>
#include <type_traits>
#include <utility>
#include "traits.hpp"
#include "logging.hpp"

namespace ufsm
{
namespace back
{
namespace detail
{

template<typename State, typename = void_t<>, typename... Args>
struct HasExitT : std::false_type { };

template<typename State, typename... Args>
using state_exit_call = decltype(std::declval<State>().exit(std::declval<Args>()...));

template<typename State, typename... Args>
struct HasExitT<State,
                void_t<state_exit_call<State, Args...>>,
                Args...>
: std::true_type { };

template<typename State, typename... Args>
constexpr inline auto HasExit{HasExitT<State, void, Args...>::value};

struct NoExitAction { };
struct ExitActionFsm { };
struct ExitActionFsmEvent { };

// TODO: Implement exit action for different overloads

template<typename State, typename FsmT, typename = void_t<>>
struct SelectExitActionFsmT { using type = NoExitAction; };

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

} // namespace detail

// template<typename State_, typename ExitActionSignature,
//          typename... Args_>
// struct fsmExitT;

// template<typename State, typename... Args_>
// struct fsmExitT<State, detail::NoExitAction, Args_...>{
//     template<typename State, typename... Args>
//     constexpr inline void operator()(State&& state, Args&&... args) const noexcept
//     {
//         detail::tryExit<State>{}(std::forward<State>(state));
//     }
// };
// // Select the correct number of arguments somehow?
// template<typename State, typename... Args>
// struct fsmExitT<State, detail::ExitActionFsm, Args...> {
//     template<typename State, typename... Args>
//     constexpr inline void operator()(State&& state, Args&&... args) const noexcept
//     {
//         detail::tryExit<State>{}(std::forward<State>(state));
//         logging::fsm_log_exit(fsm, detail::asBaseState(state));
//         std::forward<State>(state).exit(std::forward<Args>(args)...);
//     }
// };

// Intentionally do not decay the types here - HasExit should decide if FsmT has an exit()
// member callable with the given state including the qualifiers
template<typename State_, typename FsmT_, typename Event_,
         typename = detail::SelectExitActionSignature<State_, FsmT_, Event_>>
struct fsmExit {
    template<typename State, typename FsmT, typename Event>
    constexpr inline void operator()(State&& state, FsmT&&, Event&& event) const noexcept
    {
        detail::tryExit<State>{}(std::forward<State>(state), std::forward<Event>(event));
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

// template<typename FsmT_, typename State_,
//          bool = detail::HasExit<State_, FsmT_>>
// struct fsmExit {
//     template<typename FsmT, typename State>
//     constexpr inline void operator()(FsmT&&, State&& state) const noexcept
//     {
//         detail::tryExit<State>{}(std::forward<State>(state));
//     }
// };

// template<typename FsmT_, typename State_>
// struct fsmExit<FsmT_, State_, true> {
//     template<typename FsmT, typename State>
//     constexpr inline void operator()(FsmT&& fsm, State&& state) const noexcept
//     {
//         detail::tryExit<State>{}(std::forward<State>(state));
//         logging::fsm_log_exit(fsm, detail::asBaseState(state));
//         std::forward<State>(state).exit(std::forward<FsmT>(fsm));
//     }
// };

// template<typename FsmT, typename State>
// constexpr inline void fsm_exit(FsmT&& fsm, State&& state) noexcept
// {
//     // Intentionally do not decay the types here - HasExit should decide if FsmT has an exit()
//     // member callable with the given state including the qualifiers
//     fsmExit<FsmT, State>{}(std::forward<FsmT>(fsm), std::forward<State>(state));
// }

namespace detail
{
// TODO: optimization - narrow down the Indices to only those that refer to states
// which actually do have an exit action
template<typename Indices> struct exitCurrentState;
template<>
struct exitCurrentState<IndexSequence<>> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&&, Event&&) const noexcept {/* nop */}
};
template<size_type I, size_type... Is>
struct exitCurrentState<IndexSequence<I, Is...>> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&& fsm, Event&& event) const noexcept
    {
        if (I == fsm.state()) {
            fsmExit<StateAt<I, FsmT>, FsmT, Event>{}(
                Get<I>(std::forward<FsmT>(fsm)),
                std::forward<FsmT>(fsm),
                std::forward<Event>(event)
            );
            return;
        }
        else {
            exitCurrentState<IndexSequence<Is...>>{}(
                std::forward<FsmT>(fsm),
                std::forward<Event>(event)
            );
        }
    }
};

template<typename State, bool>
struct tryExit {
    template<typename Event>
    constexpr inline void operator()(State const&, Event const&) const noexcept
    {
        /* nop */
    }
};

template<typename State>
struct tryExit<State, true> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&& fsm, Event&& event) const noexcept
    {
        exitCurrentState<typename std::decay_t<FsmT>::Indices>{}(
            std::forward<FsmT>(fsm),
            std::forward<Event>(event)
        );
    }
};

} // namespace detail


} // namespace back
} // namespace ufsm
