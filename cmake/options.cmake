include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/colors.cmake)

macro(SetOptionNames prefix)
    set(EnableDeveloperMode_      ENABLE_DEVELOPER_MODE CACHE INTERNAL "")
    set(EnableWerror_             ENABLE_WERROR CACHE INTERNAL "")
    set(EnableIpo_                ENABLE_IPO CACHE INTERNAL "")
    set(EnableTests_              ENABLE_TESTS CACHE INTERNAL "")
    set(EnableDoxygen_            ENABLE_DOXYGEN CACHE INTERNAL "")
    set(EnableCoverage_           ENABLE_COVERAGE CACHE INTERNAL "")
    set(EnableSanitizeAddress_    ENABLE_SANITIZE_ADDRESS CACHE INTERNAL "")
    set(EnableSanitizeLeak_       ENABLE_SANITIZE_LEAK CACHE INTERNAL "")
    set(EnableSanitizeMemory_     ENABLE_SANITIZE_MEMORY CACHE INTERNAL "")
    set(EnableSanitizeThread_     ENABLE_SANITIZE_THREAD CACHE INTERNAL "")
    set(EnableSanitizeUb_         ENABLE_SANITIZE_UB CACHE INTERNAL "")
    set(EnableCppcheck_           ENABLE_CPPCHECK CACHE INTERNAL "")
    set(EnableClangTidy_          ENABLE_CLANG_TIDY CACHE INTERNAL "")
    set(EnableIwyu_               ENABLE_IWYU CACHE INTERNAL "")
    set(EnableLwyu_               ENABLE_LWYU CACHE INTERNAL "")
    set(OptDeveloperMode_      ${prefix}_DEVELOPER_MODE CACHE INTERNAL "")
    set(OptWerror_             ${prefix}_WERROR CACHE INTERNAL "")
    set(OptIpo_                ${prefix}_IPO CACHE INTERNAL "")
    set(OptTests_              ${prefix}_TESTS CACHE INTERNAL "")
    set(OptDoxygen_            ${prefix}_DOXYGEN CACHE INTERNAL "")
    set(OptCoverage_           ${prefix}_COVERAGE CACHE INTERNAL "")
    set(OptSanitizeAddress_    ${prefix}_SANITIZE_ADDRESS CACHE INTERNAL "")
    set(OptSanitizeLeak_       ${prefix}_SANITIZE_LEAK CACHE INTERNAL "")
    set(OptSanitizeMemory_     ${prefix}_SANITIZE_MEMORY CACHE INTERNAL "")
    set(OptSanitizeThread_     ${prefix}_SANITIZE_THREAD CACHE INTERNAL "")
    set(OptSanitizeUb_         ${prefix}_SANITIZE_UB CACHE INTERNAL "")
    set(OptCppcheck_           ${prefix}_CPPCHECK CACHE INTERNAL "")
    set(OptClangTidy_          ${prefix}_CLANG_TIDY CACHE INTERNAL "")
    set(OptIwyu_               ${prefix}_IWYU CACHE INTERNAL "")
    set(OptLwyu_               ${prefix}_LWYU CACHE INTERNAL "")
endmacro()

macro(ShowOptions)
    message(STATUS "${ColorYellow}${EnableDeveloperMode_}     = ${${EnableDeveloperMode_}} | ${OptDeveloperMode_}   = ${${OptDeveloperMode_}}  ${ColorReset}")
    message(STATUS "${ColorYellow}${EnableWerror_}             = ${${EnableWerror_}} | ${OptWerror_}            = ${${OptWerror_}}  ${ColorReset}")
    message(STATUS "${ColorYellow}${EnableIpo_}                = ${${EnableIpo_}} | ${OptIpo_}               = ${${OptIpo_}}  ${ColorReset}")
    message(STATUS "${ColorYellow}${EnableTests_}              = ${${EnableTests_}} | ${OptTests_}             = ${${OptTests_}}  ${ColorReset}")
    message(STATUS "${ColorYellow}${EnableDoxygen_}            = ${${EnableDoxygen_}} | ${OptDoxygen_}           = ${${OptDoxygen_}}  ${ColorReset}")
    message(STATUS "${ColorYellow}${EnableCoverage_}           = ${${EnableCoverage_}} | ${OptCoverage_}          = ${${OptCoverage_}}  ${ColorReset}")
    message(STATUS "${ColorYellow}${EnableSanitizeAddress_}   = ${${EnableSanitizeAddress_}} | ${OptSanitizeAddress_}  = ${${OptSanitizeAddress_}}  ${ColorReset}")
    message(STATUS "${ColorYellow}${EnableSanitizeLeak_}      = ${${EnableSanitizeLeak_}} | ${OptSanitizeLeak_}     = ${${OptSanitizeLeak_}}  ${ColorReset}")
    message(STATUS "${ColorYellow}${EnableSanitizeMemory_}    = ${${EnableSanitizeMemory_}} | ${OptSanitizeMemory_}   = ${${OptSanitizeMemory_}}  ${ColorReset}")
    message(STATUS "${ColorYellow}${EnableSanitizeThread_}    = ${${EnableSanitizeThread_}} | ${OptSanitizeThread_}   = ${${OptSanitizeThread_}}  ${ColorReset}")
    message(STATUS "${ColorYellow}${EnableSanitizeUb_}        = ${${EnableSanitizeUb_}} | ${OptSanitizeUb_}       = ${${OptSanitizeUb_}}  ${ColorReset}")
    message(STATUS "${ColorYellow}${EnableCppcheck_}           = ${${EnableCppcheck_}} | ${OptCppcheck_}          = ${${OptCppcheck_}}  ${ColorReset}")
    message(STATUS "${ColorYellow}${EnableClangTidy_}         = ${${EnableClangTidy_}} | ${OptClangTidy_}        = ${${OptClangTidy_}}  ${ColorReset}")
    message(STATUS "${ColorYellow}${EnableIwyu_}               = ${${EnableIwyu_}} | ${OptIwyu_}              = ${${OptIwyu_}}  ${ColorReset}")
    message(STATUS "${ColorYellow}${EnableLwyu_}               = ${${EnableLwyu_}} | ${OptLwyu_}              = ${${OptLwyu_}}  ${ColorReset}")
endmacro()

#[[
SetOptions - adds options for controlling the build process.
    Options names are prefixed with the given PREFIX argument.
    Given a PREFIX = "EXAMPLE", the following options are added:

    EXAMPLE_WERROR
    EXAMPLE_TESTS
    EXAMPLE_COVERAGE
    EXAMPLE_SANITIZE_ADDRESS
    EXAMPLE_SANITIZE_LEAK
    EXAMPLE_SANITIZE_Memory
    EXAMPLE_SANITIZE_THREAD
    EXAMPLE_SANITIZE_UNDEFINED_BEHAVIOR

    All of the options are set to OFF by default.
]]
macro(DefineOptions)
    set(options)
    set(args PREFIX)
    set(list_args)
    cmake_parse_arguments(defineOptions "${options}" "${args}" "${list_args}" ${ARGN})

    foreach(unparsed_arg IN LISTS arg_UNPARSED_ARGUMENTS)
        message(WARNING "${ColorYellow}Unparsed argument: ${unparsed_arg}${ColorReset}")
    endforeach()

    if(NOT defineOptions_PREFIX)
        # message(FATAL_ERROR "${ColorRed}DefineOptions: PREFIX argument required${ColorReset}")
        message(STATUS "${ColorYello}DefineOptions: PREFIX defaulted to ${PROJECT_NAME}${ColorReset}")
        set(defineOptions_PREFIX ${PROJECT_NAME})
    endif()

    if(PROJECT_NAME)
        set(opt_options_defined_ ${PROJECT_NAME}_OptionsDefined_)
    else()
        set(opt_options_defined_ DefaultProject_OptionsDefined_)
    endif()
    set(${opt_options_defined_} TRUE CACHE INTERNAL "")

    include(CMakeDependentOption)
    SetOptionNames(${defineOptions_PREFIX})

    if(${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
        option(${EnableDeveloperMode_} "Enable set of options suitable for development for ALL Projects" OFF)
        cmake_dependent_option(${EnableWerror_} "Enable warning as errors for ALL Projects" OFF "NOT ${EnableDeveloperMode_}" ON)
        cmake_dependent_option(${EnableTests_} "Build ALL Projects tests" OFF "NOT ${EnableDeveloperMode_}" ON)
        option(${EnableIpo_} "Enable Interprocedural Optimization, aka Link Time Optimization (LTO)" OFF)
        option(${EnableDoxygen_} "Build ALL Projects documentation" OFF)
        option(${EnableCoverage_} "Generate ALL Projects code coverage" OFF)
        option(${EnableSanitizeAddress_} "Enable address-sanitizer for ALL Projects" OFF)
        option(${EnableSanitizeLeak_} "Enable leak-sanitizer for ALL Projects" OFF)
        option(${EnableSanitizeMemory_} "Enable memory-sanitizer for ALL Projects" OFF)
        option(${EnableSanitizeThread_} "Enable thread-sanitizer for ALL Projects" OFF)
        option(${EnableSanitizeUb_} "Enable undefined-behavior-sanitizer for ALL Projects" OFF)
        option(${EnableCppcheck_} "Enable static analysis with cppcheck" OFF)
        option(${EnableClangTidy_} "Enable static analysis with clang-tidy" OFF)
        option(${EnableIwyu_} "Enable static analysis with include-what-you-use" OFF)
        option(${EnableLwyu_} "Enable static analysis with link-what-you-use" OFF)
    endif()

    cmake_dependent_option(${OptDeveloperMode_} "Enable set of options suitable for development for ${PROJECT_NAME}" OFF "NOT ${EnableDeveloperMode_}" ON)
    cmake_dependent_option(${OptWerror_} "Enable warning as errors for ${PROJECT_NAME}" OFF "NOT ${OptDeveloperMode_} OR NOT ${EnableWerror_}" ON)
    cmake_dependent_option(${OptTests_} "Build ${PROJECT_NAME} tests" OFF "NOT ${OptDeveloperMode_} OR NOT ${EnabletTests_}" ON)
    cmake_dependent_option(${OptCoverage_} "Generate ${PROJECT_NAME} code coverage" OFF "NOT ${EnableCoverage_}" ON)
    cmake_dependent_option(${OptCppcheck_} "Enable static analysis with cppcheck" OFF "NOT ${EnableCppcheck_}" ON)
    cmake_dependent_option(${OptClangTidy_} "Enable static analysis with clang-tidy" OFF "NOT ${EnableClangTidy_}" ON)
    cmake_dependent_option(${OptDoxygen_} "Build ${PROJECT_NAME} documentation" OFF "NOT ${EnableDoxygen_}" ON)
    cmake_dependent_option(${OptIpo_} "Enable Interprocedural Optimization, aka Link Time Optimization (LTO)" OFF "NOT ${EnableIpo_}" ON)
    cmake_dependent_option(${OptSanitizeAddress_} "Enable address-sanitizer for ${PROJECT_NAME}" OFF "NOT ${EnableSanitizeAddress_}" ON)
    cmake_dependent_option(${OptSanitizeLeak_} "Enable leak-sanitizer for ${PROJECT_NAME}" OFF "NOT ${EnableSanitizeLeak_}" ON)
    cmake_dependent_option(${OptSanitizeMemory_} "Enable memory-sanitizer for ${PROJECT_NAME}" OFF "NOT ${EnableSanitizeMemory_}" ON)
    cmake_dependent_option(${OptSanitizeThread_} "Enable thread-sanitizer for ${PROJECT_NAME}" OFF "NOT ${EnableSanitizeThread_}" ON)
    cmake_dependent_option(${OptSanitizeUb_} "Enable undefined-behavior-sanitizer for ${PROJECT_NAME}" OFF "NOT ${EnableSanitizeUb_}" ON)
    cmake_dependent_option(${OptIwyu_} "Enable static analysis with include-what-you-use" OFF "NOT ${EnableIwyu_}" ON)
    cmake_dependent_option(${OptLwyu_} "Enable static analysis with link-what-you-use" OFF "NOT ${EnableLwyu_}" ON)
endmacro()

function(AssertOptionsDefined)
if (NOT $CACHE{opt_options_defined_})
    message(FATAL_ERROR "
${ColorGreen}Configure project option names by including
`options.cmake` and calling
    DefineOptions(PREFIX myPrefix)
Before including project_config.cmake
${ColorReset}")
    endif()
endfunction()
