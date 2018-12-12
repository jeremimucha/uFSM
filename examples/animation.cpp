#include <iostream>
#include "fsm.hpp"


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
    void log_event(Animation const& fsm, State const&, Event const&) const noexcept;
    // {
    //     std::cerr << '[' << logging::get_type_name<Animation>()
    //         << "]: count = " << fsm.counter " in state [" << logging::get_type_name<State>()
    //         << "], got event [" << logging::get_type_name<Event>() << "]\n";
    // }
    template<typename Guard>
    void log_guard(Animation const&, Guard const&, bool result) const noexcept
    {
        std::cerr << "[" << ufsm::logging::get_type_name<Animation>()
            << "]: guard [" << ufsm::logging::get_type_name<Guard>() << "]"
            << (result ? " [OK]" : " [Reject]") << "\n";
    }
    template<typename Action>
    void log_action(Animation const&, Action const&) const noexcept
    {
        std::cerr << "[" << ufsm::logging::get_type_name<Animation>()
            << "]: action [" << ufsm::logging::get_type_name<Action>() << "]\n";
    }
    template<typename State>
    void log_exit(Animation const&, State const&) const noexcept
    {
        std::cerr << "[" << ufsm::logging::get_type_name<Animation>()
            << "]: state exit [" << ufsm::logging::get_type_name<State>() << "]\n";
    }
    template<typename State>
    void log_entry(Animation const&, State const&) const noexcept
    {
        std::cerr << "[" << ufsm::logging::get_type_name<Animation>()
            << "]: state entry [" << ufsm::logging::get_type_name<State>() << "]\n";
    }
    template<typename SrcState, typename DstState>
    void log_state_change(Animation const&, SrcState const&, DstState const&) const noexcept
    {
        std::cerr << "[" << ufsm::logging::get_type_name<Animation>()
            << "]: state change [" << ufsm::logging::get_type_name<SrcState>() << "] -> ["
            << ufsm::logging::get_type_name<DstState>() << "]\n";
    }
};

class Animation : public ufsm::Fsm<Animation, sAnimating, sPaused, sIdle>
{
public:
    using Base = ufsm::Fsm<Animation, sAnimating, sPaused, sIdle>;
    using Base::Base;

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
    // static inline constexpr TransitionTraits<
    //     TransitionEntry<sIdle,ePlay,sAnimating,guard1,action1>,
    //     TransitionEntry<sAnimating,eUpdate,sIdle,guard2,void>,
    //     TransitionEntry<sAnimating,ePause,sPaused,void,void>,
    //     TransitionEntry<sAnimating,eStop,sIdle,void,void>,
    //     TransitionEntry<sPaused,ePlay,sAnimating,void,void>,
    //     TransitionEntry<sPaused,eStop,sIdle,void,void>
    // > transition_table{};

    /* static */ constexpr inline auto transition_table() noexcept
    {
        using namespace ufsm;
        return make_transition_table(
            make_entry(wrap<sIdle>, wrap<ePlay>, wrap<sAnimating>, guard1{}, action1{}),
            make_entry(wrap<sAnimating>, wrap<eUpdate>, wrap<sIdle>)
                .add_guard([](Animation const&)noexcept{return false;}),
            make_entry(wrap<sAnimating>,wrap<ePause>,wrap<sPaused>),
            make_entry(wrap<sAnimating>,wrap<eStop>,wrap<sIdle>),
            make_entry(wrap<sPaused>,wrap<ePlay>,wrap<sAnimating>),
            make_entry(wrap<sPaused>,wrap<eStop>,wrap<sIdle>)
        );
    }
private:
    static inline animation_logger logger_{};
};

template<typename State, typename Event>
void animation_logger::log_event(Animation const& fsm, State const&, Event const&) const noexcept
{
    std::cerr << '[' << ufsm::logging::get_type_name<Animation>()
        << "]: count = " << fsm.counter << " in state [" << ufsm::logging::get_type_name<State>()
        << "], got event [" << ufsm::logging::get_type_name<Event>() << "]\n";
}

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
    auto animation = Animation{};
    animation.set_initial_state(ufsm::initial_state_v<sIdle>);    // or explicitly later
    run_animation(animation);
}
