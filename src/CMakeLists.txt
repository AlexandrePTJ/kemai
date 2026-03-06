project(Kemai)

#
# Source listing
#
set(KEMAI_QML_FILES
        qml/Components/CurrentTimesheetSelectionItem.qml
        qml/Components/KemaiTextField.qml
        qml/Components/PrimaryEditButton.qml
        qml/Components/PrimaryRedButton.qml
        qml/Components/ProfileComboBox.qml
        qml/Components/ProfileEntryDelegate.qml
        qml/Components/ProfileForm.qml
        qml/Components/TimeEntryDelegate.qml
        qml/Components/TimesheetHistoryItem.qml
        qml/Views/LoginView.qml
        qml/Views/MainView.qml
        qml/Views/ProfileManagerDialog.qml
        qml/Main.qml
        qml/Theme.qml
)

set(KEMAI_QML_SOURCES
        cpp/context/applicationContext.cpp
        cpp/context/applicationContext.h
        cpp/context/profileManager.cpp
        cpp/context/profileManager.h
)

set(KEMAI_CPP_FILES
        cpp/main.cpp
        cpp/client/kimaiSystem.h
        cpp/client/kimaiSystem.cpp
        cpp/client/kimaiResources.h
        cpp/client/kimaiResources.cpp
        cpp/client/kimaiTimesheets.h
        cpp/client/kimaiTimesheets.cpp
        cpp/client/kimaiClient.h
        cpp/client/kimaiClient_p.h
        cpp/client/kimaiClient.cpp
        cpp/data/profile.cpp
        cpp/data/profile.h
        cpp/misc/jsonHelpers.h
        cpp/misc/jsonHelpers.cpp
        cpp/misc/pathHelpers.h
        cpp/misc/pathHelpers.cpp
        cpp/model/profile.cpp
        cpp/model/profile.h
        cpp/storage/profileStore.cpp
        cpp/storage/profileStore.h
        cpp/storage/secretStorage.cpp
        cpp/storage/secretStorage.h
)

#
# Target configuration
#
qt_standard_project_setup(
        REQUIRES 6.10
        # I18N_TRANSLATED_LANGUAGES fr
)

qt_add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${KEMAI_CPP_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
)

set_source_files_properties(qml/Theme.qml
        PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

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

configure_file(cpp/kemaiConfig.h.in ${PROJECT_BINARY_DIR}/cpp/kemaiConfig.h)

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cpp/context)

target_compile_definitions(${PROJECT_NAME}
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME}
        PRIVATE Qt6::Gui Qt6::Network Qt6::Quick spdlog::spdlog magic_enum::magic_enum qt6keychain)

#
# Install
#
install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
qt_generate_deploy_qml_app_script(
        TARGET ${PROJECT_NAME}
        OUTPUT_SCRIPT deploy_script
)
install(SCRIPT ${deploy_script})