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
template<typename State, typename FsmT, typename = void_t<>>
struct has_exit : std::false_type { };

template<typename State, typename FsmT>
struct has_exit<State, FsmT,
    void_t<decltype(std::declval<State>().exit(std::declval<FsmT>()))>>
    : std::true_type
{
};

template<typename State, typename FsmT>
constexpr inline auto has_exit_v{has_exit<State,FsmT>::value};

template<typename State, bool = isFsm<std::decay_t<State>>>
struct tryExit;

} // namespace detail

template<typename FsmT_, typename State_,
         bool HasExit = detail::has_exit_v<State_, FsmT_>>
struct FsmExit {
    template<typename FsmT, typename State>
    constexpr inline void operator()(FsmT&&, State&&) noexcept { }
};

template<typename FsmT_, typename State_>
struct FsmExit<FsmT_, State_, true> {
    template<typename FsmT, typename State>
    constexpr inline void operator()(FsmT&& fsm, State&& state) noexcept
    {
        logging::fsm_log_exit(fsm, state);
        std::forward<State>(state).exit(std::forward<FsmT>(fsm));
        detail::tryExit<std::decay_t<State>>{}(std::forward<State>(state));
    }
};

template<typename FsmT, typename State>
constexpr inline void fsm_exit(FsmT&& fsm, State&& state) noexcept
{
    using fsm_t = std::decay_t<FsmT>;
    using state_t = std::decay_t<State>;
    FsmExit<fsm_t, state_t>{}(std::forward<FsmT>(fsm), std::forward<State>(state));
}

// template<typename FsmT, typename State>
// constexpr inline std::enable_if_t<detail::has_exit_v<State, Self<FsmT>>>
// fsm_exit(FsmT&& fsm, State&& state) noexcept {
//     logging::fsm_log_exit(fsm.self(), state);
//     std::forward<State>(state).exit(std::forward<FsmT>(fsm).self());
// }

// template<typename FsmT, typename State>
// constexpr inline std::enable_if_t<!detail::has_exit_v<State, Self<FsmT>>>
// fsm_exit(FsmT&&, State&&) noexcept {/* nop */}

namespace detail
{
template<typename Indices> struct exitCurrentState;
template<>
struct exitCurrentState<Index_sequence<>> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&&) const noexcept {/* nop */}
};
template<size_type I, size_type... Is>
struct exitCurrentState<Index_sequence<I, Is...>> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&& fsm) const noexcept
    {
        if (I == fsm.state()) {
            fsm_exit(std::forward<FsmT>(fsm), Get<I>(std::forward<FsmT>(fsm)));
            return;
        }
        exitCurrentState<Index_sequence<Is...>>{}(std::forward<FsmT>(fsm));
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
        exitCurrentState<typename FsmT::Indices>{}(std::forward<FsmT>(fsm));
    }
};

} // namespace detail


} // namespace back
} // namespace ufsm
