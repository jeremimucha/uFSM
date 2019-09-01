#pragma once

#include "traits.hpp"
// #include "fsm_state.hpp"
#include "fsmfwd.hpp"
#include "fsm_impl.hpp"
#include "transition_table.hpp"
#include "dispatch_event.hpp"
// #include "get.hpp"
#include "logging.hpp"


namespace ufsm
{

// template<typename Impl, typename Statelist = detail::get_fsm_state_list_t<Impl>>
// class Fsm;

template<typename Impl, typename... States>
class Fsm<Impl, typelist<States...>>
    : public Impl, public back::Fsm_impl<MakeIndexSequence<sizeof...(States)>, States...>
{
    using Base = back::Fsm_impl<MakeIndexSequence<sizeof...(States)>, States...>;
public:
    // TODO: Make this private and create an accessor trait
    using Indices = MakeIndexSequence<sizeof...(States)>;
    using InitialState = typename initial_state_or<Impl,
                                                   typename front<typelist<States...>>::type
                                                  >::type;
    using EntryPolicy = GetEntryPolicy<Impl>;

    constexpr Fsm() noexcept = default;

    template<typename State>
    constexpr explicit Fsm(initial_state<State> init_state) noexcept
        : Base{init_state}
    {
        // back::fsm_entry(*this, back::Get<StateIndex<typelist<States...>,State>>(*this));
        back::fsmEntry<decltype(*this),
            decltype(back::Get<StateIndex<typelist<States...>,State>>(*this))>{}(
                *this, back::Get<StateIndex<typelist<States...>,State>>(*this));
    }

    template<typename State>
    constexpr void set_initial_state(initial_state<State> init_state) noexcept
    {
        Base::set_initial_state(init_state);
        back::fsm_entry(*this, back::Get<StateIndex<typelist<States...>,State>>(*this));
    }

    template<typename FsmT, typename Event, size_type Idx, size_type... Idxs>
    friend constexpr inline void
    ::ufsm::back::dispatch_event(FsmT&& fsm, Event&& event, IndexSequence<Idx,Idxs...>) noexcept;

    template<typename Event>
    constexpr inline void dispatch_event(Event&& event) noexcept
    {
        ::ufsm::back::dispatch_event(*this, std::forward<Event>(event), Indices{});
    }

    using Impl::transition_table;
};


} // namespace ufsm
