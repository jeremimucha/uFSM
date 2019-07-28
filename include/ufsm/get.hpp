#pragma once

#include <utility>
#include "traits.hpp"

namespace ufsm
{
namespace back
{

template<size_type Idx, typename FsmT>
inline constexpr decltype(auto) Get(FsmT&& fsm) noexcept;

// template<size_type Idx, typename FsmT>
// inline constexpr decltype(auto) Get(FsmT&& fsm) noexcept
// {
//     return Get_impl<Idx>(std::forward<FsmT>(fsm));
// }

template<size_type Idx, typename FsmT>
using StateAt = decltype( Get<Idx>(std::declval<FsmT>()) );

} // namespace back
} // namespace ufsm
