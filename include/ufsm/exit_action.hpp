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

} // namespace back
} // namespace ufsm
