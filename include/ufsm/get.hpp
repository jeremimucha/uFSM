#pragma once

#include <utility>
#include "traits.hpp"

namespace ufsm
{

template<typename State, typename FsmT>
inline constexpr decltype(auto) get_state(FsmT&& fsm) noexcept;

namespace back
{

template<SizeT Idx, typename FsmT>
inline constexpr decltype(auto) Get(FsmT&& fsm) noexcept;

template<SizeT Idx, typename FsmT>
using StateAt = decltype( Get<Idx>(std::declval<FsmT>()) );

} // namespace back
} // namespace ufsm
