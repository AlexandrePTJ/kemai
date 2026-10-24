project(Kemai)

set(KEMAI_QML_FILES
        qml/Components/AccountMenu.qml
        qml/Components/ActiveHintBar.qml
        qml/Components/ActivityComboBox.qml
        qml/Components/CalendarView.qml
        qml/Components/DashedBorder.qml
        qml/Components/DialogButton.qml
        qml/Components/EntriesHeader.qml
        qml/Components/Icon.qml
        qml/Components/KemaiTextField.qml
        qml/Components/LucideIcons.js
        qml/Components/NotificationBanner.qml
        qml/Components/PrimaryEditButton.qml
        qml/Components/PrimaryRedButton.qml
        qml/Components/ProfileComboBox.qml
        qml/Components/ProfileEntryDelegate.qml
        qml/Components/ProfileForm.qml
        qml/Components/ProjectActivityField.qml
        qml/Components/SecondaryButton.qml
        qml/Components/SettingsRow.qml
        qml/Components/SettingsSection.qml
        qml/Components/SettingsToggle.qml
        qml/Components/Spinner.qml
        qml/Components/StartStopButton.qml
        qml/Components/TagsInput.qml
        qml/Components/WheelScroller.qml
        qml/Components/TimeEntryDelegate.qml
        qml/Components/TimeWheel.qml
        qml/Components/TimerBar.qml
        qml/Views/DateTimeEditDialog.qml
        qml/Views/LoginView.qml
        qml/Views/LogoutConfirmDialog.qml
        qml/Views/MainView.qml
        qml/Views/ProfileManagerDialog.qml
        qml/Views/SettingsDialog.qml
        qml/Views/TimeEntryEditDialog.qml
        qml/Views/TimesheetHistoryView.qml
        qml/Main.qml
        qml/Theme.qml
)

set(KEMAI_QML_SOURCES
        context/appPreferences.cpp
        context/appPreferences.h
        context/kemaiContext.cpp
        context/kemaiContext.h
        context/sessionContext.cpp
        context/sessionContext.h
        context/profileManager.cpp
        context/profileManager.h
        helpers/qmlFormatHelpers.cpp
        helpers/qmlFormatHelpers.h
        model/activityFilterProxyModel.cpp
        model/activityFilterProxyModel.h
)

qt_add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
)

set_source_files_properties(qml/Theme.qml
        PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

# Presence monitor: one backend per platform
list(APPEND KEMAI_QML_SOURCES monitor/presenceMonitor.cpp monitor/presenceMonitor.h)
if (WIN32)
    list(APPEND KEMAI_QML_SOURCES monitor/windowsPresenceBackend.cpp)
elseif (APPLE)
    list(APPEND KEMAI_QML_SOURCES monitor/macPresenceBackend.mm)
else ()
    list(APPEND KEMAI_QML_SOURCES monitor/linuxPresenceBackend.cpp monitor/linuxPresenceBackend.h)
endif ()

qt_add_qml_module(${PROJECT_NAME}
        URI Kemai
        QML_FILES ${KEMAI_QML_FILES}
        SOURCES ${KEMAI_QML_SOURCES}
)

# qt_add_translations(${PROJECT_NAME}
#         TS_FILE_DIR translations
# )

qt_add_resources(
        ${PROJECT_NAME} "assets"
        FILES
        assets/fonts/RaveoVF.ttf
        assets/images/kimai_logo.svg
)

qt_add_resources(
        ${PROJECT_NAME} "controls_conf"
        FILES "qtquickcontrols2.conf")

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/context
        ${CMAKE_CURRENT_SOURCE_DIR}/helpers
        ${CMAKE_CURRENT_SOURCE_DIR}/model
        ${CMAKE_CURRENT_SOURCE_DIR}/monitor)

target_compile_definitions(${PROJECT_NAME}
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME}
        PRIVATE KemaiCore Qt6::Gui Qt6::Quick spdlog::spdlog magic_enum::magic_enum)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE wtsapi32)
elseif (APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreGraphics" "-framework Foundation")
else ()
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::DBus)
    # X11 sessions outside GNOME/KDE: idle time from the screensaver extension, when available at build time
    find_package(PkgConfig QUIET)
    if (PkgConfig_FOUND)
        pkg_check_modules(XCB_SCREENSAVER IMPORTED_TARGET xcb xcb-screensaver)
    endif ()
    if (XCB_SCREENSAVER_FOUND)
        target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::XCB_SCREENSAVER)
        target_compile_definitions(${PROJECT_NAME} PRIVATE KEMAI_HAS_XCB_SCREENSAVER)
    endif ()
endif ()

install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
qt_generate_deploy_qml_app_script(
        TARGET ${PROJECT_NAME}
        OUTPUT_SCRIPT deploy_script
)
install(SCRIPT ${deploy_script})
