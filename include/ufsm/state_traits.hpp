#pragma once

#include "traits.hpp"

namespace ufsm {

template<typename State, bool = HasTransitionTable<State>>
struct StateTraitsT {
    using state_type = State;
};

template<typename State>
struct StateTraitsT<State, true> {
    using state_type = ::ufsm::Fsm<State>;
};

}  // namespace ufsm
