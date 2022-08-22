cmake_minimum_required(VERSION 3.11)
project(KemaiProject VERSION 0.8.0 LANGUAGES CXX)

# Point CMake to the custom modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Install spdlog
include(FetchContent)
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.10.0)
FetchContent_GetProperties(spdlog)
if (NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
    add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
endif ()
include_directories(${spdlog_SOURCE_DIR}/include)

# Common configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
else ()
    set(OS_BUNDLE)
    set(DEPLOY_DIR bin)
endif ()

# Project code
add_subdirectory(src)

# Install targets and files
install(TARGETS Kemai DESTINATION ${DEPLOY_DIR})
install(FILES LICENSE.txt DESTINATION ${DEPLOY_DIR})

if (WIN32)
    windeployqt(Kemai ${DEPLOY_DIR})
    windeployopenssl(${DEPLOY_DIR})
endif ()

include(CPack)
