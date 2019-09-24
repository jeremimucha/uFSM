#pragma once

#include "logging.hpp"
#include "traits.hpp"
#include "transition_guard.hpp"
#include "transition_table.hpp"
#include <type_traits>
#include <utility>

namespace ufsm {
namespace back {
namespace detail {

template<typename T, typename = void_t<>, typename... Args>
struct HasActionT : std::false_type { };

template<typename T, typename... Args>
using transition_action_call = decltype(std::declval<T>().action(std::declval<Args>()...));

template<typename T, typename... Args>
struct HasActionT<T, void_t<transition_action_call<T, Args...>>, Args...>
    : std::true_type { };

template<typename T, typename... Args>
constexpr inline auto HasAction{HasActionT<T, void, Args...>::value};

struct NoTransitionAction { };
struct TransitionActionEvent { };
struct TransitionActionFsmEvent { };

template<typename TTraits, typename Event, typename = void_t<>>
struct SelectTransitionActionEventT { using type = NoTransitionAction; };

template<typename TTraits, typename Event>
struct SelectTransitionActionEventT<TTraits, Event,
    void_t<transition_action_call<TTraits, Event>>>
{
    using type = TransitionActionEvent;
};

template<typename TTraits, typename FsmT, typename Event, typename = void_t<>>
struct SelectTransitionActionFsmEventT : SelectTransitionActionEventT<TTraits, Event> { };

template<typename TTraits, typename FsmT, typename Event>
struct SelectTransitionActionFsmEventT<TTraits, FsmT, Event,
    void_t<transition_action_call<TTraits, FsmT, Event>>>
{
    using type = TransitionActionFsmEvent;
};

template<typename TTraits, typename FsmT, typename Event>
using SelectTransitionActionSignature =
    typename SelectTransitionActionFsmEventT<TTraits, FsmT, Event>::type;

template<typename Indices> struct executeAction;

template<typename Indices> struct propagateActionImpl;
template<> struct propagateActionImpl<IndexSequence<>> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&&, Event&&) const noexcept
    {
        /* nop */
    }
};

template<typename State, bool = IsFsm<State>> struct propagateAction {
    template<typename State_, typename Event>
    constexpr inline void operator()(State_&&, Event&&) const noexcept
    {
        /* nop */
    }
};

template<typename State>
struct propagateAction<State, true> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&& fsm, Event&& event) const noexcept
    {
        propagateActionImpl<typename std::decay_t<FsmT>::Indices>{}(
            std::forward<FsmT>(fsm), std::forward<Event>(event)
        );
    }
};

} // namespace detail

template <typename FsmT_, typename Event_, typename TTraits_,
          typename = detail::SelectTransitionActionSignature<TTraits_, FsmT_, Event_>>
struct fsmAction {
    template <typename FsmT, typename Event, typename TTraits, typename State>
    constexpr inline void operator()(FsmT&&, Event&& event, TTraits&&, State&& state) const noexcept
    {
        /* nop */
        detail::propagateAction<std::decay_t<State>>{}(
            std::forward<State>(state),
            std::forward<Event>(event)
        );
    }
};

template <typename FsmT_, typename Event_, typename TTraits_>
struct fsmAction<FsmT_, Event_, TTraits_, detail::TransitionActionEvent> {
    template <typename FsmT, typename Event, typename TTraits, typename State>
    constexpr inline void operator()(FsmT&& fsm, Event&& event, TTraits&& ttraits, State&& state) const noexcept
    {
        logging::fsm_log_action(fsm, ttraits.action, event);
        std::forward<TTraits>(ttraits).action(std::forward<Event>(event));
        detail::propagateAction<std::decay_t<State>>{}(
            std::forward<State>(state),
            std::forward<Event>(event)
        );
    }
};

template <typename FsmT_, typename Event_, typename TTraits_>
struct fsmAction<FsmT_, Event_, TTraits_, detail::TransitionActionFsmEvent> {
    template <typename FsmT, typename Event, typename TTraits, typename State>
    constexpr inline void operator()(FsmT&& fsm, Event&& event, TTraits&& ttraits, State&& state) const noexcept
    {
        logging::fsm_log_action(fsm, ttraits.action, event);
        std::forward<TTraits>(ttraits).action(std::forward<FsmT>(fsm), std::forward<Event>(event));
        detail::propagateAction<std::decay_t<State>>{}(
            std::forward<State>(state),
            std::forward<Event>(event)
        );
    }
};

template <typename FsmT, typename Event, typename TTraits>
constexpr inline void fsm_action(FsmT&& fsm, Event&& event, TTraits&& ttraits) noexcept
{
    using fsm_t = std::decay_t<FsmT>;
    using event_t = std::decay_t<Event>;
    using ttraits_t = std::decay_t<TTraits>;
    fsmAction<fsm_t, event_t, ttraits_t>{}(
        std::forward<FsmT>(fsm), std::forward<Event>(event), std::forward<TTraits>(ttraits)
    );
}

namespace detail
{
template<> struct executeAction<IndexSequence<>> {
    template<typename TraitsTuple, typename FsmT, typename State, typename Event>
    constexpr inline void operator()(TraitsTuple&&, FsmT&&, State&&, Event&&) const noexcept
    {
        /* nop */
    }
};

template<SizeT I, SizeT... Is> struct executeAction<IndexSequence<I, Is...>> {
    template<typename TraitsTuple, typename FsmT, typename State, typename Event>
    constexpr inline void
    operator()(TraitsTuple&& traits_tuple, FsmT&& fsm, State&& state, Event&& event) const noexcept
    {
        using traits_t = std::decay_t<std::tuple_element_t<I, std::decay_t<TraitsTuple>>>;
        decltype(auto) traits{std::get<I>(traits_tuple)};
        if (fsmGuard<FsmT, traits_t, Event>(fsm, traits, event))
        {
            fsmAction<FsmT, Event, traits_t>(
                std::forward<FsmT>(fsm),
                std::forward<Event>(event),
                std::forward<decltype(traits)>(traits)
            );
            // return;
        }
        else {
            executeAction<IndexSequence<Is...>>{}(
                std::forward<TraitsTuple>(traits_tuple),
                std::forward<FsmT>(fsm),
                std::forward<State>(state),
                std::forward<Event>(event)
            );
        }
    }
};

template<SizeT I, SizeT... Is> struct propagateActionImpl<IndexSequence<I, Is...>> {
    template<typename FsmT, typename Event>
    constexpr inline void operator()(FsmT&& fsm, Event&& event) const noexcept
    {
        if (I == fsm.state()) {
            using state_fsm_t = StateAt<I, FsmT>;
            using state_t = BaseFsmState<state_fsm_t>;
            using event_t = std::decay_t<Event>;
            decltype(auto) traits_tuple{
                getTransitionTraits<state_t, event_t>(fsm.transition_table())
            };
            decltype(auto) state{Get<I>(fsm)};
            using Indices = MakeIndexSequence<
                std::tuple_size_v<std::decay_t<decltype(traits_tuple)>>>;
            executeAction<Indices>{}(
                std::forward<decltype(traits_tuple)>(traits_tuple),
                std::forward<FsmT>(fsm),
                std::forward<state_fsm_t>(state),
                std::forward<Event>(event)
            );
            // return;
        }
        else {
            propagateActionImpl<IndexSequence<Is...>>{}(
                std::forward<FsmT>(fsm), std::forward<Event>(event)
            );
        }
    }
};
} // namespace detail


} // namespace back
} // namespace ufsm
