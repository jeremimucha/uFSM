include(${CMAKE_CURRENT_LIST_DIR}/colors.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/prefix.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/options.cmake)

macro(SetDefaultBuildType)
    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        message(STATUS "${ColorYellow}Setting build type to 'Debug' as none was specified.${ColorReset}")
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    endif()
endmacro()

function(DeclareCompilerConfigTarget target)
    SetOptionNames()
    add_library(${target} INTERFACE)
    target_compile_features(${target}
        INTERFACE
            cxx_std_17
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        target_compile_options(${target} INTERFACE
            /W4
            /permissive-
            $<$<BOOL:${OPT_WERROR}>:/WX>
        )
    else()
        target_compile_options(${target} INTERFACE
        -Wall
        -Wdouble-promotion
        -Weffc++
        -Wextra
        -Wformat=2
        -Wnon-virtual-dtor
        -Wnull-dereference
        -Wold-style-cast
        -Woverloaded-virtual
        -Wpedantic
        -Wshadow
        # -Wsign-conversion
        -Wunused
        -Wcast-align
        -pedantic-errors

        -Wimplicit-fallthrough
        -Wsuggest-attribute=pure
        -Wsuggest-attribute=const
        -Wsuggest-attribute=cold
        -Wsuggest-final-types
        -Wsuggest-final-methods
        -Wduplicated-branches
        -Wduplicated-cond

        $<$<BOOL:${OPT_WERROR}>:-Werror>
        $<$<CONFIG:Debug>:-Og>
        $<$<CONFIG:Debug>:-ggdb3>
        # -fuse-ld=gold
        )
        target_link_options(${target} INTERFACE
            # -fuse-ld=gold
            $<$<BOOL:${${OPT_COVERAGE}}>:--coverage>
            $<$<BOOL:${${OPT_COVERAGE}}>:-ftest-coverage>
            $<$<BOOL:${${OPT_COVERAGE}}>:-fprofile-arcs>
            $<$<BOOL:${${OPT_SANITIZE_ADDRESS}}>:-fno-omit-frame-pointer>
            $<$<BOOL:${${OPT_SANITIZE_ADDRESS}}>:-fsanitize=address>
            $<$<BOOL:${${OPT_SANITIZE_LEAK}}>:-fsanitize=leak>
            $<$<BOOL:${${OPT_SANITIZE_THREAD}}>:-fsanitize=thread>
            $<$<BOOL:${${OPT_SANITIZE_UB}}>:-fsanitize=undefined>
        )
    endif()
endfunction()
