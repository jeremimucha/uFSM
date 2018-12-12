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
struct has_entry : std::false_type { };

template<typename State, typename FsmT>
struct has_entry<State, FsmT,
    void_t<decltype(std::declval<State>().entry(std::declval<FsmT>()))>>
    : std::true_type
{
};

template<typename State, typename FsmT>
constexpr inline auto has_entry_v{has_entry<State,FsmT>::value};
} // namespace detail

// template<typename FsmT, typename State>
// constexpr inline std::enable_if_t<!has_entry_v<State, Self<FsmT>>>
// fsm_entry(FsmT&&, State&&) noexcept;

// template<typename FsmT, typename State>
// constexpr inline std::enable_if_t<has_entry_v<State, Self<FsmT>>>
// fsm_entry(FsmT&& fsm, State&& state) noexcept;

template<typename FsmT, typename State>
constexpr inline std::enable_if_t<!detail::has_entry_v<State, Self<FsmT>>>
fsm_entry(FsmT&&, State&&) noexcept {/* nop */}

template<typename FsmT, typename State>
constexpr inline std::enable_if_t<detail::has_entry_v<State, Self<FsmT>>>
fsm_entry(FsmT&& fsm, State&& state) noexcept
{
    logging::fsm_log_entry(fsm.self(), state);
    std::forward<State>(state).entry(std::forward<FsmT>(fsm).self());
}

} // namespace back
} // namespace ufsm
