set(ufsm_include_headers
    action.hpp
    any_event.hpp
    dispatch_event.hpp
    entry_action.hpp
    entry_policy.hpp
    exit_action.hpp
    fsm_impl.hpp
    fsm.hpp
    fsmfwd.hpp
    get.hpp
    initial_state.hpp
    logging.hpp
    next_state.hpp
    state_index.hpp
    state_traits.hpp
    state_transition.hpp
    traits.hpp
    transition_guard.hpp
    transition_table.hpp
    try_dispatch.hpp
    try_set_initial_state.hpp
)
list(TRANSFORM ufsm_include_headers REPLACE
    "^.+$" "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ufsm/\\0>"
    OUTPUT_VARIABLE ufsm_include_headers_build
)
list(TRANSFORM ufsm_include_headers REPLACE
    "^.+$" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ufsm/\\0>"
    OUTPUT_VARIABLE ufsm_include_headers_install
)

target_sources(uFSM_uFSM
    INTERFACE
        ${ufsm_include_headers_build}
        ${ufsm_include_headers_install}
)
target_include_directories(uFSM_uFSM
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(
    DIRECTORY ufsm
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PATTERN "CMakeLists.txt" EXCLUDE
)
