cmake_minimum_required(VERSION 3.15)

FetchContent_Declare(googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark
    GIT_TAG v1.7.0
)

message(STATUS "${ColorGreen}Fetching googlebenchmark${ColorReset}")

set(BENCHMARK_ENABLE_LTO true)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(BENCHMARK_ENABLE_TESTING OFF)

FetchContent_MakeAvailable(googlebenchmark)
target_compile_options(benchmark
    PUBLIC
        $<$<CXX_COMPILER_ID:Clang,Gnu>:-Wno-unused-parameter>
)
get_target_property(_bench_include_dirs benchmark INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(benchmark
    SYSTEM INTERFACE
        ${_bench_include_dirs}
)
