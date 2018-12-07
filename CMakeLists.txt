cmake_minimum_required( VERSION 3.1 )

set( Project uFSM )
project( ${Project} )


###############################################################################
# Prepare external dependencies
###############################################################################
add_subdirectory( external/google-test )


# Find any external libraries via find_backage
# see cmake --help-module-list and cmake --help-module ModuleName
# for details on a specific module
# If using boost
# find_package( Boost 1.60.0
#   REQUIRED COMPONENTS
#   filesystem system iostreams
#   )
#
# if( Boost_FOUND )
#   include_directories( ${Boost_INCLUDE_DIRS} )
# else()
#   message( FATAL_ERROR "Cannon find Boost" )
# endif()



###############################################################################
# Configure build
###############################################################################
# Set required C++ standard
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED TRUE )
# Set build type
if( NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message("Setting build type to 'Debug' as none was specified.")
  set( CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

# Export compile_commands.json for use with cppcheck
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

option(ENABLE_ASAN "Enable memory sanitizers" FALSE)
option(ENABLE_USAN "Enable undefined sanitizers" FALSE)
option(ENABLE_TSAN "Enable thread sanitizers" FALSE)
option(ENABLE_WERROR "Treat warnings as errors" FALSE)

if(CMAKE_COMPILER_IS_GNUCC)
  option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" FALSE)
endif()

add_library(Project_config INTERFACE)
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options( Project_config INTERFACE /W4 /WX )
else()
    if(CMAKE_BUILD_TYPE MATCHES Debug)
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
    endif()
    target_compile_options( Project_config INTERFACE
      -Wall
      -Wextra # reasonable and standard
      -Weffc++ # Warn about violations of Effective C++ style rules
      -Wshadow # warn the user if a variable declaration shadows one from a parent context
      -Wnon-virtual-dtor # warn the user if a class with virtual functions has a
                      # non-virtual destructor. This helps catch hard to track down memory errors
      -Wold-style-cast # warn for c-style casts
      -Wcast-align # warn for potential performance problem casts
      -Wunused # warn on anything being unused
      -Woverloaded-virtual # warn if you overload (not override) a virtual function
      -Wpedantic # warn if non-standard C++ is used
      -Wconversion # warn on type conversions that may lose data
      -Wsign-conversion # warn on sign conversions
      -Wnull-dereference # warn if a null dereference is detected
      -Wdouble-promotion # warn if float is implicit promoted to double
      -Wformat=2 # warn on security issues around functions that format output
              # (ie printf) 
    )
    if(ENABLE_WERROR)
      target_compile_options( Project_config INTERFACE
        -Werror
      )
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" )
      target_compile_options( Project_config INTERFACE
        -Wmisleading-indentation # warn if identation implies blocks where blocks do not exist
        -Wduplicated-cond # warn if if / else chain has duplicated conditions
        -Wduplicated-branches # warn if if / else branches have duplicated code
        -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
        -Wuseless-cast # warn if you perform a cast to the same type
      )
    endif()
    if(ENABLE_ASAN OR ENABLE_USAN OR ENABLE_TSAN)
      if(NOT CMAKE_BUILD_TYPE MATCHES "Debug")
        message(WARNING "Sanitizers used with build other than 'Debug' flags set -Og -g")
      endif()
      target_compile_options( Project_config INTERFACE
          -g
          -Og
      )
    endif()
    if(ENABLE_COVERAGE)
      target_compile_options( Project_config INTERFACE
          -fprofile-arcs
          -ftest-coverage
        #   --coverage  # only needed at linktime
      )
      target_link_libraries( Project_config INTERFACE
          -fprofile-arcs
          -ftest-coverage
          --coverage
      )
    endif()
    if(ENABLE_ASAN OR ENABLE_USAN OR ENABLE_TSAN)
    #   target_compile_options( Project_config INTERFACE
    #       -fuse-ld=gold   # only needed at linktime
    #   )
      target_link_libraries( Project_config INTERFACE
          -fuse-ld=gold
      )
    endif()
    if(ENABLE_ASAN)
      target_compile_options( Project_config INTERFACE
        -fno-omit-frame-pointer
        -fsanitize=address
        -fsanitize=leak
      )
      target_link_libraries( Project_config INTERFACE
          -fno-omit-frame-pointer
          -fsanitize=address
          -fsanitize=leak
      )
    endif()
    if(ENABLE_USAN)
      target_compile_options( Project_config INTERFACE
        -fsanitize=undefined
      )
      target_link_libraries( Project_config INTERFACE
          -fsanitize=undefined
      )
    endif()
    if(ENABLE_TSAN)
      target_compile_options( Project_config INTERFACE
        -fsanitize=thread
      )
      target_link_libraries( Project_config INTERFACE
          -fsanitize=thread
      )
    endif()
endif()

option(CPP_USE_CPPCHECK "Enable cppcheck build step" ON)
if(CPP_USE_CPPCHECK)
  find_program(Cppcheck NAMES cppcheck)
  if (Cppcheck)
      list(
          APPEND Cppcheck
              "--enable=all"
              "--inconclusive"
              "--force"
              "--verbose"
              "--language=c++"
              "--inline-suppr"
              "${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME}/*.h"
              "${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME}/*.hpp"
              "${CMAKE_SOURCE_DIR}/examples/*.cpp"
              "${CMAKE_SOURCE_DIR}/test/*.cpp"
      )
      message(STATUS ${Cppcheck})
  endif()
endif()

option(CPP_USE_CLANGTIDY "Enable clang-tidy build step" ON)
if(CPP_USE_CLANGTIDY)
  find_program(Clangtidy NAMES clang-tidy)
  if (Clangtidy)
      list(
          APPEND Clangtidy
      )
      message(STATUS ${Clangtidy})
  endif()
endif()
###############################################################################
# Build target library
###############################################################################

file( GLOB LibHeaders
      "${PROJECT_SOURCE_DIR}/include/ufsm/*.hpp"
    )

add_library( ${PROJECT_NAME}
    INTERFACE
    # ${LibHeaders}
    )
target_sources( ${PROJECT_NAME}
    INTERFACE
    ${LibHeaders}
    )
add_library( Lib::${PROJECT_NAME}
    ALIAS ${PROJECT_NAME}
    )
target_include_directories( ${PROJECT_NAME}
    INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/ufsm>
    $<INSTALL_INTERFACE:include>
    # PRIVATE
    # $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}>
    # $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
    )
target_compile_options( ${PROJECT_NAME}
    INTERFACE
    # -fconcepts
    # -lstdc++fs
    )
# target_compile_definitions( ${PROJECT_NAME}
#     INTERFACE
#     )
target_link_libraries( ${PROJECT_NAME}
    INTERFACE
    Project_config
    )

file( GLOB ExamplesSrc
    RELATIVE "${PROJECT_SOURCE_DIR}/examples/"
    "${PROJECT_SOURCE_DIR}/examples/*.cpp"
  )
foreach(Example ${ExamplesSrc})
    string(REGEX MATCH "^[^ .]*" ExampleName ${Example})
    add_executable(${ExampleName} "examples/${Example}")
    target_link_libraries(
        ${ExampleName}
        Project_config
        Lib::${PROJECT_NAME}
    )
    target_compile_definitions(
        ${ExampleName} PRIVATE
        FSM_DEBUG_LOG
        )
    # set_target_properties( ${ExampleName}
    #     PROPERTIES
    #     CXX_CPPCHECK $<$<BOOL:CPP_USE_CPPCHECK>:${Cppcheck}>
    #     CXX_CLANG_TIDY $<$<BOOL:CPP_USE_CLANGTIDY>:${Clangtidy}>
    #     )
    if(CPP_USE_CPPCHECK)
    set_target_properties( ${ExampleName}
        PROPERTIES
        CXX_CPPCHECK "${Cppcheck}"
    )
    endif()
    if(CPP_USE_CLANGTIDY)
        set_target_properties( ${ExampleName}
            PROPERTIES
            CXX_CLANG_TIDY "${Clangtidy}"
        )
    endif()
endforeach()

file( GLOB BenchmarksSrc
    RELATIVE "${PROJECT_SOURCE_DIR}/benchmarks/"
    "${PROJECT_SOURCE_DIR}/benchmarks/*.cpp"
    )

foreach(Benchmark ${BenchmarksSrc})
    string(REGEX MATCH "^[^ .]*" BenchmarkName ${Benchmark})
    add_executable(${BenchmarkName} "benchmarks/${Benchmark}")
    target_link_libraries(
        ${BenchmarkName}
        Project_config
        Lib::${PROJECT_NAME}
    )
    target_compile_definitions(
        ${BenchmarkName} PRIVATE
        $<$<CONFIG:Debug>:FSM_DEBUG_LOG>
        )
    # set_target_properties( ${BenchmarkName}
    #     PROPERTIES
    #     CXX_CPPCHECK $<$<BOOL:CPP_USE_CPPCHECK>:${Cppcheck}>
    #     CXX_CLANG_TIDY $<$<BOOL:CPP_USE_CLANGTIDY>:${Clangtidy}>
    #     )
    if(CPP_USE_CPPCHECK)
    set_target_properties( ${BenchmarkName}
        PROPERTIES
        CXX_CPPCHECK "${Cppcheck}"
    )
    endif()
    if(CPP_USE_CLANGTIDY)
        set_target_properties( ${BenchmarkName}
            PROPERTIES
            CXX_CLANG_TIDY "${Clangtidy}"
        )
    endif()
endforeach()
###############################################################################
# Unit Tests
###############################################################################
add_subdirectory( tests )

###############################################################################
# Install
###############################################################################
install( TARGETS ${PROJECT_NAME}
         DESTINATION lib
         LIBRARY
       )
