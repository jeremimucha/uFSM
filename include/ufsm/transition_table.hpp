#pragma once

#include <type_traits>
// #include "fsmfwd.hpp"
#include "traits.hpp"

namespace ufsm
{

template<typename State, typename Event, typename NextState = void,
         typename Guard = void, typename Action = void>
struct TransitionEntry {
    using state_type = State;
    using event_type = Event;
    using next_state = NextState;
    Guard guard{};
    Action action{};
};

template<typename State, typename Event, typename NextState>
struct TransitionEntry<State, Event, NextState, void, void> {
    using state_type = State;
    using event_type = Event;
    using next_state = NextState;
    template<typename Action>
    constexpr inline auto add_action(Action&& action) noexcept
    {
        return TransitionEntry<State,Event,NextState,void,std::decay_t<Action>>{
            std::forward<Action>(action)};
    }
    template<typename Guard>
    constexpr inline auto add_guard(Guard&& guard) noexcept
    {
        return TransitionEntry<State,Event,NextState,std::decay_t<Guard>,void>{
            std::forward<Guard>(guard)};
    }
};

template<typename State, typename Event, typename NextState, typename Guard>
struct TransitionEntry<State, Event, NextState, Guard, void> {
    using state_type = State;
    using event_type = Event;
    using next_state = NextState;
    Guard guard{};
    template<typename Action>
    constexpr inline auto add_action(Action&& action) noexcept
    {
        return TransitionEntry<State,Event,NextState,Guard,std::decay_t<Action>>{
            std::move(guard), std::forward<Action>(action)};
    }
};

template<typename State, typename Event, typename NextState, typename Action>
struct TransitionEntry<State, Event, NextState, void, Action> {
    using state_type = State;
    using event_type = Event;
    using next_state = NextState;
    Action action{};
    template<typename Guard>
    constexpr inline auto add_guard(Guard&& guard) noexcept
    {
        return TransitionEntry<State,Event,NextState,std::decay_t<Guard>,Action>{
            std::forward<Guard>(guard), std::move(action)
        };
    }
};

template<typename State, typename Event, typename Guard, typename Action>
struct TransitionEntry<State, Event, void, Guard, Action>
{
    using state_type = State;
    using event_type = Event;
    Guard guard{};
    Action action{};
};

template<typename State, typename Event, typename Action>
struct TransitionEntry<State, Event, void, void, Action>
{
    using state_type = State;
    using event_type = Event;
    Action action{};
};

// template<typename State, typename Event>
// struct TransitionEntry<State, Event, void, void, void> {

// };


template<typename... Entries>
struct TransitionTraits : public Entries...
{
};

// TODO: It might be better to return by value. Define a traits type for the return type
// return by value if sizeof(TransitionEntry) < 2 * sizeof(double) ?
template<typename State, typename Event, typename NextState, typename Guard, typename Action>
inline constexpr TransitionEntry<State,Event,NextState,Guard,Action> const&
Get_transition_traits(TransitionEntry<State,Event,NextState,Guard,Action> const& te) noexcept
{
    return te;
}

// template<typename State, typename Event, typename... Entries>
// inline constexpr decltype(auto)
// Get_transition_traits(TransitionTraits<Entries...> const& tt) noexcept
// {
//     return Get_transition_traits_impl<State,Event>(tt);
// }

// template<typename State, typename Event, typename FsmT>
// inline constexpr decltype(auto)
// Get_fsm_ttraits(FsmT&& fsm) noexcept
// {
//     return Get_transition_traits<State,Event>(fsm.derived().transition_table);
// }


template<typename... Entries>
constexpr auto make_transition_table(Entries&&... entries) noexcept
{
    return TransitionTraits<Entries...>{std::forward<Entries>(entries)...};
}

namespace detail
{
// template<typename T> struct initial_state_t { using type = T; };
// template<typename T> struct state_t { using type = T; };
template<typename T> struct event_t { using type = T; };
template<typename T> struct next_state_t { using type = T; };
} // namespace detail

// template<typename T> constexpr inline auto initial_state = detail::initial_state_t<T>{};
template<typename T> constexpr inline auto from_state = state_t<T>{};
template<typename T> constexpr inline auto event = detail::event_t<T>{};
template<typename T> constexpr inline auto next_state = detail::next_state_t<T>{};

template<typename T> struct is_initial_state : std::false_type { };
template<typename T>
struct is_initial_state<initial_state_t<T>> : std::true_type { };
template<typename T>
constexpr inline auto is_initial_state_v{is_initial_state<T>::value};

// TODO: Account for the possibility that the user declares multiple initial-states,
// static-assert to inform against the mistake.
// Current implementation also imposes a requirement that the transition table must definie an
// initial_state. Account for the possibility of not declaring the initial_state?
template<typename... States> struct get_initial_state_t;
template<typename T, typename... Ts>
struct get_initial_state_t<initial_state_t<T>, Ts...> { using type = T; };
template<typename T, typename... Ts>
struct get_initial_state_t<T, Ts...> : get_initial_state_t<Ts...> { };

template<typename... Ts>
using get_initial_state = typename get_initial_state_t<Ts...>::type;

template<typename State, typename Event, typename NextState, typename Guard, typename Action>
constexpr auto make_entry(state_t<State>, detail::event_t<Event>,
                          detail::next_state_t<NextState>, Guard guard, Action action) noexcept
{
    return TransitionEntry<State,Event,NextState,Guard,Action>{std::move(guard), std::move(action)};
}

template<typename State, typename Event, typename NextState>
constexpr auto make_entry(state_t<State>, detail::event_t<Event>,
                          detail::next_state_t<NextState>) noexcept
{
    return TransitionEntry<State,Event,NextState,void,void>{};
}

template<typename State, typename Event, typename NextState, typename Guard>
constexpr auto make_gentry(state_t<State>, detail::event_t<Event>,
                           detail::next_state_t<NextState>, Guard guard) noexcept
{
    return TransitionEntry<State,Event,NextState,Guard,void>{std::move(guard)};
}

template<typename State, typename Event, typename NextState, typename Action>
constexpr auto make_aentry(state_t<State>, detail::event_t<Event>,
                           detail::next_state_t<NextState>, Action action) noexcept
{
    return TransitionEntry<State,Event,NextState,void,Action>{std::move(action)};
}

namespace detail
{

template<typename Statelist, typename... Ts> struct build_state_list;

template<template<typename...>class Statelist, typename T, typename... Ts>
struct build_state_list<Statelist<>, T, Ts...> : build_state_list<Statelist<T>, Ts...> { };

// TODO: Warn about a duplicate state transition, rather than silently filtering it out
// and ignoring it
template<template<typename...>class Statelist, typename T, typename... Ts, typename... Ss>
struct build_state_list<Statelist<Ss...>, T, Ts...>
    : std::conditional_t<contains_v<Statelist<Ss...>, T>,
                         build_state_list<Statelist<Ss...>, Ts...>,
                         build_state_list<Statelist<Ss...,T>, Ts...>
                        >
{
};

template<template<typename...>class Statelist, typename... Ss>
struct build_state_list<Statelist<Ss...>>
{
    using type = Statelist<Ss...>;
};

template<typename TTraits> struct fsm_state_list;
template<typename... Ts>
struct fsm_state_list<TransitionTraits<Ts...>>
    : build_state_list<typelist<>, typename Ts::state_type...>
{
};

template<typename TTraits>
using fsm_state_list_t = typename fsm_state_list<TTraits>::type;

template<typename SM>
struct get_fsm_state_list {
    static_assert(has_transition_table_v<SM>,
    "A uFSM client class must implement `constexpr inline auto transition_table()` member function"
    ", which constructs and returns a transition table via ufsm::make_transition_table");
    using type = fsm_state_list_t<decltype(std::declval<SM>().transition_table())>;
};

template<typename SM>
using get_fsm_state_list_t = typename get_fsm_state_list<SM>::type;

} // namespace detail

} // namespace ufsm
