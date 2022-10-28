#pragma once

#include "traits.hpp"

namespace ufsm {

template<typename T, typename = void_t<>>
struct HasNextStateT : std::false_type { };
template<typename T>
struct HasNextStateT<T, void_t<typename std::decay_t<T>::next_state>> : std::true_type { };
template<typename T>
constexpr inline auto HasNextState{HasNextStateT<T>::value};

template<typename T, bool = HasNextState<T>>
struct NextStateT { };
template<typename Traits>
struct NextStateT<Traits, true> {
    using type = typename std::decay_t<Traits>::next_state;
};
template<typename Traits>
using NextState = typename NextStateT<Traits>::type;

}  // namespace ufsm
