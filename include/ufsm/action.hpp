#pragma once

#include "logging.hpp"
#include "traits.hpp"
#include <type_traits>
#include <utility>

namespace ufsm {
namespace back {
namespace detail {

// template <typename T, typename FsmT, typename Event, typename = void_t<>>
// struct HasActionT : std::false_type {
// };

// template <typename T, typename FsmT, typename Event>
// struct HasActionT<T, FsmT, Event, void_t<decltype(std::declval<T>().action(std::declval<FsmT>(), std::declval<Event>()))>>
//     : std::true_type {
// };

// template <typename T, typename FsmT, typename Event>
// constexpr inline auto HasAction{HasActionT<T, FsmT, Event>::value};

template<typename T, typename = void_t<>, typename... Args>
struct HasActionT : std::false_type { };

template<typename T, typename... Args>
struct HasActionT<T, void_t<decltype(std::declval<T>().action(std::declval<Args>()...))>, Args...>
    : std::true_type { };

template<typename T, typename... Args>
constexpr inline auto HasAction{HasActionT<T, void, Args...>::value};

} // namespace detail

template <typename FsmT_, typename Event_, typename TTraits_,
          bool = detail::HasAction<TTraits_, FsmT_, Event_>>
struct fsmAction {
    template <typename FsmT, typename Event, typename TTraits>
    constexpr inline void operator()(FsmT&&, Event&&, TTraits&&) const noexcept {/* nop */}
};

template <typename FsmT_, typename Event_, typename TTraits_>
struct fsmAction<FsmT_, Event_, TTraits_, true> {
    template <typename FsmT, typename Event, typename TTraits>
    constexpr inline void operator()(FsmT&& fsm, Event&& event, TTraits&& ttraits) const noexcept
    {
        logging::fsm_log_action(fsm, event, ttraits.action);
        std::forward<TTraits>(ttraits).action(std::forward<FsmT>(fsm), std::forward<Event>(event));
    }
};

template <typename FsmT, typename Event, typename TTraits>
constexpr inline void fsm_action(FsmT&& fsm, Event&& event, TTraits&& ttraits) noexcept
{
    using fsm_t = std::decay_t<FsmT>;
    using event_t = std::decay_t<Event>;
    using ttraits_t = std::decay_t<TTraits>;
    fsmAction<fsm_t, event_t, ttraits_t>{}(std::forward<FsmT>(fsm), std::forward<Event>(event), std::forward<TTraits>(ttraits));
}

} // namespace back
} // namespace ufsm
