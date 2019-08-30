#pragma once

#include <type_traits>
#include <utility>
#include "traits.hpp"
#include "get.hpp"
#include "action.hpp"
#include "entry_action.hpp"
#include "exit_action.hpp"
#include "logging.hpp"
#include "enter_substate.hpp"


namespace ufsm
{
namespace back
{
namespace detail
{

template<typename T, typename Void, typename... Args>
struct HasGuardT : std::false_type { };

template<typename T, typename... Args>
struct HasGuardT<T,
                 void_t<decltype(std::declval<T>().guard(std::declval<Args>()...))>,
                 Args...>
: std::true_type { };

template<typename T, typename... Args>
constexpr inline auto HasGuard{HasGuardT<T, void, Args...>::value};

struct NoTransitionTraitsTag : IndexConstant<0> { };
struct AnyTransitionTraitsTag : IndexConstant<1> { };
struct ExactTransitionTraitsTag : IndexConstant<2> { };

constexpr inline auto NoTransitionTraits{NoTransitionTraitsTag::value};
constexpr inline auto AnyTransitionTraits{AnyTransitionTraitsTag::value};
constexpr inline auto ExactTransitionTraits{ExactTransitionTraitsTag::value};

template<typename FsmT, typename State, typename = void_t<>>
struct HasTraitsForAnyEvent : NoTransitionTraitsTag { };

template<typename FsmT, typename State>
struct HasTraitsForAnyEvent<FsmT, State,
    void_t<decltype(
        Get_transition_traits<std::decay_t<State>, ufsm::AnyEvent_t>(
            std::declval<FsmT>().transition_table()))>>
    : AnyTransitionTraitsTag
{
};

template<typename FsmT, typename State, typename Event, typename = void_t<>>
struct HasTraitsFor : HasTraitsForAnyEvent<FsmT, State> { };

template<typename FsmT,typename State, typename Event>
struct HasTraitsFor<FsmT, State, Event,
    void_t<decltype(
        Get_transition_traits<std::decay_t<State>,std::decay_t<Event>>(
            std::declval<FsmT>().transition_table()))>>
    : ExactTransitionTraitsTag
{
};
template<typename FsmT, typename State, typename Event>
constexpr inline auto HasTraitsFor_v = HasTraitsFor<FsmT,State,Event>::value;

// Alternative approach:
// ------------------------------------------------------------------------------------------------
struct NoTransitionTraitsForEvent { };
struct TransitionTraitsForAnyEvent { };
struct TransitionTraitsForExactEvent { };

template<typename FsmT, typename State, typename = void_t<>>
struct SelectTransitionCategoryAnyT { using type = NoTransitionTraitsForEvent; };

template<typename FsmT, typename State>
struct SelectTransitionCategoryAnyT<FsmT, State,
    void_t<decltype(
        Get_transition_traits<std::decay_t<State>, ufsm::AnyEvent_t>(
            std::declval<FsmT>().transition_table()))>>
{
    using type = TransitionTraitsForAnyEvent;
};

template<typename FsmT, typename State, typename Event, typename = void_t<>>
struct SelectTransitionCategoryT : SelectTransitionCategoryAnyT<FsmT, State> { };

template<typename FsmT, typename State, typename Event>
struct SelectTransitionCategoryT<FsmT, State, Event,
    void_t<decltype(
        Get_transition_traits<std::decay_t<State>,std::decay_t<Event>>(
            std::declval<FsmT>().transition_table()))>>
{
    using type = TransitionTraitsForExactEvent;
};

template<typename FsmT, typename State, typename Event>
using SelectTransitionCategory = typename SelectTransitionCategoryT<FsmT, State, Event>::type;
// ------------------------------------------------------------------------------------------------

} // namespace detail

template<typename FsmT_, typename TTraits_, typename Event_,
         bool = detail::HasGuard<TTraits_, Event_>>
struct fsmGuard {
    template<typename FsmT, typename TTraits, typename Event>
    constexpr inline bool operator()(FsmT&&, TTraits&&, Event&&) const noexcept
    {
        return true;
    }
};

template<typename FsmT_, typename TTraits_, typename Event_>
struct fsmGuard<FsmT_, TTraits_, Event_, true> {
    template<typename FsmT, typename TTraits, typename Event>
    constexpr inline bool operator()(FsmT&& fsm, TTraits&& ttraits, Event&& event) const noexcept
    {
        auto const result = ttraits.guard(event);
        logging::fsm_log_guard(fsm, ttraits.guard, result);
        return result;
    }
};

// -NextState
template<typename FsmT_, typename TTraits_, typename Event_,
         bool = HasNextState<TTraits_>>
struct stateTransitionImpl {
    template<typename FsmT, typename TTraits, typename State, typename Event>
    constexpr inline void operator()(FsmT&& fsm, TTraits&& traits, State&&, Event&& event) noexcept
    {
        if (fsmGuard<FsmT, TTraits, Event>{}(fsm, traits, event)) {
            fsmAction<FsmT, Event, TTraits>{}(
                std::forward<FsmT>(fsm), std::forward<Event>(event), std::forward<TTraits>(traits));
        }
    }
};

// +NextState
template<typename FsmT_, typename TTraits_, typename Event_>
struct stateTransitionImpl<FsmT_, TTraits_, Event_, true> {
    template<typename FsmT, typename TTraits, typename State, typename Event>
    constexpr inline void operator()(FsmT&& fsm, TTraits&& ttraits, State&& state, Event&& event) noexcept
    {
        if (fsmGuard<FsmT, TTraits, Event>{}(fsm, ttraits, event)) {
            fsmExit<FsmT, State>{}(fsm, state);
            fsmAction<FsmT, Event, TTraits>{}(fsm, event, ttraits);
            using ttraits_t = std::decay_t<TTraits>;
            using fsm_statelist = GetStateList<std::decay_t<FsmT>>;
            constexpr auto next_state_idx = StateIndex<fsm_statelist, NextState<ttraits_t>>;
            fsm.state(next_state_idx);
            auto&& next_state = Get<next_state_idx>(fsm);
            using next_state_t = std::decay_t<decltype(next_state)>;
            enterSubstate<next_state_t, ttraits_t>{}(next_state, ttraits);
            logging::fsm_log_state_change(fsm, detail::asBaseState(state), next_state);
            fsmEntry<FsmT, decltype(next_state)>{}(std::forward<FsmT>(fsm), std::forward<decltype(next_state)>(next_state));
        }
    }
};

template<typename Event_, typename FsmT_, typename State_,
         auto = detail::HasTraitsFor_v<FsmT_, detail::BaseFsmState<State_>, Event_>>
struct stateTransition {
    template<typename FsmT, typename State, typename Event>
    constexpr inline void operator()(FsmT&&, State&&, Event&&) noexcept
    {
        /* nop */
    }
};

template<typename Event_, typename FsmT_, typename State_>
struct stateTransition<Event_, FsmT_, State_, detail::AnyTransitionTraits> {
    template<typename FsmT, typename State, typename Event>
    constexpr inline void operator()(FsmT&& fsm, State&& state, Event&& event) noexcept
    {
        using state_t = detail::BaseFsmState<std::decay_t<State>>;
        using event_t = std::decay_t<Event>;
        auto&& ttraits = Get_transition_traits<state_t, ufsm::AnyEvent_t>(fsm.transition_table());
        stateTransitionImpl<std::decay_t<FsmT>, std::decay_t<decltype(ttraits)>, event_t>{}(
            std::forward<FsmT>(fsm), std::forward<decltype(ttraits)>(ttraits),
            std::forward<State>(state), std::forward<Event>(event)
        );
    }
};

template<typename Event_, typename FsmT_, typename State_>
struct stateTransition<Event_, FsmT_, State_, detail::ExactTransitionTraits> {
    template<typename FsmT, typename State, typename Event>
    constexpr inline void operator()(FsmT&& fsm, State&& state, Event&& event) noexcept
    {
        // std::cerr << "First we're here\n";
        using state_t = detail::BaseFsmState<std::decay_t<State>>;
        using event_t = std::decay_t<Event>;
        // TODO: Is auto&& ok here? Use decltype(auto) instead?
        auto&& ttraits = Get_transition_traits<state_t, event_t>(fsm.transition_table());
        stateTransitionImpl<std::decay_t<FsmT>, std::decay_t<decltype(ttraits)>, event_t>{}(
            std::forward<FsmT>(fsm), std::forward<decltype(ttraits)>(ttraits),
            std::forward<State>(state), std::forward<Event>(event)
        );
    }
};

// Alternative approach - specialize on type, rather than an integral value
// ------------------------------------------------------------------------------------------------
// template<typename Event, typename FsmT_, typename State_,
//          typename = detail::SelectTransitionCategory<FsmT_, State_, Event>>
// struct stateTransition {
//     template<typename FsmT, typename State>
//     constexpr inline void operator()(FsmT&&, State&&) noexcept
//     {
//         /* nop */
//     }
// };

// template<typename Event, typename FsmT_, typename State_>
// struct stateTransition<Event, FsmT_, State_, detail::TransitionTraitsForAnyEvent> {
//     template<typename FsmT, typename State>
//     constexpr inline void operator()(FsmT&& fsm, State&& state) noexcept
//     {
//         using state_t = std::decay_t<State>;
//         using event_t = ufsm::AnyEvent_t;
//         auto&& ttraits = Get_transition_traits<state_t, event_t>(fsm.transition_table());
//         stateTransitionImpl<std::decay_t<FsmT>, std::decay_t<decltype(ttraits)>>{}(
//             std::forward<FsmT>(fsm), std::forward<decltype(ttraits)>(ttraits),
//             std::forward<State>(state));
//     }
// };

// template<typename Event, typename FsmT_, typename State_>
// struct stateTransition<Event, FsmT_, State_, detail::TransitionTraitsForExactEvent> {
//     template<typename FsmT, typename State>
//     constexpr inline void operator()(FsmT&& fsm, State&& state) noexcept
//     {
//         using state_t = std::decay_t<State>;
//         using event_t = std::decay_t<Event>;
//         auto&& ttraits = Get_transition_traits<state_t, event_t>(fsm.transition_table());
//         stateTransitionImpl<std::decay_t<FsmT>, std::decay_t<decltype(ttraits)>>{}(
//             std::forward<FsmT>(fsm), std::forward<decltype(ttraits)>(ttraits),
//             std::forward<State>(state));
//     }
// };
// ------------------------------------------------------------------------------------------------

} // namespace back
} // namespace ufsm
