project(uFSM_include)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${CMAKE_PROJECT_NAME}::uFSM ALIAS ${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    INTERFACE
        ufsm/action.hpp
        ufsm/dispatch_event.hpp
        ufsm/entry_action.hpp
        ufsm/entry_policy.hpp
        ufsm/exit_action.hpp
        ufsm/fsm_impl.hpp
        ufsm/fsm.hpp
        ufsm/fsmfwd.hpp
        ufsm/get.hpp
        ufsm/logging.hpp
        ufsm/state_transition.hpp
        ufsm/traits.hpp
        ufsm/transition_guard.hpp
        ufsm/transition_table.hpp
        ufsm/try_dispatch.hpp
        ufsm/try_set_initial_state.hpp
)
target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)
