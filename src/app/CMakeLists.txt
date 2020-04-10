project(Kemai)

find_package(Qt5 COMPONENTS Widgets Network CONFIG REQUIRED)

configure_file(kemai_version.h.in ${PROJECT_BINARY_DIR}/kemai_version.h)

add_executable(${PROJECT_NAME} ${OS_BUNDLE})

set(SRCS
        activitywidget.cpp
        customerdialog.cpp
        main.cpp
        mainwindow.cpp
        settings.cpp
        settingswidget.cpp)

set(HDRS
        activitywidget.h
        customerdialog.h
        mainwindow.h
        settings.h
        settingswidget.h)

set(UIS
        activitywidget.ui
        customerdialog.ui
        mainwindow.ui
        settingswidget.ui)

set(RESX
        icons.qrc)

if (WIN32)
    list(APPEND SRCS resources/windows/kemai.rc)
elseif(APPLE)
    set(KEMAI_ICNS "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/kemai.icns")
    set_source_files_properties(${KEMAI_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/Info.plist)
endif()

target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${HDRS} ${UIS} ${RESX} ${KEMAI_ICNS})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets KemaiClient)
