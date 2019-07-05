#pragma once

#include <type_traits>
#include <utility>
// #include "transition_table.hpp"
#include "fsmfwd.hpp"


namespace ufsm
{

using size_type = int;

template<size_type Idx>
using Index_constant = std::integral_constant<size_type, Idx>;

template<size_type... Idxs>
using Index_sequence = std::integer_sequence<size_type, Idxs...>;

template<size_type N>
using Make_index_sequence = std::make_integer_sequence<size_type, N>;

template<typename... Ts>
using Index_sequence_for = Make_index_sequence<sizeof...(Ts)>;

template<typename...> struct typelist { };
template<typename...> using void_t = void;
template<typename T> struct identity { using type = T; };
template<typename...> struct always_false : std::false_type { };
template<typename... Ts> constexpr inline auto always_false_v{always_false<Ts...>::value};
template<typename Typelist> struct front;
template<template<typename...> class List, class T, class... Ts>
struct front<List<T, Ts...>> { using type = T; };
template<template<typename...> class List>
struct front<List<>> { };

template<typename State> struct isFsmT : std::false_type { };
template<typename Impl, typename States>
struct isFsmT<::ufsm::Fsm<Impl, States>> : std::true_type { };
template<typename State>
constexpr inline auto isFsm{isFsmT<State>::value};

struct AnyEvent_t { };
constexpr inline AnyEvent_t AnyEvent{};

template<typename C, typename T> struct contains_impl;

template<template<class...> class C, typename T, typename... Us>
struct contains_impl<C<Us...>, T> : std::disjunction<std::is_same<T, Us>...> { };

template<typename C, typename T>
struct contains_t : contains_impl<C,T> { };

template<typename C, typename T>
static constexpr inline auto contains_v{contains_impl<C,T>::value};

template<typename T> struct initial_state { };
template<typename T> constexpr inline auto initial_state_v{initial_state<T>{}};

template<typename T, typename = void_t<>> struct has_initial_state : std::false_type { };
template<typename T>
struct has_initial_state<T, void_t<typename T::InitialState>> : std::true_type { };
template<typename T>
constexpr inline auto has_initial_state_v{has_initial_state<T>::value};

template<typename T, typename U, bool = has_initial_state<T>::value> struct initial_state_or {
    using type = U;
};
template<typename T, typename U>
struct initial_state_or<T, U, true> { using type = typename T::InitialState; };

template<typename T> struct get_initial_state {
    using type = typename T::initial_state;
};
// template<typename T> struct get_initial_state<T, false> {
//     using type = typename front<decltype(std::declval<T>().transition_table())>::type;
// };
template<typename FsmT, typename = void_t<>> struct hasEntryPolicyT : std::false_type { };
template<typename FsmT>
struct hasEntryPolicyT<FsmT, void_t<typename FsmT::EntryPolicy>> : std::true_type { };
template<typename FsmT>
constexpr inline auto hasEntryPolicy{hasEntryPolicyT<FsmT>::value};

struct InitialStateEntryPolicy { };
struct CurrentStateEntryPolicy { };

template<typename FsmT, bool = hasEntryPolicy<std::decay_t<FsmT>>>
struct get_entry_policy {
    using type = InitialStateEntryPolicy;
};
template<typename FsmT>
struct get_entry_policy<FsmT, true> {
    using type = typename FsmT::EntryPolicy;
};

template<typename FsmT, typename = void_t<>> struct has_transition_table : std::false_type { };
template<typename FsmT>
struct has_transition_table<FsmT, void_t<decltype(std::declval<FsmT>().transition_table())>>
    : std::true_type
{
};
template<typename FsmT>
constexpr inline auto has_transition_table_v{has_transition_table<FsmT>::value};

template<typename State, bool = has_transition_table_v<State>>
struct state_traits {
    using state_type = State;
};

template<typename State>
struct state_traits<State, true> {
    using state_type = ufsm::Fsm<State>;
};

template<typename List, typename T, size_type N = 0>
struct state_index { };

template<template<class...>class List, typename U, typename T, typename... Ts, size_type Idx>
struct state_index<List<T,Ts...>, U, Idx>
    : std::conditional_t<std::is_same_v<T,U>,
                         Index_constant<Idx>,
                         state_index<List<Ts...>, U, Idx+1>>
{
};

template<typename T, typename = void_t<>> struct has_state_list : std::false_type { };
template<typename T>
struct has_state_list<T, void_t<typename T::Statelist>> : std::true_type { };
template<typename T>
constexpr inline auto has_state_list_v{has_state_list<T>::value};

template<typename T> struct get_state_list;
// {
//     static_assert(HasStatelist, "Fsm must define a Statelist type");
// };
// template<template<class,class...>class FsmT, typename T, typename... States>
// struct get_state_list<FsmT<T, States...>> { using type = typelist<States...>; };
template<template<typename,typename>class FsmT, typename Impl, typename Statelist>
struct get_state_list<FsmT<Impl, Statelist>> { using type = Statelist; };
// template<typename T>
// struct get_state_list<T,true> { using type = typename T::Statelist; };
template<typename T>
using get_state_list_t = typename get_state_list<T>::type;

template<typename List, typename T, size_type Idx = 0>
constexpr inline auto state_index_v{state_index<List, T, Idx>::value};

template<typename FsmT, typename = void_t<>> struct SelfT { };
template<typename FsmT>
struct SelfT<FsmT, void_t<decltype(std::declval<FsmT>().self())>> {
    using type = decltype(std::declval<FsmT>().self());
};
template<typename FsmT>
using Self = typename SelfT<FsmT>::type;

template<typename T, typename = void_t<>>
struct has_next_state : std::false_type { };
template<typename T>
struct has_next_state<T, void_t<typename std::decay_t<T>::next_state>> : std::true_type { };
template<typename T>
constexpr inline auto has_next_state_v{has_next_state<T>::value};

template<typename T, bool = has_next_state_v<T>>
struct Next_stateT { };
template<typename Traits>
struct Next_stateT<Traits, true> { using type = typename std::decay_t<Traits>::next_state; };
template<typename Traits>
using Next_state = typename Next_stateT<Traits>::type;

} // namespace ufsm
