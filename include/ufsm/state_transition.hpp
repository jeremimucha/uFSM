#pragma once

#include <type_traits>
#include <utility>

#include "traits.hpp"
#include "get.hpp"
#include "action.hpp"
#include "transition_guard.hpp"
#include "entry_action.hpp"
#include "exit_action.hpp"
#include "logging.hpp"
#include "any_event.hpp"
#include "traits.hpp"
#include "state_index.hpp"
#include "next_state.hpp"

namespace ufsm
{
namespace back
{
namespace detail
{

struct NoTransitionTraitsCategory { };
struct AnyEventTransitionCategory { };
struct ExactEventTransitionCategory { };

template<typename FsmT, typename State, typename Event>
using get_transition_traits_call = decltype(
    getTransitionTraits<std::decay_t<State>, std::decay_t<Event>>(
        std::declval<FsmT>().transition_table())
    );

template<typename FsmT, typename State, typename = void_t<>>
struct SelectTransitionCategoryAnyT { using type = NoTransitionTraitsCategory; };

template<typename FsmT, typename State>
struct SelectTransitionCategoryAnyT<FsmT, State,
    void_t<get_transition_traits_call<FsmT, State, ufsm::AnyEventT>>> {
    using type = AnyEventTransitionCategory;
};

template<typename FsmT, typename State, typename Event, typename = void_t<>>
struct SelectTransitionCategoryExactT : SelectTransitionCategoryAnyT<FsmT, State> { };

template<typename FsmT, typename State, typename Event>
struct SelectTransitionCategoryExactT<FsmT, State, Event,
    void_t<get_transition_traits_call<FsmT, State, Event>>> {
    using type = ExactEventTransitionCategory;
};

template<typename FsmT, typename State, typename Event>
using SelectTransitionCategory = typename SelectTransitionCategoryExactT<FsmT, State, Event>::type;

} // namespace detail

// -NextState
template<typename FsmT_, typename TTraits_, typename Event_, bool = HasNextState<TTraits_>>
struct stateTransitionImpl {
    template<typename FsmT, typename TTraits, typename State, typename Event>
    constexpr inline bool
    operator()(FsmT&& fsm, TTraits&& traits, State&& state, Event&& event) const noexcept
    {
        if (fsmGuard<FsmT, TTraits, Event>{}(fsm, traits, event)) {
            fsmAction<TTraits, FsmT, Event>{}(
                std::forward<TTraits>(traits), std::forward<FsmT>(fsm),
                std::forward<State>(state), std::forward<Event>(event)
            );
            return true;
        }
        else {
            return false;
        }
    }
};

// +NextState
template<typename FsmT_, typename TTraits_, typename Event_>
struct stateTransitionImpl<FsmT_, TTraits_, Event_, true> {
    template<typename FsmT, typename TTraits, typename State, typename Event>
    constexpr inline bool
    operator()(FsmT&& fsm, TTraits&& ttraits, State&& state, Event&& event) const noexcept
    {
        if (fsmGuard<FsmT, TTraits, Event>{}(fsm, ttraits, event)) {
            fsmExit<State, FsmT, Event>{}(state, fsm, event);
            fsmAction<TTraits, FsmT, Event>{}(ttraits, fsm, state, event);
            using ttraits_t = std::decay_t<TTraits>;
            using fsm_statelist = GetStateList<std::decay_t<FsmT>>;
            constexpr auto next_state_idx = StateIndex<fsm_statelist, NextState<ttraits_t>>;
            fsm.state(next_state_idx);
            auto&& next_state = get<next_state_idx>(fsm);
            logging::fsm_log_state_change(
                fsm, detail::asBaseState(state), detail::asBaseState(next_state)
            );
            fsmEntry<decltype(next_state), FsmT, Event>{}(
                std::forward<decltype(next_state)>(next_state),
                std::forward<FsmT>(fsm),
                std::forward<Event>(event)
            );
            return true;
        }
        else {
            return false;
        }
    }
};

template<typename Indices> struct applyStateTransition;

template<>
struct applyStateTransition<IndexSequence<>> {
    template<typename TraitsTuple, typename FsmT, typename State, typename Event>
    constexpr inline void
    operator()(TraitsTuple&&, FsmT&&, State&&, Event&&) const noexcept
    {
        /* nop */
    }
};

template<SizeT I, SizeT... Is>
struct applyStateTransition<IndexSequence<I, Is...>> {
    template<typename TraitsTuple, typename FsmT, typename State, typename Event>
    constexpr inline void
    operator()(TraitsTuple&& traits, FsmT&& fsm, State&& state, Event&& event) const noexcept
    {
        using fsm_t = std::decay_t<FsmT>;
        using traits_t = std::decay_t<std::tuple_element_t<I, std::decay_t<TraitsTuple>>>;
        using event_t = std::decay_t<Event>;
        if (!stateTransitionImpl<fsm_t, traits_t, event_t>{}(
                std::forward<FsmT>(fsm),
                std::get<I>(std::forward<TraitsTuple>(traits)),
                std::forward<State>(state),
                std::forward<Event>(event)))
        {
            applyStateTransition<IndexSequence<Is...>>{}(
                std::forward<TraitsTuple>(traits),
                std::forward<FsmT>(fsm),
                std::forward<State>(state),
                std::forward<Event>(event)
            );
        }
    }
};

template<typename Event_, typename FsmT_, typename State_,
         typename = detail::SelectTransitionCategory<FsmT_, detail::BaseFsmState<State_>, Event_>>
struct stateTransition {
    template<typename FsmT, typename State, typename Event>
    constexpr inline void operator()(FsmT&&, State&&, Event&&) const noexcept
    {
        /* nop */
    }
};

template<typename Event_, typename FsmT_, typename State_>
struct stateTransition<Event_, FsmT_, State_, detail::AnyEventTransitionCategory> {
    template<typename FsmT, typename State, typename Event>
    constexpr inline void operator()(FsmT&& fsm, State&& state, Event&& event) const noexcept
    {
        using state_t = detail::BaseFsmState<std::decay_t<State>>;
        auto&& ttraitstuple = getTransitionTraits<state_t, ufsm::AnyEventT>(fsm.transition_table());
        using Indices = MakeIndexSequence<std::tuple_size_v<std::decay_t<decltype(ttraitstuple)>>>;
        applyStateTransition<Indices>{}(
            std::forward<decltype(ttraitstuple)>(ttraitstuple),
            std::forward<FsmT>(fsm),
            std::forward<State>(state),
            std::forward<Event>(event));
    }
};

template<typename Event_, typename FsmT_, typename State_>
struct stateTransition<Event_, FsmT_, State_, detail::ExactEventTransitionCategory> {
    template<typename FsmT, typename State, typename Event>
    constexpr inline void operator()(FsmT&& fsm, State&& state, Event&& event) const noexcept
    {
        using state_t = detail::BaseFsmState<std::decay_t<State>>;
        using event_t = std::decay_t<Event>;
        // TODO: Is auto&& ok here? Use decltype(auto) instead?
        auto&& ttraitstuple = getTransitionTraits<state_t, event_t>(fsm.transition_table());
        using Indices = MakeIndexSequence<std::tuple_size_v<std::decay_t<decltype(ttraitstuple)>>>;
        applyStateTransition<Indices>{}(
            std::forward<decltype(ttraitstuple)>(ttraitstuple),
            std::forward<FsmT>(fsm),
            std::forward<State>(state),
            std::forward<Event>(event));
    }
};


} // namespace back
} // namespace ufsm
