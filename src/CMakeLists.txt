project(Kemai)

add_executable(${PROJECT_NAME} ${OS_BUNDLE})

configure_file(kemaiConfig.h.in ${PROJECT_BINARY_DIR}/kemaiConfig.h)

# Sources
set(SRCS
        client/kimaiAPI.cpp
        client/kimaiCache.cpp
        client/kimaiClient.cpp
        client/kimaiReply.cpp
        client/parser.cpp
        context/kemaiSession.cpp
        gui/aboutDialog.cpp
        gui/activityDialog.cpp
        gui/activityWidget.cpp
        gui/autoCompleteComboBox.cpp
        gui/customerDialog.cpp
        gui/durationEdit.cpp
        gui/mainWindow.cpp
        gui/projectDialog.cpp
        gui/settingsDialog.cpp
        gui/taskWidget.cpp
        gui/timeSheetListWidgetItem.cpp
        main.cpp
        misc/dataReader.cpp
        misc/helpers.cpp
        models/kimaiDataListModel.cpp
        models/kimaiDataSortFilterProxyModel.cpp
        models/taskFilterProxyModel.cpp
        models/taskListModel.cpp
        monitor/desktopEventsMonitor.cpp
        monitor/kimaiEventsMonitor.cpp
        settings/settings.cpp
        updater/kemaiUpdater.cpp)

set(HDRS
        client/kimaiAPI.h
        client/kimaiCache.h
        client/kimaiClient.h
        client/kimaiClient_p.h
        client/kimaiReply.h
        client/parser.h
        context/kemaiSession.h
        gui/aboutDialog.h
        gui/activityDialog.h
        gui/activityWidget.h
        gui/autoCompleteComboBox.h
        gui/customerDialog.h
        gui/durationEdit.h
        gui/mainWindow.h
        gui/projectDialog.h
        gui/settingsDialog.h
        gui/taskWidget.h
        gui/timeSheetListWidgetItem.h
        misc/dataReader.h
        misc/helpers.h
        models/kimaiDataListModel.h
        models/kimaiDataSortFilterProxyModel.h
        models/taskFilterProxyModel.h
        models/taskListModel.h
        monitor/desktopEventsMonitor.h
        monitor/kimaiEventsMonitor.h
        settings/settings.h
        updater/kemaiUpdater.h
        updater/kemaiUpdater_p.h)

# Forms
set(UIS
        gui/aboutDialog.ui
        gui/activityDialog.ui
        gui/activityWidget.ui
        gui/customerDialog.ui
        gui/mainWindow.ui
        gui/projectDialog.ui
        gui/settingsDialog.ui
        gui/taskWidget.ui
        gui/timeSheetListWidgetItem.ui)


# Localization
include(LocalizationTools)
set(KEMAI_TS_FILES
        resources/l10n/kemai_cs.ts
        resources/l10n/kemai_de.ts
        resources/l10n/kemai_el.ts
        resources/l10n/kemai_fr.ts
        resources/l10n/kemai_hr.ts
        resources/l10n/kemai_it.ts
        resources/l10n/kemai_nb_NO.ts
        resources/l10n/kemai_nl.ts
        resources/l10n/kemai_tr.ts)
qt_add_translation(KEMAI_QM_FILES ${KEMAI_TS_FILES})
add_qm_files_to_qrc(KEMAI_L10N_QRC ${KEMAI_QM_FILES})

# Update .ts files on demand
get_target_property(_lupdate_executable Qt::lupdate IMPORTED_LOCATION)
add_custom_target(kemai-update-ts COMMAND
        ${_lupdate_executable} -no-obsolete ${CMAKE_CURRENT_SOURCE_DIR} -ts ${KEMAI_TS_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Resources
set(RESX
        resources/kemai.qrc
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
    list(APPEND SRCS
            monitor/macDesktopEventsMonitor.mm)
    list(APPEND HDRS
            monitor/macDesktopEventsMonitor.h)
    set(KEMAI_ICNS "${CMAKE_SOURCE_DIR}/bundle/macos/kemai.icns")
    set_source_files_properties(${KEMAI_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/bundle/macos/Info.plist)
    target_link_libraries(${PROJECT_NAME} "-framework CoreGraphics")
elseif (UNIX)
    list(APPEND SRCS
            monitor/linuxDesktopEventsMonitor.cpp)
    list(APPEND HDRS
            monitor/linuxDesktopEventsMonitor.h)
    target_link_libraries(${PROJECT_NAME} Xss X11)
endif ()

# Target configuration
target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${HDRS} ${UIS} ${RESX} ${KEMAI_ICNS})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} Qt::Widgets Qt::Network spdlog::spdlog magic_enum::magic_enum range-v3::range-v3)
