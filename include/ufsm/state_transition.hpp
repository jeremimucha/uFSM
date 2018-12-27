#pragma once

#include <type_traits>
#include <utility>
#include "traits.hpp"
#include "get.hpp"
#include "action.hpp"
#include "entry_action.hpp"
#include "exit_action.hpp"
#include "logging.hpp"


namespace ufsm
{
namespace back
{
namespace detail
{
template<typename T, typename FsmT, typename = void_t<>>
struct has_guard : std::false_type { };

template<typename T, typename FsmT>
struct has_guard<T, FsmT, void_t<decltype(std::declval<T>().guard)>>
    : std::is_invocable_r<bool, decltype(std::declval<T>().guard), FsmT>
{
};

template<typename T, typename FsmT>
constexpr inline auto has_guard_v{has_guard<T,FsmT>::value};

template<typename T, typename... Args>
struct is_valid_guard : std::is_invocable_r<bool, T, Args...> { };
template<typename T, typename... Args>
constexpr inline auto is_valid_guard_v{is_valid_guard<T,Args...>::value};

template<typename FsmT, typename State, typename Event, typename = void_t<>>
struct HasTraitsFor : std::false_type { };
template<typename FsmT,typename State, typename Event>
struct HasTraitsFor<FsmT, State, Event,
    void_t<decltype(
        Get_transition_traits<std::decay_t<State>,std::decay_t<Event>>(
            std::declval<FsmT>().transition_table()))>>
    : std::true_type
{
};
template<typename FsmT, typename State, typename Event>
constexpr inline auto HasTraitsFor_v = HasTraitsFor<FsmT,State,Event>::value;

} // namespace detail

// -Guard, -NextState
template<typename FsmT_, typename TTraits_,
         bool HasGuard = detail::has_guard_v<TTraits_, Self<FsmT_>>,
         bool HasNextState = has_next_state_v<TTraits_>>
struct StateTransition_impl {
    template<typename FsmT, typename TTraits, typename State>
    constexpr inline void operator()(FsmT&& fsm, TTraits&& traits, State&&) noexcept
    {
        FsmAction<FsmT,TTraits>{}(std::forward<FsmT>(fsm), std::forward<TTraits>(traits));
        fsm_action(std::forward<FsmT>(fsm), std::forward<TTraits>(traits));
    }
};

// +Guard, -NextState
template<typename FsmT_, typename TTraits_>
struct StateTransition_impl<FsmT_, TTraits_, true, false> {
    template<typename FsmT, typename TTraits, typename State>
    constexpr inline void operator()(FsmT&& fsm, TTraits&& ttraits, State&&) noexcept
    {
        static_assert(detail::is_valid_guard_v<decltype(ttraits.guard), decltype(fsm.self())>);
        const auto guard_result = ttraits.guard(fsm.self());
        logging::fsm_log_guard(fsm.self(), ttraits.guard, guard_result);
        if (guard_result) {
            fsm_action(std::forward<FsmT>(fsm), std::forward<TTraits>(ttraits));
        }
    }
};

// -Guard, +NextState
template<typename FsmT_, typename TTraits_>
struct StateTransition_impl<FsmT_, TTraits_, false, true> {
    template<typename FsmT, typename TTraits, typename State>
    constexpr inline void operator()(FsmT&& fsm, TTraits&& ttraits, State&& state) noexcept
    {
        using fsm_t = std::decay_t<FsmT>;
        using ttraits_t = std::decay_t<TTraits>;
        // using state_t = std::decay_t<State>;
        fsm_exit(fsm, state);
        fsm_action(fsm, ttraits);
        using fsm_statelist = get_state_list_t<fsm_t>;
        constexpr auto next_state_idx = state_index_v<fsm_statelist, Next_state<ttraits_t>>;
        fsm.state(next_state_idx);
        auto&& next_state = Get<next_state_idx>(fsm);
        logging::fsm_log_state_change(fsm.self(), state, next_state);
        fsm_entry(std::forward<FsmT>(fsm), std::forward<decltype(next_state)>(next_state));
    }
};

// +Guard, +NextState
template<typename FsmT_, typename TTraits_>
struct StateTransition_impl<FsmT_, TTraits_, true, true> {
    template<typename FsmT, typename TTraits, typename State>
    constexpr inline void operator()(FsmT&& fsm, TTraits&& ttraits, State&& state) noexcept
    {
        static_assert(detail::is_valid_guard_v<decltype(ttraits.guard), decltype(fsm.self())>);
        const auto guard_result = ttraits.guard(fsm.self());
        logging::fsm_log_guard(fsm.self(), ttraits.guard, guard_result);
        if (guard_result) {
            fsm_exit(fsm, state);
            fsm_action(fsm, ttraits);
            using fsm_statelist = get_state_list_t<std::decay_t<FsmT>>;
            constexpr auto next_state_idx = state_index_v<fsm_statelist, Next_state<std::decay_t<TTraits>>>;
            fsm.state(next_state_idx);
            auto&& next_state = Get<next_state_idx>(fsm);
            logging::fsm_log_state_change(fsm.self(), state, next_state);
            fsm_entry(std::forward<FsmT>(fsm), std::forward<decltype(next_state)>(next_state));
        }
    }
};

template<typename Event, typename FsmT_, typename State_,
         bool = detail::HasTraitsFor_v<FsmT_, State_, Event>>
struct StateTransition {
    template<typename FsmT, typename State>
    constexpr inline void operator()(FsmT&&, State&&) noexcept
    {
        /* nop */
    }
};

template<typename Event, typename FsmT_, typename State_>
struct StateTransition<Event, FsmT_, State_, true> {
    template<typename FsmT, typename State>
    constexpr inline void operator()(FsmT&& fsm, State&& state) noexcept
    {
        using state_t = std::decay_t<State>;
        using event_t = std::decay_t<Event>;
        auto&& ttraits = Get_transition_traits<state_t, event_t>(fsm.transition_table());
        StateTransition_impl<std::decay_t<FsmT>, std::decay_t<decltype(ttraits)>>{}(
            std::forward<FsmT>(fsm), std::forward<decltype(ttraits)>(ttraits),
            std::forward<State>(state));
    }
};

// // -Guard, -NextState
// template<typename FsmT, typename TTraits, typename State>
// constexpr inline std::enable_if_t<!detail::has_guard_v<TTraits, Self<FsmT>>&&
//                                   !has_next_state_v<TTraits>>
// state_transition_impl(FsmT&& fsm, TTraits&& ttraits, State&&) noexcept {
//     FsmAction<FsmT,TTraits>{}(std::forward<FsmT>(fsm), std::forward<TTraits>(ttraits));
//     // fsm_action(std::forward<FsmT>(fsm), std::forward<TTraits>(ttraits));
// }

// // +Guard, -NextState
// template<typename FsmT, typename TTraits, typename State>
// constexpr inline std::enable_if_t<detail::has_guard_v<TTraits,Self<FsmT>> &&
//                                   !has_next_state_v<TTraits>>
// state_transition_impl(FsmT&& fsm, TTraits&& ttraits, State&& state) noexcept {
//     static_assert(detail::is_valid_guard_v<decltype(ttraits.guard), decltype(fsm.self())>);
//     const auto guard_result = ttraits.guard(fsm.self());
//     logging::fsm_log_guard(fsm.self(), ttraits.guard, guard_result);
//     if (guard_result) {
//         FsmAction<FsmT,TTraits>{}(std::forward<FsmT>(fsm), std::forward<TTraits>(ttraits));
//         // fsm_action(std::forward<FsmT>(fsm), std::forward<TTraits>(ttraits));
//     }
// }

// // -Guard, +NextState
// template<typename FsmT, typename TTraits, typename State>
// constexpr inline std::enable_if_t<!detail::has_guard_v<TTraits,Self<FsmT>> &&
//                                   has_next_state_v<TTraits>>
// state_transition_impl(FsmT&& fsm, TTraits&& ttraits, State&& state) noexcept {
//     // fsm_exit(fsm, state);
//     FsmExit<FsmT,State>{}(fsm, state);
//     // fsm_action(fsm, ttraits);
//     FsmAction<FsmT,TTraits>{}(fsm, ttraits);
//     using fsm_statelist = get_state_list_t<std::decay_t<FsmT>>;
//     constexpr auto next_state_idx = state_index_v<fsm_statelist, Next_state<std::decay_t<TTraits>>>;
//     fsm.state(next_state_idx);
//     auto&& next_state = Get<next_state_idx>(fsm);
//     logging::fsm_log_state_change(fsm.self(), state, next_state);
//     fsm_entry(std::forward<FsmT>(fsm), std::forward<decltype(next_state)>(next_state));
// }

// // +Guard, +NextState
// template<typename FsmT, typename TTraits, typename State>
// constexpr inline std::enable_if_t<detail::has_guard_v<TTraits,Self<FsmT>> &&
//                                   has_next_state_v<TTraits>>
// state_transition_impl(FsmT&& fsm, TTraits&& ttraits, State&& state) noexcept {
//     static_assert(detail::is_valid_guard_v<decltype(ttraits.guard), decltype(fsm.self())>);
//     const auto guard_result = ttraits.guard(fsm.self());
//     logging::fsm_log_guard(fsm.self(), ttraits.guard, guard_result);
//     if (guard_result) {
//         // fsm_exit(fsm, state);
//         FsmExit<FsmT,State>{}(fsm, state);
//         // fsm_action(fsm, ttraits);
//         FsmAction<FsmT,TTraits>{}(fsm, ttraits);
//         using fsm_statelist = get_state_list_t<std::decay_t<FsmT>>;
//         constexpr auto next_state_idx = state_index_v<fsm_statelist, Next_state<std::decay_t<TTraits>>>;
//         fsm.state(next_state_idx);
//         auto&& next_state = Get<next_state_idx>(fsm);
//         logging::fsm_log_state_change(fsm.self(), state, next_state);
//         fsm_entry(std::forward<FsmT>(fsm), std::forward<decltype(next_state)>(next_state));
//     }
// }

// template<typename Event, typename FsmT, typename State>
// constexpr inline std::enable_if_t<!detail::HasTraitsFor_v<FsmT, State, Event>>
// state_transition(FsmT&&, State&&) noexcept { /* nop */ }

// template<typename Event, typename FsmT, typename State>
// constexpr inline std::enable_if_t<detail::HasTraitsFor_v<FsmT, State, Event>>
// state_transition(FsmT&& fsm, State&& state) noexcept {
//     using state_t = std::decay_t<State>;
//     using event_t = std::decay_t<Event>;
//     auto&& ttraits = Get_transition_traits<state_t, event_t>(fsm.transition_table());
//     state_transition_impl(std::forward<FsmT>(fsm),
//         std::forward<decltype(ttraits)>(ttraits), std::forward<State>(state));
// }

} // namespace back
} // namespace ufsm
