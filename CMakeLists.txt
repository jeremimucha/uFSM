cmake_minimum_required(VERSION 3.15)

project(uFSM VERSION 0.1.0)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(project_config)

ConfigureBuildType(DEFAULT Debug)
ConfigureOutputDirectories()
ConfigureGlobalFlags()
ProjectConfigTarget(OUT ProjectConfig)
ConfigureProjectInstallDirectories()
# ShowOptions()

option(uFSM_BUILD_EXAMPLES "Build example binaries" ON)
option(uFSM_BUILD_BENCHMARKS "Build benchmark binaries" ON)
option(uFSM_INSTALL_EXAMPLES "Install example binaries" ON)
option(uFSM_INSTALL_BENCHMARKS "Install benchmark binaries" ON)
option(uFSM_DEBUG_LOG "Enable debug logs" OFF)

add_subdirectory(deps)

add_library(uFSM_uFSM INTERFACE)
add_library(uFSM::uFSM ALIAS uFSM_uFSM)
set_target_properties(uFSM_uFSM
    PROPERTIES EXPORT_NAME uFSM
)
add_subdirectory(include)

if(uFSM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(uFSM_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

include(CTest)
message(STATUS "BUILD_TESTING: ${BUILD_TESTING}")
if(uFSM_TESTS OR (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR AND BUILD_TESTING))
    add_subdirectory(tests)
endif()
