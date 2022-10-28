cmake_minimum_required(VERSION 3.15)

add_executable(dummy_unit_test test.cpp)
target_link_libraries(dummy_unit_test
    PRIVATE
        uFSM::uFSM
        uFSM::ProjectConfig
        gtest
        gtest_main
)

add_test(NAME dummy_unit_test COMMAND dummy_unit_test)
