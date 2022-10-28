include_guard()

function(enable_ipo)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _ipo_supported OUTPUT output)
    if(_ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON PARENT_SCOPE)
    else()
        message(SEND_ERROR "${ColorRed}IPO is not supported: ${output}${ColorReset}")
    endif()
endfunction()

