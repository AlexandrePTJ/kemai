project(Kemai)

qt_standard_project_setup(
        I18N_TRANSLATED_LANGUAGES cs de el fr hr it nb_NO nl tr
        I18N_SOURCE_LANGUAGE en)

configure_file(kemaiConfig.h.in ${PROJECT_BINARY_DIR}/kemaiConfig.h)

# Sources
set(SRCS
        client/kimaiAPI.cpp
        client/kimaiCache.cpp
        client/kimaiClient.cpp
        client/kimaiFeatures.cpp
        client/kimaiReply.cpp
        client/parser.cpp
        context/kemaiSession.cpp
        gui/aboutDialog.cpp
        gui/activityDialog.cpp
        gui/activityWidget.cpp
        gui/autoCompleteComboBox.cpp
        gui/customerDialog.cpp
        gui/durationEdit.cpp
        gui/loggerWidget.cpp
        gui/loginDialog.cpp
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
        models/loggerTreeModel.cpp
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
        client/kimaiFeatures.h
        client/kimaiReply.h
        client/parser.h
        context/kemaiSession.h
        gui/aboutDialog.h
        gui/activityDialog.h
        gui/activityWidget.h
        gui/autoCompleteComboBox.h
        gui/customerDialog.h
        gui/durationEdit.h
        gui/loggerWidget.h
        gui/loginDialog.h
        gui/mainWindow.h
        gui/projectDialog.h
        gui/settingsDialog.h
        gui/taskWidget.h
        gui/timeSheetListWidgetItem.h
        misc/customFmt.h
        misc/dataReader.h
        misc/helpers.h
        models/kimaiDataListModel.h
        models/kimaiDataSortFilterProxyModel.h
        models/loggerTreeModel.h
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
        gui/loggerWidget.ui
        gui/loginDialog.ui
        gui/mainWindow.ui
        gui/projectDialog.ui
        gui/settingsDialog.ui
        gui/taskWidget.ui
        gui/timeSheetListWidgetItem.ui)

# OS Specifics
set(KEMAI_ADDITIONAL_LIBS)
if (WIN32)
    list(APPEND SRCS
            ${CMAKE_SOURCE_DIR}/bundle/windows/kemai.rc
            monitor/windowsDesktopEventsMonitor.cpp)
    list(APPEND HDRS
            monitor/windowsDesktopEventsMonitor.h)
    list(APPEND KEMAI_ADDITIONAL_LIBS
            Wtsapi32)
elseif (APPLE)
    list(APPEND SRCS
            monitor/macDesktopEventsMonitor.mm)
    list(APPEND HDRS
            monitor/macDesktopEventsMonitor.h)

    set(KEMAI_ICNS "${CMAKE_SOURCE_DIR}/bundle/macos/kemai.icns")
    set_source_files_properties(${KEMAI_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    qt_add_resources(RESX ${KEMAI_ICNS})

    list(APPEND KEMAI_ADDITIONAL_LIBS "-framework CoreGraphics")
elseif (UNIX)
    list(APPEND SRCS
            monitor/linuxDesktopEventsMonitor.cpp)
    list(APPEND HDRS
            monitor/linuxDesktopEventsMonitor.h)
    list(APPEND KEMAI_ADDITIONAL_LIBS
            Xss X11)
endif ()

# Target configuration
qt_add_resources(RESX
        resources/kemai.qrc)

qt_add_executable(${PROJECT_NAME}
        WIN32 MACOSX_BUNDLE
        ${SRCS} ${HDRS} ${UIS} ${RESX})

set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/bundle/macos/Info.plist)

qt_add_translations(
        TARGETS ${PROJECT_NAME}
        TS_FILE_DIR resources/l10n
        RESOURCE_PREFIX l10n)


target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Widgets Qt::Network spdlog::spdlog magic_enum::magic_enum range-v3::range-v3 ${KEMAI_ADDITIONAL_LIBS})

# Install configuration
install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

qt_generate_deploy_app_script(
        TARGET ${PROJECT_NAME}
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR)

install(SCRIPT ${deploy_script})
