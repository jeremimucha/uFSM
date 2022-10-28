include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/assertions.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/colors.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/options.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/build_type.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/build_cache.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/coverage.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/doxygen.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/enable_ipo.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/output_directories.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/prevent_in_source_build.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/project_install.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/sanitizers.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/static_analysis.cmake)

if (NOT opt_options_defined_)
message(STATUS "
${ColorGreen}Configuring default project option names.
To override the names include `options.cmake` and call
    DefineOptions(PREFIX myPrefix)
Before including project_config.cmake ${ColorReset}
")

    DefineOptions(PREFIX ${PROJECT_NAME})
endif()

macro(ConfigureGlobalFlags)
    set(CMAKE_CXX_STANDARD            20)
    set(CMAKE_CXX_STANDARD_REQUIRED   ON)
    set(CMAKE_CXX_EXTENSIONS          OFF)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    # Sane RUNPATH/RPATH for Linux
    file(RELATIVE_PATH relativeRpath
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}
    )
    set(CMAKE_INSTALL_RPATH $ORIGIN $ORIGIN/${relativeRpath})
    # set(CMAKE_INSTALL_RPATH $ORIGIN $ORIGIN/../lib)

    if(${OptIpo_})
        enable_ipo()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # add_link_options(-fuse-ld=gold)
    endif()

    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<BOOL:${BUILD_SHARED_LIBS}>:DLL>")

    ConfigureStaticAnalysis()
    StaticAnalysisIgnoreBuildDirectory()
endmacro()

function(ProjectConfigTarget)
    set(arg_options)
    set(arg_kwargs NAMESPACE TARGET OUT)
    set(arg_list_args)
    cmake_parse_arguments(arg "${arg_options}" "${arg_kwargs}" "${arg_list_args}" ${ARGN})
    foreach(unparsed_arg IN LISTS arg_UNPARSED_ARGUMENTS)
        message(WARNING "${ColorYellow}Unparsed argument: ${unparsed_arg}${ColorReset}")
    endforeach()

    AssertOptionsDefined()

    if(NOT arg_NAMESPACE)
        set(arg_NAMESPACE ${PROJECT_NAME})
    endif()
    if(NOT arg_TARGET)
        set(arg_TARGET ProjectConfig)
    endif()

    if(NOT arg_OUT)
        set(arg_OUT PROJECT_CONFIG_TARGET)
    endif()

    set(_targetName ${arg_NAMESPACE}_${arg_TARGET})
    message(STATUS "ProjectConfigTarget: available as ${_targetName} and \$\{${arg_OUT}\}")

    add_library(${_targetName} INTERFACE IMPORTED)
    set(${arg_OUT} ${_targetName} PARENT_SCOPE)
    add_library(${arg_NAMESPACE}::${arg_TARGET} ALIAS ${_targetName})
    # set_target_properties(${_targetName} PROPERTIES EXPORT_NAME ${arg_TARGET})

    set(MSVC_WARNINGS
        /W4          # Baseline reasonable warnings
        /w14242      # 'identifier': conversion from 'type1' to 'type1', possible loss of data
        /w14254      # 'operator': conversion from 'type1:field_bits' to
                     # 'type2:field_bits', possible loss of data
        /w14263      # 'function': member function does not override any base class
                     # virtual member function
        /w14265      # 'classname': class has virtual functions, but destructor is not
                     # virtual instances of this class may not be destructed correctly
        /w14287      # 'operator': unsigned/negative constant mismatch
        /we4289      # nonstandard extension used: 'variable': loop control variable
                     # declared in the for-loop is used outside the for-loop scope
        /w14296      # 'operator': expression is always 'boolean_value'
        /w14311      # 'variable': pointer truncation from 'type1' to 'type2'
        /w14545      # expression before comma evaluates to a function which is missing
                     # an argument list
        /w14546      # function call before comma missing argument list
        /w14547      # 'operator': operator before comma has no effect; expected
                     # operator with side-effect
        /w14549      # 'operator': operator before comma has no effect; did you intend
                     # 'operator'?
        /w14555      # expression has no effect; expected expression with side- effect
        /w14619      # pragma warning: there is no warning number 'number'
        /w14640      # Enable warning on thread un-safe static member initialization
        /w14826      # Conversion from 'type1' to 'type_2' is sign-extended. This may
                     # cause unexpected runtime behavior.
        /w14905      # wide string literal cast to 'LPSTR'
        /w14906      # string literal cast to 'LPWSTR'
        /w14928      # illegal copy-initialization; more than one user-defined
                     # conversion has been implicitly applied
        /permissive- # standards conformance mode for MSVC compiler.
        $<$<BOOL:${${OptWerror_}}>:/WX>
    )

    set(CLANG_WARNINGS
        -Wall
        -Wextra
        $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
        -Wdouble-promotion      # warn if float is implicit promoted to double
        -Wconversion            # warn on type conversions that may lose data
        -Wformat=2              # warn on printf security issues
        $<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>      # warn the user if a class with virtual functions has a
                                # non-virtual destructor
        -Wnull-dereference      # warn if a null dereference is detected
        # $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>        # warn for c-style casts
        $<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>    # warn if you overload (not override) a virtual function
        -Wpedantic              # warn if non-standard C++ is used
        -Wshadow                # warn the user if a variable declaration shadows one from a
                                # parent context
        -Wsign-conversion       # warn on sign conversions
        -Wunused                # warn on anything being unused
        -Wcast-align            # warn for potential performance problem casts
        -Wimplicit-fallthrough
        $<$<BOOL:${${OptWerror_}}>:-Werror>
        $<$<CONFIG:Debug>:-ggdb3>
        # $<$<CONFIG:Debug>:-Og>
    )
    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wsuggest-attribute=pure
        -Wsuggest-attribute=cold
        -Wsuggest-final-types
        -Wsuggest-final-methods
        -Wdeprecated-declarations
        -Wduplicated-cond
        -Wduplicated-branches    # warn if if / else branches have duplicated code
        -Wlogical-op             # warn about logical operations being used where bitwise were
                                 # probably wanted
        # $<$<COMPILE_LANGUAGE:CXX>:-Wuseless-cast>           # warn if you perform a cast to the same type
        -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        target_compile_options(${_targetName} INTERFACE
            ${MSVC_WARNINGS}
            "/Zc:__cplusplus"   # correct handling of __cplusplus macro by MSVC
                                # see https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        target_compile_options(${_targetName} INTERFACE
            ${CLANG_WARNINGS}
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${_targetName} INTERFACE
            ${GCC_WARNINGS}
        )
    else()
        message(AUTHOR_WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
    endif()

    ConfigureCoverage()
    ConfigureSanitizers()
    ConfigureAssertions()

    target_link_libraries(${_targetName} INTERFACE ${PROJECT_COVERAGE_TARGET})
    target_link_libraries(${_targetName} INTERFACE ${PROJECT_SANITIZERS_TARGET})
    target_link_libraries(${_targetName} INTERFACE ${PROJECT_ASSERTIONS_TARGET})
endfunction()
