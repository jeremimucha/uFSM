#pragma once

#include "traits.hpp"

namespace ufsm {

template<typename List, typename T, SizeT N = 0>
struct StateIndexT { };

template<template<class...> class List, typename U, typename T, typename... Ts, SizeT Idx>
struct StateIndexT<List<T, Ts...>, U, Idx>
  : std::conditional_t<std::is_same_v<T, U>, IndexConstant<Idx>, StateIndexT<List<Ts...>, U, Idx + 1>> { };

template<typename List, typename T, SizeT Idx = 0>
constexpr inline auto StateIndex{StateIndexT<List, T, Idx>::value};

}  // namespace ufsm
