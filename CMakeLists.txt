cmake_minimum_required(VERSION 3.11)
project(KemaiProject VERSION 0.0.3 LANGUAGES CXX)

# Point CMake to the custom modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Common configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OS Dependant options / configurations
if (WIN32)
    # Hide console
    set(OS_BUNDLE WIN32)

    # Enable isos646
    add_compile_options("/FIiso646.h")

    #
    set(DEPLOY_DIR .)
elseif(APPLE)
    set(OS_BUNDLE MACOSX_BUNDLE)
else ()
    set(OS_BUNDLE)
    set(DEPLOY_DIR bin)
endif ()

# Project code
add_subdirectory(src)

# Install targets and files
if (WIN32 OR (UNIX AND NOT APPLE))
    install(TARGETS Kemai DESTINATION ${DEPLOY_DIR})
    install(FILES LICENSE.txt DESTINATION ${DEPLOY_DIR})
endif()

# Add Qt libs and installer for windows platform
if (WIN32)
    include(WinDeployQt)
    windeployqt(Kemai ${DEPLOY_DIR})

    set(KEMAI_GUID "88815E44-85A0-469C-9740-B4887D456BAA")
    set(KEMAI_CAPTURE_SHORTCUT_NAME "Kemai")
    set(CPACK_GENERATOR WIX)
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
    set(CPACK_PACKAGE_EXECUTABLES Kemai;${KEMAI_CAPTURE_SHORTCUT_NAME})
    set(CPACK_CREATE_DESKTOP_LINKS Kemai;${KEMAI_CAPTURE_SHORTCUT_NAME})
    set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.txt)
    set(CPACK_WIX_UPGRADE_GUID ${KEMAI_GUID})
    set(CPACK_WIX_PRODUCT_GUID ${KEMAI_GUID})
    include(CPack)
endif ()
