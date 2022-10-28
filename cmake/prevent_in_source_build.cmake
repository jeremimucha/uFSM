include_guard()

function(ensure_out_of_source_builds)
    get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
    get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)

    if("${srcdir} STREQUAL ${bindir}")
        message("=====================================")
        message(FATAL_ERROR
[[Error: In-source builds are disabled.
Please create a dedicated build directory or build out of source.]])
        message("=====================================")
    endif()
    
endfunction()
