cmake_minimum_required(VERSION 3.15)

include(FetchContent)

if(uFSM_TESTS OR (CMAKE_SOURCE_DIR STREQUAL uFSM_SOURCE_DIR AND BUILD_TESTING))
    add_subdirectory(googletest)
endif()

# if(uFSM_BUILD_BENCHMARKS)
#     add_subdirectory(googlebenchmark)
# endif()

# After the content has been populated initially - speed up the configure stage by disabling updates
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Disable FetchContent updates" FORCE)
