#pragma once

#include <type_traits>
#include <utility>

#include "logging.hpp"
#include "traits.hpp"
#include "transition_guard.hpp"
#include "transition_table.hpp"

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

enum class NoTransitionAction { };
enum class TransitionActionNoArgs { };
enum class TransitionActionEvent { };
enum class TransitionActionFsmEvent { };

template<typename TTraits, typename = void_t<>>
struct SelectTransitionActionT { using type = NoTransitionAction; };

template<typename TTraits>
struct SelectTransitionActionT<TTraits, void_t<transition_action_call<TTraits>>>
{
    using type = TransitionActionNoArgs;
};

template<typename TTraits, typename Event, typename = void_t<>>
struct SelectTransitionActionEventT : SelectTransitionActionT<TTraits> { };

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
        propagateActionImpl<GetIndices<FsmT>>{}(
            std::forward<FsmT>(fsm), std::forward<Event>(event)
        );
    }
};

} // namespace detail

template <typename TTraits_, typename FsmT_, typename Event_,
          typename = detail::SelectTransitionActionSignature<TTraits_, FsmT_, Event_>>
struct fsmAction {
    template <typename TTraits, typename FsmT, typename State, typename Event>
    constexpr inline void operator()(TTraits&&, FsmT&&, State&& state, Event&& event) const noexcept
    {
        /* nop */
        detail::propagateAction<std::decay_t<State>>{}(
            std::forward<State>(state),
            std::forward<Event>(event)
        );
    }
};

template <typename TTraits_, typename FsmT_, typename Event_>
struct fsmAction<TTraits_, FsmT_, Event_, detail::TransitionActionNoArgs> {
    template <typename TTraits, typename FsmT, typename State, typename Event>
    constexpr inline void operator()(TTraits&& ttraits, FsmT&& fsm, State&& state, Event&& event) const noexcept
    {
        logging::fsm_log_action(fsm, ttraits.action, event);
        std::forward<TTraits>(ttraits).action();
        detail::propagateAction<std::decay_t<State>>{}(
            std::forward<State>(state),
            std::forward<Event>(event)
        );
    }
};

template <typename TTraits_, typename FsmT_, typename Event_>
struct fsmAction<TTraits_, FsmT_, Event_, detail::TransitionActionEvent> {
    template <typename TTraits, typename FsmT, typename State, typename Event>
    constexpr inline void operator()(TTraits&& ttraits, FsmT&& fsm, State&& state, Event&& event) const noexcept
    {
        logging::fsm_log_action(fsm, ttraits.action, event);
        std::forward<TTraits>(ttraits).action(std::forward<Event>(event));
        detail::propagateAction<std::decay_t<State>>{}(
            std::forward<State>(state),
            std::forward<Event>(event)
        );
    }
};

// This is necessary to allow actions that access states of sub-fsms.
// Simply capturing `this` would require casting it to Fsm<decltype(*this)>
// which is impossible because instantiating (specializing) Fsm evaluates the transiiton_table(),
// which in turn would be recursive
template <typename TTraits_, typename FsmT_, typename Event_>
struct fsmAction<TTraits_, FsmT_, Event_, detail::TransitionActionFsmEvent> {
    template <typename TTraits, typename FsmT, typename State, typename Event>
    constexpr inline void operator()(TTraits&& ttraits, FsmT&& fsm, State&& state, Event&& event) const noexcept
    {
        logging::fsm_log_action(fsm, ttraits.action, event);
        std::forward<TTraits>(ttraits).action(std::forward<FsmT>(fsm), std::forward<Event>(event));
        detail::propagateAction<std::decay_t<State>>{}(
            std::forward<State>(state),
            std::forward<Event>(event)
        );
    }
};

template <typename TTraits, typename FsmT, typename State, typename Event>
constexpr inline void fsm_action(TTraits&& ttraits, FsmT&& fsm, State&& state, Event&& event) noexcept
{
    using fsm_t = std::decay_t<FsmT>;
    using event_t = std::decay_t<Event>;
    using ttraits_t = std::decay_t<TTraits>;
    fsmAction<ttraits_t, fsm_t, event_t>{}(
        std::forward<TTraits>(ttraits), std::forward<FsmT>(fsm),
        std::forward<State>(state), std::forward<Event>(event)
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
            fsmAction<traits_t, FsmT, Event>(
                std::forward<decltype(traits)>(traits),
                std::forward<FsmT>(fsm),
                std::forward<State>(state),
                std::forward<Event>(event)
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
            // auto&& or decltype(auto) ?
            auto&& traits_tuple{
                getTransitionTraits<state_t, event_t>(fsm.transition_table())
            };
            // auto&& or decltype(auto) ?
            auto&& state{get<I>(fsm)};
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
