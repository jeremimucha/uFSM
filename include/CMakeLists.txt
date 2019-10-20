project(uFSM_include)
add_library(${PROJECT_NAME} INTERFACE)
add_library(${CMAKE_PROJECT_NAME}::include ALIAS ${PROJECT_NAME})
# target_sources(${PROJECT_NAME}
#     INTERFACE
        # action.hpp
        # dispatch_event.hpp
        # entry_action.hpp
        # entry_policy.hpp
        # exit_action.hpp
        # fsm_impl.hpp
        # fsm.hpp
        # fsmfwd.hpp
        # get.hpp
        # logging.hpp
        # state_transition.hpp
        # traits.hpp
        # transition_guard.hpp
        # transition_table.hpp
        # try_dispatch.hpp
        # try_set_initial_state.hpp
# )
target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${project_export_targets}
    PUBLIC_HEADER DESTINATION ${project_install_include_dir}
    # LIBRARY DESTINATION ${project_install_library_dir}
    # ARCHIVE DESTINATION ${project_install_library_dir}
    # INCLUDES DESTINATION ${project_install_include_dir}
    # RUNTIME DESTINATION ${project_install_runtime_dir}
)
install(
    DIRECTORY ufsm
    DESTINATION ${project_install_include_dir}
)
