#if !defined(TRACE_LOGGER_HEADER_GUARD_HPP_)
#define TRACE_LOGGER_HEADER_GUARD_HPP_

template <typename FsmType>
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

#endif  // TRACE_LOGGER_HEADER_GUARD_HPP_
