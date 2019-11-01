#pragma once

#include "traits.hpp"
#include "fsmfwd.hpp"
#include "fsm_impl.hpp"
#include "transition_table.hpp"
#include "dispatch_event.hpp"
#include "logging.hpp"
#include "state_index.hpp"
#include "initial_state.hpp"


namespace ufsm
{

template<typename Impl, typename... States>
class Fsm<Impl, Typelist<States...>>
    : public Impl, public back::Fsm_impl<MakeIndexSequence<sizeof...(States)>, States...>
{
    using Base = back::Fsm_impl<MakeIndexSequence<sizeof...(States)>, States...>;
    using Indices = MakeIndexSequence<sizeof...(States)>;
public:
    using InitialState = typename initial_state_or<Impl,
                                                   typename Front<Typelist<States...>>::type
                                                  >::type;
    using EntryPolicy = GetEntryPolicy<Impl>;

    constexpr Fsm() noexcept = default;

    template<typename State>
    constexpr explicit Fsm(initial_state_t<State> init_state) noexcept
        : Base{init_state}
    {
        back::fsmEntry<decltype(back::get<StateIndex<Typelist<States...>,State>>(*this)),
                       decltype(*this),
                       ufsm::InitialTransitionEvent>{}(
            back::get<StateIndex<Typelist<States...>,State>>(*this),
            *this,
            ufsm::InitialTransitionEvent{}
        );
    }

    template<typename State>
    constexpr void set_initial_state(initial_state_t<State> init_state) noexcept
    {
        Base::set_initial_state(init_state);
        back::fsmEntry<decltype(back::get<StateIndex<Typelist<States...>,State>>(*this)),
                       decltype(*this),
                       ufsm::InitialTransitionEvent>{}(
            back::get<StateIndex<Typelist<States...>,State>>(*this),
            *this,
            ufsm::InitialTransitionEvent{}
        );
    }

    // template<typename FsmT, typename Event, SizeT Idx, SizeT... Idxs>
    // friend constexpr inline void
    // ::ufsm::back::dispatch_event(FsmT&& fsm, Event&& event, IndexSequence<Idx,Idxs...>) noexcept;

    template<typename Event>
    constexpr inline void dispatch_event(Event&& event) noexcept
    {
        // ::ufsm::back::dispatch_event(*this, std::forward<Event>(event), Indices{});
        ::ufsm::back::dispatchEvent<Indices>{}(*this, std::forward<Event>(event));
    }

    using Impl::transition_table;
};

} // namespace ufsm
