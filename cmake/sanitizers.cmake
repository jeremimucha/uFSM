include(${CMAKE_CURRENT_LIST_DIR}/colors.cmake)

if(${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
  option(ENABLE_ASAN "Enable address-sanitizer for ALL Projects" OFF)
  option(ENABLE_LSAN "Enable leak-sanitizer for ALL Projects" OFF)
  option(ENABLE_MSAN "Enable memory-sanitizer for ALL Projects" OFF)
  option(ENABLE_TSAN "Enable thread-sanitizer for ALL Projects" OFF)
  option(ENABLE_UBSAN "Enable undefined-behavior-sanitizer for ALL Projects" OFF)
  message(STATUS "ENABLE_ASAN == ${ENABLE_ASAN}")
  message(STATUS "ENABLE_LSAN == ${ENABLE_LSAN}")
  message(STATUS "ENABLE_MSAN == ${ENABLE_MSAN}")
  message(STATUS "ENABLE_TSAN == ${ENABLE_TSAN}")
  message(STATUS "ENABLE_UBSAN == ${ENABLE_UBSAN}")
endif()
cmake_dependent_option(${PROJECT_NAME}_ASAN "Enable address-sanitizer for ${PROJECT_NAME}" OFF "NOT ENABLE_ASAN" ON)
cmake_dependent_option(${PROJECT_NAME}_LSAN "Enable leak-sanitizer for ${PROJECT_NAME}" OFF "NOT ENABLE_LSAN" ON)
cmake_dependent_option(${PROJECT_NAME}_MSAN "Enable memory-sanitizer for ${PROJECT_NAME}" OFF "NOT ENABLE_MSAN" ON)
cmake_dependent_option(${PROJECT_NAME}_TSAN "Enable thread-sanitizer for ${PROJECT_NAME}" OFF "NOT ENABLE_TSAN" ON)
cmake_dependent_option(${PROJECT_NAME}_UBSAN "Enable undefined-behavior-sanitizer for ${PROJECT_NAME}" OFF "NOT ENABLE_UBSAN" ON)
message(STATUS "${PROJECT_NAME}_ASAN == ${${PROJECT_NAME}_ASAN}")
message(STATUS "${PROJECT_NAME}_LSAN == ${${PROJECT_NAME}_LSAN}")
message(STATUS "${PROJECT_NAME}_MSAN == ${${PROJECT_NAME}_MSAN}")
message(STATUS "${PROJECT_NAME}_TSAN == ${${PROJECT_NAME}_TSAN}")
message(STATUS "${PROJECT_NAME}_UBSAN == ${${PROJECT_NAME}_UBSAN}")


function(ConfigureSanitizers)
  set(arg_kwargs NAMESPACE TARGET OUT)
  cmake_parse_arguments(arg "" "${arg_kwargs}" "" ${ARGN})
  foreach(unparsed_arg IN LISTS arg_UNPARSED_ARGUMENTS)
      message(WARNING "${ColorYellow}Unparsed argument: ${unparsed_arg}${ColorReset}")
  endforeach()

  if(NOT arg_NAMESPACE)
  set(arg_NAMESPACE ${PROJECT_NAME})
  endif()
  if(NOT arg_TARGET)
  set(arg_TARGET SanitizersConfig)
  endif()

  if(NOT arg_OUT)
  set(arg_OUT PROJECT_SANITIZERS_TARGET)
  endif()

  set(_targetName ${arg_NAMESPACE}_${arg_TARGET})
  message(STATUS "ConfigureSanitizers: available as ${_targetName} and \$\{${arg_OUT}\}")
  set(${arg_OUT} ${_targetName} PARENT_SCOPE)

  if(TARGET ${_targetName})
    return()
  endif()
  
  add_library(${_targetName} INTERFACE IMPORTED)
  add_library(${arg_NAMESPACE}::${arg_TARGET} ALIAS ${_targetName})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(Sanitizers_ "")

    if(${PROJECT_NAME}_ASAN)
      list(APPEND Sanitizers_ "address")
    endif()

    if(${PROJECT_NAME}_LSAN)
      list(APPEND Sanitizers_ "leak")
    endif()

    if(${PROJECT_NAME}_UBSAN)
      list(APPEND Sanitizers_ "undefined")
    endif()

    if(${PROJECT_NAME}_TSAN)
      if("address" IN_LIST Sanitizers_ OR "leak" IN_LIST Sanitizers_)
        message(WARNING "${ColorYellow}Thread sanitizer does not work with Address and Leak sanitizers. TSan disabled.")
      else()
        list(APPEND Sanitizers_ "thread")
      endif()
    endif()

    if(${PROJECT_NAME}_MSAN AND CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
      message(
        WARNING
          "Memory sanitizer requires all the code (including libc++) to be MSan-instrumented otherwise it reports false positives"
      )
      if("address" IN_LIST Sanitizers_
         OR "thread" IN_LIST Sanitizers_
         OR "leak" IN_LIST Sanitizers_)
        message(WARNING "${ColorYellow}Memory sanitizer does not work with Address, Thread and Leak sanitizer enabled. MSan ignored.${ColorReset}")
      else()
        list(APPEND Sanitizers_ "memory")
      endif()
    endif()
elseif(MSVC)
  if(${PROJECT_NAME}_ASAN)
    list(APPEND Sanitizers_ "address")
  endif()
  if(${PROJECT_NAME}_LSAN OR ${PROJECT_NAME}_MSAN OR ${PROJECT_NAME}_TSAN OR ${PROJECT_NAME}_UBSAN)
    message(WARNING "MSVC only supports address sanitizer")
  endif()
endif()

  list(JOIN Sanitizers_ "," ListOfSanitizers_)
  message(STATUS "${ColorGreen}Sanitizers = ${ListOfSanitizers_}${ColorReset}")

  if(ListOfSanitizers_ AND (NOT "${ListOfSanitizers_}" STREQUAL ""))
    if(NOT MSVC)
      target_compile_options(${_targetName}
                            INTERFACE -fsanitize=${ListOfSanitizers_})
                            #  INTERFACE -fno-omit-frame-pointer -fsanitize=${ListOfSanitizers_})
      target_link_options(${_targetName}
                            INTERFACE -fsanitize=${ListOfSanitizers_})
    else()
      string(FIND "$ENV{PATH}" "$ENV{VSINSTALLDIR}" index_of_vs_install_dir)
      if("${index-of_vs_install_dir}" STREQUAL "-1")
        message(SEND_ERROR
          "Using MSVC sanitizers requires setting the MSVC environment before building the project.\
          Please run the build from the MSVC command prompt."
        )
      endif()
      target_compile_options(${_targetName} INTERFACE /fsanitize=${ListOfSanitizers_} /Zi /INCREMENTAL:NO)
      target_link_options(${_targetName} INTERFACE /INCREMENTAL:NO)
    endif()
  endif()

endfunction()
