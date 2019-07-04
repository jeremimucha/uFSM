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
    : public Impl, public back::Fsm_impl<Make_index_sequence<sizeof...(States)>, States...>
{
    using Base = back::Fsm_impl<Make_index_sequence<sizeof...(States)>, States...>;
public:
    using Indices = Make_index_sequence<sizeof...(States)>;
    using InitialState = typename initial_state_or<Impl,
                                                   typename front<typelist<States...>>::type
                                                  >::type;

    constexpr Fsm() noexcept = default;

    template<typename State>
    constexpr explicit Fsm(initial_state<State> init_state) noexcept
        : Base{init_state}
    {
        back::fsm_entry(*this, back::Get<state_index_v<typelist<States...>,State>>(*this));
    }

    template<typename State>
    constexpr void set_initial_state(initial_state<State> init_state) noexcept
    {
        Base::set_initial_state(init_state);
        back::fsm_entry(*this, back::Get<state_index_v<typelist<States...>,State>>(*this));
    }

    template<typename FsmT, typename Event, size_type Idx, size_type... Idxs>
    friend constexpr inline void
    ::ufsm::back::dispatch_event(FsmT&& fsm, Event&& event, Index_sequence<Idx,Idxs...>) noexcept;

    template<typename Event>
    constexpr inline void dispatch_event(Event&& event) noexcept
    {
        ::ufsm::back::dispatch_event(*this, std::forward<Event>(event), Indices{});
    }

    using Impl::transition_table;
    // constexpr decltype(auto) transition_table() const noexcept { return derived().transition_table(); }
    // constexpr decltype(auto) transition_table() noexcept { return derived().transition_table(); }
    // constexpr decltype(auto) self() & noexcept { return *this; }
    // constexpr decltype(auto) self() const& noexcept { return *this; }
    // constexpr decltype(auto) self() && noexcept { return std::move(*this); }
    // constexpr decltype(auto) self() const&& noexcept { return std::move(*this); }
// private:
    // constexpr Derived& derived() & noexcept { return *static_cast<Derived*>(this); }
    // constexpr Derived const& derived() const& noexcept { return *static_cast<Derived const*>(this); }
    // constexpr Derived&& derived() && noexcept { return *static_cast<Derived*>(this); }
};


} // namespace ufsm
