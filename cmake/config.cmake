include(${CMAKE_CURRENT_LIST_DIR}/colors.cmake)

macro(SetOptionNames)
    set(prefix ${CMAKE_PROJECT_NAME})
    set(OPT_WERROR           ${prefix}_WERROR)
    set(OPT_TESTS            ${prefix}_TESTS)
    set(OPT_COVERAGE         ${prefix}_COVERAGE)
    set(OPT_SANITIZE_ADDRESS ${prefix}_SANITIZE_ADDRESS)
    set(OPT_SANITIZE_LEAK    ${prefix}_SANITIZE_LEAK)
    set(OPT_SANITIZE_THREAD  ${prefix}_SANITIZE_THREAD)
    set(OPT_SANITIZE_UB      ${prefix}_SANITIZE_UDEFINED_BEHAVIOR)
endmacro()

function(SetOptions)
    SetOptionNames()
    option(${OPT_WERROR} "Enable warning as errors for ${CMAKE_PROJECT_NAME}" ON)
    option(${OPT_TESTS} "Build ${CMAKE_PROJECT_NAME} tests" ON)
    option(${OPT_COVERAGE} "Generate ${CMAKE_PROJECT_NAME} code coverage" OFF)
    option(${OPT_SANITIZE_ADDRESS} "Enable address-sanitizer for ${CMAKE_PROJECT_NAME}" OFF)
    option(${OPT_SANITIZE_LEAK} "Enable leak-sanitizer for ${CMAKE_PROJECT_NAME}" OFF)
    option(${OPT_SANITIZE_THREAD} "Enable thread-sanitizer for ${CMAKE_PROJECT_NAME}" OFF)
    option(${OPT_SANITIZE_UB} "Enable undefined-behavior-sanitizer for ${CMAKE_PROJECT_NAME}" OFF)

    if (NOT CMAKE_BUILD_TYPE STREQUAL Debug)
        if (CODE_COVERAGE)
            message(FATAL_ERROR
                "${ColorBoldRed}CODE_COVERAGE=ON valid only for CMAKE_BUILD_TYPE=Debug${ColorReset}"
            )
        endif()
        if (SANITIZE_ADDRESS OR SANITIZE_LEAK OR SANITIZE_THREAD OR SANITIZE_UNDEFINED_BEHAVIOR)
            message(FATAL_ERROR
                "${ColorBoldRed}Sanitizers can be used only with CMAKE_BUILD_TYPE=Debug${ColorReset}"
            )
        endif()
    endif()
endfunction()

macro(SetDefaultBuildType)
    if( NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
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
