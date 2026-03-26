project(Kemai)

set(KEMAI_QML_FILES
        qml/Components/ActiveHintBar.qml
        qml/Components/EntriesHeader.qml
        qml/Components/KemaiTextField.qml
        qml/Components/PrimaryEditButton.qml
        qml/Components/PrimaryRedButton.qml
        qml/Components/ProfileComboBox.qml
        qml/Components/ProfileEntryDelegate.qml
        qml/Components/ProfileForm.qml
        qml/Components/SecondaryButton.qml
        qml/Components/StartStopButton.qml
        qml/Components/TimeEntryDelegate.qml
        qml/Components/TimerBar.qml
        qml/Views/LoginView.qml
        qml/Views/MainView.qml
        qml/Views/ProfileManagerDialog.qml
        qml/Views/TimesheetHistoryView.qml
        qml/Main.qml
        qml/Theme.qml
)

set(KEMAI_QML_SOURCES
        context/kemaiContext.cpp
        context/kemaiContext.h
        context/sessionContext.cpp
        context/sessionContext.h
        context/profileManager.cpp
        context/profileManager.h
)

qt_add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE main.cpp)

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

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/context)

target_compile_definitions(${PROJECT_NAME}
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME}
        PRIVATE KemaiCore Qt6::Gui Qt6::Quick spdlog::spdlog)

install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
qt_generate_deploy_qml_app_script(
        TARGET ${PROJECT_NAME}
        OUTPUT_SCRIPT deploy_script
)
install(SCRIPT ${deploy_script})
