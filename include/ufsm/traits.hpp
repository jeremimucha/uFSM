#pragma once

#include <type_traits>
#include <utility>
#include "transition_table.hpp"


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


template<typename T> struct initial_state { };
template<typename T> constexpr inline auto initial_state_v{initial_state<T>{}};

template<typename List, typename T, size_type N = 0>
struct state_index { };

template<template<class...>class List, typename U, typename T, typename... Ts, size_type Idx>
struct state_index<List<T,Ts...>, U, Idx>
    : std::conditional_t<std::is_same_v<T,U>,
                         Index_constant<Idx>,
                         state_index<List<Ts...>, U, Idx+1>>
{
};

template<typename T> struct get_state_list { };
template<template<class,class...>class FsmT, typename T, typename... States>
struct get_state_list<FsmT<T, States...>> { using type = typelist<States...>; };
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
