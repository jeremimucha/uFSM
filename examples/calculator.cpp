#include <iostream>
#include "fsm.hpp"
#include "trace_logger.hpp"

namespace e
{
struct Op { char key; };
struct OpMinus { };
struct CE { };
struct C { };
struct Digit_0 { };
struct Digit_1_9 { int value; };
struct Point { };
struct Equals { };
struct OFF { };
} // namespace e

struct Calculator;
struct On;
struct Off;
struct Ready;
struct Result { };
struct Begin { };
struct Operand1;
struct Zero1 { };
struct Int1 { };
struct Fraction1 { };
struct Negated1 { };
struct Zero2;
struct Int2;
struct Fraction2;
struct Operand2;
struct OpEntered;
struct Negated2;
struct Error;

struct Init { };
struct Monostate { };

struct Ready
{
    static inline trace_logger<Ready> logger_{};
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }
    using InitialState = Begin;
    constexpr inline auto transition_table() noexcept {
        using namespace ufsm;
        return make_transition_table(
            make_entry(from_state<Begin>, event<e::Equals>, next_state<Result>),
            make_entry(from_state<Result>, event<AnyEvent_t>, next_state<Begin>)
        );
    }
};

struct Operand1 {
    double value;

    static inline trace_logger<Ready> logger_{};
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }
    using InitialState = Zero1;
    constexpr inline auto transition_table() noexcept {
        using namespace ufsm;
        return make_transition_table(
            make_entry(from_state<Zero1>, event<e::Digit_1_9>, next_state<Int1>),
            make_entry(from_state<Zero1>, event<e::Point>, next_state<Fraction1>),
            // make_aentry(from_state<Int1>, event<e::Digit_0>, [this](Operand1& state)),
            make_entry(from_state<Int1>, event<e::Point>, next_state<Fraction1>)
        );
    }
};

struct On {
    double lhs_operand;
    double rhs_operand;
    char op;

    using InitialState = Ready;
    constexpr inline auto transition_table() noexcept
    {
        using namespace ufsm;
        return make_transition_table(
            make_entry(from_state<Ready>, event<e::Digit_0>, next_state<Operand1>),
            make_entry(from_state<Ready>, event<e::Digit_1_9>, next_state<Operand1>),
            make_entry(from_state<Ready>, event<e::Point>, next_state<Operand1>),
            make_entry(from_state<Ready>, event<e::OpMinus>, next_state<Negated1>),
            make_entry(from_state<Ready>, event<e::Op>, next_state<OpEntered>),
            make_entry(from_state<Negated1>, event<e::Digit_0>, next_state<Operand1>),
            make_entry(from_state<Negated1>, event<e::Digit_1_9>, next_state<Operand1>),
            make_entry(from_state<Negated1>, event<e::Point>, next_state<Operand1>),
            make_entry(from_state<Operand1>, event<e::CE>, next_state<Ready>)
            // make_entry(from_state<Operand1>, event<e::Op>, next_state<OpEntered>),
            // make_entry(from_state<OpEntered>, event<e::Digit_0>, next_state<Zero2>),
            // make_entry(from_state<OpEntered>, event<e::Digit_1_9>, next_state<Int2>),
            // make_entry(from_state<OpEntered>, event<e::Point>, next_state<Fraction2>),
            // make_entry(from_state<OpEntered>, event<e::OpMinus>, next_state<Negated2>)
        );
    }
};

struct Calculator {
    using InitialState = On;
    constexpr inline auto transition_table() noexcept
    {
        using namespace ufsm;
        return make_transition_table(
            make_entry(from_state<On>, event<e::C>, next_state<On>),
            make_entry(from_state<On>, event<e::OFF>, next_state<Off>)
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
    ufsm::Fsm<Calculator> calculator;
    send_events(calculator,
        e::Digit_0{}
    );
}
