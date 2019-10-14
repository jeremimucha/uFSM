include(${CMAKE_CURRENT_LIST_DIR}/colors.cmake)

function(ClangTidyTarget)
    set(options)
    set(args)
    set(list_args PATHS PATH_SUFFIXES)
    cmake_parse_arguments(arg "${options}" "${args}" "${list_args}" ${ARGN})
    foreach(unparsed_arg IN LISTS arg_UNPARSED_ARGUMENTS)
        message(WARNING "${ColorYellow}Unparsed argument: ${unparsed_arg}${ColorReset}")
    endforeach()

    find_program(RUN_CLANG_TIDY
        NAMES run-clang-tidy.py
        PATHS ${arg_PATHS}
        PATH_SUFFIXES ${arg_PATH_SUFFIXES}
        )
    if (NOT RUN_CLANG_TIDY)
        message(WARNING
            "${ColorYellow}run-clang-tidy.py not found; clang-tidy target unavailable${ColorReset}"
        )
    endif()
    set(RUN_CLANG_TIDY_COMMAND
        ${RUN_CLANG_TIDY}
            -extra-arg=-std=c++${CMAKE_CXX_STANDARD}
            # \"-header-filter=${CMAKE_SOURCE_DIR}/\(include|src|tests|examples|benchmarks\).*\"
            -p ${CMAKE_BINARY_DIR}
        )
    add_custom_target(
        clang-tidy
            COMMAND ${RUN_CLANG_TIDY_COMMAND}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        USES_TERMINAL
        )
    add_custom_target(
        clang-tidy-fix
            COMMAND ${RUN_CLANG_TIDY_COMMAND} -fix
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        VERBATIM
        USES_TERMINAL
        )
endfunction()
