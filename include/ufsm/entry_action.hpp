#pragma once

#include <type_traits>
#include <utility>
#include "traits.hpp"
#include "logging.hpp"
#include "try_set_initial_state.hpp"


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

template<typename FsmT_, typename State_,
         bool HasEntry = detail::has_entry_v<State_, FsmT_>>
struct FsmEntry {
    template<typename FsmT, typename State>
    constexpr inline void operator()(FsmT&&, State&&) noexcept { }
};

template<typename FsmT_, typename State_>
struct FsmEntry<FsmT_, State_, true> {
    template<typename FsmT, typename State>
    constexpr inline void operator()(FsmT&& fsm, State&& state) noexcept
    {
        logging::fsm_log_entry(fsm, state);
        // TODO: remove first std::forward call
        std::forward<State>(state).entry(std::forward<FsmT>(fsm));
        detail::trySetInitialState<std::decay_t<State>>{}(std::forward<State>(state));
    }
};

template<typename FsmT, typename State>
constexpr inline void fsm_entry(FsmT&& fsm, State&& state) noexcept
{
    using fsm_t = std::decay_t<FsmT>;
    using state_t = std::decay_t<State>;
    FsmEntry<fsm_t, state_t>{}(std::forward<FsmT>(fsm), std::forward<State>(state));
}

// template<typename FsmT, typename State>
// constexpr inline std::enable_if_t<!detail::has_entry_v<State, Self<FsmT>>>
// fsm_entry(FsmT&&, State&&) noexcept {/* nop */}

// template<typename FsmT, typename State>
// constexpr inline std::enable_if_t<detail::has_entry_v<State, Self<FsmT>>>
// fsm_entry(FsmT&& fsm, State&& state) noexcept
// {
//     logging::fsm_log_entry(fsm.self(), state);
//     std::forward<State>(state).entry(std::forward<FsmT>(fsm).self());
// }

} // namespace back
} // namespace ufsm
