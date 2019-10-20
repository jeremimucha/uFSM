include(${CMAKE_CURRENT_LIST_DIR}/colors.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/prefix.cmake)

function(CoverageTargets)
    GetProjectPrefix(prefix)
    if(${OPT_COVERAGE})
        set(COVERAGE_DIR ${CMAKE_BINARY_DIR}/coverage)
        file(MAKE_DIRECTORY ${COVERAGE_DIR})

        find_program(GCOV NAMES gcov gcov-9 gcov-8)
        if (NOT GCOV)
            message(FATAL_ERROR "${ColorRed}Failed to find gcov${ColorReset}")
        endif()
        find_program(GCOVR gcovr)
        if (NOT GCOVR)
            message(FATAL_ERROR "${ColorRed}Failed to find gcovr${ColorReset}")
        endif()

        set(COVERAGE_COMMAND
            ${GCOVR}
                --gcov-executable ${GCOV}
                --root ${CMAKE_SOURCE_DIR}
                --exclude-directories "build.*"
                --print-summary
                --exclude-unreachable-branches
                --object-directory ${CMAKE_BINARY_DIR}
        )
        set(COVERAGE_COMMAND_HTML
            ${COVERAGE_COMMAND}
            --html
            --html-details
            --output ${COVERAGE_DIR}/index.html
        )
        set(COVERAGE_COMMAND_XML
            ${COVERAGE_COMMAND}
                --xml-pretty
                --output ${COVERAGE_DIR}/coverage.xml
        )
        add_custom_target(coverage
            COMMAND ${COVERAGE_COMMAND_HTML}
            USES_TERMINAL
        )
        add_custom_target(coverage-xml
            COMMAND ${COVERAGE_COMMAND_XML}
            USES_TERMINAL
        )
        CoverageClearCommand(COVERAGE_COMMAND_CLEAR)
        add_custom_target(coverage-clear
            COMMAND ${COVERAGE_COMMAND_CLEAR}
            USES_TERMINAL
        )
    endif()
endfunction()

function(CoverageClearCommand out)
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        message(STATUS "${ColorYellow}MSVC toolchain detected: coverage-clear target is no-op${ColorReset}")
        set(${out} "")
    else()
        set(${out}
            find ${CMAKE_BINARY_DIR}/ -name "*.gcda" -delete
            PARENT_SCOPE
        )
    endif()
endfunction()
