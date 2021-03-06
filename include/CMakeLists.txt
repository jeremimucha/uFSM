project(uFSM_include)

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
    "^.+$" "$<INSTALL_INTERFACE:${project_install_include_dir}/ufsm/\\0>"
    OUTPUT_VARIABLE ufsm_include_headers_install
)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${CMAKE_PROJECT_NAME}::include ALIAS ${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    INTERFACE
        ${ufsm_include_headers_build}
        ${ufsm_include_headers_install}
)
target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${project_export_targets}
)
install(
    DIRECTORY ufsm
    DESTINATION ${project_install_include_dir}
    PATTERN "CMakeLists.txt" EXCLUDE
)
