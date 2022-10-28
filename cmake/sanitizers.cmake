include(${CMAKE_CURRENT_LIST_DIR}/colors.cmake)

function(ConfigureSanitizers)
  set(arg_kwargs NAMESPACE TARGET OUT)
  cmake_parse_arguments(arg "" "${arg_kwargs}" "" ${ARGN})
  foreach(unparsed_arg IN LISTS arg_UNPARSED_ARGUMENTS)
      message(WARNING "${ColorYellow}Unparsed argument: ${unparsed_arg}${ColorReset}")
  endforeach()

  AssertOptionsDefined()

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

    if(${OptSanitizeAddress_})
      list(APPEND Sanitizers_ "address")
    endif()

    if(${OptSanitizeLeak_})
      list(APPEND Sanitizers_ "leak")
    endif()

    if(${OptSanitizeUb_})
      list(APPEND Sanitizers_ "undefined")
    endif()

    if(${OptSanitizeThread_})
      if("address" IN_LIST Sanitizers_ OR "leak" IN_LIST Sanitizers_)
        message(WARNING "${ColorYellow}Thread sanitizer does not work with Address and Leak sanitizers. TSan disabled.")
      else()
        list(APPEND Sanitizers_ "thread")
      endif()
    endif()

    if(${OptSanitizeMemory_} AND CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
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
  if(${OptSanitizeAddress_})
    list(APPEND Sanitizers_ "address")
  endif()
  if(${OptSanitizeLeak_} OR ${OptSanitizeMemory_} OR ${OptSanitizeThread_} OR ${OptSanitizeUb_})
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
