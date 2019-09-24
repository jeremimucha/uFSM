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

struct AnyEvent_t { };
constexpr inline AnyEvent_t AnyEvent{};

template<typename C, typename T> struct ContainsTImpl;

template<template<class...> class C, typename T, typename... Us>
struct ContainsTImpl<C<Us...>, T> : std::disjunction<std::is_same<T, Us>...> { };

template<typename C, typename T>
struct ContainsT : ContainsTImpl<C,T> { };

template<typename C, typename T>
static constexpr inline auto Contains{ContainsTImpl<C,T>::value};

template<typename T> struct initial_state_t { };
template<typename T> constexpr inline auto initial_state_v{initial_state_t<T>{}};

template<typename T, typename = void_t<>> struct HasInitialStateT : std::false_type { };
template<typename T>
struct HasInitialStateT<T, void_t<typename T::InitialState>> : std::true_type { };
template<typename T>
constexpr inline auto HasInitialState{HasInitialStateT<T>::value};

template<typename T, typename U, bool = HasInitialState<T>> struct initial_state_or {
    using type = U;
};
template<typename T, typename U>
struct initial_state_or<T, U, true> { using type = typename T::InitialState; };

template<typename FsmT, typename = void_t<>>
struct HasTransitionTableT : std::false_type { };
template<typename FsmT>
struct HasTransitionTableT<FsmT, void_t<decltype(std::declval<FsmT>().transition_table())>>
    : std::true_type { };
template<typename FsmT>
constexpr inline auto HasTransitionTable{HasTransitionTableT<FsmT>::value};

template<typename State, bool = HasTransitionTable<State>>
struct StateTraitsT {
    using state_type = State;
};

template<typename State>
struct StateTraitsT<State, true> {
    using state_type = ::ufsm::Fsm<State>;
};

template<typename List, typename T, SizeT N = 0>
struct StateIndexT { };

template<template<class...>class List, typename U, typename T, typename... Ts, SizeT Idx>
struct StateIndexT<List<T,Ts...>, U, Idx>
    : std::conditional_t<std::is_same_v<T,U>,
                         IndexConstant<Idx>,
                         StateIndexT<List<Ts...>, U, Idx+1>>
{
};

template<typename List, typename T, SizeT Idx = 0>
constexpr inline auto StateIndex{StateIndexT<List, T, Idx>::value};

template<typename T> struct GetStateListT;
template<template<typename,typename>class FsmT, typename Impl, typename Statelist>
struct GetStateListT<FsmT<Impl, Statelist>> {
    using type = Statelist;
};
template<typename T>
using GetStateList = typename GetStateListT<T>::type;

template<typename T, typename = void_t<>>
struct HasNextStateT : std::false_type { };
template<typename T>
struct HasNextStateT<T, void_t<typename std::decay_t<T>::next_state>> : std::true_type { };
template<typename T>
constexpr inline auto HasNextState{HasNextStateT<T>::value};

template<typename T, bool = HasNextState<T>>
struct NextStateT { };
template<typename Traits>
struct NextStateT<Traits, true> { using type = typename std::decay_t<Traits>::next_state; };
template<typename Traits>
using NextState = typename NextStateT<Traits>::type;

} // namespace ufsm
