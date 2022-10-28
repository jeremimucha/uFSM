#pragma once

#include "traits.hpp"
#include <utility>

namespace ufsm {
namespace back {

template<SizeT Idx, typename State>
class FsmState {
  private:
    State value_{};

  protected:
    ~FsmState() noexcept = default;

  public:
    using type = State;

    constexpr FsmState() noexcept = default;
    constexpr FsmState(FsmState const&) = default;
    constexpr FsmState(FsmState&&) noexcept = default;
    constexpr FsmState& operator=(FsmState const&) = default;
    constexpr FsmState& operator=(FsmState&&) noexcept = default;

    template<typename U, typename = std::enable_if_t<!std::is_same_v<FsmState, std::decay_t<U>>>>
    constexpr FsmState(U&& fvalue) noexcept(std::is_nothrow_constructible_v<State, U>)
      : value_{std::forward<U>(fvalue)}
    { }

    constexpr State& get() & noexcept { return value_; }
    constexpr State const& get() const& noexcept { return value_; }
    constexpr State&& get() && noexcept { return std::move(value_); }
    constexpr State const&& get() const&& noexcept { return std::move(value_); }
};

template<SizeT Idx, typename State>
using FsmStateT = typename FsmState<Idx, State>::type;

}  // namespace back

}  // namespace ufsm
