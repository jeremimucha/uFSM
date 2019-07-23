#ifndef ENTER_SUBSTATE_HEADER_GUARD_HPP_
#define ENTER_SUBSTATE_HEADER_GUARD_HPP_

#include "traits.hpp"

namespace ufsm
{
namespace back
{
namespace detail
{

template<typename T, typename = void_t<>>
struct has_substate : std::false_type { };
template<typename T>
struct has_substate<T, void_t<typename std::decay_t<T>::substate_type>> : std::true_type { };

template<typename T, bool = has_substate<T>::value>
struct get_substate_t { };
template<typename TTraits>
struct get_substate_t<TTraits, true> { using type = typename std::decay_t<TTraits>::substate_type; };
template<typename TTraits>
using get_substate = typename get_substate_t<TTraits>::type;

} // namespace detail

template<typename State, typename TTraits,
         bool = isFsm<std::decay_t<State>>,
         bool = detail::has_substate<std::decay_t<TTraits>>::value>
struct enter_substate {
    template<typename State_, typename TTraits_>
    constexpr inline void operator()(State_&&, TTraits_&&) const noexcept
    {
        /* nop */
    }
};

template<typename State, typename TTraits>
struct enter_substate<State, TTraits, true, true> {
    template<typename Subfsm, typename TTraits_>
    constexpr inline void operator()(Subfsm&& subfsm, TTraits_&&) const noexcept
    {
        using subfsm_t = std::decay_t<Subfsm>;
        using ttraits_t = std::decay_t<TTraits>;
        using subfsm_statelist = get_state_list_t<subfsm_t>;
        constexpr auto substate_idx = state_index_v<subfsm_statelist, detail::get_substate<ttraits_t>>;
        subfsm.state(substate_idx);
    }
};

} // namespace back
} // namespace ufsm

#endif // ENTER_SUBSTATE_HEADER_GUARD_HPP_
