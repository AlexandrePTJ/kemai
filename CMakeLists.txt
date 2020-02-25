cmake_minimum_required(VERSION 3.11)
project(Kemai VERSION 0.0.1 LANGUAGES CXX)

# Common configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OS Dependant options / configurations
if (WIN32)
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        set(OS_BUNDLE WIN32)
    endif()
    if (MSVC)
        # Enable isos646
        add_compile_options("/FIiso646.h")
    endif ()
else ()
    set(OS_BUNDLE)
endif ()

#
set(KEMAI_INCDIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${KEMAI_INCDIR})

# Project code
add_subdirectory(src)
