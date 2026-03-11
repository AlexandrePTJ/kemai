cmake_minimum_required(VERSION 3.30)

string(TIMESTAMP _ver_year  "%Y")
string(TIMESTAMP _ver_month "%m")
math(EXPR _ver_month "${_ver_month} + 0")  # strip leading zero (03 → 3)
project(KemaiProject VERSION "${_ver_year}.${_ver_month}" LANGUAGES CXX)

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
    CPMAddPackage("gh:fmtlib/fmt#12.1.0")
    CPMAddPackage("gh:Neargye/magic_enum#v0.9.7")
    CPMAddPackage("gh:catchorg/Catch2#v3.8.1")
    CPMAddPackage(URI "gh:gabime/spdlog#v1.17.0" OPTIONS "SPDLOG_FMT_EXTERNAL ON")
    CPMAddPackage(URI "gh:frankosterfeld/qtkeychain#0.15.0" OPTIONS "BUILD_WITH_QT6 ON" "BUILD_SHARED_LIBS OFF" "BUILD_TRANSLATIONS OFF")
endif (KEMAI_BUILD_LOCAL_DEPENDENCIES)

# Find dependencies
find_package(fmt REQUIRED)
find_package(magic_enum REQUIRED)
find_package(spdlog REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Gui Network LinguistTools Quick)
find_package(qtkeychain REQUIRED)

# Enable Qt auto-tools globally (AUTOMOC, AUTOUIC, AUTORCC) for all subdirectories
qt_standard_project_setup(REQUIRES 6.10)

# Write version to file to ease packaging
file(WRITE ${CMAKE_BINARY_DIR}/version.txt ${PROJECT_VERSION})

# Project code
include(CTest)
add_subdirectory(src)

# Tests (opt-in via BUILD_TESTING, default OFF)
if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Common package configuration
set(KEMAI_GUID "88815E44-85A0-469C-9740-B4887D456BAA")
set(KEMAI_PROJECT_NAME "Kemai")
set(CPACK_PACKAGE_NAME ${KEMAI_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
include(CPack)
