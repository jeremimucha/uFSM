#pragma once

#include <tuple>
#include <type_traits>
// #include "fsmfwd.hpp"
#include "traits.hpp"

namespace ufsm {

template<typename State,
         typename Event,
         typename NextState = void,
         typename Guard = void,
         typename Action = void,
         typename Substate = void>
struct TransitionEntry {
    using state_type = State;
    using event_type = Event;
    using next_state = NextState;
    using substate_type = Substate;
    Guard guard{};
    Action action{};
};

template<typename State, typename Event, typename NextState>
struct TransitionEntry<State, Event, NextState, void, void, void> {
    using state_type = State;
    using event_type = Event;
    using next_state = NextState;
    template<typename Action>
    constexpr inline auto add_action(Action&& action) noexcept
    {
        return TransitionEntry<State, Event, NextState, void, std::decay_t<Action>>{std::forward<Action>(action)};
    }
    template<typename Guard>
    constexpr inline auto add_guard(Guard&& guard) noexcept
    {
        return TransitionEntry<State, Event, NextState, std::decay_t<Guard>, void>{std::forward<Guard>(guard)};
    }
};

template<typename State, typename Event, typename NextState, typename Substate>
struct TransitionEntry<State, Event, NextState, void, void, Substate> {
    using state_type = State;
    using event_type = Event;
    using next_state = NextState;
    using substate_type = Substate;
    template<typename Action>
    constexpr inline auto add_action(Action&& action) noexcept
    {
        return TransitionEntry<State, Event, NextState, void, std::decay_t<Action>>{std::forward<Action>(action)};
    }
    template<typename Guard>
    constexpr inline auto add_guard(Guard&& guard) noexcept
    {
        return TransitionEntry<State, Event, NextState, std::decay_t<Guard>, void>{std::forward<Guard>(guard)};
    }
};

template<typename State, typename Event, typename NextState, typename Guard>
struct TransitionEntry<State, Event, NextState, Guard, void, void> {
    using state_type = State;
    using event_type = Event;
    using next_state = NextState;
    Guard guard{};
    template<typename Action>
    constexpr inline auto add_action(Action&& action) noexcept
    {
        return TransitionEntry<State, Event, NextState, Guard, std::decay_t<Action>>{std::move(guard),
                                                                                     std::forward<Action>(action)};
    }
};

template<typename State, typename Event, typename NextState, typename Guard, typename Substate>
struct TransitionEntry<State, Event, NextState, Guard, void, Substate> {
    using state_type = State;
    using event_type = Event;
    using next_state = NextState;
    using substate_type = Substate;
    Guard guard{};
    template<typename Action>
    constexpr inline auto add_action(Action&& action) noexcept
    {
        return TransitionEntry<State, Event, NextState, Guard, std::decay_t<Action>>{std::move(guard),
                                                                                     std::forward<Action>(action)};
    }
};

template<typename State, typename Event, typename NextState, typename Action>
struct TransitionEntry<State, Event, NextState, void, Action, void> {
    using state_type = State;
    using event_type = Event;
    using next_state = NextState;
    Action action{};
    template<typename Guard>
    constexpr inline auto add_guard(Guard&& guard) noexcept
    {
        return TransitionEntry<State, Event, NextState, std::decay_t<Guard>, Action>{std::forward<Guard>(guard),
                                                                                     std::move(action)};
    }
};

template<typename State, typename Event, typename NextState, typename Action, typename Substate>
struct TransitionEntry<State, Event, NextState, void, Action, Substate> {
    using state_type = State;
    using event_type = Event;
    using next_state = NextState;
    using substate_type = Substate;
    Action action{};
    template<typename Guard>
    constexpr inline auto add_guard(Guard&& guard) noexcept
    {
        return TransitionEntry<State, Event, NextState, std::decay_t<Guard>, Action>{std::forward<Guard>(guard),
                                                                                     std::move(action)};
    }
};

template<typename State, typename Event, typename Guard, typename Action>
struct TransitionEntry<State, Event, void, Guard, Action, void> {
    using state_type = State;
    using event_type = Event;
    Guard guard{};
    Action action{};
};

template<typename State, typename Event, typename Action>
struct TransitionEntry<State, Event, void, void, Action, void> {
    using state_type = State;
    using event_type = Event;
    Action action{};
};

// template<typename State, typename Event>
// struct TransitionEntry<State, Event, void, void, void> {

// };


template<typename... Entries>
struct TransitionTraits : public Entries... { };

// TODO: It might be better to return by value. Define a traits type for the return type
// return by value if sizeof(TransitionEntry) < 2 * sizeof(double) ?
template<typename State, typename Event, typename NextState, typename Guard, typename Action, typename Substate>
inline constexpr TransitionEntry<State, Event, NextState, Guard, Action, Substate> const&
  Get_transition_traits(TransitionEntry<State, Event, NextState, Guard, Action, Substate> const& te) noexcept
{
    return te;
}

template<typename State, typename Event, typename Entry>
struct IsMatchingEntryT : std::false_type { };
template<typename State, typename Event, template<typename...> class Entry, typename... Ts>
struct IsMatchingEntryT<State, Event, Entry<State, Event, Ts...>> : std::true_type { };

template<typename State, typename Event, typename Entry>
constexpr inline auto IsMatchingEntry{IsMatchingEntryT<State, Event, Entry>::value};

template<typename List, typename State, typename Event, typename... Entries>
struct GetTtraitsListImplT;

template<template<typename...> class List, typename State, typename Event, typename... Es>
struct GetTtraitsListImplT<List<Es...>, State, Event> {
    using type = List<Es...>;
};

template<template<typename...> class List,
         typename State,
         typename Event,
         typename Entry,
         typename... Entries,
         typename... Es>
struct GetTtraitsListImplT<List<Es...>, State, Event, Entry, Entries...>
  : std::conditional_t<IsMatchingEntry<State, Event, Entry>,
                       GetTtraitsListImplT<List<Es..., Entry>, State, Event, Entries...>,
                       GetTtraitsListImplT<List<Es...>, State, Event, Entries...>> { };

template<typename State, typename Event, typename... Entries>
struct GetTtraitsListT : GetTtraitsListImplT<Typelist<>, State, Event, Entries...> { };

template<typename State, typename Event, typename... Entries>
using GetTtraitsList = typename GetTtraitsListT<State, Event, Entries...>::type;

template<typename Entry>
struct asTransitionEntry;

template<template<typename...> class Entry,
         typename State,
         typename Event,
         typename NextState,
         typename Guard,
         typename Action,
         typename Substate>
struct asTransitionEntry<Entry<State, Event, NextState, Guard, Action, Substate>> {
    constexpr inline decltype(auto)
      operator()(TransitionEntry<State, Event, NextState, Guard, Action, Substate> const& te) const noexcept
    {
        return te;
    }
};

template<typename TtraitsList>
struct getTransitionTraitsImpl;
template<template<typename...> class List, typename... Entries>
struct getTransitionTraitsImpl<List<Entries...>> {
    template<typename TTraits>
    inline constexpr auto operator()(TTraits const& ttraits) const noexcept
    {
        // std::cerr << __PRETTY_FUNCTION__ << "\n";
        return std::make_tuple(asTransitionEntry<Entries>{}(ttraits)...);
    }
};

template<typename State, typename Event, typename... Entries>
inline constexpr auto getTransitionTraits(TransitionTraits<Entries...>&& ttraits) noexcept
{
    using TtraitsList = GetTtraitsList<State, Event, Entries...>;
    return getTransitionTraitsImpl<TtraitsList>{}(ttraits);
}

template<typename... Entries>
constexpr auto make_transition_table(Entries&&... entries) noexcept
{
    return TransitionTraits<Entries...>{std::forward<Entries>(entries)...};
}

namespace detail {
template<typename T>
struct state_t {
    using type = T;
};
template<typename T>
struct event_t {
    using type = T;
};
template<typename T>
struct next_state_t {
    using type = T;
};
template<typename T>
struct substate_t {
    using type = T;
};
}  // namespace detail

template<typename T>
constexpr inline auto ufsm_from_state = detail::state_t<T>{};
template<typename T>
constexpr inline auto ufsm_event = detail::event_t<T>{};
template<typename T>
constexpr inline auto ufsm_next_state = detail::next_state_t<T>{};
template<typename T>
constexpr inline auto usfm_substate = detail::substate_t<T>{};


template<typename State, typename Event, typename NextState, typename Guard, typename Action>
constexpr auto make_entry(detail::state_t<State>,
                          detail::event_t<Event>,
                          detail::next_state_t<NextState>,
                          Guard guard,
                          Action action) noexcept
{
    return TransitionEntry<State, Event, NextState, Guard, Action>{std::move(guard), std::move(action)};
}

template<typename State, typename Event, typename NextState>
constexpr auto make_entry(detail::state_t<State>, detail::event_t<Event>, detail::next_state_t<NextState>) noexcept
{
    return TransitionEntry<State, Event, NextState, void, void>{};
}

template<typename State, typename Event, typename Guard, typename Action>
constexpr auto make_entry(detail::state_t<State>, detail::event_t<Event>, Guard guard, Action action) noexcept
{
    return TransitionEntry<State, Event, void, Guard, Action>{std::move(guard), std::move(action)};
}

template<typename State, typename Event, typename NextState, typename Guard>
constexpr auto
  make_gentry(detail::state_t<State>, detail::event_t<Event>, detail::next_state_t<NextState>, Guard guard) noexcept
{
    return TransitionEntry<State, Event, NextState, Guard, void>{std::move(guard)};
}

template<typename State, typename Event, typename Guard>
constexpr auto make_gentry(detail::state_t<State>, detail::event_t<Event>, Guard guard) noexcept
{
    return TransitionEntry<State, Event, void, Guard, void>{std::move(guard)};
}

template<typename State, typename Event, typename NextState, typename Action>
constexpr auto
  make_aentry(detail::state_t<State>, detail::event_t<Event>, detail::next_state_t<NextState>, Action action) noexcept
{
    return TransitionEntry<State, Event, NextState, void, Action>{std::move(action)};
}

template<typename State, typename Event, typename Action>
constexpr auto make_aentry(detail::state_t<State>, detail::event_t<Event>, Action action) noexcept
{
    return TransitionEntry<State, Event, void, void, Action>{std::move(action)};
}

template<typename State, typename Event, typename NextState, typename Substate, typename Guard, typename Action>
constexpr auto make_entry(detail::state_t<State>,
                          detail::event_t<Event>,
                          detail::next_state_t<NextState>,
                          detail::substate_t<Substate>,
                          Guard guard,
                          Action action) noexcept
{
    return TransitionEntry<State, Event, NextState, Guard, Action, Substate>{std::move(guard), std::move(action)};
}

template<typename State, typename Event, typename NextState, typename Substate>
constexpr auto make_entry(detail::state_t<State>,
                          detail::event_t<Event>,
                          detail::next_state_t<NextState>,
                          detail::substate_t<Substate>) noexcept
{
    return TransitionEntry<State, Event, NextState, void, void, Substate>{};
}

template<typename State, typename Event, typename NextState, typename Substate, typename Guard>
constexpr auto make_gentry(detail::state_t<State>,
                           detail::event_t<Event>,
                           detail::next_state_t<NextState>,
                           detail::substate_t<Substate>,
                           Guard guard) noexcept
{
    return TransitionEntry<State, Event, NextState, Guard, void, Substate>{std::move(guard)};
}

template<typename State, typename Event, typename NextState, typename Substate, typename Action>
constexpr auto make_aentry(detail::state_t<State>,
                           detail::event_t<Event>,
                           detail::next_state_t<NextState>,
                           detail::substate_t<Substate>,
                           Action action) noexcept
{
    return TransitionEntry<State, Event, NextState, void, Action, Substate>{std::move(action)};
}

namespace detail {

template<typename Statelist, typename... Ts>
struct build_state_list;

template<template<typename...> class Statelist, typename T, typename... Ts>
struct build_state_list<Statelist<>, T, Ts...> : build_state_list<Statelist<T>, Ts...> { };

// TODO: Warn about a duplicate state transition, rather than silently filtering it out
// and ignoring it
template<template<typename...> class Statelist, typename T, typename... Ts, typename... Ss>
struct build_state_list<Statelist<Ss...>, T, Ts...>
  : std::conditional_t<Contains<Statelist<Ss...>, T>,
                       build_state_list<Statelist<Ss...>, Ts...>,
                       build_state_list<Statelist<Ss..., T>, Ts...>> { };

template<template<typename...> class Statelist, typename... Ss>
struct build_state_list<Statelist<Ss...>> {
    using type = Statelist<Ss...>;
};

template<typename TTraits>
struct fsm_state_list;
template<typename... Ts>
struct fsm_state_list<TransitionTraits<Ts...>> : build_state_list<Typelist<>, typename Ts::state_type...> { };

template<typename TTraits>
using fsm_state_list_t = typename fsm_state_list<TTraits>::type;

template<typename SM>
struct get_fsm_state_list {
    static_assert(HasTransitionTable<SM>,
                  "A uFSM client class must implement `constexpr inline auto transition_table()` member function"
                  ", which constructs and returns a transition table via ufsm::make_transition_table");
    using type = fsm_state_list_t<decltype(std::declval<SM>().transition_table())>;
};

template<typename SM>
using get_fsm_state_list_t = typename get_fsm_state_list<SM>::type;

}  // namespace detail

}  // namespace ufsm
