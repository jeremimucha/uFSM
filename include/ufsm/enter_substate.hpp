#ifndef ENTER_SUBSTATE_HEADER_GUARD_HPP_
#define ENTER_SUBSTATE_HEADER_GUARD_HPP_

#include "traits.hpp"
#include "action.hpp"
#include <utility>
#include <tuple>

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

template<typename NextState, typename TraitsTuple, typename Indices> struct GetSubstateTTraitsIndexT;

template<typename NextState, typename TraitsTuple>
struct GetSubstateTTraitsIndexT<NextState, TraitsTuple, IndexSequence<>>
{
    static constexpr auto value{std::tuple_size_v<TraitsTuple>};
};

template<typename NextStateT, typename TraitsTuple, size_type I, size_type... Is>
struct GetSubstateTTraitsIndexT<NextStateT, TraitsTuple, IndexSequence<I, Is...>>
    : std::conditional_t<std::is_same_v<NextStateT,
                        ufsm::NextState<std::tuple_element_t<I, TraitsTuple>>>,
                        IndexConstant<I>,
                        GetSubstateTTraitsIndexT<NextStateT, TraitsTuple, IndexSequence<Is...>>
                        >
{
};

template<typename NextState, typename TraitsTuple, typename Indices>
constexpr inline auto GetSubstateTTraitsIndex{
    GetSubstateTTraitsIndexT<NextState, TraitsTuple, Indices>::value};

template<typename NextState, std::size_t TraitsTupleSize> struct applySubstateActionImpl;
template<typename NextState> struct applySubstateActionImpl<NextState, 0> {
    template<typename TraitsTuple, typename Subfsm, typename Event>
    constexpr inline void operator()(TraitsTuple&&, Subfsm&&, Event&&) const noexcept
    {
        // std::cerr << __PRETTY_FUNCTION__ << "\n";
        /* nop */
    }
};

template<typename NextState, std::size_t TraitsTupleSize>
struct applySubstateActionImpl {
    template<typename TraitsTuple, typename Subfsm, typename Event>
    constexpr inline void operator()(TraitsTuple&& traits_tuple, Subfsm&& fsm, Event&& event) const noexcept
    {
        // std::cerr << __PRETTY_FUNCTION__ << "\n";
        // using traits_tuple_t = std::decay_t<TraitsTuple>;
        using Indices = MakeIndexSequence<TraitsTupleSize>;
        // constexpr auto idx{GetSubstateTTraitsIndex<NextState, traits_tuple_t, Indices>};
        decltype(auto) ttraits{std::get<0>(std::forward<TraitsTuple>(traits_tuple))};
        fsmAction<Subfsm, Event, std::decay_t<decltype(ttraits)>>{}(
            std::forward<Subfsm>(fsm),
            std::forward<Event>(event),
            std::forward<decltype(ttraits)>(ttraits)
        );
    }
};

template<typename NextState, typename Indices> struct applySubstateAction;

template<typename NextState> struct applySubstateAction<NextState, IndexSequence<>> {
    template<typename Subfsm, typename Event>
    constexpr inline void operator()(Subfsm&&, Event&&) const noexcept
    {
        /* nop */
    }
};

template<typename T> struct show
{
    inline void operator()() const
    {
        std::cerr << __PRETTY_FUNCTION__ << "\n";
    }
};

template<typename NextState, size_type I, size_type... Is>
struct applySubstateAction<NextState, IndexSequence<I, Is...>> {
    template<typename SubfsmT, typename Event>
    constexpr inline void operator()(SubfsmT&& fsm, Event&& event) const noexcept
    {
        // std::cerr << __PRETTY_FUNCTION__ << "\n";
        if (I == fsm.state()) {
            using current_substate_fsm_t = std::decay_t<StateAt<I, std::decay_t<SubfsmT>>>;
            using current_substate_t = detail::BaseFsmState<current_substate_fsm_t>;
            using event_t = std::decay_t<Event>;
            decltype(auto) traits_tuple{
                getTransitionTraits<current_substate_t, event_t>(fsm.transition_table())
            };
            const auto traits_tuple_size{
                std::tuple_size_v<std::decay_t<decltype(traits_tuple)>>
            };
            applySubstateActionImpl<NextState, traits_tuple_size>{}(
                std::forward<decltype(traits_tuple)>(traits_tuple),
                std::forward<SubfsmT>(fsm),
                std::forward<Event>(event)
            );
            return;
        }
        else {
            applySubstateAction<NextState, IndexSequence<Is...>>{}(
                std::forward<SubfsmT>(fsm), std::forward<Event>(event)
            );
        }
    }
};

} // namespace detail

template<typename State, typename TTraits,
         bool = IsFsm<std::decay_t<State>>,
         bool = detail::HasSubstateT<std::decay_t<TTraits>>::value>
struct enterSubstate {
    template<typename State_, typename Event>
    constexpr inline void operator()(State_&&, Event&&) const noexcept
    {
        /* nop */
    }
};

template<typename State, typename TTraits>
struct enterSubstate<State, TTraits, true, true> {
    template<typename Subfsm, typename Event>
    constexpr inline void operator()(Subfsm&& subfsm, Event&& event) const noexcept
    {
        // std::cerr << __PRETTY_FUNCTION__ << "\n";
        using subfsm_t = std::decay_t<Subfsm>;
        using ttraits_t = std::decay_t<TTraits>;
        using event_t = std::decay_t<Event>;
        using subfsm_statelist = GetStateList<subfsm_t>;
        using next_substate_t = detail::GetSubstate<ttraits_t>;
        detail::applySubstateAction<next_substate_t, typename subfsm_t::Indices>{}(
            std::forward<Subfsm>(subfsm),
            std::forward<Event>(event)
        );
        constexpr auto substate_idx = StateIndex<subfsm_statelist, detail::GetSubstate<ttraits_t>>;
        subfsm.state(substate_idx);
    }
};

} // namespace back
} // namespace ufsm

#endif // ENTER_SUBSTATE_HEADER_GUARD_HPP_
