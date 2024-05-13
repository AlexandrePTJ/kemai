cmake_minimum_required(VERSION 3.16)

project(KemaiProject VERSION 0.11.0 LANGUAGES CXX)

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
find_package(Qt6 QUIET COMPONENTS Widgets Network LinguistTools)

# Write version to file to ease packaging
file(WRITE ${CMAKE_BINARY_DIR}/version.txt ${PROJECT_VERSION})

# Project code
add_subdirectory(src)

# Common CPack configuration
set(KEMAI_GUID "88815E44-85A0-469C-9740-B4887D456BAA")
set(KEMAI_PROJECT_NAME "Kemai")
set(CPACK_PACKAGE_NAME ${KEMAI_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

# OS Dependant options / configurations
if (WIN32)
    set(CPACK_GENERATOR WIX)
    set(CPACK_WIX_UPGRADE_GUID ${KEMAI_GUID})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${KEMAI_PROJECT_NAME})
    set(CPACK_PACKAGE_EXECUTABLES Kemai;${KEMAI_PROJECT_NAME})
    set(CPACK_CREATE_DESKTOP_LINKS Kemai;${KEMAI_PROJECT_NAME})
    set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.txt)
elseif (APPLE)
    set(CPACK_GENERATOR DragNDrop)
    install(FILES LICENSE.txt DESTINATION ${DEPLOY_DIR})
else ()
    install(FILES share/kemai.desktop
            DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES src/resources/icons/kimai.png
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps/)
    install(FILES LICENSE.txt DESTINATION ${CMAKE_INSTALL_DOCDIR}/)
endif ()

include(CPack)
