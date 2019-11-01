#pragma once

#include <type_traits>
#include <utility>
#include "fsmfwd.hpp"

namespace ufsm
{

using SizeT = int;

template<SizeT Idx>
using IndexConstant = std::integral_constant<SizeT, Idx>;

template<SizeT... Idxs>
using IndexSequence = std::integer_sequence<SizeT, Idxs...>;

template<SizeT N>
using MakeIndexSequence = std::make_integer_sequence<SizeT, N>;

template<typename... Ts>
using IndexSequenceFor = MakeIndexSequence<sizeof...(Ts)>;

template<typename...> struct Typelist { };
template<typename...> using void_t = void;
template<typename T> struct Identity { using type = T; };
template<typename...> struct AlwaysFalseT : std::false_type { };
template<typename... Ts> constexpr inline auto AlwaysFalse{AlwaysFalseT<Ts...>::value};
template<typename Typelist> struct Front;
template<template<typename...> class List, class T, class... Ts>
struct Front<List<T, Ts...>> { using type = T; };
template<template<typename...> class List>
struct Front<List<>> { };

template<typename C, typename T> struct ContainsTImpl;

template<template<class...> class C, typename T, typename... Us>
struct ContainsTImpl<C<Us...>, T> : std::disjunction<std::is_same<T, Us>...> { };

template<typename C, typename T>
struct ContainsT : ContainsTImpl<C,T> { };

template<typename C, typename T>
static constexpr inline auto Contains{ContainsTImpl<C,T>::value};

template<typename FsmT, typename = void_t<>>
struct HasTransitionTableT : std::false_type { };
template<typename FsmT>
struct HasTransitionTableT<FsmT, void_t<decltype(std::declval<FsmT>().transition_table())>>
    : std::true_type { };
template<typename FsmT>
constexpr inline auto HasTransitionTable{HasTransitionTableT<FsmT>::value};

template<typename T> struct GetStateListT;
template<template<typename,typename>class FsmT, typename Impl, typename Statelist>
struct GetStateListT<FsmT<Impl, Statelist>> {
    using type = Statelist;
};
template<typename T>
using GetStateList = typename GetStateListT<T>::type;

template<typename T> struct GetIndicesT;
template<typename Impl, typename... States>
struct GetIndicesT<Fsm<Impl, States...>> {
    using type = MakeIndexSequence<sizeof...(States)>;
};
template<typename FsmT>
using GetIndices = typename GetIndicesT<std::decay_t<FsmT>>::type;

} // namespace ufsm
