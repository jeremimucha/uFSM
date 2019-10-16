cmake_minimum_required( VERSION 3.1 )
project( test_${CMAKE_PROJECT_NAME} )

add_subdirectory(${CMAKE_SOURCE_DIR}/external/google-test)
include(GoogleTest)
include(CTest)
enable_testing()

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE test.cpp
    )
target_link_libraries( ${PROJECT_NAME}
    uFSM::uFSM
    uFSM::Flags
    GTest::GTest
    GTest::Main
    )

# enable_testing()
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
