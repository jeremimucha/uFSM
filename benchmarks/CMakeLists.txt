project(benchmarks)

add_executable(bench_animation)
target_sources(bench_animation PRIVATE bench_animation.cpp)
target_include_directories(bench_animation
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_link_libraries(bench_animation
    PRIVATE
        uFSM::uFSM
        uFSM::ProjectConfig
)

if(uFSM_INSTALL_BENCHMARKS)
    install(
        TARGETS
            bench_animation
        EXPORT ${project_export_targets}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/benchmarks
    )
endif()
