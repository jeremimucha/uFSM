#include <iostream>
#include <cmath>
#include <ufsm/fsm.hpp>
#include "trace_logger.hpp"

namespace e
{
struct Op {
    char key;
};
struct CE { };
struct C { };
struct Digit_0 { };
struct Digit_1_9 { int value; };
struct Point { };
struct Equals { };
struct Off { };
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

// ------------------------------------------------------------------------------------------------
// An alternative to achieving hierarhical behavior would be to implement the common behavior in
// a base class and then the state-specific behavior in subclasses (substates). This however
// would require the Base class to be stateless, implementing only some logic. CRTP could be used.
// template<typename Base>
// struct ZeroBase : public Base {
//     static inline trace_logger<ZeroBase> logger_{};
//     static constexpr inline decltype(auto) logger() noexcept { return logger_; }
//     template<typename SM> constexpr void entry(SM const& sm) const noexcept { Base::entry(sm); }
//     template<typename SM> constexpr void exit(SM const& sm) const noexcept { Base::exit(sm); }
// };

// template<typename Base>
// struct IntBase : Base {
//     static inline trace_logger<IntBase> logger_{};
//     static constexpr inline decltype(auto) logger() noexcept { return logger_; }

//     template<typename SM> constexpr void entry(SM const& sm) const noexcept { Base::entry(sm); }
//     template<typename SM> constexpr void exit(SM const& sm) const noexcept { Base::exit(sm); }
// };

// struct Oper {
//     static inline trace_logger<Oper> logger_{};
//     static constexpr inline decltype(auto) logger() noexcept { return logger_; }
//     template<typename SM> constexpr void entry(SM const&) const noexcept { }
//     template<typename SM> constexpr void exit(SM const&) const noexcept { }
// };
// using Zero_ = ZeroBase<Oper>;
// using Int_ = IntBase<Oper>;
// ------------------------------------------------------------------------------------------------

template<std::size_t I> struct Operand;

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

struct OpEntered { };
struct Error { };
struct Init { };

struct Ready
{
    static inline trace_logger<Ready> logger_{};
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }

    template<typename SM> constexpr void entry(SM const&) const noexcept { }
    template<typename SM> constexpr void exit(SM const&) const noexcept { }

    using InitialState = Begin;
    // using EntryPolicy = ufsm::InitialStateEntryPolicy;
    constexpr inline auto transition_table() const noexcept {
        using namespace ufsm;
        return make_transition_table(
            make_entry(from_state<Begin>, event<e::Equals>, next_state<Result>),
            make_entry(from_state<Result>, event<AnyEvent_t>, next_state<Begin>),
            make_entry(from_state<Error>, event<AnyEvent_t>, next_state<Begin>)
        );
    }
};

template<typename T>
[[nodiscard]] constexpr inline auto digit_count(T value) noexcept
{
    auto digits{0};
    while(++digits && value) {
        value /= 10;
    }
    return digits;
}

template<std::size_t I>
struct Operand {
    int integral_;
    int fractional_;

    static inline trace_logger<Operand> logger_{};
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }

    constexpr void entry() noexcept // no-argument entry-action, the same is valid for exit
    {
        integral_ = 0;
        fractional_ = 0;
    }

    template<typename T>
    static constexpr inline void increment(T& v) noexcept
    {
        v *= 10;
    }

    [[nodiscard]] constexpr inline auto eval() const noexcept
    {
        const auto ord_magn = std::pow(10.0, digit_count(fractional_) - 1);
        return integral_ + fractional_ / ord_magn;
    }

    using InitialState = Zero;
    // States which we'd like to explicitly enter substates for
    // need to set the CurrentStateEntryPolicy - otherwise the state will be reset
    // to the initial state on entry
    using EntryPolicy = ufsm::CurrentStateEntryPolicy;

    constexpr inline auto transition_table() const noexcept {
        using namespace ufsm;
        auto const add_zero = [](Operand& s, e::Digit_0) noexcept {
            increment(s.integral_);
        };
        auto const add_digit = [](Operand& s, e::Digit_1_9 e) noexcept {
            increment(s.integral_);
            s.integral_ += e.value;
        };

        return make_transition_table(
            make_aentry(from_state<Zero>, event<e::Digit_1_9>, next_state<Int>, add_digit),
            make_entry(from_state<Zero>, event<e::Point>, next_state<Fraction>),
            make_aentry(from_state<Int>, event<e::Digit_0>, add_zero),
            make_aentry(from_state<Int>, event<e::Digit_1_9>, add_digit),
            make_entry(from_state<Int>, event<e::Point>, next_state<Fraction>),
            make_aentry(from_state<Fraction>, event<e::Digit_0>,
                [](Operand& s, e::Digit_0){
                    increment(s.fractional_);
            }),
            make_aentry(from_state<Fraction>, event<e::Digit_1_9>,
                [](Operand& s, e::Digit_1_9 e){
                    increment(s.fractional_);
                    s.fractional_ += e.value;
            })
        );
    }
};
using Operand1 = Operand<1>;
using Operand2 = Operand<2>;

struct On {
    double total_;
    char op_;

    static inline trace_logger<On> logger_{};
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }

    template<typename SM> constexpr void entry(SM const&) noexcept
    {
        total_ = 0.0;
        op_ = '!';  // None
    }
    template<typename SM> constexpr void exit(SM const&) const noexcept { }

    template<typename T>
    void update(T rhs)
    {
        switch (op_) {
        case '+':
            total_ += rhs;
            break;
        case '-':
            total_ -= rhs;
            break;
        case '*':
            total_ *= rhs;
            break;
        case '/':
            total_ /= rhs;
            break;
        default:
            throw std::runtime_error{"Unknown operator"};
        }
    }

    using InitialState = Ready;
    constexpr inline auto transition_table() const noexcept
    {
        using namespace ufsm;
        auto const guard_op_minus{ [](e::Op evt) noexcept { return evt.key == '-'; } };
        auto const guard_division_by_zero{ [](auto const& fsm, e::Equals) noexcept {
                return fsm.op_ == '/' && ufsm::get_state<Operand2>(fsm).eval() == 0;
            }};
        auto const action_set_op{ [](auto& fsm, e::Op op) noexcept {fsm.op_ = op.key;} };

        return make_transition_table(
            make_entry(from_state<Ready>, event<e::Digit_0>, next_state<Operand1>),
            make_entry(from_state<Ready>, event<e::Digit_1_9>, next_state<Operand1>),
            make_entry(from_state<Ready>, event<e::Point>, next_state<Operand1>),
            make_entry(from_state<Ready>, event<e::Op>, next_state<OpEntered>, action_set_op),
            make_entry(from_state<Operand1>, event<e::CE>, next_state<Ready>),
            make_aentry(from_state<Operand1>, event<e::Op>, next_state<OpEntered>,
                [](auto& fsm, e::Op op) noexcept {
                    fsm.total_ = ufsm::get_state<Operand1>(fsm).eval();
                    fsm.op_ = op.key;
                }),
            make_entry(from_state<OpEntered>, event<e::Digit_0>, next_state<Operand2>),
            make_entry(from_state<OpEntered>, event<e::Digit_1_9>, next_state<Operand2>),
            make_entry(from_state<OpEntered>, event<e::Point>, next_state<Operand2>),
            make_aentry(from_state<OpEntered>, event<e::Op>, next_state<OpEntered>, action_set_op),
            make_entry(from_state<Operand2>, event<e::CE>, next_state<Operand2> /* Operand2 cleared on entry */),
            make_aentry(from_state<Operand2>, event<e::Op>, next_state<OpEntered>,
                [](auto& fsm, e::Op op) noexcept {
                    // calculate and display the current total
                    auto const rhs{ufsm::get_state<Operand2>(fsm).eval()};
                    fsm.update(rhs);
                    fsm.op_ = op.key;
            }),
            make_entry(from_state<Operand2>, event<e::Equals>, next_state<Ready>,
                guard_division_by_zero,
                [](auto& fsm, e::Equals) noexcept {
                    fsm.total_ = 0.0;
                    fsm.op_ = '!';
                }),
            make_aentry(from_state<Operand2>, event<e::Equals>, next_state<Ready>,
                [](auto& fsm, e::Equals) noexcept {
                    // calculate the current total
                    auto const lhs{ufsm::get_state<Operand1>(fsm).eval()};
                    auto const rhs{ufsm::get_state<Operand2>(fsm).eval()};
                    fsm.update(rhs);
                    std::cout << "Operand1 = " << lhs << "\n";
                    std::cout << "Operand2 = " << rhs << "\n";
                    std::cout << "total = " << fsm.total_ << "\n";
                }
            )
        );
    }
};

struct Calculator {
    static inline trace_logger<Calculator> logger_{};
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }

    template<typename SM> constexpr void entry(SM const&) const noexcept { }
    template<typename SM> constexpr void exit(SM const&) const noexcept { }

    using InitialState = On;
    constexpr inline auto transition_table() const noexcept
    {
        using namespace ufsm;
        return make_transition_table(
            make_entry(from_state<On>, event<e::C>, next_state<On>),
            make_entry(from_state<On>, event<e::Off>, next_state<Off>),
            make_entry(from_state<Off>, event<ufsm::AnyEvent_t>, next_state<Off>)
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

    // for (char ch{}; std::cin >> ch;) {
    //     switch (ch) {
    //     case '+': case '-': case '*': case '/':
    //         calculator.dispatch_event(e::Op{ch});
    //         break;
    //     case '0':
    //         calculator.dispatch_event(e::Digit_0{});
    //         break;
    //     case '1': case '2': case '3': case '4': case '5':
    //     case '6': case '7': case '8': case '9':
    //         calculator.dispatch_event(e::Digit_1_9{ch - '0'});
    //         break;
    //     case '.':
    //         calculator.dispatch_event(e::Point{});
    //         break;
    //     case 'e': case 'E':
    //         calculator.dispatch_event(e::CE{});
    //         break;
    //     case 'c': case 'C':
    //         calculator.dispatch_event(e::C{});
    //         break;
    //     case '\n': case '=':
    //         calculator.dispatch_event(e::Equals{});
    //         break;
    //     case 'q': case 'Q':
    //         calculator.dispatch_event(e::Off{});
    //         std::cin.setstate(std::ios::eofbit);
    //         break;
    //     }
    // }
    // send_events(calculator,
    //     e::Digit_0{},
    //     e::Digit_1_9{42},
    //     e::Op{'+'},
    //     e::Point{},
    //     e::Digit_1_9{3},
    //     e::Digit_1_9{1},
    //     e::Digit_1_9{4},
    //     e::Equals{},
    //     e::Op{'+'},
    //     e::Op{},
    //     e::C{}
    // );
    send_events(calculator,
        e::Digit_1_9{4},
        e::Op{'+'},
        e::Digit_1_9{1},
        e::Equals{}
    );
}
