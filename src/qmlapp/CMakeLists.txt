project(KemaiQml)


#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

if (ANDROID)
    add_library(${PROJECT_NAME} SHARED)
else ()
    add_executable(${PROJECT_NAME} ${OS_BUNDLE})
endif ()

set(SRCS
        main.cpp
        activityfiltermodel.cpp
        kimaibasedatamodel.cpp
        kimaiclientbridge.cpp
        projectfiltermodel.cpp
        settingsviewbridge.cpp
        timesheetviewbridge.cpp)

set(HDRS
        activityfiltermodel.h
        kimaibasedatamodel.h
        kimaiclientbridge.h
        projectfiltermodel.h
        settingsviewbridge.h
        timesheetviewbridge.h)

set(RESX
        qml/qml.qrc
        resources/resources.qrc)

if (WIN32)
    list(APPEND SRCS ${CMAKE_SOURCE_DIR}/bundle/windows/kemai.rc)
elseif (APPLE)
    set(KEMAI_ICNS "${CMAKE_SOURCE_DIR}/bundle/macos/kemai.icns")
    set_source_files_properties(${KEMAI_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/bundle/macos/Info.plist)
endif ()

target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${HDRS} ${RESX} ${KEMAI_ICNS})
target_compile_definitions(${PROJECT_NAME}
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME}
        KemaiCore
        Qt::Gui Qt::Quick)
