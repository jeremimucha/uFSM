#pragma once

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
struct HasExitT<State,
                void_t<decltype(std::declval<State>().exit(std::declval<Args>()...))>,
                Args...>
: std::true_type { };

template<typename State, typename... Args>
constexpr inline auto HasExit{HasExitT<State, void, Args...>::value};

template<typename State, bool = IsFsm<std::decay_t<State>>>
struct tryExit;

} // namespace detail

// Intentionally do not decay the types here - HasExit should decide if FsmT has an exit()
// member callable with the given state including the qualifiers
template<typename FsmT_, typename State_,
         bool = detail::HasExit<State_, FsmT_>>
struct fsmExit {
    template<typename FsmT, typename State>
    constexpr inline void operator()(FsmT&&, State&&) const noexcept { }
};

template<typename FsmT_, typename State_>
struct fsmExit<FsmT_, State_, true> {
    template<typename FsmT, typename State>
    constexpr inline void operator()(FsmT&& fsm, State&& state) const noexcept
    {
        detail::tryExit<std::decay_t<State>>{}(std::forward<State>(state));
        logging::fsm_log_exit(fsm, state);
        std::forward<State>(state).exit(std::forward<FsmT>(fsm));
    }
};

template<typename FsmT, typename State>
constexpr inline void fsm_exit(FsmT&& fsm, State&& state) noexcept
{
    // Intentionally do not decay the types here - HasExit should decide if FsmT has an exit()
    // member callable with the given state including the qualifiers
    // using fsm_t = std::decay_t<FsmT>;
    // using state_t = std::decay_t<State>;
    fsmExit<FsmT, State>{}(std::forward<FsmT>(fsm), std::forward<State>(state));
}

namespace detail
{
// TODO: optimization - narrow down the Indices to only those that refer to states
// which actually do have an exit action
template<typename Indices> struct exitCurrentState;
template<>
struct exitCurrentState<IndexSequence<>> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&&) const noexcept {/* nop */}
};
template<size_type I, size_type... Is>
struct exitCurrentState<IndexSequence<I, Is...>> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&& fsm) const noexcept
    {
        if (I == fsm.state()) {
            fsm_exit(std::forward<FsmT>(fsm), Get<I>(std::forward<FsmT>(fsm)));
            return;
        }
        exitCurrentState<IndexSequence<Is...>>{}(std::forward<FsmT>(fsm));
    }
};

template<typename State, bool>
struct tryExit {
    constexpr inline void operator()(State const&) const noexcept {/* nop */}
};

template<typename State>
struct tryExit<State, true> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&& fsm) const noexcept
    {
        exitCurrentState<typename std::decay_t<FsmT>::Indices>{}(std::forward<FsmT>(fsm));
    }
};

} // namespace detail


} // namespace back
} // namespace ufsm
