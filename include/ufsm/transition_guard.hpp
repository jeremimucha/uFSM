#if !defined(UFSM_TRANSITION_GUARD_HPP)
#define UFSM_TRANSITION_GUARD_HPP

#include "traits.hpp"

namespace ufsm
{
namespace back
{
namespace detail
{
template<typename T, typename Void, typename... Args>
struct HasGuardT : std::false_type { };

template<typename T, typename... Args>
using transition_guard_call = decltype(std::declval<T>().guard(std::declval<Args>()...));

template<typename T, typename... Args>
struct HasGuardT<T, void_t<transition_guard_call<T, Args...>>, Args...>
: std::true_type { };

template<typename T, typename... Args>
constexpr inline auto HasGuard{HasGuardT<T, void, Args...>::value};

struct NoTransitionGuard { };
struct TransitionGuardNoArgs { };
struct TransitionGuardEvent { };
struct TransitionGuardFsmEvent { };

template<typename TTraits, typename = void_t<>>
struct SelectTransitionGuardT { using type = NoTransitionGuard; };

template<typename TTraits>
struct SelectTransitionGuardT<TTraits, void_t<transition_guard_call<TTraits>>> {
    using type = TransitionGuardNoArgs;
};

template<typename TTraits, typename Event, typename = void_t<>>
struct SelectTransitionGuardEventT : SelectTransitionGuardT<TTraits> { };

template<typename TTraits, typename Event>
struct SelectTransitionGuardEventT<TTraits, Event, void_t<transition_guard_call<TTraits, Event>>>
{
    using type = TransitionGuardEvent;
};

template<typename TTraits, typename FsmT, typename Event, typename = void_t<>>
struct SelectTransitionGuardFsmEventT : SelectTransitionGuardEventT<TTraits, Event> { };

template<typename TTraits, typename FsmT, typename Event>
struct SelectTransitionGuardFsmEventT<TTraits, FsmT, Event,
    void_t<transition_guard_call<TTraits, FsmT, Event>>>
{
    using type = TransitionGuardFsmEvent;
};

template<typename TTraits, typename FsmT, typename Event>
using SelectTransitionGuardSignature =
    typename SelectTransitionGuardFsmEventT<TTraits, FsmT, Event>::type;

} // namespace detail

template<typename FsmT_, typename TTraits_, typename Event_,
         typename = detail::SelectTransitionGuardSignature<TTraits_, FsmT_, Event_>>
struct fsmGuard {
    template<typename FsmT, typename TTraits, typename Event>
    constexpr inline bool operator()(FsmT&&, TTraits&&, Event&&) const noexcept
    {
        return true;
    }
};

template<typename FsmT_, typename TTraits_, typename Event_>
struct fsmGuard<FsmT_, TTraits_, Event_, detail::TransitionGuardNoArgs> {
    template<typename FsmT, typename TTraits, typename Event>
    constexpr inline bool operator()(FsmT&& fsm, TTraits&& ttraits, Event&& event) const noexcept
    {
        auto const result = ttraits.guard();
        logging::fsm_log_guard(fsm, ttraits.guard, result);
        return result;
    }
};

template<typename FsmT_, typename TTraits_, typename Event_>
struct fsmGuard<FsmT_, TTraits_, Event_, detail::TransitionGuardEvent> {
    template<typename FsmT, typename TTraits, typename Event>
    constexpr inline bool operator()(FsmT&& fsm, TTraits&& ttraits, Event&& event) const noexcept
    {
        auto const result = ttraits.guard(event);
        logging::fsm_log_guard(fsm, ttraits.guard, result);
        return result;
    }
};

template<typename FsmT_, typename TTraits_, typename Event_>
struct fsmGuard<FsmT_, TTraits_, Event_, detail::TransitionGuardFsmEvent> {
    template<typename FsmT, typename TTraits, typename Event>
    constexpr inline bool operator()(FsmT&& fsm, TTraits&& ttraits, Event&& event) const noexcept
    {
        auto const result = ttraits.guard(fsm, event);
        logging::fsm_log_guard(fsm, ttraits.guard, result);
        return result;
    }
};

} // namespace back

} // namespace ufsm

#endif // UFSM_TRANSITION_GUARD_HPP
