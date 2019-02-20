cmake_minimum_required( VERSION 3.1 )

project( "test_${Project}" )


###############################################################################
# Prepare test sources

# These are all sources EXCLUDING the tests_main.cpp which contains main()
# function or the appropriate Catch2 define.
# Naming convention is assumed - test_someFeatureUnderTest.cpp
file( GLOB TestSources
      "${PROJECT_SOURCE_DIR}/test_*.cpp"
    )


###############################################################################
# Build executable

# test_main.cpp assumed to contain the main() function or the appropriate
# Catch2 define.
add_executable( ${PROJECT_NAME}
    main.cpp
    )
target_sources( ${PROJECT_NAME}
    PUBLIC ${TestSources}
    )
# target_compile_options( ${PROJECT_NAME}
#     PRIVATE
#     # -fconcepts
#     # -lstdc++fs
#     )
target_link_libraries( ${PROJECT_NAME}
    Project_config
    Lib::${Project}
    GTest::GTest
    GTest::Main
    )

###############################################################################
# CTest

# enable_testing()
add_test(
    ${PROJECT_NAME} ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
# gtest_discover_tests(${PROJECT_NAME} ${PROJECT_NAME}
#     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#     )
