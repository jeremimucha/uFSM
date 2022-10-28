cmake_minimum_required(VERSION 3.15)

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main
)

message(STATUS "${ColorGreen}Fetching googletest${ColorReset}")
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

foreach(_gtestTarget IN ITEMS gtest gtest_main gmock)
get_target_property(_gtestIncludeDirs ${_gtestTarget} INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(${_gtestTarget}
        SYSTEM INTERFACE
            ${_gtestIncludeDirs}
    )
endforeach()
