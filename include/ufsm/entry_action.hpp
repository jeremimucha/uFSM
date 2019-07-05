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

template<typename State, bool = isFsm<State>>
struct tryEnter;

template<typename State, typename = get_entry_policy<std::decay_t<State>>>
struct propagateEntry;
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
        // detail::trySetInitialState<std::decay_t<State>>{}(std::forward<State>(state));
        // or if we want to remember the last state:
        detail::tryEnter<std::decay_t<State>>{}(std::forward<State>(state));
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

namespace detail
{
template<typename Indices> struct enterCurrentState;
template<>
struct enterCurrentState<Index_sequence<>> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&&) const noexcept { };
};

template<size_type I, size_type... Is>
struct enterCurrentState<Index_sequence<I, Is...>> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&& fsm) {
        if (I == fsm.state()) {
            fsm_entry(std::forward<FsmT>(fsm), Get<I>(std::forward<FsmT>(fsm)));
            return;
        }
        enterCurrentState<Index_sequence<Is...>>{}(std::forward<FsmT>(fsm));
    }
};

template<typename State, bool>
struct tryEnter {
    constexpr inline void operator()(State const&) const noexcept {/* nop */}
};

template<typename State>
struct tryEnter<State, true> {
    template<typename FsmT>
    constexpr inline void operator()(FsmT&& fsm) const noexcept {
        enterCurrentState<typename std::decay_t<FsmT>::Indices>{}(std::forward<FsmT>(fsm));
    }
};
template<typename State, typename>
struct propagateEntry {
    constexpr inline void operator()(State const&) const noexcept { }
};
template<typename State>
struct propagateEntry<State, InitialStateEntryPolicy> {

};
} // namespace detail


} // namespace back
} // namespace ufsm
