#include <iostream>
#include "fsm.hpp"
#include "stopwatch/stopwatch.h"


struct ePlay { };
struct eStop { };
struct ePause { };
struct eUpdate { };

struct sAnimating;
struct sPaused;
struct sIdle;


class Animation;

struct sAnimating {
    inline void handle_event(Animation& fsm, eUpdate) const noexcept;
    // template<typename Event>
    // inline void handle_event(Animation&,Event) const noexcept;

    void entry(Animation const& fsm) const;
};
struct sPaused {
    // template<typename Event>
    // inline void handle_event(Animation&,Event) const noexcept;

    void entry(Animation const& fsm) const;
};
struct sIdle {
    // template<typename Event>
    // constexpr void handle_event(Animation&,Event) const noexcept { }

    inline void entry(Animation& fsm) noexcept;
};

struct animation_logger;
struct animation_logger {
    template<typename State, typename Event>
    void log_event(Animation const&, State const&, Event const&) const noexcept
    {
    }
    template<typename Guard>
    void log_guard(Animation const&, Guard const&, bool result) const noexcept
    {
    }
    template<typename Action>
    void log_action(Animation const&, Action const&) const noexcept
    {
    }
    template<typename State>
    void log_exit(Animation const&, State const&) const noexcept
    {
    }
    template<typename State>
    void log_entry(Animation const&, State const&) const noexcept
    {
    }
    template<typename SrcState, typename DstState>
    void log_state_change(Animation const&, SrcState const&, DstState const&) const noexcept
    {
    }
};

class Animation //: public ufsm::Fsm<Animation, sAnimating, sPaused, sIdle>
{
public:
    // using Base = ufsm::Fsm<Animation, sAnimating, sPaused, sIdle>;
    // using Base::Base;

    static constexpr inline animation_logger& logger() noexcept
    {
        return Animation::logger_;
    }

    // static constexpr inline auto const& transition_table() noexcept { return transition_table_; }

// --- data members
    unsigned counter{0};
    static constexpr inline unsigned counter_limit{42};

    // constexpr auto guard1 = [](Animation const&) noexcept { return true; };
    // constexpr auto action1 = [](Animation const&) noexcept { std::cerr << __PRETTY_FUNCTION__ << "\n" };
    struct guard1 {
        constexpr bool operator()(Animation const&) const noexcept {return true;}
    };
    struct action1 {
        void operator()(Animation const&) const {
            // std::cerr << __PRETTY_FUNCTION__ << "\n";
        }
    };
    struct guard2 {
        constexpr bool operator()(Animation const& fsm) const noexcept {
            return fsm.counter >= Animation::counter_limit;
        }
    };

    /* static */ constexpr inline auto transition_table() noexcept
    {
        using namespace ufsm;
        return make_transition_table(
            make_entry(from_state<sIdle>, event<ePlay>, next_state<sAnimating>, guard1{}, action1{}),
            make_entry(from_state<sAnimating>, event<eUpdate>, next_state<sIdle>)
                .add_guard([](Animation const&)noexcept{return false;}),
            make_entry(from_state<sAnimating>,event<ePause>,next_state<sPaused>),
            make_entry(from_state<sAnimating>,event<eStop>,next_state<sIdle>),
            make_entry(from_state<sPaused>,event<ePlay>,next_state<sAnimating>),
            make_entry(from_state<sPaused>,event<eStop>,next_state<sIdle>)
        );
    }
private:
    static inline animation_logger logger_{};
};


inline void sAnimating::handle_event(Animation& fsm, eUpdate) const noexcept
{
    ++fsm.counter;
    // std::cerr << "\t" << __PRETTY_FUNCTION__ << ", counter = " << fsm.counter << "\n";
}

// template<typename Event>
// inline void sAnimating::handle_event(Animation& fsm,Event) const noexcept
// {
    // std::cerr << "\t" << __PRETTY_FUNCTION__ << ", counter = " << fsm.counter << "\n";
// }

// template<typename Event>
// inline void sPaused::handle_event(Animation& fsm,Event) const noexcept
// {
    // std::cerr << "\t" << __PRETTY_FUNCTION__ << ", counter = " << fsm.counter << "\n";
// }

inline void sIdle::entry(Animation& fsm) noexcept
{
    fsm.counter = 0;
    // std::cerr << "\tIdle::entry, counter = " << fsm.counter << "\n";
}

void sAnimating::entry(Animation const& fsm) const
{
    // std::cerr << "\tAnimating::entry, counter = " << fsm.counter << "\n";
}

void sPaused::entry(Animation const& fsm) const
{
    // std::cerr << "\tPaused::entry, counter = " << fsm.counter << "\n";
}

struct testguard1 {
    inline bool operator()(Animation const&) const{
        std::cerr << "testguard1\n";
        return true;
    }
};

struct testguard2 {
    inline bool operator()(Animation const&) const{
        std::cerr << "testguard2\n";
        return false;
    }
};

struct testaction {
    inline void operator()(Animation const&) const {
        std::cerr << "testaction\n";
    }
};

template<typename SM, typename... Events>
inline void send_events(SM&& fsm, Events&&... events) noexcept
{
    ( ... , fsm.dispatch_event(std::forward<Events>(events)) );
}

template<typename T>
inline void run_animation(T&& fsm) noexcept
{
    send_events(std::forward<T>(fsm),
        ePlay{},
        eUpdate{},
        ePause{},
        eUpdate{},
        ePlay{},
        eUpdate{},
        eStop{},
        ePlay{},
        eUpdate{},
        eStop{}
        );
}

int main()
{
    // auto animation = Animation{initial_state_v<sIdle>};  // set initial state on construction
    // std::cerr << "has_guard = " << has_guard_v<transition_table<sAnimating,eUpdate>> << "\n";
    // auto animation = Animation{};
    ufsm::Fsm<Animation> animation{};
    animation.set_initial_state(ufsm::initial_state_v<sIdle>);    // or explicitly later

    constexpr auto num_laps = 1'000'000u;
    auto sw = Stopwatch{"Animation"};
    for (auto i=0u; i<num_laps; ++i) {
        run_animation(animation);
    }
    sw.stop();
    const auto total = sw.lap_get();
    std::cout << "Animation benchmark over " << num_laps << " iterations:\n"
        << "-   total = " << total << " ms\n"
        << "-   average per 10 transitions = " << static_cast<double>(total) / num_laps << " ms\n"
        << "-   average per transition = " << static_cast<double>(total) / num_laps /10 << " ms\n";

}
