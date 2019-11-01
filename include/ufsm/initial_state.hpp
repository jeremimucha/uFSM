#pragma once

#include "traits.hpp"

namespace ufsm
{

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

} // namespace ufsm
