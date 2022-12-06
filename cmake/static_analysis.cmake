if((${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR}))
  option(ENABLE_CPPCHECK "Enable static analysis with cppcheck" OFF)
  option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
  option(ENABLE_IWYU "Enable static analysis with include-what-you-use" OFF)
  option(ENABLE_LWYU "Enable static analysis with link-what-you-use" OFF)
  message(STATUS "ENABLE_CPPCHECK == ${${PROJECT_NAME}_CPPCHECK}")
  message(STATUS "ENABLE_CLANG_TIDY == ${${PROJECT_NAME}_CLANG_TIDY}")
  message(STATUS "ENABLE_IWYU == ${${PROJECT_NAME}_IWYU}")
  message(STATUS "ENABLE_LWYU == ${${PROJECT_NAME}_LWYU}")
endif()
cmake_dependent_option(${PROJECT_NAME}_CPPCHECK "Enable static analysis with cppcheck" OFF "NOT ENABLE_CPPCHECK" ON)
cmake_dependent_option(${PROJECT_NAME}_CLANG_TIDY "Enable static analysis with clang-tidy" OFF "NOT ENABLE_CLANG_TIDY" ON)
cmake_dependent_option(${PROJECT_NAME}_IWYU "Enable static analysis with include-what-you-use" OFF "NOT ENABLE_IWYU" ON)
cmake_dependent_option(${PROJECT_NAME}_LWYU "Enable static analysis with link-what-you-use" OFF "NOT ENABLE_LWYU" ON)
message(STATUS "${PROJECT_NAME}_CPPCHECK == ${${PROJECT_NAME}_CPPCHECK}")
message(STATUS "${PROJECT_NAME}_CLANG_TIDY == ${${PROJECT_NAME}_CLANG_TIDY}")
message(STATUS "${PROJECT_NAME}_IWYU == ${${PROJECT_NAME}_IWYU}")
message(STATUS "${PROJECT_NAME}_LWYU == ${${PROJECT_NAME}_LWYU}")


function(ConfigureStaticAnalysis)
  if(${PROJECT_NAME}_CPPCHECK)
    find_program(Cppcheck_ cppcheck)
    if(Cppcheck_)
      set(CMAKE_CXX_CPPCHECK ${Cppcheck_}
        --enable=style,performance,warning,portability
        --suppress=missingInclude
        --suppress=internalAstError
        --suppress=unmatchedSuppression
        --suppress=passedByValue
        --suppress=syntaxError
        --inconclusive
      )
    else()
      message(SEND_ERROR
          "${ColorRed}cppcheck requested but executable not found${ColorReset}")
    endif()
  endif()

  if(${PROJECT_NAME}_CLANG_TIDY)
    find_program(Clangtidy_ clang-tidy)
    if(Clangtidy_)
      set(CMAKE_CXX_CLANG_TIDY ${Clangtidy_} -extra-arg=-Wno-unknown-warning-option)
    else()
      message(SEND_ERROR
          "${ColorRed}clang-tidy requested but executable not found${ColorReset}")
    endif()
  endif()

  if(${PROJECT_NAME}_IWYU)
    find_program(IncludeWhatYouUse_ include-what-you-use)
    if(IncludeWhatYouUse_)
      set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IncludeWhatYouUse_})
    else()
      message(SEND_ERROR
          "${ColorRed}include-what-you-use requested but executable not found${ColorReset}")
    endif()
  endif()

  if(${PROJECT_NAME}_LWYU)
      find_program(LinkWhatYouUse_ link-what-you-use)
      if(LinkWhatYouUse_)
          set(CMAKE_CXX_LINK_WHAT_YOU_USE ${LinkWhatYouUse_})
      else()
          message(SEND_ERROR
              "${ColorRed}link-what-you-use requested but executable not found${ColorReset}")
      endif()
  endif()
endfunction()


function(StaticAnalysisIgnoreBuildDirectory)
    # Ignore everything in the build directory from clang-tidy analysis.
    file(WRITE "${CMAKE_BINARY_DIR}/.clang-tidy" [===[
---
Checks: \"-*\"
]===]
)
endfunction()
