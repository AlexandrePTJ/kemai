cmake_minimum_required(VERSION 3.11)
project(KemaiProject VERSION 0.0.5 LANGUAGES CXX)

# Point CMake to the custom modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Common configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    set(OS_BUNDLE )

    # Enable isos646
    add_compile_options("/FIiso646.h")

    # Add Qt libs and installer for windows platform
    include(WinDeployQt)

    # Package
    set(DEPLOY_DIR .)
    set(CPACK_GENERATOR WIX)
    set(CPACK_WIX_UPGRADE_GUID ${KEMAI_GUID})
    set(CPACK_WIX_PRODUCT_GUID ${KEMAI_GUID})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${KEMAI_PROJECT_NAME})
    set(CPACK_PACKAGE_EXECUTABLES Kemai;${KEMAI_PROJECT_NAME})
    set(CPACK_CREATE_DESKTOP_LINKS Kemai;${KEMAI_PROJECT_NAME})
    set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.txt)
elseif(APPLE)
    set(OS_BUNDLE MACOSX_BUNDLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12)
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
endif ()

include(CPack)
