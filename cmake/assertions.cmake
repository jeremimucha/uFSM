include_guard()

function(ConfigureAssertions)
    set(arg_options)
    set(arg_kwargs NAMESPACE TARGET OUT)
    set(arg_list_args)
    cmake_parse_arguments(arg "${arg_options}" "${arg_kwargs}" "${arg_list_args}" ${ARGN})
    foreach(unparsed_arg IN LISTS arg_UNPARSED_ARGUMENTS)
        message(WARNING "${ColorYellow}Unparsed argument: ${unparsed_arg}${ColorReset}")
    endforeach()

    if(NOT arg_NAMESPACE)
    set(arg_NAMESPACE ${PROJECT_NAME})
    endif()
    if(NOT arg_TARGET)
    set(arg_TARGET AssertionsConfig)
    endif()

    if(NOT arg_OUT)
    set(arg_OUT PROJECT_ASSERTIONS_TARGET)
    endif()

    set(_targetName ${arg_NAMESPACE}_${arg_TARGET})
    message(STATUS "ConfigureAssertions: available as ${_targetName} and \$\{${arg_OUT}\}")
    set(${arg_OUT} ${_targetName} PARENT_SCOPE)
  
    if(TARGET ${_targetName})
      return()
    endif()
    
    add_library(${_targetName} INTERFACE IMPORTED)
    add_library(${arg_NAMESPACE}::${arg_TARGET} ALIAS ${_targetName})

    set(validAssertLevels OFF ASSUME IGNORED ENFORCE AUDIT)
    if (NOT ${PROJECT_NAME}_CONTRACT_LEVEL)
        set(allowedBuildTypes Debug RelWithDebInfo MinSizeRel Release)
        if(CMAKE_BUILD_TYPE STREQUAL Release)
            set(${PROJECT_NAME}_CONTRACT_LEVEL "ASSUME" CACHE STRING "Specify contract enforcement level")
        else()
            set(${PROJECT_NAME}_CONTRACT_LEVEL "ENFORCE" CACHE STRING "Specify contract enforcement level")
        endif()
        message(STATUS "${PROJECT_NAME}_CONTRACT_LEVEL unspecified - set to ${${PROJECT_NAME}_CONTRACT_LEVEL}; specify one of ${validAssertLevels}")
    endif()
    if (NOT ${PROJECT_NAME}_CONTRACT_LEVEL IN_LIST validAssertLevels)
        message(FATAL_ERROR "Invalid ${PROJECT_NAME}_CONTRACT_LEVEL specified valid values = ${validAssertLevels}")
    endif()
    target_compile_definitions(${_targetName}
        INTERFACE
            "${PROJECT_NAME}_CONTRACT_LEVEL=${${PROJECT_NAME}_CONTRACT_LEVEL}"
            "${PROJECT_NAME}_ASSERT_USE_DEBUGTRAP_HANDLER=1"
            $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>
    )
endfunction()
