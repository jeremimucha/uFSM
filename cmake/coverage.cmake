include(${CMAKE_CURRENT_LIST_DIR}/colors.cmake)

if((${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR}) OR (NOT DEFINED ENABLE_COVERAGE))
  option(ENABLE_COVERAGE "Generate ALL Projects code coverage" OFF)
  message(STATUS "ENABLE_COVERAGE = ${ENABLE_COVERAGE}")
endif()
cmake_dependent_option(${PROJECT_NAME}_COVERAGE "Generate ${PROJECT_NAME} code coverage" OFF "NOT ENABLE_COVERAGE" ON)
message(STATUS "${PROJECT_NAME}_COVERAGE = ${${PROJECT_NAME}_COVERAGE}")


function(ConfigureCoverage)
  set(arg_options)
  set(arg_kwargs NAMESPACE TARGET OUT)
  set(arg_list_args)
  cmake_parse_arguments(arg "${arg_options}" "${arg_kwargs}" "${arg_list_args}" ${ARGN})
  foreach(unparsed_arg IN LISTS arg_UNPARSED_ARGUMENTS)
      message(WARNING "${ColorYellow}Unparsed argument: ${unparsed_arg}${ColorReset}")
  endforeach()

  if(${PROJECT_NAME}_COVERAGE)
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
