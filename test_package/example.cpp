#include <ufsm/fsm.hpp>

struct test_event { };
struct test_state { };

struct TestFsm {

    using InitialState = test_state;

    constexpr auto transition_table() const noexcept
    {
        using namespace ufsm;
        return make_transition_table(
            make_aentry(from_state<test_state>, event<test_event>, []{ std::cerr << "Hello FSM!"; })
        );
    }
};

int main() {
    ufsm::Fsm<TestFsm> fsm;
    fsm.dispatch_event(test_event{});
}
