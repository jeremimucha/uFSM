macro(DeclareInstallDirs)
    set(project_install_include_dir ${CMAKE_INSTALL_INCLUDEDIR})
    set(project_install_library_dir ${CMAKE_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME})
    set(project_install_runtime_dir ${CMAKE_INSTALL_BINDIR})
    set(project_export_targets      ${CMAKE_PROJECT_NAME}-targets)
endmacro()

macro(ConfigureInstallStep)
    set(project_config_install_dir ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME})
    set(project_config_file ${CMAKE_PROJECT_NAME}Config.cmake)
    set(project_config_template_file ${CMAKE_SOURCE_DIR}/cmake/${CMAKE_PROJECT_NAME}Config.cmake.in)
    set(project_config_version_file ${CMAKE_PROJECT_NAME}ConfigVersion.cmake)
    set(project_config_targets_file ${CMAKE_PROJECT_NAME}Targets.cmake)

    install(
        EXPORT ${project_export_targets}
        FILE ${project_config_targets_file}
        NAMESPACE ${CMAKE_PROJECT_NAME}::
        DESTINATION ${project_config_install_dir}
    )

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${project_config_version_file}
        VERSION ${CMAKE_PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )
    configure_package_config_file(
        ${project_config_template_file}
        ${CMAKE_CURRENT_BINARY_DIR}/${project_config_file}
        INSTALL_DESTINATION ${project_config_install_dir}
    )

    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/${project_config_file}
            ${CMAKE_CURRENT_BINARY_DIR}/${project_config_version_file}
        DESTINATION ${project_config_install_dir}
    )
    export(
        EXPORT ${project_export_targets}
        FILE ${CMAKE_CURRENT_BINARY_DIR}/${project_config_targets_file}
        NAMESPACE ${CMAKE_PROJECT_NAME}::
    )
    export(PACKAGE ${CMAKE_PROJECT_NAME})
endmacro()
