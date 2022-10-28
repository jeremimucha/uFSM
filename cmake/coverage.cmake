include_guard()

include(${CMAKE_CURRENT_LIST_DIR}/colors.cmake)

function(ConfigureCoverage)
  set(arg_options)
  set(arg_kwargs NAMESPACE TARGET OUT)
  set(arg_list_args)
  cmake_parse_arguments(arg "${arg_options}" "${arg_kwargs}" "${arg_list_args}" ${ARGN})
  foreach(unparsed_arg IN LISTS arg_UNPARSED_ARGUMENTS)
      message(WARNING "${ColorYellow}Unparsed argument: ${unparsed_arg}${ColorReset}")
  endforeach()

  AssertOptionsDefined()

  if(${OptCoverage_})
    if(NOT arg_NAMESPACE)
      set(arg_NAMESPACE ${PROJECT_NAME})
    endif()
    if(NOT arg_TARGET)
      set(arg_TARGET CoverageConfig)
    endif()

    if(NOT arg_OUT)
      set(arg_OUT PROJECT_COVERAGE_TARGET)
    endif()

    set(_targetName ${arg_NAMESPACE}_${arg_TARGET})
    message(STATUS "ConfigureCoverage: available as ${_targetName} and \$\{${arg_OUT}\}")
    set(${arg_OUT} ${_targetName} PARENT_SCOPE)

    if(TARGET ${_targetName})
      return()
    endif()
    
    add_library(${_targetName} INTERFACE IMPORTED)
    add_library(${arg_NAMESPACE}::${arg_TARGET} ALIAS ${_targetName})

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
      target_compile_options(${_targetName}
          INTERFACE --coverage -O0 -ggdb3
      )
      target_link_libraries(${_targetName}
          INTERFACE --coverage
      )
    endif()
  endif()
endfunction()
