cmake_minimum_required(VERSION 3.11)
project(KemaiProject VERSION 0.12.0 LANGUAGES CXX)

# Point CMake to the custom modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/cmake)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

# Common configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(KEMAI_ENABLE_UPDATE_CHECK "Allow Kemai to check for update from Github releases" ON)
option(KEMAI_BUILD_LOCAL_DEPENDENCIES "Download and build dependencies (except Qt6)" ON)

# Install dependencies sources to build if required
if (KEMAI_BUILD_LOCAL_DEPENDENCIES)
    set(SPDLOG_FMT_EXTERNAL ON)

    include(FetchContent)
    FetchContent_Declare(fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt
            GIT_TAG 10.2.1
            OVERRIDE_FIND_PACKAGE)
    FetchContent_Declare(magic_enum
            GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
            GIT_TAG v0.9.5
            OVERRIDE_FIND_PACKAGE)
    FetchContent_Declare(spdlog
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG v1.14.0
            OVERRIDE_FIND_PACKAGE)
    FetchContent_Declare(range-v3
            GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
            GIT_TAG 0.12.0
            OVERRIDE_FIND_PACKAGE)
endif (KEMAI_BUILD_LOCAL_DEPENDENCIES)

# Find external dependencies
find_package(fmt REQUIRED)
find_package(magic_enum REQUIRED)
find_package(spdlog REQUIRED)
find_package(range-v3 REQUIRED)

# Setup Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 QUIET COMPONENTS Widgets Network LinguistTools)
if (NOT Qt6_FOUND)
    message(NOTICE "Qt6 not found. Fallback to Qt5")
    find_package(Qt5 COMPONENTS Widgets Network LinguistTools REQUIRED)
    add_compile_definitions(UNICODE _UNICODE)
    message(STATUS "Using Qt ${Qt5_VERSION}")
else ()
    message(STATUS "Using Qt ${Qt6_VERSION}")
endif ()

# Write version to file to ease packaging
file(WRITE ${CMAKE_BINARY_DIR}/version.txt ${PROJECT_VERSION})

# Common package configuration
set(KEMAI_GUID "88815E44-85A0-469C-9740-B4887D456BAA")
set(KEMAI_PROJECT_NAME "Kemai")
set(CPACK_PACKAGE_NAME ${KEMAI_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

# OS Dependant options / configurations
if (WIN32)
    # Hide console
    if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        set(OS_BUNDLE WIN32)
    endif ()

    # Add Qt libs and installer for windows platform
    include(WinDeployQt)

    # Add OpenSSL binaries for installer
    include(WinDeployOpenSSL)

    # Package
    set(DEPLOY_DIR .)
    set(CPACK_GENERATOR WIX)
    set(CPACK_WIX_UPGRADE_GUID ${KEMAI_GUID})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${KEMAI_PROJECT_NAME})
    set(CPACK_PACKAGE_EXECUTABLES Kemai;${KEMAI_PROJECT_NAME})
    set(CPACK_CREATE_DESKTOP_LINKS Kemai;${KEMAI_PROJECT_NAME})
    set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.txt)
elseif (APPLE)
    set(OS_BUNDLE MACOSX_BUNDLE)
    #    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12)
    set(DEPLOY_DIR .)
    set(CPACK_GENERATOR DragNDrop)
    install(FILES LICENSE.txt DESTINATION ${DEPLOY_DIR})
else ()
    include(GNUInstallDirs)
    set(OS_BUNDLE)
    set(DEPLOY_DIR bin)
    # install(FILES share/kemai.desktop DESTINATION ${)
    install(FILES share/kemai.desktop
            DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
    install(FILES src/resources/icons/kimai.png
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps/
    )
    install(FILES LICENSE.txt DESTINATION ${CMAKE_INSTALL_DOCDIR}/)
endif ()

# Project code
add_subdirectory(src)

# Install targets and files
install(TARGETS Kemai DESTINATION ${DEPLOY_DIR})

if (WIN32)
    windeployqt(Kemai ${DEPLOY_DIR})
    windeployopenssl(${DEPLOY_DIR})
endif ()

include(CPack)
