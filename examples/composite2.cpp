#include <iostream>
#include "fsm.hpp"

class S;
class S1;
struct S11 {
    template<typename SM> void entry(SM const&) const noexcept { }
    template<typename SM> void exit(SM const&) const noexcept { }
};
class S2;
class S21;
struct S211 {
    template<typename SM> void entry(SM const&) const noexcept { }
    template<typename SM> void exit(SM const&) const noexcept { }
};

class S1 {
public:
    template<typename SM> void entry(SM const&) const noexcept { }
    template<typename SM> void exit(SM const&) const noexcept { }

    // constexpr inline auto transition_table() const noexcept
    // {
    //     using namespace ufsm;
    //     return make_transition_table(
            
    //     );
    // }

};

class S {
public:
    template<typename SM> void entry(SM const&) const noexcept { }
    template<typename SM> void exit(SM const&) const noexcept { }

    // constexpr inline auto transition_table() const noexcept
    // {
    //     using namespace ufsm;
    //     make_entry(from_state<Init>, event<e::Init>, next_state<S2>),
    //     make_entry(from_state<S2>, event<e::C>, next_state<S1>),
    //     make_entry(from_state<S1>, event<e::C>, next_state<S2>),
    //     make_entry(from_state<S2>, event<e::G>, next_state<S1>),
    //     make_entry(from_state<S1>, event<e::G>, next_state<S2>),
    //     make_entry(from_state<S2>, event<e::H>, next_state<Idle>),
    //     make_entry(from_state<S1>, event<e::H>, next_state<Idle>),
    // }
};


int main()
{

}
