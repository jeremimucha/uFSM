include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/colors.cmake)

function(EnableCCache)
    option(ENABLE_CACHE "Enable cache if available" ON)
    message(STATUS "ENABLE_CACHE = ${ENABLE_CACHE}")
    if(NOT ENABLE_CACHE)
        return()
    endif()

    set(CACHE_OPTION "ccache" CACHE STRING "Compiler cache to be used")
    set(CACHE_OPTION_VALUES "ccache" "sccache")
    set_property(CACHE CACHE_OPTION PROPERTY STRINGS ${CACHE_OPTION_VALUES})
    list(FIND CACHE_OPTION_VALUES ${CACHE_OPTION} CACHE_OPTION_INDEX)

    if(${CACHE_OPTION_INDEX} EQUAL -1)
        message(STATUS [[
    Using custom compiler cache system: '${CACHE_OPTION}',
    explicitly supported entries are ${CACHE_OPTION_VALUES}]]
    )
    endif()

    find_program(CACHE_PROGRAM ${CACHE_OPTION})
    if(CACHE_PROGRAM)
        message(STATUS "${ColorGreen}${CACHE_OPTION} found and enabled${ColorReset}")
        set(CMAKE_C_COMPILER_LAUNCHER ${CACHE_PROGRAM} PARENT_SCOPE)
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CACHE_PROGRAM} PARENT_SCOPE)
        set(CMAKE_CUDA_COMPILER_LAUNCHER ${CACHE_PROGRAM} PARENT_SCOPE)
    else()
        message(STATUS
            "${ColorYellow}${CACHE_OPTION} is enabled but was not found. Not using it${ColorReset}")
    endif()
endfunction()
