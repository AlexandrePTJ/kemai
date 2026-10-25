project(kemai_tests)

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)

qt_add_executable(${PROJECT_NAME}
    main.cpp
    mocks/fakeKimaiClient.h
    client/tst_certificateTrust.cpp
    client/tst_kemaiUpdateChecker.cpp
    client/tst_kimaiCache.cpp
    client/tst_kimaiClient.cpp
    client/tst_kimaiTasks.cpp
    context/tst_appPreferences.cpp
    context/tst_profileManager.cpp
    # The preferences are a plain QObject of the app (QML macros only): tested here with their source
    ../app/context/appPreferences.cpp
    ../app/context/appPreferences.h
    context/tst_sessionController.cpp
    misc/tst_formatHelpers.cpp
    misc/tst_fuzzyMatch.cpp
    misc/tst_logFiles.cpp
    misc/tst_windowPlacement.cpp
    model/tst_logEntryModel.cpp
    model/tst_taskModel.cpp
    model/tst_timesheetModel.cpp
    storage/tst_appSettingsStore.cpp
    storage/tst_profileStore.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../app/context)

target_link_libraries(${PROJECT_NAME} PRIVATE
    KemaiCore
    Qt6::QmlIntegration
    Catch2::Catch2
    spdlog::spdlog # tests capture or write logs themselves (core links it privately)
)

# Recorded Kimai replies (data/)
target_compile_definitions(${PROJECT_NAME} PRIVATE KEMAI_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

include(CTest)
include(Catch)

get_target_property(_qt6_core_loc Qt6::Core LOCATION)
get_filename_component(_qt6_bin_dir "${_qt6_core_loc}" DIRECTORY)

catch_discover_tests(${PROJECT_NAME} DL_PATHS "${_qt6_bin_dir}")
