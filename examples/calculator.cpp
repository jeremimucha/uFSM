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
struct Off {
    template<typename SM> constexpr void entry(SM const&) const noexcept { }
    template<typename SM> constexpr void exit(SM const&) const noexcept { }
};
struct Ready;
struct Result {
    template<typename SM> constexpr void entry(SM const&) const noexcept { }
    template<typename SM> constexpr void exit(SM const&) const noexcept { }
};
struct Begin {
    template<typename SM> constexpr void entry(SM const&) const noexcept { }
    template<typename SM> constexpr void exit(SM const&) const noexcept { }
};

struct Zero {
    template<typename SM> constexpr void entry(SM const&) const noexcept { }
    template<typename SM> constexpr void exit(SM const&) const noexcept { }
};

struct Int {
    static inline trace_logger<Int> logger_{};
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }

    template<typename SM> constexpr void entry(SM const&) const noexcept { }
    template<typename SM> constexpr void exit(SM const&) const noexcept { }
};

template<std::size_t I> struct Operand;
template<typename Base>
struct ZeroBase : public Base {
    static inline trace_logger<ZeroBase> logger_{};
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }
    template<typename SM> constexpr void entry(SM const& sm) const noexcept { Base::entry(sm); }
    template<typename SM> constexpr void exit(SM const& sm) const noexcept { Base::exit(sm); }
};

template<typename Base>
struct IntBase : Base {
    static inline trace_logger<IntBase> logger_{};
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }

    template<typename SM> constexpr void entry(SM const& sm) const noexcept { Base::entry(sm); }
    template<typename SM> constexpr void exit(SM const& sm) const noexcept { Base::exit(sm); }
};

struct Oper {
    static inline trace_logger<Oper> logger_{};
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }
    template<typename SM> constexpr void entry(SM const&) const noexcept { }
    template<typename SM> constexpr void exit(SM const&) const noexcept { }
};
using Zero_ = ZeroBase<Oper>;
using Int_ = IntBase<Oper>;
struct Fraction {
    template<typename SM> constexpr void entry(SM const&) const noexcept { }
    template<typename SM> constexpr void exit(SM const&) const noexcept { }
};
template<std::size_t I> struct Negated {
    template<typename SM> constexpr void entry(SM const&) const noexcept { }
    template<typename SM> constexpr void exit(SM const&) const noexcept { }
};
using Negated1 = Negated<1>;
using Negated2 = Negated<2>;

// struct Zero2;
// struct Int2;
// struct Fraction2;
// struct Operand2;
struct OpEntered { };
// struct Negated2;
struct Error;

struct Init { };

struct Ready
{
    static inline trace_logger<Ready> logger_{};
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }

    template<typename SM> constexpr void entry(SM const&) const noexcept { }
    template<typename SM> constexpr void exit(SM const&) const noexcept { }

    using InitialState = Begin;
    constexpr inline auto transition_table() noexcept {
        using namespace ufsm;
        return make_transition_table(
            make_entry(from_state<Begin>, event<e::Equals>, next_state<Result>),
            make_entry(from_state<Result>, event<AnyEvent_t>, next_state<Begin>)
        );
    }
};

template<std::size_t I>
struct Operand {
    int integral;
    int fractional;

    static inline trace_logger<Operand> logger_{};
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }

    template<typename SM> constexpr void entry(SM const&) const noexcept { }
    template<typename SM> constexpr void exit(SM const&) const noexcept { }

    // constexpr void entry(On&) noexcept { }
    // constexpr void exit(On const&) noexcept { }

    using InitialState = Zero;
    // States which we'd like to explicitly enter substates for
    // need to set the CurrentStateEntryPolicy - otherwise the state will be reset
    // to the initial state on entry
    using EntryPolicy = ufsm::CurrentStateEntryPolicy;
    constexpr inline auto transition_table() noexcept {
        using namespace ufsm;
        auto const add_zero = [](Operand& s, e::Digit_0) noexcept {
            s.integral *= 10;
        };
        auto const add_digit = [](Operand& s, e::Digit_1_9 e) noexcept {
            s.integral *= 10;
            s.integral += e.value;
        };
        return make_transition_table(
            make_aentry(from_state<Zero>, event<e::Digit_1_9>, next_state<Int>, add_digit),
            make_entry(from_state<Zero>, event<e::Point>, next_state<Fraction>),
            make_aentry(from_state<Int>, event<e::Digit_0>, add_zero),
            make_aentry(from_state<Int>, event<e::Digit_1_9>, add_digit),
            make_entry(from_state<Int>, event<e::Point>, next_state<Fraction>),
            make_aentry(from_state<Fraction>, event<e::Digit_0>,
                [](Operand& state, e::Digit_0){
                    state.fractional *= 10;
            }),
            make_aentry(from_state<Fraction>, event<e::Digit_1_9>,
                [](Operand& state, e::Digit_1_9 e){
                    state.fractional *= 10; state.fractional += e.value;
            })
        );
    }
};
using Operand1 = Operand<1>;
using Operand2 = Operand<2>;

struct On {
    double lhs_operand;
    double rhs_operand;
    char op;

    static inline trace_logger<On> logger_{};
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }

    template<typename SM> constexpr void entry(SM const&) const noexcept { }
    template<typename SM> constexpr void exit(SM const&) const noexcept { }

    using InitialState = Ready;
    constexpr inline auto transition_table() noexcept
    {
        using namespace ufsm;
        auto const guard_op_minus = [](e::Op evt) noexcept { return evt.key == '-'; };
        return make_transition_table(
            make_entry(from_state<Ready>, event<e::Digit_0>, next_state<Operand1>, substate<Zero>),
            make_entry(from_state<Ready>, event<e::Digit_1_9>, next_state<Operand1>, substate<Int>),
            make_entry(from_state<Ready>, event<e::Point>, next_state<Operand1>, substate<Fraction>),
            make_gentry(from_state<Ready>, event<e::Op>, next_state<Negated1>, guard_op_minus),
            make_entry(from_state<Ready>, event<e::Op>, next_state<OpEntered>),
            make_entry(from_state<Negated1>, event<e::Digit_0>, next_state<Operand1>, substate<Zero>),
            make_entry(from_state<Negated1>, event<e::Digit_1_9>, next_state<Operand1>, substate<Int>),
            make_entry(from_state<Negated1>, event<e::Point>, next_state<Operand1>, substate<Fraction>),
            make_entry(from_state<Negated1>, event<e::CE>, next_state<Ready>),
            make_entry(from_state<Operand1>, event<e::CE>, next_state<Ready>),
            make_aentry(from_state<Operand1>, event<e::Op>, next_state<OpEntered>,
                [](auto const& e) noexcept {
                    std::cerr << __PRETTY_FUNCTION__ << "\n";
                    // std::cerr << "Operand1 integral = " << ufsm::get_state<Operand1>(f).integral << "\n";
                }),
            make_entry(from_state<OpEntered>, event<e::Digit_0>, next_state<Operand2>, substate<Zero>),
            make_entry(from_state<OpEntered>, event<e::Digit_1_9>, next_state<Operand2>, substate<Int>),
            make_entry(from_state<OpEntered>, event<e::Point>, next_state<Operand2>, substate<Fraction>),
            make_gentry(from_state<OpEntered>, event<e::Op>, next_state<Negated2>, guard_op_minus),
            make_entry(from_state<OpEntered>, event<e::Op>, next_state<Operand2>),
            make_entry(from_state<Negated2>, event<e::Digit_0>, next_state<Operand2>, substate<Zero>),
            make_entry(from_state<Negated2>, event<e::Digit_1_9>, next_state<Operand2>, substate<Int>),
            make_entry(from_state<Negated2>, event<e::Point>, next_state<Operand2>, substate<Fraction>),
            make_entry(from_state<Negated2>, event<e::CE>, next_state<OpEntered>),
            make_entry(from_state<Operand2>, event<e::CE>, next_state<OpEntered>),
            make_entry(from_state<Operand2>, event<e::Op>, next_state<OpEntered>),
            make_entry(from_state<Operand2>, event<e::Equals>, next_state<Ready>, substate<Result>)
        );
    }
};

struct Calculator {
    static inline trace_logger<Calculator> logger_{};
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }

    template<typename SM> constexpr void entry(SM const&) const noexcept { }
    template<typename SM> constexpr void exit(SM const&) const noexcept { }

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
        e::Digit_0{},
        e::Digit_1_9{42},
        e::Op{'+'},
        e::C{}
    );
}
