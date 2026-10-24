cmake_minimum_required(VERSION 3.30)

# Version: a release build gets the calendar git tag (KEMAI_VERSION) and its channel (KEMAI_CHANNEL, from the
# tagged branch) from the release workflow; any other build is a development build named after its commit.
set(KEMAI_VERSION "" CACHE STRING "Release version (calendar tag YEAR.MONTH.PATCH); empty for a development build")
set(KEMAI_CHANNEL "" CACHE STRING "Release channel (alpha or stable); required with KEMAI_VERSION")
if (KEMAI_VERSION)
    if (NOT KEMAI_VERSION MATCHES "^20[0-9][0-9]\\.(1[0-2]|[1-9])\\.[0-9]+$")
        message(FATAL_ERROR "KEMAI_VERSION '${KEMAI_VERSION}' is not a calendar version YEAR.MONTH.PATCH")
    endif ()
    if (NOT KEMAI_CHANNEL MATCHES "^(alpha|stable)$")
        message(FATAL_ERROR "KEMAI_CHANNEL must be alpha or stable with KEMAI_VERSION (got '${KEMAI_CHANNEL}')")
    endif ()
    set(_kemai_project_version ${KEMAI_VERSION})
else ()
    set(_kemai_project_version 0.0.0)
endif ()
project(KemaiProject VERSION ${_kemai_project_version} LANGUAGES CXX)

if (KEMAI_VERSION)
    set(KEMAI_DISPLAY_VERSION ${KEMAI_VERSION})
    set(KEMAI_BUILD_CHANNEL ${KEMAI_CHANNEL})
else ()
    # Read at configure time: goes stale after new commits until CMake reconfigures
    set(KEMAI_DISPLAY_VERSION "dev")
    set(KEMAI_BUILD_CHANNEL "dev")
    find_package(Git QUIET)
    if (GIT_FOUND)
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE _kemai_commit OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        if (_kemai_commit)
            set(KEMAI_DISPLAY_VERSION "dev-${_kemai_commit}")
        endif ()
    endif ()
endif ()
message(STATUS "Kemai version ${KEMAI_DISPLAY_VERSION} (${KEMAI_BUILD_CHANNEL})")

# Point CMake to the custom modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/cmake)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

# Common configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Setup output dirs to <build root>/bin and <build root>/lib
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)

# Options
option(KEMAI_ENABLE_UPDATE_CHECK "Allow Kemai to check for update from Github releases" ON)
option(KEMAI_BUILD_LOCAL_DEPENDENCIES "Download and build dependencies (except Qt6)" ON)

# Install dependencies sources to build if required
if (KEMAI_BUILD_LOCAL_DEPENDENCIES)
    include(cmake/get_cpm.cmake)
    CPMAddPackage("gh:fmtlib/fmt#12.2.0")
    CPMAddPackage("gh:Neargye/magic_enum#v0.9.8")
    CPMAddPackage("gh:catchorg/Catch2#v3.16.1")
    CPMAddPackage(URI "gh:gabime/spdlog#v1.17.0" OPTIONS "SPDLOG_FMT_EXTERNAL ON")
    CPMAddPackage(URI "gh:frankosterfeld/qtkeychain#0.17.0" OPTIONS "BUILD_WITH_QT6 ON" "BUILD_SHARED_LIBS OFF" "BUILD_TRANSLATIONS OFF" "BUILD_TESTING OFF")
endif (KEMAI_BUILD_LOCAL_DEPENDENCIES)

# Find dependencies
find_package(fmt REQUIRED)
find_package(magic_enum REQUIRED)
find_package(spdlog REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Gui Network LinguistTools Quick)
find_package(qtkeychain REQUIRED)

# Enable Qt auto-tools globally (AUTOMOC, AUTOUIC, AUTORCC) for all subdirectories
qt_standard_project_setup(REQUIRES 6.11)

# Write version to file to ease scripting
file(WRITE ${CMAKE_BINARY_DIR}/version.txt ${KEMAI_DISPLAY_VERSION})

# Project code
include(CTest)
add_subdirectory(code)
