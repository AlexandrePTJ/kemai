project(Kemai)

set(KEMAI_QML_FILES
        qml/Components/AccountMenu.qml
        qml/Components/ActiveHintBar.qml
        qml/Components/AwayStopper.qml
        qml/Components/CreateOffer.qml
        qml/Components/ActivityComboBox.qml
        qml/Components/CalendarView.qml
        qml/Components/CustomFieldRow.qml
        qml/Components/CustomFieldsSection.qml
        qml/Components/DashedBorder.qml
        qml/Components/DialogButton.qml
        qml/Components/EntriesHeader.qml
        qml/Components/FingerprintBlock.qml
        qml/Components/FormField.qml
        qml/Components/FormTextField.qml
        qml/Components/Icon.qml
        qml/Components/KemaiTextField.qml
        qml/Components/MenuEntry.qml
        qml/Components/LucideIcons.js
        qml/Components/NotificationBanner.qml
        qml/Components/PrimaryEditButton.qml
        qml/Components/PrimaryRedButton.qml
        qml/Components/ProfileComboBox.qml
        qml/Components/ProfileEntryDelegate.qml
        qml/Components/ProfileForm.qml
        qml/Components/ProjectActivityField.qml
        qml/Components/SearchSelect.qml
        qml/Components/SecondaryButton.qml
        qml/Components/SegmentedControl.qml
        qml/Components/SettingsRow.qml
        qml/Components/SettingsSection.qml
        qml/Components/SettingsStepper.qml
        qml/Components/SettingsToggle.qml
        qml/Components/Spinner.qml
        qml/Components/StartStopButton.qml
        qml/Components/TagsInput.qml
        qml/Components/TaskFilterField.qml
        qml/Components/TasksPane.qml
        qml/Components/WheelScroller.qml
        qml/Components/TimeEntryDelegate.qml
        qml/Components/TrayIcon.qml
        qml/Components/TimeWheel.qml
        qml/Components/TimerBar.qml
        qml/Components/UpdateStrip.qml
        qml/Components/ViewSwitch.qml
        qml/Views/AboutDialog.qml
        qml/Views/CertificatePrompt.qml
        qml/Views/DateTimeEditDialog.qml
        qml/Views/LogJournalDialog.qml
        qml/Views/LoginView.qml
        qml/Views/LegacyImportDialog.qml
        qml/Views/LogoutConfirmDialog.qml
        qml/Views/MainView.qml
        qml/Views/ProfileManagerDialog.qml
        qml/Views/ResourceCreationDialog.qml
        qml/Views/SettingsDialog.qml
        qml/Views/TimeEntryEditDialog.qml
        qml/Views/ThirdParty.js
        qml/Views/TimesheetHistoryView.qml
        qml/Main.qml
        qml/Theme.qml
)

set(KEMAI_QML_SOURCES
        context/appPreferences.cpp
        context/appPreferences.h
        context/buildInfo.cpp
        context/buildInfo.h
        context/customFieldsForm.cpp
        context/customFieldsForm.h
        context/kemaiContext.cpp
        context/kemaiContext.h
        context/logJournal.cpp
        context/logJournal.h
        context/referenceData.cpp
        context/referenceData.h
        context/sessionContext.cpp
        context/sessionContext.h
        context/updateNotifier.cpp
        context/updateNotifier.h
        context/profileManager.cpp
        context/profileManager.h
        helpers/qmlFormatHelpers.cpp
        helpers/qmlFormatHelpers.h
        model/activityFilterProxyModel.cpp
        model/activityFilterProxyModel.h
        model/logFilterProxyModel.cpp
        model/logFilterProxyModel.h
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
        assets/fonts/RaveoVF-OFL.txt
        assets/images/kimai_logo.svg
        assets/images/tray/kemai-tray-idle.svg
        assets/images/tray/kemai-tray-idle-template.svg
        assets/images/tray/kemai-tray-running.svg
        assets/images/tray/kemai-tray-running-template.svg
        assets/data/countries.json
        assets/data/currencies.json
        assets/data/languages.json
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
        PRIVATE KemaiCore Qt6::Gui Qt6::Quick Qt6::LabsPlatform spdlog::spdlog magic_enum::magic_enum)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE wtsapi32)
    # Icon and product metadata (name, version) of Kemai.exe; the numeric version is 0.0.0.0 for development builds
    set(KEMAI_WINDOWS_ICON "${CMAKE_SOURCE_DIR}/bundle/windows/kemai.ico")
    set(KEMAI_RC_VERSION "${KemaiProject_VERSION_MAJOR},${KemaiProject_VERSION_MINOR},${KemaiProject_VERSION_PATCH},0")
    configure_file(kemai.rc.in ${CMAKE_CURRENT_BINARY_DIR}/kemai.rc @ONLY)
    target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/kemai.rc)
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
if (UNIX AND NOT APPLE)
    # Desktop integration, named after the app ID (main.cpp sets it as the desktop file name)
    install(FILES ${CMAKE_SOURCE_DIR}/packaging/linux/io.github.AlexandrePTJ.Kemai.desktop
            DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/images/kimai_logo.svg
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
            RENAME io.github.AlexandrePTJ.Kemai.svg)

    # AppStream metadata: releases from the changelog, screenshots at the build's tag
    include(changelogToMetainfo)
    kemai_metainfo_releases(${CMAKE_SOURCE_DIR}/CHANGELOG.md "${KEMAI_VERSION}" KEMAI_METAINFO_RELEASES)
    if (KEMAI_VERSION)
        set(KEMAI_SCREENSHOTS_REF ${KEMAI_VERSION})
    else ()
        set(KEMAI_SCREENSHOTS_REF develop)
    endif ()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/CHANGELOG.md)
    configure_file(${CMAKE_SOURCE_DIR}/packaging/linux/io.github.AlexandrePTJ.Kemai.metainfo.xml.in
            ${CMAKE_CURRENT_BINARY_DIR}/io.github.AlexandrePTJ.Kemai.metainfo.xml @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/io.github.AlexandrePTJ.Kemai.metainfo.xml
            DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
endif ()
if (WIN32)
    # MSVC runtime next to Kemai.exe, so the installed folder runs without the Visual C++ redistributable installed
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_BINDIR})
    include(InstallRequiredSystemLibraries)
    # ... instead of the vc_redist installer windeployqt would add
    set(_kemai_deploy_options DEPLOY_TOOL_OPTIONS --no-compiler-runtime)
endif ()
if (KEMAI_DEPLOY_QT)
    qt_generate_deploy_qml_app_script(
            TARGET ${PROJECT_NAME}
            OUTPUT_SCRIPT deploy_script
            ${_kemai_deploy_options}
    )
    install(SCRIPT ${deploy_script})
endif ()

# Windows installer of stable releases (cpack -G WIX, WiX 4+ .NET tool); none for development builds.
# Same upgrade code as the 0.x installers; those were per-user installs, which a per-machine MSI cannot upgrade, so
# setup stops until Kemai 0.11 is uninstalled (packaging/windows/legacy-check.xml). Later stable MSIs upgrade it.
if (WIN32 AND KEMAI_VERSION)
    set(CPACK_GENERATOR WIX)
    set(CPACK_WIX_VERSION 4)
    set(CPACK_PACKAGE_NAME "Kemai")
    set(CPACK_PACKAGE_VENDOR "Alexandre Petitjean")
    set(CPACK_PACKAGE_VERSION "${KEMAI_VERSION}")
    set(CPACK_PACKAGE_FILE_NAME "Kemai-${KEMAI_VERSION}-windows-x64")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Kemai")
    set(CPACK_PACKAGE_EXECUTABLES Kemai Kemai)
    set(CPACK_CREATE_DESKTOP_LINKS Kemai)
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
    set(CPACK_WIX_UPGRADE_GUID "88815E44-85A0-469C-9740-B4887D456BAA")
    set(CPACK_WIX_INSTALL_SCOPE perMachine)
    set(CPACK_WIX_PRODUCT_ICON "${KEMAI_WINDOWS_ICON}")
    set(CPACK_WIX_PATCH_FILE "${CMAKE_SOURCE_DIR}/packaging/windows/legacy-check.xml")
    include(CPack)
endif ()
