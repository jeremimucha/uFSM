project(examples)

add_executable(animation)
target_sources(animation
    PRIVATE animation.cpp
    )
target_include_directories(animation
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )
target_link_libraries(animation
    PRIVATE
        uFSM::uFSM
        uFSM::ProjectConfig
    )
target_compile_definitions(animation PRIVATE
    $<$<CONFIG:Debug>:uFSM_DEBUG_LOG>
)

add_executable(composite)
target_sources(composite PRIVATE composite.cpp)
target_include_directories(composite
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_link_libraries(composite
    PRIVATE
        uFSM::uFSM
        uFSM::ProjectConfig
    )
target_compile_definitions(composite PRIVATE
    $<$<CONFIG:Debug>:uFSM_DEBUG_LOG>
)

add_executable(calculator)
target_sources(calculator PRIVATE calculator.cpp)
target_include_directories(calculator
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_link_libraries(calculator
    PRIVATE
        uFSM::uFSM
        uFSM::ProjectConfig
)
target_compile_definitions(calculator PRIVATE
    $<$<CONFIG:Debug>:uFSM_DEBUG_LOG>
)

if(uFSM_INSTALL_EXAMPLES)
    install(
        TARGETS
            animation
            composite
            calculator
        EXPORT ${uFSM_export_targets}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples
    )
endif()
