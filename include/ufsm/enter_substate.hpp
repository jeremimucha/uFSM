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
struct HasSubstateT : std::false_type { };
template<typename T>
struct HasSubstateT<T, void_t<typename std::decay_t<T>::substate_type>> : std::true_type { };

template<typename T, bool = HasSubstateT<T>::value>
struct GetSubstateT { };
template<typename TTraits>
struct GetSubstateT<TTraits, true> { using type = typename std::decay_t<TTraits>::substate_type; };
template<typename TTraits>
using GetSubstate = typename GetSubstateT<TTraits>::type;

} // namespace detail

template<typename State, typename TTraits,
         bool = IsFsm<std::decay_t<State>>,
         bool = detail::HasSubstateT<std::decay_t<TTraits>>::value>
struct enterSubstate {
    template<typename State_, typename TTraits_>
    constexpr inline void operator()(State_&&, TTraits_&&) const noexcept
    {
        /* nop */
    }
};

template<typename State, typename TTraits>
struct enterSubstate<State, TTraits, true, true> {
    template<typename Subfsm, typename TTraits_>
    constexpr inline void operator()(Subfsm&& subfsm, TTraits_&&) const noexcept
    {
        using subfsm_t = std::decay_t<Subfsm>;
        using ttraits_t = std::decay_t<TTraits>;
        using subfsm_statelist = GetStateList<subfsm_t>;
        constexpr auto substate_idx = StateIndex<subfsm_statelist, detail::GetSubstate<ttraits_t>>;
        subfsm.state(substate_idx);
    }
};

} // namespace back
} // namespace ufsm

#endif // ENTER_SUBSTATE_HEADER_GUARD_HPP_
