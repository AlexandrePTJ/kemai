project(Kemai)

add_executable(${PROJECT_NAME} ${OS_BUNDLE})

configure_file(kemai_version.h.in ${PROJECT_BINARY_DIR}/kemai_version.h)

# Sources
set(SRCS
        client/kimaiapi.cpp
        client/kimaiclient.cpp
        client/kimaireply.cpp
        client/parser.cpp
        context/kemaisession.cpp
        gui/activitydialog.cpp
        gui/activitywidget.cpp
        gui/customerdialog.cpp
        gui/durationedit.cpp
        gui/mainwindow.cpp
        gui/projectdialog.cpp
        gui/settingsdialog.cpp
        gui/taskwidget.cpp
        main.cpp
        misc/datareader.cpp
        models/taskfilterproxymodel.cpp
        models/tasklistmodel.cpp
        monitor/desktopEventsMonitor.cpp
        settings/settings.cpp
        updater/kemaiupdater.cpp)

set(HDRS
        client/kimaiapi.h
        client/kimaiclient.h
        client/kimaiclient_p.h
        client/kimaireply.h
        client/parser.h
        context/kemaisession.h
        gui/activitydialog.h
        gui/activitywidget.h
        gui/customerdialog.h
        gui/durationedit.h
        gui/mainwindow.h
        gui/projectdialog.h
        gui/settingsdialog.h
        gui/taskwidget.h
        misc/datareader.h
        models/taskfilterproxymodel.h
        models/tasklistmodel.h
        monitor/desktopEventsMonitor.h
        settings/settings.h
        updater/kemaiupdater.h
        updater/kemaiupdater_p.h)

# Forms
set(UIS
        gui/activitydialog.ui
        gui/activitywidget.ui
        gui/customerdialog.ui
        gui/mainwindow.ui
        gui/projectdialog.ui
        gui/settingsdialog.ui
        gui/taskwidget.ui)


# Localization
include(LocalizationTools)
set(KEMAI_TS_FILES
        resources/l10n/kemai_cs.ts
        resources/l10n/kemai_de.ts
        resources/l10n/kemai_el.ts
        resources/l10n/kemai_fr.ts
        resources/l10n/kemai_hr.ts
        resources/l10n/kemai_it.ts
        resources/l10n/kemai_nb_NO.ts)
qt_add_translation(KEMAI_QM_FILES ${KEMAI_TS_FILES})
add_qm_files_to_qrc(KEMAI_L10N_QRC ${KEMAI_QM_FILES})

# Update .ts files on demand
get_target_property(_lupdate_executable Qt::lupdate IMPORTED_LOCATION)
add_custom_target(kemai-update-ts COMMAND
        ${_lupdate_executable} -no-obsolete ${CMAKE_CURRENT_SOURCE_DIR} -ts ${KEMAI_TS_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Resources
set(RESX
        resources/data.qrc
        resources/icons.qrc
        ${KEMAI_L10N_QRC})

# OS Specifics
if (WIN32)
    list(APPEND SRCS
            ${CMAKE_SOURCE_DIR}/bundle/windows/kemai.rc
            monitor/windowsDesktopEventsMonitor.cpp)
    list(APPEND HDRS
            monitor/windowsDesktopEventsMonitor.h)
    target_link_libraries(${PROJECT_NAME} Wtsapi32)
elseif (APPLE)
    set(KEMAI_ICNS "${CMAKE_SOURCE_DIR}/bundle/macos/kemai.icns")
    set_source_files_properties(${KEMAI_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/bundle/macos/Info.plist)
endif ()

# Target configuration
target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${HDRS} ${UIS} ${RESX} ${KEMAI_ICNS})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} Qt::Widgets Qt::Network)
