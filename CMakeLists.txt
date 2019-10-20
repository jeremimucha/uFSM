cmake_minimum_required(VERSION 3.13)

project(uFSM VERSION 0.1.0)

include(${CMAKE_SOURCE_DIR}/cmake/clang_tidy.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/config.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/release.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/coverage.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SetOptions()
SetDefaultBuildType()
ClangTidyTarget(PATHS "C:/msys64/mingw64/share/clang")
CoverageTargets()
DeclareCompilerConfigTarget(${CMAKE_PROJECT_NAME}_flags)
DeclareInstallDirs()
add_library(${CMAKE_PROJECT_NAME}::Flags ALIAS ${CMAKE_PROJECT_NAME}_flags)

GetProjectPrefix(prefix)
set(OPT_INSTALL_EXAMPLES ${prefix}_INSTALL_EXAMPLES)
set(OPT_INSTALL_BENCHMARKS ${prefix}_INSTALL_BENCHMARKS)
option(${OPT_INSTALL_EXAMPLES} "Install example binaries" ON)
option(${OPT_INSTALL_BENCHMARKS} "Install benchmark binaries" ON)
option(FSM_DEBUG_LOG "Enable debug logs" OFF)

add_subdirectory(include)
# target_compile_definitions(${CMAKE_PROJECT_NAME} INTERFACE
#     $<$<CONFIG:Debug>:FSM_DEBUG_LOG>
# )
add_library(${CMAKE_PROJECT_NAME} INTERFACE)
add_library(${CMAKE_PROJECT_NAME}::${CMAKE_PROJECT_NAME} ALIAS ${CMAKE_PROJECT_NAME})
target_link_libraries(${CMAKE_PROJECT_NAME} INTERFACE ${CMAKE_PROJECT_NAME}::include)
add_subdirectory(examples)
add_subdirectory(benchmarks)

message(STATUS "OPT_TESTS: ${${OPT_TESTS}}")
if (${OPT_TESTS})
    include(CTest)
    enable_testing()
    add_subdirectory(${CMAKE_SOURCE_DIR}/external/google-test)
    add_subdirectory(tests)
endif()

install(
    TARGETS ${CMAKE_PROJECT_NAME}
    EXPORT ${project_export_targets}
    LIBRARY DESTINATION ${project_install_library_dir}
    ARCHIVE DESTINATION ${project_install_library_dir}
    RUNTIME DESTINATION ${project_install_runtime_dir}
    PUBLIC_HEADER DESTINATION ${project_install_include_dir}
    BUNDLE DESTINATION ${project_install_runtime_dir}
)

InstallProject()

