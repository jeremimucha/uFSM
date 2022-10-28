#include <iostream>
#include <type_traits>

#include "trace_logger.hpp"
#include <ufsm/fsm.hpp>

namespace e {
struct A { };
struct B { };
struct C { };
struct D { };
struct E { };
}  // namespace e

class Composite;
class Sub;
struct Idle {
    template<typename SM> void entry(SM const&) const noexcept { }
    template<typename SM> void exit(SM const&) const noexcept { }
};  // empty initial state
class C1 {
  public:
    void entry(Composite const&) const { }
    void exit(Composite const&) const { }
};
class S1 {
  public:
    template<typename SM> void entry(SM const&) const noexcept { }
    template<typename SM> void exit(SM const&) const noexcept { }
};
class Final { };  // empty terminating state


class Sub {
    static inline trace_logger<Sub> logger_{};

  public:
    template<typename SM> void entry(SM const&) const { }
    template<typename SM> void exit(SM const&) const { }
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }

    using InitialState = Idle;
    using EntryPolicy = ufsm::CurrentStateEntryPolicy;

    constexpr inline auto transition_table() const noexcept
    {
        using namespace ufsm;
        return make_transition_table(make_entry(ufsm_from_state<Idle>, ufsm_event<e::C>, ufsm_next_state<S1>),
                                     make_entry(ufsm_from_state<S1>, ufsm_event<e::D>, ufsm_next_state<S1>));
    }
};

class Composite {
  public:
    using InitialState = Idle;
    constexpr inline auto transition_table() const noexcept
    {
        using namespace ufsm;
        return make_transition_table(make_entry(ufsm_from_state<Idle>, ufsm_event<e::A>, ufsm_next_state<C1>),
                                     make_entry(ufsm_from_state<C1>, ufsm_event<e::B>, ufsm_next_state<Sub>),
                                     make_entry(ufsm_from_state<Sub>, ufsm_event<e::E>, ufsm_next_state<Idle>));
    }

    static constexpr inline decltype(auto) logger() noexcept { return logger_; }

  private:
    static inline trace_logger<Composite> logger_{};
};

template<typename SM, typename... Events> inline void send_events(SM&& fsm, Events&&... events) noexcept
{
    (..., fsm.dispatch_event(std::forward<Events>(events)));
}


int main()
{
    ufsm::Fsm<Composite> sm{ufsm::initial_state_v<Idle>};
    send_events(sm, e::A{}, e::B{}, e::C{}, e::D{}, e::E{});
    send_events(sm, e::A{}, e::B{}, e::C{}, e::D{}, e::E{});
}
