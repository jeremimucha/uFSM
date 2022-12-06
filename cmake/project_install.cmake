include_guard()
include(GNUInstallDirs)
#[[ ConfigureProjectInstallDirectories
    Declares project-specific global variables used as a point of reference when installing
    custom components.
    This macro needs to be called after declaring the project for which target will be installed,
    and before the call to InstallProject() for this particular project.
]]
macro(ConfigureProjectInstallDirectories)
    set(${PROJECT_NAME}_export_targets      ${PROJECT_NAME}-targets)
endmacro()


#[[ InstallProject
    Installs targets for the given project and all necessary scripting to support CMake find_package
    along with versioning. Relies on previous call to ConfigureProjectInstallDirectories()
]]
function(InstallProject)
    set(options)
    set(args)
    set(list_args TARGETS DEPENDENCIES)

    cmake_parse_arguments(arg "${options}" "${args}" "${list_args}" ${ARGN})

    foreach(unparsed_arg IN LISTS arg_UNPARSED_ARGUMENTS)
        message(WARNING "${ColorYellow}Unparsed argument: ${unparsed_arg}${ColorReset}")
    endforeach()

    if(NOT arg_TARGETS)
        message(FATAL_ERROR "${ColorRed}InstallProject: at lest one TARGETS argument required${ColorReset}")
    endif()

    set(project_config_file ${PROJECT_NAME}Config.cmake)
    set(project_config_version_file ${PROJECT_NAME}ConfigVersion.cmake)
    set(project_runtime_component ${PROJECT_NAME}_Runtime)
    set(project_development_component ${PROJECT_NAME}_Development)

    #[[ If a project defines a packageConfig.cmake.in in its ${PROJECT_SOURCE_DIR} or in ${PROJECT_SOURCE_DIR}/cmake
        use that custom config file template.
        Fallback on the generic packageConfig.cmake.in defined here.
    ]]
    find_file(PACKAGE_CONFIG_TEMPLATE_FILE
        NAMES ${PROJECT_NAME}Config.cmake.in ${PROJECT_NAME}-config.cmake.in packageConfig.cmake.in
        PATHS "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_LIST_DIR}"
        NO_DEFAULT_PATH
    )
    if(NOT PACKAGE_CONFIG_TEMPLATE_FILE)
        message(FATAL_ERROR "Failed to find packageConfig.cmake.in. Can not package the project.")
    endif()

    install(
        TARGETS ${arg_TARGETS}
        EXPORT ${${PROJECT_NAME}_export_targets}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT           ${project_runtime_component}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT           ${project_runtime_component}
            NAMELINK_COMPONENT  ${project_development_component}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT           ${project_development_component}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${project_config_version_file}
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    set(InstallProject_INCLUDE_INSTALL_DIR include/)
    set(InstallProject_DEPENDENCIES ${arg_DEPENDENCIES})
    set(_pathVars InstallProject_INCLUDE_INSTALL_DIR)
    if(arg_DEPENDENCIES)
        list(APPEND _pathVars InstallProject_DEPENDENCIES)
    endif()

    configure_package_config_file(
        ${PACKAGE_CONFIG_TEMPLATE_FILE}
        ${CMAKE_CURRENT_BINARY_DIR}/${project_config_file}
        INSTALL_DESTINATION cmake
        PATH_VARS ${_pathVars}
    )

    install(
        EXPORT ${${PROJECT_NAME}_export_targets}
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION cmake
    )

    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/${project_config_file}
            ${CMAKE_CURRENT_BINARY_DIR}/${project_config_version_file}
        DESTINATION cmake
    )
    export(
        EXPORT ${${PROJECT_NAME}_export_targets}
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
    )
    export(PACKAGE ${PROJECT_NAME})
endfunction()
