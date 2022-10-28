cmake_minimum_required(VERSION 3.15)

include(FetchContent)
include(GNUInstallDirs)
include("${CMAKE_CURRENT_LIST_DIR}/../get_win32_ver.cmake")


FetchContent_Declare(asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-18-0
)


message(STATUS "${ColorGreen}Fetching ASIO${ColorReset}")
FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
    FetchContent_Populate(asio)

    add_library(asio_asio INTERFACE)
    add_library(asio::asio ALIAS asio_asio)
    set_target_properties(asio_asio PROPERTIES EXPORT_NAME asio)

    target_include_directories(asio_asio
        SYSTEM INTERFACE
            $<BUILD_INTERFACE:${asio_SOURCE_DIR}/asio/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    target_compile_definitions(asio_asio
        INTERFACE
            ASIO_STANDALONE
    )

    if (WIN32)
        # function(get_WIN32_WINNT out_name)
        #     if(CMAKE_SYSTEM_VERSION)
        #         set(ver ${CMAKE_SYSTEM_VERSION})
        #         string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
        #         string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
        #         # Check for Windows 10, b/c we'll need to convert to hex 'A'.
        #         if("${verMajor}" MATCHES "10")
        #             set(verMajor "A")
        #             string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
        #         endif()
        #         # Remove all remaining '.' characters.
        #         string(REPLACE "." "" ver ${ver})
        #         # Prepend each digit with a zero.
        #         string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
        #         set(${out_name} "0x${ver}" PARENT_SCOPE)
        #     endif()
        # endfunction()

        target_link_libraries(asio_asio INTERFACE ws2_32 wsock32)
        get_win32_winnt_ver(winnt_ver)
        message(STATUS "WIN32_WINNT version: ${winnt_ver}")
        target_compile_definitions(asio_asio INTERFACE
            _WIN32_WINNT=${winnt_ver}
        )
    endif()

    install(
        DIRECTORY ${asio_SOURCE_DIR}/asio/include
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        PATTERN Makefile.am EXCLUDE
        PATTERN .gitignore EXCLUDE
    )

endif()
