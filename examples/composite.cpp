#include <iostream>
#include <type_traits>

#include "fsm.hpp"

namespace e {
struct A {
};
struct B {
};
struct C {
};
struct D {
};
struct E {
};
}  // namespace e

class Composite;
class Sub;
class Idle {
};  // empty initial state
class C1 {
};
class S1 {
};
class Final {
};  // empty terminating state

template<typename FsmType>
struct trace_logger {
        template <typename State, typename Event>
        void log_event(FsmType const&, State const&, Event const&) const noexcept
        {
            std::cerr << '[' << ufsm::logging::get_type_name<FsmType>() << "]: "
                      << " in state [" << ufsm::logging::get_type_name<State>() << "], got event ["
                      << ufsm::logging::get_type_name<Event>() << "]\n";
        }
        template <typename State>
        void log_exit(FsmType const&, State const&) const noexcept
        {
            std::cerr << "[" << ufsm::logging::get_type_name<FsmType>() << "]: state exit ["
                      << ufsm::logging::get_type_name<State>() << "]\n";
        }
        template <typename State>
        void log_entry(FsmType const&, State const&) const noexcept
        {
            std::cerr << "[" << ufsm::logging::get_type_name<FsmType>() << "]: state entry ["
                      << ufsm::logging::get_type_name<State>() << "]\n";
        }
        template <typename SrcState, typename DstState>
        void log_state_change(FsmType const&, SrcState const&, DstState const&) const noexcept
        {
            std::cerr << "[" << ufsm::logging::get_type_name<FsmType>() << "]: state change ["
                      << ufsm::logging::get_type_name<SrcState>() << "] -> ["
                      << ufsm::logging::get_type_name<DstState>() << "]\n";
        }
    };

class Sub {
    static inline trace_logger<Sub> logger_{ };
public:
    template<typename SM>
    void entry(SM const& fsm) const { std::cerr << "enter 'Sub' state\n"; }
    template<typename SM>
    void exit(SM const& fsm) const { std::cerr << "exit 'Sub' state\n"; }
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }

    constexpr inline auto transition_table() const noexcept
    {
        using namespace ufsm;
        return make_transition_table(
                make_entry(from_state<Idle>, event<e::C>, next_state<S1>),
                make_entry(from_state<S1>, event<e::D>, next_state<Idle>)
            );
    }
};

class Composite {
    static inline trace_logger<Composite> logger_{ };
public:
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }
    constexpr inline auto transition_table() const noexcept
    {
        using namespace ufsm;
        return make_transition_table(
                make_entry(from_state<Idle>, event<e::A>, next_state<C1>),
                make_entry(from_state<C1>, event<e::B>, next_state<Sub>),
                make_entry(from_state<Sub>, event<e::E>, next_state<Idle>)
            );
    }
};

template<typename SM, typename... Events>
inline void send_events(SM&& fsm, Events&&... events) noexcept
{
    ( ... , fsm.dispatch_event(std::forward<Events>(events)) );
}



int main()
{
    ufsm::Fsm<Composite> sm{ufsm::initial_state_v<Idle>};
    send_events(sm,
        e::A{}, e::B{}, e::C{}, e::D{}, e::E{}
    );
}
