project(Kemai)

configure_file(kemai_version.h.in ${PROJECT_BINARY_DIR}/kemai_version.h)

add_executable(${PROJECT_NAME} ${OS_BUNDLE})

set(SRCS
        activitydialog.cpp
        activitywidget.cpp
        customerdialog.cpp
        datareader.cpp
        durationedit.cpp
        main.cpp
        mainwindow.cpp
        projectdialog.cpp
        settingswidget.cpp)

set(HDRS
        activitydialog.h
        activitywidget.h
        customerdialog.h
        datareader.h
        durationedit.h
        mainwindow.h
        projectdialog.h
        settingswidget.h)

set(UIS
        activitydialog.ui
        activitywidget.ui
        customerdialog.ui
        mainwindow.ui
        projectdialog.ui
        settingswidget.ui)

set(RESX
        data.qrc
        icons.qrc)

if (WIN32)
    list(APPEND SRCS resources/windows/kemai.rc)
elseif (APPLE)
    set(KEMAI_ICNS "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/kemai.icns")
    set_source_files_properties(${KEMAI_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/Info.plist)
endif ()

target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${HDRS} ${UIS} ${RESX} ${KEMAI_ICNS})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} Qt::Widgets KemaiCore)
