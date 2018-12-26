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
template<typename T, typename FsmT, typename = void_t<>>
struct has_action : std::false_type { };

template<typename T, typename FsmT>
struct has_action<T, FsmT, void_t<decltype(std::declval<T>().action(std::declval<FsmT>()))>>
    : std::true_type
{
};

template<typename T, typename FsmT>
constexpr inline auto has_action_v{has_action<T,FsmT>::value};

template<typename T, typename... Args>
struct is_valid_action : std::is_invocable_r<void, T, Args...> { };
template<typename T, typename... Args>
constexpr inline auto is_valid_action_v{is_valid_action<T,Args...>::value};

} // namespace detail

template<typename FsmT_, typename TTraits_,
         bool HasAction = detail::has_action_v<TTraits_, Self<FsmT_>>>
struct FsmAction {
    template<typename FsmT, typename TTraits>
    constexpr inline void operator()(FsmT&&, TTraits&&) noexcept { }
};

template<typename FsmT_, typename TTraits_>
struct FsmAction<FsmT_, TTraits_, true> {
    template<typename FsmT, typename TTraits>
    constexpr inline void operator()(FsmT&& fsm, TTraits&& ttraits) noexcept
    {
        static_assert(detail::is_valid_action_v<decltype(ttraits.action), decltype(fsm.self())>);
        logging::fsm_log_action(fsm.self(), ttraits.action);
        std::forward<TTraits>(ttraits).action(std::forward<FsmT>(fsm).self());
    }
};

template<typename FsmT, typename TTraits>
constexpr inline void fsm_action(FsmT&& fsm, TTraits&& ttraits) noexcept
{
    using fsm_t = std::decay_t<FsmT>;
    using ttraits_t = std::decay_t<TTraits>;
    FsmAction<fsm_t, ttraits_t>{}(std::forward<FsmT>(fsm), std::forward<TTraits>(ttraits));
}

// template<typename FsmT, typename TTraits>
// constexpr inline std::enable_if_t<detail::has_action_v<TTraits, Self<FsmT>>>
// fsm_action(FsmT&& fsm, TTraits&& ttraits) noexcept
// {
//     static_assert(detail::is_valid_action_v<decltype(ttraits.action), decltype(fsm.self())>);
//     logging::fsm_log_action(fsm.self(), ttraits.action);
//     ttraits.action(std::forward<FsmT>(fsm).self());
// }

// template<typename FsmT, typename TTraits>
// constexpr inline std::enable_if_t<!detail::has_action_v<TTraits, Self<FsmT>>>
// fsm_action(FsmT&&, TTraits&&) noexcept {/* nop */}

} // namespace back
} // namespace ufsm
