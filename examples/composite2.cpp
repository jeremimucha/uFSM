#include <iostream>
#include "fsm.hpp"
#include "trace_logger.hpp"

namespace e
{
struct A{};
struct B{};
struct C{};
struct D{};
struct E{};
struct F{};
struct G{};
} // namespace e


class S;
class S1;
struct S11 {
    template<typename SM> void entry(SM const&) const noexcept { }
    template<typename SM> void exit(SM const&) const noexcept { }
};
struct S2 {
    template<typename SM> void entry(SM const&) const noexcept { }
    template<typename SM> void exit(SM const&) const noexcept { }
};
struct S21 : S2 {
    template<typename SM> void entry(SM const&) const noexcept { }
    template<typename SM> void exit(SM const&) const noexcept { }
};
struct S22 : S2 {
    template<typename SM> void entry(SM const&) const noexcept { }
    template<typename SM> void exit(SM const&) const noexcept { }
};
struct S211 {
    template<typename SM> void entry(SM const&) const noexcept { }
    template<typename SM> void exit(SM const&) const noexcept { }
};

class S1 {
public:

    template<typename SM> void entry(SM const&) const noexcept { }
    template<typename SM> void exit(SM const&) const noexcept { }

    using InitialState = S11;
    using EntryPolicy = ufsm::InitialStateEntryPolicy;

    // constexpr inline auto transition_table() const noexcept
    // {
    //     using namespace ufsm;
    //     return make_transition_table(
    //         make_gentry(from_state<S11>, event<e::D>, next_state<S11>, [])
    //     );
    // }

};

class S {
    static inline trace_logger<S> logger_{};
public:
    template<typename SM> void entry(SM const&) const noexcept { }
    template<typename SM> void exit(SM const&) const noexcept { }
    static constexpr inline decltype(auto) logger() noexcept { return logger_; }

    using InitialState = S1;
    constexpr inline auto transition_table() const noexcept
    {
        using namespace ufsm;
        return make_transition_table(
            make_entry(from_state<S1>, event<e::A>, next_state<S21>),
            make_entry(from_state<S1>, event<e::B>, next_state<S22>),
            make_entry(from_state<S21>, event<e::C>, next_state<S22>),
            make_entry(from_state<S22>, event<e::D>, next_state<S21>),
            make_entry(from_state<S2>, event<e::E>, next_state<S1>)
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
    ufsm::Fsm<S> sfsm;
    send_events(sfsm, e::A{}, e::B{}, e::E{});
}
